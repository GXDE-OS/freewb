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

    bool bindDBusCallbacks(const DBusCallbacks &callbacks) override;

    void emitUpdateProperties(const ToolbarPropertiesPayload &payload) override;
    void emitShowToolbar() override;
    void emitHideToolbar() override;

    void sendSetSpotRect(const SpotRectPayload &payload) override;
    void sendSetCandidate(const CandidatePayload &payload) override;

    void emitUpdateCandidate(const SpotRectPayload &spotRect, const CandidatePayload &candidate, const CandidatePreeditPayload &preedit, const CandidateAuxPayload &aux) override;

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
    DBusCallbacks callbacks_;
    sd_bus_slot *panelSignalSlot_ = nullptr;
    bool available_ = true;
};

} // namespace freewb::ipc

#endif
