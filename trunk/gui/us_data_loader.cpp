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

// main constructor with flags for edit, latest-edit and local-data
US_DataLoader::US_DataLoader( bool editsel, bool late, bool local,
      QString search, QString invtext ) :US_WidgetsDialog( 0, 0 )
{
   ldedit    = editsel;
   latest    = late;
   ondisk    = local;
   dsearch   = search;
   dinvtext  = invtext;

   if ( ldedit )
      setWindowTitle( tr( "Load Edited Data" ) );

   else
      setWindowTitle( tr( "Load Raw AUC Data" ) );

   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize( 320, 300 );

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout( );
   int row           = 0;

   QGridLayout* db_layout   =
      us_radiobutton( tr( "Use Database" ),   rb_db,  !ondisk );
   QGridLayout* disk_layout =
      us_radiobutton( tr( "Use Local Disk" ), rb_disk, ondisk );

   pb_invest = us_pushbutton( tr( "Select Investigator" ) );
   le_invest = us_lineedit();
   le_invest->setReadOnly( false );

   pb_filtdata     = us_pushbutton( tr( "Search" ) );
   le_dfilter      = us_lineedit();
   le_dfilter->setReadOnly( false );
   dsearch         = dsearch.isEmpty() ? QString( "" ) : dsearch;
   le_dfilter->setText( dsearch );

   top->addLayout( db_layout,   row,   0 );
   top->addLayout( disk_layout, row++, 1 );
   top->addWidget( pb_invest,   row,   0 );
   top->addWidget( le_invest,   row++, 1 );
   top->addWidget( pb_filtdata, row,   0 );
   top->addWidget( le_dfilter,  row++, 1 );

   connect( rb_disk,     SIGNAL( toggled(     bool ) ),
            this,        SLOT(   select_disk( bool ) ) );
   connect( pb_invest,   SIGNAL( clicked()           ),
            this,        SLOT(   get_person()        ) );
   connect( le_invest,   SIGNAL( returnPressed()     ),
            this,        SLOT(   investigator()      ) );
   connect( pb_filtdata, SIGNAL( clicked()           ),
            this,        SLOT(   list_data()         ) );
   connect( le_dfilter,  SIGNAL( returnPressed()     ),
            this,        SLOT(   list_data()         ) );

   main->addLayout( top );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

   // List widget to show data choices
   tw_data         = new QTreeWidget( this );
   tw_data->setFrameStyle( QFrame::NoFrame );
   tw_data->setPalette( US_GuiSettings::editColor() );
   tw_data->setFont(    font );
   tw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );

   tw_data->installEventFilter( this );
   main->addWidget( tw_data );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help,   SIGNAL( clicked() ), this, SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( cancelled() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Load" ) );
   connect( pb_accept, SIGNAL( clicked() ), this, SLOT( accepted() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // Default investigator; possibly to current user, based on home directory
   if ( dinvtext.compare( QString( "USER" ) ) == 0 )
   {
      QStringList DB     = US_Settings::defaultDB();
      int         idPers = US_Settings::us_inv_ID();

      if ( DB.size() < 5  ||  idPers < 1 )
         dinvtext = QDir( QDir::homePath() ).dirName();

      else
         dinvtext = QString::number( idPers ) + ": "
                    + US_Settings::us_inv_name();
   }

   le_invest->setText( dinvtext );

   // Trigger data list from disk or db source
   select_disk( ondisk );
}

// Public method to load raw data list from selected set
int US_DataLoader::load_raw( QVector< US_DataIO2::RawData >& rawList,
                             QStringList&                    triples )
{
   int rc = 0;
   int     idRec    = ddesc.DB_id;
   QString filename = ddesc.filename;
   QString dbID     = QString::number( idRec );
   QString dataGUID = ddesc.dataGUID;
   QString aucGUID  = ddesc.aucGUID;
   QString filebase = filename.section( "/", -1, -1 );
   QString filedir  = filename.section( "/",  0, -2 );
   QString triple   = filebase.section( ".",  2,  4 )
                      .replace( ".", " / " );
   QString runID    = filebase.section( ".",  0,  0 );
   QString typeCode = filebase.section( ".",  1,  1 );
   QStringList filt = QStringList() << runID + "."
                                     + typeCode + ".*.*.*.auc";
   QStringList files;
   US_DataIO2::RawData  rdata;

   rawList .clear();
   triples .clear();

   if ( idRec < 0 )
   {  // load files from local disk
      QDir wdir( filedir );
      filedir  = filedir + "/";
      files    = wdir.entryList( filt,
            QDir::Files | QDir::Readable, QDir::Name );
      
      for ( int ii = 0; ii < files.size(); ii++ )
      {
         QString fname = files[ ii ];
         QString fpath = filedir + fname;

         triple        = fname.section( ".", 2, 4 ).replace( ".", " / " );

         if ( !triples.contains( triple ) )
            triples << triple;

         US_DataIO2::readRawData( fpath, rdata );
         rawList << rdata;
         qApp->processEvents();
      }
   }

   else
   {  // load data from database
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );
      QStringList query;
      QStringList rawIDs;
      QStringList rawFNs;
      QString invID   = le_invest->text().section( ":", 0, 0 );
      QString tempdir = US_Settings::tmpDir() + "/";

      query.clear();
      rawIDs.clear();
      rawFNs.clear();

      query << "all_rawDataIDs" << invID;
      db.query( query );

      while ( db.next() )
      {
         rawIDs << db.value( 0 ).toString();
         rawFNs << db.value( 2 ).toString();
      }

      for ( int ii = 0; ii < rawIDs.size(); ii++ )
      {
         QString recID    = rawIDs.at( ii );
         QString fname    = rawFNs.at( ii );
         QString rrunid   = fname.section( ".", 0, 0 );
         QString rtype    = fname.section( ".", 1, 1 );
         triple           = fname.section( ".", 2, 4 ).replace( ".", " / " );

         if ( rrunid == runID  &&  rtype == typeCode )
         {
            if ( !triples.contains( triple ) )
               triples << triple;

            QString afn = tempdir + fname;

            db.readBlobFromDB( afn, "download_aucData", recID.toInt() );

            US_DataIO2::readRawData( afn, rdata );
            rawList << rdata;

            QFile( afn ).remove();
         }
         qApp->processEvents();
      }
   }

   return rc;
}

// Public method to load edit data list (along with raw) from selected set
int US_DataLoader::load_edit( QVector< US_DataIO2::EditedData >& dataList,
                              QVector< US_DataIO2::RawData    >& rawList,
                              QStringList&                       triples )
{
   int     rc       = 0;

   dataList.clear();
   rawList .clear();
   triples .clear();

   if ( ondisk )
   {  // load files from local disk
      for ( int ii = 0; ii < sdescs.size(); ii++ )
      {
         ddesc            = sdescs[ ii ];
         QString filename = ddesc.filename;
         QString triple   = ddesc.tripID.replace( ".", " / " );
         QString filedir  = filename.section( "/", 0, -2 );
         filename         = filename.section( "/", -1, -1 );


         if ( !triples.contains( triple ) )
            triples << triple;

         US_DataIO2::loadData( filedir, filename, dataList, rawList );
         qApp->processEvents();
      }
   }

   else
   {  // load data from database
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );
      QStringList query;

      for ( int ii = 0; ii < sdescs.size(); ii++ )
      {
         ddesc            = sdescs[ ii ];
         int     idRec    = ddesc.DB_id;
         QString triple   = ddesc.tripID.replace( ".", " / " );
         QString filename = ddesc.filename;
         QString recID    = QString::number( idRec );
         QString invID    = le_invest->text().section( ":", 0, 0 );
         QString aucfn    = ddesc.runID + "."
                            + filename.section( ".", 2, 5 ) + ".auc";
         QString tempdir  = US_Settings::tmpDir() + "/";
         QString afn      = tempdir + aucfn;
         QString efn      = tempdir + filename;

         if ( !triples.contains( triple ) )
            triples << triple;

         query.clear();
         query << "get_editedData" << recID;
         db.query( query );
         db.next();
         
         int idAUC        = db.value( 0 ).toString().toInt();

         db.readBlobFromDB( afn, "download_aucData", idAUC );
         qApp->processEvents();

         db.readBlobFromDB( efn, "download_editData", idRec );
         qApp->processEvents();

         US_DataIO2::loadData( tempdir, filename, dataList, rawList );

         QFile( afn ).remove();
         QFile( efn ).remove();
         qApp->processEvents();
      }
   }

   return rc;
}

