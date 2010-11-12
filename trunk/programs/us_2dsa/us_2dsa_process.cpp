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

// Class to process 2DSA simulations
US_2dsaProcess::US_2dsaProcess( US_DataIO2::EditedData* da_exper,
      QObject* parent /*=0*/ ) : QObject( parent )
{
   edata            = da_exper;
   parentw          = parent;
   dbg_level        = US_Settings::us_debug();
}

// Start a specified 2DSA fit run
void US_2dsaProcess::start_fit( double sll, double sul, int nss,
   double kll, double kul, int nks, int ngr, int nthr, int noif )
{
DbgLv(1) << "2P(2dsaProc): start_fit()";
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

DbgLv(1) << "2P: sll sul nss" << slolim << suplim << nssteps
 << " kll kul nks" << klolim << kuplim << nksteps
 << " ngref nthr noif" << ngrefine << nthreads << noisflag;

   timer.start();

   nscans      = edata->scanData.size();
   npoints     = edata->x.size();
   gdelta_s    = ( suplim - slolim ) / (double)( nssteps - 1 );
   gdelta_k    = ( kuplim - klolim ) / (double)( nksteps - 1 );
   sdelta_s    = gdelta_s * (double)ngrefine;
   sdelta_k    = gdelta_k * (double)ngrefine;

   nsubgrid    = ngrefine * ngrefine;
   ntpsteps    = nksteps * nssteps;
   kctask      = 0;
   kstask      = 0;
   nthreads    = ( nthreads < nsubgrid ) ? nthreads : nsubgrid;
DbgLv(1) << "2P:   nscans npoints" << nscans << npoints << " gdlts gdltk"
 << gdelta_s << gdelta_k << " sdlts sdltk" << sdelta_s << sdelta_k;
DbgLv(1) << "2P:   nsubgrid ntpsteps nthreads"
 << nsubgrid << ntpsteps << nthreads;

   int    ktask = 0;
   double llss  = slolim;

   for ( int ii = 0; ii < ngrefine; ii++ )
   {
      double llsk = klolim;

      for ( int jj = 0; jj < ngrefine; jj++ )
      {
         WorkDefine wdef;
         wdef.thrx      = 0;
         wdef.taskx     = ++ktask;
         wdef.noisf     = noisflag;
         wdef.ll_s      = llss;
         wdef.ll_k      = llsk;
         wdef.edata     = edata;
         wdef.isolutes  = create_solutes( llss, suplim, sdelta_s,
                                          llsk, kuplim, sdelta_k );
         workdefs << wdef;

         llsk          += gdelta_k;
      }

      llss     += gdelta_s;
   }

   // start the first threads
   for ( int ii = 0; ii < nthreads; ii++ )
   {
      WorkDefine*   wdef = &workdefs[ ii ];
      WorkerThread* wthr = new WorkerThread( this );
      wthreads << wthr;
      wdef->thrx         = ii + 1;

      wthr->define_work( *wdef );

      connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
               this, SLOT( thread_finished( WorkerThread* ) ) );
      connect( wthr, SIGNAL( work_progress( int           ) ),
               this, SLOT( step_progress  ( int           ) ) );

      wthr->start();
   }

   kstask = nthreads;

   emit message_update(
      tr( "Starting computations of %1 subgrids\n using %2 threads ..." )
      .arg( nsubgrid ).arg( nthreads ) );
}

// slot for thread step progress:  signal control/main progress bars
void US_2dsaProcess::step_progress( int ksteps )
{
   emit progress_update( ksteps );
}

