//! \file us_data_loader.cpp

#include "us_data_loader.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_dataIO2.h"
#include "us_util.h"
#include "us_editor.h"

// Main constructor with flags for edit, latest-edit and local-data

US_DataLoader::US_DataLoader(
      bool                               late,
      int                                local,
      QVector< US_DataIO2::RawData    >& rData,
      QVector< US_DataIO2::EditedData >& eData,
      QStringList&                       trips,
      QString&                           desc )
 : US_WidgetsDialog( 0, 0 ),
   latest     ( late ),
   rawData    ( rData ),
   editedData ( eData ),
   triples    ( trips ),
   description( desc )
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
   QGridLayout* top  = new QGridLayout;
   int row           = 0;

   // Disk/ DB
   disk_controls = new US_Disk_DB_Controls( local );
   connect( disk_controls, SIGNAL( changed     ( bool ) ),
                           SLOT( update_disk_db( bool ) ) );
   top->addLayout( disk_controls, row++, 0, 1, 2 );

   // Investigator
   // Only enable the investigator button for privileged users
   pb_invest = us_pushbutton( tr( "Select Investigator" ) );

   int inv = US_Settings::us_inv_level();
   pb_invest->setEnabled( ( inv > 0 )  && disk_controls->db() );
   connect( pb_invest, SIGNAL( clicked() ), SLOT( get_person() ) );
   top->addWidget( pb_invest, row, 0 );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QString name = ( inv > 0 ) ? QString::number( inv ) + ": " : "";

   le_invest = us_lineedit( name + US_Settings::us_inv_name() );
   le_invest->setPalette ( gray );
   le_invest->setReadOnly( true );
   top->addWidget( le_invest, row++, 1 );

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );
   top->addWidget( lb_filtdata, row, 0 );

   le_dfilter      = us_lineedit();
   top->addWidget( le_dfilter, row++, 1 );

   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   main->addLayout( top );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

   // Tree widget to show data choices
   tw_data = new QTreeWidget( this );
   tw_data->setFrameStyle   ( QFrame::NoFrame );
   tw_data->setPalette      ( US_GuiSettings::editColor() );
   tw_data->setFont         ( font );
   tw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );

   tw_data->installEventFilter( this );
   main->addWidget( tw_data );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Load" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // List from disk or db source
   list_data();
}

void US_DataLoader::search( const QString& search_string )
{
   tw_data->setCurrentItem( NULL );

   for ( int i = 0; i < tw_data->topLevelItemCount(); i++ )
   {
      QTreeWidgetItem* twi  = tw_data->topLevelItem( i );
      bool             hide = ! twi->text( 0 ).contains( search_string ); 
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
      if ( ! topLevel.contains( twi ) ) topLevel << twi;
   }

   if ( topLevel.size() > 1 )
   { 
      QMessageBox::warning( this,
            tr( "Invalid Selection" ),
            tr( "Only items from one run may be selected." ) );
      return false;
   }

   // Get a list of data to load
   int          index;
   QList< int > indexes;

   foreach ( twi, selections )
   {
      index = twi->type();
      if ( ! indexes.contains( index ) ) indexes << index;
   }

   // Disallow multiple EditIDs from the same triple
   foreach ( index, indexes )
   {
      QString key    = dlabels[ index ];
      QString triple = datamap[ key ].tripID.replace( ".", " / " );

      if ( triples.contains( triple ) )
      {
         QMessageBox::warning( this,
               tr( "Invalid Selection" ),
               tr( "Only one edit from each triple may be selected." ) );
         return false;
      }
   }

   // If we have only a top level item, add the first edit item from
   // each triple

   if ( selections.size() == 1  &&  selections[ 0 ]->parent() == NULL )
   {
      indexes.clear();
      twi = selections[ 0 ];

      for ( int i = 0; i < twi->childCount(); i++ )
      {
         QTreeWidgetItem* child = twi->child( i );
         if ( child == NULL ) continue;

         // Get type of grandchild
         indexes << child->child( 0 )->type();
      }
   }

   if ( ! disk_controls->db() ) // Load files from local disk
   { 
      for ( int ii = 0; ii < indexes.size(); ii++ )
      {
         QString  key      = dlabels[ indexes[ ii ] ];
         ddesc             = datamap[ key ];
         QString  filename = ddesc.filename;
         QString  triple   = ddesc.tripID.replace( ".", " / " );
         triples << triple;

         QString  filedir  = filename.section( "/", 0, -2 );
         filename          = filename.section( "/", -1, -1 );
         QString  message  = tr( "Loading triple " ) + triple;

         emit progress( message );
         qApp->processEvents();

         try
         {
            US_DataIO2::loadData( filedir, filename, editedData, rawData );
         }
         catch ( US_DataIO2::ioError error )
         {
            QMessageBox::warning( this,
                  tr( "Data Error" ),
                  US_DataIO2::errorString( error ) );

            return false;
         }
         catch ( int err )
         {
            QMessageBox::warning( this,
                  tr( "Data Error" ),
                  US_DataIO2::errorString( err ) );

            return false;
         }
      }
   }
   else // Load data from database
   { 
      US_Passwd   pw;
      US_DB2      db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::information( this,
            tr( "DB Connection Problem" ),
            tr( "There was an error connecting to the database:\n" )
            + db.lastError() );

         return false;
      }

      QStringList query;

      for ( int ii = 0; ii < indexes.size(); ii++ )
      {
         QString  key      = dlabels[ indexes[ ii ] ];
         ddesc             = datamap[ key ];
         int      idRec    = ddesc.DB_id;
         QString  triple   = ddesc.tripID.replace( ".", " / " );
         triples << triple;

         QString  filename = ddesc.filename;
         QString  recID    = QString::number( idRec );
         QString  invID    = QString::number( US_Settings::us_inv_ID() );
         QString  aucfn    = ddesc.runID + "."
                             + filename.section( ".", 2, 5 ) + ".auc";
         QString  tempdir  = US_Settings::tmpDir() + "/";
         QString  afn      = tempdir + aucfn;
         QString  efn      = tempdir + filename;

         query.clear();
         query << "get_editedData" << recID;
         db.query( query );
         db.next();
         
         QString message = tr( "Loading triple " ) + triple;
         emit progress( message );
         qApp->processEvents();

         int idAUC = db.value( 0 ).toString().toInt();

         db.readBlobFromDB( afn, "download_aucData", idAUC );
         qApp->processEvents();

         db.readBlobFromDB( efn, "download_editData", idRec );
         qApp->processEvents();

         US_DataIO2::loadData( tempdir, filename, editedData, rawData );

         QFile( afn ).remove();
         QFile( efn ).remove();
      }
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
                    + sep + aucGUID;
}

