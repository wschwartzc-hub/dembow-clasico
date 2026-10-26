// Sonidos base: PIANO, ÓRGANO, CAMPANAS y MARIMBA
#include "Build.h"

namespace dmb
{
using namespace build;

void addKeysOrganBellsMallets (std::vector<Archetype> (&cats)[numCategories])
{
    // =========================================================================================
    // PIANO: acústicos (modelo de cuerdas) y eléctricos (FM)
    cats[catPiano] = {
        DMB_ARCH ("Piano Romántico", "Piano cálido con sala, para el reggaetón romántico.", "A warm piano with a room, for romantic reggaeton.",
            modal (A (p), modalPiano, 6, 0.45f, 0.5f, 0.5f, 0.35f); filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 0.45f);
            reverb (p, 0.6f, 2.5f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano de Cola", "Piano grande y brillante, con sustain largo.", "A big, bright grand with a long sustain.",
            modal (A (p), modalPiano, 8, 0.6f, 0.65f, 0.45f, 0.4f); filt (p, filterOff, 20000); amp (p, 0.001f, 14, 1, 0.5f);
            reverb (p, 0.7f, 3.0f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Piano Vertical", "Piano de casa: más corto y con más carácter.", "An upright: shorter and with more character.",
            modal (A (p), modalPiano, 4, 0.5f, 0.45f, 0.6f, 0.6f); filt (p, filterOff, 20000); amp (p, 0.001f, 10, 1, 0.35f);
            reverb (p, 0.4f, 1.6f); macros (m, 18, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano Cantina", "Dos pianos un poco desafinados entre sí, como de bar.", "Two slightly detuned pianos, like a bar piano.",
            modal (A (p), modalPiano, 5, 0.6f, 0.55f, 0.5f, 0.5f); modal (B (p), modalPiano, 5, 0.6f, 0.55f, 0.5f, 0.5f, 0, 0, 0.7f);
            B (p)[lp::fine] = 14.0f; filt (p, filterOff, 20000); amp (p, 0.001f, 10, 1, 0.35f); reverb (p, 0.4f, 1.5f);
            macros (m, 18, 0, charWarm, 30, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano Algodón", "Martillos suaves y tono redondo.", "Soft hammers and a round tone.",
            modal (A (p), modalPiano, 6, 0.2f, 0.3f, 0.5f, 0.35f); filt (p, filterLP12, 4000); amp (p, 0.002f, 12, 1, 0.45f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 0, charWarm, 15, playPoly, 0, 0, 50); ),
        DMB_ARCH ("Piano Dance", "Piano muy brillante con un brillo de campana: corta en la mezcla.",
                  "A very bright piano with a bell sparkle: cuts through the mix.",
            modal (A (p), modalPiano, 3, 0.85f, 0.8f, 0.5f, 0.4f); fm (B (p), 0, 1, 4, 1.0f, 0.4f, 0.0f, 0, 0.25f);
            filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 0.3f); macros (m, 18, 15, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("EP Clásico", "Piano eléctrico de FM: cuerpo redondo con la campanita del ataque.",
                  "An FM electric piano: a round body with the bell of the attack.",
            fm (A (p), 0, 1, 1, 1.6f, 0.8f, 0.2f); fm2 (A (p), 1, 14, 1.1f, 0.4f, 0.3f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 5, 0, 0.35f); chorus (p, 0.3f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("EP Campana", "Piano eléctrico con más campana, de los teclados de los 2000.", "An electric piano with more bell, 2000s-keyboard style.",
            fm (A (p), 0, 1, 1, 1.2f, 1.0f, 0.3f); fm2 (A (p), 1, 3.5f, 2.0f, 0.5f, 1.2f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 5, 0, 0.4f); chorus (p, 0.25f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("EP Seda", "Piano eléctrico suave y oscuro, con chorus.", "A soft, dark electric piano with chorus.",
            fm (A (p), 0, 1, 1, 0.8f, 1.2f, 0.15f); fm2 (A (p), 1, 14, 0.5f, 0.2f, 0.2f); filt (p, filterLP12, 3500);
            amp (p, 0.002f, 6, 0, 0.4f); chorus (p, 0.4f); macros (m, 22, 0, charWarm, 20, playPoly, 0, 0, 60); ),
        DMB_ARCH ("EP Wurly", "Piano eléctrico de lengüetas: más nasal y con trémolo.", "A reed electric piano: more nasal, with tremolo.",
            fm (A (p), 0, 1, 1, 1.8f, 0.6f, 0.35f, 0.4f); filt (p, filterLP12, 4500); amp (p, 0.001f, 4, 0, 0.3f);
            lfo (p, 5.5f, 0, 0, 0.18f); macros (m, 18, 0, charWarm, 20, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano Lofi", "Piano pasado por casete: opaco y con siseo.", "A piano run through a cassette: dull and hissy.",
            modal (A (p), modalPiano, 5, 0.45f, 0.45f, 0.5f, 0.4f); filt (p, filterOff, 20000); amp (p, 0.001f, 10, 1, 0.4f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 0, charLofi, 55, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Piano Juguete", "Piano de juguete: barras de metal brillantes.", "A toy piano: bright metal rods.",
            modal (A (p), modalGlock, 1.2f, 0.7f, 0.6f, 0.5f, 0.3f); modal (B (p), modalGlock, 1.2f, 0.7f, 0.6f, 0.5f, 0.3f, 0, 0, 0.5f);
            B (p)[lp::fine] = 18.0f; filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.3f); macros (m, 20, 0); ),
        DMB_ARCH ("Piano Eléctrico Viejo", "Piano de cuerdas eléctrico, brillante y con chorus.", "An electric grand: bright, with chorus.",
            modal (A (p), modalPiano, 4, 0.7f, 0.7f, 0.6f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 0.35f);
            chorus (p, 0.45f); macros (m, 18, 0, charVintage, 30, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Clavinet Dembow", "Cuerda muy brillante y apagada: funky y percusivo.", "A very bright, muted string: funky and percussive.",
            pluck (A (p), 0.95f, 0.08f, 0.85f, 0.8f); filt (p, filterLP24, 2500, 0.3f, 2.0f); fenv (p, 0.001f, 0.2f, 0.3f, 0.1f);
            amp (p, 0.001f, 1.5f, 1, 0.08f); macros (m, 10, 15, charClean, 15, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Piano Octavas", "Piano con su octava arriba: más grande y brillante.", "A piano with its octave above: bigger and brighter.",
            modal (A (p), modalPiano, 6, 0.5f, 0.55f, 0.5f, 0.4f); modal (B (p), modalPiano, 5, 0.5f, 0.55f, 0.5f, 0.4f, 0, 0, 0.5f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 0.45f); reverb (p, 0.6f, 2.5f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano Nocturno", "Piano oscuro con mucha sala, para intros.", "A dark piano with a big room, for intros.",
            modal (A (p), modalPiano, 7, 0.3f, 0.3f, 0.5f, 0.35f); filt (p, filterOff, 20000); amp (p, 0.001f, 14, 1, 0.6f);
            reverb (p, 0.85f, 4.5f); macros (m, 45, 15, charWarm, 20, playPoly, 0, 0, 60); ),
        DMB_ARCH ("EP Trémolo", "Piano eléctrico que va de lado a lado.", "An electric piano that pans side to side.",
            fm (A (p), 0, 1, 1, 1.6f, 0.8f, 0.2f); fm2 (A (p), 1, 14, 1.0f, 0.35f, 0.3f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 5, 0, 0.35f); lfo (p, 4.8f, 0, 0, 0.35f, 0, 0.4f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Piano Synth 80s", "Sierra corta con brillo de FM: el piano digital de los 80.", "A short saw with FM shine: the 80s digital piano.",
            va (A (p), waveSaw, 2, 6, 0.5f, 0.7f); fm (B (p), 0, 1, 4, 1.0f, 0.4f, 0.0f, 0, 0.35f);
            filt (p, filterLP24, 1200, 0.1f, 3.0f); fenv (p, 0.001f, 0.4f, 0.1f, 0.3f); amp (p, 0.001f, 2, 0.3f, 0.3f);
            chorus (p, 0.35f); macros (m, 18, 10); ),
        DMB_ARCH ("Clavecín Callejero", "Clavecín: cuerdas punteadas con su octava.", "A harpsichord: plucked strings with their octave.",
            pluck (A (p), 0.95f, 0.05f, 0.9f, 1.5f, 0.15f); pluck (B (p), 0.9f, 0.08f, 0.9f, 1.5f, 0.1f, 0.5f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 3, 1, 0.15f); macros (m, 18, 0, charVintage, 25); ),
        DMB_ARCH ("Piano con Colchón", "Piano con un pad suave por debajo que se queda mientras sostienes.",
                  "A piano with a soft pad underneath that holds while you sustain.",
            modal (A (p), modalPiano, 6, 0.5f, 0.5f, 0.5f, 0.35f); va (B (p), waveSaw, 3, 10, 0.7f, 0.22f);
            filt (p, filterLP12, 3500); amp (p, 0.002f, 12, 1, 0.8f); reverb (p, 0.7f, 3.0f); macros (m, 30, 0, charWarm, 15, playPoly, 0, 0, 65); ),
    };

    // =========================================================================================
    // ÓRGANO: órganos de barras, combos y lengüetas (acordeón, melódica)
    cats[catOrgan] = {
        DMB_ARCH ("Órgano Gospel", "Órgano de barras con percusión y Leslie rápido.", "A drawbar organ with percussion and a fast Leslie.",
            modal (A (p), modalOrgan, 1, 0.5f, 0.55f, 0.5f, 0, 0, 0.45f); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.45f, 5.8f, 0.25f); macros (m, 18, 0, charWarm, 30); ),
        DMB_ARCH ("Órgano Lleno", "Todas las barras afuera: grande y brillante.", "All drawbars out: big and bright.",
            modal (A (p), modalOrgan, 1, 0.5f, 0.5f, 0.5f, 0, 1); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.35f, 0.9f, 0.4f); macros (m, 15, 0, charClean, 20); ),
        DMB_ARCH ("Órgano Dulce", "Solo la barra de 8': suave como flauta.", "Only the 8' drawbar: soft like a flute.",
            modal (A (p), modalOrgan, 1, 0.3f, 0.5f, 0.5f, 0, 2); filt (p, filterOff, 20000); amp (p, 0.004f, 1, 1, 0.15f);
            chorus (p, 0.3f, 0.7f, 0.4f); macros (m, 20, 0); ),
        DMB_ARCH ("Órgano Iglesia", "Órgano grande en una iglesia, sin click.", "A big organ in a church, no click.",
            modal (A (p), modalOrgan, 1, 0.0f, 0.5f, 0.5f, 0, 3); filt (p, filterOff, 20000); amp (p, 0.05f, 1, 1, 0.8f);
            reverb (p, 0.95f, 6.0f); macros (m, 55, 0); ),
        DMB_ARCH ("Órgano Rock", "Órgano saturado con Leslie, áspero.", "A saturated organ with Leslie, gritty.",
            modal (A (p), modalOrgan, 1, 0.6f, 0.6f, 0.5f, 0, 4, 0.3f); filt (p, filterLP12, 6000, 0.1f, 0, 0.5f, 0.5f);
            amp (p, 0.003f, 1, 1, 0.06f); chorus (p, 0.4f, 6.0f, 0.25f); macros (m, 12, 0, charWarm, 55); ),
        DMB_ARCH ("Órgano Hueco", "Registro hueco (solo armónicos impares): suena nasal.", "A hollow registration (odd harmonics): sounds nasal.",
            modal (A (p), modalOrgan, 1, 0.4f, 0.5f, 0.5f, 0, 5); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.08f);
            chorus (p, 0.3f, 0.8f, 0.4f); macros (m, 18, 0); ),
        DMB_ARCH ("Órgano Cine", "Órgano grave con mucha sala, para momentos dramáticos.", "A deep organ with a big room, for dramatic moments.",
            modal (A (p), modalOrgan, 1, 0.2f, 0.45f, 0.5f, 0, 6); filt (p, filterOff, 20000); amp (p, 0.08f, 1, 1, 1.0f);
            reverb (p, 0.85f, 4.0f); macros (m, 40, 0); ),
        DMB_ARCH ("Acordeón Viejo", "Dos lengüetas desafinadas entre sí (musette), como un acordeón.", "Two reeds detuned against each other (musette), like an accordion.",
            modal (A (p), modalOrgan, 1, 0.2f, 0.6f, 0.5f, 0, 7); modal (B (p), modalOrgan, 1, 0.2f, 0.6f, 0.5f, 0, 7, 0, 0.8f);
            B (p)[lp::fine] = 12.0f; filt (p, filterLP12, 5000); amp (p, 0.04f, 1, 1, 0.12f); macros (m, 15, 0, charVintage, 25); ),
        DMB_ARCH ("Órgano Bubble", "Órgano corto con percusión, para hacer el 'bubble' de reggae.", "A short organ with percussion, for the reggae 'bubble'.",
            modal (A (p), modalOrgan, 1, 0.5f, 0.55f, 0.5f, 0, 0, 0.6f); filt (p, filterOff, 20000); amp (p, 0.002f, 0.25f, 0.2f, 0.05f);
            chorus (p, 0.4f, 6.0f, 0.25f); macros (m, 15, 10); ),
        DMB_ARCH ("Órgano Combo", "Órgano de transistores con vibrato, de grupo de los 60.", "A transistor combo organ with vibrato, 60s-band style.",
            va (A (p), wavePulse); pw (A (p), 0.4f); osc2 (A (p), waveSquare, 12, 0, 0.5f); filt (p, filterLP12, 4500);
            amp (p, 0.003f, 1, 1, 0.05f); vib (p, 6.5f, 0); macros (m, 15, 0, charClean, 0, playPoly, 0, 25); ),
        DMB_ARCH ("Órgano Transistor", "Cuadrados con la quinta arriba, sabor de teclado barato.", "Squares with the fifth above, cheap-keyboard flavor.",
            va (A (p), waveSquare); osc2 (A (p), waveSquare, 19, 0, 0.35f); filt (p, filterLP12, 3500); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.3f); macros (m, 15, 0, charVintage, 20); ),
        DMB_ARCH ("Órgano Percusivo", "Mucha percusión: cada nota tiene un golpe brillante.", "Lots of percussion: every note has a bright click.",
            modal (A (p), modalOrgan, 1, 0.6f, 0.5f, 0.5f, 0, 2, 0.9f); filt (p, filterOff, 20000); amp (p, 0.002f, 1, 1, 0.06f);
            chorus (p, 0.35f, 5.5f, 0.25f); macros (m, 15, 0); ),
        DMB_ARCH ("Melódica Callejera", "Lengüeta con aire, como una melódica.", "A breathy reed, like a melodica.",
            va (A (p), wavePulse); pw (A (p), 0.42f); osc2 (A (p), waveSaw, 0, 6, 0.3f); noise (p, 0.06f, 0.6f, 2.0f);
            filt (p, filterLP12, 2600, 0.1f); amp (p, 0.03f, 1, 1, 0.08f); vib (p, 5.0f, 0.4f); macros (m, 18, 10, charClean, 0, playPoly, 0, 15); ),
        DMB_ARCH ("Armónica Synth", "Pulso delgado con vibrato, como una armónica.", "A thin pulse with vibrato, like a harmonica.",
            va (A (p), wavePulse); pw (A (p), 0.15f); noise (p, 0.05f, 0.7f, 2.0f); filt (p, filterLP12, 3000, 0.3f);
            amp (p, 0.02f, 1, 1, 0.08f); vib (p, 6.0f, 0.2f); macros (m, 15, 10, charClean, 0, playLegato, 8, 30); ),
        DMB_ARCH ("Órgano Leslie Lento", "Órgano con el Leslie girando despacio.", "An organ with the Leslie spinning slowly.",
            modal (A (p), modalOrgan, 1, 0.4f, 0.5f, 0.5f, 0, 0); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.08f);
            chorus (p, 0.6f, 0.8f, 0.5f); lfo (p, 0.8f, 0, 0, 0.12f, 0, 0.4f); macros (m, 20, 0); ),
        DMB_ARCH ("Órgano Casete", "Órgano rock grabado en casete.", "A rock organ recorded on cassette.",
            modal (A (p), modalOrgan, 1, 0.5f, 0.55f, 0.5f, 0, 4, 0.2f); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.35f, 5.0f, 0.25f); macros (m, 15, 0, charLofi, 50); ),
        DMB_ARCH ("Bandoneón", "Lengüetas que entran despacio, con vibrato.", "Reeds that swell in, with vibrato.",
            modal (A (p), modalOrgan, 1, 0.1f, 0.55f, 0.5f, 0, 7); modal (B (p), modalOrgan, 1, 0.1f, 0.55f, 0.5f, 0, 7, 0, 0.8f);
            B (p)[lp::fine] = -8.0f; filt (p, filterLP12, 4000); amp (p, 0.12f, 1, 1, 0.2f); vib (p, 4.5f, 0.3f);
            macros (m, 20, 0, charWarm, 15, playPoly, 0, 15); ),
        DMB_ARCH ("Órgano FM", "Órgano digital de FM, limpio y brillante.", "A digital FM organ, clean and bright.",
            fm (A (p), 2, 1, 2, 0.6f, 5, 1.0f); fm2 (A (p), 1, 3, 0.4f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.35f, 5.5f, 0.25f); macros (m, 15, 0, charVintage, 15); ),
        DMB_ARCH ("Órgano Flauta", "Flautas de órgano en dos octavas.", "Organ flutes in two octaves.",
            modal (A (p), modalOrgan, 1, 0.2f, 0.5f, 0.5f, 0, 2); modal (B (p), modalOrgan, 1, 0.2f, 0.5f, 0.5f, 0, 2, 0, 0.5f, 1);
            filt (p, filterOff, 20000); amp (p, 0.01f, 1, 1, 0.12f); chorus (p, 0.3f, 0.6f, 0.4f); macros (m, 22, 0); ),
        DMB_ARCH ("Órgano Alto", "Órgano lleno una octava arriba, con Leslie rápido.", "A full organ an octave up, with a fast Leslie.",
            modal (A (p), modalOrgan, 1, 0.5f, 0.55f, 0.5f, 0, 1, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.003f, 1, 1, 0.06f);
            chorus (p, 0.45f, 6.0f, 0.25f); macros (m, 18, 0); ),
    };

    // =========================================================================================
    // CAMPANAS
    cats[catBells] = {
        DMB_ARCH ("Campana Iglesia", "Campana grande con sus parciales graves.", "A big bell with its low partials.",
            modal (A (p), modalBell, 6, 0.6f, 0.6f, 0.4f); filt (p, filterOff, 20000); amp (p, 0.001f, 20, 1, 3.0f);
            reverb (p, 0.85f, 4.5f); macros (m, 35, 0, charClean, 0, playPoly, 0, 0, 50); ),
        DMB_ARCH ("Campana Tubular", "Campana de orquesta que suena larga.", "An orchestral tubular bell with a long ring.",
            modal (A (p), modalTubular, 5, 0.6f, 0.55f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 20, 1, 2.5f);
            reverb (p, 0.75f, 3.5f); macros (m, 30, 0, charClean, 0, playPoly, 0, 0, 50); ),
        DMB_ARCH ("Caja de Música", "Las lengüetas de una caja de música.", "The tines of a music box.",
            modal (A (p), modalMusicBox, 2.2f, 0.7f, 0.7f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 1.2f);
            reverb (p, 0.5f, 2.0f); macros (m, 25, 10, charClean, 0, playPoly, 0, 0, 50); ),
        DMB_ARCH ("Glockenspiel", "Barras de metal brillantes una octava arriba.", "Bright metal bars an octave up.",
            modal (A (p), modalGlock, 2.5f, 0.75f, 0.7f, 0.5f, 0.3f, 0, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 1.0f);
            reverb (p, 0.6f, 2.5f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Celesta", "Celesta: dulce y delicada.", "A celesta: sweet and delicate.",
            modal (A (p), modalCelesta, 1.6f, 0.5f, 0.5f, 0.5f, 0.3f, 0, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.8f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 0, charClean, 0, playPoly, 0, 0, 55); ),
        DMB_ARCH ("Campanita FM", "Campanita digital de FM.", "A small digital FM bell.",
            fm (A (p), 0, 1, 3.5f, 2.2f, 1.5f, 0.05f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 0, 1.5f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 10); ),
        DMB_ARCH ("Campana Cristal", "Una copa de cristal golpeada.", "A struck crystal glass.",
            modal (A (p), modalGlass, 3.5f, 0.5f, 0.6f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 2.0f);
            reverb (p, 0.7f, 3.5f); macros (m, 32, 0); ),
        DMB_ARCH ("Campana de Viento", "Campanitas de viento, agudas y con mucho espacio.", "Wind chimes: high and spacious.",
            modal (A (p), modalGlass, 4, 0.6f, 0.7f, 0.5f, 0.3f, 0, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 2.5f);
            chorus (p, 0.3f, 0.4f); reverb (p, 0.85f, 5.0f); macros (m, 45, 0); ),
        DMB_ARCH ("Campana Digital 80s", "La campana tubular de FM de los 80.", "The 80s FM tubular bell.",
            fm (A (p), 0, 1, 1.4f, 3.0f, 2.5f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 0, 1.5f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 0, charVintage, 20); ),
        DMB_ARCH ("Campana Profunda", "Campana grave y oscura, una octava abajo.", "A deep, dark bell an octave down.",
            modal (A (p), modalBell, 8, 0.35f, 0.3f, 0.4f, 0.3f, 0, 0, 1.0f, -1); filt (p, filterOff, 20000); amp (p, 0.001f, 24, 1, 4.0f);
            reverb (p, 0.9f, 6.0f); macros (m, 40, 0, charClean, 0, playPoly, 0, 0, 45); ),
        DMB_ARCH ("Campana Lofi", "Caja de música vieja, con siseo.", "An old music box, with hiss.",
            modal (A (p), modalMusicBox, 2.0f, 0.6f, 0.55f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 1.0f);
            reverb (p, 0.5f, 2.0f); macros (m, 25, 10, charLofi, 60); ),
        DMB_ARCH ("Triangulito", "Un timbre metálico y agudo, como un triángulo afinado.", "A high metallic ting, like a tuned triangle.",
            fm (A (p), 0, 1, 2.76f, 1.5f, 3.0f, 0.1f, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 0, 2.0f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 0); ),
        DMB_ARCH ("Campana Templo", "Como un cuenco que canta: largo y con latido.", "Like a singing bowl: long and beating.",
            modal (A (p), modalGlass, 10, 0.3f, 0.4f, 0.3f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.002f, 30, 1, 4.0f);
            chorus (p, 0.35f, 0.2f, 0.5f); reverb (p, 0.9f, 6.0f); macros (m, 45, 0, charClean, 0, playPoly, 0, 0, 45); ),
        DMB_ARCH ("Campana Pluck", "Campanita corta, casi un pluck.", "A short little bell, almost a pluck.",
            fm (A (p), 0, 1, 3.5f, 2.5f, 0.25f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.8f, 0, 0.4f);
            reverb (p, 0.5f, 2.0f); macros (m, 25, 15); ),
        DMB_ARCH ("Campanas Hielo", "Glockenspiel con eco y una sala enorme.", "A glockenspiel with echo and a huge room.",
            modal (A (p), modalGlock, 2.5f, 0.8f, 0.8f, 0.5f, 0.3f, 0, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 1.2f);
            reverb (p, 0.9f, 5.0f, 0.8f); delay (p, 5, 0.4f, 0.7f); macros (m, 45, 25); ),
        DMB_ARCH ("Gamelán", "Metalófono con una segunda capa desafinada que 'late'.", "A metallophone with a detuned second layer that beats.",
            modal (A (p), modalSteelPan, 3, 0.5f, 0.5f, 0.4f, 1.0f); modal (B (p), modalGlass, 3, 0.5f, 0.5f, 0.4f, 0.5f, 0, 0, 0.4f, 1);
            B (p)[lp::fine] = 20.0f; filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 1.5f); reverb (p, 0.7f, 3.0f); macros (m, 30, 0); ),
        DMB_ARCH ("Campana Sintética", "FM inarmónico con retroalimentación: campana de sintetizador.", "Inharmonic FM with feedback: a synth bell.",
            fm (A (p), 0, 1, 2.41f, 2.0f, 2.0f, 0.1f, 0.2f); filt (p, filterOff, 20000); amp (p, 0.001f, 5, 0, 1.5f);
            reverb (p, 0.6f, 2.5f); macros (m, 28, 10); ),
        DMB_ARCH ("Campanas Ensueño", "Celesta con un colchón suave que se queda.", "A celesta with a soft held bed.",
            modal (A (p), modalCelesta, 2.0f, 0.5f, 0.55f, 0.5f, 0.3f, 0, 0, 1.0f, 1); va (B (p), waveSine, 3, 10, 0.8f, 0.25f);
            filt (p, filterOff, 20000); amp (p, 0.001f, 10, 1, 2.0f); reverb (p, 0.85f, 5.0f); macros (m, 45, 0); ),
        DMB_ARCH ("Campana Doble", "Campana con su octava arriba.", "A bell with its octave above.",
            modal (A (p), modalBell, 5, 0.55f, 0.55f, 0.4f); modal (B (p), modalBell, 4, 0.55f, 0.55f, 0.4f, 0.3f, 0, 0, 0.5f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 18, 1, 2.5f); reverb (p, 0.8f, 4.0f); macros (m, 35, 0, charClean, 0, playPoly, 0, 0, 50); ),
        DMB_ARCH ("Carillón", "Campanas tubulares con glockenspiel encima.", "Tubular bells with a glockenspiel on top.",
            modal (A (p), modalTubular, 5, 0.6f, 0.55f, 0.5f); modal (B (p), modalGlock, 2.5f, 0.7f, 0.6f, 0.5f, 0.3f, 0, 0, 0.4f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 18, 1, 2.5f); reverb (p, 0.8f, 4.0f); macros (m, 35, 0); ),
    };

    // =========================================================================================
    // MARIMBA (mallets): marimba, vibráfono, xilófono, kalimba, steel pan
    cats[catMallets] = {
        DMB_ARCH ("Marimba Clásica", "Marimba de madera con resonadores.", "A wooden marimba with resonators.",
            modal (A (p), modalMarimba, 1.4f, 0.5f, 0.5f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.5f);
            reverb (p, 0.5f, 2.0f); macros (m, 18, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Marimba Terciopelo", "Baquetas suaves: redonda y cálida.", "Soft mallets: round and warm.",
            modal (A (p), modalMarimba, 1.5f, 0.25f, 0.35f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.002f, 6, 1, 0.5f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 0, charWarm, 15, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Marimba Tropical", "Baquetas duras y eco: muy de playa.", "Hard mallets and echo: very beachy.",
            modal (A (p), modalMarimba, 1.0f, 0.75f, 0.65f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 5, 1, 0.4f);
            delay (p, 5, 0.35f); reverb (p, 0.5f, 2.0f); macros (m, 18, 20, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Vibráfono", "Barras de metal con sustain largo.", "Metal bars with a long sustain.",
            modal (A (p), modalVibes, 4, 0.45f, 0.5f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 0.8f);
            reverb (p, 0.6f, 2.5f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Vibráfono Motor", "Vibráfono con el motor prendido (trémolo).", "A vibraphone with the motor on (tremolo).",
            modal (A (p), modalVibes, 4, 0.45f, 0.5f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 12, 1, 0.8f);
            lfo (p, 5.0f, 0, 0, 0.35f); reverb (p, 0.6f, 2.5f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 60); ),
        DMB_ARCH ("Xilófono", "Xilófono brillante una octava arriba.", "A bright xylophone an octave up.",
            modal (A (p), modalXylo, 0.6f, 0.7f, 0.65f, 0.5f, 0.3f, 0, 0, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 4, 1, 0.3f);
            reverb (p, 0.45f, 1.6f); macros (m, 18, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Kalimba", "Piano de pulgar africano.", "An African thumb piano.",
            modal (A (p), modalKalimba, 1.6f, 0.5f, 0.5f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.5f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Steel Pan Caribe", "Tambor metálico del Caribe.", "A Caribbean steel drum.",
            modal (A (p), modalSteelPan, 1.8f, 0.5f, 0.55f, 0.5f); pitchEnv (p, 0.3f, 0.03f); filt (p, filterOff, 20000);
            amp (p, 0.003f, 6, 1, 0.5f); reverb (p, 0.5f, 2.0f); macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Steel Pan Doble", "Dos steel pans en octavas con chorus.", "Two steel pans in octaves with chorus.",
            modal (A (p), modalSteelPan, 1.8f, 0.5f, 0.55f, 0.5f); modal (B (p), modalSteelPan, 1.5f, 0.5f, 0.55f, 0.5f, 0.3f, 0, 0, 0.4f, 1);
            filt (p, filterOff, 20000); amp (p, 0.003f, 6, 1, 0.5f); chorus (p, 0.3f); reverb (p, 0.5f, 2.0f); macros (m, 22, 0); ),
        DMB_ARCH ("Marimba FM 80s", "Marimba digital de FM.", "A digital FM marimba.",
            fm (A (p), 0, 1, 4, 2.0f, 0.15f, 0.0f); filt (p, filterOff, 20000); amp (p, 0.001f, 1.2f, 0, 0.3f);
            reverb (p, 0.45f, 1.6f); macros (m, 18, 10, charVintage, 20); ),
        DMB_ARCH ("Balafón", "Marimba africana con zumbido.", "An African marimba with a buzz.",
            modal (A (p), modalMarimba, 1.0f, 0.65f, 0.55f, 0.5f); noise (p, 0.0f, 0.7f, 1.5f, 0.6f, 0.08f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 5, 1, 0.4f); reverb (p, 0.4f, 1.5f); macros (m, 18, 0); ),
        DMB_ARCH ("Marimba Grave", "Marimba una octava abajo, con mucho cuerpo.", "A marimba an octave down, with lots of body.",
            modal (A (p), modalMarimba, 2.0f, 0.45f, 0.45f, 0.5f, 0.3f, 0, 0, 1.0f, -1); filt (p, filterOff, 20000); amp (p, 0.002f, 8, 1, 0.6f);
            reverb (p, 0.5f, 2.0f); macros (m, 18, 0, charClean, 0, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Kalimba Oscura", "Kalimba suave y oscura con sala.", "A soft, dark kalimba with a room.",
            modal (A (p), modalKalimba, 1.8f, 0.3f, 0.25f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.002f, 6, 1, 0.6f);
            reverb (p, 0.7f, 3.0f); macros (m, 35, 0, charWarm, 15); ),
        DMB_ARCH ("Marimba Synth", "Senoidal con un destello dos octavas arriba.", "A sine with a sparkle two octaves up.",
            va (A (p), waveSine); osc2 (A (p), waveSine, 24, 0, 0.2f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.5f, 0, 0.2f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 15); ),
        DMB_ARCH ("Mallet Madera", "Bloques de madera suaves.", "Soft wooden blocks.",
            modal (A (p), modalXylo, 0.5f, 0.3f, 0.35f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 3, 1, 0.3f);
            reverb (p, 0.4f, 1.5f); macros (m, 15, 0, charClean, 0, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Vibes Nocturno", "Vibráfono oscuro con chorus y sala grande.", "A dark vibraphone with chorus and a big room.",
            modal (A (p), modalVibes, 5, 0.3f, 0.3f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.002f, 14, 1, 1.0f);
            chorus (p, 0.3f); reverb (p, 0.8f, 4.0f); macros (m, 40, 0, charWarm, 15); ),
        DMB_ARCH ("Marimba Dembow", "Marimba con eco con puntillo, lista para el riff.", "A marimba with a dotted echo, ready for the riff.",
            modal (A (p), modalMarimba, 1.2f, 0.55f, 0.55f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.45f);
            delay (p, 5, 0.38f); macros (m, 18, 30, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Steel Drum Synth", "Steel drum de FM, de teclado viejo.", "An FM steel drum from an old keyboard.",
            fm (A (p), 0, 1, 2, 1.5f, 0.4f, 0.1f); pitchEnv (p, -0.4f, 0.04f); filt (p, filterOff, 20000); amp (p, 0.003f, 1.5f, 0, 0.3f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 10, charVintage, 20); ),
        DMB_ARCH ("Glock Marimba", "Marimba con un glockenspiel encima.", "A marimba with a glockenspiel on top.",
            modal (A (p), modalMarimba, 1.4f, 0.5f, 0.5f, 0.5f); modal (B (p), modalGlock, 1.5f, 0.6f, 0.6f, 0.5f, 0.3f, 0, 0, 0.35f, 1);
            filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.5f); reverb (p, 0.5f, 2.0f); macros (m, 20, 0); ),
        DMB_ARCH ("Kalimba Agua", "Kalimba con chorus y eco: suena mojada.", "A kalimba with chorus and echo: sounds wet.",
            modal (A (p), modalKalimba, 1.6f, 0.5f, 0.55f, 0.5f); filt (p, filterOff, 20000); amp (p, 0.001f, 6, 1, 0.5f);
            chorus (p, 0.5f, 0.5f, 0.6f); delay (p, 5, 0.35f); macros (m, 25, 25); ),
    };
}
} // namespace dmb
