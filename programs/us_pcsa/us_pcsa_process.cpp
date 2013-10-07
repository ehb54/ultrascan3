//! \file us_pcsa_process.cpp
#include <QApplication>
#include <QtCore>
#include <float.h>

#include "us_pcsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_lm.h"
#include "us_solve_sim.h"
#include "us_constants.h"
#include "us_memory.h"

// Class to process PCSA simulations
US_pcsaProcess::US_pcsaProcess( QList< US_SolveSim::DataSet* >& dsets,
   QObject* parent /*=0*/ ) : QObject( parent ), dsets( dsets )
{
   edata            = &dsets[ 0 ]->run_data;  // pointer to  experiment data
   dbg_level        = US_Settings::us_debug();
   maxrss           = 0;              // max memory

   mrecs    .clear();                 // computed model records
   simparms = &dsets[ 0 ]->simparams; // pointer to simulation parameters

   nscans           = edata->scanCount();
   npoints          = edata->pointCount();
   cresolu          = 100;
   curvtype         = 0;
   nmtasks          = 0;
   kctask           = 0;
   kstask           = 0;
   varimin          = 9.e+9;
   minvarx          = 99999;

   parlims[ 0 ]     = -1.0;
}

// Get maximum used memory
long int US_pcsaProcess::max_rss( void )
{
   return US_Memory::rss_max( maxrss );
}

// Start a specified PCSA fit run
void US_pcsaProcess::start_fit( double sll, double sul, double kll, double kul,
                                int    nkp, int    res, int    typ, int    nth,
                                int    noi, int  lmmxc, int  gfits,
                                double gfthr, double alf )
{
DbgLv(1) << "PC(pcsaProc): start_fit()";
   abort       = false;
   slolim      = sll;
   suplim      = sul;
   klolim      = kll;
   kuplim      = kul;
   nkpts       = nkp;
   cresolu     = res;
   curvtype    = typ;
   nthreads    = nth;
   noisflag    = noi;
   alpha_scn   = false;
   alpha       = alf;
   alpha_fx    = 0.0;
   alpha_lm    = 0.0;
   fi_itermax  = gfits;
   rd_thresh   = gfthr;
   lmmxcall    = lmmxc;
   parlims[ 0 ] = -1.0;

   if ( alpha < 0.0 )
   {  // Negative alpha acts as flag
      if ( alpha == (-99.0) )
      {  // This is an alpha scan run
         alpha_scn   = true;
         alpha       = 0.0;
      }

      else
      {  // This is a run with regularize in earlier stage(s)
         alpha       = -alpha;
         alpha_lm    = alpha;       // In L-M stage for sure

         if ( alpha > 1.0 )
         {
            alpha      -= 1.0;
            alpha_fx    = alpha;   // Also in Fixed stage
            alpha_lm    = alpha;
         }
      }
   }

   pfi_rmsd    = 1.0e+99;
   cfi_rmsd    = pfi_rmsd;
   fi_iter     = 1;

DbgLv(1) << "PC: alf alpha lm fx scn"
   << alf << alpha << alpha_lm << alpha_fx << alpha_scn;
   errMsg      = tr( "NO ERROR: start" );
   maxrss      = 0;
   varimin     = 9.e+9;
   minvarx     = 99999;

   wkstates .resize(  nthreads );
   wthreads .clear();
   job_queue.clear();

   orig_sols.clear();
   mrecs    .clear();

DbgLv(1) << "PC: sll sul" << slolim << suplim
 << " kll kul nkp" << klolim << kuplim << nkpts
 << " type reso nth noi" << curvtype << cresolu << nthreads << noisflag;

   timer.start();              // start a timer to measure run time

   // experiment data dimensions
   nscans      = edata->scanCount();
   npoints     = edata->pointCount();

   if ( curvtype == 0  ||  curvtype == 3 )
   { // Determine models for straight-line curves
      nmtasks     = slmodels( curvtype, slolim, suplim, klolim, kuplim, nkpts,
                              cresolu );
   }

   else if ( curvtype == 1  ||  curvtype == 2 )
   { // Determine models for sigmoid curves
      nmtasks     = sigmodels( curvtype, slolim, suplim, klolim, kuplim, nkpts,
                               cresolu );
   }

   else
      nmtasks     = 0;

   kcsteps     = 0;
   nctotal     = nmtasks;
   emit stage_complete( kcsteps, nctotal );

   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nmtasks ) ? nthreads : nmtasks;
DbgLv(1) << "PC:   nscans npoints" << nscans << npoints;
DbgLv(1) << "PC:   nctotal nthreads" << nctotal << nthreads;
   max_rss();
DbgLv(1) << "PC: (1)maxrss" << maxrss;

   // Queue all the tasks
   for ( int ktask = 0; ktask < nmtasks; ktask++ )
   {
      WorkPacket wtask;
      int    mm   = orig_sols[ ktask ].size() - 1;
      double strk = orig_sols[ ktask ][ 0  ].k;
      double endk = orig_sols[ ktask ][ mm ].k;
      wtask.par1  = mrecs[ ktask ].par1;
      wtask.par2  = mrecs[ ktask ].par2;
      wtask.depth = 0;

      wtask.sim_vals.alpha = alpha_fx;

      queue_task( wtask, strk, endk, ktask, noisflag, orig_sols[ ktask ] );
   }

   // Start the first threads. This will begin the first work units (subgrids).
   // Thereafter, work units are started in new threads when previous
   // threads signal that they have completed their work.

   for ( int ii = 0; ii < nthreads; ii++ )
   {
      wthreads << 0;

      WorkPacket wtask = job_queue.takeFirst();
      submit_job( wtask, ii );
   }

   mrecs    .clear();
   max_rss();
   kstask = nthreads;     // count of started tasks is initially thread count
DbgLv(1) << "PC:   kstask nthreads" << kstask << nthreads << job_queue.size();

   emit message_update( pmessage_head() +
      tr( "Starting computations of %1 models\n using %2 threads ..." )
      .arg( nmtasks ).arg( nthreads ), false );
}

// Abort a fit run
void US_pcsaProcess::stop_fit()
{
   abort   = true;

   for ( int ii = 0; ii < nthreads; ii++ )
   {
DbgLv(1) << "StopFit test Thread" << ii + 1;
      WorkerThread* wthr = wthreads[ ii ];

      if ( wthr != 0  &&  wthr->isRunning() )
      {
         wthr->flag_abort();
DbgLv(1) << "  STOPTHR:  thread aborted";
      }

      else if ( wthr != 0  &&  ! wthr->isFinished() )
      {
         delete wthr;
DbgLv(1) << "  STOPTHR:  thread deleted";
      }

      wthreads[ ii ] = 0;
   }

   job_queue.clear();

   emit message_update( pmessage_head() +
      tr( "All computations have been aborted." ), false );
}

// Complete a specified PCSA fit run after alpha scan or alpha change
void US_pcsaProcess::final_fit( double alf )
{
DbgLv(1) << "PC(pcsaProc): final_fit()";
   abort       = false;
   alpha       = alf;

   compute_final();

   emit process_complete( 9 );     // signal that all processing is complete
}

// Slot to handle the low-variance record of fixed final calculated solutes
void US_pcsaProcess::process_fxfinal( ModelRecord& mrec )
{
   if ( abort ) return;

   if ( ( noisflag & 1 ) != 0 )
   {  // copy TI noise to caller and internal vector
      ti_noise.minradius = edata->radius( 0 );
      ti_noise.maxradius = edata->radius( npoints - 1 );
      ti_noise.values.resize( npoints );
      ti_noise.count = npoints;

      for ( int rr = 0; rr < npoints; rr++ )
      {
         ti_noise.values[ rr ] = mrec.ti_noise[ rr ];
      }
   }

   if ( ( noisflag & 2 ) != 0 )
   {  // copy RI noise to caller and internal vector
      ri_noise.values.resize( nscans );
      ri_noise.count = nscans;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         ri_noise.values[ ss ] = mrec.ri_noise[ ss ];
      }
   }
DbgLv(1) << "FIN_FIN:    ti,ri counts" << ti_noise.count << ri_noise.count;

   US_SolveSim::DataSet* dset = dsets[ 0 ];
   int    nsolutes    = mrec.csolutes.size();
   double vbar20      = dset->vbar20;
   model.components.resize( nsolutes );
   qSort( mrec.csolutes );

   // build the final model
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      // Get standard-space solute values (20,W)
      US_Model::SimulationComponent mcomp;
      mcomp.vbar20 = vbar20;
      mcomp.s      = mrec.csolutes[ cc ].s;
      mcomp.D      = 0.0;
      mcomp.mw     = 0.0;
      mcomp.f      = 0.0;
      mcomp.f_f0   = mrec.csolutes[ cc ].k;
      mcomp.signal_concentration
                   = mrec.csolutes[ cc ].c;

      // Complete other coefficients in standard-space
      model.calc_coefficients( mcomp );
DbgLv(1) << " Bcc comp D" << mcomp.D;

      model.components[ cc ]  = mcomp;
   }

DbgLv(1) << "FIN_FIN:    c0 cn" << mrec.csolutes[0].c
 << mrec.csolutes[qMax(0,nsolutes-1)].c << "  nsols" << nsolutes;
   nscans           = edata->scanCount();
   npoints          = edata->pointCount();
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_DataIO::RawData* simdat = &mrec.sim_data;
   US_DataIO::RawData* resids = &mrec.residuals;
DbgLv(1) << "FIN_FIN: nscans npoints" << nscans << npoints;
DbgLv(1) << "FIN_FIN: simdat nsc npt"
 << simdat->scanCount() << simdat->pointCount();
DbgLv(1) << "FIN_FIN: resids nsc npt"
 << resids->scanCount() << resids->pointCount();
DbgLv(1) << "FIN_FIN: rdata  nsc npt"
 << rdata.scanCount() << rdata.pointCount();
