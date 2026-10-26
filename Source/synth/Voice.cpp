#include "Voice.h"

namespace dmb
{
using namespace dsp;

namespace
{
    constexpr float invTwoPi = 0.15915494f;
    constexpr float voiceGain = 0.3f;

    // ------------------------------------------------------------------------------------------
    // Instrumentos de percusión afinada: proporción, nivel y duración relativa de cada parcial.
    struct ModalTable
    {
        int n;
        float r[10], a[10], d[10];
        float mallet, malletRatio, malletTime;
    };

    const ModalTable& modalTable (int model)
    {
        static const ModalTable none { 1, { 1 }, { 1 }, { 1 }, 0.0f, 2.0f, 0.01f };
        static const ModalTable t[] = {
            // piano (se calcula aparte)
            none,
            // marimba: barras afinadas 1 : 4 : 10
            { 4, { 1.0f, 3.98f, 9.2f, 15.5f }, { 1.0f, 0.32f, 0.09f, 0.03f }, { 1.0f, 0.38f, 0.18f, 0.1f }, 0.45f, 2.0f, 0.012f },
            // vibráfono
            { 3, { 1.0f, 4.0f, 10.0f }, { 1.0f, 0.28f, 0.07f }, { 1.0f, 0.55f, 0.3f }, 0.22f, 3.0f, 0.008f },
            // xilófono 1 : 3 : 6
            { 4, { 1.0f, 3.0f, 6.0f, 9.95f }, { 1.0f, 0.55f, 0.25f, 0.12f }, { 1.0f, 0.6f, 0.45f, 0.3f }, 0.55f, 4.0f, 0.006f },
            // glockenspiel (barra libre)
            { 4, { 1.0f, 2.76f, 5.40f, 8.93f }, { 1.0f, 0.45f, 0.25f, 0.12f }, { 1.0f, 0.55f, 0.35f, 0.25f }, 0.3f, 6.0f, 0.004f },
            // campana de iglesia (afinada al "nominal")
            { 9, { 0.25f, 0.5f, 0.595f, 0.75f, 1.0f, 1.25f, 1.335f, 1.5f, 2.0f },
                 { 0.06f, 0.28f, 0.5f, 0.2f, 1.0f, 0.35f, 0.28f, 0.25f, 0.2f },
                 { 2.2f, 1.6f, 1.3f, 0.9f, 1.0f, 0.55f, 0.45f, 0.4f, 0.3f }, 0.25f, 3.0f, 0.005f },
            // campana tubular
            { 6, { 1.0f, 2.0f, 3.0f, 4.2f, 5.4f, 6.8f }, { 1.0f, 0.55f, 0.45f, 0.3f, 0.2f, 0.12f },
                 { 1.0f, 0.85f, 0.7f, 0.5f, 0.4f, 0.3f }, 0.35f, 5.0f, 0.005f },
            // caja de música
            { 4, { 1.0f, 2.0f, 6.27f, 17.55f }, { 1.0f, 0.12f, 0.3f, 0.08f }, { 1.0f, 0.7f, 0.35f, 0.15f }, 0.3f, 8.0f, 0.003f },
            // kalimba
            { 3, { 1.0f, 5.85f, 15.2f }, { 1.0f, 0.25f, 0.07f }, { 1.0f, 0.3f, 0.15f }, 0.45f, 3.0f, 0.008f },
            // tambor metálico (steel pan)
            { 6, { 1.0f, 2.003f, 2.997f, 4.01f, 5.03f, 6.0f }, { 1.0f, 0.75f, 0.45f, 0.3f, 0.15f, 0.08f },
                 { 1.0f, 0.85f, 0.65f, 0.5f, 0.4f, 0.3f }, 0.18f, 2.0f, 0.01f },
            // celesta
            { 4, { 1.0f, 2.0f, 3.01f, 4.1f }, { 1.0f, 0.22f, 0.06f, 0.1f }, { 1.0f, 0.6f, 0.5f, 0.3f }, 0.2f, 6.0f, 0.004f },
            none,   // órgano (se calcula aparte)
            none,   // flauta (se calcula aparte)
            // cristal
            { 4, { 1.0f, 2.32f, 4.25f, 6.63f }, { 1.0f, 0.4f, 0.2f, 0.1f }, { 1.0f, 0.7f, 0.5f, 0.4f }, 0.12f, 5.0f, 0.004f },
        };
        return t[juce::jlimit (0, (int) numModalModels - 1, model)];
    }

    // registros del órgano (barras: 16' 5⅓' 8' 4' 2⅔' 2' 1⅗' 1⅓' 1')
    const float drawbarRatio[9] = { 0.5f, 1.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 8.0f };
    const int registrations[8][9] = {
        { 8, 8, 8, 0, 0, 0, 0, 0, 0 },   // clásico
        { 8, 8, 8, 8, 8, 8, 8, 8, 8 },   // lleno
        { 0, 0, 8, 0, 0, 0, 0, 0, 0 },   // suave
        { 8, 8, 8, 8, 0, 0, 0, 0, 0 },   // iglesia
        { 8, 8, 8, 6, 0, 0, 0, 0, 0 },   // rock
        { 0, 0, 8, 0, 8, 0, 8, 0, 0 },   // hueco
        { 8, 6, 8, 0, 0, 0, 0, 0, 0 },   // cine
        { 0, 0, 8, 8, 0, 4, 0, 0, 4 },   // acordeón / musette
    };

