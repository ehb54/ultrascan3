//! \file us_simparms.cpp

#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_settings.h"

US_SimulationParameters::US_SimulationParameters()
{
   mesh_radius.clear(); 
   speed_step.clear();

   speed_step << SpeedProfile();

   simpoints         = 200;  
   meshType          = ASTFEM;
   gridType          = MOVING;
   radial_resolution = 0.001;
   meniscus          = 5.8;
   bottom            = 7.2;
   rnoise            = 0.0;  
   tinoise           = 0.0;      
   rinoise           = 0.0;     
   rotorSerial       = "UTHSCSA 1001";      
   rotorType         = "Simulation";      
   band_forming      = false;   
   band_volume       = 0.0015;
   bottom_position   = 7.2;
   rotorcoeffs[ 0 ]  = 0.0;
   rotorcoeffs[ 1 ]  = 0.0;
   rotorcoeffs[ 2 ]  = 0.0;
   rotorcoeffs[ 3 ]  = 0.0;
   rotorcoeffs[ 4 ]  = 0.0;
}

US_SimulationParameters::SpeedProfile::SpeedProfile()
{
   duration_hours    = 0;
   duration_minutes  = 0;
   delay_hours       = 0;
   delay_minutes     = 0;
   scans             = 0;
   acceleration      = 400;
   rotorspeed        = 0;
   acceleration_flag = true;
}

// Set simulation parameter values from an experimental EditedData set.
void US_SimulationParameters::initFromData( US_DB2* db, US_DataIO2::EditedData& editdata )
{
   SpeedProfile sp;

   int    scanCount    = editdata.scanData.size();
   double time1        = editdata.scanData[ 0 ].seconds;
   double time2        = editdata.scanData[ 0 ].seconds;
   double rpm          = editdata.scanData[ 0 ].rpm;
   double rpmnext      = rpm;
   int    jj           = 0;

   rotorSerial         = "UTHSCSA 1002";
   rotorType           = "AN50";
   QString fn          = US_Settings::resultDir() + "/" + editdata.runID + "/"
                         + editdata.runID + "." + editdata.dataType + ".xml";
   QFile file( fn );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // if experiment/run file exists, get rotor serial from it
      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "rotor" )
         {  // pick up rotor serial from  <rotor ...serial="UTHSCSA 1002"...
            QXmlStreamAttributes a = xml.attributes();
            rotorSerial   = a.value( "serial" ).toString();
         }
      }

      file.close();
   }

   bottom_position     = 72.0;
   meniscus            = editdata.meniscus;

   speed_step.clear();

   for ( int ii = 1; ii < scanCount; ii++ )
   {  // loop to build speed steps where RPM changes
      rpmnext          = editdata.scanData[ ii ].rpm;

      if ( rpm != rpmnext )
      {  // rpm has changed, so need to create speed step for previous scans
         time2               = editdata.scanData[ ii - 1 ].seconds;
         sp.duration_hours   = (int)( time2 / 3600.0 );
         sp.duration_minutes = (int)( time2 / 60.0 )
                               - ( sp.duration_hours * 60 );
         sp.delay_hours      = (int)( time1 / 3600.0 );
         sp.delay_minutes    = ( time1 / 60.0 )
                               - ( (double)sp.delay_hours * 60.0 );
         sp.scans            = ii - jj;
         sp.rotorspeed       = (int)rpm;
         speed_step.append( sp );

         jj                  = ii;
         rpm                 = rpmnext;
         time1               = editdata.scanData[ ii     ].seconds;
      }
   }

   // set final (only?) speed step
   time2               = editdata.scanData[ scanCount - 1 ].seconds;
   sp.duration_hours   = (int)( time2 / 3600.0 );
   sp.duration_minutes = (int)( time2 / 60.0 ) - ( sp.duration_hours * 60 );
   sp.delay_hours      = (int)( time1 / 3600.0 );
   sp.delay_minutes    = ( time1 / 60.0 ) - ( (double)sp.delay_hours * 60.0 );
   sp.scans            = scanCount - jj;
   sp.rotorspeed       = (int)rpm;
   speed_step.append( sp );

   // set rotor coefficients, channel bottom position from hardware files
   setHardware( db, rotorSerial, 0, 0 );

   // calculate bottom using RPM, start bottom, and rotor coefficients
   bottom              = US_AstfemMath::calc_bottom( rpm, bottom_position,
                                                     rotorcoeffs );
}

// Set parameters from hardware files, related to rotor and centerpiece
void US_SimulationParameters::setHardware( US_DB2* db, QString serial,
      int cp, int ch )
{
   rotorSerial      = serial;
   bottom_position  = 72.0;

   QVector< US_Hardware::CenterpieceInfo > cp_list;
   QMap   < QString, QString             > rotor_map;
   rotor_map.clear();

   if ( US_Hardware::readCenterpieceInfo( cp_list ) )
      bottom_position     = cp_list[ cp ].bottom_position[ ch ];

   if ( US_Hardware::readRotorMap( db, rotor_map ) )
   {
      US_Hardware::rotorValues( serial, rotor_map, rotorType, rotorcoeffs );
//qDebug() << "setHardware: serial type coeff0" << serial << rotorType
//   << rotorcoeffs[0];
   }

   else
      qDebug() << "setHardware:readRotorMap *ERROR*";

   return;
}

// Set parameters from hardware files, related to rotor and centerpiece
void US_SimulationParameters::setHardware( QString serial, int cp, int ch )
{
   return setHardware( NULL, serial, cp, ch );
}

