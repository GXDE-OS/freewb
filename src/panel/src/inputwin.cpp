#include "inputwin.h"

#include <QHBoxLayout>
#include <QRegExp>

#include "config.h"
#include "key.h"
#include "log.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "toolbarwin.h"
#include "ui_inputwin.h"
#include "waylandwinhelper.h"

namespace
{
QColor fwbcQColorFromSpec(const std::string &spec)
{
    return QColor(toQStringUtf8(spec));
}

QString keyTokenName(const std::string &token)
{
    if (token.empty() || token == "KEY_NONE")
        return {};
    const FreewbKeySym sym = freewb::Key::keySymFromUniqueName(token.c_str());
    if (sym == FreewbKey_None)
        return {};
    return QString::fromUtf8(freewb::Key::keySymToName(sym));
}

/* 两枚物理键拼成 "name1/name2"；任一解析失败返回空串。 */
QString pairKeyDisplayText(const std::string &first, const std::string &second)
{
    const QString a = keyTokenName(first);
    const QString b = keyTokenName(second);
    if (a.isEmpty() || b.isEmpty())
        return {};
    return QString("%1/%2").arg(a, b);
}

/* "CTRL+KEY_XXX" 配置值 → "Ctrl+<name>" 展示；解析失败或 KEY_NONE 返回空。 */
QString customShortcutDisplayText(const std::string &stored)
{
    return toQStringUtf8(freewb_custom_shortcut_format(stored));
}

/* 单键存储的是 KEY_* token（"KEY_NONE" 表示禁用）；与输入字符是否匹配。 */
bool singleShortcutMatches(QChar ch, const std::string &stored)
{
    if (stored.empty() || stored == "KEY_NONE")
        return false;
    const FreewbKeySym sym = freewb::Key::keySymFromUniqueName(stored.c_str());
    if (sym == FreewbKey_None)
        return false;
    return ch.toLatin1() == static_cast<char>(sym);
}

/* 中英切换落 INI 的是 KEY_* token；反查公共预设表取 displayName。 */
QString cnEnSwitchDisplayText(const std::string &token)
{
    const int idx = freewb_cn_en_switch_preset_index(token);
    if (idx < 0)
        return {};
    return QString::fromUtf8(freewb_cn_en_switch_presets()[static_cast<size_t>(idx)].displayName);
}

} // namespace

#define MIN_WIN_WIDTH 250 // 最小候选窗口宽度

#define QSS_IM_PROMPT "color:rgb(136, 138, 133);"

// 输入候选窗样式表
#define QSS_BG_CENTER QString("#frameText{border-image: url(%1);}").arg(m_skinData.bgCenterImagePath)
#define QSS_BG_TOP QString("#labelTop{border-image: url(%1);}").arg(m_skinData.bgTopImagePath)
#define QSS_BG_BOTTOM QString("#labelBottom{border-image: url(%1);}").arg(m_skinData.bgBottomImagePath)
#define QSS_BG_LEFT QString("#labelLeft{border-image: url(%1);}").arg(m_skinData.bgLeftImagePath)
#define QSS_BG_RIGHT QString("#labelRight{border-image: url(%1);}").arg(m_skinData.bgRightImagePath)
#define QSS_FULL_WIDTH QString("border-image: url(%1);").arg(m_skinData.fullIcoPath)
#define QSS_HALF_WIDTH QString("border-image: url(%1);").arg(m_skinData.halfIcoPath)
#define QSS_MARK_CN QString("border-image: url(%1);").arg(m_skinData.cnMarkIcoPath)
#define QSS_MARK_EN QString("border-image: url(%1);").arg(m_skinData.enMarkIcoPath)

#define QSS_FREEIME_LOGO QString("background-image: url(%1);").arg(m_skinData.logoIcoPath)

#define QSS_PREV0_PAGE                                                                                                           \
    QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.prev0PageIcoPath)
#define QSS_PREV1_PAGE                                                                                                           \
    QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.prev1PageIcoPath)
#define QSS_NEXT0_PAGE                                                                                                           \
    QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.next0PageIcoPath)
#define QSS_NEXT1_PAGE                                                                                                           \
    QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.next1PageIcoPath)

#define QSS_DICT_INFO_WIN                                                                                                        \
    "color: rgb(56, 56, 56);"                                                                                                    \
    "font: 12pt \"Ubuntu\";"                                                                                                     \
    "padding: 10px;"                                                                                                             \
    "background-color: rgb(254, 255, 226);"                                                                                      \
    "border-radius: 5px;"                                                                                                        \
    "border-width: 2px;"                                                                                                         \
    "border-style: solid;"                                                                                                       \
    "border-color: rgb(200, 200, 200);"

InputWin::InputWin(QWidget *parent) : QWidget(parent), ui(new Ui::InputWin)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint |
                   Qt::WindowDoesNotAcceptFocus);

    setAttribute(Qt::WA_TranslucentBackground);

    m_desktopSize = QApplication::desktop()->size();
    m_defaultPosition = QPoint((m_desktopSize.width() - width() / 2), (m_desktopSize.height() - height()) / 2);
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();
    m_isUserWordMode = false;
    m_candiWordItem = 0;
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; ++i)
    {
        m_candidateItems[i] = nullptr;
        m_multiRowRows[i] = nullptr;
    }

    m_dictFindWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint |
                                 Qt::WindowDoesNotAcceptFocus);
    m_dictFindWin.setAttribute(Qt::WA_TranslucentBackground);
    m_dictFindWin.setAttribute(Qt::WA_ShowWithoutActivating, true);
    m_dictFindLabel = new QLabel(&m_dictFindWin);
    m_dictFindLabel->setWordWrap(true);
    m_dictFindLabel->setTextFormat(Qt::RichText);
    m_dictFindLabel->setStyleSheet(QSS_DICT_INFO_WIN);

    init_im_prompt_lable();
    init_ui_candidates();
    install_evt_filter();
    slot_load_setting_data();

    move(m_defaultPosition);

    connect(&m_caretBlinkTimer, &QTimer::timeout, this, &InputWin::slot_caret_blink);

    freewb::applyWaylandOverlayWindowHints(this);
    freewb::applyWaylandOverlayWindowHints(&m_dictFindWin);
}

