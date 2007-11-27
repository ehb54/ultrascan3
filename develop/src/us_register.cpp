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
	connect(proc, SIGNAL(readyReadStdout()), this, SLOT(captureStdout()));
	connect(proc, SIGNAL(readyReadStderr()), this, SLOT(captureStderr()));
	connect(proc, SIGNAL(processExited()), this, SLOT(endProcess()));
#ifdef UNIX
	stderrSize = 0;
	trials = 0;
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
	if(!proc->start()) //try netscape instead
	{
		proc->clearArguments();
		proc->addArgument("netscape");
		proc->addArgument("-remote");
		proc->addArgument("openURL(http://www.ultrascan.uthscsa.edu/register.html,new-window)");
		if(!proc->start()) //try firefox
		{
			proc->clearArguments();
			proc->addArgument("firefox");
			proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
			if(!proc->start()) //couldn't find anything, tell the user
			{		
				cout << "Error: Couldn't find Netscape, Mozilla or Firefox\nPlease make sure your have a browser installed!\n\n";
				cout << "Please visit http://www.ultrascan.uthscsa.edu/register.html to register manually\n";
				QMessageBox::message("UltraScan Error:", "Couldn't start Netscape, Mozilla or Firefox...\n\n"
										"Please make sure you have a Browser installed.\n\n"
										"You can visit http://www.ultrascan.uthscsa.edu/register.html\n"
										"to register your UltraScan copy manually.");
				exit(-1);
			}
		}
	}
#endif

#ifdef WIN32
	proc->addArgument("explorer.exe");
	proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
	if(!proc->start()) //error
	{
		cout << "Error: Can't start browser window - please make sure you have Explorer installed!\n\n";
		cout << "Please visit http://www.ultrascan.uthscsa.edu/register.html to register manually\n";
		QMessageBox::message("UltraScan Error:", "Can't start browser window...\n\n"
									"Please make sure you have Explorer installed.\n\n"
									"You can visit http://www.ultrascan.uthscsa.edu/register.html\n"
									"to register your UltraScan copy manually.");
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
	cout << "The following error occured while attempting to run Netscape:\n" << QString(list) << endl;
}

