/*  This is a highly customized version of GNU Gzip.  It has 
 *  been converted to Qt and C++. 
 *  Assumptions include that only a regular file is passed.
 *  Compression is always -9.
 *
 *  Since this file is derived from a GPLed application, this file is
 *  also licensed under the GPL.
 *
 *  Bruce Dubbs
 *  Univerity of Texas Health Science Center
*/

#include <qfileinfo.h> 
#include <qdatastream.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>

#ifdef WIN32
#   include <io.h>
#   include <sys/utime.h>
#   include <sys/stat.h>
#   include <qdatetime.h>
#   define utime   _utime
#   define open    _open
#   define read    _read
#   define write   _write
#   define close   _close
#   define fstat   _fstat
#   define stat    _stat
#   define utimbuf _utimbuf
// The following is for Qt4 
//#   define ascii   toAscii  
#else
#  include <utime.h>
#  include <sys/stat.h>  // For OSX
#  define O_BINARY 0
#endif

#include <iostream>
using namespace std;

#include "../include/us_gzip.h"

#ifdef WIN32
#  define ssize_t long
#  define bzero(p, size) (void)memset((p), 0, (size))
#endif


namespace gzip_data
{
/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by makecrc.c)  */
  ulg crc_32_tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
  };

    /* Tables for deflate from PKZIP's appnote.txt. */
    unsigned border[] =     /* Order of the bit length code lengths */
    {
      16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
          
    ush cplens[] =          /* Copy lengths for literal codes 257..285 */
    {
      3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
      35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
    };
      /* note: see note #13 above about the 258 in this list. */
    
    ush cplext[]=          /* Extra bits for literal codes 257..285 */
    {
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
      3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99 /* 99==invalid */
    };

    ush cpdist[] =          /* Copy offsets for distance codes 0..29 */
    {
      1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
      257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
      8193, 12289, 16385, 24577
    };
      
    ush cpdext[] =           /* Extra bits for distance codes */
    {
      0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
      7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
      12, 12, 13, 13
    };

    ush mask_bits[] = 
    {
      0x0000,
      0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
      0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
    };

    typedef struct config 
    {
      ush good_length; /* reduce lazy search above this match length */
      ush max_lazy;    /* do not perform lazy search above this match length */
      ush nice_length; /* quit search above this match length */
      ush max_chain;
    } config;

    config configuration_table[] = 
    {
      /*      good lazy nice chain */
      /* 0 */ {0,    0,  0,    0},  /* store only */
      /* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
      /* 2 */ {4,    5, 16,    8},
      /* 3 */ {4,    6, 32,   32},
      /* 4 */ {4,    4, 16,   16},  /* lazy matches */
      /* 5 */ {8,   16, 32,   32},
      /* 6 */ {8,   16, 128, 128},
      /* 7 */ {8,   32, 128, 256},
      /* 8 */ {32, 128, 258, 1024},
      /* 9 */ {32, 258, 258, 4096} /* maximum compression */
    };

    int extra_lbits[] = /* extra bits for each length code */
    {
      0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
    };

    int extra_dbits[] = /* extra bits for each distance code */
    {
      0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
    };

    int extra_blbits[] = /* extra bits for each bit length code */
    {
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7
    };

    unsigned char bl_order[] = 
    {
      16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
    };
    /* The lengths of the bit length codes are sent in order of decreasing
     * probability, to avoid transmitting the lengths for unused bit length codes.
     */

}

using namespace gzip_data;

US_Gzip::US_Gzip()
{
  static_dtree[ 0 ].GZLen = 0;
}

int US_Gzip::gzip( const QString& filename )
{
   bytes_out = 0;
  return treat_file( filename, false );
}

int US_Gzip::gunzip( const QString& filename )
{
   bytes_out = 0;
  return treat_file( filename, true );
}

int US_Gzip::treat_file( const QString& iname, bool decompress )
{
  //unsigned int crc;
  time_t       filetime;
  QFileInfo    filename( iname );

  // Check if the input file is present and valid 
  if ( ! filename.exists() )     return GZIP_NOEXIST;
  if ( ! filename.isFile() )     return GZIP_NOTFILE;
  if (   filename.isSymLink() )  return GZIP_NOTFILE;
  if ( ! filename.isReadable() ) return GZIP_NOREAD;

  // Get the times and other data
  // QDateTime    lastRead   = filename.lastRead();
  QDateTime    lastMod    = filename.lastModified();
  
  filetime = lastMod.toTime_t();

  ifd = open( iname.toLatin1().data(), O_RDONLY | O_BINARY );
  if ( ifd < 0 ) return GZIP_READERROR;

  // Generate output file name. 
  QString oname = make_ofname( iname, decompress );

  // cout << "us_gzip: oname is :" << oname << ":\n";

  if ( oname == "" ) return GZIP_NOTGZ;

  // Open the input file and determine decompression method.

  if ( decompress )
  {
    ssize_t count;

    // Two byte signature
    char signature[2];
    count = read( ifd, signature, 2 );
    
    if ( count != 2 )
    {
      close( ifd );
      return GZIP_READERROR;
    }

    if ( signature[0] != (char) 0x1f  || 
         signature[1] != (char) 0x8b ) return GZIP_NOTGZ;

    // One byte method.  Only 0x08, deflate, is supported
    char method;
    count = read( ifd, &method, 1 );

    if ( count != 1 )
    {
      close( ifd );
      return GZIP_READERROR;
    }
    
    // One byte flags.  00111111.  Only bit 3 ( file name present ) is supported
    // Bit 0 is ignored.  Bits 1,2,4,5 (multi-part, extra field, comment,
    // encrypyion) are flagged as unsupported.

    char flags;
    count = read( ifd, &flags, 1 );

    if ( count != 1 )
    {
      close( ifd );
      return GZIP_READERROR;
    }

    if ( flags & 0x36 ) return GZIP_OPTIONNOTSUPPORTED;

    // Four bytes.  File modification time in Unix format.
    count = read( ifd, (char*) &filetime, 4 );

    if ( count != 4 )
    {
      close( ifd );
      return GZIP_READERROR;
    }

    char timestring[40];
#ifdef WIN32
    //ctime_s( timestring, sizeof( timestring ),  &filetime );
    /* int stringCount = */ sprintf( timestring, "%s", ctime( &filetime ) );
#else
    ctime_r( &filetime, timestring );
#endif

    timestring[24] = 0;

    // One byte.   Extra flags (depend on compression method).
    // FAST = 4 ( -1 compression ) or SLOW = 2 ( -9 compression )

    char extra;
    // QString extraString;

    count = read( ifd, &extra, 1 );
    if ( count != 1 )
    {
      close( ifd );
      return GZIP_READERROR;
    }

    // One byte OS.  0x03 = UNIX, 0x0b = WIN32
    // This parameter can be ignored.

    char OS;
    count = read( ifd, &OS, 1 );
    
    if ( count != 1 )
    {
      close( ifd );
      return GZIP_READERROR;
    }

    // Variable bytes  Optional original file name, zero terminated.

    QString embedded_name( "" );
    char    c;

    if ( flags & 0x08 ) // Filename present
    {
      while ( true )
      {
        count = read( ifd, &c, 1 );
        
        if ( count != 1 )
        {
          close( ifd );
          return GZIP_READERROR;
        }

        if ( c == 0 ) break;
        embedded_name.append( c );
      } 

      oname = embedded_name;
      // cout << "us_gzip: embedded oname makes oname now:" << oname << ":\n";
      
    }

    // Open the output file but check that it doesn't exist first
    QFileInfo output_file( oname );
    last_written_name = oname;
    
    if ( output_file.exists() ) return GZIP_OUTFILEEXISTS;

    ofd    = open( oname.toLatin1().data(), O_CREAT | O_WRONLY | O_BINARY , 0664 );
    outcnt = 0;

    updcrc( NULL, 0 );           /* initialize crc */

    // Expand compressd data
    try
    {
      inflate();
    }
    catch ( int inflate_error )
    {
      close( ifd );
      close( ofd );
      QFile::remove( oname.toLatin1().data() );
      return inflate_error;
    }

#define wp outcnt
#define GETBYTE() (inptr < insize ? inbuf[inptr++] : (wp = w, fill_inbuf(0)))
    unsigned w = 0;  // Dummy to make define above work

    // CRC
    unsigned char buf[ 4 ];

    for ( int i = 0; i < 4; i++ ) buf[ i ] = GETBYTE();
    
    crc = buf[ 3 ] << 24 | buf[ 2 ] << 16 | buf[ 1 ] << 8 | buf[ 0 ];

    /* Validate decompression */
    if ( crc != updcrc(outbuf, 0) ) 
    {
      close( ifd );
      close( ofd );
      QFile::remove( oname.toLatin1().data() );
      return GZIP_CRCERROR;
    }

    // uncompressed input size modulo 2^32

    for ( int i = 0; i < 4; i++ ) buf[ i ] = GETBYTE();

    ulg size = buf[ 3 ] << 24 | buf[ 2 ] << 16 | buf[ 1 ] << 8 | buf[ 0 ];

    if ( size != (unsigned int) bytes_out )
    {
      close( ifd );
      close( ofd );
      QFile::remove( oname.toLatin1().data() );
      return GZIP_LENGTHERROR;
    }
  }
//////////////////////////////////////////////////////
  else  // compress the input file
  {
    // Check output file -- exists? writable?
    QFileInfo filename( oname );
////    if ( filename.exists() ) return GZIP_OUTFILEEXISTS;
    last_written_name = oname;

    ofd = open( oname.toLatin1().data(), O_CREAT | O_WRONLY | O_BINARY, 0664 );
    if ( ofd < 0 ) return GZIP_WRITEERROR;

    outcnt   = 0;
    bytes_in = 0;

#define flush_output(w) (wp=(w),flush_window())

#define put_byte(c) \
    { \
      outbuf[ outcnt++ ] = (uch)(c); \
      if ( outcnt == OUTBUFSIZ ) flush_outbuf();\
    }

/* Output a 16 bit value, lsb first */
#define put_short(w) \
    { \
    if ( outcnt < OUTBUFSIZ-2 ) \
    { \
      outbuf[ outcnt++ ] = (uch) ((w) & 0xff); \
      outbuf[ outcnt++ ] = (uch) ((ush)(w) >> 8); \
    } \
    else \
    { \
      put_byte( (uch) ( (w) & 0xff) ); \
      put_byte( (uch) ( (ush) (w) >> 8 ) ); \
     } \
    }

/* Output a 32 bit value to the bit stream, lsb first */
#define put_long(n) \
    { \
      put_short( (n) & 0xffff ); \
      put_short( ( (ulg) (n) ) >> 16) ; \
    }

#define GZIP_MAGIC  "\037\213" /* Magic header for gzip files, 1F 8B */
#define DEFLATED    8
#define ORIG_NAME   0x08       /* bit 3 set: original file name present */

    try
    {
      /* Write the header to the gzip file. See algorithm.doc for the format */
      put_byte( GZIP_MAGIC[0] ); /* magic header */
      put_byte( GZIP_MAGIC[1] );
      put_byte( DEFLATED );      /* compression method */

      uch flags = ORIG_NAME;     /* general purpose bit flags */
      put_byte( flags );         /* general flags */

      /* original time stamp (modification time) */
      time_t time_stamp = lastMod.toTime_t();  
      put_long( (ulg) time_stamp == ( time_stamp & 0xffffffff ) ? 
          (ulg) time_stamp : (ulg) 0);

#define SLOW 2
      uch deflate_flags = SLOW;  // Compression level 9
      /* Write deflated file to zip file */
      put_byte( (uch) deflate_flags ); /* extra flags */

#ifdef WIN32
#  define OS_CODE 0x0b 
#else
#  define OS_CODE 0x03
#endif

      put_byte( OS_CODE );                  /* OS identifier */

      if ( strlen( iname.toLatin1().data() ) > 255 ) return GZIP_FILENAMEERROR;
      char f[256];
      strcpy( f, iname.toLatin1().data() );
      char* p = base_name( f ); /* Don't save the directory part. */
      do { put_byte( *p ); } while ( *p++ );
                        
      crc = updcrc( 0, 0 );

      bi_init();
      ct_init();
      lm_init();
      deflate();
    
      /* Write the crc and uncompressed size */
      put_long( crc );
      put_long( (ulg) bytes_in );
    }
    catch ( int error )
    {
      close( ifd );
      close( ofd );
      return error;
    }

    flush_outbuf();
  }

  // Get input file data
  struct stat ifstat;
  fstat( ifd, &ifstat );

  // Close files
  close( ifd );
  close( ofd );

  // Set the permissions 
  chmod( oname.toLatin1().data(), ifstat.st_mode & 07777 );

#ifndef WIN32
  // Change the ownership (may fail if not root)
  chown( oname.toLatin1().data(), ifstat.st_uid, ifstat.st_gid );
#endif

  // Reset oname metadata to ifile metadata
  
  struct utimbuf timep;

  timep.actime  = ifstat.st_atime;
  timep.modtime = filetime;
  utime( oname.toLatin1().data(), &timep );
  // Now delete the input file
  QFile::remove( iname.toLatin1().data() );

  return GZIP_OK;
}

