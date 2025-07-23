//! \file us_report.h
#ifndef US_REPORT_H
#define US_REPORT_H

#include "us_extern.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

/*! \class US_Report
   This class provides a low-level interface to the report tables, allowing
   the user to read and write them to db.
*/

class US_UTIL_EXTERN US_Report
{

   public:

      //! \brief   Some status codes to keep track of latest conditions
      enum Status
      {
         REPORT_OK,         //!< The last db operation completed successfully
         NOT_FOUND,         //!< The report, reportTriple or reportDocument ID/GUID was not found
         CONNECT_ERROR,     //!< Could not connect to the DB
         DB_ERROR,          //!< A US_DB2 error occurred; check db->lastErrno();
         MISC_ERROR,        //!< An unspecified error occurred
         SAVED_DB           //!< The report has been saved to the DB
      };

      //! \brief Map report types: name-to-label and name-to-mimetype.
      //!
      //! Object that creates and holds mappings of name-to-label for analysis
      //! (application), subAnalysis (report), and reportType (extension);
      //! as well as a mapping of extension names to mime-types.
      class US_UTIL_EXTERN ReportTypes
      {
         public:
            //! \brief US_ReportTypes constructor to create mappings.
            ReportTypes();

            QMap< QString, QString > appLabels;  //!< Application Name-to-Label
            QMap< QString, QString > rptLabels;  //!< Report Name-to-Label
            QMap< QString, QString > extLabels;  //!< Extension Name-to-Label
            QMap< QString, QString > extMTypes;  //!< Exten. Name-to-MimeType

            //! \brief Displays the contents of the class variables in qDebug() statements
            void    show  ( void );
   
      };

      /*! \brief      The ReportDocument class describes an individual report document. This
                      document would be a png or an html snippet that could be included as
                      one of many documents available to be a part of a report
      */
      class US_UTIL_EXTERN ReportDocument
      {
         public:
         int     documentID;    //!< The ID of this report document
         QString documentGUID;  //!< The GUID of this report document
         int     editedDataID;  //!< The edit profile in place when report was generated
         QString label;         //!< An identifying label for the report document profile
         QString filename;      //!< The filename of the disk file that stored this document
         QString analysis;      //!< The type of analysis that this document was derived from
                                //!< (2DSA, GA, dcdt, etc.)
         QString subAnalysis;   //!< Further information that helps to identify what type of content it is
                                //!< (report, residuals, rinoise, etc.)
         QString documentType;  //!< What type of document this is (png, svg, etc.)

         //! \brief Generic constructor for the ReportDocument class.
         ReportDocument();

         /*! \brief    Function to save/update the current report document to the DB

             \param    tripleID The database ID of the report triple this 
                       document is associated with
             \param    dir The directory where the document contents file is located
             \param    db For database access, an open database connection
         */
         Status        saveDB( int, QString, US_DB2* = 0 );

         /*! \brief    Function to read the entire current report document from the DB
         
             \param    dir The directory where the document will be written
             \param    db  For database access, an open database connection
         */
         Status readDB( QString, US_DB2* = 0 );

         //! \brief Resets the class variables to their default vaules
         void    reset ( void );
   
         //! \brief Displays the contents of the class variables in qDebug() statements
         void    show  ( void );
   
         private:
         int     dbg_level;     //!< Debug level value
      };

      /*! \brief      The ReportTriple class describes an individual report. This report
                      would ordinarily be associated with a particular runID and 
                      triple ( c/c/w ) within the experiment, and could contain any number
                      of report documents.
      */
      class US_UTIL_EXTERN ReportTriple
      {
         public:
         int     tripleID;      //!< The ID of this report
         QString tripleGUID;    //!< The GUID of this report
         int     resultID;      //!< The resultID from the HPCAnalysisResult table record 
                                //!< that this triple was derived from
         QString triple;        //!< The cell/channel/wavelength identifying which channel 
                                //!< this triple was derived from (Format: c/c/w)
         QString dataDescription; //!< The data description from the first line of the
                                  //!< original data file
         QVector< ReportDocument > docs; //!< The report documents themselves

