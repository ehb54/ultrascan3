//! \file us_solution.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_solution.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_math2.h"

// The constructor clears out the data structure
US_Solution::US_Solution()
{
   clear();
}

// Function to load a solution from the disk
void US_Solution::readFromDisk( QString& guid )
{
   QString filename;
   bool found = diskFilename( guid, filename );

   if ( ! found )
   {
      qDebug() << "Error: file not found for guid "
               << guid;
      return;
   }
   
   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return;
   }

   QXmlStreamReader xml( &file );

   clear();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "solution" )
         {
            QXmlStreamAttributes a = xml.attributes();
            solutionID   = a.value( "id" ).toString().toInt();
            solutionGUID = a.value( "guid" ).toString();
            commonVbar20 = a.value( "commonVbar20" ).toString().toDouble();
            storageTemp  = a.value( "storageTemp" ).toString().toDouble();

            readSolutionInfo( xml );
         }
      }
   }

   file.close();

   if ( xml.hasError() ) 
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return;
   }

   saveStatus = HD_ONLY;
}

void US_Solution::readSolutionInfo( QXmlStreamReader& xml )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "solution" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "description" )
         {
            xml.readNext();
            solutionDesc = xml.text().toString();
         }

         else if ( xml.name() == "notes" )
         {
            xml.readNext();
            notes = xml.text().toString();
         }

         else if ( xml.name() == "buffer" )
         {
            QXmlStreamAttributes a = xml.attributes();
            bufferID        = a.value( "id" )  .toString().toInt();
            bufferGUID      = a.value( "guid" ).toString();
            bufferDesc      = a.value( "desc" ).toString();
         }
   
         else if ( xml.name() == "analyte" )
         {
            AnalyteInfo analyte;

            QXmlStreamAttributes a = xml.attributes();
            analyte.analyteID       = a.value( "id" ).toString().toInt();
            analyte.analyteGUID     = a.value( "guid" ).toString();
            analyte.analyteDesc     = a.value( "desc" ).toString();
            analyte.amount          = a.value( "amount" ).toString().toDouble();
            analyte.mw              = a.value( "mw" ).toString().toDouble();
            analyte.vbar20          = a.value( "vbar20" ).toString().toDouble();
            analyte.type            = a.value( "type" ).toString().toInt();

            analytes << analyte;
         }
      }
   }
}

// Function to load a solution from the db
void US_Solution::readFromDB  ( int solutionID, US_DB2* db )
{
   // Try to get solution info
   QStringList q( "get_solution" );
   q  << QString::number( solutionID );
   db->query( q );

   clear();

   if ( db->next() )
   {
      this->solutionID = solutionID;
      solutionGUID     = db->value( 0 ).toString();
      solutionDesc     = db->value( 1 ).toString();
      commonVbar20     = db->value( 2 ).toDouble();
      storageTemp      = db->value( 3 ).toDouble();
      notes            = db->value( 4 ).toString();

      q.clear();
      q  << "get_solutionBuffer"
         << QString::number( solutionID );
      db->query( q );
      if ( db->next() )
      {
         bufferID   = db->value( 0 ).toInt();
         bufferGUID = db->value( 1 ).toString();
         bufferDesc = db->value( 2 ).toString();
      }

      q.clear();
      q  << "get_solutionAnalyte"
         << QString::number( solutionID );
      db->query( q );
      while ( db->next() )
      {
         AnalyteInfo analyte;

         analyte.analyteID   = db->value( 0 ).toInt();
         analyte.analyteGUID = db->value( 1 ).toString();
         analyte.analyteDesc = db->value( 2 ).toString();
         analyte.amount      = db->value( 3 ).toDouble();
         analyte.mw          = db->value( 4 ).toDouble();
         analyte.vbar20      = db->value( 5 ).toDouble();
         analyte.type        = db->value( 6 ).toInt();

         analytes << analyte;
      }
   }

   saveStatus = DB_ONLY;
}

