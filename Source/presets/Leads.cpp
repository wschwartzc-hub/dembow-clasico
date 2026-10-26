// Sonidos base: LEAD, PLUCK, BAJO y PAD
#include "Build.h"

namespace dmb
{
using namespace build;

void addLeadsPlucksBassPads (std::vector<Archetype> (&cats)[numCategories])
{
    // =========================================================================================
    // LEAD: melodías al frente, casi todas monofónicas con un poco de portamento
    cats[catLead] = {
        DMB_ARCH ("Lead Marquesina", "El lead cuadrado clásico de las marquesinas: con portamento corto y eco.",
                  "The classic square lead of the old-school era: short glide and echo.",
            va (A (p), waveSquare, 2, 8, 0.4f); osc2 (A (p), waveSquare, 12, 4, 0.25f);
            filt (p, filterLP24, 2600, 0.15f, 1.5f, 0.6f); fenv (p, 0.001f, 0.35f, 0.5f, 0.3f); amp (p, 0.003f, 1, 1, 0.12f);
            vib (p, 5.5f, 0.35f); delay (p, 5, 0.3f); macros (m, 14, 22, charVintage, 30, playLegato, 14, 22, 50); ),
        DMB_ARCH ("Lead Perreo", "Diente de sierra con un cuadrado abajo: brillante y directo para la melodía principal.",
                  "Saw with a square underneath: bright and direct for the main hook.",
            va (A (p), waveSaw, 3, 14, 0.5f); osc2 (A (p), waveSquare, -12, 0, 0.35f);
            filt (p, filterLP24, 4200, 0.2f, 1.0f); fenv (p, 0.001f, 0.25f, 0.6f, 0.2f); amp (p, 0.002f, 1, 1, 0.12f);
            macros (m, 12, 20, charClean, 10, playLegato, 10, 15); ),
        DMB_ARCH ("Lead Caserío", "Pulso delgado con vibrato y eco con puntillo, muy de mixtape.",
                  "Thin pulse with vibrato and a dotted echo, very mixtape.",
            va (A (p), wavePulse); pw (A (p), 0.25f); filt (p, filterLP12, 3500, 0.25f, 1.2f);
            fenv (p, 0.001f, 0.3f, 0.4f, 0.2f); amp (p, 0.004f, 1, 1, 0.15f); vib (p, 5.8f, 0.25f); delay (p, 5, 0.35f);
            macros (m, 18, 28, charVintage, 25, playLegato, 18, 30); ),
        DMB_ARCH ("Silbato del Barrio", "Un silbido: casi una senoidal pura con aire y portamento largo.",
                  "A whistle: almost a pure sine with air and a long glide.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.08f); filt (p, filterOff, 20000);
            amp (p, 0.02f, 1, 1, 0.12f); noise (p, 0.04f, 0.6f, 6.0f, 0.3f, 0.04f); vib (p, 6.2f, 0.15f);
            macros (m, 22, 25, charClean, 0, playLegato, 30, 45, 40); ),
        DMB_ARCH ("Lead Hoover Viejo", "Siete sierras desafinadas que entran desde abajo: el lead gordo de las discotecas.",
                  "Seven detuned saws scooping in from below: the fat club lead.",
            va (A (p), waveSaw, 7, 35, 0.8f); osc2 (A (p), wavePulse, -12, 0, 0.4f); pw (A (p), 0.3f);
            lfo (p, 0.6f, 0, 0, 0, 0.2f); pitchEnv (p, -3, 0.12f); filt (p, filterLP24, 3000, 0.25f, 1.0f);
            fenv (p, 0.001f, 0.5f, 0.6f, 0.3f); amp (p, 0.003f, 1, 1, 0.2f); macros (m, 15, 0, charClean, 25, playLegato, 22, 0); ),
        DMB_ARCH ("Square Mixtape", "Cuadrado limpio y redondo, el lead más usado del 2004.",
                  "Clean, round square, the most-used lead of 2004.",
            va (A (p), waveSquare); filt (p, filterLP24, 5000, 0.05f, 0.5f); amp (p, 0.002f, 1, 1, 0.08f);
            macros (m, 10, 15, charClean, 0, playLegato, 8, 15); ),
        DMB_ARCH ("Lead Trompetica", "Una trompeta de sintetizador: abre el filtro al soplar y entra un poquito desafinada.",
                  "A synth trumpet: the filter opens as it blows and it scoops in slightly flat.",
            va (A (p), waveSaw, 2, 6); filt (p, filterLP24, 900, 0.15f, 2.6f, 0.7f); fenv (p, 0.04f, 0.4f, 0.55f, 0.15f);
            amp (p, 0.02f, 1, 1, 0.1f); pitchEnv (p, -1.5f, 0.05f); vib (p, 5.5f, 0.4f);
            macros (m, 15, 15, charWarm, 20, playLegato, 12, 30); ),
        DMB_ARCH ("Saw Underground", "Dos sierras crudas con resonancia y un poco de distorsión.",
                  "Two raw saws with resonance and a bit of grit.",
            va (A (p), waveSaw); osc2 (A (p), waveSaw, 0, 9, 0.6f); filt (p, filterLP24, 1800, 0.45f, 2.0f, 0.5f, 0.3f);
            fenv (p, 0.001f, 0.22f, 0.35f, 0.2f); amp (p, 0.002f, 1, 1, 0.12f); macros (m, 10, 20, charClean, 20, playLegato, 10, 10); ),
        DMB_ARCH ("Lead Chiptune", "Pulso muy delgado de videojuego, sin filtro y con vibrato rápido.",
                  "Very thin video-game pulse, unfiltered, with fast vibrato.",
            va (A (p), wavePulse); pw (A (p), 0.125f); filt (p, filterOff, 20000); amp (p, 0.001f, 1, 1, 0.05f); vib (p, 7.5f, 0.12f);
            macros (m, 8, 18, charLofi, 25, playMono, 0, 35); ),
        DMB_ARCH ("Lead Sintonía", "Lead digital de FM, con brillo de teclado de los 2000.",
                  "Digital FM lead with the shine of a 2000s keyboard.",
            fm (A (p), 1, 1, 2, 2.2f, 0.8f, 0.5f, 0.25f); fm2 (A (p), 1, 3, 0.8f, 0.0f); filt (p, filterLP12, 7000);
            amp (p, 0.003f, 1, 1, 0.12f); macros (m, 14, 20, charClean, 0, playLegato, 12, 20); ),
        DMB_ARCH ("Lead Disco Duro", "Ataque metálico de FM que se suaviza, con una sierra de fondo.",
                  "Metallic FM attack that mellows out, with a saw underneath.",
            fm (A (p), 0, 1, 1, 5.0f, 0.25f, 0.25f, 0.5f); va (B (p), waveSaw, 1, 10, 0.5f, 0.4f);
            filt (p, filterLP24, 6000, 0.1f); amp (p, 0.002f, 1, 1, 0.12f); macros (m, 12, 18, charClean, 0, playLegato, 10, 15); ),
        DMB_ARCH ("Supersaw Bellaqueo", "Muro de sierras anchas; sirve para melodía y para acordes.",
                  "A wall of wide saws; works for melodies and chords.",
            va (A (p), waveSaw, 7, 22, 0.85f); filt (p, filterLP24, 6500, 0.1f, 0.6f); fenv (p, 0.001f, 0.4f, 0.7f, 0.3f);
            amp (p, 0.003f, 1, 1, 0.18f); chorus (p, 0.2f); macros (m, 20, 20); ),
        DMB_ARCH ("Lead Ocarina Synth", "Triángulo con soplido: un lead suave que suena a ocarina electrónica.",
                  "Triangle with breath: a soft lead like an electronic ocarina.",
            va (A (p), waveTri); osc2 (A (p), waveSine, 12, 0, 0.12f); noise (p, 0.05f, 0.45f, 4.0f, 0.4f, 0.05f);
            filt (p, filterLP12, 4000); amp (p, 0.03f, 1, 1, 0.15f); vib (p, 5.2f, 0.4f);
            macros (m, 22, 25, charClean, 0, playLegato, 16, 40, 45); ),
        DMB_ARCH ("Lead Corneta", "Cuadrado con sierra que entra desde abajo, como una corneta de sintetizador.",
                  "Square and saw scooping up like a synth cornet.",
            va (A (p), waveSquare, 2, 6); osc2 (A (p), waveSaw, 0, 5, 0.3f); pitchEnv (p, -2.0f, 0.07f);
            filt (p, filterLP24, 1400, 0.2f, 2.2f); fenv (p, 0.01f, 0.3f, 0.5f, 0.15f); amp (p, 0.01f, 1, 1, 0.1f);
            macros (m, 14, 15, charVintage, 35, playLegato, 10, 25); ),
        DMB_ARCH ("Lead Gata", "Pulso que se mueve solo (PWM) y se abre a los lados.",
                  "A self-moving pulse (PWM) that spreads out to the sides.",
            va (A (p), wavePulse, 3, 12, 0.7f); pw (A (p), 0.4f); lfo (p, 0.8f, 0, 0, 0, 0.3f);
            filt (p, filterLP24, 3200, 0.15f, 1.0f); amp (p, 0.003f, 1, 1, 0.15f); chorus (p, 0.25f);
            macros (m, 15, 20, charClean, 0, playLegato, 12, 15); ),
        DMB_ARCH ("Lead Calentura", "Sierra caliente con sub y saturación, oscura y gruesa.",
                  "Hot saw with sub and saturation, dark and thick.",
            va (A (p), waveSaw); sub (A (p), 0.5f); filt (p, filterLP12, 1600, 0.3f, 1.8f, 0.5f, 0.45f);
            fenv (p, 0.001f, 0.3f, 0.5f, 0.2f); amp (p, 0.002f, 1, 1, 0.12f); macros (m, 10, 15, charWarm, 55, playLegato, 10, 10); ),
        DMB_ARCH ("Lead Sirena", "Senoidal con vibrato ancho y portamento largo: suena a sirena.",
                  "Sine with wide vibrato and a long glide: sounds like a siren.",
            va (A (p), waveSine); osc2 (A (p), waveSaw, 12, 0, 0.05f); filt (p, filterOff, 20000); amp (p, 0.01f, 1, 1, 0.15f);
            vib (p, 7.0f, 0.0f); macros (m, 18, 30, charClean, 0, playLegato, 45, 70); ),
        DMB_ARCH ("Lead Romántico", "Triángulo suave con una octava arriba; para el reggaetón romántico.",
                  "Soft triangle with an octave above; for romantic reggaeton.",
            va (A (p), waveTri, 2, 5); osc2 (A (p), waveSine, 12, 0, 0.3f); filt (p, filterLP12, 3000);
            amp (p, 0.06f, 1, 1, 0.4f); vib (p, 5.0f, 0.5f); macros (m, 30, 25, charWarm, 25, playLegato, 18, 30, 50); ),
        DMB_ARCH ("Lead Discoteca", "Cuadrado ancho con una sierra una octava arriba y chorus.",
                  "Wide square with a saw an octave up and chorus.",
            va (A (p), waveSquare, 3, 15, 0.6f); osc2 (A (p), waveSaw, 12, 7, 0.4f); filt (p, filterLP24, 5000, 0.1f, 0.8f);
            amp (p, 0.002f, 1, 1, 0.15f); chorus (p, 0.35f, 0.8f); macros (m, 20, 25, charClean, 0, playLegato, 8, 15); ),
        DMB_ARCH ("Lead Malianteo", "Sierra oscura y sucia, con filtro resonante.",
                  "Dark, dirty saw with a resonant filter.",
            va (A (p), waveSaw, 3, 20, 0.4f); filt (p, filterLP24, 1300, 0.35f, 1.5f, 0.5f, 0.5f);
            fenv (p, 0.001f, 0.4f, 0.4f, 0.2f); amp (p, 0.002f, 1, 1, 0.12f); macros (m, 10, 15, charLofi, 35, playLegato, 12, 10); ),
    };

    // =========================================================================================
    // PLUCK: notas cortas para arpegios y riffs
    cats[catPluck] = {
        DMB_ARCH ("Pluck Dembow", "Sierra corta que se cierra rápido: el pluck de todos los días.",
                  "A short saw that closes fast: the everyday pluck.",
            va (A (p), waveSaw, 2, 8); filt (p, filterLP24, 500, 0.25f, 4.5f, 0.7f); fenv (p, 0.001f, 0.16f, 0, 0.15f);
            amp (p, 0.001f, 0.45f, 0, 0.2f); delay (p, 5, 0.32f); macros (m, 15, 25); ),
        DMB_ARCH ("Pluck Marquesina", "Cuadrado con su octava, el pluck de los arpegios del 2004.",
                  "Square with its octave, the arpeggio pluck of 2004.",
            va (A (p), waveSquare); osc2 (A (p), waveSquare, 12, 4, 0.35f); filt (p, filterLP24, 700, 0.3f, 4.0f);
            fenv (p, 0.001f, 0.2f, 0, 0.15f); amp (p, 0.001f, 0.5f, 0, 0.2f); macros (m, 15, 25, charVintage, 35); ),
        DMB_ARCH ("Pluck Caserío", "Cuerda pulsada brillante y corta, entre guitarra y sintetizador.",
                  "Bright, short plucked string, halfway between guitar and synth.",
            pluck (A (p), 0.85f, 0.12f, 0.8f, 0.9f); filt (p, filterLP12, 7000); amp (p, 0.001f, 2, 1, 0.2f); macros (m, 18, 25); ),
        DMB_ARCH ("Pluck Supersaw", "Siete sierras que suenan un instante: brillante y ancho.",
                  "Seven saws for an instant: bright and wide.",
            va (A (p), waveSaw, 7, 20, 0.85f); filt (p, filterLP24, 900, 0.15f, 4.0f); fenv (p, 0.001f, 0.22f, 0.05f, 0.2f);
            amp (p, 0.001f, 0.6f, 0, 0.25f); macros (m, 20, 25); ),
        DMB_ARCH ("Pluck Cristal", "FM de vidrio, corto y brillante.", "Glassy FM, short and bright.",
            fm (A (p), 0, 1, 3.5f, 3.0f, 0.15f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.8f, 0, 0.4f); macros (m, 25, 25); ),
        DMB_ARCH ("Pluck Kalimba Synth", "FM redondo con un brillo de campanita al principio.",
                  "Round FM with a little bell sparkle at the start.",
            fm (A (p), 0, 1, 2, 2.0f, 0.08f, 0.05f); fm2 (A (p), 1, 7, 1.0f, 0.25f, 0.05f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 0.6f, 0, 0.3f); macros (m, 20, 20); ),
        DMB_ARCH ("Pluck Gota", "Senoidal que cae de tono, como una gota de agua.", "A sine that drops in pitch, like a water drop.",
            va (A (p), waveSine); pitchEnv (p, 12, 0.03f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.25f, 0, 0.15f); macros (m, 25, 30); ),
        DMB_ARCH ("Pluck Ácido", "Sierra con mucha resonancia que se cierra de golpe.", "A highly resonant saw that snaps shut.",
            va (A (p), waveSaw); filt (p, filterLP24, 350, 0.65f, 5.0f); fenv (p, 0.001f, 0.15f, 0, 0.1f);
            amp (p, 0.001f, 0.35f, 0, 0.15f); macros (m, 12, 25); ),
        DMB_ARCH ("Pluck Arpegio", "Pulso cortito con eco en corcheas, hecho para arpegios rápidos.",
                  "A tiny pulse with eighth-note echo, made for fast arpeggios.",
            va (A (p), wavePulse); pw (A (p), 0.3f); filt (p, filterLP24, 1200, 0.2f, 3.0f); fenv (p, 0.001f, 0.12f, 0, 0.1f);
            amp (p, 0.001f, 0.25f, 0, 0.12f); delay (p, 3, 0.35f); macros (m, 15, 30); ),
        DMB_ARCH ("Pluck Madera", "Cuerda apagada y oscura, suena a madera.", "A muted, dark string that sounds woody.",
            pluck (A (p), 0.3f, 0.2f, 0.25f, 0.5f, 0.5f); filt (p, filterLP12, 4000); amp (p, 0.001f, 1, 1, 0.12f); macros (m, 15, 15); ),
        DMB_ARCH ("Pluck Calle", "Cuadrado corto pasado por casete.", "A short square run through a cassette.",
            va (A (p), waveSquare); filt (p, filterLP24, 900, 0.2f, 3.0f); fenv (p, 0.001f, 0.18f, 0, 0.15f);
            amp (p, 0.001f, 0.4f, 0, 0.15f); macros (m, 12, 20, charLofi, 50); ),
        DMB_ARCH ("Pluck Chicle", "FM con retroalimentación: elástico, como de goma.", "FM with feedback: elastic, rubbery.",
            fm (A (p), 0, 1, 1, 3.5f, 0.12f, 0.0f, 0.5f); filt (p, filterLP12, 6000); amp (p, 0.001f, 0.4f, 0, 0.15f); macros (m, 15, 20); ),
        DMB_ARCH ("Pluck Koto Callejero", "Cuerda muy brillante pulsada cerca del puente, con un pequeño quiebre de tono.",
                  "Very bright string plucked near the bridge, with a small pitch bend.",
            pluck (A (p), 0.9f, 0.06f, 0.7f, 1.8f); pitchEnv (p, 0.6f, 0.03f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 3, 1, 0.3f); macros (m, 22, 20); ),
        DMB_ARCH ("Pluck Plástico", "Triángulo corto con un toque de cuadrado: juguetón.", "A short triangle with a touch of square: playful.",
            va (A (p), waveTri); osc2 (A (p), waveSquare, 12, 0, 0.15f); filt (p, filterLP12, 2500, 0.1f, 2.0f);
            fenv (p, 0.001f, 0.12f, 0, 0.1f); amp (p, 0.001f, 0.3f, 0, 0.12f); macros (m, 15, 20); ),
        DMB_ARCH ("Pluck Doble", "Sierra con su quinta: un pluck con mucho cuerpo.", "Saw with its fifth: a pluck with lots of body.",
            va (A (p), waveSaw, 2, 10); osc2 (A (p), waveSquare, 7, 0, 0.6f); filt (p, filterLP24, 600, 0.25f, 4.0f);
            fenv (p, 0.001f, 0.2f, 0, 0.15f); amp (p, 0.001f, 0.5f, 0, 0.2f); macros (m, 15, 22); ),
        DMB_ARCH ("Pluck Campanita", "FM con un brillo alto, entre pluck y campana.", "FM with a high shine, between pluck and bell.",
            fm (A (p), 0, 1, 7, 1.6f, 0.12f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.001f, 1.0f, 0, 0.5f); macros (m, 25, 22); ),
        DMB_ARCH ("Pluck Esquina", "Sierras desafinadas cortas con chorus.", "Short detuned saws with chorus.",
            va (A (p), waveSaw, 3, 18, 0.7f); filt (p, filterLP24, 800, 0.2f, 3.5f); fenv (p, 0.001f, 0.25f, 0, 0.2f);
            amp (p, 0.001f, 0.55f, 0, 0.25f); chorus (p, 0.4f); macros (m, 18, 20); ),
        DMB_ARCH ("Pluck Rebote", "Pluck con eco que rebota de lado a lado.", "A pluck with an echo bouncing side to side.",
            va (A (p), waveSquare); osc2 (A (p), waveSaw, 0, 7, 0.5f); filt (p, filterLP24, 1000, 0.3f, 3.0f);
            fenv (p, 0.001f, 0.15f, 0, 0.12f); amp (p, 0.001f, 0.35f, 0, 0.15f); delay (p, 5, 0.45f, 0.6f, true); macros (m, 15, 40); ),
        DMB_ARCH ("Pluck Luna", "Senoidal y triángulo suaves con mucha sala.", "Soft sine and triangle with lots of room.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.3f); filt (p, filterLP12, 3000, 0, 1.0f);
            fenv (p, 0.001f, 0.3f, 0, 0.3f); amp (p, 0.001f, 0.8f, 0, 0.6f); macros (m, 40, 25, charWarm, 20); ),
        DMB_ARCH ("Pluck Metálico", "FM inarmónico: suena a metal golpeado.", "Inharmonic FM: sounds like struck metal.",
            fm (A (p), 0, 1, 1.41f, 2.5f, 0.2f, 0.0f, 0.2f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.6f, 0, 0.3f); macros (m, 20, 20); ),
    };

    // =========================================================================================
    // BAJO: secos y al centro (sin reverb), casi todos monofónicos
    cats[catBass] = {
        DMB_ARCH ("Bajo Dembow", "Senoidal con un golpecito al principio: el bajo que va con el dembow.",
                  "A sine with a little click at the start: the bass that rides the dembow.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.15f); pitchEnv (p, 10, 0.012f);
            filt (p, filterLP12, 900, 0, 1.5f, 0.3f); fenv (p, 0.001f, 0.08f, 0, 0.1f); amp (p, 0.002f, 1.2f, 0.75f, 0.12f);
            eq (p, 28, 20000); macros (m, 0, 0, charClean, 10, playLegato, 4, 0, 50); ),
        DMB_ARCH ("Bajo 808 Clásico", "Senoidal que cae de tono y dura, con un poco de saturación.",
                  "A long sine that drops in pitch, with a bit of saturation.",
            va (A (p), waveSine); pitchEnv (p, 5, 0.06f); filt (p, filterOff, 20000); amp (p, 0.001f, 2.2f, 0, 0.3f);
            eq (p, 25, 20000); macros (m, 0, 0, charClean, 35, playMono, 0, 0, 45); ),
        DMB_ARCH ("Bajo Marquesina", "Cuadrado filtrado con sub: redondo y con ataque.", "A filtered square with sub: round with some bite.",
            va (A (p), waveSquare); sub (A (p), 0.2f); filt (p, filterLP24, 280, 0.2f, 2.8f, 0.4f);
            fenv (p, 0.001f, 0.18f, 0.15f, 0.1f); amp (p, 0.002f, 1, 0.8f, 0.08f); eq (p, 28, 20000);
            macros (m, 0, 0, charVintage, 25, playLegato, 6); ),
        DMB_ARCH ("Bajo Saw Callejero", "Sierra filtrada con saturación de cinta.", "A filtered saw with tape saturation.",
            va (A (p), waveSaw); filt (p, filterLP24, 380, 0.3f, 2.4f, 0.4f, 0.3f); fenv (p, 0.001f, 0.2f, 0.2f, 0.1f);
            amp (p, 0.002f, 1, 0.85f, 0.08f); eq (p, 28, 20000); macros (m, 0, 0, charWarm, 40, playLegato, 6); ),
        DMB_ARCH ("Bajo Sub Profundo", "Senoidal pura: solo se siente, no estorba.", "A pure sine: you feel it more than hear it.",
            va (A (p), waveSine); filt (p, filterOff, 20000); amp (p, 0.004f, 1, 1, 0.12f); eq (p, 22, 20000);
            macros (m, 0, 0, charClean, 0, playLegato, 5, 0, 40); ),
        DMB_ARCH ("Bajo Tóxico", "Bajo FM de los 2000: golpe brillante que se apaga rápido.", "2000s FM bass: a bright punch that fades fast.",
            fm (A (p), 0, 1, 1, 2.5f, 0.18f, 0.15f, 0.15f); filt (p, filterLP12, 2000); amp (p, 0.001f, 1, 0.85f, 0.1f);
            eq (p, 28, 20000); macros (m, 0, 0, charClean, 0, playLegato, 5); ),
        DMB_ARCH ("Bajo Goma", "Cuadrado con filtro resonante que rebota.", "A square with a bouncy resonant filter.",
            va (A (p), waveSquare); filt (p, filterLP24, 220, 0.55f, 3.5f, 0.4f); fenv (p, 0.001f, 0.12f, 0.1f, 0.08f);
            amp (p, 0.001f, 1, 0.8f, 0.08f); eq (p, 28, 20000); macros (m, 0, 0, charClean, 15, playLegato, 10); ),
        DMB_ARCH ("Bajo Pulso", "Pulso con sub, firme y moderno.", "A pulse with sub, firm and modern.",
            va (A (p), wavePulse); pw (A (p), 0.3f); sub (A (p), 0.4f); filt (p, filterLP24, 600, 0.15f, 1.5f, 0.4f);
            fenv (p, 0.001f, 0.25f, 0.3f, 0.1f); amp (p, 0.002f, 1, 0.9f, 0.1f); eq (p, 28, 20000);
            macros (m, 0, 0, charClean, 0, playLegato, 6); ),
        DMB_ARCH ("Bajo Reese Viejo", "Dos sierras que se desafinan entre ellas: se mueve solo.", "Two saws detuning against each other: it moves by itself.",
            va (A (p), waveSaw, 2, 25, 0.2f); filt (p, filterLP24, 450, 0.2f, 0.8f, 0.4f); amp (p, 0.004f, 1, 1, 0.12f);
            eq (p, 28, 20000); macros (m, 0, 0, charWarm, 30, playLegato, 8); ),
        DMB_ARCH ("Bajo Caja", "Senoidal con un triángulo una octava arriba: corto y preciso.",
                  "A sine with a triangle an octave up: short and tight.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.35f); filt (p, filterLP12, 1200);
            amp (p, 0.002f, 0.6f, 0.5f, 0.08f); eq (p, 28, 20000); macros (m, 0, 0, charClean, 15, playMono, 0); ),
        DMB_ARCH ("Bajo Pluck", "Cuerda de bajo pulsada, con golpe.", "A plucked bass string with punch.",
            pluck (A (p), 0.5f, 0.25f, 0.3f, 1.2f, 0.2f); filt (p, filterLP12, 1500); amp (p, 0.001f, 2, 1, 0.1f);
            eq (p, 30, 20000); macros (m, 0, 0, charClean, 10); ),
        DMB_ARCH ("Bajo Eléctrico", "Como un bajo eléctrico con dedos: cálido y con cuerpo.", "Like a fingered electric bass: warm with body.",
            pluck (A (p), 0.35f, 0.3f, 0.25f, 2.5f, 0.5f); filt (p, filterLP24, 1100, 0.1f, 1.0f); fenv (p, 0.001f, 0.3f, 0.3f, 0.2f);
            amp (p, 0.002f, 3, 1, 0.12f); eq (p, 30, 20000); macros (m, 0, 0, charWarm, 25, playMono); ),
        DMB_ARCH ("Bajo Fretless", "Redondo y con portamento largo, para líneas que cantan.", "Round with a long glide, for singing lines.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 0, 0, 0.5f); filt (p, filterLP12, 900); amp (p, 0.03f, 1, 1, 0.15f);
            eq (p, 28, 20000); macros (m, 0, 0, charWarm, 15, playLegato, 40, 20); ),
        DMB_ARCH ("Bajo Analógico", "Sierra con un cuadrado abajo y filtro resonante, de sintetizador clásico.",
                  "Saw with a square below and a resonant filter, classic-synth style.",
            va (A (p), waveSaw); osc2 (A (p), waveSquare, -12, 0, 0.4f); filt (p, filterLP24, 320, 0.4f, 2.2f, 0.4f);
            fenv (p, 0.001f, 0.25f, 0.25f, 0.12f); amp (p, 0.002f, 1, 0.9f, 0.1f); eq (p, 28, 20000);
            macros (m, 0, 0, charWarm, 30, playLegato, 8); ),
        DMB_ARCH ("Bajo Tumbao", "Triángulo corto, como un bajo de salsa.", "A short triangle, like a salsa bass.",
            va (A (p), waveTri); osc2 (A (p), waveSine, 0, 0, 0.6f); filt (p, filterLP12, 1000, 0, 1.0f);
            fenv (p, 0.001f, 0.12f, 0, 0.08f); amp (p, 0.001f, 0.55f, 0.2f, 0.08f); eq (p, 30, 20000); macros (m, 0, 0, charClean, 10); ),
        DMB_ARCH ("Bajo Ácido Viejo", "Sierra con mucha resonancia que chilla al abrir.", "A saw with lots of resonance that squeals as it opens.",
            va (A (p), waveSaw); filt (p, filterLP24, 300, 0.75f, 3.5f, 0.4f, 0.3f); fenv (p, 0.001f, 0.2f, 0, 0.1f);
            amp (p, 0.001f, 1, 0.85f, 0.08f); eq (p, 30, 20000); macros (m, 0, 8, charClean, 0, playLegato, 15); ),
        DMB_ARCH ("Bajo Hueco", "Cuadrado por un filtro de banda: hueco y nasal, con sub.", "A square through a band filter: hollow and nasal, with sub.",
            va (A (p), waveSquare); sub (A (p), 0.5f); filt (p, filterBP, 500, 0.3f, 1.2f, 0.4f); fenv (p, 0.001f, 0.2f, 0.3f, 0.1f);
            amp (p, 0.002f, 1, 0.9f, 0.08f); eq (p, 28, 20000); macros (m, 0, 0, charClean, 10, playLegato, 6); ),
        DMB_ARCH ("Bajo Distorsión", "808 saturado: se oye hasta en bocinas chiquitas.", "A saturated 808: you hear it even on tiny speakers.",
            va (A (p), waveSine); pitchEnv (p, 3, 0.05f); filt (p, filterLP12, 2500, 0, 0, 0, 0.9f); amp (p, 0.001f, 1.8f, 0.4f, 0.2f);
            eq (p, 28, 20000); macros (m, 0, 0, charClean, 55, playMono); ),
        DMB_ARCH ("Bajo Synth Pop", "Dos sierras en octavas, corto y brillante.", "Two saws in octaves, short and bright.",
            va (A (p), waveSaw, 2, 8); osc2 (A (p), waveSaw, 12, 5, 0.35f); filt (p, filterLP24, 700, 0.2f, 2.5f, 0.4f);
            fenv (p, 0.001f, 0.15f, 0.1f, 0.1f); amp (p, 0.001f, 0.6f, 0.5f, 0.1f); eq (p, 30, 20000); macros (m, 0, 5); ),
        DMB_ARCH ("Bajo Sintético FM", "FM con una octava de brillo, al estilo de los teclados de los 2000.",
                  "FM with an octave of shine, like 2000s keyboards.",
            fm (A (p), 0, 1, 2, 3.0f, 0.1f, 0.1f); filt (p, filterLP12, 1800); amp (p, 0.001f, 0.9f, 0.6f, 0.08f);
            eq (p, 28, 20000); macros (m, 0, 0, charVintage, 30, playLegato, 5); ),
    };

    // =========================================================================================
    // PAD: fondos que entran despacio, con mucho espacio
    cats[catPad] = {
        DMB_ARCH ("Pad Marquesina", "Sierras cálidas que entran despacio: el colchón de siempre.", "Warm saws fading in: the classic bed.",
            va (A (p), waveSaw, 5, 14, 0.8f); filt (p, filterLP24, 2200, 0.1f, 0.6f); fenv (p, 0.6f, 2, 0.6f, 1.5f);
            amp (p, 0.5f, 2, 1, 1.4f); chorus (p, 0.35f); reverb (p, 0.7f, 3.5f); macros (m, 40, 0, charWarm, 20); ),
        DMB_ARCH ("Pad Noche", "Oscuro, con una octava abajo y chorus.", "Dark, with an octave below and chorus.",
            va (A (p), waveSaw, 3, 10, 0.7f); osc2 (A (p), waveSaw, -12, 6, 0.4f); filt (p, filterLP24, 1100, 0.15f, 0.5f);
            fenv (p, 1.0f, 2, 0.7f, 1.5f); amp (p, 0.8f, 2, 1, 1.8f); chorus (p, 0.4f); reverb (p, 0.75f, 4.0f); macros (m, 45, 0); ),
        DMB_ARCH ("Pad Cristalino", "FM brillante y limpio, como de vidrio.", "Bright, clean FM, like glass.",
            fm (A (p), 0, 1, 2, 1.2f, 3.0f, 0.4f); fm2 (A (p), 2, 5, 0.6f, 0.35f); filt (p, filterOff, 20000);
            amp (p, 0.4f, 2, 1, 1.5f); chorus (p, 0.3f); reverb (p, 0.7f, 3.5f); macros (m, 45, 10); ),
        DMB_ARCH ("Pad Respira", "El filtro abre y cierra solo, como si respirara.", "The filter opens and closes by itself, like breathing.",
            va (A (p), waveSaw, 3, 12, 0.7f); filt (p, filterLP24, 900, 0.3f); lfo (p, 0.15f, 0, 1.4f);
            amp (p, 0.6f, 2, 1, 1.6f); reverb (p, 0.7f, 3.5f); macros (m, 40, 0); ),
        DMB_ARCH ("Pad Romántico", "Triángulos suaves con un coro de fondo.", "Soft triangles with a choir behind.",
            va (A (p), waveTri, 3, 8, 0.6f); vox (B (p), 2, 3.0f, 0.1f, 3, 0, 0.5f); filt (p, filterLP12, 4000);
            amp (p, 0.7f, 2, 1, 1.6f); reverb (p, 0.75f, 4.0f); macros (m, 45, 0, charWarm, 15); ),
        DMB_ARCH ("Pad Vidrio", "FM con parciales de cristal, frío y brillante.", "FM with glass partials, cold and bright.",
            fm (A (p), 2, 1, 2.32f, 0.9f, 3.0f, 0.6f); fm2 (A (p), 1, 4.25f, 0.4f, 0.0f); filt (p, filterOff, 20000);
            amp (p, 0.5f, 2, 1, 2.0f); reverb (p, 0.8f, 4.5f); macros (m, 50, 0); ),
        DMB_ARCH ("Pad Sweep 2004", "El filtro se abre despacio en cada acorde, muy de la época.", "The filter slowly sweeps open on every chord, very era-specific.",
            va (A (p), waveSaw, 3, 12); osc2 (A (p), waveSquare, -12, 0, 0.3f); filt (p, filterLP24, 300, 0.35f, 4.5f);
            fenv (p, 2.5f, 3, 0.3f, 2.0f); amp (p, 0.3f, 2, 1, 1.5f); reverb (p, 0.7f, 3.5f); macros (m, 40, 0, charVintage, 35); ),
        DMB_ARCH ("Pad Coro Synth", "Un coro con sierras detrás.", "A choir with saws behind it.",
            vox (A (p), 1, 0.0f, 0.1f, 4); va (B (p), waveSaw, 3, 10, 0.7f, 0.45f); filt (p, filterLP12, 3500);
            amp (p, 0.5f, 2, 1, 1.5f); reverb (p, 0.75f, 4.0f); macros (m, 45, 0); ),
        DMB_ARCH ("Pad Cálido", "Cuadrados filtrados con saturación de cinta.", "Filtered squares with tape saturation.",
            va (A (p), waveSquare, 3, 10, 0.6f); filt (p, filterLP24, 1500, 0.1f); amp (p, 0.4f, 2, 1, 1.2f);
            reverb (p, 0.6f, 3.0f); macros (m, 35, 0, charWarm, 35); ),
        DMB_ARCH ("Pad Panorama", "Siete sierras abiertas a los lados.", "Seven saws spread wide.",
            va (A (p), waveSaw, 7, 25, 1.0f); filt (p, filterLP24, 3500, 0.1f); amp (p, 0.3f, 2, 1, 1.5f); chorus (p, 0.3f);
            reverb (p, 0.7f, 3.5f); macros (m, 40, 0); ),
        DMB_ARCH ("Pad Calle Lluviosa", "Sierras con ruido de lluvia, pasadas por casete.", "Saws with rain noise, run through a cassette.",
            va (A (p), waveSaw, 3, 10); noise (p, 0.25f, 0.9f, 0.7f); filt (p, filterLP24, 1500); amp (p, 0.6f, 2, 1, 1.6f);
            reverb (p, 0.7f, 3.5f); macros (m, 45, 0, charLofi, 55); ),
        DMB_ARCH ("Pad Órgano Suave", "Órgano suave con sierras detrás, para coros de iglesia moderna.", "Soft organ with saws behind it.",
            modal (A (p), modalOrgan, 1, 0.3f, 0.4f, 0.5f, 0, 2); va (B (p), waveSaw, 3, 8, 0.6f, 0.4f); filt (p, filterLP12, 3000);
            amp (p, 0.4f, 2, 1, 1.2f); chorus (p, 0.4f, 0.4f); reverb (p, 0.7f, 3.5f); macros (m, 40, 0); ),
        DMB_ARCH ("Pad Aire", "Senoidal con mucho aire: casi solo soplido.", "A sine with lots of air: almost pure breath.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.2f); noise (p, 0.35f, 0.55f, 1.2f); filt (p, filterLP12, 5000);
            amp (p, 0.8f, 2, 1, 2.0f); reverb (p, 0.8f, 4.5f); macros (m, 50, 0); ),
        DMB_ARCH ("Pad Metálico", "FM metálico que se mueve con el filtro.", "Metallic FM moving with the filter.",
            fm (A (p), 1, 1, 1.41f, 1.5f, 4.0f, 0.5f, 0.1f); fm2 (A (p), 1, 2.76f, 0.8f, 0.0f); filt (p, filterLP24, 4000, 0.2f);
            lfo (p, 0.2f, 0, 0.6f); amp (p, 0.6f, 2, 1, 2.0f); reverb (p, 0.8f, 4.5f); macros (m, 50, 10); ),
        DMB_ARCH ("Pad Estrellas", "Pulsos que se mueven con brillo arriba, como estrellas.", "Moving pulses with a high shimmer, like stars.",
            va (A (p), wavePulse, 3, 12, 0.8f); pw (A (p), 0.5f); osc2 (A (p), waveSine, 24, 0, 0.15f); lfo (p, 0.3f, 0, 0, 0, 0.35f);
            filt (p, filterLP24, 4000, 0.15f); amp (p, 0.4f, 2, 1, 1.6f); reverb (p, 0.8f, 4.0f); macros (m, 45, 15); ),
        DMB_ARCH ("Pad Profundo", "Sierras una octava abajo, oscuras.", "Saws an octave down, dark.",
            va (A (p), waveSaw, 3, 8, 0.5f, 1.0f, -1); filt (p, filterLP24, 700, 0.1f); amp (p, 0.8f, 2, 1, 2.0f);
            reverb (p, 0.75f, 4.0f); macros (m, 40, 0); ),
        DMB_ARCH ("Pad Nostalgia", "Pulso con trémolo suave y sabor de sampler viejo.", "A pulse with gentle tremolo and an old-sampler flavor.",
            va (A (p), wavePulse, 2, 8); pw (A (p), 0.35f); filt (p, filterLP24, 1800); lfo (p, 4.5f, 0, 0, 0.12f);
            amp (p, 0.5f, 2, 1, 1.5f); reverb (p, 0.6f, 3.0f); macros (m, 35, 0, charVintage, 45); ),
        DMB_ARCH ("Pad Agua", "Senoidales con chorus profundo: ondulan como agua.", "Sines with deep chorus: they ripple like water.",
            va (A (p), waveSine, 3, 15, 0.9f); osc2 (A (p), waveSine, 12, 6, 0.3f); vib (p, 0.4f, 0); filt (p, filterOff, 20000);
            chorus (p, 0.6f, 0.3f, 0.7f); amp (p, 0.6f, 2, 1, 2.0f); reverb (p, 0.75f, 4.0f); macros (m, 45, 15, charClean, 0, playPoly, 0, 25); ),
        DMB_ARCH ("Pad Celestial", "Sopranos en 'u' con sierras una octava arriba.", "Sopranos on 'oo' with saws an octave above.",
            vox (A (p), 3, 4.0f, 0.12f, 4); va (B (p), waveSaw, 5, 14, 0.8f, 0.35f, 1); filt (p, filterLP12, 6000);
            amp (p, 0.8f, 2, 1, 2.2f); reverb (p, 0.85f, 5.0f); macros (m, 55, 0); ),
        DMB_ARCH ("Pad Tensión", "Cuadrados desafinados con un semitono encima: suspenso.", "Detuned squares with a semitone on top: suspense.",
            va (A (p), waveSquare, 2, 30, 0.6f); osc2 (A (p), waveSaw, 1, 0, 0.3f); lfo (p, 0.12f, 0, 1.2f);
            filt (p, filterLP24, 1000, 0.35f); amp (p, 1.0f, 2, 1, 2.0f); reverb (p, 0.8f, 4.5f); macros (m, 45, 0); ),
    };
}
} // namespace dmb
