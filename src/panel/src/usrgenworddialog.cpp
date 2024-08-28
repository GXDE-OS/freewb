#include "usrgenworddialog.h"
#include "ui_usrgenworddialog.h"
#include "commdefine.h"
#include "settings.h"


#define QSS_BORDER_ACTIVE "color: rgb(255, 255, 255);background-color: rgb(10, 120, 203);"
#define QSS_BORDER_DEACTIVE "color: rgb(0, 0, 0);background-color: rgb(200, 200, 200);"


UsrGenWordDialog::UsrGenWordDialog( QWidget *parent ) : QDialog(parent), ui(new Ui::UsrGenWordDialog)
{
    ui->setupUi(this);
    //setWindowFlags( Qt::Tool | Qt::FramelessWindowHint );
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::Tool);
    m_mouseIsPressed = false;
    m_mouseLastPosition = QPoint();

    QRegExp rx( "^[a-z]{1,16}$" );
    QRegExpValidator *validator = new QRegExpValidator( rx, this );
    ui->ledtWordCode->setValidator( validator );

    QDesktopWidget *d = QApplication::desktop();
    m_defaultPopPosition = QPoint( (d->width() - size().width())/2, (d->height() - size().height())/2 );

    m_userWordFile = INSTALL_DIR + "/data/user_word.txt";

    init_user_word_file();
}

UsrGenWordDialog::~UsrGenWordDialog()
{
    delete ui;
}


bool UsrGenWordDialog::eventFilter( QObject *obj, QEvent *event )
{
    bool isProcessed = false;
     QKeyEvent *keyEvt = static_cast<QKeyEvent *>( event );

    if(keyEvt->key()==Qt::Key_Escape)
    {
        on_btnExit_clicked();
    }   

    if ( isProcessed == false )
    {
        return QWidget::eventFilter( obj, event );
    }

    return isProcessed;
}

void UsrGenWordDialog::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_mouseIsPressed = true;
        m_mouseLastPosition = event->globalPos();
    }
}


void UsrGenWordDialog::mouseReleaseEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        m_mouseIsPressed = false;
    }
}


void UsrGenWordDialog::mouseMoveEvent( QMouseEvent *event )
{
    if( m_mouseIsPressed )
     {
        QPoint mouseCurrPosition = event->globalPos();
        move( pos() + mouseCurrPosition - m_mouseLastPosition );
        m_mouseLastPosition = mouseCurrPosition;
    }
}



void UsrGenWordDialog::init_user_word_file()
{
    QFile textFile( m_userWordFile );
    QTextStream textStream( &textFile );
    textStream.setCodec( "UTF-8" );

    if( !textFile.exists() || textFile.size()<15 )
    {
        if ( !textFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) )
        {
            qWarning() << textFile.fileName() << " open failed!";
            return;
        }

        QStringList strList;
        strList << "date=$y年$m月$d日\n";
        strList << "date=$Y年$M月$D日\n";
        strList << "day=$D日\n";
        strList << "day=$d日\n";
        strList << "hour=$H时\n";
        strList << "hour=$h时\n";
        strList << "joke=嘦巭好，兲嫑跑*_*!\n";
        strList << "minute=$MI分\n";
        strList << "minute=$mi分\n";
        strList << "month=$M月\n";
        strList << "month=$m月\n";
        strList << "now=$y年$m月$d日$0h时$0mi分$0s秒\n";
        strList << "second=$S秒\n";
        strList << "second=$s秒\n";
        strList << "time=$h时$mi分\n";
        strList << "time=$H时$MI分\n";
        strList << "week=$W\n";
        strList << "week=$w\n";
        strList << "year=$Y年\n";
        strList << "year=$y年\n";

        strList.sort();
        strList.insert( 0, "[UserWord]\n" );

        foreach( QString str, strList )
        {
            textStream << str;
        }
        textStream.flush();
        textFile.close();

        Settings::save_userWord_change_flg_to_file( 1 );
    }
}


void UsrGenWordDialog::add_user_word( const QString &wordText, const QString &wordCode )
{
    QFile textFile( m_userWordFile );
    QTextStream textStream( &textFile );
    textStream.setCodec( "UTF-8" );

    if ( !textFile.open(QIODevice::ReadWrite | QIODevice::Text) )
    {
        qWarning() << textFile.fileName() << " open failed!";
        return;
    }

    QStringList strList = textStream.readAll().split( '\n' );
    textFile.resize( 0 );

    if ( strList.length() )
    {
        strList.takeAt( 0 );
    }
    strList << QString("%1=%2").arg(wordCode).arg(wordText);
    strList.removeDuplicates();
    strList.sort();
    strList.insert( 0, "[UserWord]" );

    foreach( QString str, strList )
    {
        if ( !str.isEmpty() )
        {
            textStream << str + "\n";
        }
    }
    textStream.flush();
    textFile.close();
}

void UsrGenWordDialog::delete_user_word( const QString &wordText, const QString &wordCode )
{
    QFile textFile( m_userWordFile );
    QTextStream textStream( &textFile );
    textStream.setCodec( "UTF-8" );

    if ( !textFile.open(QIODevice::ReadWrite | QIODevice::Text) )
    {
        qWarning() << textFile.fileName() << " open failed!";
        return;
    }

    QStringList strList = textStream.readAll().split( '\n' );
    textFile.resize( 0 );
    if ( strList.length() )
    {
        int pos;
        if ( (pos = strList.indexOf(QString("%1=%2").arg(wordCode).arg(wordText))) != -1 )
        {
            strList.removeAt( pos );
        }
    }

    foreach( QString str, strList )
    {
        if ( !str.isEmpty() )
        {
            textStream << str + "\n";
        }
    }
    textStream.flush();
    textFile.close();
}


void UsrGenWordDialog::slot_show_dialog( const QString &wordText, const QString &wordCode )
{
    ui->ledtWordText->setText( wordText );
    ui->ledtWordCode->setText( wordCode );
    if( wordCode.length()<1 )
    {
        ui->ledtWordCode->setFocus();
    }
    exec();
}

void UsrGenWordDialog::slot_userWord_file_saved()
{
    Settings::save_userWord_change_flg_to_file( 1 );
    emit signal_user_word_changed();
}



void UsrGenWordDialog::on_btnOk_clicked()
{
    add_user_word( ui->ledtWordText->text(), ui->ledtWordCode->text() );
    accept();

    Settings::save_userWord_change_flg_to_file( 1 );
    emit signal_user_word_changed();
}

void UsrGenWordDialog::on_btnExit_clicked()
{
    ui->ledtWordText->clear();
    ui->ledtWordCode->clear();
    accept();
}

