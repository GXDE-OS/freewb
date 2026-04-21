#include "inputwin.h"

#include "config.h"
#include "key.h"
#include "log.h"
#include "settings.h"
#include "settingshelper.h"
#include "sound.h"
#include "toolbarwin.h"
#include "ui_inputwin.h"

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

#define MIN_WIN_WIDTH 250           // 最小候选窗口宽度
#define MAX_CANDIDATE_WORD_COUNT 10 // 能够显示候选词组的最多个数

#define TABLE_ROW MAX_CANDIDATE_WORD_COUNT
#define TABLE_COLUMN (MAX_CANDIDATE_WORD_COUNT + 3) // 空白列＋上下翻页按钮

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

#define QSS_PREV0_PAGE QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.prev0PageIcoPath)
#define QSS_PREV1_PAGE QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.prev1PageIcoPath)
#define QSS_NEXT0_PAGE QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.next0PageIcoPath)
#define QSS_NEXT1_PAGE QString("background-image: url(%1);background-position:center;background-repeat:no-repeat;").arg(m_skinData.next1PageIcoPath)

#define QSS_DICT_INFO_WIN                                                                                                                                                                                                                                                                                  \
    "color: rgb(56, 56, 56);"                                                                                                                                                                                                                                                                              \
    "font: 12pt \"Ubuntu\";"                                                                                                                                                                                                                                                                               \
    "padding: 10px;"                                                                                                                                                                                                                                                                                       \
    "background-color: rgb(254, 255, 226);"                                                                                                                                                                                                                                                                \
    "border-radius: 5px;"                                                                                                                                                                                                                                                                                  \
    "border-width: 2px;"                                                                                                                                                                                                                                                                                   \
    "border-style: solid;"                                                                                                                                                                                                                                                                                 \
    "border-color: rgb(200, 200, 200);"

InputWin::InputWin(QWidget *parent) : QWidget(parent), ui(new Ui::InputWin)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowDoesNotAcceptFocus);

    setAttribute(Qt::WA_TranslucentBackground);

    m_desktopSize = QApplication::desktop()->size();
    m_defaultPosition = QPoint((m_desktopSize.width() - width() / 2), (m_desktopSize.height() - height()) / 2);
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();
    m_isUserWordMode = false;

    m_dictFindWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    m_dictFindWin.setAttribute(Qt::WA_TranslucentBackground);
    m_dictFindLabel = new QLabel(&m_dictFindWin);
    m_dictFindLabel->setWordWrap(true);
    m_dictFindLabel->setTextFormat(Qt::RichText);
    m_dictFindLabel->setStyleSheet(QSS_DICT_INFO_WIN);

    init_im_prompt_lable();
    init_ui_table();
    install_evt_filter();
    slot_load_setting_data();

    move(m_defaultPosition);

    connect(&m_caretBlinkTimer, &QTimer::timeout, this, &InputWin::slot_caret_blink);
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
    m_labelImPrompt->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    m_labelImPrompt->setAttribute(Qt::WA_TranslucentBackground, true);
    m_labelImPrompt->hide();
}

void InputWin::slot_load_setting_data()
{
    m_separateChar = freewb_separate_char_from_string(settings::instance().get_separateChar());
    m_candiCharCount = settings::instance().get_candiCharCount();
    m_cursorFollow = settings::instance().get_cursorFollow();
    m_hideCandiWin = settings::instance().get_hideCandiWin();
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

    ui->labelPreEdit->setStyleSheet(QString("border-image: url(:/image/transparent.png);color:rgb(%1,%2,%3);").arg(wordCcolor.red()).arg(wordCcolor.green()).arg(wordCcolor.blue()));
    ui->labelPrompt->setStyleSheet(QString("border-image: url(:/image/transparent.png);color:rgb(%1,%2,%3);").arg(promptColor.red()).arg(promptColor.green()).arg(promptColor.blue()));

    CandidateItem *item;
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(0, i));
        Q_ASSERT(item);
        item->set_text_font(font);
        item->set_hover_color(qRgb(255, 0, 0));

        if (i == 0)
        {
            // item->set_word_text_color( qRgb(55,126,236) );
            item->set_word_text_color(qRgb(0, 0, 255));
        }
        else
        {
            item->set_word_text_color(wordCcolor);
        }
        item->set_prompt_text_color(promptColor);
        item->set_disp_max_char_count(m_candiCharCount);

        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(i, 0));
        Q_ASSERT(item);
        item->set_text_font(font);

        item->set_hover_color(qRgb(255, 0, 0));
        if (i == 0)
        {
            item->set_word_text_color(qRgb(0, 0, 255));
        }
        else
        {
            item->set_word_text_color(wordCcolor);
        }
        item->set_prompt_text_color(promptColor);
        item->set_disp_max_char_count(m_candiCharCount);
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

