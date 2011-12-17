#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "mpi.h"

// Analysis jobs with Supervisor and 2 or more Master+Workers parallel groups
void US_MPI_Analysis::pmasters_start()
{
   // Determine group (0,...), worker count in group, rank within group
   my_group       = my_rank / gcores_count;
   my_workers     = ( my_group == 0 ) ? gcores_count - 2 : gcores_count - 1;
   group_rank     = my_rank - my_group * gcores_count;

   if ( my_rank == 0 )
   {  // Supervisor is its own group
      my_group       = MPI_UNDEFINED;
      group_rank     = 0;
   }

   else if ( my_group == 0 )
   {  // Group 0 Master(1) and workers(2,...) get group rank 0,   1,...
      group_rank--;
   }

DbgLv(0) << "my_group" << my_group << "my_workers" << my_workers << "group_rank" << group_rank
 << "my_rank" << my_rank;
DbgLv(0) << "HOST=" << QHostInfo::localHostName();

DbgLv(0) << "MPI_Barrier" << my_rank;
   // Synch up everyone then split into communicator groups
   MPI_Barrier( MPI_COMM_WORLD );
DbgLv(0) << "Synced";
DbgLv(0) << "START COMM_SPLIT (g r m)" << my_group << group_rank << my_rank;
int sstat = MPI_Comm_split( MPI_COMM_WORLD, my_group, group_rank, &my_communicator );
DbgLv(0) << "COMM_SPLIT stat" << sstat << my_rank;

   if ( my_rank == 0 )
   {  // Run parallel-masters supervisor             (world rank 0)
      pmasters_supervisor();
   }

   else if ( group_rank == 0 )
   {  // Run parallel-masters master within a group  (group rank 0)
      pmasters_master();
   }

   else
   {  // Run parallel-masters worker within a group  (group rank 1,...)
      pmasters_worker();
   }

DbgLv(0) << "Final-my_rank" << my_rank << " secs=" << startTime.secsTo(QDateTime::currentDateTime());
   MPI_Finalize();
   exit( 0 );
}

// Parse Job XML, mainly for max wall-time and master-groups-count
void US_MPI_Analysis::job_parse( const QString& xmlfile )
{
   QFile file ( xmlfile );
   job_params[ "walltime"     ] = "2880";
   job_params[ "mgroupscount" ] = "1";

   if ( ! file.open( QIODevice::ReadOnly | QIODevice::Text) )
   {  // If no job xml or us3.pbs, return now
      if ( my_rank == 0 ) DbgLv(0) << "Cannot open file " << xmlfile;

      return;
   }

   if ( xmlfile == "us3.pbs" )
   {  // If no jobxmlfile.xml, but us3.pbs (-local), get values from it
      QTextStream ts( &file );

      while ( ! ts.atEnd() )
      {  // Read us3.pbs lines
         QString line = ts.readLine();
         int     jwt  = line.indexOf( "walltime=" );
         int     jgr  = line.indexOf( "pmgroups=" );

         if ( jwt > 0 )
         {  // Parse PBS line with walltime=
            QString svalu = line.mid( jwt ).section( "=", 1, 1 );
            int     valu1 = svalu.section( ":", 0, 0 ).toInt();
            int     valu2 = svalu.section( ":", 1, 1 ).toInt();
            svalu         = QString::number( valu1 * 60 + valu2 );
            job_params[ "walltime"     ] = svalu;
//if (my_rank==0) DbgLv(0) << "h m" << valu1 << valu2 << "svalu" << svalu;
         }

         if ( jgr > 0 )
         {  // Parse comment line with pmgroups=
            QString svalu = line.mid( jgr ).section( "=", 1, 1 );
            int     valu1 = svalu.toInt();
                    valu1 = valu1 < 0 ? 1 : valu1;
            svalu         = QString::number( valu1 );
            job_params[ "mgroupscount" ] = svalu;
//if (my_rank==0) DbgLv(0) << "group" << svalu << "valu1" << valu1;
         }
      }

      file.close();
      return;
   }

   QXmlStreamReader xml( &file );

   while ( ! xml.atEnd() )
   {  // Parse the *jobxmlfile.xml file  (mainly for walltime,mgroupscount)
      xml.readNext();

      if ( xml.isStartElement() )
      {
         QString name = xml.name().toString();

         if ( name != "Message"  &&  name != "Header" )
         {
            QString value = xml.readElementText();

            job_params[ name ] = value;
         }
      }

      if ( xml.isEndElement()  &&  xml.name() == "Header" )
         break;
   }

   file.close();
//if ( my_rank == 0 ) DbgLv(0) << "walltime=" << job_params["walltime"]
//   << "mgroupscount=" << job_params["mgroupscount"];
}

