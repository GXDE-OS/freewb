#ifndef SDBUS_PROXY_H
#define SDBUS_PROXY_H

#include <string>
#include <vector>

#include <systemd/sd-bus.h>

#include "idbus.h"
#include "ifreewb.h"

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

    void callPanelUpdateProperties(const ToolbarPropertys &props) override;
    void callPanelShowToolbar() override;
    void callPanelHideToolbar() override;
    void callPanelUpdateSpotRect(const SpotRectPayload &payload) override;
    void callPanelUpdateCandidate(const CandidatePayload &payload) override;
    void callPanelUpdatePreeditText(const PreeditPayload &payload) override;
    void callPanelUpdatePreeditCaret(int caret) override;
    void callPanelUpdateAux(const CandidateAuxPayload &payload) override;

    void callPanelSwitchInputModeMethod(const std::string &inputMode) override;
    void callPanelSwitchCharSetMethod() override;
    void callPanelSwitchChttransMethod() override;
    void callPanelSwitchCharWidthMethod() override;
    void callPanelSwitchPunctuationModeMethod() override;
    void callPanelToggleCapsStateMethod() override;

    void callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) override;
    void callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) override;
    void callDictQueryMethod(const std::string &wordText) override;
    void callSwitchRecodeProofMethod() override;
    void callSwitchUncommonParseStateMethod(const std::string &wordText, int flg) override;
    void callOpenUiSettingMethod() override;
    void callShowVersionInfoMethod() override;
    void callOpenProfessionalSettingMethod() override;
    void callModQuickTableMethod() override;
    void callModUserTableMethod() override;
    void callModWubiTableMethod() override;
    void callModPinyinTableMethod() override;
    void callOpenConfDirMethod() override;
    void callSwitchVirtualKeyboardModeMethod(int flg) override;
    void callShowVkBoardMethod() override;
    void callHideVkBoardMethod() override;
    void callSwitchTableMethod() override;
    void callSwitchToolbarHideFlgMethod() override;
    void callSwitchMarkAutoPairsFlgMethod() override;
    std::string callGetClipboardMethod() override;

private:
    void sendPanelMethod(const char *member, const char *types, ...) const;
    void callSettingsMethod(const char *member, const char *types, ...) const;
    std::string callSettingsMethodReplyString(const char *member) const;

    static int handlePanelSignal(sd_bus_message *m, void *userdata, sd_bus_error *retError);

    bool registerPanelMatches();
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
