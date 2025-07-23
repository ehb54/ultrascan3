/*  This is a highly customized version of GNU tar.  It has 
 *  been converted to Qt and C++. 
 *  It only includes the extract and create functions and NO options.
 *
 *  Since this file is derived from a GPLed application, this file is
 *  also licensed under the GPL.
 *
 *  Bruce Dubbs
 *  Univerity of Texas Health Science Center
 */

#include "us_tar.h"

#include <sys/stat.h>
#include <fcntl.h>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#ifdef Q_OS_WIN
#   include <io.h>
#   include <sys/utime.h>
#   include <sys/timeb.h>
#   include <BaseTsd.h>
#   include <winsock2.h>
#   define open    _open
#   define read    _read
#   define write   _write
#   define close   _close
#   define fstat   _fstat
#   define stat    _stat
#   define utime   _utime
#   define utimbuf _utimbuf
#   define umask   _umask
#   define ssize_t  SSIZE_T
#else
#   include <utime.h>
#   include <pwd.h>
#   include <grp.h>
#   include <sys/time.h>
#endif

#define LONGFILE "././@LongLink"

#include <time.h>

#include <vector>
using namespace std;

US_Tar::US_Tar()
{
}

int US_Tar::create( const QString& archive, const QString& directory,
                    QStringList* list )
{
   // Just put the directory in a list and create the file that way.
   QStringList dir;
   dir << directory;
   return create( archive, dir, list );
}

int US_Tar::create( const QString& archive, const QStringList& files,
                    QStringList* list )
{
   // To create the file, we do the following:
   // 1.  Open the archive filefor writing.  This will overwrite any existing
   //     tar file by the same name
   // 2.  For each file in the list
   //     a.  If the file is a directory, continue for each file in the directory
   //     b.  Write the header to the archive
   //     c.  Copy the file to the archive
   // 3.  Write two null headers (512 bytes)
   
   QStringList all;

   if ( list ) list->clear();

   for ( int i = 0; i < files.size(); i++ )
   {
      QString   current = files[ i ];
      QFileInfo f( current );

      if ( ! f.exists() ) return TAR_NOTFOUND;

      if ( f.isDir() )
      {
         // Remove any trailing slash
         if ( current.endsWith( "/" ) ) 
         {
            current = current.left( current.length() - 1 );
         }

         all << current;
         process_dir( current, all );
         continue;
      }

      if ( f.isHidden() )  // No hidden files
      {
         continue;   
      }

      // Append regular files to the list
      if ( f.isFile() )
      {
         all << current;
      }

      // Block and character devices, pipes, and symbolic links should be ignored
   }

   if ( list ) *list = all;
   // Process all files

#ifndef O_BINARY
# define O_BINARY 0
#endif

   ofd = open( archive.toLatin1().constData(), O_WRONLY | O_CREAT | O_BINARY, 0644 );
   
   if ( ofd < 0 ) return TAR_CANNOTCREATE;

   QStringList::iterator it = all.begin();
   blocks_written           = 0;

   try
   {

      while ( it != all.end() )
      {
         write_file ( *it++ );
      }

      archive_end();
   }
   catch ( int error )
   {
      close( ifd );
      close( ofd );
      unlink( archive.toLatin1().constData() );
      return error;
   }

   close( ofd );
   return TAR_OK;
}

void US_Tar::process_dir( const QString& path, QStringList& all )
{
  qDebug() << "In process dir: path -- " << path;
   QDir                  dir( path );
   QStringList           files = dir.entryList( QDir::Files | QDir::NoSymLinks );
   QStringList::Iterator it    = files.begin();

   // Add the files
   while(  it != files.end() )
   {
     all << path + "/" +  *it;
      it++;
   }

   QStringList dirs = dir.entryList( QDir::Dirs );
   it = dirs.begin();

   // Recurse into subdirectories
   while ( it != dirs.end() )
   {
      if ( *it != "." && *it != ".." )
      {
         all << path + "/" +  *it + "/";
         process_dir( path + "/" + *it, all );
      }

      it++;
   }
}

