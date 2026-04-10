#include "texteditwin.h"

#include "../../tools/freewbConversionTool.h"
#include "commdefine.h"
#include "settings.h"
#include "settingshelper.h"
#include "ui_texteditwin.h"

#define WUBI_TABLE_FILE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/freeime.mb"
#define PINYIN_TABLE_FILE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/attach.mb"

TextEditWin::TextEditWin(QWidget *parent) : QMainWindow(parent), ui(new Ui::TextEditWin)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    m_modifiedFlg = 0;

    m_textFindDialog = new TextFindDialog(this);

    m_actFind = new QAction("查找(&F)", this);
    m_actFind->setShortcut(QKeySequence("ctrl+f"));
    m_actSave = new QAction("保存(&S)", this);
    m_actSave->setShortcut(QKeySequence("ctrl+s"));
    m_actQuit = new QAction("退出(&E)", this);
    m_actQuit->setShortcut(QKeySequence("ctrl+e"));
    ui->menubar->addAction(m_actFind);
    ui->menubar->addAction(m_actSave);
    ui->menubar->addAction(m_actQuit);

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    connect(m_actFind, &QAction::triggered, this, &TextEditWin::slot_open_find_dialog);
    connect(m_actSave, &QAction::triggered, this, &TextEditWin::slot_save_text);
    connect(m_actQuit, &QAction::triggered, this, &TextEditWin::slot_close_win);
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &TextEditWin::slot_text_is_changed);
    connect(m_textFindDialog, &TextFindDialog::signal_find_text, this, &TextEditWin::slot_find_text);

    init_quick_table_file();
}

TextEditWin::~TextEditWin()
{
    delete ui;
}

void TextEditWin::init_quick_table_file()
{
    QFile textFile(INSTALL_DIR + "/data/quick_table.txt");
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    if (!textFile.exists())
    {
        QString tips = "#快速码表\n"
                       "#编码规则：编码=词条\n"
                       "#编码只能是1位［a-z,标点]\n"
                       "#词条最多128个汉字\n"
                       "#每个词条一行，最多36个词条\n"
                       "#※最后一行不要忘记回车换行\n"
                       "s=……\n"
                       "d=、\n";

        if (!textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            qWarning() << textFile.fileName() << " open failed!";
        }
        else
        {
            textStream << tips;
            textStream.flush();
            textFile.close();
        }
    }
}

void TextEditWin::slot_open_textEdit_win(TextEditMode mode)
{
    m_textEditMode = mode;

    if (mode == TEM_USER_WORD)
    {
        open_user_word_file();
    }
    else if (mode == TEM_QUICK_TABLE)
    {
        open_quick_table_file();
    }
    else if (mode == TEM_WUBI_TABLE)
    {
        open_wubi_table_file();
    }
    else if (mode == TEM_PINYIN_TABLE)
    {
        open_pinyin_table_file();
    }
    else if (mode == TEM_SETTING_FILE)
    {
        open_setting_file();
    }
}

void TextEditWin::open_user_word_file()
{
    m_textFileName = INSTALL_DIR + "/data/user_word.txt";

    QFile textFile(m_textFileName);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    if (!textFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        setWindowTitle("文件打开失败!");
        ui->plainTextEdit->setPlainText("");
    }
    else
    {
        m_modifiedFlg = 0;
        setWindowTitle(m_textFileName + "[*]");
        ui->plainTextEdit->setPlainText(textStream.readAll());
        move(m_defaultPopPosition);
        textFile.close();
    }

    show();
    setWindowModified(false);
    activateWindow();
}

void TextEditWin::open_quick_table_file()
{
    m_textFileName = INSTALL_DIR + "/data/quick_table.txt";

    QFile textFile(m_textFileName);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    if (!textFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        setWindowTitle("文件打开失败!");
        ui->plainTextEdit->setPlainText("");
    }
    else
    {
        m_modifiedFlg = 0;
        setWindowTitle(m_textFileName + "[*]");
        ui->plainTextEdit->setPlainText(textStream.readAll());
        move(m_defaultPopPosition);
        textFile.close();
    }
    show();
    setWindowModified(false);
    activateWindow();
}

