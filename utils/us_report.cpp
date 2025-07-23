//! \file us_rotor_gui.cpp

#include "us_report.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_db2.h"

// Report types
US_Report::ReportTypes::ReportTypes()
{
   // Open the etc/reports.xml file, then read to create mappings
   QString path = US_Settings::appBaseDir() + "/etc/reports.xml";
   QFile file( path );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            QXmlStreamAttributes atts = xml.attributes();
            QString name  = atts.value( "name"  ).toString();
            QString label = atts.value( "label" ).toString();

            if ( xml.name() == "application" )
            {  // Map application (analysis) name to label
               appLabels[ name ] = label;
            }

            else if ( xml.name() == "report" )
            {  // Map report (subAnalysis) name to label
               rptLabels[ name ] = label;
            }

            else if ( xml.name() == "extension" )
            {  // Map extension (documentType) name to label and to mime-type
               extLabels[ name ] = label;
               extMTypes[ name ] = atts.value( "mimetype" ).toString();
            }
         }
      }
   }
}

// Function to show the current values of the class variables
void US_Report::ReportTypes::show( void )
{
   qDebug() << "Application (analysis) mappings:";
   foreach ( QString label, appLabels.keys() )
      qDebug() << label << ": " << appLabels.value( label );

   qDebug() << "";

   qDebug() << "Report (subAnalysis) mappings:";
   foreach ( QString label, rptLabels.keys() )
      qDebug() << label << ": " << rptLabels.value( label );

   qDebug() << "";

   qDebug() << "Extension (documentType) mappings:";
   foreach ( QString label, extLabels.keys() )
      qDebug() << label << ": " << extLabels.value( label );

   qDebug() << "";

   qDebug() << "Extension (mime-type) mappings:";
   foreach ( QString label, extMTypes.keys() )
      qDebug() << label << ": " << extMTypes.value( label );

   qDebug() << "";
}

// Methods relating to the report document
US_Report::ReportDocument::ReportDocument()
{
  reset();
}

// Saves a report document record to DB
US_Report::Status US_Report::ReportDocument::saveDB( 
           int tripleID, QString dir, US_DB2* db )
{
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   int status;

   // First let's be sure we have a valid GUID
   if ( ! rx.exactMatch( this->documentGUID ) )
      this->documentGUID = US_Util::new_guid();
DbgLv(1) << "Doc::saveDB - GUID" << this->documentGUID;

   // Find out if this document has been saved already or not
   QStringList q( "get_reportDocumentID" );
   q  << this->documentGUID;
   db->query( q );
   
   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
DbgLv(1) << "Doc::saveDB - UPD ID(old)" << this->documentID;
      db->next();
      QString docID    = db->value( 0 ).toString();
      this->documentID = docID.toInt();
DbgLv(1) << "Doc::saveDB - UPD ID" << this->documentID;
      // Update the existing report document record in the DB
      QStringList q( "update_reportDocument" );
      q << docID
        << QString::number( this->editedDataID )
        << this->label
        << this->filename
        << this->analysis
        << this->subAnalysis
        << this->documentType;
      db->query( q );

      int updateStatus = db->lastErrno();

      if ( updateStatus != US_DB2::OK )
      {
         qDebug() << "update_reportDocument error"
                  << updateStatus;
         return DB_ERROR;
      }
   }

   else if ( status == US_DB2::NOROWS )
   {
DbgLv(1) << "Doc::saveDB - NEW ID" << this->documentID << "tripID" << tripleID;
DbgLv(1) << "Doc::saveDB -  NEW editID" << this->editedDataID;
      // Create a new report document record in the DB
      QStringList q( "new_reportDocument" );
      q << QString::number( tripleID )
        << this->documentGUID
        << QString::number( this->editedDataID )
        << this->label
        << this->filename
        << this->analysis
        << this->subAnalysis
        << this->documentType;
      db->query( q );

      int newStatus = db->lastErrno();

      if ( newStatus != US_DB2::OK )
      {
         qDebug() << "new_reportDocument error"
                  << newStatus << db->lastError();
         return DB_ERROR;
      }

      this->documentID = db->lastInsertID(); 
   }

   else // some other database error
   {
      qDebug() << "get_reportDocumentID error"
               << status;
      return DB_ERROR;
   }

   // We can also upload the report contents
   QString fpath   = ( dir.endsWith( "/" ) ? dir : dir + "/" ) + this->filename;
   int writeStatus = db->writeBlobToDB( fpath,
                     QString( "upload_reportContents" ), this->documentID );

   if ( writeStatus != US_DB2::OK )
   {
      qDebug() << "upload_reportContents error"
               << writeStatus << db->lastError();
      return DB_ERROR;
   }

   return REPORT_OK;
}

// Function to read a document from the DB, including document content,
// into the local data structure
US_Report::Status US_Report::ReportDocument::readDB( QString dir, US_DB2* db )
{
   QStringList q( "get_reportDocument_info" );
   q << QString::number( this->documentID );
   db->query( q );

   int status = db->lastErrno();

   if ( status == US_DB2::NOROWS )
   {
      qDebug() << "get_reportDocument_info NOT_FOUND error" << status;
      return NOT_FOUND;
   }

   else if ( status != US_DB2::OK )
   {
      qDebug() << "get_reportDocument_info error" << status;
      return DB_ERROR;
   }

   int docID = this->documentID;
   this->reset();
   db->next();

   this->documentID   = docID;
   this->documentGUID = db->value(0).toString();
   this->editedDataID = db->value(1).toInt();
   this->label        = db->value(2).toString();
   this->filename     = db->value(3).toString();
   this->analysis     = db->value(4).toString();
   this->subAnalysis  = db->value(5).toString();
   this->documentType = db->value(6).toString();

   // We can also download the document contents
   QString fpath  = ( dir.endsWith( "/" ) ? dir : dir + "/" ) + this->filename;
   int readStatus = db->readBlobFromDB( fpath,
                    QString( "download_reportContents" ), this->documentID );

   if ( readStatus != US_DB2::OK )
   {
      qDebug() << "download_reportContents error"
               << readStatus;
      return DB_ERROR;
   }

   return REPORT_OK;
}

