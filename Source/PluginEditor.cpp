#include "PluginEditor.h"

using namespace dmb;

#ifndef DMB_YEAR
 #define DMB_YEAR 2026
#endif
#ifndef DMB_DISPLAY_NAME
 #define DMB_DISPLAY_NAME "Dembow Clásico"
#endif

namespace
{
    juce::Rectangle<int> R (int x0, int y0, int x1, int y1) { return { x0, y0, x1 - x0, y1 - y0 }; }

    PlateKnob::Spec big (float x, float y, float body, float arcR, bool bipolar)
    {
        PlateKnob::Spec s { { x, y }, body, arcR, 9.2f, true };
        s.bipolar = bipolar;
        return s;
    }
    PlateKnob::Spec small (float x, float y, float body, bool bipolar = false)
    {
        PlateKnob::Spec s { { x, y }, body, 47.5f, 6.2f, false };
        s.bipolar = bipolar;
        return s;
    }

    const juce::Colour labelBig    { 0xffd6d7d7 };
    const juce::Colour labelBigSub { 0xffacb2b4 };
    const juce::Colour labelSmall  { 0xffd8dbe1 };
    const juce::Colour labelSub    { 0xffa2adb6 };
    const juce::Colour labelPanel  { 0xffbec8d4 };

    juce::String displayName() { return juce::String::fromUTF8 (DMB_DISPLAY_NAME); }
}

// =============================================================================================
DembowEditor::Stage::Stage()
{
    plate = juce::ImageCache::getFromMemory (DmbAssets::plate_jpg, DmbAssets::plate_jpgSize);
    setOpaque (true);
}

void DembowEditor::Stage::paint (juce::Graphics& g)
{
    const float s = g.getInternalContext().getPhysicalPixelScaleFactor();
    if (s <= 0.0f || std::abs (s - 1.0f) < 0.001f)
    {
        g.drawImageAt (plate, 0, 0);
        return;
    }
    if (cached.isNull() || std::abs (s - cachedScale) > 0.001f)
    {
        cached = plate.rescaled (juce::roundToInt ((float) plate.getWidth() * s), juce::roundToInt ((float) plate.getHeight() * s),
                                 juce::Graphics::highResamplingQuality);
        cachedScale = s;
    }
    g.drawImageTransformed (cached, juce::AffineTransform::scale ((float) plate.getWidth() / (float) cached.getWidth(),
                                                                  (float) plate.getHeight() / (float) cached.getHeight()));
}

