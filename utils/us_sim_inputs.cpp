//! \file us_sim_inputs.cpp
#include "us_sim_inputs.h"
#include "us_astfem_math.h"
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

US_SimulationParameters US_SimInputs::simParams( const Params& p )
{
   US_SimulationParameters sp_out;
   US_SimulationParameters::SpeedProfile sp;

   sp_out.setHardware( NULL, p.rotor_calibr, p.centerpiece,
                       p.centerpiece_channel );
   double bottom        = US_AstfemMath::calc_bottom( p.rpm,
                                                      sp_out.bottom_position,
                                                      sp_out.rotorcoeffs );
   double menisc_curr   = 5.8 + bottom - sp_out.bottom_position;

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
      double accel_minutes = ( p.acceleration > 0.0 )
                             ? ( p.rpm / ( p.acceleration * 60.0 ) ) : 0.0;
      sp.delay_hours    = (int)( accel_minutes / 60.0 );
      sp.delay_minutes  = accel_minutes - ( sp.delay_hours * 60.0 );
   }

   sp_out.speed_step << sp;

   sp_out.simpoints         = p.simpoints;
   sp_out.radial_resolution = p.radial_resolution;
   sp_out.meshType          = p.meshType;
   sp_out.gridType          = p.gridType;
   sp_out.meniscus          = menisc_curr;
   sp_out.bottom            = bottom;
   sp_out.rnoise            = p.rnoise;
   sp_out.lrnoise           = p.lrnoise;
   sp_out.tinoise           = p.tinoise;
   sp_out.rinoise           = p.rinoise;
   sp_out.baseline          = p.baseline;
   sp_out.band_volume       = p.band_volume;
   sp_out.rotorCalID        = p.rotor_calibr;
   sp_out.band_forming      = p.band_forming;

   return sp_out;
}

US_Buffer US_SimInputs::buffer()
{
   US_Buffer buffer_out;
   buffer_out.description = "us3-sim-inputs generated water buffer v1";
   buffer_out.GUID         = US_Util::new_guid();

   return buffer_out;
}

bool US_SimInputs::writeAll( const QString& dir )
{
   QDir outdir( dir );
   if ( ! outdir.exists() )
      return false;

   if ( simParams().save_simparms( outdir.filePath( "sp_default.xml" ) ) != 0 )
      return false;

   if ( US_SimSpecies::model().write( outdir.filePath( "model_default.xml" ) )
        != IUS_DB2::OK )
      return false;

   if ( ! buffer().writeToDisk( outdir.filePath( "buffer_default.xml" ) ) )
      return false;

   return true;
}
