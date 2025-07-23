//! \file us_run_protocol.cpp

#include "us_run_protocol.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

#ifndef _TR_
#define _TR_(a) QObject::tr(a)
#endif

// RunProtocol constructor
US_RunProtocol::US_RunProtocol()
{
   protoname       = "";
   protoGUID       = QString( "00000000-0000-0000-0000-000000000000" );
   optimahost      = "192.168.1.1";
   investigator    = "";
   temperature     = 20.0;
   temeq_delay     = 10.0;
   exp_label       = "";
   runname         = "";

   scanCount       = 0;
   scanCount_int   = 0;
}

// RunProtocol Equality operator
bool US_RunProtocol::operator== ( const US_RunProtocol& rp ) const
{
qDebug() << "RP:eq?";
   if ( investigator != rp.investigator )  return false;
qDebug() << "RP: EQ inv";
   if ( protoname    != rp.protoname    )  return false;
qDebug() << "RP: EQ pnm";
   if ( protoGUID    != rp.protoGUID    )  return false;
qDebug() << "RP: EQ pgu";
   if ( optimahost   != rp.optimahost   )  return false;
qDebug() << "RP: EQ oho";
   if ( temperature  != rp.temperature  )  return false;
qDebug() << "RP: EQ tem";
   if ( temeq_delay  != rp.temeq_delay  )  return false;
qDebug() << "RP: EQ dly";

   if ( rpRotor      != rp.rpRotor      )  return false;
qDebug() << "RP: EQ ROT";
   if ( rpSpeed      != rp.rpSpeed      )  return false;
qDebug() << "RP: EQ SPD";
   if ( rpCells      != rp.rpCells      )  return false;
qDebug() << "RP: EQ CEL";
   if ( rpSolut      != rp.rpSolut      )  return false;
qDebug() << "RP: EQ SOL";
   if ( rpOptic      != rp.rpOptic      )  return false;
qDebug() << "RP: EQ OPT";
   if ( rpRange      != rp.rpRange      )  return false;
qDebug() << "RP: EQ RNG";
   if ( rpAprof      != rp.rpAprof      )  return false;
qDebug() << "RP: EQ APR";

   return true;
}

// Write all current controls to an XML stream
bool US_RunProtocol::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartDocument();
   xmlo.writeDTD          ( "<!DOCTYPE US_RunProtocol>" );
   xmlo.writeStartElement ( "ProtocolData" );
   xmlo.writeAttribute    ( "version", "1.0" );

   xmlo.writeStartElement ( "protocol" );
   xmlo.writeAttribute    ( "description",  protoname );
   xmlo.writeAttribute    ( "guid",         protoGUID );

   xmlo.writeAttribute    ( "project",      project );
   xmlo.writeAttribute    ( "projectid",    QString::number( projectID ) );
   
   xmlo.writeAttribute    ( "optima_host",  optimahost );
   xmlo.writeAttribute    ( "investigator", investigator );
   xmlo.writeAttribute    ( "temperature",  QString::number( temperature ) );
   xmlo.writeAttribute    ( "temeq_delay",  QString::number( temeq_delay ) );

   rpRotor.toXml( xmlo );
   rpSpeed.toXml( xmlo );
   rpCells.toXml( xmlo );
   rpSolut.toXml( xmlo );
   rpOptic.toXml( xmlo );
   rpRange.toXml( xmlo );
   rpAprof.toXml( xmlo );

   xmlo.writeEndElement();    // protocol

   xmlo.writeEndElement();    // ProtocolData
   xmlo.writeEndDocument();

   return ( ! xmlo.hasError() );
}

// Read all current controls from an XML stream
bool US_RunProtocol::fromXml( QXmlStreamReader& xmli )
{
   while( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isStartElement() )
      {
         QString ename   = xmli.name().toString();

         if ( ename == "protocol" )
         {
            QXmlStreamAttributes attr = xmli.attributes();

            protoname       = attr.value( "description"  ).toString();
            protoGUID       = attr.value( "guid"         ).toString();
            project         = attr.value( "project"      ).toString();
            projectID       = attr.value( "projectid"    ).toString().toInt();
            optimahost      = attr.value( "optima_host"  ).toString();
            investigator    = attr.value( "investigator" ).toString();
            temperature     = attr.value( "temperature"  ).toString().toDouble();
            QString s_ted   = attr.value( "temeq_delay"  ).toString();
            temeq_delay     = s_ted.isEmpty() ? temeq_delay : s_ted.toDouble();
         }

         else if ( ename == "rotor" )      { rpRotor.fromXml( xmli ); }
         else if ( ename == "speed" )      { rpSpeed.fromXml( xmli ); }
         else if ( ename == "cells" )      { rpCells.fromXml( xmli ); }
         else if ( ename == "solutions" )  { rpSolut.fromXml( xmli ); }
         else if ( ename == "optics" )     { rpOptic.fromXml( xmli ); }
         else if ( ename == "ranges"  )    { rpRange.fromXml( xmli ); }
         else if ( ename == "spectra" )    { rpRange.fromXml( xmli ); }
         else if ( ename == "aprofile" )   { rpAprof.fromXml( xmli ); }
      }
   }

   return ( ! xmli.hasError() );
}

