//! \file us_data_loader.cpp

#include "us_data_loader.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_dataIO.h"
#include "us_util.h"
#include "us_editor.h"
#include "us_constants.h"

// Main constructor with flags for edit, latest-edit and local-data

US_DataLoader::US_DataLoader(
      bool                              late,
      int                               local,
      QVector< US_DataIO::RawData    >& rData,
      QVector< US_DataIO::EditedData >& eData,
      QStringList&                      trips,
      QString&                          desc,
      QString                           tfilt )
 : US_WidgetsDialog( 0, 0 ),
   latest     ( late ),
   rawData    ( rData ),
   editedData ( eData ),
   triples    ( trips ),
   description( desc ),
   etype_filt ( tfilt )
{
   setAttribute  ( Qt::WA_DeleteOnClose );
   setWindowTitle( tr( "Load Edited Data" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 320, 300 );

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top    = new QGridLayout;
   int row             = 0;

   // Disk/ DB
   disk_controls       = new US_Disk_DB_Controls( local );
   connect( disk_controls, SIGNAL( changed     ( bool ) ),
                           SLOT( update_disk_db( bool ) ) );
   top->addLayout( disk_controls, row++, 0, 1, 2 );

   // Investigator
   // Only enable the investigator button for privileged users
   pb_invest           = us_pushbutton( tr( "Select Investigator" ) );
   int invlev          = US_Settings::us_inv_level();
   pb_invest->setEnabled( ( invlev > 2 )  && disk_controls->db() );
   connect( pb_invest, SIGNAL( clicked() ), SLOT( get_person() ) );
   top->addWidget( pb_invest, row, 0 );

   QString inv_name    = ( ( invlev > 0 )
                         ? QString::number( US_Settings::us_inv_ID() ) + ": "
                         : "" ) + US_Settings::us_inv_name();

   le_invest           = us_lineedit( inv_name );
   us_setReadOnly( le_invest, true );
   top->addWidget( le_invest, row++, 1 );

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );
   top->addWidget( lb_filtdata, row, 0 );

   le_dfilter          = us_lineedit();
   top->addWidget( le_dfilter, row++, 1 );

   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   main->addLayout( top );

   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );

   // Tree widget to show data choices
   tw_data = new QTreeWidget( this );
   tw_data->setFrameStyle   ( QFrame::NoFrame );
   tw_data->setPalette      ( US_GuiSettings::editColor() );
   tw_data->setFont         ( tw_font );
   tw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );
   tw_data->installEventFilter( this );
   main->addWidget( tw_data );

   // Notes
   te_notes             = new QTextEdit();
   te_notes->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   te_notes->setTextColor( Qt::blue );
   te_notes->setText( tr( "Right-mouse-button-click on a list selection"
                          " for details." ) );
   int font_ht          = QFontMetrics( tw_font ).lineSpacing();
   te_notes->setMaximumHeight( font_ht * 2 + 12 );
   main->addWidget( te_notes );

   // Button Row
   QHBoxLayout* buttons   = new QHBoxLayout;
   QPushButton* pb_help   = us_pushbutton( tr( "Help"       ) );
   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel"     ) );
   QPushButton* pb_shedit = us_pushbutton( tr( "Show Edits" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Load"       ) );
   buttons->addWidget( pb_help );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_shedit );
   buttons->addWidget( pb_accept );
   connect( pb_help,   SIGNAL( clicked() ), SLOT( help()      ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   connect( pb_shedit, SIGNAL( clicked() ), SLOT( selected()  ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted()  ) );

   main->addLayout( buttons );

   // List from disk or db source
   sel_run    = false;
   etype_filt = etype_filt.isEmpty() ? "velocity" : etype_filt.toLower();

   list_data();                // Populate an initial (runs) list

   resize( 720, 500 );
}

// Re-display the list after a change in the search field
void US_DataLoader::search( const QString& search_string )
{
   tw_data->setCurrentItem( NULL );

   for ( int i = 0; i < tw_data->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* twi = tw_data->topLevelItem( i );
      bool hide = ! twi->text( 0 ).contains( search_string,
            Qt::CaseInsensitive ); 
      twi->setHidden( hide );
   }
}

// Load edit data list (along with raw) from selected set
bool US_DataLoader::load_edit( void )
{
   editedData.clear();
   rawData   .clear();
   triples   .clear();

   QList< QTreeWidgetItem* > selections = tw_data->selectedItems();

   // Disallow selections from multiple RunIDs
   QTreeWidgetItem*          twi;
   QList< QTreeWidgetItem* > topLevel;
   
   foreach ( twi, selections )
   {
      while ( twi->parent() != NULL ) twi = twi->parent();
      if ( ! topLevel.contains( twi ) )
         topLevel << twi;

      if ( ! sel_run )
         break;
   }

   if ( topLevel.size() > 1 )
   { 
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "Only items from one run may be selected." ) );
      return false;
   }

   // If we are here after a whole-run load at stage 1 (top-level only),
   //  then we must first fill out the data map for the selected run.
   if ( ! sel_run )
   {
//qDebug() << "LdEd: sel_run=false : scan edits";
      QTreeWidgetItem* twi  = selections[ 0 ];

      while ( twi->parent() != NULL )
         twi          = twi->parent();

      runID_sel    = twi->text( 0 );     // Get the selected run ID
      sel_run      = true;               // Mark run selected
      datamap.clear();
      dlabels.clear();

      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
      te_notes->setText( tr( "Gathering information on triples and edits"
                             " for run ID\n\"%1\"..." ).arg( runID_sel ) );
      qApp->processEvents();
      qApp->processEvents();
//qDebug() << "LdEd: gath info... message noted";

      if ( disk_controls->db() )         // Scan database data
      {
         scan_dbase_edit();
      }
      else                               // Scan local disk data
      {
         scan_local_edit();
      }
//qDebug() << "LdEd: scan edit complete";

      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
      qApp->processEvents();
      dlabels      = datamap.keys();     // Get keys for new mappings
      sel_run      = false;              // Mark full edits data tree
   }

   // Get a list of data to load
   int          index;
   QList< int > indexes;

   if ( selections.size() > 1  ||  selections[ 0 ]->parent() != NULL )
   {  // Multiple selections, so test that they are valid
      foreach ( twi, selections )
      {
         index = twi->type();
         if ( ! indexes.contains( index ) ) indexes << index;
      }

      // Disallow multiple EditIDs from the same triple
      foreach ( index, indexes )
      {
         QString key    = dlabels[ index ];
         QString triple = QString( datamap[ key ].tripID )
                          .replace( ".", " / " );

         if ( triples.contains( triple ) )
         {
            QMessageBox::warning( this,
                  tr( "Invalid Selection" ),
                  tr( "Only one edit from each triple may be selected." ) );
            return false;
         }
      }
   }

   // If we have only a top level item, add the first edit item from
   // each triple

   if ( selections.size() == 1  &&  selections[ 0 ]->parent() == NULL )
   {
qDebug() << "LdEd: Only 1 top-level item  dlabsize" << dlabels.size();
      indexes.clear();

      if ( sel_run )
      {
         twi = selections[ 0 ];

         for ( int i = 0; i < twi->childCount(); i++ )
         {
            QTreeWidgetItem* child = twi->child( i );
            if ( child == NULL ) continue;

            // Get type (label index) of grandchild
            indexes << child->child( 0 )->type();
         }
      }

      else
      {
         for ( int ii = 0; ii < dlabels.size(); ii++ )
            indexes << ii;
      }
   }
else
qDebug() << "LdEd: selsz" << selections.size() << "dlabsz" << dlabels.size();

   if ( indexes.size() < 1 )
   { 
      twi = selections[ 0 ];
      QMessageBox::warning( this,
            tr( "No Edits Available" ),
            tr( "No Edit children exist for selected run"
                " '%1'. The run will be hidden." ).arg( runID_sel ) );
      twi->setHidden( true );
      return false;
   }

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   te_notes->setText( tr( "Loading data from " ) +
                      ( disk_controls->db() ? "Database" : "Local Disk" ) +
                      " ..." );
   qApp->processEvents();

   if ( ! disk_controls->db() ) // Load files from local disk
   { 
      for ( int ii = 0; ii < indexes.size(); ii++ )
      {
         QString  key      = dlabels[ indexes[ ii ] ];
         ddesc             = datamap[ key ];
         QString  filename = ddesc.filename;
         QString  dtriple  = ddesc.tripID;
         QString  triple   = QString( dtriple ).replace( ".", " / " );

         triples << triple;

         QString  clambda  = dtriple .section( ".", -1, -1 );
         QString  filedir  = filename.section( "/",  0, -2 );
         filename          = filename.section( "/", -1, -1 );
         QString  ftriple  = filename.section( ".", -4, -2 );
         QString  message  = tr( "Loading triple " ) + triple;

         if ( triple != ftriple   &&  ftriple.contains( "-" ) )
         {  // Modify filename to signal MWL
            QString  ftrnode  = "." + ftriple + ".";
            QString  utrnode  = "." + ftriple  + "@" + clambda + ".";
            filename          = filename.replace( ftrnode, utrnode );
         }

         emit progress( message );
         qApp->processEvents();

         try
         {
            US_DataIO::loadData( filedir, filename, editedData, rawData );
         }
         catch ( US_DataIO::ioError error )
         {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning( this,
                  tr( "Data Error" ),
                  US_DataIO::errorString( error ) );

            return false;
         }
         catch ( int err )
         {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning( this,
                  tr( "Data Error" ),
                  US_DataIO::errorString( err ) );

            return false;
         }
      }
   }

   else                         // Load data from database
   { 
      US_Passwd   pw;
      US_DB2      db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QApplication::restoreOverrideCursor();
         QMessageBox::information( this,
            tr( "DB Connection Problem" ),
            tr( "There was an error connecting to the database:\n" )
            + db.lastError() );

         return false;
      }

      QStringList query;
      QString  prvfname = "";
      QString  efn      = "";
      bool     dnld_auc = true;
      bool     dnld_edt = true;

      // Read first selection from DB, then generate a map of AUCfile::idAUC
      ddesc             = datamap[ dlabels[ indexes[ 0 ] ] ];
qDebug() << "LdEd: ndx0" << indexes[0] << "dlab0" << dlabels[indexes[0]];
      QString  recID    = QString::number( ddesc.DB_id );
      QString  runID    = ddesc.runID;
      QString  rdir     = US_Settings::resultDir();
      QString  uresdir  = rdir + "/" + runID + "/";
      QDir( rdir ).mkpath( runID );
      QString  message  = tr( "Browsing AUC data..." );
      emit progress( message );
      qApp->processEvents();
qDebug() << "LdEd:TM:10: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

      for ( int ii = 0; ii < indexes.size(); ii++ )
      {  // Read in each of the edits that was selected
         QString  key      = dlabels[ indexes[ ii ] ];
         ddesc             = datamap[ key ];
         int      idRec    = ddesc.DB_id;
         QString  dtriple  = ddesc.tripID;
         QString  triple   = QString( dtriple ).replace( ".", " / " );
qDebug() << "LdEd:  ii" << ii << "ndxii" << indexes[ii] << "triple" << triple;

         triples << triple;

         QString  dscfname = ddesc.filename;
         QString  filename = dscfname;
         QString  clambda  = dtriple .section( ".", -1, -1 );
         QString  filedir  = filename.section( "/",  0, -2 );
         filename          = filename.section( "/", -1, -1 );
         QString  ftriple  = filename.section( ".", -4, -2 );

         if ( triple != ftriple   &&  ftriple.contains( "-" ) )
         {  // Modify filename to signal MWL
            QString  ftrnode  = "." + ftriple + ".";
            QString  utrnode  = "." + ftriple  + "@" + clambda + ".";
            filename          = filename.replace( ftrnode, utrnode );
         }

         QString  recID    = QString::number( idRec );
         QString  invID    = QString::number( US_Settings::us_inv_ID() );
         QString  aucfn    = ddesc.runID + "."
                             + filename.section( ".", -5, -3 )
                             + "." + clambda + ".auc";
         QString  afn      = uresdir + aucfn;
         int      idAUC    = ddesc.auc_id;
         efn               = uresdir + dscfname;
         dnld_auc          = true;

         QString message   = tr( "Loading triple " ) + triple;
         emit progress( message );
         qApp->processEvents();

         if ( QFile( afn ).exists() )
         {  // AUC file exists, so only download if checksum mismatch
            QString  fcheck   = US_Util::md5sum_file( afn );

            if ( ddesc.acheck.isEmpty() )
            {  // No database checksum+size, so get it
               QString aucID    = QString::number( idAUC );
qDebug() << "LdEd: was-empty, aucID" << aucID << idAUC;
               query.clear();
               query << "get_rawData" << aucID;
               db.query( query );
qDebug() << "LdEd: w-e, num_rows" << db.numRows();
               db.next();
qDebug() << "LdEd: w-e, dberr" << db.lastErrno() << US_DB2::OK;
               ddesc.acheck     = db.value( 8 ).toString() + " " +
                                  db.value( 9 ).toString();
qDebug() << "LdEd: was-empty, now acheck" << ddesc.acheck;
            }
qDebug() << "LdEd: fcheck" << fcheck;
qDebug() << "LdEd: acheck" << ddesc.acheck;

            dnld_auc          = ( fcheck != ddesc.acheck );
         }
qDebug() << "LdEd: dnld_auc" << dnld_auc << "afn" << afn;

         if ( dnld_auc )
         {
            db.readBlobFromDB( afn, "download_aucData", idAUC );
qDebug() << "LdEd:  dnld_auc DONE" << dscfname << prvfname;
         }

         if ( dscfname != prvfname )
         {
            efn               = uresdir + dscfname;
            prvfname          = dscfname;
            dnld_edt          = true;

            if ( QFile( efn ).exists() )
            {  // Edit XML file exists, so only download if checksum mismatch
               QString  fcheck   = US_Util::md5sum_file( efn );
               dnld_edt          = ( fcheck != ddesc.echeck );
            }

qDebug() << "LdEd: dnld_edt" << dnld_edt << "efn" << efn;
            if ( dnld_edt )
               db.readBlobFromDB( efn, "download_editData", idRec );
qDebug() << "LdEd: dnld_edt DONE";
         }

         qApp->processEvents();

	 qDebug() << "In LOAD auc/edited data in fematch: uresdir, filename, afn, efn: " << uresdir << filename << afn <<  efn;
	 qDebug() << "In LOAD auc/edited data in fematch: idRec(editID), idAUC(rawData): " << idRec << idAUC;

qDebug() << "LdEd: loadData uresdir filename" << uresdir << filename;
         US_DataIO::loadData( uresdir, filename, editedData, rawData );
qDebug() << "LdEd: loadData DONE";
      }  // END: edits loop
qDebug() << "LdEd:TM:11: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

 
 
   }  // END: Load from DB

   QApplication::restoreOverrideCursor();
   double dt    = 0.0;
   US_DataIO::EditedData ed;

   foreach( ed, editedData )
   {
      double delta = ed.temperature_spread();
      dt           = ( dt < delta ) ? delta : dt;
   }

   if ( dt > US_Settings::tempTolerance() )
   {
      QMessageBox::warning( this,
            tr( "Temperature Problem" ),
            tr( "The temperature in this run varied over the course\n"
                "of the run to a larger extent than allowed by the\n"
                "current threshold (" )
                + QString::number( US_Settings::tempTolerance(), 'f', 1 )
                + " " + DEGC + tr( ". The accuracy of experimental\n"
                "results may be affected significantly." ) );
   }

   return true;
}

