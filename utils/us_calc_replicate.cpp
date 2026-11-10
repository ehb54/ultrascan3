//! \file us_calc_replicate.cpp
#include "us_calc_replicate.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include "us_calc_version.h"

#include <cmath>

namespace US_Calc
{
ReplicateResult replicate_summary( const QVector<RangeResult>& members )
{
   ReplicateResult result;
   result.member_count = members.size();
   result.algorithm    = *algorithm( "replicate_summary" );

   QVector<double> rs;
   QVector<double> fs;
   for ( int i = 0; i < members.size(); ++i )
   {
      const RangeResult& m = members[i];
      if ( m.range_concentration < 0.0 )
      {
         result.diagnostics.push_back( { Issue::NegativeConcentration,
                                         Severity::Warning,
                                         QString( "member %1 has a negative in-range concentration and is not used" ).arg( i ) } );
         continue;
      }
      if ( !m.fraction_percent )
      {
         result.diagnostics.push_back(
            { Issue::ZeroTotal, Severity::Warning, QString( "member %1 has no fraction and is not used" ).arg( i ) } );
         continue;
      }
      rs.push_back( m.range_concentration );
      fs.push_back( *m.fraction_percent );
   }
   result.used_count = rs.size();
   if ( rs.isEmpty() )
   {
      result.diagnostics.push_back( { Issue::NoEligibleMembers, Severity::Error, "no member of the group can be used" } );
      return result;
   }

   // As get_replicate_group_results:  sums in member order, then
   // sd = sqrt( sum of squared deviations ) / sqrt( n ).
   const double n     = static_cast<double>( rs.size() );
   double       r_sum = 0.0;
   double       f_sum = 0.0;
   for ( int i = 0; i < rs.size(); ++i )
   {
      r_sum += rs[i];
      f_sum += fs[i];
   }
   const double r_mean = r_sum / n;
   const double f_mean = f_sum / n;
   double       r_dev  = 0.0;
   double       f_dev  = 0.0;
   for ( int i = 0; i < rs.size(); ++i )
   {
      r_dev += ( r_mean - rs[i] ) * ( r_mean - rs[i] );
      f_dev += ( f_mean - fs[i] ) * ( f_mean - fs[i] );
   }
   result.mean_range_concentration = r_mean;
   result.mean_fraction_percent    = f_mean;
   result.sd_range_concentration   = std::sqrt( r_dev ) / std::sqrt( n );
   result.sd_fraction_percent      = std::sqrt( f_dev ) / std::sqrt( n );
   return result;
}
} // namespace US_Calc
