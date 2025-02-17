#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_tar.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

void US_MPI_Analysis::_2dsa_master( void )
{
   init_solutes();
   fill_queue();
   bool shutdown = false;
   work_rss.resize( gcores_count );

   current_dataset     = 0;
   datasets_to_process = count_datasets;
   dset_calc_solutes.clear();

   int max_iters_all   = max_iterations;

   if ( mc_iterations > 1 )
      max_iterations   = max_iters_all > 1 ? max_iters_all : 5;

   menibott_ndx        = 0;
   meniscus_run        = 0;
   bottom_run          = 0;
   if ( fit_mb_select == 0 )
   {
      menibott_count      = 1;
      meniscus_points     = 1;
      bottom_points       = 1;
   }
   else if ( fit_menbot )
   {
      menibott_count      = meniscus_points * bottom_points;
   }
   else if ( fit_meni )
   {
      menibott_count      = meniscus_points;
      bottom_points       = 1;
   }
   else if ( fit_bott )
   {
      menibott_count      = bottom_points;
      meniscus_points     = 1;
   }

   while ( true )
   {
      int worker;
      meniscus_value       = ( meniscus_points == 1 )
                           ? data_sets[ current_dataset ]->run_data.meniscus
                           : meniscus_values[ meniscus_run ];
      bottom_value       = ( bottom_points == 1 )
                           ? data_sets[ current_dataset ]->run_data.bottom
                           : bottom_values  [ bottom_run   ];
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         worker    = ready_worker();

         Sa_Job job              = job_queue.takeFirst();
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // All done with the pass if no jobs are ready or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) )
      {
         US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
         QString tripleID = edata->cell + edata->channel + edata->wavelength;
         QString progress =
            "Iteration: "    + QString::number( iterations );

         if ( datasets_to_process > 1 )
            progress     += "; Datasets: "
                            + QString::number( datasets_to_process );
         else
            progress     += "; Dataset: "
                            + QString::number( current_dataset + 1 )
                            + " (" + tripleID + ") of "
                            + QString::number( count_datasets );

         if ( mc_iterations > 1 )
            progress     += "; MonteCarlo: "
                            + QString::number( mc_iteration + 1 );

         else if ( fit_menbot )
            progress     += "; Meniscus: "
               + QString::number( meniscus_value, 'f', 4 )
               + "; Bottom: "
               + QString::number( bottom_value,   'f', 4 )
               + tr( "  ( m%1 b%2 )").arg( meniscus_run + 1 ).arg( bottom_run + 1 );
         else if ( fit_meni )
            progress     += "; Meniscus: "
               + QString::number( meniscus_value, 'f', 4 )
               + tr( " (%1 of %2)" ).arg( meniscus_run + 1 )
                                    .arg( meniscus_points );
         else if ( fit_bott )
            progress     += "; Bottom: "
               + QString::number( bottom_value,   'f', 4 )
               + tr( " (%1 of %2)" ).arg( bottom_run + 1 )
                                    .arg( bottom_points );
         else
            progress     += "; RMSD: "
               + QString::number( sqrt( simulation_values.variance ) );

         send_udp( progress );

         // Iterative refinement
         if ( max_iterations > 1 )
         {
            if ( data_sets.size() > 1  &&  iterations == 1 )
            {
               if ( datasets_to_process == 1 )
               {
                  qDebug() << "   == Refinement Iterations for Dataset"
                     << current_dataset + 1 << "==";
               }
               else
               {
                  qDebug() << "   == Refinement Iterations for Datasets 1 to"
                     << datasets_to_process << "==";
               }
            }
            qDebug() << "Iteration:" << iterations << " Variance:"
               << simulation_values.variance << "RMSD:"
               << sqrt( simulation_values.variance );

            iterate();
         }

         if ( ! job_queue.isEmpty() ) continue;

         iterations = 1;
DbgLv(1) << " master loop-BOT: dssize" << data_sets.size() << "ds_to_p"
 << datasets_to_process << "curr_ds" << current_dataset;
US_DataIO::EditedData* edat=&data_sets[current_dataset]->run_data;
int ks=edat->scanCount() - 10;
int kr=edat->pointCount() - 10;
int ss=ks/2;
int rr=kr/2;
DbgLv(1) << " master loop-BOT: ds" << current_dataset+1 << "data l m h"
 << edat->value(10,10) << edat->value(ss,rr) << edat->value(ks,kr);
DbgLv(1) << " master loop-BOT: GF job_queue empty" << job_queue.isEmpty();

         if ( ! job_queue.isEmpty() ) continue;

         if ( is_global_fit )
            write_global();

         else
            write_output();

         // Fit meniscus
         if ( ( menibott_ndx + 1 ) < menibott_count )
         {
            set_meniscus();
         }

         if ( ! job_queue.isEmpty() ) continue;

         // Monte Carlo
         if ( mc_iterations > 1 )
         {  // Recompute final fit to get simulation and residual
            mc_iteration++;
            wksim_vals           = simulation_values;
            wksim_vals.solutes   = calculated_solutes[ max_depth ];
            int bfg_offset = -1;
            int csd_offset = -1;
            if (data_sets.size() == 1){
                if (!data_sets[current_dataset]->solution_rec.buffer.cosed_component.isEmpty()) {
                    US_SimulationParameters simulationParameters = data_sets[current_dataset]->simparams;
                    US_DataIO::RawData pedata = data_sets[current_dataset]->run_data.convert_to_raw_data();
                    US_SolveSim::DataSet* dataSet = data_sets[current_dataset];
                    // we need a
                    for (int i = 0; i < data_sets_bfgs.length(); i++){
                        US_Math_BF::Band_Forming_Gradient* bfg = &data_sets_bfgs[i];
                        if (dataSet->solution_rec.buffer.cosed_component == bfg->cosed_component ||
                            abs(simulationParameters.meniscus - bfg->meniscus) < GSL_ROOT5_DBL_EPSILON ||
                            abs(simulationParameters.bottom - bfg->bottom) < GSL_ROOT5_DBL_EPSILON ||
                            abs(simulationParameters.band_volume - bfg->overlay_volume) < GSL_ROOT5_DBL_EPSILON ||
                            abs( simulationParameters.cp_pathlen - bfg->cp_pathlen) < GSL_ROOT5_DBL_EPSILON ||
                            abs( simulationParameters.cp_angle - bfg->cp_angle) < GSL_ROOT5_DBL_EPSILON ||
                            simulationParameters.radial_resolution == bfg->simparms.radial_resolution ||
                            simulationParameters.temperature == bfg->simparms.temperature ||
                            pedata.scanData.last().seconds < bfg->dens_bfg_data.scanData.last().seconds){
                            // recalculation needed
                            bfg_offset = i;
                            break;
                        }
                    }
                    if (bfg_offset == -1){
                        US_Math_BF::Band_Forming_Gradient bfg = US_Math_BF::Band_Forming_Gradient(
                                simulationParameters.meniscus,
                                simulationParameters.bottom,
                                simulationParameters.band_volume,
                                dataSet->solution_rec.buffer.cosed_component,
                                simulationParameters.cp_pathlen,
                                simulationParameters.cp_angle);
                       bfg.get_eigenvalues();
                       bfg.calculate_gradient(simulationParameters, &pedata);
                       data_sets_bfgs << bfg;
                       bfg_offset = data_sets_bfgs.length() -1;
                    }
                }
            }
            calc_residuals( 0, data_sets.size(), wksim_vals, bfg_offset );

            qDebug() << "Base-Sim RMSD" << sqrt( simulation_values.variance )
                     << "  Exp-Sim RMSD" << sqrt( wksim_vals.variance )
                     << "  of MC_Iteration" << mc_iteration;
            max_iterations              = max_iters_all;
            simulation_values           = wksim_vals;

            if ( mc_iteration < mc_iterations )
            {
               time_mc_iterations();

               set_monteCarlo();
            }
         }

         if ( ! job_queue.isEmpty() ) continue;

         if ( is_composite_job )
         {  // Composite job:  update outputs in TAR and bump dataset count
            QString ttripleID = QString( data_sets[ current_dataset ]->model
                               .description ).section( ".", -3, -3 );
            current_dataset++;
            dset_calc_solutes << calculated_solutes[ max_depth ];

            update_outputs();

            if ( simulation_values.noisflag == 0 )
            {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset
                        << "(" << ttripleID << ")"
                        << " :  model was output.";
            }
            else
            {
               DbgLv(0) << my_rank << ": Dataset" << current_dataset
                        << "(" << ttripleID << ")"
                        << " :  model/noise(s) were output.";
            }

DbgLv(1) << " master loop-BOT:    cds kds" << current_dataset << count_datasets;
            if ( current_dataset < count_datasets )
            {
               menibott_ndx    = 0;
               meniscus_run    = 0;
               bottom_run      = 0;
               iterations      = 1;
               mc_iteration    = 0;

               if ( menibott_count > 1 )
               {  // Reset the range of fit-meniscus/bottom points for this data set
                  US_DataIO::EditedData* EditedData
                                  = &data_sets[ current_dataset ]->run_data;
                  double dat_str  = EditedData->radius( 0 );
                  double men_dpt  = ( meniscus_points > 1 ) ?
                                    (double)( meniscus_points - 1 ) : 1;
                  double bot_dpt  = ( bottom_points > 1 ) ?
                                    (double)( bottom_points   - 1 ) : 1;
                  double men_str  = EditedData->meniscus - ( meniscus_range * 0.5 );
                  double bot_str  = EditedData->bottom   - ( bottom_range   * 0.5 );
                  double men_inc  = meniscus_range / men_dpt;
                  double bot_inc  = bottom_range   / bot_dpt;
                  double men_end  = men_str + meniscus_range - men_inc;
                  if ( men_end >= dat_str )
                  {  // Adjust first meniscus so range remains below data range
                     men_end         = dat_str - ( men_inc * 0.5 );
                     men_str         = men_end - meniscus_range + men_inc;
                  }
                  for ( int ii = 0; ii < meniscus_points; ii++ )
                     meniscus_values[ ii ] = men_str + men_inc * ii;
                  for ( int ii = 0; ii < bottom_points; ii++ )
                     bottom_values[ ii ]   = bot_str + bot_inc * ii;
DbgLv(0) << " master loop-BOT:     menpt" << meniscus_points << "mv0 mvn"
 << meniscus_values[0] << meniscus_values[meniscus_points-1]
 << "botpt" << bottom_points << "bv0 bvn"
 << bottom_values[0] << bottom_values[bottom_points-1]
 << "gcores_count" << gcores_count;
               }

//               for ( int ii = 1; ii < gcores_count; ii++ )
//                  worker_status[ ii ] = READY;

               fill_queue();

               for ( int ii = 1; ii < gcores_count; ii++ )
                  worker_status[ ii ] = READY;
DbgLv(1) << " master loop-BOT:      wkst1 wkstn" << worker_status[1]
 << worker_status[gcores_count-1];

               for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
                  calculated_solutes[ ii ].clear();

               continue;
            }
         }

         shutdown_all();  // All done
         shutdown = true;
         break;           // Break out of main loop.
      }
      if ( shutdown ) return;
      // Wait for worker to send a message
      int        sizes[ 4 ];
      MPI_Status status;

      MPI_Recv( sizes,
                4,
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status);

      worker = status.MPI_SOURCE;

if ( max_depth > 0 )
 DbgLv(1) << " master loop-BOTTOM:   status TAG" << status.MPI_TAG
  << MPI_Job::READY << MPI_Job::RESULTS << "  source" << status.MPI_SOURCE;
      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
            process_results( worker, sizes );
            work_rss[ worker ] = sizes[ 3 ];
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

// Generate the initial set of solutes
void US_MPI_Analysis::init_solutes( void )
{
   calculated_solutes.clear();
   orig_solutes.clear();
   simulation_values.noisflag    = ( parameters[ "tinoise_option" ].toInt() > 0 ?
                                     1 : 0 )
                                 + ( parameters[ "rinoise_option" ].toInt() > 0 ?
                                     2 : 0 );
   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   // Test to see if there is a custom grid model
   QString model_filename = data_sets[ 0 ]->model_file;
   double  cnstff0        = 0.0;

   if ( model_filename.isEmpty() )
   {  // If no model file given, calculate initial solutes in a fixed grid

      double s_min   = parameters[ "s_min"           ].toDouble() * 1.0e-13;
      double s_max   = parameters[ "s_max"           ].toDouble() * 1.0e-13;
      double ff0_min = parameters[ "ff0_min"         ].toDouble();
      double ff0_max = parameters[ "ff0_max"         ].toDouble();

      int grid_reps  = qMax( parameters[ "uniform_grid" ].toInt(), 1 );
      double s_pts   = 60.0;
      double ff0_pts = 60.0;

      if ( parameters.contains( "s_grid_points"   ) )
         s_pts   = parameters[ "s_grid_points"   ].toDouble();

      else if ( parameters.contains( "s_resolution"    ) )
         s_pts   = parameters[ "s_resolution"    ].toDouble() * grid_reps;

      if ( parameters.contains( "ff0_grid_points" ) )
         ff0_pts = parameters[ "ff0_grid_points" ].toDouble();

      else if ( parameters.contains( "ff0_resolution"  ) )
         ff0_pts = parameters[ "ff0_resolution"  ].toDouble() * grid_reps;

      int    nsstep   = (int)( s_pts );
      int    nkstep   = (int)( ff0_pts );

DbgLv(0) << "InSol: nss nks" << s_pts << ff0_pts << nsstep << nkstep << "grid_reps" << grid_reps;
      US_Solute::init_solutes( s_min,   s_max,   nsstep,
                               ff0_min, ff0_max, nkstep,
                               grid_reps, cnstff0, orig_solutes );
DbgLv(0) << "InSol:  s range" << s_min*1.e+13 << s_max*1.e+13 << "k range"
 << ff0_min << ff0_max;
int j0=orig_solutes.count()-1;
int j1=orig_solutes[0].count()-1;
int j2=orig_solutes[j0].count()-1;
DbgLv(0) << "orig_solutes:"
 << orig_solutes[ 0][ 0].s*1.e+13 << orig_solutes[ 0][ 0].k << "  "
 << orig_solutes[ 0][j1].s*1.e+13 << orig_solutes[ 0][j1].k << "  "
 << orig_solutes[j0][ 0].s*1.e+13 << orig_solutes[j0][ 0].k << "  "
 << orig_solutes[j0][j2].s*1.e+13 << orig_solutes[j0][j2].k;
   }

   else
   {  // If a model file was given, use it to set the initial solutes
      US_Model model;
      QString fn = "../" + model_filename;
      model.load( fn );
      int    nsubgrid = model.subGrids;
      int    ncomps   = model.components.size();
DbgLv(0) << "InSol: fn" << fn;
DbgLv(0) << "InSol:  nsubgrid ncomps" << nsubgrid << ncomps;

      if ( nsubgrid < 1 )
         abort( "Custom grid model file has no subgrids", -1 );

      if ( ( ncomps / nsubgrid ) > 150 )
      {  // Subgrids too large:  adjust subgrid count and size
         nsubgrid       = ( ncomps / 100 + 1 ) | 1;
         model.subGrids = nsubgrid;
DbgLv(0) << "InSol:  nsubgrid sbsize" << nsubgrid << ( ncomps / nsubgrid );
      }

      QVector< US_Solute > solvec;

      for ( int ii = 0; ii < nsubgrid; ii++ )
      {
         solvec.clear();

         for ( int jj = ii; jj < ncomps; jj += nsubgrid )
         {
            double ffval    = model.components[ jj ].f_f0;
            double vbval    = model.components[ jj ].vbar20;

            // Save each solute contained in the custom grid model
            US_Solute soli( model.components[ jj ].s,
                            ffval,
                            0.0,
                            vbval,
                            model.components[ jj ].D );
            solvec << soli;
         }

         orig_solutes << solvec;
      }
   }
}

// Fill the job queue, using the list of initial solutes
void US_MPI_Analysis::fill_queue( void )
{
   DbgLv(1) << "start filling queue";
   worker_status.resize( gcores_count );
   worker_depth .resize( gcores_count );

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   max_depth           = 0;
   worknext            = 1;
   max_experiment_size = min_experiment_size;

   // Put all jobs in the queue
   job_queue.clear();

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      max_experiment_size = qMax( max_experiment_size,
                                  orig_solutes[ i ].size() );
      Sa_Job job;
      job.solutes         = orig_solutes[ i ];
      job_queue << job;
   }
   DbgLv(1) << "queue filled";
}

//////////////////
void US_MPI_Analysis::global_fit( void )
{
   // To do a global fit across multiple data sets:
   //  1. Each individual data set must be run
   //  2. Sum the total concentration of all returned solutes
   //  3. Divide all experiment concentrations by the total concentration
   //  4. Send the concentration-equalized data to the workers
   //  5. Do an additional run against the combined datasets for the baseline
   // Any additional Monte Carlo iterations will use the adjusted data for
   // all data sets.

   double concentration = 0.0;
   US_Model::AnalysisType mdl_type = model_type( analysis_type );

   // The first dataset is done automatically.
   if ( mdl_type != US_Model::PCSA )
   {
      for ( int solute = 0; solute < simulation_values.solutes.size(); solute++ )
      {
         concentration += simulation_values.solutes[ solute ].c;
      }
DbgLv(1) << ":gf: 2DSA: nsols" << simulation_values.solutes.size() << "concen" << concentration;
   }
   else
   {
      for ( int solute = 0; solute < simulation_values.zsolutes.size(); solute++ )
      {
         concentration += simulation_values.zsolutes[ solute ].c;
      }
DbgLv(1) << ":gf: PCSA: nzsols" << simulation_values.zsolutes.size() << "concen" << concentration;
   }

   qDebug() << "   == Dataset" << current_dataset + 1
            << "Total Concentration" << concentration << "==";

   // Point to current dataset
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;

   concentrations[ current_dataset ] = concentration;
   edata->ODlimit   /= concentration;
   int scan_count    = edata->scanCount();
   int radius_points = edata->pointCount();
int ks=scan_count;
int kp=radius_points;
DbgLv(1) << ":gf:  ee" << current_dataset << "BB-dset(m)" << edata->value(ks/2,kp/2);
   int index         = 0;
   QVector< double > pscaled_data(scan_count * radius_points + 1 );
double isum=0.0;
double dsum=0.0;

   // Scale the data
   for ( int ss = 0; ss < scan_count; ss++ )
   {
      for ( int rr = 0; rr < radius_points; rr++ )
      {
double ival=edata->value(ss,rr);
isum+=ival;
         double scaled_value    = edata->value( ss, rr ) / concentration;
         pscaled_data[ index++ ] = scaled_value;
         edata->setValue( ss, rr, scaled_value );
dsum+=scaled_value;
      }
   }

   pscaled_data[ index ] = edata->ODlimit;
DbgLv(0) << "ScaledData sum" << dsum << "iSum" << isum << "concen" << concentration;

   // Send the scaled version of current data to the workers
   MPI_Job job;
   job.command         = MPI_Job::NEWDATA;
   job.length          = pscaled_data.size();
   job.solution        = 1;
   job.meniscus_value  = data_sets[ current_dataset ]->run_data.meniscus;
   job.dataset_offset  = current_dataset;
   job.dataset_count   = 1;

   // Tell each worker that new data is coming.
   // Cannot use a broadcast, since the worker is expecting a Send.
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      job.solution        = worker;
      job.depth           = worker;
      MPI_Send( &job,
                sizeof( MPI_Job ),
                MPI_BYTE,
                worker,
                MPI_Job::MASTER,
                my_communicator );
   }

   // Get everybody synced up
   MPI_Barrier( my_communicator );

   MPI_Bcast(pscaled_data.data(),
             pscaled_data.size(),
             MPI_DOUBLE,
             MPI_Job::MASTER,
             my_communicator );

   // Go to the next dataset
   job_queue.clear();
   if ( mdl_type != US_Model::PCSA )
      dset_calc_solutes << calculated_solutes[ max_depth ];
   current_dataset++;

   if ( current_dataset >= count_datasets )
   {  // If all datasets have been scaled, do all datasets from now on
      datasets_to_process = count_datasets;
      current_dataset     = 0;
   }

   else
   {
      for ( int ii = 1; ii < gcores_count; ii++ )
         worker_status[ ii ] = READY;
   }

   if ( mdl_type != US_Model::PCSA )
   {
      fill_queue();

      for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
         calculated_solutes[ ii ].clear();
   }

   else
   {
      fill_pcsa_queue();

      for ( int ii = 0; ii < calculated_zsolutes.size(); ii++ )
         calculated_zsolutes[ ii ].clear();
DbgLv(1) << "ScaledData fill/solclear complete";

      iterations      = 1;
      max_iterations  = parameters[ "gfit_iterations" ].toInt();
   }
}

