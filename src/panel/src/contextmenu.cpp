#include "contextmenu.h"

#include <vector>

#include <QWindow>

#include "config.h"
#include "settings.h"
#include "settingshelper.h"
#include "waylandwinhelper.h"

// 一级菜单
#define STR_MENU1 _("Input method settings")
#define STR_MENU2 _("Management tools")
#define STR_ACTION3 _("Manual word creation")
#define STR_MENU4 _("Usage instructions")

// 二级菜单
#define STR_ACTION11 _("Graphical settings mode")
#define STR_ACTION12 _("Expert settings mode")
#define STR_ACTION13 _("Class Wangma settings")
#define STR_ACTION14 _("Freewb input mode")
#define STR_ACTION15 _("Restore default settings")

#define STR_ACTION21 _("Edit user word group")
#define STR_ACTION22 _("Edit shortcut code table")
#define STR_MENU21 _("Lexicon tool")
#define STR_MENU22 _("Switch lexicon")

#define STR_ACTION31 _("Quick start")
#define STR_ACTION32 _("Shortcut command")
#define STR_ACTION33 _("Version information")
#define STR_ACTION34 _("Software registration")

// 三级菜单
#define STR_ACTION211 _("Lexicon generation and maintenance")
#define STR_ACTION212 _("Backup lexicon and settings")
#define STR_ACTION213 _("Restore lexicon and settings")
#define STR_ACTION222 _("Current lexicon information")

#define ICO_CHECKED ":/image/toolbar/checked.png"

#define QSS_MENU                                                                                                                 \
    "QMenu::item{color: rgb(56, 56, 56);}"                                                                                       \
    "QMenu::item:selected:enabled{background-color:#DFDFDF;color: rgb(56, 56, 56)}"                                              \
    "QMenu::item:!enabled{color: rgba(56, 56, 56, 128);}"

//"QMenu::icon:unchecked{border-image: url(:/image/toolbar/checked.png);}"

ContextMenu::ContextMenu(QWidget *parent) : QMenu(parent)
{
    setStyleSheet(QSS_MENU);
    m_menu1.setStyleSheet(QSS_MENU);
    m_menu2.setStyleSheet(QSS_MENU);
    m_menu21.setStyleSheet(QSS_MENU);
    m_menu22.setStyleSheet(QSS_MENU);
    m_menu4.setStyleSheet(QSS_MENU);
    m_action13.setEnabled(false);
    m_action14.setEnabled(false);
    m_action221.setEnabled(false);
    m_action222.setEnabled(false);

    // 一级菜单
    m_menu1.setTitle(STR_MENU1);
    m_menu2.setTitle(STR_MENU2);
    m_action3.setText(STR_ACTION3);
    m_menu4.setTitle(STR_MENU4);
    addMenu(&m_menu1);
    addMenu(&m_menu2);
    addAction(&m_action3);
    addMenu(&m_menu4);

    // 二级菜单
    m_action11.setText(STR_ACTION11);
    m_action12.setText(STR_ACTION12);
    m_action13.setText(STR_ACTION13);
    m_action14.setText(STR_ACTION14);
    m_action15.setText(STR_ACTION15);
    m_menu1.addAction(&m_action11);
    m_menu1.addAction(&m_action12);
    m_menu1.addAction(&m_action15);

    m_action21.setText(STR_ACTION21);
    m_action22.setText(STR_ACTION22);
    m_menu21.setTitle(STR_MENU21);
    m_menu22.setTitle(STR_MENU22);
    m_menu2.addAction(&m_action21);
    m_menu2.addAction(&m_action22);
    m_menu2.addMenu(&m_menu21);
    m_menu2.addMenu(&m_menu22);

    m_action31.setText(STR_ACTION31);
    m_action32.setText(STR_ACTION32);
    m_action33.setText(STR_ACTION33);
    m_action34.setText(STR_ACTION34);
    m_menu4.addAction(&m_action31);
    m_menu4.addAction(&m_action32);
    m_menu4.addAction(&m_action33);
    m_menu4.addAction(&m_action34);

    // 三级菜单
    m_action211.setText(STR_ACTION211);
    m_action212.setText(STR_ACTION212);
    m_action213.setText(STR_ACTION213);
    m_menu21.addAction(&m_action211);
    m_menu21.addAction(&m_action212);
    m_menu21.addAction(&m_action213);

    m_actGrpLexicon = new QActionGroup(this);
    slot_update_lexicon_list();

    connect(&m_action3, SIGNAL(triggered()), this, SLOT(on_action3_clicked()));
    connect(&m_action11, SIGNAL(triggered()), this, SLOT(on_action11_clicked()));
    connect(&m_action12, SIGNAL(triggered()), this, SLOT(on_action12_clicked()));
    connect(&m_action13, SIGNAL(triggered()), this, SLOT(on_action13_clicked()));
    connect(&m_action14, SIGNAL(triggered()), this, SLOT(on_action14_clicked()));
    connect(&m_action15, SIGNAL(triggered()), this, SLOT(on_action15_clicked()));
    connect(&m_action21, SIGNAL(triggered()), this, SLOT(on_action21_clicked()));
    connect(&m_action22, SIGNAL(triggered()), this, SLOT(on_action22_clicked()));
    connect(&m_action31, SIGNAL(triggered()), this, SLOT(on_action31_clicked()));
    connect(&m_action32, SIGNAL(triggered()), this, SLOT(on_action32_clicked()));
    connect(&m_action33, SIGNAL(triggered()), this, SLOT(on_action33_clicked()));
    connect(&m_action34, SIGNAL(triggered()), this, SLOT(on_action34_clicked()));
    connect(&m_action211, SIGNAL(triggered()), this, SLOT(on_action211_clicked()));
    connect(&m_action212, SIGNAL(triggered()), this, SLOT(on_action212_clicked()));
    connect(&m_action213, SIGNAL(triggered()), this, SLOT(on_action213_clicked()));

    connect(&m_menu22, SIGNAL(aboutToShow()), this, SLOT(slot_update_lexicon_list()));
    connect(m_actGrpLexicon, SIGNAL(triggered(QAction *)), this, SLOT(on_actionGrpLexicon_clicked(QAction *)));
    connect(&m_action222, SIGNAL(triggered()), this, SLOT(on_action222_clicked()));
    connect(this, &QMenu::aboutToShow, this, [this]() { emit signal_menu_visibility_changed(true); });
    connect(this, &QMenu::aboutToHide, this, [this]() { emit signal_menu_visibility_changed(false); });

    freewb::applyWaylandOverlayWindowHints(this);
}

