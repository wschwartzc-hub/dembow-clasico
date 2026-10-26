#include "Factory.h"
#include "../synth/Synth.h"
#include "../I18n.h"

#if __has_include ("Levels.inc") && ! defined (DMB_LAB)
 #include "Levels.inc"
 #define DMB_HAVE_LEVEL_TABLE 1
#else
 #define DMB_HAVE_LEVEL_TABLE 0
#endif

namespace dmb
{
namespace
{
    struct Tables
    {
        std::vector<Archetype> cats[numCategories];
        Tables()
        {
            addLeadsPlucksBassPads (cats);
            addKeysOrganBellsMallets (cats);
            addOrchestra (cats);
            addGuitarFluteChoirFx (cats);
        }
    };

    const Tables& tables() { static const Tables t; return t; }

    const char* categoryNames[numCategories][3] = {
        { "Lead", "Lead", "lead" }, { "Pluck", "Pluck", "pluck" }, { "Bajo", "Bass", "bass" }, { "Pad", "Pad", "pad" },
        { "Piano", "Piano", "piano" }, { "Órgano", "Organ", "organ" }, { "Campanas", "Bells", "bells" }, { "Marimba", "Mallets", "mallets" },
        { "Cuerdas", "Strings", "strings" }, { "Pizzicato", "Pizzicato", "pizz" }, { "Metales", "Brass", "brass" }, { "Orch hit", "Orch hit", "hit" },
        { "Guitarra", "Guitar", "guitar" }, { "Flauta", "Flute", "flute" }, { "Coros", "Choir", "choir" }, { "FX", "FX", "fx" },
    };

    const char* variationNames[variationsPerArchetype][2] = {
        { "Original", "Original" }, { "Brillante", "Bright" }, { "Oscuro", "Dark" }, { "Vintage", "Vintage" },
        { "Vintage sucio", "Dirty vintage" }, { "Cálido", "Warm" }, { "Lo-fi", "Lo-fi" }, { "Casete", "Cassette" },
        { "Con eco", "With echo" }, { "Eco largo", "Long echo" }, { "En sala", "In a room" }, { "Lejano", "Distant" },
        { "Ancho", "Wide" }, { "Fino", "Thin" }, { "Corto", "Short" }, { "Largo", "Long" },
        { "Suave", "Soft" }, { "Duro", "Hard" }, { "Octava arriba", "Octave up" }, { "Octava abajo", "Octave down" },
        { "Con vibrato", "With vibrato" }, { "Callejero", "Street" }, { "Discoteca", "Club" }, { "Noche", "Night" },
        { "2004", "2004" },
    };

    // algunas variaciones cambian según el tipo (un bajo "lejano" no sirve; mejor "redondo")
    struct Swap { int category, variation; const char* es; const char* en; };
    const Swap swaps[] = {
        { catBass, 11, "Redondo", "Round" },  { catBass, 19, "Sub", "Sub" },     { catBass, 20, "Con glide", "With glide" },
        { catBass, 12, "Gordo", "Fat" },
        { catHit, 20, "Con caída", "Falling" }, { catHit, 15, "Con cola", "With tail" },
        { catPad, 14, "Respira", "Breathing" },
        { catFx, 20, "Con movimiento", "Moving" },
        { catPizz, 15, "Sostenido", "Held" },
    };

    const Swap* findSwap (int cat, int var)
    {
        for (auto& s : swaps) if (s.category == cat && s.variation == var) return &s;
        return nullptr;
    }

    float jit (juce::Random& r, float amount) { return std::exp2 ((r.nextFloat() * 2.0f - 1.0f) * amount); }

