//! \file us_project.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_project.h"

// The constructor clears out the data structure
US_Project::US_Project()
{
   clear();
}

// Function to load a project from the disk
int US_Project::readFromDisk( QString& guid )
{
   QString filename;
   bool found = diskFilename( guid, filename );

   if ( ! found )
   {
      qDebug() << "Error: project file not found -- guid "
               << guid;
      return US_DB2::NO_PROJECT;
   }
   
   QFile file( filename );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {
      qDebug() << "Error: can't open file for reading"
               << filename;
      return US_DB2::NO_PROJECT;
   }

   QXmlStreamReader xml( &file );

   clear();

   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "project" )
         {
            QXmlStreamAttributes a = xml.attributes();
            projectID   = a.value( "id" ).toString().toInt();
            projectGUID = a.value( "guid" ).toString();
            lastUpdated = QFileInfo( filename ).lastModified().toUTC();

            readProjectInfo( xml );
         }
      }
   }

   file.close();

   if ( xml.hasError() ) 
   {
      qDebug() << "Error: xml error: \n"
               << xml.errorString();
      return US_DB2::DBERROR;
   }

   saveStatus = HD_ONLY;

   return US_DB2::OK;
}

void US_Project::readProjectInfo( QXmlStreamReader& xml )
{
   while ( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isEndElement()  &&  xml.name() == "project" ) return;

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "goals" )
         {
            xml.readNext();
            goals = xml.text().toString();
         }

         else if ( xml.name() == "molecules" )
         {
            xml.readNext();
            molecules = xml.text().toString();
         }

         else if ( xml.name() == "purity" )
         {
            xml.readNext();
            purity = xml.text().toString();
         }

         else if ( xml.name() == "expense" )
         {
            xml.readNext();
            expense = xml.text().toString();
         }

         else if ( xml.name() == "bufferComponents" )
         {
            xml.readNext();
            bufferComponents = xml.text().toString();
         }

         else if ( xml.name() == "saltInformation" )
         {
            xml.readNext();
            saltInformation = xml.text().toString();
         }

         else if ( xml.name() == "AUC_questions" )
         {
            xml.readNext();
            AUC_questions = xml.text().toString();
         }

         else if ( xml.name() == "expDesign" )
         {
            xml.readNext();
            expDesign = xml.text().toString();
         }

         else if ( xml.name() == "notes" )
         {
            xml.readNext();
            notes = xml.text().toString();
         }

         else if ( xml.name() == "description" )
         {
            xml.readNext();
            projectDesc = xml.text().toString();
         }
      }
   }
}

// Function to load a project from the db
int US_Project::readFromDB  ( int projectID, US_DB2* db )
{
   // Try to get project info
   QStringList q( "get_project_info" );
   q  << QString::number( projectID );
   db->query( q );

   if ( db->next() )
   {
      this->projectID  = projectID;
      projectGUID      = db->value(  1 ).toString();
      goals            = db->value(  2 ).toString();
      molecules        = db->value(  3 ).toString();
      purity           = db->value(  4 ).toString();
      expense          = db->value(  5 ).toString();
      bufferComponents = db->value(  6 ).toString();
      saltInformation  = db->value(  7 ).toString();
      AUC_questions    = db->value(  8 ).toString();
      notes            = db->value(  9 ).toString();
      projectDesc      = db->value( 10 ).toString();
      status           = db->value( 11 ).toString();
      // value 12 is personID
      expDesign        = db->value( 13 ).toString();
      lastUpdated      = db->value( 14 ).toDateTime();

   }

   else
      return US_DB2::NO_PROJECT;

   saveStatus = DB_ONLY;

   return US_DB2::OK;
}

// Function to save project information to disk
void US_Project::saveToDisk( void )
{
   // First make sure we have a GUID
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.exactMatch( projectGUID ) )
      projectGUID = US_Util::new_guid();

   // Get a path and file name for project
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
   xml.writeDTD("<!DOCTYPE US_Project>");
   xml.writeStartElement("ProjectData");
   xml.writeAttribute("version", "1.0");

   // elements
   xml.writeStartElement( "project" );
   xml.writeAttribute   ( "id", QString::number( projectID ) );
   xml.writeAttribute   ( "guid", projectGUID );

   xml.writeTextElement( "goals",            goals            );
   xml.writeTextElement( "molecules",        molecules        );
   xml.writeTextElement( "purity",           purity           );
   xml.writeTextElement( "expense",          expense          );
   xml.writeTextElement( "bufferComponents", bufferComponents );
   xml.writeTextElement( "saltInformation",  saltInformation  );
   xml.writeTextElement( "AUC_questions",    AUC_questions    );
   xml.writeTextElement( "expDesign",        expDesign        );
   xml.writeTextElement( "notes",            notes            );
   xml.writeTextElement( "description",      projectDesc      );

   xml.writeEndElement  ();        // project
   xml.writeEndDocument ();

   file.close();

   saveStatus = ( saveStatus == DB_ONLY ) ? BOTH : HD_ONLY;
}

