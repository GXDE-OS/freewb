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
    static QRect unitedAvailableGeometry();
    static QPoint clampTopLeft(const QPoint &topLeft, const QSize &windowSize);
};

} // namespace freewb

#endif
