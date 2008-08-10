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

#include "../include/us_tar.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h> 

#include <sys/stat.h>
#include <fcntl.h>

#ifdef WIN32
#   include <io.h>
#   include <sys/utime.h>
#   include <sys/timeb.h>
#   include <BaseTsd.h>
#   include <winsock2.h>
#   define utime   _utime
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

#include <time.h>


#include <vector>
#include <iostream>
using namespace std;

int US_Tar::create( const QString& archive, const QString& directory )
{
	// Just put the directory in a list and create the file that way.
	QStringList dir;
	dir << directory;
	return create( archive, dir );
}

int US_Tar::create( const QString& archive, const QStringList& files )
{

  // To crate the file, we do the following:
	// 1.  Open the archive filefor writing.  This will overwrite any existing
	//     tar file by the same name
	// 2.  For each file in the list
	//     a.  If the file is a directory, continue for each file in the directory
  //     b.  Write the header to the archive
	//     c.  Copy the file to the archive
	// 3.  Write two null headers (512 bytes)
	
  QStringList all;

	for ( unsigned int i = 0; i < files.size(); i++ )
	{
    QString   current = files[ i ];
		QFileInfo f( current );

    if ( ! f.exists() )
		{
			return TAR_NOTFOUND;
		}

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

	// Debug -- list the files
	QStringList::Iterator it = all.begin();
	//while ( it != all.end() ) cout << *it++ << endl;

  // Process all files

#ifndef O_BINARY
# define O_BINARY 0
#endif

	umask( 0133 );  // Permissions should be 644
	ofd = open( archive.latin1(), O_WRONLY | O_CREAT | O_BINARY, 0644 );
	//cout << "ofd=" << ofd << endl;
	if ( ofd < 0 ) return TAR_CANNOTCREATE;

	it             = all.begin();
	blocks_written = 0;

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
		unlink( archive.latin1() );
		return error;
	}

	close( ofd );
  return TAR_OK;
}

void US_Tar::process_dir( const QString& path, QStringList& all )
{
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
	// Crate and output the header
	QFileInfo f( file );

	struct stat stats;
	int ret = stat( file.latin1(), &stats );
	if ( ret < 0 ) throw TAR_CANTSTAT;

	memset( (void*) tar_header.h, 0, sizeof( tar_header ) );
	
	// Populate the header
	if ( file.length() > sizeof( tar_header.header.name ) - 1 )
		throw TAR_FILENAMETOOLONG;

	strcpy( tar_header.header.name, file.latin1() );

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
	
	sprintf ( tar_header.header.magic, "ustar  " );

#ifndef WIN32
  // uid and gid are always zero on WIN32 systems
	//char uname[32];     /* 265 */
	struct passwd* pwbuf = getpwuid( stats.st_uid );
	sprintf ( tar_header.header.uname, "%s", pwbuf->pw_name );

	//char gname[32];     /* 297 */
	struct group* grpbuf = getgrgid( stats.st_gid );
	sprintf ( tar_header.header.gname, "%s", grpbuf->gr_name );
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
		ifd = open( file.latin1(), O_RDONLY | O_BINARY );
		if ( ifd < 0 ) throw TAR_READERROR;

		ssize_t input;
		int full_blocks = stats.st_size / BLOCK_SIZE;
    int space       = BLOCKING_FACTOR - blocks_written;

		//cout << "full_blocks=" << full_blocks << "; space=" << space << endl;
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

		//printf( "buf addr=0x%08x; offset=%08x\n", &(buffer[0]), blocks_written * BLOCK_SIZE );
		//cout << "input=" << input << endl;
		//cout << "final blocks_written=" << blocks_written << endl;
		close( ifd );
	}
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
	flush_buffer();
}


void US_Tar::flush_buffer( void )
{
	//cout << "blocks written=" << blocks_written << endl;
	if ( blocks_written == 0 ) return;

  if ( blocks_written < BLOCKING_FACTOR )
	{
		size_t size     = ( BLOCKING_FACTOR - blocks_written ) * BLOCK_SIZE;
		void*  location = (void*) ( buffer + blocks_written * BLOCK_SIZE );
		memset( location, 0, size );
	}

	//cout << "starting write, ofd = " << ofd << endl;
	int ret = write( ofd, buffer, sizeof buffer );
	//cout << endl << "ending write" << endl;
	if ( ret != sizeof buffer )
	{
		throw TAR_WRITEERROR;
	}

	blocks_written = 0;
}


