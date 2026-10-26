// Herramienta de pruebas de Dembow Clásico.
//   DembowTests test                     -> pruebas (sonido, presets, estado, interfaz)
//   DembowTests snapshot <png> [preset] [escala] [extras]   -> captura de la interfaz
//   DembowTests demo <wav>               -> demo de audio con varios tipos

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ui/PresetBrowser.h"
#include <iostream>

using namespace dmb;

static int failures = 0;

static void check (bool ok, const juce::String& what)
{
    std::cout << (ok ? "  ok     " : "  FALLA  ") << what << std::endl;
    if (! ok) ++failures;
}

template <typename T>
static T* findChild (juce::Component& root)
{
    if (auto* t = dynamic_cast<T*> (&root)) return t;
    for (auto* c : root.getChildren())
        if (auto* t = findChild<T> (*c)) return t;
    return nullptr;
}

// renderiza notas MIDI por el procesador completo (como en un DAW)
struct Note { double t; int note; float vel; bool on; };

static juce::AudioBuffer<float> renderNotes (DembowProcessor& p, std::vector<Note> notes, double seconds, int block = 512, double sr = 48000.0)
{
    std::sort (notes.begin(), notes.end(), [] (auto& a, auto& b) { return a.t < b.t; });
    const int total = (int) (seconds * sr);
    juce::AudioBuffer<float> out (2, total);
    out.clear();
    size_t k = 0;
    for (int pos = 0; pos < total; pos += block)
    {
        const int n = std::min (block, total - pos);
        juce::AudioBuffer<float> view (out.getArrayOfWritePointers(), 2, pos, n);
        juce::MidiBuffer midi;
        while (k < notes.size() && notes[k].t * sr < pos + n)
        {
            const int at = juce::jlimit (0, n - 1, (int) (notes[k].t * sr) - pos);
            midi.addEvent (notes[k].on ? juce::MidiMessage::noteOn (1, notes[k].note, notes[k].vel)
                                       : juce::MidiMessage::noteOff (1, notes[k].note), at);
            ++k;
        }
        p.processBlock (view, midi);
    }
    return out;
}

static void stats (const juce::AudioBuffer<float>& b, float& peak, float& rms, bool& finite)
{
    peak = 0; finite = true; double s = 0;
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float v = b.getSample (ch, i);
            if (! std::isfinite (v)) finite = false;
            peak = std::max (peak, std::abs (v));
            s += (double) v * v;
        }
    rms = (float) std::sqrt (s / std::max (1, b.getNumChannels() * b.getNumSamples()));
}

// ---------------------------------------------------------------------------------------------
static void runTests();

static void snapshot (const juce::File& out, int preset, float scale, const juce::String& extra)
{
    juce::ScopedJuceInitialiser_GUI gui;
    DembowProcessor p;
    p.prepareToPlay (48000.0, 512);
    setLang (extra.contains ("en") ? Lang::en : Lang::es);
    if (preset >= 0) p.loadFactoryPreset (preset);
    p.keyboardOpen = extra.contains ("keys");

    std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
    auto* e = dynamic_cast<DembowEditor*> (ed.get());
    const int h = DembowEditor::baseHeight + (p.keyboardOpen ? KeyboardDrawer::height : 0);
    ed->setSize (juce::roundToInt (DembowEditor::baseWidth * scale), juce::roundToInt ((float) h * scale));

    if (extra.contains ("play"))
    {
        // unas notas sonando para que se vea la grabadora y el osciloscopio
        p.keyboardState.noteOn (1, 60, 0.9f);
        p.keyboardState.noteOn (1, 64, 0.9f);
        p.keyboardState.noteOn (1, 67, 0.9f);
        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer midi;
        for (int i = 0; i < 20; ++i) { buf.clear(); p.processBlock (buf, midi); }
    }
    if (extra.contains ("browser"))
        e->getBrowser().open (false, false);
    if (extra.contains ("search"))
    {
        e->getBrowser().open (false, false);
        e->getBrowser().setSearchText ("vintage campana");
    }
    for (int i = 0; i < 3; ++i) e->refreshNow();

    auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    out.deleteFile();
    juce::FileOutputStream os (out);
    juce::PNGImageFormat png;
    png.writeImageToStream (img, os);
    std::cout << "captura: " << out.getFullPathName() << std::endl;
    p.keyboardState.allNotesOff (1);
    ed.reset();
}