    // espectros de las flautas (armónicos 1..6)
    const float fluteSpectra[8][6] = {
        { 1.0f, 0.35f, 0.12f, 0.06f, 0.03f, 0.015f },   // flauta
        { 1.0f, 0.10f, 0.22f, 0.04f, 0.03f, 0.01f },    // zampoña
        { 1.0f, 0.06f, 0.04f, 0.01f, 0.0f, 0.0f },      // ocarina
        { 1.0f, 0.22f, 0.06f, 0.02f, 0.0f, 0.0f },      // piccolo
        { 1.0f, 0.015f, 0.005f, 0.0f, 0.0f, 0.0f },     // silbido
        { 1.0f, 0.45f, 0.22f, 0.10f, 0.05f, 0.02f },    // flauta dulce
        { 1.0f, 0.40f, 0.25f, 0.12f, 0.06f, 0.03f },    // quena
        { 1.0f, 0.02f, 0.45f, 0.02f, 0.22f, 0.02f },    // clarinete
    };

    // formantes (F1..F4: frecuencia, ancho de banda, dB) para bajo, tenor, alto, soprano × A E I O U
    struct Formant { float f, bw, db; };
    const Formant formants[4][5][4] = {
        { // bajo
            { { 600, 60, 0 }, { 1040, 70, -7 }, { 2250, 110, -9 }, { 2450, 120, -9 } },
            { { 400, 40, 0 }, { 1620, 80, -12 }, { 2400, 100, -9 }, { 2800, 120, -12 } },
            { { 250, 60, 0 }, { 1750, 90, -30 }, { 2600, 100, -16 }, { 3050, 120, -22 } },
            { { 400, 40, 0 }, { 750, 80, -11 }, { 2400, 100, -21 }, { 2600, 120, -20 } },
            { { 350, 40, 0 }, { 600, 80, -20 }, { 2400, 100, -32 }, { 2675, 120, -28 } } },
        { // tenor
            { { 650, 80, 0 }, { 1080, 90, -6 }, { 2650, 120, -7 }, { 2900, 130, -8 } },
            { { 400, 70, 0 }, { 1700, 80, -14 }, { 2600, 100, -12 }, { 3200, 120, -14 } },
            { { 290, 40, 0 }, { 1870, 90, -15 }, { 2800, 100, -18 }, { 3250, 120, -20 } },
            { { 400, 40, 0 }, { 800, 80, -10 }, { 2600, 100, -12 }, { 2800, 120, -12 } },
            { { 350, 40, 0 }, { 600, 60, -20 }, { 2700, 100, -17 }, { 2900, 120, -14 } } },
        { // alto
            { { 800, 80, 0 }, { 1150, 90, -4 }, { 2800, 120, -20 }, { 3500, 130, -36 } },
            { { 400, 60, 0 }, { 1600, 80, -24 }, { 2700, 120, -30 }, { 3300, 150, -35 } },
            { { 350, 50, 0 }, { 1700, 100, -20 }, { 2700, 120, -30 }, { 3700, 150, -36 } },
            { { 450, 70, 0 }, { 800, 80, -9 }, { 2830, 100, -16 }, { 3500, 130, -28 } },
            { { 325, 50, 0 }, { 700, 60, -12 }, { 2530, 170, -30 }, { 3500, 180, -40 } } },
        { // soprano
            { { 800, 80, 0 }, { 1150, 90, -6 }, { 2900, 120, -32 }, { 3900, 130, -20 } },
            { { 350, 60, 0 }, { 2000, 100, -20 }, { 2800, 120, -15 }, { 3600, 150, -40 } },
            { { 270, 60, 0 }, { 2140, 90, -12 }, { 2950, 100, -26 }, { 3900, 120, -26 } },
            { { 450, 70, 0 }, { 800, 80, -11 }, { 2830, 100, -22 }, { 3800, 130, -22 } },
            { { 325, 50, 0 }, { 700, 60, -16 }, { 2700, 170, -35 }, { 3800, 180, -40 } } },
    };

    inline float oscWave (int wave, float ph, float inc, float pw, Rng& rng)
    {
        switch (wave)
        {
            case waveSaw:    return 2.0f * ph - 1.0f - polyBlep (ph, inc);
            case waveSquare:
            {
                float v = ph < 0.5f ? 1.0f : -1.0f;
                v += polyBlep (ph, inc);
                float t2 = ph + 0.5f; if (t2 >= 1.0f) t2 -= 1.0f;
                return v - polyBlep (t2, inc);
            }
            case wavePulse:
            {
                float v = ph < pw ? 1.0f : -1.0f;
                v += polyBlep (ph, inc);
                float t2 = ph + (1.0f - pw); if (t2 >= 1.0f) t2 -= 1.0f;
                v -= polyBlep (t2, inc);
                return v - (2.0f * pw - 1.0f);
            }
            case waveTri:    return ph < 0.5f ? 4.0f * ph - 1.0f : 3.0f - 4.0f * ph;
            case waveSine:   return sine01 (ph);
            case waveNoise:  return rng.bipolar() * 1.7f;
            default:         return 0.0f;
        }
    }

