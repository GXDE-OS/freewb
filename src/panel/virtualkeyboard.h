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
#include "vklayouts.h"

namespace Ui
{
class VirtualKeyboard;
}

class VirtualKeyboard : public QWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(VirtualKeyboardMode mode = VK_MODE_PC, QWidget *parent = nullptr);
    ~VirtualKeyboard();

signals:
    void signal_custom_key_clicked(VkKey keyIdx, const QString &keName,
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
    void update_customkey_button(VkKey keyIdx, const QString &commChar, const QString &shiftChar);
    void switch_vk(int flg);
    void switch_caps_flg(int capsFlg);

    static int get_caps_flg();

protected:
    void init_keyboard_keygroup();
    void init_fixed_key_value();
    void set_work_mode(VirtualKeyboardMode mode);

    void handle_fixed_keyboard_input_clicked(int keyIdx);
    void handle_custom_keyboard_clicked(VkKey keyIdx, const QString &keyName);

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

    VirtualKeyboardMode m_vkWorkMode;
    QButtonGroup m_btnGroup;          // 键盘所有按钮加入的按钮组

    QChar m_ctrlKeyValue[kVkKeyCount - kVkSymbolKeyCount]; // 固定的控制类型按键值

    bool m_shiftFlag; // 上档标志
    bool m_capsFlag;  // 大写标志

    QPoint m_vkDefaultPos; // 软件盘默认打开位置
};

#endif
