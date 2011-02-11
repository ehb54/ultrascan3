//! \file us_eqmath.cpp

#include "us_eqmath.h"
#include "us_settings.h"
#include "us_math2.h"
#include "us_constants.h"
#include <cfloat>

const double dflt_min = (double)FLT_MIN;
const double dflt_max = (double)FLT_MAX;

// Main constructor:  pass references to EditedData, ScanEdit, EqScanFit,
//  and EqRunFit objects needed by methods of this EqMath object
US_EqMath::US_EqMath(
      QVector< US_DataIO2::EditedData >& dataList,
      QVector< ScanEdit >&               scedits,
      QVector< EqScanFit >&              scanfits,
      EqRunFit&                          runfit )
 : QObject(),
   dataList   ( dataList ),
   scedits    ( scedits  ),
   scanfits   ( scanfits ),
   runfit     ( runfit   )
{

   dbg_level = US_Settings::us_debug();
}

// Initialize parameters
void US_EqMath::init_params( int modelx, bool update_mw,
   QList< double >& ds_vbar20s, QList< double >&aud_pars )
{
   // Find the index to data corresponding to the first fitted scan
   int  jdx       = -1;

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( scanfits[ ii ].scanFit  &&  jdx < 0 )
      {
         jdx = scedits[ ii ].dsindex;
         break;
      }
   }

DbgLv(1) << "EM:IP: jdx" << jdx << "modelx" << modelx << "update_mw" << update_mw;
   if ( jdx < 0 )  return;

   double molecwt  = runfit.mw_vals[ 0 ];
   double portion  = molecwt;
   double mwll;
   double mwul;
   double uvbar;
   double dnumc;
   double mwinc;
   double tempa;
   double total;

   // Set runfit and scanfits parameters for the selected model
   switch( modelx )
   {
      case 0:     //  0: "1-Component, Ideal"
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ 0 ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         break;
      case 1:     //  1: "2-Component, Ideal, Noninteracting"
         if ( update_mw )
         {
            runfit.mw_vals[ 0 ]    = portion - ( portion * 0.2 );
            runfit.mw_vals[ 1 ]    = portion + ( portion * 0.2 );
         }

         for ( int ii = 0; ii < runfit.nbr_comps; ii++ )
         {
            runfit.mw_rngs  [ ii ] = runfit.mw_vals[ ii ] * 0.2;
            runfit.vbar_vals[ ii ] = ds_vbar20s[ jdx ];
            runfit.vbar_rngs[ ii ] = runfit.vbar_vals[ ii ] * 0.2;
         }

         if ( (int)( 100.0 * runfit.vbar_vals[ 1 ] ) == 72 )
         {
            runfit.vbar_vals[ 1 ]  = runfit.vbar_vals[ 0 ];
            runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;
         }

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            portion = exp( scanfits[ ii ].amp_vals[ 0 ] ) * 0.5;
            scanfits[ ii ].amp_vals[ 0 ] = log( portion * 0.7 );
            scanfits[ ii ].amp_vals[ 1 ] = log( portion * 0.3 );
            scanfits[ ii ].amp_rngs[ 0 ] = scanfits[ ii ].amp_vals[ 0 ] * 0.2;
            scanfits[ ii ].amp_rngs[ 1 ] = scanfits[ ii ].amp_rngs[ 0 ];
         }
         break;
      case 2:     //  2: "3-Component, Ideal, Noninteracting"
         if ( update_mw )
         {
            runfit.mw_vals[ 0 ]    = molecwt - ( molecwt * 0.2 );
            runfit.mw_vals[ 1 ]    = molecwt;
            runfit.mw_vals[ 2 ]    = molecwt + ( molecwt * 0.2 );
         }

DbgLv(1) << "EM:IP: C2: nbrcomps" << runfit.nbr_comps;
         for ( int ii = 0; ii < runfit.nbr_comps; ii++ )
         {
            runfit.mw_rngs  [ ii ] = runfit.mw_vals[ ii ] * 0.2;
            runfit.vbar_vals[ ii ] = ds_vbar20s[ jdx ];
            runfit.vbar_rngs[ ii ] = runfit.vbar_vals[ ii ] * 0.2;
         }

         if ( (int)( 100.0 * runfit.vbar_vals[ 1 ] ) == 72 )
         {
            runfit.vbar_vals[ 1 ]  = runfit.vbar_vals[ 0 ];
            runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;
         }
         if ( (int)( 100.0 * runfit.vbar_vals[ 2 ] ) == 72 )
         {
            runfit.vbar_vals[ 2 ]  = runfit.vbar_vals[ 0 ];
            runfit.vbar_rngs[ 2 ]  = runfit.vbar_vals[ 2 ] * 0.2;
         }

         total = runfit.mw_vals[ 0 ] + runfit.mw_vals[ 1 ]
               + runfit.mw_vals[ 2 ];

