//! \file us_1dsa_process.cpp
#include <QApplication>
#include <QtCore>

#include "us_1dsa_process.h"
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

// Class to process 1DSA simulations
US_1dsaProcess::US_1dsaProcess( QList< US_SolveSim::DataSet* >& dsets,
   QObject* parent /*=0*/ ) : QObject( parent ), dsets( dsets )
{
   edata            = &dsets[ 0 ]->run_data;  // pointer to  experiment data
   dbg_level        = US_Settings::us_debug();
   maxrss           = 0;              // max memory

   mrecs    .clear();                 // computed model records
   simparms = &dsets[ 0 ]->simparams; // pointer to simulation parameters

   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   cresolu          = 100;
   curvtype         = 0;
   nmtasks          = 100;
   kctask           = 0;
   kstask           = 0;
   varimin          = 9.e+9;
   minvarx          = 99999;
}

// Get maximum used memory
long int US_1dsaProcess::max_rss( void )
{
   return US_Memory::rss_max( maxrss );
}

// Start a specified 1DSA fit run
void US_1dsaProcess::start_fit( double sll, double sul, double kll, double kul,
                                double kin, int    res, int    typ, int    nth,
                                int    noi )
{
DbgLv(1) << "2P(1dsaProc): start_fit()";
   abort       = false;
   slolim      = sll;
   suplim      = sul;
   klolim      = kll;
   kuplim      = kul;
   kincr       = kin;
   cresolu     = res;
   curvtype    = typ;
   nthreads    = nth;
   noisflag    = noi;
   errMsg      = tr( "NO ERROR: start" );
   maxrss      = 0;
   varimin     = 9.e+9;
   minvarx     = 99999;

   wkstates .resize(  nthreads );
   wthreads .clear();
   job_queue.clear();

   orig_sols.clear();
   mrecs    .clear();

DbgLv(1) << "2P: sll sul" << slolim << suplim
 << " kll kul kin" << klolim << kuplim << kincr
 << " type reso nth noi" << curvtype << cresolu << nthreads << noisflag;

   timer.start();              // start a timer to measure run time

   // experiment data dimensions
   nscans      = edata->scanData.size();
   npoints     = edata->x.size();

   if ( curvtype == 0 )
   { // Determine models for straight-line curves
      nmtasks     = slmodels( slolim, suplim, klolim, kuplim, kincr, cresolu );
   }

   else if ( curvtype == 1  ||  curvtype == 2 )
   { // Determine models for sigmoid curves
      int nkpts   = (int)kincr;
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
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints;
DbgLv(1) << "2P:   nctotal nthreads" << nctotal << nthreads;
   max_rss();
DbgLv(1) << "2P: (1)maxrss" << maxrss;

   // Queue all the tasks
   for ( int ktask = 0; ktask < nmtasks; ktask++ )
   {
      WorkPacket wtask;
      int    mm   = orig_sols[ ktask ].size() - 1;
      double strk = orig_sols[ ktask ][ 0  ].k;
      double endk = orig_sols[ ktask ][ mm ].k;
      wtask.par1  = mrecs[ ktask ].par1;
      wtask.par2  = mrecs[ ktask ].par2;

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
DbgLv(1) << "2P:   kstask nthreads" << kstask << nthreads << job_queue.size();

   emit message_update( pmessage_head() +
      tr( "Starting computations of %1 models\n using %2 threads ..." )
      .arg( nmtasks ).arg( nthreads ), false );
}

// Abort a fit run
void US_1dsaProcess::stop_fit()
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

// Slot to handle the low-variance record of calculated solutes
void US_1dsaProcess::process_final( ModelRecord& mrec )
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
   double sfactor     = 1.0 / dset->s20w_correction;
   double dfactor     = 1.0 / dset->D20w_correction;
   double vbar20      = dset->vbar20;
   model.components.resize( nsolutes );
   qSort( mrec.csolutes );
DbgLv(1) << "FIN_FIN: s20w,D20w_corr" << dset->s20w_correction
 << dset->D20w_correction << "sfac dfac" << sfactor << dfactor;

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

      // Convert to experiment-space for simulation below
      mcomp.s     *= sfactor;
      mcomp.D     *= dfactor;
DbgLv(1) << "  Bcc 20w comp D" << mcomp.D;

      model.components[ cc ]  = mcomp;
   }

DbgLv(1) << "FIN_FIN:    c0 cn" << mrec.csolutes[0].c
 << mrec.csolutes[qMax(0,nsolutes-1)].c << "  nsols" << nsolutes;
   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_DataIO2::RawData* simdat = &mrec.sim_data;
   US_DataIO2::RawData* resids = &mrec.residuals;
DbgLv(1) << "FIN_FIN: nscans npoints" << nscans << npoints;
DbgLv(1) << "FIN_FIN: simdat nsc npt"
 << simdat->scanData.size() << simdat->x.size();
DbgLv(1) << "FIN_FIN: resids nsc npt"
 << resids->scanData.size() << resids->x.size();
DbgLv(1) << "FIN_FIN: rdata  nsc npt"
 << rdata.scanData.size() << rdata.x.size();
DbgLv(1) << "FIN_FIN: sdata  nsc npt"
 << sdata.scanData.size() << sdata.x.size();

   // build residuals data set (experiment minus simulation minus any noise)
   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         sdata.scanData[ ss ].readings[ rr ].value = simdat->value( ss, rr );
         rdata.scanData[ ss ].readings[ rr ].value = resids->value( ss, rr );
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
 << "slun klun" << slolim << suplim << klolim << kuplim << kincr
 << "ets" << ktimes;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // compose final status message
   QString pmsg = pmessage_head() +
      tr( "The Solution has converged...\n"
          "Threads:  %1 ;   Models:  %2\nRun time:  " )
      .arg( nthreads ).arg( nmtasks );

   if ( ktimeh > 0 )
      pmsg += tr( "%1 hr., %2 min., %3 sec.;" )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      pmsg += tr( "%1 min., %2 sec.;" )
         .arg( ktimem ).arg( ktimes );

   max_rss();
   int memmb  = qRound( (double)maxrss / 1024.0 );

   pmsg += tr( "   Max. memory:  " ) +
           QString::number( memmb ) + " MB\n\n" +
           tr( "The best model (RMSD=%1, %2 solutes, index %3) is:\n" )
           .arg( mrec.rmsd ).arg( nsolutes ).arg( mrec.taskx );
   if ( curvtype == 0 )
   {
      pmsg += tr( "  the line from s,f/f0  %1, %2  to %3, %4 ." )
              .arg( slolim ).arg( mrec.str_k ).arg( suplim ).arg( mrec.end_k );
   }
   else if ( curvtype == 1  ||  curvtype == 2 )
   {
      int    nkpts  = (int)kincr;
      int    p1ndx  = mrec.taskx / nkpts;
      int    p2ndx  = mrec.taskx - ( p1ndx * nkpts );
      double krng   = (double)( nkpts - 1 );
      double p1off  = (double)p1ndx / krng;
      double par1   = exp( log( 0.001 )
                           + ( log( 0.5 ) - log( 0.001 ) ) * p1off );
      double par2   = (double)p2ndx / krng;
      pmsg += tr( "  the curve with par1=%1 and par2=%2." )
              .arg( par1 ).arg( par2 );
   }

   emit message_update( pmsg, false );          // signal final message

DbgLv(1) << "FIN_FIN: Run Time: hr min sec" << ktimeh << ktimem << ktimes;
DbgLv(1) << "FIN_FIN: maxrss memmb nthr" << maxrss << memmb << nthreads
 << " nsubg noisf" << nmtasks << noisflag;
DbgLv(1) << "FIN_FIN:   kcsteps nctotal" << kcsteps << nctotal;

   // Compute a best model using Levenberg-Marquardt
   LevMarq_fit();

   nsolutes           = model.components.size();

   // Convert model components s,D back to 20,w form for output
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
DbgLv(1) << "cc comp D" << model.components[ cc ].D;
      model.components[ cc ].s *= dset->s20w_correction;
      model.components[ cc ].D *= dset->D20w_correction;
DbgLv(1) << " cc 20w comp D" << model.components[ cc ].D;
   }

   emit process_complete( 9 );     // signal that all processing is complete
}

