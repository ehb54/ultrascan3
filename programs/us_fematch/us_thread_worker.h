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
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

//! \brief Worker thread to do Lamm equation calculations for us_fematch

/*! \class ThreadWorker
    This class is for each of the individual worker threads that do the
    actual work of FeMatch analysis using a partitioned model.
*/
class ThreadWorker : public QObject {
      Q_OBJECT

   public:
      ThreadWorker(US_Model &, US_SimulationParameters &, US_DataIO::RawData &, US_Buffer &, int);

   public slots:
      void calc_simulation(); // Where the real work is done
      void forward_progress(int);

   signals:
      void work_progress(int, int);
      void work_complete(int);

   private:
      US_Model &model; // Model for thread
      US_SimulationParameters &simparams; // Simulation parameters
      US_DataIO::RawData &simdat; // Simulation data (pre-inited)
      US_Buffer &buffer; // Buffer (density,compress)
      int thrn; // thread number (0,...)

      int dbg_level; // debug flag
};

/*! \class QThreadEx
    A special QThread that calls exec() to start an event loop
*/
class QThreadEx : public QThread {
   protected:
      void run() { exec(); }
};

#endif
