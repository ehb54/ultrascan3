//! \file us_load_db.cpp

#include "us_load_db.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"

US_LoadDB::US_LoadDB( QString& wd ) : US_WidgetsDialog( 0, 0 ), workingDir( wd )
{
   setWindowTitle( tr( "Load AUC Data From DB" ) );
   setPalette( US_GuiSettings::frameColor() );

   investigators.clear();

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   // Investigator selection
   personID = -1;
   QHBoxLayout* investigator = new QHBoxLayout;

   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   investigator->addWidget( pb_investigator );
 
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   le_investigator->setPalette ( gray );
   investigator->addWidget( le_investigator );
   main->addLayout( investigator );

   // Search
   QHBoxLayout* search = new QHBoxLayout;
   QLabel* lb_search = us_label( tr( "Search" ) );
   search->addWidget( lb_search );
 
   le_search = us_lineedit( "" );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( limit_data ( const QString& ) ) );
   search->addWidget( le_search );
   main->addLayout( search );

   // Tree
   tree = new QTreeWidget;
   tree->setAllColumnsShowFocus( true );
   tree->setIndentation        ( 20 );
   tree->setSelectionBehavior  ( QAbstractItemView::SelectRows );
   tree->setAutoFillBackground ( true );

   tree->setPalette( US_GuiSettings::editColor() );
   tree->setFont   ( QFont( US_GuiSettings::fontFamily(),
                     US_GuiSettings::fontSize  () ) );

   QStringList headers;
   headers << tr( "Run ID" ) << tr( "Investigator/DB Timestamp" );
   tree->setHeaderLabels( headers );

   populate_tree();
   main->addWidget( tree );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_expand = us_pushbutton( tr( "Expand All" ) );
   connect( pb_expand, SIGNAL( clicked() ), SLOT( expand() ) );
   buttons->addWidget( pb_expand );

   QPushButton* pb_collapse = us_pushbutton( tr( "Collapse All" ) );
   connect( pb_collapse, SIGNAL( clicked() ), SLOT( collapse() ) );
   buttons->addWidget( pb_collapse );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Load" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( load() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   resize( 600, 400 );
}

void US_LoadDB::load( void )
{
   QTreeWidgetItem* item = tree->currentItem();
   if ( item == NULL ) return;

   // Get top level item
   while ( item->parent() != NULL ) item = item->parent();

   // Set working directory and create it if necessary
   QString runID = item->text( 0 );
   workingDir    = US_Settings::resultDir() + "/" + runID;

   QDir work( US_Settings::resultDir() );
   work.mkdir( runID );

   // Connect to DB
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to databasee \n" ) + db.lastError() );
      return;
   }

   // Get auc files from DB
   for ( int i = 0; i < dbIDs[ runID ].size(); i++ )
   {
      QString f = workingDir + "/" + filenames[ runID ][ i ];
      db.readBlobFromDB( f, "download_aucData", dbIDs[ runID ][ i ].toInt() );
   }

   accept();
}

void US_LoadDB::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

void US_LoadDB::assign_investigator( int invID,
      const QString& lname, const QString& fname)
{
   personID = invID;
   le_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );

   populate_tree();
}

void US_LoadDB::populate_tree( void )
{
   le_search->clear();
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to databasee \n" ) + db.lastError() );
      return;
   }

   QStringList q( "all_rawDataIDs" );
   q << QString::number( personID );
   db.query( q );

   QTreeWidgetItem*                  top;

   runIDs   .clear();
   dbIDs    .clear();
   filenames.clear();

   while ( db.next() )
   {
      QString rawDataID = db.value( 0 ).toString();
      QString filename  = db.value( 2 ).toString();
      QString date      = db.value( 5 ).toString();
      QString p_id      = db.value( 8 ).toString();

      QStringList sl    = filename.split( "." );
      QString     runID = sl[ 0 ];

      // Handle multiple auc files for one RunID
      if ( ! runIDs.contains( runID ) )
      {
         QStringList topItem;
         topItem << runID << investigator_name( p_id );
         top  = new QTreeWidgetItem( topItem );
         tree->addTopLevelItem( top );
         runIDs[ runID ] = top;

         dbIDs    [ runID ] = QStringList( rawDataID );
         filenames[ runID ] = QStringList( filename );
      }
      else
      {
         top = runIDs[ runID ];
         dbIDs    [ runID ] << rawDataID;
         filenames[ runID ] << filename;
      }
      
      QStringList item( filename );
      item << date;
      new QTreeWidgetItem( top, item );
   }

   if ( runIDs.size() == 0 )
   {
      QMessageBox::information( this,
         tr( "No AUC Files" ),
         tr( "No AUC files were found for the investigator." ) );
      
      return;
   }

   tree->expandAll();
   tree->resizeColumnToContents( 0 );
   tree->collapseAll();
}

void US_LoadDB::limit_data( const QString& search )
{
   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      bool found = ( search.isEmpty() ) ? true 
                                        : item->text( 0 ).contains( search );
      item->setHidden( ! found );
   }
}

void US_LoadDB::expand( void )
{
   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      item->setExpanded( true );
   }
}

void US_LoadDB::collapse( void )
{
   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      item->setExpanded( false );
   }
}

QString US_LoadDB::investigator_name( const QString& id )
{
   if ( ! investigators.contains( id ) )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );

      QStringList q( "get_person_info" );
      q << id;
      db.query( q );

      db.next();

      QString lname = db.value( 0 ).toString();
      QString fname = db.value( 1 ).toString();

      investigators[ id ] = fname + " " + lname;
   }

   return investigators[ id ];
}
