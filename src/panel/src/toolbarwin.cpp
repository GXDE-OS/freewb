#include "toolbarwin.h"

#include <QDebug>
#include <QFile>
#include <QLabel>

#include "config.h"
#include "log.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "ui_toolbarwin.h"

// 桌面工具条按钮样式表
#define QSS_BG0 QString("border-image: url(%1);").arg(m_skinData.bg0ImagePath)
#define QSS_BG1 QString("border-image: url(%1);").arg(m_skinData.bg1ImagePath)
#define QSS_MENU_EXTEND_OPEN QString("border-image: url(%1);").arg(m_skinData.stbMenuExtendBtn.closeIcoPath)
#define QSS_MENU_EXTEND_CLOSE QString("border-image: url(%1);").arg(m_skinData.stbMenuExtendBtn.openIcoPath)
#define QSS_MODE_BTN QStringLiteral("border:none;padding:0;margin:0;background:transparent;")
#define QSS_FULL_WIDTH QString("border-image: url(%1);").arg(m_skinData.stbFullHalfBtn.fullIcoPath)
#define QSS_HALF_WIDTH QString("border-image: url(%1);").arg(m_skinData.stbFullHalfBtn.halfIcoPath)
#define QSS_MARK_CN QString("border-image: url(%1);").arg(m_skinData.stbCnEnMarkBtn.cnMarkIcoPath)
#define QSS_MARK_EN QString("border-image: url(%1);").arg(m_skinData.stbCnEnMarkBtn.enMarkIcoPath)
#define QSS_SETTING QString("border-image: url(%1);").arg(m_skinData.stbSettingBtn.icoPath)
#define QSS_GENERATE QString("border-image: url(%1);").arg(m_skinData.stbGenerateBtn.icoPath)
#define QSS_SEARCH QString("border-image: url(%1);").arg(m_skinData.stbSearchBtn.icoPath)
#define QSS_CHAR_SIMPLIFIED QString("border-image: url(%1);").arg(m_skinData.stbCharFontBtn.simpIcoPath)
#define QSS_CHAR_TRADITIONAL QString("border-image: url(%1);").arg(m_skinData.stbCharFontBtn.tradIcoPath)
#define QSS_CHAR_GB QString("border-image: url(%1);").arg(m_skinData.stbCharSetBtn.gbIcoPath)
#define QSS_CHAR_GBK QString("border-image: url(%1);").arg(m_skinData.stbCharSetBtn.gbkIcoPath)
#define QSS_KEYBOARD QString("border-image: url(%1);").arg(m_skinData.stbKeyboardBtn.icoPath)

// 桌面工具条窗口样式表文件
#define QSS_FILE ":/qss/toolbar.qss"

#define ICO_KEYBOARD_MODE_CHECKED ":/image/toolbar/checked.png"
#define ICO_KEYBOARD_MODE_UNCHECKED ""

#define QSS_MENU                                                                                                                 \
    "QMenu::item{color: rgb(56, 56, 56);}"                                                                                       \
    "QMenu::item:selected{background-color:#DFDFDF;color: rgb(56, 56, 56)}"

#define QSS_TOOL_TIPS                                                                                                            \
    "color: rgb(56, 56, 56);"                                                                                                    \
    "font: 12pt \"Ubuntu\";"                                                                                                     \
    "padding: 10px;"                                                                                                             \
    "background-color: rgb(254, 255, 226);"                                                                                      \
    "border-radius: 5px;"                                                                                                        \
    "border-width: 2px;"                                                                                                         \
    "border-style: solid;"                                                                                                       \
    "border-color: rgb(200, 200, 200);"

/**********************************************　静态成员　************************************************/
// 工具条按钮默认状态值
QString ToolbarWin::s_inputMode = ToolbarWin::kEngineWbzx;
CharWidthMode ToolbarWin::s_charWidthMode = WIDTH_HALF;
MarkMode ToolbarWin::s_markMode = MARK_CN;
bool ToolbarWin::s_isTraditionalMode = false;
CharSetMode ToolbarWin::s_charSetMode = CHAR_GB;
int ToolbarWin::s_capsFlg;

// 设置输入模式
void ToolbarWin::set_input_mode(const QString &inputMode)
{
    s_inputMode = inputMode;
}

// 获取输入模式
const QString &ToolbarWin::get_input_mode()
{
    return s_inputMode;
}

// 获取全/半角模式
CharWidthMode ToolbarWin::get_char_width_mode()
{
    return s_charWidthMode;
}

// 设置字符全/半角模式
void ToolbarWin::set_char_width_mode(CharWidthMode charMode)
{
    s_charWidthMode = charMode;
}

// 设置中英文标点
void ToolbarWin::set_mark_mode(MarkMode markMode)
{
    s_markMode = markMode;
}

// 获取标点模式
MarkMode ToolbarWin::get_mark_mode()
{
    return s_markMode;
}

bool ToolbarWin::is_traditional_mode()
{
    return s_isTraditionalMode;
}

void ToolbarWin::switch_char_set_mode()
{
    s_charSetMode = (s_charSetMode == CHAR_GB) ? CHAR_GBK : CHAR_GB;
    settings::instance().set_currentCharset(s_charSetMode);
}

CharSetMode ToolbarWin::get_char_set_mode()
{
    return s_charSetMode;
}

/*****************************************************************************************/

