//! \file us_select_runid.cpp

#include "us_select_runid.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_dataIO2.h"
#include "us_util.h"
#include "us_editor.h"
#include "us_constants.h"
#include "us_report.h"

// Main constructor with flags for select-runID dialog

US_SelectRunid::US_SelectRunid( bool dbase, QStringList& runIDs,
   QStringList& mdescrs ) : US_WidgetsDialog( 0, 0 ),
   runIDs( runIDs ), mDescrs( mdescrs )
{
   sel_db        = dbase;
   dbg_level     = US_Settings::us_debug();

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
   int fwide = fm.width( QChar( '6' ) );
   int lhigh = fhigh * 3 + 12;
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
void US_SelectRunid::search( const QString& search_string )
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
void US_SelectRunid::list_data()
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

   for ( int ii = 0; ii < rlabels.size(); ii++ )
   {  // Propagate list widget with labels
      QString  clabel  = rlabels.at( ii );

      lw_data->addItem( new QListWidgetItem( clabel ) );
   }

   // Report list state in status box
   count_list = lw_data->count();
   count_seld = lw_data->selectedItems().size();
   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated discrete distribution data (models).\n"
          "%3 %4 currently selected for combination plot components." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld > 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

// Cancel button:  no runIDs returned
void US_SelectRunid::cancelled()
{
   reject();
   close();
}

// Accept button:  set up to return runs and associated model information
void US_SelectRunid::accepted()
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

   // Get and return runIDs from selected edit items
   for ( int ii = 0; ii < selitems.size(); ii++ )
   {
      QListWidgetItem* lwi_data = selitems.at( ii );
      QString clabel = lwi_data->text();
DbgLv(1) << "SE:  ii clabel" << ii << clabel;

      runIDs << clabel;
   }
DbgLv(1) << "SE: runID" << runIDs[0];

   accept();        // Signal that selection was accepted
   close();
}

// Scan database for run sets
void US_SelectRunid::scan_dbase_runs()
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );
   count_allr = 0;
   count_list = 0;
   count_seld = 0;

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return;
   }

   QStringList  runIDs;  // All runs
   QStringList  expIDs;  // All experiments
   QStringList  edtIDs;  // All edits
   QStringList  eexIDs;  // Parallel edit experiments
   QList< int > rmknts;  // Run model counts
   QString      runid;
   QString      expid;
   QString      edtid;
   QString      eexid;

   QStringList query;
   QString     invID  = QString::number( US_Settings::us_inv_ID() );

   query.clear();
   query << "get_experiment_desc" << invID;
   db.query( query );

   while ( db.next() )
   {  // Get lists of all experiments and runs; initialize run model counts
      expid            = db.value( 0 ).toString();
      runid            = db.value( 1 ).toString();
DbgLv(1) << "ScDB:     runid" << runid << "expid" << expid;

      if ( ! runIDs.contains( runid ) )
      {
         count_allr++;
         runIDs << runid;
         expIDs << expid;
         rmknts << 0;
      }
   }

   query.clear();
   query << "all_editedDataIDs" << invID;
   db.query( query );

   while ( db.next() )
   {  // Get lists of all edits and their corresponding experiments
      edtid            = db.value( 0 ).toString();
      expid            = db.value( 4 ).toString();
      edtIDs << edtid;
      eexIDs << expid;
   }

   for ( int ii = 0; ii < edtIDs.count(); ii++ )
   {  // Get models for each edit
      edtid            = edtIDs[ ii ];
      expid            = eexIDs[ ii ];
      int     jj       = expIDs.indexOf( expid );
      if ( jj < 0 )  continue;
      runid            = runIDs[ jj ];
      int     nmodel   = 0;

      query.clear();
      query << "get_model_desc_by_editID" << invID << edtid;
      db.query( query );

      while ( db.next() )
      {  // Save each model for a run
         QString mGUID  = db.value( 1 ).toString();
         QString mdesc  = db.value( 2 ).toString();
         int     kk     = mdesc.lastIndexOf( ".model" );
         mdesc          = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         QString odesc  = runid + "\t" + mGUID + "\t" + mdesc;
         mRunIDs << runid;    // Save run ID
         mDescrs << odesc;    // Save model description string
         nmodel++;
if((dbg_level>0) && (!mdesc.contains("-MC_0")||mdesc.contains("_mc0001")))
 DbgLv(1) << "ScDB: odesc" << odesc;
      }

      if ( nmodel > 0 )
      {  // If run had associated models, mark it with the model count
         rmknts.replace( jj, nmodel );
      }
   }

