#include "backupdialog.h"

#include "config.h"
#include "settings.h"
#include "settingshelper.h"
#include "ui_backupdialog.h"
#include "waylandwinhelper.h"

#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"

#define QSS_BTN_CLOSE0 "border-image: url(:/image/setting/close0.png);"
#define QSS_BTN_CLOSE1 "border-image: url(:/image/setting/close1.png);"
#define QSS_BTN_CLOSE2 "border-image: url(:/image/setting/close2.png);"

#define FILE_WUBI_TABLE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/wbzx.mb"
#define FILE_PINYIN_TABLE INSTALL_DIR + "/data/mb/" + toQStringUtf8(settings::instance().get_curUsedLexicon()) + "/pinyin.mb"
#define FILE_USER_TABLE INSTALL_DIR + "/data/user_word.txt"
#define FILE_QUICK_TABLE INSTALL_DIR + "/data/quick_table.txt"
#define FILE_SETTINGS INSTALL_DIR + "/config/config.ini"
#define DIR_DEFAULT_BACKUP INSTALL_DIR + "/backup"
#define FILE_BACKUP DIR_DEFAULT_BACKUP + "/freewb_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".bak"

const qint32 g_backupFileHead = 1618188;

BackupWorker::BackupWorker(QObject *parent) : QObject(parent)
{
}

BackupWorker::~BackupWorker()
{
}

void BackupWorker::set_op_param(int opFlg, const QString &backupFile)
{
    m_opFlg = opFlg;
    m_backupFile = backupFile;
}

void BackupWorker::slot_start_work()
{
    if (m_opFlg)
    {
        start_restore();
    }
    else
    {
        start_backup();
    }
}

