//! \file us_anapro_parms.cpp

#include "us_anapro_parms.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

#ifndef _TR_
#define _TR_(a) QObject::tr(a)
#endif

// AnaProfParms constructor
US_AnaProfParms::US_AnaProfParms()
{
   aprofname       = "";
   aprofGUID       = QString( "00000000-0000-0000-0000-000000000000" );
   protoname       = "";
   protoGUID       = QString( "00000000-0000-0000-0000-000000000000" );
}

// AnaProfParms Equality operator
bool US_AnaProfParms::operator== ( const US_AnaProfParms& rp ) const
{
   if ( aprofname    != rp.aprofname    )  return false;
   if ( aprofGUID    != rp.aprofGUID    )  return false;
   if ( protoname    != rp.protoname    )  return false;
   if ( protoGUID    != rp.protoGUID    )  return false;

   if ( ap2DSA       != rp.ap2DSA  )  return false;
   if ( apPCSA       != rp.apPCSA  )  return false;

   return true;
}

// Write all current controls to an XML stream
bool US_AnaProfParms::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartDocument();
   xmlo.writeDTD          ( "<!DOCTYPE US_AnalysisProfile>" );
   xmlo.writeStartElement ( "AnalysisProfileData" );
   xmlo.writeAttribute    ( "version", "1.0" );

   xmlo.writeStartElement ( "analysis_profile" );
   xmlo.writeAttribute    ( "description",    aprofname );
   xmlo.writeAttribute    ( "guid",           aprofGUID );

   for ( int ii = 0; ii < pchans.count(); ii++ )
   {
      xmlo.writeStartElement ( "channel_parms" );
      xmlo.writeAttribute    ( "channel",  pchans  [ ii ] );
      xmlo.writeAttribute    ( "chandesc", chndescs[ ii ] );
      xmlo.writeAttribute    ( "load_concen_ratio",
                               QString::number( lc_ratios[ ii ] ) );
      xmlo.writeAttribute    ( "lcr_tolerance",
                               QString::number( lc_tolers[ ii ] ) );
      xmlo.writeAttribute    ( "load_volume",
                               QString::number( l_volumes[ ii ] ) );
      xmlo.writeAttribute    ( "lv_tolerance",
                               QString::number( lv_tolers[ ii ] ) );
      xmlo.writeEndElement();    // channel_parms
   }

   ap2DSA.toXml( xmlo );
   apPCSA.toXml( xmlo );

   xmlo.writeEndElement();    // analysis_profile

   xmlo.writeEndElement();    // AnalysisProfileData
   xmlo.writeEndDocument();

   return ( ! xmlo.hasError() );
}

// Read all current controls from an XML stream
bool US_AnaProfParms::fromXml( QXmlStreamReader& xmli )
{
   int chx            = 0;

   while( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isStartElement() )
      {
         QString ename   = xmli.name().toString();

         if ( ename == "analysis_profile" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
	    aprofname       = attr.value( "project"  ).toString();
	    aprofGUID       = attr.value( "projectid"  ).toInt();
	    
            protoname       = attr.value( "description"  ).toString();
            protoGUID       = attr.value( "guid"         ).toString();
         }

         else if ( ename == "channel_parms" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            pchans   [ chx ] = attr.value( "channel"           ).toString();
            chndescs [ chx ] = attr.value( "chandesc"          ).toString();
            lc_ratios[ chx ] = attr.value( "load_concen_ratio" ).toString().toDouble();
            lc_tolers[ chx ] = attr.value( "lcr_toleraance"    ).toString().toDouble();
            l_volumes[ chx ] = attr.value( "load_volume"       ).toString().toDouble();
            lv_tolers[ chx ] = attr.value( "lv_toleraance"     ).toString().toDouble();
            chx++;
         }

         else if ( ename == "2DSA" )      { ap2DSA.fromXml( xmli ); }
         else if ( ename == "PCSA" )      { apPCSA.fromXml( xmli ); }
      }
   }

   return ( ! xmli.hasError() );
}

// Function to convert from a time to a day,hour,minute,second list
void US_AnaProfParms::timeToList( double& sectime, QList< int >& dhms )
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
void US_AnaProfParms::timeToList( QTime& timeobj, int& days, QList< int >& dhms )
{
   dhms.clear();
   dhms << days
        << timeobj.hour()
        << timeobj.minute()
        << timeobj.second();
}