DbgLv(1) << "ScDB: count_allr" << count_allr << runIDs.size();
   for ( int ii = 0; ii < count_allr; ii++ )
   {  // Loop to save to a new list only runs that had associated models
      runid      = runIDs[ ii ];
      if ( rmknts[ ii ] > 0 )
      {
         count_list++;         // Bump count of runs to be listed
DbgLv(1) << "ScDB:   ii count_list" << ii << count_list
   << "models" << rmknts[ii] << "run" << runid;
         rlabels << runid;     // Save run ID to list of selectable runs
      }
   }
DbgLv(1) << "ScDB:count_list" << count_list;
}


// Scan local disk for edit sets
void US_SelectRunid::scan_local_runs( void )
{
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
               QString mGUID = attr.value( "modelGUID"   ).toString();
               int     kk    = mdesc.lastIndexOf( ".model" );
                       mdesc = ( kk < 1 ) ? mdesc : mdesc.left( kk );
               QString runid = mdesc.section( ".", 0, -3 );
               // Skip the model if it has no valid runID part
               if ( runid.isEmpty() || runid.length() < 2 )  continue;

               // Save run ID and model string of RunID+GUID+Description
               QString odesc  = runid + "\t" + mGUID + "\t" + mdesc;
               mRunIDs << runid;
               mDescrs << odesc;
if((dbg_level>0) && (!mdesc.contains("-MC_0")||mdesc.contains("_mc0001")))
 DbgLv(1) << "ScLo: odesc" << odesc;
            }

            if ( xml.name() == "analyte" )
               break;
         }
      }
   }

   QString     rdir    = US_Settings::resultDir();
   QStringList aucdirs = QDir( rdir ).entryList( 
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
DbgLv(1) << "ScLo:rdir" << rdir << "aucdir count" << aucdirs.count();
   
   QStringList aucfilt( "*.auc" );
   QStringList datfilt;
   count_allr = 0;
   count_list = 0;
   count_seld = 0;
   
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
      count_allr++;             // Bump the count of all runIDs examined

      if ( mRunIDs.contains( runID ) )
      {  // If this runID is represented for models, it is selectable
         count_list++;          // Bump the count of runIDs to list
         rlabels << runID;      // Save selectable runID
      }
DbgLv(1) << "ScLo:    count_allr" << count_allr << "count_list" << count_list
 << "   runID" << runID;
   }
DbgLv(1) << "ScLo:rlabels count" << count_list << rlabels.count();
}


// Investigator button clicked:  get investigator from dialog
void US_SelectRunid::get_person()
{
   int invID     = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect( dialog, SIGNAL( investigator_accepted( int ) ),
                    SLOT(   update_person(         int ) ) );

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_SelectRunid::update_person( int ID )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_invest->setText( number + US_Settings::us_inv_name() );

   list_data();
}

// Slot to update disk/db selection
void US_SelectRunid::update_disk_db( bool isDB )
{
   emit changed( isDB );

   sel_db     = isDB;
   list_data();

   pb_invest->setEnabled( isDB );
   setWindowTitle( tr( "Select Run ID(s) for Discrete Distributions (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
}

// Slot to record a change in list item selection
void US_SelectRunid::selectionChanged()
{
   count_seld = lw_data->selectedItems().size();

   te_status->setText(
      tr( "%1 scanned run IDs were used to derive the list. Of these,\n"
          "%2 have associated discrete distribution data (models).\n"
          "%3 %4 currently selected for combination plot components." )
      .arg( count_allr ).arg( count_list ).arg( count_seld )
      .arg( count_seld > 1 ? tr( "runs are" ) : tr( "run is" ) ) );
}

