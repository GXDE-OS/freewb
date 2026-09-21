#include "virtualkeyboard.h"

#include <QDesktopWidget>

#include "config.h"
#include "keybutton.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "ui_virtualkeyboard.h"
#include "vklayouts.h"

/*
 * X11 头文件会定义 Status、Data、index、min、max 等宏，若先于 Qt 包含会破坏
 * QtCore（例如 qtextstream.h 要求不能在已定义 Status 之后再被包含）。
 */
#include <X11/XKBlib.h>

#include "fakekey/fakekey.h"

using namespace freewb;

#define QSS_FILE ":/qss/keyboard.qss"
#define QSS_CAPS_SHIFT_FLG "background-color: rgba(21, 151, 242, 180)"

/********************************************************************************************/
enum KeyEventType
{
    KEY_EVT_PRESSED,
    KEY_EVT_RELEASED,
    KEY_EVT_CLICKED
};

static Display *g_x11Dpy = nullptr;
static struct FakeKey *g_fk = nullptr;

static void init_virtual_keyboard_x11()
{
    if (g_x11Dpy == nullptr)
    {
        if ((g_x11Dpy = XOpenDisplay(nullptr)) == nullptr)
        {
            return;
        }

        if (g_fk == nullptr)
        {
            if ((g_fk = fakekey_init(g_x11Dpy)) == nullptr)
            {
                return;
            }
        }
    }
}

static void report_key_event_to_x11(const QString &keyValue, KeyEventType evtType)
{
    const QByteArray utf8Bytes = keyValue.toUtf8();
    int len = utf8Bytes.length();
    unsigned char utf8[5] = {0};

    Q_ASSERT(static_cast<unsigned int>(len + 1) < sizeof(utf8));

    strncpy(reinterpret_cast<char *>(utf8), utf8Bytes.constData(), static_cast<unsigned int>(len));

    if (evtType == KEY_EVT_CLICKED)
    {
        if (g_fk == nullptr)
        {
            return;
        }
        fakekey_press(g_fk, utf8, len, 0);
        fakekey_release(g_fk);
    }
}

static int get_caps_state()
{
    int capsState = 0;

    if (g_x11Dpy)
    {
        unsigned int n;
        XkbGetIndicatorState(g_x11Dpy, XkbUseCoreKbd, &n);
        capsState = (n & 0x01) == 1;
    }

    return capsState;
}

/********************************************************************************************/
VirtualKeyboard::VirtualKeyboard(VirtualKeyboardMode mode, QWidget *parent) : QWidget(parent), ui(new Ui::VirtualKeyboard)
{
    ui->setupUi(this);
    ui->frame->installEventFilter(this);
    init_keyboard_keygroup();

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();
    m_shiftFlag = false;
    m_capsFlag = false;

    if (mode == VK_MODE_PC)
    {
        setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
        init_fixed_key_value();

        QDesktopWidget *d = QApplication::desktop();
        m_vkDefaultPos = QPoint((d->width() - size().width()) / 2, d->height() - size().height() - 100);

        settings::instance().set_vkMode(-1);

        // 虚拟键盘输入模式连接X11服务器
        init_virtual_keyboard_x11();

        update_caps_flg();
    }

    set_work_mode(mode);

    // 载入窗口全局UI样式表
    QFile qssFile(QSS_FILE);
    if (!qssFile.open(QFile::ReadOnly))
    {
    }
    else
    {
        this->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }
}

VirtualKeyboard::~VirtualKeyboard()
{
    delete ui;
}