DbgLv(1) << "FIN_FIN: sdata  nsc npt"
 << sdata.scanCount() << sdata.pointCount();

   // build residuals data set (experiment minus simulation minus any noise)
   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         sdata.setValue( ss, rr, simdat->value( ss, rr ) );
         rdata.setValue( ss, rr, resids->value( ss, rr ) );
      }
   }

int mms=nscans/2;
int mmr=npoints/2;
DbgLv(1) << "FIN_FIN: edatm sdatm rdatm" << edata->value(mms,mmr)
 << sdata.value(mms,mmr) << rdata.value(mms,mmr);

DbgLv(1) << "FIN_FIN: vari" << mrec.variance << "bmndx" << mrec.taskx;

   // determine elapsed time
   time_fg    = timer.elapsed();
   int ktimes = ( time_fg + 500 ) / 1000;
   int ktimeh = ktimes / 3600;
   int ktimem = ( ktimes - ktimeh * 3600 ) / 60;
double bvol = dsets[0]->simparams.band_volume;
bvol=dsets[0]->simparams.band_forming?bvol:0.0;
DbgLv(1) << "done: vari bvol" << mrec.variance << bvol
 << "slun klun" << slolim << suplim << klolim << kuplim << nkpts
 << "ets" << ktimes;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // compose final status message
   QString pmsg = tr( "The Solution has converged...\n"
                      "Threads:  %1 ;   Models:  %2 ;  Iterations:  %3 ."
                      "\nRun time:  " )
                  .arg( nthreads ).arg( nmtasks ).arg( fi_iter );

   if ( ktimeh > 0 )
      pmsg += tr( "%1 hr., %2 min., %3 sec.;" )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      pmsg += tr( "%1 min., %2 sec.;" )
         .arg( ktimem ).arg( ktimes );

   max_rss();
   int memmb  = qRound( (double)maxrss / 1024.0 );

   pmsg += tr( "   Maximum memory used:  " ) +
           QString::number( memmb ) + " MB\n\n" +
           tr( "The best model (RMSD=%1, %2 solutes, index %3) is:\n" )
           .arg( mrec.rmsd ).arg( nsolutes ).arg( mrec.taskx );
   if ( curvtype == 0 )
   {
      double slopel = (double)( mrec.end_k - mrec.str_k ) 
                    / (double)( suplim     - slolim     );
      pmsg += tr( "  the line from s,f/f0  %1, %2  to %3, %4  (slope %5)." )
              .arg( slolim ).arg( mrec.str_k ).arg( suplim ).arg( mrec.end_k )
              .arg( slopel );
   }
   else if ( curvtype == 1  ||  curvtype == 2 )
   {
      pmsg += tr( "  the curve with par1=%1 and par2=%2." )
              .arg( mrec.par1 ).arg( mrec.par2 );
DbgLv(1) << "FIN_FIN: par1,par2" << mrec.par1 << mrec.par2;
   }

   else if ( curvtype == 3 )
   {
      pmsg += tr( "  the line from s,f/f0  %1, %2  to %3, %4." )
              .arg( slolim ).arg( mrec.str_k ).arg( suplim ).arg( mrec.end_k );
   }
   emit message_update( pmsg, false );          // signal final message

DbgLv(1) << "FIN_FIN: Run Time: hr min sec" << ktimeh << ktimem << ktimes;
DbgLv(1) << "FIN_FIN: maxrss memmb nthr" << maxrss << memmb << nthreads
 << " nsubg noisf" << nmtasks << noisflag;
DbgLv(1) << "FIN_FIN:   kcsteps nctotal" << kcsteps << nctotal;
DbgLv(1) << "FIN_FIN:    alpha_fx" << alpha_fx;
   pfi_rmsd      = cfi_rmsd;
   cfi_rmsd      = mrecs[ 0 ].rmsd;
   rd_frac       = ( fi_iter > 1 ) ?
                   qAbs( ( pfi_rmsd - cfi_rmsd ) / pfi_rmsd ) :
                   rd_thresh * 2;
DbgLv(1) << "FIN_FIN:    rd_frac rd_thresh" << rd_frac << rd_thresh;

   if ( fi_iter < fi_itermax  &&  rd_frac > rd_thresh )
   {
      restart_fit();

      return;
   }

   emit process_complete( 8 );     // signal that L-M is starting

   // Compute a best model using Levenberg-Marquardt
   LevMarq_fit();

   if ( alpha_scn )
   {  // Signal analysis control that an alpha scan may begin
      emit process_complete( 7 );
      return;
   }

   if ( alpha_lm == 0.0  &&  alpha != 0.0 )
   {  // L-M nonregularized, but need to do a final regularized computation
      compute_final();
   }

   nsolutes           = model.components.size();

   emit process_complete( 9 );     // signal that all processing is complete
}

// Public slot to get results upon completion of all refinements
bool US_pcsaProcess::get_results( US_DataIO::RawData*     da_sim,
                                  US_DataIO::RawData*     da_res,
                                  US_Model*               da_mdl,
                                  US_Noise*               da_tin,
                                  US_Noise*               da_rin,
                                  int&                    bm_ndx,
                                  QStringList&            modstats,
                                  QVector< ModelRecord >& p_mrecs )
{
   bool all_ok      = true;
   model.alphaRP    = alpha;
   mrecs[ 0 ].model = model;

   if ( abort ) return false;

   *da_sim     = sdata;                           // copy simulation data
   *da_res     = rdata;                           // copy residuals data
   *da_mdl     = model;                           // copy model

   if ( ( noisflag & 1 ) != 0  &&  da_tin != 0 )
      *da_tin     = ti_noise;                     // copy any ti noise

   if ( ( noisflag & 2 ) != 0  &&  da_rin != 0 )
      *da_rin     = ri_noise;                     // copy any ri noise

   p_mrecs     = mrecs;                           // copy model records vector
   bm_ndx      = mrecs[ 0 ].taskx;
DbgLv(0) << " GET_RES:   ti,ri counts" << ti_noise.count << ri_noise.count;
DbgLv(0) << " GET_RES:    VARI,RMSD" << mrecs[0].variance << mrecs[0].rmsd
 << "BM_NDX" << bm_ndx << "ALPHA" << alpha;

   model_statistics( mrecs, modstats );

DbgLv(1) << "PC:GR:   RTN";
   return all_ok;
}

// Public slot to get best model record in preparation for an alpha scan
void US_pcsaProcess::get_mrec( ModelRecord& p_mrec )
{
   p_mrec      = mrecs[ 0 ];              // Copy best model record
}

// Replace the top-spot model record
void US_pcsaProcess::put_mrec( ModelRecord& a_mrec )
{
   mrecs[ 0 ]  = a_mrec;                  // Copy best model record

   model       = a_mrec.model;
   alpha       = 0.0;
   sdata       = a_mrec.sim_data;
   rdata       = a_mrec.residuals;
}

// Replace the model records list and related variables after file load
void US_pcsaProcess::put_mrecs( QVector< ModelRecord >& a_mrecs )
{
   mrecs       = a_mrecs;                 // Copy model records list
   int nmrecs  = mrecs.size();
DbgLv(1) << "PC:putMRs: nmrecs" << nmrecs;

   if ( nmrecs < 1 )
      return;

   model       = mrecs[ 0 ].model;
   sdata       = mrecs[ 0 ].sim_data;
   rdata       = mrecs[ 0 ].residuals;
   slolim      = mrecs[ 0 ].smin;
   suplim      = mrecs[ 0 ].smax;
   klolim      = mrecs[ 0 ].kmin;
   kuplim      = mrecs[ 0 ].kmax;
   curvtype    = mrecs[ 0 ].ctype;
   nmtasks     = ( curvtype != 3 ) ? sq( nkpts ) : nkpts;
   alpha       = 0.0;
DbgLv(1) << "PC:putMRs:  nkpts nmtasks" << nkpts << nmtasks;

   for ( int ii = 0; ii < nmrecs; ii++ )
   {
      varimin     = qMin( varimin, mrecs[ ii ].variance );
   }

DbgLv(1) << "PC:putMRs:  curvtype" << curvtype << "nkpts" << nkpts;
}

// Submit a job
void US_pcsaProcess::submit_job( WorkPacket& wtask, int thrx )
{
   wtask.thrn         = thrx + 1;

   WorkerThread* wthr = new WorkerThread( this );
   wthreads[ thrx ]   = wthr;
   wkstates[ thrx ]   = WORKING;

   wthr->define_work( wtask );

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT(   process_job(   WorkerThread* ) ) );
DbgLv(1) << "SUBMIT_JOB taskx" << wtask.taskx
 << "sk ek" << wtask.str_k << wtask.end_k;

   wthr->start();
}

