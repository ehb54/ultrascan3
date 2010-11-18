//! \file us_2dsa_process.cpp
#include "us_2dsa_process.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_sleep.h"
#include "us_math2.h"
#include "us_constants.h"

#include <sys/user.h>

// Class to process 2DSA simulations
US_2dsaProcess::US_2dsaProcess( US_DataIO2::EditedData* da_exper,
      QObject* parent /*=0*/ ) : QObject( parent )
{
   edata            = da_exper;
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
   workdefs.clear();
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
   ntpsteps    = nksteps * nssteps;

   if ( noisflag == 0 )
      ntpsteps    = ntpsteps * 8 / 3;

   else
      ntpsteps    = ntpsteps * 8;

   kcpsteps    = 0;
   simult      = 1;
   sidivi      = 1;
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nsubgrid ) ? nthreads : nsubgrid;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints << " gdlts gdltk"
 << gdelta_s << gdelta_k << " sdlts sdltk" << sdelta_s << sdelta_k;
DbgLv(1) << "2P:   nsubgrid ntpsteps nthreads"
 << nsubgrid << ntpsteps << nthreads;
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
         WorkDefine wdef;
         wdef.thrx      = 0;
         wdef.taskx     = ++ktask;       // task index
         wdef.noisf     = 0;             // no noise for initial passes
         wdef.ll_s      = llss;          // lower limit s
         wdef.ll_k      = llsk;          // lower limit k
         wdef.edata     = edata;         // pointer to experiment data
                                         // solutes for subgrid
         wdef.isolutes  = create_solutes( llss, suplim, sdelta_s,
                                          llsk, kuplim, sdelta_k );
         workdefs << wdef;

         llsk          += gdelta_k;
      }

      llss     += gdelta_s;
   }

   // Start the first threads. This will begin the first work units (subgrids).
   // Thereafter, work units are started in new threads when threads signal
   // that they have completed their work.
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkDefine*   wdef = &workdefs[ ii ];
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
      .arg( nsubgrid ).arg( nthreads ), true );
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
   workdefs.clear();
   workouts.clear();

   emit message_update( tr( "All computations have been aborted." ), false );
}

// Slot for thread step progress:  signal control/main progress bars
void US_2dsaProcess::step_progress( int ksteps )
{
   max_rss();
   kcpsteps  += ksteps;
   int osteps = ( ksteps * simult ) / sidivi;
   osteps     = ( osteps < 2 ) ? 1 : osteps;
   emit progress_update( osteps );   // pass progress on to control and main
}

// Slot to handle a worker thread having finished. Accumulate computed solutes.
// If more work left, start a new thread for a new work unit.
void US_2dsaProcess::thread_finished( WorkerThread* wthrd )
{
   if ( abort ) return;

   WorkResult wresult;

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
   WorkDefine*   wdef = &workdefs[ kstask ];
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

   emit subgrids_complete();

   max_rss();
   WorkDefine* wdef = &workdefs[ 0 ];
   wdef->taskx      = -1;          // special task index signalling final task
   wdef->noisf      = noisflag;    // in this case, we use the noise flag

   int nstprem      = ntpsteps - kcpsteps;  // steps remaining
   int nsolrem      = c_solutes.size();     // solutes remaining
   simult           = nstprem / 2;          // progress step incr. multiplier
   sidivi           = nsolrem;              // progress step incr. divisor
qDebug() << "FinalComp:   kcp ntp nstr nsor" << kcpsteps << ntpsteps
 << nstprem << nsolrem << "simult sidivi" << simult << sidivi;

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

   WorkResult wresult;

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

   US_SimulationParameters simpars;
   simpars.initFromData( NULL, *edata );
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_Astfem_RSA astfem_rsa( model, simpars );

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
//DbgLv(1) << "2P(WT):  Thread destructor";
   //mutex.lock();
   //condition.wakeOne();
   //mutex.unlock();

   wait();
DbgLv(1) << "2P(WT):   Thread destroyed";
}

// define work for a worker thread
void WorkerThread::define_work( WorkDefine& workin )
{
   thrx        = workin.thrx;
   taskx       = workin.taskx;
   edata       = workin.edata;
   noisflag    = workin.noisf;
   llim_s      = workin.ll_s;
   llim_k      = workin.ll_k;

   solute_i    = workin.isolutes;
}

// get results of a completed worker thread
void WorkerThread::get_result( WorkResult& workout )
{
   workout.thrx     = thrx;
   workout.taskx    = taskx;
   workout.ll_s     = llim_s;
   workout.ll_k     = llim_k;

   workout.csolutes = solute_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
}