// Function to convert from a time to a day,hour,minute,second list
void US_RunProtocol::timeToList( double& sectime, QList< int >& dhms )
{
   int t_day = (int)( sectime / (24*3600) );
   sectime -= t_day * 24 * 3600;

   int t_hour = (int)( sectime / 3600 );
   sectime -= t_hour * 3600;

   int t_minute = (int)( sectime / 60 );
   sectime -= t_minute * 60;

   int t_second = sectime;

  // int t_minute     = (int)( sectime / 60.0 );
   // int t_second     = qRound( sectime - t_minute * 60.0 );
   // int t_hour       = (int)( t_minute / 60.0 );
   //     t_minute    -= ( t_hour * 60 );
   // int t_day        = (int)( t_hour / 24 );
   //     t_hour      -= ( t_day * 24 ); 

   qDebug() << "TimeToList(): DD: " << t_day;
   qDebug() << "TimeToList(): HH: " << t_hour;
   qDebug() << "TimeToList(): MM: " << t_minute;
   qDebug() << "TimeToList(): SS: " << t_second;
   
   dhms.clear();
   dhms << t_day << t_hour << t_minute << t_second;
}

// Function to convert from a time to a day,hour,minute,second list
void US_RunProtocol::timeToList( QTime& timeobj, int& days, QList< int >& dhms )
{
   dhms.clear();
   dhms << days
        << timeobj.hour()
        << timeobj.minute()
        << timeobj.second();
}

// Function to convert to a time from a day,hour,minute,second list
void US_RunProtocol::timeFromList( double& sectime, QList< int >& dhms )
{
   sectime     = dhms[ 0 ] * ( 24 * 60 * 60 ) +
                 dhms[ 1 ] * ( 60 * 60 ) +
                 dhms[ 2 ] * 60 +
                 dhms[ 3 ];
}

// Function to convert to a time from a day,hour,minute,second list
void US_RunProtocol::timeFromList( QTime& timeobj, int& days,
                                   QList< int >& dhms )
{
   timeobj     = QTime( dhms[ 1 ], dhms[ 2 ], dhms[ 3 ] );
   days        = dhms[ 0 ];
}

// Function to convert from a time to "0d 00:06:30" type string
void US_RunProtocol::timeToString( double& sectime, QString& strtime )
{
   QList< int > dhms;
   // timeToList( sectime, dhms );         //ALEXEY: bug!!!
   double sectime_to_list = sectime;
   timeToList( sectime_to_list, dhms );
   strtime          = QString().sprintf( "%dd %02d:%02d:%02d",
                         dhms[ 0 ], dhms[ 1 ], dhms[ 2 ], dhms[ 3 ] );
}

// Function to convert from a time to "0d 00:06:30" type string
void US_RunProtocol::timeToString( QTime& timeobj, int& days, QString& strtime )
{
   strtime          = QString().sprintf( "%dd %02d:%02d:%02d", days,
                         timeobj.hour(), timeobj.minute(), timeobj.second()  );
}

// Function to convert to a time from a "0d 00:06:30" type string
void US_RunProtocol::timeFromString( double& sectime, QString& strtime )
{
   QList< int > dhms;
   int t_day        = strtime.section( "d", 0, 0 ).toInt();
   int t_hour       = strtime.section( ":", 0, 0 ).right( 2 ).toInt();
   int t_minute     = strtime.section( ":", 1, 1 ).toInt();
   int t_second     = strtime.section( ":", 2, 2 ).toInt();

   dhms << t_day << t_hour << t_minute << t_second;

   timeFromList( sectime, dhms );
}

// Function to convert to a time from a "0d 00:06:30" type string
void US_RunProtocol::timeFromString( QTime& timeobj, int& days,
                                     QString& strtime )
{
   days             = strtime.section( "d", 0, 0 ).toInt();
   int t_hour       = strtime.section( ":", 0, 0 ).right( 2 ).toInt();
   int t_minute     = strtime.section( ":", 1, 1 ).toInt();
   int t_second     = strtime.section( ":", 2, 2 ).toInt();
   timeobj          = QTime( t_hour, t_minute, t_second );
}


// RunProtoRotor subclass constructor
US_RunProtocol::RunProtoRotor::RunProtoRotor()
{
   laboratory           = "New Lab";
   rotor                = "New Rotor";
   calibration          = "New Calibration";
   labGUID              = QString( "00000000-0000-0000-0000-000000000000" );
   rotGUID              = labGUID;
   calGUID              = labGUID;
   absGUID              = labGUID;
   labID                = -1;
   rotID                = -1;
   calID                = -1;
   absID                = -1;
}
//3-------------------------------------------------------------------------->80

