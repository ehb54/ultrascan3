//! \file test_us_sim_residuals.cpp
// US_SimResiduals against the copies it replaced in FEMatch, autoflow analysis
// and the GMP reporter, kept here word for word as they were at 94e74d4f3.
#include "qt_test_base.h"
#include "us_dataIO.h"
#include "us_math2.h"
#include "us_noise.h"
#include "us_sim_residuals.h"
#include <QVector>
#include <cmath>
#include <cstring>
#include <random>

namespace
{
// The old code writes its results into line edits or a string.
struct TextSink
{
      QString text;
      void    setText( const QString& t )
      {
         text = t;
      }
};

// The members the old functions read and write, with the programs' names
struct FrozenState
{
      US_DataIO::EditedData*   edata = nullptr;
      US_DataIO::RawData*      sdata = nullptr;
      US_Noise                 ti_noise;
      US_Noise                 ri_noise;
      QList<int>               excludedScans;
      int                      scanCount = 0;
      QVector<QVector<double>> resids;
      int                      dbg_level = 0;
};

struct FrozenFeMatch : FrozenState
{
      TextSink  variance_box, rmsd_box;
      TextSink* le_variance = &variance_box;
      TextSink* le_rmsd     = &rmsd_box;
      void      calc_residuals();
      double    interp_sval( double, double*, double*, int );
};

struct FrozenAutoflow : FrozenState
{
      void   calc_residuals();
      double interp_sval( double, double*, double*, int );
};

struct FrozenReporter : FrozenState
{
      QString rmsd_global;
      void    calc_residuals();
      double  interp_sval( double, double*, double*, int );
};

// The old code logs through qDebug and DbgLv; keep the test output quiet.
#undef qDebug
#define qDebug QT_NO_QDEBUG_MACRO
#ifndef DbgLv
#define DbgLv( a )                                                                                                                 \
   if ( dbg_level >= a )                                                                                                           \
   qDebug()
#endif

// programs/us_fematch/us_fematch.cpp US_FeMatch::calc_residuals at 94e74d4f3, word for word but for the class name
void FrozenFeMatch::calc_residuals()
{
   int             dsize = edata->pointCount();
   int             ssize = sdata->pointCount();
   QVector<double> vecxx( dsize );
   QVector<double> vecsx( ssize );
   QVector<double> vecsy( ssize );
   double*         xx = vecxx.data();
   double*         sx = vecsx.data();
   double*         sy = vecsy.data();
   double          yval;
   double          sval;
   double          rmsd   = 0.0;
   double          tnoi   = 0.0;
   double          rnoi   = 0.0;
   bool            ftin   = ti_noise.count > 0;
   bool            frin   = ri_noise.count > 0;
   bool            matchd = ( dsize == ssize );
   int             kpts   = 0;
   DbgLv( 1 ) << "CALC_RESID: matchd" << matchd << "dsize ssize" << dsize << ssize;
   int             kexcls = 0;

   QVector<double> resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[jj] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[jj] = sdata->radius( jj );
      if ( sx[jj] != xx[jj] )
      {
         matchd = false;
      }
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );
      if ( !usescan )
      {
         kexcls++;
      }

      rnoi = frin ? ri_noise.values[ii] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[jj] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi = ftin ? ti_noise.values[jj] : 0.0;

         if ( matchd )
         {
            sval = sy[jj];
         }
         else
         {
            sval = interp_sval( xx[jj], sx, sy, ssize );
         }

         yval = edata->value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd += sq( yval );
            kpts++;
         }

         resscan[jj] = yval;
      }

      resids << resscan;
   }

   rmsd /= (double) ( kpts );
   le_variance->setText( QString::number( rmsd ) );
   rmsd = sqrt( rmsd );
   le_rmsd->setText( QString::number( rmsd ) );
   DbgLv( 1 ) << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;
}

// programs/us_fematch/us_fematch.cpp US_FeMatch::interp_sval at 94e74d4f3, word for word but for the class name
double FrozenFeMatch::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[jj] )
      {
         return sy[jj];
      }

      if ( xv < sx[jj] )
      { // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[jj] - sx[ii];
         double dy = sy[jj] - sy[ii];
         return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[jj] - sx[ii];
   double dy = sy[jj] - sy[ii];
   return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
}

