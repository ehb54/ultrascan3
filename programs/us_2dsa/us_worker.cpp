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
void WorkerThread::define_work( WorkPacket& workin )
{
   llim_s      = workin.ll_s;
   llim_k      = workin.ll_k;
   thrn        = workin.thrn;
   taskx       = workin.taskx;
   depth       = workin.depth;
   iter        = workin.iter;
   menmcx      = workin.menmcx;
   noisflag    = workin.noisf;
   edata       = workin.edata;
   simparms    = *workin.sparms;

   solute_i    = workin.isolutes;
}

// get results of a completed worker thread
void WorkerThread::get_result( WorkPacket& workout )
{
   workout.ll_s     = llim_s;
   workout.ll_k     = llim_k;
   workout.thrn     = thrn;
   workout.taskx    = taskx;
   workout.depth    = depth;
   workout.iter     = iter;
   workout.menmcx   = menmcx;
   workout.noisf    = noisflag;

   workout.csolutes = solute_c;
   workout.ti_noise = ti_noise.values;
   workout.ri_noise = ri_noise.values;
}

// run the worker thread
void WorkerThread::run()
{
DbgLv(1) << "THR RUN: taskx thrn" << taskx << thrn;

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

// do the real work of a thread:  solution from solutes set
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

   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   int nsolutes     = solute_i.size();
   int ntotal       = npoints * nscans;
   int navals       = ntotal  * nsolutes;
   int ntinois      = npoints;
   int nrinois      = nscans;

   QVector< double > nnls_a( navals,   0.0 );
   QVector< double > nnls_b( ntotal,   0.0 );
   QVector< double > nnls_x( nsolutes, 0.0 );
   QVector< double > tinvec( ntinois,  0.0 );
   QVector< double > rinvec( nrinois,  0.0 );
DbgLv(1) << "   CR:na nb nx" << navals << ntotal << nsolutes;

   QString dvv      = edata->dataType;
   double density   = dvv.section( " ", 1, 1 ).toDouble();
   double viscosity = dvv.section( " ", 2, 2 ).toDouble();
   double vbar      = dvv.section( " ", 3, 3 ).toDouble();
   double avgtemp   = edata->average_temperature();

DbgLv(1) << "TM:BEG:calcres" << QTime::currentTime().toString("hh:mm:ss.zzz");
   // populate b array with experiment data concentrations
   int    kk        = 0;
   for ( int ss = 0; ss < nscans; ss++ )
      for ( int rr = 0; rr < npoints; rr++ )
         nnls_b[ kk++ ] = edata->value( ss, rr );

   if ( abort ) return;

   // determine s correction factor
   US_Math2::SolutionData solution;
   solution.density   = density;
   solution.viscosity = viscosity;
   solution.vbar20    = vbar;
   solution.vbar      = vbar;
   US_Math2::data_correction( avgtemp, solution );
if (taskx==0) DbgLv(1) << "   CR: dens visc vbar temp corr" << density
 << viscosity << vbar << avgtemp << solution.s20w_correction;
   double sfactor = 1.0 / solution.s20w_correction;

   // simulate data using models with single s,f/f0 component
   int    increp  = nsolutes / 10;
          increp  = ( increp < 10 ) ? 10 : increp;
   int    kstep   = 0;
          kk      = 0;
//DbgLv(1) << "  TR:     BEG astfem_rsa loop";

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      if ( abort ) return;
      // set model with s,k point; update other coefficients
      model.components[ 0 ]      = zcomponent;
      model.components[ 0 ].s    = qAbs( solute_i[ cc ].s ) * sfactor;
      model.components[ 0 ].f_f0 = solute_i[ cc ].k;
      model.update_coefficients();
//DbgLv(2) << "  TR:   cc" << cc << "   s k D"
// << model.components[0].s << model.components[0].k << model.components[0].D;

      // initialize simulation data with experiment grid
      US_AstfemMath::initSimData( sdata, *edata, 0.0 );

//DbgLv(2) << "  TR:     astfem_rsa calc  cc" << cc;
      // calculate Astfem_RSA solution
      US_Astfem_RSA astfem_rsa( model, simparms );
      astfem_rsa.calculate( sdata );
      if ( abort ) return;

//DbgLv(2) << "  TR:     populate nnls_a";
      // Populate the A matrix for the NNLS routine with the model function
      for ( int ss = 0; ss < nscans; ss++ )
         for ( int rr = 0; rr < npoints; rr++ )
            nnls_a[ kk++ ] = sdata.value( ss, rr );

      if ( ++kstep == increp )
      {
         emit work_progress( increp );
         kstep = 0;
      }
   }
