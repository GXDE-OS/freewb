#include "mainprogram.h"

#include <QDebug>

#include "../../ipc/ipc.h"
#include "config.h"
#include "settings.h"
#include "log.h"

MainProgram::MainProgram(QObject *parent) : QObject(parent)
{
    m_x11EventMonitor = new X11EventMonitor(this);
    m_x11EventMonitor->start();

    m_kimAgent = new KimAgent(this); // 创建与fcitx通信的代理对象

    m_virtualKeyboard = new Keyboard;
    m_toolbar = new ToolbarWin;
    m_inputWin = new InputWin;
    m_contextmenu = new ContextMenu;
    m_settingWin = new SettingWin;
    m_lexicontoolWin = new LexiconToolWin;
    m_usrGenWordDialog = new UsrGenWordDialog;
    m_textEditWin = new TextEditWin;
    m_dictQueryWin = new DictQueryWin;
    m_backupDialog = new BackupDialog;

    // 创建本地DBUS通信服务
    create_host_dbus_service();
    // org.fcitx.FcitxConfigGtk3
    // org.freedesktop.Aplication
    // fcitx通信代理发出的信号
    // m_kimAgent --> m_inputWin
    connect(m_kimAgent, &KimAgent::signal_ShowPreedit, m_inputWin, &InputWin::slot_kim_ShowPreedit);
    connect(m_kimAgent, &KimAgent::signal_ShowAux, m_inputWin, &InputWin::slot_kim_ShowAux);
    connect(m_kimAgent, &KimAgent::signal_ShowLookupTable, m_inputWin, &InputWin::slot_kim_ShowLookupTable);
    connect(m_kimAgent, &KimAgent::signal_UpdateLookupTable, m_inputWin, &InputWin::slot_kim_UpdateLookupTable);
    connect(m_kimAgent, &KimAgent::signal_SetLookupTable, m_inputWin, &InputWin::slot_kim_SetLookupTable);
    connect(m_kimAgent, &KimAgent::signal_UpdatePreeditCaret, m_inputWin, &InputWin::slot_kim_UpdatePreeditCaret);
    connect(m_kimAgent, &KimAgent::signal_UpdatePreeditText, m_inputWin, &InputWin::slot_kim_UpdatePreeditText);
    connect(m_kimAgent, &KimAgent::signal_UpdateAux, m_inputWin, &InputWin::slot_kim_UpdateAux);
    connect(m_kimAgent, &KimAgent::signal_UpdateSpotLocation, m_inputWin, &InputWin::slot_kim_UpdateSpotLocation);
    connect(m_kimAgent, &KimAgent::signal_SetSpotLocation, m_inputWin, &InputWin::slot_kim_SetSpotLocation);
    // m_kimAgent --> m_toolbar
    connect(m_kimAgent, &KimAgent::signal_UpdateProperty, m_toolbar, &ToolbarWin::slot_kim_UpdateProperty);
    connect(m_kimAgent, &KimAgent::signal_RegisterProperties, m_toolbar, &ToolbarWin::slot_kim_RegisterProperties);

    // 输入面板发送的信号
    // m_inputWin --> m_kimAgent
    connect(m_inputWin, &InputWin::signal_candidate_select, m_kimAgent, &KimAgent::SelectCandidate);
    connect(m_inputWin, &InputWin::signal_candidate_page_up, m_kimAgent, &KimAgent::LookupTablePageUp);
    connect(m_inputWin, &InputWin::signal_candidate_page_down, m_kimAgent, &KimAgent::LookupTablePageDown);
    // m_inputWin --> m_toolbar
    connect(m_inputWin, &InputWin::signal_btn_charWidth_clicked, m_toolbar, &ToolbarWin::on_btnCharWidth_clicked);
    connect(m_inputWin, &InputWin::signal_btn_mark_clicked, m_toolbar, &ToolbarWin::on_btnMark_clicked);
    // m_inputWin --> m_contextmenu
    connect(m_inputWin, &InputWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu);

    // 工具条发送的信号
    // m_toolbar --> m_kimAgent
    connect(m_toolbar, &ToolbarWin::signal_request_next_input_mode, m_kimAgent, &KimAgent::RequestNextInputMode);
    connect(m_toolbar, &ToolbarWin::signal_fcitx_switch_char_width, m_kimAgent, &KimAgent::SwitchFullWidth);
    connect(m_toolbar, &ToolbarWin::signal_fcitx_switch_mark, m_kimAgent, &KimAgent::SwitchPunctuation);
    connect(m_toolbar, &ToolbarWin::signal_switch_chttrans, m_kimAgent, &KimAgent::SwitchChttrans);
    connect(m_toolbar, &ToolbarWin::signal_switch_char_set, m_kimAgent, &KimAgent::ReloadConfig);

    // m_toolbar --> m_settingWin
    connect(m_toolbar, &ToolbarWin::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win);
    // m_toolbar --> m_contextmenu
    connect(m_toolbar, &ToolbarWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu);
    // m_toolbar --> m_virtualKeyboard
    connect(m_toolbar, &ToolbarWin::signal_toggle_vk, m_virtualKeyboard, &Keyboard::slot_toggle_win);
    connect(m_toolbar, &ToolbarWin::signal_open_vk, m_virtualKeyboard, &Keyboard::slot_open_win);
    // m_toolbar --> m_inputWin
    connect(m_toolbar, &ToolbarWin::signal_btn_charWidth_clicked, m_inputWin, &InputWin::slot_update_charWidth_btn_ico);
    connect(m_toolbar, &ToolbarWin::signal_btn_mark_clicked, m_inputWin, &InputWin::slot_update_mark_btn_ico);
    // m_toolbar --> m_usrGenWordDialog
    connect(m_toolbar, &ToolbarWin::signal_open_generate_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog);
    // m_toolbar --> m_dictQueryWin
    connect(m_toolbar, &ToolbarWin::signal_open_dict_query_win, m_dictQueryWin, &DictQueryWin::slot_open_win);

    // m_virtualKeyboard --> m_kimAgent
    connect(m_virtualKeyboard, &Keyboard::signal_vk_flg_changed, m_kimAgent, &KimAgent::ReloadConfig);
    // m_virtualKeyboard --> m_toolbar
    connect(m_virtualKeyboard, &Keyboard::signal_kb_caps_changed, m_toolbar, &ToolbarWin::slot_kb_caps_changed);

    // 右键菜单发出的信号
    // m_contextmenu --> m_kimAgent
    connect(m_contextmenu, &ContextMenu::signal_ime_table_changed, m_kimAgent, &KimAgent::ReloadConfig);
    // m_contextmenu --> m_lexicontoolWin
    connect(m_contextmenu, &ContextMenu::signal_open_lexicon_tool, m_lexicontoolWin, &LexiconToolWin::open_win);
    // m_contextmenu --> m_settingWin
    connect(m_contextmenu, &ContextMenu::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win);
    connect(m_contextmenu, &ContextMenu::signal_restore_all_settings, m_settingWin, &SettingWin::slot_init_all_setting_page);
    connect(m_contextmenu, &ContextMenu::signal_show_version_info, m_settingWin, &SettingWin::slot_show_version_info);
    // m_contextmenu --> m_textEditWin
    connect(m_contextmenu, &ContextMenu::signal_open_textEdit_win, m_textEditWin, &TextEditWin::slot_open_textEdit_win);
    // m_contextmenu --> m_usrGenWordDialog
    connect(m_contextmenu, &ContextMenu::signal_open_user_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog);
    // m_contextmenu --> m_backup
    connect(m_contextmenu, &ContextMenu::signal_backup_lexicon_and_settings, m_backupDialog,
            &BackupDialog::slot_backup_lexicon_and_settings);
    connect(m_contextmenu, &ContextMenu::signal_restore_lexicon_and_settings, m_backupDialog,
            &BackupDialog::slot_restore_lexicon_and_settings);

    // 系统事件过滤器发出的信号
    // m_x11EventMonitor --> m_contextmenu
    connect(m_x11EventMonitor, &X11EventMonitor::signal_button_pressed, m_contextmenu, &ContextMenu::slot_button_pressed);
    connect(m_x11EventMonitor, &X11EventMonitor::signal_key_pressed, m_contextmenu, &ContextMenu::slot_key_pressed);
    // m_x11EventMonitor --> m_toolbar
    connect(m_x11EventMonitor, &X11EventMonitor::signal_button_pressed, m_toolbar, &ToolbarWin::slot_button_pressed);
    connect(m_x11EventMonitor, &X11EventMonitor::signal_key_pressed, m_toolbar, &ToolbarWin::slot_key_pressed);
    // m_x11EventMonitor --> m_virtualKeyboard
    connect(m_x11EventMonitor, &X11EventMonitor::signal_key_clicked, m_virtualKeyboard, &Keyboard::slot_key_clicked);

    // 造词对话框发送的信号
    // m_usrGenWordDialog --> m_kimAgent
    connect(m_usrGenWordDialog, &UsrGenWordDialog::signal_user_word_changed, m_kimAgent, &KimAgent::ReloadConfig);

    // 用户手动编辑文本文件改变发送的信号
    // m_textEditWin --> m_kimAgent
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_kimAgent, &KimAgent::ReloadConfig);
    connect(m_textEditWin, &TextEditWin::signal_quickTable_file_saved, m_kimAgent, &KimAgent::ReloadConfig);
    connect(m_textEditWin, &TextEditWin::signal_imTable_file_changed, m_kimAgent, &KimAgent::ReloadConfig);
    // m_textEditWin --> m_toolbar
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_toolbar, &ToolbarWin::slot_load_setting_data);
    // m_textEditWin --> m_virtualKeyboard
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_virtualKeyboard, &Keyboard::slot_load_setting_data);
    // m_textEditWin --> m_inputWin
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_inputWin, &InputWin::slot_load_setting_data);
    // m_textEditWin --> m_usrGenWordDialog
    connect(m_textEditWin, &TextEditWin::signal_userWord_file_saved, m_usrGenWordDialog,
            &UsrGenWordDialog::slot_userWord_file_saved);

    // 词典工具箱发送的信号
    // m_lexicontoolWin --> m_kimAgent
    connect(m_lexicontoolWin, &LexiconToolWin::signal_user_word_file_changed, m_kimAgent, &KimAgent::ReloadConfig);

    // 备份窗口发送的信号
    // m_backupDialog --> m_kimAgent
    connect(m_backupDialog, &BackupDialog::signal_restore_lexicon_and_settings_ok, m_kimAgent, &KimAgent::ReloadConfig);

    // 配置数据改变发送的信号
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_fcitx, m_kimAgent, &KimAgent::ReloadConfig);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_toolbar,
            &ToolbarWin::slot_load_setting_data);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_virtualKeyboard,
            &Keyboard::slot_load_setting_data);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_inputWin,
            &InputWin::slot_load_setting_data);
}

