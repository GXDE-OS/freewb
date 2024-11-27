#include "mainprogram.h"
#include "commdefine.h"
#include <QDebug>

#include "../../ipc/ipc.h"

#define DEBUG

MainProgram::MainProgram( QObject *parent ) : QObject( parent )
{
//    if ( !fcitx_service_is_running() )
//    {
//        qWarning() << "fcitx service isn't running, Freewb quit!";
//        QCoreApplication::instance()->quit();
//    }

    //puts("极点后台程序启动.....");

    m_x11EventMonitor = new X11EventMonitor( this );
    m_x11EventMonitor->start();

    m_kimAgent = new KimAgent( this );//创建与fcitx通信的代理对象

    m_virtualKeyboard = new Keyboard;
    m_sysTrayMenu = new SysTrayMenu;
    m_toolbar = new ToolbarWin;
    m_inputWin = new InputWin;
    m_contextmenu = new ContextMenu;
    m_settingWin = new SettingWin;
    m_lexicontoolWin = new LexiconToolWin;
    m_usrGenWordDialog = new UsrGenWordDialog;
    m_textEditWin = new TextEditWin;
    m_dictQueryWin = new DictQueryWin;
    m_backupDialog = new BackupDialog;

    //创建本地DBUS通信服务
    create_host_dbus_service();
//org.fcitx.FcitxConfigGtk3
//org.freedesktop.Aplication
    //fcitx通信代理发出的信号
    //m_kimAgent --> m_inputWin
    connect( m_kimAgent, &KimAgent::signal_Enable, m_inputWin, &InputWin::slot_kim_Enable );
    connect( m_kimAgent, &KimAgent::signal_ShowPreedit, m_inputWin, &InputWin::slot_kim_ShowPreedit );
    connect( m_kimAgent, &KimAgent::signal_ShowAux, m_inputWin, &InputWin::slot_kim_ShowAux );
    connect( m_kimAgent, &KimAgent::signal_ShowLookupTable, m_inputWin, &InputWin::slot_kim_ShowLookupTable );
    connect( m_kimAgent, &KimAgent::signal_UpdateLookupTableCursor, m_inputWin, &InputWin::slot_kim_UpdateLookupTableCursor );
    connect( m_kimAgent, &KimAgent::signal_UpdateLookupTable, m_inputWin, &InputWin::slot_kim_UpdateLookupTable );
    connect( m_kimAgent, &KimAgent::signal_SetLookupTable, m_inputWin, &InputWin::slot_kim_SetLookupTable );
    connect( m_kimAgent, &KimAgent::signal_UpdatePreeditCaret, m_inputWin, &InputWin::slot_kim_UpdatePreeditCaret );
    connect( m_kimAgent, &KimAgent::signal_UpdatePreeditText, m_inputWin, &InputWin::slot_kim_UpdatePreeditText );
    connect( m_kimAgent, &KimAgent::signal_UpdateAux, m_inputWin, &InputWin::slot_kim_UpdateAux );
    connect( m_kimAgent, &KimAgent::signal_UpdateSpotLocation, m_inputWin, &InputWin::slot_kim_UpdateSpotLocation );
    connect( m_kimAgent, &KimAgent::signal_SetSpotLocation, m_inputWin, &InputWin::slot_kim_SetSpotLocation );
    connect( m_kimAgent, &KimAgent::signal_UpdateScreen,m_inputWin, &InputWin::slot_kim_UpdateScreen );
    connect( m_kimAgent, &KimAgent::signal_UpdateProperty, m_inputWin, &InputWin::slot_kim_UpdateProperty );
    connect( m_kimAgent, &KimAgent::signal_RegisterProperties, m_inputWin, &InputWin::slot_kim_RegisterProperties );
    connect( m_kimAgent, &KimAgent::signal_ExecDialog, m_inputWin, &InputWin::slot_kim_ExecDialog );
    connect( m_kimAgent, &KimAgent::signal_ExecMenu, m_inputWin, &InputWin::slot_kim_ExecMenu );
    //m_kimAgent --> m_toolbar
    connect( m_kimAgent, &KimAgent::signal_UpdateProperty, m_toolbar, &ToolbarWin::slot_kim_UpdateProperty );
    connect( m_kimAgent, &KimAgent::signal_RegisterProperties, m_toolbar, &ToolbarWin::slot_kim_RegisterProperties );
    //m_kimAgent --> m_sysTrayMenu
    connect( m_kimAgent, &KimAgent::signal_UpdateProperty, m_sysTrayMenu, &SysTrayMenu::slot_kim_UpdateProperty );
    connect( m_kimAgent, &KimAgent::signal_RegisterProperties, m_sysTrayMenu, &SysTrayMenu::slot_kim_RegisterProperties );
    connect( m_kimAgent, &KimAgent::signal_ExecMenu, m_sysTrayMenu, &SysTrayMenu::slot_kim_ExecMenu );

    //输入面板发送的信号
    //m_inputWin --> m_kimAgent
    connect( m_inputWin, &InputWin::signal_candidate_select, m_kimAgent, &KimAgent::SelectCandidate );
    connect( m_inputWin, &InputWin::signal_candidate_page_up, m_kimAgent, &KimAgent::LookupTablePageUp );
    connect(m_inputWin, &InputWin::signal_candidate_page_down, m_kimAgent, &KimAgent::LookupTablePageDown );
    //m_inputWin --> m_toolbar
    connect( m_inputWin, &InputWin::signal_btn_charWidth_clicked, m_toolbar, &ToolbarWin::on_btnCharWidth_clicked );
    connect( m_inputWin, &InputWin::signal_btn_mark_clicked, m_toolbar, &ToolbarWin::on_btnMark_clicked );
    //m_inputWin --> m_contextmenu
    connect( m_inputWin, &InputWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu );


    //工具条发送的信号
    //m_toolbar --> m_kimAgent
    connect( m_toolbar, &ToolbarWin::signal_fcitx_switch_inputmethod_1, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_toolbar, &ToolbarWin::signal_fcitx_switch_inputmethod, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_toolbar, &ToolbarWin::signal_fcitx_switch_char_font, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_toolbar, &ToolbarWin::signal_fcitx_switch_char_width, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_toolbar, &ToolbarWin::signal_fcitx_switch_mark, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_toolbar, &ToolbarWin::signal_switch_char_font, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_toolbar, &ToolbarWin::signal_switch_char_set, m_kimAgent, &KimAgent::ReloadConfig );
    //m_toolbar --> m_sysTrayMenu
    connect( m_toolbar, &ToolbarWin::signal_char_font_changed, m_sysTrayMenu, &SysTrayMenu::slot_update_char_font_ico );
    //m_toolbar --> m_settingWin
    connect( m_toolbar, &ToolbarWin::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win );
    //m_toolbar --> m_contextmenu
    connect( m_toolbar, &ToolbarWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu );
    //m_toolbar --> m_virtualKeyboard
    connect( m_toolbar, &ToolbarWin::signal_toggle_vk, m_virtualKeyboard, &Keyboard::slot_toggle_win );
    connect( m_toolbar, &ToolbarWin::signal_open_vk, m_virtualKeyboard, &Keyboard::slot_open_win );
    //m_toolbar --> m_inputWin
    connect( m_toolbar, &ToolbarWin::signal_btn_charWidth_clicked, m_inputWin, &InputWin::slot_update_charWidth_btn_ico );
    connect( m_toolbar, &ToolbarWin::signal_btn_mark_clicked, m_inputWin, &InputWin::slot_update_mark_btn_ico );
    //m_toolbar --> m_usrGenWordDialog
    connect( m_toolbar, &ToolbarWin::signal_open_generate_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog );
    //m_toolbar --> m_dictQueryWin
    connect( m_toolbar, &ToolbarWin::signal_open_dict_query_win, m_dictQueryWin, &DictQueryWin::slot_open_win );

    //虚拟键盘发出的信号
    //m_virtualKeyboard --> m_sysTrayMenu
    connect( m_virtualKeyboard, &Keyboard::signal_vk_mode_changed, m_sysTrayMenu, &SysTrayMenu::slot_vk_mode_changed );
    connect( m_virtualKeyboard, &Keyboard::signal_vk_mode_changed, m_toolbar, &ToolbarWin::slot_vk_mode_changed );
    //m_virtualKeyboard --> m_kimAgent
    connect( m_virtualKeyboard, &Keyboard::signal_vk_flg_changed, m_kimAgent, &KimAgent::ReloadConfig );
    //m_virtualKeyboard --> m_toolbar
    connect( m_virtualKeyboard, &Keyboard::signal_kb_caps_changed, m_toolbar, &ToolbarWin::slot_kb_caps_changed );

    //右键菜单发出的信号
    //m_contextmenu --> m_kimAgent
    connect( m_contextmenu, &ContextMenu::signal_ime_table_changed, m_kimAgent, &KimAgent::ReloadConfig );
    //m_contextmenu --> m_lexicontoolWin
    connect( m_contextmenu, &ContextMenu::signal_open_lexicon_tool, m_lexicontoolWin, &LexiconToolWin::open_win );
    //m_contextmenu --> m_settingWin
    connect( m_contextmenu, &ContextMenu::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win );
    connect( m_contextmenu, &ContextMenu::signal_restore_all_settings, m_settingWin, &SettingWin::slot_init_all_setting_page );
    connect( m_contextmenu, &ContextMenu::signal_show_version_info, m_settingWin, &SettingWin::slot_show_version_info );
    //m_contextmenu --> m_textEditWin
    connect( m_contextmenu, &ContextMenu::signal_open_textEdit_win, m_textEditWin, &TextEditWin::slot_open_textEdit_win );
    //m_contextmenu --> m_usrGenWordDialog
    connect( m_contextmenu, &ContextMenu::signal_open_user_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog );
    //m_contextmenu --> m_backup
    connect( m_contextmenu, &ContextMenu::signal_backup_lexicon_and_settings, m_backupDialog, &BackupDialog::slot_backup_lexicon_and_settings );
    connect( m_contextmenu, &ContextMenu::signal_restore_lexicon_and_settings , m_backupDialog, &BackupDialog::slot_restore_lexicon_and_settings );


    //系统事件过滤器发出的信号
    //m_x11EventMonitor --> m_contextmenu
    connect( m_x11EventMonitor, &X11EventMonitor::signal_button_pressed, m_contextmenu, &ContextMenu::slot_button_pressed );
    connect( m_x11EventMonitor, &X11EventMonitor::signal_key_pressed, m_contextmenu, &ContextMenu::slot_key_pressed );
    //m_x11EventMonitor --> m_toolbar
    connect( m_x11EventMonitor, &X11EventMonitor::signal_button_pressed, m_toolbar, &ToolbarWin::slot_button_pressed );
    connect( m_x11EventMonitor, &X11EventMonitor::signal_key_pressed, m_toolbar, &ToolbarWin::slot_key_pressed );
    //m_x11EventMonitor --> m_virtualKeyboard
    connect( m_x11EventMonitor, &X11EventMonitor::signal_key_clicked, m_virtualKeyboard, &Keyboard::slot_key_clicked );

    //系统托盘菜单发送的信号
    //m_sysTrayMenu --> this
    connect( m_sysTrayMenu, &SysTrayMenu::signal_create_freewb_panel, this, &MainProgram::slot_create_freewb_panel );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_delete_freewb_panel, this, &MainProgram::slot_delete_freewb_panel );
    //m_sysTrayMenu --> m_kimAgent
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_switch_inputmethod_1, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_switch_inputmethod, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_switch_char_font, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_switch_char_width, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_switch_mark, m_kimAgent, &KimAgent::TriggerProperty );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_reload_config, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_fcitx_config, m_kimAgent, &KimAgent::Configure );
    //m_sysTrayMenu --> m_toolbar
    connect( m_sysTrayMenu, &SysTrayMenu::signal_load_skin, m_toolbar, &ToolbarWin::slot_load_skin );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_switch_char_font, m_toolbar, &ToolbarWin::slot_switch_char_font_mode );
    //m_sysTrayMenu --> m_virtualKeyboard
    connect( m_sysTrayMenu, &SysTrayMenu::signal_vk_toggle, m_virtualKeyboard, &Keyboard::slot_toggle_win );
    connect( m_sysTrayMenu, &SysTrayMenu::signal_open_vk, m_virtualKeyboard, &Keyboard::slot_open_win );
    //m_sysTrayMenu --> m_inputWin
    connect( m_sysTrayMenu, &SysTrayMenu::signal_load_skin, m_inputWin, &InputWin::slot_load_skin );
    //m_sysTrayMenu --> m_settingWin
    connect( m_sysTrayMenu, &SysTrayMenu::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win );


    //造词对话框发送的信号
    //m_usrGenWordDialog --> m_kimAgent
    connect( m_usrGenWordDialog, &UsrGenWordDialog::signal_user_word_changed, m_kimAgent, &KimAgent::ReloadConfig );


    //用户手动编辑文本文件改变发送的信号
    //m_textEditWin --> m_kimAgent
    connect( m_textEditWin, &TextEditWin::signal_setting_file_changed, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_textEditWin, &TextEditWin::signal_quickTable_file_saved, m_kimAgent, &KimAgent::ReloadConfig );
    connect( m_textEditWin, &TextEditWin::signal_imTable_file_changed, m_kimAgent, &KimAgent::ReloadConfig );
    //m_textEditWin --> m_toolbar
    connect( m_textEditWin, &TextEditWin::signal_setting_file_changed, m_toolbar, &ToolbarWin::slot_load_setting_data );
    //m_textEditWin --> m_virtualKeyboard
    connect( m_textEditWin, &TextEditWin::signal_setting_file_changed, m_virtualKeyboard, &Keyboard::slot_load_setting_data );
    //m_textEditWin --> m_inputWin
    connect( m_textEditWin, &TextEditWin::signal_setting_file_changed, m_inputWin, &InputWin::slot_load_setting_data );
    //m_textEditWin --> m_sysTrayMenu
    connect( m_textEditWin, &TextEditWin::signal_setting_file_changed, m_sysTrayMenu, &SysTrayMenu::slot_load_setting_data );
    //m_textEditWin --> m_usrGenWordDialog
    connect( m_textEditWin, &TextEditWin::signal_userWord_file_saved, m_usrGenWordDialog, &UsrGenWordDialog::slot_userWord_file_saved );


    //词典工具箱发送的信号
    //m_lexicontoolWin --> m_kimAgent
    connect( m_lexicontoolWin, &LexiconToolWin::signal_user_word_file_changed, m_kimAgent, &KimAgent::ReloadConfig );


    //备份窗口发送的信号
    //m_backupDialog --> m_kimAgent
    connect( m_backupDialog, &BackupDialog::signal_restore_lexicon_and_settings_ok, m_kimAgent, &KimAgent::ReloadConfig );


    //配置数据改变发送的信号
    //g_settings --> m_kimAgent
    connect( &g_settings, &Settings::signal_setting_data_changed_to_fcitx, m_kimAgent, &KimAgent::ReloadConfig );
    //g_settings --> m_toolbar
    connect( &g_settings, &Settings::signal_setting_data_changed_to_local, m_toolbar, &ToolbarWin::slot_load_setting_data );
    //g_settings --> m_virtualKeyboard
    connect( &g_settings, &Settings::signal_setting_data_changed_to_local, m_virtualKeyboard, &Keyboard::slot_load_setting_data );
    //g_settings --> m_inputWin
    connect( &g_settings, &Settings::signal_setting_data_changed_to_local, m_inputWin, &InputWin::slot_load_setting_data );
    //g_settings --> m_sysTrayMenu
    connect( &g_settings, &Settings::signal_setting_data_changed_to_local, m_sysTrayMenu, &SysTrayMenu::slot_load_setting_data );
}


