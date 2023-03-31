//! \file us_protocol_util.cpp

#include "us_protocol_util.h"


// Build a list of string lists containing basic protocol
// information for all records as needed to present a dialog list.
//
// This function reads all protocol records from the database
// or from local disk. It then composes a list of QStringLists,
// each of which contains protocol_name, date, db_id/file_name, guid.
//
// protdata  Output list of QStringLists for each protocol.
// dbP       Pointer to DB connector or NULL to read from disk.
// returns   The count of records listed.
int US_ProtocolUtil::list_all( QList< QStringList >& protdata,
                               US_DB2* dbP )
{
   int nrecs        = 0;    // Count of records found
   protdata.clear();        // Initialize list of entries

qDebug() << "PU:l_all: dbP" << dbP;
   if ( dbP != NULL )
   {  // Read protocol records from the database
      QString inv_id      = QString::number( US_Settings::us_inv_ID() );
      QStringList qry;
      qry << "get_protocol_desc" << inv_id;
      dbP->query( qry );
      
      qDebug() << "PU:l_all: qry" << qry;
      qDebug() << "PU:l_all:  qry stat" << dbP->lastError();
      qDebug() << "PU:l_all:  qry errno, US_DB2::OK" << dbP->lastErrno() << US_DB2::OK;
 
      if ( dbP->lastErrno() != US_DB2::OK )
         return nrecs;

      while ( dbP->next() )
      {
         QStringList protentry;
         QString pdbid    = dbP->value( 0 ).toString();
         QString pguid    = dbP->value( 1 ).toString();
         QString pname    = dbP->value( 2 ).toString();
         QDateTime date   = dbP->value( 5 ).toDateTime().toUTC();
//         QString pdate    = US_Util::toUTCDatetimeText( date
//                               .toString( Qt::ISODate ), true )
//                               .section( " ", 0, 0 ).simplified();
         QString pdate    = US_Util::toUTCDatetimeText( date
                               .toString( Qt::ISODate ), true )
                               .section( ":", 0, 1 ) + " UTC";
         protentry << pname << pdate << pdbid << pguid;
qDebug() << "PU:l_all:  protentry" << protentry;

         protdata  << protentry;
         nrecs++;
      }
   }

   else
   {  // Read protocol records from local disk
      QString datdir      = US_Settings::dataDir() + "/projects/";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      QStringList rfilt( "R*.xml" );      // "~/ultrascan/data/projects/R*.xml"
      QStringList pfiles  = QDir( datdir ).entryList(
                                              rfilt, QDir::Files, QDir::Name );
      int nfiles          = pfiles.count();

      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each "R000*.xml" file in the directory
         QStringList protentry;
         QString protname;
         QString prot_id;
         QString pfname      = pfiles[ ii ];
         QString pfpath      = datdir + pfname;
//         QString fdate       = US_Util::toUTCDatetimeText(
//                                  QFileInfo( pfpath ).lastModified().toUTC()
//                                  .toString( Qt::ISODate ), true )
//                                  .section( " ", 0, 0 ).simplified();
         QString fdate       = US_Util::toUTCDatetimeText(
                                  QFileInfo( pfpath ).lastModified().toUTC()
                                  .toString( Qt::ISODate ), true )
                                  .section( ":", 0, 1 ) + " UTC";
         QFile pfile( pfpath );
         // Skip if there is a file-open problem
         if ( ! pfile.open( QIODevice::ReadOnly ) )  continue;

         QXmlStreamReader xmli( &pfile );

         while( ! xmli.atEnd() )
         {  // Parse XML for description and guid
            xmli.readNext();
            QString ename       = xmli.name().toString();
            if ( xmli.isStartElement()  &&  ename == "protocol" )
            {
               QXmlStreamAttributes attr = xmli.attributes();
               protname            = attr.value( "description" ).toString();
               prot_id             = attr.value( "guid" ).toString();
               break;
            }
         }

         protentry << protname << fdate << pfname << prot_id;
         protdata  << protentry;
         nrecs++;
      }  // END: file loop
   }  // END: local disk

qDebug() << "PU:l_all: nrecs" << nrecs;
   return nrecs;
}

