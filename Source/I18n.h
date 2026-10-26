#pragma once

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

namespace dmb
{
// Idioma de la interfaz. Es global (todas las ventanas del plugin cambian juntas)
// y se recuerda en la configuración del usuario.
enum class Lang { es = 0, en = 1 };

Lang lang();
void setLang (Lang newLang);
int  langVersion();                      // cambia cada vez que cambia el idioma
inline bool english() { return lang() == Lang::en; }

// Texto en el idioma actual (las dos versiones en UTF-8).
juce::String T (const char* es, const char* en);

// Texto con acentos escrito en el código (UTF-8).
inline juce::String U (const char* utf8) { return juce::String::fromUTF8 (utf8); }

// Mayúsculas / minúsculas que sí entienden acentos (á → Á).
juce::String upper (const juce::String&);
juce::String lower (const juce::String&);

// Archivo de configuración del usuario (idioma, actualizaciones...).
juce::PropertiesFile::Options appSettings();
} // namespace dmb