MainProgram::~MainProgram()
{
    if (m_contextmenu)
        delete m_contextmenu;
    if (m_toolbar)
        delete m_toolbar;
    if (m_virtualKeyboard)
        delete m_virtualKeyboard;
    if (m_inputWin)
        delete m_inputWin;
    if (m_settingWin)
        delete m_settingWin;
    if (m_lexicontoolWin)
        delete m_lexicontoolWin;
    if (m_usrGenWordDialog)
        delete m_usrGenWordDialog;
    if (m_textEditWin)
        delete m_textEditWin;
    if (m_dictQueryWin)
        delete m_dictQueryWin;
    if (m_backupDialog)
        delete m_backupDialog;

    if (QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).isConnected())
    {
        QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).unregisterObject(FREEWUBI_SETTINGS_OBJECTPATH);
        QDBusConnection(FREEWUBI_SETTINGS_BUSNAME).unregisterService(FREEWUBI_SETTINGS_SERVICENAME);
        QDBusConnection::disconnectFromBus(FREEWUBI_SETTINGS_BUSNAME);
    }
}

void MainProgram::create_host_dbus_service()
{
    // 注册本地ＤＢＵＳ服务与对象
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME)
             .registerService(FREEWUBI_SETTINGS_SERVICENAME))
    {
        qWarning() << "create host service failed!";
        return;
    }

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, FREEWUBI_SETTINGS_BUSNAME)
        .registerObject(FREEWUBI_SETTINGS_OBJECTPATH, this, QDBusConnection::ExportAllSlots);
}