// Carga el .vst3 compilado como lo haría un DAW y lo pone a prueba.
static void hostTest (const juce::String& path)
{
    std::cout << "\n[host] " << path << std::endl;
    juce::VST3PluginFormat fmt;
    juce::OwnedArray<juce::PluginDescription> found;
    fmt.findAllTypesForFile (found, path);
    check (found.size() == 1, "el DAW encuentra el plugin");
    if (found.isEmpty()) return;
    const auto& d = *found[0];
    std::cout << "         " << d.name << " · " << d.manufacturerName << " · " << d.version << " · " << d.category << std::endl;
    check (d.isInstrument, "se anuncia como instrumento");

    auto make = [&] (double rate, int block)
    {
        juce::String err;
        auto inst = fmt.createInstanceFromDescription (d, rate, block, err);
        if (inst == nullptr) std::cout << "         error: " << err << std::endl;
        return inst;
    };
    auto a = make (48000.0, 512);
    check (a != nullptr, "se crea una instancia");
    if (a == nullptr) return;
    check (a->acceptsMidi() && ! a->producesMidi(), "recibe MIDI");
    check (a->getTotalNumInputChannels() == 0 && a->getTotalNumOutputChannels() == 2,
           U ("salida estéreo, sin entradas (") + juce::String (a->getTotalNumInputChannels()) + "/" + juce::String (a->getTotalNumOutputChannels()) + ")");
    check (a->getParameters().size() >= 14, U ("expone sus 14 perillas al DAW (") + juce::String (a->getParameters().size()) + ")");

    a->enableAllBuses();
    a->prepareToPlay (48000.0, 512);
    auto play = [] (juce::AudioPluginInstance& p, int blocks, int note)
    {
        juce::AudioBuffer<float> buf (2, 512);
        float peak = 0; bool fin = true;
        for (int b = 0; b < blocks; ++b)
        {
            juce::MidiBuffer midi;
            if (b == 0) { midi.addEvent (juce::MidiMessage::noteOn (1, note, 0.9f), 10); midi.addEvent (juce::MidiMessage::noteOn (1, note + 7, 0.9f), 10); }
            if (b == blocks / 2) { midi.addEvent (juce::MidiMessage::noteOff (1, note), 0); midi.addEvent (juce::MidiMessage::noteOff (1, note + 7), 0); }
            buf.clear();
            p.processBlock (buf, midi);
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 512; ++i) { const float v = buf.getSample (ch, i); fin = fin && std::isfinite (v); peak = std::max (peak, std::abs (v)); }
        }
        return fin ? peak : -1.0f;
    };
    const float pk = play (*a, 60, 60);
    check (pk > 0.01f && pk <= 1.0f, U ("suena al recibir notas (pico ") + juce::String (pk, 2) + ")");

    // mover todas las perillas al azar mientras suena
    juce::Random rnd (3);
    bool ok = true;
    for (int round = 0; round < 20; ++round)
    {
        for (auto* prm : a->getParameters()) prm->setValueNotifyingHost (rnd.nextFloat());
        const float p = play (*a, 6, 48 + round);
        ok = ok && p >= 0.0f && p <= 1.0f;
    }
    check (ok, U ("mover todas las perillas al azar no rompe nada"));

    // guardar en el proyecto y abrir en otra instancia
    juce::MemoryBlock state;
    a->getStateInformation (state);
    auto b = make (44100.0, 256);
    b->enableAllBuses();
    b->prepareToPlay (44100.0, 256);
    b->setStateInformation (state.getData(), (int) state.getSize());
    // (el VST3 agrega parámetros ocultos para los CC de MIDI: esos no se guardan, se comparan solo las perillas)
    bool same = a->getParameters().size() == b->getParameters().size();
    int compared = 0;
    for (int i = 0; same && i < a->getParameters().size(); ++i)
    {
        auto* pa = a->getParameters()[i];
        if (pa->getName (64).startsWith ("MIDI CC") || pa->isMetaParameter() || pa->getName (64).containsIgnoreCase ("program")) continue;
        // se compara lo que muestra la perilla: el valor guardado va redondeado a su paso
        auto* pb = b->getParameters()[i];
        same = pa->getCurrentValueAsText() == pb->getCurrentValueAsText();
        if (! same) std::cout << "         distinto: " << pa->getName (64) << " " << pa->getCurrentValueAsText() << " vs " << pb->getCurrentValueAsText() << std::endl;
        ++compared;
    }
    same = same && compared >= 14;
    check (same && state.getSize() > 100, U ("el proyecto guarda y recupera el sonido (") + juce::String ((int) state.getSize()) + " bytes)");

    // ventana
    std::unique_ptr<juce::AudioProcessorEditor> ed (a->createEditorIfNeeded());
    check (ed != nullptr && ed->getWidth() > 600 && ed->getHeight() > 300,
           U ("abre su ventana (") + juce::String (ed ? ed->getWidth() : 0) + "x" + juce::String (ed ? ed->getHeight() : 0) + ")");
    if (ed != nullptr)
    {
        ed->setSize (900, 480);
        auto img = ed->createComponentSnapshot (ed->getLocalBounds());
        check (img.isValid(), U ("se puede cambiar de tamaño"));
        a->editorBeingDeleted (ed.get());
        ed.reset();
    }
    a->releaseResources();
    b->releaseResources();
}

