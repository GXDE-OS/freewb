#include "keybutton.h"
#include "keybutton.h"
#include "commdefine.h"



KeyButton::KeyButton( QWidget *parent ): QPushButton ( parent )
{
}

void KeyButton::set_key_name( const QString &name )
{
    m_keyName = name;
}

const QString &KeyButton::get_key_name()
{
    return m_keyName;
}


void KeyButton::set_custom_symbol( const QString &commSymbol, const QString &shiftSymbol )
{
    m_commSymbol = commSymbol;
    m_shiftSymbol = shiftSymbol;
}


void KeyButton::paintEvent( QPaintEvent *paintEvent )
{
    QPushButton::paintEvent( paintEvent );

    QPainter painter(this);

    painter.setFont( QFont( "Ubuntu", 8) );

    if ( !m_commSymbol.isEmpty() )
    {
        painter.setPen( QColor(Qt::blue) );
        painter.drawText( 5, height()-3, m_commSymbol );
    }

    if ( !m_shiftSymbol.isEmpty() )
    {
        painter.setPen( QColor(Qt::red) );
        painter.drawText( width()/2+2, height()/2, m_shiftSymbol );
    }
}