int US_ProtocolUtil::list_all_auto( QList< QStringList >& protdata,
				    US_DB2* dbP )
{
  int nrecs        = 0;    // Count of records found
  protdata.clear();        // Initialize list of entries

qDebug() << "[AUTO] PU:l_all: dbP" << dbP;
 if ( dbP != NULL )
   {  // Read protocol records from the database
     QString inv_id      = QString::number( US_Settings::us_inv_ID() );
     QStringList qry;
     qry << "get_protocol_desc_auto" << inv_id;
     dbP->query( qry );
     
     qDebug() << "[AUTO] PU:l_all: qry" << qry;
     qDebug() << "[AUTO] PU:l_all:  qry stat" << dbP->lastError();
     qDebug() << "[AUTO] PU:l_all:  qry errno, US_DB2::OK" << dbP->lastErrno() << US_DB2::OK;
     
     if ( dbP->lastErrno() != US_DB2::OK )
       return nrecs;
     
     while ( dbP->next() )
       {
         QStringList protentry;
         QString pdbid    = dbP->value( 0 ).toString();
         QString pguid    = dbP->value( 1 ).toString();
         QString pname    = dbP->value( 2 ).toString();
         QDateTime date   = dbP->value( 5 ).toDateTime().toUTC();
	 //         QString pdate    = US_Util::toUTCDatetimeText( date
	 //                               .toString( Qt::ISODate ), true )
	 //                               .section( " ", 0, 0 ).simplified();
         QString pdate = US_Util::toUTCDatetimeText( date
						     .toString( Qt::ISODate ), true )
	   .section( ":", 0, 1 ) + " UTC";
         protentry << pname << pdate << pdbid << pguid;
	 qDebug() << "[AUTO] PU:l_all:  protentry" << protentry;
	 
         protdata  << protentry;
         nrecs++;
      }
   }
 qDebug() << "[AUTO] PU:l_all: nrecs" << nrecs;
 return nrecs;
}



// Update the protocol data list with a new entry.
//
// This adds a new entry to the list of protocol summary entries or
// returns a negative index if the protocol name is already in the list.
//
// protentry  Protocol entry to add.
// protdata   Input/output list of protocol summary data entries.
// returns    The index in the list of the new record (<0->error).
int US_ProtocolUtil::update_list( const QStringList protentry,
                                  QList< QStringList >& protdata )
{
   int nrecs        = protdata.count();   // Count of entries in list
   int newpx        = nrecs;              // Default index of added entry

   for ( int ii = 0; ii < nrecs; ii++ )
   {
      if ( protentry[ 0 ] == protdata[ ii ][ 0 ] )
      {  // *ERROR* A name match already exists!
         newpx            = -( ii + 1 );  // Flag: negative of ii+1
         return newpx;    // Return with error flag and no update
      }
   }

   // The entry's name is unmatched:  append the entry to the list
   protdata << protentry;

   return newpx;          // Success flag:  index of new entry
}