    // variaciones pequeñas en la receta para que ningún preset sea idéntico a otro
    void jitter (Patch& p, juce::Random& r, float k)
    {
        p[gp::fCut] *= jit (r, 0.18f * k);
        for (auto id : { gp::aA, gp::aD, gp::aR, gp::feA, gp::feD, gp::feR })
            p[id] *= jit (r, 0.15f * k);
        p[gp::fRes] = juce::jlimit (0.0f, 1.0f, p[gp::fRes] + (r.nextFloat() - 0.5f) * 0.06f * k);
        p[gp::chRate] *= jit (r, 0.3f * k);
        p[gp::vibRate] *= jit (r, 0.1f * k);
        for (auto& l : p.layer)
        {
            l[lp::uniDetune] *= jit (r, 0.25f * k);
            l[lp::fmI2] *= jit (r, 0.12f * k);
            l[lp::fmI4] *= jit (r, 0.12f * k);
            l[lp::fmDec] *= jit (r, 0.15f * k);
            l[lp::plDecay] *= jit (r, 0.15f * k);
            l[lp::plBright] = juce::jlimit (0.0f, 1.0f, l[lp::plBright] + (r.nextFloat() - 0.5f) * 0.08f * k);
            l[lp::moDecay] *= jit (r, 0.15f * k);
            l[lp::moHard] = juce::jlimit (0.0f, 1.0f, l[lp::moHard] + (r.nextFloat() - 0.5f) * 0.1f * k);
            l[lp::vxVowel] = juce::jlimit (0.0f, 4.0f, l[lp::vxVowel] + (r.nextFloat() - 0.5f) * 0.25f * k);
            l[lp::pw] = juce::jlimit (0.08f, 0.92f, l[lp::pw] + (r.nextFloat() - 0.5f) * 0.06f * k);
        }
    }

    void addTo (float& v, float d, float lo = 0.0f, float hi = 100.0f) { v = juce::jlimit (lo, hi, v + d); }

    void applyVariation (int cat, int var, Patch& p, Macros& m)
    {
        auto vintage = [&] (float s) { m.caracter = charVintage; m.suciedad = s; };
        switch (var)
        {
            case 1:  addTo (m.brillo, 18); break;
            case 2:  addTo (m.brillo, -20); break;
            case 3:  vintage (45); break;
            case 4:  vintage (82); addTo (m.brillo, -4); break;
            case 5:  m.caracter = charWarm; m.suciedad = 50; break;
            case 6:  m.caracter = charLofi; m.suciedad = 45; break;
            case 7:  m.caracter = charLofi; m.suciedad = 80; addTo (m.brillo, -8); break;
            case 8:  m.eco = std::max (m.eco, 28.0f); p[gp::dlDiv] = 5; break;
            case 9:  m.eco = std::max (m.eco, 40.0f); p[gp::dlDiv] = 7; p[gp::dlFb] = std::min (0.6f, p[gp::dlFb] + 0.15f); break;
            case 10: addTo (m.espacio, 22); p[gp::rvSize] = 0.35f; p[gp::rvDecay] = 1.4f; break;
            case 11:
                if (cat == catBass) { addTo (m.brillo, -25); m.fuerza = 30; }
                else { addTo (m.espacio, 42); addTo (m.brillo, -12); p[gp::rvSize] = 0.85f; p[gp::rvDecay] = std::max (p[gp::rvDecay], 3.5f); }
                break;
            case 12: addTo (m.grosor, cat == catBass ? 18.0f : 28.0f); break;
            case 13: addTo (m.grosor, -25); addTo (m.brillo, 6); break;
            case 14:
                if (cat == catPad) { p[gp::l2Amp] = 0.35f; p[gp::l2Rate] = 0.25f; p[gp::l2Wave] = 0; }
                else addTo (m.duracion, -22);
                break;
            case 15:
                if (cat == catHit) { addTo (m.espacio, 30); p[gp::rvDecay] = std::max (p[gp::rvDecay], 3.0f); addTo (m.duracion, 15); }
                else if (cat == catPizz) addTo (m.duracion, 35);
                else addTo (m.duracion, 22);
                break;
            case 16: addTo (m.ataque, 18); addTo (m.brillo, -8); m.fuerza = std::max (m.fuerza, 70.0f); break;
            case 17: addTo (m.ataque, -25); addTo (m.brillo, 10); m.suciedad = std::max (m.suciedad, 18.0f); break;
            case 18: m.octava = std::min (2, m.octava + 1); break;
            case 19:
                if (cat == catBass) p.layer[0][lp::sub] = std::min (1.0f, p.layer[0][lp::sub] + 0.6f);
                else m.octava = std::max (-2, m.octava - 1);
                break;
            case 20:
                if (cat == catBass) { m.modo = playLegato; m.glide = std::max (m.glide, 32.0f); }
                else if (cat == catHit) { p[gp::pAmt] = -12.0f; p[gp::pTime] = 0.45f; }
                else if (cat == catFx) { p[gp::l2Cut] = std::max (p[gp::l2Cut], 1.5f); p[gp::l2Rate] = 0.4f; }
                else m.vibrato = std::max (m.vibrato, 42.0f);
                break;
            case 21: m.caracter = charLofi; m.suciedad = 60; addTo (m.eco, 22); addTo (m.brillo, -6); break;
            case 22: addTo (m.brillo, 12); addTo (m.eco, 22); addTo (m.espacio, 25); addTo (m.grosor, 15); p[gp::dlDiv] = 5; break;
            case 23: addTo (m.brillo, -18); addTo (m.espacio, 40); addTo (m.eco, 18); addTo (m.duracion, 10); p[gp::rvDecay] = std::max (p[gp::rvDecay], 3.0f); break;
            case 24: vintage (60); addTo (m.eco, 18); addTo (m.espacio, 18); p[gp::dlDiv] = 5; break;
            default: break;
        }
    }