// Function to clear out the report document structure
void US_Report::ReportDocument::reset( void )
{
   documentID     = -1;
   documentGUID   = "";
   editedDataID   = 1;
   label          = "";
   filename       = "";
   analysis       = "";
   subAnalysis    = "";
   documentType   = "";
   dbg_level      = US_Settings::us_debug();
}

// Function to show the current values of the class variables
void US_Report::ReportDocument::show( void )
{
   qDebug() << "    documentID =   " <<  documentID     ;
   qDebug() << "    documentGUID = " <<  documentGUID   ;
   qDebug() << "    editedDataID = " <<  editedDataID   ;
   qDebug() << "    label =        " <<  label          ;
   qDebug() << "    filename =     " <<  filename       ;
   qDebug() << "    analysis =     " <<  analysis       ;
   qDebug() << "    subAnalysis =  " <<  subAnalysis    ;
   qDebug() << "    documentType = " <<  documentType   ;
   qDebug() << "";

}

// Methods relating to the report triple structure (each triple)
US_Report::ReportTriple::ReportTriple()
{
   reset();
}

// Saves a report triple record to DB
US_Report::Status US_Report::ReportTriple::saveDB( int reportID, US_DB2* db )
{
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   int status;

   // First let's be sure we have a valid GUID
   if ( ! rx.exactMatch( this->tripleGUID ) )
      this->tripleGUID = US_Util::new_guid();

   // Find out if this triple has been saved already or not
   QStringList q( "get_reportTripleID" );
   q  << this->tripleGUID;
   db->query( q );

   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
      // Update the existing report triple record in the DB
      QStringList q( "update_reportTriple" );
      q << QString::number( this->tripleID )
        << QString::number( -1 )        // resultID unknown in this context
        << this->triple
        << this->dataDescription;
      db->query( q );

      int updateStatus = db->lastErrno();

      if ( updateStatus != US_DB2::OK )
      {
         qDebug() << "update_reportTriple error"
                  << updateStatus;
         return DB_ERROR;
      }
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create a new report triple record in the DB
      QStringList q( "new_reportTriple" );
      q << this->tripleGUID
        << QString::number( reportID )
        << QString::number( -1 )        // resultID unknown in this context
        << this->triple
        << this->dataDescription;
      db->query( q );

      int newStatus = db->lastErrno();

      if ( newStatus != US_DB2::OK )
      {
         qDebug() << "new_reportTriple error"
                  << newStatus;
         return DB_ERROR;
      }

      this->tripleID = db->lastInsertID(); 
   }

   else // some other database error
   {
      qDebug() << "get_reportTripleID error"
               << status;
      return DB_ERROR;
   }

   return REPORT_OK;
}

// Reads all the documents for the current triple, except document content
US_Report::Status US_Report::ReportTriple::readDocsDB( US_DB2* db )
{
   docs.clear();
   QStringList q( "get_reportDocument_desc" );
   q << QString::number( tripleID );
   db->query( q );

   int status = db->lastErrno();
   if ( status == US_DB2::OK )
   {
      while ( db->next() )
      {
         US_Report::ReportDocument d;

         d.documentID   = db->value(0).toInt();
         d.documentGUID = db->value(1).toString();
         d.editedDataID = db->value(2).toInt();
         d.label        = db->value(3).toString();
         d.filename     = db->value(4).toString();
         d.analysis     = db->value(5).toString();
         d.subAnalysis  = db->value(6).toString();
         d.documentType = db->value(7).toString();

         docs << d;
      }
   }

   else if ( status != US_DB2::NOROWS )
     return DB_ERROR;

   return REPORT_OK;
}

// Function to add a report document record, both in the object and the DB
US_Report::Status US_Report::ReportTriple::addDocument(
   int     editedDataID,
   QString label,
   QString dir,
   QString filename,
   QString analysis,
   QString subAnalysis,
   QString documentType,
   US_DB2* db )
{
   US_Report::ReportDocument d;

   d.editedDataID = editedDataID;
   d.label        = label;
   d.filename     = filename;
   d.analysis     = analysis;
   d.subAnalysis  = subAnalysis;
   d.documentType = documentType;

   return this->addDocument( d, dir, db );
}

// Function to add/replace an entire document record 
US_Report::Status US_Report::ReportTriple::addDocument(
   US_Report::ReportDocument d,
   QString dir,
   US_DB2* db )
{
   int ndx = this->findDocument( d.analysis, d.subAnalysis, d.documentType );
DbgLv(1) << "Trip::addDoc - ndx" << ndx << "ana,subA,Type"
 << d.analysis << d.subAnalysis << d.documentType;


   // Easier to delete/add the document if it exists
   if ( ndx != -1 )
   {
DbgLv(1) << "Trip::addDoc - remove Doc";
      this->removeDocument( ndx, db );
   }

   this->docs << d;

   // Refresh ndx
   ndx = this->findDocument( d.analysis, d.subAnalysis, d.documentType );
DbgLv(1) << "Trip::addDoc - ndx aft list add" << ndx;

   return this->docs[ndx].saveDB( this->tripleID, dir, db );
}

US_Report::Status US_Report::ReportTriple::removeDocument(
   int     ndx,
   US_DB2* db )
{
   QStringList q( "delete_reportDocument" );
   q  << QString::number( this->docs[ndx].documentID );
   db->query( q );

   int status = db->lastErrno();

   if ( status != US_DB2::OK )
      return DB_ERROR;

   this->docs.remove( ndx );
   return REPORT_OK;
}

