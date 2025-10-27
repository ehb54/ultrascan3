//! \file us_table.cpp
#include "us_choice.h"
#include "us_gui_settings.h"
#include "us_analyte.h"

US_Choice::US_Choice( const US_Solution& solution )
   : US_WidgetsDialog( nullptr, Qt::WindowFlags() )

{
   setPalette  ( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   int row = 0;

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QPalette p = US_GuiSettings::normalColor();

   main->addWidget( us_textlabel( "Description" ), row,   0 );
   main->addWidget( us_textlabel( "MW"          ), row,   1 );
   main->addWidget( us_textlabel( "vbar20"      ), row++, 2 );

   QLayoutItem* item = main->itemAtPosition( 0, 0 );
   item->widget()->setPalette( p );


   for ( int i = 0; i < solution.analyteInfo.size(); i++ )
   {
      bool          selected = ( i == 0 );
      QRadioButton* radio;

      const US_Analyte* analyte = &solution.analyteInfo[ i ].analyte;

      QGridLayout* radioLayout = us_radiobutton( analyte->description,
                                                 radio, selected );
      radios << radio;

      QLabel* mw   = us_textlabel( QString::number( analyte->mw,     'e', 4 ) );
      QLabel* vbar = us_textlabel( QString::number( analyte->vbar20, 'f', 4 ) );
      
      main->addLayout( radioLayout, row,   0 );
      main->addWidget( mw,          row,   1 );
      main->addWidget( vbar,        row++, 2 );
   }

   QPushButton* pb_ok = us_pushbutton( tr( "OK" ) );
   connect( pb_ok, SIGNAL( clicked() ), SLOT( done() ) );
   main->addWidget( pb_ok, row, 0, 1, 3 );
}

void US_Choice::done( void )
{
   for ( int i = 0; i < radios.size(); i++ )
   {
      if ( radios[ i ]->isChecked() )
      {
         emit choice( i );
         break;
      }
   }
   close();
}


