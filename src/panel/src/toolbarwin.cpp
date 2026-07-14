#include "toolbarwin.h"

#include <QFile>
#include <QGuiApplication>
#include <QLabel>
#include <QScreen>
#include <QWindow>

#include "config.h"
#include "log.h"
#include "screenhelper.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "ui_toolbarwin.h"
#include "ukuiwaylandhelper.h"

// 桌面工具条按钮样式表
#define QSS_BG0 QString("border-image: url(%1);").arg(Skin::instance().toolbar().bg0ImagePath)
#define QSS_BG1 QString("border-image: url(%1);").arg(Skin::instance().toolbar().bg1ImagePath)
#define QSS_MENU_EXTEND_OPEN QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbMenuExtendBtn.closeIcoPath)
#define QSS_MENU_EXTEND_CLOSE QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbMenuExtendBtn.openIcoPath)
#define QSS_MODE_BTN QStringLiteral("border:none;padding:0;margin:0;background:transparent;")
#define QSS_FULL_WIDTH QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbFullHalfBtn.fullIcoPath)
#define QSS_HALF_WIDTH QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbFullHalfBtn.halfIcoPath)
#define QSS_MARK_CN QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCnEnMarkBtn.cnMarkIcoPath)
#define QSS_MARK_EN QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCnEnMarkBtn.enMarkIcoPath)
#define QSS_SETTING QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbSettingBtn.icoPath)
#define QSS_GENERATE QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbGenerateBtn.icoPath)
#define QSS_SEARCH QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbSearchBtn.icoPath)
#define QSS_CHAR_SIMPLIFIED QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCharFontBtn.simpIcoPath)
#define QSS_CHAR_TRADITIONAL QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCharFontBtn.tradIcoPath)
#define QSS_CHAR_GB QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCharSetBtn.gbIcoPath)
#define QSS_CHAR_GBK QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbCharSetBtn.gbkIcoPath)
#define QSS_KEYBOARD QString("border-image: url(%1);").arg(Skin::instance().toolbar().stbKeyboardBtn.icoPath)

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

MarkMode ToolbarWin::effective_mark_mode()
{
    return (s_inputMode == kEngineEn) ? MARK_EN : s_markMode;
}

bool ToolbarWin::is_traditional_mode()
{
    return s_isTraditionalMode;
}

void ToolbarWin::switch_char_set_mode()
{
    s_charSetMode = (s_charSetMode == CHAR_GB) ? CHAR_GBK : CHAR_GB;
}

CharSetMode ToolbarWin::get_char_set_mode()
{
    return s_charSetMode;
}

/*****************************************************************************************/

ToolbarWin::ToolbarWin(QWidget *parent) : QWidget(parent), ui(new Ui::ToolbarWin), m_keyboardMenu(this)
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
    // 简繁/GB/全半角/中英标点为运行态：仅启动时读一次配置初值，之后由引擎 Sync/Show 与本地切换维护。
    s_isTraditionalMode = settings::instance().get_simpTradFlg();
    s_charSetMode = static_cast<CharSetMode>(settings::instance().get_charSet());
    s_charWidthMode = settings::instance().get_fullWidthFlg() ? WIDTH_FULL : WIDTH_HALF;
    s_markMode = settings::instance().get_chinesePunc() ? MARK_CN : MARK_EN;

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

    QPoint anchor(0, 0);
    if (const QScreen *primary = QGuiApplication::primaryScreen())
    {
        anchor = primary->availableGeometry().center();
    }
    const QRect geo = freewb::ScreenHelper::availableGeometryAt(anchor);
    if (!geo.isNull())
    {
        m_defaultPosition = QPoint(geo.x() + geo.width() - size().width() - 12, geo.y() + geo.height() - size().height() - 50);
    }
    else
    {
        m_defaultPosition = QPoint(0, 0);
    }
    move(m_defaultPosition);

    m_tooltipsWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint |
                                 Qt::WindowDoesNotAcceptFocus);
    m_tooltipsWin.setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipsLabel = new QLabel(&m_tooltipsWin);
    m_tooltipsLabel->setStyleSheet(QSS_TOOL_TIPS);

    m_toolbarCmdDebounceTimer.setSingleShot(true);
    m_toolbarCmdDebounceTimer.setInterval(80);
    connect(&m_toolbarCmdDebounceTimer, &QTimer::timeout, this, &ToolbarWin::slot_apply_pending_toolbar_cmd);

    connect(&m_keyboardMenu, &QMenu::aboutToShow, this, [this]() { m_keyboardMenuVisible = true; });
    connect(&m_keyboardMenu, &QMenu::aboutToHide, this, [this]() { m_keyboardMenuVisible = false; });

    s_capsFlg = Keyboard::get_caps_flg();

    freewb::UkuiWaylandHelper::applyInputPanelHints(this);
    freewb::UkuiWaylandHelper::applyInputPanelHints(&m_tooltipsWin);
    freewb::UkuiWaylandHelper::applyInputPanelHints(&m_keyboardMenu);
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
    const QString curSkinId = toQStringUtf8(settings::instance().get_curSkinId());
    if (Skin::instance().currentSkinId() != curSkinId)
    {
        slot_load_skin(curSkinId);
    }
    else
    {
        update_skin();
    }
}

