#include "Keyboard.h"
#include "../PluginProcessor.h"

namespace dmb
{
namespace
{
    constexpr int keysX = 22, keysY = 58, numOctaves = 5;
}

KeyboardDrawer::KeyboardDrawer (DembowProcessor& p) : proc (p), keys (p.keyboardState)
{
    bg = juce::ImageCache::getFromMemory (DmbAssets::drawer_bg_jpg, DmbAssets::drawer_bg_jpgSize);
    keys.setScrollButtonsVisible (false);
    keys.setBlackNoteLengthProportion (0.62f);
    keys.setVelocity (0.85f, true);
    keys.setMidiChannel (1);
    keys.setOctaveForMiddleC (3);
    addAndMakeVisible (keys);

    for (auto* b : { &downButton, &upButton })
        addAndMakeVisible (b);
    downButton.onClick = [this] { shiftOctave (-1); };
    upButton.onClick   = [this] { shiftOctave (+1); };
    setHelpProps (downButton, U ("BAJAR OCTAVA"), U ("OCTAVE DOWN"), U ("Muestra las notas más graves."), U ("Shows lower notes."));
    setHelpProps (upButton, U ("SUBIR OCTAVA"), U ("OCTAVE UP"), U ("Muestra las notas más agudas."), U ("Shows higher notes."));
    setHelpProps (keys, U ("TECLADO"), U ("KEYBOARD"),
                  U ("Toca con el mouse, o con tu teclado: A W S E D F T G Y H U J K = una octava; Z / X cambian de octava."),
                  U ("Play with the mouse, or with your computer keys: A W S E D F T G Y H U J K = one octave; Z / X change octave."));
    updateRange();
}

KeyboardDrawer::~KeyboardDrawer() = default;

void KeyboardDrawer::updateRange()
{
    const int low = 12 * (proc.keyboardOctave + 1);
    keys.setAvailableRange (juce::jlimit (0, 127, low), juce::jlimit (0, 127, low + 12 * numOctaves));
    keys.setLowestVisibleKey (low);
    keys.setKeyPressBaseOctave (juce::jlimit (0, 10, proc.keyboardOctave + 3));   // A S D F… tocan dos octavas arriba (do central por defecto)
    resized();
    repaint();
}

void KeyboardDrawer::shiftOctave (int delta)
{
    proc.keyboardOctave = juce::jlimit (0, 4, proc.keyboardOctave + delta);
    updateRange();
}

void KeyboardDrawer::resized()
{
    const int w = width - 2 * keysX;
    keys.setBounds (keysX, keysY, w, height - keysY - 20);
    keys.setKeyWidth ((float) w / (float) (7 * numOctaves + 1));
    downButton.setBounds (width - 250, 12, 40, 34);
    upButton.setBounds (width - 64, 12, 40, 34);
}

void KeyboardDrawer::paint (juce::Graphics& g)
{
    g.drawImage (bg, getLocalBounds().toFloat());
    g.setFont (font (17.0f, false, 0.12f));
    g.setColour (juce::Colour (0xffbec8d4));
    g.drawText (T ("TECLADO", "KEYBOARD"), juce::Rectangle<float> (26.0f, 14.0f, 200.0f, 30.0f), juce::Justification::centredLeft);
    g.setFont (font (14.5f));
    g.setColour (colours::helpDesc);
    g.drawText (T ("Toca con el mouse o con tu teclado (A S D F G H J = blancas, W E T Y U = negras)",
                   "Play with the mouse or your computer keys (A S D F G H J = white, W E T Y U = black)"),
                juce::Rectangle<float> (160.0f, 14.0f, 1000.0f, 30.0f), juce::Justification::centredLeft, true);

    // selector de octava
    const int low = 12 * (proc.keyboardOctave + 1);
    g.setFont (font (17.0f, true, 0.08f));
    g.setColour (colours::lime);
    g.drawText (juce::MidiMessage::getMidiNoteName (low, true, true, 3) + U (" – ") + juce::MidiMessage::getMidiNoteName (low + 12 * numOctaves, true, true, 3),
                juce::Rectangle<float> ((float) width - 206.0f, 12.0f, 138.0f, 34.0f), juce::Justification::centred);
    for (int k = 0; k < 2; ++k)
    {
        auto r = (k == 0 ? downButton : upButton).getBounds().toFloat().reduced (2.0f);
        g.setColour (juce::Colour (0xff141617));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (juce::Colour (0xff4a5054));
        g.drawRoundedRectangle (r, 4.0f, 1.2f);
        juce::Path t;
        const auto c = r.getCentre();
        if (k == 1) t.addTriangle (c.x - 4.0f, c.y - 6.0f, c.x - 4.0f, c.y + 6.0f, c.x + 5.0f, c.y);
        else        t.addTriangle (c.x + 4.0f, c.y - 6.0f, c.x + 4.0f, c.y + 6.0f, c.x - 5.0f, c.y);
        g.setColour (colours::text);
        g.fillPath (t);
    }
}

void KeyboardDrawer::Keys::drawWhiteNote (int note, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver,
                                          juce::Colour, juce::Colour)
{
    auto r = area.reduced (1.0f, 0.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (isDown ? 0xffb9ef72 : (isOver ? 0xffe9ece8 : 0xffd9dcd8)), r.getX(), r.getY(),
                                             juce::Colour (isDown ? 0xff79c626 : 0xffaeb3b0), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);
    if (note % 12 == 0)
    {
        g.setFont (font (12.5f, true));
        g.setColour (juce::Colour (0xff3a3f42));
        g.drawText (juce::MidiMessage::getMidiNoteName (note, true, true, 3), r.withTrimmedBottom (6.0f), juce::Justification::centredBottom, false);
    }
}

void KeyboardDrawer::Keys::drawBlackNote (int, juce::Graphics& g, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour)
{
    auto r = area.reduced (1.5f, 0.0f);
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (r.translated (1.5f, 2.0f), 3.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (isDown ? 0xff8ee63b : (isOver ? 0xff3a3f42 : 0xff222527)), r.getX(), r.getY(),
                                             juce::Colour (isDown ? 0xff4f8f12 : 0xff0d0e0f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawLine (r.getX() + 3.0f, r.getY() + 2.0f, r.getX() + 3.0f, r.getBottom() - 6.0f, 1.2f);
}
} // namespace dmb
