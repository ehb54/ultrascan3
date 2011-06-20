#ifndef US_DATA_PROCESS_H
#define US_DATA_PROCESS_H

#include <QtGui>

#include "us_extern.h"
#include "us_data_model.h"
#include "us_sync_exper.h"
#include "us_dataIO2.h"
#include "us_db2.h"

class US_DataProcess : public QObject
{
   Q_OBJECT

   public:
      US_DataProcess( US_DataModel*, QWidget* = 0 );

      int     record_upload      ( int );
      int     record_download    ( int );
      int     record_remove_db   ( int );
      int     record_remove_local( int );

      QString lastError          ( void     );
      void    partialError       ( int, int );
      void    appendError        ( QString  );
      bool    raw_ancestor_ok    ( int      );

   private:
      QString                 errMsg;     // message from last error

      QWidget*                parentw;    // parent widget

      US_DB2*                 db;         // pointer to database connection

      US_DataModel*           da_model;   // data model object

      US_DataModel::DataDesc  cdesc;      // current record description

      US_SyncExperiment*      syncExper;  // experiment synchronizer

      int                     dbg_level;

   private slots:
      QString get_model_filename( QString );
      QString get_noise_filename( QString );
};
#endif