// Investigator button clicked:  get investigator from dialog
void US_DataLoader::get_person()
{
   int              invID  = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect(
      dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT(   update_person(         int, const QString&, const QString& ) ));

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_DataLoader::update_person( int ID, 
      const QString& /*lname*/, const QString& /*fname*/ )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_invest->setText( number + US_Settings::us_inv_name() );
   list_data();
}

// List data choices (from db or disk)
void US_DataLoader::list_data()
{
   datamap.clear();

   if ( disk_controls->db() ) // Scan database data
   {
      scan_dbase_edit();
   }
   else                       // Scan local disk data
   {
      scan_local_edit();
   }

   tw_data->clear();
   tw_data->setColumnCount( 1 );
   tw_data->setHeaderLabel( tr( "Edited Data Sets" ) );
   
   QTreeWidgetItem* twi_edit;
   QTreeWidgetItem* twi_runi;
   QTreeWidgetItem* twi_trip;

   dlabels                   = datamap.keys();
   QList< DataDesc > ddescrs = datamap.values();
   QString           prlabel = "";
   QString           ptlabel = "";
   int               ndxt    = 1;
   int               ndxe    = 1;

   if ( dlabels.size() == 0 )
   {
      QString clabel = tr( "No data found." );
      twi_runi = new QTreeWidgetItem( QStringList( clabel), 0 );
      tw_data->addTopLevelItem( twi_runi );
      return;
   }

   for ( int ii = 0; ii < dlabels.size(); ii++ )
   {  // Propagate list widget with labels
      QString  cdescr  = dlabels.at( ii );
      DataDesc ddesc   = ddescrs.at( ii );
      QString  crlabel = ddesc.runID;
      QString  ctlabel = ddesc.tripID;
      QString  celabel = ddesc.editID;

      if ( crlabel != prlabel )
      {  // New runID:  add top-level item
         twi_runi = new QTreeWidgetItem( QStringList( crlabel ), ii );
         tw_data->addTopLevelItem( twi_runi );

         twi_trip = new QTreeWidgetItem( QStringList( ctlabel ), ii );
         twi_runi->addChild( twi_trip );
         
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

      // Always add an edit child of triple
      twi_edit = new QTreeWidgetItem( QStringList( celabel ), ii );
      twi_trip->addChild( twi_edit );

      ddesc.tripknt     = ndxt;
      ddesc.tripndx     = ndxt;
      ddesc.editknt     = ndxe;
      ddesc.editndx     = ndxe;
      datamap[ cdescr ] = ddesc;
   }

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
}

// Cancel button:  no models returned
void US_DataLoader::cancelled()
{
   reject();
   close();
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
   QStringList edtIDs;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );

   setWindowTitle( tr( "Load Edited Data from DB" ) );

   query << "all_editedDataIDs" << invID;

   db.query( query );

   while ( db.next() )
   {
      QString recID = db.value( 0 ).toString();
      QString fname = db.value( 2 ).toString().replace( "\\", "/" );
      edtIDs << fname + "\\" + recID;
   }

   for ( int ii = 0; ii < edtIDs.size(); ii++ )
   {
      QString recID   = edtIDs.at( ii ).section( "\\", 1, 1 );
      int     idRec   = recID.toInt();

      query.clear();
      query << "get_editedData" << recID;
      db.query( query );
      db.next();

      QString parID    = db.value( 0 ).toString();
      QString recGUID  = db.value( 1 ).toString();
      QString descrip  = db.value( 2 ).toString();
      QString filename = db.value( 3 ).toString().replace( "\\", "/" );
      QString date     = US_Util::toUTCDatetimeText( db.value( 5 )
                         .toDateTime().toString( Qt::ISODate ), true );
      QString filebase = filename.section( "/", -1, -1 );
      QString runID    = descrip.isEmpty() ? filebase.section( ".", 0, 0 )
                         : descrip;
      QString editID   = filebase.section( ".", 1, 1 );
      QString tripID   = filebase.section( ".", -4, -2 );

      query.clear();
      query << "get_rawData" << parID;
      db.query( query );
      db.next();

      QString parGUID  = db.value( 0 ).toString();
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
      ddesc.aucGUID    = parGUID;
      ddesc.DB_id      = idRec;
      ddesc.date       = date;
      ddesc.tripknt    = 1;
      ddesc.tripndx    = 1;
      ddesc.editknt    = 1;
      ddesc.editndx    = 1;
      ddesc.isEdit     = true;
      ddesc.isLatest   = latest;

      datamap[ descrip ] = ddesc;
   }

   if ( latest )
      pare_to_latest();
}