// RunProtoRotor subclass Equality operator
bool US_RunProtocol::RunProtoRotor::operator== 
                  ( const RunProtoRotor& rp ) const
{
qDebug() << "RP:ROT: labG" << labGUID << rp.labGUID;
qDebug() << "RP:ROT: rotG" << rotGUID << rp.rotGUID;
qDebug() << "RP:ROT: calG" << calGUID << rp.calGUID;
qDebug() << "RP:ROT: absG" << absGUID << rp.absGUID;
   if ( labGUID != rp.labGUID ) return false;
qDebug() << "RP:ROT: EQ labG";
   if ( rotGUID != rp.rotGUID ) return false;
qDebug() << "RP:ROT: EQ rotG";
   if ( calGUID != rp.calGUID ) return false;
qDebug() << "RP:ROT: EQ calG";
   if ( absGUID != rp.absGUID ) return false;
qDebug() << "RP:ROT: EQ absG";

   return true;
}

// Read all current Rotor controls from an XML stream
bool US_RunProtocol::RunProtoRotor::fromXml( QXmlStreamReader& xmli )
{
   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "rotor" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            laboratory  = attr.value( "laboratory"  ).toString();
            rotor       = attr.value( "rotor"       ).toString();
            calibration = attr.value( "calibration" ).toString();
            labID       = attr.value( "labid"       ).toString().toInt();
            rotID       = attr.value( "rotid"       ).toString().toInt();
            calID       = attr.value( "calid"       ).toString().toInt();
            absID       = attr.value( "absid"       ).toString().toInt();
            labGUID     = attr.value( "labguid"     ).toString();
            rotGUID     = attr.value( "rotguid"     ).toString();
            calGUID     = attr.value( "calguid"     ).toString();
            absGUID     = attr.value( "absguid"     ).toString();

            exptype     = attr.value( "exptype"     ).toString();
            opername    = attr.value( "opername"   ).toString();
            operID      = attr.value( "operid"       ).toString().toInt();
            instID      = attr.value( "instid"       ).toString().toInt();
            instrname   = attr.value( "instname" ).toString();
         }

         else
            break;
      }

      else if ( xmli.isEndElement()  &&  ename == "rotor" )
         break;

      xmli.readNext();
   }

   return ( ! xmli.hasError() );
}

// Write the current Rotor portion of controls to an XML stream
bool US_RunProtocol::RunProtoRotor::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "rotor" );

   xmlo.writeAttribute( "laboratory",  laboratory );
   xmlo.writeAttribute( "rotor",       rotor );
   xmlo.writeAttribute( "calibration", calibration );
   xmlo.writeAttribute( "labid",       QString::number( labID ) );
   xmlo.writeAttribute( "rotid",       QString::number( rotID ) );
   xmlo.writeAttribute( "calid",       QString::number( calID ) );
   xmlo.writeAttribute( "absid",       QString::number( absID ) );

   xmlo.writeAttribute( "instid",      QString::number( instID ) );
   xmlo.writeAttribute( "instname",    instrname );
   xmlo.writeAttribute( "operid",      QString::number( operID ) );
   xmlo.writeAttribute( "opername",    opername );
   xmlo.writeAttribute( "exptype",     exptype );
   
   xmlo.writeAttribute( "labguid",     labGUID );
   xmlo.writeAttribute( "rotguid",     rotGUID );
   xmlo.writeAttribute( "calguid",     calGUID );
   xmlo.writeAttribute( "absguid",     absGUID );

   xmlo.writeEndElement();    // rotor

   return ( ! xmlo.hasError() );
}


// RunProtoSpeed subclass constructor
US_RunProtocol::RunProtoSpeed::RunProtoSpeed()
{
   nstep                = 1;
   ssteps << SpeedStep();
}

// RunProtoSpeed subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::operator== 
                  ( const RunProtoSpeed& rp ) const
{
   if ( nstep  != rp.nstep  ) return false;
   if ( ssteps != rp.ssteps ) return false;

   return true;
}

