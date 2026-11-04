//! \file us_convertio.h
#ifndef US_CONVERTIO_H
#define US_CONVERTIO_H

#include <QtCore>

// The interface only: no signature here names the concrete connection type,
// so callers can pass a test double in place of a live connection.
#include "ius_db2.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "us_extern.h"

struct cellInfo
{
  QString     cellName;
  QString     channelName;
  int         centerpieceID;
};

/*! \class US_ConvertIO
           This class provides the ability to save converted US3
           data to the disk and the database.
           All methods are static.
*/
class US_UTIL_EXTERN US_ConvertIO
{
   public:
      /*! \brief One run as it exists on disk, loaded but not interpreted.

          The value owns loaded domain data and nothing else: no widgets, no
          database connection, no edit files, no load policy, no settings, no
          temporary-directory lifecycle.
      */
      struct US_UTIL_EXTERN DiskRun
      {
            QString                       directory;  //!< With a trailing separator
            QString                       runID;      //!< From the directory basename
            QString                       runType;    //!< "RA", "RI", "IP", ...
            QVector<US_DataIO::RawData>   rawData;    //!< One per AUC, filename order
            QList<US_Convert::TripleInfo> triples;    //!< Parallel to rawData
            QVector<SP_SPEEDPROFILE>      speedSteps; //!< As serialized; may be empty
            US_Experiment                 experiment;
      };

      /*! \brief Read one run directory into a DiskRun.

          A thin coordinator over the existing readers.  It reports structural
          problems and does not reject them: it returns the most specific
          status it can, fills `error` with the detail, and populates `run` as
          far as the read got.  Whether a non-OK status ends the load is the
          caller's decision; US_ConvertGui, for example, loads most of them
          and shows the detail.

          It does not split archives, require edits, connect to a database,
          start a transaction, write records, stage files, change settings,
          present dialogs, or reinterpret scientific data.

          Speed steps are returned as the experiment XML gives them.  They are
          never computed from the scans and never compared against them; see
          US_Experiment::readFromDisk()'s diagnostic overload.

          \param directory One run directory, with or without a trailing
                           separator.  Its basename is the run ID.
          \param run       Filled as far as the read reached
          \param error     Set to specific context when the status is not OK

          \returns A US_Convert::ioError value.  US_Convert::BAD_RUNID means
                   the directory name cannot be a run ID and nothing was
                   read.  US_Convert::INVALID_RUN means the filenames carry
                   mixed run IDs or run types.
      */
      static int                   readDiskRun( const QString& directory, DiskRun& run, QString& error );

      //! \brief readDiskRun() error formats, shared with the tests; fill them
      //! with QString::arg().
      //! A directory name that cannot be a run ID: %1 the name.
      static constexpr const char* invalidRunIdMessage  = "\"%1\" cannot be a run ID: a run directory name may hold only letters, "
                                                          "digits, the underscore and the hyphen, and at most 80 of them";
      //! No .auc files: %1 the directory.
      static constexpr const char* noAucFilesMessage    = "No .auc files were found in %1";
      //! An .auc file that could not be read: %1 the directory.
      static constexpr const char* unreadableAucMessage = "A data file in %1 could not be read";
      //! Mixed run types: %1 the types found, %2 the one read.
      static constexpr const char* mixedRunTypesMessage = "the data files name more than one run type (%1); only %2 was read";
      //! Mixed run IDs: %1 the run IDs found.
      static constexpr const char* mixedRunIdsMessage   = "the data files name more than one run (%1)";
      //! The experiment XML could not be opened: %1 its path.
      static constexpr const char* experimentUnopenableMessage = "The experiment record %1 could not be opened";
      //! The experiment XML is malformed: %1 its path.
      static constexpr const char* experimentMalformedMessage  = "The experiment record %1 is not well-formed XML";
      //! The experiment XML and the .auc files disagree: %1 the XML file name,
      //! %2 the directory, %3 US_Experiment's detail.
      static constexpr const char* experimentDisagreesMessage  = "%1 and the data files in %2 disagree: %3";
      //! Appended to an unreadable-experiment error: %1 the mixed-run detail.
      static constexpr const char* alsoMixedRunMessage         = ". Also, %1";
      //! A mixed run: %1 the directory, %2 the mixed-run detail.
      static constexpr const char* notOneRunMessage            = "The run in %1 is not one run: %2";
      //! A mixed run that caused a disagreement: %1 the directory, %2 the
      //! mixed-run detail, %3 US_Experiment's detail.
      static constexpr const char* notOneRunCausedMessage      = "The run in %1 is not one run: %2. As a result, %3";
      //! A triple whose GUID differs from its .auc's: %1 the triple
      //! description, %2 the experiment's GUID, %3 the data file's GUID.
      static constexpr const char* identityMismatchMessage     = "The experiment record gives %1 the identity %2, "
                                                                 "but its data file carries %3";

