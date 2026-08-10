//! \file us_sim_inputs.cpp
#include "us_sim_inputs.h"
#include "us_astfem_math.h"
#include "us_util.h"
#include <QDir>

US_SimulationParameters US_SimInputs::simParams()
{
   US_SimulationParameters sp_out;
   US_SimulationParameters::SpeedProfile sp;
   QString rotor_calibr = "0";
   double  rpm           = 45000.0;

   sp_out.setHardware( NULL, rotor_calibr, 0, 0 );
   double bottom        = US_AstfemMath::calc_bottom( rpm, sp_out.bottom_position,
                                                        sp_out.rotorcoeffs );
   double menisc_curr   = 5.8 + bottom - sp_out.bottom_position;

   sp_out.mesh_radius.clear();
   sp_out.speed_step .clear();

   sp.duration_hours    = 2;
   sp.duration_minutes  = 30.0;
   sp.delay_hours       = 0;
   sp.delay_minutes     = 20.0;
   sp.rotorspeed        = (int)rpm;
   sp.avg_speed         = rpm;
   sp.set_speed         = (int)rpm;
   sp.scans             = 30;
   sp.acceleration      = 400;
   sp.acceleration_flag = true;
   sp.delay_minutes     = (double)( sp.rotorspeed / ( 60.0 * sp.acceleration ) );
   sp_out.speed_step << sp;

   sp_out.simpoints         = 200;
   sp_out.radial_resolution = 0.001;
   sp_out.meshType          = US_SimulationParameters::ASTFEM;
   sp_out.gridType          = US_SimulationParameters::MOVING;
   sp_out.meniscus          = menisc_curr;
   sp_out.bottom            = bottom;
   sp_out.rnoise            = 0.0;
   sp_out.lrnoise           = 0.0;
   sp_out.tinoise           = 0.0;
   sp_out.rinoise           = 0.0;
   sp_out.baseline          = 0.0;
   sp_out.band_volume       = 0.015;
   sp_out.rotorCalID        = rotor_calibr;
   sp_out.band_forming      = false;

   return sp_out;
}

US_Model US_SimInputs::model()
{
   US_Model model_out;
   model_out.description = "us3-sim-inputs generated protein model v1";
   model_out.modelGUID    = US_Util::new_guid();
   model_out.optics       = US_Model::ABSORBANCE;
   model_out.analysis     = US_Model::MANUAL;
   model_out.components << US_Model::SimulationComponent();
   model_out.update_coefficients();

   return model_out;
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

   if ( model().write( outdir.filePath( "model_default.xml" ) ) != IUS_DB2::OK )
      return false;

   if ( ! buffer().writeToDisk( outdir.filePath( "buffer_default.xml" ) ) )
      return false;

   return true;
}
