#pragma once

#include "Patch.h"
#include "Dsp.h"

namespace dmb
{
// Lo que comparten todas las voces en cada bloque: el patch y lo que hacen las macros.
struct VoiceParams
{
    const Patch* patch = nullptr;
    float sr = 48000.0f;

    float bright = 0.0f;        // -1..1 (BRILLO)
    float cutoffMul = 1.0f;
    float decayMul = 1.0f;      // DURACIÓN
    float attackMul = 1.0f, attackAdd = 0.0f;   // ATAQUE
    float detuneMul = 1.0f, widthAdd = 0.0f;    // GROSOR
    float vibDepth = 0.0f;      // semitonos (VIBRATO + rueda de modulación)
    float glideTime = 0.0f;     // segundos
    float velSens = 0.6f;       // FUERZA
    float transpose = 0.0f;     // OCTAVA en semitonos
    float bend = 0.0f;          // semitonos (ya con el rango)
};

class Voice
{
public:
    static constexpr int maxPartials = 24;
    static constexpr int controlRate = 16;   // muestras entre actualizaciones de modulación

    void prepare (float sampleRate);
    void noteOn (int note, float velocity, float startPitch, const VoiceParams&, uint32_t seed);
    void legatoTo (int note, float velocity);          // modo legato: cambia de nota sin volver a empezar
    void retrigger (int note, float velocity, const VoiceParams&);   // modo mono: vuelve a empezar desde la nota actual
    void noteOff();
    void fastKill();                                   // se apaga en ~5 ms (cuando se roba la voz)
    void reset();

    bool isActive() const    { return active; }
    bool isReleasing() const { return active && (amp.stage == dsp::Adsr::release || killing); }
    float currentPitch() const { return pitchNow; }
    float level() const      { return amp.value; }

    // suma su sonido a L y R
    void render (float* L, float* R, int numSamples, const VoiceParams&);

    int note = -1;
    float velocity = 0.0f;
    uint64_t order = 0;
    bool keyDown = false, sustained = false;

private:
    struct LayerState
    {
        Engine eng = Engine::off;
        float gain = 0.0f, pitchOffset = 0.0f, panL = 0.707f, panR = 0.707f;
        // analógico
        int uni = 1;
        float ph[7] {}, det[7] {}, gl[7] {}, gr[7] {}, uniNorm = 1.0f;
        float ph2 = 0.0f, phSub = 0.0f, inc1 = 0.0f;
        // FM
        float p1 = 0, p2 = 0, p3 = 0, p4 = 0, m2a = 0, m2b = 0, idxEnv = 1, idxMul = 1, car3 = 1, car3Mul = 1;
        // cuerda pulsada
        dsp::DelayLine line;
        float loopZ = 0.0f, loopA = 0.3f, loopG = 0.99f, loopDelay = 100.0f, dcZ = 0.0f, dcX = 0.0f, plT60 = 2.0f;
        dsp::Svf body1, body2;
        // modal
        int nPart = 0;
        float pph[maxPartials] {}, pratio[maxPartials] {}, pinc[maxPartials] {};
        float pamp[maxPartials] {}, pamp2[maxPartials] {}, pmul[maxPartials] {}, pmul2[maxPartials] {};
        float mallet = 0.0f, malletMul = 0.0f;
        dsp::Svf malletBp;
        // voces (formantes)
        int ens = 1;
        float vph[4] {}, vdet[4] {}, vgl[4] {}, vgr[4] {}, drift[4] {}, driftTarget[4] {};
        dsp::Svf form[2][4];
        float formGain[4] {};
        dsp::OnePole glottalL, glottalR;
    };

    void startLayer (int index, const VoiceParams&, bool resetPhases);
    void setupModal (LayerState&, const Layer&, float f0, const VoiceParams&);
    void setupPluck (LayerState&, const Layer&, float f0, const VoiceParams&);
    void setupVox (LayerState&, const Layer&, const VoiceParams&);
    void updatePluckTuning (LayerState&, float f0);
    void updateControl (const VoiceParams&, int blockLen);
    inline void renderLayer (LayerState&, const Layer&, float& l, float& r);

    float sr = 48000.0f;
    bool active = false, killing = false;
    float killGain = 1.0f, killStep = 0.0f;
    dsp::Rng rng;

    LayerState layers[2];

    // tono
    float pitchNow = 60.0f, pitchTarget = 60.0f, glideCoef = 1.0f;
    float pitchEnv = 0.0f, pitchEnvMul = 1.0f;
    float vibPhase = 0.0f, lfoPhase = 0.0f, lfoValue = 0.0f, shValue = 0.0f;
    float timeSinceOn = 0.0f;
    float pwMod = 0.0f, panMod = 0.0f, ampMod = 1.0f;

    // envolventes y filtro
    dsp::Adsr amp, fenv;
    dsp::Svf filtL1, filtL2, filtR1, filtR2;
    int filterType = filterLP24;
    float driveK = 0.0f, driveNorm = 1.0f;
    float velGain = 1.0f, velCut = 0.0f, velBright = 0.0f;

    // ruido / soplido
    dsp::Svf noiseBp;
    float noiseLevel = 0.0f, chiff = 0.0f, chiffMul = 0.0f;
    int controlCounter = 0;
    float lastDetuneMul = 1.0f, lastFmIndexMul = 1.0f, panL = 1.0f, panR = 1.0f;
};
} // namespace dmb
