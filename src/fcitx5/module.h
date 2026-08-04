#ifndef _MODULE_H_
#define _MODULE_H_

#include <iostream>
#include <map>

#include <fcitx/action.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

#include "freewb.h"
#if defined(__HAS_WAYLAND__)
#include "ukuiwaylandhelper.h"
#endif

#if __FCITX5_MAJOR_VERSION__ >= 5
class FreewbIMModule final : public fcitx::InputMethodEngineV3
#else
class FreewbIMModule final : public fcitx::InputMethodEngine
#endif
{
public:
    FreewbIMModule(fcitx::Instance *instance);
    ~FreewbIMModule();

    void activate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void deactivate(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void keyEvent(const fcitx::InputMethodEntry &entry, fcitx::KeyEvent &keyEvent);
    void reloadConfig();
    void reset(const fcitx::InputMethodEntry &entry, fcitx::InputContextEvent &event);
    void save();

private:
    void updateCursorPosition();
    void commitString(const std::string &text) const;
    void initActions();
    void registerTrayMenu(fcitx::InputContext &inputContext);

private:
    fcitx::Instance *instance_;
    fcitx::InputContext *activeInputContext_ = nullptr;
    std::unique_ptr<freewb::Freewb> freewb_;
    std::map<std::string, fcitx::SimpleAction> actions_;
#if defined(__HAS_WAYLAND__)
    std::unique_ptr<freewb::UkuiWaylandHelper> ukuiWaylandHelper_ = nullptr;
#endif
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