// Return description string of data last selected
void US_DataLoader::describe( )
{
   QString label    = ddesc.label;
   QString descript = ddesc.descript;
   QString dbID     = QString::number( ddesc.DB_id );
   QString filename = ddesc.filename;
   QString dataGUID = ddesc.dataGUID;
   QString aucGUID  = ddesc.aucGUID;
   QString aucID    = QString::number( ddesc.auc_id );
   QString cdesc    = label + descript + filename + dataGUID + aucGUID + dbID;
   QString sep      = ";";     // Use semi-colon as separator

   if ( cdesc.contains( sep ) )
      sep           = "^";  // Use carat if semi-colon already in use

   // Create and return a composite description string
   description      = sep + label
                    + sep + descript
                    + sep + dbID
                    + sep + filename
                    + sep + dataGUID
                    + sep + aucID
                    + sep + aucGUID;
}

// Investigator button clicked:  get investigator from dialog
void US_DataLoader::get_person()
{
   int              invID  = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect(
      dialog,
      SIGNAL( investigator_accepted( int ) ),
      SLOT(   update_person(         int ) ));

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_DataLoader::update_person( int ID )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_invest->setText( number + US_Settings::us_inv_name() );
   list_data();
}

// List data choices (from db or disk)
void US_DataLoader::list_data()
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   datamap.clear();

   if ( sel_run )
   {  // A run has been selected, so scan for edits
      if ( disk_controls->db() ) // Scan database data
      {
         scan_dbase_edit();
      }
      else                       // Scan local disk data
      {
         scan_local_edit();
      }

      te_notes->setText( tr( "Right-mouse-button-click on a list selection"
                             " for details.\n"
                             "Select triples/edits and click on \"Load\""
                             " to load selected data." ) );
   }
   else
   {  // No run is selected, so scan for runs
      if ( disk_controls->db() ) // Scan database data
      {
         scan_dbase_runs();
      }
      else                       // Scan local disk data
      {
         scan_local_runs();
      }

      te_notes->setText( tr( "Select a run, then click on \"Show Edits\""
                             " to fill in triples and edits;\n"
                             "or click on \"Load\" to load all the latest"
                             " edits of triples for the selected run." ) );
   }

   // Start building the data tree
   qApp->processEvents();
   QStringList crlabels;
   QStringList hdrs;
   hdrs << ( sel_run ? tr( "Run|Triple|Edit" ) : tr( "Run" ) )
        << tr( "Date" )
        << tr( "DbID" )
        << tr( "Label" );
   tw_data->clear();
   tw_data->setColumnCount( 4 );
   tw_data->setHeaderLabels( hdrs );
   tw_data->setSortingEnabled( false );
   
   QTreeWidgetItem* twi_edit = NULL;
   QTreeWidgetItem* twi_runi = NULL;
   QTreeWidgetItem* twi_trip = NULL;

   dlabels                   = datamap.keys();
   QList< DataDesc > ddescrs = datamap.values();
   QString           prlabel = "";
   QString           ptlabel = "";
   int               ndxt    = 1;
   int               ndxe    = 1;
   bool              fromDB  = disk_controls->db();

   if ( dlabels.size() == 0 )
   {
      QString clabel = tr( "No data found." );
      twi_runi = new QTreeWidgetItem( QStringList( clabel), 0 );
      tw_data->addTopLevelItem( twi_runi );
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
      return;
   }

   for ( int ii = 0; ii < dlabels.size(); ii++ )
   {  // Propagate list widget with labels
      QString  cdescr  = dlabels.at( ii );
      DataDesc ddesc   = ddescrs.at( ii );
      QString  dbID    = fromDB
                         ? QString().sprintf( "%6d", ddesc.DB_id )
                         : QString( "" );
      crlabels.clear();
      crlabels << ddesc.runID
               << QString( ddesc.date ).section( " ", 0, 0 ).simplified()
               << dbID
               << ddesc.elabel;
      QString  crlabel = ddesc.runID;
      QString  ctlabel = ddesc.tripID;
      QString  celabel = ddesc.editID;

      if ( crlabel != prlabel )
      {  // New runID:  add top-level item
         twi_runi = new QTreeWidgetItem( crlabels, ii );
         tw_data->addTopLevelItem( twi_runi );

         if ( sel_run ) 
         {  // If in show-edits mode, add a triple child
            twi_trip = new QTreeWidgetItem( QStringList( ctlabel ), ii );
            twi_runi->addChild( twi_trip );
         }
         
         prlabel  = crlabel;
         ptlabel  = ctlabel;
         ndxt     = 1;
         ndxe     = 1;
      }
      else if ( ctlabel != ptlabel )
      {  // New triple in same runID: add triple child of run
         twi_trip = new QTreeWidgetItem( QStringList( ctlabel ), ii );
         twi_runi->addChild( twi_trip );
         ptlabel  = ctlabel;
         ndxt++;
         ndxe = 1;
      }
      else
      {  // Same triple as before
         ndxe++;
      }

      // Always add an edit child of triple (if in show-edits mode)
      if ( sel_run ) 
      {
         twi_edit = new QTreeWidgetItem( QStringList( celabel ), ii );
         twi_trip->addChild( twi_edit );
      }

      ddesc.tripknt     = ndxt;
      ddesc.tripndx     = ndxt;
      ddesc.editknt     = ndxe;
      ddesc.editndx     = ndxe;
      datamap[ cdescr ] = ddesc;
   }

   tw_data->expandAll();
   tw_data->resizeColumnToContents( 0 );
   tw_data->resizeColumnToContents( 1 );
   tw_data->resizeColumnToContents( 2 );
   tw_data->collapseAll();
   tw_data->setSortingEnabled( true );
   tw_data->sortByColumn( 0, Qt::AscendingOrder );  // Insure triples in order
   tw_data->sortByColumn( 1, Qt::DescendingOrder ); // Default latest-top order

   // Walk through entries backwards to propagate edit,triple counts
   prlabel = "";
   ptlabel = "";
   ndxt    = 1;
   ndxe    = 1;
   ddescrs = datamap.values();

   for ( int ii = dlabels.size() - 1; ii >= 0; ii-- )
   {
      bool     update  = false;
      QString  cdescr  = dlabels.at( ii );
      DataDesc ddesc   = ddescrs.at( ii );
      QString  crlabel = ddesc.runID;
      QString  ctlabel = ddesc.tripID;

      if ( crlabel != prlabel )
      {  // New run:   get triple and edit count
         ndxt          = ddesc.tripknt;
         ndxe          = ddesc.editknt;
         prlabel       = crlabel;
         ptlabel       = "";
      }

      else
      {  // Same run:  update triple count
         ddesc.tripknt = ndxt;
         update        = true;
      }

      if ( ctlabel != ptlabel )
      {  // New triple:  get edit count
         ndxe          = ddesc.editknt;
         ptlabel       = ctlabel;
      }

      else
      {  // Same triple:  update edit count
         ddesc.editknt = ndxe;
         update        = true;
      }

      if ( update )
      {  // One or other count has been updated
         datamap[ cdescr ] = ddesc;
      }
   }
   QApplication::restoreOverrideCursor();
}

