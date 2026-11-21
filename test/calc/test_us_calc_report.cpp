//! \file test_us_calc_report.cpp
// The GMP reporter's rules in US_Calc, with hand-checked values at their
// edges:  report items on a model.

#include <gtest/gtest.h>

#include <cmath>

#include <QStringList>
#include <QtNumeric>

#include "us_calc_report_item.h"
#include "us_calc_test_print.h"
#include "us_calc_tolerance.h"
#include "us_calc_units.h"

using namespace US_Calc;

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
