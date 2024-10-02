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
                "json          \tjson\n"
                "              \tprocess commands from json provided as command line argument\n"
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


   if ( cmds[0].toLower() == "iq" ) 
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

   if ( cmds[0].toLower() == "nsa" ) 
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

   if ( cmds[0].toLower() == "1d" ) 
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

   if ( cmds[0].toLower() == "pm" ) 
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

   if ( cmds[0].toLower() == "json" ) 
   {
      if ( cmds.size() != 2 ) 
      {
         if ( !myrank )
         {
            printf(
                   "%s{\"errors\":\"%s %s incorrect number of arguments\"}\n%s"
                   , MPI_JSON_SNIP_START
                   , argv[0]
                   , argv[1]
                   , MPI_JSON_SNIP_END
                   );
         }
         MPI_Finalize();
         exit( errorbase );
      }
      errorbase--;

      int p = 1;
      QString      json         = cmds[ p++ ];

      US_Saxs_Util usu;
      if ( !usu.run_json_mpi( json ) )
      {
         if ( !myrank )
         {
            printf( "%s{\"errors\":\"mpi run failed\"}\n%s"
                    , MPI_JSON_SNIP_START
                    , MPI_JSON_SNIP_END
                    );
         }
      }
      MPI_Finalize();
      exit( 0 );
   }
   errorbase -= 1000;

   if ( !myrank )
   {
      printf("%s error: %s unknown command\n", argv[0], argv[1]);
   }
   MPI_Finalize();
   exit(-2);
}
