//! \file us_load_auc.cpp

#include "us_load_auc.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_util.h"
#include "us_editor.h"


US_LoadAUC::US_LoadAUC( bool local, QString auto_mode, QVector< US_DataIO::RawData >& rData,
   QStringList& trips, QString& wdir ) : US_WidgetsDialog( 0, 0 ),
   rawList( rData ), triples( trips ), workingDir( wdir )
{
   int ddstate;

   us_auto_mode   = true;

   if ( local )
   {
      setWindowTitle( tr( "Load AUC Data from Local Disk" ) );
      ddstate   = US_Disk_DB_Controls::Disk;
   }
   else
   {
      setWindowTitle( tr( "Load AUC Data from DB" ) );
      ddstate   = US_Disk_DB_Controls::DB;
   }

   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   dkdb_cntrls   = new US_Disk_DB_Controls( ddstate );
   connect( dkdb_cntrls, SIGNAL( changed     ( bool ) ),
            this,        SLOT( update_disk_db( bool ) ) );

   // Investigator selection
   personID = US_Settings::us_inv_ID();
   QHBoxLayout* investigator = new QHBoxLayout;

   pb_invest = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_invest, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   investigator->addWidget( pb_invest );
   if ( US_Settings::us_inv_level() < 3 )
      pb_invest->setEnabled( false );
 
   int id = US_Settings::us_inv_ID();
   QString number = ( id > 0 )
      ? QString::number( personID ) + ": "
      : "";

   le_invest = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   investigator->addWidget( le_invest );

   // Search
   QHBoxLayout* search = new QHBoxLayout;
   QLabel* lb_search   = us_label( tr( "Search" ) );
   search->addWidget( lb_search );
 
   le_search = us_lineedit( "" );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( limit_data ( const QString& ) ) );
   search->addWidget( le_search );

   // Tree
   QFont tr_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   tree = new QTreeWidget;
   tree->setFrameStyle         ( QFrame::NoFrame );
   tree->setPalette            ( US_GuiSettings::editColor() );
   tree->setFont               ( tr_font );
   tree->setIndentation        ( 20 );
   tree->setSelectionBehavior  ( QAbstractItemView::SelectRows );
   tree->setSelectionMode      ( QAbstractItemView::ExtendedSelection );
   tree->setAutoFillBackground ( true );
   tree->installEventFilter    ( this );


   QStringList headers;
   headers << tr( "Run|Triple" )
           << tr( "Date" )
           << tr( "DbID" )
           << tr( "Label" );
   tree->setColumnCount( 4 );
   tree->setHeaderLabels( headers );
   tree->setSortingEnabled( false );

   te_notes            = new QTextEdit();
   te_notes->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   te_notes->setTextColor( Qt::blue );
   te_notes->setText( tr( "Right-mouse-button-click on a list selection"
                          " for details." ) );
   int font_ht = QFontMetrics( tr_font ).lineSpacing();
   te_notes->setMaximumHeight( font_ht * 2 + 12 );

   sel_run    = false;
   populate_tree();

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_expand   = us_pushbutton( tr( "Expand All"   ) );
   QPushButton* pb_collapse = us_pushbutton( tr( "Collapse All" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help"         ) );
   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel"       ) );
   QPushButton* pb_shedits  = us_pushbutton( tr( "Show Triples" ) );
   QPushButton* pb_accept   = us_pushbutton( tr( "Load"         ) );

   buttons->addWidget( pb_expand );
   buttons->addWidget( pb_collapse );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_shedits );
   buttons->addWidget( pb_accept );

   connect( pb_expand,   SIGNAL( clicked() ), SLOT( expand()   ) );
   connect( pb_collapse, SIGNAL( clicked() ), SLOT( collapse() ) );
   connect( pb_help,     SIGNAL( clicked() ), SLOT( help()     ) );
   connect( pb_cancel,   SIGNAL( clicked() ), SLOT( reject()   ) );
   connect( pb_shedits,  SIGNAL( clicked() ), SLOT( fill_in()  ) );
   connect( pb_accept,   SIGNAL( clicked() ), SLOT( load()     ) );

   main->addLayout( dkdb_cntrls );
   main->addLayout( investigator );
   main->addLayout( search );
   main->addWidget( tree );
   main->addWidget( te_notes );
   main->addLayout( buttons );

   resize( 800, 500 );
}



