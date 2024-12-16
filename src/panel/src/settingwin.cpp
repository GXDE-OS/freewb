#include "settingwin.h"
#include "ui_settingwin.h"
#include "commdefine.h"
#include "settings.h"

//设置窗口样式表
#define QSS_FILE ":/qss/settingwin.qss"

#define QSS_TOOL_TIPS \
"color: rgb(56, 56, 56);"\
"font: 12pt \"Ubuntu\";"\
"padding: 10px;"\
"background-color: rgb(254, 255, 226);"\
"border-radius: 5px;"\
"border-width: 2px;"\
"border-style: solid;"\
"border-color: rgb(200, 200, 200);"



#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"


#define QSS_BTN_CLOSE0 "border-image: url(:/image/setting/close0.png);"
#define QSS_BTN_CLOSE1 "border-image: url(:/image/setting/close1.png);"
#define QSS_BTN_CLOSE2 "border-image: url(:/image/setting/close2.png);"

//设置组子分组图标
#define ICO_SETTING_GROUP  ":/image/setting/group.png"


#define VERSION_ARM "本产品基于极点五笔Windows版本开发，极点五笔版权归杜志民先生所有。"
#define VERSION_X86 ""\
"<html><head/><body><font style='font-family:Ubuntu;font-size:13px;'>"\
"－感谢极点五笔的开创者杜志民先生，设计出属于输入人员"\
"<br/>&nbsp;&nbsp;&nbsp;&nbsp;自己的输入法"\
"<br/>－感谢鹏城实验室自主可控方向对项目开发提供的大力支持"\
"<br/>－感谢北弓智能的开发人员为项目付出的努力"\
"<br/>－感谢openKylin InputMethod SIG的开发支持"\
"<br/>－感谢银河麒麟操作系统团队的技术支持</font></body></html>"


static QDate g_buildDate = QLocale( QLocale::English ).toDate( QString(__DATE__).replace( "  ", " 0" ), "MMM dd yyyy");
static QTime g_buildTime = QTime::fromString( __TIME__ );


SettingWin::SettingWin(QWidget *parent) : QWidget(parent), ui(new Ui::SettingWin)
{
    ui->setupUi(this);

    init_member_data();//确保相关的数据成员初始化完成后再初始化界面外观
    init_mouse_hover_tips();
    init_window_appearance();
    install_evt_filter();

    move( m_defaultPopPosition );


    /////////////////////////
    //ui->labelBugFeedback->hide();
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

    if ( m_kbCustomKeyChar )
    {
        delete m_kbCustomKeyChar;
        m_kbCustomKeyChar = nullptr;
    }

    if ( m_kbCustomKeyMark )
    {
        delete m_kbCustomKeyMark;
        m_kbCustomKeyMark = nullptr;
    }

    if ( m_customKeyDialog )
    {
        delete m_customKeyDialog;
        m_customKeyDialog = nullptr;
    }
}



void SettingWin::init_window_appearance()
{
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::Tool);
  
    setWindowIcon( QIcon(":/image/setting/logo.png") );
    setWindowTitle( "极点设置" );
    //setFont(Settings::get_candidate_text_font());

    ui->labelVersionNum->setText( QString("v3.0  %1 %2")
                                  .arg(g_buildDate.toString("yyyy-MM-dd"))
                                  .arg(g_buildTime.toString("hh:mm:ss")) );
    ui->labelVersion->setText( "极点五笔麒麟版" );
    ui->labelCopyright2->setText( VERSION_X86 );

    // 载入窗口全局UI样式表
    QFile qssFile( QSS_FILE );
    if ( !qssFile.open(QFile::ReadOnly) )
    {
        qWarning() << "open qss file failed!";
    }
    else
    {
        this->setStyleSheet( qssFile.readAll() );
        qssFile.close();
    }
}


void SettingWin::init_member_data()
{
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint( (d->width() - size().width())/2, (d->height() - size().height())/2 );

    //初始化设置界面的软键盘
    m_kbCustomKeyChar = new Keyboard( VKM_CUSTOM_CHAR, ui->pageCustomKeyChar );
    m_kbCustomKeyChar->move( 40, 120 );

    m_kbCustomKeyMark = new Keyboard( VKM_CUSTOM_MARK, ui->pageCustomKeyMark );
    m_kbCustomKeyMark->move( 40, 120 );

    //自定义软键盘点击
    connect( m_kbCustomKeyChar, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString&, const CustomKeyValue&)),
             this, SLOT(slot_custom_keyboard_char_clicked(SymbolKeyIdx, const QString&, const CustomKeyValue&)) );
    connect( m_kbCustomKeyMark, SIGNAL(signal_custom_key_clicked(SymbolKeyIdx, const QString&, const CustomKeyValue&)),
             this, SLOT(slot_custom_keyboard_mark_clicked(SymbolKeyIdx, const QString&, const CustomKeyValue&)) );


    m_customKeyDialog = new CustomKeyDialog();
    connect( m_customKeyDialog, SIGNAL(signal_custom_ok_btn_clicked(const QString&, const QString&)),
             this, SLOT(slot_custom_btn_ok_clicked(const QString&, const QString&)) );
}


