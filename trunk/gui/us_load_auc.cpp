//! \file us_load_auc.cpp

#include "us_load_auc.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_util.h"
#include "us_editor.h"

US_LoadAUC::US_LoadAUC( bool local, QVector< US_DataIO2::RawData >& rData,
   QStringList& trips, QString& wdir ) : US_WidgetsDialog( 0, 0 ),
   rawList( rData ), triples( trips ), workingDir( wdir )
{
   int ddstate;

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
 
   int id = US_Settings::us_inv_ID();
   QString number = ( id > 0 )
      ? QString::number( personID ) + ": "
      : "";

   le_invest = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   investigator->addWidget( le_invest );

   // Search
   QHBoxLayout* search = new QHBoxLayout;
   QLabel* lb_search = us_label( tr( "Search" ) );
   search->addWidget( lb_search );
 
   le_search = us_lineedit( "" );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
                       SLOT  ( limit_data ( const QString& ) ) );
   search->addWidget( le_search );

   // Tree
   tree = new QTreeWidget;
   tree->setAllColumnsShowFocus( true );
   tree->setIndentation        ( 20 );
   tree->setSelectionBehavior  ( QAbstractItemView::SelectRows );
   tree->setSelectionMode      ( QAbstractItemView::ExtendedSelection );
   tree->setAutoFillBackground ( true );
   tree->installEventFilter    ( this );

   tree->setPalette( US_GuiSettings::editColor() );
   tree->setFont   ( QFont( US_GuiSettings::fontFamily(),
                     US_GuiSettings::fontSize  () ) );

   QStringList headers;
   headers << tr( "Run ID" );
   tree->setHeaderLabels( headers );

   populate_tree();

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_expand   = us_pushbutton( tr( "Expand All" ) );
   QPushButton* pb_collapse = us_pushbutton( tr( "Collapse All" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept   = us_pushbutton( tr( "Load" ) );

   buttons->addWidget( pb_expand );
   buttons->addWidget( pb_collapse );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   connect( pb_expand,   SIGNAL( clicked() ), SLOT( expand() ) );
   connect( pb_collapse, SIGNAL( clicked() ), SLOT( collapse() ) );
   connect( pb_help,     SIGNAL( clicked() ), SLOT( help() ) );
   connect( pb_cancel,   SIGNAL( clicked() ), SLOT( reject() ) );
   connect( pb_accept,   SIGNAL( clicked() ), SLOT( load() ) );

   main->addLayout( dkdb_cntrls );
   main->addLayout( investigator );
   main->addLayout( search );
   main->addWidget( tree );
   main->addLayout( buttons );

   resize( 600, 400 );
}

// Load the selected raw data
void US_LoadAUC::load( void )
{
   QList< QTreeWidgetItem* > items = tree->selectedItems();
   int nitems = items.count();

   if ( nitems == 0 )
   {
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "No selection has been made" ) );
      return;
   }

   QList< DataDesc >  sdescs;
   QTreeWidgetItem*   item = items.at( 0 );
   int ntops  = 0;
   int nruns  = 0;
   QStringList tripls;
   QStringList runIDs;
   QStringList labels;
   QString     runID;

   for ( int ii = 0; ii < nitems; ii++ )
   {  // Review and record selected list items
      item          = items.at( ii );
      QString label = item->text( 0 );
      DataDesc ddesc;

      if ( item->parent() != NULL )
      {  // Not top-level
         label      = item->parent()->text( 0 ) + "." + label;
         ddesc      = datamap[ label ];
         tripls << ddesc.tripID;           // Save selected triples
      }

      else
      {  // Top-level
         ntops++;                          // Count top-level items
         label      = label + "." + item->child( 0 )->text( 0 );
         ddesc      = datamap[ label ];
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
qDebug() << "ntops" << ntops << "trips" << tripls.count()
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
         QString label  = runID + "." + item->child( ii )->text( 0 );
         sdescs << datamap[ label ];
      }
   }

   // Load the actual data from DB or Disk
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

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

   if ( dkdb_cntrls->db() )
   {  // Scan database for AUC
      naucf = scan_db();
   }

   else
   {  // Scan local files for AUC
      naucf = scan_disk();
   }

   if ( naucf == 0 )
   {
      QMessageBox::information( this,
         tr( "No AUC Files" ),
         tr( "No AUC files were found." ) );
      
      return;
   }

   QStringList       runIDs;
   QTreeWidgetItem*  top = NULL;
   QList< DataDesc > ddescs = datamap.values();