ToolbarWin::ToolbarWin(QWidget *parent) : QWidget(parent), ui(new Ui::ToolbarWin)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint |
                   Qt::WindowStaysOnTopHint);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true); // Enables tooltips for inactive windows

    // 安装事件过滤器
    install_evt_filter();

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();
    m_extendMenuOpenState = true;
    m_mouseMoveFlag = false;

    set_input_mode(QString::fromStdString(settings::instance().get_inputMode()));
    s_isTraditionalMode = settings::instance().get_simpTradFlg();
    s_charSetMode = (CharSetMode)settings::instance().get_charSet();

    // 初始化虚拟键盘的输入模式选择菜单
    m_keyboardMenu.setStyleSheet(QSS_MENU);
    m_kbInputModeAction = new QActionGroup(this);
    m_kbPcInputMode = new QAction("PC键盘", this);
    m_kbInputGreek = new QAction("希腊字母", this);
    m_kbInputRussian = new QAction("俄文字母", this);
    m_kbInputPhonetic = new QAction("注音符号", this);
    m_kbInputPinyin = new QAction("拼音符号", this);
    m_kbInputJapanFlat = new QAction("日文平假名", this);
    m_kbInputJapanPiece = new QAction("日文片假名", this);
    m_kbInputPunctuation = new QAction("标点符号", this);
    m_kbInputDigitalOrder = new QAction("数字序号", this);
    m_kbInputMath = new QAction("数学符号", this);
    m_kbInputUnit = new QAction("单位符号", this);
    m_kbInputTabs = new QAction("制表符号", this);
    m_kbInputSpecial = new QAction("特殊符号", this);
    m_kbUserCharInputMode = new QAction("用户符号", this);
    m_kbInputModeAction->addAction(m_kbPcInputMode);
    m_kbInputModeAction->addAction(m_kbInputGreek);
    m_kbInputModeAction->addAction(m_kbInputRussian);
    m_kbInputModeAction->addAction(m_kbInputPhonetic);
    m_kbInputModeAction->addAction(m_kbInputPinyin);
    m_kbInputModeAction->addAction(m_kbInputJapanFlat);
    m_kbInputModeAction->addAction(m_kbInputJapanPiece);
    m_kbInputModeAction->addAction(m_kbInputPunctuation);
    m_kbInputModeAction->addAction(m_kbInputDigitalOrder);
    m_kbInputModeAction->addAction(m_kbInputMath);
    m_kbInputModeAction->addAction(m_kbInputUnit);
    m_kbInputModeAction->addAction(m_kbInputTabs);
    m_kbInputModeAction->addAction(m_kbInputSpecial);
    m_kbInputModeAction->addAction(m_kbUserCharInputMode);

    m_keyboardMenu.addActions(m_kbInputModeAction->actions());
    update_vk_mode_ckecked_state(VKM_INPUT_PC);
    connect(m_kbInputModeAction, SIGNAL(triggered(QAction *)), this, SLOT(slot_vk_mode_triggered(QAction *)));

    // 载入配置数据
    slot_load_setting_data();

    m_desktopSize = QApplication::desktop()->size();

    // 工具条初始化默认显示位置,注意必须要载入皮肤数据后才能知道工具条的尺寸!!!
    m_defaultPosition = QPoint(m_desktopSize.width() - size().width() - 12, m_desktopSize.height() - size().height() - 50);
    move(m_defaultPosition);

    m_tooltipsWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint |
                                 Qt::WindowDoesNotAcceptFocus);
    m_tooltipsWin.setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipsLabel = new QLabel(&m_tooltipsWin);
    m_tooltipsLabel->setStyleSheet(QSS_TOOL_TIPS);

    Qt::WindowFlags winflgs = Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint | Qt::Tool | Qt::WindowStaysOnTopHint;
    m_keyboardMenu.setWindowFlags(winflgs);

    m_hideDelayTimer.setSingleShot(true);
    connect(&m_hideDelayTimer, &QTimer::timeout, this, &ToolbarWin::slot_hide_toolbar);

    m_kimPropertyDebounceTimer.setSingleShot(true);
    m_kimPropertyDebounceTimer.setInterval(80);
    connect(&m_kimPropertyDebounceTimer, &QTimer::timeout, this, &ToolbarWin::slot_apply_pending_kim_property);

    s_capsFlg = Keyboard::get_caps_flg();
    // printf("执行Freewb时加载slot-open-toolbar\n");
}

ToolbarWin::~ToolbarWin()
{
    delete ui;
}

void ToolbarWin::slot_load_setting_data()
{
    m_autoLocate = settings::instance().get_toolbarAutoLocate(); // 工具条自动定位
    // if ( !m_autoLocate )
    // {
    //     move( m_defaultPosition );
    // }

    m_autoMenuExpand = settings::instance().get_toolbarAutoExpand(); // 工具条菜单自动扩展
    if (m_autoMenuExpand)
    {
        update_extend_menu(false);
    }
    else
    {
        update_extend_menu(true);
    }

    m_useUiAudioEffect = settings::instance().get_uiAudioEffect();    // 是否使用界面音效
    m_showRealtimeHelp = settings::instance().get_showRealtimeHelp(); // 是否显示实时帮助

    if (settings::instance().get_hideToolbar())
    {
        hide();
    }

    m_transparency = settings::instance().get_toolbarTransparency(); // 工具条透明度
    setWindowOpacity(1 - m_transparency / 100.0);

    update_mouse_hover_tips();

    // 载入皮肤
    if (m_curSkinId != toQStringUtf8(settings::instance().get_curSkinId()))
    {
        slot_load_skin(toQStringUtf8(settings::instance().get_curSkinId()));
    }
}

