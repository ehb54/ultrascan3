#include "../include/us_hydrodyn.h"
#include "../include/us_register.h"
#include <qregexp.h>

int main (int argc, char **argv)
{
   QDir *dir = new QDir(QDir::currentDirPath());
   QApplication a(argc, argv);
   US_Register *us_register;
   us_register = new US_Register();

   if(us_register->read())      //this is a correct file
   {
      bool expert = false;
      bool debug = false;
      bool residue_file = false;
      QString residue_filename;
      delete us_register;
      US_Hydrodyn *hydrodyn;
      vector < QString > batch_file;
      int argcbase = 1;
      while ( a.argc() >= argcbase &&
           QString(a.argv()[argcbase]).contains(QRegExp("^-")) )
      {
         if ( QString(a.argv()[argcbase]).contains(QRegExp("^-e")) )
         {
            puts("expert mode");
            argcbase++;
            expert = true;
         }
         if ( QString(a.argv()[argcbase]).contains(QRegExp("^-d")) )
         {
            puts("debug mode");
            argcbase++;
            debug = true;
         }
         if ( QString(a.argv()[argcbase]).contains(QRegExp("^-r")) )
         {
            puts("residue file");
            argcbase++;
            residue_file = true;
            residue_filename = a.argv()[argcbase];
            argcbase++;
         }
      }
      for ( int i = argcbase; i < a.argc(); i++ ) 
      {
         batch_file.push_back( dir->filePath(a.argv()[i]) );      
      }
      hydrodyn = new US_Hydrodyn(batch_file);
      hydrodyn->setCaption("SOMO Solution Modeler");
      if ( residue_file )
      {
         hydrodyn->residue_filename = residue_filename;
         hydrodyn->read_residue_file();
         hydrodyn->lbl_table->setText( QDir::convertSeparators( residue_filename ) );
      }
      if ( expert )
      {
         hydrodyn->advanced_config.auto_view_pdb = false;
         hydrodyn->advanced_config.scroll_editor = true;
         hydrodyn->advanced_config.expert_mode = true;
      }
      if ( debug )
      {
         hydrodyn->advanced_config.debug_1 = true;
         hydrodyn->advanced_config.debug_2 = true;
         hydrodyn->advanced_config.debug_3 = true;
         hydrodyn->advanced_config.debug_4 = true;
         hydrodyn->advanced_config.debug_5 = true;
         hydrodyn->advanced_config.debug_6 = true;
         hydrodyn->advanced_config.debug_7 = true;
      }
         
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