// Function to convert to a time from a day,hour,minute,second list
void US_AnaProfParms::timeFromList( double& sectime, QList< int >& dhms )
{
   sectime     = dhms[ 0 ] * ( 24 * 60 * 60 ) +
                 dhms[ 1 ] * ( 60 * 60 ) +
                 dhms[ 2 ] * 60 +
                 dhms[ 3 ];
}

// Function to convert to a time from a day,hour,minute,second list
void US_AnaProfParms::timeFromList( QTime& timeobj, int& days,
                                   QList< int >& dhms )
{
   timeobj     = QTime( dhms[ 1 ], dhms[ 2 ], dhms[ 3 ] );
   days        = dhms[ 0 ];
}

// Function to convert from a time to "0d 00:06:30" type string
void US_AnaProfParms::timeToString( double& sectime, QString& strtime )
{
   QList< int > dhms;
   // timeToList( sectime, dhms );         //ALEXEY: bug!!!
   double sectime_to_list = sectime;
   timeToList( sectime_to_list, dhms );
   strtime          = QString().sprintf( "%dd %02d:%02d:%02d",
                         dhms[ 0 ], dhms[ 1 ], dhms[ 2 ], dhms[ 3 ] );
}

// Function to convert from a time to "0d 00:06:30" type string
void US_AnaProfParms::timeToString( QTime& timeobj, int& days, QString& strtime )
{
   strtime          = QString().sprintf( "%dd %02d:%02d:%02d", days,
                         timeobj.hour(), timeobj.minute(), timeobj.second()  );
}

// Function to convert to a time from a "0d 00:06:30" type string
void US_AnaProfParms::timeFromString( double& sectime, QString& strtime )
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
void US_AnaProfParms::timeFromString( QTime& timeobj, int& days,
                                     QString& strtime )
{
   days             = strtime.section( "d", 0, 0 ).toInt();
   int t_hour       = strtime.section( ":", 0, 0 ).right( 2 ).toInt();
   int t_minute     = strtime.section( ":", 1, 1 ).toInt();
   int t_second     = strtime.section( ":", 2, 2 ).toInt();
   timeobj          = QTime( t_hour, t_minute, t_second );
}


// AProfParms2DSA subclass constructor
US_AnaProfParms::AProfParms2DSA::AProfParms2DSA()
{
   nstep                = 1;
   ssteps << SpeedStep();
}

// AProfParms2DSA subclass Equality operator
bool US_AnaProfParms::AProfParms2DSA::operator== 
                  ( const AProfParms2DSA& rp ) const
{
   if ( nstep  != rp.nstep  ) return false;
   if ( ssteps != rp.ssteps ) return false;

   return true;
}

// Read all current Speed controls from an XML stream
bool US_AnaProfParms::AProfParms2DSA::fromXml( QXmlStreamReader& xmli )
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
            QString s_dy = attr.value( "delay" )        .toString();
            QString s_si = attr.value( "scan_interval" ).toString();
            double d_du  = attr.value( "duration_minutes" ).toString().toDouble();
            double d_dy  = attr.value( "delay_seconds"    ).toString().toDouble();

            if ( ! s_du.isEmpty() )
               US_AnaProfParms::timeFromString( ss.duration, s_du );
            else
               ss.duration  = d_du * 60.0;

            if ( ! s_dy.isEmpty() )
               US_AnaProfParms::timeFromString( ss.delay, s_dy );
            else
               ss.delay     = d_dy;

            US_AnaProfParms::timeFromString( ss.scanintv, s_si );

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
bool US_AnaProfParms::AProfParms2DSA::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "speed" );
   xmlo.writeAttribute( "spin_down",          spin_down    ? "1" : "0" );
   xmlo.writeAttribute( "radial_calibration", radial_calib ? "1" : "0" );

   for ( int ii = 0; ii < ssteps.count(); ii++ )
   {
     qDebug() << "SPEED toXml ssteps[ii].duration, ssteps[ii].scanintv  0 : " << ssteps[ii].duration << ", "<< ssteps[ii].scanintv;
      QString s_durat;
      QString s_delay;
      QString s_sintv;
      US_AnaProfParms::timeToString( ssteps[ ii ].duration, s_durat );
      US_AnaProfParms::timeToString( ssteps[ ii ].delay,    s_delay );
      US_AnaProfParms::timeToString( ssteps[ ii ].scanintv, s_sintv );

      qDebug() << "SPEED toXml ssteps[ii].duration, ssteps[ii].scanintv  1 : " << ssteps[ii].duration << ", "<< ssteps[ii].scanintv;

      xmlo.writeStartElement( "speedstep" );
      xmlo.writeAttribute   ( "rotorspeed",
                              QString::number( ssteps[ ii ].speed ) );
      xmlo.writeAttribute   ( "acceleration",
                              QString::number( ssteps[ ii ].accel ) );
      xmlo.writeAttribute   ( "duration",      s_durat );
      xmlo.writeAttribute   ( "delay",         s_delay );
      xmlo.writeAttribute   ( "scan_interval", s_sintv );
      xmlo.writeEndElement  (); // speedstep

      // qDebug() << "SPEED toXml 2 : " << ap2DSA->ssteps[ii].duration;
   }
   xmlo.writeEndElement();    // speed

   //qDebug() << "SPEED toXml 3 : " << ap2DSA->ssteps[ii].duration;

   return ( ! xmlo.hasError() );
}