// Scan local disk for edit sets
void US_DataLoader::scan_local_edit( void )
{
   setWindowTitle( tr( "Load Edited Data from Local Disk" ) );

   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   
   QStringList aucfilt( "*.auc" );
   QStringList edtfilt;
   
   for ( int ii = 0; ii < aucdirs.size(); ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList( 
            aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString aucfbase  = aucfiles.at( 0 );
      QString aucfname  = subdir + "/" + aucfbase;
      QString runID     = aucfbase.section( ".", 0, 0 );
      QString subType   = aucfbase.section( ".", 1, 1 );
      QString tripl     = aucfbase.section( ".", 2, 4 );

      edtfilt.clear();
      //edtfilt <<  runID + ".*."  + subType + "." + tripl + ".xml";
      edtfilt <<  runID + ".*."  + subType + ".*.xml";
      QStringList edtfiles = QDir( subdir ).entryList( 
            edtfilt, QDir::Files, QDir::Name );
      edtfiles.sort();

      if ( edtfiles.size() < 1 )
         continue;

      for ( int jj = 0; jj < edtfiles.size(); jj++ )
      {
         QString filebase = edtfiles.at( jj );
         QString filename = subdir + "/" + filebase;
         QString runID    = filebase.section( ".", 0, 0 );
         QString editID   = filebase.section( ".", 1, 1 );
         editID  = ( editID.length() == 12  &&  editID.startsWith( "20" ) ) ?
                   editID.mid( 2 ) : editID;
         QString tripID   = filebase.section( ".", -4, -2 );
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
            }
         }

         filei.close();

         ddesc.runID      = runID;
         ddesc.tripID     = tripID;
         ddesc.editID     = editID;
         ddesc.label      = label;
         ddesc.descript   = descrip;
         ddesc.filename   = filename;
         ddesc.dataGUID   = recGUID;
         ddesc.aucGUID    = parGUID;
         ddesc.DB_id      = -1;
         ddesc.date       = date;
         ddesc.tripknt    = 1;
         ddesc.tripndx    = 1;
         ddesc.editknt    = 1;
         ddesc.editndx    = 1;
         ddesc.isEdit     = true;
         ddesc.isLatest   = latest;

         datamap[ descrip ] = ddesc;
      }
   }

   if ( latest )
      pare_to_latest();
}

// Pare down data description map to only latest edit
void US_DataLoader::pare_to_latest( void )
{
   QStringList keys = datamap.keys();

   for ( int ii = 0; ii < keys.size() - 1; ii++ )
   {
      int jj = ii + 1;

      QString clabel = keys.at( ii );
      QString flabel = keys.at( jj );

      QString crunid = clabel.section( ".", 0, 1 );
      QString frunid = flabel.section( ".", 0, 1 );

      if ( crunid != frunid )
         continue;

      QString cstype  = clabel.section( ".", 2, 2 );
      QString fstype  = flabel.section( ".", 2, 2 );

      if ( cstype != fstype )
         continue;

      // this record's label differs from next only by edit code: remove it
      datamap.remove( clabel );
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
      + tr( "\n  Database ID:           " ) + dbID
      + tr( "\n  Filename:              " ) + filespec
      + tr( "\n  Last Updated:          " ) + ddesc.date
      + tr( "\n  Data Global ID:        " ) + ddesc.dataGUID
      + tr( "\n  AUC Global ID:         " ) + ddesc.aucGUID
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
   edit->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
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