// run the worker thread
void WorkerThread::run()
{
DbgLv(1) << "THR RUN: lls llk" << llim_s << llim_k;

   calc_residuals();

//DbgLv(1) << "  RUN call quit";
   quit();
//DbgLv(1) << "  RUN call exec";
   exec();
//DbgLv(1) << "  RUN return";

   emit work_complete( this );
}

void WorkerThread::flag_abort()
{
   abort      = true;
}

// do the real work of a thread:  subgrid solution from solutes set
void WorkerThread::calc_residuals()
{
   // set up for single-component model
   model.components.resize( 1 );
   US_Model::SimulationComponent zcomponent;
   zcomponent.s     = 0.0;
   zcomponent.D     = 0.0;
   zcomponent.mw    = 0.0;
   zcomponent.f     = 0.0;
   zcomponent.f_f0  = 0.0;

   // populate simulation parameters based on experiment data
   US_SimulationParameters simpars;
   simpars.initFromData( NULL, *edata );

   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   int nsolutes     = solute_i.size();
   int ntotal       = npoints * nscans;
   int navalues     = ntotal  * nsolutes;
   int increp       = nsolutes / 10;
       increp       = ( increp < 10 ) ? 10 : increp;
   int kstep        = 0;
   int lstep        = 0;
   int ntinois      = npoints;
   int nrinois      = nscans;

   QVector< double > nnls_a( navalues, 0.0 );
   QVector< double > nnls_b( ntotal,   0.0 );
   QVector< double > nnls_x( nsolutes, 0.0 );
   QVector< double > tinvec( ntinois,  0.0 );
   QVector< double > rinvec( nrinois,  0.0 );
DbgLv(1) << "   CR:na nb nx" << navalues << ntotal << nsolutes;

   QString dvv      = edata->dataType;
   double density   = dvv.section( " ", 1, 1 ).toDouble();
   double viscosity = dvv.section( " ", 2, 2 ).toDouble();
   double vbar      = dvv.section( " ", 3, 3 ).toDouble();
   double avgtemp   = 0.0;
   int    kk        = 0;

qDebug() << "TM:BEG:calcres" << QTime::currentTime().toString("hh:mm:ss.zzz");
   // populate b array with experiment data concentrations
   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
         nnls_b[ kk++ ] = edata->value( ss, rr );

      avgtemp += edata->scanData[ ss ].temperature;
   }
   avgtemp /= (double)nscans;

   if ( abort ) return;

   // determine s correction factor
   US_Math2::SolutionData solution;
   solution.density   = density;
   solution.viscosity = viscosity;
   solution.vbar20    = vbar;
   solution.vbar      = vbar;
   US_Math2::data_correction( avgtemp, solution );
DbgLv(1) << "   CR: dens visc vbar temp corr" << density << viscosity
   << vbar << avgtemp << solution.s20w_correction;
   double sfactor     = 1.0e-13 / solution.s20w_correction;

   // simulate data using models with single s,f/f0 component
   kk  = 0;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      if ( abort ) return;
      // set model with s,k point; update other coefficients
      double sval                = solute_i[ cc ].s;
      double kval                = solute_i[ cc ].k;
      model.components[ 0 ]      = zcomponent;
      model.components[ 0 ].s    = qAbs( sval ) * sfactor;
      model.components[ 0 ].f_f0 = kval;
      model.update_coefficients();
//DbgLv(1) << "  TR:   s k D" << sval << kval << model.components[0].D;

      // initialize simulation data with experiment grid
      US_AstfemMath::initSimData( sdata, *edata, 0.0 );

//DbgLv(1) << "  TR:     astfem_rsa calc";
      // calculate Astfem_RSA solution
      US_Astfem_RSA astfem_rsa( model, simpars );
      astfem_rsa.calculate( sdata );
      if ( abort ) return;

      // Populate the A matrix for the NNLS routine with the model function
      for ( int ss = 0; ss < nscans; ss++ )
         for ( int rr = 0; rr < npoints; rr++ )
            nnls_a[ kk++ ] = sdata.value( ss, rr );

      kstep++;

      if ( ( kstep % increp ) == 0 )
      {
         emit work_progress( increp );
         lstep = kstep;
      }
   }

   emit work_progress( nsolutes - lstep );
   int kstodo = nsolutes;
   if ( abort ) return;

