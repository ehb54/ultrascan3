//! \file us_2dsa_process.h
#ifndef US_2DSA_PROCESS_H
#define US_2DSA_PROCESS_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO2.h"
#include "us_simparms.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_db2.h"
#include "us_solute.h"
#include "us_worker.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


//! \brief 2DSA Processor object

/*! \class US_2dsaProcess
 *
    This class sets up a set of 2DSA simulations for a
    grid across an s and k range. It divides the refinements
    in the grid across a specified number of worker threads.
*/
class US_EXTERN US_2dsaProcess : public QObject
{
   Q_OBJECT

   public:

      //! The type of refinement used
      enum RefineType { UGRID, LUGRID, RLGRID, SOLCO, CLIPLO };

      //! The state of a task
      enum TaskState  { READY, WORKING, ABORTED };

      //! \brief Create a 2DSA processor object
      //! \param da_exper  Pointer to input experiment data
      //! \param sim_pars  Pointer to simulation parameters
      //! \param parent    Pointer to parent object
      US_2dsaProcess( US_DataIO2::EditedData*,
                      US_SimulationParameters*,
                      QObject* = 0 );

      //! \brief Start the fit calculations
      //! \param sll     s lower limit
      //! \param sul     s upper limit
      //! \param nss     number of s steps
      //! \param kll     k lower limit
      //! \param kul     k upper limit
      //! \param nks     number of k steps
      //! \param ngf     number of grid refinements
      //! \param nthr    number of threads
      //! \param noif    noise flag: 0-3 for none|ti|ri|both
      void start_fit( double, double, int, double, double, int,
                      int, int, int );

      //! \brief Set up iteration-related parameters for a fit
      //! \param mxiter  Maximum refinement iterations
      //! \param mciter  Number of monte carlo iterations
      //! \param mniter  Number of meniscus iterations
      //! \param vtoler  Variance difference tolerance
      //! \param menrng  Meniscus range
      void set_iters( int, int, int, double, double );

      //! \brief Get results upon completion of all refinements
      //! \param da_sim  Calculated simulation data
      //! \param da_res  Residuals data (exper - simul)
      //! \param da_mdl  Composite model
      //! \param da_tin  Time-invariant noise (or null)
      //! \param da_rin  Radially-invariant noise (or null)
      //! \returns       Success flag:  true if successful
      bool get_results( US_DataIO2::RawData*, US_DataIO2::RawData*,
                        US_Model*, US_Noise*, US_Noise* );

      void stop_fit(       void );

      //! \brief Get message for last error
      //! \returns       Message about last error
      QString lastError( void ) { return errMsg; }

      static const int solute_doubles = sizeof( Solute ) / sizeof( double );

      class Simulation
      {
         public:
            double variance;
            QVector< double > ti_noise;
            QVector< double > ri_noise;
            QVector< Solute > solutes;
      };

      public slots:
      void thread_finished( WorkerThread* );
      void final_finished(  WorkerThread* );
      void step_progress( int );

      signals:
      void progress_update(  int  );
      void refine_complete(  int  );
      void process_complete( void );
      void stage_complete(   int,     int  );
      void message_update(   QString, bool );

      private slots:
      QVector< Solute > create_solutes( double, double, double,
                                        double, double, double );
      void queue_task(     WorkPacket&, double, double,
                           int, int, int, QVector< Solute > );
      void final_computes( void );
      void iterate(        void );
      void submit_job(     WorkPacket&, int );
      void process_job(    WorkerThread*    );

      private:

      long int maxrss;

      long int max_rss( void );

      QList< WorkerThread* >     wthreads;   // worker threads
      QList< WorkPacket >        job_queue;  // job queue

      QVector< int >             thstates;   // thread states
      QVector< int >             wkdepths;   // work packet depths

      QList< double >            itvaris;    // iteration variances

      QVector< Solute >          c_solutes;  // calculated solutes
      QVector< Solute >          d_solutes;  // next-depth calculated solutes

      QList< QVector< Solute > > orig_sols;  // original solutes
      QList< QVector< Solute > > icmp_sols;  // iteration computed solutes

      US_DataIO2::EditedData*    edata;      // experimental data (mc_iter)
      US_DataIO2::EditedData*    bdata;      // base experimental data
      US_DataIO2::EditedData     wdata;      // work experimental data

      US_DataIO2::RawData        sdata;      // simulation data

      US_DataIO2::RawData        rdata;      // residuals data

      US_Model                   model;      // constructed model

      US_Noise                   ti_noise;   // time-invariant noise
      US_Noise                   ri_noise;   // radially-invariant noise

      US_SimulationParameters*   simparms;   // simulation parameters

      QObject*   parentw;      // parent object

      QString    errMsg;       // message from last error

      int        dbg_level;    // debug level
      int        nthreads;     // number of worker threads
      int        nssteps;      // number of s steps
      int        nksteps;      // number of k steps
      int        ngrefine;     // number of grid refinements
      int        nctotal;      // number of total compute-progress steps
      int        kcsteps;      // count of completed progress steps
      int        noisflag;     // noise out flag: 0(none), 1(ti), 2(ri), 3(both)
      int        nscans;       // number of experiment scans
      int        npoints;      // number of reading points per experiment scan
      int        nsubgrid;     // number of subgrids (tasks)
      int        kctask;       // count of completed subgrid tasks
      int        kstask;       // count of started subgrid tasks;
      int        mmiters;      // number of meniscus or monte carlo iterations
      int        mmtype;       // mm type: 0,1,2 for NONE|MENISCUS|MONTECARLO
      int        maxtsols;     // maximum number of task solutes
      int        mintsols;     // minimum number of depth 1ff task solutes
      int        deptho;       // depth of current outputs
      int        maxiters;     // maximum number of refinement iterations
      int        r_iter;       // refinement iteration index
      int        mm_iter;      // meniscus/MC iteration index

      bool       abort;        // flag used with stop_fit clicked

      double     slolim;       // s lower limit
      double     suplim;       // s upper limit
      double     klolim;       // k lower limit
      double     kuplim;       // k upper limit
      double     gdelta_s;     // grid delta in s
      double     gdelta_k;     // grid delta in k
      double     sdelta_s;     // subgrid delta in s
      double     sdelta_k;     // subgrid delta in k
      double     varitol;      // variance difference tolerance
      double     menrange;     // meniscus range

      QTime      timer;        // timer for elapsed time measure
};
#endif

