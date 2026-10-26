#include "Widgets.h"
#include "../PluginProcessor.h"

namespace dmb
{
void setHelpProps (juce::Component& c, const juce::String& tEs, const juce::String& tEn, const juce::String& es, const juce::String& en)
{
    auto& props = c.getProperties();
    props.set ("help", true);
    props.set ("helpTitleEs", tEs);
    props.set ("helpTitleEn", tEn);
    props.set ("helpEs", es);
    props.set ("helpEn", en);
}

// =============================================================================================
// Íconos de los tipos (dibujados en una caja de 0..1)
// =============================================================================================
void drawTypeIcon (juce::Graphics& g, int category, juce::Rectangle<float> box, juce::Colour c, float th)
{
    const float x = box.getX(), y = box.getY(), w = box.getWidth(), h = box.getHeight();
    auto P = [&] (float px, float py) { return juce::Point<float> (x + px * w, y + py * h); };
    juce::Path p;
    const juce::PathStrokeType stroke (th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.setColour (c);

    switch (category)
    {
        case 0:   // lead: diente de sierra
            p.startNewSubPath (P (0.05f, 0.7f)); p.lineTo (P (0.35f, 0.25f)); p.lineTo (P (0.35f, 0.7f));
            p.lineTo (P (0.65f, 0.25f)); p.lineTo (P (0.65f, 0.7f)); p.lineTo (P (0.95f, 0.25f));
            break;
        case 1:   // pluck: golpe que se apaga
            p.startNewSubPath (P (0.05f, 0.85f)); p.lineTo (P (0.2f, 0.85f)); p.lineTo (P (0.25f, 0.12f));
            p.quadraticTo (P (0.35f, 0.75f), P (0.95f, 0.85f));
            break;
        case 2:   // bajo: una onda grande y gruesa
            p.startNewSubPath (P (0.05f, 0.5f));
            p.cubicTo (P (0.2f, 0.0f), P (0.35f, 0.0f), P (0.5f, 0.5f));
            p.cubicTo (P (0.65f, 1.0f), P (0.8f, 1.0f), P (0.95f, 0.5f));
            g.strokePath (p, juce::PathStrokeType (th * 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            return;
        case 3:   // pad: nube
            p.startNewSubPath (P (0.12f, 0.75f));
            p.cubicTo (P (0.0f, 0.75f), P (0.0f, 0.45f), P (0.2f, 0.47f));
            p.cubicTo (P (0.22f, 0.2f), P (0.5f, 0.15f), P (0.56f, 0.38f));
            p.cubicTo (P (0.7f, 0.25f), P (0.92f, 0.35f), P (0.86f, 0.52f));
            p.cubicTo (P (1.02f, 0.55f), P (1.0f, 0.78f), P (0.85f, 0.75f));
            p.closeSubPath();
            break;
        case 4:   // piano: teclas
            p.addRoundedRectangle (P (0.08f, 0.2f).x, P (0.08f, 0.2f).y, 0.84f * w, 0.62f * h, 2.0f);
            for (float k : { 0.29f, 0.5f, 0.71f }) { p.startNewSubPath (P (k, 0.2f)); p.lineTo (P (k, 0.82f)); }
            g.strokePath (p, stroke);
            for (float k : { 0.29f, 0.71f })
                g.fillRect (juce::Rectangle<float> (P (k - 0.06f, 0.2f), P (k + 0.06f, 0.55f)));
            return;
        case 5:   // órgano: tubos
        {
            const float hs[] = { 0.55f, 0.75f, 0.9f, 0.75f, 0.55f };
            for (int i = 0; i < 5; ++i)
            {
                const float cx = 0.14f + 0.18f * (float) i;
                p.addRoundedRectangle (P (cx - 0.06f, 0.92f - hs[i]).x, P (cx - 0.06f, 0.92f - hs[i]).y, 0.12f * w, hs[i] * h, 2.0f);
            }
            break;
        }
        case 6:   // campanas
            p.startNewSubPath (P (0.18f, 0.78f));
            p.cubicTo (P (0.3f, 0.7f), P (0.25f, 0.18f), P (0.5f, 0.15f));
            p.cubicTo (P (0.75f, 0.18f), P (0.7f, 0.7f), P (0.82f, 0.78f));
            p.closeSubPath();
            p.addEllipse (P (0.45f, 0.8f).x, P (0.45f, 0.8f).y, 0.1f * w, 0.1f * h);
            p.startNewSubPath (P (0.5f, 0.06f)); p.lineTo (P (0.5f, 0.15f));
            break;
        case 7:   // marimba: barras
            for (int i = 0; i < 4; ++i)
            {
                const float len = 0.9f - 0.15f * (float) i;
                p.addRoundedRectangle (P (0.05f, 0.14f + 0.2f * (float) i).x, P (0.05f, 0.14f + 0.2f * (float) i).y, len * w, 0.11f * h, 2.0f);
            }
            break;
        case 8:   // cuerdas: arco sobre cuerdas
            for (float k : { 0.3f, 0.43f, 0.57f, 0.7f }) { p.startNewSubPath (P (k, 0.08f)); p.lineTo (P (k, 0.92f)); }
            g.strokePath (p, juce::PathStrokeType (th * 0.7f));
            p.clear();
            p.startNewSubPath (P (0.05f, 0.75f)); p.lineTo (P (0.95f, 0.3f));
            break;
        case 9:   // pizzicato: cuerda pellizcada
            p.startNewSubPath (P (0.5f, 0.05f)); p.quadraticTo (P (0.78f, 0.5f), P (0.5f, 0.95f));
            g.strokePath (p, stroke);
            g.fillEllipse (juce::Rectangle<float> (P (0.62f, 0.4f), P (0.82f, 0.6f)));
            p.clear();
            p.startNewSubPath (P (0.25f, 0.05f)); p.lineTo (P (0.25f, 0.95f));
            break;
        case 10:  // metales: trompeta
            p.startNewSubPath (P (0.05f, 0.45f)); p.lineTo (P (0.6f, 0.45f));
            p.lineTo (P (0.92f, 0.18f)); p.lineTo (P (0.92f, 0.82f)); p.lineTo (P (0.6f, 0.55f)); p.lineTo (P (0.05f, 0.55f));
            p.closeSubPath();
            p.addEllipse (P (0.22f, 0.25f).x, P (0.22f, 0.25f).y, 0.08f * w, 0.08f * h);
            p.addEllipse (P (0.36f, 0.25f).x, P (0.36f, 0.25f).y, 0.08f * w, 0.08f * h);
            break;
        case 11:  // orch hit: estallido
            for (int i = 0; i < 10; ++i)
            {
                const float a = juce::MathConstants<float>::twoPi * (float) i / 10.0f;
                const float r = (i % 2 == 0) ? 0.47f : 0.2f;
                const auto pt = P (0.5f + std::cos (a) * r, 0.5f + std::sin (a) * r);
                if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
            }
            p.closeSubPath();
            break;
        case 12:  // guitarra
            p.addEllipse (P (0.08f, 0.45f).x, P (0.08f, 0.45f).y, 0.42f * w, 0.42f * h);
            p.addEllipse (P (0.22f, 0.3f).x, P (0.22f, 0.3f).y, 0.3f * w, 0.3f * h);
            p.startNewSubPath (P (0.42f, 0.5f)); p.lineTo (P (0.92f, 0.08f));
            break;
        case 13:  // flauta
            p.addRoundedRectangle (P (0.04f, 0.42f).x, P (0.04f, 0.42f).y, 0.92f * w, 0.16f * h, 3.0f);
            g.strokePath (p, stroke);
            for (float k : { 0.4f, 0.55f, 0.7f, 0.85f })
                g.fillEllipse (juce::Rectangle<float> (P (k - 0.03f, 0.47f), P (k + 0.03f, 0.53f)));
            return;
        case 14:  // coros: tres voces
            for (float k : { 0.2f, 0.5f, 0.8f })
            {
                p.addEllipse (P (k - 0.1f, 0.2f).x, P (k - 0.1f, 0.2f).y, 0.2f * w, 0.2f * h);
                p.startNewSubPath (P (k - 0.15f, 0.85f)); p.quadraticTo (P (k, 0.4f), P (k + 0.15f, 0.85f));
            }
            break;
        default:  // FX: rayo
            p.startNewSubPath (P (0.6f, 0.05f)); p.lineTo (P (0.25f, 0.55f)); p.lineTo (P (0.5f, 0.55f));
            p.lineTo (P (0.38f, 0.95f)); p.lineTo (P (0.78f, 0.4f)); p.lineTo (P (0.52f, 0.4f)); p.closeSubPath();
            break;
    }
    g.strokePath (p, stroke);
}

// =============================================================================================
// PlateKnob
// =============================================================================================
PlateKnob::PlateKnob (Spec s) : spec (s)
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    setMouseDragSensitivity (spec.big ? 300 : 220);
    setScrollWheelEnabled (true);
    setPaintingIsUnclipped (false);
}

juce::Rectangle<int> PlateKnob::boundsInDesign() const
{
    const float half = spec.arcR + spec.arcW * 0.5f + (spec.big ? 20.0f : 14.0f);
    return juce::Rectangle<float> (spec.centre.x - half, spec.centre.y - half, half * 2.0f, half * 2.0f)
               .getSmallestIntegerContainer();
}

bool PlateKnob::hitTest (int x, int y)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    return c.getDistanceFrom ({ (float) x, (float) y }) < spec.arcR + spec.arcW * 0.5f + 8.0f;
}

void PlateKnob::paint (juce::Graphics& g)
{
    const auto origin = getBounds().getPosition().toFloat();
    const juce::Point<float> c (spec.centre.x - origin.x, spec.centre.y - origin.y);

    const auto rp = getRotaryParameters();
    const float a0 = rp.startAngleRadians, a1 = rp.endAngleRadians;
    const float pos = (float) valueToProportionOfLength (getValue());
    const float ang = a0 + pos * (a1 - a0);
    const bool hot = isMouseOverOrDragging() && isEnabled();

    const float R = spec.arcR, W = spec.arcW;
    const bool big = spec.big;

    constexpr int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        const float t = a0 + (float) i / (float) (numTicks - 1) * (a1 - a0);
        const bool end = (i == 0 || i == numTicks - 1 || (spec.bipolar && i == numTicks / 2));
        const float r0 = R + W * 0.5f + (big ? 4.5f : 3.0f);
        const float r1 = r0 + (big ? (end ? 9.0f : 6.0f) : (end ? 6.0f : 4.0f));
        g.setColour (juce::Colour (0xffc9ced3).withAlpha (end ? 0.8f : 0.55f));
        g.drawLine (juce::Line<float> (c.getPointOnCircumference (r0, t), c.getPointOnCircumference (r1, t)), big ? 1.7f : 1.3f);
    }

    {
        juce::Path track;
        track.addCentredArc (c.x, c.y, R, R, 0.0f, a0, a1, true);
        g.setColour (juce::Colours::white.withAlpha (0.045f));
        g.strokePath (track, juce::PathStrokeType (W, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    const float from = spec.bipolar ? (a0 + a1) * 0.5f : a0;
    if (std::abs (ang - from) > 0.004f)
    {
        juce::Path arc;
        arc.addCentredArc (c.x, c.y, R, R, 0.0f, std::min (from, ang), std::max (from, ang), true);
        const auto col = spec.colour;
        const float glow = hot ? 1.5f : 1.0f;
        g.setColour (col.withAlpha (0.035f * glow));
        g.strokePath (arc, juce::PathStrokeType (W + (big ? 12.0f : 7.0f), juce::PathStrokeType::curved, juce::PathStrokeType::butt));
        g.setColour (col.withAlpha (0.07f * glow));
        g.strokePath (arc, juce::PathStrokeType (W + (big ? 6.0f : 3.5f), juce::PathStrokeType::curved, juce::PathStrokeType::butt));
        g.setColour (col.withAlpha (0.16f * glow));
        g.strokePath (arc, juce::PathStrokeType (W + 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
        g.setColour (col);
        g.strokePath (arc, juce::PathStrokeType (W, juce::PathStrokeType::curved, juce::PathStrokeType::butt));

        juce::Path shine;
        const float rs = R - W * 0.22f;
        shine.addCentredArc (c.x, c.y, rs, rs, 0.0f, std::min (from, ang), std::max (from, ang), true);
        g.setColour (col.brighter (0.55f).withAlpha (0.55f));
        g.strokePath (shine, juce::PathStrokeType (W * 0.28f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
    }

    {
        const float r0 = spec.bodyR * (big ? 0.24f : 0.22f);
        const float r1 = spec.bodyR * (big ? 0.89f : 0.85f);
        const float w  = big ? 6.5f : 3.8f;
        juce::Path ptr;
        ptr.startNewSubPath (c.getPointOnCircumference (r0, ang));
        ptr.lineTo (c.getPointOnCircumference (r1, ang));
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.strokePath (ptr, juce::PathStrokeType (w + 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                      juce::AffineTransform::translation (0.8f, 1.6f));
        g.setColour (juce::Colour (0xffbdbfc1));
        g.strokePath (ptr, juce::PathStrokeType (w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xfff4f4f4));
        g.strokePath (ptr, juce::PathStrokeType (w * 0.55f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                      juce::AffineTransform::translation (-0.3f, -0.4f));
    }
}

// =============================================================================================
// ChoiceButtons
// =============================================================================================
ChoiceButtons::ChoiceButtons (juce::Component& parent, juce::RangedAudioParameter& param,
                              const juce::StringArray& labels, juce::Array<float> valuesIn,
                              const juce::Array<juce::Rectangle<int>>& rects, float fontSize)
    : values (std::move (valuesIn))
{
    constexpr int pad = 9;
    for (int i = 0; i < labels.size(); ++i)
    {
        auto* b = buttons.add (new juce::TextButton (labels[i]));
        b->setClickingTogglesState (false);
        b->getProperties().set ("glowPad", (float) pad);
        b->getProperties().set ("fontSize", fontSize);
        b->setMouseCursor (juce::MouseCursor::PointingHandCursor);
        b->setBounds (rects[i].expanded (pad));
        b->onClick = [this, i] { attachment->setValueAsCompleteGesture (values[i]); };
        parent.addAndMakeVisible (b);
    }
    attachment = std::make_unique<juce::ParameterAttachment> (param, [this] (float v) { update (v); }, nullptr);
    attachment->sendInitialUpdate();
}

void ChoiceButtons::update (float v)
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (std::abs (values[i] - v) < 0.5f, juce::dontSendNotification);
    if (onChange) onChange (v);
}

// =============================================================================================
// TypeSelector
// =============================================================================================
juce::Rectangle<float> TypeSelector::tileRect (int i) const
{
    const float gap = 8.0f;
    const float w = ((float) getWidth() - 7.0f * gap) / 8.0f;
    const float h = ((float) getHeight() - gap) / 2.0f;
    return { (float) (i % 8) * (w + gap), (float) (i / 8) * (h + gap), w, h };
}

int TypeSelector::tileAt (juce::Point<float> p) const
{
    for (int i = 0; i < numCategories; ++i)
        if (tileRect (i).contains (p)) return i;
    return -1;
}

void TypeSelector::tick()
{
    if (proc.getCategory() != shownCategory || langVersion() != shownLang)
    {
        shownCategory = proc.getCategory();
        shownLang = langVersion();
        repaint();
    }
}

void TypeSelector::mouseMove (const juce::MouseEvent& e)
{
    const int i = tileAt (e.position);
    if (i != hover) { hover = i; repaint(); }
}

void TypeSelector::mouseExit (const juce::MouseEvent&)
{
    hover = -1;
    repaint();
}

void TypeSelector::mouseDown (const juce::MouseEvent& e)
{
    const int i = tileAt (e.position);
    if (i >= 0 && onPick) onPick (i);
    repaint();
}

void TypeSelector::paint (juce::Graphics& g)
{
    const int cur = proc.getCategory();
    for (int i = 0; i < numCategories; ++i)
    {
        auto r = tileRect (i).reduced (1.0f);
        const bool sel = i == cur, hot = i == hover;
        if (sel)
        {
            g.setColour (colours::lime.withAlpha (0.10f));
            g.fillRoundedRectangle (r.expanded (3.0f), 9.0f);
            g.setGradientFill (juce::ColourGradient (colours::lime.withAlpha (0.24f), r.getX(), r.getY(),
                                                     colours::arc.withAlpha (0.10f), r.getX(), r.getBottom(), false));
            g.fillRoundedRectangle (r, 7.0f);
            g.setColour (colours::lime.withAlpha (0.9f));
            g.drawRoundedRectangle (r.reduced (0.5f), 7.0f, 1.6f);
        }
        else
        {
            g.setColour (juce::Colours::white.withAlpha (hot ? 0.07f : 0.025f));
            g.fillRoundedRectangle (r, 7.0f);
            g.setColour (hot ? juce::Colour (0xff8a9196) : juce::Colour (0xff2f3436));
            g.drawRoundedRectangle (r.reduced (0.5f), 7.0f, 1.1f);
        }

        const auto col = sel ? colours::lime : (hot ? colours::text : juce::Colour (0xff9aa4ab));
        auto iconBox = juce::Rectangle<float> (r.getX() + 14.0f, r.getCentreY() - 17.0f, 34.0f, 34.0f);
        drawTypeIcon (g, i, iconBox, col, 2.2f);

        auto text = r.withTrimmedLeft (60.0f).withTrimmedRight (6.0f);
        g.setFont (font (19.5f, true, 0.08f));
        g.setColour (sel ? colours::lime : (hot ? colours::text : juce::Colour (0xffd2d6d9)));
        g.drawFittedText (upper (categoryName (i)), text.withTrimmedBottom (text.getHeight() * 0.42f).toNearestInt(),
                          juce::Justification::bottomLeft, 1, 0.75f);
        g.setFont (font (13.5f));
        g.setColour (sel ? colours::lime.withAlpha (0.8f) : colours::helpDesc);
        g.drawText (T ("500 sonidos", "500 sounds"), text.withTrimmedTop (text.getHeight() * 0.6f), juce::Justification::topLeft, false);
    }
}

// =============================================================================================
// BoomboxModule
// =============================================================================================
bool BoomboxModule::hitTest (int x, int y)
{
    return boxArea().expanded (8.0f, 30.0f).contains ((float) x, (float) y);
}

void BoomboxModule::mouseDown (const juce::MouseEvent&)
{
    pressed = true;
    pulse = 1.0f;
    if (onPress) onPress (true);
    repaint();
}

void BoomboxModule::mouseUp (const juce::MouseEvent&)
{
    pressed = false;
    if (onPress) onPress (false);
    repaint();
}

void BoomboxModule::tick (float level)
{
    const float target = juce::jlimit (0.0f, 1.0f, std::sqrt (juce::jmax (0.0f, level)) * 1.4f);
    smoothed = juce::jmax (target, smoothed * 0.78f);
    pulse *= 0.85f;
    const bool active = smoothed > 0.004f || pulse > 0.01f;
    if (active || wasActive) repaint();
    wasActive = active;
}

void BoomboxModule::paint (juce::Graphics& g)
{
    const auto b = boxArea();
    const float I = juce::jmax (smoothed, pulse * 0.6f);
    const juce::Colour line (0xffd4d8d8);
    const juce::Colour lineDim (0xff9ea5a8);
    const float lw = 2.4f;

    // sombra de fondo (como el brillo del emblema original)
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRoundedRectangle (b.translated (2.0f, 3.0f).expanded (6.0f), 18.0f);

    juce::Path body;
    body.addRoundedRectangle (b.getX(), b.getY() + 26.0f, b.getWidth(), b.getHeight() - 26.0f, 16.0f);
    g.setColour (juce::Colour (0xff121415).withAlpha (0.65f));
    g.fillPath (body);
    g.setColour (line);
    g.strokePath (body, juce::PathStrokeType (lw));

    // asa
    juce::Path handle;
    handle.startNewSubPath (b.getX() + 46.0f, b.getY() + 26.0f);
    handle.lineTo (b.getX() + 52.0f, b.getY() + 4.0f);
    handle.lineTo (b.getRight() - 52.0f, b.getY() + 4.0f);
    handle.lineTo (b.getRight() - 46.0f, b.getY() + 26.0f);
    g.strokePath (handle, juce::PathStrokeType (lw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    // antena
    g.setColour (lineDim);
    g.drawLine (b.getRight() - 22.0f, b.getY() + 26.0f, b.getRight() + 18.0f, b.getY() - 22.0f, 1.8f);
    g.fillEllipse (b.getRight() + 15.0f, b.getY() - 26.0f, 6.0f, 6.0f);

    // casetera
    const auto deck = juce::Rectangle<float> (b.getCentreX() - 22.0f, b.getY() + 38.0f, 44.0f, 26.0f);
    g.setColour (line);
    g.drawRoundedRectangle (deck, 3.0f, 1.6f);
    g.setColour (lineDim);
    g.drawEllipse (deck.getX() + 8.0f, deck.getY() + 8.0f, 9.0f, 9.0f, 1.4f);
    g.drawEllipse (deck.getRight() - 17.0f, deck.getY() + 8.0f, 9.0f, 9.0f, 1.4f);
    // botoncitos
    for (int k = 0; k < 4; ++k)
        g.fillRoundedRectangle (b.getCentreX() - 26.0f + 14.0f * (float) k, b.getY() + 20.0f, 9.0f, 5.0f, 1.0f);
    // ecualizador en medio
    for (int k = 0; k < 5; ++k)
    {
        const float hgt = 3.0f + 14.0f * I * (0.5f + 0.5f * std::sin ((float) k * 1.7f + I * 6.0f));
        g.setColour (colours::lime.withAlpha (0.35f + 0.6f * I));
        g.fillRect (b.getCentreX() - 18.0f + 8.0f * (float) k, b.getBottom() - 12.0f - hgt, 5.0f, hgt);
    }

    // bocinas (laten con el sonido)
    for (int s = 0; s < 2; ++s)
    {
        const juce::Point<float> c (s == 0 ? b.getX() + 46.0f : b.getRight() - 46.0f, b.getY() + 80.0f);
        const float outer = 31.0f, cone = 15.0f + 4.5f * I;
        if (I > 0.02f)
        {
            g.setColour (colours::lime.withAlpha (0.10f + 0.20f * I));
            g.fillEllipse (c.x - cone - 9.0f, c.y - cone - 9.0f, (cone + 9.0f) * 2.0f, (cone + 9.0f) * 2.0f);
        }
        g.setColour (line);
        g.drawEllipse (c.x - outer, c.y - outer, outer * 2.0f, outer * 2.0f, lw);
        g.setColour (colours::ring.interpolatedWith (juce::Colour (0xffe2ff9e), I));
        g.drawEllipse (c.x - cone, c.y - cone, cone * 2.0f, cone * 2.0f, 2.2f + 1.2f * I);
        g.fillEllipse (c.x - 4.0f, c.y - 4.0f, 8.0f, 8.0f);
    }

    if (hover || pressed)
    {
        g.setColour (colours::lime.withAlpha (pressed ? 0.35f : 0.18f));
        g.strokePath (body, juce::PathStrokeType (lw + 4.0f));
    }

    // etiqueta grabada (como el TAP de ECO)
    g.setFont (font (17.0f, true, 0.12f));
    g.setColour ((hover || pressed) ? colours::lime : colours::engraved);
    g.drawText (T ("TOCAR", "PLAY"), juce::Rectangle<float> (30.0f, 118.0f, 180.0f, 20.0f), juce::Justification::centredRight);
    g.setFont (font (13.0f));
    g.setColour (colours::engraved);
    g.drawText (T ("mantén para escuchar", "hold to listen"), juce::Rectangle<float> (4.0f, 139.0f, 206.0f, 16.0f), juce::Justification::centredRight);
}

// =============================================================================================
// SlotScope
// =============================================================================================
void SlotScope::tick()
{
    const int w = proc.scopeWrite.load();
    const int N = DembowProcessor::scopeSize;
    // busca un cruce por cero (subiendo) para que la onda se quede quieta
    int start = (w - 600 + N) & (N - 1);
    for (int k = 0; k < 300; ++k)
    {
        const int i = (w - 600 - k + 2 * N) & (N - 1);
        const int j = (i + 1) & (N - 1);
        if (proc.scope[i] <= 0.0f && proc.scope[j] > 0.0f) { start = i; break; }
    }
    float peak = 0.0f;
    for (size_t p = 0; p < points.size(); ++p)
    {
        const float v = proc.scope[(start + (int) p * 3) & (N - 1)];
        points[p] = v;
        peak = std::max (peak, std::abs (v));
    }
    const bool nowActive = peak > 0.0005f;
    if (nowActive || active) repaint();
    active = nowActive;
}

void SlotScope::paint (juce::Graphics& g)
{
    const float h = (float) getHeight(), w = (float) getWidth();
    float peak = 0.0f;
    for (auto v : points) peak = std::max (peak, std::abs (v));
    const float gain = peak > 0.0f ? std::min (1.0f / peak, 6.0f) * 0.42f : 0.0f;
    juce::Path p;
    for (size_t i = 0; i < points.size(); ++i)
    {
        const float x = w * (float) i / (float) (points.size() - 1);
        const float y = h * 0.5f - points[i] * gain * h;
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    const float a = active ? 1.0f : 0.25f;
    g.setColour (colours::lime.withAlpha (0.15f * a));
    g.strokePath (p, juce::PathStrokeType (4.0f));
    g.setColour (colours::lime.withAlpha (0.85f * a));
    g.strokePath (p, juce::PathStrokeType (1.4f));
}

// =============================================================================================
// Botones del encabezado y de abajo
// =============================================================================================
void DiceButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 5.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff222526), 0.0f, r.getY(), juce::Colour (0xff121314), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (juce::Colour (over ? 0xffb9c0c4 : 0xff858b8f).withAlpha (down ? 0.6f : 0.85f));
    g.drawRoundedRectangle (r.reduced (0.6f), 5.0f, 1.3f);

    // dado
    const float s = 24.0f;
    auto die = juce::Rectangle<float> (r.getX() + 12.0f, r.getCentreY() - s * 0.5f, s, s);
    {
    juce::Graphics::ScopedSaveState save (g);
    g.addTransform (juce::AffineTransform::rotation (spin * 1.6f, die.getCentreX(), die.getCentreY()));
    const auto col = (over || spin > 0.0f) ? colours::lime : juce::Colour (0xffd2d6d9);
    g.setColour (col);
    g.drawRoundedRectangle (die, 4.0f, 1.8f);
    const int pips[6][7] = { { 4 }, { 0, 8 }, { 0, 4, 8 }, { 0, 2, 6, 8 }, { 0, 2, 4, 6, 8 }, { 0, 2, 3, 5, 6, 8 } };
    const int counts[6] = { 1, 2, 3, 4, 5, 6 };
    for (int k = 0; k < counts[face]; ++k)
    {
        const int cell = pips[face][k];
        const float px = die.getX() + s * (0.22f + 0.28f * (float) (cell % 3));
        const float py = die.getY() + s * (0.22f + 0.28f * (float) (cell / 3));
        g.fillEllipse (px - 2.2f, py - 2.2f, 4.4f, 4.4f);
    }
    }
    g.setFont (font (17.0f, true, 0.1f));
    g.setColour (over ? colours::text : juce::Colour (0xffd2d6d9));
    g.drawText (T ("DADO", "DICE"), r.withTrimmedLeft (44.0f).withTrimmedRight (6.0f), juce::Justification::centred, false);
}

void LangToggle::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 5.0f);
    g.setColour (juce::Colour (0xff141516));
    g.fillRoundedRectangle (r, 5.0f);
    const bool en = english();
    for (int i = 0; i < 2; ++i)
    {
        auto seg = r.withWidth (r.getWidth() * 0.5f).translated (r.getWidth() * 0.5f * (float) i, 0.0f).reduced (3.0f);
        const bool sel = (i == 1) == en;
        if (sel)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc4f75c), 0.0f, seg.getY(), juce::Colour (0xff76c61f), 0.0f, seg.getBottom(), false));
            g.fillRoundedRectangle (seg, 3.5f);
        }
        else if (hover == i)
        {
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.fillRoundedRectangle (seg, 3.5f);
        }
        g.setFont (font (17.0f, true, 0.08f));
        g.setColour (sel ? juce::Colour (0xff142007) : juce::Colour (0xffaab2b9));
        g.drawText (i == 0 ? "ES" : "EN", seg, juce::Justification::centred, false);
    }
    g.setColour (juce::Colour (0xff858b8f).withAlpha (0.85f));
    g.drawRoundedRectangle (r.reduced (0.6f), 5.0f, 1.3f);
}

void LangToggle::mouseUp (const juce::MouseEvent& e)
{
    if (! contains (e.getPosition())) return;
    setLang (e.x < getWidth() / 2 ? Lang::es : Lang::en);
    repaint();
    if (onChange) onChange();
}

void DrawerToggle::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (1.5f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 5.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (over ? 0xff262a2b : 0xff1e2122), 0.0f, r.getY(),
                                             juce::Colour (0xff111213), 0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, 5.0f);
    if (open)
    {
        g.setColour (colours::lime.withAlpha (0.12f));
        g.drawRoundedRectangle (r.expanded (1.5f), 6.5f, 3.0f);
    }
    g.setColour (open ? colours::lime.withAlpha (down ? 0.5f : 0.85f) : juce::Colour (over ? 0xffb9c0c4 : 0xff858b8f));
    g.drawRoundedRectangle (r.reduced (0.6f), 5.0f, 1.3f);

    // teclitas
    auto keys = juce::Rectangle<float> (r.getX() + 12.0f, r.getCentreY() - 8.0f, 20.0f, 16.0f);
    g.setColour (active ? colours::lime : juce::Colour (0xff9aa4ab));
    g.drawRoundedRectangle (keys, 2.0f, 1.3f);
    for (int k = 1; k < 4; ++k) g.drawVerticalLine ((int) (keys.getX() + 5.0f * (float) k), keys.getY(), keys.getBottom());
    g.fillRect (keys.getX() + 3.5f, keys.getY(), 3.0f, 9.0f);
    g.fillRect (keys.getX() + 13.5f, keys.getY(), 3.0f, 9.0f);

    g.setFont (font (16.5f, true, 0.10f));
    g.setColour (open || over ? colours::text : juce::Colour (0xffd2d6d9));
    g.drawText (T ("TECLADO", "KEYBOARD"), r.withTrimmedLeft (40.0f).withTrimmedRight (28.0f), juce::Justification::centredLeft, false);

    juce::Path arrow;
    const float ax = r.getRight() - 18.0f, ay = r.getCentreY();
    if (open) { arrow.startNewSubPath (ax - 5.0f, ay + 2.5f); arrow.lineTo (ax, ay - 2.5f); arrow.lineTo (ax + 5.0f, ay + 2.5f); }
    else      { arrow.startNewSubPath (ax - 5.0f, ay - 2.5f); arrow.lineTo (ax, ay + 2.5f); arrow.lineTo (ax + 5.0f, ay - 2.5f); }
    g.setColour (open ? colours::lime : colours::textDim);
    g.strokePath (arrow, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MidiLeds::tick (int notes, int v)
{
    if (notes != lastNotes) { lastNotes = notes; flash = 1.0f; }
    const float old = flash;
    flash *= 0.8f;
    if (flash < 0.02f) flash = 0.0f;
    if (old > 0.0f || v != voices) { voices = v; repaint(); }
}

void MidiLeds::paint (juce::Graphics& g)
{
    const float r = 6.0f;
    const float levels[3] = { flash, voices > 0 ? 1.0f : 0.15f, 0.55f };
    for (int i = 0; i < 3; ++i)
    {
        const juce::Point<float> c (r + 3.0f + (float) i * 22.0f, (float) getHeight() * 0.5f);
        const float l = levels[i];
        if (l > 0.05f)
        {
            g.setColour (colours::lime.withAlpha (0.25f * l));
            g.fillEllipse (c.x - r - 4.0f, c.y - r - 4.0f, (r + 4.0f) * 2.0f, (r + 4.0f) * 2.0f);
        }
        g.setColour (juce::Colour (0xff2f3a22).interpolatedWith (juce::Colour (0xff8ee63b), l));
        g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.12f + 0.35f * l));
        g.fillEllipse (c.x - r * 0.45f - 1.0f, c.y - r * 0.55f - 1.0f, r * 0.7f, r * 0.6f);
    }
}

void HelpBar::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().withTrimmedLeft (9.0f).withTrimmedRight (10.0f);
    drawOneLine (g, area, [this] (float k)
    {
        juce::AttributedString s;
        s.setJustification (juce::Justification::centredLeft);
        s.append (title, font (16.5f * k, true, 0.03f), colours::text);
        s.append (U ("   ·   "), font (15.5f * k), colours::helpDesc);
        s.append (text, font (15.5f * k), colours::helpDesc);
        return s;
    }, 0.66f);
}
} // namespace dmb