void ToolbarWin::slot_load_skin(const QString &skinId)
{
    if (m_curSkinId == skinId)
        return;

    m_curSkinId = skinId;
    QString skinFolder = INSTALL_DIR + "/skin/" + m_curSkinId + "/";

    if (!QFile(skinFolder + "skin.ini").exists())
    {
        qWarning() << "skin file isn't exist:" << skinFolder;
        return;
    }

    QSettings settings(skinFolder + "skin.ini", QSettings::IniFormat);

    // 背景框
    settings.beginGroup("ToolbarBg");
    m_skinData.size0 = settings.value("size0").toSize();
    m_skinData.size1 = settings.value("size1").toSize();
    m_skinData.bg0ImagePath = skinFolder + settings.value("bgImg0").toString();
    m_skinData.bg1ImagePath = skinFolder + settings.value("bgImg1").toString();
    settings.endGroup();

    // 按钮
    settings.beginGroup("ToolbarBtn");

    m_skinData.stbMenuExtendBtn.isExist = settings.value("btnExtMenuFlg").toInt();
    m_skinData.stbMenuExtendBtn.rect = settings.value("btnExtMenuGeometry").toRect();
    m_skinData.stbMenuExtendBtn.openIcoPath = skinFolder + settings.value("btnExtMenuOpenImg").toString();
    m_skinData.stbMenuExtendBtn.closeIcoPath = skinFolder + settings.value("btnExtMenuClosedImg").toString();

    m_skinData.stbLogoBtn.isExist = settings.value("btnLogoFlg").toInt();
    m_skinData.stbLogoBtn.rect = settings.value("btnLogoGeometry").toRect();
    m_skinData.stbLogoBtn.icoPath = skinFolder + settings.value("btnLogoImg").toString();

    m_skinData.stbModeBtn.isExist = settings.value("btnModeFlg").toInt();
    m_skinData.stbModeBtn.rect = settings.value("btnModeGeometry").toRect();
    m_skinData.stbModeBtn.wbFontIcoPath = skinFolder + settings.value("btnModewbFontImg").toString();
    m_skinData.stbModeBtn.wbPinyinIcoPath = skinFolder + settings.value("btnModewbPyImg").toString();
    m_skinData.stbModeBtn.stdPinyinIcoPath = skinFolder + settings.value("btnModeStdPyImg").toString();
    m_skinData.stbModeBtn.englishIcoPath = skinFolder + settings.value("btnModeEnglishImg").toString();
    m_skinData.stbModeBtn.capsIcoPath = skinFolder + settings.value("btnModeCapsImg").toString();

    m_skinData.stbFullHalfBtn.isExist = settings.value("btnCharWidthFlg").toInt();
    m_skinData.stbFullHalfBtn.rect = settings.value("btnCharWidthGeometry").toRect();
    m_skinData.stbFullHalfBtn.fullIcoPath = skinFolder + settings.value("btnCharWidthFullImg").toString();
    m_skinData.stbFullHalfBtn.halfIcoPath = skinFolder + settings.value("btnCharWidthHalfImg").toString();

    m_skinData.stbCnEnMarkBtn.isExist = settings.value("btnMarkFlg").toInt();
    m_skinData.stbCnEnMarkBtn.rect = settings.value("btnMarkGeometry").toRect();
    m_skinData.stbCnEnMarkBtn.cnMarkIcoPath = skinFolder + settings.value("btnMarkCnImg").toString();
    m_skinData.stbCnEnMarkBtn.enMarkIcoPath = skinFolder + settings.value("btnMarkEnImg").toString();

    m_skinData.stbSettingBtn.isExist = settings.value("btnSettingFlg").toInt();
    m_skinData.stbSettingBtn.rect = settings.value("btnSettingGeometry").toRect();
    m_skinData.stbSettingBtn.icoPath = skinFolder + settings.value("btnSettingCnImg").toString();

    m_skinData.stbGenerateBtn.isExist = settings.value("btnGenerateFlg").toInt();
    m_skinData.stbGenerateBtn.rect = settings.value("btnGenerateGeometry").toRect();
    m_skinData.stbGenerateBtn.icoPath = skinFolder + settings.value("btnGenerateImg").toString();

    m_skinData.stbSearchBtn.isExist = settings.value("btnSearchFlg").toInt();
    m_skinData.stbSearchBtn.rect = settings.value("btnSearchGeometry").toRect();
    m_skinData.stbSearchBtn.icoPath = skinFolder + settings.value("btnSearchImg").toString();

    m_skinData.stbCharFontBtn.isExist = settings.value("btnCharFontFlg").toInt();
    m_skinData.stbCharFontBtn.rect = settings.value("btnCharFontGeometry").toRect();
    m_skinData.stbCharFontBtn.simpIcoPath = skinFolder + settings.value("btnCharFontSimpImg").toString();
    m_skinData.stbCharFontBtn.tradIcoPath = skinFolder + settings.value("btnCharFontTradImg").toString();

    m_skinData.stbCharSetBtn.isExist = settings.value("btnCharSetFlg").toInt();
    m_skinData.stbCharSetBtn.rect = settings.value("btnCharSetGeometry").toRect();
    m_skinData.stbCharSetBtn.gbIcoPath = skinFolder + settings.value("btnCharSetGbImg").toString();
    m_skinData.stbCharSetBtn.gbkIcoPath = skinFolder + settings.value("btnCharSetGbkImg").toString();

    m_skinData.stbKeyboardBtn.isExist = settings.value("btnKeyboardFlg").toInt();
    m_skinData.stbKeyboardBtn.rect = settings.value("btnKeyboardGeometry").toRect();
    m_skinData.stbKeyboardBtn.icoPath = skinFolder + settings.value("btnKeyboardImg").toString();

    settings.endGroup();

    update_skin();
}

