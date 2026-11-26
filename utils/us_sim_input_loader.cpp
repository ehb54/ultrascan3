//! \file us_sim_input_loader.cpp
#include "us_sim_input_loader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QXmlStreamReader>

#include "us_settings.h"

namespace
{
// The value of an attribute on the first <element> in an XML file, with the
// file's description; false when the file cannot be read or has no element.
bool firstElementAttributes( const QString& path, const QString& element, QXmlStreamAttributes& attributes )
{
   QFile file( path );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      return false;
   }
   QXmlStreamReader xml( &file );
   while ( !xml.atEnd() )
   {
      xml.readNext();
      if ( xml.isStartElement() && xml.name() == element )
      {
         attributes = xml.attributes();
         return true;
      }
   }
   return false;
}

// A database that is given but not connected: report why, rather than
// running lookups that can only find nothing.
bool unusable( IUS_DB2* db, QString& error )
{
   if ( db == nullptr || db->isConnected() )
   {
      return false;
   }
   error = QObject::tr( "the database is not connected: %1" ).arg( db->lastError() );
   return true;
}

bool isReadableFile( const QString& spec )
{
   const QFileInfo info( spec );
   return info.exists() && info.isFile() && info.isReadable();
}
} // namespace

bool US_SimInputLoader::model( const QString& spec, IUS_DB2* db, int investigator, US_Model& model, QString& error )
{
   if ( spec.isEmpty() )
   {
      error = QObject::tr( "no model was given" );
      return false;
   }

   if ( isReadableFile( spec ) )
   {
      US_Model loaded;
      if ( loaded.load( spec ) != IUS_DB2::OK )
      {
         error = QObject::tr( "the model file %1 could not be read" ).arg( spec );
         return false;
      }
      model = loaded;
      return true;
   }

   if ( db == nullptr )
   {
      // A model GUID among the local model files.
      QString path;
      if ( !US_Model::model_path( path ) )
      {
         error = QObject::tr( "the model path is not set" );
         return false;
      }
      const QStringList names = QDir( path ).entryList( { "M*.xml" }, QDir::Files, QDir::Name );
      for ( const QString& name : names )
      {
         QXmlStreamAttributes a;
         if ( firstElementAttributes( path + "/" + name, "model", a ) && a.value( "modelGUID" ).toString() == spec )
         {
            US_Model loaded;
            if ( loaded.load( path + "/" + name ) == IUS_DB2::OK )
            {
               model = loaded;
               return true;
            }
         }
      }
      error = QObject::tr( "no local model has GUID %1" ).arg( spec );
      return false;
   }

   if ( unusable( db, error ) )
   {
      return false;
   }
   if ( investigator < 0 )
   {
      error = QObject::tr( "the investigator is not set" );
      return false;
   }

   // A model GUID or ID among the investigator's models.
   QStringList ids;
   db->query( QStringList{ "get_model_desc", QString::number( investigator ) } );
   while ( db->next() )
   {
      if ( db->value( 0 ).toString() == spec || db->value( 1 ).toString() == spec )
      {
         ids << db->value( 0 ).toString();
      }
   }
   for ( const QString& id : ids )
   {
      US_Model loaded;
      if ( loaded.load( id, db ) == IUS_DB2::OK )
      {
         model = loaded;
         return true;
      }
   }
   error = ids.isEmpty() ? QObject::tr( "the investigator has no model %1" ).arg( spec )
                         : QObject::tr( "model %1 could not be read from the database: %2" ).arg( spec, db->lastError() );
   return false;
}

bool US_SimInputLoader::buffer( const QString& spec, IUS_DB2* db, int investigator, US_Buffer& buffer, QString& error )
{
   if ( spec.isEmpty() )
   {
      error = QObject::tr( "no buffer was given" );
      return false;
   }

   if ( isReadableFile( spec ) )
   {
      US_Buffer loaded;
      if ( !loaded.readFromDisk( spec ) )
      {
         error = QObject::tr( "the buffer file %1 could not be read" ).arg( spec );
         return false;
      }
      buffer = loaded;
      return true;
   }

   if ( db == nullptr )
   {
      // A buffer GUID among the local buffer files; its components come from
      // the local component list.
      const QString     path  = US_Settings::dataDir() + "/buffers";
      const QStringList names = QDir( path ).entryList( { "B*.xml" }, QDir::Files, QDir::Name );
      for ( const QString& name : names )
      {
         QXmlStreamAttributes a;
         if ( !firstElementAttributes( path + "/" + name, "buffer", a ) || a.value( "guid" ).toString() != spec )
         {
            continue;
         }
         US_Buffer loaded;
         if ( !loaded.readFromDisk( path + "/" + name ) )
         {
            error = QObject::tr( "the buffer file %1 could not be read" ).arg( path + "/" + name );
            return false;
         }
         QMap<QString, US_BufferComponent> components;
         US_BufferComponent::getAllFromHD( components );
         loaded.component.clear();
         for ( const QString& id : loaded.componentIDs )
         {
            loaded.component << components[id];
         }
         buffer = loaded;
         return true;
      }
      error = QObject::tr( "no local buffer has GUID %1" ).arg( spec );
      return false;
   }

   if ( unusable( db, error ) )
   {
      return false;
   }

   // A buffer ID among the investigator's buffers, or a buffer GUID.
   QString id;
   db->query( QStringList{ "get_buffer_desc", QString::number( investigator ) } );
   while ( db->next() )
   {
      if ( db->value( 0 ).toString() == spec )
      {
         id = spec;
         break;
      }
   }
   if ( id.isEmpty() )
   {
      db->query( QStringList{ "get_bufferID", spec } );
      if ( db->lastErrno() == IUS_DB2::OK && db->next() )
      {
         id = db->value( 0 ).toString();
      }
   }
   if ( id.isEmpty() )
   {
      error = QObject::tr( "the investigator has no buffer %1" ).arg( spec );
      return false;
   }

   US_Buffer loaded;
   if ( !loaded.readFromDB( db, id ) )
   {
      error =
         QObject::tr( "buffer %1 could not be read from the database: %2" )
            .arg( id, db->lastError().isEmpty() ? QObject::tr( "it or one of its components was not found" ) : db->lastError() );
      return false;
   }
   buffer = loaded;
   return true;
}