// Slot to handle the results of a just-completed worker thread.
// Accumulate computed solutes.
// If there is more work to do, start a new thread for a new work unit.
void US_pcsaProcess::process_job( WorkerThread* wthrd )
{
   if ( abort )  return;
   WorkPacket wresult;

   wthrd->get_result( wresult );   // get results of thread task
   int thrn   = wresult.thrn;      // thread number of task
   int thrx   = thrn - 1;          // index into thread list
   int taskx  = wresult.taskx;     // task index of task
   int orecx  = mrecs.size();      // output record index 
DbgLv(1) << "PROCESS_JOB thrn" << thrn << "taskx orecx" << taskx << orecx;
//DBG-CONC
if (dbg_level>0) for (int mm=0; mm<wresult.csolutes.size(); mm++ ) {
 if ( wresult.csolutes[mm].c > 100.0 ) {
   DbgLv(1) << "PJ:  CONC=" << wresult.csolutes[mm].c
    << " s,ff0" << wresult.csolutes[mm].s*1.0e+13
    << wresult.csolutes[mm].k; } }
//DBG-CONC
   double variance = wresult.sim_vals.variance;

   // Save variance and model record for this task result
   ModelRecord mrec;
   mrec.taskx      = taskx;
   mrec.str_k      = wresult.str_k;
   mrec.end_k      = wresult.end_k;
   mrec.par1       = wresult.par1;
   mrec.par2       = wresult.par2;
   mrec.variance   = variance;
   mrec.rmsd       = ( variance > 0.0 ) ? sqrt( variance ) : 99.9;
   mrec.isolutes   = wresult.isolutes;
   mrec.csolutes   = wresult.csolutes;
   mrec.ctype      = curvtype;
   mrec.smin       = slolim;
   mrec.smax       = suplim;
   mrec.kmin       = klolim;
   mrec.kmax       = kuplim;

   if ( variance < varimin )
   { // Handle a new minimum variance record
      if ( minvarx < orecx )
      { // Clear vectors from the previous minimum
         mrecs[ minvarx ].clear_data();
DbgLv(1) << "PJ: CLEAR: VARI VMIN ORECX" << variance << varimin << orecx;
      }
      // Save information from the minimum-variance model record
      varimin         = variance;
      minvarx         = orecx;
      mrec.sim_data   = wresult.sim_vals.sim_data;
      mrec.residuals  = wresult.sim_vals.residuals;
      mrec.ti_noise   = wresult.ti_noise;
      mrec.ri_noise   = wresult.ri_noise;
DbgLv(1) << "PJ: MINVARX=" << minvarx;
   }
   else if ( variance > varimin )
   { // Clear vectors from a model record that is not minimum-variance
DbgLv(1) << "PJ:  CLEAR: VARI VMIN MVARX" << variance << varimin << minvarx;
      mrec.clear_data();
   }

   mrecs     << mrec;                 // Append to the vector of model records
DbgLv(1) << "THR_FIN:  taskx minvarx varimin" << taskx << minvarx << varimin;

   max_rss();

   free_worker( thrx );               // Free up this worker thread

   if ( abort )
      return;

   kctask++;                          // Bump count of completed subgrid tasks
   emit progress_update( variance );  // Pass progress on to control window
DbgLv(1) << "THR_FIN: thrn" << thrn << " taskx orecx" << taskx << orecx
 << " kct kst" << kctask << kstask;

   emit message_update( pmessage_head() +
      tr( "Of %1 models, computations are complete for %2." )
      .arg( nmtasks ).arg( kctask ), false );

   if ( kctask >= nmtasks )
   {  // All model tasks are now complete
      emit stage_complete( kcsteps, nctotal );

      emit message_update( pmessage_head() +
         tr( "All models have now been completed. Evaluating..." ), false );
   }

   if ( kctask < nmtasks )
   { // Not the last:  add to the queue if necessary
      // Submit jobs while queue is not empty and a worker thread is ready
      while ( ! job_queue.isEmpty() &&
              ( thrx = wkstates.indexOf( READY ) ) >= 0 )
      {
         WorkPacket wtask = next_job();

         submit_job( wtask, thrx );
         kstask++;                       // Bump count of started worker threads

      }
   }

   else
   { // We have done the last computation, so determine the low-rmsd result

      qSort( mrecs );                    // Sort model records by variance

DbgLv(1) << "THR_FIN: thrn" << thrn << " mrecs size" << mrecs.size()
 << "mrec0 taskx,vari" << mrecs[0].taskx << mrecs[0].variance;
      process_fxfinal( mrecs[ 0 ] );
//*DBG*
if (dbg_level>0) {
 for (int ii=0;ii<mrecs.size();ii++)
 {
   DbgLv(1) << "MREC:rank" << ii << "taskx" << mrecs[ii].taskx
    << "st_k en_k" << mrecs[ii].str_k << mrecs[ii].end_k
    << "vari,rmsd" << mrecs[ii].variance << mrecs[ii].rmsd
    << "ncsols" << mrecs[ii].csolutes.size();
 }
}
//*DBG*
   }
}

// Build a task and add it to the queue
void US_pcsaProcess::queue_task( WorkPacket& wtask, double strk, double endk,
      int taskx, int noisf, QVector< US_Solute > isolutes )
{
   wtask.thrn     = 0;             // thread number (none while queued)
   wtask.taskx    = taskx;         // task index
   wtask.noisf    = noisf;         // noise flag
   wtask.state    = READY;         // initialized state, ready for submit
   wtask.str_k    = strk;          // start k value
   wtask.end_k    = endk;          // end   k value
   wtask.dsets    = dsets;         // pointer to experiment data
   wtask.isolutes = isolutes;      // solutes for calc_residuals task

   wtask.csolutes.clear();         // clear output vectors
   wtask.ti_noise.clear();
   wtask.ri_noise.clear();

   job_queue << wtask;             // put the task on the queue
}

// Free up a worker thread
void US_pcsaProcess::free_worker( int tx )
{
   if ( tx >= 0  &&  tx < nthreads )
   {
      if ( wthreads[ tx ] != 0 )
         delete wthreads[ tx ];       // destroy thread

      wthreads[ tx ] = 0;             // set thread pointer to null
      wkstates[ tx ] = READY;         // mark its slot as available
   }
}

QString US_pcsaProcess::pmessage_head()
{
   const char* ctp[] = { "Straight Line",
                         "Increasing Sigmoid",
                         "Decreasing Sigmoid",
                         "Horizontal Line [ C(s) ]",
                         "?UNKNOWN?"
                       };
   QString ctype = QString( ctp[ curvtype ] );
   return tr( "Analysis of %1 %2 %3-solute models.\n"
              "Grid Fit Iteration %4.\n" )
          .arg( nmtasks ).arg( ctype ).arg( cresolu ).arg( fi_iter );
}

// Get next job from queue, insuring we get the lowest depth
WorkPacket US_pcsaProcess::next_job()
{
   WorkPacket wtask;
   if ( job_queue.size() == 0 )  return wtask;

   int jobx    = 0;
   wtask       = job_queue[ jobx ];

if(jobx>0) {
DbgLv(1) << "NEXTJ: jobx taskx taskx0"
 << jobx << wtask.taskx << job_queue[0].taskx; }
else {
DbgLv(1) << "NEXTJ: jobx taskx" << jobx << wtask.taskx; }
DbgLv(1) << "NEXTJ:   wtask" << &wtask << &job_queue[jobx];

   job_queue.removeAt( jobx );          // Remove job from queue
   return wtask;
}

// Build all the straight-line models
int US_pcsaProcess::slmodels( int ctp, double slo, double sup, double klo,
      double kup, int nkp, int res )
{
DbgLv(1) << "SLMO: slo sup klo kup nkp res" << slo << sup << klo << kup
   << nkp << res;
   int    nmodels  = 0;
   double vbar20   = dsets[ 0 ]->vbar20;
   orig_sols.clear();

   // Compute straight-line model records
   if ( ctp == 0 )
      nmodels = ModelRecord::compute_slines( slo, sup, klo, kup, nkp,
                                             res, parlims, mrecs );
   else if ( ctp == 3 )
      nmodels = ModelRecord::compute_hlines( slo, sup, klo, kup, nkp,
                                             res, parlims, mrecs );

   // Update the solutes with vbar and add to task solutes list
   for ( int ii = 0; ii < nmodels; ii++ )
   {
      QVector< US_Solute >* isols = &mrecs[ ii ].isolutes;

      for ( int jj = 0; jj < isols->size(); jj++ )
      {
         (*isols)[ jj ].v    = vbar20;
      }

      orig_sols << *isols;
   }
DbgLv(1) << "SLMO:  orig_sols size" << orig_sols.size() << "nmodels" << nmodels;

   return nmodels;
}

// Build all the sigmoid models
int US_pcsaProcess::sigmodels( int ctp, double slo, double sup, double klo,
      double kup, int nkp, int nlpts )
{
DbgLv(1) << "SGMO: ctp slo sup klo kup nkp nlp" << ctp << slo << sup
   << klo << kup << nkp << nlpts;
   double vbar20   = dsets[ 0 ]->vbar20;
   orig_sols.clear();

   // Compute sigmoid model records
   int nmodels = ModelRecord::compute_sigmoids( ctp, slo, sup, klo, kup,
                                                nkp, nlpts, parlims, mrecs );

   // Update the solutes with vbar and add to task solutes list
   for ( int ii = 0; ii < nmodels; ii++ )
   {
      QVector< US_Solute >* isols = &mrecs[ ii ].isolutes;

      for ( int jj = 0; jj < isols->size(); jj++ )
      {
         (*isols)[ jj ].v    = vbar20;
      }

      orig_sols << *isols;
   }
DbgLv(1) << "SGMO:  orig_sols size" << orig_sols.size() << "nmodels" << nmodels;

   return nmodels;
}

