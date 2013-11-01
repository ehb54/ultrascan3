//! \file us_solve_sim.cpp
#include "us_solve_sim.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_math2.h"
#include "us_constants.h"

// Define level-conditioned debug print that includes thread/processor
#ifdef DbgLv
#undef DbgLv
//!< special definition of DbgLv(a) for threads
#define DbgLv(a) if(dbg_level>=a)qDebug()<<"SS-w:"<<thrnrank<<":"
#endif

double zerothr = 0.020;    //!< zero threshold OD value
double linethr = 0.050;    //!< linear threshold OD value
double maxod   = 1.50;     //!< maximum OD value
double mfactor = 3.00;     //!< peak multiplier value
double mfactex = 1.00;     //!< peak multiplier - experiment
double minnzsc = 0.005;    //!< minimum non-zero scale factor

// Create a Solve-Simulation object
US_SolveSim::US_SolveSim( QList< DataSet* >& data_sets, int thrnrank,
   bool signal_wanted ) : QObject(), data_sets( data_sets ),
   thrnrank( thrnrank ), signal_wanted( signal_wanted )
{
   abort        = false;     // Default: no abort
   dbg_level    = 0;         // Default: no debug prints
   dbg_timing   = false;     // Default: no debug timing prints
   banddthr     = false;     // Default: no bandform data_threshold

   // If band-forming, possibly read in threshold control values
   if ( data_sets[ 0 ]->simparams.band_forming )
   {
      QString bfcname = US_Settings::appBaseDir() + "/etc/bandform.config";
      QFile   bfcfile( bfcname );
      if ( bfcfile.open( QIODevice::ReadOnly ) )
      {
         QTextStream tsi( &bfcfile );
         // Read in, in order: zero-thresh, linear-thresh, max-od, min-nz-scale
         while ( ! tsi.atEnd() )
         {
            QString fline  = tsi.readLine();
            int     cmx    = fline.indexOf( "#" ) - 1;
            double  val    = cmx > 0
                             ? fline.left( cmx ).simplified().toDouble() : 0.0;
            if ( cmx <= 0 )
               continue;
            else if ( fline.contains( "zero threshold" ) )
               zerothr  = val;
            else if ( fline.contains( "linear threshold" ) )
               linethr  = val;
            else if ( fline.contains( "maximum OD" ) )
               maxod    = val;
            else if ( fline.contains( "minimum non-zero" ) )
               minnzsc  = val;
            else if ( fline.contains( "sim multiply factor" ) )
               mfactor  = val;
            else if ( fline.contains( "exp multiply factor" ) )
               mfactex  = val;
         }
         bfcfile.close();

         banddthr    = true;
      }
if(thrnrank==1) DbgLv(1) << "CR:zthr lthr mxod mnzc mfac mfex bthr"
 << zerothr << linethr << maxod << minnzsc << mfactor << mfactex << banddthr;
   }
}

// Create a Simulation object
US_SolveSim::Simulation::Simulation()
{
   variance      = 0.0;
   xnormsq       = 0.0;
   alpha         = 0.0;
   variances.clear();
   ti_noise .clear();
   ri_noise .clear();
   solutes  .clear();
   dbg_level     = 0;
   dbg_timing    = false;
   noisflag      = 0;
}

