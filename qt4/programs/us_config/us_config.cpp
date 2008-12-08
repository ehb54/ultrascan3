//! \file us_config.cpp
#include <QApplication>

#include "us_config.h"
#include "us_defines.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_admin.h"

int main( int argc, char* argv[] )
{
  QApplication application( argc, argv );
  
  // Set up language localization
  QString locale = QLocale::system().name();

  QTranslator translator;
  translator.load( QString( "us_config_" ) + locale );
  application.installTranslator( &translator );
    
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

  // License is OK.  Start up.
  US_Config w;
  w.show();
  return application.exec();
}

///////////////////
US_Config::US_Config( QWidget* parent, Qt::WindowFlags flags )
  : US_Widgets( parent, flags )
{
  font   = NULL;
  db     = NULL;
  colors = NULL;

  if ( ! g.isValid() ) 
  {
    // Do something for invalid global memory
    qDebug( "us_win: invalid global memory" );
  }

  QPoint p = g.global_position();
  g.set_global_position( p + QPoint( 30, 30 ) );
  move( p );

  setWindowTitle( "UltraScan Configuration" );
  setPalette( US_GuiSettings::frameColor() );

  // Directories
  
  QBoxLayout* topbox = new QVBoxLayout( this );

  QLabel* banner = us_banner( tr("UltraScan %1 Configuration" ).arg( US_Version ) );
  topbox->addWidget( banner );

  QLabel* paths  = us_banner( tr("Directories" ) );
  paths->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  topbox->addWidget( paths );

  int row = 0;
  QGridLayout* directories = new QGridLayout();

  // Calculate width for directory pushbuttons
  QFont*        f  = new QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
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

  QString tmpDir = US_Settings::resultDir();
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

  // Temperature Tolerance
  QLabel* temperature = us_label( "Temperature Tolerance (ºC):" );
  otherSettings->addWidget( temperature, row, 0 );

  le_temperature_tol = us_lineedit( QString( "%1" ).arg( US_Settings::tempTolerance() ), 0 );
  otherSettings->addWidget( le_temperature_tol, row++, 1 );

  // Beckman Bug
  QLabel* beckman = us_label( "Beckman Time Bug Correction:" );
  otherSettings->addWidget( beckman, row, 0 );

  QBoxLayout* radiobutton = new QHBoxLayout();

  rb_on = us_radiobutton( tr( "On" ) );
  radiobutton->addWidget( rb_on );

  rb_off = us_radiobutton( tr( "Off" ), Qt::Checked );
  radiobutton->addWidget( rb_off );

  otherSettings->addLayout( radiobutton, row++, 1 );

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

  // Number of Threads
  QLabel* threads = us_label( "Number of Threads:" );
  otherSettings->addWidget( threads, row, 0 );

  // Use a spin box here
  QSpinBox* sb_threads = new QSpinBox();
  sb_threads->setRange( 1, 64 );
  sb_threads->setValue( US_Settings::threads() );
  sb_threads->setFixedWidth( 50 );
  sb_threads->setPalette( US_GuiSettings::editColor() );
  otherSettings->addWidget( sb_threads, row++, 1 );

  // Master Password
  QLabel* password = us_label( "Master Password:" );
  otherSettings->addWidget( password, row, 0 );

  pb_password = us_pushbutton( tr( "Change" ) );
  otherSettings->addWidget( pb_password, row++, 1 );
  connect( pb_password, SIGNAL( clicked() ), this, SLOT( update_password() ) );

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

US_Config::~US_Config()
{
  QPoint p = g.global_position();
  g.set_global_position( p - QPoint( 30, 30 ) );
}
  
void US_Config::help( void )
{
  showhelp.show_help( "manual/config.html" );
}

void US_Config::save( void )
{
  qDebug() << "Save Pushed";
}

void US_Config::update_colors( void )
{
  if ( colors ) delete colors;
  colors = new US_Color;
  colors->show();
}

void US_Config::update_font( void )
{
  if ( font ) delete font;
  font = new US_Font;
  font->show();
}

void US_Config::update_db( void )
{
  if ( db ) delete db;
  db = new US_Database;
  db->show();
}

void US_Config::update_password( void )
{
  US_Admin* admin = new US_Admin;
  admin->show();
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

void US_Config::closeEvent( QCloseEvent* e )
{
  if ( colors ) delete colors;
  if ( db     ) delete db;
  if ( font   ) delete font;

  e->accept();
}