void MainProgram::slot_delete_freewb_panel()
{
    m_toolbar->hide();
    m_inputWin->hide();
}

/********************************* 以下槽函数供输入法引擎通过DBUS调用 ***************************************/
// 切换输入法
void MainProgram::slot_switch_input_mode(const QString &inputMode)
{
    if (inputMode == ToolbarWin::get_input_mode())
    {
        return;
    }

    m_virtualKeyboard->switch_caps_flg(0);

    ToolbarWin::set_input_mode(inputMode);
    m_toolbar->slot_update_input_mode_ico();
}

// 输入法面板程序退出
void MainProgram::slot_dbus_panel_exit()
{
    qDebug() << "Freewb quit!";
    slot_delete_freewb_panel();
}

// 字典查询
void MainProgram::slot_dbus_dict_query(const QString &text)
{
    //    qDebug() << text;
    m_dictQueryWin->slot_open_win(text);
}

// 用户造词
void MainProgram::slot_dbus_generate_usr_word(int flg, const QString &wordText, const QString &wordCode)
{
    //    qDebug() << flg << wordText << wordCode;

    if (flg == 0) // 显示待造词组
    {
        m_inputWin->show_user_word_operation_prompt(1, wordText, wordCode);
    }
    else if (flg == 1) // 确认造词
    {
        m_usrGenWordDialog->add_user_word(wordText, wordCode);
        m_inputWin->close_user_word_operation_prompt();
        settings::instance().set_userWordFlg(1);
        m_kimAgent->ReloadConfig();
    }
    else if (flg == 2) // 取消造词
    {
        m_inputWin->close_user_word_operation_prompt();
    }
    else if (flg == 3) // 自定义词组编码
    {
        m_inputWin->close_user_word_operation_prompt();
        m_usrGenWordDialog->slot_show_dialog(wordText, wordCode);
    }
}

