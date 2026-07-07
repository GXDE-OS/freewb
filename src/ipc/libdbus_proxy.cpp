#include "libdbus_proxy.h"

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <vector>

#include "ipc.h"
#include "log.h"

namespace
{

bool appendDBusArgs(DBusMessage *msg, const char *types, va_list ap)
{
    if (!types || types[0] == '\0')
    {
        return true;
    }
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);
    for (const char *t = types; *t != '\0'; ++t)
    {
        switch (*t)
        {
        case 's':
        {
            const char *value = va_arg(ap, const char *);
            if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &value))
            {
                return false;
            }
            break;
        }
        case 'i':
        {
            dbus_int32_t value = va_arg(ap, dbus_int32_t);
            if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &value))
            {
                return false;
            }
            break;
        }
        case 'b':
        {
            const int value = va_arg(ap, int);
            dbus_bool_t b = value ? TRUE : FALSE;
            if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_BOOLEAN, &b))
            {
                return false;
            }
            break;
        }
        default:
            return false;
        }
    }
    return true;
}

bool appendStringArray(DBusMessageIter *parent, const std::vector<std::string> &strings)
{
    DBusMessageIter arrayIter;
    if (!dbus_message_iter_open_container(parent, DBUS_TYPE_ARRAY, "s", &arrayIter))
    {
        return false;
    }
    for (const auto &s : strings)
    {
        const char *cstr = s.c_str();
        if (!dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_STRING, &cstr))
        {
            dbus_message_iter_close_container(parent, &arrayIter);
            return false;
        }
    }
    return dbus_message_iter_close_container(parent, &arrayIter);
}

/** Append @p payload as SetLookupTable D-Bus body: as, as, as, b, b, i. */
bool appendSetCandidateBody(DBusMessage *msg, const freewb::CandidatePayload &payload)
{
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    if (!appendStringArray(&args, payload.fullCodes) || !appendStringArray(&args, payload.texts) ||
        !appendStringArray(&args, payload.prompts))
    {
        return false;
    }
    dbus_bool_t hasPrev = payload.hasPrev ? TRUE : FALSE;
    dbus_bool_t hasNext = payload.hasNext ? TRUE : FALSE;
    dbus_int32_t cursor = payload.cursor;
    return dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &hasPrev) &&
           dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &hasNext) &&
           dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &cursor);
}

} // namespace

namespace freewb::ipc
{

std::string LibDbusProxy::toolbarPayloadToPropertyLine(const ::freewb::ToolbarPropertiesPayload &p)
{
    if (p.uniqueName == "fullwidth" || p.uniqueName == "punc")
    {
        const std::string st = p.active ? "active" : "inactive";
        return "/Fcitx/" + p.uniqueName + ":" + p.shortDescription + ":fcitx-" + p.uniqueName + "-" + st + ":" +
               p.longDescription;
    }
    return "/Fcitx/im:" + p.uniqueName + ":" + p.name;
}

LibDbusProxy::LibDbusProxy(void *dbus_connection) : conn_(static_cast<DBusConnection *>(dbus_connection))
{
    FREEWB_DEBUG("LibDbusProxy: dbus_connection={}", static_cast<const void *>(conn_));
}

LibDbusProxy::~LibDbusProxy()
{
    clearMatches();
    closeBus();
}

void LibDbusProxy::closeBus()
{
    if (!conn_)
    {
        return;
    }
    dbus_connection_unref(conn_);
    conn_ = nullptr;
}

const char *LibDbusProxy::name() const
{
    return "ipc-libdbusproxy";
}

bool LibDbusProxy::available() const
{
    return available_ && conn_ != nullptr;
}

void LibDbusProxy::changeAvailable()
{
    available_ = !available_;
}

bool LibDbusProxy::bindDBusSignalCallback(DBusSignalCallback callback)
{
    onDBusSignal_ = std::move(callback);
    if (!conn_)
    {
        return false;
    }
    if (filterAdded_)
    {
        return true;
    }
    return registerPanelMatches();
}

void LibDbusProxy::callPanelUpdateProperties(const ::freewb::ToolbarPropertiesPayload &payload)
{
    sendPanelRegisterProperties({toolbarPayloadToPropertyLine(payload)});
}

void LibDbusProxy::callPanelShowToolbar()
{
    sendPanelMethod("UpdateProperty", "s", "/Fcitx/im:Freewb");
}

void LibDbusProxy::callPanelHideToolbar()
{
    sendPanelMethod("UpdateProperty", "s", "/Fcitx/im:us");
}

void LibDbusProxy::callPanelUpdateSpotRect(const ::freewb::SpotRectPayload &payload)
{
    FREEWB_DEBUG("callPanelUpdateSpotRect x={} y={} w={} h={}", payload.x, payload.y, payload.w, payload.h);
    sendPanelMethod("SetSpotRect", "iiii", payload.x, payload.y, payload.w, payload.h);
}

void LibDbusProxy::callPanelUpdateCandidate(const ::freewb::CandidatePayload &payload)
{
    if (!conn_ || !available_)
    {
        FREEWB_ERROR("callPanelUpdateCandidate skipped: conn={} available_={}", static_cast<const void *>(conn_), available_);
        return;
    }
    FREEWB_DEBUG("callPanelUpdateCandidate: texts={} prompts={} hasPrev={} hasNext={} cursor={}", payload.texts.size(),
                 payload.prompts.size(), payload.hasPrev, payload.hasNext, payload.cursor);

    DBusMessage *msg = dbus_message_new_method_call(FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH,
                                                    FREEWUBI_PANEL_INTERFACE, "SetLookupTable");
    if (!msg)
    {
        FREEWB_ERROR("callPanelUpdateCandidate: new_method_call(SetLookupTable) failed");
        return;
    }
    if (!appendSetCandidateBody(msg, payload))
    {
        FREEWB_ERROR("callPanelUpdateCandidate: append candidate body failed");
        dbus_message_unref(msg);
        return;
    }
    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(conn_, msg, &serial))
    {
        FREEWB_ERROR("callPanelUpdateCandidate: send(SetLookupTable) failed");
        dbus_message_unref(msg);
        return;
    }
    dbus_message_unref(msg);

