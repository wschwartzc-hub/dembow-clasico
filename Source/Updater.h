#pragma once

#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#include <atomic>
#include <array>

namespace dmb
{
// Revisa (en segundo plano y como máximo una vez al día) si hay una versión nueva.
// Lee un archivo público pequeño (latest.json); no envía ningún dato del usuario.
// Una sola instancia compartida por todas las ventanas del plugin (SharedResourcePointer).
class UpdateChecker : private juce::Thread
{
public:
    UpdateChecker();
    ~UpdateChecker() override;

    enum class Status { idle, checking, upToDate, available, failed, disabled };

    struct Latest
    {
        juce::String version, page, mac, win;
        juce::StringArray notesEs, notesEn;
        bool valid() const { return version.isNotEmpty(); }
        juce::String downloadUrl() const;          // el instalador para este sistema (o la página)
    };

    static juce::String currentVersion();
    static juce::String feedUrl();
    static bool isNewer (const juce::String& candidate, const juce::String& current);
    static Latest parse (const juce::String& json);

    bool isAutomatic() const;                       // el usuario lo puede apagar
    void setAutomatic (bool);
    void checkIfDue();                              // al abrir la ventana: solo si pasaron 24 h
    void checkNow();

    Status getStatus() const   { return status.load(); }
    Latest getLatest() const;
    bool hasUpdate() const;                         // hay una versión más nueva que esta
    bool shouldNotify() const;                      // hay una y el usuario no dijo "más tarde"
    void dismiss();                                 // "más tarde": no vuelve a avisar de ESTA versión
    int changeCount() const    { return changes.load(); }
    void simulate (const Latest& l)                  // para capturas y pruebas (no se guarda)
    {
        { const juce::ScopedLock sl (lock); latest = l; }
        status = Status::available;
        ++changes;
    }

private:
    void run() override;
    void store (const Latest&);

    mutable juce::CriticalSection lock;
    Latest latest;
    juce::String dismissed;
    std::atomic<Status> status { Status::idle };
    std::atomic<int> changes { 0 };
};
} // namespace dmb
