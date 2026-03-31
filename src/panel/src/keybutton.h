/****************************************************************************************
** @作者：lcj
**
** @说明：
**      KeyButton是一个从QPushButton类继承而来的UI类，该类设计目的为作为Keyboard类的基本自定义按钮，
**      之所以自定义一个按钮类，是为了重写painEvent事件函数在按钮的不同区域绘画出不同的符号用来显示一个
**      真实按键对应的正常字符与上档字符。
*************************************************************************************×**/

#ifndef KEYBUTTON_H
#define KEYBUTTON_H

#include <QChar>
#include <QDebug>
#include <QKeyEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

class KeyButton : public QPushButton
{
    Q_OBJECT

public:
    KeyButton(QWidget *parent = nullptr);

public:
    void set_key_name(const QString &name);
    const QString &get_key_name();
    void set_custom_symbol(const QString &commSymbol, const QString &shiftSymbol);

protected:
    void paintEvent(QPaintEvent *paintEvent);

private:
    QString m_keyName;     // 按键名称
    QString m_commSymbol;  // 按键显示的正常符号
    QString m_shiftSymbol; // 按键显示的上档符号
};

#endif
