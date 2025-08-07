//! \file us_predict2_main.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_predict2.h"

//! \brief Main program for US_Predict2. Loads translators and starts
//         the class US_Predict2.

int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   US_Predict2 w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}