// Cancel button:  no models returned
void US_DataLoader::cancelled()
{
   reject();
   close();
}

// Select button:  fill in list tree for selected run
void US_DataLoader::selected()
{
   QList< QTreeWidgetItem* > selitems = tw_data->selectedItems();

   if ( selitems.size() < 1 )
   {  // "Show Edits" with no run selected:  build full data tree
      runID_sel    = "";
      te_notes->setText( tr( "Reading edit information to fully populate"
                             " the list data tree..." ) );
   }

   else
   {  // "Show Edits" with run selected:  build a data tree for the selected run
      QTreeWidgetItem* twi  = selitems[ 0 ];

      while ( twi->parent() != NULL )
         twi          = twi->parent();

      runID_sel    = twi->text( 0 );
      te_notes->setText( tr( "Reading edit information for run \"%1\",\n"
                             " to populate its list data tree..." )
                         .arg( runID_sel ) );
   }

   qApp->processEvents();
   sel_run      = true;

   list_data();
}

// Accept button:  set up to return data information
void US_DataLoader::accepted()
{
   QList< QTreeWidgetItem* > selitems = tw_data->selectedItems();
   QList< DataDesc >         ddescrs  = datamap.values();

   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect+Load or Cancel" ) );
      return;
   }

   if ( load_edit() )
   {
      describe();
      accept();        // Signal that selection was accepted
      close();
   }
}

