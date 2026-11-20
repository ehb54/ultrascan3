//! \file us_helpdaemon.cpp
#include "us_helpdaemon.h"
#include "us_settings.h"

#include <cstdlib>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

US_HelpDaemon::US_HelpDaemon( const QString& page, QObject* o ) : QObject( o )
{
   const QString location = US_Settings::appBaseDir() + "/bin/manual.qhc";

   QString       url = "qthelp://ultrascaniii/";
   if ( !page.contains( "manual/" ) )
   {
      url.append( "manual/" );
   }
   url.append( page );

   debug( "page=" + page );
   debug( "url=" + url );
   debug( "location=" + location );

   if ( page.isEmpty() )
   {
      fail( tr( "No help page was requested." ) );
   }

   const QStringList args = { "-collectionFile", location, "-showURL", url };

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

   // Discard stdout and keep only the tail of stderr, so a talkative
   // Assistant can never block on a full pipe.
   daemon.setStandardOutputFile( QProcess::nullDevice() );

   connect( &daemon,
            &QProcess::readyReadStandardError,
            this,
            [this]()
            {
               errors.append( daemon.readAllStandardError() );
               errors = errors.right( 8192 );
            } );
   connect( &daemon,
            &QProcess::errorOccurred,
            this,
            [this]( QProcess::ProcessError ) { debug( "assistant process error: " + daemon.errorString() ); } );
   connect( &daemon, qOverload<int, QProcess::ExitStatus>( &QProcess::finished ), this, &US_HelpDaemon::close );

   daemon.start( assisloc, args );

   if ( !daemon.waitForStarted() )
   {
      debug( "assistant failed to start: " + daemon.errorString() );
      fail( tr( "Qt Assistant could not be started.\n\n%1\n\n%2" ).arg( assisloc, daemon.errorString() ) );
   }
}

void US_HelpDaemon::close( int exitCode, QProcess::ExitStatus status )
{
   if ( status != QProcess::NormalExit || exitCode != 0 )
   {
      errors.append( daemon.readAllStandardError() );
      const QString text = QString::fromLocal8Bit( errors ).trimmed();
      debug( QString( "assistant exited with code %1: %2" ).arg( exitCode ).arg( text ) );
      fail( tr( "Qt Assistant exited with an error (code %1).\n\n%2\n\n%3" ).arg( exitCode ).arg( daemon.program(), text ) );
   }

   exit( 0 );
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
   if ( !d.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      return;
   }
   QTextStream out( &d );
   out << message << Qt::endl;
}

/*! \brief Main program to start up Qt's Assistant

    Starts Qt Assistant on the requested page and exits when Assistant
    exits.  Each invocation starts its own Assistant.
*/
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
   application.setApplicationDisplayName( "UltraScan Help Daemon" );

   const QStringList arguments = application.arguments();
   US_HelpDaemon     daemon( arguments.value( 1 ) );

   return application.exec();
}
