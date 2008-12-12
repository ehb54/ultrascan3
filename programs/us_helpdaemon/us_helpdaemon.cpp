//! \file us_helpdaemon.cpp
#include <QtSingleApplication>

#include "us_helpdaemon.h"

us_helpdaemon::us_helpdaemon( const QString& page, QObject* o ) : QObject( o )
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

void us_helpdaemon::close( int /*exitCode*/, QProcess::ExitStatus /*status*/ ) 
{
  exit( 0 );
}
void us_helpdaemon::show( const QString& page )
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

int main( int argc, char* argv[] )
{
  //////////////////////  Need to add uid to identifier ????
  //note: for doc files, it may be necessary to remove 
  //       ~/.local/share/data/Trolltech/Assistant/manual.qhc
  QtSingleApplication application( "UltraScan Help Daemon", 
      argc, argv );
  
  QString message = QString( argv[ 1 ] );

  if ( application.sendMessage( message ) ) return 0;

  application.initialize();
  us_helpdaemon* daemon = new us_helpdaemon( message );
 
  QObject::connect( &application, SIGNAL( messageReceived( const QString& ) ),
                    daemon,       SLOT  ( show           ( const QString& ) ) );

  //application.setActivationWindow( &daemon );
  return application.exec();
}
