//! \file us_simparms.cpp

#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"

//!< level-conditioned debug print
#define DbgLv(a) if(dbg_level>=a)qDebug()
#define DSS_RESO   100   // default SetSpeedResolution
#define DSS_LO_RPM 1500  // default SetSpeedLowRpm
#define DSS_LO_SEC 20    // default SpeedStepLowSec

US_SimulationParameters::US_SimulationParameters()
{
   mesh_radius.clear();
   speed_step.clear();

   speed_step << SpeedProfile();
   sim_speed_prof << SimSpeedProf();
   simpoints         = 200;
   meshType          = ASTFEM;
   gridType          = MOVING;
   radial_resolution = 0.001;
   meniscus          = 5.8;
   bottom            = 7.2;
   rnoise            = 0.0;
   lrnoise           = 0.0;
   tinoise           = 0.0;
   rinoise           = 0.0;
   temperature       = NORMAL_TEMP;
   rotorCalID        = "0";
   band_forming      = false;
   band_volume       = 0.015;
   bottom_position   = 7.2;
   rotorcoeffs[ 0 ]  = 0.0;
   rotorcoeffs[ 1 ]  = 0.0;
   cp_sector         = 0;
   cp_pathlen        = 1.2;
   cp_angle          = 2.5;
   cp_width          = 0.0;
   sim               = false ;
   tsobj             = NULL ;
   firstScanIsConcentration = false;
}

US_SimulationParameters::SpeedProfile::SpeedProfile()
{
   duration_hours    = 0;
   duration_minutes  = 0.0;
   delay_hours       = 0;
   delay_minutes     = 0.0;
   scans             = 0;
   rotorspeed        = 0;
   acceleration      = 400;
   acceleration_flag = true;
   w2t_first         = 0.0;
   w2t_last          = 0.0;
   time_first        = 0;
   time_last         = 0;

   avg_speed         = 0.0;
   speed_stddev      = 0.0;
   set_speed         = 0;
}

// Constructor for simulation speed profile
US_SimulationParameters::SimSpeedProf::SimSpeedProf()
{
   acceleration      = 400.0;

   rpm_timestate.clear() ;
   w2t_timestate.clear() ;

   w2t_b_accel       = 0.0;
   w2t_e_accel       = 0.0;
   w2t_e_step        = 0.0;
   avg_speed         = 0.0;
   rotorspeed        = 0;
   duration          = 0;
   time_b_accel      = 0;
   time_e_accel      = 0;
   time_f_scan       = 0;
   time_l_scan       = 0;
  // sim               = false ;
}

// Set simulation parameter values from an experimental RawData set.
void US_SimulationParameters::initFromData( US_DB2* db,
   US_DataIO::RawData& rawdata, bool incl_speed, QString runID, QString dataType )
{
   SpeedProfile sp;

   int     dbg_level   = US_Settings::us_debug();
   int     cp_id       = 0;
   QString channel     = QChar(rawdata.channel);
   int     iechan      = QString( "ABCDEFGH" ).indexOf( channel );
   int     ch          = qMax( 0, iechan ) / 2;
           iechan      = qMax( 0, iechan ) + 1;
   QString ecell       = QString::number(rawdata.cell);
   int     iecell      = ecell.toInt();

DbgLv(1) << "SP:iFD: cell chan ch" << ecell << channel << ch
 << rawdata.channel << (ch+1);

   rotorCalID          = "0";
   QString fn          = US_Settings::resultDir() + "/" + runID + "/"
                         + runID + "." + dataType + ".xml";
   QFile file( fn );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // If experiment/run file exists, get calibration,centerpiece IDs from it
      QXmlStreamReader xml( &file );
      int dcp_id       = 0;
      int dcell        = 0;
      QString dchan    = "";

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "calibration" )
         {  // Pick up rotor calibration ID from  <calibration ... id=...
            QXmlStreamAttributes a = xml.attributes();
            rotorCalID       = a.value( "id"      ).toString();
            rotorcoeffs[ 0 ] = a.value( "coeff1"  ).toString().toDouble();
            rotorcoeffs[ 1 ] = a.value( "coeff2"  ).toString().toDouble();
         }

         if ( xml.isStartElement()  &&  xml.name() == "dataset" )
         {  // Pick up cell and channel for comparison
            QXmlStreamAttributes a = xml.attributes();
            dcell            = a.value( "cell"    ).toString().toInt();
            dchan            = a.value( "channel" ).toString();
         }

         if ( xml.isStartElement()  &&  xml.name() == "centerpiece" )
         {  // Pick up centerpiece ID from  <centerpiece ... id=...
            QXmlStreamAttributes a = xml.attributes();
            dcp_id           = a.value( "id"      ).toString().toInt();
DbgLv(1) << "SP:iFD:  dcell dchan" << dcell << dchan;
            if ( dcell == iecell  &&  dchan == channel )
            { // If cell,channel match edit, pick up CpID
               cp_id            = dcp_id;
DbgLv(1) << "SP:iFD:    cp_id" << cp_id;
            }
         }
      }

      file.close();

      if ( cp_id == 0 )    // If no cell,chan match; use last CP ID
         cp_id            = dcp_id;
   }
//DbgLv(2) << "SP:iFD: cp_id ch" << cp_id << ch;

   bottom_position     = 7.2;

   if ( incl_speed )
      computeSpeedSteps( &rawdata.scanData, speed_step );

#ifndef NO_DB
   double  rpm         = rawdata.scanData[ 0 ].rpm;
   if ( db != NULL )
   {  // If reading from the database, get rotor,centerpiece info from DB
      int         stat_db = 0;
      bool        ok_db;
      QString     expID;
      QStringList query;
      QString     rcalIDsv = rotorCalID;  // Save IDs gotten from local file
      int         cpIDsv   = cp_id;
//DbgLv(2) << "Sim parms:runID" << runID;
//DbgLv(2) << "Sim parms:invID" << US_Settings::us_inv_ID();
      query << "get_experiment_info_by_runID"
            << runID
            << QString::number( US_Settings::us_inv_ID() );
      db->query( query );
      stat_db = db->lastErrno();
//DbgLv(2) << "Sim parms:query() stat" << stat_db;
      if ( stat_db != US_DB2::NOROWS )
      {  // Info by runID:  experiment and calibration IDs
         ok_db      = db->next();
//DbgLv(2) << "Sim parms: next() ok_db" << ok_db;
         if ( ok_db )
         {
            expID      = db->value( 1 ).toString();
            rotorCalID = db->value( 7 ).toString();
//DbgLv(2) << "Sim parms: expID" << expID;
//DbgLv(2) << "Sim parms: rotorCalID" << rotorCalID << "sv" << rcalIDsv;
         }
         else
            rotorCalID = "";

         if ( rotorCalID.isEmpty()  &&  ! expID.isEmpty() )
         {  // If still no calibration ID, try it another way
            query.clear();
            query << "get_experiment_info" << expID;
            db->query( query );
            stat_db = db->lastErrno();
            if ( stat_db != US_DB2::NOROWS  &&  db->next() )
            {
               rotorCalID = db->value( 7 ).toString();
//DbgLv(2) << "Sim parms(2):     rotorCalID" << rotorCalID;
            }
         }

         // If unable to get calibration ID from DB, fall back to local info
         if ( rotorCalID.isEmpty() || rotorCalID == "0" )
            rotorCalID = rcalIDsv;
//DbgLv(2) << "Sim parms(3):     rotorCalID" << rotorCalID;

         if ( ! expID.isEmpty() )
         {  // Get centerpiece ID from cell records for this experiment
            query.clear();
            query << "all_cell_experiments" << expID;
            db->query( query );
            while ( db->next() )
            {
               int cell     = db->value( 2 ).toInt();
               int ichan    = db->value( 3 ).toInt();
               int cellCpId = db->value( 4 ).toInt();

DbgLv(1) << "Sim parms: cell iecell ichan iechan" << cell << iecell
   << ichan << iechan << "ccId" << cellCpId;
               if ( cellCpId > 0  &&  cell == iecell  &&  ichan == iechan )
               { // Valid CpID with cell,channel matching edit
                  cp_id        = cellCpId;
                  break;
               }
            }
         }

DbgLv(1) << "Sim parms:        cp_id" << cp_id << "sv" << cpIDsv;
         // If no centerpiece ID from DB, fall back to local info
         if ( cp_id < 1 )
            cp_id     = cpIDsv;
//DbgLv(2) << "Sim parms(2):     cp_id" << cp_id;
      }
   }

   // Set rotor coefficients, channel bottom position from hardware files
   setHardware( db, rotorCalID, -cp_id, ch );

   // Calculate bottom using RPM, start bottom, and rotor coefficients
   bottom = US_AstfemMath::calc_bottom( rpm, bottom_position, rotorcoeffs );
