//! \file us_simparms.cpp

#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"

//!< level-conditioned debug print
#define DbgLv(a) if(dbg_level>=a)qDebug()

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


// Set simulation parameter values from an experimental EditedData set.
void US_SimulationParameters::initFromData( US_DB2* db,
   US_DataIO::EditedData& editdata, bool incl_speed )
{
   SpeedProfile sp;

   int     dbg_level   = US_Settings::us_debug();
   double  rpm         = editdata.scanData[ 0 ].rpm;
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
   bottom = US_AstfemMath::calc_bottom( rpm, bottom_position, rotorcoeffs );
#else
   // For NO_DB (back end) the bottom needs to be set after this function
   bottom = bottom_position;
   db     = NULL; // Stop compiler warning
#endif
DbgLv(2) << "SP:iFD: bottom" << bottom;
}

// Read the speed steps vector from runID file
int US_SimulationParameters::readSpeedSteps( QString runID, QString dataType,
      QVector< US_SimulationParameters::SpeedProfile >& speedsteps )
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
      QVector< US_SimulationParameters::SpeedProfile >& speedsteps )
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
   double  rpm_sum     = 0.0;
DbgLv(1) << "SP:cSS: scan" << 1 << "rpm time omega2t"
 << rpm << qRound(time1) << (*scans)[ 0 ].omega2t;

   for ( int ii = 1; ii < scanCount; ii++ )
   {  // Loop to build speed steps where RPM changes
      rpmnext          = (*scans)[ ii ].rpm;
DbgLv(1) << "SP:cSS: scan" << (ii+1) << "rpm time omega2t"
 << rpmnext << qRound((*scans)[ii].seconds) << (*scans)[ii].omega2t;

      if ( rpm != rpmnext  &&  ( qAbs( rpm - rpmnext ) / rpm ) < 0.005 )
      {  // Ignore apparent speed-step if the rpm change is small & for 1 scan
         if ( ii == ( scanCount - 1 )  ||
              rpm == (*scans)[ ii + 1 ].rpm )
            rpmnext = rpm;
         if ( ii == 1 )
            rpm     = rpmnext;
      }

      if ( rpm != rpmnext )
      {  // RPM has changed, so need to create speed step for previous scans
         time2               = (*scans)[ ii - 1 ].seconds;
         w2t2                = (*scans)[ ii - 1 ].omega2t;
//         step_secs           = time2 - time1 + delay_secs;
         step_secs           = time2 - time1;
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
         double rpm_avg      = rpm_sum / (double)sp.scans;
         sp.avg_speed        = rpm_avg;
         sp.rotorspeed       = qRound( rpm_avg );
         sp.set_speed        = sp.rotorspeed;
         double rpm_stdd     = 0.0;
         for ( int jj = lscx; jj < ii; jj++ )
            rpm_stdd           += sq( (*scans)[ jj ].rpm - rpm_avg );
         rpm_stdd            = sqrt( rpm_stdd ) / (double)sp.scans;
         sp.speed_stddev     = rpm_stdd;

         speedsteps << sp;
DbgLv(1) << "SP:cSS:   speedsteps" << speedsteps.size() << "scans" << sp.scans
 << "duration h m" << sp.duration_hours << sp.duration_minutes
 << "delay h m" << sp.delay_hours << sp.delay_minutes << "rpm" << rpm;
DbgLv(1) << "SP:cSS:      w2t1 w2t2 time1 time2" << sp.w2t_first << sp.w2t_last
 << sp.time_first << sp.time_last;

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
//   step_secs           = time2 - time1 + delay_secs;
   step_secs           = time2 - time1;
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
   double rpm_avg      = rpm_sum / (double)sp.scans;
   sp.avg_speed        = rpm_avg;
   sp.rotorspeed       = qRound( rpm_avg );
   sp.set_speed        = sp.rotorspeed;
   double rpm_stdd     = 0.0;
   for ( int jj = lscx; jj < scanCount; jj++ )
      rpm_stdd           += sq( (*scans)[ jj ].rpm - rpm_avg );
   rpm_stdd            = sqrt( rpm_stdd ) / (double)sp.scans;
   sp.speed_stddev     = rpm_stdd;

   speedsteps << sp;
DbgLv(1) << "SP:cSS:   speedsteps" << speedsteps.size() << "scans" << sp.scans
 << "duration h m" << sp.duration_hours << sp.duration_minutes
 << "delay h m" << sp.delay_hours << sp.delay_minutes << "rpm" << rpm;
DbgLv(1) << "SP:cSS:      w2t1 w2t2 time1 time2" << sp.w2t_first << sp.w2t_last
 << sp.time_first << sp.time_last;
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
         QString::number( spi->set_speed        ) );
      xmlo.writeAttribute   ( "avg_speed", 
         QString::number( spi->avg_speed        ) );
      xmlo.writeAttribute   ( "speed_stddev",
         QString::number( spi->speed_stddev     ) );
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

