#include "shd.h"

int main( int argc, char **argv )
{
   unsigned int max_harmonics = 20;

   vector < shd_point >         model;
   vector < vector < double > > F;
   vector < double >            q( 20 );
   vector < double >            I;

   MPI_Init( &argc, &argv);

   int world_size;
   int world_rank;

   MPI_Comm_size(MPI_COMM_WORLD, &world_size);
   MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

   char processor_name[MPI_MAX_PROCESSOR_NAME];
   int name_len;
   MPI_Get_processor_name(processor_name, &name_len);

   int is_mic = std::string::npos != std::string( processor_name ).find( "-mic" );

   printf("Hello world from processor %s, rank %d"
          " out of %d processors and %s\n",
          processor_name, world_rank, world_size, is_mic ? "and I am a mic" : "and I am NOT a mic" );

   if ( !world_rank )
   {
      // read in data file

      // broadcast F, q, max_harmonics to all
      // split model & distribute to processes
   }


   SHD tSHD( max_harmonics, model, F, q, I, 0 );

   printf( "done\n" );
   MPI_Finalize();
   return 0;
}
             

