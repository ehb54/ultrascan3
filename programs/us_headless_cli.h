//! \file us_headless_cli.h
//! \brief Shared boilerplate for QApplication-based headless/CLI drivers
//! (us_astfem_sim, us_mwl_species_sim): standard --help/--version/parse-error
//! handling, the log-or-defer-to-gui pattern used for each --model/--buffer/
//! --simparams/--rotor load, and the final show-GUI-or-return decision.
#ifndef US_HEADLESS_CLI_H
#define US_HEADLESS_CLI_H

#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QString>

//! \brief Handle --help, --version and parser errors the same way both
//! programs do. Returns true (with *exit_code set) if the caller should
//! return immediately; false to continue processing the remaining options.
inline bool handleStandardCliOptions( QCommandLineParser& parser,
                                       const QCommandLineOption& help_option,
                                       const QCommandLineOption& version_option,
                                       int& exit_code )
{
   if ( ! parser.parse( QApplication::arguments() ) )
   {
      QTextStream( stderr ) << qUtf8Printable( parser.errorText() ) << Qt::endl;
      QApplication::exit( 1 );
      exit_code = 1;
      return true;
   }
   if ( parser.isSet( version_option ) )
   {
      QTextStream( stdout ) << QString::asprintf( "%s (%s)\nVersion %s\n\n",
         qUtf8Printable( QApplication::applicationDisplayName() ),
         qUtf8Printable( QApplication::applicationName() ),
         qUtf8Printable( QApplication::applicationVersion() ) ) << Qt::endl;
      QApplication::exit( 0 );
      exit_code = 0;
      return true;
   }
   if ( parser.isSet( help_option ) )
   {
      QTextStream( stdout ) << qUtf8Printable( parser.helpText() ) << Qt::endl;
      QApplication::exit( 0 );
      exit_code = 0;
      return true;
   }
   return false;
}

//! \brief Record a failed headless load: under --errors-cl, log and exit(2)
//! immediately; otherwise mark the run as needing the GUI. Shared by the
//! per-flag load blocks in each program's init_from_args().
inline void reportHeadlessLoadFailure( const QString& what, const QString& id,
                                        bool errors_to_cl,
                                        bool& gui_needed, bool& error_occured )
{
   if ( errors_to_cl )
   {
      qDebug() << "Error loading" << what << id;
      exit( 2 );
   }
   gui_needed    = true;
   error_occured = true;
}

//! \brief Show the GUI (and run the event loop) if the headless run didn't
//! finish and --errors-cl wasn't set to suppress it; otherwise return the
//! init status code as-is.
template <typename Widget>
inline int showGuiIfNeeded( Widget& w, int init_status,
                             const QMap<QString, QString>& args )
{
   if ( init_status != 0 && ! args.contains( "errors-cl" ) )
   {
      w.show();
      return QApplication::exec();
   }
   return init_status;
}

#endif // US_HEADLESS_CLI_H
