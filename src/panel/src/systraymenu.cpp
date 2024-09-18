#include "systraymenu.h"
#include "commdefine.h"
#include "settings.h"
#include "toolbarwin.h"

#define ICO_CHECKED     ":/image/toolbar/checked.png"

#define ICO_FREEWB      ":/image/traymenu/freewb.png"
#define ICO_WUBI        ":/image/traymenu/wubi.png"
#define ICO_WBPY        ":/image/traymenu/wbpy.png"
#define ICO_PINYIN      ":/image/traymenu/pinyin.png"
#define ICO_ENGLISH     ":/image/traymenu/english.png"

#define ICO_FULL_WIDTH  ":/image/traymenu/full.png"
#define ICO_HALF_WIDTH  ":/image/traymenu/half.png"
#define ICO_MARK_CN     ":/image/traymenu/mark_cn.png"
#define ICO_MARK_EN     ":/image/traymenu/mark_en.png"
#define ICO_SIMP        ":/image/traymenu/simp.png"
#define ICO_TRAD        ":/image/traymenu/trad.png"
#define ICO_VK          ":/image/traymenu/vk.png"
#define ICO_SETTING     ":/image/traymenu/setting.png"

#define QSS_MENU \
"QMenu::item{color: rgb(56, 56, 56);}"\
"QMenu::item:selected{background-color:#DFDFDF;color: rgb(56, 56, 56)}"\


bool SysTrayMenu::s_externImFlg =  true;

bool SysTrayMenu::is_extern_im()
{
    return s_externImFlg;
}

void SysTrayMenu::set_extern_im( bool flg )
{
    s_externImFlg = flg;
}

