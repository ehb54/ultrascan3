#include "us_mpi_analysis.h"
#include "us_math2.h"

QDateTime elapsed = QDateTime::currentDateTime();
#define ELAPSEDNOW (elapsed.msecsTo(QDateTime::currentDateTime())/1000.0)
#define DbTimMsg(a) DbTiming << a << generation << my_rank << ELAPSEDNOW;

void US_MPI_Analysis::ga_worker( void )
{
   current_dataset     = 0;
   datasets_to_process = 1;

   // Initialize grid values
   QStringList keys = parameters.keys();

   s_grid = ( keys.contains( "s_grid" ) ) ? parameters[ "s_grid" ].toInt() : 100;
   k_grid = ( keys.contains( "k_grid" ) ) ? parameters[ "k_grid" ].toInt() : 100;

   for ( int b = 0; b < buckets.size(); b++ )
   {
      double s_min   = buckets[ b ].s_min;
      double s_max   = buckets[ b ].s_max;
      double ff0_min = buckets[ b ].ff0_min;
      double ff0_max = buckets[ b ].ff0_max;
      
      buckets[ b ].ds = ( s_max   - s_min   ) / s_grid;
      buckets[ b ].dk = ( ff0_max - ff0_min ) / k_grid;
   }

   MPI_GA_MSG msg;
   MPI_Status status;
   MPI_Job    job;
   bool       finished = false;

   while ( ! finished )
   {
      ga_worker_loop();

DbgLv(0) << "Worker send finish message" << my_rank << ELAPSEDNOW;

      MPI_Send( &msg,           // This iteration is finished
                sizeof( msg ),  // to MPI #1
                MPI_BYTE,
                MPI_Job::MASTER,
                FINISHED,
                MPI_COMM_WORLD );

      MPI_Recv( &job,          // Find out what to do next
                sizeof( job ), // from MPI #0, MPI #7, MPI #9
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );

      int dataset = job.dataset_offset;
      int count   = job.dataset_count;
      int length  = job.length;
      int index   = 0;

      switch ( status.MPI_TAG )
      {
         case FINISHED: 
            finished = true;
DbgLv(1) << "Worker fitness hits" << my_rank << fitness_hits;
            break;

         case UPDATE:   
            // Update data for global fit or Monte Carlo
            // Global fit comes before MC (if necessary), one dataset at a time
            // Monte Carlo always comes as a sequence of all datasets

            mc_data.resize( length );

            MPI_Barrier( MPI_COMM_WORLD );

            // This is a receive
            MPI_Bcast( mc_data.data(),  // from MPI #8, #10
                       length,
                       MPI_DOUBLE,
                       MPI_Job::MASTER,
                       MPI_COMM_WORLD );

            for ( int e = dataset; e < dataset + count; e++ )
            {
               US_DataIO2::EditedData* data = &data_sets[ e ]->run_data;

               int scan_count    = data->scanData.size();
               int radius_points = data->x.size();

               for ( int s = 0; s < scan_count; s++ )
               {
                  US_DataIO2::Scan* scan = &data->scanData[ s ];

                  for ( int r = 0; r < radius_points; r++ )
                  {
                     scan->readings[ r ].value = mc_data[ index++ ];
                  }
               }
            }

            if ( count == data_sets.size() ) // Next iteration will be global
            {
               current_dataset     = 0;
               datasets_to_process = data_sets.size();
            }
            else // Next dataset is a part of global fit
            {
               current_dataset = dataset;
               // datasets_to_process will stay at 1
            }

            break;

         default:
            abort( "Unknown message at end of GA worker loop" );
            break;

      } // end switch
   }  // end while
}

