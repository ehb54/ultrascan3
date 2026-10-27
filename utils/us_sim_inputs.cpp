//! \file us_sim_inputs.cpp
#include "us_sim_inputs.h"
#include "us_constants.h"
#include "us_hardware.h"
#include "us_sim_record.h"
#include "us_util.h"
#include <QDir>
#include <QStringList>

US_SimInputs::Params::Params()
{
   // Share defaults with the underlying parameter types.
   US_SimulationParameters               sp_defaults;
   US_SimulationParameters::SpeedProfile step_defaults;

   // Match the run conditions constructed by us_astfem_sim.
   rpm              = 45000.0;
   duration_hours   = 2;
   duration_minutes = 30.0;
   scans            = 30;

   // Negative means "derive from the time needed to reach speed".
   delay_hours   = -1;
   delay_minutes = -1.0;

   acceleration        = step_defaults.acceleration;
   simpoints           = sp_defaults.simpoints;
   radial_resolution   = sp_defaults.radial_resolution;
   meshType            = sp_defaults.meshType;
   gridType            = sp_defaults.gridType;
   rnoise              = sp_defaults.rnoise;
   lrnoise             = sp_defaults.lrnoise;
   tinoise             = sp_defaults.tinoise;
   rinoise             = sp_defaults.rinoise;
   baseline            = sp_defaults.baseline;
   band_forming        = sp_defaults.band_forming;
   band_volume         = sp_defaults.band_volume;
   rotor_calibr        = sp_defaults.rotorCalID;
   centerpiece         = 0;
   centerpiece_channel = 0;
}

QString US_SimInputs::validateParams( const Params& p )
{
   struct PositiveValue
   {
         const char* name;
         double      value;
         bool        allow_zero;
   };
   const PositiveValue positive[] = {
      { "speed", p.rpm, false },
      { "acceleration", p.acceleration, false },
      { "points", (double) p.simpoints, false },
      { "radial resolution", p.radial_resolution, false },
      { "duration hours", (double) p.duration_hours, true },
      { "duration minutes", p.duration_minutes, true },
      { "band volume", p.band_volume, true },
   };

   for ( const PositiveValue& check : positive )
   {
      if ( !qIsFinite( check.value ) || ( check.allow_zero ? check.value < 0.0 : check.value <= 0.0 ) )
      {
         return QString( check.allow_zero ? negativeMessage : notPositiveMessage ).arg( check.name ).arg( check.value );
      }
   }

   if ( p.scans < 1 )
   {
      return QString( tooFewScansMessage ).arg( p.scans );
   }

   // No ultracentrifuge rotor runs this fast; catch an order-of-magnitude typo
   // rather than let the simulator spin forever on it.
   if ( p.rpm > 200000.0 )
   {
      return QString( "speed must be 200000 rpm or less (got %1)" ).arg( p.rpm );
   }

   // Bounds memory and run time; a legitimate run rarely needs more than this.
   if ( p.scans > 10000 )
   {
      return QString( "scans must be 10000 or less (got %1)" ).arg( p.scans );
   }

   if ( p.duration_hours == 0 && p.duration_minutes <= 0.0 )
   {
      return "run duration must be greater than zero";
   }

   // The speed profile stores acceleration as a whole number of rpm/s, and a
   // rounded value of zero makes the acceleration ramp infinitely long.
   if ( qRound( p.acceleration ) < 1 )
   {
      return QString( "acceleration must be at least 1 rpm/s (got %1)" ).arg( p.acceleration );
   }

   const double finite_values[] = { p.delay_minutes, p.rnoise, p.lrnoise, p.tinoise, p.rinoise, p.baseline };
   for ( double value : finite_values )
   {
      if ( !qIsFinite( value ) )
      {
         return "delay, noise, and baseline values must be finite";
      }
   }

   // The scan delay must be shorter than the run duration.
   const double duration_min = p.duration_hours * 60.0 + p.duration_minutes;
   const double delay_min    = ( p.delay_hours >= 0 || p.delay_minutes >= 0.0 )
                                  ? qMax( 0, p.delay_hours ) * 60.0 + qMax( 0.0, p.delay_minutes )
                                  : p.rpm / ( qRound( p.acceleration ) * 60.0 );

   if ( delay_min >= duration_min )
   {
      return QString( "delay (%1 min) must be shorter than the run duration"
                      " (%2 min)" )
         .arg( delay_min )
         .arg( duration_min );
   }

   if ( p.meshType < US_SimulationParameters::ASTFEM || p.meshType > US_SimulationParameters::ASTFVM )
   {
      return "mesh type is out of range";
   }
   if ( p.gridType < US_SimulationParameters::FIXED || p.gridType > US_SimulationParameters::MOVING )
   {
      return "grid type is out of range";
   }

   // A negative centerpiece is not out-of-range to setHardware(): it means
   // "look up by serial number" instead of by list index (see its cp < 0
   // branch). The CLI only ever means a list index, so a negative value
   // here is a bad index, not a serial lookup; left unchecked, a mistyped
   // negative silently falls back to centerpiece 0 when no serial matches,
   // instead of failing.
   if ( p.centerpiece < 0 )
   {
      return QString( "centerpiece must be zero or greater (got %1)" ).arg( p.centerpiece );
   }

   // The rest of hardware (channel, rotor calibration) is not checked here.
   // simParams() calls setHardware() right after this, which already reads
   // it; re-reading it here to pre-validate would read it twice on every
   // call. See hardwareFailureReason() below for the failure path.

   return QString();
}

