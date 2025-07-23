//! \file us_solution_gui.cpp

#include "us_solution_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_solution.h"


#if QT_VERSION < 0x050000
#define setSymbol(a)      setSymbol(*a)
#define setSamples(a,b,c) setData(a,b,c)
#endif

US_SolutionMgrSelect::US_SolutionMgrSelect( int *invID, int *select_db_disk,
      US_Solution *tmp_solution, int tmp_experimentID, int tmp_channelID ) : US_Widgets()
{
   solution     = tmp_solution;
   personID     = invID;
   db_or_disk   = select_db_disk;
   from_db      = ( (*db_or_disk) == 1 );
   dbg_level    = US_Settings::us_debug();
   experimentID = tmp_experimentID;
   channelID    = tmp_channelID;

   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept   = us_pushbutton( tr( "Accept" ) );
   pb_spectrum = us_pushbutton( tr( "View Spectrum" ) );
   pb_delete   = us_pushbutton( tr( "Delete Solution" ) );
   pb_help     = us_pushbutton( tr( "Help" ) );
   pb_upload   = us_pushbutton( tr( "Upload to the Database" ) );

   QLabel* bn_select     = us_banner( tr( "Select a solution to use" ) );
   bn_select->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   lw_solutions = us_listwidget();
   lw_solutions-> setSortingEnabled( true );


   pb_accept->setEnabled( false );
   pb_delete->setEnabled( false );
   pb_upload->setEnabled( false );
   //pb_info  ->setEnabled( false );

   QLabel* lb_search     = us_label( tr( "Search:" ) );
   le_search             = us_lineedit();

   //QLabel* lb_solutionDesc = us_label( tr( "Solution Name:" ) );
   //le_solutionDesc = us_lineedit( "", 1 );

   lb_amount = us_label( tr( "Analyte Molar Ratio:" ) );
   QLabel* lb_bufferInfo = us_label( tr( "Buffer Name:" ) );
   QLabel* lb_commonVbar20 = us_label( tr( "Common VBar (20C):" ) );
   QLabel* lb_density = us_label( tr( "Buffer density:" ) );
   QLabel* lb_viscosity = us_label( tr( "Buffer viscosity:" ) );
   QLabel* lb_storageTemp = us_label( tr( "Storage Temperature:" ) );


   le_amount = us_lineedit();
   us_setReadOnly( le_amount,   true );
   le_bufferInfo = us_lineedit();
   us_setReadOnly( le_bufferInfo,   true );
   le_commonVbar20 = us_lineedit();
   us_setReadOnly( le_commonVbar20,   true );
   le_density = us_lineedit();
   us_setReadOnly( le_density,     true );
   le_viscosity = us_lineedit();
   us_setReadOnly( le_viscosity,     true );
   le_storageTemp = us_lineedit();
   us_setReadOnly( le_storageTemp,     true );
   te_notes = us_textedit();
   te_notes->setMaximumSize( 600, 100 );
   te_notes->setReadOnly( true );

   QLabel* lb_banner3 = us_banner( tr( "Current solution contents" )  );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   QLabel* lb_banner4 = us_banner( tr( "Solution notes" )  );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   lw_analytes = us_listwidget();
   lw_analytes-> setSortingEnabled( true );
   connect( lw_analytes, SIGNAL( itemClicked  ( QListWidgetItem* ) ),   // To see Molar ratio for each analyte...
                         SLOT  ( selectAnalyte( QListWidgetItem* ) ) );
   //lw_analytes->setSelectionMode( QAbstractItemView::NoSelection );

   int row = 0;
   main->addWidget( bn_select,       row++,  0,  1, 12 );
   main->addWidget( lb_search,       row,    0,  1,  1 );
   main->addWidget( le_search,       row,    1,  1,  5 );
   main->addWidget( pb_help,         row,    6,  1,  2 );
   main->addWidget( pb_cancel,       row,    8,  1,  2 );
   main->addWidget( pb_accept,       row++, 10,  1,  2 );
   main->addWidget( lw_solutions,    row,    0, 10,  6 );
   main->addWidget( pb_spectrum,     row,    6,  1,  3 );
   main->addWidget( pb_delete,       row++,  9,  1,  3 );
   main->addWidget( pb_upload,       row++,  6,  1,  6 );

   main->addWidget( lb_banner4,      row++,  6,  1,  6 );
   main->addWidget( te_notes,        row++,  6,  1,  6 );
   main->addWidget( lb_banner3,      row++,  6,  1,  6 );
   main->addWidget( lw_analytes,     row++,  6,  4,  6 );

   row += 5;

   main->addWidget( lb_amount,       row,    0, 1,  2 );
   main->addWidget( le_amount,       row,    2, 1,  4 );
   main->addWidget( lb_bufferInfo,   row,    6, 1,  2 );
   main->addWidget( le_bufferInfo,   row++,  8, 1,  4 );
   main->addWidget( lb_commonVbar20, row,    0, 1,  2 );
   main->addWidget( le_commonVbar20, row,    2, 1,  4 );
   main->addWidget( lb_density,      row,    6, 1,  2 );
   main->addWidget( le_density,      row++,  8, 1,  4 );
   main->addWidget( lb_viscosity,    row,    0, 1,  2 );
   main->addWidget( le_viscosity,    row,    2, 1,  4 );
   main->addWidget( lb_storageTemp,  row,    6, 1,  2);
   main->addWidget( le_storageTemp,  row,    8, 1,  4);

   //connect( lw_analytes, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
   //                      SLOT  ( selectAnalyte( QListWidgetItem* ) ) );
   //connect( lw_analytes, SIGNAL( itemDoubleClicked  ( QListWidgetItem* ) ),
   //                      SLOT  ( changeAnalyte      ( QListWidgetItem* ) ) );

   connect( le_search,      SIGNAL( textChanged( const QString& ) ),
            this,           SLOT  ( search     ( const QString& ) ) );
   connect( lw_solutions, SIGNAL  ( itemClicked    ( QListWidgetItem* ) ),
            this,           SLOT  ( selectSolution ( QListWidgetItem* ) ) );
   connect( pb_help,        SIGNAL( clicked() ),
            this,           SLOT  ( help()    ) );
   connect( pb_cancel,      SIGNAL( clicked() ),
            this,           SLOT  ( reject()  ) );
   connect( pb_accept,      SIGNAL( clicked()       ),
            this,           SLOT  ( accept_solution() ) );
   connect( pb_delete,      SIGNAL( clicked() ),
            this,           SLOT  ( delete_solution() ) );
   connect( pb_spectrum,    SIGNAL( clicked()  ),
            this,           SLOT  ( spectrum() ) );
   connect( pb_upload,      SIGNAL( clicked()       ),
            this,           SLOT  ( upload_solution() ) );

   if ( US_Settings::us_inv_level() < 4 )
      pb_upload->setVisible( false );

   // Load the solution descriptions
   search("");
DbgLv(1) << "Before load: ";
   load();

   // Select the current one if we know what it is
   if ( solution->solutionID > 0 )
   {
      QList< QListWidgetItem* > items
        = lw_solutions->findItems( solution->solutionDesc, Qt::MatchExactly );

      // should be exactly 1, but let's make sure
      if ( items.size() == 1 )
      {
         selectSolution( items[ 0 ] );
         lw_solutions->setCurrentItem( items[ 0 ] );
      }
   }
}