    const int hasLookup = payload.texts.empty() ? 0 : 1;
    sendPanelMethod("ShowLookupTable", "b", hasLookup);
}

void LibDbusProxy::callPanelUpdatePreeditText(const ::freewb::PreeditPayload &payload)
{
    static const char *const kEmptyAttr = "";
    sendPanelMethod("UpdatePreeditText", "ss", payload.text.c_str(), kEmptyAttr);
}

void LibDbusProxy::callPanelUpdatePreeditCaret(int caret)
{
    sendPanelMethod("UpdatePreeditCaret", "i", caret);
}

void LibDbusProxy::callPanelUpdateAux(const ::freewb::CandidateAuxPayload &payload)
{
    static const char *const kEmptyAttr = "";
    sendPanelMethod("UpdateAux", "ss", payload.text.c_str(), kEmptyAttr);
}

void LibDbusProxy::callAddUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText)
{
    callSettingsMethod("slot_dbus_generate_usr_word", "iss", flg, wordText.c_str(), wordCode.c_str());
}

void LibDbusProxy::callDeleteUsrParseMethod(int flg, const std::string &wordCode, const std::string &wordText)
{
    callSettingsMethod("slot_dbus_delete_usr_word", "iss", flg, wordText.c_str(), wordCode.c_str());
}

void LibDbusProxy::callDictQueryMethod(const std::string &wordText)
{
    callSettingsMethod("slot_dbus_dict_query", "s", wordText.c_str());
}

void LibDbusProxy::callPanelSwitchInputModeMethod(const std::string &inputMode)
{
    sendPanelMethod("SwitchInputMode", "s", inputMode.c_str());
}

void LibDbusProxy::callSwitchVirtualKeyboardModeMethod(int flg)
{
    callSettingsMethod("slot_dbus_switch_vk", "i", flg);
}

void LibDbusProxy::callPanelSwitchCharSetMethod()
{
    sendPanelMethod("SwitchCharSet", "");
}

void LibDbusProxy::callSwitchRecodeProofMethod()
{
    callSettingsMethod("slot_dbus_set_recode_calib_flg", "i", 0);
}

void LibDbusProxy::callSwitchUncommonParseStateMethod(const std::string &wordText, int flg)
{
    callSettingsMethod("slot_dbus_word_freq_switch_ok", "is", flg, wordText.c_str());
}

void LibDbusProxy::callPanelSwitchChttransMethod()
{
    sendPanelMethod("SwitchSimpOrTrad", "");
}

void LibDbusProxy::callOpenUiSettingMethod()
{
    callSettingsMethod("slot_dbus_open_ui_setting", "");
}

void LibDbusProxy::callShowVersionInfoMethod()
{
    callSettingsMethod("slot_dbus_show_version_info", "");
}

void LibDbusProxy::callOpenProfessionalSettingMethod()
{
    callSettingsMethod("slot_dbus_open_advanced_setting", "");
}

void LibDbusProxy::callModQuickTableMethod()
{
    callSettingsMethod("slot_dbus_edit_quick_table", "");
}

