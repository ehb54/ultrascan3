#include "../include/us_arch.h"

#include <qdebug.h>
#include <qprocess.h>
#include <qstring.h>
#include <qstringlist.h>

bool US_Arch::is_arm() {
#if 0 && !defined( Q_OS_OSX )
   return false;
#else 
   QProcess program;

   QStringList arguments;
   arguments << "-p";

   program.start( "uname", arguments );

   bool started = program.waitForStarted();

   if ( !started ) {
      qDebug() << "US_Arch::is_arm() : failed to start";
      program.kill();
      return false;
   }

   if (!program.waitForFinished(2000)) { // 2 Second timeout
      qDebug() << "US_Arch::is_arm() : failed to finish in 2 seconds";
      program.kill();
      return false;
   }
   
   QString uname_p = QString::fromLocal8Bit(program.readAllStandardOutput()).trimmed();

   QTextStream(stdout) << "US_Arch::is_arm() : finished : returned '" << uname_p << "'" << "\n";

   return uname_p == "arm";
#endif
}