// Display a spectrum dialog for list/manage
void US_SolutionMgrSelect::spectrum( void )
{
DbgLv(1) << solution->extinction;

  if (solution->extinction.isEmpty())
    {
      QMessageBox::information( this,
      tr( "WARNING" ),
      tr( "Solution does not have spectrum data!" ) );
    }
  else
    {
      US_ViewSpectrum *w = new US_ViewSpectrum(solution->extinction);
      w->setParent(this, Qt::Window);
      w->show();
    }
}


// Function to delete a solution from disk, db, or in the current form
void US_SolutionMgrSelect::delete_solution( void )
{
   int status = US_DB2::OK;

   if ( from_db )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      if ( solution->countInProtocols( &db ) > 0 )
      {
         status = US_DB2::SOLUT_IN_USE;
      }
      else
      {
         status = solution->deleteFromDB( &db );
#if 0
 QMessageBox::warning( this,
  tr( "Delete (DB) proceeds" ),
  tr( "Solution would be Deleted" ) );
#endif
      }
   }

   else
   {
      if ( solution->countInProtocols( NULL ) > 0 )
      {
         status = US_DB2::SOLUT_IN_USE;
      }
      else
      {
         status = solution->deleteFromDisk();
#if 0
 QMessageBox::warning( this,
  tr( "Delete (Disk) proceeds" ),
  tr( "Solution would be Deleted" ) );
#endif
      }
   }

   if ( status == US_DB2::SOLUT_IN_USE )
   {
      QMessageBox::warning( this,
         tr( "Delete aborted" ),
         tr( "Solution NOT Deleted, since it is in use\n"
             "by one or more experiments or protocols" ) );
      return;
   }
else if ( status != US_DB2::OK )
{
 QMessageBox::warning( this,
  tr( "Delete aborted (status=%1)" ).arg( status ),
  tr( "Solution NOT Deleted, since the delete\n"
      "attempt got a non-zero status." ) );
 return;
}

   solution->clear();
   analyteMap.clear();
   load();
   reset();

   QMessageBox::information( this,
         tr( "Delete results" ),
         tr( "Solution Deleted" ) );
   //changed = true;
}

// Function to load solutions into solutions list widget
void US_SolutionMgrSelect::load( void )
{
   from_db       = ( (*db_or_disk) == 1 );

   /*  In Buffers/Analytes - skipped here ... */
   QString sguid = solution->solutionGUID;
   int solid = solution->solutionID;
   //int idSol     = solid.toInt();

   if ( ( from_db  &&  solid < 0 )  ||
        ( !from_db  &&  sguid.isEmpty() ) )
   {
      lw_solutions->setCurrentRow( -1 );
   }
   /* */

DbgLv(1) << "From_DB: " << from_db;

   if ( from_db )
   {
      loadDB();
   }
   else
   {
      loadDisk();
   }

   if ( ! sguid.isEmpty() )
   {  // There is a selected solution, select a list item
      if ( from_db ) // DB access
      {
         // Search for solutionID
         for ( int ii = 0; ii < info.size(); ii++ )
         {
            if ( solid == info[ ii ].solutionID )
            {
//DbgLv(1) << "agS-initb:  ii" << ii << "match anaID"<< analyte->analyteID;
DbgLv(1) <<  "agS-initb:  ii" << ii << "match SolID"<< solution->solutionID;
               lw_solutions->setCurrentRow( ii );
               QListWidgetItem* item = lw_solutions->item( ii );
               //select_analyte( item );
               selectSolution( item );
               break;
            }
         }
      }

      else            // Disk access
      {
         // Search for GUID
         for ( int ii = 0; ii < info.size(); ii++ )
         {
            if ( sguid == info[ ii ].GUID )
            {
               lw_solutions->setCurrentRow( ii );
               QListWidgetItem* item = lw_solutions->item( ii );
               //select_analyte( item );
               selectSolution( item );
               break;
            }
         }
      }
   }
   else
   {  // There is no selected analyte, de-select a list item
      lw_solutions->setCurrentRow( -1 );

DbgLv(1) << "in load(): before RESET: ";
      reset();
   }

}

// Function to load solutions from disk
void US_SolutionMgrSelect::loadDisk( void )
{

DbgLv(1) << "load Disk";
  QString path;
   if ( ! solution->diskPath( path ) ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();
   le_search->  clear();
   le_search->  setReadOnly( true );

   QDir dir( path );
   QStringList filter( "S*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );

   QFile a_file;

   for ( int i = 0; i < names.size(); i++ )
   {
      a_file.setFileName( path + "/" + names[ i ] );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &a_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "solution" )
            {
               QXmlStreamAttributes a = xml.attributes();

               IDs          << a.value( "id" ).toString();
               GUIDs        << a.value( "guid" ).toString();
               filenames    << path + "/" + names[ i ];

            }

            else if ( xml.name() == "description" )
            {
               xml.readNext();
               descriptions << xml.text().toString();
            }
         }
      }

      a_file.close();
   }

   lw_solutions->clear();

   if ( descriptions.size() == 0 )
   {
      lw_solutions->setSelectionMode( QAbstractItemView::NoSelection );
      lw_solutions->addItem( "No solution files found." );
   }
   else
   {
      lw_solutions->setSelectionMode( QAbstractItemView::SingleSelection );
      le_search->setReadOnly( false );
      search();
   }

   //loadSolutions();
}

// Function to load solutions from db
void US_SolutionMgrSelect::loadDB( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );
DbgLv(1) << "loadDB: lasterr" << db.lastError();

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }
DbgLv(1) << "loadDB: AA";

   QString IDperson = QString::number( *personID );
   QStringList q;
   q << "all_solutionIDs" << IDperson;
   db.query( q );
DbgLv(1) << "loadDB: BB";

   // if ( investigatorID < 1 ) investigatorID = US_Settings::us_inv_ID();
   // QStringList q( "all_solutionIDs" );
   // q << QString::number( investigatorID );
   // db.query( q );

   IDs          .clear();
   descriptions .clear();
   GUIDs        .clear();
   filenames    .clear();
   lw_solutions->clear();
DbgLv(1) << "loadDB: CC";

   le_search->   clear();
   le_search->   setText( "" );
   le_search->   setReadOnly( true );

DbgLv(1) << "loadDB: lastErrno" << db.lastErrno() << US_DB2::OK << db.lastError();
   if ( db.lastErrno() == US_DB2::NOROWS )
   {
      lw_solutions->setSelectionMode( QAbstractItemView::NoSelection );
      lw_solutions->addItem( "No solution files found." );
   }
   else if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   while ( db.next() )
   {
      QString newID = db.value( 0 ).toString();
      IDs          << newID;
      descriptions << db.value( 1 ).toString();
      GUIDs        << QString( "" );
      filenames    << QString( "" );
   }
DbgLv(1) << "loadDB: descr size" << descriptions.size();

   // NEW from Analyte ////
   lw_solutions->clear();
   if ( descriptions.size() == 0 )
   {
      lw_solutions->setSelectionMode( QAbstractItemView::NoSelection );
      lw_solutions->addItem( "No solution files found." );
   }
   else
   {
      lw_solutions->setSelectionMode( QAbstractItemView::SingleSelection );
      le_search->setReadOnly( false );
DbgLv(1) << "loadDB: before search";
      search();
   }

   //loadSolutions();
}



