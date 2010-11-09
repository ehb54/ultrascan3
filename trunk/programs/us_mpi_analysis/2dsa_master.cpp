#include "us_mpi_analysis.h"
#include "us_math2.h"

QVector< US_MPI_Analysis::Solute > US_MPI_Analysis::create_solutes( 
        double s_min,   double s_max,   double s_step,
        double ff0_min, double ff0_max, double ff0_step )
{
   QVector< Solute > solute_vector;

   for ( double ff0 = ff0_min; ff0 <= ff0_max; ff0 += ff0_step )
      for ( double s = s_min; s <= s_max; s += s_step )
      {
         // Omit s values close to zero.
         if ( s >= -1.0e-14  &&  s <= 1.0e-14 ) continue;

         solute_vector << Solute( s, ff0, 0.0 ); 
      }

   return solute_vector;
}

void US_MPI_Analysis::_2dsa_master( void )
{
// For now assume one data set

   MPI_Status                 status;
   QList< QVector< Solute > > solutes;
   calculated_solutes.clear();

   double s_min   = parameters[ "s_min"          ].toDouble() * 1.0e-13;
   double s_max   = parameters[ "s_max"          ].toDouble() * 1.0e-13;
   double s_res   = parameters[ "s_resolution"   ].toDouble();
   double ff0_min = parameters[ "ff0_min"        ].toDouble();
   double ff0_max = parameters[ "ff0_max"        ].toDouble();
   double ff0_res = parameters[ "ff0_resolution" ].toDouble();

   int grid_repetitions = parameters[ "uniform_grid" ].toInt();
   if ( grid_repetitions < 1 ) grid_repetitions = 1;

   double s_step   = ( s_max   - s_min   ) / s_res;
   double ff0_step = ( ff0_max - ff0_min ) / ff0_res;

   // Allow a 5% overscan
   s_max   += 0.05 * s_step;  // Assumes positive s
   ff0_max += 0.05 * ff0_step;

   solutes.reserve( sq( grid_repetitions ) );

   double s_grid   = s_step   / grid_repetitions;
   double ff0_grid = ff0_step / grid_repetitions;

   // Generate solutes for each grid repetition
   for ( int i = 0; i < grid_repetitions; i++ )
      for ( int j = 0; j < grid_repetitions; j++ )
         solutes << create_solutes( s_min   + s_grid   * i,   s_max,   s_step, 
                                    ff0_min + ff0_grid * j, ff0_max, ff0_step );

   int index      = 0;
   int jobs_left  = solutes.size();
   int total_jobs = solutes.size();
   int jobs_sent  = 0;

   while ( jobs_left > 0 )
   {
      // Wait for worker to send a message
      int size[ 3 ];

      MPI_Recv( &size, 
                3, 
                MPI_INT2,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD2,
                &status);

      MPI_Job   job;
      int       worker;
      QTime     time;

      switch( status.MPI_TAG )
      {
         case MPI_Job::READY: // Ready for work
            if ( jobs_sent >= total_jobs ) break;
            worker      = status.MPI_SOURCE;
            job.command = MPI_Job::PROCESS;
            job.length  = solutes[ index ].size(); 
            send_udp( "Begin solute set " + QString::number ( index + 1 ) );

//time = QTime::currentTime();
//qDebug() << "Master: Ready from " << worker << time.toString( "hh:mm:ss.zzz" ) << jobs_left;

            // Tell worker that solutes are coming
            MPI_Send( &job, 
                sizeof( job ), 
                MPI_BYTE2,
                worker,      // Send to system wanting work
                MPI_Job::MASTER,
                MPI_COMM_WORLD2 );

            // Send solutes
            MPI_Send( solutes[ index++ ].data(), 
                job.length * solute_doubles, 
                MPI_DOUBLE2,  // Pass solute vector as hw independent values
                worker,       // to worker
                MPI_Job::MASTER,
                MPI_COMM_WORLD2 );

            jobs_sent++;
            break;

         case MPI_Job::RESULTS: // Return solute data
            get_results( status, size );
            jobs_left--;
            break;

         default:  // Should never happen
            QString msg =  "Master 2DSA:  Received invalid status " +
                           QString::number( status.MPI_TAG );
            abort( msg );
            break;
      } // switch
   }  // while jobs left

   qSort( calculated_solutes );

   // We need to feed back the results through calc_residuals one
   // more time.  It doesn't do any good to send it to another
   // processor, so jsut do it here.

   Simulation simulation_values;
   simulation_values.solutes = calculated_solutes;
   calc_residuals( 0, 1, simulation_values );

   maxrss = max_rss();

   calculated_solutes = simulation_values.solutes;

   write_2dsa();

   /*
   for ( int i = 0; i < calculated_solutes.size(); i++ )
      qDebug() << QString::number( calculated_solutes[ i ].s, 'e', 3 )
               << QString::number( calculated_solutes[ i ].k, 'f', 3 )
               << QString::number( calculated_solutes[ i ].c, 'f', 5 );
   */


   // Shutdown the workers
   MPI_Job   job;
   job.command = MPI_Job::SHUTDOWN;
 
   for ( int i = 1; i < node_count; i++ )
   {
      //qDebug() << "Shutting down " << i;
      MPI_Send( &job, 
         sizeof( job ), 
         MPI_BYTE2,
         i,               // Send to each worker
         MPI_Job::MASTER,
         MPI_COMM_WORLD2 );
   }
}

