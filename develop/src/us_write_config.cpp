#include "../include/us_write_config.h"
#include <qmessagebox.h>

// note: this program uses cout and/or cerr and this should be replaced

US_Write_Config::US_Write_Config( QObject* parent, const char* name) 
   : QObject( parent, name) 
{
}

US_Write_Config::~US_Write_Config()
{
}

bool US_Write_Config::write_config( struct Config config_list )
{
   QString warning=tr("Warning"), message;
   QDir temp_dir = config_list.help_dir;
   if ( ! temp_dir.exists() )
   {
      if ( ! temp_dir.mkdir( config_list.help_dir, true ) )
      {
         message = tr("Could not create the Help File Directory!\n\n"
                      + config_list.help_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.root_dir;
   if ( ! temp_dir.exists() )
   {
      if (!temp_dir.mkdir(config_list.root_dir, true))
      {
         message = tr("Could not create the Root Directory!\n\n"
                      + config_list.root_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.data_dir;
   if ( ! temp_dir.exists() )
   {
      if (!temp_dir.mkdir(config_list.data_dir, true))
      {
         message = tr("Could not create the Data Directory!\n\n"
                      + config_list.data_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.tmp_dir;
   if ( ! temp_dir.exists() )
   {
      if (!temp_dir.mkdir(config_list.tmp_dir, true))
      {
         message = tr("Could not create the Temporary Directory!\n\n"
                      + config_list.tmp_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.archive_dir;
   if ( ! temp_dir.exists() )
   {
      if (!temp_dir.mkdir(config_list.archive_dir, true))
      {
         message = tr("Could not create the Archive Directory!\n\n"
                      + config_list.archive_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.system_dir;
   if ( ! temp_dir.exists() )
   {
      message = tr("The UltraScan System Directory could not be found!\n\n"
                   + config_list.system_dir + "\nPlease check your settings!");
      errorMessage(warning, message);
      cerr << warning << ":\n" << message << endl;
      return false;
   }

   temp_dir = config_list.html_dir;
   if ( ! temp_dir.exists() )
   {
      if ( ! temp_dir.mkdir(config_list.html_dir, true) )
      {
         message = tr("Could not create the Reports Directory!\n\n"
                      + config_list.html_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   temp_dir = config_list.result_dir;
   if ( ! temp_dir.exists() )
   {
      if ( ! temp_dir.mkdir( config_list.result_dir, true ) )
      {
         message = tr("Could not create the Results Directory!\n\n"
                      + config_list.result_dir + "\nPlease check your write permissions!");
         errorMessage(warning, message);
         cerr << warning << ":\n" << message << endl;
         return false;
      }
   }

   QString rcfile = US_Config::get_home_dir() + USRC;
   QFile f( rcfile );
   QTextStream ts ( &f );
   
   if ( f.open( IO_WriteOnly | IO_Translate ) )
   {
      QTextStream ts ( &f );
      ts << config_list.version         << "\n";
      ts << config_list.browser         << "\n";
      ts << "tar_dummy"                 << "\n";
      ts << "zip_dummy"                 << "\n";
      ts << config_list.help_dir        << "\n";
      ts << config_list.data_dir        << "\n";
      ts << config_list.root_dir        << "\n";
      ts << config_list.archive_dir     << "\n";
      ts << config_list.result_dir      << "\n";
      ts << config_list.beckman_bug     << "\n";
      ts << config_list.temperature_tol << "\n";
      ts << config_list.html_dir        << "\n";
      ts << config_list.system_dir      << "\n";
      ts << config_list.fontFamily      << "\n";
      ts << config_list.fontSize        << "\n";
      ts << config_list.margin          << "\n";
      ts << config_list.numThreads      << "\n";
      ts << config_list.tmp_dir         << "\n";
      f.close();
   }
   else
   {
      message = tr( "Could not open Configuration File for update.\n\n"
                    "$HOME/ultrascan/etc/usrc.conf\nPlease check your write permissions!" );
      errorMessage( warning, message );
      cerr << warning << ":\n" << message << endl;
      return false;
   }
   return true ;
}

void US_Write_Config::errorMessage( QString& str1, QString& str2 )
{
   QMessageBox::message( str1, str2 ); 
}