MainProgram::~MainProgram()
{
    if ( m_contextmenu )
        delete m_contextmenu;
    if ( m_toolbar )
        delete m_toolbar;
    if ( m_virtualKeyboard )
        delete m_toolbar;
    if ( m_inputWin )
        delete m_inputWin;
    if ( m_settingWin )
        delete m_settingWin;
    if ( m_lexicontoolWin )
        delete m_lexicontoolWin;
    if ( m_sysTrayMenu )
        delete m_sysTrayMenu;
    if ( m_usrGenWordDialog )
        delete m_usrGenWordDialog;
    if ( m_textEditWin )
        delete m_textEditWin;
    if ( m_dictQueryWin )
        delete m_dictQueryWin;
    if ( m_backupDialog )
        delete m_backupDialog;

    if ( QDBusConnection( FREEWUBI_SETTINGS_BUSNAME ).isConnected() )
    {
        QDBusConnection( FREEWUBI_SETTINGS_BUSNAME ).unregisterObject( FREEWUBI_SETTINGS_OBJECTPATH );
        QDBusConnection( FREEWUBI_SETTINGS_BUSNAME ).unregisterService( FREEWUBI_SETTINGS_SERVICENAME );
        QDBusConnection::disconnectFromBus( FREEWUBI_SETTINGS_BUSNAME );
    }
}

