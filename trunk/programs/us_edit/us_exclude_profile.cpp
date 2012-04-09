//! \file us_exclude_profile.cpp

#include "us_exclude_profile.h"
#include "us_settings.h"
#include "us_gui_settings.h"


US_ExcludeProfile::US_ExcludeProfile( QList< int > includes )
   : US_WidgetsDialog( 0, 0 )
{
   original = includes;
   finished = false;
   current.clear();

   setWindowTitle( tr( "Scan Exclusion Profile Editor" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   int row       = 0;
   int scanCount = original.size();

   // Row
   QLabel* lb_banner = us_banner( tr( "Create a Scan Exclusion Profile" ) );
   main->addWidget( lb_banner, row++, 0, 1, 2 );

   // Row
   QLabel* lb_start = us_label( tr( "Start Exclusion at Scan:" ) );
   main->addWidget( lb_start, row, 0 );

   ct_start = us_counter( 3, 1.0, scanCount, 1.0 );
   ct_start->setStep( 1.0 );
   QFontMetrics fm( ct_start->font() );
   ct_start->setMinimumWidth( fm.maxWidth() * 10 );

   connect( ct_start, SIGNAL( valueChanged ( double ) ),
                      SLOT  ( update_start ( double ) ) );
   main->addWidget( ct_start, row++, 1 );

   // Row
   QLabel* lb_stop = us_label( tr( "Stop Exclusion at Scan:" ) );
   main->addWidget( lb_stop, row, 0 );

   ct_stop = us_counter( 3, 1.0, scanCount, scanCount );
   ct_stop->setStep( 1.0 );
   connect( ct_stop, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( update_stop  ( double ) ) );
   main->addWidget( ct_stop, row++, 1 );

   // Row
   QLabel* lb_nth = us_label( tr( "Include every nth Scan:" ) );
   main->addWidget( lb_nth, row, 0 );

   ct_nth = us_counter( 2, 1.0, scanCount, 1.0 );
   ct_nth->setStep( 1.0 );
   connect( ct_nth, SIGNAL( valueChanged ( double ) ),
                    SLOT  ( update       ( double ) ) );
   main->addWidget( ct_nth, row++, 1 );

   // Row
   QLabel* lb_remaining = us_label( tr( "Remaining Scans:" ) );
   main->addWidget( lb_remaining, row, 0 );

   le_remaining = us_lineedit( QString::number( scanCount ) + tr( " scans" ), -1, true );
   main->addWidget( le_remaining, row++, 1 );

   // Row
   QLabel* lb_excluded = us_label( tr( "Excluded Scans:" ) );
   main->addWidget( lb_excluded, row, 0 );

   le_excluded = us_lineedit( tr( "0 scans" ), -1, true );
   main->addWidget( le_excluded, row++, 1 );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;
   
   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( terminate() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_apply = us_pushbutton( tr( "Apply" ) );
   connect( pb_apply, SIGNAL( clicked() ), SLOT( apply() ) );
   buttons->addWidget( pb_apply );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( done() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row++, 0, 1, 2 );
}

void US_ExcludeProfile::update_start( double v )
{
   if ( ct_stop->value() < v )
   {
      ct_stop->disconnect();
      ct_stop->setValue( v );
      connect( ct_stop, SIGNAL( valueChanged ( double ) ),
                        SLOT  ( update_stop  ( double ) ) );
   }
   update();
}

void US_ExcludeProfile::update_stop( double v )
{
   if ( ct_start->value() > v )
   {
      ct_start->disconnect();
      ct_start->setValue( v );
      connect( ct_start, SIGNAL( valueChanged ( double ) ),
                         SLOT  ( update_start ( double ) ) );
   }
   update();
}

void US_ExcludeProfile::apply( void )
{
   current = excludes;
   ct_start->setValue( 1.0 );
   ct_stop ->setValue( original.size() );
   ct_nth  ->setValue( 1.0 );
}

void US_ExcludeProfile::update( double /* unused */ )
{
   int start = (int)ct_start->value() - 1;  // Excludes are 0 based
   int stop  = (int)ct_stop ->value() - 1;
   int nth   = (int)ct_nth  ->value();

   excludes = current;

   // Handle nth
   for ( int i = start + 1; i <= stop; i++ )
      if ( ( i - start ) % nth != 0  &&  ! excludes.contains( i ) ) excludes << i;

   int remaining = original.size() - excludes.size();
   int excluded  = excludes.size();

   QString scans = ( remaining == 1 ) ? tr( " scan" ) : tr( " scans" );
   le_remaining->setText( QString::number( remaining ) + scans );

   scans = ( excluded == 1 ) ? tr( " scan" ) : tr( " scans" );
   le_excluded->setText( QString::number( excluded ) + scans );

   if ( finished )
   {
      if ( remaining < 10 )
      {
         int result = QMessageBox::question( this,
               tr( "Exclude Error" ),
               tr( "Your profile should include\nat least 10 scans..." ),
               QMessageBox::Cancel, QMessageBox::Ok);

         if ( result == QMessageBox::Cancel ) return;
      }

      emit finish_exclude_profile( excludes );
   }
   else
      emit update_exclude_profile( excludes );
}

void US_ExcludeProfile::reset( void )
{
   int scanCount = original.size();
   excludes.clear();
   current.clear();

   ct_start->setValue( 1.0 );
   ct_stop ->setValue( scanCount );
   ct_nth  ->setValue( 1.0 );
   QList< int > excludes;
   emit update_exclude_profile( excludes );
}

void US_ExcludeProfile::terminate( void )
{
   emit cancel_exclude_profile();
   close();
}

void US_ExcludeProfile::done( void )
{
   finished = true;
   update();
   close();
}

