#include "textfinddialog.h"
#include "ui_textfinddialog.h"
#include "commdefine.h"


TextFindDialog::TextFindDialog( QWidget *parent ) : QDialog(parent), ui(new Ui::TextFindDialog)
{
    ui->setupUi(this);
    setWindowFlags( Qt::Tool | Qt::WindowStaysOnTopHint );
    setWindowTitle( "查找" );
}

TextFindDialog::~TextFindDialog()
{
    delete ui;
}

void TextFindDialog::set_find_text( const QString &text )
{
    ui->lineEdit->setText( text );
}

void TextFindDialog::on_btnNext_clicked()
{
    emit signal_find_text( ui->lineEdit->text(),
                           false,
                           ui->ckbCaseSensitive->isChecked(),
                           ui->ckbWholeWordMatch->isChecked() );
}

void TextFindDialog::on_btnPrev_clicked()
{
    emit signal_find_text( ui->lineEdit->text(),
                           true,
                           ui->ckbCaseSensitive->isChecked(),
                           ui->ckbWholeWordMatch->isChecked() );
}
