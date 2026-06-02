#ifndef WAYLANDWINHELPER_H
#define WAYLANDWINHELPER_H

#include <QWidget>

namespace freewb
{

// UKUI Wayland 下为浮层窗口设置跳过任务栏、窗口切换器。
void applyWaylandOverlayWindowHints(QWidget *widget);

} // namespace freewb

#endif