// Reset for a fit-meniscus iteration
void US_MPI_Analysis::set_meniscus( void )
{
   menibott_ndx++;

   if ( fit_menbot )
   {
      meniscus_run   = menibott_ndx / meniscus_points;
      bottom_run     = menibott_ndx % meniscus_points;
   }
   else if ( fit_meni )
   {
      meniscus_run   = menibott_ndx;
   }
   else if ( fit_bott )
   {
      bottom_run     = menibott_ndx;
   }
   else
   {
DbgLv(0) << "FMB:set_meniscus: HUH????: mb m b:"
 << fit_menbot << fit_meni << fit_bott;
   }
DbgLv(0) << "FMB:set_meniscus:  mb_ndx men_run bot_run"
 << menibott_ndx << meniscus_run << bottom_run << "mb m b mbc"
 << fit_menbot << fit_meni << fit_bott << menibott_count;

   // We incremented meniscus_run above.  Just rerun from the beginning.
   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      Sa_Job job;
      job.solutes = orig_solutes[ i ];

      job_queue << job;
   }

   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

// Reset for a Monte Carlo iteration
void US_MPI_Analysis::set_monteCarlo( void )
{
DbgLv(1) << "sMC: max_depth" << max_depth << "calcsols size" << calculated_solutes[max_depth].size()
 << "simvsols size" << simulation_values.solutes.size();

   // Set up new data modified by a gaussian distribution
   if ( mc_iteration == 1 )
   {
      set_gaussians();

      sim_data1      = simulation_values.sim_data;

      double fitrmsd = sqrt( simulation_values.variance );
      qDebug() << "  MC_Iteration 1 Simulation RMSD"
               << QString::number( fitrmsd, 'f', 7 );
   }

   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   int dsPoints     = total_points;
   int ds_start      = 0;
   int ds_end        = count_datasets;

   if ( is_composite_job )
   {
      dsPoints         = edata->scanCount() * edata->pointCount();
      ds_start          = current_dataset;
      ds_end            = ds_start + datasets_to_process;
   }
   mc_data.resize(dsPoints );
DbgLv(1) << "sMC: totpts" << total_points << "mc_iter" << mc_iteration;
DbgLv(1) << "sMC:  sig-size" << sigmas.count() << "mcd-size" << mc_data.count()
 << "tot-scans" << sim_data1.scanCount();
int mcdscn=sim_data1.scanCount();
int sigsiz=sigmas.count();
int mcdsiz=mc_data.count();
   int    index      = 0;
   int    scnx       = 0;
   double varrmsd    = 0.0;
   double varisum    = 0.0;
   double varimin    = 1.0;
   double varimax    = -1.0;
   double datasum    = 0.0;

   // Get a randomized variation of the concentrations
   // Use a gaussian distribution with the residual as the standard deviation
   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
int indxh=((scan_count/2)*radius_points)+(radius_points/2);

      for ( int ss = 0; ss < scan_count; ss++, scnx++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
if(scnx>=mcdscn)
DbgLv(1) << "sMC:  *ERR* scnx" << scnx << "mcdscn" << mcdscn << "ee,ss,rr" << ee << ss << rr;
if(index>=sigsiz)
DbgLv(1) << "sMC:  *ERR* index" << index << "sigsiz" << sigsiz << "ee,ss,rr" << ee << ss << rr;
if(index>=mcdsiz)
DbgLv(1) << "sMC:  *ERR* index" << index << "mcdsiz" << mcdsiz << "ee,ss,rr" << ee << ss << rr;
            double variation = US_Math2::box_muller( 0.0, sigmas[ index ] );
            double mcdata    = sim_data1.value( scnx, rr ) + variation;
            varrmsd         += sq( variation );
            varisum         += variation;
            varimin          = qMin( varimin, variation );
            varimax          = qMax( varimax, variation );
            datasum         += mcdata;

if ( index<5 || index>(total_points-6) || (index>(indxh-4)&&index<(indxh+3)) )
DbgLv(1) << "sMC:  index" << index << "sdat" << sim_data1.value(scnx,rr)
 << "sigma" << sigmas[index] << "vari" << variation << "mdat" << mcdata;

            mc_data[ index++ ] = mcdata;
         }
      }
   }
