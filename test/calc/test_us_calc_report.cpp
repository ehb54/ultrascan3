//! \file test_us_calc_report.cpp
// The GMP reporter's rules in US_Calc, with hand-checked values at their
// edges:  report items on a model, replicate groups, run checks, the
// report's duration format and the lookups that choose a triple's inputs.

#include <gtest/gtest.h>

#include <cmath>

#include <QStringList>
#include <QtNumeric>

#include "us_calc_matching.h"
#include "us_calc_replicate_group.h"
#include "us_calc_report_item.h"
#include "us_calc_report_lookup.h"
#include "us_calc_run_checks.h"
#include "us_calc_test_print.h"
#include "us_calc_tolerance.h"
#include "us_calc_units.h"
#include "us_reporter_gmp_duration.h"

using namespace US_Calc;
using namespace US_Calc::Gmp;

namespace
{
// A component with the given s (seconds) and f/f0; the concentrations below
// are chosen so that every fraction R / C is exact in binary.
Component solute( double s, double f_f0, double c )
{
   Component x;
   x.s                    = s;
   x.D                    = 5.0e-7;
   x.mw                   = 75000.0;
   x.f                    = 4.0e-8;
   x.f_f0                 = f_f0;
   x.vbar20               = 0.72;
   x.signal_concentration = c;
   return x;
}

// C = 1.0:  s 4.5 S (0.25), 6.0 S (0.5), 9.0 S (0.25); f/f0 1.2, 1.6, 2.0.
Model three_solutes()
{
   Model m;
   m.components  = { solute( 4.5e-13, 1.2, 0.25 ), solute( 6.0e-13, 1.6, 0.5 ), solute( 9.0e-13, 2.0, 0.25 ) };
   m.description = "fixA.1A260.ee1_a2611090000_2DSA-IT_000001_i01.model";
   return m;
}

ReportItem item( const QString& type, double low, double high, double target, double tolerance )
{
   return { type, "2DSA-IT", low, high, target, tolerance };
}
} // namespace

// 1a.  Each range type gives range_summary's result; the fraction is R / C.
TEST( UsCalcReportItem, RangeTypesUseTheRangeSummary )
{
   const Model m = three_solutes();
   for ( const ReportItem& it : { item( "s", 4.0, 7.0, 75.0, 0.0 ),
                                  item( "f/f0", 1.0, 1.7, 75.0, 0.0 ),
                                  item( "D", 0.0, 10.0, 100.0, 0.0 ),
                                  item( "MW", 0.0, 100.0, 100.0, 0.0 ) } )
   {
      SCOPED_TRACE( it.type );
      const ReportItemResult r = evaluate_report_item( m, it );
      bool                   ok;
      const RangeResult      direct = range_summary( m, { quantity_from_report_type( it.type, &ok ), it.low, it.high } );
      EXPECT_TRUE( r.range_type );
      EXPECT_EQ( r.total_concentration, 1.0 );
      EXPECT_EQ( r.range_concentration, direct.range_concentration );
      EXPECT_EQ( r.fraction_percent, direct.fraction_percent );
      EXPECT_EQ( r.within_range_mean, direct.within_range_mean );
      EXPECT_EQ( r.fraction_percent, it.target_percent );
      EXPECT_TRUE( r.passed );
   }
}

// 1a.  A type that is not a range type selects nothing:  R is 0, the mean is
// NaN, and the fraction is (0 / C) * 100.
TEST( UsCalcReportItem, OtherTypesGiveZeroOverTheTotal )
{
   const Model m = three_solutes();
   for ( const QString& type : { QString( "vbar" ), QString( "Radius" ), QString() } )
   {
      SCOPED_TRACE( type );
      const ReportItemResult r = evaluate_report_item( m, item( type, 0.0, 100.0, 0.0, 0.0 ) );
      EXPECT_FALSE( r.range_type );
      EXPECT_EQ( r.range_concentration, 0.0 );
      EXPECT_TRUE( qIsNaN( r.within_range_mean ) );
      EXPECT_EQ( r.fraction_percent, 0.0 );
      EXPECT_TRUE( r.passed );                         // a 0 % target with tolerance 0
      EXPECT_TRUE( has_error( r.range.diagnostics ) ); // UnsupportedQuantity
   }

   // A zero total:  0 / 0 is NaN, which fails every target.
   Model zero;
   zero.components          = { solute( 4.5e-13, 1.2, 1.0 ), solute( 6.0e-13, 1.6, -1.0 ) };
   const ReportItemResult z = evaluate_report_item( zero, item( "vbar", 0.0, 1.0, 0.0, 100.0 ) );
   EXPECT_EQ( z.total_concentration, 0.0 );
   EXPECT_TRUE( qIsNaN( z.fraction_percent ) );
   EXPECT_FALSE( z.passed );

   // A negative total:  (0 / -1) * 100 is -0, which equals 0.
   Model negative;
   negative.components      = { solute( 4.5e-13, 1.2, -1.0 ) };
   const ReportItemResult n = evaluate_report_item( negative, item( "vbar", 0.0, 1.0, 0.0, 0.0 ) );
   EXPECT_EQ( n.fraction_percent, 0.0 );
   EXPECT_TRUE( std::signbit( n.fraction_percent ) );
   EXPECT_TRUE( n.passed );

   // No components:  C is 0, so the fraction is NaN.
   EXPECT_TRUE( qIsNaN( evaluate_report_item( Model(), item( "vbar", 0.0, 1.0, 0.0, 0.0 ) ).fraction_percent ) );
}

