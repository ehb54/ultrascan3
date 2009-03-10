//! \file us_db_tbl_investigator_main.cpp

#include <QApplication>

#include "us_db_tbl_investigator.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_DB_TblInvestigator. Loads translators and starts
//    the class US_DB_TblInvestigator.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );
  
   #include "main1.inc"

   // License is OK.  Start up.
   US_DB_TblInvestigator* w = new US_DB_TblInvestigator;
   w->show();                  //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}