InputWin::~InputWin()
{
    delete ui;
    if (m_labelImPrompt)
    {
        delete m_labelImPrompt;
    }
}

void InputWin::init_im_prompt_lable()
{
    m_labelImPrompt = new QLabel;
    m_labelImPrompt->setStyleSheet(QSS_IM_PROMPT);
    m_labelImPrompt->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                                    Qt::X11BypassWindowManagerHint);
    m_labelImPrompt->setAttribute(Qt::WA_TranslucentBackground, true);
    m_labelImPrompt->hide();
    freewb::applyWaylandOverlayWindowHints(m_labelImPrompt);
}

void InputWin::slot_load_setting_data()
{
    m_separateChar = freewb_separate_char_from_string(settings::instance().get_separateChar());
    m_candiCharCount = settings::instance().get_candiCharCount();
    m_cursorFollow = settings::instance().get_cursorFollow();
    m_radius = settings::instance().get_radius();

    m_transparency = settings::instance().get_transparency();
    setWindowOpacity(1 - m_transparency / 100.0);

    m_showOpRemindInfo = settings::instance().get_showOpRemindInfo();
    if (m_showOpRemindInfo && m_displayMode == CWDM_MULTI_ROW)
    {
        ui->labelPrompt->show();
    }
    else
    {
        ui->labelPrompt->hide();
    }

    m_displayMode = static_cast<CandiWinDispMode>(settings::instance().get_candiWinDispMode());
    set_display_mode(m_displayMode);

    QFont font = freewb_candi_text_qfont(settings::instance());
    QColor wordCcolor = fwbcQColorFromSpec(settings::instance().get_candiWordTextColor());
    QColor promptColor = fwbcQColorFromSpec(settings::instance().get_candiPromptTextColor());

    m_labelImPrompt->setFont(font);
    ui->labelPreEdit->setFont(font);

    QFont font2(font);
    font2.setPointSize(font.pointSize() * 0.95);
    ui->labelPrompt->setFont(font2);

    ui->labelPreEdit->setStyleSheet(QString("border-image: url(:/image/transparent.png);color:rgb(%1,%2,%3);")
                                        .arg(wordCcolor.red())
                                        .arg(wordCcolor.green())
                                        .arg(wordCcolor.blue()));
    ui->labelPrompt->setStyleSheet(QString("border-image: url(:/image/transparent.png);color:rgb(%1,%2,%3);")
                                       .arg(promptColor.red())
                                       .arg(promptColor.green())
                                       .arg(promptColor.blue()));

    CandidateItem *item;
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        item = m_candidateItems[i];
        Q_ASSERT(item);
        style_candidate_item(item, i, font, wordCcolor, promptColor);
    }

    // 载入皮肤
    if (m_curSkinId != toQStringUtf8(settings::instance().get_curSkinId()))
    {
        slot_load_skin(toQStringUtf8(settings::instance().get_curSkinId()));
    }
    else
    {
        update_skin();
    }
}

void InputWin::style_candidate_item(CandidateItem *item, int idx, const QFont &font, const QColor &wordColor,
                                    const QColor &promptColor)
{
    item->set_text_font(font);
    item->set_hover_color(qRgb(255, 0, 0));
    if (idx == 0)
    {
        item->set_word_text_color(qRgb(0, 0, 255));
    }
    else
    {
        item->set_word_text_color(wordColor);
    }
    item->set_prompt_text_color(promptColor);
    item->set_disp_max_char_count(m_candiCharCount);
}

void InputWin::init_ui_candidates()
{
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; ++i)
    {
        m_candidateItems[i] = new CandidateItem(ui->widgetCandiOneRow);
        m_candidateItems[i]->set_word_text_cursor();
        connect(m_candidateItems[i], &CandidateItem::signal_cursor_hover, this, &InputWin::slot_dict_find);
        connect(m_candidateItems[i], &CandidateItem::signal_clicked, this, [this, i]() { on_candidate_clicked(i); });
        ui->hLayoutCandiOneRow->addWidget(m_candidateItems[i]);

        m_multiRowRows[i] = new QWidget(ui->widgetCandiMultiRow);
        m_multiRowRows[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        auto *rowLayout = new QHBoxLayout(m_multiRowRows[i]);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(0);
        rowLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        rowLayout->addStretch(1);
        ui->vLayoutCandiMultiRows->addWidget(m_multiRowRows[i], 0, Qt::AlignTop);
    }

    ui->vLayoutCandiMultiRows->setAlignment(Qt::AlignTop);
    ui->hLayoutCandiMultiOuter->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    for (int i = 0; i < ui->hLayoutCandiMultiOuter->count(); ++i)
    {
        if (QLayoutItem *layoutItem = ui->hLayoutCandiMultiOuter->itemAt(i))
        {
            layoutItem->setAlignment(Qt::AlignTop);
        }
    }

    ui->hLayoutCandiOneRow->addStretch(1);

    m_btnPrevPage = new QPushButton(ui->widgetCandiOneRow);
    m_btnNextPage = new QPushButton(ui->widgetCandiOneRow);
    m_btnPrevPage->setFixedWidth(20);
    m_btnNextPage->setFixedWidth(20);
    m_btnPrevPage->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnNextPage->setCursor(QCursor(Qt::PointingHandCursor));
    ui->hLayoutCandiOneRow->addWidget(m_btnPrevPage);
    ui->hLayoutCandiOneRow->addWidget(m_btnNextPage);

    connect(m_btnPrevPage, &QPushButton::clicked, this, &InputWin::slot_btnPrevPage_clicked);
    connect(m_btnNextPage, &QPushButton::clicked, this, &InputWin::slot_btnNextPage_clicked);

    ui->widgetCandiArea->setCursor(Qt::ArrowCursor);
    ui->widgetCandiMultiRow->hide();
}