qDebug() << "TM:BEG:clcr-nn" << QTime::currentTime().toString("hh:mm:ss.zzz");
   if ( ( noisflag & 1 ) != 0 )
   {
      if ( abort ) return;
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( nrinois, 0.0 );

      if ( noisflag == 3 )
         compute_a_tilde( a_tilde );

      // Compute a_bar, the average experiment signal at each radius
      QVector< double > a_bar( ntinois, 0.0 );
      compute_a_bar( a_bar, a_tilde );

      // Compute L_tildes, the average signal at each radius;
      QVector< double > L_tildes( nrinois * nsolutes, 0.0 );

      if ( noisflag == 3 )
         compute_L_tildes( nrinois, ntotal, nsolutes, L_tildes, nnls_a );

      // Compute L_bars
      QVector< double > L_bars(   ntinois * nsolutes, 0.0 );
      compute_L_bars( nsolutes, nrinois, ntinois, ntotal,
                      L_bars, nnls_a, L_tildes );

      // Set up small_a, small_b for alternate nnls
qDebug() << "  set SMALL_A+B";
      QVector< double > small_a( nsolutes * nsolutes, 0.0 );
      QVector< double > small_b( nsolutes           , 0.0 );

      ti_small_a_and_b( nsolutes, ntotal, ntinois,
                        small_a, small_b, a_bar, L_bars, nnls_a );
      if ( abort ) return;
      kstodo     = 4;

      // This is Sum( concentration * Lamm ) for the models after NNLS
qDebug() << "  noise small NNLS";
      US_Math2::nnls( small_a.data(), nsolutes, nsolutes, nsolutes,
                      small_b.data(), nnls_x.data() );
      if ( abort ) return;
      kstodo     = 2;
      emit work_progress( kstodo );

      // This is Sum( concentration * Lamm ) for the models after NNLS
      QVector< double > L( ntotal, 0.0 );
      compute_L( ntotal, nsolutes, L, nnls_a, nnls_x );

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius
      QVector< double > L_tilde( nrinois, 0.0 );

      if ( noisflag == 3 )
         compute_L_tilde( L_tilde, L );

      // Compute L_bar, the average model signal at each radius
      QVector< double > L_bar(   ntinois, 0.0 );
      compute_L_bar( L_bar, L, L_tilde );

      // Compute ti noise
      for ( int ii = 0; ii < ntinois; ii++ )
         tinvec[ ii ] = a_bar[ ii ] - L_bar[ ii ];

      if ( noisflag == 3 )
      {  // Compute ri_noise  (Is this correct????)
         for ( int ii = 0; ii < nrinois; ii++ )
            rinvec[ ii ] = a_tilde[ ii ] - L_tilde[ ii ];
      }

      emit work_progress( kstodo );
   }  // End tinoise and optional rinoise calculation

   else if ( ( noisflag & 2 ) != 0 )
   {
      if ( abort ) return;
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( nrinois, 0.0 );
      compute_a_tilde( a_tilde );

      // Compute L_tildes, the average signal at each radius
      QVector< double > L_tildes( nrinois * nsolutes, 0.0 );
      compute_L_tildes( nrinois, ntotal, nsolutes, L_tildes, nnls_a );

      // Set up small_a, small_b for the nnls
      QVector< double > small_a( nsolutes * nsolutes, 0.0 );
      QVector< double > small_b( nsolutes,            0.0 );
      if ( abort ) return;
      ri_small_a_and_b( nsolutes, ntotal, nrinois, small_a, small_b,
                        a_tilde, L_tildes, nnls_a );
      if ( abort ) return;

      US_Math2::nnls( small_a.data(), nsolutes, nsolutes, nsolutes,
                      small_b.data(), nnls_x.data() );
      if ( abort ) return;

      // This is sum( concentration * Lamm ) for the models after NNLS
      QVector< double > L( ntotal, 0.0 );
      compute_L( ntotal, nsolutes, L, nnls_a, nnls_x );

      // Now L contains the best fit sum of L equations
      // Compute L_tilde, the average model signal at each radius
      QVector< double > L_tilde( nrinois, 0.0 );
      compute_L_tilde( L_tilde, L );

      // Compute ri_noise  (Is this correct????)
      for ( int ii = 0; ii < nrinois; ii++ )
         rinvec[ ii ] = a_tilde[ ii ] - L_tilde[ ii ];

      emit work_progress( kstodo );
   }  // End rinoise alone calculation

   else
   {
      if ( abort ) return;
      emit work_progress( 2 );
      kstodo -= 2;

      US_Math2::nnls( nnls_a.data(), ntotal, ntotal, nsolutes,
                      nnls_b.data(), nnls_x.data() );
      if ( abort ) return;

      emit work_progress( kstodo );
      // Note:  ti_noise and ri_noise are already zero

   }  // End of core calculations
