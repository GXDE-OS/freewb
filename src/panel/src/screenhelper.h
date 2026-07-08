#ifndef SCREEN_HELPER_H
#define SCREEN_HELPER_H

#include <QPoint>
#include <QRect>
#include <QSize>

namespace freewb
{

class ScreenHelper
{
public:
    static QRect availableGeometryAt(const QPoint &globalPos);
    /** 钳制到 topLeft 所在屏幕（候选窗跟随光标） */
    static QPoint clampTopLeft(const QPoint &topLeft, const QSize &windowSize);
    /** 钳制到所有屏幕并集（工具条可跨屏拖动） */
    static QPoint clampTopLeftToUnitedDesktop(const QPoint &topLeft, const QSize &windowSize);

private:
    static QRect unitedAvailableGeometry();
};

} // namespace freewb

#endif
