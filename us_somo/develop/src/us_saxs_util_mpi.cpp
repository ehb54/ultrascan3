#include <mpi.h>

#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
//Added by qt3to4:
#include <QTextStream>

extern int npes;
extern int myrank;

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#if defined( CUDA )
#  include "us_cuda.h"
   extern int env_mpi_node;
#endif

#define SHUTDOWN_MINUTES 60

QString outputData;

#define SLASH QDir::separator()

#include <qdatetime.h>

bool       use_warning_time;
QDateTime  start_time;
// make warning time 15 minutes (?) before end time
QDateTime  warning_time;
bool       timed_out;

#if defined( USE_MPI )
   extern void debug_mpi( QString );

#  include <qdatetime.h>
   void debug_mpi( QString msg )
   {
      QFile f( QString( "mpidebug-%1" ).arg( myrank ) );
      if( f.open( QIODevice::WriteOnly | QIODevice::Append ) )
      {
         QTextStream ts( &f );
         ts << QTime::currentTime().toString( "hh:mm:ss.zzz" ) << ":" << msg;
         f.close();
      }
      if ( msg.contains( QRegExp( "^\\d+: (SEND|RECV) " ) ) )
      {
         QFile f( QString( "debug-%1" ).arg( myrank ) );
         QStringList msgs = (msg ).split( "\n" , Qt::SkipEmptyParts );
         if( f.open( QIODevice::WriteOnly | QIODevice::Append ) )
         {
            QTextStream ts( &f );
            ts << QTime::currentTime().toString( "hh:mm:ss.zzz" ) << ":" << msgs[ 0 ] << Qt::endl;
            f.close();
         }
      }
   }
#endif