int main (int argc, char** argv)
{
    const juce::String cmd = argc > 1 ? argv[1] : "test";
    if (cmd == "snapshot" && argc > 2)
    {
        snapshot (juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]),
                  argc > 3 ? juce::String (argv[3]).getIntValue() : -1,
                  argc > 4 ? juce::String (argv[4]).getFloatValue() : 1.0f,
                  argc > 5 ? juce::String (argv[5]) : juce::String());
        return 0;
    }
    if (cmd == "host" && argc > 2)
    {
        juce::ScopedJuceInitialiser_GUI gui;
        hostTest (argv[2]);
        std::cout << "\n" << (failures == 0 ? "TODO BIEN" : juce::String (failures) + " FALLAS") << std::endl;
        return failures == 0 ? 0 : 1;
    }
    if (cmd == "list")     // tabla de sonidos base (para el README)
    {
        setLang (argc > 2 && juce::String (argv[2]) == "en" ? Lang::en : Lang::es);
        for (int c = 0; c < numCategories; ++c)
        {
            std::cout << "\n### " << categoryName (c) << " (500)\n\n";
            juce::StringArray names;
            for (auto& a : archetypes (c)) names.add (juce::String::fromUTF8 (a.name));
            std::cout << names.joinIntoString (" · ") << "\n";
        }
        std::cout << "\nVariaciones:";
        for (int v = 0; v < variationsPerArchetype; ++v) std::cout << (v ? ", " : " ") << variationName (catLead, v);
        std::cout << std::endl;
        return 0;
    }
    if (cmd == "levels")   // diagnóstico: presets cuyo volumen no quedó parejo
    {
        const int step = argc > 2 ? juce::String (argv[2]).getIntValue() : 5;
        for (int i = 0; i < numFactoryPresets; i += step)
        {
            Patch p; Macros m;
            buildCalibratedPreset (i, p, m);
            const float corr = measureLevelCorrection (p, m, presetRef (i).category) - p.g[gp::level];
            if (std::abs (corr) > 1.5f)
                std::cout << i << "\t" << presetName (i) << "\tcorr " << corr << "\tlevel " << p.g[gp::level]
                          << "\tchar " << m.caracter << "\tsuc " << m.suciedad << "\tvol " << m.volumen << std::endl;
        }
        return 0;
    }
    juce::ScopedJuceInitialiser_GUI gui;
    runTests();
    return failures == 0 ? 0 : 1;
}

// ----- utilidades para probar el motor directo ------------------------------------------------
static constexpr double SR = 48000.0;

static juce::AudioBuffer<float> renderSynth (const Patch& p, const Macros& m, std::vector<Note> notes, double seconds,
                                             std::function<void (Synth&, double)> atTime = {})
{
    Synth s;
    s.prepare (SR, 512);
    s.setPatch (p);
    s.setMacros (m);
    std::sort (notes.begin(), notes.end(), [] (auto& a, auto& b) { return a.t < b.t; });
    const int total = (int) (seconds * SR);
    juce::AudioBuffer<float> out (2, total);
    out.clear();
    size_t k = 0;
    const int block = 128;
    for (int pos = 0; pos < total; pos += block)
    {
        while (k < notes.size() && notes[k].t * SR < pos + block)
        {
            if (notes[k].on) s.noteOn (notes[k].note, notes[k].vel); else s.noteOff (notes[k].note);
            ++k;
        }
        if (atTime) atTime (s, pos / SR);
        const int n = std::min (block, total - pos);
        s.render (out.getWritePointer (0, pos), out.getWritePointer (1, pos), n);
    }
    return out;
}

// frecuencia con más energía cerca de lo esperado (Goertzel, ±range cents)
static float detectCents (const juce::AudioBuffer<float>& b, float expectedHz, int from, int len, float range = 80.0f)
{
    auto power = [&] (float f)
    {
        const double w = 2.0 * juce::MathConstants<double>::pi * f / SR;
        const double c = 2.0 * std::cos (w);
        double s1 = 0, s2 = 0;
        for (int i = 0; i < len; ++i)
        {
            const double win = 0.5 - 0.5 * std::cos (2.0 * juce::MathConstants<double>::pi * i / (len - 1));
            const double x = 0.5 * (b.getSample (0, from + i) + b.getSample (1, from + i)) * win;
            const double s0 = x + c * s1 - s2;
            s2 = s1; s1 = s0;
        }
        return s1 * s1 + s2 * s2 - c * s1 * s2;
    };
    float best = 0; double bp = -1;
    for (float c = -range; c <= range; c += 1.0f)
    {
        const double pw = power (expectedHz * std::exp2 (c / 1200.0f));
        if (pw > bp) { bp = pw; best = c; }
    }
    return best;
}

static double energy (const juce::AudioBuffer<float>& b, double fromSec, double toSec)
{
    double s = 0;
    const int a = (int) (fromSec * SR), z = std::min (b.getNumSamples(), (int) (toSec * SR));
    for (int ch = 0; ch < 2; ++ch)
        for (int i = a; i < z; ++i) s += (double) b.getSample (ch, i) * b.getSample (ch, i);
    return s / std::max (1, 2 * (z - a));
}

