#include "customkeydialog.h"

#include "ui_customkeydialog.h"

CustomKeyDialog::CustomKeyDialog(QWidget *parent) : QDialog(parent), ui(new Ui::CustomKeyDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    installEventFilter(this);
}

CustomKeyDialog::~CustomKeyDialog()
{
    delete ui;
}

void CustomKeyDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }
}

void CustomKeyDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }
}

void CustomKeyDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }
}

bool CustomKeyDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

void CustomKeyDialog::set_custom_symbol(VirtualKeyboardMode vkm, const QString &keyName, const QString &commSymbol,
                                        const QString &shiftSymbol)
{
    ui->labelKeyName->setText(keyName);

    if (vkm == VKM_CUSTOM_MARK && keyName.at(0).isDigit())
    {
        ui->ledtComm->setEnabled(false);
        ui->ledtComm->clear();
    }
    else
    {
        ui->ledtComm->setEnabled(true);
        ui->ledtComm->setText(commSymbol);
    }

    ui->ledtShift->setText(shiftSymbol);
}

void CustomKeyDialog::on_btnCancle_clicked()
{
    accept();
    move(m_defaultPopPosition);
}

void CustomKeyDialog::on_btnOk_clicked()
{
    if (ui->ledtComm->text() == " ")
        ui->ledtComm->clear();
    if (ui->ledtShift->text() == " ")
        ui->ledtShift->clear();

    emit signal_custom_ok_btn_clicked(ui->ledtComm->text(), ui->ledtShift->text());

    accept();
    move(m_defaultPopPosition);
}