/* The bits in mode: */
#define TSUID   04000
#define TSGID   02000
#define TSVTX   01000
#define TUREAD  00400
#define TUWRITE 00200
#define TUEXEC  00100
#define TGREAD  00040
#define TGWRITE 00020
#define TGEXEC  00010
#define TOREAD  00004
#define TOWRITE 00002
#define TOEXEC  00001

void US_Tar::write_file( const QString& file )
{
   // Create and output the header
   QFileInfo f( file );

   struct stat stats;
   int ret = stat( file.toLatin1().constData(), &stats );
   if ( ret < 0 ) throw TAR_CANTSTAT;

   memset( (void*) tar_header.h, 0, sizeof( tar_header ) );

   // Populate the header
   if ( file.length() > (int)sizeof( tar_header.header.name ) - 1 )
      write_long_filename( file );

   strcpy( tar_header.header.name, file.toLatin1().constData() );

   int perms = TSUID  | TSGID   | TSVTX  |
      TUREAD | TUWRITE | TUEXEC |
      TGREAD | TGWRITE | TGEXEC |
      TOREAD | TOWRITE | TOEXEC ;

   sprintf ( tar_header.header.mode,  "%07o",  stats.st_mode & perms );
   sprintf ( tar_header.header.uid,   "%07o",  stats.st_uid );
   sprintf ( tar_header.header.gid,   "%07o",  stats.st_gid );
   sprintf ( tar_header.header.size,  "%011o", (unsigned int) stats.st_size );
   sprintf ( tar_header.header.mtime, "%011o", (unsigned int) stats.st_mtime );

   // Fill with blanks befor checksumming
   memcpy( &tar_header.header.chksum, "        ", sizeof tar_header.header.chksum );

   //char typeflag;      /* 156 */

   if      ( f.isDir()  ) tar_header.header.typeflag = '5';
   else if ( f.isFile() ) tar_header.header.typeflag = '0';
   else throw TAR_INTERNAL;

   //char linkname[100]; /* 157 */
   //char magic[6];      /* 257 */
   //char version[2];    /* 263 */

   memcpy( tar_header.header.magic,   "ustar ", 6 );
   memcpy( tar_header.header.version, " ",      2 );

#ifndef WIN32
   // uid and gid are always zero on WIN32 systems
   //char uname[32];     /* 265 */
   struct passwd* pwbuf = getpwuid( stats.st_uid );
   if ( pwbuf ) sprintf ( tar_header.header.uname, "%s", pwbuf->pw_name );

   //char gname[32];     /* 297 */
   struct group* grpbuf = getgrgid( stats.st_gid );
   if ( grpbuf ) sprintf ( tar_header.header.gname, "%s", grpbuf->gr_name );
#endif
   /* Fill in the checksum field.  It's formatted differently from the
    * other fields: it has [6] digits, a null, then a space -- rather than
    * digits, then a null. */
   
   //char chksum[8];
   int   sum = 0;
   char* p   = (char*) &tar_header;
   
   for ( int i = sizeof tar_header; i-- != 0; )
   {
      sum += 0xFF & *p++;
   }
   sprintf ( tar_header.header.chksum, "%06o", sum );
   
   // Copy the header to the buffer
   memcpy( (void*) ( buffer + blocks_written * BLOCK_SIZE ), 
           (void*) tar_header.h, 
           sizeof tar_header );

   // Write the buffer if it is full
   blocks_written++;
   if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();

   // Output the file
   if ( ! f.isDir() )
   {
      ifd = open( file.toLatin1().constData(), O_RDONLY | O_BINARY );
      if ( ifd < 0 ) throw TAR_READERROR;

      ssize_t input;
      int     full_blocks = stats.st_size / BLOCK_SIZE;
      int     space       = BLOCKING_FACTOR - blocks_written;

      if ( full_blocks > 0  &&  space >= full_blocks ) 
      {
         input = read( ifd, buffer + blocks_written * BLOCK_SIZE,
                       full_blocks * BLOCK_SIZE );
         if ( input != full_blocks * BLOCK_SIZE ) throw TAR_READERROR;
         blocks_written += full_blocks;
         if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();
      }
      else if ( full_blocks > 0 )
      {
         input = read( ifd, buffer + blocks_written * BLOCK_SIZE,
                       space * BLOCK_SIZE );
         blocks_written += space;
         full_blocks    -= space;
         flush_buffer();

         // Read/write in BLOCKING_FACTOR blocks at a time
         while ( full_blocks >= BLOCKING_FACTOR )
         {
            space = BLOCKING_FACTOR;
            input = read( ifd, buffer, space * BLOCK_SIZE );
            if ( input != space * BLOCK_SIZE ) throw TAR_READERROR;
            blocks_written += space;
            full_blocks    -= space;
            flush_buffer();
         }

         // Write out remaining full blocks
         input = read( ifd, buffer, full_blocks * BLOCK_SIZE );
         if ( input != full_blocks * BLOCK_SIZE ) throw TAR_READERROR;
         blocks_written += full_blocks;

         // We don't need ot flush here because we always have
         // less than BLOCKING_FACTOR blocks
      }

      // Now finish up any partial block
      input = read( ifd, buffer + blocks_written * BLOCK_SIZE, BLOCK_SIZE );
      if ( input < 0 ) throw TAR_READERROR;

      if ( input > 0 )
      {
         // Zero out anyting left
         memset( buffer + blocks_written * BLOCK_SIZE + input, 0, 
                 BLOCK_SIZE - input );
         blocks_written++;
         if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();
      }

      close( ifd );
   }
}