void LibDbusProxy::callModUserTableMethod()
{
    callSettingsMethod("slot_dbus_edit_usr_table", "");
}

void LibDbusProxy::callModWubiTableMethod()
{
    callSettingsMethod("slot_dbus_edit_wubi_table", "");
}

void LibDbusProxy::callModPinyinTableMethod()
{
    callSettingsMethod("slot_dbus_edit_pinyin_table", "");
}

void LibDbusProxy::callOpenConfDirMethod()
{
    callSettingsMethod("slot_dbus_open_freewb_dir", "");
}

void LibDbusProxy::callCloseVkBoardMethod()
{
    callSettingsMethod("slot_dbus_close_vk", "");
}

void LibDbusProxy::callSwitchTableMethod()
{
    callSettingsMethod("slot_dbus_switch_lexicon", "");
}

void LibDbusProxy::callSwitchToolbarHideFlgMethod()
{
    callSettingsMethod("slot_dbus_switch_toolbar_hide_flg", "");
}

void LibDbusProxy::callSwitchMarkAutoPairsFlgMethod()
{
    callSettingsMethod("slot_dbus_switch_mark_auto_pairs_flg", "");
}

void LibDbusProxy::callPanelSwitchCharWidthMethod()
{
    sendPanelMethod("SwitchCharWidth", "");
}

void LibDbusProxy::callPanelSwitchPunctuationModeMethod()
{
    sendPanelMethod("SwitchPunctuationMode", "");
}

std::string LibDbusProxy::callGetClipboardMethod()
{
    return callSettingsMethodReplyString("slot_dbus_get_clipboard_text");
}

void LibDbusProxy::callPanelToggleCapsStateMethod()
{
    callSettingsMethod("slot_dbus_switch_caps_state", "");
}