void US_Register::endProcess()
{
//	cout << "Process " << trials << " exited\n";
#ifdef UNIX
	trials ++; 
	if (trials == 1 && stderrSize > 0) // error attaching to already running process, start new
	{
		proc->clearArguments();
		proc->addArgument("netscape");
		proc->addArgument("http://www.ultrascan.uthscsa.edu/register.html");
		if(!proc->start()) //error
		{
			cout << "Error: Can't start browser window - please make sure you have Netscape installed!\n";
			QMessageBox::message("UltraScan Error:", "Can't start browser window...\nPlease make sure you have Netscape installed");
			return;
		}
	}
	else
	{
		exit(0);
	}
#endif
#ifdef WIN32
	exit(0);
#endif
}
void US_Register::us_license()
{
	proc = new QProcess(this);
	proc->addArgument("us_license");
	if(!proc->start()) //error
	{
		QMessageBox::message(tr("Please note:"), tr("There was a problem creating a sub process\n"
														 "for US_LICENSE\n\n"
														 "Please check and try again..."));
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
	QString lcfile, temp_license, str;
	bool flag = false;
#ifdef UNIX
	lcfile = (getenv("HOME"));
   if (lcfile != "/")
	{
		lcfile += "/";
	}
	lcfile.append(".uslicense");
#endif
#ifdef WIN32
	lcfile = "C:\\Program\ Files\\Ultrascan\\etc\\uslicense";
#endif
	
	QFile f(lcfile);	
// identify license file
// does license file exists?
// 	yes: continue
// 	no: update instruction and return false
// is license file correct?
//		yes: is it expired?
//			yes: update instruction and return false
//			no: return true
//		no: update instruction and return false
// no: update instruction and return false
	if(f.open(IO_ReadOnly))
	{
		QTextStream ts (&f);
		register_list.lastname = ts.readLine();
		register_list.lastname = register_list.lastname.stripWhiteSpace();
//	cout<<"["<<register_list.lastname<<"]"<<endl;	
		register_list.firstname = ts.readLine();
		register_list.firstname = register_list.firstname.stripWhiteSpace();
//	cout<<"["<<register_list.firstname<<"]"<<endl;
		register_list.company = ts.readLine();
		register_list.company = register_list.company.stripWhiteSpace();
//	cout<<"["<<register_list.company<<"]"<<endl;
		register_list.address = ts.readLine();
		register_list.address = register_list.address.stripWhiteSpace();
//	cout<<"["<<register_list.address<<"]"<<endl;
		register_list.city = ts.readLine();
		register_list.city = register_list.city.stripWhiteSpace();
//	cout<<"["<<register_list.city<<"]"<<endl;
		register_list.state = ts.readLine();
		register_list.state = register_list.state.stripWhiteSpace();
//	cout<<"["<<register_list.state<<"]"<<endl;			
		register_list.zip = ts.readLine();
		register_list.zip = register_list.zip.stripWhiteSpace();
//	cout<<"["<<register_list.zip<<"]"<<endl;
		register_list.phone = ts.readLine();
		register_list.phone = register_list.phone.stripWhiteSpace();
//	cout<<"["<<register_list.phone<<"]"<<endl;
		register_list.email = ts.readLine();
		register_list.email = register_list.email.stripWhiteSpace();
//	cout<<"["<<register_list.email<<"]"<<endl;
		register_list.platform = ts.readLine();
		register_list.platform = register_list.platform.stripWhiteSpace();
//	cout<<"["<<register_list.platform<<"]"<<endl;
//	cout << "register_list.platform: " << register_list.platform << endl;
#ifdef SPARC
		if(register_list.platform != "sparc")
		{
			str = "You are running UltraScan on a Sun Sparc platform,\n"
					"but your license is issued for the " + register_list.platform.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the Sun Sparc platform.";
			license_info(str);
			flag = false;
			exit(-1);

		}		
#endif
#ifdef INTEL
//	cout << "COMMERCIAL_UNIX\n";
		if(register_list.platform != "intel")
		{
			str = "You are running UltraScan on an Intel platform,\n"
					"but your license is issued for the " + register_list.platform.upper() + " platform\n\n"
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
					"but your license is issued for the " + register_list.platform.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the Macintosh platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef OPTERON
//	cout << "WIN32\n";
		if(register_list.platform != "opteron")
		{
			str = "You are running UltraScan on a 64-bit AMD Opteron platform,\n"
					"but your license is issued for the " + register_list.platform.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the 64-bit AMD Opteron platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef SGI
//	cout << "WIN32\n";
		if(register_list.platform != "sgi")
		{
			str = "You are running UltraScan on a Silicon Graphics platform,\n"
					"but your license is issued for the " + register_list.platform.upper() + " platform\n\n"
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
	//	cout<<"["<<register_list.os<<"]"<<endl;
#ifdef WIN32
		if(register_list.os != "win32")
		{
			str = "You are running UltraScan on a Microsoft Windows platform,\n"
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the Microsoft Windows platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef FREEBSD
//	cout << "WIN32\n";
		if(register_list.os != "freebsd")
		{
			str = "You are running UltraScan on a FreeBSD platform,\n"
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the FreeBSD platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef OPENBSD
		if(register_list.os != "openbsd")
		{
			str = "You are running UltraScan on an OpenBSD platform,\n"
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
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
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the NetBSD platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef LINUX
		if(register_list.os != "linux")
		{
			str = "You are running UltraScan on a Linux platform,\n"
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain an\n"
					"UltraScan License for the Linux platform.";
			license_info(str);
			flag = false;
			exit(-1);
		}
#endif
#ifdef OSX
//	cout << "OSX defined , platform: "  << register_list.platform << " \n";
		if(register_list.os != "osx")
		{
			str = "You are running UltraScan on a Macintosh OS-X platform,\n"
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
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
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
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
					"but your license is issued for the " + register_list.os.upper() + " platform\n\n"
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
	//	cout<<"["<<register_list.version<<"]"<<endl;
		register_list.license_type = ts.readLine();
		register_list.license_type = register_list.license_type.stripWhiteSpace();
	//	cout<<"["<<register_list.license_type<<"]"<<endl;
		register_list.license  = ts.readLine();
		register_list.license = register_list.license.stripWhiteSpace();
	//	cout<<"["<<register_list.license<<"]"<<endl;	
		register_list.expiration = ts.readLine();
		register_list.expiration = register_list.expiration.stripWhiteSpace();
	//	cout<<"["<<register_list.expiration<<"]"<<endl;
		f.close();
	}
	else				// file not found
	{
		
		str = "UltraScan could not find your license file:\n\n" + 
				lcfile + 
				"\n\nYou will have to update your license file before\n" +
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

//	cout << "Generated license: " << temp_license << endl;
//	cout << "Stored license:    " << register_list.license << endl;
//	QMessageBox::message(temp_license, register_list.license);
	if(QString::compare(temp_license, register_list.license) == 0)
	{
		QDate today = QDate::currentDate();
		QDate expiration = QDate::fromString(register_list.expiration,  Qt::TextDate);
		if (expiration >= today)
		{
			flag = true;
		}
		else
		{
			str = "The license in your home directory is expired.\n"
					"You will have to update your license file before\n"
					"proceeding. Click on \'Register\' to obtain a new\n"
					"UltraScan License";
			license_info(str);
			flag = false;
			exit(-1);
		}
	}
	else	// file does not match
	{
		str = "The license in your home directory is invalid.\n"
				"You will have to update your license file before\n"
				"proceeding. Click on \'Register\' to obtain a new\n"
				"UltraScan License";
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
void US_Register::license_info(const QString &str)
{
	switch( QMessageBox::critical( this, "UltraScan License Error", 
	str,	"&Register", "&Cancel", 0, 1))  // Enter == button 0, Escape == button 1
	{
		case 0:
		{
			//online();
			us_license();
			break;
		}
		case 1:
		{
			exit(0);
			break;
		}
	}
}

/*!
	A secure function for create license key.
*/
QString US_Register::encode(QString str1, QString str2)
{
	QString STR1, STR2, SUM, SUM1, SUM2, code, CODE;
	QChar c;
	int sum1 = 0, sum2 = 0, x;
	float j;
	STR1 = str1.upper();
	STR2 = str2.upper();
	for(unsigned int i=0; i<STR1.length(); i++)
	{
		c = STR1[i];
		sum1 += c.unicode();	
	}
	SUM1 = QString::number(sum1);
	for(unsigned int i=0; i<STR2.length(); i++)
	{
		c = STR2[i];
		sum2 += c.unicode();
	}
	SUM2 = QString::number(sum2);
	SUM = SUM1 + SUM2;
	j = SUM.toFloat();
	x = int(fabs(sin(j))*65535);
	code.sprintf("%x", x);
	if(code.length() < 4 )
	{
		if(code.length() == 3)
		{
			code = "0" + code;
		}
		if(code.length() == 2)
		{
			code = "00" + code;
		}
		if(code.length() == 1)
		{
			code = "000" + code;
		}
	}
	CODE = code.upper();
//	cout << CODE << endl;
	return CODE;
}
