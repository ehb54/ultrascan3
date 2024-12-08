//! \file us_get_run.cpp

#include "us_get_run.h"
#include "us_experiment.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_util.h"
#include "us_investigator.h"

// Primary constructor to establish the dialog
US_GetRun::US_GetRun( QString& run, bool inDB ) 
: US_WidgetsDialog( 0, 0 ), runID( run )
{
   int ddstate;
   if ( inDB )
   {
      setWindowTitle( tr( "Available US3 Runs in DB" ) );
      ddstate        = US_Disk_DB_Controls::DB;
   }
   else
   {
      setWindowTitle( tr( "Available US3 Runs on Local Disk" ) );
      ddstate        = US_Disk_DB_Controls::Disk;
   }

   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   dkdb_cntrls       = new US_Disk_DB_Controls( ddstate );
   connect( dkdb_cntrls, SIGNAL( changed       ( bool ) ),
            this,        SLOT  ( update_disk_db( bool ) ) );

   // Investigator
   personID          = US_Settings::us_inv_ID();
   QHBoxLayout* investigator = new QHBoxLayout;
                pb_invest    = us_pushbutton( tr( "Select Investigator" ) );
   if ( US_Settings::us_inv_level() < 3 )
      pb_invest->setEnabled( false );
   investigator->addWidget( pb_invest );
   QString invval    = QString::number( personID ) + ": " +
                       US_Settings::us_inv_name();
                le_invest    = us_lineedit( invval, 1, true );
   investigator->addWidget( le_invest );
   connect( pb_invest, SIGNAL( clicked         () ),
            this,      SLOT  ( sel_investigator() ) );

   // Search
   QHBoxLayout* search       = new QHBoxLayout;
   QLabel*      lb_search    = us_label( tr( "Search" ) );
                le_search    = us_lineedit( "" );
   search      ->addWidget( lb_search );
   search      ->addWidget( le_search );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
            this,      SLOT  ( limit_data ( const QString& ) ) );

   // Load the runInfo structure with current data
   if ( inDB )
      load_db();
   else
      load_disk();

   // Tree
   tw                     = new QTableWidget( runInfo.size(), 4, this );
   populate_list();

   // Button Row
   QHBoxLayout* buttons   = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_delete = us_pushbutton( tr( "Delete" ) );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( deleteRun() ) );
   buttons->addWidget( pb_delete );

   QPushButton* pb_accept = us_pushbutton( tr( "Select" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( select() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( dkdb_cntrls );
   main->addLayout( investigator );
   main->addLayout( search );
   main->addWidget( tw );
   main->addLayout( buttons );
qDebug() << "gDBr: size" << size();
   resize( 720, 440 );
qDebug() << "gDBr: size" << size();
}

// Function to load the runInfo structure with all runID's in the DB
void US_GetRun::load_db( void )
{
qDebug() << "LdDb: IN";
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Error making the DB connection.\n" ) );
      return;
   }

   // Get information we want about all experiments
   runInfo.clear();
   QStringList q( "get_experiment_desc" );
   q << QString::number( US_Settings::us_inv_ID() );
   db.query( q );

   while( db.next() )
   {
      RunInfo rr;
      rr.ID     = db.value( 0 ).toString().toInt();
      rr.runID  = db.value( 1 ).toString();
      rr.label  = db.value( 4 ).toString();
      rr.date   = US_Util::toUTCDatetimeText( db.value( 5 )
                  .toDateTime().toString( Qt::ISODate ), true )
                  .section( " ", 0, 0 ).simplified();
//qDebug() << "expID" << expID << "ID date runID label"
// << rr.ID << rr.date << rr.runID << rr.label;

      runInfo << rr;
   }

   if ( runInfo.size() < 1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "There are no US3 runs in the DB to load.\n" ) );
   }

   return;
}

