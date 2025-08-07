//! \file us_worker_calcnorm.h
#ifndef US_THREAD_WORKCN_H
#define US_THREAD_WORKCN_H

#include <QtCore>

#include "us_astfem_rsa.h"
#include "us_dataIO.h"
#include "us_extern.h"
#include "us_math2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_simparms.h"
#include "us_solute.h"
#include "us_solve_sim.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

//! \brief Worker thread task packet
typedef struct work_packet_cn_s {
      int thrn; //!< thread number (1,...)
      int nthrd; //!< total threads count
      int amask; //!< attributes mask
      int nsolutes; //!< number of total solutes for all threads
      int nwsols; //!< number of solutes handled by this worker

      double cff0; //!< constant f/f0 (or zero)

      QVector<US_Solute> isolutes; //!< input solutes
      QVector<US_Solute> csolutes; //!< computed solutes

      QList<int> solxs; //!< solute indexes list

      US_SolveSim::DataSet *dset; //!< data set object pointer
} WorkPacketCN;

//! \brief Worker thread to do actual work of 2DSA analysis

//! \class WorkerThreadCalcNorm
//! This class is for each of the individual worker threads that do the
//! actual computational work of calculating norm values
class WorkerThreadCalcNorm : public QThread {
      Q_OBJECT

   public:
      WorkerThreadCalcNorm(QObject *parent = 0);
      ~WorkerThreadCalcNorm();

      enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_F };

   public slots:
      //! \brief Define the work packet for a worker thread
      //! \param workin   Input work definition packet
      void define_work(WorkPacketCN &);
      //! \brief Get the results work packet of a completed worker thread
      //! \param workout  Output work definition packet
      void get_result(WorkPacketCN &);
      //! \brief Run the worker thread
      void run();

   signals:
      void work_progress(int);
      void work_complete(WorkerThreadCalcNorm *);

   private:
      void calc_norms(void);
      void set_comp_attr(US_Model::SimulationComponent &, US_Solute &, int);

      int thrn; // thread number (1,...)
      int nthrd; // number of total threads
      int nsolutes; // number of total input solutes
      int nwsols; // number of solutes for this worker thread
      int amask; // attribute mask (xyz combination)
      int attr_x; // x attribute flag
      int attr_y; // y attribute flag
      int attr_z; // z attribute flag
      int dbg_level; // debug flag

      double cff0; // constant f/f0 (or zero)

      US_DataIO::EditedData *edata; // experiment data (pointer)
      US_Model model1; // output model
      US_SimulationParameters simparms; // simulation parameters

      US_SolveSim::DataSet *dset; // data set object pointer

      QList<int> solxs; // list of solute indexes for thread

      QVector<US_Solute> solutes_i; // solutes input
      QVector<US_Solute> solutes_c; // solutes computed
};

#endif
