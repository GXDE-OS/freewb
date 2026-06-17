#ifndef TEXTFINDDIALOG_H
#define TEXTFINDDIALOG_H

#include <QDialog>

namespace Ui
{
class TextFindDialog;
}

class TextFindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextFindDialog(QWidget *parent = nullptr);
    ~TextFindDialog();

signals:
    void signal_find_text(const QString &text, bool prevFlg, bool caseSensitiveFlg, bool wholeWordMatchFlg);

public:
    void set_find_text(const QString &text);

private slots:
    void on_btnNext_clicked();
    void on_btnPrev_clicked();

private:
    void setUiTexts();

private:
    Ui::TextFindDialog *ui;
};

#endif
