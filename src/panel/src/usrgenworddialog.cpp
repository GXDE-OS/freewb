#include "usrgenworddialog.h"

#include <QTextStream>

#include "config.h"
#include "settings.h"
#include "ui_usrgenworddialog.h"
#include "waylandwinhelper.h"

#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"

UsrGenWordDialog::UsrGenWordDialog(QWidget *parent) : QDialog(parent), ui(new Ui::UsrGenWordDialog)
{
    ui->setupUi(this);
    setUiTexts();
    // setWindowFlags( Qt::Tool | Qt::FramelessWindowHint );
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QRegExp rx("^[a-z]{1,16}$");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    ui->ledtWordCode->setValidator(validator);

    m_userWordFile = INSTALL_DIR + "/data/user_word.txt";

    init_user_word_file();

    freewb::WaylandWinHelper::applyOverlayHints(this);
}

void UsrGenWordDialog::setUiTexts()
{
    setWindowTitle(_("Freewb word creation"));
    ui->label_3->setText(_("Phrase:"));
    ui->label_4->setText(_("Code:"));
    ui->btnOk->setText(_("OK"));
    ui->btnExit->setText(_("Exit"));
}

UsrGenWordDialog::~UsrGenWordDialog()
{
    delete ui;
}

bool UsrGenWordDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;
    QKeyEvent *keyEvt = static_cast<QKeyEvent *>(event);

    if (keyEvt->key() == Qt::Key_Escape)
    {
        on_btnExit_clicked();
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

void UsrGenWordDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }
}

void UsrGenWordDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }
}

void UsrGenWordDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }
}

void UsrGenWordDialog::init_user_word_file()
{
    QFile textFile(m_userWordFile);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");

    if (!textFile.exists() || textFile.size() < 15)
    {
        if (!textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            return;
        }

        QStringList strList;
        strList << "date=$y年$m月$d日\n";
        strList << "date=$Y年$M月$D日\n";
        strList << "day=$D日\n";
        strList << "day=$d日\n";
        strList << "hour=$H时\n";
        strList << "hour=$h时\n";
        strList << "minute=$MI分\n";
        strList << "minute=$mi分\n";
        strList << "month=$M月\n";
        strList << "month=$m月\n";
        strList << "now=$y年$m月$d日$0h时$0mi分$0s秒\n";
        strList << "second=$S秒\n";
        strList << "second=$s秒\n";
        strList << "time=$h时$mi分\n";
        strList << "time=$H时$MI分\n";
        strList << "week=$W\n";
        strList << "week=$w\n";
        strList << "year=$Y年\n";
        strList << "year=$y年\n";

        strList.sort();
        strList.insert(0, "[UserWord]\n");

        foreach(QString str, strList)
        {
            textStream << str;
        }
        textStream << "[DeletedWord]\n";
        textStream.flush();
        textFile.close();
    }
}

void UsrGenWordDialog::writeUserWordFile(const QStringList &userLines, const QStringList &deletedLines)
{
    QFile textFile(m_userWordFile);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");

    if (!textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        return;
    }

    textStream << "[UserWord]\n";
    foreach(const QString &str, userLines)
    {
        if (!str.isEmpty())
        {
            textStream << str << "\n";
        }
    }
    textStream << "[DeletedWord]\n";
    foreach(const QString &str, deletedLines)
    {
        if (!str.isEmpty())
        {
            textStream << str << "\n";
        }
    }
    textStream.flush();
    textFile.close();
}

void UsrGenWordDialog::add_user_word(const QString &wordText, const QString &wordCode)
{
    QFile textFile(m_userWordFile);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");

    if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QStringList userLines;
    QStringList deletedLines;
    bool inDeleted = false;
    const QStringList strList = textStream.readAll().split('\n');
    textFile.close();

    for (QString line : strList)
    {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
        {
            continue;
        }
        if (line == "[UserWord]")
        {
            inDeleted = false;
            continue;
        }
        if (line == "[DeletedWord]")
        {
            inDeleted = true;
            continue;
        }
        if (inDeleted)
        {
            deletedLines << line;
        }
        else
        {
            userLines << line;
        }
    }

    userLines << QString("%1=%2").arg(wordCode).arg(wordText);
    userLines.removeDuplicates();
    userLines.sort();
    writeUserWordFile(userLines, deletedLines);
}

void UsrGenWordDialog::delete_user_word(const QString &wordText, const QString &wordCode)
{
    QFile textFile(m_userWordFile);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");

    if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QStringList userLines;
    QStringList deletedLines;
    bool inDeleted = false;
    const QString target = QString("%1=%2").arg(wordCode).arg(wordText);
    const QStringList strList = textStream.readAll().split('\n');
    textFile.close();

    for (QString line : strList)
    {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
        {
            continue;
        }
        if (line == "[UserWord]")
        {
            inDeleted = false;
            continue;
        }
        if (line == "[DeletedWord]")
        {
            inDeleted = true;
            continue;
        }
        if (inDeleted)
        {
            deletedLines << line;
        }
        else if (line != target)
        {
            userLines << line;
        }
    }

    writeUserWordFile(userLines, deletedLines);
}

void UsrGenWordDialog::showDialog(const QString &wordText, const QString &wordCode, bool online)
{
    m_onlineMode = online;
    ui->ledtWordText->setText(wordText);
    ui->ledtWordCode->setText(wordCode);
    if (wordCode.length() < 1)
    {
        ui->ledtWordCode->setFocus();
    }
    exec();
    m_onlineMode = false;
}

void UsrGenWordDialog::slot_show_dialog(const QString &wordText, const QString &wordCode)
{
    showDialog(wordText, wordCode, false);
}

void UsrGenWordDialog::slot_show_dialog_online(const QString &wordText, const QString &wordCode)
{
    showDialog(wordText, wordCode, true);
}

void UsrGenWordDialog::slot_userWord_file_saved()
{
    emit signal_user_word_changed();
}

void UsrGenWordDialog::on_btnOk_clicked()
{
    const QString wordText = ui->ledtWordText->text();
    const QString wordCode = ui->ledtWordCode->text();
    if (m_onlineMode)
    {
        emit signal_commit_user_word(wordText, wordCode);
    }
    else
    {
        add_user_word(wordText, wordCode);
        emit signal_user_word_changed();
    }
    accept();
}

void UsrGenWordDialog::on_btnExit_clicked()
{
    ui->ledtWordText->clear();
    ui->ledtWordCode->clear();
    accept();
}
