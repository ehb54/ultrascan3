//! \file us_protocol_dev_main.cpp

#include "us_protocol_dev_gui.h"



int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ProtocolDevMain * w = new US_ProtocolDevMain;
   w->show(); 

   w->call_AutoflowDialogue();
   
   if ( w->window_closed )
     {
       qDebug() << "Closing --- ";
       return 0;
     }
   
   return application.exec();  //!< \memberof QApplication
}

