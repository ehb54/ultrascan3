#include "mpi.h"
#include "us_math2.h"
#include "us_mpi_analysis.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "us_tar.h"

// Analysis jobs with Supervisor and 2 or more Master+Workers parallel groups
void US_MPI_Analysis::pm_cjobs_start() {
   // Determine group (0,...), worker count in group, rank within group
   my_group = my_rank / gcores_count;
   my_workers = (my_group == 0) ? gcores_count - 2 : gcores_count - 1;
   group_rank = my_rank - my_group * gcores_count;

   if (my_rank == 0) { // Supervisor is its own group
      my_group = MPI_UNDEFINED;
      group_rank = 0;
   }

   else if (my_group == 0) { // Group 0 Master(1) and workers(2,...): set group rank 0,   1,...
      group_rank--;
   }

   DbgLv(1) << "my_group" << my_group << "my_workers" << my_workers << "group_rank" << group_rank << "my_rank"
            << my_rank << "HOST=" << QHostInfo::localHostName();

   DbgLv(1) << "MPI_Barrier" << my_rank;
   // Synch up everyone then split into communicator groups
   MPI_Barrier(MPI_COMM_WORLD);
   int sstat = MPI_Comm_split(MPI_COMM_WORLD, my_group, group_rank, &my_communicator);
   DbgLv(1) << "COMM_SPLIT (g r m)" << my_group << group_rank << my_rank << "stat" << sstat;

   if (my_rank == 0) { // Run parallel-masters supervisor             (world rank 0)
      pm_cjobs_supervisor();
   }

   else if (group_rank == 0) { // Run parallel-masters master within a group  (group rank 0)
      pm_cjobs_master();
   }

   else { // Run parallel-masters worker within a group  (group rank 1,...)
      pm_cjobs_worker();
   }

   DbgLv(1) << "Final-my_rank" << my_rank << " msecs=" << startTime.msecsTo(QDateTime::currentDateTime());
   MPI_Finalize();
   exit(0);
}

