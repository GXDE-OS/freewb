#include "settingwin.h"

#include <vector>

#include <QDateTime>
#include <QFont>

#include "commdefine.h"
#include "settings.h"
#include "settingshelper.h"
#include "ui_settingwin.h"

namespace
{
QColor swQColorFromSpec(const std::string &s)
{
    return QColor(toQStringUtf8(s));
}

void swBuildSingleShortcutCombo(QComboBox *combo, int selectedIdx, int forbiddenA, int forbiddenB)
{
    combo->clear();
    for (int i = 0; i < SSK_NUM; ++i)
    {
        if (i != SSK_NONE && (i == forbiddenA || i == forbiddenB))
            continue;
        combo->addItem(toQStringUtf8(freewb_single_shortcut_display_name(i)), i);
    }

    int target = selectedIdx;
    if (target != SSK_NONE && (target == forbiddenA || target == forbiddenB))
        target = SSK_NONE;

    const int pos = combo->findData(target);
    combo->setCurrentIndex(pos >= 0 ? pos : 0);
}

} // namespace

// 设置窗口样式表
#define QSS_FILE ":/qss/settingwin.qss"

#define QSS_TOOL_TIPS                                                                                                                                                                                                                                                                                      \
    "color: rgb(56, 56, 56);"                                                                                                                                                                                                                                                                              \
    "font: 12pt \"Ubuntu\";"                                                                                                                                                                                                                                                                               \
    "padding: 10px;"                                                                                                                                                                                                                                                                                       \
    "background-color: rgb(254, 255, 226);"                                                                                                                                                                                                                                                                \
    "border-radius: 5px;"                                                                                                                                                                                                                                                                                  \
    "border-width: 2px;"                                                                                                                                                                                                                                                                                   \
    "border-style: solid;"                                                                                                                                                                                                                                                                                 \
    "border-color: rgb(200, 200, 200);"

#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"

#define QSS_BTN_CLOSE0 "border-image: url(:/image/setting/close0.png);"
#define QSS_BTN_CLOSE1 "border-image: url(:/image/setting/close1.png);"
#define QSS_BTN_CLOSE2 "border-image: url(:/image/setting/close2.png);"

// 设置组子分组图标
#define ICO_SETTING_GROUP ":/image/setting/group.png"

SettingWin::SettingWin(QWidget *parent) : QWidget(parent), ui(new Ui::SettingWin)
{
    ui->setupUi(this);

    init_member_data(); // 确保相关的数据成员初始化完成后再初始化界面外观
    init_mouse_hover_tips();
    init_window_appearance();
    install_evt_filter();

    move(m_defaultPopPosition);

    /////////////////////////
    // ui->labelBugFeedback->hide();
    ui->ckbInputStatistic->hide();
    ui->label_37->hide();
    ui->label_38->hide();
    ui->label_40->hide();
    ui->label_41->hide();
    ui->ledt2ndRecode->hide();
    ui->ledt3rdRecode->hide();
    ui->ledtPrecPage->hide();
    ui->ledtNextPage->hide();
    ui->ckbShiftSelectRecode->hide();
    ui->ckbAutoLocate->hide();
    ui->cmbWhenLossLocation->hide();
    ui->labelLossLocate->hide();
}

SettingWin::~SettingWin()
{
    delete ui;

    if (m_kbCustomKeyChar)
    {
        delete m_kbCustomKeyChar;
        m_kbCustomKeyChar = nullptr;
    }

    if (m_kbCustomKeyMark)
    {
        delete m_kbCustomKeyMark;
        m_kbCustomKeyMark = nullptr;
    }

    if (m_customKeyDialog)
    {
        delete m_customKeyDialog;
        m_customKeyDialog = nullptr;
    }
}

void SettingWin::init_window_appearance()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);

    setWindowIcon(QIcon(":/image/setting/logo.png"));
    setWindowTitle("属性设置");
    // setFont(freewb_candi_text_qfont(settings::instance()));

    ui->labelVersionNum->setText(FREEWB_VERSION);
    ui->labelVersion->setText("极点五笔输入法");

    // 载入窗口全局UI样式表
    QFile qssFile(QSS_FILE);
    if (!qssFile.open(QFile::ReadOnly))
    {
        qWarning() << "open qss file failed!";
    }
    else
    {
        this->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }
}

void SettingWin::init_member_data()
{
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    // 初始化设置界面的软键盘
    m_kbCustomKeyChar = new Keyboard(VKM_CUSTOM_CHAR, ui->pageCustomKeyChar);
    m_kbCustomKeyChar->move(40, 120);

    m_kbCustomKeyMark = new Keyboard(VKM_CUSTOM_MARK, ui->pageCustomKeyMark);
    m_kbCustomKeyMark->move(40, 120);

    // 自定义软键盘点击
    connect(m_kbCustomKeyChar, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)), this, SLOT(slot_custom_keyboard_char_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)));
    connect(m_kbCustomKeyMark, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)), this, SLOT(slot_custom_keyboard_mark_clicked(SymbolKeyIdx, const QString &, const CustomKeyValue &)));

    m_customKeyDialog = new CustomKeyDialog();
    connect(m_customKeyDialog, SIGNAL(signal_custom_ok_btn_clicked(const QString &, const QString &)), this, SLOT(slot_custom_btn_ok_clicked(const QString &, const QString &)));
}

