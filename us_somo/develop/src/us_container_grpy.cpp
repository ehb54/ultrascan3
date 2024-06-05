#include "../include/us_container_grpy.h"

#include <qdebug.h>

#define PULL_TIMEOUT_SECS 120

US_Container_Grpy::US_Container_Grpy( bool pull, bool do_not_use_container ) {
   qDebug() << "US_Container_Grpy::US_Container_Grpy()";

   if ( usc.installed() && !do_not_use_container ) {
      image_name =
         QString( "ehb1/grpy:" )
         + ( US_Arch::is_arm()
             ? "arm64"
             : "amd64"
             )
         ;
      if ( pull ) {
         QProcess program;
         QStringList arguments;
         arguments << "pull" << image_name;

         qDebug() << "US_Container_Grpy::US_Container_Grpy() : pull : starting " << usc.type() << arguments.join( " " );

         program.start( usc.type(), arguments );

         bool started = program.waitForStarted();

         if ( !started ) {
            qDebug() << "US_Container_Grpy::US_Container_Grpy() : pull : failed to start";
            image_name = ""; // no retry?
         }

         if ( !image_name.isEmpty() ) {
            qDebug() << "US_Container_Grpy::US_Container_Grpy() : pull : started";
   
            if (!program.waitForFinished( PULL_TIMEOUT_SECS * 1000)) { 
               qDebug() << "US_Container_Grpy::US_Container_Grpy() : pull : failed to finish in " << PULL_TIMEOUT_SECS << " seconds";
               image_name = ""; // no retry?
            }
         }
         
         if ( !image_name.isEmpty() ) {
            qDebug() << "US_Container::US_Container_Grpy() : pull : finished, exit code " << program.exitCode();

            QTextStream(stdout)
               << "US_Container::US_Container_Grpy() : pull : stdout\n"
               << "--------------------------------------------------------------------------------\n"
               << QString::fromLocal8Bit(program.readAllStandardOutput())
               << "--------------------------------------------------------------------------------\n"
               << "US_Container::US_Container_Grpy() : pull : stderr\n"
               << "--------------------------------------------------------------------------------\n"
               << QString::fromLocal8Bit(program.readAllStandardError())
               << "--------------------------------------------------------------------------------\n"
               ;
         }
      }
   } else {
      image_name = "";
   }
   USglobal = new US_Config();
}

US_Container_Grpy::~US_Container_Grpy() {
   qDebug() << "US_Container_Grpy::~US_Container_Grpy()";
}


QString US_Container_Grpy::executable() {
   qDebug() << "US_Container_Grpy::executable()";

   QString result;

   if ( image_name.isEmpty() ) {
      result =
         USglobal->config_list.system_dir + QDir::separator() +
#if defined(BIN64)
         "bin64"
#else
         "bin"
#endif
         + QDir::separator()
         + "GRPY"
#if defined(Q_OS_WIN)
         + "_win64.exe"
#else
# if defined(Q_OS_MAC)
         + "_osx10.11"
# else
         + "_linux64"
# endif
#endif
         ;
   } else {
      result = usc.type();
   }
   

   qDebug() << "US_Container_Grpy::executable() : " << result;
   return result;
}

QStringList US_Container_Grpy::arguments( QString directory ) {
   QStringList result;
   if ( !image_name.isEmpty() ) {
      // generate a new container name
      container_names.push_back( usc.unique_name( "grpy" ) );
      result
         << "run"
         << "--rm"
         << QString( "--cpus=%1" ).arg( USglobal->config_list.numThreads )
         << "--name"
         << container_names.back()
         << "-v"
         << QString( "%1:/work" ).arg( directory.isEmpty() ? QDir::currentPath() : directory )
         << image_name
         ;
   }
   
   qDebug() << "US_Container_Grpy::arguments() : " << result.join( " : " );
   return result;
}