// programs/us_autoflow_analysis/us_autoflow_analysis.cpp US_Analysis_auto::calc_residuals at 94e74d4f3, word for word but for the class name
void FrozenAutoflow::calc_residuals()
{
   int             dsize = edata->pointCount();
   int             ssize = sdata->pointCount();
   QVector<double> vecxx( dsize );
   QVector<double> vecsx( ssize );
   QVector<double> vecsy( ssize );
   double*         xx = vecxx.data();
   double*         sx = vecsx.data();
   double*         sy = vecsy.data();
   double          yval;
   double          sval;
   double          rmsd   = 0.0;
   double          tnoi   = 0.0;
   double          rnoi   = 0.0;
   bool            ftin   = ti_noise.count > 0;
   bool            frin   = ri_noise.count > 0;
   bool            matchd = ( dsize == ssize );
   int             kpts   = 0;
   qDebug() << "CALC_RESID: matchd" << matchd << "dsize ssize" << dsize << ssize;
   int             kexcls = 0;

   QVector<double> resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[jj] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[jj] = sdata->radius( jj );
      if ( sx[jj] != xx[jj] )
      {
         matchd = false;
      }
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );
      if ( !usescan )
      {
         kexcls++;
      }

      rnoi = frin ? ri_noise.values[ii] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[jj] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi = ftin ? ti_noise.values[jj] : 0.0;

         if ( matchd )
         {
            sval = sy[jj];
         }
         else
         {
            sval = interp_sval( xx[jj], sx, sy, ssize );
         }

         yval = edata->value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd += sq( yval );
            kpts++;
         }

         resscan[jj] = yval;
      }

      resids << resscan;
   }

   rmsd /= (double) ( kpts );
   //le_variance->setText( QString::number( rmsd ) );
   rmsd = sqrt( rmsd );
   //le_rmsd    ->setText( QString::number( rmsd ) );
   qDebug() << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;
}

// programs/us_autoflow_analysis/us_autoflow_analysis.cpp US_Analysis_auto::interp_sval at 94e74d4f3, word for word but for the class name
double FrozenAutoflow::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[jj] )
      {
         return sy[jj];
      }

      if ( xv < sx[jj] )
      { // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[jj] - sx[ii];
         double dy = sy[jj] - sy[ii];
         return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[jj] - sx[ii];
   double dy = sy[jj] - sy[ii];
   return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
}

// programs/us_reporter_gmp/us_reporter_gmp.cpp US_ReporterGMP::calc_residuals at 94e74d4f3, word for word but for the class name
void FrozenReporter::calc_residuals()
{
   int             dsize = edata->pointCount();
   int             ssize = sdata->pointCount();
   QVector<double> vecxx( dsize );
   QVector<double> vecsx( ssize );
   QVector<double> vecsy( ssize );
   double*         xx = vecxx.data();
   double*         sx = vecsx.data();
   double*         sy = vecsy.data();
   double          yval;
   double          sval;
   double          rmsd   = 0.0;
   double          tnoi   = 0.0;
   double          rnoi   = 0.0;
   bool            ftin   = ti_noise.count > 0;
   bool            frin   = ri_noise.count > 0;
   bool            matchd = ( dsize == ssize );
   int             kpts   = 0;
   qDebug() << "CALC_RESID: matchd" << matchd << "dsize ssize" << dsize << ssize;
   int             kexcls = 0;

   QVector<double> resscan;

   resids.clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[jj] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[jj] = sdata->radius( jj );
      if ( sx[jj] != xx[jj] )
      {
         matchd = false;
      }
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );
      if ( !usescan )
      {
         kexcls++;
      }

      rnoi = frin ? ri_noise.values[ii] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[jj] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi = ftin ? ti_noise.values[jj] : 0.0;

         if ( matchd )
         {
            sval = sy[jj];
         }
         else
         {
            sval = interp_sval( xx[jj], sx, sy, ssize );
         }

         yval = edata->value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd += sq( yval );
            kpts++;
         }

         resscan[jj] = yval;
      }

      resids << resscan;
   }

   rmsd /= (double) ( kpts );
   //le_variance->setText( QString::number( rmsd ) );
   rmsd = sqrt( rmsd );

   rmsd_global = QString::number( rmsd );

   qDebug() << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;
}

// programs/us_reporter_gmp/us_reporter_gmp.cpp US_ReporterGMP::interp_sval at 94e74d4f3, word for word but for the class name
double FrozenReporter::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[jj] )
      {
         return sy[jj];
      }

      if ( xv < sx[jj] )
      { // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[jj] - sx[ii];
         double dy = sy[jj] - sy[ii];
         return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[jj] - sx[ii];
   double dy = sy[jj] - sy[ii];
   return ( sy[ii] + ( xv - sx[ii] ) * dy / dx );
}

// Data with the given radii and readings, one row per scan
template <class Data> Data makeData( const QVector<double>& radii, const QVector<QVector<double>>& scans )
{
   Data d;
   d.xvalues = radii;
   for ( const QVector<double>& readings : scans )
   {
      US_DataIO::Scan s;
      s.rvalues = readings;
      d.scanData << s;
   }
   return d;
}

US_Noise noiseOf( const QVector<double>& values )
{
   US_Noise n;
   n.values = values;
   n.count  = values.size();
   return n;
}

