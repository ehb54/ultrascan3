#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_tar.h"
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
   {  // Group 0 Master(1) and workers(2,...): set group rank 0,   1,...
      group_rank--;
   }

DbgLv(1) << "my_group" << my_group << "my_workers" << my_workers
 << "group_rank" << group_rank << "my_rank" << my_rank
 << "HOST=" << QHostInfo::localHostName();

DbgLv(1) << "MPI_Barrier" << my_rank;
   // Synch up everyone then split into communicator groups
   MPI_Barrier( MPI_COMM_WORLD );
   int sstat = MPI_Comm_split( MPI_COMM_WORLD, my_group, group_rank,
                               &my_communicator );
DbgLv(1) << "COMM_SPLIT (g r m)" << my_group << group_rank << my_rank
 << "stat" << sstat;

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

DbgLv(1) << "Final-my_rank" << my_rank << " msecs=" << startTime.msecsTo(QDateTime::currentDateTime());
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
   // Initialize masters states to READY
   QVector< int > mstates( mgroup_count, READY );
   QByteArray     msg;
   MPI_Status     status;
   long int       maxrssma = 0L;

   mc_iteration = 1;
   int  master  = 1;
   int  iwork   = 0;
   int  tag;
   max_rss();

   // Start off all masters doing iteration 1.
   //   Send each group's master the analysis date for use with the model
   //   description. Then, send the iteration index as a trigger to begin
   //   an iteration loop; all groups do iteration 1.

   for ( int ii = 0; ii < mgroup_count; ii++ )
   {
      master = ( ii == 0 ) ? 1 : ( ii * gcores_count );   // Master world rank

DbgLv(1) << "SUPER: master msgs" << master << "analysisDate" << analysisDate;
      msg    = analysisDate.toAscii();
      iwork  = msg.size();
      MPI_Send( &iwork,
                1,
                MPI_INT,
                master,
                ADATESIZE,
                MPI_COMM_WORLD );

      MPI_Send( msg.data(),
                iwork,
                MPI_BYTE,
                master,
                ADATE,
                MPI_COMM_WORLD );

      MPI_Send( &mc_iteration,
                1,
                MPI_INT,
                master,
                STARTITER,
                MPI_COMM_WORLD );

      mstates[ ii ] = WORKING;                 // Mark all masters busy
   }

   // Loop waiting on masters results.
   // Each time through this masters loop, wait for an integer sent
   //   from a master.
   // Three types (tags) may be received:
   //   (1) UDP message size (message itself will follow);
   //   (2) Iteration-done;
   //   (3) Last-iteration-done (integer received is max group memory used).

   while ( true )
   {
      max_rss();

      // Get UDP message or iteration-done flag
DbgLv(1) << "SUPER: wait on iter done/udp";
      MPI_Recv( &iwork,
                1,
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );
      bool udpmsg = false;
      bool islast = false;
      int  isize  = 0;
      master      = status.MPI_SOURCE;
      tag         = status.MPI_TAG;
      QByteArray msg;
DbgLv(1) << "SUPER:   wait recv'd iwork" << iwork << "master tag" << master << tag;

      switch( tag )
      {
         case UDPSIZE:  // UDP message size: get the following UDP message string
            isize = iwork;
            msg.resize( isize );

DbgLv(1) << "SUPER: wait on UDPmsg - master" << master;
            MPI_Recv( msg.data(),
                      isize,
                      MPI_BYTE,
                      master,
                      UDPMSG,
                      MPI_COMM_WORLD,
                      &status );

DbgLv(1) << "SUPER:     UDPmsg:" << QString(msg);
            send_udp( QString( msg ) );       // Send forwarded UDP message

            udpmsg   = true;
            break;

         case DONELAST:  // Iteration done and it was the last in group
            islast   = true;

         case DONEITER:  // Iteration done
            break;

         default:
            DbgLv(0) << "Unknown message type in supervisor" << tag;
            break;
      }

      if ( udpmsg )  continue;                // Loop for next master message

      // If here, the message was a signal that an MC iteration is complete.
      // Most commonly, the group master that just completed an iteration
      //   is sent the next iteration to begin.
      // As the total iterations nears the limit, the message tag used may be
      //   one that signals that the master should treat the iteration as the
      //   last one for the group; the group next alerted may not be the same
      //   as the one that just finished (since it may have shut down).
      // The data received with a done-iteration message is the max memory
      //   used in the master group.

      master     = status.MPI_SOURCE;        // Master that has completed
      int kgroup = master / gcores_count;    // Group to which master belongs
      int mgroup = mstates.indexOf( READY ); // Find next ready master

      int jgroup = ( mgroup < 0 ) ? kgroup : mgroup; // Ready master index
      mstates[ kgroup ] = READY;                     // Mark current as ready
      int nileft = mstates.count( WORKING );         // Iterations left working
DbgLv(1) << "SUPER: mgr kgr jgr" << mgroup << kgroup << jgroup
 << "left iter iters" << nileft << mc_iteration << mc_iterations;

      if ( islast )    
      {  // Just finished was last iteration for that group
         mstates[ kgroup ] = INIT;           // Mark as finished
         maxrssma += (long)( iwork );        // Sum in master maxrss
DbgLv(1) << "SUPER:  (A)maxrssma" << maxrssma << "iwork" << iwork;
      }

      if ( mc_iteration >= mc_iterations )
      {  // All iterations have been started
         if ( nileft == 0 ) break;           // All are complete
         continue;                           // Some iters are still working
      }

      else if ( islast  &&  mgroup < 0 )
      {  // Last iteration of group and no group ready, so loop
         continue;                          // Some iters still needed
      }

      // Alert the next available master to do an iteration. Use a
      //   different tag when it is to be the last iteration for a group.
      // This enables the master and its workers to do normal shutdown.
      int iter_wk = ++mc_iteration;        // Next iteration to do
      tag         = STARTITER;             // Flag as normal iteration

      if ( mc_iteration > ( mc_iterations - mgroup_count ) )
         tag         = STARTLAST;          // Flag as last iter for group

      master      = ( jgroup == 0 ) ? 1 : ( jgroup * gcores_count );

DbgLv(1) << "SUPER:  Send next iter" << iter_wk << "gr ma tg" << jgroup << master << tag;
      MPI_Send( &iter_wk,
                1,
                MPI_INT,
                master,
                tag,
                MPI_COMM_WORLD );

      mstates[ jgroup ] = WORKING;         // Mark group as busy
      max_rss();                           // Memory use of supervisor
   }

   // In the parallel-masters case, the supervisor handles end-of-job
   //  messages and file creation.

   // Get job end time (after waiting, so it has the greatest time stamp)
   US_Sleep::msleep( 900 );
   QDateTime endTime = QDateTime::currentDateTime();

   // Send message and build file with run-time statistics
   max_rss();                              // Max memory for supervisor
