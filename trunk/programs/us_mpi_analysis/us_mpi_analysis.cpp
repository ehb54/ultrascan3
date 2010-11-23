#include "us_mpi_analysis.h"
#include "us_math2.h"

#include <mpi.h>
#include <sys/user.h>

int main( int argc, char* argv[] )
{
   MPI_Init( &argc, &argv );
   QCoreApplication application( argc, argv );
   new US_MPI_Analysis( argv[ 1 ] );
   // return application.exec();
}

// Constructor
US_MPI_Analysis::US_MPI_Analysis( const QString& xmlfile ) : QObject()
{
   MPI_Comm_size( MPI_COMM_WORLD2, &node_count );
   MPI_Comm_rank( MPI_COMM_WORLD2, &my_rank );

   if ( my_rank == 0 ) 
      socket = new QUdpSocket( this );

   maxrss         = 0;
   set_count      = 0;
   iterations     = 1;

   previous_values.variance = 1.0e99;  // A large number

   data_sets .clear();
   parameters.clear();
   analysisDate = QDateTime::currentDateTime().toString( "yyMMddhhmm" );

   // Clear old list of output files if it exists
   QFile::remove( "analysis_files.txt" );

   US_Math2::randomize();   // Set system random sequence

   parse( xmlfile );
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
   meniscus_offsets.resize( meniscus_points );

   double meniscus_start = data_sets[ 0 ]->run_data.meniscus 
                         - meniscus_range / 2.0;
   
   double dm = ( meniscus_points > 1 ) ? meniscus_range / ( meniscus_points - 1 ): 0.0;

   for ( int i = 0; i < meniscus_points; i++ )
   {
      meniscus_offsets[ i ] = meniscus_start + dm * i;
   }

   // Get lower limit of data and last (largest) meniscus value
   double start_range   = data_sets[ 0 ]->run_data.radius( 0 );
   double last_meniscus = meniscus_offsets[ meniscus_points - 1 ];

   if ( last_meniscus >= start_range )
   {
      abort( "Meniscus offset extends into data" );
   }

   meniscus_run = 0;
   mc_iteration = 0;

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
   
   if ( my_rank == 0 )
   {
      send_udp( "Finished: " + QString::number( maxrss) );
      debug   ( "Finished: " + QString::number( maxrss) );
   }

   /*
   else if ( analysis_type == "GA" )
   {

   }
   */

   MPI_Finalize();
   exit ( 0 );
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

// This can probably go away
void US_MPI_Analysis::debug( const QString& s )
{
   qDebug() << s;
}