         //! \brief Generic constructor for the ReportTriple class.
         ReportTriple();

         /*! \brief    Function to save/update the current triple to the DB

             \param    reportID The database ID of the global report structure this 
                       document is associated with
             \param    db For database access, an open database connection
             \return   One of the US_Report error codes
         */
         Status        saveDB( int, US_DB2* = 0 );

         /*! \brief    Function to read all the documents for the current triple,
                       except for the document content itself
         
             \param    db For database access, an open database connection
         */
         Status        readDocsDB( US_DB2* = 0 );

         /*! \brief    Function to add an empty report document record, both in the object 
                       and the DB

             \param    editedDataID       The edit profile in effect when the document
                                          was created
             \param    label              An identifying label
             \param    dir                The directory where the document can be found
             \param    filename           The filename of the document itself
             \param    analysis           The type of analysis (vHW, dcdt, 2DSA, etc.)
             \param    subAnalysis        The sub-analysis type (residuals, rinoise, sdistrib, etc.)
             \param    documentType       The type of document (png, csv, svg, html, etc.)
             \param    db For database access, an open database connection
             \return   One of the US_Report error codes
         */
         Status        addDocument( int    ,
                                    QString,
                                    QString,
                                    QString,
                                    QString,
                                    QString,
                                    QString,
                                    US_DB2* = 0 );

         /*! \brief    Function to add/replace an entire document record

             \param    d    A US_Report::ReportDocument object
             \param    dir  The directory where the document can be found
             \param    db   For database access, an open database connection
             \return   One of the US_Report error codes
         */
         Status        addDocument( US_Report::ReportDocument , QString , US_DB2* = 0 );

         /*! \brief    Function to delete a report document from the DB

             \param    ndx The index into the docs QVector of the report document to delete
             \param    db  For database access, an open database connection
         */
         Status        removeDocument( int, US_DB2* = 0 );

         /*! \brief    Function to find an existing report document record, based on the
                       analysis, subAnalysis, and documentType fields

             \param    searchAnal         The type of analysis (vHW, dcdt, 2DSA, etc.)
             \param    searchSubanal      The sub-analysis type (residuals, rinoise, sdistrib, etc.)
             \param    searchType         The type of document (png, csv, svg, html, etc.)
             \return   The index into the QVector docs that matches all three analysis fields,
                       or -1 if no match
         */
         int           findDocument( QString, QString, QString );

         //! \brief  Resets the class variables to their default values
         void          reset ( void );
                       
         //! \brief  Displays the contents of the class variables in qDebug()s
         void          show  ( void );
   
         private:
         int     dbg_level;     //!< Debug level value
      };

      int     ID;            //!< The ID of this global report structure
      QString GUID;          //!< The GUID of this global report structure
      int     experimentID;  //!< The experimentID from which triple is derived
      QString runID;         //!< The runID that identifies this experiment
      QString title;         //!< The title of the report
      QString html;          //!< Some introductory html that would appear 
                             //!< at the top of the report
      QVector< ReportTriple > triples; //!< The report triple
      ReportTypes rTypes;    //!< Persistent structure of report type mappings
          
      //!  \brief Generic constructor for the US_Report class.
      US_Report();
          
      //!  A destructor.
      ~US_Report() {};
             
      //! \brief    Function to read an entire report structure from the DB,
      //!           except for the document content itself
      //! \param    new_runID The runID of the associated experiment
      //! \param    db For database access, an open database connection
      //! \param    new_triple The triple string for associated documents
      //! \return   One of the US_Report error codes
      Status        readDB( QString, US_DB2* = 0, QString = "" );
      Status        readDB_auto( int, QString, US_DB2* = 0, QString = "" );
      
