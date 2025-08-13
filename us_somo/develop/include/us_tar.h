#ifndef US_TAR_H
#define US_TAR_H

#include <qstring.h>
#include <qstringlist.h>

#include <sys/types.h>

#ifdef UNIX
#include <unistd.h>
#endif

#include "us_extern.h"

// Error codes


#define TAR_OK                  0
#define TAR_CANNOTCREATE        1
#define TAR_NOTFOUND            2
#define TAR_CANTSTAT            3
#define TAR_FILENAMETOOLONG     4
#define TAR_INTERNAL            5
#define TAR_READERROR           6
#define TAR_WRITEERROR          7
#define TAR_ARCHIVEERROR        8
#define TAR_MKDIRFAILED         9

class US_EXTERN US_Tar
{
   public:
      ~US_Tar(){};
      US_Tar(){};

      // From a directory
      int     create ( const QString&, const QString&, QStringList* = 0 ); 

      // From a list of files ( including directories )
      int     create ( const QString&, const QStringList&, QStringList* = 0 );
      
      int     extract( const QString&, QStringList* = 0 ); 
      int     list   ( const QString&, QStringList&, bool brief = false ); 
      QString explain( const int ); 

   private:

      // Class variables
      int ofd;                 // Output file descriptor
      int ifd;                 // Input file descriptor

      typedef struct posix_header
      {  /* byte offset */
         char name[100];     /*   0 */
         char mode[8];       /* 100 */
         char uid[8];        /* 108 */
         char gid[8];        /* 116 */
         char size[12];      /* 124 */
         char mtime[12];     /* 136 */
         char chksum[8];     /* 148 */
         char typeflag;      /* 156 */
         char linkname[100]; /* 157 */
         char magic[6];      /* 257 */
         char version[2];    /* 263 */
         char uname[32];     /* 265 */
         char gname[32];     /* 297 */
         char devmajor[8];   /* 329 */
         char devminor[8];   /* 337 */
         char prefix[155];   /* 345 */
         char undefined[12]; /* 500 */
      } posix_header;

#define BLOCK_SIZE      512
#define BLOCKING_FACTOR  20

      union
      {
         posix_header  header;
         unsigned char h[ BLOCK_SIZE ];
      } tar_header;

      unsigned char buffer[ BLOCK_SIZE * BLOCKING_FACTOR ];

      int           blocks_written;
      int           blocks_read;
      // int           archive_size;

      // Internal methods

      void    process_dir         ( const QString&, QStringList& );
      void    write_file          ( const QString& );
      void    write_long_filename ( const QString& );
      void    flush_buffer        ( void );
      void    archive_end         ( void );
      bool    validate_header     ( void );
      void    read_block          ( void );
      QString get_long_filename   ( void );
      QString format_permissions  ( const unsigned int, const bool );
      QString format_datetime     ( const unsigned int );
};
#endif