      /*! \brief Give each triple with an empty solution GUID the GUID of the
          triple before it.

          US_ConvertGui::loadUS3Disk() calls this before it reads the
          solutions.  Leading triples with an empty GUID stay empty, since
          there is nothing before them to take one from.

          \param triples The triples of one run, in load order
      */
      static void                  fillSolutionGaps( QList<US_Convert::TripleInfo>& triples );

      /*! \brief The values an editedData row is created or updated with.

          Only what the stored procedures take.  Deciding whether a record is
          created or updated, what its label and comment say, and whether a
          missing raw record is fatal are all the caller's, which is why none
          of that is represented here.
      */
      struct US_UTIL_EXTERN EditedDataRecord
      {
            int     rawDataID = -1; //!< The raw record this edit belongs to
            QString editGUID;       //!< The edit's own identity
            QString label;          //!< Row label; callers pass the run ID
            QString filename;       //!< Edit XML basename, not a path
            QString comment;        //!< Free text; may be empty
      };

      /*! \brief A policy-free request to retrieve and decode one edited/raw
                   data pair.

          `editFilename` is the basename stored in editedData.  Most callers
          leave `loadFilename` empty, making it the same name.  An MWL caller
          may instead provide a logical name ending in `range@wavelength.xml`:
          US_DataIO then reads the stored range edit and the selected raw AUC.

          `rawFilename` may be supplied from a DB lookup.  If empty it is
          derived from the logical load filename by US_DataIO's canonical
          filename rule.

          The download flags let a caller retain its own cache policy.  A file
          whose flag is false must already exist in `directory`; this supports
          US_DataLoader's checksum cache and US_QueryRmsd's in-memory raw cache
          without putting either policy into this utility.
      */
      struct US_UTIL_EXTERN EditedDataReadRequest
      {
            QString directory;           //!< Destination directory
            QString editFilename;        //!< Stored edit XML basename
            QString rawFilename;         //!< Raw AUC basename, or empty
            QString loadFilename;        //!< Logical edit name, or empty
            int     editedDataID = -1;   //!< editedData primary key
            int     rawDataID    = -1;   //!< rawData primary key
            bool    downloadEdit = true; //!< Fetch edit blob when true
            bool    downloadRaw  = true; //!< Fetch raw blob when true
      };

      //! Non-database failures returned by readEditedDataFromDB().
      enum EditedDataReadStatus
      {
         EDITED_DATA_INVALID_REQUEST = 1001,
         EDITED_DATA_DIRECTORY_ERROR = 1002,
         EDITED_DATA_FILE_MISSING    = 1003,
         EDITED_DATA_DECODE_ERROR    = 1004,
         EDITED_DATA_SHAPE_ERROR     = 1005
      };

      // \brief Generic constructor for the US_ConvertIO class.
      US_ConvertIO( void );

