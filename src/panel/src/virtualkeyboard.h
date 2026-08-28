#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QButtonGroup>
#include <QChar>
#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>

#include "settingshelper.h"

namespace Ui
{
class VirtualKeyboard;
}

// 软键盘工作模式
typedef enum
{
    // 用于桌面工具条软键盘输入
    VKM_INPUT_PC = 0,        // ＰＣ键盘输入模式
    VKM_INPUT_GREEK,         // 希腊字母
    VKM_INPUT_RUSSIAN,       // 俄文字母
    VKM_INPUT_PHONETIC,      // 注音符号
    VKM_INPUT_PINYIN,        // 汉语拼音
    VKM_INPUT_JAPAN_FLAT,    // 日文平假名
    VKM_INPUT_JAPAN_PIECE,   // 日文片假名
    VKM_INPUT_PUNCTUATION,   // 标点符号
    VKM_INPUT_DIGITAL_ORDER, // 数字序号
    VKM_INPUT_MATH,          // 数学符号
    VKM_INPUT_UNIT,          // 单位符号
    VKM_INPUT_TABS,          // 制表符
    VKM_INPUT_SPECIAL,       // 特殊符号

    VKM_INPUT_USER_CHAR, // 用户自定义字符输入模式

    // 用于设置界面自定义按键符号
    VKM_CUSTOM_CHAR, // 用户自定义按键字符模式
    VKM_CUSTOM_MARK, // 用户自定义按键标点模式

    VKM_NUM
} VirtualKeyboardMode;

typedef QVector<QString> KeyValue;

class VirtualKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(VirtualKeyboardMode mode = VKM_INPUT_PC, QWidget *parent = nullptr);
    ~VirtualKeyboard();

signals:
    void signal_custom_key_clicked(SymbolKeyIdx keyIdx, const QString &keName,
                                   const CustomKeyValue &keyValue); // 用于设置界面软键盘按键自定义
    void signal_vk_mode_changed(VirtualKeyboardMode mode);
    void signal_kb_caps_changed(int capsFlg);

public slots:
    void slot_load_setting_data();
    void slot_open_win(VirtualKeyboardMode mode);
    void slot_key_clicked(int keyCode);

public:
    void openWin();  // 以当前模式打开虚拟键盘
    void closeWin(); // 关闭虚拟键盘
    void update_keyboard_button();
    void update_customkey_button(SymbolKeyIdx keyIdx, const QString &commChar, const QString &shiftChar);
    void switch_vk(int flg);
    void switch_caps_flg(int capsFlg);

    static int get_caps_flg();

protected:
    void init_keyboard_keygroup();
    void init_fixed_key_value();
    void set_work_mode(VirtualKeyboardMode mode);

    void handle_fixed_keyboard_input_clicked(int keyIdx);
    void handle_userChar_keyboard_input_clicked(int keyIdx);
    void handle_custom_keyboard_clicked(SymbolKeyIdx keyIdx, const QString &keyName);

    void update_caps_flg();
    void update_shift_flg(int keyIdx);

    // 重载函数
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void slot_virtual_keyboard_clicked(int idx);

private:
    Ui::VirtualKeyboard *ui;
    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    QPoint m_mouseLastPosition;

    VirtualKeyboardMode m_vkWorkMode; // 虚拟键盘工作模式
    QButtonGroup m_btnGroup;          // 键盘所有按钮加入的按钮组

    QChar m_ctrlKeyValue[KEY_ALL_NUM - KEY_SYMBOL_NUM]; // 固定的控制类型按键值
    QVector<KeyValue> *m_fixedKeyValue;
    QVector<KeyValue> m_pcKeyValue;           // PC键盘符号
    QVector<KeyValue> m_greekKeyValue;        // 希腊字母
    QVector<KeyValue> m_russianKeyValue;      // 俄文字母
    QVector<KeyValue> m_phoneticKeyValue;     // 注音符号
    QVector<KeyValue> m_pinyinKeyValue;       // 汉语拼音
    QVector<KeyValue> m_japanFlatKeyValue;    // 日文平假
    QVector<KeyValue> m_japanPieceKeyValue;   // 日文片假
    QVector<KeyValue> m_punctuationKeyValue;  // 标点符号
    QVector<KeyValue> m_digitalOrderKeyValue; // 数字序号
    QVector<KeyValue> m_mathKeyValue;         // 数学符号
    QVector<KeyValue> m_unitKeyValue;         // 单位符号
    QVector<KeyValue> m_tabsKeyValue;         // 制表符号
    QVector<KeyValue> m_specialKeyValue;      // 特殊符号

    bool m_shiftFlag; // 上档标志
    bool m_capsFlag;  // 大写标志

    QPoint m_vkDefaultPos; // 软件盘默认打开位置
};

#endif