US_LoadAUC::US_LoadAUC( bool local, QVector< US_DataIO::RawData >& rData,
   QStringList& trips, QString& wdir ) : US_WidgetsDialog( 0, 0 ),
   rawList( rData ), triples( trips ), workingDir( wdir )
{
   int ddstate;

   us_auto_mode   = false;

   if ( local )
   {
      setWindowTitle( tr( "Load AUC Data from Local Disk" ) );
      ddstate   = US_Disk_DB_Controls::Disk;
   }
   else
   {
      setWindowTitle( tr( "Load AUC Data from DB" ) );
      ddstate   = US_Disk_DB_Controls::DB;
   }

   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   dkdb_cntrls   = new US_Disk_DB_Controls( ddstate );
   connect( dkdb_cntrls, SIGNAL( changed     ( bool ) ),
            this,        SLOT( update_disk_db( bool ) ) );

   // Investigator selection
   personID = US_Settings::us_inv_ID();
   QHBoxLayout* investigator = new QHBoxLayout;

   pb_invest = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_invest, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   investigator->addWidget( pb_invest );
   if ( US_Settings::us_inv_level() < 3 )
      pb_invest->setEnabled( false );
 
   int id = US_Settings::us_inv_ID();
   QString number = ( id > 0 )
      ? QString::number( personID ) + ": "
      : "";

   le_invest = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   investigator->addWidget( le_invest );

   // Search
   QHBoxLayout* search = new QHBoxLayout;
   QLabel* lb_search   = us_label( tr( "Search" ) );
   search->addWidget( lb_search );
 
   le_search = us_lineedit( "" );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( limit_data ( const QString& ) ) );
   search->addWidget( le_search );

   // Tree
   QFont tr_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   tree = new QTreeWidget;
   tree->setFrameStyle         ( QFrame::NoFrame );
   tree->setPalette            ( US_GuiSettings::editColor() );
   tree->setFont               ( tr_font );
   tree->setIndentation        ( 20 );
   tree->setSelectionBehavior  ( QAbstractItemView::SelectRows );
   tree->setSelectionMode      ( QAbstractItemView::ExtendedSelection );
   tree->setAutoFillBackground ( true );
   tree->installEventFilter    ( this );


   QStringList headers;
   headers << tr( "Run|Triple" )
           << tr( "Date" )
           << tr( "DbID" )
           << tr( "Label" );
   tree->setColumnCount( 4 );
   tree->setHeaderLabels( headers );
   tree->setSortingEnabled( false );

   te_notes            = new QTextEdit();
   te_notes->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   te_notes->setTextColor( Qt::blue );
   te_notes->setText( tr( "Right-mouse-button-click on a list selection"
                          " for details." ) );
   int font_ht = QFontMetrics( tr_font ).lineSpacing();
   te_notes->setMaximumHeight( font_ht * 2 + 12 );

   sel_run    = false;
   populate_tree();

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_expand   = us_pushbutton( tr( "Expand All"   ) );
   QPushButton* pb_collapse = us_pushbutton( tr( "Collapse All" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help"         ) );
   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel"       ) );
   QPushButton* pb_shedits  = us_pushbutton( tr( "Show Triples" ) );
   QPushButton* pb_accept   = us_pushbutton( tr( "Load"         ) );

   buttons->addWidget( pb_expand );
   buttons->addWidget( pb_collapse );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_shedits );
   buttons->addWidget( pb_accept );

   connect( pb_expand,   SIGNAL( clicked() ), SLOT( expand()   ) );
   connect( pb_collapse, SIGNAL( clicked() ), SLOT( collapse() ) );
   connect( pb_help,     SIGNAL( clicked() ), SLOT( help()     ) );
   connect( pb_cancel,   SIGNAL( clicked() ), SLOT( reject()   ) );
   connect( pb_shedits,  SIGNAL( clicked() ), SLOT( fill_in()  ) );
   connect( pb_accept,   SIGNAL( clicked() ), SLOT( load()     ) );

   main->addLayout( dkdb_cntrls );
   main->addLayout( investigator );
   main->addLayout( search );
   main->addWidget( tree );
   main->addWidget( te_notes );
   main->addLayout( buttons );

   resize( 800, 500 );
}





