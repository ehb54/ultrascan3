#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_tar.h"
#include "us_astfem_rsa.h"
#include "us_simparms.h"
#include "us_constants.h"

#define DbgTime() qDebug() << "TM:" << (startTime.msecsTo(QDateTime::currentDateTime())/1000.0)
void US_MPI_Analysis::pcsa_master( void )
{
   current_dataset     = 0;
   datasets_to_process = count_datasets;
   US_DataIO::EditedData* edata;

DbgLv(1) << "pcsa_mast: IN";
   init_pcsa_solutes();
DbgLv(1) << "pcsa_mast: init complete";
   fill_pcsa_queue();
DbgLv(1) << "pcsa_mast: fill_pcsa_queue complete  qsize" << job_queue.size();

   work_rss.resize( gcores_count );

   alpha               = 0.0;
   mc_iterations       = 0;
   max_iterations      = parameters[ "gfit_iterations" ].toInt();
   int kcurve          = 0;

   while ( true )
   {
int kwsi=worker_status.count(INIT);
int kwsr=worker_status.count(READY);
int kwsw=worker_status.count(WORKING);
DbgLv(1) << "pcsa_mast:MLOOP - TOP  jqsize" << job_queue.size() << "wstat-K:IniRdyWrk" << kwsi << kwsr << kwsw;
      int worker;
      meniscus_value   = data_sets[ current_dataset ]->run_data.meniscus;
//if ( max_depth > 1 )
// DbgLv(1) << " master loop-TOP:  jq-empty?" << job_queue.isEmpty() << "   areReady?" << worker_status.contains(READY)
//    << "  areWorking?" << worker_status.contains(WORKING);

int jql=job_queue.size()-1;
      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {  // There are jobs in the queue and workers ready
         worker           = ready_worker();   // Get the next ready worker

         Sa_Job job              = job_queue.takeFirst();  // Grab next job
         job.mpi_job.depth       = kcurve++;               // Bump curve count
DbgLv(1) << "pcsa_mast:   submit_pcsa kc" << kcurve << "ready worker"
 << worker << "depth" << job.mpi_job.depth << "JQue size" << job_queue.size();

jql=job_queue.size()-1;
if(jql>=0) {
 DbgLv(1) << "pcsa_mast:MLOOP - (00)JQue size" << job_queue.size()
  << "JQ[n].zsol.siz" << job_queue[jql].zsolutes.size(); }
         submit_pcsa( job, worker );          // Submit a job to the worker
      }
jql=job_queue.size()-1;
if(jql>=0) {
 DbgLv(1) << "pcsa_mast:MLOOP - (01)JQue size" << job_queue.size()
  << "JQ[n].zsol.siz" << job_queue[jql].zsolutes.size();
} else {
 DbgLv(1) << "pcsa_mast:MLOOP - (01)JQue size" << job_queue.size();
}

      // All done with the pass if no jobs are queued or running
      if ( job_queue.isEmpty()  &&  ! worker_status.contains( WORKING ) ) 
      {  // No jobs left in the queue and no workers still busy
DbgLv(1) << "pcsa_mast:   END PASS mrecs size" << mrecs.size();
         kcurve           = 0;
         qSort( mrecs );                      // Sort curve model records
         edata            = &data_sets[ current_dataset ]->run_data;
         QString tripleID = edata->cell + edata->channel + edata->wavelength;

         simulation_values.variance    = mrecs[ 0 ].variance;
         simulation_values.zsolutes    = mrecs[ 0 ].csolutes;
         simulation_values.ti_noise    = mrecs[ 0 ].ti_noise;
         simulation_values.ri_noise    = mrecs[ 0 ].ri_noise;
         simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                         1 : 0;
         simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                         2 : 0;
//         simulation_values.dbg_level   = dbg_level;
         simulation_values.dbg_timing  = dbg_timing;
         simulation_values.solutes.clear();
//*DEBUG*
wksim_vals          = simulation_values;
wksim_vals.zsolutes = mrecs[ 0 ].isolutes;
DbgLv(1) << "final-mr0: enter CALC_RES" << current_dataset << datasets_to_process;
calc_residuals( current_dataset, datasets_to_process, wksim_vals );
DbgLv(1) << "final-mr0: variance1 variance2" << simulation_values.variance << wksim_vals.variance;
DbgLv(1) << "final-mr0: csolsize1 csolsize2" << simulation_values.zsolutes.size() << wksim_vals.zsolutes.size();
DbgLv(1) << "final-mr0: worker_status" << worker_status;
int ki=worker_status.count(INIT);
int kw=worker_status.count(WORKING);
int kr=worker_status.count(READY);
int km=mrecs.size();
int kg=0;
for(int jm=0; jm<km; jm++ ) if(mrecs[jm].rmsd<1.0) kg++;
DbgLv(1) << "final-mr0: ki kw kr" << ki << kw << kr << "km kg" << km << kg;
//*DEBUG*

         // Compose the progress message
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

         else
            progress     += "; RMSD: "
                            + QString::number( mrecs[ 0 ].rmsd );

         send_udp( progress );

         // Iterative refinement
         if ( max_iterations > 1 )
         {
DbgLv(1) << "pcsa_mast: iters" << iterations;
            if ( data_sets.size() > 1  &&  iterations == 1 )
            {
               if ( datasets_to_process == 1 )
               {
                  qDebug() << "   == Grid-Fit Iterations for Dataset"
                     << current_dataset + 1 << "==";
               }
               else
               {
                  qDebug() << "   == Grid-Fit Iterations for Datasets 1 to"
                     << datasets_to_process << "==";
               }
            }
            qDebug() << "Iteration:" << iterations
               << " Variance:" << mrecs[ 0 ].variance
               << "RMSD:" << mrecs[ 0 ].rmsd;

DbgLv(1) << "pcsa_mast:MLOOP - (02)JQue size" << job_queue.size();
            iterate_pcsa();
DbgLv(1) << "pcsa_mast:MLOOP - (03)JQue size" << job_queue.size();
         }
DbgLv(1) << "pcsa_mast:MLOOP - (04)JQue size" << job_queue.size();

         // Back to top of main loop if job queue has been refilled
         if ( ! job_queue.isEmpty() ) continue;

         // Primary grid-fit iterations are done
DbgLv(1) << "pcsa_mast: END of iters";
         iterations     = 1;
         max_iterations = 1;
DbgLv(1) << "pcsa_mast: loop-BOT: dssize" << data_sets.size() << "ds_to_p"
 << datasets_to_process << "curr_ds" << current_dataset;
US_DataIO::EditedData* edat=&data_sets[current_dataset]->run_data;
int ks=edat->scanCount() - 10;
int kp=edat->pointCount() - 10;
int ss=ks/2;
int rr=kp/2;
DbgLv(1) << "pcsa_mast: loop-BOT: ds" << current_dataset+1 << "data l m h"
 << edat->value(10,10) << edat->value(ss,rr) << edat->value(ks,kp);

         // Clean up mrecs of any empty-calculated-solutes records
         clean_mrecs( mrecs );

         // Manage multiple data sets in a global fit
         if ( is_global_fit )
         {
            mrec              = mrecs[ 0 ];
            simulation_values.zsolutes  = mrec.isolutes;
            simulation_values.alpha     = 0.0;
            simulation_values.noisflag  = 0;
//            simulation_values.dbg_level = dbg_level;
            simulation_values.solutes .clear();
            simulation_values.ti_noise.clear();
            simulation_values.ri_noise.clear();

            calc_residuals( current_dataset, datasets_to_process, simulation_values );
         }
DbgLv(1) << "pcsa_mast: loop-BOT: GF job_queue empty" << job_queue.isEmpty();

         if ( is_global_fit )
            write_global();          // Write global model

         else
            write_output();          // Write interim output (models, ...)

         // Save information from best model
         pcsa_best_model();
DbgLv(1) << "pcsa_mast: loop-BOT:   pcsa_best_model() complete";

         // Tikhonov Regularization
         tikreg_pcsa();
DbgLv(1) << "pcsa_mast: loop-BOT:   tikreg_pcsa() complete";

         // Monte Carlo
         montecarlo_pcsa();
DbgLv(1) << "pcsa_mast: loop-BOT:   montecarlo_pcsa() complete";

         // All Done
//         shutdown_all();  // All done
DbgLv(1) << "pcsa_mast: loop-BOT:     ALL DONE";
         break;           // Break out of main loop.
      }
jql=job_queue.size()-1;
if (jql>=0) {
 DbgLv(1) << "pcsa_mast:MLOOP - (05)JQue size" << job_queue.size()
  << "JQ[n].zsol.siz" << job_queue[jql].zsolutes.size();
} else {
 DbgLv(1) << "pcsa_mast:MLOOP - (05)JQue size" << job_queue.size();
}

      // Wait for worker to send a message
      MPI_Status status;
      int        sizes[ 4 ];

DbgLv(1) << "PM:Recv: 1:sizes" << 4;
      MPI_Recv( sizes, 
                4, 
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status );
DbgLv(1) << "PM:Recv:   sizes" << sizes[0] << sizes[1] << sizes[2] << sizes[3]
 << "statTAG" << status.MPI_TAG;

      worker                  = status.MPI_SOURCE;
jql=job_queue.size()-1;
if(jql>=0) {
 DbgLv(1) << "pcsa_mast:MLOOP - (06)JQue size" << job_queue.size()
  << "JQ[n].zsol.siz" << job_queue[jql].zsolutes.size();
}
DbgLv(1) << "pcsa_mast:MLOOP -  status TAG" << status.MPI_TAG
 << "  source" << status.MPI_SOURCE;

      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
DbgLv(1) << "pcsa_mast: loop-BOTTOM:  READY  worker" << worker;
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data
DbgLv(1) << "pcsa_mast: loop-BOTTOM:  RESULTS  worker" << worker << "sizes"
 << sizes[0] << sizes[1] << sizes[2] << sizes[3];
DbgLv(1) << "pcsa_mast:MLOOP - (07)JQue size" << job_queue.size();
            process_pcsa_results( worker, sizes );
DbgLv(1) << "pcsa_mast:MLOOP - (08)JQue size" << job_queue.size();
            work_rss[ worker ]      = sizes[ 3 ];
            break;

         default:  // Should never happen
DbgLv(1) << "pcsa_mast: loop-BOTTOM:   NOT READY/RESULTS TAG=" << status.MPI_TAG;
            QString msg =  "Master PCSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      }
jql=job_queue.size()-1;
if(jql>=0) {
 DbgLv(1) << "pcsa_mast:MLOOP - (09)JQue size" << job_queue.size()
  << "JQ[n].zsol.siz" << job_queue[jql].zsolutes.size();
}

      max_rss();
   }

   shutdown_all();  // All done
}

