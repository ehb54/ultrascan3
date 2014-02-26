#include "../include/us_license.h"
//Added by qt3to4:
#include <Q3TextStream>
#include <QCloseEvent>
#include <Q3Frame>
#include <QLabel>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_License::US_License(QWidget *parent, const char *name) : QDialog( parent, name, false)
{
   int spacing=5, column1 = 90, column2 = 10, column3=110;
   int xpos=spacing, ypos=spacing;
   int buttonw = 180;
   int buttonh = 26;
   int span = buttonw + 4*spacing + 2*column1 + 2*column3;
   int half_buttonw = (int) ((buttonw + 2*column1 + 2*column3 + 2*spacing - column2)/2);
   int full_buttonw = buttonw + 2*column1 + 2*column3 + 4*spacing;
   int pushbutton = (int) (buttonw+span)/5;

   setCaption("UltraScan License Configuration");

   lbl_blank = new QLabel(
                          tr( "Please enter all fields exactly as shown in the issued license,\n"
                              "or import a license from an E-mail text file:" ), this );
   lbl_blank->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_blank->setGeometry(xpos, ypos, span+buttonw+spacing, 2*buttonh+spacing);
   lbl_blank->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);

   ypos += spacing + 3*buttonh;

   lbl_firstname = new QLabel(tr("Name (first, last):"),this);
   lbl_firstname->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_firstname->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_firstname = new QLineEdit(this);
   le_firstname->setGeometry(xpos, ypos, half_buttonw, buttonh);
   connect(le_firstname, SIGNAL(textChanged(const QString &)), 
           SLOT(update_firstname(const QString &)));

   xpos += half_buttonw + spacing;

   lbl_lastname = new QLabel(tr(","),this);
   lbl_lastname->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_lastname->setGeometry(xpos, ypos, column2, buttonh);

   xpos += spacing + column2;

   le_lastname = new QLineEdit(this);
   le_lastname->setGeometry(xpos, ypos, half_buttonw, buttonh);
   connect(le_lastname, SIGNAL(textChanged(const QString &)), 
           SLOT(update_lastname(const QString &)));

   xpos = spacing;
   ypos += buttonh + spacing;

   lbl_email = new QLabel(tr("E-mail Address:"),this);
   lbl_email->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_email->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_email = new QLineEdit(this);
   le_email->setGeometry(xpos, ypos, full_buttonw, buttonh);
   connect(le_email, SIGNAL(textChanged(const QString &)), 
           SLOT(update_email(const QString &)));

   xpos = spacing;
   ypos += buttonh + spacing;

   lbl_institution = new QLabel(tr("Institution:"),this);
   lbl_institution->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_institution->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_institution = new QLineEdit(this);
   le_institution->setGeometry(xpos, ypos, full_buttonw, buttonh);
   connect(le_institution, SIGNAL(textChanged(const QString &)), 
           SLOT(update_institution(const QString &)));

   xpos = spacing;
   ypos += buttonh + spacing;

   lbl_address = new QLabel(tr("Address:"),this);
   lbl_address->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_address->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_address = new QLineEdit(this);
   le_address->setGeometry(xpos, ypos, full_buttonw, buttonh);
   connect(le_address, SIGNAL(textChanged(const QString &)), 
           SLOT(update_address(const QString &)));

   xpos = spacing;
   ypos += buttonh + spacing;

   lbl_city = new QLabel(tr("City:"),this);
   lbl_city->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_city->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_city = new QLineEdit(this);
   le_city->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(le_city, SIGNAL(textChanged(const QString &)), 
           SLOT(update_city(const QString &)));

   xpos += buttonw + spacing;

   lbl_state = new QLabel(tr(" State:"),this);
   lbl_state->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_state->setGeometry(xpos, ypos, column1, buttonh);

   xpos += spacing + column1;

   cbb_state = new Q3ComboBox(this, "state");
   cbb_state->insertItem("NON-US");
   cbb_state->insertItem("AL");
   cbb_state->insertItem("AR");
   cbb_state->insertItem("AZ");
   cbb_state->insertItem("CA");
   cbb_state->insertItem("CO");
   cbb_state->insertItem("CT");
   cbb_state->insertItem("DC");
   cbb_state->insertItem("DE");
   cbb_state->insertItem("FL");
   cbb_state->insertItem("GA");
   cbb_state->insertItem("HI");
   cbb_state->insertItem("IA");
   cbb_state->insertItem("ID");
   cbb_state->insertItem("IL");
   cbb_state->insertItem("IN");
   cbb_state->insertItem("KS");
   cbb_state->insertItem("KY");
   cbb_state->insertItem("LA");
   cbb_state->insertItem("MA");
   cbb_state->insertItem("MD");
   cbb_state->insertItem("ME");
   cbb_state->insertItem("MI");
   cbb_state->insertItem("MN");
   cbb_state->insertItem("MO");
   cbb_state->insertItem("MS");
   cbb_state->insertItem("MT");
   cbb_state->insertItem("NC");
   cbb_state->insertItem("ND");
   cbb_state->insertItem("NE");
   cbb_state->insertItem("NH");
   cbb_state->insertItem("NJ");
   cbb_state->insertItem("NM");
   cbb_state->insertItem("NV");
   cbb_state->insertItem("NY");
   cbb_state->insertItem("OH");
   cbb_state->insertItem("OK");
   cbb_state->insertItem("OR");
   cbb_state->insertItem("PA");
   cbb_state->insertItem("PR");
   cbb_state->insertItem("RI");
   cbb_state->insertItem("SC");
   cbb_state->insertItem("SD");
   cbb_state->insertItem("TN");
   cbb_state->insertItem("TX");
   cbb_state->insertItem("UT");
   cbb_state->insertItem("VA");
   cbb_state->insertItem("VI");
   cbb_state->insertItem("VT");
   cbb_state->insertItem("WA");
   cbb_state->insertItem("WI");
   cbb_state->insertItem("WV");
   cbb_state->insertItem("WY");
   cbb_state->setGeometry(xpos, ypos, column3, buttonh);
   connect(cbb_state, SIGNAL(activated(int)), SLOT(update_state(int)));
   connect(cbb_state, SIGNAL(highlighted(int)), SLOT(update_state(int)));

   xpos += column3 + spacing;

   lbl_zip = new QLabel(tr(" Zip:"),this);
   lbl_zip->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_zip->setGeometry(xpos, ypos, column1, buttonh);

   xpos += spacing + column1;

   le_zip = new QLineEdit(this);
   le_zip->setGeometry(xpos, ypos, column3, buttonh);
   connect(le_zip, SIGNAL(textChanged(const QString &)), 
           SLOT(update_zip(const QString &)));

   xpos = spacing;
   ypos += buttonh + spacing;

   lbl_phone = new QLabel(tr("Phone Number:"),this);
   lbl_phone->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_phone->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += spacing + buttonw;

   le_phone = new QLineEdit(this);
   le_phone->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(le_phone, SIGNAL(textChanged(const QString &)), 
           SLOT(update_phone(const QString &)));

   xpos += buttonw + spacing;

   lbl_licensetype = new QLabel(tr(" License:"),this);
   lbl_licensetype->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_licensetype->setGeometry(xpos, ypos, column1, buttonh);

   xpos += spacing + column1;

   cbb_licensetype = new Q3ComboBox(this, "licensetype");
   cbb_licensetype->setGeometry(xpos, ypos, column3, buttonh);
   cbb_licensetype->insertItem("academic");
   cbb_licensetype->insertItem("commercial");
   cbb_licensetype->insertItem("trial");
  
   connect(cbb_licensetype, SIGNAL(activated(int)), 
           SLOT(update_licensetype(int)));
  
   connect(cbb_licensetype, SIGNAL(highlighted(int)), 
           SLOT(update_licensetype(int)));

   xpos += spacing + column3;

   lbl_version = new QLabel(tr(" Version:"),this);
   lbl_version->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_version->setGeometry(xpos, ypos, column1, buttonh);

   xpos += spacing + column1;

   cbb_version = new Q3ComboBox(this, "version");
   cbb_version->setGeometry(xpos, ypos, column3, buttonh);
   cbb_version->insertItem("6.0");
   cbb_version->insertItem("6.2");
   cbb_version->insertItem("7.0");
   cbb_version->insertItem("7.1");
   cbb_version->insertItem("7.2");
   cbb_version->insertItem("7.3");
   cbb_version->insertItem("7.4");
   cbb_version->insertItem("8.0");
   cbb_version->insertItem("8.1");
   cbb_version->insertItem("9.0");
   cbb_version->insertItem("9.2");
   cbb_version->insertItem("9.3");
   cbb_version->insertItem("9.4");
   cbb_version->insertItem("9.5");
   cbb_version->insertItem("9.6");
   cbb_version->insertItem("9.7");
   cbb_version->insertItem("9.8");
   cbb_version->insertItem("9.9");
   connect(cbb_version, SIGNAL(activated(int)), SLOT(update_version(int)));
   connect(cbb_version, SIGNAL(highlighted(int)), SLOT(update_version(int)));

   xpos = spacing;
   ypos += 2*buttonh + spacing;

   //lbl_platform = new QLabel(tr("Platform:"),this);
   //lbl_platform->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   //lbl_platform->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   //rb_intel = new QRadioButton(tr("Intel"), this);
   //rb_intel->setGeometry(xpos, ypos, buttonw, buttonh);
   //connect(rb_intel, SIGNAL(clicked()), SLOT(update_intel_rb()));

   xpos += spacing*4 + buttonw;

   lb_os = new Q3ListBox(this, "software");
   lb_os->setGeometry(xpos, ypos, half_buttonw, buttonh*5 + 4*spacing);
   lb_os->insertItem("Linux");
   lb_os->insertItem("Windows");
   lb_os->insertItem("Mac OS-X");
   lb_os->insertItem("Irix 6.5");
   lb_os->insertItem("Solaris");
   lb_os->insertItem("FreeBSD");
   lb_os->insertItem("NetBSD");
   lb_os->insertItem("OpenBSD");

   connect(lb_os, SIGNAL(selected(int)), SLOT(update_os(int)));
   connect(lb_os, SIGNAL(highlighted(int)), SLOT(update_os(int)));

   ypos += spacing + buttonh;
   xpos = buttonw + 2*spacing;

   //rb_sparc = new QRadioButton(tr("Sparc"), this);
   //rb_sparc->setGeometry(xpos, ypos, buttonw, buttonh);
   //connect(rb_sparc, SIGNAL(clicked()), SLOT(update_sparc_rb()));

   ypos += buttonh + spacing;

   //rb_mac = new QRadioButton(tr("Macintosh"), this);
   //rb_mac->setGeometry(xpos, ypos, buttonw, buttonh);
   //connect(rb_mac, SIGNAL(clicked()), SLOT(update_mac_rb()));

   ypos += buttonh + spacing;

   //rb_opteron = new QRadioButton(tr("Opteron"), this);
   //rb_opteron->setGeometry(xpos, ypos, buttonw, buttonh);
   //connect(rb_opteron, SIGNAL(clicked()), SLOT(update_opteron_rb()));

   ypos += buttonh + spacing;

   //rb_sgi = new QRadioButton(tr("Silicon Graphics"), this);
   //rb_sgi->setGeometry(xpos, ypos, buttonw, buttonh);
   //connect(rb_sgi, SIGNAL(clicked()), SLOT(update_sgi_rb()));


   ypos += spacing + 2*buttonh;
   xpos = spacing;

   lbl_code = new QLabel(tr("License Code:"),this);
   lbl_code->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_code->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   le_code = new QLineEdit(this);
   le_code->setGeometry(xpos, ypos, full_buttonw, buttonh);
   connect(le_code, SIGNAL(textChanged(const QString &)), 
           SLOT(update_code(const QString &)));

   ypos += spacing + buttonh;
   xpos = spacing;

   lbl_expiration = new QLabel(tr("Expiration Date:"),this);
   lbl_expiration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_expiration->setGeometry(xpos, ypos, buttonw, buttonh);

   xpos += buttonw + spacing;

   le_expiration = new QLineEdit(this);
   le_expiration->setGeometry(xpos, ypos, buttonw, buttonh);
   connect(le_expiration, SIGNAL(textChanged(const QString &)), 
           SLOT(update_expiration(const QString &)));

   xpos = spacing;
   ypos += spacing + 2*buttonh;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setGeometry(xpos, ypos, pushbutton, 26);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += pushbutton + spacing;

   pb_import = new QPushButton(tr("E-mail Import"), this);
   Q_CHECK_PTR(pb_import);
   pb_import->setAutoDefault(false);
   pb_import->setGeometry(xpos, ypos, pushbutton, 26);
   connect(pb_import, SIGNAL(clicked()), SLOT(import()));

   xpos += pushbutton + spacing;

   pb_request = new QPushButton(tr("Request New"), this);
   Q_CHECK_PTR(pb_request);
   pb_request->setAutoDefault(false);
   pb_request->setGeometry(xpos, ypos, pushbutton, 26);
   connect(pb_request, SIGNAL(clicked()), SLOT(request()));

   xpos += pushbutton + spacing;

   pb_save = new QPushButton(tr("Save"), this);
   Q_CHECK_PTR(pb_save);
   pb_save->setAutoDefault(false);
   pb_save->setGeometry(xpos, ypos, pushbutton, 26);
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   xpos += pushbutton + spacing;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setGeometry(xpos, ypos, pushbutton, 26);
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   ypos +=30;
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry( global_Xpos, global_Ypos, buttonw+span+3*spacing, ypos );
   setMinimumSize(buttonw+span+3*spacing, ypos);

}