void InputWin::apply_candidate_container_layout()
{
    const bool itemsInOneRowLayout =
        m_candidateItems[0] != nullptr && m_candidateItems[0]->parentWidget() == ui->widgetCandiOneRow;

    if (m_displayMode == CWDM_ONE_ROW)
    {
        ui->widgetCandiOneRow->show();
        ui->widgetCandiMultiRow->hide();

        if (!itemsInOneRowLayout)
        {
            for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; ++i)
            {
                m_multiRowRows[i]->layout()->removeWidget(m_candidateItems[i]);
                m_candidateItems[i]->setParent(ui->widgetCandiOneRow);
                ui->hLayoutCandiOneRow->insertWidget(i, m_candidateItems[i]);
            }
            ui->hLayoutCandiMultiPage->removeWidget(m_btnPrevPage);
            ui->hLayoutCandiMultiPage->removeWidget(m_btnNextPage);
            m_btnPrevPage->setParent(ui->widgetCandiOneRow);
            m_btnNextPage->setParent(ui->widgetCandiOneRow);
            ui->hLayoutCandiOneRow->addWidget(m_btnPrevPage);
            ui->hLayoutCandiOneRow->addWidget(m_btnNextPage);
        }
    }
    else
    {
        ui->widgetCandiOneRow->hide();
        ui->widgetCandiMultiRow->show();

        if (itemsInOneRowLayout)
        {
            for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; ++i)
            {
                ui->hLayoutCandiOneRow->removeWidget(m_candidateItems[i]);
                m_candidateItems[i]->setParent(m_multiRowRows[i]);
                qobject_cast<QHBoxLayout *>(m_multiRowRows[i]->layout())->insertWidget(0, m_candidateItems[i]);
            }
            ui->hLayoutCandiOneRow->removeWidget(m_btnPrevPage);
            ui->hLayoutCandiOneRow->removeWidget(m_btnNextPage);
            m_btnPrevPage->setParent(ui->widgetCandiMultiRow);
            m_btnNextPage->setParent(ui->widgetCandiMultiRow);
            ui->hLayoutCandiMultiPage->addWidget(m_btnPrevPage);
            ui->hLayoutCandiMultiPage->addWidget(m_btnNextPage);
        }
    }

    m_btnPrevPage->show();
    m_btnNextPage->show();
}

void InputWin::update_candidate_visibility(int activeCount)
{
    activeCount = qBound(0, activeCount, MAX_CANDIDATE_WORD_COUNT);
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; ++i)
    {
        const bool visible = (i < activeCount);
        m_candidateItems[i]->setVisible(visible);
        if (m_displayMode == CWDM_MULTI_ROW)
        {
            m_multiRowRows[i]->setVisible(visible);
        }
    }
}

void InputWin::slot_load_skin(const QString &skinId)
{
    if (m_curSkinId == skinId)
        return;
    m_curSkinId = skinId;

    QString skinFolder = INSTALL_DIR + "/skin/" + m_curSkinId + "/";

    if (!QFile(skinFolder + "skin.ini").exists())
    {
        return;
    }

    QSettings settings(skinFolder + "skin.ini", QSettings::IniFormat);
    settings.beginGroup("CandidateWin");
    m_skinData.bgTopImageHeight = settings.value("bgTopImgHeight").toInt();
    m_skinData.bgBottomImageHeight = settings.value("bgBottomImgHeight").toInt();
    m_skinData.bgLeftImageWidth = settings.value("bgLeftImgWidth").toInt();
    m_skinData.bgRightImageWidth = settings.value("bgRightImgWidth").toInt();
    m_skinData.bgCenterImagePath = skinFolder + settings.value("bgCenterImg").toString();
    m_skinData.bgTopImagePath = skinFolder + settings.value("bgTopImg").toString();
    m_skinData.bgBottomImagePath = skinFolder + settings.value("bgBottomImg").toString();
    m_skinData.bgLeftImagePath = skinFolder + settings.value("bgLeftImg").toString();
    m_skinData.bgRightImagePath = skinFolder + settings.value("bgRightImg").toString();
    m_skinData.fullIcoPath = skinFolder + settings.value("fullIco").toString();
    m_skinData.halfIcoPath = skinFolder + settings.value("halfIco").toString();
    m_skinData.cnMarkIcoPath = skinFolder + settings.value("cnMarkIco").toString();
    m_skinData.enMarkIcoPath = skinFolder + settings.value("enMarkIco").toString();

    m_skinData.logoIcoPath = INSTALL_DIR + "/skin/freewb.png";

    m_skinData.prev0PageIcoPath = skinFolder + settings.value("prev0PageIco").toString();
    m_skinData.prev1PageIcoPath = skinFolder + settings.value("prev1PageIco").toString();
    m_skinData.next0PageIcoPath = skinFolder + settings.value("next0PageIco").toString();
    m_skinData.next1PageIcoPath = skinFolder + settings.value("next1PageIco").toString();
    settings.endGroup();

    update_skin();
}