// Read all current Speed controls from an XML stream
bool US_RunProtocol::RunProtoSpeed::fromXml( QXmlStreamReader& xmli )
{
   ssteps.clear();
   nstep           = 0;

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "speedstep" )
         {
            SpeedStep ss;
            QXmlStreamAttributes attr = xmli.attributes();
            ss.speed     = attr.value( "rotorspeed"       ).toString().toDouble();
            ss.accel     = attr.value( "acceleration"     ).toString().toDouble();
            QString s_du = attr.value( "duration" )     .toString();
	    //Uv-vis
	    QString s_dy = attr.value( "delay" )        .toString();
            QString s_si = attr.value( "scan_interval" ).toString();
	    ss.scancount = attr.value( "scancount" )    .toString().toInt();
	    //interference
	    QString s_dy_int = attr.value( "delay_int" )        .toString();
            QString s_si_int = attr.value( "scan_interval_int" ).toString();
	    ss.scancount_int = attr.value( "scancount_int" )    .toString().toInt();
	    
            double d_du  = attr.value( "duration_minutes" ).toString().toDouble();
            double d_dy  = attr.value( "delay_seconds"    ).toString().toDouble();
	    QString stage_delay = attr.value( "stage_delay"    ).toString();

	    //duration
	    if ( ! s_du.isEmpty() )
	      US_RunProtocol::timeFromString( ss.duration, s_du );
            else
	      ss.duration  = d_du * 60.0;                      // ALEXEY why do we need this?

	    //Uv-vis
            if ( ! s_dy.isEmpty() )
	      US_RunProtocol::timeFromString( ss.delay, s_dy );
            else
	      ss.delay     = d_dy;                            // ALEXEY why do we need this?  
	    if ( ! s_si.isEmpty() )
	      US_RunProtocol::timeFromString( ss.scanintv, s_si );
	    
	    //interference
	    if ( ! s_dy_int.isEmpty() )
	      US_RunProtocol::timeFromString( ss.delay_int, s_dy_int );
	    if ( ! s_si_int.isEmpty() )
	      US_RunProtocol::timeFromString( ss.scanintv_int, s_si_int );

	    //Stage delay
	    if ( ! stage_delay.isEmpty() )
	      US_RunProtocol::timeFromString( ss.delay_stage, stage_delay );

	    ss.total_time = ss.duration + ss.delay_stage;

            ssteps << ss;
         }

         else if ( ename == "speed" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            int i_spin    = attr.value( "spin_down" ).toString().toInt();
            int i_rcal    = attr.value( "radial_calibration" ).toString().toInt();
            spin_down     = ( i_spin != 0 );
            radial_calib  = ( i_rcal != 0 );
         }

         else
            break;
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "speed" )    // Break after "</speed>"
         break;
   }

   nstep           = ssteps.count();

   return ( ! xmli.hasError() );
}

// Write the current Speed portion of controls to an XML stream
bool US_RunProtocol::RunProtoSpeed::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "speed" );
   xmlo.writeAttribute( "spin_down",          spin_down    ? "1" : "0" );
   xmlo.writeAttribute( "radial_calibration", radial_calib ? "1" : "0" );

   for ( int ii = 0; ii < ssteps.count(); ii++ )
   {
     qDebug() << "SPEED toXml ssteps[ii].duration, ssteps[ii].scanintv  0 : " << ssteps[ii].duration << ", "<< ssteps[ii].scanintv;
      QString s_durat;
      QString s_stdelay;
      //Uv-vis
      QString s_delay;
      QString s_sintv;
      //interference
      QString s_delay_int;
      QString s_sintv_int;

      US_RunProtocol::timeToString( ssteps[ ii ].duration, s_durat );
      US_RunProtocol::timeToString( ssteps[ ii ].delay_stage, s_stdelay );
      //Uv-vis
      US_RunProtocol::timeToString( ssteps[ ii ].delay,    s_delay );
      US_RunProtocol::timeToString( ssteps[ ii ].scanintv, s_sintv );
      //onterference
      US_RunProtocol::timeToString( ssteps[ ii ].delay_int,    s_delay_int );
      US_RunProtocol::timeToString( ssteps[ ii ].scanintv_int, s_sintv_int );
     
    
      qDebug() << "SPEED toXml ssteps[ii].duration, ssteps[ii].scanintv  1 : " << ssteps[ii].duration << ", "<< ssteps[ii].scanintv << ", " << ssteps[ ii ].delay_stage;

      xmlo.writeStartElement( "speedstep" );
      xmlo.writeAttribute   ( "rotorspeed",
                              QString::number( ssteps[ ii ].speed ) );
      xmlo.writeAttribute   ( "acceleration",
                              QString::number( ssteps[ ii ].accel ) );
      xmlo.writeAttribute   ( "duration",      s_durat );

      //Uv_vis
      xmlo.writeAttribute   ( "delay",         s_delay );
      xmlo.writeAttribute   ( "scan_interval", s_sintv );
      xmlo.writeAttribute   ( "scancount",     QString::number( ssteps[ ii ].scancount ) );
      
      //interference
      xmlo.writeAttribute   ( "delay_int",         s_delay_int );
      xmlo.writeAttribute   ( "scan_interval_int", s_sintv_int );
      xmlo.writeAttribute   ( "scancount_int",     QString::number( ssteps[ ii ].scancount_int ) );

      //stage delay
      xmlo.writeAttribute   ( "stage_delay", s_stdelay );
      
      xmlo.writeEndElement  (); // speedstep

      // qDebug() << "SPEED toXml 2 : " << rpSpeed->ssteps[ii].duration;
   }
   xmlo.writeEndElement();    // speed

   //qDebug() << "SPEED toXml 3 : " << rpSpeed->ssteps[ii].duration;

   return ( ! xmlo.hasError() );
}

// RunProtoSpeed::SpeedStep subclass constructor
US_RunProtocol::RunProtoSpeed::SpeedStep::SpeedStep()
{
   speed       = 45000.0;
   accel       = 400.0;
   //duration    = 330.0;
   duration    = 5*3600 + 30*60;  // 5h 30 min - total in seconds
   delay_stage = 0.0;
   //Total time
   total_time = duration + delay_stage;
   
   //Uv-vis
   delay       = 120.0;
   scanintv    = 16.85;             //ALEXEY: set default scanint (in secs) which corresponds to 45000 RPM
   scanintv_min = scanintv;
   //Interference
   delay_int       = 300.0;
   scanintv_int    = 5.0;  
   scanintv_int_min = scanintv_int;

   //ALEXEY: also initialize new added params
   scancount = 0;
   scancount_int = 0;

   has_uvvis = false;
   has_interference = false;
}

