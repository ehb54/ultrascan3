//! \file us_select_edits.cpp

#include "us_select_edits.h"
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

// Main constructor with flags for edit, latest-edit and local-data

US_SelectEdits::US_SelectEdits( bool dbase, bool& runsel, bool& late,
   QStringList& edIDs )
 : US_WidgetsDialog( 0, 0 ),
   sel_run    ( runsel ),
   sel_late   ( late ),
   editIDs    ( edIDs )
{
   sel_db        = dbase;

   setWindowTitle( tr( "Select Run/Edit as Models Pre-Filter (%1)" )
         .arg( sel_db ? "DB" : "Local" ) );
   setPalette    ( US_GuiSettings::frameColor() );
   setMinimumSize( 480, 300 );
qDebug() << "SE:sel_db" << sel_db;
qDebug() << "SE:sel_run" << sel_run;
qDebug() << "SE:sel_late" << sel_late;

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout;
   int row           = 0;

   QButtonGroup* selButtons = new QButtonGroup( this );
   QGridLayout* rb1 = us_radiobutton( tr( "by Run ID" ),
      rb_runid,   sel_run );
   QGridLayout* rb2 = us_radiobutton( tr( "by Latest Edit" ),
      rb_latest,  ( ! sel_run && sel_late ) );
   QGridLayout* rb3 = us_radiobutton( tr( "by Edit"   ),
      rb_alledit, ( ! sel_run && ! sel_late ) );
   selButtons->addButton( rb_runid,   0 );
   selButtons->addButton( rb_latest,  1 );
   selButtons->addButton( rb_alledit, 2 );
   top->addLayout( rb1, row,   0 );
   top->addLayout( rb2, row,   1 );
   top->addLayout( rb3, row++, 2 );
   connect( selButtons, SIGNAL( buttonClicked( int ) ),
                        SLOT  ( list_data()          ) );


   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xc0, 0xc0, 0xc0 ) );

   // Search line
   QLabel* lb_filtdata = us_label( tr( "Search" ) );
   top->addWidget( lb_filtdata, row, 0 );

   le_dfilter      = us_lineedit();
   top->addWidget( le_dfilter, row++, 1, 1, 2 );

   connect( le_dfilter,  SIGNAL( textChanged( const QString& ) ),
                         SLOT  ( search     ( const QString& ) ) );

   main->addLayout( top );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );

   // List widget to show data choices
   lw_data = new QListWidget( this );
   lw_data->setFrameStyle   ( QFrame::NoFrame );
   lw_data->setPalette      ( US_GuiSettings::editColor() );
   lw_data->setFont         ( font );
   lw_data->setSelectionMode( QAbstractItemView::ExtendedSelection );

   main->addWidget( lw_data );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancelled() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Select PreFilter(s)" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accepted() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // List from disk or db source
   list_data();
}

void US_SelectEdits::search( const QString& search_string )
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
void US_SelectEdits::list_data()
{
   if ( rb_runid->isChecked() )
   {
      sel_run   = true;
      sel_late  = false;
   }

   else if ( rb_latest->isChecked() )
   {
      sel_run   = false;
      sel_late  = true;
   }

   else if ( rb_alledit->isChecked() )
   {
      sel_run   = false;
      sel_late  = false;
   }

   editmap.clear();
   rlabels.clear();

   if ( sel_db )              // Scan database data
   {
      scan_dbase_edit();
   }
   else                       // Scan local disk data
   {
      scan_local_edit();
   }

   lw_data->clear();

   if ( sel_run )
   {
      if ( rlabels.size() == 0 )
      {
         QString clabel = tr( "No data found." );
         lw_data->addItem( new QListWidgetItem( clabel ) );
         return;
      }

      for ( int ii = 0; ii < rlabels.size(); ii++ )
      {  // Propagate list widget with labels
         QString  clabel  = rlabels.at( ii );

         lw_data->addItem( new QListWidgetItem( clabel ) );
      }
   }

   else
   {
      elabels                   = editmap.keys();
      QList< EditDesc > edescrs = editmap.values();

      if ( elabels.size() == 0 )
      {
         QString clabel = tr( "No data found." );
         lw_data->addItem( new QListWidgetItem( clabel ) );
         return;
      }

      for ( int ii = 0; ii < elabels.size(); ii++ )
      {  // Propagate list widget with labels
         QString  clabel  = elabels.at( ii );

         lw_data->addItem( new QListWidgetItem( clabel ) );
      }
   }
}

// Cancel button:  no editIDs returned
void US_SelectEdits::cancelled()
{
   editIDs.clear();
   reject();
   close();
}

// Accept button:  set up to return editID pre-filter information
void US_SelectEdits::accepted()
{
qDebug() << "SE:accepted()";

   editIDs.clear();

   QList< QListWidgetItem* > selitems = lw_data->selectedItems();

   if ( selitems.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "No Data Selected" ),
            tr( "You have not selected any data.\nSelect or Cancel" ) );
      return;
   }

   if ( sel_run )
   {  // Get and return editIDs from selected runID items
      elabels                   = editmap.keys();

      for ( int ii = 0; ii < selitems.size(); ii++ )
      {  // Loop thru selected runIDs
         QListWidgetItem* lwi_data = selitems.at( ii );
         QString          rlabel   = lwi_data->text() + " :";

         // Get a list of edit items whose label contains runID label
         QStringList      elabs    = elabels.filter( rlabel );

         for ( int jj = 0; jj < elabs.size(); jj++ )
         {  // Return editIDs from each edit in a run
            QString clabel = elabs[ jj ];

            editIDs << ( sel_db ? editmap[ clabel ].editID
                                : editmap[ clabel ].editGUID );
         }
      }
   }

   else
   {  // Get and return editIDs from selected edit items
      for ( int ii = 0; ii < selitems.size(); ii++ )
      {
         QListWidgetItem* lwi_data = selitems.at( ii );
         QString clabel = lwi_data->text();
qDebug() << "SE:  ii clabel" << ii << clabel;

         editIDs << ( sel_db ? editmap[ clabel ].editID
                             : editmap[ clabel ].editGUID );
      }
   }