void InputWin::init_ui_table()
{
    ui->tableWidget->setRowCount(TABLE_ROW);
    ui->tableWidget->setColumnCount(TABLE_COLUMN);
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        for (int j = 0; j < MAX_CANDIDATE_WORD_COUNT; j++)
        {
            if ((i == 0) || (i != 0 && j == 0))
            {
                CandidateItem *item = new CandidateItem(ui->tableWidget);
                ui->tableWidget->setCellWidget(i, j, item);
                item->set_word_text_cursor();
                connect(item, &CandidateItem::signal_cursor_hover, this, &InputWin::slot_dict_find);
            }
        }
    }

    m_btnPrevPage = new QPushButton(ui->tableWidget);
    m_btnNextPage = new QPushButton(ui->tableWidget);
    m_btnPrevPage->setFixedWidth(20);
    m_btnNextPage->setFixedWidth(20);
    m_btnPrevPage->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnNextPage->setCursor(QCursor(Qt::PointingHandCursor));
    ui->tableWidget->setCellWidget(0, MAX_CANDIDATE_WORD_COUNT + 1, m_btnPrevPage);
    ui->tableWidget->setCellWidget(0, MAX_CANDIDATE_WORD_COUNT + 2, m_btnNextPage);
    connect(m_btnPrevPage, &QPushButton::clicked, this, &InputWin::slot_btnPrevPage_clicked);
    connect(m_btnNextPage, &QPushButton::clicked, this, &InputWin::slot_btnNextPage_clicked);
}

void InputWin::slot_load_skin(const QString &skinId)
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

        QString borderColorStyle = QString("border-color:rgb(%1,%2,%3);").arg(boderColor.red()).arg(boderColor.green()).arg(boderColor.blue());

        if (settings::instance().get_useGradientColor())
        {
            QString gradienColorStyle =
                QString("background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));").arg(gradienColor0.red()).arg(gradienColor0.green()).arg(gradienColor0.blue()).arg(gradienColor1.red()).arg(gradienColor1.green()).arg(gradienColor1.blue());
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
            QString bgImageStyle = QString("%1:url(%2);").arg(settings::instance().get_enableTiled() ? "background-image" : "border-image").arg(toQStringUtf8(settings::instance().get_bgImage()));

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
    // qDebug() << obj->objectName() << event->type();

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
    int lenPreEdtLine = 0, lenTable = 0, lenPrompt = 0, len = 0;
    int tableColumn = ui->tableWidget->columnCount();

    // int fontWidth = QFontMetrics( freewb_candi_text_qfont(settings::instance()) ).width("中");
    lenPreEdtLine = ui->labelPreEdit->sizeHint().width() + ui->btnCharWidth->width() + ui->btnMark->width() + ui->btnLogo->width();
    for (int i = 0; i < tableColumn; i++)
    {
        ui->tableWidget->resizeColumnToContents(i);
        lenTable += ui->tableWidget->horizontalHeader()->sectionSize(i);
    }
    lenPrompt = ui->labelPrompt->sizeHint().width();
    len = lenPreEdtLine > lenTable ? lenPreEdtLine : lenTable;
    len = len > lenPrompt ? len : lenPrompt;
    if (len < MIN_WIN_WIDTH)
    {
        len = MIN_WIN_WIDTH;
    }
    if (lenTable < len)
    {
        ui->tableWidget->setColumnWidth(tableColumn - 3, len - lenTable + ui->tableWidget->horizontalHeader()->sectionSize(tableColumn - 3));
    }
    len += ui->horizontalLayout->contentsMargins().left() + ui->horizontalLayout->contentsMargins().right();
    len += (ui->labelLeft->width() + ui->labelRight->width());

    m_winWidth = len;

    // qDebug() << lenPreEdtLine << lenTable << lenPrompt << len;
}