void InputWin::update_skin()
{
    if (m_curSkinId == "default")
    {
        ui->frameText->setStyleSheet("");
        ui->labelTop->setFixedHeight(0);
        ui->labelBottom->setFixedHeight(0);
        ui->labelLeft->setFixedWidth(0);
        ui->labelRight->setFixedWidth(0);

        QString style;
        QColor boderColor = fwbcQColorFromSpec(settings::instance().get_borderColor());
        QColor bgColor = fwbcQColorFromSpec(settings::instance().get_bgColor());
        QColor gradienColor0 = fwbcQColorFromSpec(settings::instance().get_gradientColor0());
        QColor gradienColor1 = fwbcQColorFromSpec(settings::instance().get_gradientColor1());

        QString borderColorStyle =
            QString("border-color:rgb(%1,%2,%3);").arg(boderColor.red()).arg(boderColor.green()).arg(boderColor.blue());

        if (settings::instance().get_useGradientColor())
        {
            QString gradienColorStyle = QString("background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 "
                                                "rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));")
                                            .arg(gradienColor0.red())
                                            .arg(gradienColor0.green())
                                            .arg(gradienColor0.blue())
                                            .arg(gradienColor1.red())
                                            .arg(gradienColor1.green())
                                            .arg(gradienColor1.blue());
            style = QString("#frameBg{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(m_radius)
                        .arg(borderColorStyle)
                        .arg(gradienColorStyle);
        }
        else if (settings::instance().get_useBgImage())
        {
            QString bgImageStyle = QString("%1:url(%2);")
                                       .arg(settings::instance().get_enableTiled() ? "background-image" : "border-image")
                                       .arg(toQStringUtf8(settings::instance().get_bgImage()));

            style = QString("#frameBg{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(m_radius)
                        .arg(borderColorStyle)
                        .arg(bgImageStyle);
        }
        else
        {
            style = QString("#frameBg{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "background-color:rgb(%3,%4,%5);"
                            "}")
                        .arg(m_radius)
                        .arg(borderColorStyle)
                        .arg(bgColor.red())
                        .arg(bgColor.green())
                        .arg(bgColor.blue());
        }

        ui->frameBg->setStyleSheet(style);
    }
    else
    {
        ui->frameBg->setStyleSheet("");

        ui->labelTop->setFixedHeight(m_skinData.bgTopImageHeight);
        ui->labelBottom->setFixedHeight(m_skinData.bgBottomImageHeight);
        ui->labelLeft->setFixedWidth(m_skinData.bgLeftImageWidth);
        ui->labelRight->setFixedWidth(m_skinData.bgRightImageWidth);

        ui->frameText->setStyleSheet(QSS_BG_CENTER);
        ui->labelTop->setStyleSheet(QSS_BG_TOP);
        ui->labelBottom->setStyleSheet(QSS_BG_BOTTOM);
        ui->labelLeft->setStyleSheet(QSS_BG_LEFT);
        ui->labelRight->setStyleSheet(QSS_BG_RIGHT);
    }

    slot_update_charWidth_btn_ico();
    slot_update_mark_btn_ico();
    ui->btnLogo->hide();

    m_btnPrevPage->setStyleSheet(QSS_PREV0_PAGE);
    m_btnNextPage->setStyleSheet(QSS_NEXT0_PAGE);
}

void InputWin::install_evt_filter()
{
    QList<QWidget *> widgets = findChildren<QWidget *>();
    foreach(QWidget * widget, widgets)
    {
        widget->installEventFilter(this);
    }
}

bool InputWin::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (event->type() == QEvent::Leave)
    {
        if (obj->metaObject()->className() == QStringLiteral("CandidateItem"))
        {
            m_dictFindWin.hide();
        }
    }
    else if (event->type() == QEvent::ContextMenu)
    {
        isProcessed = true;
        emit signal_open_context_menu();
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
            QPoint mouseCurrPosition = evt->globalPos();
            move(pos() + mouseCurrPosition - m_mouseLastPosition);
            m_mouseLastPosition = mouseCurrPosition;

            m_mouseMoveFlag = true;
        }
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }
    return isProcessed;
}

void InputWin::adjust_candi_win_width()
{
    int lenPreEdtLine = 0;
    int lenCandi = 0;
    int lenPrompt = 0;
    int len = 0;

    lenPreEdtLine =
        ui->labelPreEdit->sizeHint().width() + ui->btnCharWidth->width() + ui->btnMark->width() + ui->btnLogo->width();

    if (m_displayMode == CWDM_ONE_ROW)
    {
        lenCandi = ui->widgetCandiOneRow->sizeHint().width();
    }
    else
    {
        lenCandi = ui->widgetCandiMultiRow->sizeHint().width();
    }

    lenPrompt = ui->labelPrompt->sizeHint().width();
    len = lenPreEdtLine > lenCandi ? lenPreEdtLine : lenCandi;
    len = len > lenPrompt ? len : lenPrompt;
    if (len < MIN_WIN_WIDTH)
    {
        len = MIN_WIN_WIDTH;
    }

    len += ui->horizontalLayout->contentsMargins().left() + ui->horizontalLayout->contentsMargins().right();
    len += (ui->labelLeft->width() + ui->labelRight->width());

    m_winWidth = len;
}

void InputWin::adjust_candi_win_height()
{
    int fontHeight = QFontMetrics(freewb_candi_text_qfont(settings::instance())).height() * 1.2;

    m_winHeight = fontHeight > ui->btnCharWidth->height() ? fontHeight : ui->btnCharWidth->height();

    ui->labelPreEdit->setFixedHeight(fontHeight);
    ui->labelPrompt->setFixedHeight(fontHeight);
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        CandidateItem *item = m_candidateItems[i];
        Q_ASSERT(item);
        item->setFixedHeight(fontHeight);
    }
    m_btnPrevPage->setFixedHeight(fontHeight);
    m_btnNextPage->setFixedHeight(fontHeight);

    if (m_displayMode == CWDM_ONE_ROW)
    {
        m_winHeight += fontHeight;
    }
    else if (m_displayMode == CWDM_MULTI_ROW)
    {
        m_winHeight += fontHeight * m_candiWordItem;
        if (m_showOpRemindInfo && !ui->labelPrompt->isHidden())
        {
            m_winHeight += fontHeight;
        }
        m_winHeight += 8;
    }

    // 0 0
    // printf("top=%d bot=%d\n",ui->labelTop->height() , ui->labelBottom->height());

    m_winHeight += (ui->labelTop->height() + ui->labelBottom->height());

    m_winHeight += 6;
}