qDebug() << "TM:END:clcr-nn" << QTime::currentTime().toString("hh:mm:ss.zzz");


   // Clear simulation data and computed solutes
   for ( int ss = 0; ss < nscans; ss++ )
      for ( int rr = 0; rr < npoints; rr++ )
         sdata.scanData[ ss ].readings[ rr ] = US_DataIO2::Reading( 0.0 );

   solute_c.clear();
   if ( abort ) return;

   // Populate simulation data and computed solutes
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      double soluval = nnls_x[ cc ];

      if ( soluval > 0.0 )
      {
         for ( int ss = 0; ss < nscans; ss++ )
         {
            for ( int rr = 0; rr < npoints; rr++ )
            {
               sdata.scanData[ ss ].readings[ rr ] = 
                  US_DataIO2::Reading( soluval * edata->value( ss, rr ) );
            }
         }

         solute_i[ cc ].c = soluval;
         solute_c << solute_i[ cc ];
      }
   }
   if ( abort ) return;

   // Fill noise objects with any calculated vectors
   if ( ( noisflag & 1 ) != 0 )
   {
      ti_noise.values << tinvec;
      ti_noise.count  =  ntinois;
   }

   if ( ( noisflag & 2 ) != 0 )
   {
      ri_noise.values << rinvec;
      ri_noise.count  =  nrinois;
   }

qDebug() << "TM:END:calcres" << QTime::currentTime().toString("hh:mm:ss.zzz");
}


// Compute a_tilde, the average experiment signal at each time
void WorkerThread::compute_a_tilde( QVector< double >& a_tilde )
{
   double avgscale = 1.0 / (double)npoints;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
        a_tilde[ ss ] += edata->value( ss, rr );

      a_tilde[ ss ] *= avgscale;
   }
}

// Compute L_tildes, the average signal at each radius
void WorkerThread::compute_L_tildes( int                      nrinois,
                                     int                      ntotal,
                                     int                      nsolutes,
                                     QVector< double >&       L_tildes,
                                     const QVector< double >& nnls_a )
{
   double avgscale = 1.0 / (double)npoints;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int solute_index = cc * ntotal;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         int index      = cc * nrinois + ss;
         int scan_index = solute_index + ss * npoints;

         for ( int rr = 0; rr < npoints; rr++ )
            L_tildes[ index ] += nnls_a[ scan_index + rr ];

         L_tildes[ index ] *= avgscale;
      }
   }
}

// Compute L_tilde, the average model signal at each radius
void WorkerThread::compute_L_tilde( QVector< double >&       L_tilde,
                                    const QVector< double >& L )
{
   double avgscale = 1.0 / (double)npoints;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      int s_index  = ss;
      int L_offset = ss * npoints;

      for ( int rr = 0; rr < npoints; rr++ )
         L_tilde[ s_index ] += L[ L_offset  + rr ];

      L_tilde[ s_index] *= avgscale;
   }
}

void WorkerThread::compute_L( int                      ntotal,
                              int                      nsolutes,
                              QVector< double >&       L,
                              const QVector< double >& nnls_a,
                              const QVector< double >& nnls_x )
{
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      if ( nnls_x[ cc ] > 0 )
      {
         int kk      = 0;

         for ( int ss = 0; ss < nscans; ss++ )
         {
            for ( int rr = 0; rr < npoints; rr++ )
            {
               L[ kk++ ] += ( nnls_x[ cc ] *
                              nnls_a[ cc * ntotal + ss * npoints + rr ] );
            }
         }
      }
   }
}

void WorkerThread::ri_small_a_and_b( int                      nsolutes,
                                     int                      ntotal,
                                     int                      nrinois,
                                     QVector< double >&       small_a,
                                     QVector< double >&       small_b,
                                     const QVector< double >& a_tilde,
                                     const QVector< double >& L_tildes,
                                     const QVector< double >& nnls_a )
{
qDebug() << "TM:BEG:ri-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
#if 0
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         for ( int ss = 0; ss < nscans; ss++ )
         {
            small_b[ cc ] +=
               ( edata->value( ss, rr ) - a_tilde[ ss ] )
               *
               ( nnls_a[ cc * ntotal + ss * npoints + rr ] 
                 -
                 L_tildes[ cc * nrinois + ss ]
               );

            for ( int kk = 0; kk < nsolutes; kk++ )
            {
               small_a[ kk * nsolutes + cc ] +=
                  ( nnls_a[ kk * ntotal + ss * npoints + rr ]
                    - 
                    L_tildes[ kk * nrinois + ss  ]
                  ) 
                  *
                  ( nnls_a[ cc * ntotal + ss * npoints + rr ]
                    -  
                    L_tildes[ cc * nrinois + ss ]
                  );
            }
         }
      }
   }