void ToolbarWin::slot_load_skin(const QString &skinId)
{
    if (Skin::instance().load(skinId))
    {
        update_skin();
    }
}

// 初始工具条外观
void ToolbarWin::update_skin()
{
    update_toolbar_bg();

    // LOGO按钮，没有LOGO按钮
    if (Skin::instance().toolbar().stbLogoBtn.isExist)
    {
    }
    else
    {
        ui->btnLogo->hide();
    }

    // 扩展菜单按钮
    if (Skin::instance().toolbar().stbMenuExtendBtn.isExist)
    {
        ui->btnMenuExtend->setGeometry(Skin::instance().toolbar().stbMenuExtendBtn.rect);
        update_extend_menu_ico();
    }
    else
    {
        ui->btnMenuExtend->hide();
    }

    // 输入模式按钮
    if (Skin::instance().toolbar().stbModeBtn.isExist)
    {
        ui->btnMode->setGeometry(Skin::instance().toolbar().stbModeBtn.rect);
        slot_update_input_mode_ico();
    }
    else
    {
        ui->btnMode->hide();
    }

    // 全半角模式切换按钮
    if (Skin::instance().toolbar().stbFullHalfBtn.isExist)
    {
        ui->btnCharWidth->setGeometry(Skin::instance().toolbar().stbFullHalfBtn.rect);
        slot_update_char_width_mode_ico();
    }
    else
    {
        ui->btnCharWidth->hide();
    }

    // 中英文标点按钮
    if (Skin::instance().toolbar().stbCnEnMarkBtn.isExist)
    {
        ui->btnMark->setGeometry(Skin::instance().toolbar().stbCnEnMarkBtn.rect);
        update_mark_mode_ico();
    }
    else
    {
        ui->btnMark->hide();
    }

    // 设置按钮
    if (Skin::instance().toolbar().stbSettingBtn.isExist)
    {
        ui->btnSetting->setGeometry(Skin::instance().toolbar().stbSettingBtn.rect);
        ui->btnSetting->setStyleSheet(QSS_SETTING);
    }
    else
    {
        ui->btnSetting->hide();
    }

    // 造词按钮
    if (Skin::instance().toolbar().stbGenerateBtn.isExist)
    {
        ui->btnGenerate->setGeometry(Skin::instance().toolbar().stbGenerateBtn.rect);
        ui->btnGenerate->setStyleSheet(QSS_GENERATE);
    }
    else
    {
        ui->btnGenerate->hide();
    }

    // 搜索按钮
    if (Skin::instance().toolbar().stbSearchBtn.isExist)
    {
        ui->btnSearch->setGeometry(Skin::instance().toolbar().stbSearchBtn.rect);
        ui->btnSearch->setStyleSheet(QSS_SEARCH);
    }
    else
    {
        ui->btnSearch->hide();
    }

    // 简体繁体切换按钮
    if (Skin::instance().toolbar().stbCharFontBtn.isExist)
    {
        ui->btnCharFont->setGeometry(Skin::instance().toolbar().stbCharFontBtn.rect);
        update_char_font_ico();
    }
    else
    {
        ui->btnCharFont->hide();
    }

    // 字符集选择按钮
    if (Skin::instance().toolbar().stbCharSetBtn.isExist)
    {
        ui->btnCharSet->setGeometry(Skin::instance().toolbar().stbCharSetBtn.rect);

        update_char_set_ico();
    }
    else
    {
        ui->btnCharSet->hide();
    }

    // 虚拟键盘按钮
    if (Skin::instance().toolbar().stbKeyboardBtn.isExist)
    {
        ui->btnKeyboard->setGeometry(Skin::instance().toolbar().stbKeyboardBtn.rect);
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
    ui->btnMode->setToolTip(QString(_("Input mode button\nShortcut: %1")).arg(fmt(settings::instance().get_switchInputMode())));
    ui->btnGenerate->setToolTip(
        QString(_("Online word creation button\nShortcut: %1")).arg(fmt(settings::instance().get_onlineAddWord())));
    ui->btnSearch->setToolTip(
        QString(_("Code and definition lookup button\nShortcut: %1")).arg(fmt(settings::instance().get_backFindCode())));
    ui->btnCharWidth->setToolTip(_("Character width toggle\nShortcut: Shift+Space"));
    ui->btnMark->setToolTip(_("Chinese/English punctuation toggle\nShortcut: Ctrl+."));
    ui->btnKeyboard->setToolTip(
        QString(_("Toggle or switch virtual keyboard\nShortcut: %1")).arg(fmt(settings::instance().get_switchVKb())));
    ui->btnSetting->setToolTip(_("Open settings"));
    ui->btnCharFont->setToolTip(
        QString(_("Simplified/Traditional output toggle\nShortcut: %1")).arg(fmt(settings::instance().get_switchChttrans())));
    ui->btnCharSet->setToolTip(
        QString(_("Character set toggle\nShortcut: %1")).arg(fmt(settings::instance().get_switchCharSet())));
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
    const QRect desktopBounds = freewb::ScreenHelper::availableGeometryAt(position);
    const int desktopRight = desktopBounds.x() + desktopBounds.width();
    const int desktopBottom = desktopBounds.y() + desktopBounds.height();

    if (position.x() + m_tooltipsWin.width() > desktopRight)
    {
        position.setX(desktopRight - m_tooltipsWin.width());
    }
    else
    {
        position.setX(position.x() + 10);
    }

    if (position.y() + m_tooltipsWin.height() > desktopBottom && m_tooltipsWin.height() < position.y())
    {
        position.setY(position.y() - m_tooltipsWin.height() - 10);
    }
    else
    {
        position.setY(position.y() + 10);
    }

    m_tooltipsWin.move(freewb::ScreenHelper::clampTopLeft(position, m_tooltipsWin.size()));
    m_tooltipsWin.show();
    m_tooltipsWinShowFlg = true;
}

void ToolbarWin::show_keyboard_menu()
{
    QPoint pos = QCursor::pos();

    winId();
    m_keyboardMenu.winId();
    QWindow *parentWindow = windowHandle();
    QWindow *menuWindow = m_keyboardMenu.windowHandle();
    if (parentWindow && menuWindow)
    {
        menuWindow->setTransientParent(parentWindow);
    }
    m_keyboardMenu.popup(pos);
}

void ToolbarWin::move_toolbar(QPoint targetPos)
{
    move(freewb::ScreenHelper::clampTopLeftToUnitedDesktop(targetPos, size()));
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
        resize(Skin::instance().toolbar().size1);
        ui->frameToolbar->setStyleSheet(QSS_BG1);
    }
    else
    {
        resize(Skin::instance().toolbar().size0);
        ui->frameToolbar->setGeometry(QRect(QPoint(0, 0), Skin::instance().toolbar().size0));
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
    QString iconPath = Skin::instance().toolbar().stbModeBtn.wbFontIcoPath;
    if (s_capsFlg)
    {
        iconPath = Skin::instance().toolbar().stbModeBtn.capsIcoPath;
    }
    else if (inputMode == kEngineWbpy)
    {
        iconPath = Skin::instance().toolbar().stbModeBtn.wbPinyinIcoPath;
    }
    else if (inputMode == kEnginePy)
    {
        iconPath = Skin::instance().toolbar().stbModeBtn.stdPinyinIcoPath;
    }
    else if (inputMode == kEngineEn)
    {
        iconPath = Skin::instance().toolbar().stbModeBtn.englishIcoPath;
    }

    ui->btnMode->setStyleSheet(QSS_MODE_BTN);
    const QSize iconSize = Skin::instance().toolbar().stbModeBtn.rect.size();
    const qreal dpr = qMax(1.0, ui->btnMode->devicePixelRatioF());
    ui->btnMode->setIcon(freewb_icon_from_skin_path(iconPath, iconSize, dpr));
    ui->btnMode->setIconSize(iconSize);
    update_mark_mode_ico();
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

    emit signal_btn_charWidth_clicked();
}

void ToolbarWin::slot_set_traditional_mode(bool isTraditional)
{
    set_traditional_mode(isTraditional);
    emit signal_switch_chttrans();
}

void ToolbarWin::slot_update_mark_mode_ico()
{
    if (s_inputMode != kEngineEn)
    {
        s_markMode = (s_markMode == MARK_CN) ? MARK_EN : MARK_CN;
    }
    update_mark_mode_ico();
}

void ToolbarWin::update_mark_mode_ico()
{
    if (effective_mark_mode() == MARK_CN)
    {
        ui->btnMark->setStyleSheet(QSS_MARK_CN);
    }
    else
    {
        ui->btnMark->setStyleSheet(QSS_MARK_EN);
    }
    ui->btnMark->setEnabled(s_inputMode != kEngineEn);
    emit signal_btn_mark_clicked();
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

    if (param.contains("fcitx-punc-inactive"))
    {
        set_mark_mode(MARK_EN);
    }
    else if (param.contains("fcitx-punc-active"))
    {
        set_mark_mode(MARK_CN);
    }

    update_mark_mode_ico();
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
    if (s_inputMode == kEngineEn)
    {
        return;
    }

    Sound::play(SOUND_LETTER);

    s_markMode = (s_markMode == MARK_CN) ? MARK_EN : MARK_CN;
    update_mark_mode_ico();
    emit signal_fcitx_switch_mark("/Fcitx/punc");
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

void ToolbarWin::slot_update_toolbar_properties(const QString &engineName, bool traditional, int charSet, bool fullWidth,
                                                bool chinesePunc)
{
    if (!engineName.isEmpty())
    {
        set_input_mode(engineName);
    }
    s_isTraditionalMode = traditional;
    s_charSetMode = (charSet == 0) ? CHAR_GB : CHAR_GBK;
    set_char_width_mode(fullWidth ? WIDTH_FULL : WIDTH_HALF);
    set_mark_mode(chinesePunc ? MARK_CN : MARK_EN);

    update_char_font_ico();
    update_char_set_ico();
    slot_update_char_width_mode_ico();
    slot_update_input_mode_ico();
}

void ToolbarWin::slot_show_toolbar()
{
    m_pendingToolbarVisible = true;
    m_toolbarCmdDebounceTimer.start();
}

void ToolbarWin::slot_hide_toolbar()
{
    m_pendingToolbarVisible = false;
    m_toolbarCmdDebounceTimer.start();
}

void ToolbarWin::set_context_menu(ContextMenu *contextMenu)
{
    connect(contextMenu, &ContextMenu::signal_menu_visibility_changed, this, &ToolbarWin::slot_context_menu_visibility_changed);
}

void ToolbarWin::slot_context_menu_visibility_changed(bool visible)
{
    m_contextMenuVisible = visible;
}

bool ToolbarWin::is_panel_menu_visible() const
{
    return m_contextMenuVisible || m_keyboardMenuVisible;
}

void ToolbarWin::hide()
{
    m_toolbarCmdDebounceTimer.stop();
    QWidget::hide();
}

void ToolbarWin::slot_apply_pending_toolbar_cmd()
{
    if (!m_pendingToolbarVisible)
    {
        if (is_panel_menu_visible())
        {
            FREEWB_DEBUG("skip hide toolbar while panel menu is visible");
            return;
        }
        FREEWB_DEBUG("hide toolbar");
        hide();
        return;
    }

    if (!settings::instance().get_hideToolbar())
    {
        FREEWB_DEBUG("show toolbar");
        show();
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

void ToolbarWin::slot_vk_mode_triggered(QAction *action)
{
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