void US_MPI_Analysis::ga_worker_loop( void )
{
   // Initialize genes
   genes.clear();
   int population = parameters[ "population" ].toInt();

   for ( int i = 0; i < population; i++ ) genes << new_gene();

   // Let calc_residuals get the meniscus directly from the edited data for
   // each data set
   meniscus_value = -1.0;

   fitness.reserve( population );

   Fitness empty_fitness;
   empty_fitness.fitness = LARGE;

   for ( int i = 0; i < population; i++ )
   {
      empty_fitness.index = i;
      fitness << empty_fitness;
   }

   int generations = parameters[ "generations" ].toInt();
   int crossover   = parameters[ "crossover"   ].toInt();
   int mutation    = parameters[ "mutation"    ].toInt();
   int plague      = parameters[ "plague"      ].toInt();
   int elitism     = parameters[ "elitism"     ].toInt();

   int p_mutate    = mutation;
   int p_crossover = p_mutate + crossover;
   int p_plague    = p_crossover + plague;

   fitness_map.clear();
   fitness_hits = 0;

   QDateTime  start = QDateTime::currentDateTime();
   MPI_GA_MSG msg;

////////////DEBUG
//generations = 10;

   for ( generation = 0; generation < generations; generation++ )
   {
DbTimMsg("Worker start generation/rank/elapsed");
      // Calculate fitness
      for ( int i = 0; i < population; i++ )
      {
         fitness[ i ].index   = i;
         fitness[ i ].fitness = get_fitness( genes[ i ] );
      }

      // Sort fitness
      qSort( fitness );

      // Refine with gradient search method (gsm) on last generation
      if ( generation == generations - 1 )
      {

DbTimMsg("Worker before gsm generation/rank/elapsed");

         fitness[ 0 ].fitness = minimize( genes[ fitness[ 0 ].index ], 
                                          fitness[ 0 ].fitness );
DbTimMsg("Worker after gsm generation/rank/elapsed");
      }


      // Ensure gene is on grid
      align_gene( genes[ fitness[ 0 ].index ] );
DbTimMsg("Worker after align_gene");

      // Send best gene to master
      msg.generation = generation;
      msg.size       = genes[ fitness[ 0 ].index ].size();
      msg.fitness    = fitness[ 0 ].fitness;

      MPI_Send( &msg,                                // to MPI #1
                sizeof( msg ),
                MPI_BYTE,
                MPI_Job::MASTER,
                GENERATION,
                MPI_COMM_WORLD );

      MPI_Send( genes[ fitness[ 0 ].index ].data(),  // to MPI #2
                solute_doubles * buckets.size(),
                MPI_DOUBLE,
                MPI_Job::MASTER,
                GENE,
                MPI_COMM_WORLD );

DbTimMsg("Worker after send fitness,genes");
      // Receive instructions from master (continue or finish)
      MPI_Status status;

      MPI_Recv( &msg,                                // from MPI #3
                sizeof( msg ),
                MPI_BYTE,
                MPI_ANY_SOURCE,
                MPI_ANY_TAG,
                MPI_COMM_WORLD,
                &status );
DbTimMsg("Worker after receive instructions");

      if ( status.MPI_TAG == FINISHED ) break;

      // See if we are really done
      if ( generation == generation - 1 ) continue;

      // Mark duplicate genes 
      int f0 = 0;  // An index into the fitness array
      int f1 = 1;  // A second index

      while ( f1 < population )
      {
         // The value of 1.0e-8 for close fitness is arbitrary. Paramaterize?
         if ( fabs( fitness[ f0 ].fitness - fitness[ f1 ].fitness ) < 1.0e-8 )
         {
            bool match = true;

            for ( int i = 0; i < buckets.size(); i++ )
            {
               int g0 = fitness[ f0 ].index;
               int g1 = fitness[ f1 ].index;

               if ( fabs( genes[ g0 ][ i ].s -  genes[ g1 ][ i ].s ) > 1.0e-8 ||
                    fabs( genes[ g0 ][ i ].k -  genes[ g1 ][ i ].k ) > 1.0e-8 )
               {
                  match = false;
                  f0    = f1;
                  break;
               }
            }

            if ( match )
               fitness[ f1 ].fitness = LARGE;  // Invalidate gene/sim_values 
         }
         else
            f0 = f1;

         f1++;
      }

      // Re-sort
      qSort( fitness );
      
      QList< Gene > old_genes = genes;

      // Create new generation from old
      // First copy elite genes
      for ( int g = 0; g < elitism; g++ )
         genes[ g ] = old_genes[ fitness[ g ].index ];

      int emigrants = migrate_genes();
DbTimMsg("Worker before elitism loop");

      for ( int g = elitism + emigrants; g < population; g++ )
      {
         // Select a random gene from old population using exponential 
         // distribution
         int  gene_index  = e_random();
         Gene gene        = old_genes[ gene_index ];
         int  probability = u_random();

         if      ( probability < p_mutate    ) mutate_gene( gene );
         else if ( probability < p_crossover ) cross_gene ( gene );
         else if ( probability < p_plague    ) gene = new_gene();

         genes[ g ] = gene;
      }
DbTimMsg("Worker after elitism loop");
   }  // End of generation loop
DbTimMsg("  +++Worker after generation loop");
}

