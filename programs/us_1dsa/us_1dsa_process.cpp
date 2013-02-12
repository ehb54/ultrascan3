//! \file us_1dsa_process.cpp
#include "us_1dsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_sleep.h"
#include "us_math2.h"
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
   mrecs    .clear();

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
   int ktimes = ( timer.elapsed() + 500 ) / 1000;
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
          "Threads:  %1 ;   Lines/Models:  %2\n"
          "Run time:  " )
      .arg( nthreads ).arg( nmtasks );

   if ( ktimeh > 0 )
      pmsg += tr( "%1 hr., %2 min., %3 sec.\n" )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      pmsg += tr( "%1 min., %2 sec.\n" )
         .arg( ktimem ).arg( ktimes );

   max_rss();
   int memmb  = qRound( (double)maxrss / 1024.0 );

   pmsg += tr( "Maximum memory used:  " ) +
           QString::number( memmb ) + " MB\n\n" +
           tr( "The best model (RMSD=%1) is at index %2 :\n" )
           .arg( mrec.rmsd ).arg( mrec.taskx );
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
DbgLv(1) << " GET_RES:   ti,ri counts" << ti_noise.count << ri_noise.count;
DbgLv(1) << " GET_RES:    VARI,RMSD" << mrecs[0].variance << mrecs[0].rmsd
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