bool US_SimInputLoader::rotorCoefficients( const QString& spec, double& coeff1, double& coeff2 )
{
   const QStringList parts = spec.split( ":" );
   if ( parts.size() != 2 )
   {
      return false;
   }
   coeff1 = parts[0].toDouble();
   coeff2 = parts[1].toDouble();
   return true;
}

bool US_SimInputLoader::rotorEntryMatches( const QString& entry, const QString& spec )
{
   return entry.split( ":" ).contains( spec );
}

bool US_SimInputLoader::rotor( const QString& spec, IUS_DB2* db, RotorChoice& choice, QString& error )
{
   choice = RotorChoice();
   if ( rotorCoefficients( spec, choice.coeff1, choice.coeff2 ) )
   {
      choice.coefficientsOnly = true;
      return true;
   }

   if ( unusable( db, error ) )
   {
      return false;
   }

   QVector<US_Rotor::Lab> labs;
   if ( db != nullptr ? US_Rotor::readLabsDB( labs, db ) != US_Rotor::ROTOR_OK
                      : US_Rotor::readLabsDisk( labs ) == US_Rotor::NOT_FOUND )
   {
      error = QObject::tr( "the lab list could not be read" );
      return false;
   }

   // Entries "ID:name:labID", lab by lab, as the rotor dialog lists them.
   QStringList entries;
   for ( const US_Rotor::Lab& lab : labs )
   {
      if ( db != nullptr )
      {
         db->query( QStringList{ "get_rotor_names", QString::number( lab.ID ) } );
         while ( db->next() )
         {
            entries << db->value( 0 ).toString() + ":" + db->value( 1 ).toString() + ":" + QString::number( lab.ID );
         }
      }
      else
      {
         // A lab without local rotor files has nothing to match.
         QVector<US_Rotor::Rotor> rotors;
         if ( US_Rotor::readRotorsFromDisk( rotors, lab.ID ) == US_Rotor::NOT_FOUND )
         {
            continue;
         }
         for ( const US_Rotor::Rotor& r : rotors )
         {
            entries << QString::number( r.ID ) + ":" + r.name + ":" + QString::number( r.labID );
         }
      }
   }

   int rotorID = -1;
   for ( const QString& entry : entries )
   {
      if ( rotorEntryMatches( entry, spec ) )
      {
         rotorID = entry.section( ":", 0, 0 ).toInt();
         break;
      }
   }
   if ( rotorID <= 0 )
   {
      error = QObject::tr( "no rotor matches %1" ).arg( spec );
      return false;
   }

   const US_Rotor::Status rotor_status = db != nullptr ? choice.rotor.readDB( rotorID, db ) : choice.rotor.readDisk( rotorID );
   if ( rotor_status != US_Rotor::ROTOR_OK )
   {
      error = QObject::tr( "rotor %1 could not be read" ).arg( rotorID );
      return false;
   }

   // The first calibration profile, as the dialog selects it.
   int calibrationID = -1;
   if ( db != nullptr )
   {
      choice.rotor.saveDisk();
      db->query( QStringList{ "get_rotor_calibration_profiles", QString::number( rotorID ) } );
      if ( db->next() )
      {
         calibrationID = db->value( 0 ).toInt();
      }
   }
   else
   {
      QVector<US_Rotor::RotorCalibration> profiles;
      if ( US_Rotor::readCalibrationProfilesDisk( profiles, rotorID ) != US_Rotor::NOT_FOUND )
      {
         for ( const US_Rotor::RotorCalibration& profile : profiles )
         {
            if ( profile.rotorID == rotorID )
            {
               calibrationID = profile.ID;
               break;
            }
         }
      }
   }

   if ( calibrationID > 0 )
   {
      const US_Rotor::Status calibration_status =
         db != nullptr ? choice.calibration.readDB( calibrationID, db ) : choice.calibration.readDisk( calibrationID );
      if ( calibration_status == US_Rotor::ROTOR_OK && db != nullptr )
      {
         choice.calibration.saveDisk();
      }
   }
   choice.coeff1 = choice.calibration.coeff1;
   choice.coeff2 = choice.calibration.coeff2;
   return true;
}