void VirtualKeyboard::init_keyboard_keygroup()
{
    ui->btnChar0->set_key_name("0");
    ui->btnChar1->set_key_name("1");
    ui->btnChar2->set_key_name("2");
    ui->btnChar3->set_key_name("3");
    ui->btnChar4->set_key_name("4");
    ui->btnChar5->set_key_name("5");
    ui->btnChar6->set_key_name("6");
    ui->btnChar7->set_key_name("7");
    ui->btnChar8->set_key_name("8");
    ui->btnChar9->set_key_name("9");
    m_btnGroup.addButton(ui->btnChar0, VK_KEY_DIGIT0);
    m_btnGroup.addButton(ui->btnChar1, VK_KEY_DIGIT1);
    m_btnGroup.addButton(ui->btnChar2, VK_KEY_DIGIT2);
    m_btnGroup.addButton(ui->btnChar3, VK_KEY_DIGIT3);
    m_btnGroup.addButton(ui->btnChar4, VK_KEY_DIGIT4);
    m_btnGroup.addButton(ui->btnChar5, VK_KEY_DIGIT5);
    m_btnGroup.addButton(ui->btnChar6, VK_KEY_DIGIT6);
    m_btnGroup.addButton(ui->btnChar7, VK_KEY_DIGIT7);
    m_btnGroup.addButton(ui->btnChar8, VK_KEY_DIGIT8);
    m_btnGroup.addButton(ui->btnChar9, VK_KEY_DIGIT9);

    ui->btnCharA->set_key_name("A");
    ui->btnCharB->set_key_name("B");
    ui->btnCharC->set_key_name("C");
    ui->btnCharD->set_key_name("D");
    ui->btnCharE->set_key_name("E");
    ui->btnCharF->set_key_name("F");
    ui->btnCharG->set_key_name("G");
    ui->btnCharH->set_key_name("H");
    ui->btnCharI->set_key_name("I");
    ui->btnCharJ->set_key_name("J");
    ui->btnCharK->set_key_name("K");
    ui->btnCharL->set_key_name("L");
    ui->btnCharM->set_key_name("M");
    ui->btnCharN->set_key_name("N");
    ui->btnCharO->set_key_name("O");
    ui->btnCharP->set_key_name("P");
    ui->btnCharQ->set_key_name("Q");
    ui->btnCharR->set_key_name("R");
    ui->btnCharS->set_key_name("S");
    ui->btnCharT->set_key_name("T");
    ui->btnCharU->set_key_name("U");
    ui->btnCharV->set_key_name("V");
    ui->btnCharW->set_key_name("W");
    ui->btnCharX->set_key_name("X");
    ui->btnCharY->set_key_name("Y");
    ui->btnCharZ->set_key_name("Z");
    m_btnGroup.addButton(ui->btnCharA, VK_KEY_A);
    m_btnGroup.addButton(ui->btnCharB, VK_KEY_B);
    m_btnGroup.addButton(ui->btnCharC, VK_KEY_C);
    m_btnGroup.addButton(ui->btnCharD, VK_KEY_D);
    m_btnGroup.addButton(ui->btnCharE, VK_KEY_E);
    m_btnGroup.addButton(ui->btnCharF, VK_KEY_F);
    m_btnGroup.addButton(ui->btnCharG, VK_KEY_G);
    m_btnGroup.addButton(ui->btnCharH, VK_KEY_H);
    m_btnGroup.addButton(ui->btnCharI, VK_KEY_I);
    m_btnGroup.addButton(ui->btnCharJ, VK_KEY_J);
    m_btnGroup.addButton(ui->btnCharK, VK_KEY_K);
    m_btnGroup.addButton(ui->btnCharL, VK_KEY_L);
    m_btnGroup.addButton(ui->btnCharM, VK_KEY_M);
    m_btnGroup.addButton(ui->btnCharN, VK_KEY_N);
    m_btnGroup.addButton(ui->btnCharO, VK_KEY_O);
    m_btnGroup.addButton(ui->btnCharP, VK_KEY_P);
    m_btnGroup.addButton(ui->btnCharQ, VK_KEY_Q);
    m_btnGroup.addButton(ui->btnCharR, VK_KEY_R);
    m_btnGroup.addButton(ui->btnCharS, VK_KEY_S);
    m_btnGroup.addButton(ui->btnCharT, VK_KEY_T);
    m_btnGroup.addButton(ui->btnCharU, VK_KEY_U);
    m_btnGroup.addButton(ui->btnCharV, VK_KEY_V);
    m_btnGroup.addButton(ui->btnCharW, VK_KEY_W);
    m_btnGroup.addButton(ui->btnCharX, VK_KEY_X);
    m_btnGroup.addButton(ui->btnCharY, VK_KEY_Y);
    m_btnGroup.addButton(ui->btnCharZ, VK_KEY_Z);

    ui->btnBackQuote->set_key_name("`");
    ui->btnSub->set_key_name("-");
    ui->btnAdd->set_key_name("=");
    ui->btnLeftBracket->set_key_name("[");
    ui->btnRightBracket->set_key_name("]");
    ui->btnBackSlash->set_key_name("\\");
    ui->btnSemicolon->set_key_name(";");
    ui->btnQuote->set_key_name("'");
    ui->btnComma->set_key_name(",");
    ui->btnPeriod->set_key_name(".");
    ui->btnSlash->set_key_name("/");
    m_btnGroup.addButton(ui->btnBackQuote, VK_KEY_BACKQUOTE);
    m_btnGroup.addButton(ui->btnSub, VK_KEY_SUB);
    m_btnGroup.addButton(ui->btnAdd, VK_KEY_EQUAL);
    m_btnGroup.addButton(ui->btnBacksapce, VK_KEY_BACKSPACE);
    m_btnGroup.addButton(ui->btnTab, VK_KEY_TAB);
    m_btnGroup.addButton(ui->btnLeftBracket, VK_KEY_LEFT_BRACKET);
    m_btnGroup.addButton(ui->btnRightBracket, VK_KEY_RIGHT_BRACKET);
    m_btnGroup.addButton(ui->btnBackSlash, VK_KEY_BACKSLASH);
    m_btnGroup.addButton(ui->btnCaps, VK_KEY_CAPS);
    m_btnGroup.addButton(ui->btnSemicolon, VK_KEY_SEMICOLON);
    m_btnGroup.addButton(ui->btnQuote, VK_KEY_QUOTE);
    m_btnGroup.addButton(ui->btnEnter, VK_KEY_ENTER);
    m_btnGroup.addButton(ui->btnShift, VK_KEY_SHIFT);
    m_btnGroup.addButton(ui->btnComma, VK_KEY_COMMA);
    m_btnGroup.addButton(ui->btnPeriod, VK_KEY_PERIOD);
    m_btnGroup.addButton(ui->btnSlash, VK_KEY_SLASH);
    m_btnGroup.addButton(ui->btnInsert, VK_KEY_INSERT);
    m_btnGroup.addButton(ui->btnDel, VK_KEY_DEL);
    m_btnGroup.addButton(ui->btnSpace, VK_KEY_SPACE);
    m_btnGroup.addButton(ui->btnEsc, VK_KEY_ESC);

    connect(&m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_virtual_keyboard_clicked(int)));
}