    inline void advance (float& ph, float inc) { ph += inc; if (ph >= 1.0f) ph -= 1.0f; }

    inline void panGains (float pan, float& l, float& r)
    {
        pan = clamp (pan, -1.0f, 1.0f);
        const float a = (pan + 1.0f) * 0.25f * pi;
        l = std::cos (a) * 1.41421356f * 0.7071f;
        r = std::sin (a) * 1.41421356f * 0.7071f;
    }
}

// =============================================================================================
void Voice::prepare (float sampleRate)
{
    sr = sampleRate;
    for (auto& l : layers)
        l.line.resize ((int) (sr / 18.0f) + 8);
    reset();
}

void Voice::reset()
{
    active = killing = false;
    amp.kill();
    fenv.kill();
    note = -1;
    keyDown = sustained = false;
}

void Voice::noteOn (int n, float vel, float startPitch, const VoiceParams& vp, uint32_t seed)
{
    const auto& P = *vp.patch;
    rng.seed (seed * 2654435761u + 12345u);
    note = n;
    velocity = juce::jlimit (0.0f, 1.0f, vel);
    keyDown = true;
    sustained = false;
    killing = false;
    killGain = 1.0f;
    active = true;
    timeSinceOn = 0.0f;
    controlCounter = 0;

    pitchTarget = (float) n;
    pitchNow = vp.glideTime > 0.0005f ? startPitch : pitchTarget;

    // velocidad
    const float sens = vp.velSens;
    velGain = 1.0f - sens * (1.0f - velocity) * 0.85f;
    velCut = P[gp::fVel] * sens * 3.0f * (velocity - 0.75f);
    velBright = sens * (velocity - 0.75f) * 0.6f;

    // envolventes
    const float atk = juce::jlimit (0.0005f, 10.0f, P[gp::aA] * vp.attackMul + vp.attackAdd);
    amp.set (atk, P[gp::aD] * vp.decayMul, P[gp::aS], P[gp::aR] * vp.decayMul, sr);
    amp.value = 0.0f;
    amp.noteOn();
    fenv.set (juce::jlimit (0.0005f, 10.0f, P[gp::feA] * vp.attackMul + vp.attackAdd * 0.5f),
              P[gp::feD] * vp.decayMul, P[gp::feS], P[gp::feR] * vp.decayMul, sr);
    fenv.value = 0.0f;
    fenv.noteOn();

    pitchEnv = P[gp::pAmt];
    pitchEnvMul = std::exp (-(float) controlRate / std::max (1.0f, P[gp::pTime] * sr));

    vibPhase = 0.0f;
    lfoPhase = P.i (gp::l2Sync) ? 0.0f : rng.unipolar();
    shValue = rng.bipolar();

    // filtro
    filterType = P.i (gp::fType);
    driveK = P[gp::fDrive] * 6.0f;
    driveNorm = 1.0f / (1.0f + P[gp::fDrive] * 1.5f);
    for (auto* f : { &filtL1, &filtL2, &filtR1, &filtR2 }) f->reset();

    // ruido / soplido
    noiseLevel = P[gp::nLevel];
    chiff = P[gp::nChiff];
    chiffMul = std::exp (-1.0f / std::max (1.0f, P[gp::nChiffT] * sr * 0.35f));
    noiseBp.reset();

    for (int li = 0; li < 2; ++li)
        startLayer (li, vp, true);

    updateControl (vp, 0);
}

void Voice::legatoTo (int n, float vel)
{
    note = n;
    velocity = vel;
    keyDown = true;
    sustained = false;
    pitchTarget = (float) n;
}

void Voice::retrigger (int n, float vel, const VoiceParams& vp)
{
    // vuelve a empezar las envolventes desde donde van (sin click)
    const float from = pitchNow, level = amp.value;
    noteOn (n, vel, from, vp, (uint32_t) n * 7919u + (uint32_t) order);
    amp.value = level;
}

void Voice::noteOff()
{
    keyDown = false;
    amp.noteOff();
    fenv.noteOff();
}

void Voice::fastKill()
{
    if (! active) return;
    killing = true;
    killStep = 1.0f / std::max (1.0f, 0.005f * sr);
}

// ---------------------------------------------------------------------------------------------
void Voice::startLayer (int index, const VoiceParams& vp, bool resetPhases)
{
    auto& S = layers[index];
    const auto& L = vp.patch->layer[(size_t) index];
    S.eng = L.engine();
    if (S.eng == Engine::off || L[lp::level] <= 0.0001f)
    {
        S.eng = Engine::off;
        return;
    }

    S.gain = L[lp::level];
    S.pitchOffset = 12.0f * (float) L.i (lp::octave) + L[lp::semi] + L[lp::fine] * 0.01f;
    panGains (L[lp::pan], S.panL, S.panR);

    const float f0 = noteToHz (pitchTarget + vp.transpose + S.pitchOffset);

    switch (S.eng)
    {
        case Engine::analog:
        {
            S.uni = juce::jlimit (1, 7, L.i (lp::unison));
            const float width = clamp (L[lp::uniWidth] + vp.widthAdd, 0.0f, 1.0f);
            for (int u = 0; u < S.uni; ++u)
            {
                const float x = S.uni == 1 ? 0.0f : 2.0f * (float) u / (float) (S.uni - 1) - 1.0f;
                const float jitter = S.uni == 1 ? 0.0f : rng.bipolar() * 0.12f;
                S.det[u] = (x + jitter * std::abs (x));   // en unidades de "detune"
                panGains (x * width, S.gl[u], S.gr[u]);
                if (resetPhases) S.ph[u] = S.uni == 1 ? 0.0f : rng.unipolar();
            }
            S.uniNorm = 1.0f / std::sqrt ((float) S.uni);
            if (resetPhases) { S.ph2 = rng.unipolar() * 0.25f; S.phSub = 0.0f; }
            break;
        }
        case Engine::fm:
            if (resetPhases) { S.p1 = S.p2 = S.p3 = S.p4 = 0.0f; S.m2a = S.m2b = 0.0f; }
            S.idxEnv = 1.0f;
            S.idxMul = std::exp (-(float) 1.0f / std::max (1.0f, L[lp::fmDec] * vp.decayMul * sr * 0.3f));
            S.car3 = 1.0f;
            S.car3Mul = L[lp::fmDec3] > 0.0f ? std::exp (-6.9f / std::max (1.0f, L[lp::fmDec3] * vp.decayMul * sr)) : 1.0f;
            break;
        case Engine::pluck:
            setupPluck (S, L, f0, vp);
            break;
        case Engine::modal:
            setupModal (S, L, f0, vp);
            break;
        case Engine::vox:
            setupVox (S, L, vp);
            break;
        default: break;
    }
}

void Voice::setupPluck (LayerState& S, const Layer& L, float f0, const VoiceParams& vp)
{
    const float bright = clamp (L[lp::plBright] + vp.bright * 0.35f + velBright, 0.0f, 1.0f);
    const float damp = clamp (L[lp::plDamp] + vp.bright * 0.3f + velBright * 0.5f, 0.0f, 1.0f);
    S.loopA = 0.03f + 0.72f * (1.0f - damp) * (1.0f - damp);
    S.line.clear();
    S.loopZ = 0.0f;
    S.dcZ = S.dcX = 0.0f;
    S.plT60 = clamp (L[lp::plDecay] * vp.decayMul, 0.03f, 60.0f);
    updatePluckTuning (S, f0);

    // la "púa": ruido filtrado según el brillo, con el filtro de peine de la posición
    const float period = sr / f0;
    const int len = juce::jlimit (4, S.line.mask - 8, (int) std::ceil (period) + 2);
    OnePole lpEx;
    lpEx.setLowpass (f0 * 1.5f + bright * bright * sr * 0.45f, sr);
    float exc[8192];
    const int n = std::min (len, 8192);
    float mean = 0.0f;
    for (int k = 0; k < n; ++k)
    {
        // un poco de impulso (ataque más definido) + ruido
        const float tri = 1.0f - std::abs (2.0f * (float) k / (float) n - 1.0f);
        exc[k] = lpEx.lp (rng.bipolar() * 0.85f + tri * 0.6f * (0.4f + bright));
        mean += exc[k];
    }
    mean /= (float) n;
    const int pos = juce::jlimit (1, n - 1, (int) (L[lp::plPos] * period));
    float peak = 1.0e-6f;
    for (int k = n - 1; k >= 0; --k)
    {
        exc[k] -= mean;
        if (k >= pos) exc[k] -= (exc[k - pos] - mean) * 0.9f;
        peak = std::max (peak, std::abs (exc[k]));
    }
    for (int k = 0; k < n; ++k)
        S.line.push (exc[k] / peak);

    // cuerpo (resonancias de caja)
    S.body1.set (110.0f, 3.0f, sr);
    S.body2.set (240.0f, 4.0f, sr);
    S.body1.reset();
    S.body2.reset();
}

void Voice::updatePluckTuning (LayerState& S, float f0)
{
    f0 = clamp (f0, 18.0f, sr * 0.3f);
    const float w = twoPi * f0 / sr;
    const float a = S.loopA;
    // retraso del filtro del lazo a esa frecuencia (para que quede afinada)
    const float phaseDelay = std::atan2 (a * std::sin (w), 1.0f - a * std::cos (w)) / w;
    S.loopDelay = clamp (sr / f0 - phaseDelay, 2.0f, (float) S.line.mask - 4.0f);
    const float mag = (1.0f - a) / std::sqrt (1.0f - 2.0f * a * std::cos (w) + a * a);
    // pérdida por vuelta para que la nota dure plT60 segundos (a -60 dB)
    S.loopG = std::min (0.99995f, std::pow (10.0f, -3.0f / (S.plT60 * f0)) / std::max (0.05f, mag));
}

void Voice::setupModal (LayerState& S, const Layer& L, float f0, const VoiceParams& vp)
{
    const int model = L.i (lp::moModel);
    const float bright = clamp (L[lp::moBright] + vp.bright * 0.35f + velBright, 0.0f, 1.0f);
    const float hard = clamp (L[lp::moHard] + vp.bright * 0.3f + velBright * 0.6f, 0.0f, 1.0f);
    const float tilt = L[lp::moTilt];
    const float keyDecay = std::pow (261.63f / clamp (f0, 20.0f, 8000.0f), 0.45f);
    const float T = clamp (L[lp::moDecay] * vp.decayMul * keyDecay, 0.03f, 60.0f);
    const float nyq = sr * 0.45f;
    const float rolloff = lerp (0.12f, 1.0f, bright);

    int n = 0;
    auto add = [&] (float ratio, float a, float t, float a2 = 0.0f, float t2 = 1.0f)
    {
        if (n >= maxPartials || ratio * f0 >= nyq || (a <= 1.0e-5f && a2 <= 1.0e-5f)) return;
        S.pratio[n] = ratio;
        S.pamp[n] = a;
        S.pamp2[n] = a2;
        S.pmul[n] = t > 0.0f ? std::exp (-6.9078f / std::max (1.0f, t * sr)) : 1.0f;
        S.pmul2[n] = t2 > 0.0f ? std::exp (-6.9078f / std::max (1.0f, t2 * sr)) : 1.0f;
        S.pph[n] = 0.0f;
        ++n;
    };

    float malletAmt = 0.0f, malletRatio = 2.0f, malletTime = 0.008f;
    const bool sustain = model == modalOrgan || model == modalFlute;

    if (model == modalPiano)
    {
        const float B = L[lp::moStretch] * 0.0012f * std::sqrt (clamp (f0 / 261.63f, 0.25f, 8.0f));
        const float p = lerp (2.1f, 0.85f, hard * 0.6f + bright * 0.4f);
        for (int k = 1; k <= 14; ++k)
        {
            const float r = (float) k * std::sqrt (1.0f + B * (float) (k * k));
            const float notch = 0.12f + std::abs (std::sin (pi * (float) k / 8.3f));
            const float a = notch / std::pow ((float) k, p);
            const float tk = T / (1.0f + tilt * 0.3f * (float) (k - 1) * (1.0f + 0.1f * (float) k));
            add (r, a * 0.62f, tk * 0.2f, a * 0.38f, tk);
            if (k <= 8)   // segunda cuerda, un poco desafinada (el "latido" del piano)
                add (r * 1.0013f, a * 0.45f, tk * 0.2f, a * 0.28f, tk * 0.9f);
        }
        malletAmt = 0.12f * (0.4f + hard);
        malletRatio = 1.6f;
        malletTime = 0.012f;
    }
    else if (model == modalOrgan)
    {
        const int reg = juce::jlimit (0, 7, L.i (lp::moReg));
        for (int k = 0; k < 9; ++k)
        {
            const int bar = registrations[reg][k];
            if (bar <= 0) continue;
            const float a = std::pow (10.0f, (float) (bar - 8) * 3.0f / 20.0f) * std::pow (rolloff, (float) k * 0.35f);
            add (drawbarRatio[k], a, 0.0f);
        }
        if (L[lp::moPerc] > 0.001f)
            add (3.0f, L[lp::moPerc] * 0.7f, 0.35f * vp.decayMul);   // percusión: el 3er armónico se apaga rápido
        for (int k = 0; k < n; ++k) S.pph[k] = rng.unipolar();
        malletAmt = 0.08f * hard;
        malletRatio = 8.0f;
        malletTime = 0.003f;
    }
    else if (model == modalFlute)
    {
        const int sp = juce::jlimit (0, 7, L.i (lp::moReg));
        for (int k = 0; k < 6; ++k)
            add ((float) (k + 1), fluteSpectra[sp][k] * std::pow (rolloff, (float) k * 0.8f), 0.0f);
    }
    else
    {
        const auto& t = modalTable (model);
        for (int k = 0; k < t.n; ++k)
        {
            float a = t.a[k];
            if (k > 0) a *= lerp (0.45f, 1.5f, hard) * std::pow (rolloff, (float) k * 0.7f);
            const float d = std::pow (t.d[k], 0.5f + tilt);
            // un poco de desorden en los parciales (menos el que da la nota)
            const float r = t.r[k] == 1.0f ? 1.0f : t.r[k] * (1.0f + L[lp::moStretch] * 0.02f * (float) k * (rng.bipolar() * 0.5f));
            add (r, a, T * d);
        }
        malletAmt = t.mallet * (0.35f + hard);
        malletRatio = t.malletRatio;
        malletTime = t.malletTime * lerp (1.6f, 0.6f, hard);
    }

    // normaliza: la suma de los parciales vale ~1
    float sum = 0.0f;
    for (int k = 0; k < n; ++k) sum += S.pamp[k] + S.pamp2[k];
    const float norm = sum > 0.0f ? 1.0f / sum : 1.0f;
    for (int k = 0; k < n; ++k) { S.pamp[k] *= norm; S.pamp2[k] *= norm; }
    S.nPart = n;

    S.mallet = malletAmt;
    (void) sustain;
    S.malletMul = std::exp (-1.0f / std::max (1.0f, malletTime * sr));
    S.malletBp.set (clamp (f0 * malletRatio, 120.0f, 9000.0f), 0.9f, sr);
    S.malletBp.reset();
}

void Voice::setupVox (LayerState& S, const Layer& L, const VoiceParams& vp)
{
    S.ens = juce::jlimit (1, 4, L.i (lp::vxEnsemble));
    const float spread = 9.0f * vp.detuneMul;
    const float width = clamp (0.7f + vp.widthAdd, 0.0f, 1.0f);
    for (int e = 0; e < S.ens; ++e)
    {
        const float x = S.ens == 1 ? 0.0f : 2.0f * (float) e / (float) (S.ens - 1) - 1.0f;
        S.vdet[e] = x * spread + rng.bipolar() * 2.0f;
        panGains (x * width, S.vgl[e], S.vgr[e]);
        S.vph[e] = rng.unipolar();
        S.drift[e] = 0.0f;
        S.driftTarget[e] = rng.bipolar() * 5.0f;
    }

    const int voice = juce::jlimit (0, 3, L.i (lp::vxVoice));
    const float vowel = clamp (L[lp::vxVowel], 0.0f, 4.0f);
    const int v0 = std::min (3, (int) vowel);
    const float t = vowel - (float) v0;
    const float shift = std::exp2 (clamp (L[lp::vxShift] + vp.bright * 0.15f, -1.0f, 1.0f) * 0.32f);
    for (int k = 0; k < 4; ++k)
    {
        const auto& a = formants[voice][v0][k];
        const auto& b = formants[voice][std::min (4, v0 + 1)][k];
        const float f = lerp (a.f, b.f, t) * shift;
        const float bw = lerp (a.bw, b.bw, t) * 1.3f;
        const float db = lerp (a.db, b.db, t);
        for (int c = 0; c < 2; ++c)
        {
            S.form[c][k].set (f, f / bw, sr);
            S.form[c][k].reset();
        }
        S.formGain[k] = dbToGain (db) * (k == 0 ? 1.0f : 1.4f);
    }
    S.glottalL.setLowpass (3200.0f, sr);
    S.glottalR.setLowpass (3200.0f, sr);
}

// ---------------------------------------------------------------------------------------------
void Voice::updateControl (const VoiceParams& vp, int blockLen)
{
    const auto& P = *vp.patch;
    const float dt = (float) blockLen / sr;
    timeSinceOn += dt;

    // glide
    if (vp.glideTime > 0.0005f)
    {
        const float c = 1.0f - std::exp (-(float) std::max (1, blockLen) / (vp.glideTime * sr * 0.33f));
        pitchNow += (pitchTarget - pitchNow) * c;
    }
    else
        pitchNow = pitchTarget;

    pitchEnv *= pitchEnvMul;

    // vibrato (entra poco a poco)
    vibPhase += P[gp::vibRate] * dt;
    vibPhase -= std::floor (vibPhase);
    const float fade = clamp ((timeSinceOn - P[gp::vibDelay]) / 0.45f, 0.0f, 1.0f);
    const float vib = vp.vibDepth * fade * sine01 (vibPhase);

    // LFO 2
    lfoPhase += P[gp::l2Rate] * dt;
    if (lfoPhase >= 1.0f) { lfoPhase -= std::floor (lfoPhase); shValue = rng.bipolar(); }
    switch (P.i (gp::l2Wave))
    {
        case 1:  lfoValue = lfoPhase < 0.5f ? 4.0f * lfoPhase - 1.0f : 3.0f - 4.0f * lfoPhase; break;
        case 2:  lfoValue = lfoPhase < 0.5f ? 1.0f : -1.0f; break;
        case 3:  lfoValue = shValue; break;
        default: lfoValue = sine01 (lfoPhase); break;
    }
    pwMod = P[gp::l2PW] * lfoValue;
    panMod = P[gp::l2Pan] * lfoValue;
    ampMod = 1.0f - P[gp::l2Amp] * (0.5f + 0.5f * lfoValue);

    const float pitch = pitchNow + vp.transpose + vp.bend + pitchEnv + vib + P[gp::l2Pitch] * lfoValue;

    for (int li = 0; li < 2; ++li)
    {
        auto& S = layers[li];
        if (S.eng == Engine::off) continue;
        const auto& L = P.layer[(size_t) li];
        const float f = noteToHz (pitch + S.pitchOffset);
        const float inc = f / sr;
        switch (S.eng)
        {
            case Engine::analog: S.inc1 = inc; break;
            case Engine::fm:     S.inc1 = inc; break;
            case Engine::pluck:  updatePluckTuning (S, f); break;
            case Engine::modal:
                for (int k = 0; k < S.nPart; ++k) S.pinc[k] = std::min (0.49f, inc * S.pratio[k]);
                break;
            case Engine::vox:
                S.inc1 = inc;
                for (int e = 0; e < S.ens; ++e)
                {
                    S.drift[e] += (S.driftTarget[e] - S.drift[e]) * std::min (1.0f, dt * 1.5f);
                    if (rng.unipolar() < dt * 0.8f) S.driftTarget[e] = rng.bipolar() * 5.0f;
                }
                break;
            default: break;
        }
        (void) L;
    }

    // filtro
    if (filterType != filterOff)
    {
        const float f0 = noteToHz (pitchNow + vp.transpose);
        const float key = std::pow (clamp (f0 / 261.63f, 0.05f, 20.0f), P[gp::fKey]);
        const float oct = P[gp::fEnv] * fenv.value + velCut + P[gp::l2Cut] * lfoValue;
        const float cut = clamp (P[gp::fCut] * vp.cutoffMul * key * std::exp2 (oct), 20.0f, sr * 0.45f);
        const float q = 0.5f * std::pow (24.0f, P[gp::fRes]);
        if (filterType == filterLP24)
        {
            filtL1.set (cut, 0.54f, sr); filtR1.set (cut, 0.54f, sr);
            filtL2.set (cut, std::max (0.6f, q), sr); filtR2.set (cut, std::max (0.6f, q), sr);
        }
        else
        {
            filtL1.set (cut, std::max (0.5f, q), sr);
            filtR1.set (cut, std::max (0.5f, q), sr);
        }
    }

    if (noiseLevel > 0.0f || chiff > 1.0e-4f)
    {
        const float f0 = noteToHz (pitchNow + vp.transpose);
        const float nf = clamp (f0 * std::exp2 (P[gp::nColor] * 5.0f - 1.0f), 60.0f, sr * 0.45f);
        noiseBp.set (nf, P[gp::nQ], sr);
    }
}

// ---------------------------------------------------------------------------------------------
inline void Voice::renderLayer (LayerState& S, const Layer& L, float& outL, float& outR)
{
    switch (S.eng)
    {
        case Engine::analog:
        {
            const int w1 = L.i (lp::wave1);
            const float pw = clamp (L[lp::pw] + pwMod, 0.05f, 0.95f);
            const float detCents = L[lp::uniDetune] * lastDetuneMul;
            float l = 0.0f, r = 0.0f;
            for (int u = 0; u < S.uni; ++u)
            {
                const float inc = S.inc1 * (S.uni == 1 ? 1.0f : std::exp2 (S.det[u] * detCents * (1.0f / 1200.0f)));
                advance (S.ph[u], inc);
                const float s = oscWave (w1, S.ph[u], inc, pw, rng);
                l += s * S.gl[u];
                r += s * S.gr[u];
            }
            l *= S.uniNorm;
            r *= S.uniNorm;
            float mono = 0.0f;
            const float m2 = L[lp::mix2];
            if (m2 > 0.0001f)
            {
                const float inc2 = S.inc1 * std::exp2 ((L[lp::semi2] + L[lp::fine2] * 0.01f) * (1.0f / 12.0f));
                advance (S.ph2, inc2);
                mono += m2 * oscWave (L.i (lp::wave2), S.ph2, inc2, pw, rng);
            }
            const float sub = L[lp::sub];
            if (sub > 0.0001f)
            {
                advance (S.phSub, S.inc1 * 0.5f);
                mono += sub * sine01 (S.phSub);
            }
            const float norm = 1.0f / (1.0f + 0.6f * m2 + 0.3f * sub);
            outL += (l + mono * 0.7071f) * norm * S.gain * S.panL * 1.414f;
            outR += (r + mono * 0.7071f) * norm * S.gain * S.panR * 1.414f;
            break;
        }

        case Engine::fm:
        {
            const float inc = S.inc1;
            const float idx = (L[lp::fmSus] + (1.0f - L[lp::fmSus]) * S.idxEnv) * lastFmIndexMul;
            S.idxEnv *= S.idxMul;
            const float fb = L[lp::fmFb] * 0.5f * (S.m2a + S.m2b) * 0.35f;
            advance (S.p1, inc * L[lp::fmR1]);
            advance (S.p2, inc * L[lp::fmR2]);
            advance (S.p4, inc * L[lp::fmR4]);
            const float m4 = sine01 (S.p4);
            float c1 = 0.0f;
            switch (L.i (lp::fmAlgo))
            {
                case 1:   // pila 4 → 2 → 1
                {
                    const float m2 = sine01 (S.p2 + fb + L[lp::fmI4] * idx * m4 * invTwoPi);
                    S.m2b = S.m2a; S.m2a = m2;
                    c1 = sine01 (S.p1 + L[lp::fmI2] * idx * m2 * invTwoPi);
                    break;
                }
                case 2:   // (2 + 4) → 1
                {
                    const float m2 = sine01 (S.p2 + fb);
                    S.m2b = S.m2a; S.m2a = m2;
                    c1 = sine01 (S.p1 + (L[lp::fmI2] * m2 + L[lp::fmI4] * m4) * idx * invTwoPi);
                    break;
                }
                default:  // dos pares: 2 → 1 y 4 → 3
                {
                    const float m2 = sine01 (S.p2 + fb);
                    S.m2b = S.m2a; S.m2a = m2;
                    c1 = sine01 (S.p1 + L[lp::fmI2] * idx * m2 * invTwoPi);
                    break;
                }
            }
            float out = c1;
            const float m3 = L[lp::fmMix3];
            if (m3 > 0.0001f)
            {
                advance (S.p3, inc * L[lp::fmR3]);
                const float mod = L.i (lp::fmAlgo) == 0 ? L[lp::fmI4] * idx * m4 * invTwoPi : 0.0f;
                out += m3 * S.car3 * sine01 (S.p3 + mod);
                S.car3 *= S.car3Mul;
                out *= 1.0f / (1.0f + 0.5f * m3);
            }
            outL += out * S.gain * S.panL;
            outR += out * S.gain * S.panR;
            break;
        }

        case Engine::pluck:
        {
            const float y = S.line.readCubic (S.loopDelay);
            S.loopZ += (1.0f - S.loopA) * (y - S.loopZ);
            S.line.push (S.loopZ * S.loopG);
            // quita la componente continua
            const float hp = S.loopZ - S.dcX + 0.995f * S.dcZ;
            S.dcX = S.loopZ;
            S.dcZ = hp;
            float out = hp;
            const float body = L[lp::plBody];
            if (body > 0.0001f)
                out += body * (S.body1.bpNorm (hp) * 0.9f + S.body2.bpNorm (hp) * 0.6f);
            out *= 3.2f;   // la cuerda sola es bajita: se compensa aquí
            outL += out * S.gain * S.panL;
            outR += out * S.gain * S.panR;
            break;
        }

        case Engine::modal:
        {
            float s = 0.0f;
            for (int k = 0; k < S.nPart; ++k)
            {
                s += sine01 (S.pph[k]) * (S.pamp[k] + S.pamp2[k]);
                S.pph[k] += S.pinc[k];
                if (S.pph[k] >= 1.0f) S.pph[k] -= 1.0f;
                S.pamp[k] *= S.pmul[k];
                S.pamp2[k] *= S.pmul2[k];
            }
            if (S.mallet > 1.0e-5f)
            {
                s += S.malletBp.bpNorm (rng.bipolar()) * S.mallet;
                S.mallet *= S.malletMul;
            }
            outL += s * S.gain * S.panL;
            outR += s * S.gain * S.panR;
            break;
        }

        case Engine::vox:
        {
            float l = 0.0f, r = 0.0f;
            for (int e = 0; e < S.ens; ++e)
            {
                const float inc = S.inc1 * std::exp2 ((S.vdet[e] * lastDetuneMul + S.drift[e]) * (1.0f / 1200.0f));
                advance (S.vph[e], inc);
                const float s = 2.0f * S.vph[e] - 1.0f - polyBlep (S.vph[e], inc);
                l += s * S.vgl[e];
                r += s * S.vgr[e];
            }
            const float breath = L[lp::vxBreath] * rng.bipolar() * 0.8f;
            const float norm = 1.0f / std::sqrt ((float) S.ens);
            l = S.glottalL.lp (l * norm) + breath;
            r = S.glottalR.lp (r * norm) + breath;
            float ol = 0.0f, orr = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                ol += S.form[0][k].bpNorm (l) * S.formGain[k];
                orr += S.form[1][k].bpNorm (r) * S.formGain[k];
            }
            outL += ol * 1.6f * S.gain * S.panL * 1.414f;
            outR += orr * 1.6f * S.gain * S.panR * 1.414f;
            break;
        }

        default: break;
    }
}

