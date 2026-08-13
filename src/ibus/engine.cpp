#include "engine.h"

#include <string>

#include <dbus/dbus.h>
#include <glib.h>

#include "config.h"
#include "dbus_gmain.h"
#include "freewb.h"
#include "libdbus_proxy.h"
#include "log.h"
#include "types.h"

struct _FreewbEngine
{
    IBusEngine parent;
    freewb::ipc::LibDbusProxy *dbusProxy;
    freewb::Freewb *freewb;
    IBusPropList *propList;
    ::freewb::SpotRectPayload spotRect;
};

struct _FreewbEngineClass
{
    IBusEngineClass parent;
};

static DBusConnection *g_sessionBus = nullptr;

static void freewb_engine_destroy(IBusObject *object);
static gboolean freewb_engine_process_key_event(IBusEngine *engine, guint keyval, guint keycode, guint modifiers);
static void freewb_engine_focus_in(IBusEngine *engine);
static void freewb_engine_focus_out(IBusEngine *engine);
static void freewb_engine_reset(IBusEngine *engine);
static void freewb_engine_enable(IBusEngine *engine);
static void freewb_engine_disable(IBusEngine *engine);
static void freewb_engine_set_cursor_location(IBusEngine *engine, gint x, gint y, gint w, gint h);
static void freewb_engine_property_activate(IBusEngine *engine, const gchar *prop_name, guint prop_state);

static void updateSpotRect(FreewbEngine *engine);
static void commitString(FreewbEngine *engine, const std::string &text);
static IBusPropList *createPropList();

G_DEFINE_TYPE(FreewbEngine, freewb_engine, IBUS_TYPE_ENGINE)

static void freewb_engine_class_init(FreewbEngineClass *klass)
{
    auto *objectClass = IBUS_OBJECT_CLASS(klass);
    auto *engineClass = IBUS_ENGINE_CLASS(klass);

    objectClass->destroy = freewb_engine_destroy;
    engineClass->process_key_event = freewb_engine_process_key_event;
    engineClass->focus_in = freewb_engine_focus_in;
    engineClass->focus_out = freewb_engine_focus_out;
    engineClass->reset = freewb_engine_reset;
    engineClass->enable = freewb_engine_enable;
    engineClass->disable = freewb_engine_disable;
    engineClass->set_cursor_location = freewb_engine_set_cursor_location;
    engineClass->property_activate = freewb_engine_property_activate;
}

static DBusConnection *sessionBus()
{
    if (g_sessionBus != nullptr)
    {
        return g_sessionBus;
    }

    DBusError error;
    dbus_error_init(&error);
    g_sessionBus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (g_sessionBus == nullptr)
    {
        FREEWB_ERROR("ibus: dbus_bus_get failed: {}", error.message ? error.message : "unknown");
        dbus_error_free(&error);
        return nullptr;
    }
    if (!freewb::ibus::setupDBusWithGMain(g_sessionBus))
    {
        FREEWB_ERROR("ibus: setupDBusWithGMain failed");
        dbus_connection_unref(g_sessionBus);
        g_sessionBus = nullptr;
        return nullptr;
    }
    return g_sessionBus;
}

static void freewb_engine_init(FreewbEngine *engine)
{
    engine->dbusProxy = nullptr;
    engine->freewb = nullptr;
    engine->propList = nullptr;
    engine->spotRect = {0, 0, 0, 0};

    DBusConnection *conn = sessionBus();
    if (conn == nullptr)
    {
        FREEWB_ERROR("ibus: no session bus, engine will not work");
        return;
    }

    dbus_connection_ref(conn);
    engine->dbusProxy = new freewb::ipc::LibDbusProxy(conn);
    engine->freewb = new freewb::Freewb(engine->dbusProxy, [engine](const std::string &text) { commitString(engine, text); });
    engine->propList = createPropList();
}

static void freewb_engine_destroy(IBusObject *object)
{
    auto *engine = FREEWB_ENGINE(object);

    if (engine->freewb != nullptr)
    {
        delete engine->freewb;
        engine->freewb = nullptr;
    }
    if (engine->dbusProxy != nullptr)
    {
        delete engine->dbusProxy;
        engine->dbusProxy = nullptr;
    }
    if (engine->propList != nullptr)
    {
        g_object_unref(engine->propList);
        engine->propList = nullptr;
    }

    const auto *parentClass = IBUS_OBJECT_CLASS(freewb_engine_parent_class);
    if (parentClass != nullptr && parentClass->destroy != nullptr)
    {
        parentClass->destroy(object);
    }
}