// Parallel-masters supervisor
void US_MPI_Analysis::pmasters_supervisor()
{
#if 0
   MPI_GA_MSG msg;
   MPI_Status status;
   int master   = 1;

/*
   while ( mgroupcount > 0 )
   {
      recv

      //determine if we want another
      if ( more )
         send continue
      else 
         send stop

      tar up files
      finish
   }
*/
   // Send start to all masters
   int tag    = 1;
   for ( int ii = 0; ii < mgroup_count; ii++ )
   {
      master = ( ii == 0 ) ? 1 : ( ii * gcores_count );
DbgLv(0) << "SUPER   Send to ii,master" << ii << master;
      MPI_Send( &msg,
                0,
                MPI_BYTE,
                master,
                tag,
                MPI_COMM_WORLD );
   }

   // Wait for end from all masters
   tag = 9;
   for ( int ii = 0; ii < mgroup_count; ii++ )
   {
      master = ( ii == 0 ) ? 1 : ( ii * gcores_count );
DbgLv(0) << "SUPER     Recv from master index" << ii;
      MPI_Recv( &msg,
                0,
                MPI_BYTE,
                MPI_ANY_SOURCE,
                tag,
                MPI_COMM_WORLD,
                &status );

      master = status.MPI_SOURCE;
DbgLv(0) << "SUPER      Recv was from master" << master;
   }
#endif
#if 1
   MPI_Status status;
   // Initialize masters states to READY
   QVector< int > mstates( mgroup_count, READY );
   QVector< int > mditers( mgroup_count, 0 );
   mc_iteration    = 0;
   int  master     = 1;
   int  iter_done  = 0;

   // Start off all masters doing iteration 1
   int tag    = 1001;
   for ( int ii = 0; ii < mgroup_count; ii++ )
   {
      master = ( ii == 0 ) ? 1 : ( ii * gcores_count );
DbgLv(0) << "SUPER   Send to ii,master" << ii << master;
      MPI_Send( &iter_done,
                0,
                MPI_BYTE,
                master,
                tag,
                MPI_COMM_WORLD );
      mstates[ ii ] = WORKING;
   }

   // Loop waiting on masters results
   while ( mc_iteration < mc_iterations )
   {
      tag        = 1009;
      MPI_Recv( &iter_done,
                0,
                MPI_BYTE,
                MPI_ANY_SOURCE,
                tag,
                MPI_COMM_WORLD,
                &status );

      master     = status.MPI_SOURCE;      // Master that has completed
      int kgroup = master / gcores_count;  // Group to which master belongs

      int kk     = mditers.count( 0 );

      if ( kk == mgroup_count  ||  kk == 0 )
      {  // 1st iteration 1 or any other iteration:  write model, bump iter
         pm_write_model();

         mc_iteration++;
         mditers[ kgroup ] = mditers[ kgroup ] + 1;
      }

      int jgroup = mstates.indexOf( READY );
      mstates[ kgroup ] = READY;
      jgroup     = ( jgroup < 0 ) ? kgroup : jgroup;

      if ( mc_iteration >= ( mc_iterations - mstates.count( WORKING ) ) )
         continue;    // Let master go idle if working on last few iterations

      // Alert a master to do an iteration
      tag    = 1002;
      master = ( jgroup == 0 ) ? 1 : ( jgroup * gcores_count );
      MPI_Send( &iter_done,
                0,
                MPI_BYTE,
                master,
                tag,
                MPI_COMM_WORLD );
      mstates[ jgroup ] = WORKING;

DbgLv(0) << "SUPER      Recv was from master" << master;
   }
#endif
}

