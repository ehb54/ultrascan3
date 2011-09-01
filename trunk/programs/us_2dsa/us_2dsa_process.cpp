//! \file us_2dsa_process.cpp
#include "us_2dsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_constants.h"

#ifdef Q_WS_MAC
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#endif
#ifndef Q_WS_WIN
#include <sys/user.h>
#else
#include <windows.h>
#include <psapi.h>
#include <process.h>
#endif

// Class to process 2DSA simulations
US_2dsaProcess::US_2dsaProcess( QList< US_SolveSim::DataSet* >& dsets,
   QObject* parent /*=0*/ ) : QObject( parent ), dsets( dsets )
{
   bdata    = &dsets[ 0 ]->run_data;  // pointer to base experiment data
   edata            = bdata;          // working pointer to experiment
   parentw          = parent; 
   dbg_level        = US_Settings::us_debug();
   maxrss           = 0;              // max memory
   maxdepth         = 0;              // maximum depth index of tasks
   ntisols          = 0;              // number total task input solutes
   ntcsols          = 0;              // number total task computed solutes
   varitol          = 1e-12;          // variance difference tolerance
   r_iter           = 0;              // refinement iteration index
   mm_iter          = 0;              // meniscus/MC iteration index
   mintsols         = 100;            // minimum solutes per task, depth 1 ff.
   maxiters         = 1;              // maximum refinement iterations
   mmtype           = 0;              // meniscus/montecarlo type (NONE)
   mmiters          = 0;              // meniscus/montecarlo iterations
   fnoionly         = US_Settings::debug_match( "2dsaFinalNoiseOnly" );

   itvaris  .clear();                 // iteration variances
   ical_sols.clear();                 // iteration final calculated solutes
   simparms = &dsets[ 0 ]->simparams; // pointer to simulation parameters
}

// Get maximum used memory
long int US_2dsaProcess::max_rss( void )
{
#ifdef Q_WS_X11         // Unix: based on /proc/$PID/stat
   // Read /proc/$pid/stat
   QFile f( "/proc/" + QString::number( getpid() ) + "/stat" );
   f.open( QIODevice::ReadOnly );
   QByteArray ba = f.read( 512 );
   f.close();

   const static int kk = PAGE_SIZE / 1024;

   maxrss = max( maxrss, QString( ba ).section( " ", 23, 23 ).toLong() * kk );
#endif

#ifdef Q_WS_MAC         // Mac : use task_info call
   struct task_basic_info task_stats;
   mach_msg_type_number_t inf_count = TASK_BASIC_INFO_COUNT;
   task_t   task    = current_task();
   long int usedmem = 0;

   int stat1 = task_info( task, TASK_BASIC_INFO, (task_info_t)&task_stats, &inf_count );
   if ( stat1 == KERN_SUCCESS )
   {
      usedmem = ( (int64_t)task_stats.resident_size + 512 ) / 1024;
DbgLv(1) << "2P(2dsaProc): usedmem" << usedmem;
   }

   maxrss = max( maxrss, usedmem );
#endif

#ifdef Q_WS_WIN         // Windows: direct use of GetProcessMemoryInfo
//#ifdef NEVER
   HANDLE hProcess;
   DWORD processID;
   PROCESS_MEMORY_COUNTERS pmc;
   pmc.cb       = (DWORD)sizeof( pmc );
   processID    = _getpid();
   long int usedmem = 0;

   hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                           FALSE, processID );
   if ( hProcess == NULL )
      return maxrss;

   if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof( pmc ) ) )
   {
      usedmem = ( (int64_t)pmc.PeakWorkingSetSize + 512 ) / 1024;
   }

   maxrss = max( maxrss, (int)usedmem );
   CloseHandle( hProcess );
#endif

   return maxrss;
}

