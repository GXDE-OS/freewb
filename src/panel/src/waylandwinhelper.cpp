#include "waylandwinhelper.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QPair>
#include <QVariant>
#include <QWindow>

namespace freewb
{

const char WaylandWinHelper::kSurfaceRole[] = "ukui_surface_role";
const char WaylandWinHelper::kSurfaceState[] = "ukui_surface_state";
const char WaylandWinHelper::kSurfaceNoTitlebar[] = "ukui_surface_no_titlebar";
const char WaylandWinHelper::kSurfaceSkipTaskbar[] = "ukui_surface_skip_taskbar";
const char WaylandWinHelper::kSurfaceSkipSwitcher[] = "ukui_surface_skip_switcher";
const char WaylandWinHelper::kRoleInputPanel[] = "inputpanel";

bool WaylandWinHelper::isUkuiWayland()
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
    {
        return false;
    }
    return qgetenv("XDG_CURRENT_DESKTOP").toLower().contains("ukui");
}

void WaylandWinHelper::setWidgetProperty(QWidget *widget, const char *name, const QVariant &value)
{
    widget->setProperty(name, value);
    if (QWindow *window = widget->windowHandle())
    {
        window->setProperty(name, value);
    }
}

void WaylandWinHelper::applyOverlayHints(QWidget *widget)
{
    if (!widget || !isUkuiWayland())
    {
        return;
    }

    setWidgetProperty(widget, kSurfaceSkipTaskbar, true);
    setWidgetProperty(widget, kSurfaceSkipSwitcher, true);
}

void WaylandWinHelper::applyInputPanelWindowFlags(QWidget *widget)
{
    if (!widget || !isUkuiWayland())
    {
        return;
    }

    widget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
}

void WaylandWinHelper::applyInputPanelHints(QWidget *widget)
{
    if (!widget || !isUkuiWayland())
    {
        return;
    }

    widget->setProperty("useStyleWindowManager", QVariant(false));
    setWidgetProperty(widget, kSurfaceRole, kRoleInputPanel);
    const QPair<uint32_t, uint32_t> movableState(kWindowStateMaskAll, kWindowStateMovable);
    setWidgetProperty(widget, kSurfaceState, QVariant::fromValue(movableState));
    setWidgetProperty(widget, kSurfaceNoTitlebar, true);
    setWidgetProperty(widget, kSurfaceSkipTaskbar, true);
    setWidgetProperty(widget, kSurfaceSkipSwitcher, true);
}

} // namespace freewb
