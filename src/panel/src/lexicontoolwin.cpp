#include "lexicontoolwin.h"

#include <QEventLoop>
#include <QTextStream>

#include "config.h"
#include "settings.h"
#include "settingshelper.h"
#include "tools/ConversionTool.h"
#include "types.h"
#include "ui_lexicontoolwin.h"
#include "waylandwinhelper.h"

// 词库工具窗口样式表
#define QSS_FILE ":/qss/lexiconwin.qss"

#define CUR_USED_WUBI_TABLE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/wbzx.mb"
#define CUR_USED_PINYIN_TABLE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/pinyin.mb"

LexiconWorker::LexiconWorker(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LexiconToolOp>("LexiconToolOp");
}

LexiconWorker::~LexiconWorker()
{
}

void LexiconWorker::set_op_param(LexiconToolOp opType, const QString &srcFile, const QString &destFile)
{
    m_opType = opType;
    m_srcFile = srcFile;
    m_destFile = destFile;
}

void LexiconWorker::slot_start_work()
{
    m_count = 0;
    QByteArray srcUtf8 = m_srcFile.toUtf8();
    QByteArray destUtf8 = m_destFile.toUtf8();

    if (m_opType == LTO_DUMP_SYS_TABLE || m_opType == LTO_DUMP_PINYIN_TABLE)
    {
        if (freewb::tools::mb2txt(srcUtf8.data(), destUtf8.data(), &m_count))
        {
            emit signal_process_updated(m_opType, 1, m_count);
        }
        else
        {
            emit signal_process_updated(m_opType, -1, m_count);
        }
    }
    else if (m_opType == LTO_GEN_SYS_TABLE || m_opType == LTO_GEN_PINYIN_TABLE)
    {
        if (freewb::tools::txt2mb(srcUtf8.data(), destUtf8.data(), &m_count))
        {
            emit signal_process_updated(m_opType, 1, m_count);
        }
        else
        {
            emit signal_process_updated(m_opType, -1, m_count);
        }
    }
    else if (m_opType == LTO_MARK_RARE_CHAR)
    {
    }
    else if (m_opType == LTO_MARK_THINK_WORD)
    {
    }
    else if (m_opType == LTO_OPTIMIZE_TABLE)
    {
    }
}

LexiconToolWin::LexiconToolWin(QWidget *parent) : QWidget(parent), ui(new Ui::LexiconToolWin)
{
    ui->setupUi(this);
    setUiTexts();
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    setFixedSize(size());
    setWindowIcon(QIcon::fromTheme("freewb"));
    setWindowTitle(_("Freewb lexicon tool"));

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);
    move(m_defaultPopPosition);

    m_lexiconThread = new QThread(this);
    m_lexiconWorker = new LexiconWorker();
    m_lexiconWorker->moveToThread(m_lexiconThread);
    connect(m_lexiconThread, &QThread::started, m_lexiconWorker, &LexiconWorker::slot_start_work);
    connect(m_lexiconThread, &QThread::finished, this, &LexiconToolWin::slot_worker_thread_finished);
    connect(m_lexiconWorker, &LexiconWorker::signal_process_updated, this, &LexiconToolWin::slot_process_updated,
            Qt::QueuedConnection);

    m_msgBox = new QMessageBox(this);
    m_msgBox->setIcon(QMessageBox::NoIcon);

    m_tmpSysTable = QString(qgetenv("HOME")) + "/wbzx.mb";
    m_tmpPinyinTable = QString(qgetenv("HOME")) + "/pinyin.mb";

    // 载入窗口全局UI样式表
    QFile qssFile(QSS_FILE);
    if (qssFile.open(QFile::ReadOnly))
    {
        // 仅作用于 frame 内控件，避免 QMessageBox 子窗口继承 QPushButton 样式。
        ui->frame->setStyleSheet(qssFile.readAll());
        qssFile.close();
    }

    ui->btnMarkRareWord->hide();
    ui->btnMarkThinkWord->hide();
    ui->btnOptimize->hide();

    freewb::applyWaylandOverlayWindowHints(this);
}

