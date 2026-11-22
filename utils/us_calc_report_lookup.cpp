//! \file us_calc_report_lookup.cpp
#include "us_calc_report_lookup.h"
#include "us_calc_fp.h" // floating-point policy

#include "us_calc_matching.h"

#include <QtNumeric>

namespace US_Calc::Gmp
{
ModelDescriptionParts parse_model_description( const QString& description )
{
   ModelDescriptionParts parts;
   const QStringList     fields = description.split( "." );
   if ( fields.size() < 2 )
   {
      return parts;
   }
   const QString     analysis = analysis_label( description );
   const QStringList names    = analysis.split( "_" );
   if ( names.size() < 2 )
   {
      return parts;
   }
   parts.ok         = true;
   parts.triple     = fields[1];
   parts.cell       = parts.triple.mid( 0, 1 );
   parts.wavelength = parts.triple.mid( 2, 3 );
   parts.analysis   = analysis;
   parts.model_name = names[1];
   if ( parts.model_name.contains( "PCSA" ) )
   {
      parts.model_name = "PCSA";
   }
   parts.model_name.replace( "-CG", "" );
   return parts;
}

int report_channel_index( const QStringList& channel_descriptions,
                          const QString&     triple,
                          bool               interference_triple,
                          bool               simulated_absorbance )
{
   for ( int i = 0; i < channel_descriptions.size(); ++i )
   {
      const QString& description = channel_descriptions[i];
      if ( interference_triple && !description.contains( "Interf" ) )
      {
         continue;
      }
      QString channel = description.split( ":" )[0];
      if ( simulated_absorbance )
      {
         channel.replace( "A", "S" );
      }
      if ( triple.contains( channel ) )
      {
         return i;
      }
   }
   return -1;
}

QString intensity_triple( const QString& triple, bool simulated_absorbance )
{
   QString name = triple;
   if ( simulated_absorbance )
   {
      name.replace( "S", "A" );
   }
   return name;
}

double triple_intensity( const QMap<QString, QString>& intensities, const QString& triple )
{
   if ( intensities.size() == 1 )
   {
      return intensities.first().toDouble();
   }
   for ( auto it = intensities.begin(); it != intensities.end(); ++it )
   {
      if ( triple.contains( it.key() ) )
      {
         return it.value().toDouble();
      }
   }
   return qQNaN();
}

QString mask_triple( const QString& triple, bool interference_triple )
{
   QString name = triple;
   if ( interference_triple )
   {
      name.replace( "660", "Interference" );
   }
   return name;
}

MaskRows mask_rows( const QMap<QString, QString>& model_parts )
{
   MaskRows rows;
   for ( auto it = model_parts.begin(); it != model_parts.end(); ++it )
   {
      if ( !it.value().toInt() )
      {
         continue;
      }
      const QString& name = it.key();
      rows.total_concentration |= name.contains( "Concentration" );
      rows.rmsd |= name.contains( "RMSD" );
      rows.experiment_duration |= name.contains( "Duration" );
      rows.minimum_intensity |= name.contains( "Intensity" );
      rows.loading_volume |= name.contains( "Loading" );
      rows.integration_results |= name.contains( "Integration" );
   }
   return rows;
}
} // namespace US_Calc::Gmp
