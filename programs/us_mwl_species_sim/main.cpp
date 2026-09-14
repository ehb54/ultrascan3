//! \file main.cpp
//!
//! The us_mwl_species_sim entry point. Only main() lives here; the
//! US_MwlSpeciesSim implementation is in us_mwl_species_sim.cpp so that a test
//! can link the class without linking a second main().

#include <QApplication>

#include "us_headless_cli.h"
#include "us_mwl_species_sim.h"
#include "us_data_loader.h"
#include "us_select_runs.h"
#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_math2.h"
#include "us_sim_inputs.h"
#include "us_hardware.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_images.h"

#include "qwt_picker_machine.h"
#define dPlotClearAll(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,true)

//! \brief Main program. Loads translators and starts
//         the class US_Convert.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_MwlSpeciesSim w;

   QCommandLineParser parser;
   auto help_option = QCommandLineOption({"help", "h", "?"},
      "Display command-line help");
   parser.addOption(help_option);
   auto version_option = parser.addVersionOption();
   auto models_option = QCommandLineOption("models",
      "Comma-separated list of model file paths, one per wavelength",
      "models");
   parser.addOption(models_option);
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
      "Row within the centerpiece (default: 0), given either as a channel "
      "letter (A-H, where a channel and its reference share a row, so A and B "
      "are row 0) or as a bare row index",
      "channel");
   parser.addOption(centerpiece_channel_option);
   auto start_option = QCommandLineOption("start",
      "Start simulations automatically");
   parser.addOption(start_option);
   auto save_option = QCommandLineOption("save",
      "Save simulation data to a directory",
      "save");
   parser.addOption(save_option);
   auto guid_seed_option = QCommandLineOption("guid-seed",
      "Derive this run's experiment, raw and edit GUIDs from this text "
      "instead of minting random ones, so simulating the same inputs twice "
      "produces the same identities. Use a value unique to the run",
      "text");
   parser.addOption(guid_seed_option);
   auto edit_stamp_option = QCommandLineOption("edit-timestamp",
      "Stamp the edit filenames with this yyMMddhhmm instead of the current "
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

   int cli_exit_code = 0;
   if ( handleStandardCliOptions( parser, help_option, version_option, cli_exit_code ) )
      return cli_exit_code;

   int default_data_location = US_Settings::default_data_location();
   if ( parser.isSet( ignore_db_option ) )
   {
      US_Settings::set_default_data_location( 2 );
   }

   QMap<QString, QString> args;
   if ( parser.isSet( models_option ) && !parser.value( models_option ).isEmpty() )
      args["models"] = parser.value( models_option );
   if ( parser.isSet( buffer_option ) && !parser.value( buffer_option ).isEmpty() )
      args["buffer"] = parser.value( buffer_option );
   if ( parser.isSet( sim_parameters_option ) && !parser.value( sim_parameters_option ).isEmpty() )
      args["simparams"] = parser.value( sim_parameters_option );
   if ( parser.isSet( rotor_option ) && !parser.value( rotor_option ).isEmpty() )
      args["rotor"] = parser.value( rotor_option );
   if ( parser.isSet( centerpiece_option ) )
      args["centerpiece"] = parser.value( centerpiece_option );
   if ( parser.isSet( centerpiece_channel_option ) )
      args["centerpiece-channel"] = parser.value( centerpiece_channel_option );
   if ( parser.isSet( start_option ) )
      args["start"] = "true";
   if ( parser.isSet( errors_option ) )
      args["errors-cl"] = "true";
   if ( parseRunTypeOption( parser, run_type_option, args, cli_exit_code ) )
      return cli_exit_code;
   if ( parser.isSet( save_option ) && !parser.value( save_option ).isEmpty() )
      args["save"] = parser.value( save_option ).replace("\\", "/");
   if ( parser.isSet( guid_seed_option ) && !parser.value( guid_seed_option ).isEmpty() )
      args["guid-seed"] = parser.value( guid_seed_option );
   if ( parser.isSet( edit_stamp_option ) && !parser.value( edit_stamp_option ).isEmpty() )
      args["edit-timestamp"] = parser.value( edit_stamp_option );
   if ( parser.isSet( close_option ) )
      args["close"] = "true";

   int init_status = args.isEmpty() ? 1 : w.init_from_args( args );

   if ( default_data_location != US_Settings::default_data_location() && parser.isSet( ignore_db_option ) )
   {
      US_Settings::set_default_data_location( default_data_location );
   }

   if ( init_status == 1 && args.contains( "errors-cl" ) )
   {
      QTextStream(stderr) << "GUI would be required to complete this run "
         "(some inputs were omitted or could not be loaded); exiting without it "
         "because --errors-cl was set." << Qt::endl;
      return init_status;
   }

   // Show the GUI if no options were supplied or user interaction is needed.
   return showGuiIfNeeded( w, init_status, args );
}
