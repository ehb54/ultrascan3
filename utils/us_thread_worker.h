//! \file us_thread_worker.h
#ifndef US_THREAD_WORKER_H
#define US_THREAD_WORKER_H

#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_simparms.h"

#ifndef DbgLv
#define DbgLv( a )                                                                                                                 \
   if ( dbg_level >= a )                                                                                                           \
   qDebug()
#endif

//! \brief Worker thread that simulates one part of a partitioned model

/*! \class US_ThreadWorker
    Each worker solves the Lamm equations for its share of a model's
    components. FEMatch, autoflow analysis and the GMP reporter run several
    and add their simulations together.
*/
class US_UTIL_EXTERN US_ThreadWorker : public QObject
{
      Q_OBJECT

   public:
      US_ThreadWorker( US_Model&, US_SimulationParameters&, US_DataIO::RawData&, US_Buffer&, int );

   public slots:
      void calc_simulation(); // Where the real work is done
      void forward_progress( int );

   signals:
      void work_progress( int, int );
      void work_complete( int );

   private:
      US_Model&                model;     // Model for thread
      US_SimulationParameters& simparams; // Simulation parameters
      US_DataIO::RawData&      simdat;    // Simulation data (pre-inited)
      US_Buffer&               buffer;    // Buffer (density,compress)
      int                      thrn;      // thread number (0,...)

      int                      dbg_level; // debug flag
};

/*! \class US_ThreadEx
    A QThread that runs an event loop, for a US_ThreadWorker moved to it
*/
class US_UTIL_EXTERN US_ThreadEx : public QThread
{
   protected:
      void run()
      {
         exec();
      }
};

#endif
