// Sonidos base: CUERDAS, PIZZICATO, METALES y ORCH HIT
#include "Build.h"

namespace dmb
{
using namespace build;

namespace
{
    // sección de cuerdas: sierras desafinadas con ataque de arco
    void strings (Patch& p, int unison, float detune, float cutoff, float attack, float release, int octave = 0)
    {
        va (A (p), waveSaw, unison, detune, 0.85f, 1.0f, octave);
        filt (p, filterLP24, cutoff, 0.1f, 0.4f, 0.4f);
        fenv (p, attack, 1.0f, 0.8f, release);
        amp (p, attack, 1, 1, release);
        noise (p, 0.025f, 0.85f, 1.0f);
        vib (p, 5.4f, 0.45f);
        chorus (p, 0.3f);
        reverb (p, 0.75f, 3.0f);
        eq (p, 70, 20000);
    }

    // golpe de orquesta: sierras + capa grave + ruido + caída de tono + sala
    void hit (Patch& p, float third, float fifth)
    {
        va (A (p), waveSaw, 5, 14, 0.8f);
        osc2 (A (p), waveSaw, third, 0, 0.5f);
        va (B (p), waveSaw, 3, 10, 0.7f, 0.6f);
        B (p)[lp::semi] = fifth;
        filt (p, filterLP24, 3500, 0.1f, 1.5f, 0.4f);
        fenv (p, 0.001f, 0.3f, 0.2f, 0.3f);
        amp (p, 0.001f, 0.7f, 0, 0.4f);
        noise (p, 0.0f, 0.6f, 0.8f, 0.8f, 0.06f);
        pitchEnv (p, 1.5f, 0.05f);
        reverb (p, 0.85f, 3.5f);
    }

