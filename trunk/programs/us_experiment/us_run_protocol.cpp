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
}

// RunProtocol Equality operator
bool US_RunProtocol::operator== ( const US_RunProtocol& rp ) const
{
   if ( investigator != rp.investigator )  return false;
   if ( protname     != rp.protname     )  return false;
   if ( pGUID        != rp.pGUID        )  return false;
   if ( optimahost   != rp.optimahost   )  return false;
   if ( temperature  != rp.temperature  )  return false;

   if ( rpRotor      != rp.rpRotor      )  return false;
   if ( rpSpeed      != rp.rpSpeed      )  return false;
   if ( rpCells      != rp.rpCells      )  return false;
   if ( rpSolut      != rp.rpSolut      )  return false;
   if ( rpOptic      != rp.rpOptic      )  return false;
   if ( rpSpect      != rp.rpSpect      )  return false;

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

   rpRotor.toXml( xmlo );
   rpSpeed.toXml( xmlo );
   rpCells.toXml( xmlo );
   rpSolut.toXml( xmlo );
   rpOptic.toXml( xmlo );
   rpSpect.toXml( xmlo );

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
         }

         else if ( ename == "rotor" )      { rpRotor.fromXml( xmli ); }
         else if ( ename == "speed" )      { rpSpeed.fromXml( xmli ); }
         else if ( ename == "cells" )      { rpCells.fromXml( xmli ); }
         else if ( ename == "solutions" )  { rpSolut.fromXml( xmli ); }
         else if ( ename == "optics" )     { rpOptic.fromXml( xmli ); }
         else if ( ename == "spectra" )    { rpSpect.fromXml( xmli ); }
      }
   }

   return ( ! xmli.hasError() );
}

// Load a protocol from DB (by GUID)
int US_RunProtocol::load_db( const QString& guid, US_DB2* db )
{
   QStringList qry;

   qry << "get_protocolID" << guid;
   db->query( qry );

   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();
   
   db->next();
   QString id = db->value( 0 ).toString();
   //return load( id, db );
   return id.toInt();
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
   int nspeed      = 0;
   ssteps.clear();

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
            ss.duration  = attr.value( "duration_minutes" ).toString().toDouble();
            ss.delay     = attr.value( "delay_seconds"    ).toString().toDouble();
            ssteps << ss;
            nspeed++;
         }

         else if ( ename != "speed" )
            break;
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "speed" )    // Break after "</speed>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current Speed portion of controls to an XML stream
bool US_RunProtocol::RunProtoSpeed::toXml( QXmlStreamWriter& xmlo )
{
   xmlo.writeStartElement( "speed" );
   for ( int ii = 0; ii < ssteps.count(); ii++ )
   {
      xmlo.writeStartElement( "speedstep" );
      xmlo.writeAttribute( "rotorspeed",
                           QString::number( ssteps[ ii ].speed    ) );
      xmlo.writeAttribute( "acceleration",
                           QString::number( ssteps[ ii ].accel    ) );
      xmlo.writeAttribute( "duration_minutes",
                           QString::number( ssteps[ ii ].duration ) );
      xmlo.writeAttribute( "delay_seconds",
                           QString::number( ssteps[ ii ].delay    ) );
      xmlo.writeEndElement(); // speedstep
   }
   xmlo.writeEndElement();    // speed

   return ( ! xmlo.hasError() );
}

// RunProtoSpeed::SpeedStep subclass constructor
US_RunProtocol::RunProtoSpeed::SpeedStep::SpeedStep()
{
   speed       = 45000.0;
   accel       = 400.0;
   duration    = 330.0;
   delay       = 120.0;
}