void SettingWin::init_mouse_hover_tips()
{
#define TIPS_FILE INSTALL_DIR + "/data/setting_tips.txt"

    m_tooltipsWin.setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    m_tooltipsWin.setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipsLabel = new QLabel(&m_tooltipsWin);
    m_tooltipsLabel->setStyleSheet(QSS_TOOL_TIPS);

    QFile tipsFile(TIPS_FILE);
    QTextStream textStream(&tipsFile);
    if (!tipsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << TIPS_FILE << "open failed!";
        return;
    }

    QMap<int, QString> tipsMap;
    QStringList textList = textStream.readAll().split('\n');
    tipsFile.close();
    foreach(QString line, textList)
    {
        if (line.startsWith("1"))
        {
            // QStringList tips = line.split( '=' );
            QStringList tips;
            tips << line.left(4);
            tips << line.mid(5);
            if (tips.length() > 1)
            {
                QString value = tips.at(1);
                tipsMap.insert(tips.at(0).toInt(), value.replace("\\n", "\n"));
            }
        }
    }

    m_tipsTextMap.insert(ui->ckbCodeRemind, tipsMap.value(1001));
    m_tipsTextMap.insert(ui->ckbSpaceFullWhenCharHalf, tipsMap.value(1002));
    m_tipsTextMap.insert(ui->ckbWordThink, tipsMap.value(1003));
    m_tipsTextMap.insert(ui->ckbSmartMark, tipsMap.value(1004));
    m_tipsTextMap.insert(ui->ckbRemindExistWord, tipsMap.value(1005));
    m_tipsTextMap.insert(ui->ckbAlertWhenEmptyCode, tipsMap.value(1006));
    m_tipsTextMap.insert(ui->ckbUseAudioFile, tipsMap.value(1007));
    m_tipsTextMap.insert(ui->ckbAutoAdjustFreq, tipsMap.value(1008));

    m_tipsTextMap.insert(ui->ckbShiftCommitChar, tipsMap.value(1101));
    m_tipsTextMap.insert(ui->ckbInputStatistic, tipsMap.value(1102));
    m_tipsTextMap.insert(ui->ckbTypeEffect, tipsMap.value(1103));
    m_tipsTextMap.insert(ui->ckbRepeatCalib, tipsMap.value(1104));
    m_tipsTextMap.insert(ui->ckbAutoWordGroup, tipsMap.value(1105));

    m_tipsTextMap.insert(ui->ledtAutoToEnStr, tipsMap.value(1201));
    m_tipsTextMap.insert(ui->ledtAutoToHalf, tipsMap.value(1202));

    m_tipsTextMap.insert(ui->cmbSkinSelect, tipsMap.value(1301));
    m_tipsTextMap.insert(ui->ckbAutoLocate, tipsMap.value(1302));
    m_tipsTextMap.insert(ui->cmbWhenLossLocation, tipsMap.value(1303));
    m_tipsTextMap.insert(ui->ckbAutoExtend, tipsMap.value(1304));
    m_tipsTextMap.insert(ui->ckbEnableUiAudioEffect, tipsMap.value(1305));
    m_tipsTextMap.insert(ui->ckbDispRealHelp, tipsMap.value(1306));
    m_tipsTextMap.insert(ui->ckbHideToolbar, tipsMap.value(1307));
    m_tipsTextMap.insert(ui->spbToolbarTransparency, tipsMap.value(1308));

    m_tipsTextMap.insert(ui->cmbCandiWinMode, tipsMap.value(1401));
    m_tipsTextMap.insert(ui->ledtSeparateChar, tipsMap.value(1402));
    m_tipsTextMap.insert(ui->ckbUseGradientBgColor, tipsMap.value(1403));
    m_tipsTextMap.insert(ui->ckbUseBgImage, tipsMap.value(1404));
    m_tipsTextMap.insert(ui->ckbUseTile, tipsMap.value(1405));
    m_tipsTextMap.insert(ui->spbCornerRadian, tipsMap.value(1406));
    m_tipsTextMap.insert(ui->spbCandiTransparency, tipsMap.value(1407));
    m_tipsTextMap.insert(ui->spbCandiItemNum, tipsMap.value(1408));
    m_tipsTextMap.insert(ui->spbCandiCharNum, tipsMap.value(1409));

    m_tipsTextMap.insert(ui->btnCandiFont, tipsMap.value(1410));
    m_tipsTextMap.insert(ui->btnCandiBg, tipsMap.value(1411));
    m_tipsTextMap.insert(ui->btnCandiBgColor0, tipsMap.value(1412));
    m_tipsTextMap.insert(ui->btnCandiBgColor1, tipsMap.value(1413));
    m_tipsTextMap.insert(ui->btnCandiBorderColor, tipsMap.value(1414));
    m_tipsTextMap.insert(ui->btnCandiAutoWord, tipsMap.value(1415));
    m_tipsTextMap.insert(ui->btnCandiPrompt, tipsMap.value(1416));

    m_tipsTextMap.insert(ui->ledt2ndRecode, tipsMap.value(1501));
    m_tipsTextMap.insert(ui->ledt3rdRecode, tipsMap.value(1502));
    m_tipsTextMap.insert(ui->ckbCursorFollow, tipsMap.value(1503));
    m_tipsTextMap.insert(ui->ckbHideCandiChinese, tipsMap.value(1504));
    m_tipsTextMap.insert(ui->ckbDispOpPrompt, tipsMap.value(1505));
    m_tipsTextMap.insert(ui->ckbShiftSelectRecode, tipsMap.value(1506));
    m_tipsTextMap.insert(ui->ledtPrecPage, tipsMap.value(1507));
    m_tipsTextMap.insert(ui->ledtNextPage, tipsMap.value(1508));
    m_tipsTextMap.insert(ui->cmb23RecodeSelect, tipsMap.value(1509));
    m_tipsTextMap.insert(ui->cmbPrevNextPage, tipsMap.value(1510));
    m_tipsTextMap.insert(ui->ckbDispOpDict, tipsMap.value(1511));

    m_tipsTextMap.insert(ui->cmbFunction, tipsMap.value(1601));
    m_tipsTextMap.insert(ui->cmbShortcutKey, tipsMap.value(1602));
    m_tipsTextMap.insert(ui->ckbDisableAllShortcutKey, tipsMap.value(1603));
    m_tipsTextMap.insert(ui->ckbDisableFullHalfKey, tipsMap.value(1604));
    m_tipsTextMap.insert(ui->cmbTmpEnglish, tipsMap.value(1605));
    m_tipsTextMap.insert(ui->cmbShortcutInput, tipsMap.value(1606));
    m_tipsTextMap.insert(ui->cmbTmpPinyin, tipsMap.value(1607));
    m_tipsTextMap.insert(ui->cmbSwitchCnEn, tipsMap.value(1608));
}