// Load the selected raw data
void US_LoadAUC::load( void )
{
   QList< QTreeWidgetItem* > items = tree->selectedItems();
   int nitems = items.count();
   QList< DataDesc >  sdescs;

   if ( nitems == 0 )
   {
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "No selection has been made" ) );
      return;
   }

   // If we are here after a whole-run load at stage 1 (top-level only),
   //  then we fill out the data map for the selected run and load all AUCS.
   if ( ! sel_run )
   {
     QTreeWidgetItem* item  = items[ 0 ];

      while ( item->parent() != NULL )
         item          = item->parent();

      runID_sel     = item->text( 0 );         // Get the selected run ID
      sel_run       = true;                    // Mark that a run is selected
      datamap.clear();
qDebug() << "Ed:Ld: runID_sel" << runID_sel;

      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
      te_notes->setText( tr( "Gathering information on raw triples for"
                             " run ID\n\"%1\"..." ).arg( runID_sel ) );
      qApp->processEvents();
      qApp->processEvents();


      
      ////////////////////////////////////////////////////////////////////////////

      if ( dkdb_cntrls->db() )
      {  // Scan database for AUC then load all of them
         scan_db();

         sdescs    = datamap.values();
         te_notes->setText( tr( "Loading data from Database ..." )  );
         qApp->processEvents();
         qApp->processEvents();

	 //ALEXEY: DEBUG /////////////////////////////////////////////////////////////
	 if ( us_auto_mode )
	   {
	     //check if edit profile(s) exist for selected run:
	     QStringList editDataIDs;
	     QStringList filenames;
	     
	     qDebug() << "DataMap Keys -- " <<  datamap.keys();
	     for ( int i=0; i< datamap.keys().size(); ++i )
	       {
		 qDebug() << datamap.keys()[ i ] << ", rawDataGUID, rawDataID -- "
			  << datamap[ datamap.keys()[ i ] ].rawGUID
			  << datamap[ datamap.keys()[ i ] ].DB_id;

		 //read editedData IDs for all triples in the selected RUN
		 US_Passwd pw;
		 QString masterPW = pw.getPasswd();
		 US_DB2 db( masterPW );
		 
		 if ( db.lastErrno() != US_DB2::OK )
		   {
		     QMessageBox::warning( this, tr( "Connection Problem" ),
					   tr( "Assessing EditedData IDs: Could not connect to database \n" ) + db.lastError() );
		     return;
		   }

		 QStringList qry;
		 qry.clear();
		 qry << "get_editedDataIDs" << QString::number( datamap[ datamap.keys()[ i ] ].DB_id );
		 db.query( qry );
		 		 
		 while ( db.next() )
		   {
		     editDataIDs << db.value( 0 ).toString();
		     filenames   << db.value( 2 ).toString();
		   }
	       }
	     
	     if ( editDataIDs.size() == 0 )
	       {
		 QMessageBox::warning( this,
				       tr( "No Edits Available" ),
				       tr( "No Edit children exist for selected run"
					   " '%1'. The run will be hidden." ).arg( runID_sel ) );
		 
		 item->setHidden( true );
		 sel_run = false;

		 QApplication::restoreOverrideCursor();
		 QApplication::restoreOverrideCursor();
		 return;
	       }
	   }
	 
         load_db( sdescs );
      }

      else
      {  // Scan local files for AUC then load all of them
         scan_disk();

         sdescs    = datamap.values();
         te_notes->setText( tr( "Loading data from Local Disk ..." )  );
         qApp->processEvents();
         qApp->processEvents();

         load_disk( sdescs );
      }

      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

      accept();
      return;
   }

   QTreeWidgetItem*   item = items.at( 0 );
qDebug() << "Ed:Ld: nitems" << items.size();
   int ntops  = 0;
   int nruns  = 0;
   QStringList tripls;
   QStringList runIDs;
   QStringList lkeys;
   QString     runID;

   for ( int ii = 0; ii < nitems; ii++ )
   {  // Review and record selected list items
      item          = items.at( ii );
      QString lkey  = item->text( 0 );
      DataDesc ddesc;

      if ( item->parent() != NULL )
      {  // Not top-level
         lkey       = item->parent()->text( 0 ) + "." + lkey;
         ddesc      = datamap[ lkey ];
         tripls << ddesc.tripID;           // Save selected triples
      }

      else
      {  // Top-level
         ntops++;                          // Count top-level items
         lkey       = lkey + "." + item->child( 0 )->text( 0 );
         ddesc      = datamap[ lkey ];
      }

      runID         = ddesc.runID;

      if ( !runIDs.contains( runID ) )
      {  // New runID:  save it and bump run count
         runIDs << runID;
         nruns++;
      }

      // Save a description of the selected item
      sdescs << ddesc;
   }
qDebug() << "Ed:Ld: ntops" << ntops << "trips" << tripls.count()
 << "nitems" << nitems << "nruns" << nruns;

   // Verify that selections are consistent
   if ( ntops > 1  ||  nruns > 1 )
   {
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "Only items from one run may be selected." ) );
      return;
   }

   if ( ntops != nitems  &&  tripls.count() != nitems )
   {
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "A single run or items from a single run must be selected." ) );
      return;
   }

   if ( ntops == 1 )
   {  // If a single top-level item was chosen, get all its triples
      QString runID  = sdescs.at( 0 ).runID;    // We have the first triple
      item           = items .at( 0 );

      for ( int ii = 1; ii < item->childCount(); ii++ )
      {  // Save any run triples beyond first
         QString lkey   = runID + "." + item->child( ii )->text( 0 );
         sdescs << datamap[ lkey ];
      }
   }

   // Load the actual data from DB or Disk
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   te_notes->setText( tr( "Loading data from " ) +
                      ( dkdb_cntrls->db() ? tr("Database") : tr("Local Disk") )
                      + " ..." );

   if ( dkdb_cntrls->db() )
      load_db  ( sdescs );
   else
      load_disk( sdescs );

   QApplication::restoreOverrideCursor();

   accept();
}

