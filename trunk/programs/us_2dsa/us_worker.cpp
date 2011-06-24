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


// construct worker thread
WorkerThread::WorkerThread( QObject* parent )
   : QThread( parent )
{
   dbg_level  = US_Settings::us_debug();
   abort      = false;
//DbgLv(1) << "2P(WT): Thread created";
}

// worker thread destructor
WorkerThread::~WorkerThread()
{
   wait();
//DbgLv(1) << "2P(WT):   Thread destroyed";
}

// define work for a worker thread
void WorkerThread::define_work( WorkPacket& workin )
{
   llim_s      = workin.ll_s;
   llim_k      = workin.ll_k;
   thrn        = workin.thrn;
   taskx       = workin.taskx;
   depth       = workin.depth;
   iter        = workin.iter;
   menmcx      = workin.menmcx;
   noisflag    = workin.noisf;

   solutes_i   = workin.isolutes;

   dsets << workin.dsets[ 0 ];
   sim_vals    = workin.sim_vals;
}

// get results of a completed worker thread
void WorkerThread::get_result( WorkPacket& workout )
{
   workout.ll_s     = llim_s;
   workout.ll_k     = llim_k;
   workout.thrn     = thrn;
   workout.taskx    = taskx;
   workout.depth    = depth;
   workout.iter     = iter;
   workout.menmcx   = menmcx;
   workout.noisf    = noisflag;

   workout.csolutes = solutes_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
   workout.sim_vals = sim_vals;
int nn=workout.csolutes.size();
DbgLv(1) << "2P(WT): thr nn" << thrn << nn << "out sol0 soln"
 << workout.csolutes[0].c << workout.csolutes[nn-1].c;
}

// run the worker thread
void WorkerThread::run()
{
DbgLv(1) << "THR RUN: taskx thrn" << taskx << thrn;

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
   solvesim         = new US_SolveSim( dsets, thrn, true );

   connect( solvesim, SIGNAL(  work_progress( int ) ),
            this,     SLOT( forward_progress( int ) ) );

   sim_vals.solutes.clear();
   sim_vals.solutes << solutes_i;

   sim_vals.noisflag   = noisflag;
   sim_vals.dbg_level  = dbg_level;
   sim_vals.dbg_timing = US_Settings::debug_match( "2dsaTiming" );

   solvesim->calc_residuals( 0, 1, sim_vals );

   solutes_c      .clear();
   ti_noise.values.clear();
   ri_noise.values.clear();

   solutes_c       << sim_vals.solutes;
   ti_noise.values << sim_vals.ti_noise;
   ri_noise.values << sim_vals.ri_noise;
   return;
}

// Slot to forward a progress signal
void WorkerThread::forward_progress( int steps )
{
   emit work_progress( steps );
}