// 1b.  The fraction passes within the tolerance of the target, in
// percentage points, both edges included.
TEST( UsCalcReportItem, PercentagePointEdges )
{
   const Model m      = three_solutes(); // s 4.0 - 7.0 gives exactly 75 %
   auto        passed = [&]( double target, double tolerance )
   { return evaluate_report_item( m, item( "s", 4.0, 7.0, target, tolerance ) ).passed; };
   EXPECT_TRUE( passed( 65.0, 10.0 ) ); // lower edge of the window
   EXPECT_TRUE( passed( 85.0, 10.0 ) ); // upper edge
   EXPECT_FALSE( passed( 64.99, 10.0 ) );
   EXPECT_FALSE( passed( 85.01, 10.0 ) );
   EXPECT_TRUE( passed( 75.0, 0.0 ) ); // tolerance 0, fraction on target
   EXPECT_FALSE( passed( 74.0, 0.0 ) );
   // Percentage points, not percent of target:  the relative rule's window
   // for target 85, tolerance 10 starts at 76.5 and would fail 75.
   EXPECT_FALSE( passes( 75.0, { ToleranceRule::RelativePercentOfTarget, 85.0, 10.0 } ) );

   // 0 % targets
   EXPECT_FALSE( passed( 0.0, 74.0 ) );
   EXPECT_TRUE( passed( 0.0, 75.0 ) );
   const ReportItemResult none = evaluate_report_item( m, item( "s", 10.0, 20.0, 0.0, 0.0 ) );
   EXPECT_EQ( none.fraction_percent, 0.0 );
   EXPECT_TRUE( none.passed );
}

// 1c.  Inverted and degenerate ranges.
TEST( UsCalcReportItem, InvertedAndDegenerateRanges )
{
   const Model            m = three_solutes();

   // low > high selects nothing:  0 %, as on main (not nan%).
   const ReportItemResult inverted = evaluate_report_item( m, item( "s", 7.0, 4.0, 0.0, 0.0 ) );
   EXPECT_EQ( inverted.range_concentration, 0.0 );
   EXPECT_EQ( inverted.fraction_percent, 0.0 );
   EXPECT_TRUE( qIsNaN( inverted.within_range_mean ) );
   EXPECT_TRUE( inverted.passed );
   EXPECT_FALSE( has_error( inverted.range.diagnostics ) );

   // low == high on a component's value selects that component (f/f0 is
   // not scaled, so the bound equals the stored value).
   const ReportItemResult point = evaluate_report_item( m, item( "f/f0", 1.6, 1.6, 50.0, 0.0 ) );
   EXPECT_EQ( point.range.selected_count, 1 );
   EXPECT_EQ( point.fraction_percent, 50.0 );
   EXPECT_EQ( point.within_range_mean, 1.6 );
   EXPECT_TRUE( point.passed );

   // low == high between values selects nothing.
   const ReportItemResult between = evaluate_report_item( m, item( "f/f0", 1.4, 1.4, 0.0, 0.0 ) );
   EXPECT_EQ( between.range.selected_count, 0 );
   EXPECT_EQ( between.fraction_percent, 0.0 );

   // A range around every component gives exactly 100 %.
   const ReportItemResult all = evaluate_report_item( m, item( "f/f0", 0.0, 10.0, 100.0, 0.0 ) );
   EXPECT_EQ( all.range.selected_count, 3 );
   EXPECT_EQ( all.fraction_percent, 100.0 );
   EXPECT_TRUE( all.passed );
}

// 1d.  The Integration Results table is shown when any item's method
// applies to the model.
TEST( UsCalcReportItem, AnyMethodMatches )
{
   const QString it = "fixA.1A260.ee1_a2611090000_2DSA-IT_000001_i01.model";
   const QString mc = "fixA.1A260.ee1_a2611090000_2DSA-MC_000001_mcN003.model";
   const QString pc = "fixA.1A260.ee1_a2611090000_PCSA-SL_000001_i01.model";

   EXPECT_FALSE( any_method_matches( {}, it ) );
   EXPECT_FALSE( any_method_matches( { "2DSA-MC", "PCSA-SL", "GA" }, it ) );
   EXPECT_TRUE( any_method_matches( { "2DSA-MC", "2DSA-IT", "GA" }, it ) ); // one of several
   EXPECT_TRUE( any_method_matches( { "2DSA-MC" }, mc ) );
   EXPECT_TRUE( any_method_matches( { "PCSA-IS" }, pc ) ); // every PCSA method counts as PCSA
   EXPECT_TRUE( any_method_matches( { "2DSA" }, mc ) );    // a part of the label
   // An empty method has one empty part, which every label contains.
   EXPECT_TRUE( any_method_matches( { QString() }, it ) );
}

