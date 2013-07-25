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
   depth      = 0;
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
   depth       = workin.depth;
   psv_nnls_a  = workin.psv_nnls_a;
   psv_nnls_b  = workin.psv_nnls_b;

   solutes_i   = workin.isolutes;

QString phdr = QString( "2P(WT)dw:%1:%2:" ).arg(taskx).arg(thrn);
if(depth>0) {
DbgLv(1) << phdr << "depth1 psv_nnlsab" << psv_nnls_a << psv_nnls_b; }
else { 
DbgLv(1) << phdr << "DefWk: sols size" << solutes_i.size(); }
   dset_wk              = *(workin.dsets[ 0 ]);  // local copy of data set
   dset_wk.noise_files  = workin.dsets[ 0 ]->noise_files;
   dset_wk.run_data     = workin.dsets[ 0 ]->run_data;
   dset_wk.model        = workin.dsets[ 0 ]->model;
   dset_wk.simparams    = workin.dsets[ 0 ]->simparams;
   dset_wk.solution_rec = workin.dsets[ 0 ]->solution_rec;
   dsets.clear();
   dsets << &dset_wk;                        // save its pointer

   sim_vals             = workin.sim_vals;
   sim_vals.variance    = workin.sim_vals.variance;
   sim_vals.ti_noise    = workin.sim_vals.ti_noise;
   sim_vals.ri_noise    = workin.sim_vals.ti_noise;
   sim_vals.solutes     = solutes_i;
}

// get results of a completed worker thread
void WorkerThread::get_result( WorkPacket& workout )
{
DbgLv(1) << "2P(WT): get_result IN";
   workout.str_k    = str_k;
   workout.end_k    = end_k;
   workout.par1     = par1;
   workout.par2     = par2;
   workout.thrn     = thrn;
   workout.taskx    = taskx;
   workout.noisf    = noisflag;
   workout.depth    = depth;

   workout.isolutes = solutes_i;
   workout.csolutes = solutes_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
   workout.sim_vals = sim_vals;
   workout.dsets    = dsets;
//*DEBUG*
int nn=workout.csolutes.size();
int kk=nn/2;
int ni=solutes_i.size();
if(depth==0) {
DbgLv(1) << "2P(WT): thr nn" << thrn << nn << "out sol0 solk soln"
 << workout.csolutes[0].c << workout.csolutes[kk].c << workout.csolutes[nn-1].c
 << "ni sol0 soln s" << ni << solutes_i[0].s*1.e13 << solutes_i[ni-1].s*1.e13
 << "c" << solutes_i[0].c << solutes_i[ni-1].c; }
else {
DbgLv(1) << "2P(WT): thr nn" << thrn << nn 
 << "ni sol0 soln s" << ni << solutes_i[0].s*1.e13 << solutes_i[ni-1].s*1.e13
 << "c" << solutes_i[0].c << solutes_i[ni-1].c; }
//*DEBUG*
}

// run the worker thread
void WorkerThread::run()
{
QString phdr = QString( "WT:RUN:%1:%2:" ).arg(taskx).arg(thrn);
   calc_residuals();              // do all the work here

DbgLv(1) << phdr << " c_r return";
   quit();
   exec();

DbgLv(1) << phdr << "   sig w_c";
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
QString phdr = QString( "2P(WT):CR:%1:%2:" ).arg(taskx).arg(thrn);
DbgLv(1) << phdr << "depth" << depth;

   if ( depth == 0 )
   {  // Fit task:  do full compute of model
      solvesim            = new US_SolveSim( dsets, thrn, true );

      sim_vals.solutes    = solutes_i;
      sim_vals.noisflag   = noisflag;
//      sim_vals.dbg_level  = dbg_level;
      sim_vals.dbg_timing = US_Settings::debug_match( "pcsaTiming" );

      solvesim->calc_residuals( 0, 1, sim_vals );

      solutes_c           = sim_vals.solutes;
      ti_noise.values     = sim_vals.ti_noise;
      ri_noise.values     = sim_vals.ri_noise;
   }

   else
   {  // Alpha scan task:  apply alpha using saved A,B matrices
      int    nscans       = dsets[ 0 ]->run_data.scanCount();
      int    npoints      = dsets[ 0 ]->run_data.pointCount();
      int    nisols       = solutes_i.size();
      double variance     = 0.0;
      double xnormsq      = 0.0;
      double alpha        = sim_vals.alpha;

DbgLv(1) << phdr << "   call apply_alpha" << alpha;
      apply_alpha( alpha, psv_nnls_a, psv_nnls_b,
            nscans, npoints, nisols, variance, xnormsq );
DbgLv(1) << phdr << "     get apply_alpha: vari xnsq" << variance << xnormsq;
qApp->processEvents();

      sim_vals.variance   = variance;
      sim_vals.xnormsq    = xnormsq;
DbgLv(1) << phdr << "      sv.xnormsq" << sim_vals.xnormsq;
   }

   return;
}

