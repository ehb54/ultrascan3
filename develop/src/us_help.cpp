#include "../include/us_util.h"

US_Help::US_Help(QWidget *parent, const char* name) : QWidget( parent, name )
{
  USglobal = new US_Config();
}

//destructor
US_Help::~US_Help()
{
}

void US_Help::show_help( QString helpFile )
{
  QString help = QDir::convertSeparators( 
      USglobal->config_list.help_dir + "/" + helpFile );

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
  proc = new QProcess( this );
  
  connect( proc, SIGNAL( readyReadStdout() ), 
           this, SLOT  ( captureStdout  () ) );
  
  connect( proc, SIGNAL( readyReadStderr() ), 
           this, SLOT  ( captureStderr  () ) );
  
  connect( proc, SIGNAL( processExited() ), 
           this, SLOT  ( endProcess   () ) );
  
  proc->clearArguments();

  stderrSize = 0;
  trials     = 0;

  proc->addArgument( USglobal->config_list.browser );

#if defined(UNIX) && ! defined(MAC)
  // Optimized for Firefox
  proc->addArgument( "-remote" );
  proc->addArgument( "openURL(" + URL + ", new-window)" ); 
#else
  proc->addArgument( URL );
#endif

  if ( ! proc->start() ) // Error
  {
    QMessageBox::message(
        tr( "UltraScan Error:" ), 
        tr( "Can't start browser window...\n"
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
      QMessageBox::message(
          tr( "UltraScan Error:" ), 
          tr( "Can't start browser window...\n"
              "Please make sure you have the configured browser installed\n\n"
              "Currently configured: " + USglobal->config_list.browser ) );
      return;
    }
  }
#endif
}

void US_Help::captureStdout()
{
  //cout << proc->readLineStdout() << endl;
}

void US_Help::captureStderr()
{
  QByteArray list = proc->readStderr();
  stderrSize      = list.size();
  
  //cout << "The following error occured while attempting to run Mozilla:\n" 
  //     << QString(list) << endl;
}

