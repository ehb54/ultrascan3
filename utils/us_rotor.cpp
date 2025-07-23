//! \file us_rotor_gui.cpp

#include "us_rotor.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_db2.h"

// The constructor clears out the data structure
US_Rotor::US_Rotor()
{
}

// A function to read information about all labs
US_Rotor::Status US_Rotor::readLabsDB( 
    QVector< US_Rotor::Lab >& labList, US_DB2* db )
{
   QStringList q( "get_lab_names" );
   db->query( q );

   QStringList labIDs;
   while ( db->next() )
      labIDs << db->value( 0 ).toString();

   if ( labIDs.size() == 0 )
      return NOT_FOUND;

   // Save information about all the labs
   labList.clear();
   for ( int i = 0; i < labIDs.size(); i++ )
   {
      US_Rotor::Lab lab;

      lab.readDB( labIDs[ i ].toInt(), db );
      labList.push_back( lab );
   }

   // Since we're not really editing labs, let's just copy
   // the information to disk directly
   saveLabsDisk( labList );

   return ROTOR_OK;
}

// Function to save abstract rotor information to disk
void US_Rotor::saveLabsDisk( QVector< US_Rotor::Lab >& labList )
{
   // Currently labs.xml is in with the rotors 
   QString path; 
   if ( ! diskPath( path ) ) return; 
   
   QString filename = path + "/labs.xml"; 
   
   QFile file( filename );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for writing"
               << filename;
      return;
   }

   // Generate xml
   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Lab>");
   xml.writeStartElement("LabData");
   xml.writeAttribute("version", "1.0");

   // Loop through all the labs
   for ( int i = 0; i < labList.size(); i++ )
   {
      US_Rotor::Lab lab = labList[ i ];

      xml.writeStartElement( "lab" );
      xml.writeAttribute   ( "id",           QString::number( lab.ID              ) );
      xml.writeAttribute   ( "guid",                          lab.GUID              );
      xml.writeAttribute   ( "name",                          lab.name              );
      xml.writeAttribute   ( "building",                      lab.building          );
      xml.writeAttribute   ( "room",                          lab.room              );

      foreach ( US_Rotor::Instrument instrument, lab.instruments )
      {
         xml.writeStartElement( "instrument" );
         xml.writeAttribute   ( "id",        QString::number( instrument.ID )   );
         xml.writeAttribute   ( "name",                       instrument.name   );
         xml.writeAttribute   ( "serial",                     instrument.serial );

         foreach( US_Rotor::Operator oper, instrument.operators )
         {
            xml.writeStartElement( "operator" );
            xml.writeAttribute   ( "id",     QString::number( oper.ID )  );
            xml.writeAttribute   ( "guid",                    oper.GUID  );
            xml.writeAttribute   ( "lname",                   oper.lname );
            xml.writeAttribute   ( "fname",                   oper.fname );
            xml.writeEndElement  ();    // Operator
         }

         xml.writeEndElement  ();       // Instrument
      }

      xml.writeEndElement  ();          // Lab
   }

   xml.writeEndElement  ();             // LabData
   xml.writeEndDocument ();

   file.close();
}

// Function to read all the lab info from disk
US_Rotor::Status US_Rotor::readLabsDisk( QVector< US_Rotor::Lab >& labList )
{
   // Currently labs.xml is in with the rotors 
   // Make sure the rotors directory is there and has something in it
   QString path; 
   if ( ! diskPath( path ) ) return US_Rotor::NOT_FOUND; 
   
   QString labFilename = US_Settings::dataDir() + "/rotors/labs.xml";
   QFile file( labFilename );

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      // copy from etc
      QString defaultFilename = US_Settings::appBaseDir() + "/etc/labs.xml";
      QFile::copy( defaultFilename, labFilename );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
      {
         qDebug() << "Error: Could not read lab file\n"
                  << labFilename;
         return US_Rotor::NOT_FOUND;
      }
   }

   labList.clear();

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "lab" )
         {
            US_Rotor::Lab lab;

            QXmlStreamAttributes a = xml.attributes();
            lab.ID           = a.value( "id"           ).toString().toInt();
            lab.GUID         = a.value( "guid"         ).toString();
            lab.name         = a.value( "name"         ).toString();
            lab.building     = a.value( "building"     ).toString();
            lab.room         = a.value( "room"         ).toString();
            readInstrumentInfo( xml, lab );

            labList.push_back( lab );
         }
      }
   }

   file.close();

   if ( xml.hasError() )
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return US_Rotor::MISC_ERROR;
   }

   return US_Rotor::ROTOR_OK;
}

