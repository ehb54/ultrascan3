//! \file us_credential_source.cpp
#include "us_credential_source.h"

#include <QTextStream>

#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#else
#include <csignal>
#include <termios.h>
#include <unistd.h>
#endif

namespace
{
bool isTerminal( int fd )
{
#ifdef Q_OS_WIN
   return _isatty( fd ) != 0;
#else
   return isatty( fd ) != 0;
#endif
}

#ifdef Q_OS_WIN
// The console mode to put back if Ctrl+C or a close ends the prompt.
HANDLE      restore_handle = nullptr;
DWORD       restore_mode   = 0;

BOOL WINAPI restoreConsole( DWORD )
{
   SetConsoleMode( restore_handle, restore_mode );
   return FALSE; // let the default handler end the process
}
#else
// The terminal mode to put back if a signal ends the prompt; one prompt runs
// at a time. The handler restores it, puts back the action that was there
// before the prompt, and raises the signal again so that action runs.
volatile sig_atomic_t restore_pending = 0;
int                   restore_fd      = -1;
termios               restore_mode{};
const int             caught_signals[] = { SIGINT, SIGTERM, SIGQUIT, SIGHUP };
struct sigaction      previous_actions[sizeof caught_signals / sizeof caught_signals[0]];

extern "C" void       restoreTerminalAndRaise( int sig )
{
   if ( restore_pending )
   {
      tcsetattr( restore_fd, TCSANOW, &restore_mode );
      restore_pending = 0;
   }
   for ( size_t ii = 0; ii < sizeof caught_signals / sizeof caught_signals[0]; ii++ )
   {
      if ( caught_signals[ii] == sig )
      {
         sigaction( sig, &previous_actions[ii], nullptr );
      }
   }
   raise( sig );
}
#endif

// Turns terminal echo off for its lifetime and restores the saved mode,
// including when a signal (POSIX) or a console event (Windows) ends the
// process meanwhile. active() is false when echo could not be turned off.
class EchoOff
{
   public:
      explicit EchoOff( int fd )
      {
#ifdef Q_OS_WIN
         handle = reinterpret_cast<HANDLE>( _get_osfhandle( fd ) );
         if ( handle == INVALID_HANDLE_VALUE || !GetConsoleMode( handle, &mode ) )
         {
            return;
         }
         restore_handle = handle;
         restore_mode   = mode;
         SetConsoleCtrlHandler( restoreConsole, TRUE );
         DWORD quiet = 0;
         if ( !SetConsoleMode( handle, mode & ~ENABLE_ECHO_INPUT ) || !GetConsoleMode( handle, &quiet ) ||
              ( quiet & ENABLE_ECHO_INPUT ) != 0 )
         {
            SetConsoleMode( handle, mode );
            SetConsoleCtrlHandler( restoreConsole, FALSE );
            return;
         }
         active_ = true;
#else
         this->fd = fd;
         if ( tcgetattr( fd, &mode ) != 0 )
         {
            return;
         }

         restore_fd   = fd;
         restore_mode = mode;
         struct sigaction action
         {
         };
         action.sa_handler = restoreTerminalAndRaise;
         sigemptyset( &action.sa_mask );
         for ( size_t ii = 0; ii < sizeof caught_signals / sizeof caught_signals[0]; ii++ )
         {
            sigaction( caught_signals[ii], &action, &previous_actions[ii] );
         }

         // Echo goes off before the prompt is shown; typed-ahead input is
         // discarded, as getpass() does.
         termios quiet = mode;
         quiet.c_lflag &= ~static_cast<tcflag_t>( ECHO );
         restore_pending = 1;
         termios now{};
         if ( tcsetattr( fd, TCSAFLUSH, &quiet ) != 0 || tcgetattr( fd, &now ) != 0 || ( now.c_lflag & ECHO ) != 0 )
         {
            restore();
            return;
         }
         active_ = true;
#endif
      }

      ~EchoOff()
      {
         if ( active_ )
         {
            restore();
         }
      }

      bool active() const
      {
         return active_;
      }

      EchoOff( const EchoOff& )            = delete;
      EchoOff& operator=( const EchoOff& ) = delete;

   private:
      void restore()
      {
#ifdef Q_OS_WIN
         SetConsoleMode( handle, mode );
         SetConsoleCtrlHandler( restoreConsole, FALSE );
#else
         restore_pending = 0;
         tcsetattr( fd, TCSANOW, &mode );
         for ( size_t ii = 0; ii < sizeof caught_signals / sizeof caught_signals[0]; ii++ )
         {
            sigaction( caught_signals[ii], &previous_actions[ii], nullptr );
         }
#endif
      }

      bool active_ = false;
#ifdef Q_OS_WIN
      HANDLE handle = nullptr;
      DWORD  mode   = 0;
#else
      int     fd = 0;
      termios mode{};
#endif
};

// Reads one line from fd, without its line ending.
// Returns false at end of input, or when a signal interrupts the read,
// with nothing read.
bool readLine( int fd, QByteArray& line )
{
   line.clear();
   bool any = false;
   char ch  = 0;
   while ( true )
   {
#ifdef Q_OS_WIN
      const int got = _read( fd, &ch, 1 );
#else
      const ssize_t got = read( fd, &ch, 1 );
#endif
      if ( got <= 0 )
      {
         return any;
      }
      any = true;
      if ( ch == '\n' )
      {
         break;
      }
      line.append( ch );
   }
   if ( line.endsWith( '\r' ) )
   {
      line.chop( 1 );
   }
   return true;
}
} // namespace

US_EnvironmentCredentialSource::US_EnvironmentCredentialSource( const QByteArray& variable ) : variable( variable )
{
}

bool US_EnvironmentCredentialSource::available() const
{
   return !qEnvironmentVariableIsEmpty( variable.constData() );
}

US_CredentialSource::Outcome US_EnvironmentCredentialSource::request( const QString&, int, QString& secret )
{
   if ( !available() )
   {
      return Outcome::Unavailable;
   }
   secret = qEnvironmentVariable( variable.constData() );
   return Outcome::Provided;
}

int US_EnvironmentCredentialSource::maxAttempts() const
{
   return 1;
}

US_TerminalCredentialSource::US_TerminalCredentialSource( int input_fd ) : input_fd( input_fd )
{
}

bool US_TerminalCredentialSource::available() const
{
   return isTerminal( input_fd );
}

US_CredentialSource::Outcome US_TerminalCredentialSource::request( const QString& prompt, int, QString& secret )
{
   if ( !available() )
   {
      return Outcome::Unavailable;
   }

   QTextStream err( stderr );
   QByteArray  line;
   bool        read_ok = false;
   {
      EchoOff quiet( input_fd );
      if ( !quiet.active() )
      {
         // Never read a password the terminal would show.
         err << "Cannot turn off echo on the terminal, so the password is not asked for." << Qt::endl;
         return Outcome::Unavailable;
      }
      err << prompt << ": " << Qt::flush;
      read_ok = readLine( input_fd, line );
   }
   // The typed newline was not echoed.
   err << Qt::endl;

   if ( !read_ok )
   {
      return Outcome::Cancelled;
   }
   secret = QString::fromLocal8Bit( line );
   return Outcome::Provided;
}

void US_TerminalCredentialSource::rejected()
{
   QTextStream( stderr ) << "The password is incorrect." << Qt::endl;
}

int US_TerminalCredentialSource::maxAttempts() const
{
   return 3;
}