//  will search solutions matching search string
 void US_SolutionMgrSelect::search( QString const& text )
 {

    QString sep = ";";
    QStringList sortdesc;
    lw_solutions  ->clear();
    info.clear();
    solutionMap.clear();

    sortdesc       .clear();
    int dsize   = descriptions.size();

    for ( int ii = 0; ii < descriptions.size(); ii++ )
    {  // get list of filtered-description + index strings
       if ( descriptions[ ii ].contains(
          QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) )  &&
          ! descriptions[ ii].isEmpty() )
       {
          sortdesc << descriptions[ ii ] + sep + QString::number( ii );
       }
    }

    // sort the descriptions
    sortdesc.sort();

DbgLv(1) << "Sol-search:  descsize" << dsize  << "sortsize" << sortdesc.size();

//DbgLv(1) << "BufS-search:  descsize" << dsize
// << "sortsize" << sortdesc.size();

    for ( int jj = 0; jj < sortdesc.size(); jj++ )
    {  // build list of sorted meta data and ListWidget entries

      int ii      = sortdesc[ jj ].section( sep, 1, 1 ).toInt();
      //int ii = jj;

      if ( ii < 0  ||  ii >= dsize )
       {
DbgLv(1) << "BufS-search:  *ERROR* ii" << ii << "jj" << jj
 << "sdesc" << sortdesc[jj].section(sep,0,0);
          continue;
       }

DbgLv(1) << "search 11";
       SolutionInfo si;
       si.index       = ii;
       si.description = descriptions[ ii ];
       si.GUID        = GUIDs       [ ii ];
       si.filename    = filenames   [ ii ];
       si.solutionID  = IDs         [ ii ].toInt();

       info << si;

DbgLv(1) << "search 22";
       QListWidgetItem* item = new QListWidgetItem( descriptions[ ii ], lw_solutions );
       solutionMap[ item ] = ii;

DbgLv(1) << "Search ii(ndx): " << ii;
       lw_solutions->addItem( item );
       //lw_solutions->addItem( si.description );
    }
 }



/*
// Function to load the solutions list widget from the solutions data structure
void US_SolutionMgrSelect::loadSolutions( void )
{
   lw_solutions->clear();
   info.clear();
   solutionMap.clear();

   for ( int i = 0; i < descriptions.size(); i++ )
   {
      SolutionInfo si;
      si.solutionID  = IDs         [ i ].toInt();
      si.description = descriptions[ i ];
      si.GUID        = GUIDs       [ i ];
      si.filename    = filenames   [ i ];
      si.index       = i;
      info << si;

      // Create a map to account for automatic sorting of the list
      QListWidgetItem* item = new QListWidgetItem( descriptions[ i ], lw_solutions );
      solutionMap[ item ] = i;

      lw_solutions->addItem( item );
   }

}
*/

// Function to display an error returned from the database
void US_SolutionMgrSelect::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}


// Reject the selected solutiom and return to caller with no change
void US_SolutionMgrSelect::reject( void )
{
   emit selectionCanceled();
}

// Accept the currently selected solutiom
void US_SolutionMgrSelect::accept_solution( void )
{
   emit solutionAccepted();
}


// Function to handle when analyte listwidget item is selected
void US_SolutionMgrSelect::selectSolution( QListWidgetItem* item )
{
   int     solutionID = 0;
   QString solutionGUID = "";
DbgLv(1) << "selecSol item: ";
   // Account for the fact that the list has been sorted
   int     ndx          = solutionMap[ item ];
DbgLv(1) << "selecSol 2  ndx: " << ndx;

   // Iterate over 'info' and check for info[i].index - should be equal to ndx //
   for ( int i = 0; i < info.size(); ++i )
   {
DbgLv(2) << "INFO_index: "<< info[i].index;
      if ( ndx == info[i].index )
      {
         solutionID   = info[ i ].solutionID;
         solutionGUID = info[ i ].GUID;
         break;
      }
   }

   //int     solutionID   = info[ ndx ].solutionID;
   //QString solutionGUID = info[ ndx ].GUID;

DbgLv(1) << "Inside Select: Extinction_1 " << solution->extinction.keys().count();

   solution->clear();
DbgLv(1) << "Inside Select: Extinction_2 " << solution->extinction.keys().count();

DbgLv(1) << "SolutionID: " << solutionID;

   int status = US_DB2::OK;

   if ( from_db )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      status = solution->readFromDB  ( solutionID, &db );

      // Error reporting
      if ( status == US_DB2::NO_BUFFER )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "The buffer this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }

      else if ( status == US_DB2::NO_ANALYTE )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "One of the analytes this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }

      else if ( status != US_DB2::OK )
         db_error( db.lastError() );

   }

   else
   {
      status = solution->readFromDisk( solutionGUID );

      // Error reporting
      if ( status == US_DB2::NO_BUFFER )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "The buffer this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }

      else if ( status == US_DB2::NO_ANALYTE )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "One of the analytes this solution refers to was not found.\n"
                   "Please restore and try again.\n" ) );
      }

      else if ( status != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "Disk Read Problem" ),
               tr( "Could not read data from the disk.\n"
                   "Disk status: " ) + QString::number( status ) );
      }
   }

   reset();
   changed = false;
}

// Upload the currently selected local solutiom
void US_SolutionMgrSelect::upload_solution( void )
{
DbgLv(1) << "UplSol: IN upload_solution";
   int status     = US_DB2::OK;
   QRegExp rx( "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$" );
   QString solutionGUID  = solution->solutionGUID;
DbgLv(1) << "UplSol: solutionGUID" << solutionGUID;
   solutionGUID   = ( !solutionGUID.isEmpty()  &&
                      rx.exactMatch( solutionGUID ) ) ?
                    solutionGUID : "";
DbgLv(1) << "UplSol: solutionGUID" << solutionGUID;

   // Cannot upload solution without a valid GUID
   if ( solutionGUID.isEmpty() )
   {
      qDebug() << "UplSol: ** invalid solution GUID"
               << solution->solutionGUID;
      return;
   }

   // Look for a solution in the DB with that GUID
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   status         = db.lastErrno();
   if ( status != US_DB2::OK )
   {  // Return with no upload if there are DB problems
      db_error( db.lastError() );
      return;
   }

   QStringList query( "get_solutionID_from_GUID" );
   query << solutionGUID;
   db.query( query );
   db.next();
   status         = db.lastErrno();

   if ( db.lastErrno() != US_DB2::NOROWS )
   {  // Return with no upload if the solution exists in the DB
      QMessageBox::warning( this,
         tr( "Solution Already in DB" ),
         tr( "The selected solution already exists\n"
             "in the database and cannot be overwritten" ) );
      qDebug() << "UplSol: ** pre-existing solution in the DB"
               << solutionGUID;
      return;
   }

   // We can add the solution to the database since the
   //  solution (it's GUID) was not found

   US_Solution solution_db  = *solution;
   solution_db.buffer.bufferID = "-1";  // Force skip of disk re-save

   status         = solution_db.saveToDB( experimentID, channelID, &db );

   if ( status == US_DB2::OK )
   {  // Report successful upload
      QMessageBox::information( this,
         tr( "Solution Upload Successful" ),
         tr( "The selected solution has been successfully\n"
             "uploaded to the database" ) );
      qDebug() << "UplSol: ++ Successful solution upload to the DB";
   }

   else
   {  // Report any problems uploading
      QMessageBox::critical( this,
         tr( "Solution Upload Error" ),
         tr( "The solution upload encountered an error:\n" )
            + db.lastError() + " " + status );
   }
}

