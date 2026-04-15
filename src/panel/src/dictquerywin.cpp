#include "dictquerywin.h"

#include "config.h"
#include "ui_dictquerywin.h"

DictQueryWin::DictQueryWin(QWidget *parent) : QMainWindow(parent), ui(new Ui::DictQueryWin)
{
    ui->setupUi(this);
    setWindowTitle("极点查询");
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);

    setWindowIcon(QIcon(":/image/setting/logo.png"));

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    m_actCut = new QAction("剪切");
    m_actCopy = new QAction("复制");
    m_actPaste = new QAction("粘贴");
    m_actSelectCopyAll = new QAction("选择所有并复制");
    m_actSaveCurWordInfo = new QAction("保存当前词条信息");
    m_actDelCurWordInfo = new QAction("删除当前词条信息");

    m_menuEdit = new QMenu("编辑", this);
    m_menuEdit->addAction(m_actCut);
    m_menuEdit->addAction(m_actCopy);
    m_menuEdit->addAction(m_actPaste);
    m_menuEdit->addAction(m_actSelectCopyAll);
    m_menuEdit->addSeparator();
    m_menuEdit->addAction(m_actSaveCurWordInfo);
    m_menuEdit->addAction(m_actDelCurWordInfo);
    ui->menubar->addMenu(m_menuEdit);

    connect(m_actCut, &QAction::triggered, this, &DictQueryWin::slot_action_cut);
    connect(m_actCopy, &QAction::triggered, this, &DictQueryWin::slot_action_copy);
    connect(m_actPaste, &QAction::triggered, this, &DictQueryWin::slot_action_paste);
    connect(m_actSelectCopyAll, &QAction::triggered, this, &DictQueryWin::slot_action_select_copy_all);
    connect(m_actSaveCurWordInfo, &QAction::triggered, this, &DictQueryWin::slot_action_save_custom_word);
    connect(m_actDelCurWordInfo, &QAction::triggered, this, &DictQueryWin::slot_action_del_custom_word);

    // ui->textEditBase->setReadOnly( true );
    ui->ledtFind->installEventFilter(this);
}

DictQueryWin::~DictQueryWin()
{
    delete ui;
}

bool DictQueryWin::eventFilter(QObject *obj, QEvent *event)
{
    bool flg = false;

    QKeyEvent *keyEvt = static_cast<QKeyEvent *>(event);
    if (obj == ui->ledtFind && event->type() == QEvent::KeyPress)
    {
        if (keyEvt->key() == Qt::Key_Return)
        {
            flg = true;
            on_btnQuery_clicked();
        }
    }
    if (keyEvt->key() == Qt::Key_Escape)
    {
        close();
    }

    if (flg == true)
    {
        return true;
    }
    else
    {
        return QMainWindow::eventFilter(obj, event);
    }
}

void DictQueryWin::update_candidate_word_list(const QStringList &wordList)
{
    ui->listWidgetCandi->disconnect(); // 清空列表前断开与其连接的信号
    ui->listWidgetCandi->clear();

    if (wordList.length())
    {
        foreach(QString str, wordList)
        {
            ui->listWidgetCandi->addItem(str);
        }
    }

    connect(ui->listWidgetCandi, &QListWidget::currentItemChanged, this, &DictQueryWin::on_listWidgetCandi_currentItemChanged);
}

void DictQueryWin::find_word(const QString &text)
{
    ui->textEditBase->clear();
    ui->textEditExplain->clear();

    QString prefix = QString("【 %1 】\n").arg(text);

    QString paraphrase = m_dictQuery.dict_query_word_paraphrase(text);
    paraphrase.replace("\r\n", "\r\n  ");
    if (!paraphrase.isEmpty())
    {
        // ui->textEditExplain->setReadOnly( true );
    }
    else
    {
        // ui->textEditExplain->setReadOnly( false );
        paraphrase = "\n  当前词条无解释，你可以在此输入解释后存盘．";
    }

    ui->textEditExplain->setText(prefix + paraphrase);

    QString dispStr, tmp;
    if (!text.isEmpty())
    {
        foreach(QChar wchar, text)
        {
            if (tmp.contains(wchar))
                continue;
            tmp += wchar;

            QStringList wbpy = m_dictQuery.dict_query_word_wubi_pinyin(wchar).split(';');
            if (wbpy.length() == 1)
            {
                QString wb = wbpy.at(0);
                if (!wb.isEmpty())
                {
                    dispStr += QString("【 %1 】\n  编码：%2\n").arg(wchar).arg(wb.replace(",", "  "));
                }
            }
            else if (wbpy.length() == 2)
            {
                QString wb = wbpy.at(0);
                QString py = wbpy.at(1);
                dispStr += QString("【 %1 】\n  编码：%2\n  拼音：%3\n").arg(wchar).arg(wb.replace(",", "  ")).arg(py.replace(",", "  "));
            }
        }

        ui->textEditBase->setText(dispStr);
    }

    m_findWordText = text;
}

