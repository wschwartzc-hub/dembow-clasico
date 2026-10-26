#include "PresetBrowser.h"

namespace dmb
{
namespace
{
    constexpr int W = 1590, H = 752;
    constexpr int headerRowH = 46, itemRowH = 50;

    const juce::Colour panelBg   { 0xff0e1011 };
    const juce::Colour listBg    { 0xff090a0a };
    const juce::Colour edge      { 0xff353a3d };
    const juce::Colour textMain  { 0xffe6e7ea };
    const juce::Colour textSoft  { 0xffc9ced3 };

    juce::String normalise (juce::String s)
    {
        s = lower (s);
        const char* from[] = { "á", "é", "í", "ó", "ú", "ü", "ñ" };
        const char* to[]   = { "a", "e", "i", "o", "u", "u", "n" };
        for (int i = 0; i < 7; ++i)
            s = s.replace (U (from[i]), to[i]);
        return s;
    }

    void drawStar (juce::Graphics& g, juce::Point<float> c, float r, bool filled, bool hot)
    {
        juce::Path star;
        star.addStar (c, 5, r * 0.45f, r, 0.0f);
        if (filled) { g.setColour (colours::lime); g.fillPath (star); }
        else        { g.setColour (hot ? textSoft : juce::Colour (0xff5d656c)); g.strokePath (star, juce::PathStrokeType (1.4f)); }
    }

    void styleButton (juce::TextButton& b, float size = 17.0f)
    {
        b.getProperties().set ("fontSize", size);
        b.getProperties().set ("glowPad", 0.0f);
        b.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void styleEditor (juce::TextEditor& t)
    {
        t.setFont (font (20.0f));
        t.setIndents (14, 0);
        t.setJustification (juce::Justification::centredLeft);
        t.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff060707));
        t.setColour (juce::TextEditor::outlineColourId, edge);
        t.setColour (juce::TextEditor::focusedOutlineColourId, colours::lime.withAlpha (0.8f));
        t.setColour (juce::TextEditor::textColourId, textMain);
        t.setSelectAllWhenFocused (false);
    }
}

// =============================================================================================
// Lista de categorías (columna izquierda)
// =============================================================================================
class PresetBrowser::CategoryList : public juce::Component
{
public:
    explicit CategoryList (PresetBrowser& b) : owner (b) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    static constexpr float rowH = 28.0f, pitch = 29.5f;   // 19 filas + 2 separaciones caben en 582 px

    void paint (juce::Graphics& g) override
    {
        float y = 0.0f;
        for (size_t i = 0; i < owner.categories.size(); ++i)
        {
            auto& c = owner.categories[i];
            if (c.gapBefore) y += 10.0f;
            auto r = juce::Rectangle<float> (0.0f, y, (float) getWidth(), rowH);
            const bool sel = c.key == owner.categoryKey;
            const bool hot = (int) i == hover;
            if (sel)
            {
                g.setColour (colours::lime.withAlpha (0.10f));
                g.fillRoundedRectangle (r, 6.0f);
                g.setColour (colours::lime);
                g.fillRoundedRectangle (r.withWidth (4.0f).reduced (0.0f, 6.0f), 2.0f);
            }
            else if (hot)
            {
                g.setColour (juce::Colours::white.withAlpha (0.04f));
                g.fillRoundedRectangle (r, 6.0f);
            }
            float x = 14.0f;
            if (c.type >= 0)
            {
                drawTypeIcon (g, c.type, juce::Rectangle<float> (x, y + 6.0f, 18.0f, 18.0f), sel ? colours::lime : juce::Colour (0xff8a9399), 1.6f);
                x += 28.0f;
            }
            g.setFont (font (17.0f, sel));
            g.setColour (sel ? textMain : textSoft);
            g.drawText (c.label, r.withTrimmedLeft (x).withTrimmedRight (56.0f), juce::Justification::centredLeft, true);
            g.setFont (font (14.0f));
            g.setColour (sel ? colours::lime : juce::Colour (0xff6e767d));
            g.drawText (juce::String (c.count), r.withTrimmedRight (12.0f), juce::Justification::centredRight, false);
            y += pitch;
        }
    }