// RunProtoSpeed::SpeedStep subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::SpeedStep::operator==
                  ( const SpeedStep& ss ) const
{
  qDebug() << "RP:SPEED: speed, ss.speed -- " << speed << ", " << ss.speed;
  
   if ( speed    != ss.speed    ) return false;
   if ( accel    != ss.accel    ) return false;
   if ( duration != ss.duration ) return false;
   if ( delay    != ss.delay    ) return false;
   if ( scanintv != ss.scanintv ) return false;

   return true;
}


// RunProtoCells subclass constructor
US_RunProtocol::RunProtoCells::RunProtoCells()
{
   ncell                = 0;
   nused                = 0;
   used.clear();
}

// RunProtoCells subclass Equality operator
bool US_RunProtocol::RunProtoCells::operator== 
                  ( const RunProtoCells& rp ) const
{
   if ( ncell  != rp.ncell ) return false;
qDebug() << "RP:CEL: EQ nce" << ncell << rp.ncell;
   if ( nused  != rp.nused ) return false;
qDebug() << "RP:CEL: EQ nus" << nused << rp.nused;
   if ( used   != rp.used  ) return false;
qDebug() << "RP:CEL: EQ use";

   return true;
}

// Read all current Cells controls from an XML stream
bool US_RunProtocol::RunProtoCells::fromXml( QXmlStreamReader& xmli )
{
   used.clear();

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {

         if ( ename == "cells" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            ncell          = attr.value( "total_holes" ).toString().toInt();
            nused          = attr.value( "used_holes"  ).toString().toInt();
         }

         else if ( ename == "cell" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            CellUse cu;
            cu.cell        = attr.value( "id"             ).toString().toInt();
            cu.centerpiece = attr.value( "centerpiece"    ).toString();
            cu.windows     = attr.value( "windows"        ).toString();
            cu.cbalance    = attr.value( "counterbalance" ).toString();
qDebug() << "fX:cel: cel cen win bal"
 << cu.cell << cu.centerpiece << cu.windows << cu.cbalance;
            used << cu;
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "cells" )    // Break after "</cells>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current Cells portion of controls to an XML stream
bool US_RunProtocol::RunProtoCells::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "cells" );
   xmlo.writeAttribute   ( "total_holes", QString::number( ncell ) );
   xmlo.writeAttribute   ( "used_holes",  QString::number( nused ) );

  
   for ( int ii = 0; ii < nused; ii++ )
   {
      xmlo.writeStartElement( "cell" );
      xmlo.writeAttribute   ( "id", QString::number( used[ ii ].cell ) );
      // if ( ( ii + 1 ) < nused )                                         //ALEXEY bug
      // {
      //    xmlo.writeAttribute( "centerpiece",    used[ ii ].centerpiece );
      //    xmlo.writeAttribute( "windows",        used[ ii ].windows     );
      // }
      if ( used[ ii ].cell != ncell )
      {
         xmlo.writeAttribute( "centerpiece",    used[ ii ].centerpiece );
         xmlo.writeAttribute( "windows",        used[ ii ].windows     );
      }           
      else
      {
	//if ( used[ ii ].cbalance.contains( "centerpiece"  ) )
	if ( !used[ ii ].cbalance.contains( "counterbalance"  ) )
         {
            xmlo.writeAttribute( "centerpiece",    used[ ii ].centerpiece );
            xmlo.writeAttribute( "windows",        used[ ii ].windows     );
         }
         else
            xmlo.writeAttribute( "counterbalance", used[ ii ].cbalance    );   // ALEXEY: not always counterbalance
      }
      xmlo.writeEndElement(); // cell
   }

   xmlo.writeEndElement(); // cells

   return ( ! xmlo.hasError() );
}

// RunProtoCells::CellUse subclass constructor
US_RunProtocol::RunProtoCells::CellUse::CellUse()
{
   cell                 = 0;
   centerpiece          = _TR_( "empty" );
   windows              = _TR_( "quartz" );
   cbalance             = _TR_( "empty (counterbalance)" );
}

// RunProtoCells::CellUse subclass Equality operator
bool US_RunProtocol::RunProtoCells::CellUse::operator==
                  ( const CellUse& c ) const
{
qDebug() << "RP: CU: cel" << cell << c.cell;
qDebug() << "RP: CU: cen" << centerpiece << c.centerpiece;
qDebug() << "RP: CU: win" << windows << c.windows;
qDebug() << "RP: CU: bal" << cbalance << c.cbalance;
   if ( cell        != c.cell        ) return false;
qDebug() << "RP: CU: EQ cel";
   if ( centerpiece != c.centerpiece ) return false;
qDebug() << "RP: CU: EQ cen";
   if ( windows     != c.windows     ) return false;
qDebug() << "RP: CU: EQ win";
   if ( cbalance    != c.cbalance    ) return false;
qDebug() << "RP: CU: EQ bal";

   return true;
}