// =============================================================================================
DembowEditor::DembowEditor (DembowProcessor& p)
    : AudioProcessorEditor (p), proc (p), types (p), scope (p)
{
    auto& ap = proc.apvts;

    setLookAndFeel (&lnf);
    addAndMakeVisible (stage);
    stage.setBounds (0, 0, layout::designW, layout::designH);
    stage.addMouseListener (this, true);

    addLabel ("Guardar", "Save", R (1410, 48, 1576, 82), font (20.5f), juce::Colour (0xffd1d9e2));

    addKnobLabels (211.0f,  306.0f, 327.0f, true, "BRILLO", "brightness", "BRIGHTNESS", "tone");
    addKnobLabels (1250.0f, 309.5f, 330.5f, true, "DURACIÓN", "length", "LENGTH", "how long notes ring");
    addKnobLabels (1486.5f, 308.5f, 331.0f, true, "ESPACIO", "reverb", "SPACE", "reverb");

    const struct { int x; float y; const char* es; const char* en; } panels[] = {
        { 91, 620.5f, "FORMA", "SHAPE" }, { 357, 619.5f, "CARÁCTER", "CHARACTER" },
        { 692, 621.0f, "MOVIMIENTO", "MOTION" }, { 985, 621.0f, "TOCAR", "PLAY" },
        { 1390, 622.5f, "SALIDA", "OUTPUT" },
    };
    for (auto& pn : panels)
        addLabel (pn.es, pn.en, juce::Rectangle<int> (pn.x, juce::roundToInt (pn.y) - 12, 240, 24),
                  font (17.0f, false, 0.12f), labelPanel, juce::Justification::centredLeft);

    addKnobLabels (118.0f,  654.0f, 672.5f, false, "ATAQUE", "attack", "ATTACK", "fade-in");
    addKnobLabels (248.0f,  653.5f, 673.0f, false, "GROSOR", "thickness", "THICKNESS", "detune & width");
    addKnobLabels (581.5f,  650.5f, 671.0f, false, "SUCIEDAD", "dirt", "DIRT", "amount");
    addKnobLabels (743.5f,  652.5f, 672.5f, false, "VIBRATO", "vibrato", "VIBRATO", "pitch wobble");
    addKnobLabels (875.5f,  653.0f, 671.5f, false, "GLIDE", "portamento", "GLIDE", "slide between notes");
    addKnobLabels (1171.0f, 654.5f, 673.0f, false, "OCTAVA", "octave", "OCTAVE", "transpose");
    addKnobLabels (1290.5f, 654.0f, 673.0f, false, "FUERZA", "velocity", "VELOCITY", "touch response");
    addKnobLabels (1440.5f, 654.5f, 674.5f, false, "ECO", "delay", "ECHO", "tempo delay");
    addKnobLabels (1570.5f, 653.5f, 674.5f, false, "VOLUMEN", "output", "VOLUME", "output level");

    // ----- encabezado ------------------------------------------------------------------------
    stage.addAndMakeVisible (leds);
    leds.setBounds (R (492, 56, 562, 80));

    addReadout (std::make_unique<Readout> ([this] { return presetKey(); },
        [this] (juce::Graphics& g, juce::Rectangle<float> r)
        {
            drawOneLine (g, r.reduced (8.0f, 0.0f), [this] (float k)
            {
                juce::AttributedString s;
                s.setJustification (juce::Justification::centred);
                s.append (upper (categoryName (proc.getCategory())) + U ("  ·  "), font (17.0f * k, true, 0.1f), colours::lime);
                s.append (proc.getPresetName(), font (21.0f * k), colours::text);
                if (proc.isPresetModified())
                    s.append (T ("  (modificado)", "  (modified)"), font (17.0f * k), colours::textDim);
                return s;
            });
        }), R (655, 48, 1053, 86));

    for (auto* b : { &prevButton, &nextButton, &presetButton, &saveButton, &logoButton })
        stage.addAndMakeVisible (b);
    logoButton.setBounds (R (64, 40, 480, 94));
    logoButton.onClick = [this] { if (updater->shouldNotify()) showUpdateDialog(); else showAbout(); };
    prevButton.setBounds   (R (586, 45, 643, 90));
    nextButton.setBounds   (R (1061, 45, 1118, 90));
    presetButton.setBounds (R (648, 45, 1057, 90));
    saveButton.setBounds   (R (1408, 43, 1581, 89));
    prevButton.onClick   = [this] { stepPreset (-1); };
    nextButton.onClick   = [this] { stepPreset (+1); };
    presetButton.onClick = [this] { toggleBrowser(); };
    saveButton.onClick   = [this] { browser->open (true); };

    setHelp (presetButton, { "PRESETS", "PRESETS",
        "Abre el gestor: los 16 tipos con 500 sonidos cada uno, buscador, favoritos y tus presets.",
        "Opens the manager: the 16 types with 500 sounds each, search, favorites and your own presets." });
    const Help stepHelp { "ANTERIOR / SIGUIENTE", "PREVIOUS / NEXT",
        "Pasa al sonido anterior o al siguiente del mismo tipo.", "Goes to the previous or next sound of the same type." };
    setHelp (prevButton, stepHelp);
    setHelp (nextButton, stepHelp);
    setHelp (saveButton, { "GUARDAR", "SAVE",
        "Guarda tu sonido como preset propio (Documentos / Dembow Clasico / Presets).",
        "Saves your sound as your own preset (Documents / Dembow Clasico / Presets)." });

    stage.addAndMakeVisible (diceButton);
    diceButton.setBounds (R (1136, 46, 1266, 86));
    diceButton.onClick = [this] { diceButton.roll(); proc.rollDice(); browser->presetChangedElsewhere(); };
    setHelp (diceButton, { "DADO", "DICE",
        "Inventa un sonido nuevo al azar del tipo que tienes elegido. Si te gusta, guárdalo.",
        "Invents a random new sound of the type you have selected. If you like it, save it." });

    stage.addAndMakeVisible (langToggle);
    langToggle.setBounds (R (1280, 46, 1398, 86));
    langToggle.onChange = [this] { applyLanguage(); };
    setHelp (langToggle, { "IDIOMA", "LANGUAGE",
        "Cambia el idioma del plugin entre español e inglés. Se recuerda para la próxima vez.",
        "Switches the plugin between Spanish and English. It's remembered for next time." });

    // ----- pantalla: los 16 tipos -------------------------------------------------------------
    stage.addAndMakeVisible (types);
    types.setBounds (TypeSelector::boundsInDesign());
    types.onPick = [this] (int c) { proc.selectCategory (c); browser->presetChangedElsewhere(); };
    setHelp (types, { "TIPO DE SONIDO", "SOUND TYPE",
        "Elige qué quieres: lead, pluck, bajo, pad, piano… Cada tipo tiene 500 sonidos; recórrelos con las flechas de arriba.",
        "Pick what you need: lead, pluck, bass, pad, piano… Each type has 500 sounds; browse them with the arrows above." });

    // ----- grabadora (tocar sin teclado) y osciloscopio ---------------------------------------
    stage.addAndMakeVisible (boombox);
    boombox.setBounds (BoomboxModule::boundsInDesign());
    boombox.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    boombox.onPress = [this] (bool down) { pressBoombox (down); };
    setHelp (boombox, { "TOCAR", "PLAY",
        "Mantén apretada la grabadora para escuchar el sonido sin teclado.",
        "Hold the boombox to hear the sound without a keyboard." });
    stage.addAndMakeVisible (scope);
    scope.setBounds (SlotScope::boundsInDesign());

    // ----- BRILLO, DURACIÓN, ESPACIO -----------------------------------------------------------
    auto& brillo = addKnob (big (198.5f, 430.0f, 62.0f, 87.0f, true), pid::brillo);
    auto& duracion = addKnob (big (1252.5f, 433.5f, 59.5f, 81.0f, true), pid::duracion);
    auto& espacio = addKnob (big (1490.5f, 433.5f, 57.5f, 78.5f, false), pid::espacio);
    setHelp (brillo, { "BRILLO", "BRIGHTNESS",
        "Más oscuro a la izquierda, más brillante a la derecha. En medio suena como viene el preset.",
        "Darker to the left, brighter to the right. In the middle it sounds as the preset comes." });
    setHelp (duracion, { "DURACIÓN", "LENGTH",
        "Qué tanto duran las notas: a la izquierda cortitas, a la derecha largas. En medio, como viene el preset.",
        "How long the notes ring: short to the left, long to the right. In the middle, as the preset comes." });
    setHelp (espacio, { "ESPACIO", "SPACE",
        "Cuánta reverb (sala). 0 = seco, 20-35 = sala normal, más = lejano y grande.",
        "How much reverb (room). 0 = dry, 20-35 = normal room, more = big and far away." });

    addValue (brillo, R (90, 516, 320, 546), 26.0f, colours::lime);
    addReadout (Readout::simple ([this]
        {
            const float v = proc.apvts.getRawParameterValue (pid::brillo)->load();
            if (std::abs (v - 50.0f) < 1.0f) return T ("como viene el sonido", "as the sound comes");
            return v < 50.0f ? T ("más oscuro", "darker") : T ("más brillante", "brighter");
        }, font (16.0f), [] { return colours::textDim; }, juce::Justification::centred), R (70, 545, 340, 569));

    addValue (duracion, R (1140, 516, 1366, 546), 26.0f, colours::lime);
    addReadout (Readout::simple ([this]
        {
            const float v = proc.apvts.getRawParameterValue (pid::duracion)->load();
            if (std::abs (v - 50.0f) < 1.0f) return T ("como viene el sonido", "as the sound comes");
            return v < 50.0f ? T ("notas más cortas", "shorter notes") : T ("notas más largas", "longer notes");
        }, font (16.0f), [] { return colours::textDim; }, juce::Justification::centred), R (1130, 545, 1376, 569));

    addValue (espacio, R (1378, 516, 1604, 546), 26.0f, colours::lime);
    addReadout (Readout::simple ([this]
        {
            const float v = proc.apvts.getRawParameterValue (pid::espacio)->load();
            if (v < 0.5f) return T ("seco", "dry");
            if (v < 20.0f) return T ("un toque de sala", "a touch of room");
            if (v < 40.0f) return T ("sala", "room");
            if (v < 65.0f) return T ("sala grande", "big room");
            return T ("lejos, muy grande", "far away, huge");
        }, font (16.0f), [] { return colours::textDim; }, juce::Justification::centred), R (1366, 545, 1616, 569));

    // ----- fila de abajo ---------------------------------------------------------------------
    struct SmallDef { float x, y, body; const char* id; bool bipolar; Help help; };
    const SmallDef smalls[] = {
        { 112.0f,  743.0f, 35.5f, pid::ataque, true, { "ATAQUE", "ATTACK",
          "Cómo empieza la nota: a la izquierda más golpe, a la derecha entra despacio (como un pad).",
          "How the note starts: more punch to the left, slower fade-in to the right (like a pad)." } },
        { 244.0f,  742.5f, 35.5f, pid::grosor, true, { "GROSOR", "THICKNESS",
          "A la derecha el sonido se hace más gordo y ancho (más desafinado y con chorus); a la izquierda, más fino.",
          "To the right the sound gets fatter and wider (more detune and chorus); to the left, thinner." } },
        { 579.0f,  741.5f, 36.0f, pid::suciedad, false, { "SUCIEDAD", "DIRT",
          "Cuánto se nota el carácter elegido: más bits rotos, más cinta, más saturación.",
          "How much of the chosen character you hear: more broken bits, more tape, more saturation." } },
        { 741.5f,  741.5f, 35.5f, pid::vibrato, false, { "VIBRATO", "VIBRATO",
          "Hace que la nota ondule de afinación después de un momento. La rueda de modulación de tu teclado también lo sube.",
          "Makes the note waver in pitch after a moment. Your keyboard's mod wheel adds it too." } },
        { 873.5f,  741.5f, 35.5f, pid::glide, false, { "GLIDE", "GLIDE",
          "La nota se desliza desde la anterior (portamento). Ideal en MONO o LEGATO para leads y bajos.",
          "The note slides from the previous one (portamento). Great in MONO or LEGATO for leads and basses." } },
        { 1172.0f, 740.5f, 34.5f, pid::octava, true, { "OCTAVA", "OCTAVE",
          "Sube o baja todo el sonido una o dos octavas.", "Moves the whole sound up or down one or two octaves." } },
        { 1289.0f, 740.5f, 34.5f, pid::fuerza, false, { "FUERZA", "VELOCITY",
          "Qué tanto cambia el sonido con la fuerza con la que tocas. 0 % = todas las notas igual.",
          "How much the sound responds to how hard you play. 0 % = every note the same." } },
        { 1443.0f, 742.0f, 35.0f, pid::eco, false, { "ECO", "ECHO",
          "Eco al ritmo de tu canción (normalmente con puntillo, muy de la época). 0 = sin eco.",
          "Echo in time with your song (usually dotted, very era-specific). 0 = no echo." } },
        { 1572.5f, 742.5f, 34.5f, pid::volumen, false, { "VOLUMEN", "VOLUME",
          "Volumen final. Todos los presets vienen nivelados. Doble clic en cualquier knob lo regresa a su valor inicial.",
          "Final volume. Every preset comes level-matched. Double-click any knob to reset it." } },
    };
    for (auto& d : smalls)
    {
        auto spec = small (d.x, d.y, d.body, d.bipolar);
        const bool isOutput = juce::String (d.id) == pid::volumen;
        if (isOutput) spec.colour = juce::Colour (0xffa7acaf);
        auto& k = addKnob (spec, d.id);
        setHelp (k, d.help);
        const int cx = juce::roundToInt (d.x);
        addValue (k, R (cx - 62, 792, cx + 62, 819), 19.0f, isOutput ? juce::Colour (0xffdcdcde) : colours::lime);
    }

    characterButtons = std::make_unique<ChoiceButtons> (stage, *ap.getParameter (pid::caracter),
        juce::StringArray { "", "", "", "" }, juce::Array<float> { 0.0f, 1.0f, 2.0f, 3.0f },
        juce::Array<juce::Rectangle<int>> { R (348, 650, 497, 681), R (348, 695, 497, 726), R (348, 739, 497, 770), R (348, 784, 497, 815) },
        17.5f);
    modeButtons = std::make_unique<ChoiceButtons> (stage, *ap.getParameter (pid::modo),
        juce::StringArray { "", "", "" }, juce::Array<float> { 0.0f, 1.0f, 2.0f },
        juce::Array<juce::Rectangle<int>> { R (979, 667, 1099, 704), R (979, 717, 1099, 754), R (979, 766, 1099, 803) },
        17.5f);
    const Help charHelp { "CARÁCTER", "CHARACTER",
        "LIMPIO: tal cual. VINTAGE: como un sampler de los 2000 (menos bits). CÁLIDO: cinta y saturación. LO-FI: casete viejo. SUCIEDAD dice cuánto.",
        "CLEAN: as is. VINTAGE: like a 2000s sampler (fewer bits). WARM: tape and saturation. LO-FI: old cassette. DIRT sets how much." };
    for (int i = 0; i < characterButtons->size(); ++i) setHelp (*characterButtons->button (i), charHelp);
    const Help modeHelp { "MODO", "PLAY MODE",
        "POLY: acordes. MONO: una nota a la vez. LEGATO: una nota a la vez y, si tocas ligado, se desliza sin volver a atacar.",
        "POLY: chords. MONO: one note at a time. LEGATO: one note at a time and, if you play smoothly, it slides without re-attacking." };
    for (int i = 0; i < modeButtons->size(); ++i) setHelp (*modeButtons->button (i), modeHelp);

    // ----- ayuda y teclado ---------------------------------------------------------------------
    stage.addAndMakeVisible (helpBar);
    helpBar.setBounds (HelpBar::boundsInDesign());

    stage.addAndMakeVisible (keyboardToggle);
    keyboardToggle.setBounds (R (1452, 858, 1624, 895));
    keyboardToggle.onClick = [this] { setKeyboardOpen (! proc.keyboardOpen); };
    setHelp (keyboardToggle, { "TECLADO", "KEYBOARD",
        "Abre un teclado para tocar con el mouse o con las teclas de tu computadora.",
        "Opens a keyboard to play with the mouse or your computer keys." });

    keyboard = std::make_unique<KeyboardDrawer> (proc);
    addChildComponent (*keyboard);
    keyboard->setBounds (0, 0, KeyboardDrawer::width, KeyboardDrawer::height);
    keyboard->addMouseListener (this, true);

    browser = std::make_unique<PresetBrowser> (proc);
    stage.addChildComponent (*browser);
    browser->setBounds (PresetBrowser::boundsInDesign());
    browser->onAbout = [this] { showAbout(); };
    browser->onClose = [this] { showDefaultHelp(); };
    setHelp (*browser, { "PRESETS", "PRESETS",
        "Clic en un preset para escucharlo, doble clic para elegirlo y cerrar. ★ = favorito. Escribe para buscar.",
        "Click a preset to audition it, double-click to pick it and close. ★ = favorite. Type to search." });

    setResizable (true, true);
    keyboard->setVisible (proc.keyboardOpen);
    updateConstraints();
    updater->checkIfDue();

    timerCallback();
    showDefaultHelp();
    setSize (juce::roundToInt (baseWidth * defaultScale), juce::roundToInt ((float) totalHeight() * defaultScale));
    startTimerHz (30);
}

