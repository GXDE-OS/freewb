#include "waylandwinhelper.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QWidget>
#include <QWindow>

namespace freewb
{

void applyWaylandOverlayWindowHints(QWidget *widget)
{
    if (!widget)
    {
        return;
    }
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
    {
        return;
    }
    if (!qgetenv("XDG_CURRENT_DESKTOP").toLower().contains("ukui"))
    {
        return;
    }

    if (!widget->windowHandle())
    {
        widget->winId();
    }
    QWindow *window = widget->windowHandle();
    if (!window)
    {
        return;
    }

    window->setProperty("ukui_surface_skip_taskbar", true);
    window->setProperty("ukui_surface_skip_switcher", true);
}

} // namespace freewb
