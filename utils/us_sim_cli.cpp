//! \file us_sim_cli.cpp
#include "us_sim_cli.h"
#include "us_hardware.h"
#include "us_util.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>

#include <climits>
#include <functional>

namespace
{
// Store an option's value; return false if the value is invalid.
using Apply = std::function<bool( const QString& value, US_SimCliArgs& args )>;

struct Option
{
      int         tools; // US_SimCli::Tool values accepting the option
      QStringList names;
      QString     help;
      QString     value_name; // Empty for a flag
      QString     expected;   // What a valid value is, for the error message
      Apply       apply;
};

// Accepted by every simulator.
const int All = US_SimCli::Astfem | US_SimCli::MwlSpecies;

// A flag sets one field.
Apply     flag( bool US_SimCliArgs::*field )
{
   return [field]( const QString&, US_SimCliArgs& args )
   {
      args.*field = true;
      return true;
   };
}

// An identifier or path; an empty value leaves it unset.
Apply text( QString US_SimCliArgs::*field )
{
   return [field]( const QString& value, US_SimCliArgs& args )
   {
      args.*field = value;
      return true;
   };
}

// A centerpiece index or row; negative values are never valid.
Apply centerpieceValue( int US_SimCliArgs::*field, bool row )
{
   return [field, row]( const QString& value, US_SimCliArgs& args )
   {
      int        parsed = -1;
      QString    unused;
      const bool ok = row ? US_AbstractCenterpiece::parse_channel( value, parsed, unused )
                          : US_AbstractCenterpiece::parse_index( value, parsed, unused );
      if ( !ok || parsed < 0 )
      {
         return false;
      }
      args.*field = parsed;
      return true;
   };
}

const QString          centerpieceHelp = "Centerpiece-list index used for channel geometry (default: 0). "
                                         "Overrides the bottom position recovered from --simparams, which is the "
                                         "only way to select a row of a multi-row centerpiece. Indexes the local "
                                         "etc/abstractCenterpieces.xml, never the database, so that a given index "
                                         "means the same thing on every machine";

const QString          loadHelp = "Load a %1 from a file path, GUID, or database ID";

const QString          centerpieceRowExpected = "a channel letter (S or A-H) or a row index of 0 or more";

// The options, in --help order. Flags and paths have no expected text: any
// value is valid.
const QVector<Option>& options()
{
   static const QVector<Option> table = {
      { US_SimCli::Astfem, { "model" }, loadHelp.arg( "model" ), "model", "", text( &US_SimCliArgs::model ) },
      { US_SimCli::MwlSpecies,
        { "models" },
        "Comma-separated list of model file paths, one per wavelength",
        "models",
        "",
        []( const QString& value, US_SimCliArgs& args )
        {
           args.models = value.split( ",", Qt::SkipEmptyParts );
           return true;
        } },
      { All, { "buffer" }, loadHelp.arg( "buffer" ), "buffer", "", text( &US_SimCliArgs::buffer ) },
      { All, { "simparams" }, "Load simulation parameters from a file path", "simparams", "", text( &US_SimCliArgs::simparams ) },
      { All, { "rotor" }, loadHelp.arg( "rotor" ), "rotor", "", text( &US_SimCliArgs::rotor ) },
      { All,
        { "centerpiece" },
        centerpieceHelp,
        "index",
        "a centerpiece-list index of 0 or more",
        centerpieceValue( &US_SimCliArgs::centerpiece, false ) },
      { US_SimCli::Astfem,
        { "centerpiece-channel" },
        "Row within the centerpiece, given either as a channel letter (S or A-H; "
        "a channel and its reference share a row, so S, A and B are row 0) "
        "or as a bare row index. Defaults to the row of --channel, else 0; "
        "must match --channel when both are given",
        "channel",
        centerpieceRowExpected,
        centerpieceValue( &US_SimCliArgs::centerpiece_row, true ) },
      { US_SimCli::MwlSpecies,
        { "centerpiece-channel" },
        "Row within the centerpiece (default: 0), given either as a channel "
        "letter (S or A-H; a channel and its reference share a row, so S, A and "
        "B are row 0) or as a bare row index",
        "channel",
        centerpieceRowExpected,
        centerpieceValue( &US_SimCliArgs::centerpiece_row, true ) },
      { US_SimCli::Astfem, { "movie" }, "Show the simulation as a movie", "", "", flag( &US_SimCliArgs::movie ) },
      { US_SimCli::Astfem, { "timecorr" }, "Use time correction", "", "", flag( &US_SimCliArgs::timecorr ) },
      { All, { "start" }, "Start the simulation automatically", "", "", flag( &US_SimCliArgs::start ) },
      { US_SimCli::Astfem,
        { "save" },
        "Directory to write the run into; its last path component becomes the "
        "run ID. Writes several files, not one: the .auc data, the time-state "
        "pair, the edit file, and any noise CSVs. A multi-speed run instead "
        "writes one sibling directory per speed, suffixed with -<rpm>. Implies --start",
        "dir",
        "",
        text( &US_SimCliArgs::save ) },
      { US_SimCli::MwlSpecies,
        { "save" },
        "Save simulation data to a directory. Implies --start",
        "save",
        "",
        text( &US_SimCliArgs::save ) },
      { All,
        { "guid-seed" },
        "Derive this run's experiment, raw and edit GUIDs from this text "
        "instead of minting random ones, so simulating the same inputs twice "
        "produces the same identities. Use a value unique to the run",
        "text",
        "",
        text( &US_SimCliArgs::guid_seed ) },
      { All,
        { "edit-timestamp" },
        "Stamp the edit filenames with this yyMMddhhmm instead of the current "
        "clock, so the run's filenames are reproducible too",
        "yyMMddhhmm",
        "a real date and time as yyMMddhhmm",
        []( const QString& value, US_SimCliArgs& args )
        {
           // The value names edit files, so it must be a real time.
           static const QRegularExpression digits( "^[0-9]{10}$" );
           if ( !digits.match( value ).hasMatch() || !QDateTime::fromString( value, "yyMMddhhmm" ).isValid() )
           {
              return false;
           }
           args.edit_stamp = value;
           return true;
        } },
      { All, { "close" }, "Close application if no errors occurred", "", "", flag( &US_SimCliArgs::close ) },
      { All,
        { "no-db" },
        "Ignore database preferences and use only locally available data",
        "",
        "",
        flag( &US_SimCliArgs::no_db ) },
      { All, { "errors-cl" }, "Write errors to the console without opening the GUI", "", "", flag( &US_SimCliArgs::errors_cl ) },
      { All,
        { "runtype" },
        "Two-character optical data type to tag the output with (" + US_SimCli::runTypes().join( ", " ) +
           "; default RA). Affects the type tag and the derived file names only -- "
           "the simulated data is identical for every type",
        "runtype",
        "one of " + US_SimCli::runTypes().join( ", " ),
        []( const QString& value, US_SimCliArgs& args )
        {
           const QString run_type = value.toUpper();
           if ( !US_SimCli::runTypes().contains( run_type ) )
           {
              return false;
           }
           args.run_type = run_type;
           return true;
        } },
      { All,
        { "description" },
        "Description stored with the simulated data (default "
        "\"Simulation\"; up to 239 Latin-1 characters)",
        "text",
        "1 to 239 Latin-1 characters",
        []( const QString& value, US_SimCliArgs& args )
        {
           // The AUC format stores at most 239 Latin-1 characters.
           if ( value.isEmpty() || value.length() > 239 || QString::fromLatin1( value.toLatin1() ) != value )
           {
              return false;
           }
           args.description = value;
           return true;
        } },
      { US_SimCli::Astfem,
        { "cell" },
        "Cell (rotor hole) to tag the output with (1 to 8; default 1). Part "
        "of the cell/channel/wavelength triple that identifies a dataset "
        "within a run",
        "cell",
        "1 to 8",
        []( const QString& value, US_SimCliArgs& args )
        {
           // Eight is the largest supported rotor hole count.
           bool      ok   = false;
           const int cell = value.toInt( &ok );
           if ( !ok || cell < 1 || cell > 8 )
           {
              return false;
           }
           args.cell = cell;
           return true;
        } },
      { US_SimCli::Astfem,
        { "channel" },
        "Channel letter to tag the output with (" + US_Util::channel_letters() +
           "; default S). S is a single-channel centerpiece. A-H are the "
           "channels of a multi-channel centerpiece, in sample/reference pairs "
           "A/B, C/D, E/F and G/H",
        "channel",
        "one of " + US_Util::channel_letters(),
        []( const QString& value, US_SimCliArgs& args )
        {
           const QString channel = value.toUpper();
           if ( channel.length() != 1 || !US_Util::channel_letters().contains( channel ) )
           {
              return false;
           }
           args.channel = channel.at( 0 ).toLatin1();
           return true;
        } },
      { All,
        { "noise-seed" },
        QString( "Seed for the noise generator (1 to %1). Given the same "
                 "seed and the same inputs, a run's noise is reproducible; "
                 "omitted, a started run picks a fresh seed and prints it" )
           .arg( UINT_MAX ),
        "seed",
        QString( "a decimal value from 1 to %1" ).arg( UINT_MAX ),
        []( const QString& value, US_SimCliArgs& args )
        {
           // Zero is invalid because US_Math2::randomize() reads it as "use the clock".
           bool          ok   = false;
           const quint32 seed = value.toUInt( &ok );
           if ( !ok || seed == 0 )
           {
              return false;
           }
           args.noise_seed = seed;
           return true;
        } },
   };
   return table;
}
}

