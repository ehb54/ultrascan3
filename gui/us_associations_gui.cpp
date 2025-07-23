//! \file us_associations_gui.cpp

#include "us_associations_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"

US_PushButton::US_PushButton( const QString& text, int i )
    : QPushButton( text ), index( i )
{
   setFont( QFont( US_GuiSettings::fontFamily(), 
                   US_GuiSettings::fontSize() ) );

   setPalette( US_GuiSettings::pushbColor() );
   setAutoDefault( false );
}

void US_PushButton::mousePressEvent( QMouseEvent* e )
{
  emit pushed( index );
  e->accept();
}

US_AssociationsGui::US_AssociationsGui( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Associations" );
   setPalette       ( US_GuiSettings::frameColor() );
   setAttribute     ( Qt::WA_DeleteOnClose );
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
   tw->setColumnCount( 7 );
   tw->setRowCount   ( 0 );
  
   QStringList headers;
   headers << "" << "Analyte 1" << "Analyte 2" << "<==>" << "Product" 
           << "K_dissociation\n(molar units)" << "K_off Rate\n(1/sec)";
   int flwidth = fm->width( "8888.3456e+00" );
   tw->setMinimumWidth( flwidth * 5 + 100 );
   tw->setRowHeight(   0, fm->height() + 4 );
   tw->setColumnWidth( 0, fm->width( "D" ) + 6 );
   tw->setColumnWidth( 1, flwidth );
   tw->setColumnWidth( 2, flwidth );
   tw->setColumnWidth( 3, fm->width( "<==>" ) - 2 );
   tw->setColumnWidth( 4, flwidth );
   tw->setColumnWidth( 5, flwidth );
   tw->setColumnWidth( 6, flwidth );

   new_row();

   tw->setHorizontalHeaderLabels( headers );
   tw->setDragDropMode( QAbstractItemView::DropOnly );
   tw->horizontalHeader()->setStretchLastSection( true );

   connect( tw, SIGNAL( cellChanged( int, int ) ), 
                SLOT  ( changed    ( int, int ) ) );

   main->addWidget( tw, row, 0, 5, 2 );
   row += 5;

   // Brief help label
   QTextEdit* te_help = us_textedit();
   QPalette pa( US_GuiSettings::labelColor() );
   te_help->setPalette( pa );
   te_help->setTextBackgroundColor( pa.color( QPalette::Window ) );
   te_help->setTextColor(           pa.color( QPalette::WindowText ) );
   QFontMetrics fm( te_help->font() );
   te_help->setMaximumHeight( fm.lineSpacing() * 13 / 2 );
   te_help->setText( tr(
      "* Drag a component from the upper list and drop it in an"
      " Analyte or Product cell.\n"
      "* Set the stoichiometry counter to the left of each component.\n"
      "* Enter values for K_dissociation and K_off Rate in their text cells.\n"
      "* Click on the \"D\" on the left side of any row to delete that row.\n"
      "* Click the \"Accept\" button when all equations are as desired." ) );
   main->addWidget( te_help, row, 0, 5, 2 );
   row += 5;

   // Pushbuttons
   QBoxLayout* buttonbox = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help") );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   //QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
   pb_accept = us_pushbutton( tr( "Accept") );
   buttonbox->addWidget( pb_accept );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( complete()) );

   main->addLayout( buttonbox, row++, 0, 1, 2 );

   populate();
}

void US_AssociationsGui::disable_gui( void )
{
  pb_accept-> setEnabled(false);

}

void US_AssociationsGui::populate( void )
{
   for ( int i = 0; i < model.associations.size(); i++ )
   {
      int                    index = 0;
      US_Model::Association* as    = &model.associations[ i ];
      
      // First set K_d and k_off
      QString s = QString::number( as->k_d,   'e', 4 );
      tw->setItem( i, 5, new QTableWidgetItem( s ) );

              s = QString::number( as->k_off, 'e', 4 );
      tw->setItem( i, 6, new QTableWidgetItem( s ) );


      // reaction_components must be size 2 or 3 
      set_component( index++, i, 1 );
      
      if ( as->rcomps.size() > 2 )
         set_component( index++, i, 2 );

      set_component( index++, i, 4 );
   }
}

void US_AssociationsGui::set_component( int index, int row, int col )
{
   US_Model::Association* as = &model.associations[ row ];
         
   int     component = as->rcomps[ index ];
   QString s         = lw_analytes->item( component )->text();
   int     koligo    = model.components[ component ].oligomer;
           s         = s.left( 1 ) + QString::number( koligo );
   tw->setItem( row, col, new QTableWidgetItem( s ) );

   qApp->processEvents();  // Let the signals work

   // Set the counter
   QWidget* w     = tw->cellWidget( row, col );
   QLayout* L     = w->layout();

   w              = L->itemAt( 0 )->widget();
   QwtCounter*  c = dynamic_cast< QwtCounter* >( w );

   c->setValue( fabs( (double)as->stoichs[ index ] ) ); 
}