// Scan database for edit sets
void US_DataLoader::scan_dbase_edit()
{
qDebug() << "ScDB:TM:00: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   bool     tfilter  = ( etype_filt != "none" );     // Type filtering?
   bool     rfilter  = ( ! runID_sel.isEmpty() );    // Run filtering?
   QString  tempdir  = US_Settings::tmpDir() + "/";
   QDir dir;
   if ( ! dir.exists( tempdir ) )
      dir.mkpath( tempdir );
   QStringList query;
   QStringList edtIDs;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );

   setWindowTitle( tr( "Load Edited Data from DB" ) );

   // Accumulate a map of AUC filenames and IDs
   QMap< QString, QString > aucIDs;
qDebug() << "ScDB:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   query.clear();

   if ( rfilter )
     {
       query << "get_raw_desc_by_runID" << invID << runID_sel;
       qDebug() << "Run Fitering true";
     }
   else
     {
       query << "get_rawData_desc" << invID;
       qDebug() << "Run Fitering false";
     }
   db.query( query );
qDebug() << "ScDB:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   while( db.next() )
   {  // Accumulate a mapping of AUC Filename to DB ID
      QString rLabel    = db.value( 1 ).toString();
      QString aFname    = db.value( 2 ).toString();
      QString aucID     = db.value( 0 ).toString();
      QString expID     = db.value( 3 ).toString();
      QString aucGUID   = db.value( 7 ).toString();
      aucIDs[ aFname ]  = aucID + "^" + aucGUID + "^" + expID + "^" + rLabel;
//qDebug() << "ScDB:  aucIDs" << aucIDs[aFname] << "aF" << aFname;
   }

qDebug() << "ScDB:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   QStringList editpars;
   query.clear();

   if ( rfilter )
     {
       query << "get_edit_desc_by_runID" << invID << runID_sel;
     }
   else
     query << "all_editedDataIDs" << invID;

   qDebug() << "Query for getting edits: " << query;
   db.query( query );