void SettingWin::install_evt_filter()
{
    installEventFilter(this);

#if 0
//    QList<QWidget *> widgets = findChildren<QWidget *>();
//    foreach( QWidget *widget, widgets )
//    {
//        widget->installEventFilter( this );
//    }
#else
    ui->ckbCodeRemind->installEventFilter(this);
    ui->ckbSpaceFullWhenCharHalf->installEventFilter(this);
    ui->ckbWordThink->installEventFilter(this);
    ui->ckbSmartMark->installEventFilter(this);
    ui->ckbRemindExistWord->installEventFilter(this);
    ui->ckbAlertWhenEmptyCode->installEventFilter(this);
    ui->ckbUseAudioFile->installEventFilter(this);
    ui->ckbAutoAdjustFreq->installEventFilter(this);

    ui->ckbShiftCommitChar->installEventFilter(this);
    ui->ckbInputStatistic->installEventFilter(this);
    ui->ckbTypeEffect->installEventFilter(this);
    ui->ckbRepeatCalib->installEventFilter(this);
    ui->ckbAutoWordGroup->installEventFilter(this);

    ui->ledtAutoToEnStr->installEventFilter(this);
    ui->ledtAutoToHalf->installEventFilter(this);

    ui->cmbSkinSelect->installEventFilter(this);
    ui->ckbAutoLocate->installEventFilter(this);
    ui->cmbWhenLossLocation->installEventFilter(this);
    ui->ckbAutoExtend->installEventFilter(this);
    ui->ckbEnableUiAudioEffect->installEventFilter(this);
    ui->ckbDispRealHelp->installEventFilter(this);
    ui->ckbHideToolbar->installEventFilter(this);
    ui->spbToolbarTransparency->installEventFilter(this);

    ui->cmbCandiWinMode->installEventFilter(this);
    ui->ledtSeparateChar->installEventFilter(this);
    ui->ckbUseGradientBgColor->installEventFilter(this);
    ui->ckbUseBgImage->installEventFilter(this);
    ui->ckbUseTile->installEventFilter(this);
    ui->spbCornerRadian->installEventFilter(this);
    ui->spbCandiTransparency->installEventFilter(this);
    ui->spbCandiItemNum->installEventFilter(this);
    ui->spbCandiCharNum->installEventFilter(this);
    ui->cmb23RecodeSelect->installEventFilter(this);
    ui->cmbPrevNextPage->installEventFilter(this);

    ui->btnCandiFont->installEventFilter(this);
    ui->btnCandiBg->installEventFilter(this);
    ui->btnCandiBgColor0->installEventFilter(this);
    ui->btnCandiBgColor1->installEventFilter(this);
    ui->btnCandiBorderColor->installEventFilter(this);
    ui->btnCandiAutoWord->installEventFilter(this);
    ui->btnCandiPrompt->installEventFilter(this);

    ui->ledt2ndRecode->installEventFilter(this);
    ui->ledt3rdRecode->installEventFilter(this);
    ui->ckbCursorFollow->installEventFilter(this);
    ui->ckbHideCandiChinese->installEventFilter(this);
    ui->ckbDispOpPrompt->installEventFilter(this);
    ui->ckbDispOpDict->installEventFilter(this);

    ui->ckbShiftSelectRecode->installEventFilter(this);
    ui->ledtPrecPage->installEventFilter(this);
    ui->ledtNextPage->installEventFilter(this);

    ui->cmbFunction->installEventFilter(this);
    ui->cmbShortcutKey->installEventFilter(this);
    ui->ckbDisableAllShortcutKey->installEventFilter(this);
    ui->ckbDisableFullHalfKey->installEventFilter(this);
    ui->cmbTmpEnglish->installEventFilter(this);
    ui->cmbShortcutInput->installEventFilter(this);
    ui->cmbTmpPinyin->installEventFilter(this);
    ui->cmbSwitchCnEn->installEventFilter(this);
#endif
}

void SettingWin::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void SettingWin::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void SettingWin::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

bool SettingWin::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvt = static_cast<QKeyEvent *>(event);
        if (keyEvt->key() == Qt::Key_Escape)
            close();
    }
    else if (event->type() == QEvent::ToolTip)
    {
        // qDebug() << obj->objectName();
        if (m_tipsTextMap.contains(qobject_cast<QWidget *>(obj)))
        {
            QString tips = m_tipsTextMap.value(qobject_cast<QWidget *>(obj));
            if (!tips.isEmpty())
            {
                m_tooltipsLabel->setText(tips);
                m_tooltipsLabel->adjustSize();
                m_tooltipsWin.adjustSize();
                m_tooltipsWin.move(QCursor::pos().x() + 10, QCursor::pos().y() + 10);
                m_tooltipsWin.show();
                m_tooltipsWinShowFlg = true;
            }
        }
    }
    else if (event->type() == QEvent::Leave && m_tooltipsWinShowFlg)
    {
        m_tooltipsWin.hide();
        m_tooltipsWinShowFlg = false;
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

void SettingWin::slot_open_win()
{
    slot_init_all_setting_page();

    show();

    activateWindow();

    ui->listWidget->setCurrentRow(0);
    ui->stackedWidget->setCurrentIndex(0);
}

void SettingWin::slot_init_all_setting_page()
{
    update_listwidget_item();

    init_common_page();
    init_advance_page();
    init_others_page();
    init_shortcutkey_page();
    init_ui_setting_page();
    init_candidate_ui_page();
    init_candidate_option_page();
}

void SettingWin::slot_show_version_info()
{
    slot_init_all_setting_page();
    show();
    activateWindow();

    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        if (ui->listWidget->item(i)->text() == "版本信息")
        {
            ui->listWidget->setCurrentRow(i);
        }
    }
    ui->stackedWidget->setCurrentWidget(ui->pageVersionInfo);
}

void SettingWin::slot_open_advanced_settting_page()
{
    slot_init_all_setting_page();
    show();
    activateWindow();

    ui->listWidget->setCurrentRow(1);
    ui->stackedWidget->setCurrentWidget(ui->pageAdvance);
}

// 更新设置界面左侧的设置选项组
void SettingWin::update_listwidget_item()
{
    ui->listWidget->clear();

    m_listItemCommon = new QListWidgetItem("常用选项", ui->listWidget);
    m_listItemAdvance = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "高级选项", ui->listWidget);
    m_listItemOthers = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "其他设置", ui->listWidget);

    if (freewb_runtime_show_all_group())
    {
        m_listItemUi = new QListWidgetItem("界面设置", ui->listWidget);
        m_listItemCandidateWinUi = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "候选窗界面", ui->listWidget);
        m_listItemCandidateWinOption = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "候选窗选项", ui->listWidget);
    }

    m_listItemShortcutKey = new QListWidgetItem("设置快捷键", ui->listWidget);
    m_listItemCustomKeyChar = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "定义软键盘", ui->listWidget);
    m_listItemCustomKeyMark = new QListWidgetItem(QIcon(ICO_SETTING_GROUP), "自定义标点", ui->listWidget);
    m_listItemVersionInfo = new QListWidgetItem("版本信息", ui->listWidget);
    if (g_cpuType == CT_X86)
    {
        // m_listItemBug = new QListWidgetItem( "问题反馈", ui->listWidget );
    }

    ui->stackedWidget->setCurrentWidget(ui->pageCommon);
    ui->listWidget->setCurrentRow(0);
}

