#include <qapplication.h>
#include "../include/us_win.h"
#include "../include/us_register.h"

// Helper class used only here
// The purpose is to ensure the usrc.conf or the depricated $HOME/.usrc exists

// If not, create a default usrc.conf
// The main issue is to determine the location of the UltraScan system files
// These are found by (in order) 
// 1. ULTRASCAN environment variable
// 2. Checking a default location
// 3. Ask the user

class USconfig_check : public QWidget
{
	public:
    USconfig_check(){};
		~USconfig_check(){};

		struct Config config;

		bool check_config();

	protected:
    bool exists       ( const QString& );
		void set_default  ( const QString& );
		void write_default( const QString& );

};

class USconfig_setup : public USconfig_check
{
	public:
		USconfig_setup();
		~USconfig_setup(){};
};


//////////////
int main (int argc, char **argv)
{
	// These are *not* gloabal
	int global_Xpos = 50;
	int global_Ypos = 50;

 	QApplication a( argc, argv );
	QTranslator translator( 0 );
	translator.load( "../us/us_", "." );
	a.installTranslator( &translator );
	
	US_Register* us_register = new US_Register();

	if ( us_register->read() )  // Is this a correct license file?
	{
		delete us_register;

		// Check to see if we are configured
		USconfig_check* check = new USconfig_check();
		bool OK               = check->check_config();
		delete check;

		// Check that we know at least the base system directory
    if ( ! OK )
		{
			cout << "Starting USconfig_set()" << endl;
      USconfig_setup* set = new USconfig_setup();
			// Not found, ask the user
			set->move( global_Xpos, global_Ypos );
			set->show();
			a.setMainWidget( set );
		}
    else  // OK to continue
		{
			UsWin* us = new UsWin();
			us->setCaption( "UltraScan Analysis" );
			us->move( global_Xpos, global_Ypos );
			us->show();
			a.setMainWidget( us );
		}
	}
	else		
	{
		a.setMainWidget( us_register );
	}

	a.setDesktopSettingsAware( false );
	return a.exec();
}

bool USconfig_check::check_config()
{
	// If we have a config file, retrun true
	if ( exists( US_Config::get_home_dir() + ETC_DIR + "/usrc.conf" ) ||
			 exists( QDir::homeDirPath() + "/.usrc" ) )
	{
		cout << "config file exists" << endl;
		return true;
	}

  // Check the default location
#ifdef UNIX
	QString dir = "/usr/local/ultrascan";
#endif

#ifdef WIN32
	QString dir = "C:/Program Files/UltraScan";
#endif

	if ( exists ( dir + "/etc/ultra.xpm" ) )
	{
		// If we got here,  ETC_DIR + "/usrc.conf" does not yet exist
    write_default( dir );

		cout << "Ultrascan is installed in the default location: " + dir << endl;
		return true;
	}

	// Check the environment variable
  QString ultrascan = getenv( "ULTRASCAN" );
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
  bool OK = w_config->write_config( config );
	delete w_config;

	if ( ! OK )
	{
		QMessageBox::information( this,
		  tr( "Setup" ),
		  tr( "Could not create configuration file.  Aborting." ) );

		exit ( -1 );
	}
}

void USconfig_check::set_default( const QString& system_dir )
{
	config.version         = US_Version_string;  // Defined in us_util.h

	config.browser         = "/usr/bin/firefox";
	config.tar             = "/bin/tar";
	config.gzip            = "/bin/gzip";

#ifdef WIN32
	config.browser         = "C:\\Program Files\\Internet Explorer\\iexplore.exe";
	config.tar             = "tar";
	config.gzip            = "gzip";
#endif

	// Set up system directories
	config.system_dir      = QDir::convertSeparators( system_dir );
	config.help_dir        = QDir::convertSeparators( system_dir + "/doc" );

	// Set the per user directory and subdirectories
  QString root_dir       = US_Config::get_home_dir();

	config.root_dir        = root_dir;
	config.data_dir        = QDir::convertSeparators( root_dir + "data" );
	config.archive_dir     = QDir::convertSeparators( root_dir + "archive" );
	config.result_dir      = QDir::convertSeparators( root_dir + "results" );
	config.html_dir        = QDir::convertSeparators( root_dir + "reports" );

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
		tr( "Setup Error" ), 
		tr( "You need to specify the location of the UltraScan\n" 
		    "system directory") );

  int     result = QMessageBox::Ok;
	QString msg    = "Configuration initialized.  Please restart to continue.";

	do
	{
		// Ask for the directory
	  QString dir = QFileDialog::getExistingDirectory(
			QString::null, this, NULL, "Select the UltraScan System Directory" );
	
		// Check it
	  if ( exists( dir + "/etc/ultra.xpm" ) )
		{
			// Found.  Create the config file.
			write_default( dir );
			break;
		}

		// Not a valid directory
		QMessageBox::information( this,
			tr( "Setup" ),
			tr( "The selected directory is not the UltraScan system diredtory.\n"
			    "Try again." ),
			QMessageBox::Ok, QMessageBox::Cancel );

	} while ( result != QMessageBox::Cancel );

  if ( result ==  QMessageBox::Cancel )
	{
		msg = "System directory not found.  Aborting.";
	}

	// Show results
	QMessageBox::information( this,
	   tr( "Setup" ),
	   tr( msg ) );

	exit(0);
}