DbgLv(1) << "SUPER:  maxrss maxrssma" << maxrss << maxrssma;
   maxrss       += maxrssma;               // Sum in master groups' use

   int walltime  = qRound(
      submitTime.msecsTo( endTime ) / 1000.0 );
   int cputime   = qRound(
      startTime .msecsTo( endTime ) / 1000.0 );
   int maxrssmb  = qRound( (double)maxrss / 1024.0 );
   int kc_iters  = parameters[ "mc_iterations" ].toInt();

   if ( mc_iterations < kc_iters )
   {
      send_udp( "Finished:  maxrss " + QString::number( maxrssmb )
            + " MB,  total run seconds " + QString::number( cputime )
            + "  (Reduced MC Iterations)" );
      DbgLv(0) << "Finished:  maxrss " << maxrssmb
               << "MB,  total run seconds " << cputime
               << "  (Reduced MC Iterations)";
   }

   else
   {
      send_udp( "Finished:  maxrss " + QString::number( maxrssmb )
            + " MB,  total run seconds " + QString::number( cputime ) );
      DbgLv(0) << "Finished:  maxrss " << maxrssmb
               << "MB,  total run seconds " << cputime;
   }

   stats_output( walltime, cputime, maxrssmb,
         submitTime, startTime, endTime );

   // Build list and archive of output files
   QDir        d( "." );
   QStringList files = d.entryList( QStringList( "*" ), QDir::Files );

   US_Tar tar;
   tar.create( "analysis-results.tar", files );

   // Remove the files we just put into the tar archive
   QString file;
   foreach( file, files ) d.remove( file );
}

// Parallel-masters master within a group
void US_MPI_Analysis::pmasters_master()
{
   MPI_Status status;
   QByteArray msg;

   // Get analysis date for model descriptions from supervisor
DbgLv(1) << "  MASTER   Recv from super. my_rank" << my_rank
 << "group" << my_group;
   int super  = 0;
   int isize  = 0;

   MPI_Recv( &isize,
             1,
             MPI_INT,
             super,
             ADATESIZE,
             MPI_COMM_WORLD,
             &status );
DbgLv(1) << "  MASTER:   wait recv'd isize" << isize;

   msg.resize( isize );

   MPI_Recv( msg.data(),
             isize,
             MPI_BYTE,
             super,
             ADATE,
             MPI_COMM_WORLD,
             &status );

   analysisDate = QString( msg );
DbgLv(1) << "  MASTER:   Recv'd from super. (g r m)" << my_group << group_rank
 << my_rank << "analysisDate" << analysisDate << "atype" << analysis_type;

   // Do the master loop for MC 2DSA or GA
   if ( analysis_type == "2DSA" )
   {
      pm_2dsa_master();
   }

   else if ( analysis_type == "GA" )
   {
      pm_ga_master();
   }
}