void VirtualKeyboard::init_fixed_key_value()
{
    // 以下这些为控制类型按键
    m_ctrlKeyValue[VK_KEY_BACKSPACE - kVkSymbolKeyCount] = 0x08; // 退格
    m_ctrlKeyValue[VK_KEY_TAB - kVkSymbolKeyCount] = 0x09;       // 制表符
    m_ctrlKeyValue[VK_KEY_CAPS - kVkSymbolKeyCount] = 0xe5;      // 大小写
    m_ctrlKeyValue[VK_KEY_ENTER - kVkSymbolKeyCount] = 0x0d;     // Enter
    m_ctrlKeyValue[VK_KEY_SHIFT - kVkSymbolKeyCount] = 0xe1;     // Shift
    m_ctrlKeyValue[VK_KEY_INSERT - kVkSymbolKeyCount] = 0x9e;    // 插入
    m_ctrlKeyValue[VK_KEY_DEL - kVkSymbolKeyCount] = 0xff;       // 删除
    m_ctrlKeyValue[VK_KEY_SPACE - kVkSymbolKeyCount] = 0x20;     // 空格
    m_ctrlKeyValue[VK_KEY_ESC - kVkSymbolKeyCount] = 0x1b;       // Esc
}

// 设置键盘的工作模式
void VirtualKeyboard::set_work_mode(VirtualKeyboardMode mode)
{
    m_vkWorkMode = mode;

    if (m_vkWorkMode >= VK_MODE_CUSTOM_CHAR)
    {
        ui->btnBacksapce->setEnabled(false);
        ui->btnTab->setEnabled(false);
        ui->btnCaps->setEnabled(false);
        ui->btnEnter->setEnabled(false);
        ui->btnShift->setEnabled(false);
        ui->btnInsert->setEnabled(false);
        ui->btnDel->setEnabled(false);
        ui->btnSpace->setEnabled(false);
        ui->btnEsc->setEnabled(false);
        if (m_vkWorkMode == VK_MODE_CUSTOM_MARK)
        {
            for (int i = VK_KEY_A; i <= VK_KEY_Z; ++i)
            {
                KeyButton *btn = static_cast<KeyButton *>(m_btnGroup.button(i));
                btn->setEnabled(false);
            }
        }
    }
}

