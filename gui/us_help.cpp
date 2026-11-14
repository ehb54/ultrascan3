#include "us_help.h"
#include "us_settings.h"

// Null constructor
US_Help::US_Help( QObject* parent ) : QObject( parent ) { }

// Show a specified help page in US3 manual
void US_Help::show_help( const QString& helpFile )
{
   // Each request starts its own us_helpdaemon, which owns Assistant and
   // reports its own failures. The manual.qhc collection registers the manual.
   QString helpbin = US_Settings::appBaseDir() + "/bin/us_helpdaemon";

#ifdef Q_OS_MAC
   // QProcess cannot pass arguments to a .app bundle, so use the inner binary.
   const QString helpapp_bin = helpbin + ".app/Contents/MacOS/us_helpdaemon";

   if ( !QFile::exists( helpbin ) && QFile::exists( helpapp_bin ) )
   {
      helpbin = helpapp_bin;
   }
#endif

   if ( !QProcess::startDetached( helpbin, { helpFile } ) )
   {
      QMessageBox::warning(
         nullptr, tr( "UltraScan Help" ), tr( "The UltraScan help program could not be started:\n\n%1" ).arg( helpbin ) );
   }
}

// Show a specified URL in the user's configured browser
void US_Help::show_URL( const QString& location )
{
   openBrowser( location );
}

// Show a specified HTML file in the user's configured browser
void US_Help::show_html_file( const QString& location )
{
   openBrowser( "file://" + location );
}

// Open a web page or file in the user's configured browser
void US_Help::openBrowser( const QString& location )
{
   QString program = US_Settings::browser();

   if ( program.isEmpty() )
   {
      // No browser explicitly configured -- use the OS default.
      if ( !QDesktopServices::openUrl( QUrl( location ) ) )
      {
         QMessageBox::warning( 0,
            tr( "Ultrascan III Error" ),
            tr( "Cannot open the system default browser ...\n"
                "Please insure you have a default browser configured "
                "for your operating system or set one explicitly in "
                "UltraScan Configuration.\n\n"
                "  URL: %1" ).arg( location ) );
      }
      return;
   }

   QProcess*   proc    = new QProcess( this );
   QStringList args;

#ifndef Q_OS_MAC
   args << location;
#else
   args << "-a" << program << location;
   program = "open";
#endif

   proc->start( program, args );

   if ( !proc->waitForStarted( 10000 ) )
   {
      QMessageBox::warning( 0,
         tr( "Ultrascan III Error" ),
         tr( "Cannot start browser window ...\n"
             "Please insure you have the configured browser installed.\n\n"
             "  Configured browser: %1\n"
             "  URL: %2" ).arg( program ).arg( location ) );
   }
}