SysTrayMenu::SysTrayMenu( QObject *parent ) : QSystemTrayIcon( parent )
{  
    setToolTip( "极点五笔" );
    setIcon( QIcon(ICO_FREEWB) );
    m_curDispIm = IM_OTHER;

    m_panelIsCreated = false;

    m_actCharWidth = new QAction( QIcon(ICO_FULL_WIDTH), "全/半角", this );
    m_actMarkMode = new QAction( QIcon(ICO_MARK_CN), "中/英标点", this );
    m_actCharFont = new QAction( QIcon(ICO_SIMP), "简/繁体", this );
    m_actGrpChar = new QActionGroup( this );
    m_actGrpChar->addAction( m_actCharWidth );
    m_actGrpChar->addAction( m_actMarkMode );
    m_actGrpChar->addAction( m_actCharFont );

    m_actVkSwitch = new QAction( QIcon(ICO_VK), "虚拟键盘开关", this );
    m_actVkPcMode = new QAction( "PC键盘", this );
    m_actVkGreek = new QAction( "希腊字母", this );
    m_actVkRussian = new QAction( "俄文字母", this );
    m_actVkPhonetic = new QAction( "注音符号", this );
    m_actVkPinyin = new QAction( "汉语拼音", this );
    m_actVkJapanFlat = new QAction( "日文平假名", this );
    m_actVkJapanPiece = new QAction( "日文片假名", this );
    m_actVkPunctuation = new QAction( "标点符号", this );
    m_actVkDigitalOrder = new QAction( "数字序号", this );
    m_actVkMath = new QAction( "数学符号", this );
    m_actVkUnit = new QAction( "单位符号", this );
    m_actVkTabs = new QAction( "制表符号", this );
    m_actVkSpecial = new QAction( "特殊符号", this );
    m_actVkCustomCharMode = new QAction( "用户符号", this );

    m_actGrpVk = new QActionGroup( this );
    m_actGrpVk->addAction( m_actVkPcMode );
    m_actGrpVk->addAction( m_actVkGreek );
    m_actGrpVk->addAction( m_actVkRussian );
    m_actGrpVk->addAction( m_actVkPhonetic );
    m_actGrpVk->addAction( m_actVkPinyin );
    m_actGrpVk->addAction( m_actVkJapanFlat );
    m_actGrpVk->addAction( m_actVkJapanPiece );
    m_actGrpVk->addAction( m_actVkPunctuation );
    m_actGrpVk->addAction( m_actVkDigitalOrder );
    m_actGrpVk->addAction( m_actVkMath );
    m_actGrpVk->addAction( m_actVkUnit );
    m_actGrpVk->addAction( m_actVkTabs );
    m_actGrpVk->addAction( m_actVkSpecial );
    m_actGrpVk->addAction( m_actVkCustomCharMode );
    m_menuVkMode = new QMenu( "虚拟键盘模式" );
    m_menuVkMode->setStyleSheet( QSS_MENU );
    m_menuVkMode->addActions( m_actGrpVk->actions() );

    update_vk_mode_ckecked_state( VKM_INPUT_PC );

    m_menuSkin = new QMenu( "皮肤" );
    m_menuSkin->setStyleSheet( QSS_MENU );
    m_actGrpSkin = new QActionGroup( this );
    slot_update_skin_list();

    m_actSetting = new QAction( QIcon(ICO_SETTING), "极点设置", this );
    m_actFcitx = new QAction( "Fcitx设置", this );

    m_mainMenu = new QMenu;
    m_mainMenu->setStyleSheet( QSS_MENU );
    m_mainMenu->addActions( m_actGrpChar->actions() );
    m_mainMenu->addSeparator();
    m_mainMenu->addAction( m_actVkSwitch );
    m_mainMenu->addMenu( m_menuVkMode );
    m_mainMenu->addSeparator();
    m_mainMenu->addMenu( m_menuSkin );
    m_mainMenu->addAction( m_actSetting );
    m_mainMenu->addSeparator();
    m_mainMenu->addAction( m_actFcitx );

    setContextMenu( m_mainMenu );

    connect( m_actGrpChar, &QActionGroup::triggered, this, &SysTrayMenu::slot_char_switch_triggered );
    connect( m_actGrpVk, &QActionGroup::triggered, this, &SysTrayMenu::slot_vk_mode_triggered );
    connect( m_actGrpSkin, &QActionGroup::triggered, this, &SysTrayMenu::slot_skin_switch_triggered );
    connect( m_actVkSwitch, &QAction::triggered, this, &SysTrayMenu::slot_vk_switch_triggered );
    connect( m_actSetting, &QAction::triggered, this, &SysTrayMenu::slot_setting_triggered );
    connect( m_actFcitx, &QAction::triggered, this, &SysTrayMenu::slot_fcitx_config_triggered );

    if ( g_desktopType == DT_UBUNTU )//Ubuntu
    {
        connect( m_mainMenu, &QMenu::aboutToShow, this, &SysTrayMenu::slot_update_skin_list );
    }
    else
    {
        connect( m_menuSkin, &QMenu::aboutToShow, this, &SysTrayMenu::slot_update_skin_list );
    }

    connect( this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(slot_systemtray_actived(QSystemTrayIcon::ActivationReason)));
}

SysTrayMenu::~SysTrayMenu()
{
    if ( m_mainMenu )
    {
        delete m_mainMenu;
    }
}


void SysTrayMenu::update_vk_mode_ckecked_state( VirtualKeyboardMode vkMode )
{
    foreach ( QAction *act, m_actGrpVk->actions() )
    {
        if ( (vkMode == VKM_INPUT_PC && act == m_actVkPcMode)
             || (vkMode == VKM_INPUT_GREEK && act == m_actVkGreek )
             || (vkMode == VKM_INPUT_RUSSIAN && act == m_actVkRussian )
             || (vkMode == VKM_INPUT_PHONETIC && act == m_actVkPhonetic )
             || (vkMode == VKM_INPUT_PINYIN && act == m_actVkPinyin )
             || (vkMode == VKM_INPUT_JAPAN_FLAT && act == m_actVkJapanFlat )
             || (vkMode == VKM_INPUT_JAPAN_PIECE && act == m_actVkJapanPiece )
             || (vkMode == VKM_INPUT_PUNCTUATION && act == m_actVkPunctuation )
             || (vkMode == VKM_INPUT_DIGITAL_ORDER && act == m_actVkDigitalOrder )
             || (vkMode == VKM_INPUT_MATH && act == m_actVkMath )
             || (vkMode == VKM_INPUT_UNIT && act == m_actVkUnit )
             || (vkMode == VKM_INPUT_TABS && act == m_actVkTabs )
             || (vkMode == VKM_INPUT_SPECIAL && act == m_actVkSpecial )
             || (vkMode == VKM_INPUT_USER_CHAR && act == m_actVkCustomCharMode) )
        {
            act->setIcon( QIcon(ICO_CHECKED) );
        }
        else
        {
            act->setIcon( QIcon() );
        }
    }
}

