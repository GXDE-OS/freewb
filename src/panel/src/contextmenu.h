/**************************************************************************************
** @作者：lcj
**
** @说明：
**      ContextMenu是一个从QMenu类继承而来的UI类，但其通过主要包含一个QMenu类对象可独立于其它窗口
**      显示出固定的右键多级弹出菜单，该类设计目的为被MainProgram所包含实例化，当鼠标在桌面工具条和
**      输入候选框上点击右键时可弹出统一的输入法菜单。
*************************************************************************************/

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QApplication>
#include <QWidget>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QAbstractButton>
#include <QActionGroup>
#include <QDesktopWidget>
#include <QDesktopServices>

#include "texteditwin.h"

class ContextMenu : public QMenu
{
    Q_OBJECT

public:
    explicit ContextMenu( QWidget *parent = nullptr );
    ~ContextMenu();

signals:
    void signal_open_setting_win();//图形界面设置action点击
    void signal_open_user_word_dialog( const QString &wordText, const QString &wordCode );//手工造词
    void signal_restore_all_settings();//恢复所有默认设置action点击
    void signal_open_lexicon_tool();//打开词库工具箱
    void signal_open_textEdit_win( TextEditMode mode );//打开文本编辑框
    void signal_backup_lexicon_and_settings();//备份词库与设置
    void signal_restore_lexicon_and_settings();//恢复词库与设置
    void signal_show_version_info();//查看版本信息
    void signal_ime_table_changed();
    void signal_app_register();

public slots:
    void slot_key_pressed( int key );
    void slot_button_pressed( int button );
    void slot_show_context_menu();

public:
    void update_lexicon_checked_ico();

protected slots:
    //右键菜单按钮点击执行函数
    void on_action3_clicked();
    void on_action11_clicked();
    void on_action12_clicked();
    void on_action13_clicked();
    void on_action14_clicked();
    void on_action15_clicked();
    void on_action21_clicked();
    void on_action22_clicked();
    void on_action31_clicked();
    void on_action32_clicked();
    void on_action33_clicked();
    void on_action34_clicked();
    void on_action211_clicked();
    void on_action212_clicked();
    void on_action213_clicked();
    void on_actionGrpLexicon_clicked( QAction *act );
    void on_action222_clicked();

    void slot_update_lexicon_list();

private:
    //一级菜单
    QMenu m_menu1;          //输入法设置
    QMenu m_menu2;          //管理工具
    QAction m_action3;      //手工造词
    QMenu m_menu4;          //使用说明

    // 二级菜单
    QAction m_action11;     //图形设置模式
    QAction m_action12;     //专家设置模式
    QAction m_action13;     //类王码设置
    QAction m_action14;     //极点输入模式
    QAction m_action15;     //恢复默认设置

    QAction m_action21;     //编辑用户词组
    QAction m_action22;     //编辑快捷码表
    QMenu m_menu21;         //词库工具
    QMenu m_menu22;         //切换词库

    QAction m_action31;     //快速入门
    QAction m_action32;     //快捷命令
    QAction m_action33;     //版本信息
    QAction m_action34;     //软件注册

    // 三级菜单
    QAction m_action211;     //词库生成与维护
    QAction m_action212;     //备份词库与设置
    QAction m_action213;     //恢复词库与设置


    QAction m_action221;     //default
    QAction m_action222;     //现用词库信息

    QActionGroup *m_actGrpLexicon;
    QString m_curUsedLexicon;
};

#endif