void SettingWin::init_mouse_hover_tips()
{
    #define TIPS_FILE INSTALL_DIR + "/data/setting_tips.txt"

    m_tooltipsWin.setWindowFlags( Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );
    m_tooltipsWin.setAttribute( Qt::WA_TranslucentBackground );
    m_tooltipsLabel = new QLabel( &m_tooltipsWin );
    m_tooltipsLabel->setStyleSheet( QSS_TOOL_TIPS );


    QFile tipsFile( TIPS_FILE );
    QTextStream textStream( &tipsFile );
    if ( !tipsFile.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        qWarning() << TIPS_FILE << "open failed!";
        return;
    }

    QMap<int, QString> tipsMap;
    QStringList textList = textStream.readAll().split( '\n' );
    tipsFile.close();
    foreach ( QString line, textList )
    {
        if ( line.startsWith("1") )
        {
            //QStringList tips = line.split( '=' );
            QStringList tips;
            tips << line.left( 4 );
            tips << line.mid( 5 );
            if( tips.length() > 1 )
            {
                QString value = tips.at(1);
                tipsMap.insert( tips.at(0).toInt(), value.replace("\\n", "\n") );
            }
        }
    }

    m_tipsTextMap.insert( ui->ckbCodeRemind, tipsMap.value(1001) );
    m_tipsTextMap.insert( ui->ckbSpaceFullWhenCharHalf, tipsMap.value(1002) );
    m_tipsTextMap.insert( ui->ckbWordThink, tipsMap.value(1003) );
    m_tipsTextMap.insert( ui->ckbSmartMark, tipsMap.value(1004) );
    m_tipsTextMap.insert( ui->ckbRemindExistWord, tipsMap.value(1005) );
    m_tipsTextMap.insert( ui->ckbAlertWhenEmptyCode, tipsMap.value(1006) );
    m_tipsTextMap.insert( ui->ckbUseAudioFile, tipsMap.value(1007) );
    m_tipsTextMap.insert( ui->ckbAutoAdjustFreq, tipsMap.value(1008) );

    m_tipsTextMap.insert( ui->ckbShiftCommitChar, tipsMap.value(1101) );
    m_tipsTextMap.insert( ui->ckbInputStatistic, tipsMap.value(1102) );
    m_tipsTextMap.insert( ui->ckbTypeEffect, tipsMap.value(1103) );
    m_tipsTextMap.insert( ui->ckbRepeatCalib, tipsMap.value(1104) );
    m_tipsTextMap.insert( ui->ckbAutoWordGroup, tipsMap.value(1105) );

    m_tipsTextMap.insert( ui->ledtAutoToEnStr, tipsMap.value(1201) );
    m_tipsTextMap.insert( ui->ledtAutoToHalf, tipsMap.value(1202) );

    m_tipsTextMap.insert( ui->cmbSkinSelect, tipsMap.value(1301) );
    m_tipsTextMap.insert( ui->ckbAutoLocate, tipsMap.value(1302) );
    m_tipsTextMap.insert( ui->cmbWhenLossLocation, tipsMap.value(1303) );
    m_tipsTextMap.insert( ui->ckbAutoExtend, tipsMap.value(1304) );
    m_tipsTextMap.insert( ui->ckbEnableUiAudioEffect, tipsMap.value(1305) );
    m_tipsTextMap.insert( ui->ckbDispRealHelp, tipsMap.value(1306) );
    m_tipsTextMap.insert( ui->ckbHideToolbar, tipsMap.value(1307) );
    m_tipsTextMap.insert( ui->spbToolbarTransparency, tipsMap.value(1308) );

    m_tipsTextMap.insert( ui->cmbCandiWinMode, tipsMap.value(1401) );
    m_tipsTextMap.insert( ui->ledtSeparateChar, tipsMap.value(1402) );
    m_tipsTextMap.insert( ui->ckbUseGradientBgColor, tipsMap.value(1403) );
    m_tipsTextMap.insert( ui->ckbUseBgImage, tipsMap.value(1404) );
    m_tipsTextMap.insert( ui->ckbUseTile, tipsMap.value(1405) );
    m_tipsTextMap.insert( ui->spbCornerRadian, tipsMap.value(1406) );
    m_tipsTextMap.insert( ui->spbCandiTransparency, tipsMap.value(1407) );
    m_tipsTextMap.insert( ui->spbCandiItemNum, tipsMap.value(1408) );
    m_tipsTextMap.insert( ui->spbCandiCharNum, tipsMap.value(1409) );

    m_tipsTextMap.insert( ui->btnCandiFont, tipsMap.value(1410) );
    m_tipsTextMap.insert( ui->btnCandiBg, tipsMap.value(1411) );
    m_tipsTextMap.insert( ui->btnCandiBgColor0, tipsMap.value(1412) );
    m_tipsTextMap.insert( ui->btnCandiBgColor1, tipsMap.value(1413) );
    m_tipsTextMap.insert( ui->btnCandiBorderColor, tipsMap.value(1414) );
    m_tipsTextMap.insert( ui->btnCandiAutoWord, tipsMap.value(1415) );
    m_tipsTextMap.insert( ui->btnCandiPrompt, tipsMap.value(1416) );

    m_tipsTextMap.insert( ui->ledt2ndRecode, tipsMap.value(1501) );
    m_tipsTextMap.insert( ui->ledt3rdRecode, tipsMap.value(1502) );
    m_tipsTextMap.insert( ui->ckbCursorFollow, tipsMap.value(1503) );
    m_tipsTextMap.insert( ui->ckbHideCandiChinese, tipsMap.value(1504) );
    m_tipsTextMap.insert( ui->ckbDispOpPrompt, tipsMap.value(1505) );
    m_tipsTextMap.insert( ui->ckbShiftSelectRecode, tipsMap.value(1506) );
    m_tipsTextMap.insert( ui->ledtPrecPage, tipsMap.value(1507) );
    m_tipsTextMap.insert( ui->ledtNextPage, tipsMap.value(1508) );
    m_tipsTextMap.insert( ui->cmb23RecodeSelect, tipsMap.value(1509) );
    m_tipsTextMap.insert( ui->cmbPrevNextPage, tipsMap.value(1510) );
    m_tipsTextMap.insert( ui->ckbDispOpDict, tipsMap.value(1511) );


    m_tipsTextMap.insert( ui->cmbFunction, tipsMap.value(1601) );
    m_tipsTextMap.insert( ui->cmbShortcutKey, tipsMap.value(1602) );
    m_tipsTextMap.insert( ui->ckbDisableAllShortcutKey, tipsMap.value(1603) );
    m_tipsTextMap.insert( ui->ckbDisableFullHalfKey, tipsMap.value(1604) );
    m_tipsTextMap.insert( ui->cmbTmpEnglish, tipsMap.value(1605) );
    m_tipsTextMap.insert( ui->cmbShortcutInput, tipsMap.value(1606) );
    m_tipsTextMap.insert( ui->cmbTmpPinyin, tipsMap.value(1607) );
    m_tipsTextMap.insert( ui->cmbSwitchCnEn, tipsMap.value(1608) );
}


void SettingWin::install_evt_filter()
{
    installEventFilter( this );

#if 0
//    QList<QWidget *> widgets = findChildren<QWidget *>();
//    foreach( QWidget *widget, widgets )
//    {
//        widget->installEventFilter( this );
//    }
#else
    ui->ckbCodeRemind->installEventFilter( this );
    ui->ckbSpaceFullWhenCharHalf->installEventFilter( this );
    ui->ckbWordThink->installEventFilter( this );
    ui->ckbSmartMark->installEventFilter( this );
    ui->ckbRemindExistWord->installEventFilter( this );
    ui->ckbAlertWhenEmptyCode->installEventFilter( this );
    ui->ckbUseAudioFile->installEventFilter( this );
    ui->ckbAutoAdjustFreq->installEventFilter( this );

    ui->ckbShiftCommitChar->installEventFilter( this );
    ui->ckbInputStatistic->installEventFilter( this );
    ui->ckbTypeEffect->installEventFilter( this );
    ui->ckbRepeatCalib->installEventFilter( this );
    ui->ckbAutoWordGroup->installEventFilter( this );

    ui->ledtAutoToEnStr->installEventFilter( this );
    ui->ledtAutoToHalf->installEventFilter( this );

    ui->cmbSkinSelect->installEventFilter( this );
    ui->ckbAutoLocate->installEventFilter( this );
    ui->cmbWhenLossLocation->installEventFilter( this );
    ui->ckbAutoExtend->installEventFilter( this );
    ui->ckbEnableUiAudioEffect->installEventFilter( this );
    ui->ckbDispRealHelp->installEventFilter( this );
    ui->ckbHideToolbar->installEventFilter( this );
    ui->spbToolbarTransparency->installEventFilter( this );

    ui->cmbCandiWinMode->installEventFilter( this );
    ui->ledtSeparateChar->installEventFilter( this );
    ui->ckbUseGradientBgColor->installEventFilter( this );
    ui->ckbUseBgImage->installEventFilter( this );
    ui->ckbUseTile->installEventFilter( this );
    ui->spbCornerRadian->installEventFilter( this );
    ui->spbCandiTransparency->installEventFilter( this );
    ui->spbCandiItemNum->installEventFilter( this );
    ui->spbCandiCharNum->installEventFilter( this );
    ui->cmb23RecodeSelect->installEventFilter( this );
    ui->cmbPrevNextPage->installEventFilter( this );

    ui->btnCandiFont->installEventFilter( this );
    ui->btnCandiBg->installEventFilter( this );
    ui->btnCandiBgColor0->installEventFilter( this );
    ui->btnCandiBgColor1->installEventFilter( this );
    ui->btnCandiBorderColor->installEventFilter( this );
    ui->btnCandiAutoWord->installEventFilter( this );
    ui->btnCandiPrompt->installEventFilter( this );

    ui->ledt2ndRecode->installEventFilter( this );
    ui->ledt3rdRecode->installEventFilter( this );
    ui->ckbCursorFollow->installEventFilter( this );
    ui->ckbHideCandiChinese->installEventFilter( this );
    ui->ckbDispOpPrompt->installEventFilter( this );
    ui->ckbDispOpDict->installEventFilter( this );

    ui->ckbShiftSelectRecode->installEventFilter( this );
    ui->ledtPrecPage->installEventFilter( this );
    ui->ledtNextPage->installEventFilter( this );

    ui->cmbFunction->installEventFilter( this );
    ui->cmbShortcutKey->installEventFilter( this );
    ui->ckbDisableAllShortcutKey->installEventFilter( this );
    ui->ckbDisableFullHalfKey->installEventFilter( this );
    ui->cmbTmpEnglish->installEventFilter( this );
    ui->cmbShortcutInput->installEventFilter( this );
    ui->cmbTmpPinyin->installEventFilter( this );
    ui->cmbSwitchCnEn->installEventFilter( this );
#endif
}