// RunProtoSpeed::SpeedStep subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::SpeedStep::operator==
                  ( const SpeedStep& ss ) const
{
   if ( speed    != ss.speed    ) return false;
   if ( accel    != ss.accel    ) return false;
   if ( duration != ss.duration ) return false;
   if ( delay    != ss.delay    ) return false;

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
      if ( ( ii + 1 ) < nused )
      {
         xmlo.writeAttribute( "centerpiece",    used[ ii ].centerpiece );
         xmlo.writeAttribute( "windows",        used[ ii ].windows     );
      }
      else
      {
         xmlo.writeAttribute( "counterbalance", used[ ii ].cbalance    );
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


// RunProtoSpectra subclass constructor
US_RunProtocol::RunProtoSpectra::RunProtoSpectra()
{
   nspect               = 0;
   chspecs.clear();
}

// RunProtoSpectra subclass Equality operator
bool US_RunProtocol::RunProtoSpectra::operator== 
                  ( const RunProtoSpectra& rp ) const
{
   if ( nspect  != rp.nspect  ) return false;
   if ( chspecs != rp.chspecs ) return false;

   return true;
}

// Read all current Spectra controls from an XML stream
bool US_RunProtocol::RunProtoSpectra::fromXml( QXmlStreamReader& xmli )
{
   nspect               = 0;
   chspecs.clear();
   Spectrum sp;

   while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "spectrum" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            sp.channel           = attr.value( "channel" ).toString();
            QString fl_auto      = attr.value( "auto"    ).toString();
            QString fl_load      = attr.value( "load"    ).toString();
            QString fl_manual    = attr.value( "manual"  ).toString();

            if ( fl_auto   == "1" )   sp.typeinp   = "auto";
            if ( fl_load   == "1" )   sp.typeinp   = "load";
            if ( fl_manual == "1" )   sp.typeinp   = "manual";

            sp.lambdas.clear();
            sp.values .clear();
         }

         else if ( ename == "point" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            sp.lambdas << attr.value( "lambda" ).toString().toDouble();
            sp.values  << attr.value( "value"  ).toString().toDouble();
         }
      }

      else if ( xmli.isEndElement()  &&  ename == "spectrum" )
      {
         chspecs << sp;
         nspect++;
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "spectra" )  // Break after "</spectra>"
         break;
   }

   return ( ! xmli.hasError() );
}

// Write the current Spectra portion of controls to an XML stream
bool US_RunProtocol::RunProtoSpectra::toXml( QXmlStreamWriter& xmlo )
{
   nspect               = chspecs.count();
   xmlo.writeStartElement( "spectra" );
   for ( int ii = 0; ii < nspect; ii++ )
   {
      QString inpt         = chspecs[ ii ].typeinp;
      xmlo.writeStartElement( "spectrum" );
      xmlo.writeAttribute   ( "channel", chspecs[ ii ].channel );
      xmlo.writeAttribute   ( "auto",   ( inpt == "auto"   ) ? "1" : "0" );
      xmlo.writeAttribute   ( "load",   ( inpt == "load"   ) ? "1" : "0" );
      xmlo.writeAttribute   ( "manual", ( inpt == "manual" ) ? "1" : "0" );
      for ( int jj = 0; jj < chspecs[ ii ].lambdas.count(); jj++ )
      {
         xmlo.writeStartElement( "point" );
         xmlo.writeAttribute( "lambda",
                              QString::number( chspecs[ ii ].lambdas[ jj ] ) );
         xmlo.writeAttribute( "value",
                              QString::number( chspecs[ ii ].values [ jj ] ) );
         xmlo.writeEndElement(); // point
      }
      xmlo.writeEndElement(); // spectrum
   }
   xmlo.writeEndElement(); // spectra

   return ( ! xmlo.hasError() );
}

// RunProtoSpectra::Spectrum subclass constructor
US_RunProtocol::RunProtoSpectra::Spectrum::Spectrum()
{
   channel .clear();
   typeinp .clear();
   lambdas .clear();
   values  .clear();
}

// RunProtoSpectra::Spectrum subclass Equality operator
bool US_RunProtocol::RunProtoSpectra::Spectrum::operator==
                  ( const Spectrum& s ) const
{
   if ( channel     != s.channel     ) return false;
   if ( typeinp     != s.typeinp     ) return false;
   if ( lambdas     != s.lambdas     ) return false;
   if ( values      != s.values      ) return false;

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