//qDebug() << "PopTr: naucf valsize" << naucf << ddescs.size();

   // Populate the tree
   for ( int ii = 0; ii < naucf; ii++ )
   {
      DataDesc ddesc  = ddescs.at( ii );
      QString  runID  = ddesc.runID;
      QStringList item   ( ddesc.tripID );
      QStringList topItem( ddesc.runID );
//qDebug() << "PopTr:   run trip ii" << ddesc.runID << ddesc.tripID << ii;

      if ( ! runIDs.contains( runID ) )
      {  // First entry for a run, so preceed with top-level item
         top  = new QTreeWidgetItem( topItem );
         tree->addTopLevelItem( top );
         runIDs << runID;
      }

      new QTreeWidgetItem( top, item );     // Create 2nd-level tree items
   }

   tree->expandAll();
   tree->resizeColumnToContents( 0 );
   tree->collapseAll();
}

// Limit displayed tree data based on the search text
void US_LoadAUC::limit_data( const QString& search )
{
   bool have_search = ! search.isEmpty();

   for ( int i = 0; i < tree->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* item  = tree->topLevelItem( i );
      QString          label = item->text( 0 );

      // Hide the item if search text exists and the label does not contain it
      item->setHidden( have_search  &&
                       ! label.contains( search, Qt::CaseInsensitive ) );
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
   QString  tripID = toplev ? citem->text( 0 ) : item ->text( 0 );
   QString  label  = runID + "." + tripID;
   DataDesc ddesc  = datamap[ label ];
   QString  dtext  = tr( "Data Information for " );
   dtext          += toplev ? tr( "the first triple of run %1" ).arg( runID )
                            : tr( "item %1" ).arg( label );
   dtext           = dtext + "\n"
      + tr( "\n  Label:                  " ) + label
      + tr( "\n  Database ID:            " ) + QString::number( ddesc.DB_id )
      + tr( "\n  AUC Global ID:          " ) + ddesc.rawGUID
      + tr( "\n  Filename:               " ) + ddesc.filename
      + tr( "\n  Last Updated:           " ) + ddesc.date
      + tr( "\n  Run ID:                 " ) + runID
      + tr( "\n  Triple ID:              " ) + tripID
      + tr( "\n  Triple Index:           " ) + QString::number( ddesc.tripndx )
      + tr( "\n  Triples per Run:        " ) + QString::number( ddesc.tripknt )
      + "";

   // Open a dialog and display run/triple information
   US_Editor* eddiag = new US_Editor( US_Editor::LOAD, true, "", this );
   eddiag->setWindowTitle( tr( "Data Information" ) );
   eddiag->move( this->pos() + pos + QPoint( 100, 100 ) );
   eddiag->resize( 720, 240 );
   eddiag->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   eddiag->e->setText( dtext );
   eddiag->show();
}

// Scan for AUC entries in the database
int US_LoadAUC::scan_db()
{
   int         naucf = 0;
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

   QStringList q( "all_rawDataIDs" );
   q << QString::number( personID );
   db.query( q );

   while ( db.next() )
   {  // Accumulate data description objects from database information
      QString rawDataID = db.value( 0 ).toString();
      QString filename  = db.value( 2 ).toString();
      QString date      = db.value( 5 ).toString() + " UTC";
      QString rawGUID   = db.value( 9 ).toString();
      QString runID     = filename.section( ".",  0, -6 );
      QString tripID    = filename.section( ".", -4, -2 );
      QString label     = runID + "." + tripID;
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
   }

   // Create the data descriptions map
   create_descs( runIDs, infoDs, naucf );

   return naucf;
}

// Scan for AUC files on local disk
int US_LoadAUC::scan_disk()
{
   int         naucf    = 0;
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

      for ( int jj = 0; jj < aucfiles.size(); jj++ )
      {
         QString     aucfbase = aucfiles.at( jj );
         QString     aucfname = subdir + "/" + aucfbase;
         QString     runID    = aucfbase.section( ".",  0, -6 );
         QString     tripID   = aucfbase.section( ".", -4, -2 );
         QString     label    = runID + "." + tripID;
//qDebug() << "ScDk: ii aucfbase" << ii << aucfbase << "label" << label;
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

            if ( xml.isStartElement()  &&  xml.name() == "dataset" )
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
                  break;
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
      ddesc.DB_id       = rawDataID.toInt();
      ddesc.tripknt     = runIDs.count( runID );
      ddesc.tripndx     = tripndx;

//qDebug() << "CrDe: ii tknt" << ii << ddesc.tripknt << "label" << label;
      datamap[ label ]  = ddesc;
   }
   return;
}

// Load the data from the database
void US_LoadAUC::load_db( QList< DataDesc >& sdescs )
{
   QStringList tempfiles;

   int     nerr  = 0;
   QString emsg;
   QString rdir  = US_Settings::resultDir();
   QString tdir  = US_Settings::tmpDir();
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
      US_DataIO2::RawData rdata;
      DataDesc ddesc     = sdescs.at( ii );
      int      idRaw     = ddesc.DB_id;
      QString  filebase  = ddesc.filename;
      QString  filename  = workingDir + "/" + filebase;
      QString  tempname  = tdir       + "/" + filebase;
      QString  triple    = ddesc.tripID.replace( ".", " / " );

      // Compare the date-time stamps of DB and Local Disk records.
      // If the local is newer than DB, download to a TMP directory.
      // Otherwise, replace the local file with the dowloaded DB record.
      QString  ddate     = ddesc.date;
      QString  fdate     = US_Util::toUTCDatetimeText(
                           QFileInfo( filename ).lastModified().toUTC()
                           .toString( Qt::ISODate ), true );
qDebug() << "LdDB: ii" << ii << "ddate" << ddate << "fdate" << fdate;
qDebug() << "LdDB:   (ddate<fdate)" << (ddate<fdate);

      if ( ddate < fdate )
      {  // Download to TMP directory and record for later deletion
         filename           = tempname;
         tempfiles << filename;
qDebug() << "LdDB:     filename==tempname" << filename;
      }

      emit progress( tr( "Loading triple " ) + triple );
      qApp->processEvents();

      // Download the DB record to a file
      db.readBlobFromDB( filename, "download_aucData", idRaw );
      int stat           = db.lastErrno();

      if ( stat != US_DB2::OK )
      {
         nerr++;
         emsg += tr( "Error (%1) downloading to file %2\n" )
                 .arg( stat ).arg( filebase );
      }

      // Read the raw record to memory
      stat = US_DataIO2::readRawData( filename, rdata );

      if ( stat != US_DataIO2::OK )
      {
         nerr++;
         emsg += tr( "Error (%1) reading file %2\n" )
                 .arg( stat ).arg( filebase );
      }

      // Accumulate lists of data and triples
      rawList << rdata;
      triples << triple;
   }

   // Delete any temporary files that we have created
   for ( int ii = 0; ii < tempfiles.count(); ii++ )
   {
      QFile::remove( tempfiles.at( ii ) );
      qDebug() << "Removed:" << tempfiles.at( ii );
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
      US_DataIO2::RawData rdata;
      DataDesc ddesc     = sdescs.at( ii );
      QString  filebase  = ddesc.filename;
      QString  filename  = workingDir + "/" + filebase;
      QString  triple    = ddesc.tripID.replace( ".", " / " );

      emit progress( tr( "Loading triple " ) + triple );
      qApp->processEvents();

      // Read the raw data record from the file
      int stat = US_DataIO2::readRawData( filename, rdata );

      if ( stat != US_DataIO2::OK )
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

