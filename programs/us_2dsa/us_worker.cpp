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
   wait();
//DbgLv(1) << "2P(WT):   Thread destroyed";
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

   calc_residuals();              // do all the work here

   quit();
   exec();

   emit work_complete( this );    // signal that a thread's work is done
}

// set a flag so that a worker thread will abort as soon as possible
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
   // use a zeroed component for initializing models
   zcomponent.s     = 0.0;
   zcomponent.D     = 0.0;
   zcomponent.mw    = 0.0;
   zcomponent.f     = 0.0;
   zcomponent.f_f0  = 0.0;

   // get dimensions
   nscans           = edata->scanData.size();
   npoints          = edata->x.size();
   int nsolutes     = solute_i.size();
   int ntotal       = npoints * nscans;
   int navals       = ntotal  * nsolutes;
   int ntinois      = npoints;
   int nrinois      = nscans;

   // set up and clear work vectors
   QVector< double > nnls_a( navals,   0.0 );
   QVector< double > nnls_b( ntotal,   0.0 );
   QVector< double > nnls_x( nsolutes, 0.0 );
   QVector< double > tinvec( ntinois,  0.0 );
   QVector< double > rinvec( nrinois,  0.0 );
DbgLv(1) << "   CR:na nb nx" << navals << ntotal << nsolutes;

   // get buffer,solution values communicated through edited data
   QString dvv      = edata->dataType;
   double density   = dvv.section( " ", 1, 1 ).toDouble();
   double viscosity = dvv.section( " ", 2, 2 ).toDouble();
   double vbar20    = dvv.section( " ", 3, 3 ).toDouble();
   double vbar      = dvv.section( " ", 4, 4 ).toDouble();
   double avgtemp   = edata->average_temperature();

DebugTime("BEG:calcres");
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
   solution.vbar20    = vbar20;
   solution.vbar      = vbar;
   US_Math2::data_correction( avgtemp, solution );
if (taskx==0) DbgLv(1) << "   CR: dens visc vbar20 vbar temp scorr dcorr"
 << density << viscosity << vbar20 << vbar << avgtemp
 << solution.s20w_correction << solution.D20w_correction;
   double sfactor = 1.0 / solution.s20w_correction;
   double dfactor = 1.0 / solution.D20w_correction;

   // simulate data using models with single s,f/f0 component
   int    increp  = nsolutes / 10;                 // progress report increment
          increp  = ( increp < 10 ) ? 10 : increp;
   int    kstep   = 0;                             // progress step count
          kk      = 0;                             // nnls_a output index
   zcomponent.vbar20  = vbar20;                    // vbar-20 for all components

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      if ( abort ) return;
      // set model with s,k point; update other coefficients
      model.components[ 0 ]      = zcomponent;     // Standard space coeffs
      model.components[ 0 ].s    = qAbs( solute_i[ cc ].s );
      model.components[ 0 ].f_f0 = solute_i[ cc ].k;
      model.update_coefficients();                 // Fill in any missing
      model.components[ 0 ].s   *= sfactor;        // Convert to experimental
      model.components[ 0 ].D   *= dfactor;

      // initialize simulation data with experiment grid
      US_AstfemMath::initSimData( sdata, *edata, 0.0 );

      // calculate Astfem_RSA solution
      US_Astfem_RSA astfem_rsa( model, simparms );
      astfem_rsa.calculate( sdata );
      if ( abort ) return;

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

   if ( kstep > 0 )
      emit work_progress( kstep );

   if ( abort ) return;

   int kstodo = nsolutes / 50;
   kstodo     = max( kstodo, 2 );

DebugTime("BEG:clcr-nn");
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
         compute_L_tildes( nrinois, nsolutes, L_tildes, nnls_a );

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
      compute_L_tildes( nrinois, nsolutes, L_tildes, nnls_a );

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
DebugTime("END:clcr-nn");

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