// Open the dialog to select a new investigator
void US_LoadAUC::sel_investigator( void )
{
   if ( !dkdb_cntrls->db() )  return;   // Ignore if Disk selected

   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog, SIGNAL( investigator_accepted( int ) ),
            this,       SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Assign this dialog's investigator text and re-populate tree
void US_LoadAUC::assign_investigator( int invID )
{
   personID = invID;
   QString number = ( invID > 0 ) ? QString::number( invID ) + ": " : "";

   le_invest->setText( number + US_Settings::us_inv_name() );

   populate_tree();
}

// Populate the list tree of runs and triples
void US_LoadAUC::populate_tree( void )
{
   int naucf = 0;
   le_search->clear();
   tree     ->clear();
   datamap   .clear();
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
qDebug() << "Ed:Ptree: sel_run" << sel_run;

   if ( sel_run )
   {  // A run has been selected, so scan for AUCs
      if ( dkdb_cntrls->db() )
      {  // Scan database for AUC
         naucf = scan_db();
      }

      else
      {  // Scan local files for AUC
         naucf = scan_disk();
      }

      te_notes->setText( tr( "Right-mouse-button-click on a list selection"
                             " for details.\n"
                             "Select triples and click on \"Load\""
                             " to load selected AUC data." ) );
   }

   else
   {  // No run is selected, so scan for runs
      if ( dkdb_cntrls->db() )
      {  // Scan database for runs
         naucf = scan_run_db();
      }

      else
      {  // Scan local files for runs
         naucf = scan_run_disk();
      }

      te_notes->setText( tr( "Select a run, then click on \"Show Triples\""
                             " to fill in triples;\n"
                             "or click on \"Load\" to load all triples"
                             " AUC data for the selected run." ) );
   }

   qApp->processEvents();
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

   if ( naucf == 0 )
   {
      QMessageBox::information( this,
         tr( "No AUC Files" ),
         tr( "No AUC files were found." ) );
      
      return;
   }

   // Populate the tree with top level items sorted
   QStringList runIDs;
   QList< DataDesc > ddescs = datamap.values();
   QTreeWidgetItem* top = NULL;
   QStringList headers;
   headers << ( sel_run ? tr( "Run|Triple" ) : tr( "Run" ) )
           << tr( "Date" )
           << tr( "DbID" )
           << tr( "Label" );
   tree->setColumnCount( 4 );
   tree->setHeaderLabels( headers );
   tree->setSortingEnabled( false );

   for ( int ii = 0; ii < naucf; ii++ )
   {
      DataDesc ddesc  = ddescs.at( ii );     // Item description
      QStringList item( ddesc.tripID );
      QStringList topItem;
      topItem << ddesc.runID
              << QString( ddesc.date ).section( " ", 0, 0 )
              << QString().sprintf( "%6d", ddesc.DB_id )
              << ddesc.label;

      if ( ! runIDs.contains( ddesc.runID ) )
      {  // First entry for a run, so preceed with top-level item
         top  = new QTreeWidgetItem( topItem );
         tree->addTopLevelItem( top );
         runIDs << ddesc.runID;
      }

      // If in auc mode, create a 2nd-level tree item
      if ( sel_run )
         new QTreeWidgetItem( top, item );
   }

   tree->resizeColumnToContents( 0 );
   tree->resizeColumnToContents( 1 );
   tree->resizeColumnToContents( 2 );
   tree->setSortingEnabled( true );
   tree->sortByColumn( 0, Qt::AscendingOrder );   // Insure triples in order
   tree->sortByColumn( 1, Qt::DescendingOrder );  // Default latest-on-top
qDebug() << "PopTr: naucf valsize" << naucf << datamap.values().size();

}

// Limit displayed tree data based on the search text
void US_LoadAUC::limit_data( const QString& search )
{
   bool have_search = ! search.isEmpty();

   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item  = tree->topLevelItem( i );
      QString          lkey  = item->text( 0 );

      // Hide the item if search text exists and the lkey does not contain it
      item->setHidden( have_search  &&
                       ! lkey.contains( search, Qt::CaseInsensitive ) );
   }
}

// Expand the entire tree
void US_LoadAUC::expand( void )
{
   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      item->setExpanded( true );
   }
}

// Collapse the entire tree
void US_LoadAUC::collapse( void )
{
   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item = tree->topLevelItem( i );
      item->setExpanded( false );
   }
}

// Fill in tree (AUC) children for a selected run
void US_LoadAUC::fill_in( void )
{
   QList< QTreeWidgetItem* > selitems = tree->selectedItems();

   if ( selitems.size() < 1 )
   {  // "Show Triples" with no run selected:  build full data tree
      runID_sel    = "";
      te_notes->setText( tr( "Reading AUC information to fully populate"
                             " the list data tree..." ) );
   }

   else
   {  // "Show Triples" with run selected:  build a data tree for selected run
      QTreeWidgetItem* twi = selitems[ 0 ];
      while ( twi->parent() != NULL )
         twi          = twi->parent();

      runID_sel    = twi->text( 0 );
      te_notes->setText( tr( "Reading AUC information for run \"%1\",\n"
                             " to populate its list data tree..." )
                         .arg( runID_sel ) );
   }

   qApp->processEvents();
   sel_run      = true;

   populate_tree();

   expand();
   tree->resizeColumnToContents( 0 );
   collapse();
}

