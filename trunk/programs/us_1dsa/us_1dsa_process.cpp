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
   bdata    = &dsets[ 0 ]->run_data;  // pointer to base experiment data
   edata            = bdata;          // working pointer to experiment
   parentw          = parent; 
   dbg_level        = US_Settings::us_debug();
   maxrss           = 0;              // max memory
   ntisols          = 0;              // number total task input solutes
   ntcsols          = 0;              // number total task computed solutes

   tkvaris  .clear();                 // task variances
   mrecs    .clear();                 // computed model records
   simparms = &dsets[ 0 ]->simparams; // pointer to simulation parameters

   nscans           = bdata->scanData.size();
   npoints          = bdata->x.size();
   cresolu          = 100;
   curvtype         = 0;
   nmtasks          = 100;
   kctask           = 0;
   kstask           = 0;
   varimin          = 99e+99;
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
   ntisols     = 0;
   ntcsols     = 0;

   wkstates .resize(  nthreads );
   wthreads .clear();
   job_queue.clear();
   c_solutes.clear();

   orig_sols.clear();
   tkvaris  .clear();
   mrecs    .clear();

DbgLv(1) << "2P: sll sul" << slolim << suplim
 << " kll kul kin" << klolim << kuplim << kincr
 << " type reso nth noi" << curvtype << cresolu << nthreads << noisflag;

   timer.start();              // start a timer to measure run time

   edata       = bdata;        // initial iteration base experiment data

   // experiment data dimensions
   nscans      = edata->scanData.size();
   npoints     = edata->x.size();

   if ( curvtype == 0 )
   {  // For straight-line curves, determine the models
      nmtasks     = slmodels( slolim, suplim, klolim, kuplim, kincr, cresolu );
   }
   else
      nmtasks     = 0;

   nctotal     = nmtasks;

   kcsteps     = 0;
   emit stage_complete( kcsteps, nctotal );
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nmtasks ) ? nthreads : nmtasks;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints;
DbgLv(1) << "2P:   nctotal nthreads" << nctotal << nthreads;
   max_rss();
DbgLv(1) << "2P: (1)maxrss" << maxrss;

   QList< QVector< US_Solute > > solute_list;

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
      tr( "Starting computations of %1 subgrids\n using %2 threads ..." )
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
   c_solutes.clear();
   ntisols   = 0;
   ntcsols   = 0;

   emit message_update( pmessage_head() +
      tr( "All computations have been aborted." ), false );
}

// Slot to handle the low-variance record of calculated solutes
void US_1dsaProcess::process_final( ModelRecord& mrec )
{
   if ( abort ) return;

   int nsolutes = mrec.csolutes.size();

   QVector< double > tinvec( npoints,  0.0 );
   QVector< double > rinvec( nscans,   0.0 );

   if ( ( noisflag & 1 ) != 0 )
   {  // copy TI noise to caller and internal vector
      ti_noise.minradius = edata->radius( 0 );
      ti_noise.maxradius = edata->radius( npoints - 1 );
      ti_noise.values.resize( npoints );
      ti_noise.count = npoints;

      for ( int rr = 0; rr < npoints; rr++ )
      {
         ti_noise.values[ rr ] = mrec.ti_noise[ rr ];
         tinvec         [ rr ] = mrec.ti_noise[ rr ];
      }
   }

   if ( ( noisflag & 2 ) != 0 )
   {  // copy RI noise to caller and internal vector
      ri_noise.values.resize( nscans );
      ri_noise.count = nscans;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         ri_noise.values[ ss ] = mrec.ri_noise[ ss ];
         rinvec         [ ss ] = mrec.ri_noise[ ss ];
      }
   }
DbgLv(1) << "FIN_FIN:    ti,ri counts" << ti_noise.count << ri_noise.count;

   US_SolveSim::DataSet* dset = dsets[ 0 ];
   double sfactor     = 1.0 / dset->s20w_correction;
   double dfactor     = 1.0 / dset->D20w_correction;
   double vbar20      = dset->vbar20;
DbgLv(1) << "FIN_FIN: s20w,D20w_corr" << dset->s20w_correction
 << dset->D20w_correction << "sfac dfac" << sfactor << dfactor;
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

DbgLv(1) << "FIN_FIN: vari" << mrec.variance;

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
           .arg( mrec.rmsd ).arg( mrec.taskx ) + 
           tr( "  the line from s,f/f0  %1, %2  to %3, %4 ." )
           .arg( slolim ).arg( mrec.str_k ).arg( suplim ).arg( mrec.end_k );

   emit message_update( pmsg, false );          // signal final message

DbgLv(1) << "FIN_FIN: Run Time: hr min sec" << ktimeh << ktimem << ktimes;
DbgLv(1) << "FIN_FIN: maxrss memmb nthr" << maxrss << memmb << nthreads
 << " nsubg noisf" << nmtasks << noisflag;
DbgLv(1) << "FIN_FIN:   kcsteps nctotal" << kcsteps << nctotal;
   model.message = QString( "BINDEX=%1" ).arg( mrec.taskx );

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
bool US_1dsaProcess::get_results( US_DataIO2::RawData* da_sim,
                                  US_DataIO2::RawData* da_res,
                                  US_Model*            da_mdl,
                                  US_Noise*            da_tin,
                                  US_Noise*            da_rin,
                                  int&                 bm_ndx )
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

