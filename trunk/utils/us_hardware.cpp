//! \file us_hardware.cpp
#include "us_hardware.h"
#include "us_settings.h"

// Read in and build a vector of centerpiece information
bool US_Hardware::readCenterpieceInfo( QVector< CenterpieceInfo >& cp_list )
{
   //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QString home = US_Settings::appBaseDir();

   QFile cp_file( home + "/etc/centerpiece.dat");

   if ( cp_file.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts( &cp_file );
      
      while ( ! ts.atEnd() )
      {
         QString s = ts.readLine();          // Process line by line

         bool            ok;
         CenterpieceInfo cp;
         
         cp.serial_number = US_Util::getToken( s, " " ).toInt( &ok );
         
         // If the first token is not an integer, 
         // we got a comment and we'll skip this line
         if ( ok )  
         {
            cp.material = US_Util::getToken( s, " " );
            cp.channels = US_Util::getToken( s, " ").toInt();;

            for ( int i = 0; i < cp.channels; i++ )
               cp.bottom_position[ i ] = US_Util::getToken( s, " " ).toFloat();

            cp.sector     = US_Util::getToken( s, " " ).toInt();
            cp.pathlength = US_Util::getToken( s, " " ).toFloat();
            cp.angle      = US_Util::getToken( s, " " ).toFloat();
            cp.width      = US_Util::getToken( s, " " ).toFloat();
            
            cp_list .push_back( cp );
         }
      }

      cp_file.close();
      return true;
   }


   return false;
}

// Read rotor information and generate a rotorCalID,coeffs map from Local
bool US_Hardware::readRotorMap( QMap< QString, QString >& rotor_map )
{
   bool    ok    = false;
   QString path  = US_Settings::dataDir() + "/rotors";

   rotor_map.clear();
   rotor_map[ "0" ] = "0 0";   // Create a simulation entry (no stretch)

   QDir    dir( path );

   if ( !dir.exists() )
      return ok;

   QStringList filter( "C*.xml" );
   QStringList fnames = dir.entryList( filter, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < fnames.size(); ii++ )
   {  // Browse the calibration files
      QFile file( path + "/" + fnames[ ii ] );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         qDebug() << "*ERROR: Could not read rotor file\n" << fnames[ ii ];
         continue;
      }

      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {  // Get ID and coefficients from a file
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "Calibration" )
         {
            QXmlStreamAttributes att = xml.attributes();

            QString calID      = att.value( "id"  ).toString();
            QString coeffs     = att.value( "coeff1" ).toString() + " "
                               + att.value( "coeff2" ).toString();

            rotor_map[ calID ] = coeffs;
            ok                 = true;
         }
      }

   }

   return ok;
}

// Read rotor information and generate a rotorCalID,coeffs map from DB
bool US_Hardware::readRotorMap( US_DB2* db,
                                QMap< QString, QString >& rotor_map )
{
   bool    ok    = false;

   if ( db == 0  ||  ! db->isConnected() )
      return readRotorMap( rotor_map );

   rotor_map.clear();
   rotor_map[ "0" ] = "0 0";   // Create a simulation entry (no stretch)

   QStringList query;
   QStringList labIDs;
   QStringList rotorIDs;
   QStringList calibIDs;

   query << "get_lab_names";
   db->query( query );

   if ( db->lastErrno() != US_DB2::OK )
   {
      qDebug() << "*ERROR* Unable to get lab IDs";
      return ok;
   }

   while ( db->next() )          // Get a list of lab IDs
   {
      labIDs << db->value( 0 ).toString();   // Add a lab ID
   }

   for ( int ii = 0; ii < labIDs.size(); ii++ )
   {  // Add to the list of rotor IDs, from each lab
      QString labID = labIDs[ ii ];

      query.clear();
      query << "get_rotor_names" << labID;
      db->query( query );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "*WARNING* Unable to get rotors for lab ID" << labID;
         continue;
      }

      while ( db->next() )
      {
         rotorIDs << db->value( 0 ).toString();  // Add rotor ID
      }
   }

   for ( int ii = 0; ii < rotorIDs.size(); ii++ )
   {  // Add to the list of calibration IDs, from each rotor
      QString rotorID = rotorIDs[ ii ];

      query.clear();
      query << "get_rotor_calibration_profiles" << rotorID;
      db->query( query );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "*WARNING* Unable to get calIDs for rotor ID" << rotorID;
         continue;
      }

      while ( db->next() )
      {
         calibIDs << db->value( 0 ).toString();  // Add a calibration ID
      }
   }

   for ( int ii = 0; ii < calibIDs.size(); ii++ )
   {  // Build coefficient pairs mapped to each calibration ID
      QString calibID = calibIDs[ ii ];

      query.clear();
      query << "get_rotor_calibration_info" << calibID;
      db->query( query );

      if ( db->lastErrno() != US_DB2::OK )
      {
         qDebug() << "*WARNING* Unable to get info for calib ID" << calibID;
         continue;
      }

      while ( db->next() )
      {  // Map this coefficient pair to a calibration ID
         QString coeffs = db->value( 4 ).toString() + " " +
                          db->value( 5 ).toString();

         rotor_map[ calibID ] = coeffs;
         ok                   = true;     // Mark that at least one pair found
      }
   }
#if 0
//*DEBUG*
QStringList cpieces;
query << "get_abstractCenterpiece_names";
db->query( query );
if ( db->lastErrno() != US_DB2::OK )
 { qDebug() << "*NOTE* Unable to get centerpiece names"; return true; }
while ( db->next() )
 cpieces << db->value(0).toString();
qDebug() << "HW:rdRMap: cpieces size" << cpieces.size();
for ( int ii=0;ii<cpieces.size();ii++ )
{
 QString cpID=cpieces[ii];
 query.clear();
 query << "get_abstractCenterpiece_info" << cpID;
 db->query(query);
 if( db->next() )
 {
   qDebug() << " cpID" << cpID
    << "guid"     << db->value(0).toString()
    << "name"     << db->value(1).toString()
    << "channels" << db->value(2).toString()
    << "bottom"   << db->value(3).toString();
   qDebug()
    << "   shape"   << db->value(4).toString()
    << "maxRPM"     << db->value(5).toString()
    << "pathLength" << db->value(6).toString()
    << "angle"      << db->value(7).toString()
    << "width"      << db->value(8).toString();
 }
}
#endif

   return ok;
}

// Get rotor coefficients for a given rotor calibration identifier
bool US_Hardware::rotorValues( QString rCalID,
      QMap< QString, QString >rotor_map, double* rotcoeffs )
{
   bool ok = rotor_map.contains( rCalID );   // Flag:  calibration ID found?

   if ( ok )
   {  // Return the coefficient pair for the given calibration ID
      QString coeffs = rotor_map[ rCalID ];
      rotcoeffs[ 0 ] = coeffs.section( " ", 0, 0 ).toDouble();
      rotcoeffs[ 1 ] = coeffs.section( " ", 1, 1 ).toDouble();
   }

   return ok;
}