#else
   // For NO_DB (back end) the bottom needs to be set after this function
   bottom = bottom_position;
   db     = NULL; // Stop compiler warning
DbgLv(2) << "SP:iFD: db" << db;
#endif
DbgLv(2) << "SP:iFD: bottom" << bottom;
}

// Set simulation parameter values from an experimental EditedData set.
void US_SimulationParameters::initFromData( US_DB2* db,
   US_DataIO::EditedData& editdata, bool incl_speed )
{
   SpeedProfile sp;

   int     dbg_level   = US_Settings::us_debug();
   int     cp_id       = 0;
   QString channel     = editdata.channel;
   int     iechan      = QString( "ABCDEFGH" ).indexOf( channel );
   int     ch          = qMax( 0, iechan ) / 2;
           iechan      = qMax( 0, iechan ) + 1;
   QString ecell       = editdata.cell;
   int     iecell      = ecell.toInt();

DbgLv(1) << "SP:iFD: cell chan ch" << ecell << channel << ch
 << editdata.channel << (ch+1);

   rotorCalID          = "0";
   QString fn          = US_Settings::resultDir() + "/" + editdata.runID + "/"
                         + editdata.runID + "." + editdata.dataType + ".xml";
   QFile file( fn );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // If experiment/run file exists, get calibration,centerpiece IDs from it
      QXmlStreamReader xml( &file );
      int dcp_id       = 0;
      int dcell        = 0;
      QString dchan    = "";

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "calibration" )
         {  // Pick up rotor calibration ID from  <calibration ... id=...
            QXmlStreamAttributes a = xml.attributes();
            rotorCalID       = a.value( "id"      ).toString();
            rotorcoeffs[ 0 ] = a.value( "coeff1"  ).toString().toDouble();
            rotorcoeffs[ 1 ] = a.value( "coeff2"  ).toString().toDouble();
         }

         if ( xml.isStartElement()  &&  xml.name() == "dataset" )
         {  // Pick up cell and channel for comparison
            QXmlStreamAttributes a = xml.attributes();
            dcell            = a.value( "cell"    ).toString().toInt();
            dchan            = a.value( "channel" ).toString();
         }

         if ( xml.isStartElement()  &&  xml.name() == "centerpiece" )
         {  // Pick up centerpiece ID from  <centerpiece ... id=...
            QXmlStreamAttributes a = xml.attributes();
            dcp_id           = a.value( "id"      ).toString().toInt();
DbgLv(1) << "SP:iFD:  dcell dchan" << dcell << dchan;
            if ( dcell == iecell  &&  dchan == channel )
            { // If cell,channel match edit, pick up CpID
               cp_id            = dcp_id;
// x  x  x  x  x  x  x  x
DbgLv(1) << "SP:iFD:    cp_id" << cp_id;
            }
         }
      }

      file.close();

      if ( cp_id == 0 )    // If no cell,chan match; use last CP ID
         cp_id            = dcp_id;
   }
//DbgLv(2) << "SP:iFD: cp_id ch" << cp_id << ch;

   bottom_position     = 7.2;
   meniscus            = editdata.meniscus;

   if ( incl_speed )
      computeSpeedSteps( &editdata.scanData, speed_step );

#ifndef NO_DB
   double  rpm         = editdata.scanData[ 0 ].rpm;
   if ( db != NULL )
   {  // If reading from the database, get rotor,centerpiece info from DB
      int         stat_db = 0;
      bool        ok_db;
      QString     expID;
      QStringList query;
      QString     rcalIDsv = rotorCalID;  // Save IDs gotten from local file
      int         cpIDsv   = cp_id;
//DbgLv(2) << "Sim parms:runID" << editdata.runID;
//DbgLv(2) << "Sim parms:invID" << US_Settings::us_inv_ID();
      query << "get_experiment_info_by_runID"
            << editdata.runID
            << QString::number( US_Settings::us_inv_ID() );
      db->query( query );
      stat_db = db->lastErrno();
//DbgLv(2) << "Sim parms:query() stat" << stat_db;
      if ( stat_db != US_DB2::NOROWS )
      {  // Info by runID:  experiment and calibration IDs
         ok_db      = db->next();
//DbgLv(2) << "Sim parms: next() ok_db" << ok_db;
         if ( ok_db )
         {
            expID      = db->value( 1 ).toString();
            rotorCalID = db->value( 7 ).toString();
//DbgLv(2) << "Sim parms: expID" << expID;
//DbgLv(2) << "Sim parms: rotorCalID" << rotorCalID << "sv" << rcalIDsv;
         }
         else
            rotorCalID = "";

         if ( rotorCalID.isEmpty()  &&  ! expID.isEmpty() )
         {  // If still no calibration ID, try it another way
            query.clear();
            query << "get_experiment_info" << expID;
            db->query( query );
            stat_db = db->lastErrno();
            if ( stat_db != US_DB2::NOROWS  &&  db->next() )
            {
               rotorCalID = db->value( 7 ).toString();
//DbgLv(2) << "Sim parms(2):     rotorCalID" << rotorCalID;
            }
         }

         // If unable to get calibration ID from DB, fall back to local info
         if ( rotorCalID.isEmpty() || rotorCalID == "0" )
            rotorCalID = rcalIDsv;
//DbgLv(2) << "Sim parms(3):     rotorCalID" << rotorCalID;

         if ( ! expID.isEmpty() )
         {  // Get centerpiece ID from cell records for this experiment
            query.clear();
            query << "all_cell_experiments" << expID;
            db->query( query );
            while ( db->next() )
            {
               int cell     = db->value( 2 ).toInt();
               int ichan    = db->value( 3 ).toInt();
               int cellCpId = db->value( 4 ).toInt();

DbgLv(1) << "Sim parms: cell iecell ichan iechan" << cell << iecell
   << ichan << iechan << "ccId" << cellCpId;
               if ( cellCpId > 0  &&  cell == iecell  &&  ichan == iechan )
               { // Valid CpID with cell,channel matching edit
                  cp_id        = cellCpId;
                  break;
               }
            }
         }

DbgLv(1) << "Sim parms:        cp_id" << cp_id << "sv" << cpIDsv;
         // If no centerpiece ID from DB, fall back to local info
         if ( cp_id < 1 )
            cp_id     = cpIDsv;
//DbgLv(2) << "Sim parms(2):     cp_id" << cp_id;
      }
   }

   // Set rotor coefficients, channel bottom position from hardware files
   setHardware( db, rotorCalID, -cp_id, ch );

   // Calculate bottom using RPM, start bottom, and rotor coefficients
   bottom    = US_AstfemMath::calc_bottom( rpm, bottom_position, rotorcoeffs );
#else
   // For NO_DB (back end) the bottom needs to be set after this function
   bottom    = bottom_position;
   db        = NULL; // Stop compiler warning
DbgLv(2) << "SP:iFD: db" << db;
#endif
DbgLv(1) << "SP:iFD: bottom" << bottom;

   // If EditedData has bottom given, use that
   bottom    = ( editdata.bottom > 0.0 ) ? editdata.bottom : bottom;
DbgLv(1) << "SP:iFD: bottom" << bottom << "bottom_ed"
 << editdata.bottom;

//-------------------------------------------

}