void US_Tar::write_long_filename( const QString& filename )
{
   // If there is a long filename, write a special header, followed by the 
   // necessary number of blocks that are needed for the filename

   // Add 1 for null byte teminating filename
   unsigned int length = (unsigned int) filename.length() + 1;

   //char name[100];     /*   0 */
   strcpy( tar_header.header.name, LONGFILE );
   
   //char mode[8];       /* 100 */
   //char uid[8];        /* 108 */
   //char gid[8];        /* 116 */
   //char size[12];      /* 124 */
   //char mtime[12];     /* 136 */

   sprintf ( tar_header.header.mode,   "%07o", 0 );
   sprintf ( tar_header.header.uid,    "%07o", 0 );
   sprintf ( tar_header.header.gid,    "%07o", 0 );
   sprintf ( tar_header.header.size,  "%011o", length );
   sprintf ( tar_header.header.mtime, "%011o", 0 );

   // Fill with blanks befor checksumming
   memcpy( &tar_header.header.chksum, "        ", sizeof tar_header.header.chksum );

   //char typeflag;      /* 156 */

   tar_header.header.typeflag = 'L';

   //char linkname[100]; /* 157 */
   //char magic[6];      /* 257 */
   //char version[2];    /* 263 */

   memcpy( tar_header.header.magic,   "ustar ", 6 );
   memcpy( tar_header.header.version, " ",      2 );

   //char uname[32];     /* 265 */
   //char gname[32];     /* 297 */
   strcpy ( tar_header.header.uname, "root" );
   strcpy ( tar_header.header.gname, "root" );

   /* Fill in the checksum field.  It's formatted differently from the
    * other fields: it has [6] digits, a null, then a space -- rather than
    * digits, then a null. */

   //char chksum[8];
   int   sum = 0;
   char* p   = (char*) &tar_header;

   for ( int i = sizeof tar_header; i-- != 0; )
   {
      sum += 0xFF & *p++;
   }

   sprintf ( tar_header.header.chksum, "%06o", sum );
   
   // Copy the header to the buffer
   memcpy( (void*) ( buffer + blocks_written * BLOCK_SIZE ), 
           (void*) tar_header.h, 
           sizeof tar_header );

   // Write the buffer if it is full
   blocks_written++;
   if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();

   // Now write the filename in one of more blocks
   int full_blocks = length / BLOCK_SIZE;

   for ( int i = 0; i < full_blocks; i++ )
   {
      memset( (void*) tar_header.h, 0, sizeof tar_header );
      
      memcpy( (void*) tar_header.h,
              (void*) filename.mid( i * BLOCK_SIZE, BLOCK_SIZE ).constData(),
              BLOCK_SIZE );

      // Copy the header to the buffer
      memcpy( (void*) ( buffer + blocks_written * BLOCK_SIZE ), 
              (void*) tar_header.h, 
              sizeof tar_header );

      // Write the buffer if it is full
      blocks_written++;
      if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();
   }

   // Copy what is left
   if (  ( length ) % BLOCK_SIZE ) 
   {
      memset( (void*) tar_header.h, 0, sizeof tar_header );

      memcpy( (void*) tar_header.h, 
              (void*) filename.mid( full_blocks * BLOCK_SIZE ).constData(),
              length % BLOCK_SIZE );

      // Copy the header to the buffer
      memcpy( (void*) ( buffer + blocks_written * BLOCK_SIZE ), 
              (void*) tar_header.h, 
              sizeof tar_header );

      // Write the buffer if it is full
      blocks_written++;
      if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();
   }

   // Finally write the first 100 bytes of the original filename
   memset( (void*) tar_header.h, 0, sizeof( tar_header ) );
   memcpy( (void*) tar_header.h, filename.toLatin1().constData(), 100 ); 
}

