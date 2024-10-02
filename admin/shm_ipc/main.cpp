// shared memory utility using sysv ipc

#include <QtCore>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <set>
#include <csignal>

// includes required by sysv ipc
#include <sys/ipc.h>
#include <sys/shm.h>

#define TSO QTextStream( stdout )
#define TSE QTextStream( stderr )

int shmid;
void *shmbuf;

// update if US_Global::Global changes structure
class Global
{
public:
   QPoint current_position;
   char   passwd[64];
   // Add other global values as necessary
};

void signal_handler( int signal_num ) {
   TSE << "Caught signal:" << signal_num << ". Exiting.\n";
   // deletes it :(
   // if ( shmid != -1 ) {
   //    struct shmid_ds shmds;
   //    if ( -1 == shmctl( shmid, IPC_RMID, &shmds ) ) {
   //       TSE << "shmctl set IPC_RMID failed\n";
   //    } else {
   //       TSE << "shmctl set IPC_RMID\n";
   //    }
   // }
   if ( shmbuf != (void *)-1 ) {
      TSE << "Detaching Shared Memory\n";
      shmdt( shmbuf );
   } else {
      TSE << "No shared memory to detach\n";
   }
   exit(-signal_num);
}

int main(int argc, char *argv[])
{
   shmid = -1;
   shmbuf = (void *)-1;

   QCoreApplication a(argc, argv);
   QCoreApplication::setApplicationName("shmutil");
   QCoreApplication::setApplicationVersion("Version: 1");

   QCommandLineParser parser;
   parser.setApplicationDescription("Description: shmnew tests shared memory");
   parser.addHelpOption();
   parser.addVersionOption();

   
   parser.addOption(
                    {
                     { "u", "user" }
                     ,"set user name"
                     ,"user"
                    }
                    );
   
   parser.addOption(
                    {
                     { "t", "test-attach" }
                     ,"test attaching to the users shm"
                    }
                    );

   parser.addOption(
                    {
                     { "c", "create-shm" }
                     ,"creates an shm for the specified user"
                    }
                    );
   
   parser.addOption(
                    {
                     { "s", "sleep" }
                     ,"sleep in seconds before exiting"
                     ,"sleep"
                    }
                    );
   
   parser.addOption(
                    {
                     { "S", "signal" }
                     ,"catch signals"
                    }
                    );

   parser.process(a);

   const QStringList args = parser.optionNames();
   if (args.size() < 1) {
      TSE << "Error: Must specify an argument.\n";
      parser.showHelp(1);
   }

   QString user           = qgetenv( "USER" );
   bool    testattach     = false;
   bool    createshm      = false;
   int     sleepsec       = 0;
   bool    catchsigs      = false;

   for ( int i = 0; i < (int) args.size(); ++i ) {
      QString arg = args[i];
      if ( arg == "u" ) {
         user      = parser.value( arg );
      } else if ( arg == "s" ) {
         sleepsec  = parser.value( arg ).toInt();
      } else if ( arg == "t" ) {
         testattach = true;
      } else if ( arg == "c" ) {
         createshm = true;
      } else if ( arg == "S" ) {
         catchsigs = true;
      } 
   }   

   if ( catchsigs ) {
      std::set < int > sigs = {
                               SIGABRT
                               ,SIGALRM
                               ,SIGFPE
                               ,SIGHUP
                               ,SIGILL
                               ,SIGINT
                               ,SIGPIPE
                               ,SIGQUIT
                               ,SIGSEGV
                               ,SIGTERM
                               ,SIGUSR1
                               ,SIGUSR2
      };
      for ( auto it = sigs.begin();
            it != sigs.end();
            ++it ) {
         signal( *it, signal_handler );
      }
   }

   passwd * pwinfo = getpwnam( user.toLocal8Bit().data() );

   Global        global;

   // Make the key specific to the uid
   key_t key = 0xe1000000 + pwinfo->pw_uid;
   

   if ( testattach ) {
      TSO << "testing shm attach for user " << user << "\n";

      TSO << "Key is " << key << "\n";

      // attach to existing
      {
         shmid = shmget( key
                         ,sizeof(Global)
                         ,0
                         );
         if ( shmid == -1 ) {
            TSO << "shmget (attach) failed\n";
            exit(-1);
         }
         TSO << "shmget (attach) succeeded\n";

         // verify size
         struct shmid_ds shmds;

         if ( -1 == shmctl( shmid, IPC_STAT, &shmds ) ) {
            TSO << "shmctl (attach) IPC_STAT failed\n";
            exit(-1);
         }
         TSO << "shmctl (attach) IPC_STAT succeeded\n";
         TSO << "shmctl nattch is " << shmds.shm_nattch << "\n";

         if ( shmds.shm_segsz != sizeof(Global) ) {
            TSO << "shmctl (attach) size mismatch " << shmds.shm_segsz << " vs " << sizeof(Global) << "\n";
            exit(-1);
         }

         if ( (void *)-1 == (shmbuf = shmat( shmid, 0, 0 ) ) ) {
            TSO << "shmat (attach) failed\n";
            exit(-1);
         }
         TSO << "shmat (attach) succeeded\n";

         if ( shmds.shm_nattch == 0 ) {
            TSO << "zeroing data\n";
            memset( shmbuf, 0, sizeof(Global) );
         }

         // copy data
         memcpy( (void*)&global, shmbuf, sizeof(Global) );

         // output data

         TSO << QString( "global passwd info '%1'\n" ).arg( global.passwd );
      }                 
   }

   if ( createshm ) {
      TSO << "create shm for user " << user << "\n";

      TSO << "Key is " << key << "\n";

      // create
      {
         // shm_open
         shmid = shmget( key
                         ,sizeof(Global)
                         ,IPC_CREAT | 0600
                         );
         if ( shmid == -1 ) {
            TSO << "shmget (create) failed\n";
            exit(-1);
         }
         TSO << "shmget (create) succeeded\n";
         
         struct shmid_ds shmds;
         
         if ( -1 == shmctl( shmid, IPC_STAT, &shmds ) ) {
            TSO << "shmctl (create) IPC_STAT failed\n";
            exit(-1);
         }
         TSO << "shmctl (create) IPC_STAT succedded\n";
         TSO << "shmctl nattch is " << shmds.shm_nattch << "\n";

         void *shmbuf;
         if ( (void *)-1 == (shmbuf = shmat( shmid, 0, 0 ) ) ) {
            TSO << "shmat (create) failed\n";
            exit(-1);
         }
         TSO << "shmat (create) succeeded\n";

         if ( shmds.shm_nattch == 0 ) {
            TSO << "zeroing data\n";
            memset( shmbuf, 0, sizeof(Global) );
         }

         // setup silly data
         {
            const char * sillydata = "TEST GLOBAL DATA";
            strncpy( global.passwd, sillydata, strlen( sillydata ) + 1 );
            memcpy( shmbuf, (void*)&global, sizeof(Global) );
         }

         // if ( -1 == shmctl( shmid, IPC_RMID, &shmds ) ) {
         //    TSO << "shmctl (create) IPC_RMID failed\n";
         //    exit(-1);
         // }
         // TSO << "shmctl (create) IPC_RMID succedded\n";
      }
   }

   if ( sleepsec ) {
      TSO << "sleeping for " << sleepsec << " seconds\n";
      sleep( sleepsec );
   }

   signal_handler(0);

   return 0;
}