// slot to handle a worker thread having finished
void US_2dsaProcess::thread_finished( WorkerThread* wthrd )
{
   WorkResult wresult;

   wthrd->get_result( wresult );   // get results of thread task
   int thrx   = wresult.thrx;      // thread index of task
   int taskx  = wresult.taskx;     // task index of task

   if ( kctask == 0 )
      c_solutes.clear();

   c_solutes << wresult.csolutes;  // build composite of all computed solutes

   kctask++;                       // bump count of completed tasks (subgrids)
DbgLv(1) << "THR_FIN: thrx" << thrx << " taskx" << taskx << " ll_s ll_k"
 << wresult.ll_s << wresult.ll_k << " kct kst" << kctask << kstask;
   int tx = thrx - 1;              // get index into thread list
   delete wthreads[ tx ];          // destroy thread

   emit refine_complete( kctask ); 

   emit message_update( 
      tr( "Computations for %1 of %2 subgrids are complete" )
      .arg( kctask ).arg( nsubgrid ) );

   if ( kctask >= nsubgrid )
   {  // all subgrids computed

      final_computes();
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

// use a worker thread one last time to compute using all computed solutes
void US_2dsaProcess::final_computes()
{
   emit subgrids_complete();

   WorkDefine* wdef = &workdefs[ 0 ];
   wdef->taskx      = -1;

   wdef->isolutes.clear();

   for ( int ii = 0; ii < c_solutes.size(); ii++ )
   {
      if ( c_solutes[ ii ].c > 0.0 )
         wdef->isolutes << c_solutes[ ii ];
   }
DbgLv(1) << "FinalComp: szSoluC szSoluI"
 << c_solutes.size() << wdef->isolutes.size();

   WorkerThread* wthr = new WorkerThread( this );
   wthr->define_work( *wdef );

   connect( wthr, SIGNAL( work_complete( WorkerThread* ) ),
            this, SLOT(  final_finished( WorkerThread* ) ) );
   connect( wthr, SIGNAL( work_progress( int           ) ),
            this, SLOT( step_progress  ( int           ) ) );

   QString pmsg = tr( "Computing final NNLS ..." );
   emit message_update( pmsg );

   wthreads[ 0 ] = wthr;
   wthr->start( );
}

// final pass to use composite computed solutes
void US_2dsaProcess::final_finished( WorkerThread* wthrd )
{
   WorkResult wresult;

   wthrd->get_result( wresult );  // get results of thread task
DbgLv(1) << "FIN_FIN: thrx taskx ll_s ll_k"
   << wresult.thrx << wresult.taskx << wresult.ll_s << wresult.ll_k;

   c_solutes = wresult.csolutes;
DbgLv(1) << "FIN_FIN:    c_sol size" << c_solutes.size();

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
   model.components.resize( c_solutes.size() );

   // build the final model
   for ( int ss = 0; ss < c_solutes.size(); ss++ )
   {
      US_Model::SimulationComponent mcomp;
      mcomp.s     = qAbs( c_solutes[ ss ].s ) * sfactor;
      mcomp.D     = 0.0;
      mcomp.mw    = 0.0;
      mcomp.f     = 0.0;
      mcomp.f_f0  = c_solutes[ ss ].k;
      mcomp.signal_concentration = c_solutes[ ss ].c;

      model.components[ ss ]  = mcomp;
   }

   model.update_coefficients();
   model.description = "demo1_veloc.test01.model.11";
   //model.write( "/home/gary/ultrascan/data/models/M0000099.xml" );

   US_SimulationParameters simpars;
   simpars.initFromData( NULL, *edata );
   US_AstfemMath::initSimData( sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( rdata, *edata, 0.0 );
   US_Astfem_RSA astfem_rsa( model, simpars );

   // calculate the simulation data
   astfem_rsa.calculate( sdata );

   nscans           = edata->scanData.size();
   npoints          = edata->x.size();

   // build residuals data set (exper - simul)
   for ( int ii = 0; ii < nscans; ii++ )
      for ( int jj = 0; jj < npoints; jj++ )
         rdata.scanData[ ii ].readings[ jj ] =
            US_DataIO2::Reading(
                  edata->value( ii, jj ) - sdata.value( ii, jj ) );

   // determine elapsed time
   int ktimes = ( timer.elapsed() + 500 ) / 1000;
   int ktimeh = ktimes / 3600;
   int ktimem = ( ktimes - ktimeh * 3600 ) / 60;
   ktimes     = ktimes - ktimeh * 3600 - ktimem * 60;

   // compose final status message
   QString pmsg =
      tr( "Solution converged...\n"
          "Iterations: 1\n"
          "Threads: %1\n"
          "Subgrids: %2\n"
          "Number of s Points: %3\n"
          "Number of f/f0 Points: %4\n"
          "Elapsed wall clock time:\n  " )
      .arg( nthreads ).arg( nsubgrid ).arg( nssteps ).arg( nksteps );

   if ( ktimeh > 0 )
      pmsg = pmsg + tr( "%1 hour(s), %2 minute(s), %3 second(s)" )
         .arg( ktimeh ).arg( ktimem ).arg( ktimes );

   else
      pmsg = pmsg + tr( "%1 minute(s), %2 second(s)" )
         .arg( ktimem ).arg( ktimes );

   emit message_update( pmsg );  // signal final message
   emit process_complete();      // signal that processing is complete

   delete wthreads[ 0 ];         // destroy thread
}

// Get results upon completion of all refinements
bool US_2dsaProcess::get_results( US_DataIO2::RawData* da_sim,
                                  US_DataIO2::RawData* da_res,
                                  US_Model*            da_mdl,
                                  US_Noise*            da_tin,
                                  US_Noise*            da_rin )
{
   bool all_ok = true;

   *da_sim     = sdata;                           // pointer to simul data
   *da_res     = rdata;                           // pointer to resid data
   *da_mdl     = model;                           // pointer to model

   if ( ( noisflag & 1 ) != 0  &&  da_tin != 0 )
      *da_tin     = ti_noise;                     // pointer to any ti noise

   if ( ( noisflag & 2 ) != 0  &&  da_rin != 0 )
      *da_rin     = ri_noise;                     // pointer to any ri noise

   return all_ok;
}

// Build solutes vector for a subgrid
QVector< Solute > US_2dsaProcess::create_solutes(
   double lls, double uls, double deltas,
   double llk, double ulk, double deltak )
{
   QVector< Solute > solu;

   for ( double sval = lls; sval <= uls; sval += deltas )
      for ( double kval = llk; kval <= ulk; kval += deltak )
         solu << Solute( sval, kval );

   return solu;
}


// construct worker thread
WorkerThread::WorkerThread( QObject* parent )
   : QThread( parent )
{
   dbg_level        = US_Settings::us_debug();
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
//DbgLv(1) << "2P(WT):   Thread destroyed";
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
   int ntstep       = solute_i.size();
   int ndapts       = npoints * nscans;
   int navals       = ndapts  * ntstep;
   int increp       = ntstep / 10;
       increp       = ( increp < 10 ) ? 10 : increp;
   int kstep        = 0;
   int lstep        = 0;

   QVector< double > nnls_a( navals, 0.0 );
   QVector< double > nnls_b( ndapts, 0.0 );
   QVector< double > nnls_x( ntstep, 0.0 );
DbgLv(1) << "   CR:na nb nx" << navals << ndapts << ntstep;

   QString dvv      = edata->dataType;
   double density   = dvv.section( " ", 1, 1 ).toDouble();
   double viscosity = dvv.section( " ", 2, 2 ).toDouble();
   double vbar      = dvv.section( " ", 3, 3 ).toDouble();
   double avgtemp   = 0.0;
   int    kk        = 0;

   // populate b array with experiment data concentrations
   for ( int ii = 0; ii < nscans; ii++ )
   {
      for ( int jj = 0; jj < npoints; jj++ )
         nnls_b[ kk++ ] = edata->value( ii, jj );

      avgtemp += edata->scanData[ ii ].temperature;
   }
   avgtemp /= (double)nscans;

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

   for ( int ss = 0; ss < ntstep; ss++ )
   {
      // set model with s,k point; update other coefficients
      double sval                = solute_i[ ss ].s;
      double kval                = solute_i[ ss ].k;
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

      for ( int ii = 0; ii < nscans; ii++ )
         for ( int jj = 0; jj < npoints; jj++ )
            nnls_a[ kk++ ] = sdata.value( ii, jj );

      kstep++;

      if ( ( kstep % increp ) == 0 )
      {
         emit work_progress( increp );
         lstep = kstep;
      }
   }

   emit work_progress( ntstep - lstep );

   if ( ( noisflag & 1 ) != 0 )
   {
   }

   else if ( ( noisflag & 2 ) != 0 )
   {
   }

   else
   {
      US_Math2::nnls( nnls_a.data(), ndapts, ndapts, ntstep,
                      nnls_b.data(), nnls_x.data() );

      for ( int ii = 0; ii < nscans; ii++ )
         for ( int jj = 0; jj < npoints; jj++ )
            sdata.scanData[ ii ].readings[ jj ] = US_DataIO2::Reading( 0.0 );

      solute_c.clear();

      for ( int ss = 0; ss < ntstep; ss++ )
      {
         double soluval = nnls_x[ ss ];

         if ( soluval > 0.0 )
         {
            for ( int ii = 0; ii < nscans; ii++ )
            {
               for ( int jj = 0; jj < npoints; jj++ )
               {
                  sdata.scanData[ ii ].readings[ jj ] = 
                     US_DataIO2::Reading( soluval * edata->value( ii, jj ) );
               }
            }

            solute_i[ ss ].c = soluval;
            solute_c << solute_i[ ss ];
         }
      }
   }

   emit work_progress( ntstep );
}

