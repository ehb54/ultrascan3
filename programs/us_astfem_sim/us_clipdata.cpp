#include "us_clipdata.h"
#include "us_gui_settings.h"

US_ClipData::US_ClipData( double& concentration, double& radius, 
      double meniscus, double loading, QWidget* p, Qt::WindowFlags f ) 
  : US_WidgetsDialog( p, f ), conc( concentration ), rad( radius )
{
   setPalette    ( US_GuiSettings::frameColor() );
   setAttribute  ( Qt::WA_DeleteOnClose );
        
   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins ( 2, 2, 2, 2 );
   main->setSpacing( 2 );

   setWindowTitle( tr( "Select Finite Element Simulation"
                       " Data Range" ) );

   QLabel* lb_info    = us_banner( tr( "Please select the Cropping Range\n"
                                       "for the Data to be saved" ) );

   QLabel* lb_conc    = us_label ( tr( "Maximum Concentration:" ) );

   QString s;
   s.sprintf( "%4.2f", loading );

   QLabel* lb_loading = us_label( tr( "( Loading concentration: " )
                        + QString::asprintf( "%4.2f" , loading ) + " )" );

   ct_conc = us_counter( 2, loading, conc, loading * 2 );
   ct_conc->setSingleStep( 0.1 );

   QLabel* lb_rad     = us_label( tr( "Maximum Radius:" ) );

   ct_rad = us_counter( 2, meniscus, rad, rad ); 
   ct_rad->setSingleStep( 0.01 );

   QBoxLayout* buttons    = new QHBoxLayout();
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "OK" ) );

   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()   ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( ok()     ) );

   buttons->addWidget( pb_help );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   QTextEdit*   te_guide  = us_textedit();
   us_setReadOnly( te_guide, true );
   te_guide->setTextColor( Qt::blue );
   te_guide->setText( "<div style='color:blue'>" +
         tr( "To accept the data as computed, with no"
             " limits on amplitude or radius range,"
             " click on the <b>Cancel</b> button.<br/><br/>"
             "To impose amplitude or radius limits on"
             " exported data, set the maximum concentration"
             " value and/or maximum radius and then click"
             " on the <b>OK</b> button.<br/><br/>"
             "( The current maximum OD is %1 )." ).arg( conc )
           + "</div>" );

   int row = 0;
   main->addWidget( lb_info,    row++, 0, 1, 2 );
   main->addWidget( lb_loading, row++, 0, 1, 2 );
   main->addWidget( lb_conc,    row,   0, 1, 1 );
   main->addWidget( ct_conc,    row++, 1, 1, 1 );
   main->addWidget( lb_rad,     row,   0, 1, 1 );
   main->addWidget( ct_rad,     row++, 1, 1, 1 );
   main->addLayout( buttons,    row++, 0, 1, 2 );
   main->addWidget( te_guide,   row++, 0, 5, 2 );

   adjustSize();
qDebug() << "(1) size" << size();
   int rowht  = ct_conc->height();
   int wwid   = width();
   int whgt   = rowht * ( row + 7 );
qDebug() << "(2) size" << QSize( wwid, whgt );
   resize( wwid, whgt );
qDebug() << "(3) size" << size();
}

void US_ClipData::ok( void )
{
   conc = ct_conc->value();
   rad  = ct_rad ->value();
   accept();
}