void LexiconToolWin::setUiTexts()
{
    ui->btnDumpSysLexicon->setText(_("Export"));
    ui->label->setText(_("Wbzx lexicon"));
    ui->label_2->setText(_("Pinyin lexicon"));
    ui->label_3->setText(_("User lexicon"));
    ui->btnMarkRareWord->setText(_("Mark rare characters"));
    ui->btnMakeSysLexicon->setText(_("Generate lexicon"));
    ui->btnHelp->setText(_("Help"));
    ui->btnOptimize->setText(_("Optimize"));
    ui->btnMarkThinkWord->setText(_("Mark associative words"));
    ui->btnDumpPinyinLexicon->setText(_("Export"));
    ui->btnMakePinyinLexicon->setText(_("Generate lexicon"));
    ui->btnDumpUserLexicon->setText(_("Export"));
    ui->btnBatchDel->setText(_("Batch delete words"));
    ui->btnBatchAdd->setText(_("Batch add words"));
}

LexiconToolWin::~LexiconToolWin()
{
    delete ui;
    if (m_lexiconWorker)
    {
        delete m_lexiconWorker;
    }
}

void LexiconToolWin::open_win()
{
    showNormal();
    activateWindow();
}

void LexiconToolWin::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void LexiconToolWin::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void LexiconToolWin::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

void LexiconToolWin::lexicon_thread_quit()
{
    if (m_lexiconThread)
    {
        if (m_lexiconThread->isRunning())
        {
            m_lexiconThread->quit();
            m_lexiconThread->wait();
        }
    }
}

void LexiconToolWin::showLexiconProgressMsgBox(QMessageBox *box, const char *textMsgid)
{
    box->setIcon(QMessageBox::NoIcon);
    box->setText(_(textMsgid));
    box->setStandardButtons(QMessageBox::NoButton);
    box->show();
    QApplication::processEvents();
}

void LexiconToolWin::hideLexiconProgressMsgBox(QMessageBox *box)
{
    box->hide();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void LexiconToolWin::startDumpLexicon(LexiconToolOp opType, const QString &txtPath, const QString &mbPath)
{
    m_workerOpStatus = 0;
    m_workerOpCount = 0;
    m_lexiconWorker->set_op_param(opType, txtPath, mbPath);
    if (m_lexiconThread->isRunning())
    {
        lexicon_thread_quit();
    }

    QMessageBox *progressBox = new QMessageBox(this);
    showLexiconProgressMsgBox(progressBox, "Exporting lexicon...");

    m_lexiconThread->start();

    QEventLoop loop;
    m_opWaitLoop = &loop;
    loop.exec();
    m_opWaitLoop = nullptr;

    hideLexiconProgressMsgBox(progressBox);
    delete progressBox;

    QMessageBox *msgBox = new QMessageBox(this);
    if (m_workerOpStatus == 1)
    {
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(QString(_("Lexicon export completed! Number: %1")).arg(m_workerOpCount));
    }
    else
    {
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setText(_("File open failed or format error!"));
    }
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
    msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&OK)"));
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
    delete msgBox;
}

int LexiconToolWin::startGenLexicon(LexiconToolOp opType, const QString &txtPath, const QString &mbPath)
{
    m_workerOpStatus = 0;
    m_workerOpCount = 0;
    m_lexiconWorker->set_op_param(opType, txtPath, mbPath);
    if (m_lexiconThread->isRunning())
    {
        lexicon_thread_quit();
    }

    QMessageBox *progressBox = new QMessageBox(this);
    showLexiconProgressMsgBox(progressBox, "Generating lexicon...");

    m_lexiconThread->start();

    QEventLoop loop;
    m_opWaitLoop = &loop;
    loop.exec();
    m_opWaitLoop = nullptr;

    hideLexiconProgressMsgBox(progressBox);
    delete progressBox;

    if (m_workerOpStatus != 1)
    {
        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setText(_("File open failed or format error!"));
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
        msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&OK)"));
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        delete msgBox;
        return QMessageBox::No;
    }

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(_("Lexicon generated successfully, whether to replace the current used lexicon?"));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
    msgBox->setDefaultButton(QMessageBox::Yes);
    const int ret = msgBox->exec();
    delete msgBox;
    return ret;
}

