//! \file us_sync_exper.h
#ifndef US_SYNC_EXPER_H
#define US_SYNC_EXPER_H

#include <QtGui>
#include <QtXml>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_data_model.h"

/*! \class US_SyncExperiment
           This class provides the ability to synchronize an
           experiment record on local disk and in the database.
*/
class US_EXTERN US_SyncExperiment : public QObject
{
   Q_OBJECT

   public:
      /*! \brief Constructor for the US_SyncExperiment class.
      
      Sets up the database person ID and pointer to the database connection

          \param    db       A pointer to an open database connection
          \param    investig Investigator string ("ID: LastName, FirstName")
          \param    parent   Pointer to parent QWidget.
      */
      US_SyncExperiment( US_DB2*, QString, QWidget* = 0 );

      /*! \brief    Synchronize database and local experiment records

                    This function evaluates the state of the experiment
                    records associated with a raw data record and updates
                    flags whether the upload/download can proceed.

          \param    cdesc   Current description object for the raw data
                            record being operated on (uploaded/downloaded).
          \returns          A status flag (0 if all OK)
      */
      int synchronize( US_DataModel::DataDesc& );

   private:
      US_DB2*     db;

      QStringList expIDs;
      QStringList runIDs;

      QWidget*    parentw;

      QString     invID;
      QString     expID;
      QString     rawID;

      QString expGUIDauc( QString );  // get exper GUID from AUC-related file

      int           dbg_level;
};
#endif