// 初始工具条外观
void ToolbarWin::update_skin()
{
    update_toolbar_bg();

    // LOGO按钮，没有LOGO按钮
    if (m_skinData.stbLogoBtn.isExist)
    {
    }
    else
    {
        ui->btnLogo->hide();
    }

    // 扩展菜单按钮
    if (m_skinData.stbMenuExtendBtn.isExist)
    {
        ui->btnMenuExtend->setGeometry(m_skinData.stbMenuExtendBtn.rect);
        update_extend_menu_ico();
    }
    else
    {
        ui->btnMenuExtend->hide();
    }

    // 输入模式按钮
    if (m_skinData.stbModeBtn.isExist)
    {
        ui->btnMode->setGeometry(m_skinData.stbModeBtn.rect);
        slot_update_input_mode_ico();
    }
    else
    {
        ui->btnMode->hide();
    }

    // 全半角模式切换按钮
    if (m_skinData.stbFullHalfBtn.isExist)
    {
        ui->btnCharWidth->setGeometry(m_skinData.stbFullHalfBtn.rect);
        slot_update_char_width_mode_ico();
    }
    else
    {
        ui->btnCharWidth->hide();
    }

    // 中英文标点按钮
    if (m_skinData.stbCnEnMarkBtn.isExist)
    {
        ui->btnMark->setGeometry(m_skinData.stbCnEnMarkBtn.rect);
        update_mark_mode_ico();
    }
    else
    {
        ui->btnMark->hide();
    }

    // 设置按钮
    if (m_skinData.stbSettingBtn.isExist)
    {
        ui->btnSetting->setGeometry(m_skinData.stbSettingBtn.rect);
        ui->btnSetting->setStyleSheet(QSS_SETTING);
    }
    else
    {
        ui->btnSetting->hide();
    }

    // 造词按钮
    if (m_skinData.stbGenerateBtn.isExist)
    {
        ui->btnGenerate->setGeometry(m_skinData.stbGenerateBtn.rect);
        ui->btnGenerate->setStyleSheet(QSS_GENERATE);
    }
    else
    {
        ui->btnGenerate->hide();
    }

    // 搜索按钮
    if (m_skinData.stbSearchBtn.isExist)
    {
        ui->btnSearch->setGeometry(m_skinData.stbSearchBtn.rect);
        ui->btnSearch->setStyleSheet(QSS_SEARCH);
    }
    else
    {
        ui->btnSearch->hide();
    }

    // 简体繁体切换按钮
    if (m_skinData.stbCharFontBtn.isExist)
    {
        ui->btnCharFont->setGeometry(m_skinData.stbCharFontBtn.rect);
        update_char_font_ico();
    }
    else
    {
        ui->btnCharFont->hide();
    }

    // 字符集选择按钮
    if (m_skinData.stbCharSetBtn.isExist)
    {
        ui->btnCharSet->setGeometry(m_skinData.stbCharSetBtn.rect);

        update_char_set_ico();
    }
    else
    {
        ui->btnCharSet->hide();
    }

    // 虚拟键盘按钮
    if (m_skinData.stbKeyboardBtn.isExist)
    {
        ui->btnKeyboard->setGeometry(m_skinData.stbKeyboardBtn.rect);
        ui->btnKeyboard->setStyleSheet(QSS_KEYBOARD);
    }
    else
    {
        ui->btnKeyboard->hide();
    }
}

void ToolbarWin::update_extend_menu(bool state)
{
    if (!state)
    {
        m_extendMenuOpenState = false;
        ui->btnGenerate->hide();
        ui->btnSearch->hide();
        ui->btnCharFont->hide();
        ui->btnCharSet->hide();
        ui->btnKeyboard->hide();
    }
    else
    {
        m_extendMenuOpenState = true;
        ui->btnGenerate->show();
        ui->btnSearch->show();
        ui->btnCharFont->show();
        ui->btnCharSet->show();
        ui->btnKeyboard->show();
    }

    update_extend_menu_ico();
    update_toolbar_bg();
}