void US_Tar::archive_end( void )
{
   // Write a null block
   void* location = (void*) ( buffer + blocks_written * BLOCK_SIZE );
   memset( location, 0, BLOCK_SIZE );
   blocks_written++;

   if ( blocks_written == BLOCKING_FACTOR ) flush_buffer();

   // Write a second null block and finish
   location = (void*) ( buffer + blocks_written * BLOCK_SIZE );
   memset( location, 0, BLOCK_SIZE );
   blocks_written++;
   flush_buffer();
}


void US_Tar::flush_buffer( void )
{
   if ( blocks_written == 0 ) return;

   if ( blocks_written < BLOCKING_FACTOR )
   {
      size_t size     = ( BLOCKING_FACTOR - blocks_written ) * BLOCK_SIZE;
      void*  location = (void*) ( buffer + blocks_written * BLOCK_SIZE );
      memset( location, 0, size );
   }

   int ret = write( ofd, buffer, sizeof buffer );

   if ( ret != sizeof buffer ) throw TAR_WRITEERROR;

   blocks_written = 0;
}

///////////////////////////
int US_Tar::extract( const QString& archive, QStringList* list, const QString& outpath)
{
   /* 1. Open the archve
    * 2. while header is not null
    *    a.  read header
    *        i. validate chksum
    *    b.  if file
    *        i.  copy from archive to file
    *        ii. fix permissions, owner, group, size, and date
    *    c.  if directory
    *          i.  mkdir
    *          ii. fix owner, group
    * 3. fix directory times
    */
  
   ofd = -1;  // Initialize output file to closed
   ifd = open( archive.toLatin1().constData(), O_RDONLY | O_BINARY );
   if ( ifd < 0 ) return TAR_NOTFOUND;

   if ( list ) list->clear();

   blocks_read = 0;
   QStringList files;
   
   QStringList dirs;
   vector<int> times;

   QDir outdir;
   if (outpath.isEmpty()) {
      outdir = QDir(".");
   } else {
      outdir = QDir(outpath);
   }
   if (! outdir.exists()) {
      outdir.mkpath(outdir.absolutePath());
   }

   try
   {
      while ( true )
      {
         // Read header
         read_block();

         // Validate checksum
         bool zero = validate_header();

         // The archive ends with two zero blocks
         if ( zero )
         {
            read_block();
            bool second_zero = validate_header();
            
            if ( second_zero ) 
            {
               break;
            }
            else 
            {
               throw TAR_ARCHIVEERROR;
            }
         }

         // Now get the data from the header

         QString filename;

         if ( tar_header.header.typeflag == 'L' )
            filename = get_long_filename();
         else
            filename = tar_header.header.name;

         QString uname    = tar_header.header.uname;
         QString gname    = tar_header.header.gname;

         unsigned int mode;
#ifndef WIN32
         uid_t        uid;
         gid_t        gid;
#endif
         unsigned int fsize;
         unsigned int mtime;
         
         sscanf( tar_header.header.mode,   "%7o", &mode  );
#ifndef WIN32
         sscanf( tar_header.header.uid,    "%7o", &uid   );
         sscanf( tar_header.header.gid,    "%7o", &gid   );
#endif
         sscanf( tar_header.header.size,  "%11o", &fsize );
         sscanf( tar_header.header.mtime, "%11o", &mtime );

         bool directory;
         switch ( tar_header.header.typeflag )
         {
         case '5':
            directory = true;
            mode |= 0111; // Make sure directory is executable
            break;

         case '0':
            directory = false;
            break;

         default:
            throw TAR_ARCHIVEERROR;
         }

         files << filename;   // Save the file name for error processing
         if ( list ) list->append( filename ); // Return list if requested

         if ( directory )
         {
            QDir f( "." );
            if ( ! f.exists( outdir.absoluteFilePath( filename ) ) )
            {
               bool success = f.mkpath(outdir.absoluteFilePath( filename ) );
               if ( ! success ) throw TAR_MKDIRFAILED;
            }

            // Save directory name and mode to restore at the end
            dirs.append( filename );
            times.push_back( mtime );
         }
         else // It's a file.  Create it.
         {
            const QString absfpath = outdir.absoluteFilePath( filename );
            QFileInfo finfo( absfpath );
            QDir dir = finfo.absoluteDir();
            if ( ! dir.exists() ) {
               dir.mkdir( dir.absolutePath() );
            }
            int flags = O_WRONLY | O_CREAT | O_BINARY | O_TRUNC;
            ofd = open( absfpath.toLatin1().constData(), flags, 0644 );
            if ( ofd < 0 ) throw TAR_WRITEERROR;

            // Copy from archive to file

            unsigned int bytes_to_write = fsize;
            int          skip           = BLOCK_SIZE - fsize % BLOCK_SIZE;

            if (  skip == BLOCK_SIZE ) skip = 0;  // If file size is exact multple of blocks

            int size;

            while ( bytes_to_write > sizeof buffer )
            {
               size = read( ifd, buffer, sizeof buffer );
               if ( size != sizeof buffer ) throw TAR_READERROR;

               size = write( ofd, buffer, sizeof buffer );
               if ( size != sizeof buffer ) throw TAR_WRITEERROR;

               bytes_to_write -= sizeof buffer;
            }

            size = read( ifd, buffer, bytes_to_write );
            if ( size != (int) bytes_to_write ) throw TAR_READERROR;

            size = write( ofd, buffer, bytes_to_write );
            if ( size != (int) bytes_to_write ) throw TAR_WRITEERROR;

            // Skip to start of next block
            lseek( ifd, skip, SEEK_CUR );

            // Clost output file
            close( ofd );
            ofd = -1;  // Mark as closed
         }

         // Fix permissions, owner, group, and date
         // We do the utime before the chmod because some versions of utime are
         // broken and trash the modes of the file.  

         // Get current time
         struct timeval tv;
#ifndef WIN32
         gettimeofday( &tv, NULL );
#else
         struct _timeb timebuffer;
         _ftime(&timebuffer);
         tv.tv_sec  = timebuffer.time;
         tv.tv_usec = timebuffer.millitm * 1000;
#endif
         // Set the times on the file
         struct utimbuf time;
         time.actime  = tv.tv_sec;  // now
         time.modtime = mtime;
         utime( filename.toLatin1().constData(), &time );

#ifndef WIN32
         // Update permissions
         chmod( filename.toLatin1().constData(), mode );

         // Update owner/group
         if ( geteuid() != 0 ) uid = (uid_t) -1;
         int choerr = chown( filename.toLatin1().constData(), uid, gid );
         if ( choerr != 0 )    uid = (uid_t) -1;
#endif
      }  // while ( true )
   }
   catch ( int error )
   {
      close( ifd );
      if ( ofd > 0 ) close( ofd );

      // Cycle through files and delete everything created
      for ( size_t i = files.size() -1 ; i <= 0; i++ )
      {
         unlink( files[i].toLatin1().constData() );
      }

      return error;
   }

   close( ifd );

   // Fix directory times
   for ( int i = 0; i < dirs.size(); i++ )
   {
      struct utimbuf time;
      time.actime  = times[ i ];  
      time.modtime = times[ i ];

      utime( dirs[ i ].toLatin1().constData(), &time );
   }

   return TAR_OK;
}