// 初始化常用选项设置页面
void SettingWin::init_common_page()
{
    ui->ckbCodeRemind->setChecked(settings::instance().get_codeRemind());
    ui->ckbSpaceFullWhenCharHalf->setChecked(settings::instance().get_spaceFullWhenCharHalf());
    ui->ckbWordThink->setChecked(settings::instance().get_wordThink());
    ui->ckbSmartMark->setChecked(settings::instance().get_smartMark());
    ui->ckbRemindExistWord->setChecked(settings::instance().get_remindExistWord());
    ui->ckbAlertWhenEmptyCode->setChecked(settings::instance().get_alertWhenEmptyCode());
    ui->ckbUseAudioFile->setChecked(freewb_runtime_use_audio_file());
    ui->labelUseAudioFile->hide();
    ui->ckbUseAudioFile->hide(); ///////////////
    ui->ckbAutoAdjustFreq->setChecked(settings::instance().get_autoAdjustFreq());
}

// 初始化高级设置页面
void SettingWin::init_advance_page()
{
    ui->ckbShiftCommitChar->setChecked(settings::instance().get_shiftCommitChar());
    ui->ckbInputStatistic->setChecked(settings::instance().get_inputStatistic());
    ui->ckbTypeEffect->setChecked(settings::instance().get_typeEffect());
    ui->ckbRepeatCalib->setChecked(settings::instance().get_recodeCalib());

    AutoWordGroupOpt opt = static_cast<AutoWordGroupOpt>(settings::instance().get_autoWordGroupOpt());
    if (opt == AWGO_FORBID)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(0);
    }
    else if (opt == AWGO_LOSS)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(1);
    }
    else if (opt == AWGO_SAVE)
    {
        ui->ckbAutoWordGroup->setCurrentIndex(2);
    }
}

// 初始化其它选项设置页面
void SettingWin::init_others_page()
{
    ui->ledtAutoToEnStr->setText(toQStringUtf8(settings::instance().get_autoToEnStr()));
    // ui->ledtAutoToHalf->setText( settings::instance().get_CoustomMark() );
    ui->ledtAutoToHalf->hide();
    ui->ckbAutoHalfMarkAfterNum->setChecked(settings::instance().get_autoToHalfMarkFlg());
}

// 初始化快捷键设置页
void SettingWin::init_shortcutkey_page()
{
    update_custom_shortkey_cmb();
    update_tmp_engish_cmb();
    update_short_input_cmb();
    update_tmp_pinyin_cmb();

    ui->ckbDisableFullHalfKey->setChecked(settings::instance().get_disableFullHalfSwitch());
    ui->ckbDisableAllShortcutKey->setChecked(settings::instance().get_disableAllShortcutKey());
    // 该项依赖 fcitx 全局配置，不再由本页提供设置
    ui->labelCnEn->hide();
    ui->cmbSwitchCnEn->hide();
}

// 设置界面--更新自定义功能快捷键选择框
void SettingWin::update_custom_shortkey_cmb()
{
    ui->cmbShortcutKey->clear();
    //@note update_custom_shortkey_cmb
    for (int i = 0; i < CSK_NUM; i++)
    {
        bool isUsed = false;
        for (int j = 0; j < CSF_NUM; j++)
        {
            if (i != CSK_NONE && (j != ui->cmbFunction->currentIndex()) && (freewb_custom_shortcut_get_combine_index(settings::instance(), j) == i))
            {
                isUsed = true;
            }
        }

        if (!isUsed)
        {
            ui->cmbShortcutKey->addItem(toQStringUtf8(freewb_combine_shortcut_display_name(i)));
        }
    }

    for (int i = 0; i < ui->cmbShortcutKey->count(); i++)
    {
        if (ui->cmbShortcutKey->itemText(i) == toQStringUtf8(freewb_custom_shortcut_key_display(settings::instance(), ui->cmbFunction->currentIndex())))
        {
            ui->cmbShortcutKey->setCurrentIndex(i);
            break;
        }
    }
}

// 设置界面--更新临时英文选项框
void SettingWin::update_tmp_engish_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbTmpEnglish, freewb_single_shortcut_index_from_token(settings::instance().get_tempEnglish()), freewb_single_shortcut_index_from_token(settings::instance().get_shortcutInput()), freewb_single_shortcut_index_from_token(settings::instance().get_tempPinyin()));
}

// 设置界面--更新快捷输入选项框
void SettingWin::update_short_input_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbShortcutInput, freewb_single_shortcut_index_from_token(settings::instance().get_shortcutInput()), freewb_single_shortcut_index_from_token(settings::instance().get_tempEnglish()), freewb_single_shortcut_index_from_token(settings::instance().get_tempPinyin()));
}

// 设置界面--更新临时拼音选项框
void SettingWin::update_tmp_pinyin_cmb()
{
    swBuildSingleShortcutCombo(ui->cmbTmpPinyin, freewb_single_shortcut_index_from_token(settings::instance().get_tempPinyin()), freewb_single_shortcut_index_from_token(settings::instance().get_tempEnglish()), freewb_single_shortcut_index_from_token(settings::instance().get_shortcutInput()));
}

void SettingWin::init_ui_setting_page()
{
    init_skin_select_cmb();
    ui->ckbAutoLocate->setChecked(settings::instance().get_toolbarAutoLocate());
    ui->ckbAutoExtend->setChecked(settings::instance().get_toolbarAutoExpand());
    ui->ckbEnableUiAudioEffect->setChecked(settings::instance().get_uiAudioEffect());
    ui->ckbDispRealHelp->setChecked(settings::instance().get_showRealtimeHelp());
    ui->ckbHideToolbar->setChecked(settings::instance().get_hideToolbar());
    ui->ckbDispRealHelp->setChecked(settings::instance().get_showRealtimeHelp());
    ui->spbToolbarTransparency->setValue(settings::instance().get_toolbarTransparency());
    // ui->labelToolbar->setWindowOpacity( 1 - ui->spbToolbarTransparency->value()/100.0 ); ?无效
}

void SettingWin::init_skin_select_cmb()
{
    ui->cmbSkinSelect->clear();

    QString skinDir = INSTALL_DIR + "/skin/";
    QStringList dirList = QDir(skinDir).entryList(QDir::Dirs);
    dirList.removeOne(".");
    dirList.removeOne("..");

    QStringList skinIdList;
    foreach(QString skinId, dirList)
    {
        if (QFile(skinDir + skinId + "/skin.ini").exists())
        {
            skinIdList << skinId;
            ui->cmbSkinSelect->addItem(skinId);
        }
    }
    std::vector<std::string> skinVec;
    skinVec.reserve(static_cast<size_t>(skinIdList.size()));
    for (const QString &id : skinIdList)
        skinVec.push_back(fromStdUtf8(id));
    freewb_runtime_set_skin_list(skinVec);

    for (int i = 0; i < ui->cmbSkinSelect->count(); i++)
    {
        if (ui->cmbSkinSelect->itemText(i) == toQStringUtf8(settings::instance().get_curSkinId()))
        {
            ui->cmbSkinSelect->setCurrentIndex(i);
            break;
        }
    }

    update_toolbar_preview(ui->cmbSkinSelect->currentText());
}