void ToolbarWin::update_mouse_hover_tips()
{
    auto fmt = [](const std::string &value) -> QString
    {
        const std::string s = freewb_custom_shortcut_format(value);
        return s.empty() ? QString(_("None")) : toQStringUtf8(s);
    };
    ui->btnMenuExtend->setToolTip(_("Toggle extended menu bar"));
    ui->btnMode->setToolTip(                         QString(_("Input mode button\nShortcut: %1")).arg(fmt(settings::instance().get_switchInputMode())));
    ui->btnGenerate->setToolTip(                         QString(_("Online word creation button\nShortcut: %1"))
                             .arg(fmt(settings::instance().get_onlineAddWord())));
    ui->btnSearch->setToolTip(                         QString(_("Code and definition lookup button\nShortcut: %1"))
                             .arg(fmt(settings::instance().get_backFindCode())));
    ui->btnCharWidth->setToolTip(_("Character width toggle\nShortcut: Shift+Space"));
    ui->btnMark->setToolTip(_("Chinese/English punctuation toggle\nShortcut: Ctrl+."));
    ui->btnKeyboard->setToolTip(                         QString(_("Toggle or switch virtual keyboard\nShortcut: %1"))
                             .arg(fmt(settings::instance().get_switchVKb())));
    ui->btnSetting->setToolTip(_("Open settings"));
    ui->btnCharFont->setToolTip(                         QString(_("Simplified/Traditional output toggle\nShortcut: %1"))
                             .arg(fmt(settings::instance().get_switchChttrans())));
    ui->btnCharSet->setToolTip(                         QString(_("Character set toggle\nShortcut: %1"))
                             .arg(fmt(settings::instance().get_switchCharSet())));
}

void ToolbarWin::show_mouse_hover_tips(QWidget *widget)
{
    QString tips = widget->toolTip();
    if (tips.isEmpty())
    {
        return;
    }

    if (m_tooltipsWinShowFlg)
    {
        return;
    }

    m_tooltipsLabel->setText(tips);
    m_tooltipsLabel->adjustSize();
    m_tooltipsWin.adjustSize();

    QPoint position = QCursor::pos();
    QSize desktopSize = QApplication::desktop()->size();
    if (position.x() + m_tooltipsWin.width() > desktopSize.width())
    {
        position.setX(desktopSize.width() - m_tooltipsWin.width());
    }
    else
    {
        position.setX(position.x() + 10);
    }

    if (position.y() + m_tooltipsWin.height() > desktopSize.height() && m_tooltipsWin.height() < position.y())
    {
        position.setY(position.y() - m_tooltipsWin.height() - 10);
    }
    else
    {
        position.setY(position.y() + 10);
    }

    m_tooltipsWin.move(position);
    m_tooltipsWin.show();
    m_tooltipsWinShowFlg = true;
}

void ToolbarWin::show_keyboard_menu()
{
    // qDebug() << "show" << m_keyboardMenu.sizeHint();
    QPoint posite = QCursor::pos();
    m_keyboardMenu.move(posite);
    m_keyboardMenu.show();

    if (posite.x() + m_keyboardMenu.sizeHint().width() > m_desktopSize.width())
    {
        posite.setX(posite.x() - m_keyboardMenu.size().width());
        m_keyboardMenu.move(posite);
    }
    if (posite.y() + m_keyboardMenu.sizeHint().height() > m_desktopSize.height())
    {
        posite.setY(posite.y() - m_keyboardMenu.sizeHint().height());
        m_keyboardMenu.move(posite);
    }
}

void ToolbarWin::move_toolbar(QPoint targetPos)
{
    if (targetPos.x() < 0)
    {
        targetPos.setX(0);
    }
    else if (targetPos.x() + size().width() > m_desktopSize.width())
    {
        targetPos.setX(m_desktopSize.width() - size().width());
    }

    if (targetPos.y() < 0)
    {
        targetPos.setY(0);
    }
    else if (targetPos.y() + size().height() > m_desktopSize.height())
    {
        targetPos.setY(m_desktopSize.height() - size().height());
    }

    move(targetPos);
}

void ToolbarWin::install_evt_filter()
{
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach(QWidget * widget, widgets)
    {
        widget->installEventFilter(this);
    }

    //    ui->frameToolbar->installEventFilter( this );
    //    ui->btnMenuExtend->installEventFilter( this );
    //    ui->btnLogo->installEventFilter( this );
    //    ui->btnMode->installEventFilter( this );
    //    ui->btnGenerate->installEventFilter( this );
    //    ui->btnSearch->installEventFilter( this );
    //    ui->btnCharWidth->installEventFilter( this );
    //    ui->btnMark->installEventFilter( this );
    //    ui->btnKeyboard->installEventFilter( this );
    //    ui->btnSetting->installEventFilter( this );
    //    ui->btnCharFont->installEventFilter( this );
    //    ui->btnCharSet->installEventFilter( this );
}

