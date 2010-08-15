//! \file us_util.cpp
#include "us_util.h"
#include <uuid/uuid.h> 

// get next token in a string, as defined by a separator; update for next pass
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

// generate a new GUID value and return it in QString form
QString US_Util::new_guid( void )
{
   uchar c[ 16 ];           // Binary
   char  u[ 37 ];           // Char array

   uuid_generate( c );      // generate a new UUID in binary form

   uuid_unparse ( c, u );   // unparse it the standard character form

   return QString( u );     // return a string representation
}

// calculate the md5hash and size of a named file (full path)
QString US_Util::md5sum_file( QString filename )
{
   QFile f( filename );

   // if unable to open the file, return "hash size" of "0 0"
   if ( ! f.open( QIODevice::ReadOnly ) )
      return "0 0";

   // otherwise, get the full contents of the file into a QByteArray
   QByteArray data = f.readAll();
   f.close();

   // get the md5 hash, convert to hex, get file size;  then format "hash size"
   QString hashandsize =
      QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 )
      .toHex() ) + " " + QString::number( QFileInfo( filename ).size() );

   // return the "hash size" string
   return hashandsize;
}

// convert DateTime string to unambiguous UTC form ("yyyy-mm-dd HH:MM:SS UTC")
QString US_Util::toUTCDatetimeText( QString dttext, bool knownUTC )
{
   QString utctext = dttext;
   int     ixLT    = dttext.length() - 2;          // expected last 'T'
   int     ixT     = dttext.indexOf( "T" );        // index of first 'T'
   bool    haveT   = ( ixT > 0  &&  ixT != ixLT ); // have lone 'T'
   bool    haveUTC = dttext.endsWith( "UTC" );     // have "UTC" on end

   if ( knownUTC )
   {  // if known to be UTC, just insure no single 'T' and " UTC" suffix

      if ( haveT )
      {  // if single 'T' between date and time, replace with ' '
         utctext.replace( ixT, 1, " " );
      }

      if ( !haveUTC )
      {  // if no " UTC" suffix, append it
         utctext.append( " UTC" );
      }
   }

   else
   {  // if we don't know that it's UTC, convert when " UTC" is missing

      if ( !haveUTC )
      {  // if no " UTC", convert to UTC and append " UTC"
         QDateTime dt;

         if ( dttext.indexOf( "-" ) == 4 )
         {  // if already in ISO form, get UTC DateTime from string in ISO form
            dt      = QDateTime::fromString( dttext, Qt::ISODate ).toUTC();
         }

         else
         {  // if in unknown form, get UTC DateTime assuming default form
            dt      = QDateTime::fromString( dttext ).toUTC();
         }

         // new text version in ISO form
         utctext = dt.toString( Qt::ISODate );

         // re-determine whether we have single 'T'
         haveT   = ( utctext.indexOf( "T" ) > 0 );

         // append the "UTC"
         utctext.append( " UTC" );
      }

      if ( haveT )
      {  // if single 'T' between date and time, replace with ' '
         utctext.replace( ixT, 1, " " );
      }
   }

   return utctext;
}

// insure DateTime text is in ISO form (ISO, UTC, or unknown on input )
QString US_Util::toISODatetimeText( QString dttext )
{
   QString isotext = dttext;   // default assumes already in ISO form

   if ( dttext.contains( " UTC" ) )
   {  // if UTC form on input, convert by removing " UTC" and adding 'T'
      isotext.replace( " UTC", "" ).replace( dttext.indexOf( " " ), 1, "T" );
   }

   else if ( dttext.indexOf( "-" ) != 4 )
   {  // if not ISO (unknown), convert by back-and-forth from DateTime
      isotext = QDateTime::fromString( dttext ).toString( Qt::ISODate );
      // otherwise, we must already have ISO form
   }

   return isotext;
}

