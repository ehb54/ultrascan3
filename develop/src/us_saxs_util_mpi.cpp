#include "../include/us_saxs_util.h"

#include <mpi.h>
extern int npes;
extern int myrank;

#define SLASH QDir::separator()

bool US_Saxs_Util::run_iq_mpi( QString controlfile )
{
   errormsg = "";
   if ( !controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
   {
      errormsg = QString( "controlfile must be .tgz, was %1" ).arg( controlfile );
      return false;
   }

   if ( !QFile::exists( controlfile ) )
   {
      errormsg = "controlfile must be .tgz";
      return false;
   }

   QStringList qslt;
   unsigned int sizeoflist;
   int errorno = -1;

   if ( myrank )
   {
      // wait until rank 0 process has prepared things;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      char char_files[ sizeoflist + 1 ];

      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      qslt = QStringList::split( "\n", QString( "%1" ).arg( char_files ) );
   } else {
      cout << QString("%1: extracting job files\n").arg( myrank ) << flush;
      
      // gunzip controlfile, must be renamed for us_gzip

      // rename
      QString dest = controlfile;
      dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
      QDir qd;
      qd.remove( dest );
      if ( !qd.rename( controlfile, dest ) )
      {
         cout << QString("Error renaming %1 to %2\n").arg( controlfile ).arg( dest );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      controlfile = dest;

      US_Gzip usg;
      int result = usg.gunzip( controlfile );
      if ( GZIP_OK != result )
      {
         cout << QString("Error: %1 problem gunzipping (%2)\n").arg( controlfile ).arg( usg.explain( result ) );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      controlfile.replace( QRegExp( "\\.gz$" ), "" );

      // tar open controlfile
      US_Tar ust;
      result = ust.extract( controlfile, &qslt );
      if ( TAR_OK != result )
      {
         cout << QString("Error: %1 problem extracting tar archive (%2)\n").arg( controlfile ).arg( ust.explain( result ) );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      // check to make sure all files .tgz

      if ( qslt.grep( QRegExp( "\\.(tgz|TGZ)$" ) ).size() != qslt.size() )
      {
         // not all tgz
         cout << QString("Error: %1 control file does not contain only .tgz files\n").arg( controlfile );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      cout << QString("%1: finished extracting job files\n" ).arg( myrank ) << flush;

      QString qs_files = qslt.join( "\n" ).ascii();
      sizeoflist = qs_files.length();
      char char_files[ sizeoflist + 1 ];
      strncpy( char_files, qs_files, sizeoflist + 1 );

      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
         
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

   }

   // we should all have the same qslt now

   cout << QString( "%1: %2\n" ).arg( myrank ).arg( qslt.join( ":" ) ) << flush;

   for ( unsigned int i = myrank; i < qslt.size(); i += npes )
   {
      cout << QString( "%1: I have job %2\n" ).arg( myrank ).arg( qslt[ i ] ) << flush;
   }      


   MPI_Finalize();
   exit( 0 );
   return true;
}
