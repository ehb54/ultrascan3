//! \file us_2dsa_process.cpp
#include "us_2dsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_constants.h"

#include <sys/user.h>

// Class to process 2DSA simulations
US_2dsaProcess::US_2dsaProcess( US_DataIO2::EditedData* da_exper,
   US_SimulationParameters* sim_pars, QObject* parent /*=0*/ )
   : QObject( parent )
{
   edata            = da_exper;
   simparms         = sim_pars;
   parentw          = parent;
   dbg_level        = US_Settings::us_debug();
   maxrss           = 0;
}

long int US_2dsaProcess::max_rss( void )
{
   // Read /prod/$pid/stat
   QFile f( "/proc/" + QString::number( getpid() ) + "/stat" );
   f.open( QIODevice::ReadOnly );
   QByteArray ba = f.read( 512 );
   f.close();

   const static int kk = PAGE_SIZE / 1024;

   maxrss = max( maxrss, QString( ba ).section( " ", 23, 23 ).toLong() * kk );

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
   ngrefine    = ngr;
   nthreads    = nthr;
   noisflag    = noif;
   errMsg      = tr( "NO ERROR: start" );
   maxrss      = 0;

   wthreads.clear();
   worktsks.clear();
   workouts.clear();

DbgLv(1) << "2P: sll sul nss" << slolim << suplim << nssteps
 << " kll kul nks" << klolim << kuplim << nksteps
 << " ngref nthr noif" << ngrefine << nthreads << noisflag;

   timer.start();              // start a timer to measure run time

   // data dimensions
   nscans      = edata->scanData.size();
   npoints     = edata->x.size();
   // grid deltas     (overall increment between points)
   gdelta_s    = ( suplim - slolim ) / (double)( nssteps - 1 );
   gdelta_k    = ( kuplim - klolim ) / (double)( nksteps - 1 );
   // subgrid deltas  (increments between subgrid points)
   sdelta_s    = gdelta_s * (double)ngrefine;
   sdelta_k    = gdelta_k * (double)ngrefine;

   nsubgrid    = ngrefine * ngrefine;
   int kksubg  = nksteps * nssteps;
   int kkcsol  = kksubg / 8;
   int kknnls  = kkcsol + kkcsol / 50;
   if ( noisflag > 0 )
      kknnls     += ( sq( kkcsol ) / 10 );
   nctotal     = kksubg + kkcsol + kknnls + 10;

   kcsteps     = 0;
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nsubgrid ) ? nthreads : nsubgrid;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints << " gdlts gdltk"
 << gdelta_s << gdelta_k << " sdlts sdltk" << sdelta_s << sdelta_k;
DbgLv(1) << "2P:   nsubgrid nctotal nthreads"
 << nsubgrid << nctotal << nthreads;
   max_rss();
DbgLv(1) << "2P: (1)maxrss" << maxrss;

   int    ktask = 0;
   double llss  = slolim;

   // define work unit parameters, including solutes for each subgrid
   for ( int ii = 0; ii < ngrefine; ii++ )
   {
      double llsk = klolim;

      for ( int jj = 0; jj < ngrefine; jj++ )
      {
         WorkPacket wdef;
         wdef.thrx      = 0;
         wdef.taskx     = ++ktask;       // task index
         wdef.noisf     = 0;             // no noise for initial passes
         wdef.typeref   = UGRID;         // uniform grid type
         wdef.state     = READY;         // initialized state, ready for job
         wdef.depth     = 0;             // depth 0:  subgrid calcs
         wdef.iter      = 1;             // iteration 1
         wdef.ll_s      = llss;          // lower limit s
         wdef.ll_k      = llsk;          // lower limit k
         wdef.edata     = edata;         // pointer to experiment data
         wdef.sparms    = simparms;      // pointer to simulation parameters
                                         // solutes for subgrid
         wdef.isolutes  = create_solutes( llss, suplim, sdelta_s,
                                          llsk, kuplim, sdelta_k );
         worktsks << wdef;

         llsk          += gdelta_k;
      }

      llss     += gdelta_s;
   }

   // Start the first threads. This will begin the first work units (subgrids).
   // Thereafter, work units are started in new threads when threads signal
   // that they have completed their work.
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkPacket*   wdef = &worktsks[ ii ];
      WorkerThread* wthr = new WorkerThread( this );
      wthreads << wthr;
      wdef->thrx         = ii + 1;

      wthr->define_work( *wdef );

      connect( wthr, SIGNAL( work_complete(   WorkerThread* ) ),
               this, SLOT(   thread_finished( WorkerThread* ) ) );
      connect( wthr, SIGNAL( work_progress(   int           ) ),
               this, SLOT(   step_progress  ( int           ) ) );

      wthr->start();
   }

   max_rss();
   kstask = nthreads;     // count of started tasks is initially thread count

   emit message_update(
      tr( "Starting computations of %1 subgrids\n using %2 threads ..." )
      .arg( nsubgrid ).arg( nthreads ), false );
}

