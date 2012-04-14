#include "../include/us_json.h"

// #define USJ_DEBUG
// #define USJ_DEBUG_SUMMARY

map < QString, QString > US_Json::split( QString qs )
{
#if defined( US_JSON_DEBUG )
   cout << "call json with <" << qs.ascii() << ">\n";
#endif

   map < QString, QString > result;

   qs.stripWhiteSpace();

   bool in_json = false;
   bool in_tok1 = false;
   bool in_tok2 = false;
   bool in_quote = false;
   bool in_bracket = false;
   bool in_brace = false;

   QString tok1;
   QString tok2;

   for ( unsigned i = 0; i < qs.length(); i++ )
   {
      QChar qc = qs.at( i );
#if defined( USJ_DEBUG )
      cout << QString( "pos %1 char %2\n" ).arg( i ).arg( qc ).ascii();
#endif

      if ( qc == '{' && !in_json )
      {
#if defined( USJ_DEBUG )
         cout << QString( "start json toks %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         in_json = true;
         in_tok1 = true;
         continue;
      }
      
      if ( qc == '}' && in_json && !in_quote && !in_bracket && !in_brace )
      {
#if defined( USJ_DEBUG )
         cout << QString( "end json toks %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         result[ tok1 ] = tok2;
         tok1 = "";
         tok2 = "";
         in_json = false;
         in_tok1 = false;
         in_tok2 = false;
         break;
      }
      
      if ( qc == ':' && in_json && !in_quote && !in_bracket && !in_brace )
      {
#if defined( USJ_DEBUG )
         cout << QString( "colon found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         if ( !in_tok1 )
         {
            result[ "json parsing error" ] = "json error: unexpected ':'";
            return result;
         }
         in_tok1 = false;
         in_tok2 = true;
         continue;
      }

      if ( qc == '[' && in_json && !in_quote && in_tok2 && !in_bracket && !in_brace && !tok2.length() )
      {
#if defined( USJ_DEBUG )
         cout << QString( "open bracket found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         in_bracket = true;
         continue;
      }

      if ( qc == ']' && in_json && !in_quote && in_tok2 && in_bracket && !in_brace )
      {
#if defined( USJ_DEBUG )
         cout << QString( "close bracket found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         in_bracket = false;
         continue;
      }

      if ( qc == '{' && in_json && !in_quote && in_tok2 && !in_bracket && !in_brace && !tok2.length() )
      {
#if defined( USJ_DEBUG )
         cout << QString( "open bracket found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         in_brace = true;
         continue;
      }

      if ( qc == '}' && in_json && !in_quote && in_tok2 && in_brace && !in_bracket )
      {
#if defined( USJ_DEBUG )
         cout << QString( "close bracket found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         in_brace = false;
         continue;
      }
      
      if ( qc == ',' && in_json && !in_quote && !in_bracket && !in_brace )
      {
#if defined( USJ_DEBUG )
         cout << QString( "comma found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         if ( !in_tok2 )
         {
            result[ "json parsing error" ] = "json error: unexpected ','";
            return result;
         }
         result[ tok1 ] = tok2;
         tok1 = "";
         tok2 = "";
         in_tok1 = true;
         in_tok2 = false;
         continue;
      }
      
      if ( qc == '"' && in_json && !in_bracket && !in_brace )
      {
#if defined( USJ_DEBUG )
         cout << QString( "quote found %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         if ( in_quote )
         {
            in_quote = false;
            continue;
         }
         if ( !in_quote )
         {
            in_quote = true;
            continue;
         }
      }
      
      if ( in_json )
      {
#if defined( USJ_DEBUG )
         cout << QString( "appending %1 %2\n" ).arg( tok1 ).arg( tok2 ).ascii();
#endif
         if ( in_tok1 )
         {
            tok1.append( qc );
            continue;
         }
         if ( in_tok2 )
         {
            tok2.append( qc );
            continue;
         }
         result[ "json parsing error" ] = "json error: unexpected character";
         return result;
      }
   }
#if defined( USJ_DEBUG ) || defined( USJ_DEBUG_SUMMARY )
   for ( map < QString, QString >::iterator it = result.begin();
         it != result.end();
         it++ )
   {
      cout << QString( "Json <%1>:<%2>\n" ).arg( it->first ).arg( it->second ).ascii();
   }
#endif

   return result;
}


QString US_Json::compose( map < QString, QString > &mqq )
{
   QString result = "{";

   QRegExp rx_no_quotes( "^(\\d+|null|\\[.*\\]|\\{.*\\})$" );

   for ( map < QString, QString >::iterator it = mqq.begin();
         it != mqq.end();
         it++ )
   {

      result += 
         ( rx_no_quotes.search( it->first ) != -1  ?
           it->first :
           "\"" + it->first + "\"" )
         + ":" +
         ( rx_no_quotes.search( it->second ) != -1  ?
           it->second :
           "\"" + it->second + "\"" )
         + "," 
         ;
   }
#ifdef QT4
   result.chop( 1 );
   result += "}";
#else
   result.at( result.length() - 1 ) = '}';
#endif

#if defined( USJ_DEBUG ) || defined (USJ_DEBUG_SUMMARY )
   cout << QString( "compose summary:\n%1\n" ).arg( result ).ascii();
#endif
   return result;
}
