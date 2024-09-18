#include "kimagent.h"
#include "commdefine.h"
#include <unistd.h>

/***************　fcitx通信地址相关参数(固定不可变)　*****************/
#define DBUS_FCITX_INTERFACE        "org.kde.kimpanel.inputmethod"
/***************************************************************/

#define DBUS_KIM_PANEL_SERVICE_NAME     "org.kde.impanel"//固定不可变
#define DBUS_KIM_PANEL_OBJECT_PATH      "/org/kde/impanel"//固定不可变
#define DBUS_KIM_PANEL_CONNECT_NAME     "kimpanel_bus"//自定义


KimPanel::KimPanel( QObject *parent ) : QDBusAbstractAdaptor( parent )
{
    //继承父对象的信号(调用父对象信号实际就是调用本对象信号)
    setAutoRelaySignals( true );
}

KimPanel::~KimPanel()
{
}

KimPanel2::KimPanel2( QObject *parent ) : QDBusAbstractAdaptor( parent )
{
    //继承父对象的信号(调用父对象信号实际就是调用本对象信号)
    setAutoRelaySignals( true );
}

KimPanel2::~KimPanel2()
{
}

KimAgent::KimAgent( QObject *parent ) : QObject( parent )
{
    //创建与fcitx通信的DBUS信号与方法
    m_kimPanel = new KimPanel( this );
    m_kimPanel2 = new KimPanel2( this );
}


KimAgent::~KimAgent()
{
}


int KimAgent::create_fcitx_panel()
{

    //puts("create_fcitx_panel::创建面板*******\n");

    //等待kimpanel面板注册成功
    int i = 0;
    while ( true )
    {
        //注册dbus服务与对象，服务名必须与fcitx源码规定的一样!!
        if ( !QDBusConnection::connectToBus( QDBusConnection::SessionBus, DBUS_KIM_PANEL_CONNECT_NAME )
             .registerService( DBUS_KIM_PANEL_SERVICE_NAME ) )
        {
            qWarning() << "create panel service failed! count: " << ++i;

            QEventLoop loop;
            QTimer::singleShot( 500, &loop, SLOT(quit()));
            loop.exec();

            if ( i == 10 )
                return 1; //失败

            continue;
        }
        else
        {
            break;
        }
    }


    QDBusConnection::connectToBus( QDBusConnection::SessionBus, DBUS_KIM_PANEL_CONNECT_NAME )
                        .registerObject( DBUS_KIM_PANEL_OBJECT_PATH, this );

    //连接fcitx发送出来的相关信号
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "Enable", this, SIGNAL(signal_Enable(bool)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "ShowPreedit", this, SIGNAL(signal_ShowPreedit(bool)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "ShowAux", this, SIGNAL(signal_ShowAux(bool)));
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "ShowLookupTable", this, SIGNAL(signal_ShowLookupTable(bool)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateLookupTableCursor", this, SIGNAL(signal_UpdateLookupTableCursor(int)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateLookupTable", this, SIGNAL(signal_UpdateLookupTable(QStringList, QStringList, QStringList, bool, bool)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdatePreeditCaret", this, SIGNAL(signal_UpdatePreeditCaret(int)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdatePreeditText", this, SIGNAL(signal_UpdatePreeditText(QString, QString)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateAux", this, SIGNAL(signal_UpdateAux(QString, QString)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateSpotLocation", this, SIGNAL(signal_UpdateSpotLocation(int, int)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateScreen", this, SIGNAL(UpdateScreen(int)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "UpdateProperty", this, SIGNAL(signal_UpdateProperty(QString)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "RegisterProperties", this, SIGNAL(signal_RegisterProperties(QStringList)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "ExecDialog", this, SIGNAL(signal_ExecDialog(QString)) );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).connect( "", "", DBUS_FCITX_INTERFACE,
                                           "ExecMenu", this, SIGNAL(signal_ExecMenu(QStringList)) );

    emit PanelCreated2();

    return 0;
}

void KimAgent::delete_fcitx_panel()
{
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).unregisterObject( DBUS_KIM_PANEL_OBJECT_PATH );
    QDBusConnection( DBUS_KIM_PANEL_CONNECT_NAME ).unregisterService( DBUS_KIM_PANEL_SERVICE_NAME );
    QDBusConnection::disconnectFromBus( DBUS_KIM_PANEL_CONNECT_NAME );
    #ifdef DEBUG
    qDebug() << "kimpanel exit！";
    #endif
}


//设置候选框位置
void KimAgent::slot_SetSpotLocation( int x, int y, int w, int h )
{
    emit signal_SetSpotLocation( x, y, w, h );
}

//设置候选框显示内容
void KimAgent::slot_SetLookupTable( const QStringList &label,
                                    const QStringList &text,
                                    const QStringList &attr,
                                    bool hasPrev,
                                    bool hasNext,
                                    int cursor )
{
    emit signal_SetLookupTable( label, text, attr, hasPrev, hasNext, cursor );
}


void KimPanel2::SetLookupTable( const QStringList &label,
                                const QStringList &text,
                                const QStringList &attr,
                                bool hasPrev,
                                bool hasNext,
                                int cursor,
                                int layout )
{
    Q_UNUSED(layout);
    QMetaObject::invokeMethod( parent(), "slot_SetLookupTable",
                               Q_ARG(QStringList, label),
                               Q_ARG(QStringList, text),
                               Q_ARG(QStringList, attr),
                               Q_ARG(bool, hasPrev),
                               Q_ARG(bool, hasNext),
                               Q_ARG(int, cursor) );
}


void KimPanel2::SetSpotRect( int x, int y, int w, int h )
{
    QMetaObject::invokeMethod( parent(), "slot_SetSpotLocation",
                               Q_ARG(int, x),
                               Q_ARG(int, y),
                               Q_ARG(int, w),
                               Q_ARG(int, h));
}


