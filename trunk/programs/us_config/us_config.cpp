//! \file us_config.cpp
#include <QApplication>

#include "us_config.h"
#include "us_defines.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

int main( int argc, char* argv[] )
{
  QApplication application( argc, argv );

  #include "main1.inc"

  // License is OK.  Start up.
  US_Config w;               
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

///////////////////
US_Config::US_Config( QWidget* parent, Qt::WindowFlags flags )
  : US_Widgets( true, parent, flags )
{
  font   = NULL;
  db     = NULL;
  colors = NULL;
  admin  = NULL;

  setWindowTitle( "UltraScan Configuration" );
  setPalette( US_GuiSettings::frameColor() );

  // Directories
  
  QBoxLayout* topbox = new QVBoxLayout( this );
  topbox->setContentsMargins ( 2, 2, 2, 2 );
  topbox->setSpacing         ( 2 );

  QLabel* banner = us_banner( tr("UltraScan %1 Configuration" ).arg( US_Version ) );
  topbox->addWidget( banner );

  QLabel* paths  = us_banner( tr("Directories" ) );
  paths->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  topbox->addWidget( paths );

  int row = 0;
  QGridLayout* directories = new QGridLayout();

  // Calculate width for directory pushbuttons
  QFont*        f  = new QFont( US_GuiSettings::fontFamily(), 
                                US_GuiSettings::fontSize() );
  QFontMetrics* fm = new QFontMetrics ( *f );
  int w = fm->width( tr( "Temporary Directory:" ) ) + 20;

  // Browser
  pb_browser = us_pushbutton( tr("WWW Browser:" ) );
  pb_browser->setFixedWidth( w );  
  directories->addWidget( pb_browser, row, 0 );
  connect( pb_browser, SIGNAL( clicked() ), this, SLOT( open_browser() ) );

  // Calculate the minimum width of all the le boxes
  QString browser = US_Settings::browser();
  int le_w = fm->width( browser ) + 20;
  
  le_browser = us_lineedit( browser, 0 );
  directories->addWidget( le_browser, row++, 1 );

  // Data Directory
  pb_dataDir = us_pushbutton( tr( "Data Directory:" ) );
  pb_dataDir->setFixedWidth( w );  
  directories->addWidget( pb_dataDir, row, 0 );
  connect( pb_dataDir, SIGNAL( clicked() ), this, SLOT( open_dataDir() ) );

  QString dataDir = US_Settings::dataDir();
  le_w = qMax( fm->width( dataDir ) + 20, le_w );

  le_dataDir = us_lineedit( dataDir, 0 );
  directories->addWidget( le_dataDir, row++, 1 );

  // Result Directory
  pb_resultDir = us_pushbutton( tr( "Result Directory:" ) );
  pb_resultDir->setFixedWidth( w );  
  directories->addWidget( pb_resultDir, row, 0 );
  connect( pb_resultDir, SIGNAL( clicked() ), this, SLOT( open_resultDir() ) );

  QString resultDir = US_Settings::resultDir();
  le_w = qMax( fm->width( resultDir ) + 20, le_w );

  le_resultDir = us_lineedit( resultDir, 0 );
  directories->addWidget( le_resultDir, row++, 1 );

  // Report Directory
  pb_reportDir = us_pushbutton( tr( "HTML Reports:" ) );
  pb_reportDir->setFixedWidth( w );  
  directories->addWidget( pb_reportDir, row, 0 );
  connect( pb_reportDir, SIGNAL( clicked() ), this, SLOT( open_reportDir() ) );

  QString reportDir = US_Settings::reportDir();
  le_w = qMax( fm->width( reportDir ) + 20, le_w );

  le_reportDir = us_lineedit( reportDir, 0 );
  directories->addWidget( le_reportDir, row++, 1 );

  // Archive Directory
  pb_archiveDir = us_pushbutton( tr( "Archive Directory:" ) );
  pb_archiveDir->setFixedWidth( w );  
  
  directories->addWidget( pb_archiveDir, row, 0 );
  connect( pb_archiveDir, SIGNAL( clicked() ), this, SLOT( open_archiveDir() ) );

  QString archiveDir = US_Settings::archiveDir();
  le_w = qMax( fm->width( archiveDir ) + 20, le_w );

  le_archiveDir = us_lineedit( archiveDir, 0 );
  directories->addWidget( le_archiveDir, row++, 1 );

  // Temporary Directory
  pb_tmpDir = us_pushbutton( tr( "Temporary Directory:" ) );
  pb_tmpDir->setFixedWidth( w );  
  directories->addWidget( pb_tmpDir, row, 0 );
  connect( pb_tmpDir, SIGNAL( clicked() ), this, SLOT( open_tmpDir() ) );

  QString tmpDir = US_Settings::tmpDir();
  le_w = qMax( fm->width( tmpDir ) + 20, le_w );

  le_tmpDir = us_lineedit( tmpDir, 0 );
  directories->addWidget( le_tmpDir, row++, 1 );
  
  le_browser   ->setMinimumWidth( le_w );
  le_dataDir   ->setMinimumWidth( le_w );
  le_resultDir ->setMinimumWidth( le_w );
  le_reportDir ->setMinimumWidth( le_w );
  le_archiveDir->setMinimumWidth( le_w );
  le_tmpDir    ->setMinimumWidth( le_w );

  topbox->addLayout( directories );

  QLabel* misc  = us_banner( tr("Miscellaneous Settings" ) );
  misc->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  topbox->addWidget( misc );

  // Misc Settings
  row = 0;
  QGridLayout* otherSettings = new QGridLayout();

/* Not used any more.  Hard coded to 0.5C.
  // Temperature Tolerance
  QLabel* temperature = us_label( tr( "Temperature Tolerance (" ) + DEGC
        + "):" );
  otherSettings->addWidget( temperature, row, 0 );

  sb_temperature_tol = new QDoubleSpinBox();
  sb_temperature_tol->setRange( 0.1, 1.0 );
  sb_temperature_tol->setSingleStep( 0.1 );
  sb_temperature_tol->setDecimals( 1 );
  sb_temperature_tol->setValue( US_Settings::tempTolerance() );
  sb_temperature_tol->setPalette( US_GuiSettings::editColor() );
  sb_temperature_tol->setFont( QFont( US_GuiSettings::fontFamily(), 
                                      US_GuiSettings::fontSize() ) );
  otherSettings->addWidget( sb_temperature_tol, row++, 1 );
*/


/*  Not used any more
  // Beckman Bug
  QLabel* beckman = us_label( "Beckman Time Bug Correction:" );
  otherSettings->addWidget( beckman, row, 0 );

  QGridLayout* radiobutton = new QGridLayout();
  radiobutton->setSpacing        ( 0 );
  radiobutton->setContentsMargins( 0, 0, 0, 0 );

  QLabel* lb_background = new QLabel;
  lb_background->setAutoFillBackground( true );
  lb_background->setPalette( US_GuiSettings::normalColor() );

  QGridLayout* rb1 = us_radiobutton( tr( "On" ), rb_on );
  radiobutton->addLayout( rb1, 0, 0 );

  QGridLayout* rb2 = us_radiobutton( tr( "Off" ), rb_off, true );
  radiobutton->addLayout( rb2, 0, 1 );

  otherSettings->addLayout( radiobutton, row++, 1 );
*/
  // Disk/DB preference
  QLabel* lb_disk_db = us_label( "Default Data Location:" );
  otherSettings->addWidget( lb_disk_db, row, 0 );

  disk_db_control = new US_Disk_DB_Controls( US_Disk_DB_Controls::Default );
  otherSettings->addLayout( disk_db_control, row++, 1 );
  connect( disk_db_control, SIGNAL( changed        ( bool ) ), 
                            SLOT( set_data_location( bool ) ) );
  // Color Preferences
  QLabel* color = us_label( "Color Preferences:" );
  otherSettings->addWidget( color, row, 0 );

  pb_color = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_color, row++, 1 );
  connect( pb_color, SIGNAL( clicked() ), this, SLOT( update_colors() ) );

  // Font Preferences
  QLabel* font = us_label( "Font Preferences:" );
  otherSettings->addWidget( font, row, 0 );

  pb_font = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_font, row++, 1 );
  connect( pb_font, SIGNAL( clicked() ), this, SLOT( update_font() ) );

  // Database Preferences
  QLabel* db = us_label( "Database Preferences:" );
  otherSettings->addWidget(db, row, 0 );

  pb_db = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_db, row++, 1 );
  connect( pb_db, SIGNAL( clicked() ), this, SLOT( update_db() ) );

  // Master Password
  QLabel* password = us_label( "Master Password:" );
  otherSettings->addWidget( password, row, 0 );

  pb_password = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_password, row++, 1 );
  connect( pb_password, SIGNAL( clicked() ), this, SLOT( update_password() ) );

  // Advanced settings
  QLabel* advanced = us_label( "Advanced Settings:" );
  otherSettings->addWidget( advanced, row, 0 );

  pb_advanced = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_advanced, row++, 1 );
  connect( pb_advanced, SIGNAL( clicked() ), this, SLOT( update_advanced() ) );

  topbox->addLayout( otherSettings );

  // Pushbuttons
  pb_help = us_pushbutton( tr( "Help" ) );
  connect( pb_help, SIGNAL( clicked() ), this, SLOT( help() ) );
  
  pb_save = us_pushbutton( tr( "Save" ) );
  connect( pb_save, SIGNAL( clicked() ), this, SLOT( save() ) );

  pb_cancel = us_pushbutton( tr( "Close" ) );
  connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( close() ) );

  QBoxLayout* buttons = new QHBoxLayout();
  buttons->addWidget( pb_save   );
  buttons->addWidget( pb_help   );
  buttons->addWidget( pb_cancel );
  topbox->addLayout( buttons );
}

