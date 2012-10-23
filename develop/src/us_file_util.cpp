#include "../include/us_file_util.h"


bool US_File_Util::copy( QString from, QString to, bool overwrite )
{
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

#if defined ( Q_WS_WIN )
   FILE *fin = fopen( from, "rb" );
#else 
   FILE *fin = fopen( from, "r" );
#endif
   if ( !fin )
   {
      errormsg = QString( "Copy: could not open %1 for reading" ).arg( from );
      return false;
   }

#if defined ( Q_WS_WIN )
   FILE *fout = fopen( to, "wb" );
#else
   FILE *fout = fopen( to, "w" );
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

   for ( unsigned int i = 0; i < from.size(); i++ )
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

   for ( unsigned int i = 0; i < from.size(); i++ )
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
