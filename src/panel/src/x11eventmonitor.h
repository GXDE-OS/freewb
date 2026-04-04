#ifndef X11EVENTMONITOR_H
#define X11EVENTMONITOR_H

#include <QDebug>
#include <QThread>
#include <X11/Xlibint.h> //其内部包含了"X11/Xlib.h"
#include <X11/extensions/record.h>
// 在头文件"X11/Xlib.h"中的第82行定义了"#define Bool int",
// 不知为何会使得在用camke编译的时候报错,
// 在此取消Bool的定义方可使用cmake编译通过
#ifdef Bool
#undef Bool
#endif
/* Xlibint.h defines min/max macros; they break std::min in later C++ headers. */
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class X11EventMonitor : public QThread
{
    Q_OBJECT

public:
    X11EventMonitor(QObject *parent = nullptr);

signals:
    void signal_key_pressed(int code);
    void signal_key_released(int code);
    void signal_key_clicked(int code);
    void signal_button_pressed(int button, int x, int y);
    void signal_button_released(int button, int x, int y);
    void signal_button_drag(int x, int y);
    void signal_button_clicked(int button, int x, int y);

protected:
    void run();
    void handleMonitorX11Event(XRecordInterceptData *);
    static void x11EventCallback(XPointer trash, XRecordInterceptData *data);

private:
    int m_keyValue;
    int m_buttonValue;
    bool m_mouseIsPressed;
};

#endif