DembowEditor::~DembowEditor()
{
    stopTimer();
    pressBoombox (false);
    stage.removeMouseListener (this);
    keyboard->removeMouseListener (this);
    setLookAndFeel (nullptr);
}

// =============================================================================================
PlateKnob& DembowEditor::addKnob (PlateKnob::Spec spec, const char* paramID)
{
    auto* k = knobs.add (new PlateKnob (spec));
    stage.addAndMakeVisible (k);
    k->setBounds (k->boundsInDesign());
    attachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, paramID, *k));
    if (auto* p = proc.apvts.getParameter (paramID))
        k->setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    return *k;
}

void DembowEditor::addReadout (std::unique_ptr<Readout> r, juce::Rectangle<int> b)
{
    stage.addAndMakeVisible (*r);
    r->setBounds (b);
    readouts.add (r.release());
}

void DembowEditor::addValue (PlateKnob& knob, juce::Rectangle<int> area, float size, juce::Colour colour)
{
    auto* k = &knob;
    addReadout (Readout::simple ([k] { return k->getTextFromValue (k->getValue()); },
                                 font (size, true), [colour] { return colour; }, juce::Justification::centred), area);
}

void DembowEditor::addLabel (const char* es, const char* en, juce::Rectangle<int> area, juce::Font f, juce::Colour c, juce::Justification j)
{
    const juce::String sEs = U (es), sEn = U (en);
    addReadout (Readout::simple ([sEs, sEn] { return english() ? sEn : sEs; }, f, [c] { return c; }, j), area);
}

