//! \file us_headless_cli.h
//! \brief Shared command-line support for QApplication-based simulation tools.
//! Parses the options US_SimCli declares, handles input-loading failures, and
//! decides whether to show the GUI or return a status code.
#ifndef US_HEADLESS_CLI_H
#define US_HEADLESS_CLI_H

#include <QApplication>
#include <QDebug>
#include <QRandomGenerator>
#include <QTextStream>

#include "us_credential_source.h"
#include "us_db_provider.h"
#include "us_notify.h"
#include "us_session.h"
#include "us_settings.h"
#include "us_sim_cli.h"
#include "us_util.h"
#include "us_widgets.h"

#include <memory>

//! \brief Use Qt's offscreen platform when no window will be shown, so
//! headless runs need no display.
//! --help and --version only print, and --errors-cl never shows the GUI.
//! Call before constructing QApplication. A QT_QPA_PLATFORM the user set is
//! kept, and the native platform is the fallback for a package that ships
//! without the offscreen plugin.
inline void useOffscreenPlatformIfHeadless( int argc, char* argv[] )
{
   if ( qEnvironmentVariableIsSet( "QT_QPA_PLATFORM" ) )
   {
      return;
   }

#if defined( Q_OS_MACOS )
   const char* native = "cocoa";
#elif defined( Q_OS_WIN )
   const char* native = "windows";
#else
   const char* native = "xcb;wayland";
#endif

   const QStringList headless = { "--errors-cl", "--help", "-h", "-?", "--version", "-v" };
   for ( int ii = 1; ii < argc; ii++ )
   {
      if ( headless.contains( QString::fromLocal8Bit( argv[ii] ) ) )
      {
         qputenv( "QT_QPA_PLATFORM", QByteArray( "offscreen;" ) + native );
         return;
      }
   }
}

//! \brief Parse the simulator's command line, printing help, version or errors.
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue with args.
inline bool parseSimCli( US_SimCli::Tool tool, US_SimCliArgs& args, int& exit_code )
{
   QString output;
   exit_code = US_SimCli::parse( tool, QApplication::arguments(), args, output );
   if ( exit_code < 0 )
   {
      return false;
   }
   QTextStream( exit_code == 0 ? stdout : stderr ) << output << Qt::endl;
   return true;
}

//! \brief Apply the process-wide choices the command line makes.
//! --guid-seed makes every GUID the run creates reproducible, not only the
//! keyed ones. A headless (--errors-cl) run needs no dialogs: messages go to
//! the console, and the master password comes from US_MASTER_PASSWORD or,
//! when that is unset, a no-echo prompt on the terminal, asked for only when
//! a load first needs the database. Never from the command line: it would
//! show in process listings and shell history. A headless run uses an
//! unlock the desktop has cached but never writes its own into that cache.
//! Call once at startup, before any other thread runs.
inline void useHeadlessServices( const US_SimCliArgs& args )
{
   if ( !args.guid_seed.isEmpty() )
   {
      US_Util::setGuidSource( QSharedPointer<US_GuidSource>( new US_SeededGuidSource( args.guid_seed ) ) );
   }
   if ( !args.errors_cl )
   {
      return;
   }
   US_Notify::setNotifier( QSharedPointer<US_Notifier>( new US_ConsoleNotifier ) );
   if ( args.no_db )
   {
      return;
   }
   US_Session& session = US_Session::instance();
   session.keepUnlockInProcess();
   session.setPromptSources( { QSharedPointer<US_CredentialSource>( new US_EnvironmentCredentialSource( "US_MASTER_PASSWORD" ) ),
                               QSharedPointer<US_CredentialSource>( new US_TerminalCredentialSource ) } );
}

//! \brief Give a started run without --noise-seed a fresh seed, and print it.
//! Unseeded, the generator starts from the same fixed state in every process,
//! so replicate runs would all get the same noise.
inline void chooseNoiseSeedIfUnset( US_SimCliArgs& args )
{
   if ( !args.start || args.noise_seed != 0 )
   {
      return;
   }

   // Zero is reserved: US_Math2::randomize() reads it as "use the clock".
   quint32 seed = 0;
   while ( seed == 0 )
   {
      seed = QRandomGenerator::system()->generate();
   }

   args.noise_seed = seed;
   QTextStream( stdout ) << "Noise seed: " << seed << " (pass --noise-seed " << seed << " to reproduce this run's noise)"
                         << Qt::endl;
}

//! \brief Handle an input-loading failure during a headless run.
//! With --errors-cl, log the failure and exit with status 2. Otherwise, mark
//! the run as requiring the GUI.
inline void reportHeadlessLoadFailure( const QString& what,
                                       const QString& id,
                                       bool           errors_to_cl,
                                       bool&          gui_needed,
                                       bool&          error_occured )
{
   if ( errors_to_cl )
   {
      qDebug() << "Error loading" << what << id;
      exit( 2 );
   }
   gui_needed    = true;
   error_occured = true;
}

//! \brief The connection the command line's model, buffer and rotor are read
//! from: the database when that is the data location, else none (the local
//! data folders). A failed connection is reported and returned, so each load
//! then fails with its reason.
inline std::unique_ptr<IUS_DB2> openSimInputDatabase( const US_SimCliArgs& args )
{
   if ( args.model.isEmpty() && args.buffer.isEmpty() && args.rotor.isEmpty() )
   {
      return nullptr;
   }
   if ( US_Settings::default_data_location() != US_Disk_DB_Controls::DB )
   {
      return nullptr;
   }
   std::unique_ptr<IUS_DB2> db = US_DBProvider::open();
   if ( db->lastErrno() != IUS_DB2::OK )
   {
      US_Notify::warning(
         nullptr, QObject::tr( "Connection Problem" ), QObject::tr( "Could not connect to database \n" ) + db->lastError() );
   }
   return db;
}

//! \brief Report why a command-line input could not be loaded, then handle
//! the failure as reportHeadlessLoadFailure() does.
inline void reportSimInputFailure( const QString& what,
                                   const QString& id,
                                   const QString& reason,
                                   bool           errors_to_cl,
                                   bool&          gui_needed,
                                   bool&          error_occured )
{
   US_Notify::warning( nullptr, QObject::tr( "Could not load the %1" ).arg( what ), reason );
   reportHeadlessLoadFailure( what, id, errors_to_cl, gui_needed, error_occured );
}

//! \brief Show the GUI if the headless run could not finish.
//! If --errors-cl is set, return the initialization status without opening
//! the GUI.
template <typename Widget> inline int showGuiIfNeeded( Widget& w, int init_status, const US_SimCliArgs& args )
{
   if ( init_status != 0 && !args.errors_cl )
   {
      w.show();
      return QApplication::exec();
   }
   return init_status;
}

#endif // US_HEADLESS_CLI_H
