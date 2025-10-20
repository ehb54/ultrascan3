//! \file us_select_rundd.cpp

#include "us_select_rundd.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_editor.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_sleep.h"

// Main constructor with flags for select-runID dialog

US_SelectRunDD::US_SelectRunDD( bool dbase, QStringList& runIDs,
   QStringList& mdescrs ) : US_WidgetsDialog( nullptr, Qt::WindowFlags() ),
   runIDs( runIDs ), mDescrs( mdescrs )
{
   sel_db        = dbase;
   dbg_level     = US_Settings::us_debug();
   nimodel       = mDescrs.count();
   mcounted      = false;

   setWindowTitle( tr( "Select Run ID(s) for Discrete Distributions (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 480, 300 );
DbgLv(1) << "SE:sel_db" << sel_db;

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout;

   dkdb_cntrls         = new US_Disk_DB_Controls(
      sel_db ? US_Disk_DB_Controls::DB : US_Disk_DB_Controls::Disk );
   pb_invest           = us_pushbutton( tr( "Select Investigator" ) );
   QString invnum      = QString::number( US_Settings::us_inv_ID() ) + ": ";
   QString invusr      = US_Settings::us_inv_name();
   le_invest           = us_lineedit( invnum + invusr, 0, true );
   pb_invest->setEnabled( sel_db );

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );

   le_dfilter      = us_lineedit();

   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );
   connect( pb_invest,   SIGNAL( clicked()    ),
                         SLOT  ( get_person() ) );
   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   int row           = 0;
   top->addLayout( dkdb_cntrls, row++, 0, 1, 3 );
   top->addWidget( pb_invest,   row,   0, 1, 1 );
   top->addWidget( le_invest,   row++, 1, 1, 2 );
   top->addWidget( lb_filtdata, row,   0, 1, 1 );
   top->addWidget( le_dfilter,  row++, 1, 1, 2 );

   main->addLayout( top );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

   // List widget to show data choices
   lw_data = new QListWidget( this );
   lw_data->setFrameStyle   ( QFrame::NoFrame );
   lw_data->setPalette      ( US_GuiSettings::editColor() );
   lw_data->setFont         ( font );
   lw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );
   connect( lw_data,  SIGNAL( itemSelectionChanged() ),
            this,     SLOT  ( selectionChanged()     ) );

   main->addWidget( lw_data );

   // Button Row
   QHBoxLayout* buttons   = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );

   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );

   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // Status Row
   QFontMetrics fm( font );
   int fhigh = fm.lineSpacing();
   int fwide = fm.horizontalAdvance( QChar( '6' ) );
   int lhigh = fhigh * 4 + 12;
   int lwide = fwide * 32;

   te_status               = us_textedit();
   te_status->setMaximumHeight( lhigh );
   te_status->resize( lwide, lhigh );
   us_setReadOnly( te_status, true );
   te_status->setTextColor( Qt::blue );

   main->addWidget( te_status );

   // List from disk or db source
   list_data();
}

// Hide list items that do not contain search string
void US_SelectRunDD::search( const QString& search_string )
{
   lw_data->setCurrentItem( NULL );

   for ( int ii = 0; ii < lw_data->count(); ii++ )
   {
      QListWidgetItem* lwi = lw_data->item( ii );
      bool hide = ! lwi->text().contains( search_string, Qt::CaseInsensitive ); 
      lwi->setHidden( hide );
   }
}

// List data choices (from db or disk)
void US_SelectRunDD::list_data()
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   rlabels.clear();

   if ( sel_db )              // Scan database data
   {
      scan_dbase_runs();
   }
   else                       // Scan local disk data
   {
      scan_local_runs();
   }

   QApplication::restoreOverrideCursor();
   lw_data->clear();

   if ( rlabels.size() == 0 )
   {  // Report and return now if no items found
      QString clabel = tr( "No data found." );
      lw_data->addItem( new QListWidgetItem( clabel ) );
      return;
   }
DbgLv(1) << "LD:sel_db" << sel_db << "rlsize" << rlabels.size();

   sort_rlabels( rlabels );

   for ( int ii = 0; ii < slabels.size(); ii++ )
   {  // Propagate list widget with labels
      QString  clabel  = slabels.at( ii );

      lw_data->addItem( new QListWidgetItem( clabel ) );
   }

   // Report list state in status box
   count_list = lw_data->count();
   count_seld = lw_data->selectedItems().size();
   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 (may) have associated distributions (models), and\n"
          "%3 %4 currently selected for combination plot components.\n"
          "List items are in recent-to-older order." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld != 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

