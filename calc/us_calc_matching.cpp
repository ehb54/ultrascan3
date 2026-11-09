//! \file us_calc_matching.cpp
#include "us_calc_matching.h"

#include <vector>

namespace US_Calc
{
namespace
{
std::vector<std::string> split( const std::string& text, char separator )
{
   // Like QString::split with empty parts kept.
   std::vector<std::string> parts;
   std::string::size_type   from = 0;
   while ( true )
   {
      std::string::size_type at = text.find( separator, from );
      if ( at == std::string::npos )
      {
         parts.push_back( text.substr( from ) );
         return parts;
      }
      parts.push_back( text.substr( from, at - from ) );
      from = at + 1;
   }
}
} // namespace

std::string text_section( const std::string& text, char separator, int start, int end )
{
   const std::vector<std::string> fields = split( text, separator );
   const int                      n      = static_cast<int>( fields.size() );
   if ( start < 0 )
   {
      start += n;
   }
   if ( end < 0 )
   {
      end += n;
   }
   if ( start >= n || end < 0 || start > end )
   {
      return "";
   }
   if ( start < 0 )
   {
      start = 0;
   }
   if ( end >= n )
   {
      end = n - 1;
   }
   std::string out;
   for ( int i = start; i <= end; ++i )
   {
      if ( i > start )
      {
         out += separator;
      }
      out += fields[i];
   }
   return out;
}

std::string analysis_label( const std::string& description )
{
   std::string label = text_section( text_section( description, '.', -2, -2 ), '_', 1, -1 );
   if ( label.empty() )
   {
      label = text_section( description, '.', 0, -2 );
   }
   return label;
}

bool method_matches( const std::string& item_method, const std::string& model_description )
{
   const std::string method = item_method.find( "PCSA" ) != std::string::npos ? "PCSA" : item_method;
   const std::string label  = analysis_label( model_description );
   for ( const std::string& part : split( method, '-' ) )
   {
      if ( label.find( part ) == std::string::npos )
      {
         return false;
      }
   }
   return true;
}
} // namespace US_Calc
