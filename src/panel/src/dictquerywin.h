#ifndef DICTQUERYWIN_H
#define DICTQUERYWIN_H

#include <QAbstractButton>
#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPoint>

#include "dictquery.h"

namespace Ui
{
class DictQueryWin;
}

class DictQueryWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit DictQueryWin(QWidget *parent = nullptr);
    ~DictQueryWin();

public slots:
    void slot_open_win(const QString &queryText);

protected:
    void update_candidate_word_list(const QStringList &wordList);
    void find_word(const QString &text);
    void find_word_list(const QString &text);

    bool eventFilter(QObject *obj, QEvent *event);

protected slots:
    void slot_action_cut();
    void slot_action_copy();
    void slot_action_paste();
    void slot_action_select_copy_all();
    void slot_action_save_custom_word();
    void slot_action_del_custom_word();

private slots:
    void on_btnQuery_clicked();
    void on_btnExit_clicked();
    void on_listWidgetCandi_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void setUiTexts();

private:
    Ui::DictQueryWin *ui;

    QPoint m_defaultPopPosition;

    QMenu *m_menuEdit;             // 编辑菜单
    QAction *m_actCut;             // 剪切
    QAction *m_actCopy;            // 复制
    QAction *m_actPaste;           // 粘贴
    QAction *m_actSelectCopyAll;   // 选择所有并复制
    QAction *m_actSaveCurWordInfo; // 保存当前词条信息
    QAction *m_actDelCurWordInfo;  // 删除当前词条信息

    DictQuery m_dictQuery;  // 字典查询对象
    QString m_findWordText; // 当前查找的词条
};

#endif
