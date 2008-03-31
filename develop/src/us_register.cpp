#include "../include/us_register.h"
#include "../include/us_license.h"


//! Constructor
/*! 
  Constractor a new <var>US_Register</var> class, 
  \param p Parent widget.
  \param name Widget name.
*/  
US_Register::US_Register(QWidget *p, const char *name) : QFrame(p, name)
{
  USConfig = new US_Config();
}

//! Destructor
/*! destroy <var>US_Register</var>. */
US_Register::~US_Register()
{
}

/*!
  Open a browser to register on web.
*/
void US_Register::online()
{
  proc = new QProcess(this);
  
  connect( proc, SIGNAL(readyReadStdout()), 
           this, SLOT(captureStdout()));
  connect( proc, SIGNAL(readyReadStderr()), 
           this, SLOT(captureStderr()));
  connect( proc, SIGNAL(processExited()),   
           this, SLOT(endProcess())   );

#ifdef UNIX
  stderrSize = 0;
  trials     = 0;
  proc->clearArguments();
  proc->addArgument("mozilla");
  proc->addArgument("-remote");
  proc->addArgument("openURL(http://www.ultrascan.uthscsa.edu/register.html,new-window)");
/*
  QStringList list = proc->arguments();
  QStringList::Iterator it = list.begin();
  while( it != list.end() )
  {
    cout << *it << endl;
    ++it;
  }
*/
  if ( ! proc->start() ) //try netscape instead
  {
    proc->clearArguments();
    proc->addArgument("netscape");
    proc->addArgument("-remote");
    proc->addArgument("openURL(http://www.ultrascan.uthscsa.edu/register.html,new-window)");
    
    if ( ! proc->start() ) // try firefox
    {
      proc->clearArguments();
      proc->addArgument("firefox");
      proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
      
      if(!proc->start()) //couldn't find anything, tell the user
      {   
        cout << 
          "Error: Couldn't find Netscape, Mozilla or Firefox\n"
          "Please make sure your have a browser installed!\n\n";
        
        cout << 
          "Please visit http://www.ultrascan.uthscsa.edu/register.html "
          "to register manually\n";
        
        QMessageBox::message(
         tr( "UltraScan Error:" ), 
         tr( "Couldn't start Netscape, Mozilla or Firefox...\n\n"
             "Please make sure you have a Browser installed.\n\n"
             "You can visit http://www.ultrascan.uthscsa.edu/register.html\n"
             "to register your UltraScan copy manually." ) );
        exit(-1);
      }
    }
  }
#endif

#ifdef WIN32
  proc->addArgument( USConfig->config_list.browser );
  proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
  
  if(!proc->start()) //error
  {
     QMessageBox::message(
       tr( "UltraScan Error:" ),
       tr( "Could not start " + USConfig->config_list.browser + " ...\n\n"
           "Please make sure you have a Browser installed.\n\n"
           "You can visit http://www.ultrascan.uthscsa.edu/register.html\n"
           "to register your UltraScan copy manually." ) );

    exit(-1);
  }
#endif
}

void US_Register::captureStdout()
{
  cout << proc->readLineStdout() << endl;
}

void US_Register::captureStderr()
{
  QByteArray list = proc->readStderr();
  stderrSize = list.size();
  cout << "The following error occured while attempting to run your browser:\n" 
       << QString(list) << endl;
}

void US_Register::endProcess()
{
#ifdef UNIX
  trials ++; 
  
  // Error attaching to already running process, start new
  if ( trials == 1 && stderrSize > 0 )
  {
    proc->clearArguments();
    proc->addArgument("netscape");
    proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
    
    if ( ! proc->start() ) // Error
    {
      cout << "Error: Can't start browser window - please make sure you have "
        "Netscape installed!\n";
      
      QMessageBox::message(
          tr( "UltraScan Error:" ), 
          tr( "Can't start browser window...\n"
              "Please make sure you have Netscape installed" ) );
      return;
    }
  }
#endif

  exit(0);
}
void US_Register::us_license()
{
  proc = new QProcess(this);
  proc->addArgument("us_license");
  
  if ( ! proc->start() ) // Error
  {
    QMessageBox::message(
        tr( "Please note:" ), 
        tr( "There was a problem creating a sub process\n"
            "for US_LICENSE\n\n"
            "Please check and try again..." ) );
    return;
  }
}