void US_MPI_Analysis::align_gene( Gene& gene )
{
   for ( int i = 0; i < gene.size(); i++ )
   {
      double s     = gene[ i ].s;
      double s_min = buckets[ i ].s_min;
      double ds    = s - s_min;

      if ( ds < 0 )
         s = s_min;
      else
      {
         int gridpoint = (int)round( ds / buckets[ i ].ds );
         gridpoint     = min( s_grid - 1, gridpoint );
         s             = s_min + gridpoint * buckets[ i ].ds;
      }

      double k     = gene[ i ].k;
      double k_min = buckets[ i ].ff0_min;
      double dk    = k - k_min;

      if ( dk < 0 )
         k = k_min;
      else
      {
         int gridpoint = (int)round( dk / buckets[ i ].dk );
         gridpoint     = min( k_grid - 1, gridpoint );
         k             = k_min + gridpoint * buckets[ i ].dk;
      }

      gene[ i ].s = s;
      gene[ i ].k = k;
   }
}

double US_MPI_Analysis::get_fitness( const Gene& gene )
{
   US_SolveSim::Simulation sim;
   sim.solutes = gene;

   QString key = "";
   QString str;

   for ( int s = 0; s < gene.size(); s++ )
   {
      key += str.sprintf( "%.5f%.5f", sim.solutes[ s ].s,  sim.solutes[ s ].k );
   }

   if ( fitness_map.contains( key ) )
   {
      fitness_hits++;
      return fitness_map.value( key );
   }

   for ( int s = 0; s < gene.size(); s++ ) 
      sim.solutes[ s ].s *= 1.0e-13;

   calc_residuals( current_dataset, datasets_to_process, sim );

   double fitness      = sim.variance;
   int    solute_count = 0;

   for ( int i = 0; i < sim.solutes.size(); i++ )
   {
      if ( sim.solutes[ i ].c >= concentration_threshold ) solute_count++;
   }

   fitness *= ( 1.0 + sq( regularization * solute_count ) );
   fitness_map.insert( key, fitness ); 

   return fitness;
}

double US_MPI_Analysis::get_fitness_v( const US_Vector& v )
{
   // Convert from US_Vector to Gene
   int  size  = v.size() / 2;
   int  index = 0;
   Gene gene( size );
   
   for ( int i = 0; i < size; i++ )
   {
      gene[ i ].s = v[ index++ ];
      gene[ i ].k = v[ index++ ];
   }

   return get_fitness( gene );
}

int US_MPI_Analysis::u_random( int modulo )
{
   // Returns a uniform random integer between 0 and modulo - 1
   // Default modulo is 100
   double rand = (double)qrand() / ( (double)RAND_MAX + 1.0 );
   return (int)floor( modulo * rand );
}

int US_MPI_Analysis::e_random( void )
{
   // Exponential distribution
   double              rand    = (double)qrand() / ( (double)RAND_MAX + 1.0 );
   const double        divisor = 8;  // Parameterize?
   static const double beta    = parameters[ "population" ].toDouble() / divisor;

   int gene_index = (int)( -log( 1.0 - rand ) * beta );
   if ( gene_index >= genes.size() ) gene_index = genes.size() - 1;
   
   return gene_index;
}

void US_MPI_Analysis::mutate_gene( Gene& gene )
{
   static const double p_mutate_s  = parameters[ "p_mutate_s"  ].toDouble();
   static const double p_mutate_k  = parameters[ "p_mutate_k"  ].toDouble();
   static const double p_mutate_sk = parameters[ "p_mutate_sk" ].toDouble();

   static const double p_sk        = 100.0 - p_mutate_sk;  // e.g. 80.0
   static const double p_k         = p_sk  - p_mutate_k;   // e.g. 60.0
   static const double p_s         = p_k   - p_mutate_s;   // e.g. 40.0

   for ( int solute = 0; solute < gene.size(); solute++ )
   {
      int rand = u_random();

      if      ( rand < p_s  ) continue;
      else if ( rand < p_k  ) mutate_s( gene[ solute ], solute );
      else if ( rand < p_sk ) mutate_k( gene[ solute ], solute );
      else
      {
         mutate_s( gene[ solute ], solute );
         mutate_k( gene[ solute ], solute );
      }
   }
}

