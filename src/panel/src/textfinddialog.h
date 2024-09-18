/****************************************************************************************
** @作者：lcj
**
** @说明：
**      TextFindDialog是一个从QDialog类继承而来的UI类，其对应的UI设计文件为textfinddialog.ui，该
**      类设计目的为被TextEditWin类所包含实例化，为用户提供一个文本内容搜索框.
*************************************************************************************×**/

#ifndef TEXTFINDDIALOG_H
#define TEXTFINDDIALOG_H

#include <QDialog>
#include <QDebug>


namespace Ui {
class TextFindDialog;
}

class TextFindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextFindDialog( QWidget *parent = nullptr );
    ~TextFindDialog();

signals:
    void signal_find_text( const QString &text, bool prevFlg, bool caseSensitiveFlg, bool wholeWordMatchFlg );

public:
    void set_find_text( const QString &text );

private slots:
    void on_btnNext_clicked();
    void on_btnPrev_clicked();

private:
    Ui::TextFindDialog *ui;

};

#endif