// Function to find an existing report document record, based on the
//   analysis, subAnalysis, and documentType fields
int US_Report::ReportTriple::findDocument( 
    QString searchAnal, QString searchSubanal, QString searchType )
{
   for ( int i = 0; i < this->docs.size(); i++ )
   {
      if ( ( this->docs[i].analysis     == searchAnal    ) &&
           ( this->docs[i].subAnalysis  == searchSubanal ) &&
           ( this->docs[i].documentType == searchType    ) )
         return i;
   }

   // If we're here then the triple string we were searching for
   //  was not found
   return -1;
}

// Function to clear out the entire report triple structure
void US_Report::ReportTriple::reset( void )
{
   tripleID        = -1;
   tripleGUID      = "";
   resultID        = -1;
   triple          = "";
   dataDescription = "";
   dbg_level       = US_Settings::us_debug();

   docs.clear();
}

// Function to show the current values of the class variables
void US_Report::ReportTriple::show( void )
{
   qDebug() << "  tripleID =        " <<  tripleID        ;
   qDebug() << "  tripleGUID =      " <<  tripleGUID      ;
   qDebug() << "  resultID =        " <<  resultID        ;
   qDebug() << "  triple =          " <<  triple          ;
   qDebug() << "  dataDescription = " <<  dataDescription ;
   qDebug() << "";

   if ( docs.size() > 0 )
   {
      qDebug() << "  Documents:";
      foreach ( US_Report::ReportDocument document, docs )
        document.show();
   }
}

// Methods relating to the global report
US_Report::US_Report()
{
   reset();
}

// Reads all the report information from DB, except document content
US_Report::Status US_Report::readDB( QString new_runID, US_DB2* db,
                                     QString new_triple )
{
   QStringList q( "get_report_info_by_runID" );
   q << QString::number( US_Settings::us_inv_ID() )
     << new_runID;
   db->query( q );

   int status = db->lastErrno();

   if ( status == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( status != US_DB2::OK )
      return DB_ERROR;

   this->reset();
   db->next();

   this->ID     = db->value(0).toInt();
   GUID         = db->value(1).toString();
   experimentID = db->value(2).toInt();
   this->runID  = db->value(3).toString();
   title        = db->value(4).toString();
   html         = db->value(5).toString();
   bool edittr  = ! new_triple.isEmpty();

   // Now lets get all the report triple records
   triples.clear();
   q.clear();
   q << "get_reportTriple_desc"
     << QString::number( ID );
   db->query( q );

   status = db->lastErrno();
   if ( status == US_DB2::OK )
   {
      while ( db->next() )
      {
         US_Report::ReportTriple t;

         t.tripleID        = db->value(0).toInt();
         t.tripleGUID      = db->value(1).toString();
         t.resultID        = db->value(2).toInt();
         t.triple          = db->value(3).toString();
         t.dataDescription = db->value(4).toString();

         if ( edittr  &&  t.triple != new_triple )
            continue;

         triples << t;
      }
   }

   else if ( status != US_DB2::NOROWS )
      return DB_ERROR;

   // Now cycle through and load all the document metadata
   for ( int i = 0; i < triples.size(); i++ )
   {
      US_Report::Status writeStatus = triples[i].readDocsDB( db );
      if ( writeStatus != REPORT_OK )
         return writeStatus;
   }

   return REPORT_OK;
}

// COPY for autoflow: Reads all the report information from DB, except document content
US_Report::Status US_Report::readDB_auto( int invID_passed, QString new_runID, US_DB2* db,
                                          QString new_triple )
{
   // Find out if the runID is in the DB already
   QString invID = QString::number( invID_passed );

   QStringList q( "get_report_info_by_runID" );
   q << invID
     << new_runID;
   db->query( q );

   int status = db->lastErrno();

   if ( status == US_DB2::NOROWS )
      return NOT_FOUND;

   else if ( status != US_DB2::OK )
      return DB_ERROR;

   this->reset();
   db->next();

   this->ID     = db->value(0).toInt();
   GUID         = db->value(1).toString();
   experimentID = db->value(2).toInt();
   this->runID  = db->value(3).toString();
   title        = db->value(4).toString();
   html         = db->value(5).toString();
   bool edittr  = ! new_triple.isEmpty();

   // Now lets get all the report triple records
   triples.clear();
   q.clear();
   q << "get_reportTriple_desc"
     << QString::number( ID );
   db->query( q );

   status = db->lastErrno();
   if ( status == US_DB2::OK )
   {
      while ( db->next() )
      {
         US_Report::ReportTriple t;

         t.tripleID        = db->value(0).toInt();
         t.tripleGUID      = db->value(1).toString();
         t.resultID        = db->value(2).toInt();
         t.triple          = db->value(3).toString();
         t.dataDescription = db->value(4).toString();

         if ( edittr  &&  t.triple != new_triple )
            continue;

         triples << t;
      }
   }

   else if ( status != US_DB2::NOROWS )
      return DB_ERROR;

   // Now cycle through and load all the document metadata
   for ( int i = 0; i < triples.size(); i++ )
   {
      US_Report::Status writeStatus = triples[i].readDocsDB( db );
      if ( writeStatus != REPORT_OK )
         return writeStatus;
   }

   return REPORT_OK;
}




// Saves the global report information to DB
US_Report::Status US_Report::saveDB( US_DB2* db )
{
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   int status;
   QString now   = QDateTime::currentDateTime().toString();

   // First let's be sure we have a valid GUID
   if ( ! rx.exactMatch( this->GUID ) )
      this->GUID    = US_Util::new_guid();

   // Find out if the runID is in the DB already
   QString invID = QString::number( US_Settings::us_inv_ID() );
   
   QStringList q( "get_reportID_by_runID" );
   q  << invID
      << this->runID;
   db->query( q );

   this->title   = this->title.isEmpty()
                 ? this->runID + " Report"
                 : this->title;

   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
      // Update existing global report structure in the DB
      db->next();
      this->ID      = db->value( 0 ).toInt();
      this->html    = "<p>Report updated " + now + "</p>";

      q.clear();
      q << "update_report" 
        << QString::number( this->ID )
        << this->title
        << this->html;
      db->query( q );

      int updateStatus = db->lastErrno();

      if ( updateStatus != US_DB2::OK )
      {
         qDebug() << "update_report error"
                  << updateStatus;
         return DB_ERROR;
      }
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create a new global report structure in the DB
      this->html    = "<p>Report created " + now + "</p>";

      q.clear();
      q << "new_report"
        << this->GUID
        << this->runID
        << this->title
        << this->html
        << invID;
      db->query( q );

      int newStatus = db->lastErrno();

      if ( newStatus != US_DB2::OK )
      {
         qDebug() << "new_report error"
                  << newStatus;
         return DB_ERROR;
      }

      this->ID = db->lastInsertID(); 
   }

   else // some other database error
   {
      qDebug() << "get_reportID error"
               << status;
      return DB_ERROR;
   }

   // Ensure that we have the experimentID
   q.clear();
   q  << "get_report_info"
      << QString::number( this->ID );
   db->query( q );

   db->next();
   this->experimentID = db->value(2).toInt();

   return REPORT_OK;
}