// Parallel-masters master within a group
void US_MPI_Analysis::pmasters_master()
{
   MPI_GA_MSG msg;
   MPI_Status status;

   // Wait for start message from supervisor
DbgLv(0) << "  MASTER   Recv from super. my_rank" << my_rank << "group" << my_group;
   int super  = 0;
   int tag    = 1001;
   MPI_Recv( &msg,
             0,
             MPI_BYTE,
             super,
             tag,
             MPI_COMM_WORLD,
             &status );
DbgLv(0) << "  MASTER    Recv'd from super. (g r m)" << my_group << group_rank << my_rank;


   // Send start to each worker
   tag = 2;
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
DbgLv(0) << "  MASTER      Send to worker" << worker << " (g r m)" << my_group << group_rank << my_rank;
      MPI_Send( &msg,
                0,
                MPI_BYTE,
                worker,
                tag,
                my_communicator );
   }

   // Receive end from each worker
   tag = 8;
   for ( int ii = 0; ii < my_workers; ii++ )
   {
//DbgLv(0) << "  MASTER       ii" << ii << " (g r m)" << my_group << group_rank << my_rank;
      MPI_Recv( &msg,
                sizeof( msg ),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                tag,
                my_communicator,
                &status );
      int worker = status.MPI_SOURCE;
DbgLv(0) << "  MASTER        Recv fr worker" << worker << " (g r m)" << my_group << group_rank << my_rank;
   }

   // Send master-done to supervisor
   tag = 1009;

   MPI_Send( &msg,
             0,
             MPI_BYTE,
             super,
             tag,
             MPI_COMM_WORLD );
}

// Parallel-masters worker within a group
void US_MPI_Analysis::pmasters_worker()
{
#if 0
   MPI_GA_MSG msg;

   // Receive start from master
DbgLv(0) << "    WORKER  Recv start from master (g r m)" << my_group << group_rank << my_rank;
   int tag   = 2;
   MPI_Recv( &msg,
             sizeof( msg ),
             MPI_BYTE,
             MPI_ANY_SOURCE,
             tag,
             my_communicator,
             MPI_STATUS_IGNORE );
DbgLv(0) << "    WORKER  Recv'd start from master (g r m)" << my_group << group_rank << my_rank;

   // Do the work
   for ( int ii = 0; ii < 10; ii++ )
   {
DbgLv(0) << "    WORKER   doing unit" << ii << "(g r m)" << my_group << group_rank << my_rank;
      US_Sleep::msleep( 9000 );
   }

   // Tell master that we are done
   tag = 8;
DbgLv(0) << "    WORKER    Send DONE (g r m)" << my_group << group_rank << my_rank;
   MPI_Send( &msg,
             0,
             MPI_BYTE,
             0,
             tag,
             my_communicator );
#endif
#if 1
   if ( analysis_type == "2DSA" )
   {  // Standard 2DSA worker
      _2dsa_worker();
   }

   else if ( analysis_type == "GA" )
   {  // Standard GA worker
      ga_worker();
   }

   else
   {  // What???  Should not get here
   }
#endif

}

// Test time for MC iterations left; compare to walltime
void US_MPI_Analysis::time_mc_iterations()
{
   if ( mc_iteration < 4 )
      return;                      // Don't bother until MC iteration 4

   QDateTime currTime  = QDateTime::currentDateTime();
   int mins_so_far     = ( startTime.secsTo( currTime ) + 59 ) / 60;
   int mins_left_allow = max_walltime - mins_so_far;
   int mc_iters_left   = ( mins_left_allow * mc_iteration ) / mins_so_far;
   int mc_iters_estim  = mc_iteration + mc_iters_left;

   if ( mc_iters_estim < mc_iterations  &&  mc_iters_left < 4 )
   {  // In danger of exceeding allowed time:   reduce MC iterations
      int old_mciters     = mc_iterations;
      mc_iterations       = mc_iters_estim;

      QString msg = tr( "MC Iterations reduced from %1 to %2, "
                        "due to max. time restrictions." )
         .arg( old_mciters ).arg( mc_iterations );
      send_udp( msg );

      DbgLv(0) << "  Specified Maximum Wall-time minutes:" << max_walltime;
      DbgLv(0) << "  Number of minutes used so far:      " << mins_so_far;
      DbgLv(0) << "  Allowed minutes remaining:          " << mins_left_allow;
      DbgLv(0) << "  MC iterations run so far:           " << mc_iteration;
      DbgLv(0) << "  Estimated allowed iterations left:  " << mc_iters_left;
      DbgLv(0) << "MC Iterations reduced from" << old_mciters << "to"
         << mc_iterations << ", due to max. time restrictions.";
   }

   return;
}

// Parallel-masters version of 2DSA master
void US_MPI_Analysis::pm_2dsa_master()
{
}

// Parallel-masters version of GA master
void US_MPI_Analysis::pm_ga_master()
{
}

// Parallel-master write model
void US_MPI_Analysis::pm_write_model()
{
}