// Do the real work of a 2dsa/ga thread/processor:  simulation from solutes set
void US_SolveSim::calc_residuals( int offset, int dataset_count,
      Simulation& sim_vals, bool padAB,
      QVector< double >* ASave, QVector< double >* BSave )
{
   QVector< double > sv_nnls_a;
   QVector< double > sv_nnls_b;
   dbg_level     = sim_vals.dbg_level;            // Debug level
   dbg_timing    = sim_vals.dbg_timing;           // Debug-timing flag
   noisflag      = sim_vals.noisflag;             // Noise-calculation flag
   int nsolutes  = sim_vals.solutes.size();       // Input solutes count
   calc_ti       = ( ( noisflag & 1 ) != 0 );     // Calculate-TI flag
   calc_ri       = ( ( noisflag & 2 ) != 0 );     // Calculate-RI flag
   startCalc     = QDateTime::currentDateTime();  // Start calc time for timings
   int ntotal    = 0;                             // Total points count
   int ntinois   = 0;                             // TI noise value count
   int nrinois   = 0;                             // RI noise value count
   double alphad = sim_vals.alpha;                // Alpha for diagonal
   int lim_offs  = offset + dataset_count;        // Offset limit
#if 0
#ifdef NO_DB
   US_Settings::set_us_debug( dbg_level );
#endif
#endif

   if ( banddthr )
   {
      mfactor       = data_sets[ 0 ]->simparams.cp_width;
      if ( mfactor < 0.0 )
      {
         mfactor       = 1.0;
         zerothr       = 0.0;
         linethr       = 0.0;
         maxod         = 1.0e+20;
      }
if(thrnrank==1) DbgLv(1) << "CR:zthr lthr mxod mfac"
 << zerothr << linethr << maxod << mfactor;
   }

   US_DataIO::EditedData wdata;

   for ( int ee = offset; ee < lim_offs; ee++ )
   {  // Count scan,point totals for all data sets
      DataSet* dset = data_sets[ ee ];
      int npoints   = dset->run_data.xvalues.size();
      int nscans    = dset->run_data.scanData.size();
      ntotal       += ( nscans * npoints );
      ntinois      += npoints;
      nrinois      += nscans;
   }

   bool tikreg      = ( sim_vals.alpha != 0.0 );
   int  narows      = tikreg ? ( ntotal + nsolutes ) : ntotal;

   // Set up and clear work vectors
   int  navals      = narows * nsolutes;   // Size of "A" matrix
DbgLv(1) << "   CR:na nb nx" << navals << ntotal << nsolutes;
   QVector< double > nnls_a( navals,   0.0 );
DbgLv(1) << "   CR:na  size" << nnls_a.size();
   QVector< double > nnls_b( narows,   0.0 );
   QVector< double > nnls_x( nsolutes, 0.0 );
   QVector< double > tinvec( ntinois,  0.0 );
   QVector< double > rinvec( nrinois,  0.0 );

   // Set up for a single-component model
   US_Model model;
   model.components.resize( 1 );

   US_Model::SimulationComponent zcomponent; // Zeroed component to init models
   zcomponent.s     = 0.0;
   zcomponent.D     = 0.0;
   zcomponent.mw    = 0.0;
   zcomponent.f     = 0.0;
   zcomponent.f_f0  = 0.0;

   if ( banddthr )
   {  // If band forming, hold data within thresholds
//mfactex=mfactor;
      wdata          = data_sets[ 0 ]->run_data;
      data_threshold( &wdata, zerothr, linethr, maxod, mfactex );
   }

DebugTime("BEG:calcres");
   // Populate b array with experiment data concentrations
   int    kk     = 0;
   int    kodl   = 0;
#if 0
   double s0max  = 0.0;
#endif

   for ( int ee = offset; ee < lim_offs; ee++ )
   {
      US_DataIO::EditedData* edata = &data_sets[ ee ]->run_data;
      edata       = banddthr ? &wdata : edata;
      int npoints = edata->xvalues.size();
      int nscans  = edata->scanData.size();
      double odlim = edata->ODlimit;

      for ( int ss = 0; ss < nscans; ss++ )
      {
         for ( int rr = 0; rr < npoints; rr++ )
         {  // Fill the B matrix with experiment data (or zero)
            double evalue  = edata->value( ss, rr );

            if ( evalue >= odlim )
            {  // Where ODlimit is exceeded, substitute 0.0 and bump count
               evalue         = 0.0;
               kodl++;
            }

#if 0
            else if ( ss == 0 )
            {  // Find max of scan 0 values
               s0max          = qMax( s0max, evalue );
            }
#endif

            nnls_b[ kk++ ] = evalue;
         }
      }
   }
DbgLv(1) << "   CR:B fill kodl" << kodl;

#if 0
   // If needed, scale the alpha used in A-matrix appendix diagonals
   if ( tikreg )
   {
      alphad         = ( s0max == 0.0 ) ? sim_vals.alpha
                       : ( sim_vals.alpha * sqrt( s0max ) );
   }
#endif

   if ( abort ) return;

   QList< US_DataIO::RawData > simulations;
   simulations.reserve( nsolutes * dataset_count );

   // Simulate data using models, each with a single s,f/f0 component
   int    increp  = nsolutes / 10;                 // Progress report increment
          increp  = ( increp < 10 ) ? 10 : increp;
   int    kstep   = 0;                             // Progress step count
          kk      = 0;                             // nnls_a output index
   int    ksols   = 0;
   int    stype   = data_sets[ offset ]->solute_type;
DbgLv(1) << "   CR:BF STYPE" << stype;

   qSort( sim_vals.solutes );

   if ( stype == 0 )
   {  // Normal case of varying f/f0 with constant vbar
DataSet* dset=data_sets[0];
DbgLv(2) << "   CR:BF s20wcorr D20wcorr manual" << dset->s20w_correction
 << dset->D20w_correction << dset->solution_rec.buffer.manual;
      for ( int cc = 0; cc < nsolutes; cc++ )
      {  // Solve for each solute
         if ( abort ) return;
         int bx   = 0;

         for ( int ee = offset; ee < lim_offs; ee++ )
         {  // Solve for each data set
            DataSet*               dset  = data_sets[ ee ];
            US_DataIO::EditedData* edata = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData     simdat;
            int npoints = edata->xvalues.size();
            int nscans  = edata->scanData.size();
            zcomponent.vbar20          = dset->vbar20;

            // Set model with standard space s and k
            model.components[ 0 ]      = zcomponent;
            model.components[ 0 ].s    = sim_vals.solutes[ cc ].s;
            model.components[ 0 ].f_f0 = sim_vals.solutes[ cc ].k;

            // Fill in the missing component values
            model.update_coefficients();

            // Convert to experimental space
            model.components[ 0 ].s   /= dset->s20w_correction;
            model.components[ 0 ].D   /= dset->D20w_correction;

            // Initialize simulation data with the experiment's grid
            US_AstfemMath::initSimData( simdat, *edata, 0.0 );
if (dbg_level>1 && thrnrank<2 && cc==0) {
 model.debug(); dset->simparams.debug(); }

            // Calculate Astfem_RSA solution (Lamm equations)
            US_Astfem_RSA astfem_rsa( model, dset->simparams );

            astfem_rsa.set_debug_flag( dbg_level );

            astfem_rsa.calculate( simdat );
            if ( abort ) return;

            if ( banddthr )
            {  // If band forming, hold data within thresholds; skip if all-zero
               if ( data_threshold( &simdat, zerothr, linethr, maxod, mfactor ) )
                  continue;

               ksols++;
            }

            simulations << simdat;   // Save simulation (each dataset,solute)

            // Populate the A matrix for the NNLS routine with simulation
DbgLv(1) << "   CR: A fill kodl" << kodl;
int ks=kk;
            if ( kodl == 0 )
            {  // Normal case of no ODlimit substitutions
               for ( int ss = 0; ss < nscans; ss++ )
                  for ( int rr = 0; rr < npoints; rr++ )
                     nnls_a[ kk++ ] = simdat.value( ss, rr );
if(lim_offs>1&&(thrnrank==1||thrnrank==11)) DbgLv(1) << "CR: ks kk" << ks << kk
 << "nnA s...k" << nnls_a[ks] << nnls_a[ks+1] << nnls_a[kk-2] << nnls_a[kk-1]
 << "cc ee" << cc << ee;
            }
            else
            {  // Special case where ODlimit substitutions are in B matrix
               for ( int ss = 0; ss < nscans; ss++ )
               {
                  for ( int rr = 0; rr < npoints; rr++ )
                  {  // Fill A with simulations (or zero where B has zero)
                     if ( nnls_b[ bx++ ] != 0.0 )
                        nnls_a[ kk++ ] = simdat.value( ss, rr );
                     else
                        nnls_a[ kk++ ] = 0.0;
                  }
               }
            }

            if ( tikreg )
            {  // For Tikhonov Regularization append to each column
               for ( int aa = 0; aa < nsolutes; aa++ )
               {
                  nnls_a[ kk++ ] = ( aa == cc ) ? alphad : 0.0;
               }
            }

         }  // Each data set

         if ( signal_wanted  &&  ++kstep == increp )
         {  // If asked for and step at increment, signal progress
            emit work_progress( increp );
            kstep = 0;                     // Reset step count
         }
      }   // Each solute
   }   // Constant vbar

   else if ( stype == 1 )
   {  // Special case of varying vbar with constant f/f0
      zcomponent.f_f0   = sim_vals.solutes[ 0 ].k;

      for ( int cc = 0; cc < nsolutes; cc++ )
      {  // Solve for each solute
         if ( abort ) return;
         int bx   = 0;

         for ( int ee = offset; ee < lim_offs; ee++ )
         {  // Solve for each data set
            US_Math2::SolutionData  sd;
            DataSet*               dset  = data_sets[ ee ];
            US_DataIO::EditedData* edata = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData     simdat;
            int npoints    = edata->xvalues.size();
            int nscans     = edata->scanData.size();
            double avtemp  = dset->temperature;
            sd.viscosity   = dset->viscosity;
            sd.density     = dset->density;
            sd.manual      = dset->manual;
            sd.vbar20      = sim_vals.solutes[ cc ].v;
            sd.vbar        = US_Math2::adjust_vbar20( sd.vbar20, avtemp );

            US_Math2::data_correction( avtemp, sd );

            // Set model with standard space s and k
            model.components[ 0 ]        = zcomponent;
            model.components[ 0 ].s      = sim_vals.solutes[ cc ].s;
            model.components[ 0 ].vbar20 = sd.vbar20;

            // Fill in the missing component values
            model.update_coefficients();

            // Convert to experimental space
            model.components[ 0 ].s   /= sd.s20w_correction;
            model.components[ 0 ].D   /= sd.D20w_correction;

            // Initialize simulation data with the experiment's grid
            US_AstfemMath::initSimData( simdat, *edata, 0.0 );
if (dbg_level>1 && thrnrank==1 && cc==0) {
 model.debug(); dset->simparams.debug(); }

            // Calculate Astfem_RSA solution (Lamm equations)
            US_Astfem_RSA astfem_rsa( model, dset->simparams );

            astfem_rsa.set_debug_flag( dbg_level );

            astfem_rsa.calculate( simdat );
            if ( abort ) return;

            if ( banddthr )
            {  // If band forming, hold data within thresholds; skip if all-zero
               if ( data_threshold( &simdat, zerothr, linethr, maxod, mfactor ) )
                  continue;

               ksols++;
            }

            simulations << simdat;   // Save simulation (each datset,solute)

            // Populate the A matrix for the NNLS routine with simulation
            if ( kodl == 0 )
            {  // Normal case of no ODlimit substitutions
               for ( int ss = 0; ss < nscans; ss++ )
                  for ( int rr = 0; rr < npoints; rr++ )
                     nnls_a[ kk++ ] = simdat.value( ss, rr );
            }
            else
            {  // Special case where ODlimit substitutions are in B matrix
               for ( int ss = 0; ss < nscans; ss++ )
               {
                  for ( int rr = 0; rr < npoints; rr++ )
                  {  // Fill A with simulations (or zero where B has zero)
                     if ( nnls_b[ bx++ ] != 0.0 )
                        nnls_a[ kk++ ] = simdat.value( ss, rr );
                     else
                        nnls_a[ kk++ ] = 0.0;
                  }
               }
            }

            if ( tikreg )
            {  // For Tikhonov Regularization append to each column
               for ( int aa = 0; aa < nsolutes; aa++ )
               {
                  nnls_a[ kk++ ] = ( aa == cc ) ? alphad : 0.0;
               }
            }

         }  // Each data set

         if ( signal_wanted  &&  ++kstep == increp )
         {  // If asked for and step at increment, signal progress
            emit work_progress( increp );
            kstep = 0;                     // Reset step count
         }
      }   // Each solute
   }  // Constant f/f0

   else
   {  // Special case of custom grid

      for ( int cc = 0; cc < nsolutes; cc++ )
      {  // Solve for each solute
         if ( abort ) return;
         int bx   = 0;

         for ( int ee = offset; ee < lim_offs; ee++ )
         {  // Solve for each data set
            US_Math2::SolutionData  sd;
            DataSet*               dset   = data_sets[ ee ];
            US_DataIO::EditedData* edata  = banddthr ? &wdata : &dset->run_data;
            US_DataIO::RawData     simdat;
            int npoints    = edata->xvalues.size();
            int nscans     = edata->scanData.size();
            double avtemp  = dset->temperature;
            model.components[ 0 ]        = zcomponent;
            model.components[ 0 ].s      = sim_vals.solutes[ cc ].s;
            model.components[ 0 ].D      = sim_vals.solutes[ cc ].d;
            model.components[ 0 ].vbar20 = sim_vals.solutes[ cc ].v;

            // Fill in the missing component values
            model.update_coefficients();

            sd.viscosity   = dset->viscosity;
            sd.density     = dset->density;
            sd.manual      = dset->manual;
            sd.vbar20      = model.components[ 0 ].vbar20;
            sd.vbar        = US_Math2::adjust_vbar20( sd.vbar20, avtemp );

            US_Math2::data_correction( avtemp, sd );

            // Convert to experimental space
            model.components[ 0 ].s   /= sd.s20w_correction;
            model.components[ 0 ].D   /= sd.D20w_correction;

            // Initialize simulation data with the experiment's grid
            US_AstfemMath::initSimData( simdat, *edata, 0.0 );
if (dbg_level>1 && thrnrank==1 && cc==0) {
 model.debug(); dset->simparams.debug(); }

            // Calculate Astfem_RSA solution (Lamm equations)
            US_Astfem_RSA astfem_rsa( model, dset->simparams );

            astfem_rsa.set_debug_flag( dbg_level );

            astfem_rsa.calculate( simdat );
            if ( abort ) return;

            if ( banddthr )
            {  // If band forming, hold data within thresholds; skip if all-zero
               if ( data_threshold( &simdat, zerothr, linethr, maxod, mfactor ) )
                  continue;

               ksols++;
            }

            simulations << simdat;   // Save simulation (each datset,solute)

            // Populate the A matrix for the NNLS routine with simulation
            if ( kodl == 0 )
            {  // Normal case of no ODlimit substitutions
               for ( int ss = 0; ss < nscans; ss++ )
                  for ( int rr = 0; rr < npoints; rr++ )
                     nnls_a[ kk++ ] = simdat.value( ss, rr );
            }
            else
            {  // Special case where ODlimit substitutions are in B matrix
               for ( int ss = 0; ss < nscans; ss++ )
               {
                  for ( int rr = 0; rr < npoints; rr++ )
                  {  // Fill A with simulations (or zero where B has zero)
                     if ( nnls_b[ bx++ ] != 0.0 )
                        nnls_a[ kk++ ] = simdat.value( ss, rr );
                     else
                        nnls_a[ kk++ ] = 0.0;
                  }
               }
            }

            if ( tikreg )
            {  // For Tikhonov Regularization append to each column
               for ( int aa = 0; aa < nsolutes; aa++ )
               {
                  nnls_a[ kk++ ] = ( aa == cc ) ? alphad : 0.0;
               }
            }
         }  // Each data set

         if ( signal_wanted  &&  ++kstep == increp )
         {  // If asked for and step at increment, signal progress
            emit work_progress( increp );
            kstep = 0;                     // Reset step count
         }
      }   // Each solute
   }

DbgLv(1) << "   CR:BF nsol ksol" << nsolutes << ksols;
   nsolutes   = banddthr ? ksols : nsolutes;

   if ( signal_wanted  &&  kstep > 0 )  // If signals and steps done, report
      emit work_progress( kstep );

   if ( abort ) return;

   int kstodo = nsolutes / 50;          // Set steps count for NNLS
   kstodo     = max( kstodo, 2 );

DebugTime("BEG:clcr-nn");
   if ( calc_ti )
   {  // Compute TI Noise (and, optionally, RI Noise)
      if ( abort ) return;
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( nrinois, 0.0 );

      if ( calc_ri )
         compute_a_tilde( a_tilde, nnls_b );

      // Compute a_bar, the average experiment signal at each radius
      QVector< double > a_bar( ntinois, 0.0 );
      compute_a_bar( a_bar, a_tilde, nnls_b );

      // Compute L_tildes, the average signal at each radius (if RI noise)
      QVector< double > L_tildes( nrinois * nsolutes, 0.0 );

      if ( calc_ri )
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
                        small_a, small_b, a_bar, L_bars, nnls_a, nnls_b );
      if ( abort ) return;

      // Do NNLS to compute concentrations (nnls_x)
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

      if ( calc_ri )
         compute_L_tilde( L_tilde, L );

      // Compute L_bar, the average model signal at each radius
      QVector< double > L_bar(   ntinois, 0.0 );
      compute_L_bar( L_bar, L, L_tilde );

      // Compute ti noise
      for ( int ii = 0; ii < ntinois; ii++ )
         tinvec[ ii ] = a_bar[ ii ] - L_bar[ ii ];

      if ( calc_ri )
      {  // Compute RI_noise
         for ( int ii = 0; ii < nrinois; ii++ )
            rinvec[ ii ] = a_tilde[ ii ] - L_tilde[ ii ];
      }

      if ( signal_wanted )
         emit work_progress( kstodo );  // Report noise NNLS steps done
   }  // End tinoise and optional rinoise calculation

   else if ( calc_ri )
   {  // Compute RI noise (when RI only)
      if ( abort ) return;
      // Compute a_tilde, the average experiment signal at each time
      QVector< double > a_tilde( nrinois, 0.0 );
      compute_a_tilde( a_tilde, nnls_b );

      // Compute L_tildes, the average signal at each radius
      QVector< double > L_tildes( nrinois * nsolutes, 0.0 );
      compute_L_tildes( nrinois, nsolutes, L_tildes, nnls_a );

      // Set up small_a, small_b for the nnls
      QVector< double > small_a( sq( nsolutes ), 0.0 );
      QVector< double > small_b( nsolutes,       0.0 );
      if ( abort ) return;

      ri_small_a_and_b( nsolutes, ntotal, nrinois,
                        small_a, small_b, a_tilde, L_tildes, nnls_a, nnls_b );
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

      if ( signal_wanted )
         emit work_progress( kstodo );     // Report noise NNLS steps done
   }  // End rinoise alone calculation

   else
   {  // No TI or RI noise
      if ( abort ) return;

      if ( ASave != NULL  &&  BSave != NULL )
      {
         sv_nnls_a = nnls_a;
         sv_nnls_b = nnls_b;
DbgLv(1) << "CR: sv_nnls_a size" << sv_nnls_a.size() << nnls_a.size();
      }

      US_Math2::nnls( nnls_a.data(), narows, narows, nsolutes,
                      nnls_b.data(), nnls_x.data() );
if(lim_offs>1&&(thrnrank==1||thrnrank==11)) DbgLv(1) << "CR: narows nsolutes" << narows << nsolutes;
if(lim_offs>1&&(thrnrank==1||thrnrank==11)) DbgLv(1) << "CR:  a0 a1 b0 b1"
 << nnls_a[0] << nnls_a[1] << nnls_b[0] << nnls_b[1];

      if ( abort ) return;

      if ( signal_wanted )
         emit work_progress( kstodo );     // Report no-noise NNLS steps done
      // Note:  ti_noise and ri_noise are already zero

   }  // End of core calculations

   nnls_a.clear();
   nnls_b.clear();
