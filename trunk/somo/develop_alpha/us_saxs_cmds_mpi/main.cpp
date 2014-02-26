#include <mpi.h>
#include "../include/us_saxs_cmds_t.h"
#include "../include/us_revision.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

int npes;
int myrank;

QString US_Version = REVISION;

int main (int argc, char **argv)
{
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &npes);
   MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

   if (argc < 2) 
   {
      if ( !myrank )
      {
         printf(
                "usage: %s command params\n"
                "Valid commands \tparams:\n"
                "iq            \tcontrolfile\n"
                "              \tcompute a saxs curve (can be a .tar)\n"
                "nsa           \tcontrolfile\n"
                "              \tperform nsa analysis\n"
                "1d            \tcontrolfile\n"
                "              \tperform 1d analysis\n"
                "pm            \tcontrolfile\n"
                "              \tperform pm\n"
                , argv[0]
                );
      }
      MPI_Finalize();
      exit(-1);
   }
   vector < QString > cmds;
   for (int i = 1; i < argc; i++) 
   {
      cmds.push_back(argv[i]);
   }
   int errorbase = -1000;


   if ( cmds[0].lower() == "iq" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         if ( !myrank )
         {
            printf(
                   "usage: %s %s controlfile\n"
                   , argv[0]
                   , argv[1]
                   );
         }
         MPI_Finalize();
         exit( errorbase );

      }
      errorbase--;

      int p = 1;
      QString controlfile     = cmds[ p++ ];

      US_Saxs_Util usu;
      // cout << QString("%1: starting processing\n" ).arg( myrank ) << flush;
      //       if ( !usu.align_test() )
      //       {
      //          cout << "align test aborted\n" << flush;
      //          cout << usu.errormsg << endl << flush;
      //          MPI_Finalize();
      //          exit( errorbase - 1 );
      //       }

      if ( !usu.run_iq_mpi( controlfile ) )
      {
         if ( !myrank )
         {
            cout << usu.errormsg << endl;
         }
         MPI_Finalize();
         exit( errorbase - 1 );
      }
      MPI_Finalize();
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].lower() == "nsa" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         if ( !myrank )
         {
            printf(
                   "usage: %s %s controlfile\n"
                   , argv[0]
                   , argv[1]
                   );
         }
         MPI_Finalize();
         exit( errorbase );

      }
      errorbase--;

      int p = 1;
      QString controlfile     = cmds[ p++ ];

      US_Saxs_Util usu;
      // cout << QString("%1: starting processing\n" ).arg( myrank ) << flush;
      if ( !usu.run_nsa_mpi( controlfile ) )
      {
         if ( !myrank )
         {
            cout << usu.errormsg << endl;
         }
         MPI_Finalize();
         exit( errorbase - 1 );
      }
      MPI_Finalize();
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].lower() == "1d" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         if ( !myrank )
         {
            printf(
                   "usage: %s %s controlfile\n"
                   , argv[0]
                   , argv[1]
                   );
         }
         MPI_Finalize();
         exit( errorbase );

      }
      errorbase--;

      int p = 1;
      QString controlfile     = cmds[ p++ ];

      US_Saxs_Util usu;
      // cout << QString("%1: starting processing\n" ).arg( myrank ) << flush;
      if ( !usu.run_1d_mpi( controlfile ) )
      {
         if ( !myrank )
         {
            cout << usu.errormsg << endl;
         }
         MPI_Finalize();
         exit( errorbase - 1 );
      }
      MPI_Finalize();
      exit(0);
   }
   errorbase -= 1000;

   if ( cmds[0].lower() == "pm" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         if ( !myrank )
         {
            printf(
                   "usage: %s %s controlfile\n"
                   , argv[0]
                   , argv[1]
                   );
         }
         MPI_Finalize();
         exit( errorbase );

      }
      errorbase--;

      int p = 1;
      QString controlfile     = cmds[ p++ ];

      US_Saxs_Util usu;
      // cout << QString("%1: starting processing\n" ).arg( myrank ) << flush;
      if ( !usu.run_pm_mpi( controlfile ) )
      {
         if ( !myrank )
         {
            cout << usu.errormsg << endl;
         }
         MPI_Finalize();
         exit( errorbase - 1 );
      }
      MPI_Finalize();
      exit(0);
   }
   errorbase -= 1000;

   if ( !myrank )
   {
      printf("%s error: %s unknown command\n", argv[0], argv[1]);
   }
   MPI_Finalize();
   exit(-2);
}