// Write a new protocol record to database or disk.
//
// This function adds a new record, as embodied in the input XML
// string. The XML forms the bulk of the actual record. In the case
// of database upload, it is parsed to determine the other fields
// to add to the database protocol table.
//
// xml       XML string comprising protocol record information.
// dbP       Pointer to DB connector or NULL to write to disk.
// returns   The db_id or filename numeric of the new record.
int US_ProtocolUtil::write_record( const QString xml,
                                   US_DB2* dbP )
{
   int newpx        = -1;   // Default new protocol record index

   // Parse the XML for protocol name and other information
   QString protname;
   QString protguid;
   QString investig;
   QString personID;
   QString ohost;
   QString speed1;
   QString solut1;
   QString solut2;
   QString rotorID( "-1" );
   int nspeeds      = 0;
   int nucells      = 0;
   int escans       = 0;
   int nwvlens      = 0;
   int nsolut       = 0;
   double duration  = 0.0;
   QStringList lambdas;
   QXmlStreamReader xmli( xml );
qDebug() << "PU:wrrec: len(xml)" << xml.length() << "dbP" << dbP;

   while( ! xmli.atEnd() )
   {  // Parse XML
      xmli.readNext();
      if ( xmli.isStartElement() )
      {
         QXmlStreamAttributes attr = xmli.attributes();
         QString ename    = xmli.name().toString();
         if ( ename == "protocol" )
         {
            protname         = attr.value( "description"  ).toString();
            protguid         = attr.value( "guid"         ).toString();
            ohost            = attr.value( "optima_host"  ).toString();
            investig         = attr.value( "investigator" ).toString();
         }
         else if ( ename == "rotor" )
         {
            rotorID          = attr.value( "rotid" ).toString();
         }
         else if ( ename == "speedstep" )
         {
            nspeeds++;
            double dura      = attr.value( "duration_minutes" ).toString()
                                                               .toDouble();
            double delay     = attr.value( "delay_seconds"    ).toString()
                                                               .toDouble();
            dura            *= 60.0;
            duration        += ( dura + delay );
            int sescans      = qRound( dura * 0.1 );
            escans          += sescans;
            if ( nspeeds == 1 )
               speed1           = attr.value( "rotorspeed" ).toString();
         }
         else if ( ename == "cell" )
         {
            nucells++;
         }
         else if ( ename == "solution" )
         {
            nsolut++;
            if ( nsolut == 1 )
               solut1           = attr.value( "name" ).toString();
            else if ( nsolut == 2 )
               solut2           = attr.value( "name" ).toString();
         }
         else if ( ename == "point" )
         {
            QString lambda   = attr.value( "lambda" ).toString();
            if ( ! lambdas.contains( lambda ) )
               lambdas << lambda;
                
         }
      }  // END: start Element
   }  // END:  Xml parse

   // Person ID parsed from investigator
   personID         = investig.section( ":", 0, 0 ).simplified();
   // Count of unique wavelengths encountered
   nwvlens          = lambdas.count();
   // Total run duration in minutes rounded to two significant digits
   duration         = qRound( ( duration / 60.0 ) * 100.0 ) * 0.01;
qDebug() << "PU:wrrec:  personID" << personID << "protguid" << protguid;

   if ( dbP != NULL )
   {  // Write a record to the database
      QStringList qry( "new_protocol" );
      qry << personID
          << protguid
          << protname
          << xml
          << ohost
          << rotorID
          << speed1
          << QString::number( duration )
          << QString::number( nucells )
          << QString::number( escans )
          << solut1
          << solut2
          << QString::number( nwvlens );

      dbP->query( qry );
qDebug() << "PU:wrrec:  qry" << qry;
qDebug() << "PU:wrrec:   stat" << dbP->lastErrno() << dbP->lastError();

      if ( dbP->lastErrno() != US_DB2::OK )
         return newpx;

      newpx            = dbP->lastInsertID();
qDebug() << "PU:wrrec:    newpx" << newpx;
   }

   else
   {  // Write a record to local disk
      QString datdir      = US_Settings::dataDir() + "/projects";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      QString filename    = US_DataFiles::get_filename( datdir, protguid, "R",
                                                        "protocol", "guid" );
      QFile pfile( filename );
qDebug() << "PU:wrrec:    filename" << filename;

      if ( ! pfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
         return newpx;

      QTextStream tso( &pfile );
      tso << xml;

      pfile.close();

      // Returned protocol index is numeric part of output file name
      newpx               = QString( filename ).section( "/", -1, -1 )
                                               .section( ".", 0, 0 ).mid( 1 ).toInt();
   }

qDebug() << "PU:wrrec:    rtn newpx" << newpx;
   return newpx;
}

// Read in a protocol XML and entry, as indicated by a given name.
//
// This function uses a protocol name (description text) to find a
// protocol record, read its XML, and compose its summary data entry.
//
// protname   Protocol name of record to find and fetch.
// xml        Pointer to XML string read from the found record.
// protentry  Pointer to summary protocol created.
// dbP        Pointer to DB connector or NULL to write to disk.
// returns    Database Id of record or filename numeric (-ve. -> error).
int US_ProtocolUtil::read_record( const QString protname, QString* xml,
                                  QStringList* protentry, US_DB2* dbP )
{
   int idprot       = -1;

   if ( xml != NULL )
      xml      ->clear();
   if ( protentry != NULL )
      protentry->clear();

   if ( dbP != NULL )
   {  // Find the record in the database with a matching name
      QStringList qry;
      qry << "get_protocol_desc" << QString::number( US_Settings::us_inv_ID() );
      dbP->query( qry );

      if ( dbP->lastErrno() != US_DB2::OK )
         return idprot;    // Error exit:  unable to read DB record

      while ( dbP->next() )
      {
         QString pname    = dbP->value( 2 ).toString();

         if ( pname == protname )
         {  // Name match:  store information and break out
            QString pdbid    = dbP->value( 0 ).toString();
            QString pguid    = dbP->value( 1 ).toString();
            QDateTime date   = dbP->value( 5 ).toDateTime().toUTC();
            QString pdate    = US_Util::toUTCDatetimeText( date
                                  .toString( Qt::ISODate ), true )
                                  .section( " ", 0, 0 ).simplified();

            // Return entry, xml, and protocol ID
            if ( protentry != NULL )
               *protentry << pname << pdate << pdbid << pguid;
            if ( xml != NULL )
               *xml             = dbP->value( 3 ).toString();

            idprot           = pdbid.toInt();
            break;
         }
      }  // END: db records
   }  // END: from database

   else
   {  // Find the record in a local file with a matching name
      QString datdir      = US_Settings::dataDir() + "/projects/";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      QStringList rfilt( "R*.xml" );      // "~/ultrascan/data/projects/R*.xml"
      QStringList pfiles  = QDir( datdir ).entryList(
                                              rfilt, QDir::Files, QDir::Name );
      int nfiles          = pfiles.count();

      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each "R000*.xml" file in the directory
         QString pname;
         QString pguid;  
         QString pfname      = pfiles[ ii ];
         QString pfpath      = datdir + pfname;
         QFile pfile( pfpath );
         // Skip if there is a file-open problem
         if ( ! pfile.open( QIODevice::ReadOnly ) )  continue;

         // Capture the XML as a string and start XML reader
         QTextStream tsi( &pfile );
         QString xmlstr      = tsi.readAll();
         pfile.close();
         QXmlStreamReader xmli( xmlstr );

         while( ! xmli.atEnd() )
         {  // Parse XML for description and guid
            xmli.readNext();
            QString ename       = xmli.name().toString();
            if ( xmli.isStartElement()  &&  ename == "protocol" )
            {
               QXmlStreamAttributes attr = xmli.attributes();
               pname               = attr.value( "description" ).toString();
               pguid               = attr.value( "guid" ).toString();
               break;
            }
         }

         if ( pname == protname )
         {  // Name match:  store information and break out
            QString fdate       = US_Util::toUTCDatetimeText(
                                  QFileInfo( pfpath ).lastModified().toUTC()
                                  .toString( Qt::ISODate ), true )
                                  .section( " ", 0, 0 ).simplified();

            // Return entry, xml, and protocol ID (negative of filename numeric)
            if ( protentry != NULL )
               *protentry << pname << fdate << pfname << pguid;
            if ( xml != NULL )
               *xml             = xmlstr;

            idprot           = -( QString( pfname ).section( ".", 0, 0 ).mid( 1 ).toInt() );
            break;
         }
      }  // END: file loop
   }  // END: local disk

   return idprot;
}

