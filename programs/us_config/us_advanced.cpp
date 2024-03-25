//! \file us_advanced.cpp
#include "us_advanced.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_help.h"

US_Advanced::US_Advanced( QWidget* w, Qt::WindowFlags flags ) 
  : US_Widgets( true, w, flags )
{
  // Frame layout
  setPalette( US_GuiSettings::frameColor() );

  setWindowTitle( "Advanced Settings" );
  setAttribute( Qt::WA_DeleteOnClose );

  // Set up the database list window
  QLabel* banner = us_banner( tr( "Debug Text List" ) );

  QBoxLayout* topbox = new QVBoxLayout( this );
  topbox->addWidget( banner );
  topbox->setSpacing( 2 );

  te_dbgtext = new QTextEdit();
  te_dbgtext->setPalette(  US_GuiSettings::editColor() );
  te_dbgtext->setFont(     QFont( US_GuiSettings::fontFamily(),
                                  US_GuiSettings::fontSize() ) );
  us_setReadOnly( te_dbgtext, false );

  topbox->addWidget( te_dbgtext );

  // Detail info
  QLabel* info = us_banner( tr( "Advanced Counters" ) );
  topbox->addWidget( info );

  int row = 0;
  QGridLayout* details = new QGridLayout();
  
  // Row 1
  QLabel* lb_debug    = us_label( "Debug Level:" );
  details->addWidget( lb_debug,    row,   0, 1, 3 );

  sb_debug            = new QSpinBox;
  sb_debug->setRange( 0, 4 );
  sb_debug->setValue( 0 );
  sb_debug->setPalette( US_GuiSettings::editColor() );
  sb_debug->setFont( QFont( US_GuiSettings::fontFamily(),
                            US_GuiSettings::fontSize() ) );
  details->addWidget( sb_debug,    row++, 3, 1, 1 );

  // Row 2
  QLabel* lb_advanced = us_label( "Advanced Level:" );
  details->addWidget( lb_advanced, row,   0, 1, 3 );

  sb_advanced         = new QSpinBox;
  sb_advanced->setRange( 0, 4 );
  sb_advanced->setValue( 0 );
  sb_advanced->setPalette( US_GuiSettings::editColor() );
  sb_advanced->setFont( QFont( US_GuiSettings::fontFamily(),
                               US_GuiSettings::fontSize() ) );
  details->addWidget( sb_advanced, row++, 3, 1, 1 );

  // Row 3
  QLabel* lb_threads  = us_label( "Number of Threads:" );
  details->addWidget( lb_threads,  row,   0, 1, 3 );

  sb_threads          = new QSpinBox;
  sb_threads->setRange( 1, 64 );
  sb_threads->setValue( 1 );
  sb_threads->setPalette( US_GuiSettings::editColor() );
  sb_threads->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );
  details->addWidget( sb_threads,  row++, 3, 1, 1 );

  // Row 4
  QLabel* lb_noisdiag  = us_label( "Noise Dialog Flag:" );
  details->addWidget( lb_noisdiag,  row,   0, 1, 3 );

  sb_noisdiag          = new QSpinBox;
  sb_noisdiag->setRange( 0, 1 );
  sb_noisdiag->setValue( 0 );
  sb_noisdiag->setPalette( US_GuiSettings::editColor() );
  sb_noisdiag->setFont( QFont( US_GuiSettings::fontFamily(),
                               US_GuiSettings::fontSize() ) );
  sb_noisdiag->setToolTip( tr( "Flag: 0 -> Auto-load noises;"
                               " 1 -> Pop up noise load dialog" ) );
  details->addWidget( sb_noisdiag,  row++, 3, 1, 1 );

  topbox->addLayout( details );

  //Pushbuttons

  row = 0;
  QGridLayout* buttons = new QGridLayout();

  pb_reset = us_pushbutton( tr( "Reset to Last Saved" ) );
  connect( pb_reset, SIGNAL( clicked() ), this, SLOT( reset() ) );
  buttons->addWidget( pb_reset, row++, 0, 1, 3 );

  pb_save   = us_pushbutton( tr( "Save" ) );
  connect( pb_save,  SIGNAL( clicked() ), this, SLOT( save() ) );
  buttons->addWidget( pb_save,  row,   0, 1, 1 );

  pb_help   = us_pushbutton( tr( "Help" ) );
  connect( pb_help,  SIGNAL( clicked() ), this, SLOT( help() ) );
  buttons->addWidget( pb_help,  row,   1, 1, 1 );

  pb_close  = us_pushbutton( tr( "Close" ) );
  connect( pb_close, SIGNAL( clicked() ), this, SLOT( close() ) );
  buttons->addWidget( pb_close, row++, 2, 1, 1 );

  topbox->addLayout( buttons );

  reset();
}

// set values as they were at last save
void US_Advanced::reset()
{
   QStringList dbg_text  = US_Settings::debug_text();
   int         dbg_level = US_Settings::us_debug();
   int         adv_level = US_Settings::advanced_level();
   int         threads   = US_Settings::threads();
   int         noisdiag  = US_Settings::noise_dialog();

   QString dbg_str = "";

   if ( dbg_text.size() > 0 )
   {

      for ( int ii = 0; ii < dbg_text.size() - 1; ii++ )
         dbg_str += dbg_text.at( ii ) + "\n";

      dbg_str += dbg_text.last();
   }

   te_dbgtext ->setPlainText(  dbg_str   );

   sb_debug   ->setValue(      dbg_level );
   sb_advanced->setValue(      adv_level );
   sb_threads ->setValue(      threads   );
   sb_noisdiag->setValue(      noisdiag  );
}

// save the values from the current GUI elements
void US_Advanced::save()
{
   QStringList dbg_text  = te_dbgtext->toPlainText().split( "\n" );

   for ( int ii = 0; ii < dbg_text.size(); ii++ )
   {
      dbg_text.replace( ii, dbg_text.at( ii ).simplified() );
   }


   US_Settings::set_debug_text(     dbg_text  );
   US_Settings::set_us_debug(       sb_debug   ->value() );
   US_Settings::set_advanced_level( sb_advanced->value() );
   US_Settings::set_threads(        sb_threads ->value() );
   US_Settings::set_noise_dialog(   sb_noisdiag->value() );

   QMessageBox::information( this,
         tr( "Settings Saved" ),
         tr( "The settings have been saved." ) );
}

void US_Advanced::help( void )
{
  US_Help* showhelp = new US_Help(this);
  showhelp->show_help( "advanced_config.html" );
}

