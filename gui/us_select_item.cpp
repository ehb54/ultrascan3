//! \file us_select_item.cpp

#include "us_select_item.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_help.h"
#include "us_crypto.h"
#include "us_db2.h"

//! \brief Class for simple table widget display of
//!        a list of item columns.

// Main constructor to enable selecting a single item
US_SelectItem::US_SelectItem( QList< QStringList >& items,
   QStringList& hdrs, const QString titl, int* aselxP,
   const int def_sort )
   : US_WidgetsDialog( 0, 0 ), items( items ), hdrs( hdrs )
{
   multi_sel         = false;
   deleted_button    = false;
   autoflow_button   = false;
   autoflow_da       = false;
   selxP             = aselxP;
   selxsP            = NULL;
   sort_ord          = ( def_sort < 0 ) ? Qt::DescendingOrder
                                        : Qt::AscendingOrder;
   sort_col          = qAbs( def_sort ) - 1;

   build_layout( titl );

   tw_data->setSelectionMode( QAbstractItemView::SingleSelection );

   show();
}

// Alternate constructor to enable selecting a single item with Delete button
US_SelectItem::US_SelectItem( QList< QStringList >& items,
			      QStringList& hdrs, const QString titl, int* aselxP, QString add_label,
			      const int def_sort )
  : US_WidgetsDialog( 0, 0 ), items( items ), hdrs( hdrs )
{
   multi_sel         = false;
   deleted_button    = false;
   deleted_button_autoflow    = false;
   autoflow_button   = false;
   autoflow_da       = false;
   
   
   if ( !add_label.isEmpty() )
     {
       if ( add_label == "DELETE" )
	 deleted_button    = true;
       if ( add_label == "AUTOFLOW_GMP" )
	 {
	   autoflow_button = true;
	   deleted_button_autoflow  = true;
	 }
       if ( add_label == "AUTOFLOW_DA" )
	 {
	   autoflow_button = true;
	   deleted_button_autoflow  = true;
	   autoflow_da     = true;
	 }
     }
   
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
   deleted_button    = false;
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

   //Inormation field for autoflow
   QTextEdit*     le_info;

   if ( autoflow_button )
     {
       le_info = us_textedit();
       QFontMetrics m (le_info -> font()) ;
       int RowHeight = m.lineSpacing() ;
       le_info -> setFixedHeight  (5 * RowHeight) ;

       QPalette p = le_info->palette(); 
       p.setColor(QPalette::Base, Qt::lightGray);
       p.setColor(QPalette::Text, Qt::darkRed);
       le_info->setPalette(p);
       
       le_info->setText(tr( "<ul><li>Information on one or more experimental methods submitted to Bechman Optima(s) is available. "
			    "You can reattach to the job by selecting the run from the list below. "
			    "Alternatively, you can define and submit a new experiment method to the availabale Optima instrument(s). </ul></li>" ));
       
       main->addWidget( le_info );
     }
   
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
   ncols               = hdrs.count();

   // nitems              = items.count();
   // itemlist.clear();

   // for ( int ii = 0; ii < nitems; ii++ )
   // {  // Save column0 description in a separate list
   //    itemlist << items[ ii ][ 0 ];
   // }

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

   QString cancel_pb_label( tr("Cancel") );
   if ( autoflow_button )
     cancel_pb_label = tr("Define Another Experiment");
          
   //QPushButton* pb_cancel = us_pushbutton( cancel_pb_label  );

   pb_cancel = us_pushbutton( cancel_pb_label  );
   
   QString accept_pb_label( tr( "Select Item" ) );
   if ( autoflow_button )
     accept_pb_label = tr("Select Optima Run to Follow");
   
   QPushButton* pb_accept = us_pushbutton( multi_sel ?
                                           tr( "Select Item(s)" ) :
                                           accept_pb_label );

   
   QPushButton* pb_delete          = us_pushbutton( tr( "Delete Item" ) );
   QPushButton* pb_delete_autoflow = us_pushbutton( tr( "Delete Record" ) );
   
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_delete );
   buttons->addWidget( pb_delete_autoflow );
   buttons->addWidget( pb_accept );

   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( deleted() ) );
   connect( pb_delete_autoflow, SIGNAL( clicked() ), SLOT( deleted_autoflow() ) );

   if ( !deleted_button )
     pb_delete->hide();

   if ( !deleted_button_autoflow )
     pb_delete_autoflow->hide();

   
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

   //ALEXEY: moved here from build_layout()
   nitems              = items.count();
   itemlist.clear();

   for ( int ii = 0; ii < nitems; ii++ )
   {  // Save column0 description in a separate list
      itemlist << items[ ii ][ 0 ];
   }
   //////////////////

   
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

      
      //ALEXEY: if GMP run ("YES") & open with DA software (autoflow_da == true), make item unselectable:
      if ( autoflow_da && items[ ii ][ ncols -1 ] == "YES" ) 
	{
	  tw_data->item( kk, 0)->setFlags(Qt::NoItemFlags);
	  // //tw_data->item( kk, 0)->setForeground(QBrush(QColor(250,0,0)));
	  tw_data->item( kk, 0)->setForeground(QBrush(Qt::gray));
	}
      
      for ( int jj = 1; jj < ncols; jj++ )
      {  // Set fields for remaining columns of the present row
         tw_data->setItem( kk, jj,
                           new QTableWidgetItem( items[ ii ][ jj ] ) );


	 //ALEXEY: if GMP run ("YES") & open with DA software (autoflow_da == true), make item unselectable:
	 if ( autoflow_da && items[ ii ][ ncols -1 ] == "YES" ) 
	   {
	     tw_data->item( kk, jj)->setFlags(Qt::NoItemFlags);
	     // //tw_data->item( kk, 0)->setForeground(QBrush(QColor(250,0,0)));
	     tw_data->item( kk, jj)->setForeground(QBrush(Qt::gray));
	   }
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


// Delete button:
      /*  ALEXEY: For future Delete() function:

	  to get autoflow ID -> items[selxP][0]; 
	  delete (stored procedure) 
	         + emit signal to us_experiment to update protdata BEFORE any selection; 
	  then remove items[selxP] from QList; 
	  then rebuild list by running list_data(); [move part of ]
      */
void US_SelectItem::deleted_autoflow()
{
   QList< QTableWidgetItem* > selitems = tw_data->selectedItems();
   
   int     AutoflowRow;
   QString AutoflowID;
   if ( selitems.size() == 0 )
     {
       QMessageBox::information( this,
				 tr( "No Autoflow Record Selected" ),
				 tr( "You have not selected any auflow record.\nSelect or Cancel" ) );
       return;
     }

   // Return the index to the selected item
   QTableWidgetItem* twi  = selitems.at( 0 );
   int irow          = twi->row();
   twi               = tw_data->item( irow, 0 );
   
   AutoflowRow            = qMax( 0, itemlist.indexOf( twi->text() ) );
   
   AutoflowID = items[ AutoflowRow ][ 0 ];

   //Attempt autoflow record deletion:
   qDebug() << "Autoflow ID to delete: ID, name, run status: " << AutoflowID << ", " << items[ AutoflowRow ][ 1 ] << ", " << items[ AutoflowRow ][ 4 ];

   QMessageBox msgBox;
   msgBox.setText(tr( "You have selected the following Record to delete:<br><br>" )
		  + tr("<b>ID:&emsp;</b>") + items[ AutoflowRow ][ 0 ]
		  + tr("<br>")
		  + tr("<b>Name:&emsp;</b>") + items[ AutoflowRow ][ 1 ]
		  + tr("<br>")
		  + tr("<b>Status:&emsp;</b> ") + items[ AutoflowRow ][ 4 ]
		  + tr( "<br><br>Proceed?" ));
   msgBox.setInformativeText("<font color='red'><b>NOTE:</b> if deleted, this run cannot be monitored with this program anymore!</font>");
   msgBox.setWindowTitle(tr("Delete Autoflow Record"));
   QPushButton *Confirm   = msgBox.addButton(tr("Delete"), QMessageBox::YesRole);
   QPushButton *Cancel    = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);

   msgBox.setIcon(QMessageBox::Question);
   msgBox.exec();
    
   if (msgBox.clickedButton() == Cancel) 
     return;
   else if (msgBox.clickedButton() == Confirm)
     {
       US_Passwd pw;
       US_DB2* db = new US_DB2( pw.getPasswd() );
       QStringList q( "" );
       q.clear();
       q  << QString( "delete_autoflow_record_by_id" )
	  << AutoflowID;

       int status = db->statusQuery( q );
       
       if ( status == US_DB2::NO_AUTOFLOW_RECORD )
	 {
	   QMessageBox::warning( this,
				 tr( "Autoflow Record Not Deleted" ),
				 tr( "This record could not be deleted since\n"
				     "it is not present in the LIMS DB." ) );
	   return;
	 }

       items.removeAt( AutoflowRow );     // Remove deleted item row
       list_data();                       // Rebuild protocol list in the dialog
       
       QString msg("Autoflow record  has been successfully deleted.");
       QMessageBox::information( this,
			     tr( "Autoflow Record  Deleted" ),
			     msg );


       emit accept_autoflow_deletion();        // Signal to pass to us_comproject to update (re-read reduced) autoflow records
       
     }
}


