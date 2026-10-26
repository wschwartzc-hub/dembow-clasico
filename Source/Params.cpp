#include "Params.h"

namespace dmb
{
namespace
{
    // Parámetros cuyo nombre (el que ve el DAW en la automatización) sigue el idioma elegido.
    template <typename Base>
    struct Named : public Base
    {
        template <typename... Args>
        Named (const char* es, const char* en, Args&&... args)
            : Base (std::forward<Args> (args)...), nameEs (es), nameEn (en) {}

        juce::String getName (int maximumLength) const override
        {
            return T (nameEs, nameEn).substring (0, maximumLength);
        }

        const char* nameEs;
        const char* nameEn;
    };

    using FloatP  = Named<juce::AudioParameterFloat>;
    using ChoiceP = Named<juce::AudioParameterChoice>;
    using IntP    = Named<juce::AudioParameterInt>;
}

juce::StringArray characterNames()
{
    return { T ("Limpio", "Clean"), U ("Vintage"), T ("Cálido", "Warm"), U ("Lo-Fi") };
}

juce::StringArray modeNames()
{
    return { U ("Poly"), U ("Mono"), U ("Legato") };
}

juce::String relativeText (float v)
{
    const int d = juce::roundToInt ((v - 50.0f) * 2.0f);
    if (d == 0) return T ("Original", "Original");
    return (d > 0 ? "+" : U ("−")) + juce::String (std::abs (d)) + " %";
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto pct = [] (float v, int) { return String (roundToInt (v)) + " %"; };
    auto num = [] (const String& t) { return t.retainCharacters ("0123456789.-").getFloatValue(); };
    auto off = [] { return T ("Apagado", "Off"); };
    auto rel = [] (float v, int) { return relativeText (v); };
    auto relFromText = [] (const String& t)
    {
        if (t.containsIgnoreCase ("orig")) return 50.0f;
        const float d = t.retainCharacters ("0123456789.").getFloatValue();
        return 50.0f + (t.contains ("-") || t.contains (U ("−")) ? -d : d) * 0.5f;
    };
    auto attr = [&] (std::function<String (float, int)> toText, std::function<float (const String&)> fromText)
    {
        return AudioParameterFloatAttributes().withStringFromValueFunction (std::move (toText))
                                              .withValueFromStringFunction (std::move (fromText));
    };
    const NormalisableRange<float> range100 (0.0f, 100.0f, 0.1f);

    // --- los tres grandes ------------------------------------------------------------------------
    layout.add (std::make_unique<FloatP> ("Brillo", "Brightness", ParameterID { pid::brillo, 1 }, U ("Brillo"), range100, 50.0f, attr (rel, relFromText)));
    layout.add (std::make_unique<FloatP> ("Duración", "Length", ParameterID { pid::duracion, 1 }, U ("Duración"), range100, 50.0f, attr (rel, relFromText)));
    layout.add (std::make_unique<FloatP> ("Espacio", "Space", ParameterID { pid::espacio, 1 }, U ("Espacio"), range100, 15.0f,
        attr ([pct, off] (float v, int l) { return v < 0.5f ? off() : pct (v, l); }, num)));

    // --- forma -----------------------------------------------------------------------------------
    layout.add (std::make_unique<FloatP> ("Ataque", "Attack", ParameterID { pid::ataque, 1 }, U ("Ataque"), range100, 50.0f, attr (rel, relFromText)));
    layout.add (std::make_unique<FloatP> ("Grosor", "Thickness", ParameterID { pid::grosor, 1 }, U ("Grosor"), range100, 50.0f, attr (rel, relFromText)));

    // --- carácter --------------------------------------------------------------------------------
    layout.add (std::make_unique<ChoiceP> ("Carácter", "Character", ParameterID { pid::caracter, 1 }, U ("Carácter"),
        StringArray { "Limpio", "Vintage", "Calido", "Lo-Fi" }, charClean,
        AudioParameterChoiceAttributes().withStringFromValueFunction ([] (int i, int) { return characterNames()[i]; })));
    layout.add (std::make_unique<FloatP> ("Suciedad", "Dirt", ParameterID { pid::suciedad, 1 }, U ("Suciedad"), range100, 0.0f,
        attr ([pct, off] (float v, int l) { return v < 0.5f ? off() : pct (v, l); }, num)));

    // --- movimiento ------------------------------------------------------------------------------
    layout.add (std::make_unique<FloatP> ("Vibrato", "Vibrato", ParameterID { pid::vibrato, 1 }, U ("Vibrato"), range100, 0.0f,
        attr ([pct, off] (float v, int l) { return v < 0.5f ? off() : pct (v, l); }, num)));
    layout.add (std::make_unique<FloatP> ("Glide", "Glide", ParameterID { pid::glide, 1 }, U ("Glide"), range100, 0.0f,
        attr ([off] (float v, int)
        {
            if (v < 0.5f) return off();
            const float g = v * 0.01f;
            return String (roundToInt ((0.01f + g * g * 1.2f) * 1000.0f)) + " ms";
        }, num)));

    // --- tocar -----------------------------------------------------------------------------------
    layout.add (std::make_unique<ChoiceP> ("Modo", "Play mode", ParameterID { pid::modo, 1 }, U ("Modo"),
        StringArray { "Poly", "Mono", "Legato" }, playPoly,
        AudioParameterChoiceAttributes().withStringFromValueFunction ([] (int i, int) { return modeNames()[i]; })));
    layout.add (std::make_unique<IntP> ("Octava", "Octave", ParameterID { pid::octava, 1 }, U ("Octava"), -2, 2, 0,
        AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int) { return v == 0 ? String ("0") : (v > 0 ? "+" : U ("−")) + String (std::abs (v)); })));
    layout.add (std::make_unique<FloatP> ("Fuerza", "Velocity", ParameterID { pid::fuerza, 1 }, U ("Fuerza"), range100, 60.0f, attr (pct, num)));

    // --- salida ----------------------------------------------------------------------------------
    layout.add (std::make_unique<FloatP> ("Eco", "Echo", ParameterID { pid::eco, 1 }, U ("Eco"), range100, 0.0f,
        attr ([pct, off] (float v, int l) { return v < 0.5f ? off() : pct (v, l); }, num)));
    layout.add (std::make_unique<FloatP> ("Volumen", "Volume", ParameterID { pid::volumen, 1 }, U ("Volumen"),
        NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f,
        attr ([] (float v, int) { return (v > 0.04f ? "+" : "") + String (v, 1) + " dB"; }, num)));

    return layout;
}

