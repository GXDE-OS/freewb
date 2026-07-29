#ifndef FCITX4_MODULE_H
#define FCITX4_MODULE_H

#include <cstring>
#include <string>

#include <dbus/dbus.h>
#include <fcitx/context.h>
#include <fcitx/hook.h>
#include <fcitx/ime.h>
#include <fcitx/instance.h>
#include <fcitx/keys.h>
#include <fcitx/module.h>
#include <fcitx/module/dbus/fcitx-dbus.h>
#include <fcitx/ui.h>

#include "config.h"
#include "freewb.h"
#include "idbus.h"
#include "libdbus_proxy.h"
#include "types.h"

typedef struct
{
    freewb::Freewb *freewb_;
    FcitxInstance *fcitxInstance_;
} freewb_fcitx4_imclass;

static void *FreewbIMCreate(FcitxInstance *instance);
static void FreewbIMDestroy(void *arg);
static boolean FreewbIMInit(void *arg);
static void FreewbIMReset(void *arg);
static INPUT_RETURN_VALUE FreewbIMDoInput(void *arg, FcitxKeySym sym, unsigned int state);
static INPUT_RETURN_VALUE FreewbIMDoReleaseInput(void *arg, FcitxKeySym sym, unsigned int state);
static void FreewbIMOnChanged(void *arg);

static void updateCursorPosition(freewb_fcitx4_imclass *imclass);
static void detachFcitxGlobalCharWidthPunc(FcitxInstance *instance, boolean detach);

static void registerTrayMenu(FcitxInstance *instance, freewb_fcitx4_imclass *imclass);
static void freewbSettingsStatusToggle(void *arg);
static void freewbAboutStatusToggle(void *arg);
static boolean freewbStatusGetInactive(void *arg);
static void setFreewbStatusVisible(FcitxInstance *instance, boolean visible);

#ifdef __cplusplus
extern "C"
{
#endif
    FCITX_DEFINE_PLUGIN(fcitx_freewb, ime, FcitxIMClass) = {FreewbIMCreate, FreewbIMDestroy};
#ifdef __cplusplus
}
#endif

static boolean FreewbIMInit(void *arg)
{
    return true;
}

static void FreewbIMReset(void *arg)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return;
    }

    imclass->freewb_->reset();
}

static INPUT_RETURN_VALUE FreewbIMDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return IRV_TO_PROCESS;
    }

    const bool processed = imclass->freewb_->processKeyPress(static_cast<FreewbKeySym>(sym), static_cast<FreewbKeyState>(state));

    imclass->freewb_->updateCandidateAndPreeditToUI();
    updateCursorPosition(imclass);

    return processed ? IRV_DO_NOTHING : IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE FreewbIMDoReleaseInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return IRV_TO_PROCESS;
    }

    const bool processed =
        imclass->freewb_->processKeyRelease(static_cast<FreewbKeySym>(sym), static_cast<FreewbKeyState>(state));
    if (processed)
    {
        imclass->freewb_->updateCandidateAndPreeditToUI();
        return IRV_DO_NOTHING;
    }
    return IRV_TO_PROCESS;
}

static void FreewbIMOnChanged(void *arg)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return;
    }

    FcitxIM *im = FcitxInstanceGetCurrentIM(imclass->fcitxInstance_);
    if (im == nullptr)
    {
        return;
    }

    const char *im_name = im->uniqueName;
    if (im_name == nullptr)
    {
        return;
    }

    if (strncmp(im_name, "freewb", sizeof("freewb")) == 0)
    {
        FcitxLog(INFO, "will activate freewb and show ui.");
        FREEWB_DEBUG("will activate freewb and show ui.");
        detachFcitxGlobalCharWidthPunc(imclass->fcitxInstance_, true);
        setFreewbStatusVisible(imclass->fcitxInstance_, true);
        imclass->freewb_->activate();
    }
    else
    {
        FcitxLog(INFO, "will deactivate freewb and hide ui.");
        FREEWB_DEBUG("will deactivate freewb and hide ui.");
        detachFcitxGlobalCharWidthPunc(imclass->fcitxInstance_, false);
        setFreewbStatusVisible(imclass->fcitxInstance_, false);
        imclass->freewb_->deactivate();
    }
}