// Generate the strings of model statistics for a report
void US_pcsaProcess::model_statistics( QVector< ModelRecord >& mrecs,
                                       QStringList&            modstats )
{
   const char* ctp[] = { "Straight Line",
                         "Increasing Sigmoid",
                         "Decreasing Sigmoid",
                         "Horizontal Line [ C(s) ]",
                         "?UNKNOWN?"
                       };

   // Accumulate the statistics
   int    nbmods    = nmtasks / 10;
   int    nlpts     = cresolu;
   double rmsdmin   = 99999.0;
   double rmsdmax   = 0.0;
   double rmsdavg   = 0.0;
   double brmsmin   = 99999.0;
   double brmsmax   = 0.0;
   double brmsavg   = 0.0;
DbgLv(1) << "PC:MS: nmtasks mecssize" << nmtasks << mrecs.size();
   double rmsdmed   = mrecs[ nmtasks / 2 ].rmsd;
   double brmsmed   = mrecs[ nbmods  / 2 ].rmsd;
   int    nsolmin   = 999999;
   int    nsolmax   = 0;
   int    nsolavg   = 0;
   int    nbsomin   = 999999;
   int    nbsomax   = 0;
   int    nbsoavg   = 0;

   for ( int ii = 0; ii < nmtasks; ii++ )
   {
      double rmsd      = mrecs[ ii ].rmsd;
      int    nsols     = mrecs[ ii ].csolutes.size();
      rmsdmin          = qMin( rmsdmin, rmsd );
      rmsdmax          = qMax( rmsdmax, rmsd );
      rmsdavg         += rmsd;
      nsolmin          = qMin( nsolmin, nsols );
      nsolmax          = qMax( nsolmax, nsols );
      nsolavg         += nsols;

      if ( ii < nbmods )
      {
         brmsmin          = qMin( brmsmin, rmsd );
         brmsmax          = qMax( brmsmax, rmsd );
         brmsavg         += rmsd;
         nbsomin          = qMin( nbsomin, nsols );
         nbsomax          = qMax( nbsomax, nsols );
         nbsoavg         += nsols;
      }
   }

   rmsdavg         /= (double)nmtasks;
   nsolavg          = ( nsolavg + nmtasks / 2 ) / nmtasks;
   brmsavg         /= (double)nbmods;
   nbsoavg          = ( nbsoavg + nbmods  / 2 ) / nbmods;

   modstats.clear();

   modstats << tr( "Curve Type:" )
            << QString( ctp[ curvtype ] );
   modstats << tr( "s (x 1e13) Range:" )
            << QString().sprintf( "%10.4f  %10.4f", slolim, suplim );
   double str_k  = mrecs[ 0 ].str_k;
   double end_k  = mrecs[ 0 ].end_k;

   if ( curvtype == 0  ||  curvtype == 3 )
   {
      double slope  = ( end_k - str_k ) / ( suplim - slolim );
      double kincr  = ( kuplim - klolim ) / (double)( nkpts - 1 );
      modstats << tr( "k (f/f0) Range + delta:" )
               << QString().sprintf( "%10.4f  %10.4f  %10.4f",
                     klolim, kuplim, kincr );
      modstats << tr( "Best curve f/f0 end points + slope:" )
               << QString().sprintf( "%10.4f  %10.4f  %10.4f",
                     str_k, end_k, slope );
DbgLv(1) << "PC:MS:  best str_k,end_k" << str_k << end_k;
   }
   else
   {
      int    p1ndx  = mrecs[ 0 ].taskx / nkpts;
      int    p2ndx  = mrecs[ 0 ].taskx - ( p1ndx * nkpts );
      double krng   = (double)( nkpts - 1 );
      double p1off  = (double)p1ndx / krng;
      double par1   = exp( log( 0.001 )
                           + ( log( 0.5 ) - log( 0.001 ) ) * p1off );
      double par2   = (double)p2ndx / krng;
      modstats << tr( "k (f/f0) Range:" )
               << QString().sprintf( "%10.4f  %10.4f", klolim, kuplim );
      modstats << tr( "Best curve par1 and par2:" )
               << QString().sprintf( "%10.4f  %10.4f", par1, par2 );
      modstats << tr( "Best curve f/f0 end points:" )
               << QString().sprintf( "%10.4f  %10.4f",
                     mrecs[ 0 ].str_k, mrecs[ 0 ].end_k );
   }
   modstats << tr( "Number of models:" )
            << QString().sprintf( "%5d", nmtasks );
   modstats << tr( "Number of curve variations:" )
            << QString().sprintf( "%5d", nkpts );
   modstats << tr( "Solute points per curve:" )
            << QString().sprintf( "%5d", nlpts );
   modstats << tr( "Index of best model:" )
            << QString().sprintf( "%5d", mrecs[ 0 ].taskx );
   modstats << tr( "Best curve calculated solutes:" )
            << QString().sprintf( "%5d", mrecs[ 0 ].csolutes.size() );
   modstats << tr( "Minimum, Maximum calculated solutes:" )
            << QString().sprintf( "%5d  %5d", nsolmin, nsolmax );
   modstats << tr( "Average calculated solutes:" )
            << QString().sprintf( "%5d", nsolavg );
   modstats << tr( "Minimum variance:" )
            << QString().sprintf( "%12.6e", varimin );
   modstats << tr( "Minimum, Maximum rmsd:" )
            << QString().sprintf( "%12.8f  %12.8f", rmsdmin, rmsdmax );
   modstats << tr( "Average, Median rmsd:" )
            << QString().sprintf( "%12.8f  %12.8f", rmsdavg, rmsdmed );
   modstats << tr( "Number of \"better\" models:" )
            << QString().sprintf( "%5d", nbmods );
   modstats << tr( "%1 Best Min,Max calculated solutes:" ).arg( nbmods )
            << QString().sprintf( "%5d  %5d", nbsomin, nbsomax );
   modstats << tr( "%1 Best Average calculated solutes:" ).arg( nbmods )
            << QString().sprintf( "%5d", nbsoavg );
   modstats << tr( "%1 Best Minimum, Maximum rmsd:" ).arg( nbmods )
            << QString().sprintf( "%12.8f  %12.8f", brmsmin, brmsmax );
   modstats << tr( "%1 Best Average, Median rmsd:" ).arg( nbmods )
            << QString().sprintf( "%12.8f  %12.8f", brmsavg, brmsmed );
   modstats << tr( "Tikhonov regularization parameter:" )
            << QString().sprintf( "%12.3f", alpha );
DbgLv(1) << "PC:MS:   RTN";

}

// Do curve-fit evaluate function (return RMSD) for a Straight Line model
double US_pcsaProcess::fit_function_SL( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 18 ];     // Static array for holding parameters
   static const int nepar = sizeof( epar ) / sizeof( epar[ 0 ] );

   if ( t > 0.0 )
   { // If not t[0], return immediately
      return 0.0;
   }

   if ( t < 0.0 )
   { // If t is special flag (-ve.), reset ffcall and return
      ffcall = 0;
      return 0.0;
   }

   QTime ftimer;
   ftimer.start();
   QList< US_SolveSim::DataSet* > dsets;
   void** iparP  = (void**)par;
   void** parP   = (void**)epar;
   double par1   = par[ 0 ];
   double par2   = par[ 1 ];
   int    px     = ( sizeof( double ) / sizeof( void* ) ) * 2;
   if ( ffcall == 0 )
   { // On 1st call, copy par array to internal static one
      for ( int ii = 0; ii < nepar - 2; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double smin   = epar[ 2 ];
   double smax   = epar[ 3 ];
   double klow   = epar[ 6 ];
   double khigh  = epar[ 7 ];
   double p1lo   = epar[ 8 ];
   double p1hi   = epar[ 9 ];
   double p2lo   = epar[ 10 ];
   double p2hi   = epar[ 11 ];
   int    noisfl = (int)epar[ 12 ];
   int    dbg_lv = (int)epar[ 13 ];
   double alpha  = epar[ 14 ];
   double kstart = par1;
   double kend   = kstart + par2 * ( smax - smin );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      klow         -= 0.1;
      khigh        += 0.1;
      p1lo         -= 0.1;
      p1hi         += 0.1;
      p2lo         -= 0.01;
      p2hi         += 0.02;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           kstart < klow   ||  kend   < klow  ||
           kstart > khigh  ||  kend   > khigh )
      {
qDebug() << "ffSL: call" << ffcall << "par1 par2" << par1 << par2
 << "ks ke" << kstart << kend << "*OUT-OF-LIMITS*";
         return 1e+99;
      }
   }

   double prange = (double)( nlpts - 1 );
   double sinc   = ( smax - smin ) / prange;
   double kinc   = ( kend - kstart ) / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double scurr  = smin;
   double kcurr  = kstart;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;
   sim_vals.alpha     = alpha;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      sim_vals.solutes << US_Solute( scurr * 1e-13, kcurr, 0.0, vbar20 );
      scurr        += sinc;
      kcurr        += kinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 15 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffSL: call" << ffcall << "par1 par2" << par1 << par2
 << "rmsd" << rmsd << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffSL:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];
//qDebug() << "ffSL:  dsets[0]" << dsets[0] << parP[0]
// << "dsets[0]->vbar20" << dsets[0]->vbar20;
qDebug() << "ffSL:    ks ke kl kh" << kstart << kend << klow << khigh;

   return rmsd;
}

// Do curve-fit evaluate function (return RMSD) for a Increasing Sigmoid model
double US_pcsaProcess::fit_function_IS( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 18 ];     // Static array for holding parameters
   static const int nepar = sizeof( epar ) / sizeof( epar[ 0 ] );

   if ( t > 0.0 )
   { // If not t[0], return immediately
      return 0.0;
   }

   if ( t < 0.0 )
   { // If t is special flag (-ve.), reset ffcall and return
      ffcall = 0;
      return 0.0;
   }

   QTime ftimer;
   ftimer.start();
   QList< US_SolveSim::DataSet* > dsets;
   void** iparP  = (void**)par;
   void** parP   = (void**)epar;
   double par1   = par[ 0 ];
   double par2   = par[ 1 ];
   int    px     = ( sizeof( double ) / sizeof( void* ) ) * 2;
   if ( ffcall == 0 )
   { // On 1st call, copy par array to internal static one
      for ( int ii = 0; ii < nepar - 2; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double smin   = epar[ 2 ];
   double smax   = epar[ 3 ];
   double kmin   = epar[ 4 ];
   double kmax   = epar[ 5 ];
   double klow   = epar[ 6 ];
   double khigh  = epar[ 7 ];
   double p1lo   = epar[ 8 ];
   double p1hi   = epar[ 9 ];
   double p2lo   = epar[ 10 ];
   double p2hi   = epar[ 11 ];
   int    noisfl = (int)epar[ 12 ];
   int    dbg_lv = (int)epar[ 13 ];
   double alpha  = epar[ 14 ];
   double kstr   = kmin;
   double kdif   = kmax - kmin;
   double srange = smax - smin;
   double p1fac  = sqrt( 2.0 * qMax( par1, p1lo ) );
   double kstart = kstr + kdif * ( 0.5 * erf( ( 0.0 - par2 ) / p1fac ) + 0.5 );
   double kend   = kstr + kdif * ( 0.5 * erf( ( 1.0 - par2 ) / p1fac ) + 0.5 );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      klow         -= 0.1;
      khigh        += 0.1;
      p1lo         -= 0.00001;
      p1hi         += 0.00001;
      p2lo         -= 0.01;
      p2hi         += 0.01;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           kstart < klow   ||  kend   < klow  ||
           kstart > khigh  ||  kend   > khigh )
      {
qDebug() << "ffIS: call" << ffcall << "par1 par2" << par1 << par2
 << "ks ke" << kstart << kend << "*OUT-OF-LIMITS*";
         return 1e+99;
      }
   }

   double prange = (double)( nlpts - 1 );
   double xinc   = 1.0 / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double scurr  = smin;
   double kcurr  = kmin;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;
   sim_vals.alpha     = alpha;

   double xval   = 0.0;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      double efac   = 0.5 * erf( ( xval - par2 ) / p1fac ) + 0.5;
      scurr         = smin + xval * srange;
      kcurr         = kmin + kdif * efac;
      sim_vals.solutes << US_Solute( scurr * 1e-13, kcurr, 0.0, vbar20 );
      xval         += xinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 15 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffIS: call" << ffcall << "par1 par2" << par1 << par2
 << "rmsd" << rmsd << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffIS:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];

   return rmsd;
}