// 用户删词
void MainProgram::slot_dbus_delete_usr_word(int flg, const QString &wordText, const QString &wordCode)
{
    //    qDebug() << flg << wordText << wordCode;

    if (flg == 0) // 显示待删词组
    {
        m_inputWin->show_user_word_operation_prompt(0, wordText, wordCode);
    }
    else if (flg == 1) // 确认删词
    {
        m_usrGenWordDialog->delete_user_word(wordText, wordCode);
        m_inputWin->close_user_word_operation_prompt();
        settings::instance().set_userWordFlg(1);
        m_kimAgent->ReloadConfig();
    }
    else if (flg == 2) // 取消删词
    {
        m_inputWin->close_user_word_operation_prompt();
    }
}

// 输入法引擎载入用户词组文件完成
void MainProgram::slot_dbus_usr_word_load_ok()
{
    settings::instance().set_userWordFlg(0);
}

// 快捷码表加载完成
void MainProgram::slot_dbus_quick_table_load_ok()
{
    settings::instance().set_quickTableFlg(0);
}

// 切换虚拟键盘
void MainProgram::slot_dbus_switch_vk(int flg)
{
    m_virtualKeyboard->switch_vk(flg);
}

// 退出软键盘
void MainProgram::slot_dbus_close_vk()
{
    m_virtualKeyboard->slot_toggle_win();
}

// 切换字符集
void MainProgram::slot_dbus_switch_char_set()
{
    m_toolbar->switch_char_set();
}

// 切换简/繁体
void MainProgram::slot_dbus_switch_simp_or_trad()
{
    m_toolbar->set_traditional_mode(!ToolbarWin::is_traditional_mode());
}

// 切换大小写状态
void MainProgram::slot_dbus_switch_caps_state()
{

    m_toolbar->slot_update_input_mode_ico();
}

// 显/隐状态栏
void MainProgram::slot_dbus_switch_toolbar_hide_flg()
{

    bool flg = settings::instance().get_hideToolbar() ? false : true;

    if (flg)
    {
        m_toolbar->hide();
    }
    else
    {
        m_toolbar->show();
    }
    settings::instance().set_hideToolbar(flg);
}

// 显/隐候选框
void MainProgram::slot_dbus_switch_candiwin_hide_flg()
{
    bool flg = settings::instance().get_hideCandiWin() ? false : true;
    settings::instance().set_hideCandiWin(flg);
    g_settingsNotifier.notifySettingDataChangedToLocal();
}

