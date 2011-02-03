//! \file us_rotor_gui.cpp

#include "us_rotor.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_db2.h"

// The constructor clears out the data structure
US_Rotor::US_Rotor()
{
}

US_Rotor::~US_Rotor()
{
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

   // Since we're not really editing abstract rotors, let's just copy
   // the information to disk directly
   saveAbstractRotorsDisk( arList );

   return ROTOR_OK;
}

// Function to save abstract rotor information to disk
void US_Rotor::saveAbstractRotorsDisk( QVector< US_Rotor::AbstractRotor >& arList )
{
   // Get a path and file name for abstract rotors
   QString path;
   if ( ! diskPath( path ) ) return;

   QString filename = path + "/abstractRotors.xml";
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
   xml.writeDTD("<!DOCTYPE US_AbstractRotor>");
   xml.writeStartElement("AbstractRotorData");
   xml.writeAttribute("version", "1.0");

   // Loop through all the abstract rotors
   for ( int i = 0; i < arList.size(); i++ )
   {
      US_Rotor::AbstractRotor ar = arList[ i ];

      xml.writeStartElement( "abstractRotor" );
      xml.writeAttribute   ( "id",           QString::number( ar.ID              ) );
      xml.writeAttribute   ( "guid",                          ar.GUID              );
      xml.writeAttribute   ( "name",                          ar.name              );
      xml.writeAttribute   ( "materialName",                  ar.material          );
      xml.writeAttribute   ( "numHoles",     QString::number( ar.numHoles        ) );
      xml.writeAttribute   ( "maxRPM",       QString::number( ar.maxRPM          ) );
      xml.writeAttribute   ( "magnetOffset", QString::number( ar.magnetOffset    ) );
      xml.writeAttribute   ( "cellCenter",   QString::number( ar.cellCenter      ) );
      xml.writeAttribute   ( "manufacturer",                  ar.manufacturer      );
      xml.writeEndElement  ();
   }

   xml.writeEndElement  ();        // abstractRotorData
   xml.writeEndDocument ();

   file.close();
}

// Function to read all the abstract rotor info from disk
US_Rotor::Status US_Rotor::readAbstractRotorsDisk( QVector< US_Rotor::AbstractRotor >& arList )
{
   QString filename = US_Settings::dataDir() + "/rotors/abstractRotors.xml";
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
   }

   return true;
}

// Function to check if filename already exists, and perhaps generate a new one
QString US_Rotor::get_filename(
      const QString& path, bool& newFile )
{
   QDir        f( path );
   QStringList filter( "R???????.xml" );
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
            if ( xml.name() == "rotor" )
            {
               QXmlStreamAttributes a = xml.attributes();

/*
 * Probably need to replace this with a GUID parameter to the function
               if ( a.value( "guid" ).toString() == solutionGUID )
               {
                  newFile  = false;
                  filename = path + "/" + f_names[ i ];
               }
*/
               break;
           }
         }
      }

      b_file.close();
      if ( ! newFile ) return filename;
   }

   // If we get here, generate a new filename
   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;

   return path + "/R" + QString().sprintf( "%07i", number + 1 ) + ".xml";
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
      << QString::number( calibrationExperimentID );

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
   this->report      = db->value( 3 ).toString();
   this->lastUpdated = QDate::fromString( db->value( 7 ).toString() );
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

void US_Rotor::RotorCalibration::reset( void )
{
   ID                        = -1;
   GUID                      = "";
   calibrationExperimentGUID = "";
   coeff1                    = 0.0;
   coeff2                    = 0.0;
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
   qDebug() << "last updated = "                << lastUpdated.toString();
   qDebug() << "omega2t = "                     << QString::number( omega2t );
   qDebug() << "report";
   qDebug() << report;
}
