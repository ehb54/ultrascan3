//! \file us_selectmodel3.cpp
#include "us_selectmodel3.h"
#include "us_gui_settings.h"

US_SelectModel3::US_SelectModel3( 
      double&         temp_mw_upperLimit, 
      double&         temp_mw_lowerLimit,
      int&            temp_mw_slots, 
      double&         temp_model3_vbar, 
      bool&           temp_model3_vbar_flag, 
      int             temp_InvID,
      QWidget*        p, 
      Qt::WindowFlags f ) 
   : US_WidgetsDialog( p, f ), 
     mwUpperLimit  ( temp_mw_upperLimit ),
     mwLowerLimit  ( temp_mw_lowerLimit ),
     mwSlots       ( temp_mw_slots ),
     model3VbarFlag( temp_model3_vbar_flag ),
     model3Vbar    ( temp_model3_vbar )
{
   InvID = temp_InvID;
   
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Fixed Molecular Weight Distribution" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );

   QLabel* lbl_info = us_banner( tr( "Please provide additional Information" ) );
   main->addWidget( lbl_info, row++, 0, 1, 2 );
   
   QLabel* lbl_mwSlots = us_label( tr( "Number of MW Slots:" ) );
   main->addWidget( lbl_mwSlots, row, 0 );
   
   mwSlots = 100;
   le_mwSlots = us_lineedit(  QString::number( mwSlots ) );
   main->addWidget( le_mwSlots, row++, 1 );

   QLabel* lbl_mwLowerLimit = us_label( tr( "Lower Limit of MW Range:" ) );
   main->addWidget( lbl_mwLowerLimit, row, 0 );
   
   mwLowerLimit = 10000.0;
   le_mwLowerLimit = us_lineedit( QString::number( mwLowerLimit, 'e', 3 ) );
   main->addWidget( le_mwLowerLimit, row++, 1 );

   QLabel* lbl_mwUpperLimit = us_label( tr( "Upper Limit of MW Range:" ) );
   main->addWidget( lbl_mwUpperLimit, row, 0 );
   
   mwUpperLimit = 500000.0;
   le_mwUpperLimit = us_lineedit( QString::number( mwUpperLimit, 'e', 3 ) );
   main->addWidget( le_mwUpperLimit, row++, 1 );

   QLabel* lbl_commonVbar = us_label( tr( "Use common vbar value?" ) );
   main->addWidget( lbl_commonVbar, row, 0 );
   
   model3VbarFlag = true;
   cb_commonVbar = us_checkbox( "", Qt::Checked );
   main->addWidget( cb_commonVbar, row++, 1 );

   QPushButton* pb_vbar = us_pushbutton( tr( "Common vbar Value" ) );
   main->addWidget( pb_vbar, row, 0 );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( read_vbar() ) );
   
   le_vbar = us_lineedit( QString::number( model3Vbar, 'e', 3  ) );
   main->addWidget( le_vbar, row++, 1 );

   QBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   buttons->addWidget( pb_help );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   buttons->addWidget( pb_cancel );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );

   QPushButton* pb_ok = us_pushbutton( tr( "OK" ) );
   buttons->addWidget( pb_ok );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok() ) );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

US_SelectModel3::US_SelectModel3( int&            temp_mw_slots, 
                                  QWidget*        p, 
                                  Qt::WindowFlags f ) 
   : US_WidgetsDialog( p, f ),
     mwUpperLimit  ( (double&) temp_mw_slots ), // dummy
     mwLowerLimit  ( (double&) temp_mw_slots ), // dummy
     mwSlots       (           temp_mw_slots ),
     model3VbarFlag( (bool&)   temp_mw_slots ), // dummy
     model3Vbar    ( (double&) temp_mw_slots )  // dummy
{
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Fixed Molecular Weight Distribution" ) );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );

   QLabel* lbl_info = us_banner( tr( "Please provide additional Information" ) );
   main->addWidget( lbl_info, row++, 0, 1, 2 );
   
   QLabel* lbl_mwSlots = us_label( tr( "Number of MW Slots:" ) );
   main->addWidget( lbl_mwSlots, row, 0 );
   
   mwSlots = 100;
   le_mwSlots = us_lineedit(  QString::number( mwSlots ) );
   main->addWidget( le_mwSlots, row++, 1 );

   QBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   buttons->addWidget( pb_help );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   QPushButton* pb_ok = us_pushbutton( tr( "OK" ) );
   buttons->addWidget( pb_ok );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok2() ) );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   buttons->addWidget( pb_cancel );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
}

void US_SelectModel3::ok( void )
{

   model3Vbar     = le_vbar        ->text().toFloat();
   mwUpperLimit   = le_mwUpperLimit->text().toFloat();
   mwLowerLimit   = le_mwLowerLimit->text().toFloat();
   mwSlots        = le_mwSlots     ->text().toInt();
   model3VbarFlag = cb_commonVbar  ->isChecked();
   accept();
}

void US_SelectModel3::ok2( void )
{
   mwSlots        = le_mwSlots     ->text().toInt();
   accept();
}

void US_SelectModel3::cancel( void )
{
   model3Vbar   = 0.72;
   mwUpperLimit = 0.0;
   mwLowerLimit = 0.0;
   mwSlots      = 0;
   reject();
}

void US_SelectModel3::read_vbar()
{

   QMessageBox::information( this, "N/A", "Not implemented yet" );
   /*
   float vbar = model3Vbar;
   vbar_dlg = new US_Vbar_DB( 20.0, &vbar, &(*model3Vbar), true, false, InvID, this );
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   vbar_dlg->show();

   if ( vbar_dlg ) vbar_dlg->raise(); 

   connect( vbar_dlg, SIGNAL( valueChanged  ( float, float ) ), 
            this    , SLOT  (update_vbar_lbl( float, float ) ) );
   */
}
/*
void US_SelectModel3::update_vbar_lbl( float, float val20 )
{
   model3Vbar = val20;
   le_vbar->setText( QString::number( val20, 'f', 4 ) );
}
*/
