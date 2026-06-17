#ifndef CANDIDATEITEM_H
#define CANDIDATEITEM_H

#include <QDebug>
#include <QEvent>
#include <QWidget>

namespace Ui
{
class CandidateItem;
}

class CandidateItem : public QWidget
{
    Q_OBJECT

public:
    explicit CandidateItem(QWidget *parent = nullptr);
    ~CandidateItem();

signals:
    void signal_cursor_hover(const QString &wordText);
    void signal_clicked();

public:
    void set_text(const QString &label, const QString &wordText, const QString &promptText);
    const QString &get_word_text();
    void clear_text();
    void set_word_text_cursor();
    void set_text_font(const QFont &font);
    void set_hover_color(const QColor &color);
    void set_word_text_color(const QColor &color);
    void set_prompt_text_color(const QColor &color);
    void set_disp_max_char_count(int count);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mouseReleaseEvent(QMouseEvent *event) override;

protected slots:
    void slot_cursor_hover_timeout();

private:
    Ui::CandidateItem *ui;

    int m_maxCharCount;  // 候选词能显示最多的字符数
    QString m_wordText;  // 候选词内容
    QColor m_wordColor;  // 候选词颜色
    QColor m_hoverColor; // 候选词鼠标停留颜色

    QTimer *m_hoverTimer;
    bool m_hoverFlg;
};

#endif
