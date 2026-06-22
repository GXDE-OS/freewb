#include "mainprogram.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMessageBox>

#include "config.h"
#include "ipc.h"
#include "log.h"
#include "settings.h"
#include "settingshelper.h"
#include "types.h"

MainProgram::MainProgram(QObject *parent) : QObject(parent)
{
    m_x11EventMonitor = new X11EventMonitor(this);
    m_x11EventMonitor->start();

    m_panelDBusService = new freewb::ipc::QDBusPanelService(this);
    m_settingsDBusService = new freewb::ipc::QDBusSettingsService(this);

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

    m_toolbar->set_context_menu(m_contextmenu);

    connectPanelDBus();
    connectSettingsDBus();

    initFcitxServiceWatcher();

    m_panelDBusService->registerQDBusService();

    m_panelDBusService->ReloadConfig();
    m_panelDBusService->ReloadDictionaries(freewb::DictReloadAll);
}

MainProgram::~MainProgram()
{
    delete m_contextmenu;
    delete m_toolbar;
    delete m_virtualKeyboard;
    delete m_inputWin;
    delete m_settingWin;
    delete m_lexicontoolWin;
    delete m_usrGenWordDialog;
    delete m_textEditWin;
    delete m_dictQueryWin;
    delete m_backupDialog;
}