DebugTime("END:clcr-nn");

DbgLv(1) << "CR: kstodo" << kstodo;
   if ( abort ) return;

   // Size simulation data and initialize concentrations to zero
   int kscans = 0;
   int jscan  = 0;

   for ( int ee = offset; ee < lim_offs; ee++ )
   {
      US_DataIO::RawData tdata;      // Init temp sim data with edata's grid
      US_AstfemMath::initSimData( tdata, data_sets[ ee ]->run_data, 0.0 );

      int nscans  = tdata.scanData.size();
      kscans     += nscans;
      sim_vals.sim_data.scanData.resize( kscans );

      if ( ee == offset )
      {
         sim_vals.sim_data = tdata;   // Init (first) dataset sim data
         jscan += nscans;
      }
      else
      {
         for ( int ss = 0; ss < nscans; ss++ )
         {  // Append zeroed-scans sim_data for multiple data sets
DbgLv(1) << "CR:     ss jscan" << ss << jscan << "ee kscans nscans" << ee << kscans << nscans;
            sim_vals.sim_data.scanData[ jscan++ ] = tdata.scanData[ ss ];
         }
      }
   }
if(lim_offs>1&&(thrnrank==1||thrnrank==11))
DbgLv(1) << "CR:       jscan kscans" << jscan << kscans;

   // This is a little tricky.  The simulations structure was created above
   // in a loop that alternates experiments with each solute.  In the loop
   // below, the simulation structure alters that order to group all solutes
   // for each experiment together