DbgLv(1) << " GET_RES:   ti,ri counts" << ti_noise.count << ri_noise.count;
DbgLv(1) << " GET_RES:    VARI" << rdata.scanData[0].delta_r
 << da_res->scanData[0].delta_r;
   bm_ndx      = mrecs[ 0 ].taskx;
DbgLv(1) << " GET_RES:    BM_NDX" << bm_ndx;
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
   WorkPacket wresult;

   wthrd->get_result( wresult );   // get results of thread task
   int thrn   = wresult.thrn;      // thread number of task
   int thrx   = thrn - 1;          // index into thread list
   int taskx  = wresult.taskx;     // task index of task
DbgLv(1) << "PROCESS_JOB thrn" << thrn << "taskx" << taskx;
   int nrcso  = wresult.csolutes.size();
   ntcsols   += nrcso;
if ( taskx < 9 || taskx > (nmtasks-4) )
DbgLv(1) << "PJ: taskx csolutes size tot" << taskx << nrcso << ntcsols
   << QDateTime::currentDateTime().toMSecsSinceEpoch();
//DBG-CONC
if (dbg_level>0) for (int mm=0; mm<wresult.csolutes.size(); mm++ ) {
 if ( wresult.csolutes[mm].c > 100.0 ) {
   DbgLv(1) << "PJ:  CONC=" << wresult.csolutes[mm].c
    << " s,ff0" << wresult.csolutes[mm].s*1.0e+13
    << wresult.csolutes[mm].k; } }
//DBG-CONC
   double variance = wresult.sim_vals.variances[ 0 ];
   if ( variance < varimin )
   {
      varimin   = variance;
      minvarx   = taskx;
   }

   // Save variance and model record for this task result
   ModelRecord mrec;
   mrec.taskx      = taskx;
   mrec.str_k      = wresult.str_k;
   mrec.end_k      = wresult.end_k;
   mrec.variance   = variance;
   mrec.rmsd       = ( variance > 0.0 ) ? sqrt( variance ) : 99.9;
   mrec.isolutes   = wresult.isolutes;
   mrec.csolutes   = wresult.csolutes;
   mrec.ti_noise   = wresult.ti_noise;
   mrec.ri_noise   = wresult.ri_noise;
   mrec.sim_data   = wresult.sim_vals.sim_data;
   mrec.residuals  = wresult.sim_vals.residuals;

   tkvaris   << variance;
   c_solutes << mrec.csolutes;
   mrecs     << mrec;
DbgLv(1) << "THR_FIN:  taskx minvarx varimin" << taskx << minvarx << varimin;

   max_rss();

   free_worker( thrx );               // free up this worker thread

   if ( abort )
      return;

   kctask++;                          // bump count of completed subgrid tasks
   emit progress_update( variance );  // pass progress on to control window
DbgLv(1) << "THR_FIN: thrn" << thrn << " taskx" << taskx
 << " kct kst" << kctask << kstask << "csols size" << c_solutes.size();

   emit message_update( pmessage_head() +
      tr( "Computations for %1 of %2 models are complete." )
      .arg( kctask ).arg( nmtasks ), false );

   if ( kctask >= nmtasks )
   {  // all subgrid tasks are now complete
      emit stage_complete( kcsteps, nctotal );

      emit message_update( pmessage_head() +
         tr( "All models have now been completed. Evaluating..." ), false );
   }

   if ( kctask < nmtasks )
   { // Not the last:  add to the queue is necessary
      // Submit jobs while queue is not empty and a worker thread is ready
      while ( ! job_queue.isEmpty() &&
              ( thrx = wkstates.indexOf( READY ) ) >= 0 )
      {
         WorkPacket wtask = next_job();

         submit_job( wtask, thrx );
         kstask++;                       // bump count of started worker threads

      }
   }

   else
   { // We have done the last computation, so determine the low-rmsd result

      qSort( mrecs );                    // sort model records by variance

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
   int nrisols    = isolutes.size();
   ntisols       += nrisols;
if ( taskx < 9 || taskx > (nmtasks-4) )
DbgLv(1) << "QT: taskx" << taskx << " isolutes size tot" << nrisols << ntisols;

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
   QString ctype = tr( "Straight Line" );
   return tr( "Analysis of %1 %2 Models, with %3 solutes each.\n" )
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
   double srng     = sup - slo;
   double rinc     = 1.0 / ( res - 1 );
   double incs     = srng * rinc;
   int    nkpts    = qRound( ( kup - klo ) / kin ) + 1;
   int    nmodels  = nkpts * nkpts;
   orig_sols.clear();

   double kst      = klo;

   for ( int ii = 0; ii < nkpts; ii++ )
   {
      double ken      = klo;

      for ( int jj = 0; jj < nkpts; jj++ )
      {
         double inck     = ( ken - kst ) * rinc;
         QVector< US_Solute > solute_i;

         for ( int kk = 0; kk < res; kk++ )
         {
            US_Solute solu;
            double sval     = slo + incs * kk;
            double kval     = kst + inck * kk;
            solu.s          = sval * 1.0e-13;
            solu.k          = kval;
            solu.v          = vbar20;
            solute_i << solu;
DbgLv(1) << "SLMO:  ii jj kk" << ii << jj << kk << "s k" << solu.s << solu.k;
         }

DbgLv(1) << "SLMO:   solute_i size" <<  solute_i.size();
         orig_sols << solute_i;
         ken            += kin;
      }
      kst            += kin;
   }
DbgLv(1) << "SLMO:  orig_sols size" << orig_sols.size();

   return nmodels;
}
