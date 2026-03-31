/****************************************************************************************
** @作者：lcj
**
** @说明：
**      UsrGenWordDialog是一个从QDialog类继承而来的UI类，其对应的UI设计文件为usrgenworddialog.ui
**      ，该类设计目的为被MainProgram类所包含实例化，在用户进行自定义造词操作时弹出对话框。
******************************************************************************×*********/

#ifndef USRGENWORDDIALOG_H
#define USRGENWORDDIALOG_H

#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QMouseEvent>
#include <QRegExpValidator>
#include <QSettings>

namespace Ui
{
class UsrGenWordDialog;
}

class UsrGenWordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UsrGenWordDialog(QWidget *parent = nullptr);
    ~UsrGenWordDialog();

signals:
    void signal_user_word_changed();

public slots:
    void slot_show_dialog(const QString &wordText, const QString &wordCode);
    void slot_userWord_file_saved();

public:
    void add_user_word(const QString &wordText, const QString &wordCode);
    void delete_user_word(const QString &wordText, const QString &wordCode);

protected:
    void init_user_word_file();

    // 重载函数，用于窗口拖动
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_btnOk_clicked();
    void on_btnExit_clicked();

private:
    Ui::UsrGenWordDialog *ui;

    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    QPoint m_mouseLastPosition;
    QPoint m_defaultPopPosition;

    QString m_userWordFile;
};

#endif