// abort a fit run
void US_2dsaProcess::stop_fit()
{
   abort   = true;

   for ( int ii = 0; ii < wthreads.size(); ii++ )
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

   wthreads.clear();
   worktsks.clear();
   workouts.clear();

   emit message_update( tr( "All computations have been aborted." ), false );
}

// Slot for thread step progress:  signal control/main progress bars
void US_2dsaProcess::step_progress( int ksteps )
{
   max_rss();
   kcsteps   += ksteps;
   ksteps     = ( ksteps < 2 ) ? 1 : ksteps;
   emit progress_update( ksteps );   // pass progress on to control and main
}

// Slot to handle a worker thread having finished. Accumulate computed solutes.
// If more work left, start a new thread for a new work unit.
void US_2dsaProcess::thread_finished( WorkerThread* wthrd )
{
   if ( abort ) return;

   WorkPacket wresult;

   wthrd->get_result( wresult );   // get results of thread task
   int thrx   = wresult.thrx;      // thread index of task
   int taskx  = wresult.taskx;     // task index of task

   max_rss();

   if ( kctask == 0 )
      c_solutes.clear();

   c_solutes << wresult.csolutes;  // build composite of all computed solutes

   kctask++;                       // bump count of completed tasks (subgrids)
DbgLv(1) << "THR_FIN: thrx" << thrx << " taskx" << taskx << " ll_s ll_k"
 << wresult.ll_s << wresult.ll_k << " kct kst" << kctask << kstask;
   int tx = thrx - 1;              // get index into thread list
   delete wthreads[ tx ];          // destroy thread
   wthreads[ tx ] = 0;

   emit refine_complete( kctask ); 

   emit message_update( 
      tr( "Computations for %1 of %2 subgrids are complete" )
      .arg( kctask ).arg( nsubgrid ), false );

   if ( kctask >= nsubgrid )
   {  // all subgrids computed

      final_computes();             // proceed to do final computations
      return;
   }

   if ( kstask > ( nsubgrid - 1 ) )
   {  // no more tasks need to be started
      return;
   }

   // get next task definition and create new thread
   WorkPacket*   wdef = &worktsks[ kstask ];
   WorkerThread* wthr = new WorkerThread( this );

   wthreads[ tx ]     = wthr;      // set up next thread
   wdef->thrx         = thrx;      // define its index (same as one just done)
   wthr->define_work( *wdef );     // define the work

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT( thread_finished( WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress( int           ) ),
            this, SLOT( step_progress  ( int           ) ) );

   wthr->start();                  // start a new worker thread
   kstask++;                       // bump count of started worker threads
}