void MainProgram::connectPanelDBus()
{
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_ShowLookupTable, m_inputWin,
            &InputWin::slot_kim_ShowLookupTable);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdateLookupTable, m_inputWin,
            &InputWin::slot_kim_UpdateLookupTable);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_SetLookupTable, m_inputWin,
            &InputWin::slot_kim_SetLookupTable);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdatePreeditCaret, m_inputWin,
            &InputWin::slot_kim_UpdatePreeditCaret);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdatePreeditText, m_inputWin,
            &InputWin::slot_kim_UpdatePreeditText);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdateAux, m_inputWin, &InputWin::slot_kim_UpdateAux);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdateSpotLocation, m_inputWin,
            &InputWin::slot_kim_UpdateSpotLocation);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_SetSpotLocation, m_inputWin,
            &InputWin::slot_kim_SetSpotLocation);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_UpdateProperty, m_toolbar,
            &ToolbarWin::slot_kim_UpdateProperty);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_RegisterProperties, m_toolbar,
            &ToolbarWin::slot_kim_RegisterProperties);

    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_switch_input_mode, this,
            [this](const QString &inputMode)
            {
                if (inputMode == ToolbarWin::get_input_mode())
                {
                    return;
                }
                m_virtualKeyboard->switch_caps_flg(0);
                ToolbarWin::set_input_mode(inputMode);
                m_toolbar->slot_update_input_mode_ico();
            });
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_switch_char_set, m_toolbar, &ToolbarWin::switch_char_set);
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_switch_simp_or_trad, m_toolbar,
            [this]() { m_toolbar->set_traditional_mode(!ToolbarWin::is_traditional_mode()); });
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_switch_char_width, m_toolbar,
            [this]() { m_toolbar->update_char_width_mode_ico(ToolbarWin::get_char_width_mode()); });
    connect(m_panelDBusService, &freewb::ipc::QDBusPanelService::signal_switch_punctuation_mode, m_toolbar,
            [this]() { m_toolbar->update_mark_mode_ico(ToolbarWin::get_mark_mode()); });

    connect(m_inputWin, &InputWin::signal_candidate_select, m_panelDBusService, &freewb::ipc::QDBusPanelService::SelectCandidate);
    connect(m_inputWin, &InputWin::signal_candidate_page_up, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::LookupTablePageUp);
    connect(m_inputWin, &InputWin::signal_candidate_page_down, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::LookupTablePageDown);
    connect(m_inputWin, &InputWin::signal_btn_charWidth_clicked, m_toolbar, &ToolbarWin::on_btnCharWidth_clicked);
    connect(m_inputWin, &InputWin::signal_btn_mark_clicked, m_toolbar, &ToolbarWin::on_btnMark_clicked);
    connect(m_inputWin, &InputWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu);

    connect(m_toolbar, &ToolbarWin::signal_request_next_input_mode, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::RequestNextInputMode);
    connect(m_toolbar, &ToolbarWin::signal_fcitx_switch_char_width, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::SwitchFullWidth);
    connect(m_toolbar, &ToolbarWin::signal_fcitx_switch_mark, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::SwitchPunctuation);
    connect(m_toolbar, &ToolbarWin::signal_switch_chttrans, m_panelDBusService, &freewb::ipc::QDBusPanelService::SwitchChttrans);
    connect(m_toolbar, &ToolbarWin::signal_switch_char_set, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::SwitchCharSetMode);
    connect(m_toolbar, &ToolbarWin::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win);
    connect(m_toolbar, &ToolbarWin::signal_open_context_menu, m_contextmenu, &ContextMenu::slot_show_context_menu);
    connect(m_toolbar, &ToolbarWin::signal_toggle_vk, m_virtualKeyboard, &Keyboard::slot_toggle_win);
    connect(m_toolbar, &ToolbarWin::signal_open_vk, m_virtualKeyboard, &Keyboard::slot_open_win);
    connect(m_toolbar, &ToolbarWin::signal_btn_charWidth_clicked, m_inputWin, &InputWin::slot_update_charWidth_btn_ico);
    connect(m_toolbar, &ToolbarWin::signal_btn_mark_clicked, m_inputWin, &InputWin::slot_update_mark_btn_ico);
    connect(m_toolbar, &ToolbarWin::signal_open_generate_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog);
    connect(m_toolbar, &ToolbarWin::signal_open_dict_query_win, m_dictQueryWin, &DictQueryWin::slot_open_win);

    connect(m_virtualKeyboard, &Keyboard::signal_vk_flg_changed, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadConfig);
    connect(m_virtualKeyboard, &Keyboard::signal_kb_caps_changed, m_toolbar, &ToolbarWin::slot_kb_caps_changed);

    connect(m_contextmenu, &ContextMenu::signal_reload_dictionaries, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadDictionaries);
    connect(m_contextmenu, &ContextMenu::signal_open_lexicon_tool, m_lexicontoolWin, &LexiconToolWin::open_win);
    connect(m_contextmenu, &ContextMenu::signal_open_setting_win, m_settingWin, &SettingWin::slot_open_win);
    connect(m_contextmenu, &ContextMenu::signal_restore_all_settings, m_settingWin, &SettingWin::slot_init_all_setting_page);
    connect(m_contextmenu, &ContextMenu::signal_show_version_info, m_settingWin, &SettingWin::slot_show_version_info);
    connect(m_contextmenu, &ContextMenu::signal_open_textEdit_win, m_textEditWin, &TextEditWin::slot_open_textEdit_win);
    connect(m_contextmenu, &ContextMenu::signal_open_user_word_dialog, m_usrGenWordDialog, &UsrGenWordDialog::slot_show_dialog);
    connect(m_contextmenu, &ContextMenu::signal_backup_lexicon_and_settings, m_backupDialog,
            &BackupDialog::slot_backup_lexicon_and_settings);
    connect(m_contextmenu, &ContextMenu::signal_restore_lexicon_and_settings, m_backupDialog,
            &BackupDialog::slot_restore_lexicon_and_settings);

    connect(m_x11EventMonitor, &X11EventMonitor::signal_key_clicked, m_virtualKeyboard, &Keyboard::slot_key_clicked);

    connect(m_usrGenWordDialog, &UsrGenWordDialog::signal_user_word_changed, m_panelDBusService,
            [this]() { m_panelDBusService->ReloadDictionaries(freewb::DictReloadUserWord); });
    connect(m_usrGenWordDialog, &UsrGenWordDialog::signal_commit_user_word, this,
            [this](const QString &wordText, const QString &wordCode)
            { m_panelDBusService->commitUserWordAdd(wordCode, wordText); });

    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadConfig);
    connect(m_textEditWin, &TextEditWin::signal_quickTable_file_saved, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadConfig);
    connect(m_textEditWin, &TextEditWin::signal_reload_dictionaries, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadDictionaries);
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_toolbar, &ToolbarWin::slot_load_setting_data);
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_virtualKeyboard, &Keyboard::slot_load_setting_data);
    connect(m_textEditWin, &TextEditWin::signal_setting_file_changed, m_inputWin, &InputWin::slot_load_setting_data);
    connect(m_textEditWin, &TextEditWin::signal_userWord_file_saved, m_usrGenWordDialog,
            &UsrGenWordDialog::slot_userWord_file_saved);

    connect(m_lexicontoolWin, &LexiconToolWin::signal_reload_dictionaries, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadDictionaries);
    connect(m_backupDialog, &BackupDialog::signal_restore_lexicon_and_settings_ok, this,
            [this]()
            {
                m_panelDBusService->ReloadConfig();
                m_panelDBusService->ReloadDictionaries(freewb::DictReloadAll);
            });

    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_fcitx, m_panelDBusService,
            &freewb::ipc::QDBusPanelService::ReloadConfig);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_toolbar,
            &ToolbarWin::slot_load_setting_data);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_virtualKeyboard,
            &Keyboard::slot_load_setting_data);
    connect(&g_settingsNotifier, &SettingsNotifier::signal_setting_data_changed_to_local, m_inputWin,
            &InputWin::slot_load_setting_data);
}

