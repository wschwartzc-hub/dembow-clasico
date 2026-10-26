#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace dmb;

namespace
{
    const char* macroIds[] = { pid::brillo, pid::duracion, pid::espacio, pid::ataque, pid::grosor, pid::caracter, pid::suciedad,
                               pid::vibrato, pid::glide, pid::modo, pid::octava, pid::fuerza, pid::eco, pid::volumen };

    juce::ValueTree macrosToTree (const Macros& m)
    {
        juce::ValueTree t ("MACROS");
        t.setProperty ("brillo", m.brillo, nullptr);     t.setProperty ("duracion", m.duracion, nullptr);
        t.setProperty ("ataque", m.ataque, nullptr);     t.setProperty ("grosor", m.grosor, nullptr);
        t.setProperty ("suciedad", m.suciedad, nullptr); t.setProperty ("vibrato", m.vibrato, nullptr);
        t.setProperty ("glide", m.glide, nullptr);       t.setProperty ("fuerza", m.fuerza, nullptr);
        t.setProperty ("espacio", m.espacio, nullptr);   t.setProperty ("eco", m.eco, nullptr);
        t.setProperty ("volumen", m.volumen, nullptr);   t.setProperty ("caracter", m.caracter, nullptr);
        t.setProperty ("modo", m.modo, nullptr);         t.setProperty ("octava", m.octava, nullptr);
        return t;
    }

    Macros macrosFromTree (const juce::ValueTree& t)
    {
        Macros m;
        auto f = [&t] (const char* id, float def) { return (float) t.getProperty (id, def); };
        m.brillo = f ("brillo", 50);     m.duracion = f ("duracion", 50); m.ataque = f ("ataque", 50);
        m.grosor = f ("grosor", 50);     m.suciedad = f ("suciedad", 0);  m.vibrato = f ("vibrato", 0);
        m.glide = f ("glide", 0);        m.fuerza = f ("fuerza", 60);     m.espacio = f ("espacio", 15);
        m.eco = f ("eco", 0);            m.volumen = f ("volumen", 0);
        m.caracter = juce::jlimit (0, (int) numCharacters - 1, (int) t.getProperty ("caracter", 0));
        m.modo = juce::jlimit (0, 2, (int) t.getProperty ("modo", 0));
        m.octava = juce::jlimit (-2, 2, (int) t.getProperty ("octava", 0));
        return m;
    }
}

// =============================================================================================
DembowProcessor::DembowProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    for (auto* id : macroIds)
        apvts.addParameterListener (id, this);
    for (int c = 0; c < numCategories; ++c)
        lastInCategory[(size_t) c] = presetIndex (c, 0, 0);
    loadFavorites();
    loadFactoryPreset (presetIndex (catLead, 0, 0));
}

DembowProcessor::~DembowProcessor()
{
    for (auto* id : macroIds)
        apvts.removeParameterListener (id, this);
}

bool DembowProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void DembowProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepare (sampleRate, samplesPerBlock);
    monoTmp.assign ((size_t) std::max (samplesPerBlock, 4096), 0.0f);
    {
        const juce::SpinLock::ScopedLockType sl (patchLock);
        synth.setPatch (pendingPatch);
        patchPending = false;
    }
    keyboardState.reset();
}

void DembowProcessor::parameterChanged (const juce::String&, float)
{
    if (! applyingPreset)
        presetModified = true;
}

