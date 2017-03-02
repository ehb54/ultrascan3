//! \file us_select_item.cpp

#include "us_select_item.h"
#include "us_settings.h"
#include "us_gui_settings.h"

//! \brief Class for simple table widget display of
//!        a list of item columns.

// Main constructor to enable selecting a single item
US_SelectItem::US_SelectItem( QList< QStringList >& items,
   QStringList& hdrs, const QString titl, int* aselxP,
   const int def_sort )
   : US_WidgetsDialog( 0, 0 ), items( items ), hdrs( hdrs )
{
   multi_sel         = false;
   selxP             = aselxP;
   selxsP            = NULL;
   sort_ord          = ( def_sort < 0 ) ? Qt::DescendingOrder
                                        : Qt::AscendingOrder;
   sort_col          = qAbs( def_sort ) - 1;

   build_layout( titl );

   tw_data->setSelectionMode( QAbstractItemView::SingleSelection );

   show();
}

// Alternate contructor to enable selecting multiple items
US_SelectItem::US_SelectItem( QList< QStringList >& items,
   QStringList& hdrs, const QString titl, QList< int >* aselxsP,
   const int def_sort )
   : US_WidgetsDialog( 0, 0 ), items( items ), hdrs( hdrs )
{
   multi_sel         = true;
   selxP             = NULL;
   selxsP            = aselxsP;
   sort_ord          = ( def_sort < 0 ) ? Qt::DescendingOrder
                                        : Qt::AscendingOrder;
   sort_col          = qAbs( def_sort ) - 1;

   build_layout( titl );

   tw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );
}

// Common function for building the dialog layout
void US_SelectItem::build_layout( const QString titl )
{
   setWindowTitle( titl );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 500, 300 );

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout;

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );
   le_dfilter          = us_lineedit();
   dsearch             = QString( "" );

   int row             = 0;
   top->addWidget( lb_filtdata, row,   0, 1, 1 );
   top->addWidget( le_dfilter,  row++, 1, 1, 3 );

   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   main->addLayout( top );

   // Create a list of column 0 values
   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   nitems              = items.count();
   ncols               = hdrs.count();
   itemlist.clear();

   for ( int ii = 0; ii < nitems; ii++ )
   {  // Save column0 description in a separate list
      itemlist << items[ ii ][ 0 ];
   }

   // Construct the table widget 

   tw_data = new QTableWidget( 20, ncols, this );
   tw_data->setFrameStyle ( QFrame::NoFrame );
   tw_data->setPalette    ( US_GuiSettings::editColor() );
   tw_data->setFont       ( tw_font );
   tw_data->setEditTriggers          ( QAbstractItemView::NoEditTriggers );
   tw_data->setSelectionBehavior     ( QAbstractItemView::SelectRows );
   tw_data->setHorizontalHeaderLabels( hdrs );
   tw_data->verticalHeader()->hide();
   tw_data->setShowGrid   ( false );

   main->addWidget( tw_data );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept = us_pushbutton( multi_sel ?
                                           tr( "Select Item(s)" ) :
                                           tr( "Select Item"    ) );

   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );

   main->addLayout( buttons );
   resize( 700, 250 );

   // List items
   list_data();

   show();
qDebug() << "SelI: main: post-ld tw size" << tw_data->size();
   resize( ( tw_data->width() + 4 ), ( tw_data->height() + 30 ) );
qDebug() << "SelI: main: post-ld ma size" << size();
   qApp->processEvents();
}

// Slot for search string change: save string and re-list data
void US_SelectItem::search( const QString& search_string )
{
   dsearch          = search_string;

   list_data();
}

// List data choices (possibly filtered)
void US_SelectItem::list_data()
{
   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics( tw_font );
   int rowhgt       = fm->height() + 2;     // Row height based on font
   bool have_search = ! dsearch.isEmpty();  // Flag if searching by filter
   int mxrows       = itemlist.size();      // Count of total items
   int nrows        = 0;                    // Initial displayed items count

   tw_data->clearContents();

   if ( mxrows == 0 )
   {  // Return now if there are no list items
      QString clabel = tr( "No data found." );
      tw_data->setItem ( 0, 0, new QTableWidgetItem( clabel ) );
      return;
   }

   tw_data->setSortingEnabled( false );     // Temporarily turn off sorting

   // Determine how many rows will be displayed
   if ( have_search )
   {  // We have search text, so we must count the filtered items
      nrows            = 0;
      for ( int ii = 0; ii < mxrows; ii++ )
      {  // Bump therow count for each item name matching the filter
         QString iname    = itemlist.at( ii );
         if ( iname.contains( dsearch, Qt::CaseInsensitive ) )
            nrows++;
      }
   }
   else
   {  // There is no filter, so the row count is the full list count
      nrows            = mxrows;
   }

   tw_data->setRowCount( nrows );

   // Populate the rows with (filtered) items
   int kk           = 0;
   for ( int ii = 0; ii < mxrows; ii++ )
   {  // Propagate list widget with labels
      QString iname    = itemlist.at( ii );
      // Skip where name does not match the filter
      if ( have_search  &&
           ! iname.contains( dsearch, Qt::CaseInsensitive ) )
         continue;

      // Set the column 0 name field
      tw_data->setItem( kk, 0, new QTableWidgetItem( iname ) );

      for ( int jj = 1; jj < ncols; jj++ )
      {  // Set fields for remaining columns of the present row
         tw_data->setItem( kk, jj,
                           new QTableWidgetItem( items[ ii ][ jj ] ) );
      }

      tw_data->setRowHeight( kk++, rowhgt );
   }

   // Complete specification of the table widget
   tw_data->setSortingEnabled( true );                // Turn on sorting
   tw_data->sortByColumn     ( sort_col, sort_ord );  // Default sort column,order
   tw_data->resizeColumnsToContents();                // Size to column contents

   qApp->processEvents();
qDebug() << "SelI: listd: rco tw size" << tw_data->size();
qDebug() << "SelI: listd: adj tw size" << tw_data->size();
qDebug() << "SelI: listd: adj ma size" << size();
}

// Cancel button:  no item index(es) returned
void US_SelectItem::cancelled()
{
   reject();
   close();
}

// Accept button:  return index(es) to selected items
void US_SelectItem::accepted()
{
   QList< QTableWidgetItem* > selitems = tw_data->selectedItems();

   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect or Cancel" ) );
      return;
   }

   if ( multi_sel  &&  selxsP != NULL )
   {  // Possibly multiple selections:  return their indexes
      selxsP->clear();

      for ( int ii = 0; ii < selitems.size(); ii++ )
      {  // Return indexes to selected items
         QTableWidgetItem* twi  = selitems.at( ii );
         int irow          = twi->row();
         twi               = tw_data->item( irow, 0 );

         (*selxsP) << qMax( 0, itemlist.indexOf( twi->text() ) );
      }
   }

   else if ( ! multi_sel  &&  selxP != NULL )
   {  // Return the index to the selected item
      QTableWidgetItem* twi  = selitems.at( 0 );
      int irow          = twi->row();
      twi               = tw_data->item( irow, 0 );

      (*selxP)          = qMax( 0, itemlist.indexOf( twi->text() ) );
   }

   accept();        // Signal that selection was accepted
   close();
}