void TextEditWin::open_wubi_table_file()
{
    m_textFileName = "/tmp/freewb_sys.txt";

    QMessageBox msgBox;
    msgBox.setWindowFlag(Qt::FramelessWindowHint);
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.setText("正在打开......");
    msgBox.show();

    int count;
    if (!mb2txt(m_textFileName.toUtf8().data(), QString(WUBI_TABLE_FILE).toUtf8().data(), &count))
    {
        qWarning() << "convert mb to txt failed!";
        return;
    }

    QFile textFile(m_textFileName);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    if (!textFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        setWindowTitle("文件打开失败!");
        ui->plainTextEdit->setPlainText("");
    }
    else
    {
        m_modifiedFlg = 0;
        setWindowTitle(m_textFileName + "[*]");
        ui->plainTextEdit->setPlainText(textStream.readAll());
        move(m_defaultPopPosition);
        textFile.close();
    }

    show();
    setWindowModified(false);
    activateWindow();
}

void TextEditWin::open_pinyin_table_file()
{
    m_textFileName = "/tmp/freewb_pinyin.txt";

    QMessageBox msgBox;
    msgBox.setWindowFlag(Qt::FramelessWindowHint);
    msgBox.setIcon(QMessageBox::NoIcon);
    msgBox.setText("正在打开......");
    msgBox.show();

    int count;
    if (!mb2txt(m_textFileName.toUtf8().data(), QString(PINYIN_TABLE_FILE).toUtf8().data(), &count))
    {
        qWarning() << "convert mb to txt failed!";
        return;
    }

    QFile textFile;
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    textFile.setFileName(m_textFileName);
    if (!textFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        setWindowTitle("文件打开失败!");
        ui->plainTextEdit->setPlainText("");
    }
    else
    {
        m_modifiedFlg = 0;
        setWindowTitle(m_textFileName + "[*]");
        ui->plainTextEdit->setPlainText(textStream.readAll());
        move(m_defaultPopPosition);
        textFile.close();
    }

    show();
    setWindowModified(false);
    activateWindow();
}

void TextEditWin::open_setting_file()
{
    m_textFileName = INSTALL_DIR + "/config/config.ini";

    QFile textFile(m_textFileName);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    if (!textFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        setWindowTitle("文件打开失败!");
        ui->plainTextEdit->setPlainText("");
    }
    else
    {
        m_modifiedFlg = 0;
        setWindowTitle(m_textFileName + "[*]");
        ui->plainTextEdit->setPlainText(textStream.readAll());
        move(m_defaultPopPosition);
        textFile.close();
    }
    show();
    setWindowModified(false);
    activateWindow();
}

bool TextEditWin::save_text_to_file()
{
    QFile textFile(m_textFileName);
    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");

    if (!textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        qWarning() << textFile.fileName() << " open failed!";
        return false;
    }

    if (m_textEditMode == TEM_USER_WORD)
    {
        QStringList tmp = ui->plainTextEdit->toPlainText().split('\n');
        if (tmp.length())
        {
            tmp.removeFirst();
            tmp.removeDuplicates();
            // tmp.sort(Qt::CaseInsensitive);
            qSort(tmp.begin(), tmp.end(), [](const QString &a, const QString &b) { return QString::compare(a, b, Qt::CaseInsensitive) < 0; });

            tmp.insert(0, "[UserWord]");
            foreach(QString str, tmp)
            {
                // puts(str.toUtf8().constData());

                if (!str.isEmpty())
                {
                    textStream << str + "\n";
                }
            }
        }
        textStream.flush();
        textFile.close();
    }
    else if (m_textEditMode == TEM_WUBI_TABLE)
    {
        textStream << ui->plainTextEdit->toPlainText();
        textStream.flush();
        textFile.close();

        int count;
        if (!txt2mb(m_textFileName.toUtf8().data(), QString(WUBI_TABLE_FILE).toUtf8().data(), &count))
        {
            qWarning() << "convert txt to mb failed!";
            return false;
        }
    }
    else if (m_textEditMode == TEM_PINYIN_TABLE)
    {
        textStream << ui->plainTextEdit->toPlainText();
        textStream.flush();
        textFile.close();

        int count;
        if (!txt2mb(m_textFileName.toUtf8().data(), QString(PINYIN_TABLE_FILE).toUtf8().data(), &count))
        {
            qWarning() << "convert txt to mb failed!";
            return false;
        }
    }
    else if (m_textEditMode == TEM_QUICK_TABLE || m_textEditMode == TEM_SETTING_FILE)
    {
        textStream << ui->plainTextEdit->toPlainText();
        textStream.flush();
        textFile.close();
    }

    m_modifiedFlg = 0;

    if (m_textEditMode == TEM_SETTING_FILE)
    {
        settings::instance().reload();
        emit signal_setting_file_changed();
    }
    else if (m_textEditMode == TEM_USER_WORD)
    {
        emit signal_userWord_file_saved();
        emit signal_imTable_file_changed();
    }
    else if (m_textEditMode == TEM_QUICK_TABLE)
    {
        settings::instance().set_quickTableFlg(1);
        emit signal_quickTable_file_saved();
    }
    else if (m_textEditMode == TEM_WUBI_TABLE || m_textEditMode == TEM_PINYIN_TABLE)
    {
        settings::instance().set_imeTableChanged(1);
        emit signal_imTable_file_changed();
    }

    return true;
}

