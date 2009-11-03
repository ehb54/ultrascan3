//! \file us_progressbar.cpp

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_progressbar.h"

US_Progressbar::US_Progressbar( void ) : US_WidgetsDialog( 0, 0 )
{
   US_Progressbar( 0, 100, 0 );
}

US_Progressbar::US_Progressbar( int low, int high, int value ) : US_WidgetsDialog( 0, 0 )
{
   reset();

   setWindowTitle( tr( "Progress..." ) );
   setPalette( US_GuiSettings::frameColor() );

   // Everything will be in the main layout
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   // Progress bar
   QLabel* lb_placeholder = new QLabel();
   main -> addWidget( lb_placeholder, row, 0, 1, 2 );

   lb_progress = us_label( tr( "Progress:" ) , -1 );
   lb_progress->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   main       ->addWidget( lb_progress, row, 0 );

   progress = us_progressBar( low, high, value );
   progress -> reset();
   progress -> setVisible( true );
   main     -> addWidget( progress, row++, 1 );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   buttons->addWidget( pb_cancel );

   main ->setColumnMinimumWidth( 1, 200 );
   main ->addLayout( buttons, row++, 1 );

}

void US_Progressbar::reset( void )
{
}

void US_Progressbar::cancel( void )
{
   this->hide();
   emit cancelConvertOperation();
}

void US_Progressbar::setLegend( QString legend )
{
   lb_progress ->setText( legend );
}

void US_Progressbar::setValue( int value )
{
   progress->setValue( value );
   qApp ->processEvents();
}

void US_Progressbar::setRange( int low, int high )
{
   progress ->setRange( low, high );
}