bool MainProgram::fcitx_service_is_running()
{
    QDBusMessage msg = QDBusMessage::createMethodCall( "org.fcitx.Fcitx",
                                                       "/inputmethod",
                                                       "org.fcitx.Fcitx.InputMethod",
                                                       "GetCurrentState" );

    QDBusMessage rep = QDBusConnection::sessionBus().call( msg );
    if ( rep.type() == QDBusMessage::ReplyMessage )
    {
        return true;
    }

    return false;
}


void MainProgram::create_host_dbus_service()
{
    //注册本地ＤＢＵＳ服务与对象
    if ( !QDBusConnection::connectToBus( QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME )
         .registerService( FREEWUBI_SETTINGS_SERVICENAME ) )
    {
        qWarning() << "create host service failed!";
        return;
    }

    QDBusConnection::connectToBus( QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME )
            .registerObject( FREEWUBI_SETTINGS_OBJECTPATH, this, QDBusConnection::ExportAllSlots );
}

void MainProgram::slot_create_freewb_panel()
{
#ifdef DEBUG
    puts("show tool-bar 00000000000000\n");
#endif
    m_kimAgent->create_fcitx_panel();
    if ( !Settings::get_hide_toolbar_flg() )
    {
        m_toolbar->show();
    }
    m_sysTrayMenu->show();
}