void US_MPI_Analysis::mutate_s( US_Solute& solute, int b )
{
   // Consider paramaterizing the sigma and x calculations
   double s_min = buckets[ b ].s_min;
   double sigma = ( s_grid - 1 ) / ( 6.0 * ( log2( generation + 2 ) ) );
   double rand  = (double)qrand() / ( (double)RAND_MAX + 1.0 );
   double x     = 4.0 * ( rand - 0.5 );  // Random between -2 and +2
   double delta = round( sigma * x );

   // Ensure new value is at a grid point
   solute.s += delta * buckets[ b ].ds;
   solute.s  = qMax( solute.s, s_min );
   solute.s  = qMin( solute.s, s_min + ( s_grid - 1 ) * buckets[ b ].ds );
}

void US_MPI_Analysis::mutate_k( US_Solute& solute, int b )
{
   static const double mutate_sigma = parameters[ "mutate_sigma" ].toDouble();

   double ff0_min = buckets[ b ].ff0_min;
   double sigma   = ( k_grid - 1 ) / ( 6.0 * ( log2( generation + 2 ) ) );
   double rand    = (double)qrand() / ( (double)RAND_MAX + 1.0 );
   double x       = 2.0 * ( rand - 0.5 ) * mutate_sigma;  // +/- std dev
   double delta   = round( sigma * x );

   // Ensure new value is at a grid point
   solute.k += delta * buckets[ b ].dk;
   solute.k  = qMax( solute.k, ff0_min );
   solute.k  = qMin( solute.k, ff0_min + ( k_grid - 1 ) * buckets[ b ].dk );
}

void US_MPI_Analysis::cross_gene( Gene& gene )
{
   // Get the crossing gene according to an exponential distribution
   int  gene_index = e_random();
   Gene cross_from = genes[ fitness[ gene_index ].index ]; 

   // Select a random solute.  The number will alwys be between
   // 1 and size - 1.
   int solute_index = u_random( gene.size() - 1 ) + 1;
   
   // Swap the genes half the time before cross so if we have
   // (abcde, ABCDE) the crossed gene will be either abCDE or ABcde
   if ( u_random() < 50 ) 
   {
      Gene temp  = gene;
      gene       = cross_from;
      cross_from = temp;
   }

   for ( int i = solute_index; i < buckets.size(); i++ )
   {
      gene[ i ] = cross_from[ i ];
   }
}

int US_MPI_Analysis::migrate_genes( void )
{
   static const int migrate_count = parameters[ "migration" ].toInt();
   static const int elitism_count = parameters[ "elitism"   ].toInt();

   QVector< US_Solute > emmigrants;

   // Send genes to master
   for ( int i = 0; i < migrate_count; i++ )
   {
      int  gene_index = e_random();
      emmigrants += genes[ fitness[ gene_index ].index ];
   }

   // MPI send msg type
   MPI_GA_MSG msg;
   msg.size = migrate_count;

   MPI_Send( &msg,               // to MPI #1
             sizeof( msg ),
             MPI_BYTE,
             MPI_Job::MASTER,
             EMMIGRATE,
             MPI_COMM_WORLD );

   // MPI send emmigrants
   MPI_Send( emmigrants.data(),  // to MPI #4
             buckets.size() * solute_doubles * migrate_count,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             EMMIGRATE,
             MPI_COMM_WORLD );

   // Get genes from master as concatenated genes
   QVector< US_Solute > immigrants( migrate_count * buckets.size() );
   MPI_Status        status;

   MPI_Recv( immigrants.data(),  // from MPI #5
             migrate_count * buckets.size() * solute_doubles,
             MPI_DOUBLE,
             MPI_Job::MASTER,
             IMMIGRATE,
             MPI_COMM_WORLD,
             &status );

   int solutes_sent;
   MPI_Get_count( &status, MPI_DOUBLE, &solutes_sent );

   // The number returned equals the number sent or zero.
   if ( solutes_sent > 0 )
   {
      for ( int i = 0; i < migrate_count; i++ )
      {
         Gene gene = immigrants.mid( i * buckets.size(), buckets.size() );
         genes[ elitism_count + i ] = gene;
      }

      return migrate_count;
   }

   return 0;
}

