//! \file us_reporter_gmp_main.cpp

#include "us_reporter_gmp.h"
#include "us_license_t.h"
#include "us_license.h"


//! \brief Main program for US_Analysis_auto. Loads translators and starts
//         the class US_Analysis_auto

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   //QMap<QString,QString> t_c;
   //US_ReporterGMP * w = new US_ReporterGMP( t_c );
   US_ReporterGMP * w = new US_ReporterGMP;
   //US_ReporterGMP * w = new US_ReporterGMP( "AUTO" );
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