// AProfParms2DSA::SpeedStep subclass constructor
US_AnaProfParms::AProfParms2DSA::SpeedStep::SpeedStep()
{
   speed       = 45000.0;
   accel       = 400.0;
   //duration    = 330.0;
   duration    = 5*3600 + 30*60;  // 5h 30 min - total in seconds
   delay       = 120.0;
   delay_stage = 0.0;
   scanintv    = 16.85;             //ALEXEY: set default scanint (in secs) which corresponds to 45000 RPM
   scanintv_min = scanintv;
}

// AProfParms2DSA::SpeedStep subclass Equality operator
bool US_AnaProfParms::AProfParms2DSA::SpeedStep::operator==
                  ( const SpeedStep& ss ) const
{
   if ( speed    != ss.speed    ) return false;
   if ( accel    != ss.accel    ) return false;
   if ( duration != ss.duration ) return false;
   if ( delay    != ss.delay    ) return false;
   if ( scanintv != ss.scanintv ) return false;

   return true;
}


// AProfParmsPCSA subclass constructor
US_AnaProfParms::AProfParmsPCSA::AProfParmsPCSA()
{
   ncell                = 0;
   nused                = 0;
   used.clear();
}

// AProfParmsPCSA subclass Equality operator
bool US_AnaProfParms::AProfParmsPCSA::operator== 
                  ( const AProfParmsPCSA& rp ) const
{
   if ( ncell  != rp.ncell ) return false;
   if ( nused  != rp.nused ) return false;
   if ( used   != rp.used  ) return false;

   return true;
}

// Read all current Cells controls from an XML stream
bool US_AnaProfParms::AProfParmsPCSA::fromXml( QXmlStreamReader& xmli )
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
bool US_AnaProfParms::AProfParmsPCSA::toXml( QXmlStreamWriter& xmlo )
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
	if ( used[ ii ].cbalance.contains( "centerpiece"  ) )
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

// AProfParmsPCSA::CellUse subclass constructor
US_AnaProfParms::AProfParmsPCSA::CellUse::CellUse()
{
   cell                 = 0;
   centerpiece          = _TR_( "empty" );
   windows              = _TR_( "quartz" );
   cbalance             = _TR_( "empty (counterbalance)" );
}

// AProfParmsPCSA::CellUse subclass Equality operator
bool US_AnaProfParms::AProfParmsPCSA::CellUse::operator==
                  ( const CellUse& c ) const
{
   if ( cell        != c.cell        ) return false;
   if ( centerpiece != c.centerpiece ) return false;
   if ( windows     != c.windows     ) return false;
   if ( cbalance    != c.cbalance    ) return false;

   return true;
}


#if 0
// AProfParmsStatus subclass constructor
US_AnaProfParms::AProfParmsStatus::AProfParmsStatus()
{
   us_xml .clear();
}

// AProfParmsStatus subclass Equality operator
bool US_AnaProfParms::AProfParmsStatus::operator== 
                  ( const AProfParmsStatus& u ) const
{
   if ( us_xml  != u.us_xml  ) return false;

   return true;
}
#endif

