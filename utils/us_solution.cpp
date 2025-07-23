//! \file us_solution.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_solution.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_datafiles.h"
#include "us_math2.h"
#include "us_eprofile.h"

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
      qDebug() << "Error: solution file not found - guid "
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
      return US_DB2::DBERROR;
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
   buffer.GUID = "";

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

   QString compType("Sample");
   QString valType("molarExtinction");
   US_ExtProfile::fetch_eprofile( db, solutionID, compType, valType, extinction );

   saveStatus = DB_ONLY;

   return US_DB2::OK;
}

// Function to save solution information to disk
void US_Solution::saveToDisk( void )
{
   // Check that we have a solution with valid values
   if ( solutionDesc.isEmpty()     ||
        buffer.GUID.isEmpty()      ||
        buffer.bufferID.isEmpty()  ||
        buffer.bufferID.toInt() < 0 )
   {
      qDebug() << "Error: Solution cannot be written to disk,"
               << " due to invalid values:";
      qDebug() << "  commonVbar20" << commonVbar20;
      qDebug() << "  solutionDesc" << solutionDesc;
      qDebug() << "  bufferGUID" << buffer.GUID;
      qDebug() << "  bufferID" << buffer.bufferID;
      return;
   }

   // First make sure we have a GUID
   static const QRegularExpression rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );

   if ( ! rx.match( solutionGUID ).hasMatch() )
      solutionGUID = US_Util::new_guid();

   // Get a path and file name for solution
   QString path;
   if ( ! diskPath( path ) ) return;

   bool    newFile;
   QString filename = US_DataFiles::get_filename( path, solutionGUID,
                         "S", "solution", "guid", newFile );

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
   QStringList q;
   int status;

   // Save to disk too
   saveToDisk();
qDebug() << "SolSvDB: ToDisk complete" << "expID,chnID" << expID << channelID;

   if ( ! buffer.GUID.isEmpty() )
   {
      // Let's see if the buffer is in the db already
      q  <<  "get_bufferID";
      q  << buffer.GUID;
      db->query( q );

      status = db->lastErrno();
qDebug() << "SolSvDB: bufID stat" << status;

      if ( status == US_DB2::NOROWS )
         buffer.saveToDB( db, QString::number( 1 ) );      // 1 = private

      else if ( status != US_DB2::OK )
         return status;
   }

   // Now let's see if the analytes are in the db already
   foreach( AnalyteInfo newInfo, analyteInfo )
   {
      q.clear();
      q  << "get_analyteID"
         << newInfo.analyte.analyteGUID;
      db->query( q );

      status = db->lastErrno();
qDebug() << "SolSvDB: anaID stat" << status;
      if ( status == US_DB2::NOROWS )
         newInfo.analyte.write( true, "", db );

      else if ( status != US_DB2::OK )
         return status;
   }

   // Make sure we have a solutionGUID
   static const QRegularExpression rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   if ( ! rx.match( solutionGUID ).hasMatch() )
      solutionGUID = US_Util::new_guid();

   // Check for solutionGUID in database
   solutionID = 0;
   q.clear();
   q  << "get_solutionID_from_GUID"
      << solutionGUID;
   db->query( q );
   db->next();

   status = db->lastErrno();
qDebug() << "SolSvDB: soGID stat" << status;
   if ( status == US_DB2::OK )
   {
      // Edit existing solution entry
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
qDebug() << "SolSvDB: updSO solID" << solutionID << "lastErr" << db->lastError() << db->lastErrno();
   }

   else if ( status == US_DB2::NOROWS  &&
             !solutionDesc.isEmpty()  &&
             !solutionDesc.startsWith( "New Sol" ) )
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
qDebug() << "SolSvDB: newSO newID" << solutionID << "lastErr" << db->lastError()
 << db->lastErrno() << "expID" << expID << "chnID" << channelID;