// ---------------------------------------------------------------------------------------------
void DembowProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    // un sonido nuevo (preset): se cambia al inicio del bloque y las notas que sonaban se apagan rápido
    if (patchPending.load())
    {
        const juce::SpinLock::ScopedTryLockType sl (patchLock);
        if (sl.isLocked())
        {
            synth.setPatch (pendingPatch);
            synth.allNotesOff (true);
            patchPending = false;
        }
    }

    synth.setMacros (readMacros (apvts));
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
            {
                synth.setTempo (*bpm);
                hostBpm = *bpm;
            }

    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    auto* L = buffer.getWritePointer (0);
    auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    if (R == nullptr)   // bus mono (raro): se calcula en estéreo y se junta al final
    {
        if ((int) monoTmp.size() < n) monoTmp.resize ((size_t) n);
        R = monoTmp.data();
    }

    int pos = 0;
    int played = 0;
    for (const auto meta : midi)
    {
        const int at = juce::jlimit (0, n, meta.samplePosition);
        if (at > pos)
        {
            synth.render (L + pos, R + pos, at - pos);
            pos = at;
        }
        const auto msg = meta.getMessage();
        if (msg.isNoteOn()) ++played;
        synth.handleMidi (msg);
    }
    if (pos < n)
        synth.render (L + pos, R + pos, n - pos);

    if (buffer.getNumChannels() == 1)
        for (int i = 0; i < n; ++i) L[i] = 0.5f * (L[i] + R[i]);
    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, n);

    // para la interfaz: nivel, osciloscopio y actividad
    float peak = 0.0f;
    int w = scopeWrite.load (std::memory_order_relaxed);
    for (int i = 0; i < n; ++i)
    {
        const float v = 0.5f * (L[i] + (buffer.getNumChannels() > 1 ? R[i] : L[i]));
        peak = std::max (peak, std::abs (v));
        scope[w] = v;
        w = (w + 1) & (scopeSize - 1);
    }
    scopeWrite.store (w, std::memory_order_relaxed);
    outputLevel = std::max (peak, outputLevel.load() * 0.85f);
    if (played > 0) notesPlayed += played;
    voicesActive = synth.activeVoiceCount();
}

// =============================================================================================
// Presets
// =============================================================================================
void DembowProcessor::applyPreset (const Patch& p, const Macros& m)
{
    patch = p;
    {
        const juce::SpinLock::ScopedLockType sl (patchLock);
        pendingPatch = p;
        patchPending = true;
    }
    applyingPreset = true;
    writeMacros (apvts, m);
    applyingPreset = false;
    presetModified = false;
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

void DembowProcessor::loadFactoryPreset (int index)
{
    index = juce::jlimit (0, numFactoryPresets - 1, index);
    Patch p;
    Macros m;
    buildCalibratedPreset (index, p, m);
    factoryIndex = index;
    category = presetRef (index).category;
    userFile = juce::File();
    customName.clear();
    applyPreset (p, m);
    rememberInCategory();
}

void DembowProcessor::rememberInCategory()
{
    if (factoryIndex >= 0)
        lastInCategory[(size_t) category] = factoryIndex;
}

bool DembowProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("DMBPRESET"))
        return false;
    const auto tree = juce::ValueTree::fromXml (*xml);
    const Patch p = Patch::fromTree (tree.getChildWithName ("PATCH"));
    const Macros m = macrosFromTree (tree.getChildWithName ("MACROS"));
    factoryIndex = -1;
    userFile = file;
    customName.clear();
    category = juce::jlimit (0, numCategories - 1, (int) tree.getProperty ("category", category));
    applyPreset (p, m);
    return true;
}

void DembowProcessor::loadEntry (const PresetEntry& e)
{
    if (e.isUser()) loadUserPreset (e.file);
    else            loadFactoryPreset (e.factoryIndex);
}

juce::File DembowProcessor::getUserPresetFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
               .getChildFile ("Dembow Clasico").getChildFile ("Presets");
}

juce::Array<juce::File> DembowProcessor::getUserPresets() const
{
    auto files = getUserPresetFolder().findChildFiles (juce::File::findFiles, false, "*.dmbpreset");
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
               { return a.getFileName().compareNatural (b.getFileName()) < 0; });
    return files;
}

bool DembowProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty() || ! getUserPresetFolder().createDirectory())
        return false;
    juce::ValueTree t ("DMBPRESET");
    t.setProperty ("version", 1, nullptr);
    t.setProperty ("name", clean, nullptr);
    t.setProperty ("category", category, nullptr);
    t.setProperty ("basedOn", factoryIndex >= 0 ? presetName (factoryIndex) : getPresetName(), nullptr);
    t.appendChild (patch.toTree(), nullptr);
    t.appendChild (macrosToTree (readMacros (apvts)), nullptr);
    const auto file = getUserPresetFolder().getChildFile (clean + ".dmbpreset");
    if (auto xml = t.createXml(); xml != nullptr && xml->writeTo (file))
    {
        factoryIndex = -1;
        userFile = file;
        customName.clear();
        presetModified = false;
        updateHostDisplay (ChangeDetails().withProgramChanged (true));
        return true;
    }
    return false;
}

