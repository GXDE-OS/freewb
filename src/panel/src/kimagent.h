/****************************************************************************************
** @作者：lcj
**
** @说明：
**      KimAgent是一个从QObject类继承而来的非UI类，该类设计通过使用DBUS协议来与fcitx通信进行输入
** 　　　面板的相关显示与控制．
*************************************************************************************×**/

#ifndef KIMAGENT_H
#define KIMAGENT_H

#include <QByteArray>
#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QEventLoop>
#include <QList>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVariant>

#include "../../ipc/ipc.h"

class KimAgent : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", FREEWUBI_PANEL_INTERFACE)

public:
    KimAgent(QObject *parent = nullptr);
    ~KimAgent();

Q_SIGNALS:
    // signals listened by fcitx's dbus service
    Q_SCRIPTABLE void LookupTablePageDown();
    Q_SCRIPTABLE void LookupTablePageUp();
    Q_SCRIPTABLE void ReloadConfig();
    Q_SCRIPTABLE void SelectCandidate(int index);
    Q_SCRIPTABLE void SwitchPunctuation();
    Q_SCRIPTABLE void SwitchFullWidth();
    Q_SCRIPTABLE void SwitchChttrans();
    Q_SCRIPTABLE void RequestNextInputMode();

    // 以下信号是接收到fcitx对应的信号后转发出去的程序内部信号
    void signal_ShowPreedit(bool);
    void signal_ShowAux(bool);
    void signal_ShowLookupTable(bool);
    void signal_UpdateLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool);
    void signal_UpdatePreeditCaret(int);
    void signal_UpdatePreeditText(const QString &, const QString &);
    void signal_UpdateAux(const QString &, const QString &);
    void signal_UpdateSpotLocation(int, int);
    void signal_UpdateProperty(const QString &);
    void signal_RegisterProperties(const QStringList &);

    // 以下信号是接收到fcitx对应的方法调用后转发出去的程序内部信号
    void signal_SetSpotLocation(int, int, int, int);
    void signal_SetLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool, int);

public:
    int create_fcitx_panel();
    void delete_fcitx_panel();

private:
    void create_freewubi_panel_service();

    // METHODS(暴露给DBUS提供给外界进程调用的方法)
public Q_SLOTS:
    Q_SCRIPTABLE void SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                     bool hasNext, int cursor, int layout);

    Q_SCRIPTABLE void SetSpotRect(int x, int y, int w, int h);
};
#endif