qDebug() << "ScDB:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
qDebug() << "ScDB: tfilter etype_filt" << tfilter << etype_filt;

   // Edit record parameters from the DB are first accumulated,
   //  since we may need to download the content blob for some entries (MWL)
   while ( db.next() )
   {  // Accumulate edit record parameters from DB
      QString etype    = db.value( 8 ).toString().toLower();

      qDebug() << "etype -- " << etype;

      if ( tfilter  &&  etype != etype_filt )                        //ALEXEY < --- HERE, etype != etype_filt (etype_filt == "velocity")
         continue;

      QString recID    = db.value( 0 ).toString();
      QString descrip  = db.value( 1 ).toString();
      QString filename = db.value( 2 ).toString().replace( "\\", "/" );
      QString filebase = filename.section( "/", -1, -1 );
      QString runID    = descrip.isEmpty() ? filebase.section( ".", 0, -7 )
                         : descrip;
      QString parID    = db.value( 3 ).toString();
      QString date     = US_Util::toUTCDatetimeText( db.value( 5 )
                         .toDateTime().toString( Qt::ISODate ), true );
      QString cksum    = db.value( 6 ).toString();
      QString recsize  = db.value( 7 ).toString();
      QString recGUID  = db.value( 9 ).toString();

      edtIDs << recID;
      editpars << descrip;
      editpars << filename;
      editpars << parID;
      editpars << date;
      editpars << recGUID;
      editpars << cksum + " " + recsize;
   }
qDebug() << "ScDB:TM:05: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   // Now loop through the list of edit entries, building description records
   int nedit = edtIDs.size();
qDebug() << "ScDB: nedit" << nedit;
   int kp    = 0;
   for ( int ii = 0; ii < nedit; ii++ )
   {
      QString recID    = edtIDs[ ii ];
      int     idRec    = recID.toInt();
//qDebug() << "ScDB:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      QString descrip  = editpars[ kp++ ];
      QString filename = editpars[ kp++ ];
      QString parID    = editpars[ kp++ ];
      QString date     = editpars[ kp++ ];
      QString recGUID  = editpars[ kp++ ];
      QString echeck   = editpars[ kp++ ];

      QString filebase = filename.section( "/", -1, -1 );
      QString runID    = descrip.isEmpty() ? filebase.section( ".", 0, -7 )  // 'DubnauD_ComEA-high_121021-run1237'
                         : descrip;
      QString editID   = filebase.section( ".", -6, -6 );  // '2201051846-midscans'
      QString dataType = filebase.section( ".", -5, -5 );
      QString tripID   = filebase.section( ".", -4, -2 );
      QString edtlamb  = tripID  .section( ".",  2,  2 );
      QString aucfname = runID + "." + dataType + "." + tripID + ".auc";
      bool    isMwl    = edtlamb.contains( "-" );
      int     idAUC    = aucIDs[ aucfname ].section( "^", 0, 0 ).toInt();
      QString aucGUID  = aucIDs[ aucfname ].section( "^", 1, 1 );
      int     idExp    = aucIDs[ aucfname ].section( "^", 2, 2 ).toInt();
      QString elabel   = aucIDs[ aucfname ].section( "^", 3, 3 );
      QString acheck   = aucIDs[ aucfname ].section( "^", 4, 4 ).simplified();
      if ( isMwl )
      {
         idAUC            = 0;
         idExp            = 0;
      }
//qDebug() << "ScDB:   isMwl idAUC" << isMwl << idAUC << "aucfname" << aucfname;

//qDebug() << "ScDB:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      QString label    = runID;
      descrip          = runID + "." + tripID + "." + editID;
      QString baselabl = label;

      ddesc.runID      = runID;
      ddesc.tripID     = tripID;
      ddesc.editID     = editID;
      ddesc.label      = label;
      ddesc.descript   = descrip;
      ddesc.filename   = filename;
      ddesc.dataGUID   = recGUID;
      ddesc.aucGUID    = aucGUID;
      ddesc.DB_id      = idRec;
      ddesc.elabel     = elabel;
      ddesc.exp_id     = idExp;
      ddesc.auc_id     = idAUC;
      ddesc.date       = date;
      ddesc.acheck     = acheck;
      ddesc.echeck     = echeck;
      ddesc.tripknt    = 1;
      ddesc.tripndx    = 1;
      ddesc.editknt    = 1;
      ddesc.editndx    = 1;
      ddesc.isEdit     = true;
      ddesc.isLatest   = latest;

      if ( isMwl )
      {  // For MWL we must generate multiple entries for a Cell/Channel edit
         QString edtfname = runID + "." + dataType + "." + tripID + ".xml";
         QString edtpath  =  tempdir + "/" + edtfname;

         // Read the XML contents from the edit DB record
         db.readBlobFromDB( edtpath, "download_editData", idRec );
         qApp->processEvents();
         QString elambda;

         // Open the temporary file and parse Lambdas from the XML
         QFile filei( edtpath );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;
         QStringList mwlambds;
         QXmlStreamReader xml( &filei );
         QXmlStreamAttributes attr;
         QString recGUID;
         QString parGUID;
         QString expType;

         while( ! xml.atEnd() )
         {  // Build up a string list of Lambdas
            xml.readNext();

            if ( xml.isStartElement()  &&  xml.name() == "lambda" )
            {
               attr      = xml.attributes();
               elambda   = attr.value( "value" ).toString();
               mwlambds << elambda;
            }

            else if ( xml.isEndElement()  &&  xml.name() == "lambdas" )
               break;
         }

         filei.close();
         filei.remove();

         QString otripID  = tripID;
         QString odescrip = descrip;
         QString aucEntr;
         int     tripknt  = mwlambds.size();

         for ( int ii = 0; ii < tripknt; ii++ )
         {  // Add an entry for each Lambda in the edit
            elambda       = mwlambds[ ii ];
            tripID        = QString( otripID  ).replace( edtlamb, elambda );
            descrip       = QString( odescrip ).replace( otripID, tripID  );
            aucfname      = runID + "." + dataType + "." + tripID + ".auc";
            aucEntr       = aucIDs[ aucfname ];
            idAUC         = aucEntr.section( "^", 0, 0 ).toInt();
            parGUID       = aucEntr.section( "^", 1, 1 );
            idExp         = aucEntr.section( "^", 2, 2 ).toInt();

            ddesc.tripID       = tripID;
            ddesc.editID       = editID + "@" + elambda;
            ddesc.descript     = descrip;
            ddesc.aucGUID      = parGUID;
            ddesc.exp_id       = idExp;
            ddesc.auc_id       = idAUC;
            ddesc.tripknt      = tripknt;
            ddesc.tripndx      = ii + 1;
            datamap[ descrip ] = ddesc;
         }
      } // END: isMwl=true

      else
         datamap[ descrip ] = ddesc;
//qDebug() << "ScDB:TM:06: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
//qDebug() << "ScDB:   descrip" << descrip;
   } // END: loop to read DB edit records
qDebug() << "ScDB:TM:08: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   if ( latest )
      pare_to_latest();