void SettingWin::update_toolbar_preview(const QString &skinId)
{
    QString iamge = INSTALL_DIR + "/skin/" + skinId + "/toolbar.png";
    ui->labelToolbar->setStyleSheet(QString("border-image:url(%1);").arg(iamge));
}

void SettingWin::init_candidate_ui_page()
{
    if (static_cast<CandiWinDispMode>(settings::instance().get_candiWinDispMode()) == CWDM_ONE_ROW)
    {
        ui->cmbCandiWinMode->setCurrentIndex(0);
    }
    else if (static_cast<CandiWinDispMode>(settings::instance().get_candiWinDispMode()) == CWDM_MULTI_ROW)
    {
        ui->cmbCandiWinMode->setCurrentIndex(1);
    }
    const char separateChar = freewb_separate_char_from_string(settings::instance().get_separateChar());
    ui->ledtSeparateChar->setText(separateChar == 0 ? QString() : QString(QChar(separateChar)));

    ui->ckbUseGradientBgColor->setChecked(settings::instance().get_useGradientColor());
    ckb_useGradientBgColor_updated(settings::instance().get_useGradientColor());
    ui->ckbUseBgImage->setChecked(settings::instance().get_useBgImage());
    ckb_useBgImage_updated(settings::instance().get_useBgImage());
    ui->ckbUseTile->setChecked(settings::instance().get_enableTiled());
    if (settings::instance().get_curSkinId() == std::string("default"))
    {
        ui->spbCornerRadian->setEnabled(true);
        ui->ckbUseGradientBgColor->setEnabled(true);
        ui->ckbUseBgImage->setEnabled(true);
        if (settings::instance().get_useBgImage())
        {
            ui->ckbUseTile->setEnabled(true);
        }
    }
    else
    {
        ui->spbCornerRadian->setEnabled(false);
        ui->ckbUseGradientBgColor->setEnabled(false);
        ui->ckbUseBgImage->setEnabled(false);
        ui->ckbUseTile->setEnabled(false);
    }

    ui->spbCornerRadian->setValue(settings::instance().get_radius());
    ui->spbCandiTransparency->setValue(settings::instance().get_transparency());
    ui->spbCandiItemNum->setValue(settings::instance().get_candiWordCount());
    ui->spbCandiCharNum->setValue(settings::instance().get_candiCharCount());

    update_fram_candidate_win();
}

void SettingWin::ckb_useGradientBgColor_updated(bool checked)
{
    if (checked)
    {
        ui->ckbUseBgImage->setChecked(false);
        ui->ckbUseTile->setEnabled(false);
        ui->btnCandiBg->hide();
        ui->btnCandiBgColor0->show();
        ui->btnCandiBgColor1->show();
    }
    else if (!ui->ckbUseBgImage->isChecked())
    {
        ui->btnCandiBg->setText("背景色");
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::ckb_useBgImage_updated(bool checked)
{
    if (checked)
    {
        ui->ckbUseGradientBgColor->setChecked(false);
        ui->ckbUseTile->setEnabled(true);
        ui->btnCandiBg->setText("背景图");
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
    else if (!ui->ckbUseGradientBgColor->isChecked())
    {
        ui->ckbUseTile->setEnabled(false);
        ui->btnCandiBg->setText("背景色");
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::update_fram_candidate_win()
{
    QString style;

    if (settings::instance().get_curSkinId() == std::string("default"))
    {
        int boederRadius = settings::instance().get_radius();
        QColor boderColor = swQColorFromSpec(settings::instance().get_borderColor());
        QColor bgColor = swQColorFromSpec(settings::instance().get_bgColor());
        QColor gradienColor0 = swQColorFromSpec(settings::instance().get_gradientColor0());
        QColor gradienColor1 = swQColorFromSpec(settings::instance().get_gradientColor1());

        QString borderColorStyle = QString("border-color:rgb(%1,%2,%3);").arg(boderColor.red()).arg(boderColor.green()).arg(boderColor.blue());

        if (settings::instance().get_useGradientColor())
        {
            QString gradienColorStyle =
                QString("background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));").arg(gradienColor0.red()).arg(gradienColor0.green()).arg(gradienColor0.blue()).arg(gradienColor1.red()).arg(gradienColor1.green()).arg(gradienColor1.blue());
            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(gradienColorStyle);
        }
        else if (settings::instance().get_useBgImage())
        {
            QString bgImageStyle = QString("%1:url(%2);").arg(settings::instance().get_enableTiled() ? "background-image" : "border-image").arg(toQStringUtf8(settings::instance().get_bgImage()));

            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "%3"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(bgImageStyle);
        }
        else
        {
            style = QString("#framCandidateWin{"
                            "border-width:1px;"
                            "border-style:solid;"
                            "border-radius:%1px;"
                            "%2"
                            "background-color:rgb(%3,%4,%5);"
                            "}")
                        .arg(boederRadius)
                        .arg(borderColorStyle)
                        .arg(bgColor.red())
                        .arg(bgColor.green())
                        .arg(bgColor.blue());
        }

        if (settings::instance().get_useGradientColor())
        {
            ui->btnCandiBgColor0->show();
            ui->btnCandiBgColor1->show();
        }
        else
        {
            ui->btnCandiBg->show();
        }

        ui->btnCandiBorderColor->show();
    }
    else
    {
        ui->btnCandiBg->hide();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
        ui->btnCandiBorderColor->hide();
    }

    ui->framCandidateWin->setStyleSheet(style);

    QColor color = swQColorFromSpec(settings::instance().get_candiWordTextColor());
    ui->btnCandiAutoWord->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));

    color = swQColorFromSpec(settings::instance().get_candiPromptTextColor());
    ui->btnCandiPrompt->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));

    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::init_candidate_option_page()
{
    ui->cmb23RecodeSelect->setCurrentIndex(settings::instance().get_recodeSelectKey());
    ui->cmbPrevNextPage->setCurrentIndex(freewb_candi_page_index_from_token(settings::instance().get_candiPageKey()));
    // ui->ledt2ndRecode->setText( QChar(settings::instance().get_second_recode_key()) );
    // ui->ledt3rdRecode->setText( QChar(settings::instance().get_third_recode_key()) );
    ui->ckbCursorFollow->setChecked(settings::instance().get_cursorFollow());
    ui->ckbHideCandiChinese->setChecked(settings::instance().get_hideCandiWin());
    ui->ckbDispOpPrompt->setChecked(settings::instance().get_showOpRemindInfo());
    ui->ckbDispOpDict->setChecked(settings::instance().get_showCandDictInfo());

    // ui->ledtPrecPage->setText( QChar(settings::instance().get_prevPage_key()) );
    // ui->ledtNextPage->setText( QChar(settings::instance().get_nextPage_key()) );
    ui->ckbShiftSelectRecode->setChecked(settings::instance().get_shiftSelectRecode());
}

// 切换设置界面右侧选项页
void SettingWin::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);

    if (current == m_listItemCommon)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCommon);
    }
    else if (current == m_listItemAdvance)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageAdvance);
    }
    else if (current == m_listItemOthers)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageOthers);
    }
    else if (current == m_listItemUi)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageUi);
    }
    else if (current == m_listItemCandidateWinUi)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCandidateWinUi);
    }
    else if (current == m_listItemCandidateWinOption)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageCandidateWinOption);
    }
    else if (current == m_listItemShortcutKey)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageShortcutKey);
    }
    else if (current == m_listItemCustomKeyChar)
    {
        m_kbCustomKeyChar->update_keyboard_button(); // 初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget(ui->pageCustomKeyChar);
    }
    else if (current == m_listItemCustomKeyMark)
    {
        m_kbCustomKeyMark->update_keyboard_button(); // 初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget(ui->pageCustomKeyMark);
    }
    else if (current == m_listItemVersionInfo)
    {
        ui->stackedWidget->setCurrentWidget(ui->pageVersionInfo);
    }
    else if (current == m_listItemBug)
    {
        // QDesktopServices::openUrl( QUrl("http://www.freewb.org") );
    }
}