qDebug() << "SolSvDB: newSO q" << q;
   }

   else   // unspecified error
   {
      qDebug() << "MySQL error: " << db->lastError();
      return status;
   }

   if ( solutionID == 0 )        // double check
      return US_DB2::NO_SOLUTION;

   if ( ! buffer.GUID.isEmpty() )
   {
      // new_solutionBuffer will Remove existing buffer associations,
      //   and associate the solution with this buffer
      q.clear();
      q  << "new_solutionBuffer"
         << QString::number( solutionID )
         << QString( "0" )        // skip bufferID and use GUID instead
         << buffer.GUID;

      status = db->statusQuery( q );
qDebug() << "SolSvDB: soBuf stat" << status << db->lastError() << "bGID" << buffer.GUID
 << "solID" << solutionID;
qDebug() << "SolSvDB: soBuf q=" << q;
      if ( status != US_DB2::OK )
      {
         qDebug() << "MySQL error associating buffer with solution in database:"
                  << db->lastError() << status;
         return status;
      }
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
         << QString( "0" )          // skip analyteID and use GUID instead
         << newInfo.analyte.analyteGUID
         << QString::number( newInfo.amount );
   
      status = db->statusQuery( q );
qDebug() << "SolSvDB: soAna stat" << status << db->lastError();
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

   /// Write extinctionProfile /////////////////////////////
   QString compType("Sample");
   QString valType("molarExtinction");
   qDebug() << "SampleID for extProfile: " << solutionID;
   
   //if ( !extinction.isEmpty() )
   if ( !extinction.isEmpty() and new_or_changed_spectrum ) 
   {
      if ( !replace_spectrum )
      {
         qDebug() << "Creating Spectrum!!!";
         US_ExtProfile::create_eprofile( db, solutionID, compType, valType, extinction);
      }
      else
      {
         qDebug() << "Updating Spectrum!!!";

         QMap<double, double> new_extinction = extinction;
         int profileID = US_ExtProfile::fetch_eprofile(  db, solutionID, compType, valType, extinction);
         
         qDebug() << "Old Extinction keys: " << extinction.keys().count() << ", ProfileID: " << profileID;
         US_ExtProfile::update_eprofile( db, profileID, solutionID, compType, valType, new_extinction);
         qDebug() << "New Extinction keys: " << new_extinction.keys().count() << ", ProfileID: " << profileID;
         
         replace_spectrum = false;
      }
      new_or_changed_spectrum = false; 
   }
   ////////////////////////////////////////    


   saveStatus = BOTH;
qDebug() << "SolSvDB: END: svstat" << saveStatus;

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
   if ( buffer.GUID.isEmpty() )  return;

   // Find the buffer directory on disk
   QDir dir;
   QString path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for buffers\n" 
                  << path;
         return ;
      }
   }

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
   QDir dir;
   QString path = US_Settings::dataDir() + "/analytes";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         qDebug() << "Error: Could not create default directory for analytes\n" 
                  << path;
         return;
      }
   }


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
int US_Solution::deleteFromDisk( void )
{
   int status = US_DB2::OK;
   QString filename;
   bool found = diskFilename( solutionGUID, filename );

   if ( ! found )
   {
      // No file to delete
      return US_DB2::NO_SOLUTION;
   }
   
   if ( solutionInUse( solutionGUID ) )
      return US_DB2::SOLUT_IN_USE;

   // Delete it
   QFile file( filename );
   if ( file.exists() )
      file.remove();

   saveStatus = ( saveStatus == BOTH ) ? DB_ONLY : NOT_SAVED;
   return status;
}

// Function to delete a solution from db
int US_Solution::deleteFromDB( US_DB2* db )
{
   int status = US_DB2::OK;
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

      status = db->statusQuery( q );

      if ( status == US_DB2::SOLUT_IN_USE )
         return status;

      if ( status != US_DB2::OK )
         qDebug() << "MySQL error: " << db->lastError();

   }

   saveStatus = ( saveStatus == BOTH ) ? HD_ONLY : NOT_SAVED;
   return status;
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

   bool newFile = true;
   filename =
      US_DataFiles::get_filename( path, guid, "S", "solution", "guid",
                                  newFile );
//qDebug() << "SOL: dkFn: path" << path << "guid" << guid << "newFile" << newFile;

   return ( ! newFile );
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
   QString filename = US_DataFiles::get_filename( path, solutionGUID,
                         "S", "solution", "guid", newFile );
   return filename;
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
   replace_spectrum = false;
   new_or_changed_spectrum = false;

   solutionID   = 0;
   solutionGUID = QString( "" );
   solutionDesc = QString( "" );
   buffer.bufferID    = QString( "-1" );
   buffer.GUID        = QString( ""  );
   buffer.description = QString( "" );
   buffer.density     = 0.0;
   buffer.viscosity   = 0.0;
   commonVbar20 = 0.0;
   storageTemp  = 20.0;
   notes        = QString( "" );
   saveStatus   = NOT_SAVED;
   analyteInfo.clear();

   extinction.clear();
}