if(thrnrank==1) DbgLv(1) << "CR: nsolutes" << nsolutes;
if(lim_offs>1&&(thrnrank==1||thrnrank==11)) DbgLv(1) << "CR: nsolutes" << nsolutes;
   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      double soluval = nnls_x[ cc ];  // Computed concentration, this solute
if(thrnrank==1) DbgLv(1) << "CR: cc soluval" << cc << soluval;
if(lim_offs>1&&(thrnrank==1||thrnrank==11)) DbgLv(1) << "CR: cc soluval" << cc << soluval;

      if ( soluval > 0.0 )
      {  // If concentration non-zero, need to sum in simulation data
if(lim_offs>1&&(thrnrank==1||thrnrank==11))
DbgLv(1) << "CR: cc soluval" << cc << soluval;
         int scnx    = 0;

         for ( int ee = offset; ee < lim_offs; ee++ )
         {
            // Input sims (ea.dset, ea.solute); out sims (sum.solute, ea.dset)
            int sim_ix  = cc * dataset_count + ee - offset;
            US_DataIO::RawData*     idata = &simulations[ sim_ix ];
            US_DataIO::RawData*     sdata = &sim_vals.sim_data;
            int npoints = idata->pointCount();
            int nscans  = idata->scanCount();
if(lim_offs>1&&(thrnrank==1||thrnrank==11))
DbgLv(1) << "CR:    ee sim_ix np ns scnx" << ee << sim_ix << npoints << nscans << scnx;

            for ( int ss = 0; ss < nscans; ss++, scnx++ )
            {
               for ( int rr = 0; rr < npoints; rr++ )
               {
                  sdata->scanData[ scnx ].rvalues[ rr ] += 
                     ( soluval * idata->value( ss, rr ) );
               }
            }
//*DEBUG*
int ss=nscans/2;
int rr=npoints/2;
if( thrnrank==1 ) {
DbgLv(1) << "CR:   scnx ss rr" << scnx << ss << rr;
DbgLv(1) << "CR:     s k v" << sim_vals.solutes[cc].s*1.0e+13
 << sim_vals.solutes[cc].k << sim_vals.solutes[cc].v << "sval" << soluval
 << "idat sdat" << idata->value(ss,rr) << sdata->value(ss,rr);
if (soluval>100.0) {
 double drval=0.0; double dmax=0.0; double dsum=0.0;
 for ( int ss=0;ss<nscans;ss++ ) { for ( int rr=0; rr<npoints; rr++ ) {
  drval=idata->value(ss,rr); dmax=qMax(dmax,drval); dsum+=drval; }}
 DbgLv(1) << "CR:B s k" << sim_vals.solutes[cc].s*1.0e+13
  << sim_vals.solutes[cc].k << "sval" << soluval << "amax asum" << dmax << dsum; }
}
//*DEBUG*
         }
      }
   }

   double rmsds[ dataset_count ];
   int    kntva[ dataset_count ];
   double variance   = 0.0;
   int    tinoffs    = 0;
   int    rinoffs    = 0;
   int    ktotal     = 0;
   int    scnx       = 0;
   int    kdsx       = 0;
   US_DataIO::RawData*     sdata = &sim_vals.sim_data;
   US_DataIO::RawData*     resid = &sim_vals.residuals;
   sim_vals.residuals            = *sdata;

   // Calculate residuals and rmsd values
   for ( int ee = offset; ee < lim_offs; ee++, kdsx++ )
   {
      DataSet*                dset  = data_sets[ ee ];
      US_DataIO::EditedData*  edata = banddthr ? &wdata : &dset->run_data;
//      US_AstfemMath::initSimData( sim_vals.residuals, *edata, 0.0 );
      int    npoints   = edata->pointCount();
      int    nscans    = edata->scanCount();
      int    kntcs     = 0;
      double varidset  = 0.0;

      for ( int ss = 0; ss < nscans; ss++, scnx++ )
      {  // Create residuals dset:  exp - sim - noise(s)

         for ( int rr = 0; rr < npoints; rr++ )
         {

            double resval = edata->value( ss, rr )
                          - sdata->value( scnx, rr )
                          - tinvec[ rr + tinoffs ]
                          - rinvec[ ss + rinoffs ];
            resid->setValue( scnx, rr, resval );

            double r2    = sq( resval );
            variance    += r2;
            varidset    += r2;
            kntcs++;
         }
      }

      rmsds[ kdsx ]  = varidset;          // Variance for a data set
      kntva[ kdsx ]  = kntcs;
      ktotal        += kntcs;
      tinoffs       += npoints;
      rinoffs       += nscans;
DbgLv(1) << "CR: kdsx variance" << kdsx << varidset << variance;
   }

   sim_vals.variances.resize( dataset_count );
   variance         /= (double)ktotal;    // Total sets variance
   sim_vals.variance = variance;
   kk        = 0;

   for ( int ee = offset; ee < lim_offs; ee++ )
   {  // Scale variances for each data set
      sim_vals.variances[ kk ] = rmsds[ kk ] / (double)( kntva[ kk ] );
DbgLv(1) << "CR:     kk variance" <<  sim_vals.variances[kk];
      kk++;
   }

   // Store solutes for return
   kk        = 0;

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      if ( nnls_x[ cc ] > 0.0 )
      {  // Store solutes with non-zero concentrations
         sim_vals.solutes[ cc ].c = nnls_x[ cc ];
         sim_vals.solutes[ kk++ ] = sim_vals.solutes[ cc ];
      }
   }

   // Truncate solutes at non-zero count
   sim_vals.solutes.resize( qMax( kk, 1 ) );