bool DembowProcessor::renameUserPreset (const juce::File& file, const juce::String& newName)
{
    const auto clean = juce::File::createLegalFileName (newName.trim());
    if (clean.isEmpty() || ! file.existsAsFile()) return false;
    const auto target = file.getSiblingFile (clean + ".dmbpreset");
    if (target.existsAsFile() && target != file) return false;
    if (! file.moveFileTo (target)) return false;
    if (auto xml = juce::XmlDocument::parse (target))
    {
        xml->setAttribute ("name", clean);
        xml->writeTo (target);
    }
    const auto oldId = "u:" + file.getFileName();
    if (isFavorite (oldId)) { setFavorite (oldId, false); setFavorite ("u:" + target.getFileName(), true); }
    if (userFile == file) userFile = target;
    return true;
}

bool DembowProcessor::deleteUserPreset (const juce::File& file)
{
    if (! file.existsAsFile() || ! file.isAChildOf (getUserPresetFolder())) return false;
    setFavorite ("u:" + file.getFileName(), false);
    if (userFile == file)
    {
        userFile = juce::File();
        customName = file.getFileNameWithoutExtension();
    }
    return file.moveToTrash() || file.deleteFile();
}

void DembowProcessor::stepPreset (int delta)
{
    if (factoryIndex < 0)
    {
        // desde un preset propio o del dado: regresa al último de fábrica de este tipo
        loadFactoryPreset (lastInCategory[(size_t) category]);
        return;
    }
    const int base = category * presetsPerCategory;
    const int inCat = ((factoryIndex - base) + delta + presetsPerCategory) % presetsPerCategory;
    loadFactoryPreset (base + inCat);
}

void DembowProcessor::selectCategory (int c)
{
    c = juce::jlimit (0, numCategories - 1, c);
    loadFactoryPreset (lastInCategory[(size_t) c]);
}

void DembowProcessor::rollDice()
{
    Patch p;
    Macros m;
    juce::String name;
    const uint32_t seed = (uint32_t) juce::Time::currentTimeMillis() * 2654435761u + diceCounter++ * 40503u;
    randomPreset (category, seed, p, m, name);
    factoryIndex = -1;
    userFile = juce::File();
    customName = name;
    applyPreset (p, m);
}

juce::String DembowProcessor::getPresetName() const
{
    if (factoryIndex >= 0) return presetName (factoryIndex);
    if (userFile != juce::File()) return userFile.getFileNameWithoutExtension();
    return customName.isNotEmpty() ? customName : T ("Sin nombre", "Untitled");
}

juce::String DembowProcessor::getPresetDescription() const
{
    if (factoryIndex >= 0) return presetDescription (factoryIndex);
    if (userFile != juce::File()) return T ("Tu preset guardado. Ajusta lo que quieras y vuelve a guardarlo con el mismo nombre para actualizarlo.",
                                            "Your saved preset. Tweak anything and save it again with the same name to update it.");
    return T ("Un sonido nuevo hecho por el dado. Si te gusta, guárdalo.", "A new sound rolled by the dice. If you like it, save it.");
}

juce::String DembowProcessor::getCurrentPresetId() const
{
    if (factoryIndex >= 0) return "f:" + juce::String (factoryIndex);
    if (userFile != juce::File()) return "u:" + userFile.getFileName();
    return {};
}

