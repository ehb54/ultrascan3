#include "us_mpi_analysis.h"
#include "us_math2.h"
#include "us_tar.h"

#include <mpi.h>
#include <sys/user.h>

int main( int argc, char* argv[] )
{
   MPI_Init( &argc, &argv );
   QCoreApplication application( argc, argv );

   new US_MPI_Analysis( argv[ 1 ] );
}

// Constructor
US_MPI_Analysis::US_MPI_Analysis( const QString& tarfile ) : QObject()
{
   MPI_Comm_size( MPI_COMM_WORLD2, &node_count );
   MPI_Comm_rank( MPI_COMM_WORLD2, &my_rank );

   if ( my_rank == 0 ) 
      socket = new QUdpSocket( this );

   QString output_dir = "output";
   QDir    d( "." );

   if ( my_rank == 0 )
   {
      // Unpack the input tarfile
      US_Tar tar;

      int result = tar.extract( tarfile );

      if ( result != TAR_OK ) abort( "Could not unpack " + tarfile );

      // Create a dedicated output directory and make sure it's empty
      // During testing, it may not always be empty
      QDir output( "." );
      output.mkdir  ( output_dir );
      output.setPath( output_dir );

      QStringList files = output.entryList( QStringList( "*" ), QDir::Files );
      QString     file;

      foreach( file, files ) output.remove( file );
   }
 
   MPI_Barrier( MPI_COMM_WORLD2 ); // Sync everybody up

   QStringList files = d.entryList( QStringList( "hpc*.xml" ) );
   if ( files.size() != 1 ) abort( "Could not find unique hpc input file." );
   
   QString xmlfile = files[ 0 ];

   maxrss         = 0;
   set_count      = 0;
   iterations     = 1;

   previous_values.variance = 1.0e99;  // A large number

   data_sets .clear();
   parameters.clear();
   buckets   .clear();
   analysisDate = QDateTime::currentDateTime().toString( "yyMMddhhmm" );

   parse( xmlfile );

   uint seed = 0;
   
   if ( parameters.keys().contains( "seed" ) ) 
   {
      seed = parameters[ "seed" ].toUInt();
      qsrand( seed + my_rank );   // Set system random sequence
   }
   else
      US_Math2::randomize();

   send_udp( "Starting" );

   // Read data 
   for ( int i = 0; i < data_sets.size(); i++ )
   {
      DataSet* d = data_sets[ i ];

      try
      {
         int result = US_DataIO2::loadData( ".", d->edit_file, d->run_data );

         if ( result != US_DataIO2::OK ) throw result;
      }
      catch ( int error )
      {
         QString msg = "Abort.  Bad data file " + d->auc_file + " " + d->edit_file;
         abort( msg, error );
      }
      catch ( US_DataIO2::ioError error )
      {
         QString msg = "Abort.  Bad data file " + d->auc_file + " " + d->edit_file;
         abort( msg, error );
      }

      for ( int j = 0; j < d->noise_files.size(); j++ )
      {
          US_Noise noise;

          if ( noise.load( d->noise_files[ j ] ) != 0 )
          {
             QString msg = "Abort.  Bad noise file " + d->noise_files[ j ];
             abort( msg );
          }

          if ( noise.apply_to_data( d->run_data  ) != 0 )
          {
             QString msg = "Abort.  Bad noise file " + d->noise_files[ j ];
             abort( msg );
          }
      }
   }

   // After reading all input, set the working directory for file output.
   QDir::setCurrent( output_dir );

   // Set some minimums
   max_iterations  = parameters[ "max_iterations" ].toInt();
   max_iterations  = max( max_iterations, 1 );

   mc_iterations   = parameters[ "mc_iterations" ].toInt();
   mc_iterations   = max( mc_iterations, 1 );

   meniscus_range  = parameters[ "meniscus_range"  ].toDouble();
   meniscus_points = parameters[ "meniscus_points" ].toInt();
   meniscus_points = max( meniscus_points, 1 );

   // Do some parameter checking
   bool global_fit = data_sets.size() > 1;

   if ( global_fit  &&  meniscus_points > 1 )
   {
      abort( "Meniscus fit is not compatible with multiple data sets" );
   }

   if ( meniscus_points > 1  &&  mc_iterations > 1 )
   {
      abort( "Meniscus fit is not compatible with Monte Carlo analysis" );
   }

   bool noise = parameters[ "rinoise_option" ].toInt() > 0  ||
                parameters[ "rinoise_option" ].toInt() > 0;

   if ( mc_iterations > 1  &&  noise )
   {
      abort( "Monte Carlo iteration is not compatible with noise computation" );
   }

   if ( global_fit && noise )
   {
      abort( "Global fit is not compatible with noise computation" );
   }

   // Calculate meniscus values
   meniscus_values.resize( meniscus_points );

   double meniscus_start = data_sets[ 0 ]->run_data.meniscus 
                         - meniscus_range / 2.0;
   
   double dm = ( meniscus_points > 1 ) ? meniscus_range / ( meniscus_points - 1 ): 0.0;

   for ( int i = 0; i < meniscus_points; i++ )
   {
      meniscus_values[ i ] = meniscus_start + dm * i;
   }

   // Get lower limit of data and last (largest) meniscus value
   double start_range   = data_sets[ 0 ]->run_data.radius( 0 );
   double last_meniscus = meniscus_values[ meniscus_points - 1 ];

   if ( last_meniscus >= start_range )
   {
      abort( "Meniscus value extends into data" );
   }

   regularization          = parameters[ "regularization" ].toDouble();
   concentration_threshold = parameters[ "conc_threshold" ].toDouble();

   // Calculate vbar20, vbar_tb, and s, D corrections for calc_residuals
   for ( int i = 0; i < data_sets.size(); i++ )
   {
      DataSet* ds = data_sets[ i ];

      // Calculate average temperature

      int    scan_count = ds->run_data.scanData.size();
      double sum        = 0.0;

      for ( int j = 0; j < scan_count; j++ )
         sum += ds->run_data.scanData[ j ].temperature;

      ds->temperature = sum / scan_count;

      // Calculate common vbar
      US_Solution solution;

      for ( int j = 0; j < scan_count; j++ )
      {
         US_Solution::AnalyteInfo ai;
         
         ai.amount         = ds->analytes[ j ].amount;
         ai.analyte.vbar20 = ds->analytes[ j ].vbar20;
         ai.analyte.mw     = ds->analytes[ j ].mw;

         if ( ds->analytes[ j ].type == "Protein" )
            ai.analyte.type   = US_Analyte::PROTEIN;
         else
            ai.analyte.type   = US_Analyte::DNA;  // Actually 'other'

         solution.analyteInfo << ai;
      }

      ds->vbar20  = US_Math2::calcCommonVbar( solution, 20.0 );
      ds->vbar_tb = US_Math2::calcCommonVbar( solution, ds->temperature );

      // Convert to a different structure and calulate the s and D corrections
      US_Math2::SolutionData sd;
      sd.density   = ds->density;
      sd.viscosity = ds->viscosity;
      sd.vbar20    = ds->vbar20;
      sd.vbar      = ds->vbar_tb;

      US_Math2::data_correction( ds->temperature, sd );

      ds->s20w_correction = sd.s20w_correction;
      ds->D20w_correction = sd.D20w_correction;
   }

   // Check GA buckets
   if ( analysis_type == "GA" )
   {
      if ( buckets.size() < 2 )
         abort( "Insufficient buckets defined" );

      QList< QRectF > bucket_rects;

      // Put into Qt rectangles (upper left, lower right points)
      for ( int i = 0; i < buckets.size(); i++ )
      {
         bucket_rects << QRectF( 
               QPointF( buckets[ i ].s_min, buckets[ i ].ff0_max ),
               QPointF( buckets[ i ].s_max, buckets[ i ].ff0_min ) );
      }

      for ( int i = 0; i < bucket_rects.size() - 1; i++ )
      {
         for ( int j = i + 1; j < bucket_rects.size(); j++ )
         {
            if ( bucket_rects[ i ].intersects( bucket_rects[ j ] ) )
               abort( "Buckets overlap" );
         }
      }
   }

   // Set some defaults
   if ( ! parameters.contains( "mutate_sigma" ) ) 
      parameters[ "mutate_sigma" ] = "2.0";

   if ( ! parameters.contains( "p_mutate_s"   ) ) 
      parameters[ "p_mutate_s"   ] = "20";

   if ( ! parameters.contains( "p_mutate_k"   ) ) 
      parameters[ "p_mutate_k"   ] = "20";

   if ( ! parameters.contains( "p_mutate_sk"  ) ) 
      parameters[ "p_mutate_sk"  ] = "20";

   count_calc_residuals = 0;   // Internal instrumentation
   meniscus_run         = 0;
   mc_iteration         = 0;
   start();
}

