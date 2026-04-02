#include "module.h"

FreewbIMModule::FreewbIMModule(fcitx::Instance *instance) : instance_(instance)
{
}

FreewbIMModule::~FreewbIMModule()
{
}

void FreewbIMModule::keyEvent(const fcitx::InputMethodEntry &entry, fcitx::KeyEvent &keyEvent)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(keyEvent);
}

void FreewbIMModule::activate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
}

void FreewbIMModule::deactivate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
}

void FreewbIMModule::reloadConfig()
{
}

void FreewbIMModule::reset(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event)
{
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
}

void FreewbIMModule::doReset(fcitx::InputContext *inputContext)
{
    FCITX_UNUSED(inputContext);
}

void FreewbIMModule::save()
{
}

FCITX_ADDON_FACTORY(FreewbIMModuleFactory)
