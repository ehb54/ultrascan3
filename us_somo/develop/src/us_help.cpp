#include "../include/us_util.h"

// #ifdef WIN32
// #include <QDesktopServices.h>
// #endif

US_Help::US_Help(QWidget *parent, const char* ) : QWidget( parent )
{
  USglobal = new US_Config();
}

//destructor
US_Help::~US_Help()
{
}

void US_Help::show_help( QString helpFile )
{
  QString help = QDir::toNativeSeparators( 
      USglobal->config_list.help_dir + "/" + helpFile );

  if ( !QFileInfo( help ).exists() ) {
     help =
        QDir::toNativeSeparators( 
                                USglobal->config_list.help_dir 
                                + "/manual/somo/underconstruction.html"
                                 );
  }
  URL = "file://" + help;
  openBrowser();
}

void US_Help::show_URL( QString location )
{
  URL = location;
  openBrowser();
}

void US_Help::show_html_file( QString helpFile )
{
  URL = "file://" + helpFile;
  openBrowser();
}

void US_Help::openBrowser()
{
   // removed since windows (10?) doesn't seem to honor the default browser setting
   // by removing the user can configure their favorite browser in us3_config
   // and it will be initialized in us_config.cpp to the QSettings() value
// #ifdef WIN32
//    QDesktopServices::openUrl( QUrl( URL, QUrl::TolerantMode ) );
//    return;
// #endif
   

#if QT_VERSION < 0x040000
  proc = new QProcess( this );
# ifdef Q_OS_MAC
  proc->addArgument( "open" );
  proc->addArgument( "-a" );
# endif
  proc->addArgument( USglobal->config_list.browser );
  proc->addArgument( URL );

  if ( ! proc->start() ) // Error
  {
    US_Static::us_message(
        us_tr( "UltraScan Error:" ), 
        us_tr( "Can't start browser window...\n"
            "Please make sure you have the configured browser installed\n\n"
            "Currently configured: " + USglobal->config_list.browser ) );
  }
#else
  {
     QProcess * process = new QProcess( this );
     QString prog = USglobal->config_list.browser;
     QStringList args;
# ifdef Q_OS_MAC
     args
        << "-a"
        << prog
        ;
     prog = "open";
# endif
     args
        <<  URL
        ;

     if ( !process->startDetached( prog, args ) ) {
        US_Static::us_message(
                             us_tr( "UltraScan Error:" ), 
                             us_tr( "Can't start browser window...\n"
                                 "Please make sure you have the configured browser installed\n\n"
                                 "Currently configured: " + USglobal->config_list.browser ) );
     }
  }
#endif
}


// These are just here to avoid recompiling everything if us_help.h
// is changed.
void US_Help::endProcess(){}
void US_Help::captureStdout(){}
void US_Help::captureStderr(){}

// Removed the -remote and 2nd try logic because it isn't needed any more.


 /* 
  connect( proc, SIGNAL( readyReadStandardOutput() ), 
           this, SLOT  ( captureStdout  () ) );
  
  connect( proc, SIGNAL( readyReadStandardError() ), 
           this, SLOT  ( captureStderr  () ) );
  
  connect( proc, SIGNAL( finished( int, QProcess::ExitStatus ) ), 
           this, SLOT  ( endProcess   () ) );
  
  proc->clearArguments();

  stderrSize = 0;
  trials     = 0;

  proc->addArgument( USglobal->config_list.browser );
  proc->addArgument( URL );

  if ( ! proc->start() ) // Error
  {
    US_Static::us_message(
        us_tr( "UltraScan Error:" ), 
        us_tr( "Can't start browser window...\n"
            "Please make sure you have the configured browser installed\n\n"
            "Currently configured: " + USglobal->config_list.browser ) );
  }
}

void US_Help::endProcess()
{
#ifdef UNIX
  trials++; 
  
  if ( trials == 1 && stderrSize > 0 ) // Error attaching to already running
                                       // process, start new
  {
    proc->clearArguments();
    proc->addArgument( USglobal->config_list.browser );
    proc->addArgument(URL); 

    if ( ! proc->start() ) // Error
    {
      US_Static::us_message(
          us_tr( "UltraScan Error:" ), 
          us_tr( "Can't start browser window...\n"
              "Please make sure you have the configured browser installed\n\n"
              "Currently configured: " + USglobal->config_list.browser ) );
      return;
    }
  }
#endif
}

void US_Help::captureStdout()
{
  cout << "std: " << proc->readLineStdout().toLatin1().data() << endl;
}

void US_Help::captureStderr()
{
  QByteArray list = proc->readStderr();
  stderrSize      = list.size();
  
  cout << "err: The following error occured while attempting to run Mozilla:\n" 
       << QString(list) << endl;
}
*/