void BackupWorker::start_backup()
{
#define BUF_SIZE (1024 * 4)
    QString curUsedLexiconName = toQStringUtf8(settings::instance().get_curUsedLexicon());
    qint32 lexiconNameSize = curUsedLexiconName.toUtf8().length();
    qint32 wbTableSize = 0;
    qint32 pyTableSize = 0;
    qint32 userTableSize = 0;
    qint32 quickTableSize = 0;
    qint32 settingFileSize = 0;
    QFile wbTableFile(FILE_WUBI_TABLE);
    QFile pyTableFile(FILE_PINYIN_TABLE);
    QFile userTableFile(FILE_USER_TABLE);
    QFile quickTableFile(FILE_QUICK_TABLE);
    QFile settingFile(FILE_SETTINGS);

    int failedFlg = 0;

    QFile backupFile(m_backupFile);
    QDataStream backupStream(&backupFile);
    if (!backupFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qWarning() << m_backupFile << "open failed!";
        failedFlg = 1;
    }
    else
    {
        backupFile.seek(sizeof(g_backupFileHead) + sizeof(lexiconNameSize) + sizeof(wbTableSize) + sizeof(pyTableSize) +
                        sizeof(userTableSize) + sizeof(quickTableSize) + sizeof(settingFileSize));
        QDataStream dataStream;
        char *bytebuf = new char[BUF_SIZE];

        // 写入词库名
        backupStream.writeRawData(curUsedLexiconName.toUtf8().data(), lexiconNameSize);

        // 备份五笔词库
        if (!wbTableFile.open(QIODevice::ReadOnly))
        {
            qWarning() << FILE_WUBI_TABLE << "open failed!";
            failedFlg = 1;
        }
        else
        {
            dataStream.setDevice(&wbTableFile);
            while (1)
            {
                int size = dataStream.readRawData(bytebuf, BUF_SIZE);
                if (size > 0)
                {
                    backupStream.writeRawData(bytebuf, size);
                    wbTableSize += size;
                }
                else
                {
                    break;
                }
            }
            wbTableFile.close();
            // qDebug() << "wubimb file size: " << wbTableSize;
        }

        // 备份拼音词库
        if (!pyTableFile.open(QIODevice::ReadOnly))
        {
            qWarning() << FILE_PINYIN_TABLE << "open failed!";
        }
        else
        {
            dataStream.setDevice(&pyTableFile);
            while (1)
            {
                int size = dataStream.readRawData(bytebuf, BUF_SIZE);
                if (size > 0)
                {
                    backupStream.writeRawData(bytebuf, size);
                    pyTableSize += size;
                }
                else
                {
                    break;
                }
            }
            pyTableFile.close();
            // qDebug() << "pinyinmb file size: " << pyTableSize;
        }

        // 备份用户词组表
        if (!userTableFile.open(QIODevice::ReadOnly))
        {
            qWarning() << FILE_USER_TABLE << "open failed!";
        }
        else
        {
            dataStream.setDevice(&userTableFile);
            while (1)
            {
                int size = dataStream.readRawData(bytebuf, BUF_SIZE);
                if (size > 0)
                {
                    backupStream.writeRawData(bytebuf, size);
                    userTableSize += size;
                }
                else
                {
                    break;
                }
            }
            userTableFile.close();
            // qDebug() << "userword file size: " << userTableSize;
        }

        // 备份快捷码表
        if (!quickTableFile.open(QIODevice::ReadOnly))
        {
            qWarning() << FILE_QUICK_TABLE << "open failed!";
        }
        else
        {
            dataStream.setDevice(&quickTableFile);
            while (1)
            {
                int size = dataStream.readRawData(bytebuf, BUF_SIZE);
                if (size > 0)
                {
                    backupStream.writeRawData(bytebuf, size);
                    quickTableSize += size;
                }
                else
                {
                    break;
                }
            }
            quickTableFile.close();
            // qDebug() << "quick table file size: " << quickTableSize;
        }

        // 备份用户设置文件
        if (!settingFile.open(QIODevice::ReadOnly))
        {
            qWarning() << FILE_SETTINGS << "open failed!";
        }
        else
        {
            dataStream.setDevice(&settingFile);
            while (1)
            {
                int size = dataStream.readRawData(bytebuf, BUF_SIZE);
                if (size > 0)
                {
                    backupStream.writeRawData(bytebuf, size);
                    settingFileSize += size;
                }
                else
                {
                    break;
                }
            }
            settingFile.close();
            // qDebug() << "settings file size: " << sysSettingSize;
        }

        // 写入文件头部信息
        backupFile.seek(0);
        backupStream << g_backupFileHead << lexiconNameSize << wbTableSize << pyTableSize << userTableSize << quickTableSize
                     << settingFileSize;

        backupFile.close();
        delete[] bytebuf;
    }

    if (!failedFlg)
    {
        emit signal_process_updated(m_opFlg, 100);
    }
    else
    {
        emit signal_process_updated(m_opFlg, -1);
    }
}

