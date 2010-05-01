#include "us_util.h"
#include <uuid/uuid.h> 

QString US_Util::getToken( QString& s, const QString& separator )
{
   QString token;
   
   // Find position of 1st delimeter
   int pos = s.indexOf( separator, 0 );
   
   if ( pos < 0 ) // No delimeter
   {
      token = s;
      s     = "";
      return token;
   }

   while ( pos == 0 ) // Strip leading delimiters
   {
      s.remove( 0, 1 ); // Remove 1st character 
      pos = s.indexOf( separator, 0 );

      if ( pos < 0 ) // No delimeter
      {
         token = s;
         s     = "";
         return token;
      }
   }

   token = s.left( pos );
   s.remove( 0, pos + 1 ); // Remove token plus delimeter
   return token;
}

QString US_Util::new_guid( void )
{
   uchar c[ 16 ];  // Binary
   char  u[ 37 ];  // String
   uuid_generate( c );
   uuid_unparse ( c, u );
   return QString( u );
}
