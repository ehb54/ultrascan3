//! \file us_calc_matching.cpp
#include "us_calc_matching.h"
#include "us_calc_fp.h" // first:  floating-point policy

#include <QStringList>

namespace US_Calc
{
QString analysis_label( const QString& description )
{
   QString label = description.section( '.', -2, -2 ).section( '_', 1, -1 );
   if ( label.isEmpty() )
   {
      label = description.section( '.', 0, -2 );
   }
   return label;
}

bool method_matches( const QString& item_method, const QString& model_description )
{
   const QString method = item_method.contains( "PCSA" ) ? QString( "PCSA" ) : item_method;
   const QString label  = analysis_label( model_description );
   for ( const QString& part : method.split( '-' ) )
   {
      if ( !label.contains( part ) )
      {
         return false;
      }
   }
   return true;
}
} // namespace US_Calc
