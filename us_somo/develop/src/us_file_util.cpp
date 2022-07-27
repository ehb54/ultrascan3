#include "../include/us_file_util.h"
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>

bool US_File_Util::copy( QString from, QString to, bool overwrite, QString qsheader )
{
   char header[ qsheader.length() + 1 ];
   header[ 0 ] = 0;

   if ( !qsheader.isEmpty() ) {
      strcpy( header, qsheader.toLatin1().data() );
         ;
   }
   errormsg = "";
   if ( !QFile( from ).exists() )
   {
      errormsg = QString( "Copy from file %1 does not exist (current dir %2, copying to %3)" )
         .arg( from )
         .arg( QDir::current().path() )
         .arg( to )
         ;
      return false;
   }

   if ( QFileInfo( to ).isDir() )
   {
      to = to + QDir::separator() + QFileInfo( from ).fileName();
   }

   if ( !overwrite && 
        QFile( to ).exists() &&
        !QFileInfo( to ).isDir() )
   {
      errormsg = QString( "Copy to file %1 exists" ).arg( to );
      return false;
   }

   // use FILE since QFile doesn't always seem to copy files correctly
   // had a case with binary files getting an extra 2 bytes
   // this caused US_Gzip::extract() to fail
   // I didn't try QFile::readBlock, ::writeBlock, so that may make the difference

#if defined ( Q_OS_WIN )
   FILE *fin = us_fopen( from, "rb" );
#else 
   FILE *fin = us_fopen( from, "r" );
#endif
   if ( !fin )
   {
      errormsg = QString( "Copy: could not open %1 for reading" ).arg( from );
      return false;
   }

#if defined ( Q_OS_WIN )
   FILE *fout = us_fopen( to, "wb" );
#else
   FILE *fout = us_fopen( to, "w" );
#endif
   if ( !fout )
   {
      errormsg = QString( "Copy: could not open %1 for writing" ).arg( from );
      fclose( fin );
      return false;
   }
      
   char buf[ 2048 ];
   size_t bytes_read;
   size_t bytes_written;

   if ( strlen( header ) ) {
      const char * buf = header;
      size_t bytes_read = strlen( header );
      size_t pos = 0;
      size_t bytes_left = bytes_read;
      while ( bytes_left && 
              ( bytes_written = fwrite( buf + pos, 1, bytes_left, fout ) ) )
      {
         if ( bytes_left >= bytes_written )
         {
            bytes_left -= bytes_written;
         } else {
            bytes_left = 0;
            errormsg = QString( "Copy: Error reading file %1 read overrun" ).arg( from );
            fclose( fin );
            fclose( fout );
            QFile::remove( to );
            return false;
         }
         pos += bytes_written;
      }
   }      
      
   while ( ( bytes_read = fread( buf, 1, 2048, fin ) ) )
   {
      size_t pos = 0;
      size_t bytes_left = bytes_read;
      while ( bytes_left && 
              ( bytes_written = fwrite( buf + pos, 1, bytes_left, fout ) ) )
      {
         if ( bytes_left >= bytes_written )
         {
            bytes_left -= bytes_written;
         } else {
            bytes_left = 0;
            errormsg = QString( "Copy: Error reading file %1 read overrun" ).arg( from );
            fclose( fin );
            fclose( fout );
            QFile::remove( to );
            return false;
         }
         pos += bytes_written;
      }
   }

   if ( !feof( fin ) )
   {
      errormsg = QString( "Copy: Error reading file %1" ).arg( from );
      fclose( fin );
      fclose( fout );
      QFile::remove( to );
      return false;
   }

   fclose( fin );
   fclose( fout );
   return true;
}

bool US_File_Util::copy( QStringList from, QString toDir, bool overwrite )
{
   errormsg = "";
   if ( !QFileInfo( toDir ).isDir() )
   {
      errormsg = QString( "Copy: destination %1 is not a directory for a multi file copy" ).arg( toDir );
      return false;
   }

   for ( unsigned int i = 0; i < (unsigned int)from.size(); i++ )
   {
      if ( !copy( from[ i ], toDir, overwrite ) )
      {
         return false;
      }
   }
   return true;
}

