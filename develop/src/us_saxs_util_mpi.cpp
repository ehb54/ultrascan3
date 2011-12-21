#include "../include/us_saxs_util.h"

#include <mpi.h>
extern int npes;
extern int myrank;

#define SLASH QDir::separator()

bool US_Saxs_Util::run_iq_mpi( QString controlfile )
{
   int errorno = -1;
   QString original_controlfile = controlfile;

   errormsg = "";
   if ( !controlfile.contains( QRegExp( "\\.(tgz|TGZ|tar|TAR)$" ) ) )
   {
      errormsg = QString( "controlfile must be .tgz or .tar, was %1" ).arg( controlfile );
      cout << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   if ( !QFile::exists( controlfile ) )
   {
      errormsg = QString( "controlfile %1 does not exist" ).arg( controlfile );
      cout << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   QStringList qslt;
   unsigned int sizeoflist;

   // this first barrier seems useless, but there seems to be an issue
   // where if one process takes awhile to get started, then the later barrier
   // gets stuck
   // this was noted on mpich2-1.4.1p1 &
   // openmpi2-1.4.3

   cout << QString("%1: initial universal barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   if ( myrank )
   {
      // wait until rank 0 process has prepared things;
      cout << QString("%1: waiting for process 0 to finish\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      cout << QString("%1: waiting for broadcast of size of list\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      char char_files[ sizeoflist + 1 ];

      cout << QString("%1: waiting for broadcast of list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      qslt = QStringList::split( "\n", QString( "%1" ).arg( char_files ) );
   } else {
      cout << QString("%1: extracting job files\n").arg( myrank ) << flush;
      
      QString dest = controlfile;
      int result;
      if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
      {
         // gunzip controlfile, must be renamed for us_gzip
         
         // rename
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
         result = usg.gunzip( controlfile );
         if ( GZIP_OK != result )
         {
            cout << QString("Error: %1 problem gunzipping (%2)\n").arg( controlfile ).arg( usg.explain( result ) );
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         errorno--;

         controlfile.replace( QRegExp( "\\.gz$" ), "" );
      }

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

      cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      cout << QString("%1: broadcasting size\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
         
      cout << QString("%1: broadcast list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
   }

   // we should all have the same qslt now

   cout << QString( "%1: %2\n" ).arg( myrank ).arg( qslt.join( ":" ) ) << flush;

   QString qs_base_dir = QDir::currentDirPath();

   US_Tar ust;

   QStringList full_output_list;

   for ( unsigned int i = myrank; i < qslt.size(); i += npes )
   {
      cout << QString( "%1: processing job %2\n" ).arg( myrank ).arg( qslt[ i ] ) << flush;
      // mkdir 
      QString qs_run_dir = QString( "%1/tmp_%2" ).arg( qs_base_dir ).arg( i );
      QDir qd( qs_run_dir );
      if ( !qd.exists() )
      {
         qd.mkdir( qs_run_dir );
      }
      QDir::setCurrent( qs_run_dir );
      
      if ( !read_control( QString( "../%1" ).arg( qslt[ i ] ) ) )
      {
         cout << QString( "%1: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // collect result files
      for ( unsigned int j = 0; j < job_output_files.size(); j++ )
      {
         full_output_list << QString( "tmp_%1/" ).arg( i ) + job_output_files[ j ];
      }
   } 

   cout << QString("%1: end of computation barrier\n" ).arg( myrank ) << flush;
   if ( (unsigned int) myrank >= qslt.size() )
   {
      full_output_list << "null_remove";
   }

   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   // now collect results

   QString qs_files = full_output_list.join( "\n" ).ascii();
   sizeoflist = qs_files.length();

   unsigned int max_individual_size;

   cout << QString("%1: All reduce to find max size\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Allreduce( &sizeoflist, &max_individual_size, 1, MPI_UNSIGNED, MPI_MAX, MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;
   cout << QString("%1: My size %2 max size %3\n" ).arg( myrank ).arg( sizeoflist ).arg( max_individual_size ) << flush;

   char char_output_files[ max_individual_size + 1 ];
   strncpy( char_output_files, qs_files, sizeoflist + 1 );

   char gathered_output_files[ ( max_individual_size + 1 ) * npes ];

   cout << QString("%1: Gather results\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Gather( char_output_files, max_individual_size + 1, MPI_CHAR, 
                                   gathered_output_files, max_individual_size + 1, MPI_CHAR,
                                   0, MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;
   cout << QString("%1: Gather complete\n" ).arg( myrank ) << flush;
   
   // now root should have all the data
   if ( !myrank )
   {
      QStringList qsl_final_files;
      
      for ( unsigned int i = 0; i < (unsigned int) npes; i++ )
      {
         qslt = QStringList::split( "\n", QString( "%1" )
                                    .arg( &gathered_output_files[ i * ( max_individual_size + 1 ) ] ) );
         for ( unsigned int j = 0; j < qslt.size(); j++ )
         {
            if ( qslt[ j ] != "null_remove" )
            {
               qsl_final_files << qslt[ j ];
            }
         }
      }

      cout << QString( "%1: Final output files %2\n" ).arg( myrank ).arg( qsl_final_files.join( ":" ) ) << flush;

      // create results.tar

      QString results_file = original_controlfile;
      results_file.replace( QRegExp( "\\.(tgz|TGZ|tar|TGZ)$" ), "" );
      results_file += "_out.tar";

      QStringList list;
      
      QDir::setCurrent( qs_base_dir );

      US_Tar ust;
      
      int result = ust.create( results_file, qsl_final_files, &list );

      if ( result != TAR_OK )
      {
         cout << QString("Error: Problem creating tar archive %1 %2\n").arg( results_file ).arg( ust.explain( result ) ) << flush;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      
      cout << QString( "%1: results file %2\n" ).arg( myrank ).arg( results_file );
   }

   MPI_Finalize();
   exit( 0 );
   return true;
}

bool US_Saxs_Util::run_nsa_mpi( QString controlfile )
{
   // for now, everyone reads the control file & sets things up to the point of nsa run
   int errorno = -1;
   nsa_mpi = true;

   errormsg = "";
   if ( !controlfile.contains( QRegExp( "\\.(tgz|TGZ|tar|TAR)$" ) ) )
   {
      errormsg = QString( "controlfile must be .tgz or .tar, was %1" ).arg( controlfile );
      cerr << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   if ( !QFile::exists( controlfile ) )
   {
      errormsg = QString( "controlfile %1 does not exist" ).arg( controlfile );
      cerr << errormsg << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   QStringList qslt;
   unsigned int sizeoflist;

   // this first barrier seems useless, but there seems to be an issue
   // where if one process takes awhile to get started, then the later barrier
   // gets stuck
   // this was noted on mpich2-1.4.1p1 &
   // openmpi2-1.4.3

   // cout << QString("%1: initial universal barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   if ( myrank )
   {
      // wait until rank 0 process has extracted the file:
      // cout << QString("%1: waiting for process 0 to finish\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // cout << QString("%1: waiting for broadcast of size of list\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      char char_files[ sizeoflist + 1 ];

      // cout << QString("%1: waiting for broadcast of list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      qslt = QStringList::split( "\n", QString( "%1" ).arg( char_files ) );
   } else {
      // cout << QString("%1: extracting job files\n").arg( myrank ) << flush;
      
      QString dest = controlfile;
      int result;
      if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
      {
         // gunzip controlfile, must be renamed for us_gzip
         
         // rename
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
         result = usg.gunzip( controlfile );
         if ( GZIP_OK != result )
         {
            cout << QString("Error: %1 problem gunzipping (%2)\n").arg( controlfile ).arg( usg.explain( result ) );
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         errorno--;

         controlfile.replace( QRegExp( "\\.gz$" ), "" );
      }

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

      // cout << QString("%1: finished extracting job files\n" ).arg( myrank ) << flush;

      QString qs_files = qslt[ 0 ];
      sizeoflist = qs_files.length();
      char char_files[ sizeoflist + 1 ];
      strncpy( char_files, qs_files, sizeoflist + 1 );

      // cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      // cout << QString("%1: broadcasting size\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &sizeoflist, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
         
      // cout << QString("%1: broadcast list data\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( char_files, sizeoflist + 1, MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;
   }

   // we should all have the same qslt now

   // cout << QString( "%1: %2\n" ).arg( myrank ).arg( qslt.join( ":" ) ) << flush;

   // everyone reads for now
   controlfile = qslt[ 0 ];

   if ( !read_control( controlfile ) )
   {
      cout <<  cout << QString( "%1: Error: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   MPI_Finalize();
   exit( 0 );
   return true;
}