DbgLv(1) << "sMC:   mcdata sum" << datasum;

   varrmsd          = sqrt( varrmsd / (double)( dsPoints ) );
   qDebug() << "  Box_Muller Variation RMSD"
            << QString::number( varrmsd, 'f', 7 )
            << "  for MC_Iteration" << mc_iteration + 1;

DbgLv(1) << "sMC:   variation  sum min max" << varisum << varimin << varimax
 << "mcdata sum" << datasum;

   // Broadcast Monte Carlo data to all workers
   MPI_Job newdata;
   newdata.command        = MPI_Job::NEWDATA;
   newdata.length         = dsPoints;
   newdata.solution       = mc_iteration + 1;
   newdata.meniscus_value = data_sets[ 0 ]->run_data.meniscus;
   newdata.dataset_offset = ds_start;
   newdata.dataset_count  = ds_end - ds_start;

   // Tell each worker that new data coming
   // Can't use a broadcast because the worker is expecting a Send
DbgLv(1) << "sMC: MPI send   my_workers" << my_workers;
   for ( int worker = 1; worker <= my_workers; worker++ )
   {
      MPI_Send( &newdata,
          sizeof( MPI_Job ),
          MPI_BYTE,
          worker,
          MPI_Job::MASTER,
          my_communicator );
   }

   // Get everybody synced up
