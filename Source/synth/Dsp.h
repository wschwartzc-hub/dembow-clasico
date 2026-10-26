#pragma once

// Bloques básicos de DSP (sin dependencias de JUCE para que sean rápidos y fáciles de probar).
#include <cmath>
#include <cstdint>
#include <array>
#include <vector>
#include <algorithm>

namespace dmb::dsp
{
inline constexpr float pi = 3.14159265358979f;
inline constexpr float twoPi = 6.28318530717959f;

inline float clamp (float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline float lerp (float a, float b, float t) { return a + (b - a) * t; }
inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g) { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }
inline float noteToHz (float note) { return 440.0f * std::exp2 ((note - 69.0f) / 12.0f); }

// tanh rápida (error < 0.1 %), suficiente para saturación
inline float fastTanh (float x)
{
    if (x > 4.97f) return 1.0f;
    if (x < -4.97f) return -1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// ---------------------------------------------------------------------------------------------
// Ruido rápido (xorshift)
struct Rng
{
    uint32_t s = 0x9e3779b9u;
    void seed (uint32_t v) { s = v ? v : 0x9e3779b9u; }
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float bipolar() { return (float) (int32_t) next() * (1.0f / 2147483648.0f); }   // -1..1
    float unipolar() { return (float) (next() >> 8) * (1.0f / 16777216.0f); }        // 0..1
};

// ---------------------------------------------------------------------------------------------
// Tabla de seno (con interpolación lineal). Fase en ciclos (0..1).
struct SineTable
{
    static constexpr int size = 4096;
    std::array<float, size + 1> t {};
    SineTable()
    {
        for (int i = 0; i <= size; ++i) t[(size_t) i] = std::sin (twoPi * (float) i / (float) size);
    }
    static const SineTable& get() { static const SineTable s; return s; }
};

inline float sine01 (float phase)   // phase en ciclos, cualquier valor
{
    phase -= std::floor (phase);
    const float x = phase * (float) SineTable::size;
    const int i = (int) x;
    const float f = x - (float) i;
    const auto& t = SineTable::get().t;
    return t[(size_t) i] + (t[(size_t) i + 1] - t[(size_t) i]) * f;
}

// ---------------------------------------------------------------------------------------------
// PolyBLEP para osciladores sin aliasing
inline float polyBlep (float t, float dt)
{
    if (t < dt)       { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}

// ---------------------------------------------------------------------------------------------
// Filtro de variables de estado (TPT / Zavalishin): estable con modulación rápida
struct Svf
{
    float g = 0.1f, k = 1.4f, a1 = 0, a2 = 0, a3 = 0;
    float ic1 = 0, ic2 = 0;

    void set (float cutoffHz, float q, float sampleRate)
    {
        const float fc = clamp (cutoffHz, 10.0f, sampleRate * 0.49f);
        g = std::tan (pi * fc / sampleRate);
        k = 1.0f / std::max (q, 0.05f);
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }
    void reset() { ic1 = ic2 = 0.0f; }

    // devuelve low, band y high a la vez
    inline void tick (float v0, float& low, float& band, float& high)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;
        low = v2; band = v1; high = v0 - k * v1 - v2;
    }
    inline float lp (float x)   { float l, b, h; tick (x, l, b, h); return l; }
    inline float bp (float x)   { float l, b, h; tick (x, l, b, h); return b; }
    inline float hp (float x)   { float l, b, h; tick (x, l, b, h); return h; }
    inline float bpNorm (float x) { float l, b, h; tick (x, l, b, h); return b * k; }   // ganancia 1 en el centro
};

// Pasa-bajos / pasa-altos de un polo
struct OnePole
{
    float a = 0.5f, z = 0.0f;
    void setLowpass (float hz, float sr) { a = 1.0f - std::exp (-twoPi * clamp (hz, 1.0f, sr * 0.49f) / sr); }
    inline float lp (float x) { z += a * (x - z); return z; }
    inline float hp (float x) { z += a * (x - z); return x - z; }
    void reset (float v = 0.0f) { z = v; }
};

// Suavizado de parámetros
struct Smooth
{
    float v = 0.0f, target = 0.0f, coef = 0.002f;
    void setTime (float seconds, float sr) { coef = 1.0f - std::exp (-1.0f / std::max (1.0f, seconds * sr)); }
    void reset (float x) { v = target = x; }
    inline float next() { v += coef * (target - v); return v; }
};

// ---------------------------------------------------------------------------------------------
// Envolvente ADSR con curvas "analógicas" (exponenciales). Tiempos en segundos.
struct Adsr
{
    enum Stage { idle, attack, decay, sustain, release };
    Stage stage = idle;
    float value = 0.0f;
    float decayCoef = 0, releaseCoef = 0, sustainLevel = 1.0f, attackStep = 0;

    static float coefFor (float seconds, float sr)   // llega a -60 dB en 'seconds'
    {
        return std::exp (-6.9078f / std::max (1.0f, seconds * sr));
    }

    void set (float a, float d, float s, float r, float sr)
    {
        attackStep = 1.293f / std::max (1.0f, a * sr);
        decayCoef = coefFor (d, sr);
        releaseCoef = coefFor (r, sr);
        sustainLevel = clamp (s, 0.0f, 1.0f);
    }
    void noteOn (bool retrigger = true)
    {
        if (retrigger || stage == idle || stage == release) stage = attack;
    }
    void noteOff() { if (stage != idle) stage = release; }
    void kill() { stage = idle; value = 0.0f; }
    bool active() const { return stage != idle; }

    inline float next()
    {
        switch (stage)
        {
            case attack:
                value += attackStep * (1.05f - 0.5f * value);   // casi lineal, un poco curvo al final
                if (value >= 1.0f) { value = 1.0f; stage = decay; }
                break;
            case decay:
                value = sustainLevel + (value - sustainLevel) * decayCoef;
                if (value - sustainLevel < 1.0e-4f) { value = sustainLevel; stage = sustain; }
                break;
            case sustain:
                value = sustainLevel;
                if (sustainLevel <= 1.0e-5f) { stage = idle; value = 0.0f; }
                break;
            case release:
                value *= releaseCoef;
                if (value < 1.0e-5f) { value = 0.0f; stage = idle; }
                break;
            case idle: break;
        }
        return value;
    }
};

// Línea de retardo con lectura fraccional
struct DelayLine
{
    std::vector<float> buf;
    int mask = 0, w = 0;
    void resize (int minSize)
    {
        int n = 1;
        while (n < minSize + 4) n <<= 1;
        buf.assign ((size_t) n, 0.0f);
        mask = n - 1; w = 0;
    }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    inline void push (float x) { buf[(size_t) w] = x; w = (w + 1) & mask; }
    inline float read (float delaySamples) const   // lineal
    {
        const float rp = (float) w - delaySamples;
        const float fl = std::floor (rp);
        const int i = (int) fl;
        const float f = rp - fl;
        const float a = buf[(size_t) (i & mask)], b = buf[(size_t) ((i + 1) & mask)];
        return a + (b - a) * f;
    }
    inline float readCubic (float delaySamples) const
    {
        const float rp = (float) w - delaySamples;
        const float fl = std::floor (rp);
        const int i = (int) fl;
        const float f = rp - fl;
        const float y0 = buf[(size_t) ((i - 1) & mask)], y1 = buf[(size_t) (i & mask)];
        const float y2 = buf[(size_t) ((i + 1) & mask)], y3 = buf[(size_t) ((i + 2) & mask)];
        const float c0 = y1, c1 = 0.5f * (y2 - y0), c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * f + c2) * f + c1) * f + c0;
    }
};
} // namespace dmb::dsp
