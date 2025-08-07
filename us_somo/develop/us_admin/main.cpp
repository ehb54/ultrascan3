#include <qapplication.h>

#include "../include/us_admin.h"
#include "../include/us_register.h"

int main(int argc, char **argv) {
  QApplication a(argc, argv);
  US_Register *us_register;
  us_register = new US_Register();

  if (us_register->read())  // this is a correct file
  {
    delete us_register;
    US_Admin *us_admin;
    us_admin = new US_Admin();
    us_admin->setWindowTitle("Administrator Password");
    us_admin->show();
    //      a.setMainWidget(us_admin);
    a.setDesktopSettingsAware(false);
  } else  // problem with license (either missing, incorrect or expired)
  {
    //      a.setMainWidget(us_register);
    a.setDesktopSettingsAware(false);
  }
  return a.exec();
}