void LexiconToolWin::slot_process_updated(LexiconToolOp opType, int opStatus, int count)
{
    if (opType == LTO_DUMP_SYS_TABLE || opType == LTO_DUMP_PINYIN_TABLE || opType == LTO_GEN_SYS_TABLE ||
        opType == LTO_GEN_PINYIN_TABLE)
    {
        if (opStatus == 0)
        {
            return;
        }

        m_workerOpStatus = opStatus;
        m_workerOpCount = count;
        lexicon_thread_quit();
        if (m_opWaitLoop != nullptr)
        {
            m_opWaitLoop->quit();
        }
        return;
    }

    if (opStatus == 0)
    {
        m_msgBox->setText(QString(_("Number: ")) + QString::number(count));
    }
    else if (opStatus == 1)
    {
        lexicon_thread_quit();
        if (opType == LTO_MARK_RARE_CHAR || opType == LTO_MARK_THINK_WORD)
        {
            m_msgBox->setText(QString(_("Lexicon marked completed! Number: %1")).arg(count));
            m_msgBox->button(QMessageBox::Cancel)->setEnabled(false);
            m_msgBox->button(QMessageBox::Ok)->setEnabled(true);
        }
        else if (opType == LTO_OPTIMIZE_TABLE)
        {
            m_msgBox->setText(_("Lexicon optimized completed!"));
            m_msgBox->button(QMessageBox::Cancel)->setEnabled(false);
            m_msgBox->button(QMessageBox::Ok)->setEnabled(true);
        }
    }
    else if (opStatus == -1)
    {
        lexicon_thread_quit();
        m_msgBox->setText(_("File open failed or format error!"));
        m_msgBox->button(QMessageBox::Cancel)->setEnabled(false);
        m_msgBox->button(QMessageBox::Ok)->setEnabled(true);
    }
}

void LexiconToolWin::slot_worker_thread_finished()
{
}

void LexiconToolWin::on_btnHelp_clicked()
{
    QString helpInfo = "\n"
                       "一、五笔词库\n"
                       "①导出：将词库以纯文本格式导出，供用户编辑、修改。\n"
                       "②生成词库：根据用户提供的纯文本文件生成极点的五笔\n"
                       "    词库。这个文本文件的格式为：编码+英文空格+候选\n"
                       "    项，如果有重码需要在同一行以英文空格分隔），整\n"
                       "    个文件按编码顺序排列：\n"
                       "    a 工 其\n"
                       "    aa 式\n"
                       "    aaaa 工 恭恭敬敬\n"
                       "    ......\n"
                       //    "③标记生僻字：根据字频标注生僻字，供生成词库时使用。\n"
                       //    "    有关生僻字见极点自带帮助文档。\n"
                       //    "④标记联想词：根据用户的文件将这些词组标记为联想词，\n"
                       //    "    其文件格式同“生成词库”。\n"
                       //    "⑤优化：清理无用信息，减少词库大小。\n"
                       "\n"
                       "二、拼音词库\n"
                       "①导出：将词库以纯文本格式导出，供用户编辑、修改。\n"
                       "②生成词库：同“五笔词库”中的生成词库，只是这个生成的\n"
                       "    是拼音词库。\n"
                       "\n"
                       "三、用户词组\n"
                       "①导出：将词库中所有的用户词组以纯文本格式导出，供用\n"
                       "    户编辑、修改。\n"
                       "②批量删词：根据用户提供的文本文件从词库中删去这些词\n"
                       "    ，其文件格式同“生成词库”。\n"
                       "③批量加词：与“批量删词”类似，用户批量添加词组，文件\n"
                       "    格式同“生成词库”。\n\n";

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText(helpInfo);
    msgBox->setStandardButtons(QMessageBox::Ok);
    msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
    msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&OK)"));
    msgBox->setDefaultButton(QMessageBox::Ok);
    msgBox->exec();
    delete msgBox;
}

void LexiconToolWin::on_btnDumpSysLexicon_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, _("Generate TXT file"), QString(qgetenv("HOME")) + "/wbzx.txt");
    if (!fileName.isEmpty())
    {
        startDumpLexicon(LTO_DUMP_SYS_TABLE, fileName, CUR_USED_WUBI_TABLE);
    }
}

void LexiconToolWin::on_btnMakeSysLexicon_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select TXT file"), QString(qgetenv("HOME")));
    if (!fileName.isEmpty())
    {
        const int ret = startGenLexicon(LTO_GEN_SYS_TABLE, fileName, m_tmpSysTable);
        if (ret == QMessageBox::Yes)
        {
            QString cmd = QString("cp %1 %2").arg(m_tmpSysTable).arg(CUR_USED_WUBI_TABLE);
            system(cmd.toUtf8().data());

            emit signal_reload_dictionaries(freewb::DictReloadWubiTable);
        }
        else if (ret == QMessageBox::No)
        {
            QString cmd;
            cmd = QString("xdg-open %1 > /dev/null 2>&1 &").arg(m_tmpSysTable);
            system(cmd.toUtf8().data());
        }
    }
}

