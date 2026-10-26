#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "I18n.h"
#include "synth/Patch.h"

namespace dmb
{
// Controles que el usuario mueve (y que el DAW puede automatizar).
namespace pid
{
    inline constexpr const char* brillo   = "brillo";
    inline constexpr const char* duracion = "duracion";
    inline constexpr const char* espacio  = "espacio";
    inline constexpr const char* ataque   = "ataque";
    inline constexpr const char* grosor   = "grosor";
    inline constexpr const char* caracter = "caracter";
    inline constexpr const char* suciedad = "suciedad";
    inline constexpr const char* vibrato  = "vibrato";
    inline constexpr const char* glide    = "glide";
    inline constexpr const char* modo     = "modo";
    inline constexpr const char* octava   = "octava";
    inline constexpr const char* fuerza   = "fuerza";
    inline constexpr const char* eco      = "eco";
    inline constexpr const char* volumen  = "volumen";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

juce::StringArray characterNames();   // en el idioma actual
juce::StringArray modeNames();

// Lee las macros de los parámetros (en el hilo de audio o en el de la interfaz).
Macros readMacros (juce::AudioProcessorValueTreeState&);
// Escribe las macros en los parámetros (al cargar un preset).
void writeMacros (juce::AudioProcessorValueTreeState&, const Macros&);

// Texto de los controles relativos (50 = como viene el sonido)
juce::String relativeText (float v);
} // namespace dmb