void US_Rotor::readInstrumentInfo( QXmlStreamReader& xml, US_Rotor::Lab& lab )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "lab" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "instrument" )
         {
            US_Rotor::Instrument instrument;

            QXmlStreamAttributes a = xml.attributes();
            instrument.ID      = a.value( "id"     ).toString().toInt();
            instrument.name    = a.value( "name"   ).toString();
            instrument.serial  = a.value( "serial" ).toString();

            readOperatorInfo( xml, instrument );
            lab.instruments << instrument;
         }
      }
   }
}

void US_Rotor::readOperatorInfo( QXmlStreamReader& xml, US_Rotor::Instrument& instrument )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "instrument" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "operator" )
         {
            US_Rotor::Operator oper;

            QXmlStreamAttributes a = xml.attributes();
            oper.ID      = a.value( "id"    ).toString().toInt();
            oper.GUID    = a.value( "guid"  ).toString();
            oper.lname   = a.value( "lname" ).toString();
            oper.fname   = a.value( "fname" ).toString();

            instrument.operators << oper;
         }
      }
   }
}

// A function to read information about all abstract rotors from DB
US_Rotor::Status US_Rotor::readAbstractRotorsDB( 
    QVector< US_Rotor::AbstractRotor >& arList, US_DB2* db )
{
   QStringList q( "get_abstractRotor_names" );
   db->query( q );

   QStringList arIDs;
   while ( db->next() )
      arIDs << db->value( 0 ).toString();

   if ( arIDs.size() == 0 )
      return NOT_FOUND;

   // Save information about all the abstract rotors
   arList.clear();
   for ( int i = 0; i < arIDs.size(); i++ )
   {
      US_Rotor::AbstractRotor ar;

      ar.readDB( arIDs[ i ].toInt(), db );
      arList.push_back( ar );
   }

   return ROTOR_OK;
}

// Function to read all the abstract rotor info from disk
US_Rotor::Status US_Rotor::readAbstractRotorsDisk( QVector< US_Rotor::AbstractRotor >& arList )
{
   QString filename = US_Settings::appBaseDir() + "/etc/rotors/abstractRotors.xml";
   QFile   file( filename );

   arList.clear();

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: Could not read abstract rotor file\n"
               << filename;
      return US_Rotor::NOT_FOUND;
   }

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "abstractRotor" )
         {
            US_Rotor::AbstractRotor ar;

            QXmlStreamAttributes a = xml.attributes();
            ar.ID           = a.value( "id"           ).toString().toInt();
            ar.GUID         = a.value( "guid"         ).toString();
            ar.name         = a.value( "name"         ).toString();
            ar.material     = a.value( "materialName" ).toString();
            ar.numHoles     = a.value( "numHoles"     ).toString().toInt();
            ar.maxRPM       = a.value( "maxRPM"       ).toString().toInt();
            ar.magnetOffset = a.value( "magnetOffset" ).toString().toFloat();
            ar.cellCenter   = a.value( "cellCenter"   ).toString().toFloat();
            ar.manufacturer = a.value( "manufacturer" ).toString();

            arList.push_back( ar );
         }
      }
   }

   file.close();

   if ( xml.hasError() )
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return US_Rotor::MISC_ERROR;
   }

   return US_Rotor::ROTOR_OK;
}

// Function to read all the abstract rotor info from disk
US_Rotor::Status US_Rotor::readRotorsFromDB( QVector< US_Rotor::Rotor >& rotors,
                                             int labID, US_DB2* db )
{
   QStringList rotorIDs;
   QStringList qry;
   qry << "get_rotor_names" << QString::number( labID );
   db->query( qry );

   while ( db->next() )
   {  // Get the list of rotor IDs for the specified laboratory
      rotorIDs << db->value( 0 ).toString();
   }

   qry.clear();     
   qry << "get_rotor_info" << " ";
   for ( int ii = 0; ii < rotorIDs.count(); ii++ )
   {  // Get detailed profile information for each calibration ID
      QString rotorID = rotorIDs[ ii ];
      qry[ 1 ]        = rotorID;
      db->query( qry );
      db->next();

      US_Rotor::Rotor rotor;
      rotor.ID                 = rotorID.toInt();
      rotor.GUID               = db->value( 0 ).toString();
      rotor.abstractRotorID    = db->value( 4 ).toInt();
      rotor.abstractRotorGUID  = db->value( 5 ).toString();
      rotor.labID              = labID;
      rotor.name               = db->value( 1 ).toString();
      rotor.serialNumber       = db->value( 2 ).toString();

      rotors << rotor;

      //qDebug() << "ROTOR INFO !!!!: " << rotors[ ii ].name << ", " << rotors[ ii ].abstractRotorID <<  ", " << rotors[ ii ].labID << ", "  << rotors[ ii ].serialNumber;
   }

   if ( rotors.size() == 0 )
      return US_Rotor::NOT_FOUND;

   return US_Rotor::ROTOR_OK;
}