// Use a worker thread one last time to compute, using all computed solutes
void US_2dsaProcess::final_computes()
{
   if ( abort ) return;

   max_rss();
   WorkPacket* wdef = &worktsks[ 0 ];
   wdef->taskx   = -1;          // special task index signalling final task
   wdef->noisf   = noisflag;    // in this case, we use the noise flag

   int nsolest   = ( nksteps * nssteps ) / 8;   // solutes estimated
   int nsolact   = c_solutes.size();            // solutes actually computed
   int todoest   = nsolest + nsolest / 50;      // count to-do estimated
   int todoact   = nsolact + nsolact / 50;      // count to-do actual
   if ( noisflag > 0 )
   {
      todoest      += ( sq( nsolest ) / 10 );   // if noise, a lot more!
      todoact      += ( sq( nsolact ) / 10 );
   }
   todoest      += 10;
   todoact      += 10;

DbgLv(1) << "FinalComp:   (est)kcp ntp nsol ntodo" <<  kcsteps << nctotal
 << nsolest << todoest;
   // adjust the estimate of total progress steps
   nctotal       = kcsteps + todoact;    
DbgLv(1) << "FinalComp:   (new)kcp ntp nsol ntodo" <<  kcsteps << nctotal
 << nsolact << todoact;

   emit subgrids_complete( kcsteps, nctotal );


   // This time, input solutes are all the subgrid-computed ones where
   // the concentration is positive.
   qSort( c_solutes );
   wdef->isolutes.clear();

   for ( int ii = 0; ii < c_solutes.size(); ii++ )
      wdef->isolutes << c_solutes[ ii ];
DbgLv(1) << "FinalComp: szSoluC" << wdef->isolutes.size();

   WorkerThread* wthr = new WorkerThread( this );
   wthr->define_work( *wdef );

   connect( wthr, SIGNAL( work_complete(  WorkerThread* ) ),
            this, SLOT(   final_finished( WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress(  int           ) ),
            this, SLOT(   step_progress(  int           ) ) );

   QString pmsg = tr( "Computing final NNLS ..." );
   emit message_update( pmsg, false );

   wthreads[ 0 ] = wthr;
   wthr->start( );
}

// Final pass to use composite computed solutes
void US_2dsaProcess::final_finished( WorkerThread* wthrd )
{
   if ( abort ) return;

   WorkPacket wresult;

   wthrd->get_result( wresult );  // get results of thread task
DbgLv(1) << "FIN_FIN: thrx taskx ll_s ll_k"
   << wresult.thrx << wresult.taskx << wresult.ll_s << wresult.ll_k;

   c_solutes = wresult.csolutes;
DbgLv(1) << "FIN_FIN:    c_sol size" << c_solutes.size();

   QVector< double > tinvec( npoints,  0.0 );
   QVector< double > rinvec( nscans,   0.0 );

   if ( ( noisflag & 1 ) != 0 )
   {
      ti_noise.values.resize( npoints );
      ti_noise.count = npoints;

      for ( int rr = 0; rr < npoints; rr++ )
      {
         ti_noise.values[ rr ] = wresult.ti_noise[ rr ];
         tinvec         [ rr ] = wresult.ti_noise[ rr ];
      }
   }

   if ( ( noisflag & 2 ) != 0 )
   {
      ri_noise.values.resize( nscans );
      ri_noise.count = nscans;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         ri_noise.values[ ss ] = wresult.ri_noise[ ss ];
         rinvec         [ ss ] = wresult.ri_noise[ ss ];
      }
   }
DbgLv(1) << "FIN_FIN:    ti,ri counts" << ti_noise.count << ri_noise.count;

   QString dvv      = edata->dataType;
   double density   = dvv.section( " ", 1, 1 ).toDouble();
   double viscosity = dvv.section( " ", 2, 2 ).toDouble();
   double vbar      = dvv.section( " ", 3, 3 ).toDouble();
   double avgtemp   = 0.0;
   for ( int ii = 0; ii < nscans; ii++ )
         avgtemp += edata->scanData[ ii ].temperature;
   avgtemp /= (double)nscans;

   US_Math2::SolutionData solution;
   solution.density   = density;
   solution.viscosity = viscosity;
   solution.vbar20    = vbar;
   solution.vbar      = vbar;
   US_Math2::data_correction( avgtemp, solution );
   double sfactor     = 1.0e-13 / solution.s20w_correction;
   int    nsolutes    = c_solutes.size();
   model.components.resize( nsolutes );

   // build the final model
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      US_Model::SimulationComponent mcomp;
      mcomp.s     = qAbs( c_solutes[ cc ].s ) * sfactor;
      mcomp.D     = 0.0;
      mcomp.mw    = 0.0;
      mcomp.f     = 0.0;
      mcomp.f_f0  = c_solutes[ cc ].k;
      mcomp.signal_concentration
                  = c_solutes[ cc ].c;

      model.components[ cc ]  = mcomp;
   }

   model.update_coefficients();
DbgLv(1) << "FIN_FIN:    c0 cn" << c_solutes[0].c << c_solutes[nsolutes-1].c;

   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_Astfem_RSA astfem_rsa( model, *simparms );

   // calculate the simulation data
   astfem_rsa.calculate( sdata );

   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   double vsum      = 0.0;

   // build residuals data set (experiment minus simulation minus any noise)
   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double rval = edata->value( ss, rr ) - sdata.value( ss, rr )
                       - tinvec[ rr ] - rinvec[ ss ];
         vsum       += ( rval * rval );

         rdata.scanData[ ss ].readings[ rr ] = US_DataIO2::Reading( rval );
      }
   }

   // communicate variance through scan0's delta_r value
   rdata.scanData[ 0 ].delta_r = vsum / (double)( nscans * npoints );

   // determine elapsed time
   int ktimes = ( timer.elapsed() + 500 ) / 1000;
   int ktimeh = ktimes / 3600;
   int ktimem = ( ktimes - ktimeh * 3600 ) / 60;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // compose final status message
   QString pmsg =
      tr( "The Solution has converged...\n"
          "Iterations:  1\n"
          "Threads:  %1 ;   Subgrids:  %2\n"
          "Grid points (s x f/f0):  %3 x %4\n"
          "Run time:  " )
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

   emit message_update( pmsg, false );  // signal final message
   emit process_complete();             // signal that processing is complete

   delete wthreads[ 0 ];                // destroy thread
   wthreads[ 0 ]  = 0;
DbgLv(1) << "FIN_FIN: maxrss memmb nthr nsubg nssteps nksteps noisf"
 << maxrss << memmb << nthreads << nsubgrid << nssteps << nksteps << noisflag;
}

// Get results upon completion of all refinements
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
   return all_ok;
}

// Build solutes vector for a subgrid
QVector< Solute > US_2dsaProcess::create_solutes(
   double ll_s, double ul_s, double delta_s,
   double ll_k, double ul_k, double delta_k )
{
   QVector< Solute > solu;

   for ( double sval = ll_s; sval <= ul_s; sval += delta_s )
      for ( double kval = ll_k; kval <= ul_k; kval += delta_k )
         solu << Solute( sval, kval );

   return solu;
}

