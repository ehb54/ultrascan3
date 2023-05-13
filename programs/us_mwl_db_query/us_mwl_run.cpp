//! \file us_mwl_run.cpp

#include "us_mwl_run.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"

// Primary constructor to establish the dialog
US_MwlRun::US_MwlRun( QString& runID, bool isRawMwl ) 
: US_WidgetsDialog( 0, 0 ), runID( runID ), isRawMwl( isRawMwl )
{
   if ( isRawMwl )
   {
      setWindowTitle( tr( "Available Raw MWL Directories with .mwrs Files" ) );
   }
   else
   {
      setWindowTitle( tr( "Available US3 Directories with MWL .auc Files" ) );
   }

   setPalette( US_GuiSettings::frameColor() );

   runID             = "";
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   // Search
   QHBoxLayout* search       = new QHBoxLayout;
   QLabel*      lb_search    = us_label( tr( "Search" ) );
                le_search    = us_lineedit( "" );
   search      ->addWidget( lb_search );
   search      ->addWidget( le_search );
   connect( le_search, SIGNAL( textChanged( const QString& ) ),
            this,      SLOT  ( limit_data ( const QString& ) ) );

   // Load the runInfo structure with current data
   load_files();

   // Tree
   tw                     = new QTableWidget( runInfo.size(), 3, this );
   populate_list();

   // Button Row
   QHBoxLayout* buttons   = new QHBoxLayout;

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Select" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( select() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( search );
   main->addWidget( tw );
   main->addLayout( buttons );
qDebug() << "gDBr: size" << size();
//   resize( 120, 120 );
   resize( 480, 360 );
qDebug() << "gDBr: size" << size();
}

// Function to load the runInfo structure with all runID's on local disk
void US_MwlRun::load_files( void )
{
   QStringList efilt;

   if ( isRawMwl )
   {
      resdir = US_Settings::importDir();
      efilt << "*.mwrs";
   }
   else
   {
      resdir = US_Settings::resultDir();
      efilt << "*.auc";
   }
   QStringList rdirs = QDir( resdir ).entryList(
         QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
qDebug() << "LdDk:  rdirs count" << rdirs.count() << "resdir" << resdir;
qDebug() << "LdDk:  RawMwl" << isRawMwl << "efilt" << efilt << "rdirscount"
 << rdirs.count();
   resdir            = resdir + "/";
   QStringList runids;

   // Get the list of all Run IDs with data in their work directories
   for ( int ii = 0; ii < rdirs.count(); ii++ )
   {
      QString     runID  = rdirs[ ii ];
      QString     wdir   = resdir + runID;
      QStringList efiles = QDir( wdir ).entryList( efilt, QDir::Files,
                                                   QDir::Name );

      int         nfiles = efiles.count();
qDebug() << "LdDk:   ii" << ii << "run" << rdirs[ii] << "count" << nfiles;
      if ( nfiles < 2 )              // Definitely not MWL
         continue;

      if ( ! isRawMwl )
      {  // For US3 openAUC, count wavelengths
         QStringList wavelns;

         for ( int jj = 0; jj < nfiles; jj++ )
         {
            QString waveln = QString( efiles[ jj ] ).section( ".", -2, -2 );
            if ( ! wavelns.contains( waveln ) )
               wavelns << waveln;
            if ( wavelns.count() > 2 )  break;
         }
qDebug() << "LdDk:   ii" << ii << "run" << rdirs[ii] << "nwaveln"
 << wavelns.count();

         if ( wavelns.count() < 2 )  // Unique wavelengths count says non MWL
            continue;
      }

      QString     rfn    = wdir + "/" + efiles[ 0 ];
      QString     date   = US_Util::toUTCDatetimeText(
                              QFileInfo( rfn ).lastModified().toUTC()
                              .toString( Qt::ISODate ), true )
                              .section( " ", 0, 0 ).simplified();

//qDebug() << "LdDk:   ii" << ii << "  rfn" << rfn;
      RunInfo rr;
      rr.runID  = runID;
      rr.date   = date;
      rr.nfiles = nfiles;
//qDebug() << "LdDk:   ii" << ii << "     runID date count"
// << rr.runID << rr.date << rr.nfiles;

      runInfo << rr;
   }
/*
   if ( runInfo.size() < 1 )
   {
      QMessageBox::information( this,
             tr( "Error" ),
             tr( "There are no US3 runs on the local Disk to load.\n" ) );
   }
*/
   return;
}

// Function to pass information back when select button is pressed
void US_MwlRun::select( void )
{
   int ndx = tw ->currentRow();

   if ( ndx < 0 )
   {
      QMessageBox::information( this, tr( "No Run Selected" ),
             tr( "You have not selected a run to load."
                 " To cancel loading, click on the \"Cancel\" button."
                 " Otherwise, make a selection in the list before"
                 " clicking on the \"Select\" button" ) );
      return;
   }

   runID   = resdir + tw ->item( ndx, 0 )->text();
qDebug() << "MwlRun:  accept : runID" << runID;
   accept();
}

// Function to populate the data tree
void US_MwlRun::populate_list()
{
   QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
   QFontMetrics* fm = new QFontMetrics( tw_font );
   int rowht        = fm->height() + 2;
   tw->setFont   ( tw_font );
   tw->setPalette( US_GuiSettings::editColor() );
   tw->setRowCount( runInfo.count() );

   QStringList headers;
   headers << tr( "Run" )
           << tr( "Date" )
           << tr( "Count of " ) + ( isRawMwl ? ".mwrs" : ".auc" );

   tw->setHorizontalHeaderLabels( headers );
   tw->verticalHeader()->hide();
   tw->setShowGrid( false );
   tw->setSelectionBehavior( QAbstractItemView::SelectRows );
   tw->setMinimumWidth ( 100 );
   tw->setMinimumHeight( 100 );
   tw->setColumnWidth( 0, 250 );
   tw->setColumnWidth( 1, 150 );
   tw->setColumnWidth( 2,  50 );
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

      item = new QTableWidgetItem( QString().sprintf( "%6d", rr.nfiles ) );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 2, item );
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

// Function to limit table data shown based on search criteria
void US_MwlRun::limit_data( const QString& sfilt )
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

      item = new QTableWidgetItem( QString().sprintf( "%5d", rr.nfiles ) );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
      tw  ->setItem(  ii, 2, item );
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

