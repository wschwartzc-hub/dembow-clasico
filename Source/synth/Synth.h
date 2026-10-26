#pragma once

#include "Voice.h"
#include "Effects.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>

namespace dmb
{
// El sintetizador completo: voces + notas MIDI + efectos. No sabe nada de JUCE ni de plugins
// (sirve igual dentro del plugin, en las pruebas o para medir un sonido).
class Synth
{
public:
    static constexpr int numVoices = 24;   // 16 de polifonía + colas que se apagan
    static constexpr int polyphony = 16;

    void prepare (double sampleRate, int maxBlock);
    void reset();

    // patch y macros: se llaman desde el hilo de audio, al inicio de cada bloque
    void setPatch (const Patch& p);
    void setMacros (const Macros& m) { macros = m; }
    void setTempo (double bpm) { tempo = bpm > 20.0 ? bpm : 95.0; }

    void handleMidi (const juce::MidiMessage&);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void allNotesOff (bool immediately);

    // escribe (no suma) el sonido en L y R
    void render (float* L, float* R, int numSamples);

    int activeVoiceCount() const;
    float outputPeak() const { return peak; }
    const Patch& getPatch() const { return patch; }

private:
    void buildParams();
    Voice* findFreeVoice();
    void startVoice (Voice&, int note, float vel, float startPitch);
    void releaseVoicesFor (int note);

    Patch patch;
    Macros macros;
    VoiceParams vp;
    Voice voices[numVoices];
    Effects fx;

    double sr = 48000.0, tempo = 95.0;
    uint64_t counter = 0;
    float lastPitch = -1.0f;   // -1 = todavía no hay nota anterior (no hay glide)
    bool sustainPedal = false;
    float bendNorm = 0.0f, modWheel = 0.0f;
    std::vector<int> heldNotes;          // para modo mono / legato
    std::vector<float> mixL, mixR;
    float peak = 0.0f;
};
} // namespace dmb
