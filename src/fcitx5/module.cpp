#include "module.h"

#include <algorithm>
#include <cmath>

#include <fcitx-utils/event.h>
#include <fcitx/action.h>
#include <fcitx/userinterfacemanager.h>

#include "config.h"
#include "log.h"
#include "sdbus_proxy.h"
#include "types.h"

FreewbIMModule::FreewbIMModule(fcitx::Instance *instance) : instance_(instance)
{
    bindtextdomain(FREEWB_TEXT_DOMAIN, FREEWB_INSTALL_LOCALEDIR);
    bind_textdomain_codeset(FREEWB_TEXT_DOMAIN, "UTF-8");

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
    bool processed = false;
    const auto keysym = static_cast<FreewbKeySym>(keyEvent.rawKey().sym());
    const auto state = static_cast<FreewbKeyState>(keyEvent.rawKey().states().toInteger());

    if (keyEvent.isRelease())
    {
        processed = freewb_->processKeyRelease(keysym, state);
    }
    else
    {
        processed = freewb_->processKeyPress(keysym, state);
    }

    if (processed)
    {
        keyEvent.filterAndAccept();
    }

    updateCursorPosition();
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
    double compositorScale = 1.0;

    fcitx::InputContext *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        freewb_->dbusProxy()->callPanelUpdateSpotRect(spotRect);
        return;
    }

    // 从 UKUI 合成器获取焦点窗口坐标与全局缩放（XWayland 坐标空间使用 max scale）
#if defined(__HAS_WAYLAND__)
    if (ukuiWaylandHelper_ != nullptr)
    {
        focusWindowPositionFromWlcom = ukuiWaylandHelper_->focusWindowPosition();
        compositorScale = ukuiWaylandHelper_->maxScreenScaleFactor();
    }
#endif

    // get cursor position from input method framework
    fcitx::Rect rectFromFcitx = inputContext->cursorRect();
#if (__FCITX5_MAJOR_VERSION__ >= 5)
    scaleFactorFromFcitx = inputContext->scaleFactor();
#endif
    const bool appIsWaylandDisplay = (strncmp(inputContext->display().c_str(), "wayland", 7) == 0);

    if (appIsWaylandDisplay)
    {
        spotRect.x = focusWindowPositionFromWlcom[0] + rectFromFcitx.left() / scaleFactorFromFcitx;
        spotRect.y = focusWindowPositionFromWlcom[1] + rectFromFcitx.top() / scaleFactorFromFcitx;
        spotRect.w = rectFromFcitx.width() / scaleFactorFromFcitx;
        spotRect.h = rectFromFcitx.height() / scaleFactorFromFcitx;
        FREEWB_DEBUG("{} is running on wayland, will update spotRect: x={} y={} w={} h={}", inputContext->display().c_str(),
                     spotRect.x, spotRect.y, spotRect.w, spotRect.h);
    }
    else
    {
        const int rawX = rectFromFcitx.left();
        const int rawY = rectFromFcitx.top();
        const int rawW = rectFromFcitx.width();
        const int rawH = rectFromFcitx.height();
        spotRect.x = static_cast<int>(std::lround(rawX / compositorScale));
        spotRect.y = static_cast<int>(std::lround(rawY / compositorScale));
        spotRect.w = std::max(1, static_cast<int>(std::lround(rawW / compositorScale)));
        spotRect.h = std::max(1, static_cast<int>(std::lround(rawH / compositorScale)));
        FREEWB_DEBUG("{} is running on x11, cursorRect=({},{},{},{}) scaleFactor={} compositorScale={} spotRect=({},{},{},{})",
                     inputContext->display().c_str(), rawX, rawY, rawW, rawH, scaleFactorFromFcitx, compositorScale, spotRect.x,
                     spotRect.y, spotRect.w, spotRect.h);
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
    actions_["about"].setShortText(dgettext(FREEWB_TEXT_DOMAIN, "About"));
    actions_["about"].connect<fcitx::SimpleAction::Activated>(
        [this](fcitx::InputContext *ic)
        {
            FCITX_UNUSED(ic);
            freewb_->dbusProxy()->callShowVersionInfoMethod();
        });

    actions_["settings"].setShortText(dgettext(FREEWB_TEXT_DOMAIN, "Settings"));
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