US_License::~US_License()
{
}

void US_License::closeEvent(QCloseEvent* e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_License::cancel()
{
   close();
}

void US_License::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/license.html");
}

void US_License::save()
{
   QString home    = US_Config::get_home_dir();

   //   create new $HOME/ultrascan directory, if it doesn't exist
   if ( ! QDir( home ).exists() )
   {
      QDir tmp;
      tmp.mkdir(home);
   }

   // Make sure user's etc directory exists
   if ( ! QDir( home + ETC_DIR ).exists() )
   {
      QDir etc;
      etc.mkdir( home + ETC_DIR );
   }

   QString lcfile = US_Config::get_home_dir() + USLICENSE;

   QFile f( lcfile );
   if( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      Q3TextStream ts (&f);
      ts<<lastname<<"\n";
      ts<<firstname<<"\n";
      ts<<institution<<"\n";
      ts<<address<<"\n";
      ts<<city<<"\n";
      ts<<state<<"\n";
      ts<<zip<<"\n";
      ts<<phone<<"\n";
      ts<<email<<"\n";
      ts<<platform<<"\n";
      ts<<os<<"\n";
      ts<<version<<"\n";
      ts<<licensetype<<"\n";
      ts<<code<<"\n";
      ts<<expiration<<"\n";

      QMessageBox::message(
                           tr( "Thanks!" ), 
                           tr( "Your license information was saved to the following file:\n\n"
                               + QDir::convertSeparators( lcfile ) ) );
      pb_cancel->setText("Close");
   }
   else
   {
      QMessageBox::message(
                           tr( "Error" ), 
                           tr( "Could not save license file - "
                               "please make sure you have write permission" ) );
      return;
   }
   f.close();
}