void InputWin::adjust_candi_win_height()
{
    int fontHeight = QFontMetrics(freewb_candi_text_qfont(settings::instance())).height() * 1.2;

    m_winHeight = fontHeight > ui->btnCharWidth->height() ? fontHeight : ui->btnCharWidth->height();

    ui->labelPreEdit->setFixedHeight(fontHeight);
    ui->labelPrompt->setFixedHeight(fontHeight);
    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        ui->tableWidget->setRowHeight(i, fontHeight);

        CandidateItem *item = nullptr;
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(i, 0));
        Q_ASSERT(item);
        item->setFixedHeight(fontHeight);
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(0, i));
        Q_ASSERT(item);
        item->setFixedHeight(fontHeight);
    }
    QPushButton *item;
    item = qobject_cast<QPushButton *>(ui->tableWidget->cellWidget(0, MAX_CANDIDATE_WORD_COUNT + 1));
    Q_ASSERT(item);
    item->setFixedHeight(fontHeight);
    item = qobject_cast<QPushButton *>(ui->tableWidget->cellWidget(0, MAX_CANDIDATE_WORD_COUNT + 2));
    Q_ASSERT(item);
    item->setFixedHeight(fontHeight);

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
        ui->labelPrompt->setText("←和→修改词长，Enter确认，Esc放弃，Ctrl+Enter编码");
    }
    else
    {
        ui->labelPrompt->setText("按Enter确认删除词组，Esc放弃");
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

void InputWin::set_display_mode(CandiWinDispMode mode)
{
    // 单行模式
    if (mode == CWDM_ONE_ROW)
    {
        for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
        {
            ui->tableWidget->setColumnHidden(i, false);

            // 行显示处理
            if (i == 0)
            {
                ui->tableWidget->setRowHidden(i, false);
            }
            else
            {
                ui->tableWidget->setRowHidden(i, true);
            }
        }

        ui->labelPrompt->hide();
    }

    // 多行模式
    else if (mode == CWDM_MULTI_ROW)
    {
        for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
        {
            ui->tableWidget->setRowHidden(i, false);

            // 列显示处理
            if (i == 0)
            {
                ui->tableWidget->setColumnHidden(i, false);
            }
            else
            {
                ui->tableWidget->setColumnHidden(i, true);
            }
        }

        if (m_showOpRemindInfo)
        {
            // ui->labelPrompt->setText( "【极点五笔银河麒麟版】" );
            ui->labelPrompt->show();
        }
        else
        {
            ui->labelPrompt->clear();
            ui->labelPrompt->hide();
        }
    }
}

void InputWin::enter_user_word_mode()
{
    m_isUserWordMode = true;

    ui->btnCharWidth->hide();
    ui->btnMark->hide();
    ui->tableWidget->hide();

    ui->btnLogo->setStyleSheet(QSS_FREEIME_LOGO);
    ui->btnLogo->show();
    ui->labelPrompt->show();
}

void InputWin::exit_user_word_mode()
{
    m_isUserWordMode = false;

    ui->btnCharWidth->show();
    ui->btnMark->show();
    ui->tableWidget->show();

    ui->btnLogo->hide();

    set_display_mode(m_displayMode);
}

// 设置候选框单元格内容
void InputWin::set_candidate_text(int idx, const QString &label, const QString &wordText, const QString &promptText)
{
    Q_ASSERT(idx < MAX_CANDIDATE_WORD_COUNT);

    qDebug() << "label : " << label << ",wordText : " << wordText << ",promptText : " << promptText;
    CandidateItem *item = nullptr;

    if (m_displayMode == CWDM_ONE_ROW)
    {
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(0, idx));
    }
    else if (m_displayMode == CWDM_MULTI_ROW)
    {
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(idx, 0));
    }

    Q_ASSERT(item);

    QString str = label;
    str.replace('.', m_separateChar);

    // item->set_text( " " + str + " ", wordText, promptText );

    // printf("[%s]\n[%s]",word.toUtf8().constData(),str.toUtf8().constData());

    item->set_text(str, wordText, promptText);
    // set candwin width
    int width = str.length() + wordText.length(); //+2;
    if (promptText.length() == 0)
        width += 1;
    else
        width += promptText.length();

    QFont font = freewb_candi_text_qfont(settings::instance());

    QSize sz(width * font.pointSize(), 20);
    item->setMinimumSize(sz);
}

