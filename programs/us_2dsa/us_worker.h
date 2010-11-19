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
      void compute_L_tildes( int, int, int,
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

      QMutex mutex;
      QWaitCondition condition;

      double  llim_s;
      double  llim_k;

      int     thrx;
      int     taskx;
      int     depth;
      int     iter;
      int     typeref;
      int     nscans;
      int     npoints;
      int     nsolutes;
      int     noisflag;
      int     dbg_level;

      bool    abort;

      US_DataIO2::EditedData* edata;
      US_DataIO2::RawData     sdata;
      US_DataIO2::RawData     rdata;
      US_Model                model;
      US_Noise                ri_noise;
      US_Noise                ti_noise;
      US_Noise                ra_noise;
      US_SimulationParameters simparms;

      QVector< Solute >       solute_i;
      QVector< Solute >       solute_c;
};

#endif