// Public slot to get results upon completion of all refinements
bool US_1dsaProcess::get_results( US_DataIO2::RawData*    da_sim,
                                  US_DataIO2::RawData*    da_res,
                                  US_Model*               da_mdl,
                                  US_Noise*               da_tin,
                                  US_Noise*               da_rin,
                                  int&                    bm_ndx,
                                  QStringList&            modstats,
                                  QVector< ModelRecord >& p_mrecs )
{
   bool all_ok = true;

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
 << "BM_NDX" << bm_ndx;

   model_statistics( mrecs, modstats );

   return all_ok;
}

// Submit a job
void US_1dsaProcess::submit_job( WorkPacket& wtask, int thrx )
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
void US_1dsaProcess::process_job( WorkerThread* wthrd )
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
   double variance = wresult.sim_vals.variances[ 0 ];

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

   if ( variance < varimin )
   { // Handle a new minimum variance record
      if ( minvarx < orecx )
      { // Clear vectors from the previous minimum
         mrecs[ minvarx ].clear_data();
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
   else
   { // Clear vectors from a model record that is not minimum-variance
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
      tr( "Computations for %1 of %2 models are complete." )
      .arg( kctask ).arg( nmtasks ), false );

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

      process_final( mrecs[ 0 ] );
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
void US_1dsaProcess::queue_task( WorkPacket& wtask, double strk, double endk,
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
void US_1dsaProcess::free_worker( int tx )
{
   if ( tx >= 0  &&  tx < nthreads )
   {
      if ( wthreads[ tx ] != 0 )
         delete wthreads[ tx ];       // destroy thread

      wthreads[ tx ] = 0;             // set thread pointer to null
      wkstates[ tx ] = READY;         // mark its slot as available
   }
}

QString US_1dsaProcess::pmessage_head()
{
   const char* ctp[] = { "Straight Line",
                         "Increasing Sigmoid",
                         "Decreasing Sigmoid",
                         "?UNKNOWN?"
                       };
   QString ctype = QString( ctp[ curvtype ] );
   return tr( "Analysis of %1 %2 models,\n  with %3 solutes each.\n" )
          .arg( nmtasks ).arg( ctype ).arg( cresolu );
}

// Get next job from queue, insuring we get the lowest depth
WorkPacket US_1dsaProcess::next_job()
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
int US_1dsaProcess::slmodels( double slo, double sup, double klo,
      double kup, double kin, int res )
{
DbgLv(1) << "SLMO: slo sup klo kup kin res" << slo << sup << klo << kup
   << kin << res;
   double vbar20   = dsets[ 0 ]->vbar20;
   orig_sols.clear();

   // Compute straight-line model records
   int nmodels = ModelRecord::compute_slines( slo, sup, klo, kup, kin,
                                              res, mrecs );

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
int US_1dsaProcess::sigmodels( int ctp, double slo, double sup, double klo,
      double kup, int nkpts, int nlpts )
{
DbgLv(1) << "SGMO: ctp slo sup klo kup nkp nlp" << ctp << slo << sup
   << klo << kup << nkpts << nlpts;
   double vbar20   = dsets[ 0 ]->vbar20;
   orig_sols.clear();

   // Compute sigmoid model records
   int nmodels = ModelRecord::compute_sigmoids( ctp, slo, sup, klo, kup,
                                                nkpts, nlpts, mrecs );

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
void US_1dsaProcess::model_statistics( QVector< ModelRecord >& mrecs,
                                       QStringList&            modstats )
{
   const char* ctp[] = { "Straight Line",
                         "Increasing Sigmoid",
                         "Decreasing Sigmoid",
                         "?UNKNOWN?"
                       };

   // Accumulate the statistics
   int    nbmods    = nmtasks / 10;
   int    nkpts     = (int)kincr;
   int    nlpts     = cresolu;
   if ( curvtype == 0 )
      nkpts            = qRound( ( kuplim - klolim ) / kincr ) + 1;
   double rmsdmin   = 99999.0;
   double rmsdmax   = 0.0;
   double rmsdavg   = 0.0;
   double brmsmin   = 99999.0;
   double brmsmax   = 0.0;
   double brmsavg   = 0.0;
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
            << QString().sprintf( "%10.4f %10.4f", slolim, suplim );
   if ( curvtype == 0 )
   {
      modstats << tr( "k (f/f0) Range:" )
               << QString().sprintf( "%10.4f  %10.4f  %10.4f",
                     klolim, kuplim, kincr );
      modstats << tr( "Best curve f/f0 end points:" )
               << QString().sprintf( "%10.4f  %10.4f",
                     mrecs[ 0 ].str_k, mrecs[ 0 ].end_k );
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
   modstats << tr( "Best curve calculated solutes:" )
            << QString().sprintf( "%5d", mrecs[ 0 ].csolutes.size() );
   modstats << tr( "Index of best model:" )
            << QString().sprintf( "%5d", mrecs[ 0 ].taskx );
   modstats << tr( "Minimum, Maximum calculated solutes:" )
            << QString().sprintf( "%5d  %5d", nsolmin, nsolmax );
   modstats << tr( "Average calculated solutes:" )
            << QString().sprintf( "%5d", nsolavg );
   modstats << tr( "Number of \"better\" models:" )
            << QString().sprintf( "%5d", nbmods );
   modstats << tr( "Min,Max calculated solutes for better:" )
            << QString().sprintf( "%5d  %5d", nbsomin, nbsomax );
   modstats << tr( "Average calculated solutes for better:" )
            << QString().sprintf( "%5d", nbsoavg );
   modstats << tr( "Minimum variance:" )
            << QString().sprintf( "%12.6e", varimin );
   modstats << tr( "Minimum, Maximum rmsd:" )
            << QString().sprintf( "%12.8f  %12.8f", rmsdmin, rmsdmax );
   modstats << tr( "Average, Median rmsd:" )
            << QString().sprintf( "%12.8f  %12.8f", rmsdavg, rmsdmed );
   modstats << tr( "Minimum, Maximum rmsd for better:" )
            << QString().sprintf( "%12.8f  %12.8f", brmsmin, brmsmax );
   modstats << tr( "Average, Median rmsd for better:" )
            << QString().sprintf( "%12.8f  %12.8f", brmsavg, brmsmed );

}

// Do curve-fit evaluate function (return RMSD) for a Straight Line model
double US_1dsaProcess::fit_function_SL( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 13 ];     // Static array for holding parameters

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
      for ( int ii = 0; ii < 12; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double xstart = epar[ 2 ];
   double xend   = epar[ 3 ];
   double ylow   = epar[ 4 ];
   double yhigh  = epar[ 5 ];
   double p1lo   = epar[ 6 ];
   double p1hi   = epar[ 7 ];
   double p2lo   = epar[ 8 ];
   double p2hi   = epar[ 9 ];
   int    noisfl = (int)epar[ 10 ];
   int    dbg_lv = (int)epar[ 11 ];
   double ystart = par1;
   double slope  = par2;
   double yend   = ystart + slope * ( xend - xstart );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      ylow         -= 0.1;
      yhigh        += 0.1;
      p1lo         -= 0.1;
      p1hi         += 0.1;
      p2lo         -= 0.01;
      p2hi         += 0.02;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           ystart < ylow   ||  yend   < ylow  ||
           ystart > yhigh  ||  yend   > yhigh )
         return 1e+99;
   }

   double prange = (double)( nlpts - 1 );
   double xinc   = ( xend - xstart ) / prange;
   double yinc   = ( yend - ystart ) / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double xcurr  = xstart;
   double ycurr  = ystart;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      sim_vals.solutes << US_Solute( xcurr * 1e-13, ycurr, 0.0, vbar20 );
      xcurr        += xinc;
      ycurr        += yinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 12 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffSL: call" << ffcall << "par1 par2 t" << par1 << par2 << t
 << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffSL:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];
//qDebug() << "ffSL:  dsets[0]" << dsets[0] << parP[0]
// << "dsets[0]->vbar20" << dsets[0]->vbar20;
qDebug() << "ffSL:    ys ye sl yl yh" << ystart << yend << slope
 << ylow << yhigh;

   return rmsd;
}

// Do curve-fit evaluate function (return RMSD) for a Increasing Sigmoid model
double US_1dsaProcess::fit_function_IS( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 13 ];     // Static array for holding parameters

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
      for ( int ii = 0; ii < 12; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double xstart = epar[ 2 ];
   double xend   = epar[ 3 ];
   double ylow   = epar[ 4 ];
   double yhigh  = epar[ 5 ];
   double p1lo   = epar[ 6 ];
   double p1hi   = epar[ 7 ];
   double p2lo   = epar[ 8 ];
   double p2hi   = epar[ 9 ];
   int    noisfl = (int)epar[ 10 ];
   int    dbg_lv = (int)epar[ 11 ];
   double kstr   = ylow;
   double kdif   = yhigh - ylow;
   double srange = xend - xstart;
   double p1fac  = sqrt( 2.0 * par1 );
   double ystart = kstr + kdif * ( 0.5 + erf( ( 0.0 - par2 ) / p1fac ) + 0.5 );
   double yend   = kstr + kdif * ( 0.5 + erf( ( 1.0 - par2 ) / p1fac ) + 0.5 );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      ylow         -= 0.1;
      yhigh        += 0.1;
      p1lo         -= 0.1;
      p1hi         += 0.1;
      p2lo         -= 0.01;
      p2hi         += 0.02;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           ystart < ylow   ||  yend   < ylow  ||
           ystart > yhigh  ||  yend   > yhigh )
         return 1e+99;
   }

   double prange = (double)( nlpts - 1 );
   double xinc   = 1.0 / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double xcurr  = xstart;
   double ycurr  = ystart;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;

   double xval   = 0.0;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      double efac   = 0.5 * erf( ( xval - par2 ) / p1fac ) + 0.5;
      xcurr         = xstart + xval * srange;
      ycurr         = kstr   + kdif * efac;
      sim_vals.solutes << US_Solute( xcurr * 1e-13, ycurr, 0.0, vbar20 );
      xval         += xinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 12 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffIS: call" << ffcall << "par1 par2 t" << par1 << par2 << t
 << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffIS:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];

   return rmsd;
}