      //! \brief    Function to save the global report information to db
      //  \param    db For database access, an open database connection
      //  \returns  One of the US_Report error codes
      Status        saveDB( US_DB2* = 0 );
      Status        saveDB_auto( int, US_DB2* = 0 );
      

      //! \brief    Function to add a new empty triple record to the report
      //! \param    triple          The triple identifying which channel
      //! \param    dataDescription The data description from the original file
      //! \param    db For database access, an open database connection
      //! \returns  One of the US_Report error codes
      Status        addTriple( QString, QString = "", US_DB2* = 0 );

      //! \brief    Function to add or replace an entire triple
      //! \param    t  A US_Report::ReportTriple object
      //! \param    db For database access, an open database connection
      //! \returns  One of the US_Report error codes
      Status        addTriple( US_Report::ReportTriple , US_DB2* = 0 );

      //! \brief    Function to save the entire report structure to db
      //! \param    dir The directory where the document contents file is
      //!               located
      //! \param    db For database access, an open database connection
      //! \returns  One of the US_Report error codes
      Status        saveAllToDB( QString, US_DB2* = 0 );

      //! \brief  Function to delete a report triple from the DB,
      //!         along with all documents
      //! \param  ndx The index into the triples QVector of the report triple
      //!             to delete
      //! \param  db For database access, an open database connection
      //! \returns  One of the US_Report error codes
      Status        removeTriple( int, US_DB2* = 0 );

      //! \brief Function to locate a triple record using the triple string
      //! \param    searchTriple The triple identifying which channel
      //! \returns  The index into the QVector triples that matches the triple
      //!           string, or -1 if no match
      int           findTriple( QString );

      //! \brief Store a single reportDocument record based on filename info.
      //! \param    dir      The directory where the report file is located.
      //!                    The is required to end with the runID, e.g.,
      //!                    dir = "/home/user/ultrascan/reports/demo1_veloc"
      //! \param    filename Base file name of the local report document file.
      //!                    This file should be named as follows:
      //!                    analysis.triple.subAnalysis.docType --- e.g.,
      //!                    2dsa.2A260.tinoise.svg
      //! \param    db       For database access, an open database connection
      //! \param    idEdit   ID of EditedData with which document is associated.
      //! \param    dataDescription The data description from the first line of
      //!                    the original file, in case adding new triple
      //! \returns  One of the US_Report error codes
      Status       saveDocumentFromFile( const QString&, const QString&,
                                         US_DB2*, int = 1, const QString = "" );

      Status       saveDocumentFromFile_auto( int, const QString&, const QString&,
					      US_DB2*, int = 1, const QString = "" );

      //! \brief Store multiple reportDocument records from the same triple.
      //! \param  dir      The directory where the report file is located.
      //!                  The is required to end with the runID, e.g.,
      //!                  dir = "/home/user/ultrascan/reports/demo1_veloc"
      //! \param  filepaths Base file paths of the local report document files.
      //!                   These file should be named as follows:
      //!                   analysis.triple.subAnalysis.docType --- e.g.,
      //!                   2dsa.2A260.tinoise.svg
      //! \param  db        For database access, an open database connection
      //! \param  idEdit    ID of EditedData with which docs are associated.
      //! \param  dataDescription The data description from the first line of
      //!                         the original file, in case adding new triple
      //! \returns  One of the US_Report error codes
      Status       saveFileDocuments   ( const QString&, const QStringList&,
                                         US_DB2*, int = 1, const QString = "" );
      
      Status       saveFileDocuments_auto  ( int, const QString&, const QStringList&,
					     US_DB2*, int = 1, const QString = "" );
      
      //! \brief Function to delete the specified report from the DB
      //! \param    reportID The database reportID of the report to delete
      //! \param    db       For database access, an open database connection
      Status       removeReport( int, US_DB2* = 0 );

      //! \brief Resets the class variables to default values
      void         reset();

      //! \brief Displays the contents of the class variables in qDebug()'s
      void         show ( void );

      private:
      int          dbg_level;     //!< Debug level value
};

#endif
