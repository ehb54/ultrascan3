#include "../include/us_hydrodyn.h"
#include "../include/us_register.h"

int main (int argc, char **argv)
{
   QDir *dir = new QDir(QDir::currentDirPath());
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      delete us_register;
      US_Hydrodyn *hydrodyn;
      vector < QString > batch_file;
      for ( int i = 1; i < a.argc(); i++ ) 
      {
         batch_file.push_back( dir->filePath(a.argv()[i]) );      
      }
      hydrodyn = new US_Hydrodyn(batch_file);
      hydrodyn->setCaption("SOMO Solution Modeler");
      hydrodyn->show();
      a.setMainWidget(hydrodyn);
      a.setDesktopSettingsAware(false);
   }
   else     // problem with license (either missing, incorrect or expired)
   {
      a.setMainWidget(us_register);
      a.setDesktopSettingsAware(false);
   }
   return a.exec();
}