    float categoryTargetDb (int cat)
    {
        static const float t[numCategories] = { -15.0f, -15.0f, -14.0f, -17.0f, -15.5f, -17.0f, -16.5f, -15.5f,
                                                -17.0f, -15.5f, -15.0f, -14.0f, -15.5f, -15.5f, -17.0f, -16.0f };
        return t[juce::jlimit (0, numCategories - 1, cat)];
    }

    bool isChordCategory (int cat)
    {
        return cat == catPad || cat == catPiano || cat == catOrgan || cat == catStrings || cat == catChoir || cat == catBrass;
    }
}

// =============================================================================================
const std::vector<Archetype>& archetypes (int category)
{
    return tables().cats[juce::jlimit (0, numCategories - 1, category)];
}

PresetRef presetRef (int index)
{
    index = juce::jlimit (0, numFactoryPresets - 1, index);
    PresetRef r;
    r.category = index / presetsPerCategory;
    const int inCat = index % presetsPerCategory;
    r.archetype = inCat / variationsPerArchetype;
    r.variation = inCat % variationsPerArchetype;
    return r;
}

int presetIndex (int category, int archetype, int variation)
{
    return category * presetsPerCategory + archetype * variationsPerArchetype + variation;
}

juce::String categoryName (int category)
{
    const auto& n = categoryNames[juce::jlimit (0, numCategories - 1, category)];
    return T (n[0], n[1]);
}

juce::String categoryKey (int category)
{
    return categoryNames[juce::jlimit (0, numCategories - 1, category)][2];
}

juce::String variationName (int category, int variation)
{
    variation = juce::jlimit (0, variationsPerArchetype - 1, variation);
    if (auto* s = findSwap (category, variation)) return T (s->es, s->en);
    return T (variationNames[variation][0], variationNames[variation][1]);
}

juce::String presetName (int index)
{
    const auto r = presetRef (index);
    const auto& a = archetypes (r.category);
    juce::String name = r.archetype < (int) a.size() ? U (a[(size_t) r.archetype].name) : juce::String ("?");
    if (r.variation > 0) name << U (" · ") << variationName (r.category, r.variation);
    return name;
}

juce::String presetDescription (int index)
{
    const auto r = presetRef (index);
    const auto& a = archetypes (r.category);
    if (r.archetype >= (int) a.size()) return {};
    return T (a[(size_t) r.archetype].descEs, a[(size_t) r.archetype].descEn);
}

void buildPreset (int index, Patch& p, Macros& m)
{
    const auto r = presetRef (index);
    p = Patch();
    m = Macros();
    const auto& a = archetypes (r.category);
    if (r.archetype < (int) a.size())
        a[(size_t) r.archetype].build (p, m);
    if (r.variation > 0)
    {
        juce::Random rnd ((juce::int64) index * 7919 + 17);
        jitter (p, rnd, 1.0f);
        applyVariation (r.category, r.variation, p, m);
    }
    p.clampAll();
}

float measureLevelCorrection (const Patch& patchIn, const Macros& macrosIn, int category)
{
    constexpr double sr = 48000.0;
    Synth s;
    s.prepare (sr, 512);
    Patch p = patchIn;
    p[gp::level] = 0.0f;
    Macros m = macrosIn;
    m.volumen = 0.0f;
    s.setPatch (p);
    s.setMacros (m);
    s.setTempo (95.0);

    const int base = category == catBass ? 36 : 60;
    if (isChordCategory (category)) { s.noteOn (base, 0.8f); s.noteOn (base + 4, 0.8f); s.noteOn (base + 7, 0.8f); }
    else s.noteOn (base, 0.8f);

    // volumen "momentáneo": el RMS más alto en ventanas de 100 ms
    // si el sonido entra despacio, se mide hasta que termina de entrar
    const double seconds = juce::jlimit (0.65, 5.0, (double) p[gp::aA] * 1.3 + 0.4);
    const int total = (int) (sr * seconds), block = 240, win = 4800;
    std::vector<float> L ((size_t) block), R ((size_t) block);
    double winSum = 0.0, best = 0.0;
    std::vector<double> ring ((size_t) (win / block), 0.0);
    size_t ri = 0;
    float peak = 0.0f;
    for (int pos = 0; pos < total; pos += block)
    {
        s.render (L.data(), R.data(), block);
        double bs = 0.0;
        for (int i = 0; i < block; ++i)
        {
            bs += 0.5 * ((double) L[(size_t) i] * L[(size_t) i] + (double) R[(size_t) i] * R[(size_t) i]);
            peak = std::max (peak, std::max (std::abs (L[(size_t) i]), std::abs (R[(size_t) i])));
        }
        winSum += bs - ring[ri];
        ring[ri] = bs;
        ri = (ri + 1) % ring.size();
        if (pos + block >= win) best = std::max (best, winSum / win);
    }
    const float rmsDb = dsp::gainToDb ((float) std::sqrt (std::max (best, 1.0e-18)));
    if (rmsDb < -90.0f) return 0.0f;
    float corr = categoryTargetDb (category) - rmsDb;
    // que una nota (o el acorde de prueba) no pase de -5 dBFS
    corr = std::min (corr, -5.0f - dsp::gainToDb (peak));
    return juce::jlimit (-30.0f, 30.0f, corr);
}

void buildCalibratedPreset (int index, Patch& p, Macros& m)
{
    buildPreset (index, p, m);
   #if DMB_HAVE_LEVEL_TABLE
    p[gp::level] = juce::jlimit (-36.0f, 30.0f, factoryLevels[juce::jlimit (0, numFactoryPresets - 1, index)]);
   #else
    p[gp::level] = juce::jlimit (-36.0f, 30.0f, measureLevelCorrection (p, m, presetRef (index).category));
   #endif
}

void randomPreset (int category, uint32_t seed, Patch& p, Macros& m, juce::String& name)
{
    juce::Random r ((juce::int64) seed);
    category = juce::jlimit (0, numCategories - 1, category);
    const int arch = r.nextInt ((int) archetypes (category).size());
    const int var = r.nextInt (variationsPerArchetype);
    buildPreset (presetIndex (category, arch, var), p, m);
    jitter (p, r, 2.0f);
    m.brillo = juce::jlimit (15.0f, 85.0f, m.brillo + (r.nextFloat() - 0.5f) * 40.0f);
    m.duracion = juce::jlimit (15.0f, 85.0f, m.duracion + (r.nextFloat() - 0.5f) * 40.0f);
    m.grosor = juce::jlimit (15.0f, 85.0f, m.grosor + (r.nextFloat() - 0.5f) * 40.0f);
    if (r.nextFloat() < 0.35f) { m.caracter = r.nextInt (numCharacters); m.suciedad = 20.0f + r.nextFloat() * 60.0f; }
    if (r.nextFloat() < 0.4f) { m.eco = r.nextFloat() * 40.0f; p[gp::dlDiv] = (float) (r.nextBool() ? 5 : 3); }
    m.espacio = juce::jlimit (0.0f, 80.0f, m.espacio + (r.nextFloat() - 0.3f) * 30.0f);
    p.clampAll();
    p[gp::level] = measureLevelCorrection (p, m, category);
    name = U (archetypes (category)[(size_t) arch].name) + U (" · ") + T ("Dado", "Dice") + " " + juce::String ((int) (seed % 1000));
}
} // namespace dmb