DbgLv(1) << "CR: out solutes size" << kk;
DbgLv(1) << "CR:   jj solute-c" << 0 << sim_vals.solutes[0].c;
DbgLv(1) << "CR:   jj solute-c" << 1 << (kk>1?sim_vals.solutes[1].c:0.0);
DbgLv(1) << "CR:   jj solute-c" << kk-2 << (kk>1?sim_vals.solutes[kk-2].c:0.0);
DbgLv(1) << "CR:   jj solute-c" << kk-1 << (kk>0?sim_vals.solutes[kk-1].c:0.0);
   if ( abort ) return;

   // Fill noise objects with any calculated vectors
   if ( calc_ti )
      sim_vals.ti_noise << tinvec;

   if ( calc_ri )
      sim_vals.ri_noise << rinvec;

   // Compute and return the xnorm-squared value of concentrations
   nsolutes = sim_vals.solutes.size();
DbgLv(1) << "CR:     nsolutes" << nsolutes;
   double xnorm = 0.0;
   for ( int jj = 0; jj < nsolutes; jj++ )
   {
      xnorm += sq( sim_vals.solutes[ jj ].c );
   }

   sim_vals.xnormsq = xnorm;
DbgLv(1) << "CR:       xnormsq" << xnorm;

   // If specified, return the A and B matrices (with or without padding)
   if ( ASave != NULL  &&  BSave != NULL )
   {
      if ( padAB )
      {  // Return A and B with padding for regularization
         if ( tikreg )
         {  // If regularization was used, return A and B with padding intact
            (*ASave)     = sv_nnls_a;
            (*BSave)     = sv_nnls_b;
         }

         else
         {  // If no regularization, return A and B with padding added
            ASave->clear();
            BSave->clear();
            int    kk    = 0;

            for ( int cc = 0; cc < nsolutes; cc++ )
            {  // Save and pad A matrix, a column at a time
               for ( int jj = 0; jj < ntotal; jj++ )
               {  // Save an A column
                  (*ASave) << sv_nnls_a[ kk++ ];
               }

               for ( int jj = 0; jj < nsolutes; jj++ )
               {
                  (*ASave) << 0.0;   // Pad an A column (zeroes for now)
               }
            }

            for ( int jj = 0; jj < ntotal; jj++ )
            {  // Save the B matrix (vector)
               (*BSave) << sv_nnls_b[ jj ];
            }

            for ( int cc = 0; cc < nsolutes; cc++ )
            {
               (*BSave) << 0.0;      // Pad the B vector with zeroes
            }
         }

         // If we are doing padding, it is in preparation for regularization.
         // So, if any noise was calculated, it should be added to the
         // B vector.
         if ( calc_ri || calc_ti )
         {
            int nscans   = data_sets[ 0 ]->run_data.scanCount();
            int npoints  = data_sets[ 0 ]->run_data.pointCount();
            int kk       = 0;

            for ( int ss = 0; ss < nscans; ss++ )
            {
               double rinoi = calc_ri ? rinvec[ ss ] : 0.0;

               for ( int rr = 0; rr <  npoints; rr++ )
               {
                  double tinoi = calc_ti ? tinvec[ rr ] : 0.0;
                  (*BSave)[ kk++ ] += ( tinoi + rinoi );
               }
            }
         }

      }

      else
      {  // Return A and B without any regularization padding
         if ( tikreg )
         {  // If regularization was used, remove padding
            ASave->clear();
            BSave->clear();
            int    kk    = 0;

            for ( int cc = 0; cc < nsolutes; cc++ )
            {
               for ( int jj = 0; jj < ntotal; jj++ )
               {  // Save an A column
                  (*ASave) << sv_nnls_a[ kk++ ];
               }

               kk  += nsolutes;    // Bump past regularization padding
            }

            for ( int jj = 0; jj < ntotal; jj++ )
            {  // Save the B matrix (vector)
               (*BSave) << sv_nnls_b[ jj ];
            }
         }
               
         else
         {  // If no regularization, return A and B as they are
            (*ASave)     = sv_nnls_a;
            (*BSave)     = sv_nnls_b;
DbgLv(1) << "CR: ASv: sv_nnls_a size" << sv_nnls_a.size() << ASave->size();
         }
      }
   }
         