//DbgLv(1) << "  TR:     END astfem_rsa loop";

   if ( kstep > 0 )
      emit work_progress( kstep );

   if ( abort ) return;

   int kstodo = nsolutes / 50;
   kstodo     = max( kstodo, 2 );

DbgLv(1) << "TM:BEG:clcr-nn" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
DbgLv(1) << "  set SMALL_A+B";
      QVector< double > small_a( sq( nsolutes ), 0.0 );
      QVector< double > small_b( nsolutes,       0.0 );

      ti_small_a_and_b( nsolutes, ntotal, ntinois,
                        small_a, small_b, a_bar, L_bars, nnls_a );
      if ( abort ) return;

      // This is Sum( concentration * Lamm ) for the models after NNLS
DbgLv(1) << "  noise small NNLS";
      US_Math2::nnls( small_a.data(), nsolutes, nsolutes, nsolutes,
                      small_b.data(), nnls_x.data() );
      if ( abort ) return;

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
      QVector< double > small_a( sq( nsolutes ), 0.0 );
      QVector< double > small_b( nsolutes,       0.0 );
      if ( abort ) return;

      ri_small_a_and_b( nsolutes, ntotal, nrinois,
                        small_a, small_b, a_tilde, L_tildes, nnls_a );
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

      US_Math2::nnls( nnls_a.data(), ntotal, ntotal, nsolutes,
                      nnls_b.data(), nnls_x.data() );
      if ( abort ) return;

      emit work_progress( kstodo );
      // Note:  ti_noise and ri_noise are already zero

   }  // End of core calculations
DbgLv(1) << "TM:END:clcr-nn" << QTime::currentTime().toString("hh:mm:ss.zzz");

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
                  US_DataIO2::Reading( soluval * sdata.value( ss, rr ) );
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

DbgLv(1) << "TM:END:calcres" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
      double concentration = nnls_x[ cc ];

      if ( concentration > 0 )
      {
         int r_index = cc * ntotal;
         int count   = 0;

         for ( int ss = 0; ss < nscans; ss++ )
         {
            for ( int rr = 0; rr < npoints; rr++ )
            {
               L[ count++ ] += ( concentration * nnls_a[ r_index++ ] );
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
DbgLv(1) << "TM:BEG:ri-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
   int kstodo = sq( nsolutes ) / 10;   // progress steps to report
   int incprg = nsolutes / 20;         // increment between reports
   incprg     = max( incprg,  1 );
   incprg     = min( incprg, 10 );
   int jstprg = ( kstodo * incprg ) / nsolutes;  // steps for each report
   int kstep  = 0;                               // progress counter

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int    jsa2  = cc * ntotal;
      int    jst2  = cc * nrinois;
      int    jjna  = jsa2;
      int    jjlt  = jst2;

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

      if ( ++kstep == incprg )
      {
         emit work_progress( jstprg );
         kstodo   -= jstprg;
         kstep     = 0;
      }

      if ( abort ) return;
   }

   if ( kstodo > 0 )
      emit work_progress( kstodo );
DbgLv(1) << "TM:END:ri-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
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
DbgLv(1) << "TM:BEG:ti-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
   int kstodo = sq( nsolutes ) / 10;   // progress steps to report
   int incprg = nsolutes / 20;         // increment between reports
   incprg     = max( incprg,  1 );
   incprg     = min( incprg, 10 );
   int jstprg = ( kstodo * incprg ) / nsolutes;  // steps for each report
   int kstep  = 0;                               // progress counter

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

      if ( ++kstep == incprg )
      {
         emit work_progress( jstprg );
         kstodo   -= jstprg;
         kstep     = 0;
      }

      if ( abort ) return;
   }

   if ( kstodo > 0 )
      emit work_progress( kstodo );
DbgLv(1) << "TM:END:ti-smab" << QTime::currentTime().toString("hh:mm:ss.zzz");
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

