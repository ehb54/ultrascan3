//! \file us_get_dbexp.cpp

#include "us_get_dbexp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db2.h"
#include "us_passwd.h"

// Primary constructor to establish the dialog
US_GetDBExp::US_GetDBExp( QString& eID ) 
: US_WidgetsDialog( nullptr, Qt::WindowFlags() ), expID( eID )
{
   setWindowTitle( tr( "Available US3 Runs..." ) );
   setPalette( US_GuiSettings::frameColor() );
//   setAttribute( Qt::WA_DeleteOnClose, true );

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics( font );

   // Load the runInfo structure with current data
   if ( ! loadData() )
   {
      // Why doesn't this work?
      // hide();
      // close(); // done( -1 );
      // return;
   }

   tw = new QTableWidget( runInfo.size(), 4, this ); // rows, columns, parent
   tw->setPalette( US_GuiSettings::editColor() );

   QStringList headers;
   headers << "ID"
           << "Date"
           << "RunID"
           << "Label";

   tw->setHorizontalHeaderLabels( headers );
   tw->verticalHeader()->hide();
   tw->setShowGrid( false );
   tw->setSelectionBehavior( QAbstractItemView::SelectRows );
   tw->setMinimumWidth( 640 );
   tw->setMinimumHeight( 480 );
   tw->setRowHeight( 0, fm->height() + 4 );
   tw->setColumnWidth( 0,  50 );
   tw->setColumnWidth( 1, 150 );
   tw->setColumnWidth( 2, 250 );
   tw->setColumnWidth( 3, 350 );

   // Now load the table, marking each as not-editable
   for ( int i = 0; i < runInfo.size(); i++ )
   {
      RunInfo r = runInfo[ i ];

      QTableWidgetItem* item = new QTableWidgetItem( QString::number( r.ID ) );
      item ->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw   ->setItem( i, 0, item );

      item = new QTableWidgetItem( r.date );
      item ->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw   ->setItem( i, 1, item );

      item = new QTableWidgetItem( r.runID );
      item ->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw   ->setItem( i, 2, item );

      item = new QTableWidgetItem( r.label );
      item ->setFlags(item->flags() ^ Qt::ItemIsEditable);
      tw   ->setItem( i, 3, item );
   }

   // Enable sorting by a particular column
   QHeaderView* qHeader = tw ->horizontalHeader();
   connect( qHeader, SIGNAL( sectionClicked( int ) ),
                     SLOT  ( columnClicked ( int ) ) );

   main->addWidget( tw );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Select" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( select() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );
}

// Function to load the runInfo structure with all runID's in the DB
bool US_GetDBExp::loadData( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Error making the DB connection.\n" ) );
      return false;
   }

   // Get all the experiment ID's
   QStringList expIDs;
   expIDs.clear();
   QStringList q( "get_experiment_desc" );
   q << QString::number( US_Settings::us_inv_ID() );
   db.query( q );
   while( db.next() )
      expIDs << db.value( 0 ).toString();

   // Now get information we want about each experiment
   runInfo.clear();
   foreach ( QString expID, expIDs )
   {
      q.clear();
      q  << QString( "get_experiment_info" )
         << expID;
      db.query( q );
      db.next();

      RunInfo r;
      r.ID     = expID.toInt();
      r.date   = db.value( 13 ).toString();
      r.runID  = db.value(  2 ).toString();
      r.label  = db.value( 10 ).toString();

      runInfo << r;
   }

   if ( runInfo.size() < 1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "There are no US3 runs in the DB to load.\n" ) );
      return false;
   }

   return true;
}

// Function to sort rows when column header is clicked
void US_GetDBExp::columnClicked( int col )
{
   tw -> sortItems( col );
}

// Function to pass information back when select button is pressed
void US_GetDBExp::select( void )
{
   int ndx = tw ->currentRow();

   expID = tw ->item( ndx, 0 )->text();
   accept();
}