// COPY for autoflow - with invID passed 
US_Report::Status US_Report::saveDB_auto( int invID_passed, US_DB2* db )
{
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   int status;
   QString now   = QDateTime::currentDateTime().toString();

   // First let's be sure we have a valid GUID
   if ( ! rx.exactMatch( this->GUID ) )
      this->GUID    = US_Util::new_guid();

   // Find out if the runID is in the DB already
   QString invID = QString::number( invID_passed );
   
   QStringList q( "get_reportID_by_runID" );
   q  << invID
      << this->runID;
   db->query( q );

   this->title   = this->title.isEmpty()
                 ? this->runID + " Report"
                 : this->title;

   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
      // Update existing global report structure in the DB
      db->next();
      this->ID      = db->value( 0 ).toInt();
      this->html    = "<p>Report updated " + now + "</p>";

      q.clear();
      q << "update_report" 
        << QString::number( this->ID )
        << this->title
        << this->html;
      db->query( q );

      int updateStatus = db->lastErrno();

      if ( updateStatus != US_DB2::OK )
      {
         qDebug() << "update_report error"
                  << updateStatus;
         return DB_ERROR;
      }
   }

   else if ( status == US_DB2::NOROWS )
   {
      // Create a new global report structure in the DB
      this->html    = "<p>Report created " + now + "</p>";

      q.clear();
      q << "new_report"
        << this->GUID
        << this->runID
        << this->title
        << this->html
        << invID;
      db->query( q );

      int newStatus = db->lastErrno();

      if ( newStatus != US_DB2::OK )
      {
         qDebug() << "new_report error"
                  << newStatus;
         return DB_ERROR;
      }

      this->ID = db->lastInsertID(); 
   }

   else // some other database error
   {
      qDebug() << "get_reportID error"
               << status;
      return DB_ERROR;
   }

   // Ensure that we have the experimentID
   q.clear();
   q  << "get_report_info"
      << QString::number( this->ID );
   db->query( q );

   db->next();
   this->experimentID = db->value(2).toInt();

   return REPORT_OK;
}


// Function to add a new, empty triple record, both in the object and the DB
US_Report::Status US_Report::addTriple(
   QString triple, 
   QString dataDescription,
   US_DB2* db )
{
   US_Report::ReportTriple d;

   d.triple          = triple;
   d.dataDescription = dataDescription;

   return this->addTriple( d, db );
}

// Function to add or replace an entire triple
US_Report::Status US_Report::addTriple(
   US_Report::ReportTriple t,
   US_DB2* db )
{
   int ndx = this->findTriple( t.triple );

   // Easier to delete/add the triple if it exists
   if ( ndx != -1 )
      this->removeTriple( ndx, db );

   this->triples << t;

   // Refresh ndx
   ndx = this->findTriple( t.triple );

   return this->triples[ndx].saveDB( this->ID, db );
}

// Function to remove a triple record, and all the documents
US_Report::Status US_Report::removeTriple(
   int     ndx,
   US_DB2* db )
{
   QStringList q( "delete_reportTriple" );
   q  << QString::number( this->triples[ ndx ].tripleID );
   db->query( q );

   int status = db->lastErrno();

   if ( status != US_DB2::OK )
      return DB_ERROR;

   this->triples.remove( ndx );
   return REPORT_OK;
}

// Function to find the index of the triple that matches the
//  supplied triple string
int US_Report::findTriple( QString searchTriple )
{
   for ( int i = 0; i < this->triples.size(); i++ )
   {
      if ( this->triples[i].triple == searchTriple )
         return i;
   }

   // If we're here then the triple string we were searching for
   //  was not found
   return -1;
}

