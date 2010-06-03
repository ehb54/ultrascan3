//! \file us_assiciations_gui.cpp

#include "us_associations_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"

US_AssociationsGui::US_AssociationsGui( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Assiciations" );
   setPalette       ( US_GuiSettings::frameColor() );
   setWindowModality( Qt::WindowModal );
   
   // Very light gray
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   int row = 0;
   
   // Start widgets
   // Models List Box
   lw_analytes = new US_ListWidget;

   //connect( lw_models, SIGNAL( currentRowChanged( int  ) ),
   //                    SLOT  ( change_model     ( int  ) ) );

   //connect( lw_models, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
   //                    SLOT  ( select_model     ( QListWidgetItem* ) ) );

   main->addWidget( lw_analytes, row, 0, 5, 2 );
   row += 5;


   // Pushbuttons
   QBoxLayout* buttonbox = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help") );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
   buttonbox->addWidget( pb_accept );
   //connect( pb_accept, SIGNAL( clicked() ), SLOT( accept_associations()) );

   main->addLayout( buttonbox, row++, 0, 1, 2 );
}