// RunProtoSolutions subclass constructor
US_RunProtocol::RunProtoSolutions::RunProtoSolutions()
{
   nschan               = 0;
   nuniqs               = 0;
   chsols.clear();
   solus .clear();
   sids  .clear();
}

// RunProtoSolutions subclass Equality operator
bool US_RunProtocol::RunProtoSolutions::operator== 
                  ( const RunProtoSolutions& rp ) const
{
qDebug() << "RP:SOL: nch" << nschan << rp.nschan;
qDebug() << "RP:SOL: nun" << nuniqs << rp.nuniqs;
if(nschan>0&&rp.nschan>0)
qDebug() << "RP:SOL: chs"
 << chsols[0].channel   << rp.chsols[0].channel
 << chsols[0].solution  << rp.chsols[0].solution
 << chsols[0].sol_id    << rp.chsols[0].sol_id
 << chsols[0].ch_comment<< rp.chsols[0].ch_comment;
//            cs.channel     = attr.value( "channel"      ).toString();
//            cs.solution    = attr.value( "name"         ).toString();
//            cs.sol_id      = attr.value( "id"           ).toString();
//            cs.ch_comment  = attr.value( "chan_comment" ).toString();
qDebug() << "RP:SOL: sid" << sids << rp.sids;
   if ( nschan != rp.nschan ) return false;
qDebug() << "RP:SOL: EQ nsc";
   if ( nuniqs != rp.nuniqs ) return false;
qDebug() << "RP:SOL: EQ nun";
   if ( chsols != rp.chsols ) return false;
qDebug() << "RP:SOL: EQ chs";
   if ( sids   != rp.sids   ) return false;
qDebug() << "RP:SOL: EQ sid";

   return true;
}

// Read all current Solutions controls from an XML stream
bool US_RunProtocol::RunProtoSolutions::fromXml( QXmlStreamReader& xmli )
{
   nschan               = 0;
   chsols.clear();

   while( ! xmli.atEnd() )
   {  // Read elements from solution portion of XML stream
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "solution" )
         {  // Accumulate each solution object
            ChanSolu cs;
            QXmlStreamAttributes attr = xmli.attributes();
            cs.channel     = attr.value( "channel"      ).toString();
            cs.solution    = attr.value( "name"         ).toString();
            cs.sol_id      = attr.value( "id"           ).toString();
            cs.ch_comment  = attr.value( "chan_comment" ).toString();
            chsols << cs;
            nschan++;
         }
      }

      bool was_end    = xmli.isEndElement();   // Just read was End of element?
      xmli.readNext();                         // Read the next element

      if ( was_end  &&  ename == "solutions" ) // Break after "</solutions>"
         break;
   }

   // Build unique solutions list, with corresponding solution IDs
   nuniqs               = 0;
   solus.clear();
   sids .clear();

   //qDebug() << "NSCHAN FOR PROTO: !!!!!!!!!!!!!!!!!!! " << nschan;
   for ( int ii = 0; ii < nschan; ii++ )
   {
      QString sdesc   = chsols[ ii ].solution;

      if ( ! solus.contains( sdesc ) )
      {  // Update unique solutions list and correspond Id list
         solus << sdesc;
         sids  << chsols[ ii ].sol_id;
         nuniqs++;
      }
   }

   return ( ! xmli.hasError() );
}

// Write the current Solutions portion of controls to an XML stream
bool US_RunProtocol::RunProtoSolutions::toXml( QXmlStreamWriter& xmlo )
{
   nschan               = chsols.count();
   xmlo.writeStartElement( "solutions" );
   for ( int ii = 0; ii < nschan; ii++ )
   {
      xmlo.writeStartElement( "solution" );
      xmlo.writeAttribute   ( "channel",      chsols[ ii ].channel    );
      xmlo.writeAttribute   ( "name",         chsols[ ii ].solution   );
      xmlo.writeAttribute   ( "id",           chsols[ ii ].sol_id     );
      xmlo.writeAttribute   ( "chan_comment", chsols[ ii ].ch_comment );
      xmlo.writeEndElement(); // solution
   }
   xmlo.writeEndElement();    // solutions

   return ( ! xmlo.hasError() );
}

// RunProtoSolutions::ChanSolu subclass constructor
US_RunProtocol::RunProtoSolutions::ChanSolu::ChanSolu()
{
   channel    .clear();
   solution   .clear();
   sol_id     .clear();
   ch_comment .clear();
}

// RunProtoSolutions::ChanSolu subclass Equality operator
bool US_RunProtocol::RunProtoSolutions::ChanSolu::operator==
                  ( const ChanSolu& c ) const
{
   if ( channel     != c.channel     ) return false;
   if ( solution    != c.solution    ) return false;
   if ( sol_id      != c.sol_id      ) return false;
   if ( ch_comment  != c.ch_comment  ) return false;

   return true;
}


// RunProtoOptics subclass constructor
US_RunProtocol::RunProtoOptics::RunProtoOptics()
{
   nochan               = 0;
   chopts.clear();
}