/* ========================================================================
 *  Generate ofname given filename. 
*/

QString US_Gzip::make_ofname( const QString& filename, bool decompress )
{
    QString ofile = filename;
    
    if ( decompress ) 
    {
      // Get the gzip suffix if present
      if ( filename.right( 3 ) != ".gz" ) return QString( "" ); 

      // Strip the .gz
      // oname might be changed later if infile contains an original name
      return ofile.left( ofile.length() - 3 );
    } 

    // Compress
    return ofile + ".gz";
}

/////////////////////////////////////
/* Inflate deflated data

   Copyright (C) 1997, 1998, 1999, 2002 Free Software Foundation, Inc.
   Updated for c++ and qt by Bruce Dubbs - 2008
*/

/* Not copyrighted 1992 by Mark Adler
   version c10p1, 10 January 1993 */

/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.
   [The history has been moved to the file ChangeLog.]
 */

/*
   Inflate deflated (PKZIP's method 8 compressed) data.  The compression
   method searches for as much of the current string of bytes (up to a
   length of 258) in the previous 32K bytes.  If it doesn't find any
   matches (of at least length 3), it codes the next byte.  Otherwise, it
   codes the length of the matched string and its distance backwards from
   the current position.  There is a single Huffman code that codes both
   single bytes (called "literals") and match lengths.  A second Huffman
   code codes the distance information, which follows a length code.  Each
   length or distance code actually represents a base value and a number
   of "extra" (sometimes zero) bits to get to add to the base value.  At
   the end of each deflated block is a special end-of-block (EOB) literal/
   length code.  The decoding process is basically: get a literal/length
   code; if EOB then done; if a literal, emit the decoded byte; if a
   length then get the distance and emit the referred-to bytes from the
   sliding window of previously emitted data.

   There are (currently) three kinds of inflate blocks: stored, fixed, and
   dynamic.  The compressor deals with some chunk of data at a time, and
   decides which method to use on a chunk-by-chunk basis.  A chunk might
   typically be 32K or 64K.  If the chunk is uncompressible, then the
   "stored" method is used.  In this case, the bytes are simply stored as
   is, eight bits per byte, with none of the above coding.  The bytes are
   preceded by a count, since there is no longer an EOB code.

   If the data is compressible, then either the fixed or dynamic methods
   are used.  In the dynamic method, the compressed data is preceded by
   an encoding of the literal/length and distance Huffman codes that are
   to be used to decode this block.  The representation is itself Huffman
   coded, and so is preceded by a description of that code.  These code
   descriptions take up a little space, and so for small blocks, there is
   a predefined set of codes, called the fixed codes.  The fixed method is
   used if the block codes up smaller that way (usually for quite small
   chunks), otherwise the dynamic method is used.  In the latter case, the
   codes are customized to the probabilities in the current block, and so
   can code it much better than the pre-determined fixed codes.
 
   The Huffman codes themselves are decoded using a mutli-level table
   lookup, in order to maximize the speed of decoding plus the speed of
   building the decoding tables.  See the comments below that precede the
   lbits and dbits tuning parameters.
 */


/*
   Notes beyond the 1.93a appnote.txt:

   1. Distance pointers never point before the beginning of the output
      stream.
   2. Distance pointers can point back across blocks, up to 32k away.
   3. There is an implied maximum of 7 bits for the bit length table and
      15 bits for the actual data.
   4. If only one code exists, then it is encoded using one bit.  (Zero
      would be more efficient, but perhaps a little confusing.)  If two
      codes exist, they are coded using one bit each (0 and 1).
   5. There is no way of sending zero distance codes--a dummy must be
      sent if there are none.  (History: a pre 2.0 version of PKZIP would
      store blocks with no distance codes, but this was discovered to be
      too harsh a criterion.)  Valid only for 1.93a.  2.04c does allow
      zero distance codes, which is sent as one code of zero bits in
      length.
   6. There are up to 286 literal/length codes.  Code 256 represents the
      end-of-block.  Note however that the static length tree defines
      288 codes just to fill out the Huffman codes.  Codes 286 and 287
      cannot be used though, since there is no length base or extra bits
      defined for them.  Similarly, there are up to 30 distance codes.
      However, static trees define 32 codes (all 5 bits) to fill out the
      Huffman codes, but the last two had better not show up in the data.
   7. Unzip can check dynamic Huffman blocks for complete code sets.
      The exception is that a single code would not be complete (see #4).
   8. The five bits following the block type is really the number of
      literal codes sent minus 257.
   9. Length codes 8,16,16 are interpreted as 13 length codes of 8 bits
      (1+6+6).  Therefore, to output three times the length, you output
      three codes (1+1+1), whereas to output four times the same length,
      you only need two codes (1+3).  Hmm.
  10. In the tree reconstruction algorithm, Code = Code + Increment
      only if BitLength(i) is not zero.  (Pretty obvious.)
  11. Correction: 4 Bits: # of Bit Length codes - 4     (4 - 19)
  12. Note: length code 284 can represent 227-258, but length code 285
      really is 258.  The last length deserves its own, short code
      since it gets used a lot in very redundant files.  The length
      258 is special since 258 - 3 (the min match length) is 255.
  13. The literal/length and distance code bit lengths are read as a
      single stream of lengths.  It is possible (and advantageous) for
      a repeat code (16, 17, or 18) to go across the boundary between
      the two sets of lengths.
 */

#define slide window

/* The inflate algorithm uses a sliding 32K byte window on the uncompressed
   stream to find repeated byte strings.  This is implemented here as a
   circular buffer.  The index is updated simply by incrementing and then
   and'ing with 0x7fff (32K-1). */
/* It is left to other modules to supply the 32K area.  It is assumed
   to be usable as if it were declared "uch slide[32768];" or as just
   "uch *slide;" and then malloc'ed in the latter case.  The definition
   must be in unzip.h, included above. */
/* unsigned wp;             current position in slide */


/* Macros for inflate() bit peeking and grabbing.
   The usage is:
   
        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed, and are initialized at the beginning of a
   routine that uses these macros from a global bit buffer and count.
   The macros also use the variable w, which is a cached copy of wp.

   If we assume that EOB will be the longest code, then we will never
   ask for bits with NEEDBITS that are beyond the end of the stream.
   So, NEEDBITS should not read any more bytes than are needed to
   meet the request.  Then no bytes need to be "returned" to the buffer
   at the end of the last block.

   However, this assumption is not true for fixed blocks--the EOB code
   is 7 bits, but the other literal/length codes can be 8 or 9 bits.
   (The EOB code is shorter than other codes because fixed blocks are
   generally short.  So, while a block always has an EOB, many other
   literal/length codes have a significantly lower probability of
   showing up at all.)  However, by making the first table have a
   lookup of seven bits, the EOB code will be found in that first
   lookup, and so will not require that too many bits be pulled from
   the stream.
 */


//#ifdef CRYPT
//  uch cc;
/*#  define NEXTBYTE() \
     (decrypt ? (cc = GETBYTE(), zdecode(cc), cc) : GETBYTE())
*/
//#else
#  define NEXTBYTE()  (uch)GETBYTE()
//#endif

#define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}

/*
#define NEEDBITS(n) 
{
  while ( k < (n) )
  {
    b |= ( (ulg)NEXTBYTE()) << k;
    k += 8;
  }
}
 
*/

/*
#define NEEDBITS(n) 
{
  while ( k < (n) )
  { 
    b |= ( (ulg) (uch)( inptr < insize   ? 
                        inbuf[ inptr++ ] : 
                        ( wp = w, fill_inbuf( 0 ) ) ) ) << k;
    k += 8;
  }
}

*/

#define DUMPBITS(n) {b>>=(n);k-=(n);}

/*
 
#define DUMPBITS(n) 
{
  b >>= (n);
  k  -= (n);
}

*/


/* Huffman code decoding is performed using a multi-level table lookup.
   The fastest way to decode is to simply build a lookup table whose
   size is determined by the longest code.  However, the time it takes
   to build this table can also be a factor if the data being decoded
   is not very long.  The most common codes are necessarily the
   shortest codes, so those codes dominate the decoding time, and hence
   the speed.  The idea is you can have a shorter table that decodes the
   shorter, more probable codes, and then point to subsidiary tables for
   the longer codes.  The time it costs to decode the longer codes is
   then traded against the time it takes to make longer tables.

   This results of this trade are in the variables lbits and dbits
   below.  lbits is the number of bits the first level table for literal/
   length codes can decode in one step, and dbits is the same thing for
   the distance codes.  Subsequent tables are also less than or equal to
   those sizes.  These values may be adjusted either when all of the
   codes are shorter than that, in which case the longest code length in
   bits is used, or when the shortest code is *longer* than the requested
   table size, in which case the length of the shortest code in bits is
   used.

   There are two different values for the two tables, since they code a
   different number of possibilities each.  The literal/length table
   codes 286 possible values, or in a flat code, a little over eight
   bits.  The distance table codes 30 possible values, or a little less
   than five bits, flat.  The optimum values for speed end up being
   about one bit more than those, so lbits is 8+1 and dbits is 5+1.
   The optimum values may differ though from machine to machine, and
   possibly even between compilers.  Your mileage may vary.
 */


