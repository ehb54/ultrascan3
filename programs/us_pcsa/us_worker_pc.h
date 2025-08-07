//! \file us_worker_pc.h
#ifndef US_THREAD_WORKER_H
#define US_THREAD_WORKER_H

#include <QtCore>

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_simparms.h"
#include "us_solve_sim.h"
#include "us_zsolute.h"

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

//! \brief Worker thread task packet
typedef struct work_packet_pc_s {
  int thrn;   //!< thread number (1,...)
  int taskx;  //!< task index (0,...)
  int depth;  //!< depth (0->fit, 1->alpha-scan)
  int state;  //!< state flag (0-3 for READY,RUNNING,COMPLETE,ABORTED)
  int noisf;  //!< noise flag (0-3 for NONE,TI,RI,BOTH)

  double str_y;  //!< model start y value
  double end_y;  //!< model end   y value
  double par1;   //!< sigmoid model par1 value
  double par2;   //!< sigmoid model par2 value
  double par3;   //!< power law model par3 value

  QVector<US_ZSolute> isolutes;  //!< input solutes
  QVector<US_ZSolute> csolutes;  //!< computed solutes
  QVector<double> ti_noise;      //!< computed ti noise
  QVector<double> ri_noise;      //!< computed ri noise
  QVector<double>* psv_nnls_a;   //!< pointer to nnls A matrix
  QVector<double>* psv_nnls_b;   //!< pointer to nnls B matrix

  QList<US_SolveSim::DataSet*> dsets;  //!< list of data set object pointers
  US_SolveSim::Simulation sim_vals;    //!< simulation values

} WorkPacketPc;

//! \brief Worker thread to do the actual computational work of PCSA analysis

//! \class WorkerThreadPc
//! This class is for each of the individual worker threads that do the
//! actual computational work of PCSA analysis.
class WorkerThreadPc : public QThread {
  Q_OBJECT

 public:
  WorkerThreadPc(QObject* parent = 0);
  ~WorkerThreadPc();

 public slots:
  //! \brief Define the work packet for a worker thread
  //! \param workin   Input work definition packet
  void define_work(WorkPacketPc&);
  //! \brief Get the results work packet of a completed worker thread
  //! \param workout  Output work definition packet
  void get_result(WorkPacketPc&);
  //! \brief Run the worker thread
  void run();
  //! \brief Set a flag so a worker thread will abort as soon as possible
  void flag_abort();
  //! \brief Public slot to forward a progress signal
  void forward_progress(int);

 signals:
  void work_progress(int);
  void work_complete(WorkerThreadPc*);

 private:
  void calc_residuals(void);
  void calc_resids_ratio(void);
  long int max_rss(void);
  void apply_alpha(const double, QVector<double>*, QVector<double>*, const int,
                   const int, const int, double&, double&);

  QMutex mutex;  // mutex

  long int maxrss;  // maximum rss memory used

  double str_y;  // start model y value
  double end_y;  // end   model y value
  double par1;   // task par1 value
  double par2;   // task par2 value
  double par3;   // task par3 value

  int thrn;       // thread number (1,...)
  int taskx;      // grid refinement task index
  int depth;      // depth index
  int iter;       // iteration index
  int menmcx;     // meniscus / monte carlo index
  int typeref;    // type of refinement (0=UGRID, ...)
  int nscans;     // number of scans in experiment
  int npoints;    // number of radius points in experiment
  int nsolutes;   // number of input solutes for this task
  int noisflag;   // noise flag (0-3 for NONE|TI|RI|BOTH)
  int dbg_level;  // debug flag

  bool abort;  // should this thread be aborted?

  US_DataIO::EditedData* edata;      // experiment data (pointer)
  US_DataIO::RawData sdata;          // simulation data
  US_DataIO::RawData rdata;          // residuals
  US_Model model;                    // output model
  US_Noise ri_noise;                 // computed radially-invariant noise
  US_Noise ti_noise;                 // computed time-invariant noise
  US_Noise ra_noise;                 // computed random noise
  US_SimulationParameters simparms;  // simulation parameters

  QList<US_SolveSim::DataSet*> dsets;  // list of data set obj. ptrs.
  US_SolveSim::Simulation sim_vals;    // simulation values
  US_SolveSim* solvesim;               // object for calc_residuals()
  US_SolveSim::DataSet dset_wk;        // local copy of data set

  QVector<US_ZSolute> solutes_i;  // solutes input
  QVector<US_ZSolute> solutes_c;  // solutes computed

  QVector<double>* psv_nnls_a;  // pointer to saved nnls A matrix
  QVector<double>* psv_nnls_b;  // pointer to saved nnls B matrix
};

#endif
