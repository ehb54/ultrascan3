#include "us_help.h"
#include "us_settings.h"

// Null constructor
US_Help::US_Help( QObject* parent ) : QObject( parent ) { }

void US_Help::show_help( const QString& helpFile )
{
  // Create a new process each time.  If show_help() is called multiple
  // times from a process, the previous process will become unavailable,
  // but we don't use it.  It's a 'memory leak', but should be minimal.
  // This is done because a QProcess can't be started multiple times
  // and we would still need a way to get the helpFile to assistant.
  // QProcess is a link to the daemon and is each program that calls
  // help needs its own Qprocess instance.

  // If we wanted to clean up, we would need to do something like the
  // following (not tested).  This seems like a lot of work for
  // little gain.
  
  // header:
  // QList< QProcess*> processes;
  
  // here:
  // assistant = new QProcess;
  // processes << assistant;
  // connect( assistant, SIGNAL(  finished  ( int, QProcess::ExitStatus ) )
  //        ( this     , SLOT  (  endProcess( ( int, QProcess::ExitStatus ) ) );
  
  // Function ( note that we don't really need status
  // void US_Help::end_process( int, QProcess::ExitStatus ) 
  // {
  //    for ( int i = processes.size() - 1; i >= 0; i-- )
  //    {
  //       if ( processes[ i ]->state() == QProcess::NotRunning )
  //       {
  //          delete processes[ i ];
  //          processes.takeAt( i );
  //       }
  //    }
  // }

  assistant = new QProcess;
  QStringList args;
  args << helpFile;

  // This us_helpdaemon will check if an instance is already running and
  // exit immediately if it is.

  assistant->start( "us_helpdaemon", args );
  // Don't bother to wait
}

