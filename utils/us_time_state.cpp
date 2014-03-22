//! \file us_time_state.cpp
#include "us_time_state.h"
#include "us_settings.h"

#ifndef _TR_
#define _TR_(a) QObject::tr(a)
#endif
#ifndef DbgLv
#define DbgLv(a) if(dbg_level>a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

// Constructor of the US_TimeState object
US_TimeState::US_TimeState() : QObject()
{
   filename    = QString( "" );
   filepath    = filename;
   fvers       = QString( _TMST_VERS_ );
   fileo       = NULL;
   filei       = NULL;
   dso         = NULL;
   dsi         = NULL;
   error_msg   = QString( "" );
   wr_open     = false;
   rd_open     = false;
   const_ti    = false;
   ntimes      = 0;
   nvalues     = 0;
   timex       = -1;
   fhdr_size   = 0;
   rec_size    = 0;
   file_size   = (qint64)0;
   dbg_level   = US_Settings::us_debug();
   cdata       = (char*)cwork;

   keys.clear();
   fmts.clear();
   offs.clear();

   // Determine little-endian flag and integer-size for conversion utilities
   unsigned char vbuf[ 8 ];
   int* vint   = (int*)vbuf;
   *vint       = 1;
   lit_endian  = ( vbuf[ 0 ] != '\0' );  // Flag if machine is little-endian
   int_size    = sizeof( int );          // Save word size of integer
}

// Open a file to write TMST data
int US_TimeState::open_write_data( QString fpath,
                                   double timeinc, double ftime )
{
   int status = 0;

   filepath    = fpath;
   filename    = filepath.section( "/", -1, -1 );
   fvers       = QString( _TMST_VERS_ );
   time_inc    = timeinc;
   time_first  = ftime;
   const_ti    = ( timeinc > 0.0 );

   fileo       = new QFile( filepath );

   if ( ! fileo->open( QIODevice::WriteOnly ) )
   {
      status     = 100;
      set_error( status );
      error_msg += filepath;
      return status;
   }

   dso         = new QDataStream( fileo );
   wr_open     = true;
   rd_open     = false;
   ntimes      = 0;
   nvalues     = 0;
   rec_size    = 0;

   strncpy( cdata,     _TMST_MAGI_, 4 );  // "USTS"
   strncpy( cdata + 4, _TMST_VERS_, 3 );  // "1.0"
   cdata[ 5 ]  = cdata[ 6 ];              // '1','0'
   fhdr_size   = 6;

   keys.clear();
   fmts.clear();
   offs.clear();

   // Write out the file header
   dso->writeRawData( cdata, fhdr_size );
   file_size   = (qint64)fhdr_size;

   return status;
}

// Specify key and format of a data field
int US_TimeState::set_key( QString key, QString format )
{
   int status  = 0;

   QString fl1 = QString( format ).left( 1 );

   if ( QString( "FIC" ).indexOf( fl1 ) < 0 )
   {
      status      = 101;
      set_error( status );
      error_msg += format;
      return status;
   }

   int flen     = QString( format ).mid( 1 ).toInt();
   int offset   = ( nvalues == 0 ) ? 0 : offs[ nvalues - 1 ] + flen;
   keys << key;
   fmts << format;
   offs << offset;
   nvalues++;

   return status;
}

// Specify keys and formats for all data fields
int US_TimeState::set_keys( QStringList& fkeys, QStringList& ffmts )
{
   int status  = 0;

   nvalues     = keys.count();

   if ( ffmts.count() != nvalues )
   {
      nvalues     = 0;
      return set_error( 102 );
   }

   for ( int jj = 0; jj < nvalues; jj++ )
   {
      status      = set_key( fkeys[ jj ], ffmts[ jj ] );
      if ( status != 0 )
         break;
   }

   return status;
}