// 2.  VELOCITY-MWL items:  as report items when the total is positive.
TEST( UsCalcVelMwlItem, PositiveTotalAsReportItem )
{
   const Model m = three_solutes();
   for ( const ReportItem& it :
         { item( "s", 4.0, 7.0, 75.0, 0.0 ), item( "f/f0", 1.6, 1.6, 40.0, 10.0 ), item( "s", 7.0, 4.0, 0.0, 0.0 ) } )
   {
      SCOPED_TRACE( it.type + " " + QString::number( it.low ) + "-" + QString::number( it.high ) );
      const VelMwlItemResult v = evaluate_velmwl_item( m, it );
      const ReportItemResult r = evaluate_report_item( m, it );
      EXPECT_TRUE( v.supported );
      EXPECT_EQ( v.fraction_percent, r.fraction_percent );
      EXPECT_EQ( v.passed, r.passed );
   }
   // Percentage-point edges
   EXPECT_TRUE( evaluate_velmwl_item( m, item( "s", 4.0, 7.0, 65.0, 10.0 ) ).passed );
   EXPECT_FALSE( evaluate_velmwl_item( m, item( "s", 4.0, 7.0, 64.99, 10.0 ) ).passed );
}

// 2.  A type that is not a range type shows n/a; a model without components
// is never examined and gives 0; a total that is not positive gives 0.
TEST( UsCalcVelMwlItem, UnsupportedEmptyAndNonPositiveTotals )
{
   const VelMwlItemResult unsupported = evaluate_velmwl_item( three_solutes(), item( "vbar", 0.0, 1.0, 0.0, 0.0 ) );
   EXPECT_FALSE( unsupported.supported );
   EXPECT_EQ( unsupported.fraction_percent, 0.0 );

   for ( const QString& type : { QString( "s" ), QString( "vbar" ) } )
   {
      SCOPED_TRACE( type );
      const VelMwlItemResult empty = evaluate_velmwl_item( Model(), item( type, 0.0, 10.0, 0.0, 0.0 ) );
      EXPECT_TRUE( empty.supported );
      EXPECT_EQ( empty.fraction_percent, 0.0 );
      EXPECT_TRUE( empty.passed );
   }

   // C = 0:  the report item divides (NaN), the VELOCITY-MWL item gives 0.
   Model zero;
   zero.components = { solute( 4.5e-13, 1.2, 1.0 ), solute( 9.0e-13, 2.0, -1.0 ) };
   EXPECT_TRUE( std::isinf( evaluate_report_item( zero, item( "s", 4.0, 7.0, 0.0, 0.0 ) ).fraction_percent ) );
   EXPECT_EQ( evaluate_velmwl_item( zero, item( "s", 4.0, 7.0, 0.0, 0.0 ) ).fraction_percent, 0.0 );

   // C < 0 with R > 0:  the report item gives -100 %, the VELOCITY-MWL item 0.
   Model negative;
   negative.components = { solute( 4.5e-13, 1.2, 1.0 ), solute( 9.0e-13, 2.0, -2.0 ) };
   EXPECT_EQ( evaluate_report_item( negative, item( "s", 4.0, 7.0, 0.0, 0.0 ) ).fraction_percent, -100.0 );
   const VelMwlItemResult n = evaluate_velmwl_item( negative, item( "s", 4.0, 7.0, 0.0, 0.0 ) );
   EXPECT_EQ( n.fraction_percent, 0.0 );
   EXPECT_TRUE( n.passed );
}

namespace
{
ReplicateItem replicate_item( double range_concentration, double fraction_percent )
{
   ReplicateItem r;
   r.type                = "s";
   r.method              = "2DSA-IT";
   r.low                 = 3.2;
   r.high                = 3.7;
   r.integration_value   = 0.57;
   r.target_percent      = 26.0;
   r.tolerance           = 10.0;
   r.range_concentration = range_concentration;
   r.fraction_percent    = fraction_percent;
   return r;
}

// One channel per member, each with one item.
QVector<QVector<ReplicateItem>> one_item_each( const QVector<ReplicateItem>& items )
{
   QVector<QVector<ReplicateItem>> channels;
   for ( const ReplicateItem& item : items )
   {
      channels.push_back( { item } );
   }
   return channels;
}
} // namespace