// Read the speed steps vector from runID file
int US_SimulationParameters::readSpeedSteps( QString runID, QString dataType,
      QVector< SpeedProfile >& speedsteps )
{
   speedsteps.clear();
   SpeedProfile sp;
   int     nsteps      = 0;
   int     dbg_level   = US_Settings::us_debug();
   QString fn          = US_Settings::resultDir() + "/" + runID + "/"
                         + runID + "." + dataType + ".xml";
   QFile filei( fn );

   if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // If experiment/run file exists, get speed steps from it
DbgLv(1) << "SP:rSS: fn" << fn;
      QXmlStreamReader xml( &filei );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "speedstep" )
         {
            speedstepFromXml( xml, sp );

            speedsteps << sp;
            nsteps++;
DbgLv(1) << "SP:rSS:   step" << nsteps << "rotorspeed" << sp.rotorspeed
 << "avg_speed" << sp.avg_speed;
         }
      }

      filei.close();
   }

   return nsteps;
}

// Compute the speed steps vector from data scans
void US_SimulationParameters::computeSpeedSteps(
      QVector< US_DataIO::Scan >* scans,
      QVector< SpeedProfile >& speedsteps )
{
   SpeedProfile sp;
   speedsteps.clear();
   int     dbg_level   = US_Settings::us_debug();
   int     scanCount   = (*scans).size();
   double  time1       = (*scans)[ 0 ].seconds;
   double  time2       = 0.0;
   double  w2t1        = (*scans)[ 0 ].omega2t;
   double  w2t2        = 0.0;
   double  delay_secs  = time1;
   double  rpm         = (*scans)[ 0 ].rpm;
   double  rpmnext     = rpm;
   double  step_secs   = 0.0;
   int     lscx        = 0;
   double  rpm_sum     = rpm;
   int ss_reso         = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso       = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
DbgLv(1) << "SP:cSS: ii ss_reso" << ii << ss_reso << dbgtxt[ii];
   }
DbgLv(1) << "SP:cSS: scan" << 1 << "rpm time omega2t"
 << rpm << qRound(time1) << (*scans)[ 0 ].omega2t << "ss_reso" << ss_reso;

   for ( int ii = 1; ii < scanCount; ii++ )
   {  // Loop to build speed steps where RPM changes
      rpm              = rpmnext;
      rpmnext          = (*scans)[ ii ].rpm;
      // Get set_speeds, the speeds rounded to nearest 100 (or other resolution)
      int ss_next      = (int)qRound( rpmnext / (double)ss_reso ) * ss_reso;
      int ss_curr      = (int)qRound( rpm     / (double)ss_reso ) * ss_reso;
DbgLv(1) << "SP:cSS: scan" << (ii+1) << "rpm srpm time omega2t"
 << rpmnext << ss_next << qRound((*scans)[ii].seconds) << (*scans)[ii].omega2t;

      if ( ss_curr != ss_next )
      {  // RPM has changed, so need to create speed step for previous scans
         time2               = (*scans)[ ii - 1 ].seconds;
         w2t2                = (*scans)[ ii - 1 ].omega2t;
         step_secs           = time2 - time1 + delay_secs;
         sp.duration_hours   = (int)( step_secs / 3600.0 );
         sp.duration_minutes = ( step_secs / 60.0 )
                               - ( (double)sp.duration_hours * 60.0 );
         sp.delay_hours      = (int)( delay_secs / 3600.0 );
         sp.delay_minutes    = ( delay_secs / 60.0 )
                               - ( (double)sp.delay_hours * 60.0 );
         sp.scans            = ii - lscx;
         sp.w2t_first        = w2t1;
         sp.w2t_last         = w2t2;
         sp.time_first       = qRound( time1 );
         sp.time_last        = qRound( time2 );
         sp.avg_speed        = rpm_sum / (double)sp.scans;
         sp.rotorspeed       = qRound( sp.avg_speed );
         sp.set_speed        = ( ( sp.rotorspeed + 50 ) / 100 ) * 100;
         sp.speed_stddev     = 0.0;
         for ( int jj = lscx; jj < ii; jj++ )
            sp.speed_stddev    += sq( (*scans)[ jj ].rpm - sp.avg_speed );
         sp.speed_stddev     = sqrt( sp.speed_stddev ) / (double)sp.scans;

         speedsteps << sp;
DbgLv(1) << "SP:cSS:   speedsteps" << speedsteps.size() << "scans" << sp.scans
 << "duration h m" << sp.duration_hours << sp.duration_minutes
 << "delay h m" << sp.delay_hours << sp.delay_minutes << "rpm" << rpm;
DbgLv(1) << "SP:cSS:      w2t1 w2t2 time1 time2" << sp.w2t_first << sp.w2t_last
 << sp.time_first << sp.time_last;
DbgLv(1) << "SP:cSS:       sp set avg sdev" << sp.set_speed << sp.avg_speed
 << sp.speed_stddev;

         lscx                = ii;
         rpm                 = rpmnext;
         time1               = (*scans)[ ii ].seconds;
         w2t1                = (*scans)[ ii ].omega2t;
         delay_secs          = time1 - time2;
         rpm_sum             = 0.0;
      }

      rpm_sum            += rpm;
   }

   // Set final (only?) speed step
   time2               = (*scans)[ scanCount - 1 ].seconds;
   w2t2                = (*scans)[ scanCount - 1 ].omega2t;
   step_secs           = time2 - time1 + delay_secs;
   sp.duration_hours   = (int)( step_secs / 3600.0 );
   sp.duration_minutes = ( step_secs / 60.0 )
                         - ( (double)sp.duration_hours * 60.0 );
   sp.delay_hours      = (int)( delay_secs / 3600.0 );
   sp.delay_minutes    = ( delay_secs / 60.0 )
                         - ( (double)sp.delay_hours * 60.0 );
   sp.scans            = scanCount - lscx;
   sp.w2t_first        = w2t1;
   sp.w2t_last         = w2t2;
   sp.time_first       = qRound( time1 );
   sp.time_last        = qRound( time2 );
   sp.avg_speed        = rpm_sum / (double)sp.scans;
   sp.rotorspeed       = qRound( sp.avg_speed );
   sp.set_speed        = ( ( sp.rotorspeed + 50 ) / 100 ) * 100;
   sp.speed_stddev     = 0.0;
   for ( int jj = lscx; jj < scanCount; jj++ )
      sp.speed_stddev    += sq( (*scans)[ jj ].rpm - sp.avg_speed );
   sp.speed_stddev     = sqrt( sp.speed_stddev ) / (double)sp.scans;

   speedsteps << sp;
DbgLv(1) << "SP:cSS:   speedsteps" << speedsteps.size() << "scans" << sp.scans
 << "duration h m" << sp.duration_hours << sp.duration_minutes
 << "delay h m" << sp.delay_hours << sp.delay_minutes << "rpm" << rpm;
DbgLv(1) << "SP:cSS:      w2t1 w2t2 time1 time2" << sp.w2t_first << sp.w2t_last
 << sp.time_first << sp.time_last;
DbgLv(1) << "SP:cSS:       sp set avg sdev" << sp.set_speed << sp.avg_speed
 << sp.speed_stddev;
}

// Compute the experiment speed steps vector from all data scans
void US_SimulationParameters::computeSpeedSteps(
      QVector< US_DataIO::RawData >& allrData,
      QVector< SpeedProfile >& speedsteps )
{
   if ( allrData.count() == 1 )
   {  // For a single triple, compute the speed steps from it
      computeSpeedSteps( &allrData[ 0 ].scanData, speedsteps );
      return;
   }
   QVector<US_DataIO::Scan> scans;
   QVector<double> times;

   for ( int ii = 0; ii < allrData.count(); ii++ ) {
      for ( int jj = 0; jj < allrData[ii].scanCount(); jj++ ) {
         if ( !times.contains(allrData[ii].scanData[jj].seconds) ) {
            scans << allrData[ii].scanData[jj];
            times << allrData[ii].scanData[jj].seconds;
         }
      }
   }
   times.clear();

   struct SortBySeconds {
      bool operator()(const US_DataIO::Scan& a, const US_DataIO::Scan& b) const {
         return a.seconds < b.seconds;
      }
   };
   // Sort the scans by their respective attribute seconds
   std::sort(scans.begin(), scans.end(), SortBySeconds());

   // Compute time steps for two triples at the extreme
   computeSpeedSteps( &scans, speedsteps );
}

