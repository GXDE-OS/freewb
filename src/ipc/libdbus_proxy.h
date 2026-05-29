#ifndef LIBDBUS_PROXY_H
#define LIBDBUS_PROXY_H

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "idbus.h"
#include "ifreewb.h"

namespace freewb::ipc
{

/** libdbus-1 会话总线上的 IDBus，并实现 freewb::IFreewb。 */
class LibDbusProxy final : public IDBus, public ::freewb::IFreewb
{
public:
    explicit LibDbusProxy(void *dbus_connection = nullptr);
    ~LibDbusProxy() override;

    const char *name() const override;
    bool available() const override;
    void changeAvailable() override;

    bool bindDBusSignalCallback(DBusSignalCallback callback) override;

    void callPanelUpdateProperties(const ::freewb::ToolbarPropertiesPayload &payload) override;
    void callPanelShowToolbar() override;
    void callPanelHideToolbar() override;
    void callPanelUpdateSpotRect(const ::freewb::SpotRectPayload &payload) override;
    void callPanelUpdateCandidate(const ::freewb::CandidatePayload &payload) override;
    void callPanelUpdatePreeditText(const ::freewb::PreeditPayload &payload) override;
    void callPanelUpdatePreeditCaret(int caret) override;
    void callPanelUpdateAux(const ::freewb::CandidateAuxPayload &payload) override;

    void callPanelSwitchInputModeMethod(const std::string &inputMode) override;
    void callPanelSwitchCharSetMethod() override;
    void callPanelSwitchChttransMethod() override;
    void callPanelSwitchCharWidthModeMethod() override;
    void callPanelSwitchPuncModeMethod() override;
    void callPanelToggleCapsStateMethod() override;

    void callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) override;
    void callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) override;
    void callDictQueryMethod(const std::string &wordText) override;
    void callSwitchSkinMethod() override;
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
    void callCloseVkBoardMethod() override;
    void callSwitchTableMethod() override;
    void callSwitchToolbarHideFlgMethod() override;
    std::string callGetClipboardMethod() override;
    void callImeTableLoadOkMethod() override;
    void callUsrWordLoadOkMethod() override;
    void callQuickTableLoadOkMethod() override;

private:
    static std::string toolbarPayloadToPropertyLine(const ::freewb::ToolbarPropertiesPayload &p);

    void sendPanelMethod(const char *member, const char *types, ...) const;
    void sendPanelRegisterProperties(const std::vector<std::string> &props) const;
    void callSettingsMethod(const char *member, const char *types, ...) const;
    std::string callSettingsMethodReplyString(const char *member) const;

    static DBusHandlerResult handlePanelSignal(DBusConnection *conn, DBusMessage *msg, void *userdata);

    bool registerPanelMatches();
    void clearMatches();
    void closeBus();

private:
    DBusConnection *conn_ = nullptr;
    DBusSignalCallback onDBusSignal_;
    bool filterAdded_ = false;
    bool available_ = true;
};

} // namespace freewb::ipc

#endif
