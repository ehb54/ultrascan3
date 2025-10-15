#include "us_fematch.h"
#include "us_license_t.h"
#include "us_license.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_FeMatch w;
   qDebug() << "US_FeMatch::main()" << w.pos() << w.mapToGlobal(w.pos()) << w.frameGeometry() << w.size();
   w.show();                   //!< \memberof QWidget
   qDebug() << "US_FeMatch::main()" << w.pos() << w.mapToGlobal(w.pos()) << w.frameGeometry() << w.size();
   w.move(300,300);
   qDebug() << "US_FeMatch::main()" << w.pos() << w.mapToGlobal(w.pos()) << w.frameGeometry() << w.size();

   return application.exec();  //!< \memberof QApplication
}
