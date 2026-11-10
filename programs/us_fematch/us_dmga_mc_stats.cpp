//! \file us_dmga_mc_stats.cpp

#include "us_dmga_mc_stats.h"
#include "../../calc/us_calc_statistics.h"

#include <limits>

// Class of static functions for DMGA-MC statistics

// Build a vector of iteration models from a DMGA-MC model
int US_DmgaMcStats::build_imodels( US_Model& model,
      QVector< US_Model >& imodels )
{
   imodels.clear();
   QStringList xmls;
   int niters      = 0;
   int ncomp       = 0;
   int nasso       = 0;
   int kcomp       = 0;
   int kasso       = 0;
   int nxmls       = model.mc_iter_xmls( xmls );

   for ( int ii = 0; ii < nxmls; ii++ )
   {  // Build the individual iteration models
      QString mxml   = xmls[ ii ];
      int jj         = mxml.indexOf( "description=" );
      QString mdesc  = QString( mxml ).mid( jj, 100 )
                       .section( '"', 1, 1 );

      US_Model imodel;
      imodel.load_string( mxml );
      kcomp          = imodel.components.size();
      kasso          = imodel.associations.size();;
qDebug() << "DmS:AdjMd:  ii" << ii << "xml.desc" << mdesc;
qDebug() << "DmS:AdjMd:     ncomp" << kcomp << "nassoc" << kasso;

      if ( ii == 0 )
      {
         ncomp          = kcomp;
         nasso          = kasso;
      }

      if ( kcomp == ncomp  ||  kasso == nasso )
      {
         niters++;
         imodels << imodel;
      }
      else
qDebug() << "DmS:AdjMd: ***ii=" << ii << "kcomp" << kcomp << "kassoc" << kasso;

   }

   return niters;
}

// Build RMSD statistics from iteration models
void US_DmgaMcStats::build_rmsd_stats( int niters,
      QVector< US_Model >& imodels, QVector< double >& rstats )
{
   const int stsiz    = 23;
   QVector< double > rmsds;
   QVector< double > rconcs;

   rstats.fill( 0.0, stsiz );

   // Get statistics for RMSDs
   for ( int jj = 0; jj < niters; jj++ )
   {
      double rmsd      = sqrt( imodels[ jj ].variance );
      rmsds  << rmsd;
      rconcs << 1.0;
   }

   compute_statistics( niters, rmsds, rconcs, rstats );
qDebug() << "Dst:BRs:   iters" << niters << "RMSD min max mean median"
   << rstats[0] << rstats[1] << rstats[2] << rstats[3];
}

