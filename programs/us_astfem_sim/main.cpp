//! \file main.cpp
//! \brief Entry point for us_astfem_sim.

#include <QApplication>

#include "us_astfem_sim.h"
#include "us_defines.h"
#include "us_gui_settings.h"
#include "us_headless_cli.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"

//! \brief Main program for US_Astfem_Sim.  Loads translators and starts
//! the class US_Astfem_Sim.
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
   QApplication::setApplicationName( "us_astfem_sim" );
   QApplication::setApplicationDisplayName( "US Astfem Simulation Module" );
   QApplication::setApplicationVersion( US_Version );
   QApplication::setOrganizationDomain( "https://ultrascan.aucsoltions.com" );
   QApplication::setOrganizationName( "AUC Solutions, LLC" );
   // --help, --version and invalid options should work without a license,
   // so the command line is parsed and checked before main1.inc's license
   // dialog, not after it.
   US_SimCliArgs args;
   int           cli_exit_code = 0;
   if ( parseSimCli( US_SimCli::Astfem, args, cli_exit_code ) )
   {
      return cli_exit_code;
   }

#include "main1.inc"

   // License is OK. Start up.

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

   // Constructed after --no-db is applied, so it reads the overridden
   // data location.
   US_Astfem_Sim w;

   chooseNoiseSeedIfUnset( args );
   int init_status = w.init_from_args( args );
   // Show the GUI only if needed.
   return showGuiIfNeeded( w, init_status, args );
}
