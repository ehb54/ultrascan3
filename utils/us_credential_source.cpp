//! \file us_credential_source.cpp
#include "us_credential_source.h"

#include <QTextStream>

#ifdef Q_OS_WIN
#include <io.h>
#include <windows.h>
#else
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

// Turns terminal echo off for its lifetime and restores the saved mode.
class EchoOff
{
   public:
      explicit EchoOff( int fd )
      {
#ifdef Q_OS_WIN
         handle = reinterpret_cast<HANDLE>( _get_osfhandle( fd ) );
         saved  = GetConsoleMode( handle, &mode ) != 0;
         if ( saved )
         {
            SetConsoleMode( handle, mode & ~ENABLE_ECHO_INPUT );
         }
#else
         this->fd = fd;
         saved    = tcgetattr( fd, &mode ) == 0;
         if ( saved )
         {
            termios quiet = mode;
            quiet.c_lflag &= ~static_cast<tcflag_t>( ECHO );
            tcsetattr( fd, TCSAFLUSH, &quiet );
         }
#endif
      }

      ~EchoOff()
      {
         if ( !saved )
         {
            return;
         }
#ifdef Q_OS_WIN
         SetConsoleMode( handle, mode );
#else
         tcsetattr( fd, TCSAFLUSH, &mode );
#endif
      }

      EchoOff( const EchoOff& )            = delete;
      EchoOff& operator=( const EchoOff& ) = delete;

   private:
      bool saved = false;
#ifdef Q_OS_WIN
      HANDLE handle = nullptr;
      DWORD  mode   = 0;
#else
      int     fd = 0;
      termios mode{};
#endif
};

// Reads one line from fd, without its line ending.
// Returns false at end of input with nothing read.
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
   err << prompt << ": " << Qt::flush;

   QByteArray line;
   bool       read_ok = false;
   {
      EchoOff quiet( input_fd );
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