// Store a single reportDocument record.
// For example, dir = /home/user/ultrascan/reports/demo1_veloc
//     and filename = 2dsa.2A260.tinoise.svg
US_Report::Status US_Report::saveDocumentFromFile( const QString& dir,
      const QString& filename, US_DB2* db, int idEdit,
      const QString dataDescription )
{
   // Parse the directory for the runID
   QStringList parts  = dir.split( "/" );
   if ( parts.size() < 2 )
      return US_Report::MISC_ERROR;

   QString new_runID  = parts.last();
   if ( new_runID.isEmpty() )
      new_runID = parts[ parts.size() - 2 ];

   // Now parse the filename for the other information
   parts.clear();
   parts              = filename.split( '.' );
   if ( parts.size() != 4 )
      return US_Report::MISC_ERROR;

   QString newAnal    = parts[0];
   QString newTriple  = US_Util::expanded_triple( parts[1], false );
   QString newSubanal = parts[2];
   QString newDoctype = parts[3];
   
   // Create a label
   QString newLabel = this->rTypes.appLabels[newAnal]    + ":" +
                      this->rTypes.rptLabels[newSubanal] + ":" +
                      this->rTypes.extLabels[newDoctype] ;

   // Start by reading any DB info we have, or create new report
   QString now = QDateTime::currentDateTime().toString();
   US_Report::Status status = this->readDB( new_runID, db );

   if ( status == US_Report::NOT_FOUND )
   {
      US_Report::Status saveStatus = this->saveDB( db );              
      if ( saveStatus != US_Report::REPORT_OK )
      {
         qDebug() << "report.saveDB error"
                  << saveStatus;
         qDebug() << db->lastError() << db->lastErrno();
         this->show();
      }
   }

   // Read an existing triple, or create a new one
   int tripNdx = this->findTriple( newTriple );
   if ( tripNdx < 0 )
   {
      // Not found
      status = this->addTriple( newTriple, dataDescription, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.addTriple error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }

   }
   
   else if ( this->triples[tripNdx].dataDescription != dataDescription )
   {
      // Then the data description field has changed and needs to be updated
      this->triples[tripNdx].dataDescription = dataDescription;
      this->triples[tripNdx].saveDB( this->ID, db );
   }

   // Refresh tripNdx
   tripNdx = this->findTriple( newTriple );
   US_Report::ReportTriple t = this->triples[tripNdx];

   // Now find this document if it already exists
   int docNdx = t.findDocument( newAnal, newSubanal, newDoctype );

if(docNdx<0)
DbgLv(1) << "Doc::saveDB: NOT FOUND newDoctype" << newDoctype;
   if ( docNdx < 0  &&  newDoctype.endsWith( "svgz" ) )
   {  // If SVGZ not found, test for SVG
      docNdx     = t.findDocument( newAnal, newSubanal, QString( "svg" ) );
DbgLv(1) << "Doc::saveDB:  NOT FOUND svg docNdx" << docNdx;
   }

   if ( docNdx < 0 )
   {
      // Not found
      status = t.addDocument( idEdit,
                              newLabel,
                              dir,
                              filename,
                              newAnal,
                              newSubanal,
                              newDoctype,
                              db );
   }

   else
   {
DbgLv(1) << "Doc::saveDB: Replace ndx label" << docNdx << newLabel;
      t.docs[ docNdx ].editedDataID = idEdit;
      t.docs[ docNdx ].label        = newLabel;
      t.docs[ docNdx ].filename     = filename;
      t.docs[ docNdx ].documentType = newDoctype;

      status = t.docs[ docNdx ].saveDB( t.tripleID, dir, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.docs.saveDB error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }

   }

   // Refresh docNdx
   docNdx = t.findDocument( newAnal, newSubanal, newDoctype );
   
   // Finally, update the triple
   status = t.saveDB( this->ID, db );
   this->triples[tripNdx] = t;

   //status = this->saveAllToDB( dir, db );
   if ( status != US_Report::REPORT_OK )
      return status;

   return US_Report::REPORT_OK;
}


// COPY for autoflow - with invID passed  
US_Report::Status US_Report::saveDocumentFromFile_auto( int invID, const QString& dir,
							const QString& filename, US_DB2* db, int idEdit,
							const QString dataDescription  )
{
   // Parse the directory for the runID
   QStringList parts  = dir.split( "/" );
   if ( parts.size() < 2 )
      return US_Report::MISC_ERROR;

   QString new_runID  = parts.last();
   if ( new_runID.isEmpty() )
      new_runID = parts[ parts.size() - 2 ];

   // Now parse the filename for the other information
   parts.clear();
   parts              = filename.split( '.' );
   if ( parts.size() != 4 )
      return US_Report::MISC_ERROR;

   QString newAnal    = parts[0];
   QString newTriple  = US_Util::expanded_triple( parts[1], false );
   QString newSubanal = parts[2];
   QString newDoctype = parts[3];
   
   // Create a label
   QString newLabel = this->rTypes.appLabels[newAnal]    + ":" +
                      this->rTypes.rptLabels[newSubanal] + ":" +
                      this->rTypes.extLabels[newDoctype] ;

   // Start by reading any DB info we have, or create new report
   QString now = QDateTime::currentDateTime().toString();
   US_Report::Status status = this->readDB_auto( invID, new_runID, db );

   if ( status == US_Report::NOT_FOUND )
   {
      US_Report::Status saveStatus = this->saveDB_auto( invID, db );               //ALEXEY <-- for autoflow, pass invID to saveDB( db, invID ) 
      if ( saveStatus != US_Report::REPORT_OK )
      {
         qDebug() << "report.saveDB error"
                  << saveStatus;
         qDebug() << db->lastError() << db->lastErrno();
         this->show();
      }
   }

   // Read an existing triple, or create a new one
   int tripNdx = this->findTriple( newTriple );
   if ( tripNdx < 0 )
   {
      // Not found
      status = this->addTriple( newTriple, dataDescription, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.addTriple error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }

   }
   
   else if ( this->triples[tripNdx].dataDescription != dataDescription )
   {
      // Then the data description field has changed and needs to be updated
      this->triples[tripNdx].dataDescription = dataDescription;
      this->triples[tripNdx].saveDB( this->ID, db );
   }

   // Refresh tripNdx
   tripNdx = this->findTriple( newTriple );
   US_Report::ReportTriple t = this->triples[tripNdx];

   // Now find this document if it already exists
   int docNdx = t.findDocument( newAnal, newSubanal, newDoctype );

if(docNdx<0)
DbgLv(1) << "Doc::saveDB: NOT FOUND newDoctype" << newDoctype;
   if ( docNdx < 0  &&  newDoctype.endsWith( "svgz" ) )
   {  // If SVGZ not found, test for SVG
      docNdx     = t.findDocument( newAnal, newSubanal, QString( "svg" ) );
DbgLv(1) << "Doc::saveDB:  NOT FOUND svg docNdx" << docNdx;
   }

   if ( docNdx < 0 )
   {
      // Not found
      status = t.addDocument( idEdit,
                              newLabel,
                              dir,
                              filename,
                              newAnal,
                              newSubanal,
                              newDoctype,
                              db );
   }

   else
   {
DbgLv(1) << "Doc::saveDB: Replace ndx label" << docNdx << newLabel;
      t.docs[ docNdx ].editedDataID = idEdit;
      t.docs[ docNdx ].label        = newLabel;
      t.docs[ docNdx ].filename     = filename;
      t.docs[ docNdx ].documentType = newDoctype;

      status = t.docs[ docNdx ].saveDB( t.tripleID, dir, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.docs.saveDB error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }

   }

   // Refresh docNdx
   docNdx = t.findDocument( newAnal, newSubanal, newDoctype );
   
   // Finally, update the triple
   status = t.saveDB( this->ID, db );
   this->triples[tripNdx] = t;

   //status = this->saveAllToDB( dir, db );
   if ( status != US_Report::REPORT_OK )
      return status;

   return US_Report::REPORT_OK;
}


