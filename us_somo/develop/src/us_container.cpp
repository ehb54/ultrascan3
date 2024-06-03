#include "../include/us_container.h"
#include <qdebug.h>
#include <qprocess.h>

US_Container::US_Container() : QObject() {
   qDebug() << "US_Container::US_Container()";
   type = "";
}

US_Container::~US_Container() {
   qDebug() << "US_Container::~US_Container()";
}

bool US_Container::containers_supported() {
   qDebug() << "US_Container::containers_supported()";
   static vector < QString > types =
      {
         "docker"
         ,"podman"
      };

   // QStringList environment = program.systemEnvironment();

   for ( const auto & t : types ) {
      QProcess program;
      program.start( t );
      bool started = program.waitForStarted();
      if (!program.waitForFinished(1000)) { // 1 Second timeout
         program.kill();
      }

      // int exitCode = program.exitCode();
      // QString stdOutput = QString::fromLocal8Bit(program.readAllStandardOutput());
      // QString stdError = QString::fromLocal8Bit(program.readAllStandardError());

      if ( started ) {
         qDebug() << "US_Container::containers_supported() : " << t;
         type = t;
         return true;
      }
   }

   qDebug() << "US_Container::containers_supported() : NONE!";
   return false;
}
