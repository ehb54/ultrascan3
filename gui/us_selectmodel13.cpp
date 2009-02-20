//! \file us_selectmodel13.cpp
#include "us_selectmodel13.h"
#include "us_gui_settings.h"

US_SelectModel13::US_SelectModel13( float&          temp_stoich1, 
                                    float&          temp_stoich2,
                                    QWidget*        p, 
                                    Qt::WindowFlags f ) 
   : US_WidgetsDialog( p, f ), stoich1( temp_stoich1 ), stoich2( temp_stoich2 )
{
   setPalette( US_GuiSettings::frameColor() );
   setWindowTitle( tr( "Stoichiometry Selection - UltraScan Equilibrium Analysis" ) );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;
   QGridLayout* main = new QGridLayout( this );

   QLabel* lbl_info = us_banner( tr( "Please provide the Stoichiometry\n"
                                     "for the Monomer - N-mer - M-mer\n"
                                     "Equilibrium" ) );

   main->addWidget( lbl_info, row++, 0, 1, 2 );
   
   QLabel* lbl_stoich1 = us_label( tr( "N-mer:" ) );
   main->addWidget( lbl_stoich1, row, 0 );
   
   le_stoich1 = us_lineedit(  QString::number( stoich1 ) );
   main->addWidget( le_stoich1, row++, 1 );

   QLabel* lbl_stoich2 = us_label( tr( "M-mer:" ) );
   main->addWidget( lbl_stoich2, row, 0 );
   
   le_stoich2 = us_lineedit(  QString::number( stoich2 ) );
   main->addWidget( le_stoich2, row++, 1 );

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

void US_SelectModel13::ok( void )
{
   stoich1 = le_stoich1->text().toInt();
   accept();
}

void US_SelectModel13::cancel( void )
{
   stoich1 = 2;
   reject();
}