// Function to save project information to db
int US_Project::saveToDB( US_DB2* db )
{
   // Save it to disk too
   saveToDisk();

   // Check for GUID in database
   projectID = 0;
   QStringList q( "get_projectID_from_GUID" );
   q << projectGUID;
   db->query( q );

   int db_status = db->lastErrno();
   if ( db_status == US_DB2::OK )
   {
      // Edit existing project entry
      db->next();
      projectID = db->value( 0 ).toInt();
      q.clear();
      q  << "update_project2"
         << QString::number( projectID )
         << projectGUID
         << goals
         << molecules
         << purity
         << expense
         << bufferComponents
         << saltInformation
         << AUC_questions
         << expDesign
         << notes
         << projectDesc
         << status;

      db->statusQuery( q );
   }

   else if ( db_status == US_DB2::NOROWS )
   {
      // Create new project entry
      q.clear();
      q  << "new_project2"
         << projectGUID
         << goals
         << molecules
         << purity
         << expense
         << bufferComponents
         << saltInformation
         << AUC_questions
         << expDesign
         << notes
         << projectDesc
         << status
         << QString::number( US_Settings::us_inv_ID() );


      db->statusQuery( q );
      projectID = db->lastInsertID();
   }

   else   // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return db_status;
   }

   if ( projectID == 0 )        // double check
      return US_DB2::NO_PROJECT;

   saveStatus = BOTH;

   return US_DB2::OK;
}

// Function to save project information to db
int US_Project::saveToDB_auto( int invID_passed, US_DB2* db )
{
   // Save it to disk too
   saveToDisk();

   // Check for GUID in database
   projectID = 0;
   QStringList q( "get_projectID_from_GUID" );
   q << projectGUID;
   db->query( q );

   int db_status = db->lastErrno();
   if ( db_status == US_DB2::OK )
   {
      // Edit existing project entry
      db->next();
      projectID = db->value( 0 ).toInt();
      q.clear();
      q  << "update_project2"
         << QString::number( projectID )
         << projectGUID
         << goals
         << molecules
         << purity
         << expense
         << bufferComponents
         << saltInformation
         << AUC_questions
         << expDesign
         << notes
         << projectDesc
         << status;

      db->statusQuery( q );
   }

   else if ( db_status == US_DB2::NOROWS )
   {
      // Create new project entry
      q.clear();
      q  << "new_project2"
         << projectGUID
         << goals
         << molecules
         << purity
         << expense
         << bufferComponents
         << saltInformation
         << AUC_questions
         << expDesign
         << notes
         << projectDesc
         << status
	 << QString::number( invID_passed ); //ALEXEY: passed invID, NOT the US_Settings ::us_inv_ID()


      db->statusQuery( q );
      projectID = db->lastInsertID();
   }

   else   // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return db_status;
   }

   if ( projectID == 0 )        // double check
      return US_DB2::NO_PROJECT;

   saveStatus = BOTH;

   return US_DB2::OK;
}


// Function to delete a project from disk
void US_Project::deleteFromDisk( void )
{
   QString filename;
   bool found = diskFilename( projectGUID, filename );

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

// Function to delete a project from db
void US_Project::deleteFromDB( US_DB2* db )
{
   QStringList q;
   if ( projectID == 0 )
   {
      // Then probably user selected from disk but is trying to delete from db,
      // so let's try to get the db projectID from the GUID
      q.clear();
      q << "get_projectID_from_GUID"
        << projectGUID;
      db->query( q );
   
      if ( db->next() )
         projectID = db->value( 0 ).toInt();

    }

   if ( projectID > 0 ) // otherwise it's not there to delete
   {
      q.clear();
      q << "delete_project"
        << QString::number( projectID );
   
      int status = db->statusQuery( q );
      if ( status != US_DB2::OK )
         qDebug() << "MySQL error: " << db->lastError();

   }

   clear();

   saveStatus = ( saveStatus == BOTH ) ? HD_ONLY : NOT_SAVED;
}

// Function to find the file name of a project on disk, if it exists
bool US_Project::diskFilename( const QString& guid, QString& filename )
{
   // Get a path and file name for project
   QString path;
   if ( ! diskPath( path ) )
   {
      qDebug() << "Error: could not create the projects directory";
      return false;
   }

   QDir        dir( path );
   QStringList filter( "P*.xml" );
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
            if ( xml.name() == "project" )
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

// Get the path to the projects.  Create it if necessary.
bool US_Project::diskPath( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/projects";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for projects\n" 
                  << path;
         return false;
      }
   }

   return true;
}

// Function to check if filename already exists, and perhaps generate a new one
QString US_Project::get_filename(
      const QString& path, bool& newFile )
{
   QDir        f( path );
   QStringList filter( "P???????.xml" );
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
            if ( xml.name() == "project" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "guid" ).toString() == projectGUID )
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

   return path + "/P" + QString().sprintf( "%07i", number + 1 ) + ".xml";
}

void US_Project::clear( void )
{
   projectID   = 0;
   projectGUID = QString( "" );
   goals       = QString( "" );
   molecules   = QString( "" );
   purity      = QString( "" );
   expense     = QString( "" );
   bufferComponents = QString( "" );
   saltInformation  = QString( "" );
   AUC_questions    = QString( "" );
   expDesign   = QString( "" );
   notes       = QString( "" );
   projectDesc = QString( "" );
   status      = QString( "submitted" );

   saveStatus   = NOT_SAVED;
}

void US_Project::show( void )
{
   qDebug() << "projectID        = " << projectID        << '\n'
            << "projectGUID      = " << projectGUID      << '\n'
            << "goals            = " << goals            << '\n'
            << "molecules        = " << molecules        << '\n'
            << "purity           = " << purity           << '\n'
            << "expense          = " << expense          << '\n'
            << "bufferComponents = " << bufferComponents << '\n'
            << "saltInformation  = " << saltInformation  << '\n'
            << "AUC_questions    = " << AUC_questions    << '\n'
            << "expDesign        = " << expDesign        << '\n'
            << "notes            = " << notes            << '\n'
            << "projectDesc      = " << projectDesc      << '\n'
            << "status           = " << status           << '\n'
            << "saveStatus       = " << saveStatus       << '\n';
}
