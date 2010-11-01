//! \file us_2dsa_process.h
#ifndef US_2DSA_PROCESS_H
#define US_2DSA_PROCESS_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_EXTERN US_2dsaProcess : public QObject
{
   Q_OBJECT

   public:
      US_2dsaProcess( US_DataIO2::EditedData*, QObject* = 0 );

      QString lastError( void ) { return errMsg; }

   private:
      QString                 errMsg;     // message from last error

      QObject*                parentw;    // parent object

      US_DataIO2::EditedData* edata;      // experimental data

      US_DataIO2::RawData*    sdata;      // simulation data

      US_Model*               model;      // constructed model

      int                     dbg_level;

      QString get_model_filename( QString );
      QString get_noise_filename( QString );

};
#endif

