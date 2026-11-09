//! \file test_us_calc_fixtures.cpp
// The us_calc reporting calculations against the calculation fixtures.
//
// fixtures/expected/reporting.json holds every fixture model as the
// calculations receive it and the expected results from calc_oracle.py,
// which are exact for those inputs.  The tolerances come from
// fixtures/expected/tolerances.json.  Qt Core is used here only to read the
// JSON files; the library itself has no Qt dependency.

#include <gtest/gtest.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <string>

#include "us_calc_composite.h"
#include "us_calc_integral.h"
#include "us_calc_matching.h"
#include "us_calc_range.h"
#include "us_calc_replicate.h"
#include "us_calc_smoothing.h"
#include "us_calc_tolerance.h"
#include "us_calc_units.h"

using namespace US_Calc;

namespace
{
QJsonObject readJson( const QString& name )
{
   QFile file( QStringLiteral( CALC_FIXTURE_DIR ) + "/" + name );
   if ( !file.open( QIODevice::ReadOnly ) )
   {
      ADD_FAILURE() << "cannot read " << name.toStdString();
      return QJsonObject();
   }
   return QJsonDocument::fromJson( file.readAll() ).object();
}

struct Tolerances
{
      double sums_rel;
      double fraction_abs;
      double means_rel;
      double sd_abs_of_mean;
};

Tolerances tolerances()
{
   const QJsonObject t = readJson( "expected/tolerances.json" );
   return { t["sums"].toObject()["relative"].toDouble(),
            t["fractions_percent"].toObject()["absolute_percentage_points"].toDouble(),
            t["weighted_means"].toObject()["relative"].toDouble(),
            t["standard_deviations"].toObject()["absolute_relative_to_mean"].toDouble() };
}

bool nearRel( double actual, double expected, double rel )
{
   return std::fabs( actual - expected ) <= rel * std::fabs( expected ) + 1e-300;
}

Model toModel( const QJsonObject& m )
{
   Model model;
   model.description = m["description"].toString().toStdString();
   for ( const QJsonValue& v : m["components"].toArray() )
   {
      const QJsonArray a = v.toArray();
      Component        c;
      c.s                    = a[0].toDouble();
      c.D                    = a[1].toDouble();
      c.mw                   = a[2].toDouble();
      c.f                    = a[3].toDouble();
      c.f_f0                 = a[4].toDouble();
      c.vbar20               = a[5].toDouble();
      c.signal_concentration = a[6].toDouble();
      model.components.push_back( c );
   }
   return model;
}

// The report item a range result refers to.
QJsonObject itemFor( const QJsonObject& items, const QJsonObject& result )
{
   const QString sample = result["sample"].toString();
   const int     index  = result["item"].toInt();
   if ( sample == "edge" )
   {
      return items["edge"].toArray()[index].toObject();
   }
   return items["samples"].toObject()[sample].toObject()["items"].toArray()[index].toObject();
}

Range toRange( const QJsonObject& item )
{
   const std::string type = item["type"].toString().toStdString();
   Range             range;
   auto              q = quantity_from_report_type( type );
   range.quantity      = q ? *q : Quantity::PartialSpecificVolume; // "vbar":  not a range type
   range.low           = item["low"].toDouble();
   range.high          = item["high"].toDouble();
   return range;
}

bool hasIssue( const std::vector<Diagnostic>& d, Issue issue )
{
   for ( const Diagnostic& x : d )
   {
      if ( x.issue == issue )
      {
         return true;
      }
   }
   return false;
}

class UsCalcFixtures : public ::testing::Test
{
   protected:
      void SetUp() override
      {
         expected = readJson( "expected/reporting.json" );
         items    = readJson( "report_items.json" );
         tol      = tolerances();
         ASSERT_FALSE( expected.isEmpty() );
         ASSERT_FALSE( items.isEmpty() );
         const QJsonObject models = expected["models"].toObject();
         for ( auto it = models.begin(); it != models.end(); ++it )
         {
            inputs[it.key().toStdString()] = toModel( it.value().toObject() );
         }
      }