// RunProtoOptics subclass Equality operator
bool US_RunProtocol::RunProtoOptics::operator== 
                  ( const RunProtoOptics& rp ) const
{
   if ( nochan != rp.nochan ) return false;
qDebug() << "RP:OPT: EQ nch";
   if ( chopts != rp.chopts ) return false;
qDebug() << "RP:OPT: EQ cho";

   return true;
}

// Read all current Optical Systems controls from an XML stream
bool US_RunProtocol::RunProtoOptics::fromXml( QXmlStreamReader& xmli )
{
   nochan               = 0;
   chopts.clear();

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "optical_system" )
         {
            OpticSys os;
            QXmlStreamAttributes attr = xmli.attributes();
            os.channel   = attr.value( "channel" ).toString();
            os.scan1     = attr.value( "scan1"   ).toString();
            os.scan2     = attr.value( "scan2"   ).toString();
            os.scan3     = attr.value( "scan3"   ).toString();
qDebug() << "RP:OPT: fX: ch,s1,s2,s3"
 << os.channel << os.scan1 << os.scan2 << os.scan3;
            chopts << os;
            nochan++;
         }
      }

      bool was_end    = xmli.isEndElement();   // Just read was End of element?
      xmli.readNext();                         // Read the next element

      if ( was_end  &&  ename == "optics" )    // Break after "</optics>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current Optical Systems portion of controls to an XML stream
bool US_RunProtocol::RunProtoOptics::toXml( QXmlStreamWriter& xmlo )
{
   nochan               = chopts.count();
   xmlo.writeStartElement( "optics" );
   for ( int ii = 0; ii < nochan; ii++ )
   {
      xmlo.writeStartElement( "optical_system" );
      xmlo.writeAttribute   ( "channel", chopts[ ii ].channel );
      if ( ! chopts[ ii ].scan1.isEmpty() )
         xmlo.writeAttribute   ( "scan1",   chopts[ ii ].scan1 );
      if ( ! chopts[ ii ].scan2.isEmpty() )
         xmlo.writeAttribute   ( "scan2",   chopts[ ii ].scan2 );
      if ( ! chopts[ ii ].scan3.isEmpty() )
         xmlo.writeAttribute   ( "scan3",   chopts[ ii ].scan3 );
      xmlo.writeEndElement(); // optical_system
qDebug() << "RP:OPT: tX: ch,s1,s2,s3"
 << chopts[ii].channel << chopts[ii].scan1 << chopts[ii].scan2 << chopts[ii].scan3;
   }
   xmlo.writeEndElement(); // optics

   return ( ! xmlo.hasError() );
}

// RunProtoOptics::OpticSys subclass constructor
US_RunProtocol::RunProtoOptics::OpticSys::OpticSys()
{
   channel .clear();
   scan1   .clear();
   scan2   .clear();
   scan3   .clear();
}

// RunProtoOptics::OpticSys subclass Equality operator
bool US_RunProtocol::RunProtoOptics::OpticSys::operator==
                  ( const OpticSys& c ) const
{
   if ( channel     != c.channel     ) return false;
   if ( scan1       != c.scan1       ) return false;
   if ( scan2       != c.scan2       ) return false;
   if ( scan3       != c.scan3       ) return false;

   return true;
}


// RunProtoRanges subclass constructor
US_RunProtocol::RunProtoRanges::RunProtoRanges()
{
   nranges              = 0;
   chrngs.clear();
}

// RunProtoRanges subclass Equality operator
bool US_RunProtocol::RunProtoRanges::operator== 
                  ( const RunProtoRanges& rp ) const
{
   if ( nranges != rp.nranges  ) return false;
qDebug() << "RP:RNG: EQ nrn";
   if ( chrngs  != rp.chrngs )   return false;
qDebug() << "RP:RNG: EQ chr";

   return true;
}

// Read all current Ranges controls from an XML stream
bool US_RunProtocol::RunProtoRanges::fromXml( QXmlStreamReader& xmli )
{
   nranges              = 0;
   chrngs.clear();
   Ranges rng;

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "range"  ||  ename == "spectrum" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            rng.channel          = attr.value( "channel" )     .toString();
            QString slorad       = attr.value( "start_radius" ).toString();
            QString shirad       = attr.value( "end_radius" )  .toString();
            rng.lo_rad           = slorad.isEmpty() ? rng.lo_rad : slorad.toDouble();
            rng.hi_rad           = shirad.isEmpty() ? rng.hi_rad : shirad.toDouble();

	    //abde: for backward compatibility:
	    if (  attr.hasAttribute ("abde_buffer") )
	      {
		( attr.value( "abde_buffer" ) .toString().toInt() ) ?
		  rng.abde_buffer_spectrum = true : rng.abde_buffer_spectrum = false;
	      }
	    else
	      rng.abde_buffer_spectrum = false;

	    if (  attr.hasAttribute ("abde_mwl_deconv") )
	      {
		( attr.value( "abde_mwl_deconv" ) .toString().toInt() ) ?
		  rng.abde_mwl_deconvolution = true : rng.abde_mwl_deconvolution = false;
	      }
	    else
	      rng.abde_mwl_deconvolution = false;
	    
	    //end of abde
	    
            rng.wvlens.clear();
         }

         else if ( ename == "wavelength"  ||  ename == "point" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            rng.wvlens << attr.value( "lambda" ).toString().toDouble();
         }
      }

      else if ( xmli.isEndElement()  &&
               ( ename == "range"  ||  ename == "spectrum" ) )
      {
         chrngs << rng;
         nranges++;
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end )
      {
         if ( ename == "ranges"  ||          // Break after "</ranges>"
              ename == "spectra" )           // Break after "</spectra>"
           break;
      }
   }

   return ( ! xmli.hasError() );
}

