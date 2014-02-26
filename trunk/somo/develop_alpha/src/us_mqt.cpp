#include "../include/us_mqt.h"
//Added by qt3to4:
#include <QFocusEvent>
#include <QLabel>
#include <QMouseEvent>

mQLineEdit::mQLineEdit( QWidget *parent, const char * name ) : QLineEdit( parent, name ) {}

mQLineEdit::~mQLineEdit() {}

void mQLineEdit::focusInEvent ( QFocusEvent *e )
{
   QLineEdit::focusInEvent( e );
   emit( focussed( true ) );
}

void mQLineEdit::focusOutEvent ( QFocusEvent *e )
{
   QLineEdit::focusOutEvent( e );
   emit( focussed( false ) );
}

mQLabel::mQLabel( QWidget *parent, const char * name ) : QLabel( parent, name ) {}
mQLabel::mQLabel( const QString & text, QWidget *parent, const char * name ) : QLabel( text, parent, name ) {}

mQLabel::~mQLabel() {}

void mQLabel::mousePressEvent ( QMouseEvent *e )
{
   QLabel::mousePressEvent( e );
   emit( pressed() );
}

mQPushButton::mQPushButton( QWidget *parent, const char * name ) : QPushButton( parent, name ) {}
mQPushButton::mQPushButton( const QString & text, QWidget *parent, const char * name ) : QPushButton( text, parent, name ) {}

mQPushButton::~mQPushButton() {}

void mQPushButton::mouseDoubleClickEvent ( QMouseEvent *e )
{
   QPushButton::mouseDoubleClickEvent( e );
   emit( doubleClicked() );
}

void mQThread::sleep( unsigned long secs )
{
   QThread::sleep( secs );
}

void mQThread::msleep( unsigned long msecs )
{
   QThread::msleep( msecs );
}

void mQThread::usleep( unsigned long usecs )
{
   QThread::usleep( usecs );
}