// Signal to the caller that Disk/DB choice has changed and repopulate tree
void US_LoadAUC::update_disk_db( bool db )
{
   emit changed( db );

   bool invest = US_Settings::us_inv_level() > 0  &&  db;
   pb_invest->setEnabled( invest );
   le_invest->setEnabled( invest );

   setWindowTitle( tr( "Load AUC Data from " )
      + ( db ? tr( "DB" ) : tr( "Local Disk" ) ) );

   populate_tree();
}

// Filter events to catch right-mouse-button-click on list widget
bool US_LoadAUC::eventFilter( QObject* obj, QEvent* e )
{
   if ( obj == tree  &&  e->type() == QEvent::ContextMenu )
   {  // It is right-mouse-button-click, so show data information
      QPoint mpos = ((QContextMenuEvent*)e)->pos();

      show_data_info( mpos );

      return false;
   }

   else
   {  // Pass all other events to normal handler
      return US_WidgetsDialog::eventFilter( obj, e );
   }
}

// Show detailed information about a selected list item
void US_LoadAUC::show_data_info( QPoint pos )
{
   QList< QTreeWidgetItem* > selitems = tree->selectedItems();
   QTreeWidgetItem* item  = selitems.size() > 0 ? selitems.at( 0 )
                                                : tree->itemAt( pos );
   QTreeWidgetItem* pitem = item->parent();
   QTreeWidgetItem* citem = item->child( 0 );
   bool     toplev = ( pitem == NULL );
   QString  runID  = toplev ? item ->text( 0 ) : pitem->text( 0 );
   runID           = runID.section( "[", 0, 0 ).simplified();
   QString  tripID = toplev ? citem->text( 0 ) : item ->text( 0 );
   QString  lkey   = runID + "." + tripID;
   DataDesc ddesc  = datamap[ lkey ];
   QString  dtext  = tr( "Data Information for " );
   dtext          += toplev ? tr( "the first triple of run %1" ).arg( runID )
                            : tr( "item %1" ).arg( lkey );
   dtext           = dtext + "\n"
      + tr( "\n  List Item Key:          " ) + lkey
      + tr( "\n  AUC Database ID:        " ) + QString::number( ddesc.DB_id )
      + tr( "\n  AUC Global ID:          " ) + ddesc.rawGUID
      + tr( "\n  Filename:               " ) + ddesc.filename
      + tr( "\n  Last Updated:           " ) + ddesc.date
      + tr( "\n  Label:                  " ) + ddesc.label
      + tr( "\n  Run ID:                 " ) + runID
      + tr( "\n  Triple ID:              " ) + tripID
      + tr( "\n  Triple Index:           " ) + QString::number( ddesc.tripndx )
      + tr( "\n  Triples per Run:        " ) + QString::number( ddesc.tripknt )
      + "";

   // Open a dialog and display run/triple information
   US_Editor* eddiag = new US_Editor( US_Editor::LOAD, true, "", this );
   eddiag->setWindowTitle( tr( "Data Information" ) );
   eddiag->move( this->pos() + pos + QPoint( 500, 100 ) );
   eddiag->resize( 720, 240 );
   eddiag->e->setFont( QFont( US_Widgets::fixedFont().family(),
                              US_GuiSettings::fontSize() ) );
   eddiag->e->setText( dtext );
   eddiag->show();
}

// Scan for AUC entries in the database
int US_LoadAUC::scan_db()
{
   int  naucf        = 0;
   bool rfilter      = ! runID_sel.isEmpty();
   QStringList runIDs;
   QStringList infoDs;
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to database\n" ) + db.lastError() );
      return naucf;
   }

   QStringList query;
   QString idInv     = QString::number( personID );
   QString label     = db.value( 1 ).toString();

   if ( rfilter )
      query << "get_raw_desc_by_runID" << idInv << runID_sel;
   else
      query << "get_rawData_desc" << idInv;

   db.query( query );

   while ( db.next() )
   {  // Accumulate data description objects from database information
      QString rawDataID = db.value( 0 ).toString();
      QString label     = db.value( 1 ).toString();
      QString filename  = db.value( 2 ).toString();
      QString date      = db.value( 5 ).toString() + " UTC";
      QString rawPers   = db.value( 6 ).toString();
      QString rawGUID   = db.value( 7 ).toString();
      QString runID     = filename.section( ".",  0, -6 );

      QString tripID    = filename.section( ".", -4, -2 );
      QString lkey      = runID + "." + tripID;
      QString idata     = label + "^" +
                          runID + "^" +
                          tripID + "^" +
                          filename + "^" +
                          rawGUID + "^" +
                          rawDataID + "^" +
                          date;

      runIDs << runID;    // Save each run
      infoDs << idata;    // Save concatenated description string
      naucf++;
//if(rfilter)
//qDebug() << "LdA: naucf" << naucf << "runID" << runID << "idata" << idata
// << "rawPers" << rawPers << "idInv" << idInv;
   }

   // Create the data descriptions map
   create_descs( runIDs, infoDs, naucf );

   return naucf;
}