void US_SelectItem::deleted()
{
   QList< QTableWidgetItem* > selitems = tw_data->selectedItems();

   int     ProtRow;
   QString ProtID;
   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect or Cancel" ) );
      return;
   }

   // Return the index to the selected item
   QTableWidgetItem* twi  = selitems.at( 0 );
   int irow          = twi->row();
   twi               = tw_data->item( irow, 0 );
   
   ProtRow            = qMax( 0, itemlist.indexOf( twi->text() ) );
   
   ProtID = items[ ProtRow ][ 2 ];

   //Attempt protocol deletion:
   qDebug() << "Prtotcol ID to delete: ID, name: " << ProtID << ", " << items[ ProtRow ][ 0 ];

   int response = QMessageBox::question( this,
					 tr( "Delete Protocol?" ),
					 tr( "You have selected the following Protocol to delete:\n    \"" )
					 + items[ ProtRow ][ 0 ] + tr( "\"\n\nProceed?" ),
					 QMessageBox::Yes, QMessageBox::Cancel );
   
   if ( response != QMessageBox::Yes )
     return;

   US_Passwd pw;
   US_DB2* db = new US_DB2( pw.getPasswd() );
   QStringList q( "" );
   q.clear();
   q  << QString( "delete_protocol" )
      << ProtID;      
   
   int status = db->statusQuery( q );

   if ( status == US_DB2::PROTOCOL_IN_USE )
     {
       QMessageBox::warning( this,
			     tr( "Protocol Not Deleted" ),
			     tr( "This protocol could not be deleted since\n"
				 "it is in use in one or more experiments." ) );
       return;
     }

   qDebug() << "Items.size() BEROFE deletion: " << items.size();
   items.removeAt( ProtRow );     // Remove deleted item row
   qDebug() << "Items.size() AFTER deletion: " << items.size();
   
   list_data();                   // Rebuild protocol list in the dialog

   QString msg("Protocol has been successfully deleted.");
   QMessageBox::information( this,
			     tr( "Protocol Deleted" ),
			     msg );
   
   emit accept_deletion();        // Signal to pass to us_experiment to update (re-read reduced) protdata
}