bool TextEditWin::close_text_win()
{
    if (m_modifiedFlg == 2)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setWindowFlag(Qt::FramelessWindowHint);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setText("关闭之前是否保存修改？");
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
        msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
        msgBox->button(QMessageBox::No)->setIcon(QIcon());
        msgBox->button(QMessageBox::No)->setText("否(&N)");
        msgBox->button(QMessageBox::Cancel)->setIcon(QIcon());
        msgBox->button(QMessageBox::Cancel)->setText("取消(&C)");
        msgBox->setDefaultButton(QMessageBox::Yes);

        int ret = msgBox->exec();
        delete msgBox;
        if (ret == QMessageBox::Yes)
        {
            bool savOK = save_text_to_file();
            if (!savOK)
            {
                qWarning() << m_textFileName << "save failed!";
            }
        }
        else if (ret == QMessageBox::No)
        {
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }

    return true;
}

void TextEditWin::closeEvent(QCloseEvent *event)
{
    if (!close_text_win())
    {
        event->ignore();
    }
    else if (m_textFindDialog->isVisible())
    {
        m_textFindDialog->close();
    }
}

void TextEditWin::slot_open_find_dialog()
{
    m_textFindDialog->set_find_text(ui->plainTextEdit->textCursor().selectedText());
    m_textFindDialog->show();
}

void TextEditWin::slot_save_text()
{
    bool saveOk;

    m_modifiedFlg = 1;
    saveOk = save_text_to_file();
    setWindowModified(false);

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText(saveOk ? "保存成功！" : "保存失败！");
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
    msgBox->button(QMessageBox::Ok)->setText("确认(&OK)");
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
    delete msgBox;
}

void TextEditWin::slot_close_win()
{
    if (close_text_win())
    {
        close();
        if (m_textFindDialog->isVisible())
        {
            m_textFindDialog->close();
        }
    }
}

void TextEditWin::slot_text_is_changed()
{
    if (!m_modifiedFlg)
    {
        m_modifiedFlg = 1;
    }
    else if (m_modifiedFlg == 1)
    {
        m_modifiedFlg = 2;
        setWindowModified(true);
    }
}

void TextEditWin::slot_find_text(const QString &text, bool prevFlg, bool caseSensitiveFlg, bool wholeWordMatchFlg)
{
    // qDebug() << text << prevFlg << caseSensitiveFlg << wholeWordMatchFlg;

    QTextDocument::FindFlags findOpt = static_cast<QTextDocument::FindFlag>(0);
    if (prevFlg)
    {
        findOpt |= QTextDocument::FindBackward;
    }
    if (caseSensitiveFlg)
    {
        findOpt |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordMatchFlg)
    {
        findOpt |= QTextDocument::FindWholeWords;
    }

    ui->plainTextEdit->find(text, findOpt);
}
