#include "../include/us_fe_nnls_t.h"
#include <mpi.h>

int npes;
int myrank;

int main (int argc, char **argv)
{

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &npes);
   MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
   QString gridopt = "";
   cout << "hello from " << myrank << endl;
   {
      char b[256];
      sprintf(b, "echo %d: `uname -n`\n", myrank);
      system(b);
      fflush(stdout);
   }


   QString data_file, solute_file, job_id;
   //   QApplication a(argc, argv);
   int result = 0;
   if(argc < 4)
   {
      cout << "Usage: us_fe_nnls_t <path/input_file> <path/solute_file> job_id\n\n";
      MPI_Finalize();
      return(-1);
   }
   else
   {
      data_file = argv[1];
      solute_file = argv[2];
      job_id = argv[3];
      if(argc >= 5) {
         gridopt = argv[4];
      }
   }
   
   QString checkpoint_file = "";
   if ( argc >= 6 ) 
   {
      checkpoint_file = argv[5];
   }
      
   US_fe_nnls_t *us_fe_nnls_t;
   us_fe_nnls_t = new US_fe_nnls_t();
   //   if(!myrank) {
   result = us_fe_nnls_t->init_run(data_file, solute_file, job_id, gridopt, checkpoint_file);
   //   }
   us_fe_nnls_t->run(result);
   //   return (result);

   MPI_Finalize();

   exit(0);
}
