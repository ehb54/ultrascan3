//! \file test_us_calc_rules.cpp
// Tolerance rules, replicate summaries and report item matching of the
// calculation library, with hand-checked values.

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <random>
#include <vector>

#include "us_calc_matching.h"
#include "us_calc_replicate.h"
#include "us_calc_statistics.h"
#include "us_calc_tolerance.h"

using namespace US_Calc;

// Extraction plan example:  target 90 %, tolerance 10.
TEST( UsCalcTolerance, PercentagePointAndRelativeRulesDiffer )
{
   const ToleranceCheck pp{ ToleranceRule::AbsolutePercentagePoints, 90.0, 10.0 };
   const ToleranceCheck rel{ ToleranceRule::RelativePercentOfTarget, 90.0, 10.0 };
   EXPECT_TRUE( passes( 80.0, pp ) );
   EXPECT_FALSE( passes( 79.99, pp ) );
   EXPECT_TRUE( passes( 100.0, pp ) );
   EXPECT_FALSE( passes( 80.5, rel ) ); // lower bound 81
   EXPECT_TRUE( passes( 81.0, rel ) );
   EXPECT_TRUE( passes( 99.0, rel ) );
   EXPECT_FALSE( passes( 99.5, rel ) );
}

TEST( UsCalcTolerance, AbsoluteUnitsLimitsAndMinimum )
{
   EXPECT_TRUE( passes( 19.5, { ToleranceRule::AbsoluteUnits, 20.0, 0.5 } ) );
   EXPECT_TRUE( passes( 20.5, { ToleranceRule::AbsoluteUnits, 20.0, 0.5 } ) );
   EXPECT_FALSE( passes( 20.51, { ToleranceRule::AbsoluteUnits, 20.0, 0.5 } ) );
   EXPECT_TRUE( passes( 0.01, { ToleranceRule::UpperLimit, 0.01, 0.0 } ) );
   EXPECT_FALSE( passes( 0.0100001, { ToleranceRule::UpperLimit, 0.01, 0.0 } ) );
   EXPECT_FALSE( passes( 0.8, { ToleranceRule::ExclusiveMinimum, 0.8, 0.0 } ) );
   EXPECT_TRUE( passes( 0.80001, { ToleranceRule::ExclusiveMinimum, 0.8, 0.0 } ) );
}

TEST( UsCalcTolerance, NanFailsEveryRule )
{
   const double nan = std::numeric_limits<double>::quiet_NaN();
   for ( ToleranceRule rule : { ToleranceRule::RelativePercentOfTarget,
                                ToleranceRule::AbsolutePercentagePoints,
                                ToleranceRule::AbsoluteUnits,
                                ToleranceRule::UpperLimit,
                                ToleranceRule::ExclusiveMinimum } )
   {
      EXPECT_FALSE( passes( nan, { rule, 50.0, 10.0 } ) ) << tolerance_rule_name( rule );
      EXPECT_FALSE( tolerance_rule_name( rule ).empty() );
   }
}

namespace
{
RangeResult member( double r, std::optional<double> fraction )
{
   RangeResult m;
   m.range_concentration = r;
   m.fraction_percent    = fraction;
   return m;
}
} // namespace

TEST( UsCalcReplicate, MeanAndPopulationSd )
{
   const ReplicateResult r = replicate_summary( { member( 0.2, 20.0 ), member( 0.4, 40.0 ), member( 0.6, 60.0 ) } );
   EXPECT_EQ( r.used_count, 3u );
   EXPECT_DOUBLE_EQ( *r.mean_range_concentration, 0.4 );
   EXPECT_DOUBLE_EQ( *r.mean_fraction_percent, 40.0 );
   EXPECT_NEAR( *r.sd_fraction_percent, std::sqrt( 800.0 / 3.0 ), 1e-12 ); // divisor N, not N-1
   EXPECT_NEAR( *r.sd_range_concentration, std::sqrt( 0.08 / 3.0 ), 1e-15 );
   EXPECT_TRUE( r.diagnostics.empty() );
}

TEST( UsCalcReplicate, OneMemberHasZeroSd )
{
   const ReplicateResult r = replicate_summary( { member( 0.3, 30.0 ) } );
   EXPECT_EQ( r.used_count, 1u );
   EXPECT_EQ( *r.sd_fraction_percent, 0.0 );
   EXPECT_EQ( *r.mean_fraction_percent, 30.0 );
}

TEST( UsCalcReplicate, NegativeAndUndefinedMembersAreLeftOut )
{
   const ReplicateResult r =
      replicate_summary( { member( -0.1, -10.0 ), member( 0.4, std::nullopt ), member( 0.2, 20.0 ), member( 0.4, 40.0 ) } );
   EXPECT_EQ( r.member_count, 4u );
   EXPECT_EQ( r.used_count, 2u );
   EXPECT_DOUBLE_EQ( *r.mean_fraction_percent, 30.0 );
   EXPECT_EQ( r.diagnostics.size(), 2u );
   EXPECT_FALSE( has_error( r.diagnostics ) );
}

TEST( UsCalcReplicate, NoUsableMemberIsAnError )
{
   for ( const auto& members : { std::vector<RangeResult>{}, std::vector<RangeResult>{ member( -0.1, -10.0 ) } } )
   {
      const ReplicateResult r = replicate_summary( members );
      EXPECT_EQ( r.used_count, 0u );
      EXPECT_FALSE( r.mean_fraction_percent.has_value() );
      EXPECT_FALSE( r.sd_fraction_percent.has_value() );
      EXPECT_TRUE( has_error( r.diagnostics ) );
   }
}