// Saves a list of report document records to DB
US_Report::Status US_Report::saveFileDocuments( const QString& dir,
      const QStringList& filepaths, US_DB2* db, int idEdit,
      const QString dataDescription )
{
   // Get the runID by parsing the directory
   QString new_runID  = QString( dir ).section( "/", -1, -1 );
qDebug() << "rpt:svFD: dir" << dir << "idEdit" << idEdit << "datDsc" << dataDescription
 << "new_runID" << new_runID;
   if ( new_runID.isEmpty() )
      return US_Report::MISC_ERROR;

   // Parse the triple string from the first filename
   int     nfiles     = filepaths.count();
qDebug() << "rpt:svFD:  nfiles" << nfiles << "fpath0" << filepaths[0];

   if ( nfiles < 1 )
      return US_Report::MISC_ERROR;

   QStringList filenames;
   for ( int jj = 0; jj < nfiles; jj++ )
      filenames << QString( filepaths[ jj ] ).section( "/", -1, -1 );

   QString filename   = filenames[ 0 ];
   QString newTriple  = filename.section( '.', -3, -3 );
   newTriple          = US_Util::expanded_triple( newTriple, false );
qDebug() << "rpt:svFD:  filename" << filename << "newTriple" << newTriple;

   // Get any existing report for this run
   // Start by reading any DB info we have, or create new report
   QString now        = QDateTime::currentDateTime().toString();
   US_Report::Status status = this->readDB( new_runID, db, newTriple );    
qDebug() << "rpt:svFD:  rdDB stat" << status;

   if ( status == US_Report::NOT_FOUND )
   {  // For a new report, save what we have
      US_Report::Status saveStatus = this->saveDB( db );                   
      if ( saveStatus != US_Report::REPORT_OK )
      {
         qDebug() << "report.saveDB error"
                  << saveStatus;
         qDebug() << db->lastError() << db->lastErrno();
         this->show();
      }
   }

   // Read an existing triple, or create a new one
   int     tripNdx    = this->findTriple( newTriple );
qDebug() << "rpt:svFD:  tripNdx" << tripNdx;
   if ( tripNdx < 0 )
   {
      // Not found, so create one
      status             = this->addTriple( newTriple, dataDescription, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.addTriple error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }
      // Refresh tripNdx
      tripNdx            = this->findTriple( newTriple );
qDebug() << "rpt:svFD:  2)tripNdx" << tripNdx;
   }
   
   else if ( this->triples[ tripNdx ].dataDescription != dataDescription )
   {  // The data description field has changed and needs to be updated
      this->triples[ tripNdx ].dataDescription = dataDescription;
      this->triples[ tripNdx ].saveDB( this->ID, db );
qDebug() << "rpt:svFD:  *triples UPDATE";
   }

   US_Report::ReportTriple* trip = &this->triples[ tripNdx ];

   // Build a list of documents already assigned to the triple
   QStringList  tdnames;
   QList< int > tdNdxs;
   QString filepath;
   QString dirfile    = dir.endsWith( "/" ) ? dir : ( dir + "/" );
   int     ntdocs     = trip->docs.count();
   int     idTrip     = trip->tripleID;
qDebug() << "rpt:svFD:  dirfile" << dirfile << "ntdocs" << ntdocs << "idTrip" << idTrip;

   for ( int ii = 0; ii < ntdocs; ii++ )
   {
      filename           = trip->docs[ ii ].filename;
      int tidEdit        = trip->docs[ ii ].editedDataID;

      if ( tidEdit == idEdit  &&  filenames.contains( filename ) )
      {
         tdnames << filename;
         tdNdxs  << ii;
qDebug() << "rpt:svFD: trip docs filename" << filename << "ii" << ii;
      }
else
qDebug() << "rpt:svFD:  MISS: trip docs filename" << filename << "ii" << ii
 << "tidEdit" << tidEdit << "idEdit" << idEdit;
   }
qDebug() << "rpt:svFD: tdNdxs.count()" << tdNdxs.count() << "nfiles" << nfiles;

   // Add any new documents to the triple's list
   //if ( tdNdxs.count() < nfiles )
   {
      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each specified file name
         filename           = filenames[ ii ];
qDebug() << "rpt:svFD:  fnames filename" << filename << "ii" << ii;

         if ( !tdnames.contains( filename ) )
         {  // This document is new and needs to be added to the triple's list
qDebug() << "rpt:svFD:    NEW to triple doc";
            US_Report::ReportDocument rdoc;
            QString newAnal    = filename.section( ".", -4, -4 );
            QString newSubanal = filename.section( ".", -2, -2 );
            QString newDoctype = filename.section( ".", -1, -1 );
            QString newLabel   = this->rTypes.appLabels[ newAnal    ] + ":" +
                                 this->rTypes.rptLabels[ newSubanal ] + ":" +
                                 this->rTypes.extLabels[ newDoctype ];
            QString docGUID    = US_Util::new_guid();

            rdoc.documentGUID  = docGUID;             // Build report doc
            rdoc.label         = newLabel;
            rdoc.filename      = filename;
            rdoc.analysis      = newAnal;
            rdoc.subAnalysis   = newSubanal;
            rdoc.documentType  = newDoctype;
            rdoc.editedDataID  = idEdit;

            QStringList qry( "new_reportDocument" );  // Create new doc record
            qry << QString::number( idTrip )
                << docGUID
                << QString::number( idEdit )
                << newLabel
                << filename
                << newAnal
                << newSubanal
                << newDoctype;
            db->query( qry );

            int ndstat = db->lastErrno();
            if ( ndstat != US_DB2::OK )
            {
               qDebug() << "new_reportDocument error"
                        << ndstat << db->lastError();
               return US_Report::DB_ERROR;
            }

            rdoc.documentID    = db->lastInsertID();  // Save doc DB Id

            trip->docs << rdoc;                       // Add to triple's docs
qDebug() << "rpt:svFD:    new_reportDoc (no error) docID" << rdoc.documentID;

            tdnames << filename;                      // Save doc file name
            tdNdxs  << ntdocs;                        // Save index in doc list
            ntdocs++;                                 // Bump triple's doc count
         }
      }
   }

   // Update the contents of each of the specified report documents
   for ( int ii = 0; ii < nfiles; ii++ )
   {
      filepath           = filepaths[ ii ];
      filename           = filenames[ ii ];
      int tdnamx         = tdnames.indexOf( filename );

      if ( tdnamx < 0 )
      {
qDebug() << "rpt:svFD:    *TDNAMX err* filename" << filename;
         qDebug() << "upload_reportContents TDNAMX error"
                  << filename;
         return US_Report::MISC_ERROR;
      }

      int tdndx          = tdNdxs[ tdnamx ];
      int idDoc          = trip->docs[ tdndx ].documentID;

      int wrstat         = db->writeBlobToDB( filepath,
                              QString( "upload_reportContents" ), idDoc );
qDebug() << "rpt:svFD:    upld contents size" << QFileInfo(filepath).size()
 << "filepath" << filepath << "idDoc" << idDoc;

      if ( wrstat != US_DB2::OK )
      {
qDebug() << "rpt:svFD:    *wrBlob err* wrstat" << wrstat << db->lastError();
         qDebug() << "upload_reportContents error"
                  << wrstat << db->lastError();
         return US_Report::DB_ERROR;
      }
   }

   return US_Report::REPORT_OK;
}