// A function to read rotor information concerning a single lab from disk
US_Rotor::Status US_Rotor::readRotorsFromDisk(
         QVector< US_Rotor::Rotor >& rotors, int labID )
{
   // Get the rotor files
   QString path = US_Settings::dataDir() + "/rotors";

   rotors.clear();
   if ( ! diskPath( path ) ) return( US_Rotor::NOT_FOUND );

   QDir        dir( path );
   QStringList filter( "R*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );

   for ( int i = 0; i < names.size(); i++ )
   {
      QFile file( path + "/" + names[ i ] );
      
      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
      {
         qDebug() << "Error: Could not read rotor file\n"
                  << names[ i ];
         continue;
      }
      
      QXmlStreamReader xml( &file );
      
      while ( ! xml.atEnd() )
      {
         xml.readNext();
      
         if ( xml.isStartElement() )
         {
            if ( xml.name() == "Rotor" )
            {
               US_Rotor::Rotor r;
               QXmlStreamAttributes a = xml.attributes();
            
               r.ID                = a.value( "id"                ).toString().toInt();
               r.GUID              = a.value( "guid"              ).toString();
               r.abstractRotorID   = a.value( "abstractRotorID"   ).toString().toInt();
               r.abstractRotorGUID = a.value( "abstractRotorGUID" ).toString();
               r.labID             = a.value( "labID"             ).toString().toInt();
               r.name              = a.value( "name"              ).toString();
               r.serialNumber      = a.value( "serialNumber"      ).toString();

               if ( r.labID == labID )
                  rotors.push_back( r );

            }
         }
      }

      file.close();

      if ( xml.hasError() )
      {
         qDebug() << "Error: xml error: \n"
                  << xml.errorString();
      }

   }

   if ( rotors.size() == 0 )
      return US_Rotor::NOT_FOUND;

   return US_Rotor::ROTOR_OK;
}

// A function to read rotor calibration profile information about a single rotor from DB
US_Rotor::Status US_Rotor::readCalibrationProfilesDB(
         QVector< US_Rotor::RotorCalibration >& profiles, int rotorID, US_DB2* db )
{
   QStringList calibIDs;
   QStringList qry;
   qry << "get_rotor_calibration_profiles" << QString::number( rotorID );
   db->query( qry );

   while ( db->next() )
   {  // Get the list of calibration IDs for the specified rotor
      calibIDs << db->value( 0 ).toString();
   }

   qry.clear();     
   qry << "get_rotor_calibration_info" << QString::number( rotorID );
   for ( int ii = 0; ii < calibIDs.count(); ii++ )
   {  // Get detailed profile information for each calibration ID
      QString calibID = calibIDs[ ii ];
      qry[ 1 ]        = calibID;
      db->query( qry );
      db->next();

      US_Rotor::RotorCalibration rc;
      rc.ID          = calibID.toInt();
      rc.GUID        = db->value( 0 ).toString();
      rc.rotorID     = db->value( 1 ).toInt();
      rc.rotorGUID   = db->value( 2 ).toString();
      rc.calibrationExperimentID   = db->value( 8 ).toInt();
      rc.calibrationExperimentGUID = QString( "" );
      rc.coeff1      = db->value( 4 ).toDouble();
      rc.coeff2      = db->value( 5 ).toDouble();
      rc.label       = db->value( 9 ).toString();
      rc.report      = db->value( 3 ).toString();
      rc.lastUpdated = db->value( 7 ).toDate();
      rc.omega2t     = db->value( 6 ).toDouble();

      profiles << rc;
   }

   if ( profiles.size() == 0 )
      return US_Rotor::NOT_FOUND;

   return US_Rotor::ROTOR_OK;
}

// A function to read rotor calibration profile information about a single rotor from disk
US_Rotor::Status US_Rotor::readCalibrationProfilesDisk(
         QVector< US_Rotor::RotorCalibration >& profiles, int rotorID )
{
   // Get the calibration files
   QString path = US_Settings::dataDir() + "/rotors";

   profiles.clear();
   if ( ! diskPath( path ) ) return( US_Rotor::NOT_FOUND );

   QDir        dir( path );
   QStringList filter( "C*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );

   for ( int i = 0; i < names.size(); i++ )
   {
      QFile file( path + "/" + names[ i ] );
      
      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
      {
         qDebug() << "Error: Could not read rotor calibration file\n"
                  << names[ i ];
         continue;
      }
      
      QXmlStreamReader xml( &file );
      US_Rotor::RotorCalibration rc;
      
      while ( ! xml.atEnd() )
      {
         xml.readNext();
      
         if ( xml.isStartElement() )
         {
            if ( xml.name() == "Calibration" )
            {
               QXmlStreamAttributes a = xml.attributes();
            
               rc.ID                = a.value( "id"                 ).toString().toInt();
               rc.GUID              = a.value( "guid"               ).toString();
               rc.rotorID           = a.value( "rotorID"            ).toString().toInt();
               rc.rotorGUID         = a.value( "rotorGUID"          ).toString();
               rc.calibrationExperimentID   = a.value( "calExpID"   ).toString().toInt();
               rc.calibrationExperimentGUID = a.value( "calExpGUID" ).toString();
               rc.coeff1            = a.value( "coeff1"             ).toString().toFloat();
               rc.coeff2            = a.value( "coeff2"             ).toString().toFloat();
               rc.label             = a.value( "label"              ).toString();
               rc.report            = QString( "" );
               rc.lastUpdated       = QDate::fromString( a.value( "lastUpdated" ).toString(), "yyyy-MM-dd" );
               rc.omega2t           = a.value( "omega2t"            ).toString().toFloat();
            
               rc.readReport( xml );
            }
         }
      }

      file.close();

      if ( xml.hasError() )
      {
         qDebug() << "Error: xml error: \n"
                  << xml.errorString();
      }

      if ( rc.rotorID == rotorID )
         profiles.push_back( rc );
   }

   if ( profiles.size() == 0 )
      return US_Rotor::NOT_FOUND;

   return US_Rotor::ROTOR_OK;
}

// Function to find the file name of a rotor or calibration on disk, if it exists
bool US_Rotor::diskFilename( const QString& fileMask,
                             const QString& lookupTag,
                             const int& lookupID, 
                             QString& filename )
{
   // Get a path and file name
   QString path;
   if ( ! diskPath( path ) )
   {
      qDebug() << "Error: could not create the directory";
      return false;
   }

   QDir        dir( path );
   QStringList filter( fileMask ); // like "R*.xml"
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );
   bool        found = false;

   for ( int i = 0; i < names.size(); i++ )
   {
      filename = path + "/" + names[ i ];
      QFile file( filename );

      if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &file );
      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == lookupTag )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "id" ).toString().toInt() == lookupID ) found = true;
               break;
            }
         }
      }

      file.close();
      if ( found ) break;  // Break out of this loop too
   }

   if ( ! found )
   {
      filename = QString( "" );
      return false;
   }

   return true;
}