// Do curve-fit evaluate function (return RMSD) for a Decreasing Sigmoid model
double US_pcsaProcess::fit_function_DS( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 18 ];     // Static array for holding parameters
   static const int nepar = sizeof( epar ) / sizeof( epar[ 0 ] );

   if ( t > 0.0 )
   { // If not t[0], return immediately
      return 0.0;
   }

   if ( t < 0.0 )
   { // If t is special flag (-ve.), reset ffcall and return
      ffcall = 0;
      return 0.0;
   }
   QTime ftimer;
   ftimer.start();
   QList< US_SolveSim::DataSet* > dsets;
   void** iparP  = (void**)par;
   void** parP   = (void**)epar;
   double par1   = par[ 0 ];
   double par2   = par[ 1 ];
   int    px     = ( sizeof( double ) / sizeof( void* ) ) * 2;
   if ( ffcall == 0 )
   { // On 1st call, copy par array to internal static one
      for ( int ii = 0; ii < nepar - 2; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double smin   = epar[ 2 ];
   double smax   = epar[ 3 ];
   double kmin   = epar[ 4 ];
   double kmax   = epar[ 5 ];
   double klow   = epar[ 6 ];
   double khigh  = epar[ 7 ];
   double p1lo   = epar[ 8 ];
   double p1hi   = epar[ 9 ];
   double p2lo   = epar[ 10 ];
   double p2hi   = epar[ 11 ];
   int    noisfl = (int)epar[ 12 ];
   int    dbg_lv = (int)epar[ 13 ];
   double alpha  = epar[ 14 ];
   double kstr   = kmax;
   double kdif   = kmin - kmax;
   double srange = smax - smin;
   double p1fac  = sqrt( 2.0 * qMax( par1, p1lo ) );
   double kstart = kstr + kdif * ( 0.5 * erf( ( 0.0 - par2 ) / p1fac ) + 0.5 );
   double kend   = kstr + kdif * ( 0.5 * erf( ( 1.0 - par2 ) / p1fac ) + 0.5 );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      klow         -= 0.1;
      khigh        += 0.1;
      p1lo         -= 0.00001;
      p1hi         += 0.00001;
      p2lo         -= 0.01;
      p2hi         += 0.01;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           kstart < klow   ||  kend   < klow  ||
           kstart > khigh  ||  kend   > khigh )
      {
qDebug() << "ffDS: call" << ffcall << "par1 par2" << par1 << par2
 << "ks ke" << kstart << kend << "*OUT-OF-LIMITS*";
         return 1e+99;
      }
   }

   double prange = (double)( nlpts - 1 );
   double xinc   = 1.0 / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double scurr  = smin;
   double kcurr  = kmin;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;
   sim_vals.alpha     = alpha;

   double xval   = 0.0;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      double efac   = 0.5 * erf( ( xval - par2 ) / p1fac ) + 0.5;
      scurr         = smin + xval * srange;
      kcurr         = kstr + kdif * efac;
      sim_vals.solutes << US_Solute( scurr * 1e-13, kcurr, 0.0, vbar20 );
      xval         += xinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 15 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffDS: call" << ffcall << "par1 par2" << par1 << par2
 << "rmsd" << rmsd << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffDS:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];

   return rmsd;
}

// Do curve-fit evaluate function (return RMSD) for a Horizontal Line model
double US_pcsaProcess::fit_function_HL( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 18 ];     // Static array for holding parameters
   static const int nepar = sizeof( epar ) / sizeof( epar[ 0 ] );

   if ( t > 0.0 )
   { // If not t[0], return immediately
      return 0.0;
   }

   if ( t < 0.0 )
   { // If t is special flag (-ve.), reset ffcall and return
      ffcall = 0;
      return 0.0;
   }

   QTime ftimer;
   ftimer.start();
   QList< US_SolveSim::DataSet* > dsets;
   void** iparP  = (void**)par;
   void** parP   = (void**)epar;
   double par1   = par[ 0 ];
   double par2   = par[ 0 ];
   int    px     = ( sizeof( double ) / sizeof( void* ) ) * 2;
   if ( ffcall == 0 )
   { // On 1st call, copy par array to internal static one
      for ( int ii = 0; ii < nepar - 2; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double smin   = epar[ 2 ];
   double smax   = epar[ 3 ];
   double klow   = epar[ 6 ];
   double khigh  = epar[ 7 ];
   double p1lo   = epar[ 8 ];
   double p1hi   = epar[ 9 ];
   int    noisfl = (int)epar[ 12 ];
   int    dbg_lv = (int)epar[ 13 ];
   double alpha  = epar[ 14 ];
   double kval   = par1;

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      klow         -= 0.1;
      khigh        += 0.1;
      p1lo         -= 0.1;
      p1hi         += 0.1;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par1   > p1hi  ||
           kval   < klow   ||  kval   > khigh )
      {
qDebug() << "ffHL: call" << ffcall << "par1" << par1 << "kv" << kval 
 << "*OUT-OF-LIMITS*";
         return 1e+99;
      }
   }

   double prange = (double)( nlpts - 1 );
   double sinc   = ( smax - smin ) / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double scurr  = smin;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;
   sim_vals.alpha     = alpha;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      sim_vals.solutes << US_Solute( scurr * 1e-13, kval, 0.0, vbar20 );
      scurr        += sinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 15 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffHL: call" << ffcall << "par1 par2" << par1 << par2
 << "rmsd" << rmsd << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffHL:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];
//qDebug() << "ffHL:  dsets[0]" << dsets[0] << parP[0]
// << "dsets[0]->vbar20" << dsets[0]->vbar20;
qDebug() << "ffHL:    kv kl kh" << kval << klow << khigh;

   return rmsd;
}

