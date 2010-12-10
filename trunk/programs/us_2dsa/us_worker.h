//! \file us_worker.h
#ifndef US_THREAD_WORKER_H
#define US_THREAD_WORKER_H

#include <QtCore>

#include "us_extern.h"
#include "us_solute.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Worker thread to do actual work of 2DSA analysis

/*! \class WorkerThread
 *
    This class is for each of the individual worker threads that do the
    actual work of 2DSA analysis.
*/
class WorkerThread : public QThread
{
   Q_OBJECT

   public:
      WorkerThread( QObject* parent = 0 );
      ~WorkerThread();

      void define_work( WorkPacket& );
      void get_result(  WorkPacket& );
      void run();
      void flag_abort();

   signals:
      void work_progress( int );
      void work_complete( WorkerThread* );

   private:

      void calc_residuals(    void );
      void compute_a_tilde(  QVector< double >& );
      void compute_L_tildes( int, int,
                             QVector< double >&,
                             const QVector< double >& );
      void compute_L_tilde(  QVector< double >&,
                             const QVector< double >& );
      void compute_L(        int, int,
                             QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >& );
      void ri_small_a_and_b( int, int, int,
                             QVector< double >&,
                             QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >& );
      void ti_small_a_and_b( int, int, int,
                             QVector< double >&,
                             QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >& );
      void compute_L_bar(    QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >& );
      void compute_a_bar(    QVector< double >&,
                             const QVector< double >& );
      void compute_L_bars(   int, int, int, int,
                             QVector< double >&,
                             const QVector< double >&,
                             const QVector< double >& );

      double  llim_s;        // lower limit in s (UGRID)
      double  llim_k;        // lower limit in k (UGRID)

      int     thrn;          // thread number (1,...)
      int     taskx;         // grid refinement task index
      int     depth;         // depth index
      int     iter;          // iteration index
      int     menmcx;        // meniscus / monte carlo index
      int     typeref;       // type of refinement (0=UGRID, ...)
      int     nscans;        // number of scans in experiment
      int     npoints;       // number of radius points in experiment
      int     nsolutes;      // number of input solutes for this task
      int     noisflag;      // noise flag (0-3 for NONE|TI|RI|BOTH)
      int     dbg_level;     // debug flag

      bool    abort;         // should this thread be aborted?

      US_DataIO2::EditedData* edata;       // experiment data (pointer)
      US_DataIO2::RawData     sdata;       // simulation data
      US_DataIO2::RawData     rdata;       // residuals
      US_Model                model;       // output model
      US_Noise                ri_noise;    // computed radially-invariant noise
      US_Noise                ti_noise;    // computed time-invariant noise
      US_Noise                ra_noise;    // computed random noise
      US_SimulationParameters simparms;    // simulation parameters

      QVector< Solute >       solute_i;    // input solutes
      QVector< Solute >       solute_c;    // computed solutes
};

#endif

