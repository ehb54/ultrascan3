//! \file test_us_calc_version.cpp
// Library and algorithm versions of the calculation library.

#include <gtest/gtest.h>

#include <regex>
#include <set>

#include "us_calc_version.h"

using namespace US_Calc;

TEST( UsCalcVersion, LibraryVersionIsSemantic )
{
   EXPECT_TRUE( std::regex_match( library_version(), std::regex( "[0-9]+\\.[0-9]+\\.[0-9]+" ) ) );
}

TEST( UsCalcVersion, AlgorithmsAreNamedVersionedAndDescribed )
{
   std::set<std::string> names;
   for ( const AlgorithmVersion& a : algorithms() )
   {
      EXPECT_FALSE( a.name.empty() );
      EXPECT_GE( a.version, 1 ) << a.name;
      EXPECT_FALSE( a.description.empty() ) << a.name;
      EXPECT_TRUE( names.insert( a.name ).second ) << "duplicate " << a.name;
   }
   // The calculations in the extraction scope
   for ( const char* name : { "range_summary",
                              "replicate_summary",
                              "tolerance",
                              "mc_composite",
                              "integral_distribution",
                              "gaussian_smoothing",
                              "solute_statistics",
                              "report_item_matching" } )
   {
      EXPECT_TRUE( names.count( name ) ) << name;
   }
}

TEST( UsCalcVersion, LookupByName )
{
   auto a = algorithm( "range_summary" );
   ASSERT_TRUE( a.has_value() );
   EXPECT_EQ( a->name, "range_summary" );
   EXPECT_FALSE( algorithm( "no_such_algorithm" ).has_value() );
}