// Do Levenberg-Marquardt fit
void US_pcsaProcess::LevMarq_fit( void )
{
   const int eslnc = 32;   // Estimated straight-line LM eval calls
   const int esigc = 44;   // Estimated sigmoid LM eval calls
   static US_LM::LM_Control control( 1.e-5, 1.e-5, 1.e-5, 1.e-5,
                                     100., 100, 0,  3 );
   if ( lmmxcall < 1 )
      return;

   static US_LM::LM_Status  status;
   static int    n_par  = 2;
   static int    m_dat  = 3;
DbgLv(1) << "LMf: n_par m_dat" << n_par << m_dat;
   static double tarray[ 3 ] = { 0.0, 1.0, 2.0 };
   static double yarray[ 3 ] = { 0.0, 0.0, 0.0 };
   static double parray[ 20 ];
   bool   LnType = ( curvtype == 0  ||  curvtype == 3 );
   double minkv  = kuplim;
   double maxkv  = klolim;
   double maxsl  = ( kuplim - klolim ) / ( suplim - slolim );
   double minsl  = -maxsl;
   double minp1  = LnType ? minkv : 0.5;
   double maxp1  = LnType ? maxkv : 0.001;
   double minp2  = LnType ? maxsl : 1.0;
   double maxp2  = LnType ? minsl : 0.0;
   int    npar   = ( curvtype != 3 ) ? n_par : 1;
   control.maxcall      = lmmxcall / ( npar + 1 );
   lm_done       = false;
   // Start timer for L-M progress bar, based on estimated duration
   kcsteps       = 0;
   int    stepms = 500;
   kctask        = LnType ?
                   ( time_fg * nthreads * eslnc + nmtasks / 2 ) / nmtasks :
                   ( time_fg * nthreads * esigc + nmtasks / 2 ) / nmtasks;
   nctotal       = ( kctask + stepms / 2 ) / stepms;
   kctask        = nctotal * stepms;
   lmtm_id       = startTimer( stepms );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   emit message_update( tr( "\nNow refining the best model with a "
                            "Levenberg-Marquardt fit ...\n" ), true );

   elite_limits( mrecs, minkv, maxkv, minp1, maxp1, minp2, maxp2 );
#if 0
const double par1lo=0.001;
const double par1up=0.5;
const double par2lo=0.0;
const double par2up=1.0;
minkv=klolim;
maxkv=kuplim;
minp1=(minp1+par1lo)*0.5;
maxp1=(maxp1+par1up)*0.5;
minp2=(minp2+par2lo)*0.5;
maxp2=(maxp2+par2up)*0.5;
#endif

   double ibm_rmsd = mrecs[ 0 ].rmsd;  // Initial Best Model RMSD
   double *t     = tarray;
   double *y     = yarray;
   double *par   = parray;
   void   **ppar = (void**)par;
   int    px     = ( sizeof( double ) / sizeof( void* ) ) * 2;
   par[ 0 ]      = mrecs[ 0 ].par1;
   par[ 1 ]      = mrecs[ 0 ].par2;
DbgLv(0) << "LMf:  par1 par2" << par[0] << par[1];
   par[ 2 ]      = 0.0;
   ppar[ px ]    = (void*)dsets[ 0 ];
   par[ 3 ]      = (double)cresolu;
   par[ 4 ]      = slolim;
   par[ 5 ]      = suplim;
   par[ 6 ]      = klolim;
   par[ 7 ]      = kuplim;
   par[ 8 ]      = minkv;
   par[ 9 ]      = maxkv;
   par[ 10 ]     = minp1;
   par[ 11 ]     = maxp1;
   par[ 12 ]     = minp2;
   par[ 13 ]     = maxp2;
   par[ 14 ]     = noisflag;
//   par[ 13 ]     = dbg_level;
   par[ 15 ]     = 0;
   par[ 16 ]     = alpha_lm;
DbgLv(1) << "LMf:  ppar2" << ppar[4] << dsets[0] << curvtype << ppar[5];
DbgLv(1) << "LMf:  alpha" << alpha_lm << par[14];
   timer.start();              // start a timer to measure run time

   if ( curvtype == 0 )
   { // Fit with Levenberg-Marquardt for straight-line curves
      control.ftol     = 1.e-5;
      control.xtol     = 1.e-5;
      control.gtol     = 1.e-5;
      control.epsilon  = 1.e-5;
DbgLv(0) << "lmcurve_fit (SL) with par1,par2" << par[0] << par[1]
   << "ftol,epsl" << control.ftol << control.epsilon;
      fit_function_SL( -1.0, par );    // Make sure to reset eval. function

      US_LM::lmcurve_fit_rmsd( npar, par, m_dat, t, y,
            &(US_pcsaProcess::fit_function_SL), &control, &status );

DbgLv(0) << "  lmcurve_fit (SL) return: par1,par2" << par[0] << par[1];
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info
   << US_LM::lm_statmsg( &status, false );
double ys = par[0];
double ye = ys + par[1] * ( suplim - slolim );
//double rmsd = par[10];
double rmsd = sqrt( dsets[0]->model.variance );
int    nsol = dsets[0]->model.components.size();
DbgLv(0) << "   lmcfit xs,ys xe,ye rmsd" << slolim << ys << suplim << ye
 << rmsd << "ncsol" << nsol;
   }

   else if ( curvtype == 1 )
   { // Fit with Levenberg-Marquardt for increasing-sigmoid curves
      //control.ftol     = 30.0 * DBL_EPSILON;
      control.ftol     = 1.0e-5;
      control.xtol     = control.ftol;
      control.gtol     = control.ftol;
      control.epsilon  = ibm_rmsd * 8.0e-5;
      fit_function_IS( -1.0, par );    // Make sure to reset eval. function
DbgLv(0) << "lmcurve_fit (IS) with par1,par2" << par[0] << par[1]
   << QString().sprintf( "%14.8e %14.8e", par[0], par[1] )
   << "ftol,epsl" << control.ftol << control.epsilon;

      US_LM::lmcurve_fit_rmsd( npar, par, m_dat, t, y,
            &(US_pcsaProcess::fit_function_IS), &control, &status );

DbgLv(0) << "  lmcurve_fit (IS) return: par1,par2" << par[0] << par[1]
   << QString().sprintf( "%14.8e %14.8e", par[0], par[1] );
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info
   << US_LM::lm_statmsg( &status, false );
double rmsd = sqrt( dsets[0]->model.variance );
int    nsol = dsets[0]->model.components.size();
DbgLv(0) << "   lmcfit rmsd" << rmsd << "#solutes" << nsol; 
   }

   else if ( curvtype == 2 )
   { // Fit with Levenberg-Marquardt for decreasing-sigmoid curves
      control.ftol     = 1.e-16;
      control.xtol     = 1.e-16;
      control.gtol     = 1.e-16;
      control.epsilon  = 1.e-4;
      fit_function_DS( -1.0, par );    // Make sure to reset eval. function
DbgLv(0) << "lmcurve_fit (DS) with par1,par2" << par[0] << par[1]
   << QString().sprintf( "%14.8e %14.8e", par[0], par[1] )
   << "ftol,epsl" << control.ftol << control.epsilon;

      US_LM::lmcurve_fit_rmsd( npar, par, m_dat, t, y,
            &(US_pcsaProcess::fit_function_DS), &control, &status );

DbgLv(0) << "  lmcurve_fit (DS) return: par1,par2" << par[0] << par[1]
   << QString().sprintf( "%14.8e %14.8e", par[0], par[1] );
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info;
double rmsd = sqrt( dsets[0]->model.variance );
int    nsol = dsets[0]->model.components.size();
DbgLv(0) << "   lmcfit rmsd" << rmsd << "#solutes" << nsol; 
   }

   else if ( curvtype == 3 )
   { // Fit with Levenberg-Marquardt for horizontal-line curves
      control.ftol     = 1.e-5;
      control.xtol     = 1.e-5;
      control.gtol     = 1.e-5;
      control.epsilon  = 1.e-5;
DbgLv(0) << "lmcurve_fit (HL) with par1" << par[0]
   << "ftol,epsl" << control.ftol << control.epsilon;
      fit_function_HL( -1.0, par );    // Make sure to reset eval. function

      US_LM::lmcurve_fit_rmsd( npar, par, m_dat, t, y,
            &(US_pcsaProcess::fit_function_HL), &control, &status );

DbgLv(0) << "  lmcurve_fit (HL) return: par1" << par[0];
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info
   << US_LM::lm_statmsg( &status, false );
double yv = par[0];
double rmsd = sqrt( dsets[0]->model.variance );
int    nsol = dsets[0]->model.components.size();
DbgLv(0) << "   lmcfit xs,yv xe,yv rmsd" << slolim << yv << suplim << yv
 << rmsd << "ncsol" << nsol;
   }

   else
   {
      DbgLv( 0 ) << "*ERROR* invalid curvtype" << curvtype;
   }

   lm_done       = true;
   QApplication::restoreOverrideCursor();
   US_SolveSim::DataSet* dset = dsets[ 0 ];
   double rmsd   = sqrt( dset->model.variance );
   int    nsol   = dset->model.components.size();
   int    nfev   = status.nfev;
   time_lm       = timer.elapsed();
   int    ktimes = ( time_lm + 500 ) / 1000;
   int    ktimeh = ktimes / 3600;
   int    ktimem = ( ktimes - ktimeh * 3600 ) / 60;
   ktimes        = ktimes - ktimeh * 3600 - ktimem * 60;
DbgLv(0) << "     lmcfit time: " << ktimeh << "h" << ktimem
 << "m" << ktimes << "s";
DbgLv(0) << "     lmcfit  LM time(ms):  estimated" << kctask
 << "actual" << time_lm;
   QString fmsg = tr( "The new best model has par1 %1,  par2 %2,\n"
                      "  RMSD %3,  %4 solutes,  %5 LM iters.  " )
       .arg( par[ 0 ] ).arg( par[ 1 ] ).arg( rmsd ).arg( nsol ).arg( nfev );
   if ( ktimeh == 0 )
      fmsg      = fmsg + tr( "(%1 min., %2 sec.)" )
                         .arg( ktimem ).arg( ktimes );
   else
      fmsg      = fmsg + tr( "(%1 hr., %2 min., %3 sec.)" )
                         .arg( ktimeh ).arg( ktimem ).arg( ktimes );
   if ( alpha_lm != 0.0 )
      fmsg      = fmsg + tr( "\nA Tikhonov regularization parameter of %1"
                             " was used." ).arg( alpha_lm );
   emit message_update( fmsg, true );

   // Replace best model in vector and build out model more completely
   ModelRecord mrec = mrecs[ 0 ];
   US_SimulationParameters* spar = &dset->simparams;
   int  jsp       = 0;

   if ( curvtype == 0 )
   {
      mrec.str_k     = par[ 0 ];
      mrec.end_k     = par[ 0 ] + par[ 1 ] * ( suplim -slolim );
   }

   else if ( curvtype == 3 )
   {
      mrec.str_k     = par[ 0 ];
      mrec.end_k     = par[ 0 ];
      par[ 1 ]       = 0.0;
   }

   mrec.par1      = par[ 0 ];
   mrec.par2      = par[ 1 ];
   mrec.variance  = dset->model.variance;
   mrec.rmsd      = rmsd;
   mrec.ctype     = curvtype;
   mrec.smin      = slolim;
   mrec.smax      = suplim;
   mrec.kmin      = klolim;
   mrec.kmax      = kuplim;

   for ( int ii = 0; ii < mrec.isolutes.size(); ii++ )
   { // Replace s and k in top model input solutes
      mrec.isolutes[ ii ].s = spar->mesh_radius[ jsp++ ];
      mrec.isolutes[ ii ].k = spar->mesh_radius[ jsp++ ];
   }

   mrec.csolutes.clear();
   model          = dset->model;
   double sfactor = 1.0 / dset->s20w_correction;
   double dfactor = 1.0 / dset->D20w_correction;

   for ( int ii = 0; ii < nsol; ii++ )
   {
      // Insert calculated solutes into top model record
      US_Solute solute;
      solute.s   = model.components[ ii ].s;
      solute.k   = model.components[ ii ].f_f0;
      solute.c   = model.components[ ii ].signal_concentration;
      solute.v   = model.components[ ii ].vbar20;
      mrec.csolutes << solute;
DbgLv(1) << "LMf:  ii" << ii << "s k c" << solute.s << solute.k << solute.c;

      // Calculate the remainder of component values
      model.components[ ii ].D   = 0.0;
      model.components[ ii ].mw  = 0.0;
      model.components[ ii ].f   = 0.0;
      model.calc_coefficients( model.components[ ii ] );

      // Convert to experiment-space for simulation below
      model.components[ ii ].s  *= sfactor;
      model.components[ ii ].D  *= dfactor;
DbgLv(1) << "LMf:     s D mw" << model.components[ii].s
 << model.components[ii].D << model.components[ii].mw;
   }

   // Recalculate final refined simulation and residual
   US_DataIO::RawData* simdat = &mrec.sim_data;
   US_DataIO::RawData* resids = &mrec.residuals;
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
DbgLv(1) << "LMf:simparms: spts meni bott temp bpos"
 << dset->simparams.simpoints
 << dset->simparams.meniscus
 << dset->simparams.bottom
 << dset->simparams.temperature
 << dset->simparams.bottom_position;
DbgLv(1) << "LMf:model: desc analys vari" << model.description
 << model.analysis << model.variance;

   US_Astfem_RSA astfem_rsa( model, dset->simparams );
   astfem_rsa.calculate( sdata );

   // Convert model back to standard space and save in model record
   for ( int ii = 0; ii < nsol; ii++ )
   {
      model.components[ ii ].s  /= sfactor;
      model.components[ ii ].D  /= dfactor;
DbgLv(1) << "LMf:     s D mw" << model.components[ii].s
 << model.components[ii].D << model.components[ii].mw;
   }

   mrec.model      = model;

   // Fetch any noise saved in dset
   bool tino       = ( ( noisflag & 1 ) != 0 );
   bool rino       = ( ( noisflag & 2 ) != 0 );
   ti_noise.count  = 0;
   ri_noise.count  = 0;
   ti_noise.values.clear();
   ri_noise.values.clear();
   mrec.ti_noise.clear();
   mrec.ri_noise.clear();

   // Compose any noise records
   if ( tino )
   {
      for ( int ii = 0; ii < npoints; ii++ )
         ti_noise.values << spar->mesh_radius[ jsp++ ];

      ti_noise.minradius = edata->radius( 0 );
      ti_noise.maxradius = edata->radius( npoints - 1 );
      ti_noise.count     = npoints;
      mrec.ti_noise      = ti_noise.values;
DbgLv(1) << "LMf: ti count size" << ti_noise.count << ti_noise.values.size();
   }

   if ( rino )
   {
      for ( int ii = 0; ii < nscans; ii++ )
         ri_noise.values << spar->mesh_radius[ jsp++ ];

      ri_noise.count     = nscans;
      mrec.ri_noise      = ri_noise.values;
DbgLv(1) << "LMf: ri count size" << ri_noise.count << ri_noise.values.size();
   }

   // Insert new refined best model at the top of the list
DbgLv(0) << "LMf:insert-new: old par1 par2" << mrecs[0].par1 << mrecs[0].par2
 << "new par1 par2" << mrec.par1 << mrec.par2;
DbgLv(0) << "LMf: old01 s,k" << mrecs[0].isolutes[0].s << mrecs[0].isolutes[0].k
 << mrecs[0].isolutes[1].s << mrecs[0].isolutes[1].k;
DbgLv(0) << "LMf: new01 s,k" << mrec.isolutes[0].s << mrec.isolutes[0].k
 << mrec.isolutes[1].s << mrec.isolutes[1].k;
   mrecs.insert( 0, mrec );

   // Re-compute simulation and residuals
DbgLv(0) << "LMf: tino rino" << tino << rino;
DbgLv(0) << "LMf: simdat nsc npt"
 << simdat->scanCount() << simdat->pointCount();
DbgLv(1) << "LMf: resids nsc npt"
 << resids->scanCount() << resids->pointCount();
DbgLv(1) << "LMf: rdata  nsc npt"
 << rdata.scanCount() << rdata.pointCount();
DbgLv(1) << "LMf: sdata  nsc npt"
 << sdata.scanCount() << sdata.pointCount();
   spar->mesh_radius.clear();
   double cvari = 0.0;
   double crmsd = 0.0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      double rnois = rino ? mrec.ri_noise[ ss ] : 0.0;

      for ( int rr = 0; rr < npoints; rr++ )
      {
         double tnois  = tino ? mrec.ti_noise[ rr ] : 0.0;
         double resval = edata->value( ss, rr )
                       - sdata. value( ss, rr ) - tnois - rnois;
         rdata.  setValue( ss, rr, resval );

         simdat->setValue( ss, rr, sdata.value( ss, rr ) );
         resids->setValue( ss, rr, resval );
if ((ss<3 && rr<3)||((ss+4)>nscans && (rr+4)>npoints)||(rr==(npoints/2)))
DbgLv(1) << "LMf:  ss rr" << ss << rr << "edat sdat resv"
 << edata->value(ss,rr) << sdata.value(ss,rr) << resval;
         cvari         += sq( resval );
      }
   }
   cvari         /= (double)( nscans * npoints );
   crmsd          = sqrt( cvari );
   emit progress_update( cvari );     // Pass progress on to control window