// Cancel button:  no runIDs returned
void US_SelectRunDD::cancelled()
{
   runIDs.clear();

   reject();
   close();
}

// Accept button:  set up to return runs and associated model information
void US_SelectRunDD::accepted()
{
DbgLv(1) << "SE:accepted()";
   QList< QListWidgetItem* > selitems = lw_data->selectedItems();

   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect or Cancel" ) );
      return;
   }

   QString slines = te_status->toPlainText() + 
      tr( "\nScanning models for selected run(s)..." );
   te_status->setText( slines );
   qApp->processEvents();
DbgLv(1) << "SE: slines" << slines;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   runIDs.clear();

   // Get and return runIDs from selected edit items
   for ( int ii = 0; ii < selitems.size(); ii++ )
   {
      QListWidgetItem* lwi_data = selitems.at( ii );
      QString clabel = lwi_data->text();
DbgLv(1) << "SE:  ii clabel" << ii << clabel;

      runIDs << clabel;
   }
DbgLv(1) << "SE: runID0" << runIDs[0];

   // Scan models to build descriptions for selected runs
   if ( sel_db )
      scan_dbase_models();
   else
      scan_local_models();

   QApplication::restoreOverrideCursor();
   qApp->processEvents();
   int namodel = wDescrs.size();

   if ( namodel == 0 )
   {
      QMessageBox::warning( this,
         tr( "No Implied Models" ),
         tr( "There were no Discrete Distributions associated\n"
             " with the selected run(s).\n"
             "Cancel or select a new set of runs." ) );
      return;
   }

   mDescrs << wDescrs;      // Append new model descriptions

   accept();                // Signal that selection was accepted
   close();
}

// Scan database for run sets
void US_SelectRunDD::scan_dbase_runs()
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   count_list = 0;
   count_seld = 0;
   rlabels.clear();
   runIDs .clear();
   mRDates.clear();

   rlabels << "UNASSIGNED";
   runIDs  << "UNASSIGNED";
   mRDates[ "UNASSIGNED" ] = "all";
   count_allr = 1;

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::critical( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   QStringList  query;
   QString invID    = QString::number( US_Settings::us_inv_ID() );
QTime timer;
timer.start();

   // Scan experiments. getting run and experiment IDs
   query.clear();
   query << "get_experiment_desc" << invID;
   db.query( query );

   while ( db.next() )
   {  // Get a list of all runs in the database
      QString runid    = db.value( 1 ).toString();
      QString rdate    = db.value( 5 ).toDateTime().toString( "yyMMddhhmm" );
DbgLv(1) << "ScDB:     runid" << runid;

      if ( ! runIDs.contains( runid ) )
      {
         count_allr++;
         rlabels << runid;     // Save run ID to list of selectable runs
         runIDs  << runid;     // Save run ID to list of total runs
         mRDates[ runid ] = rdate;    // Save mapping of date to runID
      }
   }
DbgLv(1) << "ScDB:count_allr" << count_allr;
DbgLv(1) << "ScDB:scan time(1)" << timer.elapsed();

   mcounted = false;
DbgLv(1) << "ScDB:counts: runIDs" << runIDs.count() << "rlabels" << rlabels.count();
DbgLv(1) << "ScDB:scan time(9)" << timer.elapsed();
}