// Scan for AUC files on local disk
int US_LoadAUC::scan_disk()
{
   int     naucf    = 0;
   bool    rfilter  = ! runID_sel.isEmpty();
   QString rdir     = US_Settings::resultDir();
   QStringList aucfilt( "*.auc" );
   QStringList runIDs;
   QStringList infoDs;
   QStringList aucdirs  = QDir( rdir ).entryList(
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );

   for ( int ii = 0; ii < aucdirs.size(); ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
//qDebug() << "ScDk: ii" << ii << "subdir" << subdir;
      QStringList aucfiles = QDir( subdir ).entryList(
            aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString aucfbase     = aucfiles.at( 0 );
      QString runID        = QString( aucfiles.at( 0 ) ).section( ".",  0, -6 );

      if ( rfilter  &&  runID != runID_sel )
         continue;

      for ( int jj = 0; jj < aucfiles.size(); jj++ )
      {
         QString     aucfbase = aucfiles.at( jj );
         QString     aucfname = subdir + "/" + aucfbase;
         QString     runID    = aucfbase.section( ".",  0, -6 );
         QString     tripID   = aucfbase.section( ".", -4, -2 );
         QString     lkey     = runID + "." + tripID;
         QString     label    = runID;
//qDebug() << "ScDk:   jj" << jj << "lkey" << lkey;
         QString     date     = US_Util::toUTCDatetimeText(
             QFileInfo( aucfname ).lastModified().toUTC()
             .toString( Qt::ISODate ), true );

         QString     xmlfbase = aucfbase.section( ".",  0, -5 ) + ".xml";
         QString     xmlfname = subdir + "/" + xmlfbase;
         QFile filei( xmlfname );
//qDebug() << "ScDk:  xmlfbase" << xmlfbase;

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QString     rawGUID;
         QString     rawDataID;
         QXmlStreamReader xml( &filei );
         QXmlStreamAttributes atts;

         while ( ! xml.atEnd() )
         {
            QString rID;
            QString rGUID;
            QString cell;
            QString chan;
            QString wlen;
            xml.readNext();

            if ( xml.isStartElement() )
            {
               if ( xml.name() == "dataset" )
               {
                  atts          = xml.attributes();
                  QString cell  = atts.value( "cell"       ).toString();
                  QString chan  = atts.value( "channel"    ).toString();
                  QString wlen  = atts.value( "wavelength" ).toString();
                  QString trip  = cell + "." + chan + "." + wlen;

                  if ( trip == tripID )
                  {
                     rawDataID     = atts.value( "id"         ).toString();
                     rawGUID       = atts.value( "guid"       ).toString();
                  }
               }

               else if ( xml.name() == "label" )
               {
                  xml.readNext();
                  label         = xml.text().toString();
               }
            }
         }

         filei.close();
         QString idata     = label + "^" +
                             runID + "^" +
                             tripID + "^" +
                             aucfbase + "^" +
                             rawGUID + "^" +
                             rawDataID + "^" +
                             date;
         runIDs << runID;    // Save each run
         infoDs << idata;    // Save concatenated description string
         naucf++;
//qDebug() << "ScDk: run" << runID << "naucf" << naucf << "label" << label;
      }
   }
//qDebug() << "ScDk:   naucf" << naucf << "runs" << runIDs.size();

   // Create the data descriptions map
   create_descs( runIDs, infoDs, naucf );

   return naucf;
}

// Scan for Run entries in the database
int US_LoadAUC::scan_run_db()
{
   int nruns         = 0;
   QStringList runIDs;
   QStringList infoDs;
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
        tr( "Could not connect to database\n" ) + db.lastError() );
      return nruns;
   }

   QString rawGUID   = "(unknown)";
   QString tripID    = "0.A.999";
   QString filename  = "(unknown)";
   QStringList q( "get_experiment_desc" );
   q << QString::number( personID );
   db.query( q );

   while ( db.next() )
   {  // Accumulate data description objects from database information
      QString rawDataID = db.value( 0 ).toString();
      QString runID     = db.value( 1 ).toString();
      QString label     = db.value( 4 ).toString();
      QString date      = db.value( 5 ).toString() + " UTC";

      QString lkey      = runID + "." + tripID;
      QString idata     = label + "^" +
                          runID + "^" +
                          tripID + "^" +
                          filename + "^" +
                          rawGUID + "^" +
                          rawDataID + "^" +
                          date;

      //ALEXEY : db.value( 2 ).toString() => type [..., buoyancy,... ]
      QString etype      = db.value( 2 ).toString();
      bool expIsBuoyancy = ( etype.compare( "Buoyancy", Qt::CaseInsensitive ) == 0 );
      
      if ( us_auto_mode )
	{
	  if ( expIsBuoyancy  )
	    {
	      runIDs << runID;    // Save each run
	      infoDs << idata;    // Save concatenated description string
	      nruns++;
	    }
	}
      else
	{
	  runIDs << runID;    // Save each run
	  infoDs << idata;    // Save concatenated description string
	  nruns++;
	}
   }

   // Create the data descriptions map
   create_descs( runIDs, infoDs, nruns );
   return nruns;
}