void MainProgram::slot_delete_freewb_panel()
{
    m_kimAgent->delete_fcitx_panel();
    m_toolbar->hide();
    m_inputWin->hide();
    m_sysTrayMenu->hide();
}


/********************************* 以下槽函数供输入法引擎通过DBUS调用 ***************************************/
QString MainProgram::slot_dbus_test( const QString &text )
{
    qDebug() << text;
    return text;
}

void MainProgram::slot_dbus_switch_freewb( int flag )
{

    if ( flag == 0 && SysTrayMenu::is_extern_im() ) //极点五笔
    {
        if ( !m_kimAgent->create_fcitx_panel() ) //只有面板创建成功才认为是完全切换到了极点五笔输入法
        {
            SysTrayMenu::set_extern_im( false );
            m_toolbar->show();
            //Settings::save_simpTradSwitchEnable_to_fcitx_config_file( false );
            //m_kimAgent->ReloadConfig();//ReloadConfig信号必须在建立kimpanel面板之后发送
            //m_sysTrayMenu->show();//x86版本不显示系统托盘，因为只要显示就无法隐藏了，是Qt的bug？
        }
    }
    else if ( flag == 1 || (flag == 2 && !SysTrayMenu::is_extern_im()) ) //退出极点五笔
    {
        #ifdef DEBUG
        qDebug()<<"退出极点五笔\n";
        #endif
        SysTrayMenu::set_extern_im( true );
        Settings::save_simpTradSwitchEnable_to_fcitx_config_file( true );
        m_kimAgent->ReloadConfig();//ReloadConfig信号必须在删除kimpanel面板之前发送
        m_kimAgent->delete_fcitx_panel();
        //m_sysTrayMenu->hide();
        m_toolbar->hide();
        m_inputWin->hide();
    }
}


