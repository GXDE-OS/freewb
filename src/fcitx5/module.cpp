#include "module.h"

#include <fcitx-utils/event.h>

#include "sdbus_proxy.h"
#include "types.h"

FreewbIMModule::FreewbIMModule(fcitx::Instance *instance) : instance_(instance)
{
    freewb_ = std::make_unique<freewb::Freewb>(dynamic_cast<freewb::ipc::IDBus *>(
                                                   new freewb::ipc::SDBusProxy(instance->eventLoop().nativeHandle())),
                                               [this](const std::string &text) { commitString(text); });
#if defined(__HAS_WAYLAND__)
    ukuiWaylandHelper_ = std::make_unique<freewb::UkuiWaylandHelper>();
#endif
}

FreewbIMModule::~FreewbIMModule()
{
}

void FreewbIMModule::keyEvent(const fcitx::InputMethodEntry &entry, fcitx::KeyEvent &keyEvent)
{
    FCITX_UNUSED(entry);
    if (keyEvent.isRelease())
    {
        const auto keysym = static_cast<FreewbKeySym>(keyEvent.key().sym());
        const auto state = static_cast<FreewbKeyState>(keyEvent.key().states().toInteger());
        bool processed = freewb_->processKeyRelease(keysym, state);
        if (processed)
        {
            keyEvent.filterAndAccept();
            freewb_->updateCandidateAndPreeditToUI();
        }
        return;
    }

    updateCursorPosition();

    bool processed = freewb_->processKey(static_cast<FreewbKeySym>(keyEvent.key().sym()),
                                         static_cast<FreewbKeyState>(keyEvent.key().states().toInteger()));
    if (processed)
    {
        keyEvent.filterAndAccept();
    }

    freewb_->updateCandidateAndPreeditToUI();
}

void FreewbIMModule::activate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
    freewb_->activate();
}

void FreewbIMModule::deactivate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
    freewb_->deactivate();
}

void FreewbIMModule::reloadConfig()
{
}

void FreewbIMModule::reset(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
    freewb_->reset();
}

void FreewbIMModule::doReset(fcitx::InputContext *inputContext)
{
    FCITX_UNUSED(inputContext);
}

void FreewbIMModule::save()
{
}

void FreewbIMModule::updateCursorPosition()
{
    ::freewb::SpotRectPayload spotRect = {0, 0, 0, 0};
    std::array<int32_t, 2> focusWindowPositionFromWlcom = {0, 0};
    double maxScreenScaleFactorFromWlcom = 1.0;
    double scaleFactorFromFcitx = 1.0;
    std::string appDisplay;

    fcitx::InputContext *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        freewb_->dbusProxy()->callPanelUpdateSpotRect(spotRect);
        return;
    }

    // get focus window position and scale factor from wayland wlcom
#if defined(__HAS_WAYLAND__)
    if (ukuiWaylandHelper_ != nullptr)
    {
        focusWindowPositionFromWlcom = ukuiWaylandHelper_->focusWindowPosition();
        maxScreenScaleFactorFromWlcom = ukuiWaylandHelper_->maxScreenScaleFactor();
    }
#endif

    // get cursor position from input method framework
    fcitx::Rect rectFromFcitx = inputContext->cursorRect();
    scaleFactorFromFcitx = inputContext->scaleFactor();
    const bool appIsWaylandDisplay = (strncmp(inputContext->display().c_str(), "wayland", 7) == 0);

    // calculate cursor position
    if (appIsWaylandDisplay)
    {
        // app is running on wayland.
        spotRect.x = focusWindowPositionFromWlcom[0] + rectFromFcitx.left() / scaleFactorFromFcitx;
        spotRect.y = focusWindowPositionFromWlcom[1] + rectFromFcitx.top() / scaleFactorFromFcitx;
        spotRect.w = rectFromFcitx.width() / scaleFactorFromFcitx;
        spotRect.h = rectFromFcitx.height() / scaleFactorFromFcitx;
    }
    else if (!appIsWaylandDisplay)
    {
        // app is running on x11.
        spotRect.x = rectFromFcitx.left();
        spotRect.y = rectFromFcitx.top();
        spotRect.w = rectFromFcitx.width();
        spotRect.h = rectFromFcitx.height();
    }

    freewb_->dbusProxy()->callPanelUpdateSpotRect(spotRect);
}

void FreewbIMModule::commitString(const std::string &text) const
{
    fcitx::InputContext *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        return;
    }

    FREEWB_DEBUG("will commit string: {}", text);
    inputContext->commitString(text.c_str());
}

FCITX_ADDON_FACTORY(FreewbIMModuleFactory)
