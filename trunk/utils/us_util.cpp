//! \file us_util.cpp
#include "us_util.h"

// Get next token in a string, as defined by a separator; update for next pass
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

// Generate a new GUID value and return it in QString form
QString US_Util::new_guid( void )
{
   return QUuid::createUuid().toString().mid( 1, 36 );
}

// Calculate the md5hash and size of a named file (full path)
QString US_Util::md5sum_file( QString filename )
{
   QFile f( filename );

   // If unable to open the file, return "hash size" of "0 0"
   if ( ! f.open( QIODevice::ReadOnly ) )
      return "0 0";

   // Otherwise, get the full contents of the file into a QByteArray
   QByteArray data = f.readAll();
   f.close();

   // Get the md5 hash, convert to hex, get file size;  then format "hash size"
   QString hashandsize =
      QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 )
      .toHex() ) + " " + QString::number( QFileInfo( filename ).size() );

   // Return the "hash size" string
   return hashandsize;
}

// Convert DateTime string to unambiguous UTC form ("yyyy-mm-dd HH:MM:SS UTC")
QString US_Util::toUTCDatetimeText( QString dttext, bool knownUTC )
{
   QString utctext = dttext;
   int     ixLT    = dttext.length() - 2;          // expected last 'T'
   int     ixT     = dttext.indexOf( "T" );        // index of first 'T'
   bool    haveT   = ( ixT > 0  &&  ixT != ixLT ); // have lone 'T'
   bool    haveUTC = dttext.endsWith( "UTC" );     // have "UTC" on end

   if ( knownUTC )
   {  // If known to be UTC, just insure no single 'T' and " UTC" suffix

      if ( haveT )
      {  // If single 'T' between date and time, replace with ' '
         utctext.replace( ixT, 1, " " );
      }

      if ( !haveUTC )
      {  // If no " UTC" suffix, append it
         utctext.append( " UTC" );
      }
   }

   else
   {  // If we don't know that it's UTC, convert when " UTC" is missing

      if ( !haveUTC )
      {  // If no " UTC", convert to UTC and append " UTC"
         QDateTime dt;

         if ( dttext.indexOf( "-" ) == 4 )
         {  // If already in ISO form, get UTC DateTime from string in ISO form
            dt      = QDateTime::fromString( dttext, Qt::ISODate ).toUTC();
         }

         else
         {  // If in unknown form, get UTC DateTime assuming default form
            dt      = QDateTime::fromString( dttext ).toUTC();
         }

         // New text version in ISO form
         utctext = dt.toString( Qt::ISODate );

         // Re-determine whether we have single 'T'
         haveT   = ( utctext.indexOf( "T" ) > 0 );

         // Append the "UTC"
         utctext.append( " UTC" );
      }

      if ( haveT )
      {  // If single 'T' between date and time, replace with ' '
         utctext.replace( ixT, 1, " " );
      }
   }

   return utctext;
}

// Ensure DateTime text is in ISO form (ISO, UTC, or unknown on input )
QString US_Util::toISODatetimeText( QString dttext )
{
   QString isotext = dttext;   // default assumes already in ISO form

   if ( dttext.contains( " UTC" ) )
   {  // If UTC form on input, convert by removing " UTC" and adding 'T'
      isotext.replace( " UTC", "" ).replace( dttext.indexOf( " " ), 1, "T" );
   }

   else if ( dttext.indexOf( "-" ) != 4 )
   {  // If not ISO (unknown), convert by back-and-forth from DateTime
      isotext = QDateTime::fromString( dttext ).toString( Qt::ISODate );
      // Otherwise, we must already have ISO form
   }

   return isotext;
}

// Convert a binary uuid to a QString
QString US_Util::uuid_unparse( unsigned char* uu )
{
   return QString().sprintf(
         "%02hhx%02hhx%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-%02hhx%02hhx-"
         "%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
         uu[  0 ], uu[  1 ], uu[  2 ], uu[  3 ], uu[  4 ], uu[  5 ],
         uu[  6 ], uu[  7 ], uu[  8 ], uu[  9 ], uu[ 10 ], uu[ 11 ],
         uu[ 12 ], uu[ 13 ], uu[ 14 ], uu[ 15 ] );
}

// Convert a triple string from expanded to compressed form
QString US_Util::compressed_triple( const QString& ccw )
{
   // For example, convert "1 / A / 290" to "1A290"
   return ( ccw.section( "/", 0, 0 ).simplified() +
            ccw.section( "/", 1, 1 ).simplified() +
            ccw.section( "/", 2, 2 ).simplified() );
}

// Convert a triple string from expanded to compressed form
QString US_Util::expanded_triple( const QString& ccw, bool spaces )
{
   QString cells( "ABCDEFGH" );

   // Get, for example, {"4","A","280"} from "4A280"
   int     jj   = 1;
           jj   = cells.contains( ccw.mid( 2, 1 ) ) ? 2 : jj;
           jj   = cells.contains( ccw.mid( 3, 1 ) ) ? 3 : jj;
           jj   = cells.contains( ccw.mid( 4, 1 ) ) ? 4 : jj;
   QString cell = ccw.left( jj     );
   QString chan = ccw.mid ( jj, 1  );
   QString wvln = ccw.mid ( jj + 1 );

   // Spaces value determines separator as " / " or "/"
   QString sep  = spaces ? " / " : "/";

   // Return, for example, "4 / A / 280" or "4/A/280"
   return ( cell + sep + chan + sep + wvln );
}

// A helper function to convert a character hex digit to decimal
unsigned char US_Util::hex2int( unsigned char c )
{
   if ( c <= '9' )
      return (unsigned char)( c - '0' );

   c &= 0x5f;  // Make upper case if necessary
   return (unsigned char)( c - 'A' + 10 );
}

// Convert a QString uuid into a 16-byte binary array
void US_Util::uuid_parse( const QString& in, unsigned char* uu )
{
   QByteArray     ba = in.toUtf8();
   unsigned char* p  = (unsigned char*)ba.data();

   for ( int i = 0; i < 16; i++ )
   {
      while ( *p == '-' ) p++;

      char n = (char)( hex2int( *p++ ) << 4 );
      n =  (char)( (int)n | hex2int( *p++ ) );

      uu[ i ] = n;
   }
}