// 更新键盘所有按键的显示内容
void VirtualKeyboard::update_keyboard_button()
{
    KeyButton *btn;
    CustomKeyValue customKeyValue;

    for (int i = 0; i < kVkSymbolKeyCount; i++)
    {
        btn = static_cast<KeyButton *>(m_btnGroup.button(i));

        // 输入模式
        if (m_vkWorkMode < VK_MODE_USER_CHAR)
        {
            const VkKeyPair &pair = VkLayouts::getKeyPair(m_vkWorkMode, static_cast<VkKey>(i));
            btn->set_custom_symbol(QString::fromStdString(pair.normal), QString::fromStdString(pair.shift));
        }
        else if (m_vkWorkMode == VK_MODE_USER_CHAR)
        {
            customKeyValue =
                customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(),
                                                                 settings::instance().get_CoustomMark(), i, kVkSymbolKeyCount));
            btn->set_custom_symbol(customKeyValue.commChar, customKeyValue.shiftChar);
        }

        // 自定义模式
        else if (m_vkWorkMode == VK_MODE_CUSTOM_CHAR)
        {
            customKeyValue =
                customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(),
                                                                 settings::instance().get_CoustomMark(), i, kVkSymbolKeyCount));
            btn->set_custom_symbol(customKeyValue.commChar, customKeyValue.shiftChar);
        }
        else if (m_vkWorkMode == VK_MODE_CUSTOM_MARK)
        {
            customKeyValue =
                customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(),
                                                                 settings::instance().get_CoustomMark(), i, kVkSymbolKeyCount));
            btn->set_custom_symbol(customKeyValue.commMark, customKeyValue.shiftMark);
        }

        btn->update();
    }
}

// 更新当前正在自定义的按键显示内容
void VirtualKeyboard::update_customkey_button(VkKey keyIdx, const QString &commChar, const QString &shiftChar)
{
    KeyButton *btn = static_cast<KeyButton *>(m_btnGroup.button(keyIdx));
    btn->set_custom_symbol(commChar, shiftChar);
    btn->update();
}

void VirtualKeyboard::switch_vk(int flg)
{
    VirtualKeyboardMode vkm = m_vkWorkMode;

    if (!isHidden())
    {
        if (flg)
        {
            if (vkm == VK_MODE_PC)
            {
                vkm = VK_MODE_USER_CHAR;
            }
            else
            {
                vkm = static_cast<VirtualKeyboardMode>(vkm - 1);
            }
        }
        else
        {
            vkm = static_cast<VirtualKeyboardMode>(vkm + 1);
            if (vkm >= VK_MODE_CUSTOM_CHAR)
            {
                vkm = VK_MODE_PC;
            }
        }
    }

    slot_open_win(vkm);
}

void VirtualKeyboard::switch_caps_flg(int capsFlg)
{
    if (capsFlg != m_capsFlag)
    {
        report_key_event_to_x11(m_ctrlKeyValue[VK_KEY_CAPS - kVkSymbolKeyCount], KEY_EVT_CLICKED);
    }
}

int VirtualKeyboard::get_caps_flg()
{
    return get_caps_state();
}

void VirtualKeyboard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void VirtualKeyboard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void VirtualKeyboard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_vkWorkMode < VK_MODE_CUSTOM_CHAR && m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

bool VirtualKeyboard::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->frame)
    {
        if (event->type() == QEvent::Enter)
        {
            QApplication::restoreOverrideCursor();
            QApplication::setOverrideCursor(Qt::PointingHandCursor);
        }
        else if (event->type() == QEvent::Leave)
        {
            QApplication::restoreOverrideCursor();
        }
    }

    return false;
}