      QJsonObject                  expected;
      QJsonObject                  items;
      Tolerances                   tol{};
      std::map<std::string, Model> inputs;
};
} // namespace

TEST_F( UsCalcFixtures, RangeSummariesMatchTheOracle )
{
   int checked = 0;
   for ( const QJsonValue& v : expected["ranges"].toArray() )
   {
      const QJsonObject e    = v.toObject();
      const std::string key  = e["model"].toString().toStdString();
      const QJsonObject item = itemFor( items, e );
      SCOPED_TRACE( key + " item " + std::to_string( e["item"].toInt() ) + " (" + e["sample"].toString().toStdString() + ")" );
      const Model& model = inputs.at( key );

      EXPECT_EQ( method_matches( item["method"].toString().toStdString(), model.description ), e["matched"].toBool() );

      const RangeResult r = range_summary( model, toRange( item ) );
      if ( !e["supported"].toBool() )
      {
         EXPECT_TRUE( hasIssue( r.diagnostics, Issue::UnsupportedQuantity ) );
         EXPECT_FALSE( r.fraction_percent.has_value() );
         ++checked;
         continue;
      }

      EXPECT_EQ( static_cast<int>( r.selected_count ), e["selected"].toInt() );
      EXPECT_TRUE( nearRel( r.total_concentration, e["C"].toDouble(), tol.sums_rel ) );
      EXPECT_TRUE( nearRel( r.range_concentration, e["R"].toDouble(), tol.sums_rel ) );

      if ( e["fraction"].isNull() )
      {
         EXPECT_FALSE( r.fraction_percent.has_value() );
         EXPECT_TRUE( hasIssue( r.diagnostics, Issue::ZeroTotal ) || hasIssue( r.diagnostics, Issue::EmptyModel ) );
      }
      else
      {
         ASSERT_TRUE( r.fraction_percent.has_value() );
         EXPECT_LE( std::fabs( *r.fraction_percent - e["fraction"].toDouble() ), tol.fraction_abs );
         // The legacy double arithmetic in the legacy order
         EXPECT_EQ( *r.fraction_percent, e["fraction_double"].toDouble() );
         if ( e.contains( "pass_pp" ) && !e["pass_pp"].isNull() )
         {
            const ToleranceCheck check{ ToleranceRule::AbsolutePercentagePoints,
                                        item["target"].toDouble(),
                                        item["tolerance"].toDouble() };
            EXPECT_EQ( passes( *r.fraction_percent, check ), e["pass_pp"].toBool() );
         }
      }

      if ( e["within_mean"].isNull() )
      {
         EXPECT_FALSE( r.within_range_mean.has_value() );
      }
      else
      {
         ASSERT_TRUE( r.within_range_mean.has_value() );
         EXPECT_TRUE( nearRel( *r.within_range_mean, e["within_mean"].toDouble(), tol.means_rel ) );
      }
      ++checked;
   }
   EXPECT_EQ( checked, expected["ranges"].toArray().size() );
}

TEST_F( UsCalcFixtures, WholeModelMeansMatchTheOracle )
{
   const QJsonObject models = expected["models"].toObject();
   for ( auto it = models.begin(); it != models.end(); ++it )
   {
      SCOPED_TRACE( it.key().toStdString() );
      const QJsonObject e = it.value().toObject();
      const ModelMeans  m = model_means( inputs.at( it.key().toStdString() ) );
      EXPECT_EQ( static_cast<int>( m.component_count ), e["ncomp"].toInt() );
      EXPECT_TRUE( nearRel( m.total_concentration, e["C"].toDouble(), tol.sums_rel ) );
      const std::pair<const char*, std::optional<double>> means[] = {
         { "mean_s", m.s }, { "mean_D", m.D }, { "mean_mw", m.mw }, { "mean_f_f0", m.f_f0 }, { "mean_vbar20", m.vbar20 }
      };
      for ( const auto& mean : means )
      {
         if ( e[mean.first].isNull() )
         {
            EXPECT_FALSE( mean.second.has_value() ) << mean.first;
         }
         else
         {
            ASSERT_TRUE( mean.second.has_value() ) << mean.first;
            EXPECT_TRUE( nearRel( *mean.second, e[mean.first].toDouble(), tol.means_rel ) ) << mean.first;
         }
      }
   }
}