// 3a.  Members match the reference exactly in type, method and bounds, and
// in the integration value as QString::number formats it.
TEST( UsCalcReplicateGroup, SameItem )
{
   const ReplicateItem ref = replicate_item( -1.0, -1.0 );
   EXPECT_TRUE( same_replicate_item( ref, ref ) );
   auto changed = [&]( auto change )
   {
      ReplicateItem x = ref;
      change( x );
      return same_replicate_item( ref, x );
   };
   EXPECT_FALSE( changed( []( ReplicateItem& x ) { x.type = "D"; } ) );
   EXPECT_FALSE( changed( []( ReplicateItem& x ) { x.method = "2DSA-MC"; } ) );
   EXPECT_FALSE( changed( []( ReplicateItem& x ) { x.low = std::nextafter( 3.2, 4.0 ); } ) );
   EXPECT_FALSE( changed( []( ReplicateItem& x ) { x.high = 3.71; } ) );
   EXPECT_FALSE( changed( []( ReplicateItem& x ) { x.integration_value = 0.570001; } ) );
   // QString::number keeps 6 significant digits, so these format as "0.57".
   EXPECT_TRUE( changed( []( ReplicateItem& x ) { x.integration_value = 0.5700001; } ) );
   EXPECT_TRUE( changed( []( ReplicateItem& x ) { x.integration_value = 0.56999999; } ) );
   // A member's own target and tolerance do not matter.
   EXPECT_TRUE( changed(
      []( ReplicateItem& x )
      {
         x.target_percent = 50.0;
         x.tolerance      = 1.0;
      } ) );
}

// 3a.  A member's report is the one of the first channel description that
// contains the channel.
TEST( UsCalcReplicateGroup, ChannelIndex )
{
   const QStringList descs = { "1A:UV/vis.:(unspecified)", "2A:UV/vis.:buffer", "1A:Interf.:(unspecified)" };
   EXPECT_EQ( replicate_channel_index( descs, "1A:UV/vis." ), 0 );
   EXPECT_EQ( replicate_channel_index( descs, "2A:UV/vis." ), 1 );
   EXPECT_EQ( replicate_channel_index( descs, "1A:Interf." ), 2 );
   EXPECT_EQ( replicate_channel_index( descs, "3A:UV/vis." ), -1 );
   EXPECT_EQ( replicate_channel_index( {}, "1A:UV/vis." ), -1 );
}

// 3b.  Which triples take part, and how they split into subgroups.
TEST( UsCalcReplicateGroup, TriplesAndSubgroups )
{
   EXPECT_TRUE( replicate_triple_included( "260", "", true ) );
   EXPECT_FALSE( replicate_triple_included( "260", "260:280", true ) );
   EXPECT_TRUE( replicate_triple_included( "280", "260", true ) );
   EXPECT_TRUE( replicate_triple_included( "260", "2600", true ) ); // whole wavelengths only
   EXPECT_FALSE( replicate_triple_included( "260", "", false ) );   // Integration Results off

   const QStringList     triples = { "1A.260", "2A.280", "3A.260", "1A.280" };
   const ReplicateLayout all     = replicate_layout( triples, { true, true, true, true } );
   EXPECT_EQ( all.channels, QStringList( { "1A", "2A", "3A" } ) );
   ASSERT_EQ( all.subgroups.size(), 2 );
   EXPECT_EQ( all.subgroups[0].wavelength, "260" );
   EXPECT_EQ( all.subgroups[0].channels, QStringList( { "1A", "3A" } ) );
   EXPECT_EQ( all.subgroups[1].wavelength, "280" );
   EXPECT_EQ( all.subgroups[1].channels, QStringList( { "2A", "1A" } ) );

   const ReplicateLayout some = replicate_layout( triples, { true, false, true, true } );
   EXPECT_EQ( some.channels, QStringList( { "1A", "3A" } ) );
   ASSERT_EQ( some.subgroups.size(), 2 );
   EXPECT_EQ( some.subgroups[1].channels, QStringList( { "1A" } ) );

   EXPECT_TRUE( replicate_layout( triples, { false, false, false, false } ).subgroups.isEmpty() );
}

// 3c.  Members not evaluated (-1) or with a negative in-range concentration
// are left out; a group without members gives 0.
TEST( UsCalcReplicateGroup, NegativeAndEmptyMembers )
{
   const ReplicateItem  ref   = replicate_item( -1.0, -1.0 );
   ReplicateGroupResult empty = evaluate_replicate_group( ref, one_item_each( { ref, ref } ), 2 );
   EXPECT_EQ( empty.summary.used_count, 0 );
   EXPECT_EQ( empty.mean_range_concentration, 0.0 );
   EXPECT_EQ( empty.mean_fraction_percent, 0.0 );
   EXPECT_EQ( empty.sd_range_concentration, 0.0 );
   EXPECT_EQ( empty.sd_fraction_percent, 0.0 );
   EXPECT_FALSE( empty.passed ); // 0 is outside 26 +/- 10 % of 26

   ReplicateItem zero_target  = ref;
   zero_target.target_percent = 0.0;
   EXPECT_TRUE( evaluate_replicate_group( zero_target, one_item_each( { ref } ), 1 ).passed ); // 0 within 0 +/- 0

   // A channel without a report contributes nothing.
   EXPECT_EQ( evaluate_replicate_group( ref, { {} }, 1 ).summary.member_count, 0 );

   // One of three left out (negative in-range concentration).
   const ReplicateGroupResult two = evaluate_replicate_group(
      ref, one_item_each( { replicate_item( 0.2, 24.0 ), replicate_item( -0.1, 90.0 ), replicate_item( 0.4, 28.0 ) } ), 3 );
   EXPECT_EQ( two.summary.used_count, 2 );
   EXPECT_EQ( two.mean_fraction_percent, 26.0 );
   EXPECT_EQ( two.sd_fraction_percent, 2.0 );
   EXPECT_TRUE( two.passed );

   // Every matching item of a channel is a member; others are not.
   ReplicateItem other = replicate_item( 0.9, 90.0 );
   other.type          = "D";
   const ReplicateGroupResult several =
      evaluate_replicate_group( ref, { { replicate_item( 0.2, 24.0 ), other, replicate_item( 0.4, 28.0 ) } }, 1 );
   EXPECT_EQ( several.summary.used_count, 2 );
   EXPECT_EQ( several.mean_fraction_percent, 26.0 );

   // A member with a NaN fraction (zero total) makes the mean NaN, which fails.
   const ReplicateGroupResult nan =
      evaluate_replicate_group( ref, one_item_each( { replicate_item( 0.2, 24.0 ), replicate_item( 0.0, qQNaN() ) } ), 2 );
   EXPECT_TRUE( qIsNaN( nan.mean_fraction_percent ) );
   EXPECT_FALSE( nan.passed );
}