void DictQueryWin::find_word_list(const QString &text)
{
    QStringList wordList = m_dictQuery.dict_query_word_list(text);
    update_candidate_word_list(wordList);
}

void DictQueryWin::slot_open_win(const QString &queryText)
{
    if (!queryText.isEmpty())
    {
        ui->ledtFind->setText(queryText);
        find_word_list(queryText);
        find_word(queryText);
    }

    move(m_defaultPopPosition);
    show();
    activateWindow();
}

void DictQueryWin::on_btnQuery_clicked()
{
    if (!ui->ledtFind->text().isEmpty())
    {
        find_word_list(ui->ledtFind->text());
        find_word(ui->ledtFind->text());
    }
}

void DictQueryWin::on_btnExit_clicked()
{
    close();
}

void DictQueryWin::on_listWidgetCandi_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    ui->ledtFind->setText(current->text());
    find_word(current->text());
}

void DictQueryWin::slot_action_cut()
{
    if (qobject_cast<QLineEdit *>(QApplication::focusWidget()) == ui->ledtFind)
    {
        ui->ledtFind->cut();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditExplain)
    {
        ui->textEditExplain->cut();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditBase)
    {
        ui->textEditBase->cut();
    }
}

void DictQueryWin::slot_action_copy()
{
    if (qobject_cast<QLineEdit *>(QApplication::focusWidget()) == ui->ledtFind)
    {
        ui->ledtFind->copy();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditExplain)
    {
        ui->textEditExplain->copy();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditBase)
    {
        ui->textEditBase->copy();
    }
}

void DictQueryWin::slot_action_paste()
{
    if (qobject_cast<QLineEdit *>(QApplication::focusWidget()) == ui->ledtFind)
    {
        ui->ledtFind->paste();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditExplain)
    {
        ui->textEditExplain->paste();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditBase)
    {
        ui->textEditBase->paste();
    }
}

void DictQueryWin::slot_action_select_copy_all()
{
    if (qobject_cast<QLineEdit *>(QApplication::focusWidget()) == ui->ledtFind)
    {
        ui->ledtFind->selectAll();
        ui->ledtFind->copy();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditExplain)
    {
        ui->textEditExplain->selectAll();
        ui->textEditExplain->copy();
    }
    else if (qobject_cast<QTextEdit *>(QApplication::focusWidget()) == ui->textEditBase)
    {
        ui->textEditBase->selectAll();
        ui->textEditBase->copy();
    }
}

void DictQueryWin::slot_action_save_custom_word()
{
    int ret = 0;

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(QString("确定保存对词条【 %1 】的修改？").arg(m_findWordText));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText("否(&N)");
    msgBox->setDefaultButton(QMessageBox::Yes);
    ret = msgBox->exec();
    delete msgBox;

    if (ret == QMessageBox::Yes)
    {
        ret = m_dictQuery.dict_add_custom_word(ui->ledtFind->text(), ui->textEditExplain->toPlainText().remove(QString("【 %1 】\n").arg(m_findWordText)));

        msgBox = new QMessageBox(this);
        msgBox->setWindowFlag(Qt::FramelessWindowHint);
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
        msgBox->button(QMessageBox::Ok)->setText("确认(&OK)");
        msgBox->setDefaultButton(QMessageBox::Ok);

        if (ret == 0)
        {
            msgBox->setText("保存成功！");
        }
        else
        {
            msgBox->setText("保存失败！");
        }

        msgBox->exec();
        delete msgBox;
    }
}

void DictQueryWin::slot_action_del_custom_word()
{
    int ret = 0;

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowFlag(Qt::FramelessWindowHint);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(QString("确定删除词条【 %1 】？").arg(m_findWordText));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText("是(&Y)");
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText("否(&N)");
    msgBox->setDefaultButton(QMessageBox::Yes);
    ret = msgBox->exec();
    delete msgBox;

    if (ret == QMessageBox::Yes)
    {
        ret = m_dictQuery.dict_del_custom_word(ui->ledtFind->text());

        msgBox = new QMessageBox(this);
        msgBox->setWindowFlag(Qt::FramelessWindowHint);
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
        msgBox->button(QMessageBox::Ok)->setText("确认(&OK)");
        msgBox->setDefaultButton(QMessageBox::Ok);

        if (ret == 0)
        {
            msgBox->setText("词条删除成功！");
            ui->textEditExplain->clear();
        }
        else
        {
            msgBox->setText("词条删除失败！");
        }

        msgBox->exec();
        delete msgBox;
    }
}