qDebug() << "ScDB:TM:09: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Scan local disk for edit sets
void US_DataLoader::scan_local_edit( void )
{
   setWindowTitle( tr( "Load Edited Data from Local Disk" ) );

   bool    tfilter     = ( etype_filt != "none" );
   bool    rfilter     = ( ! runID_sel.isEmpty() );
   QString rdir        = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   
   QStringList aucfilt( "*.auc" );
   QStringList edtfilt;
   QString elabel;
   QString expID;
   
   for ( int ii = 0; ii < aucdirs.size(); ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList( 
            aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString aucfbase  = aucfiles.at( 0 );
      QString aucfname  = subdir + "/" + aucfbase;
      QString runID     = aucfbase.section( ".",  0, -6 );

      if ( rfilter  &&  runID != runID_sel )
         continue;

      experiment_info( subdir, elabel, expID );

      QString subType   = aucfbase.section( ".", -5, -5 );
      QString tripl     = aucfbase.section( ".", -4, -2 );
      QString auclamb   = tripl   .section( ".",  2,  2 );

      edtfilt.clear();
      edtfilt <<  runID + ".*."  + subType + ".*.xml";
      QStringList edtfiles = QDir( subdir ).entryList( 
            edtfilt, QDir::Files, QDir::Name );
      edtfiles.sort();

      if ( edtfiles.size() < 1 )
         continue;

      for ( int jj = 0; jj < edtfiles.size(); jj++ )
      {
         QStringList mwlambds;
         QString filebase = edtfiles.at( jj );
         QString filename = subdir + "/" + filebase;
         QString runID    = filebase.section( ".",  0, -7 );
         QString editID   = filebase.section( ".", -6, -6 );
         editID  = ( editID.length() == 12  &&  editID.startsWith( "20" ) ) ?
                   editID.mid( 2 ) : editID;
         QString tripID   = filebase.section( ".", -4, -2 );
         QString edtlamb  = tripID  .section( ".",  2,  2 );
         QString label    = runID;
         QString descrip  = runID + "." + tripID + "." + editID;
         QString baselabl = label;

         QFile filei( filename );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QString date = US_Util::toUTCDatetimeText( QFileInfo( filename )
            .lastModified().toUTC().toString( Qt::ISODate ), true );
         
         QXmlStreamReader xml( &filei );
         QXmlStreamAttributes a;
         QString recGUID;
         QString parGUID;
         QString expType;
         QString elambda;
         bool    isMwl   = edtlamb.contains( "-" );

         while( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isStartElement() )
            {
               if ( xml.name() == "editGUID" )
               {
                  a         = xml.attributes();
                  recGUID   = a.value( "value" ).toString();
               }

               else if ( xml.name() == "rawDataGUID" )
               {
                  a         = xml.attributes();
                  parGUID   = a.value( "value" ).toString();
               }

               else if ( xml.name() == "experiment" )
               {
                  a         = xml.attributes();
                  expType   = a.value( "type" ).toString().toLower();
               }

               if ( isMwl   &&  xml.name() == "lambda" )
               {
                  a         = xml.attributes();
                  elambda   = a.value( "value" ).toString();
                  mwlambds << elambda;
//qDebug() << "DLdr:    elambda" << elambda;
               }
            }
         }

         filei.close();

         // If type filtering, ignore runIDs that do not match experiment type
         if ( tfilter  &&  expType != etype_filt )
               continue;

         ddesc.runID      = runID;
         ddesc.tripID     = tripID;
         ddesc.editID     = editID;
         ddesc.label      = label;
         ddesc.descript   = descrip;
         ddesc.filename   = filename;
         ddesc.dataGUID   = recGUID;
         ddesc.aucGUID    = parGUID;
         ddesc.elabel     = elabel;
         ddesc.DB_id      = -1;
         ddesc.exp_id     = expID.toInt();
         ddesc.auc_id     = -1;
         ddesc.date       = date;
         ddesc.acheck     = US_Util::md5sum_file( filename );
         ddesc.tripknt    = 1;
         ddesc.tripndx    = 1;
         ddesc.editknt    = 1;
         ddesc.editndx    = 1;
         ddesc.isEdit     = true;
         ddesc.isLatest   = latest;

         if ( isMwl )
         {
            QString otripID  = tripID;
            QString odescrip = descrip;
            int     tripknt  = mwlambds.size();

            for ( int ii = 0; ii < tripknt; ii++ )
            {
               elambda       = mwlambds[ ii ];
               tripID        = QString( otripID  ).replace( edtlamb, elambda );
               descrip       = QString( odescrip ).replace( otripID, tripID  );
               ddesc.tripID       = tripID;
               ddesc.editID       = editID + "@" + elambda;
               ddesc.descript     = descrip;
               ddesc.tripknt      = tripknt;
               ddesc.tripndx      = ii + 1;
               datamap[ descrip ] = ddesc;
            }
         }

         else
            datamap[ descrip ] = ddesc;
      }
   }

   if ( latest )
      pare_to_latest();
}