// 3d.  The SD is shown when the subgroup lists more than one triple, even if
// only one of them gives a member.
TEST( UsCalcReplicateGroup, ShowSd )
{
   const ReplicateItem ref = replicate_item( -1.0, -1.0 );
   EXPECT_FALSE( evaluate_replicate_group( ref, one_item_each( { replicate_item( 0.2, 26.0 ) } ), 1 ).show_sd );
   const ReplicateGroupResult one_used = evaluate_replicate_group( ref, one_item_each( { replicate_item( 0.2, 26.0 ), ref } ), 2 );
   EXPECT_TRUE( one_used.show_sd );
   EXPECT_EQ( one_used.summary.used_count, 1 );
   EXPECT_EQ( one_used.sd_fraction_percent, 0.0 );
}

// 3e.  The mean passes within tolerance percent of the target (relative),
// unlike a single triple (percentage points); see ultrascan-tickets#1299.
TEST( UsCalcReplicateGroup, RelativeTolerance )
{
   const ReplicateItem ref         = replicate_item( -1.0, -1.0 );
   const double        lower       = 26.0 * ( 1.0 - 10.0 / 100.0 );
   const double        upper       = 26.0 * ( 1.0 + 10.0 / 100.0 );
   auto                mean_passes = [&]( double fraction )
   { return evaluate_replicate_group( ref, one_item_each( { replicate_item( 0.2, fraction ) } ), 1 ).passed; };
   EXPECT_TRUE( mean_passes( lower ) );
   EXPECT_TRUE( mean_passes( upper ) );
   EXPECT_TRUE( mean_passes( std::nextafter( lower, 0.0 ) ) );
   EXPECT_TRUE( mean_passes( std::nextafter( upper, 100.0 ) ) );

   // Three triples at 30 % pass one by one (26 +/- 10 points), but their
   // 30 % mean fails (26 +/- 10 % is 23.4 - 28.6).
   EXPECT_TRUE( passes( 30.0, { ToleranceRule::AbsolutePercentagePoints, 26.0, 10.0 } ) );
   const ReplicateGroupResult three = evaluate_replicate_group(
      ref, one_item_each( { replicate_item( 0.3, 30.0 ), replicate_item( 0.3, 30.0 ), replicate_item( 0.3, 30.0 ) } ), 3 );
   EXPECT_EQ( three.mean_fraction_percent, 30.0 );
   EXPECT_TRUE( three.passed );
}

// 4a.  Total concentration:  the target times the channel's factor, within
// tolerance percent of it.
TEST( UsCalcRunChecks, TotalConcentration )
{
   const RunCheckResult r = check_total_concentration( 0.6, 0.5, 1.2, 10.0 );
   EXPECT_EQ( r.check.rule, ToleranceRule::RelativePercentOfTarget );
   EXPECT_EQ( r.check.target, 0.5 * 1.2 );
   EXPECT_TRUE( r.passed );
   EXPECT_EQ( check_total_concentration( 0.6, 0.6, 1.0, 10.0 ).check.target, 0.6 );
   EXPECT_FALSE( check_total_concentration( 0.6, 0.5, 0.0, 10.0 ).passed ); // factor 0:  target 0
   EXPECT_TRUE( check_total_concentration( 0.0, 0.5, 0.0, 10.0 ).passed );
   const double lower = 0.6 * ( 1 - 10.0 / 100.0 );
   EXPECT_TRUE( check_total_concentration( lower, 0.6, 1.0, 10.0 ).passed );
   EXPECT_FALSE( check_total_concentration( std::nextafter( lower, 0.0 ), 0.6, 1.0, 10.0 ).passed );
}