bool sameBits( double a, double b )
{
   return std::memcmp( &a, &b, sizeof a ) == 0;
}

bool sameBits( const QVector<QVector<double>>& a, const QVector<QVector<double>>& b )
{
   if ( a.size() != b.size() )
   {
      return false;
   }
   for ( int i = 0; i < a.size(); i++ )
   {
      if ( a[i].size() != b[i].size() )
      {
         return false;
      }
      for ( int j = 0; j < a[i].size(); j++ )
      {
         if ( !sameBits( a[i][j], b[i][j] ) )
         {
            return false;
         }
      }
   }
   return true;
}

// One random case the old code handles: at least two simulation points and no
// more than the data has (the old code reads past its radius list otherwise).
struct Case
{
      US_DataIO::EditedData edited;
      US_DataIO::RawData    simulated;
      US_Noise              ti, ri;
      QList<int>            excluded;
      int                   scanCount = 0;
};

Case randomCase( std::mt19937& rng )
{
   std::uniform_int_distribution<int>     pts( 2, 40 ), scans( 1, 10 ), coin( 0, 1 ), pick( 0, 3 );
   std::uniform_real_distribution<double> val( -2.0, 2.0 ), step( 0.001, 0.02 ), shift( -0.01, 0.01 );
   Case                                   c;
   const int                              dsize = pts( rng );
   const int                              nscan = scans( rng );
   QVector<double>                        dr( dsize );
   double                                 r = 5.9 + step( rng );
   for ( double& x : dr )
   {
      x = r;
      r += step( rng );
   }

   int             ssize = dsize;
   QVector<double> sr    = dr;
   switch ( pick( rng ) )
   {
      case 1: // same count, other radii
         for ( double& x : sr )
         {
            x += shift( rng );
         }
         std::sort( sr.begin(), sr.end() );
         break;
      case 2: // fewer points, spread over the same range
         ssize = std::max( 2, dsize / 2 );
         sr.resize( ssize );
         for ( int i = 0; i < ssize; i++ )
         {
            sr[i] = dr.first() + ( dr.last() - dr.first() ) * i / ( ssize - 1 );
         }
         break;
      case 3: // a data radius on a simulation radius, others between
         for ( int i = 0; i < ssize; i += 2 )
         {
            sr[i] += shift( rng );
         }
         std::sort( sr.begin(), sr.end() );
         break;
      default:
         break;
   }

   QVector<QVector<double>> dv( nscan, QVector<double>( dsize ) ), sv( nscan, QVector<double>( ssize ) );
   for ( auto& row : dv )
   {
      for ( double& x : row )
      {
         x = val( rng );
      }
   }
   for ( auto& row : sv )
   {
      for ( double& x : row )
      {
         x = val( rng );
      }
   }
   c.edited    = makeData<US_DataIO::EditedData>( dr, dv );
   c.simulated = makeData<US_DataIO::RawData>( sr, sv );
   if ( coin( rng ) )
   {
      QVector<double> t( dsize );
      for ( double& x : t )
      {
         x = val( rng ) / 10;
      }
      c.ti = noiseOf( t );
   }
   if ( coin( rng ) )
   {
      QVector<double> t( nscan );
      for ( double& x : t )
      {
         x = val( rng ) / 10;
      }
      c.ri = noiseOf( t );
   }
   for ( int i = 0; i < nscan; i++ )
   {
      if ( pick( rng ) == 0 )
      {
         c.excluded << i;
      }
   }
   if ( pick( rng ) == 0 )
   {
      for ( int i = 0; i < nscan; i++ )
      {
         if ( !c.excluded.contains( i ) )
         {
            c.excluded << i;
         }
      }
   }
   c.scanCount = nscan;
   return c;
}

template <class Frozen> void load( Frozen& f, Case& c )
{
   f.edata         = &c.edited;
   f.sdata         = &c.simulated;
   f.ti_noise      = c.ti;
   f.ri_noise      = c.ri;
   f.excludedScans = c.excluded;
   f.scanCount     = c.scanCount;
   f.resids << QVector<double>{ 99.0 }; // replaced, not appended to
}
} // namespace

class TestUSSimResiduals : public QtTestBase
{
};