void US_License::import()
{
   Q3FileDialog fd;
   int count=0;
   QFile texfile;
   QString filename, line="";
   filename = fd.getOpenFileName( "", "", this, "", 
                                  "Please select an e-mail file containing an UltraScan license..." );
   texfile.setName(filename);

   if (texfile.open(QIODevice::ReadOnly))
   {
      Q3TextStream ts(&texfile);
      lastname = ts.readLine();
      le_lastname->setText(lastname);
      count++;
      if (!ts.atEnd())
      {
         firstname = ts.readLine();
         le_firstname->setText(firstname);
      }
      if (!ts.atEnd())
      {
         institution = ts.readLine();
         le_institution->setText(institution);
      }
      if (!ts.atEnd())
      {
         address = ts.readLine();
         le_address->setText(address);
      }
      if (!ts.atEnd())
      {
         city = ts.readLine();
         le_city->setText(city);
      }
      if (!ts.atEnd())
      {
         state = ts.readLine();
         int i=0;
         while (i<cbb_state->count() && state != cbb_state->text(i))
         {
            i++;
         }
         cbb_state->setCurrentItem(i);
      }
      if (!ts.atEnd())
      {
         zip = ts.readLine();
         le_zip->setText(zip);
      }
      if (!ts.atEnd())
      {
         phone = ts.readLine();
         le_phone->setText(phone);
      }
      if (!ts.atEnd())
      {
         email = ts.readLine();
         le_email->setText(email);
      }
      if (!ts.atEnd())
      {
         platform = ts.readLine();
         if (platform == "opteron")
         {
            //update_opteron_rb();
         }
         if (platform == "intel")
         {
            //update_intel_rb();
         }
         if (platform == "sparc")
         {
            //update_sparc_rb();
         }
         if (platform == "mac")
         {
            //update_mac_rb();
         }
         if (platform == "sgi")
         {
            //update_sgi_rb();
         }

         if (!ts.atEnd())
         {
            os = ts.readLine();
         }
         if (os == "linux")
         {
            lb_os->setCurrentItem(0);
         }
         if (os == "win32")
         {
            lb_os->setCurrentItem(1);
         }
         if (os == "osx")
         {
            lb_os->setCurrentItem(2);
         }
         if (os == "irix")
         {
            lb_os->setCurrentItem(3);
         }
         if (os == "solaris")
         {
            lb_os->setCurrentItem(4);
         }
         if (os == "freebsd")
         {
            lb_os->setCurrentItem(5);
         }
         if (os == "netbsd")
         {
            lb_os->setCurrentItem(6);
         }
         if (os == "openbsd")
         {
            lb_os->setCurrentItem(7);
         }
      }
      if (!ts.atEnd())
      {
         version = ts.readLine();
         int i=0;
         while (i<cbb_version->count() && version != cbb_version->text(i))
         {
            i++;
         }
         cbb_version->setCurrentItem(i);
      }
      if (!ts.atEnd())
      {
         licensetype = ts.readLine();
         int i=0;
         while (i<cbb_licensetype->count() && licensetype != cbb_licensetype->text(i))
         {
            i++;
         }
         cbb_licensetype->setCurrentItem(i);
      }
      if (!ts.atEnd())
      {
         QString Code=ts.readLine();
         le_code->setText(Code);
      }
      if (!ts.atEnd())
      {
         expiration = ts.readLine();
         le_expiration->setText(expiration);
      }
      texfile.close();
   }
   display();
}