// Get the path to the rotor info.  Create it if necessary.
bool US_Rotor::diskPath( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/rotors"; 

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for rotors\n"
                  << path;
         return false;
      }

      // Copy default rotors and rotor calibrations here
      QString defaultDir = US_Settings::appBaseDir() + "/etc/rotors";
      QDir    etcDir  ( defaultDir );
      QStringList files = etcDir.entryList( QDir::NoDotAndDotDot | QDir::Files );

      foreach( QString file, files )
         QFile::copy( defaultDir + "/" + file, path + "/" + file );
   }

   return true;
}

// Function to check if filename already exists, and perhaps generate a new one
QString US_Rotor::get_filename(
      const QString& path, const QString& fileMask,
      const QString& lookupTag, const int& lookupID, bool& newFile )
{
   QDir        f( path );
   QStringList filter( fileMask );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );
   QString     filename;
   newFile = true;

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile b_file( path + "/" + f_names[ i ] );

      if ( ! b_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &b_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == lookupTag )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "id" ).toString().toInt() == lookupID )
               {
                  newFile  = false;
                  filename = path + "/" + f_names[ i ];
               }

               break;
           }
         }
      }

      b_file.close();
      if ( ! newFile ) return filename;
   }

   // If we get here, generate a new filename
   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;

   QString startName = "/" + fileMask.left( 1 );  // for instance "/R" for rotors
   return path + startName + QString( "%1" ).arg( number + 1, 7, 10, QChar( '0' ) ) + ".xml";
}

US_Rotor::Lab::Lab()
{
   reset();
}

