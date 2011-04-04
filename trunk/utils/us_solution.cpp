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
int US_Solution::readFromDisk( QString& guid )
{
   QString filename;
   bool found = diskFilename( guid, filename );

   if ( ! found )
   {
      qDebug() << "Error: file not found for guid "
               << guid;
      return US_DB2::NO_SOLUTION;
   }
   
   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return US_DB2::NO_SOLUTION;
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
      return US_DB2::ERROR;
   }

   // Load actual buffer and analyte files if we can find them
   US_Buffer newBuffer;
   int status = readBufferDiskGUID( newBuffer, buffer.GUID );

   if ( status != US_DB2::OK )      // Probably US_DB2::NO_BUFFER
      return status;

   // Then we found the actual buffer.xml file
   buffer = newBuffer;

   for ( int i = 0; i < analyteInfo.size(); i++ )
   {
      AnalyteInfo newInfo;
      status = newInfo.analyte.load( false, analyteInfo[ i ].analyte.analyteGUID );

      if ( status != US_DB2::OK )  // Probably US_DB2::NO_ANALYTE
         return status;       

      // Found the analyte.xml file
      newInfo.amount = analyteInfo[ i ].amount;
      analyteInfo[ i ] = newInfo;
   }

   saveStatus = HD_ONLY;

   return US_DB2::OK;
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
            buffer.bufferID    = a.value( "id" )  .toString();
            buffer.GUID        = a.value( "guid" ).toString();
            buffer.description = a.value( "desc" ).toString();
         }
   
         else if ( xml.name() == "analyte" )
         {
            AnalyteInfo ai;

            QXmlStreamAttributes a     = xml.attributes();
            ai.analyte.analyteID       = a.value( "id" ).toString();
            ai.analyte.analyteGUID     = a.value( "guid" ).toString();
            ai.analyte.description     = a.value( "desc" ).toString();
            ai.amount                  = a.value( "amount" ).toString().toDouble();
            ai.analyte.mw              = a.value( "mw" ).toString().toDouble();
            ai.analyte.vbar20          = a.value( "vbar20" ).toString().toDouble();
            QString typetext           = a.value( "type" ).toString();
            ai.analyte.type            = (US_Analyte::analyte_t)
                                         analyte_type( typetext );

            analyteInfo << ai;
      
         }
      }
   }
}

// Function to load a solution from the db
int US_Solution::readFromDB  ( int solutionID, US_DB2* db )
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
         buffer.bufferID    = db->value( 0 ).toString();
         buffer.GUID        = db->value( 1 ).toString();
         buffer.description = db->value( 2 ).toString();
      }
      
      if ( ! buffer.readFromDB( db, buffer.bufferID ) )
         return US_DB2::NO_BUFFER;

      // We need to get a list of analyteGUID's and amounts first
      q.clear();
      q  << "get_solutionAnalyte"
         << QString::number( solutionID );
      db->query( q );

      QList< QString > GUIDs;
      QList< double  > amounts;
      while ( db->next() )
      {
         GUIDs   << db->value( 1 ).toString();
         amounts << db->value( 3 ).toDouble();
      }

      // Now get the rest of the analyte info
      for ( int i = 0; i < GUIDs.size(); i++ )
      {
         AnalyteInfo newInfo;

         int status = newInfo.analyte.load( true, GUIDs[ i ], db );
         if ( status != US_DB2::OK )
            return US_DB2::NO_ANALYTE;

         newInfo.amount = amounts[ i ];
         analyteInfo << newInfo;
      }
   }

   saveStatus = DB_ONLY;

   return US_DB2::OK;
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
      xml.writeAttribute   ( "id",   buffer.bufferID    );
      xml.writeAttribute   ( "guid", buffer.GUID        );
      xml.writeAttribute   ( "desc", buffer.description );
      xml.writeEndElement  ();
   
      // Loop through all the analytes
      for ( int i = 0; i < analyteInfo.size(); i++ )
      {
         AnalyteInfo ai = analyteInfo[ i ];

         xml.writeStartElement( "analyte" );
         xml.writeAttribute   ( "id",   ai.analyte.analyteID   );
         xml.writeAttribute   ( "guid", ai.analyte.analyteGUID );
         xml.writeAttribute   ( "desc", ai.analyte.description );
         xml.writeAttribute   ( "amount", QString::number( ai.amount  ) );
         xml.writeAttribute   ( "mw", QString::number( ai.analyte.mw  ) );
         xml.writeAttribute   ( "vbar20", QString::number( ai.analyte.vbar20  ) );
         xml.writeAttribute   ( "type", analyte_typetext( (int)ai.analyte.type ) );
         xml.writeEndElement  ();
      }

   xml.writeEndElement  ();        // solution
   xml.writeEndDocument ();

   file.close();

   // Save the buffer and analytes to disk
   saveBufferDisk();
   saveAnalytesDisk();

   saveStatus = ( saveStatus == DB_ONLY ) ? BOTH : HD_ONLY;
}