// Function to refresh the display with values from the solution structure,
//  and to enable/disable features
void US_SolutionMgrSelect::reset( void )
{

DbgLv(1) << "We are at RESET: ";
   QList< US_Solution::AnalyteInfo >&   ai         = solution->analyteInfo;
   QString                              bufferDesc = solution->buffer.description;

   le_bufferInfo   -> setText( solution->buffer.description );
   //le_solutionDesc -> setText( solution->solutionDesc );
   le_commonVbar20 -> setText( QString::number( solution->commonVbar20 ) );
   le_density      -> setText( QString::number( solution->buffer.density ) );
   le_viscosity    -> setText( QString::number( solution->buffer.viscosity ) );
   le_storageTemp  -> setText( QString::number( solution->storageTemp  ) );
   te_notes        -> setText( solution->notes        );

   le_amount       -> setText( "1" );
   //le_guid         -> setText( solution->solutionGUID );

   //pb_buffer       -> setEnabled( true );

   //pb_addAnalyte   -> setEnabled( true );
   //pb_removeAnalyte-> setEnabled( false );

   // Let's calculate if we're eligible to save this solution
   //pb_save         -> setEnabled( false );

DbgLv(1) << "We are at RESET_1:  aInf count" << solution->analyteInfo.count();

   if ( ! bufferDesc.isEmpty() ) //we can have a solution with buffer only
   {
     //pb_save      -> setEnabled( true );
   }

   // Enable/disable delete/upload based on solution being selected
   bool list_sel  = ( lw_solutions->currentRow() != (-1) );
   pb_delete-> setEnabled( list_sel );
   pb_upload-> setEnabled( list_sel && !from_db );
DbgLv(1) << "pb_upload enable:" << ( list_sel && !from_db ) << list_sel << from_db;

   // Display analytes that have been selected
   lw_analytes->clear();
   analyteMap.clear();
   for ( int i = 0; i < ai.size(); i++ )
   {
      // Create a map to account for sorting of the list
      QListWidgetItem* item = new QListWidgetItem( ai[ i ].analyte.description, lw_analytes );
      analyteMap[ item ] = i;

      lw_analytes->addItem( item );
   }
DbgLv(1) << "We are at RESET_2:  aInf count" << solution->analyteInfo.count() << ai.size();

   // Turn the red label back
   //QPalette p = lb_amount->palette();
   //p.setColor( QPalette::WindowText, Qt::white );
   //lb_amount->setPalette( p );

   // Figure out if the accept button should be enabled
   if ( ! signal )      // Then it's just a close button
      pb_accept->setEnabled( true );

   else if ( solution->saveStatus == US_Solution::BOTH )
      pb_accept->setEnabled( true );

   // else if ( ( ! disk_controls->db() ) && solution->saveStatus == US_Solution::HD_ONLY )
   else if ( ( ! from_db ) && solution->saveStatus == US_Solution::HD_ONLY )
      pb_accept->setEnabled( true );

   //else if ( (   disk_controls->db() ) && solution->saveStatus == US_Solution::DB_ONLY )
   else if ( (  from_db ) && solution->saveStatus == US_Solution::DB_ONLY )
     pb_accept->setEnabled( true );

   else
      pb_accept->setEnabled( false );

   // // Display investigator
   // investigatorID = US_Settings::us_inv_ID();

   // QString number = ( investigatorID > 0 )
   //    ? QString::number( investigatorID ) + ": "
   //    : "";

   // le_investigator->setText( number +  US_Settings::us_inv_name() );
DbgLv(1) << "We are at RESET_9:  aInf count" << solution->analyteInfo.count() << ai.size();
}

// Function to handle when solution listwidget item is selected
void US_SolutionMgrSelect::selectAnalyte( QListWidgetItem* item )
{
qDebug() << "Sol:SselAna: IN";
   // Get the right index in the sorted list, and load the amount
qDebug() << "Sol:SselAna:  aMap count" << analyteMap.size();
   int ndx = analyteMap[ item ];
qDebug() << "Sol:SselAna:  ndx" << ndx;
qDebug() << "Sol:SselAna:  aInf count" << solution->analyteInfo.size();
qDebug() << "Sol:SselAna:  amt" << solution->analyteInfo[ ndx ].amount;
   le_amount ->setText( QString::number( solution->analyteInfo[ ndx ].amount ) );
qDebug() << "Sol:SselAna:  amt" << le_amount->text();

   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_amount->setPalette( p );
}