void Voice::render (float* outL, float* outR, int numSamples, const VoiceParams& vp)
{
    if (! active) return;
    const auto& P = *vp.patch;
    lastDetuneMul = vp.detuneMul;
    lastFmIndexMul = std::exp2 (vp.bright * 1.3f + velBright * 1.2f);

    int done = 0;
    while (done < numSamples)
    {
        if (controlCounter <= 0)
        {
            updateControl (vp, controlRate);
            controlCounter = controlRate;
            float pl, pr;
            panGains (panMod, pl, pr);
            panL = pl * 1.414f;
            panR = pr * 1.414f;
        }
        const int todo = std::min (controlCounter, numSamples - done);

        for (int i = 0; i < todo; ++i)
        {
            float l = 0.0f, r = 0.0f;
            for (int li = 0; li < 2; ++li)
                if (layers[li].eng != Engine::off)
                    renderLayer (layers[li], P.layer[(size_t) li], l, r);

            if (noiseLevel > 0.0f || chiff > 1.0e-4f)
            {
                const float nz = noiseBp.bpNorm (rng.bipolar()) * (noiseLevel + chiff * 3.0f);
                chiff *= chiffMul;
                l += nz;
                r += nz;
            }

            if (filterType != filterOff)
            {
                if (driveK > 0.0f)
                {
                    l = fastTanh (l * (1.0f + driveK)) * driveNorm;
                    r = fastTanh (r * (1.0f + driveK)) * driveNorm;
                }
                switch (filterType)
                {
                    case filterLP12: l = filtL1.lp (l); r = filtR1.lp (r); break;
                    case filterLP24: l = filtL2.lp (filtL1.lp (l)); r = filtR2.lp (filtR1.lp (r)); break;
                    case filterBP:   l = filtL1.bpNorm (l); r = filtR1.bpNorm (r); break;
                    case filterHP:   l = filtL1.hp (l); r = filtR1.hp (r); break;
                    default: break;
                }
            }

            fenv.next();
            float e = amp.next() * velGain * ampMod * voiceGain;
            if (killing)
            {
                e *= killGain;
                killGain -= killStep;
                if (killGain <= 0.0f) { active = false; amp.kill(); break; }
            }
            outL[done + i] += l * e * panL;
            outR[done + i] += r * e * panR;
        }

        controlCounter -= todo;
        done += todo;
        if (! active || ! amp.active())
        {
            active = false;
            note = -1;
            break;
        }
    }
}
} // namespace dmb
