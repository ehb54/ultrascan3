//! \file us_rotor_gui.cpp

#include "us_report.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_db2.h"

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
        << QString::number( editedDataID )
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
                  << newStatus;
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

   this->docs << d;

   int last = docs.size() - 1;

   US_Report::Status status = this->docs[last].saveDB( this->tripleID, dir, db );
   return status;
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

// Function to add a triple record, both in the object and the DB
US_Report::Status US_Report::addTriple(
   QString triple, 
   US_DB2* db )
{
   US_Report::ReportTriple d;

   d.triple = triple;

   triples << d;

   int last = triples.size() - 1;

   return this->triples[last].saveDB( this->ID, db );
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

