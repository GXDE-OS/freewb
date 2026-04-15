#include "keyboard.h"

#include <QDesktopWidget>

#include "commdefine.h"
#include "keybutton.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "ui_keyboard.h"

/*
 * X11 头文件会定义 Status、Data、index、min、max 等宏，若先于 Qt 包含会破坏
 * QtCore（例如 qtextstream.h 要求不能在已定义 Status 之后再被包含）。
 */
#include <X11/XKBlib.h>

#include "fakekey/fakekey.h"

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
    int len = keyValue.toUtf8().length();
    unsigned char utf8[5] = {0};

    Q_ASSERT(static_cast<unsigned int>(len + 1) < sizeof(utf8));

    strncpy(reinterpret_cast<char *>(utf8), keyValue.toUtf8().data(), static_cast<unsigned int>(len));

    if (evtType == KEY_EVT_CLICKED)
    {
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
Keyboard::Keyboard(VirtualKeyboardMode mode, QWidget *parent) : QWidget(parent), ui(new Ui::Keyboard)
{
    ui->setupUi(this);
    ui->frame->installEventFilter(this);
    init_keyboard_keygroup();

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();
    m_shiftFlag = false;
    m_capsFlag = false;

    if (mode == VKM_INPUT_PC)
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

Keyboard::~Keyboard()
{
    delete ui;
}

void Keyboard::init_keyboard_keygroup()
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
    m_btnGroup.addButton(ui->btnChar0, KEY_0);
    m_btnGroup.addButton(ui->btnChar1, KEY_1);
    m_btnGroup.addButton(ui->btnChar2, KEY_2);
    m_btnGroup.addButton(ui->btnChar3, KEY_3);
    m_btnGroup.addButton(ui->btnChar4, KEY_4);
    m_btnGroup.addButton(ui->btnChar5, KEY_5);
    m_btnGroup.addButton(ui->btnChar6, KEY_6);
    m_btnGroup.addButton(ui->btnChar7, KEY_7);
    m_btnGroup.addButton(ui->btnChar8, KEY_8);
    m_btnGroup.addButton(ui->btnChar9, KEY_9);

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
    m_btnGroup.addButton(ui->btnCharA, KEY_A);
    m_btnGroup.addButton(ui->btnCharB, KEY_B);
    m_btnGroup.addButton(ui->btnCharC, KEY_C);
    m_btnGroup.addButton(ui->btnCharD, KEY_D);
    m_btnGroup.addButton(ui->btnCharE, KEY_E);
    m_btnGroup.addButton(ui->btnCharF, KEY_F);
    m_btnGroup.addButton(ui->btnCharG, KEY_G);
    m_btnGroup.addButton(ui->btnCharH, KEY_H);
    m_btnGroup.addButton(ui->btnCharI, KEY_I);
    m_btnGroup.addButton(ui->btnCharJ, KEY_J);
    m_btnGroup.addButton(ui->btnCharK, KEY_K);
    m_btnGroup.addButton(ui->btnCharL, KEY_L);
    m_btnGroup.addButton(ui->btnCharM, KEY_M);
    m_btnGroup.addButton(ui->btnCharN, KEY_N);
    m_btnGroup.addButton(ui->btnCharO, KEY_O);
    m_btnGroup.addButton(ui->btnCharP, KEY_P);
    m_btnGroup.addButton(ui->btnCharQ, KEY_Q);
    m_btnGroup.addButton(ui->btnCharR, KEY_R);
    m_btnGroup.addButton(ui->btnCharS, KEY_S);
    m_btnGroup.addButton(ui->btnCharT, KEY_T);
    m_btnGroup.addButton(ui->btnCharU, KEY_U);
    m_btnGroup.addButton(ui->btnCharV, KEY_V);
    m_btnGroup.addButton(ui->btnCharW, KEY_W);
    m_btnGroup.addButton(ui->btnCharX, KEY_X);
    m_btnGroup.addButton(ui->btnCharY, KEY_Y);
    m_btnGroup.addButton(ui->btnCharZ, KEY_Z);

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
    m_btnGroup.addButton(ui->btnBackQuote, KEY_BACKQUOTE);
    m_btnGroup.addButton(ui->btnSub, KEY_SUB);
    m_btnGroup.addButton(ui->btnAdd, KEY_EQUAL);
    m_btnGroup.addButton(ui->btnBacksapce, KEY_BACKSAPCE);
    m_btnGroup.addButton(ui->btnTab, KEY_TAB);
    m_btnGroup.addButton(ui->btnLeftBracket, KEY_LEFT_BRACKET);
    m_btnGroup.addButton(ui->btnRightBracket, KEY_RIGHT_BRACKET);
    m_btnGroup.addButton(ui->btnBackSlash, KEY_BACKSLASH);
    m_btnGroup.addButton(ui->btnCaps, KEY_CAPS);
    m_btnGroup.addButton(ui->btnSemicolon, KEY_SEMICOLON);
    m_btnGroup.addButton(ui->btnQuote, KEY_QUOTE);
    m_btnGroup.addButton(ui->btnEnter, KEY_ENTER);
    m_btnGroup.addButton(ui->btnShift, KEY_SHIFT);
    m_btnGroup.addButton(ui->btnComma, KEY_COMMA);
    m_btnGroup.addButton(ui->btnPeriod, KEY_PERIOD);
    m_btnGroup.addButton(ui->btnSlash, KEY_SLASH);
    m_btnGroup.addButton(ui->btnInsert, KEY_INSERT);
    m_btnGroup.addButton(ui->btnDel, KEY_DEL);
    m_btnGroup.addButton(ui->btnSpace, KEY_SPACE);
    m_btnGroup.addButton(ui->btnEsc, KEY_ESC);

    connect(&m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(slot_virtual_keyboard_clicked(int)));
}

void Keyboard::init_fixed_key_value()
{
    // 以下这些为控制类型按键
    m_ctrlKeyValue[KEY_BACKSAPCE - KEY_SYMBOL_NUM] = 0x08; // 退格
    m_ctrlKeyValue[KEY_TAB - KEY_SYMBOL_NUM] = 0x09;       // 制表符
    m_ctrlKeyValue[KEY_CAPS - KEY_SYMBOL_NUM] = 0xe5;      // 大小写
    m_ctrlKeyValue[KEY_ENTER - KEY_SYMBOL_NUM] = 0x0d;     // Enter
    m_ctrlKeyValue[KEY_SHIFT - KEY_SYMBOL_NUM] = 0xe1;     // Shift
    m_ctrlKeyValue[KEY_INSERT - KEY_SYMBOL_NUM] = 0x9e;    // 插入
    m_ctrlKeyValue[KEY_DEL - KEY_SYMBOL_NUM] = 0xff;       // 删除
    m_ctrlKeyValue[KEY_SPACE - KEY_SYMBOL_NUM] = 0x20;     // 空格
    m_ctrlKeyValue[KEY_ESC - KEY_SYMBOL_NUM] = 0x1b;       // Esc

    /********************************* PC键盘　*********************************/
    m_pcKeyValue.resize(KEY_SYMBOL_NUM);
    m_pcKeyValue[KEY_0] << "0" << ")";
    m_pcKeyValue[KEY_1] << "1" << "!";
    m_pcKeyValue[KEY_2] << "2" << "@";
    m_pcKeyValue[KEY_3] << "3" << "#";
    m_pcKeyValue[KEY_4] << "4" << "$";
    m_pcKeyValue[KEY_5] << "5" << "%";
    m_pcKeyValue[KEY_6] << "6" << "^";
    m_pcKeyValue[KEY_7] << "7" << "&";
    m_pcKeyValue[KEY_8] << "8" << "*";
    m_pcKeyValue[KEY_9] << "9" << "(";
    m_pcKeyValue[KEY_A] << "a" << "A";
    m_pcKeyValue[KEY_B] << "b" << "B";
    m_pcKeyValue[KEY_C] << "c" << "C";
    m_pcKeyValue[KEY_D] << "d" << "D";
    m_pcKeyValue[KEY_E] << "e" << "E";
    m_pcKeyValue[KEY_F] << "f" << "F";
    m_pcKeyValue[KEY_G] << "g" << "G";
    m_pcKeyValue[KEY_H] << "h" << "H";
    m_pcKeyValue[KEY_I] << "i" << "I";
    m_pcKeyValue[KEY_J] << "j" << "J";
    m_pcKeyValue[KEY_K] << "k" << "K";
    m_pcKeyValue[KEY_L] << "l" << "L";
    m_pcKeyValue[KEY_M] << "m" << "M";
    m_pcKeyValue[KEY_N] << "n" << "N";
    m_pcKeyValue[KEY_O] << "o" << "O";
    m_pcKeyValue[KEY_P] << "p" << "P";
    m_pcKeyValue[KEY_Q] << "q" << "Q";
    m_pcKeyValue[KEY_R] << "r" << "R";
    m_pcKeyValue[KEY_S] << "s" << "S";
    m_pcKeyValue[KEY_T] << "t" << "T";
    m_pcKeyValue[KEY_U] << "u" << "U";
    m_pcKeyValue[KEY_V] << "v" << "V";
    m_pcKeyValue[KEY_W] << "w" << "W";
    m_pcKeyValue[KEY_X] << "x" << "X";
    m_pcKeyValue[KEY_Y] << "y" << "Y";
    m_pcKeyValue[KEY_Z] << "z" << "Z";
    m_pcKeyValue[KEY_BACKQUOTE] << "`" << "~";     // 反引号
    m_pcKeyValue[KEY_SUB] << "-" << "_";           // 减号
    m_pcKeyValue[KEY_EQUAL] << "=" << "+";         // 等号
    m_pcKeyValue[KEY_LEFT_BRACKET] << "[" << "{";  // 左中括号
    m_pcKeyValue[KEY_RIGHT_BRACKET] << "]" << "}"; // 右中括号
    m_pcKeyValue[KEY_BACKSLASH] << "\\" << "|";    // 反斜杠
    m_pcKeyValue[KEY_SEMICOLON] << ";" << ":";     // 分号
    m_pcKeyValue[KEY_QUOTE] << "'" << "\"";        // 引号
    m_pcKeyValue[KEY_COMMA] << "," << "<";         // 逗号
    m_pcKeyValue[KEY_PERIOD] << "." << ">";        // 句号
    m_pcKeyValue[KEY_SLASH] << "/" << "?";         // 斜杠

    /********************************* 希腊字母　*********************************/
    m_greekKeyValue.resize(KEY_SYMBOL_NUM);
    m_greekKeyValue[KEY_0] << "" << "";
    m_greekKeyValue[KEY_1] << "" << "";
    m_greekKeyValue[KEY_2] << "" << "";
    m_greekKeyValue[KEY_3] << "" << "";
    m_greekKeyValue[KEY_4] << "" << "";
    m_greekKeyValue[KEY_5] << "" << "";
    m_greekKeyValue[KEY_6] << "" << "";
    m_greekKeyValue[KEY_7] << "" << "";
    m_greekKeyValue[KEY_8] << "" << "";
    m_greekKeyValue[KEY_9] << "" << "";
    m_greekKeyValue[KEY_A] << "κ" << "Κ";
    m_greekKeyValue[KEY_B] << "χ" << "Χ";
    m_greekKeyValue[KEY_C] << "υ" << "Υ";
    m_greekKeyValue[KEY_D] << "μ" << "Μ";
    m_greekKeyValue[KEY_E] << "γ" << "Γ";
    m_greekKeyValue[KEY_F] << "ν" << "Ν";
    m_greekKeyValue[KEY_G] << "ξ" << "Ξ";
    m_greekKeyValue[KEY_H] << "ο" << "Ο";
    m_greekKeyValue[KEY_I] << "θ" << "Θ";
    m_greekKeyValue[KEY_J] << "π" << "Π";
    m_greekKeyValue[KEY_K] << "ρ" << "Ρ";
    m_greekKeyValue[KEY_L] << "" << "";
    m_greekKeyValue[KEY_M] << "ω" << "Ω";
    m_greekKeyValue[KEY_N] << "ψ" << "Ψ";
    m_greekKeyValue[KEY_O] << "ι" << "Ι";
    m_greekKeyValue[KEY_P] << "" << "";
    m_greekKeyValue[KEY_Q] << "α" << "Α";
    m_greekKeyValue[KEY_R] << "δ" << "Δ";
    m_greekKeyValue[KEY_S] << "λ" << "Λ";
    m_greekKeyValue[KEY_T] << "ε" << "Ε";
    m_greekKeyValue[KEY_U] << "η" << "Η";
    m_greekKeyValue[KEY_V] << "φ" << "Φ";
    m_greekKeyValue[KEY_W] << "β" << "Β";
    m_greekKeyValue[KEY_X] << "τ" << "Τ";
    m_greekKeyValue[KEY_Y] << "ζ" << "Ζ";
    m_greekKeyValue[KEY_Z] << "σ" << "Σ";
    m_greekKeyValue[KEY_BACKQUOTE] << "" << "";     // 反引号
    m_greekKeyValue[KEY_SUB] << "" << "";           // 减号
    m_greekKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_greekKeyValue[KEY_LEFT_BRACKET] << "" << "";  // 左中括号
    m_greekKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_greekKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_greekKeyValue[KEY_SEMICOLON] << "" << "";     // 分号
    m_greekKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_greekKeyValue[KEY_COMMA] << "" << "";         // 逗号
    m_greekKeyValue[KEY_PERIOD] << "" << "";        // 句号
    m_greekKeyValue[KEY_SLASH] << "" << "";         // 斜杠

    /********************************* 俄文字母　*********************************/
    m_russianKeyValue.resize(KEY_SYMBOL_NUM);
    m_russianKeyValue[KEY_0] << "" << "";
    m_russianKeyValue[KEY_1] << "" << "";
    m_russianKeyValue[KEY_2] << "" << "";
    m_russianKeyValue[KEY_3] << "" << "";
    m_russianKeyValue[KEY_4] << "" << "";
    m_russianKeyValue[KEY_5] << "" << "";
    m_russianKeyValue[KEY_6] << "" << "";
    m_russianKeyValue[KEY_7] << "" << "";
    m_russianKeyValue[KEY_8] << "" << "";
    m_russianKeyValue[KEY_9] << "" << "";
    m_russianKeyValue[KEY_A] << "л" << "Л";
    m_russianKeyValue[KEY_B] << "ъ" << "Ъ";
    m_russianKeyValue[KEY_C] << "ш" << "Ш";
    m_russianKeyValue[KEY_D] << "н" << "Н";
    m_russianKeyValue[KEY_E] << "в" << "В";
    m_russianKeyValue[KEY_F] << "о" << "О";
    m_russianKeyValue[KEY_G] << "п" << "П";
    m_russianKeyValue[KEY_H] << "р" << "Р";
    m_russianKeyValue[KEY_I] << "ж" << "Ж";
    m_russianKeyValue[KEY_J] << "с" << "С";
    m_russianKeyValue[KEY_K] << "т" << "Т";
    m_russianKeyValue[KEY_L] << "у" << "У";
    m_russianKeyValue[KEY_M] << "ь" << "Ь";
    m_russianKeyValue[KEY_N] << "ы" << "Ы";
    m_russianKeyValue[KEY_O] << "з" << "З";
    m_russianKeyValue[KEY_P] << "и" << "И";
    m_russianKeyValue[KEY_Q] << "а" << "А";
    m_russianKeyValue[KEY_R] << "г" << "Г";
    m_russianKeyValue[KEY_S] << "м" << "М";
    m_russianKeyValue[KEY_T] << "д" << "Д";
    m_russianKeyValue[KEY_U] << "ё" << "Ё";
    m_russianKeyValue[KEY_V] << "щ" << "Щ";
    m_russianKeyValue[KEY_W] << "б" << "Б";
    m_russianKeyValue[KEY_X] << "ч" << "Ч";
    m_russianKeyValue[KEY_Y] << "е" << "Е";
    m_russianKeyValue[KEY_Z] << "ц" << "Ц";
    m_russianKeyValue[KEY_BACKQUOTE] << "" << "";       // 反引号
    m_russianKeyValue[KEY_SUB] << "" << "";             // 减号
    m_russianKeyValue[KEY_EQUAL] << "" << "";           // 等号
    m_russianKeyValue[KEY_LEFT_BRACKET] << "й" << "Й";  // 左中括号
    m_russianKeyValue[KEY_RIGHT_BRACKET] << "к" << "К"; // 右中括号
    m_russianKeyValue[KEY_BACKSLASH] << "" << "";       // 反斜杠
    m_russianKeyValue[KEY_SEMICOLON] << "ф" << "Ф";     // 分号
    m_russianKeyValue[KEY_QUOTE] << "х" << "Х";         // 引号
    m_russianKeyValue[KEY_COMMA] << "э" << "Э";         // 逗号
    m_russianKeyValue[KEY_PERIOD] << "ю" << "Ю";        // 句号
    m_russianKeyValue[KEY_SLASH] << "я" << "Я";         // 斜杠

    /********************************* 注音符号　*********************************/
    m_phoneticKeyValue.resize(KEY_SYMBOL_NUM);
    m_phoneticKeyValue[KEY_0] << "ㄦ" << "";
    m_phoneticKeyValue[KEY_1] << "ㄉ" << "";
    m_phoneticKeyValue[KEY_2] << "" << "";
    m_phoneticKeyValue[KEY_3] << "" << "";
    m_phoneticKeyValue[KEY_4] << "ㄓ" << "";
    m_phoneticKeyValue[KEY_5] << "" << "";
    m_phoneticKeyValue[KEY_6] << "" << "";
    m_phoneticKeyValue[KEY_7] << "ㄚ" << "";
    m_phoneticKeyValue[KEY_8] << "ㄞ" << "";
    m_phoneticKeyValue[KEY_9] << "ㄢ" << "";
    m_phoneticKeyValue[KEY_A] << "ㄇ" << "";
    m_phoneticKeyValue[KEY_B] << "ㄖ" << "";
    m_phoneticKeyValue[KEY_C] << "ㄏ" << "";
    m_phoneticKeyValue[KEY_D] << "ㄎ" << "";
    m_phoneticKeyValue[KEY_E] << "ㄍ" << "";
    m_phoneticKeyValue[KEY_F] << "ㄑ" << "";
    m_phoneticKeyValue[KEY_G] << "ㄕ" << "";
    m_phoneticKeyValue[KEY_H] << "ㄘ" << "";
    m_phoneticKeyValue[KEY_I] << "ㄛ" << "";
    m_phoneticKeyValue[KEY_J] << "ㄨ" << "";
    m_phoneticKeyValue[KEY_K] << "ㄜ" << "";
    m_phoneticKeyValue[KEY_L] << "ㄠ" << "";
    m_phoneticKeyValue[KEY_M] << "ㄩ" << "";
    m_phoneticKeyValue[KEY_N] << "ㄙ" << "";
    m_phoneticKeyValue[KEY_O] << "ㄟ" << "";
    m_phoneticKeyValue[KEY_P] << "ㄣ" << "";
    m_phoneticKeyValue[KEY_Q] << "ㄆ" << "";
    m_phoneticKeyValue[KEY_R] << "ㄐ" << "";
    m_phoneticKeyValue[KEY_S] << "ㄋ" << "";
    m_phoneticKeyValue[KEY_T] << "ㄔ" << "";
    m_phoneticKeyValue[KEY_U] << "ㄧ" << "";
    m_phoneticKeyValue[KEY_V] << "ㄒ" << "";
    m_phoneticKeyValue[KEY_W] << "ㄊ" << "";
    m_phoneticKeyValue[KEY_X] << "" << "";
    m_phoneticKeyValue[KEY_Y] << "ㄗ" << "";
    m_phoneticKeyValue[KEY_Z] << "ㄈ" << "";
    m_phoneticKeyValue[KEY_BACKQUOTE] << "ㄅ" << "";   // 反引号
    m_phoneticKeyValue[KEY_SUB] << "" << "";           // 减号
    m_phoneticKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_phoneticKeyValue[KEY_LEFT_BRACKET] << "" << "";  // 左中括号
    m_phoneticKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_phoneticKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_phoneticKeyValue[KEY_SEMICOLON] << "ㄤ" << "";   // 分号
    m_phoneticKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_phoneticKeyValue[KEY_COMMA] << "ㄝ" << "";       // 逗号
    m_phoneticKeyValue[KEY_PERIOD] << "ㄡ" << "";      // 句号
    m_phoneticKeyValue[KEY_SLASH] << "ㄥ" << "";       // 斜杠

    /********************************* 汉语拼音　*********************************/
    m_pinyinKeyValue.resize(KEY_SYMBOL_NUM);
    m_pinyinKeyValue[KEY_0] << "" << "";
    m_pinyinKeyValue[KEY_1] << "" << "";
    m_pinyinKeyValue[KEY_2] << "" << "";
    m_pinyinKeyValue[KEY_3] << "" << "";
    m_pinyinKeyValue[KEY_4] << "" << "";
    m_pinyinKeyValue[KEY_5] << "" << "";
    m_pinyinKeyValue[KEY_6] << "" << "";
    m_pinyinKeyValue[KEY_7] << "" << "";
    m_pinyinKeyValue[KEY_8] << "" << "";
    m_pinyinKeyValue[KEY_9] << "" << "";
    m_pinyinKeyValue[KEY_A] << "ē" << "";
    m_pinyinKeyValue[KEY_B] << "" << "";
    m_pinyinKeyValue[KEY_C] << "ǔ" << "";
    m_pinyinKeyValue[KEY_D] << "ě" << "";
    m_pinyinKeyValue[KEY_E] << "ǎ" << "";
    m_pinyinKeyValue[KEY_F] << "è" << "";
    m_pinyinKeyValue[KEY_G] << "" << "";
    m_pinyinKeyValue[KEY_H] << "ī" << "";
    m_pinyinKeyValue[KEY_I] << "ǒ" << "";
    m_pinyinKeyValue[KEY_J] << "í" << "";
    m_pinyinKeyValue[KEY_K] << "ǐ" << "";
    m_pinyinKeyValue[KEY_L] << "ì" << "";
    m_pinyinKeyValue[KEY_M] << "ǘ" << "";
    m_pinyinKeyValue[KEY_N] << "ǖ" << "";
    m_pinyinKeyValue[KEY_O] << "ò" << "";
    m_pinyinKeyValue[KEY_P] << "" << "";
    m_pinyinKeyValue[KEY_Q] << "ā" << "";
    m_pinyinKeyValue[KEY_R] << "à" << "";
    m_pinyinKeyValue[KEY_S] << "é" << "";
    m_pinyinKeyValue[KEY_T] << "" << "";
    m_pinyinKeyValue[KEY_U] << "ó" << "";
    m_pinyinKeyValue[KEY_V] << "ù" << "";
    m_pinyinKeyValue[KEY_W] << "á" << "";
    m_pinyinKeyValue[KEY_X] << "ú" << "";
    m_pinyinKeyValue[KEY_Y] << "ō" << "";
    m_pinyinKeyValue[KEY_Z] << "ū" << "";
    m_pinyinKeyValue[KEY_BACKQUOTE] << "" << "";     // 反引号
    m_pinyinKeyValue[KEY_SUB] << "" << "";           // 减号
    m_pinyinKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_pinyinKeyValue[KEY_LEFT_BRACKET] << "ê" << ""; // 左中括号
    m_pinyinKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_pinyinKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_pinyinKeyValue[KEY_SEMICOLON] << "" << "";     // 分号
    m_pinyinKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_pinyinKeyValue[KEY_COMMA] << "ǚ" << "";        // 逗号
    m_pinyinKeyValue[KEY_PERIOD] << "ǜ" << "";       // 句号
    m_pinyinKeyValue[KEY_SLASH] << "ü" << "";        // 斜杠

    /********************************* 日文平假　*********************************/
    m_japanFlatKeyValue.resize(KEY_SYMBOL_NUM);
    m_japanFlatKeyValue[KEY_0] << "" << "";
    m_japanFlatKeyValue[KEY_1] << "ぃ" << "い";
    m_japanFlatKeyValue[KEY_2] << "ぅ" << "う";
    m_japanFlatKeyValue[KEY_3] << "ぇ" << "え";
    m_japanFlatKeyValue[KEY_4] << "ぉ" << "お";
    m_japanFlatKeyValue[KEY_5] << "か" << "が";
    m_japanFlatKeyValue[KEY_6] << "き" << "ぎ";
    m_japanFlatKeyValue[KEY_7] << "く" << "ぐ";
    m_japanFlatKeyValue[KEY_8] << "け" << "げ";
    m_japanFlatKeyValue[KEY_9] << "こ" << "ご";
    m_japanFlatKeyValue[KEY_A] << "な" << "ぱ";
    m_japanFlatKeyValue[KEY_B] << "も" << "ろ";
    m_japanFlatKeyValue[KEY_C] << "む" << "る";
    m_japanFlatKeyValue[KEY_D] << "ぬ" << "ぷ";
    m_japanFlatKeyValue[KEY_E] << "す" << "ず";
    m_japanFlatKeyValue[KEY_F] << "ね" << "ぺ";
    m_japanFlatKeyValue[KEY_G] << "の" << "ぽ";
    m_japanFlatKeyValue[KEY_H] << "は" << "ぼ";
    m_japanFlatKeyValue[KEY_I] << "つ" << "づ";
    m_japanFlatKeyValue[KEY_J] << "ひ" << "び";
    m_japanFlatKeyValue[KEY_K] << "ふ" << "ぶ";
    m_japanFlatKeyValue[KEY_L] << "へ" << "べ";
    m_japanFlatKeyValue[KEY_M] << "ゅ" << "ゆ";
    m_japanFlatKeyValue[KEY_N] << "ゃ" << "や";
    m_japanFlatKeyValue[KEY_O] << "っ" << "";
    m_japanFlatKeyValue[KEY_P] << "て" << "で";
    m_japanFlatKeyValue[KEY_Q] << "さ" << "ざ";
    m_japanFlatKeyValue[KEY_R] << "せ" << "ぜ";
    m_japanFlatKeyValue[KEY_S] << "に" << "ぴ";
    m_japanFlatKeyValue[KEY_T] << "そ" << "ぞ";
    m_japanFlatKeyValue[KEY_U] << "ち" << "ぢ";
    m_japanFlatKeyValue[KEY_V] << "め" << "れ";
    m_japanFlatKeyValue[KEY_W] << "し" << "じ";
    m_japanFlatKeyValue[KEY_X] << "み" << "り";
    m_japanFlatKeyValue[KEY_Y] << "た" << "だ";
    m_japanFlatKeyValue[KEY_Z] << "ま" << "ら";
    m_japanFlatKeyValue[KEY_BACKQUOTE] << "ぁ" << "あ";    // 反引号
    m_japanFlatKeyValue[KEY_SUB] << "ん" << "";            // 减号
    m_japanFlatKeyValue[KEY_EQUAL] << "" << "";            // 等号
    m_japanFlatKeyValue[KEY_LEFT_BRACKET] << "と" << "ど"; // 左中括号
    m_japanFlatKeyValue[KEY_RIGHT_BRACKET] << "ゐ" << "";  // 右中括号
    m_japanFlatKeyValue[KEY_BACKSLASH] << "" << "";        // 反斜杠
    m_japanFlatKeyValue[KEY_SEMICOLON] << "ほ" << "ぼ";    // 分号
    m_japanFlatKeyValue[KEY_QUOTE] << "ゑ" << "";          // 引号
    m_japanFlatKeyValue[KEY_COMMA] << "ょ" << "よ";        // 逗号
    m_japanFlatKeyValue[KEY_PERIOD] << "ゎ" << "わ";       // 句号
    m_japanFlatKeyValue[KEY_SLASH] << "を" << "";          // 斜杠

    /********************************* 日文片假　*********************************/
    m_japanPieceKeyValue.resize(KEY_SYMBOL_NUM);
    m_japanPieceKeyValue[KEY_0] << "ケ" << "ゲ";
    m_japanPieceKeyValue[KEY_1] << "ィ" << "イ";
    m_japanPieceKeyValue[KEY_2] << "ゥ" << "ウ";
    m_japanPieceKeyValue[KEY_3] << "ヴ" << "";
    m_japanPieceKeyValue[KEY_4] << "ェ" << "エ";
    m_japanPieceKeyValue[KEY_5] << "ォ" << "オ";
    m_japanPieceKeyValue[KEY_6] << "カ" << "ガ";
    m_japanPieceKeyValue[KEY_7] << "ヵ" << "";
    m_japanPieceKeyValue[KEY_8] << "キ" << "ギ";
    m_japanPieceKeyValue[KEY_9] << "ク" << "グ";
    m_japanPieceKeyValue[KEY_A] << "ナ" << "パ";
    m_japanPieceKeyValue[KEY_B] << "モ" << "ロ";
    m_japanPieceKeyValue[KEY_C] << "ム" << "ル";
    m_japanPieceKeyValue[KEY_D] << "ヌ" << "プ";
    m_japanPieceKeyValue[KEY_E] << "ス" << "ズ";
    m_japanPieceKeyValue[KEY_F] << "ネ" << "ペ";
    m_japanPieceKeyValue[KEY_G] << "ノ" << "ポ";
    m_japanPieceKeyValue[KEY_H] << "ハ" << "バ";
    m_japanPieceKeyValue[KEY_I] << "ツ" << "ヅ";
    m_japanPieceKeyValue[KEY_J] << "ヒ" << "ビ";
    m_japanPieceKeyValue[KEY_K] << "フ" << "ブ";
    m_japanPieceKeyValue[KEY_L] << "ヘ" << "ベ";
    m_japanPieceKeyValue[KEY_M] << "ュ" << "ユ";
    m_japanPieceKeyValue[KEY_N] << "ャ" << "ヤ";
    m_japanPieceKeyValue[KEY_O] << "ッ" << "";
    m_japanPieceKeyValue[KEY_P] << "テ" << "デ";
    m_japanPieceKeyValue[KEY_Q] << "サ" << "ザ";
    m_japanPieceKeyValue[KEY_R] << "セ" << "ゼ";
    m_japanPieceKeyValue[KEY_S] << "ニ" << "ピ";
    m_japanPieceKeyValue[KEY_T] << "ソ" << "ゾ";
    m_japanPieceKeyValue[KEY_U] << "チ" << "ヂ";
    m_japanPieceKeyValue[KEY_V] << "メ" << "レ";
    m_japanPieceKeyValue[KEY_W] << "シ" << "ジ";
    m_japanPieceKeyValue[KEY_X] << "ミ" << "リ";
    m_japanPieceKeyValue[KEY_Y] << "タ" << "ダ";
    m_japanPieceKeyValue[KEY_Z] << "マ" << "ラ";
    m_japanPieceKeyValue[KEY_BACKQUOTE] << "ァ" << "ア";    // 反引号
    m_japanPieceKeyValue[KEY_SUB] << "ヶ" << "";            // 减号
    m_japanPieceKeyValue[KEY_EQUAL] << "コ" << "ゴ";        // 等号
    m_japanPieceKeyValue[KEY_LEFT_BRACKET] << "ト" << "ド"; // 左中括号
    m_japanPieceKeyValue[KEY_RIGHT_BRACKET] << "ヰ" << "";  // 右中括号
    m_japanPieceKeyValue[KEY_BACKSLASH] << "ン" << "";      // 反斜杠
    m_japanPieceKeyValue[KEY_SEMICOLON] << "ホ" << "ボ";    // 分号
    m_japanPieceKeyValue[KEY_QUOTE] << "ヱ" << "";          // 引号
    m_japanPieceKeyValue[KEY_COMMA] << "ョ" << "ヨ";        // 逗号
    m_japanPieceKeyValue[KEY_PERIOD] << "ヮ" << "ワ";       // 句号
    m_japanPieceKeyValue[KEY_SLASH] << "ヲ" << "";          // 斜杠

    /********************************* 标点符号　*********************************/
    m_punctuationKeyValue.resize(KEY_SYMBOL_NUM);
    m_punctuationKeyValue[KEY_0] << "ˉ" << "";
    m_punctuationKeyValue[KEY_1] << "，" << "";
    m_punctuationKeyValue[KEY_2] << "、" << "";
    m_punctuationKeyValue[KEY_3] << ";" << "";
    m_punctuationKeyValue[KEY_4] << "：" << "";
    m_punctuationKeyValue[KEY_5] << "？" << "";
    m_punctuationKeyValue[KEY_6] << "！" << "";
    m_punctuationKeyValue[KEY_7] << "…" << "";
    m_punctuationKeyValue[KEY_8] << "—" << "";
    m_punctuationKeyValue[KEY_9] << "·" << "";
    m_punctuationKeyValue[KEY_A] << "〔" << "";
    m_punctuationKeyValue[KEY_B] << "（" << "";
    m_punctuationKeyValue[KEY_C] << "【" << "";
    m_punctuationKeyValue[KEY_D] << "〈" << "";
    m_punctuationKeyValue[KEY_E] << "“" << "";
    m_punctuationKeyValue[KEY_F] << "〉" << "";
    m_punctuationKeyValue[KEY_G] << "《" << "";
    m_punctuationKeyValue[KEY_H] << "》" << "";
    m_punctuationKeyValue[KEY_I] << "∶" << "";
    m_punctuationKeyValue[KEY_J] << "「" << "";
    m_punctuationKeyValue[KEY_K] << "」" << "";
    m_punctuationKeyValue[KEY_L] << "『" << "";
    m_punctuationKeyValue[KEY_M] << "［" << "";
    m_punctuationKeyValue[KEY_N] << "）" << "";
    m_punctuationKeyValue[KEY_O] << "＂" << "";
    m_punctuationKeyValue[KEY_P] << "＇" << "";
    m_punctuationKeyValue[KEY_Q] << "‘" << "";
    m_punctuationKeyValue[KEY_R] << "”" << "";
    m_punctuationKeyValue[KEY_S] << "〕" << "";
    m_punctuationKeyValue[KEY_T] << "々" << "";
    m_punctuationKeyValue[KEY_U] << "‖" << "";
    m_punctuationKeyValue[KEY_V] << "】" << "";
    m_punctuationKeyValue[KEY_W] << "’" << "";
    m_punctuationKeyValue[KEY_X] << "〗" << "";
    m_punctuationKeyValue[KEY_Y] << "～" << "";
    m_punctuationKeyValue[KEY_Z] << "〖" << "";
    m_punctuationKeyValue[KEY_BACKQUOTE] << "。" << "";     // 反引号
    m_punctuationKeyValue[KEY_SUB] << "ˇ" << "";            // 减号
    m_punctuationKeyValue[KEY_EQUAL] << "¨" << "";          // 等号
    m_punctuationKeyValue[KEY_LEFT_BRACKET] << "｀" << "";  // 左中括号
    m_punctuationKeyValue[KEY_RIGHT_BRACKET] << "｜" << ""; // 右中括号
    m_punctuationKeyValue[KEY_BACKSLASH] << "〃" << "";     // 反斜杠
    m_punctuationKeyValue[KEY_SEMICOLON] << "』" << "";     // 分号
    m_punctuationKeyValue[KEY_QUOTE] << "．" << "";         // 引号
    m_punctuationKeyValue[KEY_COMMA] << "］" << "";         // 逗号
    m_punctuationKeyValue[KEY_PERIOD] << "｛" << "";        // 句号
    m_punctuationKeyValue[KEY_SLASH] << "｝" << "";         // 斜杠

    /********************************* 数字序号　*********************************/
    m_digitalOrderKeyValue.resize(KEY_SYMBOL_NUM);
    m_digitalOrderKeyValue[KEY_0] << "Ⅺ" << "";
    m_digitalOrderKeyValue[KEY_1] << "Ⅱ" << "";
    m_digitalOrderKeyValue[KEY_2] << "Ⅲ" << "";
    m_digitalOrderKeyValue[KEY_3] << "Ⅳ" << "";
    m_digitalOrderKeyValue[KEY_4] << "Ⅴ" << "";
    m_digitalOrderKeyValue[KEY_5] << "Ⅵ" << "";
    m_digitalOrderKeyValue[KEY_6] << "Ⅶ" << "";
    m_digitalOrderKeyValue[KEY_7] << "Ⅷ" << "";
    m_digitalOrderKeyValue[KEY_8] << "Ⅸ" << "";
    m_digitalOrderKeyValue[KEY_9] << "Ⅹ" << "";
    m_digitalOrderKeyValue[KEY_A] << "㈠" << "①";
    m_digitalOrderKeyValue[KEY_B] << "⑸" << "⒂";
    m_digitalOrderKeyValue[KEY_C] << "⑶" << "⒀";
    m_digitalOrderKeyValue[KEY_D] << "㈢" << "③";
    m_digitalOrderKeyValue[KEY_E] << "⒊" << "⒔";
    m_digitalOrderKeyValue[KEY_F] << "㈣" << "④";
    m_digitalOrderKeyValue[KEY_G] << "㈤" << "⑤";
    m_digitalOrderKeyValue[KEY_H] << "㈥" << "⑥";
    m_digitalOrderKeyValue[KEY_I] << "⒏" << "⒙";
    m_digitalOrderKeyValue[KEY_J] << "㈦" << "⑦";
    m_digitalOrderKeyValue[KEY_K] << "㈧" << "⑧";
    m_digitalOrderKeyValue[KEY_L] << "㈨" << "⑨";
    m_digitalOrderKeyValue[KEY_M] << "⑺" << "⒄";
    m_digitalOrderKeyValue[KEY_N] << "⑹" << "⒃";
    m_digitalOrderKeyValue[KEY_O] << "⒐" << "⒚";
    m_digitalOrderKeyValue[KEY_P] << "⒑" << "⒛";
    m_digitalOrderKeyValue[KEY_Q] << "⒈" << "⒒";
    m_digitalOrderKeyValue[KEY_R] << "⒋" << "⒕";
    m_digitalOrderKeyValue[KEY_S] << "㈡" << "②";
    m_digitalOrderKeyValue[KEY_T] << "⒌" << "⒖";
    m_digitalOrderKeyValue[KEY_U] << "⒎" << "⒘";
    m_digitalOrderKeyValue[KEY_V] << "⑷" << "⒁";
    m_digitalOrderKeyValue[KEY_W] << "⒉" << "⒓";
    m_digitalOrderKeyValue[KEY_X] << "⑵" << "⑿";
    m_digitalOrderKeyValue[KEY_Y] << "⒍" << "⒗";
    m_digitalOrderKeyValue[KEY_Z] << "⑴" << "⑾";
    m_digitalOrderKeyValue[KEY_BACKQUOTE] << "Ⅰ" << "";    // 反引号
    m_digitalOrderKeyValue[KEY_SUB] << "Ⅻ" << "";          // 减号
    m_digitalOrderKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_digitalOrderKeyValue[KEY_LEFT_BRACKET] << "" << "";  // 左中括号
    m_digitalOrderKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_digitalOrderKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_digitalOrderKeyValue[KEY_SEMICOLON] << "㈩" << "⑩";  // 分号
    m_digitalOrderKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_digitalOrderKeyValue[KEY_COMMA] << "⑻" << "⒅";       // 逗号
    m_digitalOrderKeyValue[KEY_PERIOD] << "⑼" << "⒆";      // 句号
    m_digitalOrderKeyValue[KEY_SLASH] << "⑽" << "⒇";       // 斜杠

    /********************************* 数学符号　*********************************/
    m_mathKeyValue.resize(KEY_SYMBOL_NUM);
    m_mathKeyValue[KEY_0] << "" << "";
    m_mathKeyValue[KEY_1] << "≡" << "";
    m_mathKeyValue[KEY_2] << "≠" << "";
    m_mathKeyValue[KEY_3] << "＝" << "";
    m_mathKeyValue[KEY_4] << "≤" << "";
    m_mathKeyValue[KEY_5] << "≥" << "";
    m_mathKeyValue[KEY_6] << "＜" << "";
    m_mathKeyValue[KEY_7] << "＞" << "";
    m_mathKeyValue[KEY_8] << "≮" << "";
    m_mathKeyValue[KEY_9] << "≯" << "";
    m_mathKeyValue[KEY_A] << "∧" << "";
    m_mathKeyValue[KEY_B] << "⊙" << "";
    m_mathKeyValue[KEY_C] << "∠" << "";
    m_mathKeyValue[KEY_D] << "∑" << "";
    m_mathKeyValue[KEY_E] << "－" << "";
    m_mathKeyValue[KEY_F] << "∏" << "";
    m_mathKeyValue[KEY_G] << "∪" << "";
    m_mathKeyValue[KEY_H] << "∩" << "";
    m_mathKeyValue[KEY_I] << "∫" << "";
    m_mathKeyValue[KEY_J] << "∈" << "";
    m_mathKeyValue[KEY_K] << "" << "";
    m_mathKeyValue[KEY_L] << "∵" << "";
    m_mathKeyValue[KEY_M] << "∽" << "";
    m_mathKeyValue[KEY_N] << "≌" << "";
    m_mathKeyValue[KEY_O] << "∮" << "";
    m_mathKeyValue[KEY_P] << "∝" << "";
    m_mathKeyValue[KEY_Q] << "±" << "";
    m_mathKeyValue[KEY_R] << "×" << "";
    m_mathKeyValue[KEY_S] << "∨" << "";
    m_mathKeyValue[KEY_T] << "÷" << "";
    m_mathKeyValue[KEY_U] << "" << "";
    m_mathKeyValue[KEY_V] << "⌒" << "";
    m_mathKeyValue[KEY_W] << "＋" << "";
    m_mathKeyValue[KEY_X] << "∥" << "";
    m_mathKeyValue[KEY_Y] << "／" << "";
    m_mathKeyValue[KEY_Z] << "⊥" << "";
    m_mathKeyValue[KEY_BACKQUOTE] << "≈" << "";    // 反引号
    m_mathKeyValue[KEY_SUB] << "∷" << "";          // 减号
    m_mathKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_mathKeyValue[KEY_LEFT_BRACKET] << "∞" << ""; // 左中括号
    m_mathKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_mathKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_mathKeyValue[KEY_SEMICOLON] << "∴" << "";    // 分号
    m_mathKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_mathKeyValue[KEY_COMMA] << "" << "";         // 逗号
    m_mathKeyValue[KEY_PERIOD] << "√" << "";       // 句号
    m_mathKeyValue[KEY_SLASH] << "" << "";         // 斜杠

    /********************************* 单位符号　*********************************/
    m_unitKeyValue.resize(KEY_SYMBOL_NUM);
    m_unitKeyValue[KEY_0] << "¤" << "";
    m_unitKeyValue[KEY_1] << "°" << "";
    m_unitKeyValue[KEY_2] << "′" << "";
    m_unitKeyValue[KEY_3] << "″" << "";
    m_unitKeyValue[KEY_4] << "＄" << "";
    m_unitKeyValue[KEY_5] << "￡" << "";
    m_unitKeyValue[KEY_6] << "￥" << "";
    m_unitKeyValue[KEY_7] << "‰" << "";
    m_unitKeyValue[KEY_8] << "％" << "";
    m_unitKeyValue[KEY_9] << "℃" << "";
    m_unitKeyValue[KEY_A] << "百" << "佰";
    m_unitKeyValue[KEY_B] << "" << "";
    m_unitKeyValue[KEY_C] << "毫" << "";
    m_unitKeyValue[KEY_D] << "万" << "";
    m_unitKeyValue[KEY_E] << "二" << "贰";
    m_unitKeyValue[KEY_F] << "亿" << "";
    m_unitKeyValue[KEY_G] << "兆" << "";
    m_unitKeyValue[KEY_H] << "吉" << "";
    m_unitKeyValue[KEY_I] << "七" << "柒";
    m_unitKeyValue[KEY_J] << "太" << "";
    m_unitKeyValue[KEY_K] << "拍" << "";
    m_unitKeyValue[KEY_L] << "艾" << "";
    m_unitKeyValue[KEY_M] << "" << "";
    m_unitKeyValue[KEY_N] << "" << "";
    m_unitKeyValue[KEY_O] << "八" << "捌";
    m_unitKeyValue[KEY_P] << "九" << "玖";
    m_unitKeyValue[KEY_Q] << "○" << "零";
    m_unitKeyValue[KEY_R] << "三" << "叁";
    m_unitKeyValue[KEY_S] << "千" << "仟";
    m_unitKeyValue[KEY_T] << "四" << "肆";
    m_unitKeyValue[KEY_U] << "六" << "陆";
    m_unitKeyValue[KEY_V] << "微" << "";
    m_unitKeyValue[KEY_W] << "一" << "壹";
    m_unitKeyValue[KEY_X] << "厘" << "";
    m_unitKeyValue[KEY_Y] << "五" << "伍";
    m_unitKeyValue[KEY_Z] << "分" << "";
    m_unitKeyValue[KEY_BACKQUOTE] << "" << "";        // 反引号
    m_unitKeyValue[KEY_SUB] << "￠" << "";            // 减号
    m_unitKeyValue[KEY_EQUAL] << "" << "";            // 等号
    m_unitKeyValue[KEY_LEFT_BRACKET] << "十" << "拾"; // 左中括号
    m_unitKeyValue[KEY_RIGHT_BRACKET] << "" << "";    // 右中括号
    m_unitKeyValue[KEY_BACKSLASH] << "" << "";        // 反斜杠
    m_unitKeyValue[KEY_SEMICOLON] << "" << "";        // 分号
    m_unitKeyValue[KEY_QUOTE] << "" << "";            // 引号
    m_unitKeyValue[KEY_COMMA] << "" << "";            // 逗号
    m_unitKeyValue[KEY_PERIOD] << "" << "";           // 句号
    m_unitKeyValue[KEY_SLASH] << "" << "";            // 斜杠

    /********************************* 制表符号　*********************************/
    m_tabsKeyValue.resize(KEY_SYMBOL_NUM);
    m_tabsKeyValue[KEY_0] << "┄" << "┅";
    m_tabsKeyValue[KEY_1] << "┍" << "┕";
    m_tabsKeyValue[KEY_2] << "┎" << "┖";
    m_tabsKeyValue[KEY_3] << "┏" << "┗";
    m_tabsKeyValue[KEY_4] << "┐" << "┘";
    m_tabsKeyValue[KEY_5] << "┑" << "┙";
    m_tabsKeyValue[KEY_6] << "┒" << "┚";
    m_tabsKeyValue[KEY_7] << "┓" << "┛";
    m_tabsKeyValue[KEY_8] << "" << "";
    m_tabsKeyValue[KEY_9] << "─" << "━";
    m_tabsKeyValue[KEY_A] << "┬" << "┴";
    m_tabsKeyValue[KEY_B] << "╀" << "╈";
    m_tabsKeyValue[KEY_C] << "┾" << "╆";
    m_tabsKeyValue[KEY_D] << "┮" << "┶";
    m_tabsKeyValue[KEY_E] << "┞" << "┦";
    m_tabsKeyValue[KEY_F] << "┯" << "┷";
    m_tabsKeyValue[KEY_G] << "┰" << "┸";
    m_tabsKeyValue[KEY_H] << "┱" << "┹";
    m_tabsKeyValue[KEY_I] << "┣" << "┫";
    m_tabsKeyValue[KEY_J] << "┲" << "┺";
    m_tabsKeyValue[KEY_K] << "┳" << "┻";
    m_tabsKeyValue[KEY_L] << "" << "";
    m_tabsKeyValue[KEY_M] << "╂" << "╊";
    m_tabsKeyValue[KEY_N] << "╁" << "╉";
    m_tabsKeyValue[KEY_O] << "" << "";
    m_tabsKeyValue[KEY_P] << "│" << "┃";
    m_tabsKeyValue[KEY_Q] << "├" << "┤";
    m_tabsKeyValue[KEY_R] << "┟" << "┧";
    m_tabsKeyValue[KEY_S] << "┭" << "┵";
    m_tabsKeyValue[KEY_T] << "┠" << "┨";
    m_tabsKeyValue[KEY_U] << "┢" << "┪";
    m_tabsKeyValue[KEY_V] << "┿" << "╇";
    m_tabsKeyValue[KEY_W] << "┝" << "┥";
    m_tabsKeyValue[KEY_X] << "┽" << "╅";
    m_tabsKeyValue[KEY_Y] << "┡" << "┩";
    m_tabsKeyValue[KEY_Z] << "┼" << "╄";
    m_tabsKeyValue[KEY_BACKQUOTE] << "┌" << "└";     // 反引号
    m_tabsKeyValue[KEY_SUB] << "┈" << "┉";           // 减号
    m_tabsKeyValue[KEY_EQUAL] << "" << "";           // 等号
    m_tabsKeyValue[KEY_LEFT_BRACKET] << "┆" << "┇";  // 左中括号
    m_tabsKeyValue[KEY_RIGHT_BRACKET] << "┊" << "┋"; // 右中括号
    m_tabsKeyValue[KEY_BACKSLASH] << "" << "";       // 反斜杠
    m_tabsKeyValue[KEY_SEMICOLON] << "" << "";       // 分号
    m_tabsKeyValue[KEY_QUOTE] << "╃" << "╋";         // 引号
    m_tabsKeyValue[KEY_COMMA] << "" << "";           // 逗号
    m_tabsKeyValue[KEY_PERIOD] << "" << "";          // 句号
    m_tabsKeyValue[KEY_SLASH] << "" << "";           // 斜杠

    /********************************* 特殊符号　*********************************/
    m_specialKeyValue.resize(KEY_SYMBOL_NUM);
    m_specialKeyValue[KEY_0] << "" << "";
    m_specialKeyValue[KEY_1] << "" << "";
    m_specialKeyValue[KEY_2] << "" << "";
    m_specialKeyValue[KEY_3] << "" << "";
    m_specialKeyValue[KEY_4] << "" << "";
    m_specialKeyValue[KEY_5] << "" << "";
    m_specialKeyValue[KEY_6] << "" << "";
    m_specialKeyValue[KEY_7] << "" << "";
    m_specialKeyValue[KEY_8] << "" << "";
    m_specialKeyValue[KEY_9] << "" << "";
    m_specialKeyValue[KEY_A] << "■" << "";
    m_specialKeyValue[KEY_B] << "＾" << "";
    m_specialKeyValue[KEY_C] << "＠" << "";
    m_specialKeyValue[KEY_D] << "▲" << "";
    m_specialKeyValue[KEY_E] << "☆" << "";
    m_specialKeyValue[KEY_F] << "※" << "";
    m_specialKeyValue[KEY_G] << "→" << "";
    m_specialKeyValue[KEY_H] << "←" << "";
    m_specialKeyValue[KEY_I] << "◇" << "";
    m_specialKeyValue[KEY_J] << "↑" << "";
    m_specialKeyValue[KEY_K] << "↓" << "";
    m_specialKeyValue[KEY_L] << "〓" << "";
    m_specialKeyValue[KEY_M] << "￣" << "";
    m_specialKeyValue[KEY_N] << "＿" << "";
    m_specialKeyValue[KEY_O] << "◆" << "";
    m_specialKeyValue[KEY_P] << "□" << "";
    m_specialKeyValue[KEY_Q] << "§" << "";
    m_specialKeyValue[KEY_R] << "★" << "";
    m_specialKeyValue[KEY_S] << "△" << "";
    m_specialKeyValue[KEY_T] << "○" << "";
    m_specialKeyValue[KEY_U] << "◎" << "";
    m_specialKeyValue[KEY_V] << "＼" << "";
    m_specialKeyValue[KEY_W] << "№" << "";
    m_specialKeyValue[KEY_X] << "＆" << "";
    m_specialKeyValue[KEY_Y] << "●" << "";
    m_specialKeyValue[KEY_Z] << "＃" << "";
    m_specialKeyValue[KEY_BACKQUOTE] << "" << "";     // 反引号
    m_specialKeyValue[KEY_SUB] << "" << "";           // 减号
    m_specialKeyValue[KEY_EQUAL] << "" << "";         // 等号
    m_specialKeyValue[KEY_LEFT_BRACKET] << "" << "";  // 左中括号
    m_specialKeyValue[KEY_RIGHT_BRACKET] << "" << ""; // 右中括号
    m_specialKeyValue[KEY_BACKSLASH] << "" << "";     // 反斜杠
    m_specialKeyValue[KEY_SEMICOLON] << "" << "";     // 分号
    m_specialKeyValue[KEY_QUOTE] << "" << "";         // 引号
    m_specialKeyValue[KEY_COMMA] << "" << "";         // 逗号
    m_specialKeyValue[KEY_PERIOD] << "" << "";        // 句号
    m_specialKeyValue[KEY_SLASH] << "" << "";         // 斜杠
}

// 设置键盘的工作模式
void Keyboard::set_work_mode(VirtualKeyboardMode mode)
{
    m_vkWorkMode = mode;

    if (m_vkWorkMode >= VKM_CUSTOM_CHAR)
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
        if (m_vkWorkMode == VKM_CUSTOM_MARK)
        {
            for (int i = KEY_A; i <= KEY_Z; i++)
            {
                KeyButton *btn = static_cast<KeyButton *>(m_btnGroup.button(i));
                btn->setEnabled(false);
            }
        }
    }
}