std::vector<PresetEntry> DembowProcessor::getPresetEntries() const
{
    std::vector<PresetEntry> list;
    list.reserve ((size_t) numFactoryPresets + 64);
    for (int i = 0; i < numFactoryPresets; ++i)
    {
        PresetEntry e;
        const auto r = presetRef (i);
        e.factoryIndex = i;
        e.id = "f:" + juce::String (i);
        e.name = presetName (i);
        e.category = r.category;
        e.archetype = r.archetype;
        list.push_back (std::move (e));
    }
    for (auto& f : getUserPresets())
    {
        PresetEntry e;
        e.file = f;
        e.id = "u:" + f.getFileName();
        e.name = f.getFileNameWithoutExtension();
        if (auto xml = juce::XmlDocument::parse (f))
        {
            e.category = juce::jlimit (0, numCategories - 1, xml->getIntAttribute ("category", 0));
            e.description = xml->getStringAttribute ("basedOn");
        }
        list.push_back (std::move (e));
    }
    return list;
}

// ---------------------------------------------------------------------------------------------
void DembowProcessor::loadFavorites()
{
    juce::PropertiesFile props (appSettings());
    favorites = juce::StringArray::fromTokens (props.getValue ("favorites"), ",", "");
    favorites.removeEmptyStrings();
}

void DembowProcessor::saveFavorites()
{
    juce::PropertiesFile props (appSettings());
    props.setValue ("favorites", favorites.joinIntoString (","));
    props.saveIfNeeded();
}

bool DembowProcessor::isFavorite (const juce::String& id) const { return id.isNotEmpty() && favorites.contains (id); }

void DembowProcessor::setFavorite (const juce::String& id, bool on)
{
    if (id.isEmpty()) return;
    loadFavorites();   // otra ventana del plugin pudo cambiarlos
    if (on) favorites.addIfNotAlreadyThere (id);
    else    favorites.removeString (id);
    saveFavorites();
}

// =============================================================================================
// Estado (proyecto del DAW)
// =============================================================================================
void DembowProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::ValueTree state ("DMBSTATE");
    state.setProperty ("version", 1, nullptr);
    state.setProperty ("factory", factoryIndex, nullptr);
    state.setProperty ("user", userFile.getFullPathName(), nullptr);
    state.setProperty ("custom", customName, nullptr);
    state.setProperty ("category", category, nullptr);
    state.setProperty ("modified", presetModified.load(), nullptr);
    state.setProperty ("keyboard", keyboardOpen, nullptr);
    state.setProperty ("kbOctave", keyboardOctave, nullptr);
    juce::String last;
    for (auto v : lastInCategory) last << v << ",";
    state.setProperty ("lastInCat", last, nullptr);
    state.appendChild (patch.toTree(), nullptr);
    state.appendChild (apvts.copyState(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void DembowProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr || ! xml->hasTagName ("DMBSTATE"))
        return;
    const auto state = juce::ValueTree::fromXml (*xml);

    const Patch p = Patch::fromTree (state.getChildWithName ("PATCH"));
    patch = p;
    {
        const juce::SpinLock::ScopedLockType sl (patchLock);
        pendingPatch = p;
        patchPending = true;
    }

    applyingPreset = true;
    const auto params = state.getChildWithName (apvts.state.getType());
    if (params.isValid())
        apvts.replaceState (params);
    applyingPreset = false;

    factoryIndex = juce::jlimit (-1, numFactoryPresets - 1, (int) state.getProperty ("factory", 0));
    const auto u = state.getProperty ("user").toString();
    userFile = juce::File::isAbsolutePath (u) ? juce::File (u) : juce::File();
    customName = state.getProperty ("custom").toString();
    category = juce::jlimit (0, numCategories - 1, (int) state.getProperty ("category", 0));
    presetModified = (bool) state.getProperty ("modified", false);
    keyboardOpen = (bool) state.getProperty ("keyboard", false);
    keyboardOctave = juce::jlimit (0, 4, (int) state.getProperty ("kbOctave", 2));
    const auto last = juce::StringArray::fromTokens (state.getProperty ("lastInCat").toString(), ",", "");
    for (int c = 0; c < numCategories && c < last.size(); ++c)
        if (last[c].isNotEmpty())
            lastInCategory[(size_t) c] = juce::jlimit (c * presetsPerCategory, (c + 1) * presetsPerCategory - 1, last[c].getIntValue());
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

juce::AudioProcessorEditor* DembowProcessor::createEditor()
{
    return new DembowEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DembowProcessor();
}