void SettingWin::on_btnSettingOption_clicked()
{
    freewb_runtime_toggle_show_all_group();
    if (freewb_runtime_show_all_group())
    {
        ui->btnSettingOption->setText("显示【常用】选项");
    }
    else
    {
        ui->btnSettingOption->setText("显示【所有】选项");
    }

    update_listwidget_item();
}

void SettingWin::on_btnOk_clicked()
{

    close();
    move(m_defaultPopPosition);
    puts("win seting");
    settings::instance().save();
    g_settingsNotifier.notifySettingDataChangedToFcitx();
}

void SettingWin::on_btnCancel_clicked()
{
    close();
    move(m_defaultPopPosition);
}

void SettingWin::on_btnHelp_clicked()
{
    system("firefox ~/.local/freewb/help/help.html > /dev/null 2>&1 &");
}

void SettingWin::on_ckbCodeRemind_stateChanged(int arg1)
{
    //    qDebug() << DBG_TRACE << arg1;

    if (arg1 == Qt::Checked)
    {
        settings::instance().set_codeRemind(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_codeRemind(false);
    }
}

void SettingWin::on_ckbSpaceFullWhenCharHalf_toggled(bool checked)
{
    settings::instance().set_spaceFullWhenCharHalf(checked);
}

void SettingWin::on_ckbWordThink_toggled(bool checked)
{
    settings::instance().set_wordThink(checked);
}

void SettingWin::on_ckbSmartMark_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_smartMark(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_smartMark(false);
    }
}

void SettingWin::on_ckbRemindExistWord_toggled(bool checked)
{
    settings::instance().set_remindExistWord(checked);
}

void SettingWin::on_ckbAlertWhenEmptyCode_toggled(bool checked)
{
    settings::instance().set_alertWhenEmptyCode(checked);
}

void SettingWin::on_ckbUseAudioFile_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        freewb_runtime_set_use_audio_file(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        freewb_runtime_set_use_audio_file(false);
    }
}

void SettingWin::on_ckbAutoAdjustFreq_toggled(bool checked)
{
    settings::instance().set_autoAdjustFreq(checked);
}

void SettingWin::on_ckbShiftCommitChar_toggled(bool checked)
{
    settings::instance().set_shiftCommitChar(checked);
}

void SettingWin::on_ckbInputStatistic_toggled(bool checked)
{
    settings::instance().set_inputStatistic(checked);
}

void SettingWin::on_ckbRepeatCalib_toggled(bool checked)
{
    settings::instance().set_recodeCalib(checked);
}

void SettingWin::on_ckbTypeEffect_toggled(bool checked)
{
    settings::instance().set_typeEffect(checked);
}

void SettingWin::on_ckbAutoWordGroup_activated(int index)
{
    if (index == 0)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_LOSS);
    }
    else if (index == 1)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_FORBID);
    }
    else if (index == 2)
    {
        settings::instance().set_autoWordGroupOpt(AWGO_SAVE);
    }
}

void SettingWin::on_ledtAutoToEnStr_textChanged(const QString &arg1)
{
    settings::instance().set_autoToEnStr(fromStdUtf8(arg1));
}

void SettingWin::on_ledtAutoToHalf_textChanged(const QString &arg1)
{
    settings::instance().set_CoustomMark(fromStdUtf8(arg1));
}

void SettingWin::on_ckbAutoHalfMarkAfterNum_toggled(bool checked)
{
    settings::instance().set_autoToHalfMarkFlg(checked);
}

void SettingWin::on_cmbFunction_activated(int index)
{
    Q_UNUSED(index)
    update_custom_shortkey_cmb();
}

void SettingWin::on_cmbShortcutKey_activated(int index)
{
    for (int i = 0; i < CSK_NUM; i++)
    {
        if (ui->cmbShortcutKey->itemText(index) == toQStringUtf8(freewb_combine_shortcut_display_name(i)))
        {
            freewb_custom_shortcut_set_combine_index(settings::instance(), ui->cmbFunction->currentIndex(), i);
        }
    }
}

void SettingWin::on_ckbDisableAllShortcutKey_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_disableAllShortcutKey(true);
        ui->cmbFunction->setEnabled(false);
        ui->cmbShortcutKey->setEnabled(false);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_disableAllShortcutKey(false);
        ui->cmbFunction->setEnabled(true);
        ui->cmbShortcutKey->setEnabled(true);
    }
}

void SettingWin::on_ckbDisableFullHalfKey_toggled(bool checked)
{
    settings::instance().set_disableFullHalfSwitch(checked);
}

void SettingWin::on_cmbSwitchCnEn_activated(int index)
{
    RcodeSelectShortcutKey rssk = static_cast<RcodeSelectShortcutKey>(settings::instance().get_recodeSelectKey());
    CnEnSwitchShortcutKey key = static_cast<CnEnSwitchShortcutKey>(index);
    if ((rssk == RSSK_CTRL && (key == CESSK_CTRL || key == CESSK_LEFT_CTRL || key == CESSK_RIGHT_CTRL)) || (rssk == RSSK_SHIFT && (key == CESSK_SHIFT || key == CESSK_LEFT_SHIFT || key == CESSK_RIGHT_SHIFT)))
    {
        m_msgBox = new QMessageBox(this);
        // m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText("您设置的快捷键将与二三重码选择键冲突，确认设置？");
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText("否(&N)");
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmbSwitchCnEn->setCurrentIndex(freewb_cn_en_switch_index_from_token(settings::instance().get_cnEnSwitch()));
            return;
        }
    }

    settings::instance().set_cnEnSwitch(freewb_cn_en_switch_token_from_index(key));
}

