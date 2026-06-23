#include "module.h"

#include <fcitx-utils/event.h>
#include <fcitx/action.h>
#include <fcitx/userinterfacemanager.h>

#include "config.h"
#include "log.h"
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

    initActions();
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
    if (auto *inputContext = event.inputContext())
    {
        registerTrayMenu(*inputContext);
    }
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

void FreewbIMModule::save()
{
}

void FreewbIMModule::updateCursorPosition()
{
    ::freewb::SpotRectPayload spotRect = {0, 0, 0, 0};
    std::array<int32_t, 2> focusWindowPositionFromWlcom = {0, 0};
    double scaleFactorFromFcitx = 1.0;

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
    }
#endif

    // get cursor position from input method framework
    fcitx::Rect rectFromFcitx = inputContext->cursorRect();
#if (__FCITX5_MAJOR_VERSION__ >= 5)
    scaleFactorFromFcitx = inputContext->scaleFactor();
#endif
    const bool appIsWaylandDisplay = (strncmp(inputContext->display().c_str(), "wayland", 7) == 0);

    // calculate cursor position
    if (appIsWaylandDisplay)
    {
        // app is running on wayland.
        spotRect.x = focusWindowPositionFromWlcom[0] + rectFromFcitx.left() / scaleFactorFromFcitx;
        spotRect.y = focusWindowPositionFromWlcom[1] + rectFromFcitx.top() / scaleFactorFromFcitx;
        spotRect.w = rectFromFcitx.width() / scaleFactorFromFcitx;
        spotRect.h = rectFromFcitx.height() / scaleFactorFromFcitx;
        FREEWB_DEBUG("{} is running on wayland, will update spotRect: x={} y={} w={} h={}", inputContext->display().c_str(),
                     spotRect.x, spotRect.y, spotRect.w, spotRect.h);
    }
    else if (!appIsWaylandDisplay)
    {
        // app is running on x11.
        spotRect.x = rectFromFcitx.left();
        spotRect.y = rectFromFcitx.top();
        spotRect.w = rectFromFcitx.width();
        spotRect.h = rectFromFcitx.height();
        FREEWB_DEBUG("{} is running on x11, will update spotRect: x={} y={} w={} h={}", inputContext->display().c_str(),
                     spotRect.x, spotRect.y, spotRect.w, spotRect.h);
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

void FreewbIMModule::initActions()
{
    actions_["about"].setShortText(_("About"));
    actions_["about"].connect<fcitx::SimpleAction::Activated>(
        [this](fcitx::InputContext *ic)
        {
            FCITX_UNUSED(ic);
            freewb_->dbusProxy()->callShowVersionInfoMethod();
        });

    actions_["settings"].setShortText(_("Settings"));
    actions_["settings"].connect<fcitx::SimpleAction::Activated>(
        [this](fcitx::InputContext *ic)
        {
            FCITX_UNUSED(ic);
            freewb_->dbusProxy()->callOpenUiSettingMethod();
        });
}

void FreewbIMModule::registerTrayMenu(fcitx::InputContext &inputContext)
{
    auto &statusArea = inputContext.statusArea();
    for (auto &action : actions_)
    {
        if (instance_->userInterfaceManager().lookupAction(action.first) == nullptr)
        {
            bool isRegister = instance_->userInterfaceManager().registerAction(action.first, &action.second);
            FREEWB_DEBUG("will register action: {} return value: {}", action.first, isRegister);
        }

        FREEWB_DEBUG("will add action: {} to status area", action.first);
        statusArea.addAction(fcitx::StatusGroup::InputMethod, &action.second);
    }
    inputContext.updateUserInterface(fcitx::UserInterfaceComponent::StatusArea);
}

FCITX_ADDON_FACTORY(FreewbIMModuleFactory)