void US_Config::help( void )
{
  showhelp.show_help( "config.html" );
}

void US_Config::save( void )
{
//   US_Settings::set_tempTolerance( sb_temperature_tol->value() );
   US_Settings::set_browser      ( le_browser    ->text()      );
   US_Settings::set_dataDir      ( le_dataDir    ->text()      );
   US_Settings::set_resultDir    ( le_resultDir  ->text()      );
   US_Settings::set_reportDir    ( le_reportDir  ->text()      );
   US_Settings::set_archiveDir   ( le_archiveDir ->text()      );
   US_Settings::set_tmpDir       ( le_tmpDir     ->text()      );
//   US_Settings::set_beckmanBug   ( rb_on         ->isChecked() );

   QMessageBox::information( this,
         tr( "Settings Saved" ),
         tr( "The settings were successfully saved" ) );
}

void US_Config::set_data_location( bool )
{
   if ( disk_db_control->db() ) 
      US_Settings::set_default_data_location( US_Disk_DB_Controls::DB );
   else
      US_Settings::set_default_data_location( US_Disk_DB_Controls::Disk );
}

void US_Config::update_colors( void )
{
  colors = new US_Color; // Automatic delete
  colors->show();
}

void US_Config::update_font( void )
{
  font = new US_Font;  // Automatic delete
  font->show();
}