void US_License::request()
{
   proc = new Q3Process(this);
   proc->clearArguments();
   connect(proc, SIGNAL(readyReadStdout()), this, SLOT(captureStdout()));
   connect(proc, SIGNAL(readyReadStderr()), this, SLOT(captureStderr()));
   connect(proc, SIGNAL(processExited())  , this, SLOT(endProcess()));

   stderrSize = 0;
   trials = 0;
   bool browser_started = false;
  
   struct 
   {
      QString program;
      QString remote;
      QString url;
   } browser[] = 
      { 
#ifdef MAC
#undef UNIX
#define BROWSERS "Safari or Firefox"
         { "safari" , "", 
           "http://www.ultrascan2.uthscsa.edu/registration.php" },
         { "firefox", "", 
           "http://www.ultrascan2.uthscsa.edu/registration.php" },
         { "", "", "" }
#endif

#ifdef UNIX
#define BROWSERS "Netscape, Mozilla or Firefox"
         { "firefox", "",
           "http://www.ultrascan.uthscsa.edu/register.html" },
         { "mozilla" , "-remote", 
           "openURL(http://www.ultrascan.uthscsa.edu/register.html,new-window)" },
         { "netscape", "-remote", 
           "openURL(http://www.ultrascan.uthscsa.edu/register.html,new-window)" },
         { "", "", "" }
#endif

#ifdef WIN32
#define BROWSERS "Internet Explorer"
         { "explorer", "", 
           "http://www.ultrascan.uthscsa.edu/register.html" },
         { "C:/Program Files/Internet Explorer/iexplore.exe", "",
           "http://www.ultrascan.uthscsa.edu/register.html" },
         { "", "", "" }
#endif
      }; 

   int i = 0;
   while ( strlen( browser[i].program ) != 0 )
   {
      if ( start_browser( browser[i].program, browser[i].remote, browser[i].url ) )
      {
         browser_started = true;
         break;
      }

      i++;
   }

   if ( ! browser_started )
   {
      cout << "Error: Couldn't start " BROWSERS "\n"
         "Please make sure your have the appropriate browser installed!\n\n"
         "Please visit http://www.ultrascan.uthscsa.edu/register.html "
         "to register manually\n";
    
      QMessageBox::message(
                           tr( "UltraScan Error:" ), 
                           tr( "Couldn't start " BROWSERS "\n\n"
                               "Please make sure you have the appropriate ibrowser installed.\n\n"
                               "You can visit http://www.ultrascan.uthscsa.edu/register.html\n"
                               "to register your UltraScan copy manually." ) );
      exit(-1);
   }

}