void US_AssociationsGui::changed( int row, int col )
{
   tw->disconnect();

   QTableWidgetItem* item = tw->item( row, col );

   if ( col > 4 )
   {
      double value = item->text().toDouble();
      item->setText( QString::number( value, 'e', 4 ) );
   }

   else if ( col == 3 )
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
      c->setSingleStep( 1.0 );
      L->addWidget( c );

      QString text = item->text().left( 1 );
      QString sscr = item->text().mid( 1 );

      if ( sscr.startsWith( " " ) )
      {
         int index = lw_analytes->currentRow();
         int colig = model.components[ index ].oligomer;
             sscr  = QString::number( colig );
      }

      delete item;
      QLabel* label = us_label( text + "<sub>" + sscr + "</sub>" );
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

   QPushButton* pb = new US_PushButton( "D", count );
   pb->setMaximumWidth( fm->width( "D" ) + 6 );
   connect( pb, SIGNAL( pushed( int ) ), SLOT( del( int ) ) );

   tw->setCellWidget( count, 0, pb );
}

void US_AssociationsGui::del( int index )
{
   // Don't delete last row
   if ( index == tw->rowCount() - 1 ) return;
   tw->removeRow( index );

   for ( int i = 0; i <  tw->rowCount(); i++ )
   {
      QWidget*       w  = tw->cellWidget( i, 0 );
      US_PushButton* pb = dynamic_cast< US_PushButton* >( w );
      pb->setIndex( i );
   }
}

void US_AssociationsGui::complete( void )
{
   QVector< US_Model::Association > associations;

   // Check validity
   for ( int i = 0; i < tw->rowCount() - 1; i++ )
   {
      int moles_left  = 0;
      int moles_right = 0;

      QLayout*    L;
      QwtCounter* c;
      int         index;
      int         count;
      int         koligo;
      
      US_Model::Association association;
      QTableWidgetItem*     item;

      // If Kd and koff are not set, the default is zero
      item = tw->item( i, 5 );
      if ( item != 0 ) association.k_d   = item->text().toDouble();

      item = tw->item( i, 6 );
      if ( item != 0 ) association.k_off = item->text().toDouble();

      QWidget* w = tw->cellWidget( i, 1 );
      
      if ( w != 0 )
      {
         L = w->layout();

         if ( L != 0 )
         {
            w     = L->itemAt( 0 )->widget();
            c     = dynamic_cast< QwtCounter* >( w );
            count = (int) c->value(); 
            association.stoichs << count;

            w     = L->itemAt( 1 )->widget();
            index = dynamic_cast< QLabel* >( w )->text().at( 0 ).cell() - 'A';
            association.rcomps  << index;

            koligo = model.components[ index ].oligomer;

            moles_left += count * koligo;
         }
      }

      w = tw->cellWidget( i, 2 );
      
      if ( w != 0 )
      {
         L = w->layout();

         if ( L != 0 )
         {
            w     = L->itemAt( 0 )->widget();
            c     = dynamic_cast< QwtCounter* >( w );
            count = (int) c->value(); 
            association.stoichs << count;

            w     = L->itemAt( 1 )->widget();
            index = dynamic_cast< QLabel* >( w )->text().at( 0 ).cell() - 'A';
            association.rcomps  << index;

            koligo = model.components[ index ].oligomer;

            moles_left += count * koligo;
         }
      }

      w = tw->cellWidget( i, 4 );
      
      if ( w != 0 )
      {
         L = w->layout();

         if ( L != 0 )
         {
            w     = L->itemAt( 0 )->widget();
            c     = dynamic_cast< QwtCounter* >( w );
            count = (int) c->value(); 
            association.stoichs << -count;

            w     = L->itemAt( 1 )->widget();
            index = dynamic_cast< QLabel* >( w )->text().at( 0 ).cell() - 'A';
            association.rcomps  << index;

            koligo = model.components[ index ].oligomer;

            moles_right += count * koligo;
         }
         if ( koligo == 1 )
            moles_right  = moles_left;   // Skip test for product ogilomer=1
      }

      if ( moles_right != moles_left )
      {
         QMessageBox::information( this,
               tr( "Equations do not balance" ),
               tr( "Equation %1 does not balance" ).arg( i + 1 ) );
         return;
      }

      if ( combine_reactants( &association ) )
      {
         QMessageBox::information( this,
               tr( "Combined Identical Reactants" ),
               tr( "In association %1, two identical reactants"
                   " have been combined into a single one, with"
                   " double the stoichiometry value." ).arg( i + 1 ) );
      }

      associations << association;
   }

   // Update model associations
   model.associations = associations;
 
   emit done();
   close();
}

// Combine any identical reactants
bool US_AssociationsGui::combine_reactants( US_Model::Association* as )
{
   bool combined  = false;

   if ( as->rcomps.size() == 3  &&  as->stoichs[ 1 ] > 0 )
   {  // 2 reactants and a product:  check if reactants are identical
      int comp1      = as->rcomps[ 0 ];
      int comp2      = as->rcomps[ 1 ];
      int stoi1      = as->stoichs[ 0 ];
      int stoi2      = as->stoichs[ 1 ];

      combined       = ( comp1 == comp2  &&  stoi1 == stoi2 );

      if ( combined )
      {  // Identical reactants:  combine and double stoichiometry
         comp2            = as->rcomps [ 2 ];  // Product component
         stoi2            = as->stoichs[ 2 ];  // Product stoichiometry
         as->rcomps .resize( 2 );              // Single reactant and product
         as->stoichs.resize( 2 );
         as->rcomps [ 0 ] = comp1;             // Reactant component
         as->stoichs[ 0 ] = stoi1 * 2;         // Doubled reactant stoich.
         as->rcomps [ 1 ] = comp2;             // Product component
         as->stoichs[ 1 ] = stoi2;             // Product stoichiometry
      }
   }

   return combined;
}