void BackupWorker::start_restore()
{
#define BUF_SIZE (1024 * 4)
    qint32 backupFileHead;
    qint32 lexiconNameSize = 0;
    qint32 wbTableSize = 0;
    qint32 pyTableSize = 0;
    qint32 userTableSize = 0;
    qint32 quickTableSize = 0;
    qint32 settingFileSize = 0;
    QFile wbTableFile;
    QFile pyTableFile;
    QFile userTableFile(FILE_USER_TABLE);
    QFile quickTableFile(FILE_QUICK_TABLE);
    QFile settingFile(FILE_SETTINGS);
    int failedFlg = 0;

    QFile backupFile(m_backupFile);
    QDataStream backupStream(&backupFile);
    if (!backupFile.open(QIODevice::ReadOnly))
    {
        qWarning() << m_backupFile << "open failed!";
        failedFlg = 1;
    }
    else
    {
        // 读取文件头部信息
        quint32 dataHeadSize = sizeof(g_backupFileHead) + sizeof(lexiconNameSize) + sizeof(wbTableSize) + sizeof(pyTableSize) +
                               sizeof(userTableSize) + sizeof(quickTableSize) + sizeof(settingFileSize);
        backupStream >> backupFileHead >> lexiconNameSize >> wbTableSize >> pyTableSize >> userTableSize >> quickTableSize >>
            settingFileSize;

        char str[128] = {0};
        Q_ASSERT(lexiconNameSize < 128);
        backupStream.readRawData(str, lexiconNameSize);
        QString backupLexiconName(str);

        quint32 dataSize =
            static_cast<quint32>(lexiconNameSize + wbTableSize + pyTableSize + userTableSize + quickTableSize + settingFileSize);
        // 判断备份文件是否有效
        if ((backupFileHead != g_backupFileHead) || (dataHeadSize + dataSize != backupFile.size()))
        {
            qWarning() << m_backupFile << "is invalid!";
            failedFlg = 1;
        }
        else
        {
            QDataStream dataStream;
            char *bytebuf = new char[BUF_SIZE];

            QString lexiconDir = INSTALL_DIR + "/data/mb/" + backupLexiconName;
            if (!QDir(lexiconDir).exists())
            {
                if (!QDir().mkdir(lexiconDir))
                {
                    qWarning() << lexiconDir << "create failed!";
                }
            }

            // 恢复五笔词库
            wbTableFile.setFileName(lexiconDir + "/wbzx.mb");
            if (!wbTableFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << FILE_WUBI_TABLE << "open failed!";
            }
            else
            {
                backupFile.seek(dataHeadSize + static_cast<quint32>(lexiconNameSize));
                dataStream.setDevice(&wbTableFile);
                qint32 readBytes = 0, totalWriteBytes = 0;
                while (totalWriteBytes < wbTableSize)
                {
                    if (totalWriteBytes + BUF_SIZE <= wbTableSize)
                    {
                        readBytes = BUF_SIZE;
                    }
                    else
                    {
                        readBytes = wbTableSize - totalWriteBytes;
                    }

                    int size = backupStream.readRawData(bytebuf, readBytes);
                    if (size > 0)
                    {
                        dataStream.writeRawData(bytebuf, size);
                        totalWriteBytes += size;
                    }
                    else
                    {
                        break;
                    }
                }
                wbTableFile.close();
            }

            // 恢复拼音词库
            pyTableFile.setFileName(lexiconDir + "/pinyin.mb");
            if (!pyTableFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << FILE_PINYIN_TABLE << "open failed!";
            }
            else
            {
                backupFile.seek(dataHeadSize + static_cast<quint32>(lexiconNameSize) + static_cast<quint32>(wbTableSize));
                dataStream.setDevice(&pyTableFile);
                qint32 readBytes = 0, totalWriteBytes = 0;
                while (totalWriteBytes < pyTableSize)
                {
                    if (totalWriteBytes + BUF_SIZE <= pyTableSize)
                    {
                        readBytes = BUF_SIZE;
                    }
                    else
                    {
                        readBytes = pyTableSize - totalWriteBytes;
                    }

                    int size = backupStream.readRawData(bytebuf, readBytes);
                    if (size > 0)
                    {
                        dataStream.writeRawData(bytebuf, size);
                        totalWriteBytes += size;
                    }
                    else
                    {
                        break;
                    }
                }
                pyTableFile.close();
            }

            // 恢复用户词组表
            if (!userTableFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << FILE_USER_TABLE << "open failed!";
            }
            else
            {
                backupFile.seek(dataHeadSize + static_cast<quint32>(lexiconNameSize) + static_cast<quint32>(wbTableSize) +
                                static_cast<quint32>(pyTableSize));
                dataStream.setDevice(&userTableFile);
                qint32 readBytes = 0, totalWriteBytes = 0;
                while (totalWriteBytes < userTableSize)
                {
                    if (totalWriteBytes + BUF_SIZE <= userTableSize)
                    {
                        readBytes = BUF_SIZE;
                    }
                    else
                    {
                        readBytes = userTableSize - totalWriteBytes;
                    }

                    int size = backupStream.readRawData(bytebuf, readBytes);
                    if (size > 0)
                    {
                        dataStream.writeRawData(bytebuf, size);
                        totalWriteBytes += size;
                    }
                    else
                    {
                        break;
                    }
                }
                userTableFile.close();
            }

            // 恢复快捷码表
            if (!quickTableFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << FILE_QUICK_TABLE << "open failed!";
            }
            else
            {
                backupFile.seek(dataHeadSize + static_cast<quint32>(lexiconNameSize) + static_cast<quint32>(wbTableSize) +
                                static_cast<quint32>(pyTableSize) + static_cast<quint32>(userTableSize));
                dataStream.setDevice(&quickTableFile);
                qint32 readBytes = 0, totalWriteBytes = 0;
                while (totalWriteBytes < quickTableSize)
                {
                    if (totalWriteBytes + BUF_SIZE <= quickTableSize)
                    {
                        readBytes = BUF_SIZE;
                    }
                    else
                    {
                        readBytes = quickTableSize - totalWriteBytes;
                    }

                    int size = backupStream.readRawData(bytebuf, readBytes);
                    if (size > 0)
                    {
                        dataStream.writeRawData(bytebuf, size);
                        totalWriteBytes += size;
                    }
                    else
                    {
                        break;
                    }
                }
                quickTableFile.close();
            }

            // 恢复用户设置文件
            if (!settingFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                qWarning() << FILE_SETTINGS << "open failed!";
            }
            else
            {
                backupFile.seek(dataHeadSize + static_cast<quint32>(lexiconNameSize) + static_cast<quint32>(wbTableSize) +
                                static_cast<quint32>(pyTableSize) + static_cast<quint32>(userTableSize) +
                                static_cast<quint32>(quickTableSize));
                dataStream.setDevice(&settingFile);
                qint32 readBytes = 0, totalWriteBytes = 0;
                while (totalWriteBytes < settingFileSize)
                {
                    if (totalWriteBytes + BUF_SIZE <= settingFileSize)
                    {
                        readBytes = BUF_SIZE;
                    }
                    else
                    {
                        readBytes = settingFileSize - totalWriteBytes;
                    }

                    int size = backupStream.readRawData(bytebuf, readBytes);
                    if (size > 0)
                    {
                        dataStream.writeRawData(bytebuf, size);
                        totalWriteBytes += size;
                    }
                    else
                    {
                        break;
                    }
                }
                settingFile.close();
            }

            backupFile.close();
            delete[] bytebuf;
        }
    }

    if (!failedFlg)
    {
        emit signal_process_updated(m_opFlg, 100);
    }
    else
    {
        emit signal_process_updated(m_opFlg, -1);
    }
}

