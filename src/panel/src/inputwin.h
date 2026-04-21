/****************************************************************************************
** @作者：lcj
**
** @说明：
**      InputWin是一个从QWidget类继承而来的UI类，其对应的UI设计文件为inputwin.ui,该类设计作为文
**      字输入显示面板被MainProgram类所包含实例化，其主要包含了一个QTableWidget部件为用户提供候选
**      词组等相关信息。
******************************************************************************×*********/

#ifndef INPUTWIN_H
#define INPUTWIN_H

#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTableWidgetItem>
#include <QTimer>
#include <QWidget>

#include "candidateitem.h"
#include "contextmenu.h"
#include "dictquery.h"
#include "kimagent.h"
#include "settingshelper.h"

namespace Ui
{
class InputWin;
}

// 操作提示信息项
typedef enum
{
    OTI_LOGO,
    OTI_MOUSE_MENU,
    OTI_DELETE_WORD,
    OTI_ADJUST_WORD,
    OTI_SK_SWITCH_CHAR_WIDTH,
    OTI_SK_SWITCH_MARK,

    OTI_SK_BACK_FIND_CODE,    // 反查编码
    OTI_SK_ONLINE_ADD_WORD,   // 在线加词
    OTI_SK_ONLINE_DEL_WORD,   // 在线删词
    OTI_SK_SWITCH_KEYBOARD,   // 切换软键盘
    OTI_SK_SWITCH_CHAR_SET,   // 切换字符集
    OTI_SK_SWITCH_INPUT_MODE, // 切换输入模式
    //    OTI_SK_SWITCH_WORD_STATE,//切换字词状态
    OTI_SK_SWITCH_S_IN_T_OUT,       // 切换简入繁出
    OTI_SK_SHOW_HIDE_STATUS_BAR,    // 显示/隐藏状态栏
    OTI_SK_SHOW_HIDE_CANDIDATE_WIN, // 显示/隐藏候选窗
    OTI_SK_SWITCH_WORD_LEXICON,     // 切换词库
    //    OTI_SK_ADD_CHAR_AFTER_OUTPUT,//输出项后加字符
    OTI_SK_SWITCH_SKIN,           // 切换皮肤
    OTI_SK_QUICK_DEL_SCREEN_CHAR, // 快删上屏项
    OTI_SK_MARK_AUTO_PAIR,        // 标点自动配对

    OTI_SK_TEMP_ENGLISH,
    OTI_SK_QUICK_INPUT,
    OTI_SK_TEMP_PINYIN,
    OTI_SK_SWITCH_CN_EN,
    OTI_SK_RECODE_SELECT,
    OTI_SK_CANDI_PAGE,

    OTI_RANDOM_NUM,

    OTI_TEMP_ENGLISH,
    OTI_QUICK_INPUT,
    OTI_TEMP_PINYIN,
    OTI_TEMP_ENGLISH_SELECT,
    OTI_TEMP_ENGLISH_EMPTY,

    OTI_TOTAL_NUM
} OpTipsInfo;

// 文字输入框界面皮肤配置数据
typedef struct
{
    int bgTopImageHeight;      // 顶部背景图片高度
    int bgBottomImageHeight;   //
    int bgLeftImageWidth;      //
    int bgRightImageWidth;     //
    QString bgCenterImagePath; // 中心背景图片路径
    QString bgTopImagePath;    //
    QString bgBottomImagePath; //
    QString bgLeftImagePath;   //
    QString bgRightImagePath;  //
    QString fullIcoPath;       //
    QString halfIcoPath;       //
    QString cnMarkIcoPath;     //
    QString enMarkIcoPath;     //
    QString prev0PageIcoPath;  //
    QString prev1PageIcoPath;  //
    QString next0PageIcoPath;  //
    QString next1PageIcoPath;  //
    QString logoIcoPath;       //
} SkinInputWin;