// Start a specified 2DSA fit run
void US_2dsaProcess::start_fit( double sll, double sul,  int nss,
                                double kll, double kul,  int nks,
                                int    ngr, int    nthr, int noif )
{
DbgLv(1) << "2P(2dsaProc): start_fit()";
   abort       = false;
   slolim      = sll;
   suplim      = sul;
   nssteps     = nss;
   klolim      = kll;
   kuplim      = kul;
   nksteps     = nks;
   jgrefine    = ngr;
   ngrefine    = ngr;
   nthreads    = nthr;
   noisflag    = noif;
   errMsg      = tr( "NO ERROR: start" );
   maxrss      = 0;
   maxdepth    = 0;
   ntisols     = 0;
   ntcsols     = 0;
   r_iter      = 0;
   mm_iter     = 0;

   if ( jgrefine < 0 )
   {  // Special model-grid or model-ratio grid refinement
      ngrefine    = 1;
      nthreads    = 1;
      model       = dsets[ 0 ]->model;
      slolim      = model.components[ 0 ].s * 1.0e+13;
      klolim      = model.components[ 0 ].f_f0;
      nksteps     = model.components.size();
      int kk      = qMax( 1, nksteps ) - 1;
      suplim      = model.components[ kk ].s * 1.0e+13;
      kuplim      = model.components[ kk ].f_f0;
      nssteps     = qMax( 1, nssteps );
      nssteps     = (int)sqrt( (double)nksteps );
      nssteps     = qMax( 1, nssteps );
      nksteps     = nksteps / nssteps;
      nksteps     = qMax( 1, nksteps );
   }

   wkstates .resize(  nthreads );
   wkdepths .resize(  nthreads );
   wthreads .clear();
   tkdepths .clear();
   job_queue.clear();
   c_solutes.clear();

   orig_sols.clear();
   itvaris  .clear();
   ical_sols.clear();

DbgLv(1) << "2P: sll sul nss" << slolim << suplim << nssteps
 << " kll kul nks" << klolim << kuplim << nksteps
 << " ngref nthr noif" << ngrefine << nthreads << noisflag;

   timer.start();              // start a timer to measure run time

   edata       = bdata;        // initial iteration base experiment data

   if ( mmtype > 0 )
   {  // for meniscus or monte carlo, make a working copy of the base data
      wdata = *bdata;

      if ( mmtype == 1 )
      {  // if meniscus, use the start meniscus value
         edata              = &wdata;
         double bmeniscus   = bdata->meniscus;
         edata->meniscus    = bmeniscus - menrange * 0.5;
         dsets[ 0 ]->simparams.meniscus = edata->meniscus;
DbgLv(1) << "MENISC: mm_iter meniscus bmeniscus"
 << mm_iter << edata->meniscus << bmeniscus;
      }
   }

   // experiment data dimensions
   nscans      = edata->scanData.size();
   npoints     = edata->x.size();
   // subgrid deltas  (increments between subgrid points)
   int nsubp_s = ( nssteps + ngrefine - 1 ) / ngrefine;
   int nsubp_k = ( nksteps + ngrefine - 1 ) / ngrefine;
   sdelta_s    = ( suplim - slolim ) / (double)( max( nsubp_s - 1, 1 ) );
   sdelta_k    = ( kuplim - klolim ) / (double)( max( nsubp_k - 1, 1 ) );
   // grid deltas     (overall increment between grid points)
   gdelta_s    = sdelta_s / (double)ngrefine;
   gdelta_k    = sdelta_k / (double)ngrefine;
   nsubgrid    = sq( ngrefine );
   int kgref   = ngrefine - 1;
   int kgrefsq = sq( kgref );
   int kksubg  = nksteps * nssteps
                 - ( kgref + kgrefsq ) * ( nsubp_s + nsubp_k ) + kgrefsq;
DbgLv(1) << "2P:    kgref kgrefsq kksubg" << kgref << kgrefsq << kksubg;
   maxtsols    = nsubp_s * nsubp_k;
   maxtsols    = max( maxtsols, mintsols );

   int ktcsol  = maxtsols - 5;
   int nnstep  = ( noisflag > 0 ? ( sq( ktcsol ) / 10 + 2 ) : 2 ) * nsubgrid;
   nctotal     = kksubg + nnstep + estimate_steps( ( kksubg / 8 ) );

   kcsteps     = 0;
   emit stage_complete( kcsteps, nctotal );
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nsubgrid ) ? nthreads : nsubgrid;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints;
DbgLv(1) << "2P:   gdelta_s gdelta_k" << gdelta_s << gdelta_k
 << " sdelta_s sdelta_k" << sdelta_s << sdelta_k;
DbgLv(1) << "2P:   nsubgrid nctotal nthreads maxtsols"
 << nsubgrid << nctotal << nthreads << maxtsols;
   max_rss();
DbgLv(1) << "2P: (1)maxrss" << maxrss << "jgrefine" << jgrefine;

   int    ktask = 0;
   int    jdpth = 0;
   int    jnois = fnoionly ? 0 : noisflag;
   QList< QVector< US_Solute > > solute_list;
   double ssllim = slolim * 1.0e-13;
   double ssulim = suplim * 1.0e-13;

   // Generate the original sub-grid solutes list
   if ( jgrefine > 0 )
   {
      US_Solute::init_solutes( ssllim, ssulim, nssteps,
                               klolim, kuplim, nksteps, ngrefine, orig_sols );
   }

   else if ( jgrefine == (-1) )
   {  // model-grid
      QVector< US_Solute > solvec;

      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         US_Solute soli( model.components[ ii ].s,
                         model.components[ ii ].f_f0,
                         0.0 );
DbgLv(0) << "ii" << ii << "soli" << soli.s << soli.k << soli.c;
         solvec << soli;
      }

      orig_sols << solvec;
   }

   else if ( jgrefine == (-2) )
   {  // model-ratio
      QVector< US_Solute > solvec;

      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         US_Solute soli( model.components[ ii ].s,
                         model.components[ ii ].f_f0,
                         model.components[ ii ].signal_concentration );
DbgLv(0) << "ii" << ii << "soli" << soli.s << soli.k << soli.c;
         solvec << soli;
      }

      orig_sols << solvec;
   }

   // Queue all the depth-0 tasks
   for ( int ii = 0; ii < sq( ngrefine ); ii++ )
   {
      WorkPacket wtask;
      double llss = orig_sols[ ii ][ 0 ].s;
      double llsk = orig_sols[ ii ][ 0 ].k;

      queue_task( wtask, llss, llsk, ktask++, jdpth, jnois, orig_sols[ ii ] );

      maxtsols       = max( maxtsols, wtask.isolutes.size() );
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
      .arg( nsubgrid ).arg( nthreads ), false );
}

// Set iteration parameters
void US_2dsaProcess::set_iters( int    mxiter, int    mciter, int    mniter,
                                double vtoler, double menrng, double cff0 )
{
   maxiters   = mxiter;
   mmtype     = ( mniter > 1 ) ? 1 : 0;
   mmtype     = ( mciter > 1 ) ? 2 : mmtype;
   mmiters    = ( mmtype == 0 ) ? 0 : max( mciter, mniter );
   varitol    = vtoler;
   menrange   = menrng;
   cnstff0    = cff0;

   if ( cnstff0 > 0.0 )
   {
      dsets[ 0 ]->vbartb = -cnstff0;
   }
}