// Build Model attribute statistics from iteration models
int US_DmgaMcStats::build_model_stats( int niters,
      QVector< US_Model >& imodels, QVector< QVector< double > >& astats )
{
   QVector< double > concs;
   QVector< double > vbars;
   QVector< double > mwts;
   QVector< double > scos;
   QVector< double > dcos;
   QVector< double > ff0s;
   QVector< double > kds;
   QVector< double > koffs;
   const int ncatt  = 6;
   const int naatt  = 2;
   int ncomp        = imodels[ 0 ].components.size();
   int nasso        = imodels[ 0 ].associations.size();
   int ntatts       = ncomp * ncatt + nasso * naatt;
   astats.resize( ntatts );

   int kt           = 0;
qDebug() << "DmS:BMs:  ncomp" << ncomp << "nasso" << nasso
 << "ntatts" << ntatts << "niters" << niters;

   // Get statistics for each component
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      concs.fill( 0.0, niters );
      vbars.fill( 0.0, niters );
      mwts .fill( 0.0, niters );
      scos .fill( 0.0, niters );
      dcos .fill( 0.0, niters );
      ff0s .fill( 0.0, niters );
      int i0         = qMax( 0, ii - 1 );

      for ( int jj = 0; jj < niters; jj++ )
      {
         US_Model::SimulationComponent* sc = &imodels[ jj ].components[ ii ];
         US_Model::SimulationComponent* s0 = &imodels[ jj ].components[ i0 ];
         concs[ jj ]    = sc->signal_concentration > 0.0 ?
                          sc->signal_concentration :
                          s0->signal_concentration;
         vbars[ jj ]    = sc->vbar20;
         mwts [ jj ]    = sc->mw;
         scos [ jj ]    = sc->s;
         dcos [ jj ]    = sc->D;
         ff0s [ jj ]    = sc->f_f0;
      }

      compute_statistics( niters, concs, concs, astats[ kt++ ] );
      compute_statistics( niters, vbars, concs, astats[ kt++ ] );
      compute_statistics( niters, mwts , concs, astats[ kt++ ] );
      compute_statistics( niters, scos , concs, astats[ kt++ ] );
      compute_statistics( niters, dcos , concs, astats[ kt++ ] );
      compute_statistics( niters, ff0s , concs, astats[ kt++ ] );
qDebug() << "DmS:BMs:   ii" << ii << "mean c v w s d k"
 << astats[kt-6][2] << astats[kt-5][2] << astats[kt-4][2]
 << astats[kt-3][2] << astats[kt-2][2] << astats[kt-1][2];
   }

   // Get statistics for reactions
   for ( int ii = 0; ii < nasso; ii++ )
   {
      US_Model::Association* as = &imodels[ 0 ].associations[ ii ];
      int nrcs        = as->rcomps.size();
      int rc1         = as->rcomps[ 0 ];
      int rc2         = as->rcomps[ 1 ];
      //int stoi1       = as->stoichs[ 0 ];
      //int stoi2       = as->stoichs[ 1 ];
      //int stoi3       = ( nrcs == 2 ) ? stoi2 : as->stoichs[ 2 ];
      concs.fill( 0.0, niters );
      kds  .fill( 0.0, niters );
      koffs.fill( 0.0, niters );

      if ( nrcs == 2 )
      {  // Single reactant and a product
         for ( int jj = 0; jj < niters; jj++ )
         {
            US_Model::Association* as = &imodels[ jj ].associations[ ii ];
            US_Model::SimulationComponent* sc = &imodels[ jj ].components[rc1];
            concs[ jj ]    = sc->signal_concentration;
            kds  [ jj ]    = as->k_d;
            koffs[ jj ]    = as->k_off;
         }

         compute_statistics( niters, kds,   concs, astats[ kt++ ] );
         compute_statistics( niters, koffs, concs, astats[ kt++ ] );
      }

      else
      {  // Two reactants and a product (sum concentrations)
         for ( int jj = 0; jj < niters; jj++ )
         {
            US_Model::Association* as = &imodels[ jj ].associations[ ii ];
            US_Model::SimulationComponent* sc = &imodels[ jj ].components[rc1];
            US_Model::SimulationComponent* c2 = &imodels[ jj ].components[rc2];
            concs[ jj ]    = sc->signal_concentration
                           + c2->signal_concentration;
            kds  [ jj ]    = as->k_d;
            koffs[ jj ]    = as->k_off;
         }

         compute_statistics( niters, kds,   concs, astats[ kt++ ] );
         compute_statistics( niters, koffs, concs, astats[ kt++ ] );
qDebug() << "DmS:BMs:   ii" << ii << "mean kd ko"
 << astats[kt-2][2] << astats[kt-1][2];
      }
   }

qDebug() << "DmS:BMs: RETURN w ntatts" << ntatts;
   return ntatts;
}

