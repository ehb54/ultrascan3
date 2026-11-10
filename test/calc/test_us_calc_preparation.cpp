//! \file test_us_calc_preparation.cpp
// Coefficient preparation and smoothing of the calculation library.

#include <gtest/gtest.h>

#include <QVector>

#include "us_calc_coefficients.h"
#include "us_calc_constants.h"
#include "us_calc_smoothing.h"

using namespace US_Calc;

// The LIMS test-corpus model stability-v1-single was written by
// US_Model::update_coefficients from s, f/f0 and vbar; its D, mw and f are
// the reference (lims scripts/test-corpus/config/models).
TEST( UsCalcCoefficients, SAndFrictionalRatioGiveTheCorpusValues )
{
   Component c;
   c.s                    = 5e-13;
   c.f_f0                 = 1.5;
   c.vbar20               = 0.72;
   c.signal_concentration = 1.0;
   ASSERT_TRUE( calc_coefficients( c ) );
   EXPECT_EQ( c.D, 4.858600876870282e-07 );
   EXPECT_EQ( c.mw, 89173.17878739854 );
   EXPECT_EQ( c.f, 8.330334488961826e-08 );
   EXPECT_EQ( c.f_f0, 1.5 );
   EXPECT_EQ( c.signal_concentration, 1.0 );
}

TEST( UsCalcCoefficients, EveryPairGivesTheSameSolute )
{
   Component ref;
   ref.s      = 5e-13;
   ref.f_f0   = 1.5;
   ref.vbar20 = 0.72;
   ASSERT_TRUE( calc_coefficients( ref ) );
   auto from = [&]( auto set )
   {
      Component c;
      c.vbar20 = 0.72;
      set( c );
      EXPECT_TRUE( calc_coefficients( c ) );
      return c;
   };
   const Component sd = from( [&]( Component& c ) { c.s = ref.s, c.D = ref.D; } );
   const Component mf = from( [&]( Component& c ) { c.mw = ref.mw, c.f_f0 = ref.f_f0; } );
   const Component df = from( [&]( Component& c ) { c.D = ref.D, c.f_f0 = ref.f_f0; } );
   for ( const Component& c : { sd, mf, df } )
   {
      EXPECT_NEAR( c.s, ref.s, 1e-9 * ref.s );
      EXPECT_NEAR( c.D, ref.D, 1e-9 * ref.D );
      EXPECT_NEAR( c.mw, ref.mw, 1e-9 * ref.mw );
      EXPECT_NEAR( c.f_f0, ref.f_f0, 1e-9 );
   }
}

TEST( UsCalcCoefficients, InvalidInputs )
{
   Component none;
   none.vbar20 = 0.72;
   none.s      = 5e-13;
   EXPECT_FALSE( calc_coefficients( none ) ); // s alone is not a pair

   Component sign;
   sign.vbar20 = 0.72;
   sign.s      = -5e-13; // negative s needs negative buoyancy
   sign.f_f0   = 1.5;
   EXPECT_FALSE( calc_coefficients( sign ) );
   EXPECT_EQ( sign.D, 0.0 ); // left unchanged

   Component typical;
   typical.s    = 5e-13;
   typical.f_f0 = 1.5;
   EXPECT_TRUE( calc_coefficients( typical ) );
   EXPECT_EQ( typical.vbar20, TYPICAL_VBAR );
}

TEST( UsCalcSmoothing, WidthOneOrLessChangesNothing )
{
   QVector<double> v    = { 1, 5, 2, 8, 3 };
   const auto      orig = v;
   EXPECT_TRUE( gaussian_smoothing( v, 1 ).empty() );
   EXPECT_TRUE( gaussian_smoothing( v, 0 ).empty() );
   EXPECT_EQ( v, orig );
}

TEST( UsCalcSmoothing, ShortSeriesAreRejectedUnchanged )
{
   for ( int width : { 2, 3, 10, 60 } )
   {
      QVector<double> v( 2 * width - 2, 1.5 );
      v[0]            = 7.0;
      const auto orig = v;
      auto       d    = gaussian_smoothing( v, width );
      ASSERT_EQ( d.size(), 1 ) << width;
      EXPECT_EQ( d[0].issue, Issue::SeriesTooShort );
      EXPECT_EQ( d[0].severity, Severity::Error );
      EXPECT_EQ( v, orig );
   }
}

TEST( UsCalcSmoothing, ShortestAcceptedSeriesAndConstants )
{
   QVector<double> v( 2 * 5 - 1, 2.5 ); // a constant stays constant
   EXPECT_TRUE( gaussian_smoothing( v, 5 ).empty() );
   for ( double x : v )
   {
      EXPECT_NEAR( x, 2.5, 1e-15 );
   }
}

TEST( UsCalcSmoothing, NormalDistribution )
{
   EXPECT_NEAR( normal_distribution( 1.0, 0.0, 0.0 ), 0.3989422804014327, 1e-16 );
   EXPECT_NEAR( normal_distribution( 2.0, 1.0, 1.0 ), 0.19947114020071635, 1e-16 );
   EXPECT_EQ( normal_distribution( 1.0, 0.0, 1.5 ), normal_distribution( 1.0, 0.0, -1.5 ) );
}