// Set parameters from hardware files, related to rotor and centerpiece
void US_SimulationParameters::setHardware( US_DB2* db, QString rCalID,
      int cp, int ch )
{
   int dbg_level    = US_Settings::us_debug();
   rotorCalID       = rCalID;
   bottom_position  = 7.2;
//DbgLv(2) << "sH: cp ch rCalID" << cp << ch << rCalID;

   QList< US_AbstractCenterpiece > cp_list;
   QMap < QString, QString       > rotor_map;
   rotor_map.clear();

   if ( US_AbstractCenterpiece::read_centerpieces( db, cp_list ) )
   {
      if ( cp < 0 )
      {  // If cp given is negative, this means look for a serial number
         int cp_id = -cp;
             cp    = 0;

         for ( int jj = 0; jj < cp_list.size(); jj++ )
         {
            if ( cp_id == cp_list[ jj ].serial_number )
            {  // Replace cp value (serial) with an index in the list
               cp   = jj;
               break;
            }
         }
DbgLv(1) << "sH: cp ch cp_id" << cp << ch << cp_id;
      }

      // Pick up centerpiece info by Centerpiece and Channel indecies
      QStringList shapes;
      shapes << "sector" << "standard" << "rectangular" << "band forming"
             << "meniscus matching" << "circular" << "synthetic";
      QString shape   = cp_list[ cp ].shape;
      bottom_position = cp_list[ cp ].bottom_position[ ch ];
      cp_pathlen      = cp_list[ cp ].path_length    [ ch ];
      cp_angle        = cp_list[ cp ].angle;
      cp_width        = cp_list[ cp ].width;
      cp_sector       = qMax( 0, shapes.indexOf( shape ) );
      band_forming    = ( shape == "band forming" );

   }

   if ( US_Hardware::readRotorMap( db, rotor_map ) )
   {  // Get rotor coefficients by matching calibration ID
      US_Hardware::rotorValues( rotorCalID, rotor_map, rotorcoeffs );
   }

   else
      qDebug() << "setHardware:readRotorMap *ERROR*";

   return;
}

// Set parameters from hardware files, related to rotor and centerpiece (Local)
void US_SimulationParameters::setHardware( QString rCalID, int cp, int ch )
{
   return setHardware( NULL, rCalID, cp, ch );
}

// Load simulation parameters from file
int US_SimulationParameters::load_simparms( QString fname )
{
   int stat = 0;
   int kr   = 0;
   QStringList meshlist;
   QStringList gridlist;
   meshlist << "ASTFEM" << "Claverie" << "MovingHat" << "User" << "ASTFVM";
   gridlist << "Fixed" <<  "Moving";
   US_SimulationParameters::SpeedProfile sp;

   QFile xfile( fname );

   if ( xfile.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &xfile );
      QXmlStreamAttributes a;
      QString astr;
      int     kk;
      speed_step.clear();

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "params" )
         {
            a     = xml.attributes();

            astr  = a.value( "meshType" ).toString();
            if ( !astr.isEmpty() )
            {
               kk        = meshlist.indexOf( astr );
               meshType  = (US_SimulationParameters::MeshType)kk;
            }
            astr      = a.value( "gridType" ).toString();
            if ( !astr.isEmpty() )
            {
               kk        = gridlist.indexOf( astr );
               gridType  = (US_SimulationParameters::GridType)kk;
            }
            astr  = a.value( "simpoints"   ).toString();
            if ( !astr.isEmpty() )
               simpoints    = astr.toInt();
            astr  = a.value( "radialres"   ).toString();
            if ( !astr.isEmpty() )
               radial_resolution = astr.toDouble();
            astr  = a.value( "meniscus"    ).toString();
            if ( !astr.isEmpty() )
               meniscus     = astr.toDouble();
            astr  = a.value( "bottom"      ).toString();
            if ( !astr.isEmpty() )
               bottom       = astr.toDouble();
            astr  = a.value( "rnoise"      ).toString();
            if ( !astr.isEmpty() )
               rnoise       = astr.toDouble();
            astr  = a.value( "lrnoise"      ).toString();
            if ( !astr.isEmpty() )
               lrnoise      = astr.toDouble();
            astr  = a.value( "tinoise"     ).toString();
            if ( !astr.isEmpty() )
               tinoise      = astr.toDouble();
            astr  = a.value( "rinoise"     ).toString();
            if ( !astr.isEmpty() )
               rinoise      = astr.toDouble();
            astr  = a.value( "temperature" ).toString();
            if ( !astr.isEmpty() )
               temperature  = astr.toDouble();
            astr  = a.value( "bandform"    ).toString();
            if ( !astr.isEmpty() )
               band_forming = ( astr == "yes" || astr == "1" );
            else
               band_forming = false;
            if ( band_forming )
               band_volume  = a.value( "bandvolume" ).toString().toDouble();
            else
               band_volume  = 0.0;
            astr  = a.value( "rotorCalID" ).toString();
            if ( !astr.isEmpty() )
               rotorCalID   = astr;
            astr  = a.value( "rotorcoeffs" ).toString().simplified();
            if ( !astr.isEmpty() )
            {
               rotorcoeffs[ 0 ] = astr.section( " ", 0, 0 ).toDouble();
               rotorcoeffs[ 1 ] = astr.section( " ", 1, 1 ).toDouble();
            }
            astr  = a.value( "sector"      ).toString();
            if ( !astr.isEmpty() )
               cp_sector    = astr.toInt();
            astr  = a.value( "pathlength"  ).toString();
            if ( !astr.isEmpty() )
               cp_pathlen   = astr.toDouble();
            astr  = a.value( "angle"       ).toString();
            if ( !astr.isEmpty() )
               cp_angle     = astr.toDouble();
            astr  = a.value( "width"       ).toString();
            if ( !astr.isEmpty() )
               cp_width     = astr.toDouble();
         }

         else if ( xml.isStartElement()  &&  xml.name() == "speedstep" )
         {
            speedstepFromXml( xml, sp );

            speed_step.append( sp );
         }

         else if ( xml.isStartElement()  &&  xml.name() == "usermesh" )
         {
            a     = xml.attributes();

            if ( kr == 0 )
               mesh_radius.clear();

            kr++;
            mesh_radius << a.value( "radius" ).toString().toDouble();
         }
      }
   }

   else
   {
      stat = -1;
   }

   return stat;
}

// Save simulation parameters to file
int US_SimulationParameters::save_simparms( QString fname )
{
   int stat = 0;
   const char* mesh[] = { "ASTFEM", "Claverie", "MovingHat", "User", "ASTFVM" };
   const char* grid[] = { "Fixed",  "Moving" };
   US_SimulationParameters::SpeedProfile* spi;

   QFile xfile( fname );

   if ( xfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QXmlStreamWriter xml( &xfile );
      xml.setAutoFormatting( true );

      xml.writeStartDocument();
      xml.writeDTD         ( "<!DOCTYPE US_SimParams>" );
      xml.writeStartElement( "SimParams" );
      xml.writeAttribute   ( "version", "1.0" );

      xml.writeStartElement( "params" );
      xml.writeAttribute   ( "meshType",    QString( mesh[ (int)meshType ] ) );
      xml.writeAttribute   ( "gridType",    QString( grid[ (int)gridType ] ) );
      xml.writeAttribute   ( "simpoints",   QString::number( simpoints ) );
      xml.writeAttribute   ( "radialres", QString::number( radial_resolution ));
      xml.writeAttribute   ( "meniscus",    QString::number( meniscus ) );
      xml.writeAttribute   ( "bottom",      QString::number( bottom ) );
      xml.writeAttribute   ( "rnoise",      QString::number( rnoise ) );
      xml.writeAttribute   ( "lrnoise",     QString::number( lrnoise ) );
      xml.writeAttribute   ( "tinoise",     QString::number( tinoise ) );
      xml.writeAttribute   ( "rinoise",     QString::number( rinoise ) );
      xml.writeAttribute   ( "temperature", QString::number( temperature ) );

      if ( ! rotorCalID.isEmpty() )
         xml.writeAttribute   ( "rotorCalID", rotorCalID );

      if ( rotorcoeffs[ 0 ] != 0.0 )
      {
         xml.writeAttribute   ( "rotorcoeffs", QString().sprintf( "%.3e %.3e",
            rotorcoeffs[ 0 ], rotorcoeffs[ 1 ] ) );
      }

      xml.writeAttribute   ( "bandform",  band_forming ? "1" : "0" );

      if ( band_forming )
         xml.writeAttribute   ( "bandvolume",  QString::number( band_volume ) );

      xml.writeAttribute   ( "sector",      QString::number( cp_sector  ) );
      xml.writeAttribute   ( "pathlength",  QString::number( cp_pathlen ) );
      xml.writeAttribute   ( "angle",       QString::number( cp_angle   ) );
      xml.writeAttribute   ( "width",       QString::number( cp_width   ) );

      if ( meshType == US_SimulationParameters::USER )
      {
         for ( int ii = 0; ii < mesh_radius.size(); ii++ )
         {
            xml.writeStartElement( "usermesh" );
            xml.writeAttribute( "radius",
               QString().sprintf( "%11.5e", mesh_radius[ ii ] ).simplified() );
            xml.writeEndElement();
         }
      }

      for ( int ii = 0; ii < speed_step.size(); ii++ )
      {
         spi = &speed_step[ ii ];

         speedstepToXml( xml, spi );
      }

      xml.writeEndElement  ();   // params
      xml.writeEndElement  ();   // SimParams

      xml.writeEndDocument ();
      xfile.close();
   }

   else
   {
      stat  = -1;
   }

   return stat;
}

