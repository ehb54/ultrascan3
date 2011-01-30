#include "us_mpi_analysis.h"
#include "us_util.h"

//TODO: Send udp progress message when appropriate

void US_MPI_Analysis::ga_master( void )
{
   QList  < Gene    > best_genes;   // Size is number of processors
   QList  < Gene    > emigres;      // Holds genes passed as emmigrants
   QList  < Fitness > best_fitness; // Size is number of processors
   QVector< int     > generations( node_count, 0 ); 

   best_genes.reserve  ( node_count);
   best_fitness.reserve( node_count);

   Fitness empty_fitness;
   int     workers = node_count - 1;
   Gene    working_gene( buckets.count(), Solute() );

   empty_fitness.fitness = 9.9e99;  // A large number

   for ( int i = 0; i < node_count; i++ )
   {
      best_genes << working_gene;

      empty_fitness.index = i;
      best_fitness << empty_fitness;
   }

   int max_generation = -1;

   while ( workers > 0 )
   {
      MPI_GA_MSG msg;
      MPI_Status status;
      int        worker;

      MPI_Recv( &msg,          // Get a message
                sizeof( msg ),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );

      worker = status.MPI_SOURCE;

      switch ( status.MPI_TAG )
      {
         case GENERATION:
            generations[ worker ] = msg.generation;

            if ( msg.generation > max_generation )
            {
               max_generation = msg.generation;
               
               QString progress =
                  " Generation: "  + QString::number( max_generation ) +
                  "; MonteCarlo: " + QString::number( mc_iteration );

               send_udp( progress );
            }

            // Get the best gene for the current generation from the worker
            MPI_Recv( best_genes[ worker ].data(),
                      buckets.size() * solute_doubles,
                      MPI_DOUBLE,  
                      worker,
                      GENE,
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE );

            if ( msg.fitness < best_fitness[ worker ].fitness )
            {
               best_fitness[ worker ].fitness = msg.fitness;
            }

            break;

         case FINISHED:
            workers--;
            break;


         case EMMIGRATE:
         {
            // First get a set of genes as a concatenated vector.  
            int               gene_count    = msg.size;
            int               doubles_count = gene_count * buckets.size() * 
                                              solute_doubles;
            QVector< double > emmigrants( doubles_count ) ;

            MPI_Recv( emmigrants.data(),
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      EMMIGRATE,
                      MPI_COMM_WORLD,
                      MPI_STATUS_IGNORE );

            // Add the genes to the emmigres list
            int solute = 0;

            for ( int i = 0; i < gene_count; i++ )
            {
               Gene gene;

               for ( int b = 0; b < buckets.size(); b++ )
               {
                  double s = emmigrants[ solute++ ];
                  double k = emmigrants[ solute++ ];
                  gene << Solute( s, k );
                  solute++; // Concentration         
               }

               emigres << gene;
            }

            // Don't send any back if the pool is too small
            if ( emigres.size() < gene_count * 5 ) doubles_count = 0;

            // Get immigrents from emmigres
            QVector< Solute > immigrants;

            if ( doubles_count > 0 )
            {
               // Prepare a vector of concatenated genes form the emmigrant list
               for ( int i = 0; i < gene_count; i++ )
                  immigrants += emigres.takeAt( u_random( emigres.size() ) );
            }

            MPI_Send( immigrants.data(),
                      doubles_count,
                      MPI_DOUBLE,
                      worker,
                      IMMIGRATE,
                      MPI_COMM_WORLD );
            break;
         }
      }
   }

   qSort( best_fitness );

   Simulation sim;

   sim.solutes = best_genes[ best_fitness[ 0 ].index ];

   // TODO: Handle multiple data sets here
   meniscus_value = data_sets[ 0 ]->run_data.meniscus; // Used in calc_residuals

   for ( int g = 0; g < buckets.size(); g++ )
      sim.solutes[ g ].s *= 1.0e-13;

   calc_residuals( 0, data_sets.size(), sim );

   write_model( sim, US_Model::GA );
}

// TODO: Combine this function with write_2dsa()
void US_MPI_Analysis::write_ga_output( const Simulation& sim )
{
   US_DataIO2::EditedData* data = &data_sets[ 0 ]->run_data;

   // Fill in and write out the model file
   US_Model model;

   model.monteCarlo  = mc_iteration > 1;
   model.wavelength  = data->wavelength.toDouble();
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = data->editGUID;
   model.requestGUID = requestGUID;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = US_Model::GA;
   model.global      = US_Model::NONE;   // For now.  Will change later.

   model.description = data->runID + ".ga a" + analysisDate +
                       " e" + data->editID +
                       db_name + "-" + requestID;
   // Save as class variable for later reference
   modelGUID = model.modelGUID;

   for ( int i = 0; i < sim.solutes.size(); i++ )
   {
      const Solute* solute = &sim.solutes[ i ];

      US_Model::SimulationComponent component;
      component.s                    = solute->s;
      component.f_f0                 = solute->k;
      component.signal_concentration = solute->c;

      US_Model::calc_coefficients( component );
      model.components << component;
   }

   QString fn = data->runID + ".ga." + model.modelGUID + ".xml";
   model.write( fn );

   // Add the file name of the model file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream out( &f );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   out << fn << ";meniscus_value=" << meniscus_value
             << ";MC_iteration="   << mc_iteration
             << ";variance="       << sim.variance
             << "\n";
   f.close();
}

void US_MPI_Analysis::write_model( const Simulation&      sim, 
                                   US_Model::AnalysisType type )
{
   QString id;

   switch ( type )
   {
      case US_Model::TWODSA:
         id = "2dsa";
         break;

      case US_Model::GA:
         id = "ga";
         break;

      default:
         id = "unk";
         break;
   }

   US_DataIO2::EditedData* data = &data_sets[ 0 ]->run_data;

   // Fill in and write out the model file
   US_Model model;

   model.monteCarlo  = mc_iteration > 1;
   model.wavelength  = data->wavelength.toDouble();
   model.modelGUID   = US_Util::new_guid();
   model.editGUID    = data->editGUID;
   model.requestGUID = requestGUID;
   //model.optics      = ???  How to get this?  Is is needed?
   model.analysis    = type;
   model.global      = US_Model::NONE;   // For now.  Will change later.

   model.description = data->runID + "." + id + "_a" + analysisDate +
                       " e" + data->editID +
                       db_name + "-" + requestID;
   // Save as class variable for later reference
   modelGUID = model.modelGUID;

   for ( int i = 0; i < sim.solutes.size(); i++ )
   {
      const Solute* solute = &sim.solutes[ i ];

      US_Model::SimulationComponent component;
      component.s                    = solute->s;
      component.f_f0                 = solute->k;
      component.signal_concentration = solute->c;

      US_Model::calc_coefficients( component );
      model.components << component;
   }

   QString fn = data->runID + "." + id + "." + model.modelGUID + ".xml";
   model.write( fn );

   // Add the file name of the model file to the output list
   QFile f( "analysis_files.txt" );
   if ( ! f.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append ) )
   {
      abort( "Could not open 'analysis_files.txt' for writing" );
      return;
   }

   QTextStream out( &f );

   QString meniscus = QString::number( meniscus_value, 'e', 4 );
   QString variance = QString::number( sim.variance,   'e', 4 );

   out << fn << ";meniscus_value=" << meniscus_value
             << ";MC_iteration="   << mc_iteration
             << ";variance="       << sim.variance
             << "\n";
   f.close();
}
