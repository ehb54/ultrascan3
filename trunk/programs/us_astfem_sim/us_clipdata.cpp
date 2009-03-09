#include "us_clipdata.h"
#include "us_gui_settings.h"

US_ClipData::US_ClipData( double& concentration, double& radius, 
      double meniscus, double loading, QWidget* p, Qt::WindowFlags f ) 
  : US_WidgetsDialog( p, f ), conc( concentration ), rad( radius )
{
   setWindowTitle( tr( "Data Range Selection - "
                       "UltraScan Finite Element Simulation" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setAttribute  ( Qt::WA_DeleteOnClose );
        
   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   int row = 0;

   QLabel* lb_info = us_banner( "Please select the Cropping Range\n"
                                "for the Data to be saved" );
   main->addWidget( lb_info, row++, 0, 1, 2 );

   QLabel* lb_conc = us_label (tr( "Maximum Concentration\n"
                                   "( 0 = entire range ):") );
   main->addWidget( lb_conc, row, 0, 2, 1 );

   QString s;
   s.sprintf( "%4.2f", loading );

   QLabel* lb_loading = us_label( tr( "( Loading concentration: " ) + s + " )" );
   main->addWidget( lb_loading, row, 1 );

   ct_conc = us_counter( 2, loading, conc, loading * 2 );
   ct_conc->setStep( 0.1 );
   main->addWidget( ct_conc, row + 1, 1 );
   row += 2;

   QLabel* lb_rad = us_label( tr( "Maximum Radius:" ) );
   main->addWidget( lb_rad, row, 0 );

   ct_rad = us_counter( 2, meniscus, rad, rad ); 
   ct_rad->setStep( 0.01 );
   main->addWidget( ct_rad, row++, 1 );

   QBoxLayout* buttons = new QHBoxLayout();

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons ->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons ->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "OK" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( ok() ) );
   buttons ->addWidget( pb_accept );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

void US_ClipData::ok( void )
{
   conc = ct_conc->value();
   rad  = ct_rad ->value();
   accept();
}

