#pragma once

// Atajos para escribir los sonidos base de forma compacta.
#include "Factory.h"

namespace dmb::build
{
inline Layer& A (Patch& p) { return p.layer[0]; }
inline Layer& B (Patch& p) { return p.layer[1]; }

inline void layerBase (Layer& l, Engine e, float level = 1.0f, int octave = 0, float semi = 0.0f, float fine = 0.0f, float pan = 0.0f)
{
    l[lp::engine] = (float) e;
    l[lp::level] = level;
    l[lp::octave] = (float) octave;
    l[lp::semi] = semi;
    l[lp::fine] = fine;
    l[lp::pan] = pan;
}

// analógico: forma de onda, unísono, desafinación (cents) y ancho
inline void va (Layer& l, int wave, int unison = 1, float detune = 10.0f, float width = 0.6f, float level = 1.0f, int octave = 0)
{
    layerBase (l, Engine::analog, level, octave);
    l[lp::wave1] = (float) wave;
    l[lp::unison] = (float) unison;
    l[lp::uniDetune] = detune;
    l[lp::uniWidth] = width;
}
inline void osc2 (Layer& l, int wave, float semis, float fineCents, float mix)
{
    l[lp::wave2] = (float) wave; l[lp::semi2] = semis; l[lp::fine2] = fineCents; l[lp::mix2] = mix;
}
inline void sub (Layer& l, float level) { l[lp::sub] = level; }
inline void pw (Layer& l, float w) { l[lp::pw] = w; }

// FM: algoritmo, proporciones portadora/moduladora, índice, duración del índice, nivel que se queda
inline void fm (Layer& l, int algo, float r1, float r2, float index, float decay, float sustain, float feedback = 0.0f, float level = 1.0f, int octave = 0)
{
    layerBase (l, Engine::fm, level, octave);
    l[lp::fmAlgo] = (float) algo; l[lp::fmR1] = r1; l[lp::fmR2] = r2; l[lp::fmI2] = index;
    l[lp::fmDec] = decay; l[lp::fmSus] = sustain; l[lp::fmFb] = feedback;
}
// segundo par (o cuarto operador): portadora 3 / moduladora 4
inline void fm2 (Layer& l, float r3, float r4, float index4, float mix3, float decay3 = 0.0f)
{
    l[lp::fmR3] = r3; l[lp::fmR4] = r4; l[lp::fmI4] = index4; l[lp::fmMix3] = mix3; l[lp::fmDec3] = decay3;
}

// cuerda pulsada
inline void pluck (Layer& l, float bright, float pos, float damp, float decay, float body = 0.0f, float level = 1.0f, int octave = 0)
{
    layerBase (l, Engine::pluck, level, octave);
    l[lp::plBright] = bright; l[lp::plPos] = pos; l[lp::plDamp] = damp; l[lp::plDecay] = decay; l[lp::plBody] = body;
}

// modal (piano, mallets, campanas, órgano, flauta)
inline void modal (Layer& l, int model, float decay, float hard = 0.5f, float bright = 0.5f, float tilt = 0.5f,
                   float stretch = 0.3f, int reg = 0, float perc = 0.0f, float level = 1.0f, int octave = 0)
{
    layerBase (l, Engine::modal, level, octave);
    l[lp::moModel] = (float) model; l[lp::moDecay] = decay; l[lp::moHard] = hard; l[lp::moBright] = bright;
    l[lp::moTilt] = tilt; l[lp::moStretch] = stretch; l[lp::moReg] = (float) reg; l[lp::moPerc] = perc;
}

// voces: 0 bajo, 1 tenor, 2 alto, 3 soprano; vocal 0 A, 1 E, 2 I, 3 O, 4 U
inline void vox (Layer& l, int voice, float vowel, float breath = 0.15f, int ensemble = 3, float shift = 0.0f, float level = 1.0f, int octave = 0)
{
    layerBase (l, Engine::vox, level, octave);
    l[lp::vxVoice] = (float) voice; l[lp::vxVowel] = vowel; l[lp::vxBreath] = breath;
    l[lp::vxEnsemble] = (float) ensemble; l[lp::vxShift] = shift;
}

inline void filt (Patch& p, int type, float cutoff, float res = 0.1f, float envOct = 0.0f, float key = 0.5f, float drive = 0.0f, float vel = 0.3f)
{
    p[gp::fType] = (float) type; p[gp::fCut] = cutoff; p[gp::fRes] = res; p[gp::fEnv] = envOct;
    p[gp::fKey] = key; p[gp::fDrive] = drive; p[gp::fVel] = vel;
}
inline void fenv (Patch& p, float a, float d, float s, float r) { p[gp::feA] = a; p[gp::feD] = d; p[gp::feS] = s; p[gp::feR] = r; }
inline void amp (Patch& p, float a, float d, float s, float r)  { p[gp::aA] = a; p[gp::aD] = d; p[gp::aS] = s; p[gp::aR] = r; }
inline void pitchEnv (Patch& p, float semis, float time)        { p[gp::pAmt] = semis; p[gp::pTime] = time; }
inline void vib (Patch& p, float rate, float delay)              { p[gp::vibRate] = rate; p[gp::vibDelay] = delay; }
inline void lfo (Patch& p, float rate, int wave, float toCut, float toAmp = 0.0f, float toPw = 0.0f, float toPan = 0.0f, bool sync = false)
{
    p[gp::l2Rate] = rate; p[gp::l2Wave] = (float) wave; p[gp::l2Cut] = toCut; p[gp::l2Amp] = toAmp;
    p[gp::l2PW] = toPw; p[gp::l2Pan] = toPan; p[gp::l2Sync] = sync ? 1.0f : 0.0f;
}
inline void lfoPitch (Patch& p, float rate, int wave, float semis, bool sync = true)
{
    p[gp::l2Rate] = rate; p[gp::l2Wave] = (float) wave; p[gp::l2Pitch] = semis; p[gp::l2Sync] = sync ? 1.0f : 0.0f;
}
// atajo para declarar un sonido base
#define DMB_ARCH(name, es, en, ...) Archetype { name, es, en, [] (Patch& p, Macros& m) { (void) p; (void) m; __VA_ARGS__ } }
inline void noise (Patch& p, float level, float color, float q = 2.0f, float chiff = 0.0f, float chiffTime = 0.05f)
{
    p[gp::nLevel] = level; p[gp::nColor] = color; p[gp::nQ] = q; p[gp::nChiff] = chiff; p[gp::nChiffT] = chiffTime;
}
inline void chorus (Patch& p, float mix, float rate = 0.6f, float depth = 0.4f) { p[gp::chMix] = mix; p[gp::chRate] = rate; p[gp::chDepth] = depth; }
// divisiones: 0 1/16, 1 1/16D, 2 1/8T, 3 1/8, 4 1/4T, 5 1/8D, 6 1/4, 7 1/4D, 8 1/2, 9 1/2D
inline void delay (Patch& p, int div, float fb = 0.35f, float tone = 0.5f, bool ping = true)
{
    p[gp::dlDiv] = (float) div; p[gp::dlFb] = fb; p[gp::dlTone] = tone; p[gp::dlPing] = ping ? 1.0f : 0.0f;
}
inline void reverb (Patch& p, float size, float decay, float tone = 0.5f) { p[gp::rvSize] = size; p[gp::rvDecay] = decay; p[gp::rvTone] = tone; }
inline void eq (Patch& p, float lowCut, float highCut) { p[gp::eqLow] = lowCut; p[gp::eqHigh] = highCut; }
inline void bendRange (Patch& p, int semis) { p[gp::bend] = (float) semis; }

inline void macros (Macros& m, float espacio, float eco, int caracter = charClean, float suciedad = 0.0f,
                    int modo = playPoly, float glide = 0.0f, float vibrato = 0.0f, float fuerza = 60.0f)
{
    m.espacio = espacio; m.eco = eco; m.caracter = caracter; m.suciedad = suciedad;
    m.modo = modo; m.glide = glide; m.vibrato = vibrato; m.fuerza = fuerza;
}
} // namespace dmb::build
