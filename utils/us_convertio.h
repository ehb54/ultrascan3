//! \file us_convertio.h
#ifndef US_CONVERTIO_H
#define US_CONVERTIO_H

#include <QtCore>

#include "us_extern.h"
#include "us_convert.h"
#include "us_experiment.h"
#include "ius_db2.h"
#include "us_db2.h"

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
          database connection, no edit files, no corpus rules, no settings, no
          temporary-directory lifecycle.
      */
      struct US_UTIL_EXTERN DiskRun
      {
         QString                         directory;   //!< With a trailing separator
         QString                         runID;       //!< From the directory basename
         QString                         runType;     //!< "RA", "RI", "IP", ...
         QVector< US_DataIO::RawData >   rawData;     //!< One per AUC, filename order
         QList< US_Convert::TripleInfo > triples;     //!< Parallel to rawData
         QVector< SP_SPEEDPROFILE >      speedSteps;  //!< As serialized; may be empty
         US_Experiment                   experiment;
      };

      /*! \brief Read one run directory into a DiskRun.

          A thin coordinator over the existing readers.  It reports structural
          problems and does not reject them: it returns the most specific
          status it can, fills `error` with the detail, and populates `run` as
          far as the read got.  Whether a non-OK status ends the load is the
          caller's decision -- `us_import_run` treats every one as fatal, while
          US_ConvertGui recovers from exactly what it recovered from before and
          uses the detail only to say more in its existing messages.

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

          \returns A US_Convert::ioError value.  US_Convert::INVALID_RUN marks
                   a structural contradiction with no honest existing code:
                   an unusable run ID, no raw data, a raw dataset with no
                   scans, or filenames carrying mixed run IDs or run types.
      */
      static int readDiskRun( const QString& directory, DiskRun& run,
                              QString& error );

      /*! \brief The values an editedData row is created or updated with.

          Only what the stored procedures take.  Deciding whether a record is
          created or updated, what its label and comment say, and whether a
          missing raw record is fatal are all the caller's, which is why none
          of that is represented here.
      */
      struct US_UTIL_EXTERN EditedDataRecord
      {
         int     rawDataID = -1;  //!< The raw record this edit belongs to
         QString editGUID;        //!< The edit's own identity
         QString label;           //!< Row label; callers pass the run ID
         QString filename;        //!< Edit XML basename, not a path
         QString comment;         //!< Free text; may be empty
      };

      // \brief Generic constructor for the US_ConvertIO class.
      US_ConvertIO( void );

      /*! \brief Look up the rawData row id an edit should hang from.

          \param db        An opened db connection
          \param rawGUID   The raw dataset's GUID, as the AUC header carries it
          \param rawDataID Set to the row id on success, left alone otherwise
          \param error     Appended with context on failure

          \returns IUS_DB2::OK, or the underlying status.  A query that
                   succeeds but names no row is IUS_DB2::NO_RAWDATA: whether
                   that is fatal is the caller's decision.
      */
      static int findRawDataId( IUS_DB2* db, const QString& rawGUID,
                                int& rawDataID, QString& error );

      /*! \brief Create an editedData row.

          \param db           An opened db connection
          \param record       The values to write
          \param editedDataID Set to the new row id on success
          \param error        Appended with context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int createEditedData( IUS_DB2* db,
                                   const EditedDataRecord& record,
                                   int& editedDataID, QString& error );

      /*! \brief Update an existing editedData row.

          \param db           An opened db connection
          \param editedDataID The row to update
          \param record       The values to write
          \param error        Appended with context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int updateEditedData( IUS_DB2* db, int editedDataID,
                                   const EditedDataRecord& record,
                                   QString& error );

      /*! \brief Attach an edit XML file to an editedData row.

          \param db           An opened db connection
          \param editedDataID The row to attach to
          \param filename     Full path of the edit XML to upload
          \param error        Appended with context on failure

          \returns IUS_DB2::OK, or the underlying status
      */
      static int uploadEditedDataBlob( IUS_DB2* db, int editedDataID,
                                       const QString& filename,
                                       QString& error );

      /*! \brief Reads entire experiment and auc files from the database,
                 save to HD
          \param runID      The run ID to look up in the database
          \param dir        The location where the binary auc files are to go.
          \param db         An opened db connection
          \param speedsteps Reference for returned experiment speed steps vector
      */
      static QString readDBExperiment( QString, QString, US_DB2*,
				       QVector< SP_SPEEDPROFILE >&, const QString = QString("") );

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
      static QString writeRawDataToDB(
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 const QString&,
                 US_DB2* = 0 );

      /*! \brief Checks some info that was read from disk with values from DB

          \param ExpData A reference to a structure provided by the calling
                         function that contains the hardware and other database
                         connection information provided by the xml file.
          \param triples A reference to a structure provided by the calling
                         function that contains all the different
                         cell/channel/wavelength defined by the xml file.
          \param db      An opened db connection
      */
      static int checkDiskData( 
                 US_Experiment&,
                 QList< US_Convert::TripleInfo >& ,
                 US_DB2* = 0 );
      static int checkDiskData_auto( 
                 US_Experiment&,
                 QList< US_Convert::TripleInfo >& ,
                 US_DB2* = 0 );

   private:
      static QString readRawDataFromDB( 
                 US_Experiment& , 
                 QList< US_Convert::TripleInfo >& ,
                 QString& ,
                 US_DB2* = 0 );
      
};
#endif