//int lbits = 9;          /* bits in base literal/length lookup table */
#define lbits 9
//int dbits = 6;          /* bits in base distance lookup table */
#define dbits 6

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX   16       /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */


//unsigned hufts;         /* track memory usage */

#define memzero(s, n)  bzero((s), (n))

int US_Gzip::huft_build(
  unsigned*     b,        /* code lengths in bits (all assumed <= BMAX) */
  unsigned      n,        /* number of codes (assumed <= N_MAX) */
  unsigned      s,        /* number of simple-valued codes (0..s-1) */
  ush*          d,        /* list of base values for non-simple codes */
  ush*          e,        /* list of extra bits for non-simple codes */
  struct huft** t,        /* result: starting table */
  int*          m )       /* maximum lookup bits, returns actual */

/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
  unsigned              a;         /* counter for codes of length k */
  unsigned              c[BMAX+1]; /* bit length count table */
  unsigned              f;         /* i repeats in table every f entries */
  int                   g;         /* maximum code length */
  int                   h;         /* table level */
  /* register */ unsigned     i;         /* counter, current code */
  /* register */ unsigned     j;         /* counter */
  /* register */ int          k;         /* number of bits in current code */
  int                   l;         /* bits per table (returned in m) */
  /* register */ unsigned*    p;         /* pointer into c[], b[], or v[] */
  /* register */ struct huft* q;         /* points to current table */
  struct huft           r;         /* table entry for structure assignment */
  struct huft*          u[BMAX];   /* table stack */
  unsigned              v[N_MAX];  /* values in order of bit length */
  /* register */ int          w;         /* bits before this table == (l * h) */
  unsigned              x[BMAX+1]; /* bit offsets, then code stack */
  unsigned*             xp;        /* pointer into x */
  int                   y;         /* number of dummy codes added */
  unsigned              z;         /* number of entries in current table */


  /* Generate counts for each bit length */
  memzero( c, sizeof(c) );
  p = b;  
  i = n;
  
  do 
  {
    //Tracecv(*p, (stderr, (n-i >= ' ' && n-i <= '~' ? "%c %d\n" :"0x%x %d\n"), 
    //  n-i, *p));
    c[*p]++;                  /* assume all entries <= BMAX */
    p++;                      /* Can't combine with above line (Solaris bug) */
  } while ( --i );
  
  if ( c[0] == n )            /* null input--all zero length codes */
  {
    *t = (struct huft*) NULL;
    *m = 0;
    return 0;
  }


  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  
  for ( j = 1; j <= BMAX; j++ )
  {
    if ( c[j] ) break;
  }

  k = j;                        /* minimum code length */
  
  if ( (unsigned)l < j ) l = j;

  for ( i = BMAX; i; i-- )
  {
    if ( c[i] )  break;
  }
  
  g = i;                        /* maximum code length */
  
  if ((unsigned)l > i) l = i;
  
  *m = l;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
  {
    if ( (y -= c[j]) < 0 ) return 2;      /* bad input: more codes than bits */
  }
  
  if ( ( y -= c[i] ) < 0 ) return 2;
  
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = 0;
  j    = 0;
  p    = c + 1;  
  xp   = x + 2;
  
  while ( --i ) 
  {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }

  /* Make a table of values in order of bit lengths */
  p = b;  
  i = 0;
  
  do 
  {
    if ( (j = *p++) != 0 ) v[x[j]++] = i;
  } while ( ++i < n );
  
  n = x[g];                   /* set n to length of v */


  /* Generate the Huffman codes and for each, make the table entries */
  
  x[0] = 0;
  i    = 0;                     /* first Huffman code is zero */
  p    = v;                     /* grab values in bit order */
  h    = -1;                    /* no tables yet--level -1 */
  w    = -l;                    /* bits decoded == (l * h) */
  u[0] = (struct huft*) NULL;   /* just to keep compilers happy */
  q    = (struct huft*) NULL;   /* ditto */
  z    = 0;                     /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for ( ; k <= g; k++ )
  {
    a = c[k];
    while ( a-- )
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while ( k > w + l )
      {
        h++;
        w += l;                 /* previous table always l bits */

        /* compute minimum size table less than or equal to l bits */
        z = (z = g - w) > (unsigned)l ? l : z;  /* upper limit on table size */
        
        if ( (f = 1 << (j = k - w)) > a + 1 )     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          if ( j < z )
          {
             while ( ++j < z )   /* try smaller tables up to z bits */
             {
               if ( (f <<= 1) <= *++xp )
               {
                 break;            /* enough codes to use up j bits */
               }
        
               f -= *xp;           /* else deduct codes from patterns */
             }
          }
        }
        
        z = 1 << j;             /* table entries for j-bit table */

        /* allocate and link in new table */
        if ( (q = (struct huft*) malloc( (z + 1) * sizeof(struct huft)) ) ==
             (struct huft *) NULL )
        {
          if ( h ) huft_free( u[0] );
          return 3;             /* not enough memory */
        }
      
        hufts           += z + 1;            /* track memory usage */
        *t               = q + 1;            /* link to list for huft_free() */
        *(t = &(q->v.t)) = (struct huft *)NULL;
        u[h]             = ++q;                 /* table starts after link */

        /* connect to last table, if there is one */
        if ( h )
        {
          x[h]      = i;             /* save pattern for backing up */
          r.b       = (uch)l;        /* bits to dump before this table */
          r.e       = (uch)(16 + j); /* bits in this table */
          r.v.t     = q;             /* pointer to this table */
          j         = i >> (w - l);  /* (get around Turbo C bug) */
          u[h-1][j] = r;             /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);

      if (p >= v + n)
      {
        r.e = 99;               /* out of values--invalid code */
      }
      else if (*p < s)
      {
        r.e = (uch)(*p < 256 ? 16 : 15);  /* 256 is end-of-block code */
        r.v.n = (ush)(*p);                /* simple code is just the value */
        p++;                              /* one compiler does not like *p++ */
      }
      else
      {
        r.e   = (uch) e[*p - s];   /* non-simple--look up in lists */
        r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      
      for ( j = i >> w; j < z; j += f )
      {
        q[j] = r;
      }

      /* backwards increment the k-bit code i */
      for ( j = 1 << (k - 1); i & j; j >>= 1 ) 
      {
        i ^= j;
      }
      
      i ^= j;

      /* backup over finished tables */
      while ( (i & ((1 << w) - 1)) != x[h] )
      {
        h--;                    /* don't need to update q */
        w -= l;
      }
    }
  }

  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}

int US_Gzip::huft_free( struct huft* t )       /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  /* register */ struct huft* p;
  /* register */ struct huft* q;

  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  
  while ( p != (struct huft*) NULL )
  {
    q = (--p)->v.t;
    free( (char*) p );
    p = q;
  } 

  return 0;
}


int US_Gzip::inflate_codes(
  struct huft* tl,
  struct huft* td,   /* literal/length and distance decoder tables */
  int    bl,
  int    bd          /* number of bits decoded by tl[] and td[] */
  )
/* inflate (decompress) the codes in a deflated (compressed) block.
   Return an error code or zero if it all goes ok. */
{
  /* register */ unsigned e;  /* table entry flag/number of extra bits */
  unsigned          n;
  unsigned          d;  /* length and index for copy */
  unsigned          w;  /* current window position */
  struct huft*      t;  /* pointer to table entry */
  unsigned          ml;
  unsigned          md; /* masks for bl and bd bits */
  /* register */ ulg      b;  /* bit buffer */
  /* register */ unsigned k;  /* number of bits in bit buffer */

  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */

  /* inflate the coded data */
  ml = mask_bits [ bl ];           /* precompute masks for speed */
  md = mask_bits [ bd ];
  
  for ( ; ; )                   /* do until end of block */
  {
    NEEDBITS( (unsigned)bl )
    
    if ((e = (t = tl + ((unsigned)b & ml))->e) > 16)
    {
      do 
      {
        if (e == 99) return 1;
        DUMPBITS( t->b )
        e -= 16;
        NEEDBITS( e )
      } while ( (e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16 );
    }

    DUMPBITS(t->b)
    
      if (e == 16)                /* then it's a literal */
    {
      slide[w++] = (uch)t->v.n;
      //Tracevv((stderr, "%c", slide[w-1]));
      
      if (w == WSIZE)
      {
        flush_output(w);
        w = 0;
      }
    }
    else                        /* it's an EOB or a length */
    {
      /* exit if end of block */
      if (e == 15) break;

      /* get length of block to copy */
      NEEDBITS( e )
      n = t->v.n + ( (unsigned)b & mask_bits[e] );
      DUMPBITS( e );

      /* decode distance of block to copy */
      NEEDBITS( (unsigned) bd )
      
      if ( (e = (t = td + ((unsigned)b & md))->e) > 16 )
      {
        do 
        {
          if ( e == 99 ) return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ( (e = (t = t->v.t + ((unsigned)b & mask_bits[e]))->e) > 16 );
      }

      DUMPBITS( t->b )
      NEEDBITS( e )
      d = w - t->v.n - ( (unsigned)b & mask_bits[e] );
      DUMPBITS( e )
      //Tracevv((stderr,"\\[%d,%d]", w-d, n));

      /* do the copy */
      do 
      {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);

        if (w - d >= e)         /* (this test assumes unsigned comparison) */
        {
          memcpy( slide + w, slide + d, e );
          w += e;
          d += e;
        }
        else      /* do it slow to avoid memcpy() overlap */
        {
          do 
          {
            slide[w++] = slide[d++];
            //Tracevv((stderr, "%c", slide[w-1]));
          } while (--e);
  }

        if ( w == WSIZE )
        {
          flush_output(w);
          w = 0;
        }
      } while ( n );

    }
  }

  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;

  /* done */
  return 0;
}



int US_Gzip::inflate_stored()
/* "decompress" an inflated type 0 (stored) block. */
{
  unsigned          n;  /* number of bytes in block */
  unsigned          w;  /* current window position */
  /* register */ ulg      b;  /* bit buffer */
  /* register */ unsigned k;  /* number of bits in bit buffer */


  /* make local copies of globals */
  b = bb;                       /* initialize bit buffer */
  k = bk;
  w = wp;                       /* initialize window position */


  /* go to byte boundary */
  n = k & 7;
  DUMPBITS( n );


  /* get the length and its complement */
  NEEDBITS( 16 )
  n = ( (unsigned) b & 0xffff );
  DUMPBITS( 16 )

  NEEDBITS( 16 )
  if ( n != (unsigned)( (~b) & 0xffff ) )
  {
    return 1;                   /* error in compressed data */
  }
  DUMPBITS(16)


  /* read and output the compressed data */
  while ( n-- )
  {
    NEEDBITS( 8 )
    slide [ w++ ] = (uch) b;
    if ( w == WSIZE )
    {
      flush_output( w );
      w = 0;
    }
    DUMPBITS( 8 )
  }

  /* restore the globals from the locals */
  wp = w;                       /* restore global window pointer */
  bb = b;                       /* restore global bit buffer */
  bk = k;
  return 0;
}



int US_Gzip::inflate_fixed()
/* decompress an inflated type 1 (fixed Huffman codes) block.  We should
   either replace this with a custom decoder, or at least precompute the
   Huffman tables. */
{
  int          i;       /* temporary variable */
  struct huft* tl;      /* literal/length code table */
  struct huft* td;      /* distance code table */
  int          bl;      /* lookup bits for tl */
  int          bd;      /* lookup bits for td */
  unsigned     l[288];  /* length list for huft_build */


  /* set up literal table */
  for ( i = 0; i < 144; i++ ) l [ i ] = 8;
  for (      ; i < 256; i++ ) l [ i ] = 9;
  for (      ; i < 280; i++ ) l [ i ] = 7;

  /* make a complete, but wrong code set */
  for (      ; i < 288; i++ ) l [ i ] = 8;     
 
  bl = 7;
  
  if ( ( i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl ) ) != 0 )
  {
    return i;
  }


  /* set up distance table */
  for ( i = 0; i < 30; i++ ) l [ i ] = 5;   /* make an incomplete code set */
  
  bd = 5;
  
  if ( ( i = huft_build( l, 30, 0, cpdist, cpdext, &td, &bd ) ) > 1 )
  {
    huft_free( tl );
    return i;
  }


  /* decompress until an end-of-block code */
  if ( inflate_codes( tl, td, bl, bd ) )
  {
    return 1;
  }

  /* free the decoding tables, return */
  huft_free( tl );
  huft_free( td );
  return 0;
}



