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
