// shared memory utility using shm_open

#include <QtCore>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <set>
#include <csignal>

// includes required by shm_open ?
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TSO QTextStream( stdout )
#define TSE QTextStream( stderr )


// update if US_Global::Global changes structure
class Global
{
public:
   QPoint current_position;
   char   passwd[64];
   // Add other global values as necessary
};

int main(int argc, char *argv[])
{
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
      } 
   }   

   passwd * pwinfo = getpwnam( user.toLocal8Bit().data() );

   Global        global;

   // Make the key specific to the uid
   QString key = QString( "UltraScan%1" ).arg( pwinfo->pw_uid );

   if ( testattach ) {
      TSO << "testing shm attach for user " << user << "\n";

      TSO << "Key is " << key << "\n";

      // attach to existing
      {
         // shm_open
         int fd = shm_open(
                           key.toLocal8Bit().data()
                           ,O_RDWR
                           ,0
                           );
         if ( fd == -1 ) {
            TSO << "shm_open failed\n";
            exit(-1);
         }
         TSO << "shm_open succeeded\n";

         // verify size

         {
            struct stat s;
            if ( fstat( fd, &s ) == -1 ) {
               TSO << "fstat failed\n";
               exit(-1);
            }
            if ( s.st_size != sizeof( Global ) ) {
               TSO << "shm_open has incorrect size!\n";
               exit(-1);
            }
            TSO << "shm_open has correct size\n";
         }

         // map memory
         Global *shmp = (Global *) mmap(
                                        NULL
                                        ,sizeof(Global)
                                        ,PROT_READ | PROT_WRITE
                                        ,MAP_SHARED
                                        ,fd
                                        ,0
                                        );
         if (shmp == MAP_FAILED) {
            TSO << "mmap failed\n";
            exit(-2);
         }
         TSO << "mmap succeeded\n";
         // copy data
         memcpy( (void*)&global, shmp, sizeof(Global) );

         // output data

         TSO << QString( "global passwd info '%1'\n" ).arg( global.passwd );
      }                 
   }

   if ( createshm ) {
      TSO << "create shm for user " << user << "\n";

      TSO << "Key is " << key << "\n";

      // create new
      {
         // shm_open
         int fd = shm_open(
                           key.toLocal8Bit().data()
                           ,O_CREAT | O_EXCL | O_RDWR
                           ,S_IRUSR | S_IWUSR
                           );
         if ( fd == -1 ) {
            TSO << "shm_open create failed\n";
            exit(-1);
         }
         TSO << "shm_open succeeded\n";
         // truncate
         if ( ftruncate(fd, sizeof(Global)) == -1 ) {
            TSO << "shm_open create truncate failed\n";
            exit(-1);
         }
         // map memory
         Global *shmp = (Global *) mmap(
                                        NULL
                                        ,sizeof(Global)
                                        ,PROT_READ | PROT_WRITE
                                        ,MAP_SHARED
                                        ,fd
                                        ,0
                                        );
         if (shmp == MAP_FAILED) {
            TSO << "mmap failed\n";
            exit(-2);
         }
         TSO << "mmap succeeded\n";
         // setup silly data
         {
            const char * sillydata = "TEST GLOBAL DATA";
            strncpy( global.passwd, sillydata, strlen( sillydata ) + 1 );
            memcpy( shmp, (void*)&global, sizeof(Global) );
         }
         TSO << "mmap succeeded\n";
         // setup the shared memory to empty
         shm_unlink(key.toLocal8Bit().data());
      }

   }

   if ( sleepsec ) {
      TSO << "sleeping for " << sleepsec << " seconds\n";
      sleep( sleepsec );
   }

   return 0;
}

