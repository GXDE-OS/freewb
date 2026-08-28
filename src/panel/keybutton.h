#ifndef KEYBUTTON_H
#define KEYBUTTON_H

#include <QChar>
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
