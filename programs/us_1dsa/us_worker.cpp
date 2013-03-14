//! \file us_worker.cpp
#include "us_worker.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_memory.h"


// construct worker thread
WorkerThread::WorkerThread( QObject* parent )
   : QThread( parent )
{
   dbg_level  = US_Settings::us_debug();
   abort      = false;
   solvesim   = NULL;
   thrn       = -1;
DbgLv(1) << "2P(WT): Thread created";
}

// worker thread destructor
WorkerThread::~WorkerThread()
{
   //if ( solvesim != NULL )
   //   delete solvesim;

DbgLv(1) << "2P(WT):   Thread destroy - (1)finished?" << isFinished() << thrn;
   if ( ! wait( 2000 ) )
   {
      qDebug() << "Thread destroy wait timeout(2secs) : Thread" << thrn;
   }
DbgLv(1) << "2P(WT):   Thread destroy - (2)finished?" << isFinished() << thrn;
DbgLv(1) << "2P(WT):    Thread destroyed" << thrn;
}

// define work for a worker thread
void WorkerThread::define_work( WorkPacket& workin )
{

   str_k       = workin.str_k;
   end_k       = workin.end_k;
   par1        = workin.par1;
   par2        = workin.par2;
   thrn        = workin.thrn;
   taskx       = workin.taskx;
   noisflag    = workin.noisf;
   typeref     = workin.typeref;

   solutes_i   = workin.isolutes;

   dset_wk              = *(workin.dsets[ 0 ]);  // local copy of data set
   dset_wk.noise_files  = workin.dsets[ 0 ]->noise_files;
   dset_wk.run_data     = workin.dsets[ 0 ]->run_data;
   dset_wk.model        = workin.dsets[ 0 ]->model;
   dset_wk.simparams    = workin.dsets[ 0 ]->simparams;
   dset_wk.solution_rec = workin.dsets[ 0 ]->solution_rec;
   dsets.clear();
   dsets << &dset_wk;                        // save its pointer

   sim_vals             = workin.sim_vals;
   sim_vals.variances   = workin.sim_vals.variances;
   sim_vals.ti_noise    = workin.sim_vals.ti_noise;
   sim_vals.ri_noise    = workin.sim_vals.ti_noise;
   sim_vals.solutes     = workin.sim_vals.solutes;
}

// get results of a completed worker thread
void WorkerThread::get_result( WorkPacket& workout )
{
   workout.str_k    = str_k;
   workout.end_k    = end_k;
   workout.par1     = par1;
   workout.par2     = par2;
   workout.thrn     = thrn;
   workout.taskx    = taskx;
   workout.noisf    = noisflag;

   workout.isolutes = solutes_i;
   workout.csolutes = solutes_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
   workout.sim_vals = sim_vals;
int nn=workout.csolutes.size();
int kk=nn/2;
int ni=solutes_i.size();
DbgLv(1) << "2P(WT): thr nn" << thrn << nn << "out sol0 solk soln"
 << workout.csolutes[0].c << workout.csolutes[kk].c << workout.csolutes[nn-1].c
 << "in sol0 soln" << ni << solutes_i[0].s*1.e13 << solutes_i[ni-1].s*1.e13
 << solutes_i[0].c << solutes_i[ni-1].c;
}

// run the worker thread
void WorkerThread::run()
{

   calc_residuals();              // do all the work here

   quit();
   exec();

   emit work_complete( this );    // signal that a thread's work is done
}

// set a flag so that a worker thread will abort as soon as possible
void WorkerThread::flag_abort()
{
   solvesim->abort_work();
}

// Do the real work of a thread:  solution from solutes set
void WorkerThread::calc_residuals()
{

   solvesim            = new US_SolveSim( dsets, thrn, true );

//   connect( solvesim, SIGNAL(  work_progress( int ) ),
//            this,     SLOT( forward_progress( int ) ) );

   sim_vals.solutes    = solutes_i;

   sim_vals.noisflag   = noisflag;
   sim_vals.dbg_level  = dbg_level;
   sim_vals.dbg_timing = US_Settings::debug_match( "1dsaTiming" );

   solvesim->calc_residuals( 0, 1, sim_vals );

   solutes_c           = sim_vals.solutes;
   ti_noise.values     = sim_vals.ti_noise;
   ri_noise.values     = sim_vals.ri_noise;

   return;
}

// Slot to forward a progress signal
void WorkerThread::forward_progress( int steps )
{
   emit work_progress( steps );
}