bool US_License::start_browser( const QString& browser, 
                                const QString& remote, const QString& url )
{
   proc->clearArguments();
#ifdef Q_WS_MAC
   proc->addArgument( "open" );
   proc->addArgument( "-a" );
#endif
   proc->addArgument( browser );
   if ( remote != "" ) proc->addArgument( remote );
   proc->addArgument( url );

   return ( proc->start() );
}


void US_License::captureStdout()
{
   cout << proc->readLineStdout() << endl;
}

void US_License::captureStderr()
{
   QByteArray list = proc->readStderr();
   stderrSize = list.size();
   cout << "The following error occured while attempting to run Mozilla:\n" 
        << QString(list) << endl;
}

void US_License::endProcess()
{
   trials ++;

   // error attaching to already running process, start new$
   if ( trials == 1 && stderrSize > 0 )
   {
      proc->clearArguments();
      proc->addArgument( "mozilla" );
      proc->addArgument( "http://www.ultrascan.uthscsa.edu/register.html" );
    
      if ( ! proc->start() ) //error
      {
         cout << "Error: Can't start browser window\n";

         QMessageBox::message(
                              tr( "UltraScan Error:" ), 
                              tr( "Can't start browser window..." ) );
      
         return;
      }
   }
   else
   {
      exit(0);
   }
}