DbgLv(1) << "EM:IP: C2: total" << total;
         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            portion = exp( scanfits[ ii ].amp_vals[ 0 ] ) / 3.0;
            scanfits[ ii ].amp_vals[ 0 ] = log( portion * 0.6 );
            scanfits[ ii ].amp_vals[ 1 ] = log( portion * 0.3 );
            scanfits[ ii ].amp_vals[ 2 ] = log( portion * 0.1 );
            scanfits[ ii ].amp_rngs[ 0 ] = scanfits[ ii ].amp_vals[ 0 ] * 0.2;
            scanfits[ ii ].amp_rngs[ 1 ] = scanfits[ ii ].amp_rngs[ 0 ];
            scanfits[ ii ].amp_rngs[ 2 ] = scanfits[ ii ].amp_rngs[ 0 ];
         }
         break;
      case 3:     //  3: "Fixed Molecular Weight Distribution"
         mwll    = aud_pars[ 1 ];
         mwul    = aud_pars[ 2 ];
         uvbar   = aud_pars[ 3 ];
         uvbar   = uvbar > 0.0 ? uvbar : TYPICAL_VBAR;
         dnumc   = (double)runfit.nbr_comps;
         mwinc   = ( mwul - mwll ) / ( dnumc - 1.0 );
         tempa   = log( 1.0e-7 / dnumc );

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
            {
               scanfits[ ii ].amp_vals[ jj ] = tempa;
               scanfits[ ii ].amp_rngs[ jj ] =
                  scanfits[ ii ].amp_vals[ jj ] * 0.2;
               scanfits[ ii ].baseline       = 0.0;
               scanfits[ ii ].baseln_rng     = 0.05;
               runfit.mw_vals[ jj ]    = mwll + ( (double)jj * mwinc );
               runfit.vbar_vals[ jj ]  = uvbar;
               runfit.vbar_rngs[ jj ]  = uvbar * 0.2;
            }
         }
         break;
      case 4:     //  4: "Monomer-Dimer Equilibrium"
      case 5:     //  5: "Monomer-Trimer Equilibrium"
      case 6:     //  6: "Monomer-Tetramer Equilibrium"
      case 7:     //  7: "Monomer-Pentamer Equilibrium"
      case 8:     //  8: "Monomer-Hexamer Equilibrium"
      case 9:     //  9: "Monomer-Heptamer Equilibrium"
      case 10:    // 10: "User-Defined Monomer-Nmer Equilibrium"
      case 11:    // 11: "Monomer-Dimer-Trimer Equilibrium"
      case 12:    // 12: "Monomer-Dimer-Tetramer Equilibrium"
      case 13:    // 13: "User-Defined Monomer - N-mer - M-mer Equilibrium"
         runfit.eq_vals[ 0 ]     = -1.0;
         runfit.eq_vals[ 1 ]     = -1.0e4;
         runfit.eq_rngs[ 0 ]     = 5.0;
         runfit.eq_rngs[ 1 ]     = 5.0;
         runfit.vbar_vals[ 0 ]   = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]   = runfit.vbar_vals[ 0 ] * 0.2;
         break;
      case 14:    // 14: "2-Component Hetero-Association: A + B <=> AB"
         if ( update_mw )
         {
            runfit.mw_vals[ 0 ]    = molecwt - ( molecwt * 0.2 );
            runfit.mw_vals[ 1 ]    = molecwt + ( molecwt * 0.2 );
         }

         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;

         if ( (int)( 100.0 * runfit.vbar_vals[ 1 ] ) == 72 )
         {
            runfit.vbar_vals[ 1 ]  = runfit.vbar_vals[ 0 ];
         }

         runfit.eq_vals[ 0 ]    = -1.0e4;
         runfit.eq_rngs[ 0 ]    = 5.0e2;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            portion = exp( scanfits[ ii ].amp_vals[ 0 ] ) / 2.0;
            scanfits[ ii ].amp_vals[ 0 ] = log( portion );
            scanfits[ ii ].amp_vals[ 1 ] = scanfits[ ii ].amp_vals[ 0 ];
            scanfits[ ii ].amp_rngs[ 0 ] = scanfits[ ii ].amp_vals[ 0 ] * 0.2;
            scanfits[ ii ].amp_rngs[ 1 ] = scanfits[ ii ].amp_rngs[ 0 ];
         }
         break;
      case 15:    // 15: "U-Defined self/Hetero-Assoc.: A + B <=> AB, nA <=> An"
         if ( update_mw )
         {
            runfit.mw_vals[ 0 ]    = molecwt - ( molecwt * 0.2 );
            runfit.mw_vals[ 1 ]    = molecwt + ( molecwt * 0.2 );
         }

         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;

         if ( (int)( 100.0 * runfit.vbar_vals[ 1 ] ) == 72 )
         {
            runfit.vbar_vals[ 1 ]  = runfit.vbar_vals[ 0 ];
         }

         runfit.eq_vals[ 0 ]    = -1.0e4;
         runfit.eq_rngs[ 0 ]    = 5.0e2;
         runfit.eq_vals[ 1 ]    = -1.0e4;
         runfit.eq_rngs[ 1 ]    = 5.0e2;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            portion = exp( scanfits[ ii ].amp_vals[ 0 ] ) / 2.0;
            scanfits[ ii ].amp_vals[ 0 ] = log( portion );
            scanfits[ ii ].amp_vals[ 1 ] = scanfits[ ii ].amp_vals[ 0 ];
            scanfits[ ii ].amp_rngs[ 0 ] = scanfits[ ii ].amp_vals[ 0 ] * 0.2;
            scanfits[ ii ].amp_rngs[ 1 ] = scanfits[ ii ].amp_rngs[ 0 ];
         }
         break;
      case 16:    // 16: "U-Defined Monomer-Nmer, some monomer is incompetent"
         runfit.mw_vals  [ 1 ]  = runfit.mw_vals[ 0 ];
         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;
         runfit.eq_vals[ 0 ]    = -0.1;
         runfit.eq_rngs[ 0 ]    = 5.0;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            scanfits[ ii ].amp_vals[ 1 ] = -1.0e4;
            scanfits[ ii ].amp_rngs[ 1 ] = 1.0e-3;
         }
         break;
      case 17:    // 17: "User-Defined Monomer-Nmer, some Nmer is incompetent"
         runfit.mw_vals  [ 1 ]  = runfit.stoichs[ 0 ] * runfit.mw_vals[ 0 ];
         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;
         runfit.eq_vals[ 0 ]    = -0.1;
         runfit.eq_rngs[ 0 ]    = 5.0;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            scanfits[ ii ].amp_vals[ 1 ] = -1.0e4;
            scanfits[ ii ].amp_rngs[ 1 ] = 1.0e-3;
         }
         break;
      case 18:    // 18: "User-Defined irreversible Monomer-Nmer"
         runfit.mw_vals  [ 1 ]  = runfit.stoichs[ 0 ] * runfit.mw_vals[ 0 ];
         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            portion = exp( scanfits[ ii ].amp_vals[ 0 ] ) / 2.0;
            scanfits[ ii ].amp_vals[ 0 ] = log( portion * 0.7 );
            scanfits[ ii ].amp_vals[ 1 ] = log( portion * 0.3 );
            scanfits[ ii ].amp_rngs[ 0 ] = scanfits[ ii ].amp_vals[ 0 ] * 0.2;
            scanfits[ ii ].amp_rngs[ 1 ] = scanfits[ ii ].amp_rngs[ 0 ];
         }
         break;
      case 19:    // 19: "User-Defined Monomer-Nmer plus contaminant"
         runfit.mw_vals  [ 1 ]  = runfit.mw_vals[ 0 ];
         runfit.mw_rngs  [ 0 ]  = runfit.mw_vals[ 0 ] * 0.2;
         runfit.mw_rngs  [ 1 ]  = runfit.mw_vals[ 1 ] * 0.2;
         runfit.vbar_vals[ 0 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 0 ]  = runfit.vbar_vals[ 0 ] * 0.2;
         runfit.vbar_vals[ 1 ]  = ds_vbar20s[ jdx ];
         runfit.vbar_rngs[ 1 ]  = runfit.vbar_vals[ 1 ] * 0.2;
         runfit.eq_vals[ 0 ]    = -1.0e4;
         runfit.eq_rngs[ 0 ]    = 5.0;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            if ( ! scanfits[ ii ].scanFit )  continue;

            scanfits[ ii ].amp_vals[ 1 ] = -1.0e4;
            scanfits[ ii ].amp_rngs[ 1 ] = 1.0e-3;
         }
         break;
   }
}