/////////////////////////////
int US_Tar::list( const QString& archive, QStringList& files )
{
   ifd = open( archive.toLatin1().constData(), O_RDONLY | O_BINARY );
   if ( ifd < 0 ) return TAR_NOTFOUND;

   blocks_read = 0;

   try
   {
      while ( true )
      {
         // Read header
         read_block();
         bool zero = validate_header();

         // The archive ends with two zero blocks
         if ( zero )
         {
            read_block();
            bool second_zero = validate_header();

            if ( second_zero ) 
            {
               break;
            }
            else 
            {
               throw TAR_ARCHIVEERROR;
            }
         }

         // Now get the data from the header
         QString filename;   

         if ( tar_header.header.typeflag == 'L' )
            filename = get_long_filename();
         else
            filename = tar_header.header.name;

         QString uname    = tar_header.header.uname;
         QString gname    = tar_header.header.gname;

         unsigned int mode;
         unsigned int fsize;
         unsigned int mtime;
         
         sscanf( tar_header.header.mode,   "%7o", &mode  );
         sscanf( tar_header.header.size,  "%11o", &fsize );
         sscanf( tar_header.header.mtime, "%11o", &mtime );

         bool directory;
         switch ( tar_header.header.typeflag )
         {
         case '5':
            directory = true;
            break;

         case '0':
            directory = false;
            break;

         default:
            throw TAR_ARCHIVEERROR;
         }

         QString s;

         // perms user/group size date time filename
         files << format_permissions( mode, directory ) + " " +
            uname + "/" + gname                   + " " +
            s.sprintf( "%10d", fsize )            + " " +
            format_datetime( mtime )              + " " +
            filename;

         if ( ! directory )
         {
            // Skip file data
            unsigned int fsize;
            sscanf( tar_header.header.size,  "%11o", &fsize );

            int skip = BLOCK_SIZE - fsize % BLOCK_SIZE;

            // If file size is exact multple of blocks
            if (  skip == BLOCK_SIZE ) skip = 0;

            // Skip to start of next block
            lseek( ifd, fsize + skip, SEEK_CUR );
         }
      }  // while ( true )
   }
   catch ( int error )
   {
      close( ifd );
      return error;
   }

   close( ifd );
   return TAR_OK;
}

