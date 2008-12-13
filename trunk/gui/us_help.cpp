#include "us_help.h"
#include "us_settings.h"

US_Help::US_Help( QWidget* parent ) : QWidget( parent )
{
  assistant = new QProcess;
  proc      = new QProcess;

  connect( proc, SIGNAL( readyReadStandardOutput() ), 
           this, SLOT  ( captureStdout()           ) );
  
  connect( proc, SIGNAL( readyReadStandardError() ), 
           this, SLOT  ( captureStderr()          ) );
  
  connect( proc, SIGNAL( finished  ( int, QProcess::ExitStatus ) ), 
           this, SLOT  ( endProcess( int, QProcess::ExitStatus ) ) );
};

void US_Help::show_help( const QString& helpFile )
{
  QStringList args;
  args << helpFile;
  assistant->start( "us_helpdaemon", args );
  // Don't bother to wait
}

void US_Help::show_URL( const QString& location )
{
  URL = location;
  openBrowser();
}

void US_Help::openBrowser()
{
  QString     browser = US_Settings::browser();
  QStringList arguments;

  stderrSize = 0;
  trials     = 0;

#ifndef WIN32
  arguments << "-remote" << "openURL(" + URL + ", new-window)";
#else
  arguments << URL;
#endif
  proc->start( browser, arguments );

  if ( ! proc->waitForStarted( 10000 ) ) // 10 second timeout
  {
    QMessageBox::information( this, 
        tr( "UltraScan Error:" ), 
        tr( QString( "Can't start browser window...\n"
            "Please make sure you have the configured browser installed\n\n" 
            "Currently configured: " + browser ).toAscii() ) );

    qDebug() << "Error: Can't start browser window...\n"
             << "Please make sure you have the configured browser installed\n\n"
             << "Currently configured: " << browser;
  }
}

void US_Help::endProcess( int /* exitCode */, QProcess::ExitStatus /* exitStatus */ )
{
#ifndef WIN32
  QString browser = US_Settings::browser();

  trials++; 
  
  if ( trials > 0 && stderrSize > 0 ) // Error attaching to already running
                                      // process, start new
  {
    stderrSize = 0;
    QStringList arguments;
    arguments << URL;

    proc->start( browser, arguments );

    if ( ! proc->waitForStarted( 10000 ) ) // 10 second timeout
    {
      qDebug() << "Error: Can't start browser window...\n"
               << "Please make sure you have the configured browser installed\n\n"
               << "Currently configured: " << browser;

      QMessageBox::information( this,
          tr( "UltraScan Error:" ), 
          tr( QString( "Can't start browser window...\n"
              "Please make sure you have the configured browser installed\n\n"
              "Currently configured: " + browser ).toAscii() ) );
    }
  }
#endif
}

void US_Help::captureStdout()
{
  //qDebug() << "stdout: " << proc->readAllStandardOutput();
}

void US_Help::captureStderr()
{
  QByteArray list = proc->readAllStandardError();
  stderrSize      = list.size();
  
  //qDebug() << "The following error occured while attempting to run the browser:\n" 
  //         << QString(list);
}

