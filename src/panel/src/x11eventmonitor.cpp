#include "x11eventmonitor.h"

X11EventMonitor::X11EventMonitor(QObject *parent) : QThread(parent)
{
    m_keyValue = 0;
}

void X11EventMonitor::run()
{
    Display *x11Display = XOpenDisplay(nullptr);
    if (x11Display == nullptr)
    {
        qWarning() << "unable to open x11 display!";
        return;
    }

    // Receive from ALL clients, including future clients.
    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange *range = XRecordAllocRange();
    if (range == nullptr)
    {
        qWarning() << "unable to allocate XRecordRange!";
        return;
    }

    // Receive KeyPress, KeyRelease, ButtonPress, ButtonRelease and MotionNotify events.
    memset(range, 0, sizeof(XRecordRange));
    range->device_events.first = KeyPress;
    range->device_events.last = MotionNotify;

    // And create the XRECORD context.
    XRecordContext context = XRecordCreateContext(x11Display, 0, &clients, 1, &range, 1);
    if (context == 0)
    {
        qWarning() << "XRecordCreateContext failed!";
        return;
    }

    XFree(range);
    XSync(x11Display, True);

    Display *displayDatalink = XOpenDisplay(nullptr);
    if (displayDatalink == nullptr)
    {
        qWarning() << "unable to open second display!";
        return;
    }

    if (!XRecordEnableContext(displayDatalink, context, x11EventCallback, reinterpret_cast<XPointer>(this)))
    {
        qWarning() << "XRecordEnableContext failed!";
        return;
    }
}

void X11EventMonitor::x11EventCallback(XPointer ptr, XRecordInterceptData *data)
{
    reinterpret_cast<X11EventMonitor *>(ptr)->handleMonitorX11Event(data);
}

void X11EventMonitor::handleMonitorX11Event(XRecordInterceptData *data)
{
    if (data->category == XRecordFromServer)
    {
        int value;
        xEvent *event = reinterpret_cast<xEvent *>(data->data);
        switch (event->u.u.type)
        {
        case KeyPress:
        {
            value = reinterpret_cast<unsigned char *>(data->data)[1];
            emit signal_key_pressed(value);
            m_keyValue = value;
            // qDebug() << "KeyPress:" << m_keyValue;
            break;
        }
        case KeyRelease:
        {
            value = reinterpret_cast<unsigned char *>(data->data)[1];
            if (m_keyValue == value)
            {
                emit signal_key_clicked(value);
                // qDebug() << "keyClicked:" << value;
            }
            m_keyValue = 0;
            break;
        }
        case ButtonPress:
        {
            value = event->u.u.detail;
            emit signal_button_pressed(value, event->u.keyButtonPointer.rootX, event->u.keyButtonPointer.rootY);
            break;
        }
        case ButtonRelease:
        {
            break;
        }
        case MotionNotify:
            break;
        }
    }

    XRecordFreeData(data);
}
