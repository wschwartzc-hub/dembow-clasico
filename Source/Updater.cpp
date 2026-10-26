#include "Updater.h"
#include "I18n.h"

#ifndef DMB_UPDATE_URL
 #define DMB_UPDATE_URL ""
#endif

namespace dmb
{
namespace
{
    constexpr juce::int64 dayMs = 24 * 60 * 60 * 1000;

    std::array<int, 3> parts (const juce::String& v)
    {
        std::array<int, 3> p { 0, 0, 0 };
        auto toks = juce::StringArray::fromTokens (v.trim().trimCharactersAtStart ("vV"), ".", "");
        for (int i = 0; i < 3 && i < toks.size(); ++i)
            p[(size_t) i] = toks[i].getIntValue();
        return p;
    }

    juce::StringArray lines (const juce::var& v)
    {
        juce::StringArray out;
        if (auto* a = v.getArray())
            for (auto& x : *a) out.add (x.toString());
        else if (v.toString().isNotEmpty())
            out.add (v.toString());
        return out;
    }
}

UpdateChecker::UpdateChecker() : juce::Thread ("Dembow Clasico update check")
{
    // lo último que se supo (para avisar aunque hoy no se revise)
    juce::PropertiesFile props (appSettings());
    Latest l;
    l.version = props.getValue ("latestVersion");
    l.page    = props.getValue ("latestPage");
    l.mac     = props.getValue ("latestMac");
    l.win     = props.getValue ("latestWin");
    l.notesEs = juce::StringArray::fromLines (props.getValue ("latestNotesEs"));
    l.notesEn = juce::StringArray::fromLines (props.getValue ("latestNotesEn"));
    l.notesEs.removeEmptyStrings();
    l.notesEn.removeEmptyStrings();
    latest = l;
    dismissed = props.getValue ("dismissedVersion");
    if (feedUrl().isEmpty())           status = Status::disabled;
    else if (hasUpdate())              status = Status::available;
}

UpdateChecker::~UpdateChecker()
{
    stopThread (5000);
}

juce::String UpdateChecker::currentVersion()
{
   #ifdef JucePlugin_VersionString
    return JucePlugin_VersionString;
   #else
    return "0.0.0";
   #endif
}

juce::String UpdateChecker::feedUrl() { return DMB_UPDATE_URL; }

bool UpdateChecker::isNewer (const juce::String& candidate, const juce::String& current)
{
    if (candidate.trim().isEmpty()) return false;
    return parts (candidate) > parts (current);
}

UpdateChecker::Latest UpdateChecker::parse (const juce::String& json)
{
    Latest l;
    const auto v = juce::JSON::parse (json);
    if (! v.isObject())
        return l;
    l.version = v["version"].toString().trim();
    l.page    = v["page"].toString();
    l.mac     = v["mac"].toString();
    l.win     = v["win"].toString();
    l.notesEs = lines (v["notes_es"]);
    l.notesEn = lines (v["notes_en"]);
    if (parts (l.version) == std::array<int, 3> { 0, 0, 0 })
        l.version.clear();
    return l;
}

juce::String UpdateChecker::Latest::downloadUrl() const
{
   #if JUCE_MAC
    if (mac.startsWith ("https://")) return mac;
   #elif JUCE_WINDOWS
    if (win.startsWith ("https://")) return win;
   #endif
    return page;
}

bool UpdateChecker::isAutomatic() const
{
    if (feedUrl().isEmpty()) return false;
    juce::PropertiesFile props (appSettings());
    return props.getBoolValue ("checkUpdates", true);
}

void UpdateChecker::setAutomatic (bool on)
{
    juce::PropertiesFile props (appSettings());
    props.setValue ("checkUpdates", on);
    props.saveIfNeeded();
    ++changes;
}

void UpdateChecker::checkIfDue()
{
    if (! isAutomatic() || isThreadRunning())
        return;
    juce::PropertiesFile props (appSettings());
    const auto last = props.getValue ("lastUpdateCheck").getLargeIntValue();
    const auto now = juce::Time::currentTimeMillis();
    if (now - last >= dayMs || now < last)
        checkNow();
}

void UpdateChecker::checkNow()
{
    if (feedUrl().isEmpty() || isThreadRunning())
        return;
    status = Status::checking;
    ++changes;
    startThread (juce::Thread::Priority::background);
}

void UpdateChecker::run()
{
    auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                    .withConnectionTimeoutMs (4000)
                    .withNumRedirectsToFollow (3)
                    .withExtraHeaders ("User-Agent: Dembow Clasico/" + currentVersion() + "\r\nCache-Control: no-cache");

    juce::String body;
    if (auto stream = juce::URL (feedUrl()).createInputStream (opts))
        body = stream->readEntireStreamAsString();

    if (threadShouldExit())
        return;

    juce::PropertiesFile props (appSettings());
    props.setValue ("lastUpdateCheck", juce::String (juce::Time::currentTimeMillis()));
    props.saveIfNeeded();

    const auto l = parse (body);
    if (! l.valid())
    {
        status = Status::failed;
        ++changes;
        return;
    }
    store (l);
}

void UpdateChecker::store (const Latest& l)
{
    {
        const juce::ScopedLock sl (lock);
        latest = l;
    }
    juce::PropertiesFile props (appSettings());
    props.setValue ("latestVersion", l.version);
    props.setValue ("latestPage", l.page);
    props.setValue ("latestMac", l.mac);
    props.setValue ("latestWin", l.win);
    props.setValue ("latestNotesEs", l.notesEs.joinIntoString ("\n"));
    props.setValue ("latestNotesEn", l.notesEn.joinIntoString ("\n"));
    props.saveIfNeeded();
    status = isNewer (l.version, currentVersion()) ? Status::available : Status::upToDate;
    ++changes;
}

UpdateChecker::Latest UpdateChecker::getLatest() const
{
    const juce::ScopedLock sl (lock);
    return latest;
}

bool UpdateChecker::hasUpdate() const
{
    return isNewer (getLatest().version, currentVersion());
}

bool UpdateChecker::shouldNotify() const
{
    if (! hasUpdate()) return false;
    const juce::ScopedLock sl (lock);
    return latest.version != dismissed;
}

void UpdateChecker::dismiss()
{
    {
        const juce::ScopedLock sl (lock);
        dismissed = latest.version;
    }
    juce::PropertiesFile props (appSettings());
    props.setValue ("dismissedVersion", getLatest().version);
    props.saveIfNeeded();
    ++changes;
}
} // namespace dmb
