#include "screenhelper.h"

#include <QGuiApplication>
#include <QScreen>

namespace freewb
{

QRect ScreenHelper::availableGeometryAt(const QPoint &globalPos)
{
    if (const QScreen *screen = QGuiApplication::screenAt(globalPos))
    {
        return screen->availableGeometry();
    }

    return unitedAvailableGeometry();
}

QRect ScreenHelper::unitedAvailableGeometry()
{
    QRect bounds;
    for (const QScreen *screen : QGuiApplication::screens())
    {
        if (screen == nullptr)
        {
            continue;
        }
        const QRect geo = screen->availableGeometry();
        bounds = bounds.isNull() ? geo : bounds.united(geo);
    }
    return bounds;
}

QPoint ScreenHelper::clampTopLeft(const QPoint &topLeft, const QSize &windowSize)
{
    const QRect bounds = availableGeometryAt(topLeft);
    if (bounds.isNull() || !windowSize.isValid())
    {
        return topLeft;
    }

    const int maxX = bounds.x() + bounds.width() - windowSize.width();
    const int maxY = bounds.y() + bounds.height() - windowSize.height();
    QPoint clamped = topLeft;
    if (maxX >= bounds.x())
    {
        clamped.setX(qBound(bounds.x(), topLeft.x(), maxX));
    }
    if (maxY >= bounds.y())
    {
        clamped.setY(qBound(bounds.y(), topLeft.y(), maxY));
    }
    return clamped;
}

QPoint ScreenHelper::clampTopLeftToUnitedDesktop(const QPoint &topLeft, const QSize &windowSize)
{
    const QRect bounds = unitedAvailableGeometry();
    if (bounds.isNull() || !windowSize.isValid())
    {
        return topLeft;
    }

    const int maxX = bounds.x() + bounds.width() - windowSize.width();
    const int maxY = bounds.y() + bounds.height() - windowSize.height();
    QPoint clamped = topLeft;
    if (maxX >= bounds.x())
    {
        clamped.setX(qBound(bounds.x(), topLeft.x(), maxX));
    }
    if (maxY >= bounds.y())
    {
        clamped.setY(qBound(bounds.y(), topLeft.y(), maxY));
    }
    return clamped;
}

} // namespace freewb
