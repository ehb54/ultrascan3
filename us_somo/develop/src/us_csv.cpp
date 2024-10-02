#include "../include/us_csv.h"

QStringList US_Csv::parse_line( QString qs )
{
   QStringList qsl;
   if ( qs.isEmpty() )
   {
      return qsl;
   }
   if ( !qs.contains(",") )
   {
      qsl << qs;
      return qsl;
   }

   QStringList qsl_chars = (qs ).split( "" , Qt::SkipEmptyParts );
   QString token = "";

   bool in_quote = false;

   for ( QStringList::iterator it = qsl_chars.begin();
         it != qsl_chars.end();
         it++ )
   {
      if ( !in_quote && *it == "," )
      {
         qsl << token;
         token = "";
         continue;
      }
      if ( in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = true;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      token += *it;
   }
   if ( !token.isEmpty() )
   {
      qsl << token;
   }
   return qsl;
}