US_SolutionMgrNew::US_SolutionMgrNew( int *invID, int *select_db_disk,
      US_Solution *tmp_solution, int tmp_experimentID, int tmp_channelID ) : US_Widgets()
{
   solution     = tmp_solution;
   personID     = invID;
   db_or_disk   = select_db_disk;
   from_db      = ( (*db_or_disk) == 1 );
   dbg_level    = US_Settings::us_debug();
//   experimentID = tmp_experimentID;
   experimentID = 1;
   channelID    = tmp_channelID;

   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   QLabel* bn_newsolution    = us_banner( tr( "Create New Solution" ) );
   bn_newsolution  ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   lb_descrip   = us_label( tr( "Solution Name:" ) );
   le_descrip   = us_lineedit( "New Solution", 0, false );

   lb_bufferInfo   = us_label( tr( "Buffer Name:" ) );
   le_bufferInfo = us_lineedit();
   us_setReadOnly( le_bufferInfo,   true );

   pb_analyte = us_pushbutton( tr( "Add Analyte" ) );
   pb_buffer  = us_pushbutton( tr( "Select Buffer" ) );

   pb_removeAnalyte = us_pushbutton( tr( "Remove Analyte" ), false );

   pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );

   pb_cancel   = us_pushbutton( tr( "Cancel" ) );

   pb_reset   = us_pushbutton( tr( "Reset" ) );

   //pb_save     = us_pushbutton( tr( "Accept" ), false);
   //connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );

   pb_accept     = us_pushbutton( tr( "Accept" ), false);

   QHBoxLayout* lo_amount = new QHBoxLayout();

   lb_amount = us_label( tr( "Analyte Molar Ratio:" ) );
   lo_amount->addWidget( lb_amount );

   ct_amount = us_counter ( 2, 0, 100, 1 ); // #buttons, low, high, start_value
   ct_amount->setSingleStep( 1 );
   ct_amount->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );
   lo_amount->addWidget( ct_amount );


   QLabel* lb_banner3 = us_banner( tr( "Current solution contents" )  );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   lw_analytes = us_listwidget();
   lw_analytes-> setSortingEnabled( true );


   QLabel* lb_commonVbar20 = us_label( tr( "Common VBar (20C):" ) );
   QLabel* lb_density = us_label( tr( "Buffer density:" ) );
   QLabel* lb_viscosity = us_label( tr( "Buffer viscosity:" ) );
   QLabel* lb_storageTemp = us_label( tr( "Storage Temperature:" ) );


   le_commonVbar20 = us_lineedit();
   us_setReadOnly( le_commonVbar20,   true );
   le_density = us_lineedit();
   us_setReadOnly( le_density,     true );
   le_viscosity = us_lineedit();
   us_setReadOnly( le_viscosity,     true );
   le_storageTemp = us_lineedit();

   te_notes = us_textedit();
   //te_notes->setMaximumSize( 600, 100 );
   te_notes->setReadOnly( false );

   QLabel* lb_banner1 = us_banner( tr( "Solution notes" )  );
   lb_banner1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   int row = 0;
   main->addWidget( bn_newsolution,  row++,  0, 1, 12 );
   main->addWidget( lb_descrip,      row,    0, 1,  4 );
   main->addWidget( le_descrip,      row++,  4, 1,  8 );

   main->addWidget( lb_bufferInfo,   row,    0, 1,  2 );
   main->addWidget( le_bufferInfo,   row,    2, 1,  4 );

   main->addWidget( pb_analyte,      row,    6, 1,  3 );
   main->addWidget( pb_buffer,       row++,  9, 1,  3 );
   main->addLayout( lo_amount,       row,    0, 1,  6 );

   main->addWidget( pb_removeAnalyte,  row,    6, 1,  3 );
   main->addWidget( pb_spectrum,       row++,  9, 1,  3 );

   main->addWidget( lb_banner3,      row,    0, 1, 6 );

   main->addWidget( pb_cancel,       row,    6, 1,  2 );
   main->addWidget( pb_reset,        row,    8, 1,  2 );
   main->addWidget( pb_accept,       row++, 10, 1,  2 );

   main->addWidget( lw_analytes,     row,     0, 4, 6 );
   main->addWidget( lb_banner1,      row++,   6, 1, 6 );
   main->addWidget( te_notes,        row++,   6, 3, 6 );

   row += 7;

   main->addWidget( lb_commonVbar20, row,   0, 1, 2 );
   main->addWidget( le_commonVbar20, row,   2, 1, 4 );
   main->addWidget( lb_density,      row,   6, 1, 2 );
   main->addWidget( le_density,      row++, 8, 1, 4 );
   main->addWidget( lb_viscosity,    row,   0, 1, 2 );
   main->addWidget( le_viscosity,    row,   2, 1, 4 );
   main->addWidget( lb_storageTemp,  row,   6, 1, 2 );
   main->addWidget( le_storageTemp,  row,   8, 1, 4 );

   //main->setRowStretch( 5, 5 );
   connect( le_descrip, SIGNAL( editingFinished   () ), SLOT  ( new_description() ) );
   connect( pb_analyte, SIGNAL( clicked() ), SLOT( addAnalyte() ) );
   connect( pb_reset,   SIGNAL( clicked() ), this, SLOT  ( newSolution() ) );

   connect( lw_analytes, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                         SLOT  ( selectAnalyte( QListWidgetItem* ) ) );

   connect( pb_removeAnalyte, SIGNAL( clicked() ), SLOT( removeAnalyte() ) );

   connect( pb_cancel,   SIGNAL( clicked()     ),
            this,        SLOT  ( newCanceled() ) );
   connect( pb_buffer,   SIGNAL( clicked() ), SLOT( selectBuffer() ) );

   connect( pb_accept,   SIGNAL( clicked()     ),
            this,        SLOT  ( newAccepted() ) );

   connect( le_storageTemp, SIGNAL( textEdited      ( const QString&   ) ),
                            SLOT  ( saveTemperature ( const QString&   ) ) );

   connect( te_notes, SIGNAL( textChanged( void ) ),  SLOT  ( saveNotes  ( void ) ) );

   // connect( pb_spectrum,     SIGNAL( clicked() ),
   //          this,            SLOT  ( spectrum()    ) );
   connect( pb_spectrum,     SIGNAL( clicked() ),
            this,            SLOT  ( spectrum_class()    ) );


   newSolution();
}

void US_SolutionMgrNew::spectrum_class( void )
{
  US_NewSpectrum *w = new US_NewSpectrum("SOLUTION", le_descrip->text(), "", solution);
  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show();
}

// Function to save solution information to disk or db
void US_SolutionMgrNew::newAccepted()
{

   if ( le_storageTemp->text().isEmpty() )
      solution->storageTemp = 0;

   if ( from_db )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + db.lastError() );
         //db_error( db.lastError() );
         return;
      }
DbgLv(1) << "Inside newAccepted(): " << "ExpID: " << experimentID << ", cID: " << channelID;
      int status = solution->saveToDB( experimentID, channelID, &db );

      // if ( status != US_DB2::OK && ! display_status )  // then we return but no status msg
      //    return;

      if ( status == US_DB2::NO_BUFFER )
      {
         QMessageBox::warning( this,
               tr( "Solution DB Save Error" ),
               tr( "There was a problem saving the buffer to the database.\n" ) );
         return;
      }

      else if ( status == US_DB2::NOROWS )
      {
         QMessageBox::warning( this,
               tr( "Solution DB Save Error" ),
               tr( "A solution component is missing from the database, "
                   "and the attempt to save it failed.\n") );
         return;
      }

      else if ( status != US_DB2::OK )
      {
QString emsg = "solSv2DB: status=" + QString::number(status) + " " + QString::number(db.lastErrno())
 + " LErr=\"" + db.lastError() + "\"\n"
 + "expID,chnID " + QString::number(experimentID) + " " + QString::number(channelID);
         QMessageBox::warning( this,
               tr( "Solution DB Save Error" ),
//               db.lastError() );
               emsg );
         return;
      }
   }

   else
      solution->saveToDisk();

   emit newSolAccepted();
}



// Slot to capture new solution description
void US_SolutionMgrNew::new_description()
{
  solution->solutionDesc = le_descrip->text();

  bool can_accept = ( !le_descrip->text().isEmpty()  &&
                      !le_bufferInfo ->text().isEmpty() );
  pb_accept    ->setEnabled( can_accept );
  pb_spectrum  ->setEnabled( can_accept );
}

void US_SolutionMgrNew::newSolution( void )
{
   from_db       = ( (*db_or_disk) == 1 );

   solution->clear();
   analyteMap.clear();
   reset();
   //changed = true;
}

void US_SolutionMgrNew::reset( void )
{
   QList< US_Solution::AnalyteInfo >&   ai         = solution->analyteInfo;
   QString                              bufferDesc = solution->buffer.description;

   le_bufferInfo   -> setText( solution->buffer.description );
   le_descrip      -> setText( solution->solutionDesc );
   le_commonVbar20 -> setText( QString::number( solution->commonVbar20 ) );
   le_density      -> setText( QString::number( solution->buffer.density ) );
   le_viscosity    -> setText( QString::number( solution->buffer.viscosity ) );
   le_storageTemp  -> setText( QString::number( solution->storageTemp  ) );
   te_notes        -> setText( solution->notes        );
   //le_guid         -> setText( solution->solutionGUID );
   ct_amount       -> disconnect();
   ct_amount       -> setEnabled( false );
   ct_amount       -> setValue( 1 );

   pb_buffer       -> setEnabled( true );

   pb_analyte      -> setEnabled( true );
   pb_removeAnalyte-> setEnabled( false );
   pb_spectrum     -> setEnabled( false );

   // Let's calculate if we're eligible to save this solution
   pb_accept       -> setEnabled( false );
   if ( !le_descrip->text().isEmpty()  && ! bufferDesc.isEmpty() ) //we can have a solution with buffer only
   {
      pb_accept    -> setEnabled( true );
      pb_spectrum  -> setEnabled( true );
   }

   // Display analytes that have been selected
   lw_analytes->clear();
   analyteMap.clear();
   for ( int i = 0; i < ai.size(); i++ )
   {
      // Create a map to account for sorting of the list
      QListWidgetItem* item = new QListWidgetItem( ai[ i ].analyte.description, lw_analytes );
      analyteMap[ item ] = i;

      lw_analytes->addItem( item );
   }

   // Turn the red label back
   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::white );
   lb_amount->setPalette( p );
}