static FreewbKeyState toFreewbState(guint modifiers)
{
    const guint cleaned = modifiers & ~static_cast<guint>(IBUS_RELEASE_MASK);
    return static_cast<FreewbKeyState>(cleaned);
}

static gboolean freewb_engine_process_key_event(IBusEngine *ibusEngine, guint keyval, guint /*keycode*/, guint modifiers)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb == nullptr)
    {
        return FALSE;
    }

    const auto keysym = static_cast<FreewbKeySym>(keyval);
    const FreewbKeyState state = toFreewbState(modifiers);
    bool processed = false;

    if ((modifiers & IBUS_RELEASE_MASK) != 0U)
    {
        processed = engine->freewb->processKeyRelease(keysym, state);
    }
    else
    {
        processed = engine->freewb->processKeyPress(keysym, state);
    }

    updateSpotRect(engine);
    engine->freewb->updateCandidateAndPreeditToUI();
    return processed ? TRUE : FALSE;
}

static void freewb_engine_focus_in(IBusEngine *ibusEngine)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->propList != nullptr)
    {
        ibus_engine_register_properties(ibusEngine, engine->propList);
    }
    if (engine->freewb != nullptr)
    {
        engine->freewb->activate();
    }
}

static void freewb_engine_focus_out(IBusEngine *ibusEngine)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb != nullptr)
    {
        engine->freewb->deactivate();
    }
}

static void freewb_engine_reset(IBusEngine *ibusEngine)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb != nullptr)
    {
        engine->freewb->reset();
        engine->freewb->updateCandidateAndPreeditToUI();
    }
}

static void freewb_engine_enable(IBusEngine *ibusEngine)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb != nullptr)
    {
        engine->freewb->activate();
    }
}

static void freewb_engine_disable(IBusEngine *ibusEngine)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb != nullptr)
    {
        engine->freewb->deactivate();
    }
}

static void freewb_engine_set_cursor_location(IBusEngine *ibusEngine, gint x, gint y, gint w, gint h)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    engine->spotRect.x = x;
    engine->spotRect.y = y;
    engine->spotRect.w = w > 0 ? w : 0;
    engine->spotRect.h = h > 0 ? h : 0;
    updateSpotRect(engine);
}

static void freewb_engine_property_activate(IBusEngine *ibusEngine, const gchar *prop_name, guint /*prop_state*/)
{
    auto *engine = FREEWB_ENGINE(ibusEngine);
    if (engine->freewb == nullptr || engine->freewb->dbusProxy() == nullptr || prop_name == nullptr)
    {
        return;
    }

    if (g_strcmp0(prop_name, "settings") == 0)
    {
        engine->freewb->dbusProxy()->callOpenUiSettingMethod();
    }
    else if (g_strcmp0(prop_name, "about") == 0)
    {
        engine->freewb->dbusProxy()->callShowVersionInfoMethod();
    }
}

static void updateSpotRect(FreewbEngine *engine)
{
    if (engine->freewb == nullptr || engine->freewb->dbusProxy() == nullptr)
    {
        return;
    }
    engine->freewb->dbusProxy()->callPanelUpdateSpotRect(engine->spotRect);
}

static void commitString(FreewbEngine *engine, const std::string &text)
{
    if (text.empty())
    {
        return;
    }
    IBusText *ibusText = ibus_text_new_from_string(text.c_str());
    ibus_engine_commit_text(IBUS_ENGINE(engine), ibusText);
}

static IBusPropList *createPropList()
{
    IBusPropList *props = ibus_prop_list_new();
    g_object_ref_sink(props);

    IBusText *settingsLabel = ibus_text_new_from_string(dgettext(FREEWB_TEXT_DOMAIN, "Settings"));
    IBusText *settingsTip = ibus_text_new_from_string(dgettext(FREEWB_TEXT_DOMAIN, "Open input method settings"));
    IBusProperty *settings =
        ibus_property_new("settings", PROP_TYPE_NORMAL, settingsLabel, "gtk-preferences", settingsTip, TRUE, TRUE, PROP_STATE_UNCHECKED, nullptr);
    ibus_prop_list_append(props, settings);

    IBusText *aboutLabel = ibus_text_new_from_string(dgettext(FREEWB_TEXT_DOMAIN, "About"));
    IBusText *aboutTip = ibus_text_new_from_string(dgettext(FREEWB_TEXT_DOMAIN, "Show version information"));
    IBusProperty *about =
        ibus_property_new("about", PROP_TYPE_NORMAL, aboutLabel, "gtk-about", aboutTip, TRUE, TRUE, PROP_STATE_UNCHECKED, nullptr);
    ibus_prop_list_append(props, about);

    return props;
}