// Load simulation parameters from file
int US_SimulationParameters::get_simparms( US_SimulationParameters& sparms,
      QString fname )
{
   return sparms.load_simparms( fname );
}

// Save simulation parameters to file
int US_SimulationParameters::put_simparms( US_SimulationParameters& sparms,
      QString fname )
{
   return sparms.save_simparms( fname );
}

// Get a speed step from an XML portion
void US_SimulationParameters::speedstepFromXml( QXmlStreamReader& xmli,
      SpeedProfile& spo )
{
   const QString trueStr( " 1YesyesTruetrue" );
   QXmlStreamAttributes attr = xmli.attributes();
   QString astr;

   astr  = attr.value( "duration_hrs"  ).toString();
   if ( !astr.isEmpty() )
      spo.duration_hours    = astr.toInt();
   astr  = attr.value( "duration_mins" ).toString();
   if ( !astr.isEmpty() )
      spo.duration_minutes  = astr.toDouble();
   astr  = attr.value( "delay_hrs"     ).toString();
   if ( !astr.isEmpty() )
      spo.delay_hours       = astr.toInt();
   astr  = attr.value( "delay_mins"    ).toString();
   if ( !astr.isEmpty() )
      spo.delay_minutes     = astr.toDouble();
   astr  = attr.value( "rotorspeed"    ).toString();
   if ( !astr.isEmpty() )
      spo.rotorspeed        = astr.toInt();
   astr  = attr.value( "acceleration"  ).toString();
   if ( !astr.isEmpty() )
      spo.acceleration      = astr.toInt();
   astr  = attr.value( "accelerflag"   ).toString();
   if ( !astr.isEmpty() )
      spo.acceleration_flag = ( trueStr.indexOf( astr ) > 0 );
   astr  = attr.value( "scans"         ).toString();
   if ( !astr.isEmpty() )
      spo.scans             = astr.toInt();
   astr  = attr.value( "w2tfirst"      ).toString();
   if ( !astr.isEmpty() )
      spo.w2t_first         = astr.toDouble();
   astr  = attr.value( "w2tlast"       ).toString();
   if ( !astr.isEmpty() )
      spo.w2t_last          = astr.toDouble();
   astr  = attr.value( "timefirst"     ).toString();
   if ( !astr.isEmpty() )
      spo.time_first        = astr.toInt();
   astr  = attr.value( "timelast"      ).toString();
   if ( !astr.isEmpty() )
      spo.time_last         = astr.toInt();

   // MWL enhancements
   astr  = attr.value( "set_speed"     ).toString();
   if ( !astr.isEmpty() )
      spo.set_speed         = astr.toInt();
   astr  = attr.value( "avg_speed"     ).toString();
   if ( !astr.isEmpty() )
      spo.avg_speed         = astr.toDouble();
   astr  = attr.value( "speed_stddev"  ).toString();
   if ( !astr.isEmpty() )
      spo.speed_stddev      = astr.toDouble();
}

// Write a speed step to an XML stream
void US_SimulationParameters::speedstepToXml( QXmlStreamWriter& xmlo,
      SpeedProfile* spi )
{
   xmlo.writeStartElement( "speedstep" );
   xmlo.writeAttribute   ( "rotorspeed",
      QString::number( spi->rotorspeed       ) );
   xmlo.writeAttribute   ( "scans",
      QString::number( spi->scans            ) );
   xmlo.writeAttribute   ( "timefirst",
      QString::number( spi->time_first       ) );
   xmlo.writeAttribute   ( "timelast",
      QString::number( spi->time_last        ) );
   xmlo.writeAttribute   ( "w2tfirst",
      QString::number( spi->w2t_first        ) );
   xmlo.writeAttribute   ( "w2tlast",
      QString::number( spi->w2t_last         ) );
   xmlo.writeAttribute   ( "duration_hrs",
      QString::number( spi->duration_hours   ) );
   xmlo.writeAttribute   ( "duration_mins",
      QString::number( spi->duration_minutes ) );
   xmlo.writeAttribute   ( "delay_hrs",
      QString::number( spi->delay_hours      ) );
   xmlo.writeAttribute   ( "delay_mins",
      QString::number( spi->delay_minutes    ) );
   xmlo.writeAttribute   ( "acceleration",
      QString::number( spi->acceleration     ) );
   xmlo.writeAttribute   ( "accelerflag",
      ( spi->acceleration_flag ? "1" : "0"   ) );

   // Possible MWL enhancements
   if ( spi->set_speed > 0 )
   {
      xmlo.writeAttribute   ( "set_speed",
         QString::number( spi->set_speed     ) );
      xmlo.writeAttribute   ( "avg_speed",
         QString::number( spi->avg_speed     ) );
      xmlo.writeAttribute   ( "speed_stddev",
         QString::number( spi->speed_stddev  ) );
   }

   xmlo.writeEndElement  ();  // speedstep
}

// Get all speed steps for an experiment from the database
int US_SimulationParameters::speedstepsFromDB( US_DB2* dbP, int expID,
      QVector< SpeedProfile >& sps )
{
   int nspeeds    = 0;
   sps.clear();

   if ( dbP == NULL  ||  expID < 1 )
      return nspeeds;

int dbg_level=US_Settings::us_debug();
   QString idExp = QString::number( expID );
   QStringList query;
   query << "all_speedsteps" << idExp;

   dbP->query( query );
   while( dbP->next() )
   {
      SpeedProfile spo;
      int sspeedID          = dbP->value(  0 ).toInt();
      spo.scans             = dbP->value(  1 ).toInt();
      spo.duration_hours    = dbP->value(  2 ).toInt();
      spo.duration_minutes  = dbP->value(  3 ).toDouble();
      spo.delay_hours       = dbP->value(  4 ).toInt();
      spo.delay_minutes     = dbP->value(  5 ).toDouble();
      spo.rotorspeed        = dbP->value(  6 ).toInt();
      spo.acceleration      = dbP->value(  7 ).toInt();
      QString aflag         = dbP->value(  8 ).toString();
      spo.acceleration_flag = ( aflag.isEmpty() || aflag == "1" );
      spo.w2t_first         = dbP->value(  9 ).toDouble();
      spo.w2t_last          = dbP->value( 10 ).toDouble();
      spo.time_first        = dbP->value( 11 ).toInt();
      spo.time_last         = dbP->value( 12 ).toInt();;
      spo.set_speed         = dbP->value( 13 ).toInt();;
      spo.avg_speed         = dbP->value( 14 ).toDouble();;
      spo.speed_stddev      = dbP->value( 15 ).toDouble();;
      sps << spo;
      nspeeds++;
DbgLv(1) << "SP:ssFromDB: speedstep" << nspeeds << "id" << sspeedID;
   }

   return nspeeds;
}

