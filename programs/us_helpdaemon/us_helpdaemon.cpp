//! \file us_helpdaemon.cpp
#include <QtSingleApplication>

#include "us_helpdaemon.h"
#include "us_settings.h"

US_HelpDaemon::US_HelpDaemon( const QString& page, QObject* o ) : QObject( o )
{
  QString location = US_Settings::appBaseDir() + "/bin/manual.qhc";
  QString url      = "qthelp://ultrascaniii/";
  if ( !page.contains( "manual/" ) )
     url.append( "manual/" );
  url.append( page );

  QStringList args;

  args << QLatin1String( "-collectionFile" )
       << location
       << QLatin1String( "-enableRemoteControl" )
       << QLatin1String( "-showURL" )
       << url;

  debug( args.join( " " ) );
#ifndef Q_WS_MAC
  daemon.start( QLatin1String( "assistant" ), args );
#else
  QString assisloc  = US_Settings::appBaseDir() + "/bin/Assistant.app";
  daemon.start( assisloc.toLatin1(), args );
#endif
  daemon.waitForStarted();

  connect( &daemon, SIGNAL( finished ( int, QProcess::ExitStatus ) ),
                    SLOT  ( close    ( int, QProcess::ExitStatus ) ) );
}

void US_HelpDaemon::close( int /*exitCode*/, QProcess::ExitStatus /*status*/ ) 
{
  exit( 0 );
}

void US_HelpDaemon::show( const QString& helpPage )
{
  if ( helpPage == "Quit" )
  {
    daemon.close();
    exit( 0 );
  }

  if ( daemon.state() == QProcess::NotRunning ) 
  {
    debug( "assistant not running" );
  }

  QString page = helpPage;
  if ( ! helpPage.contains( "manual/" ) ) page.prepend( "manual/" );

  debug( "setSource qthelp://ultrascaniii/" + page );

  QByteArray ba;
  ba.append( "setSource qthelp://ultrascaniii/" );
  ba.append( page.toAscii() );
  ba.append( '\0' );

  daemon.write( ba );
}

void US_HelpDaemon::debug( const QString& message )
{
   QFile d( "/tmp/helpdaemon.log" );
   d.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
   QTextStream out ( &d );
   out << message << endl;
   d.close();
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
  QtSingleApplication application( "UltraScan Help Daemon", argc, argv );
  
  QString message = QString( argv[ 1 ] );

  if ( application.sendMessage( message ) ) return 0;

  application.initialize();
  US_HelpDaemon* daemon = new US_HelpDaemon( message );
 
  QObject::connect( &application, SIGNAL( messageReceived( const QString& ) ),
                    daemon,       SLOT  ( show           ( const QString& ) ) );

  return application.exec();
}