//切换输入法
void MainProgram::slot_dbus_switch_internal_input_method( int im )
{

    //printf("slot_dbus_switch_internal_input_method=%d\n",im);
    if ( SysTrayMenu::is_extern_im() ) 
    {
        return;
    }

    if ( (ToolbarWin::get_input_mode() == IM_ENGLISH && im != IM_ENGLISH) ||
         (ToolbarWin::get_input_mode() != IM_ENGLISH && im != IM_ENGLISH) )
    {
        m_virtualKeyboard->switch_caps_flg( 0 );
    }
    Settings::save_inputmethod_flg_to_file( static_cast<InputMode>(im) );

    // 光标处提示极点五笔子输入法
    QString childIm;
    if ( im == IM_WUBI_FONT && im != ToolbarWin::get_input_mode() )
    {
        childIm = "五笔字型";
    }
    else if ( im == IM_WUBI_PINYIN && im != ToolbarWin::get_input_mode() )
    {
        childIm = "五笔拼音";
    }
    if ( im == IM_STD_PINYIN && im != ToolbarWin::get_input_mode() )
    {
        childIm = "拼音输入";
    }
    m_inputWin->slot_kim_UpdateAux( childIm, "" );
    m_inputWin->slot_kim_ShowAux( true );

    ToolbarWin::set_input_mode( static_cast<InputMode>(im) );
    m_toolbar->slot_update_input_mode_ico();
    if( g_cpuType==CT_ARM) //+ 2023-11-7
    m_sysTrayMenu->slot_update_input_mode_ico();//x86版本不显示系统托盘，因为只要显示就无法隐藏了，是Qt的bug？
}

