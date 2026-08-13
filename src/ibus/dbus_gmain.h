#ifndef FREEWB_IBUS_DBUS_GMAIN_H
#define FREEWB_IBUS_DBUS_GMAIN_H

#include <dbus/dbus.h>

namespace freewb::ibus
{

/** 将 libdbus 连接接入当前 GLib 主循环，以便 Panel 信号可被分发。 */
bool setupDBusWithGMain(DBusConnection *conn);

} // namespace freewb::ibus

#endif
