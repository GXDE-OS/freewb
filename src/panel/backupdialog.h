#ifndef BACKUPDIALOG_H
#define BACKUPDIALOG_H

#include <QDateTime>
#include <QDesktopWidget>
#include <QDialog>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QThread>
#include <QWidget>

class BackupWorker : public QObject
{
    Q_OBJECT

public:
    BackupWorker(QObject *parent = nullptr);
    ~BackupWorker();

signals:
    void signal_process_updated(int opFlg, int percentage);

public slots:
    void slot_start_work();

public:
    void set_op_param(int opFlg, const QString &backupFile);

protected:
    void start_backup();
    void start_restore();

private:
    int m_opFlg;
    QString m_backupFile;
};

namespace Ui
{
class BackupDialog;
}

class BackupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BackupDialog(QDialog *parent = nullptr);
    ~BackupDialog();

signals:
    void signal_restore_lexicon_and_settings_ok();

public slots:
    void slot_backup_lexicon_and_settings();
    void slot_restore_lexicon_and_settings();

protected:
    // 重载函数
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

    void backup_thread_quit();

protected slots:
    void slot_progress_updated(int opFlg, int percentage);

private slots:
    void on_btnClose_clicked();
    void on_btnYes_clicked();
    void on_btnNo_clicked();
    void on_btnOk_clicked();

private:
    void setUiTexts();

private:
    Ui::BackupDialog *ui;

    bool m_mouseIsPressed;
    QPoint m_mouseLastPosition;
    QPoint m_defaultPopPosition;

    QThread *m_backupThread;
    BackupWorker *m_backupWorker;
    QString m_backupFile;
};

#endif