// Initialize for fit
void US_EqMath::init_fit( int modx, int methx )
{
   modelx   = modx;     // Model type index
   nlsmeth  = methx;    // NLS method index

   // Find the index to the first fitted scan and count data sets, points
   ffitx    = -1;
   ntpts    = 0;
   ndsets   = 0;
   nfpars   = 0;
   nspts    = 0;
   nslpts   = 0;
   v_setpts .clear();
   v_setlpts.clear();

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( ! scanfits[ ii ].scanFit )  continue;

      if ( ffitx < 0 )                 ffitx    = ii;

      nspts    = scanfits[ ii ].stop_ndx - scanfits[ ii ].start_ndx + 1;
      v_setpts  << nspts;
      v_setlpts << nspts;
      ndsets++;
      ntpts   += nspts;
   }

   if ( ffitx < 0  ||  ntpts == 0 )    return;

   // Count the number of fitting parameters
   for ( int ii = 0; ii < runfit.nbr_comps; ii++ )
   {
      if ( runfit.mw_fits[ ii ] == true  ||  runfit.vbar_fits[ ii ] == true )
         nfpars++;

      if ( runfit.viri_fits[ ii ] == true )
         nfpars++;
   }

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( ! scanfits[ ii ].scanFit )  continue;

      EqScanFit* scnf = &scanfits[ ii ];

      for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
         if ( scnf->amp_fits[ jj ] )
            nfpars++;

      if ( scnf->baseln_fit )
         nfpars++;
   }

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
      if ( runfit.eq_fits[ jj ] )
         nfpars++;

   // Allocate vectors and matrices
   v_yraw  .fill( 0.0, ntpts  );
   v_yguess.fill( 0.0, ntpts  );
   v_ydelta.fill( 0.0, ntpts  );
   v_BB    .fill( 0.0, nfpars );
   v_guess .fill( 0.0, nfpars );
   v_tguess.fill( 0.0, nfpars );

   m_jacobi.fill(   0, ntpts  );
   m_info  .fill(   0, nfpars );
   m_LLtrns.fill(   0, nfpars );
   m_dcr2  .fill(   0, ndsets );
   m_dlncr2.fill(   0, ndsets );
   m_lncr2 .fill(   0, ndsets );

   int njacvs = ntpts  * nfpars;
   int nfpsq  = nfpars * nfpars;
   v_jacobi.fill( 0.0, njacvs );
   v_info  .fill( 0.0, nfpsq  );
   v_LLtrns.fill( 0.0, nfpsq  );
   v_lncr2 .fill( 0.0, ntpts  );
   v_dlncr2.fill( 0.0, ntpts  );
   v_dcr2  .fill( 0.0, ntpts  );

   int dsx    = 0;
   int ptx    = 0;
   d_lncr2    = v_lncr2 .data();
   d_dlncr2   = v_dlncr2.data();
   d_dcr2     = v_dcr2  .data();

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( ! scanfits[ ii ].scanFit )  continue;

      EqScanFit* scnf = &scanfits[ ii ];

      m_dlncr2[ dsx ] = d_dlncr2;
      m_lncr2 [ dsx ] = d_lncr2;
      m_dcr2  [ dsx ] = d_dcr2;

      nspts      = v_setpts[ dsx++ ];
      d_dlncr2  += nspts;
      d_lncr2   += nspts;
      d_dcr2    += nspts;
      int jy     = scnf->start_ndx;

      for ( int jj = 0; jj < nspts; jj++ )
         v_yraw[ ptx++ ] = scnf->yvs[ jy++ ];
   }

   // Initialize parameter guess
   guess_mapForward( );

   // Set up jacobian, info, and LLtranspose matrices
   d_jacobi = v_jacobi.data();
   d_info   = v_info  .data();
   d_LLtrns = v_LLtrns.data();

   for ( int ii = 0; ii < ntpts; ii++ )
   {
      m_jacobi[ ii ] = d_jacobi;
      d_jacobi      += nfpars;
   }

   for ( int ii = 0; ii < nfpars; ii++ )
   {
      m_info  [ ii ] = d_info;
      m_LLtrns[ ii ] = d_LLtrns;
      d_info        += nfpars;
      d_LLtrns      += nfpars;
   }

DbgLv(1) << "EM:FI: ffitx" << ffitx << "modelx" << modelx;
}

// Fill in guess parameter vector
//   Parameter order (needs to be maintained so Jacobian columns match):
//     for each component k:
//       1. Molecular Weight ( k )
//       2. Vbar ( k )
//       3. Virial Coefficient ( k )
//
//     for each scan:
//       for each component:
//         4. Amplitude
//       5. Baseline
//
//     for each association constant:
//       6. Association constant
//
void US_EqMath::guess_mapForward()
{
   int jpx = 0;

   for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
   {
      if ( runfit.mw_fits[ jj ] )
      {
         v_guess[ jpx ] = runfit.mw_vals[ jj ];
         runfit.mw_ndxs[ jj ] = jpx++;
      }

      if ( runfit.vbar_fits[ jj ] )
      {
         v_guess[ jpx ] = runfit.vbar_vals[ jj ];
         runfit.vbar_ndxs[ jj ] = jpx++;
      }

      if ( runfit.viri_fits[ jj ] )
      {
         v_guess[ jpx ] = runfit.viri_vals[ jj ];
         runfit.viri_ndxs[ jj ] = jpx++;
      }
   }

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( ! scanfits[ ii ].scanFit )  continue;

      EqScanFit* scnf = &scanfits[ ii ];

      for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
      {
         if ( scnf->amp_fits[ jj ] )
         {
            v_guess[ jpx ] = scnf->amp_vals[ jj ];
            scnf->amp_ndxs[ jj ] = jpx++;
         }

         if ( scnf->baseln_fit )
         {
            v_guess[ jpx ] = scnf->baseline;
            scnf->baseln_ndx = jpx++;
         }
      }
   }

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
   {
      if ( runfit.eq_fits[ jj ] )
      {
         v_guess[ jpx ] = runfit.eq_vals[ jj ];
         runfit.eq_ndxs[ jj ] = jpx++;
      }
   }
}

// Map parameters back from guesses
void US_EqMath::parameter_mapBackward()
{
   int jpx = 0;

   for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
   {
      if ( runfit.mw_fits[ jj ] )
         runfit.mw_vals[ jj ]   = v_guess[ jpx++ ];

      if ( runfit.vbar_fits[ jj ] )
         runfit.vbar_vals[ jj ] = v_guess[ jpx++ ];

      if ( runfit.viri_fits[ jj ] )
         runfit.viri_vals[ jj ] = v_guess[ jpx++ ];
   }

   for ( int ii = 0; ii < scanfits.size(); ii++ )
   {
      if ( ! scanfits[ ii ].scanFit )  continue;

      EqScanFit* scnf = &scanfits[ ii ];

      for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
      {
         if ( scnf->amp_fits[ jj ] )
            scnf->amp_vals[ jj ] = v_guess[ jpx++ ];

         if ( scnf->baseln_fit )
            scnf->baseline       = v_guess[ jpx++ ];
      }
   }

   for ( int jj = 0; jj < runfit.nbr_assocs; jj++ )
      if ( runfit.eq_fits[ jj ] )
         runfit.eq_vals[ jj ] = v_guess[ jpx++ ];

   if ( modelx == 3 )
   {
      EqScanFit* sc1f = &scanfits[ ffitx ];

      for ( int ii = ffitx + 1; ii < scanfits.size(); ii++ )
      {
         EqScanFit* scnf = &scanfits[ ii ];

         if ( ! scnf->scanFit )   continue;

         for ( int jj = 0; jj < runfit.nbr_comps; jj++ )
            scnf->amp_vals[ jj ] = sc1f->amp_vals[ jj ];
      }
   }

}