// Upload a speed step for an experiment to the database
int US_SimulationParameters::speedstepToDB( US_DB2* dbP, int expID,
      SpeedProfile* spi )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "SP:ssToDB: dbP expid spi" << dbP << expID << spi;
   int sstepID    = -1;

   if ( dbP == NULL  ||  expID < 0  ||  spi == NULL )
      return sstepID;

   QString idExp = QString::number( expID );
   QStringList query;
   query << "new_speedstep" << idExp
         << QString::number( spi->scans )
         << QString::number( spi->duration_hours )
         << QString::number( spi->duration_minutes )
         << QString::number( spi->delay_hours )
         << QString::number( spi->delay_minutes )
         << QString::number( spi->rotorspeed )
         << QString::number( spi->acceleration )
         << QString(         spi->acceleration_flag ? "1" : "0" )
         << QString::number( spi->w2t_first )
         << QString::number( spi->w2t_last )
         << QString::number( spi->time_first )
         << QString::number( spi->time_last )
         << QString::number( spi->set_speed )
         << QString::number( spi->avg_speed )
         << QString::number( spi->speed_stddev );
   dbP->statusQuery( query );
   sstepID    = dbP->lastInsertID();
DbgLv(1) << "SP:ssToDB: speedstep" << sstepID << dbP->lastError();
DbgLv(1) << "SP:ssToDB:  w2t" << spi->w2t_first << spi->w2t_last
 << QString::number(spi->w2t_first) << QString::number(spi->w2t_last);

   return sstepID;
}

// Pre-fetch a TimeState object from a pair of files, create the pointer
//  internally for that object, and create the internal simulation
//  speed profile vector implied.
int US_SimulationParameters::simSpeedsFromTimeState( const QString tmst_fpath )
{
   tsobj            = new US_TimeState();        // Create TimeState
   tsobj->open_read_data( tmst_fpath, true );    // Open with prefetch
   ssProfFromTimeState( tsobj, sim_speed_prof ); // Create SSP vector
   int dbg_level    = US_Settings::us_debug();
DbgLv(1) << "SP: ssFts: ispeed" << sim_speed_prof[0].rotorspeed
 << "ssp count" << sim_speed_prof.count();
   return sim_speed_prof.count();                // Return number steps
}

