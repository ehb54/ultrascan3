//! \file us_sim_inputs_gen.cpp
//! \brief Generate reproducible inputs for headless UltraScan simulations.
//! Uses US_SimInputs without GUI interaction, database access, or network
//! access.

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QDir>

#include "us_sim_inputs.h"
#include "us_hardware.h"
#include "us_util.h"
#include "us_defines.h"

// Derive option help from the same coefficient table used for validation.
static QString coefficient_help( const US_SimSpecies::Coefficient& coeff )
{
   QString help( coeff.description );

   if ( QString( coeff.name ) == "s" )
      help += "; append S for Svedbergs (for example, 4.5S or 4.5e-13)";

   return help;
}

// Accept Svedbergs with an S suffix or the stored unit of seconds. Reject a
// likely suffix omission instead of applying a value 1e13 times too large.
static bool parse_svedberg( const QString& text, double& value, QString& error )
{
   QString trimmed = text.trimmed();
   bool    svedberg = trimmed.endsWith( "S", Qt::CaseInsensitive )
                      && ! trimmed.endsWith( "eS", Qt::CaseInsensitive );

   if ( svedberg )
      trimmed.chop( 1 );

   bool numeric = false;
   value        = trimmed.toDouble( &numeric );

   if ( ! numeric )
   {
      error = QString( "--s \"%1\" is not numeric" ).arg( text );
      return false;
   }

   if ( svedberg )
   {
      value *= 1.0e-13;
      return true;
   }

   // Values in seconds are normally on the order of 1e-13.
   if ( qAbs( value ) > 1.0e-9 )
   {
      error = QString( "--s %1 is too large to be a value in seconds; append "
                       "S for Svedbergs (--s %1S) or give seconds directly" )
         .arg( text );
      return false;
   }

   return true;
}

// Leave omitted coefficients unsupplied unless no coefficient was given.
static bool parse_component( const QCommandLineParser& parser,
                             const QMap< QString, QCommandLineOption >& options,
                             US_SimSpecies::Component& component,
                             bool& any_supplied, QString& error )
{
   US_SimSpecies::Component supplied;
   any_supplied = false;

   for ( const US_SimSpecies::Coefficient& coeff : US_SimSpecies::coefficients() )
   {
      const QString name = coeff.name;
      if ( ! parser.isSet( *options.constFind( name ) ) )
         continue;

      any_supplied = true;
      const QString text = parser.value( *options.constFind( name ) );
      double        value = 0.0;

      if ( name == "s" )
      {
         if ( ! parse_svedberg( text, value, error ) )
            return false;
      }
      else
      {
         bool numeric = false;
         value        = text.toDouble( &numeric );
         if ( ! numeric )
         {
            error = QString( "--%1 \"%2\" is not numeric" ).arg( name, text );
            return false;
         }
      }

      supplied.*( coeff.field ) = value;
   }

   if ( parser.isSet( *options.constFind( "vbar20" ) ) )
   {
      bool numeric = false;
      supplied.vbar20 = parser.value( *options.constFind( "vbar20" ) ).toDouble( &numeric );
      if ( ! numeric )
      {
         error = QString( "--vbar20 \"%1\" is not numeric" )
            .arg( parser.value( *options.constFind( "vbar20" ) ) );
         return false;
      }
   }

   if ( ! any_supplied )
   {  // Only vbar20, or nothing at all, was given
      double vbar20 = supplied.vbar20;
      supplied        = US_SimSpecies::defaultComponent();
      supplied.vbar20 = vbar20;
   }

   error = US_SimSpecies::validateComponent( supplied );
   if ( ! error.isEmpty() )
      return false;

   component = supplied;
   return true;
}