BackupDialog::BackupDialog(QDialog *parent) : QDialog(parent), ui(new Ui::BackupDialog)
{
    ui->setupUi(this);
    setUiTexts();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint((d->width() - size().width()) / 2, (d->height() - size().height()) / 2);

    m_backupThread = new QThread(this);
    m_backupWorker = new BackupWorker();
    m_backupWorker->moveToThread(m_backupThread);
    connect(m_backupThread, &QThread::started, m_backupWorker, &BackupWorker::slot_start_work);
    connect(m_backupWorker, &BackupWorker::signal_process_updated, this, &BackupDialog::slot_progress_updated);

    ui->btnClose->installEventFilter(this);

    freewb::applyWaylandOverlayWindowHints(this);
}

void BackupDialog::setUiTexts()
{
    ui->label->setText(_("Backup in progress, please wait..."));
    ui->label_2->setText(_("Backup completed. Open the backup file?"));
    ui->btnYes->setText(_("Yes"));
    ui->btnNo->setText(_("No"));
    ui->btnOk->setText(_("OK"));
    ui->labelPrompt->setText(_("Dictionary and settings recovery completed!"));
}

BackupDialog::~BackupDialog()
{
    delete ui;
    if (m_backupWorker)
    {
        delete m_backupWorker;
    }
}

void BackupDialog::backup_thread_quit()
{
    if (m_backupThread)
    {
        if (m_backupThread->isRunning())
        {
            m_backupThread->quit();
            m_backupThread->wait();
        }
    }
}

void BackupDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }

    QWidget::mousePressEvent(event);
}

void BackupDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_mouseIsPressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void BackupDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseIsPressed)
    {
        QPoint mouseCurrPosition = event->globalPos();
        move(pos() + mouseCurrPosition - m_mouseLastPosition);
        m_mouseLastPosition = mouseCurrPosition;
    }

    QWidget::mouseMoveEvent(event);
}