    int indexAt (float py) const
    {
        float y = 0.0f;
        for (size_t i = 0; i < owner.categories.size(); ++i)
        {
            if (owner.categories[i].gapBefore) y += 10.0f;
            if (py >= y && py < y + rowH) return (int) i;
            y += pitch;
        }
        return -1;
    }

    void mouseMove (const juce::MouseEvent& e) override { setHover (indexAt ((float) e.y)); }
    void mouseExit (const juce::MouseEvent&) override  { setHover (-1); }
    void mouseDown (const juce::MouseEvent& e) override
    {
        const int i = indexAt ((float) e.y);
        if (i >= 0) owner.selectCategory (owner.categories[(size_t) i].key);
    }

private:
    void setHover (int i) { if (i != hover) { hover = i; repaint(); } }
    PresetBrowser& owner;
    int hover = -1;
};

// =============================================================================================
// Lista de presets (derecha)
// =============================================================================================
class PresetBrowser::PresetList : public juce::Component
{
public:
    explicit PresetList (PresetBrowser& b) : owner (b) {}

    int totalHeight() const { return owner.rowY.empty() ? 0 : owner.rowY.back() + 12; }

    int rowAt (int py) const
    {
        const auto& ry = owner.rowY;
        if (ry.size() < 2 || py < 0) return -1;
        auto it = std::upper_bound (ry.begin(), ry.end(), py);
        const int idx = (int) (it - ry.begin()) - 1;
        return idx >= 0 && idx < (int) owner.rows.size() ? idx : -1;
    }