int US_Tar::extract( const QString& archive )
{
	/* 1. Open the archve
	 * 2. while header is not null
	 *    a.  read header
	 *        i. validate chksum
	 *    b.  if file
	 *        i.  copy from archive to file
	 *        ii. fix permissions, owner, group, size, and date
	 *    c.  if directory
	 *    		i.  mkdir
	 *    		ii. fix owner, group
	 * 3. fix directory times
	 */
  
  ofd = -1;  // Initialize output file to closed
	ifd = open( archive.latin1(), O_RDONLY | O_BINARY );
	if ( ifd < 0 ) return TAR_NOTFOUND;

	blocks_read = 0;
	QStringList files;
	
	QStringList dirs;
	vector<int> times;

	try
	{
		while ( true )
		{
			// Read header
			ssize_t size = read( ifd, tar_header.h, BLOCK_SIZE );
			if ( size != BLOCK_SIZE ) throw TAR_READERROR;

			// Validate checksum
			unsigned char* p = tar_header.h;
		  int            unsigned_sum;
			unsigned int   i;

      unsigned_sum = 0;

			for ( i = sizeof tar_header; i-- != 0; )
			{
			   unsigned_sum += (unsigned char) (*p++);
			}

			// Signal that the end of archive has arived.  Verify with another
			// empty block
      
			if ( unsigned_sum == 0 )
			{
				size = read( ifd, tar_header.h, BLOCK_SIZE );
			  if ( size != BLOCK_SIZE ) throw TAR_READERROR;

        p = tar_header.h;
				for ( i = sizeof tar_header; i-- != 0; )
			  {
				   unsigned_sum += (unsigned char) *p;
				}
				
				if ( unsigned_sum == 0 ) break;
				else 
        {
          throw TAR_ARCHIVEERROR;
        }
			}

			// Adjust checksum to count the "chksum" field as blanks.
			for ( i = sizeof tar_header.header.chksum; i-- != 0; )
			{
				unsigned_sum -= (unsigned char) tar_header.header.chksum[i];
			}

			unsigned_sum += ' ' * sizeof tar_header.header.chksum;

			// Get the checksum from the header and compare to calculated sum
			unsigned int parsed_sum;
			int count = sscanf( tar_header.header.chksum, "%6o", &parsed_sum );
 
			if ( parsed_sum != unsigned_sum )
      {
        throw TAR_ARCHIVEERROR;
      }

			// And check the magic string
			QString magic = tar_header.header.magic;
			if ( magic != "ustar  " ) 
      {
        throw TAR_ARCHIVEERROR;
      }

			// Now get the data from the header

		  QString filename = tar_header.header.name;	
			QString uname    = tar_header.header.uname;
			QString gname    = tar_header.header.gname;

			unsigned int mode;
#ifndef WIN32
      uid_t        uid;
			gid_t        gid;
#endif
			unsigned int fsize;
			unsigned int mtime;
			
			count = sscanf( tar_header.header.mode,   "%7o", &mode  );
#ifndef WIN32
      count = sscanf( tar_header.header.uid,    "%7o", &uid   );
			count = sscanf( tar_header.header.gid,    "%7o", &gid   );
#else
      count = sscanf( tar_header.header.uid,    "0000000" );
			count = sscanf( tar_header.header.gid,    "0000000" );
#endif
			count = sscanf( tar_header.header.size,  "%11o", &fsize );
			count = sscanf( tar_header.header.mtime, "%11o", &mtime );

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

			files << filename;	// Save the file name for error processing

			if ( directory )
			{
				QDir f( "." );
				if ( ! f.exists( filename ) ) 
				{
					bool success = f.mkdir( filename );
					if ( ! success ) throw TAR_MKDIRFAILED;
				}

				// Save directory name and mode to restore at the end
				dirs.append( filename );
				times.push_back( mtime );
			}
			else // It's a file.  Create it.
			{
				ofd = open( filename.latin1(), O_WRONLY | O_CREAT | O_BINARY, 0644 );
				if ( ofd < 0 ) throw TAR_WRITEERROR;

	 			// Copy from archive to file
	 
	 			unsigned int bytes_to_write = fsize;
				int          skip           = BLOCK_SIZE - fsize % BLOCK_SIZE;

        if (  skip == BLOCK_SIZE ) skip = 0;  // If file size is exact multple of blocks

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
      utime( filename.latin1(), &time );
	
#ifndef WIN32
			// Update permissions
      chmod( filename.latin1(), mode );

			// Update owner/group
  		if ( geteuid() != 0 ) uid = (uid_t) -1;
			chown( filename.ascii(), uid, gid );
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
			unlink( files[i].latin1() );
		}

		return error;
	}

  close( ifd );

	// Fix directory times
	
	

	for ( unsigned int i = 0; i < dirs.size(); i++ )
	{
		struct utimbuf time;
		time.actime  = times[ i ];  
		time.modtime = times[ i ];

		utime( dirs[ i ].latin1(), &time );
	}

	return TAR_OK;
}

