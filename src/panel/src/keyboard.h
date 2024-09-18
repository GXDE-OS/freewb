/****************************************************************************************
** @作者：lcj
**
** @说明：
**      Keyboard是一个从QWidget类继承而来的UI类，其对应的UI设计文件为keyboard.ui，该类设计目的为被
**      ToolBar类与SettingWin类所包含实例化，在桌面工具条上点击软键盘按钮时弹出该虚拟键盘为用户提供输
**      入功能，在自定义按键字符设置界面时显示该键盘为用户提供自定义功能。
*************************************************************************************×**/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QChar>
#include <QButtonGroup>
#include <QFile>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>


namespace Ui {
class Keyboard;
}



//软键盘工作模式
typedef enum
{
    //用于桌面工具条软键盘输入
    VKM_INPUT_PC = 0,//ＰＣ键盘输入模式
    VKM_INPUT_GREEK,//希腊字母
    VKM_INPUT_RUSSIAN,//俄文字母
    VKM_INPUT_PHONETIC,//注音符号
    VKM_INPUT_PINYIN,//汉语拼音
    VKM_INPUT_JAPAN_FLAT,//日文平假名
    VKM_INPUT_JAPAN_PIECE,//日文片假名
    VKM_INPUT_PUNCTUATION,//标点符号
    VKM_INPUT_DIGITAL_ORDER,//数字序号
    VKM_INPUT_MATH,//数学符号
    VKM_INPUT_UNIT,//单位符号
    VKM_INPUT_TABS,//制表符
    VKM_INPUT_SPECIAL,//特殊符号

    VKM_INPUT_USER_CHAR,//用户自定义字符输入模式

    //用于设置界面自定义按键符号
    VKM_CUSTOM_CHAR,//用户自定义按键字符模式
    VKM_CUSTOM_MARK,//用户自定义按键标点模式

    VKM_NUM
}VirtualKeyboardMode;


//用户可自定义的按键，具有上档功能
typedef enum
{
    KEY_0 = 0, //按钮组控件需要用到该枚举类型，必须从0开始连续分配数值
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_BACKQUOTE,//反引号
    KEY_SUB,//减号
    KEY_EQUAL,//等号
    KEY_LEFT_BRACKET,//左中括号
    KEY_RIGHT_BRACKET,//右中括号
    KEY_BACKSLASH,//反斜杠
    KEY_SEMICOLON,//分号
    KEY_QUOTE,//引号
    KEY_COMMA,//逗号
    KEY_PERIOD,//句号
    KEY_SLASH,//斜杠

    KEY_SYMBOL_NUM
}SymbolKeyIdx;


//用户可不可自定义的控制类型按键，没有上档功能
typedef enum
{
    KEY_BACKSAPCE = KEY_SYMBOL_NUM,//退格
    KEY_TAB,//制表符
    KEY_CAPS,//大小写
    KEY_ENTER,//Enter
    KEY_SHIFT,//Shift
    KEY_INSERT,//插入
    KEY_DEL,//删除
    KEY_SPACE,//空格
    KEY_ESC,//Esc

    KEY_ALL_NUM//虚拟键盘上的按键数目
}CtrlKeyIdx;



typedef struct
{
    QString commChar;//自定义正常字符
    QString shiftChar;//自定义上档字符
    QString commMark;//自定义正常标点
    QString shiftMark;//自定义上档标点
}CustomKeyValue;


typedef QVector<QString> KeyValue;


class Keyboard : public QWidget
{
    Q_OBJECT

public:
    explicit Keyboard( VirtualKeyboardMode mode = VKM_INPUT_PC, QWidget *parent = nullptr );
    ~Keyboard();

signals:
    void signal_custom_key_clicked( SymbolKeyIdx keyIdx, const QString &keName, const CustomKeyValue &keyValue );//用于设置界面软键盘按键自定义
    void signal_vk_mode_changed( VirtualKeyboardMode mode );
    void signal_vk_flg_changed();
    void signal_kb_caps_changed( int capsFlg );

public slots:
    void slot_load_setting_data();
    void slot_toggle_win();
    void slot_open_win( VirtualKeyboardMode mode );
    void slot_key_clicked( int keyCode );

public:
    void update_keyboard_button();
    void update_customkey_button( SymbolKeyIdx keyIdx, const QString &commChar, const QString &shiftChar );
    void switch_vk( int flg );
    void switch_caps_flg( int capsFlg );

    static int get_caps_flg();

protected:
    void init_keyboard_keygroup();
    void init_fixed_key_value();
    void set_work_mode( VirtualKeyboardMode mode );

    void handle_fixed_keyboard_input_clicked( int keyIdx );
    void handle_userChar_keyboard_input_clicked( int keyIdx );
    void handle_custom_keyboard_clicked( SymbolKeyIdx keyIdx, const QString &keyName );

    void update_caps_flg();
    void update_shift_flg( int keyIdx );

    //重载函数
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    bool eventFilter( QObject *obj, QEvent *event );

private slots:
    void slot_virtual_keyboard_clicked( int idx );

private:
    Ui::Keyboard *ui;
    //用于窗口拖动计算
    bool  m_mouseIsPressed;
    QPoint m_mouseLastPosition;

    VirtualKeyboardMode m_vkWorkMode;//虚拟键盘工作模式
    QButtonGroup m_btnGroup;//键盘所有按钮加入的按钮组

    QChar m_ctrlKeyValue[KEY_ALL_NUM-KEY_SYMBOL_NUM];//固定的控制类型按键值
    QVector<KeyValue> *m_fixedKeyValue;
    QVector<KeyValue> m_pcKeyValue;//PC键盘符号
    QVector<KeyValue> m_greekKeyValue;//希腊字母
    QVector<KeyValue> m_russianKeyValue;//俄文字母
    QVector<KeyValue> m_phoneticKeyValue;//注音符号
    QVector<KeyValue> m_pinyinKeyValue;//汉语拼音
    QVector<KeyValue> m_japanFlatKeyValue;//日文平假
    QVector<KeyValue> m_japanPieceKeyValue;//日文片假
    QVector<KeyValue> m_punctuationKeyValue;//标点符号
    QVector<KeyValue> m_digitalOrderKeyValue;//数字序号
    QVector<KeyValue> m_mathKeyValue;//数学符号
    QVector<KeyValue> m_unitKeyValue;//单位符号
    QVector<KeyValue> m_tabsKeyValue;//制表符号
    QVector<KeyValue> m_specialKeyValue;//特殊符号

    bool m_shiftFlag;//上档标志
    bool m_capsFlag;//大写标志

    QPoint m_vkDefaultPos;//软件盘默认打开位置
};


#endif




