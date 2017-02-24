//! \file us_run_protocol.cpp

#include "us_run_protocol.h"
#include "us_constants.h"
#include "us_settings.h"
#include "us_util.h"

// RunProtocol constructor
US_RunProtocol::US_RunProtocol()
{
   pname           = "new protocol";
   pGUID           = "";
   optimahost      = "192.168.1.1";
   investigator    = "Stephen Hawking";
   temperature     = 20.0;
}

// RunProtocol Equality operator
bool US_RunProtocol::operator== ( const US_RunProtocol& rp ) const
{
   if ( pname         != rp.pname         ) return false;
   if ( pGUID         != rp.pGUID         ) return false;
   if ( optimahost    != rp.optimahost    ) return false;
   if ( investigator  != rp.investigator  ) return false;
   if ( temperature   != rp.temperature   ) return false;

   if ( rpRotor       != rp.rpRotor       ) return false;
   if ( rpSpeed       != rp.rpSpeed       ) return false;
   if ( rpCells       != rp.rpCells       ) return false;
   if ( rpSolut       != rp.rpSolut       ) return false;
   if ( rpSpect       != rp.rpSpect       ) return false;
   if ( rpUload       != rp.rpUload       ) return false;

   return true;
}

// Store a control set's double parameters
bool US_RunProtocol::storeDControls( const QString type,
                                     QMap< QString, double >& parmap )
{
qDebug() << "type" << type << "ln-map" << parmap.keys().count();
   return true;
}

// Store a control set's string parameters
bool US_RunProtocol::storeSControls( const QString type,
                                     QMap< QString, QString >& parmap )
{
qDebug() << "type" << type << "ln-map" << parmap.keys().count();
   return true;
}

// Load a protocol from DB (by GUID)
int US_RunProtocol::load_db( const QString& guid, US_DB2* db )
{
   QStringList q;

   q << "get_protocolID" << guid;
   db->query( q );

   if ( db->lastErrno() != US_DB2::OK ) return db->lastErrno();
   
   db->next();
   QString id = db->value( 0 ).toString();
   //return load( id, db );
   return id.toInt();
}


// RunProtoRotor subclass constructor
US_RunProtocol::RunProtoRotor::RunProtoRotor()
{
   lab                  = "New Lab";
   rotor                = "New Rotor";
   calibration          = "New Calibration";
   labGUID              = QString( "00000000-0000-0000-0000-000000000000" );
   rotorGUID            = labGUID;
   calibGUID            = labGUID;
   absrotGUID           = labGUID;
   labID                = -1;
   rotorID              = -1;
   calibrationID        = -1;
   absRotorID           = -1;
}

// RunProtoRotor subclass Equality operator
bool US_RunProtocol::RunProtoRotor::operator== 
                  ( const RunProtoRotor& rp ) const
{
   if ( labGUID        != rp.labGUID        ) return false;
   if ( rotorGUID      != rp.rotorGUID      ) return false;
   if ( calibGUID      != rp.calibGUID      ) return false;
   if ( absrotGUID     != rp.absrotGUID     ) return false;

   return true;
}


// RunProtoSpeed subclass constructor
US_RunProtocol::RunProtoSpeed::RunProtoSpeed()
{
   nstep                = 1;
   SpeedStep ss;
   ssteps << ss;
}

// RunProtoSpeed subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::operator== 
                  ( const RunProtoSpeed& rp ) const
{
   if ( nstep  != rp.nstep  ) return false;
   if ( ssteps != rp.ssteps ) return false;

   return true;
}

// RunProtoSpeed::SpeedStep subclass constructor
US_RunProtocol::RunProtoSpeed::SpeedStep::SpeedStep()
{
   rotorspeed           = 45000.0;
   accel                = 400.0;
   duration             = 330.0;
   delay                = 120.0;
}

// RunProtoSpeed::SpeedStep subclass Equality operator
bool US_RunProtocol::RunProtoSpeed::SpeedStep::operator==
                  ( const SpeedStep& ss ) const
{
   if ( rotorspeed  != ss.rotorspeed ) return false;
   if ( accel       != ss.accel      ) return false;
   if ( duration    != ss.duration   ) return false;
   if ( delay       != ss.delay      ) return false;

   return true;
}


// RunProtoCells subclass constructor
US_RunProtocol::RunProtoCells::RunProtoCells()
{
   ncell                = 0;
   nused                = 0;
   all .clear();
   used.clear();
}

// RunProtoCells subclass Equality operator
bool US_RunProtocol::RunProtoCells::operator== 
                  ( const RunProtoCells& rp ) const
{
   if ( ncell  != rp.ncell ) return false;
   if ( nused  != rp.nused ) return false;
   if ( all    != rp.all   ) return false;
   if ( used   != rp.used  ) return false;

   return true;
}

// RunProtoCells::CellUse subclass constructor
US_RunProtocol::RunProtoCells::CellUse::CellUse()
{
   cell                 = 0;
   centerpiece.clear();
   windows    .clear();
   counterbal .clear();
}

// RunProtoCells::CellUse subclass Equality operator
bool US_RunProtocol::RunProtoCells::CellUse::operator==
                  ( const CellUse& c ) const
{
   if ( cell        != c.cell        ) return false;
   if ( centerpiece != c.centerpiece ) return false;
   if ( windows     != c.windows     ) return false;
   if ( counterbal  != c.counterbal  ) return false;

   return true;
}


// RunProtoSolutions subclass constructor
US_RunProtocol::RunProtoSolutions::RunProtoSolutions()
{
   nschan               = 0;
   nuniqs               = 0;
   chsols.clear();
   solus .clear();
   sguids.clear();
   sids  .clear();
}

// RunProtoSolutions subclass Equality operator
bool US_RunProtocol::RunProtoSolutions::operator== 
                  ( const RunProtoSolutions& rp ) const
{
   if ( nschan != rp.nschan ) return false;
   if ( nuniqs != rp.nuniqs ) return false;
   if ( chsols != rp.chsols ) return false;
   if ( sguids != rp.sguids ) return false;

   return true;
}

// RunProtoSolutions::ChanSolu subclass constructor
US_RunProtocol::RunProtoSolutions::ChanSolu::ChanSolu()
{
   channel .clear();
   solution.clear();
   comment .clear();
}

// RunProtoSolutions::ChanSolu subclass Equality operator
bool US_RunProtocol::RunProtoSolutions::ChanSolu::operator==
                  ( const ChanSolu& c ) const
{
   if ( channel     != c.channel     ) return false;
   if ( solution    != c.solution    ) return false;
   if ( comment     != c.comment     ) return false;

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