void InputWin::clear_candidate_text(int idx)
{
    Q_ASSERT(idx < MAX_CANDIDATE_WORD_COUNT);

    CandidateItem *item = nullptr;

    if (m_displayMode == CWDM_ONE_ROW)
    {
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(0, idx));
    }
    else if (m_displayMode == CWDM_MULTI_ROW)
    {
        item = qobject_cast<CandidateItem *>(ui->tableWidget->cellWidget(idx, 0));
    }

    Q_ASSERT(item);

    item->clear_text();
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
#ifdef DEBUG
    qDebug() << "快捷键捕捉……………";
#endif
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
        tips = "【极点五笔银河麒麟版】";
    }
    else if (oti == OTI_MOUSE_MENU)
    {
        tips = "【设置：鼠标右键点击窗口】";
    }
    else if (oti == OTI_DELETE_WORD)
    {
        tips = "【Del+序号 词组删除】";
    }
    else if (oti == OTI_ADJUST_WORD)
    {
        tips = "【Ctrl+序号 词组调序】";
    }
    else if (oti == OTI_SK_SWITCH_CHAR_WIDTH)
    {
        tips = "【Shift+Space 切换全/半角】";
    }
    else if (oti == OTI_SK_SWITCH_MARK)
    {
        tips = "【Ctrl+. 切换中/英文标点】";
    }
    else if (oti == OTI_SK_BACK_FIND_CODE)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_backFindCode());
        if (!keyText.isEmpty())
            tips = QString("【%1 反查编码】").arg(keyText);
    }
    else if (oti == OTI_SK_ONLINE_ADD_WORD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_onlineAddWord());
        if (!keyText.isEmpty())
            tips = QString("【%1 在线加词】").arg(keyText);
    }
    else if (oti == OTI_SK_ONLINE_DEL_WORD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_onlineDelWord());
        if (!keyText.isEmpty())
            tips = QString("【%1 在线删词】").arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_KEYBOARD)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchVKb());
        if (!keyText.isEmpty())
            tips = QString("【%1 切换软键盘】").arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_CHAR_SET)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchCharSet());
        if (!keyText.isEmpty())
            tips = QString("【%1 切换字符集】").arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_INPUT_MODE)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchInputMode());
        if (!keyText.isEmpty())
            tips = QString("【%1 切换输入模式】").arg(keyText);
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
            tips = QString("【%1 切换简入繁出】").arg(keyText);
    }
    else if (oti == OTI_SK_SHOW_HIDE_STATUS_BAR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_showHideToolbar());
        if (!keyText.isEmpty())
            tips = QString("【%1 显/隐状态栏】").arg(keyText);
    }
    else if (oti == OTI_SK_SHOW_HIDE_CANDIDATE_WIN)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_showHideCandiWin());
        if (!keyText.isEmpty())
            tips = QString("【%1 显/隐候选窗】").arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_WORD_LEXICON)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_switchLexicon());
        if (!keyText.isEmpty())
            tips = QString("【%1 切换词库】").arg(keyText);
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
            tips = QString("【%1 切换皮肤】").arg(keyText);
    }
    else if (oti == OTI_SK_QUICK_DEL_SCREEN_CHAR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_quickDelScreenItem());
        if (!keyText.isEmpty())
            tips = QString("【%1 快删上屏项】").arg(keyText);
    }
    else if (oti == OTI_SK_MARK_AUTO_PAIR)
    {
        const QString keyText = customShortcutDisplayText(settings::instance().get_markAutoPair());
        if (!keyText.isEmpty())
            tips = QString("【%1 标点自动配对】").arg(keyText);
    }
    else if (oti == OTI_SK_TEMP_ENGLISH)
    {
        const QString keyText = keyTokenName(settings::instance().get_tempEnglish());
        if (!keyText.isEmpty())
            tips = QString("【%1 临时英文输入】").arg(keyText);
    }
    else if (oti == OTI_SK_QUICK_INPUT)
    {
        const QString keyText = keyTokenName(settings::instance().get_shortcutInput());
        if (!keyText.isEmpty())
            tips = QString("【%1 快捷短语输入】").arg(keyText);
    }
    else if (oti == OTI_SK_TEMP_PINYIN)
    {
        const QString keyText = keyTokenName(settings::instance().get_tempPinyin());
        if (!keyText.isEmpty())
            tips = QString("【%1 临时拼音/生癖字输入】").arg(keyText);
    }
    else if (oti == OTI_SK_SWITCH_CN_EN)
    {
        const QString keyText = cnEnSwitchDisplayText(settings::instance().get_cnEnSwitch());
        if (!keyText.isEmpty())
            tips = QString("【%1 切换中/英文】").arg(keyText);
    }
    else if (oti == OTI_SK_RECODE_SELECT)
    {
        const QString keyText = pairKeyDisplayText(settings::instance().get_secondRecodeKey(), settings::instance().get_thirdRecodeKey());
        if (!keyText.isEmpty())
            tips = QString("【%1 选择二三重码】").arg(keyText);
    }
    else if (oti == OTI_SK_CANDI_PAGE)
    {
        const QString keyText = pairKeyDisplayText(settings::instance().get_prevPageKey(), settings::instance().get_nextPageKey());
        if (!keyText.isEmpty())
            tips = QString("【%1 候选词上下翻页】").arg(keyText);
    }

    else if (oti == OTI_TEMP_ENGLISH)
    {
        tips = "[ 临时英文 ]";
    }
    else if (oti == OTI_TEMP_ENGLISH_EMPTY)
    {
        tips = "[ 按回车输出英文，Esc放弃。 ]";
    }
    else if (oti == OTI_QUICK_INPUT)
    {
        tips = "[ 快捷输入 ]";
    }
    else if (oti == OTI_TEMP_PINYIN)
    {
        tips = "[ 临时拼音/生癖字 ]";
    }
    else if (oti == OTI_TEMP_ENGLISH_SELECT)
    {
        const QString name2 = keyTokenName(settings::instance().get_secondRecodeKey());
        const QString name3 = keyTokenName(settings::instance().get_thirdRecodeKey());
        if (!name2.isEmpty() && !name3.isEmpty())
        {
            tips = QString("[ Space选1, %1选2, %2选3 ]").arg(name2, name3);
        }
    }

    if (tips.isEmpty())
    {
        tips = "【极点五笔银河麒麟版】";
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
    if (ToolbarWin::get_mark_mode() == MARK_CN)
    {
        ui->btnMark->setStyleSheet(QSS_MARK_CN);
    }
    else
    {
        ui->btnMark->setStyleSheet(QSS_MARK_EN);
    }
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
                    findInfo += QString("<p><font color='#ef2929'>%1</font></p>  编码：%2\n").arg(wchar).arg(wb.replace(",", "  "));
                }
            }
            else if (wbpy.length() == 2)
            {
                QString wb = wbpy.at(0);
                QString py = wbpy.at(1);
                findInfo += QString("<p><font color='#ef2929'>%1</font></p>  编码：%2\n  拼音：%3\n").arg(wchar).arg(wb.replace(",", "  ")).arg(py.replace(",", "  "));
            }
        }
    }
    else
    {
        QString str = m_dictquery.rstrip(wordText);
        findInfo.insert(0, QString("<p><font color='#ef2929'>【 %1 】</font></p>\n").arg(str));
        // qDebug() << findInfo;
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
    //    qDebug() << "";
    emit signal_candidate_page_up();
}

