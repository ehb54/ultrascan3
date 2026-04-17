/*! \file main1.inc

 \brief A standard main program for UltraScan III processes.

 This code fragment adds internationalization and checks for a 
 proper UltraScan license prior to launching the main program.

 \code
 Usage:
   // Other includes 

int main( int argc, char* argv[] )
{
  QApplication application( argc, argv );

  #include "main.1"
  // License is OK.  Start up.

  US_Config w;  // Use main class of choice here
  w.show();
  return application.exec();
  \endcode
}

*/

//! \brief Standard main program startup include file

  // Set up language localization
  QString locale = QLocale::system().name();
  QString program = application.applicationFilePath();
  program.remove( QRegularExpression( "/.*/" ) );

  QTranslator translator1;
  translator1.load( program + "_" + locale );
  application.installTranslator( &translator1 );

  QTranslator translator2;
  translator2.load( QString( "us_gui_" ) + locale );
  application.installTranslator( &translator2 );

  QTranslator translator3;
  translator3.load( QString( "us_utils_" ) + locale );
  application.installTranslator( &translator3 );

  QTranslator translator4;
  translator3.load( QString( "us_db_" ) + locale );
  application.installTranslator( &translator4 );

  // See if we need to update the license
  QString ErrorMessage;

  int result = US_License_t::isValid( ErrorMessage );
  if ( result != US_License_t::OK )
  {
    QMessageBox mBox;

    QPushButton* cancel   = mBox.addButton( QMessageBox::Cancel );
    QPushButton* Register = mBox.addButton( qApp->translate( "UltraScan", "Register"),
        QMessageBox::ActionRole);

    mBox.setDefaultButton( Register );
    mBox.setWindowTitle  ( qApp->translate( "UltraScan", "UltraScan License Problem" ) );
    mBox.setText         ( ErrorMessage );
    mBox.setIcon         ( QMessageBox::Critical );
    mBox.exec();

    if ( mBox.clickedButton() == cancel )  exit( -1 );

    US_License* license = new US_License();
    license->show();
    return application.exec();
  }
