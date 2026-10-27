//! \file us_mwl_species_sim_main.cpp
//! \brief Entry point for us_mwl_species_sim.

#include <QApplication>

#include "us_gui_settings.h"
#include "us_headless_cli.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_mwl_species_sim.h"
#include "us_settings.h"

//! \brief Load translators and start US_MwlSpeciesSim.
int main( int argc, char* argv[] )
{
   QApplication  application( argc, argv );

   // --help, --version and invalid options should work without a license,
   // so the command line is parsed and checked before main1.inc's license
   // dialog, not after it.
   US_SimCliArgs args;
   int           cli_exit_code = 0;
   if ( parseSimCli( US_SimCli::MwlSpecies, args, cli_exit_code ) )
   {
      return cli_exit_code;
   }

#include "main1.inc"

   // License is OK.  Start up.

   // A process-only override, never persisted, so headless runs cannot race
   // on or permanently change the user's setting.
   if ( args.no_db )
   {
      US_Settings::set_default_data_location_override( 2 );
   }
   if ( args.errors_cl )
   {
      application.setProperty( "us3_headless", true );
   }

   // Constructed after --no-db is applied: the constructor reads dbload
   // from the current default data location.
   US_MwlSpeciesSim w;

   chooseNoiseSeedIfUnset( args );
   int init_status = args.given ? w.init_from_args( args ) : 1;

   if ( init_status == 1 && args.errors_cl )
   {
      QTextStream( stderr ) << "GUI would be required to complete this run "
                               "(some inputs were omitted or could not be loaded); exiting without it "
                               "because --errors-cl was set."
                            << Qt::endl;
      return init_status;
   }

   // Show the GUI if no options were supplied or user interaction is needed.
   return showGuiIfNeeded( w, init_status, args );
}
