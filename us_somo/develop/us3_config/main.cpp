#include <qapplication.h>
//#include <q3textstream.h>
#include "../include/us_write_config.h"
#include "../include/us_version.h"
#include "../include/us_config_gui.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// Helper class used only here
// The purpose is to ensure the usrc.conf or the depricated $HOME/.usrc exists

// If not, create a default usrc.conf
// The main issue is to determine the location of the UltraScan system files
// These are found by (in order) 
// 1. Checking a default location
// 2. ULTRASCAN environment variable
// 3. Ask the user

class USconfig_check : public QWidget
{
public:
   USconfig_check()  {};
   ~USconfig_check() {};

   //Class variable
   struct Config config;

   // Public method
   bool        check_config();
   static void debug       ( const QString& );

protected:
   bool exists             ( const QString& );
   void set_default        ( const QString& );
   void write_default      ( const QString& );
};

class USconfig_setup : public USconfig_check
{
public:
   USconfig_setup();
   ~USconfig_setup(){};
};


//////////////
int main ( int argc, char **argv )
{
   // These are *not* gloabal
   int global_Xpos = 50;
   int global_Ypos = 50;
   QLocale::setDefault( QLocale::c() );
   
	QApplication a( argc, argv );
   // Check to see if we are configured
	QString etcdir = US_Config::get_home_dir() + "/etc";
	QDir dir1;
	dir1.mkdir(etcdir );
   USconfig_check* check = new USconfig_check();
   bool            OK    = check->check_config();
   delete check;

   // Check that we know at least the base system directory
   if ( ! OK )
   {
      USconfig_check::debug( "Starting USconfig_setup()" );
      USconfig_setup* set = new USconfig_setup();
      // Not found, ask the user
      set->move( global_Xpos, global_Ypos );
      set->show();
 //      a.setMainWidget( set );
   }
   else  // OK to continue
   {
      US_Config_GUI *configuration;
      configuration = new US_Config_GUI();
      configuration->show();
 //      a.setMainWidget(configuration);
      a.setDesktopSettingsAware(false);
#if QT_VERSION >= 0x040000
      {
         QString icon = 
            configuration->USglobal->config_list.system_dir + "/etc/" + "somo3_config_128x128.ico";
         if ( QFile( icon ).exists() )
         {
            a.setWindowIcon( QIcon( icon ) );
         }
      }
#endif

      return a.exec();
   }
   return 0;
}

void USconfig_check::debug( const QString& str )
{
   //QFile f( "debug.txt" );
   //f.open( IO_WriteOnly | IO_Append );
   //QTextStream debug( &f );
   //debug << str << "\r\n";
   //f.close();
   cout << str << "\n";
}

bool USconfig_check::check_config()
{
   // If the $HOME/ultrascan/etc directory doesn't exist, create it:
   if ( !exists( US_Config::get_home_dir() + "ultrascan/etc"))
   {
      QDir etcdir;
      QString path = US_Config::get_home_dir() + "ultrascan/etc";
      cout <<  path << endl;
      etcdir.mkdir(path );
   }
   // If we have a config file, return true
   if ( exists( US_Config::get_home_dir() + USRC ) )
   {
      US_Config* config = new US_Config( "dummy" );
      if ( config->read() )
      {
         return true;
      }
      else
      {
         int result = QMessageBox::information( this,
         us_tr( "Setup" ),
         us_tr( "The configuration file format is out of date.\n"
         "Selecting OK will delete it and recreate a valid\n"
         "configuration file, but any customized settings will be lost." ),
         QMessageBox::Ok, QMessageBox::Cancel );

         if ( result == QMessageBox::Cancel ) exit( 0 );

         // Delete the configuration file
         QFile::remove( US_Config::get_home_dir() + USRC );
         QFile::remove( QDir::homePath() + "/.usrc" );
      }
   }

   // Check the default location
   QString dir;
#ifdef UNIX
   dir = "/usr/local/ultrascan";
#endif

#ifdef WIN32
   dir = "C:/Program Files/UltraScan";
#endif

#ifdef MAC
   dir = "/Applications/UltraScanII";
#endif

   if ( exists ( dir + "/etc" ) )
   {
      // If we got here,  ETC_DIR + "/usrc.conf" does not yet exist
      debug( "Writing " + dir + "/" + QString( USRC ) + " with system dir=" + dir );
      write_default( dir );

      debug( "UltraScan is installed in the default location: " + dir );
      return true;
   }

   debug( "system dir=" + dir + "/etc/ultra.xpm not found" );
   // Check the environment variable
   QString ultrascan = getenv( "ULTRASCAN" );
   debug( "returning exists( " + ultrascan + "/etc/ultra.xpm" );
   return  exists( ultrascan + "/etc/ultra.xpm" ); 
}