US_MPI_Analysis::Gene US_MPI_Analysis::new_gene( void )
{
   Gene gene;

   for ( int b = 0; b < buckets.size(); b++ )
   {
      US_Solute solute;

      double s_min   = buckets[ b ].s_min;
      double ff0_min = buckets[ b ].ff0_min;
      
      solute.s = s_min   + u_random( s_grid ) * buckets[ b ].ds;
      solute.k = ff0_min + u_random( k_grid ) * buckets[ b ].dk;

      gene << solute;
   }

   return gene;
}

// Find the minimum fitness value close to a gene using
// inverse hessian minimization 
double  US_MPI_Analysis::minimize( Gene& gene, double fitness )
{
   int       vsize = gene.size() * 2;
   US_Vector v( vsize );  // Input values
   US_Vector u( vsize );  // Vector of derivitives

   // Create hessian as identity matrix
   QVector< QVector< double > > hessian( vsize );

   for ( int i = 0; i < vsize; i++ ) 
   {
      hessian[ i ] = QVector< double >( vsize, 0.0 );
      hessian[ i ][ i ] = 1.0;
   }

   int index = 0;

   // Convert gene to array of doubles
   for ( int i = 0; i < gene.size(); i++ )
   {
      v.assign( index++, gene[ i ].s );
      v.assign( index++, gene[ i ].k );
   }

   lamm_gsm_df( v, u );   // u is vector of derivitives

   static const double epsilon        = 1.0e-7;
   static const int    max_iterations = 20;
   int                 iteration      = 0;

   while ( u.L2norm() >= epsilon  && iteration < max_iterations )
   {
      iteration++;
      if ( fitness == 0.0 ) break;

      US_Vector v_s1 = v;
      double g_s1    = fitness;
      double s1      = 0.0;
      double s2      = 0.5;
      double s3      = 1.0;

      US_Vector v_s2 = u;
      v_s2.scale( -s2 );
      v_s2.add  ( v );                   // v_s2 = u * -s2 + v

      double g_s2 = get_fitness_v( v_s2 );

      // Cut down until we have a decrease
      while ( s2 > epsilon  && g_s2 > g_s1 )
      {
         s3  = s2;
         s2 *= 0.5;
         v_s2 = u;
         v_s2.scale( -s2 );
         v_s2.add  ( v );                // v_s2 = u * -s2 + v
         g_s2 = get_fitness_v( v_s2 );
      }

      // Test for initial decrease
      if ( s2 <= epsilon  ||  s3 - s2 < epsilon ) break;

      US_Vector v_s3 = u;
      v_s3.scale( -s3 );
      v_s3.add  ( v );                   // v_s3 = u * -s3 + v
      double g_s3 = get_fitness_v( v_s3 );

      int              reps     = 0;
      static const int max_reps = 100;

      while ( s2 - s1 > epsilon  &&  s3 - s2 > epsilon && reps < max_reps )
      {
         double s1_s2 = 1.0 / ( s1 - s2 );
         double s1_s3 = 1.0 / ( s1 - s3 );
         double s2_s3 = 1.0 / ( s2 - s3 );

         double s1_2 = sq( s1 );
         double s2_2 = sq( s2 );
         double s3_2 = sq( s3 );

         double a = ( ( g_s1 - g_s3 ) * s1_s3 -
                      ( g_s2 - g_s3 ) * s2_s3
                    ) * s1_s2;

         double b = ( g_s3 * ( s2_2 - s1_2 ) +
                      g_s2 * ( s1_2 - s3_2 ) +
                      g_s1 * ( s3_2 - s2_2 )
                    ) *
                    s1_s2 * s1_s3 * s2_s3;

         static const double max_a = 1.0e-25;

         if ( fabs( a ) < max_a )
         {
            index = 0;

            // Restore gene from array of doubles
            for ( int i = 0; i < gene.size(); i++ )
            {
               gene[ i ].s = v[ index++ ];
               gene[ i ].k = v[ index++ ];
            }

            return fitness;
         }

         double x         = -b / ( 2.0 * a );
         double prev_g_s2 = g_s2;

         if ( x < s1 )
         {
            if ( x < ( s1 + s1 - s2 ) ) // Keep it close
            {
               x = s1 + s1 - s2;             // x <- s1 + ds
               if ( x < 0 ) x = s1 / 2.0;
            }

            if ( x < 0 )  //  Wrong direction!
            {
               if ( s1 < 0 ) s1 = 0.0;
               x = 0;
            }

            // OK, take x, s1, s2 
            US_Vector temp = v_s3;
            v_s3  = v_s2;
            g_s3  = g_s2;                  // 3 <- 2
            s3    = s2;
            v_s2  = v_s1;
            g_s2  = g_s1;
            s2    = s1;                    // 2 <- 1
            v_s1  = temp;
            s1    = x;                     // 1 <- x
 
            v_s1 = u;
            v_s1.scale( -s1 );
            v_s1.add( v );            // v_s1 = u * -s1 + v

            g_s1 = get_fitness_v( v_s1 ); 
         }
         else if ( x < s2 ) // Take s1, x, s2
         {
            US_Vector temp = v_s3;
            v_s3  = v_s2;
            g_s3  = g_s2;             // 3 <- 2
            s3    = s2;
            v_s2  = temp;
            s2    = x;                // 2 <- x

            v_s2 = u;
            v_s2.scale( -s2 );
            v_s2.add( v );           // v_s2 = u * -s2 +v

            g_s2 = get_fitness_v( v_s2 );
         }
         else if ( x < s3 )  // Take s2, x, s3
         {
            US_Vector temp = v_s3;
            v_s1  = v_s2;
            g_s1  = g_s2;
            s1    = s2;              // 2 <- 1
            v_s2  = temp;
            s2    = x;               // 2 <- x

            v_s2 = u;
            v_s2.scale( -s2 );
            v_s2.add  ( v );         // v_s2 = u * -s2 + v

            g_s2 = get_fitness_v( v_s2 );
         }
         else  // x >= s3
         {
            if ( x > ( s3 + s3 - s2 ) ) // if x > s3 + ds/2
            { 
               US_Vector v_s4 = u;
               v_s4.scale( -x );
               v_s4.add  ( v );        // v_s4 = u * -x + v

               double g_s4 = get_fitness_v( v_s4 );

               if ( g_s4 > g_s2 && g_s4 > g_s3 && g_s4 > g_s1 ) 
               {
                  x = s3 + s3 - s2;   // x = s3 + ds/2
               }
            }

            // Take s2, s3, x 
            US_Vector temp = v_s1;
            v_s1  = v_s2;
            g_s1  = g_s2;            // 1 <- 2
            s1    = s2;
            v_s2  = v_s3;
            g_s2  = g_s3;
            s2    = s3;              // 2 <- 3
            v_s3  = temp;
            s3    = x;               // 3 <- x

            v_s3 = u;
            v_s3.scale( -s3 );
            v_s3.add  ( v );         // v_s3 = u * -s3 + v

            g_s3 = get_fitness_v( v_s3 );
         }

         if ( fabs( prev_g_s2 - g_s2 ) < epsilon ) break;
      }  // end of inner loop

      US_Vector v_p( vsize );

      if ( g_s2 < g_s3  &&  g_s2 < g_s1 )
      {
         v_p     = v_s2;
         fitness = g_s2;
      }
      else if ( g_s1 < g_s3 )
      {
         v_p     = v_s1;
         fitness = g_s1;
      }
      else
      {
         v_p     = v_s3;
         fitness = g_s3;
      }
      
      US_Vector v_g( vsize );        // Vector of derivitives
      lamm_gsm_df( v_p, v_g );       // New gradient in v_g (old in u) 

      US_Vector v_dx = v_p;
      US_Vector temp = v;
      temp.scale( -1.0 );
      v_dx.add( temp );              // v_dx = v_p - v

      v = v_p;                       // v    = v_p

      US_Vector v_dg = v_g;
      temp           = u;
      temp.scale( -1.0 );
      v_dg.add( temp );              // dgradient  v_dg = v_g - u

      US_Vector v_hdg( vsize );

      // v_hdg = hessian * v_dg ( matrix * vector )
      for ( int i = 0; i < vsize; i++ )
      {
         v_hdg.assign( i, 0.0 );

         for ( int j = 0; j < vsize; j++ )
            v_hdg.assign( i, v_hdg[ i ] + hessian[ i ][ j ] * v_dg[ j ] );
      }

      double fac   = v_dg.dot( v_dx  );
      double fae   = v_dg.dot( v_hdg );
      double sumdg = v_dg.dot( v_dg  );
      double sumxi = v_dx.dot( v_dx  );

      if ( fac > sqrt( epsilon * sumdg * sumxi ) )
      {
         fac        = 1.0 / fac;
         double fad = 1.0 / fae;

         for ( int i = 0; i < vsize; i++ )
         {
            v_dg.assign( i, fac * v_dx[ i ] - fad * v_hdg[ i ] );
         }

         for ( int i = 0; i < vsize; i++ )
         {
            for ( int j = i; j < vsize; j++ )
            {
               hessian[ i ][ j ] +=
                  fac * v_dx [ i ] * v_dx [ j ] -
                  fad * v_hdg[ i ] * v_hdg[ j ] +
                  fae * v_dg [ i ] * v_dg [ j ];

                 // It's a symmetrical matrix
                 hessian[ j ][ i ] = hessian[ i ][ j ];
            }
         }
      }

      // u = hessian * v_g ( matrix * vector )
      for ( int i = 0; i < vsize; i++ )
      {
         u.assign( i, 0.0 );

         for ( int j = 0; j < vsize; j++ )
            u.assign( i, u[ i ] + hessian[ i ][ j ] * v_g[ j ] );
      }

   }  // end while ( u.L2norm() > epsilon )

   index = 0;

   // Restore gene from array of doubles
   for ( int i = 0; i < gene.size(); i++ )
   {
      gene[ i ].s = v[ index++ ];
      gene[ i ].k = v[ index++ ];
   }

   return fitness;
}