DBusHandlerResult LibDbusProxy::handlePanelSignal(DBusConnection *conn, DBusMessage *msg, void *userdata)
{
    auto *self = static_cast<LibDbusProxy *>(userdata);
    if (!self)
    {
        FREEWB_WARN("panel signal: userdata null");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    if (!msg)
    {
        FREEWB_WARN("panel signal: message null");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    const char *iface = dbus_message_get_interface(msg);
    if (!iface || std::strcmp(iface, FREEWUBI_PANEL_INTERFACE) != 0)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    const char *member = dbus_message_get_member(msg);
    if (!member)
    {
        FREEWB_WARN("panel signal: member null");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (self->onDBusSignal_)
    {
        if (std::strcmp(member, "CommitUserWordAdd") == 0)
        {
            const char *code = nullptr;
            const char *text = nullptr;
            DBusMessageIter iter;
            if (dbus_message_iter_init(msg, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
            {
                dbus_message_iter_get_basic(&iter, &code);
                if (dbus_message_iter_next(&iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
                {
                    dbus_message_iter_get_basic(&iter, &text);
                }
            }
            if (code != nullptr && text != nullptr)
            {
                FREEWB_DEBUG("panel signal CommitUserWordAdd code={} text={}", code, text);
                PanelSignalEvent evt;
                evt.member = member;
                evt.str0 = code;
                evt.str1 = text;
                self->onDBusSignal_(evt);
            }
            else
            {
                FREEWB_ERROR("panel signal CommitUserWordAdd: invalid payload");
            }
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        FREEWB_DEBUG("panel signal passthrough: member={}", member);
        int index = 0;
        DBusMessageIter iter;
        if (dbus_message_iter_init(msg, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INT32)
        {
            dbus_int32_t tmp = 0;
            dbus_message_iter_get_basic(&iter, &tmp);
            index = static_cast<int>(tmp);
        }
        PanelSignalEvent evt;
        evt.member = member;
        evt.index = index;
        self->onDBusSignal_(evt);
    }
    else
    {
        FREEWB_WARN("panel signal: {} no callback", member);
    }
    return DBUS_HANDLER_RESULT_HANDLED;
}

void LibDbusProxy::sendPanelMethod(const char *member, const char *types, ...) const
{
    if (!conn_ || !available_ || !member)
    {
        return;
    }
    DBusMessage *msg =
        dbus_message_new_method_call(FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, member);
    if (!msg)
    {
        return;
    }
    if (types && types[0] != '\0')
    {
        va_list ap;
        va_start(ap, types);
        if (!appendDBusArgs(msg, types, ap))
        {
            va_end(ap);
            dbus_message_unref(msg);
            return;
        }
        va_end(ap);
    }
    dbus_uint32_t serial = 0;
    dbus_connection_send(conn_, msg, &serial);
    dbus_message_unref(msg);
}

void LibDbusProxy::sendPanelRegisterProperties(const std::vector<std::string> &props) const
{
    if (!conn_ || !available_)
    {
        return;
    }
    DBusMessage *msg = dbus_message_new_method_call(FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH,
                                                    FREEWUBI_PANEL_INTERFACE, "RegisterProperties");
    if (!msg)
    {
        return;
    }
    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);
    if (!appendStringArray(&args, props))
    {
        dbus_message_unref(msg);
        return;
    }
    dbus_uint32_t serial = 0;
    dbus_connection_send(conn_, msg, &serial);
    dbus_message_unref(msg);
}

void LibDbusProxy::callSettingsMethod(const char *member, const char *types, ...) const
{
    if (!conn_ || !available_ || !member)
    {
        FREEWB_WARN("callSettingsMethod skipped: conn={} available_={} member={}", static_cast<const void *>(conn_), available_,
                    member ? member : "(null)");
        return;
    }

    FREEWB_DEBUG("callSettingsMethod: member={} types={}", member, (types && types[0] != '\0') ? types : "(none)");

    DBusMessage *msg = dbus_message_new_method_call(FREEWUBI_SETTINGS_SERVICENAME, FREEWUBI_SETTINGS_OBJECTPATH,
                                                    FREEWUBI_SETTINGS_INTERFACE, member);
    if (!msg)
    {
        FREEWB_ERROR("callSettingsMethod: new_method_call({}) failed", member);
        return;
    }
    if (types && types[0] != '\0')
    {
        va_list ap;
        va_start(ap, types);
        if (!appendDBusArgs(msg, types, ap))
        {
            va_end(ap);
            FREEWB_ERROR("callSettingsMethod: append member={} types={} failed", member, types);
            dbus_message_unref(msg);
            return;
        }
        va_end(ap);
    }
    dbus_uint32_t serial = 0;
    if (!dbus_connection_send(conn_, msg, &serial))
    {
        FREEWB_ERROR("callSettingsMethod: send member={} failed", member);
    }
    else
    {
        FREEWB_DEBUG("callSettingsMethod: send member={} ok", member);
    }
    dbus_message_unref(msg);
}

std::string LibDbusProxy::callSettingsMethodReplyString(const char *member) const
{
    if (!conn_ || !available_ || !member)
    {
        return {};
    }
    DBusMessage *msg = dbus_message_new_method_call(FREEWUBI_SETTINGS_SERVICENAME, FREEWUBI_SETTINGS_OBJECTPATH,
                                                    FREEWUBI_SETTINGS_INTERFACE, member);
    if (!msg)
    {
        return {};
    }
    DBusPendingCall *pending = nullptr;
    if (!dbus_connection_send_with_reply(conn_, msg, &pending, -1))
    {
        dbus_message_unref(msg);
        return {};
    }
    dbus_connection_flush(conn_);
    dbus_message_unref(msg);
    if (!pending)
    {
        return {};
    }
    dbus_pending_call_block(pending);
    DBusMessage *reply = dbus_pending_call_steal_reply(pending);
    dbus_pending_call_unref(pending);
    if (!reply)
    {
        return {};
    }
    const char *value = nullptr;
    DBusMessageIter iter;
    if (dbus_message_iter_init(reply, &iter) && dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
    {
        dbus_message_iter_get_basic(&iter, &value);
    }
    const std::string result = value ? value : std::string();
    dbus_message_unref(reply);
    return result;
}

bool LibDbusProxy::registerPanelMatches()
{
    if (!conn_)
    {
        return false;
    }
    DBusError err;
    dbus_error_init(&err);
    dbus_bus_add_match(conn_, "type='signal',interface='" FREEWUBI_PANEL_INTERFACE "',path='" FREEWUBI_PANEL_OBJECTPATH "'",
                       &err);
    if (dbus_error_is_set(&err))
    {
        FREEWB_ERROR("LibDbusProxy: add_match failed: {}", err.message);
        dbus_error_free(&err);
        return false;
    }
    dbus_connection_add_filter(conn_, handlePanelSignal, this, nullptr);
    filterAdded_ = true;
    return true;
}

void LibDbusProxy::clearMatches()
{
    if (!conn_ || !filterAdded_)
    {
        return;
    }
    dbus_connection_remove_filter(conn_, handlePanelSignal, this);
    DBusError err;
    dbus_error_init(&err);
    dbus_bus_remove_match(conn_, "type='signal',interface='" FREEWUBI_PANEL_INTERFACE "',path='" FREEWUBI_PANEL_OBJECTPATH "'",
                          &err);
    dbus_error_free(&err);
    filterAdded_ = false;
}

} // namespace freewb::ipc