// Function to load the runInfo structure with all runID's on local disk
void US_GetRun::load_disk( void )
{
   QString resdir    = US_Settings::resultDir();
   QStringList rdirs = QDir( resdir ).entryList(
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
qDebug() << "LdDk:  rdirs count" << rdirs.count();
   resdir            = resdir + "/";
   QStringList efilt;
   efilt << "*.auc" << "*.??.xml";
   QStringList runids;

   // Get the list of all Run IDs with data in their work directories
   for ( int ii = 0; ii < rdirs.count(); ii++ )
   {
      QString     wdir   = resdir + rdirs[ ii ];
      QStringList efiles = QDir( wdir ).entryList( efilt, QDir::Files,
                                                   QDir::Name );

      if ( efiles.count() < 1 )
         continue;

      runids << rdirs[ ii ];
//qDebug() << "LdDk:   ii" << ii << "run" << rdirs[ii];
   }
qDebug() << "LdDk:  runids count" << runids.count();

   runInfo.clear();

   // Now fill in the runInfo for those runs
   for ( int ii = 0; ii < runids.count(); ii++ )
   {
      QString     runID  = runids[ ii ];
      QString     wdir   = resdir + runID;
      efilt.clear();
      efilt << runID + ".??.xml";
      QStringList efiles = QDir( wdir ).entryList( efilt, QDir::Files,
                                                   QDir::Name );

//qDebug() << "LdDk:   ii" << ii << "ef" << efilt << "efcount" << efiles.count()
// << "  run" << runID;
      if ( efiles.count() != 1 )
         continue;

      QString     rfn    = wdir + "/" + efiles[ 0 ];
      QString     expID;
      QString     label;
      QString     date   = US_Util::toUTCDatetimeText(
                              QFileInfo( rfn ).lastModified().toUTC()
                              .toString( Qt::ISODate ), true )
                              .section( " ", 0, 0 ).simplified();

//qDebug() << "LdDk:   ii" << ii << "  rfn" << rfn;
      QFile xfi( rfn );
      if ( ! xfi.open( QIODevice::ReadOnly | QIODevice::Text ) )
         continue;

      QXmlStreamReader xml( &xfi );

      while ( ! xml.atEnd() )
      {  // Read experiment elements
         QString xname;
         QString attv;
         QXmlStreamAttributes attr;

         xml.readNext();

         if ( xml.isStartElement() )
         {
            xname          = xml.name().toString();

            if ( xname == "experiment" )
            {
               attr           = xml.attributes();
               expID          = attr.value( "id" ).toString();
            }

            else if ( xname == "label" )
            {
               xml.readNext();
               label          = xml.text().toString();
            }
         }
      }

      RunInfo rr;
      rr.ID     = expID.toInt();
      rr.date   = date;
      rr.runID  = runID;
      rr.label  = label;
//qDebug() << "LdDk:   ii" << ii << "   expID" << expID << "ID date runID label"
// << rr.ID << rr.date << rr.runID << rr.label;

      runInfo << rr;
   }

   if ( runInfo.size() < 1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "There are no US3 runs on the local Disk to load.\n" ) );
   }

   return;
}

// Function to pass information back when select button is pressed
void US_GetRun::select( void )
{
   int ndx = tw ->currentRow();

   runID = tw ->item( ndx, 0 )->text();
qDebug() << "GetRun:  accept : runID" << runID;
   accept();
}

//Check if the run [filename] a part of the GMP framework
bool US_GetRun::check_filename_for_autoflow( QString rFilename )
{
  bool isRequired = false;
  int  autoflowNumber = 0;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );

  QStringList q;
  q  << QString( "check_filename_for_autoflow" ) //for now, both GMP and R&D
     << rFilename;

  autoflowNumber  = db -> functionQuery( q );

  (autoflowNumber > 0) ? isRequired = true : isRequired = false;

  qDebug() << "in check_filename_for_autoflow(): autoflowNumber, isRequired -- "
	   << autoflowNumber << isRequired;
  
  return isRequired;
}


