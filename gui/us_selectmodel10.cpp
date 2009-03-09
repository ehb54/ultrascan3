//! \file us_selectmodel10.cpp
#include "us_selectmodel10.h"
#include "us_gui_settings.h"

US_SelectModel10::US_SelectModel10( double&         temp_stoich1, 
                                    QWidget*        p, 
                                    Qt::WindowFlags f ) 
   : US_WidgetsDialog( p, f ), stoich1( temp_stoich1 )
{
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Stoichiometry Selection - UltraScan Equilibrium Analysis" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );

   QLabel* lbl_info = us_banner( tr( "Please provide the Stoichiometry\n"
                                     "for the Monomer- N-mer Equilibrium" ) );
   main->addWidget( lbl_info, row++, 0, 1, 2 );
   
   QLabel* lbl_stoich1 = us_label( tr( "N-mer:" ) );
   main->addWidget( lbl_stoich1, row, 0 );
   
   le_stoich1 = us_lineedit(  QString::number( stoich1 ) );
   main->addWidget( le_stoich1, row++, 1 );

   QBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   buttons->addWidget( pb_help );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   QPushButton* pb_ok = us_pushbutton( tr( "OK" ) );
   buttons->addWidget( pb_ok );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( ok() ) );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   buttons->addWidget( pb_cancel );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

void US_SelectModel10::ok( void )
{
   stoich1 = le_stoich1->text().toInt();
   accept();
}

void US_SelectModel10::cancel( void )
{
   stoich1 = 2;
   reject();
}