void MainProgram::connectSettingsDBus()
{
    auto *s = m_settingsDBusService;

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_input_mode, this,
            [this](const QString &inputMode)
            {
                if (inputMode == ToolbarWin::get_input_mode())
                {
                    return;
                }
                m_virtualKeyboard->switch_caps_flg(0);
                ToolbarWin::set_input_mode(inputMode);
                m_toolbar->slot_update_input_mode_ico();
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_dict_query, m_dictQueryWin, &DictQueryWin::slot_open_win);

    connect(s, &freewb::ipc::QDBusSettingsService::signal_generate_usr_word, this,
            [this](int flg, const QString &wordText, const QString &wordCode)
            {
                if (flg == 0)
                {
                    m_inputWin->show_user_word_operation_prompt(1, wordText, wordCode);
                }
                else if (flg == 1)
                {
                    m_inputWin->close_user_word_operation_prompt();
                }
                else if (flg == 2)
                {
                    m_inputWin->close_user_word_operation_prompt();
                }
                else if (flg == 3)
                {
                    m_inputWin->close_user_word_operation_prompt();
                    m_usrGenWordDialog->slot_show_dialog_online(wordText, wordCode);
                }
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_delete_usr_word, this,
            [this](int flg, const QString &wordText, const QString &wordCode)
            {
                if (flg == 0)
                {
                    m_inputWin->show_user_word_operation_prompt(0, wordText, wordCode);
                }
                else if (flg == 1)
                {
                    m_inputWin->close_user_word_operation_prompt();
                }
                else if (flg == 2)
                {
                    m_inputWin->close_user_word_operation_prompt();
                }
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_panel_exit, this,
            [this]()
            {
                m_toolbar->hide();
                m_inputWin->hide();
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_vk, m_virtualKeyboard, &Keyboard::switch_vk);
    connect(s, &freewb::ipc::QDBusSettingsService::signal_close_vk, m_virtualKeyboard, &Keyboard::slot_toggle_win);
    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_char_set, m_toolbar, &ToolbarWin::switch_char_set);
    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_simp_or_trad, m_toolbar,
            [this]() { m_toolbar->set_traditional_mode(!ToolbarWin::is_traditional_mode()); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_caps_state, m_toolbar, &ToolbarWin::slot_update_input_mode_ico);

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_toolbar_hide_flg, this,
            [this]()
            {
                const bool hide = !settings::instance().get_hideToolbar();
                settings::instance().set_hideToolbar(hide);
                settings::instance().save();
                g_settingsNotifier.notifySettingDataChangedToLocal();
                if (!hide)
                {
                    m_toolbar->show();
                }
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_lexicon, this,
            [this]()
            {
                const std::vector<std::string> &lexiconList = freewb_runtime_lexicon_list();
                const std::string &curlexicon = settings::instance().get_curUsedLexicon();
                if (lexiconList.size() < 2)
                {
                    return;
                }
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
                        settings::instance().set_wubiTable(lexicon + "/wbzx.mb");
                        settings::instance().set_pinyinTable(lexicon + "/pinyin.mb");
                        (void)settings::instance().save();
                        m_contextmenu->update_lexicon_checked_ico();
                        m_panelDBusService->ReloadDictionaries(freewb::DictReloadMainTables);
                        break;
                    }
                }
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_skin, this,
            []()
            {
                const std::vector<std::string> &skinList = freewb_runtime_skin_list();
                const std::string &curSkin = settings::instance().get_curSkinId();
                if (skinList.size() < 2)
                {
                    return;
                }
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
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_mark_auto_pairs_flg, this,
            []()
            {
                const bool enabled = !settings::instance().get_smartMark();
                settings::instance().set_smartMark(enabled);
                settings::instance().save();
                g_settingsNotifier.notifySettingDataChangedToLocal();
            });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_set_recode_calib_flg, this,
            [](int flg) { settings::instance().set_recodeCalib(flg != 0); });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_char_width, m_toolbar,
            [this]() { m_toolbar->update_char_width_mode_ico(ToolbarWin::get_char_width_mode()); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_switch_punctuation_mode, m_toolbar,
            [this]() { m_toolbar->update_mark_mode_ico(ToolbarWin::get_mark_mode()); });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_open_freewb_dir, this,
            []()
            {
                char cmd[128] = {0};
                sprintf(cmd, "xdg-open %s > /dev/null 2>&1 &", QString(INSTALL_DIR).toUtf8().data());
                system(cmd);
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_word_freq_switch_ok, this,
            [](const QString &wordText, int flg)
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
            });

    connect(s, &freewb::ipc::QDBusSettingsService::signal_open_ui_setting, m_settingWin, &SettingWin::slot_open_win);
    connect(s, &freewb::ipc::QDBusSettingsService::signal_open_advanced_setting, this,
            [this]() { m_textEditWin->slot_open_textEdit_win(TEM_SETTING_FILE); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_edit_usr_table, this,
            [this]() { m_textEditWin->slot_open_textEdit_win(TEM_USER_WORD); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_edit_wubi_table, this,
            [this]() { m_textEditWin->slot_open_textEdit_win(TEM_WUBI_TABLE); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_edit_pinyin_table, this,
            [this]() { m_textEditWin->slot_open_textEdit_win(TEM_PINYIN_TABLE); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_edit_quick_table, this,
            [this]() { m_textEditWin->slot_open_textEdit_win(TEM_QUICK_TABLE); });
    connect(s, &freewb::ipc::QDBusSettingsService::signal_show_version_info, m_settingWin, &SettingWin::slot_show_version_info);
}

void MainProgram::initFcitxServiceWatcher()
{
    static constexpr char kFcitx5Service[] = "org.fcitx.Fcitx5";
    static constexpr char kFcitx4ServicePrefix[] = "org.fcitx.Fcitx-";

    auto *iface = QDBusConnection::sessionBus().interface();
    connect(iface, &QDBusConnectionInterface::serviceOwnerChanged, this,
            [this](const QString &serviceName, const QString &oldOwner, const QString &newOwner)
            {
                const bool isFcitx5 = serviceName == QLatin1String(kFcitx5Service);
                const bool isFcitx4 = serviceName.startsWith(QLatin1String(kFcitx4ServicePrefix));
                if (!isFcitx5 && !isFcitx4)
                {
                    return;
                }
                if (!oldOwner.isEmpty() && newOwner.isEmpty())
                {
                    FREEWB_ERROR("fcitx service lost (name: {}, oldOwner: {}, newOwner: {}), reset and hide panel ui",
                                 serviceName.toUtf8().constData(), oldOwner.toUtf8().constData(), newOwner.toUtf8().constData());
                    m_toolbar->reset();
                    m_toolbar->hide();
                    m_inputWin->reset();
                    m_inputWin->hide();
                }
            });
}