// 更新键盘所有按键的显示内容
void Keyboard::update_keyboard_button()
{
    KeyButton *btn;
    CustomKeyValue customKeyValue;

    for (int i = 0; i < KEY_SYMBOL_NUM; i++)
    {
        btn = static_cast<KeyButton *>(m_btnGroup.button(i));

        // 输入模式
        if (m_vkWorkMode < VKM_INPUT_USER_CHAR)
        {
            if (m_vkWorkMode == VKM_INPUT_PC)
            {
                m_fixedKeyValue = &m_pcKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_GREEK)
            {
                m_fixedKeyValue = &m_greekKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_RUSSIAN)
            {
                m_fixedKeyValue = &m_russianKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_PHONETIC)
            {
                m_fixedKeyValue = &m_phoneticKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_PINYIN)
            {
                m_fixedKeyValue = &m_pinyinKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_JAPAN_FLAT)
            {
                m_fixedKeyValue = &m_japanFlatKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_JAPAN_PIECE)
            {
                m_fixedKeyValue = &m_japanPieceKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_PUNCTUATION)
            {
                m_fixedKeyValue = &m_punctuationKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_DIGITAL_ORDER)
            {
                m_fixedKeyValue = &m_digitalOrderKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_MATH)
            {
                m_fixedKeyValue = &m_mathKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_UNIT)
            {
                m_fixedKeyValue = &m_unitKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_TABS)
            {
                m_fixedKeyValue = &m_tabsKeyValue;
            }
            else if (m_vkWorkMode == VKM_INPUT_SPECIAL)
            {
                m_fixedKeyValue = &m_specialKeyValue;
            }

            Q_ASSERT(m_fixedKeyValue->size() > i);
            KeyValue keyValue = m_fixedKeyValue->at(i);
            btn->set_custom_symbol(keyValue.at(0), keyValue.at(1));
        }
        else if (m_vkWorkMode == VKM_INPUT_USER_CHAR)
        {
            customKeyValue = customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(), settings::instance().get_CoustomMark(), i, KEY_SYMBOL_NUM));
            btn->set_custom_symbol(customKeyValue.commChar, customKeyValue.shiftChar);
        }

        // 自定义模式
        else if (m_vkWorkMode == VKM_CUSTOM_CHAR)
        {
            customKeyValue = customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(), settings::instance().get_CoustomMark(), i, KEY_SYMBOL_NUM));
            btn->set_custom_symbol(customKeyValue.commChar, customKeyValue.shiftChar);
        }
        else if (m_vkWorkMode == VKM_CUSTOM_MARK)
        {
            customKeyValue = customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(), settings::instance().get_CoustomMark(), i, KEY_SYMBOL_NUM));
            btn->set_custom_symbol(customKeyValue.commMark, customKeyValue.shiftMark);
        }

        btn->update();
    }
}