// Function to add analyte to solution
void US_SolutionMgrNew::addAnalyte( void )
{
   int dbdisk = ( from_db ) ? US_Disk_DB_Controls::DB
                            : US_Disk_DB_Controls::Disk;

   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( true, QString(), dbdisk );

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   // connect( analyte_dialog, SIGNAL( use_db        ( bool ) ),
   //                          SLOT  ( update_disk_db( bool ) ) );

   analyte_dialog->exec();
   qApp->processEvents();
   //changed = true;

}

// Get information about selected analyte
void US_SolutionMgrNew::assignAnalyte( US_Analyte data )
{
   US_Solution::AnalyteInfo newInfo;
   newInfo.analyte = data;
   newInfo.amount  = 1;

   if ( from_db )
   {
      // Now double-check analyteID from db if we can
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() == US_DB2::OK )
      {
         QStringList q( "get_analyteID" );
         q << newInfo.analyte.analyteGUID;
         db.query( q );

         if ( db.next() )
            newInfo.analyte.analyteID = db.value( 0 ).toString();
      }
   }

   // Make sure item has not been added already
   // Check manually because the amounts could be different, but it's still
   // the same analyte
   bool found = false;
   foreach ( US_Solution::AnalyteInfo itemInfo, solution->analyteInfo )
   {
      if ( itemInfo.analyte == newInfo.analyte )
      {
         found = true;
         break;
      }
   }

   if ( found )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Your solution already contains this analyte\n"
                "If you wish to change the amount, remove it and "
                "add it again.\n" ) );
      return;
   }

   solution->analyteInfo << newInfo;

   calcCommonVbar20();

   // We're maintaining a map to account for automatic sorting of the list
   QListWidgetItem* item = new QListWidgetItem( newInfo.analyte.description, lw_analytes );
   analyteMap[ item ] = solution->analyteInfo.size() - 1;      // The one we just added

   reset();
   //changed = true;
}

// Slot to cancel edited analyte
void US_SolutionMgrNew::newCanceled()
{
    emit newSolCanceled();
}

// Function to calculate the default commonVbar20 value
void US_SolutionMgrNew::calcCommonVbar20( void )
{
   solution->commonVbar20 = 0.0;

   if ( solution->analyteInfo.size() == 1 )
      solution->commonVbar20 = solution->analyteInfo[ 0 ].analyte.vbar20;

   else     // multiple analytes
   {
      double numerator   = 0.0;
      double denominator = 0.0;
      foreach ( US_Solution::AnalyteInfo ai, solution->analyteInfo )
      {
         numerator   += ai.analyte.vbar20 * ai.analyte.mw * ai.amount;
         denominator += ai.analyte.mw * ai.amount;
      }

      solution->commonVbar20 = ( denominator == 0 ) ? 0.0 : ( numerator / denominator );

   }
   //changed = true;
}

// Function to handle when solution listwidget item is selected
void US_SolutionMgrNew::selectAnalyte( QListWidgetItem* item )
{
qDebug() << "Sol:NselAna: IN";
   // Get the right index in the sorted list, and load the amount
qDebug() << "Sol:NselAna:  aMap count" << analyteMap.size();
   int ndx = analyteMap[ item ];
qDebug() << "Sol:NselAna:  ndx" << ndx;
   ct_amount ->setValue( solution->analyteInfo[ ndx ].amount );

   // Now turn the label red to catch attention
   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_amount->setPalette( p );

   pb_removeAnalyte ->setEnabled( true );
   ct_amount        ->setEnabled( true );
   connect( ct_amount, SIGNAL( valueChanged ( double ) ),      // if the user has changed it
                       SLOT  ( saveAmount   ( double ) ) );
   //changed = true;
}

// Function to update the amount that is associated with an individual analyte
void US_SolutionMgrNew::saveAmount( double amount )
{
   // Get the right index in the sorted list of analytes
   QListWidgetItem* item = lw_analytes->currentItem();

   // if item not selected return

   int ndx = analyteMap[ item ];
   solution->analyteInfo[ ndx ].amount = amount;

   calcCommonVbar20();

   // Update commonVbar20 value in GUI
   le_commonVbar20 -> setText( QString::number( solution->commonVbar20 ) );
   //changed = true;
}

// Function to update the storage temperature associated with the current solution
void US_SolutionMgrNew::saveTemperature( const QString& )
{
   solution->storageTemp = le_storageTemp ->text().toDouble();
   //changed = true;
}

// Function to update the notes associated with the current solution
void US_SolutionMgrNew::saveNotes( void )
{
   // Let's see if the notes have actually changed
   if ( solution->notes != te_notes->toPlainText() )
   {
      solution->notes        = te_notes        ->toPlainText();
   }
   //changed = true;
}

// Function to add analyte to solution
void US_SolutionMgrNew::removeAnalyte( void )
{
   // Allow for the fact that this listwidget is sorted
   QListWidgetItem* item = lw_analytes->currentItem();
   int ndx = analyteMap[ item ];

   solution->analyteInfo.removeAt( ndx );
   lw_analytes ->removeItemWidget( item );

   calcCommonVbar20();

   reset();
   //changed = true;
}

// Create a dialog to request a buffer selection
void US_SolutionMgrNew::selectBuffer( void )
{
   int dbdisk = ( from_db ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_BufferGui* buffer_dialog = new US_BufferGui( true,
                                                   solution->buffer, dbdisk );

   connect( buffer_dialog, SIGNAL( valueChanged ( US_Buffer ) ),
            this,          SLOT  ( assignBuffer ( US_Buffer ) ) );

   // connect( buffer_dialog, SIGNAL( use_db        ( bool ) ),
   //                         SLOT  ( update_disk_db( bool ) ) );

   buffer_dialog->exec();
   qApp->processEvents();
   //changed = true;
}

// Get information about selected buffer
void US_SolutionMgrNew::assignBuffer( US_Buffer newBuffer )
{
   if ( from_db )
   {
      // Now get the corresponding bufferID, if we can
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() == US_DB2::OK )
      {
         QStringList q( "get_bufferID" );
         q << newBuffer.GUID;
         db.query( q );

         if ( db.next() )
            newBuffer.bufferID = db.value( 0 ).toString();

         else
            newBuffer.bufferID = QString( "-1" );

      }
   }

   solution->buffer = newBuffer;

   reset();
   if (!le_descrip->text().isEmpty() )
     pb_spectrum -> setEnabled( true );
   //changed = true;
}






