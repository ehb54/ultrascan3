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

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   // Investigator selection
   personID = 0;
   QHBoxLayout* investigator = new QHBoxLayout;

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   investigator->addWidget( pb_investigator );
 
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   investigator->addWidget( le_investigator );
   main->addLayout( investigator );

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
   headers << tr( "Run ID" ) << tr( "DB Timestamp" );
   tree->setHeaderLabels( headers );

   populate_tree();
   main->addWidget( tree );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

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
      //qDebug() << f << "download_aucData" <<  dbIDs[ runID ][ i ].toInt();
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
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to databasee \n" ) + db.lastError() );
      return;
   }

   QStringList q( "all_rawDataIDs" );
   //q << QString::number( personID;
   db.query( q );

   QTreeWidgetItem*                  top;

   runIDs   .clear();
   dbIDs    .clear();
   filenames.clear();

   while ( db.next() )
   {
      QString rawDataID = db.value( 0 ).toString();
      QString filename  = db.value( 2 ).toString();
      //QString date  = db.value( 3 ).toString();

      QStringList sl    = filename.split( "." );
      QString     runID = sl[ 0 ];

      // Handle multiple auc files for one RunID
      if ( ! runIDs.contains( runID ) )
      {
         top  = new QTreeWidgetItem( QStringList( runID ) );
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
      
      // Need to add date to QStringList
      QStringList item( filename );
      //item << date;
      new QTreeWidgetItem( top, item );
   }

   tree->expandAll();
   tree->resizeColumnToContents( 0 );
   tree->collapseAll();
}