// Do curve-fit evaluate function (return RMSD) for a Decreasing Sigmoid model
double US_1dsaProcess::fit_function_DS( double t, double* par )
{
   static int ffcall=0;          // Fit function call counter
   static double epar[ 13 ];     // Static array for holding parameters

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
      for ( int ii = 0; ii < 12; ii++ )
         epar[ ii ]    = par[ ii + 2 ];
      parP[ 0 ]     = iparP[ px ];
   }

   ffcall++;                                    // Bump function call counter
   dsets << (US_SolveSim::DataSet*)parP[ 0 ];
   int    nlpts  = (int)epar[ 1 ];              // Get limit parameters
   double xstart = epar[ 2 ];
   double xend   = epar[ 3 ];
   double ylow   = epar[ 4 ];
   double yhigh  = epar[ 5 ];
   double p1lo   = epar[ 6 ];
   double p1hi   = epar[ 7 ];
   double p2lo   = epar[ 8 ];
   double p2hi   = epar[ 9 ];
   int    noisfl = (int)epar[ 10 ];
   int    dbg_lv = (int)epar[ 11 ];
   double kstr   = yhigh;
   double kdif   = ylow - yhigh;
   double srange = xend - xstart;
   double p1fac  = sqrt( 2.0 * par1 );
   double ystart = kstr + kdif * ( 0.5 + erf( ( 0.0 - par2 ) / p1fac ) + 0.5 );
   double yend   = kstr + kdif * ( 0.5 + erf( ( 1.0 - par2 ) / p1fac ) + 0.5 );

   // After 1st few calls, test if parameters are within limits
   if ( ffcall > 3 )
   {
      // Leave a little wiggle room on limits
      ylow         -= 0.1;
      yhigh        += 0.1;
      p1lo         -= 0.1;
      p1hi         += 0.1;
      p2lo         -= 0.01;
      p2hi         += 0.02;

      // If this record is beyond any limit, return now with it marked as bad
      if ( par1   < p1lo   ||  par2   < p2lo  ||
           par1   > p1hi   ||  par2   > p2hi  ||
           ystart < ylow   ||  yend   < ylow  ||
           ystart > yhigh  ||  yend   > yhigh )
         return 1e+99;
   }

   double prange = (double)( nlpts - 1 );
   double xinc   = 1.0 / prange;
   double vbar20 = dsets[ 0 ]->vbar20;
   double xcurr  = xstart;
   double ycurr  = ystart;
   US_SolveSim::Simulation sim_vals;
   sim_vals.noisflag  = noisfl;
   sim_vals.dbg_level = dbg_lv;

   double xval   = 0.0;

   for ( int ii = 0; ii < nlpts; ii++ )
   { // Fill the input solutes vector
      double efac   = 0.5 * erf( ( xval - par2 ) / p1fac ) + 0.5;
      xcurr         = xstart + xval * srange;
      ycurr         = kstr   + kdif * efac;
      sim_vals.solutes << US_Solute( xcurr * 1e-13, ycurr, 0.0, vbar20 );
      xval         += xinc;
   }

   // Evaluate the model
   double rmsd   = evaluate_model( dsets, sim_vals );

   epar[ 12 ]    = rmsd;
   int    ktimms = ftimer.elapsed();
