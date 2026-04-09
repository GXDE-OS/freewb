#ifndef SDBUS_PROXY_H
#define SDBUS_PROXY_H

#include <string>
#include <vector>

#include <systemd/sd-bus.h>

#include "freewb.h"
#include "idbus.h"

namespace freewb::ipc
{

/** libsystemd sd-bus 会话总线上的 IDBus，并实现 freewb::IFreewb。 */
class SDBusProxy final : public IDBus, public ::freewb::IFreewb
{
public:
    explicit SDBusProxy(void *sd_event_handle = nullptr, int priority = 0);
    ~SDBusProxy() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    bool bindDBusSignalCallback(DBusSignalCallback callback) override;

    void emitUpdateProperties(const ToolbarPropertiesPayload &payload) override;
    void emitShowToolbar() override;
    void emitHideToolbar() override;

    void emitSetSpotRect(const SpotRectPayload &payload) override;
    void emitSetCandidate(const CandidatePayload &payload) override;

    void emitUpdatePreeditText(const CandidatePreeditPayload &payload) override;
    void emitUpdatePreeditCaret(int caret) override;
    void emitUpdateAux(const CandidateAuxPayload &payload) override;

private:
    static std::string toolbarPayloadToPropertyLine(const ToolbarPropertiesPayload &p);

    void emitRegisterPropertiesSignal(const std::vector<std::string> &props);
    void emitImeSignal(const char *member, const char *types, ...) const;
    void sendPanelMethod(const char *member, const char *types, ...) const;

    static int handlePanelSignal(sd_bus_message *m, void *userdata, sd_bus_error *retError);

    bool registerPanelMatches();
    bool registerInputMethodObject();
    void clearSlots();
    void closeBus();

private:
    sd_bus *bus_ = nullptr;
    DBusSignalCallback onDBusSignal_ = nullptr;
    sd_bus_slot *panelSignalSlot_ = nullptr;
    bool available_ = true;
};

} // namespace freewb::ipc

#endif