int US_Gzip::inflate_dynamic()
/* decompress an inflated type 2 (dynamic Huffman codes) block. */
{
  int               i;           /* temporary variables */
  unsigned          j;
  unsigned          l;           /* last length */
  unsigned          m;           /* mask for bit lengths table */
  unsigned          n;           /* number of lengths to get */
  unsigned          w;           /* current window position */
  struct huft*      tl;          /* literal/length code table */
  struct huft*      td;          /* distance code table */
  int               bl;          /* lookup bits for tl */
  int               bd;          /* lookup bits for td */
  unsigned          nb;          /* number of bit length codes */
  unsigned          nl;          /* number of literal/length codes */
  unsigned          nd;          /* number of distance codes */
  unsigned          ll[286+30];  /* literal/length and distance code lengths */
  /* register */ ulg      b;           /* bit buffer */
  /* register */ unsigned k;           /* number of bits in bit buffer */

  static unsigned border[] = {    /* Order of the bit length code lengths */
         16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};


  /* make local bit buffer */
  b = bb;
  k = bk;
  w = wp;

  /* read in table lengths */
  NEEDBITS( 5 )
  nl = 257 + ( (unsigned) b & 0x1f );   /* number of literal/length codes */
  DUMPBITS( 5 )

  NEEDBITS( 5 )
  nd = 1 + ((unsigned) b & 0x1f );      /* number of distance codes */
  DUMPBITS( 5 )
  
  NEEDBITS( 4 )
  nb = 4 + ( (unsigned) b & 0xf );      /* number of bit length codes */
  DUMPBITS( 4 )
  
  if ( nl > 286 || nd > 30 ) return 1;  /* bad lengths */


  /* read in bit-length-code lengths */
  for ( j = 0; j < nb; j++ )
  {
    NEEDBITS( 3 )
    ll [ border[j] ] = (unsigned) b & 7;
    DUMPBITS( 3 )
  }

  for ( ; j < 19; j++ )  ll[ border[j] ] = 0;

  /* build decoding table for trees--single level, 7 bit lookup */
  bl = 7;
  
  if ( ( i = huft_build( ll, 19, 19, NULL, NULL, &tl, &bl ) ) != 0 )
  {
    if ( i == 1 )
      huft_free( tl );
    return i;                   /* incomplete code set */
  }

  if ( tl == NULL )   /* Grrrhhh */
  return 2;

  /* read in literal and distance code lengths */
  n = nl + nd;
  m = mask_bits[bl];
  i = 0;
  l = 0;

  while ( (unsigned) i < n )
  {
    NEEDBITS( (unsigned) bl )
    j = ( td = tl + ( (unsigned) b & m ) ) -> b;
    DUMPBITS( j )

    j = td->v.n;
    if ( j < 16 )               /* length of code in bits (0..15) */
    {
      ll [ i++ ] = l = j;       /* save last length in l */
    }
    else if ( j == 16 )         /* repeat last length 3 to 6 times */
    {
      NEEDBITS( 2 )
      j = 3 + ((unsigned)b & 3);
      DUMPBITS( 2 )
      
      if ( (unsigned) i + j > n) return 1;
      
      while ( j-- )  ll [ i++ ] = l;
    }
    else if ( j == 17 )         /* 3 to 10 zero length codes */
    {
      NEEDBITS( 3 )
      j = 3 + ( (unsigned) b & 7 );
      DUMPBITS( 3 )

      if ( (unsigned) i + j > n ) return 1;
      
      while ( j-- )  ll[ i++ ] = 0;
      l = 0;
    }
    else                        /* j == 18: 11 to 138 zero length codes */
    {
      NEEDBITS( 7 )
      j = 11 + ( (unsigned) b & 0x7f );
      DUMPBITS( 7 )

      if ( (unsigned) i + j > n )  return 1;
      
      while ( j-- )  ll[i++] = 0;
      l = 0;
    }
  }


  /* free decoding table for trees */
  huft_free(tl);


  /* restore the global bit buffer */
  bb = b;
  bk = k;


  /* build the decoding tables for literal/length and distance codes */
  bl = lbits;
  if ( ( i = huft_build( ll, nl, 257, cplens, cplext, &tl, &bl ) ) != 0 )
  {
    if ( i == 1 ) 
    {
      huft_free( tl );
    }

    return i;                   /* incomplete code set */
  }

  bd = dbits;
  if ( ( i = huft_build( ll + nl, nd, 0, cpdist, cpdext, &td, &bd ) ) != 0 )
  {
    if (i == 1) 
    {
      huft_free( td );
    }

    huft_free( tl );
    return i;                   /* incomplete code set */
  }


  /* decompress until an end-of-block code */
  if ( inflate_codes( tl, td, bl, bd ) )  return 1;


  /* free the decoding tables, return */
  huft_free( tl );
  huft_free( td );
  return 0;
}



int US_Gzip::inflate_block( int* e )
//int *e;                 /* last block flag */
/* decompress an inflated block */
{
  unsigned          t;  /* block type */
  unsigned          w;  /* current window position */
  /* register */ ulg      b;  /* bit buffer */
  /* register */ unsigned k;  /* number of bits in bit buffer */

  /* make local bit buffer */
  b = bb;
  k = bk;
  w = wp;

  /* read in last block bit */
  NEEDBITS( 1 )

  *e = (int) b & 1;
  DUMPBITS( 1 )


  /* read in block type */
  NEEDBITS( 2 )
  t = (unsigned) b & 3;
  DUMPBITS( 2 )


  /* restore the global bit buffer */
  bb = b;
  bk = k;

  /* inflate that block type */
  if ( t == 2 ) return inflate_dynamic();
  if ( t == 0 ) return inflate_stored();
  if ( t == 1 ) return inflate_fixed();

  /* bad block type */
  return 2;
}



int US_Gzip::inflate()
/* decompress an inflated entry */
{
  int      e;           /* last block flag */
  int      r;           /* result code */
  unsigned h;           /* maximum struct huft's malloc'ed */

  /* initialize window, bit buffer */
  wp = 0;
  bk = 0;
  bb = 0;

  /* decompress until the last block */
  h = 0;
  
  do 
  {
    hufts = 0;
    if ( (r = inflate_block( &e ) ) != 0 ) return r;
    if ( hufts > h ) h = hufts;
  } while ( ! e );

  /* Undo too much lookahead. The next read will be byte aligned so we
   * can discard unused bits in the last meaningful byte.
   */
  
  while ( bk >= 8 ) 
  {
    bk -= 8;
    inptr--;
  }

  /* flush out slide */
  flush_output( wp );

  /* return success */
  return 0;
}

/* ===========================================================================
 *  * Fill the input buffer. This is called only when the buffer is empty.
 *   */
