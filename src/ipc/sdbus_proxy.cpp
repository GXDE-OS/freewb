#include "sdbus_proxy.h"

#include <cerrno>
#include <cstdarg>
#include <string_view>
#include <unordered_map>

#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

#include "ipc.h"

namespace freewb::ipc
{

namespace
{

template <typename F>
static void callIf(F &&fn)
{
    if (fn)
    {
        std::forward<F>(fn)();
    }
}

using PanelSignalHandler = void (*)(sd_bus_message *, DBusCallbacks &);

static const std::unordered_map<std::string_view, PanelSignalHandler> &panelSignalHandlers()
{
    static const std::unordered_map<std::string_view, PanelSignalHandler> kHandlers = {
        {"LookupTablePageUp",
         [](sd_bus_message *msg, DBusCallbacks &c)
         {
             (void)msg;
             callIf(c.onPageUp);
         }},
        {"LookupTablePageDown",
         [](sd_bus_message *msg, DBusCallbacks &c)
         {
             (void)msg;
             callIf(c.onPageDown);
         }},
        {"ReloadConfig",
         [](sd_bus_message *msg, DBusCallbacks &c)
         {
             (void)msg;
             callIf(c.onReloadConfig);
         }},
        {"SelectCandidate",
         [](sd_bus_message *msg, DBusCallbacks &c)
         {
             int32_t index = 0;
             if (sd_bus_message_read(msg, "i", &index) >= 0 && c.onSelectCandidate)
             {
                 c.onSelectCandidate(index);
             }
         }},
    };
    return kHandlers;
}

/** NULL-terminated strv for sd_bus; pointers only valid while @p strings and @p scratch are alive. */
static char **makeStrv(const std::vector<std::string> &strings, std::vector<char *> &scratch)
{
    scratch.clear();
    scratch.reserve(strings.size() + 1U);
    for (const auto &s : strings)
    {
        scratch.push_back(const_cast<char *>(s.c_str()));
    }
    scratch.push_back(nullptr);
    return scratch.data();
}

} // namespace

std::string SDBusProxy::toolbarPayloadToPropertyLine(const ToolbarPropertiesPayload &p)
{
    if (p.uniqueName == "fullwidth" || p.uniqueName == "punc")
    {
        const std::string st = p.active ? "active" : "inactive";
        return "/Fcitx/" + p.uniqueName + ":" + p.shortDescription + ":fcitx-" + p.uniqueName + "-" + st + ":" + p.longDescription;
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
    return "SDBus";
}

bool SDBusProxy::available() const
{
    return available_ && bus_ != nullptr;
}

void SDBusProxy::changeAvailable()
{
    available_ = !available_;
}

bool SDBusProxy::bindDBusCallbacks(const DBusCallbacks &callbacks)
{
    callbacks_ = callbacks;
    if (!bus_)
    {
        return false;
    }
    if (panelSignalSlot_)
    {
        return true;
    }
    if (!registerPanelMatches())
    {
        return false;
    }
    if (!registerInputMethodObject())
    {
        clearSlots();
        return false;
    }
    return true;
}

void SDBusProxy::emitUpdateProperties(const ToolbarPropertiesPayload &payload)
{
    emitRegisterPropertiesSignal({toolbarPayloadToPropertyLine(payload)});
}

void SDBusProxy::emitShowToolbar()
{
    emitImeSignal("UpdateProperty", "s", "/Fcitx/im:Freewb:freewb-test");
}

void SDBusProxy::emitHideToolbar()
{
    emitImeSignal("UpdateProperty", "s", "/Fcitx/im:us:English");
}

void SDBusProxy::sendSetSpotRect(const SpotRectPayload &payload)
{
    sendPanelMethod("SetSpotRect", "iiii", payload.x, payload.y, payload.w, payload.h);
}

void SDBusProxy::sendSetCandidate(const CandidatePayload &payload)
{
    if (!bus_ || !available_)
    {
        return;
    }
    sd_bus_message *m = nullptr;
    if (sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, "SetLookupTable") < 0)
    {
        return;
    }

    const int layout = static_cast<int>(payload.layout);
    std::vector<char *> strvLabels;
    std::vector<char *> strvTexts;
    std::vector<char *> strvAttrs;
    /* One append: signature (as)(as)(as)bbii — sd_bus format "asasasbbii" */
    if (sd_bus_message_append(m, "asasasbbii", makeStrv(payload.labels, strvLabels), makeStrv(payload.texts, strvTexts), makeStrv(payload.attrs, strvAttrs), static_cast<int>(payload.hasPrev), static_cast<int>(payload.hasNext), payload.cursor, layout) < 0)
    {
        sd_bus_message_unref(m);
        return;
    }
    sd_bus_send(bus_, m, nullptr);
    sd_bus_message_unref(m);
}

void SDBusProxy::emitUpdateCandidate(const SpotRectPayload &spotRect, const CandidatePayload &candidate, const CandidatePreeditPayload &preedit, const CandidateAuxPayload &aux)
{
    if (!bus_ || !available_)
    {
        return;
    }
    sendSetSpotRect(spotRect);

    const bool hasLookup = !candidate.labels.empty() || !candidate.texts.empty();
    if (hasLookup)
    {
        sendSetCandidate(candidate);
    }
    emitImeSignal("ShowLookupTable", "b", hasLookup);

    emitUpdatePreeditText(preedit);
    emitUpdatePreeditCaret(preedit.caret);

    emitUpdateAux(aux);
}

void SDBusProxy::emitUpdatePreeditText(const CandidatePreeditPayload &payload)
{
    static const char *const kEmptyAttr = "";
    emitImeSignal("UpdatePreeditText", "ss", payload.text.c_str(), kEmptyAttr);
    emitImeSignal("ShowPreedit", "b", payload.show);
}

void SDBusProxy::emitUpdatePreeditCaret(int caret)
{
    emitImeSignal("UpdatePreeditCaret", "i", caret);
}

void SDBusProxy::emitUpdateAux(const CandidateAuxPayload &payload)
{
    static const char *const kEmptyAttr = "";
    emitImeSignal("UpdateAux", "ss", payload.text.c_str(), kEmptyAttr);
    emitImeSignal("ShowAux", "b", payload.show);
}

void SDBusProxy::emitRegisterPropertiesSignal(const std::vector<std::string> &props)
{
    if (!bus_ || !available_)
    {
        return;
    }
    sd_bus_message *m = nullptr;
    if (sd_bus_message_new_signal(bus_, &m, FREEWUBI_INPUTMETHOD_OBJECTPATH, FREEWUBI_INPUTMETHOD_SERVICENAME, "RegisterProperties") < 0)
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

int SDBusProxy::handlePanelSignal(sd_bus_message *m, void *userdata, sd_bus_error *)
{
    auto *self = static_cast<SDBusProxy *>(userdata);
    if (!self)
    {
        return 0;
    }
    const char *member = sd_bus_message_get_member(m);
    if (!member)
    {
        return 0;
    }

    DBusCallbacks &cb = self->callbacks_;
    const auto &handlers = panelSignalHandlers();
    if (const auto it = handlers.find(member); it != handlers.end())
    {
        it->second(m, cb);
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
    if (sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, member) < 0)
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

void SDBusProxy::emitImeSignal(const char *member, const char *types, ...) const
{
    if (!bus_ || !available_ || !member)
    {
        return;
    }
    sd_bus_message *m = nullptr;
    if (sd_bus_message_new_signal(bus_, &m, FREEWUBI_INPUTMETHOD_OBJECTPATH, FREEWUBI_INPUTMETHOD_SERVICENAME, member) < 0)
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

bool SDBusProxy::registerPanelMatches()
{
    if (!bus_)
    {
        return false;
    }
    const int r = sd_bus_match_signal(bus_, &panelSignalSlot_, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, nullptr, &SDBusProxy::handlePanelSignal, this);
    return r >= 0;
}

bool SDBusProxy::registerInputMethodObject()
{
    if (!bus_)
    {
        return false;
    }
    const int r = sd_bus_request_name(bus_, FREEWUBI_INPUTMETHOD_SERVICENAME, 0);
    return r >= 0 || r == -EALREADY;
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