bool ToolbarWin::eventFilter(QObject *obj, QEvent *event)
{
    // qDebug() << obj->objectName() << event->type();
    bool isProcessed = false;

    if (event->type() == QEvent::Enter)
    {
        if (obj == ui->frameToolbar && m_autoMenuExpand)
        {
            Sound::play(SOUND_ENTER);
            update_extend_menu(true);
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        if (obj == ui->frameToolbar && m_autoMenuExpand)
        {
            update_extend_menu(false);
        }

        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget && !widget->toolTip().isEmpty() && m_tooltipsWinShowFlg)
        {
            m_tooltipsWin.hide();
            m_tooltipsWinShowFlg = false;
        }
    }
    else if (event->type() == QEvent::ToolTip)
    {
        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget && !widget->toolTip().isEmpty() && settings::instance().get_showRealtimeHelp())
        {
            show_mouse_hover_tips(widget);
            isProcessed = true;
        }
    }
    else if (event->type() == QEvent::ContextMenu)
    {
        isProcessed = true;
        if (obj == ui->btnKeyboard)
        {
            show_keyboard_menu();
        }
        else
        {
            emit signal_open_context_menu();
        }
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *evt = static_cast<QMouseEvent *>(event);
        if (evt->button() == Qt::LeftButton)
        {
            m_mouseIsPressed = true;
            m_mouseLastPosition = evt->globalPos();
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *evt = static_cast<QMouseEvent *>(event);
        if (evt->button() == Qt::LeftButton)
        {
            m_mouseIsPressed = false;
        }
        //        else if ( evt->button() == Qt::RightButton )
        //        {
        //            isProcessed = true;
        //            if ( obj == ui->btnKeyboard )
        //            {
        //                m_keyboardMenu.exec( QCursor::pos() );
        //            }
        //            else
        //            {
        //                emit signal_open_context_menu( QCursor::pos() );
        //            }
        //        }

        if (m_mouseMoveFlag)
        {
            m_mouseMoveFlag = false;
            isProcessed = true;
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *evt = static_cast<QMouseEvent *>(event);
        if (m_mouseIsPressed)
        {
            QPoint p = evt->globalPos();
            move_toolbar(pos() + p - m_mouseLastPosition);
            m_mouseMoveFlag = true;
            m_mouseLastPosition = p;
        }
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

// 更新工具条边框与背景图片
void ToolbarWin::update_toolbar_bg()
{
    if (m_extendMenuOpenState)
    {
        resize(m_skinData.size1);
        ui->frameToolbar->setStyleSheet(QSS_BG1);
    }
    else
    {
        resize(m_skinData.size0);
        ui->frameToolbar->setGeometry(QRect(QPoint(0, 0), m_skinData.size0));
        ui->frameToolbar->setStyleSheet(QSS_BG0);
    }
}

// 更新工具条上的扩展菜单按钮图标
void ToolbarWin::update_extend_menu_ico()
{
    if (m_extendMenuOpenState)
    {
        ui->btnMenuExtend->setStyleSheet(QSS_MENU_EXTEND_OPEN);
    }
    else
    {
        ui->btnMenuExtend->setStyleSheet(QSS_MENU_EXTEND_CLOSE);
    }
}

// 更新工具条上的输入模式指示图标
void ToolbarWin::slot_update_input_mode_ico()
{
    s_capsFlg = Keyboard::get_caps_flg();
    ui->btnMode->setText(QString());

    const QString &inputMode = get_input_mode();
    QString iconPath = m_skinData.stbModeBtn.wbFontIcoPath;
    if (s_capsFlg)
    {
        iconPath = m_skinData.stbModeBtn.capsIcoPath;
    }
    else if (inputMode == kEngineWbpy)
    {
        iconPath = m_skinData.stbModeBtn.wbPinyinIcoPath;
    }
    else if (inputMode == kEnginePy)
    {
        iconPath = m_skinData.stbModeBtn.stdPinyinIcoPath;
    }
    else if (inputMode == kEngineEn)
    {
        iconPath = m_skinData.stbModeBtn.englishIcoPath;
    }

    ui->btnMode->setStyleSheet(QSS_MODE_BTN);
    const QSize iconSize = m_skinData.stbModeBtn.rect.size();
    const qreal dpr = qMax(1.0, ui->btnMode->devicePixelRatioF());
    ui->btnMode->setIcon(freewb_icon_from_skin_path(iconPath, iconSize, dpr));
    ui->btnMode->setIconSize(iconSize);
}

// 更新工具条上的全半角指示图标
void ToolbarWin::slot_update_char_width_mode_ico()
{
    if (get_char_width_mode() == WIDTH_FULL)
    {
        ui->btnCharWidth->setStyleSheet(QSS_FULL_WIDTH);
    }
    else
    {
        ui->btnCharWidth->setStyleSheet(QSS_HALF_WIDTH);
    }
}

void ToolbarWin::slot_set_traditional_mode(bool isTraditional)
{
    set_traditional_mode(isTraditional);
    emit signal_switch_chttrans();
}

// 更新工具条上的中英文标点指示图标
void ToolbarWin::update_mark_mode_ico()
{
    if (get_mark_mode() == MARK_CN)
    {
        ui->btnMark->setStyleSheet(QSS_MARK_CN);
    }
    else
    {
        ui->btnMark->setStyleSheet(QSS_MARK_EN);
    }
}

// 更新工具条上的简体繁体按钮图标
void ToolbarWin::update_char_font_ico()
{
    if (!is_traditional_mode())
    {
        ui->btnCharFont->setStyleSheet(QSS_CHAR_SIMPLIFIED);
    }
    else
    {
        ui->btnCharFont->setStyleSheet(QSS_CHAR_TRADITIONAL);
    }
}

// 更新工具条上得GB字符集图标
void ToolbarWin::update_char_set_ico()
{
    if (get_char_set_mode() == CHAR_GB)
    {
        ui->btnCharSet->setStyleSheet(QSS_CHAR_GB);
    }
    else
    {
        ui->btnCharSet->setStyleSheet(QSS_CHAR_GBK);
    }
}

// 更新虚拟键盘右键菜单中的工作模式选择图标
void ToolbarWin::update_vk_mode_ckecked_state(VirtualKeyboardMode mode)
{
    m_kbPcInputMode->setIcon(QIcon(mode == VKM_INPUT_PC ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputGreek->setIcon(QIcon(mode == VKM_INPUT_GREEK ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputRussian->setIcon(QIcon(mode == VKM_INPUT_RUSSIAN ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputPhonetic->setIcon(QIcon(mode == VKM_INPUT_PHONETIC ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputPinyin->setIcon(QIcon(mode == VKM_INPUT_PINYIN ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputJapanFlat->setIcon(QIcon(mode == VKM_INPUT_JAPAN_FLAT ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputJapanPiece->setIcon(QIcon(mode == VKM_INPUT_JAPAN_PIECE ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputPunctuation->setIcon(QIcon(mode == VKM_INPUT_PUNCTUATION ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputDigitalOrder->setIcon(
        QIcon(mode == VKM_INPUT_DIGITAL_ORDER ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputMath->setIcon(QIcon(mode == VKM_INPUT_MATH ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputUnit->setIcon(QIcon(mode == VKM_INPUT_UNIT ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputTabs->setIcon(QIcon(mode == VKM_INPUT_TABS ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbInputSpecial->setIcon(QIcon(mode == VKM_INPUT_SPECIAL ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
    m_kbUserCharInputMode->setIcon(QIcon(mode == VKM_INPUT_USER_CHAR ? ICO_KEYBOARD_MODE_CHECKED : ICO_KEYBOARD_MODE_UNCHECKED));
}

void ToolbarWin::on_btnLogo_clicked()
{
    //    qDebug() << DBG_TRACE;
}

void ToolbarWin::on_btnMenuExtend_clicked()
{
    Sound::play(SOUND_LETTER);

    if (m_extendMenuOpenState)
    {
        update_extend_menu(false);
    }
    else
    {
        update_extend_menu(true);
    }
}

void ToolbarWin::on_btnMode_clicked()
{
    Sound::play(SOUND_LETTER);
    if (Keyboard::get_caps_flg())
    {
        return;
    }
    emit signal_request_next_input_mode();
}

void ToolbarWin::fcitx_charFont_updated(const QString &param)
{
    //    qDebug() << param;
    if (param.contains("fcitx-chttrans-inactive"))
    {
        // set_char_font_mode( CHAR_SIMPLIFIED );
    }
    else if (param.contains("fcitx-chttrans-active"))
    {
        // set_char_font_mode( CHAR_TRADITIONAL );
    }

    update_char_font_ico();
}

void ToolbarWin::fcitx_charWidth_updated(const QString &param)
{
    if (param.contains("fcitx-fullwidth-active"))
    {
        set_char_width_mode(WIDTH_FULL);
    }
    else // if ( param.contains("fcitx-fullwidth-inactive") )
    {
        set_char_width_mode(WIDTH_HALF);
    }

    slot_update_char_width_mode_ico();
}

void ToolbarWin::fcitx_charMark_updated(const QString &param)
{
    // qDebug() << param;

    if (param.contains("fcitx-punc-inactive"))
    {
        set_mark_mode(MARK_EN);
    }
    else if (param.contains("fcitx-punc-active"))
    {
        set_mark_mode(MARK_CN);
    }

    update_mark_mode_ico();
    emit signal_btn_mark_clicked(); // 通知输入面板同步更新标点模式
}

void ToolbarWin::on_btnGenerate_clicked()
{
    Sound::play(SOUND_LETTER);
    emit signal_open_generate_word_dialog("", "");
}

void ToolbarWin::on_btnSearch_clicked()
{
    Sound::play(SOUND_LETTER);
    emit signal_open_dict_query_win("");
}

void ToolbarWin::on_btnCharWidth_clicked()
{
    Sound::play(SOUND_LETTER);
    update_char_width_mode_ico(s_charWidthMode);
    emit signal_fcitx_switch_char_width("/Fcitx/fullwidth");
}

void ToolbarWin::update_char_width_mode_ico(CharWidthMode charWidth)
{
    // puts("width full half");
    s_charWidthMode = s_charWidthMode == WIDTH_FULL ? s_charWidthMode = WIDTH_HALF : s_charWidthMode = WIDTH_FULL;

    if (s_charWidthMode == WIDTH_FULL)
    {
        ui->btnCharWidth->setStyleSheet(QSS_FULL_WIDTH);
    }
    else
    {
        ui->btnCharWidth->setStyleSheet(QSS_HALF_WIDTH);
    }

    emit signal_btn_charWidth_clicked();
}

void ToolbarWin::on_btnMark_clicked()
{
    Sound::play(SOUND_LETTER);

    update_mark_mode_ico(s_markMode);
    emit signal_fcitx_switch_mark("/Fcitx/punc");
}

void ToolbarWin::update_mark_mode_ico(MarkMode markMode)
{
    if (s_markMode == MARK_CN)
        s_markMode = MARK_EN;
    else
        s_markMode = MARK_CN;

    if (s_markMode == MARK_CN)
    {
        ui->btnMark->setStyleSheet(QSS_MARK_CN);
    }
    else
    {
        ui->btnMark->setStyleSheet(QSS_MARK_EN);
    }

    emit signal_btn_mark_clicked();
}

void ToolbarWin::set_traditional_mode(bool isTraditional)
{
    s_isTraditionalMode = isTraditional;
    update_char_font_ico();
    slot_update_input_mode_ico();
}

void ToolbarWin::switch_char_set()
{
    switch_char_set_mode();
    update_char_set_ico();
}

void ToolbarWin::on_btnCharFont_clicked()
{
    Sound::play(SOUND_LETTER);

    slot_set_traditional_mode(!s_isTraditionalMode);
    emit signal_traditional_mode_changed(s_isTraditionalMode);
}

void ToolbarWin::on_btnCharSet_clicked()
{
    Sound::play(SOUND_LETTER);
    switch_char_set_mode();
    update_char_set_ico();

    emit signal_switch_char_set();
}

void ToolbarWin::on_btnKeyboard_clicked()
{
    emit signal_toggle_vk();
}

// 弹出设置界面
void ToolbarWin::on_btnSetting_clicked()
{
    Sound::play(SOUND_LETTER);
    emit signal_open_setting_win();
}

// prop: 切换输入法时提示的输入法注册的相关属性信息
void ToolbarWin::slot_kim_RegisterProperties(const QStringList &prop)
{

    // printf("ToolbarWin::slot_kim_RegisterProperties:%s\n",prop[0].toUtf8().constData());
    foreach(QString param, prop)
    {
        if (param.contains("/Fcitx/chttrans:"))
        {
            // fcitx_charFont_updated( param );
        }
        else if (param.contains("/Fcitx/fullwidth:"))
        {
            fcitx_charWidth_updated(param);
        }
        else if (param.contains("/Fcitx/punc:"))
        {
            fcitx_charMark_updated(param);
        }
    }
}

// prop: 切换输入法时提示的输入法本身描述信息
void ToolbarWin::slot_kim_UpdateProperty(const QString &prop)
{
    FREEWB_DEBUG("ToolbarWin::slot_kim_UpdateProperty: prop={} (debounced)", prop.toUtf8().constData());
    m_pendingKimProperty = prop;
    m_kimPropertyDebounceTimer.start();
}

void ToolbarWin::slot_apply_pending_kim_property()
{
    const QString &prop = m_pendingKimProperty;
    FREEWB_DEBUG("ToolbarWin::slot_apply_pending_kim_property: prop={}", prop.toUtf8().constData());
    FREEWB_DEBUG("switch ime: {}", prop.toUtf8().constData());
    if (prop.contains("/Fcitx/im:Freewb") || prop.contains("/Fcitx/im:极点五笔"))
    {
        if (!settings::instance().get_hideToolbar())
        {
            FREEWB_DEBUG("show toolbar for freewb im");
            show();
        }
    }
    else if (prop.contains("/Fcitx/im:"))
    {
        FREEWB_DEBUG("hide toolbar for non-freewb im");
        hide();
    }
}

void ToolbarWin::slot_vk_mode_changed(VirtualKeyboardMode vkMode)
{
    update_vk_mode_ckecked_state(vkMode);
}

void ToolbarWin::slot_kb_caps_changed(int capsFlag)
{
    s_capsFlg = capsFlag;
    slot_update_input_mode_ico();
}

void ToolbarWin::slot_button_pressed(int button)
{
    Q_UNUSED(button);
    if (m_keyboardMenu.isVisible())
    {
        if (!m_keyboardMenu.geometry().adjusted(-5, -5, 5, 5).contains(QCursor::pos()))
        {
            m_keyboardMenu.close();
        }
    }
}

void ToolbarWin::slot_key_pressed(int key)
{
    Q_UNUSED(key);
    if (m_keyboardMenu.isVisible())
    {
        m_keyboardMenu.close();
    }
}

void ToolbarWin::slot_hide_toolbar()
{
    hide();
}

void ToolbarWin::slot_vk_mode_triggered(QAction *action)
{
    //    qDebug() << DBG_TRACE;
    m_keyboardMenu.close();

    VirtualKeyboardMode vkm = VKM_INPUT_PC;

    if (action == m_kbPcInputMode)
    {
        vkm = VKM_INPUT_PC;
    }
    else if (action == m_kbInputGreek)
    {
        vkm = VKM_INPUT_GREEK;
    }
    else if (action == m_kbInputRussian)
    {
        vkm = VKM_INPUT_RUSSIAN;
    }
    else if (action == m_kbInputPhonetic)
    {
        vkm = VKM_INPUT_PHONETIC;
    }
    else if (action == m_kbInputPinyin)
    {
        vkm = VKM_INPUT_PINYIN;
    }
    else if (action == m_kbInputJapanFlat)
    {
        vkm = VKM_INPUT_JAPAN_FLAT;
    }
    else if (action == m_kbInputJapanPiece)
    {
        vkm = VKM_INPUT_JAPAN_PIECE;
    }
    else if (action == m_kbInputPunctuation)
    {
        vkm = VKM_INPUT_PUNCTUATION;
    }
    else if (action == m_kbInputDigitalOrder)
    {
        vkm = VKM_INPUT_DIGITAL_ORDER;
    }
    else if (action == m_kbInputMath)
    {
        vkm = VKM_INPUT_MATH;
    }
    else if (action == m_kbInputUnit)
    {
        vkm = VKM_INPUT_UNIT;
    }
    else if (action == m_kbInputTabs)
    {
        vkm = VKM_INPUT_TABS;
    }
    else if (action == m_kbInputSpecial)
    {
        vkm = VKM_INPUT_SPECIAL;
    }
    else if (action == m_kbUserCharInputMode)
    {
        vkm = VKM_INPUT_USER_CHAR;
    }

    update_vk_mode_ckecked_state(vkm);

    emit signal_open_vk(vkm);
}