// Scan local disk for edit sets
void US_SelectRunDD::scan_local_runs( void )
{
QTime timer;
timer.start();
   mRDates.clear();
   QMap< QString, QString > ddmap;
   QStringList mEdtIDs;
   QString     mdir    = US_Settings::dataDir() + "/models";
   QStringList mfilt( "M*.xml" );
   QStringList f_names = QDir( mdir )
                         .entryList( mfilt, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {  // Examine each model file
      QString fname( mdir + "/" + f_names[ ii ] );
      QFile m_file( fname );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &m_file );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "model" )
            {  // Get the description and GUID of model and test it
               QXmlStreamAttributes attr = xml.attributes();
               QString mdesc = attr.value( "description" ).toString();
               QString ddesc = attr.value( "dataDescrip" ).toString();
               QString mGUID = attr.value( "modelGUID"   ).toString();
               QString eGUID = attr.value( "editGUID"    ).toString();
               int     kk    = mdesc.lastIndexOf( ".model" );
                       mdesc = ( kk < 1 ) ? mdesc : mdesc.left( kk );
               QString runid = mdesc.section( ".", 0, -3 );
               // Skip the model if it has no valid runID part
               if ( runid.isEmpty() || runid.length() < 2 )  continue;

               if ( ddesc.isEmpty() )
               {
                  if ( ddmap.contains( eGUID ) )
                     ddesc         = ddmap[ eGUID ];
               }
               else
               {
                  if ( !ddmap.contains( eGUID ) )
                     ddmap[ eGUID ] = ddesc;
               }

               // Modify the description field of any MonteCarlo
               if ( mdesc.contains( "-MC" ) )
               {
                  kk            = mdesc.indexOf( "_mc0" );
                  if ( kk > 0 )
                  {
                     int niters    = attr.value( "MCIteration" ).toString()
                                                                .toInt();
                     mdesc         = QString( mdesc ).left( kk ) +
                                     QString::asprintf( "_mcN%03d", niters ) +
                                     QString( mdesc ).mid( kk + 7 );
                  }
               }

               // Save run ID and model string of RunID+GUID+Description
               QString odesc  = runid + "\t" + mGUID + "\t" + mdesc
                                      + "\t" + ddesc;
               mRunIDs << runid;
               mEdtIDs << eGUID;
               wDescrs << odesc;
if((dbg_level>0) && (!mdesc.contains("-MC_")||mdesc.contains("_mc")))
 DbgLv(1) << "ScLo: odesc" << odesc;
            }

            if ( xml.name() == "analyte" )
               break;
         }
      }
   }

   // Do another pass on model descriptions to insure data description
   //  gets used for all models with the same edit GUID
   for ( int mm = 0; mm < wDescrs.count(); mm++ )
   {
      QString mdesc  = wDescrs[ mm ];
      QString ddesc  = mdesc.section( "\t", 3, 3 );

      if ( ddesc.isEmpty() )
      {
         QString eGUID  = mEdtIDs[ mm ];
         if ( ddmap.contains( eGUID ) )
         {
            ddesc          = ddmap[ eGUID ];
DbgLv(1) << "ScLo: mm" << mm << "eGUID" << eGUID << "new ddesc" << ddesc;
            mdesc          = mdesc.section( "\t", 0, 2 ) + "\t" + ddesc;
            wDescrs.replace( mm, mdesc );
         }
      }
   }

DbgLv(1) << "ScLo:scan time(1)" << timer.elapsed();

   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
DbgLv(1) << "ScLo:rdir" << rdir << "aucdir count" << aucdirs.count();
   
   QStringList aucfilt( "*.auc" );
   QStringList datfilt;
   count_allr = 0;
   count_list = 0;
   count_seld = 0;
   runIDs .clear();
   rlabels.clear();
   
   for ( int ii = 0; ii < aucdirs.count(); ii++ )
   {  // Examine all the AUC files that exist locally
      QString     subdir   = rdir + "/" + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir ).entryList( 
            aucfilt, QDir::Files, QDir::Name );
DbgLv(1) << "ScLo:  subdir" << subdir << "aucfiles count" << aucfiles.count();

      if ( aucfiles.count() < 1 )
         continue;

      QString aucfbase  = aucfiles.at( 0 );
      QString runID     = aucfbase.section( ".",  0, -6 );
      QString fdate     = QFileInfo( subdir ).lastModified()
                          .toString( "yyMMddhhmm" );
      count_allr++;             // Bump the count of all runIDs examined
      runIDs << runID;

      if ( mRunIDs.contains( runID ) )
      {  // If this runID is represented for models, it is selectable
         count_list++;          // Bump the count of runIDs to list
         rlabels << runID;      // Save selectable runID
         mRDates[ runID ] = fdate;    // Save mapping of date to runID
      }
DbgLv(1) << "ScLo:    count_allr" << count_allr << "count_list" << count_list
 << "   runID" << runID;
   }
DbgLv(1) << "ScLo:rlabels count" << count_list << rlabels.count();
DbgLv(1) << "ScLo:scan time(9)" << timer.elapsed();
   mcounted = true;
}