// 4a.  The other checks of the model section, each at its edge.
TEST( UsCalcRunChecks, ModelSectionEdges )
{
   // Experiment duration:  19800 s +/- 10 %
   EXPECT_TRUE( check_experiment_duration( 19800.0 * ( 1 - 10.0 / 100.0 ), 19800.0, 10.0 ).passed );
   EXPECT_TRUE( check_experiment_duration( 19800.0 * ( 1 + 10.0 / 100.0 ), 19800.0, 10.0 ).passed );
   EXPECT_FALSE( check_experiment_duration( std::nextafter( 19800.0 * ( 1 + 10.0 / 100.0 ), 1e9 ), 19800.0, 10.0 ).passed );

   // RMSD:  equal to the limit passes
   EXPECT_TRUE( check_rmsd( 0.002, 0.002 ).passed );
   EXPECT_FALSE( check_rmsd( std::nextafter( 0.002, 1.0 ), 0.002 ).passed );
   EXPECT_EQ( check_rmsd( 0.001, 0.002 ).check.rule, ToleranceRule::UpperLimit );

   // Intensity:  equal to the minimum fails
   EXPECT_FALSE( check_minimum_intensity( 10000.0, 10000.0 ).passed );
   EXPECT_TRUE( check_minimum_intensity( std::nextafter( 10000.0, 1e9 ), 10000.0 ).passed );
   EXPECT_EQ( check_minimum_intensity( 1.0, 0.0 ).check.rule, ToleranceRule::ExclusiveMinimum );

   // Loading volume:  450 uL +/- 5 %
   EXPECT_TRUE( check_loading_volume( 450.0 * ( 1 - 5.0 / 100.0 ), 450.0, 5.0 ).passed );
   EXPECT_FALSE( check_loading_volume( std::nextafter( 450.0 * ( 1 - 5.0 / 100.0 ), 0.0 ), 450.0, 5.0 ).passed );
   EXPECT_FALSE( check_loading_volume( qQNaN(), 450.0, 5.0 ).passed );
}

// 4a.  The Minimum Intensity row is shown only for intensity data of a
// triple that is not an interference triple.
TEST( UsCalcRunChecks, MinimumIntensityShown )
{
   EXPECT_FALSE( minimum_intensity_shown( "DiskAUC:Absorbance", false ) );
   EXPECT_FALSE( minimum_intensity_shown( "DiskAUC:PseudoAbsorbance", false ) );
   EXPECT_TRUE( minimum_intensity_shown( "INSTRUMENT", false ) );
   EXPECT_TRUE( minimum_intensity_shown( "DiskAUC:Intensity", false ) );
   EXPECT_FALSE( minimum_intensity_shown( "INSTRUMENT", true ) );
}

// 4b.  The run-details table:  duration, temperature and speed.
TEST( UsCalcRunChecks, RunDetails )
{
   EXPECT_EQ( run_duration_target( 3600.0, 60000.0, 400.0 ), 3750.0 );
   // Acceleration 0:  an infinite target, which every duration fails.
   EXPECT_TRUE( std::isinf( run_duration_target( 3600.0, 60000.0, 0.0 ) ) );
   EXPECT_FALSE( check_run_duration( 62.5, run_duration_target( 3600.0, 60000.0, 0.0 ), 10.0 ).passed );

   // Measured in minutes, checked in seconds
   const RunCheckResult d = check_run_duration( 62.5, 3750.0, 0.0 );
   EXPECT_EQ( d.value, 3750.0 );
   EXPECT_TRUE( d.passed );
   EXPECT_FALSE( check_run_duration( 62.4, 3750.0, 0.0 ).passed );

   // Temperature:  +/- 0.5 C
   EXPECT_EQ( check_temperature( 20.0, 20.0 ).check.tolerance, 0.5 );
   EXPECT_TRUE( check_temperature( 19.5, 20.0 ).passed );
   EXPECT_TRUE( check_temperature( 20.5, 20.0 ).passed );
   EXPECT_FALSE( check_temperature( 20.51, 20.0 ).passed );
   EXPECT_FALSE( check_temperature( 19.49, 20.0 ).passed );

   // Speed:  +/- 4 rpm
   EXPECT_EQ( check_speed( 60000.0, 60000.0 ).check.tolerance, 4.0 );
   EXPECT_TRUE( check_speed( 59996.0, 60000.0 ).passed );
   EXPECT_TRUE( check_speed( 60004.0, 60000.0 ).passed );
   EXPECT_FALSE( check_speed( 60004.01, 60000.0 ).passed );
}

// 4c.  Durations as the report shows them:  hours floored, the rest rounded
// to minutes.
TEST( UsReporterGmpDuration, HoursAndMinutes )
{
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 0.0 ), "0 h 00 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 59.0 ), "0 h 01 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 89.0 ), "0 h 01 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 90.0 ), "0 h 02 m" ); // qRound rounds half up
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 3600.0 ), "1 h 00 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 19800.0 ), "5 h 30 m" );
   // A rest of 59.5 minutes or more rounds to 60:  the report shows "60 m"
   // rather than the next hour.
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 3599.9 ), "0 h 60 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 7170.0 ), "1 h 60 m" );
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( 7169.9 ), "1 h 59 m" );
   // A negative duration floors to -1 hour.
   EXPECT_EQ( US_ReporterGmp::duration_hours_minutes( -30.0 ), "-1 h 60 m" );
}