// Write the current Ranges portion of controls to an XML stream
bool US_RunProtocol::RunProtoRanges::toXml( QXmlStreamWriter& xmlo )
{
   nranges              = chrngs.count();
   xmlo.writeStartElement( "ranges" );
   for ( int ii = 0; ii < nranges; ii++ )
   {
      xmlo.writeStartElement( "range" );
      xmlo.writeAttribute   ( "channel",      chrngs[ ii ].channel );
      xmlo.writeAttribute   ( "start_radius",
                              QString::number( chrngs[ ii ].lo_rad ) );
      xmlo.writeAttribute   ( "end_radius",
                              QString::number( chrngs[ ii ].hi_rad ) );
      //abde
      xmlo.writeAttribute   ( "abde_buffer",
                              QString::number( int(chrngs[ ii ].abde_buffer_spectrum )) );
      xmlo.writeAttribute   ( "abde_mwl_deconv",
                              QString::number( int(chrngs[ ii ].abde_mwl_deconvolution )) );
      
      for ( int jj = 0; jj < chrngs[ ii ].wvlens.count(); jj++ )
      {
         xmlo.writeStartElement( "wavelength" );
         xmlo.writeAttribute   ( "lambda",
                                 QString::number( chrngs[ ii ].wvlens[ jj ] ) );
         xmlo.writeEndElement(); // wavelength
      }
      xmlo.writeEndElement(); // range
   }
   xmlo.writeEndElement(); // ranges

   return ( ! xmlo.hasError() );
}

// RunProtoRanges::Ranges subclass constructor
US_RunProtocol::RunProtoRanges::Ranges::Ranges()
{
   channel.clear();
   wvlens .clear();
   lo_rad               = 5.75;
   hi_rad               = 7.25;
   abde_buffer_spectrum = false;
   abde_mwl_deconvolution = false;
}

// RunProtoRanges::Ranges subclass Equality operator
bool US_RunProtocol::RunProtoRanges::Ranges::operator==
                  ( const Ranges& s ) const
{
   if ( channel     != s.channel     ) return false;
   if ( wvlens      != s.wvlens      ) return false;
   if ( lo_rad      != s.lo_rad      ) return false;
   if ( hi_rad      != s.hi_rad      ) return false;

   //abde
   int abde_buff1 = int( abde_buffer_spectrum );
   int abde_buff2 = int( s.abde_buffer_spectrum );
   if (abde_buff1 != abde_buff2) return false;

   //as for 'abde_mwl_deconvolution':
   //do not compare as this can change on fly && will be checked differently

   return true;
}


// RunProtoAProfile subclass constructor
US_RunProtocol::RunProtoAProfile::RunProtoAProfile()
{
   aprofname     = "";
   aprofGUID     = QString( "00000000-0000-0000-0000-000000000000" );
   aprofID       = 0;
}

// RunProtoUpload subclass Equality operator
bool US_RunProtocol::RunProtoAProfile::operator== 
                  ( const RunProtoAProfile& u ) const
{
   if ( aprofname  != u.aprofname  ) return false;
   if ( aprofGUID  != u.aprofGUID  ) return false;

   return true;
}

// Read all current Analysis Profile controls from an XML stream
bool US_RunProtocol::RunProtoAProfile::fromXml( QXmlStreamReader& xmli )
{
   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "aprofile" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            aprofname   = attr.value( "name"  ).toString();
            aprofGUID   = attr.value( "guid"  ).toString();
         }

         else
            break;
      }

      else if ( xmli.isEndElement()  &&  ename == "aprofile" )
         break;

      xmli.readNext();
   }

   return ( ! xmli.hasError() );
}

// Write the current Analysis Profile portion to an XML stream
bool US_RunProtocol::RunProtoAProfile::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "aprofile" );

   xmlo.writeAttribute( "name", aprofname );
   xmlo.writeAttribute( "guid", aprofGUID );

   xmlo.writeEndElement();    // aprofile

   return ( ! xmlo.hasError() );
}


// RunProtoUpload subclass constructor
US_RunProtocol::RunProtoUpload::RunProtoUpload()
{
   us_xml .clear();
   op_json.clear();
}

// RunProtoUpload subclass Equality operator
bool US_RunProtocol::RunProtoUpload::operator== 
                  ( const RunProtoUpload& u ) const
{
   if ( us_xml  != u.us_xml  ) return false;
   if ( op_json != u.op_json ) return false;

   return true;
}