// A function to read information about a single lab from DB
US_Rotor::Status US_Rotor::Lab::readDB( int labID, US_DB2* db )
{
   // Try to get lab info
   QStringList q( "get_lab_info" );
   q  << QString::number( labID );
   db->query( q );
   int readStatus = db->lastErrno();

   if ( readStatus == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( readStatus != US_DB2::OK )
      return MISC_ERROR;

   db->next();
   this->ID          = labID;
   GUID              = db->value( 0 ).toString();
   name              = db->value( 1 ).toString();
   building          = db->value( 2 ).toString();
   room              = db->value( 3 ).toString();
  
   // Now try to get instrument info for this lab
   instruments.clear();
   q.clear();
   q  << QString( "get_instrument_names" )
      << QString::number( labID );
   db->query( q );
   readStatus = db->lastErrno();

   if ( readStatus == US_DB2::OK )      // If not, no instruments defined
   {
      QList< int > instrumentIDs;

      // Grab all the IDs so we can reuse the db connection
      while ( db->next() )
      {
         int ID = db->value( 0 ).toString().toInt();
         instrumentIDs << ID;
      }

      // Instrument information
      foreach ( int ID, instrumentIDs )
      {
         US_Rotor::Instrument instrument;

         q.clear();
         q  << QString( "get_instrument_info_new" )
            << QString::number( ID );
         db->query( q );
         db->next();

         instrument.ID     = ID;
         instrument.name   = db->value( 0 ).toString();
         instrument.serial = db->value( 1 ).toString();
	 instrument.optimaHost       = db->value( 5 ).toString();
	 instrument.optimaPort       = db->value( 6 ).toString().toInt();
	 instrument.optimaDBname     = db->value( 7 ).toString();
	 instrument.optimaDBusername = db->value( 8 ).toString();
	 instrument.optimaDBpassw    = db->value( 9 ).toString();
	 instrument.selected         = db->value( 10 ).toString().toInt();
	 
	 instrument.os1              = db->value( 11 ).toString();
	 instrument.os2              = db->value( 12 ).toString();
	 instrument.os3              = db->value( 13 ).toString();
	 
	 instrument.radcalwvl        = db->value( 14 ).toString();
	 instrument.chromoab         = db->value( 15 ).toString();
         instrument.operators.clear();

         this->instruments << instrument;
      }

      // Operator information
      for ( int i = 0; i < instruments.size(); i++ )
      {
         q.clear();
         q  << QString( "get_operator_names" )
            << QString::number( instruments[ i ].ID );
         db->query( q );

         if ( db->lastErrno() == US_DB2::OK )
         {
            while ( db->next() )
            {
               US_Rotor::Operator oper;

               oper.ID    = db->value( 0 ).toString().toInt();
               oper.GUID  = db->value( 1 ).toString();
               oper.lname = db->value( 2 ).toString();
               oper.fname = db->value( 3 ).toString();

               this->instruments[ i ].operators << oper;
            }
         }
      }
   }
 
   return ROTOR_OK;
}

void US_Rotor::Lab::reset( void )
{
   ID           = -1;
   GUID         = "";
   name         = "";
   building     = "";
   room         = "";
   instruments.clear();
}

void US_Rotor::Lab::show( void )
{
   qDebug() << "ID =           " <<  ID           ;
   qDebug() << "GUID =         " <<  GUID         ;
   qDebug() << "name =         " <<  name         ;
   qDebug() << "building =     " <<  building     ;
   qDebug() << "room =         " <<  room         ;

   qDebug() << "Instruments...";
   foreach ( US_Rotor::Instrument instrument, instruments )
   {
      qDebug() << "  instrument ID =     " << instrument.ID     ;
      qDebug() << "  instrument name =   " << instrument.name   ;
      qDebug() << "  instrument serial = " << instrument.serial ;

      qDebug() << "  Operators...";
      foreach( US_Rotor::Operator oper, instrument.operators )
      {
         qDebug() << "    operator ID =    " << oper.ID   ;
         qDebug() << "    operator GUID =  " << oper.GUID ;
         qDebug() << "    operator lname = " << oper.lname ;
         qDebug() << "    operator fname = " << oper.fname ;
      }
   }
}

US_Rotor::AbstractRotor::AbstractRotor()
{
   reset();
}

// A function to read information about a single abstract rotor from DB
US_Rotor::Status US_Rotor::AbstractRotor::readDB( int abstractRotorID, US_DB2* db )
{
   // Try to get rotor info
   QStringList q( "get_abstractRotor_info" );
   q  << QString::number( abstractRotorID );
   db->query( q );
   int readStatus = db->lastErrno();

   if ( readStatus == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( readStatus != US_DB2::OK )
      return MISC_ERROR;

   db->next();
   this->ID          = abstractRotorID;
   GUID              = db->value(0).toString();
   name              = db->value(1).toString();
   material          = db->value(2).toString();
   numHoles          = db->value(3).toInt();
   maxRPM            = db->value(4).toInt();
   magnetOffset      = db->value(5).toFloat();
   cellCenter        = db->value(6).toFloat();
   manufacturer      = db->value(7).toString();
   
   return ROTOR_OK;
}

void US_Rotor::AbstractRotor::reset( void )
{
   ID           = -1;
   GUID         = "";
   name         = "";
   material     = "";
   numHoles     = -1;
   maxRPM       = -1;
   magnetOffset = -1;
   cellCenter   = -1.0;
   manufacturer = "";
}

void US_Rotor::AbstractRotor::show( void )
{
   qDebug() << "ID =           " <<  ID           ;
   qDebug() << "GUID =         " <<  GUID         ;
   qDebug() << "name =         " <<  name         ;
   qDebug() << "material =     " <<  material     ;
   qDebug() << "numHoles =     " <<  numHoles     ;
   qDebug() << "maxRPM =       " <<  maxRPM       ;
   qDebug() << "magnetOffset = " <<  magnetOffset ;
   qDebug() << "cellCenter =   " <<  cellCenter   ;
   qDebug() << "manufacturer = " <<  manufacturer ;
}

US_Rotor::Rotor::Rotor()
{
   reset();
}

// A function to add the current rotor as a new rotor in the DB
int US_Rotor::Rotor::addRotorDB( US_DB2* db )
{
   QStringList q( "add_rotor" );
   q  << QString::number( abstractRotorID )
      << abstractRotorGUID
      << QString::number( labID )
      << GUID
      << name
      << serialNumber;

   int status = db->statusQuery( q );

   return status;
}

// A function to read information about a single rotor from DB
US_Rotor::Status US_Rotor::Rotor::readDB( int rotorID, US_DB2* db )
{
   // Try to get rotor info
   QStringList q( "get_rotor_info" );
   q  << QString::number( rotorID );
   db->query( q );
   int readStatus = db->lastErrno();

   if ( readStatus == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( readStatus != US_DB2::OK )
      return MISC_ERROR;

   db->next();
   this->ID          = rotorID;
   abstractRotorID   = db->value( 4 ).toString().toInt();
   abstractRotorGUID = db->value( 5 ).toString();
   labID             = db->value( 6 ).toString().toInt();
   GUID              = db->value( 0 ).toString();
   name              = db->value( 1 ).toString();
   serialNumber      = db->value( 2 ).toString();
   
   return ROTOR_OK;
}

// A function to delete the specified rotor from the DB
int US_Rotor::Rotor::deleteRotorDB( int rotorID, US_DB2* db )
{
   QStringList q( "delete_rotor" );
   q  <<  QString::number( rotorID );

   int status = db->statusQuery( q );

   // most likely status would be OK or ROTOR_IN_USE
   return status;
}

// Function to save current rotor information to disk
void US_Rotor::Rotor::saveDisk( void )
{
   // First make sure we have a GUID
   static const QRegularExpression rx( "^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$", QRegularExpression::CaseInsensitiveOption );

   if ( ! rx.match( GUID ).hasMatch() )
      GUID = US_Util::new_guid();

   // Get a path and file name for the rotor
   QString path;
   if ( ! diskPath( path ) ) return;

   bool    newFile;
   QString filename = get_filename(
                         path,
                         "R???????.xml",
                         "Rotor",
                         ID,
                         newFile );

   QFile file( filename );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for writing"
               << filename;
      return;
   }

   // Generate xml
   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_Rotor>");
   xml.writeStartElement("RotorData");
   xml.writeAttribute("version", "1.0");

   xml.writeStartElement( "Rotor" );
   xml.writeAttribute   ( "id",                QString::number( ID                ) );
   xml.writeAttribute   ( "guid",                               GUID                );
   xml.writeAttribute   ( "abstractRotorID",   QString::number( abstractRotorID   ) );
   xml.writeAttribute   ( "abstractRotorGUID",                  abstractRotorGUID   );
   xml.writeAttribute   ( "labID",             QString::number( labID             ) );
   xml.writeAttribute   ( "name",                               name                );
   xml.writeAttribute   ( "serialNumber",                       serialNumber        );
   xml.writeEndElement  ();

   xml.writeEndElement  ();        // RotorData
   xml.writeEndDocument ();

   file.close();
}

// Function to read all the rotor info from disk
US_Rotor::Status US_Rotor::Rotor::readDisk( const int& id )
{
   QString filename;
   bool found = diskFilename( "R*.xml", "Rotor", id, filename );

   if ( ! found )
   {
      qDebug() << "Error: file not found for id "
               << id;
      return US_Rotor::NOT_FOUND;
   }

   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return US_Rotor::NOT_OPENED;
   }


   QXmlStreamReader xml( &file );

   reset();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "Rotor" )
         {
            QXmlStreamAttributes a = xml.attributes();

            ID                = a.value( "id"                ).toString().toInt();
            GUID              = a.value( "guid"              ).toString();
            abstractRotorID   = a.value( "abstractRotorID"   ).toString().toInt();
            abstractRotorGUID = a.value( "abstractRotorGUID" ).toString();
            labID             = a.value( "labID"             ).toString().toInt();
            name              = a.value( "name"              ).toString();
            serialNumber      = a.value( "serialNumber"      ).toString();

         }
      }
   }

   file.close();

   if ( xml.hasError() )
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return US_Rotor::MISC_ERROR;
   }

   return US_Rotor::ROTOR_OK;
}