// Investigator button clicked:  get investigator from dialog
void US_SelectRunDD::get_person()
{
   int invID     = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect( dialog, SIGNAL( investigator_accepted( int ) ),
                    SLOT(   update_person(         int ) ) );

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_SelectRunDD::update_person( int ID )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_invest->setText( number + US_Settings::us_inv_name() );

DbgLv(1) << "UpdP: ID" << ID << "invID" << US_Settings::us_inv_ID();
   list_data();
   count_models();
DbgLv(1) << "UpdP:   rlabels count" << rlabels.count();
}

// Slot to update disk/db selection
void US_SelectRunDD::update_disk_db( bool isDB )
{
   emit changed( isDB );

   sel_db     = isDB;
   list_data();
   count_models();

   pb_invest->setEnabled( isDB );
   setWindowTitle( tr( "Select Run ID(s) for Discrete Distributions (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
}

// Slot to record a change in list item selection
void US_SelectRunDD::selectionChanged()
{
   QList< QListWidgetItem* > selitems = lw_data->selectedItems();
   int  kseld   = selitems.size();
   bool counted = mcounted;

   // If need be, count models and refresh the list
   if ( ! counted )
   {
      QString slabel = selitems[ 0 ]->text();
      selitems.clear();
      lw_data->disconnect();

DbgLv(1) << "sChg: count_models()";
      count_models();

      count_list   = rlabels.count();
      
DbgLv(1) << "sChg: count_list" << count_list << "kseld" << kseld;
      selitems     = lw_data->findItems( slabel, Qt::MatchFixedString );
      kseld        = selitems.size();
      if ( kseld > 0 )
         lw_data->setCurrentItem( selitems[ 0 ] );
      connect( lw_data,  SIGNAL( itemSelectionChanged() ),
               this,     SLOT  ( selectionChanged()     ) );
   }

   count_seld   = kseld;
   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated distributions (models), and\n"
          "%3 %4 currently selected for combination plot components.\n"
          "List items are in recent-to-older order." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld != 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

// Scan database for models associated with run sets
void US_SelectRunDD::scan_dbase_models()
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

   QStringList        mmIDs;        // Model modelIDs
   QStringList        mmGUIDs;      // Model modelGUIDs
   QStringList        meIDs;        // Model editIDs;
   QVector< QString > mmDescs;      // Model descriptions
   QMap< QString, QString > ddmap;  // editID,dataDescr map

   QStringList query;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );
   int          ntmodel = 0;
   int          nmodel  = 0;
   bool         no_una  = ! runIDs.contains( "UNASSIGNED" );
DbgLv(1) << "ScMd: UNASSGN: no-UNASSIGNED" << no_una;

   // First accumulate model information for UNASSIGNED models
   query.clear();
   query << "get_model_desc_by_editID" << invID << "1";
   db.query( query );

   while ( db.next() )
   {
      QString mdlid    = db.value( 0 ).toString();
      QString mdlGid   = db.value( 1 ).toString();
      QString mdesc    = db.value( 2 ).toString();
      QString edtid    = db.value( 6 ).toString();
      int     kk       = mdesc.lastIndexOf( ".model" );
      mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
      QString mrunid   = mdesc.section( ".", -3, -3 );
DbgLv(1) << "ScMd: UNASSGN: mid,eid,mdesc,mrun" << mdlid << edtid << mdesc << mrunid;
      if ( ! runIDs.contains( mrunid )  &&  no_una )
         continue;       // Skip any without a desired run prefix
DbgLv(1) << "ScMd: UNASSGN:  ++USED++";
      mmIDs   << mdlid;
      mmGUIDs << mdlGid;
      meIDs   << edtid;
      mmDescs << mdesc;
      nmodel++;
   }
DbgLv(1) << "ScMd: runid UNASGN editId 1   nmodel" << nmodel;

QTime timer;
timer.start();
   // Accumulate model information for runs present
   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      QString runid    = runIDs[ rr ];
      query.clear();
      query << "get_model_desc_by_runID" << invID << runid;
      db.query( query );

      while( db.next() )
      {
         QString mdlid    = db.value( 0 ).toString();
         QString mdlGid   = db.value( 1 ).toString();
         QString mdesc    = db.value( 2 ).toString();
         QString edtid    = db.value( 6 ).toString();
         int     kk       = mdesc.lastIndexOf( ".model" );
         mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         mmIDs   << mdlid;
         mmGUIDs << mdlGid;
         meIDs   << edtid;
         mmDescs << mdesc;
         nmodel++;
      }
DbgLv(1) << "ScMd: runid" << runid << "nmodel" << nmodel;

      // Repeat the scan for "global-<run>%" variation
      QString grunid   = "Global-" + runid + "%";
      query.clear();
      query << "get_model_desc_by_runID" << invID << grunid;
      db.query( query );

      while( db.next() )
      {
         QString mdlid    = db.value( 0 ).toString();
         QString mdlGid   = db.value( 1 ).toString();
         QString mdesc    = db.value( 2 ).toString();
         QString edtid    = db.value( 6 ).toString();
         int     kk       = mdesc.lastIndexOf( ".model" );
         mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         mmIDs   << mdlid;
         mmGUIDs << mdlGid;
         meIDs   << edtid;
         mmDescs << mdesc;
         nmodel++;
      }
DbgLv(1) << "ScMd:  runid" << runid << "nmodel" << nmodel;
   }
DbgLv(1) << "ScMd:scan time(1)" << timer.elapsed();

   // Accumulate model information for "Global-" UNASSIGNED models
   query.clear();
   query << "get_model_desc_by_runID" << invID << "Global-%";
   db.query( query );

   while ( db.next() )
   {
      QString mdlid    = db.value( 0 ).toString();
      if ( mmIDs.contains( mdlid ) )
         continue;

      QString mdlGid   = db.value( 1 ).toString();
      QString mdesc    = db.value( 2 ).toString();
      int     kk       = mdesc.lastIndexOf( ".model" );
      mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
      mmIDs   << mdlid;
      mmGUIDs << mdlGid;
      meIDs   << "1";
      mmDescs << mdesc;
      nmodel++;
   }
DbgLv(1) << "ScMd: runid glob% UNASGN editId 1   nmodel" << nmodel;

   query.clear();
   query << "count_models" << invID;
   ntmodel  = db.functionQuery( query );
DbgLv(1) << "ScMd: ntmodel" << ntmodel << "nmodel" << nmodel;
DbgLv(1) << "ScMd:scan time(2)" << timer.elapsed();
int m=nmodel-1;
if ( m>1 ) {
DbgLv(1) << "ScMd: 0: id,gid,eid,desc" << mmIDs[0] << mmGUIDs[0] << meIDs[0] << mmDescs[0];
DbgLv(1) << "ScMd: m: id,gid,eid,desc" << mmIDs[m] << mmGUIDs[m] << meIDs[m] << mmDescs[m]; }

   // Scan all saved models from the end back, saving any
   //   cell description by edit ID
   for ( int mm = nmodel - 1; mm >=0; mm-- )
   {
      QString medtid   = meIDs[ mm ];

      if ( ddmap.contains( medtid ) )  continue;   // Skip if already mapped

      // Not yet mapped, so find any cell description in the model XML
      QString mdlid    = mmIDs[ mm ];
      query.clear();
      query << "get_model_info" << mdlid;
      db.query( query );
      db.next();
      QString mxml     = db.value( 2 ).toString();
      int     kk       = mxml.indexOf( "dataDescrip=" );
DbgLv(1) << "ScMd: mm kk medtid" << mm << kk << medtid;

      if ( kk > 0 )
      {  // We have found the data description, so map it
         QString ddesc    = mxml.mid( kk + 13 );
         kk               = ddesc.indexOf( "\"" );
         ddesc            = ddesc.left( kk );
         ddmap[ medtid ]  = ddesc;
      }
   }
DbgLv(1) << "ScMd:scan time(3)" << timer.elapsed();

   // Do one more pass through all the models, completing
   //  the model descriptions
   for ( int mm = 0; mm < nmodel; mm++ )
   {
      QString mID    = mmIDs  [ mm ];
      QString mGUID  = mmGUIDs[ mm ];
      QString mdesc  = mmDescs[ mm ];
      QString meID   = meIDs  [ mm ];
      QString ddesc  = ddmap.contains( meID ) ? ddmap[ meID ] : "";
      QString runid  = mdesc.section( ".", 0, -3 );
      if ( meID == "1" )
              runid  = "UNASSIGNED";
      QString odesc  = runid + "\t" + mGUID + "\t" + mdesc + "\t" + ddesc;
      wDescrs << odesc;
DbgLv(1) << "ScMd:  mm meID" << mm << meID << "ddesc" << ddesc;
   }

DbgLv(1) << "ScMd:scan time(9)" << timer.elapsed();
}

