#ifndef US_DATA_PROCESS_H
#define US_DATA_PROCESS_H

#include <QtGui>

#include "us_extern.h"
#include "us_data_model.h"
#include "us_sync_exper.h"
#include "us_dataIO2.h"
#include "us_db2.h"

class US_EXTERN US_DataProcess : public QObject
{
   Q_OBJECT

   public:
      US_DataProcess( US_DataModel*, QWidget* = 0 );

      int record_upload(       int );
      int record_download(     int );
      int record_remove_db(    int );
      int record_remove_local( int );

   private:
      QWidget*                parentw;

      US_DB2*                 db;

      US_DataModel*           da_model;   // data model object

      US_DataModel::DataDesc  cdesc;      // current record description

      US_SyncExperiment*      syncExper;  // experiment synchronizer

      QString get_model_filename( QString );

};
#endif