ContextMenu::~ContextMenu()
{
}

void ContextMenu::slot_show_context_menu()
{
    QPoint pos = QCursor::pos();

    if (QWidget *transientParent = qobject_cast<QWidget *>(sender()))
    {
        transientParent->winId();
        winId();
        QWindow *parentWindow = transientParent->windowHandle();
        QWindow *menuWindow = windowHandle();
        if (parentWindow && menuWindow)
        {
            menuWindow->setTransientParent(parentWindow);
        }
    }
    popup(pos);
}

void ContextMenu::on_action3_clicked()
{
    close();
    emit signal_open_user_word_dialog("", "");
}

void ContextMenu::on_action11_clicked()
{
    close();
    emit signal_open_setting_win();
}

void ContextMenu::on_action12_clicked()
{
    close();
    emit signal_open_textEdit_win(TEM_SETTING_FILE);
}

void ContextMenu::on_action13_clicked()
{
}

void ContextMenu::on_action14_clicked()
{
}

void ContextMenu::on_action15_clicked()
{
    close();
    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(_("Are you sure you want to restore Freewb to the default state?"));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
    msgBox->setDefaultButton(QMessageBox::Yes);

    int ret = msgBox->exec();
    delete msgBox;
    if (ret == QMessageBox::Yes)
    {
        settings::instance().restoreAllDefaults();
        (void)settings::instance().save();

        // 同步配置变更到输入法引擎与本地UI
        g_settingsNotifier.notifySettingDataChangedToFcitx();
        g_settingsNotifier.notifySettingDataChangedToLocal();

        emit signal_restore_all_settings();
    }
}

void ContextMenu::on_action21_clicked()
{
    close();
    emit signal_open_textEdit_win(TEM_USER_WORD);
}

void ContextMenu::on_action22_clicked()
{
    close();
    emit signal_open_textEdit_win(TEM_QUICK_TABLE);
}

void ContextMenu::on_action31_clicked()
{
    close();
    // system( "firefox ~/.local/freewb/help/help.html > /dev/null 2>&1 &" );
    QDesktopServices::openUrl(QUrl(INSTALL_DIR + "/help/help.html"));
}