void SettingWin::on_cmbTmpEnglish_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    const int idx = ui->cmbTmpEnglish->currentData().toInt();
    settings::instance().set_tempEnglish(freewb_single_shortcut_ini_token(idx));

    update_short_input_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbShortcutInput_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    const int idx = ui->cmbShortcutInput->currentData().toInt();
    settings::instance().set_shortcutInput(freewb_single_shortcut_ini_token(idx));

    update_tmp_engish_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbTmpPinyin_activated(const QString &arg1)
{
    Q_UNUSED(arg1);
    const int idx = ui->cmbTmpPinyin->currentData().toInt();
    settings::instance().set_tempPinyin(freewb_single_shortcut_ini_token(idx));

    update_tmp_engish_cmb();
    update_short_input_cmb();
}

void SettingWin::on_btnRestoreShortcutKey_clicked()
{
    m_msgBox = new QMessageBox(this);
    // m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
    m_msgBox->setIcon(QMessageBox::Warning);
    m_msgBox->setText("确定将快捷键都恢复成默认键值吗？");
    m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
    m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
    m_msgBox->button(QMessageBox::No)->setText("否(&N)");
    m_msgBox->setDefaultButton(QMessageBox::Yes);

    int ret = m_msgBox->exec();
    delete m_msgBox;
    if (ret == QMessageBox::Yes)
    {
        settings::instance().restore_default_shortcutkey();
        init_shortcutkey_page();
    }
}

void SettingWin::slot_custom_keyboard_char_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{
    // qDebug() << keyValue.commChar;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle("设置键盘字符");
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_CHAR, keyName, keyValue.commChar, keyValue.shiftChar);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_keyboard_mark_clicked(SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue)
{
    // qDebug() << keyValue.commMark;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle("设置键盘标点");
    m_customKeyDialog->set_custom_symbol(VKM_CUSTOM_MARK, keyName, keyValue.commMark, keyValue.shiftMark);
    m_customKeyDialog->exec();
}

void SettingWin::slot_custom_btn_ok_clicked(const QString &commSymbol, const QString &shiftSymbol)
{
    // qDebug() << commSymbol << shiftSymbol;

    if (ui->stackedWidget->currentWidget() == ui->pageCustomKeyChar)
    {
        m_curCustomKeyValue.commChar = commSymbol;
        m_curCustomKeyValue.shiftChar = shiftSymbol;
        m_kbCustomKeyChar->update_customkey_button(m_curSymbolKeyIdx, commSymbol, shiftSymbol);
    }
    else
    {
        m_curCustomKeyValue.commMark = commSymbol;
        m_curCustomKeyValue.shiftMark = shiftSymbol;
        m_kbCustomKeyMark->update_customkey_button(m_curSymbolKeyIdx, commSymbol, shiftSymbol);
    }
#ifdef DEBUG
    qDebug() << m_curCustomKeyValue.commChar << m_curCustomKeyValue.shiftChar << m_curCustomKeyValue.commMark << m_curCustomKeyValue.shiftMark;
#endif
    std::string chars = settings::instance().get_CoustomChar();
    std::string marks = settings::instance().get_CoustomMark();
    if (freewb_custom_key_info_apply_to_values(chars, marks, static_cast<int>(m_curSymbolKeyIdx), KEY_SYMBOL_NUM, customKeyFromQt(m_curCustomKeyValue)))
    {
        settings::instance().set_CoustomChar(chars);
        settings::instance().set_CoustomMark(marks);
    }
}

void SettingWin::on_cmbSkinSelect_activated(const QString &arg1)
{
    settings::instance().set_curSkinId(fromStdUtf8(arg1));
    update_toolbar_preview(arg1);
    init_candidate_ui_page();
}

void SettingWin::on_ckbAutoLocate_toggled(bool checked)
{
    settings::instance().set_toolbarAutoLocate(checked);
}

void SettingWin::on_ckbAutoExtend_toggled(bool checked)
{
    settings::instance().set_toolbarAutoExpand(checked);
}

void SettingWin::on_ckbEnableUiAudioEffect_toggled(bool checked)
{
    settings::instance().set_uiAudioEffect(checked);
}

void SettingWin::on_ckbDispRealHelp_toggled(bool checked)
{
    settings::instance().set_showRealtimeHelp(checked);
}

void SettingWin::on_ckbHideToolbar_toggled(bool checked)
{
    settings::instance().set_hideToolbar(checked);
}

void SettingWin::on_spbToolbarTransparency_valueChanged(int arg1)
{
    settings::instance().set_toolbarTransparency(arg1);
    // ui->labelToolbar->setWindowOpacity( 1 - arg1/100.0 );
}

