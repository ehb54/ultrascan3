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
#include <QStringList>

#include <climits>

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

//! \brief Optical data types the headless simulators may tag their output with.
//! These are the five of US_DataIO's six that are inert with respect to the
//! simulators: nothing downstream of the type tag branches on them.
//!
//! "WA" is deliberately excluded. It is the one value that changes behaviour
//! here, via the experiment-XML writer's `runType == "WA"` branch, and that
//! branch has never executed -- it emits the model's single scalar wavelength
//! under a "radius" attribute, which is wrong for anything but the multi-speed
//! case it was written against. Accepting "WA" would activate untested code
//! rather than exercise a real capability, and real WA data is organized
//! differently anyway (its triple is cell/channel/radius, with the scan
//! sweeping wavelength), so one simulation could not produce it regardless.
//! Rework that branch before adding "WA" here.
inline QStringList supportedRunTypes()
{
   return { "RA", "IP", "RI", "FI", "WI" };
}

//! \brief Parse and validate an optional --runtype option.
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue processing options.
inline bool parseRunTypeOption( QCommandLineParser& parser,
                                 const QCommandLineOption& run_type_option,
                                 QMap<QString, QString>& args,
                                 int& exit_code )
{
   if ( ! parser.isSet( run_type_option ) )
      return false;

   const QString run_type = parser.value( run_type_option ).toUpper();

   if ( ! supportedRunTypes().contains( run_type ) )
   {
      QTextStream( stderr ) << "Invalid --runtype " << run_type
         << "; expected one of " << supportedRunTypes().join( ", " ) << Qt::endl;
      QApplication::exit( 1 );
      exit_code = 1;
      return true;
   }

   args["runtype"] = run_type;
   return false;
}

//! \brief Help text for the --runtype option, shared by the simulators.
inline QString runTypeOptionHelp()
{
   return "Two-character optical data type to tag the output with ("
        + supportedRunTypes().join( ", " ) + "; default RA). Affects the type "
          "tag and the derived file names only -- the simulated data is "
          "identical for every type";
}

//! Recognized channel letters in their persisted numbering order.
inline QString channelLetters()
{
   return "SABCDEFGH";
}

//! \brief Parse and validate the optional --cell and --channel options.
//!
//! These values must agree across the AUC data and its experiment/edit records.
//!
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue processing options.
inline bool parseTripleOptions( QCommandLineParser& parser,
                                const QCommandLineOption& cell_option,
                                const QCommandLineOption& channel_option,
                                QMap<QString, QString>& args,
                                int& exit_code )
{
   if ( parser.isSet( cell_option ) )
   {
      bool ok = false;
      const int cell = parser.value( cell_option ).toInt( &ok );

      // Eight is the largest supported rotor hole count.
      if ( ! ok  ||  cell < 1  ||  cell > 8 )
      {
         QTextStream( stderr ) << "Invalid --cell "
            << parser.value( cell_option ) << "; expected 1 to 8" << Qt::endl;
         QApplication::exit( 1 );
         exit_code = 1;
         return true;
      }

      args["cell"] = QString::number( cell );
   }

   if ( parser.isSet( channel_option ) )
   {
      const QString channel = parser.value( channel_option ).toUpper();

      if ( channel.length() != 1  ||  ! channelLetters().contains( channel ) )
      {
         QTextStream( stderr ) << "Invalid --channel " << channel
            << "; expected one of " << channelLetters() << Qt::endl;
         QApplication::exit( 1 );
         exit_code = 1;
         return true;
      }

      args["channel"] = channel;
   }

   return false;
}

//! \brief Help text for the --cell option, shared by the simulators.
inline QString cellOptionHelp()
{
   return "Cell (rotor hole) to tag the output with (1 to 8; default 1). Part "
          "of the cell/channel/wavelength triple that identifies a dataset "
          "within a run";
}

//! \brief Parse and validate the optional --noise-seed option.
//!
//! Seeding the generator makes a run's noise reproducible.  Zero is rejected
//! rather than accepted, because US_Math2::randomize() reads it as "pick a
//! seed from the clock" and would silently produce an unreproducible run.
//!
//! \return true if the caller should return immediately using exit_code;
//! false if it should continue processing options.
inline bool parseNoiseSeedOption( QCommandLineParser& parser,
                                  const QCommandLineOption& noise_seed_option,
                                  QMap<QString, QString>& args,
                                  int& exit_code )
{
   if ( ! parser.isSet( noise_seed_option ) )
      return false;

   bool ok = false;
   const QString  value = parser.value( noise_seed_option );
   const quint32  seed  = value.toUInt( &ok );

   // toUInt() rejects an empty string, a sign, a non-decimal digit and any
   // value past UINT_MAX, so only zero has to be excluded separately.
   if ( ! ok  ||  seed == 0 )
   {
      QTextStream( stderr ) << "Invalid --noise-seed " << value
         << "; expected a decimal value from 1 to " << UINT_MAX << Qt::endl;
      QApplication::exit( 1 );
      exit_code = 1;
      return true;
   }

   args["noise-seed"] = QString::number( seed );

   return false;
}

//! \brief Help text for the --noise-seed option, shared by the simulators.
inline QString noiseSeedOptionHelp()
{
   return "Seed for the noise generator (1 to 4294967295). Given the same "
          "seed and the same inputs, a run's noise is reproducible; omitted, "
          "the generator is left as it is";
}

//! \brief Help text for the --channel option, shared by the simulators.
inline QString channelOptionHelp()
{
   return "Channel letter to tag the output with (" + channelLetters()
        + "; default S). S is the single-channel centerpiece case; A and B "
          "are the two channels of a standard two-channel centerpiece";
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