// 自动调整输入候选框几何尺寸
void InputWin::auto_adjust_candi_win_geometry()
{
    adjust_candi_win_height();
    adjust_candi_win_width();

    // 调整窗口大小
    ui->frameBg->resize(m_winWidth, m_winHeight);
    resize(ui->frameBg->width(), ui->frameBg->height());

    // 调整显示位置
    int x = pos().x(), y = pos().y();
    if (x + width() > m_desktopSize.width())
    {
        x = m_desktopSize.width() - width();
        move(x, pos().y());
    }
    if (y + height() > m_desktopSize.height())
    {
        y = y - 30;
        move(x, y);
    }
}

void InputWin::show_dict_find_win()
{
    if (!settings::instance().get_showCandDictInfo())
        return;

    QPoint position = QCursor::pos();

    m_dictFindLabel->adjustSize();
    m_dictFindWin.adjustSize();

    if (position.x() + m_dictFindWin.width() > m_desktopSize.width())
    {
        position.setX(m_desktopSize.width() - m_dictFindWin.width());
    }
    else
    {
        position.setX(position.x() + 15);
    }

    if (position.y() + m_dictFindWin.height() > m_desktopSize.height() && m_dictFindWin.height() < position.y())
    {
        position.setY(position.y() - m_dictFindWin.height() - 15);
    }
    else
    {
        position.setY(position.y() + 15);
    }

    m_dictFindWin.move(position);
    m_dictFindWin.show();
    m_dictFindWin.raise();
}

void InputWin::show_user_word_operation_prompt(int addOrDel, const QString &wordText, const QString &wordCode)
{
    if (!m_isUserWordMode)
    {
        enter_user_word_mode();
    }
    QString str = wordText;
    if (str.length() > 13)
    {
        str = str.left(11) + "…" + str.right(2);
    }
    ui->labelPreEdit->setText(str + "  <font color=blue>〔 " + wordCode + " 〕</font>");
    if (addOrDel)
    {
        ui->labelPrompt->setText(_("Use ← and → to change word length, Enter to confirm, Esc to cancel, Ctrl+Enter for code"));
    }
    else
    {
        ui->labelPrompt->setText(_("Press Enter to confirm deletion, Esc to cancel"));
    }

    // 调整窗口大小
    int len1 = ui->labelPreEdit->sizeHint().width();
    int len2 = ui->labelPrompt->sizeHint().width();
    int len = len1 > len2 ? len1 : len2;
    int h = ui->labelPreEdit->sizeHint().height() + ui->labelPrompt->sizeHint().height() + 8;
    ui->frameBg->resize(len + 30, h + 15);
    resize(ui->frameBg->width(), ui->frameBg->height());

    if (!m_cursorFollow)
    {
        move(m_defaultPosition);
    }
    else
    {
        // 调整显示位置
        /*
        int f = 0, x = pos().x(), y = pos().y();
        if ( x + this->width() > m_desktopSize.width() )
        {
            x = m_desktopSize.width() - this->width();
        }
        if (  y +  this->height() > m_desktopSize.height() )
        {
            y = y - 30;
        }

        if ( f )
        {
            move( x, y );
        }
        else*/
        {
            if (m_imPromptPosition.x() < 2 || m_imPromptPosition.y() < 2)
            {
                QSize sz = QApplication::desktop()->size();
                QPoint pt = QPoint((sz.width() - len) / 2, sz.height() * 0.75);
                move(pt);
            }
            else
            {
                QPoint pt = m_imPromptPosition;
                if (pt.x() + this->width() > m_desktopSize.width())
                {
                    pt.setX(m_desktopSize.width() - this->width());
                }
                if (pt.y() + this->height() > m_desktopSize.height())
                {
                    pt.setY(pt.y() - 30);
                }

                move(pt);
            }
        }
    }
    show();
}

void InputWin::close_user_word_operation_prompt()
{
    exit_user_word_mode();
    if (m_showOpRemindInfo && m_displayMode == CWDM_MULTI_ROW)
    {
        // ui->labelPrompt->setText( "【极点五笔银河麒麟版】" );
    }
    else
    {
        ui->labelPrompt->clear();
    }
    hide();
}

void InputWin::reset()
{
    if (m_isUserWordMode)
    {
        exit_user_word_mode();
    }
    m_caretBlinkTimer.stop();
    m_preEidtText.clear();
    m_candiWordCount = 0;
    m_candiWordItem = 0;
    m_caretPos = -1;
    ui->labelPreEdit->clear();
    ui->labelPrompt->clear();
    m_labelImPrompt->clear();
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        clear_candidate_text(i);
    }
    update_candidate_visibility(0);
}

void InputWin::set_display_mode(CandiWinDispMode mode)
{
    m_displayMode = mode;
    apply_candidate_container_layout();

    if (mode == CWDM_ONE_ROW)
    {
        ui->labelPrompt->hide();
    }
    else if (mode == CWDM_MULTI_ROW)
    {
        if (m_showOpRemindInfo)
        {
            ui->labelPrompt->show();
        }
        else
        {
            ui->labelPrompt->clear();
            ui->labelPrompt->hide();
        }
    }

    update_candidate_visibility(m_candiWordItem);
}

void InputWin::enter_user_word_mode()
{
    m_isUserWordMode = true;

    ui->btnCharWidth->hide();
    ui->btnMark->hide();
    ui->widgetCandiArea->hide();

    ui->btnLogo->setStyleSheet(QSS_FREEIME_LOGO);
    ui->btnLogo->show();
    ui->labelPrompt->show();
}

void InputWin::exit_user_word_mode()
{
    m_isUserWordMode = false;

    ui->btnCharWidth->show();
    ui->btnMark->show();
    ui->widgetCandiArea->show();

    ui->btnLogo->hide();

    set_display_mode(m_displayMode);
}

