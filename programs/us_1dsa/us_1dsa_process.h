//! \file us_1dsa_process.h
#ifndef US_1DSA_PROCESS_H
#define US_1DSA_PROCESS_H

#include <QtCore>
#include <QTimer>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_simparms.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_db2.h"
#include "us_solute.h"
#include "us_worker.h"
#include "us_model_record.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief 1DSA Processor object

/*! \class US_1dsaProcess
 *
    This class sets up a set of 1DSA simulations for a
    grid across an s and k range. It divides the refinements
    in the grid across a specified number of worker threads.
*/
class US_1dsaProcess : public QObject
{
   Q_OBJECT

   public:
      //! The state of a task
      enum TaskState  { READY, WORKING, ABORTED };

      //! \brief Create a 1DSA processor object
      //! \param dsets     List of experiment data sets
      //! \param parent    Pointer to parent object
      US_1dsaProcess( QList< US_SolveSim::DataSet* >&, QObject* = 0 );

      //! \brief Start the fit calculations
      //! \param sll     s lower limit
      //! \param sul     s upper limit
      //! \param kll     k lower limit
      //! \param kul     k upper limit
      //! \param kin     k increment
      //! \param res     resolution == line points count
      //! \param typ     curve type (0->straight lines)
      //! \param nth     number of threads
      //! \param noi     noise flag: 0-3 for none|ti|ri|both
      void start_fit( double, double, double, double, double,
                      int, int, int, int );

      //! \brief Get results upon completion of all refinements
      //! \param da_sim  Calculated simulation data
      //! \param da_res  Residuals data (exper - simul)
      //! \param da_mdl  Composite model
      //! \param da_tin  Time-invariant noise (or null)
      //! \param da_rin  Radially-invariant noise (or null)
      //! \param bm_ndx  Best model index
      //! \param report  Mrecs report stringlist
      //! \param mrecs   Model record vector
      //! \returns       Success flag:  true if successful
      bool get_results( US_DataIO2::RawData*, US_DataIO2::RawData*,
                        US_Model*, US_Noise*, US_Noise*, int&,
                        QStringList&, QVector< ModelRecord >& );

      void stop_fit(       void );
      int  estimate_steps( int  );

      //! \brief Get message for last error
      //! \returns       Message about last error
      QString lastError( void ) { return errMsg; }

      static const int solute_doubles = sizeof( US_Solute ) / sizeof( double );

      static double fit_function_SL( double, double* );
      static double fit_function_IS( double, double* );
      static double fit_function_DS( double, double* );

private:

      signals:
      void progress_update(  double );
      void process_complete( int  );
      void stage_complete(   int,     int  );
      void message_update(   QString, bool );

private:
      QList< US_SolveSim::DataSet* >& dsets;

      long int maxrss;

      long int max_rss( void );

      QList< WorkerThread* >     wthreads;   // worker threads
      QList< WorkPacket >        job_queue;  // job queue

      QVector< ModelRecord >     mrecs;      // model records for each task

      QVector< int >             wkstates;   // worker thread states

      QList< QVector< US_Solute > > orig_sols;  // input solutes

      US_DataIO2::EditedData*    edata;      // experimental data (mc_iter)
      US_DataIO2::RawData        sdata;      // simulation data
      US_DataIO2::RawData        rdata;      // residuals data

      US_Model                   model;      // constructed model

      US_Noise                   ti_noise;   // time-invariant noise
      US_Noise                   ri_noise;   // radially-invariant noise

      US_SimulationParameters*   simparms;   // simulation parameters

      QString    errMsg;       // message from last error

      int        dbg_level;    // debug level
      int        nthreads;     // number of worker threads
      int        cresolu;      // curve resolution (points on the line)
      int        curvtype;     // curve type flag (0->straight line)
      int        nctotal;      // number of total compute-progress steps
      int        kcsteps;      // count of completed progress steps
      int        noisflag;     // noise out flag: 0(none), 1(ti), 2(ri), 3(both)
      int        nscans;       // number of experiment scans
      int        npoints;      // number of reading points per experiment scan
      int        nmtasks;      // number of models/tasks to do
      int        kctask;       // count of completed subgrid tasks
      int        kstask;       // count of started subgrid tasks;
      int        minvarx;      // minimum variance model index
      int        time_fg;      // time in milliseconds for fixed-grid calcs
      int        time_lm;      // time in milliseconds for L-M calcs
      int        lmtm_id;      // L-M timing event ID

      bool       abort;        // flag used with stop_fit clicked
      bool       lm_done;      // flag for L-M completion

      double     slolim;       // s lower limit
      double     suplim;       // s upper limit
      double     klolim;       // k lower limit
      double     kuplim;       // k upper limit
      double     kincr;        // k increment
      double     cparam;       // additional curve parameter
      double     varimin;      // variance minimum

      QTime      timer;        // timer for elapsed time measure

   private slots:
      void queue_task   ( WorkPacket&, double, double,
                          int, int, QVector< US_Solute > );
      int  slmodels     ( double, double, double, double, double, int );
      int  sigmodels    ( int, double, double, double, double, int, int );
      void process_job  ( WorkerThread* );
      void process_final( ModelRecord&  );
      void submit_job   ( WorkPacket&, int );
      void free_worker  ( int  );
      void model_statistics( QVector< ModelRecord >&, QStringList& );
      QString pmessage_head( void );
      WorkPacket next_job  ( void );
      void LevMarq_fit     ( void );
      void elite_limits    ( QVector< ModelRecord >&, double&, double&,
                             double&, double&, double&, double& );
      static double evaluate_model( QList< US_SolveSim::DataSet* >&,
                                    US_SolveSim::Simulation& );

   protected:
      virtual void timerEvent( QTimerEvent *e );
};
#endif