// Function to delete the highlighted run when delete button is pressed
void US_GetRun::deleteRun( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "Error making the DB connection.\n" ) );
      return;
   }

   //first, check if the run belongs / was generated to/by GMP framework:
   int ndx_1 = tw ->currentRow();
   QString rFilename = tw ->item( ndx_1, 0 )->text().simplified();
   QString rExpID = tw ->item( ndx_1, 2 )->text().simplified();
   qDebug() << "GetRun::check for GMP: ndx_1 -- " << ndx_1
	    << ", rFilename -- " << rFilename
	    << ", expID  -- " << rExpID;

   bool isRequired = check_filename_for_autoflow( rFilename );
   if ( isRequired )
     {
       QMessageBox::information( this, tr( "Data Cannot be Deleted" ),
   				 tr( "The Data for the Run:\n\n"
   				     "\"%1\"\n\n"
   				     "can NOT be deleted since it is required by the GMP framework!" )
   				 .arg( rFilename ) );
    
       return;
     }
   // End check for GMP ///////////////////////////////////////////////

   
   int status = QMessageBox::information( this,
      tr( "Warning" ),
      tr( "Are you sure you want to delete this run from the DB? " ) +
      tr( "This action is not reversible. Proceed? "               ),
      tr( "&OK" ), tr( "&Cancel" ),
      0, 0, 1 );
   if ( status != 0 ) return;

   int ndx = tw ->currentRow();
   QString expID = tw ->item( ndx, 2 )->text().simplified();
qDebug() << "GetRun:delRun: ndx" << ndx << "expID" << expID;

   // Let's make sure it's not a calibration experiment in use
   QStringList q( "count_calibration_experiments " );
   q << expID;
   int count = db.functionQuery( q );
qDebug() << "GetRun:delRun: calexp count" << count;

   if ( count < 0 )
   {
      qDebug() << "count_calibration_experiments( "
               << expID
               << " ) returned a negative count";
      return;
   }

   else if ( count > 0 )
   {
      QMessageBox::information( this,
            tr( "Error" ),
            tr( "Cannot delete an experiment that is associated "
                "with a rotor calibration\n" ) );
      return;
   }

   // Delete links between experiment and solutions
   q.clear();
   q << "delete_experiment_solutions"
     << expID ;
   status = db.statusQuery( q );
qDebug() << "GetRun:delRun: del sols status" << status;

   // Same with cell table
   q.clear();
   q  << "delete_cell_experiments"
      << expID ;
   status = db.statusQuery( q );
qDebug() << "GetRun:delRun: del cells status" << status;

   // Let's delete any pcsa_modelrecs records to avoid
   //  constraints problems
   QString invID = QString::number( personID );
   QString runID = tw ->item( ndx, 0 )->text();
qDebug() << "GetRun:delRun:  invID" << invID << "runID" << runID;

   US_Experiment::deleteRunPcsaMrecs( &db, invID, runID );

   // Now delete the experiment and all existing rawData, 
   // because we're starting over 
   q.clear();
   q << "delete_experiment"
     << expID ;
   status = db.statusQuery( q );
qDebug() << "GetRun:delRun: del_exp stat" << status;

   if ( status != US_DB2::OK )
   {
      QMessageBox::information( this,
            tr( "Error / Warning" ),
            db.lastError() + tr( " (error=%1, expID=%2)" )
            .arg( status ).arg( expID ) );
   }

   // Delete table widget row for removed run
   tw->removeRow( ndx );
}