// Scan database for runs
void US_DataLoader::scan_dbase_runs()
{
qDebug() << "ScDB:TM:00: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   QStringList query;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );

   setWindowTitle( tr( "Load Run Data from DB" ) );

   // Accumulate a map of runs to dates,IDs,labels
   QMap< QString, QString > runinfo;
qDebug() << "ScDB:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   query.clear();
   query << "get_experiment_desc" << invID;
   db.query( query );
qDebug() << "ScDB:TM:02: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   ddesc.tripID     = "undetermined";    // Dummy edit description settings
   ddesc.editID     = "undetermined";
   ddesc.filename   = "undetermined";
   ddesc.dataGUID   = "undetermined";
   ddesc.aucGUID    = "undetermined";
   ddesc.acheck     = "";
   ddesc.auc_id     = -1;
   ddesc.tripknt    = 1;
   ddesc.tripndx    = 1;
   ddesc.editknt    = 1;
   ddesc.editndx    = 1;
   ddesc.isEdit     = false;
   ddesc.isLatest   = latest;

   while( db.next() )
   {  // Accumulate a mapping of data descriptions to runs
      QString expID    = db.value( 0 ).toString();
      QString runID    = db.value( 1 ).toString();
      QString label    = db.value( 4 ).toString();
      QString date     = US_Util::toUTCDatetimeText( db.value( 5 )
                         .toDateTime().toString( Qt::ISODate ), true );

      ddesc.runID      = runID;
      ddesc.label      = label;
      ddesc.descript   = label;
      ddesc.elabel     = label;
      ddesc.exp_id     = expID.toInt();
      ddesc.DB_id      = ddesc.exp_id;
      ddesc.date       = date;

      datamap[ runID ] = ddesc;
   }

qDebug() << "ScDB:TM:09: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Scan local disk for runs
void US_DataLoader::scan_local_runs( void )
{
   setWindowTitle( tr( "Load Run Data from Local Disk" ) );

   bool        tfilter = ( etype_filt != "none" );
   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );

   QStringList aucfilt( "*.auc" );
   QStringList edtfilt;
   QString elabel;
   QString expID;

   ddesc.tripID     = "undetermined";    // Dummy edit description settings
   ddesc.editID     = "undetermined";
   ddesc.filename   = "undetermined";
   ddesc.dataGUID   = "undetermined";
   ddesc.aucGUID    = "undetermined";
   ddesc.acheck     = "";
   ddesc.DB_id      = -1;
   ddesc.auc_id     = -1;
   ddesc.tripknt    = 1;
   ddesc.tripndx    = 1;
   ddesc.editknt    = 1;
   ddesc.editndx    = 1;
   ddesc.isEdit     = false;
   ddesc.isLatest   = latest;
   
   for ( int ii = 0; ii < aucdirs.size(); ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList( 
            aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      experiment_info( subdir, elabel, expID );

      QString aucfbase  = aucfiles.at( 0 );
      QString aucfname  = subdir + "/" + aucfbase;
      QString runID     = aucfbase.section( ".",  0, -6 );
      QString subType   = aucfbase.section( ".", -5, -5 );
      QString tripl     = aucfbase.section( ".", -4, -2 );
      QString auclamb   = tripl   .section( ".",  2,  2 );

      edtfilt.clear();
      edtfilt <<  runID + ".*."  + subType + ".*.xml";
      QStringList edtfiles = QDir( subdir ).entryList( 
            edtfilt, QDir::Files, QDir::Name );
      edtfiles.sort();

      if ( edtfiles.size() < 1 )
         continue;

      QStringList mwlambds;
      QString filebase = edtfiles.at( 0 );
      QString filename = subdir + "/" + filebase;
      QString editID   = filebase.section( ".", -6, -6 );
      editID  = ( editID.length() == 12  &&  editID.startsWith( "20" ) ) ?
                editID.mid( 2 ) : editID;
      QString tripID   = filebase.section( ".", -4, -2 );
      QString edtlamb  = tripID  .section( ".",  2,  2 );
      QString label    = runID;
      QString descrip  = runID + "." + tripID + "." + editID;
      QString baselabl = label;

      QFile filei( filename );

      if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QString date = US_Util::toUTCDatetimeText( QFileInfo( filename )
         .lastModified().toUTC().toString( Qt::ISODate ), true );
         
      QXmlStreamReader xml( &filei );
      QXmlStreamAttributes a;
      QString recGUID;
      QString parGUID;
      QString expType;
      QString elambda;
      bool    isMwl   = edtlamb.contains( "-" );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "editGUID" )
            {
               a         = xml.attributes();
               recGUID   = a.value( "value" ).toString();
            }

            else if ( xml.name() == "rawDataGUID" )
            {
               a         = xml.attributes();
               parGUID   = a.value( "value" ).toString();
            }

            else if ( xml.name() == "experiment" )
            {
               a         = xml.attributes();
               expType   = a.value( "type" ).toString().toLower();
            }

            if ( isMwl   &&  xml.name() == "lambda" )
            {
               a         = xml.attributes();
               elambda   = a.value( "value" ).toString();
               mwlambds << elambda;
//qDebug() << "DLdr:    elambda" << elambda;
            }
         }
      }

      filei.close();

      // If type filtering, ignore runIDs that do not match experiment type
      if ( tfilter  &&  expType != etype_filt )
            continue;

      ddesc.runID      = runID;
      ddesc.label      = label;
      ddesc.descript   = descrip;
      ddesc.aucGUID    = parGUID;
      ddesc.elabel     = elabel;
      ddesc.exp_id     = expID.toInt();
      ddesc.date       = date;

      datamap[ descrip ] = ddesc;
   }
}

// Pare down data description map to only latest edit
void US_DataLoader::pare_to_latest( void )
{
   int kmwl = 0;

   for ( int kk = 0; kk < 2; kk++ )  // May need two passes to pare down
   {
      QStringList       keys = datamap.keys();
      QList< DataDesc > vals = datamap.values();
      int               kchg = 0;

      for ( int ii = 0; ii < keys.size() - 1; ii++ )
      {
         int jj = ii + 1;

         QString clabel = keys.at( ii );                 
	 QString flabel = keys.at( jj );               
	                                               

         QString crunid = clabel.section( ".", 0, -2 );
         QString frunid = flabel.section( ".", 0, -2 );

         if ( crunid != frunid )
         {  // Likely difference in runid, but check for MWL
            QString cruncc = clabel.section( ".", 0, -3 );
            QString fruncc = flabel.section( ".", 0, -3 );

            if ( cruncc == fruncc )
               kmwl++;                 // Mark possible MWL case

            continue;                                                   
         }

         // This record's label differs from next only by edit code: remove it
         // int       cdetm = vals.at( ii ).editID.left( 10 ).toInt();
         // int       fdetm = vals.at( jj ).editID.left( 10 ).toInt();
	 quint64      cdetm = vals.at( ii ).editID.left( 10 ).toLong();
         quint64      fdetm = vals.at( jj ).editID.left( 10 ).toLong();
	 
	 qDebug() << "PARE 1: vals.at( ii ).editID.left(10), vals.at( jj ).editID.left( 10 ) -- "
		  <<  vals.at( ii ).editID.left( 10 )
		  <<  vals.at( jj ).editID.left( 10 );
	 qDebug() << "PARE 2: vals.at( ii ).editID.left(10).toInt(), vals.at( jj ).editID.left( 10 ).toLong() -- "
		  <<  vals.at( ii ).editID.left( 10 ).toInt()
		  <<  vals.at( jj ).editID.left( 10 ).toLong();
	 
qDebug() << "PARE ii" << ii << "C,F edtm" << cdetm << fdetm;
qDebug() << "  C,F lab" << clabel << flabel;
         if ( cdetm <= fdetm )         // Remove the earlier of the two
         {
            datamap.remove( clabel );  //  Earlier is earlier in list
         }

         else
         {
            datamap.remove( flabel );  //  Earlier is later in list
            kchg++;                    //  Mark when early one later in list
         }
      }

//qDebug() << "PARE   kchg" << kchg << "kk" << kk;
      if ( kchg == 0 )   break;        // We're done

      // Need to repeat above when any removed was later in list
   }

   if ( kmwl > 0 )
      pare_latest_mwl();               // More possible paring for MWL
}