// Edit Existing Solution panel
US_SolutionMgrEdit::US_SolutionMgrEdit( int *invID, int *select_db_disk,
      US_Solution *tmp_solution, int tmp_experimentID, int tmp_channelID ) : US_Widgets()
{
   solution      = tmp_solution;
   orig_solution = *solution;
   personID    = invID;
   db_or_disk  = select_db_disk;
   from_db     = ( (*db_or_disk) == 1 );
   dbg_level   = US_Settings::us_debug();
   experimentID = tmp_experimentID;
   channelID  = tmp_channelID;

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept                = us_pushbutton( tr( "Accept" ) );
   QPushButton* pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QLabel* bn_modana        = us_banner( tr( "Edit an existing analyte" ) );
   QLabel* lb_descrip       = us_label( tr( "Description:" ) );
   le_descrip    = us_lineedit( solution-> solutionDesc );
   //us_setReadOnly( le_descrip, false );
   us_setReadOnly( le_descrip, true );

   QLabel* lb_storageTemp = us_label( tr( "Storage Temperature:" ) );
   le_storageTemp = us_lineedit();

   te_notes = us_textedit();
   //te_notes->setMaximumSize( 600, 100 );
   te_notes->setReadOnly( false );
   //connect( te_notes, SIGNAL( textChanged( void ) ),  SLOT  ( saveNotes  ( void ) ) );
   QLabel* lb_banner4 = us_banner( tr( "Solution comments" )  );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   int row = 0;
   main->addWidget( bn_modana,       row++, 0, 1, 12 );
   main->addWidget( lb_descrip,      row,   0, 1, 4 );
   main->addWidget( le_descrip,      row++, 4, 1, 8 );

   main->addWidget( lb_banner4,      row,   0, 1, 6 );
   main->addWidget( lb_storageTemp,  row,   6, 1, 2 );
   main->addWidget( le_storageTemp,  row++, 8, 1, 4 );

   main->addWidget( te_notes,        row,   0, 2, 6 );

   main->addWidget( pb_cancel,       row,   6, 1, 3 );
   main->addWidget( pb_accept,       row++, 9, 1, 3 );

   main->addWidget( pb_spectrum,     row,   6, 1, 3 );
   main->addWidget( pb_help,         row,   9, 1, 3 );

   row += 8;

   QLabel *empty = us_banner ("");
   main->addWidget( empty,           row,   0, 9, 12);

   connect( pb_spectrum, SIGNAL( clicked()  ),
            this,        SLOT  ( spectrum_class() ) );
   connect( pb_accept,   SIGNAL( clicked()  ),
            this,        SLOT  ( editAccepted() ) );
   connect( pb_cancel,   SIGNAL( clicked()  ),
            this,        SLOT  ( editCanceled() ) );
   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT  ( help()    ) );
   connect( le_storageTemp, SIGNAL( textEdited      ( const QString&   ) ),
                            SLOT  ( saveTemperature ( const QString&   ) ) );

   connect( te_notes,    SIGNAL( textChanged( void ) ),
            this,        SLOT  ( saveNotes  ( void ) ) );
   connect( le_descrip,  SIGNAL( editingFinished   () ),
            this,        SLOT  ( description() ) );
}

// Slot for manually changed description
void US_SolutionMgrEdit::description()
{
  if ( solution->solutionDesc != le_descrip->text() )
    {
      solution->solutionDesc = le_descrip->text();
      pb_accept->setEnabled( !le_descrip->text().isEmpty() );
    }
}

// Initialize solution settings, possibly after re-entry to Edit panel
void US_SolutionMgrEdit::init_solution( void )
{
   from_db     = ( (*db_or_disk) == 1 );
   orig_solution = *solution;
   le_descrip ->setText( solution->solutionDesc );
   //le_ph      ->setText( QString::number( buffer->pH ) );
   pb_accept  ->setEnabled( false );
   te_notes   -> setText( solution->notes );
   le_storageTemp -> setText( QString::number( solution->storageTemp  ) );

   edit_solution_description = le_descrip->text();
}

// Initialize analyte settings, possibly after re-entry to Edit panel
void US_SolutionMgrEdit::spectrum_class( void )
{
  QString ifexists;
  if (solution->extinction.isEmpty())
    ifexists = "NEW";
  else
    ifexists = "EXISTS";

  US_EditSpectrum *w = new US_EditSpectrum("SOLUTION", ifexists, le_descrip->text(), "1.000", solution);

  connect( w,     SIGNAL( change_spectrum( void ) ),
           this,  SLOT ( change_spectrum( void ) ) );
  connect( w,     SIGNAL( accept_enable( void ) ),
           this,  SLOT ( accept_enable( void ) ) );

  w->setParent(this, Qt::Window);
  w->setWindowModality(Qt::WindowModal);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show();
}

void US_SolutionMgrEdit::change_spectrum( void )
{
  emit editSolAccepted();
}

void US_SolutionMgrEdit::accept_enable( void )
{
  pb_accept->setEnabled( !le_descrip->text().isEmpty() );
}

void US_SolutionMgrEdit::editAccepted( void )
{
  if ( from_db )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Database Problem" ),
            tr( "Database returned the following error: \n" ) + db.lastError() );
         return;
      }
DbgLv(1) << "Inside editAccepted(): " << "ExpID: " << experimentID << ", cID: " << channelID;
      int status = solution->saveToDB( experimentID, channelID, &db );


      if ( status != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "Attention" ) ,
               db.lastError() );
         return;
      }

      else if ( status == US_DB2::NO_BUFFER )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "There was a problem saving the buffer to the database.\n" ) );
         return;
      }

      else if ( status == US_DB2::NOROWS )
      {
         QMessageBox::information( this,
               tr( "Attention" ) ,
               tr( "A solution component is missing from the database, "
                   "and the attempt to save it failed.\n") );
         return;
      }

   }

   else
      solution->saveToDisk();

  emit editSolAccepted();
}

void US_SolutionMgrEdit::editCanceled( void )
{
  emit editSolAccepted();
}

// Function to update the storage temperature associated with the current solution
void US_SolutionMgrEdit::saveTemperature( const QString& )
{
   solution->storageTemp = le_storageTemp ->text().toDouble();
   pb_accept -> setEnabled(true);
}

// Function to update the notes associated with the current solution
void US_SolutionMgrEdit::saveNotes( void )
{
   // Let's see if the notes have actually changed
   if ( solution->notes != te_notes->toPlainText() )
   {
      solution->notes        = te_notes   ->toPlainText();
      pb_accept -> setEnabled(true);
   }
}