// Saves a list of report document records to DB
US_Report::Status US_Report::saveFileDocuments_auto( int invID_passed, const QString& dir,
						     const QStringList& filepaths, US_DB2* db, int idEdit,
						     const QString dataDescription )
{
   // Get the runID by parsing the directory
   QString new_runID  = QString( dir ).section( "/", -1, -1 );
qDebug() << "rpt:svFD: dir" << dir << "idEdit" << idEdit << "datDsc" << dataDescription
 << "new_runID" << new_runID;
   if ( new_runID.isEmpty() )
      return US_Report::MISC_ERROR;

   // Parse the triple string from the first filename
   int     nfiles     = filepaths.count();
qDebug() << "rpt:svFD:  nfiles" << nfiles << "fpath0" << filepaths[0];

   if ( nfiles < 1 )
      return US_Report::MISC_ERROR;

   QStringList filenames;
   for ( int jj = 0; jj < nfiles; jj++ )
      filenames << QString( filepaths[ jj ] ).section( "/", -1, -1 );

   QString filename   = filenames[ 0 ];
   QString newTriple  = filename.section( '.', -3, -3 );
   newTriple          = US_Util::expanded_triple( newTriple, false );
qDebug() << "rpt:svFD:  filename" << filename << "newTriple" << newTriple;

   // Get any existing report for this run
   // Start by reading any DB info we have, or create new report
   QString now        = QDateTime::currentDateTime().toString();
   US_Report::Status status = this->readDB_auto( invID_passed, new_runID, db, newTriple );     //ALEXEY <-- pass invID if autoflow;  needed for us_edit
qDebug() << "rpt:svFD:  rdDB stat" << status;

   if ( status == US_Report::NOT_FOUND )
   {  // For a new report, save what we have
      US_Report::Status saveStatus = this->saveDB_auto( invID_passed, db );                    //ALEXEY <-- pass invID if autoflow;  needed for us_edit
      if ( saveStatus != US_Report::REPORT_OK )
      {
         qDebug() << "report.saveDB error"
                  << saveStatus;
         qDebug() << db->lastError() << db->lastErrno();
         this->show();
      }
   }

   // Read an existing triple, or create a new one
   int     tripNdx    = this->findTriple( newTriple );
qDebug() << "rpt:svFD:  tripNdx" << tripNdx;
   if ( tripNdx < 0 )
   {
      // Not found, so create one
      status             = this->addTriple( newTriple, dataDescription, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.addTriple error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }
      // Refresh tripNdx
      tripNdx            = this->findTriple( newTriple );
qDebug() << "rpt:svFD:  2)tripNdx" << tripNdx;
   }
   
   else if ( this->triples[ tripNdx ].dataDescription != dataDescription )
   {  // The data description field has changed and needs to be updated
      this->triples[ tripNdx ].dataDescription = dataDescription;
      this->triples[ tripNdx ].saveDB( this->ID, db );
qDebug() << "rpt:svFD:  *triples UPDATE";
   }

   US_Report::ReportTriple* trip = &this->triples[ tripNdx ];

   // Build a list of documents already assigned to the triple
   QStringList  tdnames;
   QList< int > tdNdxs;
   QString filepath;
   QString dirfile    = dir.endsWith( "/" ) ? dir : ( dir + "/" );
   int     ntdocs     = trip->docs.count();
   int     idTrip     = trip->tripleID;
qDebug() << "rpt:svFD:  dirfile" << dirfile << "ntdocs" << ntdocs << "idTrip" << idTrip;

   for ( int ii = 0; ii < ntdocs; ii++ )
   {
      filename           = trip->docs[ ii ].filename;
      int tidEdit        = trip->docs[ ii ].editedDataID;

      if ( tidEdit == idEdit  &&  filenames.contains( filename ) )
      {
         tdnames << filename;
         tdNdxs  << ii;
qDebug() << "rpt:svFD: trip docs filename" << filename << "ii" << ii;
      }
else
qDebug() << "rpt:svFD:  MISS: trip docs filename" << filename << "ii" << ii
 << "tidEdit" << tidEdit << "idEdit" << idEdit;
   }
qDebug() << "rpt:svFD: tdNdxs.count()" << tdNdxs.count() << "nfiles" << nfiles;

   // Add any new documents to the triple's list
   //if ( tdNdxs.count() < nfiles )
   {
      for ( int ii = 0; ii < nfiles; ii++ )
      {  // Examine each specified file name
         filename           = filenames[ ii ];
qDebug() << "rpt:svFD:  fnames filename" << filename << "ii" << ii;

         if ( !tdnames.contains( filename ) )
         {  // This document is new and needs to be added to the triple's list
qDebug() << "rpt:svFD:    NEW to triple doc";
            US_Report::ReportDocument rdoc;
            QString newAnal    = filename.section( ".", -4, -4 );
            QString newSubanal = filename.section( ".", -2, -2 );
            QString newDoctype = filename.section( ".", -1, -1 );
            QString newLabel   = this->rTypes.appLabels[ newAnal    ] + ":" +
                                 this->rTypes.rptLabels[ newSubanal ] + ":" +
                                 this->rTypes.extLabels[ newDoctype ];
            QString docGUID    = US_Util::new_guid();

            rdoc.documentGUID  = docGUID;             // Build report doc
            rdoc.label         = newLabel;
            rdoc.filename      = filename;
            rdoc.analysis      = newAnal;
            rdoc.subAnalysis   = newSubanal;
            rdoc.documentType  = newDoctype;
            rdoc.editedDataID  = idEdit;

            QStringList qry( "new_reportDocument" );  // Create new doc record
            qry << QString::number( idTrip )
                << docGUID
                << QString::number( idEdit )
                << newLabel
                << filename
                << newAnal
                << newSubanal
                << newDoctype;
            db->query( qry );

            int ndstat = db->lastErrno();
            if ( ndstat != US_DB2::OK )
            {
               qDebug() << "new_reportDocument error"
                        << ndstat << db->lastError();
               return US_Report::DB_ERROR;
            }

            rdoc.documentID    = db->lastInsertID();  // Save doc DB Id

            trip->docs << rdoc;                       // Add to triple's docs
qDebug() << "rpt:svFD:    new_reportDoc (no error) docID" << rdoc.documentID;

            tdnames << filename;                      // Save doc file name
            tdNdxs  << ntdocs;                        // Save index in doc list
            ntdocs++;                                 // Bump triple's doc count
         }
      }
   }

   // Update the contents of each of the specified report documents
   for ( int ii = 0; ii < nfiles; ii++ )
   {
      filepath           = filepaths[ ii ];
      filename           = filenames[ ii ];
      int tdnamx         = tdnames.indexOf( filename );

      if ( tdnamx < 0 )
      {
qDebug() << "rpt:svFD:    *TDNAMX err* filename" << filename;
         qDebug() << "upload_reportContents TDNAMX error"
                  << filename;
         return US_Report::MISC_ERROR;
      }

      int tdndx          = tdNdxs[ tdnamx ];
      int idDoc          = trip->docs[ tdndx ].documentID;

      int wrstat         = db->writeBlobToDB( filepath,
                              QString( "upload_reportContents" ), idDoc );
qDebug() << "rpt:svFD:    upld contents size" << QFileInfo(filepath).size()
 << "filepath" << filepath << "idDoc" << idDoc;

      if ( wrstat != US_DB2::OK )
      {
qDebug() << "rpt:svFD:    *wrBlob err* wrstat" << wrstat << db->lastError();
         qDebug() << "upload_reportContents error"
                  << wrstat << db->lastError();
         return US_Report::DB_ERROR;
      }
   }

   return US_Report::REPORT_OK;
}