#endif
#if 1
   int kstodo = nsolutes;
   int inprgr = kstodo / 50;
   inprgr     = ( inprgr < 1  ) ? 1  : inprgr;
   inprgr     = ( inprgr > 50 ) ? 50 : inprgr;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int    jjna  = cc * ntotal;
      int    jjlt  = cc * nrinois;
      int    jsa2  = jjna;
      int    jst2  = jjlt;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         // small_b[ cc ] +=
         //    ( edata->value( ss, rr ) - a_tilde[ ss ] )
         //    *
         //    ( nnls_a[ cc * ntotal + ss * npoints + rr ] 
         //      -
         //      L_tildes[ cc * nrinois + ss ] );
         double atil  = a_tilde [ ss ];
         double Ltil  = L_tildes[ jjlt++ ];

         for ( int rr = 0; rr < npoints; rr++ )
            small_b[ cc ] += ( ( edata->value( ss, rr ) - atil )
                             * ( nnls_a[ jjna++ ]       - Ltil ) );

      }

      for ( int kk = 0; kk < nsolutes; kk++ )
      {
         //small_a[ kk * nsolutes + cc ] +=
         //   ( nnls_a[ kk * ntotal + ss * npoints + rr ]
         //     - 
         //     L_tildes[ kk * nrinois + ss  ]
         //   ) 
         //   *
         //   ( nnls_a[ cc * ntotal + ss * npoints + rr ]
         //     -  
         //     L_tildes[ cc * nrinois + ss ] );
         int    jjma  = kk * nsolutes + cc;
         int    jja1  = kk * ntotal;
         int    jjt1  = kk * nrinois;
         int    jja2  = jsa2;
         int    jjt2  = jst2;

         for ( int ss = 0; ss < nscans; ss++ )
         {
            double Ltil1 = L_tildes[ jjt1++ ];
            double Ltil2 = L_tildes[ jjt2++ ];

            for ( int rr = 0; rr < npoints; rr++ )
               small_a[ jjma ] += ( ( nnls_a[ jja1++ ] - Ltil1 )
                                  * ( nnls_a[ jja2++ ] - Ltil2 ) );
         }
      }

      if ( ( cc % inprgr ) == 0 )
      {
         emit work_progress( inprgr );
         kstodo   -= inprgr;  
         kstodo    = ( kstodo < 2 ) ? 2 : kstodo;
      }

      if ( abort ) return;
   }
#endif
qDebug() << "TM:END:ri-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void WorkerThread::ti_small_a_and_b( int                      nsolutes,
                                     int                      ntotal,
                                     int                      ntinois,
                                     QVector< double >&       small_a,
                                     QVector< double >&       small_b,
                                     const QVector< double >& a_bar,
                                     const QVector< double >& L_bars,
                                     const QVector< double >& nnls_a )
{
qDebug() << "TM:BEG:ti-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
#if 0
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         for ( int ss = 0; ss < nscans; ss++ )
         {
            small_b[ cc ] += 
               ( edata->value( ss, rr ) - a_bar[ rr ] )
                 *
               ( nnls_a[ cc * ntotal + ss * npoints + rr ]
                 -
                 L_bars[ cc * ntinois + rr ] );

            for ( int kk = 0; kk < nsolutes; kk++ )
            {
               small_a[ kk * nsolutes + cc ] +=
                  ( nnls_a[ kk * ntotal + ss * npoints + rr ]
                    -
                    L_bars[ kk * ntinois + rr ] )
                  *
                  ( nnls_a[ cc * ntotal + ss * npoints + rr ]
                    -
                    L_bars[ cc * ntinois + rr ] );
            }
         }
      }
   }