    void paint (juce::Graphics& g) override
    {
        const auto& rows = owner.rows;
        if (rows.empty())
        {
            g.setFont (font (19.0f));
            g.setColour (juce::Colour (0xff7d868e));
            juce::String msg = T ("No hay presets que coincidan con tu búsqueda.", "No presets match your search.");
            if (owner.categoryKey == "fav")  msg = T ("Marca con ★ tus presets favoritos para tenerlos siempre a la mano.", "Mark your favorite presets with ★ to keep them at hand.");
            if (owner.categoryKey == "user") msg = T ("Todavía no guardas presets. Ajusta el sonido a tu gusto y usa «Guardar como…».", "You haven't saved any presets yet. Dial in a sound you like and use “Save as…”.");
            g.drawFittedText (msg, getLocalBounds().reduced (40, 0).withHeight (160), juce::Justification::centred, 2);
            return;
        }
        const auto clip = g.getClipBounds();
        const auto cur = owner.currentId();
        int first = std::max (0, rowAt (clip.getY()));
        for (int i = first; i < (int) rows.size(); ++i)
        {
            const int y = owner.rowY[(size_t) i];
            if (y > clip.getBottom()) break;
            const auto& row = rows[(size_t) i];
            const int h = owner.rowY[(size_t) i + 1] - y;
            auto r = juce::Rectangle<float> (0.0f, (float) y, (float) getWidth(), (float) h);
            if (row.header) paintHeader (g, r, row);
            else            paintItem (g, r, owner.entries[(size_t) row.entry], cur, i == hover);
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int r = rowAt (e.y);
        const bool overStar = e.x < 50;
        if (r != hover || overStar != hoverStar) { hover = r; hoverStar = overStar; repaint(); }
        setMouseCursor (r >= 0 && ! owner.rows[(size_t) r].header ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    }
    void mouseExit (const juce::MouseEvent&) override { hover = -1; repaint(); }
    void mouseDown (const juce::MouseEvent& e) override
    {
        const int r = rowAt (e.y);
        if (r >= 0) owner.clickedRow (r, e.x < 50, e.getNumberOfClicks() >= 2);
    }

private:
    void paintHeader (juce::Graphics& g, juce::Rectangle<float> r, const Row& row)
    {
        auto t = r.withTrimmedLeft (16.0f).withTrimmedBottom (6.0f);
        g.setFont (font (15.0f, true, 0.14f));
        g.setColour (colours::lime.withAlpha (0.9f));
        const auto title = upper (row.text);
        g.drawText (title, t, juce::Justification::bottomLeft, true);
        if (row.sub.isNotEmpty())
        {
            const float tw = juce::GlyphArrangement::getStringWidth (font (15.0f, true, 0.14f), title);
            g.setFont (font (14.5f));
            g.setColour (colours::helpDesc);
            g.drawText (row.sub, t.withTrimmedLeft (tw + 18.0f).withTrimmedRight (16.0f), juce::Justification::bottomLeft, true);
        }
        g.setColour (edge);
        g.fillRect (r.getX() + 16.0f, r.getBottom() - 2.0f, r.getWidth() - 32.0f, 1.0f);
    }

    void paintItem (juce::Graphics& g, juce::Rectangle<float> r, const PresetEntry& e, const juce::String& cur, bool hot)
    {
        auto box = r.reduced (4.0f, 3.0f);
        const bool isCurrent = e.id == cur;
        if (isCurrent)
        {
            g.setColour (colours::lime.withAlpha (0.09f));
            g.fillRoundedRectangle (box, 7.0f);
            g.setColour (colours::lime.withAlpha (0.75f));
            g.drawRoundedRectangle (box, 7.0f, 1.4f);
        }
        else if (hot)
        {
            g.setColour (juce::Colours::white.withAlpha (0.045f));
            g.fillRoundedRectangle (box, 7.0f);
        }
        drawStar (g, { box.getX() + 24.0f, box.getCentreY() }, 9.5f, owner.isFav (e.id), hot && hoverStar);

        float right = box.getRight() - 12.0f;
        for (auto& c : owner.chipsFor (e))
        {
            const auto f = font (14.0f);
            const float w = juce::GlyphArrangement::getStringWidth (f, c) + 20.0f;
            auto chip = juce::Rectangle<float> (right - w, box.getCentreY() - 13.0f, w, 26.0f);
            g.setColour (juce::Colours::white.withAlpha (0.04f));
            g.fillRoundedRectangle (chip, 13.0f);
            g.setColour (edge.brighter (0.2f));
            g.drawRoundedRectangle (chip.reduced (0.5f), 12.5f, 1.0f);
            g.setFont (f);
            g.setColour (colours::bpm);
            g.drawText (c, chip, juce::Justification::centred, false);
            right = chip.getX() - 7.0f;
        }

        auto text = box.withTrimmedLeft (50.0f).withRight (right - 12.0f);
        if (e.isUser() || e.description.isNotEmpty())
        {
            g.setFont (font (18.5f, true));
            g.setColour (isCurrent ? colours::lime : textMain);
            g.drawText (e.name, text.withHeight (26.0f).withY (box.getY() + 3.0f), juce::Justification::bottomLeft, true);
            g.setFont (font (14.0f));
            g.setColour (colours::helpDesc);
            g.drawText (e.isUser() ? (categoryName (e.category) + (e.description.isNotEmpty() ? U ("  ·  ") + T ("basado en ", "based on ") + e.description : juce::String()))
                                   : e.description,
                        text.withTrimmedTop (26.0f), juce::Justification::topLeft, true);
        }
        else
        {
            g.setFont (font (18.5f, true));
            g.setColour (isCurrent ? colours::lime : textMain);
            g.drawText (e.name, text, juce::Justification::centredLeft, true);
        }
    }

    PresetBrowser& owner;
    int hover = -1;
    bool hoverStar = false;
};

// =============================================================================================
PresetBrowser::PresetBrowser (DembowProcessor& p) : proc (p)
{
    setWantsKeyboardFocus (true);

    styleEditor (search);
    search.onTextChange = [this] { rebuildRows(); viewport.setViewPosition (0, 0); };
    search.onEscapeKey = [this] { if (search.isEmpty()) close(); else { search.clear(); rebuildRows(); } };
    search.onReturnKey = [this] { close(); };
    search.addKeyListener (this);
    addAndMakeVisible (search);

    styleEditor (nameField);
    nameField.onReturnKey = [this] { confirmInline(); };
    nameField.onEscapeKey = [this] { setMode (Mode::normal); };
    addChildComponent (nameField);

    closeButton.onClick = [this] { close(); };
    addAndMakeVisible (closeButton);

    categoryList = std::make_unique<CategoryList> (*this);
    addAndMakeVisible (*categoryList);

    list = std::make_unique<PresetList> (*this);
    viewport.setViewedComponent (list.get(), false);
    viewport.setScrollBarsShown (true, false);
    viewport.setScrollBarThickness (10);
    viewport.setSingleStepSizes (itemRowH, itemRowH);
    viewport.getVerticalScrollBar().setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff4a5054));
    addAndMakeVisible (viewport);

