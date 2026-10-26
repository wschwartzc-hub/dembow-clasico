#pragma once

// Un "patch" es la receta completa del sonido (lo que el usuario no ve).
// Los controles grandes de la interfaz (BRILLO, DURACIÓN…) son "macros" que modifican el patch.
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <array>

namespace dmb
{
enum class Engine : int { off = 0, analog, fm, pluck, modal, vox, numEngines };

enum Wave : int { waveSaw = 0, waveSquare, wavePulse, waveTri, waveSine, waveNoise, numWaves };

enum ModalModel : int
{
    modalPiano = 0, modalMarimba, modalVibes, modalXylo, modalGlock, modalBell, modalTubular, modalMusicBox,
    modalKalimba, modalSteelPan, modalCelesta, modalOrgan, modalFlute, modalGlass, numModalModels
};

enum FilterType : int { filterOff = 0, filterLP12, filterLP24, filterBP, filterHP, numFilterTypes };

enum Character : int { charClean = 0, charVintage, charWarm, charLofi, numCharacters };

enum PlayMode : int { playPoly = 0, playMono, playLegato };

// ---------------------------------------------------------------------------------------------
// Parámetros de cada capa (cada sonido tiene 2 capas: A y B)
#define DMB_LAYER_PARAMS(X) \
    X (engine,     "eng",   0.0f,   5.0f,   0.0f,  true)  \
    X (level,      "lvl",   0.0f,   1.0f,   1.0f,  false) \
    X (octave,     "oct",  -3.0f,   3.0f,   0.0f,  true)  \
    X (semi,       "semi", -24.0f, 24.0f,   0.0f,  false) \
    X (fine,       "fine", -100.0f, 100.0f, 0.0f,  false) \
    X (pan,        "pan",  -1.0f,   1.0f,   0.0f,  false) \
    X (wave1,      "w1",    0.0f,   5.0f,   0.0f,  true)  \
    X (wave2,      "w2",    0.0f,   5.0f,   0.0f,  true)  \
    X (pw,         "pw",    0.05f,  0.95f,  0.5f,  false) \
    X (mix2,       "mix2",  0.0f,   1.0f,   0.0f,  false) \
    X (semi2,      "semi2", -24.0f, 24.0f,  0.0f,  false) \
    X (fine2,      "fine2", -100.0f, 100.0f, 0.0f, false) \
    X (sub,        "sub",   0.0f,   1.0f,   0.0f,  false) \
    X (unison,     "uni",   1.0f,   7.0f,   1.0f,  true)  \
    X (uniDetune,  "unid",  0.0f, 100.0f,  12.0f,  false) \
    X (uniWidth,   "uniw",  0.0f,   1.0f,   0.6f,  false) \
    X (fmAlgo,     "fma",   0.0f,   2.0f,   0.0f,  true)  \
    X (fmR1,       "fmr1",  0.25f, 16.0f,   1.0f,  false) \
    X (fmR2,       "fmr2",  0.25f, 24.0f,   1.0f,  false) \
    X (fmR3,       "fmr3",  0.25f, 16.0f,   1.0f,  false) \
    X (fmR4,       "fmr4",  0.25f, 24.0f,   1.0f,  false) \
    X (fmI2,       "fmi2",  0.0f,  12.0f,   1.0f,  false) \
    X (fmI4,       "fmi4",  0.0f,  12.0f,   0.0f,  false) \
    X (fmDec,      "fmd",   0.005f, 10.0f,  0.5f,  false) \
    X (fmSus,      "fms",   0.0f,   1.0f,   0.3f,  false) \
    X (fmFb,       "fmfb",  0.0f,   1.0f,   0.0f,  false) \
    X (fmMix3,     "fmm3",  0.0f,   1.0f,   0.0f,  false) \
    X (fmDec3,     "fmd3",  0.0f,  10.0f,   0.0f,  false) \
    X (plBright,   "plb",   0.0f,   1.0f,   0.6f,  false) \
    X (plPos,      "plp",   0.02f,  0.5f,   0.18f, false) \
    X (plDamp,     "pld",   0.0f,   1.0f,   0.6f,  false) \
    X (plDecay,    "plt",   0.05f, 20.0f,   3.0f,  false) \
    X (plBody,     "plbo",  0.0f,   1.0f,   0.3f,  false) \
    X (moModel,    "mom",   0.0f,  13.0f,   0.0f,  true)  \
    X (moStretch,  "most",  0.0f,   1.0f,   0.3f,  false) \
    X (moHard,     "moh",   0.0f,   1.0f,   0.5f,  false) \
    X (moDecay,    "mot",   0.05f, 30.0f,   2.0f,  false) \
    X (moTilt,     "moti",  0.0f,   1.0f,   0.5f,  false) \
    X (moBright,   "mob",   0.0f,   1.0f,   0.5f,  false) \
    X (moReg,      "mor",   0.0f,   7.0f,   0.0f,  true)  \
    X (moPerc,     "mop",   0.0f,   1.0f,   0.0f,  false) \
    X (vxVoice,    "vxv",   0.0f,   3.0f,   1.0f,  true)  \
    X (vxVowel,    "vxw",   0.0f,   4.0f,   0.0f,  false) \
    X (vxBreath,   "vxb",   0.0f,   1.0f,   0.15f, false) \
    X (vxShift,    "vxs",  -1.0f,   1.0f,   0.0f,  false) \
    X (vxEnsemble, "vxe",   1.0f,   4.0f,   3.0f,  true)

// Parámetros generales del sonido
#define DMB_GLOBAL_PARAMS(X) \
    X (fType,     "ft",    0.0f,    4.0f,     2.0f,  true)  \
    X (fCut,      "fc",   20.0f, 20000.0f, 8000.0f,  false) \
    X (fRes,      "fr",    0.0f,    1.0f,     0.1f,  false) \
    X (fKey,      "fk",    0.0f,    1.0f,     0.5f,  false) \
    X (fEnv,      "fe",   -4.0f,    8.0f,     0.0f,  false) \
    X (fDrive,    "fdr",   0.0f,    1.0f,     0.0f,  false) \
    X (fVel,      "fv",    0.0f,    1.0f,     0.3f,  false) \
    X (feA,       "fea",   0.0f,   10.0f,   0.001f,  false) \
    X (feD,       "fed",   0.001f, 20.0f,     0.5f,  false) \
    X (feS,       "fes",   0.0f,    1.0f,     0.0f,  false) \
    X (feR,       "fer",   0.001f, 20.0f,     0.5f,  false) \
    X (aA,        "aa",    0.0f,   10.0f,   0.002f,  false) \
    X (aD,        "ad",    0.001f, 30.0f,     1.0f,  false) \
    X (aS,        "as",    0.0f,    1.0f,     1.0f,  false) \
    X (aR,        "ar",    0.001f, 20.0f,     0.3f,  false) \
    X (pAmt,      "pa",  -48.0f,   48.0f,     0.0f,  false) \
    X (pTime,     "pt",    0.001f,  5.0f,     0.1f,  false) \
    X (vibRate,   "vr",    0.1f,   12.0f,     5.2f,  false) \
    X (vibDelay,  "vd",    0.0f,    3.0f,     0.3f,  false) \
    X (l2Rate,    "lr",    0.02f,  20.0f,     1.0f,  false) \
    X (l2Wave,    "lw",    0.0f,    3.0f,     0.0f,  true)  \
    X (l2Cut,     "lc",    0.0f,    4.0f,     0.0f,  false) \
    X (l2Amp,     "la",    0.0f,    1.0f,     0.0f,  false) \
    X (l2PW,      "lp",    0.0f,    0.45f,    0.0f,  false) \
    X (l2Pan,     "lpn",   0.0f,    1.0f,     0.0f,  false) \
    X (l2Pitch,   "lpt",   0.0f,   24.0f,     0.0f,  false) \
    X (l2Sync,    "ls",    0.0f,    1.0f,     0.0f,  true)  \
    X (nLevel,    "nl",    0.0f,    1.0f,     0.0f,  false) \
    X (nColor,    "nc",    0.0f,    1.0f,     0.5f,  false) \
    X (nQ,        "nq",    0.3f,   20.0f,     2.0f,  false) \
    X (nChiff,    "nch",   0.0f,    1.0f,     0.0f,  false) \
    X (nChiffT,   "ncht",  0.002f,  1.0f,     0.05f, false) \
    X (velAmp,    "va",    0.0f,    1.0f,     0.6f,  false) \
    X (bend,      "bend",  1.0f,   24.0f,     2.0f,  true)  \
    X (chRate,    "chr",   0.05f,   5.0f,     0.6f,  false) \
    X (chDepth,   "chd",   0.0f,    1.0f,     0.4f,  false) \
    X (chMix,     "chm",   0.0f,    1.0f,     0.0f,  false) \
    X (dlDiv,     "dld",   0.0f,    9.0f,     5.0f,  true)  \
    X (dlFb,      "dlf",   0.0f,    0.9f,     0.35f, false) \
    X (dlTone,    "dlt",   0.0f,    1.0f,     0.5f,  false) \
    X (dlPing,    "dlp",   0.0f,    1.0f,     1.0f,  true)  \
    X (rvSize,    "rvs",   0.0f,    1.0f,     0.5f,  false) \
    X (rvDecay,   "rvd",   0.2f,   12.0f,     2.2f,  false) \
    X (rvTone,    "rvt",   0.0f,    1.0f,     0.5f,  false) \
    X (eqLow,     "eql",  20.0f, 1000.0f,    20.0f,  false) \
    X (eqHigh,    "eqh", 1000.0f, 20000.0f, 20000.0f, false) \
    X (level,     "glvl", -36.0f,  30.0f,     0.0f,  false)

namespace lp
{
    enum Id : int
    {
#define DMB_X(name, id, mn, mx, def, isInt) name,
        DMB_LAYER_PARAMS (DMB_X)
#undef DMB_X
        count
    };
}

namespace gp
{
    enum Id : int
    {
#define DMB_X(name, id, mn, mx, def, isInt) name,
        DMB_GLOBAL_PARAMS (DMB_X)
#undef DMB_X
        count
    };
}

struct ParamInfo { const char* id; float min, max, def; bool isInt; };
const ParamInfo& layerInfo (int index);
const ParamInfo& globalInfo (int index);

// ---------------------------------------------------------------------------------------------
struct Layer
{
    std::array<float, lp::count> v {};
    Layer();
    float& operator[] (lp::Id i)       { return v[(size_t) i]; }
    float  operator[] (lp::Id i) const { return v[(size_t) i]; }
    int    i (lp::Id id) const         { return (int) std::lround (v[(size_t) id]); }
    Engine engine() const              { return (Engine) i (lp::engine); }
};

struct Patch
{
    std::array<Layer, 2> layer;
    std::array<float, gp::count> g {};

    Patch();
    float& operator[] (gp::Id i)       { return g[(size_t) i]; }
    float  operator[] (gp::Id i) const { return g[(size_t) i]; }
    int    i (gp::Id id) const         { return (int) std::lround (g[(size_t) id]); }

    void clampAll();
    juce::ValueTree toTree() const;
    static Patch fromTree (const juce::ValueTree&);
};

// ---------------------------------------------------------------------------------------------
// Macros: lo que el usuario mueve. Los relativos (50 = como viene el sonido) y los absolutos.
struct Macros
{
    float brillo = 50, duracion = 50, ataque = 50, grosor = 50;   // relativos
    float suciedad = 0, vibrato = 0, glide = 0, fuerza = 60;     // absolutos 0..100
    float espacio = 15, eco = 0, volumen = 0;                    // volumen en dB
    int caracter = charClean, modo = playPoly, octava = 0;
};

// divisiones del eco (índice del parámetro dlDiv) → fracción de negra
float delayDivisionBeats (int index);
juce::String delayDivisionName (int index);
} // namespace dmb