// Function to save solution information to disk
void US_Solution::saveToDisk( void )
{
   // First make sure we have a GUID
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.exactMatch( solutionGUID ) )
      solutionGUID = US_Util::new_guid();

   // Get a path and file name for solution
   QString path;
   if ( ! diskPath( path ) ) return;

   bool    newFile;
   QString filename = get_filename( 
                         path, 
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
   xml.writeDTD("<!DOCTYPE US_Solution>");
   xml.writeStartElement("SolutionData");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "solution" );
   xml.writeAttribute   ( "id", QString::number( solutionID ) );
   xml.writeAttribute   ( "guid", solutionGUID );
   xml.writeAttribute   ( "commonVbar20", QString::number( commonVbar20 ) );
   xml.writeAttribute   ( "storageTemp", QString::number( storageTemp ) );
   xml.writeTextElement ( "description", solutionDesc );
   xml.writeTextElement ( "notes", notes );

      xml.writeStartElement( "buffer" );
      xml.writeAttribute   ( "id",   QString::number( bufferID ) );
      xml.writeAttribute   ( "guid", bufferGUID );
      xml.writeAttribute   ( "desc", bufferDesc );
      xml.writeEndElement  ();
   
      // Loop through all the analytes
      for ( int i = 0; i < analytes.size(); i++ )
      {
         AnalyteInfo analyte = analytes[ i ];

         xml.writeStartElement( "analyte" );
         xml.writeAttribute   ( "id",   QString::number( analyte.analyteID ) );
         xml.writeAttribute   ( "guid", analyte.analyteGUID );
         xml.writeAttribute   ( "desc", analyte.analyteDesc );
         xml.writeAttribute   ( "amount", QString::number( analyte.amount  ) );
         xml.writeAttribute   ( "mw", QString::number( analyte.mw  ) );
         xml.writeAttribute   ( "vbar20", QString::number( analyte.vbar20  ) );
         xml.writeAttribute   ( "type",   QString::number( analyte.type ) );
         xml.writeEndElement  ();
      }

   xml.writeEndElement  ();        // solution
   xml.writeEndDocument ();

   file.close();

   saveStatus = ( saveStatus == DB_ONLY ) ? BOTH : HD_ONLY;
}

