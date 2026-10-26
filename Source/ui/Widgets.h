#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Theme.h"

class DembowProcessor;

namespace dmb
{
// Texto de ayuda (barra de abajo) en los dos idiomas, guardado en las propiedades del componente.
void setHelpProps (juce::Component&, const juce::String& titleEs, const juce::String& titleEn,
                   const juce::String& textEs, const juce::String& textEn);

// Ícono de cada tipo de sonido (dibujado con líneas).
void drawTypeIcon (juce::Graphics&, int category, juce::Rectangle<float> box, juce::Colour, float thickness = 2.0f);

// ---------------------------------------------------------------------------------------------
// Knob dibujado encima de la imagen base: el cuerpo ya está en la imagen; aquí se pintan
// las marcas, el arco verde y el puntero. 'bipolar' = el arco sale del centro (50 = original).
class PlateKnob : public juce::Slider
{
public:
    struct Spec
    {
        juce::Point<float> centre;   // en pixeles del diseño
        float bodyR, arcR, arcW;
        bool big;
        juce::Colour colour = colours::arc;
        bool bipolar = false;
    };

    explicit PlateKnob (Spec s);

    juce::Rectangle<int> boundsInDesign() const;
    void paint (juce::Graphics&) override;
    bool hitTest (int x, int y) override;

    Spec spec;
};

// ---------------------------------------------------------------------------------------------
// Dibuja texto con estilo en una línea; si no cabe, lo hace más chico (build recibe la escala).
inline void drawOneLine (juce::Graphics& g, juce::Rectangle<float> area,
                         const std::function<juce::AttributedString (float)>& build, float minScale = 0.6f)
{
    for (float scale = 1.0f; ; scale -= 0.04f)
    {
        auto s = build (scale);
        s.setWordWrap (juce::AttributedString::none);
        juce::TextLayout layout;
        layout.createLayout (s, 100000.0f);
        if (layout.getWidth() <= area.getWidth() || scale <= minScale)
        {
            layout.createLayout (s, area.getWidth());
            layout.draw (g, area);
            return;
        }
    }
}

// ---------------------------------------------------------------------------------------------
// Texto que cambia (valores, nombres de preset...). Solo se repinta cuando su texto cambia.
class Readout : public juce::Component
{
public:
    using Painter = std::function<void (juce::Graphics&, juce::Rectangle<float>)>;

    Readout (std::function<juce::String()> keyFn, Painter painterFn)
        : key (std::move (keyFn)), painter (std::move (painterFn))
    {
        setInterceptsMouseClicks (false, false);
    }

    static std::unique_ptr<Readout> simple (std::function<juce::String()> text, juce::Font f,
                                            std::function<juce::Colour()> colour, juce::Justification just)
    {
        auto t = std::make_shared<std::function<juce::String()>> (std::move (text));
        return std::make_unique<Readout> ([t] { return (*t)(); },
            [t, f, colour, just] (juce::Graphics& g, juce::Rectangle<float> r)
            {
                g.setFont (f);
                g.setColour (colour());
                g.drawFittedText ((*t)(), r.toNearestInt(), just, 1, 0.8f);
            });
    }

    void poll()
    {
        auto k = key();
        if (k != last) { last = k; repaint(); }
    }

    void paint (juce::Graphics& g) override { painter (g, getLocalBounds().toFloat()); }

private:
    std::function<juce::String()> key;
    Painter painter;
    juce::String last;
};

// ---------------------------------------------------------------------------------------------
// Zona transparente encima de algo que ya está dibujado en la imagen (flechas, Guardar...).
class Hotspot : public juce::Button
{
public:
    explicit Hotspot (float cornerRadius = 4.0f) : juce::Button ({}), corner (cornerRadius)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        if (down)
        {
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.fillRoundedRectangle (r, corner);
        }
        if (over || down)
        {
            g.setColour (colours::lime.withAlpha (0.10f));
            g.drawRoundedRectangle (r.expanded (1.5f), corner + 1.5f, 4.0f);
            g.setColour (colours::lime.withAlpha (0.55f));
            g.drawRoundedRectangle (r, corner, 1.2f);
        }
    }

private:
    float corner;
};

// ---------------------------------------------------------------------------------------------
// Grupo de botones conectados a un parámetro de opciones.
class ChoiceButtons
{
public:
    ChoiceButtons (juce::Component& parent, juce::RangedAudioParameter& param,
                   const juce::StringArray& labels, juce::Array<float> values,
                   const juce::Array<juce::Rectangle<int>>& rectsInDesign, float fontSize);

