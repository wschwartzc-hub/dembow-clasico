#pragma once

#include "Widgets.h"

namespace dmb
{
// Teclado en pantalla: se abre debajo del aparato (como el rack de ECO).
class KeyboardDrawer : public juce::Component
{
public:
    static constexpr int width = 1650, height = 220;

    explicit KeyboardDrawer (DembowProcessor&);
    ~KeyboardDrawer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void shiftOctave (int delta);
    void applyLanguage() { repaint(); }
    juce::MidiKeyboardComponent& getKeys() { return keys; }

private:
    struct Keys : public juce::MidiKeyboardComponent
    {
        Keys (juce::MidiKeyboardState& s) : juce::MidiKeyboardComponent (s, juce::MidiKeyboardComponent::horizontalKeyboard) {}
        void drawWhiteNote (int note, juce::Graphics&, juce::Rectangle<float> area, bool isDown, bool isOver,
                            juce::Colour, juce::Colour) override;
        void drawBlackNote (int note, juce::Graphics&, juce::Rectangle<float> area, bool isDown, bool isOver, juce::Colour) override;
    };

    void updateRange();

    DembowProcessor& proc;
    juce::Image bg;
    Keys keys;
    Hotspot downButton { 5.0f }, upButton { 5.0f };
};
} // namespace dmb
