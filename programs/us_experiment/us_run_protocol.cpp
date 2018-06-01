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
   protname        = "";
   pGUID           = QString( "00000000-0000-0000-0000-000000000000" );
   optimahost      = "192.168.1.1";
   investigator    = "";
   temperature     = 20.0;
   temeq_delay     = 10.0;
}

// RunProtocol Equality operator
bool US_RunProtocol::operator== ( const US_RunProtocol& rp ) const
{
   if ( investigator != rp.investigator )  return false;
   if ( protname     != rp.protname     )  return false;
   if ( pGUID        != rp.pGUID        )  return false;
   if ( optimahost   != rp.optimahost   )  return false;
   if ( temperature  != rp.temperature  )  return false;
   if ( temeq_delay  != rp.temeq_delay  )  return false;

   if ( rpRotor      != rp.rpRotor      )  return false;
   if ( rpSpeed      != rp.rpSpeed      )  return false;
   if ( rpCells      != rp.rpCells      )  return false;
   if ( rpSolut      != rp.rpSolut      )  return false;
   if ( rpOptic      != rp.rpOptic      )  return false;
   if ( rpRange      != rp.rpRange      )  return false;

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
   xmlo.writeAttribute    ( "description",  protname );
   xmlo.writeAttribute    ( "guid",         pGUID );
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
            protname        = attr.value( "description"  ).toString();
            pGUID           = attr.value( "guid"         ).toString();
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
      }
   }

   return ( ! xmli.hasError() );
}

// Function to convert from a time to a day,hour,minute,second list
void US_RunProtocol::timeToList( double& sectime, QList< int >& dhms )
{
   int t_minute     = (int)( sectime / 60.0 );
   int t_second     = qRound( sectime - t_minute * 60.0 );
   int t_hour       = (int)( t_minute / 60 );
       t_minute    -= ( t_hour * 60 );
   int t_day        = (int)( t_hour / 24 );
       t_day       -= ( t_day * 24 );

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
   timeToList( sectime, dhms );
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
   if ( labGUID != rp.labGUID ) return false;
   if ( rotGUID != rp.rotGUID ) return false;
   if ( calGUID != rp.calGUID ) return false;
   if ( absGUID != rp.absGUID ) return false;

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
            QString s_dy = attr.value( "delay" )        .toString();
            QString s_si = attr.value( "scan_interval" ).toString();
            double d_du  = attr.value( "duration_minutes" ).toString().toDouble();
            double d_dy  = attr.value( "delay_seconds"    ).toString().toDouble();

            if ( ! s_du.isEmpty() )
               US_RunProtocol::timeFromString( ss.duration, s_du );
            else
               ss.duration  = d_du * 60.0;

            if ( ! s_dy.isEmpty() )
               US_RunProtocol::timeFromString( ss.delay, s_dy );
            else
               ss.delay     = d_dy;

            US_RunProtocol::timeFromString( ss.scanintv, s_si );

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
      QString s_durat;
      QString s_delay;
      QString s_sintv;
      US_RunProtocol::timeToString( ssteps[ ii ].duration, s_durat );
      US_RunProtocol::timeToString( ssteps[ ii ].delay,    s_delay );
      US_RunProtocol::timeToString( ssteps[ ii ].scanintv, s_sintv );

      xmlo.writeStartElement( "speedstep" );
      xmlo.writeAttribute   ( "rotorspeed",
                              QString::number( ssteps[ ii ].speed ) );
      xmlo.writeAttribute   ( "acceleration",
                              QString::number( ssteps[ ii ].accel ) );
      xmlo.writeAttribute   ( "duration",      s_durat );
      xmlo.writeAttribute   ( "delay",         s_delay );
      xmlo.writeAttribute   ( "scan_interval", s_sintv );
      xmlo.writeEndElement  (); // speedstep
   }
   xmlo.writeEndElement();    // speed

   return ( ! xmlo.hasError() );
}

// RunProtoSpeed::SpeedStep subclass constructor
US_RunProtocol::RunProtoSpeed::SpeedStep::SpeedStep()
{
   speed       = 45000.0;
   accel       = 400.0;
   //duration    = 330.0;
   duration    = 5*3600 + 30*60;  // 5h 30 min - total in seconds
   delay       = 120.0;
   delay_stage = 0.0;
   scanintv    = 16.85;             //ALEXEY: set default scanint (in secs) which corresponds to 45000 RPM
}

// RunProtoSpeed::SpeedStep subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::SpeedStep::operator==
                  ( const SpeedStep& ss ) const
{
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
   if ( nused  != rp.nused ) return false;
   if ( used   != rp.used  ) return false;

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
	xmlo.writeAttribute( "counterbalance", used[ ii ].cbalance    );   // ALEXEY: Potential BUG: WHY last cell is always counterbalance ?? 
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
   if ( cell        != c.cell        ) return false;
   if ( centerpiece != c.centerpiece ) return false;
   if ( windows     != c.windows     ) return false;
   if ( cbalance    != c.cbalance    ) return false;

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
   if ( nschan != rp.nschan ) return false;
   if ( nuniqs != rp.nuniqs ) return false;
   if ( chsols != rp.chsols ) return false;
   if ( sids   != rp.sids   ) return false;

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
   if ( chopts != rp.chopts ) return false;

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
   if ( chrngs  != rp.chrngs )   return false;

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
   lo_rad               = 5.8;
   hi_rad               = 7.2;
}

// RunProtoRanges::Ranges subclass Equality operator
bool US_RunProtocol::RunProtoRanges::Ranges::operator==
                  ( const Ranges& s ) const
{
   if ( channel     != s.channel     ) return false;
   if ( wvlens      != s.wvlens      ) return false;
   if ( lo_rad      != s.lo_rad      ) return false;
   if ( hi_rad      != s.hi_rad      ) return false;

   return true;
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