// Scan for Run entries on the local disk
int US_LoadAUC::scan_run_disk()
{
   int nruns         = 0;
   QString rawGUID   = "(unknown)";
   QString tripID    = "0.A.999";
   QString filename  = "(unknown)";
   QString     rdir     = US_Settings::resultDir();
   QStringList aucfilt( "*.auc" );
   QStringList runIDs;
   QStringList infoDs;
   QStringList aucdirs  = QDir( rdir ).entryList(
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );

   for ( int ii = 0; ii < aucdirs.size(); ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList(
            aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString     aucfbase = aucfiles.at( 0 );
      QString     aucfname = subdir + "/" + aucfbase;
      QString     runID    = aucfbase.section( ".",  0, -6 );
      QString     tripID   = aucfbase.section( ".", -4, -2 );
      QString     lkey     = runID + "." + tripID;
      QString     label    = runID;
//qDebug() << "ScDk: ii aucfbase" << ii << aucfbase << "lkey" << lkey;
      QString     date     = US_Util::toUTCDatetimeText(
          QFileInfo( aucfname ).lastModified().toUTC()
          .toString( Qt::ISODate ), true );

      QString     xmlfbase = aucfbase.section( ".",  0, -5 ) + ".xml";
      QString     xmlfname = subdir + "/" + xmlfbase;
      QFile filei( xmlfname );
//qDebug() << "ScDk:  xmlfbase" << xmlfbase;

      if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QString     rawGUID;
      QString     rawDataID;
      QXmlStreamReader xml( &filei );
      QXmlStreamAttributes atts;

      while ( ! xml.atEnd() )
      {
         QString rID;
         QString rGUID;
         QString cell;
         QString chan;
         QString wlen;
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "dataset" )
            {
               atts          = xml.attributes();
               QString cell  = atts.value( "cell"       ).toString();
               QString chan  = atts.value( "channel"    ).toString();
               QString wlen  = atts.value( "wavelength" ).toString();
               QString trip  = cell + "." + chan + "." + wlen;

               if ( trip == tripID )
               {
                  rawDataID     = atts.value( "id"         ).toString();
                  rawGUID       = atts.value( "guid"       ).toString();
               }
            }

            else if ( xml.name() == "label" )
            {
               xml.readNext();
               label         = xml.text().toString();
            }
         }
      }

      filei.close();
      QString idata     = label + "^" +
                          runID + "^" +
                          tripID + "^" +
                          aucfbase + "^" +
                          rawGUID + "^" +
                          rawDataID + "^" +
                          date;
      runIDs << runID;    // Save each run
      infoDs << idata;    // Save concatenated description string
      nruns++;
//qDebug() << "ScDk: run" << runID << "naucf" << naucf << "label" << label;
   }
//qDebug() << "ScDk:   naucf" << naucf << "runs" << runIDs.size();

   // Create the data descriptions map
   create_descs( runIDs, infoDs, nruns );

   return nruns;
}

// Create the data descriptions map with indecies and counts
void US_LoadAUC::create_descs( QStringList& runIDs, QStringList& infoDs,
      int naucf )
{
   datamap.clear();       // Clear label-descrip map
   infoDs .sort();        // Sort concat strings by label

   QString prunid  = "";
   int     tripndx = 1;

   for ( int ii = 0; ii < naucf; ii++ )
   {
      // Parse values in concatenation
      QString idata     = infoDs.at( ii );
      QString label     = idata.section( "^", 0, 0 );
      QString runID     = idata.section( "^", 1, 1 );
      QString tripID    = idata.section( "^", 2, 2 );
      QString filename  = idata.section( "^", 3, 3 );
      QString rawGUID   = idata.section( "^", 4, 4 );
      QString rawDataID = idata.section( "^", 5, 5 );
      QString date      = idata.section( "^", 6, 6 );
      QString dcheck    = idata.section( "^", 7, 7 );
      QString lkey      = runID + "." + tripID;
      tripndx           = ( runID == prunid ) ? ( tripndx + 1 ) : 1;
      prunid            = runID;

      // Fill the description object and set count,index
      DataDesc ddesc;
      ddesc.label       = label;
      ddesc.runID       = runID;
      ddesc.tripID      = tripID;
      ddesc.filename    = filename;
      ddesc.rawGUID     = rawGUID;
      ddesc.date        = date;
      ddesc.dcheck      = dcheck;
      ddesc.DB_id       = rawDataID.toInt();
      ddesc.tripknt     = runIDs.count( runID );
      ddesc.tripndx     = tripndx;

//qDebug() << "CrDe: ii tknt" << ii << ddesc.tripknt << "lkey" << lkey;
      if ( datamap.contains( lkey ) )
      {  // Handle the case where the lkey already exists
         qDebug() << "*** DUPLICATE lkey" << lkey << "***";
         lkey              = lkey + "(2)";

         if ( datamap.contains( lkey ) )
         {  // Handle two duplicates
            lkey              = lkey.replace( "(2)", "(3)" );
         }
      }

      datamap[ lkey ]   = ddesc;
   }
   return;
}

