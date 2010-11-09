#include "us_mpi_analysis.h"

#include <mpi.h>
#include <sys/user.h>

int main( int argc, char* argv[] )
{
   MPI_Init( &argc, &argv );
   QCoreApplication application( argc, argv );
   new US_MPI_Analysis( argv[ 1 ] );
   //return application.exec();
}

// Constructor
US_MPI_Analysis::US_MPI_Analysis( const QString& xmlfile ) : QObject()
{
   MPI_Comm_size( MPI_COMM_WORLD2, &node_count );
   MPI_Comm_rank( MPI_COMM_WORLD2, &my_rank );

   if ( my_rank == 0 ) 
      socket = new QUdpSocket( this );

   maxrss     = 0;
   iterations = 0;
   set_count  = 0;
   data_sets .clear();
   parameters.clear();

   // Clear old list of output files if it exists
   QFile::remove( "analysis_files.txt" );


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

/*
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
      int result = d->model.load( d->model_file );

      if ( result != US_DataIO2::OK )
      {
         QString msg = "Abort.  Bad model file " +  d->model_file;
         abort( msg );
      }
*/         
   }

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

   QString s( ba );
   QStringList sl = s.split( " " );
   
   // Item 23 is the rss in pages.  Convert to KB
   return sl[ 23 ].toLong() * PAGE_SIZE / 1024;
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