    for (auto* b : { &saveButton, &renameButton, &deleteButton, &folderButton, &aboutButton, &confirmButton, &cancelButton })
    {
        styleButton (*b);
        addChildComponent (b);
    }
    confirmButton.setToggleState (true, juce::dontSendNotification);

    saveButton.onClick   = [this] { setMode (Mode::saving); };
    renameButton.onClick = [this] { setMode (Mode::renaming); };
    deleteButton.onClick = [this] { setMode (Mode::confirmDelete); };
    folderButton.onClick = []
    {
        auto f = DembowProcessor::getUserPresetFolder();
        f.createDirectory();
        f.startAsProcess();
    };
    aboutButton.onClick   = [this] { if (onAbout) onAbout(); };
    confirmButton.onClick = [this] { confirmInline(); };
    cancelButton.onClick  = [this] { setMode (Mode::normal); };

    applyLanguage();
    setMode (Mode::normal);
}

PresetBrowser::~PresetBrowser()
{
    search.removeKeyListener (this);
}

void PresetBrowser::applyLanguage()
{
    search.setTextToShowWhenEmpty (T ("Buscar…  (ej. marquesina, vintage, campana, eco)", "Search…  (e.g. marquesina, vintage, bell, echo)"),
                                   juce::Colour (0xff6e767d));
    nameField.setTextToShowWhenEmpty (T ("Nombre del preset", "Preset name"), juce::Colour (0xff6e767d));
    saveButton.setButtonText (T ("Guardar como…", "Save as…"));
    renameButton.setButtonText (T ("Renombrar", "Rename"));
    deleteButton.setButtonText (T ("Eliminar", "Delete"));
    folderButton.setButtonText (T ("Abrir carpeta de presets", "Open presets folder"));
    aboutButton.setButtonText (T ("Acerca de…", "About…"));
    cancelButton.setButtonText (T ("Cancelar", "Cancel"));
    setMode (mode);
    if (isVisible()) reload();
    repaint();
}

void PresetBrowser::open (bool startSaving, bool animate)
{
    applyLanguage();
    reload();
    // abre en el tipo del sonido actual
    const auto* cur = currentEntry();
    if (categoryKey.startsWith ("t:") || categoryKey.isEmpty())
        selectCategory (cur != nullptr && cur->isUser() ? juce::String ("user") : "t:" + juce::String (proc.getCategory()));
    setMode (startSaving ? Mode::saving : Mode::normal);
    if (animate) juce::Desktop::getInstance().getAnimator().fadeIn (this, 140);
    else         setVisible (true);
    toFront (false);
    scrollToCurrent();
    if (! startSaving && isShowing())
        search.grabKeyboardFocus();
}

void PresetBrowser::close()
{
    setMode (Mode::normal);
    setVisible (false);
    if (onClose) onClose();
}

void PresetBrowser::presetChangedElsewhere()
{
    if (isVisible())
    {
        list->repaint();
        scrollToCurrent();
    }
}