// Calculate the jacobian matrix
int US_EqMath::calc_jacobian()
{
   int stat   = 0;
   int ncomps = runfit.nbr_comps;
   int mcomp  = max( ncomps, 4 );
   int jpx    = 0;
   int jdx    = 0;
   QVector< double > v_ufunc( mcomp );
   QVector< double > v_vbar ( mcomp );
   QVector< double > v_buoy ( mcomp );

   v_jacobi.fill( 0.0, ntpts * nfpars );

   switch( modelx )
   {
      case 0:
      case 1:
      case 2:
      case 3:
         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            EqScanFit* scnf = &scanfits[ ii ];

            if ( ! scnf->scanFit )  continue;

            int    jstx     = scnf->start_ndx;
            double xm_sqr   = sq( scnf->xvs[ jstx ] );
            double omg_sqr  = sq( M_PI * scnf->rpm / 30.0 );
            double tempera  = scnf->tempera;
            double density  = scnf->density;
            double dconst   = omg_sqr / ( 2.0 * R * ( K0 + tempera ) );

            for ( int kk = 0; kk < ncomps; kk++ )
            {
               v_vbar[ kk ]  = US_Math2::adjust_vbar20( runfit.vbar_vals[ kk ],
                                                        tempera );
               v_buoy[ kk ]  = ( 1.0 - v_vbar[ kk ] * density );
            }

            for ( int jj = jstx; jj < jstx + v_setpts[ jdx ]; jj++ )
            {
               double xv     = sq( scnf->xvs[ jj ] ) - xm_sqr;

               for ( int kk = 0; kk < ncomps; kk++ )
               {
                  double buoy   = v_buoy[ kk ];
                  double mwv    = runfit.mw_vals[ kk ];
                  double ampv   = scnf->amp_vals[ kk ];
                  double ufunc  = exp( ampv + dconst * mwv * buoy * xv );
                  v_ufunc[ kk ] = ufunc;

                  if ( runfit.mw_fits[ kk ] )
                     m_jacobi[ jpx ][ runfit.mw_ndxs[ kk ] ]
                        = dconst * xv * buoy * ufunc;

                  if ( runfit.vbar_fits[ kk ] )
                     m_jacobi[ jpx ][ runfit.vbar_ndxs[ kk ] ]
                        = (-1.0 ) * dconst * mwv * xv * ufunc * density;

                  if ( scnf->amp_fits[ kk ] )
                     m_jacobi[ jpx ][ scnf->amp_ndxs[ kk ] ]
                        = ufunc;
               }

               if ( scnf->baseln_fit )
                  m_jacobi[ jpx ][ scnf->baseln_ndx ] = 1.0;

               jpx++;
            }

            jdx++;
         }
         break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      {
         double stoich1      = (double)( modelx - 2 );
         double stoiexp      = stoich1 - 1.0;
         runfit.stoichs[ 0 ] = stoich1;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            EqScanFit* scnf = &scanfits[ ii ];

            if ( ! scnf->scanFit )  continue;

            int    jstx     = scnf->start_ndx;
            double xm_sqr   = sq( scnf->xvs[ jstx ] );
            double omg_sqr  = sq( M_PI * scnf->rpm / 30.0 );
            double tempera  = scnf->tempera;
            double density  = scnf->density;
            double dconst   = omg_sqr / ( 2.0 * R * ( K0 + tempera ) );
            double ODcorrec = log( stoich1 / pow( scnf->extincts[ 0 ]
                                   * scnf->pathlen , stoiexp ) );
            v_vbar[ 0 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 0 ],
                                                       tempera );
            v_buoy[ 0 ]     = ( 1.0 - v_vbar[ 0 ] * density );

            for ( int jj = jstx; jj < jstx + v_setpts[ jdx ]; jj++ )
            {
               double xv     = sq( scnf->xvs[ jj ] ) - xm_sqr;
               double buoy   = v_buoy[ 0 ];
               double mwv    = runfit.mw_vals[ 0 ];
               double ampv   = scnf->amp_vals[ 0 ];
               double ufunc0 = exp( ampv + dconst * mwv * buoy * xv );
               double ufunc1 = exp( stoich1 * ampv + runfit.eq_vals[ 0 ] +
                  ODcorrec + dconst * stoich1 * mwv * buoy * ufunc0 );
               v_ufunc[ 0 ]  = ufunc0;
               v_ufunc[ 1 ]  = ufunc1;
               double dcoeff = dconst * xv * buoy;

               if ( runfit.mw_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.mw_ndxs[ 0 ] ]
                     = dcoeff * ufunc0 + dcoeff * ufunc1 * stoich1;

               dcoeff        = dconst * mwv * xv * density;

               if ( runfit.vbar_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.vbar_ndxs[ 0 ] ]
                     = (-1.0 ) * dcoeff * ufunc0 - dcoeff * ufunc1 * stoich1;

               if ( scnf->amp_fits[ 0 ] )
                  m_jacobi[ jpx ][ scnf->amp_ndxs[ 0 ] ]
                     = ufunc0 + ufunc1 * stoich1;

               if ( runfit.eq_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 0 ] ] = ufunc1;

               if ( scnf->baseln_fit )
                  m_jacobi[ jpx ][ scnf->baseln_ndx ] = 1.0;

               jpx++;
            }

            jdx++;
         }
         break;
      }
      case 11:
      case 12:
      case 13:
      {
         double stoich1      = 2.0;
         double stoich2      = (double)( modelx - 8 );
         runfit.stoichs[ 0 ] = stoich1;
         runfit.stoichs[ 1 ] = stoich2;
         double stoiexp      = stoich1 - 1.0;
         double stoiex2      = stoich2 - 1.0;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            EqScanFit* scnf = &scanfits[ ii ];

            if ( ! scnf->scanFit )  continue;

            int    jstx     = scnf->start_ndx;
            double xm_sqr   = sq( scnf->xvs[ jstx ] );
            double omg_sqr  = sq( M_PI * scnf->rpm / 30.0 );
            double tempera  = scnf->tempera;
            double density  = scnf->density;
            double dconst   = omg_sqr / ( 2.0 * R * ( K0 + tempera ) );
            double ODcorec1 = log( stoich1 / pow( scnf->extincts[ 0 ]
                                   * scnf->pathlen , stoiexp ) );
            double ODcorec2 = log( stoich2 / pow( scnf->extincts[ 0 ]
                                   * scnf->pathlen , stoiex2 ) );
            v_vbar[ 0 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 0 ],
                                                       tempera );
            v_buoy[ 0 ]     = ( 1.0 - v_vbar[ 0 ] * density );

            for ( int jj = jstx; jj < jstx + v_setpts[ jdx ]; jj++ )
            {
               double xv     = sq( scnf->xvs[ jj ] ) - xm_sqr;
               double buoy   = v_buoy[ 0 ];
               double mwv    = runfit.mw_vals[ 0 ];
               double ampv   = scnf->amp_vals[ 0 ];
               double ufunc0 = exp( ampv + dconst * mwv * buoy * xv );
               double ufunc1 = exp( stoich1 * ampv + runfit.eq_vals[ 0 ] +
                  ODcorec1 + dconst * stoich1 * mwv * buoy * xv );
               double ufunc2 = exp( stoich2 * ampv + runfit.eq_vals[ 1 ] +
                  ODcorec2 + dconst * stoich2 * mwv * buoy * xv );
               v_ufunc[ 0 ]  = ufunc0;
               v_ufunc[ 1 ]  = ufunc1;
               v_ufunc[ 2 ]  = ufunc2;
               double dcoeff = dconst * xv * buoy;

               if ( runfit.mw_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.mw_ndxs[ 0 ] ]
                     = dcoeff * ufunc0 * + dcoeff * ufunc1 * stoich1
                                         + dcoeff * ufunc2 * stoich2;

               dcoeff        = dconst * mwv * xv * density;

               if ( runfit.vbar_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.vbar_ndxs[ 0 ] ]
                     = (-1.0 ) * dcoeff * ufunc0 - dcoeff * ufunc1 * stoich1
                                                 - dcoeff * ufunc2 * stoich2;

               if ( scnf->amp_fits[ 0 ] )
                  m_jacobi[ jpx ][ scnf->amp_ndxs[ 0 ] ]
                     = ufunc0 + ufunc1 * stoich1 + ufunc2 * stoich2;

               if ( runfit.eq_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 0 ] ] = ufunc1;

               if ( runfit.eq_fits[ 1 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 1 ] ] = ufunc2;

               if ( scnf->baseln_fit )
                  m_jacobi[ jpx ][ scnf->baseln_ndx ]    = 1.0;

               jpx++;
            }

            jdx++;
         }
         break;
      }
      case 14:
      {
         double mwv0         = runfit.mw_vals[ 0 ];
         double mwv1         = runfit.mw_vals[ 1 ];
         double mw_ab        = mwv0 + mwv1;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            EqScanFit* scnf = &scanfits[ ii ];

            if ( ! scnf->scanFit )  continue;

            int    jstx     = scnf->start_ndx;
            double xm_sqr   = sq( scnf->xvs[ jstx ] );
            double omg_sqr  = sq( M_PI * scnf->rpm / 30.0 );
            double tempera  = scnf->tempera;
            double density  = scnf->density;
            double dconst   = omg_sqr / ( 2.0 * R * ( K0 + tempera ) );
            double extinc0  = scnf->extincts[ 0 ];
            double extinc1  = scnf->extincts[ 1 ];
            double ODcorrec = log( ( extinc0 + extinc1 ) /
                                   ( scnf->pathlen * extinc0 * extinc1 ) );
            v_vbar[ 0 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 0 ],
                                                       tempera );
            v_vbar[ 1 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 1 ],
                                                       tempera );
            v_vbar[ 2 ]     = ( v_vbar[ 0 ] * mwv0 + v_vbar[ 1 ] * mwv1 )
                              / mw_ab;
            v_buoy[ 0 ]     = ( 1.0 - v_vbar[ 0 ] * density );
            v_buoy[ 1 ]     = ( 1.0 - v_vbar[ 1 ] * density );
            v_buoy[ 2 ]     = ( 1.0 - v_vbar[ 2 ] * density );

            for ( int jj = jstx; jj < jstx + v_setpts[ jdx ]; jj++ )
            {
               double xv     = sq( scnf->xvs[ jj ] ) - xm_sqr;
               double buoy0  = v_buoy[ 0 ];
               double buoy1  = v_buoy[ 1 ];
               double buoy2  = v_buoy[ 2 ];
               double ampv0  = scnf->amp_vals[ 0 ];
               double ampv1  = scnf->amp_vals[ 1 ];
               double constx = dconst * xv;
               double ufunc0 = exp( ampv0 + constx * mwv0 * buoy0 );
               double ufunc1 = exp( ampv1 + constx * mwv1 * buoy1 );
               double ufunc2 = exp( ampv0 + ampv1 + runfit.eq_vals[ 0 ] +
                  ODcorrec + constx * mw_ab * buoy2 );
               v_ufunc[ 0 ]  = ufunc0;
               v_ufunc[ 1 ]  = ufunc1;
               v_ufunc[ 2 ]  = ufunc2;

               if ( runfit.mw_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.mw_ndxs[ 0 ] ]
                     = constx * buoy0 * ufunc0 +
                       constx * buoy2 +
                       constx * ( v_vbar[ 2 ] * density -
                                  v_vbar[ 0 ] * density ) + ufunc2;

               if ( runfit.vbar_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.vbar_ndxs[ 0 ] ]
                     = (-1.0 ) * constx * mwv1 * density * ufunc1
                               - constx * mwv1 * density * ufunc2;

               if ( scnf->amp_fits[ 0 ] )
                  m_jacobi[ jpx ][ scnf->amp_ndxs[ 0 ] ]
                     = ufunc0 + ufunc2;

               if ( runfit.eq_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 0 ] ] = ufunc2;

               if ( scnf->baseln_fit )
                  m_jacobi[ jpx ][ scnf->baseln_ndx ]    = 1.0;

               jpx++;
            }

            jdx++;
         }
         break;
      }
      case 15:
      {
         double mwv0         = runfit.mw_vals[ 0 ];
         double mwv1         = runfit.mw_vals[ 1 ];
         double mw_ab        = mwv0 + mwv1;
         double stoich1      = runfit.stoichs[ 0 ];
         double stoiexp      = stoich1 - 1.0;

         for ( int ii = 0; ii < scanfits.size(); ii++ )
         {
            EqScanFit* scnf = &scanfits[ ii ];

            if ( ! scnf->scanFit )  continue;

            int    jstx     = scnf->start_ndx;
            double xm_sqr   = sq( scnf->xvs[ jstx ] );
            double omg_sqr  = sq( M_PI * scnf->rpm / 30.0 );
            double tempera  = scnf->tempera;
            double density  = scnf->density;
            double dconst   = omg_sqr / ( 2.0 * R * ( K0 + tempera ) );
            double extinc0  = scnf->extincts[ 0 ];
            double extinc1  = scnf->extincts[ 1 ];
            double ODcorec1 = log( ( extinc0 + extinc0 ) /
                                   ( scnf->pathlen * extinc0 * extinc1 ) );
            double ODcorec2 = log( stoich1 /
                                   pow( scnf->pathlen * extinc0, stoiexp ) );
            v_vbar[ 0 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 0 ],
                                                       tempera );
            v_vbar[ 1 ]     = US_Math2::adjust_vbar20( runfit.vbar_vals[ 1 ],
                                                       tempera );
            v_vbar[ 2 ]     = ( v_vbar[ 0 ] * mwv0 + v_vbar[ 1 ] * mwv1 )
                              / mw_ab;
            v_buoy[ 0 ]     = ( 1.0 - v_vbar[ 0 ] * density );
            v_buoy[ 1 ]     = ( 1.0 - v_vbar[ 1 ] * density );
            v_buoy[ 2 ]     = ( 1.0 - v_vbar[ 2 ] * density );

            for ( int jj = jstx; jj < jstx + v_setpts[ jdx ]; jj++ )
            {
               double xv     = sq( scnf->xvs[ jj ] ) - xm_sqr;
               double buoy0  = v_buoy[ 0 ];
               double buoy1  = v_buoy[ 1 ];
               double buoy2  = v_buoy[ 2 ];
               double ampv0  = scnf->amp_vals[ 0 ];
               double ampv1  = scnf->amp_vals[ 1 ];
               double constx = dconst * xv;
               double ufunc0 = exp( ampv0 + constx * mwv0 * buoy0 );
               double ufunc1 = exp( ampv1 + constx * mwv1 * buoy1 );
               double ufunc2 = exp( ampv0 + ampv1 + runfit.eq_vals[ 0 ] +
                  ODcorec1 + constx * mw_ab * buoy2 );
               double ufunc3 = exp( stoich1 * ampv0 + runfit.eq_vals[ 0 ] +
                  ODcorec2 + constx * stoich1 * mw_ab * buoy0 );
               v_ufunc[ 0 ]  = ufunc0;
               v_ufunc[ 1 ]  = ufunc1;
               v_ufunc[ 2 ]  = ufunc2;
               v_ufunc[ 3 ]  = ufunc3;

               if ( runfit.mw_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.mw_ndxs[ 0 ] ]
                     = constx * buoy0 * ufunc0 +
                       constx * buoy2 +
                       constx * ( v_vbar[ 2 ] * density -
                                  v_vbar[ 0 ] * density ) +
                       constx * buoy0 * ufunc3 * stoich1 + ufunc2;

               if ( runfit.mw_fits[ 1 ] )
                  m_jacobi[ jpx ][ runfit.mw_ndxs[ 1 ] ]
                     = constx * buoy1 * ufunc1 +
                       constx * buoy2 +
                       constx * ( v_vbar[ 2 ] * density -
                                  v_vbar[ 1 ] * density ) + ufunc2;

               if ( runfit.vbar_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.vbar_ndxs[ 0 ] ]
                     = (-1.0 ) * constx * mwv0 * density * ufunc0
                               - constx * mwv0 * density * ufunc3 * stoich1
                               - constx * mwv0 * density * ufunc2;

               if ( runfit.vbar_fits[ 1 ] )
                  m_jacobi[ jpx ][ runfit.vbar_ndxs[ 1 ] ]
                     = (-1.0 ) * constx * mwv1 * density * ufunc1
                               - constx * mwv1 * density * ufunc2;

               if ( scnf->amp_fits[ 0 ] )
                  m_jacobi[ jpx ][ scnf->amp_ndxs[ 0 ] ]
                     = ufunc0 + ufunc3 * stoich1 + ufunc2;

               if ( scnf->amp_fits[ 1 ] )
                  m_jacobi[ jpx ][ scnf->amp_ndxs[ 1 ] ]
                     = ufunc1 + ufunc2;

               if ( runfit.eq_fits[ 0 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 0 ] ] = ufunc2;

               if ( runfit.eq_fits[ 1 ] )
                  m_jacobi[ jpx ][ runfit.eq_ndxs[ 1 ] ] = ufunc3;

               if ( scnf->baseln_fit )
                  m_jacobi[ jpx ][ scnf->baseln_ndx ]    = 1.0;

               jpx++;
            }

            jdx++;
         }
         break;
      }
      case 16:
         break;
      case 17:
         break;
      case 18:
         break;
      case 19:
         break;
   }

   return stat;
}

