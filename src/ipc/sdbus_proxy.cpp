#include "sdbus_proxy.h"

#include <cerrno>
#include <cstdarg>
#include <string>

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
    std::vector<char *> strvAttrs;

    int r = sd_bus_message_append_strv(m, makeStrv(payload.labels, strvLabels));
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_strv(m, makeStrv(payload.texts, strvTexts));
    if (r < 0)
    {
        return r;
    }
    r = sd_bus_message_append_strv(m, makeStrv(payload.attrs, strvAttrs));
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
    emitImeSignal("UpdateProperty", "s", "/Fcitx/im:Freewb");
}

void SDBusProxy::emitHideToolbar()
{
    emitImeSignal("UpdateProperty", "s", "/Fcitx/im:us");
}

void SDBusProxy::emitSetSpotRect(const SpotRectPayload &payload)
{
    FREEWB_DEBUG("emitSetSpotRect x={} y={} w={} h={}", payload.x, payload.y, payload.w, payload.h);
    sendPanelMethod("SetSpotRect", "iiii", payload.x, payload.y, payload.w, payload.h);
}

void SDBusProxy::emitSetCandidate(const CandidatePayload &payload)
{
    if (!bus_ || !available_)
    {
        FREEWB_ERROR("emitSetCandidate skipped: bus={} available_={}", static_cast<const void *>(bus_), available_);
        return;
    }
    FREEWB_DEBUG(
        "emitSetCandidate: labels={} texts={} attrs={} hasPrev={} hasNext={} cursor={} layout={}",
        payload.labels.size(),
        payload.texts.size(),
        payload.attrs.size(),
        payload.hasPrev,
        payload.hasNext,
        payload.cursor,
        static_cast<int>(payload.layout));

    sd_bus_message *m = nullptr;
    const int newCallR = sd_bus_message_new_method_call(bus_, &m, FREEWUBI_PANEL_SERVICENAME, FREEWUBI_PANEL_OBJECTPATH, FREEWUBI_PANEL_INTERFACE, "SetLookupTable");
    if (newCallR < 0)
    {
        FREEWB_ERROR("emitSetCandidate: sd_bus_message_new_method_call(SetLookupTable) failed: {} ({})", newCallR, strerror(-newCallR));
        return;
    }

    const int appendR = appendSetCandidateBody(m, payload);
    if (appendR < 0)
    {
        FREEWB_ERROR("emitSetCandidate: append candidate body failed: {} ({})", appendR, strerror(-appendR));
        sd_bus_message_unref(m);
        return;
    }
    const int sendR = sd_bus_send(bus_, m, nullptr);
    sd_bus_message_unref(m);
    if (sendR < 0)
    {
        FREEWB_ERROR("emitSetCandidate: sd_bus_send(SetLookupTable) failed: {} ({})", sendR, strerror(-sendR));
        return;
    }
    const bool hasLookup = !payload.labels.empty() || !payload.texts.empty();
    emitImeSignal("ShowLookupTable", "b", hasLookup);
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