int US_Gzip::fill_inbuf( int eof_ok )
//int eof_ok;          /* set if EOF acceptable as a result */
{
  int len;

  /* Read as much as possible */
  insize = 0;
  do 
  {
    len = read ( ifd, (char*) inbuf + insize, INBUFSIZ - insize );
    if ( len == 0 ) break;
    if ( len == -1 ) 
    {
       throw GZIP_READERROR; //read_error();
       break;
    }
    
    insize += len;
  } while ( insize < INBUFSIZ );

  if ( insize == 0 ) 
  {
    if ( eof_ok ) return EOF;
    flush_window();
    throw GZIP_READERROR; //read_error();
  }
  
  bytes_in += (off_t) insize;
  inptr     = 1;
  return inbuf[ 0 ];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
void US_Gzip::flush_window()
{
  if ( outcnt == 0 ) return;

  updcrc( window, outcnt );
  write_buf( ofd, (char *) window, outcnt );
  
  bytes_out += (off_t) outcnt;
  outcnt     = 0;
}

/* ===========================================================================
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 */
ulg US_Gzip::updcrc( uch* s, unsigned n )
//uch *s;                 /* pointer to bytes to pump through */
//unsigned n;             /* number of bytes in s[] */
{
  /* register */ ulg c;         /* temporary variable */

  static ulg crc_internal = (ulg) 0xffffffffL; /* shift register contents */

  if ( s == NULL) 
  {
     c = 0xffffffffL;
  } 
  else 
  {
     c = crc_internal;
     if ( n ) do 
     {
       c = crc_32_tab[ ( (int) c ^ ( *s++ ) ) & 0xff ] ^ ( c >> 8 );
     } while ( --n );
  }

  crc_internal = c;
  return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}


/* ===========================================================================
 * Does the same as write(), but also handles partial pipe writes and checks
 * for error return.
 */

void US_Gzip::write_buf( int fd, void* buf, unsigned cnt )
   // int       fd;
   // voidp     buf;
   // unsigned  cnt;
{
  unsigned  n;

  while ( ( n = write( fd, buf, cnt) ) != cnt) 
  {
    if ( n == (unsigned) (-1) ) 
    {
      throw GZIP_WRITEERROR;
    }
  
    cnt -= n;
    buf  = (void*) ( (char*) buf + n );
  }
}

/* ========================================================================
 * Return the base name of a file (remove any directory prefix and
 * any version suffix). For systems with file names that are not
 * case sensitive, force the base name to lower case.
 */

#define PATH_SEP '/'

#ifdef WIN32 /* Windows NT */
#  define PATH_SEP2 '\\'
#  define PATH_SEP3 ':'
#endif

char* US_Gzip::base_name( char* fname )
{
  char* p;
  char* f = fname;

  if ( ( p = strrchr( f, PATH_SEP  ) ) != NULL ) f = p + 1;

#ifdef PATH_SEP2
  if ( ( p = strrchr( f, PATH_SEP2 ) ) != NULL ) f = p + 1;
#endif

#ifdef PATH_SEP3
  if ( ( p = strrchr( f, PATH_SEP3 ) ) != NULL ) f = p + 1;
#endif

  return f;
}

/* ===========================================================================
 * Write the output buffer outbuf[0..outcnt-1] and update bytes_out.
 * (used for the compressed data only)
 */
void US_Gzip::flush_outbuf()
{
  if ( outcnt == 0) return;

  write_buf( ofd, (char *) outbuf, outcnt );
  bytes_out += (off_t) outcnt;
  outcnt     = 0;
}

/* ===========================================================================
 * We use a lazy evaluation for matches: a match is finally adopted only if
 * there is no better match at the next window position.  */

#define HASH_BITS  15
   /* For portability to 16 bit machines, do not use values above 15. */

#define MIN_LOOKAHEAD ( MAX_MATCH + MIN_MATCH + 1 )
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH + 1.  */

#define MAX_DIST  ( WSIZE - MIN_LOOKAHEAD )
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE. */

#define HASH_SIZE (unsigned) ( 1 << HASH_BITS )
#define HASH_MASK ( HASH_SIZE - 1 )
#define WMASK     ( WSIZE - 1 )
/* HASH_SIZE and WSIZE must be powers of two */



#define H_SHIFT  ( (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH )
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS   */


/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.  */

#define UPDATE_HASH(h,c) ( h = ( ( (h) << H_SHIFT ) ^ (c))  & HASH_MASK )


/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).   */

#define INSERT_STRING(s, match_head) \
   ( UPDATE_HASH( ins_h, window[ (s) + MIN_MATCH - 1 ] ), \
                  prev[ (s) & WMASK ] = match_head = gzip_head[ ins_h ], \
                  gzip_head[ ins_h ] = (s) )

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match. */

#define FLUSH_BLOCK(eof) \
   flush_block( block_start >= 0L ? (char*) &window[ (unsigned) block_start ] : \
                   (char*) NULL, (long) strstart - block_start, (eof) )


off_t US_Gzip::deflate( void )
{ 
    IPos              hash_head;                       /* head of hash chain */
    IPos              prev_match;                          /* previous match */
    int               flush;         /* set if current block must be flushed */
    int               match_available = 0;   /* set if previous match exists */
    /* register */ unsigned match_length = MIN_MATCH - 1;  /* length of best match */

    /* Process the input block. */
    while ( lookahead != 0 ) 
    {
      /* Insert the string window[strstart .. strstart+2] in the
       * dictionary, and set hash_head to the head of the hash chain: */

      INSERT_STRING( strstart, hash_head );

      /* Find the longest match, discarding those <= prev_length. */
      
      prev_length  = match_length, prev_match = match_start;
      match_length = MIN_MATCH-1;

      if ( hash_head != (ush) 0         && 
           prev_length < max_lazy_match &&
           strstart - hash_head <= MAX_DIST ) 
      {    
        /* To simplify the code, we prevent matches with the string
         * of window index 0 (in particular we have to avoid a match
         * of the string with itself at the start of the input file). */

        match_length = longest_match( hash_head );
            
        /* longest_match() sets match_start */
        
        if ( match_length > lookahead ) match_length = lookahead;

        /* Ignore a length 3 match if it is too distant: */
            
        #define TOO_FAR 4096
        if ( match_length == MIN_MATCH && strstart - match_start > TOO_FAR )
        {
          /* If prev_match is also MIN_MATCH, match_start is garbage
           * but we will ignore the current match anyway.  */

          match_length--;
        }
      }
        
      /* If there was a match at the previous step and the current
       * match is not better, output the previous match: */
        
      if ( prev_length >= MIN_MATCH && match_length <= prev_length ) 
      {
        flush = ct_tally( strstart - 1 - prev_match, prev_length - MIN_MATCH );

        /* Insert in hash table all strings up to the end of the match.
         * strstart-1 and strstart are already inserted.  */

        lookahead   -= prev_length - 1;
        prev_length -= 2;
            
        do 
        {
          strstart++;
          INSERT_STRING(strstart, hash_head);
                
          /* strstart never exceeds WSIZE-MAX_MATCH, so there are
           * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
           * these bytes are garbage, but it does not matter since the
           * next lookahead bytes will always be emitted as literals.  */
        } while ( --prev_length != 0 );

        match_available = 0;
        match_length    = MIN_MATCH - 1;
        strstart++;
        
        if ( flush ) FLUSH_BLOCK(0), block_start = strstart;

        } 
        else if ( match_available ) 
        {
          /* If there was no match at the previous position, output a
           * single literal. If there was a match but the current match
           * is longer, truncate the previous match to a single literal.  */

          if ( ct_tally ( 0, window[ strstart - 1 ] ) ) 
          {
            FLUSH_BLOCK(0), block_start = strstart;
          }

          strstart++;
          lookahead--;
        } 
        else 
        {
          /* There is no previous match to compare with, wait for
           * the next step to decide. */

          match_available = 1;
          strstart++;
          lookahead--;
        
        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.  */

        } while ( lookahead < MIN_LOOKAHEAD && ! eofile ) fill_window();
    }

    if ( match_available ) ct_tally ( 0, window[strstart -1 ] );

    return FLUSH_BLOCK(1);  /* eof */
}

/* ===========================================================================
 *  * Initialize the "longest match" routines for a new file
 *   */
void US_Gzip::lm_init( void )
// Paramaters are not needed here
//    int pack_level; /* 0: store, 1: best speed, 9: best compression */
//    ush *flags;     /* general purpose bit flag */
{


  // While we are at it, initilize other variables needed 
  // These were globals inthe original
  
  /* tree_desc l_desc =
    {
      dyn_ltree, static_ltree, extra_lbits, LITERALS + 1, L_CODES, MAX_BITS, 0
    }; */

  l_desc.dyn_tree    = dyn_ltree;
  l_desc.static_tree = static_ltree;
  l_desc.extra_bits  = extra_lbits;
  l_desc.extra_base  = LITERALS + 1;
  l_desc.elems       = L_CODES;
  l_desc.max_length  = MAX_BITS;
  l_desc.max_code    = 0; 
  
  /*  tree_desc xd_desc =
    { 
      dyn_dtree, static_dtree, extra_dbits, 0, D_CODES, MAX_BITS, 0
    }; */

  d_desc.dyn_tree    = dyn_dtree;
  d_desc.static_tree = static_dtree;
  d_desc.extra_bits  = extra_dbits;
  d_desc.extra_base  = 0;
  d_desc.elems       = D_CODES;
  d_desc.max_length  = MAX_BITS;
  d_desc.max_code    = 0; 

  /*  tree_desc xbl_desc =
    {
      bl_tree, (ct_data*)0, extra_blbits, 0, BL_CODES, MAX_BL_BITS, 0
    };*/

  bl_desc.dyn_tree    = bl_tree;
  bl_desc.static_tree = 0;
  bl_desc.extra_bits  = extra_blbits;
  bl_desc.extra_base  = 0;
  bl_desc.elems       = BL_CODES;
  bl_desc.max_length  = MAX_BL_BITS;
  bl_desc.max_code    = 0; 

///////////////
#define pack_level 9

  /* Initialize the hash table. */
  memzero( (char*) gzip_head, HASH_SIZE * sizeof( *gzip_head ) );

  /* prev will be initialized on the fly */

  /* Set the default configuration parameters:  */
  max_lazy_match   = configuration_table[ pack_level ].max_lazy;
  good_match       = configuration_table[ pack_level ].good_length;
  nice_match       = configuration_table[ pack_level ].nice_length;
  max_chain_length = configuration_table[ pack_level ].max_chain;
  
  strstart    = 0;
  block_start = 0L;
  
  updcrc( NULL, 0 );           /* initialize crc */
  lookahead = file_read( (char*) window,  2 * WSIZE );

  if ( lookahead == 0 ) 
  {
     eofile    = 1;
     lookahead = 0;
     return;
  }

  eofile = 0;

  /* Make sure that we always have enough lookahead. This is important
   * if input comes from a device such as a tty.   */
  
  while ( lookahead < MIN_LOOKAHEAD && ! eofile ) fill_window();

  ins_h = 0;

  /* register */ unsigned j;
  for ( j = 0; j < MIN_MATCH - 1; j++ ) UPDATE_HASH( ins_h, window[ j ] );
  
  /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
   * not important since only literal bytes will be emitted.  */
}

/* ===========================================================================
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)   */
int US_Gzip::file_read( char* buf, unsigned int size )
{
    unsigned len;

    len = read( ifd, buf, size );
    if ( len == 0 ) return (int) len;
    
    if ( len == (unsigned) -1 ) 
    {
      throw GZIP_READERROR;
    }

    crc       = updcrc( (uch*)buf, len );
    bytes_in += (off_t)len;

    return (int) len;
}

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of input file.
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: at least one byte has been read, or eofile is set;
 * file reads are performed for at least two bytes (required for the
 * translate_eol option). */

void US_Gzip::fill_window( void )
{
    /* register */ unsigned n;
    /* register */ unsigned m;
    
    unsigned more = (unsigned) ( 2L * WSIZE - (ulg) lookahead - (ulg) strstart );
    /* Amount of free space at the end of the window. */

    /* If the window is almost full and there is insufficient lookahead,
     *      * move the upper half to the lower one to make room in the upper half.
     *           */
    if ( more == (unsigned) EOF ) 
    {
        /* Very unlikely, but possible on 16 bit machine if strstart == 0
         * and lookahead == 1 (input done one byte at time)  */
        more--;
    } 
    else 
    {
      if ( strstart >= WSIZE + MAX_DIST ) 
      {
        /* By the IN assertion, the window is not empty so we can't confuse
         * more == 0 with more == 64K on a 16 bit machine.  */

        memcpy( (char*) window, (char*) window + WSIZE, (unsigned) WSIZE );
        
        match_start -= WSIZE;
        strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */

        block_start -= (long) WSIZE;

        for ( n = 0; n < HASH_SIZE; n++ ) 
        {
            m       = gzip_head[n];
            gzip_head[n] = (Pos)(m >= WSIZE ? m - WSIZE : 0 );
        }

        for ( n = 0; n < WSIZE; n++ ) 
        {
            m       = prev[n];
            prev[n] = (Pos)(m >= WSIZE ? m - WSIZE : 0 );
            /* If n is not on any hash chain, prev[n] is garbage but
             * its value will never be used.  */
        }

        more += WSIZE;
      }
    }

    /* At this point, more >= 2 */
    if ( ! eofile ) 
    {
        n = file_read( (char*) window + strstart + lookahead, more );
        if ( n == 0 || n == (unsigned) EOF) 
        {
            eofile = 1;
        } 
        else 
        {
            lookahead += n;
        }
    }
}

