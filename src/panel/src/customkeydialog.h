/****************************************************************************************
** @作者：lcj
**
** @说明：
**      CustomKeyDialog是一个从QDialog类继承而来的UI类，其对应的UI设计文件为customkeydialog.ui
**      ，该类设计目的为被SettingWin类所包含实例化，在用户设置自定义软键盘按键字符时弹出自定义字符输入
**      对话框。
******************************************************************************×*********/

#ifndef CUSTOMKEYDIALOG_H
#define CUSTOMKEYDIALOG_H

#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QMouseEvent>
#include <QPoint>

#include "keyboard.h"

namespace Ui
{
class CustomKeyDialog;
}

class CustomKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomKeyDialog(QWidget *parent = nullptr);
    ~CustomKeyDialog();

signals:
    void signal_custom_ok_btn_clicked(const QString &commSymbol, const QString &shiftSymbol); // 自定义案件符号确定

public:
    void set_custom_symbol(VirtualKeyboardMode vkm, const QString &keyName, const QString &commSymbol,
                           const QString &shiftSymbol);

protected:
    // 重载函数，用于窗口拖动
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void on_btnOk_clicked();
    void on_btnCancle_clicked();

private:
    void setUiTexts();

private:
    Ui::CustomKeyDialog *ui;

    // 用于窗口拖动计算
    bool m_mouseIsPressed;
    QPoint m_mouseLastPosition;

    QPoint m_defaultPopPosition;
};

#endif