// 更新当前正在自定义的按键显示内容
void Keyboard::update_customkey_button(SymbolKeyIdx keyIdx, const QString &commChar, const QString &shiftChar)
{
    KeyButton *btn = static_cast<KeyButton *>(m_btnGroup.button(keyIdx));
    btn->set_custom_symbol(commChar, shiftChar);
    btn->update();
}

void Keyboard::switch_vk(int flg)
{
    VirtualKeyboardMode vkm = m_vkWorkMode;

    if (!isHidden())
    {
        if (flg)
        {
            if (vkm == VKM_INPUT_PC)
            {
                vkm = static_cast<VirtualKeyboardMode>(static_cast<int>(VKM_CUSTOM_CHAR) - 1);
            }
            else
            {
                vkm = static_cast<VirtualKeyboardMode>(static_cast<int>(vkm) - 1);
            }
        }
        else
        {
            vkm = static_cast<VirtualKeyboardMode>(static_cast<int>(vkm) + 1);
            if (vkm >= VKM_CUSTOM_CHAR)
            {
                vkm = VKM_INPUT_PC;
            }
        }
    }

    slot_open_win(vkm);
}

void Keyboard::switch_caps_flg(int capsFlg)
{
    if (capsFlg != m_capsFlag)
    {
        report_key_event_to_x11(m_ctrlKeyValue[static_cast<SymbolKeyIdx>(KEY_CAPS) - KEY_SYMBOL_NUM], KEY_EVT_CLICKED);
    }
}