TEST_F( UsCalcFixtures, TotalConcentrationChecksMatchTheOracle )
{
   for ( const QJsonValue& v : expected["total_concentration"].toArray() )
   {
      const QJsonObject e = v.toObject();
      SCOPED_TRACE( e["model"].toString().toStdString() );
      const double         c = model_means( inputs.at( e["model"].toString().toStdString() ) ).total_concentration;
      const ToleranceCheck check{ ToleranceRule::RelativePercentOfTarget, e["target"].toDouble(), e["tolerance"].toDouble() };
      EXPECT_EQ( passes( c, check ), e["pass_relative"].toBool() );
   }
}

TEST_F( UsCalcFixtures, ReplicateGroupsMatchTheOracle )
{
   const QJsonArray groups = items["replicate_groups"].toArray();
   const QJsonArray exp    = expected["replicates"].toArray();
   ASSERT_EQ( groups.size(), exp.size() );
   const QJsonObject models = expected["models"].toObject();
   for ( int g = 0; g < groups.size(); ++g )
   {
      SCOPED_TRACE( "group " + std::to_string( g ) );
      const QJsonObject group = groups[g].toObject();
      const QJsonObject e     = exp[g].toObject();
      const QJsonObject item  = group["item"].toObject();
      QStringList       cases;
      for ( const QJsonValue& c : group["cases"].toArray() )
      {
         cases << c.toString();
      }
      // The oracle uses the reporter's representation of each member, in
      // sorted key order.
      std::vector<RangeResult> members;
      for ( auto it = models.begin(); it != models.end(); ++it )
      {
         if ( cases.contains( it.key().section( '/', 0, 0 ) ) && it.value().toObject()["reporter"].toBool() )
         {
            members.push_back( range_summary( inputs.at( it.key().toStdString() ), toRange( item ) ) );
         }
      }
      const ReplicateResult r = replicate_summary( members );
      ASSERT_EQ( static_cast<int>( r.used_count ), e["n"].toInt() );
      ASSERT_TRUE( r.mean_fraction_percent.has_value() );
      EXPECT_TRUE( nearRel( *r.mean_range_concentration, e["R_mean"].toDouble(), tol.sums_rel ) );
      EXPECT_LE( std::fabs( *r.mean_fraction_percent - e["fraction_mean"].toDouble() ), tol.fraction_abs );
      EXPECT_LE( std::fabs( *r.sd_range_concentration - e["R_sd"].toDouble() ),
                 tol.sd_abs_of_mean * std::fabs( e["R_mean"].toDouble() ) );
      EXPECT_LE( std::fabs( *r.sd_fraction_percent - e["fraction_sd"].toDouble() ),
                 tol.sd_abs_of_mean * std::fabs( e["fraction_mean"].toDouble() ) );
      const ToleranceCheck check{ ToleranceRule::RelativePercentOfTarget, item["target"].toDouble(), item["tolerance"].toDouble() };
      EXPECT_EQ( passes( *r.mean_fraction_percent, check ), e["pass_relative"].toBool() );
   }
}

