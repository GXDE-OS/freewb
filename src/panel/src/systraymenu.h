/*****************************************************************************************
** @作者：lcj
**
** @说明：
**      SysTrayMenu是一个从QSystemTrayIcon类继承而来的UI类，起设计目的为替输入法在系统任务栏提供一个
** 　　　托盘菜单．
****************************************************************************************/


#ifndef SYSTRAYMENU_H
#define SYSTRAYMENU_H

#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QMutex>

#include "keyboard.h"
#include "toolbarwin.h"


struct FcitxImInfo
{
    QString imId;
    QString imName;
    QString imIconName;
    QString imTips;
};

class SysTrayMenu : public QSystemTrayIcon
{
    Q_OBJECT

public:
    SysTrayMenu( QObject *parent = nullptr );
    ~SysTrayMenu();

signals:
    void signal_create_freewb_panel();
    void signal_delete_freewb_panel();
    void signal_open_setting_win();//打开设置窗口
    void signal_vk_toggle();//开关虚拟键盘
    void signal_open_vk( VirtualKeyboardMode vkMode );//打开虚拟键盘
    void signal_load_skin( const QString &skinId );//载入皮肤
    void signal_switch_char_font( CharFontMode charFontMode );

    // 以下信号发给fcitx
    void signal_fcitx_get_all_ims( const QString &param );
    void signal_fcitx_switch_inputmethod( const QString &param );
    void signal_fcitx_switch_inputmethod_1();
    void signal_fcitx_switch_char_font( const QString &param );
    void signal_fcitx_switch_char_width( const QString &param );
    void signal_fcitx_switch_mark( const QString &param );
    void signal_fcitx_reload_config();
    void signal_fcitx_config();

public slots:
    void slot_load_setting_data();
    void slot_update_skin_list();
    void slot_update_input_mode_ico();
    void slot_update_char_font_ico( CharFontMode mode );
    void slot_vk_mode_changed( VirtualKeyboardMode vkMode );

    void slot_kim_ExecMenu( const QStringList & );
    void slot_kim_UpdateProperty( const QString & );
    void slot_kim_RegisterProperties( const QStringList & );

    void slot_systemtray_actived( QSystemTrayIcon::ActivationReason );

public:
    void update_char_width_mode_ico( CharWidthMode charWidth );
    void update_mark_mode_ico( MarkMode markMode );

protected:
    void fcitx_inputmethod_updated( const QString &param );
    void fcitx_charFont_updated( const QString &param );
    void fcitx_charWidth_updated( const QString &param );
    void fcitx_charMark_updated( const QString &param );

    void update_vk_mode_ckecked_state( VirtualKeyboardMode vkMode );

private slots:
    void slot_char_switch_triggered( QAction *action );
    void slot_vk_switch_triggered();
    void slot_vk_mode_triggered( QAction *action );
    void slot_skin_switch_triggered( QAction *action );
    void slot_setting_triggered();
    void slot_fcitx_config_triggered();

public:
    static bool s_externImFlg;
    static bool is_extern_im();
    static void set_extern_im( bool flg );

private:
    QMap<QString, FcitxImInfo> m_fcitxImMap;

    QMenu *m_mainMenu;//主菜单

    QActionGroup *m_actGrpIm;//输入法菜单组
    QAction *m_actImWbFont;//五笔字型
    QAction *m_actImWbpy;//五笔拼音
    QAction *m_actImPinyin;//标准拼音
    QAction *m_actImEnglish;//英文

    QActionGroup *m_actGrpChar;//字符设置组
    QAction *m_actCharWidth;//字符宽度
    QAction *m_actMarkMode;//字符标点
    QAction *m_actCharFont;//字符集

    QAction *m_actVkSwitch;//虚拟键盘开关
    QMenu *m_menuVkMode;//虚拟键盘输入模式选择菜单
    QActionGroup *m_actGrpVk;//虚拟键盘输入模式组
    QAction *m_actVkPcMode;//PC键盘输入
    QAction *m_actVkGreek;//希腊字母输入模式
    QAction *m_actVkRussian;//俄文字母输入模式
    QAction *m_actVkPhonetic;//注音符号输入模式
    QAction *m_actVkPinyin;//汉语拼音输入模式
    QAction *m_actVkJapanFlat;//日文平假名输入模式
    QAction *m_actVkJapanPiece;//日文片假名输入模式
    QAction *m_actVkPunctuation;//标点符号输入模式
    QAction *m_actVkDigitalOrder;//数字序号输入模式
    QAction *m_actVkMath;//数学符号输入模式
    QAction *m_actVkUnit;//单位符号输入模式
    QAction *m_actVkTabs;//制表符输入模式
    QAction *m_actVkSpecial;//特殊符号输入模式
    QAction *m_actVkCustomCharMode;//用户字符输入模式

    int m_curDispIm;

    QMenu *m_menuSkin;//皮肤选择菜单
    QActionGroup *m_actGrpSkin;//皮肤列表组

    QAction *m_actSetting;//设置

    QAction *m_actFcitx;//FCITX设置

    QString m_curSkinId;

    bool m_panelIsCreated;
};

#endif
