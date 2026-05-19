#include "qdbus_panel_service.h"

QDBusPanelService::QDBusPanelService(QObject *parent) : QObject(parent)
{
    registerQDBusService();

    connectDBusSignals();
}

QDBusPanelService::~QDBusPanelService()
{
    unRegisterQDBusService();
}

void QDBusPanelService::connectDBusSignals()
{
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "ShowPreedit", this, SIGNAL(signal_ShowPreedit(bool)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "ShowAux", this, SIGNAL(signal_ShowAux(bool)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "ShowLookupTable", this, SIGNAL(signal_ShowLookupTable(bool)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdateLookupTable", this,
                 SIGNAL(signal_UpdateLookupTable(QStringList, QStringList, QStringList, bool, bool)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdatePreeditCaret", this, SIGNAL(signal_UpdatePreeditCaret(int)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdatePreeditText", this,
                 SIGNAL(signal_UpdatePreeditText(QString, QString)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdateAux", this, SIGNAL(signal_UpdateAux(QString, QString)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdateSpotLocation", this,
                 SIGNAL(signal_UpdateSpotLocation(int, int)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "UpdateProperty", this, SIGNAL(signal_UpdateProperty(QString)));
    QDBusConnection(FREEWUBI_SESSION_BUSNAME)
        .connect("", "", FREEWUBI_INPUTMETHOD_SERVICENAME, "RegisterProperties", this,
                 SIGNAL(signal_RegisterProperties(QStringList)));
}

void QDBusPanelService::unRegisterQDBusService()
{
    QDBusConnection(FREEWUBI_SESSION_BUSNAME).unregisterObject(FREEWUBI_PANEL_OBJECTPATH);
    QDBusConnection(FREEWUBI_SESSION_BUSNAME).unregisterService(FREEWUBI_PANEL_SERVICENAME);
    QDBusConnection::disconnectFromBus(FREEWUBI_SESSION_BUSNAME);
}

void QDBusPanelService::registerQDBusService()
{
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SESSION_BUSNAME)
             .registerService(FREEWUBI_PANEL_SERVICENAME))
    {
        return;
    }

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SESSION_BUSNAME)
                        .registerObject(FREEWUBI_PANEL_OBJECTPATH, this,
                                        QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
}

void QDBusPanelService::SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                    bool hasNext, int cursor, int layout)
{
    Q_UNUSED(layout);
    emit signal_SetLookupTable(label, text, attr, hasPrev, hasNext, cursor);
}

void QDBusPanelService::SetSpotRect(int x, int y, int w, int h)
{
    emit signal_SetSpotLocation(x, y, w, h);
}
