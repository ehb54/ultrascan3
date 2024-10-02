//! \file us_helpdaemon.cpp
#include "us_helpdaemon.h"
#include "us_settings.h"

#if QT_VERSION > 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtSingleApplication>
#endif

US_HelpDaemon::US_HelpDaemon( const QString& page, QObject* o ) : QObject( o )
{

  // special Q_OS_WIN code apparently does not work anymore 
  // #ifdef Q_OS_WIN
  // QString location = US_Settings::appBaseDir() + "/bin/manual.qch";
  // #else
  QString location = US_Settings::appBaseDir() + "/bin/manual.qhc";
  // #endif

  QString url      = "qthelp://ultrascaniii/";
  if ( !page.contains( "manual/" ) )
     url.append( "manual/" );
  url.append( page );
debug("page="+page);
debug("url="+url);
debug("location="+location);

  QStringList args;

  args << QLatin1String( "-collectionFile" )
       << location
       << QLatin1String( "-enableRemoteControl" )
       << QLatin1String( "-showURL" )
       << url;

  debug( args.join( " " ) );
#ifndef Q_OS_MAC
  QString assisloc  = US_Settings::appBaseDir() + "/bin/assistant";
#else
  QString assisloc  = US_Settings::appBaseDir() + "/bin/Assistant.app";
#endif
  daemon.start( assisloc, args );
//debug("assisloc="+assisloc);
  daemon.waitForStarted();

  connect( &daemon, SIGNAL( finished ( int, QProcess::ExitStatus ) ),
                    SLOT  ( close    ( int, QProcess::ExitStatus ) ) );
//show(page);
}

void US_HelpDaemon::close( int /*exitCode*/, QProcess::ExitStatus /*status*/ ) 
{
  exit( 0 );
}

void US_HelpDaemon::show( const QString& helpPage )
{
debug("IN show()\n");
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
#ifdef Q_OS_WIN
  ba.append( page.toLocal8Bit() );
  ba.append( '\n' );
#else
  ba.append( page.toLatin1() );
  ba.append( '\0' );
#endif

  daemon.write( ba );
}

void US_HelpDaemon::debug( const QString& message )
{
#ifdef Q_OS_WIN
   QFile d( "c:/dist/helpdaemon.log" );
#else
   QFile d( "/tmp/helpdaemon.log" );
#endif
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
   QString message = QString( argv[ 1 ] );
  //  Need to add uid to identifier ????
  //note: for doc files to show properly after an update, it may be necessary 
  //      to remove  ~/.local/share/data/Trolltech/Assistant/manual.qch
#if QT_VERSION > 0x050000
   QApplication application( argc, argv );
   application.setApplicationDisplayName( "UltraScan Help Daemon" );
#else
   QtSingleApplication application( "UltraScan Help Daemon", argc, argv );
  
   if ( application.sendMessage( message ) ) return 0;

   application.initialize();
#endif
  US_HelpDaemon* daemon = new US_HelpDaemon( message );
 
  QObject::connect( &application, SIGNAL( messageReceived( const QString& ) ),
                    daemon,       SLOT  ( show           ( const QString& ) ) );

  return application.exec();
}