// Copy of the read_record for autoflow when inv_ID is propagated 
int US_ProtocolUtil::read_record_auto( const QString protname, int invID_passed, QString* xml,
				       QStringList* protentry, US_DB2* dbP )
{
   int idprot       = -1;

   if ( xml != NULL )
      xml      ->clear();
   if ( protentry != NULL )
      protentry->clear();

   if ( dbP != NULL )
   {  // Find the record in the database with a matching name
      QStringList qry;
      qry << "get_protocol_desc" << QString::number( invID_passed );
      dbP->query( qry );

      qDebug() << "In US_ProtocolUtil::read_record_auto: DB NOT NULL; query -- " << qry;

      if ( dbP->lastErrno() != US_DB2::OK )
         return idprot;    // Error exit:  unable to read DB record

      while ( dbP->next() )
      {
         QString pname    = dbP->value( 2 ).toString();

         if ( pname == protname )
         {  // Name match:  store information and break out
            QString pdbid    = dbP->value( 0 ).toString();
            QString pguid    = dbP->value( 1 ).toString();
            QDateTime date   = dbP->value( 5 ).toDateTime().toUTC();
            QString pdate    = US_Util::toUTCDatetimeText( date
                                  .toString( Qt::ISODate ), true )
                                  .section( " ", 0, 0 ).simplified();

            // Return entry, xml, and protocol ID
            if ( protentry != NULL )
               *protentry << pname << pdate << pdbid << pguid;
            if ( xml != NULL )
               *xml             = dbP->value( 3 ).toString();

            idprot           = pdbid.toInt();
            break;
         }
      }  // END: db records
   }  // END: from database

   qDebug() << "In US_ProtocolUtil::read_record_auto 2:";
   
   if ( idprot < 0 )
   {  // Find the record in a local file with a matching name

     qDebug() << "In US_ProtocolUtil::read_record_auto 3:";
     
      QString datdir      = US_Settings::dataDir() + "/projects/";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      QStringList rfilt( "R*.xml" );      // "~/ultrascan/data/projects/R*.xml"
      QStringList pfiles  = QDir( datdir ).entryList(
                                              rfilt, QDir::Files, QDir::Name );
      int nfiles          = pfiles.count();

      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each "R000*.xml" file in the directory
         QString pname;
         QString pguid;  
         QString pfname      = pfiles[ ii ];
         QString pfpath      = datdir + pfname;
         QFile pfile( pfpath );
         // Skip if there is a file-open problem
         if ( ! pfile.open( QIODevice::ReadOnly ) )  continue;

         // Capture the XML as a string and start XML reader
         QTextStream tsi( &pfile );
         QString xmlstr      = tsi.readAll();
         pfile.close();
         QXmlStreamReader xmli( xmlstr );

         while( ! xmli.atEnd() )
         {  // Parse XML for description and guid
            xmli.readNext();
            QString ename       = xmli.name().toString();
            if ( xmli.isStartElement()  &&  ename == "protocol" )
            {
               QXmlStreamAttributes attr = xmli.attributes();
               pname               = attr.value( "description" ).toString();
               pguid               = attr.value( "guid" ).toString();
               break;
            }
         }

         if ( pname == protname )
         {  // Name match:  store information and break out
            QString fdate       = US_Util::toUTCDatetimeText(
                                  QFileInfo( pfpath ).lastModified().toUTC()
                                  .toString( Qt::ISODate ), true )
                                  .section( " ", 0, 0 ).simplified();

            // Return entry, xml, and protocol ID (negative of filename numeric)
            if ( protentry != NULL )
               *protentry << pname << fdate << pfname << pguid;
            if ( xml != NULL )
               *xml             = xmlstr;

            idprot           = -( QString( pfname ).section( ".", 0, 0 ).mid( 1 ).toInt() );
            break;
         }
      }  // END: file loop
   }  // END: local disk

   
   qDebug() << "In US_ProtocolUtil::read_record_auto 4: idprot " << idprot ;
   
   return idprot;
}



