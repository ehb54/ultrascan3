//! \file us_worker_2d.h
#ifndef US_THREAD_WORKER_H
#define US_THREAD_WORKER_H

#include <QtCore>

#include "us_extern.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_solute.h"
#include "us_solve_sim.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Worker thread task packet
typedef struct work_packet_2d_s
{
   int     thrn;       //!< thread number (1,...)
   int     taskx;      //!< task index (0,...)
   int     depth;      //!< depth index (0,...)
   int     iter;       //!< iteration index (0,...)
   int     menmcx;     //!< meniscus/monte-carlo index (0,...)
   int     typeref;    //!< refinement-type flag (0,... for UGRID,...)
   int     state;      //!< state flag (0-3 for READY,RUNNING,COMPLETE,ABORTED)
   int     noisf;      //!< noise flag (0-3 for NONE,TI,RI,BOTH)

   double  ll_s;       //!< subgrid lower-limit s
   double  ll_k;       //!< subgrid lower-limit k

   QVector< US_Solute >     isolutes;  //!< input solutes
   QVector< US_Solute >     csolutes;  //!< computed solutes
   QVector< double >        ti_noise;  //!< computed ti noise
   QVector< double >        ri_noise;  //!< computed ri noise
   QVector< double >        Anorm;     //!< computed A matrix norm


   QList< US_SolveSim::DataSet* > dsets; //!< list of data set object pointers
   US_SolveSim::Simulation  sim_vals;  //!< simulation values
   US_Math_BF::Band_Forming_Gradient* bandFormingGradient;
   US_LammAstfvm::CosedData* cosedData;


} WorkPacket2D;

//! \brief Worker thread to do actual work of 2DSA analysis

//! \class WorkerThread2D
//! This class is for each of the individual worker threads that do the
//! actual computational work of 2DSA analysis.
class WorkerThread2D : public QThread
{
   Q_OBJECT

   public:
      explicit WorkerThread2D( QObject* parent = nullptr );
      ~WorkerThread2D() override;

      QVector< double >        norms;

   public slots:
      //! \brief Define the work packet for a worker thread
      //! \param workin   Input work definition packet
      void define_work     ( WorkPacket2D& );
      //! \brief Get the results work packet of a completed worker thread
      //! \param workout  Output work definition packet
      void get_result      ( WorkPacket2D& );
      //! \brief Run the worker thread
      void run             () override;
      //! \brief Set a flag so a worker thread will abort as soon as possible
      void flag_abort      ();
      //! \brief Public slot to forward a progress signal
      void forward_progress( int  );

   signals:
      void work_progress   ( int );
      void work_complete   ( WorkerThread2D* );

   private:

      void calc_residuals   ( );
      void calc_resids_ratio( );
//      void calculate_norm   ( QVector< double >, QVector< double >  );

      long int maxrss{};

      double  llim_s{};        // lower limit in s (UGRID)
      double  llim_k{};        // lower limit in k (UGRID)

      int     thrn;          // thread number (1,...)
      int     taskx{};         // grid refinement task index
      int     depth{};         // depth index
      int     iter{};          // iteration index
      int     menmcx{};        // meniscus / monte carlo index
      int     typeref{};       // type of refinement (0=UGRID, ...)
      int     nscans{};        // number of scans in experiment
      int     npoints{};       // number of radius points in experiment
      int     nsolutes{};      // number of input solutes for this task
      int     noisflag{};      // noise flag (0-3 for NONE|TI|RI|BOTH)
      int     dbg_level;     // debug flag

      bool    abort;         // should this thread be aborted?

      US_DataIO::EditedData*  edata{};       // experiment data (pointer)
      US_DataIO::RawData      sdata;       // simulation data
      US_DataIO::RawData      rdata;       // residuals
      US_Model                model;       // output model
      US_Noise                ri_noise;    // computed radially-invariant noise
      US_Noise                ti_noise;    // computed time-invariant noise
      US_Noise                ra_noise;    // computed random noise
      US_SimulationParameters simparms;    // simulation parameters

      QList< US_SolveSim::DataSet* > dsets;     // list of data set obj. ptrs.
      US_SolveSim::Simulation        sim_vals;  // simulation values
      US_SolveSim*                   solvesim;  // object for calc_residuals()
      US_SolveSim::DataSet           dset_wk;   // local copy of data set

      QVector< US_Solute >    solutes_i;   // solutes input
      QVector< US_Solute >    solutes_c;   // solutes computed
      US_Math_BF::Band_Forming_Gradient* bandFormingGradient{};
      US_LammAstfvm::CosedData* cosedData{};
};

#endif

