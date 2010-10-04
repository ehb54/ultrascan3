#include <QApplication>

#include "us_model_gui.h"

//! \brief Main program for US_ModelGui. Loads translators and displays
//         the window.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   US_Model     m;
   US_ModelGui* w = new US_ModelGui( m );

   w->show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication

}