void US_Solution::show( void )
{
   qDebug() << "solutionID   = " << solutionID   << '\n'
            << "solutionGUID = " << solutionGUID << '\n'
            << "solutionDesc = " << solutionDesc << '\n'
            << "bufferID     = " << buffer.bufferID << '\n'
            << "buffer GUID  = " << buffer.GUID     << '\n'
            << "buffer Desc  = " << buffer.description << '\n'
            << "buffer density = " << buffer.density << '\n'
            << "buffer viscosity = " << buffer.viscosity << '\n'
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

// Function to determine if a solution is used by any experiment
bool US_Solution::solutionInUse( QString& solutionGUID )
{
   bool in_use = false;

   QString     resdir  = US_Settings::resultDir();
   QStringList expdirs = QDir( resdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   resdir = resdir + "/";

   for ( int ii = 0; ii < expdirs.size();  ii++ )
   {
      QString     subdir  = resdir + expdirs.at( ii );
      QStringList expfilt;
      expfilt << expdirs.at( ii ) + ".*.xml";
      QStringList expfiles = QDir( subdir )
         .entryList( QDir::Files, QDir::Name );
      subdir = subdir + "/";

      for ( int jj = 0; jj < expfiles.size(); jj++ )
      {
         QString fname      = subdir + expfiles.at( jj );
         QFile xfile( fname  );
         if ( !  xfile.open( QIODevice::ReadOnly ) )
            continue;

         QXmlStreamReader xml( &xfile );

         while ( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "solution" )
            {
               QXmlStreamAttributes atts = xml.attributes();
               QString guid = atts.value( "guid" ).toString();

               if ( guid == solutionGUID )
               {
                  in_use = true;
                  break;
               }
            }
         }

         xfile.close();

         if ( in_use ) break;
      }

      if ( in_use ) break;
   }

   return in_use;
}

// Function to count solution occurrences in protocols
int US_Solution::countInProtocols( US_DB2* dbP )
{
   int kntSols      = 0;

   if ( dbP != NULL )
   {  // Get and search XML from each DB protocol record
      QStringList qry;
      qry << "get_protocol_desc" << QString::number( US_Settings::us_inv_ID() );
      dbP->query( qry );

      if ( dbP->lastErrno() != US_DB2::OK )
         return 0;    // Error exit:  unable to read DB record

      while ( dbP->next() )
      {
         QString xmlStr   = dbP->value( 3 ).toString();
         if ( solutionInProtocol( xmlStr, false ) )
            kntSols++;
qDebug() << "kntInPro:  DB: kntSols" << kntSols << dbP->value(2).toString();
      }
   }

   else
   {  // Get and search XML from each local protocol file
      QString datdir      = US_Settings::dataDir() + "/projects/";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      QStringList rfilt( "R*.xml" );      // "~/ultrascan/data/projects/R*.xml"
      QStringList pfiles  = QDir( datdir ).entryList(
                                              rfilt, QDir::Files, QDir::Name );
      int nfiles          = pfiles.count();

      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each "R000*.xml" file in the directory
         QString pfpath      = datdir + pfiles[ ii ];
         QFile filei( pfpath );
         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text) )
         {
            continue;
         }

         QTextStream tsi( &filei );
         QString xmlStr  = tsi.readAll();
         filei.close();
         if ( solutionInProtocol( xmlStr, true ) )
            kntSols++;
qDebug() << "kntInPro:  Disk: kntSols" << kntSols << pfiles[ii];
      }
   }

   return kntSols;
}

// Function to determine if a solution is found in a protocol definition (XML)
bool US_Solution::solutionInProtocol( const QString protoXml, const bool useName )
{
   bool solFound    = false;
   QString solMatch = useName ? solutionDesc :
                      QString::number( solutionID );
   QXmlStreamReader xmli( protoXml );

   while( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isStartElement()  &&  xmli.name() == "solution" )
      {
         QXmlStreamAttributes atts = xmli.attributes();
         QString name = atts.value( "name" ).toString();
         QString soid = atts.value( "id"   ).toString();
         QString test = useName ? name : soid;

         if ( test == solMatch )
         {  // Solution match found:  flag and break
            solFound     = true;
qDebug() << "solInPro: MATCH" << name << soid << test;
            break;
         }
      }
   }

   return solFound;
}

