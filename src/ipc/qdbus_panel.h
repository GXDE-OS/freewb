#ifndef QDBUS_PANEL_H
#define QDBUS_PANEL_H

#include <QDBusConnection>
#include <QObject>
#include <QString>
#include <QStringList>

#include "ipc.h"

class QDBusPanelService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", FREEWUBI_PANEL_INTERFACE)

public:
    explicit QDBusPanelService(QObject *parent = nullptr);
    ~QDBusPanelService() override;

Q_SIGNALS:
    // 由 Fcitx 插件监听：Panel UI → 输入法引擎
    Q_SCRIPTABLE void LookupTablePageDown();
    Q_SCRIPTABLE void LookupTablePageUp();
    Q_SCRIPTABLE void ReloadConfig();
    Q_SCRIPTABLE void SelectCandidate(int index);
    Q_SCRIPTABLE void SwitchPunctuation();
    Q_SCRIPTABLE void SwitchFullWidth();
    Q_SCRIPTABLE void SwitchChttrans();
    Q_SCRIPTABLE void RequestNextInputMode();

    // 输入法引擎信号转发：引擎 → Panel 内部 UI
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

    // 引擎方法调用转发：引擎 → Panel 内部 UI
    void signal_SetSpotLocation(int, int, int, int);
    void signal_SetLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool, int);

public Q_SLOTS:
    Q_SCRIPTABLE void SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                     bool hasNext, int cursor, int layout);

    Q_SCRIPTABLE void SetSpotRect(int x, int y, int w, int h);

private:
    void registerQDBusService();
    void connectDBusSignals();
    void unRegisterQDBusService();
};

#endif // QDBUS_PANEL_H