DebugTime("END:calcres");
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
                                     int                      nsolutes,
                                     QVector< double >&       L_tildes,
                                     const QVector< double >& nnls_a )
{
   double avgscale = 1.0 / (double)npoints;
   int    a_index  = 0;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int t_index      = cc * nrinois;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         for ( int rr = 0; rr < npoints; rr++ )
            L_tildes[ t_index ] += nnls_a[ a_index++ ];

         L_tildes[ t_index++ ] *= avgscale;
      }
   }
}

// Compute L_tilde, the average model signal at each radius
void WorkerThread::compute_L_tilde( QVector< double >&       L_tilde,
                                    const QVector< double >& L )
{
   double avgscale = 1.0 / (double)npoints;
   int    index    = 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
         L_tilde[ ss ] += L[ index++ ];

      L_tilde[ ss ] *= avgscale;
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
DebugTime("BEG:ri_smab");
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
      double sum_b = small_b[ cc ];

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
         {
            sum_b += ( ( edata->value( ss, rr ) - atil )
                     * ( nnls_a[ jjna++ ]       - Ltil ) );
         }

      }

      small_b[ cc ] = sum_b;

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
         int    jja2  = jsa2;
         int    jjt1  = kk * nrinois;
         int    jjt2  = jst2;
         double sum_a = small_a[ jjma ];

         for ( int ss = 0; ss < nscans; ss++ )
         {
            double Ltil1 = L_tildes[ jjt1++ ];
            double Ltil2 = L_tildes[ jjt2++ ];

            for ( int rr = 0; rr < npoints; rr++ )
            {
               sum_a += ( ( nnls_a[ jja1++ ] - Ltil1 )
                        * ( nnls_a[ jja2++ ] - Ltil2 ) );
            }
         }

         small_a[ jjma ] = sum_a;
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
DebugTime("END:ri_smab");
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
DebugTime("BEG:ti-smab");
   int kstodo = sq( nsolutes ) / 10;   // progress steps to report
   int incprg = nsolutes / 20;         // increment between reports
   incprg     = max( incprg,  1 );
   incprg     = min( incprg, 10 );
   int jstprg = ( kstodo * incprg ) / nsolutes;  // steps for each report
   int kstep  = 0;                               // progress counter

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int jjsa  = cc;
      int jssa  = cc * ntotal;
      int jssb  = cc * ntinois;
      int jjna  = jssa;

      //small_b[ cc ] += 
      //   ( edata->value( ss, rr ) - a_bar[ rr ] )
      //     *
      //   ( nnls_a[ cc * ntotal + ss * npoints + rr ]
      //     -
      //     L_bars[ cc * ntinois + rr ] );
      double sum_b = small_b[ cc ];

      for ( int ss = 0; ss < nscans; ss++ )
      {
         int jjlb  = jssb;

         for ( int rr = 0; rr < npoints; rr++ )
         {
            sum_b  += ( ( edata->value( ss, rr ) - a_bar [ rr ]     )
                      * ( nnls_a[ jjna++ ]       - L_bars[ jjlb++ ] ) );
         }
      }

      small_b[ cc ] = sum_b;

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
         int    jjna1 = kk * ntotal;
         int    jjna2 = jssa;
         int    jslb1 = kk * ntinois;
         int    jslb2 = jssb;
         double sum_a = small_a[ jjsa ];

         for ( int ss = 0; ss < nscans; ss++ )
         {
            int jjlb1 = jslb1;
            int jjlb2 = jslb2;

            for ( int rr = 0; rr < npoints; rr++ )
            {
               sum_a += ( ( nnls_a[ jjna1++ ] - L_bars[ jjlb1++ ] )
                        * ( nnls_a[ jjna2++ ] - L_bars[ jjlb2++ ] ) );
            }
         }

         small_a[ jjsa ] = sum_a;
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
DebugTime("END:ti-smab");
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

// debug message with thread number and time value
void WorkerThread::DebugTime( QString mtext )
{
DbgLv(1) << "w" << thrn << "TM:"
 << mtext + " " + QTime::currentTime().toString("hh:mm:ss.zzz");
}

