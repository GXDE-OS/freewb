#include "sdbus_proxy.h"

#include <cerrno>
#include <cstdarg>
#include <string>
#include <vector>

#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

#include "ipc.h"
#include "log.h"

namespace freewb::ipc
{

namespace
{

/** NULL-terminated strv for sd_bus; pointers only valid while @p strings and @p scratch are alive. */
static char **makeStrv(const std::vector<std::string> &strings, std::vector<char *> &scratch)
{
    if (strings.empty())
    {
        return nullptr;
    }
    scratch.clear();

    scratch.reserve(strings.size() + 1U);
    for (const auto &s : strings)
    {
        scratch.push_back(const_cast<char *>(s.c_str()));
    }
    scratch.push_back(nullptr);
    return scratch.data();
}

/** Append @p payload as SetLookupTable D-Bus body: as, as, as, b, b, i, i. */
static int appendSetCandidateBody(sd_bus_message *m, const CandidatePayload &payload)
{
    std::vector<char *> strvLabels;
    std::vector<char *> strvTexts;
    std::vector<char *> strvPrompt;

    int r = sd_bus_message_append_strv(m, makeStrv(payload.fullCodes, strvLabels));
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_strv(m, makeStrv(payload.texts, strvTexts));
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_strv(m, makeStrv(payload.prompts, strvPrompt));
    if (r < 0)
    {
        return r;
    }
    int hasPrev = payload.hasPrev ? 1 : 0;
    int hasNext = payload.hasNext ? 1 : 0;
    int cursor = payload.cursor;
    int layout = static_cast<int>(payload.layout);
    r = sd_bus_message_append_basic(m, 'b', &hasPrev);
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_basic(m, 'b', &hasNext);
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_basic(m, 'i', &cursor);
    if (r < 0)
    {
        return r;
    }
    return sd_bus_message_append_basic(m, 'i', &layout);
}

} // namespace

std::string SDBusProxy::toolbarPayloadToPropertyLine(const ToolbarPropertiesPayload &p)
{
    if (p.uniqueName == "fullwidth" || p.uniqueName == "punc")
    {
        const std::string st = p.active ? "active" : "inactive";
        return "/Fcitx/" + p.uniqueName + ":" + p.shortDescription + ":fcitx-" + p.uniqueName + "-" + st + ":" +
               p.longDescription;
    }
    return "/Fcitx/im:" + p.uniqueName + ":" + p.name;
}

SDBusProxy::SDBusProxy(void *sd_event_handle, int priority)
{
    if (sd_bus_open_user(&bus_) < 0)
    {
        bus_ = nullptr;
        return;
    }
    if (!sd_event_handle)
    {
        return;
    }
    if (sd_bus_attach_event(bus_, static_cast<sd_event *>(sd_event_handle), priority) < 0)
    {
        sd_bus_flush_close_unref(bus_);
        bus_ = nullptr;
    }
}

SDBusProxy::~SDBusProxy()
{
    clearSlots();
    closeBus();
}

void SDBusProxy::closeBus()
{
    if (!bus_)
    {
        return;
    }
    sd_bus_detach_event(bus_);
    sd_bus_flush_close_unref(bus_);
    bus_ = nullptr;
}

const char *SDBusProxy::name() const
{
    return "ipc-sdbusproxy";
}

bool SDBusProxy::available() const
{
    return available_ && bus_ != nullptr;
}

void SDBusProxy::changeAvailable()
{
    available_ = !available_;
}

bool SDBusProxy::bindDBusSignalCallback(DBusSignalCallback callback)
{
    onDBusSignal_ = callback;
    if (!bus_)
    {
        return false;
    }
    if (panelSignalSlot_)
    {
        return true;
    }
    return registerPanelMatches();
}

void SDBusProxy::callPanelUpdateProperties(const ToolbarPropertiesPayload &payload)
{
    sendPanelRegisterProperties({toolbarPayloadToPropertyLine(payload)});
}

void SDBusProxy::callPanelShowToolbar()
{
    sendPanelMethod("UpdateProperty", "s", "/Fcitx/im:Freewb");
}

void SDBusProxy::callPanelHideToolbar()
{
    sendPanelMethod("UpdateProperty", "s", "/Fcitx/im:us");
}

void SDBusProxy::callPanelUpdateSpotRect(const SpotRectPayload &payload)
{
    FREEWB_DEBUG("emitUpdateSpotRect x={} y={} w={} h={}", payload.x, payload.y, payload.w, payload.h);
    sendPanelMethod("SetSpotRect", "iiii", payload.x, payload.y, payload.w, payload.h);
}

void SDBusProxy::callPanelUpdateCandidate(const CandidatePayload &payload)
{
    if (!bus_ || !available_)
    {
        FREEWB_ERROR("emitUpdateCandidate skipped: bus={} available_={}", static_cast<const void *>(bus_), available_);
        return;
    }
    FREEWB_DEBUG("emitUpdateCandidate: texts={} prompts={} hasPrev={} hasNext={} cursor={} layout={}", payload.texts.size(),
                 payload.prompts.size(), payload.hasPrev, payload.hasNext, payload.cursor, static_cast<int>(payload.layout));

    sd_bus_message *m = nullptr;
    const int newCallR = sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH,
                                                        FREEWUBI_PANEL_INTERFACE, "SetLookupTable");
    if (newCallR < 0)
    {
        FREEWB_ERROR("emitUpdateCandidate: sd_bus_message_new_method_call(SetLookupTable) failed: {} ({})", newCallR,
                     strerror(-newCallR));
        return;
    }