// Parallel-masters supervisor
void US_MPI_Analysis::pm_cjobs_supervisor() {
   // Initialize masters states to READY
   QVector<int> mstates(mgroup_count, READY);
   QByteArray msg;
   MPI_Status status;
   long int maxrssma = 0L;

   current_dataset = 0;
   int master = 1;
   int iwork = 0;
   int tag;
   max_rss();

   // Start off all masters doing each one's first dataset.
   //   Send each group's master the analysis date for use with the model
   //   description. Then, send the dataset index as a trigger to begin
   //   a dataset loop.

   for (int ii = 0; ii < mgroup_count; ii++) {
      master = (ii == 0) ? 1 : (ii * gcores_count); // Master world rank

      DbgLv(1) << "SUPER: master msgs" << master << "analysisDate" << analysisDate;
      msg = analysisDate.toLatin1();
      iwork = msg.size();
      MPI_Send(&iwork, 1, MPI_INT, master, ADATESIZE, MPI_COMM_WORLD);

      MPI_Send(msg.data(), iwork, MPI_BYTE, master, ADATE, MPI_COMM_WORLD);

      MPI_Send(&current_dataset, 1, MPI_INT, master, STARTITER, MPI_COMM_WORLD);

      mstates[ ii ] = WORKING; // Mark all masters busy
      current_dataset++;
   }

   // Loop waiting on masters results.
   // Each time through this masters loop, wait for an integer sent
   //   from a master.
   // Three types (tags) may be received:
   //   (1) UDP message size (message itself will follow);
   //   (2) Iteration-done;
   //   (3) Last-iteration-done (integer received is max group memory used).

   while (true) {
      max_rss();

      // Get UDP message or dataset-done flag
      DbgLv(1) << "SUPER: wait on iter done/udp";
      MPI_Recv(&iwork, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      bool udpmsg = false;
      bool islast = false;
      int isize = 0;
      int ittest = current_dataset + mgroup_count;
      master = status.MPI_SOURCE;
      tag = status.MPI_TAG;
      QByteArray msg;
      DbgLv(1) << "SUPER:   wait recv'd iwork" << iwork << "master tag" << master << tag;

      switch (tag) {
         case UDPSIZE: // UDP message size: get the following UDP message string
            isize = iwork;
            msg.resize(isize);

            DbgLv(1) << "SUPER: wait on UDPmsg - master" << master;
            MPI_Recv(msg.data(), isize, MPI_BYTE, master, UDPMSG, MPI_COMM_WORLD, &status);

            DbgLv(1) << "SUPER:     UDPmsg:" << QString(msg);
            send_udp(QString(msg)); // Send forwarded UDP message

            udpmsg = true;
            break;

         case DONELAST: // Dataset done and it was the last in group
            islast = true;
            DbgLv(1) << "SUPER:     DONELAST";
            break;

         case DONEITER: // Dataset done
            //            islast   = ( ittest >= count_datasets ) ? true : islast;
            DbgLv(1) << "SUPER:     DONEITER  islast" << islast << "ittest currds cntds" << ittest << current_dataset
                     << count_datasets;
            break;

         default:
            DbgLv(0) << "Unknown message type in supervisor" << tag;
            break;
      }

      if (udpmsg)
         continue; // Loop for next master message

      // If here, the message was a signal that a group iteration is complete.
      // Most commonly, the group master that just completed an iteration
      //   is sent the next iteration to begin.
      // As the total iterations nears the limit, the message tag used may be
      //   one that signals that the master should treat the iteration as the
      //   last one for the group; the group next alerted may not be the same
      //   as the one that just finished (since it may have shut down).
      // The data received with a done-iteration message is the max memory
      //   used in the master group.

      master = status.MPI_SOURCE; // Master that has completed
      int kgroup = master / gcores_count; // Group to which master belongs
      int mgroup = mstates.indexOf(READY); // Find next ready master

      int jgroup = (mgroup < 0) ? kgroup : mgroup; // Ready master index
      mstates[ kgroup ] = READY; // Mark current as ready
      int nileft = mstates.count(WORKING); // Masters left working
      DbgLv(1) << "SUPER: mgr kgr jgr" << mgroup << kgroup << jgroup << "left dsets dset" << nileft << count_datasets
               << current_dataset;
      int kdset = (current_dataset / mgroup_count) * mgroup_count;

      if (islast) { // Just finished was last dataset for that group
         mstates[ kgroup ] = INIT; // Mark as finished
         maxrssma += ( long ) (iwork); // Sum in master maxrss
         DbgLv(1) << "SUPER:  (A)maxrssma" << maxrssma << "iwork" << iwork;

         if (nileft == 0) { // All are complete
            count_datasets = kdset + mgroup_count;
            DbgLv(1) << "SUPER: nileft==0  count_datasets" << count_datasets;
            break;
         }

         if (mgroup < 0)
            continue; // Some iters are still working
      }


      // Alert the next available master to do an iteration. Use a
      //   different tag when it is to be the last iteration for a group.
      // This enables the master and its workers to do normal shutdown.
      ittest = current_dataset + mgroup_count;
      tag = (ittest < count_datasets) ? STARTITER : STARTLAST;
      master = (jgroup == 0) ? 1 : (jgroup * gcores_count);

      DbgLv(1) << "SUPER:  Send curr_ds cnt_ds" << current_dataset << count_datasets << "gr ma tg" << jgroup << master
               << tag;
      MPI_Send(&current_dataset, 1, MPI_INT, master, tag, MPI_COMM_WORLD);

      mstates[ jgroup ] = WORKING; // Mark group as busy
      max_rss(); // Memory use of supervisor

      current_dataset++;
   }

   // In the parallel-masters case, the supervisor handles end-of-job
   //  messages and file creation.

   // Get job end time (after waiting, so it has the greatest time stamp)
   US_Sleep::msleep(900);
   QDateTime endTime = QDateTime::currentDateTime();

   // Send message and build file with run-time statistics
   max_rss(); // Max memory for supervisor
   DbgLv(1) << "SUPER:  maxrss maxrssma" << maxrss << maxrssma;
   maxrss += maxrssma; // Sum in master groups' use

   int walltime = qRound(submitTime.msecsTo(endTime) / 1000.0);
   int cputime = qRound(startTime.msecsTo(endTime) / 1000.0);
   int maxrssmb = qRound(( double ) maxrss / 1024.0);
   int kc_iters = data_sets.size();

   stats_output(walltime, cputime, maxrssmb, submitTime, startTime, endTime);

   // Create output archive file and remove other output files
   update_outputs(true);

   // Send 'Finished' message.
   int wt_hr = walltime / 3600;
   int wt_min = (walltime - wt_hr * 3600) / 60;
   int wt_sec = walltime - wt_hr * 3600 - wt_min * 60;
   int ct_hr = cputime / 3600;
   int ct_min = (cputime - ct_hr * 3600) / 60;
   int ct_sec = cputime - ct_hr * 3600 - ct_min * 60;
   printf(
      "Us_Mpi_Analysis has finished successfully"
      " (Wall=%d:%02d:%02d Cpu=%d:%02d:%02d).\n",
      wt_hr, wt_min, wt_sec, ct_hr, ct_min, ct_sec);
   fflush(stdout);

   if (count_datasets < kc_iters) {
      send_udp(
         "Finished:  maxrss " + QString::number(maxrssmb) + " MB,  total run seconds " + QString::number(cputime)
         + "  (Reduced Datasets Count)");
      DbgLv(0) << "Finished:  maxrss " << maxrssmb << "MB,  total run seconds " << cputime
               << "  (Reduced Datasets Count)";
   }

   else {
      send_udp("Finished:  maxrss " + QString::number(maxrssmb) + " MB,  total run seconds " + QString::number(cputime));
      DbgLv(0) << "Finished:  maxrss " << maxrssmb << "MB,  total run seconds " << cputime;
   }
}

// Parallel-masters master within a group
void US_MPI_Analysis::pm_cjobs_master() {
   MPI_Status status;
   QByteArray msg;

   // Get analysis date for model descriptions from supervisor
   DbgLv(1) << "  MASTER   Recv from super. my_rank" << my_rank << "group" << my_group;
   int super = 0;
   int isize = 0;

   MPI_Recv(&isize, 1, MPI_INT, super, ADATESIZE, MPI_COMM_WORLD, &status);
   DbgLv(1) << "  MASTER:   wait recv'd isize" << isize;

   msg.resize(isize);

   MPI_Recv(msg.data(), isize, MPI_BYTE, super, ADATE, MPI_COMM_WORLD, &status);

   analysisDate = QString(msg);
   DbgLv(1) << "  MASTER:   Recv'd from super. (g r m)" << my_group << group_rank << my_rank << "analysisDate"
            << analysisDate << "atype" << analysis_type;

   // Do the master loop for MC 2DSA or GA
   if (analysis_type.startsWith("2DSA")) {
      pm_2dsa_cjmast();
   }

   else if (analysis_type.startsWith("GA")) {
      pm_ga_cjmast();
   }

   else if (analysis_type.startsWith("DMGA")) {
      pm_dmga_cjmast();
   }

   else if (analysis_type.startsWith("PCSA")) {
      pm_pcsa_cjmast();
   }
}

// Parallel-masters worker within a group
void US_MPI_Analysis::pm_cjobs_worker() {
   if (analysis_type.startsWith("2DSA")) { // Standard 2DSA worker
      _2dsa_worker();
   }

   else if (analysis_type.startsWith("GA")) { // Standard GA worker
      ga_worker();
   }

   else if (analysis_type.startsWith("DMGA")) {
      dmga_worker();
   }

   else if (analysis_type.startsWith("PCSA")) {
      pcsa_worker();
   }

   else { // What???  Should not get here
      DbgLv(0) << "INVALID ANALYSIS TYPE" << analysis_type;
   }
}

// Test time for datasets left; compare to walltime
void US_MPI_Analysis::time_datasets_left() {
   if (current_dataset < 4)
      return; // Don't bother until current_dataset 4

   QDateTime currTime = QDateTime::currentDateTime();
   int curr_iter = current_dataset + 1;
   int mins_so_far = (startTime.secsTo(currTime) + 59) / 60;
   int mins_left_allow = max_walltime - mins_so_far;
   int ds_iters_left = (mins_left_allow * curr_iter) / mins_so_far;
   ds_iters_left = (ds_iters_left / mgroup_count) * mgroup_count;
   int ds_iters_estim = curr_iter + ds_iters_left;

   if (ds_iters_estim < count_datasets && ds_iters_left < 4) { // In danger of exceeding allowed time:   reduce count of datasets
      int old_dsiters = count_datasets;
      count_datasets = qMax(curr_iter, ds_iters_estim - 2);
      int ac_iters_left = count_datasets - curr_iter;
      ac_iters_left = (ac_iters_left / mgroup_count) * mgroup_count;
      count_datasets = curr_iter + ac_iters_left;

      QString msg = tr("Dataset count reduced from %1 to %2, "
                       "due to max. time restrictions.")
                       .arg(old_dsiters)
                       .arg(count_datasets);
      send_udp(msg);

      DbgLv(0) << "  Specified Maximum Wall-time minutes:" << max_walltime;
      DbgLv(0) << "  Number of minutes used so far:      " << mins_so_far;
      DbgLv(0) << "  Allowed minutes remaining:          " << mins_left_allow;
      DbgLv(0) << "  Dataset processed so far:           " << curr_iter;
      DbgLv(0) << "  Estimated allowed datasets left:    " << ds_iters_left;
      DbgLv(0) << "  Actual adjusted datasets left:      " << ac_iters_left;
      DbgLv(0) << "Datasets reduced from" << old_dsiters << "to" << count_datasets
               << ", due to max. time restrictions.";

      // Just to be sure, create tar file right now
      if (my_group == 0)
         update_outputs();
   }

   return;
}

// Parallel-masters version of group 2DSA master
void US_MPI_Analysis::pm_2dsa_cjmast(void) {
   DbgLv(1) << "master start 2DSA" << startTime;
   init_solutes();
   fill_queue();

   work_rss.resize(gcores_count);

   current_dataset = 0;
   datasets_to_process = 1; // Process one dataset at a time for now

   int super = 0;
   int iter = current_dataset;
   MPI_Status status;

   // Get 1st dataset from supervisor
   MPI_Recv(&current_dataset, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

   int tag = status.MPI_TAG;
   int ittest = current_dataset + mgroup_count;

   if (meniscus_points > 1) { // Reset the range of fit-meniscus points for this data set
      US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
      double men_str = edata->meniscus - meniscus_range / 2.0;
      double men_inc = meniscus_range / (meniscus_points - 1.0);
      double dat_str = edata->radius(0);
      double men_end = men_str + meniscus_range - men_inc;
      if (men_end >= dat_str) { // Adjust first meniscus so range remains below data range
         men_end = dat_str - men_inc / 2.0;
         men_str = men_end - meniscus_range + men_inc;
      }
      for (int ii = 0; ii < meniscus_points; ii++)
         meniscus_values[ ii ] = men_str + men_inc * ii;
   }

   while (true) {
      int worker;
      meniscus_value = meniscus_values.size() == 1 ? data_sets[ current_dataset ]->run_data.meniscus
                                                   : meniscus_values[ meniscus_run ];
      //if ( max_depth > 1 )
      // DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
      //    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while (!job_queue.isEmpty() && worker_status.contains(READY)) {
         worker = ready_worker();

         Sa_Job job = job_queue.takeFirst();

         submit(job, worker);

         worker_depth[ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if (job_queue.isEmpty() && !worker_status.contains(WORKING)) {
         US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
         QString tripleID = edata->cell + edata->channel + edata->wavelength;
         int menisc_size = meniscus_values.size();
         QString progress = "Iteration: " + QString::number(iterations)
                            + "; Dataset: " + QString::number(current_dataset + 1) + " (" + tripleID + ") of "
                            + QString::number(count_datasets);

         if (mc_iterations > 1)
            progress += "; MonteCarlo: " + QString::number(mc_iteration + 1);

         else if (menisc_size > 1)
            progress += "; Meniscus: " + QString::number(meniscus_value, 'f', 3)
                        + tr(" (%1 of %2)").arg(meniscus_run + 1).arg(menisc_size);

         else
            progress += "; RMSD: " + QString::number(sqrt(simulation_values.variance));

         send_udp(progress);

         // Iterative refinement
         if (max_iterations > 1) {
            if (iterations == 1)
               qDebug() << "  == Refinement Iterations for Dataset" << current_dataset + 1 << "==";

            qDebug() << "Iterations:" << iterations << " Variance:" << simulation_values.variance
                     << "RMSD:" << sqrt(simulation_values.variance);

            iterate();
         }

         if (!job_queue.isEmpty())
            continue;

         // Write out the model and, possibly, noise(s)
         max_rss();

         write_output();

         // Fit meniscus
         if ((meniscus_run + 1) < meniscus_values.size()) {
            set_meniscus();
         }

         if (!job_queue.isEmpty())
            continue;

         // Monte Carlo
         if (mc_iterations > 1) { // Recompute final fit to get simulation and residual
            mc_iteration++;

            wksim_vals = simulation_values;
            wksim_vals.solutes = calculated_solutes[ max_depth ];

            calc_residuals(current_dataset, 1, wksim_vals);

            simulation_values = wksim_vals;

            if (mc_iteration < mc_iterations) {
               set_monteCarlo();
            }
         }

         if (!job_queue.isEmpty())
            continue;

         ittest = current_dataset + mgroup_count;

         if (ittest >= count_datasets) {
            for (int jj = 1; jj <= my_workers; jj++)
               maxrss += work_rss[ jj ];
         }

         // Tell the supervisor that an iteration is done
         iter = ( int ) maxrss;
         tag = (ittest < count_datasets) ? DONEITER : DONELAST;

         MPI_Send(&iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD);

         if (current_dataset < count_datasets) {
            if (my_group == 0 && ittest < count_datasets) { // If group 0 master, create an intermediate archive
               update_outputs();
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << " : Intermediate archive was created.";
            }

            US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
            QString tripleID = edata->cell + edata->channel + edata->wavelength;

            if (simulation_values.noisflag == 0) {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")"
                        << " : model was output.";
            }
            else {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")"
                        << " : model/noise(s) were output.";
            }

            time_datasets_left();

            if (ittest < count_datasets) {
               // Get new dataset index from supervisor
               MPI_Recv(&iter, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

               tag = status.MPI_TAG;
               DbgLv(1) << "CJ_MAST Recv tag" << tag << "iter" << iter;

               if (tag == STARTLAST)
                  count_datasets = iter + 1;

               else if (tag != STARTITER) {
                  DbgLv(0) << "Unexpected tag in PMG 2DSA Master" << tag;
                  continue;
               }

               current_dataset = iter;
               mc_iteration = 0;
               iterations = 1;
               meniscus_run = 0;

               if (meniscus_points > 1) { // Reset the range of fit-meniscus points for this data set
                  US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
                  double men_str = edata->meniscus - meniscus_range / 2.0;
                  double men_inc = meniscus_range / (meniscus_points - 1.0);
                  double dat_str = edata->radius(0);
                  double men_end = men_str + meniscus_range - men_inc;
                  if (men_end >= dat_str) { // Adjust first meniscus so range remains below data range
                     men_end = dat_str - men_inc / 2.0;
                     men_str = men_end - meniscus_range + men_inc;
                  }
                  for (int ii = 0; ii < meniscus_points; ii++)
                     meniscus_values[ ii ] = men_str + men_inc * ii;
               }

               for (int ii = 1; ii <= my_workers; ii++)
                  worker_status[ ii ] = READY;

               fill_queue();

               for (int ii = 0; ii < calculated_solutes.size(); ii++)
                  calculated_solutes[ ii ].clear();

               continue;
            }
         }

         if (!job_queue.isEmpty())
            continue;

         shutdown_all(); // All done
         break; // Break out of main loop.
      }

      // Wait for worker to send a message
      int sizes[ 4 ];

      MPI_Recv(sizes, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, my_communicator, &status);

      worker = status.MPI_SOURCE;

      //if ( max_depth > 0 )
      // DbgLv(1) << " PMG master loop-BOTTOM:   status TAG" << status.MPI_TAG
      //  << MPI_Job::READY << MPI_Job::RESULTS << "  source" << status.MPI_SOURCE;
      switch (status.MPI_TAG) {
         case MPI_Job::READY: // Ready for work
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results(worker, sizes);
            work_rss[ worker ] = sizes[ 3 ];
            break;

         default: // Should never happen
            QString msg = "Master 2DSA:  Received invalid status " + QString::number(status.MPI_TAG);
            abort(msg);
            break;
      }

      max_rss();
   }
}

// Parallel-masters version of GA group master
void US_MPI_Analysis::pm_ga_cjmast(void) {
   current_dataset = 0;
   datasets_to_process = data_sets.size();
   max_depth = 0;
   calculated_solutes.clear();
   //DbgLv(1) << "master start GA" << startTime;

   // Set noise and debug flags
   simulation_values.noisflag = 0;
   simulation_values.dbg_level = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
   DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Initialize best fitness
   best_genes.reserve(gcores_count);
   best_fitness.reserve(gcores_count);

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   Gene working_gene(buckets.count(), US_Solute());

   // Initialize arrays
   for (int i = 0; i < gcores_count; i++) {
      best_genes << working_gene;

      empty_fitness.index = i;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while (true) {
      // Get MC iteration index from supervisor
      int iter = 1;
      int super = 0;
      MPI_Status status;

      MPI_Recv(&iter, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int tag = status.MPI_TAG;
      DbgLv(1) << "  MASTER: iter" << iter << "gr" << my_group << "tag" << tag;
      switch (tag) {
         case STARTLAST:
            mc_iterations = iter;

         case STARTITER:
            mc_iteration = iter;
            break;

         default:
            DbgLv(0) << "Unknown message to PMG Master" << tag;
            break;
      }

      ga_master_loop();

      qSort(best_fitness);
      simulation_values.solutes = best_genes[ best_fitness[ 0 ].index ];

      int nisols = simulation_values.solutes.size();

      solutes_from_gene(simulation_values.solutes, nisols);

      DbgLv(1) << "GaMast: sols size" << simulation_values.solutes.size() << "buck size" << buckets.size();
      DbgLv(1) << "GaMast:   dset size" << data_sets.size();
      DbgLv(1) << "GaMast:   sol0.s" << simulation_values.solutes[ 0 ].s;
      calc_residuals(0, data_sets.size(), simulation_values);

      DbgLv(1) << "GaMast:    calc_resids return";

      // Write out the model, but skip if not 1st of iteration 1
      bool do_write = (mc_iteration > 1) || (mc_iteration == 1 && my_group == 0);
      DbgLv(1) << "2dMast:    do_write" << do_write << "mc_iter" << mc_iteration << "variance"
               << simulation_values.variance << "my_group" << my_group;

      qSort(simulation_values.solutes);

      // Convert given solute points to s,k for model output
      double vbar20 = data_sets[ 0 ]->vbar20;
      QList<int> attrxs;
      attrxs << attr_x << attr_y << attr_z;
      bool have_s = (attrxs.indexOf(ATTR_S) >= 0);
      bool have_k = (attrxs.indexOf(ATTR_K) >= 0);
      bool have_w = (attrxs.indexOf(ATTR_W) >= 0);
      bool have_d = (attrxs.indexOf(ATTR_D) >= 0);
      bool have_f = (attrxs.indexOf(ATTR_F) >= 0);
      bool vary_v = (attr_z != ATTR_V);

      for (int gg = 0; gg < simulation_values.solutes.size(); gg++) {
         US_Solute *solu = &simulation_values.solutes[ gg ];
         US_Model::SimulationComponent mcomp;
         mcomp.s = have_s ? solu->s : 0.0;
         mcomp.f_f0 = have_k ? solu->k : 0.0;
         mcomp.mw = have_w ? solu->d : 0.0;
         mcomp.vbar20 = vary_v ? solu->v : vbar20;
         mcomp.D = have_d ? solu->d : 0.0;
         mcomp.f = have_f ? solu->d : 0.0;

         US_Model::calc_coefficients(mcomp);

         solu->s = mcomp.s;
         solu->k = mcomp.f_f0;
         solu->v = mcomp.vbar20;
      }

      calculated_solutes.clear();
      calculated_solutes << simulation_values.solutes;

      if (do_write) {
         if (data_sets.size() == 1) {
            write_output();
         }
         else {
            write_global();
         }
      }

      if (my_group == 0) { // Update the tar file of outputs in case of an aborted run
         update_outputs();
      }

      if (current_dataset < count_datasets) { // Before last iteration:  check if max is reset based on time limit
         time_datasets_left(); // Test if near time limit

         tag = (current_dataset < count_datasets) ? DONEITER : DONELAST;
      }

      else { // Mark that max iterations reached
         tag = DONELAST;
      }

      // Tell the supervisor that an iteration is done
      iter = ( int ) maxrss;
      DbgLv(1) << "GaMast:  iter done: maxrss" << iter << "tag" << tag << DONELAST;

      MPI_Send(&iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD);

      DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      if (mc_iteration < mc_iterations) { // Set up for next iteration
         if (mc_iteration == 1) { // Set scaled_data the first time
            scaled_data = simulation_values.sim_data;
         }

         set_gaMonteCarlo();
      }

      else // Break out of the loop if all iterations have been done
         break;

   } // END:  MC iterations loop


   MPI_Job job;

   // Send finish to workers ( in the tag )
   for (int worker = 1; worker <= my_workers; worker++) {
      MPI_Send(
         &job, // MPI #0
         sizeof(job), MPI_BYTE, worker, FINISHED, my_communicator);
   }
}

// Parallel-masters version of DMGA group master
void US_MPI_Analysis::pm_dmga_cjmast(void) {
   current_dataset = 0;
   datasets_to_process = data_sets.size();
   max_depth = 0;
   calculated_solutes.clear();
   //DbgLv(1) << "master start DMGA" << startTime;

   // Set noise and debug flags
   simulation_values.noisflag = 0;
   simulation_values.dbg_level = dbg_level;
   simulation_values.dbg_timing = dbg_timing;
   DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Build up the base structure of simulations and residuals
   if (data_sets.size() == 1) {
      US_AstfemMath::initSimData(simulation_values.sim_data, data_sets[ 0 ]->run_data, 0.0);
      US_AstfemMath::initSimData(simulation_values.residuals, data_sets[ 0 ]->run_data, 0.0);
   }
   else {
      int ntscan = data_sets[ 0 ]->run_data.scanCount();
      for (int ii = 1; ii < data_sets.size(); ii++)
         ntscan += data_sets[ ii ]->run_data.scanCount();

      simulation_values.sim_data.scanData.resize(ntscan);
      simulation_values.residuals.scanData.resize(ntscan);
   }

   // Initialize best fitness
   best_dgenes.reserve(gcores_count);
   best_fitness.reserve(gcores_count);

   // Read in the constraints model and build constraints
   QString cmfname = "../" + parameters[ "DC_model" ];
   wmodel.load(cmfname); // Load the constraints model
   constraints.load_constraints(&wmodel); // Build the constraints object
   constraints.get_work_model(&wmodel); // Get the base work model

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;
   dgene = wmodel;
   nfloatc = constraints.float_constraints(&cns_flt);
   nfvari = (1 << nfloatc) - 1;
   dgmarker.resize(nfloatc);
   do_astfem
      = (wmodel.components[ 0 ].sigma == 0.0 && wmodel.components[ 0 ].delta == 0.0 && wmodel.coSedSolute < 0
         && data_sets[ 0 ]->compress == 0.0);

   // Initialize arrays
   for (int ii = 0; ii < gcores_count; ii++) {
      best_dgenes << dgene;

      empty_fitness.index = ii;
      best_fitness << empty_fitness;
   }

   QDateTime time = QDateTime::currentDateTime();

   // Handle Monte Carlo iterations.  There will always be at least 1.
   while (true) {
      // Get MC iteration index from supervisor
      int iter = 1;
      int super = 0;
      MPI_Status status;

      MPI_Recv(&iter, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      int tag = status.MPI_TAG;
      DbgLv(1) << "  MASTER: iter" << iter << "gr" << my_group << "tag" << tag;
      switch (tag) {
         case STARTLAST:
            mc_iterations = iter;

         case STARTITER:
            mc_iteration = iter;
            break;

         default:
            DbgLv(0) << "Unknown message to PMG Master" << tag;
            break;
      }

      // Compute all generations of an MC iteration

      dmga_master_loop();

      // Get the best-fit gene

      qSort(best_fitness);
      dgene = best_dgenes[ best_fitness[ 0 ].index ];

      // Compute the variance (fitness) for the final best-fit model

      calc_residuals_dmga(0, data_sets.size(), simulation_values, dgene);

      // Output the model

      // Write out the model, but skip if not 1st of iteration 1
      bool do_write = (mc_iteration > 1) || (mc_iteration == 1 && my_group == 0);

      if (do_write) {
         if (data_sets.size() == 1) { // Output the single-data model
            write_output();
         }
         else { // Output the global model
            write_global();
         }
      }

      if (my_group == 0) { // Update the tar file of outputs in case of an aborted run
         update_outputs();
      }

      if (current_dataset < count_datasets) { // Before last iteration:  check if max is reset based on time limit
         time_datasets_left(); // Test if near time limit

         tag = (current_dataset < count_datasets) ? DONEITER : DONELAST;
      }

      else { // Mark that max iterations reached
         tag = DONELAST;
      }

      // Tell the supervisor that an iteration is done
      iter = ( int ) maxrss;
      DbgLv(1) << "GaMast:  iter done: maxrss" << iter << "tag" << tag << DONELAST;

      MPI_Send(&iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD);

      DbgLv(1) << "GaMast:  mc_iter iters" << mc_iteration << mc_iterations;
      if (mc_iteration < mc_iterations) { // Set up for next iteration
         if (mc_iteration == 1) { // Set scaled_data the first time
            scaled_data = simulation_values.sim_data;
         }

         set_dmga_MonteCarlo();
      }

      else // Break out of the loop if all iterations have been done
         break;

   } // END:  MC iterations loop


   MPI_Job job;

   // Send finish to workers ( in the tag )
   for (int worker = 1; worker <= my_workers; worker++) {
      MPI_Send(
         &job, // MPI #0
         sizeof(job), MPI_BYTE, worker, FINISHED, my_communicator);
   }
}

// Parallel-masters version of group PCSA master
void US_MPI_Analysis::pm_pcsa_cjmast(void) {
   DbgLv(1) << my_rank << ": master start PCSA" << startTime;
   current_dataset = 0;
   datasets_to_process = 1; // Process one dataset at a time for now

   work_rss.resize(gcores_count);

   int super = 0;
   int kcurve = 0;
   int iter = current_dataset;
   alpha = 0.0;
   mc_iterations = 0;
   max_iterations = parameters[ "gfit_iterations" ].toInt();
   MPI_Status status;

   // Get 1st dataset from supervisor
   MPI_Recv(&current_dataset, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

   int tag = status.MPI_TAG;
   int ittest = current_dataset + mgroup_count;

   init_pcsa_solutes();
   DbgLv(1) << my_rank << ": init sols return";
   fill_pcsa_queue();
   DbgLv(1) << my_rank << ": fill queue return";

   DbgLv(1) << my_rank << ": recvd ds" << current_dataset << "mgc ittest" << mgroup_count << ittest;

   while (true) {
      int worker;
      meniscus_value = data_sets[ current_dataset ]->run_data.meniscus;
      //if ( max_depth > 1 )
      // DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
      //    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while (!job_queue.isEmpty() && worker_status.contains(READY)) {
         worker = ready_worker();
         DbgLv(1) << my_rank << ": submit worker" << worker;

         Sa_Job job = job_queue.takeFirst();
         job.mpi_job.depth = kcurve++;

         submit_pcsa(job, worker);
      }

      // All done with the pass if no jobs are ready or running
      if (job_queue.isEmpty() && !worker_status.contains(WORKING)) {
         kcurve = 0;
         qSort(mrecs);
         US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
         QString tripleID = edata->cell + edata->channel + edata->wavelength;

         simulation_values.variance = mrecs[ 0 ].variance;
         simulation_values.zsolutes = mrecs[ 0 ].csolutes;
         simulation_values.ti_noise = mrecs[ 0 ].ti_noise;
         simulation_values.ri_noise = mrecs[ 0 ].ri_noise;

         QString progress = "Iteration: " + QString::number(iterations);

         if (datasets_to_process > 1)
            progress += "; Datasets: " + QString::number(datasets_to_process);
         else
            progress += "; Dataset: " + QString::number(current_dataset + 1) + " (" + tripleID + ") of "
                        + QString::number(count_datasets);

         if (mc_iterations > 1)
            progress += "; MonteCarlo: " + QString::number(mc_iteration + 1);

         else
            progress += "; RMSD: " + QString::number(mrecs[ 0 ].rmsd);

         send_udp(progress);

         // Iterative refinement
         if (max_iterations > 1) {
            if (data_sets.size() > 1 && iterations == 1) {
               if (datasets_to_process == 1) {
                  qDebug() << "  == Grid-Fit Iterations for Dataset" << current_dataset + 1 << "==";
               }
               else {
                  qDebug() << "  == Grid-Fit Iterations for Datasets 1 to" << datasets_to_process << "==";
               }
            }

            qDebug() << "Iteration:" << iterations << " Variance:" << mrecs[ 0 ].variance << "RMSD:" << mrecs[ 0 ].rmsd;

            iterate_pcsa();
         }

         if (!job_queue.isEmpty())
            continue;

         iterations = 1;
         max_iterations = 1;

         // Clean up mrecs of any empty-calculated-solutes records
         clean_mrecs(mrecs);

         // Write out the model and, possibly, noise(s)
         write_output();

         if (!job_queue.isEmpty())
            continue;

         // Save information from best model
         pcsa_best_model();

         // Tikhonov Regularization
         tikreg_pcsa();

         // Monte Carlo
         montecarlo_pcsa();

         ittest = current_dataset + mgroup_count;

         if (ittest >= count_datasets) {
            for (int jj = 1; jj <= my_workers; jj++)
               maxrss += work_rss[ jj ];
         }

         // Tell the supervisor that an iteration is done
         iter = ( int ) maxrss;
         tag = (ittest < count_datasets) ? DONEITER : DONELAST;

         MPI_Send(&iter, 1, MPI_INT, super, tag, MPI_COMM_WORLD);

         if (current_dataset < count_datasets) {
            if (my_group == 0 && ittest < count_datasets) { // If group 0 master, create an intermediate archive
               update_outputs();
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << " : Intermediate archive was created.";
            }

            US_DataIO::EditedData *edata = &data_sets[ current_dataset ]->run_data;
            QString tripleID = edata->cell + edata->channel + edata->wavelength;

            if (simulation_values.noisflag == 0) {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")"
                        << " : model was output.";
            }
            else {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset + 1 << "(" << tripleID << ")"
                        << " : model/noise(s) were output.";
            }

            time_datasets_left();

            if (ittest < count_datasets) {
               // Get new dataset index from supervisor
               MPI_Recv(&iter, 1, MPI_INT, super, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

               tag = status.MPI_TAG;
               DbgLv(1) << "CJ_MAST Recv tag" << tag << "iter" << iter;

               if (tag == STARTLAST)
                  count_datasets = iter + 1;

               else if (tag != STARTITER) {
                  DbgLv(0) << "Unexpected tag in PMG PCSA Master" << tag;
                  continue;
               }

               current_dataset = iter;
               iterations = 1;
               mc_iteration = 0;
               mc_iterations = 0;
               alpha = 0.0;
               max_iterations = parameters[ "gfit_iterations" ].toInt();
               kcurve = 0;

               fill_pcsa_queue();

               for (int ii = 1; ii <= my_workers; ii++)
                  worker_status[ ii ] = READY;

               continue;
            }
         }

         shutdown_all(); // All done
         break; // Break out of main loop.
      }

      // Wait for worker to send a message
      int sizes[ 4 ];

      MPI_Recv(sizes, 4, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, my_communicator, &status);

      worker = status.MPI_SOURCE;

      //if ( max_depth > 0 )
      DbgLv(1) << my_rank << ": PMG master loop-BOTTOM:   status TAG" << status.MPI_TAG << MPI_Job::READY
               << MPI_Job::RESULTS << "  source" << status.MPI_SOURCE;
      switch (status.MPI_TAG) {
         case MPI_Job::READY: // Ready for work
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_pcsa_results(worker, sizes);
            work_rss[ worker ] = sizes[ 3 ];
            break;

         default: // Should never happen
            QString msg = "Master 2DSA:  Received invalid status " + QString::number(status.MPI_TAG);
            abort(msg);
            break;
      }

      max_rss();
   }
}