void PresetBrowser::setSearchText (const juce::String& t)
{
    search.setText (t, true);
    rebuildRows();
}

// ---------------------------------------------------------------------------------------------
void PresetBrowser::reload()
{
    if (entries.empty() || builtLang != langVersion())
    {
        chipCache.clear();
        builtLang = langVersion();
    }
    entries = proc.getPresetEntries();

    categories.clear();
    categories.push_back ({ "all",  T ("Todos", "All") });
    categories.push_back ({ "fav",  T ("★  Favoritos", "★  Favorites") });
    categories.push_back ({ "user", T ("Mis presets", "My presets") });
    for (int t = 0; t < numCategories; ++t)
    {
        Category c { "t:" + juce::String (t), categoryName (t) };
        c.type = t;
        c.gapBefore = t == 0;
        categories.push_back (c);
    }
    for (auto& c : categories)
        c.count = (int) std::count_if (entries.begin(), entries.end(), [&] (auto& e) { return inCategory (e, c.key); });

    if (std::none_of (categories.begin(), categories.end(), [this] (auto& c) { return c.key == categoryKey; }))
        categoryKey = "t:" + juce::String (proc.getCategory());

    rebuildRows();
    categoryList->repaint();
}

bool PresetBrowser::inCategory (const PresetEntry& e, const juce::String& key) const
{
    if (key == "all")  return true;
    if (key == "fav")  return proc.isFavorite (e.id);
    if (key == "user") return e.isUser();
    if (key.startsWith ("t:")) return e.category == key.substring (2).getIntValue();
    return false;
}

bool PresetBrowser::matches (const PresetEntry& e) const
{
    const auto q = normalise (search.getText().trim());
    if (q.isEmpty()) return true;
    juce::String hay = e.name + " " + categoryName (e.category) + " " + e.description;
    if (! e.isUser())
    {
        const auto r = presetRef (e.factoryIndex);
        const auto& a = archetypes (r.category);
        if (r.archetype < (int) a.size()) hay << " " << T (a[(size_t) r.archetype].descEs, a[(size_t) r.archetype].descEn);
    }
    hay = normalise (hay);
    for (auto& word : juce::StringArray::fromTokens (q, " ", ""))
        if (word.isNotEmpty() && ! hay.contains (word))
            return false;
    return true;
}

void PresetBrowser::rebuildRows()
{
    rows.clear();
    const bool searching = search.getText().trim().isNotEmpty();
    int lastGroup = -100000;

    for (size_t i = 0; i < entries.size(); ++i)
    {
        const auto& e = entries[i];
        if (! inCategory (e, categoryKey) || ! matches (e))
            continue;

        // encabezados: por sonido base dentro de un tipo; por tipo en las demás vistas
        const bool byArchetype = categoryKey.startsWith ("t:") && ! e.isUser();
        const int group = byArchetype ? e.archetype : (e.isUser() ? 1000 + e.category : e.category);
        if (group != lastGroup)
        {
            Row h;
            h.header = true;
            if (byArchetype)
            {
                const auto& a = archetypes (e.category)[(size_t) e.archetype];
                h.text = U (a.name);
                h.sub = T (a.descEs, a.descEn);
            }
            else
                h.text = (e.isUser() ? T ("Mis presets · ", "My presets · ") : juce::String()) + categoryName (e.category);
            rows.push_back (h);
            lastGroup = group;
        }
        rows.push_back ({ false, {}, {}, (int) i });
    }
    (void) searching;

    rowY.assign (rows.size() + 1, 0);
    int y = 0;
    for (size_t k = 0; k < rows.size(); ++k)
    {
        rowY[k] = y;
        y += rows[k].header ? headerRowH : itemRowH;
    }
    rowY[rows.size()] = y;

    list->setSize (juce::jmax (100, viewport.getWidth() - viewport.getScrollBarThickness()),
                   juce::jmax (viewport.getHeight(), list->totalHeight()));
    list->repaint();
    repaint();
}

