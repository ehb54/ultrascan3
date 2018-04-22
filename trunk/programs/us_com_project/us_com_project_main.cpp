//! \file us_experiment_main.cpp

#include "us_com_project_gui.h"



int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ComProjectMain w;
   //w.setFrameShape( QFrame::Box);
   //w.setLineWidth(1);
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