// Public method to return description string of data last selected
QString US_DataLoader::description( )
{
   QString label    = ddesc.label;
   QString descript = ddesc.descript;
   QString dbID     = QString::number( ddesc.DB_id );
   QString filename = ddesc.filename;
   QString dataGUID = ddesc.dataGUID;
   QString aucGUID  = ddesc.aucGUID;
   QString cdesc    = label + descript + filename + dataGUID + aucGUID + dbID;

   QString sep      = ";";     // use semi-colon as separator

   if ( cdesc.contains( sep ) )
      sep           = "^";  // use carat if semi-colon already in use

   // create and return a composite description string
   cdesc            = sep + label
                    + sep + descript
                    + sep + dbID
                    + sep + filename
                    + sep + dataGUID
                    + sep + aucGUID;
   return cdesc;
}

// Public method to return DB pointer and last-used settings
void US_DataLoader::settings( bool& local, QString& invtext,
      QString& search )
{
   local     = rb_disk->isChecked();
   invtext   = le_invest->text();
   search    = le_dfilter->text();
   return;
}

// Slot to respond to change in disk/db radio button select
void US_DataLoader::select_disk( bool checked )
{
   // Disable investigator field if from disk; Enable if from db
   pb_invest->setEnabled( !rb_disk->isChecked() );
   le_invest->setEnabled( !rb_disk->isChecked() );

   tw_data->clear();

   if ( !checked )
   {  // Disk unchecked (DB checked):  fill edit/raw list from DB
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::information( this,
             tr( "DB Connection Problem" ),
             tr( "There was an error connecting to the database:\n" )
             + db.lastError() );
         return;
      }

      US_Investigator::US_InvestigatorData data;
      QStringList query;
      QString     invtext = le_invest->text();
      QString     newinve = invtext;
      bool        haveInv = false;

      query << "get_people" << invtext;
      db.query( query );

      if ( db.numRows() < 1 )
      {  // Investigator text yields nothing:  retry with blank field
         query.clear();
         query << "get_people" << "";
         db.query( query );
      }

      while ( db.next() )
      {  // Loop through investigators looking for match
         data.invID     = db.value( 0 ).toInt();
         data.lastName  = db.value( 1 ).toString();
         data.firstName = db.value( 2 ).toString();
         if ( db.numRows() < 2  ||
              data.lastName.contains(  invtext, Qt::CaseInsensitive )  ||
              data.firstName.contains( invtext, Qt::CaseInsensitive ) )
         {  // Single investigator or a match to last,first name
            haveInv     = true;
            newinve     = QString::number( data.invID )
               + ": " + data.lastName+ ", " + data.firstName;
            break;
         }
      }

      if ( haveInv )
         le_invest->setText( newinve );
   }

   // Show the list of available data
   list_data();
}