      /*! \brief Look up the rawData row id an edit should hang from.

          \param db        An opened db connection
          \param rawGUID   The raw dataset's GUID, as the AUC header carries it
          \param rawDataID Set to the row id on success, left alone otherwise
          \param error     Set to context on failure

          \returns IUS_DB2::OK, or the underlying status.  No matching row,
                   whether the server reports NOROWS or the query succeeds
                   without one, is IUS_DB2::NO_RAWDATA: whether that is fatal
                   is the caller's decision.
      */
      static int     findRawDataId( IUS_DB2* db, const QString& rawGUID, int& rawDataID, QString& error );

      //! \brief Edited-data error formats, shared with the tests; fill them
      //! with QString::arg().
      //! No raw data row for a GUID: %1 the GUID.
      static constexpr const char* noRawDataRecordMessage     = "No raw data record exists for GUID %1";
      //! The raw data lookup failed: %1 the GUID, %2 the database error, %3 the status.
      static constexpr const char* rawDataLookupFailedMessage = "Could not look up the raw data record for GUID %1: %2 (status %3)";
      //! A raw data id that is not a number: %1 the GUID, %2 the id as stored.
      static constexpr const char* nonNumericRawDataIdMessage = "Raw data record for GUID %1 has a non-numeric id (\"%2\")";
      //! Creating an editedData row failed: %1 the edit file, %2 the database
      //! error, %3 the status.
      static constexpr const char* createEditedDataFailedMessage = "Could not create the editedData record for %1: %2 (status %3)";
      //! Updating an editedData row failed: %1 the row, %2 the edit file, %3
      //! the database error, %4 the status.
      static constexpr const char* updateEditedDataFailedMessage = "Could not update editedData record %1 for %2: %3 (status %4)";
      //! Uploading the edit XML failed: %1 the file, %2 the row, %3 the
      //! database error, %4 the status.
      static constexpr const char* uploadEditedDataFailedMessage = "Could not attach %1 to editedData record %2: %3 (status %4)";

      // writeRawDataToDB() error formats.  Each ends in the status and/or the
      // database error, and the writer appends them in the order they occur.
      //! Clearing the run's rawData failed: %1 the status, %2 the database error.
      static constexpr const char* deleteRawDataFailedMessage             = "Error returned deleting rawData\n%1 %2\n";
      //! Clearing the run's solution links failed: %1 the status, %2 the
      //! database error.
      static constexpr const char* deleteExperimentSolutionsFailedMessage = "Error returned deleting experiment solutions\n%1 %2\n";
      //! Clearing the run's cell records failed: %1 the status, %2 the
      //! database error.
      static constexpr const char* deleteCellExperimentsFailedMessage     = "Error returned deleting cell experiments\n%1 %2\n";
      //! A triple's buffer is not in the database: %1 the buffer GUID.
      static constexpr const char* bufferNotFoundMessage   = "Error processing buffer %1\nBuffer was not found in the database";
      //! A triple's analyte is not in the database.
      static constexpr const char* analyteNotFoundMessage  = "Error processing analyte \nAn analyte was not found in the database";
      //! A triple's solution is not in the database: %1 the solution GUID.
      static constexpr const char* solutionNotFoundMessage = "Error processing solution %1\nSolution was not found in the database";
      //! Saving a triple's solution failed: %1 the database error.
      static constexpr const char* saveSolutionFailedMessage = "Error saving solution to DB \n%1";
      //! The auc file could not be read for upload: %1 the file, %2 the
      //! database error.
      static constexpr const char* aucFileUnreadableMessage  = "Error processing file:\n%1\n%2\nCould not open file or no data \n";
      //! Writing a triple's rawData row or blob failed: %1 the file, %2 the
      //! database error.
      static constexpr const char* aucFileFailedMessage      = "Error returned processing file:\n%1\n%2\n";
      //! Writing a cell record failed: %1 the cell GUID, %2 the status, %3 the
      //! database error.
      static constexpr const char* cellRecordFailedMessage   = "Error returned writing cell record: %1\n%2 %3\n";
      //! Linking a solution to the experiment failed: %1 the experiment id, %2
      //! the solution GUID, %3 the status, %4 the database error.
      static constexpr const char* experimentSolutionLinkFailedMessage =
         "MySQL error associating experiment %1\n with solution %2\n code: %3  error: %4\n";
      //! Deleting the partly written experiment failed: %1 the experiment id,
      //! %2 the database error.
      static constexpr const char* deleteExperimentFailedMessage = "MySQL error deleting experiment %1\n%2\n";