// Calculate the chi-squared for the fixed molecular weight estimate
// for a single component model
double US_EqMath::calc_testParameter( double mwval )
{
   int    points = 0;
   double chi_sq = 0.0;
   double x_temp = 0.0;
   double dconst;
   double buoyancy_tb;
   double omega_s;
   double x_val;
   double y_val;

   QVector< double* > mmat;    // M matrix (vector of pointers to arrays)
   QVector< double >  mvec;    // Concatenated matrix doubles
   QVector< double >  yrvec;   // Raw Y-values vector
   QVector< double >  covec;   // Coefficients vector
   QVector< QPointF > xyvec;   // X,Y vector (of points)

   // Determine maximum number of points for vectors
   for ( int jes = 0; jes < scanfits.size(); jes++ )
      points = max( points, scanfits[ jes ].xvs.size() );

DbgLv(1) << "ctPar: max points" << points;
   mmat .fill( NULL, points );      // Initialize matrix (array pointers)
   mvec .fill(  0.0, points * 2 );  // Initialize vector of matrix elements
   yrvec.fill(  0.0, points );      // Initialize vector of raw Y's
   covec.fill(  0.0, 2 );
   xyvec.fill( QPointF( 0.0, 0.0 ), points );

   for ( int jes = 0; jes < scanfits.size(); jes++ )
   {  // Build up chi-squared for fitted scans
      EqScanFit* scnf = &scanfits[ jes ];

      if ( ! scnf->scanFit )
         continue;             // Ignore any scan that is not fitted

      int    strtx  = scnf->start_ndx;   // Get scan parameters
      int    stopx  = scnf->stop_ndx;
      int    stopn  = stopx + 1;
      double fstemp = scnf->tempera;
      double fsdens = scnf->density;
      double fsvisc = scnf->viscosity;
      double fsrpm  = scnf->rpm;
      double fsvbar = runfit.vbar_vals[ 0 ];
//DbgLv(1) << "ctPar: jes strtx stopx" << jes << strtx << stopx;

      US_Math2::SolutionData solution;    // Calculate constant based on
      solution.vbar      = fsvbar;        //  buoyancy, based on vbar,density...
      solution.vbar20    = fsvbar;
      solution.density   = fsdens;
      solution.viscosity = fsvisc;
      US_Math2::data_correction( fstemp, solution );
      buoyancy_tb   = solution.buoyancyb;

      omega_s       = sq( fsrpm * M_PI / 30.0 );
      dconst        = ( buoyancy_tb * omega_s ) / ( 2.0 * R * ( K0 + fstemp ) );
      x_temp        = scnf->xvs[ strtx ];
      x_temp        = sq( x_temp );
//DbgLv(1) << "ctPar: xt x0 xN" << x_temp << scnf->xvs[strtx] << scnf->xvs[stopx];
      points        = 0;

      for ( int jxy = strtx; jxy < stopn; jxy++ )
      {  // Accumulate the X,Y values within the scan's start,stop range
         x_val      = scnf->xvs[ jxy ];
         y_val      = scnf->yvs[ jxy ];
         x_val      = dconst * ( sq( x_val ) - x_temp );
         xyvec[ points++ ] = QPointF( x_val, y_val );
      }

      double** MM     = mmat .data();  // Pointer to matrix data
      double*  yraw   = yrvec.data();  // Pointer to raw-Y data
      double*  mvls   = mvec .data();  // Pointer to matrix values
      double*  coeffs = covec.data();  // Pointer to output coefficients
      int      kk   = 0;

      for ( int ii = 0; ii < points; ii++ )
      {  // Build matrix and vector needed for LS
         x_val        = xyvec[ ii ].x();   // Raw X value
         y_val        = xyvec[ ii ].y();   // Raw Y value

         // The following builds and stores MM[ii][0] and MM[ii][1]
         mmat[ ii ]   = mvls + kk;         // Store pointer to 2-point array
         mvec[ kk++ ] = 1.0;               // Update matrix column
         mvec[ kk++ ] = exp( mwval * x_val );

         yrvec[ ii ]  = y_val;             // Update Y-Raw vector
      }
//DbgLv(1) << "ctPar:   yrvec[0]" << yrvec[0] << "points" << points;
//DbgLv(1) << "ctPar:    mm00 mm01" << MM[0][0] << MM[0][1];
//int N=points-1;
//DbgLv(1) << "ctPar:    mmN0 mN01" << MM[N][0] << MM[N][1];
//DbgLv(1) << "ctPar:    mwval x_val" << mwval << x_val;

      // Get coefficients thru general Least Squares, order=2
      genLeastSquaresOrd2( MM, points, yraw, &coeffs );

      for ( int ii = 0; ii < points; ii++ )
      {  // Update the chi-squared value
         double chi = yrvec[ ii ] - ( coeffs[ 0 ] +
                      coeffs[ 1 ] * exp( mwval * xyvec[ ii ].x() ) );
         chi_sq    += sq( chi );
      }
//DbgLv(1) << "ctPar:    chi_sq" << chi_sq;

      scnf->baseline   = coeffs[ 0 ];
      scnf->baseln_rng = 0.05;

      if ( coeffs[ 1 ] < dflt_min  ||  isNan( coeffs[ 1 ] ) )
      {
DbgLv(1) << "ctPar:     ** jes" << jes << "coeffs1 " << covec[1];
         scnf->amp_vals[ 0 ] = dflt_min;
      }

      else
      {
         scnf->amp_vals[ 0 ] = log( coeffs[ 1 ] );
      }

      scnf->amp_rngs[ 0 ] = scnf->amp_vals[ 0 ] * 0.2;
   }
DbgLv(1) << "ctPar: coeffs 0 1 " << covec[0] << covec[1];
DbgLv(1) << "ctPar:chi_sq" << chi_sq;

   return chi_sq;
}

