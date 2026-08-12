//! \file us_headless_cli.h
//! \brief Shared command-line support for QApplication-based simulation tools.
//! Handles --help, --version, parsing errors, input-loading failures, and the
//! decision to show the GUI or return a status code.
#ifndef US_HEADLESS_CLI_H
#define US_HEADLESS_CLI_H

#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QString>

//! \brief Handle --help, --version, and parsing errors consistently.
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue processing options.
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

//! \brief Handle an input-loading failure during a headless run.
//! With --errors-cl, log the failure and exit with status 2. Otherwise, mark
//! the run as requiring the GUI.
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

//! \brief Show the GUI if the headless run could not finish.
//! If --errors-cl is set, return the initialization status without opening
//! the GUI.
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