qDebug() << "SE: editIDs" << editIDs;

   accept();        // Signal that selection was accepted
   close();
}

// Scan database for edit sets
void US_SelectEdits::scan_dbase_edit()
{
//qDebug() << "ScDB:TM:00: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
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

   query << "all_editedDataIDs" << invID;

//qDebug() << "ScDB:TM:01: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   db.query( query );

   while ( db.next() )
   {
      QString recID    = db.value( 0 ).toString();

//qDebug() << "ScDB:TM:03: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      QString descrip  = db.value( 1 ).toString();
      QString filename = db.value( 2 ).toString().replace( "\\", "/" );
      QString date     = US_Util::toUTCDatetimeText( db.value( 5 )
                         .toDateTime().toString( Qt::ISODate ), true );
      QString recGUID  = db.value( 9 ).toString();
      QString filebase = filename.section( "/", -1, -1 );
      QString runID    = descrip.isEmpty() ? filebase.section( ".", 0, -7 )
                         : descrip;
      QString editID   = filebase.section( ".", -6, -6 );
      QString tripID   = filebase.section( ".", -4, -2 );

//qDebug() << "ScDB:TM:04: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
      QString label    = runID + " : " + tripID + " : " + editID;

      edesc.label      = label;
      edesc.editID     = recID;
      edesc.editGUID   = recGUID;
      edesc.date       = date;

      editmap[ label ] = edesc;
//qDebug() << "ScDB:TM:06: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
   }
//qDebug() << "ScDB:TM:88: " << QTime::currentTime().toString("hh:mm:ss:zzzz");

   build_runids();

   if ( sel_late )
      pare_to_latest();
//qDebug() << "ScDB:TM:99: " << QTime::currentTime().toString("hh:mm:ss:zzzz");
}

// Scan local disk for edit sets
void US_SelectEdits::scan_local_edit( void )
{
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
      QString runID     = aucfbase.section( ".",  0, -6 );
      QString subType   = aucfbase.section( ".", -5, -5 );
      QString tripl     = aucfbase.section( ".", -4, -2 );

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
         QString runID    = filebase.section( ".",  0, -7 );
         QString editID   = filebase.section( ".", -6, -6 );
         editID  = ( editID.length() == 12  &&  editID.startsWith( "20" ) ) ?
                   editID.mid( 2 ) : editID;
         QString tripID   = filebase.section( ".", -4, -2 );
         QString label    = runID + " : " + tripID + " : " + editID;

         QFile filei( filename );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QString date = US_Util::toUTCDatetimeText( QFileInfo( filename )
            .lastModified().toUTC().toString( Qt::ISODate ), true );
         
         QXmlStreamReader xml( &filei );
         QXmlStreamAttributes a;
         QString recGUID;

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
            }
         }

         filei.close();

         edesc.label      = label;
         edesc.editID     = QString( "-1" );
         edesc.editGUID   = recGUID;
         edesc.date       = date;

         editmap[ label ] = edesc;
      }
   }

   build_runids();

   if ( sel_late )
      pare_to_latest();
}

// Build the runID list from full edit map
void US_SelectEdits::build_runids( void )
{
   elabels          = editmap.keys();
   rlabels.clear();

   for ( int ii = 0; ii < elabels.size(); ii++ )
   {
      QString clabel = elabels.at( ii ).section( ":", 0, 0 ).simplified();

      if ( ! rlabels.contains( clabel ) )
         rlabels << clabel;
   }
}


// Pare down data description map to only latest edit
void US_SelectEdits::pare_to_latest( void )
{
   for ( int kk = 0; kk < 2; kk++ )  // May need two passes to pare down
   {
      QStringList       keys = editmap.keys();
      QList< EditDesc > vals = editmap.values();
      int               kchg = 0;

      for ( int ii = 0; ii < keys.size() - 1; ii++ )
      {
         int jj = ii + 1;

         QString clabel = keys.at( ii );
         QString flabel = keys.at( jj );

         QString crunid = clabel.section( " : ", 0, -2 ).simplified();
         QString frunid = flabel.section( " : ", 0, -2 ).simplified();

         if ( crunid != frunid )
            continue;

         // This record's label differs from next only by edit code: remove it
         QString   cdtxt = vals.at( ii ).date;
         QString   fdtxt = vals.at( jj ).date;
         QDateTime cdate = QDateTime::fromString( cdtxt, Qt::ISODate );
         QDateTime fdate = QDateTime::fromString( fdtxt, Qt::ISODate );
//qDebug() << "PARE ii" << ii << "C,F date" << cdtxt << fdtxt;
//qDebug() << "  C,F lab" << clabel << flabel;
//qDebug() << "   (C<=F)" << (cdate<=fdate) << " C,F dt" << cdate << fdate;

         if ( cdate <= fdate )         // Remove the earlier of the two
         {
            editmap.remove( clabel );  //  Earlier is earlier in list
         }

         else
         {
            editmap.remove( flabel );  //  Earlier is later in list
            kchg++;                    //  Mark when early one later in list
         }
      }

//qDebug() << "PARE   kchg" << kchg << "kk" << kk;
      if ( kchg == 0 )   break;        // We're done

      // Need to repeat above when any removed was later in list
   }
}