// Investigator text changed:  act like DB radio button just checked
void US_DataLoader::investigator()
{
   select_disk( false );
}

// Investigator button clicked:  get investigator from dialog
void US_DataLoader::get_person()
{
   QString invtext         = le_invest->text();
   int     invID           = invtext.section( ":", 0, 0 ).toInt();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect(
      dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT(   update_person(         int, const QString&, const QString& ) ));

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_DataLoader::update_person( int ID, const QString& lname,
      const QString& fname )
{
   le_invest->setText( QString::number( ID ) + ": " + lname + ", " + fname );
   return;
}

// List data choices (disk/db and possibly filtered by search text)
void US_DataLoader::list_data()
{
   QString dfilt = le_dfilter->text();
   bool listall  = dfilt.isEmpty();
   QRegExp dpart = QRegExp( ".*" + dfilt + ".*", Qt::CaseInsensitive );
   ondisk        = rb_disk->isChecked();
   datamap.clear();

   if ( ondisk )
   {  // scan local disk data

      if ( ldedit )
      {  // scan edit data sets
         scan_local_edit();
      }

      else
      {  // scan raw data sets
         scan_local_raw();
      }
   }

   else
   {  // scan database data

      if ( ldedit )
      {  // scan edit data sets
         scan_dbase_edit();
      }

      else
      {  // scan raw data sets
         scan_dbase_raw();
      }
   }

   // possibly pare down data list based on search field

   tw_data->clear();
   tw_data->setColumnCount( 1 );
   tw_data->setHeaderLabel( tr( "Edited Data Sets" ) );
   QTreeWidgetItem* twi_edit;
   QTreeWidgetItem* twi_runi;
   QTreeWidgetItem* twi_trip;

   QStringList       dlabels = datamap.keys();
   QList< DataDesc > ddescrs = datamap.values();
   QString prlabel = "";
   QString ptlabel = "";
   int     ndxt    = 1;
   int     ndxe    = 1;
   int     dlsize  = dlabels.size();

   if ( dlsize == 0 )
   {
      QString clabel = tr( "No data found." );
      twi_runi = new QTreeWidgetItem( QStringList( clabel), 0 );
      tw_data->addTopLevelItem( twi_runi );
      return;
   }

   for ( int ii = 0; ii < dlsize; ii++ )
   {  // propagate list widget with labels
      QString cdescr  = dlabels.at( ii );
      DataDesc ddesc  = ddescrs.at( ii );
      QString crlabel = ddesc.runID;
      QString ctlabel = ddesc.tripID;
      QString celabel = ddesc.editID;

      if ( !listall  &&  !crlabel.contains( dpart ) )
         continue;

      if ( crlabel != prlabel )
      {  // new runID:  add top-level item
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
      {  // new triple in same runID:  add triple child of run
         twi_trip = new QTreeWidgetItem( QStringList( ctlabel ), ii );
         twi_runi->addChild( twi_trip );
         ptlabel  = ctlabel;
         ndxt++;
         ndxe     = 1;
      }

      else
      {  // same triple as before
         ndxt     = 1;
         ndxe++;
      }

      // always add an edit child of triple
      twi_edit = new QTreeWidgetItem( QStringList( celabel ), ii );
      twi_trip->addChild( twi_edit );

      ddesc.tripknt   = ndxt;
      ddesc.tripndx   = ndxt;
      ddesc.editknt   = ndxe;
      ddesc.editndx   = ndxe;
      datamap[ cdescr ] = ddesc;
   }

   // walk through entries backwards to propagate edit,triple counts
   prlabel = "";
   ptlabel = "";
   ndxt    = 1;
   ndxe    = 1;
   ddescrs = datamap.values();

   for ( int ii = dlsize - 1; ii >= 0; ii-- )
   {
      bool     update  = false;
      QString  cdescr  = dlabels.at( ii );
      DataDesc ddesc   = ddescrs.at( ii );
      QString  crlabel = ddesc.runID;
      QString  ctlabel = ddesc.tripID;

      if ( !listall  &&  !crlabel.contains( dpart ) )
         continue;

      if ( crlabel != prlabel )
      {  // new run:   get triple and edit count
         ndxt          = ddesc.tripknt;
         ndxe          = ddesc.editknt;
         prlabel       = crlabel;
      }

      else
      {  // same run:  update triple count
         ddesc.tripknt = ndxt;
         ddesc.editknt = ndxe;
         update        = true;
      }

      if ( ctlabel != ptlabel )
      {  // new triple:  get edit count
         ndxe          = ddesc.editknt;
         ptlabel       = ctlabel;
      }

      else
      {  // same triple:  update edit count
         ddesc.editknt = ndxe;
         update        = true;
      }

      if ( update )
      {  // one or other count has been updated
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

   sdescs.clear();

   for ( int ii = 0; ii < selitems.size(); ii++ )
   {
      QTreeWidgetItem* sitem  = selitems[ ii ];

      int              index  = sitem->type();
      int              nchild = sitem->childCount(); 

      if ( nchild <= 1 )
      {  // item has 1 child or none:  use this item's description
         sdescs << ddescrs[ index ];
      }

      else
      {  // otherwise, get all children's descriptions
         for ( int jj = 0; jj < nchild; jj++ )
         {
            QTreeWidgetItem* citem  = sitem->child( jj );
            int              jndex  = citem->type();
            int              jchild = citem->childCount(); 

            if ( jchild <= 1 )
            {  // child item has 1 child or none:  use this child's description
               sdescs << ddescrs[ jndex ];
            }

            else
            {  // otherwise, use the last grandchild's description
               QTreeWidgetItem* gitem  = citem->child( jchild - 1 );
               int              kndex  = gitem->type();
               sdescs << ddescrs[ kndex ];
            }
         }
      }
   }

   accept();        // signal that selection was accepted
   close();
}

// scan database for edit sets
int US_DataLoader::scan_dbase_edit()
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   int nall = 0;
   int nrec = 0;
   QStringList query;
   QStringList edtIDs;
   QString invID  = le_invest->text().section( ":", 0, 0 );

   setWindowTitle( tr( "Load Edited Data from DB" ) );

   query.clear();
   edtIDs.clear();

   query << "all_editedDataIDs" << invID;
   db.query( query );

   while ( db.next() )
   {
      QString recID = db.value( 0 ).toString();
      QString fname = db.value( 2 ).toString().replace( "\\", "/" );
      QString elabl = fname + "\\" + recID;
      edtIDs << elabl;
   }

   nall    = edtIDs.size();

   for ( int ii = 0; ii < nall; ii++ )
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
      editID           = ( editID.length() == 12 ) ? editID
                         : editID + "0000";
      QString tripID   = filebase.section( ".", -4, -2 );

      query.clear();
      query << "get_rawData" << parID;
      db.query( query );
      db.next();

      QString parGUID  = db.value( 0 ).toString();
      QString label    = runID;
      descrip          = runID + "." + tripID + "." + editID;
      QString baselabl = label;
      //int nextdup      = 2;

      //while ( datamap.contains( label ) )
      //{
      //   QString numpart  = QString::number( nextdup++ );
      //   label            = baselabl + "(" + numpart + ")";
      //}

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
      nrec = pare_to_latest();

   else
      nrec = nall;

   return nrec;
}

// scan database for raw sets
int US_DataLoader::scan_dbase_raw()
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   int nrec = 0;
   QStringList query;
   QStringList rawIDs;
   QString invID  = le_invest->text().section( ":", 0, 0 );

   setWindowTitle( tr( "Load AUC Data from DB" ) );

   query.clear();
   rawIDs.clear();

   query << "all_rawDataIDs" << invID;
   db.query( query );

   while ( db.next() )
   {
      rawIDs << db.value( 0 ).toString();
   }

   nrec    = rawIDs.size();

   for ( int ii = 0; ii < nrec; ii++ )
   {
      QString recID   = rawIDs.at( ii );
      int     idRec   = recID.toInt();

      query.clear();
      query << "get_rawData" << recID;
      db.query( query );
      db.next();

      QString recGUID  = db.value( 0 ).toString();
      QString descrip  = db.value( 1 ).toString();
      QString filename = db.value( 2 ).toString().replace( "\\", "/" );
      QString parGUID  = recGUID;
      QString filebase = filename.section( "/", -1, -1 );
      QString runID    = descrip.isEmpty() ? filebase.section( ".", 0, 0 )
                         : descrip;
      QString label    = runID;
      QString baselabl = label;
      //int nextdup      = 2;

      //while ( datamap.contains( label ) )
      //{
      //   QString numpart  = QString::number( nextdup++ );
      //   label            = baselabl + "(" + numpart + ")";
      //}

      ddesc.label      = label;
      ddesc.descript   = descrip;
      ddesc.DB_id      = idRec;
      ddesc.filename   = filename;
      ddesc.dataGUID   = recGUID;
      ddesc.aucGUID    = parGUID;
      ddesc.isEdit     = false;
      ddesc.isLatest   = false;

      datamap[ descrip ] = ddesc;
   }

   return nrec;
}

// scan local disk for edit sets
int US_DataLoader::scan_local_edit()
{
   setWindowTitle( tr( "Load Edited Data from Local Disk" ) );

   int nrec  = 0;
   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   QStringList aucfilt;
   QStringList edtfilt;
   aucfilt << "*.auc";
   int naucd = aucdirs.size();

   for ( int ii = 0; ii < naucd; ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir )
         .entryList( aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString aucfbase  = aucfiles.at( 0 );
      QString aucfname  = subdir + "/" + aucfbase;
      QString runID     = aucfbase.section( ".", 0, 0 );
      QString subType   = aucfbase.section( ".", 1, 1 );
      QString tripl     = aucfbase.section( ".", 2, 4 );

      edtfilt.clear();
      edtfilt <<  runID + ".*."  + subType + "." + tripl + ".xml";
      QStringList edtfiles = QDir( subdir )
         .entryList( edtfilt, QDir::Files, QDir::Name );

      if ( edtfiles.size() < 1 )
         continue;

      for ( int jj = 0; jj < edtfiles.size(); jj++ )
      {
         QString filebase = edtfiles.at( jj );
         QString filename = subdir + "/" + filebase;
         QString runID    = filebase.section( ".", 0, 0 );
         QString editID   = filebase.section( ".", 1, 1 );
         editID           = ( editID.length() == 12 ) ? editID
                            : editID + "0000";
         QString tripID   = filebase.section( ".", -4, -2 );
         QString label    = runID;
         QString descrip  = runID + "." + tripID + "." + editID;
         QString baselabl = label;
         //int nextdup      = 2;

         //while ( datamap.contains( label ) )
         //{
         //   QString numpart  = QString::number( nextdup++ );
         //   label            = baselabl + "(" + numpart + ")";
         //}

         QFile filei( filename );

         if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QString date     = US_Util::toUTCDatetimeText( QFileInfo( filename )
                            .lastModified().toUTC().toString( Qt::ISODate )
                            , true );
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
         nrec++;
      }
   }

   if ( latest )
      nrec = pare_to_latest();

   return nrec;
}

// scan local disk for raw (AUC) sets
int US_DataLoader::scan_local_raw()
{
   setWindowTitle( tr( "Load AUC Data from Local Disk" ) );

   int nrec = 0;
   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   QStringList aucfilt;
   QStringList edtfilt;
   aucfilt << "*.auc";
   edtfilt << "*.xml";
   int naucd = aucdirs.size();

   for ( int ii = 0; ii < naucd; ii++ )
   {
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir )
         .entryList( aucfilt, QDir::Files, QDir::Name );

      if ( aucfiles.size() < 1 )
         continue;

      QString filebase = aucfiles.at( 0 );
      QString filename = subdir + "/" + filebase;
      QString descrip  = filebase.section( ".", 0, 2 );
      QString label    = label;
      QString expfname = descrip + ".xml";
      QString baselabl = label;
      //int nextdup      = 2;

      //while ( datamap.contains( label ) )
      //{
      //   QString numpart  = QString::number( nextdup++ );
      //   label            = baselabl + "(" + numpart + ")";
      //}

      QFile filei( expfname );

      if ( !filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &filei );
      QXmlStreamAttributes a;
      QString recGUID;
      QString parGUID;

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "dataset" )
         {
            a         = xml.attributes();
            recGUID   = a.value( "guid" ).toString();
            parGUID   = recGUID;
         }
      }

      filei.close();

      ddesc.label      = label;
      ddesc.descript   = descrip;
      ddesc.filename   = filename;
      ddesc.dataGUID   = recGUID;
      ddesc.aucGUID    = parGUID;
      ddesc.DB_id      = -1;
      ddesc.isEdit     = false;
      ddesc.isLatest   = false;

      datamap[ descrip ] = ddesc;
      nrec++;
   }

   return nrec;
}