bool US_File_Util::move( QString from, QString to, bool overwrite )
{
   errormsg = "";
   if ( !copy( from, to, overwrite ) )
   {
      return false;
   }

   if ( !QFile::remove( from ) )
   {
      errormsg = QString( "Could not remove file %1" ).arg( from );
      return false;
   }
   return true;
}

bool US_File_Util::move( QStringList from, QString toDir, bool overwrite )
{
   errormsg = "";
   if ( !copy( from, toDir, overwrite ) )
   {
      return false;
   }

   for ( unsigned int i = 0; i < (unsigned int)from.size(); i++ )
   {
      if ( !QFile::remove( from[ i ] ) )
      {
         errormsg += QString( "Could not remove file %1\n" ).arg( from[ i ] );
      }
   }
   if ( !errormsg.isEmpty() )
   {
      return false;
   }
   return true;
}

bool US_File_Util::read( QString from, QStringList &qsl )
{
   errormsg = "";
   qsl.clear( );
   QFile f( from );
   if ( !f.open(  QIODevice::ReadOnly | QIODevice::Text ) )
   {
      errormsg = QString( "US_File_Util:read() can not open %1 for reading" ).arg( from );
      return false;
   }

   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qsl << qs;
   }
   f.close();
   return true;
}

bool US_File_Util::writeuniq( QString & outname, QString & msg, QString basename, QString ext, QString & data )
{
   outname = basename + "." + ext;
   int count = 0;
   while( QFile::exists( outname ) )
   {
      outname = QString( "%1-%2.%3" ).arg( basename ).arg( ++count ).arg( ext );
   }

   QFile f( outname );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      msg = QString( "could not open file %1 for writing" ).arg( outname );
      return false;
   }
   QTextStream ts( &f );
   ts << data;
   f.close();
   return true;
}

US_Log::US_Log( QString name, bool autoflush, OpenMode mode )
{
   this->autoflush = autoflush;

   f.setFileName( name );
   if ( !f.open( QIODevice::WriteOnly | mode ) )
   {
      error_msg = QString( "Error opening file %1 for writing" ).arg( name );
      return;
   }
   ts = new QTextStream( & f );
}

US_Log::~US_Log()
{
   if ( error_msg.isEmpty() )
   {
      f.close();
      delete ts;
   }
}

void US_Log::datetime( QString qs )
{
   if ( error_msg.isEmpty() )
   {
      (*ts) << QDateTime::currentDateTime().toString();
      if ( !qs.isEmpty() )
      {
         (*ts) << " : ";
      }
      log( qs );
   }
}

void US_Log::time( QString qs )
{
   if ( error_msg.isEmpty() )
   {
      (*ts) << QTime::currentTime().toString();
      if ( !qs.isEmpty() )
      {
         (*ts) << " : ";
      }
      log( qs );
   }
}

void US_Log::log( const QString & qs )
{
   if ( error_msg.isEmpty() )
   {
      (*ts) << qs;
      if ( !qs.contains( QRegExp( "\\n$" ) ) )
      {
         (*ts) << Qt::endl;
      }
      if ( autoflush )
      {
         (*ts) << Qt::flush;
      }
   }
}

void US_Log::log( const QStringList & qsl )
{
   const QString qs = qsl.join( "\n" );
   log( qs );
}

void US_Log::log( const char * c )
{
   log( QString( c ) );
}

void US_Log::flushon()
{
   autoflush = true;
}

void US_Log::flushoff()
{
   autoflush = false;
}

bool US_File_Util::getcontents( QString file, QString & contents, QString & error ) {
   error = "";
   contents = "";
   QFile f( file );
   if ( !f.exists() ) {
      error = QString( "File %1 does not exist" ).arg( file );
      return false;
   }
   if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )  {
      error = QString( "File %1 could not be opened" ).arg( file );
      return false;
   }
   QTextStream ts( &f );
   contents = ts.readAll();
   f.close();
   return true;
}

bool US_File_Util::putcontents( QString file, QString & contents, QString & error ) {
   error = "";
   QFile f( file );
   if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )  {
      error = "File could not be opened for writing";
      return false;
   }
   QTextStream ts( &f );
   ts << contents;
   f.close();
   return true;
}
   
bool US_File_Util::diff( QString file1, QString file2 ) {
   QString error;
   QString contents1;
   QString contents2;

   getcontents( file1, contents1, error );
   getcontents( file2, contents2, error );
   
   return contents1 != contents2;
}
