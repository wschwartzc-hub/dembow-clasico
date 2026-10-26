#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "DmbAssets.h"
#include "../I18n.h"

namespace dmb
{
// Todas las posiciones de la interfaz están en "pixeles del diseño" (la imagen original de 1671 x 941).
// La ventana muestra solo el aparato: de (12, 26) a (1662, 910).
namespace layout
{
    inline constexpr int designW = 1671, designH = 941;
    inline constexpr int viewX = 12, viewY = 26, viewW = 1650, viewH = 884;
}

namespace colours
{
    inline const juce::Colour lime       { 0xffadf35a };   // valores
    inline const juce::Colour arc        { 0xff66c11d };   // arco de los knobs
    inline const juce::Colour arcLight   { 0xffa8e466 };
    inline const juce::Colour ring       { 0xffa5c84e };   // anillos del murciélago
    inline const juce::Colour echoL      { 0xff92f041 };
    inline const juce::Colour echoR      { 0xff4fdac5 };
    inline const juce::Colour text       { 0xffe6e7ea };
    inline const juce::Colour textDim    { 0xff9daab6 };
    inline const juce::Colour header     { 0xffa7b7c7 };
    inline const juce::Colour bpm        { 0xffb5c3cf };
    inline const juce::Colour helpDesc   { 0xff8394a3 };
    inline const juce::Colour axis       { 0xff919ba3 };
    inline const juce::Colour grid       { 0xff303333 };
    inline const juce::Colour centreLine { 0xffa2ada1 };
    inline const juce::Colour dryBar     { 0xffafafad };
    inline const juce::Colour engraved   { 0xff6e767d };
    inline const juce::Colour menuBg     { 0xff141617 };
    inline const juce::Colour menuEdge   { 0xff3a3f42 };
}

namespace fonts
{
    inline juce::Typeface::Ptr medium()
    {
        static auto tf = juce::Typeface::createSystemTypefaceFor (DmbAssets::BarlowMedium_ttf, DmbAssets::BarlowMedium_ttfSize);
        return tf;
    }

    inline juce::Typeface::Ptr semibold()
    {
        static auto tf = juce::Typeface::createSystemTypefaceFor (DmbAssets::BarlowSemiBold_ttf, DmbAssets::BarlowSemiBold_ttfSize);
        return tf;
    }
}

// Tamaño en pixeles del diseño: altura de las mayúsculas ≈ 0.7 × size (como en el mockup).
inline juce::Font font (float size, bool bold = false, float tracking = 0.0f)
{
    return juce::Font (juce::FontOptions (bold ? fonts::semibold() : fonts::medium())
                           .withPointHeight (size)
                           .withKerningFactor (tracking));
}

inline juce::Image loadImage (const void* data, int size)
{
    return juce::ImageCache::getFromMemory (data, size);
}

class DmbLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DmbLookAndFeel()
    {
        using namespace colours;
        setColour (juce::PopupMenu::backgroundColourId,            menuBg);
        setColour (juce::PopupMenu::textColourId,                  text);
        setColour (juce::PopupMenu::headerTextColourId,            lime);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, arc);
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colour (0xff0d1406));
        setColour (juce::AlertWindow::backgroundColourId,          menuBg);
        setColour (juce::AlertWindow::textColourId,                text);
        setColour (juce::AlertWindow::outlineColourId,             menuEdge);
        setColour (juce::TextEditor::backgroundColourId,           juce::Colour (0xff0b0c0c));
        setColour (juce::TextEditor::textColourId,                 text);
        setColour (juce::TextEditor::outlineColourId,              menuEdge);
        setColour (juce::TextEditor::focusedOutlineColourId,       lime);
        setColour (juce::TextEditor::highlightColourId,            lime.withAlpha (0.35f));
        setColour (juce::CaretComponent::caretColourId,            lime);
        setColour (juce::TextButton::buttonColourId,               juce::Colour (0xff1b1d1e));
        setColour (juce::TextButton::buttonOnColourId,             arc);
        setColour (juce::TextButton::textColourOffId,              text);
        setColour (juce::TextButton::textColourOnId,               juce::Colour (0xff142007));
    }

    // Botones del aparato: oscuros con borde metálico; encendidos = verde lima con brillo.
    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (b.getProperties().getWithDefault ("glowPad", 0.0f));
        const float cs = 3.5f;

        if (b.getToggleState())
        {
            // brillo exterior
            for (int i = 3; i >= 1; --i)
            {
                g.setColour (colours::arc.withAlpha (0.10f + 0.04f * (float) (3 - i)));
                g.fillRoundedRectangle (r.expanded ((float) i * 2.5f), cs + (float) i * 2.5f);
            }

            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc4f75c), r.getX(), r.getY(),
                                                     juce::Colour (0xff76c61f), r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, cs);

            g.setColour (juce::Colour (0x66ffffff));
            g.fillRoundedRectangle (r.reduced (3.0f, 2.0f).withHeight (r.getHeight() * 0.32f), cs);

            g.setColour (juce::Colour (0xffe0ff9c));
            g.drawRoundedRectangle (r.reduced (0.6f), cs, 1.2f);
            g.setColour (juce::Colour (0x55000000));
            g.drawRoundedRectangle (r.reduced (1.8f), cs - 1.0f, 0.8f);
        }
        else
        {
            g.setColour (juce::Colour (0xaa000000));
            g.fillRoundedRectangle (r.translated (0.0f, 1.5f), cs);

            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff222526), r.getX(), r.getY(),
                                                     juce::Colour (0xff131415), r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, cs);

            g.setColour (juce::Colour (over && b.isEnabled() ? 0xffb9c0c4 : 0xff858b8f)
                             .withAlpha (! b.isEnabled() ? 0.35f : (down ? 0.6f : 0.85f)));
            g.drawRoundedRectangle (r.reduced (0.6f), cs, 1.3f);
            g.setColour (juce::Colour (0x14ffffff));
            g.drawHorizontalLine (juce::roundToInt (r.getY() + 2.0f), r.getX() + 3.0f, r.getRight() - 3.0f);
        }
    }

    juce::Font getTextButtonFont (juce::TextButton& b, int) override
    {
        return font ((float) b.getProperties().getWithDefault ("fontSize", 16.0f), b.getToggleState(), 0.04f);
    }

    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (b.getProperties().getWithDefault ("glowPad", 0.0f));
        g.setFont (getTextButtonFont (b, b.getHeight()));
        g.setColour ((b.getToggleState() ? juce::Colour (0xff142007) : juce::Colour (0xffd2d6d9))
                         .withMultipliedAlpha (b.isEnabled() ? 1.0f : 0.35f));
        g.drawText (b.getButtonText(), r.translated (0.0f, 0.5f), juce::Justification::centred, false);
    }

    juce::Font getPopupMenuFont() override { return font (21.0f); }
    juce::Font getAlertWindowMessageFont() override { return font (16.0f); }
    juce::Font getAlertWindowTitleFont() override { return font (19.0f, true); }

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        g.fillAll (colours::menuBg);
        g.setColour (colours::menuEdge);
        g.drawRect (0, 0, w, h, 1);
    }
};
} // namespace dmb