      /*! \brief Create an editedData row.

          \param db           An opened db connection
          \param record       The values to write
          \param editedDataID Set to the new row id on success
          \param error        Set to context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int     createEditedData( IUS_DB2* db, const EditedDataRecord& record, int& editedDataID, QString& error );

      /*! \brief Update an existing editedData row.

          \param db           An opened db connection
          \param editedDataID The row to update
          \param record       The values to write
          \param error        Set to context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int     updateEditedData( IUS_DB2* db, int editedDataID, const EditedDataRecord& record, QString& error );

      /*! \brief Attach an edit XML file to an editedData row.

          \param db           An opened db connection
          \param editedDataID The row to attach to
          \param filename     Full path of the edit XML to upload
          \param error        Set to context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int     uploadEditedDataBlob( IUS_DB2* db, int editedDataID, const QString& filename, QString& error );

      /*! \brief Download an edit XML blob to a full destination path.

          This thin operation centralizes the stored-procedure name and error
          handling for callers which need only the XML rather than a decoded
          edited/raw pair.
      */
      static int                   downloadEditedDataBlob( IUS_DB2* db, int editedDataID, const QString& filename, QString& error );

      /*! \brief Download a raw AUC blob to a full destination path. */
      static int                   downloadRawDataBlob( IUS_DB2* db, int rawDataID, const QString& filename, QString& error );

      /*! \brief Download as requested and decode one edited/raw data pair.

          Selection of the IDs, cache/checksum decisions, destination
          lifecycle, UI, and retry policy all remain with the caller.  Output
          vectors are cleared on entry and assigned only after a complete,
          exactly-one-pair decode, so callers never observe half-loaded state.

          \returns IUS_DB2::OK; an underlying blob status; or an
                   EditedDataReadStatus value for request, filesystem, or
                   decode failures
      */
      static int                   readEditedDataFromDB( IUS_DB2*                        db,
                                                         const EditedDataReadRequest&    request,
                                                         QVector<US_DataIO::EditedData>& editedData,
                                                         QVector<US_DataIO::RawData>&    rawData,
                                                         QString&                        error );

      /*! \brief Reads entire experiment and auc files from the database,
                 save to HD
          \param runID      The run ID to look up in the database
          \param dir        The location where the binary auc files are to go.
          \param db         An opened db connection
          \param speedsteps Reference for returned experiment speed steps vector
      */
      static QString readDBExperiment( QString, QString, IUS_DB2*, QVector<SP_SPEEDPROFILE>&, const QString = QString( "" ) );

      /*! \brief Writes a new DB rawData record for each triple

          \param ExpData A reference to a structure provided by the calling
                         function that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                         function that contains all the different
                         cell/channel/wavelength defined by the xml file.
          \param dir     Local disk directory where auc files can be found
          \param db      An opened db connection
      */
      static QString writeRawDataToDB( US_Experiment&, QList<US_Convert::TripleInfo>&, const QString&, IUS_DB2* = nullptr );

      /*! \brief Checks some info that was read from disk with values from DB

          \param ExpData A reference to a structure provided by the calling
                         function that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                         function that contains all the different
                         cell/channel/wavelength defined by the xml file.
          \param db      An opened db connection
      */
      static int     checkDiskData( US_Experiment&, QList<US_Convert::TripleInfo>&, IUS_DB2* = nullptr );
      static int     checkDiskData_auto( US_Experiment&, QList<US_Convert::TripleInfo>&, IUS_DB2* = nullptr );

   private:
      static QString readRawDataFromDB( US_Experiment&, QList<US_Convert::TripleInfo>&, QString&, IUS_DB2* = nullptr );
};
#endif
