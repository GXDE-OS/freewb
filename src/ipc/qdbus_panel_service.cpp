#include "qdbus_panel_service.h"

namespace freewb::ipc
{

QDBusPanelService::QDBusPanelService(QObject *parent) : QObject(parent) 
{
    registerQDBusService();
}

QDBusPanelService::~QDBusPanelService()
{
    unRegisterQDBusService();
}

void QDBusPanelService::unRegisterQDBusService()
{
    QDBusConnection(FREEWUBI_PANEL_BUSNAME).unregisterObject(FREEWUBI_PANEL_OBJECTPATH);
    QDBusConnection(FREEWUBI_PANEL_BUSNAME).unregisterService(FREEWUBI_PANEL_SERVICENAME);
    QDBusConnection::disconnectFromBus(FREEWUBI_PANEL_BUSNAME);
}

void QDBusPanelService::registerQDBusService()
{
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_PANEL_BUSNAME)
             .registerService(FREEWUBI_PANEL_SERVICENAME))
    {
        return;
    }

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_PANEL_BUSNAME)
        .registerObject(FREEWUBI_PANEL_OBJECTPATH, this,
                        QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
}

void QDBusPanelService::ShowPreedit(bool show)
{
    emit signal_ShowPreedit(show);
}

void QDBusPanelService::ShowAux(bool show)
{
    emit signal_ShowAux(show);
}

void QDBusPanelService::ShowLookupTable(bool show)
{
    emit signal_ShowLookupTable(show);
}

void QDBusPanelService::UpdateLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                          bool hasNext)
{
    emit signal_UpdateLookupTable(label, text, attr, hasPrev, hasNext);
}

void QDBusPanelService::UpdatePreeditCaret(int position)
{
    emit signal_UpdatePreeditCaret(position);
}

void QDBusPanelService::UpdatePreeditText(const QString &text, const QString &attr)
{
    emit signal_UpdatePreeditText(text, attr);
}

void QDBusPanelService::UpdateAux(const QString &text, const QString &attr)
{
    emit signal_UpdateAux(text, attr);
}

void QDBusPanelService::UpdateSpotLocation(int x, int y)
{
    emit signal_UpdateSpotLocation(x, y);
}

void QDBusPanelService::UpdateProperty(const QString &prop)
{
    emit signal_UpdateProperty(prop);
}

void QDBusPanelService::RegisterProperties(const QStringList &props)
{
    emit signal_RegisterProperties(props);
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

void QDBusPanelService::SwitchInputMode(const QString &inputMode)
{
    emit signal_switch_input_mode(inputMode);
}

void QDBusPanelService::SwitchCharSet()
{
    emit signal_switch_char_set();
}

void QDBusPanelService::SwitchSimpOrTrad()
{
    emit signal_switch_simp_or_trad();
}

void QDBusPanelService::SetCharWidthAndMarkMode(int charWidth, int markMode)
{
    emit signal_set_charWidth_and_markMode(charWidth, markMode);
}

} // namespace freewb::ipc