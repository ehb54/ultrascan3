#include "../include/us_register.h"
#include "../include/us_license.h"
#include "qdatetime.h"
//Added by qt3to4:
#include <QTextStream>
#include <QFrame>

//! Constructor
/*! 
  Constractor a new <var>US_Register</var> class, 
  \param p Parent widget.
  \param name Widget name.
*/  
US_Register::US_Register(QWidget *p, const char *) : QFrame( p )
{
  // The following checks the old license location and moves
  // files if necessary.

  QString    d1;
  US_Config* custom = new US_Config( d1 );
  
  QFile uslicense( QDir::homePath() + "/.uslicense" );
  if ( uslicense.exists() ) custom->move_files();
}

//! Destructor
/*! destroy <var>US_Register</var>. */
US_Register::~US_Register()
{
}

void US_Register::us_license()
{
#if QT_VERSION < 0x040000
  proc = new QProcess(this);
# ifndef Q_OS_MAC
  proc->addArgument("us_license");
# else
  US_Config* USglobal = new US_Config();
  QString basedir = USglobal->config_list.system_dir;
  if ( basedir == ""  ||  ! QFile( basedir ).exists() )
    basedir = "/Applications/UltraScanII";
  QString applic  = basedir + "/bin/us_license.app";
  if ( ! QFile( applic ).exists() )
    applic  = "/Applications/UltraScanII/bin/us_license.app";
  if ( ! QFile( applic ).exists() )
    applic  = QDir::homePath() + "/ultrascan2/bin/us_license.app";
  if ( ! QFile( applic ).exists() )
    applic  = QDir::homePath() + "/ultrascan/bin/us_license.app";
  proc->addArgument("open");
  proc->addArgument("-a");
  proc->addArgument( applic);
# endif
  
  if ( ! proc->start() ) // Error
  {
    US_Static::us_message(
        us_tr( "Please note:" ), 
        us_tr( "There was a problem creating a sub process\n"
            "for US_LICENSE\n\n"
            "Please check and try again..." ) );
    return;
  }
#else
  {
     QProcess * process = new QProcess( this );
     QString prog = "us_license";
     QStringList args;
# ifdef Q_OS_MAC
     US_Config* USglobal = new US_Config();
     QString basedir = USglobal->config_list.system_dir;
     if ( basedir == ""  ||  ! QFile( basedir ).exists() )
        basedir = "/Applications/UltraScanII";
     QString applic  = basedir + "/bin/us_license.app";
     if ( ! QFile( applic ).exists() )
        applic  = "/Applications/UltraScanII/bin/us_license.app";
     if ( ! QFile( applic ).exists() )
        applic  = QDir::homePath() + "/ultrascan2/bin/us_license.app";
     if ( ! QFile( applic ).exists() )
        applic  = QDir::homePath() + "/ultrascan/bin/us_license.app";
     args
        << "-a"
        << applic
        ;
     prog = "open";
# endif

     if ( !process->startDetached( prog, args ) ) {
        US_Static::us_message(
                             us_tr( "Please note:" ), 
                             us_tr( "There was a problem creating a sub process\n"
                                 "for US_LICENSE\n\n"
                                 "Please check and try again..." ) );
        return;
     }
  }
#endif  
}