// proporción de energía en agudos (diferencia de muestras ~ filtro pasa altos)
static double brightness (const juce::AudioBuffer<float>& b, double fromSec, double toSec)
{
    double hi = 0, all = 0;
    const int a = std::max (1, (int) (fromSec * SR)), z = std::min (b.getNumSamples(), (int) (toSec * SR));
    for (int i = a; i < z; ++i)
    {
        const double x = b.getSample (0, i), d = x - b.getSample (0, i - 1);
        hi += d * d; all += x * x;
    }
    return hi / std::max (1e-12, all);
}

static Patch sinePatch()
{
    Patch p;
    p.layer[0][lp::wave1] = (float) waveSine;
    p.g[gp::fCut] = 20000.0f;
    p.g[gp::chMix] = 0.0f;
    return p;
}

static Macros dryMacros()
{
    Macros m;
    m.espacio = 0; m.eco = 0; m.suciedad = 0; m.vibrato = 0; m.glide = 0;
    m.caracter = charClean; m.modo = playPoly; m.octava = 0;
    return m;
}

static int firstOf (int category, const char* archetypeName)
{
    const auto& a = archetypes (category);
    for (int k = 0; k < (int) a.size(); ++k)
        if (juce::String (a[(size_t) k].name) == archetypeName)
            return presetIndex (category, k, 0);
    return presetIndex (category, 0, 0);
}