DebugTime("END:calcres");
}

// Set abort flag
void US_SolveSim::abort_work()
{
   abort = true;
}

// Compute a_tilde, the average experiment signal at each time
void US_SolveSim::compute_a_tilde( QVector< double >& a_tilde,
                                   const QVector< double >& nnls_b )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int    npoints  = edata->xvalues.size();
   int    nscans   = edata->scanData.size();
   int    jb       = 0;
   double avgscale = 1.0 / (double)npoints;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
        a_tilde[ ss ] += nnls_b[ jb++ ];

      a_tilde[ ss ] *= avgscale;
   }
}

// Compute L_tildes, the average signal at each radius
void US_SolveSim::compute_L_tildes( int                      nrinois,
                                    int                      nsolutes,
                                    QVector< double >&       L_tildes,
                                    const QVector< double >& nnls_a )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int    npoints  = edata->xvalues.size();
   int    nscans   = edata->scanData.size();
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
void US_SolveSim::compute_L_tilde( QVector< double >&       L_tilde,
                                   const QVector< double >& L )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int    npoints  = edata->xvalues.size();
   int    nscans   = edata->scanData.size();
   double avgscale = 1.0 / (double)npoints;
   int    index    = 0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
         L_tilde[ ss ] += L[ index++ ];

      L_tilde[ ss ] *= avgscale;
   }
}

