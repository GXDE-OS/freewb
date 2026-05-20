#include "module.h"

#include <fcitx-utils/event.h>

#include "types.h"
#include "sdbus_proxy.h"

FreewbIMModule::FreewbIMModule(fcitx::Instance *instance) : instance_(instance)
{
    freewb_ = std::make_unique<freewb::Freewb>(dynamic_cast<freewb::ipc::IDBus *>(new freewb::ipc::SDBusProxy(instance->eventLoop().nativeHandle())),
                                               [this](const std::string &text) { commitString(text); });
}

FreewbIMModule::~FreewbIMModule()
{
}

void FreewbIMModule::keyEvent(const fcitx::InputMethodEntry &entry, fcitx::KeyEvent &keyEvent)
{
    FCITX_UNUSED(entry);
    if (keyEvent.isRelease())
    {
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
    fcitx::InputContext *inputContext = instance_->lastFocusedInputContext();
    if (inputContext == nullptr)
    {
        freewb_->dbusProxy()->callPanelUpdateSpotRect(spotRect);
        return;
    }

    fcitx::Rect rect = inputContext->cursorRect();
    spotRect.x = rect.left();
    spotRect.y = rect.top();
    spotRect.w = rect.width();
    spotRect.h = rect.height();
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