void *FreewbIMCreate(FcitxInstance *instance)
{
    if (instance == nullptr)
    {
        FREEWB_ERROR("instance is nullptr,will not create freewb instance.");
        return nullptr;
    }

    bindtextdomain(FREEWB_TEXT_DOMAIN, FREEWB_INSTALL_LOCALEDIR);
    bind_textdomain_codeset(FREEWB_TEXT_DOMAIN, "UTF-8");

    DBusConnection *dbusConnection = FcitxDBusGetConnection(instance);
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(fcitx_utils_malloc0(sizeof(freewb_fcitx4_imclass)));
    imclass->freewb_ = nullptr;
    imclass->fcitxInstance_ = instance;

    auto *idbus = new ::freewb::ipc::LibDbusProxy(dbusConnection);
    imclass->freewb_ = new freewb::Freewb(idbus,
                                          [imclass](const std::string &text)
                                          {
                                              FcitxInputContext *ic = FcitxInstanceGetCurrentIC(imclass->fcitxInstance_);
                                              if (ic != nullptr)
                                              {
                                                  FcitxInstanceCommitString(imclass->fcitxInstance_, ic, text.c_str());
                                              }
                                          });

    FcitxIMEventHook imhook = {FreewbIMOnChanged, imclass};
    FcitxInstanceRegisterIMChangedHook(instance, imhook);

    FcitxIMIFace iface;
    memset(&iface, 0, sizeof(FcitxIMIFace));
    iface.Init = FreewbIMInit;
    iface.ResetIM = FreewbIMReset;
    iface.DoInput = FreewbIMDoInput;
    iface.DoReleaseInput = FreewbIMDoReleaseInput;

    FcitxInstanceRegisterIMv2(instance, imclass, "freewb", "freewb", "freewb", iface, 10, "zh_CN");

    registerTrayMenu(instance, imclass);

    return imclass;
}

void FreewbIMDestroy(void *arg)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr)
    {
        return;
    }

    if (imclass->freewb_ != nullptr)
    {
        delete imclass->freewb_;
        imclass->freewb_ = nullptr;
    }

    fcitx_utils_free(imclass);
}

static void updateCursorPosition(freewb_fcitx4_imclass *imclass)
{
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return;
    }

    ::freewb::SpotRectPayload spotRect = {0, 0, 0, 0};
    FcitxInputContext *ic = FcitxInstanceGetCurrentIC(imclass->fcitxInstance_);
    if (ic == nullptr)
    {
        return;
    }

    FcitxInstanceGetWindowRect(imclass->fcitxInstance_, ic, &spotRect.x, &spotRect.y, &spotRect.w, &spotRect.h);
    imclass->freewb_->dbusProxy()->callPanelUpdateSpotRect(spotRect);
}

static void detachFcitxGlobalCharWidthPunc(FcitxInstance *instance, boolean detach)
{
    FcitxInstanceSetContext(instance, CONTEXT_DISABLE_FULLWIDTH, &detach);
    FcitxInstanceSetContext(instance, CONTEXT_DISABLE_PUNC, &detach);
    if (detach)
    {
        FcitxUISetStatusVisable(instance, "punc", false);
    }
    else
    {
        FcitxUISetStatusVisable(instance, "punc", true);
    }
}

static boolean freewbStatusGetInactive(void *arg)
{
    FCITX_UNUSED(arg);
    return false;
}

static void freewbSettingsStatusToggle(void *arg)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return;
    }

    freewb::ipc::IDBus *dbus = imclass->freewb_->dbusProxy();
    if (dbus == nullptr)
    {
        return;
    }

    FREEWB_DEBUG("status menu: open settings");
    dbus->callOpenUiSettingMethod();
}

static void freewbAboutStatusToggle(void *arg)
{
    freewb_fcitx4_imclass *imclass = static_cast<freewb_fcitx4_imclass *>(arg);
    if (imclass == nullptr || imclass->freewb_ == nullptr)
    {
        return;
    }

    freewb::ipc::IDBus *dbus = imclass->freewb_->dbusProxy();
    if (dbus == nullptr)
    {
        return;
    }

    FREEWB_DEBUG("status menu: show about");
    dbus->callShowVersionInfoMethod();
}

static void setFreewbStatusVisible(FcitxInstance *instance, boolean visible)
{
    FcitxUISetStatusVisable(instance, "freewb-settings", visible);
    FcitxUISetStatusVisable(instance, "freewb-about", visible);
}

static void registerTrayMenu(FcitxInstance *instance, freewb_fcitx4_imclass *imclass)
{
    FcitxUIRegisterStatus(instance, imclass, "freewb-settings", dgettext(FREEWB_TEXT_DOMAIN, "Settings"),
                          dgettext(FREEWB_TEXT_DOMAIN, "Open input method settings"), freewbSettingsStatusToggle,
                          freewbStatusGetInactive);
    FcitxUIRegisterStatus(instance, imclass, "freewb-about", dgettext(FREEWB_TEXT_DOMAIN, "About"),
                          dgettext(FREEWB_TEXT_DOMAIN, "Show version information"), freewbAboutStatusToggle,
                          freewbStatusGetInactive);
    setFreewbStatusVisible(instance, false);
}

#endif