//输入法面板程序退出
void MainProgram::slot_dbus_panel_exit()
{
    #ifdef DEBUG
    qDebug() << "Freewb quit!";
    #endif
    QCoreApplication::instance()->quit();
}

//字典查询
void MainProgram::slot_dbus_dict_query( const QString &text )
{
//    qDebug() << text;
    m_dictQueryWin->slot_open_win( text );
}

//用户造词
void MainProgram::slot_dbus_generate_usr_word( int flg, const QString &wordText, const QString &wordCode )
{
//    qDebug() << flg << wordText << wordCode;

    if ( flg == 0 )//显示待造词组
    {
        m_inputWin->show_user_word_operation_prompt( 1, wordText, wordCode );
    }
    else if ( flg == 1 )//确认造词
    {
        m_usrGenWordDialog->add_user_word( wordText, wordCode );
        m_inputWin->close_user_word_operation_prompt();
    }
    else if ( flg == 2 )//取消造词
    {
        m_inputWin->close_user_word_operation_prompt();
    }
    else if ( flg == 3 )//自定义词组编码
    {
        m_inputWin->close_user_word_operation_prompt();
        m_usrGenWordDialog->slot_show_dialog( wordText, wordCode );
    }
}

//用户删词
void MainProgram::slot_dbus_delete_usr_word( int flg, const QString &wordText, const QString &wordCode )
{
//    qDebug() << flg << wordText << wordCode;

    if ( flg == 0 )//显示待删词组
    {
        m_inputWin->show_user_word_operation_prompt( 0, wordText, wordCode );
    }
    else if ( flg == 1 )//确认删词
    {
        m_usrGenWordDialog->delete_user_word( wordText, wordCode );
        m_inputWin->close_user_word_operation_prompt();
    }
    else if ( flg == 2 )//取消删词
    {
        m_inputWin->close_user_word_operation_prompt();
    }
}