QString US_Tar::format_permissions( const unsigned int mode, const bool dir )
{
   QString s = "----------";
   if ( dir )         s = s.replace( 0, 1, "d" );
   if ( mode & 0400 ) s = s.replace( 1, 1, "r" );
   if ( mode & 0200 ) s = s.replace( 2, 1, "w" );
   if ( mode & 0100 ) s = s.replace( 3, 1, "x" );
   if ( mode & 0040 ) s = s.replace( 4, 1, "r" );
   if ( mode & 0020 ) s = s.replace( 5, 1, "w" );
   if ( mode & 0010 ) s = s.replace( 6, 1, "x" );
   if ( mode & 0004 ) s = s.replace( 7, 1, "r" );
   if ( mode & 0002 ) s = s.replace( 8, 1, "w" );
   if ( mode & 0001 ) s = s.replace( 9, 1, "x" );

   return s;
}

QString US_Tar::format_datetime( const unsigned int mtime )
{
   time_t     time = mtime;
   struct tm* t    = localtime( &time );
   
   QString s;
   s = s.sprintf( "%04d-%02d-%02d %02d:%02d", 
                  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min );
   return s;
}

// The return boolean here is if header is zero
bool US_Tar::validate_header( void )
{
   // Validate checksum
   unsigned char* p = tar_header.h;
   int            unsigned_sum = 0;
   unsigned int   i;

   for ( i = sizeof tar_header; i-- != 0; )
   {
      unsigned_sum += (unsigned char) (*p++);
   }

   // Signal that the end of archive has arived.  Verify with another
   // empty block

   if ( unsigned_sum == 0 ) return true;

   // Adjust checksum to count the "chksum" field as blanks.
   for ( i = sizeof tar_header.header.chksum; i-- != 0; )
   {
      unsigned_sum -= (unsigned char) tar_header.header.chksum[i];
   }

   unsigned_sum += ' ' * sizeof tar_header.header.chksum;

   // Get the checksum from the header and compare to calculated sum
   unsigned int parsed_sum;
   sscanf( tar_header.header.chksum, "%6o", &parsed_sum );

   if ( parsed_sum != (unsigned int ) unsigned_sum )
   {
      throw TAR_ARCHIVEERROR;
   }

   // And check the magic string
   QString magic = tar_header.header.magic;
   if ( magic != "ustar  " ) 
   {
      throw TAR_ARCHIVEERROR;
   }

   return false;  // Header ok and not zero
}