class InputWin : public QWidget
{
    Q_OBJECT

public:
    explicit InputWin(QWidget *parent = nullptr);
    ~InputWin();

signals:
    void signal_candidate_page_up();       // 候选词上一页(发送给fcitx)
    void signal_candidate_page_down();     // 候选词下一页(发送给fcitx)
    void signal_candidate_select(int idx); // 候选词选择(发送给fcitx)
    void signal_btn_charWidth_clicked();   //
    void signal_btn_mark_clicked();        //
    void signal_open_context_menu();

public slots:
    void slot_load_setting_data();
    void slot_load_skin(const QString &skinId);
    void slot_update_charWidth_btn_ico();
    void slot_update_mark_btn_ico();

    // 以下槽函数被fcitx所发出的信号连接
    void slot_kim_ShowPreedit(bool);
    void slot_kim_ShowAux(bool);
    void slot_kim_ShowLookupTable(bool);
    void slot_kim_UpdateLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool);
    void slot_kim_SetLookupTable(const QStringList &, const QStringList &, const QStringList &, bool, bool, int);
    void slot_kim_UpdatePreeditCaret(int);
    void slot_kim_UpdatePreeditText(const QString &, const QString &);
    void slot_kim_UpdateAux(const QString &, const QString &);
    void slot_kim_UpdateSpotLocation(int, int);
    void slot_kim_SetSpotLocation(int, int, int, int);

public:
    void show_user_word_operation_prompt(int addOrDel, const QString &wordText, const QString &wordCode);
    void close_user_word_operation_prompt();

protected:
    void init_ui_table();
    void install_evt_filter();
    void init_im_prompt_lable();

    void set_display_mode(CandiWinDispMode mode);
    void enter_user_word_mode();
    void exit_user_word_mode();

    void set_candidate_text(int idx, const QString &label, const QString &wordText, const QString &promptText);
    void clear_candidate_text(int idx);

    void handle_candiwin_op_help_info();
    void set_candiwin_op_help_info();

    void update_skin();
    void adjust_candi_win_height();
    void adjust_candi_win_width();
    void auto_adjust_candi_win_geometry();

    void show_dict_find_win();

    bool eventFilter(QObject *obj, QEvent *event);

protected slots:
    void slot_dict_find(const QString &wordText);
    void slot_btnPrevPage_clicked();
    void slot_btnNextPage_clicked();
    void slot_caret_blink();

private slots:
    void on_btnCharWidth_clicked();
    void on_btnMark_clicked();
    void on_tableWidget_cellClicked(int row, int column);

private:
    Ui::InputWin *ui;

    // 桌面分辨率
    QSize m_desktopSize;

    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    bool m_mouseMoveFlag;
    QPoint m_mouseLastPosition;

    QPoint m_defaultPosition;

    // 切换输入法时在光标处的提示信息
    QLabel *m_labelImPrompt;

    // 输入框中光标的位置
    QPoint m_imPromptPosition;

    QString m_preEidtText; // 预编辑框中的输入法引擎发送的原始数据
    int m_candiWordCount;  // 输入法引擎发送的实际候选词个数

    int m_caretPos;
    int m_caretPhase;
    QTimer m_caretBlinkTimer;

    bool m_isUserWordMode;

    QPushButton *m_btnPrevPage;
    QPushButton *m_btnNextPage;

    int m_winWidth;
    int m_winHeight; // 根据候选框的显示模式、候选词个数、显示字体确定的候选框固定高度

    // 皮肤配置数据
    SkinInputWin m_skinData;

    // 实时字典查询
    DictQuery m_dictquery;
    QWidget m_dictFindWin;
    QLabel *m_dictFindLabel;

    /******************* 设置界面可配置数据 ***********************/
    CandiWinDispMode m_displayMode; // 显示模式
    QString m_curSkinId;            // 当前使用的皮肤
    char m_separateChar;            // 候选序号与候选词之间的分隔符
    int m_radius;                   // 候选框圆角弧度
    int m_transparency;             // 候选框透明度
    int m_candiWordItem;            // 候选框显示的候选词行数
    int m_candiCharCount;           // 候选词字数
    bool m_cursorFollow;            // 是否光标跟随
    bool m_hideCandiWin;            // 是否隐藏候选框
    bool m_showOpRemindInfo;        // 是否显示操作提示信息
};

#endif