// Abort a fit run
void US_2dsaProcess::stop_fit()
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

      wthreads[ ii ] = 0;
   }

   job_queue.clear();
   tkdepths .clear();
   c_solutes.clear();
   maxdepth  = 0;
   ntisols   = 0;
   ntcsols   = 0;

   emit message_update( pmessage_head() +
      tr( "All computations have been aborted." ), false );
}

// Slot for thread step progress:  signal control progress bar
void US_2dsaProcess::step_progress( int ksteps )
{
   max_rss();
   kcsteps   += ksteps;              // bump completed steps count
//DbgLv(2) << "StpPr: ks kcs    " << ksteps << kcsteps;

   emit progress_update( ksteps );   // pass progress on to control window
}

// Use a worker thread one last time to compute, using all computed solutes
void US_2dsaProcess::final_computes()
{
   if ( abort ) return;

   max_rss();

   WorkPacket wtask;

   int depth      = maxdepth++;
   wtask.thrn     = 0;
   wtask.taskx    = tkdepths.size();
   wtask.depth    = maxdepth;
   wtask.noisf    = noisflag;    // in this case, we use the noise flag
   wtask.dsets    = dsets;
   wtask.csolutes.clear();
   wtask.ti_noise.clear();
   wtask.ri_noise.clear();

   tkdepths << wtask.depth;

   // This time, input solutes are all the subgrid-computed ones where
   // the concentration is positive.
   qSort( c_solutes[ depth ] );
DbgLv(1) << "FinalComp: szSoluC" << c_solutes[ depth ].size();
   wtask.isolutes.clear();

   for ( int ii = 0; ii < c_solutes[ depth ].size(); ii++ )
   {
      if ( c_solutes[ depth ][ ii ].c > 0.0 )
         wtask.isolutes << c_solutes[ depth ][ ii ];
   }
DbgLv(1) << "FinalComp: szSoluI" << wtask.isolutes.size();

   c_solutes[ depth ].clear();

   WorkerThread* wthr = new WorkerThread( this );

   int thrx = wkstates.indexOf( READY );
   while ( thrx < 0 )
   {
      US_Sleep::msleep( 1 );
      thrx = wkstates.indexOf( READY );
   }

   wtask.thrn = thrx + 1;
   wthr->define_work( wtask );

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT(   process_final( WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress( int           ) ),
            this, SLOT(   step_progress( int           ) ) );

   emit message_update( pmessage_head() + tr( "Computing final NNLS ..." ),
      false );

DbgLv(1) << "(FC)SUBMIT_JOB taskx" << wtask.taskx << "depth" << wtask.depth;
   wthreads[ thrx ] = wthr;
   wthr->start( );
}

// Slot to handle output of final pass on composite calculated solutes
void US_2dsaProcess::process_final( WorkerThread* wthrd )
{
   if ( abort ) return;

   WorkPacket wresult;

   wthrd->get_result( wresult );     // get results of thread task
DbgLv(1) << "(FF)PROCESS_JOB thrn" << wresult.thrn << "taskx" << wresult.taskx
 << "depth" << wresult.depth;

   if ( c_solutes.size() < ( maxdepth + 1 ) )
      c_solutes << QVector< US_Solute >();

   c_solutes[ maxdepth ] =  wresult.csolutes;  // final iter calc'd solutes
   int nsolutes = c_solutes[ maxdepth ].size();
DbgLv(1) << "FIN_FIN:    c_sol size" << nsolutes;

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
         ti_noise.values[ rr ] = wresult.ti_noise[ rr ];
         tinvec         [ rr ] = wresult.ti_noise[ rr ];
      }
   }

   if ( ( noisflag & 2 ) != 0 )
   {  // copy RI noise to caller and internal vector
      ri_noise.values.resize( nscans );
      ri_noise.count = nscans;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         ri_noise.values[ ss ] = wresult.ri_noise[ ss ];
         rinvec         [ ss ] = wresult.ri_noise[ ss ];
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
   qSort( c_solutes[ maxdepth ] );
   //double bf_mult     = simparms->band_forming
   //                   ? simparms->cp_width
   //                   : 1.0;

   // build the final model

   if ( cnstff0 == 0.0 )
   {  // Normal case of varying f/f0
      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         // Get standard-space solute values (20,W)
         US_Model::SimulationComponent mcomp;
         mcomp.vbar20 = vbar20;
         mcomp.s      = c_solutes[ maxdepth ][ cc ].s;
         mcomp.D      = 0.0;
         mcomp.mw     = 0.0;
         mcomp.f      = 0.0;
         mcomp.f_f0   = c_solutes[ maxdepth ][ cc ].k;
         //mcomp.signal_concentration
         //             = c_solutes[ maxdepth ][ cc ].c * bf_mult;
         mcomp.signal_concentration
                      = c_solutes[ maxdepth ][ cc ].c;

         // Complete other coefficients in standard-space
         model.calc_coefficients( mcomp );
DbgLv(1) << " Bcc comp D" << mcomp.D;

         // Convert to experiment-space for simulation below
         mcomp.s     *= sfactor;
         mcomp.D     *= dfactor;
DbgLv(1) << "  Bcc 20w comp D" << mcomp.D;

         model.components[ cc ]  = mcomp;
      }
   }  // Constant vbar

   else
   {  // Special case of varying vbar
      US_Math2::SolutionData sd;
      sd.viscosity  = dset->viscosity;
      sd.density    = dset->density;
      double avtemp = dset->temperature;

      for ( int cc = 0; cc < nsolutes; cc++ )
      {
         // Get standard-space solute values (20,W)
         US_Model::SimulationComponent mcomp;
         mcomp.vbar20 = c_solutes[ maxdepth ][ cc ].k;
         mcomp.s      = c_solutes[ maxdepth ][ cc ].s;
         mcomp.D      = 0.0;
         mcomp.mw     = 0.0;
         mcomp.f      = 0.0;
         mcomp.f_f0   = cnstff0;
         mcomp.signal_concentration
                      = c_solutes[ maxdepth ][ cc ].c;

         // Complete other coefficients in standard-space
         model.calc_coefficients( mcomp );
DbgLv(1) << " Bcc comp D" << mcomp.D;

         // Convert to experiment-space for simulation below
         sd.vbar20    = mcomp.vbar20;
         sd.vbar      = US_Math2::adjust_vbar20( sd.vbar20, avtemp );
         US_Math2::data_correction( avtemp, sd );

         mcomp.s     /= sd.s20w_correction;
         mcomp.D     /= sd.D20w_correction;
DbgLv(1) << "  Bcc 20w comp D" << mcomp.D;

         model.components[ cc ]  = mcomp;
      }
   }  // Constant f/f0