// Specify keys and formats for all data fields
int US_TimeState::set_keys( QStringList& fkeys )
{
   QStringList wkeys;
   QStringList wfmts;

   for ( int jj = 0; jj < fkeys.count(); jj++ )
   {
      QString fkey = fkeys[ jj ];
      QString ffmt = fkey.section( " ", -1, -1 ).simplified();
      fkey         = fkey.section( " ",  0, -2 ).simplified();

      wkeys << fkey;
      wfmts << ffmt;
   }

   return set_keys( wkeys, wfmts );
}

// Set an integer value for the current record
int US_TimeState::set_value( QString key, int ivalue )
{
   int status  = 0;
DbgLv(1) << "DtsF: sival: key ival" << key << ivalue;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;
   double dvalue;

   status      = key_parameters( key, &rfmt, &rlen, &roff );

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      int   coff  = 0;

      switch( rfmt )
      {
         case 0:                                 // I4
            store_iword( cdata + roff, ivalue );
            break;
         case 1:                                 // I2
            store_hword( cdata + roff, ivalue );
            break;
         case 2:                                 // I1
            sprintf( cval, "%d", ivalue );
            cwrk[ 0 ]   = cwrk[ strlen( cval ) - 1 ] && 15;
            memcpy     ( cdata + roff, cval + coff, 1 );
            break;
         case 3:                                 // R4
            dvalue      = (double)ivalue;
            store_fword( cdata + roff, dvalue );
            break;
         case 4:                                 // R8
            dvalue      = (double)ivalue;
            store_dword( cdata + roff, dvalue );
            break;
         case 5:                                 // Cnnn
            sprintf( cval, "%d", ivalue );
            coff        = 0;
            memcpy     ( cdata + roff, cval + coff, rlen );
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   return status;
}

// Set a float value for the current record
int US_TimeState::set_value( QString key, double dvalue )
{
   int status  = 0;
DbgLv(1) << "DtsF: sdval: key dval" << key << dvalue;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;
   int jvalue;

   status      = key_parameters( key, &rfmt, &rlen, &roff );

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      int   coff  = 0;

      switch( rfmt )
      {
         case 0:                                 // I4
            jvalue      = qRound( dvalue );
            store_iword( cdata + roff, jvalue );
            break;
         case 1:                                 // I2
            jvalue      = qRound( dvalue );
            store_hword( cdata + roff, jvalue );
            break;
         case 2:                                 // I1
            jvalue      = qRound( dvalue );
            sprintf( cval, "%d", jvalue );
            cwrk[ 0 ]   = cwrk[ strlen( cval ) - 1 ] && 15;
            memcpy     ( cdata + roff, cval + coff, 1 );
            break;
         case 3:                                 // R4
            store_fword( cdata + roff, dvalue );
            break;
         case 4:                                 // R8
            store_dword( cdata + roff, dvalue );
            break;
         case 5:                                 // Cnnn
            sprintf( cval, "%f", dvalue );
            coff        = 0;
            memcpy     ( cdata + roff, cval + coff, rlen );
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   return status;
}

// Set a character string value for the current record
int US_TimeState::set_value( QString key, QString svalue )
{
   int status  = 0;
DbgLv(1) << "DtsF: ssval: key sval" << key << svalue;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;
   int jvalue;
   double dvalue;
   QString wstr;

   status      = key_parameters( key, &rfmt, &rlen, &roff );

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      int   coff  = 0;

      switch( rfmt )
      {
         case 0:                                 // I4
            jvalue      = svalue.toInt();
            store_iword( cdata + roff, jvalue );
            break;
         case 1:                                 // I2
            jvalue      = svalue.toInt();
            store_hword( cdata + roff, jvalue );
            break;
         case 2:                                 // I1
            jvalue      = svalue.toInt();
            sprintf( cval, "%d", jvalue );
            cwrk[ 0 ]   = cwrk[ strlen( cval ) - 1 ] && 15;
            memcpy     ( cdata + roff, cval + coff, 1 );
            break;
         case 3:                                 // R4
            dvalue      = svalue.toDouble();
            store_fword( cdata + roff, dvalue );
            break;
         case 4:                                 // R8
            dvalue      = svalue.toDouble();
            store_dword( cdata + roff, dvalue );
            break;
         case 5:                                 // Cnnn
            wstr        = svalue;

            if ( svalue.length() < rlen )
            {  // Pad with blanks
               for ( int jj = svalue.length(); jj < rlen; jj++ )
                  wstr  += QString( " " );
            }

            memcpy( cdata + roff, wstr.toAscii().constData(), rlen );
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   return status;
}

// Flush the current data record (write to file)
int US_TimeState::flush_record()
{
   int status  = 0;

   dso->writeRawData( cdata, rec_size );
   timex++;
   memset( cdata, 0, rec_size );
   file_size  += (qint64)rec_size;

   return status;
}

// Close the output data file
int US_TimeState::close_write_data()
{
   int status  = 0;

   fileo->close();
   fileo       = NULL;
   dso         = NULL;
   wr_open     = false;

   return status;
}

// Write the sibling XML file for the last opened output data file
int US_TimeState::write_xml( double timeinc )
{
   int status  = 0;
   time_inc    = timeinc < 0.0 ? time_inc : timeinc;

   QString xfname = QString( filename ).section( ".", 0, -2 ) + ".xml";
   QString xfpath = QString( filepath ).section( ".", 0, -2 ) + ".xml";

   QFile xfo( xfpath );

   if ( !xfo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      status     = 700;
      set_error( status );
      return status;
   }

   QXmlStreamWriter xml( &xfo );
   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD              ( "<!DOCTYPE US_TimeState>" );
   xml.writeStartElement( "TimeState" );
   xml.writeAttribute   ( "version",        QString( _TMST_VERS_ ) );

   xml.writeStartElement( "file" );
   xml.writeAttribute   ( "time_count",     QString::number( ntimes )     );
   xml.writeAttribute   ( "constant_incr",  ( const_ti ? "1" : "0" )      );
   xml.writeAttribute   ( "time_increment", QString::number( time_inc )   );
   xml.writeAttribute   ( "first_time",     QString::number( time_first ) );

   for ( int jj = 0; jj < nvalues; jj++ )
   {
      xml.writeStartElement( "value" );
      xml.writeAttribute   ( "key",    keys[ jj ] );
      xml.writeAttribute   ( "format", fmts[ jj ] );
      xml.writeEndElement  ( );
   }

   xml.writeEndElement  ( );  // "</file>"
   xml.writeEndElement  ( );  // "</TimeState>"
   xml.writeEndDocument ( );
   xfo.close();

   return status;
}

// Read in TMST data from a file
int US_TimeState::open_read_data( QString fpath )
{
   int status  = 0;
   file_size   = (qint64)0;
   filei       = new QFile( filepath );

   if ( ! filei->open( QIODevice::ReadOnly ) )
   {
      status     = 500;
      set_error( status );
      error_msg += filepath;
      return status;
   }

   file_size   = filei->size();
   filepath    = fpath;
   filename    = filepath.section( "/", -1, -1 );
   dsi         = new QDataStream( filei );
   fvers       = QString( _TMST_VERS_ );

   rd_open     = true;
   wr_open     = false;
   fhdr_size   = 6;

   // Read in the file header
   dsi->readRawData( cdata, fhdr_size );

   if ( strncmp( cdata, _TMST_MAGI_, 4 ) != 0 )
   {
      status     = 501;
      set_error( status );
      error_msg += QString( _TMST_MAGI_ ) + " "
                 + QString( cdata ).left( 4 );
      return status;
   }

   cdata[ 6 ]  = cdata[ 4 ];
   cdata[ 7 ]  = '.';
   cdata[ 8 ]  = cdata[ 5 ];
   cdata[ 9 ]  = '\0';
   if ( strncmp( cdata+6, _TMST_VERS_, 3 ) )
   {
      status     = 502;
      set_error( status );
      error_msg += fvers + " " + QString( cdata+6 ).left( 3 );
      return status;
   }

   ntimes      = 0;
   nvalues     = 0;
   rec_size    = 0;

   strncpy( cdata,   _TMST_MAGI_, 4 );
   strncpy( cdata+4, _TMST_VERS_, 3 );
   cdata[ 5 ]  = cdata[ 6 ];

   keys.clear();
   fmts.clear();
   offs.clear();
   int koff    = 0;

   // Read the associated XML file
   QString xfname = QString( filename ).section( ".", 0, -2 ) + ".xml";
   QString xfpath = QString( filepath ).section( ".", 0, -2 ) + ".xml";

   QFile xfi( xfpath );

   if ( !xfi.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      status     = 505;
      set_error( status );
      return status;
   }

   QXmlStreamReader xml( &xfi );

   while( ! xml.atEnd() )
   {
      QString xname;
      QString attv;
      QXmlStreamAttributes attr;
      xml.readNext();

      if ( xml.isStartElement() )
      {
         xname      = xml.name().toString();
         attr       = xml.attributes();

         if ( xname == "TimeState" )
         {
            fvers      = attr.value( "version" ).toString();
         }

         else if ( xname == "file" )
         {
            attv       = attr.value( "time_count"     ).toString();
            ntimes     = attv.isEmpty() ? ntimes     : attv.toInt();

            attv       = attr.value( "constant_incr"  ).toString();
            const_ti   = attv.isEmpty() ? const_ti   : ( attv == "1" );

            attv       = attr.value( "time_increment" ).toString();
            time_inc   = attv.isEmpty() ? time_inc   : attv.toDouble();

            attv       = attr.value( "first_time"     ).toString();
            time_first = attv.isEmpty() ? time_first : attv.toDouble();
         }

         else if ( xname == "value" )
         {
            QString ky = attr.value( "key"    ).toString();
            QString fm = attr.value( "format" ).toString();

            keys << ky;
            fmts << fm;
            offs << koff;
            koff      += fm.mid( 1 ).toInt();

            nvalues++;
         }
      }
   }

   xfi.close();

   return status;
}

// Get the count of time value records
int US_TimeState::time_count()
{
   return ntimes;
}

// Get the character and parameters of the time range
int US_TimeState::time_range( bool* constti, double* timeinc, double* ftime )
{
   if ( constti != NULL )  *constti = const_ti;
   if ( timeinc != NULL )  *timeinc = time_inc;
   if ( ftime   != NULL )  *ftime   = time_first;

   return ntimes;
}

// Get record field keys and formats
int US_TimeState::field_keys( QStringList* keysP, QStringList* fmtsP )
{
   if ( keysP != NULL )
      *keysP      = keys;

   if ( fmtsP != NULL )
      *fmtsP      = fmts;

   return nvalues;
}

// Read in the next or a specified data record
int US_TimeState::read_record( int rtimex )
{
   int status  = 0;

   if ( timex < 0 )
   {  // If current index indicates still in header, complete header calcs
      int lstv    = nvalues - 1;
      QString fm  = fmts[ lstv ];
      rec_size    = offs[ lstv ] + fm.mid( 1 ).toInt();
   }

   if ( rtimex < 0  ||  ( rtimex - timex ) == 1 )
   {  // Set to next and read it in
      timex++;
      dsi->readRawData( cdata, rec_size );
   }

   else if ( rtimex > timex )
   {  // Read in records until we have reached the designated one
      while ( timex < rtimex )
      {
         timex++;
         dsi->readRawData( cdata, rec_size );
      }
   }

   else
   {  // Error if designated time index is less than current
      status     = 510;
      return set_error( status );
   }

   return status;
}

// Get a time integer value for a given key from the current record
int US_TimeState::time_ivalue( QString key, int* stat )
{
   int ivalue  = 0;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      char* eval  = cval + rlen;
      double dvalue;

      switch( rfmt )
      {
         case 0:                                 // I4
            ivalue      = iword( rdata );
            break;
         case 1:                                 // I2
            ivalue      = hword( rdata );
            break;
         case 2:                                 // I1
            ivalue      = (unsigned int)rdata[ 0 ];
            break;
         case 3:                                 // R4
            dvalue      = dword( rdata );
            ivalue      = qRound( dvalue );
            break;
         case 4:                                 // R8
            dvalue      = d8word( rdata );
            ivalue      = qRound( dvalue );
            break;
         case 5:                                 // Cnnn
            memcpy( cval, rdata, rlen );
            *eval       = '\0';
            ivalue      = QString( cval ).left( rlen ).toInt();
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   if ( stat != NULL )
      *stat      = status;
   return ivalue;
}

// Get a time double value for a given key from the current record
double US_TimeState::time_dvalue( QString key, int* stat )
{
   double dvalue  = 0.0;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      char* eval  = cval + rlen;
      double ivalue;

      switch( rfmt )
      {
         case 0:                                 // I4
            ivalue      = iword( rdata );
            dvalue      = (double)ivalue;
            break;
         case 1:                                 // I2
            ivalue      = hword( rdata );
            dvalue      = (double)ivalue;
            break;
         case 2:                                 // I1
            ivalue      = (unsigned int)rdata[ 0 ];
            dvalue      = (double)ivalue;
            break;
         case 3:                                 // R4
            dvalue      = dword( rdata );
            break;
         case 4:                                 // R8
            dvalue      = d8word( rdata );
            break;
         case 5:                                 // Cnnn
            memcpy( cval, rdata, rlen );
            *eval       = '\0';
            dvalue      = QString( cval ).left( rlen ).toDouble();
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   if ( stat != NULL )
      *stat      = status;
   return dvalue;
}

// Get a time string value for a given key from the current record
QString US_TimeState::time_svalue( QString key, int* stat )
{
   QString svalue;
   int     ivalue;
   double  dvalue;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      switch( rfmt )
      {
         case 0:                                 // I4
            ivalue      = iword( rdata );
            svalue      = QString::number( ivalue );
            break;
         case 1:                                 // I2
            ivalue      = hword( rdata );
            svalue      = QString::number( ivalue );
            break;
         case 2:                                 // I1
            ivalue      = (unsigned int)rdata[ 0 ];
            svalue      = QString::number( ivalue );
            break;
         case 3:                                 // R4
            dvalue      = dword( rdata );
            svalue      = QString::number( dvalue );
            break;
         case 4:                                 // R8
            dvalue      = d8word( rdata );
            svalue      = QString::number( dvalue );
            break;
         case 5:                                 // Cnnn
            svalue      = QString( rdata ).left( rlen );
            break;
         default:                                // UNKNOWN
            break;
      }
   }

   if ( stat != NULL )
      *stat      = status;
   return svalue;
}

// Close the input data file
int US_TimeState::close_read_data()
{
   int status  = 0;

   filei->close();
   filei       = NULL;
   dsi         = NULL;
   rd_open     = false;

   return status;
}

// Get the error message for a given status
QString US_TimeState::error_message( int status )
{
   struct errmap                       // Error status,message entry
   {
      int     estat;
      QString emsg;
   };

   const errmap emap[] =               // Error status-to-message mappings
   {
      {  100, _TR_( "Write-file open error: " ) },
      {  101, _TR_( "Invalid field format: "  ) },
      {  102, _TR_( "Keys count out of sync." ) },
      {  700, _TR_( "Write-XML-file open error" ) },
      {  500, _TR_( "Read-file open error: " ) },
      {  501, _TR_( "Not the TMST file magic number: " ) },
      {  502, _TR_( "Incompatible file format version: " ) },
      {  505, _TR_( "Read-XML-file open error" ) },
      {  510, _TR_( "Attempt to access previous time record" ) },
      {  901, _TR_( "Invalid key parameters (key,fmt,len,off): " ) },
      {  999, _TR_( "UNKNOWN"        ) }
   };

   const int netypes = sizeof( emap ) / sizeof( emap[ 0 ] );
   QString   errmsg  = _TR_( "UNKNOWN" );

   for ( int ii = 0; ii < netypes; ii++ )
   {  // Find the error message corresponding to the status flag
      if ( emap[ ii ].estat == status )
      {
         errmsg      = emap[ ii ].emsg;
         break;
      }
   }

DbgLv(1) << "DtsF: egmsg: status" << status << "message" << errmsg;
   return errmsg;
}

// Get the error message for the last error that occurred
QString US_TimeState::last_error_message( )
{
   return error_msg;
}

/////////////////////////////////////////////////////////////////////////////
// Private slots.  Internal utilities like fetch/store half,full,float
/////////////////////////////////////////////////////////////////////////////

// Utility to extract an unsigned half-word (I2) from a data byte array.
int US_TimeState::uhword( char* dba )
{
   unsigned char* cbuf = (unsigned char*)dba;
   int    j0    = (int)cbuf[ 0 ] & 255;
   int    j1    = (int)cbuf[ 1 ] & 255;
   return ( ( j0 << 8 ) | j1 );
}

// Utility to extract a signed half-word (I2) from a data byte array.
int US_TimeState::hword( char* dba )
{
   int    iword = uhword( dba );
   return ( ( iword < 32768 ) ? iword : ( iword - 65536 ) );
}

// Utility to extract a full-word (I4) from a data byte array.
int US_TimeState::iword( char* cbuf )
{
   int    j0    = uhword( cbuf );
   int    j1    = uhword( cbuf + 2 );
   return ( ( j0 << 16 ) | j1 );
}

// Utility to extract a float (F4) from a data byte array.
float US_TimeState::fword( char* cbuf )
{
   int    ival  = iword( cbuf );
   int*   iptr  = &ival;
   float* fptr  = (float*)iptr;
   return *fptr;
}

// Utility to extract a double (F4) from a data byte array.
double US_TimeState::dword( char* cbuf )
{
   float  fval  = fword( cbuf );
   return (double)fval;
}

// Utility to extract a double (F8) from a data byte array.
double US_TimeState::d8word( char* cbuf )
{
   int iwork[ 2 ];
   double *dptr = (double*)iwork;
   int ival     = iword( cbuf );
   int hx       = lit_endian ? 1 : 0;
   int lx       = 1 - hx;
   int ratio    = sizeof( double ) / int_size;
   if ( ratio == 2 )
   {
      iwork[ hx ]  = ival;
      iwork[ lx ]  = iword( cbuf + 4 );
   }
   else
      iwork[ 0 ]   = ival;

   return *dptr;
}

// Utility to store an integer as a half-word (I2) in a data byte array.
void US_TimeState::store_hword( char* dba, int ival )
{
   unsigned char* cbuf = (unsigned char*)dba;
   unsigned char  vbuf[ 8 ];
   int*           vint = (int*)vbuf;
   *vint         = ival;

   if ( lit_endian )   // Test machine byte order
   {  // Expected little-endian:  lo,hi order
      cbuf[ 0 ]     = vbuf[ 1 ];
      cbuf[ 1 ]     = vbuf[ 0 ];
   }

   else
   {  // Surprise! big-endian:  hi,lo order
      cbuf[ 0 ]     = vbuf[ int_size - 2 ];  // 2
      cbuf[ 1 ]     = vbuf[ int_size - 1 ];  // 3
   }
}

// Utility to store an integer as a full-word (I4) in a data byte array.
void US_TimeState::store_iword( char* dba, int ival )
{
   unsigned char* cbuf = (unsigned char*)dba;
   unsigned char  vbuf[ 8 ];
   int*           vint = (int*)vbuf;
   *vint         = ival;

   if ( lit_endian )   // Test machine byte order
   {  // Expected little-endian:  lo,hi order
      cbuf[ 0 ]     = vbuf[ 3 ];
      cbuf[ 1 ]     = vbuf[ 2 ];
      cbuf[ 2 ]     = vbuf[ 1 ];
      cbuf[ 3 ]     = vbuf[ 0 ];
   }

   else
   {  // Surprise! big-endian:  hi,lo order
      cbuf[ 0 ]     = vbuf[ int_size - 4 ];  // 0
      cbuf[ 1 ]     = vbuf[ int_size - 3 ];  // 1
      cbuf[ 2 ]     = vbuf[ int_size - 2 ];  // 2
      cbuf[ 3 ]     = vbuf[ int_size - 1 ];  // 3
   }
}

// Utility to store a double as a float (F4) in a data byte array.
void US_TimeState::store_fword( char* dba, double dval )
{
   float  fwrk   = (float)dval;
   float* fptr   = (float*)&fwrk;
   int*   iptr   = (int*)fptr;

   store_iword( dba, *iptr );
}

// Utility to store a double as a double (F8) in a data byte array.
void US_TimeState::store_dword( char* dba, double dval )
{
   double dwrk   = dval;
   int*   iptr   = (int*)&dwrk;

   store_iword( dba, *iptr );
   int hx       = lit_endian ? 1 : 0;
   int lx       = 1 - hx;
   int ratio    = sizeof( double ) / int_size;
   if ( ratio == 2 )
   {
      store_iword( dba,     *(iptr + hx) );
      store_iword( dba + 4, *(iptr + lx) );
   }
   else
   {
      store_iword( dba, *iptr );
   }
}

// Utility to store an array of half-word integers
void US_TimeState::store_hwords( char* dba, int* ivals, int nvals )
{
   char* cptr    = dba;

   for ( int ii = 0; ii < nvals; ii++, cptr+=2 )
   {
      store_hword( cptr, ivals[ ii ] );
   }
}

// Utility to store an array of full-word integers
void US_TimeState::store_iwords( char* dba, int* ivals, int nvals )
{
   char* cptr    = dba;

   for ( int ii = 0; ii < nvals; ii++, cptr+=4 )
   {
      store_iword( cptr, ivals[ ii ] );
   }
}

// Utility to store an array of floats
void US_TimeState::store_fwords( char* dba, double* dvals, int nvals )
{
   char* cptr    = dba;

   for ( int ii = 0; ii < nvals; ii++, cptr+=4 )
   {
      store_fword( cptr, dvals[ ii ] );
   }
}

// Utility to set an error status value and its corresponding message
int US_TimeState::set_error( int status )
{
   error_msg    = error_message( status );
   return status;
}

// Utility to return format,length,offset parameters for a given key
//  key   = input key string to examine
//  rfmtP = optional pointer for format flag (0-5 for I4,I2,I1,F4,F8,Cn)
//  rlenP = optional pointer for field length (1,2,4,8 for F,I; 1-127 for C)
//  roffP = optional pointer for offset of field in data record
int US_TimeState::key_parameters( QString& key,
                                  int* rfmtP, int* rlenP, int* roffP )
{
   int status   = 0;

   const QString sfmts( "I4I2I2F4F8C1C2C3C4C5C6C7C8C9" );

   int keyx     = keys.indexOf( key );                // Key index
   QString fmt  = ( keyx >= 0 ) ? fmts[ keyx ] : "";  // Associated format
   QString ftst = fmt.left( 2 );
   int rfmt     = sfmts.indexOf( ftst ) / 2;          // Format flag
   rfmt         = qMin( rfmt, 5 );
   int rlen     = QString( fmt ).mid( 1 ).toInt();    // Length
   int roff     = ( keyx >= 0 ) ? offs[ keyx ] : -1;

   if ( rfmt < 0  ||  rlen < 1  ||  roff < 0 )
   {
      status       = 901;
      set_error( status );
      error_msg   += key + QString().sprintf( " %d %d %d", rfmt, rlen, roff );
   }

   if ( rfmtP != NULL )  *rfmtP = rfmt;
   if ( rlenP != NULL )  *rlenP = rlen;
   if ( roffP != NULL )  *roffP = roff;

   return status;
}