DbgLv(0) << "LMf: recomputed variance rmsd" << cvari << crmsd;

}

void US_pcsaProcess::elite_limits( QVector< ModelRecord >& mrecs,
      double& minkv, double& maxkv, double& minp1, double& maxp1,
      double& minp2, double& maxp2 )
{
   const double efrac = 0.1;
   // Set up variables that help insure that the par1,par2 extents of elites
   // extend at least one step on either side of record 0's par1,par2
   double m0p1    = mrecs[ 0 ].par1;
   double m0p2    = mrecs[ 0 ].par2;
   double m0p1l   = m0p1;
   double m0p1h   = m0p1;
   double m0p2l   = m0p2;
   double m0p2h   = m0p2;
   if ( curvtype == 0  ||  curvtype == 3 )
   {  // Possibly adjust initial par1,par2 limits for lines
      m0p1l          = ( m0p1 > klolim ) ? m0p1 : ( m0p1 * 1.0001 );
      m0p1h          = ( m0p1 < kuplim ) ? m0p1 : ( m0p1 * 0.9991 );
      m0p2l          = ( m0p2 > 0.0    ) ? m0p2 : 0.0001;
      m0p2h          = m0p2 * 1.0001;
   }

   if ( curvtype == 1  ||  curvtype == 2 )
   {  // Possibly adjust initial par1,par2 limits for sigmoids
      double dif1    = m0p1 - 0.001;
      double dif2    = m0p1 - 0.500;
      double dif3    = m0p2 - 0.000;
      double dif4    = m0p2 - 1.000;
      m0p1l          = ( dif1 > 1.e-8 ) ? m0p1 : 0.002;
      m0p1h          = ( dif2 < -1e-8 ) ? m0p1 : 0.499;
      m0p2l          = ( dif3 > 1.e-8 ) ? m0p2 : 0.001;
      m0p2h          = ( dif4 < -1e-8 ) ? m0p2 : 0.999;
DbgLv(1) << " ElLim: ADJUST SIGM: m0p1 m0p1h" << m0p1 << m0p1h
   << "m0p1<0.500" << (m0p1<0.500) << 0.500 << "(m0p1-0.5)" << (m0p1-0.5);
   }

   int nmr        = mrecs.size();
   int nelite     = qRound( efrac * nmr );          // Elite is top 10%
   int maxel      = nmr / 2;
   int minel      = qMin( maxel, 7 );
   nelite         = qMin( nelite, maxel );          // At most half of all
   nelite         = qMax( nelite, minel );          // At least 7
DbgLv(0) << " ElLim: nmr nelite nmtasks" << nmr << nelite << nmtasks;
DbgLv(1) << " ElLim: in minkv maxkv" << minkv << maxkv;
DbgLv(1) << " ElLim: in min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;
DbgLv(1) << " ElLim: in m0p1 m0p2" << m0p1 << m0p2;
DbgLv(1) << " ElLim: in m0p1l,m0p1h,m0p2l,m0p2h" << m0p1l << m0p1h
 << m0p2l << m0p2h;

   for ( int ii = 0; ii < nmr; ii++ )
   {
      double str_k   = mrecs[ ii ].str_k;
      double end_k   = mrecs[ ii ].end_k;
      double par1    = mrecs[ ii ].par1;
      double par2    = mrecs[ ii ].par2;
if(ii<3||(ii+4)>nelite)
DbgLv(1) << " ElLim:   ii" << ii << "par1 par2" << par1 << par2
 << "str_k end_k" << str_k << end_k << "rmsd" << mrecs[ii].rmsd;
      minkv          = qMin( minkv, str_k );
      maxkv          = qMax( maxkv, str_k );
      minkv          = qMin( minkv, end_k );
      maxkv          = qMax( maxkv, end_k );
      minp1          = qMin( minp1, par1  );
      maxp1          = qMax( maxp1, par1  );
      minp2          = qMin( minp2, par2  );
      maxp2          = qMax( maxp2, par2  );

      if ( curvtype == 3 )
      {  // Effectively skip par2 comparisons for Horizontal Lines
         minp2          = m0p2l - 1.0;
         maxp2          = m0p2h + 1.0;
      }

      // We want to break out of the min,max scan loop if the sorted index
      // exceeds the elite count. But we continue in the loop if we have not
      // yet found min,max par1,par2 values that are at least a step
      // on either side of the par1,par2 values for the best model (m0).
if(ii>nelite)
DbgLv(1) << " ElLim:    minp1 maxp1 m0p1" << minp1 << maxp1 << m0p1
 << "minp2 maxp2 m0p2" << minp2 << maxp2 << m0p2;
      if ( ii > nelite  &&
           minp1 < m0p1l  &&  maxp1 > m0p1h  &&
           minp2 < m0p2l  &&  maxp2 > m0p2h )
         break;
   }

   if ( curvtype == 3 )
   {
      minp2          = 0.0;
      maxp2          = 0.0;
   }
DbgLv(0) << " ElLim: out minkv maxkv" << minkv << maxkv;
DbgLv(0) << " ElLim: out min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;
}