//输入法引擎载入用户词组文件完成
void MainProgram::slot_dbus_usr_word_load_ok()
{
    Settings::save_userWord_change_flg_to_file( 0 );
}

//快捷码表加载完成
void MainProgram::slot_dbus_quick_table_load_ok()
{
    Settings::save_quickTable_change_flg_to_file( 0 );
}

//切换虚拟键盘
void MainProgram::slot_dbus_switch_vk( int flg )
{
    m_virtualKeyboard->switch_vk( flg );
}

//退出软键盘
void MainProgram::slot_dbus_close_vk()
{
    m_virtualKeyboard->slot_toggle_win();
}

//切换字符集
void MainProgram::slot_dbus_switch_char_set()
{
    m_toolbar->switch_char_set();
}

//切换简/繁体
void MainProgram::slot_dbus_switch_simp_or_trad( int tradFlg )
{
    m_toolbar->switch_char_font_mode( static_cast<CharFontMode>(tradFlg) );
    m_sysTrayMenu->slot_update_char_font_ico( static_cast<CharFontMode>(tradFlg) );
}

//切换大小写状态
void MainProgram::slot_dbus_switch_caps_state()
{

    m_toolbar->slot_update_input_mode_ico();
}

//显/隐状态栏
void MainProgram::slot_dbus_switch_toolbar_hide_flg()
{

    bool flg = Settings::get_hide_toolbar_flg()?false:true;

    #ifdef DEBUG
    printf("显/隐状态栏=%d\n",flg );
    #endif
    if ( flg )
    {
        m_toolbar->hide();
    }
    else
    {
        m_toolbar->show();
    }
    Settings::set_hide_toolbar_flg( flg );
    Settings::save_hide_toolbar_flg_to_file();
}

//显/隐候选框
void MainProgram::slot_dbus_switch_candiwin_hide_flg()
{
    bool flg = Settings::get_hide_candiWin_flg() ? false : true;
    Settings::set_hide_candiWin_flg( flg );
    Settings::save_hide_candiwin_flg_to_file();
}

//切换词库
void MainProgram::slot_dbus_switch_lexicon()
{
    QStringList lexiconList = Settings::get_exist_lexicon();
    QString curlexicon = Settings::get_cur_used_lexicon();

    if ( lexiconList.length() < 2 ) return;

    int i = 0;
    while ( i < lexiconList.length() )
    {
        if ( lexiconList.at(i++) == curlexicon )
        {
            if ( i >= lexiconList.length() )
            {
                i = 0;
            }

            Settings::save_cur_used_lexicon_to_file( lexiconList.at(i) );
            Settings::save_ime_table_changed_flg_to_file( 1 );
            m_contextmenu->update_lexicon_checked_ico();
            m_kimAgent->ReloadConfig();
            break;
        }
    }
}


//切换皮肤
void MainProgram::slot_dbus_switch_skin()
{
    #ifdef DEBUG
        puts("******** change skin ********\n");
    #endif

    QStringList skinList = Settings::get_exist_skin();
    QString curSkin = Settings::get_cur_skin_id();

    if ( skinList.length() < 2 ) return;

    int i = 0;
    while ( i < skinList.length() )
    {
        if ( skinList.at(i++) == curSkin )
        {
            if ( i >= skinList.length() )
            {
                i = 0;
            }

            Settings::set_cur_skin_id( skinList.at(i) );
            Settings::save_cur_skin_id_to_file();
            break;
        }
    }
}


//标点自动配对
void MainProgram::slot_dbus_set_mark_auto_pairs_flg( int flg )
{
    Settings::set_smartMark_flg( flg );
    Settings::save_smart_mark_flg_to_file();
}