// 5c.  The parts of a model description, by position.
TEST( UsCalcReportLookup, ModelDescription )
{
   const ModelDescriptionParts p = parse_model_description( "fixA.2A660.ee1_a2611090000_2DSA-IT_000001_i01.model" );
   EXPECT_TRUE( p.ok );
   EXPECT_EQ( p.triple, "2A660" );
   EXPECT_EQ( p.cell, "2" );
   EXPECT_EQ( p.wavelength, "660" );
   EXPECT_EQ( p.analysis, "a2611090000_2DSA-IT_000001_i01" );
   EXPECT_EQ( p.model_name, "2DSA-IT" );

   EXPECT_EQ( parse_model_description( "fixA.1A260.ee1_a2611090000_PCSA-SL_000001_i01.model" ).model_name, "PCSA" );
   EXPECT_EQ( parse_model_description( "fixA.1A260.ee1_a2611090000_2DSA-CG-IT_000001_i01.model" ).model_name, "2DSA-IT" );
   EXPECT_EQ( parse_model_description( "fixA.1A260.ee1_a2611090000_2DSA-CG_000001_i01.model" ).model_name, "2DSA" );

   // Too few fields:  no triple, or no model name in the analysis label
   EXPECT_FALSE( parse_model_description( "fixA" ).ok );
   EXPECT_FALSE( parse_model_description( "run.model" ).ok );
   EXPECT_TRUE( parse_model_description( "run.model" ).triple.isEmpty() );
}

// 5a.  The channel whose report applies to a triple.
TEST( UsCalcReportLookup, ReportChannel )
{
   const QStringList descs = { "1A:UV/vis.:(unspecified)", "2A:UV/vis.:buffer", "2A:Interf.:buffer" };
   EXPECT_EQ( report_channel_index( descs, "2A660", false, false ), 1 );
   EXPECT_EQ( report_channel_index( descs, "2A660", true, false ), 2 ); // interference triple:  Interf channels only
   EXPECT_EQ( report_channel_index( descs, "1A260", false, false ), 0 );
   EXPECT_EQ( report_channel_index( descs, "3A260", false, false ), -1 );
   EXPECT_EQ( report_channel_index( descs, "1A660", true, false ), -1 ); // no interference channel for cell 1
   EXPECT_EQ( report_channel_index( descs, "2S260", false, true ), 1 );  // simulated absorbance:  "2A" read as "2S"
   EXPECT_EQ( report_channel_index( descs, "2S260", false, false ), -1 );
   // The first channel that matches wins.
   EXPECT_EQ( report_channel_index( { "2A:UV/vis.:first", "2A:UV/vis.:second" }, "2A280", false, false ), 0 );
   EXPECT_EQ( report_channel_index( {}, "2A280", false, false ), -1 );
}

// 5b.  The intensity that applies to a triple.
TEST( UsCalcReportLookup, Intensity )
{
   EXPECT_EQ( intensity_triple( "2S260", true ), "2A260" );
   EXPECT_EQ( intensity_triple( "2S260", false ), "2S260" );

   // One wavelength:  its value whatever the key
   EXPECT_EQ( triple_intensity( { { "260", "1000" } }, "2A280" ), 1000.0 );
   // Several:  the first key, in key order, that the triple contains
   const QMap<QString, QString> mwl = { { "260", "1000" }, { "280", "2000" } };
   EXPECT_EQ( triple_intensity( mwl, "2A280" ), 2000.0 );
   EXPECT_TRUE( qIsNaN( triple_intensity( mwl, "2A230" ) ) );
   EXPECT_EQ( triple_intensity( { { "26", "5" }, { "260", "1000" } }, "2A260" ), 5.0 );
   EXPECT_TRUE( qIsNaN( triple_intensity( {}, "2A260" ) ) );
}

// 5d.  The rows the report mask shows.
TEST( UsCalcReportLookup, MaskRows )
{
   EXPECT_EQ( mask_triple( "2A660", true ), "2AInterference" );
   EXPECT_EQ( mask_triple( "2A660", false ), "2A660" );

   const MaskRows some = mask_rows( { { "Total Concentration", "1" }, { "RMSD", "0" }, { "Experiment Duration", "1" } } );
   EXPECT_TRUE( some.total_concentration );
   EXPECT_FALSE( some.rmsd );
   EXPECT_TRUE( some.experiment_duration );
   EXPECT_FALSE( some.minimum_intensity );
   EXPECT_FALSE( some.loading_volume );
   EXPECT_FALSE( some.integration_results );
   EXPECT_TRUE( some.comparison_section() );

   const MaskRows integration = mask_rows( { { "Integration Results", "1" } } );
   EXPECT_TRUE( integration.integration_results );
   EXPECT_FALSE( integration.comparison_section() );

   const MaskRows each = mask_rows( { { "Minimum Intensity", "1" }, { "Loading Volume", "2" } } );
   EXPECT_TRUE( each.minimum_intensity );
   EXPECT_TRUE( each.loading_volume ); // any nonzero integer
   EXPECT_TRUE( each.comparison_section() );

   // A missing triple or model has no entries; a value that is not an
   // integer counts as 0.
   EXPECT_FALSE( mask_rows( {} ).comparison_section() );
   EXPECT_FALSE( mask_rows( {} ).integration_results );
   EXPECT_FALSE( mask_rows( { { "RMSD", "true" } } ).rmsd );
}