// Slot to forward a progress signal
void WorkerThread::forward_progress( int steps )
{
   emit work_progress( steps );
   qApp->processEvents();
}

void WorkerThread::apply_alpha( const double alpha,
      QVector< double >* psv_nnls_a, QVector< double >* psv_nnls_b,
      const int nscans, const int npoints, const int nisols,
      double& variance, double& xnormsq )
{
QString phdr = QString( "wAA:%1:%2:" ).arg(taskx).arg(thrn);
   int    ntotal   = nscans * npoints;
   int    narows   = ntotal + nisols;
          variance = 0.0;
          xnormsq  = 0.0;
   int    ncsols   = 0;
   QVector< double > nnls_a = *psv_nnls_a;   // Local copy of A matrix
   QVector< double > nnls_b = *psv_nnls_b;   // Local copy of B matrix
   QVector< double > nnls_x;
   QVector< double > simdat;
   nnls_x  .fill( 0.0, nisols );
   simdat  .fill( 0.0, ntotal );
qDebug() << phdr << " ns np ni na" << nscans << npoints << nisols << narows;
#if 0
   double alphad   = 0.0;

   // Determine scaling factor for alpha
   for ( int rr = 0; rr < npoints; rr++ )
      alphad          = qMax( alphad, (*psv_nnls_b)[ rr ] );

   alphad          = ( alphad == 0.0 ) ? alpha : ( sqrt( alphad ) * alpha );
#endif

   // Replace alpha in the diagonal of the lower square of A
   int    dx       = ntotal;
   int    dinc     = ntotal + nisols + 1;
//qDebug() << phdr << " alf alfd" << alpha << alphad << "dx dinc" << dx << dinc;
qDebug() << phdr << " alpha" << alpha << "dx dinc" << dx << dinc;

   for ( int cc = 0; cc < nisols; cc++ )
   {
//      nnls_a[ dx ]    = alphad;
      nnls_a[ dx ]    = alpha;
      dx             += dinc;
   }

   // Compute the X vector using NNLS
   US_Math2::nnls( nnls_a.data(), narows, narows, nisols,
                   nnls_b.data(), nnls_x.data() );

   // Construct the output solutes and the implied simulation and xnorm-sq
   for ( int cc = 0; cc < nisols; cc++ )
   {
      double soluval  = nnls_x[ cc ];   // Computed concentration, this solute

      if ( soluval > 0.0 )
      {
         xnormsq        += sq( soluval );
         int    aa       = cc * narows;

         for ( int kk = 0; kk < ntotal; kk++ )
         {
            simdat[ kk ]   += ( soluval * (*psv_nnls_a)[ aa++ ] );
         }

         ncsols++;
      }
   }

   // Calculate the sum for the variance computation
   for ( int kk = 0; kk < ntotal; kk++ )
   {
      variance       += sq( ( (*psv_nnls_b)[ kk ] - simdat[ kk ] ) );
   }
qDebug() << phdr << "    ntot ncsols" << ntotal << ncsols
 << "varisum" << variance;

   // Return computed variance and xnorm-sq
   variance         /= (double)ntotal;
qDebug() << phdr << " alpha" << alpha << "vari xnsq" << variance << xnormsq;
int mm = npoints / 2;
qDebug() << phdr << " mm=" << mm << "a[m] b[m] s[m]"
 << (*psv_nnls_a)[mm] << (*psv_nnls_b)[mm] << simdat[mm];
qApp->processEvents();
}