int US_Gzip::longest_match( IPos cur_match )
//    IPos cur_match;                               /* current match */
{
    unsigned      chain_length = max_chain_length;  /* max hash chain length */
    /* register */ uch* scan         = window + strstart; /* current string */
    /* register */ uch* match;                            /* matched string */
    /* register */ int  len;                              /* length of current match */
    int           best_len     = prev_length;       /* best match length so far */

    IPos limit = strstart > (IPos) MAX_DIST ? strstart - (IPos) MAX_DIST : 0;
    
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0. */

    /* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
     * It is easy to get rid of this optimization if necessary. */

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.   */

    /* register */ uch* strend     = window + strstart + MAX_MATCH - 1;
    /* register */ ush  scan_start = *(ush*) scan;
    /* register */ ush  scan_end   = *(ush*) (scan + best_len - 1 );
#else
    /* register */ uch* strend     = window + strstart + MAX_MATCH;
    /* register */ uch  scan_end1  = scan[ best_len - 1 ];
    /* register */ uch  scan_end   = scan[ best_len ];
#endif

    /* Do not waste too much time if we already have a good match: */
    
    if ( prev_length >= good_match ) 
    {
      chain_length >>= 2;
    }

    do 
    {
      match = window + cur_match;

      /* Skip to next match if the match length cannot increase
       * or if the match length is less than 2: */

#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)
      /* This code assumes sizeof(unsigned short) == 2. Do not use
       * UNALIGNED_OK if your compiler uses a different size. */

      if ( *(ush*)( match + best_len - 1 ) != scan_end ||
           *(ush*) match != scan_start) continue;

      /* It is not necessary to compare scan[2] and match[2] since they are
       * always equal when the other bytes match, given that the hash keys
       * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
       * strstart+3, +5, ... up to strstart+257. We check for insufficient
       * lookahead only every 4th comparison; the 128th check will be made
       * at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
       * necessary to put more guard bytes at the end of the window, or
       * to check more often for insufficient lookahead.  */
        
      scan++;
      match++;
        
      do 
      {
      } while ( *(ush*) ( scan += 2 ) == *(ush*) ( match += 2 ) &&
                *(ush*) ( scan += 2 ) == *(ush*) ( match += 2 ) &&
                *(ush*) ( scan += 2 ) == *(ush*) ( match += 2 ) &&
                *(ush*) ( scan += 2 ) == *(ush*) ( match += 2 ) &&
                scan < strend);
        
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window+strstart + 257 */
        if ( *scan == *match ) scan++;

        len = ( MAX_MATCH - 1 ) - (int) ( strend - scan );
        scan = strend - ( MAX_MATCH - 1 );

#else /* UNALIGNED_OK */
        if ( match[ best_len ]     != scan_end  ||
             match[ best_len - 1 ] != scan_end1 ||
             *match                != *scan     ||
             * ++match             != scan[ 1 ] )  continue;

        /* The check at best_len-1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.   */

        scan += 2, match++;

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart+258. */

        do 
        {
        } while ( * ++scan == * ++match && * ++scan == * ++match &&
                  * ++scan == * ++match && * ++scan == * ++match &&
                  * ++scan == * ++match && * ++scan == * ++match &&
                  * ++scan == * ++match && * ++scan == * ++match &&
                  scan < strend );

        len = MAX_MATCH - (int)( strend - scan );
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        if ( len > best_len ) 
        {
          match_start = cur_match;
          best_len    = len;
          if ( len >= nice_match ) break;
#ifdef UNALIGNED_OK
          scan_end   = *(ush*) ( scan + best_len - 1 );
#else
          scan_end1  = scan[ best_len - 1 ];
          scan_end   = scan[ best_len     ];
#endif
        }
    } while ( ( cur_match = prev[ cur_match & WMASK ] ) > limit
                && --chain_length != 0 );

    return best_len;
}

/* ===========================================================================
 * Initialize the bit string routines. */

void US_Gzip::bi_init ( void )
//    file_t zipfile; /* output zip file, NO_FILE for in-memory compression */
{
    bi_buf   = 0;
    bi_valid = 0;

    /* Set the defaults for file compression. They are set by memcompress
     * for in-memory compression.  */
}

#define d_code(dist) \
   ( (dist) < 256 ? dist_code[ dist ] : dist_code[ 256 + ( ( dist ) >> 7 ) ] )
   /* Mapping from a distance to a distance code. dist is the distance - 1 and
    * must not have side effects. dist_code[256] and dist_code[257] are never
    * used. */

#define STORED_BLOCK 0
#define STORED STORED_BLOCK
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#ifndef DIST_BUFSIZE
#  define DIST_BUFSIZE  LIT_BUFSIZE
#endif
    /* Sizes of match buffers for literals/lengths and distances.  There are
     * 4 reasons for limiting LIT_BUFSIZE to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input data is
     *     still in the window so we can still emit a stored block even when input
     *     comes from standard input.  (This can also be done for all blocks if
     *     LIT_BUFSIZE is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting trees
     *     more frequently.
     *   - I can't count above 4
     * The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
     * memory at the expense of compression). Some optimizations would be possible
     * if we rely on DIST_BUFSIZE == LIT_BUFSIZE.  */

/* ===========================================================================
 * Allocate the match buffer, initialize the various tables and save the
 * location of the internal file attribute (ascii/binary) and method
 * (DEFLATE/STORE).
 */

void US_Gzip::ct_init ( void )
{
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */

    compressed_len = 0L;

    if ( static_dtree[ 0 ].GZLen != 0 ) return; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for ( code = 0; code < LENGTH_CODES - 1; code++ ) 
    {
      base_length[code] = length;
      for ( n = 0; n < ( 1 << extra_lbits[ code ] ); n++ ) 
      {
        length_code[length++] = (uch)code;
      }
    }

    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    length_code[ length - 1 ] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for ( code = 0 ; code < 16; code++ ) 
    {
      base_dist[ code ] = dist;
      for ( n = 0; n < ( 1 << extra_dbits[ code ] ); n++ ) 
      {
        dist_code[ dist++ ] = (uch) code;
      }
    }

    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++ ) 
    {
      base_dist[ code ] = dist << 7;
      for ( n = 0; n < ( 1 << ( extra_dbits[ code ] - 7 ) ); n++ ) 
      {
        dist_code[256 + dist++] = (uch)code;
      }
    }

    /* Construct the codes of the static literal tree */
    for ( bits = 0; bits <= MAX_BITS; bits++ ) bl_count[ bits ] = 0;
    
    n = 0;
    while ( n <= 143 ) static_ltree[ n++ ].GZLen = 8, bl_count[ 8 ]++;
    while ( n <= 255 ) static_ltree[ n++ ].GZLen = 9, bl_count[ 9 ]++;
    while ( n <= 279 ) static_ltree[ n++ ].GZLen = 7, bl_count[ 7 ]++;
    while ( n <= 287 ) static_ltree[ n++ ].GZLen = 8, bl_count[ 8 ]++;
    
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes( (ct_data*) static_ltree, L_CODES + 1 );

    /* The static distance tree is trivial: */
    for ( n = 0; n < D_CODES; n++ ) 
    {
        static_dtree[ n ].GZLen = 5;
        static_dtree[ n ].Code = bi_reverse( n, 5 );
    }

    /* Initialize the first block of the first file: */
    init_block();
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed. */