void SettingWin::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent( event );
}


void SettingWin::mouseReleaseEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent( event );
}


void SettingWin::mouseMoveEvent( QMouseEvent *event )
{
    if( m_mouseIsPressed )
     {
        QPoint mouseCurrPosition = event->globalPos();
        move( pos() + mouseCurrPosition - m_mouseLastPosition );
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent( event );
}

bool SettingWin::eventFilter( QObject *obj, QEvent *event )
{
    bool isProcessed = false;

    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvt = static_cast<QKeyEvent *>( event );
        if(keyEvt->key()==Qt::Key_Escape)
            close();
    }
    else if ( event->type() == QEvent::ToolTip )
    {
        //qDebug() << obj->objectName();
        if ( m_tipsTextMap.contains(qobject_cast<QWidget*>(obj)) )
        {
            QString tips = m_tipsTextMap.value(qobject_cast<QWidget*>(obj));
            if ( !tips.isEmpty() )
            {
                m_tooltipsLabel->setText( tips );
                m_tooltipsLabel->adjustSize();
                m_tooltipsWin.adjustSize();
                m_tooltipsWin.move( QCursor::pos().x() + 10, QCursor::pos().y() + 10 );
                m_tooltipsWin.show();
                m_tooltipsWinShowFlg = true;
            }
        }
    }
    else if ( event->type() == QEvent::Leave && m_tooltipsWinShowFlg )
    {
        m_tooltipsWin.hide();
        m_tooltipsWinShowFlg = false;
    }


    if ( isProcessed == false )
    {
        return QWidget::eventFilter( obj, event );
    }

    return isProcessed;
}


void SettingWin::slot_open_win()
{
    Settings::copy_all_setting_data_to_buffer();
    slot_init_all_setting_page();

    show();

    activateWindow();

    ui->listWidget->setCurrentRow( 0 );
    ui->stackedWidget->setCurrentIndex( 0 );
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
    Settings::copy_all_setting_data_to_buffer();
    slot_init_all_setting_page();
    show();
    activateWindow();

    for ( int i = 0; i < ui->listWidget->count(); i++ )
    {
        if ( ui->listWidget->item(i)->text() == "版本信息" )
        {
            ui->listWidget->setCurrentRow( i );
        }
    }
    ui->stackedWidget->setCurrentWidget( ui->pageVersionInfo );
}

void SettingWin::slot_open_advanced_settting_page()
{
    Settings::copy_all_setting_data_to_buffer();
    slot_init_all_setting_page();
    show();
    activateWindow();

    ui->listWidget->setCurrentRow( 1 );
    ui->stackedWidget->setCurrentWidget( ui->pageAdvance );
}

//更新设置界面左侧的设置选项组
void SettingWin::update_listwidget_item()
{
    ui->listWidget->clear();

    m_listItemCommon = new QListWidgetItem( "常用选项", ui->listWidget );
    m_listItemAdvance = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "高级选项", ui->listWidget );
    m_listItemOthers = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "其他设置", ui->listWidget );

    if ( Settings::is_show_all_group() )
    {
        m_listItemUi = new QListWidgetItem( "界面设置", ui->listWidget );
        m_listItemCandidateWinUi = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "候选窗界面", ui->listWidget );
        m_listItemCandidateWinOption = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "候选窗选项", ui->listWidget );
    }

    m_listItemShortcutKey = new QListWidgetItem( "设置快捷键", ui->listWidget );
    m_listItemCustomKeyChar = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "定义软键盘", ui->listWidget );
    m_listItemCustomKeyMark = new QListWidgetItem( QIcon(ICO_SETTING_GROUP), "自定义标点", ui->listWidget );
    m_listItemVersionInfo = new QListWidgetItem( "版本信息", ui->listWidget );
    if ( g_cpuType == CT_X86 )
    {
        //m_listItemBug = new QListWidgetItem( "问题反馈", ui->listWidget );
    }

    ui->stackedWidget->setCurrentWidget( ui->pageCommon );
    ui->listWidget->setCurrentRow( 0 );
}



//初始化常用选项设置页面
void SettingWin::init_common_page()
{
    ui->ckbCodeRemind->setChecked( Settings::get_codeRemind_flg() );
    ui->ckbSpaceFullWhenCharHalf->setChecked( Settings::get_spaceFullWhenCharHalf_flg() );
    ui->ckbWordThink->setChecked( Settings::get_wordThink_flg() );
    ui->ckbSmartMark->setChecked( Settings::get_smartMark_flg() );
    ui->ckbRemindExistWord->setChecked( Settings::get_remindExistWord_flg() );
    ui->ckbAlertWhenEmptyCode->setChecked( Settings::get_alertWhenEmptyCode_flg() );
    ui->ckbUseAudioFile->setChecked( Settings::get_useAudioFile_flg() );
    ui->labelUseAudioFile->hide(); ui->ckbUseAudioFile->hide();///////////////
    ui->ckbAutoAdjustFreq->setChecked( Settings::get_autoAdjustFreq_flg() );
}


//初始化高级设置页面
void SettingWin::init_advance_page()
{
    ui->ckbShiftCommitChar->setChecked( Settings::get_shiftCommitChar_flg() );
    ui->ckbInputStatistic->setChecked( Settings::get_inputStatistic_flg() );
    ui->ckbTypeEffect->setChecked( Settings::get_typeEffect_flg() );
    ui->ckbRepeatCalib->setChecked( Settings::get_recodetCalib_flg() );

    AutoWordGroupOpt opt = Settings::get_autoWordGroupOpt_flg();
    if ( opt == AWGO_FORBID )
    {
        ui->ckbAutoWordGroup->setCurrentIndex( 0 );
    }
    else if ( opt == AWGO_LOSS )
    {
        ui->ckbAutoWordGroup->setCurrentIndex( 1 );
    }
    else if ( opt == AWGO_SAVE )
    {
        ui->ckbAutoWordGroup->setCurrentIndex( 2 );
    }
}

