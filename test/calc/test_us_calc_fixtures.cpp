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

#include <cmath>
#include <map>
#include <string>

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
         // The legacy double arithmetic in the legacy order (gap 12)
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
// extraction (fixtures/expected/legacy_smoothing.json), exactly.
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
         EXPECT_EQ( values[i], expected[i].toDouble() ) << "index " << i;
      }
   }
}
