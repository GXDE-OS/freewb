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
    // Panel UI → 输入法引擎
    Q_SCRIPTABLE void LookupTablePageDown();
    Q_SCRIPTABLE void LookupTablePageUp();
    Q_SCRIPTABLE void ReloadConfig();
    Q_SCRIPTABLE void SelectCandidate(int index);
    Q_SCRIPTABLE void SwitchPunctuation();
    Q_SCRIPTABLE void SwitchFullWidth();
    Q_SCRIPTABLE void SwitchChttrans();
    Q_SCRIPTABLE void RequestNextInputMode();

public Q_SLOTS:
    // Panel UI → 引擎 D-Bus 方法
    Q_SCRIPTABLE void ShowPreedit(bool show);
    Q_SCRIPTABLE void ShowAux(bool show);
    Q_SCRIPTABLE void ShowLookupTable(bool show);
    Q_SCRIPTABLE void UpdateLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                        bool hasNext);
    Q_SCRIPTABLE void UpdatePreeditCaret(int position);
    Q_SCRIPTABLE void UpdatePreeditText(const QString &text, const QString &attr);
    Q_SCRIPTABLE void UpdateAux(const QString &text, const QString &attr);
    Q_SCRIPTABLE void UpdateSpotLocation(int x, int y);
    Q_SCRIPTABLE void UpdateProperty(const QString &prop);
    Q_SCRIPTABLE void RegisterProperties(const QStringList &props);
    Q_SCRIPTABLE void SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                     bool hasNext, int cursor, int layout);
    Q_SCRIPTABLE void SetSpotRect(int x, int y, int w, int h);

    Q_SIGNALS:
    // UI内部信号
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
    void signal_SetSpotLocation(int, int, int, int);
    void signal_SetLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool, int);

private:
    void registerQDBusService();
    void unRegisterQDBusService();
};

#endif // QDBUS_PANEL_H
