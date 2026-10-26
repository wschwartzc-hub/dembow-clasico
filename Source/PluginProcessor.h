#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"
#include "synth/Synth.h"
#include "presets/Factory.h"

// Un preset en la lista del gestor (de fábrica o del usuario).
struct PresetEntry
{
    int factoryIndex = -1;          // >= 0 si es de fábrica
    juce::File file;                // si es del usuario
    juce::String id, name, description;
    int category = 0, archetype = -1;
    bool isUser() const { return factoryIndex < 0; }
};

class DembowProcessor : public juce::AudioProcessor,
                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    DembowProcessor();
    ~DembowProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return getPresetName(); }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;

    // ----- presets ---------------------------------------------------------------------------
    void loadFactoryPreset (int index);
    bool loadUserPreset (const juce::File&);
    void loadEntry (const PresetEntry&);
    bool saveUserPreset (const juce::String& name);
    bool renameUserPreset (const juce::File&, const juce::String& newName);
    bool deleteUserPreset (const juce::File&);
    void stepPreset (int delta);              // dentro del tipo actual
    void selectCategory (int category);       // vuelve al último preset que usaste de ese tipo
    void rollDice();                          // un sonido nuevo al azar del tipo actual

    int getCategory() const              { return category; }
    int getFactoryIndex() const          { return factoryIndex; }
    juce::String getPresetName() const;
    juce::String getPresetDescription() const;
    juce::String getCurrentPresetId() const;
    bool isPresetModified() const        { return presetModified.load(); }

    std::vector<PresetEntry> getPresetEntries() const;
    static juce::File getUserPresetFolder();
    juce::Array<juce::File> getUserPresets() const;

    bool isFavorite (const juce::String& id) const;
    void setFavorite (const juce::String& id, bool);

    const dmb::Patch& getPatch() const   { return patch; }

    // ----- para la interfaz ------------------------------------------------------------------
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<int> notesPlayed { 0 }, voicesActive { 0 };
    std::atomic<double> hostBpm { 95.0 };
    static constexpr int scopeSize = 2048;
    float scope[scopeSize] {};
    std::atomic<int> scopeWrite { 0 };
    bool keyboardOpen = false;
    int keyboardOctave = 2;   // el teclado en pantalla empieza en C1 (graves visibles)

private:
    void parameterChanged (const juce::String&, float) override;
    void applyPreset (const dmb::Patch&, const dmb::Macros&);
    void rememberInCategory();
    void loadFavorites();
    void saveFavorites();

    dmb::Synth synth;
    dmb::Patch patch;                          // copia del hilo de la interfaz
    dmb::Patch pendingPatch;
    juce::SpinLock patchLock;
    std::atomic<bool> patchPending { false };

    int factoryIndex = 0;
    int category = dmb::catLead;
    juce::File userFile;
    juce::String customName;                   // "Lead Marquesina · Dado 123"
    std::atomic<bool> presetModified { false };
    bool applyingPreset = false;
    std::array<int, dmb::numCategories> lastInCategory {};
    juce::StringArray favorites;
    uint32_t diceCounter = 1;
    std::vector<float> monoTmp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DembowProcessor)
};