    const int appendR = appendSetCandidateBody(m, payload);
    if (appendR < 0)
    {
        FREEWB_ERROR("emitUpdateCandidate: append candidate body failed: {} ({})", appendR, strerror(-appendR));
        sd_bus_message_unref(m);
        return;
    }
    const int sendR = sd_bus_send(bus_, m, nullptr);
    sd_bus_message_unref(m);
    if (sendR < 0)
    {
        FREEWB_ERROR("emitUpdateCandidate: sd_bus_send(SetLookupTable) failed: {} ({})", sendR, strerror(-sendR));
        return;
    }
    const int hasLookup = !payload.texts.empty() ? 1 : 0;
    sendPanelMethod("ShowLookupTable", "b", hasLookup);
}

void SDBusProxy::callPanelUpdatePreeditText(const PreeditPayload &payload)
{
    static const char *const kEmptyAttr = "";
    sendPanelMethod("UpdatePreeditText", "ss", payload.text.c_str(), kEmptyAttr);
    sendPanelMethod("ShowPreedit", "b", payload.show ? 1 : 0);
}

void SDBusProxy::callPanelUpdatePreeditCaret(int caret)
{
    sendPanelMethod("UpdatePreeditCaret", "i", caret);
}

void SDBusProxy::callPanelUpdateAux(const CandidateAuxPayload &payload)
{
    static const char *const kEmptyAttr = "";
    sendPanelMethod("UpdateAux", "ss", payload.text.c_str(), kEmptyAttr);
    sendPanelMethod("ShowAux", "b", payload.show ? 1 : 0);
}

void SDBusProxy::callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText)
{
    callSettingsMethod("slot_dbus_generate_usr_word", "iss", flg, wordText.c_str(), wordCode.c_str());
}

void SDBusProxy::callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText)
{
    callSettingsMethod("slot_dbus_delete_usr_word", "iss", flg, wordText.c_str(), wordCode.c_str());
}

void SDBusProxy::callDictQueryMethod(const std::string &wordText)
{
    callSettingsMethod("slot_dbus_dict_query", "s", wordText.c_str());
}

void SDBusProxy::callPanelSwitchInputModeMethod(const std::string &inputMode)
{
    sendPanelMethod("SwitchInputMode", "s", inputMode.c_str());
}

void SDBusProxy::callSwitchSkinMethod()
{
    callSettingsMethod("slot_dbus_switch_skin", "");
}

void SDBusProxy::callSwitchVirtualKeyboardModeMethod(int flg)
{
    callSettingsMethod("slot_dbus_switch_vk", "i", flg);
}

void SDBusProxy::callPanelSwitchCharSetMethod()
{
    sendPanelMethod("SwitchCharSet", "");
}

void SDBusProxy::callSwitchRecodeProofMethod()
{
    callSettingsMethod("slot_dbus_set_recode_calib_flg", "i", 0);
}