void US_MPI_Analysis::get_results( const MPI_Status& status, const int* size )
{
   Simulation simulation_values;
   simulation_values.solutes.resize( size[ 0 ] );
   simulation_values.variances.resize( data_sets.size() );
   simulation_values.ri_noise.resize( size[ 1 ] );
   simulation_values.ti_noise.resize( size[ 2 ] );

   MPI_Status status2;

   // Get all simulation_values
   MPI_Recv( simulation_values.solutes.data(),
             size[ 0 ] * solute_doubles,
             MPI_DOUBLE,
             status.MPI_SOURCE,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status2 );

   calculated_solutes += simulation_values.solutes;

   MPI_Recv( &simulation_values.variance,
             1,
             MPI_DOUBLE,
             status.MPI_SOURCE,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status2 );
   
   MPI_Recv( simulation_values.variances.data(),
             data_sets.size(),
             MPI_DOUBLE,
             status.MPI_SOURCE,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status2 );

   MPI_Recv( simulation_values.ri_noise.data(),
             size[ 1 ],
             MPI_DOUBLE,
             status.MPI_SOURCE,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status2 );

   MPI_Recv( simulation_values.ti_noise.data(),
             size[ 2 ],
             MPI_DOUBLE,
             status.MPI_SOURCE,
             MPI_Job::TAG0,
             MPI_COMM_WORLD2,
             &status2 );
}

void US_MPI_Analysis::write_2dsa( void )
{
   QString fn = db_name + "-" + requestID + "-2dsa.xml";

   // Fill in and write out the model file
   US_Model model;

   model.analysis   = US_Model::TWODSA;
   model.modelGUID  = QUuid::createUuid().toString().mid( 1, 36 );
   model.description = db_name + "-" + requestID + " Generated model";

   for ( int i = 0; i < calculated_solutes.size(); i++ )
   {
      US_Model::SimulationComponent component;
      component.s                    = calculated_solutes[ i ].s;
      component.f_f0                 = calculated_solutes[ i ].k;
      component.signal_concentration = calculated_solutes[ i ].c;

      US_Model::calc_coefficients( component );
      model.components << component;
   }

   model.write( fn );

   // Add the file name of the model file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing", -1 );
      return;
   }

   QTextStream out( &f );
   out << fn << "\n";
   f.close();


}