void LexiconToolWin::on_btnMarkRareWord_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select TXT file"), QString(qgetenv("HOME")));
    if (!fileName.isEmpty())
    {
        m_lexiconWorker->set_op_param(LTO_MARK_RARE_CHAR, fileName, CUR_USED_WUBI_TABLE);
        m_lexiconThread->start();

        m_msgBox->setText(_("Marking..."));
        m_msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        m_msgBox->button(QMessageBox::Cancel)->setEnabled(true);
        m_msgBox->button(QMessageBox::Ok)->setEnabled(false);
        m_msgBox->button(QMessageBox::Cancel)->setText(_("Cancel(&C)"));
        m_msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&O)"));
        int ret = m_msgBox->exec();
        if (ret == QMessageBox::Cancel)
        {
            lexicon_thread_quit();
        }
        else if (ret == QMessageBox::Ok)
        {
            //            settings::instance().set_imeTableChanged( 1 );
            //            emit signal_ime_table_changed();
        }
    }
}

void LexiconToolWin::on_btnMarkThinkWord_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select TXT file"), QString(qgetenv("HOME")));
    if (!fileName.isEmpty())
    {
        m_lexiconWorker->set_op_param(LTO_MARK_THINK_WORD, fileName, CUR_USED_WUBI_TABLE);
        m_lexiconThread->start();

        m_msgBox->setText(_("Marking..."));
        m_msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        m_msgBox->button(QMessageBox::Cancel)->setEnabled(true);
        m_msgBox->button(QMessageBox::Ok)->setEnabled(false);
        m_msgBox->button(QMessageBox::Cancel)->setText(_("Cancel(&C)"));
        m_msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&O)"));
        int ret = m_msgBox->exec();
        if (ret == QMessageBox::Cancel)
        {
            lexicon_thread_quit();
        }
        else if (ret == QMessageBox::Ok)
        {
            //            settings::instance().set_imeTableChanged( 1 );
            //            emit signal_ime_table_changed();
        }
    }
}

void LexiconToolWin::on_btnOptimize_clicked()
{
    m_lexiconWorker->set_op_param(LTO_OPTIMIZE_TABLE, CUR_USED_WUBI_TABLE, "");
    m_lexiconThread->start();

    m_msgBox->setText(_("Optimizing..."));
    m_msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    m_msgBox->button(QMessageBox::Cancel)->setEnabled(true);
    m_msgBox->button(QMessageBox::Ok)->setEnabled(false);
    m_msgBox->button(QMessageBox::Cancel)->setText(_("Cancel(&C)"));
    m_msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&O)"));
    int ret = m_msgBox->exec();
    if (ret == QMessageBox::Cancel)
    {
        lexicon_thread_quit();
    }
}

void LexiconToolWin::on_btnDumpPinyinLexicon_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, _("Generate TXT file"), QString(qgetenv("HOME")) + "/pinyin.txt");
    if (!fileName.isEmpty())
    {
        startDumpLexicon(LTO_DUMP_PINYIN_TABLE, fileName, CUR_USED_PINYIN_TABLE);
    }
}

void LexiconToolWin::on_btnMakePinyinLexicon_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select TXT file"), QString(qgetenv("HOME")));
    if (!fileName.isEmpty())
    {
        const int ret = startGenLexicon(LTO_GEN_PINYIN_TABLE, fileName, m_tmpPinyinTable);
        if (ret == QMessageBox::Yes)
        {
            QString cmd = QString("cp %1 %2").arg(m_tmpPinyinTable).arg(CUR_USED_PINYIN_TABLE);
            system(cmd.toUtf8().data());

            emit signal_reload_dictionaries(freewb::DictReloadPinyinTable);
        }
        else if (ret == QMessageBox::No)
        {
            char cmd[128] = {0};
            sprintf(cmd, "xdg-open %s > /dev/null 2>&1 &", m_tmpPinyinTable.toUtf8().data());
            system(cmd);
        }
    }
}