bool BackupDialog::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (event->type() == QEvent::WindowActivate)
    {
        ui->labelWin->setStyleSheet(QSS_BORDER_ACTIVE);
        ui->btnClose->setStyleSheet(QSS_BTN_CLOSE1);
    }
    else if (event->type() == QEvent::WindowDeactivate)
    {
        ui->labelWin->setStyleSheet(QSS_BORDER_DEACTIVE);
        ui->btnClose->setStyleSheet(QSS_BTN_CLOSE0);
    }
    else if (event->type() == QEvent::Enter && obj == ui->btnClose)
    {
        ui->btnClose->setStyleSheet(QSS_BTN_CLOSE2);
    }
    else if (event->type() == QEvent::Leave && obj == ui->btnClose)
    {
        if (isActiveWindow())
        {
            ui->btnClose->setStyleSheet(QSS_BTN_CLOSE1);
        }
        else
        {
            ui->btnClose->setStyleSheet(QSS_BTN_CLOSE0);
        }
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }

    return isProcessed;
}

void BackupDialog::slot_progress_updated(int opFlg, int percentage)
{
    if (percentage >= 100)
    {
        backup_thread_quit(); // 请求线程退出
        if (opFlg == 0)
        {
            // ui->stackedWidget->setCurrentWidget( ui->pageBackupOk );
            ui->labelPrompt->setText(_("Dictionary and settings backup completed"));
            ui->labelDir->setText(m_backupFile);
            ui->stackedWidget->setCurrentWidget(ui->pagePrompt);
        }
        else
        {
            ui->labelPrompt->setText(_("Dictionary and settings recovery completed！"));
            ui->labelDir->setText("");
            ui->stackedWidget->setCurrentWidget(ui->pagePrompt);

            settings::instance().set_imeTableChanged(1);
            emit signal_restore_lexicon_and_settings_ok();
        }
    }
    else if (percentage < 0)
    {
        backup_thread_quit(); // 请求线程退出
        ui->labelPrompt->setText(QString(_("Dictionary and settings %1 failed！")).arg(opFlg ? _("recovery") : _("backup")));
        ui->stackedWidget->setCurrentWidget(ui->pagePrompt);
    }
    else
    {
        ui->progressBar->setValue(percentage);
    }
}

void BackupDialog::slot_backup_lexicon_and_settings()
{
    QDir backupDir(DIR_DEFAULT_BACKUP);
    if (!backupDir.exists())
    {
        backupDir.mkdir(DIR_DEFAULT_BACKUP);
    }

    m_backupFile = QFileDialog::getSaveFileName(this, _("Generate backup file"), FILE_BACKUP);
    if (!m_backupFile.isEmpty())
    {
        m_backupWorker->set_op_param(0, m_backupFile);
        m_backupThread->start();

        ui->stackedWidget->setCurrentWidget(ui->pageBackupRestore);
        ui->label->setText(_("Backup in progress, please wait..."));
        move(m_defaultPopPosition);
        exec();
    }
}

void BackupDialog::slot_restore_lexicon_and_settings()
{
    QDir backupDir(DIR_DEFAULT_BACKUP);
    if (!backupDir.exists())
    {
        backupDir.mkdir(DIR_DEFAULT_BACKUP);
    }

    QString restoreFile = QFileDialog::getOpenFileName(this, _("Select backup file"), FILE_BACKUP);
    if (!restoreFile.isEmpty())
    {
        m_backupWorker->set_op_param(1, restoreFile);
        m_backupThread->start();

        ui->stackedWidget->setCurrentWidget(ui->pageBackupRestore);
        ui->label->setText(_("Recovery in progress, please wait..."));
        move(m_defaultPopPosition);
        exec();
    }
}

void BackupDialog::on_btnClose_clicked()
{
    backup_thread_quit();
    accept();
}

void BackupDialog::on_btnYes_clicked()
{
    char cmd[128] = {0};
    sprintf(cmd, "xdg-open %s > /dev/null 2>&1 &", m_backupFile.toUtf8().data());
    system(cmd);

    accept();
}

void BackupDialog::on_btnNo_clicked()
{
    accept();
}

void BackupDialog::on_btnOk_clicked()
{
    accept();
}