#endif
#if 1
   int kstodo = nsolutes;
   int inprgr = kstodo / 50;
   inprgr     = ( inprgr < 1  ) ? 1  : inprgr;
   inprgr     = ( inprgr > 50 ) ? 50 : inprgr;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int jjsa  = cc;
      int jjna  = cc * ntotal;

      //small_b[ cc ] += 
      //   ( edata->value( ss, rr ) - a_bar[ rr ] )
      //     *
      //   ( nnls_a[ cc * ntotal + ss * npoints + rr ]
      //     -
      //     L_bars[ cc * ntinois + rr ] );

      for ( int ss = 0; ss < nscans; ss++ )
      {
         int jjlb  = cc * ntinois;

         for ( int rr = 0; rr < npoints; rr++ )
            small_b[ cc ] += ( edata->value( ss, rr ) - a_bar [ rr ]     )
                           * ( nnls_a[ jjna++ ]       - L_bars[ jjlb++ ] );
      }

      //small_a[ kk * nsolutes + cc ] +=
      //   ( nnls_a[ kk * ntotal  + ss * npoints + rr ]
      //     -
      //     L_bars[ kk * ntinois + rr ] )
      //   *
      //   ( nnls_a[ cc * ntotal  + ss * npoints + rr ]
      //     -
      //     L_bars[ cc * ntinois + rr ] );
      for ( int kk = 0; kk < nsolutes; kk++ )
      {
         int jsna1 = kk * ntotal;
         int jslb1 = kk * ntinois;
         int jsna2 = cc * ntotal;
         int jslb2 = cc * ntinois;

         for ( int ss = 0; ss < nscans; ss++ )
         {
            int jjna1 = jsna1;
            int jjlb1 = jslb1;
            int jjna2 = jsna2;
            int jjlb2 = jslb2;

            for ( int rr = 0; rr < npoints; rr++ )
            {
               small_a[ jjsa ] +=
                  ( nnls_a[ jjna1++ ] - L_bars[ jjlb1++ ] ) *
                  ( nnls_a[ jjna2++ ] - L_bars[ jjlb2++ ] );
            }

            jsna1    += npoints;  
            jsna2    += npoints;  
         }
         jjsa     += nsolutes;  
      }

      if ( ( cc % inprgr ) == 0 )
      {
         emit work_progress( inprgr );
         kstodo   -= inprgr;  
         kstodo    = ( kstodo < 2 ) ? 2 : kstodo;
      }

      if ( abort ) return;
   }

   emit work_progress( kstodo );
#endif
qDebug() << "TM:END:ti-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void WorkerThread::compute_L_bar( QVector< double >&       L_bar,
                                  const QVector< double >& L,
                                  const QVector< double >& L_tilde )
{
   double avgscale = 1.0 / (double)nscans;

   for ( int rr = 0; rr < npoints; rr++)
   {
      // Note  L_tilde is always zero when rinoise has not been requested
      for ( int ss = 0; ss < nscans; ss++ )
         L_bar[ rr ] += ( L[ ss * npoints + rr ] - L_tilde[ ss ] );

      L_bar[ rr ] *= avgscale;
   }
}

// Calculate the average measured concentration at each radius point
void WorkerThread::compute_a_bar( QVector< double >&       a_bar,
                                  const QVector< double >& a_tilde )
{
   double avgscale = 1.0 / (double)nscans;

   for ( int rr = 0; rr < npoints; rr++ )
   {
      // Note: a_tilde is always zero when rinoise has not been requested
      for ( int ss = 0; ss < nscans; ss++ )
         a_bar[ rr ] += ( edata->value( ss, rr ) - a_tilde[ ss ] );

      a_bar[ rr ] *= avgscale;
   }
}

// Calculate the average simulated concentration at each radius point
void WorkerThread::compute_L_bars( int                      nsolutes,
                                   int                      nrinois,
                                   int                      ntinois,
                                   int                      ntotal,
                                   QVector< double >&       L_bars,
                                   const QVector< double >& nnls_a,
                                   const QVector< double >& L_tildes )
{
   double avgscale = 1.0 / (double)nscans;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int solute_offset = cc * ntotal;

      for ( int rr = 0; rr < npoints; rr++ )
      {
         int r_index = cc * ntinois + rr;

         for ( int ss = 0; ss < nscans; ss++ )
         {
            // Note: L_tildes is always zero when rinoise has not been 
            // requested
               
            int n_index = solute_offset + ss * npoints + rr;
            int s_index = cc * nrinois + ss;

            L_bars[ r_index ] += ( nnls_a[ n_index ] - L_tildes[ s_index ] );
         }

         L_bars[ r_index ] *= avgscale;
      }
   }
}