/*!
  Read license file to check the user is legal or not.
  \return <tt>True</tt>, the license is valid. \n
  \return <tt>False</tt>, the license is invalid.
*/
bool US_Register::read()
{
  QString temp_license;
  QString str;
  bool    flag = false;

  QString lcfile = US_Config::get_home_dir() + USLICENSE;

  QFile f( lcfile );  

// Identify license file
// Does license file exists?
//  yes: continue
//  no: update instruction and return false
// is license file correct?
//    yes: is it expired?
//      yes: update instruction and return false
//      no: return true
//    no: update instruction and return false
// no: update instruction and return false
  
  if ( f.open( QIODevice::ReadOnly ) )
  {
    QTextStream ts ( &f );
    register_list.lastname = ts.readLine();
    register_list.lastname = register_list.lastname.trimmed();
    
    register_list.firstname = ts.readLine();
    register_list.firstname = register_list.firstname.trimmed();
    
    register_list.company = ts.readLine();
    register_list.company = register_list.company.trimmed();
    
    register_list.address = ts.readLine();
    register_list.address = register_list.address.trimmed();
    
    register_list.city = ts.readLine();
    register_list.city = register_list.city.trimmed();
    
    register_list.state = ts.readLine();
    register_list.state = register_list.state.trimmed();
    
    register_list.zip = ts.readLine();
    register_list.zip = register_list.zip.trimmed();
    
    register_list.phone = ts.readLine();
    register_list.phone = register_list.phone.trimmed();
    
    register_list.email = ts.readLine();
    register_list.email = register_list.email.trimmed();
    
    register_list.platform = ts.readLine();
    register_list.platform = register_list.platform.trimmed();

#ifdef SPARC
#define PLATFORM "sparc"
#define TITLE    "Sun Sparc"
#endif

#if defined(INTEL) || defined(WIN32) 
#define PLATFORM "intel"
#define TITLE    "Intel/AMD 32-bit"
#endif

#ifdef OPTERON
#define PLATFORM "opteron"
#undef TITLE
#define TITLE    "Intel/AMD 64-bit"
#endif

#ifdef MAC
#ifdef PLATFORM
#undef PLATFORM
#endif
#ifdef TITLE
#undef TITLE
#endif
#define PLATFORM "mac"
#define TITLE    "Macintosh"
#endif

#ifdef SGI
#define PLATFORM "sgi"
#define TITLE    "Silicon Graphics"
#endif
	QString selected_platform;
	if (register_list.platform == "opteron") selected_platform = "Intel/AMD 64-bit";
	if (register_list.platform == "intel") selected_platform = "Intel/AMD 32-bit";
	if (register_list.platform == "sparc") selected_platform = "Sun Sparc";
	if (register_list.platform == "mac") selected_platform = "Macintosh";
	if (register_list.platform == "sgi") selected_platform = "Silicon Graphics";
	if (register_list.platform == "generic") selected_platform = "Generic";

    if ( register_list.platform != PLATFORM  &&  
         register_list.platform != "generic" )
    {
      str = 
        us_tr( "You are running UltraScan on the " TITLE " platform,\n"
            "but your license is issued for the " + 
            selected_platform + " platform\n\n"
            "You will have to update your license file before\n"
            "proceeding. Click on 'Register' to obtain an\n"
            "UltraScan License for the " TITLE " platform." );
      license_info( str );
      flag = false;
      exit( -1 );
    }   

    register_list.os = ts.readLine();
    register_list.os = register_list.os.trimmed();

#ifdef WIN32
#define OS "win32"
#define OS_TITLE " Microsoft Windows"
#endif

#ifdef FREEBSD
#define OS "freebsd"
#define OS_TITLE " FreeBSD"
#endif

#ifdef OPENBSD
#define OS "openbsd"
#define OS_TITLE "n OpenBSD"
#endif

#ifdef NETBSD
#define OS "netbsd"
#define OS_TITLE " NetBSD"
#endif

#ifdef LINUX
#define OS "linux"
#undef OS_TITLE
#define OS_TITLE " Linux"
#endif

#ifdef OSX
#define OS "osx"
#define OS_TITLE " Macintosh OS-X"
#endif

#ifdef IRIX
#define OS "irix"
#define OS_TITLE " Silicon Graphics Irix"
#endif

#ifdef SOLARIS
#define OS "solaris"
#define OS_TITLE " Sun Solaris"
#endif

    if ( register_list.os != OS )
    {
      str = 
        us_tr( "You are running UltraScan with a" OS_TITLE " operating system,\n"
            "but your license is issued for the " + register_list.os.toUpper() + 
            " operating system\n\n"
            "You will have to update your license file before\n"
            "proceeding. Click on 'Register' to obtain an\n"
            "UltraScan License for a" OS_TITLE " operating system." );
      
      license_info( str );
      
      flag = false;
      exit( -1 );
    }

    register_list.version = ts.readLine();
    register_list.version = register_list.version.trimmed();
    
    register_list.license_type = ts.readLine();
    register_list.license_type = register_list.license_type.trimmed();
    
    register_list.license  = ts.readLine();
    register_list.license = register_list.license.trimmed();
    
    register_list.expiration = ts.readLine();
    register_list.expiration = register_list.expiration.trimmed();
    
    f.close();
  }
  else        // file not found
  {
    
    str = 
      us_tr("UltraScan could not find your license file:\n\n" + 
          lcfile + "\n\n"
          "You will have to update your license file before\n" +
          "proceeding. Click on 'Register' to obtain a new\n" +
          "UltraScan License" );
    
    license_info( str );
    flag = false;
    exit( -1 );
  }
  QString concat;
  QChar cval;
  concat = register_list.expiration + register_list.email;
  temp_license ="";
  for ( int i = 0; i < (int) concat.length(); i++ )
  {
     cval = concat[i];
     temp_license += str.sprintf("%X", cval.unicode());
  }
  temp_license.truncate(70);
  if ( QString::compare(temp_license, register_list.license) == 0 )
  {
    QDate today = QDate::currentDate();
    
    QDate expiration = QDate::fromString(register_list.expiration,  Qt::TextDate);

    if ( expiration >= today )
    {
      flag = true;
    }
    else
    {
      str = 
        us_tr( "The license in your home directory is expired.\n"
            "You will have to update your license file before\n"
            "proceeding. Click on 'Register' to obtain a new\n"
            "UltraScan License" );

      license_info( str );
      flag = false;
      exit( -1 );
    }
  }
  else  // file does not match
  {
    str = 
      us_tr( "The license in your home directory is invalid.\n"
          "You will have to update your license file before\n"
          "proceeding. Click on 'Register' to obtain a new\n"
          "UltraScan License" );
    
    license_info( str );
    flag = false;
    exit( -1 );
  }

  return flag;
}

/*!
  Shows a error message box to notice user to register.
  \param str the error message get from read().
*/
void US_Register::license_info( const QString& str )
{
  switch( QMessageBox::critical( this, 
        us_tr( "UltraScan License Error" ), 
        str,  
        us_tr( "&Register" ), 
        us_tr( "&Cancel" ), 0, 1 ) )  
    // Enter == button 0, Escape == button 1
  {
    case 0:
      us_license();
      break;

    case 1:
      exit(0);
      break;
  }
}