// Function to save solution information to db
int US_Solution::saveToDB( int expID, int channelID, US_DB2* db )
{
   // Let's see if the buffer is in the db already
   QStringList q( "get_bufferID" );
   q  << bufferGUID;
   db->query( q );

   int status = db->lastErrno();
   if ( status == US_DB2::NOROWS )
   {
      // Find the buffer directory on disk
      QDir dir;
      QString path = US_Settings::dataDir() + "/buffers";

      if ( ! dir.exists( path ) )
         return US_DB2::NO_BUFFER;            // So we have some idea of what happened

      // Try to find the buffer file
      bool    newFile;
      QString filename = US_Buffer::get_filename( path, bufferGUID, newFile );

      if ( newFile ) 
         return US_DB2::NO_BUFFER;

      // Then we can add it
      US_Buffer* diskBuffer = new US_Buffer;
      bool diskStatus = diskBuffer->readFromDisk( filename );   // load it from disk

      // Now create the component list
      QMap< QString, US_BufferComponent > component_list;
      US_BufferComponent::getAllFromHD( component_list );
      diskBuffer->component.clear();
   
      for ( int i = 0; i < diskBuffer->componentIDs.size(); i++ )
      {
         QString index = diskBuffer->componentIDs[ i ];
         diskBuffer->component << component_list[ index ];
      }

      if ( diskStatus )
         diskBuffer->saveToDB( db, QString::number( 1 ) );      // and write to db; 1 = private
      delete diskBuffer;

      // Double check
      QStringList q( "get_bufferID" );
      q  << bufferGUID;
      db->query( q );
      int checkStatus = db->lastErrno();
      if ( checkStatus != US_DB2::OK )
         return US_DB2::NO_BUFFER;
   }

   else if ( status != US_DB2::OK )
      return status;

   // Now let's see if the analytes are in the db already
   foreach( AnalyteInfo analyte, analytes )
   {
      q.clear();
      q  << "get_analyteID"
         << analyte.analyteGUID;
      db->query( q );

      status = db->lastErrno();
      if ( status == US_DB2::NOROWS )
      {
         // Then we need to add it
         US_Analyte* diskAnalyte = new US_Analyte;
         int diskStatus = diskAnalyte->load( false, analyte.analyteGUID ); // load it from disk
         if ( diskStatus == US_DB2::OK )
            diskAnalyte->write( true, "", db );                            // and write to db
         delete diskAnalyte;
      }

      else if ( status != US_DB2::OK )
         return status;
   }

   // Make sure we have a solutionGUID
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   if ( ! rx.exactMatch( solutionGUID ) )
      solutionGUID = US_Util::new_guid();

   // Check for solutionGUID in database
   solutionID = 0;
   q.clear();
   q  << "get_solutionID_from_GUID"
      << solutionGUID;
   db->query( q );

   status = db->lastErrno();
   if ( status == US_DB2::OK )
   {
      // Edit existing solution entry
      db->next();
      solutionID = db->value( 0 ).toInt();
      q.clear();
      q  << "update_solution"
         << QString::number( solutionID )
         << solutionGUID
         << solutionDesc
         << QString::number( commonVbar20 )
         << QString::number( storageTemp )
         << notes;

      db->statusQuery( q );
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create new solution entry
      q.clear();
      q  << "new_solution"
         << solutionGUID
         << solutionDesc
         << QString::number( commonVbar20 )
         << QString::number( storageTemp )
         << notes
         << QString::number( expID )
         << QString::number( channelID )
         << QString::number( US_Settings::us_inv_ID() );

      db->statusQuery( q );
      solutionID = db->lastInsertID();
   }

   else   // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return status;
   }

   if ( solutionID == 0 )        // double check
      return US_DB2::NO_SOLUTION;

   // new_solutionBuffer will Remove existing buffer associations,
   //   and associate the solution with this buffer
   q.clear();
   q  << "new_solutionBuffer"
      << QString::number( solutionID )
      << QString( "" )           // skip bufferID and use GUID instead
      << bufferGUID;

   status = db->statusQuery( q );
   if ( status != US_DB2::OK )
   {
      qDebug() << "MySQL error associating buffer with solution in database: " 
               << db->lastError();
      return status;
   }

   // Remove analyte associations; we'll create new ones
   q.clear();
   q  << "delete_solutionAnalytes"
      << QString::number( solutionID );

   status = db->statusQuery( q );
   if ( status != US_DB2::OK )
      qDebug() << "MySQL error: " << db->lastError();

   // Now add zero or more analyte associations
   foreach( AnalyteInfo analyte, analytes )
   {
      q.clear();
      q  << "new_solutionAnalyte"
         << QString::number( solutionID )
         << QString( "" )          // skip analyteID and use GUID instead
         << analyte.analyteGUID
         << QString::number( analyte.amount );
   
      status = db->statusQuery( q );
      if ( status != US_DB2::OK )
      {
         qDebug() << "MySQL error associating analyte "
                  << analyte.analyteGUID
                  << " with solution "
                  << solutionGUID
                  << " in database: " 
                  << db->lastError();
         return status;
      }
   }

   saveStatus = ( saveStatus == HD_ONLY ) ? BOTH : DB_ONLY;

   return US_DB2::OK;
}

// Function to delete a solution from disk
void US_Solution::deleteFromDisk( void )
{
   QString filename;
   bool found = diskFilename( solutionGUID, filename );

   if ( ! found )
   {
      // No file to delete
      return;
   }
   
   // Delete it
   QFile file( filename );
   if ( file.exists() )
      file.remove();

   saveStatus = ( saveStatus == BOTH ) ? DB_ONLY : NOT_SAVED;
}

// Function to delete a solution from db
void US_Solution::deleteFromDB( US_DB2* db )
{
   QStringList q;
   if ( solutionID == 0 )
   {
      // Then probably user selected from disk but is trying to delete from db,
      // so let's try to get the db solutionID from the GUID
      q.clear();
      q << "get_solutionID_from_GUID"
        << solutionGUID;
      db->query( q );
   
      if ( db->next() )
         solutionID = db->value( 0 ).toInt();

    }

   if ( solutionID > 0 ) // otherwise it's not there to delete
   {
      q.clear();
      q << "delete_solution"
        << QString::number( solutionID );
   
      int status = db->statusQuery( q );
      if ( status != US_DB2::OK )
         qDebug() << "MySQL error: " << db->lastError();

   }

   saveStatus = ( saveStatus == BOTH ) ? HD_ONLY : NOT_SAVED;
}

