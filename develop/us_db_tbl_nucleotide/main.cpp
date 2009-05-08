#include "../include/us_db_tbl_nucleotide.h"
#include "../include/us_register.h"
#include <qapplication.h>

int main (int argc, char **argv)
{
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      US_Nucleotide_DB *us_nucleotide_db;
      bool from_cell = false;
      us_nucleotide_db = new US_Nucleotide_DB(from_cell, -1);
      us_nucleotide_db->show();
      a.setMainWidget(us_nucleotide_db);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