// Main function
void US_MPI_Analysis::start( void )
{
   // Real processing goes here
   if ( analysis_type == "2DSA" )
   {
      iterations = parameters[ "montecarlo_value" ].toInt();

      if ( iterations < 1 ) iterations = 1;

      if ( my_rank == 0 ) 
          _2dsa_master();
      else
          _2dsa_worker();
   }

   else if ( analysis_type == "GA" )
   {
      if ( my_rank == 0 ) 
          ga_master();
      else
          ga_worker();
   }
   
   if ( my_rank == 0 )
   {
      send_udp( "Finished: " + QString::number( maxrss) );
      qDebug() << "Finished: " + QString::number( maxrss );
   }

   // Pack results
   if ( my_rank == 0 )
   {
      QDir        d( "." );
      QStringList files = d.entryList( QStringList( "*" ), QDir::Files );

      US_Tar tar;
      tar.create( "analysis-results.tar", files );

      // Remove the files we just put into the tar archive
      QString file;
      foreach( file, files ) d.remove( file );
   }

   MPI_Finalize();
   exit( 0 );
}

// Send udp
void US_MPI_Analysis::send_udp( const QString& message )
{
   if ( my_rank != 0 ) return;

   QString    jobid = db_name + "-" + requestID + ": ";
   QByteArray msg   = QString( jobid + message ).toAscii();
   socket->writeDatagram( msg.data(), msg.size(), server, port );
}

long int US_MPI_Analysis::max_rss( void )
{
   // Read /prod/$pid/stat
   QFile f( "/proc/" + QString::number( getpid() ) + "/stat" );
   f.open( QIODevice::ReadOnly );
   QByteArray ba = f.read( 1000 );
   f.close();

   //The 23rd entry is RSS in 4K pages.  Convert to kilobytes.

   const static int k = PAGE_SIZE / 1024;
   maxrss = max( maxrss, QString( ba ).section( " ", 23, 23 ).toLong() * k );
   return maxrss;
}

void US_MPI_Analysis::abort( const QString& message, int error )
{
    send_udp( message );
    qDebug() << message;
    MPI_Abort( MPI_COMM_WORLD2, error );
    exit( error );
}