void LexiconToolWin::on_btnDumpUserLexicon_clicked()
{
    QStringList args;
    args << INSTALL_DIR + "/data/user_word.txt" << QString(qgetenv("HOME"));
    QProcess::execute("cp", args);

    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(QString(_("User lexicon exported to: %1/user_word.txt, whether to view?")).arg(QString(qgetenv("HOME"))));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->button(QMessageBox::Yes)->setIcon(QIcon());
    msgBox->button(QMessageBox::Yes)->setText(_("Yes(&Y)"));
    msgBox->button(QMessageBox::No)->setIcon(QIcon());
    msgBox->button(QMessageBox::No)->setText(_("No(&N)"));
    msgBox->setDefaultButton(QMessageBox::Yes);
    int ret = msgBox->exec();
    delete msgBox;

    if (ret == QMessageBox::Yes)
    {
        char cmd[128] = {0};
        sprintf(cmd, "xdg-open ~/user_word.txt > /dev/null 2>&1 &");
        system(cmd);
    }
}

void LexiconToolWin::on_btnBatchDel_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select file"), qgetenv("HOME"), "Text files (*.txt);;");
    add_del_user_word_from_file(0, fileName);
}

void LexiconToolWin::on_btnBatchAdd_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, _("Select file"), qgetenv("HOME"), "Text files (*.txt);;");
    add_del_user_word_from_file(1, fileName);
}

// op: 0 - 删除  1 - 增加
void LexiconToolWin::add_del_user_word_from_file(int op, const QString &fileName)
{
    int count = 0;

    QFile selectFile(fileName);
    if (!fileName.isEmpty() && selectFile.size() < 1048576 * 2) // 1048576=1MB
    {
        if (!selectFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return;
        }
        QTextStream selectFileStream(&selectFile);
        selectFileStream.setCodec("UTF-8");
        QStringList opWordLines = selectFileStream.readAll().split("\n");
        selectFile.close();

        if (opWordLines.size())
        {
            QFile userWordFile(QString(INSTALL_DIR) + "/data/user_word.txt");
            QTextStream userWordStream(&userWordFile);
            if (!userWordFile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                return;
            }
            QStringList userWordList = userWordStream.readAll().split('\n');

            foreach(QString line, opWordLines)
            {
                QString str, code;
                QStringList tempList, valueList;

                line = line.trimmed();
                if (line.startsWith('#'))
                    continue;

                if (line.contains('='))
                {
                    tempList = line.split('=');
                    if (tempList.length() == 2)
                    {
                        code = tempList.at(0);
                        code = code.trimmed();
                        valueList = tempList.at(1).split(' ', QString::SkipEmptyParts);
                    }
                }
                else if (line.contains(' '))
                {
                    tempList = line.split(' ', QString::SkipEmptyParts);
                    if (tempList.length() > 1)
                    {
                        code = tempList.takeAt(0);
                        valueList = tempList;
                    }
                }
                else
                {
                    continue;
                }

                if (code.isEmpty())
                    continue;
                int invalidFlg = 0;
                foreach(QChar ch, code)
                {
                    if (ch < 'a' || ch > 'z')
                    {
                        invalidFlg = 1;
                        break;
                    }
                }
                if (invalidFlg)
                    continue;

                foreach(QString value, valueList)
                {
                    str = code + "=" + value;
                    if (!str.isEmpty())
                    {
                        // 删词
                        if (!op)
                        {
                            int pos = userWordList.indexOf(str);
                            if (pos >= 0)
                            {
                                userWordList.removeAt(pos);
                                count++;
                            }
                        }
                        // 加词
                        else
                        {
                            userWordList << str;
                            count++;
                        }
                    }
                }
            }

            if (op) // 加词
            {
                userWordList.removeFirst();
                userWordList.removeDuplicates();
                userWordList.sort();
                userWordList.insert(0, "[UserWord]");
            }

            if (count)
            {
                userWordFile.resize(0);
                foreach(QString str, userWordList)
                {
                    if (!str.isEmpty())
                    {
                        userWordStream << str + "\n";
                    }
                }
                userWordStream.flush();
            }

            userWordFile.close();
        }

        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Information);
        msgBox->setText(QString(_("Success %1 user word count: %2")).arg(op ? _("add") : _("delete")).arg(count));
        msgBox->setStandardButtons(QMessageBox::Ok);
        msgBox->button(QMessageBox::Ok)->setIcon(QIcon());
        msgBox->button(QMessageBox::Ok)->setText(_("Confirm(&OK)"));
        msgBox->setDefaultButton(QMessageBox::Ok);
        msgBox->exec();
        delete msgBox;

        if (count)
        {
            emit signal_reload_dictionaries(freewb::DictReloadUserWord);
        }
    }
}
