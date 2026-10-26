#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/Widgets.h"
#include "ui/PresetBrowser.h"
#include "ui/Keyboard.h"
#include "Updater.h"

class DembowEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit DembowEditor (DembowProcessor&);
    ~DembowEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;
    void refreshNow() { timerCallback(); }
    void setKeyboardOpen (bool open);           // abre / cierra el teclado (la ventana crece hacia abajo)
    dmb::PresetBrowser& getBrowser() { return *browser; }
    dmb::KeyboardDrawer& getKeyboard() { return *keyboard; }
    dmb::TypeSelector& getTypeSelector() { return types; }
    void pressBoombox (bool down);

    static constexpr int baseWidth  = dmb::layout::viewW;
    static constexpr int baseHeight = dmb::layout::viewH;
    static constexpr float defaultScale = 0.72f;

    struct Help { const char* titleEs; const char* titleEn; const char* textEs; const char* textEn; };

private:
    struct Stage : public juce::Component
    {
        Stage();
        void paint (juce::Graphics&) override;
        juce::Image plate, cached;
        float cachedScale = 0.0f;
    };

    void timerCallback() override;
    void applyLanguage();

    dmb::PlateKnob& addKnob (dmb::PlateKnob::Spec spec, const char* paramID);
    void addReadout (std::unique_ptr<dmb::Readout> r, juce::Rectangle<int> boundsInDesign);
    void addValue (dmb::PlateKnob& knob, juce::Rectangle<int> area, float size, juce::Colour colour);
    void addLabel (const char* es, const char* en, juce::Rectangle<int> area, juce::Font f, juce::Colour c,
                   juce::Justification j = juce::Justification::centred);
    void addKnobLabels (float cx, float titleY, float subY, bool big, const char* es, const char* sub, const char* en, const char* subEn);
    void setHelp (juce::Component& c, const Help& h);
    void showHelpFor (juce::Component* c);
    void showDefaultHelp();

    void toggleBrowser();
    void showAbout();
    void showUpdateDialog();
    void stepPreset (int delta);
    juce::String presetKey() const;
    int totalHeight() const;
    void updateConstraints();

    DembowProcessor& proc;
    dmb::DmbLookAndFeel lnf;
    Stage stage;

    dmb::Hotspot prevButton { 4.0f }, nextButton { 4.0f }, presetButton { 4.0f }, saveButton { 5.0f }, logoButton { 6.0f };
    juce::SharedResourcePointer<dmb::UpdateChecker> updater;
    int lastUpdateChange = -1;
    dmb::DiceButton diceButton;
    dmb::LangToggle langToggle;
    dmb::MidiLeds leds;
    dmb::TypeSelector types;
    dmb::BoomboxModule boombox;
    dmb::SlotScope scope;

    juce::OwnedArray<dmb::PlateKnob> knobs;
    std::unique_ptr<dmb::PresetBrowser> browser;
    std::unique_ptr<dmb::ChoiceButtons> characterButtons, modeButtons;
    dmb::HelpBar helpBar;
    dmb::DrawerToggle keyboardToggle;
    std::unique_ptr<dmb::KeyboardDrawer> keyboard;

    juce::OwnedArray<dmb::Readout> readouts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;

    juce::Component::SafePointer<juce::Component> hovered;
    juce::String lastPresetKey;
    int lastLangVersion = -1;
    int previewNote = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DembowEditor)
};
