//! \file us_time_state.cpp
#include "us_time_state.h"
#include "us_settings.h"

#ifndef _TR_
#define _TR_(a) QObject::tr(a)
#endif
#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

// Constructor of the US_TimeState object
US_TimeState::US_TimeState() : QObject()
{
   filename    = QString( "" );
   filepath    = filename;
   fvers       = QString( _TMST_VERS_ );
   imp_type    = QString( "XLA" );
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
   ss_reso     = 100;
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

   // Modify SetSpeed resolution if appropo debug_text present
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedResolution=" ) )
         ss_reso     = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }

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
   strncpy( cdata + 4, _TMST_VERS_, 3 );  // "2.0"
   cdata[ 5 ]  = cdata[ 6 ];              // '2','0'
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
//DbgLv(1) << "TS: key format" << key << format << "stat errm" << status << error_msg;
      return status;
   }

   int flen     = QString( format ).mid( 1 ).toInt();
   int offset   = rec_size;
   keys << key;
   fmts << format;
   offs << offset;
   nvalues++;
   rec_size     = offset + flen;
//DbgLv(1) << "TS: nval" << nvalues << "key format" << key << format
// << "offs flen" << offset << flen << "rsize" << rec_size;

   return status;
}

// Specify keys and formats for all data fields
int US_TimeState::set_keys( QStringList& fkeys, QStringList& ffmts )
{
   int status  = 0;

   int kvalues = fkeys.count();

   if ( ffmts.count() != kvalues )
   {
      return set_error( 102 );
   }

   for ( int jj = 0; jj < kvalues; jj++ )
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
//DbgLv(1) << "DtsF: sival: key ival" << key << ivalue;
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
//DbgLv(1) << "DtsF: sdval: key dval" << key << dvalue;
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
//DbgLv(1) << "DtsF: ssval: key sval" << key << svalue;
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

            memcpy( cdata + roff, wstr.toLatin1().constData(), rlen );
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
   ntimes++;
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

// Write the definitions (XML) file for the last opened output data file
int US_TimeState::write_defs( double timeinc, QString imptype )
{
   int status  = 0;
   time_inc    = timeinc < 0.0 ? time_inc : timeinc;
   imp_type    = imptype.isEmpty() ? imp_type : imptype;

   QString xfname = QString( filename ).section( ".", 0, -2 ) + ".xml";
   QString xfpath = QString( filepath ).section( ".", 0, -2 ) + ".xml";

   QFile xfo( xfpath );

   if ( !xfo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {  // Error opening the file for write
      status     = 700;
      set_error( status );
      return status;
   }

   QXmlStreamWriter xml( &xfo );
   xml.setAutoFormatting( true );
   xml.writeStartDocument();
   xml.writeDTD              ( "<!DOCTYPE US_TimeState>" );
   xml.writeStartElement( "TimeState" );    // <TimeState version=...>
   xml.writeAttribute   ( "version",        QString( _TMST_VERS_ ) );
   xml.writeAttribute   ( "import_type",    imp_type               );

   xml.writeStartElement( "file" );         // <file time_count=...>
   xml.writeAttribute   ( "time_count",     QString::number( ntimes )     );
   xml.writeAttribute   ( "constant_incr",  ( const_ti ? "1" : "0" )      );
   xml.writeAttribute   ( "time_increment", QString::number( time_inc )   );
   xml.writeAttribute   ( "first_time",     QString::number( time_first ) );
   xml.writeAttribute   ( "ss_reso",        QString::number( ss_reso )    );

   for ( int jj = 0; jj < nvalues; jj++ )
   {  // Key,Format for each value field in the records
      xml.writeStartElement( "value" );     // <value key=...format=.../>
      xml.writeAttribute   ( "key",    keys[ jj ] );
      xml.writeAttribute   ( "format", fmts[ jj ] );
      xml.writeEndElement  ( );
   }

   xml.writeEndElement  ( );                // </file>
   xml.writeEndElement  ( );                // </TimeState>
   xml.writeEndDocument ( );
   xfo.close();

   return status;
}

// Read in TMST data from a file
int US_TimeState::open_read_data( QString fpath, const bool pfetch )
{
   int status  = 0;
   file_size   = (qint64)0;

   filei       = new QFile( fpath );
   pre_fetch   = pfetch;

   if ( ! filei->open( QIODevice::ReadOnly ) )
   {  // Error opening file for read
      status     = 500;
      set_error( status );
      error_msg += fpath;
      return status;
   }

   file_size   = filei->size();
   filepath    = fpath;
   filename    = filepath.section( "/", -1, -1 );
   if ( pre_fetch )
   {  // If pre-fetch, read in all data bytes and close file
      dbytes      = filei->readAll();
      dsi         = new QDataStream( dbytes );
      filei->close();
      filei       = NULL;
   }

   else
   {  // By default, create data stream from opened file
      dsi         = new QDataStream( filei );
   }
   fvers       = QString( _TMST_VERS_ );
   imp_type    = QString( "XLA" );

   rd_open     = true;
   wr_open     = false;
   fhdr_size   = 6;

   // Read in the file header
   dsi->readRawData( cdata, fhdr_size );

   if ( strncmp( cdata, _TMST_MAGI_, 4 ) != 0 )
   {  // Error in magic number (wrong kind of file?)
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
//   if ( strncmp( cdata+6, _TMST_VERS_, 3 ) )
//   {  // Error in version of file
//      status     = 502;
//      set_error( status );
//      error_msg += fvers + " " + QString( cdata+6 ).left( 3 );
//      return status;
//   }

   ntimes      = 0;                     // Initialize counts and size
   nvalues     = 0;
   rec_size    = 0;
   ss_reso     = 100;

   strncpy( cdata,   _TMST_MAGI_, 4 );  // "USTS"
   strncpy( cdata+4, _TMST_VERS_, 3 );  // "2.0"
   cdata[ 5 ]  = cdata[ 6 ];            // '2','0'

   keys.clear();                        // Initialize field attributes
   fmts.clear();
   offs.clear();
   int koff    = 0;

   // Read the associated XML definitions file
   QString xfname = QString( filename ).section( ".", 0, -2 ) + ".xml";
   QString xfpath = QString( filepath ).section( ".", 0, -2 ) + ".xml";

   QFile xfi( xfpath );

   if ( !xfi.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {  // Error opening xml definitions for read
      status     = 505;
      set_error( status );
      return status;
   }

   QXmlStreamReader xml( &xfi );

   while( ! xml.atEnd() )
   {  // Read definition elements
      QString xname;
      QString attv;
      QXmlStreamAttributes attr;
      xml.readNext();

      if ( xml.isStartElement() )
      {
         xname      = xml.name().toString();
         attr       = xml.attributes();

         if ( xname == "TimeState" )
         {  // Parse file/object version and import type
            fvers      = attr.value( "version"        ).toString();
            imp_type   = attr.value( "import_type"    ).toString();
         }

         else if ( xname == "file" )
         {  // Parse overall file attributes
            attv       = attr.value( "time_count"     ).toString();
            ntimes     = attv.isEmpty() ? ntimes     : attv.toInt();

            attv       = attr.value( "constant_incr"  ).toString();
            const_ti   = attv.isEmpty() ? const_ti   : ( attv == "1" );

            attv       = attr.value( "time_increment" ).toString();
            time_inc   = attv.isEmpty() ? time_inc   : attv.toDouble();

            attv       = attr.value( "first_time"     ).toString();
            time_first = attv.isEmpty() ? time_first : attv.toDouble();

            attv       = attr.value( "ss_reso"        ).toString();
            ss_reso    = attv.isEmpty() ? ss_reso    : attv.toInt();
         }

         else if ( xname == "value" )
         {  // Parse the attributes of a record field
            QString ky = attr.value( "key"    ).toString();
            QString fm = attr.value( "format" ).toString();

            keys << ky;                         // Add key to list
            fmts << fm;                         // Add format
            offs << koff;                       // Add offset in record
            koff      += fm.mid( 1 ).toInt();   // Bump offset by value length

            nvalues++;                          // Bump count of values
         }
      }  // End: is-start-element
   }  // End: element loop

   xfi.close();

   rec_size   = koff;                                  // Record size in bytes
   int ktimes = (int)( ( file_size - fhdr_size ) / rec_size ); // Number times
   ntimes     = ( ntimes == 0 ) ? ktimes : ntimes;     // Counted/given times

   return status;
}

// Get the count of time value records
int US_TimeState::time_count()
{
   return ntimes;
}

// Get the set-speed resolution
int US_TimeState::ss_resolution()
{
   return ss_reso;
}

// Get the character and parameters of the time range
int US_TimeState::time_range( bool* constti, double* timeinc, double* ftime )
{
   if ( constti != NULL )  *constti = const_ti;
   if ( timeinc != NULL )  *timeinc = time_inc;
   if ( ftime   != NULL )  *ftime   = time_first;

   return ntimes;
}

// Get definitions version and import type
bool US_TimeState::origin( QString* dversP, QString* itypeP )
{
   bool have_type = false;
   double ddver   = fvers.toDouble();
   QString itype( "(unknown)" );

   if ( ddver > 1.0 )
   {
      itype       = imp_type;
      have_type   = true;
   }

   if ( dversP != NULL )
      *dversP     = fvers;

   if ( itypeP != NULL )
      *itypeP     = itype;

   return have_type;
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
int US_TimeState::read_record( const int rtimex )
{
   int status   = 0;

   if ( timex < 0 )
   {  // If current index indicates still in header, complete header calcs
      int lstv     = nvalues - 1;
      QString fm   = fmts[ lstv ];
      rec_size     = offs[ lstv ] + fm.mid( 1 ).toInt();
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

   else if ( pre_fetch )  // Check if rewindable
   {  // For designated record before current, reposition first
      qint64 fpos  = (qint64)rtimex * rec_size + fhdr_size;
      dsi->device()->seek( fpos );
      timex        = rtimex;
      // Then read the record
      dsi->readRawData( cdata, rec_size );
   }

   else
   {  // Error if designated time index is less than current
      //  and stream is from a disk device (non-rewindable)
      status       = 510;
      return set_error( status );
   }

   return status;
}

// Get a time integer value for a given key from the current record
int US_TimeState::time_ivalue( const QString key, int* stat )
{
   int ivalue  = 0;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   // Fetch attributes of the specified key; point to its record position
   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      char* eval  = cval + rlen;
      double dvalue;

      switch( rfmt )
      {  // Fetch the value in this key's format; get integer output value
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
double US_TimeState::time_dvalue( const QString key, int* stat )
{
   double dvalue  = 0.0;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   // Fetch attributes of the specified key; point to its record position
   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      char  cwrk[ 256 ];
      char* cval  = (char*)cwrk;
      char* eval  = cval + rlen;
      double ivalue;

      switch( rfmt )
      {  // Fetch the value in this key's format; get double output value
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
QString US_TimeState::time_svalue( const QString key, int* stat )
{
   QString svalue;
   int     ivalue;
   double  dvalue;
   int rfmt    = 0;
   int rlen    = 4;
   int roff    = 0;

   // Fetch attributes of the specified key; point to its record position
   int status  = key_parameters( key, &rfmt, &rlen, &roff );
   char* rdata = cdata + roff;

   if ( status == 0 )
   {
      switch( rfmt )
      {  // Fetch the value in this key's format; get string output value
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

   if ( pre_fetch )
      dbytes.clear();                  // Clear data byte array
   else if ( filei != NULL )
      filei->close();                  // Close the input file

   filei       = NULL;                 // Clear the file pointer
   dsi         = NULL;                 // Clear the data stream pointer
   rd_open     = false;                // Flag a closed file


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
   {  // Find a match to the specified error status code
      if ( emap[ ii ].estat == status )
      {  // A status code match is found, so get the corresponding message
         errmsg      = emap[ ii ].emsg;
         break;
      }
   }

//DbgLv(1) << "DtsF: egmsg: status" << status << "message" << errmsg;
   return errmsg;
}

// Get the error message for the last error that occurred
QString US_TimeState::last_error_message( )
{
   return error_msg;
}

// Static function to create a TMST record in the DB from a local file set
int US_TimeState::dbCreate( IUS_DB2* dbP,
                            const int expID, const QString fpath )
{
int dbg_level=US_Settings::us_debug();
   QStringList query;
   int tmstID   = -1;
   if ( dbP == NULL  ||  fpath.isEmpty()  ||  expID < 1 )
      return tmstID;   // Invalid arguments

   QString tmst_fpath = fpath;
   QString defs_fpath = QString( fpath ).section( ".", 0, -2 ) + ".xml";

   QFile dfi( tmst_fpath );
   QFile xfi( defs_fpath );
DbgLv(1) << "dbCreate: tmst_fpath" << tmst_fpath << "exists" << dfi.exists();
DbgLv(1) << "dbCreate: defs_fpath" << defs_fpath << "exists" << xfi.exists();
   if ( !dfi.exists()  ||  !xfi.exists() )
      return -2;       // Local files do not both exist

   if ( ! xfi.open( QIODevice::ReadOnly ) )
      return -3;       // Cannot open definitions file

   // Get attributes of the local files and test for existing DB record
   QString tmst_cksm  = US_Util::md5sum_file( tmst_fpath );
   QString defs_cksm  = US_Util::md5sum_file( defs_fpath );
   QString tmst_fname = QString( fpath ).section( "/", -1, -1 );
DbgLv(1) << "dbCreate: dbP fn ck" << dbP << tmst_fname << tmst_cksm;

   QByteArray defs_da = xfi.readAll();
   xfi.close();
   QString idTmst     = QString::number( tmstID  );
   QString idExp      = QString::number( expID );
   QString fnamedb;
   QString xdefs;
   QString cksumdb;
   QByteArray defs_ld;
   tmstID             = 0;
   int expIDdb        = expID;
 
   int stat           = dbExamine( dbP, &tmstID, &expIDdb, &fnamedb,
                                   &xdefs, &cksumdb );
//DbgLv(1) << "dbCreate:  dbExam stat" << stat << "tmstID expID"
// << tmstID << expIDdb << "cksumdb" << cksumdb;
   if ( stat == IUS_DB2::OK )
   {  // We have an already existing DB record, so check how it matches local
      if ( cksumdb == tmst_cksm )
      {  // Binary data appears to match, so check xml defs
         QByteArray defsdb  = xdefs.toUtf8();
         QString cksumxd    = QString( QCryptographicHash::hash(
                              defsdb, QCryptographicHash::Md5 ).toHex() )
                              + " " + QString::number( defsdb.size() );
 
         if ( tmstID > 0  &&  cksumxd == defs_cksm )
         {  // Xml definitions characters also match, return with existing DB
            if ( fnamedb != tmst_fname )
            {  // If the file name differs, we must update the record
               dbP->mysqlEscapeString( defs_ld, defs_da, defs_da.size() );
               query.clear();
               query << "update_timestate" << idTmst << idExp
                     << tmst_fname << defs_ld;
               dbP->statusQuery( query );
            }

            return tmstID;
         }
      }

      // Not a complete match, so we must delete the DB record and re-create it
      if ( tmstID > 0 )
      {
         dbDelete( dbP, tmstID );
      }
   }

   // Escape definitions xml string, then create a new DB record
   dbP->mysqlEscapeString( defs_ld, defs_da, defs_da.size() );
   query.clear();
   query << "new_timestate" << idExp << tmst_fname << defs_ld;
   stat         = dbP->statusQuery( query );
   int nsrtID   = dbP->lastInsertID();
   tmstID       = ( stat == IUS_DB2::OK  ||  stat == IUS_DB2::NOROWS )
                ? nsrtID : -4;
//DbgLv(1) << "dbCreate:  new_timestate status" << stat
// << "idExp tmstID nsrtID dsiz" << idExp << tmstID << nsrtID << defs_da.size();

   return tmstID;
}

// Static function to delete a TMST record from the DB
int US_TimeState::dbDelete( IUS_DB2* dbP, const int tmstID )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "dbDelete: dbP tmstID" << dbP << tmstID;
   QStringList query;
   query << "delete_timestate" << QString::number( tmstID );
   int status = dbP->statusQuery( query );

   return status;
}

// Static function to examine a TMST record from the DB
int US_TimeState::dbExamine( IUS_DB2* dbP, int* tmstIdP, int* expIdP,
      QString* fnameP, QString* xdefsP, QString* cksumP, QDateTime* lastupdP )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "dbExamine: dbP tmstID expID fname xdefs cksum lastupd"
 << dbP << tmstIdP << expIdP << fnameP << xdefsP << cksumP << lastupdP;
   QStringList query;
   int tmstID    = ( tmstIdP == NULL ) ? 0 : *tmstIdP;
   int expID     = ( expIdP  == NULL ) ? 0 : *expIdP;
   int status    = IUS_DB2::OK;
DbgLv(1) << "dbExamine: tmstID expID" << tmstID << expID;

   if ( tmstID > 0 )
   {  // TimeState ID is given, get experimentID; then fall thru for the rest
      query.clear();
      query << "get_timestate" << QString::number( tmstID );
      dbP->query( query );
      status        = dbP->lastErrno();
DbgLv(1) << "dbExamine:  get_timestate tmstID" << tmstID << "status" << status;
      if ( status != IUS_DB2::OK )
         return status;

//int nrows=dbP->numRows();
      dbP->next();
      expID         = dbP->value( 0 ).toString().toInt();
//DbgLv(1) << "dbExamine:  get_timestate expID" << expID << "nrows" << nrows;
      if ( expIdP != NULL )
         *expIdP       = expID;
   }

   else if ( expID > 0 )
   {  // Experiment ID is given, get timestateID; then fall thru for the rest
//query.clear();
//query << "get_experiment2_timestate" << QString::number( expID );
//dbP->query( query );
//status=dbP->lastErrno();
//DbgLv(1) << "dbExamine:  get_experiment2_timestate expID status" << expID << status;
      query.clear();
      query << "get_experiment_timestate" << QString::number( expID );
      dbP->query( query );
      status        = dbP->lastErrno();
DbgLv(1) << "dbExamine:  get_experiment_timestate expID status" << expID << status;
      if ( status != IUS_DB2::OK )
         return status;

//int nrows=dbP->numRows();
//DbgLv(1) << "dbExamine:  get_experiment_timestate  nrows" << nrows;
      dbP->next();
//bool havenx=dbP->next();
      status        = dbP->lastErrno();
//DbgLv(1) << "dbExamine:  get_experiment_timestate  next status" << status
// << "have_next" << havenx;
      tmstID        = dbP->value( 0 ).toString().toInt();
DbgLv(1) << "dbExamine:  get_experiment_timestate   tmstID" << tmstID
 << dbP->value(0).toString() << dbP->value(1).toString();
      if ( tmstIdP != NULL )
         *tmstIdP      = tmstID;
   }

   else
   {  // Neither tmstID nor expID given
      return IUS_DB2::NO_EXPERIMENT;
   }

   if ( fnameP != NULL )    // Return file name if requested
      *fnameP       = dbP->value( 1 ).toString();

   if ( xdefsP != NULL )    // Return xml definitions string if requested
      *xdefsP       = dbP->value( 2 ).toString();

   if ( cksumP != NULL )    // Return cksum+size string if requested
      *cksumP       = dbP->value( 3 ).toString() + " " +
                      dbP->value( 4 ).toString();
//if(cksumP!=NULL)
//DbgLv(1) << "dbExamine:  cksum-db" << *cksumP;

   if ( lastupdP != NULL )  // Return last-updated datetime if requested
      *lastupdP     = QDateTime::fromString( dbP->value( 5 ).toString(),
                                             Qt::ISODate ).toUTC();

   return status;
}

// Static function to download a TMST binary data record from the DB
int US_TimeState::dbDownload( IUS_DB2* dbP, const int tmstID,
      const QString fpath )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "dbDownload: dbP tmstID fpath" << dbP << tmstID << fpath;
   int status = dbP->readBlobFromDB( fpath, QString( "download_timestate" ),
                                     tmstID );
   return status;
}

// Static function to upload a TMST binary data record to the DB
int US_TimeState::dbUpload( IUS_DB2* dbP, const int tmstID,
      const QString fpath )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "dbUpload: dbP tmstID fpath" << dbP << tmstID << fpath;
   int status = dbP->writeBlobToDB( fpath, QString( "upload_timestate" ),
                                    tmstID );
   return status;
}

// Static function to synchronze a TMST in the DB to a local file
bool US_TimeState::dbSyncToLF( IUS_DB2* dbP, const QString fpath,
                              const int expID )
{
int dbg_level=US_Settings::us_debug();
DbgLv(1) << "dbSyncToLF: dbP fpath expID" << dbP << fpath << expID;
   bool newfile   = false;

   // Not possible to create a new file unless we are using DB
   if ( dbP == NULL )
      return newfile;

   // Get the state and size of the DB record
   int tmstID     = 0;
   int expID1     = expID;
   QString fpath1 = fpath;
   QString xdefs;
   QString cksumD;
   QString cksumF;
   QDateTime drecUpd;
   QDateTime frecUpd;

DbgLv(1) << "dbSTLF: call dbExamine";
   int statD  = dbExamine( dbP, &tmstID, &expID1, &fpath1, &xdefs, &cksumD,
                           &drecUpd );
DbgLv(1) << "dbSTLF: dbExamine return";
DbgLv(1) << "dbSTLF: dbExamine statD=" << statD << "tmstID=" << tmstID
 << "cksumD" << cksumD << "drecUpd" << drecUpd.toString();
   
   // Not possible to create a new file if DB record is bad or missing
   if ( statD != IUS_DB2::OK  )
      return newfile;

   // Now check any existing TimeState file
   bool needF = false;

   if ( QFile( fpath ).exists() )
   {  // The file exists, so we need to check its cksum and size
      cksumF     = US_Util::md5sum_file( fpath );
DbgLv(1) << "dbSTLF: LF exists : cksumF" << cksumF;

      if ( cksumF == cksumD )
      {  // DB,Local records match in checksum and size, so no download
         return newfile;
      }

      
      // We need to bring Db,Local in sync
      int sizeD   = QString( cksumD ).section( " ", -1, -1 ).toInt();
      int sizeF   = QString( cksumF ).section( " ", -1, -1 ).toInt();
      frecUpd     = QFileInfo( fpath ).lastModified().toUTC();
DbgLv(1) << "dbSTLF: sizeD sizeF" << sizeD << sizeF << "frecUpd"
 << frecUpd.toString();
      // Flag file update if DB record newer than local or larger
      needF       = ( drecUpd > frecUpd   ||  sizeD > sizeF );
   }

   else
   {  // The local file does not exist, so we need to create it
      needF       = true;
   }
DbgLv(1) << "dbSTLF:  NEEDF=" << needF;

   if ( needF )
   {  // We need a (new) local file copy
      int stat    = dbDownload( dbP, tmstID, fpath );
      newfile     = ( stat == IUS_DB2::OK );
      QString xfpath = QString( fpath ).replace( ".tmst", ".xml" );

      // Write the xdefs xml sibling file
      QFile fileo( xfpath );

      if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text) )
      {     
         QTextStream tso( &fileo );
         tso << xdefs;
         tso.flush();
         fileo.close();
      }
   }

   return newfile;
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
int US_TimeState::key_parameters( const QString key,
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
DbgLv(1) << "DtsF:       message" << error_msg;
DbgLv(1) << "DtsF: keyx" << keyx << "fmt" << fmt << "ftst" << ftst
 << "keys sz" << keys.count() << "fmts sz" << fmts.count();
   }

   if ( rfmtP != NULL )  *rfmtP = rfmt;
   if ( rlenP != NULL )  *rlenP = rlen;
   if ( roffP != NULL )  *roffP = roff;

   return status;
}

