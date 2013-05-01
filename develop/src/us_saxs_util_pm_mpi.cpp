#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
#include "../include/us_pm.h"
#include "../include/us_timer.h"
#include <mpi.h>
#include <sys/time.h>
extern int npes;
extern int myrank;
extern QString outputData;

// note: this program uses cout and/or cerr and this should be replaced

static US_Timer usupm_timer;

bool US_Saxs_Util::run_pm_mpi( QString controlfile )
{
   return false;
   // for now, everyone reads the control file & sets things up to the point of nsa run
   if ( !myrank )
   {
      usupm_timer.init_timer ( "pm" );
      usupm_timer.init_timer ( "pm init" );
      usupm_timer.init_timer ( "pm closeout" );
      usupm_timer.start_timer ( "pm" );
      usupm_timer.start_timer ( "pm init" );
   }

   QString qs_base_dir = QDir::currentDirPath();

   outputData = QString( "%1" ).arg( getenv( "outputData" ) );
   if ( outputData.isEmpty() )
   {
      outputData = "../outputData";
   }
   if ( !myrank )
   {
      cout << QString( "Notice: outputData is \"%1\"\n" ).arg( outputData );
   }

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

   QString original_controlfile;

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

      // copy here
      US_File_Util usu;
      usu.copy( controlfile, QDir::currentDirPath() + QDir::separator() + QFileInfo( controlfile ).fileName() );
      cout << QString( "copying %1 %2 <%3>\n" )
         .arg( controlfile )
         .arg( QDir::currentDirPath() + QDir::separator() + QFileInfo( controlfile ).fileName() )
         .arg( usu.errormsg );
      dest = QFileInfo( controlfile ).fileName();

      cout << QString( "dest is now %1\n" )
         .arg( dest );
      original_controlfile = dest;

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

   if ( !myrank )
   {
      usupm_timer.end_timer ( "pm init" );
   }
   if ( !read_control( controlfile ) )
   {
      cout <<  cout << QString( "%1: Error: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   // cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   
   //    cout << 
   //       QString( "%1: my output files:<%2>\n" )
   //       .arg( myrank )
   //       .arg( output_files.join( ":" ) ) 
   //         << flush;

   // join up outputfiles:

   if ( !myrank )
   {
      usupm_timer.start_timer ( "pm closeout" );

      QStringList org_output_files = output_files;
      output_files.clear();
      QString errors;

      for ( int i = 0; i < ( int ) org_output_files.size(); i++ )
      {
         QString qs   = org_output_files[ i ];
         qs.replace( "_rank0_" , "_rank%1_" );
         QString name = org_output_files[ i ];
         name.replace( QRegExp( "^.*_rank0_" ) , "" );
         cout << QString( "new name <%1>\n" ).arg( name );

         vector < double > q;
         vector < double > I;

         // open up & average:

         int number_of_files_read = 0;

         for ( int r = 0; r < npes; r++ )
         {
            QFile f( QString( qs ).arg( r ) );

            if ( !f.exists() )
            {
               cout << QString( "%1: expected output file %2 does not exist\n" )
                  .arg( myrank )
                  .arg( f.name() )
                       << flush;
               MPI_Abort( MPI_COMM_WORLD, -5001 );
               exit( -5001 );
            }
            if ( !set_control_parameters_from_experiment_file( f.name(), true ) )
            {
               errors +=  QString( "%1: Error trying to get grid, ignorming this processor%2\n" ).arg( myrank ).arg( errormsg );
               continue;
            }
            number_of_files_read++;
            if ( !r )
            {
               q = sgp_exp_q;
               I = sgp_exp_I;
            } else {
               if ( I.size() != sgp_exp_I.size() )
               {
                  cout << QString( "%1: collect results size mismatch %1 vs %2 \n" )
                     .arg( myrank )
                     .arg( I.size() )
                     .arg( sgp_exp_I.size() )
                       << flush;
                  MPI_Abort( MPI_COMM_WORLD, -5003 );
                  exit( -5003 );
               }
               for ( unsigned int i = 0; i < ( unsigned int )sgp_exp_I.size(); i++ )
               {
                  I[ i ] += sgp_exp_I[ i ];
               }
            }
         }
         for ( unsigned int i = 0; i < ( unsigned int )sgp_exp_I.size(); i++ )
         {
            I[ i ] /= (double) number_of_files_read;
         }
         if ( !write_output( name, q, I ) )
         {
            cout << QString( "%1: %2\n" )
               .arg( myrank )
               .arg( errormsg )
                 << flush;

            MPI_Abort( MPI_COMM_WORLD, -5004 );
            exit( -5004 );
         }
      }
      cout << 
         QString( "%1: my output files:<%2>\ncontrolfile %3\n" )
         .arg( myrank )
         .arg( output_files.join( ":" ) ) 
         .arg( controlfile )
           << flush;

      if ( !errors.isEmpty() )
      {
         QFile f( "errors" );
         if ( !f.open( IO_WriteOnly ) )
         {
            cout << "Error: errors exist but can not open errors file\n" << flush;
            MPI_Abort( MPI_COMM_WORLD, -5002 );
            exit( -5002 );
         }

         QTextStream ts( &f );
         ts << errors;
         f.close();
         output_files << "errors";
      }

      usupm_timer.end_timer ( "pm closeout" );
      usupm_timer.end_timer ( "pm" );

      QFile f( "runinfo" );
      if ( f.open( IO_WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << QString( "number of processors %1\n" ).arg( npes );
         ts << "timings:\n";
         ts << usupm_timer.list_times();;
         ts << "end-timings\n";
         QFile fc( controlfile );
         if ( fc.open( IO_ReadOnly ) )
         {
            QTextStream tsc( &fc );
            ts << "controlfile:\n";
            while( !tsc.atEnd() )
            {
               ts << tsc.readLine() << endl;
            }
            ts << "end-controlfile\n";
            fc.close();
         }
         f.close();
         output_files << "runinfo";
      } else {
         cout << "Warning: could not create timings\n" << flush;
      }

      // package output
      if ( !create_tgz_output( controlfile + "_out.tgz" ) )
      {
         cout << QString( "%1: %2\n" )
            .arg( myrank )
            .arg( errormsg )
              << flush;
         MPI_Abort( MPI_COMM_WORLD, -5005 );
         exit( -5005 );
      }
   }

   if ( !myrank )
   {
      QString results_file = original_controlfile;
      results_file.replace( QRegExp( "\\.(tgz|TGZ|tar|TGZ)$" ), "" );
      results_file += "_out.tgz";

      QDir dod( outputData );
      if ( !dod.exists() )
      {
         QDir current = QDir::current();
            
         QString newdir = outputData;
         while ( newdir.left( 3 ) == "../" )
         {
            current.cdUp();
            newdir.replace( "../", "" );
         }
         QDir::setCurrent( current.path() );
         QDir ndod;
         if ( !ndod.mkdir( newdir, true ) )
         {
            cout << QString("Warning: could not create outputData \"%1\" directory\n" ).arg( ndod.path() );
         }
         QDir::setCurrent( qs_base_dir );
      }
      if ( dod.exists() )
      {
         QString dest = outputData + QDir::separator() + QFileInfo( results_file ).fileName();
         QDir qd;
         cout << QString("renaming: %1 to %2\n" )
               .arg( results_file )
               .arg( dest );
         if ( !qd.rename( results_file, dest ) )
         {
            cout << QString("Warning: could not rename outputData %1 to %2\n" )
               .arg( results_file )
               .arg( dest );
         }
      } else {
         cout << QString( "Error: %1 does not exist\n" ).arg( outputData );
      }
   }

   MPI_Finalize();

   exit( 0 );
   return true;
}