// pare down data description map to only latest edit
int US_DataLoader::pare_to_latest()
{
   int nrec = datamap.size();

   int lrec = nrec - 1;

   QStringList keys = datamap.keys();

   for ( int ii = 0; ii < lrec; ii++ )
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
      nrec--;
   }

   return nrec;
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
   {  // pass all other events to normal handler
      return US_WidgetsDialog::eventFilter( obj, e );
   }
}

// Show selected-data information in text dialog
void US_DataLoader::show_data_info( QPoint pos )
{
   QList< QTreeWidgetItem* > selitems = tw_data->selectedItems();
   QTreeWidgetItem* item = selitems.size() > 0 ?
                           selitems.at( 0 )    :
                           tw_data->itemAt( pos );
   int keyndx       = item->type();

   ddesc            = datamap.values()[ keyndx ];

   QString label    = ddesc.label;
   QString descript = ddesc.descript;
   QString dbID     = QString::number( ddesc.DB_id );
   QString filename = ddesc.filename;
   QString filespec = filename;

   if ( ddesc.DB_id < 0 )
   {  // local disk: separate name and directory
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

   // open a dialog and display model information
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true, "", this );
   edit->setWindowTitle( tr( "Data Information" ) );
   edit->move( this->pos() + pos + QPoint( 100, 100 ) );
   edit->resize( 700, 350 );
   edit->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   edit->e->setText( dtext );
   edit->show();
}