void SysTrayMenu::slot_systemtray_actived( QSystemTrayIcon::ActivationReason reason )
{
    Q_UNUSED( reason );

    //emit signal_fcitx_get_all_ims( "/Fcitx/im" );

    //麒麟系统需要执行此操作才能在鼠标左键点击后弹出托盘菜单
    if ( g_desktopType != DT_UBUNTU )
    {
        contextMenu()->exec( QCursor::pos() );
    }
}

//prop: 切换输入法时提示的输入法注册的相关属性信息
void SysTrayMenu::slot_kim_RegisterProperties( const QStringList &prop )
{
    #ifdef DEBUG
    qDebug() <<"SysTrayMenu::slot_kim_RegisterProperties::"<< prop;
    #endif
    foreach( QString param, prop )
    {
        if ( param.contains("/Fcitx/im:") )
        {
            fcitx_inputmethod_updated( param );
        }
        else if ( param.contains("/Fcitx/chttrans:") )
        {
            fcitx_charFont_updated( param );
        }
        else if ( param.contains("/Fcitx/fullwidth:") )
        {
            fcitx_charWidth_updated( param );
        }
        else if ( param.contains("/Fcitx/punc:") )
        {
            fcitx_charMark_updated( param );
        }
    }
}


void SysTrayMenu::slot_kim_ExecMenu( const QStringList &prop )
{
    Q_UNUSED( prop );
    // /Fcitx/im/googlepinyin:Google 拼音:fcitx-googlepinyin:Google 拼音"
    //qDebug() << prop;
//    m_fcitxImMap.clear();
//    foreach ( QString im, prop )
//    {
//        im.remove( "/Fcitx/im/" );
//        QStringList list = im.split( ":" );
//        if ( list.length() >= 4 )
//        {
//            FcitxImInfo inInfo;
//            inInfo.imId = list.at( 0 );
//            inInfo.imName = list.at( 1 );
//            inInfo.imIconName = list.at( 2 );
//            inInfo.imTips = list.at( 3 );
//            m_fcitxImMap.insert( inInfo.imName, inInfo );
//        }
//    }
    //qDebug() << m_fcitxImMap.keys();
}

//prop: 切换输入法时提示的输入法本身描述信息
void SysTrayMenu::slot_kim_UpdateProperty( const QString &prop )
{
    qDebug() <<"slot_kim_UpdateProperty"<< prop;
    if ( prop.contains("/Fcitx/im:") )
    {
        fcitx_inputmethod_updated( prop );
    }
}



void SysTrayMenu::update_char_width_mode_ico( CharWidthMode charWidth )
{
    if ( charWidth == WIDTH_FULL )
    {
        m_actCharWidth->setIcon( QIcon(ICO_FULL_WIDTH) );
    }
    else if ( charWidth == WIDTH_HALF )
    {
        m_actCharWidth->setIcon( QIcon(ICO_HALF_WIDTH) );
    }
}

void SysTrayMenu::update_mark_mode_ico( MarkMode markMode )
{
    if ( markMode == MARK_EN )
    {
        m_actMarkMode->setIcon( QIcon(ICO_MARK_EN) );
    }
    else if ( markMode == MARK_CN )
    {
        m_actMarkMode->setIcon( QIcon(ICO_MARK_CN) );
    }
}

void SysTrayMenu::fcitx_inputmethod_updated( const QString &param )
{
    Q_UNUSED(param)
}