// Compute the statistical values for a vector of values
bool US_DmgaMcStats::compute_statistics( const int nvals,
      QVector< double >& vals, QVector< double >& concs,
      QVector< double >& stats )
{
   // The statistics come from us_calc; undefined ones are nan.
   const US_Calc::WeightedStatistics st = US_Calc::weighted_statistics(
      vals.mid( 0, nvals ), concs.mid( 0, nvals ), US_Calc::StatisticsConvention::MonteCarloIterations );
   const double nan      = std::numeric_limits<double>::quiet_NaN();
   const double mode_cen = st.mode_center.value_or( nan );
   const double conf95lo = st.confidence95_low.value_or( nan );
   const double conf95hi = st.confidence95_high.value_or( nan );
   const double conf99lo = st.confidence99_low.value_or( nan );
   const double conf99hi = st.confidence99_high.value_or( nan );

   stats.resize( 23 );
   stats[0]         = st.minimum;                            // Minimum
   stats[1]         = st.maximum;                            // Maximum
   stats[2]         = st.mean.value_or( nan );               // Mean
   stats[3]         = st.median.value_or( nan );             // Median
   stats[4]         = st.skew.value_or( nan );               // Skew
   stats[5]         = st.kurtosis.value_or( nan );           // Kurtosis
   stats[6]         = st.mode_low.value_or( nan );           // Lower Mode
   stats[7]         = st.mode_high.value_or( nan );          // Upper Mode
   stats[8]         = mode_cen;                              // Mode Center
   stats[9]         = conf95lo;                              // 95% Confidence Interval Low
   stats[10]        = conf95hi;                              // 95% Confidence Interval High
   stats[11]        = conf99lo;                              // 99% Confidence Interval Low
   stats[12]        = conf99hi;                              // 99% Confidence Interval High
   stats[13]        = st.standard_deviation.value_or( nan ); // Standard Deviation
   stats[14]        = st.standard_error.value_or( nan );     // Standard Error
   stats[15]        = st.variance.value_or( nan );           // Variance
   stats[16]        = st.correlation.value_or( nan );        // Correlation Coefficient
   stats[17]        = st.bins;                               // Number of Bins
   stats[18]        = st.area.value_or( nan );               // Distribution Area
   // Confidence limits relative to the mode center; equal to the mean
   // when the values are constant.
   const bool fixed = st.constant;
   stats[19]        = fixed ? stats[2] : conf95hi - mode_cen; // 95% Confidence Limit Low
   stats[20]        = fixed ? stats[2] : mode_cen - conf95lo; // 95% Confidence Limit High
   stats[21]        = fixed ? stats[2] : conf99hi - mode_cen; // 99% Confidence Limit Low
   stats[22]        = fixed ? stats[2] : mode_cen - conf99lo; // 99% Confidence Limit High

   return st.constant;
}