// 切换词库
void MainProgram::slot_dbus_switch_lexicon()
{
    const std::vector<std::string> &lexiconList = freewb_runtime_lexicon_list();
    const std::string &curlexicon = settings::instance().get_curUsedLexicon();

    if (lexiconList.size() < 2)
        return;

    size_t i = 0;
    while (i < lexiconList.size())
    {
        if (lexiconList.at(i++) == curlexicon)
        {
            if (i >= lexiconList.size())
            {
                i = 0;
            }

            const std::string &lexicon = lexiconList.at(i);
            settings::instance().set_curUsedLexicon(lexicon);
            settings::instance().set_wubiTable(lexicon + "/freeime.mb");
            settings::instance().set_pinyinTable(lexicon + "/attach.mb");
            settings::instance().set_imeTableChanged(1);
            m_contextmenu->update_lexicon_checked_ico();
            m_kimAgent->ReloadConfig();
            break;
        }
    }
}

// 切换皮肤
void MainProgram::slot_dbus_switch_skin()
{
    const std::vector<std::string> &skinList = freewb_runtime_skin_list();
    const std::string &curSkin = settings::instance().get_curSkinId();

    if (skinList.size() < 2)
        return;

    size_t i = 0;
    while (i < skinList.size())
    {
        if (skinList.at(i++) == curSkin)
        {
            if (i >= skinList.size())
            {
                i = 0;
            }

            settings::instance().set_curSkinId(skinList.at(i));
            g_settingsNotifier.notifySettingDataChangedToLocal();
            break;
        }
    }
}

// 标点自动配对
void MainProgram::slot_dbus_set_mark_auto_pairs_flg(int flg)
{
    settings::instance().set_smartMark(flg != 0);
}

// 输入法词库加载完成
void MainProgram::slot_dbus_ime_table_load_ok()
{
    settings::instance().set_imeTableChanged(0);
}

void MainProgram::slot_dbus_set_charWidth_and_markMode(int charWidth, int markMode)
{
    if (charWidth)
    {
        m_toolbar->update_char_width_mode_ico(static_cast<CharWidthMode>(0));
    }

    if (markMode)
    {
        m_toolbar->update_mark_mode_ico(static_cast<MarkMode>(0));
    }
}

QString MainProgram::slot_dbus_get_clipboard_text()
{
    QClipboard *clipboard = QApplication::clipboard();
    FREEWB_DEBUG("clipboard: {}", clipboard->text().toUtf8().data());
    return clipboard->text();
}

void MainProgram::slot_dbus_open_freewb_dir() // 进入极点目录
{
    char cmd[128] = {0};

    sprintf(cmd, "xdg-open %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data());
    system(cmd);
}

void MainProgram::slot_dbus_word_freq_switch_ok(const QString &wordText, int flg) // 词组常用与非常用切换成功
{
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText(QString("【 %1 】已切换为%2词组").arg(wordText).arg(flg ? "非常用" : "常用"));
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
    msgBox->button(QMessageBox::Ok)->setText("确定(&OK)");
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
    delete msgBox;
}

void MainProgram::slot_dbus_open_ui_setting() // 进入设置
{
    m_settingWin->slot_open_win();
}

void MainProgram::slot_dbus_open_advanced_setting() // 进入专家设置模式
{
    m_textEditWin->slot_open_textEdit_win(TEM_SETTING_FILE);
}

void MainProgram::slot_dbus_edit_usr_table() // 编辑用户码表
{
    m_textEditWin->slot_open_textEdit_win(TEM_USER_WORD);
}

void MainProgram::slot_dbus_edit_wubi_table() // 编辑五笔码表
{
    m_textEditWin->slot_open_textEdit_win(TEM_WUBI_TABLE);
}

void MainProgram::slot_dbus_edit_pinyin_table() // 编辑拼音码表
{
    m_textEditWin->slot_open_textEdit_win(TEM_PINYIN_TABLE);
}

void MainProgram::slot_dbus_show_version_info() // 显示极点版本信息
{
    m_settingWin->slot_show_version_info();
}

void MainProgram::slot_dbus_edit_quick_table() // 编辑快捷码表
{
    m_textEditWin->slot_open_textEdit_win(TEM_QUICK_TABLE);
}

void MainProgram::slot_dbus_set_recode_calib_flg(int flg) // 切换重码上屏校对模式
{
    settings::instance().set_recodeCalib(flg != 0);
}

/*****************************************************************************************************/
