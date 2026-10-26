#pragma once

#include "Widgets.h"
#include "../PluginProcessor.h"

namespace dmb
{
// Gestor de presets: los 16 tipos (500 cada uno), favoritos, tus presets y buscador.
class PresetBrowser : public juce::Component,
                      private juce::KeyListener
{
public:
    explicit PresetBrowser (DembowProcessor&);
    ~PresetBrowser() override;

    static juce::Rectangle<int> boundsInDesign() { return { 40, 100, 1590, 752 }; }

    std::function<void()> onClose, onAbout;

    void open (bool startSaving = false, bool animate = true);
    void close();
    void presetChangedElsewhere();
    void applyLanguage();

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress&) override;

    struct Row { bool header = false; juce::String text, sub; int entry = -1; };
    const std::vector<PresetEntry>& getEntries() const { return entries; }
    const std::vector<Row>& getRows() const { return rows; }
    void clickedRow (int rowIndex, bool onStar, bool doubleClick);
    void selectCategory (const juce::String& key);
    juce::String getCategoryKey() const { return categoryKey; }
    void setSearchText (const juce::String&);
    juce::String currentId() const { return proc.getCurrentPresetId(); }
    bool isFav (const juce::String& id) { return proc.isFavorite (id); }

    // chips de un preset de fábrica (carácter, eco, modo…), calculados al pintarlo
    juce::StringArray chipsFor (const PresetEntry&);

private:
    enum class Mode { normal, saving, renaming, confirmDelete, confirmOverwrite };
    struct Category { juce::String key, label; int count = 0; bool gapBefore = false; int type = -1; };

    class CategoryList;
    class PresetList;

    bool keyPressed (const juce::KeyPress&, juce::Component*) override;

    void reload();
    void rebuildRows();
    bool matches (const PresetEntry& e) const;
    bool inCategory (const PresetEntry& e, const juce::String& key) const;
    void moveSelection (int delta);
    void scrollToCurrent();
    const PresetEntry* currentEntry() const;
    void setMode (Mode m);
    void confirmInline();
    void updateButtons();

    DembowProcessor& proc;
    std::vector<PresetEntry> entries;
    std::vector<Row> rows;
    std::vector<int> rowY;   // posición de cada fila (para pintar rápido 8,000)
    std::vector<Category> categories;
    juce::String categoryKey { "t:0" };
    std::map<int, juce::StringArray> chipCache;
    int builtLang = -1;

    juce::TextEditor search, nameField;
    Hotspot closeButton { 6.0f };
    std::unique_ptr<CategoryList> categoryList;
    std::unique_ptr<PresetList> list;
    juce::Viewport viewport;

    juce::TextButton saveButton, renameButton, deleteButton, folderButton, aboutButton;
    juce::TextButton confirmButton, cancelButton;
    Mode mode = Mode::normal;
    juce::String prompt, pendingName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowser)
};
} // namespace dmb
