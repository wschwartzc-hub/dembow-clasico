#include "I18n.h"
#include <atomic>

namespace dmb
{
juce::PropertiesFile::Options appSettings()
{
    juce::PropertiesFile::Options o;
    o.applicationName     = "Dembow Clasico";
    o.filenameSuffix      = ".settings";
    o.folderName          = "Dembow Clasico";
    o.osxLibrarySubFolder = "Application Support";
    o.storageFormat       = juce::PropertiesFile::storeAsXML;
    o.processLock         = nullptr;
    return o;
}

namespace
{
    std::atomic<int> current { -1 };
    std::atomic<int> version { 0 };

    int loadInitial()
    {
        juce::PropertiesFile props (appSettings());
        const auto saved = props.getValue ("language");
        if (saved == "es") return 0;
        if (saved == "en") return 1;

        // primera vez: el idioma del sistema (español → español, cualquier otro → inglés)
        const auto sys = juce::SystemStats::getUserLanguage() + juce::SystemStats::getDisplayLanguage();
        return sys.startsWithIgnoreCase ("es") ? 0 : 1;
    }
}

Lang lang()
{
    int v = current.load();
    if (v < 0)
    {
        v = loadInitial();
        current.store (v);
    }
    return v == 1 ? Lang::en : Lang::es;
}

void setLang (Lang l)
{
    if (lang() == l) return;
    current.store (l == Lang::en ? 1 : 0);
    ++version;

    juce::PropertiesFile props (appSettings());
    props.setValue ("language", l == Lang::en ? "en" : "es");
    props.saveIfNeeded();
}

int langVersion() { return version.load(); }

juce::String T (const char* es, const char* en)
{
    return juce::String::fromUTF8 (english() ? en : es);
}

juce::String upper (const juce::String& s)
{
    auto r = s.toUpperCase();
    const char* lo[] = { "á", "é", "í", "ó", "ú", "ñ", "ü" };
    const char* up[] = { "Á", "É", "Í", "Ó", "Ú", "Ñ", "Ü" };
    for (int i = 0; i < 7; ++i)
        r = r.replace (U (lo[i]), U (up[i]));
    return r;
}

juce::String lower (const juce::String& s)
{
    auto r = s.toLowerCase();
    const char* lo[] = { "á", "é", "í", "ó", "ú", "ñ", "ü" };
    const char* up[] = { "Á", "É", "Í", "Ó", "Ú", "Ñ", "Ü" };
    for (int i = 0; i < 7; ++i)
        r = r.replace (U (up[i]), U (lo[i]));
    return r;
}
} // namespace dmb
