//! \file us_sim_inputs.cpp
#include "us_sim_inputs.h"
#include "us_hardware.h"
#include "us_util.h"
#include "us_constants.h"
#include <QDir>
#include <QStringList>

US_SimInputs::Params::Params()
{
   // Share defaults with the underlying parameter types.
   US_SimulationParameters               sp_defaults;
   US_SimulationParameters::SpeedProfile step_defaults;

   // Match the run conditions constructed by us_astfem_sim.
   rpm                 = 45000.0;
   duration_hours      = 2;
   duration_minutes    = 30.0;
   scans               = 30;

   // Negative means "derive from the time needed to reach speed".
   delay_hours         = -1;
   delay_minutes       = -1.0;

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
   struct PositiveValue { const char* name; double value; bool allow_zero; };
   const PositiveValue positive[] = {
      { "speed",             p.rpm,               false },
      { "acceleration",      p.acceleration,      false },
      { "points",            (double)p.simpoints, false },
      { "radial resolution", p.radial_resolution, false },
      { "duration hours",    (double)p.duration_hours, true },
      { "duration minutes",  p.duration_minutes,       true },
      { "band volume",       p.band_volume,            true },
   };

   for ( const PositiveValue& check : positive )
   {
      if ( ! qIsFinite( check.value ) ||
           ( check.allow_zero ? check.value < 0.0 : check.value <= 0.0 ) )
         return QString( "%1 must be finite and %2 (got %3)" )
            .arg( check.name )
            .arg( check.allow_zero ? "zero or greater" : "greater than zero" )
            .arg( check.value );
   }

   if ( p.scans < 1 )
      return QString( "scans must be at least 1 (got %1)" ).arg( p.scans );

   if ( p.duration_hours == 0 && p.duration_minutes <= 0.0 )
      return "run duration must be greater than zero";

   // The speed profile stores acceleration as a whole number of rpm/s, and a
   // rounded value of zero makes the acceleration ramp infinitely long.
   if ( qRound( p.acceleration ) < 1 )
      return QString( "acceleration must be at least 1 rpm/s (got %1)" )
         .arg( p.acceleration );

   const double finite_values[] = { p.delay_minutes, p.rnoise, p.lrnoise,
      p.tinoise, p.rinoise, p.baseline };
   for ( double value : finite_values )
      if ( ! qIsFinite( value ) )
         return "delay, noise, and baseline values must be finite";

   // Scans are spread over the time left after the delay, so a delay that
   // reaches the end of the run leaves no time to scan in.
   const double duration_min = p.duration_hours * 60.0 + p.duration_minutes;
   const double delay_min    = ( p.delay_hours >= 0 || p.delay_minutes >= 0.0 )
      ? qMax( 0, p.delay_hours ) * 60.0 + qMax( 0.0, p.delay_minutes )
      : p.rpm / ( qRound( p.acceleration ) * 60.0 );

   if ( delay_min >= duration_min )
      return QString( "delay (%1 min) must be shorter than the run duration"
                      " (%2 min)" ).arg( delay_min ).arg( duration_min );

   if ( p.meshType < US_SimulationParameters::ASTFEM ||
        p.meshType > US_SimulationParameters::ASTFVM )
      return "mesh type is out of range";
   if ( p.gridType < US_SimulationParameters::FIXED ||
        p.gridType > US_SimulationParameters::MOVING )
      return "grid type is out of range";

   QString hardware_error = US_AbstractCenterpiece::validate(
      p.centerpiece, p.centerpiece_channel );
   if ( ! hardware_error.isEmpty() )
      return hardware_error;

   QMap< QString, QString > rotor_map;
   bool loaded_rotors = US_Hardware::readRotorMap( rotor_map );
   if ( p.rotor_calibr != "0" &&
        ( ! loaded_rotors || ! rotor_map.contains( p.rotor_calibr ) ) )
      return QString( "rotor calibration ID %1 was not found" )
         .arg( p.rotor_calibr );

   return QString();
}

bool US_SimInputs::simParams( const Params& p,
                              US_SimulationParameters& params_out,
                              QString& error )
{
   error = validateParams( p );
   if ( ! error.isEmpty() )
      return false;

   US_SimulationParameters sp_out;
   US_SimulationParameters::SpeedProfile sp;

   if ( ! sp_out.setHardware( NULL, p.rotor_calibr, p.centerpiece,
                              p.centerpiece_channel ) )
   {
      error = "hardware definitions changed or could not be loaded";
      return false;
   }

   sp_out.mesh_radius.clear();
   sp_out.speed_step .clear();

   sp.duration_hours    = p.duration_hours;
   sp.duration_minutes  = p.duration_minutes;
   sp.rotorspeed        = qRound( p.rpm );
   sp.avg_speed         = p.rpm;
   sp.set_speed         = qRound( p.rpm );
   sp.scans             = p.scans;
   sp.acceleration      = qRound( p.acceleration );
   sp.acceleration_flag = true;

   if ( p.delay_hours >= 0  ||  p.delay_minutes >= 0.0 )
   {  // Caller supplied a delay
      sp.delay_hours    = qMax( 0,   p.delay_hours   );
      sp.delay_minutes  = qMax( 0.0, p.delay_minutes );
   }
   else
   {  // Store the derived delay as whole hours plus 0-59 minutes.
      // Derive from the stored acceleration so the delay matches the ramp a
      // consumer actually runs.  Validation guarantees it is at least 1.
      double accel_minutes = p.rpm / ( sp.acceleration * 60.0 );
      sp.delay_hours    = (int)( accel_minutes / 60.0 );
      sp.delay_minutes  = accel_minutes - ( sp.delay_hours * 60.0 );
   }

   sp_out.speed_step << sp;

   sp_out.simpoints         = p.simpoints;
   sp_out.radial_resolution = p.radial_resolution;
   sp_out.meshType          = p.meshType;
   sp_out.gridType          = p.gridType;
   // The simulation-parameter file contract stores at-rest radii. Consumers
   // add the calibration stretch for each speed step exactly once.
   sp_out.meniscus          = 5.8;
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

bool US_SimInputs::simParams( US_SimulationParameters& params_out,
                              QString& error )
{
   return simParams( Params(), params_out, error );
}

US_Buffer US_SimInputs::buffer()
{
   US_Buffer buffer_out;
   buffer_out.description = "us3-sim-inputs generated water buffer v1";
   buffer_out.GUID         = US_Util::new_guid();

   return buffer_out;
}

bool US_SimInputs::writeAll( const QString& dir, QString& error )
{
   QDir outdir( dir );
   if ( ! outdir.exists() )
   {
      error = QString( "output directory %1 does not exist" ).arg( dir );
      return false;
   }

   // Build both inputs before writing: construction reads hardware
   // definitions from disk and can fail, leaving an unusable run on disk.
   US_SimulationParameters params;
   if ( ! simParams( params, error ) )
      return false;

   US_Model model;
   if ( ! US_SimSpecies::model( model, error ) )
      return false;

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

   if ( ! buffer().writeToDisk( outdir.filePath( "buffer_default.xml" ) ) )
   {
      error = "could not write buffer_default.xml";
      return false;
   }

   error.clear();
   return true;
}
