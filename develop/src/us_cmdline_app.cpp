#include "../include/us_cmdline_app.h"

US_Cmdline_App::US_Cmdline_App( 
                               QApplication * qa,
                               QString        dir,
                               QStringList    args,
                               QStringList    app_text,
                               QStringList    response,
                               QString      * error_msg,
                               int            timer_delay_ms
                               // QStringList  * stdout,
                               // QStringList  * stderr
                               )
{
   this->qa             = qa;
   this->dir            = dir;
   this->args           = args;
   this->app_text       = app_text;
   this->response       = response;
   this->error_msg      = error_msg;
   this->timer_delay_ms = timer_delay_ms;
   // this->stdout         = stdout;
   // this->stderr         = stderr;

   *error_msg = "";

   if ( !args.size() ||
        args[ 0 ].isEmpty() )
   {
      *error_msg += QString( "Error: an application name must be specified\n" );
      qa->quit();
      return;
   }

   if ( app_text.size() != response.size() )
   {
      *error_msg += 
         QString( "Error: query/response size mismatch %1 vs %2\n" )
         .arg( app_text.size() )
         .arg( response.size() )
         ;
      qa->quit();
      return;
   }
      
   // process = new QProcess( this );
      
   if ( !dir.isEmpty() )
   {
      process.setWorkingDirectory( dir );
   }
   
   connect( &process, SIGNAL( readyReadStdout() ), this, SLOT( readFromStdout() ) );
   connect( &process, SIGNAL( readyReadStderr() ), this, SLOT( readFromStderr() ) );
   connect( &process, SIGNAL( processExited  () ), this, SLOT( processExited () ) );
   connect( &process, SIGNAL( launchFinished () ), this, SLOT( launchFinished() ) );

   connect( &timer,  SIGNAL( timeout()         ), this, SLOT( timeout()        ) );

   process.setArguments( args );
        
   query_response_pos = 0;
   run_to_end         = false;

   if ( !process.start() )
   {
      // qDebug( "error starting" );
      *error_msg += QString( "Error: could not start process: %1\n" ).arg( args[ 0 ] );
      process.kill();
      qa->quit();
      qa->processEvents();
      return;
   }
   // qDebug( QString( "starting process: %1" ).arg( args[ 0 ] ) );
}

US_Cmdline_App::~US_Cmdline_App()
{
   process.kill();
}

void US_Cmdline_App::timeout()
{
   *error_msg += 
      QString( "Error: out of responses to queries (timeout)\n" );
   // qDebug( "timeout" );
   process.kill();
}

void US_Cmdline_App::readFromStdout()
{
   timer.stop();
   // qDebug( "readFromStdout()" );
   QString qs;
   QString text;
   do {
      qs = process.readLineStdout();
      // if ( stdout )
      // {
      //    *stdout << qs;
      // }
      text += qs + "\n";
   } while ( qs != QString::null );

   do {
      QString read = process.readStdout();
      // if ( stdout )
      // {
      //    *stdout << qs;
      // }
      qs = QString( "%1" ).arg( read );
      text += qs;
   } while ( qs.length() );
   
   // qDebug( QString( "received <%1>" ).arg( text ) );

   if ( !run_to_end && app_text.size() )
   {
      // if not at first entry, read data to find match
      int previous_pos = query_response_pos;
      while ( ( int ) app_text.size() > query_response_pos &&
              !text.contains( app_text[ query_response_pos ] ) && 
              query_response_pos )
      {
         query_response_pos++;
      }
      if ( query_response_pos >= ( int ) app_text.size() )
      {
         query_response_pos = previous_pos;
         if ( timer_delay_ms )
         {
            // qDebug( QString( "starting timer for %1 seconds" ).arg( ( double )timer_delay_ms / 1000e0 ) );
            timer.start( timer_delay_ms );
         } else {
            *error_msg += 
               QString( "Error: out of responses to queries\n" )
               ;
            process.kill();
            qa->quit();
         }
         return;
      }         

      // do we have a match?
      if ( ( int ) app_text.size() > query_response_pos &&
           text.contains( app_text[ query_response_pos ] ) )
      {
         // qDebug( QString( "received <%1> from application" ).arg( app_text[ query_response_pos ] ) );
         if ( response[ query_response_pos ] != "___run___" )
         {
            if ( response[ query_response_pos ].left( 2 ).contains( "__" ) )
            {
               *error_msg += 
                  QString( "Error: undefined response <%1> to query <%2>\n" )
                  .arg( response[ query_response_pos ] )
                  .arg( app_text[ query_response_pos ] )
                  ;
               process.kill();
               qa->quit();
               return;
            }
            // qDebug( QString( "sent     <%1> to application"   ).arg( response[ query_response_pos ] ) );
            process.writeToStdin( response[ query_response_pos ] + "\n" );
            query_response_pos++;
         } else {
            // qDebug( "now run to end of application" );
            run_to_end = true;
         }
      }
   }
}

void US_Cmdline_App::readFromStderr()
{
   // qDebug( "readFromStderr()" );
   // if ( stderr )
   // {
   //    while ( process.canReadLineStderr() )
   //    {
   //       *stderr << process.readLineStderr();
   //    }
   // }
}

void US_Cmdline_App::processExited()
{
   // qDebug( "processExited()" );

   if ( !process.normalExit() )
   {
      *error_msg += "Error: process did not exit normally\n";
   }

   if ( process.exitStatus() )
   {
      *error_msg += QString( "Error: exit status non-zero: %1\n" ).arg( process.exitStatus() );
   }

   qa->quit();
}

void US_Cmdline_App::launchFinished()
{
   // qDebug( "launchFinished()" );
}