// Create a referenced simulation speed step profile from an opened
// TimeState object pointed to.
int US_SimulationParameters::ssProfFromTimeState( US_TimeState* tsobj,
      QVector< SimSpeedProf >& ssps )
{
   int dbg_level    = US_Settings::us_debug();
   ssps.clear();                              // Clear speed prof vector
   SimSpeedProf ssp;                          // Work sim speed profile
DbgLv(1) << "Sim parms:ssProf: ssps.count" << ssps.count();

   // Insure we have needed keys and get formats
   QStringList fkeys;
   QStringList ffmts;

   tsobj->field_keys( &fkeys, &ffmts );       // Get keys and formats

   int tmkx         = fkeys.indexOf( "Time" );            // Key indexes
   int sskx         = fkeys.indexOf( "SetSpeed" );
   int rskx         = fkeys.indexOf( "RawSpeed" );
   int w2kx         = fkeys.indexOf( "Omega2T" );
   QString tmfm     = ( tmkx < 0 ) ? "" : ffmts[ tmkx ];  // Formats
   QString ssfm     = ( sskx < 0 ) ? "" : ffmts[ sskx ];
   QString rsfm     = ( rskx < 0 ) ? "" : ffmts[ rskx ];
   QString w2fm     = ( w2kx < 0 ) ? "" : ffmts[ w2kx ];
   // Do we have the keys we need?
   bool have_keys   = ( tmkx >= 0 )  &&  ( sskx >= 0 )  &&
                      ( rskx >= 0 )  &&  ( w2kx >= 0 );
DbgLv(1) << "Sim parms:ssProf: have_keys" << have_keys;
   if ( ! have_keys )
      return -1;                           // Do not have needed keys

   // If debug_text so directs, change set_speed_resolution
   //  and/or set_speed_low_rpm and/or speed_step_low_secs
   QString dbgval   = US_Settings::debug_value( "SetSpeedReso" );
   int ss_reso      = dbgval.isEmpty() ? DSS_RESO  : dbgval.toInt();
   dbgval           = US_Settings::debug_value( "SetSpeedLowR" );
   int ss_lo_rpm    = dbgval.isEmpty() ? DSS_LO_RPM : dbgval.toInt();
   dbgval           = US_Settings::debug_value( "SpeedStepLowSec" );
   int ss_lo_sec    = dbgval.isEmpty() ? DSS_LO_SEC : dbgval.toInt();

   int nrec         = tsobj->time_count(); // Total time record count
   QList< int >  cspeeds;                  // Constant speeds list

   // Do an initial pass through timestate records to get constant speeds
   tsobj->read_record( 0 );
   int ss1          = 0;
   int ss2          = 0;
   int ss3          = 0;
   for ( int tsx = 1; tsx < nrec; tsx++ )
   {
      ss1              = ss2;              // Set-speed two back
      ss2              = ss3;              // Previous set-speed
      tsobj->read_record();
      ss3              = ssfm == "F4" ?
            (int)qRound( tsobj->time_dvalue( "SetSpeed" ) ) :
                         tsobj->time_ivalue( "SetSpeed" );  // Current set speed

      if ( ss3 == ss2  &&  ss2 == ss1  &&  ss3 > ss_lo_rpm )
      { // This non-zero set-speed and both previous are the same
         if ( ! cspeeds.contains( ss3 ) )
            cspeeds << ss3;                // Save it if first time encountered
      }
   }
DbgLv(1) << "Sim parms:ssProf: cspeeds" << cspeeds;

   // Now do a pass through records to accumulate full step records
   tsobj->read_record( 0 );                // Read the first record
DbgLv(1) << "Sim parms:ssProf: nrec" << nrec;
   int tm_p         = 0;                   // Previous acceleration time
   int tm_c         = tmfm == "F4" ?
         (int)qRound( tsobj->time_dvalue( "Time" ) ) :
                      tsobj->time_ivalue( "Time" );
   bool in_accel    = true;                // Flag in acceleration zone
   int naintvs      = 0;                   // Initial accel intervals
   int ndtimes      = 0;                   // Initial duration times
   int tsx1         = 0;                   // Initial time state index
   double w2_p      = 0.0;                 // Initial prev. omega2t
   int    ss_p      = 0;                   // Initial prev. set speed
   double w2_c      = tsobj->time_dvalue( "Omega2T" );   // 1st omega2t
   double rs_c      = rsfm == "F4" ?
                      tsobj->time_dvalue( "RawSpeed" ) :
              (double)tsobj->time_ivalue( "RawSpeed" );  // 1st raw speed
   int    ss_c      = ssfm == "F4" ?
         (int)qRound( tsobj->time_dvalue( "SetSpeed" ) ) :
                      tsobj->time_ivalue( "SetSpeed" );  // 1st set speed
   double rs_p      = 0.0;                 // Initial prev. raw_speed
   ssp.w2t_b_accel  = 0.0;                 // Set some SimSpeedProf values
   ssp.rotorspeed   = 0.0;
   ssp.time_b_accel = 0;
   int iscan        = 0;                   // On-scan flag/scan-number
   double sum_speed = 0.0;                 // Initial raw speed sum
   double sum_accel = 0.0;                 // Initial accel sum
   double accel_c   = 0.0;                 // Current acceleration
   double accel_p   = 0.0;                 // Previous acceleration
   int pscan        = 0;                   // On-scan flag/scan-number

   while ( ss_c == 0 )
   {  // Skipping until set speed is non-zero
      tsx1++;
      naintvs++;
      tm_p             = tm_c;
      tsobj->read_record();                // Read the second record
      tm_c             = tmfm == "F4" ?                    // Current time
            (int)qRound( tsobj->time_dvalue( "Time" ) ) :
                         tsobj->time_ivalue( "Time" );
      w2_c             = tsobj->time_dvalue( "Omega2T" );  // Current omega2t
      rs_c             = rsfm == "F4" ?
                         tsobj->time_dvalue( "RawSpeed" ) :
                 (double)tsobj->time_ivalue( "RawSpeed" );  // Current raw speed
      ss_c             = ssfm == "F4" ?
            (int)qRound( tsobj->time_dvalue( "SetSpeed" ) ) :
                         tsobj->time_ivalue( "SetSpeed" );  // Current set speed
      accel_c          = rs_c;             // First acceleration value
      sum_accel        = accel_c;          // Initial acceleration sum
   }

   naintvs          = qMin( naintvs, 1 );  // Number accel zone intervals
   double tm_off    = tm_p;                // Time offset: last speed=0 time
   ssp.time_b_accel = 0.0;                 // Accel begin time
   tm_c            -= tm_off;              // Second speed in step (=1.0)
DbgLv(1) << "Sim parms:ssProf: initial tm_c" << tm_c << "naintvs" << naintvs
 << "tsx1" << tsx1 << "w2 rs ss" << w2_c << rs_c << ss_c;
int tm_ci=tm_c;
int tm_cep=tm_ci+150;

   // Loop through all the remaining TimeState records
   for ( int tsx = tsx1; tsx < nrec; tsx++ )
   {
      tm_p             = tm_c;             // Set previous iter values
      w2_p             = w2_c;
      ss_p             = ss_c;
      rs_p             = rs_c;
      accel_p          = accel_c;
      pscan            = iscan;

      tsobj->read_record();                // Read the next record

      // Get current record's values
      tm_c             = tmfm == "F4" ?
            (int)qRound( tsobj->time_dvalue( "Time" ) ) :
                         tsobj->time_ivalue( "Time" );
      tm_c            -= tm_off;
      w2_c             = tsobj->time_dvalue( "Omega2T" );
      rs_c             = rsfm == "F4" ?
                         tsobj->time_dvalue( "RawSpeed" ) :
                 (double)tsobj->time_ivalue( "RawSpeed" );
      ss_c             = ssfm == "F4" ?
            (int)qRound( tsobj->time_dvalue( "SetSpeed" ) ) :
                         tsobj->time_ivalue( "SetSpeed" );
      iscan            = tsobj->time_ivalue( "Scan" );
      accel_c          = rs_c - rs_p;      // Current acceleration
      int ss_c_ts      = ss_c;
if (tm_c<tm_cep || (tsx+5)>nrec || in_accel)
 DbgLv(1) << "Sim parms:ssProf:   tm_c" << tm_c << "rs_c" << rs_c << "ss_c" << ss_c;

      if ( in_accel )
      {  // In acceleration, looking for its end
         ss_c             = (int)qRound( rs_c / (double)ss_reso ) * ss_reso;
         if ( ss_c == ss_p  &&  cspeeds.contains( ss_c ) )
         {  // Found a constant speed:  out of acceleration
DbgLv(1) << "Sim parms:ssProf: accel-end ss_p ss_c" << ss_p << ss_c
 << "tm_c" << tm_c << "rs_p rs_c" << rs_p << rs_c << "ss_c_ts" << ss_c_ts
 << "naintvs" << naintvs << "iscan" << iscan;
            ssp.w2t_e_accel  = w2_p;       // Accel end omega2t
            ssp.time_e_accel = tm_p;       // Accel end time
            //qDebug()<<"accln times"<<ssp.time_e_accel<< ss_c<<ss_p<<rs_c<<rs_p ;
            double tmi_accel = 1.0;
            // only back off one time intervall if there are more than one time interval with acceleration
            if ( naintvs > 1 )
            {
               // Back off 1 intervall of acceleration
               sum_accel    -= accel_p;
               tmi_accel     = (double)( naintvs - 1 );
            }
            ssp.acceleration = sum_accel   // Average acceleration
                             / tmi_accel;

            sum_accel        = 0.0;        // Re-init acceleration sum
            sum_speed        = rs_c;       // Initialize raw speed sum
            ndtimes          = 1;          // Initial duration times count
            in_accel         = false;      // No longer in acceleration

            if ( iscan > 0 )
            {  // On a scan as we enter constant zone:  save 1st scan time
               ssp.time_f_scan  = tm_c;
DbgLv(1) << "Sim parms:ssProf:  f_scan(c)" << tm_c;
            }
            else if ( pscan > 0 )
            {  // On a scan as we enter constant zone:  save 1st scan time
               ssp.time_f_scan  = tm_p;
DbgLv(1) << "Sim parms:ssProf:  f_scan(p)" << tm_p;
            }
         }
         else
         {  // We remain in acceleration
            naintvs++;                     // Bump accel intervals count
            sum_accel       += accel_c;    // Build acceleration sum
         }
      }

      else
      {  // In constant speed, looking for its end
         ss_c             = (int)qRound( rs_c / (double)ss_reso ) * ss_reso;
         if ( ss_c != ss_p  &&  !cspeeds.contains( ss_c ) )
         {  // Set speeds unequal:  back into acceleration
DbgLv(1) << "Sim parms:ssProf: const-end ss_p ss_c" << ss_p << ss_c
 << "ndtimes" << ndtimes << "tm_c" << tm_c
 << "rs_p rs_c" << rs_p << rs_c << "ss_c_ts" << ss_c_ts;

            // But, wait a minute! If the "constant zone" just ending is less than
            // 20 seconds long or contains no scans, then add it to the current
            // speed step's acceleration zone and resume looking for an end
            // to the acceleration zone
            if ( ndtimes < ss_lo_sec  ||  ssp.time_f_scan < 1 )
            {
               in_accel         = true;
               naintvs         += ndtimes;
               sum_speed        = ssp.avg_speed * (double)ndtimes;
               sum_accel        = ssp.acceleration * (double)( naintvs - 1 );
DbgLv(1) << "Sim parms:ssProf: BACK-to-ACCEL: naintvs" << naintvs
 << "ndtimes" << ndtimes << "sum_accel" << sum_accel << "sum_speed" << sum_speed;
               continue;
            }

            // End to a true constant zone:  save a speed step
            ssp.avg_speed    = sum_speed / (double)( ndtimes  );
            ssp.w2t_e_step   = w2_p;
            ssp.time_e_step  = tm_p;
            ssp.rotorspeed   = ss_p;
            ssp.duration     = tm_c - ssp.time_b_accel;
            ssp.rpm_timestate.resize( ssp.duration );
            ssp.w2t_timestate.resize( ssp.duration );
            ssps << ssp;                   // Save speed step

            ssp.time_b_accel = tm_c;       // Start a new one
            ssp.w2t_b_accel  = w2_c;
            ssp.time_f_scan  = 0;
            ssp.time_l_scan  = 0;
            naintvs          = 1;          // Initialize intervals counts
            sum_accel        = accel_c;    // Re-init acceleration sum
            ndtimes          = 0;          // Re-init duration times
            sum_speed        = 0.0;        // Initial raw speed sum
            in_accel         = true;       // Flag in accel zone

         }
         else
         {  // We remain in constant-speed
            sum_speed       += rs_c;       // Add to raw speed sum
            ndtimes++;                     // Bump constant speed times

            if ( iscan > 0 )
            {  // On a scan:  save first,last scan times
               ssp.time_l_scan  = tm_c;    // Keep updating last scan time

               if ( ssp.time_f_scan == 0 )
                  ssp.time_f_scan  = tm_c; // Capture first scan time
            }
         }
      }
   }

   // Finish off final speed step and save it
   ssp.avg_speed       = sum_speed / (double)( ndtimes  );
   ssp.w2t_e_step      = w2_c;
   ssp.time_e_step     = tm_c;
   ssp.rotorspeed      = ss_c;
   ssp.duration        = tm_c - ssp.time_b_accel;
   ssp.rpm_timestate.resize( ssp.duration );
   ssp.w2t_timestate.resize( ssp.duration );
   ssps << ssp;                            // Save speed step

   //-----------------------------------------------------------------
   // update simspeedprofile structure with timestate rpms and w2ts
   //-----------------------------------------------------------------
   int rtimex          = qMax( 0, tsx1 - 1 );  // Position at first accel

DbgLv(1) << "Sim parms:ssProf: ssps.count" << ssps.count();
   for ( int i1 = 0; i1 < ssps.count(); i1++ )
   {
DbgLv(1) << "Sim parms:ssProf:  i1" << i1 << "duration" << ssps[i1].duration;
DbgLv(1) << "Sim parms:ssProf:   rpmsz" << ssps[i1].rpm_timestate.count()
 << "w2tsz" << ssps[i1].w2t_timestate.count();
      for ( int i2 = 0; i2 < ssps[ i1 ].duration; i2++ )
      {
int rst=
         tsobj->read_record( rtimex );
         ssps[ i1 ].rpm_timestate[ i2 ] = rsfm == "F4" ?
                                          tsobj->time_dvalue( "RawSpeed" ) :
                                  (double)tsobj->time_ivalue( "RawSpeed" );
         ssps[ i1 ].w2t_timestate[ i2 ] = tsobj->time_dvalue( "Omega2T" );
if(i2<4 || (i2+5)>ssps[i1].duration)
 DbgLv(1) << "Sim parms:ssProf:    i2" << i2 << "rtimex" << rtimex
  << "rpm" << ssps[ i1 ].rpm_timestate[ i2 ]
  << "w2t" << ssps[ i1 ].w2t_timestate[ i2 ] << "rd_stat" << rst;
         rtimex              = -1;             // Flag "read-next"
      }
   }

   return ssps.count();
}