void US_Config::update_db( void )
{
  db = new US_Database; // Automatic delete
  db->show();
}

void US_Config::update_password( void )
{
  admin = new US_Admin;  // Automatic delete
  admin->show();
}

void US_Config::update_advanced( void )
{
   advanced = new US_Advanced;
   advanced->show();
}

void US_Config::open_browser( void )
{
  QString browser = QFileDialog::getOpenFileName( this,
         tr( "Select desired browser application" ), le_browser->text() );

  if ( browser != "" ) le_browser->setText( browser );
}

void US_Config::open_dataDir( void )
{
  QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select desired data directory" ), le_dataDir->text() );

  if ( dir != "" ) le_dataDir->setText( dir );
}

void US_Config::open_resultDir( void )
{
  QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select desired result directory" ), le_resultDir->text() );

  if ( dir != "" ) le_resultDir->setText( dir );
}

void US_Config::open_reportDir( void )
{
  QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select desired report directory" ), le_reportDir->text() );

  if ( dir != "" ) le_reportDir->setText( dir );
}

void US_Config::open_archiveDir( void )
{
  QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select desired archive directory" ), le_archiveDir->text() );

  if ( dir != "" ) le_archiveDir->setText( dir );
}

void US_Config::open_tmpDir( void )
{
  QString dir = QFileDialog::getExistingDirectory( this,
         tr( "Select desired temporary directory" ), le_tmpDir->text() );

  if ( dir != "" ) le_tmpDir->setText( dir );
}

