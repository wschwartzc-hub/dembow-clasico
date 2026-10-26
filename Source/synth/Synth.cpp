#include "Synth.h"

namespace dmb
{
void Synth::prepare (double sampleRate, int maxBlock)
{
    sr = sampleRate;
    for (auto& v : voices) v.prepare ((float) sr);
    fx.prepare (sr, maxBlock);
    mixL.assign ((size_t) std::max (64, maxBlock), 0.0f);
    mixR.assign ((size_t) std::max (64, maxBlock), 0.0f);
    heldNotes.reserve (32);
    reset();
}

void Synth::reset()
{
    for (auto& v : voices) v.reset();
    fx.reset();
    heldNotes.clear();
    sustainPedal = false;
    bendNorm = modWheel = 0.0f;
    peak = 0.0f;
    lastPitch = -1.0f;
}

void Synth::setPatch (const Patch& p)
{
    patch = p;
}

void Synth::buildParams()
{
    vp.patch = &patch;
    vp.sr = (float) sr;
    vp.bright = juce::jlimit (-1.0f, 1.0f, (macros.brillo - 50.0f) / 50.0f);
    vp.cutoffMul = std::exp2 (vp.bright * 2.5f);
    vp.decayMul = std::pow (4.0f, juce::jlimit (-1.0f, 1.0f, (macros.duracion - 50.0f) / 50.0f));
    const float a = juce::jlimit (-1.0f, 1.0f, (macros.ataque - 50.0f) / 50.0f);
    vp.attackMul = a < 0.0f ? std::max (0.05f, 1.0f + a * 0.95f) : 1.0f;
    vp.attackAdd = a > 0.0f ? a * a * 1.8f : 0.0f;
    const float g = juce::jlimit (-1.0f, 1.0f, (macros.grosor - 50.0f) / 50.0f);
    vp.detuneMul = std::exp2 (g * 1.2f);
    vp.widthAdd = g * 0.4f;
    vp.vibDepth = std::pow (juce::jlimit (0.0f, 1.0f, macros.vibrato * 0.01f), 1.5f) * 0.7f + modWheel * 0.6f;
    const float gl = juce::jlimit (0.0f, 1.0f, macros.glide * 0.01f);
    vp.glideTime = gl <= 0.001f ? 0.0f : 0.01f + gl * gl * 1.2f;
    vp.velSens = juce::jlimit (0.0f, 1.0f, macros.fuerza * 0.01f);
    vp.transpose = 12.0f * (float) juce::jlimit (-2, 2, macros.octava);
    vp.bend = bendNorm * patch[gp::bend];
}

// ---------------------------------------------------------------------------------------------
void Synth::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        noteOff (m.getNoteNumber());
    else if (m.isPitchWheel())
        bendNorm = juce::jlimit (-1.0f, 1.0f, (float) (m.getPitchWheelValue() - 8192) / 8192.0f);
    else if (m.isController())
    {
        const int cc = m.getControllerNumber();
        const int val = m.getControllerValue();
        if (cc == 1) modWheel = (float) val / 127.0f;
        else if (cc == 64)
        {
            const bool down = val >= 64;
            if (sustainPedal && ! down)
                for (auto& v : voices)
                    if (v.isActive() && v.sustained && ! v.keyDown) { v.sustained = false; v.noteOff(); }
            sustainPedal = down;
        }
        else if (cc == 120) allNotesOff (true);
        else if (cc == 123) allNotesOff (false);
    }
    else if (m.isAllNotesOff()) allNotesOff (false);
    else if (m.isAllSoundOff()) allNotesOff (true);
}

Voice* Synth::findFreeVoice()
{
    int sounding = 0;
    for (auto& v : voices)
        if (v.isActive() && ! v.isReleasing()) ++sounding;

    // si ya hay 16 sonando, se apaga rápido la más vieja (primero las que ya se soltaron)
    if (sounding >= polyphony)
    {
        Voice* victim = nullptr;
        for (auto& v : voices)
            if (v.isActive() && ! v.isReleasing() && ! v.keyDown && (victim == nullptr || v.order < victim->order)) victim = &v;
        if (victim == nullptr)
            for (auto& v : voices)
                if (v.isActive() && ! v.isReleasing() && (victim == nullptr || v.order < victim->order)) victim = &v;
        if (victim != nullptr) victim->fastKill();
    }

    for (auto& v : voices)
        if (! v.isActive()) return &v;

    // no hay lugar: se reutiliza la que suena más bajito de las que se están apagando
    Voice* best = nullptr;
    for (auto& v : voices)
        if (best == nullptr || (v.isReleasing() && (! best->isReleasing() || v.level() < best->level()))) best = &v;
    return best;
}

void Synth::startVoice (Voice& v, int note, float vel, float startPitch)
{
    if (startPitch < 0.0f) startPitch = (float) note;
    v.order = ++counter;
    v.noteOn (note, vel, startPitch, vp, (uint32_t) counter * 2246822519u + (uint32_t) note);
}

void Synth::noteOn (int note, float vel)
{
    buildParams();
    const int mode = macros.modo;

    if (mode == playPoly)
    {
        if (auto* v = findFreeVoice())
            startVoice (*v, note, vel, lastPitch);
    }
    else
    {
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
        heldNotes.push_back (note);
        // las demás voces (de antes de cambiar a mono) se sueltan
        for (int i = 1; i < numVoices; ++i)
            if (voices[i].isActive() && voices[i].keyDown) voices[i].noteOff();

        auto& v = voices[0];
        if (v.isActive() && ! v.isReleasing())
        {
            v.order = ++counter;
            if (mode == playLegato) v.legatoTo (note, vel);
            else                    v.retrigger (note, vel, vp);
        }
        else
            startVoice (v, note, vel, v.isActive() ? v.currentPitch() : lastPitch);
    }
    lastPitch = (float) note;
}

void Synth::noteOff (int note)
{
    buildParams();
    if (macros.modo != playPoly)
    {
        heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
        auto& v = voices[0];
        if (v.isActive() && v.note == note)
        {
            if (! heldNotes.empty())
            {
                v.legatoTo (heldNotes.back(), v.velocity);   // regresa a la nota que sigue apretada
                lastPitch = (float) heldNotes.back();
            }
            else if (sustainPedal) { v.keyDown = false; v.sustained = true; }
            else v.noteOff();
        }
    }
    for (auto& v : voices)
        if (v.isActive() && v.note == note && v.keyDown)
        {
            if (sustainPedal) { v.keyDown = false; v.sustained = true; }
            else v.noteOff();
        }
}

void Synth::allNotesOff (bool immediately)
{
    heldNotes.clear();
    for (auto& v : voices)
    {
        if (! v.isActive()) continue;
        v.sustained = false;
        if (immediately) v.fastKill();
        else v.noteOff();
    }
}

int Synth::activeVoiceCount() const
{
    int n = 0;
    for (auto& v : voices) if (v.isActive()) ++n;
    return n;
}

void Synth::render (float* L, float* R, int numSamples)
{
    buildParams();
    std::fill (L, L + numSamples, 0.0f);
    std::fill (R, R + numSamples, 0.0f);
    for (auto& v : voices)
        if (v.isActive())
            v.render (L, R, numSamples, vp);

    fx.process (L, R, numSamples, patch, macros, tempo);

    float p = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        p = std::max (p, std::max (std::abs (L[i]), std::abs (R[i])));
    peak = std::max (p, peak * 0.9f);
}
} // namespace dmb