// Evaluate a model; return rmsd, model, noises
double US_pcsaProcess::evaluate_model( QList< US_SolveSim::DataSet* >& dsets,
   US_SolveSim::Simulation& sim_vals )
{
   US_SolveSim::DataSet*    dset = dsets[ 0 ];
   US_SimulationParameters* spar = &dset->simparams;
   spar->mesh_radius.clear();

   // Save the s and k of the input solutes
   for ( int ii = 0; ii < sim_vals.solutes.size(); ii++ )
   {
      spar->mesh_radius << sim_vals.solutes[ ii ].s;
      spar->mesh_radius << sim_vals.solutes[ ii ].k;
   }

   // Do astfem fit, mostly to get an RMSD
   US_SolveSim* solvesim = new US_SolveSim( dsets, 0, false );

   solvesim->calc_residuals( 0, 1, sim_vals );

   // Construct a rudimentary model from computed solutes and save it
   dset->model          = US_Model();
   dset->model.variance = sim_vals.variance;
   dset->model.components.clear();

   for ( int ii = 0; ii < sim_vals.solutes.size(); ii++ )
   {
      US_Model::SimulationComponent mcomp;
      mcomp.s      = sim_vals.solutes[ ii ].s;
      mcomp.f_f0   = sim_vals.solutes[ ii ].k;
      mcomp.vbar20 = sim_vals.solutes[ ii ].v;
      mcomp.signal_concentration = sim_vals.solutes[ ii ].c;

      dset->model.components << mcomp;
   }

   // If noise was computed, save it in simparams mesh_radius vector
   if ( sim_vals.noisflag != 0 )
   {
      int ntin  = ( ( sim_vals.noisflag & 1 ) == 0 ) ? 0
                  : sim_vals.ti_noise.size();
      int nrin  = ( ( sim_vals.noisflag & 2 ) == 0 ) ? 0
                  : sim_vals.ri_noise.size();

      for ( int ii = 0; ii < ntin; ii++ )
         spar->mesh_radius << sim_vals.ti_noise[ ii ];

      for ( int ii = 0; ii < nrin; ii++ )
         spar->mesh_radius << sim_vals.ri_noise[ ii ];
   }

   // Compute the RMSD and return it
   double rmsd   = sqrt( sim_vals.variance );
   return rmsd;
}

// Protected slot to filter timer event and handle L-M status timing
void US_pcsaProcess::timerEvent( QTimerEvent *event )
{
   int tm_id   = event->timerId();

   if ( tm_id != lmtm_id )
   { // If other than L-M timing event, pass it on to the normal handler
      QObject::timerEvent( event );
      return;
   }

   // Otherwise, bump progress counter and emit a progress signal
   kcsteps++;

   if ( lm_done )
   { // If L-M is done, signal that with progress status
      nctotal     = kcsteps;
      emit stage_complete( kcsteps, nctotal );
      killTimer( tm_id );
   }

   else if ( kcsteps < nctotal )
   { // Normal less-than-100% progress
      emit stage_complete( kcsteps, nctotal );
   }

   else
   { // Not done with L-M but task count met, so adjust total
      nctotal   = ( kcsteps * 12 ) / 10;
      emit stage_complete( kcsteps, nctotal );
   }

   return;
}

// Perform one final regularization computation when L-M was non-regularized
void US_pcsaProcess::compute_final()
{
DbgLv(1) << "CFin: alpha" << alpha;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   QTime ftimer;
   ftimer.start();
   ModelRecord mrec   = mrecs[ 0 ];
   model              = mrec.model;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisflag;
   sim_vals.dbg_level = dbg_level;
   sim_vals.alpha     = alpha;
   sim_vals.solutes   = mrec.isolutes;

   // Evaluate the model
   US_SolveSim::DataSet* dset = dsets[ 0 ];

   double rmsd    = evaluate_model( dsets, sim_vals );

   sdata          = sim_vals.sim_data;
   rdata          = sim_vals.residuals;
   int    nmsol   = dset->model.components.size();
   int    nisol   = mrec.isolutes.size();
   int    ktimsc = ( ftimer.elapsed() + 500 ) / 1000;

   QString fmsg   = tr(
      "\nA final best model (RMSD=%1; %2-solute, %3 out; %4 sec.)\n"
      " used a Tikhonov regularization parameter of %5 .\n" )
      .arg( rmsd ).arg( nisol ).arg( nmsol ).arg( ktimsc ).arg( alpha );
   emit message_update( fmsg, true );

   // Replace best model in vector and build out model more completely
   mrec.variance  = dset->model.variance;
   mrec.rmsd      = rmsd;

   mrec.csolutes.clear();
   model          = dset->model;

   for ( int ii = 0; ii < nmsol; ii++ )
   {
      // Insert calculated solutes into top model record
      US_Solute solute;
      solute.s   = model.components[ ii ].s;
      solute.k   = model.components[ ii ].f_f0;
      solute.c   = model.components[ ii ].signal_concentration;
      solute.v   = model.components[ ii ].vbar20;
      mrec.csolutes << solute;
DbgLv(1) << "CFin:  ii" << ii << "s k c" << solute.s << solute.k << solute.c;

      // Calculate the remainder of component values
      model.components[ ii ].D   = 0.0;
      model.components[ ii ].mw  = 0.0;
      model.components[ ii ].f   = 0.0;
      model.calc_coefficients( model.components[ ii ] );

DbgLv(1) << "CFin:     s D mw" << model.components[ii].s
 << model.components[ii].D << model.components[ii].mw;
   }

DbgLv(1) << "CFin:model: desc analys vari" << model.description
 << model.analysis << model.variance;

   // Replace the top model with the new regularized best model
   mrec.model     = model;
   mrecs[ 0 ]     = mrec;

   // Report new variance
   emit progress_update( mrec.variance ); 
   QApplication::restoreOverrideCursor();
DbgLv(0) << "LMf: recomputed variance rmsd" << mrec.variance << rmsd;
}

// Restart the curve grid iteration sequence
void US_pcsaProcess::restart_fit()
{
   bool   LnType = ( curvtype == 0  ||  curvtype == 3 );
   bool   SgType = ( curvtype == 1  ||  curvtype == 2 );
   errMsg        = tr( "NO ERROR: start" );
   maxrss        = 0;
   varimin       = 9.e+9;
   minvarx       = 99999;
DbgLv(1) << "RF: nmr" << mrecs.size() << "cfi_rmsd" << cfi_rmsd;

   wkstates .resize(  nthreads );
   wthreads .clear();
   job_queue.clear();

   orig_sols.clear();

DbgLv(1) << "RF: sll sul" << slolim << suplim
 << " kll kul nkp" << klolim << kuplim << nkpts
 << " type reso nth noi" << curvtype << cresolu << nthreads << noisflag;

   fi_iter++;

   double minkv  = kuplim;
   double maxkv  = klolim;
   double maxsl  = ( kuplim - klolim ) / ( suplim - slolim );
   double minsl  = -maxsl;
   double minp1  = LnType ? minkv : 0.5;
   double maxp1  = LnType ? maxkv : 0.001;
   double minp2  = LnType ? maxsl : 1.0;
   double maxp2  = LnType ? minsl : 0.0;
DbgLv(1) << "RF: 2)nmr" << mrecs.size() << "iter rd_frac" << fi_iter << rd_frac;

   elite_limits( mrecs, minkv, maxkv, minp1, maxp1, minp2, maxp2 );

   parlims[ 0 ]  = minp1;
   parlims[ 1 ]  = maxp1;
   parlims[ 2 ]  = minp2;
   parlims[ 3 ]  = maxp2;

   // experiment data dimensions
   nscans      = edata->scanCount();
   npoints     = edata->pointCount();
   //ModelRecord mrec   = mrecs[ 0 ];
   mrecs    .clear();

   if ( LnType )
   { // Determine models for straight-line or horizontal-line curves
      double xrng   = suplim - slolim;
      double yslo   = qMax( minp1, minkv );
      double yshi   = qMin( maxp1, maxkv );
      double yelo   = yslo + minp2 * xrng;
             yelo   = qMax( yelo,  minkv );
             yelo   = qMin( yelo,  maxkv );
      double yehi   = yshi + maxp2 * xrng;
             yehi   = qMax( yehi,  minkv );
             yehi   = qMin( yehi,  maxkv );
      parlims[ 0 ]  = yslo;
      parlims[ 1 ]  = yshi;
      parlims[ 2 ]  = yelo;
      parlims[ 3 ]  = yehi;
DbgLv(1) << "RF: slin: mmk mmp1 mmp2" << minkv << maxkv << minp1 << maxp1
 << minp2 << maxp2;
DbgLv(1) << "RF: slin:  plims0-3: yslo,yshi:" << yslo << yshi
 << "yelo,yehi" << yelo << yehi;

      nmtasks     = slmodels( curvtype, slolim, suplim, klolim, kuplim, nkpts,
                              cresolu );
   }

   else if ( SgType )
   { // Determine models for sigmoid curves
DbgLv(1) << "RF: sigm: nkpts" << nkpts;

      nmtasks     = sigmodels( curvtype, slolim, suplim, klolim, kuplim, nkpts,
                               cresolu );
DbgLv(1) << "RF: sigm: nmtasks" << nmtasks;
   }

   else
      nmtasks     = 0;

   kcsteps     = 0;
   nctotal     = nmtasks;
   emit stage_complete( kcsteps, nctotal );

   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nmtasks ) ? nthreads : nmtasks;
DbgLv(1) << "RF:   nscans npoints" << nscans << npoints;
DbgLv(1) << "RF:   nctotal nthreads" << nctotal << nthreads;
   max_rss();
DbgLv(1) << "RF: (1)maxrss" << maxrss;

   // Queue all the tasks
   for ( int ktask = 0; ktask < nmtasks; ktask++ )
   {
      WorkPacket wtask;
      int    mm   = orig_sols[ ktask ].size() - 1;
      double strk = orig_sols[ ktask ][ 0  ].k;
      double endk = orig_sols[ ktask ][ mm ].k;
      wtask.par1  = mrecs[ ktask ].par1;
      wtask.par2  = mrecs[ ktask ].par2;
      wtask.depth = 0;

      wtask.sim_vals.alpha = alpha_fx;

      queue_task( wtask, strk, endk, ktask, noisflag, orig_sols[ ktask ] );
   }

   // Start the first threads. This will begin the first work units (subgrids).
   // Thereafter, work units are started in new threads when previous
   // threads signal that they have completed their work.

   for ( int ii = 0; ii < nthreads; ii++ )
   {
      wthreads << 0;

      WorkPacket wtask = job_queue.takeFirst();
      submit_job( wtask, ii );
   }

   mrecs    .clear();
   max_rss();
   kstask = nthreads;     // count of started tasks is initially thread count
DbgLv(1) << "RF:   kstask nthreads" << kstask << nthreads << job_queue.size();

   emit message_update( pmessage_head() +
      tr( "Starting fit iteration %1 computations of %2 models,\n"
          "  using %3 threads" )
      .arg( fi_iter ).arg( nmtasks ).arg( nthreads ), false );
}