///// Settings panel
US_SolutionMgrSettings::US_SolutionMgrSettings( int *invID, int *select_db_disk )
   : US_Widgets()
{
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

      QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";

   QLabel* lb_DB  = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   QPushButton* pb_help         = us_pushbutton( tr( "Help" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );

   QString number  = ( (*personID) > 0 )
      ? QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";

   le_investigator = us_lineedit( number + US_Settings::us_inv_name() );
   int idb_or_disk = from_db ? US_Disk_DB_Controls::DB
                             : US_Disk_DB_Controls::Disk;
   disk_controls   = new US_Disk_DB_Controls( idb_or_disk );
   QLabel *empty   = us_banner ("");

   lb_DB      ->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   us_setReadOnly( le_investigator, true );


   int row = 0;
   main->addWidget( lb_DB,           row++, 0, 1, 4 );
   main->addWidget( pb_investigator, row,   0, 1, 1 );
   main->addWidget( le_investigator, row++, 1, 1, 3 );
   main->addLayout( disk_controls,   row,   0, 1, 3 );
   main->addWidget( pb_help,         row++, 3, 1, 1 );
   main->addWidget( empty,           row,   0, 6, 4 );

   connect( disk_controls,   SIGNAL( changed   ( bool ) ),
            this,            SLOT  ( db_changed( bool ) ) );
   connect( pb_investigator, SIGNAL( clicked()          ),
            this,            SLOT(   sel_investigator() ) );
   connect( pb_help,         SIGNAL( clicked() ),
            this,            SLOT  ( help()    ) );

}
// Select a new investigator
void US_SolutionMgrSettings::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, (*personID) );

   connect( inv_dialog,
            SIGNAL( investigator_accepted( int ) ),
            SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Note and notify of change in db/disk source
void US_SolutionMgrSettings::db_changed( bool db )
{
   emit use_db( db );
   // calling class needs to query DB/disk when signal is emitted
   qApp->processEvents();
}

// Assign an investigator after a change
void US_SolutionMgrSettings::assign_investigator( int invID )
{
   (*personID) = invID;

   QString number = ( (*personID) > 0 )
   ? QString::number( invID ) + ": "
   : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
   emit investigator_changed( invID );
}



// Main Solution window with panels
US_SolutionGui::US_SolutionGui(
      int   expID,
      int   chID,
      bool  signal_wanted,
      int   select_db_disk,
      const US_Solution dataIn,
      bool  auto_save
      ) : US_WidgetsDialog( 0, 0 ), experimentID( expID ), channelID( chID ),
        signal( signal_wanted )

{
   personID      = US_Settings::us_inv_ID();
   solution      = dataIn;
   orig_solution = dataIn;
   autosave      = auto_save;
   disk_or_db    = ( select_db_disk == US_Disk_DB_Controls::Default )
                   ?  US_Settings::default_data_location()
                   : select_db_disk;
   dbg_level     = US_Settings::us_debug();


   setWindowTitle( tr( "Solution Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   this->setMinimumSize( 640, 480 );

   QGridLayout* main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   tabWidget   = us_tabwidget();
   selectTab   = new US_SolutionMgrSelect  ( &personID, &disk_or_db, &solution, experimentID, channelID );
   newTab      = new US_SolutionMgrNew     ( &personID, &disk_or_db, &solution, experimentID, channelID );
   editTab     = new US_SolutionMgrEdit    ( &personID, &disk_or_db, &solution, experimentID, channelID );
   settingsTab = new US_SolutionMgrSettings( &personID, &disk_or_db );
   tabWidget -> addTab( selectTab,   tr( "Select Solution" ) );
   tabWidget -> addTab( newTab,      tr( "Enter New Solution" ) );
   tabWidget -> addTab( editTab,     tr( "Edit Existing Solution" ) );
   tabWidget -> addTab( settingsTab, tr( "Settings" ) );

   main->addWidget( tabWidget );

   connect( tabWidget,   SIGNAL( currentChanged(       int  ) ),
            this,        SLOT (  checkTab(             int  ) ) );
   connect( selectTab,   SIGNAL( solutionAccepted(      void ) ),
            this,        SLOT (  solutionAccepted(      void ) ) );
   connect( selectTab,   SIGNAL( selectionCanceled(    void ) ),
            this,        SLOT (  solutionRejected(      void ) ) );
   connect( newTab,      SIGNAL( newSolAccepted(       void ) ),
            this,        SLOT (  newSolAccepted(       void ) ) );
   connect( newTab,      SIGNAL( newSolCanceled(       void ) ),
            this,        SLOT (  newSolCanceled(       void ) ) );
   connect( editTab,     SIGNAL( editSolAccepted(      void ) ),
            this,        SLOT (  editSolAccepted(      void ) ) );
   connect( editTab,     SIGNAL( editSolCanceled(      void ) ),
            this,        SLOT (  editSolCanceled(      void ) ) );
   connect( settingsTab, SIGNAL( use_db(               bool ) ),
            this,        SLOT (  update_disk_or_db(    bool ) ) );
   connect( settingsTab, SIGNAL( investigator_changed( int  ) ),
            this,        SLOT (  update_personID(      int  ) ) );
}

void US_SolutionGui::value_changed( const QString& )
{
   // This only is activated by changes to vbar20
   // (either protein or dna/rna) but vbar is not saved.
#if 0
   temp_changed( le_protein_temp->text() );
#endif
}

// React to a change in panel
void US_SolutionGui::checkTab( int currentTab )
{
DbgLv(1) << "ckTab: currTab" << currentTab;
   // Need to re-read the database or disk or to make other adjustments
   // in case relevant changes were made elsewhere
   if ( currentTab == 0 )
   {
DbgLv(1) << "ckTab:   selectTab  init_solution";
      selectTab  ->load();
   }
   else if ( currentTab == 1 )
   {
DbgLv(1) << "ckTab:   newTab     init_solution";
      newTab     ->newSolution();
   }
   else if ( currentTab == 2 )
   {
DbgLv(1) << "ckTab:   editTab    init_solution";
      editTab    ->init_solution();
      if ( editTab->edit_solution_description == "" )
      {
         QMessageBox::information( this,
            tr( "WARNING" ),
            tr( "No Solution selected! Please select Solution from the list" ) );

         emit editSolAccepted();
      }
   }
}

// Make global setting for change in db/disk in Settings panel
void US_SolutionGui::update_disk_or_db( bool choice )
{
   (choice) ? (disk_or_db = 1 ) : (disk_or_db = 0 );
}

// Global person ID after Settings panel change
void US_SolutionGui::update_personID( int ID )
{
   personID = ID;
}

// Slot for Edit panel solution-accepted
void US_SolutionGui::editSolAccepted( void )
{
DbgLv(1) << "main: editAnaAccepted";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for Edit panel solution-changes-rejected
void US_SolutionGui::editSolCanceled( void )
{
DbgLv(1) << "main: editAnaCanceled";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel solution accepted
void US_SolutionGui::newSolAccepted( void )
{
DbgLv(1) << "main: newAnaAccepted";
   tabWidget->setCurrentIndex( 0 );

   emit newSolAdded();
}

// Slot for New panel solution add rejected
void US_SolutionGui::newSolCanceled( void )
{
DbgLv(1) << "main: newAnaCanceled";
   tabWidget->setCurrentIndex( 0 );
}

// Exit and signal caller that changes and selected were accepted
void US_SolutionGui::solutionAccepted( void )
{
#if 0
   valueChanged      ( solution.density, solution.viscosity );
#endif
   emit valueChanged  ( solution );
   emit valueSolutionID( solution.solutionID );
   emit updateSolutionGuiSelection( solution );

DbgLv(1) << "Solution Accepted: ";
   accept();
}

// Exit and signal caller that solution selection/changes were rejected
void US_SolutionGui::solutionRejected( void )
{
   solution   = orig_solution;

   reject();
}

