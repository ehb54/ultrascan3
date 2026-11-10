//! \file test_us_calc_validation.cpp
// Input checks of the US_Calc calculations.

#include <gtest/gtest.h>

#include <QVector>

#include <limits>

#include "us_calc_validation.h"

using namespace US_Calc;

namespace
{
Component component( double s, double c )
{
   Component x;
   x.s                    = s;
   x.D                    = 5.0e-7;
   x.mw                   = 75000.0;
   x.f_f0                 = 1.6;
   x.vbar20               = 0.72;
   x.signal_concentration = c;
   return x;
}

bool has_issue( const QVector<Diagnostic>& d, Issue issue, Severity severity )
{
   for ( const Diagnostic& x : d )
   {
      if ( x.issue == issue && x.severity == severity && !x.detail.isEmpty() )
      {
         return true;
      }
   }
   return false;
}
} // namespace

TEST( UsCalcValidation, ValidModelHasNoDiagnostics )
{
   Model m;
   m.components = { component( 4.5e-13, 0.4 ), component( 6.0e-13, 0.6 ) };
   EXPECT_TRUE( validate_model( m ).empty() );
}

TEST( UsCalcValidation, EmptyModelIsAnError )
{
   auto d = validate_model( Model() );
   EXPECT_TRUE( has_issue( d, Issue::EmptyModel, Severity::Error ) );
   EXPECT_TRUE( has_error( d ) );
}

TEST( UsCalcValidation, NonFiniteValuesAreErrors )
{
   for ( double bad : { std::numeric_limits<double>::quiet_NaN(),
                        std::numeric_limits<double>::infinity(),
                        -std::numeric_limits<double>::infinity() } )
   {
      Model m;
      m.components = { component( 4.5e-13, 0.4 ), component( bad, 0.6 ) };
      EXPECT_TRUE( has_issue( validate_model( m ), Issue::NonFiniteValue, Severity::Error ) );
      m.components = { component( 4.5e-13, bad ) };
      EXPECT_TRUE( has_issue( validate_model( m ), Issue::NonFiniteValue, Severity::Error ) );
   }
}

// The legacy calculations accept negative concentrations; US_Calc warns.
TEST( UsCalcValidation, NegativeConcentrationIsAWarning )
{
   Model m;
   m.components = { component( 4.5e-13, -0.1 ), component( 7.0e-13, 1.1 ), component( 8.0e-13, -0.2 ) };
   auto d       = validate_model( m );
   EXPECT_TRUE( has_issue( d, Issue::NegativeConcentration, Severity::Warning ) );
   EXPECT_FALSE( has_error( d ) );
   EXPECT_EQ( d.size(), 1 ); // reported once per model
}

TEST( UsCalcValidation, Ranges )
{
   EXPECT_TRUE( validate_range( { Quantity::Sedimentation, 4.0, 5.0 } ).empty() );
   EXPECT_TRUE( validate_range( { Quantity::Sedimentation, 4.5, 4.5 } ).empty() );
   EXPECT_TRUE( has_issue( validate_range( { Quantity::Sedimentation, 5.0, 4.0 } ), Issue::InvalidRange, Severity::Error ) );
   EXPECT_TRUE( has_issue( validate_range( { Quantity::Diffusion, std::numeric_limits<double>::quiet_NaN(), 1.0 } ),
                           Issue::InvalidRange,
                           Severity::Error ) );
}

TEST( UsCalcValidation, HasErrorIgnoresWarnings )
{
   EXPECT_FALSE( has_error( {} ) );
   EXPECT_FALSE( has_error( { { Issue::NegativeConcentration, Severity::Warning, "w" } } ) );
   EXPECT_TRUE(
      has_error( { { Issue::NegativeConcentration, Severity::Warning, "w" }, { Issue::ZeroTotal, Severity::Error, "e" } } ) );
}