// Parallel-masters worker within a group
void US_MPI_Analysis::pmasters_worker()
{
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
      DbgLv(0) << "INVALID ANALYSIS TYPE" << analysis_type;
   }
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

// Parallel-masters version of group 2DSA master
void US_MPI_Analysis::pm_2dsa_master( void )
{
DbgLv(1) << "master start 2DSA" << startTime;
   init_solutes();
   fill_queue();

   work_rss.resize( gcores_count );

   current_dataset     = 0;
   datasets_to_process = 1;  // Process one dataset at a time for now

   int iter     = 1;
   int super    = 0;
   MPI_Status status;

   // Get 1st iteration (1) from supervisor
   MPI_Recv( &iter,
             1,
             MPI_INT,
             super,
             MPI_ANY_TAG,
             MPI_COMM_WORLD,
             &status );

   int tag      = status.MPI_TAG;
   mc_iteration = iter;

   while ( true )
   {
      int worker;
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker    = worker_status.indexOf( READY, worknext );

         if ( worker < 1 )
         {
            worknext  = 1;
            worker    = worker_status.indexOf( READY, worknext );
         }

         worknext  = worker + 1;
         worknext  = ( worknext > my_workers ) ? 1 : worknext;

         _2dsa_Job job          = job_queue.takeFirst();
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) ) 
      {
         QString progress = 
            "Iteration: "    + QString::number( iterations ) +
            "; Dataset: "    + QString::number( current_dataset + 1 ) +
            "; Meniscus: (Run 1 of 1)" +
            "; MonteCarlo: " + QString::number( mc_iteration );

         send_udp( progress );

         // Manage multiple data sets
         if ( data_sets.size() > 1  &&  datasets_to_process == 1 )
         {
            global_fit();
         }

         if ( ! job_queue.isEmpty() ) continue;

         // Write out the model, but skip if not 1st iteration-1
         max_rss();
         bool do_write = ( mc_iteration > 1 );
DbgLv(1) << "2dMast:    (1)do_write" << do_write << "mc_iter" << mc_iteration
   << "variance" << simulation_values.variance;

         if ( mc_iteration == 1 )
         {                                       // Iteration 1
            QString fn = data_sets[ 0 ]->run_data.runID + "*.xml";
            QDir d( "." );
            QStringList files = d.entryList( QStringList( fn ), QDir::Files );
            do_write   = ( files.size() == 0 );  // Only write if none exists
DbgLv(1) << "2dMast:      (2)do_write" << do_write << (do_write?"":files[0]);
         }

         qSort( simulation_values.solutes );

         if ( do_write )
            write_model( simulation_values, US_Model::TWODSA );

         if ( mc_iteration >= mc_iterations )
         {
            for ( int jj = 1; jj <= my_workers; jj++ )
               maxrss += work_rss[ jj ];
         }

         // Tell the supervisor that an iteration is done
         iter    = (int)maxrss;
         tag     = ( mc_iteration < mc_iterations ) ?
                   DONEITER : DONELAST;

         MPI_Send( &iter,
                   1,
                   MPI_INT,
                   super,
                   tag,
                   MPI_COMM_WORLD );

         if ( mc_iteration < mc_iterations )
         {
            time_mc_iterations();

            if ( mc_iteration < mc_iterations )
            {
               set_monteCarlo();

               // Get new Monte Carlo iteration index from supervisor
               MPI_Recv( &iter,
                         1,
                         MPI_INT,
                         super,
                         MPI_ANY_TAG,
                         MPI_COMM_WORLD,
                         &status );

               tag      = status.MPI_TAG;

               if ( tag == STARTLAST )
                  mc_iterations = iter;

               else if ( tag != STARTITER )
               {
                  DbgLv(0) << "Unexpected tag in PMG 2DSA Master" << tag;
                  continue;
               }

               mc_iteration  = iter;
            }
         }

         if ( ! job_queue.isEmpty() ) continue;

         shutdown_all();  // All done
         break;           // Break out of main loop.
      }

      // Wait for worker to send a message
      int        size[ 4 ];

      MPI_Recv( &size, 
                4, 
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status);

      worker = status.MPI_SOURCE;

