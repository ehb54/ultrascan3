//! \file test_us_calc_version.cpp
// Library and algorithm versions of the calculation library.

#include <gtest/gtest.h>

#include <QRegularExpression>
#include <QSet>

#include "us_calc_test_print.h"
#include "us_calc_version.h"

using namespace US_Calc;

TEST( UsCalcVersion, LibraryVersionIsSemantic )
{
   EXPECT_TRUE( QRegularExpression( "^[0-9]+\\.[0-9]+\\.[0-9]+$" ).match( library_version() ).hasMatch() );
}

TEST( UsCalcVersion, AlgorithmsAreNamedVersionedAndDescribed )
{
   QSet<QString> names;
   for ( const AlgorithmVersion& a : algorithms() )
   {
      EXPECT_FALSE( a.name.isEmpty() );
      EXPECT_GE( a.version, 1 ) << a.name;
      EXPECT_FALSE( a.description.isEmpty() ) << a.name;
      EXPECT_FALSE( names.contains( a.name ) ) << "duplicate " << a.name;
      names.insert( a.name );
   }
   // The calculations the library provides
   for ( const char* name : { "range_summary",
                              "replicate_summary",
                              "tolerance",
                              "mc_composite",
                              "integral_distribution",
                              "gaussian_smoothing",
                              "weighted_statistics",
                              "report_item_matching" } )
   {
      EXPECT_TRUE( names.contains( name ) ) << name;
   }
}

TEST( UsCalcVersion, LookupByName )
{
   auto a = algorithm( "range_summary" );
   ASSERT_TRUE( a.has_value() );
   EXPECT_EQ( a->name, "range_summary" );
   EXPECT_FALSE( algorithm( "no_such_algorithm" ).has_value() );
}