//初始化其它选项设置页面
void SettingWin::init_others_page()
{
    ui->ledtAutoToEnStr->setText( Settings::get_autoToEn_str() );
    //ui->ledtAutoToHalf->setText( Settings::get_autoToHalf_mark() );
    ui->ledtAutoToHalf->hide();
    ui->ckbAutoHalfMarkAfterNum->setChecked( Settings::get_autoToHalfMark_flg() );
}


//初始化快捷键设置页
void SettingWin::init_shortcutkey_page()
{
    update_custom_shortkey_cmb();
    update_tmp_engish_cmb();
    update_short_input_cmb();
    update_tmp_pinyin_cmb();

    ui->ckbDisableFullHalfKey->setChecked( Settings::get_disableFullHalfSwitchShortcutkey_flg() );
    ui->ckbDisableAllShortcutKey->setChecked( Settings::get_disableAllCsk_flg() );
    ui->cmbSwitchCnEn->setCurrentIndex( Settings::get_ceSwitchShortcutkey_shortcutkey() );
}




/*******************************************************************************************
** update_tmp_engish_cmb(),update_short_input_cmb()update_tmp_pinyin_cmb()
** update_custom_shortkey_cmb()这四个函数用到了静态数据成员Settings::singleShortcutKeyName，
** 必须在初始化函数Settings::init_const_data_member()执行完毕后才能调用！否则会出现显示内容为空的现象。
*******************************************************************************************/
//设置界面--更新自定义功能快捷键选择框
void SettingWin::update_custom_shortkey_cmb()
{
    ui->cmbShortcutKey->clear();
//@note update_custom_shortkey_cmb
    for ( int i = 0; i < CSK_NUM; i++ )
    {
        bool isUsed = false;
        for ( int j = 0; j < CSF_NUM; j++ )
        {
            if ( i != CSK_NONE
                 && (j != ui->cmbFunction->currentIndex())
                 && (Settings::get_customShortcutFunc_shortcutkey(static_cast<CustomShortcutFunction>(j)) == i) )
            {
                isUsed = true;
            }
        }

        if ( !isUsed )
        {
            ui->cmbShortcutKey->addItem( Settings::s_combineShortcutKeyName[static_cast<CombineShortcutKey>(i)] );
        }
    }

    for ( int i = 0; i < ui->cmbShortcutKey->count(); i++ )
    {
        if ( ui->cmbShortcutKey->itemText(i)
             == Settings::s_combineShortcutKeyName[Settings::get_customShortcutFunc_shortcutkey(static_cast<CustomShortcutFunction>(ui->cmbFunction->currentIndex()))] )
        {
            ui->cmbShortcutKey->setCurrentIndex( i );
            break;
        }
    }
}



//设置界面--更新临时英文选项框
void SettingWin::update_tmp_engish_cmb()
{
    ui->cmbTmpEnglish->clear();

    for ( int i = 0; i < SSK_NUM; i++ )
    {
        if ( i != SSK_NONE
             && (i == Settings::get_quick_input_shortcutkey() || i == Settings::get_tmp_pinyin_shortcutkey()) )
        {
            continue;
        }

        ui->cmbTmpEnglish->addItem( Settings::s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] );

    }

    for ( int i = 0; i < ui->cmbTmpEnglish->count(); i++ )
    {
        if ( ui->cmbTmpEnglish->itemText(i) == Settings::s_singleShortcutKeyName[Settings::get_tmp_english_shortcutkey()] )
        {
            ui->cmbTmpEnglish->setCurrentIndex( i );
            break;
        }
    }
}

//设置界面--更新快捷输入选项框
void SettingWin::update_short_input_cmb()
{
    ui->cmbShortcutInput->clear();

    for ( int i = 0; i < SSK_NUM; i++ )
    {
        if ( i != SSK_NONE
             && (i == Settings::get_tmp_english_shortcutkey() || i == Settings::get_tmp_pinyin_shortcutkey()) )
        {
            continue;
        }

        ui->cmbShortcutInput->addItem( Settings::s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] );
    }

    for ( int i = 0; i < ui->cmbShortcutInput->count(); i++ )
    {
        if ( ui->cmbShortcutInput->itemText(i) == Settings::s_singleShortcutKeyName[Settings::get_quick_input_shortcutkey()] )
        {
            ui->cmbShortcutInput->setCurrentIndex( i );
            break;
        }
    }
}

//设置界面--更新临时拼音选项框
void SettingWin::update_tmp_pinyin_cmb()
{
    ui->cmbTmpPinyin->clear();

    for ( int i = 0; i < SSK_NUM; i++ )
    {
        if ( i != SSK_NONE
             && (i == Settings::get_tmp_english_shortcutkey() || i == Settings::get_quick_input_shortcutkey()) )
        {
            continue;
        }

        ui->cmbTmpPinyin->addItem( Settings::s_singleShortcutKeyName[static_cast<SingleShortcutKey>(i)] );
    }

    for ( int i = 0; i < ui->cmbTmpPinyin->count(); i++ )
    {
        if ( ui->cmbTmpPinyin->itemText(i) == Settings::s_singleShortcutKeyName[Settings::get_tmp_pinyin_shortcutkey()] )
        {
            ui->cmbTmpPinyin->setCurrentIndex( i );
            break;
        }
    }
}

void SettingWin::init_ui_setting_page()
{
    init_skin_select_cmb();
    ui->ckbAutoLocate->setChecked( Settings::get_toolbar_auto_locate_flg() );
    ui->ckbAutoExtend->setChecked( Settings::get_toolbar_auto_expand_flg() );
    ui->ckbEnableUiAudioEffect->setChecked( Settings::get_ui_audio_effect_flg() );
    ui->ckbDispRealHelp->setChecked( Settings::get_show_realtime_help_flg() );
    ui->ckbHideToolbar->setChecked( Settings::get_hide_toolbar_flg() );
    ui->ckbDispRealHelp->setChecked( Settings::get_show_realtime_help_flg() );
    ui->spbToolbarTransparency->setValue( Settings::get_toolbar_transparency() );
    //ui->labelToolbar->setWindowOpacity( 1 - ui->spbToolbarTransparency->value()/100.0 ); ?无效
}

void SettingWin::init_skin_select_cmb()
{
    ui->cmbSkinSelect->clear();

    QString skinDir = INSTALL_DIR + "/skin/";
    QStringList dirList = QDir(skinDir).entryList( QDir::Dirs );
    dirList.removeOne( "." );
    dirList.removeOne( ".." );

    QStringList skinIdList;
    foreach( QString skinId, dirList )
    {
        if ( QFile( skinDir + skinId + "/skin.ini" ).exists() )
        {
            skinIdList << skinId;
            ui->cmbSkinSelect->addItem( skinId );
        }
    }
    Settings::save_exist_skin( skinIdList );

    for ( int i = 0; i < ui->cmbSkinSelect->count(); i++ )
    {
        if ( ui->cmbSkinSelect->itemText(i) == Settings::get_cur_skin_id() )
        {
            ui->cmbSkinSelect->setCurrentIndex( i );
            break;
        }
    }

    update_toolbar_preview( ui->cmbSkinSelect->currentText() );
}


void SettingWin::update_toolbar_preview( const QString &skinId )
{
    QString iamge = INSTALL_DIR + "/skin/" + skinId + "/toolbar.png";
    ui->labelToolbar->setStyleSheet( QString("border-image:url(%1);").arg(iamge) );
}


