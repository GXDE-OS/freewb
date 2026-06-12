#ifndef IDBUS_H
#define IDBUS_H

#include <string>

#include "types.h"

namespace freewb::ipc
{

class IDBus
{
public:
    virtual ~IDBus() = default;

    virtual bool bindDBusSignalCallback(DBusSignalCallback callback) = 0;

    // Panel UI
    virtual void callPanelUpdateProperties(const ::freewb::ToolbarPropertiesPayload &payload) = 0;
    virtual void callPanelShowToolbar() = 0;
    virtual void callPanelHideToolbar() = 0;
    virtual void callPanelUpdateSpotRect(const ::freewb::SpotRectPayload &payload) = 0;
    virtual void callPanelUpdateCandidate(const ::freewb::CandidatePayload &payload) = 0;
    virtual void callPanelUpdatePreeditText(const ::freewb::PreeditPayload &payload) = 0;
    virtual void callPanelUpdatePreeditCaret(int caret) = 0;
    virtual void callPanelUpdateAux(const ::freewb::CandidateAuxPayload &payload) = 0;

    virtual void callPanelSwitchInputModeMethod(const std::string &inputMode) = 0;
    virtual void callPanelSwitchCharSetMethod() = 0;
    virtual void callPanelSwitchChttransMethod() = 0;
    virtual void callPanelSwitchCharWidthMethod() = 0;
    virtual void callPanelSwitchPunctuationModeMethod() = 0;
    virtual void callPanelToggleCapsStateMethod() = 0;

    // Settings
    virtual void callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) = 0;
    virtual void callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText) = 0;
    virtual void callDictQueryMethod(const std::string &wordText) = 0;
    virtual void callSwitchSkinMethod() = 0;
    virtual void callSwitchRecodeProofMethod() = 0;
    virtual void callSwitchUncommonParseStateMethod(const std::string &wordText, int flg) = 0;
    virtual void callOpenUiSettingMethod() = 0;
    virtual void callShowVersionInfoMethod() = 0;
    virtual void callOpenProfessionalSettingMethod() = 0;
    virtual void callModQuickTableMethod() = 0;
    virtual void callModUserTableMethod() = 0;
    virtual void callModWubiTableMethod() = 0;
    virtual void callModPinyinTableMethod() = 0;
    virtual void callOpenConfDirMethod() = 0;
    virtual void callSwitchVirtualKeyboardModeMethod(int flg) = 0;
    virtual void callCloseVkBoardMethod() = 0;
    virtual void callSwitchTableMethod() = 0;
    virtual void callSwitchToolbarHideFlgMethod() = 0;
    virtual void callSwitchMarkAutoPairsFlgMethod() = 0;
    virtual std::string callGetClipboardMethod() = 0;
    virtual void callWubiTableLoadOkMethod() = 0;
    virtual void callPinyinTableLoadOkMethod() = 0;
    virtual void callUsrWordLoadOkMethod() = 0;
    virtual void callQuickTableLoadOkMethod() = 0;
};

} // namespace freewb::ipc

#endif