DbgLv(1) << "FIN_FIN:    c0 cn" << c_solutes[maxdepth][0].c
 << c_solutes[maxdepth][nsolutes-1].c;
   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   double vari      = wresult.sim_vals.variances[ 0 ];
DbgLv(1) << "FIN_FIN: vari" << vari;
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_DataIO2::RawData* simdat = &wresult.sim_vals.sim_data;
   US_DataIO2::RawData* resids = &wresult.sim_vals.residuals;

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

   // set variance and communicate to control through residual's scan 0
   itvaris   << vari;
   ical_sols << c_solutes[ maxdepth ];
   US_DataIO2::Scan* rscan0 = &rdata.scanData[ 0 ];
   rscan0->delta_r    = vari;
   rscan0->rpm        = (double)( r_iter + 1 );
   rscan0->seconds    = ( mmtype == 0 ) ? 0.0 : (double)( mm_iter + 1 );
   rscan0->plateau    = ( mmtype != 1 ) ? 0.0 : edata->meniscus;
DbgLv(1) << "FIN_FIN: vari riter miter menisc" << rscan0->delta_r
 << rscan0->rpm << rscan0->seconds << rscan0->plateau;

   // determine elapsed time
   int ktimes = ( timer.elapsed() + 500 ) / 1000;
   int ktimeh = ktimes / 3600;
   int ktimem = ( ktimes - ktimeh * 3600 ) / 60;
double bvol = dsets[0]->simparams.band_volume;
bvol=dsets[0]->simparams.band_forming?bvol:0.0;
DbgLv(1) << "done: vari bvol" << vari << bvol
 << "slun klun ngr" << slolim << suplim << nssteps << klolim << kuplim
 << nksteps << ngrefine << "ets" << ktimes;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // compose final iteration status message
   QString pmsg = pmessage_head() +
      tr( "The Solution has converged...\n"
          "Iterations:  %1\n"
          "Threads:  %2 ;   Subgrids:  %3\n"
          "Grid points (s x f/f0):  %4 x %5\n"
          "Run time:  " )
      .arg( r_iter + 1 )
      .arg( nthreads ).arg( nsubgrid ).arg( nssteps ).arg( nksteps );

   if ( ktimeh > 0 )
      pmsg += tr( "%1 hr., %2 min., %3 sec.\n" )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      pmsg += tr( "%1 min., %2 sec.\n" )
         .arg( ktimem ).arg( ktimes );

   max_rss();
   double memmb  = (double)maxrss / 1024.0;

   pmsg += tr( "Maximum memory used:  " )
           + QString().sprintf( "%.1f", memmb ) + " MB";

   emit message_update( pmsg, false );          // signal final message

   int thrx   = wresult.thrn - 1;
   free_worker( thrx );
DbgLv(1) << "FIN_FIN: Run Time: hr min sec" << ktimeh << ktimem << ktimes;
DbgLv(1) << "FIN_FIN: maxrss memmb nthr" << maxrss << memmb << nthreads
 << " nsubg nsst nkst noisf" << nsubgrid << nssteps << nksteps << noisflag;