// Re-derive a specific reason for a setHardware() failure, by checking the
// same hardware definitions it just tried to load. Only called on that
// failure path, so this does not cost an extra read on a successful call.
static QString hardwareFailureReason( const US_SimInputs::Params& p )
{
   QString hardware_error = US_AbstractCenterpiece::validate( p.centerpiece, p.centerpiece_channel );
   if ( !hardware_error.isEmpty() )
   {
      return hardware_error;
   }

   QMap<QString, QString> rotor_map;
   bool                   loaded_rotors = US_Hardware::readRotorMap( rotor_map );
   if ( p.rotor_calibr != "0" && ( !loaded_rotors || !rotor_map.contains( p.rotor_calibr ) ) )
   {
      return QString( "rotor calibration ID %1 was not found" ).arg( p.rotor_calibr );
   }

   return "hardware definitions changed or could not be loaded";
}

bool US_SimInputs::simParams( const Params& p, US_SimulationParameters& params_out, QString& error )
{
   error = validateParams( p );
   if ( !error.isEmpty() )
   {
      return false;
   }

   US_SimulationParameters               sp_out;
   US_SimulationParameters::SpeedProfile sp;

   if ( !sp_out.setHardware( NULL, p.rotor_calibr, p.centerpiece, p.centerpiece_channel ) )
   {
      error = hardwareFailureReason( p );
      return false;
   }

   // Analysis ignores the band volume unless the centerpiece forms a band.
   if ( p.band_forming && !sp_out.band_forming )
   {
      error = QString( notBandFormingMessage ).arg( p.centerpiece );
      return false;
   }

   // A radial_resolution that does not fit at least a handful of grid
   // points between the at-rest meniscus (below) and the centerpiece's
   // at-rest bottom produces a degenerate mesh.
   const double at_rest_meniscus = 5.8;
   const double column_length    = sp_out.bottom_position - at_rest_meniscus;

   if ( p.radial_resolution >= column_length )
   {
      error = QString( "radial resolution (%1) must be smaller than the "
                       "centerpiece column length (%2)" )
                 .arg( p.radial_resolution )
                 .arg( column_length );
      return false;
   }

   sp_out.mesh_radius.clear();
   sp_out.speed_step.clear();

   sp.duration_hours    = p.duration_hours;
   sp.duration_minutes  = p.duration_minutes;
   sp.rotorspeed        = qRound( p.rpm );
   sp.avg_speed         = p.rpm;
   sp.set_speed         = qRound( p.rpm );
   sp.scans             = p.scans;
   sp.acceleration      = qRound( p.acceleration );
   sp.acceleration_flag = true;

   if ( p.delay_hours >= 0 || p.delay_minutes >= 0.0 )
   { // Caller supplied a delay
      sp.delay_hours   = qMax( 0, p.delay_hours );
      sp.delay_minutes = qMax( 0.0, p.delay_minutes );
   }
   else
   { // Derive delay from stored acceleration, in hours and remaining minutes.
      double accel_minutes = p.rpm / ( sp.acceleration * 60.0 );
      sp.delay_hours       = (int) ( accel_minutes / 60.0 );
      sp.delay_minutes     = accel_minutes - ( sp.delay_hours * 60.0 );
   }

   sp_out.speed_step << sp;

   sp_out.simpoints         = p.simpoints;
   sp_out.radial_resolution = p.radial_resolution;
   sp_out.meshType          = p.meshType;
   sp_out.gridType          = p.gridType;
   // The simulation-parameter file contract stores at-rest radii. Consumers
   // add the calibration stretch for each speed step exactly once.
   sp_out.meniscus          = at_rest_meniscus;
   sp_out.bottom            = sp_out.bottom_position;
   sp_out.rnoise            = p.rnoise;
   sp_out.lrnoise           = p.lrnoise;
   sp_out.tinoise           = p.tinoise;
   sp_out.rinoise           = p.rinoise;
   sp_out.baseline          = p.baseline;
   sp_out.band_volume       = p.band_volume;
   sp_out.rotorCalID        = p.rotor_calibr;
   sp_out.band_forming      = p.band_forming;

   params_out = sp_out;
   error.clear();
   return true;
}

bool US_SimInputs::simParams( US_SimulationParameters& params_out, QString& error )
{
   return simParams( Params(), params_out, error );
}

US_Buffer US_SimInputs::buffer( const QString& guidSeed )
{
   US_Buffer buffer_out;
   buffer_out.description = "us3-sim-inputs generated water buffer v1";
   buffer_out.GUID        = US_SimRecord::guid( guidSeed, "buffer.default" );

   return buffer_out;
}

bool US_SimInputs::writeAll( const QString& dir, QString& error, const QString& guidSeed )
{
   QDir outdir( dir );
   if ( !outdir.exists() )
   {
      error = QString( "output directory %1 does not exist" ).arg( dir );
      return false;
   }

   // Construct both inputs before writing; hardware loading can fail.
   US_SimulationParameters params;
   if ( !simParams( params, error ) )
   {
      return false;
   }

   US_Model model;
   if ( !US_SimSpecies::model( model, error ) )
   {
      return false;
   }
   // Keyed apart from --emit-model's "model", so one seed never gives the
   // default model and a custom model the same identity.
   model.modelGUID = US_SimRecord::guid( guidSeed, "model.default" );

   if ( params.save_simparms( outdir.filePath( "sp_default.xml" ) ) != 0 )
   {
      error = "could not write sp_default.xml";
      return false;
   }

   if ( model.write( outdir.filePath( "model_default.xml" ) ) != IUS_DB2::OK )
   {
      error = "could not write model_default.xml";
      return false;
   }

   if ( !buffer( guidSeed ).writeToDisk( outdir.filePath( "buffer_default.xml" ) ) )
   {
      error = "could not write buffer_default.xml";
      return false;
   }

   error.clear();
   return true;
}