// Scan local disk for models associated with run sets
void US_SelectRunDD::scan_local_models()
{
   QStringList aDescrs = wDescrs;
   wDescrs.clear();

   // Model scan was already done for local. All we need to do
   //  now is limit the descriptions to runIDs selected.
   for ( int mm = 0; mm < aDescrs.count(); mm++ )
   {
      QString mdesc   = aDescrs[ mm ];
      QString runid   = mdesc.section( "\t", 0, 0 );
      QString grunid  = "Global-" + runid;

      if ( runIDs.contains( runid )  ||  runIDs.startsWith( grunid ) )
         wDescrs << mdesc;
   }
DbgLv(1) << "ScMl:counts: aDescrs" << aDescrs.count() << "wDescrs" << wDescrs.count();
}

// Count models for database case
void US_SelectRunDD::count_models()
{
   if ( mcounted  ||  ! sel_db )  return;

   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   rmodKnts.clear();
   QStringList query;
   QString invID    = QString::number( US_Settings::us_inv_ID() );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   te_status->setText( tr( "Scanning runs for model counts..." ) );
   qApp->processEvents();
QTime timer;
timer.start();

   // Build run edit and run model counts lists
   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      QString runid    = runIDs[ rr ];
      int nrmods       = 0;
      query.clear();

      if ( runid != "UNASSIGNED" )
      {
         query << "count_models_by_runID" << invID << runid;
         nrmods          += db.functionQuery( query );
         QString grunid   = "Global-" + runid + "%";
         query.clear();
         query << "count_models_by_runID" << invID << grunid;
      }

      else
      {
         query << "count_models_by_editID" << invID << "1";
         nrmods          += db.functionQuery( query );
         query.clear();
         query << "count_models_by_runID" << invID << "global%";
      }
      nrmods          += db.functionQuery( query );
//DbgLv(1) << "KntM:   rr" << rr << "nrmods" << nrmods << "runid" << runid;

      rmodKnts << nrmods;
   }