void US_Rotor::Rotor::reset( void )
{
   ID              = -1;
   abstractRotorID = -1;
   labID           = -1;
   GUID            = "";
   name            = "< Not Selected >";
   serialNumber    = "< Not Selected >";
}

void US_Rotor::Rotor::show( void )
{
   qDebug() << "ID =              " <<  ID              ;
   qDebug() << "abstractRotorID = " <<  abstractRotorID ;
   qDebug() << "labID =           " <<  labID           ;
   qDebug() << "GUID =            " <<  GUID            ;
   qDebug() << "name =            " <<  name            ;
   qDebug() << "serialNumber =    " <<  serialNumber    ;
}

US_Rotor::RotorCalibration::RotorCalibration()
{
   reset();
}

int US_Rotor::RotorCalibration::saveDB( int rotorID, US_DB2* db )
{
   QStringList q( "add_rotor_calibration" );
   q  << QString::number( rotorID )
      << GUID
      << report
      << QString::number( coeff1 )
      << QString::number( coeff2 )
      << QString::number( omega2t )
      << QString::number( calibrationExperimentID )
      << label;
   
   int status = db->statusQuery( q );
   
   if ( status == US_DB2::OK )
      this->ID   = db->lastInsertID();

   return status;
}

// A function to read information about a single rotor calibration profile from DB
US_Rotor::Status US_Rotor::RotorCalibration::readDB( int calibrationID, US_DB2* db )
{
   // Try to get rotor info
   QStringList q( "get_rotor_calibration_info" );
   q  << QString::number( calibrationID );
   db->query( q );
   int readStatus = db->lastErrno();

   if ( readStatus == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( readStatus != US_DB2::OK )
      return MISC_ERROR;

   db->next();
   this->ID          = calibrationID;
   this->GUID        = db->value( 0 ).toString();
   this->rotorID     = db->value( 1 ).toString().toInt();
   this->rotorGUID   = db->value( 2 ).toString();
   this->calibrationExperimentID = db->value( 8 ).toString().toInt();
   //this->calibrationExperimentGUID = not implemented in stored routines db->value(  ).toString();
   this->coeff1      = db->value( 4 ).toString().toFloat();
   this->coeff2      = db->value( 5 ).toString().toFloat();
   this->label       = db->value( 9 ).toString();
   this->report      = db->value( 3 ).toString();

   QStringList dateParts = db->value( 7 ).toString().split( " " );
   this->lastUpdated = QDate::fromString( dateParts[ 0 ], "yyyy-MM-dd"  );

   this->omega2t     = db->value( 6 ).toString().toFloat();
   
   //      QString calibrationExperimentGUID; //!< The GUID of the experiment that contains the calibration data
   return ROTOR_OK;
}

// A function to delete the specified rotor calibration from the DB
int US_Rotor::RotorCalibration::deleteCalibrationDB( int calibrationID, US_DB2* db )
{
   QStringList q( "delete_rotor_calibration" );
   q  <<  QString::number( calibrationID );

   int status = db->statusQuery( q );

   // most likely status would be OK or ROTOR_IN_USE
   return status;
}

// A function to find out if the original dummy calibration is still there,
//  and replace it with the current one if it does.
int US_Rotor::RotorCalibration::replaceDummyDB( int& oldCalibrationID, US_DB2* db )
{
   QStringList calibrationIDs;

   QStringList q( "get_rotor_calibration_profiles" );
   q  << QString::number( rotorID );

   db->query( q );
   
   while ( db->next() )
      calibrationIDs << db->value( 0 ).toString();

   // Let's remove the one we just added from the list
   int ndx = calibrationIDs.indexOf( QString::number( this->ID ) );
   calibrationIDs.removeAt( ndx );
   
   // Let's make sure we still have some
   if ( calibrationIDs.isEmpty() )
      return US_DB2::OK;                        // Not really an error

   oldCalibrationID = -1;                       // = -1 means we haven't found it
   foreach ( QString calibrationID, calibrationIDs )
   {
      q.clear();
      q  << "get_rotor_calibration_info"
         << calibrationID;

      db->query( q );
      db->next();
      QString report = db->value( 3 ).toString();
      QString label  = db->value( 9 ).toString();

      if ( report.contains( "This is a dummy calibration --- please replace." ) &&
           label.contains( "Dummy Calibration" ) )
      {
         oldCalibrationID = calibrationID.toInt();
         break;
      }
   }

   // Let's see if we found it
   if ( oldCalibrationID == -1 )
      return US_DB2::OK;                        // Not an error either

   // Ok found one, replace
   q.clear();
   q  << "replace_rotor_calibration"
      << QString::number( oldCalibrationID )
      << QString::number( this->ID );
   int status = db->statusQuery( q );

   return status;
}

// Function to save current calibration information to disk
void US_Rotor::RotorCalibration::saveDisk( void )
{
   // First make sure we have a GUID
   static const QRegularExpression rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.match( GUID ).hasMatch() )
      GUID = US_Util::new_guid();

   // Get a path and file name for the calibration
   QString path;
   if ( ! diskPath( path ) ) return;

   bool    newFile;
   QString filename = get_filename(
                         path,
                         "C???????.xml",
                         "Calibration",
                         ID,
                         newFile );

   QFile file( filename );
   if ( !file.open( QIODevice::WriteOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for writing"
               << filename;
      return;
   }

   // Generate xml
   QXmlStreamWriter xml;
   xml.setDevice( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD("<!DOCTYPE US_RotorCalibration>");
   xml.writeStartElement("RotorCalibrationData");
   xml.writeAttribute("version", "1.0");

   xml.writeStartElement( "Calibration" );
   xml.writeAttribute   ( "id",                QString::number( ID                ) );
   xml.writeAttribute   ( "guid",                               GUID                );
   xml.writeAttribute   ( "rotorID",           QString::number( rotorID           ) );
   xml.writeAttribute   ( "rotorGUID",                          rotorGUID           );
   xml.writeAttribute   ( "calExpID",          QString::number( calibrationExperimentID ) );
   xml.writeAttribute   ( "calExpGUID",                         calibrationExperimentGUID );
   xml.writeAttribute   ( "coeff1",            QString::number( coeff1            ) );
   xml.writeAttribute   ( "coeff2",            QString::number( coeff2            ) );
   xml.writeAttribute   ( "lastUpdated",                        lastUpdated.toString( "yyyy-MM-dd" ) );
   xml.writeAttribute   ( "omega2t",           QString::number( omega2t           ) );
   xml.writeAttribute   ( "label",                              label               );
   xml.writeTextElement ( "report",                             report              );
   xml.writeEndElement  ();

   xml.writeEndElement  ();        // RotorCalibrationData
   xml.writeEndDocument ();

   file.close();
}

// Function to read all the calibration info from disk
US_Rotor::Status US_Rotor::RotorCalibration::readDisk( const int& id )
{
   QString filename;
   bool found = diskFilename( "C*.xml", "Calibration", id, filename );

   if ( ! found )
   {
      qDebug() << "Error: file not found for id "
               << id;
      return US_Rotor::NOT_FOUND;
   }

   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return US_Rotor::NOT_OPENED;
   }


   QXmlStreamReader xml( &file );

   reset();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "Calibration" )
         {
            QXmlStreamAttributes a = xml.attributes();

            ID                = a.value( "id"                 ).toString().toInt();
            GUID              = a.value( "guid"               ).toString();
            rotorID           = a.value( "rotorID"            ).toString().toInt();
            rotorGUID         = a.value( "rotorGUID"          ).toString();
            calibrationExperimentID   = a.value( "calExpID"   ).toString().toInt();
            calibrationExperimentGUID = a.value( "calExpGUID" ).toString();
            coeff1            = a.value( "coeff1"             ).toString().toFloat();
            coeff2            = a.value( "coeff2"             ).toString().toFloat();
            lastUpdated       = QDate::fromString( a.value( "lastUpdated" ).toString(), "yyyy-MM-dd" );
            omega2t           = a.value( "omega2t"            ).toString().toFloat();
            label             = a.value( "label"              ).toString();

            readReport( xml );
         }
      }
   }

   file.close();

   if ( xml.hasError() )
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return US_Rotor::MISC_ERROR;
   }

   return US_Rotor::ROTOR_OK;
}

