//! \file us_calc_replicate_group.cpp
#include "us_calc_replicate_group.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_calc_tolerance.h"
#include "us_calc_version.h"

#include <QMap>

namespace US_Calc::Gmp
{
bool same_replicate_item( const ReplicateItem& reference, const ReplicateItem& item )
{
   return reference.type == item.type && reference.method == item.method && reference.low == item.low &&
          reference.high == item.high &&
          QString::number( reference.integration_value ) == QString::number( item.integration_value );
}

int replicate_channel_index( const QStringList& channel_descriptions, const QString& channel )
{
   for ( int i = 0; i < channel_descriptions.size(); ++i )
   {
      if ( channel_descriptions[i].contains( channel ) )
      {
         return i;
      }
   }
   return -1;
}

bool replicate_triple_included( const QString& wavelength, const QString& wavelengths_not_run, bool integration_results_shown )
{
   return !wavelengths_not_run.split( ":" ).contains( wavelength ) && integration_results_shown;
}

ReplicateLayout replicate_layout( const QStringList& triples, const QVector<bool>& included )
{
   ReplicateLayout            layout;
   QStringList                wavelengths;
   QMap<QString, QStringList> channels_of;
   for ( int i = 0; i < triples.size(); ++i )
   {
      if ( i >= included.size() || !included[i] )
      {
         continue;
      }
      const QString channel    = triples[i].split( "." )[0];
      const QString wavelength = triples[i].split( "." )[1];
      wavelengths << wavelength;
      layout.channels << channel;
      channels_of[wavelength] << channel;
   }
   wavelengths.removeDuplicates();
   layout.channels.removeDuplicates();
   for ( const QString& wavelength : wavelengths )
   {
      layout.subgroups.push_back( { wavelength, channels_of[wavelength] } );
   }
   return layout;
}

ReplicateGroupResult evaluate_replicate_group( const ReplicateItem&                   reference,
                                               const QVector<QVector<ReplicateItem>>& channel_items,
                                               int                                    listed_triples )
{
   ReplicateGroupResult result;
   result.algorithm = algorithm( "report_replicate_group" );

   QVector<RangeResult> members;
   for ( const QVector<ReplicateItem>& items : channel_items )
   {
      for ( const ReplicateItem& item : items )
      {
         if ( same_replicate_item( reference, item ) && item.range_concentration >= 0 )
         {
            RangeResult member;
            member.range_concentration = item.range_concentration;
            member.fraction_percent    = item.fraction_percent;
            members.push_back( member );
         }
      }
   }

   result.summary = replicate_summary( members );
   if ( result.summary.used_count > 0 )
   {
      result.mean_range_concentration = result.summary.mean_range_concentration;
      result.sd_range_concentration   = result.summary.sd_range_concentration;
      result.mean_fraction_percent    = result.summary.mean_fraction_percent;
      result.sd_fraction_percent      = result.summary.sd_fraction_percent;
   }
   result.show_sd = listed_triples > 1;
   result.passed  = passes( result.mean_fraction_percent,
                            { ToleranceRule::AbsolutePercentagePoints, reference.target_percent, reference.tolerance } );
   return result;
}
} // namespace US_Calc::Gmp