bool USconfig_check::exists( const QString& file )
{
   QFile f( file );
   return f.exists();
}

void USconfig_check::write_default( const QString& dir )
{
   set_default( dir );
   US_Write_Config* w_config = new US_Write_Config( this );
   bool             OK       = w_config->write_config( config );
   delete w_config;

   if ( ! OK )
   {
      QMessageBox::information( this,
                                us_tr( "Setup" ),
                                us_tr( "Could not create configuration file.  Aborting." ) );

      exit ( -1 );
   }
}

void USconfig_check::set_default( const QString& system_dir )
{
   config.version         = US_Version_string;  // Defined in us_util.h

#if defined(WIN32)
   config.browser         = "C:\\Program Files\\Internet Explorer\\iexplore.exe";
#elif defined(MAC)
   config.browser         = "/Applications/Safari.app";
#else
   config.browser         = "/usr/bin/firefox";
#endif

   // Set up system directories
   config.system_dir      = QDir::toNativeSeparators( system_dir );
   config.help_dir        = QDir::toNativeSeparators( system_dir + "/doc" );

   // Set the per user directory and subdirectories
   QString root_dir       = US_Config::get_home_dir();

   config.root_dir        = root_dir;
   config.data_dir        = QDir::toNativeSeparators( root_dir + "data" );
   config.archive_dir     = QDir::toNativeSeparators( root_dir + "archive" );
   config.result_dir      = QDir::toNativeSeparators( root_dir + "results" );
   config.html_dir        = QDir::toNativeSeparators( root_dir + "reports" );
   config.tmp_dir         = QDir::toNativeSeparators( root_dir + "tmp" );

   config.temperature_tol = 0.5;  // Allow a max of 0.5 degrees variation
                                  // over course of run
   config.beckman_bug     = 0;    // Default: False, time doesn't have to
                                  // multiplied by 10
   config.fontFamily      = "Helvetica";
   config.fontSize        = 10;
   config.margin          = 10;
   config.numThreads      = 1;   // Default: 1 thread
}

USconfig_setup::USconfig_setup()
{
   QMessageBox::information( this, 
                             us_tr( "Setup Error" ), 
                             us_tr( "You need to specify the location of the UltraScan\n" 
                                 "system directory") );

   int     result = QMessageBox::Ok;
   QString msg    = "Configuration initialized.  Please restart to continue.";

   do
   {
      // Ask for the directory
      QString dir = QFileDialog::getExistingDirectory( this , "Select the UltraScan System Directory" , QString() );

   
      // Check it
      if ( exists( dir + "/etc" ) )
      {
         // Found.  Create the config file.
         write_default( dir );
         break;
      }

      // Not a valid directory
      result = QMessageBox::information( this,
                                         us_tr( "Setup" ),
                                         us_tr( "The selected directory is not the UltraScan system directory.\n"
                                             "Try again." ),
                                         QMessageBox::Ok, QMessageBox::Cancel );

   } while ( result != QMessageBox::Cancel );

   if ( result ==  QMessageBox::Cancel )
   {
      msg = "System directory not found.  Aborting.";
   }

   // Show results
   QMessageBox::information( this,
                             us_tr( "Setup" ),
                             us_tr( msg ) );

   exit(0);
}
