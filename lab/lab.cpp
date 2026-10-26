// Laboratorio del motor: renderiza sonidos a WAV y mide afinación / niveles.
#include <juce_audio_formats/juce_audio_formats.h>
#include "synth/Synth.h"
#include "presets/Factory.h"
#include "presets/Build.h"
#include "I18n.h"
#include <iostream>

using namespace dmb;
using namespace dmb::build;

static constexpr double SR = 48000.0;

struct Ev { double t; int note; float vel; bool on; };

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b)
{
    f.deleteFile();
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> os (new juce::FileOutputStream (f));
    auto opts = juce::AudioFormatWriterOptions().withSampleRate (SR).withNumChannels (2).withBitsPerSample (24);
    if (auto w = wav.createWriterFor (os, opts))
        w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
}

static juce::AudioBuffer<float> renderEvents (const Patch& p, const Macros& m, std::vector<Ev> evs, double seconds, double* cpuMs = nullptr)
{
    Synth s;
    s.prepare (SR, 512);
    s.setPatch (p);
    s.setMacros (m);
    s.setTempo (95.0);
    std::sort (evs.begin(), evs.end(), [] (auto& a, auto& b) { return a.t < b.t; });
    const int total = (int) (seconds * SR);
    juce::AudioBuffer<float> out (2, total);
    out.clear();
    size_t e = 0;
    const int block = 128;
    const auto t0 = juce::Time::getMillisecondCounterHiRes();
    for (int pos = 0; pos < total; pos += block)
    {
        while (e < evs.size() && evs[e].t * SR < pos + block)
        {
            if (evs[e].on) s.noteOn (evs[e].note, evs[e].vel); else s.noteOff (evs[e].note);
            ++e;
        }
        const int n = std::min (block, total - pos);
        s.render (out.getWritePointer (0, pos), out.getWritePointer (1, pos), n);
    }
    if (cpuMs) *cpuMs = juce::Time::getMillisecondCounterHiRes() - t0;
    return out;
}

static std::vector<Ev> pattern (const juce::String& name, int category)
{
    std::vector<Ev> e;
    const double beat = 60.0 / 95.0;
    auto note = [&] (double startBeats, double lenBeats, int n, float v = 0.85f)
    {
        e.push_back ({ startBeats * beat, n, v, true });
        e.push_back ({ (startBeats + lenBeats) * beat, n, 0.0f, false });
    };
    if (name == "note") { note (0, 2.4, category == catBass ? 36 : 60); return e; }
    if (name == "chord") { for (int n : { 57, 60, 64 }) note (0, 3, n); return e; }
    // frase: Am - F - C - G (la clásica)
    if (category == catBass)
    {
        const int roots[] = { 33, 29, 36, 31 };
        for (int bar = 0; bar < 4; ++bar)
            for (int k : { 0, 1, 2, 3 }) note (bar * 4 + k * 1.0 + (k % 2 ? 0.5 : 0.0), 0.45, roots[bar] + (k == 2 ? 12 : 0));
        return e;
    }
    if (category == catPad || category == catStrings || category == catChoir || category == catOrgan)
    {
        const int ch[4][3] = { { 57, 60, 64 }, { 53, 57, 60 }, { 55, 60, 64 }, { 55, 59, 62 } };
        for (int bar = 0; bar < 4; ++bar) for (int n : ch[bar]) note (bar * 4, 3.8, n);
        return e;
    }
    const int riff[] = { 69, 72, 76, 74, 72, 69, 67, 69,  65, 69, 72, 71, 69, 67, 64, 67 };
    for (int k = 0; k < 16; ++k)
        note (k * 1.0 + (k % 4 == 3 ? 0.5 : 0.0), category == catPluck || category == catPizz ? 0.3 : 0.75, riff[k] - (category == catLead || category == catFlute ? 0 : 0));
    return e;
}

// afinación: busca el pico cerca de la nota esperada (Goertzel, ±80 cents)
static float detectCents (const juce::AudioBuffer<float>& b, float expectedHz, int from, int len)
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
    for (float c = -80; c <= 80; c += 1.0f)
    {
        const double pw = power (expectedHz * std::exp2 (c / 1200.0f));
        if (pw > bp) { bp = pw; best = c; }
    }
    return best;
}