void US_License::display()
{
}

void US_License::update_firstname(const QString &str)
{
   firstname = str;
}

void US_License::update_lastname(const QString &str)
{
   lastname = str;
}

void US_License::update_institution(const QString &str)
{
   institution = str;
}

void US_License::update_address(const QString &str)
{
   address = str;
}

void US_License::update_city(const QString &str)
{
   city = str;
}

void US_License::update_state(int val)
{
   state = cbb_state->text(val);
}

void US_License::update_zip(const QString &str)
{
   zip = str;
}

void US_License::update_phone(const QString &str)
{
   phone = str;
}

void US_License::update_email(const QString &str)
{
   email = str;
}

void US_License::update_version(int val)
{
   version = cbb_version->text(val);
}

void US_License::update_licensetype(int item)
{
   licensetype = cbb_licensetype->text(item);
}

void US_License::update_code(const QString &str)
{
   code = str;
}

void US_License::update_expiration(const QString &str)
{
   expiration = str;
}

void US_License::update_opteron_rb()
{
   //platform = "opteron";

   //rb_opteron->setChecked(true);
   //rb_intel->setChecked(false);
   //rb_sparc->setChecked(false);
   //rb_mac->setChecked(false);
   //rb_sgi->setChecked(false);
}

void US_License::update_intel_rb()
{
   //platform = "intel";

   //rb_opteron->setChecked(false);
   //rb_intel->setChecked(true);
   //rb_sparc->setChecked(false);
   //rb_mac->setChecked(false);
   //rb_sgi->setChecked(false);
}

void US_License::update_mac_rb()
{
   //platform = "mac";

   //rb_opteron->setChecked(false);
   //rb_intel->setChecked(false);
   //rb_sparc->setChecked(false);
   //rb_mac->setChecked(true);
   //rb_sgi->setChecked(false);
}

void US_License::update_sparc_rb()
{
   //platform = "sparc";

   //rb_opteron->setChecked(false);
   //rb_intel->setChecked(false);
   //rb_sparc->setChecked(true);
   //rb_mac->setChecked(false);
   //rb_sgi->setChecked(false);
}

void US_License::update_sgi_rb()
{
   //platform = "sgi";

   //rb_opteron->setChecked(false);
   //rb_intel->setChecked(false);
   //rb_sparc->setChecked(false);
   //rb_mac->setChecked(false);
   //rb_sgi->setChecked(true);
}

void US_License::update_os(int val)
{
   switch(val)
   {
   case 0:
      {
         os = "linux";
         break;
      }
   case 1:
      {
         os = "win32";
         break;
      }
   case 2:
      {
         os = "osx";
         break;
      }
   case 3:
      {
         os = "irix";
         break;
      }
   case 4:
      {
         os = "solaris";
         break;
      }
   case 5:
      {
         os = "freebsd";
         break;
      }
   case 6:
      {
         os = "netbsd";
         break;
      }
   case 7:
      {
         os = "openbsd";
         break;
      }
   }
}