// Pare down data description map to only latest edit of MWL RunIDs
void US_DataLoader::pare_latest_mwl( void )
{
   QStringList       keys = datamap.keys();
   QList< DataDesc > vals = datamap.values();
   QStringList       mwruns;
   QStringList       mwedts;
   QStringList       mwrmvs;

   // First accumulate Runs and Edits for any MWL entries
   for ( int ii = 0; ii < keys.size(); ii++ )
   {
      QString fname  = vals.at( ii ).filename;
      QString cwavln = fname.section( ".", -2, -2 );

      if ( ! cwavln.contains( "-" ) )
         continue;                             // Skip if not MWL

      QString cedit  = fname.section( ".",  0, -6 );
      QString crunid = cedit.section( ".",  0, -2 );

      if ( ! mwruns.contains( crunid ) )
         mwruns << crunid;

      if ( ! mwedts.contains( cedit  ) )
         mwedts << cedit;
   }

   // Go no further if there were less than 2 MWL edits
   //  or if the MWL edits count equals the MWL run count
   if ( mwedts.count() < 2  ||  mwedts.count() == mwruns.count() )
      return;

   // Now accumulate a list of earlier edits to remove
   mwruns.sort();                              // Insure sorted list
   QString prevedt = mwedts[ 0 ];
   QString prevrun = prevedt.section( ".",  0, -2 );

   for ( int ii = 1; ii < mwedts.count(); ii++ )
   {
      QString curredt = mwedts[ ii ];
      QString currrun = curredt.section( ".",  0, -2 );

      if ( currrun == prevrun )
      {  // If runs are the same, list the earlier edit
         mwrmvs << curredt;
      }

      prevedt         = curredt;
      prevrun         = currrun;
   }

   // Finally, remove any in overall list that are marked for delete
   for ( int ii = 0; ii < keys.size(); ii++ )
   {
      QString fname  = vals.at( ii ).filename;
      QString cwavln = fname.section( ".", -2, -2 );

      if ( ! cwavln.contains( "-" ) )
         continue;                             // Skip if not MWL

      QString cedit  = fname.section( ".",  0, -6 );

      if ( mwrmvs.contains( cedit ) )
      {  // Remove an entry found in remove list
         QString clabel = keys.at( ii );
         datamap.remove( clabel );
      }
   }
}

// Filter events to catch right-mouse-button-click on list widget
bool US_DataLoader::eventFilter( QObject* obj, QEvent* e )
{
   if ( obj == tw_data  &&
         e->type() == QEvent::ContextMenu )
   {
      QPoint mpos = ((QContextMenuEvent*)e)->pos();

      show_data_info( mpos );

      return false;
   }
   else
   {  // Pass all other events to normal handler
      return US_WidgetsDialog::eventFilter( obj, e );
   }
}

// Show selected-data information in text dialog
void US_DataLoader::show_data_info( QPoint pos )
{
   QList< QTreeWidgetItem* > selitems = tw_data->selectedItems();

   QTreeWidgetItem* item = selitems.size() > 0 ? selitems.at( 0 )
                                               : tw_data->itemAt( pos );
   int keyndx       = item->type();

   ddesc            = datamap.values()[ keyndx ];

   QString label    = ddesc.label;
   QString descript = ddesc.descript;
   QString dbID     = QString::number( ddesc.DB_id );
   QString aucID    = QString::number( ddesc.auc_id );
   QString expID    = QString::number( ddesc.exp_id );
   QString filename = ddesc.filename;
   QString filespec = filename;

   if ( ddesc.DB_id < 0 )
   {  // Local disk: separate name and directory
      filespec      = filename.section( "/", -1, -1 )
                      + tr( "\n  File Directory:        " )
                      + filename.section( "/", 0, -2 );
   }

   QString dtext    = tr( "Data Information:" )
      + tr( "\n  Label:                 " ) + label
      + tr( "\n  Description:           " ) + descript
      + tr( "\n  Edit Database ID:      " ) + dbID
      + tr( "\n  Filename:              " ) + filespec
      + tr( "\n  Last Updated:          " ) + ddesc.date
      + tr( "\n  Edit Global ID:        " ) + ddesc.dataGUID
      + tr( "\n  AUC Global ID:         " ) + ddesc.aucGUID
      + tr( "\n  AUC DB ID:             " ) + aucID
      + tr( "\n  Experiment DB ID:      " ) + expID
      + tr( "\n  Experiment Label:      " ) + ddesc.elabel
      + tr( "\n  Run ID:                " ) + ddesc.runID
      + tr( "\n  Triple ID:             " ) + ddesc.tripID
      + tr( "\n  Edit ID:               " ) + ddesc.editID
      + tr( "\n  Triples per Run:       " ) + QString::number( ddesc.tripknt )
      + tr( "\n  Triple Index:          " ) + QString::number( ddesc.tripndx )
      + tr( "\n  Edits per Triple:      " ) + QString::number( ddesc.editknt )
      + tr( "\n  Edit Index:            " ) + QString::number( ddesc.editndx )
      + "";

   // Open a dialog and display model information
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true, "", this );
   edit->setWindowTitle( tr( "Data Information" ) );
   edit->move( this->pos() + pos + QPoint( 100, 100 ) );
   edit->resize( 700, 350 );
   edit->e->setFont( QFont( US_Widgets::fixedFont().family(),
                            US_GuiSettings::fontSize() ) );
   edit->e->setText( dtext );
   edit->show();
}

// Slot to update disk/db selection
void US_DataLoader::update_disk_db( bool db )
{
   emit changed( db );

   le_dfilter->disconnect();
   le_dfilter->clear();
   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );
   pb_invest->setEnabled( ( US_Settings::us_inv_level() > 0 ) && db );

   list_data();
}

// Internal utility to get local file experiment information (label,id)
void US_DataLoader::experiment_info( QString& rundir, QString& elabel,
      QString& expID )
{
   elabel             = "";
   expID              = "-1";

   rundir.replace( "\\", "/" );
   if ( QString( rundir ).right( 1 ) != "/" )
      rundir            += "/";

   QString runID      = QString( rundir ).section( "/", -2, -2 );
   QStringList ffilt( runID + ".??.xml" );
   QStringList efiles = QDir( rundir ).entryList( ffilt, QDir::Files,
                                                  QDir::Name );
   if ( efiles.size() != 1 )
      return;

   QString fname      = efiles[ 0 ];
   QString fpath      = rundir + efiles[ 0 ];
   QFile filei( fpath );

   if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      return;

   QXmlStreamReader xml( &filei );

   while( ! xml.atEnd() )
   {
      xml.readNext();

      if ( xml.isStartElement() )
      {
         if ( xml.name() == "experiment" )
         {
            QXmlStreamAttributes atts = xml.attributes();
            expID              = atts.value( "id" ).toString();
         }

         else if ( xml.name() == "label" )
         {
            xml.readNext();
            elabel             = xml.text().toString();
         }
      }
   }

   filei.close();
   return;
}