    // sección de metales: sierras con el filtro que abre al soplar
    void brass (Patch& p, int unison, float cutoff, float envOct, float attack, float decay, float sustain, float release, int octave = 0)
    {
        va (A (p), waveSaw, unison, 10, 0.7f, 1.0f, octave);
        filt (p, filterLP24, cutoff, 0.15f, envOct, 0.6f);
        fenv (p, attack, decay, sustain, release);
        amp (p, std::max (0.003f, attack * 0.8f), 1, 1, release);
        vib (p, 5.5f, 0.4f);
        reverb (p, 0.6f, 2.2f);
    }
}

void addOrchestra (std::vector<Archetype> (&cats)[numCategories])
{
    // =========================================================================================
    cats[catStrings] = {
        DMB_ARCH ("Cuerdas Orquesta", "Sección de cuerdas completa, con vibrato y sala.", "A full string section, with vibrato and a room.",
            strings (p, 6, 14, 4200, 0.18f, 0.7f); macros (m, 35, 0, charClean, 0, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Romántico", "Cuerdas lentas y cálidas, para baladas.", "Slow, warm strings, for ballads.",
            strings (p, 5, 12, 2600, 0.4f, 1.2f); vib (p, 5.2f, 0.5f); chorus (p, 0.35f); reverb (p, 0.8f, 3.5f);
            macros (m, 40, 0, charWarm, 20, playPoly, 0, 22, 55); ),
        DMB_ARCH ("Cuerdas 80s", "Máquina de cuerdas: sierras con mucho chorus.", "A string machine: saws with lots of chorus.",
            va (A (p), waveSaw, 3, 10, 0.7f); filt (p, filterLP12, 5000); amp (p, 0.12f, 1, 1, 0.6f); chorus (p, 0.7f, 0.8f, 0.7f);
            reverb (p, 0.6f, 2.5f); macros (m, 30, 0, charVintage, 25); ),
        DMB_ARCH ("Cuerdas Staccato", "Golpes cortos de arco.", "Short bow strokes.",
            strings (p, 5, 12, 3500, 0.008f, 0.12f); filt (p, filterLP24, 3500, 0.1f, 1.0f, 0.4f); fenv (p, 0.001f, 0.15f, 0.3f, 0.1f);
            amp (p, 0.008f, 0.18f, 0.0f, 0.12f); reverb (p, 0.6f, 2.0f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Violín Solo", "Un violín: una sola voz con vibrato y portamento.", "A violin: one voice with vibrato and glide.",
            va (A (p), waveSaw); filt (p, filterLP24, 3800, 0.25f, 0.3f, 0.5f); noise (p, 0.03f, 0.85f, 1.0f); amp (p, 0.08f, 1, 1, 0.25f);
            vib (p, 5.8f, 0.25f); reverb (p, 0.65f, 2.5f); eq (p, 150, 20000); macros (m, 30, 0, charClean, 0, playLegato, 8, 35, 55); ),
        DMB_ARCH ("Cello", "Cello: grave, cálido y expresivo.", "A cello: deep, warm and expressive.",
            va (A (p), waveSaw, 1, 0, 0.3f, 1.0f, -1); osc2 (A (p), waveSaw, 0, 6, 0.4f); filt (p, filterLP24, 1500, 0.2f, 0.3f, 0.5f);
            amp (p, 0.12f, 1, 1, 0.35f); vib (p, 5.0f, 0.4f); reverb (p, 0.65f, 2.5f); macros (m, 30, 0, charWarm, 15, playLegato, 6, 30, 55); ),
        DMB_ARCH ("Cuerdas Cine", "Sección enorme con contrabajos y sala de concierto.", "A huge section with basses and a concert hall.",
            strings (p, 7, 16, 3000, 0.3f, 1.2f); va (B (p), waveSaw, 5, 12, 0.8f, 0.5f, -1); reverb (p, 0.9f, 4.5f);
            macros (m, 50, 0, charClean, 0, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Tensión", "Trémolo rápido de arco: suspenso.", "Fast bowed tremolo: suspense.",
            strings (p, 6, 14, 3800, 0.1f, 0.6f); lfo (p, 9.0f, 0, 0, 0.55f); macros (m, 35, 0); ),
        DMB_ARCH ("Cuerdas Altas", "Violines una octava arriba, brillantes.", "Violins an octave up, bright.",
            strings (p, 6, 12, 6000, 0.15f, 0.7f, 1); macros (m, 35, 0, charClean, 0, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Graves", "Violas y cellos una octava abajo.", "Violas and cellos an octave down.",
            strings (p, 5, 12, 1800, 0.2f, 0.8f, -1); macros (m, 32, 0, charClean, 0, playPoly, 0, 15, 55); ),
        DMB_ARCH ("Cuerdas Cálidas", "Cuerdas con saturación de cinta.", "Strings with tape saturation.",
            strings (p, 6, 14, 3200, 0.2f, 0.8f); macros (m, 35, 0, charWarm, 40, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Spiccato", "Arco saltado: muy corto y rítmico.", "Bouncing bow: very short and rhythmic.",
            strings (p, 5, 12, 4000, 0.003f, 0.08f); fenv (p, 0.001f, 0.08f, 0, 0.08f); filt (p, filterLP24, 3000, 0.1f, 1.2f, 0.4f);
            amp (p, 0.003f, 0.09f, 0, 0.08f); reverb (p, 0.55f, 1.8f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Cuerdas Pulse", "Cuerdas de sintetizador con pulsos que se mueven.", "Synth strings with moving pulses.",
            va (A (p), wavePulse, 3, 10, 0.8f); pw (A (p), 0.45f); lfo (p, 0.5f, 0, 0, 0, 0.25f); filt (p, filterLP24, 3000);
            amp (p, 0.15f, 1, 1, 0.8f); chorus (p, 0.4f); reverb (p, 0.65f, 2.5f); macros (m, 30, 0, charVintage, 25); ),
        DMB_ARCH ("Cuerdas Swell", "Entran muy despacio, como una ola.", "They swell in very slowly, like a wave.",
            strings (p, 6, 14, 3500, 0.9f, 1.2f); fenv (p, 1.2f, 1, 0.9f, 1.0f); filt (p, filterLP24, 2500, 0.1f, 1.2f, 0.4f);
            macros (m, 40, 0, charClean, 0, playPoly, 0, 15, 55); ),
        DMB_ARCH ("Cuerdas Marcato", "Ataque acentuado que luego se queda.", "An accented attack that then holds.",
            strings (p, 6, 14, 3000, 0.01f, 0.4f); fenv (p, 0.001f, 0.25f, 0.4f, 0.3f); filt (p, filterLP24, 2500, 0.1f, 1.8f, 0.4f);
            amp (p, 0.01f, 0.4f, 0.7f, 0.4f); macros (m, 32, 0, charClean, 0, playPoly, 0, 12, 65); ),
        DMB_ARCH ("Cuerdas con Coro", "Cuerdas con un coro de fondo.", "Strings with a choir behind.",
            strings (p, 5, 12, 3500, 0.3f, 1.0f); vox (B (p), 2, 0.0f, 0.1f, 3, 0, 0.5f); reverb (p, 0.85f, 4.0f);
            macros (m, 45, 0, charClean, 0, playPoly, 0, 15, 55); ),
        DMB_ARCH ("Cuerdas Viejas", "Cuerdas como sampleadas en los 2000.", "Strings as if sampled in the 2000s.",
            strings (p, 6, 14, 3800, 0.15f, 0.7f); macros (m, 32, 0, charVintage, 60, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Octavas", "Cuerdas con su octava arriba.", "Strings with their octave above.",
            strings (p, 5, 12, 4000, 0.2f, 0.8f); va (B (p), waveSaw, 4, 12, 0.85f, 0.5f, 1); macros (m, 38, 0, charClean, 0, playPoly, 0, 18, 55); ),
        DMB_ARCH ("Cuerdas Terciopelo", "Cuerdas suaves y oscuras.", "Soft, dark strings.",
            strings (p, 5, 12, 2000, 0.3f, 1.0f); macros (m, 40, 0, charWarm, 15, playPoly, 0, 15, 50); ),
        DMB_ARCH ("Cuerdas Disco", "Cuerdas brillantes y rápidas con eco, para la pista.", "Bright, fast strings with echo, for the dance floor.",
            va (A (p), waveSaw, 4, 12, 0.8f); filt (p, filterLP24, 6000, 0.1f, 0.6f); fenv (p, 0.01f, 0.4f, 0.7f, 0.3f);
            amp (p, 0.02f, 0.5f, 0.8f, 0.3f); chorus (p, 0.5f); delay (p, 3, 0.3f); reverb (p, 0.6f, 2.0f); macros (m, 25, 20); ),
    };

    // =========================================================================================
    cats[catPizz] = {
        DMB_ARCH ("Pizzicato Orquesta", "Cuerdas pellizcadas en sección.", "A section of plucked strings.",
            pluck (A (p), 0.55f, 0.15f, 0.55f, 0.5f, 0.5f); pluck (B (p), 0.5f, 0.18f, 0.5f, 0.5f, 0.5f, 0.7f); B (p)[lp::fine] = 9.0f;
            filt (p, filterLP12, 6000); amp (p, 0.001f, 2, 1, 0.12f); reverb (p, 0.6f, 2.2f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Pizzicato Solo", "Un solo violín pellizcado.", "A single plucked violin.",
            pluck (A (p), 0.6f, 0.15f, 0.6f, 0.6f, 0.6f); filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f);
            reverb (p, 0.5f, 1.8f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Pizzicato Cálido", "Pizzicato redondo, con saturación suave.", "A round pizzicato with gentle saturation.",
            pluck (A (p), 0.35f, 0.2f, 0.35f, 0.6f, 0.6f); pluck (B (p), 0.35f, 0.2f, 0.35f, 0.6f, 0.6f, 0.6f); B (p)[lp::fine] = -8.0f;
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.12f); reverb (p, 0.6f, 2.2f); macros (m, 25, 0, charWarm, 30, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Pizzicato Seco", "Corto, brillante y sin sala.", "Short, bright and dry.",
            pluck (A (p), 0.75f, 0.1f, 0.7f, 0.35f, 0.4f); filt (p, filterOff, 20000); amp (p, 0.001f, 1, 1, 0.06f);
            macros (m, 8, 0, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Pizzicato Synth", "Pizzicato de sintetizador: sierra cortita.", "A synth pizzicato: a tiny saw.",
            va (A (p), waveSaw, 2, 8); filt (p, filterLP24, 1400, 0.15f, 3.0f); fenv (p, 0.001f, 0.08f, 0, 0.06f);
            amp (p, 0.001f, 0.2f, 0, 0.1f); reverb (p, 0.55f, 2.0f); macros (m, 22, 0); ),
        DMB_ARCH ("Pizzicato Grave", "Cellos y contrabajos pellizcados.", "Plucked cellos and basses.",
            pluck (A (p), 0.45f, 0.2f, 0.45f, 0.8f, 0.7f, 1.0f, -1); filt (p, filterOff, 20000); amp (p, 0.001f, 3, 1, 0.12f);
            reverb (p, 0.6f, 2.2f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Pizzicato Agudo", "Violines pellizcados una octava arriba.", "Plucked violins an octave up.",
            pluck (A (p), 0.7f, 0.12f, 0.65f, 0.35f, 0.4f, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 1.5f, 1, 0.08f);
            reverb (p, 0.55f, 2.0f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Arpa", "Arpa de concierto: cuerdas largas y suaves.", "A concert harp: long, soft strings.",
            pluck (A (p), 0.45f, 0.25f, 0.65f, 4.0f, 0.3f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 1.5f);
            reverb (p, 0.75f, 3.5f); macros (m, 35, 0, charClean, 0, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Arpa Cristal", "Arpa con un brillo de campana encima.", "A harp with a bell shine on top.",
            pluck (A (p), 0.5f, 0.25f, 0.65f, 3.5f, 0.3f); fm (B (p), 0, 1, 3.5f, 1.2f, 0.5f, 0.0f, 0, 0.25f);
            filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 1.5f); reverb (p, 0.8f, 4.0f); macros (m, 40, 0); ),
        DMB_ARCH ("Pizzicato Doble", "Pizzicato con su octava arriba.", "A pizzicato with its octave above.",
            pluck (A (p), 0.55f, 0.15f, 0.55f, 0.5f, 0.5f); pluck (B (p), 0.6f, 0.15f, 0.6f, 0.4f, 0.4f, 0.5f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f); reverb (p, 0.6f, 2.2f); macros (m, 25, 0); ),
        DMB_ARCH ("Pizzicato Cine", "Pizzicato en una sala de concierto enorme.", "A pizzicato in a huge concert hall.",
            pluck (A (p), 0.55f, 0.15f, 0.55f, 0.6f, 0.5f); pluck (B (p), 0.5f, 0.18f, 0.5f, 0.6f, 0.5f, 0.7f); B (p)[lp::fine] = 10.0f;
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.15f); reverb (p, 0.9f, 4.5f); macros (m, 45, 0); ),
        DMB_ARCH ("Pizzicato Clásico", "El pizzicato de los teclados de la época, con eco.", "The era's keyboard pizzicato, with echo.",
            pluck (A (p), 0.6f, 0.14f, 0.6f, 0.45f, 0.5f); pluck (B (p), 0.6f, 0.16f, 0.6f, 0.45f, 0.5f, 0.7f); B (p)[lp::fine] = 7.0f;
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f); delay (p, 5, 0.3f); macros (m, 22, 18, charVintage, 35); ),
        DMB_ARCH ("Pizzicato Juguete", "Pizzicato chiquito con un brillo de juguete.", "A tiny pizzicato with a toy shine.",
            pluck (A (p), 0.9f, 0.1f, 0.85f, 0.3f, 0.2f); modal (B (p), modalGlock, 0.8f, 0.6f, 0.6f, 0.5f, 0.3f, 0, 0, 0.25f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f); macros (m, 20, 0); ),
        DMB_ARCH ("Pizzicato Madera", "Oscuro y con mucho cuerpo de caja.", "Dark, with lots of body.",
            pluck (A (p), 0.25f, 0.2f, 0.25f, 0.6f, 0.8f); filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f);
            reverb (p, 0.5f, 1.8f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Pizzicato Pop", "Cuadrado cortito con cuerda: pizzicato de pop.", "A tiny square with a string: pop pizzicato.",
            va (A (p), waveSquare); pluck (B (p), 0.6f, 0.15f, 0.6f, 0.4f, 0.4f, 0.6f); filt (p, filterLP24, 2000, 0.1f, 2.5f);
            fenv (p, 0.001f, 0.06f, 0, 0.05f); amp (p, 0.001f, 0.15f, 0, 0.08f); macros (m, 18, 15); ),
        DMB_ARCH ("Pizzicato Romántico", "Pizzicato cálido con sala.", "A warm pizzicato with a room.",
            pluck (A (p), 0.45f, 0.18f, 0.5f, 0.8f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 3, 1, 0.2f);
            reverb (p, 0.75f, 3.0f); macros (m, 35, 0, charWarm, 20); ),
        DMB_ARCH ("Pizzicato Rebote", "Pizzicato con eco con puntillo que rebota.", "A pizzicato with a bouncing dotted echo.",
            pluck (A (p), 0.6f, 0.15f, 0.6f, 0.45f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.1f);
            delay (p, 5, 0.45f, 0.6f); macros (m, 20, 35); ),
        DMB_ARCH ("Pizzicato Seda", "Pizzicato suave, con ataque redondo.", "A soft pizzicato with a round attack.",
            pluck (A (p), 0.3f, 0.2f, 0.45f, 0.6f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.006f, 2, 1, 0.15f);
            reverb (p, 0.65f, 2.5f); macros (m, 28, 0, charClean, 0, playPoly, 0, 0, 55); ),
        DMB_ARCH ("Pizzicato Metal", "Pizzicato con un ataque metálico de FM.", "A pizzicato with a metallic FM attack.",
            fm (A (p), 0, 1, 3, 2.0f, 0.08f, 0.0f); pluck (B (p), 0.6f, 0.15f, 0.6f, 0.45f, 0.5f, 0.6f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 0.6f, 0, 0.15f); reverb (p, 0.55f, 2.0f); macros (m, 22, 10); ),
        DMB_ARCH ("Pizzicato Ensamble", "Muchas cuerdas pellizcadas con chorus.", "Many plucked strings with chorus.",
            pluck (A (p), 0.55f, 0.15f, 0.55f, 0.5f, 0.5f); pluck (B (p), 0.55f, 0.17f, 0.55f, 0.5f, 0.5f, 0.8f); B (p)[lp::fine] = 12.0f;
            filt (p, filterOff, 20000); amp (p, 0.001f, 2, 1, 0.12f); chorus (p, 0.4f); reverb (p, 0.65f, 2.5f); macros (m, 28, 0); ),
    };

    // =========================================================================================
    cats[catBrass] = {
        DMB_ARCH ("Metales Stab", "Golpe de metales corto, para acentos.", "A short brass stab, for accents.",
            brass (p, 4, 700, 3.2f, 0.005f, 0.25f, 0.35f, 0.15f); amp (p, 0.005f, 0.5f, 0.6f, 0.15f);
            macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Metales Sección", "Sección de metales que abre al soplar.", "A brass section that opens as it blows.",
            brass (p, 5, 1100, 2.4f, 0.06f, 0.5f, 0.6f, 0.25f); macros (m, 25, 0, charClean, 0, playPoly, 0, 15, 65); ),
        DMB_ARCH ("Trompeta Solo", "Una trompeta: entra desde abajo y canta con vibrato.", "A trumpet: scoops in and sings with vibrato.",
            va (A (p), waveSaw); osc2 (A (p), waveSquare, 0, 3, 0.2f); filt (p, filterLP24, 1200, 0.2f, 2.4f, 0.7f);
            fenv (p, 0.03f, 0.3f, 0.6f, 0.15f); amp (p, 0.02f, 1, 1, 0.12f); pitchEnv (p, -1.2f, 0.04f); noise (p, 0.02f, 0.8f, 1.5f);
            vib (p, 5.6f, 0.35f); reverb (p, 0.6f, 2.2f); macros (m, 25, 0, charClean, 0, playLegato, 6, 30, 70); ),
        DMB_ARCH ("Trombón", "Trombón grave con portamento.", "A low trombone with glide.",
            va (A (p), waveSaw, 2, 5, 0.3f, 1.0f, -1); filt (p, filterLP24, 700, 0.15f, 2.0f, 0.6f); fenv (p, 0.05f, 0.4f, 0.6f, 0.2f);
            amp (p, 0.04f, 1, 1, 0.15f); pitchEnv (p, -1.0f, 0.06f); vib (p, 5.0f, 0.4f); reverb (p, 0.6f, 2.2f);
            macros (m, 25, 0, charClean, 0, playLegato, 15, 22, 65); ),
        DMB_ARCH ("Metales 80s", "Metales de sintetizador con chorus.", "Synth brass with chorus.",
            va (A (p), waveSaw, 3, 10, 0.8f); osc2 (A (p), wavePulse, 0, 7, 0.4f); filt (p, filterLP24, 900, 0.2f, 2.8f, 0.6f);
            fenv (p, 0.04f, 0.4f, 0.5f, 0.3f); amp (p, 0.03f, 1, 1, 0.25f); chorus (p, 0.4f); reverb (p, 0.55f, 2.0f);
            macros (m, 22, 0, charVintage, 25); ),
        DMB_ARCH ("Metales FM", "Metales digitales de FM, con 'blat' al principio.", "Digital FM brass, with a 'blat' at the start.",
            fm (A (p), 0, 1, 1, 3.2f, 0.25f, 0.55f, 0.35f); filt (p, filterLP12, 5000); amp (p, 0.03f, 1, 1, 0.15f); vib (p, 5.5f, 0.4f);
            reverb (p, 0.55f, 2.0f); macros (m, 22, 0, charClean, 0, playPoly, 0, 12); ),
        DMB_ARCH ("Corno Francés", "Corno suave y oscuro, lejos.", "A soft, dark French horn, far away.",
            va (A (p), waveSaw, 2, 4, 0.4f); filt (p, filterLP24, 600, 0.1f, 1.4f, 0.6f); fenv (p, 0.08f, 0.5f, 0.6f, 0.3f);
            amp (p, 0.08f, 1, 1, 0.3f); vib (p, 5.0f, 0.5f); reverb (p, 0.8f, 3.5f); macros (m, 40, 0, charWarm, 15, playPoly, 0, 12, 60); ),
        DMB_ARCH ("Trompetas Fiesta", "Trompetas brillantes con vibrato, de fiesta.", "Bright party trumpets with vibrato.",
            va (A (p), waveSaw, 3, 8); filt (p, filterLP24, 1500, 0.2f, 2.6f, 0.6f); fenv (p, 0.02f, 0.3f, 0.7f, 0.15f);
            amp (p, 0.015f, 1, 1, 0.12f); vib (p, 6.0f, 0.25f); reverb (p, 0.55f, 2.0f); macros (m, 22, 10, charClean, 0, playPoly, 0, 30, 70); ),
        DMB_ARCH ("Saxofón Synth", "Un saxofón de sintetizador: nasal, con aire y vibrato.", "A synth sax: nasal, breathy, with vibrato.",
            va (A (p), wavePulse); pw (A (p), 0.3f); osc2 (A (p), waveSaw, 0, 4, 0.4f); filt (p, filterLP24, 1600, 0.35f, 1.6f, 0.6f);
            fenv (p, 0.03f, 0.4f, 0.5f, 0.15f); amp (p, 0.03f, 1, 1, 0.12f); noise (p, 0.05f, 0.7f, 2.0f); vib (p, 5.2f, 0.3f);
            reverb (p, 0.55f, 2.0f); macros (m, 25, 10, charWarm, 30, playLegato, 8, 28, 70); ),
        DMB_ARCH ("Metales Swell", "Metales que crecen despacio.", "Brass that swells slowly.",
            brass (p, 5, 900, 2.0f, 0.7f, 1.0f, 0.8f, 0.5f); amp (p, 0.6f, 1, 1, 0.5f); macros (m, 30, 0); ),
        DMB_ARCH ("Metales Golpe", "Golpe de metales muy corto y seco.", "A very short, dry brass hit.",
            brass (p, 4, 900, 3.0f, 0.001f, 0.12f, 0.0f, 0.1f); amp (p, 0.003f, 0.18f, 0, 0.1f); macros (m, 15, 0, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Metales Graves", "Sección de metales una octava abajo.", "A brass section an octave down.",
            brass (p, 4, 700, 2.4f, 0.06f, 0.5f, 0.6f, 0.25f, -1); macros (m, 25, 0); ),
        DMB_ARCH ("Tuba Synth", "Tuba de sintetizador, gorda y grave.", "A synth tuba, fat and low.",
            va (A (p), waveSquare, 1, 0, 0.3f, 1.0f, -1); filt (p, filterLP24, 400, 0.1f, 1.5f, 0.5f); fenv (p, 0.04f, 0.4f, 0.6f, 0.2f);
            amp (p, 0.04f, 1, 1, 0.15f); macros (m, 15, 0, charWarm, 20, playLegato, 8); ),
        DMB_ARCH ("Metales Sucios", "Metales saturados, con mucha actitud.", "Saturated brass with attitude.",
            brass (p, 5, 1100, 2.4f, 0.04f, 0.4f, 0.6f, 0.2f); p[gp::fDrive] = 0.6f; macros (m, 20, 0, charWarm, 50); ),
        DMB_ARCH ("Fanfarria", "Sección grande con su octava: para entradas.", "A big section with its octave: for entrances.",
            brass (p, 6, 1800, 2.2f, 0.03f, 0.5f, 0.7f, 0.3f); va (B (p), waveSaw, 3, 10, 0.8f, 0.4f, 1); reverb (p, 0.8f, 3.0f);
            macros (m, 35, 0, charClean, 0, playPoly, 0, 15, 70); ),
        DMB_ARCH ("Metales Callejeros", "Golpe de metales pasado por casete.", "A brass stab run through a cassette.",
            brass (p, 4, 800, 3.0f, 0.005f, 0.3f, 0.4f, 0.15f); amp (p, 0.005f, 0.5f, 0.6f, 0.15f); macros (m, 15, 10, charLofi, 55); ),
        DMB_ARCH ("Metales Bend", "Metales que entran desde abajo, con estilo.", "Brass that scoops in from below, with style.",
            brass (p, 4, 1000, 2.4f, 0.03f, 0.4f, 0.6f, 0.2f); pitchEnv (p, -3.0f, 0.08f); macros (m, 22, 0); ),
        DMB_ARCH ("Metales Algodón", "Sección suave y oscura.", "A soft, dark section.",
            brass (p, 4, 600, 1.4f, 0.1f, 0.6f, 0.7f, 0.35f); macros (m, 30, 0, charWarm, 15, playPoly, 0, 12, 55); ),
        DMB_ARCH ("Metales Octavas", "Sección de metales con su octava arriba.", "A brass section with its octave above.",
            brass (p, 4, 1100, 2.4f, 0.04f, 0.4f, 0.6f, 0.2f); va (B (p), waveSaw, 3, 10, 0.8f, 0.5f, 1); macros (m, 25, 0); ),
        DMB_ARCH ("Metales Sintéticos", "Metales hechos con cuadrados: más electrónicos.", "Brass made of squares: more electronic.",
            va (A (p), waveSquare, 3, 10); filt (p, filterLP24, 900, 0.2f, 2.5f, 0.6f); fenv (p, 0.02f, 0.35f, 0.5f, 0.2f);
            amp (p, 0.01f, 1, 1, 0.15f); reverb (p, 0.55f, 2.0f); macros (m, 22, 10); ),
    };

    // =========================================================================================
    cats[catHit] = {
        DMB_ARCH ("Orch Hit Clásico", "El golpe de orquesta de toda la vida.", "The classic orchestra hit.",
            hit (p, 12, -12); macros (m, 35, 0, charVintage, 35, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Orch Hit Mayor", "Golpe de orquesta en acorde mayor.", "An orchestra hit as a major chord.",
            hit (p, 4, 7); macros (m, 35, 0, charVintage, 30, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Orch Hit Menor", "Golpe de orquesta en acorde menor: dramático.", "An orchestra hit as a minor chord: dramatic.",
            hit (p, 3, 7); macros (m, 35, 0, charVintage, 30, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Orch Hit 8 Bits", "Golpe de videojuego: cuadrados y pocos bits.", "A video-game hit: squares and few bits.",
            va (A (p), waveSquare); osc2 (A (p), waveSquare, 12, 0, 0.6f); va (B (p), wavePulse, 1, 0, 0.5f, 0.7f, -1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 0.5f, 0, 0.3f); pitchEnv (p, 2.0f, 0.04f); reverb (p, 0.6f, 2.0f);
            macros (m, 25, 0, charLofi, 70); ),
        DMB_ARCH ("Orch Hit Cine", "Golpe enorme con una octava abajo y sala gigante.", "A huge hit with an octave below and a giant hall.",
            hit (p, 12, -12); B (p)[lp::octave] = -1.0f; reverb (p, 0.95f, 5.0f); amp (p, 0.001f, 1.2f, 0, 0.8f); macros (m, 50, 0); ),
        DMB_ARCH ("Orch Hit Seco", "Golpe corto y seco, sin cola.", "A short, dry hit with no tail.",
            hit (p, 12, -12); amp (p, 0.001f, 0.35f, 0, 0.12f); macros (m, 5, 0, charVintage, 30); ),
        DMB_ARCH ("Orch Hit Coro", "Golpe de orquesta con coro.", "An orchestra hit with choir.",
            hit (p, 12, 0); vox (B (p), 1, 0.0f, 0.1f, 4, 0, 0.8f); macros (m, 40, 0, charVintage, 25); ),
        DMB_ARCH ("Orch Hit Sucio", "Golpe saturado y agresivo.", "A saturated, aggressive hit.",
            hit (p, 12, -12); p[gp::fDrive] = 0.7f; macros (m, 30, 0, charWarm, 60); ),
        DMB_ARCH ("Orch Hit Vidrio", "Golpe con una campana de FM encima.", "A hit with an FM bell on top.",
            hit (p, 12, 0); fm (B (p), 0, 1, 3.5f, 2.0f, 0.4f, 0.0f, 0, 0.5f); macros (m, 35, 0, charVintage, 25); ),
        DMB_ARCH ("Orch Hit Grave", "Golpe una octava abajo: pesado.", "A hit an octave down: heavy.",
            hit (p, 12, -12); A (p)[lp::octave] = -1.0f; macros (m, 35, 0, charVintage, 30); ),
        DMB_ARCH ("Orch Hit Doble", "Golpe en dos octavas.", "A hit in two octaves.",
            hit (p, 12, 12); macros (m, 35, 0, charVintage, 30); ),
        DMB_ARCH ("Orch Hit Quinta", "Golpe con la quinta: sin mayor ni menor.", "A hit with the fifth: neither major nor minor.",
            hit (p, 7, -12); macros (m, 35, 0, charVintage, 30); ),
        DMB_ARCH ("Orch Hit Metales", "Golpe con más metales: el filtro abre de golpe.", "A brassier hit: the filter snaps open.",
            hit (p, 12, -12); filt (p, filterLP24, 900, 0.15f, 3.0f, 0.5f); macros (m, 30, 0, charVintage, 25); ),
        DMB_ARCH ("Orch Hit Cuerdas", "Golpe con más cuerdas: más suave y brillante.", "A stringier hit: softer and brighter.",
            hit (p, 12, -12); filt (p, filterLP24, 5000, 0.1f, 0.5f, 0.4f); amp (p, 0.008f, 0.9f, 0, 0.5f); macros (m, 38, 0); ),
        DMB_ARCH ("Orch Hit Timbal", "Golpe con un timbal grave que retumba.", "A hit with a booming low timpani.",
            hit (p, 12, 0); va (B (p), waveSine, 1, 0, 0.3f, 0.9f, -1); pitchEnv (p, 4.0f, 0.08f); macros (m, 38, 0, charVintage, 25); ),
        DMB_ARCH ("Orch Hit Láser", "Golpe que cae de tono como un láser.", "A hit that drops in pitch like a laser.",
            hit (p, 12, -12); pitchEnv (p, 12.0f, 0.08f); macros (m, 30, 0, charVintage, 35); ),
        DMB_ARCH ("Orch Hit Retro", "Golpe digital de FM de teclado viejo.", "A digital FM hit from an old keyboard.",
            fm (A (p), 0, 1, 1, 4.0f, 0.3f, 0.2f, 0.3f); va (B (p), waveSaw, 3, 10, 0.7f, 0.6f, -1); filt (p, filterLP24, 4000, 0.1f, 1.0f);
            fenv (p, 0.001f, 0.3f, 0.2f, 0.3f); amp (p, 0.001f, 0.7f, 0, 0.4f); noise (p, 0.0f, 0.6f, 0.8f, 0.6f, 0.05f);
            reverb (p, 0.8f, 3.0f); macros (m, 32, 0, charVintage, 50); ),
        DMB_ARCH ("Orch Hit Suspendido", "Golpe en acorde suspendido: queda en el aire.", "A hit as a suspended chord: it hangs in the air.",
            hit (p, 5, 7); macros (m, 35, 0, charVintage, 30); ),
        DMB_ARCH ("Orch Hit Disco", "Golpe con eco y chorus, para la pista.", "A hit with echo and chorus, for the dance floor.",
            hit (p, 12, -12); chorus (p, 0.35f); delay (p, 3, 0.35f); macros (m, 30, 30, charVintage, 25); ),
        DMB_ARCH ("Orch Hit Épico", "Golpe grande con octava abajo y cola larga.", "A big hit with an octave below and a long tail.",
            hit (p, 12, -12); B (p)[lp::octave] = -1.0f; amp (p, 0.001f, 1.5f, 0, 1.0f); reverb (p, 0.95f, 6.0f); macros (m, 55, 10); ),
    };
}
} // namespace dmb