DbgLv(1) << "FIN_FIN:   kcsteps nctotal" << kcsteps << nctotal;

   bool   neediter = false;       // need-more-iterations false by default

   if ( ( r_iter + 1 ) < maxiters )
   {  // possibly iterate if not yet at maximum iterations

      if ( r_iter < 1 )
      {  // if max not 2 or more, we must do at least 2 iterations to compare
         neediter     = true;
      }

      else
      {  // otherwise, we must compare solutes and variance difference
         int    jc     = r_iter;
         int    jp     = r_iter - 1;
         double pvari  = itvaris[   jp ];
         double dvari  = fabs( pvari - vari );
         int    nccsol = ical_sols[ jc ].size();
         int    npcsol = ical_sols[ jp ].size();
         bool   sdiffs = true;

         if ( nccsol == npcsol )
         {  // determine if calculated solutes match previous in s and k
            sdiffs       = false;
            for ( int jj = 0; jj < nccsol; jj++ )
            {
               if ( ical_sols[ jc ][ jj ] != ical_sols[ jp ][ jj ] )
               {  // if any mismatch, we may need to iterate
                  sdiffs       = true;
                  break;
               }
            }
         }

         // iterate if solutes different and variance change large enough
         neediter     = ( sdiffs  &&  dvari > varitol ); 
DbgLv(1) << "FIN_FIN: neediter" << neediter << "  sdiffs" << sdiffs
 << " dvari varitol" << dvari << varitol << " iter" << r_iter;
      }
   }

   if ( neediter )
   {  // we must iterate
      emit process_complete( 0 );       // signal that iteration is complete
      iterate();                        // reset to run another iteration
      return;
   }

   // Convert model components s,D back to 20,w form for output
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
DbgLv(1) << "cc comp D" << model.components[ cc ].D;
      model.components[ cc ].s *= dset->s20w_correction;
      model.components[ cc ].D *= dset->D20w_correction;
DbgLv(1) << " cc 20w comp D" << model.components[ cc ].D;
   }

   // done with iterations:   check for meniscus or MC iteration

   if ( mmtype > 0  &&  ++mm_iter < mmiters )
   {  // doing meniscus or monte carlo and more to do
      emit process_complete( mmtype );  // signal that iteration is complete

      if ( mmtype == 1 )
         set_meniscus();

      else if ( mmtype == 2 )
         set_monteCarlo();

      return;
   }

   emit process_complete( 9 );     // signal that all processing is complete
}

// Public slot to get results upon completion of all refinements
bool US_2dsaProcess::get_results( US_DataIO2::RawData* da_sim,
                                  US_DataIO2::RawData* da_res,
                                  US_Model*            da_mdl,
                                  US_Noise*            da_tin,
                                  US_Noise*            da_rin )
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
   return all_ok;
}

// Submit a job
void US_2dsaProcess::submit_job( WorkPacket& wtask, int thrx )
{
   wtask.thrn         = thrx + 1;

   WorkerThread* wthr = new WorkerThread( this );
   wthreads[ thrx ]   = wthr;
   wkstates[ thrx ]   = WORKING;
   wkdepths[ thrx ]   = wtask.depth;

   wthr->define_work( wtask );

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT(   process_job(   WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress( int           ) ),
            this, SLOT(   step_progress( int           ) ) );
DbgLv(1) << "SUBMIT_JOB taskx" << wtask.taskx << "depth" << wtask.depth;

   wthr->start();
}

