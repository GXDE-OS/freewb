#ifndef TEXTEDITWIN_H
#define TEXTEDITWIN_H

#include <QAbstractButton>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDir>
#include <QMainWindow>
#include <QMessageBox>
#include <QtDebug>

#include "textfinddialog.h"

namespace Ui
{
class TextEditWin;
}

enum TextEditMode
{
    TEM_USER_WORD,    // 编辑用户词组
    TEM_QUICK_TABLE,  // 编辑快捷码表
    TEM_SETTING_FILE, // 编辑配置文件
    TEM_WUBI_TABLE,   // 编辑五笔码表
    TEM_PINYIN_TABLE  // 编辑拼音码表
};

class TextEditWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit TextEditWin(QWidget *parent = nullptr);
    ~TextEditWin();

signals:
    void signal_setting_file_changed();
    void signal_userWord_file_saved();
    void signal_quickTable_file_saved();
    void signal_reload_dictionaries(int mask);

public slots:
    void slot_open_textEdit_win(TextEditMode mode);

protected:
    void init_quick_table_file();

    void open_user_word_file();
    void open_quick_table_file();
    void open_wubi_table_file();
    void open_pinyin_table_file();
    void open_setting_file();

    bool save_text_to_file();
    bool close_text_win();

    void closeEvent(QCloseEvent *event);

protected slots:
    void slot_open_find_dialog();
    void slot_save_text();
    void slot_close_win();
    void slot_text_is_changed();
    void slot_find_text(const QString &text, bool prevFlg, bool caseSensitiveFlg, bool wholeWordMatchFlg);

private:
    Ui::TextEditWin *ui;

    QPoint m_defaultPopPosition;

    TextEditMode m_textEditMode;

    TextFindDialog *m_textFindDialog;

    QAction *m_actFind;
    QAction *m_actSave;
    QAction *m_actQuit;

    QString m_textFileName;
    int m_modifiedFlg;
};

#endif
