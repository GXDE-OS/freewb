#include "dbus_gmain.h"

#include <glib.h>

namespace freewb::ibus
{
namespace
{

struct WatchData
{
    DBusWatch *watch = nullptr;
    guint sourceId = 0;
};

struct TimeoutData
{
    DBusTimeout *timeout = nullptr;
    guint sourceId = 0;
};

gboolean onWatch(GIOChannel * /*channel*/, GIOCondition condition, gpointer userData)
{
    auto *data = static_cast<WatchData *>(userData);
    unsigned int flags = 0;
    if ((condition & G_IO_IN) != 0)
    {
        flags |= DBUS_WATCH_READABLE;
    }
    if ((condition & G_IO_OUT) != 0)
    {
        flags |= DBUS_WATCH_WRITABLE;
    }
    if ((condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) != 0)
    {
        flags |= DBUS_WATCH_ERROR;
    }
    dbus_watch_handle(data->watch, flags);
    return TRUE;
}

void destroyWatch(gpointer userData)
{
    delete static_cast<WatchData *>(userData);
}

dbus_bool_t addWatch(DBusWatch *watch, void * /*data*/)
{
    if (!dbus_watch_get_enabled(watch))
    {
        return TRUE;
    }

    const unsigned int flags = dbus_watch_get_flags(watch);
    GIOCondition condition = static_cast<GIOCondition>(0);
    if ((flags & DBUS_WATCH_READABLE) != 0U)
    {
        condition = static_cast<GIOCondition>(condition | G_IO_IN);
    }
    if ((flags & DBUS_WATCH_WRITABLE) != 0U)
    {
        condition = static_cast<GIOCondition>(condition | G_IO_OUT);
    }
    condition = static_cast<GIOCondition>(condition | G_IO_ERR | G_IO_HUP);

    auto *data = new WatchData;
    data->watch = watch;
    GIOChannel *channel = g_io_channel_unix_new(dbus_watch_get_unix_fd(watch));
    data->sourceId = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, condition, onWatch, data, destroyWatch);
    g_io_channel_unref(channel);
    dbus_watch_set_data(watch, data, nullptr);
    return data->sourceId != 0;
}

void removeWatch(DBusWatch *watch, void * /*data*/)
{
    auto *watchData = static_cast<WatchData *>(dbus_watch_get_data(watch));
    if (watchData == nullptr)
    {
        return;
    }
    dbus_watch_set_data(watch, nullptr, nullptr);
    if (watchData->sourceId != 0)
    {
        const guint sourceId = watchData->sourceId;
        watchData->sourceId = 0;
        g_source_remove(sourceId);
    }
    else
    {
        delete watchData;
    }
}

void toggleWatch(DBusWatch *watch, void *data)
{
    if (dbus_watch_get_enabled(watch))
    {
        addWatch(watch, data);
    }
    else
    {
        removeWatch(watch, data);
    }
}

gboolean onTimeout(gpointer userData)
{
    auto *data = static_cast<TimeoutData *>(userData);
    dbus_timeout_handle(data->timeout);
    return TRUE;
}

void destroyTimeout(gpointer userData)
{
    delete static_cast<TimeoutData *>(userData);
}

dbus_bool_t addTimeout(DBusTimeout *timeout, void * /*data*/)
{
    if (!dbus_timeout_get_enabled(timeout))
    {
        return TRUE;
    }

    auto *data = new TimeoutData;
    data->timeout = timeout;
    const int interval = dbus_timeout_get_interval(timeout);
    data->sourceId = g_timeout_add_full(G_PRIORITY_DEFAULT, static_cast<guint>(interval), onTimeout, data, destroyTimeout);
    dbus_timeout_set_data(timeout, data, nullptr);
    return data->sourceId != 0;
}

void removeTimeout(DBusTimeout *timeout, void * /*data*/)
{
    auto *timeoutData = static_cast<TimeoutData *>(dbus_timeout_get_data(timeout));
    if (timeoutData == nullptr)
    {
        return;
    }
    dbus_timeout_set_data(timeout, nullptr, nullptr);
    if (timeoutData->sourceId != 0)
    {
        const guint sourceId = timeoutData->sourceId;
        timeoutData->sourceId = 0;
        g_source_remove(sourceId);
    }
    else
    {
        delete timeoutData;
    }
}

void toggleTimeout(DBusTimeout *timeout, void *data)
{
    if (dbus_timeout_get_enabled(timeout))
    {
        addTimeout(timeout, data);
    }
    else
    {
        removeTimeout(timeout, data);
    }
}

gboolean dispatchConnection(gpointer userData)
{
    auto *conn = static_cast<DBusConnection *>(userData);
    while (dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS)
    {
    }
    return G_SOURCE_REMOVE;
}

void onDispatchStatus(DBusConnection *conn, DBusDispatchStatus status, void * /*data*/)
{
    if (status == DBUS_DISPATCH_DATA_REMAINS)
    {
        g_idle_add(dispatchConnection, conn);
    }
}

} // namespace

bool setupDBusWithGMain(DBusConnection *conn)
{
    if (conn == nullptr)
    {
        return false;
    }

    if (!dbus_connection_set_watch_functions(conn, addWatch, removeWatch, toggleWatch, nullptr, nullptr))
    {
        return false;
    }
    if (!dbus_connection_set_timeout_functions(conn, addTimeout, removeTimeout, toggleTimeout, nullptr, nullptr))
    {
        return false;
    }
    dbus_connection_set_dispatch_status_function(conn, onDispatchStatus, nullptr, nullptr);
    if (dbus_connection_get_dispatch_status(conn) == DBUS_DISPATCH_DATA_REMAINS)
    {
        g_idle_add(dispatchConnection, conn);
    }
    return true;
}

} // namespace freewb::ibus