DbgLv(1) << "sMC: MPI Barrier";
   MPI_Barrier( my_communicator );

DbgLv(1) << "sMC: MPI Bcast";
   MPI_Bcast(mc_data.data(),
             dsPoints,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             my_communicator );

   fill_queue();

   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();
}

//////////////////
//  Generate the simulated data and calculate the residuals
//  Use the residuals as the standard deviation for varying the
//  data in Monte Carlo iterations
void US_MPI_Analysis::set_gaussians( void )
{
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   int ds_start      = 0;
   int ds_end        = count_datasets;
   int ds_count      = count_datasets;
   int bfg_offset    = -1;
   int csd_offset = -1;
   if ( is_composite_job )
   {
      ds_start          = current_dataset;
      ds_end            = ds_start + datasets_to_process;
      ds_count          = datasets_to_process;
   }
   else {
       if (!data_sets[current_dataset]->solution_rec.buffer.cosed_component.isEmpty()) {
           US_SimulationParameters simulationParameters = data_sets[current_dataset]->simparams;
           US_DataIO::RawData pedata = data_sets[current_dataset]->run_data.convert_to_raw_data();
           US_SolveSim::DataSet* dataSet = data_sets[current_dataset];
           // we need a
           for (int i = 0; i < data_sets_bfgs.length(); i++){
               US_Math_BF::Band_Forming_Gradient* bfg = &data_sets_bfgs[i];
               if (dataSet->solution_rec.buffer.cosed_component == bfg->cosed_component ||
                   abs(simulationParameters.meniscus - bfg->meniscus) < GSL_ROOT5_DBL_EPSILON ||
                   abs(simulationParameters.bottom - bfg->bottom) < GSL_ROOT5_DBL_EPSILON ||
                   abs(simulationParameters.band_volume - bfg->overlay_volume) < GSL_ROOT5_DBL_EPSILON ||
                   abs( simulationParameters.cp_pathlen - bfg->cp_pathlen) < GSL_ROOT5_DBL_EPSILON ||
                   abs( simulationParameters.cp_angle - bfg->cp_angle) < GSL_ROOT5_DBL_EPSILON ||
                   simulationParameters.radial_resolution == bfg->simparms.radial_resolution ||
                   simulationParameters.temperature == bfg->simparms.temperature ||
                   pedata.scanData.last().seconds < bfg->dens_bfg_data.scanData.last().seconds){
                   // recalculation needed
                   bfg_offset = i;
                   break;
               }
           }
           if (bfg_offset == -1){
               US_Math_BF::Band_Forming_Gradient bfg = US_Math_BF::Band_Forming_Gradient(
                       simulationParameters.meniscus,
                       simulationParameters.bottom,
                       simulationParameters.band_volume,
                       dataSet->solution_rec.buffer.cosed_component,
                       simulationParameters.cp_pathlen,
                       simulationParameters.cp_angle);
              bfg.get_eigenvalues();
              bfg.calculate_gradient(simulationParameters, &pedata);
              data_sets_bfgs << bfg;
              bfg_offset = data_sets_bfgs.length() -1;
           }
       }
   }
DbgLv(1) << "sGA: calcsols size mxdpth" << calculated_solutes.size() << max_depth;

   simulation_values.solutes = calculated_solutes[ max_depth ];

int mm=simulation_values.solutes.size()-1;
DbgLv(1) << "sGA:   sol0.s solM.s" << simulation_values.solutes[0].s
 << simulation_values.solutes[mm].s << "  M=" << mm;
DbgLv(1) << "sGA:     solM.k" << simulation_values.solutes[mm].k;
DbgLv(1) << "sGA:     solM.c" << simulation_values.solutes[mm].c;
edata = &data_sets[ds_start]->run_data;
DbgLv(1) << "sGA:    edata scans points" << edata->scanCount() << edata->pointCount();

   calc_residuals( ds_start, ds_count, simulation_values, bfg_offset );

   sigmas.clear();
   res_data          = &simulation_values.residuals;
   int scnx          = 0;
DbgLv(1) << "sGA:  resids scans points" << res_data->scanCount() << res_data->pointCount();

   for ( int ee = ds_start; ee < ds_end; ee++ )
   {
      edata             = &data_sets[ ee ]->run_data;
      int scan_count    = edata->scanCount();
      int radius_points = edata->pointCount();
DbgLv(1) << "sGA:   ee" << ee << "scans points" << scan_count << radius_points;

      // Place the residuals magnitudes into a single vector for convenience
      for ( int ss = 0; ss < scan_count; ss++, scnx++ )
      {
         for ( int rr = 0; rr < radius_points; rr++ )
         {
            sigmas << qAbs( res_data->value( scnx, rr ) );
         }
      }
   }
int ssz=sigmas.size();
DbgLv(1) << "sGA:   sigmas size" << ssz << "sigmas[mm]" << sigmas[ssz/2];
}