void InputWin::slot_btnNextPage_clicked()
{
    //    qDebug() << "";
    emit signal_candidate_page_down();
}

void InputWin::on_btnCharWidth_clicked()
{
    //    qDebug() << "";
    emit signal_btn_charWidth_clicked();
}

void InputWin::on_btnMark_clicked()
{
    //    qDebug() << "";
    emit signal_btn_mark_clicked();
}

void InputWin::on_tableWidget_cellClicked(int row, int column)
{
    if (m_displayMode == CWDM_ONE_ROW && column < m_candiWordItem && row == 0)
    {
        emit signal_candidate_select(column);
    }
    else if (m_displayMode == CWDM_MULTI_ROW && row < m_candiWordItem && column == 0)
    {
        emit signal_candidate_select(row);
    }
}

void InputWin::slot_caret_blink()
{
    // qDebug() << m_caretPos << m_caretPhase;
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
// isShow: 切换到中文输入后是否在光标处出现短暂的提示信息， 辅助窗口显示
void InputWin::slot_kim_ShowAux(bool enable)
{
    //    printf("kim_showAux\n");
    //    qDebug() << enable;
    if (enable)
    {
        m_labelImPrompt->move(m_imPromptPosition);
    }

    if (enable)
    {
        m_labelImPrompt->show();
        QTimer::singleShot(500, m_labelImPrompt, SLOT(hide()));
    }

    //    else
    //    {
    //        m_labelImPrompt->hide();
    //    }
}

// isShow: 是否显示候选词组列表
void InputWin::slot_kim_ShowLookupTable(bool enable)
{
    if (enable)
    {
        if (isHidden() && !m_hideCandiWin)
        {
            show();
        }
    }
}

void InputWin::slot_kim_UpdateLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev, bool hasNext)
{
    Q_UNUSED(label);
    Q_UNUSED(attr);

    m_candiWordCount = text.length();

    int len = m_candiWordCount;
    if (len > MAX_CANDIDATE_WORD_COUNT)
    {
        FREEWB_DEBUG("slot_kim_UpdateLookupTable: len > MAX_CANDIDATE_WORD_COUNT, len={}, MAX_CANDIDATE_WORD_COUNT={}", len, MAX_CANDIDATE_WORD_COUNT);
        len = MAX_CANDIDATE_WORD_COUNT;
    }

    m_candiWordItem = len;

    for (int i = 0; i < MAX_CANDIDATE_WORD_COUNT; i++)
    {
        const bool visible = (i < len);
        if (visible)
        {
            const QString candidate = text.value(i);
            const int splitPos = candidate.indexOf(':');
            QString word = (splitPos >= 0) ? candidate.left(splitPos) : candidate;
            const QString prompt = (splitPos >= 0) ? candidate.mid(splitPos + 1) : QString();
            const QString indexLabel = QString::number(i + 1) + m_separateChar;

            word.remove(QRegExp("\\s* +$"));
            set_candidate_text(i, indexLabel, word, prompt);
        }
        else
        {
            clear_candidate_text(i);
        }

        if (m_displayMode == CWDM_ONE_ROW)
        {
            ui->tableWidget->setColumnHidden(i, !visible);
        }
        else
        {
            ui->tableWidget->setRowHidden(i, !visible);
        }
    }

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

void InputWin::slot_kim_SetLookupTable(const QStringList &label, const QStringList &text, const QStringList &attr, bool hasPrev, bool hasNext, int cursor)
{
    Q_UNUSED(cursor)
    slot_kim_UpdateLookupTable(label, text, attr, hasPrev, hasNext);
}

// position: 预编辑输入框中的光标位置，全角字符宽度为2,半角字符宽度为1，例如: "1.。|" 中光标位置=4
void InputWin::slot_kim_UpdatePreeditCaret(int position)
{
    // qDebug() << position;
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

// enable 是否显示预编辑输入框，每次按键输入或输入框失去焦点都会触发
void InputWin::slot_kim_ShowPreedit(bool enable)
{
    if (enable)
    {
        if (isHidden() && !m_hideCandiWin)
        {
            show();
        }
    }
    else
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
}

// text：预编辑输入框中的显示内容， attr:空
void InputWin::slot_kim_UpdatePreeditText(const QString &text, const QString &attr)
{
    Q_UNUSED(attr);
    // qDebug() << text << attr;
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

    handle_candiwin_op_help_info();
    auto_adjust_candi_win_geometry();
}

// text: 辅助显示内容  attr:空
void InputWin::slot_kim_UpdateAux(const QString &text, const QString &attr)
{
    Q_UNUSED(attr);
    // qDebug() << text << attr;
    m_labelImPrompt->setText(text);
    m_labelImPrompt->adjustSize();
}

//
void InputWin::slot_kim_UpdateSpotLocation(int x, int y)
{
    m_imPromptPosition = QPoint(x, y);

    if (!m_cursorFollow)
    {
        // move( m_defaultPosition );
    }
    else
    {
        if (x + width() > m_desktopSize.width())
        {
            x = m_desktopSize.width() - width();
        }

        if (y + height() > m_desktopSize.height())
        {
            y = y - height() - 30;
        }
        else
        {
            y += 10;
        }

        move(x, y);
    }
}

void InputWin::slot_kim_SetSpotLocation(int x, int y, int w, int h)
{
    Q_UNUSED(w)
    // printf("8888setspot\n");
    m_imPromptPosition = QPoint(x, y + h);

    if (!m_cursorFollow)
    {
        // move( m_defaultPosition );
    }
    else
    {
        if (x + width() > m_desktopSize.width())
        {
            x = m_desktopSize.width() - width();
        }

        if (y + height() > m_desktopSize.height())
        {
            y = y - height() - 10;
        }
        else
        {
            y += (h + 10);
        }

        move(x, y);
    }
}

