/********************************************************************************************
** @说明：输入法 Panel 守护进程主类，管理子窗口与 D-Bus 接线。
********************************************************************************************/

#ifndef MAINPROGRAM_H
#define MAINPROGRAM_H

#include <QObject>

#include "backupdialog.h"
#include "contextmenu.h"
#include "dictquerywin.h"
#include "inputwin.h"
#include "keyboard.h"
#include "lexicontoolwin.h"
#include "qdbus_panel_service.h"
#include "qdbus_settings_service.h"
#include "settingwin.h"
#include "texteditwin.h"
#include "toolbarwin.h"
#include "usrgenworddialog.h"
#include "x11eventmonitor.h"

class MainProgram : public QObject
{
    Q_OBJECT

public:
    MainProgram(QObject *parent = nullptr);
    ~MainProgram() override;

private:
    void connectPanelDBus();
    void connectSettingsDBus();

    X11EventMonitor *m_x11EventMonitor = nullptr;
    freewb::ipc::QDBusPanelService *m_panelDBusService = nullptr;
    freewb::ipc::QDBusSettingsService *m_settingsDBusService = nullptr;

    ContextMenu *m_contextmenu = nullptr;
    ToolbarWin *m_toolbar = nullptr;
    Keyboard *m_virtualKeyboard = nullptr;
    InputWin *m_inputWin = nullptr;
    SettingWin *m_settingWin = nullptr;
    LexiconToolWin *m_lexicontoolWin = nullptr;
    UsrGenWordDialog *m_usrGenWordDialog = nullptr;
    TextEditWin *m_textEditWin = nullptr;
    DictQueryWin *m_dictQueryWin = nullptr;
    BackupDialog *m_backupDialog = nullptr;
};

#endif