// Function to populate the data tree
void US_GetRun::populate_list()
{
   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics( tw_font );
   int rowht        = fm->height() + 2;
//   tw               = new QTableWidget( runInfo.size(), 4, this );
   tw->setFont   ( tw_font );
   tw->setPalette( US_GuiSettings::editColor() );
   tw->setRowCount( runInfo.count() );

   QStringList headers;
   headers << "Run"
           << "Date"
           << "dbID"
           << "Label";

   tw->setHorizontalHeaderLabels( headers );
   tw->verticalHeader()->hide();
   tw->setShowGrid( false );
   tw->setSelectionBehavior( QAbstractItemView::SelectRows );
   tw->setMinimumWidth( 640 );
   tw->setMinimumHeight( 480 );
   tw->setColumnWidth( 0, 250 );
   tw->setColumnWidth( 1, 150 );
   tw->setColumnWidth( 2,  50 );
   tw->setColumnWidth( 3, 350 );
   tw->setSortingEnabled( false );
   tw->clearContents();

   // Now load the table, marking each as not-editable
   for ( int ii = 0; ii < runInfo.size(); ii++ )
   {
      QTableWidgetItem* item;
      RunInfo           rr   = runInfo[ ii ];

      item = new QTableWidgetItem( rr.runID );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 0, item );

      item = new QTableWidgetItem( rr.date );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 1, item );

      item = new QTableWidgetItem( QString().sprintf( "%6d", rr.ID ) );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 2, item );

      item = new QTableWidgetItem( rr.label );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 3, item );
//qDebug() << "setItems ii" << ii << "ID date runID label"
// << rr.ID << rr.date << rr.runID << rr.label;

      tw  ->setRowHeight( ii, rowht );
   }

   tw->setSortingEnabled( true );
   tw->sortByColumn( 1, Qt::DescendingOrder );
   tw->resizeColumnsToContents();
   tw->adjustSize();
   tw->resize( size().width() - 4, tw->size().height() );
   qApp->processEvents();
}

// Function to update the selection of disk or DB
void US_GetRun::update_disk_db( bool isDB )
{
qDebug() << "UpdDkDb: isDB" << isDB;
   if ( isDB )
   {
      load_db();
      setWindowTitle( tr( "Available US3 Runs in DB" ) );
   }
   else
   {
      load_disk();
      setWindowTitle( tr( "Available US3 Runs on Local Disk" ) );
   }

   populate_list();

   QString sfilt     = le_search->text();
   if ( ! sfilt.isEmpty() )
      limit_data( sfilt );

   emit dkdb_changed( isDB );
}

// Function to limit table data shown based on search criteria
void US_GetRun::limit_data( const QString& sfilt )
{
qDebug() << "LimData: sfilt" << sfilt;
   bool have_search = ! sfilt.isEmpty();
   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics( tw_font );
   int rowht        = fm->height() + 2;
   tw->clearContents();
   tw->setSortingEnabled( false );

   for ( int ii = 0; ii < runInfo.size(); ii++ )
   {
      QTableWidgetItem* item;
      RunInfo           rr     = runInfo[ ii ];

      // Skip the item if search text exists and the runID does not contain it
      if ( have_search  &&
          ! rr.runID.contains( sfilt, Qt::CaseInsensitive ) )
         continue;

      item = new QTableWidgetItem( rr.runID );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 0, item );

      item = new QTableWidgetItem( rr.date );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 1, item );

      item = new QTableWidgetItem( QString().sprintf( "%6d", rr.ID ) );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 2, item );

      item = new QTableWidgetItem( rr.label );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 3, item );
//qDebug() << "setItems ii" << ii << "ID date runID label"
// << rr.ID << rr.date << rr.runID << rr.label;

      tw  ->setRowHeight( ii, rowht );
   }

   tw->setSortingEnabled( true );
   tw->sortByColumn( 1, Qt::DescendingOrder );
   tw->resizeColumnsToContents();
   tw->adjustSize();
//   tw->update();
//   update();
   tw->resize( size().width() - 4, tw->size().height() );
   qApp->processEvents();
}

// Open the dialog to select a new investigator
void US_GetRun::sel_investigator( void )
{
   if ( !dkdb_cntrls->db() )  return;   // Ignore if Disk selected

   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog, SIGNAL( investigator_accepted( int ) ),
            this,       SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Assign this dialog's investigator text and re-populate tree
void US_GetRun::assign_investigator( int invID )
{
   personID       = invID;
   QString number = ( invID > 0 ) ? QString::number( invID ) + ": " : "";

   le_invest->setText( number + US_Settings::us_inv_name() );

   load_db();

   populate_list();
}