// Create the internal speed steps vector from the internal
//  simulation speed profile.
int US_SimulationParameters::speedstepsFromSSprof()
{
   int dbg_level     = US_Settings::us_debug();
   int nstep         = speed_step.count();
   int nspstep       = sim_speed_prof.count();

   if ( nspstep < 1 )
      return nstep;

   int kscan         = ( nstep > 0 ) ? speed_step[ 0 ].scans : 50;
   nstep             = nspstep;
   speed_step.resize( nstep );
   int ss_reso       = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso        = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }

   // Create the new full speed step vector by copy from sim_speed_prof
   for ( int ss = 0; ss < nspstep; ss++ )
   {
      SimSpeedProf* ipp = &sim_speed_prof[ ss ];
      SpeedProfile* opp = &speed_step[ ss ];
      int ispeed        = ipp->rotorspeed;
DbgLv(1) << "SP: ssFssp: ss" << ss << "ispeed" << ispeed;
DbgLv(1) << "SP: ssFssp:  ssp:" << ipp->acceleration
 << ipp->w2t_b_accel << ipp->w2t_e_accel << ipp->w2t_e_step
 << ipp->avg_speed << ipp->rotorspeed << ipp->duration
 << ipp->time_b_accel << ipp->time_e_accel << ipp->time_f_scan
 << ipp->time_l_scan << ipp->time_e_step;

      // Get values from SimSpeedProf
      double accel      = ipp->acceleration;
      double w2t_eacc   = ipp->w2t_e_accel;
      double w2t_estep  = ipp->w2t_e_step;
      double avg_speed  = ipp->avg_speed;

      int rspeed        = ipp->rotorspeed;
      int duration      = ipp->duration;
      int time_first    = ipp->time_f_scan;
      int time_last     = ipp->time_l_scan;
      int time_eacc     = ipp->time_e_accel;
      int time_estep    = ipp->time_b_accel + duration;
      // Compute values needed by SpeedStep
      int delay         = time_first - ipp->time_b_accel;
      double dur_mins   = (double)duration / 60.0;
      double dly_mins   = (double)delay / 60.0;
      int dur_hrs       = (int)dur_mins / 60;
      int dly_hrs       = (int)dly_mins / 60;
      dur_mins          = dur_mins - (double)dur_hrs * 60.0;
      dly_mins          = dly_mins - (double)dly_hrs * 60.0;
      double delta_w    = w2t_estep - w2t_eacc;
      double delta_t    = (double)( time_estep - time_eacc );
      double w2t_first  = w2t_eacc + ( time_first - time_eacc )
                          * delta_w / delta_t;
      double w2t_last   = w2t_eacc + ( time_last  - time_eacc )
                          * delta_w / delta_t;

      if ( ( ss + 1 ) == nspstep )
      {  // Insure last step's end scan extends to last time
         time_last         = time_estep;
         w2t_last          = w2t_estep;
      }

      // Set the values of the SpeedStep
      opp->duration_minutes  = dur_mins;
      opp->delay_minutes     = dly_mins;
      opp->w2t_first         = w2t_first;
      opp->w2t_last          = w2t_last;
      opp->avg_speed         = avg_speed;
      opp->speed_stddev      = 0.0;
      opp->duration_hours    = dur_hrs;
      opp->delay_hours       = dly_hrs;
      opp->time_first        = time_first;
      opp->time_last         = time_last;
      opp->scans             = kscan;
      opp->rotorspeed        = rspeed;
      opp->acceleration      = accel;
      opp->set_speed         = (int)qRound( (double)rspeed / (double)ss_reso ) * ss_reso;
      opp->acceleration_flag = true;
DbgLv(1) << "SP: ssFssp:   tf tl wf wl" << time_first << time_last << w2t_first << w2t_last;
DbgLv(1) << "SP: ssFssp:   ss:" << opp->duration_minutes << opp->delay_minutes
 << opp->w2t_first << opp->w2t_last << opp->avg_speed << opp->speed_stddev
 << opp->duration_hours << opp->delay_hours << opp->time_first << opp->time_last
 << opp->scans << opp->rotorspeed << opp->acceleration << opp->set_speed
 << opp->acceleration_flag;
   }

   return nstep;                                 // Return number steps
}

// Debug print
void US_SimulationParameters::debug( void )
{
   qDebug() << "Sim parms Dump";
   qDebug() << "Mesh radius size:" << mesh_radius.size();
   qDebug() << "Simpoints       :" << simpoints;
   qDebug() << "Mesh Type       :" << meshType;
   qDebug() << "Grid Type       :" << gridType;
   qDebug() << "Radial Res      :" << radial_resolution;
   qDebug() << "Meniscus        :" << meniscus;
   qDebug() << "Bottom Pos      :" << bottom_position;
   qDebug() << "Bottom          :" << bottom;
   qDebug() << "Temperature     :" << temperature;
   qDebug() << "Random noise    :" << rnoise;
   qDebug() << "Random noise (l):" << lrnoise;
   qDebug() << "Time Inv Noise  :" << tinoise;
   qDebug() << "Radial Inv Noise:" << rinoise;
   qDebug() << "Band Forming    :" << band_forming;
   qDebug() << "Band Volume     :" << band_volume;
   qDebug() << "Rotor Calibr.ID :" << rotorCalID;
   qDebug() << "Rotor Coeffs    :" << rotorcoeffs[ 0 ] << rotorcoeffs[ 1 ];
   qDebug() << "CP Sector       :" << cp_sector;
   qDebug() << "CP Pathlength   :" << cp_pathlen;
   qDebug() << "CP Angle        :" << cp_angle;
   qDebug() << "CP Width        :" << cp_width;

   for ( int i = 0; i < speed_step.size(); i++ )
   {
      qDebug() << "  Step" << i;
      qDebug() << "   Duration Hours" << speed_step[ i ].duration_hours;
      qDebug() << "   Duration Mins " << speed_step[ i ].duration_minutes;
      qDebug() << "   Delay Hours   " << speed_step[ i ].delay_hours;
      qDebug() << "   Delay Mins    " << speed_step[ i ].delay_minutes;
      qDebug() << "   Scans         " << speed_step[ i ].scans;
      qDebug() << "   Acceleration  " << speed_step[ i ].acceleration;
      qDebug() << "   Rotor Speed   " << speed_step[ i ].rotorspeed;
      qDebug() << "   Accel Flag    " << speed_step[ i ].acceleration_flag;
      qDebug() << "   Omega2t First " << speed_step[ i ].w2t_first;
      qDebug() << "   Omega2t Last  " << speed_step[ i ].w2t_last;
      qDebug() << "   Time First    " << speed_step[ i ].time_first;
      qDebug() << "   Time Last     " << speed_step[ i ].time_last;
      qDebug() << "   Set Speed     " << speed_step[ i ].set_speed;
      qDebug() << "   Average Speed " << speed_step[ i ].avg_speed;
      qDebug() << "   Speed StdDev  " << speed_step[ i ].speed_stddev;
   }
}