void SysTrayMenu::slot_update_input_mode_ico()
{
    InputMode im = ToolbarWin::get_input_mode();

    //重复显示同一个图标在ubuntu16.04上会有问题
    if ( m_curDispIm == im ) return;
    m_curDispIm = im;
    //////////////////////////////////////

    if ( im == IM_WUBI_FONT )
    {
        setIcon( QIcon(ICO_WUBI) );
    }
    else if ( im == IM_WUBI_PINYIN )
    {
        setIcon( QIcon(ICO_WBPY) );
    }
    else if ( im == IM_STD_PINYIN )
    {
        setIcon( QIcon(ICO_PINYIN) );
    }
    else if ( im == IM_ENGLISH )
    {
        //setIcon( QIcon::fromTheme("fcitx-kbd") );
        setIcon( QIcon(ICO_ENGLISH) );
    }

    if ( !icon().isNull() && !isVisible() && g_desktopType != DT_DEEPIN )
    {
        show();
    }
}

void SysTrayMenu::slot_update_char_font_ico( CharFontMode mode )
{
    if ( mode == CHAR_SIMPLIFIED )
    {
        m_actCharFont->setIcon( QIcon(ICO_SIMP) );
    }
    else
    {
        m_actCharFont->setIcon( QIcon(ICO_TRAD) );
    }
}


void SysTrayMenu::fcitx_charFont_updated( const QString &param )
{
//    qDebug() << param;

    if ( param.contains("fcitx-chttrans-inactive") )
    {
        m_actCharFont->setIcon( QIcon(ICO_SIMP) );
    }
    else if ( param.contains("fcitx-chttrans-active") )
    {
        m_actCharFont->setIcon( QIcon(ICO_TRAD) );
    }
}


void SysTrayMenu::fcitx_charWidth_updated( const QString &param )
{
//    qDebug() << param;

    if ( param.contains("fcitx-fullwidth-active") )
    {
        m_actCharWidth->setIcon( QIcon(ICO_FULL_WIDTH) );
    }
    else if ( param.contains("fcitx-fullwidth-inactive") )
    {
        m_actCharWidth->setIcon( QIcon(ICO_HALF_WIDTH) );
    }
}


void SysTrayMenu::fcitx_charMark_updated( const QString &param )
{
//    qDebug() << param;

    if ( param.contains("fcitx-punc-inactive") )
    {
        m_actMarkMode->setIcon( QIcon(ICO_MARK_EN) );
    }
    else if ( param.contains("fcitx-punc-active") )
    {
        m_actMarkMode->setIcon( QIcon(ICO_MARK_CN) );
    }
}


void SysTrayMenu::slot_char_switch_triggered( QAction *action )
{
    if ( ToolbarWin::get_input_mode() == IM_ENGLISH ) return;

    if ( action == m_actCharWidth )
    {
        emit signal_fcitx_switch_char_width( "/Fcitx/fullwidth" );
    }
    else if ( action == m_actMarkMode )
    {
        emit signal_fcitx_switch_mark( "/Fcitx/punc" );
    }
    else if ( action == m_actCharFont )
    {
        if ( ToolbarWin::get_char_font_mode() == CHAR_SIMPLIFIED )
        {
            slot_update_char_font_ico( CHAR_TRADITIONAL );
            emit signal_switch_char_font( CHAR_TRADITIONAL );
        }
        else
        {
            slot_update_char_font_ico( CHAR_SIMPLIFIED );
            emit signal_switch_char_font( CHAR_SIMPLIFIED );
        }
        //emit signal_fcitx_switch_char_font( "/Fcitx/chttrans" );
    }
}

void SysTrayMenu::slot_vk_switch_triggered()
{
    emit signal_vk_toggle();
}



void SysTrayMenu::slot_vk_mode_changed( VirtualKeyboardMode vkMode )
{
    update_vk_mode_ckecked_state( vkMode );
}