void US_MPI_Analysis::lamm_gsm_df( const US_Vector& v, US_Vector& vd )
{
   static const double h        = 0.01;
   static const double h2_recip = 1.0 / ( h * 2.0 );

   // Work with a temporary vector
   US_Vector t = v;

   for ( int i = 0; i < t.size(); i++ )
   {
      double save = t[ i ];

      t.assign( i, save - h );
      double y0 = get_fitness_v( t );    // Calc fitness value -h

      t.assign( i, save + h );
      double y2 = get_fitness_v( t );    // Calc fitness value +h

      vd.assign( i, ( y2 - y0 ) * h2_recip ); // The derivitive
      t.assign( i, save );
   }

}

/////////////   Debug routines
void US_MPI_Analysis::dump_buckets( void )
{
   if ( my_rank != 1 ) return;
   DbgLv(1) << "Buckets:";
   
   for ( int b = 0; b < buckets.size(); b++ )
   {
      DbgLv(1) << buckets[ b ].s_min
               << buckets[ b ].s_max
               << buckets[ b ].ff0_min
               << buckets[ b ].ff0_max
               << buckets[ b ].ds
               << buckets[ b ].dk;
   }
}

void US_MPI_Analysis::dump_genes( int gene )
{
   if ( my_rank != 1 ) return;
   
   if ( gene > -1 )
   {
      QString s = "Gene " + QString::number( gene ) + ": ";

      for ( int b = 0; b < genes[ gene ].size(); b++ )
      {
         s += QString( "(%1, %2); " ) 
                       .arg( genes[ gene ][ b ]. s )  
                       .arg( genes[ gene ][ b ]. k );

      }

      DbgLv(1) << s;
   }
   else
   {
      DbgLv(1) << "Genes:";
      
      for ( int g = 0; g < genes.size(); g++ )
      {
         QString s;

         for ( int b = 0; b < genes[ g ].size(); b++ )
         {
            s += QString( "(%1, %2)\n" ) 
                       .arg( genes[ g ][ b ]. s )  
                       .arg( genes[ g ][ b ]. k );
         }
       
         DbgLv(1) << s;
      }
   }
}

void US_MPI_Analysis::dump_fitness( const QList< Fitness >& fitness )
{
   if ( my_rank != 1 ) return;
   
   QString s = "Fitness:\n";

   for ( int f = 0; f < fitness.size(); f++ )
   {

      s += QString().sprintf( "i, index, fitness: %3i, %3i, %.6e\n",
               f, fitness[ f ].index, fitness[ f ].fitness );
   }

   DbgLv(1) << s;
}