bool US_Saxs_Util::run_iq_mpi( QString controlfile )
{

#if defined( CUDA )
   env_mpi_node = QString( "%1" ).arg( getenv( "MPISPAWN_ID" ) ).toInt();
   // cout << QString( "%1:MPISPAWN_ID %2\n" ).arg( myrank ).arg( env_mpi_node ) << flush;
   cuda_ipcrm();
#endif

   outputData = QString( "%1" ).arg( getenv( "outputData" ) );
   if ( outputData.isEmpty() )
   {
      outputData = "outputData";
   }
   if ( !myrank )
   {
      cout << QString( "Notice: outputData is \"%1\"\n" ).arg( outputData );
   }

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

   unsigned int skip_cores_per_core = 0;

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

      cout << QString("%1: waiting for broadcast of skip_cores_per_core\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &skip_cores_per_core, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
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

      // qslt = (QString( "%1" ).split( "\n" , Qt::SkipEmptyParts ).arg( char_files ) );
      {
         QString qs = QString( "%1" ).arg( char_files );
         qslt = (qs ).split( "\n" , Qt::SkipEmptyParts );
      }
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

      if ( qslt.filter( QRegExp( "\\.(tgz|TGZ)$" ) ).size() != qslt.size() )
      {
         // not all tgz
         cout << QString("Error: %1 control file does not contain only .tgz files\n").arg( controlfile );
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      cout << QString("%1: finished extracting job files\n" ).arg( myrank ) << flush;

      // check for ^common_

      QStringList common = qslt.filter( QRegExp( "^common_" ) );
      if ( common.size() )
      {
         for ( unsigned int i = 0; i < common.size(); i++ )
         {
            cout << QString("%1: found common file %2\n" ).arg( myrank ).arg( common[ i ] ) << flush;
         
            QString controlfile = common[ i ];

            if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
            {
               // gunzip controlfile, must be renamed for us_gzip

               dest = controlfile;

               // rename
               dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
               cout << QString("Renaming %1 to %2\n").arg( controlfile ).arg( dest );
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

            // mkdir, extract
            QString qs_base_dir = QDir::currentPath();
            QString qs_run_dir = QString( "%1/common" ).arg( qs_base_dir );
            QDir qd( qs_run_dir );
            if ( !qd.exists() )
            {
               qd.mkdir( qs_run_dir );
            }
            QDir::setCurrent( qs_run_dir );
            // tar open controlfile
            QStringList qslt;
            US_Tar ust;
            result = ust.extract( qs_base_dir + QDir::separator() + controlfile, &qslt );
            if ( TAR_OK != result )
            {
               cout << QString("Error: %1 problem extracting tar archive (%2)\n").arg( controlfile ).arg( ust.explain( result ) );
               MPI_Abort( MPI_COMM_WORLD, errorno );
               exit( errorno );
            }
            errorno--;

            // check for directives
            {
               QFile f( "__directives" );
               if ( f.exists() )
               {
                  if ( !f.open( QIODevice::ReadOnly ) )
                  {
                     // us_qdebug("Error: could not open directives file" );
                     MPI_Abort( MPI_COMM_WORLD, errorno );
                     exit( errorno );
                  }
                  QTextStream ts( &f );
                  QString qs = ts.readLine();
                  f.close();
                  skip_cores_per_core = qs.toUInt();
                  // us_qdebug( QString( "%1: found directives, skip cores per core is %2" ).arg( myrank ).arg( skip_cores_per_core ) );
               }
            }
            
            QDir::setCurrent( qs_base_dir );
         }

         // remove ^common_ from qslt
         QStringList newqslt;
         for ( unsigned int i = 0; i < qslt.size(); i++ )
         {
            if ( !qslt[ i ].contains( QRegExp( "^common_" ) ) )
            {
               newqslt << qslt[ i ];
            }
         }

         qslt = newqslt;

         cout << QString("%1: done with common files\n" ).arg( myrank ) << flush;
      }

      QString qs_files = qslt.join( "\n" ).toLatin1().data();
      sizeoflist = qs_files.length();
      char char_files[ sizeoflist + 1 ];
      strncpy( char_files, qs_files.toLatin1().data(), sizeoflist + 1 );

      cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
      {
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }         
      errorno--;

      cout << QString("%1: broadcasting skip_cores_per_core\n" ).arg( myrank ) << flush;
      if ( MPI_SUCCESS != MPI_Bcast( &skip_cores_per_core, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD ) )
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

   QString qs_base_dir = QDir::currentPath();

   US_Tar ust;

   QStringList full_output_list;
   map < QString, bool > in_output;

   unsigned int inc       = skip_cores_per_core + 1;
   unsigned int use_procs = npes / inc;
   if ( npes % inc )
   {
      use_procs++;
   }

   unsigned int pn = 0;

   if ( !myrank )
   {
      // us_qdebug( QString( "0: jobs %1 np %2 skip %3 useprocs %4 inc %5" )
      //         .arg( qslt.size() )
      //         .arg( npes )
      //         .arg( skip_cores_per_core )
      //         .arg( use_procs )
      //         .arg( inc ) );
   }

   if ( myrank % inc )
   {
      // us_qdebug( QString( "%1: sleep" ).arg( myrank ) );
      if ( (unsigned int) myrank >= qslt.size() )
      {
         full_output_list << "null_remove";
      }
   } else {
      for ( unsigned int i = myrank / inc; i < qslt.size(); i += use_procs )
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
      
         noticemsg = "";
         errormsg = "";

         if ( !read_control( QString( "../%1" ).arg( qslt[ i ] ) ) )
         {
            cout << QString( "%1: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
            // MPI_Abort( MPI_COMM_WORLD, errorno );
            // exit( errorno );
         }

         if ( !errormsg.isEmpty() )
         {
            QFile f( QString( "errors-%1" ).arg( i ) );
            if( f.open( QIODevice::WriteOnly | QIODevice::Append ) )
            {
               QTextStream ts( &f );
               ts << QString( "%1: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
               f.close();
            }
            if ( !in_output.count( f.fileName() ) )
            {
               in_output[ f.fileName() ] = true;
               full_output_list << QString( "tmp_%1/%2" ).arg( i ).arg( f.fileName() );
            }
         }

         if ( !noticemsg.isEmpty() )
         {
            QFile f( QString( "notice-%1" ).arg( i ) );
            if( f.open( QIODevice::WriteOnly | QIODevice::Append ) )
            {
               QTextStream ts( &f );
               ts << QString( "%1: %2\n" ).arg( myrank ).arg( noticemsg ) << flush;
               f.close();
            }
            if ( !in_output.count( f.fileName() ) )
            {
               in_output[ f.fileName() ] = true;
               full_output_list << QString( "tmp_%1/%2" ).arg( i ).arg( f.fileName() );
            }
         }
               
         errorno--;

         // collect result files
         for ( unsigned int j = 0; j < job_output_files.size(); j++ )
         {
            full_output_list << QString( "tmp_%1/" ).arg( i ) + job_output_files[ j ];
         }
      }
   }

   //   if ( (unsigned int) myrank >= qslt.size() )
   if ( !full_output_list.size() )
   {
      full_output_list << "null_remove";
   }

   nice( 20 );

   // us_qdebug( QString("%1: end of computation barrier my full_output_list %2\n" ).arg( myrank ).arg( full_output_list.join( ":" ) ) );

   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

   // now collect results

   QString qs_files = full_output_list.join( "\n" ).toLatin1().data();
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
   strncpy( char_output_files, qs_files.toLatin1().data(), sizeoflist + 1 );

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
         // qslt = (QString( "%1" ).split( "\n" , Qt::SkipEmptyParts )
         //                            .arg( &gathered_output_files[ i * ( max_individual_size + 1 ) ] ) );
         {
            QString qs =  QString( "%1" ).arg( &gathered_output_files[ i * ( max_individual_size + 1 ) ] );
            qslt = (qs ).split( "\n" , Qt::SkipEmptyParts );
         }
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

      // rename to outputDir
      
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
         if ( !ndod.mkdir( newdir ) )
         {
            cout << QString("Warning: could not create outputData \"%1\" directory\n" ).arg( ndod.path() );
         }
         QDir::setCurrent( qs_base_dir );
      }
      if ( dod.exists() )
      {
         QString dest = outputData + QDir::separator() + QFileInfo( results_file ).fileName();
         QDir qd;
         if ( !qd.rename( results_file, dest ) )
         {
            cout << QString("Warning: could not rename outputData %1 to %2\n" )
               .arg( results_file )
               .arg( dest );
         }
      }
   }

   MPI_Finalize();
   exit( 0 );
   return true;
}

bool US_Saxs_Util::run_nsa_mpi( QString controlfile )
{
   // for now, everyone reads the control file & sets things up to the point of nsa run

   if ( npes < 2 )
   {
      cerr << "ERROR: requires at least 2 processes to run nsa" << endl << flush;
      MPI_Abort( MPI_COMM_WORLD, -10002 );
      exit( -10002 );
   }         

   QString qs_base_dir = QDir::currentPath();

   int minutes = QString( "%1" ).arg( getenv( "GFAC_JOBWALLTIME" ) ).toInt();
   if ( minutes - SHUTDOWN_MINUTES > 0 )
   {
      use_warning_time = true;
      timed_out = false;
      start_time = QDateTime::currentDateTime();
      warning_time = start_time;
      warning_time = warning_time.addSecs( ( minutes - SHUTDOWN_MINUTES ) * 60 );
      if ( !myrank )
      {
         cout << QString( "%1: Start time: %2\nWarning time: %3\n" )
            .arg( myrank )
            .arg( start_time.toString() )
            .arg( warning_time.toString() )
            ;
      }
   } else {
      use_warning_time = false;
      timed_out        = false;
      if ( !myrank )
      {
         cout << QString( "no GFAC_JOBWALLTIME or < %1 minutes run avail\n" ).arg( SHUTDOWN_MINUTES );
      }
   }

   outputData = QString( "%1" ).arg( getenv( "outputData" ) );
   if ( outputData.isEmpty() )
   {
      outputData = "outputData";
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

      // qslt = (QString( "%1" ).split( "\n" , Qt::SkipEmptyParts ).arg( char_files ) );
      {
         QString qs = QString( "%1" ).arg( char_files );
         qslt = (qs ).split( "\n" , Qt::SkipEmptyParts );
      }
   } else {
      // cout << QString("%1: extracting job files\n").arg( myrank ) << flush;
      
      QString dest = controlfile;
      int result;

      // copy here
      US_File_Util usu;
      usu.copy( controlfile, QDir::currentPath() + SLASH + QFileInfo( controlfile ).fileName() );
      cout << QString( "copying %1 %2 <%3>\n" )
         .arg( controlfile )
         .arg( QDir::currentPath() + SLASH + QFileInfo( controlfile ).fileName() )
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
      strncpy( char_files, qs_files.toLatin1().data(), sizeoflist + 1 );

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
      cout << QString( "%1: Error: %2\n" ).arg( myrank ).arg( errormsg ) << flush;
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   errorno--;

#if defined( CUDA )
   cuda_ipcrm();
#endif

   // cout << QString("%1: signaling end of barrier\n" ).arg( myrank ) << flush;
   if ( MPI_SUCCESS != MPI_Barrier( MPI_COMM_WORLD ) )
   {
      MPI_Abort( MPI_COMM_WORLD, errorno );
      exit( errorno );
   }         
   
   if ( !myrank )
   {
      if ( timed_out )
      {
         cout << "closeup knows we timed out\n";
      }
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
         if ( !ndod.mkdir( newdir ) )
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
