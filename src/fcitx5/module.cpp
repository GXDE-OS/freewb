#include "module.h"

#include <fcitx-utils/event.h>
#include "types.h"

FreewbIMModule::FreewbIMModule(fcitx::Instance *instance) : instance_(instance), freewb_(std::make_unique<freewb::Freewb>(instance->eventLoop().nativeHandle()))
{
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

    const fcitx::Key &key = keyEvent.key();
    const uint32_t stateBits = key.states().toInteger();
    freewb_->processKey(static_cast<FreewbKeySym>(key.sym()), static_cast<FreewbKeyState>(stateBits));
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
        freewb_->sdbusProxy()->emitUpdateSpotRect(spotRect);
        return;
    }


    fcitx::Rect rect = inputContext->cursorRect();
    spotRect.x = rect.left();
    spotRect.y = rect.top();
    spotRect.w = rect.width();
    spotRect.h = rect.height();
    freewb_->sdbusProxy()->emitUpdateSpotRect(spotRect);
}

FCITX_ADDON_FACTORY(FreewbIMModuleFactory)
