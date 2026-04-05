#include "sdbus_proxy.h"

#include <cerrno>
#include <cstdarg>
#include <string_view>
#include <unordered_map>

#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

#include "ipc.h"
#include "log.h"

namespace freewb::ipc
{

namespace
{

/** @p member D-Bus signal name for logs; if @p fn empty, FREEWB_WARN and do not call. */
template <typename F, typename... Args>
static void callIf(const char *member, F &&fn, Args &&... args)
{
    if (fn)
    {
        std::forward<F>(fn)(std::forward<Args>(args)...);
    }
    else
    {
        FREEWB_WARN("panel signal: {} no callback", member);
    }
}

using PanelSignalHandler = void (*)(sd_bus_message *, DBusCallbacks &);

static const std::unordered_map<std::string_view, PanelSignalHandler> &panelSignalHandlers()
{
    static const std::unordered_map<std::string_view, PanelSignalHandler> kHandlers = {
        {"LookupTablePageUp",
         [](sd_bus_message *, DBusCallbacks &c) { callIf("LookupTablePageUp", c.onPageUp); }},
        {"LookupTablePageDown",
         [](sd_bus_message *, DBusCallbacks &c) { callIf("LookupTablePageDown", c.onPageDown); }},
        {"ReloadConfig",
         [](sd_bus_message *, DBusCallbacks &c) { callIf("ReloadConfig", c.onReloadConfig); }},
        {"SelectCandidate",
         [](sd_bus_message *msg, DBusCallbacks &c)
         {
             int32_t index = 0;
             if (sd_bus_message_read(msg, "i", &index) < 0)
             {
                 FREEWB_WARN("panel signal: SelectCandidate read(i) failed");
                 return;
             }
             callIf("SelectCandidate", c.onSelectCandidate, index);
         }},
    };
    return kHandlers;
}

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
    FREEWB_DEBUG("sendSetSpotRect x={} y={} w={} h={}", payload.x, payload.y, payload.w, payload.h);
    sendPanelMethod("SetSpotRect", "iiii", payload.x, payload.y, payload.w, payload.h);
}

void SDBusProxy::sendSetCandidate(const CandidatePayload &payload)
{
    if (!bus_ || !available_)
    {
        FREEWB_ERROR("sendSetCandidate skipped: bus={} available_={}", static_cast<const void *>(bus_), available_);
        return;
    }
    FREEWB_DEBUG(
        "sendSetCandidate: labels={} texts={} attrs={} hasPrev={} hasNext={} cursor={} layout={}",
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
        FREEWB_ERROR("sendSetCandidate: sd_bus_message_new_method_call(SetLookupTable) failed: {} ({})", newCallR, strerror(-newCallR));
        return;
    }

    const int appendR = appendSetCandidateBody(m, payload);
    if (appendR < 0)
    {
        FREEWB_ERROR("sendSetCandidate: append candidate body failed: {} ({})", appendR, strerror(-appendR));
        sd_bus_message_unref(m);
        return;
    }
    const int sendR = sd_bus_send(bus_, m, nullptr);
    if (sendR < 0)
    {
        FREEWB_ERROR("sendSetCandidate: sd_bus_send(SetLookupTable) failed: {} ({})", sendR, strerror(-sendR));
    }
    sd_bus_message_unref(m);
}

void SDBusProxy::emitUpdateCandidate(const SpotRectPayload &spotRect, const CandidatePayload &candidate, const CandidatePreeditPayload &preedit, const CandidateAuxPayload &aux)
{
    FREEWB_DEBUG(
        "emitUpdateCandidate enter: bus={} available_={} spot=({},{} {}x{}) cand labels={} texts={} attrs={} preedit.len={} preedit.show={} aux.show={}",
        static_cast<const void *>(bus_),
        available_,
        spotRect.x,
        spotRect.y,
        spotRect.w,
        spotRect.h,
        candidate.labels.size(),
        candidate.texts.size(),
        candidate.attrs.size(),
        preedit.text.size(),
        preedit.show,
        aux.show);

    if (!bus_ || !available_)
    {
        FREEWB_ERROR("emitUpdateCandidate aborted: no bus or unavailable");
        return;
    }
    sendSetSpotRect(spotRect);

    const bool hasLookup = !candidate.labels.empty() || !candidate.texts.empty();
    FREEWB_DEBUG("emitUpdateCandidate: hasLookup={} (will SetLookupTable if true, then ShowLookupTable)", hasLookup);
    if (hasLookup)
    {
        sendSetCandidate(candidate);
    }
    emitImeSignal("ShowLookupTable", "b", hasLookup);

    emitUpdatePreeditText(preedit);
    emitUpdatePreeditCaret(preedit.caret);

    emitUpdateAux(aux);
    FREEWB_DEBUG("emitUpdateCandidate leave: ShowLookupTable hasLookup={} preedit+aux emitted", hasLookup);
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

    DBusCallbacks &cb = self->callbacks_;
    const auto &handlers = panelSignalHandlers();
    if (const auto it = handlers.find(member); it != handlers.end())
    {
        FREEWB_DEBUG("panel signal dispatch: member={}", member);
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