Macros readMacros (juce::AudioProcessorValueTreeState& a)
{
    auto v = [&a] (const char* id) { return a.getRawParameterValue (id)->load(); };
    Macros m;
    m.brillo = v (pid::brillo);
    m.duracion = v (pid::duracion);
    m.ataque = v (pid::ataque);
    m.grosor = v (pid::grosor);
    m.suciedad = v (pid::suciedad);
    m.vibrato = v (pid::vibrato);
    m.glide = v (pid::glide);
    m.fuerza = v (pid::fuerza);
    m.espacio = v (pid::espacio);
    m.eco = v (pid::eco);
    m.volumen = v (pid::volumen);
    m.caracter = juce::roundToInt (v (pid::caracter));
    m.modo = juce::roundToInt (v (pid::modo));
    m.octava = juce::roundToInt (v (pid::octava));
    return m;
}

void writeMacros (juce::AudioProcessorValueTreeState& a, const Macros& m)
{
    auto set = [&a] (const char* id, float value)
    {
        if (auto* p = a.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };
    set (pid::brillo, m.brillo);
    set (pid::duracion, m.duracion);
    set (pid::ataque, m.ataque);
    set (pid::grosor, m.grosor);
    set (pid::suciedad, m.suciedad);
    set (pid::vibrato, m.vibrato);
    set (pid::glide, m.glide);
    set (pid::fuerza, m.fuerza);
    set (pid::espacio, m.espacio);
    set (pid::eco, m.eco);
    set (pid::volumen, m.volumen);
    set (pid::caracter, (float) m.caracter);
    set (pid::modo, (float) m.modo);
    set (pid::octava, (float) m.octava);
}
} // namespace dmb