// Function to save solution information to db
int US_Solution::saveToDB( int expID, int channelID, US_DB2* db )
{
   // Save to disk too
   saveToDisk();

   // Let's see if the buffer is in the db already
   QStringList q( "get_bufferID" );
   q  << buffer.GUID;
   db->query( q );

   int status = db->lastErrno();
   if ( status == US_DB2::NOROWS )
      buffer.saveToDB( db, QString::number( 1 ) );      // 1 = private

   else if ( status != US_DB2::OK )
      return status;

   // Now let's see if the analytes are in the db already
   foreach( AnalyteInfo newInfo, analyteInfo )
   {
      q.clear();
      q  << "get_analyteID"
         << newInfo.analyte.analyteGUID;
      db->query( q );

      status = db->lastErrno();
      if ( status == US_DB2::NOROWS )
         newInfo.analyte.write( true, "", db );

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
      << buffer.GUID;

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
   for ( int i = 0; i < analyteInfo.size(); i++ )
   {
      AnalyteInfo newInfo = analyteInfo[ i ];

      q.clear();
      q  << "new_solutionAnalyte"
         << QString::number( solutionID )
         << QString( "" )          // skip analyteID and use GUID instead
         << newInfo.analyte.analyteGUID
         << QString::number( newInfo.amount );
   
      status = db->statusQuery( q );
      if ( status != US_DB2::OK )
      {
         qDebug() << "MySQL error associating analyte "
                  << newInfo.analyte.analyteGUID
                  << " with solution "
                  << solutionGUID
                  << " in database: " 
                  << db->lastError();
         return status;
      }
   }

   saveStatus = BOTH;

   return US_DB2::OK;
}

int US_Solution::readBufferDiskGUID( US_Buffer& buffer, QString& GUID )
{
   // Find the buffer directory on disk
   QDir dir;
   QString path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
      return US_DB2::NO_BUFFER;            // So we have some idea of what happened

   // Try to find the buffer file
   bool    newFile;
   QString filename = US_Buffer::get_filename( path, GUID, newFile );

   if ( newFile ) 
      return US_DB2::NO_BUFFER;

   // Then we can add it
   bool diskStatus = buffer.readFromDisk( filename );   // load it from disk

   return ( diskStatus ) ? US_DB2::OK : US_DB2::NO_BUFFER;
}

void US_Solution::saveBufferDisk( void )
{
   // Find the buffer directory on disk
   QDir dir;
   QString path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
      return;

   // Find the buffer file, or create a new one
   bool    newFile;
   QString filename = US_Buffer::get_filename( path, buffer.GUID, newFile );

   if ( newFile )
   {
      bool diskStatus = buffer.writeToDisk( filename );
      
      if ( diskStatus == false )
        qDebug() << "Buffer write to disk error";
   }
}

void US_Solution::saveAnalytesDisk( void )
{
   // Find the analyte directory on disk
   QString path;
   if ( ! US_Analyte::analyte_path( path ) ) return;

   foreach ( AnalyteInfo ai, analyteInfo )
   {
      // Find the analyte file, or create a new one
      QString filename = US_Analyte::get_filename( path, ai.analyte.analyteGUID );
      int status = ai.analyte.write( false, filename );
      
      if ( status != US_DB2::OK )
      {
         qDebug() << "Analyte " << ai.analyte.analyteGUID
                  << " write to disk error " << status;
      }
   }
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
   amount     = 1;
}

bool US_Solution::AnalyteInfo::operator==( const AnalyteInfo& rhs ) const
{
   if (! ( analyte == rhs.analyte ) ) return false;
   if ( amount  != rhs.amount       ) return false;

   return true;
}

US_Solution& US_Solution::operator=( const US_Solution& rhs )
{
   if ( this != &rhs )            // guard against self assignment
   {
      clear();

      solutionID    = rhs.solutionID;
      solutionGUID  = rhs.solutionGUID;
      solutionDesc  = rhs.solutionDesc;
      buffer        = rhs.buffer;
      commonVbar20  = rhs.commonVbar20;
      storageTemp   = rhs.storageTemp;
      notes         = rhs.notes;
      saveStatus    = rhs.saveStatus;

      for ( int i = 0; i < rhs.analyteInfo.size(); i++ )
         analyteInfo      << rhs.analyteInfo[ i ];
   }

   return *this;
}

void US_Solution::clear( void )
{
   solutionID   = 0;
   solutionGUID = QString( "" );
   solutionDesc = QString( "" );
   buffer.bufferID = QString( "-1" );
   buffer.GUID     = QString( ""  );
   buffer.description = QString( "" );
   commonVbar20 = 0.0;
   storageTemp  = 20.0;
   notes        = QString( "" );
   saveStatus   = NOT_SAVED;
   analyteInfo.clear();
}

void US_Solution::show( void )
{
   qDebug() << "solutionID   = " << solutionID   << '\n'
            << "solutionGUID = " << solutionGUID << '\n'
            << "solutionDesc = " << solutionDesc << '\n'
            << "bufferID     = " << buffer.bufferID << '\n'
            << "buffer GUID  = " << buffer.GUID     << '\n'
            << "buffer Desc  = " << buffer.description << '\n'
            << "commonVbar20 = " << commonVbar20 << '\n'
            << "storageTemp  = " << storageTemp  << '\n'
            << "notes        = " << notes        << '\n'
            << "saveStatus   = " << saveStatus   << '\n';

   qDebug() << "Analytes...";
   qDebug() << "Analytes size = " << QString::number( analyteInfo.size() );
   for ( int i = 0; i < analyteInfo.size(); i++ )
   {
      AnalyteInfo ai = analyteInfo[ i ];

      QString typetext = analyte_typetext( (int)ai.analyte.type );

      qDebug() << "analyteID   = " << ai.analyte.analyteID   << '\n'
               << "analyteGUID = " << ai.analyte.analyteGUID << '\n'
               << "description = " << ai.analyte.description << '\n'
               << "vbar20      = " << ai.analyte.vbar20      << '\n'
               << "type        = " << typetext               << '\n'
               << "mw          = " << ai.analyte.mw          << '\n'
               << "amount      = " << ai.amount              << '\n';
   }
}

int US_Solution::analyte_type( QString antype )
{
   US_Analyte::analyte_t type = US_Analyte::PROTEIN;

   if ( antype == "Protein" )
      type = US_Analyte::PROTEIN;
   else if ( antype == "DNA" )
      type = US_Analyte::DNA;
   else if ( antype == "RNA" )
      type = US_Analyte::RNA;
   else if ( antype == "Other" )
      type = US_Analyte::CARBOHYDRATE;
   
   return (int)type;
}

QString US_Solution::analyte_typetext( int type )
{
   QString antype = "Protein";

   if ( type == (int)US_Analyte::PROTEIN )
      antype = "Protein";
   else if ( type == (int)US_Analyte::DNA )
      antype = "DNA";
   else if ( type == (int)US_Analyte::RNA )
      antype = "RNA";
   else if ( type == (int)US_Analyte::CARBOHYDRATE )
      antype = "Other";

   return antype;
}