void US_Rotor::RotorCalibration::readReport( QXmlStreamReader& xml )
{
   while ( !xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement() && xml.name() == "Calibration" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "report" )
         {
            xml.readNext();
            report = xml.text().toString();
         }
      }
   }
}

void US_Rotor::RotorCalibration::reset( void )
{
   ID                        = -1;
   GUID                      = "";
   calibrationExperimentGUID = "";
   coeff1                    = 0.0;
   coeff2                    = 0.0;
   label                     = "";
   report                    = "";
   lastUpdated               = QDate::currentDate();
   omega2t                   = 0.0;
}

void US_Rotor::RotorCalibration::show( void )
{
   qDebug() << "GUID   = "                      << GUID;
   qDebug() << "rotorID = "                     << rotorID;
   qDebug() << "rotorGUID = "                   << rotorGUID;
   qDebug() << "calibration experiment ID = "   << calibrationExperimentID;
   qDebug() << "calibration experiment GUID = " << calibrationExperimentGUID;
   qDebug() << "coeff1 = "                      << coeff1;
   qDebug() << "coeff2 = "                      << coeff2;
   qDebug() << "last updated = "                << lastUpdated.toString( "yyyy-MM-dd" );
   qDebug() << "omega2t = "                     << QString::number( omega2t );
   qDebug() << "label = "                       << label;
   qDebug() << "report";
   qDebug() << report;
}
