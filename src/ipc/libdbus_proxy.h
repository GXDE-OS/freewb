#ifndef LIBDBUS_PROXY_H
#define LIBDBUS_PROXY_H

#include <string>
#include <vector>

#include <dbus/dbus.h>

#include "dbus.h"
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

    void callPanelUpdateProperties(const ToolbarPropertiesPayload &payload) override;
    void callPanelShowToolbar() override;
    void callPanelHideToolbar() override;
    void callPanelUpdateSpotRect(const SpotRectPayload &payload) override;
    void callPanelUpdateCandidate(const CandidatePayload &payload) override;
    void callPanelUpdatePreeditText(const PreeditPayload &payload) override;
    void callPanelUpdatePreeditCaret(int caret) override;
    void callPanelUpdateAux(const CandidateAuxPayload &payload) override;

    void callPanelSwitchInputModeMethod(const std::string &inputMode);
    void callPanelSwitchCharSetMethod();
    void callPanelSwitchChttransMethod();
    void callPanelSwitchCharWidthModeMethod();
    void callPanelSwitchPuncModeMethod();
    void callPanelToggleCapsStateMethod();

    void callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText);
    void callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText);
    void callDictQueryMethod(const std::string &wordText);
    void callSwitchSkinMethod();
    void callSwitchRecodeProofMethod();
    void callSwitchUncommonParseStateMethod(const std::string &wordText, int flg);
    void callOpenUiSettingMethod();
    void callShowVersionInfoMethod();
    void callOpenProfessionalSettingMethod();
    void callModQuickTableMethod();
    void callModUserTableMethod();
    void callModWubiTableMethod();
    void callModPinyinTableMethod();
    void callOpenConfDirMethod();
    void callSwitchVirtualKeyboardModeMethod(int flg);
    void callCloseVkBoardMethod();
    void callSwitchTableMethod();
    std::string callGetClipboardMethod();
    void callImeTableLoadOkMethod();
    void callUsrWordLoadOkMethod();
    void callQuickTableLoadOkMethod();

private:
    static std::string toolbarPayloadToPropertyLine(const ToolbarPropertiesPayload &p);

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