// 虚拟键盘按键点击
void VirtualKeyboard::slot_virtual_keyboard_clicked(int keyIdx)
{
    if (keyIdx == VK_KEY_ESC)
    {
        closeWin();
        return;
    }

    // 输入模式（含用户字符）
    if (m_vkWorkMode < VK_MODE_CUSTOM_CHAR)
    {
        handle_fixed_keyboard_input_clicked(keyIdx);
    }

    // 用户自定义键盘符号
    else if (m_vkWorkMode == VK_MODE_CUSTOM_CHAR || m_vkWorkMode == VK_MODE_CUSTOM_MARK)
    {
        handle_custom_keyboard_clicked(static_cast<VkKey>(keyIdx),
                                       static_cast<KeyButton *>(m_btnGroup.button(keyIdx))->get_key_name());
    }
}

// 处理用户自定义键盘
void VirtualKeyboard::handle_custom_keyboard_clicked(VkKey keyIdx, const QString &keyName)
{
    CustomKeyValue customKeyValue;

    if (keyIdx < kVkSymbolKeyCount)
    {
        customKeyValue = customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(),
                                                                          settings::instance().get_CoustomMark(),
                                                                          static_cast<int>(keyIdx), kVkSymbolKeyCount));
    }

    emit signal_custom_key_clicked(keyIdx, keyName, customKeyValue);
}

void VirtualKeyboard::update_caps_flg()
{
    bool flg = get_caps_state();
    if (flg != m_capsFlag)
    {
        m_capsFlag = flg;
        if (!isHidden())
        {
            if (m_capsFlag)
            {
                ui->btnCaps->setStyleSheet(QSS_CAPS_SHIFT_FLG);
            }
            else
            {
                ui->btnCaps->setStyleSheet("");
            }
        }

        emit signal_kb_caps_changed(flg);
    }
}

// 请在获取完输入键值后再调用该函数对shift标志进行设置
void VirtualKeyboard::update_shift_flg(int keyIdx)
{
    if (keyIdx == VK_KEY_SHIFT)
    {
        m_shiftFlag = (m_shiftFlag == true) ? false : true;
        if (m_shiftFlag)
        {
            ui->btnShift->setStyleSheet(QSS_CAPS_SHIFT_FLG);
        }
        else
        {
            ui->btnShift->setStyleSheet("");
        }
    }
    else if (m_shiftFlag)
    {
        m_shiftFlag = false;
        ui->btnShift->setStyleSheet("");
    }
}

void VirtualKeyboard::handle_fixed_keyboard_input_clicked(int keyIdx)
{
    QString value("");

    if (keyIdx < kVkSymbolKeyCount)
    {
        const VkKeyPair &pair = VkLayouts::getKeyPair(VK_MODE_PC, static_cast<VkKey>(keyIdx));
        value = QString::fromStdString(m_shiftFlag ? pair.shift : pair.normal);
    }
    else
    {
        value = m_ctrlKeyValue[keyIdx - kVkSymbolKeyCount];
    }

    update_shift_flg(keyIdx);

    if (keyIdx != VK_KEY_SHIFT && !value.isEmpty())
    {
        report_key_event_to_x11(value, KEY_EVT_CLICKED);
    }
}

void VirtualKeyboard::slot_load_setting_data()
{
    update_keyboard_button();
}

void VirtualKeyboard::slot_open_win(VirtualKeyboardMode mode)
{
    if (m_vkWorkMode != mode)
    {
        set_work_mode(mode);
        emit signal_vk_mode_changed(mode);
    }

    openWin();
}

void VirtualKeyboard::openWin()
{
    update_keyboard_button();

    if (isHidden())
    {
        Sound::play(SOUND_ENTER);
        move(m_vkDefaultPos);
        show();
        ui->btnCaps->setStyleSheet(m_capsFlag ? QSS_CAPS_SHIFT_FLG : "");
    }

    settings::instance().set_vkMode(m_vkWorkMode);
    g_settingsNotifier.notifySettingDataChangedToFcitx();
}

void VirtualKeyboard::closeWin()
{
    if (isHidden())
    {
        return;
    }

    Sound::play(SOUND_BACK);
    hide();
    settings::instance().set_vkMode(-1);
    g_settingsNotifier.notifySettingDataChangedToFcitx();
}

void VirtualKeyboard::slot_key_clicked(int keyCode)
{
    if (keyCode == 66) // CAPS
    {
        update_caps_flg();
    }
}