void SysTrayMenu::slot_vk_mode_triggered( QAction *action )
{
    VirtualKeyboardMode vkMode = VKM_INPUT_PC ;

    if ( action == m_actVkPcMode  )
    {
        vkMode = VKM_INPUT_PC;
    }
    else if ( action == m_actVkGreek )
    {
        vkMode = VKM_INPUT_GREEK;
    }
    else if ( action == m_actVkRussian )
    {
        vkMode = VKM_INPUT_RUSSIAN;
    }
    else if ( action == m_actVkPhonetic )
    {
        vkMode = VKM_INPUT_PHONETIC;
    }
    else if ( action == m_actVkPinyin )
    {
        vkMode = VKM_INPUT_PINYIN;
    }
    else if ( action == m_actVkJapanFlat )
    {
        vkMode = VKM_INPUT_JAPAN_FLAT;
    }
    else if ( action == m_actVkJapanPiece )
    {
        vkMode = VKM_INPUT_JAPAN_PIECE;
    }
    else if ( action == m_actVkPunctuation )
    {
        vkMode = VKM_INPUT_PUNCTUATION;
    }
    else if ( action == m_actVkDigitalOrder )
    {
        vkMode = VKM_INPUT_DIGITAL_ORDER;
    }
    else if ( action == m_actVkMath )
    {
        vkMode = VKM_INPUT_MATH;
    }
    else if ( action == m_actVkUnit )
    {
        vkMode = VKM_INPUT_UNIT;
    }
    else if ( action == m_actVkTabs )
    {
        vkMode = VKM_INPUT_TABS;
    }
    else if ( action == m_actVkSpecial )
    {
        vkMode = VKM_INPUT_SPECIAL;
    }
    else if ( action == m_actVkCustomCharMode )
    {
        vkMode = VKM_INPUT_USER_CHAR;
    }

    foreach ( QAction *act, m_actGrpVk->actions() )
    {
        if ( act == action )
        {
            act->setIcon( QIcon(ICO_CHECKED) );
        }
        else
        {
            act->setIcon( QIcon() );
        }
    }

    emit signal_open_vk( vkMode );
}

void SysTrayMenu::slot_skin_switch_triggered( QAction *action )
{
    foreach ( QAction *act, m_actGrpSkin->actions() )
    {
        if ( act == action )
        {
            act->setIcon( QIcon(ICO_CHECKED) );
        }
        else
        {
            act->setIcon( QIcon() );
        }
    }

    Settings::set_cur_skin_id( action->text() );
    Settings::save_cur_skin_id_to_file();

    emit signal_load_skin( action->text() );
}

void SysTrayMenu::slot_setting_triggered()
{
    emit signal_open_setting_win();
}

void SysTrayMenu::slot_fcitx_config_triggered()
{
    emit signal_fcitx_config();
}


void SysTrayMenu::slot_load_setting_data()
{
    QString skinId = Settings::get_cur_skin_id();
    if ( m_curSkinId != skinId )
    {
        QList<QAction *> acts = m_actGrpSkin->actions();
        foreach( QAction *act, acts )
        {
            if ( act->text() == skinId )
            {
                m_curSkinId = skinId;
                act->setIcon( QIcon(ICO_CHECKED) );
            }
            else
            {
                act->setIcon( QIcon() );
            }
        }
    }
}

void SysTrayMenu::slot_update_skin_list()
{
    m_curSkinId = Settings::get_cur_skin_id();

    QString skinDir = INSTALL_DIR + "/skin/";
    QStringList dirList = QDir(skinDir).entryList( QDir::Dirs );
    dirList.removeOne( "." );
    dirList.removeOne( ".." );

    QList<QAction *> acts = m_actGrpSkin->actions();
    foreach( QAction *act, acts )
    {
        m_actGrpSkin->removeAction( act );
    }

    QStringList skinIdList;
    foreach( QString skinId, dirList )
    {
        if ( QFile( skinDir + skinId + "/skin.ini" ).exists() )
        {
            skinIdList << skinId;

            QAction *act = new QAction( skinId, this );
            m_actGrpSkin->addAction( act );
            if ( act->text() == m_curSkinId )
            {
                act->setIcon( QIcon(ICO_CHECKED) );
            }
        }
    }

    Settings::save_exist_skin( skinIdList );

    m_menuSkin->clear();
    m_menuSkin->addActions( m_actGrpSkin->actions() );
}






