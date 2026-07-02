#ifndef WAYLANDWINHELPER_H
#define WAYLANDWINHELPER_H

#include <cstdint>

#include <QVariant>
#include <QWidget>

namespace freewb
{

class WaylandWinHelper
{
public:
    // UKUI Wayland 下为普通浮层窗口设置跳过任务栏、窗口切换器（设置页、菜单等）。
    static void applyOverlayHints(QWidget *widget);

    // UKUI Wayland 下调整 Qt 窗口 flags，配合 inputpanel 使用（候选窗等）。
    static void applyInputPanelWindowFlags(QWidget *widget);

    // UKUI Wayland 下为输入法 UI 设置 inputpanel 层级（工具栏、候选窗、tooltip 等）。
    static void applyInputPanelHints(QWidget *widget);

private:
    static bool isUkuiWayland();
    static void setWidgetProperty(QWidget *widget, const char *name, const QVariant &value);

    static const char kSurfaceRole[];
    static const char kSurfaceState[];
    static const char kSurfaceNoTitlebar[];
    static const char kSurfaceSkipTaskbar[];
    static const char kSurfaceSkipSwitcher[];
    static const char kRoleInputPanel[];

    static constexpr uint32_t kWindowStateMaskAll = 0x3ff;
    static constexpr uint32_t kWindowStateMovable = 0x10;
};

} // namespace freewb

#endif