// Slot to handle the results of a just-completed worker thread.
// Accumulate computed solutes.
// If there is more work to do, start a new thread for a new work unit.
void US_2dsaProcess::process_job( WorkerThread* wthrd )
{
   WorkPacket wresult;

   wthrd->get_result( wresult );   // get results of thread task
   int thrn   = wresult.thrn;      // thread number of task
   int thrx   = thrn - 1;          // index into thread list
   int taskx  = wresult.taskx;     // task index of task
   int depth  = wresult.depth;     // depth of result
DbgLv(1) << "PROCESS_JOB thrn" << thrn << "taskx" << taskx
 << "depth" << wresult.depth;
   int nrcso  = wresult.csolutes.size();
   ntcsols   += nrcso;
if ( taskx < 9 || taskx > (nsubgrid-4) )
DbgLv(1) << "PJ: taskx csolutes size tot" << taskx << nrcso << ntcsols;
//DBG-CONC
if (dbg_level>0) for (int mm=0; mm<wresult.csolutes.size(); mm++ ) {
 if ( wresult.csolutes[mm].c > 100.0 ) {
   DbgLv(1) << "PJ:  CONC=" << wresult.csolutes[mm].c
    << " s,ff0" << wresult.csolutes[mm].s*1.0e+13
    << wresult.csolutes[mm].k; } }
//DBG-CONC

   max_rss();

   free_worker( thrx );            // free up this worker thread

   if ( abort )
      return;

   // This loop should only execute, at most, once per result
   while( c_solutes.size() < ( depth + 1 ) )
      c_solutes << QVector< US_Solute >();

   int nextc    = c_solutes[ depth ].size() + wresult.csolutes.size();
   int jnois    = fnoionly ? 0 : noisflag;

   if ( nextc > maxtsols )
   {  // if new solutes push count over limit, queue a job at next depth
      WorkPacket wtask = wresult;
      int taskx    = tkdepths.size();
      int depthn   = depth + 1;

      queue_task( wtask, slolim, klolim, taskx, depthn, jnois,
                  c_solutes[ depth ] );

      maxdepth     = max( maxdepth, depthn );
      c_solutes[ depth ].clear();
DbgLv(1) << "THR_FIN: depth" << wtask.depth << " #solutes"
 << wtask.isolutes.size() << " nextc maxtsols" << nextc << maxtsols
 << "wres#sols" << wresult.csolutes.size();;
   }

   if ( depth == 0 )
   {  // this result is from depth 0, it is from initial subgrids pass
      kctask++;                      // bump count of completed subgrid tasks
DbgLv(1) << "THR_FIN: thrn" << thrn << " taskx" << taskx
 << " kct kst" << kctask << kstask << "csols size" << c_solutes.size();

      emit message_update( pmessage_head() +
         tr( "Computations for %1 of %2 subgrids are complete" )
         .arg( kctask ).arg( nsubgrid ), false );

      if ( kctask == nsubgrid )
      {  // all subgrid tasks are now complete
         if ( r_iter == 0 )
         {  // in 1st iteration, re-estimate total progress steps
            int nsolest   = ( nksteps * nssteps ) / 8 ;  // solutes estimated
            int nsolact   = ntcsols;                     // actually computed

            int todoest   = estimate_steps( nsolest );
            int todoact   = estimate_steps( nsolact );

DbgLv(1) << "THR_FIN:   (est)kcst ncto" <<  kcsteps << nctotal
 << " nsol ntodo" << nsolest << todoest;
            // adjust the estimate of total progress steps
            nctotal       = kcsteps + todoact;
DbgLv(1) << "THR_FIN:   (new)kcst ncto" <<  kcsteps << nctotal
 << " nsol ntodo" << nsolact << todoact;
         }

         emit stage_complete( kcsteps, nctotal );

         emit message_update( pmessage_head() +
            tr( "Computing depth 1 solutions and beyond ..." ), false );

         maxdepth       = 1;

         if ( nextc <= maxtsols  &&  jobs_at_depth( 1 ) == 0 )
            maxdepth       = 0;  // handle no depth 1 jobs yet submitted
      }
   }

   // Add the current results
   c_solutes[ depth ] += wresult.csolutes;

   // At this point we need to clean up. For each depth below the current one,
   // if there is nothing in the queue or working and there are calculated
   // solutes left, those need to be submitted

   for ( int dd = 0; dd < depth; dd++ )
   {
      if ( jobs_at_depth( dd ) == 0  &&  c_solutes[ dd ].size() > 0 )
      {  // queue a task to handle last solutes at this depth
         WorkPacket wtask = wresult;
         int taskx    = tkdepths.size();
         int depthn   = dd + 1;
DbgLv(1) << "THR_FIN:    QT: taskx depth solsz" << taskx << depth << c_solutes[dd].size();
         queue_task( wtask, slolim, klolim, taskx, depthn, jnois,
                     c_solutes[ dd ] );

         c_solutes[ dd ].clear();
      }
   }

   // Is anyone working?
   bool no_working = wkstates.indexOf( WORKING ) < 0;
DbgLv(0) << "THR_FIN: nowk dep mxdp cssz wrsz" << no_working << depth
 << maxdepth << c_solutes[depth].size() << wresult.csolutes.size();

   // Submit one last time with all solutes if necessary
   if ( depth == maxdepth    &&
        job_queue.isEmpty()  &&
        no_working           &&
        //c_solutes[ depth ].size() > wresult.csolutes.size() )
        c_solutes[ depth ].size() >= wresult.csolutes.size() )
   {
      final_computes();
      return;
   }

   // Submit jobs while queue is not empty and a worker thread is ready
   while ( ! job_queue.isEmpty() && ( thrx = wkstates.indexOf( READY ) ) >= 0 )
   {
      WorkPacket wtask = job_queue.takeFirst();

      submit_job( wtask, thrx );
      kstask++;                       // bump count of started worker threads

      if ( wtask.depth > 1  &&
           wtask.taskx == tkdepths.indexOf( wtask.depth ) )
      {  // submitting the first job of a depth pass
         QString pmsg = tr( "Computing depth %1 solutions..." )
                        .arg( wtask.depth );
         emit message_update( pmsg, true );
DbgLv(1) << pmsg;
      }
   }

}

// Build a task and add it to the queue
void US_2dsaProcess::queue_task( WorkPacket& wtask, double llss, double llsk,
      int taskx, int depth, int noisf, QVector< US_Solute > isolutes )
{
   wtask.thrn     = 0;             // thread number (none while queued)
   wtask.taskx    = taskx;         // task index
   wtask.noisf    = noisf;         // noise flag
   wtask.typeref  = UGRID;         // refinement type: uniform grid
   wtask.state    = READY;         // initialized state, ready for submit
   wtask.depth    = depth;         // depth index
   wtask.iter     = r_iter;        // refine-iteration index
   wtask.ll_s     = llss;          // lower limit s (x 1e13)
   wtask.ll_k     = llsk;          // lower limit k
   wtask.dsets    = dsets;         // pointer to experiment data
   wtask.isolutes = isolutes;      // solutes for calc_residuals task

   if ( jgrefine == (-2) )
      wtask.typeref  = jgrefine;   // mark if model-ratio grid refinement

   wtask.csolutes.clear();         // clear output vectors
   wtask.ti_noise.clear();
   wtask.ri_noise.clear();
   int nrisols    = isolutes.size();
   ntisols       += nrisols;
if ( taskx < 9 || taskx > (nsubgrid-4) )
DbgLv(1) << "QT: taskx isolutes size tot" << taskx << nrisols << ntisols;

   tkdepths << depth;              // record work task depth

   job_queue << wtask;             // put the task on the queue

   if ( tkdepths.count( depth ) == 1 )
   {  // if first task at this depth, report it
      emit message_update( tr( "Submitting first depth %1 calculations..." )
                           .arg( depth ), true );
DbgLv(1) << "Submit 1st calcs, depth" << depth;
   }
}