// Function to find the file name of a solution on disk, if it exists
bool US_Solution::diskFilename( const QString& guid, QString& filename )
{
   // Get a path and file name for solution
   QString path;
   if ( ! diskPath( path ) )
   {
      qDebug() << "Error: could not create the solutions directory";
      return false;
   }

   QDir        dir( path );
   QStringList filter( "S*.xml" );
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
            if ( xml.name() == "solution" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == guid ) found = true;
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

// Get the path to the solutions.  Create it if necessary.
bool US_Solution::diskPath( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/solutions";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for solutions\n" 
                  << path;
         return false;
      }
   }

   return true;
}

// Function to check if filename already exists, and perhaps generate a new one
QString US_Solution::get_filename(
      const QString& path, bool& newFile )
{
   QDir        f( path );
   QStringList filter( "S???????.xml" );
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
            if ( xml.name() == "solution" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == solutionGUID )
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

   return path + "/S" + QString().sprintf( "%07i", number + 1 ) + ".xml";
}

US_Solution::AnalyteInfo::AnalyteInfo()
{
  analyteID     = 0;
  analyteGUID   = QString( "" );
  analyteDesc   = QString( "" );
  vbar20        = 0.0;
  mw            = 0.0;
  amount        = 1;
  type          = US_Analyte::PROTEIN;
}

bool US_Solution::AnalyteInfo::operator== ( const AnalyteInfo& ai ) const
{
   if ( analyteGUID != ai.analyteGUID ) return false;

   return true;
}

US_Solution::US_Solution& US_Solution::operator=( const US_Solution& rhs )
{
   if ( this != &rhs )            // guard against self assignment
   {
      clear();

      solutionID    = rhs.solutionID;
      solutionGUID  = rhs.solutionGUID;
      solutionDesc  = rhs.solutionDesc;
      bufferID      = rhs.bufferID;
      bufferGUID    = rhs.bufferGUID;
      bufferDesc    = rhs.bufferDesc;
      commonVbar20  = rhs.commonVbar20;
      storageTemp   = rhs.storageTemp;
      notes         = rhs.notes;
      saveStatus    = rhs.saveStatus;

      for ( int i = 0; i < rhs.analytes.size(); i++ )
         analytes << rhs.analytes[ i ];

   }

   return *this;
}

void US_Solution::clear( void )
{
   solutionID   = 0;
   solutionGUID = QString( "" );
   solutionDesc = QString( "" );
   bufferID     = 0;
   bufferGUID   = QString( "" );
   bufferDesc   = QString( "" );
   commonVbar20 = 0.0;
   storageTemp  = 20.0;
   notes        = QString( "" );
   saveStatus   = NOT_SAVED;
   analytes.clear();
}

void US_Solution::show( void )
{
   qDebug() << "solutionID   = " << solutionID   << '\n'
            << "solutionGUID = " << solutionGUID << '\n'
            << "solutionDesc = " << solutionDesc << '\n'
            << "bufferID     = " << bufferID     << '\n'
            << "bufferGUID   = " << bufferGUID   << '\n'
            << "bufferDesc   = " << bufferDesc   << '\n'
            << "commonVbar20 = " << commonVbar20 << '\n'
            << "storageTemp  = " << storageTemp  << '\n'
            << "notes        = " << notes        << '\n'
            << "saveStatus   = " << saveStatus   << '\n';

   qDebug() << "Analytes...";
   qDebug() << "Analytes size = " << QString::number( analytes.size() );
   foreach( AnalyteInfo analyte, analytes )
   {
      qDebug() << "analyteID   = " << analyte.analyteID   << '\n'
               << "analyteGUID = " << analyte.analyteGUID << '\n'
               << "analyteDesc = " << analyte.analyteDesc << '\n'
               << "vbar20      = " << analyte.vbar20      << '\n'
               << "type        = " << analyte.type        << '\n'
               << "mw          = " << analyte.mw          << '\n'
               << "amount      = " << analyte.amount      << '\n';
   }
}
