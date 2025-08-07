//! \file us_solution_main.cpp

#include <QApplication>

#include "us_license.h"
#include "us_license_t.h"
#include "us_solution_gui.h"

//! \brief Main program for US_Solution. Loads translators and starts
//    the class US_Solution.

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.
  US_SolutionGui* w = new US_SolutionGui;
  w->show();                  //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}