void SDBusProxy::callSwitchUncommonParseStateMethod(const std::string &wordText, int flg)
{
    callSettingsMethod("slot_dbus_word_freq_switch_ok", "is", flg, wordText.c_str());
}

void SDBusProxy::callPanelSwitchChttransMethod()
{
    sendPanelMethod("SwitchSimpOrTrad", "");
}

void SDBusProxy::callOpenUiSettingMethod()
{
    callSettingsMethod("slot_dbus_open_ui_setting", "");
}

void SDBusProxy::callShowVersionInfoMethod()
{
    callSettingsMethod("slot_dbus_show_version_info", "");
}

void SDBusProxy::callOpenProfessionalSettingMethod()
{
    callSettingsMethod("slot_dbus_open_advanced_setting", "");
}

void SDBusProxy::callModQuickTableMethod()
{
    callSettingsMethod("slot_dbus_edit_quick_table", "");
}

void SDBusProxy::callModUserTableMethod()
{
    callSettingsMethod("slot_dbus_edit_usr_table", "");
}

void SDBusProxy::callModWubiTableMethod()
{
    callSettingsMethod("slot_dbus_edit_wubi_table", "");
}

void SDBusProxy::callModPinyinTableMethod()
{
    callSettingsMethod("slot_dbus_edit_pinyin_table", "");
}

void SDBusProxy::callOpenConfDirMethod()
{
    callSettingsMethod("slot_dbus_open_freewb_dir", "");
}

void SDBusProxy::callCloseVkBoardMethod()
{
    callSettingsMethod("slot_dbus_close_vk", "");
}

void SDBusProxy::callSwitchTableMethod()
{
    callSettingsMethod("slot_dbus_switch_lexicon", "");
}

void SDBusProxy::callSwitchToolbarHideFlgMethod()
{
    callSettingsMethod("slot_dbus_switch_toolbar_hide_flg", "");
}

void SDBusProxy::callPanelSwitchCharWidthModeMethod()
{
    sendPanelMethod("SetCharWidthAndMarkMode", "ii", 1, 0);
}

void SDBusProxy::callPanelSwitchPuncModeMethod()
{
    sendPanelMethod("SetCharWidthAndMarkMode", "ii", 0, 1);
}

std::string SDBusProxy::callGetClipboardMethod()
{
    return callSettingsMethodReplyString("slot_dbus_get_clipboard_text");
}

void SDBusProxy::callPanelToggleCapsStateMethod()
{
    callSettingsMethod("slot_dbus_switch_caps_state", "");
}

void SDBusProxy::callImeTableLoadOkMethod()
{
    callSettingsMethod("slot_dbus_ime_table_load_ok", "");
}

void SDBusProxy::callUsrWordLoadOkMethod()
{
    callSettingsMethod("slot_dbus_usr_word_load_ok", "");
}

void SDBusProxy::callQuickTableLoadOkMethod()
{
    callSettingsMethod("slot_dbus_quick_table_load_ok", "");
}

int SDBusProxy::handlePanelSignal(sd_bus_message *m, void *userdata, sd_bus_error *)
{
    auto *self = static_cast<SDBusProxy *>(userdata);
    if (!self)
    {
        FREEWB_WARN("panel signal: userdata null");
        return 0;
    }
    if (!m)
    {
        FREEWB_WARN("panel signal: message null");
        return 0;
    }
    const char *member = sd_bus_message_get_member(m);
    if (!member)
    {
        FREEWB_WARN("panel signal: member null");
        return 0;
    }

    if (self->onDBusSignal_)
    {
        FREEWB_DEBUG("panel signal passthrough: member={}", member);
        sd_bus_message_rewind(m, true);
        int index = 0;
        int32_t tmp = 0;
        if (sd_bus_message_read(m, "i", &tmp) >= 0)
        {
            index = static_cast<int>(tmp);
        }
        self->onDBusSignal_(member, index);
    }
    else
    {
        FREEWB_WARN("panel signal: {} no callback", member);
    }
    return 0;
}