// The library reproduces the smoothing captured from US_Math2 before the
// library replaced it (fixtures/expected/legacy_smoothing.json) within the legacy
// series tolerance:  the capture was made by a build that fused multiply-adds,
// which us_calc does not (us_calc_fp.h).
TEST( UsCalcLegacyCaptures, SmoothingMatchesTheCapture )
{
   const QJsonObject captured = readJson( "expected/legacy_smoothing.json" );
   ASSERT_FALSE( captured.isEmpty() );
   for ( auto it = captured.begin(); it != captured.end(); ++it )
   {
      SCOPED_TRACE( it.key().toStdString() );
      // Keys are n<points>_smooth<width>; the series is (i*i % 7) + 0.25*i.
      const int           n      = it.key().section( '_', 0, 0 ).mid( 1 ).toInt();
      const int           smooth = it.key().section( '_', 1, 1 ).mid( 6 ).toInt();
      std::vector<double> values;
      for ( int i = 0; i < n; ++i )
      {
         values.push_back( ( i * i % 7 ) + 0.25 * i );
      }
      EXPECT_TRUE( gaussian_smoothing( values, smooth ).empty() );
      const QJsonArray expected = it.value().toArray();
      ASSERT_EQ( static_cast<int>( values.size() ), expected.size() );
      for ( int i = 0; i < expected.size(); ++i )
      {
         EXPECT_NEAR( values[i], expected[i].toDouble(), 1e-12 * std::fabs( expected[i].toDouble() ) + 1e-300 ) << "index " << i;
      }
   }
}

// The composite key must be the same text QString::asprintf makes in
// US_Model, including at the rounding points of both fields.
TEST( UsCalcComposite, KeyMatchesQStringAsprintf )
{
   std::mt19937_64                        rng( 4711 );
   std::uniform_real_distribution<double> s_dist( 0.1, 20.0 ), k_dist( 1.0, 4.0 );
   int                                    mismatches = 0;
   for ( int i = 0; i < 1000000; ++i )
   {
      double s = s_dist( rng ) * 1e-13;
      double k = k_dist( rng );
      if ( i % 4 == 1 )
      {
         k = std::round( k * 1e5 ) / 1e5 + 0.5e-5; // on a 5th-decimal rounding point
      }
      if ( i % 4 == 2 )
      {
         s = ( std::round( s * 1e17 ) / 1e4 + 0.5e-4 ) * 1e-13; // on a 4th-decimal point in S
      }
      if ( i % 10 == 3 )
      {
         s = -s; // negative s
      }
      if ( i % 10 == 7 )
      {
         s = ( std::floor( s_dist( rng ) * 16.0 ) / 32.0 ) * 1e-13; // exact binary ties
         k = std::floor( k * 64.0 ) / 64.0 + 1.0 / 128.0 * ( i % 20 == 7 ? -1 : 1 );
      }
      if ( i % 1000 == 9 )
      {
         s = ( i % 2000 == 9 ? -1 : 1 ) * 3.0e-19; // rounds to zero
      }
      const std::string qt = QString::asprintf( "%10.4f %8.5f", s * 1.0e+13, k ).toStdString();
      if ( qt != mc_solute_key( s, k ) && ++mismatches <= 5 )
      {
         ADD_FAILURE() << "s " << s << " k " << k << ": " << qt << " vs " << mc_solute_key( s, k );
      }
   }
   EXPECT_EQ( mismatches, 0 );
}

// Each Monte Carlo fixture's composite equals the one US_Model::load built
// before the library replaced it (checked by calc_oracle.py against legacy_models.json).
TEST_F( UsCalcFixtures, CompositesMatchTheFixtures )
{
   int composites = 0;
   for ( const auto& entry : inputs )
   {
      const std::string& key = entry.first;
      if ( key.size() < 10 || key.compare( key.size() - 10, 10, "#composite" ) != 0 )
      {
         continue;
      }
      SCOPED_TRACE( key );
      const std::string  file = key.substr( 0, key.size() - 10 );
      std::vector<Model> iterations;
      for ( int it = 1; inputs.count( file + "#iter" + std::to_string( it ) ); ++it )
      {
         iterations.push_back( inputs.at( file + "#iter" + std::to_string( it ) ) );
      }
      ASSERT_GE( iterations.size(), 2u );
      const CompositeResult r        = mc_composite( iterations );
      const Model&          expected = entry.second;
      ASSERT_TRUE( r.diagnostics.empty() );
      EXPECT_EQ( r.model.description, expected.description );
      ASSERT_EQ( r.model.components.size(), expected.components.size() );
      for ( std::size_t i = 0; i < expected.components.size(); ++i )
      {
         const Component& a = r.model.components[i];
         const Component& e = expected.components[i];
         EXPECT_EQ( a.s, e.s ) << i;
         EXPECT_EQ( a.D, e.D ) << i;
         EXPECT_EQ( a.mw, e.mw ) << i;
         EXPECT_EQ( a.f, e.f ) << i;
         EXPECT_EQ( a.f_f0, e.f_f0 ) << i;
         EXPECT_EQ( a.vbar20, e.vbar20 ) << i;
         EXPECT_EQ( a.signal_concentration, e.signal_concentration ) << i;
      }
      ++composites;
   }
   EXPECT_EQ( composites, 3 );
}