// 设置候选框单元格内容
void InputWin::set_candidate_text(int idx, const QString &label, const QString &wordText, const QString &promptText)
{
    Q_ASSERT(idx < MAX_CANDIDATE_WORD_COUNT);

    CandidateItem *item = m_candidateItems[idx];
    Q_ASSERT(item);

    QString str = label;
    str.replace('.', m_separateChar);

    QString displayPrompt = promptText;
    if (m_displayMode == CWDM_ONE_ROW)
    {
        if (!promptText.isEmpty())
        {
            displayPrompt = QLatin1Char(' ') + promptText + QStringLiteral("  ");
        }
        else
        {
            displayPrompt = QStringLiteral("  ");
        }
    }

    item->set_text(str, wordText, displayPrompt);
}

void InputWin::clear_candidate_text(int idx)
{
    Q_ASSERT(idx < MAX_CANDIDATE_WORD_COUNT);
    m_candidateItems[idx]->clear_text();
}

void InputWin::handle_candiwin_op_help_info()
{
    if (m_displayMode == CWDM_MULTI_ROW && settings::instance().get_showOpRemindInfo())
    {
        set_candiwin_op_help_info();
        if (ui->labelPrompt->isHidden())
        {
            ui->labelPrompt->show();
        }
    }
}

void InputWin::set_candiwin_op_help_info()
{
    QString tips;
    quint32 oti = OTI_LOGO;

    if (m_preEidtText.isEmpty())
        return;

    if (singleShortcutMatches(m_preEidtText.at(0), settings::instance().get_tempEnglish()))
    {
        if (m_preEidtText.length() > 1 && m_candiWordCount)
        {
            oti = OTI_TEMP_ENGLISH_SELECT;
        }
        else
        {
            oti = OTI_TEMP_ENGLISH;
            if (!m_candiWordCount)
            {
                oti = OTI_TEMP_ENGLISH_EMPTY;
            }
        }
    }
    else if (m_preEidtText.at(0).isUpper())
    {
        oti = OTI_TEMP_ENGLISH;
    }
    else if (m_preEidtText.length() == 1 && singleShortcutMatches(m_preEidtText.at(0), settings::instance().get_shortcutInput()))
    {
        oti = OTI_QUICK_INPUT;
    }
    else if (m_preEidtText.length() == 1 && singleShortcutMatches(m_preEidtText.at(0), settings::instance().get_tempPinyin()))
    {
        oti = OTI_TEMP_PINYIN;
    }
    else
    {
        oti = QRandomGenerator::global()->generate() % OTI_RANDOM_NUM;
    }

    if (oti == OTI_LOGO)
    {
        tips = _("【Freewb Input Method】");
    }
    else if (oti == OTI_MOUSE_MENU)
    {
        tips = _("【Settings: right-click the window】");
    }
    else if (oti == OTI_DELETE_WORD)
    {
        tips = _("【Del+number: delete phrase】");
    }
    else if (oti == OTI_ADJUST_WORD)
    {
        tips = _("【Ctrl+number: reorder phrase】");
    }
    else if (oti == OTI_SK_SWITCH_CHAR_WIDTH)
    {
        tips = _("【Shift+Space: toggle full/half width】");
    }
    else if (oti == OTI_SK_SWITCH_MARK)
    {
        tips = _("【Ctrl+.: toggle Chinese/English punctuation】");
    }
    else if (oti == OTI_SK_BACK_FIND_CODE)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_backFindCode());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Reverse code lookup】")).arg(keyText);
    }
    else if (oti == OTI_SK_ONLINE_ADD_WORD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_onlineAddWord());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Add word online】")).arg(keyText);
    }
    else if (oti == OTI_SK_ONLINE_DEL_WORD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_onlineDelWord());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Delete word online】")).arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_KEYBOARD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchVKb());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Toggle virtual keyboard】")).arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_CHAR_SET)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchCharSet());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Switch character set】")).arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_INPUT_MODE)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchInputMode());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Switch input mode】")).arg(keyText);
    }
    //    else if ( oti == OTI_SK_SWITCH_WORD_STATE )
    //    {
    //        QString keyName = settings::instance().get_customShortcutFunc_shortcutkey_name( CSF_SWITCH_WORD_STATE );
    //        if ( keyName != "无" )
    //        {
    //            tips = QString("【%1 切换字词状态】").arg( keyName );
    //        }
    //    }
    else if (oti == OTI_SK_SWITCH_S_IN_T_OUT)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchChttrans());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Toggle simplified/traditional output】")).arg(keyText);
    }
    else if (oti == OTI_SK_SHOW_HIDE_STATUS_BAR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_showHideToolbar());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Show/hide status bar】")).arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_WORD_LEXICON)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchLexicon());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Switch lexicon】")).arg(keyText);
    }
    //    else if ( oti == OTI_SK_ADD_CHAR_AFTER_OUTPUT )
    //    {
    //        QString keyName = settings::instance().get_customShortcutFunc_shortcutkey_name( CSF_ADD_CHAR_AFTER_OUTPUT );
    //        if ( keyName != "无" )
    //        {
    //            tips = QString("【%1 输出项后加字符】").arg( keyName );
    //        }
    //    }
    else if (oti == OTI_SK_SWITCH_SKIN)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchSkin());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Switch skin】")).arg(keyText);
    }
    else if (oti == OTI_SK_QUICK_DEL_SCREEN_CHAR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_quickDelScreenItem());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Quick delete committed item】")).arg(keyText);
    }
    else if (oti == OTI_SK_MARK_AUTO_PAIR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_markAutoPair());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Auto-pair punctuation】")).arg(keyText);
    }
    else if (oti == OTI_SK_TEMP_ENGLISH)
    {
        const QString keyText = keyTokenName(settings::instance().get_tempEnglish());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Temporary English input】")).arg(keyText);
    }
    else if (oti == OTI_SK_QUICK_INPUT)
    {
        const QString keyText = keyTokenName(settings::instance().get_shortcutInput());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Shortcut phrase input】")).arg(keyText);
    }
    else if (oti == OTI_SK_TEMP_PINYIN)
    {
        const QString keyText = keyTokenName(settings::instance().get_tempPinyin());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Temporary Pinyin / rare character】")).arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_CN_EN)
    {
        const QString keyText = cnEnSwitchDisplayText(settings::instance().get_cnEnSwitch());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Switch Chinese/English】")).arg(keyText);
    }
    else if (oti == OTI_SK_RECODE_SELECT)
    {
        const QString keyText =
            pairKeyDisplayText(settings::instance().get_secondRecodeKey(), settings::instance().get_thirdRecodeKey());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Select 2nd/3rd duplicate】")).arg(keyText);
    }
    else if (oti == OTI_SK_CANDI_PAGE)
    {
        const QString keyText =
            pairKeyDisplayText(settings::instance().get_prevPageKey(), settings::instance().get_nextPageKey());
        if (!keyText.isEmpty())
            tips = QString(_("【%1 Candidate paging】")).arg(keyText);
    }

    else if (oti == OTI_TEMP_ENGLISH)
    {
        tips = _("[ Temporary English ]");
    }
    else if (oti == OTI_TEMP_ENGLISH_EMPTY)
    {
        tips = _("[ Press Enter to commit English, Esc to cancel. ]");
    }
    else if (oti == OTI_QUICK_INPUT)
    {
        tips = _("[ Shortcut input ]");
    }
    else if (oti == OTI_TEMP_PINYIN)
    {
        tips = _("[ Temporary Pinyin / rare character ]");
    }
    else if (oti == OTI_TEMP_ENGLISH_SELECT)
    {
        const QString name2 = keyTokenName(settings::instance().get_secondRecodeKey());
        const QString name3 = keyTokenName(settings::instance().get_thirdRecodeKey());
        if (!name2.isEmpty() && !name3.isEmpty())
        {
            tips = QString(_("[ Space: 1, %1: 2, %2: 3 ]")).arg(name2, name3);
        }
    }

    if (tips.isEmpty())
    {
        tips = _("【Freewb Input Method】");
    }
    ui->labelPrompt->setText(tips);
}