static void runTests()
{
    setLang (Lang::es);
    constexpr double sr = SR;

    std::cout << "\n[1] Presets: 16 tipos x 500" << std::endl;
    {
        int total = 0;
        bool namesOk = true;
        for (int c = 0; c < numCategories; ++c)
        {
            const auto& a = archetypes (c);
            total += (int) a.size() * variationsPerArchetype;
            juce::StringArray names;
            for (int k = 0; k < presetsPerCategory; ++k)
                names.add (presetName (presetIndex (c, k / variationsPerArchetype, k % variationsPerArchetype)));
            juce::StringArray unique = names;
            unique.removeDuplicates (false);
            namesOk = namesOk && (int) a.size() == archetypesPerCategory && unique.size() == presetsPerCategory;
        }
        check (total == 8000 && numFactoryPresets == 8000, juce::String (total) + " presets de fábrica");
        check (namesOk, U ("cada tipo tiene 20 sonidos base y 500 nombres distintos"));
    }

    std::cout << "\n[2] Suena y no explota" << std::endl;
    {
        DembowProcessor p;
        p.prepareToPlay (sr, 512);
        auto b = renderNotes (p, { { 0.0, 60, 0.9f, true }, { 0.0, 64, 0.9f, true }, { 0.5, 60, 0, false }, { 0.5, 64, 0, false } }, 1.5);
        float pk, rms; bool fin; stats (b, pk, rms, fin);
        check (fin && pk > 0.05f && pk <= 1.0f, U ("el preset inicial suena (pico ") + juce::String (pk, 2) + ")");
    }

    std::cout << "\n[3] Los 8,000 presets suenan, sin ruidos raros ni saturar" << std::endl;
    {
        const auto t0 = juce::Time::getMillisecondCounterHiRes();
        int bad = 0, silent = 0, loud = 0, levelOff = 0, levelChecked = 0;
        float worstLevel = 0.0f;
        juce::String firstBad;
        for (int i = 0; i < numFactoryPresets; ++i)
        {
            Patch p; Macros m;
            buildCalibratedPreset (i, p, m);
            const int cat = presetRef (i).category;
            const int note = cat == catBass ? 36 : 60;
            auto b = renderSynth (p, m, { { 0.0, note, 0.85f, true }, { 0.35, note, 0, false } }, 0.5);
            float pk, rms; bool fin; stats (b, pk, rms, fin);
            if (! fin) { ++bad; if (firstBad.isEmpty()) firstBad = presetName (i); }
            if (rms < 0.0005f) { ++silent; if (firstBad.isEmpty()) firstBad = presetName (i) + " (mudo)"; }
            if (pk > 1.0f) ++loud;
            if (i % 5 == 0)
            {
                // mide el nivel que debería tener (con level = 0) y lo compara con el de la tabla
                const float corr = measureLevelCorrection (p, m, cat) - p.g[gp::level];
                ++levelChecked;
                worstLevel = std::max (worstLevel, std::abs (corr));
                if (std::abs (corr) > 1.5f) { ++levelOff; if (firstBad.isEmpty()) firstBad = presetName (i) + " (nivel " + juce::String (corr, 1) + " dB)"; }
            }
        }
        check (bad == 0, U ("ningún preset da valores inválidos") + (bad ? " (" + juce::String (bad) + ")" : juce::String()));
        check (silent == 0, U ("ningún preset sale mudo") + (silent ? " (" + juce::String (silent) + ")" : juce::String()));
        check (loud == 0, U ("ninguno pasa de 0 dBFS"));
        check (levelOff == 0, U ("volumen parejo dentro de cada tipo (") + juce::String (levelChecked) + U (" medidos, peor ±")
                                + juce::String (worstLevel, 2) + " dB)");
        if (firstBad.isNotEmpty()) std::cout << "         primero con problema: " << firstBad << std::endl;
        std::cout << "         (" << juce::String ((juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0, 1) << " s)" << std::endl;
    }

    std::cout << "\n[4] Afinación y controles principales" << std::endl;
    {
        const auto p = sinePatch();
        auto m = dryMacros();
        auto a = renderSynth (p, m, { { 0.0, 69, 0.8f, true } }, 0.8);
        check (std::abs (detectCents (a, 440.0f, 9600, 24000)) <= 2.0f, U ("La 440 suena en 440 Hz"));

        m.octava = 1;
        auto o = renderSynth (p, m, { { 0.0, 69, 0.8f, true } }, 0.8);
        check (std::abs (detectCents (o, 880.0f, 9600, 24000)) <= 2.0f, U ("OCTAVA +1 sube una octava"));
        m.octava = 0;

        // pitch bend: rango del preset (2 semitonos por defecto)
        auto bent = renderSynth (p, m, { { 0.0, 69, 0.8f, true } }, 0.8, [] (Synth& s, double t)
        {
            if (t > 0.0 && t < 0.01) s.handleMidi (juce::MidiMessage::pitchWheel (1, 16383));
        });
        const float bendHz = 440.0f * std::exp2 (p.g[gp::bend] / 12.0f);
        check (std::abs (detectCents (bent, bendHz, 14400, 19200)) <= 3.0f, U ("la rueda de pitch dobla la nota"));

        // brillo
        Patch saw;
        saw.g[gp::fCut] = 1500.0f; saw.g[gp::fType] = (float) filterLP24; saw.g[gp::chMix] = 0.0f;
        auto mb = dryMacros();
        mb.brillo = 0;   auto dark = renderSynth (saw, mb, { { 0.0, 48, 0.8f, true } }, 0.6);
        mb.brillo = 100; auto light = renderSynth (saw, mb, { { 0.0, 48, 0.8f, true } }, 0.6);
        check (brightness (light, 0.2, 0.6) > 3.0 * brightness (dark, 0.2, 0.6), U ("BRILLO abre el sonido"));

        // volumen
        auto mv = dryMacros();
        auto v0 = renderSynth (p, mv, { { 0.0, 60, 0.5f, true } }, 0.6);
        mv.volumen = -12.0f;
        auto v1 = renderSynth (p, mv, { { 0.0, 60, 0.5f, true } }, 0.6);
        const double dbDiff = 10.0 * std::log10 (energy (v0, 0.2, 0.6) / std::max (1e-12, energy (v1, 0.2, 0.6)));
        check (std::abs (dbDiff - 12.0) < 1.0, U ("VOLUMEN -12 dB baja 12 dB (") + juce::String (dbDiff, 1) + ")");

        // duración (en un pluck)
        Patch pl; Macros pm;
        buildCalibratedPreset (presetIndex (catPluck, 0, 0), pl, pm);
        pm.espacio = 0; pm.eco = 0;
        pm.duracion = 0;   auto shortP = renderSynth (pl, pm, { { 0.0, 60, 0.8f, true } }, 2.0);
        pm.duracion = 100; auto longP  = renderSynth (pl, pm, { { 0.0, 60, 0.8f, true } }, 2.0);
        check (energy (longP, 0.8, 2.0) > 10.0 * energy (shortP, 0.8, 2.0), U ("DURACIÓN alarga la cola del sonido"));

        // espacio y eco dejan cola después de soltar
        auto me = dryMacros();
        auto dry = renderSynth (p, me, { { 0.0, 60, 0.8f, true }, { 0.2, 60, 0, false } }, 1.5);
        me.espacio = 100; auto wet = renderSynth (p, me, { { 0.0, 60, 0.8f, true }, { 0.2, 60, 0, false } }, 1.5);
        me.espacio = 0; me.eco = 100; auto echo = renderSynth (p, me, { { 0.0, 60, 0.8f, true }, { 0.2, 60, 0, false } }, 1.5);
        check (energy (wet, 0.8, 1.5) > 100.0 * energy (dry, 0.8, 1.5) + 1e-9, U ("ESPACIO deja reverberación"));
        check (energy (echo, 0.6, 1.5) > 100.0 * energy (dry, 0.6, 1.5) + 1e-9, U ("ECO repite la nota"));

        // fuerza: con 100 la velocidad cambia mucho el volumen; con 0 casi nada
        auto mf = dryMacros();
        mf.fuerza = 100;
        const double hard = energy (renderSynth (p, mf, { { 0.0, 60, 1.0f, true } }, 0.5), 0.2, 0.5);
        const double soft = energy (renderSynth (p, mf, { { 0.0, 60, 0.3f, true } }, 0.5), 0.2, 0.5);
        mf.fuerza = 0;
        const double hard0 = energy (renderSynth (p, mf, { { 0.0, 60, 1.0f, true } }, 0.5), 0.2, 0.5);
        const double soft0 = energy (renderSynth (p, mf, { { 0.0, 60, 0.3f, true } }, 0.5), 0.2, 0.5);
        check (hard / soft > 4.0 && hard0 / soft0 < 1.5, U ("FUERZA controla cuánto importa qué tan duro tocas"));
    }

    std::cout << "\n[5] Modos de tocar y MIDI" << std::endl;
    {
        const auto p = sinePatch();
        auto m = dryMacros();
        {
            Synth s; s.prepare (SR, 512); s.setPatch (p); s.setMacros (m);
            float L[512], R[512];
            for (int n = 0; n < 20; ++n) s.noteOn (40 + n, 0.8f);
            for (int k = 0; k < 20; ++k) { std::fill (L, L + 512, 0.0f); std::fill (R, R + 512, 0.0f); s.render (L, R, 512); }
            check (s.activeVoiceCount() <= Synth::polyphony, U ("POLY: máximo 16 notas a la vez (") + juce::String (s.activeVoiceCount()) + ")");
        }
        m.modo = playMono;
        {
            Synth s; s.prepare (SR, 512); s.setPatch (p); s.setMacros (m);
            float L[512], R[512];
            s.noteOn (60, 0.8f); s.noteOn (64, 0.8f); s.noteOn (67, 0.8f);
            for (int k = 0; k < 10; ++k) { std::fill (L, L + 512, 0.0f); std::fill (R, R + 512, 0.0f); s.render (L, R, 512); }
            check (s.activeVoiceCount() == 1, U ("MONO: una sola nota a la vez"));
        }
        // mono: al soltar la nota de arriba regresa a la que sigues apretando
        auto back = renderSynth (p, m, { { 0.0, 60, 0.8f, true }, { 0.3, 67, 0.8f, true }, { 0.6, 67, 0, false } }, 1.2);
        check (std::abs (detectCents (back, 261.63f, (int) (0.8 * SR), (int) (0.35 * SR))) <= 3.0f, U ("MONO: regresa a la nota que sigue apretada"));

        m.modo = playLegato; m.glide = 60;
        auto leg = renderSynth (p, m, { { 0.0, 60, 0.8f, true }, { 0.3, 72, 0.8f, true } }, 1.6);
        // a mitad del glide la nota todavía no llegó; al final sí
        const float mid = detectCents (leg, 523.25f, (int) (0.33 * SR), 2400, 1200.0f);
        const float end = detectCents (leg, 523.25f, (int) (1.3 * SR), 12000);
        check (mid < -100.0f && std::abs (end) <= 3.0f, U ("LEGATO + GLIDE: se desliza de una nota a otra"));

        // pedal de sustain
        m = dryMacros();
        Synth s; s.prepare (SR, 512); s.setPatch (p); s.setMacros (m);
        float L[512], R[512];
        auto run = [&] (int blocks) { for (int k = 0; k < blocks; ++k) { std::fill (L, L + 512, 0.0f); std::fill (R, R + 512, 0.0f); s.render (L, R, 512); } };
        s.handleMidi (juce::MidiMessage::controllerEvent (1, 64, 127));
        s.noteOn (60, 0.8f); run (4); s.noteOff (60); run (60);
        const bool held = s.activeVoiceCount() == 1;
        s.handleMidi (juce::MidiMessage::controllerEvent (1, 64, 0)); run (400);
        check (held && s.activeVoiceCount() == 0, U ("el pedal de sustain sostiene y luego suelta"));
        s.noteOn (60, 0.8f); s.noteOn (64, 0.8f); run (4);
        s.handleMidi (juce::MidiMessage::allNotesOff (1)); run (400);
        check (s.activeVoiceCount() == 0, U ("'All notes off' apaga todo"));
    }

    std::cout << "\n[6] Presets: tipos, flechas, dado y favoritos" << std::endl;
    {
        DembowProcessor p;
        p.prepareToPlay (sr, 512);
        p.loadFactoryPreset (presetIndex (catBells, 3, 7));
        check (p.getCategory() == catBells && p.getPresetName() == presetName (presetIndex (catBells, 3, 7)), U ("cargar un preset de campanas"));
        p.stepPreset (+1);
        check (p.getFactoryIndex() == presetIndex (catBells, 3, 8), U ("flecha → siguiente sonido"));
        p.loadFactoryPreset (presetIndex (catBells, 19, 24));
        p.stepPreset (+1);
        check (p.getFactoryIndex() == presetIndex (catBells, 0, 0), U ("del último sonido regresa al primero del mismo tipo"));
        p.selectCategory (catBass);
        check (p.getCategory() == catBass, U ("cambiar de tipo"));
        p.selectCategory (catBells);
        check (p.getFactoryIndex() == presetIndex (catBells, 0, 0), U ("al regresar a un tipo vuelve al último que usaste"));

        p.rollDice();
        check (p.getCategory() == catBells && p.getFactoryIndex() < 0 && p.getPresetName().isNotEmpty(), U ("DADO: sonido nuevo del mismo tipo (") + p.getPresetName() + ")");
        auto b = renderNotes (p, { { 0.0, 60, 0.8f, true }, { 0.5, 60, 0, false } }, 1.0);
        float pk, rms; bool fin; stats (b, pk, rms, fin);
        check (fin && rms > 0.001f && pk <= 1.0f, U ("el sonido del dado suena bien"));
        p.stepPreset (+1);
        check (p.getFactoryIndex() >= 0 && p.getCategory() == catBells, U ("flecha después del dado regresa a los de fábrica"));

        // todos los dados de todos los tipos suenan
        int diceBad = 0;
        for (int c = 0; c < numCategories; ++c)
            for (uint32_t seed = 1; seed <= 25; ++seed)
            {
                Patch rp; Macros rm; juce::String nm;
                randomPreset (c, seed * 7919u, rp, rm, nm);
                auto rb = renderSynth (rp, rm, { { 0.0, c == catBass ? 36 : 60, 0.85f, true }, { 0.3, c == catBass ? 36 : 60, 0, false } }, 0.5);
                float pk2, rms2; bool fin2; stats (rb, pk2, rms2, fin2);
                if (! fin2 || rms2 < 0.0005f || pk2 > 1.0f) ++diceBad;
            }
        check (diceBad == 0, U ("400 tiradas de dado (25 por tipo) suenan bien"));

        const juce::String id = "f:" + juce::String (presetIndex (catLead, 2, 0));
        const bool before = p.isFavorite (id);
        p.setFavorite (id, true);
        DembowProcessor other;
        check (other.isFavorite (id), U ("los favoritos se guardan entre sesiones"));
        p.setFavorite (id, before);
    }

    std::cout << "\n[7] Tus presets: guardar, renombrar y borrar" << std::endl;
    {
        DembowProcessor p;
        p.prepareToPlay (sr, 512);
        p.loadFactoryPreset (presetIndex (catPiano, 4, 2));
        p.apvts.getParameter (pid::brillo)->setValueNotifyingHost (0.9f);
        const juce::String name = "Prueba Dembow " + juce::String (juce::Random::getSystemRandom().nextInt (100000));
        check (p.saveUserPreset (name), U ("guardar un preset propio"));
        juce::File file;
        for (auto& f : p.getUserPresets()) if (f.getFileNameWithoutExtension() == name) file = f;
        check (file.existsAsFile(), U ("aparece en Mis presets"));

        DembowProcessor q;
        q.prepareToPlay (sr, 512);
        check (q.loadUserPreset (file) && std::abs (q.apvts.getParameter (pid::brillo)->getValue() - 0.9f) < 0.01f
               && q.getCategory() == catPiano, U ("se carga igual (con su tipo y perillas)"));
        check (q.renameUserPreset (file, name + " B"), U ("renombrar"));
        juce::File renamed = DembowProcessor::getUserPresetFolder().getChildFile (name + " B.dmbpreset");
        check (renamed.existsAsFile() && ! file.existsAsFile(), U ("el archivo cambió de nombre"));
        check (q.deleteUserPreset (renamed) && ! renamed.existsAsFile(), U ("borrar"));
        const bool hasEntries = p.getPresetEntries().size() >= (size_t) numFactoryPresets;
        check (hasEntries, U ("la lista del gestor tiene los 8,000 + los tuyos"));
    }

    std::cout << "\n[8] El DAW guarda y recupera todo" << std::endl;
    {
        DembowProcessor a;
        a.prepareToPlay (sr, 512);
        a.loadFactoryPreset (presetIndex (catChoir, 5, 11));
        a.apvts.getParameter (pid::espacio)->setValueNotifyingHost (0.77f);
        a.apvts.getParameter (pid::modo)->setValueNotifyingHost (1.0f);
        juce::MemoryBlock mb;
        a.getStateInformation (mb);

        DembowProcessor b;
        b.prepareToPlay (sr, 512);
        b.setStateInformation (mb.getData(), (int) mb.getSize());
        bool same = b.getFactoryIndex() == a.getFactoryIndex() && b.getCategory() == catChoir;
        for (auto* prm : a.getParameters())
            if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
                same = same && std::abs (r->getValue() - b.apvts.getParameter (r->paramID)->getValue()) < 1e-4f;
        check (same, U ("mismo preset y mismas perillas"));
        check (a.isPresetModified() == b.isPresetModified() && a.isPresetModified(), U ("recuerda que lo modificaste"));

        // el sonido es idéntico
        auto ra = renderNotes (a, { { 0.0, 60, 0.8f, true }, { 0.4, 60, 0, false } }, 0.8);
        auto rb = renderNotes (b, { { 0.0, 60, 0.8f, true }, { 0.4, 60, 0, false } }, 0.8);
        double diff = 0, ref = 0;
        for (int i = 0; i < ra.getNumSamples(); ++i) { const double d = ra.getSample (0, i) - rb.getSample (0, i); diff += d * d; ref += (double) ra.getSample (0, i) * ra.getSample (0, i); }
        check (diff < ref * 1e-3, U ("el sonido recuperado es igual"));

        // un sonido del dado también se recupera (no está en la lista de fábrica)
        a.rollDice();
        juce::MemoryBlock md;
        a.getStateInformation (md);
        DembowProcessor c;
        c.prepareToPlay (sr, 512);
        c.setStateInformation (md.getData(), (int) md.getSize());
        check (c.getPresetName() == a.getPresetName() && c.getFactoryIndex() < 0, U ("el sonido del dado se recupera (") + c.getPresetName() + ")");
        c.setStateInformation ("basura", 6);
        check (c.getPresetName().isNotEmpty(), U ("datos dañados no lo rompen"));
    }

    std::cout << "\n[9] Cualquier frecuencia de muestreo y tamaño de bloque" << std::endl;
    {
        bool ok = true;
        for (double rate : { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
            for (int block : { 1, 17, 64, 333, 1024, 4096 })
            {
                DembowProcessor p;
                p.prepareToPlay (rate, block);
                p.loadFactoryPreset (presetIndex (catLead, 1, 9));
                auto b = renderNotes (p, { { 0.0, 60, 0.9f, true }, { 0.0, 67, 0.9f, true }, { 0.2, 60, 0, false } }, 0.4, block, rate);
                float pk, rms; bool fin; stats (b, pk, rms, fin);
                ok = ok && fin && pk <= 1.0f && rms > 0.001f;
            }
        check (ok, U ("44.1k a 192k, bloques de 1 a 4096: suena y sin errores"));

        DembowProcessor p;
        p.prepareToPlay (sr, 512);
        juce::AudioBuffer<float> mono (1, 512);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, 0.9f), 0);
        mono.clear();
        p.processBlock (mono, midi);
        float pk = mono.getMagnitude (0, 512);
        check (std::isfinite (pk) && pk > 0.0f, U ("funciona en pistas mono"));
    }

    std::cout << "\n[10] Uso de CPU (16 notas, el preset más pesado de cada tipo)" << std::endl;
    {
        double worst = 0; juce::String worstName;
        for (int c = 0; c < numCategories; ++c)
            for (int a = 0; a < archetypesPerCategory; a += 4)
            {
                Patch pt; Macros m;
                buildCalibratedPreset (presetIndex (c, a, 0), pt, m);
                std::vector<Note> notes;
                for (int n = 0; n < 16; ++n) notes.push_back ({ 0.0, 48 + n * 2, 0.8f, true });
                const auto t0 = juce::Time::getMillisecondCounterHiRes();
                renderSynth (pt, m, notes, 1.0);
                const double ms = juce::Time::getMillisecondCounterHiRes() - t0;
                if (ms > worst) { worst = ms; worstName = presetName (presetIndex (c, a, 0)); }
            }
        std::cout << "         peor: " << worstName << " = " << juce::String (worst / 10.0, 1) << " % de un núcleo" << std::endl;
        check (worst < 1000.0, U ("tocar 16 notas cuesta menos que tiempo real en esta máquina"));
    }

    std::cout << "\n[11] Interfaz" << std::endl;
    {
        DembowProcessor p;
        p.prepareToPlay (sr, 512);
        for (float scale : { 0.6f, 0.72f, 1.0f, 1.3f })
        {
            std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
            ed->setSize (juce::roundToInt (DembowEditor::baseWidth * scale), juce::roundToInt (DembowEditor::baseHeight * scale));
            auto img = ed->createComponentSnapshot (ed->getLocalBounds());
            check (img.isValid(), U ("se dibuja a escala ") + juce::String (scale, 2));
        }
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        auto* e = dynamic_cast<DembowEditor*> (ed.get());
        const int h0 = ed->getHeight();
        e->setKeyboardOpen (true);
        const int h1 = ed->getHeight();
        e->setKeyboardOpen (false);
        check (h1 > h0 && ed->getHeight() == h0, U ("el teclado abre y cierra (la ventana crece hacia abajo)"));

        auto& br = e->getBrowser();
        br.open (false, false);
        int factory = 0;
        for (auto& en : br.getEntries()) if (! en.isUser()) ++factory;
        check (factory == numFactoryPresets, U ("el gestor muestra los 8,000"));
        br.selectCategory ("all");
        br.setSearchText ("campana");
        int hits = 0;
        for (auto& r : br.getRows()) if (! r.header) ++hits;
        check (hits > 0 && hits < 2000, U ("buscar 'campana' encuentra ") + juce::String (hits));
        br.setSearchText ({});
        br.close();

        if (e->getTypeSelector().onPick) e->getTypeSelector().onPick (catOrgan);
        check (p.getCategory() == catOrgan, U ("tocar un tipo en la interfaz cambia el sonido"));

        setLang (Lang::en);
        e->refreshNow();
        auto img = ed->createComponentSnapshot (ed->getLocalBounds());
        check (img.isValid() && categoryName (catBass) == "Bass", U ("cambio a inglés"));
        setLang (Lang::es);
        e->refreshNow();
    }

    std::cout << "\n" << (failures == 0 ? "TODO BIEN" : juce::String (failures) + " FALLAS") << std::endl;
}