int main( int argc, char* argv[] )
{
   QCoreApplication app( argc, argv );
   QCoreApplication::setApplicationName( "us_sim_inputs_gen" );
   QCoreApplication::setApplicationVersion( US_Version );

   QCommandLineParser parser;
   parser.setApplicationDescription(
      "Generate reproducible model, buffer, and simulation-parameter inputs for "
      "headless UltraScan simulation runs." );
   parser.addHelpOption();
   parser.addVersionOption();

   QCommandLineOption out_option( "out",
      "Output path: an existing directory in default or MWL mode, or a file "
      "for --emit-model, --emit-buffer, or --emit-simparams", "path" );
   parser.addOption( out_option );
   QCommandLineOption runid_option( "run-id",
      "Run ID for a per-wavelength MWL model; requires --channel and --wavelength",
      "id" );
   parser.addOption( runid_option );
   QCommandLineOption channel_option( "channel",
      "Two-character cell and channel code (for example, 1A)", "channel" );
   parser.addOption( channel_option );
   QCommandLineOption wavelength_option( "wavelength",
      "Three-digit wavelength in nm (for example, 280)", "nm" );
   parser.addOption( wavelength_option );

   // Generate model and buffer files from physical parameters supplied by the
   // caller. US_Model and US_Buffer serialize the resulting values.
   QCommandLineOption emit_model_option( "emit-model",
      "Write a single-component model XML from explicit physical parameters "
      "to the file specified by --out; requires exactly two of --s, --D, --mw, "
      "--f, and --f-f0, from which the remaining three are calculated; "
      "--vbar20 and --description are optional" );
   parser.addOption( emit_model_option );

   QMap< QString, QCommandLineOption > coefficient_opts;
   for ( const US_SimSpecies::Coefficient& coeff : US_SimSpecies::coefficients() )
   {
      QCommandLineOption option( coeff.name, coefficient_help( coeff ), "value" );
      parser.addOption( option );
      coefficient_opts.insert( coeff.name, option );
   }
   QCommandLineOption vbar20_option( "vbar20",
      "Partial specific volume at 20 C (mL/g); defaults to the typical protein "
      "value", "value" );
   parser.addOption( vbar20_option );
   coefficient_opts.insert( "vbar20", vbar20_option );
   QCommandLineOption model_description_option( "description",
      "Model or buffer description; a model description must not contain "
      "\".\", which us_mwl_species_sim treats as a field separator", "text" );
   parser.addOption( model_description_option );

   QCommandLineOption emit_buffer_option( "emit-buffer",
      "Write a buffer XML from explicit physical parameters (requires "
      "--density, --viscosity, and --ph) to the file specified by --out; "
      "--description is optional" );
   parser.addOption( emit_buffer_option );
   QCommandLineOption density_option( "density", "Buffer density (g/mL)", "value" );
   parser.addOption( density_option );
   QCommandLineOption viscosity_option( "viscosity", "Buffer viscosity (poise)", "value" );
   parser.addOption( viscosity_option );
   QCommandLineOption ph_option( "ph", "Buffer pH", "value" );
   parser.addOption( ph_option );

   // Generate simulation parameters using US_SimInputs::simParams() defaults
   // for omitted options. Omitting every option produces the same content as
   // sp_default.xml in default mode.
   QCommandLineOption emit_simparams_option( "emit-simparams",
      "Write a simulation-parameter XML to the file specified by --out; all "
      "run-condition options are optional and use documented defaults" );
   parser.addOption( emit_simparams_option );
   QCommandLineOption speed_option( "speed", "Rotor speed (rpm)", "value" );
   parser.addOption( speed_option );
   QCommandLineOption duration_hrs_option( "duration-hrs", "Hours component of the run duration", "value" );
   parser.addOption( duration_hrs_option );
   QCommandLineOption duration_mins_option( "duration-mins", "Minutes component of the run duration", "value" );
   parser.addOption( duration_mins_option );
   QCommandLineOption delay_hrs_option( "delay-hrs",
      "Hours component of the delay before the first scan; defaults to the "
      "time needed to reach speed. Useful for aligning scan times across runs "
      "at different speeds", "value" );
   parser.addOption( delay_hrs_option );
   QCommandLineOption delay_mins_option( "delay-mins",
      "Minutes component of the delay before the first scan", "value" );
   parser.addOption( delay_mins_option );
   QCommandLineOption scans_option( "scans", "Number of scans", "value" );
   parser.addOption( scans_option );
   QCommandLineOption points_option( "points", "Number of points in the radial simulation grid", "value" );
   parser.addOption( points_option );
   QCommandLineOption acceleration_option( "acceleration", "Rotor acceleration (rpm/s)", "value" );
   parser.addOption( acceleration_option );
   QCommandLineOption rnoise_option( "rnoise", "Random noise, proportional to total concentration", "value" );
   parser.addOption( rnoise_option );
   QCommandLineOption lrnoise_option( "lrnoise", "Random noise, proportional to local concentration", "value" );
   parser.addOption( lrnoise_option );
   QCommandLineOption tinoise_option( "tinoise", "Time-invariant noise", "value" );
   parser.addOption( tinoise_option );
   QCommandLineOption rinoise_option( "rinoise", "Radially invariant noise", "value" );
   parser.addOption( rinoise_option );
   QCommandLineOption sp_baseline_option( "baseline", "Constant baseline offset", "value" );
   parser.addOption( sp_baseline_option );
   QCommandLineOption radial_resolution_option( "radial-resolution", "Radial grid spacing", "value" );
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
      "entry rather than a rotor file)", "id" );
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
      if ( ! parser.isSet( out_option ) )
      {
         QTextStream( stderr ) << "Error: --out <file path> is required" << Qt::endl;
         return 1;
      }

      // --emit-model requires an explicitly defined species.
      US_SimSpecies::Component component;
      bool    any_supplied = false;
      QString component_error;
      if ( ! parse_component( parser, coefficient_opts, component,
                              any_supplied, component_error ) )
      {
         QTextStream( stderr ) << "Error: " << component_error << Qt::endl;
         return 1;
      }
      if ( ! any_supplied )
      {
         QTextStream( stderr ) << "Error: --emit-model requires exactly two of "
            "--s, --D, --mw, --f, and --f-f0" << Qt::endl;
         return 1;
      }

      QString description = parser.isSet( model_description_option )
         ? parser.value( model_description_option ) : QString( "us_sim_inputs_gen emitted model" );
      if ( description.contains( "." ) )
      {
         QTextStream( stderr ) << "Error: a model --description must not "
            "contain \".\"; us_mwl_species_sim splits descriptions on it"
            << Qt::endl;
         return 1;
      }
      US_Model model_out = US_SimSpecies::model( component );
      model_out.description          = description;
      model_out.components[ 0 ].name = description;

      if ( model_out.write( parser.value( out_option ) ) != IUS_DB2::OK )
      {
         QTextStream( stderr ) << "Error: could not write "
            << parser.value( out_option ) << Qt::endl;
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
         QTextStream( stderr ) << "Error: values for --density, --viscosity, "
            "and --ph must be numeric" << Qt::endl;
         return 1;
      }

      if ( density <= 0.0 || viscosity <= 0.0 )
      {
         QTextStream( stderr ) << "Error: --density and --viscosity must be "
            "greater than zero" << Qt::endl;
         return 1;
      }
      if ( ph < 0.0 || ph > 14.0 )
      {
         QTextStream( stderr ) << "Error: --ph must be between 0 and 14"
            << Qt::endl;
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
      // Preserve the supplied density and viscosity without deriving them
      // from buffer components.
      buffer_out.manual          = true;

      if ( ! buffer_out.writeToDisk( parser.value( out_option ) ) )
      {
         QTextStream( stderr ) << "Error: could not write "
            << parser.value( out_option ) << Qt::endl;
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

      // Use shared defaults for omitted options.
      US_SimInputs::Params sp_params;

      sp_params.rpm                 = opt_double( speed_option, sp_params.rpm );
      sp_params.duration_hours      = opt_int( duration_hrs_option, sp_params.duration_hours );
      sp_params.duration_minutes    = opt_double( duration_mins_option, sp_params.duration_minutes );
      sp_params.delay_hours         = opt_int( delay_hrs_option, sp_params.delay_hours );
      sp_params.delay_minutes       = opt_double( delay_mins_option, sp_params.delay_minutes );
      sp_params.scans               = opt_int( scans_option, sp_params.scans );
      sp_params.acceleration        = opt_double( acceleration_option, sp_params.acceleration );
      sp_params.simpoints           = opt_int( points_option, sp_params.simpoints );
      sp_params.radial_resolution   = opt_double( radial_resolution_option, sp_params.radial_resolution );
      sp_params.rnoise              = opt_double( rnoise_option, sp_params.rnoise );
      sp_params.lrnoise             = opt_double( lrnoise_option, sp_params.lrnoise );
      sp_params.tinoise             = opt_double( tinoise_option, sp_params.tinoise );
      sp_params.rinoise             = opt_double( rinoise_option, sp_params.rinoise );
      sp_params.baseline            = opt_double( sp_baseline_option, sp_params.baseline );
      sp_params.band_volume         = opt_double( band_volume_option, sp_params.band_volume );
      sp_params.centerpiece         = opt_int( centerpiece_option, sp_params.centerpiece );
      sp_params.centerpiece_channel = opt_int( centerpiece_channel_option,
                                                sp_params.centerpiece_channel );

      if ( ! ok )
      {
         QTextStream( stderr ) << "Error: one or more values supplied to "
            "--emit-simparams are not numeric" << Qt::endl;
         return 1;
      }

      // Numeric parsing alone does not reject invalid negative values.
      struct { const char* name; double value; bool allow_zero; } positive[] = {
         { "--speed",             sp_params.rpm,               false },
         { "--acceleration",      sp_params.acceleration,      false },
         { "--scans",             (double)sp_params.scans,     false },
         { "--points",            (double)sp_params.simpoints, false },
         { "--radial-resolution", sp_params.radial_resolution, false },
         { "--duration-hrs",      (double)sp_params.duration_hours,  true },
         { "--duration-mins",     sp_params.duration_minutes,        true },
         { "--band-volume",       sp_params.band_volume,             true },
      };

      for ( const auto& check : positive )
      {
         if ( check.allow_zero ? ( check.value < 0.0 ) : ( check.value <= 0.0 ) )
         {
            QTextStream( stderr ) << "Error: " << check.name << " must be "
               << ( check.allow_zero ? "zero or greater" : "greater than zero" )
               << " (got " << check.value << ")" << Qt::endl;
            return 1;
         }
      }

      if ( sp_params.duration_hours == 0  &&  sp_params.duration_minutes <= 0.0 )
      {
         QTextStream( stderr ) << "Error: the run duration must be greater "
            "than zero" << Qt::endl;
         return 1;
      }

      QString cp_error = US_AbstractCenterpiece::validate(
         sp_params.centerpiece, sp_params.centerpiece_channel );
      if ( ! cp_error.isEmpty() )
      {
         QTextStream( stderr ) << "Error: " << cp_error << Qt::endl;
         return 1;
      }

      QStringList mesh_names, grid_names;
      mesh_names << "ASTFEM" << "Claverie" << "MovingHat" << "User" << "ASTFVM";
      grid_names << "Fixed" << "Moving";

      if ( parser.isSet( mesh_type_option ) )
      {
         int idx = mesh_names.indexOf( parser.value( mesh_type_option ) );
         if ( idx < 0 )
         {
            QTextStream( stderr ) << "Error: --mesh-type must be one of "
               << mesh_names.join( "|" ) << Qt::endl;
            return 1;
         }
         sp_params.meshType = (US_SimulationParameters::MeshType)idx;
      }

      if ( parser.isSet( grid_type_option ) )
      {
         int idx = grid_names.indexOf( parser.value( grid_type_option ) );
         if ( idx < 0 )
         {
            QTextStream( stderr ) << "Error: --grid-type must be one of "
               << grid_names.join( "|" ) << Qt::endl;
            return 1;
         }
         sp_params.gridType = (US_SimulationParameters::GridType)idx;
      }

      if ( parser.isSet( rotor_calibration_option ) )
         sp_params.rotor_calibr = parser.value( rotor_calibration_option );
      sp_params.band_forming = parser.isSet( band_forming_option );

      US_SimulationParameters sp_out = US_SimInputs::simParams( sp_params );

      if ( sp_out.save_simparms( parser.value( out_option ) ) != 0 )
      {
         QTextStream( stderr ) << "Error: could not write "
            << parser.value( out_option ) << Qt::endl;
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
      // Generate one model whose description follows the
      // <runid>.<channel><wavelength>.<...> convention parsed by
      // us_mwl_species_sim. A run shares its buffer and simulation parameters
      // across wavelengths; generate those once in default mode.
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
         QTextStream( stderr ) << "Error: --channel must be exactly two "
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

      // MWL mode permits the default species when coefficients are omitted.
      US_SimSpecies::Component component;
      bool    any_supplied = false;
      QString component_error;
      if ( ! parse_component( parser, coefficient_opts, component,
                              any_supplied, component_error ) )
      {
         QTextStream( stderr ) << "Error: " << component_error << Qt::endl;
         return 1;
      }

      US_Model model    = US_SimSpecies::model( component );
      model.description = QString( "%1.%2%3.model.default" )
                           .arg( run_id, channel, wavelength );
      model.modelGUID    = US_Util::new_guid();

      QString fname = QString( "model_%1_%2.xml" ).arg( channel, wavelength );
      if ( model.write( outdir.filePath( fname ) ) != IUS_DB2::OK )
      {
         QTextStream( stderr ) << "Error: could not write " << fname << Qt::endl;
         return 2;
      }
      QTextStream( stdout ) << "Wrote " << outdir.filePath( fname ) << Qt::endl;
      return 0;
   }

   // Default mode writes the simulation parameters, model, and buffer files.
   if ( ! US_SimInputs::writeAll( outdir.path() ) )
   {
      QTextStream( stderr ) << "Error: could not write default inputs to "
         << outdir.path()
         << Qt::endl;
      return 2;
   }
   QTextStream( stdout ) << "Wrote sp_default.xml, model_default.xml, "
      "buffer_default.xml to " << outdir.path() << Qt::endl;
   return 0;
}