// QString::section semantics, checked against Qt by hand.
TEST( UsCalcMatching, TextSection )
{
   EXPECT_EQ( text_section( "a.b.c.d", '.', -2, -2 ), "c" );
   EXPECT_EQ( text_section( "a.b.c.d", '.', 0, -2 ), "a.b.c" );
   EXPECT_EQ( text_section( "x_y_z", '_', 1, -1 ), "y_z" );
   EXPECT_EQ( text_section( "plain", '.', -2, -2 ), "" );
   EXPECT_EQ( text_section( "plain", '.', 0, -2 ), "" );
   EXPECT_EQ( text_section( "a..b", '.', 1, 1 ), "" ); // empty fields count
   EXPECT_EQ( text_section( "a.b", '.', 5, 6 ), "" );
}

TEST( UsCalcMatching, AnalysisLabel )
{
   EXPECT_EQ( analysis_label( "fixA.1A260.ee1_a2611090000_2DSA-IT_000001_i01.model" ), "a2611090000_2DSA-IT_000001_i01" );
   EXPECT_EQ( analysis_label( "fixA.1A260.ee1_a2611090000_2DSA-MC_000001_mcN003.model" ), "a2611090000_2DSA-MC_000001_mcN003" );
   // No '_' in the second-to-last field:  fall back to all but the last field
   EXPECT_EQ( analysis_label( "run.1A260.edit.model" ), "run.1A260.edit" );
}

TEST( UsCalcMatching, MethodParts )
{
   const std::string it = "fixA.1A260.ee1_a2611090000_2DSA-IT_000001_i01.model";
   const std::string mc = "fixA.1A260.ee1_a2611090000_2DSA-MC_000001_mcN003.model";
   const std::string pc = "fixA.1A260.ee1_a2611090000_PCSA-SL_000001_i01.model";
   EXPECT_TRUE( method_matches( "2DSA-IT", it ) );
   EXPECT_FALSE( method_matches( "2DSA-MC", it ) );
   EXPECT_TRUE( method_matches( "2DSA-MC", mc ) );
   EXPECT_TRUE( method_matches( "2DSA", mc ) );
   EXPECT_TRUE( method_matches( "PCSA-SL", pc ) ); // reduced to PCSA
   EXPECT_TRUE( method_matches( "PCSA-IS", pc ) ); // also PCSA:  the reporter ignores the curve type
   EXPECT_FALSE( method_matches( "GA", it ) );
}

// Every value is counted once, the maximum included, so the area is the
// total concentration times the bin width.
TEST( UsCalcStatistics, AreaCountsEveryValueIncludingTheMaximum )
{
   const SoluteStatistics st = solute_statistics( { 1.0, 2.0, 3.0 }, { 1.0, 1.0, 1.0 } );
   ASSERT_TRUE( st.area.has_value() );
   EXPECT_NEAR( *st.area, 3.0 * ( 2.0 / 50.0 ), 1e-15 );

   std::mt19937_64                        rng( 7 );
   std::uniform_real_distribution<double> u( 0.0, 1.0 );
   for ( int t = 0; t < 2000; ++t )
   {
      const int           n = 2 + t % 40;
      std::vector<double> v( n ), c( n );
      double              total = 0.0;
      for ( int i = 0; i < n; ++i )
      {
         v[i] = ( 1 + 9 * u( rng ) ) * 1e-13;
         c[i] = u( rng );
         total += c[i];
      }
      const SoluteStatistics s = solute_statistics( v, c );
      ASSERT_TRUE( s.area.has_value() );
      EXPECT_NEAR( *s.area, total * ( s.maximum - s.minimum ) / 50.0, 1e-12 * total * ( s.maximum - s.minimum ) / 50.0 );
   }
}

TEST( UsCalcStatistics, UndefinedValuesAreEmpty )
{
   // One nonzero concentration:  the weighted variance divides by zero.
   const SoluteStatistics one = solute_statistics( { 1.0, 2.0 }, { 1.0, 0.0 } );
   EXPECT_FALSE( one.variance.has_value() );
   EXPECT_FALSE( has_error( one.diagnostics ) );
   EXPECT_FALSE( one.diagnostics.empty() );
   // No values
   EXPECT_TRUE( has_error( solute_statistics( {}, {} ).diagnostics ) );
   // A dominant first value gives the first value as both upper limits
   // (the legacy index is set to -1 and then corrected to 0)
   const SoluteStatistics dom = solute_statistics( { 1.0, 2.0, 3.0 }, { 1000.0, 1.0, 1.0 } );
   ASSERT_TRUE( dom.confidence99_high.has_value() );
   EXPECT_EQ( *dom.confidence99_high, 1.0 );
   // A single value has no upper index
   const SoluteStatistics single = solute_statistics( { 2.0 }, { 1.0 } );
   EXPECT_FALSE( single.confidence99_high.has_value() );
}

TEST( UsCalcStatistics, LineFitOfALine )
{
   const double  x[] = { 0, 1, 2, 3 };
   const double  y[] = { 1, 3, 5, 7 };
   const LineFit f   = linefit( x, y, 4 );
   EXPECT_DOUBLE_EQ( f.slope, 2.0 );
   EXPECT_DOUBLE_EQ( f.intercept, 1.0 );
   EXPECT_DOUBLE_EQ( f.correlation, 1.0 );
   EXPECT_DOUBLE_EQ( f.average, 4.0 );
}