void SettingWin::init_candidate_ui_page()
{
    if ( Settings::get_candidate_win_disp_mode() == CWDM_ONE_ROW )
    {
        ui->cmbCandiWinMode->setCurrentIndex( 0 );
    }
    else if ( Settings::get_candidate_win_disp_mode() == CWDM_MULTI_ROW )
    {
        ui->cmbCandiWinMode->setCurrentIndex( 1 );
    }
    ui->ledtSeparateChar->setText( QString(Settings::get_separate_char()) );

    ui->ckbUseGradientBgColor->setChecked( Settings::get_use_gradient_color_flg() );
    ckb_useGradientBgColor_updated( Settings::get_use_gradient_color_flg() );
    ui->ckbUseBgImage->setChecked( Settings::get_use_bg_image_flg() );
    ckb_useBgImage_updated( Settings::get_use_bg_image_flg() );
    ui->ckbUseTile->setChecked( Settings::get_bg_image_tiled_flg() );
    if ( Settings::get_cur_skin_id() == "default" )
    {
        ui->spbCornerRadian->setEnabled( true );
        ui->ckbUseGradientBgColor->setEnabled( true );
        ui->ckbUseBgImage->setEnabled( true );
        if ( Settings::get_use_bg_image_flg() )
        {
            ui->ckbUseTile->setEnabled( true );
        }
    }
    else
    {
        ui->spbCornerRadian->setEnabled( false );
        ui->ckbUseGradientBgColor->setEnabled( false );
        ui->ckbUseBgImage->setEnabled( false );
        ui->ckbUseTile->setEnabled( false );
    }

    ui->spbCornerRadian->setValue( Settings::get_radius() );
    ui->spbCandiTransparency->setValue( Settings::get_candi_win_transparency() );
    ui->spbCandiItemNum->setValue( Settings::get_candidate_word_count() );
    ui->spbCandiCharNum->setValue( Settings::get_candi_char_count() );

    update_fram_candidate_win();
}

void SettingWin::ckb_useGradientBgColor_updated( bool checked )
{
    if ( checked )
    {
        ui->ckbUseBgImage->setChecked( false );
        ui->ckbUseTile->setEnabled( false );
        ui->btnCandiBg->hide();
        ui->btnCandiBgColor0->show();
        ui->btnCandiBgColor1->show();
    }
    else if ( !ui->ckbUseBgImage->isChecked() )
    {
        ui->btnCandiBg->setText( "背景色" );
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::ckb_useBgImage_updated( bool checked )
{
    if ( checked )
    {
        ui->ckbUseGradientBgColor->setChecked( false );
        ui->ckbUseTile->setEnabled( true );
        ui->btnCandiBg->setText( "背景图" );
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
    else  if ( !ui->ckbUseGradientBgColor->isChecked() )
    {
        ui->ckbUseTile->setEnabled( false );
        ui->btnCandiBg->setText( "背景色" );
        ui->btnCandiBg->show();
        ui->btnCandiBgColor0->hide();
        ui->btnCandiBgColor1->hide();
    }
}

void SettingWin::update_fram_candidate_win()
{
    QString style;

    if ( Settings::get_cur_skin_id() == "default" )
    {
        int boederRadius = Settings::get_radius();
        QColor boderColor = Settings::get_border_color();
        QColor bgColor = Settings::get_bg_color();
        QColor gradienColor0 = Settings::get_gradient_color0();
        QColor gradienColor1 = Settings::get_gradient_color1();

        QString borderColorStyle = QString( "border-color:rgb(%1,%2,%3);" )
                                    .arg(boderColor.red()).arg(boderColor.green()).arg(boderColor.blue());

        if ( Settings::get_use_gradient_color_flg() )
        {
            QString gradienColorStyle = QString( "background-color:qlineargradient(spread:pad,x1:0, y1:0, x2:0, y2:1,stop:0 rgb(%1,%2,%3),stop:1 rgb(%4,%5,%6));" )
                                        .arg(gradienColor0.red()).arg(gradienColor0.green()).arg(gradienColor0.blue())
                                        .arg(gradienColor1.red()).arg(gradienColor1.green()).arg(gradienColor1.blue());
            style = QString( "#framCandidateWin{"
                    "border-width:1px;"
                    "border-style:solid;"
                    "border-radius:%1px;"
                    "%2"
                    "%3"
                    "}")
                    .arg( boederRadius )
                    .arg( borderColorStyle )
                    .arg( gradienColorStyle );
        }
        else if ( Settings::get_use_bg_image_flg() )
        {
            QString bgImageStyle = QString( "%1:url(%2);")
                    .arg( Settings::get_bg_image_tiled_flg() ? "background-image" : "border-image" )
                    .arg( Settings::get_bg_image() );

            style = QString( "#framCandidateWin{"
                    "border-width:1px;"
                    "border-style:solid;"
                    "border-radius:%1px;"
                    "%2"
                    "%3"
                    "}")
                    .arg( boederRadius )
                    .arg( borderColorStyle )
                    .arg( bgImageStyle );
        }
        else
        {
            style = QString( "#framCandidateWin{"
                    "border-width:1px;"
                    "border-style:solid;"
                    "border-radius:%1px;"
                    "%2"
                    "background-color:rgb(%3,%4,%5);"
                    "}")
                    .arg( boederRadius )
                    .arg( borderColorStyle )
                    .arg( bgColor.red() ).arg( bgColor.green() ).arg( bgColor.blue() );
        }

        if ( Settings::get_use_gradient_color_flg() )
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

    ui->framCandidateWin->setStyleSheet( style );

    QColor color = Settings::get_candidate_word_text_color();
    ui->btnCandiAutoWord->setStyleSheet( QString("color:rgb(%1,%2,%3);")
                                         .arg(color.red()).arg(color.green()).arg(color.blue()) );

    color = Settings::get_candidate_prompt_text_color();
    ui->btnCandiPrompt->setStyleSheet( QString("color:rgb(%1,%2,%3);")
                                       .arg(color.red()).arg(color.green()).arg(color.blue()) );
}

void SettingWin::init_candidate_option_page()
{
    ui->cmb23RecodeSelect->setCurrentIndex( Settings::get_recode_select_key() );
    ui->cmbPrevNextPage->setCurrentIndex( Settings::get_candi_page_key() );
    //ui->ledt2ndRecode->setText( QChar(Settings::get_second_recode_key()) );
    //ui->ledt3rdRecode->setText( QChar(Settings::get_third_recode_key()) );
    ui->ckbCursorFollow->setChecked( Settings::get_cursor_follow_flg() );
    ui->ckbHideCandiChinese->setChecked( Settings::get_hide_candiWin_flg() );
    ui->ckbDispOpPrompt->setChecked( Settings::get_show_op_remind_info_flg() );
    ui->ckbDispOpDict->setChecked( Settings::get_show_cand_dict() );

    //ui->ledtPrecPage->setText( QChar(Settings::get_prevPage_key()) );
    //ui->ledtNextPage->setText( QChar(Settings::get_nextPage_key()) );
    ui->ckbShiftSelectRecode->setChecked( Settings::get_shift_select_recode_flg() );
}


//切换设置界面右侧选项页
void SettingWin::on_listWidget_currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous )
{
    Q_UNUSED( previous );

    if ( current == m_listItemCommon )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageCommon );
    }
    else if ( current == m_listItemAdvance )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageAdvance );
    }
    else if ( current == m_listItemOthers )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageOthers );
    }
    else if ( current == m_listItemUi )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageUi );
    }
    else if ( current == m_listItemCandidateWinUi )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageCandidateWinUi );
    }
    else if ( current == m_listItemCandidateWinOption )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageCandidateWinOption );
    }
    else if ( current == m_listItemShortcutKey )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageShortcutKey );
    }
    else if ( current == m_listItemCustomKeyChar )
    {
        m_kbCustomKeyChar->update_keyboard_button();//初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget( ui->pageCustomKeyChar );
    }
    else if ( current == m_listItemCustomKeyMark )
    {
        m_kbCustomKeyMark->update_keyboard_button();//初始化虚拟键盘按钮上显示的自定义符号
        ui->stackedWidget->setCurrentWidget( ui->pageCustomKeyMark );
    }
    else if ( current == m_listItemVersionInfo )
    {
        ui->stackedWidget->setCurrentWidget( ui->pageVersionInfo );
    }
    else if ( current == m_listItemBug )
    {
        //QDesktopServices::openUrl( QUrl("http://www.freewb.org") );
    }
}

