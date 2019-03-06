//! \file us_aprof_main.cpp

#include "us_analysis_profile.h"

//! \brief Main program for US_AnalysisProfile. Loads translators and starts
//         the class US_AnalysisProfile

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_AnalysisProfile * w = new US_AnalysisProfile;
   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

