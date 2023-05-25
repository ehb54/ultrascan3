//! \file us_worker_2d.cpp
#include "us_worker_2d.h"
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
WorkerThread2D::WorkerThread2D( QObject* parent )
   : QThread( parent )
{
   dbg_level  = US_Settings::us_debug();
   abort      = false;
   solvesim   = nullptr;
   thrn       = -1;
DbgLv(1) << "2P(WT): Thread created";
}

// worker thread destructor
WorkerThread2D::~WorkerThread2D()
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
void WorkerThread2D::define_work( WorkPacket2D& workin )
{

   llim_s      = workin.ll_s;
   llim_k      = workin.ll_k;
   thrn        = workin.thrn;
   taskx       = workin.taskx;
   depth       = workin.depth;
   iter        = workin.iter;
   menmcx      = workin.menmcx;
   noisflag    = workin.noisf;
   typeref     = workin.typeref;

   solutes_i   = workin.isolutes;

   //dsets << workin.dsets[ 0 ];
   dset_wk              = *(workin.dsets[ 0 ]);  // local copy of data set
   dset_wk.noise_files  = workin.dsets[ 0 ]->noise_files;
   dset_wk.run_data     = workin.dsets[ 0 ]->run_data;
   dset_wk.model        = workin.dsets[ 0 ]->model;
   dset_wk.simparams    = workin.dsets[ 0 ]->simparams;
   dset_wk.solution_rec = workin.dsets[ 0 ]->solution_rec;
   dsets.clear();
   dsets << &dset_wk;                        // save its pointer
DbgLv(1) << "WT:DWk: " 
 << "dsi:tsobj" << workin.dsets[0]->simparams.tsobj
 << "dsw:tsobj" << dset_wk.simparams.tsobj
 << "dsi:sspk" << workin.dsets[0]->simparams.sim_speed_prof.count()
 << "dsw:sspk" << dset_wk.simparams.sim_speed_prof.count();

   sim_vals             = workin.sim_vals;
   sim_vals.variances   = workin.sim_vals.variances;
   sim_vals.ti_noise    = workin.sim_vals.ti_noise;
   sim_vals.ri_noise    = workin.sim_vals.ri_noise;
   sim_vals.solutes     = workin.sim_vals.solutes;
   bandFormingGradient  = workin.bandFormingGradient;
   cosedData            = workin.cosedData;
}

// get results of a completed worker thread
void WorkerThread2D::get_result( WorkPacket2D& workout )
{
   workout.ll_s     = llim_s;
   workout.ll_k     = llim_k;
   workout.thrn     = thrn;
   workout.taskx    = taskx;
   workout.depth    = depth;
   workout.iter     = iter;
   workout.menmcx   = menmcx;
   workout.noisf    = noisflag;
   workout.Anorm    = norms ;
DbgLv(1) << "norm_size_from_get_result_worker_2d"<< workout.Anorm.size() ;
   workout.csolutes = solutes_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
   workout.sim_vals = sim_vals;
int nn=workout.csolutes.size();
int kk=nn/2;
int ni=solutes_i.size();
//DbgLv(1) << "2P(WT): thr nn" << thrn << nn << "out sol0 solk soln"
// << workout.csolutes[0].c << workout.csolutes[kk].c << workout.csolutes[nn-1].c
// << "in sol0 soln" << ni << solutes_i[0].s*1.e13 << solutes_i[ni-1].s*1.e13
// << solutes_i[0].c << solutes_i[ni-1].c;
}

// run the worker thread
void WorkerThread2D::run()
{

   calc_residuals();              // do all the work here

   quit();
   exec();

   emit work_complete( this );    // signal that a thread's work is done
}

// set a flag so that a worker thread will abort as soon as possible
void WorkerThread2D::flag_abort()
{
   solvesim->abort_work();
}

