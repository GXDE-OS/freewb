#include "candidateitem.h"

#include <QTimer>

#include "ui_candidateitem.h"

CandidateItem::CandidateItem(QWidget *parent) : QWidget(parent), ui(new Ui::CandidateItem)
{
    ui->setupUi(this);
    adjustSize();
}

CandidateItem::~CandidateItem()
{
    delete ui;
}

bool CandidateItem::eventFilter(QObject *obj, QEvent *event)
{
    bool isProcessed = false;

    if (obj == ui->labelWord)
    {
        if (event->type() == QEvent::Enter)
        {
            m_hoverFlg = true;
            m_hoverTimer->start(300);
        }
        else if (event->type() == QEvent::Leave)
        {
            m_hoverFlg = false;
            m_hoverTimer->stop();
        }
    }

    if (isProcessed == false)
    {
        return QWidget::eventFilter(obj, event);
    }
    return isProcessed;
}

void CandidateItem::slot_cursor_hover_timeout()
{
    if (m_hoverFlg)
    {
        emit signal_cursor_hover(m_wordText);
    }
}

// 设置候选词与提示信息的光标
void CandidateItem::set_word_text_cursor()
{
    ui->labelWord->setCursor(QCursor(Qt::PointingHandCursor));

    m_hoverFlg = false;
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    ui->labelWord->installEventFilter(this);
    connect(m_hoverTimer, &QTimer::timeout, this, &CandidateItem::slot_cursor_hover_timeout);
}

// 设置候选词与提示信息
void CandidateItem::set_text(const QString &label, const QString &wordText, const QString &promptText)
{
    m_wordText = wordText;
    QString tmp = wordText;

    if (tmp.length() > m_maxCharCount + 3)
    {
        tmp = tmp.left(m_maxCharCount - 2) + "…" + tmp.right(2);
    }
    // puts(tmp.toUtf8().constData());
    ui->labelWord->setText(label + tmp);
    ui->labelPrompt->setText(promptText);
    adjustSize();
}

// 清除候选词与提示信息
void CandidateItem::clear_text()
{
    m_wordText.clear();
    ui->labelWord->clear();
    ui->labelPrompt->clear();
    adjustSize();
}

// 设置候选内容字体
void CandidateItem::set_text_font(const QFont &font)
{
    ui->labelWord->setFont(font);
    ui->labelPrompt->setFont(font);
}

// 设置鼠标在候选词上停留时的颜色
void CandidateItem::set_hover_color(const QColor &color)
{
    m_hoverColor = color;
    QString str = QString("QLabel:hover{color:rgb(%1,%2,%3)}QLabel{color:rgb(%4,%5,%6)}")
                      .arg(color.red())
                      .arg(color.green())
                      .arg(color.blue())
                      .arg(m_wordColor.red())
                      .arg(m_wordColor.green())
                      .arg(m_wordColor.blue());

    ui->labelWord->setStyleSheet(str);
}

// 设置候选词颜色
void CandidateItem::set_word_text_color(const QColor &color)
{
    m_wordColor = color;
    QString str = QString("QLabel{color:rgb(%1,%2,%3)}QLabel:hover{color:rgb(%4,%5,%6)}")
                      .arg(color.red())
                      .arg(color.green())
                      .arg(color.blue())
                      .arg(m_hoverColor.red())
                      .arg(m_hoverColor.green())
                      .arg(m_hoverColor.blue());

    ui->labelWord->setStyleSheet(str);
}

// 设置候选词提示信息颜色
void CandidateItem::set_prompt_text_color(const QColor &color)
{
    QString str = QString("color:rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    ui->labelPrompt->setStyleSheet(str);
}

// 设置候选词能显示最多的字符数
void CandidateItem::set_disp_max_char_count(int count)
{
    m_maxCharCount = count;
}

// 获取候选词内容
const QString &CandidateItem::get_word_text()
{
    return m_wordText;
}