TEST( UsCalcComposite, InvalidIterations )
{
   EXPECT_TRUE( has_error( mc_composite( {} ).diagnostics ) );
   Model one;
   one.components.push_back( Component{ 4.5e-13, 5e-7, 75000.0, 0.0, 1.6, 0.72, 0.5 } );
   Model empty;
   EXPECT_TRUE( has_error( mc_composite( { one, empty } ).diagnostics ) );
}

// f/f0 varies and vbar is constant:  the key's second field is f/f0, so
// solutes differing only in f/f0 stay apart and equal ones merge.
TEST( UsCalcComposite, MergesByKeyAndAverages )
{
   Model a, b;
   a.description           = "run.1A260.e1_a2611090000_2DSA-MC_000001_mc0001.model";
   a.components            = { Component{ 4.5e-13, 5e-7, 75000.0, 0.0, 1.6, 0.72, 0.4 },
                               Component{ 4.5e-13, 4e-7, 90000.0, 0.0, 1.9, 0.72, 0.2 } };
   b.description           = "run.1A260.e1_a2611090000_2DSA-MC_000001_mc0002.model";
   b.components            = { Component{ 4.5e-13, 5.1e-7, 75001.0, 0.0, 1.6, 0.72, 0.6 } };
   const CompositeResult r = mc_composite( { a, b } );
   ASSERT_TRUE( r.diagnostics.empty() );
   ASSERT_EQ( r.model.components.size(), 2u );
   EXPECT_DOUBLE_EQ( r.model.components[0].signal_concentration, ( 0.4 + 0.6 ) / 2 );
   EXPECT_EQ( r.model.components[0].D, 5.1e-7 ); // from the last match
   EXPECT_EQ( r.source_index[0], 2u );
   EXPECT_DOUBLE_EQ( r.model.components[1].signal_concentration, 0.2 / 2 );
   EXPECT_EQ( r.model.description, "run.1A260.e1_a2611090000_2DSA-MC_000001_mcN002.model" );
}

namespace
{
const std::pair<const char*, DistributionAxis> kAxes[] = { { "s", DistributionAxis::Sedimentation },
                                                           { "ff0", DistributionAxis::FrictionalRatio },
                                                           { "mw", DistributionAxis::MolecularWeight },
                                                           { "D", DistributionAxis::Diffusion },
                                                           { "vbar", DistributionAxis::PartialSpecificVolume },
                                                           { "Rh", DistributionAxis::HydrodynamicRadius } };

const std::vector<double>&                     series( const IntegralDistribution& d, DistributionAxis axis )
{
   switch ( axis )
   {
      case DistributionAxis::Sedimentation:
         return d.s;
      case DistributionAxis::FrictionalRatio:
         return d.f_f0;
      case DistributionAxis::MolecularWeight:
         return d.mw;
      case DistributionAxis::Diffusion:
         return d.D;
      case DistributionAxis::PartialSpecificVolume:
         return d.vbar20;
      case DistributionAxis::HydrodynamicRadius:
         return d.rh;
   }
   return d.s;
}
} // namespace