void SettingWin::on_cmbCandiWinMode_activated(int index)
{
    if (index == 0)
    {
        settings::instance().set_candiWinDispMode(CWDM_ONE_ROW);
    }
    else if (index == 1)
    {
        settings::instance().set_candiWinDispMode(CWDM_MULTI_ROW);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ledtSeparateChar_textChanged(const QString &arg1)
{
    if (!arg1.length())
    {
        settings::instance().set_separateChar(freewb_separate_char_to_string(0));
    }
    else
    {
        settings::instance().set_separateChar(freewb_separate_char_to_string(arg1.toLatin1().at(0)));
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbUseGradientBgColor_toggled(bool checked)
{
    settings::instance().set_useGradientColor(checked);
    if (checked)
    {
        settings::instance().set_useBgImage(false);
    }
    ckb_useGradientBgColor_updated(checked);
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseBgImage_toggled(bool checked)
{
    settings::instance().set_useBgImage(checked);
    if (checked)
    {
        settings::instance().set_useGradientColor(false);
    }
    ckb_useBgImage_updated(checked);
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseTile_toggled(bool checked)
{
    settings::instance().set_enableTiled(checked);
    update_fram_candidate_win();
}

// 通过setValue函数设置值时也会出发该槽函数!!!!!!!!
void SettingWin::on_spbCandiItemNum_valueChanged(int arg1)
{
    settings::instance().set_candiWordCount(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_spbCornerRadian_valueChanged(int arg1)
{
    settings::instance().set_radius(arg1);
    update_fram_candidate_win();
}

void SettingWin::on_spbCandiTransparency_valueChanged(int arg1)
{
    settings::instance().set_transparency(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_spbCandiCharNum_valueChanged(int arg1)
{
    settings::instance().set_candiCharCount(arg1);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_btnCandiFont_clicked()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont(freewb_candi_text_qfont(settings::instance()));
    if (dialog.exec() == QFontDialog::Accepted)
    {
        freewb_candi_text_font_apply_qfont(settings::instance(), dialog.selectedFont());
        g_settingsNotifier.notifySettingDataChangedToLocal();
        // ui->btnCandiFont->setFont( dialog.selectedFont() );
    }
}

void SettingWin::on_btnCandiBg_clicked()
{
    if (settings::instance().get_useBgImage())
    {
        QString file = QFileDialog::getOpenFileName(this, "选择背景图片", qgetenv("HOME"), "Images(*.png *.bmp *.jpg)");
        settings::instance().set_bgImage(fromStdUtf8(file));
        update_fram_candidate_win();
    }
    else
    {
        QColorDialog dialog(this);

        dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_bgColor()));
        if (dialog.exec() == QColorDialog::Accepted)
        {
            settings::instance().set_bgColor(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
            update_fram_candidate_win();
        }
    }
}

void SettingWin::on_btnCandiBgColor0_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_gradientColor0()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_gradientColor0(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBgColor1_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_gradientColor1()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_gradientColor1(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBorderColor_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_borderColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        settings::instance().set_borderColor(fromStdUtf8(dialog.selectedColor().name(QColor::HexArgb)));
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiAutoWord_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_candiWordTextColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        QColor color = dialog.selectedColor();
        settings::instance().set_candiWordTextColor(fromStdUtf8(color.name(QColor::HexArgb)));
        ui->btnCandiAutoWord->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));
        g_settingsNotifier.notifySettingDataChangedToLocal();
    }
}

void SettingWin::on_btnCandiPrompt_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor(swQColorFromSpec(settings::instance().get_candiPromptTextColor()));
    if (dialog.exec() == QColorDialog::Accepted)
    {
        QColor color = dialog.selectedColor();
        settings::instance().set_candiPromptTextColor(fromStdUtf8(color.name(QColor::HexArgb)));
        ui->btnCandiPrompt->setStyleSheet(QString("color:rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()));
        g_settingsNotifier.notifySettingDataChangedToLocal();
    }
}

/******************************** 候选窗选项 ********************************/
// void SettingWin::on_ledt2ndRecode_textChanged( const QString &arg1 )
//{
//     if ( !arg1.isEmpty() )
//     {
//         settings::instance().set_second_recode_key( arg1.at(0).toLatin1() );
//     }
// }

// void SettingWin::on_ledt3rdRecode_textChanged( const QString &arg1 )
//{
//     if ( !arg1.isEmpty() )
//     {
//         settings::instance().set_third_recode_key( arg1.at(0).toLatin1() );
//     }
// }

void SettingWin::on_ckbShiftSelectRecode_toggled(bool checked)
{
    settings::instance().set_shiftSelectRecode(checked);
}

void SettingWin::on_ckbCursorFollow_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_cursorFollow(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_cursorFollow(false);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbHideCandiChinese_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        settings::instance().set_hideCandiWin(true);
    }
    else if (arg1 == Qt::Unchecked)
    {
        settings::instance().set_hideCandiWin(false);
    }
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbDispOpPrompt_toggled(bool checked)
{
    settings::instance().set_showOpRemindInfo(checked);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_ckbDispOpDict_toggled(bool checked)
{
    // printf("dict=%d\n",checked);
    settings::instance().set_showCandDictInfo(checked);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

void SettingWin::on_cmb23RecodeSelect_activated(int index)
{
    QString conflictInfo;
    int conflictFlg = 0;
    CnEnSwitchShortcutKey cnEnSwitchShortcutKey = static_cast<CnEnSwitchShortcutKey>(freewb_cn_en_switch_index_from_token(settings::instance().get_cnEnSwitch()));

    RcodeSelectShortcutKey key = static_cast<RcodeSelectShortcutKey>(index);
    if (key == RSSK_COMMA_PERIOD && static_cast<CandiPageShortcutKey>(freewb_candi_page_index_from_token(settings::instance().get_candiPageKey())) == CPSK_COMMA_PERIOD)
    {
        conflictFlg = 1;
        conflictInfo = "您设置的快捷键将与上下翻页键冲突，确认设置？";
    }
    else if ((key == RSSK_CTRL && (cnEnSwitchShortcutKey == CESSK_CTRL || cnEnSwitchShortcutKey == CESSK_LEFT_CTRL || cnEnSwitchShortcutKey == CESSK_RIGHT_CTRL)) ||
             (key == RSSK_SHIFT && (cnEnSwitchShortcutKey == CESSK_SHIFT || cnEnSwitchShortcutKey == CESSK_LEFT_SHIFT || cnEnSwitchShortcutKey == CESSK_RIGHT_SHIFT)))
    {
        conflictFlg = 1;
        conflictInfo = "您设置的快捷键将与中英文切换键冲突，确认设置？";
    }

    if (conflictFlg)
    {
        m_msgBox = new QMessageBox(this);
        // m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText(conflictInfo);
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText("否(&N)");
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmb23RecodeSelect->setCurrentIndex(settings::instance().get_recodeSelectKey());
            return;
        }
    }

    settings::instance().set_recodeSelectKey(key);
    freewb_apply_recode_select_pair(settings::instance(), key);
}

void SettingWin::on_cmbPrevNextPage_activated(int index)
{
    CandiPageShortcutKey key = static_cast<CandiPageShortcutKey>(index);
    if (key == CPSK_COMMA_PERIOD && static_cast<RcodeSelectShortcutKey>(settings::instance().get_recodeSelectKey()) == RSSK_COMMA_PERIOD)
    {
        m_msgBox = new QMessageBox(this);
        // m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon(QMessageBox::Warning);
        m_msgBox->setText("您设置的快捷键将与二三重码选择键冲突，确认设置？");
        m_msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        m_msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
        m_msgBox->button(QMessageBox::No)->setIcon(QIcon());
        m_msgBox->button(QMessageBox::No)->setText("否(&N)");
        m_msgBox->setDefaultButton(QMessageBox::No);
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if (ret == QMessageBox::No)
        {
            ui->cmbPrevNextPage->setCurrentIndex(freewb_candi_page_index_from_token(settings::instance().get_candiPageKey()));
            return;
        }
    }

    settings::instance().set_candiPageKey(freewb_candi_page_token_from_index(key));
    freewb_apply_candidate_page_hotkeys(settings::instance(), key);
}