DbgLv(1) << "KntM:  counts: runIDs rmodKnts"
 << runIDs.count() << rmodKnts.count();
DbgLv(1) << "KntM:scan time(2)" << timer.elapsed();

   // Reduce the run list to only those with associated models
   lw_data->clear();
   rlabels .clear();

   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      if ( rmodKnts[ rr ] > 0 )
      {
         QString  clabel  = runIDs[ rr ];
         rlabels << clabel;
//         lw_data->addItem( new QListWidgetItem( clabel ) );
      }
   }

   sort_rlabels( rlabels );

   for ( int rr = 0; rr < rlabels.count(); rr++ )
   {
      lw_data->addItem( new QListWidgetItem( slabels[ rr ] ) );
   }
DbgLv(1) << "KntM:scan time(3)" << timer.elapsed();

   mcounted = true;
   QApplication::restoreOverrideCursor();
   count_list = lw_data->count();
   count_seld = lw_data->selectedItems().size();
   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated distributions (models), and\n"
          "%3 %4 currently selected for combination plot components.\n"
          "List items are in recent-to-older order." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld != 1 ? tr( "runs are" ) : tr( "run is" ) ) );
   qApp->processEvents();
}

// Sort given runID labels into date order, newest to oldest
void US_SelectRunDD::sort_rlabels( const QStringList rlabs )
{
   slabels.clear();
   QStringList keys = mRDates.keys();
   QStringList svals;

   for ( int ii = 0; ii < keys.count(); ii++ )
   {  // Create a list of concatenated date+label strings
      QString rlabel   = keys[ ii ];

      if ( rlabs.contains( rlabel ) )
      {
         QString rdate    = mRDates[ rlabel ];
         QString sval     = rdate + "^" + rlabel;
         svals << sval;
      }
   }

   // Sort combined values into date order
   std::sort( svals.begin(), svals.end() );

   for ( int ii = svals.count() - 1; ii >= 0; ii-- )
   {  // Create the sorted labels list (reverse date order)
      QString sval     = svals[ ii ];
      QString rlabel   = sval.section( "^", 1, -1 );
      slabels << rlabel;
   }
}