QStringList US_SimCli::runTypes()
{
   return { "RA", "IP", "FI" };
}

QString US_SimCli::invalidValueMessage( const QString& option, const QString& value )
{
   QString expected;
   for ( const Option& entry : options() )
   {
      if ( entry.names.first() == option )
      {
         expected = entry.expected;
         break;
      }
   }
   return QString( "Invalid --%1 %2; expected %3" ).arg( option, value, expected );
}

int US_SimCli::parse( Tool tool, const QStringList& arguments, US_SimCliArgs& args, QString& output )
{
   QCommandLineParser       parser;
   const QCommandLineOption help_option( { "help", "h", "?" }, "Display command-line help" );
   parser.addOption( help_option );
   const QCommandLineOption version_option = parser.addVersionOption();

   QVector<const Option*>   accepted;
   for ( const Option& option : options() )
   {
      if ( option.tools & tool )
      {
         parser.addOption( option.value_name.isEmpty() ? QCommandLineOption( option.names, option.help )
                                                       : QCommandLineOption( option.names, option.help, option.value_name ) );
         accepted << &option;
      }
   }

   if ( !parser.parse( arguments ) )
   {
      output = parser.errorText();
      return 1;
   }
   if ( parser.isSet( version_option ) )
   {
      // QGuiApplication's display name, if the program set one.
      QString name =
         QCoreApplication::instance() ? QCoreApplication::instance()->property( "applicationDisplayName" ).toString() : QString();
      if ( name.isEmpty() )
      {
         name = QCoreApplication::applicationName();
      }
      output = QString( "%1 (%2)\nVersion %3\n" )
                  .arg( name, QCoreApplication::applicationName(), QCoreApplication::applicationVersion() );
      return 0;
   }
   if ( parser.isSet( help_option ) )
   {
      output = parser.helpText();
      return 0;
   }

   args = US_SimCliArgs();
   for ( const Option* option : accepted )
   {
      const QString& name = option->names.first();
      if ( !parser.isSet( name ) )
      {
         continue;
      }

      args.given          = args.given || name != "no-db";
      const QString value = parser.value( name );
      if ( !option->apply( value, args ) )
      {
         output = invalidValueMessage( name, value );
         return 1;
      }
   }

   // Saving needs a simulation.
   args.save.replace( "\\", "/" );
   args.start = args.start || !args.save.isEmpty();
   return -1;
}