// queue up jobs for a new iteration
void US_2dsaProcess::iterate()
{
   r_iter++;                         // bump iteration index

   tkdepths .clear();
   job_queue.clear();
   QVector< US_Solute > csolutes = c_solutes[ maxdepth ];

   int    ncsol = csolutes.size();   // number of solutes calculated last iter

   for ( int ii = 0; ii < ncsol; ii++ )
      csolutes[ ii ].c = 0.0;        // clear concentration for compares
DbgLv(1) << "ITER: start of iteration" << r_iter+1;

   // Bump total steps estimate based on additional solutes in subgrids
   nctotal      = ( r_iter == 1 ) ? nctotal : ( ( nctotal + kcsteps ) / 2 );
   nctotal      = max( kcsteps, nctotal ) + 10;
DbgLv(1) << "ITER:   r-iter0 ncto ncsol" << nctotal << ncsol;
   int ktisol   = maxtsols - 5;
   int ktask    = nsubgrid + 1;
   int kstep    = ( r_iter == 1 ) ? ( ncsol + 8 ) : 4;
   int knois    = ktask * ( ( kstep * ktisol + sq( kstep ) ) / 10 );
DbgLv(1) << "ITER:    nsubg ktask kstep knois" << nsubgrid << ktask << kstep << knois;
   int kadd     = ( ktask * kstep ) + ( ( noisflag == 0 ) ? 0 : knois );
   nctotal     += kadd;
DbgLv(1) << "ITER:   r-iter1 ncto (diff)" << nctotal << kadd;

   kcsteps      = 0;
   emit stage_complete( kcsteps, nctotal );
   kctask       = 0;
   kstask       = 0;
   maxdepth     = 0;
   ntisols      = 0;
   ntcsols      = 0;
   max_rss();

   ktask        = 0;                 // task index
   int    jdpth = 0;
   int    jnois = fnoionly ? 0 : noisflag;
   double llss  = slolim;            // lower limit s to identify each subgrid

   // Build and queue the subgrid tasks

   for ( int ii = 0; ii < ngrefine; ii++ )
   {
      double llsk  = klolim;         // lower limit k to identify each subgrid

      for ( int jj = 0; jj < ngrefine; jj++ )
      {
         // get the solutes originally created for this subgrid
         QVector< US_Solute > isolutes = orig_sols[ ktask ];

         // add any calculated solutes not already in subgrid
         for ( int cc = 0; cc < ncsol; cc++ )
            if ( ! isolutes.contains( csolutes[ cc ] ) )
               isolutes << csolutes[ cc ];
//*DEBUG
int kosz=orig_sols[ktask].size();
int kasz=isolutes.size();
int kadd=kasz-kosz;
if ( kadd < ncsol )
DbgLv(1) << "ITER: kt" << ktask << "iterate nisol o a c +"
 << kosz << kasz << ncsol << kadd;
//*DEBUG

         // queue a subgrid task and update maximum task solute count
         WorkPacket wtask;
         queue_task( wtask, llss, llsk, ktask++, jdpth, jnois, isolutes );

         maxtsols       = max( maxtsols, isolutes.size() );
         llsk          += gdelta_k;
      }

      llss     += gdelta_s;
   }

   // Make sure calculated solutes are cleared out for new iteration
   for ( int ii = 0; ii < c_solutes.size(); ii++ )
      c_solutes[ ii ].clear();

   // Start the first threads. This will begin the first work units (subgrids).
   // Thereafter, work units are started in new threads when threads signal
   // that they have completed their work.
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkPacket wtask = job_queue.takeFirst();
      submit_job( wtask, ii );
   }

   max_rss();
   kstask = nthreads;     // count of started tasks is initially thread count

   emit message_update(
      tr( "Starting iteration %1 computations of %2 subgrids\n"
          "  using %3 threads ..." )
      .arg( r_iter + 1 ).arg( nsubgrid ).arg( nthreads ), true );

}

// Free up a worker thread
void US_2dsaProcess::free_worker( int tx )
{
   if ( tx >= 0  &&  tx < nthreads )
   {
      if ( wthreads[ tx ] != 0 )
         delete wthreads[ tx ];       // destroy thread

      wthreads[ tx ] = 0;             // set thread pointer to null
      wkstates[ tx ] = READY;         // mark its slot as available
      wkdepths[ tx ] = -1;            // mark no valid depth yet for worker
   }
}

// Estimate progress steps after depth 0 from given total of calculated solutes.
//   For maxtsols=100, estimate 95 solutes per task
//   Calculate tasks needed for given depth 0 calculated steps
//   Bump a bit for NNLS and any noise
//   Multiply steps per task times number of tasks
//     Repeat for subsequent depths, assuming calculated solutes 1/8 of input
int US_2dsaProcess::estimate_steps( int ncsol )
{
   // Estimate number of solutes and steps per task
   int ktisol  = maxtsols - 5;
   int ktcsol  = ktisol / 8 + 1;
   int ktstep  = ktisol + ( ( noisflag > 0 ) ? ( sq( ktisol ) / 10 + 2 ) : 2 );
DbgLv(1) << "ES: ncsol ktisol ktcsol ktstep" << ncsol << ktisol
 << ktcsol << ktstep;

   // Estimate number of depth 1 tasks, solutes,and steps
   int n1task  = ( ncsol + ktisol / 2 ) / ktisol + 1;
   int ntasks  = n1task + 1;
   int n1csol  = n1task * ktcsol;
DbgLv(1) << "ES: D1 n1task n1csol" << n1task << n1csol;

   // Estimate number of solutes for depths beyond 1
   n1task      = ( n1csol + ktisol / 2 ) / ktisol + 1;
   ntasks     += n1task;
   n1csol      = n1task * ktcsol;
DbgLv(1) << "ES: D2 n1task n1csol" << n1task << n1csol;

   while ( n1task >  1 )
   {  // Sum in steps for depth 3 and following until just 1 task left
      n1task      = ( n1csol + ktisol / 2 ) / ktisol + 1;
      ntasks     += n1task;
      n1csol      = n1task * ktcsol;
DbgLv(1) << "ES:  D3ff n1task n1csol" << n1task << n1csol;
   }

   // Return estimate of remaining steps
DbgLv(1) << "ES: returned nsteps ntasks" << (ntasks*ktstep) << ntasks;
   return ( ntasks * ktstep );
}