// Load the data from the database
void US_LoadAUC::load_db( QList< DataDesc >& sdescs )
{
   int     nerr  = 0;
   QString emsg;
   QString rdir  = US_Settings::resultDir();
   QString runID = sdescs.count() > 0 ? sdescs.at( 0 ).runID : "";
   workingDir    = rdir + "/" + runID;
   rawList.clear();
   triples.clear();

   QDir work( rdir );
   work.mkdir( runID );

   // Connect to DB
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to database\n " ) + db.lastError() );
      return;
   }

   for ( int ii = 0; ii < sdescs.count(); ii++ )
   {  // Loop to load selected data from the database
      US_DataIO::RawData rdata;
      DataDesc ddesc     = sdescs.at( ii );
      int      idRaw     = ddesc.DB_id;
      QString  filebase  = ddesc.filename;
      QString  filename  = workingDir + "/" + filebase;
      QString  triple    = ddesc.tripID.replace( ".", " / " );
      QString  dcheck    = ddesc.dcheck;
      bool     dload_auc = true;
      int      stat      = 0;

      if ( QFile( filename ).exists() )
      {  // AUC file exists, do only download if checksum+size mismatch
         QString  fcheck    = US_Util::md5sum_file( filename );

         if ( dcheck.isEmpty() )
         {
            QStringList query;
            query << "get_rawData" << QString::number( idRaw );
            db.query( query );
            db.next();
            ddesc.dcheck       = db.value( 8 ).toString() + " " +
                                 db.value( 9 ).toString();
            dcheck             = ddesc.dcheck;
         }

         dload_auc          = ( fcheck != dcheck );
      }

      emit progress( tr( "Loading triple " ) + triple );
      qApp->processEvents();

      // Download the DB record to a file (if need be)
      if ( dload_auc )
      {
         db.readBlobFromDB( filename, "download_aucData", idRaw );
         int stat           = db.lastErrno();

         if ( stat != US_DB2::OK )
         {
            nerr++;
            emsg += tr( "Error (%1) downloading to file %2\n" )
                    .arg( stat ).arg( filebase );
         }
      }

      // Read the raw record to memory
      stat = US_DataIO::readRawData( filename, rdata );

      if ( stat != US_DataIO::OK )
      {
         nerr++;
         emsg += tr( "Error (%1) reading file %2\n" )
                 .arg( stat ).arg( filebase );
      }

      // Accumulate lists of data and triples
      rawList << rdata;
      triples << triple;
   }
}

// Load the data from local disk
void US_LoadAUC::load_disk( QList< DataDesc >& sdescs )
{
   int     nerr  = 0;
   QString emsg;
   QString runID = sdescs.count() > 0 ? sdescs.at( 0 ).runID : "";
   QString rdir  = US_Settings::resultDir();
   workingDir    = rdir + "/" + runID;
   rawList.clear();
   triples.clear();
//qDebug() << "LdDk: workingDir" << workingDir;
//qDebug() << "LdDk:  sdescs count" << sdescs.count();

   for ( int ii = 0; ii < sdescs.count(); ii++ )
   {
      US_DataIO::RawData rdata;
      DataDesc ddesc     = sdescs.at( ii );
      QString  filebase  = ddesc.filename;
      QString  filename  = workingDir + "/" + filebase;
      QString  triple    = ddesc.tripID.replace( ".", " / " );

      emit progress( tr( "Loading triple " ) + triple );
      qApp->processEvents();

      // Read the raw data record from the file
      int stat = US_DataIO::readRawData( filename, rdata );

      if ( stat != US_DataIO::OK )
      {
         nerr++;
         emsg += tr( "Error (%1) reading file %2\n" )
                 .arg( stat ).arg( filebase );
      }

      // Accumulate lists of data and triples
      rawList << rdata;
      triples << ddesc.tripID.replace( ".", " / " );
//qDebug() << "LdDk:       ii" << ii << "triple" << triples.last();
   }
//qDebug() << "LdDk:   nerr" << nerr << "err(s):" << emsg;
//qDebug() << "LdDk:     rawList count" << rawList.count();
//qDebug() << "LdDk:     triples count" << triples.count();

   if ( nerr > 0 )
   {
      emsg = ( nerr == 1 ) ? tr( "*** ERROR:\n" )
                           : tr( "*** %1 ERRORS:\n" ).arg( nerr ) + emsg;
      QMessageBox::critical( this, tr( "Data Load Error" ), emsg );
   }
}