// Reset for a refinement iteration
void US_MPI_Analysis::iterate( void )
{
   // Just return if the number of iterations exceeds the max
   // or if the last two iterations converged and are essentially identical
   if ( ++iterations > max_iterations )
   {
      qDebug() << "++ Maximum refinement iterations has been reached:" << max_iterations;
      return;
   }

   double diff  = simulation_values.variance - previous_values.variance;
   double adiff = qAbs( diff );
   bool   ssame = false;
   int hiters   = max_iterations / 2;

   if ( iterations > 2 )
   {
      if ( diff > 0.0   &&  iterations > hiters )
      {  // Variance is getting worse
         iterations  -= 2;
         simulation_values = previous_values;
         simulation_values.solutes   = previous_values.solutes;
         simulation_values.variances = previous_values.variances;
         simulation_values.ti_noise  = previous_values.ti_noise;
         simulation_values.ri_noise  = previous_values.ri_noise;
         qDebug() << "++ Refinement iteration degrading" << diff
                  << ". Reverting to iteration" << iterations;
         return;
      }

      if ( adiff < min_variance_improvement )
      {
         qDebug() << "++ Refinement iteration improvement near-zero:" << adiff;
         return;
      }

      int    nsols = previous_values.solutes.size();

      if ( nsols == simulation_values.solutes.size() )
      {
         ssame        = true;

         for ( int jj = 0; jj < nsols; jj++ )
         {
            if ( previous_values.solutes[ jj ] != simulation_values.solutes[ jj ] )
            {  // Mismatch:  may need to iterate
               ssame        = false;
               break;
            }

            else
            {  // Test if concentrations are almost the same
               double pconc = previous_values.solutes[ jj ].c;
               double cconc = simulation_values.solutes[ jj ].c;
               double cdiff = qAbs( ( pconc - cconc ) / pconc );

               if ( cdiff > 1.0e-1 )  // Greater than 10% concentration diff?
               {  // Significant concentration mismatch:  may need to iterate
                  ssame        = false;
                  break;
               }
            }
         }
      }
   }

   if ( ssame )
   { // Solutes same as previous:  no more iterations
      qDebug() << "++ Refinement iteration solutes same as previous:"
               << previous_values.solutes.size()
               << simulation_values.solutes.size();
      return;
   }

   // Save the most recent simulation values for the next time
   previous_values.variance  = simulation_values.variance;
   previous_values.solutes   = simulation_values.solutes;
   previous_values.variances = simulation_values.variances;
   previous_values.ti_noise  = simulation_values.ti_noise;
   previous_values.ri_noise  = simulation_values.ri_noise;
    int bfg_offset = -1;
    int csd_offset = -1;
    if (!data_sets[current_dataset]->solution_rec.buffer.cosed_component.isEmpty()) {
        US_SimulationParameters simulationParameters = data_sets[current_dataset]->simparams;
        US_DataIO::RawData edata = data_sets[current_dataset]->run_data.convert_to_raw_data();
        US_SolveSim::DataSet* dataSet = data_sets[current_dataset];
        // we need a
        for (int i = 0; i < data_sets_bfgs.length(); i++){
            US_Math_BF::Band_Forming_Gradient* bfg = &data_sets_bfgs[i];
            if (dataSet->solution_rec.buffer.cosed_component == bfg->cosed_component ||
                abs(simulationParameters.meniscus - bfg->meniscus) < GSL_ROOT5_DBL_EPSILON ||
                abs(simulationParameters.bottom - bfg->bottom) < GSL_ROOT5_DBL_EPSILON ||
                abs(simulationParameters.band_volume - bfg->overlay_volume) < GSL_ROOT5_DBL_EPSILON ||
                abs( simulationParameters.cp_pathlen - bfg->cp_pathlen) < GSL_ROOT5_DBL_EPSILON ||
                abs( simulationParameters.cp_angle - bfg->cp_angle) < GSL_ROOT5_DBL_EPSILON ||
                simulationParameters.radial_resolution == bfg->simparms.radial_resolution ||
                simulationParameters.temperature == bfg->simparms.temperature ||
                edata.scanData.last().seconds < bfg->dens_bfg_data.scanData.last().seconds){
                // recalculation needed
                bfg_offset = i;
                break;
            }
        }
        if (bfg_offset == -1){
            US_Math_BF::Band_Forming_Gradient bfg = US_Math_BF::Band_Forming_Gradient(
                    simulationParameters.meniscus,
                    simulationParameters.bottom,
                    simulationParameters.band_volume,
                    dataSet->solution_rec.buffer.cosed_component,
                    simulationParameters.cp_pathlen,
                    simulationParameters.cp_angle);
           bfg.get_eigenvalues();
           bfg.calculate_gradient(simulationParameters, &edata);
           data_sets_bfgs << bfg;
           bfg_offset = data_sets_bfgs.length() -1;
        }
    }
   // Set up for another round at depth 0
   Sa_Job job;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
   job.mpi_job.meniscus_value = meniscus_value;
   job.mpi_job.bottom_value   = bottom_value;
   job.mpi_job.bfg_offset     = bfg_offset;
   max_experiment_size        = min_experiment_size;

   QVector< US_Solute > prev_solutes = simulation_values.solutes;

   for ( int i = 0; i < orig_solutes.size(); i++ )
   {
      job.solutes = orig_solutes[ i ];

      // Add back all non-zero Solutes to each job
      // Ensure there are no duplicates
      for ( int s = 0; s < prev_solutes.size(); s++ )
      {
         if ( ! job.solutes.contains( prev_solutes[ s ] ) )
         {
            job.solutes << prev_solutes[ s ];
         }
      }

      job_queue << job;

      // Bump max solutes per subgrid to new observed max
      max_experiment_size = qMax( max_experiment_size,
                                  job.solutes.size() );
   }

   // Clear depth and calculated-solutes lists
   worker_depth.fill( 0 );
   max_depth = 0;
   for ( int ii = 0; ii < calculated_solutes.size(); ii++ )
      calculated_solutes[ ii ].clear();

   return;
}