// Find the minimum residual.
// Residual values are f0, f1, f2; evaluated at x0, x1, x2.
// x0, x1, x2 are multipliers for incremental change of the parameter.
// Calculate bracket: assume the minimum is between x0=0 and some stepsize
// x2 away from x0. Then find an x1 in the middle between x0 and x2; and
// calculate f1(x1).
double US_EqMath::linesearch()
{
   double residm = 0.0;
   double old_f0 = 0.0;
   double old_f1 = 0.0;
   double old_f2 = 0.0;
   double x0     = 100.0;
   double x1     = 5000.0;
   double x2     = 10000.0;
   double hh     = 0.01;
   double toler  = 100.0;
   double errmx;
   double xmin;
   double fmin;
   int    iter   = 1;

   double f0     = calc_testParameter( x0 );
   double f1     = calc_testParameter( x1 );
   double f2     = calc_testParameter( x2 );
DbgLv(1) << "MinRes:iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
DbgLv(1) << "MinRes: dflt_min dflt_max" << dflt_min << dflt_max;

   if ( dataList[ 0 ].dataType == "RA" )
      errmx      = 1.0e4;      // absorbance
   else
      errmx      = 1.0e12;     // larger value for interference


   while ( f0 >= errmx  ||  f0 < 0.0  ||
           f1 >= errmx  ||  f1 < 0.0  ||
           f2 >= errmx  ||  f2 < 0.0 )
   {  // Assure f1,f2 are between 0.0 and errmx
      x1  *= 0.5;
      x2  *= 0.5;

      f1   = calc_testParameter( x1 );
      f2   = calc_testParameter( x2 );

      if ( x1 < dflt_min )
         return ( -1.0 );
   }
DbgLv(1) << "  x1 x2" << x1 << x2 << " f1 f2" << f1 << f2 << "errmx" << errmx;

   bool check_flag = true;

   while ( check_flag )
   {
      if ( ( isNan( f0 ) && isNan( f1 ) )  ||
           ( isNan( f1 ) && isNan( f2 ) )  ||
           ( isNan( f0 ) && isNan( f1 ) ) )
         return ( -1.0 );  // At least two values screwed up

      if ( ( qAbs( f2 - old_f2 ) < dflt_min )  &&
           ( qAbs( f1 - old_f1 ) < dflt_min )  &&
           ( qAbs( f0 - old_f0 ) < dflt_min ) )
         return ( 0.0 );  // Solution converged horizontally

      old_f0 = f0;
      old_f1 = f1;
      old_f2 = f2;
DbgLv(1) << "   old f0 f1 f2" << f0 << f1 << f2;

DbgLv(1) << "     test-0a";
bool t1 = ( qAbs( f2 - f0 ) < dflt_min );
bool t2 = ( qAbs( f1 - f0 ) < dflt_min );
bool t3 = ( f0 > f1 );
bool t4 = ( qAbs( f2 - f1 ) < dflt_min );
bool t12 = t1 && t2;
bool t34 = t3 && t4;
DbgLv(1) << "     t-0a t1-4" << t1 << t2 << t3 << t4 << "t12,34" << t12 << t34;
      if ( ( ( qAbs( f2 - f0 ) < dflt_min ) &&
             ( qAbs( f1 - f0 ) < dflt_min ) )  ||
           ( ( f0 > f1 )  &&
             ( qAbs( f2 - f1 ) < dflt_min ) ) )
         return ( 0.0 );

DbgLv(1) << "     test-0b";
      if ( ( qAbs( x0 ) < dflt_min )  &&
           ( qAbs( x1 ) < dflt_min )  &&
           ( qAbs( x2 ) < dflt_min ) )
         return ( 0.0 );

DbgLv(1) << "     test-0c";
      if ( ( ( qAbs( f0 - f1 ) < dflt_min ) &&
             ( qAbs( f1 - f2 ) < dflt_min ) )  ||
           ( ( f2 > f1 )  &&
             ( qAbs( f0 - f1 ) < dflt_min ) ) )
         return ( 0.0 );

DbgLv(1) << "     test-0 x0 x1 x2" << x0 << x1 << x2;
      if ( ( f0 > f1 )  &&  ( f2 > f1 ) )    // We have a bracket
      {
DbgLv(1) << "     update-0 f0 f1 f2" << f0 << f1 << f2;
         check_flag = false;
         break;
      }

      if ( ( f2 > f1  &&  f1 > f0 )  ||
           ( f1 > f0  &&  f1 > f2 )  ||
           ( f1 == f2 &&  f1 > f0 ) )
      {
         x2     = x1;
         f2     = f1;
         x1     = ( x2 + x1 ) * 0.5;
         f1     = calc_testParameter( x1 );
DbgLv(1) << "     update-1 f0 f1 f2" << f0 << f1 << f2;
      }

      else if ( ( f0 > f1 )  &&  ( f1 > f2 ) )
      {
         x0     = x1;
         f0     = f1;
         x1     = x2;
         f1     = f2;
         x2     = x2 + ( ( pow( 2.0, (double)( iter + 2 ) ) ) + hh );
         f2     = calc_testParameter( x2 );
DbgLv(1) << "     update-2 f0 f1 f2" << f0 << f1 << f2;
      }

      iter++;
DbgLv(1) << "MinRes:         iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
   }

DbgLv(1) << "MinRes:  iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
   x1     = ( x0 + x2 ) * 0.5;
   hh     = x1 - x0;
   f1     = calc_testParameter( x1 );

   while ( true )
   {
      if ( f0 < f1 )   // Shift left
      {
         x2     = x1;
         f2     = f1;
         x1     = x0;
         f1     = f0;
         x0     = x1 - hh;
         f0     = calc_testParameter( x0 );
      }

      if ( f2 < f1 )   // Shift right
      {
         x0     = x1;
         f0     = f1;
         x1     = x2;
         f1     = f2;
         x2     = x1 + hh;
         f2     = calc_testParameter( x2 );
      }

      if ( qAbs( f0 - ( f1 * 2.0 ) + f2 ) < dflt_min )
      {
         residm = 0.0;
         break;
      }

      xmin   = x1 + ( hh * ( f0 - f2 ) ) / ( 2.0 * ( f0 - f1 * 2.0 + f2 ) );
      fmin   = calc_testParameter( xmin );

      if ( fmin < f1 )
      {
         x1     = xmin;
         f1     = fmin;
      }

      hh     = hh * 0.5;

      if ( hh < toler )
      {
         //residm = x1;  // Shouldn't it be "f" we return?
         residm = f1;
         break;
      }

      x0     = x1 - hh;
      x2     = x1 + hh;
      f0     = calc_testParameter( x0 );
      f2     = calc_testParameter( x2 );
DbgLv(1) << "     update-9 x0 x2 f0 f2" << x0 << x2 << f0 << f2;
   }

   return residm;
}

