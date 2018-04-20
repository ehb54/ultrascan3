//! \file us_experiment_main.cpp

#include "us_experiment_gui.h"


//! \brief Main program for US_ExperimentMain. Loads translators and starts
//         the class US_ExperimentMain

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ExperimentMain w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

