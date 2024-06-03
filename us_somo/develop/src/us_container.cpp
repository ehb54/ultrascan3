#include "../include/us_container.h"
#include <qdebug.h>
#include <qprocess.h>
#include <qtextstream.h>

US_Container::US_Container() : QObject() {
   qDebug() << "US_Container::US_Container()";
   type = "";
}

US_Container::~US_Container() {
   qDebug() << "US_Container::~US_Container()";
}

bool US_Container::installed() {
   qDebug() << "US_Container::installed()";
   if ( !type.isEmpty() ) {
      return true;
   }
   
   static vector < QString > types =
      {
         "docker"
         ,"podman"
      };

   // QStringList environment = program.systemEnvironment();

   for ( const auto & t : types ) {
      QProcess program;
      QStringList arguments;
      program.start( t, arguments );
      bool started = program.waitForStarted();
      if (!program.waitForFinished(1000)) { // 1 Second timeout
         program.kill();
      }

      // int exitCode = program.exitCode();
      // QString stdOutput = QString::fromLocal8Bit(program.readAllStandardOutput());
      // QString stdError = QString::fromLocal8Bit(program.readAllStandardError());

      if ( started ) {
         qDebug() << "US_Container::installed() : " << t;
         type = t;
         return true;
      }
   }

   qDebug() << "US_Container::installed() : NONE!";
   return false;
}

#define TEST_PROGRAM "docker"
#define TEST_PROGRAM_TIMEOUT_SECS 36

bool US_Container::test() {
   qDebug() << "US_Container::test()";
   if ( !installed() ) {
      return false;
   }

   QProcess program;
   QStringList arguments;
   arguments << "run" << "hello-world";

   qDebug() << "US_Container::test() : starting " << TEST_PROGRAM << " " << arguments.join( " " );

   program.start( TEST_PROGRAM, arguments );

   bool started = program.waitForStarted();

   if ( !started ) {
      qDebug() << "US_Container::test() : failed to start";
      program.kill();
      return false;
   }

   qDebug() << "US_Container::test() : started";
   
   if (!program.waitForFinished( TEST_PROGRAM_TIMEOUT_SECS * 1000)) { 
      qDebug() << "US_Container::test() : failed to finish in " << TEST_PROGRAM_TIMEOUT_SECS << " seconds";
      program.kill();
      return false;
   }

   qDebug() << "US_Container::test() : finished, exit code " << program.exitCode();

   QTextStream(stdout)
      << "US_Container::test() : stdout\n"
      << "--------------------------------------------------------------------------------\n"
      << QString::fromLocal8Bit(program.readAllStandardOutput())
      << "--------------------------------------------------------------------------------\n"
      << "US_Container::test() : stderr\n"
      << "--------------------------------------------------------------------------------\n"
      << QString::fromLocal8Bit(program.readAllStandardError())
      << "--------------------------------------------------------------------------------\n"
      ;
      

   return true;
}