void SettingWin::on_btnSettingOption_clicked()
{
    Settings::switch_group_mode();
    if ( Settings::is_show_all_group() )
    {
        ui->btnSettingOption->setText( "显示【常用】选项" );
    }
    else
    {
        ui->btnSettingOption->setText( "显示【所有】选项" );
    }

    update_listwidget_item();
}

void SettingWin::on_btnOk_clicked()
{
    
    close();
    move( m_defaultPopPosition );
puts("win seting");
    Settings::save_all_setting_data_to_file();
}

void SettingWin::on_btnCancel_clicked()
{
    close();
    move( m_defaultPopPosition );
}

void SettingWin::on_btnHelp_clicked()
{
    system( "firefox ~/.local/freewb/help/help.html > /dev/null 2>&1 &" );
}


void SettingWin::on_ckbCodeRemind_stateChanged( int arg1 )
{
//    qDebug() << DBG_TRACE << arg1;

    if ( arg1 == Qt::Checked )
    {
        Settings::set_codeRemind_flg( true );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_codeRemind_flg( false );
    }
}



void SettingWin::on_ckbSpaceFullWhenCharHalf_toggled( bool checked )
{
    Settings::set_spaceFullWhenCharHalf_flg( checked );
}

void SettingWin::on_ckbWordThink_toggled( bool checked )
{
    Settings::set_wordThink_flg( checked );
}

void SettingWin::on_ckbSmartMark_stateChanged( int arg1 )
{
    if ( arg1 == Qt::Checked )
    {
        Settings::set_smartMark_flg( true );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_smartMark_flg( false );
    }
}


void SettingWin::on_ckbRemindExistWord_toggled( bool checked )
{
    Settings::set_remindExistWord_flg( checked );
}


void SettingWin::on_ckbAlertWhenEmptyCode_toggled(bool checked)
{
    Settings::set_alertWhenEmptyCode_flg( checked );
}

void SettingWin::on_ckbUseAudioFile_stateChanged( int arg1 )
{
    if ( arg1 == Qt::Checked )
    {
        Settings::set_useAudioFile_flg( true );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_useAudioFile_flg( false );
    }
}

void SettingWin::on_ckbAutoAdjustFreq_toggled( bool checked )
{
    Settings::set_autoAdjustFreq_flg( checked );
}





void SettingWin::on_ckbShiftCommitChar_toggled( bool checked )
{
    Settings::set_shiftCommitChar_flg( checked );
}

void SettingWin::on_ckbInputStatistic_toggled( bool checked )
{
    Settings::set_inputStatistic_flg( checked );
}

void SettingWin::on_ckbRepeatCalib_toggled( bool checked )
{
    Settings::set_recodeCalib_flg( checked );
}


void SettingWin::on_ckbTypeEffect_toggled( bool checked )
{
    Settings::set_typeEffect_flg( checked );
}


void SettingWin::on_ckbAutoWordGroup_activated(int index)
{
    if ( index == 0 )
    {
        Settings::set_autoWordGroupOpt_flg( AWGO_LOSS );
    }
    else if ( index == 1 )
    {
        Settings::set_autoWordGroupOpt_flg( AWGO_FORBID );
    }
    else if ( index == 2 )
    {
        Settings::set_autoWordGroupOpt_flg( AWGO_SAVE );
    }
}





void SettingWin::on_ledtAutoToEnStr_textChanged( const QString &arg1 )
{
    Settings::set_autoToEn_str( arg1 );
}

void SettingWin::on_ledtAutoToHalf_textChanged( const QString &arg1 )
{
    Settings::set_autoToHalf_mark( arg1 );
}

void SettingWin::on_ckbAutoHalfMarkAfterNum_toggled( bool checked )
{
    Settings::set_autoToHalfMark_flg( checked );
}



void SettingWin::on_cmbFunction_activated( int index )
{
    Q_UNUSED(index)
    update_custom_shortkey_cmb();
}

void SettingWin::on_cmbShortcutKey_activated( int index )
{
    for ( int i = 0; i < CSK_NUM; i++ )
    {
        if ( ui->cmbShortcutKey->itemText(index) == Settings::s_combineShortcutKeyName[static_cast<CombineShortcutKey>(i)] )
        {
            Settings::set_customShortcutFunc_shortcutkey( static_cast<CustomShortcutFunction>(ui->cmbFunction->currentIndex()),
                                                          static_cast<CombineShortcutKey>(i) );
        }
    }
}

void SettingWin::on_ckbDisableAllShortcutKey_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked )
    {
        Settings::set_disableAllCsk_flg( true );
        ui->cmbFunction->setEnabled( false );
        ui->cmbShortcutKey->setEnabled( false );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_disableAllCsk_flg( false );
        ui->cmbFunction->setEnabled( true );
        ui->cmbShortcutKey->setEnabled( true );
    }
}

void SettingWin::on_ckbDisableFullHalfKey_toggled( bool checked )
{
    Settings::set_disableFullHalfSwitchShortcutkey_flg( checked );
}

void SettingWin::on_cmbSwitchCnEn_activated(int index)
{
    RcodeSelectShortcutKey rssk = Settings::get_recode_select_key();
    CnEnSwitchShortcutKey key = static_cast<CnEnSwitchShortcutKey>(index);
    if ( (rssk == RSSK_CTRL && (key == CESSK_CTRL || key == CESSK_LEFT_CTRL || key == CESSK_RIGHT_CTRL))
        || (rssk == RSSK_SHIFT && (key == CESSK_SHIFT || key == CESSK_LEFT_SHIFT || key == CESSK_RIGHT_SHIFT)) )
    {
        m_msgBox = new QMessageBox( this );
        //m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon( QMessageBox::Warning );
        m_msgBox->setText( "您设置的快捷键将与二三重码选择键冲突，确认设置？" );
        m_msgBox->setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        m_msgBox->button( QMessageBox::Yes )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::Yes )->setText( "是(&Y)" );
        m_msgBox->button( QMessageBox::No )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::No )->setText( "否(&N)" );
        m_msgBox->setDefaultButton( QMessageBox::No );
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if ( ret == QMessageBox::No )
        {
            ui->cmbSwitchCnEn->setCurrentIndex( Settings::get_ceSwitchShortcutkey_shortcutkey() );
            return;
        }
    }

    Settings::set_ceSwitchShortcutkey_shortcutkey( key );
}


