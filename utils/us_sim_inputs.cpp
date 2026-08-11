//! \file us_sim_inputs.cpp
#include "us_sim_inputs.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_util.h"
#include <QDir>

US_SimulationParameters US_SimInputs::simParams(
   double    rpm,
   int       duration_hours,
   double    duration_minutes,
   int       scans,
   double    acceleration,
   int       simpoints,
   double    radial_resolution,
   US_SimulationParameters::MeshType meshType,
   US_SimulationParameters::GridType gridType,
   double    rnoise,
   double    lrnoise,
   double    tinoise,
   double    rinoise,
   double    baseline,
   bool      band_forming,
   double    band_volume,
   QString   rotor_calibr,
   int       centerpiece,
   int       centerpiece_channel )
{
   US_SimulationParameters sp_out;
   US_SimulationParameters::SpeedProfile sp;

   sp_out.setHardware( NULL, rotor_calibr, centerpiece, centerpiece_channel );
   double bottom        = US_AstfemMath::calc_bottom( rpm, sp_out.bottom_position,
                                                        sp_out.rotorcoeffs );
   double menisc_curr   = 5.8 + bottom - sp_out.bottom_position;

   sp_out.mesh_radius.clear();
   sp_out.speed_step .clear();

   sp.duration_hours    = duration_hours;
   sp.duration_minutes  = duration_minutes;
   sp.delay_hours       = 0;
   sp.delay_minutes     = 20.0;
   sp.rotorspeed        = (int)rpm;
   sp.avg_speed         = rpm;
   sp.set_speed         = (int)rpm;
   sp.scans             = scans;
   sp.acceleration      = acceleration;
   sp.acceleration_flag = true;
   sp.delay_minutes     = (double)( sp.rotorspeed / ( 60.0 * sp.acceleration ) );
   sp_out.speed_step << sp;

   sp_out.simpoints         = simpoints;
   sp_out.radial_resolution = radial_resolution;
   sp_out.meshType          = meshType;
   sp_out.gridType          = gridType;
   sp_out.meniscus          = menisc_curr;
   sp_out.bottom            = bottom;
   sp_out.rnoise            = rnoise;
   sp_out.lrnoise           = lrnoise;
   sp_out.tinoise           = tinoise;
   sp_out.rinoise           = rinoise;
   sp_out.baseline          = baseline;
   sp_out.band_volume       = band_volume;
   sp_out.rotorCalID        = rotor_calibr;
   sp_out.band_forming      = band_forming;

   return sp_out;
}

QString US_SimInputs::validateCenterpiece( int centerpiece, int centerpiece_channel )
{
   QList< US_AbstractCenterpiece > cp_list;
   if ( ! US_AbstractCenterpiece::read_centerpieces( NULL, cp_list ) || cp_list.isEmpty() )
      return "no centerpiece definitions could be loaded";

   if ( centerpiece < 0 || centerpiece >= cp_list.size() )
      return QString( "centerpiece index %1 is out of range (0-%2)" )
         .arg( centerpiece ).arg( cp_list.size() - 1 );

   int channel_count = cp_list[ centerpiece ].channels;
   if ( centerpiece_channel < 0 || centerpiece_channel >= channel_count )
      return QString( "centerpiece-channel index %1 is out of range for "
         "centerpiece %2 (0-%3)" )
         .arg( centerpiece_channel ).arg( centerpiece ).arg( channel_count - 1 );

   return QString();
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