// Solve a general least squares (order=2)
void US_EqMath::genLeastSquaresOrd2( double** MM, int points,
                                     double* y_raw, double** coeff )
{
   double* aa[ 2 ];            // 2 x 2 matrix
   double  a0[ 2 ];            // matrix column 1
   double  a1[ 2 ];            // matrix column 2
   double  bb[ 2 ];            // 2-point vector
   aa[ 0 ] = (double*)a0;      // finish matrix creation
   aa[ 1 ] = (double*)a1;
   a0[ 0 ] = 0.0;              // initialize work matrix
   a0[ 1 ] = 0.0;
   a1[ 0 ] = 0.0;
   a1[ 1 ] = 0.0;

   for ( int ii = 0; ii < 2; ii++ )
   {  // Fill in the lower triangle of the 2 x 2 "A" matrix
      for ( int jj = 0; jj <= ii; jj++ )
      {
         double dotp = 0.0;

         for ( int kk = 0; kk < points; kk++ )
            dotp += ( MM[ kk ][ ii ] * MM[ kk ][ jj ] );

         aa[ ii ][ jj ] = dotp;
      }
   }

   for ( int jj = 0; jj < 2; jj++ )
   {  // Fill in the 2 values of the "B" vector
      double dotp = 0.0;

      for ( int kk = 0; kk < points; kk++ )
         dotp += ( y_raw[ kk ] * MM[ kk ][ jj ] );

      bb[ jj ] = dotp;
   }

   // Do 2nd-order Cholesky decomposition and solve system
   Cholesky_DecompOrd2( (double**)aa );
   Cholesky_SolveSysOrd2( (double**)aa, bb );

   // Result is the coefficients vector
   (*coeff)[ 0 ] = bb[ 0 ];
   (*coeff)[ 1 ] = bb[ 1 ];
//DbgLv(1) << "LS2: CSS b0 b1" << bb[0] << bb[1];
}

