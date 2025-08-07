#include "us_astfem_rsa.h"
#include "us_constants.h"
#include "us_math2.h"
#include "us_mpi_analysis.h"
#include "us_simparms.h"
#include "us_sleep.h"

void US_MPI_Analysis::pcsa_worker(void) {
   bool repeat_loop = true;
   MPI_Job job;
   MPI_Status status;
   DbgLv(1) << "w:" << my_rank << ": pcsa_worker IN";

   // Use 4 here because the master will be reading 4 with the
   // same instruction when reading ::READY or ::RESULTS.
   int x[ 4 ] = { 0, 0, 0, 0 };

   while (repeat_loop) {
      MPI_Send(
         x, // Basically don't care
         4, MPI_INT, MPI_Job::MASTER, MPI_Job::READY,
         my_communicator); // let master know we are ready

      // Blocking -- Wait for instructions
      //if(my_rank==1)
      DbgLv(1) << "w:" << my_rank << ":PM:Recv: 1:job" << sizeof(job);
      MPI_Recv(
         &job, // get masters' response
         sizeof(job), MPI_BYTE, MPI_Job::MASTER, MPI_Job::TAG0, my_communicator,
         &status); // status not used
      //if(my_rank==1)
      DbgLv(1) << "w:" << my_rank << ": job_recvd  length" << job.length << "command" << job.command;

      int offset = job.dataset_offset;
      int dataset_count = job.dataset_count;
      int job_length = job.length;
      int mc_iter = job.solution;
      US_DataIO::EditedData *edata = &data_sets[ offset ]->run_data;
      meniscus_value = edata->meniscus;
      DbgLv(1) << "w:" << my_rank << ": offs cnt" << offset << dataset_count << "menisc" << meniscus_value;
      int scan_count = edata->scanCount();
      int radius_points = edata->pointCount();
      int ds_points = scan_count * radius_points;

      data_sets[ offset ]->simparams.meniscus = meniscus_value;

      switch (job.command) {
         case MPI_Job::PROCESS: // Process solutes
         {
            DbgLv(1) << "w:" << my_rank << ":Recv:PROCESS";
            US_SolveSim::Simulation simulation_values;
            simulation_values.noisflag = (parameters[ "tinoise_option" ].toInt() > 0 ? 1 : 0)
                                         + (parameters[ "rinoise_option" ].toInt() > 0 ? 2 : 0);
            //               simulation_values.dbg_level   = dbg_level;
            simulation_values.dbg_timing = dbg_timing;

            //if(my_rank==1)
            DbgLv(1) << "w:" << my_rank << ": sols size" << job.length;
            simulation_values.zsolutes.resize(job.length);

            DbgLv(1) << "w:" << my_rank << ":PM:Recv: 2:zsol" << job_length * zsolut_doubles;
            MPI_Recv(
               simulation_values.zsolutes.data(), // Get solutes
               job_length * zsolut_doubles, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0, my_communicator, &status);

            max_rss();
            //*DEBUG*
            //if(dbg_level>0 && my_rank==1)
            //if(my_rank==1)
            {
               int nn = simulation_values.zsolutes.size() - 1;
               int mm = nn / 2;
               DbgLv(1) << "w:" << my_rank << ": offs dscnt" << offset << dataset_count << "vbar s20wc bott"
                        << data_sets[ offset ]->vbar20 << data_sets[ offset ]->s20w_correction
                        << data_sets[ offset ]->centerpiece_bottom;
               DbgLv(1) << "w:" << my_rank << ": sol0 solm soln" << simulation_values.zsolutes[ 0 ].x
                        << simulation_values.zsolutes[ 0 ].y << simulation_values.zsolutes[ mm ].x
                        << simulation_values.zsolutes[ mm ].y << simulation_values.zsolutes[ nn ].x
                        << simulation_values.zsolutes[ nn ].y;
            }
            //*DEBUG*

            calc_residuals(offset, dataset_count, simulation_values);

            //*DEBUG*
            //if(my_rank==1)
            {
               int nn = simulation_values.zsolutes.size() - 1;
               int mm = nn / 2;
               DbgLv(1) << "w:" << my_rank << ": nso" << simulation_values.zsolutes.size() << "c:sol0 solm soln"
                        << simulation_values.zsolutes[ 0 ].x << simulation_values.zsolutes[ 0 ].y
                        << simulation_values.zsolutes[ mm ].x << simulation_values.zsolutes[ mm ].y
                        << simulation_values.zsolutes[ nn ].x << simulation_values.zsolutes[ nn ].y;
            }
            //*DEBUG*
            // Tell master we are sending back results
            int sizes[ 4 ] = { simulation_values.zsolutes.size(), simulation_values.ti_noise.size(),
                               simulation_values.ri_noise.size(), ( int ) max_rss() };

            DbgLv(1) << "w:" << my_rank << ":   result sols size" << sizes[ 0 ] << "max_rss" << sizes[ 3 ] << "sizes12"
                     << sizes[ 1 ] << sizes[ 2 ];
            //*DEBUG*
            if (dbg_level == 0 && my_rank == 1) {
               DbgLv(1) << "w:" << my_rank << ":   result sols size" << sizes[ 0 ] << "nsscan"
                        << simulation_values.sim_data.scanCount();
            }
            //*DEBUG*
            MPI_Send(sizes, 4, MPI_INT, MPI_Job::MASTER, MPI_Job::RESULTS, my_communicator);

            // Send back to master all of simulation_values
            MPI_Send(
               simulation_values.zsolutes.data(), sizes[ 0 ] * zsolut_doubles, MPI_DOUBLE, MPI_Job::MASTER,
               MPI_Job::TAG0, my_communicator);

            MPI_Send(&simulation_values.variance, 1, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0, my_communicator);

            MPI_Send(
               simulation_values.variances.data(), dataset_count, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0,
               my_communicator);

            MPI_Send(
               simulation_values.ti_noise.data(), sizes[ 1 ], MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0,
               my_communicator);

            MPI_Send(
               simulation_values.ri_noise.data(), sizes[ 2 ], MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0,
               my_communicator);
         }

         break;

         case MPI_Job::PROCESS_MC: // Process solutes for monte carlo
         {
            DbgLv(1) << "w:" << my_rank << ":Recv:PROCESS_MC" << "mc_iter" << mc_iter << "is_glob" << is_global_fit
                     << "jlen" << job_length;
            double varrmsd = 0.0;
            simulation_values.noisflag = 0;
            //               simulation_values.dbg_level   = dbg_level;
            simulation_values.dbg_timing = dbg_timing;

            //if(my_rank==1)
            DbgLv(1) << "w:" << my_rank << ": sols size" << job.length;
            simulation_values.zsolutes.resize(job.length);

            DbgLv(1) << "w:" << my_rank << ":PM:Recv: 3:zsol" << job_length * zsolut_doubles;
            MPI_Recv(
               simulation_values.zsolutes.data(), // Get solutes
               job_length * zsolut_doubles, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0, my_communicator, &status);

            // Construct a new MC data set with randomized noise
            int index = 0;

            if (is_global_fit) { // Global-fit new variation of base+noise
               DbgLv(1) << "w:" << my_rank << ": MC : global_fit";
               int jsc = 0;
               ds_points = 0;

               for (int ee = 0; ee < dataset_count; ee++) {
                  edata = &data_sets[ ee ]->run_data;
                  int scan_count = edata->scanCount();
                  int radius_points = edata->pointCount();

                  for (int ss = 0; ss < scan_count; ss++, jsc++) {
                     for (int rr = 0; rr < radius_points; rr++, index++) {
                        double vari = US_Math2::box_muller(0.0, sigmas[ index ]);
                        double datout = sim_data->value(jsc, rr) + vari;
                        varrmsd += sq(vari);
                        if (ss < 4 && rr < 4)
                           DbgLv(1) << "w:" << my_rank << ": ee ss rr" << ee << ss << rr << "index vari" << index
                                    << vari << "sigma vrmsd dato" << sigmas[ index ] << varrmsd << datout;

                        edata->setValue(ss, rr, datout);
                     }
                     ds_points += radius_points;
                  }
               }
            }

            else { // Non-global (single-dataset) new variation of base+noise
               edata = &data_sets[ offset ]->run_data;

               for (int ss = 0; ss < scan_count; ss++) {
                  for (int rr = 0; rr < radius_points; rr++, index++) {
                     double vari = US_Math2::box_muller(0.0, sigmas[ index ]);
                     double datout = sim_data->value(ss, rr) + vari;
                     varrmsd += sq(vari);

                     edata->setValue(ss, rr, datout);
                  }
               }
            }

            varrmsd = sqrt(varrmsd / ( double ) ds_points);
            qDebug() << "  Box_Muller Variation RMSD" << QString::number(varrmsd, 'f', 7) << "  for MC_Iteration"
                     << mc_iter;

            max_rss();
            //*DEBUG*
            //if(dbg_level>0 && my_rank==1)
            //if(my_rank==1)
            {
               int nn = simulation_values.zsolutes.size() - 1;
               int mm = nn / 2;
               DbgLv(1) << "w:" << my_rank << ": offs dscnt" << offset << dataset_count << "vbar s20wc bott"
                        << data_sets[ offset ]->vbar20 << data_sets[ offset ]->s20w_correction
                        << data_sets[ offset ]->centerpiece_bottom;
               DbgLv(1) << "w:" << my_rank << ": sol0 solm soln" << simulation_values.zsolutes[ 0 ].x
                        << simulation_values.zsolutes[ 0 ].y << simulation_values.zsolutes[ mm ].x
                        << simulation_values.zsolutes[ mm ].y << simulation_values.zsolutes[ nn ].x
                        << simulation_values.zsolutes[ nn ].y;
            }
            //*DEBUG*

            calc_residuals(offset, dataset_count, simulation_values);

            qDebug() << "Base-Sim RMSD" << sqrt(simulation_values.variance) << "  for MC_Iteration" << mc_iter;

            //*DEBUG*
            //if(my_rank==1)
            {
               int nn = simulation_values.zsolutes.size() - 1;
               int mm = nn / 2;
               DbgLv(1) << "w:" << my_rank << ": nso" << simulation_values.zsolutes.size() << "c:sol0 solm soln"
                        << simulation_values.zsolutes[ 0 ].x << simulation_values.zsolutes[ 0 ].y
                        << simulation_values.zsolutes[ mm ].x << simulation_values.zsolutes[ mm ].y
                        << simulation_values.zsolutes[ nn ].x << simulation_values.zsolutes[ nn ].y;
            }
            //*DEBUG*
            // Tell master we are sending back results
            int sizes[ 4 ] = { simulation_values.zsolutes.size(), mc_iter, 0, ( int ) max_rss() };

            DbgLv(1) << "w:" << my_rank << ":   result sols size" << sizes[ 0 ] << "max_rss" << sizes[ 3 ];
            //*DEBUG*
            if (dbg_level == 0 && my_rank == 1) {
               DbgLv(1) << "w:" << my_rank << ":   result sols size" << sizes[ 0 ] << "nsscan"
                        << simulation_values.sim_data.scanCount();
            }
            //*DEBUG*
            DbgLv(1) << "w:" << my_rank << ":Send:RESULTS_MC  sizes" << sizes[ 0 ] << sizes[ 1 ] << sizes[ 2 ]
                     << sizes[ 3 ];
            MPI_Send(sizes, 4, MPI_INT, MPI_Job::MASTER, MPI_Job::RESULTS_MC, my_communicator);

            // Send back to master all of simulation_values
            MPI_Send(
               simulation_values.zsolutes.data(), sizes[ 0 ] * zsolut_doubles, MPI_DOUBLE, MPI_Job::MASTER,
               MPI_Job::TAG0, my_communicator);

            MPI_Send(&simulation_values.variance, 1, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0, my_communicator);

            MPI_Send(
               simulation_values.variances.data(), dataset_count, MPI_DOUBLE, MPI_Job::MASTER, MPI_Job::TAG0,
               my_communicator);
         } break;

         case MPI_Job::NEWDATA: // Reset data for Monte Carlo or global fit
         {
            //if(my_rank==1)
            DbgLv(1) << "w:" << my_rank << ":Recv:NEWDATA  joblen" << job_length;
            if (is_global_fit && mc_iter < 3 && my_rank < 3) { // For global fits, check the memory requirements
               long memused = max_rss();
               long memdata = job_length * sizeof(double);
               int grid_reps = qMax(parameters[ "uniform_grid" ].toInt(), 1);
               double s_pts = 60.0;
               double ff0_pts = 60.0;
               if (parameters.contains("s_grid_points"))
                  s_pts = parameters[ "s_grid_points" ].toDouble();
               else if (parameters.contains("s_resolution"))
                  s_pts = parameters[ "s_resolution" ].toDouble() * grid_reps;
               if (parameters.contains("ff0_grid_points"))
                  ff0_pts = parameters[ "ff0_grid_points" ].toDouble();
               else if (parameters.contains("ff0_resolution"))
                  ff0_pts = parameters[ "ff0_resolution" ].toDouble() * grid_reps;
               int nsstep = ( int ) (s_pts);
               int nkstep = ( int ) (ff0_pts);
               int maxsols = nsstep * nkstep;
               long memamatr = memdata * maxsols;
               long membmatr = memdata;
               long memneed = memdata + memamatr + membmatr;
               const double mb_bytes = (1024. * 1024.);
               const double gb_bytes = (mb_bytes * 1024.);
               double gb_need = ( double ) memneed / gb_bytes;
               gb_need = qRound(gb_need * 1000.0) * 0.001;
               double gb_used = ( double ) memused / mb_bytes;
               gb_used = qRound(gb_used * 1000.0) * 0.001;
               long pgavail = sysconf(_SC_PHYS_PAGES);
               long pgsize = sysconf(_SC_PAGE_SIZE);
               long memavail = pgavail * pgsize;
               double gb_avail = ( double ) memavail / gb_bytes;
               gb_avail = qRound(gb_avail * 1000.0) * 0.001;
               long pgcurav = sysconf(_SC_AVPHYS_PAGES);
               long memcurav = pgcurav * pgsize;
               double gb_curav = ( double ) memcurav / gb_bytes;
               gb_curav = qRound(gb_curav * 1000.0) * 0.001;

               qDebug() << "++ Worker" << my_rank << ": MC iteration" << mc_iter << ": Memory Profile :"
                        << "\n    Maximum memory used to this point" << memused << "\n    Composite data memory needed"
                        << memdata << "\n    Maximum subgrid solute count" << maxsols
                        << "\n    NNLS A matrix memory needed" << memamatr << "\n    NNLS B matrix memory needed"
                        << membmatr << "\n    Total memory (GB) used" << gb_used << "\n    Total memory (GB) needed"
                        << gb_need << "\n    Total memory (GB) available" << gb_avail
                        << "\n    Memory (GB) currently available" << gb_curav;
            }

            mc_data.resize(job_length);

            if (mc_data.size() != job_length) {
               DbgLv(0) << "*ERROR* mc_data.size() job_length" << mc_data.size() << job_length;
            }

            MPI_Barrier(my_communicator);

            if (my_rank == 1 || my_rank == 11)
               DbgLv(1) << "newD:" << my_rank << " scld/newdat rcv : offs dsknt" << offset << dataset_count << "joblen"
                        << job_length;
            double dsum = 0.0;
            // This is a receive
            DbgLv(1) << "w:" << my_rank << ":PM:Recv: 5:bcast-mcd" << job_length;
            MPI_Bcast(mc_data.data(), job_length, MPI_DOUBLE, MPI_Job::MASTER, my_communicator);


            if (is_global_fit && dataset_count == 1) { // For global update to scaled data, extra value is new ODlimit
               job_length--;
               data_sets[ offset ]->run_data.ODlimit = mc_data[ job_length ];
               if ((my_rank == 1 || my_rank == 11))
                  DbgLv(1) << "newD:" << my_rank << ":offset ODlimit" << offset
                           << data_sets[ offset ]->run_data.ODlimit;
            }

            bool is_simdat = (mc_iter >= 10000);
            int index = 0;
            DbgLv(1) << "newD:" << my_rank << ": is_simdat" << is_simdat;

            if (is_simdat) { // If simulation,residuals for MC; save and get sigmas
               sim_data = &sim_data1;

               if (is_global_fit) { // Global-fit simdata,sigmas
                  QList<US_DataIO::EditedData *> edats;
                  edats.reserve(dataset_count);
                  for (int ee = 0; ee < dataset_count; ee++) {
                     edats << &data_sets[ ee ]->run_data;
                  }

                  US_AstfemMath::initSimData(sim_data1, edats);
                  sim_data = &sim_data1;
                  int jda = 0;
                  int jre = jda + total_points;
                  int ksc = 0;

                  for (int ee = 0; ee < dataset_count; ee++) {
                     int scan_count = data_sets[ ee ]->run_data.scanCount();
                     int radius_points = data_sets[ ee ]->run_data.pointCount();

                     for (int ss = 0; ss < scan_count; ss++, ksc++) {
                        for (int rr = 0; rr < radius_points; rr++) {
                           sim_data->setValue(ksc, rr, mc_data[ jda++ ]);
                           sigmas << qAbs(mc_data[ jre++ ]);
                        }
                     }
                     DbgLv(1) << "newD:" << my_rank << ":    ee jda jre" << ee << jda << jre;
                  }
                  DbgLv(1) << "newD:" << my_rank << ":   jre mcsize" << jre << mc_data.size() << "ksc" << ksc
                           << "sigsize" << sigmas.size() << dataset_count;
               }

               else { // Non-global (single-dataset) simdata,sigmas
                  US_DataIO::EditedData *edata = &data_sets[ offset ]->run_data;
                  US_AstfemMath::initSimData(sim_data1, *edata, 0.0);
                  int scan_count = edata->scanCount();
                  int radius_points = edata->pointCount();
                  sigmas.clear();

                  for (int ss = 0; ss < scan_count; ss++) // Save sim_data
                     for (int rr = 0; rr < radius_points; rr++)
                        sim_data->setValue(ss, rr, mc_data[ index++ ]);

                  for (int ss = 0; ss < scan_count; ss++) // Get sigmas
                     for (int rr = 0; rr < radius_points; rr++)
                        sigmas << qAbs(mc_data[ index++ ]);
                  DbgLv(1) << "newD:" << my_rank << ":   index mcsize" << index << mc_data.size();
               }

               // Stagger restart of processing for each worker
               US_Sleep::msleep(my_rank * 200);
            }

            else { // Straight replacement of experiment data
               for (int ee = offset; ee < offset + dataset_count; ee++) {
                  US_DataIO::EditedData *edata = &data_sets[ ee ]->run_data;

                  int scan_count = edata->scanCount();
                  int radius_points = edata->pointCount();

                  int indxh = ((scan_count / 2) * radius_points) + (radius_points / 2);
                  for (int ss = 0; ss < scan_count; ss++) {
                     for (int rr = 0; rr < radius_points; rr++, index++) {
                        edata->setValue(ss, rr, mc_data[ index ]);
                        dsum += edata->value(ss, rr);
                        if (
                           (my_rank == 1 || my_rank == 11)
                           && (index < 5 || index > (job_length - 6) || (index > (indxh - 4) && index < (indxh + 3))))
                           DbgLv(1) << "newD:" << my_rank << ":index" << index << "edat" << edata->value(ss, rr) << "ee"
                                    << ee;
                     }
                  }
               }
               if (my_rank == 1 || my_rank == 11)
                  DbgLv(1) << "newD:" << my_rank << "  length index" << job_length << index << "dsum" << dsum;
            }
         }

         break;

         default:
            repeat_loop = false;
            break;
      } // switch
   } // repeat_loop
}