//if ( max_depth > 0 )
// DbgLv(1) << " PMG master loop-BOTTOM:   status TAG" << status.MPI_TAG
//  << MPI_Job::READY << MPI_Job::RESULTS << "  source" << status.MPI_SOURCE;
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results( worker, size );
            work_rss[ worker ] = size[ 3 ];
            break;

         default:  // Should never happen
            QString msg =  "Master 2DSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }

      max_rss();
   }
}

// Parallel-masters version of GA group master
void US_MPI_Analysis::pm_ga_master( void )
{
   current_dataset = 0;
//DbgLv(1) << "master start GA" << startTime;

   // Set noise and debug flags
   simulation_values.noisflag   = 0;
   simulation_values.dbg_level  = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Initialize best fitness
   best_genes  .reserve( gcores_count );
   best_fitness.reserve( gcores_count );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   Gene working_gene( buckets.count(), US_Solute() );

   // Initialize arrays
   for ( int i = 0; i < gcores_count; i++ )
   {
      best_genes << working_gene;

      empty_fitness.index = i;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle global fit if needed
   if ( data_sets.size() > 1 )
   {
      for ( int i = 0; i < data_sets.size(); i++ )
      {
         ga_master_loop();
         qSort( best_fitness );
         simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];

         for ( int g = 0; g < buckets.size(); g++ )
            simulation_values.solutes[ g ].s *= 1.0e-13;

         calc_residuals( current_dataset, 1, simulation_values );

         ga_global_fit();  // Normalize data and update workers
      }
   }

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while ( true )
   {
      // Get MC iteration index from supervisor
      int iter       = 1;
      int super      = 0;
      MPI_Status status;

      MPI_Recv( &iter,
                1,
                MPI_INT,
                super,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );

      int tag        = status.MPI_TAG;
DbgLv(1) << "  MASTER: iter" << iter << "gr" << my_group << "tag" << tag;
      switch ( tag )
      {
         case STARTLAST:
            mc_iterations = iter;

         case STARTITER:
            mc_iteration  = iter;
            break;

         default:
            DbgLv(0) << "Unknown message to PMG Master" << tag;
            break;
      }

      ga_master_loop();

      qSort( best_fitness );
      simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];

     for ( int g = 0; g < buckets.size(); g++ )
         simulation_values.solutes[ g ].s *= 1.0e-13;

DbgLv(1) << "GaMast: sols size" << simulation_values.solutes.size()
 << "buck size" << buckets.size();
DbgLv(1) << "GaMast:   dset size" << data_sets.size();
DbgLv(1) << "GaMast:   sol0.s" << simulation_values.solutes[0].s;
      calc_residuals( 0, data_sets.size(), simulation_values );
DbgLv(1) << "GaMast:    calc_resids return";

      // Write out the model, but skip if not 1st iteration-1
      bool do_write = ( mc_iteration > 1 );
DbgLv(1) << "GaMast:    (1)do_write" << do_write << "mc_iter" << mc_iteration
 << "variance" << simulation_values.variance;

      if ( mc_iteration == 1 )
      {                                       // Iteration 1
         QString fn = data_sets[ 0 ]->run_data.runID + "*.xml";
         QDir d( "." );
         QStringList files = d.entryList( QStringList( fn ), QDir::Files );
         do_write   = ( files.size() == 0 );  // Only write if none exists
DbgLv(1) << "GaMast:      (2)do_write" << do_write << (do_write?"":files[0]);
      }

      qSort( simulation_values.solutes );

      if ( do_write )
         write_model( simulation_values, US_Model::GA );

      // Tell supervisor that an iteration is done
      iter    = (int)maxrss;
      tag     = ( mc_iteration < mc_iterations ) ?
                DONEITER : DONELAST;

      MPI_Send( &iter,
                1,
                MPI_INT,
                super,
                tag,
                MPI_COMM_WORLD );

DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      if ( mc_iteration < mc_iterations )
      {
         // Set scaled_data the first time
         if ( mc_iteration == 1 ) 
         {
            scaled_data = simulation_values.sim_data;
         }

         time_mc_iterations();

         if ( mc_iteration < mc_iterations )
         {
            set_gaMonteCarlo();
         }

      }
      else
         break;
   }


   MPI_Job job;

   // Send finish to workers ( in the tag )
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &job,              // MPI #0
                sizeof( job ),
                MPI_BYTE,
                worker,
                FINISHED,
                my_communicator );
   }            
}