// Build Model attribute statistics from iteration models
int US_DmgaMcStats::build_used_model( const QString smtype, const int iter,
      QVector< US_Model >& imodels, US_Model& umodel )
{

   int stx        = -iter;                             // Iteration index
   stx            = ( smtype == "mean"   ) ? 2 : stx;  // Mean stats index
   stx            = ( smtype == "median" ) ? 3 : stx;  // Median stats index
   stx            = ( smtype == "mode"   ) ? 8 : stx;  // Mode stats index
qDebug() << "DmS:AdjMd:  stx" << stx;

   if ( stx < 0 )
   {  // Used model is one of the iterations
      umodel         = imodels[ iter - 1 ];
   }

   else
   {  // Used model uses mean|median|mode of each attribute
      umodel         = imodels[ 0 ];
      int niters     = imodels.size();
      QVector< QVector< double > >  mstats;

      // Build statistics across iterations
      US_DmgaMcStats::build_model_stats( niters, imodels, mstats );

      int ncomp      = umodel.components.size();
      int nasso      = umodel.associations.size();
      int ks         = 0;

      for ( int ii = 0; ii < ncomp; ii++ )
      {  // Pick up mean|median|mode of any floating attributes
         bool fixs      = ( mstats[ ks + 3 ][ 0 ] == mstats[ ks + 3 ][ 1 ] );
         bool fixd      = ( mstats[ ks + 4 ][ 0 ] == mstats[ ks + 4 ][ 1 ] );
         bool fixk      = ( mstats[ ks + 5 ][ 0 ] == mstats[ ks + 5 ][ 1 ] );
         double conc    = mstats[ ks++ ][ stx ];
         double vbar    = mstats[ ks++ ][ stx ];
         double mw      = mstats[ ks++ ][ stx ];
         double sedc    = mstats[ ks++ ][ stx ];
         double difc    = mstats[ ks++ ][ stx ];
         double ff0     = mstats[ ks++ ][ stx ];
         conc           = umodel.is_product( ii ) ? 0.0 : conc;

         umodel.components[ ii ].signal_concentration = conc;
         umodel.components[ ii ].vbar20  = vbar;
         umodel.components[ ii ].mw      = mw;
         umodel.components[ ii ].s       = 0.0;
         umodel.components[ ii ].D       = 0.0;
         umodel.components[ ii ].f_f0    = 0.0;
         umodel.components[ ii ].f       = 0.0;

         if ( fixk )
            umodel.components[ ii ].f_f0    = ff0;    // Set fixed f/f0
         else if ( fixs )
            umodel.components[ ii ].s       = sedc;   // Set fixed s
         else if ( fixd )
            umodel.components[ ii ].D       = difc;   // Set fixed D
         else
            umodel.components[ ii ].f_f0    = ff0;    // Set f/f0 if none fixed
//*DEBUG*
qDebug() << "DmS:UajMd: Cii=" << ii << "c v w s d k"
 << umodel.components[ii].signal_concentration
 << umodel.components[ii].vbar20
 << umodel.components[ii].mw
 << umodel.components[ii].s
 << umodel.components[ii].D
 << umodel.components[ii].f_f0 << "fixs,d,k" << fixs << fixd << fixk;
         umodel.calc_coefficients( umodel.components[ ii ] );
qDebug() << "DmS:AdjMd:  Cii=" << ii << "c v w s d k"
 << umodel.components[ii].signal_concentration
 << umodel.components[ii].vbar20
 << umodel.components[ii].mw
 << umodel.components[ii].s
 << umodel.components[ii].D
 << umodel.components[ii].f_f0;
umodel.components[ii].D   =0.0;
umodel.components[ii].f_f0=0.0;
umodel.components[ii].f   =0.0;
umodel.calc_coefficients(umodel.components[ii]);
qDebug() << "DmS:AdjMd:   fixS s d k" << umodel.components[ii].s
 << umodel.components[ii].D << umodel.components[ii].f_f0;
umodel.components[ii].s   =0.0;
umodel.components[ii].f_f0=0.0;
umodel.components[ii].f   =0.0;
umodel.calc_coefficients(umodel.components[ii]);
qDebug() << "DmS:AdjMd:   fixD s d k" << umodel.components[ii].s
 << umodel.components[ii].D << umodel.components[ii].f_f0;
umodel.components[ii].s   =0.0;
umodel.components[ii].D   =0.0;
umodel.components[ii].f   =0.0;
umodel.calc_coefficients(umodel.components[ii]);
qDebug() << "DmS:AdjMd:   fixK s d k" << umodel.components[ii].s
 << umodel.components[ii].D << umodel.components[ii].f_f0;
umodel.components[ii].mw  =0.0;
umodel.components[ii].D   =0.0;
umodel.components[ii].f   =0.0;
umodel.calc_coefficients(umodel.components[ii]);
qDebug() << "DmS:AdjMd:   fixD w d k" << umodel.components[ii].mw
 << umodel.components[ii].D << umodel.components[ii].f_f0;
umodel.components[ii].s   =0.0;
umodel.components[ii].mw  =0.0;
umodel.components[ii].f   =0.0;
umodel.calc_coefficients(umodel.components[ii]);
qDebug() << "DmS:AdjMd:   fixK s d w" << umodel.components[ii].s
 << umodel.components[ii].D << umodel.components[ii].mw;
//*DEBUG*
      }  // END: components loop

      for ( int ii = 0; ii < nasso; ii++ )
      {
         bool fltd      = ( mstats[ ks     ][ 0 ] != mstats[ ks     ][ 1 ] );
         bool flto      = ( mstats[ ks + 1 ][ 0 ] != mstats[ ks + 1 ][ 1 ] );
         double k_d     = fltd ? mstats[ ks++ ][ stx ] : mstats[ ks++ ][ 0 ];
         double k_off   = flto ? mstats[ ks++ ][ stx ] : mstats[ ks++ ][ 0 ];
         umodel.associations[ ii ].k_d     = k_d;
         umodel.associations[ ii ].k_off   = k_off;
qDebug() << "DmS:AdjMd: Aii=" << ii << "d off"
 << umodel.associations[ii].k_d << umodel.associations[ii].k_off;
      }  // END: associations loop
   }  // END:  smtype==mean|median|mode

   return stx;
}