juce::StringArray PresetBrowser::chipsFor (const PresetEntry& e)
{
    if (e.isUser()) return {};
    auto it = chipCache.find (e.factoryIndex);
    if (it != chipCache.end()) return it->second;
    Patch p;
    Macros m;
    buildPreset (e.factoryIndex, p, m);
    juce::StringArray c;
    if (m.caracter != charClean) c.add (characterNames()[m.caracter]);
    if (m.modo != playPoly) c.add (modeNames()[m.modo]);
    if (m.eco >= 15.0f) c.add (T ("Eco ", "Echo ") + delayDivisionName (p.i (gp::dlDiv)));
    if (m.octava != 0) c.add ((m.octava > 0 ? "+" : U ("−")) + juce::String (std::abs (m.octava)) + " oct");
    chipCache[e.factoryIndex] = c;
    return c;
}

void PresetBrowser::selectCategory (const juce::String& key)
{
    categoryKey = key;
    rebuildRows();
    categoryList->repaint();
    viewport.setViewPosition (0, 0);
}

const PresetEntry* PresetBrowser::currentEntry() const
{
    const auto cur = proc.getCurrentPresetId();
    for (auto& e : entries)
        if (e.id == cur) return &e;
    return nullptr;
}

void PresetBrowser::scrollToCurrent()
{
    const auto cur = proc.getCurrentPresetId();
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (! rows[i].header && entries[(size_t) rows[i].entry].id == cur)
        {
            const int y = rowY[i];
            const int top = viewport.getViewPositionY();
            if (y < top || y + itemRowH > top + viewport.getHeight())
                viewport.setViewPosition (0, juce::jmax (0, y - viewport.getHeight() / 3));
            return;
        }
    }
}

void PresetBrowser::clickedRow (int r, bool onStar, bool doubleClick)
{
    if (! juce::isPositiveAndBelow (r, (int) rows.size()) || rows[(size_t) r].header)
        return;
    const auto& e = entries[(size_t) rows[(size_t) r].entry];
    if (onStar)
    {
        proc.setFavorite (e.id, ! proc.isFavorite (e.id));
        const auto pos = viewport.getViewPositionY();
        reload();
        viewport.setViewPosition (0, pos);
        return;
    }
    if (mode == Mode::confirmDelete || mode == Mode::renaming)
        setMode (Mode::normal);
    proc.loadEntry (e);
    list->repaint();
    updateButtons();
    if (doubleClick)
        close();
}

void PresetBrowser::moveSelection (int delta)
{
    std::vector<int> items;
    for (size_t i = 0; i < rows.size(); ++i)
        if (! rows[i].header) items.push_back ((int) i);
    if (items.empty()) return;
    const auto cur = proc.getCurrentPresetId();
    int pos = -1;
    for (size_t k = 0; k < items.size(); ++k)
        if (entries[(size_t) rows[(size_t) items[k]].entry].id == cur) pos = (int) k;
    pos = pos < 0 ? (delta > 0 ? 0 : (int) items.size() - 1) : juce::jlimit (0, (int) items.size() - 1, pos + delta);
    clickedRow (items[(size_t) pos], false, false);
    scrollToCurrent();
}

bool PresetBrowser::keyPressed (const juce::KeyPress& k, juce::Component*)
{
    if (k == juce::KeyPress::upKey)   { moveSelection (-1); return true; }
    if (k == juce::KeyPress::downKey) { moveSelection (+1); return true; }
    return false;
}

bool PresetBrowser::keyPressed (const juce::KeyPress& k)
{
    if (k == juce::KeyPress::escapeKey) { mode == Mode::normal ? close() : setMode (Mode::normal); return true; }
    if (k == juce::KeyPress::returnKey) { close(); return true; }
    return keyPressed (k, this);
}