void ContextMenu::on_action32_clicked()
{
    close();
    QString shortcutCmdInfo = ""
                              "命令     功能说明\n\n"
                              "----------------------------\n"
                              "dos.   进入极点目录\n"
                              "aa.    在线加词\n"
                              "dd.    在线删词\n"
                              "ss.    转换字词常用状态\n"
                              "jj.    切换简入繁出模式\n"
                              "ff.    查询字词编码及拼音\n"
                              "hh.    显/隐状态栏\n"
                              "kk.    打开软键盘\n"
                              "mm.    切换字符集\n"
                              "oo.    进入图形模式设置\n"
                              "pp.    进入专家模式设置\n"
                              "uu.    编辑用户码表\n"
                              "uw.    编辑五笔码表\n"
                              "up.    编辑拼音码表\n "
                              "vv.    显示极点版本信息\n"
                              "qq.    编辑快捷码表\n"
                              "tt.    切换重码上屏校对模式\n\n"
                              "注：字母后面的.是英文句号\n"
                              "【当临时英文键禁用时上述命令失效】\n";

    QMessageBox *msgBox = new QMessageBox();
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText(shortcutCmdInfo);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
    msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&OK)"));
    msgBox->setDefaultButton(QMessageBox::Ok);

    msgBox->exec();
    delete msgBox;
}

void ContextMenu::on_action33_clicked()
{
    close();
    emit signal_show_version_info();
}

void ContextMenu::on_action34_clicked()
{
    close();
    emit signal_app_register();
}

void ContextMenu::on_action211_clicked()
{
    close();
    emit signal_open_lexicon_tool();
}

void ContextMenu::on_action212_clicked()
{
    close();
    emit signal_backup_lexicon_and_settings();
}

void ContextMenu::on_action213_clicked()
{
    close();
    emit signal_restore_lexicon_and_settings();
}

void ContextMenu::on_actionGrpLexicon_clicked(QAction *action)
{
    close();
    if (toQStringUtf8(settings::instance().get_curUsedLexicon()) != action->text())
    {
        const std::string lexicon = fromStdUtf8(action->text());
        settings::instance().set_curUsedLexicon(lexicon);
        settings::instance().set_wubiTable(lexicon + "/wbzx.mb");
        settings::instance().set_pinyinTable(lexicon + "/pinyin.mb");
        settings::instance().set_wubiTableChanged(1);
        settings::instance().set_pinyinTableChanged(1);
        emit signal_ime_table_changed();

        update_lexicon_checked_ico();
    }
}

void ContextMenu::on_action222_clicked()
{
    close();
}

void ContextMenu::slot_update_lexicon_list()
{
    QString lexiconDir = INSTALL_DIR + "/data/mb/";
    QStringList dirList = QDir(lexiconDir).entryList(QDir::Dirs);
    dirList.removeOne(".");
    dirList.removeOne("..");

    QList<QAction *> acts = m_actGrpLexicon->actions();
    foreach(QAction * act, acts)
    {
        m_actGrpLexicon->removeAction(act);
    }

    QStringList lexiconList;
    foreach(QString lexiconId, dirList)
    {
        if (QFile(lexiconDir + lexiconId + "/wbzx.mb").exists() && QFile(lexiconDir + lexiconId + "/pinyin.mb").exists())
        {
            lexiconList << lexiconId;
            QAction *act = new QAction(lexiconId, this);
            m_actGrpLexicon->addAction(act);
        }
    }
    std::vector<std::string> lexiconVec;
    lexiconVec.reserve(static_cast<size_t>(lexiconList.size()));
    for (const QString &id : lexiconList)
        lexiconVec.push_back(fromStdUtf8(id));
    (void)freewb_runtime_set_lexicon_list(lexiconVec);

    update_lexicon_checked_ico();

    m_menu22.clear();
    m_menu22.addActions(m_actGrpLexicon->actions());
    m_action222.setText(STR_ACTION222);
    m_menu22.addAction(&m_action222);
}

void ContextMenu::update_lexicon_checked_ico()
{
    m_curUsedLexicon = toQStringUtf8(settings::instance().get_curUsedLexicon());

    QList<QAction *> acts = m_actGrpLexicon->actions();
    foreach(QAction * act, acts)
    {
        if (act->text() == m_curUsedLexicon)
        {
            act->setIcon(QIcon(ICO_CHECKED));
        }
        else
        {
            act->setIcon(QIcon());
        }
    }
}