void SDBusProxy::sendPanelMethod(const char *member, const char *types, ...) const
{
    if (!bus_ || !available_ || !member)
    {
        return;
    }
    sd_bus_message *m = nullptr;
    if (sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE,
                                       member) < 0)
    {
        return;
    }
    if (types && types[0] != '\0')
    {
        va_list ap;
        va_start(ap, types);
        const int r = sd_bus_message_appendv(m, types, ap);
        va_end(ap);
        if (r < 0)
        {
            sd_bus_message_unref(m);
            return;
        }
    }
    sd_bus_send(bus_, m, nullptr);
    sd_bus_message_unref(m);
}

void SDBusProxy::sendPanelRegisterProperties(const std::vector<std::string> &props) const
{
    if (!bus_ || !available_)
    {
        return;
    }
    sd_bus_message *m = nullptr;
    if (sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE,
                                       "RegisterProperties") < 0)
    {
        return;
    }
    std::vector<char *> strv;
    if (sd_bus_message_append_strv(m, makeStrv(props, strv)) < 0)
    {
        sd_bus_message_unref(m);
        return;
    }
    sd_bus_send(bus_, m, nullptr);
    sd_bus_message_unref(m);
}

void SDBusProxy::callSettingsMethod(const char *member, const char *types, ...) const
{
    if (!bus_ || !available_ || !member)
    {
        FREEWB_WARN("callSettingsMethod skipped: bus={} available_={} member={}", static_cast<const void *>(bus_), available_,
                    member ? member : "(null)");
        return;
    }

    FREEWB_DEBUG("callSettingsMethod: member={} types={}", member, (types && types[0] != '\0') ? types : "(none)");

    sd_bus_message *m = nullptr;
    const int newCallR = sd_bus_message_new_method_call(bus_, &m, FREEWUBI_SETTINGS_SERVICENAME, FREEWUBI_SETTINGS_OBJECTPATH,
                                                        FREEWUBI_SETTINGS_INTERFACE, member);
    if (newCallR < 0)
    {
        FREEWB_ERROR("callSettingsMethod: new_method_call({}) failed: {} ({})", member, newCallR, strerror(-newCallR));
        return;
    }
    if (types && types[0] != '\0')
    {
        va_list ap;
        va_start(ap, types);
        const int r = sd_bus_message_appendv(m, types, ap);
        va_end(ap);
        if (r < 0)
        {
            FREEWB_ERROR("callSettingsMethod: appendv member={} types={} failed: {} ({})", member, types, r, strerror(-r));
            sd_bus_message_unref(m);
            return;
        }
    }
    const int sendR = sd_bus_send(bus_, m, nullptr);
    if (sendR < 0)
    {
        FREEWB_ERROR("callSettingsMethod: send member={} failed: {} ({})", member, sendR, strerror(-sendR));
    }
    else
    {
        FREEWB_DEBUG("callSettingsMethod: send member={} ok", member);
    }
    sd_bus_message_unref(m);
}

std::string SDBusProxy::callSettingsMethodReplyString(const char *member) const
{
    if (!bus_ || !available_ || !member)
    {
        return {};
    }
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = nullptr;
    const int callR = sd_bus_call_method(bus_, FREEWUBI_SETTINGS_SERVICENAME, FREEWUBI_SETTINGS_OBJECTPATH,
                                         FREEWUBI_SETTINGS_INTERFACE, member, &err, &reply, "");
    if (callR < 0)
    {
        sd_bus_error_free(&err);
        return {};
    }
    const char *value = nullptr;
    const int readR = sd_bus_message_read(reply, "s", &value);
    std::string result = (readR < 0 || !value) ? std::string() : std::string(value);
    sd_bus_message_unref(reply);
    sd_bus_error_free(&err);
    return result;
}

bool SDBusProxy::registerPanelMatches()
{
    if (!bus_)
    {
        return false;
    }
    const int r = sd_bus_match_signal(bus_, &panelSignalSlot_, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH,
                                      FREEWUBI_PANEL_INTERFACE, nullptr, &SDBusProxy::handlePanelSignal, this);
    return r >= 0;
}

void SDBusProxy::clearSlots()
{
    if (panelSignalSlot_)
    {
        sd_bus_slot_unref(panelSignalSlot_);
        panelSignalSlot_ = nullptr;
    }
}

} // namespace freewb::ipc
