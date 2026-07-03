#include "ukuiwaylandhelper.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QPair>
#include <QVariant>
#include <QWindow>

namespace freewb
{

const char UkuiWaylandHelper::kSurfaceRole[] = "ukui_surface_role";
const char UkuiWaylandHelper::kSurfaceState[] = "ukui_surface_state";
const char UkuiWaylandHelper::kSurfaceNoTitlebar[] = "ukui_surface_no_titlebar";
const char UkuiWaylandHelper::kSurfaceSkipTaskbar[] = "ukui_surface_skip_taskbar";
const char UkuiWaylandHelper::kSurfaceSkipSwitcher[] = "ukui_surface_skip_switcher";
const char UkuiWaylandHelper::kRoleInputPanel[] = "inputpanel";

bool UkuiWaylandHelper::isUkuiWayland()
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland")))
    {
        return false;
    }
    return qgetenv("XDG_CURRENT_DESKTOP").toLower().contains("ukui");
}

void UkuiWaylandHelper::setWidgetProperty(QWidget *widget, const char *name, const QVariant &value)
{
    widget->setProperty(name, value);
    if (QWindow *window = widget->windowHandle())
    {
        window->setProperty(name, value);
    }
}

void UkuiWaylandHelper::applyInputPanelWindowFlags(QWidget *widget)
{
    if (!widget || !isUkuiWayland())
    {
        return;
    }

    widget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
}

void UkuiWaylandHelper::applyInputPanelHints(QWidget *widget)
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