    std::function<void (float)> onChange;
    juce::TextButton* button (int i) { return buttons[i]; }
    int size() const { return buttons.size(); }
    void refresh() { if (attachment) attachment->sendInitialUpdate(); }
    void setLabels (const juce::StringArray& labels)
    {
        for (int i = 0; i < buttons.size() && i < labels.size(); ++i)
            buttons[i]->setButtonText (labels[i]);
    }

private:
    void update (float v);

    juce::OwnedArray<juce::TextButton> buttons;
    juce::Array<float> values;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};

// ---------------------------------------------------------------------------------------------
// Pantalla de arriba: los 16 tipos de sonido.
class TypeSelector : public juce::Component
{
public:
    explicit TypeSelector (DembowProcessor& p) : proc (p) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    static juce::Rectangle<int> boundsInDesign() { return { 62, 112, 1548, 146 }; }

    std::function<void (int)> onPick;
    void tick();
    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;
    juce::Rectangle<float> tileRect (int index) const;
    int tileAt (juce::Point<float>) const;

private:
    DembowProcessor& proc;
    int hover = -1, shownCategory = -1, shownLang = -1;
};

// ---------------------------------------------------------------------------------------------
// Módulo central: la grabadora (boombox) cuyas bocinas laten con el sonido.
// Mantén apretado para escuchar el sonido sin teclado.
class BoomboxModule : public juce::Component
{
public:
    BoomboxModule() = default;

    static juce::Rectangle<int> boundsInDesign() { return { 480, 290, 620, 198 }; }

    std::function<void (bool)> onPress;   // true = apretado, false = suelto
    void tick (float level);
    void paint (juce::Graphics&) override;
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hover = false; repaint(); }

private:
    juce::Rectangle<float> boxArea() const { return { 236.0f, 36.0f, 200.0f, 124.0f }; }
    float pulse = 0.0f, smoothed = 0.0f;
    bool hover = false, pressed = false, wasActive = true;
};

// Osciloscopio dentro de la ranura de la placa.
class SlotScope : public juce::Component
{
public:
    explicit SlotScope (DembowProcessor& p) : proc (p) { setInterceptsMouseClicks (false, false); }
    static juce::Rectangle<int> boundsInDesign() { return { 716, 515, 264, 22 }; }
    void tick();
    void paint (juce::Graphics&) override;

private:
    DembowProcessor& proc;
    std::array<float, 132> points {};
    bool active = false;
};

// ---------------------------------------------------------------------------------------------
// Botón del dado (un sonido nuevo al azar del tipo actual).
class DiceButton : public juce::Button
{
public:
    DiceButton() : juce::Button ("dice") { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool over, bool down) override;
    void roll() { spin = 1.0f; face = (face + 1 + juce::Random::getSystemRandom().nextInt (5)) % 6; repaint(); }
    void tick() { if (spin > 0.0f) { spin = std::max (0.0f, spin - 0.12f); repaint(); } }

private:
    float spin = 0.0f;
    int face = 4;
};

// Selector de idioma ES | EN.
class LangToggle : public juce::Component
{
public:
    LangToggle() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    std::function<void()> onChange;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent& e) override { setHover (e.x < getWidth() / 2 ? 0 : 1); }
    void mouseExit (const juce::MouseEvent&) override  { setHover (-1); }

private:
    void setHover (int h) { if (h != hover) { hover = h; repaint(); } }
    int hover = -1;
};

// Botón "TECLADO" de la barra de abajo: abre y cierra el teclado.
class DrawerToggle : public juce::Button
{
public:
    DrawerToggle() : juce::Button ("keys") { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void setState (bool isOpen, bool isActive)
    {
        if (isOpen != open || isActive != active) { open = isOpen; active = isActive; repaint(); }
    }
    void paintButton (juce::Graphics&, bool over, bool down) override;

private:
    bool open = false, active = false;
};

// LEDs del encabezado: parpadean con cada nota.
class MidiLeds : public juce::Component
{
public:
    MidiLeds() { setInterceptsMouseClicks (false, false); }
    void tick (int notes, int voices);
    void paint (juce::Graphics&) override;

private:
    int lastNotes = 0;
    float flash = 0.0f;
    int voices = 0;
};

// ---------------------------------------------------------------------------------------------
// Barra de ayuda de abajo.
class HelpBar : public juce::Component
{
public:
    HelpBar() { setInterceptsMouseClicks (false, false); }

    static juce::Rectangle<int> boundsInDesign() { return { 88, 860, 1352, 33 }; }

    void set (const juce::String& newTitle, const juce::String& newText)
    {
        if (newTitle == title && newText == text) return;
        title = newTitle;
        text = newText;
        repaint();
    }

    void paint (juce::Graphics& g) override;

private:
    juce::String title, text;
};
} // namespace dmb