// Every result the three programs showed is unchanged: the residuals bit for
// bit, FEMatch's variance and RMSD text, and the reporter's RMSD text.
TEST_F( TestUSSimResiduals, MatchesTheThreeOldCopies )
{
   std::mt19937 rng( 1243 );
   int          compared = 0, allExcluded = 0, interpolated = 0;
   for ( int n = 0; n < 3000; n++ )
   {
      Case c = randomCase( rng );
      SCOPED_TRACE( n );

      QVector<QVector<double>> resids{ { 99.0 } };
      const double   variance = US_SimResiduals::calculate( c.edited, c.simulated, c.ti, c.ri, c.excluded, c.scanCount, resids );

      FrozenFeMatch  fe;
      FrozenAutoflow au;
      FrozenReporter re;
      load( fe, c );
      load( au, c );
      load( re, c );
      fe.calc_residuals();
      au.calc_residuals();
      re.calc_residuals();

      ASSERT_TRUE( sameBits( resids, fe.resids ) );
      ASSERT_TRUE( sameBits( resids, au.resids ) );
      ASSERT_TRUE( sameBits( resids, re.resids ) );
      EXPECT_EQ( QString::number( variance ), fe.le_variance->text );
      EXPECT_EQ( QString::number( sqrt( variance ) ), fe.le_rmsd->text );
      EXPECT_EQ( QString::number( sqrt( variance ) ), re.rmsd_global );
      compared++;
      allExcluded += c.excluded.size() == c.scanCount;
      interpolated += c.simulated.xvalues != c.edited.xvalues;
   }
   EXPECT_EQ( compared, 3000 );
   EXPECT_GT( allExcluded, 50 );    // NaN variance
   EXPECT_GT( interpolated, 1000 ); // radii differ
}

// Two scans of three points, worked by hand
TEST_F( TestUSSimResiduals, WorkedExample )
{
   auto                     edited = makeData<US_DataIO::EditedData>( { 6.0, 6.1, 6.2 }, { { 1.0, 2.0, 3.0 }, { 2.0, 2.0, 2.0 } } );
   auto                     simulated = makeData<US_DataIO::RawData>( { 6.0, 6.1, 6.2 }, { { 0.5, 1.5, 2.0 }, { 1.0, 1.0, 1.0 } } );
   QVector<QVector<double>> resids;

   // TI noise per point and RI noise per scan are subtracted; scan 1 is left out of the variance
   const double             variance =
      US_SimResiduals::calculate( edited, simulated, noiseOf( { 0.25, 0.0, 0.5 } ), noiseOf( { 0.0, 0.5 } ), { 1 }, 2, resids );
   ASSERT_EQ( resids.size(), 2 );
   EXPECT_EQ( resids[0], QVector<double>( { 0.25, 0.5, 0.5 } ) );
   EXPECT_EQ( resids[1], QVector<double>( { 0.25, 0.5, 0.0 } ) );
   EXPECT_DOUBLE_EQ( variance, ( 0.0625 + 0.25 + 0.25 ) / 3.0 );

   // No noise, nothing excluded: all six values count
   EXPECT_DOUBLE_EQ( US_SimResiduals::calculate( edited, simulated, US_Noise(), US_Noise(), {}, 2, resids ),
                     ( 0.25 + 0.25 + 1.0 + 1.0 + 1.0 + 1.0 ) / 6.0 );

   // Every scan excluded: no points, so 0/0
   EXPECT_TRUE( std::isnan( US_SimResiduals::calculate( edited, simulated, US_Noise(), US_Noise(), { 0, 1 }, 2, resids ) ) );
}

// Other simulation radii are interpolated linearly, and extended past the last point
TEST_F( TestUSSimResiduals, InterpolatesTheSimulation )
{
   const double sx[] = { 6.0, 6.2, 6.4 };
   const double sy[] = { 1.0, 3.0, 4.0 };
   EXPECT_NEAR( US_SimResiduals::interpolate( 6.1, sx, sy, 3 ), 2.0, 1e-12 );
   EXPECT_DOUBLE_EQ( US_SimResiduals::interpolate( 6.2, sx, sy, 3 ), 3.0 );
   EXPECT_NEAR( US_SimResiduals::interpolate( 6.6, sx, sy, 3 ), 5.0, 1e-12 );
   EXPECT_NEAR( US_SimResiduals::interpolate( 5.9, sx, sy, 3 ), 0.0, 1e-12 ); // before the first point: the first two

   // More simulation points than data points: interpolated, without reading past the data's radii
   auto                     edited    = makeData<US_DataIO::EditedData>( { 6.1, 6.3 }, { { 2.0, 4.0 } } );
   auto                     simulated = makeData<US_DataIO::RawData>( { 6.0, 6.2, 6.4 }, { { 1.0, 3.0, 4.0 } } );
   QVector<QVector<double>> resids;
   EXPECT_NEAR( US_SimResiduals::calculate( edited, simulated, US_Noise(), US_Noise(), {}, 1, resids ), 0.125, 1e-12 );
   ASSERT_EQ( resids.size(), 1 );
   ASSERT_EQ( resids[0].size(), 2 );
   EXPECT_NEAR( resids[0][0], 0.0, 1e-12 );
   EXPECT_NEAR( resids[0][1], 0.5, 1e-12 );
}