int Keyboard::get_caps_flg()
{
    return get_caps_state();
}

void Keyboard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void Keyboard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void Keyboard::mouseMoveEvent(QMouseEvent *event)
{
    if (m_vkWorkMode < VKM_CUSTOM_CHAR && m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

bool Keyboard::eventFilter(QObject *obj, QEvent *event)
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
void Keyboard::slot_virtual_keyboard_clicked(int keyIdx)
{
    if (keyIdx == KEY_ESC)
    {
        Sound::play(SOUND_BACK);
        hide();
        settings::instance().set_vkMode(-1);
        emit signal_vk_flg_changed();
        return;
    }

    // 标准键盘输入
    if (m_vkWorkMode < VKM_INPUT_USER_CHAR)
    {
        handle_fixed_keyboard_input_clicked(keyIdx);
    }

    // 用户自定义键盘字符输入
    else if (m_vkWorkMode == VKM_INPUT_USER_CHAR)
    {
        handle_userChar_keyboard_input_clicked(keyIdx);
    }

    // 用户自定义键盘符号
    else if (m_vkWorkMode == VKM_CUSTOM_CHAR || m_vkWorkMode == VKM_CUSTOM_MARK)
    {
        handle_custom_keyboard_clicked(static_cast<SymbolKeyIdx>(keyIdx), static_cast<KeyButton *>(m_btnGroup.button(keyIdx))->get_key_name());
    }
}

// 处理用户自定义键盘
void Keyboard::handle_custom_keyboard_clicked(SymbolKeyIdx keyIdx, const QString &keyName)
{
    CustomKeyValue customKeyValue;

    if (keyIdx < KEY_SYMBOL_NUM)
    {
        customKeyValue = customKeyToQt(freewb_custom_key_info_from_values(settings::instance().get_CoustomChar(), settings::instance().get_CoustomMark(), static_cast<int>(keyIdx), KEY_SYMBOL_NUM));
    }

    emit signal_custom_key_clicked(keyIdx, keyName, customKeyValue);
}

void Keyboard::update_caps_flg()
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
void Keyboard::update_shift_flg(int keyIdx)
{
    if (keyIdx == KEY_SHIFT)
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

// 处理标准键盘输入
void Keyboard::handle_fixed_keyboard_input_clicked(int keyIdx)
{
    QString value("");

    if (keyIdx < KEY_SYMBOL_NUM)
    {
        if (keyIdx >= KEY_A && keyIdx <= KEY_Z)
        {
            value = m_fixedKeyValue->at(keyIdx).at(0);
        }
        else
        {
            if (m_shiftFlag)
            {
                value = m_fixedKeyValue->at(keyIdx).at(1); // shift
            }
            else
            {
                value = m_fixedKeyValue->at(keyIdx).at(0);
            }
        }
    }
    else
    {
        value = m_ctrlKeyValue[keyIdx - KEY_SYMBOL_NUM];
    }

    update_shift_flg(keyIdx);

    if (keyIdx != KEY_SHIFT)
    {
        report_key_event_to_x11(value, KEY_EVT_CLICKED);
    }
}

// 处理用户自定义按键字符输入
void Keyboard::handle_userChar_keyboard_input_clicked(int keyIdx)
{
    QString stdKeyValue("");

    if (keyIdx < KEY_SYMBOL_NUM)
    {
        if (m_shiftFlag)
        {
            // stdKeyValue = settings::instance().get_custom_key_info_(static_cast<SymbolKeyIdx>(keyIdx)).shiftChar;
            stdKeyValue = m_pcKeyValue[keyIdx].at(1);
        }
        else
        {
            // stdKeyValue = settings::instance().get_custom_key_info_(static_cast<SymbolKeyIdx>(keyIdx)).commChar;
            stdKeyValue = m_pcKeyValue[keyIdx].at(0);
        }
    }
    else
    {
        stdKeyValue = m_ctrlKeyValue[static_cast<SymbolKeyIdx>(keyIdx) - KEY_SYMBOL_NUM];
    }

    update_shift_flg(keyIdx);

    if (keyIdx != KEY_SHIFT)
    {
        report_key_event_to_x11(stdKeyValue, KEY_EVT_CLICKED);
    }
}

void Keyboard::slot_load_setting_data()
{
    update_keyboard_button();
}

void Keyboard::slot_toggle_win()
{
    if (isHidden())
    {
        Sound::play(SOUND_ENTER);
        move(m_vkDefaultPos);
        update_keyboard_button();
        show();
        if (m_capsFlag)
        {
            ui->btnCaps->setStyleSheet(QSS_CAPS_SHIFT_FLG);
        }
        else
        {
            ui->btnCaps->setStyleSheet("");
        }
        settings::instance().set_vkMode(m_vkWorkMode);
    }
    else
    {
        Sound::play(SOUND_BACK);
        hide();
        settings::instance().set_vkMode(-1);
    }

    emit signal_vk_flg_changed();
}

void Keyboard::slot_open_win(VirtualKeyboardMode mode)
{
    if (m_vkWorkMode != mode)
    {
        set_work_mode(mode);
        emit signal_vk_mode_changed(mode);
    }

    update_keyboard_button();

    if (isHidden())
    {
        Sound::play(SOUND_ENTER);
        move(m_vkDefaultPos);
        show();
        if (m_capsFlag)
        {
            ui->btnCaps->setStyleSheet(QSS_CAPS_SHIFT_FLG);
        }
        else
        {
            ui->btnCaps->setStyleSheet("");
        }
    }

    settings::instance().set_vkMode(m_vkWorkMode);
    emit signal_vk_flg_changed();
}

void Keyboard::slot_key_clicked(int keyCode)
{
    if (keyCode == 66) // CAPS
    {
        update_caps_flg();
    }
}