// Generate the initial set of solutes
void US_MPI_Analysis::init_pcsa_solutes( void )
{
DbgLv(1) << "  init_pcsa_sols: IN";
   calculated_zsolutes.clear();
   orig_zsolutes      .clear();
   simulation_values.noisflag    = parameters[ "tinoise_option" ].toInt() > 0 ?
                                   1 : 0;
   simulation_values.noisflag   += parameters[ "rinoise_option" ].toInt() > 0 ?
                                   2 : 0;
//   simulation_values.dbg_level   = dbg_level;
   simulation_values.dbg_timing  = dbg_timing;
DbgLv(0) << "DEBUG_LEVEL" << simulation_values.dbg_level;

   double x_min      = parameters[ "x_min"           ].toDouble();
   double x_max      = parameters[ "x_max"           ].toDouble();
   double y_min      = parameters[ "y_min"           ].toDouble();
   double y_max      = parameters[ "y_max"           ].toDouble();
   double zval       = parameters[ "z_value"         ].toDouble();
   QString s_ctyp    = parameters[ "curve_type" ];
   int    ctype      = US_ModelRecord::ctype_flag( s_ctyp );
   QString s_styp    = parameters[ "solute_type" ];
   int    stype      = US_ModelRecord::stype_flag( s_styp );
   int    nkpts      = parameters[ "vars_count"      ].toInt();
   int    nkpto      = sq( nkpts );
   int    nlpts      = parameters[ "curves_points"   ].toInt();
DbgLv(1) << "  init_pcsa_sols: nkpts" << nkpts << "nkpto" << nkpto;
   double vbar20     = data_sets[ current_dataset ]->vbar20;
   zval              = ( zval == 0.0 ) ? vbar20 : zval;
DbgLv(1) << "  init_pcsa_sols: currds" << current_dataset << "vbar" << vbar20;
   double *parlims   = (double*)pararry;
   parlims[ 0 ]      = -1.0;
   parlims[ 4 ]      = stype;
   parlims[ 5 ]      = zval;
   mrecs.clear();
DbgLv(1) << "  init_pcsa_sols: s_ctyp" << s_ctyp << "ctype" << ctype;

   if ( ctype == CTYPE_SL )
   {
      mrecs.reserve( nkpto );
DbgLv(1) << "  init_pcsa_sols: call compute_slines  nkpts" << nkpts;
      US_ModelRecord::compute_slines( x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "  init_pcsa_sols:  compute_slines: mrecs sz" << mrecs.size();
   }

   else if ( ctype == CTYPE_IS  ||  ctype == CTYPE_DS )
   {
      mrecs.reserve( nkpto );
DbgLv(1) << "  init_pcsa_sols: call compute_sigmoids  nkpts" << nkpts;
      US_ModelRecord::compute_sigmoids( ctype, x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "  init_pcsa_sols:  compute_sigmoids: mrecs sz" << mrecs.size();
   }

   else if ( ctype == CTYPE_HL )
   {
      nkpto             = nkpts;
      mrecs.reserve( nkpto );
      US_ModelRecord::compute_hlines( x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims, mrecs );
   }

   else if ( ctype == CTYPE_ALL )
   {
      nkpto            *= 3;
      int ctype1        = CTYPE_IS;
      int ctype2        = CTYPE_DS;
      double *parlims1  = parlims + 12;
      double *parlims2  = parlims + 24;
      parlims1[ 0 ]     = parlims[ 0 ];
      parlims1[ 4 ]     = parlims[ 4 ];
      parlims1[ 5 ]     = parlims[ 5 ];
      parlims2[ 0 ]     = parlims[ 0 ];
      parlims2[ 4 ]     = parlims[ 4 ];
      parlims2[ 5 ]     = parlims[ 5 ];
      mrecs.reserve( nkpto );

      US_ModelRecord::compute_slines( x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims,  mrecs );
      US_ModelRecord::compute_sigmoids( ctype1, x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims1, mrecs );
      US_ModelRecord::compute_sigmoids( ctype2, x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims2, mrecs );
   }

   else if ( ctype == CTYPE_2O )
   {
      nkpto            *= nkpts;
DbgLv(1) << "  init_pcsa_sols: nkpts" << nkpts << "nkpto" << nkpto;
      mrecs.reserve( nkpto );
      US_ModelRecord::compute_2ndorder( x_min, x_max, y_min, y_max,
            nkpts, nlpts, parlims, mrecs );
   }

   for ( int ii = 0; ii < mrecs.size(); ii++ )
   {
      orig_zsolutes << mrecs[ ii ].isolutes;
   }
}

// Submit a PCSA job
void US_MPI_Analysis::submit_pcsa( Sa_Job& job, int worker )
{
   job.mpi_job.command        = MPI_Job::PROCESS;
   job.mpi_job.length         = job.zsolutes.size(); 
   job.mpi_job.meniscus_value = 0.0;
   job.mpi_job.solution       = mc_iteration;
   job.mpi_job.dataset_offset = current_dataset;
   job.mpi_job.dataset_count  = datasets_to_process;
int dd=job.mpi_job.depth;
if (dd==0) { DbgLv(1) << " submit_pcsa: worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
else { DbgLv(1) << " submit_pcsa:     worker" << worker << "  sols"
 << job.mpi_job.length << "mciter cds" << mc_iteration << current_dataset << " depth" << dd; }
DbgLv(1) << "Mast: submit: len sol offs cnt" 
 << job.mpi_job.length
 << job.mpi_job.solution
 << job.mpi_job.dataset_offset
 << job.mpi_job.dataset_count;

   // Tell worker that solutes are coming
DbgLv(1) << " submit_pcsa-SEND PROCESS  worker" << worker
 << "isolsiz" << job.mpi_job.length << "depth" << job.mpi_job.depth;
   MPI_Send( &job.mpi_job, 
       sizeof( MPI_Job ), 
       MPI_BYTE,
       worker,      // Send to system that needs work
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << " submit_pcsa: send #1";

   // Send solutes
   MPI_Send( job.zsolutes.data(), 
       job.mpi_job.length * zsolut_doubles, 
       MPI_DOUBLE,   // Pass solute vector as hw independent values
       worker,       // to worker
       MPI_Job::MASTER,
       my_communicator );
DbgLv(1) << " submit_pcsa: send #2  worker" << worker
 << "depth" << job.mpi_job.depth << "stat=WORKING";

   worker_depth [ worker ] = job.mpi_job.depth;
   worker_status[ worker ] = WORKING;
}

// Process the results from a just-completed worker task
void US_MPI_Analysis::process_pcsa_results( const int worker, const int* sizes )
{
   simulation_values.zsolutes.resize( sizes[ 0 ] );
   simulation_values.variances.resize( datasets_to_process );
   simulation_values.ti_noise.resize( sizes[ 1 ] );
   simulation_values.ri_noise.resize( sizes[ 2 ] );

   MPI_Status status;

DbgLv(1) << "pcsa_mast:PPRes -   (01)JQue size" << job_queue.size();
   // Get all simulation_values
DbgLv(1) << "PM:Recv: 2:zsol" << sizes[0]*zsolut_doubles;
   MPI_Recv( simulation_values.zsolutes.data(),
             sizes[ 0 ] * zsolut_doubles,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

DbgLv(1) << "PM:Recv: 3:vari" << 1;
   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );
DbgLv(1) << "pcsa_mast:PPRes -   (03)JQue size" << job_queue.size();
   
DbgLv(1) << "PM:Recv: 4:varis" << datasets_to_process;
   MPI_Recv( simulation_values.variances.data(),
             datasets_to_process,
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );

DbgLv(1) << "PM:Recv: 5:tinoi" << sizes[1];
   MPI_Recv( simulation_values.ti_noise.data(),
             sizes[ 1 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );
DbgLv(1) << "pcsa_mast:PPRes -   (05)JQue size" << job_queue.size();

DbgLv(1) << "PM:Recv: 6:rinoi" << sizes[2];
   MPI_Recv( simulation_values.ri_noise.data(),
             sizes[ 2 ],
             MPI_DOUBLE,
             worker,
             MPI_Job::TAG0,
             my_communicator,
             &status );
DbgLv(1) << "pcsa_mast:PPRes -   (06)JQue size" << job_queue.size();

   Result result;
   result.depth    = worker_depth[ worker ];
   result.worker   = worker;
   result.zsolutes = simulation_values.zsolutes;

   // Process the result solutes
   process_pcsa_solutes( result );
DbgLv(1) << "pcsa_mast:PPRes -   (07)JQue size" << job_queue.size();
}
 
// Process the calculated solute vector from a job result
void US_MPI_Analysis::process_pcsa_solutes( Result& result )
{
   int jcurve     = result.depth;
   jcurve         = qMax( 0, qMin( ( mrecs.size() - 1 ), jcurve ) );
DbgLv(1) << "pcsa_mast: PPSol -   jcurve" << jcurve << "mrecs-size" << mrecs.size();
DbgLv(1) << "pcsa_mast: PPSol -   (01)JQue size" << job_queue.size();

   // Update the model record entry with calculated solutes and RMSD
   US_ModelRecord*
      cMr         = &mrecs[ jcurve ];
   cMr->csolutes  = result.zsolutes;
   cMr->variance  = simulation_values.variance;
   cMr->rmsd      = cMr->variance > 0.0 ? sqrt( cMr->variance ) : 0.0;
   int noiflg     = simulation_values.noisflag;
DbgLv(1) << "pcsa_mast: PPSol -   (03)JQue size" << job_queue.size();

   // Add in any noises
   if ( noiflg > 0 )
   {
      if ( ( noiflg & 1 ) > 0 )
         mrecs[ jcurve ].ti_noise = simulation_values.ti_noise;

      if ( ( noiflg & 2 ) > 0 )
         mrecs[ jcurve ].ri_noise = simulation_values.ri_noise;
   }
DbgLv(1) << "pcsa_mast: PPSol -   (09)JQue size" << job_queue.size();
}

// Write the model records file
void US_MPI_Analysis::write_mrecs()
{
   US_Model * modelP     = &data_sets[ current_dataset ]->model;
   US_DataIO::EditedData*
              edata      = &data_sets[ current_dataset ]->run_data;
   mrecs[ 0 ].modelGUID  = modelP->modelGUID;
DbgLv(1) << "pcsa:wrmr: currds" << current_dataset;
   mrecs[ 0 ].editGUID   = edata->editGUID;
   mrecs[ 0 ].mrecGUID   = US_Util::new_guid();
DbgLv(1) << "pcsa:wrmr:  editGUID=" << mrecs[0].editGUID
 << "modelGUID=" << mrecs[0].modelGUID;
   QString s_desc        = QString( modelP->description )
                           .replace( ".model", ".mrecs" );
   QString s_ctyp        = parameters[ "curve_type" ];
   int    ctype          = US_ModelRecord::ctype_flag( s_ctyp );
   QString s_styp        = parameters[ "solute_type" ];
   int    stype          = US_ModelRecord::stype_flag( s_styp );
   double x_min          = parameters[ "x_min"   ].toDouble();
   double x_max          = parameters[ "x_max"   ].toDouble();
   double y_min          = parameters[ "y_min"   ].toDouble();
   double y_max          = parameters[ "y_max"   ].toDouble();
   if ( ctype != CTYPE_ALL )
      mrecs[ 0 ].ctype      = ctype;
   mrecs[ 0 ].xmin       = x_min;
   mrecs[ 0 ].xmax       = x_max;
   mrecs[ 0 ].ymin       = y_min;
   mrecs[ 0 ].ymax       = y_max;
   QString fnameo        = shorter_filename( s_desc + ".xml" );
DbgLv(0) << "pcsa:wrmr:  fnameo=" << fnameo << "  fn length" << fnameo.size();
   QFile fileo( fnameo );

   if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QXmlStreamWriter xmlo( &fileo );

DbgLv(1) << "pcsa:wrmr: call wr_mr: s_desc" << s_desc;
      US_ModelRecord::write_modelrecs( xmlo, mrecs, s_desc, ctype,
                                       x_min, x_max, y_min, y_max, stype );
DbgLv(1) << "pcsa:wrmr:  rtn wr_mr";

      fileo.close();
   }

   // Add the file name of the mrecs file to the output list
   QFile filea( "analysis_files.txt" );

   if ( ! filea.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &filea );
   QString tripleID = edata->cell + edata->channel + edata->wavelength;
   int mc_iter = mc_iterations;
   int run     = ( mc_iter > 1 ) ? mc_iter : 1;

   if ( mc_iterations > 0 )
      run        = mc_iter;

   QString runstring = "Run: " + QString::number( run ) + " " + tripleID;

   tsout << fnameo
         << ";meniscus_value=" << meniscus_value
         << ";MC_iteration="   << mc_iter
         << ";variance="       << simulation_values.variance
         << ";run="            << runstring
         << "\n";
   filea.close();
DbgLv(1) << "pcsa:wrmr: wr-to-atxt fnameo=" << fnameo;
}

// Engineer gfit iterations for PCSA
void US_MPI_Analysis::iterate_pcsa()
{
   static double rmsd_last = 0.0;
   double rmsd_curr  = mrecs[ 0 ].rmsd;
   double thr_dr_rat = parameters[ "thr_deltr_ratio" ].toDouble();

DbgLv(1) << "iter_p: iter" << iterations << "of" << max_iterations;
   // If we have reached the maximum iterations, we are done
   if ( iterations >= max_iterations )  return;

   if ( iterations > 1 )
   {  // If after first iteration, check RMSD difference
      double dr_rat     = ( rmsd_last - rmsd_curr ) / rmsd_last;
DbgLv(1) << "iter_p: rmsd_c rmsd_l" << rmsd_curr << rmsd_last
 << "dr_rat thr_dr_rat" << dr_rat << thr_dr_rat;

      // If difference in iteration RMSD less than threshold, we are done
      if ( dr_rat < 0.0 )
      {  // Increase in RMSD
         qDebug() << "Increasing Last, Current RMSDs of"
                  << rmsd_last << rmsd_curr
                  << "truncate iterations at" << iterations;
         max_iterations = iterations;
         return;
      }

      else if ( dr_rat < thr_dr_rat )
      {  // Equal RMSD (within threshold ratio)
         qDebug() << "Virtually identical RMSDs of"
                  << rmsd_last << rmsd_curr
                  << "truncate iterations at" << iterations;
         max_iterations = iterations;
         return;
      }
   }

   // Set up to create a narrower set of model record curves
   double x_min      = parameters[ "x_min"      ].toDouble();
   double x_max      = parameters[ "x_max"      ].toDouble();
   double y_min      = parameters[ "y_min"      ].toDouble();
   double y_max      = parameters[ "y_max"      ].toDouble();
   double zval       = parameters[ "z_value"    ].toDouble();
   QString s_ctyp    = parameters[ "curve_type" ];
   int    ctype      = US_ModelRecord::ctype_flag( s_ctyp );
   QString s_styp    = parameters[ "solute_type" ];
   int    stype      = US_ModelRecord::stype_flag( s_styp );
   int    nkpts      = parameters[ "vars_count"      ].toInt();
   int    nlpts      = parameters[ "curves_points"   ].toInt();
   double vbar20     = data_sets[ current_dataset ]->vbar20;
   zval              = ( zval == 0.0 ) ? vbar20 : zval;
   double *parlims   = (double*)pararry;
   parlims[ 0 ]      = -1.0;
   parlims[ 4 ]      = stype;
   parlims[ 5 ]      = zval;
DbgLv(1) << "iter_p: ctype" << ctype << s_ctyp;

   // Recompute mrecs to cover the range of the elite (top 10%) records

   if ( ctype != CTYPE_ALL )
   {  // All non-combination records:  re-compute with narrower limits

      US_ModelRecord::recompute_mrecs( ctype, x_min, x_max, y_min, y_max,
                                       nkpts, nlpts, parlims, mrecs );
DbgLv(1) << "iter_p: parlims"
 << parlims[0] << parlims[1] << parlims[2] << parlims[3] << parlims[4]
 << parlims[5] << parlims[6] << parlims[7] << parlims[8] << parlims[9];
   }  // END: non-combo records

   else
   {  // Records that are a mix of SL,IS,DS
      QVector< US_ModelRecord > mrecs_sl;
      QVector< US_ModelRecord > mrecs_is;
      QVector< US_ModelRecord > mrecs_ds;
      double* plims_sl  = parlims;
      double* plims_is  = parlims + 12;
      double* plims_ds  = parlims + 24;

      // Separate entries by curve type
      filter_mrecs( CTYPE_SL, mrecs, mrecs_sl );
      filter_mrecs( CTYPE_IS, mrecs, mrecs_is );
      filter_mrecs( CTYPE_DS, mrecs, mrecs_ds );

      // Recompute each curve type with narrower limits
      ctype             = CTYPE_SL;
      US_ModelRecord::recompute_mrecs( ctype, x_min, x_max, y_min, y_max,
                                       nkpts, nlpts, plims_sl, mrecs_sl );

      ctype             = CTYPE_IS;
      US_ModelRecord::recompute_mrecs( ctype, x_min, x_max, y_min, y_max,
                                       nkpts, nlpts, plims_is, mrecs_is );

      ctype             = CTYPE_DS;
      US_ModelRecord::recompute_mrecs( ctype, x_min, x_max, y_min, y_max,
                                       nkpts, nlpts, plims_ds, mrecs_ds );

      // Re-combine into a single vector
      int kk            = 0;
      int ncurve        = sq( nkpts );
DbgLv(1) << "iter_p: ncurve" << ncurve;

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_sl[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_is[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      for ( int ii = 0; ii < ncurve; ii++, kk++ )
      {
         mrecs[ kk ]       = mrecs_ds[ ii ];
         mrecs[ kk ].taskx = kk;
      }

      ctype             = CTYPE_ALL;
      mrecs[0].v_ctype  = ctype;          // Vector curve type is "All"
      plims_is[ 5 ]     = parlims[ 5 ];   // Set z value for all types
      plims_ds[ 5 ]     = parlims[ 5 ];
DbgLv(1) << "iter_p: plims_sl"
 << plims_sl[0] << plims_sl[1] << plims_sl[2] << plims_sl[3] << plims_sl[4];
DbgLv(1) << "iter_p: plims_is"
 << plims_is[0] << plims_is[1] << plims_is[2] << plims_is[3] << plims_is[4];
DbgLv(1) << "iter_p: plims_ds"
 << plims_ds[0] << plims_ds[1] << plims_ds[2] << plims_ds[3] << plims_ds[4];
   }  // END:  records a mix of SL,IS,DS

   // Now reset original solutes and fill queue
   orig_zsolutes.clear();
   orig_zsolutes.reserve( mrecs.size() );

   for ( int ii = 0; ii < mrecs.size(); ii++ )
   {
      orig_zsolutes << mrecs[ ii ].isolutes;
   }

   fill_pcsa_queue();

   for ( int ii = 1; ii <= my_workers; ii++ )
      worker_status[ ii ] = READY;

   rmsd_last         = rmsd_curr;
   iterations++;
}

// Engineer Tikhonov Regularization for PCSA
void US_MPI_Analysis::tikreg_pcsa()
{
   int tikreg     = parameters[ "tikreg_option" ].toInt();
DbgLv(1) << "tikr: tikreg" << tikreg << "mrecs size" << mrecs.count();

   if ( tikreg == 0 )
      return;

   alpha          = ( tikreg == 1 )
                    ? parameters[ "tikreg_alpha" ].toDouble()
                    : alpha_scan();

   wksim_vals          = simulation_values;
   //wksim_vals.zsolutes = mrecs[ 0 ].csolutes;
   wksim_vals.zsolutes = mrecs[ 0 ].isolutes;
   wksim_vals.alpha    = alpha;
   wksim_vals.noisflag = 0;
//   wksim_vals.dbg_level= dbg_level;
//wksim_vals.dbg_level= 2;
   wksim_vals.solutes .clear();
   wksim_vals.ri_noise.clear();
   wksim_vals.ti_noise.clear();
DbgLv(1) << "tikr: alpha" << alpha << "   mrecs size" << mrecs.size();;
DbgLv(1) << "tikr:  currds" << current_dataset << "ds_to_p" << datasets_to_process;
if(dbg_level>0) sleep(10);

   calc_residuals( current_dataset, datasets_to_process, wksim_vals );

DbgLv(1) << "tikr: calc_residuals complete";
   mrec           = mrecs[ 1 ];
   mrec.csolutes  = wksim_vals.zsolutes;
   mrec.variance  = wksim_vals.variance;
   mrec.rmsd      = sqrt( mrec.variance );
   mrec.sim_data  = wksim_vals.sim_data;
   mrec.residuals = wksim_vals.residuals;
   mrecs[ 1 ]     = mrec;
DbgLv(1) << "tikr: ncsols" << mrec.csolutes.count();

   write_pcsa_aux_model( 0 );

   qDebug() << "Tikhonov Regularization RMSD" << mrec.rmsd;
DbgLv(1) << "tikr: RMSD" << mrec.rmsd << "csol size" << mrec.csolutes.size();

   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   QString tripleID  = edata->cell + edata->channel + edata->wavelength;
   QString progress  = "Regularization: "
                       + QString::number( wksim_vals.alpha );

   if ( datasets_to_process > 1 )
      progress     += "; Datasets: "
                      + QString::number( datasets_to_process );
   else
      progress     += "; Dataset: " + QString::number( current_dataset + 1 )
                      + " (" + tripleID + ") of "
                      + QString::number( count_datasets );

   progress     += "; RMSD: " + QString::number( mrec.rmsd );
DbgLv(1) << "tikr: progress: " << progress;

   send_udp( progress );
}

// Engineer Monte Carlo for PCSA
void US_MPI_Analysis::montecarlo_pcsa()
{
   if ( mc_iterations < 2 )  return;

   int worker;
   meniscus_value   = data_sets[ current_dataset ]->run_data.meniscus;
   mc_iterations    = parameters[ "mc_iterations" ].toInt();
   mc_iteration     = 1;
   mrec             = mrecs[ 0 ];
   int stype        = mrec.stype;
   US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
   QString tripleID = edata->cell + edata->channel + edata->wavelength;
   job_queue.clear();

   // Build composite simulation,residuals data array
   US_DataIO::RawData* sdata;
   US_DataIO::RawData* rdata;
   US_DataIO::Scan*    sscan;
   US_DataIO::Scan*    rscan;
   sdata         = &mrec.sim_data;
   rdata         = &mrec.residuals;
   int nscan     = sdata->scanCount();
   int npoint    = sdata->pointCount();
   int mcd_incr  = nscan * npoint;
   mcd_incr      = ( is_global_fit ) ? total_points : mcd_incr;
   int mcd_size  = mcd_incr * 2;
   int kci_send  = 0;
   int kci_recv  = 0;
   int ks        = 0;
   int kr        = mcd_incr;
   mc_data.resize( mcd_size );
DbgLv(1) << " masterMC  mc_data size" << mcd_size << "glob" << is_global_fit;

   if ( is_global_fit )
   {  // Global-fit:  grab sim+resids from all datasets
DbgLv(1) << " masterMC  GLOB mc_data sim+res  nscan" << nscan;
      for ( int ii = 0; ii < nscan; ii++ )
      {
         sscan         = &sdata->scanData[ ii ];
         rscan         = &rdata->scanData[ ii ];
         npoint        = sscan->rvalues.count();

         for ( int jj = 0; jj < npoint; jj++, ks++, kr++ )
         {
            mc_data[ ks ] = sscan->rvalues[ jj ];
            mc_data[ kr ] = rscan->rvalues[ jj ];
         }
      }
DbgLv(1) << " masterMC  GLOB mc_data  ks kr" << ks << kr;
   }

   else
   {  // Single dataset:  construct sim+resids flat array
DbgLv(1) << " masterMC  NON-glob mc_data sim+res  nscan" << nscan;
      for ( int ii = 0; ii < nscan; ii++ )
      {
         sscan         = &sdata->scanData[ ii ];
         rscan         = &rdata->scanData[ ii ];
         for ( int jj = 0; jj < npoint; jj++, ks++, kr++ )
         {
            mc_data[ ks ] = sscan->rvalues[ jj ];
            mc_data[ kr ] = rscan->rvalues[ jj ];
         }
      }
   }
DbgLv(1) << " masterMC SEND-NEWDATA my_workers" << my_workers;

   // Send simulation and residuals to the workers

   for ( worker = 1; worker <= my_workers; worker++ )
   {  // Tell each worker new data coming; worker expects a Send
      MPI_Job mjob;
      mjob.command        = MPI_Job::NEWDATA;
      mjob.length         = mcd_size;
      mjob.meniscus_value = meniscus_value;
      mjob.dataset_offset = current_dataset;
      mjob.dataset_count  = datasets_to_process;
      mjob.depth          = worker;
      mjob.solution       = 10000;

DbgLv(1) << " masterMC SEND-NEWDATA  worker" << worker;
      MPI_Send( &mjob,
                sizeof( MPI_Job ),
                MPI_BYTE,
                worker,
                MPI_Job::MASTER,
                my_communicator );
   }

   MPI_Barrier( my_communicator );       // Get everybody synced up

DbgLv(1) << " masterMC BCAST-NEWDATA  mcd_size" << mcd_size;
   MPI_Bcast( mc_data.data(),            // Send simulation,residuals data
              mcd_size,
              MPI_DOUBLE,
              MPI_Job::MASTER,
              my_communicator );

   // Fill the job queue and worker status
   for ( int ii = 1; ii <= mc_iterations; ii++ )
   {
      Sa_Job job;
      job.zsolutes                = mrec.isolutes;
      job.mpi_job.command         = MPI_Job::PROCESS_MC;
      job.mpi_job.length          = job.zsolutes.size();
      job.mpi_job.meniscus_value  = meniscus_value;
      job.mpi_job.dataset_offset  = current_dataset;
      job.mpi_job.dataset_count   = datasets_to_process;
      job.mpi_job.depth           = ii;
      job.mpi_job.solution        = ii;

      job_queue << job;
DbgLv(1) << " masterMC  fill job_queue  mciter" << ii;
   }

   for ( int ii = 1; ii <= my_workers; ii++ )
   {
      worker_depth [ ii ] = ii;
      worker_status[ ii ] = READY;
   }

   worknext         = 1;
   kci_send         = 0;

   // Loop to submit and handle MC iteration jobs

   while ( true )
   {
int kwsi=worker_status.count(INIT);
int kwsr=worker_status.count(READY);
int kwsw=worker_status.count(WORKING);
DbgLv(1) << " masterMC loop-TOP  jqsize" << job_queue.size() << "wstat-K:IniRdyWrk" << kwsi << kwsr << kwsw;
      int        mc_iter;
      int        mc_iters;
      int        sizes[ 4 ];
      QString    progress;
      MPI_Status status;

      // Give the jobs to the workers
      while ( ! job_queue.isEmpty()  &&  worker_status.contains( READY ) )
      {
         kci_send++;
         worker           = ready_worker();
DbgLv(1) << " masterMC loop-ready_worker" << worker;
         Sa_Job job       = job_queue.takeFirst();

         job.mpi_job.command   = MPI_Job::PROCESS_MC;
         job.mpi_job.depth     = kci_send;
         job.mpi_job.solution  = kci_send;

         // Tell worker that solutes are coming
DbgLv(1) << " masterMC loop-SEND PROCESS_MC  worker" << worker
 << "kci_send" << kci_send;
         MPI_Send( &job.mpi_job,
                   sizeof( MPI_Job ),
                   MPI_BYTE,
                   worker,
                   MPI_Job::MASTER,
                   my_communicator );

         // Send solutes
         MPI_Send( job.zsolutes.data(),
                   job.mpi_job.length * zsolut_doubles,
                   MPI_DOUBLE,
                   worker,
                   MPI_Job::MASTER,
                   my_communicator );

         worker_depth [ worker ] = job.mpi_job.depth;
         worker_status[ worker ] = WORKING;
      }

      // Wait for worker to send a message
DbgLv(1) << "PM:Recv: 7:sizes" << 4;
      MPI_Recv( sizes, 
                4, 
                MPI_INT,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                my_communicator,
                &status );

      worker           = status.MPI_SOURCE;

      if ( worker < 1  ||  worker > my_workers )
      {
         qDebug() << "*ERROR* MPI_Recv w/ source" << worker << ", beyond limit 1 to" << my_workers;
         QString msg =  "Master PCSA:  Received invalid source (worker): " +
                        QString::number( worker ) + ", beyond limit 1 to " +
                        QString::number( my_workers );
         abort( msg );
      }
DbgLv(1) << " masterMC loop-BOTTOM:   status TAG" << status.MPI_TAG
 << "  source" << status.MPI_SOURCE
 << "sizes" << sizes[0] << sizes[1] << sizes[2] << sizes[3];

      switch( status.MPI_TAG )
      {
         case MPI_Job::READY:   // Ready for work
DbgLv(1) << " masterMC loop-RECV READY  worker" << worker;
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS_MC: // Return solute data
            kci_recv++;
DbgLv(1) << " masterMC loop-RECV RESULTS_MC  worker" << worker << "kci_recv" << kci_recv;

            // Process PCSA MC results
            wksim_vals       = simulation_values;
            wksim_vals.zsolutes .resize( sizes[ 0 ] );
            wksim_vals.variances.resize( datasets_to_process );

DbgLv(1) << "PM:Recv: 8:zsols" << sizes[0]*zsolut_doubles;
            MPI_Recv( wksim_vals.zsolutes.data(),
                      sizes[ 0 ] * zsolut_doubles,
                      MPI_DOUBLE,
                      worker,
                      MPI_Job::TAG0,
                      my_communicator,
                      &status );

DbgLv(1) << "PM:Recv: 9:vari" << 1;
            MPI_Recv( &wksim_vals.variance,
                      1,
                      MPI_DOUBLE,
                      worker,
                      MPI_Job::TAG0,
                      my_communicator,
                      &status );
   
DbgLv(1) << "PM:Recv: 10:vairs" << datasets_to_process;
            MPI_Recv( wksim_vals.variances.data(),
                      datasets_to_process,
                      MPI_DOUBLE,
                      worker,
                      MPI_Job::TAG0,
                      my_communicator,
                      &status );

            mc_iter            = sizes[ 1 ];
            work_rss[ worker ] = sizes[ 3 ];

            // Output the temporary MC iteration model file
            mc_iters         = mc_iterations;
            mc_iterations++;

            write_pcsa_aux_model( mc_iter );

            mc_iterations    = mc_iters;
DbgLv(1) << " masterMC loop-RECV RESULTS_MC    mc_iter" << mc_iter << "of" << mc_iterations;

            // Send a status message
            if ( count_datasets > 1 )
            {
               if ( datasets_to_process == 1 )
               {
                  progress  = "Dataset: "
                             + QString::number( current_dataset + 1 )
                             + " (" + tripleID + ") of "
                             + QString::number( count_datasets );
                  if ( is_global_fit )
                  {
                     progress += "; (pre-global pass)";
                  }
                  else
                  {
                     progress += "; MonteCarlo: " + QString::number( mc_iter );
                  }
               }
               else
               {
                  progress  = "Datasets: "
                             + QString::number( datasets_to_process )
                             + "; MonteCarlo: " + QString::number( mc_iter );

               }
            }
            else
            {
               progress  = "Dataset: 1 of 1; MonteCarlo: "
                          + QString::number( mc_iter );
            }

            send_udp( progress );
            worker_status[ worker ] = READY;
            break;

         case MPI_Job::RESULTS: // Return solute data (unused worker?)
DbgLv(1) << " masterMC loop-RECV RESULTS  worker" << worker << "wstat" << worker_status[worker];
            {  // Receives just to consume messages
               QVector< double > dwork;
               int mxsiz     = sizes[0] * zsolut_doubles;
               mxsiz         = qMax( mxsiz, sizes[ 1 ] );
               mxsiz         = qMax( mxsiz, sizes[ 2 ] );
               mxsiz         = qMax( mxsiz, datasets_to_process );
               dwork.resize( mxsiz );
               double* wbuf  = dwork.data();
               MPI_Status status;

DbgLv(1) << "PM:Recv: 11:wbuf" << sizes[0]*zsolut_doubles << mxsiz;
               MPI_Recv( wbuf,
                         sizes[ 0 ] * zsolut_doubles,
                         MPI_DOUBLE,
                         worker,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );
DbgLv(1) << "PM:Recv: 12:wbuf" << 1;
               MPI_Recv( wbuf,
                         1,
                         MPI_DOUBLE,
                         worker,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );
DbgLv(1) << "PM:Recv: 13:wbuf" << datasets_to_process;
               MPI_Recv( wbuf,
                         datasets_to_process,
                         MPI_DOUBLE,
                         worker,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );
DbgLv(1) << "PM:Recv: 14:wbuf" << sizes[1];
               MPI_Recv( wbuf,
                         sizes[ 1 ],
                         MPI_DOUBLE,
                         worker,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );
DbgLv(1) << "PM:Recv: 15:wbuf" << sizes[2];
               MPI_Recv( wbuf,
                         sizes[ 2 ],
                         MPI_DOUBLE,
                         worker,
                         MPI_Job::TAG0,
                         my_communicator,
                         &status );
            }
            break;

         default:  // Should never happen
DbgLv(1) << " masterMC loop-RECV invalid  worker" << worker << "tag" << status.MPI_TAG;
            progress      = "Master PCSA:  Received invalid status "
                            + QString::number( status.MPI_TAG );
            abort( progress );
            break;
      }

      max_rss();
DbgLv(1) << " masterMC loop-BOTTOM";
      if ( kci_send > 0  &&  kci_recv >= kci_send )
         break;
   }
DbgLv(1) << " masterMC  loop-FALLTHRU mc_iter mc_iters kci_send kci_recv"
 << mc_iteration << mc_iterations << kci_send << kci_recv;

   // Update output with MC model
   mc_iteration      = mc_iterations;

   update_outputs();

   // Update the MC entry of the mrecs vector
   int mrx           = ( mrecs[ 2 ].taskx == mrecs[ 0 ].taskx ) ? 2 : 1;
   mrec              = mrecs[ mrx ];
   QString     mfilt = "*" + tripleID + "*.mcN*model.xml";
   QStringList mfltl( mfilt );
   QStringList mlist = QDir( "." ).entryList( mfltl, QDir::Files );
   QString mfile;
   int nmfile        = mlist.size();

   if ( nmfile == 1 )
   {
      mfile             = mlist[ 0 ]; 
   }
   else if ( nmfile > 1 )
   {
      mfile             = mlist[ 0 ]; 
      qDebug() << "*WARNING* More than 1" << mfilt << "file exists!";
      QDateTime mtime   = QFileInfo( mfile ).lastModified();

      for ( int jj = 1; jj < mlist.size(); jj++ )
      {  // Find the last of multiple files created
         QDateTime ftime   = QFileInfo( mlist[ 0 ] ).lastModified();
         if ( ftime > mtime )
         {
            mtime          = ftime;
            mfile          = mlist[ 0 ];
         }
      }
   }
   else
   {
      qDebug() << "*ERROR* No" << mfilt << "file exists!";
      return;
   }

   wmodel.load( mfile );
   mrec.csolutes.resize( wmodel.components.size() );

   for ( int jj = 0; jj < wmodel.components.size(); jj++ )
   {
      US_ZSolute::set_solute_values( wmodel.components[ jj ],
                                     mrec.csolutes[ jj ], stype );
   }

   mrecs[ mrx ]      = mrec;

   // Write the updated mrecs
   write_mrecs();
}

// Filter model records by a specified curve type
void US_MPI_Analysis::filter_mrecs( const int ctype,
      QVector< US_ModelRecord >& mrecs_a, QVector< US_ModelRecord >& mrecs_t )
{
   int namrec        = mrecs_a.size();
   int ntmrec        = namrec / 3;
   mrecs_t.clear();
   mrecs_t.reserve( ntmrec );

   for ( int ii = 0; ii < namrec; ii++ )
      if ( mrecs_a[ ii ].ctype == ctype )
         mrecs_t << mrecs_a[ ii ];
}

// Clean up model records to handle records with empty calculated solutes
void US_MPI_Analysis::clean_mrecs( QVector< US_ModelRecord >& mrecs )
{
   // First get index to last good
   int nmrec         = mrecs.size();
   int lmrec         = nmrec - 1;
   int goodx         = 0;

   for ( int ii = lmrec; ii > 1; ii-- )
   {
      if ( mrecs[ ii ].csolutes.size() != 0 )
      {
         goodx             = ii;
         break;
      }
   }

   // If last overall is good, return with no change to records
   if ( goodx == lmrec )
      return;

DbgLv(1) << " master:clean_mrecs: goodx lmrec" << goodx << lmrec;
   if ( mrecs[ 0 ].v_ctype != CTYPE_ALL )
   {  // If records of one type, just duplicate solutes from last good
      for ( int ii = goodx + 1; ii < nmrec; ii++ )
      {
         mrecs[ ii ].csolutes = mrecs[ goodx ].csolutes;
         mrecs[ ii ].rmsd     = mrecs[ goodx ].rmsd;
      }
   }

   else
   {  // If mixed records, duplicate solutes from last of each type
      int gx_sl         = 0;
      int gx_is         = 0;
      int gx_ds         = 0;
      int ktype         = 0;

      // Get indexes of last-good for each type
      for ( int ii = goodx; ii > 1; ii-- )
      {
         if ( mrecs[ ii ].ctype == CTYPE_SL )
         {
            if ( gx_sl > 0 )   continue;
            gx_sl             = ii;
            ktype++;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_IS )
         {
            if ( gx_is > 0 )   continue;
            gx_is             = ii;
            ktype++;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_DS )
         {
            if ( gx_ds > 0 )   continue;
            gx_ds             = ii;
            ktype++;
         }

         if ( ktype >= 3 )  break;
      }

      // Then replace each type with last good of that type
      for ( int ii = goodx + 1; ii < nmrec; ii++ )
      {
         if ( mrecs[ ii ].ctype == CTYPE_SL )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_sl ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_sl ].rmsd;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_IS )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_is ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_is ].rmsd;
         }
         else if ( mrecs[ ii ].ctype == CTYPE_DS )
         {
            mrecs[ ii ].csolutes = mrecs[ gx_ds ].csolutes;
            mrecs[ ii ].rmsd     = mrecs[ gx_ds ].rmsd;
         }
      }
   }
}

// Save best pre-regularization/pre-montecarlo model and model record;
//  and insert place-holders for any regularization and/or montecarlo
void US_MPI_Analysis::pcsa_best_model()
{
   US_Model mdummy;
   mrec              = mrecs[ 0 ];
//   simulation_values.dbg_level = dbg_level;
   simulation_values.alpha     = 0.0;
   simulation_values.noisflag  = 0;
   simulation_values.zsolutes  = mrec.csolutes;
   simulation_values.solutes .clear();
   simulation_values.ti_noise.clear();
   simulation_values.ri_noise.clear();
   simulation_values.variance  = mrec.variance;

DbgLv(1) << "bestm: CALC_RESID call" << current_dataset << datasets_to_process
 << "zsols size" << simulation_values.zsolutes.size() << "variance" << mrec.variance;
   calc_residuals( current_dataset, datasets_to_process, simulation_values );
DbgLv(1) << "bestm: CALC_RESID return -- variance" << simulation_values.variance;

   mrec.csolutes     = simulation_values.zsolutes;
   mrec.variance     = simulation_values.variance;
   mrec.rmsd         = sqrt( mrec.variance );
   mrec.model        = data_sets[ current_dataset ]->model;
   mrec.sim_data     = simulation_values.sim_data;
   mrec.residuals    = simulation_values.residuals;
   mrec.ti_noise     = simulation_values.ti_noise;
   mrec.ri_noise     = simulation_values.ri_noise;
DbgLv(1) << "bestm: RMSD" << mrec.rmsd;

   mrecs[ 0 ]        = mrec;

   int tikreg        = parameters[ "tikreg_option" ].toInt();
   mc_iterations     = parameters[ "mc_iterations" ].toInt();
   int noiflg        = simulation_values.noisflag;
DbgLv(1) << "bestm: tikreg mciters" << tikreg << mc_iterations;

   if ( tikreg != 0 )
   {
      int nadd          = ( mc_iterations > 1 ) ? 2 : 1;
      mrecs.insert( 1, nadd, mrec );

      mrecs[ 1 ].model    = mdummy;
      mrecs[ 1 ].csolutes .clear();
      mrecs[ 1 ].ti_noise .clear();
      mrecs[ 1 ].ri_noise .clear();
      mrecs[ 1 ].mrecGUID .clear();
      mrecs[ 1 ].mrecGUID .clear();
      mrecs[ 1 ].modelGUID.clear();

      if ( nadd > 1 )
      {
         mrecs[ 2 ].model    = mdummy;
         mrecs[ 2 ].csolutes .clear();
         mrecs[ 2 ].ti_noise .clear();
         mrecs[ 2 ].ri_noise .clear();
         mrecs[ 2 ].mrecGUID .clear();
         mrecs[ 2 ].mrecGUID .clear();
         mrecs[ 2 ].modelGUID.clear();
      }
   }

   else if ( mc_iterations > 1 )
   {
      mrecs.insert( 1, 1, mrec );
      mrecs[ 1 ].model    = mdummy;
      mrecs[ 1 ].csolutes .clear();
      mrecs[ 1 ].ti_noise .clear();
      mrecs[ 1 ].ri_noise .clear();
      mrecs[ 1 ].mrecGUID .clear();
      mrecs[ 1 ].mrecGUID .clear();
      mrecs[ 1 ].modelGUID.clear();
   }

   if ( ( tikreg != 0  ||  mc_iterations > 1 )  &&
        ( noiflg != 0 ) )
   {  // Apply computed noise to data
      US_DataIO::EditedData* edata = &data_sets[ current_dataset ]->run_data;
      double vnoise     = 0.0;
      int nscan         = edata->scanCount();
      int npoint        = edata->pointCount();
      int rcount        = mrec.ti_noise.size();
      int scount        = mrec.ri_noise.size();
DbgLv(1) << "bestm: noise apply ns np sc rc" << nscan << npoint << scount << rcount;
      npoint            = ( noiflg & 1 ) > 0 ? npoint : rcount;
      nscan             = ( noiflg & 2 ) > 0 ? nscan  : scount;

      if ( rcount != npoint  ||  scount != nscan )
      {
         qDebug() << "*ERROR* noise count(s) do not match data dimensions!";
         return;
      }

      if ( ( noiflg & 1 ) > 0 )
      {  // Apply ti noise
DbgLv(1) << "bestm: noise flag" << noiflg << "TI_NOISE apply";
         for ( int jj = 0; jj < rcount; jj++ )
         {  // Get constant noise value for each reading and apply to scans
            vnoise            = -1.0 * mrec.ti_noise[ jj ];
            for ( int ii = 0; ii < scount; ii++ )
            {  // Apply to all scans at reading position
               edata->scanData[ ii ].rvalues[ jj ] += vnoise;
            }
         }
      }

      if ( ( noiflg & 2 ) > 0 )
      {  // Apply ri noise
DbgLv(1) << "bestm: noise flag" << noiflg << "RI_NOISE apply";
         for ( int ii = 0; ii < scount; ii++ )
         {  // Get constant noise value for each scan and apply to readings
            vnoise            = -1.0 * mrec.ri_noise[ ii ];
            for ( int jj = 0; jj < rcount; jj++ )
            {  // Apply to all readings at scan position
               edata->scanData[ ii ].rvalues[ jj ] += vnoise;
            }
         }
      }
   }
}

// Write a PCSA auxiliary (TR/MC) model and potentially model records
void US_MPI_Analysis::write_pcsa_aux_model( int iter )
{
   wmodel              = mrecs[ 0 ].model;
   wmodel.modelGUID    = US_Util::new_guid();
   wmodel.variance     = wksim_vals.variance;
   QString mdesc       = wmodel.description;
   QString runID       = QString( mdesc ).section( ".",  0, -4 );
   QString tripID      = QString( mdesc ).section( ".", -3, -3 );
   QString asysID      = QString( mdesc ).section( ".", -2, -2 );
   QString typeExt     = QString( ".model" );
   QString dates       = QString( asysID ).section( "_", 0, 1 );
   QString atype       = QString( asysID ).section( "_", 2, 2 );
   QString reqID       = QString( asysID ).section( "_", 3, 3 );
   QString iterID      = "i01" ;
DbgLv(1) << "wraux: mdesc" << mdesc;

   if ( is_global_fit )
   {  // For Global Fit, de-scale solute signal concentrations
      double avg_conc     = 0.0;

      for ( int ee = 0; ee < data_sets.size(); ee++ )
      {  // Get average total concentration of data sets
         avg_conc           += concentrations[ ee ];
      }

      avg_conc           /= (double)( data_sets.size() );
      avg_conc            = ( avg_conc == 0.0 ) ? 1.0 : avg_conc;

      for ( int ii = 0; ii < wksim_vals.zsolutes.size(); ii++ )
      {  // Scale concentration of solute points
         wksim_vals.zsolutes[ ii ].c *= avg_conc;
      }
DbgLv(1) << "wraux:   avg_conc" << avg_conc << "nsols" << wksim_vals.zsolutes.size();
   }

   if ( iter == 0 )
   {
      atype              += "-TR";
      wmodel.alphaRP      = alpha;
DbgLv(1) << "wraux:      TR: mrecs size" << mrecs.size();
      mrecs[ 1 ].modelGUID = wmodel.modelGUID;
   }
   else
   {
      atype              += "-MC";
      wmodel.monteCarlo   = true;
DbgLv(1) << "wraux:      MC: mrecs size" << mrecs.size();
      iterID              = QString().sprintf( "mc%04d", iter );
      int jj              = ( mrecs[ 2 ].taskx == mrecs[ 0 ].taskx ) ? 2 : 1;
      mrecs[ jj ].modelGUID = wmodel.modelGUID;
   }
   asysID              = dates + "_" + atype + "_" + reqID + "_" + iterID;
   mdesc               = runID + "." + tripID + "." + asysID + typeExt;
   wmodel.description  = mdesc;
   wmodel.components.clear();
DbgLv(1) << "wraux: mdesc" << mdesc;
   QString s_styp      = parameters[ "solute_type" ];
   int    stype        = US_ModelRecord::stype_flag( s_styp );

   for ( int ii = 0; ii < wksim_vals.zsolutes.size(); ii++ )
   {
      US_Model::SimulationComponent component;
      US_ZSolute::set_mcomp_values( component, wksim_vals.zsolutes[ ii ],
                                    stype, true );
      component.name      = QString().sprintf( "SC%04d", ii + 1 );

      US_Model::calc_coefficients( component );

      wmodel.components << component;
   }

   mrec.model          = wmodel;
   QString fext        = ( iter == 0 ) ? ".model.xml" : ".mdl.tmp";
   QString fileid      = "." + atype + "." + tripID + "." + iterID + fext;
   QString fn          = runID + fileid;
   int lenfn           = fn.length();

   if ( lenfn > 96 )
   {
      int lenri           = runID.length() + 96 - lenfn;
      fn                  = QString( runID ).left( lenri ) + fileid;
   }

   // Output the model to a file
DbgLv(1) << "wraux:  model write";
   wmodel.write( fn );

   // Add the model file name to the output list
   QFile fileo( "analysis_files.txt" );

   if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text
                                           | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream tsout( &fileo );

   tsout << fn
         << ";meniscus_value=" << meniscus_value
         << ";MC_iteration="   << iter
         << ";variance="       << wmodel.variance
         << ";run=Run: 1 "     << tripID << "\n";

    fileo.close();

    // If final regularization or final MC iter model, write model records
    if ( ( iter == 0  &&  mc_iterations < 2 )  ||
         ( iter > 0   &&  iter >= mc_iterations ) )
    {
DbgLv(1) << "wraux:  final mrecs write";
       write_mrecs();
    }
}

// Scan alphas
double US_MPI_Analysis::alpha_scan()
{
   const double salpha   = 0.10;
   const double ealpha   = 0.90;
   const double dalpha   = 0.01;
   const double roundv   = dalpha * 0.05;
   const double alphadef = salpha + ( ealpha - salpha ) * 0.25; 
   int nalpha    = qRound( ( ealpha - salpha ) / dalpha ) + 1;
   QVector< double >  alphas;
   QVector< double >  varias;
   QVector< double >  xnorms;
   QVector< double >  sv_nnls_a;
   QVector< double >  sv_nnls_b;
   alphas.clear();
   varias.clear();
   xnorms.clear();
   alphas.reserve( nalpha );
   varias.reserve( nalpha );
   xnorms.reserve( nalpha );

   double varmx  = 0.0;
   double xnomx  = 0.0;
   double v_vari = 0.0;
   double v_xnsq = 0.0;
   double calpha = salpha;

   for ( int ja = 0; ja < nalpha; ja++ )
   {  // Loop to populate alpha scan arrays with preliminary values
      alphas << calpha;
      varias << v_vari;
      xnorms << v_xnsq;
      calpha += dalpha;
   }

   // Do alpha scan
   QVector< double > csolutes;
   sv_nnls_a.clear();
   sv_nnls_b.clear();
   mrec          = mrecs[ 0 ];
   int nscans    = data_sets[ current_dataset ]->run_data.scanCount();
   int npoints   = data_sets[ current_dataset ]->run_data.pointCount();
   int nisols    = mrec.isolutes.size();

   for ( int ja = 0; ja < nalpha; ja++ )
   {  // Loop to evaluate each alpha in the specified range
      calpha             = alphas[ ja ];

      if ( ja == 0 )
      {  // For the first one do a full compute and save the A,B matrices
         US_SolveSim::Simulation sim_vals;
         sim_vals.alpha     = calpha;
         sim_vals.zsolutes  = mrec.isolutes;

         US_SolveSim* solvesim = new US_SolveSim( data_sets, 0, false );

         solvesim->calc_residuals( current_dataset, datasets_to_process,
               sim_vals, true, &sv_nnls_a, &sv_nnls_b );

         v_vari             = sim_vals.variance;
         v_xnsq             = sim_vals.xnormsq;
      }

      else
      {  // After 1st, regularize by modifying A matrix diagonal, then NNLS
         apply_alpha( calpha, &sv_nnls_a, &sv_nnls_b,
                      nscans, npoints, nisols, v_vari, v_xnsq );
      }

      varias[ ja ]       = v_vari;
      xnorms[ ja ]       = v_xnsq;
      varmx              = qMax( varmx, v_vari );
      xnomx              = qMax( xnomx, v_xnsq );
DbgLv(1) << "a v x" << calpha << v_vari << v_xnsq;
   }

   sv_nnls_a.clear();
   sv_nnls_b.clear();

   int lgv        = 0  - (int)qFloor( log10( varmx ) );
   int lgx        = -1 - (int)qFloor( log10( xnomx ) );
   double vscl    = qPow( 10.0, lgv );
   double xscl    = qPow( 10.0, lgx );
DbgLv(1) << "Log-varia Log-xnorm" << lgv << lgx << "vscl xscl" << vscl << xscl;

   for ( int ja = 0; ja < nalpha; ja++ )
   {  // Scale the variance,normsq points
      varias[ ja ] *= vscl;
      xnorms[ ja ] *= xscl;
   }

   // Compute lines that hint at the elbow point of the curve
   double* xx    = varias.data();
   double* yy    = xnorms.data();
   double  xa[ 5 ];
   double  ya[ 5 ];
   double* xe    = (double*)xa;
   double* ye    = (double*)ya;
   double  slope; double slop2;
   double  intcp; double intc2;
   double  sigma; double  corre;
   double  xlipt; double ylipt; double xcipt; double ycipt;
   int  nlp  = 5;

   // Compute a line fitted to the first few main curve points
   while ( nlp < nalpha )
   {
      double avg  = US_Math2::linefit( &xx, &yy, &slope, &intcp,
                                       &sigma, &corre, nlp );
DbgLv(1) << "ASCN:H1:  avg" << avg << "nlp" << nlp;
DbgLv(1) << "ASCN:H1:   sl" << slope << "in" << intcp << "sg" << sigma
   << "co" << corre;
      if ( slope < 1e99  &&  slope > (-1e99) )
         break;

      nlp      += 2;
   }

   // Compute a line fitted to the last few main curve points
   int je    = nalpha - 1;
   for ( int jj = 0; jj < 5; jj++, je-- )
   {
      xe[ jj ]  = xx[ je ];
      ye[ jj ]  = yy[ je ];
   }

   US_Math2::linefit( &xe, &ye, &slop2, &intc2, &sigma, &corre, 5 );

   // Find the intersection point for the 2 fitted lines
   US_Math2::intersect( slope, intcp, slop2, intc2, &xlipt, &ylipt );

   // Find the curve point nearest to the intersection point;
   //  then compute a line from intersection to nearest curve point.
   US_Math2::nearest_curve_point( xx, yy, nalpha, true, xlipt, ylipt,
         &xcipt, &ycipt, alphas.data(), &calpha );

   // Do a sanity check. If the intersection point is outside the
   // rectangle that encloses the curve, we likely have an aberrant curve.
   // So, forget elbow fit and default alpha.
   double xcvp1  = xx[ 0 ];
   double ycvp1  = yy[ 0 ];
   double xcvp2  = xx[ je ];
   double ycvp2  = yy[ je ];
   bool good_fit = ( xlipt >= xcvp1  &&  ylipt <= ycvp1  &&
                     xlipt <= xcvp2  &&  ylipt >= ycvp2 );
DbgLv(1) << "ASCN:T4:   cv: x1,y1" << xcvp1 << ycvp1
 << "x2,y2" << xcvp2 << ycvp2 << " good_fit" << good_fit;

   calpha        = (double)qRound( calpha / roundv ) * roundv;
   calpha        = good_fit ? calpha : alphadef;

   return calpha;
}

void US_MPI_Analysis::apply_alpha( const double alpha, QVector< double >* psv_nnls_a,
      QVector< double >* psv_nnls_b, const int nscans, const int npoints,
      const int nisols, double& variance, double& xnormsq )
{
   int ntotal   = is_global_fit ? total_points : ( nscans * npoints );
   int narows   = ntotal + nisols;
   int ncsols   = 0;
       variance = 0.0;
       xnormsq  = 0.0;
   QVector< double > nnls_a = *psv_nnls_a;
   QVector< double > nnls_b = *psv_nnls_b;
   QVector< double > nnls_x;
   QVector< double > simdat;
   nnls_x.fill( 0.0, nisols );
   simdat.fill( 0.0, ntotal );

   // Replace alpha in the diagonal of the lower square of A
   int dx       = ntotal;
   int dinc     = ntotal + nisols + 1;

   for ( int cc = 0; cc < nisols; cc++ )
   {
      nnls_a[ dx ] = alpha;
      dx          += dinc;
   }

   // Compute the X vector using NNLS
   US_Math2::nnls( nnls_a.data(), narows, narows, nisols,
                   nnls_b.data(), nnls_x.data() );

   // Construct the output solutes and the implied simulation and xnorm-sq
   for ( int cc = 0; cc < nisols; cc++ )
   {
      double soluval  = nnls_x[ cc ];   // Computed concentration, this solute

      if ( soluval > 0.0 )
      {
         xnormsq     += sq( soluval );
         ncsols++;
         int aa       = cc * narows;

         for ( int kk = 0; kk < ntotal; kk++ )
         {
            simdat[ kk ]   += ( soluval * (*psv_nnls_a)[ aa++ ] );
         }
      }
   }

   // Calculate the sum for the variance computation
   for ( int kk = 0; kk < ntotal; kk++ )
   {
      variance    += sq( ( (*psv_nnls_b)[ kk ] - simdat[ kk ] ) );
   }

   // Return computed variance and xnorm-sq
   variance    /= (double)ntotal;
}

// Fill the job queue, using the list of initial solutes
void US_MPI_Analysis::fill_pcsa_queue( void )
{
   worker_status.resize( gcores_count );
   worker_depth .resize( gcores_count );

   worker_status.fill( INIT );
   worker_depth .fill( 0 );
   max_depth           = 0;
   worknext            = 1;
   max_experiment_size = min_experiment_size;

   // Put all jobs in the queue
   job_queue.clear();

   for ( int ii = 0; ii < orig_zsolutes.size(); ii++ )
   {
      Sa_Job job;
      job.zsolutes        = orig_zsolutes[ ii ];
      job_queue << job;

      max_experiment_size = qMax( max_experiment_size,
                                  job.zsolutes.size() );
   }
}

// Insure a filename does not exceed 99 characters in length
QString US_MPI_Analysis::shorter_filename( const QString fnamei )
{
   QString fnameo     = fnamei;
   int fnlen          = fnameo.size();

   if ( fnlen > 99 )
   {  // Name too long:  break it down and shorten parts where possible
      QStringList parts  = fnamei.split( "." );
      int nparts         = parts.count();
      int nchrmv         = fnlen - 99;
      int lnpm1          = parts[ nparts - 1 ].length();
      int lnpm2          = parts[ nparts - 2 ].length();

      if ( parts[ nparts - 2 ] == "mrecs"  &&  nchrmv < 11 )
      {  // If mrecs name, try to shorten analysis description part
         QString anapart    = parts[ nparts - 3 ];
         QString anap0      = anapart.section( "_", 0, 0 );
         QString anap1      = anapart.section( "_", 1, 1 );
         QString anap2      = anapart.section( "_", 2, 2 );
         QString anap3      = anapart.section( "_", 3, 3 );
         QString anap4      = anapart.section( "_", 4, 4 );
         
         if ( nchrmv < 9 )
         {  // Remove hhmm part of edit and analysis parts (- 8 chars.)
            anapart            = anap0.left( 7 ) + "_"
                               + anap1.left( 7 ) + "_"
                               + anap2 + "_" + anap3 + "_" + anap4;
         }

         else
         {  // Remove the edit part of the analysis section (- 11 chars.)
            anapart            = anap1 + "_"
                               + anap2 + "_" + anap3 + "_" + anap4;
         }

         fnameo             = fnamei.section( ".",  0, -4 ) + "."
                              + anapart + "."
                              + fnamei.section( ".", -2, -1 );
      }

      else
      {  // Otherwise, just do brute-force name shortening
         fnameo             = QString( fnamei ).section( ".", 0, -3 );
         int kk             = 97 - lnpm1 - lnpm2;
         kk                 = fnameo[ kk - 1 ] == '.' ? ( kk - 1 ) : kk;
         fnameo             = fnameo.left( kk ) + "." + parts[ nparts - 2 ]
                              + "." + parts[ nparts - 1 ];
      }

   }

   return fnameo;
}

