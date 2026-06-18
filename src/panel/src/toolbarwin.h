#ifndef TOOLBARWIN_H
#define TOOLBARWIN_H

#include <string>

#include <QActionGroup>
#include <QDesktopWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPoint>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QWidget>

#include "contextmenu.h"
#include "keyboard.h"
#include "settingshelper.h"

namespace Ui
{
class ToolbarWin;
}

typedef enum
{
    WIDTH_FULL, // 全角
    WIDTH_HALF  // 半角
} CharWidthMode;

typedef enum
{
    MARK_CN, // 中文标点
    MARK_EN  // 英文标点
} MarkMode;

typedef enum
{
    CHAR_GB, // GB字符集
    CHAR_GBK // GBK字符集
} CharSetMode;

// 工具条界面按钮-LOGO按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString icoPath;
} SkinToolBarLogoBtn;

// 工具条界面按钮-扩展菜单按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString openIcoPath;
    QString closeIcoPath;
} SkinToolBarMenuExtendBtn;

// 工具条界面按钮-输入模式按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString wbFontIcoPath;
    QString wbPinyinIcoPath;
    QString stdPinyinIcoPath;
    QString englishIcoPath;
    QString capsIcoPath;
} SkinToolBarModeBtn;

// 工具条界面按钮-全半角按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString fullIcoPath;
    QString halfIcoPath;
} SkinToolBarFullHalfBtn;

// 工具条界面按钮-中英文标点切换按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString cnMarkIcoPath;
    QString enMarkIcoPath;
} SkinToolBarCnEnMarkBtn;

// 工具条界面按钮-设置按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString icoPath;
} SkinToolBarSettingBtn;

// 工具条界面按钮-造词按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString icoPath;
} SkinToolBarGenerateBtn;

// 工具条界面按钮-搜索按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString icoPath;
} SkinToolBarSearchBtn;

// 工具条界面按钮-简体繁体切换按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString simpIcoPath;
    QString tradIcoPath;
} SkinToolBarCharFontBtn;

// 工具条界面按钮-字符集切换按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString gbIcoPath;
    QString gbkIcoPath;
} SkinToolBarCharSetBtn;

// 工具条界面按钮-虚拟键盘按钮
typedef struct
{
    bool isExist;
    QRect rect;
    QString icoPath;
} SkinToolBarKeyboardBtn;

// 工具条界面配置数据
typedef struct
{
    QSize size0;
    QSize size1;
    QString bg0ImagePath;
    QString bg1ImagePath;

    SkinToolBarLogoBtn stbLogoBtn;
    SkinToolBarMenuExtendBtn stbMenuExtendBtn;
    SkinToolBarModeBtn stbModeBtn;
    SkinToolBarFullHalfBtn stbFullHalfBtn;
    SkinToolBarCnEnMarkBtn stbCnEnMarkBtn;
    SkinToolBarSettingBtn stbSettingBtn;
    SkinToolBarGenerateBtn stbGenerateBtn;
    SkinToolBarSearchBtn stbSearchBtn;
    SkinToolBarCharFontBtn stbCharFontBtn;
    SkinToolBarCharSetBtn stbCharSetBtn;
    SkinToolBarKeyboardBtn stbKeyboardBtn;
} SkinToolBar;

class ToolbarWin : public QWidget
{
    Q_OBJECT

public:
    static constexpr const char *kEngineWbzx = "engine:wbzx";
    static constexpr const char *kEngineWbpy = "engine:wbpy";
    static constexpr const char *kEnginePy = "engine:py";
    static constexpr const char *kEngineEn = "engine:en";

    explicit ToolbarWin(QWidget *parent = nullptr);
    ~ToolbarWin();

signals:
    void signal_open_setting_win();
    void signal_open_generate_word_dialog(const QString &wordText, const QString &wordCode);
    void signal_open_context_menu();
    void signal_toggle_vk();
    void signal_open_vk(VirtualKeyboardMode mode);
    void signal_open_dict_query_win(const QString &queryText);
    void signal_switch_char_set();
    void signal_switch_chttrans();
    void signal_traditional_mode_changed(bool isTraditional);

    // 以下信号发给输入面板
    void signal_btn_charWidth_clicked();
    void signal_btn_mark_clicked();

    // 以下信号发给fcitx
    void signal_request_next_input_mode();
    void signal_fcitx_switch_char_width(const QString &param);
    void signal_fcitx_switch_mark(const QString &param);

public slots:
    void slot_load_setting_data();
    void slot_load_skin(const QString &skinId);
    void slot_update_input_mode_ico();
    void slot_update_char_width_mode_ico();
    void slot_update_mark_mode_ico();
    void slot_set_traditional_mode(bool isTraditional);

    void slot_kim_UpdateProperty(const QString &prop);
    void slot_kim_RegisterProperties(const QStringList &prop);

    void slot_vk_mode_changed(VirtualKeyboardMode vkMode);
    void slot_kb_caps_changed(int capsFlag);

