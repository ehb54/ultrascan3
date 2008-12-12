#include "us_license.h"

int main( int argc, char* argv[] )
{
  QApplication application( argc, argv );
  US_License w;
  w.show();
  return application.exec();
}
