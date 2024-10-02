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
   return QString().asprintf(
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
   QString cells( "ABCDEFGHZ" );

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

// Return a flag true if this is the ith time an error occurred;
// in truth, that a random number over the given range has hit 1.
bool US_Util::ithTime( int timeinc )
{
   int rannum  = rand() % timeinc;
   return ( rannum == 1 );
}

// Return a flag if an XML attribute string represents true or false.
bool US_Util::bool_flag( const QString xmlattr )
{
   return ( !xmlattr.isEmpty()  &&  ( xmlattr == "1"  ||  xmlattr == "T" ) );
}

// Return an XML attribute string to represent true or false.
QString US_Util::bool_string( const bool boolval )
{
   return QString( boolval ? "1" : "0" );
}

// Return a QString representing a QList<QStringList>
//   Two delimeters (e.g., '^', ':') are chosen by finding the
//   first two not found in any QString of any QStringList.
//   Those two characters (level1 and level2) are the first
//   two characters of the output string. The level2 delimeter
//   separates strings in each list and the level1 delimeter
//   separates the stringlist substrings.
int US_Util::listlistBuild( QList< QStringList >& lsl, QString& llstring )
{
   int nlelem      = lsl.count();
//qDebug() << "llB: nlelem" << nlelem;

   if ( nlelem == 0 )
      return nlelem;

   int ndelimf     = 0;
   QString delim1;
   QString delim2;

   // List of potential delimeters
   QStringList delims;
   delims << "^" << ":" << "$" << "+" << "=" << "*" << "&" << "%"
          << "#" << "@" << ";" << ",";
//qDebug() << "llB:  delims" << delims;

   // Search the strings in the list of string lists for each delimeter
   for ( int ii = 0; ii < delims.count(); ii++ )
   {
      QString delim   = delims[ ii ];
//qDebug() << "llB:   ii delim" << ii << delim;
      int kfound      = 0;

      for ( int jj = 0; jj < nlelem; jj++ )
      {
         for ( int kk =0; kk < lsl[ jj ].count(); kk++ )
         {
            if ( lsl[ jj ][ kk ].contains( delim ) )
            {
               kfound++;
               break;
            }
         }
         if ( kfound > 0 )
            break;
      }
//qDebug() << "llB:     kbound" << kfound;

      if ( kfound == 0 )
      {  // This delimeter not found:  use it for one of the two
         ndelimf++;
//qDebug() << "llB:      ndelimf" << ndelimf << "delim" << delim;
         if ( ndelimf == 1 )
         {
            delim1          = delim;
         }
         else
         {
            delim2          = delim;
            break;
         }
      }

      if ( ndelimf > 1 )
         break;
   }

//qDebug() << "llB: ndelimf" << ndelimf << "delim1" << delim1 << "delim2" << delim2;
   if ( ndelimf < 2 )
   {  // Ouch!!! Couldn't find two delimeters to use
      llstring        = QString( "" );
      return 0;
   }

   // Now build up the output string starting with delimeters
   llstring        = delim1 + delim2;
   int lastii      = nlelem - 1;

   // Build outer strings for each QStringList
   for ( int ii = 0; ii < nlelem; ii++ )
   {
      QStringList sl  = lsl[ ii ];
//qDebug() << "llB:  ii" << ii << "sl" << sl;
      int nslelem     = sl.count();
      int lastjj      = nslelem - 1;
      QString inner( "" );

      // Build an inner string from QStringList strings
      for ( int jj = 0; jj < nslelem; jj++ )
      {  
         // Concatenate a string
         inner          += sl[ jj ];
      
         // For all but last, concatenate a delimeter
         if ( jj < lastjj )
            inner          += delim2;
      }
//qDebug() << "llB:    inner" << inner;
      
      // Concatenate a string
      llstring       += inner;
      
      // For all but last, concatenate a delimeter
      if ( ii < lastii )
         llstring       += delim1;
   }
//qDebug() << "llB: llstring" << llstring;
      
   return nlelem;          // Return the count of QList elements
}

// Return a QList<QStringList> as represented by an input QString.
//   The first two characters (level1 and level2) of the input string
//   are taken as delimeters. These delimeters are used to separate
//   out stringlist substrings and then to separate out strings within
//   each stringlist.
int US_Util::listlistParse( QList< QStringList >& lsl, QString& llstring )
{
   int nlelem      = 0;
   QString delim1  = QString( llstring ).left( 1 );   // Outer delimeter (ch 1)
   QString delim2  = QString( llstring ).mid( 1, 1 ); // Inner delimeter (ch 2)
//qDebug() << "llP: delim1" << delim1 << "delim2" << delim2;

   if ( delim1.isEmpty()  ||  delim2.isEmpty() )
   {  // Ouch!!  Input string doesn't even have delimeters
      lsl.clear();
      return nlelem;
   }

   // Get the list of formatted outer level strings
//qDebug() << "llP: llstring" << llstring;
//qDebug() << "llP: llstringM" << QString(llstring).mid(2);
   QStringList out = QString( llstring ).mid( 2 ).split( delim1 );
//qDebug() << "llP: out count" << out.count();
   nlelem          = out.count();    // List count
   lsl.clear();                      // Initialize output

   // Build the outer list of stringlists by parsing inner strings
   for ( int ii = 0; ii < nlelem; ii++ )
   {
      QString inner   = out[ ii ];   // Inner formatted string
      lsl << inner.split( delim2 );  // Parse and append stringlist
   }

   return nlelem;                    // Return the count of QList elements
}