int US_Gzip::ct_tally ( int dist, int lc )
//    int dist;  /* distance of matched string */
//    int lc;    /* match length-MIN_MATCH or unmatched char (if dist==0) */
{
    inbuf[ last_lit++ ] = (uch) lc;
    
    if ( dist == 0) 
    {
      /* lc is the unmatched char */
      dyn_ltree[ lc ].Freq++;
    } 
    else 
    {
        /* Here, lc is the match length - MIN_MATCH */
      dist--;             /* dist = match distance - 1 */

      dyn_ltree[ length_code[ lc ] + LITERALS + 1 ].Freq++;
      dyn_dtree[ d_code( dist ) ].Freq++;

      d_buf[ last_dist++ ] = (ush) dist;
      flags |= flag_bit;
    }

    flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ( ( last_lit & 7 ) == 0 ) 
    {
      flag_buf[ last_flags++ ] = flags;
      flags                    = 0;
      flag_bit                 = 1;
    }

    /* Try to guess if it is profitable to stop the current block here */
#define level 9
    if ( level > 2 && ( last_lit & 0xfff ) == 0) 
    {
      /* Compute an upper bound for the compressed length */
      ulg out_length = (ulg) last_lit * 8L;
      ulg in_length  = (ulg) strstart - block_start;
      int dcode;
      
      for ( dcode = 0; dcode < D_CODES; dcode++ ) 
      {
        out_length += (ulg)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
      }

      out_length >>= 3;
      if ( last_dist < last_lit / 2 && out_length < in_length / 2) return 1;
    }

    return ( last_lit == LIT_BUFSIZE - 1 || last_dist == DIST_BUFSIZE );
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K - 1 bytes.  */
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length for the file so far.  */

off_t US_Gzip::flush_block( char* buf, ulg stored_len, int eof )
//    char *buf;        /* input block, or NULL if too old */
//    ulg stored_len;   /* length of input block */
//    int eof;          /* true if this is the last block for a file */
{
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

    flag_buf[ last_flags ] = flags; /* Save the flags for the last 8 items */

     /* Check if the file is ascii or binary */
    //if ( *file_type == (ush) UNKNOWN) set_file_type();

    /* Construct the literal and distance trees */
    build_tree( (tree_desc*) ( &l_desc ) );

    build_tree( (tree_desc*) ( &d_desc ) );

    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations. */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send. */

    max_blindex = build_bl_tree();

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb    = ( opt_len + 3 + 7 ) >> 3;
    static_lenb = ( static_len + 3 + 7 ) >> 3;
    //input_len  += stored_len; /* for debugging only */

    if ( static_lenb <= opt_lenb ) opt_lenb = static_lenb;

    /* If compression failed and this is the first and last block,
     * and if the zip file can be seeked (to rewrite the local header),
     * the whole file is transformed into a stored file: */

#define seekable()    0  /* force sequential output */

#ifdef FORCE_METHOD
    if (level == 1 && eof && compressed_len == 0L) { /* force stored file */
#else
    if ( stored_len <= opt_lenb && eof && compressed_len == 0L && seekable() ) 
    {
#endif

      /* Since LIT_BUFSIZE <= 2*WSIZE, the input data must be there: */
      if ( buf == (char*) 0) throw GZIP_INTERNAL;

        copy_block( buf, (unsigned) stored_len, 0 ); /* without header */
        compressed_len = stored_len << 3;
        *file_method   = STORED;

#ifdef FORCE_METHOD
    } 
    else 
      if (level == 2 && buf != (char*)0) 
      { /* force stored block */
#else
    } 
    else 
      if ( stored_len + 4 <= opt_lenb && buf != (char*) 0 ) 
      {
         /* 4: two words for the lengths */
#endif
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.  */

        send_bits( ( STORED_BLOCK << 1 ) + eof, 3 );  /* send block type */
        compressed_len  = ( compressed_len + 3 + 7 ) & ~7L;
        compressed_len += ( stored_len + 4 ) << 3;

        copy_block( buf, (unsigned) stored_len, 1 ); /* with header */

#ifdef FORCE_METHOD
    } 
    else 
      if ( level == 3 ) 
      { /* force static trees */
#else
    } else 
      if ( static_lenb == opt_lenb ) 
      {
#endif
        send_bits( ( STATIC_TREES << 1 ) + eof, 3 );
        compress_block( (ct_data*) static_ltree, ( ct_data*) static_dtree );
        compressed_len += 3 + static_len;
    } else 
    {
        send_bits( (DYN_TREES << 1 ) + eof, 3 );
        send_all_trees( l_desc.max_code + 1, d_desc.max_code + 1, max_blindex + 1 );
        compress_block( (ct_data*) dyn_ltree, (ct_data*) dyn_dtree );
        compressed_len += 3 + opt_len;
    }

    init_block();

    if ( eof ) 
    {
        bi_windup();
        compressed_len += 7;  /* align on byte boundary */
    }

    return compressed_len >> 3;
}

/* ===========================================================================
 * Initialize a new block. */

void US_Gzip::init_block( void )
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for ( n = 0; n < L_CODES;  n++ ) dyn_ltree[ n ].Freq = 0;
    for ( n = 0; n < D_CODES;  n++ ) dyn_dtree[ n ].Freq = 0;
    for ( n = 0; n < BL_CODES; n++ ) bl_tree  [ n ].Freq = 0;

    dyn_ltree[ END_BLOCK ].Freq = 1;
    opt_len = static_len = 0L;
    last_lit = last_dist = last_flags = 0;
    flags = 0; flag_bit = 1;
}

#define send_code(c, tree) send_bits( tree[ c ].Code, tree[ c ].GZLen )
   /* Send a code of the given tree. c and tree must not have side effects */

#define l_buf inbuf


/* ===========================================================================
 * Send the block data compressed using the given Huffman trees */
void US_Gzip::compress_block( ct_data* ltree, ct_data* dtree)
//    ct_data* ltree; /* literal tree */
//    ct_data* dtree; /* distance tree */
{
  unsigned dist;      /* distance of matched string */
  int      lc;        /* match length or unmatched char (if dist == 0) */
  unsigned lx   = 0;  /* running index in l_buf */
  unsigned dx   = 0;  /* running index in d_buf */
  unsigned fx   = 0;  /* running index in flag_buf */
  uch      flag = 0;  /* current flags */
  unsigned code;      /* the code to send */
  int      extra;     /* number of extra bits to send */

  if ( last_lit != 0 ) 
  do 
  {
    if ( ( lx & 7 ) == 0 ) flag = flag_buf[ fx++ ];
    lc = l_buf[ lx++ ];

    if ( ( flag & 1 ) == 0 ) 
    {
      send_code( lc, ltree ); /* send a literal byte */
    } 
    else 
    {
      /* Here, lc is the match length - MIN_MATCH */
      code = length_code[ lc ];
      send_code( code + LITERALS + 1, ltree ); /* send the length code */
      extra = extra_lbits[ code ];
      
      if ( extra != 0 ) 
      {
        lc -= base_length[ code ];
        send_bits( lc, extra );        /* send the extra length bits */
      }

      dist = d_buf[ dx++ ];
      
      /* Here, dist is the match distance - 1 */
      code = d_code( dist );

      send_code( code, dtree );       /* send the distance code */
      extra = extra_dbits[ code ];
      
      if ( extra != 0 ) 
      {
        dist -= base_dist[ code ];
        send_bits( dist, extra );   /* send the extra distance bits */
      }
    } /* literal or match pair ? */
    
    flag >>= 1;

  } while ( lx < last_lit );

  send_code( END_BLOCK, ltree );
}

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.  */

void US_Gzip::send_bits( int value, int length )
//    int value;  /* value to send */
//    int length; /* number of bits */
{
  /* If not enough room in bi_buf, use (valid) bits from bi_buf and
   * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
   * unused bits in value.  */

  if ( bi_valid > (int) Buf_size - length ) 
  {
      bi_buf   |= (value << bi_valid);
      put_short( bi_buf );
      bi_buf    = (ush)value >> ( (ush) Buf_size - bi_valid );
      bi_valid += length - Buf_size;
  } 
  else
  {
      bi_buf   |= value << bi_valid;
      bi_valid += length;
  }
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15 */

unsigned US_Gzip::bi_reverse( unsigned code, int len )
//    unsigned code; /* the value to invert */
//    int len;       /* its bit length */
{
  /* register */ unsigned res = 0;
  
  do 
  {
    res  |= code & 1;
    code >>= 1;
    res  <<= 1;
  } while ( --len > 0 );

  return res >> 1;
}

/* ===========================================================================
 * Write out any remaining bits in an incomplete byte. */
void US_Gzip::bi_windup( void )
{
  if ( bi_valid > 8 ) 
  {
    put_short( bi_buf );
  } 
  else if ( bi_valid > 0 ) 
  {
      put_byte( bi_buf );
  }

  bi_buf   = 0;
  bi_valid = 0;
}

/* ===========================================================================
 *  * Copy a stored block to the zip file, storing first the length and its
 *   * one's complement if requested.
 *    */
void US_Gzip::copy_block( char* buf, unsigned len, int header )
//    char     *buf;    /* the input data */
//    unsigned len;     /* its length */
//    int      header;  /* true if block header must be written */
{
  bi_windup();              /* align on byte boundary */

  if ( header ) 
  {
      put_short( (ush)  len );
      put_short( (ush) ~len );
  }  
  
  while ( len-- ) 
  {
    put_byte( *buf++ );
  }
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */

/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.  */
#define pqremove(tree, top) \
{\
  top              = heap[ SMALLEST ]; \
  heap[ SMALLEST ] = heap[ heap_len--] ; \
  pqdownheap( tree, SMALLEST ); \
}

#define MAX(a,b) ( a >= b ? a : b )
/* the arguments must not have side effects */

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 * and corresponding code. The length opt_len is updated; static_len is
 * also updated if stree is not null. The field max_code is set.  */

void US_Gzip::build_tree( tree_desc* desc )
//    tree_desc near *desc; /* the tree descriptor */
{
  ct_data* tree     = desc->dyn_tree;
  ct_data* stree    = desc->static_tree;
  int      elems    = desc->elems;
  int      n, m;              /* iterate over heap elements */
  int      max_code = -1;     /* largest code with non zero frequency */
  int      node     = elems;  /* next internal node of the tree */

  /* Construct the initial heap, with least frequent element in
   * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
   * heap[0] is not used.  */

  heap_len = 0;
  heap_max = HEAP_SIZE;

  for ( n = 0; n < elems; n++ ) 
  {
    if ( tree[ n ].Freq != 0 ) 
    {
      heap[ ++heap_len ] = max_code = n;
      depth[ n ] = 0;
    } 
    else 
    {
      tree[ n ].GZLen = 0;
    }
  }

  /* The pkzip format requires that at least one distance code exists,
   * and that at least one bit should be sent even if there is only one
   * possible code. So to avoid special checks later on we force at least
   * two codes of non zero frequency.  */

  while ( heap_len < 2 ) 
  {
    int new1 = heap[ ++heap_len ] = ( max_code < 2 ? ++max_code : 0 );
    
    tree [ new1 ].Freq = 1;
    depth[ new1 ]      = 0;
    opt_len--; 
    if ( stree ) static_len -= stree[ new1 ].GZLen;
      /* new is 0 or 1 so it does not have extra bits */
  }

  desc->max_code = max_code;

  /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
   * establish sub-heaps of increasing lengths:  */

  for ( n = heap_len / 2; n >= 1; n-- ) pqdownheap( tree, n );

  /* Construct the Huffman tree by repeatedly combining the least two
   * frequent nodes.  */
  
  do 
  {
      pqremove( tree, n );   /* n = node of least frequency */
      m = heap[ SMALLEST ];  /* m = node of next least frequency */

      heap[ --heap_max ] = n; /* keep the nodes sorted by frequency */
      heap[ --heap_max ] = m;

      /* Create a new node father of n and m */
      tree [ node ].Freq = tree[ n ].Freq + tree[ m ].Freq;
      depth[ node ]      = (uch) ( MAX( depth[ n ], depth[ m ] ) + 1 );
      tree [ n    ].Dad  = tree[ m ].Dad = (ush) node;
      
      /* and insert the new node in the heap */
      heap[ SMALLEST ] = node++;
      pqdownheap( tree, SMALLEST );

  } while ( heap_len >= 2 );

  heap[ --heap_max ] = heap[ SMALLEST ];

  /* At this point, the fields freq and dad are set. We can now
   * generate the bit lengths. */

  gen_bitlen( (tree_desc*) desc );

  /* The field len is now set, we can generate the bit codes */
  gen_codes ( (ct_data*) tree, max_code );
}


/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.  */

int US_Gzip::build_bl_tree( void )
{
  int max_blindex;  /* index of last bit length code of non zero freq */

  /* Determine the bit length frequencies for literal and distance trees */
  scan_tree( (ct_data*) dyn_ltree, l_desc.max_code );
  scan_tree( (ct_data*) dyn_dtree, d_desc.max_code );

  /* Build the bit length tree: */
  build_tree( ( tree_desc*) ( &bl_desc ) );

  /* opt_len now includes the length of the tree representations, except
   * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.  */

  /* Determine the number of bit length codes to send. The pkzip format
   * requires that at least 4 bit length codes be sent. (appnote.txt says
   * 3 but the actual value used is 4.)  */

  for ( max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex-- ) 
  {
    if ( bl_tree[ bl_order[ max_blindex ] ].GZLen != 0 ) break;
  }

  /* Update opt_len to include the bit length tree and counts */
  opt_len += 3 * ( max_blindex + 1 ) + 5 + 5 + 4;

  return max_blindex;
}
/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.  */

void US_Gzip::gen_bitlen( tree_desc* desc )
//    tree_desc near *desc; /* the tree descriptor */
{
  ct_data* tree       = desc->dyn_tree;
  int*     extra      = desc->extra_bits;
  int      base       = desc->extra_base;
  int      max_code   = desc->max_code;
  int      max_length = desc->max_length;
  ct_data* stree      = desc->static_tree;
  int      h;              /* heap index */
  int      n;
  int      m;              /* iterate over the tree elements */
  int      bits;           /* bit length */
  int      xbits;          /* extra bits */
  ush      f;              /* frequency */
  int      overflow   = 0; /* number of elements with bit length too large */

  for ( bits = 0; bits <= MAX_BITS; bits++ ) bl_count[ bits ] = 0;

  /* In a first pass, compute the optimal bit lengths (which may
   * overflow in the case of the bit length tree). */

  tree[ heap[ heap_max ] ].GZLen = 0; /* root of the heap */

  for ( h = heap_max + 1; h < HEAP_SIZE; h++ ) 
  {
    n    = heap[ h ];
    bits = tree[ tree[ n ].Dad ].GZLen + 1;
    
    if ( bits > max_length) 
    {
      bits = max_length;
      overflow++;
    }
    
    tree[ n ].GZLen = (ush) bits;
    /* We overwrite tree[ n ].Dad which is no longer needed */

    if ( n > max_code ) continue; /* not a leaf node */

    bl_count[ bits ]++;
    xbits = 0;

    if ( n >= base ) xbits = extra[ n - base ];
    
    f        = tree[ n ].Freq;
    opt_len += (ulg) f * ( bits + xbits );
    
    if ( stree ) static_len += (ulg) f * ( stree[ n ].GZLen + xbits );
  }

  if ( overflow == 0) return;

  /* Find the first bit length which could increase: */
  do 
  {
    bits = max_length-1;
    
    while ( bl_count[ bits ] == 0 ) bits--;

    bl_count[ bits       ]--;       /* move one leaf down the tree */
    bl_count[ bits + 1   ]    += 2; /* move one overflow item as its brother */
    bl_count[ max_length ]--;
    
    /* The brother of the overflow item also moves one step up,
     * but this does not affect bl_count[max_length]  */

    overflow -= 2;

  } while ( overflow > 0 );

  /* Now recompute all bit lengths, scanning in increasing frequency.
   * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
   * lengths instead of fixing only the wrong ones. This idea is taken
   * from 'ar' written by Haruhiko Okumura.)  */

  for ( bits = max_length; bits != 0; bits-- ) 
  {
    n = bl_count[bits];

    while ( n != 0 ) 
    {
      m = heap[--h];

      if ( m > max_code ) continue;
      
      if ( tree[ m ].GZLen != (unsigned) bits ) 
      {
        opt_len      += ( (long) bits - (long) tree[ m ].GZLen ) * (long) tree[ m ].Freq;
        tree[ m ].GZLen = (ush) bits;
      }
      n--;
    }
  }
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 *    the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.  */

void US_Gzip::gen_codes ( ct_data* tree, int max_code )
//    ct_data near *tree;        /* the tree to decorate */
//    int max_code;              /* largest code with non zero frequency */
{
  ush next_code[ MAX_BITS + 1 ]; /* next code value for each bit length */
  ush code = 0;                  /* running code value */
  int bits;                      /* bit index */
  int n;                         /* code index */

  /* The distribution counts are first used to generate the code values
   * without bit reversal. */

  for ( bits = 1; bits <= MAX_BITS; bits++ ) 
  {
    next_code[ bits ] = code = ( code + bl_count[ bits - 1 ] ) << 1;
  }

  /* Check that the bit counts in bl_count are consistent. The last code
   * must be all ones.  */

  for ( n = 0;  n <= max_code; n++ ) 
  {
    int len = tree[ n ].GZLen;

    if ( len == 0 ) continue;
    
    /* Now reverse the bits */
    tree[ n ].Code = bi_reverse( next_code[ len ]++, len );
  }
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length. */

#define smaller( tree, n, m ) \
   ( tree[ n ].Freq < tree[ m ].Freq || \
     ( tree[ n ].Freq == tree[ m ].Freq && depth[ n ] <= depth[ m ] ) )

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).  */

void US_Gzip::pqdownheap( ct_data* tree, int k )
//    ct_data near *tree;  /* the tree to restore */
//    int k;               /* node to move down */
{
  int v = heap[ k ];
  int j = k << 1;      /* left son of k */

  while ( j <= heap_len )
  {
    /* Set j to the smallest of the two sons: */
    if ( j < heap_len && smaller( tree, heap[ j + 1 ], heap[ j ] ) ) j++;

    /* Exit if v is smaller than both sons */
    if ( smaller( tree, v, heap[ j ] ) ) break;

    /* Exchange v with the smallest son */
    heap[ k ] = heap[ j ];  
    k = j;

    /* And continue down the tree, setting j to the left son of k */
    j <<= 1;
  }
  heap[ k ] = v;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.  */

void US_Gzip::send_all_trees( int lcodes, int dcodes, int blcodes )
//    int lcodes, dcodes, blcodes; /* number of codes for each tree */
{
  int rank;                    /* index in bl_order */

  send_bits( lcodes - 257, 5 ); /* not +255 as stated in appnote.txt */
  send_bits( dcodes -   1, 5 );
  send_bits( blcodes -  4, 4 ); /* not -3 as stated in appnote.txt */
  
  for ( rank = 0; rank < blcodes; rank++ ) 
  {
    send_bits( bl_tree[ bl_order[ rank ] ].GZLen, 3 );
  }

  send_tree( (ct_data*) dyn_ltree, lcodes - 1 ); /* send the literal tree */
  send_tree(( ct_data*) dyn_dtree, dcodes - 1 ); /* send the distance tree */
}

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.) */

void US_Gzip::scan_tree( ct_data* tree, int max_code )
//    ct_data near *tree; /* the tree to be scanned */
//    int max_code;       /* and its largest code of non zero frequency */
{
  int n;                       /* iterates over all tree elements */
  int prevlen   = -1;          /* last emitted length */
  int curlen;                  /* length of current code */
  int nextlen   = tree[0].GZLen; /* length of next code */
  int count     = 0;           /* repeat count of the current code */
  int max_count = 7;           /* max repeat count */
  int min_count = 4;           /* min repeat count */

  if ( nextlen == 0 ) 
  {
    max_count = 138;
    min_count = 3;
  }

  tree[ max_code + 1 ].GZLen = (ush) 0xffff; /* guard */

  for ( n = 0; n <= max_code; n++ ) 
  {
    curlen  = nextlen; 
    nextlen = tree[ n + 1 ].GZLen;
    
    if ( ++count < max_count && curlen == nextlen ) 
    {
      continue;
    } 
    else if ( count < min_count ) 
    {
      bl_tree[ curlen ].Freq += count;
    } 
    else if ( curlen != 0 ) 
    {
      if ( curlen != prevlen ) bl_tree[ curlen ].Freq++;
      bl_tree[REP_3_6].Freq++;
    } 
    else if ( count <= 10 ) 
    {
      bl_tree[ REPZ_3_10 ].Freq++;
    } 
    else 
    {
      bl_tree[ REPZ_11_138 ].Freq++;
    }

    count   = 0; 
    prevlen = curlen;

    if ( nextlen == 0 ) 
    {
      max_count = 138;
      min_count = 3;
    } 
    else if ( curlen == nextlen ) 
    {
      max_count = 6;
      min_count = 3;
    } 
    else 
    {
      max_count = 7;
      min_count = 4;
    }
  }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.  */

void US_Gzip::send_tree( ct_data* tree, int max_code )
//    ct_data near *tree; /* the tree to be scanned */
//    int max_code;       /* and its largest code of non zero frequency */
{
  int n;                       /* iterates over all tree elements */
  int prevlen   = -1;          /* last emitted length */
  int curlen;                  /* length of current code */
  int nextlen   = tree[0].GZLen; /* length of next code */
  int count     = 0;           /* repeat count of the current code */
  int max_count = 7;           /* max repeat count */
  int min_count = 4;           /* min repeat count */

  /* tree[max_code+1].GZLen = -1; */  /* guard already set */

  if ( nextlen == 0 )
  {
    max_count = 138;
    min_count = 3;
  }

  for ( n = 0; n <= max_code; n++ ) 
  {
    curlen  = nextlen; 
    nextlen = tree[ n + 1 ].GZLen;

    if ( ++count < max_count && curlen == nextlen ) 
    {
      continue;
    } 
    else if ( count < min_count ) 
    {
      do 
      { 
        send_code( curlen, bl_tree ); 
      } while ( --count != 0 );
    } 
    else if ( curlen != 0 ) 
    {
      if ( curlen != prevlen ) 
      {
        send_code( curlen, bl_tree ); 
        count--;
      }

      send_code( REP_3_6, bl_tree ); 
      send_bits( count - 3, 2 );
    } 
    else if ( count <= 10 ) 
    {
      send_code( REPZ_3_10, bl_tree ); 
      send_bits( count - 3, 3 );
    } 
    else 
    {
      send_code( REPZ_11_138, bl_tree ); 
      send_bits( count - 11, 7 );
    }

    count   = 0; 
    prevlen = curlen;

    if ( nextlen == 0 ) 
    {
      max_count = 138;
      min_count = 3;
    }
    else if ( curlen == nextlen ) 
    {
      max_count = 6;
      min_count = 3;
    } 
    else 
    {
      max_count = 7;
      min_count = 4;
    }
  }
}

/////////////////////////////
QString US_Gzip::explain( const int error )
{
   QString explanation;
   switch ( error )
   {
    case GZIP_OK:
      explanation = "The g(un)zip operation was succesful.";
      break;

    case GZIP_NOEXIST:
      explanation = "Could not find the input file." ;
      break;

    case  GZIP_NOTFILE:
      explanation = "The input file name is not a regular file." ;
      break;

    case GZIP_NOREAD:
      explanation = "The input file is not readable by the user." ;
      break;

    case GZIP_OPTIONNOTSUPPORTED:
      explanation = "An unsupported option is included in the compressed file.";
      break;
      
    case GZIP_OUTFILEEXISTS:
      explanation = "The output file already exists." ;
      break;
      
    case GZIP_CRCERROR:
      explanation = "The compressed file failed an internal validation." ;
      break;
      
    case GZIP_READERROR:
      explanation = "Could not read the input file." ;
      break;
      
    case GZIP_WRITEERROR:
      explanation = "Could not write the output file." ;
      break;
      
    case GZIP_LENGTHERROR:
      explanation = "The output data does not match the internal file size." ;
      break;

    case GZIP_FILENAMEERROR:
      explanation = "The file/path name is too long." ;
      break;

    case GZIP_INTERNAL:
      explanation = "In internal gzip error occured." ;
      break;

    default:
       explanation = QString( "Unknown return code: " ) + QString::number( error );
  }

   return explanation;
}