void DembowEditor::addKnobLabels (float cx, float titleY, float subY, bool isBig, const char* es, const char* sub, const char* en, const char* subEn)
{
    const int halfW = isBig ? 140 : 66;
    const int x = juce::roundToInt (cx) - halfW;
    addLabel (es, en, juce::Rectangle<int> (x, juce::roundToInt (titleY) - 13, halfW * 2, 26),
              isBig ? font (21.0f, true, 0.06f) : font (16.5f, true, 0.05f), isBig ? labelBig : labelSmall);
    addLabel (sub, subEn, juce::Rectangle<int> (x, juce::roundToInt (subY) - 11, halfW * 2, 22),
              isBig ? font (16.0f) : font (15.0f), isBig ? labelBigSub : labelSub);
}

void DembowEditor::setHelp (juce::Component& c, const Help& h)
{
    setHelpProps (c, U (h.titleEs), U (h.titleEn), U (h.textEs), U (h.textEn));
}

void DembowEditor::showHelpFor (juce::Component* c)
{
    if (c == nullptr) { showDefaultHelp(); return; }
    const bool en = english();
    const auto& p = c->getProperties();
    helpBar.set (p[en ? "helpTitleEn" : "helpTitleEs"].toString(), p[en ? "helpEn" : "helpEs"].toString());
}

