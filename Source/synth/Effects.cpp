#include "Effects.h"

namespace dmb
{
using namespace dsp;

namespace
{
    constexpr float rvBaseMs[8] = { 31.7f, 37.3f, 41.9f, 46.3f, 53.1f, 59.3f, 66.7f, 73.3f };
    constexpr float rvDiffMs[2][4] = { { 4.77f, 3.59f, 12.73f, 9.30f }, { 4.93f, 3.71f, 12.37f, 9.61f } };
    constexpr float rvMaxScale = 1.6f;

    inline float softLimit (float x)
    {
        const float a = std::abs (x);
        if (a <= 0.9f) return x;
        return std::copysign (0.9f + 0.1f * fastTanh ((a - 0.9f) * 10.0f), x);
    }

    inline float quantize (float x, float bits)
    {
        const float q = std::exp2 (bits - 1.0f);
        return std::round (x * q) / q;
    }
}

void Effects::prepare (double sampleRate, int)
{
    sr = (float) sampleRate;
    wowL.resize ((int) (0.02f * sr));
    wowR.resize ((int) (0.02f * sr));
    chL.resize ((int) (0.04f * sr));
    chR.resize ((int) (0.04f * sr));
    dlL.resize ((int) (3.1f * sr));
    dlR.resize ((int) (3.1f * sr));
    for (int i = 0; i < N; ++i)
        rvLine[i].resize ((int) (rvBaseMs[i] * rvMaxScale * 0.001f * sr + 0.003f * sr) + 8);
    for (int c = 0; c < 2; ++c)
        for (int k = 0; k < 4; ++k)
        {
            diff[c][k].len = std::max (1, (int) std::round (rvDiffMs[c][k] * 0.001f * sr));
            diff[c][k].line.resize (diff[c][k].len + 4);
        }
    preL.resize ((int) (0.05f * sr));
    preR.resize ((int) (0.05f * sr));
    rvHpC = 1.0f - std::exp (-twoPi * 30.0f / sr);

    amountS.setTime (0.05f, sr);
    chMixS.setTime (0.05f, sr);
    dlTimeS.setTime (0.25f, sr);
    dlMixS.setTime (0.05f, sr);
    rvMixS.setTime (0.05f, sr);
    volS.setTime (0.03f, sr);
    reset();
}

void Effects::reset()
{
    for (auto* d : { &wowL, &wowR, &chL, &chR, &dlL, &dlR, &preL, &preR }) d->clear();
    for (auto& l : rvLine) l.clear();
    for (auto& c : diff) for (auto& a : c) a.line.clear();
    for (auto& z : rvDamp) z = 0.0f;
    for (int i = 0; i < N; ++i) rvOscPhase[i] = (float) i / (float) N;
    for (auto* f : { &eqHpL, &eqHpR, &eqLpL, &eqLpR, &aaL1, &aaL2, &aaR1, &aaR2, &postL, &postR, &lofiHpL, &lofiHpR }) f->reset();
    for (auto* f : { &warmLpL, &warmLpR, &warmLowL, &warmLowR, &dlLpL, &dlLpR, &dlHpL, &dlHpR }) f->reset();
    decPhase = holdL = holdR = 0.0f;
    rvSizeCur = -1.0f;
    rvHpL = rvHpR = 0.0f;
    dcL = dcR = dcXL = dcXR = 0.0f;
    eqLowCur = eqHighCur = -1.0f;
    lastChar = -1;
    charSetFor = -1.0f;
    amountS.reset (0.0f);
    chMixS.reset (0.0f);
    dlTimeS.reset (-1.0f);
    dlMixS.reset (0.0f);
    rvMixS.reset (0.0f);
    volS.reset (1.0f);
}

// ---------------------------------------------------------------------------------------------
void Effects::character (float& l, float& r, int type, float s)
{
    switch (type)
    {
        case charVintage:
        {
            // sampler de los 2000: menos resolución y menos frecuencia de muestreo
            const float rate = lerp (40000.0f, 22050.0f, s);
            const float bits = lerp (14.0f, 10.0f, s);
            float xl = aaL2.lp (aaL1.lp (l)), xr = aaR2.lp (aaR1.lp (r));
            if (rate < sr * 0.98f)
            {
                decPhase += rate / sr;
                if (decPhase >= 1.0f) { decPhase -= 1.0f; holdL = xl; holdR = xr; }
                xl = holdL; xr = holdR;
            }
            xl = quantize (xl, bits);
            xr = quantize (xr, bits);
            l = postL.lp (fastTanh (xl * 1.15f) * 0.88f);
            r = postR.lp (fastTanh (xr * 1.15f) * 0.88f);
            break;
        }
        case charWarm:
        {
            // cinta: saturación con armónicos pares, graves llenos y agudos suaves
            const float k = 1.0f + 2.5f * s;
            float xl = fastTanh (k * (l + 0.12f * l * l)) / k * (1.0f + 0.4f * s);
            float xr = fastTanh (k * (r + 0.12f * r * r)) / k * (1.0f + 0.4f * s);
            xl += 0.25f * warmLowL.lp (xl);
            xr += 0.25f * warmLowR.lp (xr);
            l = warmLpL.lp (xl);
            r = warmLpR.lp (xr);
            break;
        }
        case charLofi:
        {
            // casete / disco viejo: banda angosta, pocos bits, siseo, crujidos y oscilación
            wowL.push (l);
            wowR.push (r);
            wowPhase += 0.55f / sr;
            flutterPhase += 6.3f / sr;
            if (wowPhase >= 1.0f) wowPhase -= 1.0f;
            if (flutterPhase >= 1.0f) flutterPhase -= 1.0f;
            const float d = 0.004f * sr + (0.0007f * sine01 (wowPhase) + 0.00012f * sine01 (flutterPhase)) * sr * (0.4f + s);
            float xl = wowL.read (d), xr = wowR.read (d);
            xl = postL.lp (lofiHpL.hp (xl));
            xr = postR.lp (lofiHpR.hp (xr));
            const float rate = lerp (26000.0f, 11025.0f, s);
            decPhase += rate / sr;
            if (decPhase >= 1.0f) { decPhase -= 1.0f; holdL = xl; holdR = xr; }
            const float bits = lerp (11.0f, 6.5f, s);
            xl = quantize (holdL, bits);
            xr = quantize (holdR, bits);
            const float hiss = rng.bipolar() * dbToGain (-60.0f + 14.0f * s);
            float pop = 0.0f;
            if (rng.unipolar() < (1.5f + 9.0f * s) / sr) crackle = (0.03f + 0.12f * s) * (rng.unipolar() * 0.7f + 0.3f);
            if (crackle > 1.0e-4f) { pop = crackle * rng.bipolar(); crackle *= 0.6f; }
            l = fastTanh ((xl + hiss + pop) * 1.2f) * 0.85f;
            r = fastTanh ((xr + hiss + pop) * 1.2f) * 0.85f;
            break;
        }
        default:
            if (s > 0.001f)
            {
                const float k = 1.0f + 4.0f * s;
                const float norm = 1.0f / std::sqrt (k);
                l = fastTanh (k * l) * norm;
                r = fastTanh (k * r) * norm;
            }
            break;
    }
}

void Effects::process (float* L, float* R, int n, const Patch& P, const Macros& M, double bpm)
{
    // ----- EQ del sonido
    const float eqLow = P[gp::eqLow], eqHigh = P[gp::eqHigh];
    if (std::abs (eqLow - eqLowCur) > 0.5f) { eqHpL.set (eqLow, 0.707f, sr); eqHpR.set (eqLow, 0.707f, sr); eqLowCur = eqLow; }
    if (std::abs (eqHigh - eqHighCur) > 0.5f) { eqLpL.set (eqHigh, 0.707f, sr); eqLpR.set (eqHigh, 0.707f, sr); eqHighCur = eqHigh; }
    const bool useHp = eqLow > 25.0f, useLp = eqHigh < 19000.0f && eqHigh < sr * 0.45f;

    // ----- carácter
    const int type = juce::jlimit (0, (int) numCharacters - 1, M.caracter);
    const float amountTarget = clamp (M.suciedad * 0.01f, 0.0f, 1.0f);
    amountS.target = amountTarget;
    if (type != lastChar || std::abs (amountTarget - charSetFor) > 0.01f)
    {
        const float s = amountTarget;
        if (type == charVintage)
        {
            const float rate = lerp (40000.0f, 22050.0f, s);
            const float aa = std::min (rate * 0.45f, sr * 0.45f);
            aaL1.set (aa, 0.54f, sr); aaL2.set (aa, 1.31f, sr);
            aaR1.set (aa, 0.54f, sr); aaR2.set (aa, 1.31f, sr);
            postL.set (std::min (rate * 0.47f, sr * 0.45f), 0.707f, sr);
            postR.set (std::min (rate * 0.47f, sr * 0.45f), 0.707f, sr);
        }
        else if (type == charWarm)
        {
            warmLpL.setLowpass (lerp (17000.0f, 8500.0f, s), sr);
            warmLpR.setLowpass (lerp (17000.0f, 8500.0f, s), sr);
            warmLowL.setLowpass (160.0f, sr);
            warmLowR.setLowpass (160.0f, sr);
        }
        else if (type == charLofi)
        {
            lofiHpL.set (110.0f + 140.0f * s, 0.707f, sr);
            lofiHpR.set (110.0f + 140.0f * s, 0.707f, sr);
            postL.set (lerp (9000.0f, 3400.0f, s), 0.8f, sr);
            postR.set (lerp (9000.0f, 3400.0f, s), 0.8f, sr);
        }
        if (type != lastChar)
        {
            for (auto* f : { &aaL1, &aaL2, &aaR1, &aaR2, &postL, &postR, &lofiHpL, &lofiHpR }) f->reset();
            wowL.clear(); wowR.clear();
        }
        lastChar = type;
        charSetFor = amountTarget;
    }

    // ----- chorus
    const float grosor = (M.grosor - 50.0f) / 50.0f;
    chMixS.target = clamp (P[gp::chMix] + std::max (0.0f, grosor) * 0.4f, 0.0f, 1.0f);
    const float chRate = P[gp::chRate], chDepth = P[gp::chDepth] * 0.0045f * sr;
    const float chBase = 0.012f * sr;

    // ----- eco
    const float beats = delayDivisionBeats (P.i (gp::dlDiv));
    const float dlTime = clamp (beats * 60.0f / (float) (bpm > 20.0 ? bpm : 95.0) * sr, 16.0f, 3.0f * sr);
    if (dlTimeS.v < 0.0f) dlTimeS.reset (dlTime);
    dlTimeS.target = dlTime;
    dlMixS.target = clamp (M.eco * 0.01f, 0.0f, 1.0f) * 0.55f;
    const float dlFb = P[gp::dlFb];
    const bool ping = P.i (gp::dlPing) != 0;
    const float dlCut = lerp (1800.0f, 12000.0f, P[gp::dlTone]);
    dlLpL.setLowpass (dlCut, sr); dlLpR.setLowpass (dlCut, sr);
    dlHpL.setLowpass (160.0f, sr); dlHpR.setLowpass (160.0f, sr);

    // ----- reverb
    const float sizeTarget = 0.45f + (rvMaxScale - 0.45f) * clamp (P[gp::rvSize], 0.0f, 1.0f);
    if (rvSizeCur < 0.0f) rvSizeCur = sizeTarget;
    rvSizeCur += (sizeTarget - rvSizeCur) * std::min (1.0f, (float) n / (0.2f * sr));
    const float t60 = clamp (P[gp::rvDecay], 0.2f, 12.0f);
    for (int i = 0; i < N; ++i)
        rvG[i] = std::pow (10.0f, -3.0f * rvBaseMs[i] * 0.001f * rvSizeCur / t60);
    rvDampC = 1.0f - std::exp (-twoPi * lerp (2500.0f, 14000.0f, P[gp::rvTone]) / sr);
    rvOutGain = 0.62f / std::sqrt (1.0f + 0.35f * t60);
    rvMixS.target = clamp (M.espacio * 0.01f, 0.0f, 1.0f) * 0.7f;
    const float rvMs = 0.001f * sr * rvSizeCur;
    const float rvMod = 0.00035f * sr;
    const float preDelay = 0.012f * sr;

    volS.target = dbToGain (P[gp::level] + M.volumen);

    for (int i = 0; i < n; ++i)
    {
        float l = L[i], r = R[i];

        if (useHp) { l = eqHpL.hp (l); r = eqHpR.hp (r); }
        if (useLp) { l = eqLpL.lp (l); r = eqLpR.lp (r); }

        const float amt = amountS.next();
        if (type != charClean || amt > 0.001f)
            character (l, r, type, amt);

        // chorus
        const float cm = chMixS.next();
        chL.push (l);
        chR.push (r);
        if (cm > 0.001f)
        {
            chPhase += chRate / sr;
            if (chPhase >= 1.0f) chPhase -= 1.0f;
            const float a = sine01 (chPhase), b = sine01 (chPhase + 0.25f), c = sine01 (chPhase + 0.5f);
            const float wl = 0.5f * (chL.read (chBase + chDepth * a) + chL.read (chBase * 1.37f + chDepth * c));
            const float wr = 0.5f * (chR.read (chBase + chDepth * b) + chR.read (chBase * 1.29f - chDepth * c));
            l = l * (1.0f - 0.45f * cm) + wl * cm * 0.9f;
            r = r * (1.0f - 0.45f * cm) + wr * cm * 0.9f;
        }

        // eco
        const float dm = dlMixS.next();
        const float dt = dlTimeS.next();
        float el = dlL.read (dt), er = dlR.read (dt);
        el = dlLpL.lp (el); er = dlLpR.lp (er);
        el = dlHpL.hp (el); er = dlHpR.hp (er);
        if (ping)
        {
            dlL.push (0.5f * (l + r) * (dm > 0.0f ? 1.0f : 0.0f) + er * dlFb);
            dlR.push (el * dlFb);
        }
        else
        {
            dlL.push (l * (dm > 0.0f ? 1.0f : 0.0f) + el * dlFb);
            dlR.push (r * (dm > 0.0f ? 1.0f : 0.0f) + er * dlFb);
        }
        const float wetL = el * dm, wetR = er * dm;

        // reverb (entra el sonido y un poco del eco)
        const float rm = rvMixS.next();
        float revL = 0.0f, revR = 0.0f;
        {
            preL.push (l + wetL * 0.5f);
            preR.push (r + wetR * 0.5f);
            float xl = preL.read (preDelay), xr = preR.read (preDelay);
            for (int k = 0; k < 4; ++k)
            {
                xl = diff[0][k].tick (xl, 0.62f);
                xr = diff[1][k].tick (xr, 0.62f);
            }
            float y[N];
            float sum = 0.0f;
            for (int k = 0; k < N; ++k)
            {
                rvOscPhase[k] += (0.11f + 0.137f * (float) k) / sr;
                if (rvOscPhase[k] >= 1.0f) rvOscPhase[k] -= 1.0f;
                const float d = rvBaseMs[k] * rvMs + rvMod * (1.0f + sine01 (rvOscPhase[k]));
                y[k] = (rvDamp[k] += rvDampC * (rvLine[k].read (d) - rvDamp[k]));
                sum += y[k];
            }
            const float s = sum * (2.0f / (float) N);
            for (int k = 0; k < N; ++k)
                rvLine[k].push ((y[k] - s) * rvG[k] + ((k & 1) ? xr : xl) * 0.5f);
            float ol = (y[0] + y[2] + y[4] + y[6]) * rvOutGain;
            float orr = (y[1] + y[3] + y[5] + y[7]) * rvOutGain;
            rvHpL += rvHpC * (ol - rvHpL);
            rvHpR += rvHpC * (orr - rvHpR);
            revL = (ol - rvHpL) * rm;
            revR = (orr - rvHpR) * rm;
        }

        const float dry = 1.0f - 0.25f * rm;
        l = l * dry + wetL + revL;
        r = r * dry + wetR + revR;

        // salida: volumen, sin componente continua y sin pasarse de 0 dB
        const float v = volS.next();
        l *= v;
        r *= v;
        const float hl = l - dcXL + 0.9995f * dcL; dcXL = l; dcL = hl;
        const float hr = r - dcXR + 0.9995f * dcR; dcXR = r; dcR = hr;
        L[i] = softLimit (hl);
        R[i] = softLimit (hr);
    }
}
} // namespace dmb
