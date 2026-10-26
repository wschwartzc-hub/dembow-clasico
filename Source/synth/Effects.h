#pragma once

#include "Patch.h"
#include "Dsp.h"

namespace dmb
{
// Cadena de efectos de salida: EQ del sonido → carácter (limpio / vintage / cálido / lo-fi)
// → chorus → eco → reverb → volumen → limitador.
class Effects
{
public:
    void prepare (double sampleRate, int maxBlock);
    void reset();
    void process (float* L, float* R, int numSamples, const Patch&, const Macros&, double bpm);

private:
    void character (float& l, float& r, int type, float amount);

    float sr = 48000.0f;
    dsp::Rng rng;

    // EQ del patch
    dsp::Svf eqHpL, eqHpR, eqLpL, eqLpR;
    float eqLowCur = -1.0f, eqHighCur = -1.0f;

    // carácter
    dsp::Smooth amountS;
    int lastChar = -1;
    dsp::Svf aaL1, aaL2, aaR1, aaR2, postL, postR, lofiHpL, lofiHpR;
    dsp::OnePole warmLpL, warmLpR, warmLowL, warmLowR;
    float decPhase = 0.0f, holdL = 0.0f, holdR = 0.0f;
    dsp::DelayLine wowL, wowR;
    float wowPhase = 0.0f, flutterPhase = 0.0f, crackle = 0.0f;
    float charSetFor = -1.0f;

    // chorus
    dsp::DelayLine chL, chR;
    float chPhase = 0.0f;
    dsp::Smooth chMixS;

    // eco
    dsp::DelayLine dlL, dlR;
    dsp::OnePole dlLpL, dlLpR, dlHpL, dlHpR;
    dsp::Smooth dlTimeS, dlMixS;

    // reverb (red de retardos con retroalimentación)
    static constexpr int N = 8;
    dsp::DelayLine rvLine[N];
    float rvDamp[N] {}, rvG[N] {};
    float rvOscPhase[N] {};
    struct Allpass { dsp::DelayLine line; int len = 1;
        inline float tick (float x, float g) { const float d = line.read ((float) len); const float v = x + g * d; line.push (v); return d - g * v; } };
    Allpass diff[2][4];
    dsp::DelayLine preL, preR;
    float rvSizeCur = -1.0f, rvDampC = 0.5f, rvOutGain = 0.5f;
    float rvHpL = 0.0f, rvHpR = 0.0f, rvHpC = 0.01f;
    dsp::Smooth rvMixS;

    // salida
    dsp::Smooth volS;
    float dcL = 0.0f, dcR = 0.0f, dcXL = 0.0f, dcXR = 0.0f;
};
} // namespace dmb