static void stats (const juce::AudioBuffer<float>& b, float& peak, float& rmsDb, bool& finite)
{
    peak = 0; finite = true; double s = 0;
    for (int ch = 0; ch < 2; ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float v = b.getSample (ch, i);
            if (! std::isfinite (v)) finite = false;
            peak = std::max (peak, std::abs (v));
            s += v * v;
        }
    rmsDb = dsp::gainToDb ((float) std::sqrt (s / (2.0 * b.getNumSamples())));
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI* gui = nullptr; (void) gui;
    setLang (Lang::es);
    const juce::String cmd = argc > 1 ? argv[1] : "help";
    const auto cwd = juce::File::getCurrentWorkingDirectory();

    if (cmd == "render" && argc > 3)   // render <índice> <archivo.wav> [frase|note|chord]
    {
        const int idx = juce::String (argv[2]).getIntValue();
        Patch p; Macros m;
        buildCalibratedPreset (idx, p, m);
        const auto cat = presetRef (idx).category;
        double cpu = 0;
        auto b = renderEvents (p, m, pattern (argc > 4 ? argv[4] : "riff", cat), argc > 4 && juce::String (argv[4]) == "note" ? 3.5 : 11.0, &cpu);
        writeWav (cwd.getChildFile (argv[3]), b);
        float pk, rms; bool fin; stats (b, pk, rms, fin);
        std::cout << presetName (idx) << "  nivel " << p[gp::level] << " dB  pico " << pk << "  rms " << rms << "  cpu " << cpu << " ms" << std::endl;
        return 0;
    }

    if (cmd == "levels" && argc > 2)   // levels <archivo.inc>: mide los 8,000 presets y escribe la tabla de niveles
    {
        juce::String out;
        out << "// Generado por: DmbLab levels Source/presets/Levels.inc  (no editar a mano)\n";
        out << "// Cuánto hay que subir o bajar (dB) cada preset de fábrica para que todos suenen parejo.\n";
        out << "static const float factoryLevels[" << numFactoryPresets << "] = {\n";
        const auto t0 = juce::Time::getMillisecondCounterHiRes();
        for (int i = 0; i < numFactoryPresets; ++i)
        {
            Patch p; Macros m;
            buildPreset (i, p, m);
            const float lv = measureLevelCorrection (p, m, presetRef (i).category);
            out << juce::String (lv, 1) << "f" << (i + 1 < numFactoryPresets ? "," : "") << ((i % 20 == 19) ? "\n" : " ");
            if (i % 500 == 499) std::cout << "  " << (i + 1) << " (" << juce::String ((juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0, 0) << " s)" << std::endl;
        }
        out << "};\n";
        juce::File (juce::String (argv[2])).replaceWithText (out);
        return 0;
    }

    if (cmd == "check")   // check [categoría]: afinación, nivel y estabilidad de los sonidos base
    {
        const int onlyCat = argc > 2 ? juce::String (argv[2]).getIntValue() : -1;
        for (int c = 0; c < numCategories; ++c)
        {
            if (onlyCat >= 0 && c != onlyCat) continue;
            const auto& a = archetypes (c);
            for (int k = 0; k < (int) a.size(); ++k)
            {
                Patch p; Macros m;
                buildCalibratedPreset (presetIndex (c, k, 0), p, m);
                Macros dry = m; dry.espacio = 0; dry.eco = 0; dry.vibrato = 0; dry.caracter = charClean; dry.suciedad = 0;
                Patch pd = p; pd[gp::chMix] = 0;
                const int n = c == catBass ? 36 : 60;
                auto b = renderEvents (pd, dry, { { 0.0, n, 0.85f, true }, { 1.6, n, 0.0f, false } }, 2.0);
                const float offs = 12.0f * (float) (dry.octava + p.layer[0].i (lp::octave)) + p.layer[0][lp::semi];
                const float cents = c == catFx ? 0.0f : detectCents (b, dsp::noteToHz ((float) n + offs), (int) (0.12 * SR), (int) (0.4 * SR));
                float pk, rms; bool fin; stats (b, pk, rms, fin);
                double cpu = 0;
                auto chord = renderEvents (p, m, { { 0.0, n, 0.8f, true }, { 0.0, n + 4, 0.8f, true }, { 0.0, n + 7, 0.8f, true }, { 0.0, n + 12, 0.8f, true },
                                                   { 1.5, n, 0, false }, { 1.5, n + 4, 0, false }, { 1.5, n + 7, 0, false }, { 1.5, n + 12, 0, false } }, 2.0, &cpu);
                float pk2, rms2; bool fin2; stats (chord, pk2, rms2, fin2);
                std::cout << juce::String (categoryName (c)).paddedRight (' ', 10) << juce::String (a[(size_t) k].name).paddedRight (' ', 28)
                          << " afin " << juce::String (cents, 0).paddedLeft (' ', 4) << "c  nivel " << juce::String (p[gp::level], 1).paddedLeft (' ', 5)
                          << "  pico " << juce::String (pk2, 2) << "  cpu4 " << juce::String (cpu / 20.0, 1) << "%"
                          << ((fin && fin2) ? "" : "  NO FINITO") << (std::abs (cents) > 8 ? "  DESAFINADO" : "") << std::endl;
            }
        }
        return 0;
    }

    if (cmd == "demo" && argc > 3)   // demo <categoría> <archivo.wav>: los 20 sonidos base seguidos
    {
        const int c = juce::String (argv[2]).getIntValue();
        juce::AudioBuffer<float> all (2, 0);
        const auto& a = archetypes (c);
        for (int k = 0; k < (int) a.size(); ++k)
        {
            Patch p; Macros m;
            buildCalibratedPreset (presetIndex (c, k, 0), p, m);
            auto ev = pattern ("riff", c);
            std::vector<Ev> part;
            for (auto& x : ev) if (x.t < 60.0 / 95.0 * 8) part.push_back (x);
            if (c == catPad || c == catStrings || c == catChoir || c == catOrgan) { part.clear(); for (auto& x : ev) if (x.t < 60.0 / 95.0 * 8) part.push_back (x); }
            auto b = renderEvents (p, m, part, 60.0 / 95.0 * 8 + 1.2);
            b.applyGainRamp ((int) (b.getNumSamples() - SR * 0.3), (int) (SR * 0.3), 1.0f, 0.0f);
            const int old = all.getNumSamples();
            all.setSize (2, old + b.getNumSamples(), true, true);
            for (int ch = 0; ch < 2; ++ch) all.copyFrom (ch, old, b, ch, 0, b.getNumSamples());
            std::cout << k << " " << a[(size_t) k].name << std::endl;
        }
        writeWav (cwd.getChildFile (argv[3]), all);
        return 0;
    }

    if (cmd == "showcase" && argc > 2)   // showcase <archivo.wav>: canción corta + recorrido por los 16 tipos
    {
        const double bpm = 95.0, beat = 60.0 / bpm, s16 = beat / 4.0, bar = beat * 4.0;
        const int roots[4] = { 33, 29, 36, 31 };                          // Am F C G
        const int chords[4][3] = { { 57, 60, 64 }, { 53, 57, 60 }, { 55, 60, 64 }, { 55, 59, 62 } };
        struct N { int pos, len, note; };
        const std::vector<N> riff[4] = {
            { { 0, 2, 76 }, { 3, 1, 76 }, { 4, 2, 74 }, { 6, 2, 72 }, { 8, 3, 71 }, { 11, 1, 72 }, { 12, 4, 69 } },
            { { 0, 2, 72 }, { 3, 1, 72 }, { 4, 2, 74 }, { 6, 2, 76 }, { 8, 3, 77 }, { 11, 1, 76 }, { 12, 4, 74 } },
            { { 0, 2, 76 }, { 3, 1, 76 }, { 4, 2, 79 }, { 6, 2, 76 }, { 8, 3, 74 }, { 11, 1, 72 }, { 12, 4, 72 } },
            { { 0, 2, 71 }, { 3, 1, 72 }, { 4, 2, 74 }, { 6, 2, 71 }, { 8, 3, 67 }, { 11, 1, 69 }, { 12, 4, 71 } } };

        auto findPreset = [] (int c, const char* name)
        {
            const auto& a = archetypes (c);
            for (int k = 0; k < (int) a.size(); ++k)
                if (juce::String::fromUTF8 (a[(size_t) k].name) == juce::String::fromUTF8 (name)) return presetIndex (c, k, 0);
            std::cout << "no encontré " << name << std::endl;
            return presetIndex (c, 0, 0);
        };
        auto add = [] (std::vector<Ev>& e, double t, double len, int n, float v = 0.85f)
        {
            e.push_back ({ t, n, v, true });
            e.push_back ({ t + len, n, 0.0f, false });
        };
        // partes musicales (bar0 = compás donde empieza, bars = cuántos)
        auto bassPart = [&] (int bar0, int bars, int oct = 0)
        {
            std::vector<Ev> e;
            for (int b = 0; b < bars; ++b)
                for (int p : { 0, 3, 6, 8, 11, 14 })
                    add (e, (bar0 + b) * bar + p * s16, s16 * (p == 0 || p == 8 ? 2.6 : 1.6), roots[b % 4] + 12 + oct * 12);
            return e;
        };
        auto riffPart = [&] (int bar0, int bars, int transpose = 0)
        {
            std::vector<Ev> e;
            for (int b = 0; b < bars; ++b)
                for (auto& n : riff[b % 4]) add (e, (bar0 + b) * bar + n.pos * s16, n.len * s16 * 0.9, n.note + transpose);
            return e;
        };
        auto chordPart = [&] (int bar0, int bars, int transpose = 0, double len = 3.9)
        {
            std::vector<Ev> e;
            for (int b = 0; b < bars; ++b)
                for (int n : chords[b % 4]) add (e, (bar0 + b) * bar, beat * len, n + transpose, 0.75f);
            return e;
        };
        auto arpPart = [&] (int bar0, int bars, int transpose = 12)
        {
            std::vector<Ev> e;
            const int order[] = { 0, 1, 2, 1, 0, 2, 1, 2, 0, 1 };
            const int pos[] = { 0, 2, 3, 5, 6, 8, 10, 11, 13, 14 };
            for (int b = 0; b < bars; ++b)
                for (int k = 0; k < 10; ++k) add (e, (bar0 + b) * bar + pos[k] * s16, s16 * 1.3, chords[b % 4][order[k]] + transpose, k % 3 == 0 ? 0.9f : 0.7f);
            return e;
        };
        auto stabPart = [&] (int bar0, int bars)
        {
            std::vector<Ev> e;
            for (int b = 0; b < bars; ++b)
                for (int p : { 0, 3, 6 })
                    for (int n : chords[b % 4]) add (e, (bar0 + b) * bar + p * s16, s16 * 1.5, n, 0.9f);
            return e;
        };

        const int totalBars = 20 + 16 * 2 + 1;
        const int total = (int) (SR * (totalBars * bar + 2.0));
        juce::AudioBuffer<float> mix (2, total);
        mix.clear();
        auto layer = [&] (int category, const char* name, std::vector<Ev> ev, float gainDb, int bar0, int bars)
        {
            Patch p; Macros m;
            buildCalibratedPreset (findPreset (category, name), p, m);
            const double start = bar0 * bar, len = bars * bar + 1.8;
            for (auto& x : ev) x.t -= start;
            auto b = renderEvents (p, m, ev, len);
            const float g = juce::Decibels::decibelsToGain (gainDb);
            const int off = (int) (start * SR);
            for (int ch = 0; ch < 2; ++ch)
                mix.addFrom (ch, off, b, ch, 0, std::min (b.getNumSamples(), total - off), g);
        };

        // batería sintetizada (solo para el demo; el plugin no trae batería)
        auto drums = [&] (int bar0, int bars)
        {
            juce::Random rnd (7);
            auto hit = [&] (double t, int kind, float vel)
            {
                const int at = (int) (t * SR);
                const int len = (int) (SR * (kind == 0 ? 0.35 : kind == 1 ? 0.16 : 0.05));
                double ph = 0, lp = 0, hp = 0, prev = 0;
                for (int i = 0; i < len && at + i < total; ++i)
                {
                    const double tt = i / SR;
                    double v = 0;
                    if (kind == 0)        // bombo
                    {
                        const double f = 48.0 + 110.0 * std::exp (-tt * 38.0);
                        ph += f / SR;
                        v = std::sin (2.0 * juce::MathConstants<double>::pi * ph) * std::exp (-tt * 9.0) * 0.9;
                        v += (i < 60 ? (rnd.nextFloat() * 2 - 1) * 0.25 * (1.0 - i / 60.0) : 0.0);
                    }
                    else if (kind == 1)   // caja / rim del dembow
                    {
                        const double n = rnd.nextFloat() * 2 - 1;
                        lp += 0.45 * (n - lp);
                        ph += 210.0 / SR;
                        v = (lp * 0.7 * std::exp (-tt * 26.0) + std::sin (2.0 * juce::MathConstants<double>::pi * ph) * 0.45 * std::exp (-tt * 40.0)) * 0.8;
                    }
                    else                  // hi-hat
                    {
                        const double n = rnd.nextFloat() * 2 - 1;
                        hp = 0.6 * (hp + n - prev); prev = n;
                        v = hp * 0.35 * std::exp (-tt * 70.0);
                    }
                    v *= vel;
                    mix.addSample (0, at + i, (float) v * (kind == 2 ? 0.8f : 1.0f));
                    mix.addSample (1, at + i, (float) v * (kind == 2 ? 1.0f : 1.0f));
                }
            };
            for (int b = 0; b < bars; ++b)
            {
                const double t0 = (bar0 + b) * bar;
                for (int p : { 0, 4, 8, 12 }) hit (t0 + p * s16, 0, 0.9f);
                for (int p : { 3, 6, 11, 14 }) hit (t0 + p * s16, 1, p == 6 || p == 14 ? 0.85f : 0.7f);
                for (int p = 0; p < 16; p += 2) hit (t0 + p * s16, 2, p % 4 == 2 ? 0.8f : 0.45f);
            }
        };

        // ---- canción (20 compases) ----
        layer (catHit, "Orch Hit Clásico", { { 0.0, 57, 0.9f, true }, { 0.3, 57, 0, false }, { 0.0, 64, 0.9f, true }, { 0.3, 64, 0, false } }, -3, 0, 2);
        layer (catFx, "Riser Ruido", { { 0.0, 60, 0.8f, true }, { bar * 1.9, 60, 0, false } }, -8, 0, 2);
        layer (catPad, "Pad Marquesina", chordPart (0, 2), -6, 0, 2);
        // A: lead + bajo + pad
        drums (2, 16);
        layer (catBass, "Bajo Dembow", bassPart (2, 16), -2, 2, 16);
        layer (catLead, "Lead Marquesina", riffPart (2, 4), -3, 2, 4);
        layer (catPad, "Pad Noche", chordPart (2, 4), -9, 2, 4);
        // B: pluck + piano + campana
        layer (catPluck, "Pluck Dembow", arpPart (6, 4), -5, 6, 4);
        layer (catPiano, "Piano Romántico", chordPart (6, 4, 0, 1.8), -7, 6, 4);
        layer (catBells, "Campana Iglesia", { { 6 * bar, 81, 0.7f, true }, { 6 * bar + 0.5, 81, 0, false }, { 8 * bar, 79, 0.7f, true }, { 8 * bar + 0.5, 79, 0, false } }, -10, 6, 4);
        // C: orch hits + cuerdas + flauta
        layer (catHit, "Orch Hit Menor", stabPart (10, 4), -7, 10, 4);
        layer (catStrings, "Cuerdas Orquesta", chordPart (10, 4), -9, 10, 4);
        layer (catFlute, "Flauta Andina", riffPart (10, 4), -4, 10, 4);
        // D: marimba + coro + guitarra + metales
        layer (catMallets, "Marimba Tropical", riffPart (14, 4), -4, 14, 4);
        layer (catChoir, "Coro Aah", chordPart (14, 4), -9, 14, 4);
        layer (catGuitar, "Nylon Romántica", arpPart (14, 4, 0), -8, 14, 4);
        layer (catBrass, "Metales Stab", stabPart (17, 1), -6, 17, 1);
        layer (catFx, "Sirena Policía", { { 17.5 * bar, 72, 0.8f, true }, { 18 * bar + 1.0, 72, 0, false } }, -12, 17, 2);

        // ---- recorrido: 2 compases por tipo, con el sonido base nº 1 de cada uno ----
        int b0 = 19;
        std::cout << "recorrido (desde " << juce::String (b0 * bar, 1) << " s):" << std::endl;
        drums (b0, 32);
        for (int c = 0; c < numCategories; ++c, b0 += 2)
        {
            const char* nm = archetypes (c)[0].name;
            std::cout << "  " << juce::String (b0 * bar, 1) << " s  " << categoryName (c) << " · " << juce::String::fromUTF8 (nm) << std::endl;
            std::vector<Ev> ev;
            switch (c)
            {
                case catBass: ev = bassPart (b0, 2); break;
                case catPad: case catOrgan: case catStrings: case catChoir: case catPiano: ev = chordPart (b0, 2); break;
                case catPluck: case catPizz: ev = arpPart (b0, 2); break;
                case catHit: case catBrass: ev = stabPart (b0, 2); break;
                case catFx: add (ev, b0 * bar, bar * 1.5, 60); break;
                default: ev = riffPart (b0, 2); break;
            }
            layer (c, nm, ev, -3, b0, 2);
            if (c != catBass) layer (catBass, "Bajo Dembow", bassPart (b0, 2), -6, b0, 2);
        }

        float pk = mix.getMagnitude (0, total);
        mix.applyGain (0.89f / std::max (0.01f, pk));
        mix.applyGainRamp (total - (int) SR, (int) SR, 1.0f, 0.0f);
        writeWav (cwd.getChildFile (argv[2]), mix);
        std::cout << "pico antes de normalizar " << pk << std::endl;
        return 0;
    }

    if (cmd == "custom" && argc > 2)   // pruebas del motor con patches escritos aquí
    {
        juce::AudioBuffer<float> all (2, 0);
        auto add = [&] (const char* label, Patch p, Macros m, int cat, const char* pat = "riff")
        {
            p.clampAll();
            p[gp::level] = measureLevelCorrection (p, m, cat);
            auto ev = pattern (pat, cat);
            std::vector<Ev> part;
            for (auto& x : ev) if (x.t < 60.0 / 95.0 * 8) part.push_back (x);
            Patch dry = p; Macros md = m; md.espacio = 0; md.eco = 0; md.vibrato = 0; dry[gp::chMix] = 0;
            const int n = cat == catBass ? 36 : 60;
            auto t = renderEvents (dry, md, { { 0.0, n, 0.85f, true }, { 1.6, n, 0.0f, false } }, 2.0);
            writeWav (cwd.getChildFile (juce::String ("note_") + juce::String (label).replaceCharacter (' ', '_') + ".wav"), t);
            const float cents = detectCents (t, dsp::noteToHz ((float) n + 12 * m.octava), (int) (0.12 * SR), (int) (0.4 * SR));
            double cpu = 0;
            auto b = renderEvents (p, m, part, 60.0 / 95.0 * 8 + 1.2, &cpu);
            float pk, rms; bool fin; stats (b, pk, rms, fin);
            std::cout << juce::String (label).paddedRight (' ', 22) << " afin " << cents << "c  nivel " << p[gp::level] << "  pico " << pk
                      << "  cpu " << juce::String (cpu / ((60.0 / 95.0 * 8 + 1.2) * 10.0), 2) << "%" << (fin ? "" : " NO FINITO") << std::endl;
            const int old = all.getNumSamples();
            all.setSize (2, old + b.getNumSamples(), true, true);
            for (int ch = 0; ch < 2; ++ch) all.copyFrom (ch, old, b, ch, 0, b.getNumSamples());
        };

        { Patch p; Macros m; va (A (p), waveSaw, 5, 18, 0.7f); filt (p, filterLP24, 3000, 0.2f, 1.5f); fenv (p, 0.001f, 0.5f, 0.4f, 0.3f); amp (p, 0.002f, 1, 1, 0.2f);
          macros (m, 15, 25, charClean, 0, playMono, 12, 20); add ("supersaw lead", p, m, catLead); }
        { Patch p; Macros m; va (A (p), waveSquare, 1); osc2 (A (p), waveSquare, 12, 5, 0.4f); filt (p, filterLP24, 900, 0.3f, 3.5f); fenv (p, 0.001f, 0.18f, 0.0f, 0.2f);
          amp (p, 0.001f, 0.35f, 0.0f, 0.2f); macros (m, 15, 25); add ("square pluck", p, m, catPluck); }
        { Patch p; Macros m; fm (A (p), 0, 1, 1, 1.8f, 0.6f, 0.15f); fm2 (A (p), 1, 14, 1.2f, 0.35f, 0.25f); filt (p, filterOff, 20000); amp (p, 0.001f, 4, 1, 0.4f);
          chorus (p, 0.35f); macros (m, 15, 0); add ("fm epiano", p, m, catPiano, "chord"); }
        { Patch p; Macros m; pluck (A (p), 0.45f, 0.18f, 0.55f, 2.5f, 0.6f); filt (p, filterOff, 20000); amp (p, 0.001f, 5, 1, 0.25f);
          macros (m, 18, 0); add ("nylon", p, m, catGuitar); }
        { Patch p; Macros m; modal (A (p), modalPiano, 6, 0.5f, 0.5f, 0.5f, 0.4f); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 0.5f);
          macros (m, 18, 0); add ("piano", p, m, catPiano, "chord"); }
        { Patch p; Macros m; modal (A (p), modalMarimba, 1.2f, 0.5f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 4, 1, 0.3f);
          macros (m, 15, 0); add ("marimba", p, m, catMallets); }
        { Patch p; Macros m; modal (A (p), modalBell, 5, 0.6f, 0.6f); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 2);
          macros (m, 25, 0); add ("bell", p, m, catBells); }
        { Patch p; Macros m; modal (A (p), modalOrgan, 1, 0.5f, 0.5f, 0.5f, 0, 0, 0.5f); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.08f);
          chorus (p, 0.5f, 5.5f, 0.3f); macros (m, 15, 0); add ("organ", p, m, catOrgan, "chord"); }
        { Patch p; Macros m; modal (A (p), modalFlute, 1, 0.5f, 0.5f, 0.5f, 0, 1); filt (p, filterOff, 20000); amp (p, 0.04f, 1, 1, 0.12f);
          noise (p, 0.12f, 0.35f, 3.0f, 0.5f, 0.06f); macros (m, 20, 15, charClean, 0, playPoly, 0, 35); add ("pan flute", p, m, catFlute); }
        { Patch p; Macros m; vox (A (p), 2, 0.0f, 0.12f, 4); filt (p, filterLP12, 6000); amp (p, 0.25f, 1, 1, 0.6f);
          macros (m, 35, 0); add ("choir aah", p, m, catChoir, "chord"); }
        { Patch p; Macros m; va (A (p), waveSine, 1); sub (A (p), 0.3f); filt (p, filterLP12, 400, 0.0f); amp (p, 0.002f, 0.8f, 0.6f, 0.1f);
          pitchEnv (p, 7, 0.03f); macros (m, 0, 0, charClean, 0, playMono, 8); add ("sine bass", p, m, catBass); }
        writeWav (cwd.getChildFile (argv[2]), all);
        return 0;
    }

    std::cout << "uso: render <idx> <wav> [note|chord|riff] | check [cat] | demo <cat> <wav> | custom <wav>" << std::endl;
    return 0;
}
