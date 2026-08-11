//! \file us_sim_inputs_gen.cpp
//! \brief Headless CLI: writes reproducible default model/buffer/simparams
//! inputs (via US_SimInputs) for driving us_astfem_sim/us_mwl_species_sim
//! non-interactively. No GUI, no DB, no network access.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDir>

#include "us_sim_inputs.h"
#include "us_util.h"
#include "us_defines.h"

// Build a single-component model from explicit physical parameters, shared
// by --emit-model and the per-wavelength MWL mode's optional override.
// Starts from US_SimInputs::model() so the model-construction sequence
// (optics/analysis/GUID/component/update_coefficients) lives in one place.
static US_Model model_from_params( double mw, double vbar20, double f_f0,
                                    const QString& description )
{
   US_Model model_out = US_SimInputs::model();
   model_out.description             = description;
   model_out.components[ 0 ].mw      = mw;
   model_out.components[ 0 ].vbar20  = vbar20;
   model_out.components[ 0 ].f_f0    = f_f0;
   model_out.components[ 0 ].name    = description;
   model_out.update_coefficients();

   return model_out;
}

int main( int argc, char* argv[] )
{
   QCoreApplication app( argc, argv );
   QCoreApplication::setApplicationName( "us_sim_inputs_gen" );
   QCoreApplication::setApplicationVersion( US_Version );

   QCommandLineParser parser;
   parser.setApplicationDescription(
      "Write reproducible default model/buffer/simparams inputs for "
      "headless UltraScan simulation runs." );
   parser.addHelpOption();
   parser.addVersionOption();

   QCommandLineOption out_option( "out",
      "Output directory (must already exist)", "dir" );
   parser.addOption( out_option );
   QCommandLineOption runid_option( "run-id",
      "Run ID for a per-wavelength MWL model (requires --channel and --wavelength)",
      "id" );
   parser.addOption( runid_option );
   QCommandLineOption channel_option( "channel",
      "Two-character cell+channel code, e.g. 1A", "channel" );
   parser.addOption( channel_option );
   QCommandLineOption wavelength_option( "wavelength",
      "Three-digit wavelength in nm, e.g. 280", "nm" );
   parser.addOption( wavelength_option );

   // Generic, no-baked-in-knowledge model/buffer emitters: the caller
   // supplies real physical parameters (e.g. from a chemistry reference
   // dataset) on the command line -- this tool has no notion of what a
   // "category" is, it just writes whatever numbers it's given via
   // US_Model's/US_Buffer's own native serializers.
   QCommandLineOption emit_model_option( "emit-model",
      "Write a single-component model XML from explicit physical parameters "
      "(requires --mw, --vbar20, --f-f0; --description optional) to --out "
      "as a file path, not a directory" );
   parser.addOption( emit_model_option );
   QCommandLineOption mw_option( "mw", "Molecular weight (Da)", "value" );
   parser.addOption( mw_option );
   QCommandLineOption vbar20_option( "vbar20", "Partial specific volume at 20C (mL/g)", "value" );
   parser.addOption( vbar20_option );
   QCommandLineOption f_f0_option( "f-f0", "Frictional ratio (1.0 = perfect sphere)", "value" );
   parser.addOption( f_f0_option );
   QCommandLineOption model_description_option( "description", "Model or buffer description text", "text" );
   parser.addOption( model_description_option );

   QCommandLineOption emit_buffer_option( "emit-buffer",
      "Write a buffer XML from explicit physical parameters (requires "
      "--density, --viscosity, --ph; --description optional) to --out as a "
      "file path, not a directory" );
   parser.addOption( emit_buffer_option );
   QCommandLineOption density_option( "density", "Buffer density (g/mL)", "value" );
   parser.addOption( density_option );
   QCommandLineOption viscosity_option( "viscosity", "Buffer viscosity (poise)", "value" );
   parser.addOption( viscosity_option );
   QCommandLineOption ph_option( "ph", "Buffer pH", "value" );
   parser.addOption( ph_option );

   // Generic simparams emitter: every argument optional, defaulting to
   // US_SimInputs::simParams()'s own defaults (see us_sim_inputs.h) so
   // omitting all of them reproduces exactly what plain mode already wrote
   // as sp_default.xml.
   QCommandLineOption emit_simparams_option( "emit-simparams",
      "Write a simparams XML from explicit run-condition parameters (all "
      "optional, default to the original fixed recipe) to --out as a file "
      "path, not a directory" );
   parser.addOption( emit_simparams_option );
   QCommandLineOption speed_option( "speed", "Rotor speed (rpm)", "value" );
   parser.addOption( speed_option );
   QCommandLineOption duration_hrs_option( "duration-hrs", "Run duration, hours part", "value" );
   parser.addOption( duration_hrs_option );
   QCommandLineOption duration_mins_option( "duration-mins", "Run duration, minutes part", "value" );
   parser.addOption( duration_mins_option );
   QCommandLineOption scans_option( "scans", "Number of scans", "value" );
   parser.addOption( scans_option );
   QCommandLineOption points_option( "points", "Number of simulation radial grid points", "value" );
   parser.addOption( points_option );
   QCommandLineOption acceleration_option( "acceleration", "Rotor acceleration (rpm/sec)", "value" );
   parser.addOption( acceleration_option );
   QCommandLineOption rnoise_option( "rnoise", "Random noise, proportional to total concentration", "value" );
   parser.addOption( rnoise_option );
   QCommandLineOption lrnoise_option( "lrnoise", "Random noise, proportional to local concentration", "value" );
   parser.addOption( lrnoise_option );
   QCommandLineOption tinoise_option( "tinoise", "Time invariant noise", "value" );
   parser.addOption( tinoise_option );
   QCommandLineOption rinoise_option( "rinoise", "Radially invariant noise", "value" );
   parser.addOption( rinoise_option );
   QCommandLineOption sp_baseline_option( "baseline", "Constant baseline offset", "value" );
   parser.addOption( sp_baseline_option );
   QCommandLineOption radial_resolution_option( "radial-resolution", "Radial datapoint increment/resolution", "value" );
   parser.addOption( radial_resolution_option );
   QCommandLineOption mesh_type_option( "mesh-type", "ASTFEM|Claverie|MovingHat|User|ASTFVM", "type" );
   parser.addOption( mesh_type_option );
   QCommandLineOption grid_type_option( "grid-type", "Fixed|Moving", "type" );
   parser.addOption( grid_type_option );
   QCommandLineOption band_forming_option( "band-forming", "Use a band-forming centerpiece" );
   parser.addOption( band_forming_option );
   QCommandLineOption band_volume_option( "band-volume", "Loading volume for a band-forming centerpiece", "value" );
   parser.addOption( band_volume_option );
   QCommandLineOption rotor_calibration_option( "rotor-calibration",
      "Rotor calibration ID (default \"0\", a built-in zero-stretch-correction "
      "entry, not a real rotor file)", "id" );
   parser.addOption( rotor_calibration_option );
   QCommandLineOption centerpiece_option( "centerpiece", "Centerpiece list index", "index" );
   parser.addOption( centerpiece_option );
   QCommandLineOption centerpiece_channel_option( "centerpiece-channel",
      "Channel index within the centerpiece (not an instrument channel "
      "label like \"1A\")", "index" );
   parser.addOption( centerpiece_channel_option );

   parser.process( app );

   if ( parser.isSet( emit_model_option ) )
   {
      if ( ! ( parser.isSet( mw_option ) && parser.isSet( vbar20_option ) && parser.isSet( f_f0_option ) ) )
      {
         QTextStream( stderr ) << "Error: --emit-model requires --mw, --vbar20, and --f-f0" << Qt::endl;
         return 1;
      }
      if ( ! parser.isSet( out_option ) )
      {
         QTextStream( stderr ) << "Error: --out <file path> is required" << Qt::endl;
         return 1;
      }

      bool ok_mw = false, ok_vbar = false, ok_ff0 = false;
      double mw     = parser.value( mw_option ).toDouble( &ok_mw );
      double vbar20 = parser.value( vbar20_option ).toDouble( &ok_vbar );
      double f_f0   = parser.value( f_f0_option ).toDouble( &ok_ff0 );
      if ( ! ok_mw || ! ok_vbar || ! ok_ff0 )
      {
         QTextStream( stderr ) << "Error: --mw/--vbar20/--f-f0 must be numeric" << Qt::endl;
         return 1;
      }

      QString description = parser.isSet( model_description_option )
         ? parser.value( model_description_option ) : QString( "us_sim_inputs_gen emitted model" );
      US_Model model_out = model_from_params( mw, vbar20, f_f0, description );

      if ( model_out.write( parser.value( out_option ) ) != IUS_DB2::OK )
      {
         QTextStream( stderr ) << "Error writing " << parser.value( out_option ) << Qt::endl;
         return 2;
      }
      QTextStream( stdout ) << "Wrote " << parser.value( out_option ) << Qt::endl;
      return 0;
   }

   if ( parser.isSet( emit_buffer_option ) )
   {
      if ( ! ( parser.isSet( density_option ) && parser.isSet( viscosity_option ) && parser.isSet( ph_option ) ) )
      {
         QTextStream( stderr ) << "Error: --emit-buffer requires --density, --viscosity, and --ph" << Qt::endl;
         return 1;
      }
      if ( ! parser.isSet( out_option ) )
      {
         QTextStream( stderr ) << "Error: --out <file path> is required" << Qt::endl;
         return 1;
      }

      bool ok_d = false, ok_v = false, ok_p = false;
      double density   = parser.value( density_option ).toDouble( &ok_d );
      double viscosity = parser.value( viscosity_option ).toDouble( &ok_v );
      double ph        = parser.value( ph_option ).toDouble( &ok_p );
      if ( ! ok_d || ! ok_v || ! ok_p )
      {
         QTextStream( stderr ) << "Error: --density/--viscosity/--ph must be numeric" << Qt::endl;
         return 1;
      }

      US_Buffer buffer_out;
      buffer_out.description = parser.isSet( model_description_option )
         ? parser.value( model_description_option ) : QString( "us_sim_inputs_gen emitted buffer" );
      buffer_out.GUID            = US_Util::new_guid();
      buffer_out.density         = density;
      buffer_out.viscosity       = viscosity;
      buffer_out.pH              = ph;
      buffer_out.compressibility = 0.0;
      buffer_out.manual          = true; // explicit unadjusted density/viscosity, not component-derived

      if ( ! buffer_out.writeToDisk( parser.value( out_option ) ) )
      {
         QTextStream( stderr ) << "Error writing " << parser.value( out_option ) << Qt::endl;
         return 2;
      }
      QTextStream( stdout ) << "Wrote " << parser.value( out_option ) << Qt::endl;
      return 0;
   }

   if ( parser.isSet( emit_simparams_option ) )
   {
      if ( ! parser.isSet( out_option ) )
      {
         QTextStream( stderr ) << "Error: --out <file path> is required" << Qt::endl;
         return 1;
      }

      bool ok = true;
      auto opt_double = [&]( const QCommandLineOption& opt, double def )
      {
         if ( ! parser.isSet( opt ) ) return def;
         bool this_ok = false;
         double v = parser.value( opt ).toDouble( &this_ok );
         ok = ok && this_ok;
         return this_ok ? v : def;
      };
      auto opt_int = [&]( const QCommandLineOption& opt, int def )
      {
         if ( ! parser.isSet( opt ) ) return def;
         bool this_ok = false;
         int v = parser.value( opt ).toInt( &this_ok );
         ok = ok && this_ok;
         return this_ok ? v : def;
      };

      double rpm               = opt_double( speed_option, 45000.0 );
      int    duration_hours    = opt_int( duration_hrs_option, 2 );
      double duration_minutes  = opt_double( duration_mins_option, 30.0 );
      int    scans             = opt_int( scans_option, 30 );
      double acceleration      = opt_double( acceleration_option, 400.0 );
      int    simpoints         = opt_int( points_option, 200 );
      double radial_resolution = opt_double( radial_resolution_option, 0.001 );
      double rnoise            = opt_double( rnoise_option, 0.0 );
      double lrnoise           = opt_double( lrnoise_option, 0.0 );
      double tinoise           = opt_double( tinoise_option, 0.0 );
      double rinoise           = opt_double( rinoise_option, 0.0 );
      double baseline          = opt_double( sp_baseline_option, 0.0 );
      double band_volume       = opt_double( band_volume_option, 0.015 );
      int    centerpiece       = opt_int( centerpiece_option, 0 );
      int    centerpiece_chan  = opt_int( centerpiece_channel_option, 0 );

      if ( ! ok )
      {
         QTextStream( stderr ) << "Error: one or more --emit-simparams "
            "values is not numeric" << Qt::endl;
         return 1;
      }

      if ( acceleration <= 0.0 )
      {
         QTextStream( stderr ) << "Error: --acceleration must be greater "
            "than 0" << Qt::endl;
         return 1;
      }

      QString cp_error = US_SimInputs::validateCenterpiece( centerpiece, centerpiece_chan );
      if ( ! cp_error.isEmpty() )
      {
         QTextStream( stderr ) << "Error: " << cp_error << Qt::endl;
         return 1;
      }

      QStringList mesh_names, grid_names;
      mesh_names << "ASTFEM" << "Claverie" << "MovingHat" << "User" << "ASTFVM";
      grid_names << "Fixed" << "Moving";

      US_SimulationParameters::MeshType meshType = US_SimulationParameters::ASTFEM;
      if ( parser.isSet( mesh_type_option ) )
      {
         int idx = mesh_names.indexOf( parser.value( mesh_type_option ) );
         if ( idx < 0 )
         {
            QTextStream( stderr ) << "Error: --mesh-type must be one of "
               << mesh_names.join( "|" ) << Qt::endl;
            return 1;
         }
         meshType = (US_SimulationParameters::MeshType)idx;
      }

      US_SimulationParameters::GridType gridType = US_SimulationParameters::MOVING;
      if ( parser.isSet( grid_type_option ) )
      {
         int idx = grid_names.indexOf( parser.value( grid_type_option ) );
         if ( idx < 0 )
         {
            QTextStream( stderr ) << "Error: --grid-type must be one of "
               << grid_names.join( "|" ) << Qt::endl;
            return 1;
         }
         gridType = (US_SimulationParameters::GridType)idx;
      }

      QString rotor_calibr = parser.isSet( rotor_calibration_option )
         ? parser.value( rotor_calibration_option ) : QString( "0" );
      bool band_forming = parser.isSet( band_forming_option );

      US_SimulationParameters sp_out = US_SimInputs::simParams(
         rpm, duration_hours, duration_minutes, scans, acceleration,
         simpoints, radial_resolution, meshType, gridType,
         rnoise, lrnoise, tinoise, rinoise, baseline,
         band_forming, band_volume, rotor_calibr, centerpiece, centerpiece_chan );

      if ( sp_out.save_simparms( parser.value( out_option ) ) != 0 )
      {
         QTextStream( stderr ) << "Error writing " << parser.value( out_option ) << Qt::endl;
         return 2;
      }
      QTextStream( stdout ) << "Wrote " << parser.value( out_option ) << Qt::endl;
      return 0;
   }

   if ( ! parser.isSet( out_option ) )
   {
      QTextStream( stderr ) << "Error: --out <dir> is required" << Qt::endl;
      return 1;
   }

   QDir outdir( parser.value( out_option ) );
   if ( ! outdir.exists() )
   {
      QTextStream( stderr ) << "Error: output directory does not exist: "
         << outdir.path() << Qt::endl;
      return 1;
   }

   bool has_runid      = parser.isSet( runid_option );
   bool has_channel    = parser.isSet( channel_option );
   bool has_wavelength = parser.isSet( wavelength_option );

   if ( has_runid || has_channel || has_wavelength )
   {
      // Per-wavelength MWL mode: emit one model file whose description
      // matches the <runid>.<channel><wavelength>.<...> convention
      // us_mwl_species_sim's load_models_from_paths() parses. Buffer and
      // simparams are shared across a run's wavelengths -- generate those
      // once, separately, with plain mode below.
      if ( ! ( has_runid && has_channel && has_wavelength ) )
      {
         QTextStream( stderr ) << "Error: --run-id, --channel, and "
            "--wavelength must all be given together" << Qt::endl;
         return 1;
      }

      QString run_id    = parser.value( runid_option );
      QString channel   = parser.value( channel_option );
      QString wavelength = parser.value( wavelength_option );

      if ( channel.length() != 2 )
      {
         QTextStream( stderr ) << "Error: --channel must be exactly 2 "
            "characters" << Qt::endl;
         return 1;
      }
      bool wl_ok = false;
      wavelength.toInt( &wl_ok );
      if ( ! wl_ok || wavelength.length() != 3 )
      {
         QTextStream( stderr ) << "Error: --wavelength must be a 3-digit "
            "number" << Qt::endl;
         return 1;
      }

      // Same optional explicit-physical-parameters override as --emit-model,
      // so a multi-wavelength run can use real chemistry too, not just the
      // generic default -- falls back to the generic default if omitted.
      US_Model model;
      if ( parser.isSet( mw_option ) && parser.isSet( vbar20_option ) && parser.isSet( f_f0_option ) )
      {
         bool ok_mw = false, ok_vbar = false, ok_ff0 = false;
         double mw     = parser.value( mw_option ).toDouble( &ok_mw );
         double vbar20 = parser.value( vbar20_option ).toDouble( &ok_vbar );
         double f_f0   = parser.value( f_f0_option ).toDouble( &ok_ff0 );
         if ( ! ok_mw || ! ok_vbar || ! ok_ff0 )
         {
            QTextStream( stderr ) << "Error: --mw/--vbar20/--f-f0 must be numeric" << Qt::endl;
            return 1;
         }
         model = model_from_params( mw, vbar20, f_f0, QString() );
      }
      else
      {
         model = US_SimInputs::model();
      }
      model.description = QString( "%1.%2%3.model.default" )
                           .arg( run_id, channel, wavelength );
      model.modelGUID    = US_Util::new_guid();

      QString fname = QString( "model_%1_%2.xml" ).arg( channel, wavelength );
      if ( model.write( outdir.filePath( fname ) ) != IUS_DB2::OK )
      {
         QTextStream( stderr ) << "Error writing " << fname << Qt::endl;
         return 2;
      }
      QTextStream( stdout ) << "Wrote " << outdir.filePath( fname ) << Qt::endl;
      return 0;
   }

   // Plain mode: emit the full default sp_default.xml/model_default.xml/
   // buffer_default.xml triple.
   if ( ! US_SimInputs::writeAll( outdir.path() ) )
   {
      QTextStream( stderr ) << "Error writing defaults to " << outdir.path()
         << Qt::endl;
      return 2;
   }
   QTextStream( stdout ) << "Wrote sp_default.xml, model_default.xml, "
      "buffer_default.xml to " << outdir.path() << Qt::endl;
   return 0;
}
