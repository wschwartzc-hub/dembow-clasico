#pragma once

#include "../synth/Patch.h"
#include <vector>

namespace dmb
{
// Los 16 tipos de sonido. Cada uno trae 500 presets: 20 sonidos base × 25 variaciones.
enum Category : int
{
    catLead = 0, catPluck, catBass, catPad,
    catPiano, catOrgan, catBells, catMallets,
    catStrings, catPizz, catBrass, catHit,
    catGuitar, catFlute, catChoir, catFx,
    numCategories
};

inline constexpr int archetypesPerCategory = 20;
inline constexpr int variationsPerArchetype = 25;
inline constexpr int presetsPerCategory = archetypesPerCategory * variationsPerArchetype;   // 500
inline constexpr int numFactoryPresets = numCategories * presetsPerCategory;                // 8000

// Un sonido base: la receta y su descripción.
struct Archetype
{
    const char* name;          // nombre propio (igual en los dos idiomas)
    const char* descEs;
    const char* descEn;
    void (*build) (Patch&, Macros&);
};

// cada archivo de presets llena las categorías que le tocan
void addLeadsPlucksBassPads (std::vector<Archetype> (&cats)[numCategories]);
void addKeysOrganBellsMallets (std::vector<Archetype> (&cats)[numCategories]);
void addOrchestra (std::vector<Archetype> (&cats)[numCategories]);
void addGuitarFluteChoirFx (std::vector<Archetype> (&cats)[numCategories]);

const std::vector<Archetype>& archetypes (int category);

struct PresetRef { int category = 0, archetype = 0, variation = 0; };
PresetRef presetRef (int index);
int presetIndex (int category, int archetype, int variation);

juce::String categoryName (int category);          // en el idioma actual
juce::String categoryKey (int category);           // "lead", "pluck"… (para archivos e íconos)
juce::String variationName (int category, int variation);
juce::String presetName (int index);               // "Lead Marquesina · Vintage"
juce::String presetDescription (int index);

// Arma el preset completo (sin nivelar el volumen).
void buildPreset (int index, Patch&, Macros&);

// Mide qué tan fuerte suena y devuelve cuántos dB hay que sumarle para que todos suenen parejo.
float measureLevelCorrection (const Patch&, const Macros&, int category);

// Arma el preset y lo deja nivelado (lo que usa el plugin).
void buildCalibratedPreset (int index, Patch&, Macros&);

// Un sonido nuevo al azar dentro del tipo (botón del dado).
void randomPreset (int category, uint32_t seed, Patch&, Macros&, juce::String& name);
} // namespace dmb