QString US_Tar::get_long_filename( void )
{
   QString filename;

   unsigned int length;
   sscanf( tar_header.header.size,  "%11o", &length );

   // Skip to next header

   int final_block = ( length % BLOCK_SIZE ) ? 1 : 0;
   int blocks      = length / BLOCK_SIZE + final_block;
   
   for ( int i = 0; i < blocks; i++ )
   {
      read_block();
      filename.append( (char*) tar_header.h );
   }

   read_block();
   return filename;
}

void US_Tar::read_block( void )
{
   ssize_t size = read( ifd, tar_header.h, BLOCK_SIZE );
   if ( size != BLOCK_SIZE ) throw TAR_READERROR;
}

/////////////////////////////
QString US_Tar::explain( const int error )
{
   QString explanation;
   switch ( error )
   {
   case TAR_OK:
      explanation = "The (un)tar operation was successful.";
      break;

   case TAR_CANNOTCREATE:
      explanation = "Could not create the output file." ;
      break;

   case   TAR_NOTFOUND:
      explanation = "Could not find the input file." ;
      break;

   case TAR_CANTSTAT:
      explanation = "Could not determine the input file status." ;
      break;

   case TAR_FILENAMETOOLONG:
      explanation = "A file name was too long." ;
      break;

   case TAR_INTERNAL:
      explanation = "The internal file type was not a file or a directory." ;
      break;
         
   case TAR_READERROR:
      explanation = "Could not read input file." ;
      break;

   case TAR_WRITEERROR:
      explanation = "Could not write an output file." ;
      break;

   case TAR_ARCHIVEERROR:
      explanation = "The archive was not formatted properly." ;
      break;

   case TAR_MKDIRFAILED:
      explanation = "Could not create a directory." ;
      break;

   default:
      explanation = "Unknown return code: "
                    + QString::number( error );
   }

   return explanation;
}