void InputWin::slot_update_charWidth_btn_ico()
{
    if (ToolbarWin::get_char_width_mode() == WIDTH_FULL)
    {
        ui->btnCharWidth->setStyleSheet(QSS_FULL_WIDTH);
    }
    else
    {
        ui->btnCharWidth->setStyleSheet(QSS_HALF_WIDTH);
    }
}

void InputWin::slot_update_mark_btn_ico()
{
    const bool englishMode = ToolbarWin::get_input_mode() == ToolbarWin::kEngineEn;
    if (ToolbarWin::effective_mark_mode() == MARK_CN)
    {
        ui->btnMark->setStyleSheet(QSS_MARK_CN);
    }
    else
    {
        ui->btnMark->setStyleSheet(QSS_MARK_EN);
    }
    ui->btnMark->setEnabled(!englishMode);
}

void InputWin::slot_dict_find(const QString &wordText)
{
    QString findInfo;

    findInfo = m_dictquery.dict_query_word_paraphrase(wordText);
    if (findInfo.isEmpty())
    {
        QString tmp;
        foreach(QChar wchar, wordText)
        {
            if (tmp.contains(wchar))
                continue;
            tmp += wchar;

            QStringList wbpy = m_dictquery.dict_query_word_wubi_pinyin(wchar).split(';');
            if (wbpy.length() == 1)
            {
                QString wb = wbpy.at(0);
                if (!wb.isEmpty())
                {
                    findInfo +=
                        QString("<p><font color='#ef2929'>%1</font></p>  编码：%2\n").arg(wchar).arg(wb.replace(",", "  "));
                }
            }
            else if (wbpy.length() == 2)
            {
                QString wb = wbpy.at(0);
                QString py = wbpy.at(1);
                findInfo += QString("<p><font color='#ef2929'>%1</font></p>  编码：%2\n  拼音：%3\n")
                                .arg(wchar)
                                .arg(wb.replace(",", "  "))
                                .arg(py.replace(",", "  "));
            }
        }
    }
    else
    {
        QString str = m_dictquery.rstrip(wordText);
        findInfo.insert(0, QString("<p><font color='#ef2929'>【 %1 】</font></p>\n").arg(str));
    }

    findInfo.replace("\n", "<p></p>");
    if (!findInfo.isEmpty())
    {
        m_dictFindLabel->setText(findInfo);
        show_dict_find_win();
    }
}

void InputWin::slot_btnPrevPage_clicked()
{
    emit signal_candidate_page_up();
}

void InputWin::slot_btnNextPage_clicked()
{
    emit signal_candidate_page_down();
}

void InputWin::on_btnCharWidth_clicked()
{
    emit signal_btn_charWidth_clicked();
}

void InputWin::on_btnMark_clicked()
{
    emit signal_btn_mark_clicked();
}

void InputWin::on_candidate_clicked(int idx)
{
    if (idx < m_candiWordItem)
    {
        emit signal_candidate_select(idx);
        hide();
    }
}

void InputWin::slot_caret_blink()
{
    QString str = ui->labelPreEdit->text();
    if (m_caretPos >= 0)
    {
        if (m_caretPhase)
        {
            m_caretPhase = 0;
            str = str.replace("|", "");
        }
        else
        {
            m_caretPhase = 1;
            str = str.insert(m_caretPos, "<font color=\"#377EEC\"> |</font>");
        }
    }

    ui->labelPreEdit->setText(str);
}

