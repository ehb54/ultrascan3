//! \file us_credential_source.cpp
#include "us_credential_source.h"

#include <QTextStream>

#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#else
#include <cerrno>
#include <csignal>
#include <cstring>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace
{
#ifdef Q_OS_WIN
bool isTerminal( int fd )
{
   return _isatty( fd ) != 0;
}

// The console mode to put back if Ctrl+C or a close ends the prompt.
HANDLE      restore_handle = nullptr;
DWORD       restore_mode   = 0;

BOOL WINAPI restoreConsole( DWORD )
{
   SetConsoleMode( restore_handle, restore_mode );
   return FALSE; // let the default handler end the process
}
#else
// Whether this process may change the terminal's mode: it is the foreground
// job of its controlling terminal, or the terminal is not its controlling one
// (no job control applies). A background job would be stopped instead.
bool inForeground( int fd )
{
   const pid_t group = tcgetpgrp( fd );
   return group < 0 || group == getpgrp();
}

bool isTerminal( int fd )
{
   return isatty( fd ) != 0 && inForeground( fd );
}

// One prompt runs at a time; its state is here so the signal handlers can
// put the terminal back. Every handler forwards a signal delivered to another
// thread to the thread that reads, so the reader always sees it.
volatile sig_atomic_t restore_pending = 0; // echo is off; restore_mode goes back
volatile sig_atomic_t interrupted     = 0; // a terminating signal arrived
volatile sig_atomic_t echo_lost       = 0; // echo could not be turned off again
volatile sig_atomic_t resumed         = 0; // the job was continued after a stop
int                   restore_fd      = -1;
termios               restore_mode{};
termios               quiet_mode{};
pthread_t             reader{};
char                  prompt_text[256];
size_t                prompt_size = 0;

const int             ending_signals[] = { SIGINT, SIGTERM, SIGQUIT, SIGHUP };
constexpr size_t      ending_count     = sizeof ending_signals / sizeof ending_signals[0];
struct sigaction      previous_ending[ending_count];
struct sigaction      previous_stop;
struct sigaction      previous_continue;
bool                  handling_stop = false;

bool                  forwarded( int sig )
{
   if ( pthread_equal( pthread_self(), reader ) )
   {
      return false;
   }
   pthread_kill( reader, sig );
   return true;
}

// Puts the terminal back, restores the actions that were there before the
// prompt, and raises the signal again so that action runs.
extern "C" void restoreTerminalAndRaise( int sig )
{
   if ( forwarded( sig ) )
   {
      return;
   }
   interrupted = 1;
   if ( restore_pending )
   {
      tcsetattr( restore_fd, TCSANOW, &restore_mode );
      restore_pending = 0;
   }
   for ( size_t ii = 0; ii < ending_count; ii++ )
   {
      if ( ending_signals[ii] == sig )
      {
         sigaction( sig, &previous_ending[ii], nullptr );
      }
   }
   raise( sig );
}

// Ctrl+Z: give the terminal back with echo on, then stop as usual.
extern "C" void restoreTerminalAndStop( int sig )
{
   if ( forwarded( sig ) )
   {
      return;
   }
   const int saved_errno = errno;
   if ( restore_pending )
   {
      tcsetattr( restore_fd, TCSANOW, &restore_mode );
      restore_pending = 0;
   }
   struct sigaction own
   {
   };
   sigaction( SIGTSTP, &previous_stop, &own );
   sigset_t stop;
   sigemptyset( &stop );
   sigaddset( &stop, SIGTSTP );
   pthread_sigmask( SIG_UNBLOCK, &stop, nullptr );
   raise( SIGTSTP ); // the process stops here until it is continued
   sigaction( SIGTSTP, &own, nullptr );
   errno = saved_errno;
}

// Continued in the foreground: turn echo off again and repeat the prompt.
// Continued in the background, the terminal belongs to the shell; reading
// stops the job again until it is brought to the foreground.
extern "C" void quietTerminalAgain( int sig )
{
   if ( forwarded( sig ) )
   {
      return;
   }
   const int saved_errno = errno;
   resumed               = 1;
   if ( inForeground( restore_fd ) )
   {
      termios now{};
      if ( tcsetattr( restore_fd, TCSANOW, &quiet_mode ) != 0 || tcgetattr( restore_fd, &now ) != 0 || ( now.c_lflag & ECHO ) != 0 )
      {
         echo_lost = 1;
      }
      else
      {
         restore_pending = 1;
         ssize_t ignored = write( STDERR_FILENO, "\n", 1 );
         ignored         = write( STDERR_FILENO, prompt_text, prompt_size );
         static_cast<void>( ignored );
      }
   }
   errno = saved_errno;
}
#endif

// Turns terminal echo off for its lifetime and restores the saved mode,
// including when a signal (POSIX) or a console event (Windows) ends the
// process meanwhile, and around a Ctrl+Z stop (POSIX). Shows the prompt once
// echo is off. active() is false when echo could not be turned off.
class EchoOff
{
   public:
      EchoOff( int fd, const QByteArray& prompt )
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
         fputs( prompt.constData(), stderr );
         fflush( stderr );
#else
         this->fd = fd;
         if ( tcgetattr( fd, &mode ) != 0 )
         {
            return;
         }

         restore_fd   = fd;
         restore_mode = mode;
         quiet_mode   = mode;
         quiet_mode.c_lflag &= ~static_cast<tcflag_t>( ECHO );
         reader      = pthread_self();
         interrupted = echo_lost = resumed = 0;
         prompt_size                       = qMin( static_cast<size_t>( prompt.size() ), sizeof prompt_text );
         memcpy( prompt_text, prompt.constData(), prompt_size );

         // No SA_RESTART: a signal interrupts the read so it can be checked.
         struct sigaction action
         {
         };
         sigemptyset( &action.sa_mask );
         action.sa_handler = restoreTerminalAndRaise;
         for ( size_t ii = 0; ii < ending_count; ii++ )
         {
            sigaction( ending_signals[ii], &action, &previous_ending[ii] );
         }
         // Job control is handled only where the process has its default
         // stop action; an ignored Ctrl+Z stays ignored.
         sigaction( SIGTSTP, nullptr, &previous_stop );
         handling_stop = previous_stop.sa_handler == SIG_DFL;
         if ( handling_stop )
         {
            action.sa_handler = restoreTerminalAndStop;
            sigaction( SIGTSTP, &action, nullptr );
            action.sa_handler = quietTerminalAgain;
            sigaction( SIGCONT, &action, &previous_continue );
         }

         // Echo goes off before the prompt is shown; typed-ahead input is
         // discarded, as getpass() does.
         restore_pending = 1;
         termios now{};
         if ( tcsetattr( fd, TCSAFLUSH, &quiet_mode ) != 0 || tcgetattr( fd, &now ) != 0 || ( now.c_lflag & ECHO ) != 0 )
         {
            restore();
            return;
         }
         active_         = true;
         ssize_t ignored = write( STDERR_FILENO, prompt_text, prompt_size );
         static_cast<void>( ignored );
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
         if ( handling_stop )
         {
            sigaction( SIGTSTP, &previous_stop, nullptr );
            sigaction( SIGCONT, &previous_continue, nullptr );
         }
         if ( restore_pending )
         {
            tcsetattr( fd, TCSANOW, &mode );
         }
         restore_pending = 0;
         for ( size_t ii = 0; ii < ending_count; ii++ )
         {
            sigaction( ending_signals[ii], &previous_ending[ii], nullptr );
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

// Reads one line from fd, without its line ending. Returns false at end of
// input with nothing read, or when the prompt is interrupted, so a partly
// typed line is never used.
bool readSecret( int fd, QString& secret )
{
#ifdef Q_OS_WIN
   // The console gives UTF-16, so no code page is involved.
   const HANDLE handle = reinterpret_cast<HANDLE>( _get_osfhandle( fd ) );
   QString      line;
   bool         any = false;
   while ( true )
   {
      wchar_t ch  = 0;
      DWORD   got = 0;
      if ( !ReadConsoleW( handle, &ch, 1, &got, nullptr ) || got == 0 )
      {
         if ( !any )
         {
            return false;
         }
         break;
      }
      any = true;
      if ( ch == L'\n' )
      {
         break;
      }
      line.append( QChar( static_cast<ushort>( ch ) ) );
   }
   if ( line.endsWith( QLatin1Char( '\r' ) ) )
   {
      line.chop( 1 );
   }
   secret = line;
   return true;
#else
   QByteArray line;
   bool       any = false;
   while ( true )
   {
      char          ch  = 0;
      const ssize_t got = read( fd, &ch, 1 );
      if ( got < 0 )
      {
         if ( errno == EINTR && !interrupted && !echo_lost )
         {
            if ( resumed )
            {
               // Start again after a stop; the prompt was shown again.
               resumed = 0;
               line.clear();
               any = false;
            }
            continue;
         }
         return false;
      }
      if ( got == 0 )
      {
         if ( !any )
         {
            return false;
         }
         break;
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
   secret = QString::fromLocal8Bit( line );
   return true;
#endif
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
   // Programs this process starts do not inherit the password.
   qunsetenv( variable.constData() );
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
   QString     line;
   bool        read_ok = false;
   {
      EchoOff quiet( input_fd, ( prompt + QStringLiteral( ": " ) ).toLocal8Bit() );
      if ( !quiet.active() )
      {
         // Never read a password the terminal would show.
         err << "Cannot turn off echo on the terminal, so the password is not asked for." << Qt::endl;
         return Outcome::Unavailable;
      }
      read_ok = readSecret( input_fd, line );
   }
   // The typed newline was not echoed.
   err << Qt::endl;

   if ( !read_ok )
   {
      return Outcome::Cancelled;
   }
   secret = line;
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
