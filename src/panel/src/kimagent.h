/****************************************************************************************
** @作者：lcj
**
** @说明：
**      KimAgent是一个从QObject类继承而来的非UI类，该类设计通过使用DBUS协议来与fcitx通信进行输入
** 　　　面板的相关显示与控制．
*************************************************************************************×**/

#ifndef KIMAGENT_H
#define KIMAGENT_H

#include <QMetaObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QThread>
#include <QEventLoop>
#include <QTimer>




class KimPanel: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.impanel")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"org.kde.impanel\">\n"
                "    <signal name=\"MovePreeditCaret\">\n"
                "      <arg type=\"i\" name=\"position\"/>\n"
                "    </signal>\n"
                "    <signal name=\"SelectCandidate\">\n"
                "      <arg type=\"i\" name=\"index\"/>\n"
                "    </signal>\n"
                "    <signal name=\"PanelCreated\"/>\n"
                "    <signal name=\"PanelExit\"/>\n"
                "    <signal name=\"LookupTablePageUp\"/>\n"
                "    <signal name=\"LookupTablePageDown\"/>\n"
                "    <signal name=\"TriggerProperty\">\n"
                "      <arg type=\"s\" name=\"key\"/>\n"
                "    </signal>\n"
                "    <signal name=\"Exit\"/>\n"
                "    <signal name=\"Restart\"/>\n"
                "    <signal name=\"ReloadConfig\"/>\n"
                "    <signal name=\"Configure\"/>\n"
                "  </interface>\n"
                "")

public:
    KimPanel( QObject *parent );
    ~KimPanel();

// SIGNALS(暴露给DBUS由本进程发送给外界进程的信号)
Q_SIGNALS:
    void Configure();
    void Exit();
    void PanelCreated();
    void PanelExit();
    void LookupTablePageDown();
    void LookupTablePageUp();
    void MovePreeditCaret(int position);
    void ReloadConfig();
    void Restart();
    void SelectCandidate(int index);
    void TriggerProperty(const QString &key);

// METHODS(暴露给DBUS提供给外界进程调用的方法)
public Q_SLOTS:

};


class KimPanel2: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.impanel2")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"org.kde.impanel2\">\n"
                "    <signal name=\"PanelCreated2\"/>\n"
                "    <method name=\"SetSpotRect\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"x\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"y\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"w\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"h\"/>\n"
                "    </method>\n"
                "    <method name=\"SetLookupTable\">\n"
                "      <arg direction=\"in\" type=\"as\" name=\"label\"/>\n"
                "      <arg direction=\"in\" type=\"as\" name=\"text\"/>\n"
                "      <arg direction=\"in\" type=\"as\" name=\"attr\"/>\n"
                "      <arg direction=\"in\" type=\"b\" name=\"hasPrev\"/>\n"
                "      <arg direction=\"in\" type=\"b\" name=\"hasNext\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"cursor\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"layout\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")

public:
    KimPanel2( QObject *parent );
    ~KimPanel2();

// SIGNALS(暴露给DBUS由本进程发送给外界进程的信号)
Q_SIGNALS:
    void PanelCreated2();

// METHODS(暴露给DBUS提供给外界进程调用的方法)
public Q_SLOTS:
    void SetLookupTable( const QStringList &label,
                         const QStringList &text,
                         const QStringList &attr,
                         bool hasPrev, bool hasNext,
                         int cursor,
                         int layout );

    void SetSpotRect( int x, int y, int w, int h );
};


class KimAgent : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    KimAgent( QObject *parent = nullptr );
    ~KimAgent();

signals:
    //以下信号是fcitx源码中规定的可以捕获到的从指定接口＂org.kde.impanel＂发出来的有效信号(用来发送给fcitx)
    void Configure();
    void Exit();
    void LookupTablePageDown();
    void LookupTablePageUp();
    void MovePreeditCaret( int position );
    void PanelCreated();
    void PanelCreated2();
    void PanelExit();
    void ReloadConfig();
    void Restart();
    void SelectCandidate( int index );
    void TriggerProperty( const QString &key );

    //以下信号是接收到fcitx对应的信号后转发出去的程序内部信号
    void signal_Enable( bool );
    void signal_ShowPreedit( bool );
    void signal_ShowAux( bool );
    void signal_ShowLookupTable( bool );
    void signal_UpdateLookupTableCursor( int );
    void signal_UpdateLookupTable( const QStringList&, const QStringList&, const QStringList&, bool, bool );
    void signal_UpdatePreeditCaret( int );
    void signal_UpdatePreeditText( const QString&, const QString& );
    void signal_UpdateAux( const QString&, const QString& );
    void signal_UpdateSpotLocation( int, int );
    void signal_UpdateScreen( int );
    void signal_UpdateProperty( const QString& );
    void signal_RegisterProperties( const QStringList& );
    void signal_ExecDialog( const QString& );
    void signal_ExecMenu( const QStringList& );

    //以下信号是接收到fcitx对应的方法调用后转发出去的程序内部信号
    void signal_SetSpotLocation( int, int, int, int );
    void signal_SetLookupTable( const QStringList&, const QStringList&, const QStringList&, bool, bool, int );


public:
    int create_fcitx_panel();
    void delete_fcitx_panel();

protected slots:
    void slot_SetSpotLocation( int, int, int, int );
    void slot_SetLookupTable( const QStringList&, const QStringList&, const QStringList&, bool, bool, int );


private:
    KimPanel *m_kimPanel;
    KimPanel2 *m_kimPanel2;
};
#endif

