//! \file us_assiciations_gui.cpp

#include "us_associations_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"

US_AssociationsGui::US_AssociationsGui( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Associations" );
   setPalette       ( US_GuiSettings::frameColor() );
   setWindowModality( Qt::WindowModal );
   
   // Very light gray
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   fm = new QFontMetrics( font );

   int row = 0;
   
   // Start widgets
   // Models List Box
   lw_analytes = new US_ListWidget;
   lw_analytes->setDragDropMode( QAbstractItemView::DragOnly );

   char leading = 'A';

   for ( int i = 0; i < model.components.size(); i++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ i ];
      lw_analytes->addItem( QString( QChar( leading ) ) + " " + sc->name );
      leading++;

   }

   main->addWidget( lw_analytes, row, 0, 5, 2 );
   row += 5;

   tw = new QTableWidget();
   tw->setPalette( US_GuiSettings::editColor() );
   tw->setColumnCount( 6 );
   tw->setRowCount   ( 0 );
  
   QStringList headers;
   headers << "Analyte 1" << "Analyte 2" << "<==>" << "Product" 
           << "K_dissociation\n(molar units)" << "k_off Rate\n(moles/sec)";
   tw->setMinimumWidth( 550 );
   tw->setRowHeight( 0, fm->height() + 4 );
   tw->setColumnWidth( 2, fm->width( "<==>" ) + 6 );

   new_row();

   tw->setHorizontalHeaderLabels( headers );
   tw->setDragDropMode( QAbstractItemView::DropOnly );
   tw->horizontalHeader()->setStretchLastSection( true );

   connect( tw, SIGNAL( cellChanged( int, int ) ), 
                SLOT  ( changed    ( int, int ) ) );

   connect( tw, SIGNAL( cellEntered( int, int ) ), 
                SLOT  ( clicked    ( int, int ) ) );

   main->addWidget( tw, row, 0, 5, 2 );
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

void US_AssociationsGui::changed( int row, int col )
{
   tw->disconnect();

   QTableWidgetItem* item = tw->item( row, col );

   if ( col > 3 )
   {
      double value = item->text().toDouble();
      item->setText( QString::number( value, 'e', 4 ) );
   }
   else if ( col == 2 )
   {
      item->setText( QString() );
   }
   else
   {
      QWidget*     w = new QWidget;
      QHBoxLayout* L = new QHBoxLayout( w );
      L->setContentsMargins( 0, 0, 0, 0 );
      L->setSpacing        ( 0 );

      QwtCounter*  c = us_counter( 1, 1.0, 20.0 );
      c->setStep( 1.0 );
      L->addWidget( c );

      QString text = item->text().left( 1 );
      delete item;
      QLabel* label = us_label( text + "<sub>1</sub>" );
      label->setPalette( US_GuiSettings::editColor() );
      L->addWidget( label );
      tw->setCellWidget( row, col, w );
   }
   

   connect( tw, SIGNAL( cellChanged( int, int ) ), 
                SLOT  ( changed    ( int, int ) ) );
   
   if ( row == tw->rowCount() - 1 ) new_row();
}

void US_AssociationsGui::new_row( void )
{
   int count = tw->rowCount();
   
   tw->setRowCount ( count + 1 );
   tw->setRowHeight( count, fm->height() + 4 );

   //QWidget*     w1 = new QWidget;
   //QHBoxLayout* L1 = new QHBoxLayout( w1 );

   //QwtCounter* c1 = us_counter( 1, 0.0, 20.0 );
   //c1->setStep( 1.0 );
   //L1->addWidget( c1 );



   //QwtCounter* c2 = us_counter( 2, 0.0, 1.0 );
   //koff << c1;
   //connect ( c2, SIGNAL( valueChanged( double ) ), SLOT( counter( double ) ) );
   //tw->setCellWidget( count, 0, w1 );

}

void US_AssociationsGui::clicked( int row, int col )
{
   qDebug() << "clicked" << row << col;
}

void US_AssociationsGui::counter( double v )
{
   qDebug() << "counter value" << v;
}