// Delete a protocol record from the database or local disk.
//
// This function uses a protocol ID (database record ID or "R*.xml"
// numeric part) to delete a protocol record.
//
// protid     Protocol db ID or file GUID
// dbP        Pointer to DB connector or NULL to delete from disk.
// returns    Flag if delete was successful.
bool US_ProtocolUtil::delete_record( const QString protid, US_DB2* dbP )
{
   bool deleted     = false;

   if ( dbP != NULL )
   {  // Delete record from database, using the protocol db id
      QStringList qry( "delete_protocol" );
      qry << QString::number( US_Settings::us_inv_ID() )
          << protid;

      dbP->query( qry );

      if ( dbP->lastErrno() == US_DB2::OK )
         deleted          = true;
   }

   else
   {  // Delete record from local disk, using name of file with matching GUID
      QString datdir      = US_Settings::dataDir() + "/projects/";
      datdir.replace( "\\", "/" );        // Possible Windows fix
      bool newFile        = false;
      QString filename    = US_DataFiles::get_filename( datdir, protid, "R",
                                             "protocol", "guid", newFile );
      if ( ! newFile )
      {  // File exists, so (attempt to) remove it 
         QFile pfile( datdir + filename );

         if ( pfile.exists()  &&  pfile.remove() )  // (should always be true)
            deleted          = true;
      }
   }

   return deleted;
}