void US_SolveSim::compute_L( int                      ntotal,
                             int                      nsolutes,
                             QVector< double >&       L,
                             const QVector< double >& nnls_a,
                             const QVector< double >& nnls_x )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      double concentration = nnls_x[ cc ];

      if ( concentration > 0.0 )
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

void US_SolveSim::ri_small_a_and_b( int                      nsolutes,
                                    int                      ntotal,
                                    int                      nrinois,
                                    QVector< double >&       small_a,
                                    QVector< double >&       small_b,
                                    const QVector< double >& a_tilde,
                                    const QVector< double >& L_tildes,
                                    const QVector< double >& nnls_a, 
                                    const QVector< double >& nnls_b )
{
DebugTime("BEG:ri_smab");
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();
   int kstodo  = sq( nsolutes ) / 10;   // progress steps to report
   int incprg  = nsolutes / 20;         // increment between reports
   incprg      = max( incprg,  1 );
   incprg      = min( incprg, 10 );
   int jstprg  = ( kstodo * incprg ) / nsolutes;  // steps for each report
   int kstep   = 0;                               // progress counter

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int    jsa2  = cc * ntotal;
      int    jst2  = cc * nrinois;
      int    jjna  = jsa2;
      int    jjnb  = 0;
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
            sum_b += ( ( nnls_b[ jjnb++ ] - atil )
                     * ( nnls_a[ jjna++ ] - Ltil ) );
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

      if ( signal_wanted  &&  ++kstep == incprg )
      {
         emit work_progress( jstprg );
         kstodo   -= jstprg;
         kstep     = 0;
      }

      if ( abort ) return;
   }

   if ( signal_wanted  &&  kstodo > 0 )
      emit work_progress( kstodo );
DebugTime("END:ri_smab");
}