// 6.  Rotor stretch:  c0 * speed + c1 * speed^2, the speed not rounded.
TEST( UsCalcLoadingVolume, RotorStretch )
{
   EXPECT_EQ( rotor_stretch( 0.5, 0.25, 0.0 ), 0.0 );
   EXPECT_EQ( rotor_stretch( 0.5, 0.25, 2.0 ), 2.0 );
   EXPECT_EQ( rotor_stretch( 0.5, 0.25, 1.5 ), 1.3125 ); // 1 rpm rounded down would give 0.75
}

// 6.  Loading volume of a sector:  expected values computed independently
// in exact arithmetic.
TEST( UsCalcLoadingVolume, SectorVolume )
{
   // bottom 7.2 cm, meniscus 5.9 cm, pathlength 1.2 cm, angle 2.5 degrees
   EXPECT_NEAR( loading_volume( 7.2, 0.0, 5.9, 1.2, 2.5 ), 445.84435742195149, 1e-12 * 446.0 );
   // the bottom stretched by 0.05 cm
   EXPECT_NEAR( loading_volume( 7.2, 0.05, 5.9, 1.2, 2.5 ), 464.75936319044004, 1e-12 * 465.0 );
   // meniscus at the stretched bottom:  no volume; beyond it:  negative
   EXPECT_EQ( loading_volume( 7.2, 0.05, 7.25, 1.2, 2.5 ), 0.0 );
   EXPECT_LT( loading_volume( 7.2, 0.0, 7.3, 1.2, 2.5 ), 0.0 );
   // zero pathlength or angle:  no volume
   EXPECT_EQ( loading_volume( 7.2, 0.0, 5.9, 0.0, 2.5 ), 0.0 );
   EXPECT_EQ( loading_volume( 7.2, 0.0, 5.9, 1.2, 0.0 ), 0.0 );
}

// 1d.  Custom-grid (-CG) models:  an item applies when every part of its
// method is in the analysis label, so a 2DSA-IT item also applies to a
// 2DSA-CG-IT model.
TEST( UsCalcReportItem, CustomGridModels )
{
   const QString cg    = "fixA.1A260.ee1_a2611090000_2DSA-CG_000001_i01.model";
   const QString cg_it = "fixA.1A260.ee1_a2611090000_2DSA-CG-IT_000001_i01.model";
   const QString cg_mc = "fixA.1A260.ee1_a2611090000_2DSA-CG-MC_000001_mcN003.model";
   const QString it    = "fixA.1A260.ee1_a2611090000_2DSA-IT_000001_i01.model";

   EXPECT_TRUE( method_matches( "2DSA-CG-IT", cg_it ) );
   EXPECT_TRUE( method_matches( "2DSA-IT", cg_it ) );
   EXPECT_TRUE( method_matches( "2DSA", cg ) );
   EXPECT_TRUE( method_matches( "2DSA-CG", cg_mc ) );
   EXPECT_FALSE( method_matches( "2DSA-IT", cg ) );    // no IT part
   EXPECT_FALSE( method_matches( "2DSA-MC", cg_it ) ); // no MC part
   EXPECT_FALSE( method_matches( "2DSA-CG", it ) );    // no CG part
   EXPECT_TRUE( any_method_matches( { "2DSA-MC", "2DSA-IT" }, cg_it ) );
   EXPECT_FALSE( any_method_matches( { "2DSA-MC", "2DSA-CG-MC" }, cg_it ) );
}

// 3a, 5a.  Channel lookups match by substring, and the first channel listed
// wins, even when another channel's name contains it.  Rotors have at most
// 8 cells, so names like "11A" do not occur; this records the rule.
TEST( UsCalcReportLookup, NestedChannelNames )
{
   // A triple matches the first listed channel whose name it contains.
   const QStringList short_first = { "1A:UV/vis.:x", "11A:UV/vis.:y" };
   EXPECT_EQ( report_channel_index( short_first, "11A260", false, false ), 0 ); // "1A" is in "11A260"
   EXPECT_EQ( report_channel_index( short_first, "1A260", false, false ), 0 );
   const QStringList long_first = { "11A:UV/vis.:y", "1A:UV/vis.:x" };
   EXPECT_EQ( report_channel_index( long_first, "11A260", false, false ), 0 );
   EXPECT_EQ( report_channel_index( long_first, "1A260", false, false ), 1 );

   // A replicate member's channel matches the first description containing it.
   EXPECT_EQ( replicate_channel_index( long_first, "1A:UV/vis." ), 0 ); // "1A:UV/vis." is in "11A:UV/vis.:y"
   EXPECT_EQ( replicate_channel_index( short_first, "1A:UV/vis." ), 0 );
   EXPECT_EQ( replicate_channel_index( short_first, "11A:UV/vis." ), 1 );
}