void DembowEditor::showDefaultHelp()
{
    auto desc = proc.getPresetDescription();
    if (proc.isPresetModified())
        desc = T ("(modificado)  ", "(modified)  ") + desc;
    helpBar.set (upper (proc.getPresetName()), desc);
}

void DembowEditor::mouseEnter (const juce::MouseEvent& e)
{
    for (auto* c = e.eventComponent; c != nullptr && c != this; c = c->getParentComponent())
    {
        if (c->getProperties().contains ("help"))
        {
            hovered = c;
            showHelpFor (c);
            return;
        }
    }
    hovered = nullptr;
    showDefaultHelp();
}

void DembowEditor::mouseExit (const juce::MouseEvent&)
{
    if (! stage.isMouseOver (true) && ! keyboard->isMouseOver (true))
    {
        hovered = nullptr;
        showDefaultHelp();
    }
}

// =============================================================================================
void DembowEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0b0b));
}

void DembowEditor::resized()
{
    const float scale = (float) getWidth() / (float) baseWidth;
    stage.setTransform (juce::AffineTransform::translation ((float) -layout::viewX, (float) -layout::viewY).scaled (scale));
    keyboard->setTransform (juce::AffineTransform::translation (0.0f, (float) baseHeight).scaled (scale));
}

int DembowEditor::totalHeight() const
{
    return baseHeight + (proc.keyboardOpen ? KeyboardDrawer::height : 0);
}