void US_SolveSim::ti_small_a_and_b( int                      nsolutes,
                                    int                      ntotal,
                                    int                      ntinois,
                                    QVector< double >&       small_a,
                                    QVector< double >&       small_b,
                                    const QVector< double >& a_bar,
                                    const QVector< double >& L_bars,
                                    const QVector< double >& nnls_a,
                                    const QVector< double >& nnls_b )
{
DebugTime("BEG:ti-smab");
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();
   int kstodo  = sq( nsolutes ) / 10;   // progress steps to report
   int incprg  = nsolutes / 20;         // increment between reports
   incprg      = max( incprg,  1 );
   incprg      = min( incprg, 10 );
   int jstprg  = ( kstodo * incprg ) / nsolutes;  // steps for each report
   int kstep   = 0;                               // progress counter

   for ( int cc = 0; cc < nsolutes; cc++ )
   {
      int jjsa  = cc;
      int jssa  = cc * ntotal;
      int jssb  = cc * ntinois;
      int jjna  = jssa;
      int jjnb  = 0;

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
            sum_b  += ( ( nnls_b[ jjnb++ ] - a_bar [ rr ]     )
                      * ( nnls_a[ jjna++ ] - L_bars[ jjlb++ ] ) );
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

      if ( signal_wanted  &&  ++kstep == incprg )
      {
         emit work_progress( jstprg );
         kstodo   -= jstprg;
         kstep     = 0;
      }

      if ( abort ) return;
   }

   if ( signal_wanted  &&  kstodo > 0 )
      emit work_progress( kstodo );
DebugTime("END:ti-smab");
}

void US_SolveSim::compute_L_bar( QVector< double >&       L_bar,
                                 const QVector< double >& L,
                                 const QVector< double >& L_tilde )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();
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
void US_SolveSim::compute_a_bar( QVector< double >&       a_bar,
                                 const QVector< double >& a_tilde,
                                 const QVector< double >& nnls_b )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();
   double avgscale = 1.0 / (double)nscans;

   for ( int rr = 0; rr < npoints; rr++ )
   {
      int jb      = rr;

      // Note: a_tilde is always zero when rinoise has not been requested
      for ( int ss = 0; ss < nscans; ss++ )
      {
         a_bar[ rr ] += ( nnls_b[ jb ] - a_tilde[ ss ] );
         jb          += npoints;
      }

      a_bar[ rr ] *= avgscale;
   }
}

// Calculate the average simulated concentration at each radius point
void US_SolveSim::compute_L_bars( int                      nsolutes,
                                  int                      nrinois,
                                  int                      ntinois,
                                  int                      ntotal,
                                  QVector< double >&       L_bars,
                                  const QVector< double >& nnls_a,
                                  const QVector< double >& L_tildes )
{
   US_DataIO::EditedData* edata = &data_sets[ 0 ]->run_data;
   int npoints = edata->xvalues.size();
   int nscans  = edata->scanData.size();
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

// Debug message with thread/processor number and elapsed time value
void US_SolveSim::DebugTime( QString mtext )
{
   if ( dbg_timing )
   {
      qDebug() << "w" << thrnrank << "TM:" << mtext
         << startCalc.msecsTo( QDateTime::currentDateTime() ) / 1000.0;
   }
}

// Modify amplitude of data by thresholds and return flag if all-zero result
bool US_SolveSim::data_threshold( US_DataIO::RawData* sdata,
      double zerothr, double linethr, double maxod, double mfactor )
{
   int    nnzro   = 0;
   int    nzset   = 0;
   int    nntrp   = 0;
   int    nclip   = 0;
   int    npoints = sdata->xvalues.size();
   int    nscans  = sdata->scanData.size();
   double clipout = mfactor * maxod;
   double thrfact = mfactor / (double)( linethr - zerothr );
double maxs=0.0;
double maxsi=0.0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double avalue = sdata->value( ss, rr );
maxsi=qMax(maxsi,avalue);

         if ( avalue < zerothr )
         {  // Less than zero threshold:  set to zero
            avalue        = 0.0;
            nzset++;
         }

         else if ( avalue < linethr )
         {  // Between zero and linear threshold:  set to interpolated value
            avalue       *= ( ( avalue - zerothr ) * thrfact );
            nntrp++;
         }

         else if ( avalue < maxod )
         {  // Under maximum OD:  set to factor times input
            avalue       *= mfactor;
         }

         else
         {  // Over maximum OD;  set to factor times maximum
            avalue        = clipout;
            nclip++;
         }

         if ( avalue != 0.0 )
            nnzro++;
maxs=qMax(maxs,avalue);

         sdata->setValue( ss, rr, avalue );
      }
   }

   int lownnz = qRound( minnzsc * (double)( nscans * npoints ) );
   nnzro      = ( nnzro < lownnz ) ? 0 : nnzro;
DbgLv(1) << "  CR:THR: nnzro zs nt cl" << nnzro << nzset << nntrp << nclip;
//if(nnzro>0) {DbgLv(1) << "CR:THR: maxs" << maxs << maxsi << "mfact" << mfactor;}
//else        {DbgLv(1) << "CR:THz: maxs" << maxs << nnzro << "mfact" << mfactor;}

   return ( nnzro == 0 );
}

// Modify amplitude by thresholds and flag if all-zero (for experiment data)
bool US_SolveSim::data_threshold( US_DataIO::EditedData* edata,
      double zerothr, double linethr, double maxod, double mfactor )
{
   int    nnzro   = 0;
   int    npoints = edata->xvalues.size();
   int    nscans  = edata->scanData.size();
   double clipout = mfactor * maxod;
   double thrfact = mfactor / (double)( linethr - zerothr );

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double avalue = edata->value( ss, rr );

         if ( avalue < zerothr )
         {  // Less than zero threshold:  set to zero
            avalue        = 0.0;
         }

         else if ( avalue < linethr )
         {  // Between zero and linear threshold:  set to interpolated value
            avalue       *= ( ( avalue - zerothr ) * thrfact );
         }

         else if ( avalue < maxod )
         {  // Under maximum OD:  set to factor times input
            avalue       *= mfactor;
         }

         else
         {  // Over maximum OD;  set to factor times maximum
            avalue        = clipout;
         }

         if ( avalue != 0.0 )
            nnzro++;

         edata->setValue( ss, rr, avalue );
      }
   }

   return ( nnzro == 0 );
}

