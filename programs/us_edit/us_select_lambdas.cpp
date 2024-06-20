//! \file us_select_lambdas.cpp

#include "us_select_lambdas.h"
#include "us_settings.h"
#include "us_gui_settings.h"


US_SelectLambdas::US_SelectLambdas( QVector< int > lambdas )
   : US_WidgetsDialog( 0, 0 )
{
   original    = lambdas;
   nbr_select  = 0;
   nbr_orig    = original.size();
   dbg_level   = US_Settings::us_debug();
   selected.clear();

   setWindowTitle( tr( "MWL Custom Lambda Selector" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main    = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   QVBoxLayout* left    = new QVBoxLayout;
   QVBoxLayout* right   = new QVBoxLayout;
   QHBoxLayout* lists   = new QHBoxLayout;

   // Read-only lambda list count text
   le_original   = us_lineedit( tr( "%1 original lambdas" )
                      .arg( nbr_orig ),   -1, true );
   le_selected   = us_lineedit( tr( "%1 selected lambdas" )
                      .arg( nbr_select ), -1, true );

   // Lambda list labels
   QLabel* lb_original = us_label( tr( "Original Lambdas" ) );
   QLabel* lb_selected = us_label( tr( "Selected Lambdas" ) );

   // Lambda list widgets
   lw_original   = us_listwidget();
   lw_selected   = us_listwidget();
   lw_original->setSelectionMode( QAbstractItemView::ExtendedSelection );
   lw_selected->setSelectionMode( QAbstractItemView::ExtendedSelection );

   for ( int ii = 0; ii < nbr_orig; ii++ )
   {  // Add items to the original list
      lw_original->addItem( QString::number( original[ ii ] ) );
   }

   // Add (=>) and Remove (<=) buttons for lists
   pb_add        = us_pushbutton( tr( "Add  ===>" ) );
   pb_remove     = us_pushbutton( tr( "<===  Remove" ) );

   connect( pb_add,    SIGNAL( clicked() ), SLOT( add_selections() ) );
   connect( pb_remove, SIGNAL( clicked() ), SLOT( rmv_selections() ) );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset  = us_pushbutton( tr( "Reset" ) );
   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
                pb_accept = us_pushbutton( tr( "Accept" ) );

   connect( pb_reset,  SIGNAL( clicked() ), SLOT( reset()  ) );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()   ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( done()   ) );

   pb_accept->setEnabled( false );
   buttons->addWidget( pb_reset  );
   buttons->addWidget( pb_help   );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   left   ->addWidget( lb_original );
   left   ->addWidget( lw_original );
   left   ->addWidget( pb_add      );
   right  ->addWidget( lb_selected );
   right  ->addWidget( lw_selected );
   right  ->addWidget( pb_remove   );
   lists  ->addLayout( left   );
   lists  ->addLayout( right  );
   main   ->addWidget( le_original );
   main   ->addWidget( le_selected );
   main   ->addLayout( lists   );
   main   ->addLayout( buttons );

   resize( 140, 700 );
}

void US_SelectLambdas::add_selections()
{
DbgLv(0) << "AddSelections";
   QList< QListWidgetItem* > selitems = lw_original->selectedItems();

   for ( int ii = 0; ii < selitems.count(); ii++ )
   {
      QListWidgetItem* l_item = selitems.at( ii );
      int lambda   = l_item->text().toInt();
      lw_original->setCurrentItem( l_item, QItemSelectionModel::Deselect );

      if ( ! selected.contains( lambda ) )
         selected << lambda;
   }

   qSort( selected );
   nbr_select   = selected.size();
   lw_selected->clear();

   for ( int ii = 0; ii < nbr_select; ii++ )
      lw_selected->addItem( QString::number( selected[ ii ] ) );

   le_selected->setText( nbr_select == 1 ?
                         tr( "1 selected lambda" ) :
                         tr( "%1 selected lambdas" ).arg( nbr_select ) );
   pb_accept->setEnabled( nbr_select > 0 );
}

void US_SelectLambdas::rmv_selections()
{
DbgLv(0) << "RemoveSelections";
   QList< QListWidgetItem* > selitems = lw_selected->selectedItems();

   for ( int ii = 0; ii < selitems.count(); ii++ )
   {
      int lambda   = selitems.at( ii )->text().toInt();
      int selndx   = selected.indexOf( lambda );

      if ( selndx >= 0 )
         selected.remove( selndx );
   }

   nbr_select   = selected.size();
   lw_selected->clear();

   for ( int ii = 0; ii < nbr_select; ii++ )
      lw_selected->addItem( QString::number( selected[ ii ] ) );

   le_selected->setText( nbr_select == 1 ?
                         tr( "1 selected lambda" ) :
                         tr( "%1 selected lambdas" ).arg( nbr_select ) );
   pb_accept->setEnabled( nbr_select > 0 );
}

void US_SelectLambdas::reset( void )
{
   lw_selected->clear();
   selected    .clear();
   nbr_select    = 0;
   le_selected->setText( tr( "0 selected lambdas" ) );
   pb_accept->setEnabled( false );
}

void US_SelectLambdas::cancel( void )
{
   reject();
   close();
}

void US_SelectLambdas::done( void )
{
   std::sort( selected.begin(), selected.end() );
   emit new_lambda_list( selected );

   accept();
   close();
}

