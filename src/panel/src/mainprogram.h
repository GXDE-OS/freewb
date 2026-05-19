/********************************************************************************************
** @作者：lcj
**
** @说明：
**      MainProgram是一个从QObject类继承而来的非UI类，该类设计目的为作为输入法启动后的后台守护服务进
**      程主类，主要对其所包含的子类成员进行管理与功能调度，其主要包含实例化的类有与fcitx通信的KinAgent类
** 　　　和ToolbarWin类,InputWin类,SettingWin类,lexicontoolwin类,ContextMenu类,UsrGenWordDialog
**      类,Keyboard类,DictQueryWin类．
*****************************************************************************************×**/

#ifndef MAINPROGRAM_H
#define MAINPROGRAM_H

#include <QAbstractButton>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMessageBox>
#include <QObject>
#include <QSystemTrayIcon>
#include <QWidget>

#include "backupdialog.h"
#include "contextmenu.h"
#include "dictquerywin.h"
#include "inputwin.h"
#include "keyboard.h"
#include "qdbus_panel.h"
#include "lexicontoolwin.h"
#include "settingshelper.h"
#include "settingwin.h"
#include "texteditwin.h"
#include "toolbarwin.h"
#include "usrgenworddialog.h"
#include "x11eventmonitor.h"

#include "ipc.h"

class MainProgram : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", FREEWUBI_SETTINGS_INTERFACE)

public:
    MainProgram(QObject *parent = nullptr);
    ~MainProgram();

    void create_host_dbus_service();

public slots:                                              // 提供给外部进程调用的DBUS方法接口
    void slot_switch_input_mode(const QString &inputMode); // 按输入模式切换子输入法
    void slot_dbus_dict_query(const QString &text);        // 字典查询
    void slot_dbus_generate_usr_word(int flg, const QString &wordText, const QString &wordCode); // 用户造词
    void slot_dbus_delete_usr_word(int flg, const QString &wordText, const QString &wordCode);   // 用户删词
    void slot_dbus_usr_word_load_ok();                                                           // 用户词组加载完成
    void slot_dbus_quick_table_load_ok();                                                        // 快捷码表加载完成
    void slot_dbus_panel_exit();                                                                 // 前端面板退出
    void slot_dbus_switch_vk(int flg);                                                           // 切换软键盘
    void slot_dbus_close_vk();                                                                   // 退出软键盘
    void slot_dbus_switch_char_set();                                                            // 切换字符集
    void slot_dbus_switch_simp_or_trad();                                                        // 切换简/繁体
    void slot_dbus_switch_toolbar_hide_flg();                                                    // 显/隐状态栏
    void slot_dbus_switch_candiwin_hide_flg();                                                   // 显/隐候选框
    void slot_dbus_switch_lexicon();                                                             // 切换词库
    void slot_dbus_switch_skin();                                                                // 切换皮肤
    void slot_dbus_set_mark_auto_pairs_flg(int flg);                                             // 标点自动配对
    void slot_dbus_ime_table_load_ok();                                                          // 输入法词库加载完成
    void slot_dbus_switch_caps_state();                                                          // 切换大小写状态

    void slot_dbus_set_charWidth_and_markMode(int charWidth, int markMode);
    QString slot_dbus_get_clipboard_text(); // 获取系统粘贴板内容

    // 快捷命令
    void slot_dbus_open_freewb_dir();                                     // 进入极点目录
    void slot_dbus_word_freq_switch_ok(const QString &wordText, int flg); // 词组常用与非常用切换成功
    void slot_dbus_open_ui_setting();                                     // 进入设置界面
    void slot_dbus_open_advanced_setting();                               // 进入专家设置模式
    void slot_dbus_edit_usr_table();                                      // 编辑用户码表
    void slot_dbus_edit_wubi_table();                                     // 编辑五笔码表
    void slot_dbus_edit_pinyin_table();                                   // 编辑拼音码表
    void slot_dbus_show_version_info();                                   // 显示极点版本信息
    void slot_dbus_edit_quick_table();                                    // 编辑快捷码表
    void slot_dbus_set_recode_calib_flg(int flg);                         // 切换重码上屏校对模式

protected slots:
    void slot_delete_freewb_panel();

private:
    X11EventMonitor *m_x11EventMonitor; // X11系统事件监视器

    QDBusPanelService *m_panelDBusService; // 与输入法引擎的 D-Bus 桥接

    ContextMenu *m_contextmenu;           // 桌面右键菜单
    ToolbarWin *m_toolbar;                // 桌面工具条
    Keyboard *m_virtualKeyboard;          // 虚拟键盘
    InputWin *m_inputWin;                 // 输入候选框
    SettingWin *m_settingWin;             // 设置窗口
    LexiconToolWin *m_lexicontoolWin;     // 词库工具箱
    UsrGenWordDialog *m_usrGenWordDialog; // 用户词组编辑对话框
    TextEditWin *m_textEditWin;           // 文本编辑框
    DictQueryWin *m_dictQueryWin;         // 字典查询窗口
    BackupDialog *m_backupDialog;         // 用户词库与设置备份/恢复窗口
};

#endif
