//! \file us_pcsa_process.h
#ifndef US_PCSA_PROCESS_H
#define US_PCSA_PROCESS_H

#include <QtCore>
#include <QTimer>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_db2.h"
#include "us_solute.h"
#include "us_zsolute.h"
#include "us_worker_pc.h"
#include "us_pcsa_modelrec.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif
#ifndef US_ZS_ATTRIBS
#define US_ZS_ATTRIBS
#define ATTR_S US_ZSolute::ATTR_S
#define ATTR_K US_ZSolute::ATTR_K
#define ATTR_W US_ZSolute::ATTR_W
#define ATTR_V US_ZSolute::ATTR_V
#define ATTR_D US_ZSolute::ATTR_D
#define ATTR_C US_ZSolute::ATTR_C
#endif

//! \brief PCSA Processor object

/*! \class US_pcsaProcess
 *
    This class sets up a set of PCSA simulations for a
    grid across an s and k range. It divides the refinements
    in the grid across a specified number of worker threads.
*/
class US_pcsaProcess : public QObject
{
   Q_OBJECT

   public:
      //! The state of a task
      enum TaskState  { READY, WORKING, ABORTED };

      //! \brief Create a PCSA processor object
      //! \param dsets     List of experiment data sets
      //! \param parent    Pointer to parent object
      US_pcsaProcess( QList< US_SolveSim::DataSet* >&, QObject* = 0 );

      //! \brief Start the fit calculations
      //! \param xll     x lower limit
      //! \param xul     x upper limit
      //! \param yll     y lower limit
      //! \param yul     y upper limit
      //! \param nyp     number of y points (variations)
      //! \param res     resolution == line points count (model solutes)
      //! \param typ     curve type (0->straight lines)
      //! \param nth     number of threads
      //! \param noi     noise flag: 0-3 for none|ti|ri|both
      //! \param lmmxc   L-M maximum calls
      //! \param gfits   grid fit maximum iterations
      //! \param gfthr   grid fit threshold difference fraction
      //! \param alf     regularization alpha factor
      void start_fit( double, double, double, double, int, int,
                      int, int, int, int, int, double, double = 0.0 );

      //! \brief Complete the final fit for alpha scan or alpha change
      //! \param alf     regularization alpha factor
      void final_fit( double );

      //! \brief Get results upon completion of all refinements
      //! \param da_sim    Calculated simulation data
      //! \param da_res    Residuals data (exper - simul)
      //! \param da_mdl    Composite model
      //! \param da_tin    Time-invariant noise (or null)
      //! \param da_rin    Radially-invariant noise (or null)
      //! \param bm_ndx    Best model index
      //! \param modstats  Mrecs report stringlist
      //! \param p_mrecs   Model record vector
      //! \returns         Success flag:  true if successful
      bool get_results( US_DataIO::RawData*, US_DataIO::RawData*,
                        US_Model*, US_Noise*, US_Noise*, int&,
                        QStringList&, QVector< US_ModelRecord >& );

      //! \brief Get best mrec result needed for an alpha scan
      //! \param p_mrec  Model record
      void get_mrec( US_ModelRecord& );

      //! \brief Replace best mrec in internal mrecs list
      //! \param a_mrec  Model record
      void put_mrec( US_ModelRecord& );

      //! \brief Replace internal mrecs list
      //! \param a_mrecs Model records list
      void put_mrecs( QVector< US_ModelRecord >& );

      //! \brief Stop a fit that is in progress
      void stop_fit(       void );

      //! \brief Clear memory in preparation for fit start
      void clear_memory(   void );

      static const int solute_doubles = sizeof( US_ZSolute ) / sizeof( double );

      // Static routines needed for Levenberg-Marquardt
      static double fit_function_SL( double, double* );
      static double fit_function_IS( double, double* );
      static double fit_function_DS( double, double* );
      static double fit_function_HL( double, double* );

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

      QList< WorkerThreadPc* >   wthreads;   // worker threads
      QList< WorkPacketPc >      job_queue;  // job queue

      QVector< US_ModelRecord >  mrecs;      // model records for each task

      QVector< int >             wkstates;   // worker thread states

      QList< QVector< US_ZSolute > > orig_sols;  // input solutes

      QVector< double >          rmsds;      // gfit iteration RMSDs
      US_DataIO::EditedData*     edata;      // experimental data (mc_iter)
      US_DataIO::RawData         sdata;      // simulation data
      US_DataIO::RawData         rdata;      // residuals data

      US_Model                   model;      // constructed model

      US_Noise                   ti_noise;   // time-invariant noise
      US_Noise                   ri_noise;   // radially-invariant noise

      US_SimulationParameters*   simparms;   // simulation parameters

      QString    errMsg;       // message from last error

      int        dbg_level;    // debug level
      int        nthreads;     // number of worker threads
      int        cresolu;      // curve resolution (points on the line)
      int        curvtype;     // curve type flag (0->straight line)
      int        nypts;        // number of y points
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
      int        fi_iter;      // Fit iteration counter
      int        fi_itermax;   // Maximum fit iterations
      int        lmmxcall;     // L-M maximum calls
      int        st_mask;      // Solute type mask
      int        attr_x;       // Attribute X index
      int        attr_y;       // Attribute Y index
      int        attr_z;       // Attribute Z index

      bool       abort;        // flag used with stop_fit clicked
      bool       lm_done;      // flag for L-M completion
      bool       alpha_scn;    // flag for alpha-scan only

      double     xlolim;       // x lower limit
      double     xuplim;       // x upper limit
      double     ylolim;       // y lower limit
      double     yuplim;       // y upper limit
      double     cparam;       // additional curve parameter
      double     varimin;      // variance minimum
      double     alpha;        // Tikhonov regularization factor
      double     alpha_fx;     // Alpha for fixed curves fits
      double     alpha_lm;     // Alpha for L-M fits
      double     parlims[ 12 ];// Par1,Par2 limits and other constants
      double     pfi_rmsd;     // Previous fit iteration RMSD
      double     cfi_rmsd;     // Current fit iteration RMSD
      double     rd_frac;      // RMSD-difference fraction: (p-c)/p
      double     rd_thresh;    // RMSD-difference threshold factor

      QElapsedTimer      timer;        // timer for elapsed time measure

   private slots:
      void queue_task      ( WorkPacketPc&, double, double,
                             int, int, QVector< US_ZSolute > );
      int  slmodels        ( int, double, double, double, double, int, int );
      int  sigmodels       ( int, double, double, double, double, int, int );
      int  pl2models       ( double, double, double, double, int, int );
      void process_job     ( WorkerThreadPc* );
      void process_fxfinal ( US_ModelRecord&  );
      void submit_job      ( WorkPacketPc&, int );
      void free_worker     ( int  );
      void model_statistics( QVector< US_ModelRecord >&, QStringList& );
      QString pmessage_head( void );
      WorkPacketPc next_job( void );
      void LevMarq_fit     ( void );
      void compute_final   ( void );
      void elite_limits    ( QVector< US_ModelRecord >&, double&, double&,
                             double&, double&, double&, double& );
      void restart_fit     ( void );
      static double evaluate_model( QList< US_SolveSim::DataSet* >&,
                                    US_SolveSim::Simulation& );

   protected:
      virtual void timerEvent( QTimerEvent *e );
};
#endif