void SettingWin::on_cmbTmpEnglish_activated( const QString &arg1 )
{
//    qDebug() << DBG_TRACE << arg1;
    Settings::set_tmp_english_shortcutkey( arg1 );

    update_short_input_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbShortcutInput_activated(const QString &arg1)
{
//    qDebug() << DBG_TRACE << arg1;
    Settings::set_shortcut_input_shortcutkey( arg1 );

    update_tmp_engish_cmb();
    update_tmp_pinyin_cmb();
}

void SettingWin::on_cmbTmpPinyin_activated(const QString &arg1)
{
//    qDebug() << DBG_TRACE << arg1;
    Settings::set_tmp_pinyin_shortcutkey( arg1 );

    update_tmp_engish_cmb();
    update_short_input_cmb();
}




void SettingWin::on_btnRestoreShortcutKey_clicked()
{
    m_msgBox = new QMessageBox( this );
    //m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
    m_msgBox->setIcon( QMessageBox::Warning );
    m_msgBox->setText("确定将快捷键都恢复成默认键值吗？");
    m_msgBox->setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    m_msgBox->button( QMessageBox::Yes )->setIcon( QIcon() );
    m_msgBox->button( QMessageBox::Yes )->setText( "是(&Y)" );
    m_msgBox->button( QMessageBox::No )->setIcon( QIcon() );
    m_msgBox->button( QMessageBox::No )->setText( "否(&N)" );
    m_msgBox->setDefaultButton( QMessageBox::Yes );

    int ret = m_msgBox->exec();
    delete m_msgBox;
    if ( ret == QMessageBox::Yes )
    {
        Settings::restore_default_shortcutkey();
        init_shortcutkey_page();
    }
}



void SettingWin::slot_custom_keyboard_char_clicked( SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue )
{
    //qDebug() << keyValue.commChar;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle("设置键盘字符");
    m_customKeyDialog->set_custom_symbol( VKM_CUSTOM_CHAR, keyName, keyValue.commChar, keyValue.shiftChar );
    m_customKeyDialog->exec();
}


void SettingWin::slot_custom_keyboard_mark_clicked( SymbolKeyIdx keyIdx, const QString &keyName, const CustomKeyValue &keyValue )
{
    //qDebug() << keyValue.commMark;

    m_curSymbolKeyIdx = keyIdx;
    m_curCustomKeyValue = keyValue;

    m_customKeyDialog->setWindowTitle("设置键盘标点");
    m_customKeyDialog->set_custom_symbol( VKM_CUSTOM_MARK, keyName, keyValue.commMark, keyValue.shiftMark );
    m_customKeyDialog->exec();
}


void SettingWin::slot_custom_btn_ok_clicked( const QString &commSymbol, const QString &shiftSymbol )
{
    //qDebug() << commSymbol << shiftSymbol;

    if ( ui->stackedWidget->currentWidget() == ui->pageCustomKeyChar )
    {
        m_curCustomKeyValue.commChar = commSymbol;
        m_curCustomKeyValue.shiftChar = shiftSymbol;
        m_kbCustomKeyChar->update_customkey_button( m_curSymbolKeyIdx, commSymbol, shiftSymbol );
    }
    else
    {
        m_curCustomKeyValue.commMark = commSymbol;
        m_curCustomKeyValue.shiftMark = shiftSymbol;
        m_kbCustomKeyMark->update_customkey_button( m_curSymbolKeyIdx, commSymbol, shiftSymbol );
    }
#ifdef DEBUG
    qDebug() << m_curCustomKeyValue.commChar << m_curCustomKeyValue.shiftChar
             << m_curCustomKeyValue.commMark << m_curCustomKeyValue.shiftMark;
#endif
    Settings::set_custom_key_info( m_curSymbolKeyIdx, m_curCustomKeyValue );
}





void SettingWin::on_cmbSkinSelect_activated( const QString &arg1 )
{
    Settings::set_cur_skin_id( arg1 );
    update_toolbar_preview( arg1 );
    init_candidate_ui_page();
}

void SettingWin::on_ckbAutoLocate_toggled( bool checked )
{
    Settings::set_toolbar_auto_locate_flg( checked );
}

void SettingWin::on_ckbAutoExtend_toggled( bool checked )
{
    Settings::set_toolbar_auto_expand_flg( checked );
}

void SettingWin::on_ckbEnableUiAudioEffect_toggled( bool checked )
{
    Settings::set_ui_audio_effect_flg( checked );
}

void SettingWin::on_ckbDispRealHelp_toggled( bool checked )
{
    Settings::set_show_realtime_help_flg( checked );
}

void SettingWin::on_ckbHideToolbar_toggled( bool checked )
{
    Settings::set_hide_toolbar_flg( checked );
}

void SettingWin::on_spbToolbarTransparency_valueChanged( int arg1 )
{
    Settings::set_toolbar_transparency( arg1 );
    //ui->labelToolbar->setWindowOpacity( 1 - arg1/100.0 );
}






void SettingWin::on_cmbCandiWinMode_activated( int index )
{
    if ( index == 0 )
    {
        Settings::set_candidate_win_disp_mode( CWDM_ONE_ROW );
    }
    else if ( index == 1 )
    {
        Settings::set_candidate_win_disp_mode( CWDM_MULTI_ROW );
    }
}



void SettingWin::on_ledtSeparateChar_textChanged( const QString &arg1 )
{
    if ( !arg1.length() )
    {
        Settings::set_separate_char( 0 );
    }
    else
    {
        Settings::set_separate_char( arg1.toLatin1().at(0) );
    }
}

void SettingWin::on_ckbUseGradientBgColor_toggled( bool checked )
{
    Settings::set_use_gradient_color_flg( checked );
    if ( checked )
    {
        Settings::set_use_bg_image_flg( false );
    }
    ckb_useGradientBgColor_updated( checked );
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseBgImage_toggled( bool checked )
{
    Settings::set_use_bg_image_flg( checked );
    if ( checked )
    {
        Settings::set_use_gradient_color_flg( false );
    }
    ckb_useBgImage_updated( checked );
    update_fram_candidate_win();
}

void SettingWin::on_ckbUseTile_toggled( bool checked )
{
    Settings::set_bg_image_tiled_flg( checked );
    update_fram_candidate_win();
}


//通过setValue函数设置值时也会出发该槽函数!!!!!!!!
void SettingWin::on_spbCandiItemNum_valueChanged( int arg1 )
{
    Settings::set_candidate_word_count( arg1 );
}


void SettingWin::on_spbCornerRadian_valueChanged(int arg1)
{
    Settings::set_radius( arg1 );
    update_fram_candidate_win();
}

void SettingWin::on_spbCandiTransparency_valueChanged(int arg1)
{
    Settings::set_candi_win_transparency( arg1 );
}

void SettingWin::on_spbCandiCharNum_valueChanged(int arg1)
{
    Settings::set_candi_char_count( arg1 );
}



void SettingWin::on_btnCandiFont_clicked()
{
    QFontDialog dialog(this);

    dialog.setCurrentFont( Settings::get_candidate_text_font() );
    if( dialog.exec() == QFontDialog::Accepted )
    {
        Settings::set_candidate_text_font( dialog.selectedFont() );
        //ui->btnCandiFont->setFont( dialog.selectedFont() );
    }
}

void SettingWin::on_btnCandiBg_clicked()
{
    if ( Settings::get_use_bg_image_flg() )
    {
        QString file = QFileDialog::getOpenFileName( this, "选择背景图片", qgetenv("HOME"), "Images(*.png *.bmp *.jpg)" );
        Settings::set_bg_image( file ) ;
        update_fram_candidate_win();
    }
    else
    {
        QColorDialog dialog(this);

        dialog.setCurrentColor( Settings::get_bg_color() );
        if( dialog.exec() == QColorDialog::Accepted )
        {
            Settings::set_bg_color( dialog.selectedColor() );
            update_fram_candidate_win();
        }
    }
}

void SettingWin::on_btnCandiBgColor0_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor( Settings::get_gradient_color0() );
    if( dialog.exec() == QColorDialog::Accepted )
    {
        Settings::set_gradient_color0( dialog.selectedColor() );
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBgColor1_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor( Settings::get_gradient_color1() );
    if( dialog.exec() == QColorDialog::Accepted )
    {
        Settings::set_gradient_color1( dialog.selectedColor() );
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiBorderColor_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor( Settings::get_border_color() );
    if( dialog.exec() == QColorDialog::Accepted )
    {
        Settings::set_border_color( dialog.selectedColor() );
        update_fram_candidate_win();
    }
}

void SettingWin::on_btnCandiAutoWord_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor( Settings::get_candidate_word_text_color() );
    if( dialog.exec() == QColorDialog::Accepted )
    {
        QColor color = dialog.selectedColor();
        Settings::set_candidate_word_text_color( color );
        ui->btnCandiAutoWord->setStyleSheet( QString("color:rgb(%1,%2,%3);")
                                             .arg(color.red()).arg(color.green()).arg(color.blue()) );
    }
}

void SettingWin::on_btnCandiPrompt_clicked()
{
    QColorDialog dialog(this);

    dialog.setCurrentColor( Settings::get_candidate_prompt_text_color() );
    if( dialog.exec() == QColorDialog::Accepted )
    {
        QColor color = dialog.selectedColor();
        Settings::set_candidate_prompt_text_color( color );
        ui->btnCandiPrompt->setStyleSheet( QString("color:rgb(%1,%2,%3);")
                                             .arg(color.red()).arg(color.green()).arg(color.blue()) );
    }
}





/******************************** 候选窗选项 ********************************/
//void SettingWin::on_ledt2ndRecode_textChanged( const QString &arg1 )
//{
//    if ( !arg1.isEmpty() )
//    {
//        Settings::set_second_recode_key( arg1.at(0).toLatin1() );
//    }
//}

//void SettingWin::on_ledt3rdRecode_textChanged( const QString &arg1 )
//{
//    if ( !arg1.isEmpty() )
//    {
//        Settings::set_third_recode_key( arg1.at(0).toLatin1() );
//    }
//}


void SettingWin::on_ckbShiftSelectRecode_toggled( bool checked )
{
    Settings::set_shift_select_recode_flg( checked );
}

void SettingWin::on_ckbCursorFollow_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked )
    {
        Settings::set_candiWin_follow_flg( true );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_candiWin_follow_flg( false );
    }
}

