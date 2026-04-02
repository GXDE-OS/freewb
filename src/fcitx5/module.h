#ifndef _MODULE_H_
#define _MODULE_H_

#include <iostream>

#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

class FreewbIMModule final : public fcitx::InputMethodEngineV3
{
public:
    FreewbIMModule(fcitx::Instance *instance);
    ~FreewbIMModule();

    void activate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void deactivate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void keyEvent(const fcitx::InputMethodEntry &entry, fcitx::KeyEvent &keyEvent);
    void reloadConfig();
    void reset(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void doReset(fcitx::InputContext *inputContext);
    void save();

private:
    fcitx::Instance *instance_;
};

class FreewbIMModuleFactory : public fcitx::AddonFactory
{
public:
    fcitx::AddonInstance *create(fcitx::AddonManager *manager)
    {
        FCITX_INFO() << "FreewbIMModuleFactory ...";
        return new FreewbIMModule(manager->instance());
    }
};

#endif