// Function to remove an entire report structure, all the triples, and all the documents
US_Report::Status US_Report::removeReport(
   int     reportID,
   US_DB2* db )
{
   QStringList q( "delete_report" );
   q  << QString::number( reportID );
   db->query( q );

   int status = db->lastErrno();

   if ( status != US_DB2::OK )
      return DB_ERROR;

   this->reset();
   return REPORT_OK;
}

// Saves an entire report structure to DB
US_Report::Status US_Report::saveAllToDB( QString dir, US_DB2* db )
{
  US_Report::Status status = this->saveDB( db );                   // ALEXEY <- pass invID when autoflow; this func. not used anywhere...
   if ( status != US_Report::REPORT_OK ) 
   {
      qDebug() << "report.saveDB error"
               << status;
   }

   foreach( US_Report::ReportTriple triple, this->triples )
   {
      status = triple.saveDB( this->ID, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "triple.saveDB error"
                  << status;
      }

      foreach ( US_Report::ReportDocument doc, triple.docs )
      {
         status = doc.saveDB( triple.tripleID, dir, db );
         if ( status != US_Report::REPORT_OK )
         {
            qDebug() << "document.saveDB error"
                     << status;
         }
      }
   }

   return REPORT_OK;
}

// Function to clear out the entire report structure
void US_Report::reset( void )
{
   ID           = -1;
   GUID         = "";
   experimentID = -1;
   runID        = "";
   title        = "";
   html         = "";
   dbg_level    = US_Settings::us_debug();

   triples.clear();
}

// Function to show the current values of the class variables
void US_Report::show( void )
{
   qDebug() << "ID =           " <<  ID           ;
   qDebug() << "GUID =         " <<  GUID         ;
   qDebug() << "experimentID = " <<  experimentID ;
   qDebug() << "runID =        " <<  runID        ;
   qDebug() << "title =        " <<  title        ;
   qDebug() << "html =         " <<  html         ;
   qDebug() << "";

   if ( triples.size() > 0 )
   {
      qDebug() << "Triples:";
      foreach ( US_Report::ReportTriple triple, triples )
        triple.show();
   }
}