// Cholesky Decomposition for order=2
bool US_EqMath::Cholesky_DecompOrd2( double** aa )
{
DbgLv(1) << "LS2:CDec a00 a10 a11" << aa[0][0] << aa[1][0] << aa[1][1];
   aa[ 0 ][ 0 ] = sqrt( aa[ 0 ][ 0 ] );
   aa[ 1 ][ 0 ] = aa[ 1 ][ 0 ] / aa[ 0 ][ 0 ];

   double sum   = sq( aa[ 1 ][ 0 ] );
   double diff  = aa[ 1 ][ 1 ] - sum;
DbgLv(1) << "LS2:CDec  sum diff" << sum << diff;

   if ( diff <= 0.0 )   return false;

   aa[ 1 ][ 1 ] = sqrt( diff );
   aa[ 0 ][ 1 ] = 0.0;
DbgLv(1) << "LS2:CDec   a00 a10 a11" << aa[0][0] << aa[1][0] << aa[1][1];

   return true;
}

// Cholesky Solve System for order=2
bool US_EqMath::Cholesky_SolveSysOrd2( double** LL, double* bb )
{
   // Forward substitution

   bb[ 0 ] = bb[ 0 ] / LL[ 0 ][ 0 ];
   bb[ 1 ] = bb[ 1 ] - LL[ 1 ][ 0 ] * bb[ 0 ];
   bb[ 1 ] = bb[ 1 ] / LL[ 1 ][ 1 ];

   // Backward substitution

   bb[ 1 ] = bb[ 1 ] / LL[ 1 ][ 1 ];             // AGAIN????
   bb[ 0 ] = bb[ 0 ] - LL[ 1 ][ 0 ] * bb[ 1 ];
   bb[ 0 ] = bb[ 0 ] / LL[ 0 ][ 0 ];

   return true;
}

// Convenience function:  is a double value NAN (Not A valid Number)?
bool US_EqMath::isNan( double value )
{
   if ( value != value )           // NAN, the one case where value!=value
      return true;

   double avalue = qAbs( value );  // Also mark NAN if beyond float min,max

   if ( avalue < dflt_min  ||  avalue > dflt_max )
      return true;

   return false;
}