qDebug() << "ffDS: call" << ffcall << "par1 par2 t" << par1 << par2 << t
 << "eval time" << ktimms << "ms.";
if(ffcall<6)
qDebug() << "ffDS:  epar0 epar1-9" << parP[0] << epar[1] << epar[2] << epar[3]
 << epar[4] << epar[5] << epar[6] << epar[7] << epar[8] << epar[9];

   return rmsd;
}

// Do Levenberg-Marquardt fit
void US_1dsaProcess::LevMarq_fit( void )
{
//   static US_LM::LM_Control control;
//   static US_LM::LM_Control control( 1.e-7, 1.e-7, 1.e-7, 1.e-7,
//                                     100., 100, 0, 0 );
   static US_LM::LM_Control control( 1.e-5, 1.e-5, 1.e-5, 1.e-5 );
   static US_LM::LM_Status  status;
   static int    n_par  = 2;
   static int    m_dat  = 3;
DbgLv(1) << "LMf: n_par m_dat" << n_par << m_dat;
   static double tarray[ 3 ] = { 0.0, 1.0, 2.0 };
   static double yarray[ 3 ] = { 0.0, 0.0, 0.0 };
   static double parray[ 20 ];
   double minkv  = kuplim;
   double maxkv  = klolim;
   double maxsl  = ( kuplim - klolim ) / ( suplim - slolim );
   double minsl  = -maxsl;
   double minp1  = minkv;
   double maxp1  = maxkv;
   double minp2  = maxsl;
   double maxp2  = minsl;
   lm_done       = false;
   kcsteps       = 0;
   int    stepms = 500;
   nctotal       = ( curvtype == 0 )
                   ? ( time_fg * 3 ) / ( stepms * 2 )
                   : ( time_fg * 1 ) / ( stepms * 8 );
   lmtm_id       = startTimer( stepms );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   emit message_update( tr( "\nRefining best model with Levenberg-Marquardt"
                            " fit ...\n" ), true );

   elite_limits( mrecs, minkv, maxkv, minp1, maxp1, minp2, maxp2 );

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
   par[ 6 ]      = minkv;
   par[ 7 ]      = maxkv;
   par[ 8 ]      = minp1;
   par[ 9 ]      = maxp1;
   par[ 10 ]     = minp2;
   par[ 11 ]     = maxp2;
   par[ 12 ]     = noisflag;
   par[ 13 ]     = dbg_level;
DbgLv(1) << "LMf:  ppar2" << ppar[4] << dsets[0] << curvtype << ppar[5];
   timer.start();              // start a timer to measure run time

   if ( curvtype == 0 )
   { // Fit with Levenberg-Marquardt for straight-line curves
DbgLv(0) << "lmcurve_fit (SL) with par1,par2" << par[0] << par[1];
      control.ftol     = 1.e-5;
      control.xtol     = 1.e-5;
      control.gtol     = 1.e-5;
      control.epsilon  = 1.e-5;
      fit_function_SL( -1.0, par );    // Make sure to reset eval. function

      US_LM::lmcurve_fit_rmsd( n_par, par, m_dat, t, y,
            &(US_1dsaProcess::fit_function_SL), &control, &status );
DbgLv(0) << "  lmcurve_fit (SL) return: par1,par2" << par[0] << par[1];
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info;
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
      control.ftol     = 1.e-3;
      control.xtol     = 1.e-3;
      control.gtol     = 1.e-3;
      control.epsilon  = 1.e-3;
      par[ 8 ]      = 0.001;
      par[ 9 ]      = 0.5;
      par[ 10 ]     = 0.0;
      par[ 11 ]     = 1.0;
      fit_function_IS( -1.0, par );    // Make sure to reset eval. function

DbgLv(0) << "lmcurve_fit (IS) with par1,par2" << par[0] << par[1];
      US_LM::lmcurve_fit_rmsd( n_par, par, m_dat, t, y,
            &(US_1dsaProcess::fit_function_IS), &control, &status );
DbgLv(0) << "  lmcurve_fit (IS) return: par1,par2" << par[0] << par[1];
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info;
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
      par[ 8 ]      = 0.001;
      par[ 9 ]      = 0.5;
      par[ 10 ]     = 0.0;
      par[ 11 ]     = 1.0;
      fit_function_DS( -1.0, par );    // Make sure to reset eval. function

DbgLv(0) << "lmcurve_fit (DS) with par1,par2" << par[0] << par[1];
      US_LM::lmcurve_fit_rmsd( n_par, par, m_dat, t, y,
            &(US_1dsaProcess::fit_function_DS), &control, &status );
DbgLv(0) << "  lmcurve_fit (DS) return: par1,par2" << par[0] << par[1];
DbgLv(0) << "   lmcfit status: fnorm nfev info"
   << status.fnorm << status.nfev << status.info;
double rmsd = sqrt( dsets[0]->model.variance );
int    nsol = dsets[0]->model.components.size();
DbgLv(0) << "   lmcfit rmsd" << rmsd << "#solutes" << nsol; 
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
//   timefs ktimes = ( timer.elapsed() + 500 ) / 1000;
   time_lm       = timer.elapsed();
   int    ktimes = ( time_lm + 500 ) / 1000;
   int    ktimeh = ktimes / 3600;
   int    ktimem = ( ktimes - ktimeh * 3600 ) / 60;
   ktimes        = ktimes - ktimeh * 3600 - ktimem * 60;
DbgLv(0) << "  lmcfit time:" << ktimeh << "h" << ktimem << "m" << ktimes << "s";
   QString fmsg = tr( "New best model has par1 %1,  par2 %2,\n"
                      "  RMSD %3,  %4 solutes  " )
       .arg( par[ 0 ] ).arg( par[ 1 ] ).arg( rmsd ).arg( nsol );
   if ( ktimeh == 0 )
      fmsg      = fmsg + tr( "(%1 min., %2 sec.)" )
                         .arg( ktimem ).arg( ktimes );
   else
      fmsg      = fmsg + tr( "(%1 hr., %2 min., %3 sec.)" )
                         .arg( ktimeh ).arg( ktimem ).arg( ktimes );
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

   mrec.par1      = par[ 0 ];
   mrec.par2      = par[ 1 ];
   mrec.variance  = dset->model.variance;
   mrec.rmsd      = rmsd;

   for ( int ii = 0; ii < mrec.isolutes.size(); ii++ )
   { // Replace s and k in top model input solutes
      mrec.isolutes[ ii ].s = spar->mesh_radius[ jsp++ ];
      mrec.isolutes[ ii ].k = spar->mesh_radius[ jsp++ ];
   }

   mrec.csolutes.clear();
   model      = dset->model;
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
   US_DataIO2::RawData* simdat = &mrec.sim_data;
   US_DataIO2::RawData* resids = &mrec.residuals;
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

   // Fetch any noise saved in dset
   bool tino       = ( ( noisflag & 1 ) != 0 );
   bool rino       = ( ( noisflag & 2 ) != 0 );
   ti_noise.count  = 0;
   ri_noise.count  = 0;
   ti_noise.values.clear();
   ri_noise.values.clear();
   mrec.ti_noise.clear();
   mrec.ri_noise.clear();

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
   mrecs.insert( 0, mrec );

   // Re-compute simulation and residuals
DbgLv(0) << "LMf: tino rino" << tino << rino;
DbgLv(0) << "LMf: simdat nsc npt"
 << simdat->scanData.size() << simdat->x.size();
DbgLv(0) << "LMf: resids nsc npt"
 << resids->scanData.size() << resids->x.size();
DbgLv(0) << "LMf: rdata  nsc npt"
 << rdata.scanData.size() << rdata.x.size();
DbgLv(0) << "LMf: sdata  nsc npt"
 << sdata.scanData.size() << sdata.x.size();
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
         rdata.  scanData[ ss ].readings[ rr ].value = resval;

         simdat->scanData[ ss ].readings[ rr ].value = sdata.value( ss, rr );
         resids->scanData[ ss ].readings[ rr ].value = resval;
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

void US_1dsaProcess::elite_limits( QVector< ModelRecord >& mrecs,
      double& minkv, double& maxkv, double& minp1, double& maxp1,
      double& minp2, double& maxp2 )
{
   int nmr        = mrecs.size();
   int nelite     = ( nmr * 2 ) / 10;
DbgLv(1) << " ElLim: nmr nelite nmtasks" << nmr << nelite << nmtasks;
DbgLv(1) << " ElLim: in minkv maxkv" << minkv << maxkv;
DbgLv(1) << " ElLim: in min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;

   for ( int ii = 0; ii < nelite; ii++ )
   {
if(ii<3||(ii+4)>nelite)
DbgLv(1) << " ElLim:   ii" << ii << "par1 par2"
 << mrecs[ii].par1 << mrecs[ii].par2 << "str_k" << mrecs[ii].str_k;
      minkv          = qMin( minkv, mrecs[ ii ].str_k );
      maxkv          = qMax( maxkv, mrecs[ ii ].str_k );
      minkv          = qMin( minkv, mrecs[ ii ].end_k );
      maxkv          = qMax( maxkv, mrecs[ ii ].end_k );
      minp1          = qMin( minp1, mrecs[ ii ].par1  );
      maxp1          = qMax( maxp1, mrecs[ ii ].par1  );
      minp2          = qMin( minp2, mrecs[ ii ].par2  );
      maxp2          = qMax( maxp2, mrecs[ ii ].par2  );
   }
DbgLv(1) << " ElLim: out minkv maxkv" << minkv << maxkv;
DbgLv(1) << " ElLim: out min/max p1/p2" << minp1 << maxp1 << minp2 << maxp2;
}

// Evaluate a model; return rmsd, model, noises
double US_1dsaProcess::evaluate_model( QList< US_SolveSim::DataSet* >& dsets,
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
   dset->model.variance = sim_vals.variances[ 0 ];
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
   double rmsd   = sqrt( sim_vals.variances[ 0 ] );
   return rmsd;
}

// Protected slot to filter timer event and handle L-M status timing
void US_1dsaProcess::timerEvent( QTimerEvent *event )
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