void DembowEditor::updateConstraints()
{
    const int h = totalHeight();
    setResizeLimits (juce::roundToInt (baseWidth * 0.5f), juce::roundToInt ((float) h * 0.5f),
                     juce::roundToInt (baseWidth * 1.25f), juce::roundToInt ((float) h * 1.25f));
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) baseWidth / (double) h);
}

void DembowEditor::setKeyboardOpen (bool open)
{
    if (open == proc.keyboardOpen && keyboard->isVisible() == open)
        return;
    float scale = (float) getWidth() / (float) baseWidth;
    proc.keyboardOpen = open;
    keyboard->setVisible (open);
    const int h = totalHeight();
    if (isShowing())
        if (auto* d = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
        {
            const float maxH = (float) d->userBounds.getHeight() * 0.92f;
            if ((float) h * scale > maxH)
                scale = juce::jmax (0.5f, maxH / (float) h);
        }
    updateConstraints();
    setSize (juce::roundToInt ((float) baseWidth * scale), juce::roundToInt ((float) h * scale));
    keyboardToggle.repaint();
}

void DembowEditor::pressBoombox (bool down)
{
    if (previewNote >= 0)
    {
        proc.keyboardState.noteOff (1, previewNote, 0.0f);
        previewNote = -1;
    }
    if (down)
    {
        previewNote = proc.getCategory() == catBass ? 36 : 60;
        proc.keyboardState.noteOn (1, previewNote, 0.85f);
    }
}

// =============================================================================================
juce::String DembowEditor::presetKey() const
{
    return juce::String (proc.getCategory()) + "|" + proc.getPresetName() + "|" + (proc.isPresetModified() ? "*" : "");
}

void DembowEditor::applyLanguage()
{
    lastLangVersion = langVersion();
    characterButtons->setLabels ({ T ("LIMPIO", "CLEAN"), U ("VINTAGE"), T ("CÁLIDO", "WARM"), U ("LO-FI") });
    modeButtons->setLabels ({ U ("POLY"), U ("MONO"), U ("LEGATO") });
    for (auto* r : readouts) r->poll();
    if (browser->isVisible()) browser->applyLanguage();
    keyboard->applyLanguage();
    keyboardToggle.repaint();
    types.repaint();
    boombox.repaint();
    langToggle.repaint();
    diceButton.repaint();
    showHelpFor (hovered.getComponent());
    proc.updateHostDisplay (juce::AudioProcessor::ChangeDetails().withParameterInfoChanged (true));
}

void DembowEditor::timerCallback()
{
    if (lastLangVersion != langVersion())
        applyLanguage();

    for (auto* r : readouts)
        r->poll();

    if (updater->changeCount() != lastUpdateChange)
    {
        lastUpdateChange = updater->changeCount();
        const bool upd = updater->shouldNotify();
        setHelp (logoButton, upd ? Help { "NUEVA VERSIÓN", "NEW VERSION",
                                          "Hay una versión nueva. Haz clic para ver qué trae y descargarla.",
                                          "A new version is out. Click to see what's new and download it." }
                                 : Help { "ACERCA DE", "ABOUT", "Versión, licencia y créditos.", "Version, license and credits." });
        if (hovered.getComponent() == &logoButton) showHelpFor (&logoButton);
    }

    types.tick();
    boombox.tick (proc.outputLevel.exchange (0.0f));
    scope.tick();
    leds.tick (proc.notesPlayed.load(), proc.voicesActive.load());
    diceButton.tick();
    keyboardToggle.setState (proc.keyboardOpen, proc.voicesActive.load() > 0);

    const auto pk = presetKey();
    if (pk != lastPresetKey)
    {
        lastPresetKey = pk;
        if (hovered.getComponent() == nullptr)
            showDefaultHelp();
    }
}

void DembowEditor::toggleBrowser()
{
    if (browser->isVisible()) browser->close();
    else                      browser->open (false);
}

void DembowEditor::stepPreset (int delta)
{
    proc.stepPreset (delta);
    browser->presetChangedElsewhere();
}

void DembowEditor::showUpdateDialog()
{
    const auto l = updater->getLatest();
    juce::String text = T ("Ya está disponible ", "") + displayName() + " " + l.version
                        + T (" (tienes la ", " is out (you have ") + UpdateChecker::currentVersion() + ").\n\n";
    const auto& notes = english() ? l.notesEn : l.notesEs;
    if (! notes.isEmpty())
    {
        text << T ("Novedades:\n", "What's new:\n");
        for (auto& n : notes) text << U ("•  ") << n << "\n";
        text << "\n";
    }
    text << T ("Descarga el instalador, cierra tu DAW e instálalo encima: tus presets y proyectos se conservan.",
               "Download the installer, quit your DAW and install it over this one: your presets and projects are kept.");
    auto* w = new juce::AlertWindow (T ("Hay una versión nueva", "A new version is out"), text, juce::MessageBoxIconType::NoIcon, this);
    w->addButton (T ("Descargar", "Download"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton (T ("Más tarde", "Later"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<DembowEditor> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, l] (int r)
    {
        if (safe == nullptr) return;
        if (r == 1) juce::URL (l.downloadUrl()).launchInDefaultBrowser();
        else        safe->updater->dismiss();
    }), true);
}

void DembowEditor::showAbout()
{
    juce::String updates;
    if (UpdateChecker::feedUrl().isNotEmpty())
    {
        switch (updater->getStatus())
        {
            case UpdateChecker::Status::checking:  updates = T ("Buscando actualizaciones…", "Checking for updates…"); break;
            case UpdateChecker::Status::available: updates = T ("Hay una versión nueva: ", "A new version is out: ") + updater->getLatest().version; break;
            case UpdateChecker::Status::upToDate:  updates = T ("Tienes la versión más reciente.", "You have the latest version."); break;
            case UpdateChecker::Status::failed:    updates = T ("No se pudo revisar si hay versiones nuevas (¿sin internet?).", "Couldn't check for updates (no internet?)."); break;
            default: break;
        }
        updates << "\n\n";
    }
    const juce::String text =
        displayName() + U (" ") + JucePlugin_VersionString + U ("\n")
        + U ("© ") + juce::String (DMB_YEAR) + U (" ") + JucePlugin_Manufacturer
        + T (". Todos los derechos reservados.\n\n", ". All rights reserved.\n\n")
        + T ("16 tipos de sonido con 500 presets cada uno: 8,000 sonidos de reggaetón old school, todos hechos con síntesis (sin samples).\n\n",
             "16 sound types with 500 presets each: 8,000 old-school reggaeton sounds, all made with synthesis (no samples).\n\n")
        + T ("Formato: ", "Format: ") + juce::AudioProcessor::getWrapperTypeDescription (proc.wrapperType) + U ("\n\n")
        + T ("Hecho con JUCE. VST es una marca registrada de Steinberg Media Technologies GmbH. Audio Units es una marca de Apple Inc.\n\n",
             "Made with JUCE. VST is a registered trademark of Steinberg Media Technologies GmbH. Audio Units is a trademark of Apple Inc.\n\n")
        + updates
        + T ("La licencia de uso y los avisos de software de terceros vienen en el instalador (Licencias.txt).",
             "The license agreement and third-party notices are included with the installer (Licencias.txt).");
    auto* w = new juce::AlertWindow (T ("Acerca de ", "About ") + displayName(), text, juce::MessageBoxIconType::NoIcon, this);
    w->addButton (T ("Cerrar", "Close"), 0, juce::KeyPress (juce::KeyPress::escapeKey));
    if (UpdateChecker::feedUrl().isNotEmpty())
        w->addButton (updater->hasUpdate() ? T ("Descargar la nueva", "Download the new one") : T ("Buscar ahora", "Check now"), 1);
    juce::Component::SafePointer<DembowEditor> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe] (int r)
    {
        if (safe == nullptr || r != 1) return;
        auto& u = *safe->updater;
        if (u.hasUpdate()) juce::URL (u.getLatest().downloadUrl()).launchInDefaultBrowser();
        else               u.checkNow();
    }), true);
}