/*!
  Read license file to check the user is legal or not.
  \return <tt>True</tt>, the license is valid. \n
  \return <tt>False</tt>, the license is invalid.
*/
bool US_Register::read()
{
   QString lcfile;
   QString temp_license;
   QString str;
   bool    flag = false;

#ifdef UNIX
  lcfile = USConfig->home +  ".uslicense";
#endif

#ifdef WIN32
  lcfile  = USConfig->home + "_uslicense";
#endif

  QFile f(lcfile);  

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
  
  if ( f.open(IO_ReadOnly) )
  {
    QTextStream ts ( &f );
    register_list.lastname = ts.readLine();
    register_list.lastname = register_list.lastname.stripWhiteSpace();
//  cout<<"["<<register_list.lastname<<"]"<<endl; 
    
    register_list.firstname = ts.readLine();
    register_list.firstname = register_list.firstname.stripWhiteSpace();
//  cout<<"["<<register_list.firstname<<"]"<<endl;
    
    register_list.company = ts.readLine();
    register_list.company = register_list.company.stripWhiteSpace();
//  cout<<"["<<register_list.company<<"]"<<endl;
    
    register_list.address = ts.readLine();
    register_list.address = register_list.address.stripWhiteSpace();
//  cout<<"["<<register_list.address<<"]"<<endl;
    
    register_list.city = ts.readLine();
    register_list.city = register_list.city.stripWhiteSpace();
//  cout<<"["<<register_list.city<<"]"<<endl;
    
    register_list.state = ts.readLine();
    register_list.state = register_list.state.stripWhiteSpace();
//  cout<<"["<<register_list.state<<"]"<<endl;      
    
    register_list.zip = ts.readLine();
    register_list.zip = register_list.zip.stripWhiteSpace();
//  cout<<"["<<register_list.zip<<"]"<<endl;
    
    register_list.phone = ts.readLine();
    register_list.phone = register_list.phone.stripWhiteSpace();
//  cout<<"["<<register_list.phone<<"]"<<endl;
    
    register_list.email = ts.readLine();
    register_list.email = register_list.email.stripWhiteSpace();
//  cout<<"["<<register_list.email<<"]"<<endl;
    
    register_list.platform = ts.readLine();
    register_list.platform = register_list.platform.stripWhiteSpace();
//  cout<<"["<<register_list.platform<<"]"<<endl;
//  cout << "register_list.platform: " << register_list.platform << endl;

#ifdef SPARC
    if ( register_list.platform != "sparc" )
    {
      str = "You are running UltraScan on a Sun Sparc platform,\n"
          "but your license is issued for the " + 
          register_list.platform.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Sun Sparc platform.";
      license_info(str);
      flag = false;
      exit(-1);

    }   
#endif

#ifdef INTEL

    //  cout << "COMMERCIAL_UNIX\n";
    if(register_list.platform != "intel")
    {
      str = "You are running UltraScan on an Intel platform,\n"
          "but your license is issued for the " 
          + register_list.platform.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Intel platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef MAC
    //cout << "MAC defined , os: "  << register_list.os << " \n";
    if(register_list.platform != "mac")
    {
      str = "You are running UltraScan on a Macintosh platform,\n"
          "but your license is issued for the " 
          + register_list.platform.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Macintosh platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef OPTERON
    if(register_list.platform != "opteron")
    {
      str = "You are running UltraScan on a 64-bit AMD Opteron platform,\n"
          "but your license is issued for the " 
          + register_list.platform.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the 64-bit AMD Opteron platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef SGI
    if(register_list.platform != "sgi")
    {
      str = "You are running UltraScan on a Silicon Graphics platform,\n"
          "but your license is issued for the " 
          + register_list.platform.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Silicon Graphics platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif
    
    register_list.os = ts.readLine();
    register_list.os = register_list.os.stripWhiteSpace();

#ifdef WIN32
    if ( register_list.os != "win32" )
    {
      str = tr( "You are running UltraScan on a Microsoft Windows platform,\n"
          "but your license is issued for the " + register_list.os.upper() + 
          " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Microsoft Windows platform." );
      
      license_info( str );
      
      flag = false;
      exit(-1);
    }
#endif

#ifdef FREEBSD
    if ( register_list.os != "freebsd" )
    {
      str = "You are running UltraScan on a FreeBSD platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the FreeBSD platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef OPENBSD
    if ( register_list.os != "openbsd" )
    {
      str = "You are running UltraScan on an OpenBSD platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the OpenBSD platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef NETBSD
    if(register_list.os != "netbsd")
    {
      str = "You are running UltraScan on a NetBSD platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the NetBSD platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef LINUX
    if ( register_list.os != "linux" )
    {
      str = "You are running UltraScan on a Linux platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Linux platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef OSX
    if ( register_list.os != "osx" )
    {
      str = "You are running UltraScan on a Macintosh OS-X platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Macintosh OS-X platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef IRIX
    if(register_list.os != "irix")
    {
      str = "You are running UltraScan on a Silicon Graphics Irix platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Silicon Graphics Irix platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

#ifdef SOLARIS
    if(register_list.os != "solaris")
    {
      str = "You are running UltraScan on a Sun Solaris platform,\n"
          "but your license is issued for the " 
          + register_list.os.upper() + " platform\n\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain an\n"
          "UltraScan License for the Sun Solaris platform.";
      license_info(str);
      flag = false;
      exit(-1);
    }
#endif

    register_list.version = ts.readLine();
    register_list.version = register_list.version.stripWhiteSpace();
    
    register_list.license_type = ts.readLine();
    register_list.license_type = register_list.license_type.stripWhiteSpace();
    
    register_list.license  = ts.readLine();
    register_list.license = register_list.license.stripWhiteSpace();
    
    register_list.expiration = ts.readLine();
    register_list.expiration = register_list.expiration.stripWhiteSpace();
    
    f.close();
  }
  else        // file not found
  {
    
    str = "UltraScan could not find your license file:\n\n" + 
          lcfile + "\n\n"
          "You will have to update your license file before\n" +
          "proceeding. Click on \'Register\' to obtain a new\n" +
          "UltraScan License";
    
    license_info(str);
    flag = false;
    exit(-1);
  }

  register_list.code1 = encode(register_list.lastname, register_list.firstname);
  register_list.code2 = encode(register_list.company, register_list.address);
  register_list.code3 = encode(register_list.city, register_list.zip);
  register_list.code4 = encode(register_list.phone, register_list.email);
  
  /*
  register_list.code5 = encode((register_list.platform 
                      + register_list.os 
                      + register_list.version 
                      + register_list.license_type), register_list.expiration);
  */
  
  register_list.code5 = encode((register_list.platform + register_list.os 
                      + register_list.version), register_list.expiration);
  
  temp_license = register_list.code1 + "-" 
           + register_list.code2 + "-"
           + register_list.code3 + "-"
           + register_list.code4 + "-"
           + register_list.code5; 

  //  cout << "Generated license: " << temp_license << endl;
  //  cout << "Stored license:    " << register_list.license << endl;
  //  QMessageBox::message(temp_license, register_list.license);
  
  if ( QString::compare(temp_license, register_list.license) == 0 )
  {
    QDate today = QDate::currentDate();
    
    QDate expiration = QDate::fromString(register_list.expiration,  Qt::TextDate);
    if (expiration >= today)
    {
      flag = true;
    }
    else
    {
      str = tr( "The license in your home directory is expired.\n"
          "You will have to update your license file before\n"
          "proceeding. Click on \'Register\' to obtain a new\n"
          "UltraScan License" );

      license_info(str);
      flag = false;
      exit(-1);
    }
  }
  else  // file does not match
  {
    str = tr( "The license in your home directory is invalid.\n"
        "You will have to update your license file before\n"
        "proceeding. Click on \'Register\' to obtain a new\n"
        "UltraScan License" );
    license_info(str);
    flag = false;
    exit(-1);
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
        tr( "UltraScan License Error" ), 
        str,  
        tr( "&Register" ), 
        tr( "&Cancel" ), 0, 1 ) )  
    // Enter == button 0, Escape == button 1
  {
    case 0:
      us_license( );
      break;

    case 1:
      exit(0);
      break;
  }
}

/*!
  A secure function for create license key.
*/
QString US_Register::encode( QString str1, QString str2 )
{
  QString STR1, STR2, SUM, SUM1, SUM2, code, CODE;
  QChar c;
  int sum1 = 0, sum2 = 0, x;
  float j;
  STR1 = str1.upper();
  STR2 = str2.upper();
  
  for(unsigned int i=0; i<STR1.length(); i++)
  {
    c = STR1.at(i);
    sum1 += c.unicode();  
  }
  
  SUM1 = QString::number(sum1);
  
  for( unsigned int i=0; i<STR2.length(); i++ )
  {
    c = STR2.at(i);
    sum2 += c.unicode();
  }
  
  SUM2 = QString::number(sum2);
  SUM  = SUM1 + SUM2;
  j    = SUM.toFloat();
  x    = int(fabs(sin(j))*65535);
  code.sprintf("%x", x);
  
  if ( code.length( ) < 4 )
  {
    if ( code.length( ) == 3 )
    {
      code = "0" + code;
    }

    if ( code.length( ) == 2 )
    {
      code = "00" + code;
    }

    if ( code.length( ) == 1 )
    {
      code = "000" + code;
    }
  }

  CODE = code.upper();
//  cout << CODE << endl;
  return CODE;
}