//输入法词库加载完成
void MainProgram::slot_dbus_ime_table_load_ok()
{
    Settings::save_ime_table_changed_flg_to_file( 0 );
}

void MainProgram::slot_dbus_set_charWidth_and_markMode( int charWidth, int markMode )
{

    if(charWidth) m_toolbar->update_char_width_mode_ico(static_cast<CharWidthMode>(0));
    if(markMode) m_toolbar->update_mark_mode_ico(static_cast<MarkMode>(0));
    /*
    m_toolbar->update_char_width_mode_ico( static_cast<CharWidthMode>(charWidth) );
    m_toolbar->update_mark_mode_ico( static_cast<MarkMode>(markMode) );

    m_sysTrayMenu->update_char_width_mode_ico( static_cast<CharWidthMode>(charWidth) );
    m_sysTrayMenu->update_mark_mode_ico( static_cast<MarkMode>(markMode) );
    */
}


QString MainProgram::slot_dbus_get_clipboard_text()
{
    QClipboard *clipboard = QApplication::clipboard();
    return clipboard->text();
}


void MainProgram::slot_dbus_open_freewb_dir()//进入极点目录
{
    char cmd[128] = {0};

    if ( g_desktopType == DT_MATE )//银河麒麟系统
    {
        sprintf( cmd, "caja %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data() );
    }
    else if ( g_desktopType == DT_UKUI )//优麒麟系统
    {
        sprintf( cmd, "peony %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data() );
    }
    else if ( g_desktopType == DT_DEEPIN )//深度系统
    {
        sprintf( cmd, "pcmanfm %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data() );
    }
    else if ( g_desktopType == DT_UBUNTU ) //Ubuntu
    {
        sprintf( cmd, "nautilus %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data() );
    }
    else
    {
        sprintf( cmd, "xdg-open %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data() );
    }
    system( cmd );
}

void MainProgram::slot_dbus_word_freq_switch_ok( const QString &wordText, int flg )//词组常用与非常用切换成功
{
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowFlag( Qt::FramelessWindowHint );
    msgBox->setIcon( QMessageBox::Information );
    msgBox->setText( QString("【 %1 】已切换为%2词组").arg(wordText).arg(flg? "非常用" : "常用") );
    msgBox->setStandardButtons( QMessageBox::Ok );
    msgBox->button( QMessageBox::Ok )->setIcon( QIcon() );
    msgBox->button( QMessageBox::Ok )->setText( "确定(&OK)" );
    msgBox->setDefaultButton( QMessageBox::Ok );
    msgBox->exec();
    delete  msgBox;
}

void MainProgram::slot_dbus_open_ui_setting()//进入设置
{
    m_settingWin->slot_open_win();
}

void MainProgram::slot_dbus_open_advanced_setting()//进入专家设置模式
{
    m_textEditWin->slot_open_textEdit_win( TEM_SETTING_FILE );
}

void MainProgram::slot_dbus_edit_usr_table()//编辑用户码表
{
    m_textEditWin->slot_open_textEdit_win( TEM_USER_WORD );
}

void MainProgram::slot_dbus_edit_wubi_table()//编辑五笔码表
{
    m_textEditWin->slot_open_textEdit_win( TEM_WUBI_TABLE );
}

void MainProgram::slot_dbus_edit_pinyin_table()//编辑拼音码表
{
    m_textEditWin->slot_open_textEdit_win( TEM_PINYIN_TABLE );
}

void MainProgram::slot_dbus_show_version_info()//显示极点版本信息
{
    m_settingWin->slot_show_version_info();
}

void MainProgram::slot_dbus_edit_quick_table()//编辑快捷码表
{
    m_textEditWin->slot_open_textEdit_win( TEM_QUICK_TABLE );
}

void MainProgram::slot_dbus_set_recode_calib_flg( int flg )//切换重码上屏校对模式
{
    Settings::set_recodeCalib_flg( flg );
    Settings::save_recode_calib_flg_to_file();
}

/*****************************************************************************************************/

