// shared memory utility

#include <QtCore>
#include <QSharedMemory>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

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
   parser.setApplicationDescription("Description: shmutil helps diagnose QSharedMemory used");
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

   QString user       = qgetenv( "USER" );
   bool    testattach = false;
   bool    createshm  = false;
   int     sleepsec   = 0;

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

   QSharedMemory shm;
   Global        global;

#ifndef Q_OS_WIN
  // Make the key specific to the uid
  QString key = QString( "UltraScan%1" ).arg( pwinfo->pw_uid );
#else
  QString key = QString( "UltraScan" );
#endif

   if ( testattach ) {
      TSO << "testing shm attach for user " << user << "\n";
      TSO << "Key is " << key << "\n";

      shm.setKey( key );
      if ( shm.attach() ) {
         TSO << "attach successful\n";
         if ( shm.detach() ) {
            TSO << "detach successful\n";
         } else {
            TSO << "detach failed: " << shm.errorString() << "\n";
         }
      } else {
         TSO << "attach failed: " << shm.errorString() << "\n";
      }
   }

   if ( createshm ) {
      TSO << "create shm for user " << user << "\n";
      TSO << "Key is " << key << "\n";

      shm.setKey( key );
      if ( shm.create( sizeof( global ) ) ) {
         TSO << "create shm successful\n";
      } else {
         TSO << "create shm failed: " << shm.errorString() << "\n";
      }
   }

   if ( sleepsec ) {
      TSO << "sleeping for " << sleepsec << " seconds\n";
      sleep( sleepsec );
   }

   return 0;
}

