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

   // Find out if this document has been saved already or not
   QStringList q( "get_reportDocumentID" );
   q  << this->documentGUID;
   db->query( q );
   
   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
      // Update the existing report document record in the DB
      QStringList q( "update_reportDocument" );
      q << QString::number( this->documentID )
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
   int writeStatus = db->writeBlobToDB( dir + this->filename,
                     QString( "upload_reportContents" ), this->documentID );

   if ( writeStatus != US_DB2::OK )
      return DB_ERROR;

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
      return NOT_FOUND;

   else if ( status != US_DB2::OK )
      return DB_ERROR;

   this->reset();
   db->next();

   this->documentGUID = db->value(0).toString();
   this->editedDataID = db->value(1).toInt();
   this->label        = db->value(2).toString();
   this->filename     = db->value(3).toString();
   this->analysis     = db->value(4).toString();
   this->subAnalysis  = db->value(5).toString();
   this->documentType = db->value(6).toString();

   // We can also download the document contents
   int readStatus = db->readBlobFromDB( dir + this->filename,
                    QString( "download_reportContents" ), this->documentID );

   if ( readStatus != US_DB2::OK )
     return DB_ERROR;

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
        << this->triple;
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
        << this->triple;
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

   // Easier to delete/add the document if it exists
   if ( ndx != -1 )
      this->removeDocument( ndx, db );

   this->docs << d;

   // Refresh ndx
   ndx = this->findDocument( d.analysis, d.subAnalysis, d.documentType );

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
   tripleID     = -1;
   tripleGUID   = "";
   resultID     = -1;
   triple       = "";

   docs.clear();
}

// Function to show the current values of the class variables
void US_Report::ReportTriple::show( void )
{
   qDebug() << "  tripleID =     " <<  tripleID     ;
   qDebug() << "  tripleGUID =   " <<  tripleGUID   ;
   qDebug() << "  resultID =     " <<  resultID     ;
   qDebug() << "  triple =       " <<  triple       ;
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
US_Report::Status US_Report::readDB( QString new_runID, US_DB2* db )
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

   this->ID    = db->value(0).toInt();
   GUID        = db->value(1).toString();
   experimentID = db->value(2).toInt();
   this->runID = db->value(3).toString();
   title       = db->value(4).toString();
   html        = db->value(5).toString();

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

         t.tripleID     = db->value(0).toInt();
         t.tripleGUID   = db->value(1).toString();
         t.resultID     = db->value(2).toInt();
         t.triple       = db->value(3).toString();

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

   // First let's be sure we have a valid GUID
   if ( ! rx.exactMatch( this->GUID ) )
      this->GUID = US_Util::new_guid();

   // Find out if the runID is in the DB already
   QString invID = QString::number( US_Settings::us_inv_ID() );
   QStringList q( "get_reportID_by_runID" );
   q  << invID
      << this->runID;
   db->query( q );

   status = db->lastErrno();

   if ( status == US_DB2::OK )
   {
      // Update existing global report structure in the DB
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
   US_DB2* db )
{
   US_Report::ReportTriple d;

   d.triple = triple;

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
      const QString& filename, US_DB2* db, int idEdit )
{
   // Parse the directory for the runID
   QStringList parts  = dir.split( "/" );
   if ( parts.size() < 2 )
      return US_Report::MISC_ERROR;

   QString new_runID  = parts.last();

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
   if ( tripNdx == -1 )
   {
      // Not found
      status = this->addTriple( newTriple, db );
      if ( status != US_Report::REPORT_OK )
      {
         qDebug() << "saveDocumentFromFile.addTriple error"
                  << status;
         qDebug() << db->lastError() << db->lastErrno();
         return US_Report::DB_ERROR;
      }

   }
   
   // Refresh tripNdx
   tripNdx = this->findTriple( newTriple );
   US_Report::ReportTriple t = this->triples[tripNdx];

   // Now find this document if it already exists
   int docNdx = t.findDocument( newAnal, newSubanal, newDoctype );

   if ( docNdx == -1 )
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
      t.docs[ docNdx ].label        = newLabel;
      t.docs[ docNdx ].editedDataID = idEdit;
      t.docs[ docNdx ].saveDB( t.tripleID, dir, db );
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
   US_Report::Status status = this->saveDB( db );
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