// Without duplicate solutes the corrected distribution is the one US_Integral
// drew (fixtures/expected/legacy_integral.json), on every axis.
TEST_F( UsCalcFixtures, IntegralDistributionsMatchUsIntegral )
{
   const QJsonObject captured = readJson( "expected/legacy_integral.json" );
   const double      rel      = readJson( "expected/tolerances.json" )["legacy_series"].toObject()["relative"].toDouble();
   int               compared = 0;
   for ( auto it = captured.begin(); it != captured.end(); ++it )
   {
      // Single models only; "overlay/..." entries combine them.
      QString file = it.key();
      if ( file.startsWith( "overlay/" ) || file.contains( "duplicates" ) )
      {
         continue;
      }
      std::string key = file.startsWith( "edge/" ) ? file.toStdString() : file.mid( 7 ).toStdString(); // drop "models/"
      key += inputs.count( key + "#composite" ) ? "#composite" : "#model";
      SCOPED_TRACE( key );
      const Model& model = inputs.at( key );
      for ( const auto& axis : kAxes )
      {
         const QJsonObject curve = it.value().toObject()[axis.first].toArray()[0].toObject();
         const QJsonArray  xs    = curve["x"].toArray();
         const QJsonArray  ys    = curve["y"].toArray();
         const auto        d     = integral_distribution( model, axis.second );
         ASSERT_TRUE( d.diagnostics.empty() || !has_error( d.diagnostics ) ) << axis.first;
         const std::vector<double>& x = series( d, axis.second );
         ASSERT_EQ( static_cast<int>( x.size() ), xs.size() ) << axis.first;
         for ( int i = 0; i < xs.size(); ++i )
         {
            EXPECT_NEAR( x[i], xs[i].toDouble(), rel * std::fabs( xs[i].toDouble() ) + 1e-300 ) << axis.first << " " << i;
            EXPECT_NEAR( d.boundary_fractions[i], ys[i].toDouble(), 1e-15 ) << axis.first << " " << i;
         }
      }
      ++compared;
   }
   EXPECT_EQ( compared, 14 ); // 11 fitted and 3 edge models without duplicates
}

// With duplicates the result is corrected:  the three equal solutes
// at 4.5 S (0.2 + 0.2 + 0.1) and the different one (0.1) give 60 % at 4.5 S
// and the solute at 6.0 S the remaining 40 %, whatever the order.
TEST_F( UsCalcFixtures, IntegralDistributionMergesDuplicates )
{
   const Model& model    = inputs.at( "edge/edge_duplicates_ties.model.xml#model" );
   Model        reversed = model;
   std::reverse( reversed.components.begin(), reversed.components.end() );
   for ( const Model* m : std::vector<const Model*>{ &model, &reversed } )
   {
      const auto d = integral_distribution( *m, DistributionAxis::Sedimentation );
      ASSERT_FALSE( has_error( d.diagnostics ) );
      for ( size_t i = 0; i < d.s.size(); ++i )
      {
         // The level on the boundary itself (0.6) is decided by rounding.
         if ( std::fabs( d.boundary_fractions[i] - 0.6 ) < 1e-9 )
         {
            continue;
         }
         EXPECT_DOUBLE_EQ( d.s[i], d.boundary_fractions[i] < 0.6 ? 4.5 : 6.0 ) << i;
      }
   }
   const auto a = integral_distribution( model, DistributionAxis::Diffusion );
   const auto b = integral_distribution( reversed, DistributionAxis::Diffusion );
   EXPECT_EQ( a.D, b.D );
}

TEST( UsCalcIntegral, InvalidInputs )
{
   EXPECT_TRUE( has_error( integral_distribution( Model(), DistributionAxis::Sedimentation ).diagnostics ) );
   Model zero;
   zero.components = { Component{ 4.5e-13, 0, 0, 0, 1.6, 0.72, 0.0 } };
   EXPECT_TRUE( has_error( integral_distribution( zero, DistributionAxis::Sedimentation ).diagnostics ) );
   Model one;
   one.components = { Component{ 4.5e-13, 0, 0, 0, 1.6, 0.72, 1.0 } };
   IntegralSettings narrow;
   narrow.divisions = 10;
   narrow.smoothing = 10; // 11 levels cannot take a width of 10
   const auto d     = integral_distribution( one, DistributionAxis::Sedimentation, narrow );
   EXPECT_TRUE( has_error( d.diagnostics ) );
   EXPECT_EQ( d.s.size(), 11u ); // unsmoothed series are still returned
}
