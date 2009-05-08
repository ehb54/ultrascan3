#include "../include/us_gridcontrol_t.h"

int main (int argc, char **argv)
{
   QString infile;
   QString gridopt = "none";
   QString system_name = "none";
   if(argc < 2)
   {
      cout << "Usage: us_gridcontrol <path/input_file>\n\n";
      return(-1);
   }
   else
   {
      infile = argv[1];
      if(argc > 2) {
         gridopt = argv[2];
      }
      if(argc > 3) {
         system_name = argv[3];
      }
   }
   QApplication a(argc, argv, QApplication::Tty);
   US_GridControl_T *gridcontrol_t;
   gridcontrol_t = new US_GridControl_T(infile, gridopt, system_name);
   return a.exec();
}
