//! \file us_helpdaemon.cpp
#include <QtSingleApplication>

#include "us_helpdaemon.h"

US_HelpDaemon::US_HelpDaemon( const QString& page, QObject* o ) : QObject( o )
{
  QString location = qApp->applicationDirPath() + "/manual.qhc";

  args << QLatin1String( "-collectionFile" )
       << QLatin1String( location.toAscii().data() )
       << QLatin1String( "-enableRemoteControl" );

  daemon.start( QLatin1String( "assistant" ), args );
  daemon.waitForStarted();

  connect( &daemon, SIGNAL( finished ( int, QProcess::ExitStatus ) ),
                    SLOT  ( close    ( int, QProcess::ExitStatus ) ) );

  ts.setDevice ( &daemon );
  show( QString( page ) );
}

void US_HelpDaemon::close( int /*exitCode*/, QProcess::ExitStatus /*status*/ ) 
{
  exit( 0 );
}
void US_HelpDaemon::show( const QString& page )
{
  if ( page == "Quit" )
  {
    daemon.close();
    exit( 0 );
  }

  if ( daemon.state() == QProcess::NotRunning ) 
  {
    qDebug() << "assistant not running";
  }

  QString message = "setSource qthelp://ultrascaniii/" + page + "\0";
  ts << QLatin1String( message.toAscii().data() ) << endl;
}

/*! \brief Main program to start up Qt's Assistant

    This program uses QtSingleApplication to ensure that there is only
    one active process.  When a second instance is launched, it just sends
    a message (the page to show) to the first instance and exits.
*/
int main( int argc, char* argv[] )
{
  //  Need to add uid to identifier ????
  //note: for doc files to show properly after an update, it may be necessary 
  //      to remove  ~/.local/share/data/Trolltech/Assistant/manual.qhc
  QtSingleApplication application( "UltraScan Help Daemon", 
      argc, argv );
  
  QString message = QString( argv[ 1 ] );

  if ( application.sendMessage( message ) ) return 0;

  application.initialize();
  US_HelpDaemon* daemon = new US_HelpDaemon( message );
 
  QObject::connect( &application, SIGNAL( messageReceived( const QString& ) ),
                    daemon,       SLOT  ( show           ( const QString& ) ) );

  return application.exec();
}