/**************************** fcitx信号处理函数 *********************************/
// isShow: 是否显示候选词组列表
void InputWin::slot_kim_ShowLookupTable(bool enable)
{
    if (enable)
    {
        show();
    }
}

void InputWin::slot_kim_UpdateLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr,
                                          bool hasPrev, bool hasNext)
{
    Q_UNUSED(label);

    m_candiWordCount = text.length();

    int len = m_candiWordCount;
    if (len > MAX_CANDIDATE_WORD_COUNT)
    {
        FREEWB_DEBUG("slot_kim_UpdateLookupTable: len > MAX_CANDIDATE_WORD_COUNT, len={}, MAX_CANDIDATE_WORD_COUNT={}", len,
                     MAX_CANDIDATE_WORD_COUNT);
        len = MAX_CANDIDATE_WORD_COUNT;
    }

    m_candiWordItem = len;

    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        const bool visible = (i < len);
        if (visible)
        {
            QString word = text.value(i);
            const QString prompt = attr.value(i);
            const QString indexLabel = QString::number(i + 1) + m_separateChar;

            word.remove(QRegExp("\\s* +$"));
            set_candidate_text(i, indexLabel, word, prompt);
        }
        else
        {
            clear_candidate_text(i);
        }
    }

    update_candidate_visibility(len);

    if (!hasPrev && !hasNext)
    {
        m_btnPrevPage->setEnabled(false);
        m_btnNextPage->setEnabled(false);
        m_btnPrevPage->setStyleSheet("");
        m_btnNextPage->setStyleSheet("");
    }
    else
    {
        if (hasPrev)
        {
            m_btnPrevPage->setEnabled(true);
            m_btnPrevPage->setStyleSheet(QSS_PREV1_PAGE);
        }
        else
        {
            m_btnPrevPage->setEnabled(false);
            m_btnPrevPage->setStyleSheet(QSS_PREV0_PAGE);
        }

        if (hasNext)
        {
            m_btnNextPage->setEnabled(true);
            m_btnNextPage->setStyleSheet(QSS_NEXT1_PAGE);
        }
        else
        {
            m_btnNextPage->setEnabled(false);
            m_btnNextPage->setStyleSheet(QSS_NEXT0_PAGE);
        }
    }

    if (m_displayMode == CWDM_MULTI_ROW && settings::instance().get_showOpRemindInfo())
    {
        set_candiwin_op_help_info();
        if (ui->labelPrompt->isHidden())
        {
            ui->labelPrompt->show();
        }
    }

    handle_candiwin_op_help_info();
    auto_adjust_candi_win_geometry();
}

void InputWin::slot_kim_SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev,
                                       bool hasNext, int cursor)
{
    Q_UNUSED(cursor)
    slot_kim_UpdateLookupTable(label, text, attr, hasPrev, hasNext);
}

// position: 预编辑输入框中的光标位置，全角字符宽度为2,半角字符宽度为1，例如: "1.。|" 中光标位置=4
void InputWin::slot_kim_UpdatePreeditCaret(int position)
{
    QString str = m_preEidtText;
    if (!str.isEmpty() && str.at(0) != '#')
    {
        if (str.contains("<") || str.contains("<"))
        {
            QString stmStr = str.left(position);
            position += stmStr.count(QRegExp("[<>]")) * 3;
            str.replace("<", "&lt;");
            str.replace(">", "&gt;");
        }
        if (str.contains(" "))
        {
            QString stmStr = str.left(position);
            position += stmStr.count(" ") * 6;
            str.replace(" ", "&nbsp;");
        }
        str = str.insert(position, "<font color=\"#377EEC\"> |</font>");
        ui->labelPreEdit->setText(str);
        m_caretBlinkTimer.start(1000);
        m_caretPos = position;
        m_caretPhase = 1;
    }
    else
    {
        m_caretPos = -1;
    }
}

// text：预编辑输入框中的显示内容， attr:空
void InputWin::slot_kim_UpdatePreeditText(const QString &text, const QString &attr)
{
    Q_UNUSED(attr);
    m_preEidtText = text;
    QString str = text;
    if (!str.isEmpty() && str.at(0) == '#')
    {
        str = str.remove(0, 1);
    }
    ui->labelPreEdit->setText(str);
    if (!m_caretBlinkTimer.isActive())
    {
        m_caretBlinkTimer.start(1000);
        m_caretPhase = 0;
    }

    if (text.isEmpty())
    {
        if (m_isUserWordMode)
        {
            close_user_word_operation_prompt();
        }
        else
        {
            hide();
            m_caretBlinkTimer.stop();
        }
    }
    else
    {
        show();
    }

    handle_candiwin_op_help_info();
    auto_adjust_candi_win_geometry();
}

// text: 辅助显示内容  attr:空
void InputWin::slot_kim_UpdateAux(const QString &text, const QString &attr)
{
    Q_UNUSED(attr);
    m_labelImPrompt->setText(text);
    m_labelImPrompt->adjustSize();

    if (text.isEmpty())
    {
        return;
    }

    m_labelImPrompt->move(m_imPromptPosition);
    m_labelImPrompt->show();
    QTimer::singleShot(500, m_labelImPrompt, SLOT(hide()));
}

//
void InputWin::slot_kim_UpdateSpotLocation(int x, int y)
{
    m_imPromptPosition = QPoint(x, y);

    if (x + width() > m_desktopSize.width())
    {
        x = m_desktopSize.width() - width();
    }

    if (y + height() > m_desktopSize.height())
    {
        y = y - height();
    }

    move(x, y);
}

void InputWin::slot_kim_SetSpotLocation(int x, int y, int w, int h)
{
    x += w;
    m_imPromptPosition = QPoint(x, y + h);

    if (x + width() > m_desktopSize.width())
    {
        x = m_desktopSize.width() - width();
    }

    if (y + height() > m_desktopSize.height())
    {
        y = y - height();
    }
    else
    {
        y += h;
    }

    move(x, y);
}
