// Sonidos base: GUITARRA, FLAUTA, COROS y FX
#include "Build.h"

namespace dmb
{
using namespace build;

namespace
{
    // flauta: espectro + soplido + "chiff" del ataque + vibrato
    void flute (Patch& p, int spectrum, float breath, float chiff, float attack, int octave = 0)
    {
        modal (A (p), modalFlute, 1, 0.5f, 0.5f, 0.5f, 0, spectrum, 0, 1.0f, octave);
        filt (p, filterLP12, 6000);
        amp (p, attack, 1, 1, 0.15f);
        noise (p, breath, 0.4f, 2.5f, chiff, 0.06f);
        vib (p, 5.2f, 0.35f);
        reverb (p, 0.6f, 2.4f);
    }

    // coro: voz principal + segunda voz opcional
    void choir (Patch& p, int voice, float vowel, float breath = 0.12f)
    {
        vox (A (p), voice, vowel, breath, 4);
        filt (p, filterLP12, 7000);
        amp (p, 0.25f, 1, 1, 0.7f);
        vib (p, 5.0f, 0.3f);
        chorus (p, 0.25f);
        reverb (p, 0.8f, 3.5f);
        eq (p, 90, 20000);
    }

    void guitar (Patch& p, float bright, float pos, float damp, float decay, float body)
    {
        pluck (A (p), bright, pos, damp, decay, body);
        filt (p, filterOff, 20000);
        amp (p, 0.001f, 8, 1, 0.3f);
        reverb (p, 0.55f, 2.2f);
        eq (p, 70, 20000);
    }
}

void addGuitarFluteChoirFx (std::vector<Archetype> (&cats)[numCategories])
{
    // =========================================================================================
    cats[catGuitar] = {
        DMB_ARCH ("Nylon Romántica", "Guitarra de nylon cálida, la del reggaetón romántico.", "A warm nylon guitar, the one from romantic reggaeton.",
            guitar (p, 0.4f, 0.17f, 0.5f, 2.8f, 0.6f); macros (m, 25, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Nylon Clara", "Nylon brillante y definida.", "A bright, defined nylon.",
            guitar (p, 0.6f, 0.14f, 0.65f, 2.5f, 0.5f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Requinto Bachata", "Requinto brillante con eco, para punteos.", "A bright requinto with echo, for lead lines.",
            guitar (p, 0.75f, 0.1f, 0.75f, 1.8f, 0.35f); delay (p, 5, 0.3f); macros (m, 22, 15, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Guitarra Acústica", "Acústica de cuerdas de acero, brillante y larga.", "A steel-string acoustic, bright and long.",
            guitar (p, 0.7f, 0.12f, 0.8f, 4.0f, 0.4f); pluck (B (p), 0.7f, 0.13f, 0.8f, 4.0f, 0.3f, 0.3f); B (p)[lp::fine] = 4.0f;
            macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Guitarra Eléctrica Limpia", "Eléctrica limpia con chorus.", "A clean electric with chorus.",
            guitar (p, 0.65f, 0.2f, 0.7f, 3.5f, 0.0f); filt (p, filterLP12, 4500, 0.15f); chorus (p, 0.4f);
            macros (m, 20, 15, charClean, 0, playPoly, 0, 0, 70); ),
        DMB_ARCH ("Guitarra Muteada", "Cuerdas apagadas con la palma: corto y rítmico.", "Palm-muted strings: short and rhythmic.",
            guitar (p, 0.4f, 0.15f, 0.2f, 0.25f, 0.4f); amp (p, 0.001f, 0.3f, 0, 0.05f); macros (m, 12, 10, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Mandolina", "Cuerdas dobles brillantes.", "Bright doubled strings.",
            guitar (p, 0.8f, 0.1f, 0.8f, 1.5f, 0.3f); pluck (B (p), 0.8f, 0.1f, 0.8f, 1.5f, 0.3f, 0.9f); B (p)[lp::fine] = 7.0f;
            macros (m, 20, 0); ),
        DMB_ARCH ("Guitarra 12 Cuerdas", "Cada cuerda con su octava: brillante y amplia.", "Each string with its octave: bright and wide.",
            guitar (p, 0.65f, 0.13f, 0.75f, 3.5f, 0.4f); pluck (B (p), 0.7f, 0.12f, 0.8f, 3.0f, 0.2f, 0.6f, 1); B (p)[lp::fine] = 3.0f;
            macros (m, 22, 0); ),
        DMB_ARCH ("Guitarra Funk", "Eléctrica rasguñada, seca y brillante.", "A scratchy electric, dry and bright.",
            guitar (p, 0.85f, 0.12f, 0.6f, 0.6f, 0.0f); filt (p, filterLP12, 3000, 0.2f); amp (p, 0.001f, 0.6f, 0.6f, 0.06f);
            macros (m, 10, 10, charClean, 10, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Guitarra Jazz", "Eléctrica oscura y cálida.", "A dark, warm electric.",
            guitar (p, 0.25f, 0.25f, 0.3f, 3.0f, 0.2f); filt (p, filterLP12, 2200); macros (m, 22, 0, charWarm, 25); ),
        DMB_ARCH ("Guitarra Lofi", "Nylon grabada en casete.", "A nylon recorded on cassette.",
            guitar (p, 0.45f, 0.17f, 0.5f, 2.6f, 0.6f); macros (m, 22, 0, charLofi, 55); ),
        DMB_ARCH ("Cuatro Puertorriqueño", "Cuerdas dobles con octava, brillante: el cuatro de la isla.",
                  "Doubled strings with octave, bright: the island's cuatro.",
            guitar (p, 0.85f, 0.08f, 0.85f, 1.8f, 0.3f); pluck (B (p), 0.85f, 0.09f, 0.85f, 1.8f, 0.3f, 0.8f, 1); B (p)[lp::fine] = 4.0f;
            macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Tres Cubano", "Cuerdas dobles desafinadas: el sonido del tres.", "Detuned doubled strings: the sound of the tres.",
            guitar (p, 0.75f, 0.1f, 0.75f, 2.0f, 0.4f); pluck (B (p), 0.75f, 0.11f, 0.75f, 2.0f, 0.4f, 0.8f); B (p)[lp::fine] = 7.0f;
            macros (m, 20, 0, charClean, 0, playPoly, 0, 0, 75); ),
        DMB_ARCH ("Guitarra Española", "Nylon con ataque fuerte, como de flamenco.", "A nylon with a strong attack, flamenco-like.",
            guitar (p, 0.7f, 0.1f, 0.6f, 2.2f, 0.55f); macros (m, 22, 0, charClean, 0, playPoly, 0, 0, 80); ),
        DMB_ARCH ("Guitarra Eco", "Eléctrica limpia con eco con puntillo.", "A clean electric with a dotted echo.",
            guitar (p, 0.65f, 0.2f, 0.7f, 3.0f, 0.0f); filt (p, filterLP12, 4500, 0.15f); delay (p, 5, 0.4f);
            macros (m, 22, 35); ),
        DMB_ARCH ("Guitarra Susurro", "Nylon muy suave, tocada con las yemas.", "A very soft nylon, played with the fingertips.",
            guitar (p, 0.25f, 0.22f, 0.4f, 3.0f, 0.6f); amp (p, 0.004f, 8, 1, 0.35f); macros (m, 30, 0, charWarm, 15, playPoly, 0, 0, 55); ),
        DMB_ARCH ("Guitarra Armónicos", "Notas de armónico: una octava arriba y cristalinas.", "Harmonic notes: an octave up and glassy.",
            pluck (A (p), 0.4f, 0.5f, 0.7f, 3.0f, 0.2f, 1.0f, 1); filt (p, filterOff, 20000); amp (p, 0.001f, 8, 1, 0.4f);
            reverb (p, 0.7f, 3.0f); macros (m, 35, 0); ),
        DMB_ARCH ("Guitarra Trémolo", "Eléctrica con trémolo de amplificador.", "An electric with amp tremolo.",
            guitar (p, 0.6f, 0.2f, 0.7f, 3.0f, 0.0f); filt (p, filterLP12, 4000, 0.15f); lfo (p, 5.0f, 0, 0, 0.5f);
            macros (m, 25, 0, charWarm, 20); ),
        DMB_ARCH ("Guitarra Fiesta", "Cuerdas brillantes con chorus, para rasgueos alegres.", "Bright strings with chorus, for happy strumming.",
            guitar (p, 0.8f, 0.12f, 0.8f, 2.5f, 0.4f); pluck (B (p), 0.8f, 0.13f, 0.8f, 2.5f, 0.4f, 0.7f); B (p)[lp::fine] = 8.0f;
            chorus (p, 0.3f); macros (m, 22, 0); ),
        DMB_ARCH ("Guitarra Nocturna", "Nylon oscura con mucha sala.", "A dark nylon with lots of room.",
            guitar (p, 0.3f, 0.2f, 0.35f, 3.0f, 0.6f); reverb (p, 0.85f, 4.0f); macros (m, 45, 10, charWarm, 15); ),
    };

    // =========================================================================================
    cats[catFlute] = {
        DMB_ARCH ("Flauta Andina", "Quena con soplido y vibrato.", "A breathy quena with vibrato.",
            flute (p, 6, 0.09f, 0.6f, 0.05f); macros (m, 25, 20, charClean, 0, playLegato, 10, 40, 55); ),
        DMB_ARCH ("Zampoña", "Flauta de pan con su golpe de aire al principio: muy de la época.",
                  "A pan flute with its puff of air at the start: very era-specific.",
            flute (p, 1, 0.12f, 0.8f, 0.03f); macros (m, 25, 15, charClean, 0, playPoly, 0, 25, 55); ),
        DMB_ARCH ("Ocarina", "Ocarina redonda y pura.", "A round, pure ocarina.",
            flute (p, 2, 0.04f, 0.3f, 0.06f); macros (m, 25, 15, charClean, 0, playLegato, 10, 30, 50); ),
        DMB_ARCH ("Flauta Traversa", "Flauta de orquesta.", "An orchestral flute.",
            flute (p, 0, 0.07f, 0.5f, 0.05f); macros (m, 28, 0, charClean, 0, playLegato, 8, 35, 55); ),
        DMB_ARCH ("Piccolo", "Flautín brillante una octava arriba.", "A bright piccolo an octave up.",
            flute (p, 3, 0.05f, 0.5f, 0.04f, 1); macros (m, 25, 0, charClean, 0, playLegato, 8, 30, 55); ),
        DMB_ARCH ("Silbido", "Un silbido humano con portamento.", "A human whistle with glide.",
            flute (p, 4, 0.03f, 0.2f, 0.04f); macros (m, 25, 20, charClean, 0, playLegato, 25, 45, 50); ),
        DMB_ARCH ("Flauta Dulce", "Flauta dulce de escuela, con golpe de aire.", "A school recorder with a puff of air.",
            flute (p, 5, 0.06f, 0.6f, 0.03f); macros (m, 22, 0, charClean, 0, playLegato, 6, 10, 55); ),
        DMB_ARCH ("Clarinete", "Clarinete: madera y armónicos impares.", "A clarinet: woody, with odd harmonics.",
            flute (p, 7, 0.03f, 0.0f, 0.05f); filt (p, filterLP12, 4000); macros (m, 25, 0, charClean, 0, playLegato, 6, 10, 55); ),
        DMB_ARCH ("Flauta Synth", "Flauta de sintetizador: triángulo con aire.", "A synth flute: a triangle with air.",
            va (A (p), waveTri); osc2 (A (p), waveSine, 12, 0, 0.15f); noise (p, 0.06f, 0.4f, 2.5f, 0.4f, 0.05f);
            filt (p, filterLP12, 5000); amp (p, 0.04f, 1, 1, 0.15f); vib (p, 5.2f, 0.35f); reverb (p, 0.6f, 2.4f);
            macros (m, 25, 15, charClean, 0, playLegato, 10, 30, 55); ),
        DMB_ARCH ("Zampoña 2004", "La flauta de pan de los teclados de la época, con eco.", "The era's keyboard pan flute, with echo.",
            flute (p, 1, 0.1f, 0.7f, 0.03f); delay (p, 5, 0.35f); macros (m, 25, 25, charVintage, 50, playPoly, 0, 25, 55); ),
        DMB_ARCH ("Ocarina Grave", "Ocarina grande, una octava abajo.", "A big ocarina, an octave down.",
            flute (p, 2, 0.05f, 0.3f, 0.07f, -1); macros (m, 28, 10, charClean, 0, playLegato, 10, 30, 50); ),
        DMB_ARCH ("Flauta Aire", "Flauta con mucho soplido.", "A flute with lots of breath.",
            flute (p, 0, 0.25f, 0.6f, 0.06f); macros (m, 30, 10, charClean, 0, playLegato, 8, 35, 50); ),
        DMB_ARCH ("Silbato Lead", "Silbido para melodías, con eco.", "A whistle for melodies, with echo.",
            flute (p, 4, 0.04f, 0.3f, 0.03f); delay (p, 5, 0.35f); macros (m, 22, 30, charClean, 0, playLegato, 30, 40, 50); ),
        DMB_ARCH ("Flauta Eco", "Flauta con eco largo.", "A flute with a long echo.",
            flute (p, 0, 0.07f, 0.5f, 0.05f); delay (p, 7, 0.4f); macros (m, 28, 40, charClean, 0, playLegato, 8, 30, 55); ),
        DMB_ARCH ("Flauta Bambú", "Flauta de bambú: entra desde abajo, con mucho aire.", "A bamboo flute: scoops in, with lots of air.",
            flute (p, 6, 0.16f, 0.5f, 0.08f); pitchEnv (p, -0.6f, 0.1f); vib (p, 4.5f, 0.5f); macros (m, 35, 15, charClean, 0, playLegato, 15, 35, 50); ),
        DMB_ARCH ("Flauta Nocturna", "Flauta oscura con sala grande.", "A dark flute with a big room.",
            flute (p, 0, 0.08f, 0.4f, 0.08f); filt (p, filterLP12, 3500); reverb (p, 0.85f, 4.0f); macros (m, 45, 10, charWarm, 15, playLegato, 10, 30, 50); ),
        DMB_ARCH ("Pífano", "Pífano agudo y brillante, de banda.", "A high, bright fife.",
            flute (p, 3, 0.06f, 0.7f, 0.03f, 1); macros (m, 22, 0, charClean, 0, playLegato, 5, 20, 60); ),
        DMB_ARCH ("Ocarina Synth", "Senoidal con golpe de aire: ocarina electrónica.", "A sine with a puff of air: an electronic ocarina.",
            va (A (p), waveSine); noise (p, 0.03f, 0.4f, 3.0f, 0.5f, 0.04f); filt (p, filterOff, 20000); amp (p, 0.03f, 1, 1, 0.15f);
            vib (p, 5.5f, 0.3f); reverb (p, 0.6f, 2.4f); macros (m, 25, 20, charClean, 0, playLegato, 12, 30, 50); ),
        DMB_ARCH ("Flautas Dobles", "Dos flautas en octavas.", "Two flutes in octaves.",
            flute (p, 0, 0.07f, 0.5f, 0.05f); modal (B (p), modalFlute, 1, 0.5f, 0.5f, 0.5f, 0, 0, 0, 0.5f, 1);
            macros (m, 28, 10, charClean, 0, playPoly, 0, 30, 55); ),
        DMB_ARCH ("Flauta Cristal", "Flauta con un brillo de campana.", "A flute with a bell shine.",
            flute (p, 0, 0.06f, 0.4f, 0.05f); fm (B (p), 0, 1, 4, 1.0f, 0.5f, 0.1f, 0, 0.15f); macros (m, 30, 15, charClean, 0, playPoly, 0, 30, 55); ),
    };

    // =========================================================================================
    cats[catChoir] = {
        DMB_ARCH ("Coro Aah", "Coro de altos y sopranos cantando 'a'.", "Altos and sopranos singing 'ah'.",
            choir (p, 2, 0.0f); vox (B (p), 3, 0.0f, 0.12f, 3, 0, 0.6f); macros (m, 40, 0, charClean, 0, playPoly, 0, 12, 50); ),
        DMB_ARCH ("Coro Ooh", "Coro suave cantando 'u'.", "A soft choir singing 'oo'.",
            choir (p, 2, 4.0f); vox (B (p), 3, 4.0f, 0.12f, 3, 0, 0.6f); macros (m, 40, 0, charClean, 0, playPoly, 0, 12, 50); ),
        DMB_ARCH ("Coro Grave", "Bajos y tenores: profundo.", "Basses and tenors: deep.",
            choir (p, 0, 0.0f); vox (B (p), 1, 0.0f, 0.12f, 3, 0, 0.7f); macros (m, 40, 0, charClean, 0, playPoly, 0, 10, 50); ),
        DMB_ARCH ("Coro Soprano", "Sopranos brillantes.", "Bright sopranos.",
            choir (p, 3, 0.0f); filt (p, filterLP12, 9000); macros (m, 40, 0, charClean, 0, playPoly, 0, 15, 50); ),
        DMB_ARCH ("Voz Sintética", "Una sola voz con una sierra: casi un vocoder.", "One voice with a saw: almost a vocoder.",
            vox (A (p), 1, 0.0f, 0.05f, 1); va (B (p), waveSaw, 1, 0, 0.3f, 0.3f); filt (p, filterLP12, 5000); amp (p, 0.02f, 1, 1, 0.3f);
            reverb (p, 0.6f, 2.5f); macros (m, 25, 15, charVintage, 30); ),
        DMB_ARCH ("Coro Iglesia", "Coro en 'o' dentro de una iglesia.", "A choir on 'oh' inside a church.",
            choir (p, 2, 3.0f); reverb (p, 0.95f, 5.0f); amp (p, 0.4f, 1, 1, 1.2f); macros (m, 55, 0, charClean, 0, playPoly, 0, 10, 45); ),
        DMB_ARCH ("Coro Eeh", "Coro en 'e': más brillante.", "A choir on 'eh': brighter.",
            choir (p, 2, 1.0f); macros (m, 40, 0, charClean, 0, playPoly, 0, 12, 50); ),
        DMB_ARCH ("Coro Mmm", "Coro con la boca cerrada.", "A choir humming with closed mouths.",
            choir (p, 1, 4.0f, 0.05f); filt (p, filterLP12, 1200); macros (m, 40, 0, charClean, 0, playPoly, 0, 10, 50); ),
        DMB_ARCH ("Coro Stab", "Golpe corto de coro.", "A short choir stab.",
            choir (p, 2, 0.0f); amp (p, 0.01f, 0.4f, 0, 0.3f); macros (m, 30, 10, charVintage, 35, playPoly, 0, 0, 65); ),
        DMB_ARCH ("Coro Ensueño", "Sopranos en 'u' que entran despacio con chorus.", "Sopranos on 'oo' fading in with chorus.",
            choir (p, 3, 4.0f); amp (p, 0.6f, 1, 1, 1.5f); chorus (p, 0.5f); reverb (p, 0.9f, 5.0f); macros (m, 50, 0, charClean, 0, playPoly, 0, 12, 45); ),
        DMB_ARCH ("Coro Robot", "Una voz robótica con un cuadrado.", "A robotic voice with a square.",
            vox (A (p), 1, 0.0f, 0.0f, 1); va (B (p), waveSquare, 1, 0, 0.3f, 0.4f); filt (p, filterLP12, 4500); amp (p, 0.01f, 1, 1, 0.2f);
            reverb (p, 0.5f, 2.0f); macros (m, 20, 15, charLofi, 40); ),
        DMB_ARCH ("Coro Mixto", "Tenores en 'a' y sopranos en 'o'.", "Tenors on 'ah' and sopranos on 'oh'.",
            choir (p, 1, 0.0f); vox (B (p), 3, 3.0f, 0.12f, 3, 0, 0.6f); macros (m, 40, 0, charClean, 0, playPoly, 0, 12, 50); ),
        DMB_ARCH ("Coro Susurro", "Sopranos con mucho aire, casi susurrando.", "Very breathy sopranos, almost whispering.",
            choir (p, 3, 0.0f, 0.6f); filt (p, filterLP12, 9000); macros (m, 45, 0, charClean, 0, playPoly, 0, 10, 45); ),
        DMB_ARCH ("Coro Sombra", "Bajos en 'o' con sala oscura.", "Basses on 'oh' with a dark room.",
            choir (p, 0, 3.0f); filt (p, filterLP12, 3000); reverb (p, 0.9f, 4.5f, 0.3f); macros (m, 45, 0, charClean, 0, playPoly, 0, 10, 45); ),
        DMB_ARCH ("Coro Vintage", "Coro como sampleado en los 2000.", "A choir as if sampled in the 2000s.",
            choir (p, 2, 0.0f); vox (B (p), 3, 0.0f, 0.12f, 3, 0, 0.5f); macros (m, 38, 0, charVintage, 50, playPoly, 0, 12, 50); ),
        DMB_ARCH ("Coro Lento", "Coro que entra y se va despacio.", "A choir that fades in and out slowly.",
            choir (p, 2, 0.0f); amp (p, 1.0f, 1, 1, 1.5f); macros (m, 45, 0, charClean, 0, playPoly, 0, 12, 45); ),
        DMB_ARCH ("Coro Aire", "Coro en 'u' con aire.", "A breathy choir on 'oo'.",
            choir (p, 2, 4.0f, 0.35f); noise (p, 0.05f, 0.8f, 0.8f); macros (m, 45, 0, charClean, 0, playPoly, 0, 10, 45); ),
        DMB_ARCH ("Coro Cine", "Tenores con bajos una octava abajo y sala gigante.", "Tenors with basses an octave below and a giant hall.",
            choir (p, 1, 0.0f); vox (B (p), 0, 0.0f, 0.12f, 3, 0, 0.7f, -1); reverb (p, 0.95f, 5.0f); macros (m, 55, 0, charClean, 0, playPoly, 0, 10, 45); ),
        DMB_ARCH ("Coro Pop", "Sopranos brillantes en 'e' con eco.", "Bright sopranos on 'eh' with echo.",
            choir (p, 3, 1.0f); filt (p, filterLP12, 9000); amp (p, 0.05f, 1, 1, 0.5f); delay (p, 5, 0.3f); macros (m, 35, 20, charClean, 0, playPoly, 0, 12, 55); ),
        DMB_ARCH ("Coro Oh", "Altos y sopranos en 'o'.", "Altos and sopranos on 'oh'.",
            choir (p, 2, 3.0f); vox (B (p), 3, 3.0f, 0.12f, 3, 0, 0.6f); macros (m, 40, 0, charClean, 0, playPoly, 0, 12, 50); ),
    };

    // =========================================================================================
    cats[catFx] = {
        DMB_ARCH ("Bocina de Fiesta", "La corneta de aire de las fiestas: sierras en acorde con caída.", "The party air horn: chorded saws with a dip.",
            va (A (p), waveSaw, 5, 30, 0.8f); osc2 (A (p), waveSaw, 7, 10, 0.6f); va (B (p), waveSaw, 3, 25, 0.6f, 0.7f, 1);
            filt (p, filterLP24, 3000, 0.25f, 1.0f); fenv (p, 0.001f, 0.2f, 0.7f, 0.1f); amp (p, 0.003f, 1, 1, 0.1f);
            pitchEnv (p, 1.0f, 0.06f); lfoPitch (p, 7.0f, 0, 0.15f); reverb (p, 0.6f, 2.0f); macros (m, 25, 25, charVintage, 45); ),
        DMB_ARCH ("Sirena Policía", "Sube y baja sin parar, como sirena.", "Goes up and down nonstop, like a siren.",
            va (A (p), waveSaw); osc2 (A (p), waveSquare, 0, 0, 0.5f); lfoPitch (p, 0.6f, 1, 7.0f); filt (p, filterLP12, 5000);
            amp (p, 0.01f, 1, 1, 0.3f); reverb (p, 0.6f, 2.0f); macros (m, 25, 20); ),
        DMB_ARCH ("Láser", "Cuadrado que cae tres octavas en un instante.", "A square falling three octaves in an instant.",
            va (A (p), waveSquare); pitchEnv (p, 36, 0.12f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.35f, 0, 0.1f);
            delay (p, 3, 0.4f); macros (m, 20, 30); ),
        DMB_ARCH ("Zap", "Disparo eléctrico corto.", "A short electric zap.",
            fm (A (p), 0, 1, 1.5f, 6.0f, 0.08f, 0.0f); pitchEnv (p, 24, 0.04f); filt (p, filterOff, 20000); amp (p, 0.001f, 0.2f, 0, 0.05f);
            macros (m, 15, 20); ),
        DMB_ARCH ("Riser Ruido", "Ruido que sube mientras sostienes la nota.", "Noise that rises while you hold the note.",
            va (A (p), waveNoise); filt (p, filterLP24, 300, 0.4f, 6.0f, 0.0f); fenv (p, 4.0f, 1, 1, 0.5f); amp (p, 3.5f, 1, 1, 0.4f);
            reverb (p, 0.8f, 3.0f); macros (m, 30, 20); ),
        DMB_ARCH ("Caída Libre", "Cae dos octavas hasta la nota.", "Falls two octaves down to the note.",
            va (A (p), waveSaw, 3, 15, 0.7f); pitchEnv (p, 24, 0.8f); filt (p, filterLP24, 4000, 0.2f); amp (p, 0.005f, 1, 1, 0.4f);
            reverb (p, 0.7f, 2.5f); macros (m, 30, 15); ),
        DMB_ARCH ("Viento", "Ráfagas de viento que van y vienen.", "Gusts of wind that come and go.",
            va (A (p), waveNoise); filt (p, filterBP, 800, 0.5f, 0, 0.0f); lfo (p, 0.15f, 0, 1.5f); amp (p, 1.0f, 1, 1, 1.5f);
            reverb (p, 0.8f, 3.5f); macros (m, 40, 0); ),
        DMB_ARCH ("Rebobinar", "Como cinta rebobinando.", "Like a tape rewinding.",
            va (A (p), waveSaw, 3, 20, 0.6f); pitchEnv (p, -24, 0.4f); lfoPitch (p, 8.0f, 0, 2.0f); filt (p, filterLP12, 3000);
            amp (p, 0.01f, 1, 1, 0.2f); macros (m, 20, 10, charLofi, 50); ),
        DMB_ARCH ("Impacto", "Golpe grave con explosión y cola larga (tócalo grave).", "A low boom with a blast and a long tail (play it low).",
            va (A (p), waveSine); pitchEnv (p, 24, 0.06f); noise (p, 0.0f, 0.3f, 0.6f, 1.0f, 0.25f); filt (p, filterOff, 20000);
            amp (p, 0.001f, 1.2f, 0, 0.5f); reverb (p, 0.95f, 5.0f); macros (m, 50, 0); ),
        DMB_ARCH ("Burbujas", "Notas al azar que burbujean.", "Random bubbling notes.",
            va (A (p), waveSine); lfoPitch (p, 7.0f, 3, 12.0f); filt (p, filterOff, 20000); amp (p, 0.005f, 1, 1, 0.2f);
            delay (p, 3, 0.4f); reverb (p, 0.6f, 2.0f); macros (m, 30, 30); ),
        DMB_ARCH ("Alarma", "Dos tonos que se alternan rápido.", "Two tones alternating fast.",
            va (A (p), waveSquare); lfoPitch (p, 4.0f, 2, 5.0f); filt (p, filterLP12, 4000); amp (p, 0.002f, 1, 1, 0.05f); macros (m, 20, 10); ),
        DMB_ARCH ("OVNI", "Senoidal que tiembla y se desliza.", "A trembling, gliding sine.",
            va (A (p), waveSine); osc2 (A (p), waveTri, 12, 0, 0.2f); lfoPitch (p, 9.0f, 0, 1.0f); filt (p, filterOff, 20000);
            amp (p, 0.05f, 1, 1, 0.4f); reverb (p, 0.8f, 3.5f); macros (m, 40, 20, charClean, 0, playLegato, 60, 20); ),
        DMB_ARCH ("Explosión 8 Bits", "Explosión de videojuego.", "A video-game explosion.",
            va (A (p), waveNoise); filt (p, filterLP24, 300, 0.1f, 4.5f, 0.0f); fenv (p, 0.001f, 1.0f, 0, 0.5f); amp (p, 0.001f, 1.2f, 0, 0.4f);
            macros (m, 20, 0, charLofi, 80); ),
        DMB_ARCH ("Barrido", "Ruido que barre de arriba abajo.", "Noise sweeping from top to bottom.",
            va (A (p), waveNoise); filt (p, filterLP24, 200, 0.5f, 6.5f, 0.0f); fenv (p, 0.001f, 1.5f, 0, 0.8f); amp (p, 0.001f, 2, 0, 0.8f);
            reverb (p, 0.7f, 2.5f); macros (m, 30, 15); ),
        DMB_ARCH ("Disparo", "Ruido y tono que caen de golpe.", "Noise and tone dropping at once.",
            va (A (p), waveNoise, 1, 0, 0.3f, 0.6f); va (B (p), waveSine, 1, 0, 0.3f, 0.8f); pitchEnv (p, 30, 0.05f);
            filt (p, filterLP24, 3000, 0.1f, 3.0f, 0.0f); fenv (p, 0.001f, 0.15f, 0, 0.1f); amp (p, 0.001f, 0.3f, 0, 0.1f); macros (m, 20, 15); ),
        DMB_ARCH ("Ola", "Olas del mar que van y vienen.", "Sea waves coming and going.",
            va (A (p), waveNoise); filt (p, filterLP24, 600, 0.2f, 0, 0.0f); lfo (p, 0.12f, 0, 2.5f, 0.4f); amp (p, 1.5f, 1, 1, 2.0f);
            reverb (p, 0.8f, 3.5f); macros (m, 40, 0); ),
        DMB_ARCH ("Radio Perdida", "Una estación de radio que no se sintoniza.", "A radio station that won't tune in.",
            va (A (p), waveSine); va (B (p), waveNoise, 1, 0, 0.3f, 0.5f); filt (p, filterBP, 1500, 0.4f, 0, 0.0f); lfoPitch (p, 0.3f, 3, 0.5f);
            amp (p, 0.02f, 1, 1, 0.3f); macros (m, 25, 0, charLofi, 90); ),
        DMB_ARCH ("Pitido", "Pitido agudo y corto, con eco.", "A short high beep, with echo.",
            va (A (p), waveSine, 1, 0, 0.3f, 1.0f, 2); filt (p, filterOff, 20000); amp (p, 0.001f, 0.12f, 0, 0.05f); delay (p, 3, 0.4f);
            macros (m, 20, 35); ),
        DMB_ARCH ("Subida Supersaw", "Siete sierras que suben dos octavas hasta la nota.", "Seven saws rising two octaves into the note.",
            va (A (p), waveSaw, 7, 25, 0.9f); pitchEnv (p, -24, 1.4f); filt (p, filterLP24, 600, 0.2f, 3.5f); fenv (p, 3.0f, 1, 1, 0.5f);
            amp (p, 0.5f, 1, 1, 0.5f); reverb (p, 0.8f, 3.0f); macros (m, 30, 25); ),
        DMB_ARCH ("Golpe Reverso", "Crece mientras sostienes y se corta al soltar, como un sonido al revés.",
                  "Swells while you hold and cuts when you release, like a reversed sound.",
            va (A (p), waveSaw, 5, 15, 0.8f); filt (p, filterLP24, 1500, 0.1f, 1.0f); fenv (p, 1.2f, 1, 1, 0.05f); amp (p, 1.2f, 1, 1, 0.02f);
            reverb (p, 0.9f, 4.0f); macros (m, 50, 0); ),
    };
}
} // namespace dmb