void SettingWin::on_ckbHideCandiChinese_stateChanged(int arg1)
{
    if ( arg1 == Qt::Checked )
    {
        Settings::set_hide_candiWin_flg( true );
    }
    else if ( arg1 == Qt::Unchecked )
    {
        Settings::set_hide_candiWin_flg( false );
    }
}


void SettingWin::on_ckbDispOpPrompt_toggled( bool checked )
{
    Settings::set_show_op_remind_info_flg( checked );
}


void SettingWin::on_ckbDispOpDict_toggled( bool checked )
{
    //printf("dict=%d\n",checked);
    Settings::set_show_cand_dict( checked );
}


void SettingWin::on_cmb23RecodeSelect_activated( int index )
{
    QString conflictInfo;
    int conflictFlg = 0;
    CnEnSwitchShortcutKey cnEnSwitchShortcutKey = Settings::get_ceSwitchShortcutkey_shortcutkey();

    RcodeSelectShortcutKey key = static_cast<RcodeSelectShortcutKey>(index);
    if ( key == RSSK_COMMA_PERIOD && Settings::get_candi_page_key() == CPSK_COMMA_PERIOD )
    {
        conflictFlg = 1;
        conflictInfo = "您设置的快捷键将与上下翻页键冲突，确认设置？";
    }
    else if ( (key == RSSK_CTRL && (cnEnSwitchShortcutKey == CESSK_CTRL || cnEnSwitchShortcutKey == CESSK_LEFT_CTRL || cnEnSwitchShortcutKey == CESSK_RIGHT_CTRL))
              || (key == RSSK_SHIFT && (cnEnSwitchShortcutKey == CESSK_SHIFT || cnEnSwitchShortcutKey == CESSK_LEFT_SHIFT || cnEnSwitchShortcutKey == CESSK_RIGHT_SHIFT)) )
    {
        conflictFlg = 1;
        conflictInfo = "您设置的快捷键将与中英文切换键冲突，确认设置？";
    }

    if ( conflictFlg )
    {
        m_msgBox = new QMessageBox( this );
        //m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon( QMessageBox::Warning );
        m_msgBox->setText( conflictInfo );
        m_msgBox->setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        m_msgBox->button( QMessageBox::Yes )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::Yes )->setText( "是(&Y)" );
        m_msgBox->button( QMessageBox::No )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::No )->setText( "否(&N)" );
        m_msgBox->setDefaultButton( QMessageBox::No );
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if ( ret == QMessageBox::No )
        {
            ui->cmb23RecodeSelect->setCurrentIndex( Settings::get_recode_select_key() );
            return;
        }
    }

    Settings::set_recode_select_key( key );
}

void SettingWin::on_cmbPrevNextPage_activated( int index )
{
    CandiPageShortcutKey key = static_cast<CandiPageShortcutKey>(index);
    if ( key == CPSK_COMMA_PERIOD && Settings::get_recode_select_key() == RSSK_COMMA_PERIOD )
    {
        m_msgBox = new QMessageBox( this );
        //m_msgBox->setWindowFlag( Qt::FramelessWindowHint );
        m_msgBox->setIcon( QMessageBox::Warning );
        m_msgBox->setText( "您设置的快捷键将与二三重码选择键冲突，确认设置？" );
        m_msgBox->setStandardButtons( QMessageBox::Yes | QMessageBox::No );
        m_msgBox->button( QMessageBox::Yes )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::Yes )->setText( "是(&Y)" );
        m_msgBox->button( QMessageBox::No )->setIcon( QIcon() );
        m_msgBox->button( QMessageBox::No )->setText( "否(&N)" );
        m_msgBox->setDefaultButton( QMessageBox::No );
        int ret = m_msgBox->exec();
        delete m_msgBox;
        if ( ret == QMessageBox::No )
        {
            ui->cmbPrevNextPage->setCurrentIndex( Settings::get_candi_page_key() );
            return;
        }
    }

    Settings::set_candi_page_key( key );
}

