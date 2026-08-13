#include <ibus.h>

#include "config.h"
#include "engine.h"
#include "log.h"

namespace
{

gboolean g_ibusMode = FALSE;
gboolean g_verbose = FALSE;

const GOptionEntry kEntries[] = {
    {"ibus", 'i', 0, G_OPTION_ARG_NONE, &g_ibusMode, "component is executed by ibus", nullptr},
    {"verbose", 'v', 0, G_OPTION_ARG_NONE, &g_verbose, "verbose", nullptr},
    {nullptr},
};

void onDisconnected(IBusBus * /*bus*/, gpointer /*userData*/)
{
    ibus_quit();
}

void initEngine()
{
    bindtextdomain(FREEWB_TEXT_DOMAIN, FREEWB_INSTALL_LOCALEDIR);
    bind_textdomain_codeset(FREEWB_TEXT_DOMAIN, "UTF-8");

    ibus_init();

    IBusBus *bus = ibus_bus_new();
    g_object_ref_sink(bus);
    g_signal_connect(bus, "disconnected", G_CALLBACK(onDisconnected), nullptr);

    IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
    g_object_ref_sink(factory);
    ibus_factory_add_engine(factory, "freewb", FREEWB_TYPE_ENGINE);

    if (g_ibusMode)
    {
        ibus_bus_request_name(bus, "org.freedesktop.IBus.Freewb", 0);
    }
    else
    {
        IBusComponent *component =
            ibus_component_new("org.freedesktop.IBus.Freewb", "Freewb Input Method", FREEWB_VERSION, "GPL",
                               "Freewb Authors", "https://gitee.com/openkylin/freewb", "", "freewb");
        ibus_component_add_engine(component,
                                  ibus_engine_desc_new("freewb", "Freewb Input Method", "Freewb Input Method", "zh_CN",
                                                       "GPL", "Freewb Authors", "freewb", "us"));
        ibus_bus_register_component(bus, component);
        g_object_unref(component);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    GError *error = nullptr;
    GOptionContext *context = g_option_context_new("- freewb ibus engine");
    g_option_context_add_main_entries(context, kEntries, FREEWB_TEXT_DOMAIN);
    if (!g_option_context_parse(context, &argc, &argv, &error))
    {
        FREEWB_ERROR("ibus option parsing failed: {}", error != nullptr ? error->message : "unknown");
        if (error != nullptr)
        {
            g_error_free(error);
        }
        g_option_context_free(context);
        return 1;
    }
    g_option_context_free(context);

    initEngine();
    ibus_main();
    return 0;
}