// ---------------------------------------------------------------------------------------------
void PresetBrowser::setMode (Mode m)
{
    mode = m;
    const auto* cur = currentEntry();
    switch (mode)
    {
        case Mode::saving:
            prompt = T ("Guardar como:", "Save as:");
            nameField.setText (cur != nullptr && cur->isUser() ? cur->name : proc.getPresetName() + T (" (mío)", " (mine)"), false);
            confirmButton.setButtonText (T ("Guardar", "Save"));
            break;
        case Mode::renaming:
            prompt = T ("Nuevo nombre:", "New name:");
            nameField.setText (cur != nullptr ? cur->name : juce::String(), false);
            confirmButton.setButtonText (T ("Renombrar", "Rename"));
            break;
        case Mode::confirmDelete:
            prompt = T ("¿Eliminar «", "Delete “") + (cur != nullptr ? cur->name : juce::String()) + T ("»? Se mueve a la papelera.", "”? It goes to the trash.");
            confirmButton.setButtonText (T ("Sí, eliminar", "Yes, delete"));
            break;
        case Mode::confirmOverwrite:
            prompt = T ("Ya existe «", "“") + pendingName + T ("». ¿Reemplazarlo?", "” already exists. Replace it?");
            confirmButton.setButtonText (T ("Reemplazar", "Replace"));
            break;
        case Mode::normal:
            prompt.clear();
            break;
    }
    const bool editing = mode == Mode::saving || mode == Mode::renaming;
    nameField.setVisible (editing);
    confirmButton.setVisible (mode != Mode::normal);
    cancelButton.setVisible (mode != Mode::normal);
    resized();
    updateButtons();
    if (editing && isShowing())
    {
        nameField.grabKeyboardFocus();
        nameField.selectAll();
    }
    repaint();
}

void PresetBrowser::updateButtons()
{
    const bool normal = mode == Mode::normal;
    const auto* cur = currentEntry();
    const bool userPreset = cur != nullptr && cur->isUser();
    saveButton.setVisible (normal);
    renameButton.setVisible (normal);
    deleteButton.setVisible (normal);
    renameButton.setEnabled (userPreset);
    deleteButton.setEnabled (userPreset);
    folderButton.setVisible (true);
    aboutButton.setVisible (true);
}

void PresetBrowser::confirmInline()
{
    const auto* cur = currentEntry();
    switch (mode)
    {
        case Mode::saving:
        case Mode::confirmOverwrite:
        {
            if (mode == Mode::saving)
                pendingName = juce::File::createLegalFileName (nameField.getText().trim());
            if (pendingName.isEmpty()) return;
            const auto target = DembowProcessor::getUserPresetFolder().getChildFile (pendingName + ".dmbpreset");
            if (mode == Mode::saving && target.existsAsFile() && ! (cur != nullptr && cur->isUser() && cur->file == target))
            {
                setMode (Mode::confirmOverwrite);
                return;
            }
            if (proc.saveUserPreset (pendingName))
            {
                reload();
                selectCategory ("user");
                scrollToCurrent();
            }
            setMode (Mode::normal);
            break;
        }
        case Mode::renaming:
            if (cur != nullptr && cur->isUser() && proc.renameUserPreset (cur->file, nameField.getText()))
                reload();
            setMode (Mode::normal);
            break;
        case Mode::confirmDelete:
            if (cur != nullptr && cur->isUser())
                proc.deleteUserPreset (cur->file);
            reload();
            setMode (Mode::normal);
            break;
        case Mode::normal:
            break;
    }
}