// count queued jobs at a given depth
int US_2dsaProcess::queued_at_depth( int depth )
{
   int count = 0;

   for ( int ii = 0; ii < job_queue.size(); ii++ )
   {
      if ( job_queue[ ii ].depth == depth )
         count++;
   }

   return count;
}

// count running jobs at a given depth
int US_2dsaProcess::running_at_depth( int depth )
{
   int count = 0;

   for ( int ii = 0; ii < nthreads; ii++ )
   {
      if ( wkstates[ ii ] == WORKING  &&
           wkdepths[ ii ] == depth    &&
           wthreads[ ii ] != 0 )
         count++;
   }

   return count;
}

// count all queued and running jobs at a given depth
int US_2dsaProcess::jobs_at_depth( int depth )
{
   return queued_at_depth( depth ) + running_at_depth( depth );
}

// Set up for another meniscus pass
void US_2dsaProcess::set_meniscus()
{
   // Give the working data set an appropriate meniscus value
   double bmeniscus   = bdata->meniscus;
   double mendelta    = menrange / (double)( mmiters - 1 );
   double smeniscus   = bmeniscus - menrange * 0.5;
   edata->meniscus    = smeniscus + (double)mm_iter * mendelta;
   simparms->meniscus = edata->meniscus;
DbgLv(1) << "MENISC: mm_iter meniscus" << mm_iter << edata->meniscus;

   // Re-queue all the original subgrid tasks

   requeue_tasks();
}

// Set up for another monte carlo pass
void US_2dsaProcess::set_monteCarlo()
{
DbgLv(1) << "MCARLO: mm_iter" << mm_iter;
   // Set up new data modified by a gaussian distribution (MC iteration 2 start)
   if ( mm_iter == 1 )
      set_gaussians();

   // Get a randomized variation of the cencentrations
   // Use a gaussian distribution with the residual as the standard deviation
   int kk = 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double variation = US_Math2::box_muller( 0.0, sigmas[ kk++ ] );
         wdata.scanData[ ss ].readings[ rr ] =
            US_DataIO2::Reading( sdata.value( ss, rr ) + variation );
      }
   }

   edata       = &wdata;
DbgLv(1) << "MCARLO: mm_iter" << mm_iter << " sigma0 c0 v0 cn vn"
 << sigmas[0] << bdata->value(0,0) << edata->value(0,0)
 << bdata->value(nscans-1,npoints-1) << edata->value(nscans-1,npoints-1);
   dsets[ 0 ]->run_data = wdata;

   // Re-queue all the original subgrid tasks

   requeue_tasks();
}

// Re-queue all the original subgrid tasks
void US_2dsaProcess::requeue_tasks()
{
   kcsteps      = 0;
   emit stage_complete( kcsteps, nctotal );

   int    ktask = 0;
   int    jdpth = 0;
   int    jnois = 0;
   double llss  = slolim;

   for ( int ii = 0; ii < ngrefine; ii++ )
   {
      double llsk  = klolim;

      for ( int jj = 0; jj < ngrefine; jj++ )
      {
         QVector< US_Solute > isolutes = orig_sols[ ktask ];
         WorkPacket wtask;
         queue_task( wtask, llss, llsk, ktask++, jdpth, jnois, isolutes );
         llsk          += gdelta_k;
      }
      llss     += gdelta_s;
   }

   // Make sure calculated solutes are cleared out for new iteration
   for ( int ii = 0; ii < c_solutes.size(); ii++ )
      c_solutes[ ii ].clear();

   // Start the first threads
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkPacket wtask = job_queue.takeFirst();
      submit_job( wtask, ii );
   }

   kstask = nthreads;
   kctask       = 0;
   maxdepth     = 0;
   ntisols      = 0;
   ntcsols      = 0;
   r_iter       = 0;

   emit message_update(
      tr( "Starting iteration %1 computations of %2 subgrids\n"
          "  using %3 threads ..." )
      .arg( r_iter + 1 ).arg( nsubgrid ).arg( nthreads ), true );
}

void US_2dsaProcess::set_gaussians()
{
   sigmas.clear();
DbgLv(1) << "MCARLO:setgau";

   for ( int ss = 0; ss < nscans; ss++ )
   {
      QVector< double > vv( npoints );

      for ( int rr = 0; rr < npoints; rr++ )
      {
         vv[ rr ] = fabs( rdata.value( ss, rr ) );
      }

      // Smooth using 5 points to the left and right of each point
if ( ss < 2 ) DbgLv(1) << "MCARLO:setgau:gausmoo vv9" << vv[9];
      US_Math2::gaussian_smoothing( vv, 5 );
if ( ss < 2 ) DbgLv(1) << "MCARLO:setgau:smoothd vv9" << vv[9];
      sigmas << vv;
   }
}

QString US_2dsaProcess::pmessage_head()
{
   return tr( "Model %1,  Iteration %2:\n" )
          .arg( mm_iter + 1 ).arg( r_iter + 1 );
}