    void slot_context_menu_visibility_changed(bool visible);

    void on_btnCharWidth_clicked();
    void on_btnMark_clicked();

public:
    void switch_char_set();
    void set_traditional_mode(bool isTraditional);
    void update_char_width_mode_ico(CharWidthMode charWidth);
    void update_mark_mode_ico(MarkMode markMode);
    void update_char_font_ico();
    void set_context_menu(ContextMenu *contextMenu);
    void reset();
    void hide();

public:
    // 静态成员函数
    static void set_input_mode(const QString &inputMode);
    static const QString &get_input_mode();
    static void set_char_width_mode(CharWidthMode charMode);
    static CharWidthMode get_char_width_mode();
    static void set_mark_mode(MarkMode markMode);
    static MarkMode get_mark_mode();
    static bool is_traditional_mode();
    static void switch_char_set_mode();
    static CharSetMode get_char_set_mode();

protected:
    void install_evt_filter();
    void update_mouse_hover_tips();
    void show_mouse_hover_tips(QWidget *widget);
    void show_keyboard_menu();
    void move_toolbar(QPoint pos);

    void update_skin();
    void update_extend_menu(bool state);
    void update_toolbar_bg();
    void update_extend_menu_ico();
    void update_mark_mode_ico();
    void update_char_set_ico();
    void update_vk_mode_ckecked_state(VirtualKeyboardMode mode);

    void fcitx_charFont_updated(const QString &param);
    void fcitx_charWidth_updated(const QString &param);
    void fcitx_charMark_updated(const QString &param);

    bool eventFilter(QObject *obj, QEvent *event);
    bool is_panel_menu_visible() const;

protected slots:
    void slot_hide_toolbar();
    void slot_vk_mode_triggered(QAction *action);

private slots:
    void on_btnMenuExtend_clicked();
    void on_btnLogo_clicked();
    void on_btnMode_clicked();
    void on_btnGenerate_clicked();
    void on_btnSearch_clicked();
    void on_btnKeyboard_clicked();
    void on_btnSetting_clicked();
    void on_btnCharFont_clicked();
    void on_btnCharSet_clicked();
    void slot_apply_pending_kim_property();

private:
    // 静态数据成员
    static QString s_inputMode;           // 输入模式
    static CharWidthMode s_charWidthMode; // 字符宽度
    static MarkMode s_markMode;           // 标点模式
    static bool s_isTraditionalMode;      // 简体/繁体：true=繁体
    static CharSetMode s_charSetMode;     // 字符集
    static int s_capsFlg;

private:
    Ui::ToolbarWin *ui;

    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    bool m_mouseMoveFlag;
    QPoint m_mouseLastPosition;

    QSize m_desktopSize;
    // 工具条默认显示位置
    QPoint m_defaultPosition;
    // 扩展菜单是否打开
    bool m_extendMenuOpenState;

    QTimer m_hideDelayTimer;
    /** UpdateProperty 防抖：焦点切换时可能连发多条，合并后再决定工具条显隐。 */
    QTimer m_kimPropertyDebounceTimer;
    QString m_pendingKimProperty;
    bool m_contextMenuVisible = false;
    bool m_keyboardMenuVisible = false;

    QWidget m_tooltipsWin;
    QLabel *m_tooltipsLabel;
    bool m_tooltipsWinShowFlg;

    QMenu m_keyboardMenu; // 虚拟键盘工作模式菜单
    QActionGroup *m_kbInputModeAction;
    QAction *m_kbPcInputMode;       // PC键盘输入模式
    QAction *m_kbInputGreek;        // 希腊字母输入模式
    QAction *m_kbInputRussian;      // 俄文字母输入模式
    QAction *m_kbInputPhonetic;     // 注音符号输入模式
    QAction *m_kbInputPinyin;       // 汉语拼音输入模式
    QAction *m_kbInputJapanFlat;    // 日文平假名输入模式
    QAction *m_kbInputJapanPiece;   // 日文片假名输入模式
    QAction *m_kbInputPunctuation;  // 标点符号输入模式
    QAction *m_kbInputDigitalOrder; // 数字序号输入模式
    QAction *m_kbInputMath;         // 数学符号输入模式
    QAction *m_kbInputUnit;         // 单位符号输入模式
    QAction *m_kbInputTabs;         // 制表符输入模式
    QAction *m_kbInputSpecial;      // 特殊符号输入模式
    QAction *m_kbUserCharInputMode; // 用户自定义字符模式

    SkinToolBar m_skinData; // 皮肤配置数据

    /******************* 设置界面可配置数据 ***********************/
    QString m_curSkinId = "9999"; // 当前使用皮肤
    bool m_autoLocate;            // 工具条自动定位
    bool m_autoMenuExpand;        // 工具条菜单自动扩展
    bool m_useUiAudioEffect;      // 是否使用界面音效
    bool m_showRealtimeHelp;      // 是否显示实时提示
    int m_transparency;           // 工具条透明度
};

#endif
