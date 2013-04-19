#include "shd.h"

#define SHOW_MPI_TIMING

// #define SHOW_TIMING
#if defined( SHOW_TIMING )
#  include <sys/time.h>
   static struct timeval start_tv;
   static struct timeval end_tv;
#endif

int world_size;
int world_rank;

int main( int argc, char **argv )
{
   shd_input_data               id;

   vector < double >            q;
   vector < vector < double > > F;
   vector < double >            I;
   vector < shd_point >         my_model;

   MPI_Init( &argc, &argv);


   MPI_Comm_size(MPI_COMM_WORLD, &world_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

   char processor_name[MPI_MAX_PROCESSOR_NAME];
   int name_len;
   MPI_Get_processor_name(processor_name, &name_len);

   int is_mic = std::string::npos != std::string( processor_name ).find( "-mic" );

   printf("%s, rank %d"
          " of %d a %sa MIC\n",
          processor_name, world_rank, world_size, is_mic ? "" : "NOT " );

   if ( !world_rank )
   {
      vector < shd_point >         model;
#if defined( SHOW_TIMING )
      gettimeofday( &start_tv, NULL );
#endif
      cout << "sizeof float :" << sizeof( float ) << endl;
      cout << "sizeof double:" << sizeof( double ) << endl;

      int errorno = -1;
      if ( argc < 2 )
      {
         cerr << "Error: usage: shd inputfile\n";         
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      // read in data file

      ifstream ifs( argv[ 1 ], ios::in | ios::binary );

      if ( !ifs.is_open() )
      {
         cerr << "Error: shd coult not open inputfile " << argv[ 1 ] << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      if ( !ifs.read( (char *)&id, sizeof( id ) ) )
      {
         cerr << "Error: " << argv[ 1 ] << " can not read input data" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;
      cout << "max harmonics:" << id.max_harmonics << endl;
      cout << "F size       :" << id.F_size << endl;
      cout << "q size       :" << id.q_size << endl;
      cout << "model size   :" << id.model_size << endl;

#if defined( SHOW_TIMING )
      gettimeofday( &end_tv, NULL );
      printf( "0: init, file read %lums\n",
              ( 1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec ) / 1000l );
      fflush(stdout);
      gettimeofday( &start_tv, NULL );
#endif
      vector < double > tmp_F( id.q_size );

      q    .resize( id.q_size     );
      model.resize( id.model_size );

      for ( int32_t i = 0; i < id.F_size; ++i )
      {
         if ( !ifs.read( (char *)(&tmp_F[ 0 ]), sizeof( double ) * id.q_size ) )
         {
            cerr << "Error: " << argv[ 1 ] << " can not read F data" << endl;
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
         F.push_back( tmp_F );
      }
      errorno--;

      if ( !ifs.read( (char*)(&q[ 0 ]), sizeof( double ) * id.q_size ) )
      {
         cerr << "Error: " << argv[ 1 ] << " can not read q data" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      if ( !ifs.read( (char *)(&model[ 0 ]), sizeof( struct shd_point ) * id.model_size ) )
      {
         cerr << "Error: " << argv[ 1 ] << " can not read model data" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

      ifs.close();

      uint32_t org_model_size  = id.model_size;
      id.model_size            = ( org_model_size % world_size ? 1 : 0 ) + org_model_size / world_size;
      uint32_t extd_model_size = id.model_size * world_size;

      printf( "org model_size %d\n",  org_model_size );
      printf( "new model size %d\n",  id.model_size );
      printf( "extd model size %d\n", extd_model_size );
      
      // pad for scatter
      model.resize( extd_model_size );
      for ( int i = org_model_size; i < extd_model_size; i++ )
      {
         model[ i ].ff_type = -1;
      }

      // broadcast input data 

      if ( MPI_SUCCESS != MPI_Bcast( &id, sizeof( id ), MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;
         
      // broadcast F
      for ( int32_t i = 0; i < id.F_size; ++i )
      {
         if ( MPI_SUCCESS != MPI_Bcast( &(F[ i ][ 0 ] ), id.q_size, MPI_DOUBLE, 0, MPI_COMM_WORLD ) )
         {
            cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
      }
      errorno--;

      // Scatter model

      my_model.resize( id.model_size );

      if ( MPI_SUCCESS != MPI_Scatter( (void *)&(model[ 0 ]),    id.model_size * sizeof( struct shd_point ), MPI_CHAR,
                                       (void *)&(my_model[ 0 ]), id.model_size * sizeof( struct shd_point ), MPI_CHAR,
                                       0, MPI_COMM_WORLD ) )
      {
         cerr << "Error: MPI_Scatter( model ) failed" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      errorno--;

#if defined( SHOW_TIMING )
      gettimeofday( &end_tv, NULL );
      printf( "0: broadcast, scatter %lums\n",
              ( 1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec ) / 1000l );
      fflush(stdout);
#endif

      // split model & distribute to processes
   } else {
      int errorno = -100;
      // broadcast (receive) data

      if ( MPI_SUCCESS != MPI_Bcast( &id, sizeof( id ), MPI_CHAR, 0, MPI_COMM_WORLD ) )
      {
         cerr << "Error:" << world_rank << " MPI_Bcast( id ) failed" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }
      // cout << "max harmonics:" << world_rank << " " << id.max_harmonics << endl;
      // cout << "F size       :" << world_rank << " " << id.F_size << endl;
      // cout << "q size       :" << world_rank << " " << id.q_size << endl;
      // cout << "model size   :" << world_rank << " " << id.model_size << endl;

      F       .resize( id.F_size );
      q       .resize( id.q_size );
      my_model.resize( id.model_size );

      // broadcast (receive) F

      for ( int32_t i = 0; i < id.F_size; ++i )
      {
         F[ i ].resize( id.q_size );
         if ( MPI_SUCCESS != MPI_Bcast( &(F[ i ][ 0 ] ), id.q_size, MPI_DOUBLE, 0, MPI_COMM_WORLD ) )
         {
            cerr << "Error: MPI_Bcast( id ) sender failed" << endl;
            MPI_Abort( MPI_COMM_WORLD, errorno );
            exit( errorno );
         }
      }
      errorno--;

      void * null;

      if ( MPI_SUCCESS != MPI_Scatter( null,            id.model_size * sizeof( struct shd_point ), MPI_CHAR,
                                       &(my_model[ 0 ]), id.model_size * sizeof( struct shd_point ), MPI_CHAR,
                                       0, MPI_COMM_WORLD ) )
      {
         cerr << "Error: MPI_Scatter( model ) failed" << endl;
         MPI_Abort( MPI_COMM_WORLD, errorno );
         exit( errorno );
      }

      errorno--;
   }

   vector < complex < float > > Avp;

   // cout << world_rank << " initial barrier\n" << endl << flush;
   // MPI_Barrier( MPI_COMM_WORLD );
   // cout << world_rank << " initial barrier exit\n" << endl << flush;


#if defined( SHOW_TIMING )
   gettimeofday( &start_tv, NULL );
#endif

#if defined( SHOW_MPI_TIMING )
   double time_start;
   if ( !world_rank )
   {
      time_start = MPI_Wtime();
   }
#endif

   SHD tSHD( ( unsigned int ) id.max_harmonics, my_model, F, q, I, 0 );
   tSHD.compute_amplitudes( Avp );
#if defined( SHOW_TIMING )
   gettimeofday( &end_tv, NULL );
   printf( "%d: compute amplitudes %lums model size %d\n",
           world_rank,
           ( 1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec ) / 1000l,
           my_model.size()
           );
   fflush(stdout);
   gettimeofday( &start_tv, NULL );
#endif
#if defined( SHOW_MPI_TIMING )
   if ( !world_rank )
   {
      double time_end = MPI_Wtime();
      printf( "%d: compute amplitudes %gs model size %d\n",
              world_rank,
              ( time_end - time_start ) * 1e3,
              my_model.size()
              );
   }
#endif

   // cout << world_rank << " done" << endl << flush;

   // cout << world_rank << " final barrier\n" << endl << flush;
   // MPI_Barrier( MPI_COMM_WORLD );
   // cout << world_rank << " final barrier exit\n" << endl << flush;
   MPI_Finalize();
   exit(0);
}
             

