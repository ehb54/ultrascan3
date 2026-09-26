//! \file main.cpp
//! \brief Entry point for us_astfem_sim.

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_defines.h"
#include "us_headless_cli.h"
#include "us_astfem_sim.h"

//! \brief Main program for US_Astfem_Sim.  Loads translators and starts
//! the class US_Astfem_Sim.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
   QApplication::setApplicationName("us_astfem_sim");
   QApplication::setApplicationDisplayName("US Astfem Simulation Module");
   QApplication::setApplicationVersion( US_Version );
   QApplication::setOrganizationDomain("https://ultrascan.aucsoltions.com");
   QApplication::setOrganizationName("AUC Solutions, LLC");
   #include "main1.inc"

   // License is OK. Start up.
   US_Astfem_Sim w;

   QCommandLineParser parser;
   auto help_option = QCommandLineOption({"help", "h", "?"},
      "Display command-line help");
   parser.addOption(help_option);
   auto version_option = parser.addVersionOption();
   auto model_option = QCommandLineOption("model",
      "Load a model from a file path, GUID, or database ID",
      "model");
   parser.addOption(model_option);
   auto buffer_option = QCommandLineOption("buffer",
      "Load a buffer from a file path, GUID, or database ID",
      "buffer");
   parser.addOption(buffer_option);
   auto sim_parameters_option = QCommandLineOption("simparams",
      "Load simulation parameters from a file path",
      "simparams");
   parser.addOption(sim_parameters_option);
   auto rotor_option = QCommandLineOption("rotor",
      "Load a rotor from a file path, GUID, or database ID",
      "rotor");
   parser.addOption(rotor_option);
   auto centerpiece_option = QCommandLineOption("centerpiece",
      "Centerpiece-list index used for channel geometry (default: 0). "
      "Overrides the bottom position recovered from --simparams, which is the "
      "only way to select a row of a multi-row centerpiece. Indexes the local "
      "etc/abstractCenterpieces.xml, never the database, so that a given index "
      "means the same thing on every machine",
      "index");
   parser.addOption(centerpiece_option);
   auto centerpiece_channel_option = QCommandLineOption("centerpiece-channel",
      "Row within the centerpiece, given either as a channel letter (S or A-H; "
      "a channel and its reference share a row, so S, A and B are row 0) "
      "or as a bare row index. Defaults to the row of --channel, else 0; "
      "must match --channel when both are given",
      "channel");
   parser.addOption(centerpiece_channel_option);
   auto movie_option = QCommandLineOption("movie",
      "Show the simulation as a movie");
   parser.addOption(movie_option);
   auto time_correction_option = QCommandLineOption("timecorr",
      "Use time correction");
   parser.addOption(time_correction_option);
   auto start_option = QCommandLineOption("start",
      "Start simulation automatically");
   parser.addOption(start_option);
   auto save_option = QCommandLineOption("save",
      "Directory to write the run into; its last path component becomes the "
      "run ID. Writes several files, not one: the .auc data, the time-state "
      "pair, the edit file, and any noise CSVs. A multi-speed run instead "
      "writes one sibling directory per speed, suffixed with -<rpm>",
      "dir");
   parser.addOption(save_option);
   auto guid_seed_option = QCommandLineOption("guid-seed",
      "Derive this run's experiment, raw and edit GUIDs from this text "
      "instead of minting random ones, so simulating the same inputs twice "
      "produces the same identities. Use a value unique to the run",
      "text");
   parser.addOption(guid_seed_option);
   auto edit_stamp_option = QCommandLineOption("edit-timestamp",
      "Stamp the edit filename with this yyMMddhhmm instead of the current "
      "clock, so the run's filenames are reproducible too",
      "yyMMddhhmm");
   parser.addOption(edit_stamp_option);
   auto close_option = QCommandLineOption("close",
      "Close application if no errors occurred");
   parser.addOption(close_option);
   auto ignore_db_option = QCommandLineOption("no-db",
      "Ignore database preferences and use only locally available data");
   parser.addOption(ignore_db_option);
   auto errors_option = QCommandLineOption("errors-cl",
      "Write errors to the console without opening the GUI");
   parser.addOption(errors_option);
   auto run_type_option = QCommandLineOption("runtype",
      runTypeOptionHelp(), "runtype");
   parser.addOption(run_type_option);
   auto description_option = QCommandLineOption("description",
      descriptionOptionHelp(), "text");
   parser.addOption(description_option);
   auto cell_option = QCommandLineOption("cell", cellOptionHelp(), "cell");
   parser.addOption(cell_option);
   auto channel_option = QCommandLineOption("channel", channelOptionHelp(),
      "channel");
   parser.addOption(channel_option);
   auto noise_seed_option = QCommandLineOption("noise-seed",
      noiseSeedOptionHelp(), "seed");
   parser.addOption(noise_seed_option);

   QMap<QString, QString> args;
   int cli_exit_code = 0;
   if ( handleStandardCliOptions( parser, help_option, version_option, cli_exit_code ) )
   {
      return cli_exit_code;
   }

   // parse command-specific options

   // parse the database setting
   int default_data_location = US_Settings::default_data_location();
   if ( parser.isSet( ignore_db_option ) )
   {
      US_Settings::set_default_data_location( 2 );
   }
   // parse model
   if ( parser.isSet( model_option ) && !parser.value( model_option ).isEmpty() )
   {
      args["model"] = parser.value( model_option );
   }
   // parse buffer
   if ( parser.isSet( buffer_option ) && !parser.value( buffer_option ).isEmpty() )
   {
      args["buffer"] = parser.value( buffer_option );
   }
   // parse simulation parameters
   if ( parser.isSet( sim_parameters_option ) && !parser.value( sim_parameters_option ).isEmpty() )
   {
      args["simparams"] = parser.value( sim_parameters_option );
   }
   // parse rotor
   if ( parser.isSet( rotor_option ) && !parser.value( rotor_option ).isEmpty() )
   {
      args["rotor"] = parser.value( rotor_option );
   }
   // parse centerpiece and channel indices
   if ( parser.isSet( centerpiece_option ) )
   {
      args["centerpiece"] = parser.value( centerpiece_option );
   }
   if ( parser.isSet( centerpiece_channel_option ) )
   {
      args["centerpiece-channel"] = parser.value( centerpiece_channel_option );
   }
   // parse movie
   if ( parser.isSet( movie_option ) )
   {
      args["movie"] = "true";
   }
   // parse time correction
   if ( parser.isSet( time_correction_option ) )
   {
      args["timecorr"] = "true";
   }
   // parse start
   if ( parser.isSet( start_option ) )
   {
      args["start"] = "true";
   }
   // parse errors
   if ( parser.isSet( errors_option ) )
   {
      args["errors-cl"] = "true";
   }
   // parse run type
   if ( parseRunTypeOption( parser, run_type_option, args, cli_exit_code ) )
   {
      return cli_exit_code;
   }
   // parse description
   if ( parseDescriptionOption( parser, description_option, args,
                                cli_exit_code ) )
   {
      return cli_exit_code;
   }
   // parse the cell/channel triple
   if ( parseTripleOptions( parser, cell_option, channel_option, args,
                            cli_exit_code ) )
   {
      return cli_exit_code;
   }
   // parse the noise seed
   if ( parseNoiseSeedOption( parser, noise_seed_option, args, cli_exit_code ) )
   {
      return cli_exit_code;
   }
   // parse save
   if ( parser.isSet( save_option ) && !parser.value( save_option ).isEmpty() )
   {
      args["save"] = parser.value( save_option ).replace("\\", "/");
   }
   // parse guid-seed
   if ( parser.isSet( guid_seed_option ) && !parser.value( guid_seed_option ).isEmpty() )
   {
      args["guid-seed"] = parser.value( guid_seed_option );
   }
   // parse edit-timestamp
   if ( parser.isSet( edit_stamp_option ) && !parser.value( edit_stamp_option ).isEmpty() )
   {
      args["edit-timestamp"] = parser.value( edit_stamp_option );
   }
   // parse close
   if ( parser.isSet( close_option ) )
   {
      args["close"] = "true";
   }
   int init_status = w.init_from_args(args);
   if ( default_data_location != US_Settings::default_data_location() && parser.isSet( ignore_db_option ) )
   {
      // revert the previously changed default data location
      US_Settings::set_default_data_location( default_data_location );
   }
   // Show the GUI only if needed.
   return showGuiIfNeeded( w, init_status, args );
}
