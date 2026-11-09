//! \file test_us_calc_units.cpp
// Units and quantity access of the calculation library.

#include <gtest/gtest.h>

#include <cmath>

#include "us_calc_units.h"

using namespace US_Calc;

// The GMP reporter scales range bounds with pow(10,-13), pow(10,-7) and
// pow(10,3) (distrib_info) and with literals (velmwl_range_fraction).  The
// library must give the same doubles, or components on a bound would be
// counted differently.
TEST( UsCalcUnits, ScalingMatchesTheReporterExactly )
{
   volatile int e13 = -13, e7 = -7, e3 = 3; // keep pow from being folded
   for ( int i = 1; i <= 100000; ++i )
   {
      double low = i / 1000.0;
      ASSERT_EQ( to_model_units( Quantity::Sedimentation, low ), low * std::pow( 10, e13 ) ) << low;
      ASSERT_EQ( to_model_units( Quantity::Diffusion, low ), low * std::pow( 10, e7 ) ) << low;
      ASSERT_EQ( to_model_units( Quantity::MolecularWeight, low ), low * std::pow( 10, e3 ) ) << low;
      ASSERT_EQ( to_model_units( Quantity::FrictionalRatio, low ), low ) << low;
   }
}

TEST( UsCalcUnits, ReportScales )
{
   EXPECT_EQ( report_scale( Quantity::Sedimentation ), 1.0e-13 );
   EXPECT_EQ( report_scale( Quantity::Diffusion ), 1.0e-7 );
   EXPECT_EQ( report_scale( Quantity::MolecularWeight ), 1.0e3 );
   EXPECT_EQ( report_scale( Quantity::FrictionalRatio ), 1.0 );
   EXPECT_EQ( report_scale( Quantity::PartialSpecificVolume ), 1.0 );
}

TEST( UsCalcUnits, ComponentValueReadsEachQuantity )
{
   Component c;
   c.s                    = 4.5e-13;
   c.D                    = 5.0e-7;
   c.mw                   = 75000.0;
   c.f_f0                 = 1.6;
   c.vbar20               = 0.72;
   c.signal_concentration = 0.2;
   EXPECT_EQ( component_value( c, Quantity::Sedimentation ), 4.5e-13 );
   EXPECT_EQ( component_value( c, Quantity::Diffusion ), 5.0e-7 );
   EXPECT_EQ( component_value( c, Quantity::MolecularWeight ), 75000.0 );
   EXPECT_EQ( component_value( c, Quantity::FrictionalRatio ), 1.6 );
   EXPECT_EQ( component_value( c, Quantity::PartialSpecificVolume ), 0.72 );
}

// autoflowReportItem.type values of the reporter's range items.
TEST( UsCalcUnits, ReportTypesMapToQuantities )
{
   EXPECT_EQ( quantity_from_report_type( "s" ), Quantity::Sedimentation );
   EXPECT_EQ( quantity_from_report_type( "D" ), Quantity::Diffusion );
   EXPECT_EQ( quantity_from_report_type( "MW" ), Quantity::MolecularWeight );
   EXPECT_EQ( quantity_from_report_type( "f/f0" ), Quantity::FrictionalRatio );
   // distrib_info selects nothing for other types; they are not range types
   EXPECT_FALSE( quantity_from_report_type( "vbar" ).has_value() );
   EXPECT_FALSE( quantity_from_report_type( "S" ).has_value() );
   EXPECT_FALSE( quantity_from_report_type( "" ).has_value() );
}

TEST( UsCalcUnits, QuantityNamesRoundTrip )
{
   for ( Quantity q : { Quantity::Sedimentation, Quantity::Diffusion, Quantity::MolecularWeight, Quantity::FrictionalRatio } )
   {
      EXPECT_EQ( quantity_from_report_type( quantity_name( q ) ), q );
   }
   EXPECT_EQ( quantity_name( Quantity::PartialSpecificVolume ), "vbar" );
}
