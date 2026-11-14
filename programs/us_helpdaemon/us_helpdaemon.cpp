//! \file us_helpdaemon.cpp
#include "us_helpdaemon.h"
#include "us_settings.h"

#include <cstdlib>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

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

  if ( !QFileInfo( location ).isFile() )
  {
     debug( "help collection not found: " + location );
     fail( tr( "The UltraScan help collection was not found:\n\n%1\n\n"
               "Build the manual (doc/manual) or reinstall UltraScan." )
              .arg( location ) );
  }

  QString       error;
  QStringList   searched;
  const QString assisloc = US_Settings::assistantPath( &error, &searched );

  debug( "assisloc=" + assisloc );

  if ( assisloc.isEmpty() )
  {
     debug( error + "\nsearched:\n  " + searched.join( "\n  " ) );
     fail( tr( "%1\n\nSearched:\n%2" ).arg( error, searched.join( "\n" ) ) );
  }

  connect( &daemon,
           &QProcess::errorOccurred,
           this,
           [this]( QProcess::ProcessError ) { debug( "assistant process error: " + daemon.errorString() ); } );
  connect( &daemon, qOverload< int, QProcess::ExitStatus >( &QProcess::finished ),
                    this, &US_HelpDaemon::close );

  daemon.start( assisloc, args );

  if ( !daemon.waitForStarted() )
  {
     debug( "assistant failed to start: " + daemon.errorString() );
     fail( tr( "Qt Assistant could not be started.\n\n%1\n\n%2" ).arg( assisloc, daemon.errorString() ) );
  }
//show(page);
}

void US_HelpDaemon::close( int exitCode, QProcess::ExitStatus status )
{
   if ( status != QProcess::NormalExit || exitCode != 0 )
   {
      const QString errors = QString::fromLocal8Bit( daemon.readAllStandardError() ).trimmed();
      debug( QString( "assistant exited with code %1: %2" ).arg( exitCode ).arg( errors ) );
      fail( tr( "Qt Assistant exited with an error (code %1).\n\n%2\n\n%3" ).arg( exitCode ).arg( daemon.program(), errors ) );
   }

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

void US_HelpDaemon::fail( const QString& message )
{
   QMessageBox::warning( nullptr, tr( "UltraScan Help" ), message );
   exit( 1 );
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
   out << message << Qt::endl;
   d.close();
}

/*! \brief Main program to start up Qt's Assistant

    Starts Qt Assistant on the requested page and exits when Assistant
    exits.  Each invocation starts its own Assistant.
*/
int main( int argc, char* argv[] )
{
   QString      message = QString( argv[1] );
   //note: for doc files to show properly after an update, it may be necessary
   //      to remove  ~/.local/share/data/Trolltech/Assistant/manual.qch
   QApplication application( argc, argv );
   application.setApplicationDisplayName( "UltraScan Help Daemon" );
   US_HelpDaemon* daemon = new US_HelpDaemon( message );

   return application.exec();
}