// ---------------------------------------------------------------------------------------------
void PresetBrowser::resized()
{
    search.setBounds (330, 16, 680, 42);
    closeButton.setBounds (W - 66, 14, 48, 46);
    categoryList->setBounds (16, 86, 290, 582);
    viewport.setBounds (320, 86, W - 336, 582);
    if (! rows.empty() || ! entries.empty()) rebuildRows();

    const int by = 684, bh = 46;
    folderButton.setBounds (16, by, 290, bh);
    aboutButton.setBounds (W - 16 - 170, by, 170, bh);
    saveButton.setBounds (320, by, 200, bh);
    renameButton.setBounds (532, by, 160, bh);
    deleteButton.setBounds (704, by, 140, bh);

    const bool editing = mode == Mode::saving || mode == Mode::renaming;
    const int px = 320 + (editing ? 170 : 560);
    nameField.setBounds (490, by, 480, bh);
    confirmButton.setBounds (editing ? 982 : px, by, 170, bh);
    cancelButton.setBounds (confirmButton.getRight() + 12, by, 140, bh);
}

void PresetBrowser::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    for (int i = 1; i <= 4; ++i)
    {
        g.setColour (juce::Colours::black.withAlpha (0.12f));
        g.drawRoundedRectangle (r.reduced ((float) i * 0.5f), 14.0f, 2.0f);
    }
    g.setColour (panelBg);
    g.fillRoundedRectangle (r.reduced (2.0f), 12.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.035f), 0.0f, 0.0f,
                                             juce::Colours::transparentWhite, 0.0f, 90.0f, false));
    g.fillRoundedRectangle (r.reduced (2.0f).withHeight (90.0f), 12.0f);
    g.setColour (edge);
    g.drawRoundedRectangle (r.reduced (2.0f), 12.0f, 1.6f);

    g.setFont (font (24.0f, true, 0.28f));
    g.setColour (colours::lime);
    g.drawText ("PRESETS", juce::Rectangle<float> (32.0f, 16.0f, 200.0f, 42.0f), juce::Justification::centredLeft);
    g.setFont (font (15.0f));
    g.setColour (juce::Colour (0xff6e767d));
    g.drawText (juce::String ((int) entries.size()) + T (" en total", " in total"), juce::Rectangle<float> (190.0f, 16.0f, 130.0f, 42.0f),
                juce::Justification::centredLeft);

    {
        auto c = closeButton.getBounds().toFloat().getCentre();
        g.setColour (closeButton.isOver() ? colours::lime : textSoft);
        g.drawLine (c.x - 9.0f, c.y - 9.0f, c.x + 9.0f, c.y + 9.0f, 2.0f);
        g.drawLine (c.x + 9.0f, c.y - 9.0f, c.x - 9.0f, c.y + 9.0f, 2.0f);
    }

    g.setColour (edge);
    g.fillRect (16.0f, 72.0f, (float) W - 32.0f, 1.0f);
    g.fillRect (16.0f, 674.0f, (float) W - 32.0f, 1.0f);

    auto lb = viewport.getBounds().toFloat().expanded (2.0f);
    g.setColour (listBg);
    g.fillRoundedRectangle (lb, 8.0f);
    g.setColour (edge.withAlpha (0.8f));
    g.drawRoundedRectangle (lb, 8.0f, 1.0f);

    const juce::Rectangle<float> bar (320.0f, 684.0f, (float) W - 336.0f - 186.0f, 46.0f);
    if (mode == Mode::normal)
    {
        g.setFont (font (14.5f));
        g.setColour (juce::Colour (0xff6e767d));
        g.drawFittedText (T ("Clic = escuchar  ·  doble clic o Enter = elegir y cerrar  ·  ↑ ↓ = recorrer  ·  ★ = favorito",
                             "Click = audition  ·  double-click or Enter = pick and close  ·  ↑ ↓ = browse  ·  ★ = favorite"),
                          bar.withTrimmedLeft (540.0f).toNearestInt(), juce::Justification::centredLeft, 2);
    }
    else
    {
        g.setFont (font (17.0f, true));
        g.setColour (mode == Mode::confirmDelete ? juce::Colour (0xffff8a5c) : textMain);
        const auto pr = (mode == Mode::saving || mode == Mode::renaming) ? bar.withWidth (160.0f) : bar.withWidth (550.0f);
        g.drawFittedText (prompt, pr.toNearestInt(), juce::Justification::centredLeft, 2);
    }
}
} // namespace dmb
