#include "contextmenu.h"

#include <vector>

#include "commdefine.h"
#include "settings.h"
#include "settingshelper.h"

// 一级菜单
#define STR_MENU1 "输入法设置"
#define STR_MENU2 "管理工具"
#define STR_ACTION3 "手工造词"
#define STR_MENU4 "使用说明"

// 二级菜单
#define STR_ACTION11 "图形设置模式"
#define STR_ACTION12 "专家设置模式"
#define STR_ACTION13 "类王码设置"
#define STR_ACTION14 "极点输入模式"
#define STR_ACTION15 "恢复默认设置"

#define STR_ACTION21 "编辑用户词组"
#define STR_ACTION22 "编辑快捷码表"
#define STR_MENU21 "词库工具"
#define STR_MENU22 "切换词库"

#define STR_ACTION31 "快速入门"
#define STR_ACTION32 "快捷命令"
#define STR_ACTION33 "版本信息"
#define STR_ACTION34 "软件注册"

// 三级菜单
#define STR_ACTION211 "词库生成与维护"
#define STR_ACTION212 "备份词库与设置"
#define STR_ACTION213 "恢复词库与设置"

#define STR_ACTION222 "现用词库信息"

#define ICO_CHECKED ":/image/toolbar/checked.png"

#define QSS_MENU                                                                                                                                                                                                                                                                                           \
    "QMenu::item{color: rgb(56, 56, 56);}"                                                                                                                                                                                                                                                                 \
    "QMenu::item:selected:enabled{background-color:#DFDFDF;color: rgb(56, 56, 56)}"                                                                                                                                                                                                                        \
    "QMenu::item:!enabled{color: rgba(56, 56, 56, 128);}"

//"QMenu::icon:unchecked{border-image: url(:/image/toolbar/checked.png);}"

ContextMenu::ContextMenu(QWidget *parent) : QMenu(parent)
{
    Qt::WindowFlags winflgs = Qt::WindowDoesNotAcceptFocus | Qt::X11BypassWindowManagerHint | Qt::Tool | Qt::WindowStaysOnTopHint;
    setWindowFlags(winflgs);
    m_menu1.setWindowFlags(winflgs);
    m_menu2.setWindowFlags(winflgs);
    m_menu4.setWindowFlags(winflgs);
    m_menu21.setWindowFlags(winflgs);
    m_menu22.setWindowFlags(winflgs);

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
    if (g_cpuType == CT_ARM)
    {
        m_menu4.addAction(&m_action34);
    }

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
}

ContextMenu::~ContextMenu()
{
}

void ContextMenu::slot_show_context_menu()
{
    // qDebug() << "show";
    QDesktopWidget *d = QApplication::desktop();
    QPoint pos = QCursor::pos();
    move(pos);
    show();

    if (pos.x() + size().width() > d->width())
    {
        pos.setX(pos.x() - size().width());
        move(pos);
    }
    if (pos.y() + size().height() > d->height())
    {
        pos.setY(pos.y() - size().height());
        move(pos);
    }
}

void ContextMenu::slot_button_pressed(int button)
{
    Q_UNUSED(button);
    if (isVisible())
    {
        QPoint p = QCursor::pos();
        if (!geometry().adjusted(-5, -5, 5, 5).contains(p) && !(m_menu1.geometry().contains(p) && m_menu1.isVisible()) && !(m_menu2.geometry().contains(p) && m_menu2.isVisible()) && !(m_menu4.geometry().contains(p) && m_menu4.isVisible()) &&
            !(m_menu21.geometry().contains(p) && m_menu21.isVisible()) && !(m_menu22.geometry().contains(p) && m_menu22.isVisible()))
        {
#ifdef DEBUG
// qDebug() << "close";
#endif
            close();
        }
    }
}

void ContextMenu::slot_key_pressed(int key)
{
    Q_UNUSED(key);
    if (isVisible())
    {
        close();
    }
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
    msgBox->setText("确定将极点恢复到默认状态吗？");
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText("否(&N)");
    msgBox->setDefaultButton(QMessageBox::Yes);

    int ret = msgBox->exec();
    delete msgBox;
    if (ret == QMessageBox::Yes)
    {
        settings::instance().restoreAllDefaults();
        (void)settings::instance().save();

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
                              "cc.    显/隐候选窗\n"
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
    msgBox->button(QMessageBox::Ok)->setText("确定(&OK)");
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
        settings::instance().set_wubiTable(lexicon + "/freeime.mb");
        settings::instance().set_pinyinTable(lexicon + "/attach.mb");
        settings::instance().set_imeTableChanged(1);
        emit signal_ime_table_changed();

        update_lexicon_checked_ico();
    }
}

void ContextMenu::on_action222_clicked()
{
    close();
    //    qDebug() << "";;
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
        if (QFile(lexiconDir + lexiconId + "/freeime.mb").exists() && QFile(lexiconDir + lexiconId + "/attach.mb").exists())
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