// Do the real work of a thread:  solution from solutes set
void WorkerThread2D::calc_residuals()
{

   if ( typeref == (-2) )
   {
      calc_resids_ratio();
      return;
   }

   solvesim            = new US_SolveSim( dsets, thrn, true );

   connect( solvesim, SIGNAL(  work_progress( int ) ),
            this,     SLOT( forward_progress( int ) ) );

   sim_vals.solutes    = solutes_i;

   sim_vals.noisflag   = noisflag;
   sim_vals.dbg_level  = dbg_level;
   sim_vals.dbg_timing = US_Settings::debug_match( "2dsaTiming" );

   solvesim->calc_residuals( 0, 1, sim_vals,false, nullptr, nullptr, nullptr, bandFormingGradient, cosedData );

   solutes_c           = sim_vals.solutes;
   ti_noise.values     = sim_vals.ti_noise;
   ri_noise.values     = sim_vals.ri_noise;
}

// Slot to forward a progress signal
void WorkerThread2D::forward_progress( int steps )
{
   emit work_progress( steps );
}

// Do thread work for model-ratio:  solution from single model, find ratio
void WorkerThread2D::calc_resids_ratio()
{
   dset_wk                    = *dsets[ 0 ];
   US_SolveSim::DataSet* dset = &dset_wk;

   solutes_c      .clear();
   ti_noise.values.clear();
   ri_noise.values.clear();

   // Create a single model from the solutes
   nsolutes  = solutes_i.size();
DbgLv(1) << "WT:CRR nsolutes" << nsolutes << "0vb" << solutes_i[0].v;
   US_Model wmodel;
   wmodel.components.resize( nsolutes );

   for ( int ii = 0; ii < nsolutes; ii++ )
   {
      wmodel.components[ ii ].s      = solutes_i[ ii ].s;
      wmodel.components[ ii ].f_f0   = solutes_i[ ii ].k;
      wmodel.components[ ii ].vbar20 = solutes_i[ ii ].v;
      wmodel.components[ ii ].signal_concentration = solutes_i[ ii ].c;
      wmodel.components[ ii ].D      = 0.0;
      wmodel.components[ ii ].mw     = 0.0;
      wmodel.components[ ii ].f      = 0.0;

      US_Model::calc_coefficients( wmodel.components[ ii ] );

      wmodel.components[ ii ].s     /= dset->s20w_correction;
      wmodel.components[ ii ].D     /= dset->D20w_correction;
   }

   edata = &dset->run_data;
   npoints   = edata->pointCount();
   nscans    = edata->scanCount();
   int ntotal    = nscans * npoints;
   QVector< double > nnls_a( ntotal, 0.0 );
   QVector< double > nnls_b( ntotal, 0.0 );
   QVector< double > nnls_x( 1,      0.0 );

DbgLv(1) << "WT:CRR ns np nt" << nscans << npoints << ntotal;
   US_AstfemMath::initSimData( sim_vals.sim_data, *edata, 0.0 );

   US_Astfem_RSA astfem_rsa( wmodel, dset->simparams );

   astfem_rsa.calculate( sim_vals.sim_data );

   int kk        = 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         nnls_a[ kk   ] = sim_vals.sim_data.value( ss, rr );
         nnls_b[ kk++ ] = edata->value( ss, rr );
      }
   }

   US_Math2::nnls( nnls_a.data(), ntotal, ntotal, 1,
                   nnls_b.data(), nnls_x.data() );

   double cmult = nnls_x[ 0 ];
DbgLv(1) << "WT:CRR  CMULT" << cmult;
   solutes_c.resize( nsolutes );

   for ( int ii = 0; ii < nsolutes; ii++ )
   {
      wmodel.components[ ii ].s      = solutes_i[ ii ].s;
      wmodel.components[ ii ].f_f0   = solutes_i[ ii ].k;
      wmodel.components[ ii ].vbar20 = solutes_i[ ii ].v;
      wmodel.components[ ii ].D      = 0.0;
      wmodel.components[ ii ].mw     = 0.0;
      wmodel.components[ ii ].f      = 0.0;

      double cvali    = wmodel.components[ ii ].signal_concentration;
      double cval     = cvali * cmult;
      wmodel.components[ ii ].signal_concentration = cval;

      US_Model::calc_coefficients( wmodel.components[ ii ] );

      solutes_c[ ii ]     = solutes_i[ ii ];
      solutes_c[ ii ].c   = cval;
DbgLv(1) << "WT:CRR     ii cvali cval" << ii << cvali << cval;
   }
}

