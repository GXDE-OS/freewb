#ifndef LEXICONTOOLWIN_H
#define LEXICONTOOLWIN_H

#include <QAbstractButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPoint>
#include <QProcess>
#include <QThread>
#include <QWidget>

namespace Ui
{
class LexiconToolWin;
}

typedef enum
{
    LTO_DUMP_SYS_TABLE,
    LTO_GEN_SYS_TABLE,
    LTO_MARK_RARE_CHAR,
    LTO_MARK_THINK_WORD,
    LTO_OPTIMIZE_TABLE,
    LTO_DUMP_PINYIN_TABLE,
    LTO_GEN_PINYIN_TABLE
} LexiconToolOp;

class LexiconWorker : public QObject
{
    Q_OBJECT

public:
    LexiconWorker(QObject *parent = nullptr);
    ~LexiconWorker();

signals:
    void signal_process_updated(LexiconToolOp opType, int opStatus, int count);

public slots:
    void slot_start_work();

public:
    void set_op_param(LexiconToolOp opType, const QString &srcFile, const QString &destFile);

protected:
private:
    LexiconToolOp m_opType;
    QString m_srcFile;
    QString m_destFile;
    int m_count;
};

class LexiconToolWin : public QWidget
{
    Q_OBJECT

public:
    explicit LexiconToolWin(QWidget *parent = nullptr);
    ~LexiconToolWin();

signals:
    void signal_reload_dictionaries(int mask);

public slots:
    void open_win();

protected:
    // 重载函数,用于窗口拖动
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void add_del_user_word_from_file(int op, const QString &fileName);
    void lexicon_thread_quit();

protected slots:
    void slot_process_updated(LexiconToolOp opType, int opStatus, int count);
    void slot_worker_thread_finished();

private slots:
    void on_btnHelp_clicked();
    void on_btnDumpSysLexicon_clicked();
    void on_btnMakeSysLexicon_clicked();
    void on_btnMarkRareWord_clicked();
    void on_btnMarkThinkWord_clicked();
    void on_btnOptimize_clicked();
    void on_btnDumpPinyinLexicon_clicked();
    void on_btnMakePinyinLexicon_clicked();
    void on_btnDumpUserLexicon_clicked();
    void on_btnBatchDel_clicked();
    void on_btnBatchAdd_clicked();

private:
    void setUiTexts();
    void startDumpLexicon(LexiconToolOp opType, const QString &txtPath, const QString &mbPath);
    int startGenLexicon(LexiconToolOp opType, const QString &txtPath, const QString &mbPath);
    void showLexiconProgressMsgBox(QMessageBox *box, const char *textMsgid);
    void hideLexiconProgressMsgBox(QMessageBox *box);

private:
    Ui::LexiconToolWin *ui;

    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    QPoint m_mouseLastPosition;

    QPoint m_defaultPopPosition;

    QThread *m_lexiconThread;
    LexiconWorker *m_lexiconWorker;
    QMessageBox *m_msgBox;

    QString m_tmpSysTable;
    QString m_tmpPinyinTable;

    QEventLoop *m_opWaitLoop = nullptr;

    int m_workerOpStatus = 0;
    int m_workerOpCount = 0;
};

#endif