// Submit a queued job
void US_MPI_Analysis::submit( Sa_Job& job, int worker )
{
   int bfg_offset = -1;
   int csd_offset = -1;
   if (!data_sets[current_dataset]->solution_rec.buffer.cosed_component.isEmpty() && data_sets[current_dataset]->simparams.meshType == US_SimulationParameters::ASTFVM) {
      US_SimulationParameters simulationParameters = data_sets[current_dataset]->simparams;
      US_DataIO::RawData edata = data_sets[current_dataset]->run_data.convert_to_raw_data();
      US_SolveSim::DataSet* dataSet = data_sets[current_dataset];
      // we need a
      for (int i = 0; i < data_sets_bfgs.length(); i++){
         US_Math_BF::Band_Forming_Gradient* bfg = &data_sets_bfgs[i];
         if (dataSet->solution_rec.buffer.cosed_component == bfg->cosed_component ||
             abs(simulationParameters.meniscus - bfg->meniscus) < GSL_ROOT5_DBL_EPSILON ||
             abs(simulationParameters.bottom - bfg->bottom) < GSL_ROOT5_DBL_EPSILON ||
             abs(simulationParameters.band_volume - bfg->overlay_volume) < GSL_ROOT5_DBL_EPSILON ||
             abs(simulationParameters.cp_pathlen - bfg->cp_pathlen) < GSL_ROOT5_DBL_EPSILON ||
             abs(simulationParameters.cp_angle - bfg->cp_angle) < GSL_ROOT5_DBL_EPSILON ||
             simulationParameters.radial_resolution == bfg->simparms.radial_resolution ||
             simulationParameters.temperature == bfg->simparms.temperature ||
             edata.scanData.last().seconds < bfg->dens_bfg_data.scanData.last().seconds){
            // recalculation needed
            bfg_offset = i;
            DbgLv(2) << "bfg found in position " << i;
            break;
         }
      }
      if (bfg_offset == -1){
         auto bfg =  new US_Math_BF::Band_Forming_Gradient(
                  simulationParameters.meniscus,
                  simulationParameters.bottom,
                  simulationParameters.band_volume,
                  dataSet->solution_rec.buffer.cosed_component,
                  simulationParameters.cp_pathlen,
                  simulationParameters.cp_angle);
         if ( !bfg-> upper_comps.isEmpty() ) {
            bfg->get_eigenvalues();
            bfg->calculate_gradient(simulationParameters, &edata);
            data_sets_bfgs << *bfg;
            bfg_offset = data_sets_bfgs.length() -1;
            DbgLv(2) << "bfg calculated and stored in position " << bfg_offset;
         }
      }
      DbgLv(1) << "bfg calculated and stored in position " << bfg_offset;
   }
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.solutes.size();
   job.mpi_job.meniscus_value = meniscus_value;
   job.mpi_job.bottom_value   = bottom_value;
   job.mpi_job.solution       = mc_iteration;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
   job.mpi_job.bfg_offset     = bfg_offset;
int dd=job.mpi_job.depth;
if (dd==0) { DbgLv(1) << "Mast: submit: worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
else { DbgLv(1) << "Mast: submit:     worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
DbgLv(1) << "Mast: submit: len sol offs cnt"
 << job.mpi_job.length
 << job.mpi_job.solution
 << job.mpi_job.dataset_offset
 << job.mpi_job.dataset_count;

   // Tell worker that solutes are coming
   MPI_Send( &job.mpi_job,
       sizeof( MPI_Job ),
       MPI_BYTE,
       worker,      // Send to system that needs work
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << "Mast: submit: send #1";

   // Send solutes
   MPI_Send( job.solutes.data(),
       job.mpi_job.length * solute_doubles,
       MPI_DOUBLE,   // Pass solute vector as hw independent values
       worker,       // to worker
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << "Mast: submit: send #2";
}

// Add a job to the queue, maintaining depth order
void US_MPI_Analysis::add_to_queue( Sa_Job& job )
{
   int jdepth = job.mpi_job.depth;

   for ( int qq = 0; qq < job_queue.size(); qq++ )
   {
      if ( jdepth < job_queue[ qq ].mpi_job.depth )
      { // Insert this job before any with a greater depth
         job_queue.insert( qq, job );
         return;
      }
   }

   // In most circumstances, we just append the job to the end of the queue
   job_queue << job;
   return;
}

// Process the results from a just-completed worker task
void US_MPI_Analysis::process_results( int        worker,
                                       const int* size )
{
   simulation_values.solutes.resize( size[ 0 ] );
   simulation_values.variances.resize( datasets_to_process );
   simulation_values.ti_noise.resize( size[ 1 ] );
   simulation_values.ri_noise.resize( size[ 2 ] );

   max_experiment_size = qMax( max_experiment_size, size[ 0 ] );

   MPI_Status status;

   // Get all simulation_values
   MPI_Recv( simulation_values.solutes.data(),
             size[ 0 ] * solute_doubles,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.variances.data(),
             datasets_to_process,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.ti_noise.data(),
             size[ 1 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   MPI_Recv( simulation_values.ri_noise.data(),
             size[ 2 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

   worker_status[ worker ] = INIT;
   int depth       = worker_depth[ worker ];

if (depth == 0) { DbgLv(1) << "Mast:  process_results: worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
else { DbgLv(1) << "Mast:  process_results:      worker" << worker
 << " solsize" << size[0] << "depth" << depth; }
    Result result;
    result.depth   = depth;
    result.worker  = worker;
    result.solutes = simulation_values.solutes;

    int lwdepth    = low_working_depth();

    // If there are no cached results and the job result's depth
    // is not beyond the low working depth, just process the result solutes
    if ( cached_results.size() == 0  &&  depth <= lwdepth )
    {
       process_solutes( depth, worker, result.solutes );
    }

    // If there are cached results or the job result's depth is below
    // the low working depth, then first cache the current result
    // in its proper depth-ordered place in the cached list
    else
    {
       cache_result( result );
    }

    // Process any previous results that were cached.
    // As long as there are cached depth-ordered results and the low on
    // the list is less than or equal to the low-working depth;
    // each first-on-the-list gets taken off and processed.

    while ( cached_results.size() > 0  &&
            cached_results[ 0 ].depth <= lwdepth )
    {
       result     = cached_results.takeFirst();
       depth      = result.depth;
       worker     = result.worker;

       process_solutes( depth, worker, result.solutes );
    }
}

// Process the calculated solute vector from a job result
void US_MPI_Analysis::process_solutes( int& depth, int& worker,
                                       QVector< US_Solute >& result_solutes )
{
DbgLv(1) << "Mast:    process_solutes:      worker" << worker
 << " solsize" << result_solutes.size() << "depth" << depth;
   int next_depth  = depth + 1;
   // This loop should only execute, at most, once per result.
   while ( calculated_solutes.size() < next_depth )
      calculated_solutes << QVector< US_Solute >();

   // How big will our vector be?
   int csol_size  = calculated_solutes[ depth ].size();
   int rsol_size  = result_solutes.size();
   int new_size   = csol_size + rsol_size;

   // Submit with previous solutes if new size would be too big
   if ( new_size > max_experiment_size )
   {
      // Put current solutes on queue at depth + 1
      Sa_Job job;
      job.solutes                = calculated_solutes[ depth ];
      job.mpi_job.depth          = next_depth;
      job.mpi_job.dataset_offset = current_dataset;
      job.mpi_job.dataset_count  = datasets_to_process;
      std::sort( job.solutes.begin(), job.solutes.end() );
      add_to_queue( job );

DbgLv(1) << "Mast:   queue NEW DEPTH sols" << job.solutes.size() << " d="
 << job.mpi_job.depth << " newsz mxesz" << new_size << max_experiment_size;
      max_depth           = qMax( next_depth, max_depth );
      calculated_solutes[ depth ].clear();
   }

   new_size            = rsol_size * 2;

   if ( next_depth > 1  &&  new_size > max_experiment_size )
   { // Adjust max_experiment_size if it is only large enough for one output
      max_experiment_size = ( new_size * 11 + 9 ) / 10;  // 10% above
DbgLv(1) << "Mast:    NEW max_exp_size" << max_experiment_size
 << "from new_size rsol_size" << new_size << rsol_size;
   }

   // Add in the current results
   calculated_solutes[ depth ] += result_solutes;
   csol_size  = calculated_solutes[ depth ].size();

   // At this point we need to clean up,  For each depth
   // below the current one, if there is nothing in the queue or working
   // for that depth or below and there are calculated solutes left, those
   // tasks need to be submitted.

   int dcheck = depth;
   if ( depth == 0  &&  max_depth > 0 )  dcheck = 1;

   for ( int d = 0; d < dcheck; d++ )
   {
      bool queued = false;
      for ( int q = 0; q < job_queue.size(); q++ )
      {
         if ( job_queue[ q ].mpi_job.depth <= d )
         {
            queued = true;
            break;
         }
      }

      bool working = false;
      for ( int w = 1; w <= my_workers; w++ )
      {
         if ( worker_depth[ w ] <= d  &&  worker_status[ w ] == WORKING )
         {
            working = true;
            break;
         }
      }

      int remainder = calculated_solutes[ d ].size();

      if ( ! working && ! queued && remainder > 0 )
      { // Submit a job with remaining calculated solutes from an earlier depth
         int next_d                 = d + 1;
         Sa_Job job;
         job.solutes                = calculated_solutes[ d ];
         job.mpi_job.depth          = next_d;
         job.mpi_job.dataset_offset = current_dataset;
         job.mpi_job.dataset_count  = datasets_to_process;
         max_depth                  = qMax( next_d, max_depth );
         std::sort( job.solutes.begin(), job.solutes.end() );
         add_to_queue( job );
DbgLv(1) << "Mast:   queue REMAINDER" << remainder << " d=" << d+1;

         calculated_solutes[ d ].clear();
      }
   }

   // Is anyone working?
   bool working = false;
   for ( int w = 1; w <= my_workers; w++ )
   {
      if ( worker_status[ w ] == WORKING )
      {
         working = true;
         break;
      }
   }

   // Submit one last time with all solutes if necessary.
   // This is the case if
   //  (1) this result is from the maximum submitted-jobs depth;
   //  (2) no jobs are queued or working; and
   //  (3) the current set of calculated solutes comes from more
   //      than one task result.
   if ( depth == max_depth     &&
        job_queue.isEmpty()    &&
        ! working              &&
        csol_size > rsol_size )
   {
      Sa_Job job;
      job.solutes          = calculated_solutes[ depth ];
      job.mpi_job.depth    = next_depth;
      meniscus_value       = ( meniscus_points == 1 )
                           ? data_sets[ current_dataset ]->run_data.meniscus
                           : meniscus_values[ meniscus_run ];
      bottom_value         = ( bottom_points == 1 )
                           ? data_sets[ current_dataset ]->run_data.bottom
                           : bottom_values  [ bottom_run   ];
      std::sort( job.solutes.begin(), job.solutes.end() );
DbgLv(1) << "Mast:   queue LAST ns=" << job.solutes.size() << "  d=" << depth+1
 << max_depth << "  nsvs=" << simulation_values.solutes.size();

      calculated_solutes[ depth ].clear();
      csol_size = 0;
      max_depth = next_depth;
      worker    = ready_worker();

      if ( worker > 0 )
      { // Submit what should be the last job of this iteration
         ljob_solutes            = job.solutes;
         submit( job, worker );
         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
         // Insure calculated solutes is empty for final depth
         if ( calculated_solutes.size() > max_depth )
            calculated_solutes[ max_depth ].clear();
         else
            calculated_solutes << QVector< US_Solute >();
      }
      else
      { // Shouldn't happen, but put job in queue if no worker is yet ready
DbgLv(1) << "Mast:   WARNING: LAST depth and no worker ready!";
         job_queue << job;
      }
   }

   // Force an abort if we are in a run-away situation
   if ( max_depth > 20 )
   {
      abort( "Max Depth is exceeding 20" );
      calculated_solutes[ depth + 20 ].clear();
   }

}

// Find the lowest depth among working jobs
int US_MPI_Analysis::low_working_depth( )
{
   int depth = 99;      // Default to a depth higher than any reasonable one

   for ( int ii = 1; ii <= my_workers; ii++ )
   { // Test all worker statuses and depths
      int wdepth = worker_depth[ ii ];

      if ( worker_status[ ii ] == WORKING  &&
           wdepth < depth )
      { // If working and low depth so far, save depth
         depth      = wdepth;
      }
   }

   return depth;
}

// Cache a job result in a depth-ordered list
void US_MPI_Analysis::cache_result( Result& result )
{
   int rdepth = result.depth;

   for ( int ii = 0; ii < cached_results.size(); ii++ )
   { // Examine all cached results
      int cdepth = cached_results[ ii ].depth;

      if ( rdepth < cdepth )
      { // Insert new result before next highest depth
         cached_results.insert( ii, result );
         return;
      }
   }

   // If no higher depth cached, append new result to the end
   cached_results << result;
   return;
}

