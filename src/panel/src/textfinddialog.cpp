#include "textfinddialog.h"

#include "config.h"
#include "ui_textfinddialog.h"
#include "waylandwinhelper.h"

TextFindDialog::TextFindDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TextFindDialog)
{
    ui->setupUi(this);
    setUiTexts();
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowTitle(_("Find"));

    freewb::applyWaylandOverlayWindowHints(this);
}

void TextFindDialog::setUiTexts()
{
    ui->label->setText(_("Find what"));
    ui->btnNext->setText(_("Next (&N)"));
    ui->btnPrev->setText(_("Previous (&P)"));
    ui->ckbCaseSensitive->setText(_("Match case (&C)"));
    ui->ckbWholeWordMatch->setText(_("Whole words (&W)"));
}

TextFindDialog::~TextFindDialog()
{
    delete ui;
}

void TextFindDialog::set_find_text(const QString &text)
{
    ui->lineEdit->setText(text);
}

void TextFindDialog::on_btnNext_clicked()
{
    emit signal_find_text(ui->lineEdit->text(), false, ui->ckbCaseSensitive->isChecked(), ui->ckbWholeWordMatch->isChecked());
}

void TextFindDialog::on_btnPrev_clicked()
{
    emit signal_find_text(ui->lineEdit->text(), true, ui->ckbCaseSensitive->isChecked(), ui->ckbWholeWordMatch->isChecked());
}
