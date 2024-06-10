//! \file us_tar.h
#ifndef US_TAR_H
#define US_TAR_H

#include <QtCore>
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

//! A class to provide tar functions internally to a routine. The functions
//! provided are create, extract, and list.
class US_UTIL_EXTERN US_Tar
{
   public:
      US_Tar();

      //! Create a tar file from a directory
      //!
      //! \param archive   The name of the tar file to be created.
      //! \param directory The name of the directory to archive.
      //! \param list      The optional output list of archived files.
      //! \return          An error code. Zero for no error (see explain method)
      int     create ( const QString&, const QString&, QStringList* = 0 ); 

      //! Create a tar file from a list of files (including directories)
      //!
      //! \param archive The name of the tar file to be created.
      //! \param files   A list of names of files/directories to archive.
      //! \param list    The optional output list of archived files.
      //! \return        An error code. Zero for no error (see explain method).
      int     create ( const QString&, const QStringList&, 
                              QStringList* = 0 );
      
      //! Extract from a tar file the files archived.
      //!
      //! \param archive The name of the tar file from which to extract.
      //! \param path    The path that extracted data is saved.
      //! \param list    The optional output list of extracted files.
      //! \return        An error code. Zero for no error (see explain method).
      int     extract( const QString&, QStringList* = 0, const QString& = 0);
      
      //! List the files archived in a tar file.
      //!
      //! \param archive The name of the tar file whose content is to be listed.
      //! \param files   The output list of archived files.
      //! \return        An error code. Zero for no error (see explain method).
      int     list   ( const QString&, QStringList& ); 
      
      //! Explain in a message string the tar function error that occurred.
      //!
      //! \param error The error code to be explained.
      //! \return      A string documenting the nature of the error.
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
      int           archive_size;

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

