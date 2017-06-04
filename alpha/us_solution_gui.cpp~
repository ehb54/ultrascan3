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


US_SolutionMgrSelect::US_SolutionMgrSelect( int *invID, int *select_db_disk,
      US_Solution *tmp_solution ) : US_Widgets()
{
   solution    = tmp_solution;
   personID   = invID;
   db_or_disk = select_db_disk;
   from_db    = ( (*db_or_disk) == 1 );
   dbg_level  = US_Settings::us_debug();

   setPalette( US_GuiSettings::frameColor() );
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   pb_cancel   = us_pushbutton( tr( "Cancel" ) );
   pb_accept   = us_pushbutton( tr( "Accept" ) );
   pb_spectrum = us_pushbutton( tr( "View Spectrum" ) );
   pb_delete   = us_pushbutton( tr( "Delete Solution" ) );
   pb_help     = us_pushbutton( tr( "Help" ) );

   QLabel* bn_select     = us_banner( tr( "Select a solution to use" ) );
   bn_select->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   lw_solutions = us_listwidget();
   lw_solutions-> setSortingEnabled( true );

   pb_accept->setEnabled( false );
   pb_delete->setEnabled( false );
   //pb_info  ->setEnabled( false );
  
   QLabel* lb_search     = us_label( tr( "Search:" ) );
   le_search             = us_lineedit();

   //QLabel* lb_solutionDesc = us_label( tr( "Solution Name:" ) );
   //le_solutionDesc = us_lineedit( "", 1 );

   QLabel* lb_amount = us_label( tr( "Analyte Molar Ratio:" ) );
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
   //us_setReadOnly( le_storageTemp,     true );
   connect( le_storageTemp, SIGNAL( textEdited      ( const QString&   ) ),
                            SLOT  ( saveTemperature ( const QString&   ) ) );

   te_notes = us_textedit();
   te_notes->setMaximumSize( 600, 100 );
   te_notes->setReadOnly( false );
   //connect( te_notes, SIGNAL( textChanged( void ) ),  SLOT  ( saveNotes  ( void ) ) );
   

   QLabel* lb_banner3 = us_banner( tr( "Current solution contents" )  );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   QLabel* lb_banner4 = us_banner( tr( "Solution notes" )  );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   
   lw_analytes = us_listwidget();
   lw_analytes-> setSortingEnabled( true );
   lw_analytes->setSelectionMode( QAbstractItemView::NoSelection );
   
   int row = 0;
   main->addWidget( bn_select,       row++, 0, 1, 12 );
   main->addWidget( lb_search,       row,   0, 1, 1 );
   main->addWidget( le_search,       row,   1, 1, 5 );
   main->addWidget( pb_help,         row,   6, 1,  2 );
   main->addWidget( pb_cancel,       row,   8, 1,  2 );
   main->addWidget( pb_accept,       row++, 10, 1,  2 );
   main->addWidget( lw_solutions,    row,   0, 8, 6 );
   main->addWidget( pb_spectrum,     row,   6, 1,  3 );
   main->addWidget( pb_delete,       row++, 9, 1,  3 );

   main->addWidget( lb_banner4,      row++,   6, 1, 6 );
   main->addWidget( te_notes,        row++,   6, 1, 6 );
   main->addWidget( lb_banner3,      row++,   6, 1, 6 );
   main->addWidget( lw_analytes,     row++,   6, 4, 6 );

   row += 5;

   main->addWidget( lb_amount,       row,   0, 1, 2 );
   main->addWidget( le_amount,       row,   2, 1, 4 );
   main->addWidget( lb_bufferInfo,   row,   6, 1, 2 );
   main->addWidget( le_bufferInfo,   row++, 8, 1, 4 );
   main->addWidget( lb_commonVbar20, row,   0, 1, 2 );
   main->addWidget( le_commonVbar20, row,   2, 1, 4 );
   main->addWidget( lb_density,      row,   6, 1, 2 );
   main->addWidget( le_density,      row++, 8, 1, 4 );
   main->addWidget( lb_viscosity,    row,   0, 1, 2 );
   main->addWidget( le_viscosity,    row,   2, 1, 4 );
   main->addWidget( lb_storageTemp,  row,   6, 1, 2);
   main->addWidget( le_storageTemp,  row,   8, 1, 4);
   
   //for ( int ii = 0; ii < 12; ii++ )  main->setColumnStretch( ii, 99 );
   //for ( int ii = 0; ii < 12; ii++ )  main->setRowStretch( ii, 99 );

   //connect( lw_analytes, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
   //                      SLOT  ( selectAnalyte( QListWidgetItem* ) ) );
   //connect( lw_analytes, SIGNAL( itemDoubleClicked  ( QListWidgetItem* ) ),
   //                      SLOT  ( changeAnalyte      ( QListWidgetItem* ) ) );

   connect( le_search,      SIGNAL( textChanged( const QString& ) ),
	    this,           SLOT  ( search     ( const QString& ) ) );
   //connect( lw_solutions, SIGNAL( itemClicked    ( QListWidgetItem* ) ),
   //	                    SLOT  ( selectSolution ( QListWidgetItem* ) ) );
   connect( pb_help,        SIGNAL( clicked() ),
	    this,           SLOT  ( help()    ) );

   // connect( pb_cancel,      SIGNAL( clicked() ),
   //          this,           SLOT  ( reject()  ) );
   // connect( pb_accept,      SIGNAL( clicked()       ),
   //          this,           SLOT  ( accept() ) );

   // //connect( lw_solutions,   SIGNAL( itemSelectionChanged() ),
   // //         this,           SLOT  ( select_solution()        ) );
   
   // connect( pb_spectrum,    SIGNAL( clicked()  ),
   //          this,           SLOT  ( spectrum() ) );
   // connect( pb_delete,      SIGNAL( clicked()       ),
   //          this,           SLOT  ( delete_buffer() ) );
      
   //init_solution();

}

/*
void US_SolutionMgrSelect::selectSolution( QListWidgetItem* item )
{
   // Account for the fact that the list has been sorted
   int     ndx          = solutionMap[ item ];
   int     solutionID   = info[ ndx ].solutionID;
   QString solutionGUID = info[ ndx ].GUID;

   solution->clear();

   int status = US_DB2::OK;

   //if ( disk_controls->db() )
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

// Function to display an error returned from the database
void US_SolutionMgrSelect::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

void US_SolutionMgrSelect::init_solution( void )
{
 DbgLv(1) << "agS: init_a";
   from_db       = ( (*db_or_disk) == 1 );
 DbgLv(1) << "agS: init_a  from_db" << from_db << "dkordk" << *db_or_disk;
   QString aguid = solution->solutionGUID;
   //QString anaid = solution->solutionID;
   int anaid = solution->solutionID;
   int idAna     = anaid.toInt();
 DbgLv(1) << "agS-initb: db_or_disk" << *db_or_disk << "from_db" << from_db;
DbgLv(1) << "agS-initb:   anaID anaGUID" << anaid << aguid << idAna;

   if ( ( from_db  &&  idAna < 0 )  ||
        ( !from_db  &&  aguid.isEmpty() ) )
   {
      lw_solutions->setCurrentRow( -1 );
      idAna         = -1;
      //anaid         = "";
      aguid         = "";
   }

   query();
DbgLv(1) << "agS-initb:   post-q:anaID anaGUID" << anaid << aguid;

   if ( ! aguid.isEmpty() )
   {  // There is a selected solution, select a list item
      if ( from_db ) // DB access
      {
         // Search for solutionID
         for ( int ii = 0; ii < info.size(); ii++ )
         {
            if ( anaid == info[ ii ].solutionID )
            {
DbgLv(1) << "agS-initb:  ii" << ii << "match anaID"<< solution->solutionID;
               lw_solutions->setCurrentRow( ii );
               QListWidgetItem* item = lw_solutions->item( ii );
               select_solution( item );
               break;
            }
         }
      }

      else            // Disk access
      {
         // Search for GUID
         for ( int ii = 0; ii < info.size(); ii++ )
         {
            if ( aguid == info[ ii ].guid )
            {
               lw_solution_list->setCurrentRow( ii );
               QListWidgetItem* item = lw_solutions>item( ii );
               select_solution( item );
               break;
            }
         }
      }
   }
   else
   {  // There is no selected solution, de-select a list item
      lw_solution_list->setCurrentRow( -1 );

      reset();
   }
}


void US_SolutionMgrSelect::query( void )
{
   if ( from_db )
   {
      read_db();
   }
   else
   {
      read_soluttion();
   }
}

// Read buffer information from local disk
void US_SolutionMgrSelect::read_solution( void )
{
   QString path;
   if ( ! buffer_path( path ) ) return;

   filenames   .clear();
   descriptions.clear();
   GUIDs       .clear();
   bufferIDs   .clear();
   le_search->  clear();
   le_search->setReadOnly( true );

 

   QDir f( path );
   QStringList filter( "B*.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

   for ( int ii = 0; ii < f_names.size(); ii++ )
   {
      QFile b_file( path + "/" + f_names[ ii ] );

      if ( ! b_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &b_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "buffer" )
            {
               QXmlStreamAttributes a = xml.attributes();
               descriptions << a.value( "description" ).toString();
               GUIDs        << a.value( "guid"        ).toString();
               filenames    << path + "/" + f_names[ ii ];
               bufferIDs    << "";
               break;
            }
         }
      }
   }

   lw_buffer_list->clear();

   if ( descriptions.size() == 0 )
      lw_buffer_list->addItem( "No buffer files found." );
   else
   {
      le_search->setReadOnly( false );
      search();
   }
}

// Read buffer information from the database
void US_SolutionMgrSelect::read_db( void )
{
   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   bufferIDs   .clear();
   descriptions.clear();
   GUIDs       .clear();
   le_search->  clear();
   le_search->setText( "" );
   le_search->setReadOnly( true );

   QStringList q;
   q << "get_buffer_desc" << QString::number( *personID );
   db.query( q );

   while ( db.next() )
   {
      bufferIDs    << db.value( 0 ).toString();
      descriptions << db.value( 1 ).toString();
      GUIDs        << "";
   }

   lw_buffer_list->clear();

   if ( descriptions.size() == 0 )
   {
      lw_buffer_list->addItem( "No buffer files found." );
   }
   else
   {
      le_search->setReadOnly( false );
      search();
   }

}

// Verify path to local buffer files and create directory if need be
bool US_SolutionMgrSelect::solution_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
            tr( "Bad Buffer Path" ),
            tr( "Could not create default directory for buffers\n" ) + path );
         return false;
      }
   }

   return true;
}

// Report a database connection error
void US_SolutionMgrSelect::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
                        tr( "Could not connect to database \n" ) + error );
}
*/
// Function to refresh the display with values from the solution structure,
//  and to enable/disable features
void US_SolutionMgrSelect::reset( void )
{
   QList< US_Solution::AnalyteInfo >&   ai         = solution->analyteInfo;
   QString                              bufferDesc = solution->buffer.description;

   le_bufferInfo   -> setText( solution->buffer.description );
   le_solutionDesc -> setText( solution->solutionDesc );
   le_commonVbar20 -> setText( QString::number( solution->commonVbar20 ) );
   le_density      -> setText( QString::number( solution->buffer.density ) );
   le_viscosity    -> setText( QString::number( solution->buffer.viscosity ) );
   le_storageTemp  -> setText( QString::number( solution->storageTemp  ) );
   te_notes        -> setText( solution->notes        );
   le_guid         -> setText( solution->solutionGUID );
   //ct_amount       -> disconnect();
   //ct_amount       -> setEnabled( false );
   //ct_amount       -> setValue( 1 );

   //pb_buffer       -> setEnabled( true );

   //pb_addAnalyte   -> setEnabled( true );
   //pb_removeAnalyte-> setEnabled( false );

   // Let's calculate if we're eligible to save this solution
   //pb_save         -> setEnabled( false );
   if ( ! bufferDesc.isEmpty() ) //we can have a solution with buffer only
   {
     //pb_save      -> setEnabled( true );
   }

   // We can always delete something, even if it's just what's in the dialog
   pb_delete          -> setEnabled( false );
   if ( lw_solutions->currentRow() != -1 )
   {
      pb_delete       -> setEnabled( true );
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
}




 //  will search solutions matching search string
 void US_SolutionMgrSelect::search( QString const& text )
 {
    QString sep = ";";
    QStringList sortdesc;
    lw_solutions  ->clear();
    info.clear();
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
 DbgLv(1) << "BufS-search:  descsize" << dsize
  << "sortsize" << sortdesc.size();

    for ( int jj = 0; jj < sortdesc.size(); jj++ )
    {  // build list of sorted meta data and ListWidget entries
       int ii      = sortdesc[ jj ].section( sep, 1, 1 ).toInt();

       if ( ii < 0  ||  ii >= dsize )
       {
 DbgLv(1) << "BufS-search:  *ERROR* ii" << ii << "jj" << jj
  << "sdesc" << sortdesc[jj].section(sep,0,0);
          continue;
       }
       SolutionInfo si;
       si.index       = ii;
       si.description = descriptions[ ii ];
       si.GUID        = GUIDs       [ ii ];
       si.filename    = filenames   [ ii ];
       si.solutionID    = solutionIDs   [ ii ].toInt();

       info << si;

       lw_solutions->addItem( si.description );
    }
 }



// Main Solution window with panels
US_SolutionGui::US_SolutionGui( 
      int   expID,
      int   chID,
      bool  signal_wanted,
      int   select_db_disk,
      const US_Solution& dataIn,
      bool  auto_save
      ) : US_WidgetsDialog( 0, 0 ), experimentID( expID ), channelID( chID ),
        signal( signal_wanted ), solution( dataIn ), autosave( auto_save )

{
   personID     = US_Settings::us_inv_ID();
   solution      = dataIn;
   orig_solution = dataIn;
   disk_or_db  = ( select_db_disk == US_Disk_DB_Controls::Default )
                 ?  US_Settings::default_data_location()
                 : select_db_disk;
   dbg_level    = US_Settings::us_debug();

   
   setWindowTitle( tr( "Solution Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   this->setMinimumSize( 640, 480 );

   QGridLayout* main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   tabWidget   = us_tabwidget();
   selectTab   = new US_SolutionMgrSelect  ( &personID, &disk_or_db, &solution );
   // newTab      = new US_SolutionMgrNew     ( &personID, &disk_or_db, &solution, temperature, signal);
   // editTab     = new US_SolutionMgrEdit    ( &personID, &disk_or_db, &solution );
   // settingsTab = new US_SolutionMgrSettings( &personID, &disk_or_db );
   tabWidget -> addTab( selectTab,   tr( "Select Solution" ) );
   // tabWidget -> addTab( newTab,      tr( "Enter New Solution" ) );
   // tabWidget -> addTab( editTab,     tr( "Edit Existing Solution" ) );
   // tabWidget -> addTab( settingsTab, tr( "Settings" ) );

   main->addWidget( tabWidget );

   connect( tabWidget,   SIGNAL( currentChanged(       int  ) ),
            this,        SLOT (  checkTab(             int  ) ) );
   connect( selectTab,   SIGNAL( solutionAccepted(      void ) ),
            this,        SLOT (  solutionAccepted(      void ) ) );
   connect( selectTab,   SIGNAL( selectionCanceled(    void ) ),
            this,        SLOT (  solutionRejected(      void ) ) );
   // connect( newTab,      SIGNAL( newAnaAccepted(       void ) ),
   //          this,        SLOT (  newAnaAccepted(       void ) ) );
   // connect( newTab,      SIGNAL( newAnaCanceled(       void ) ),
   //          this,        SLOT (  newAnaCanceled(       void ) ) );
   // connect( editTab,     SIGNAL( editAnaAccepted(      void ) ),
   //          this,        SLOT (  editAnaAccepted(      void ) ) );
   // connect( editTab,     SIGNAL( editAnaCanceled(      void ) ),
   //          this,        SLOT (  editAnaCanceled(      void ) ) );
   // connect( settingsTab, SIGNAL( use_db(               bool ) ),
   //          this,        SLOT (  update_disk_or_db(    bool ) ) );
   // connect( settingsTab, SIGNAL( investigator_changed( int  ) ),
   //          this,        SLOT (  update_personID(      int  ) ) );
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
//selectTab  ->init_solution();
   }
   else if ( currentTab == 1 )
   {
DbgLv(1) << "ckTab:   newTab     init_solution";
//newTab     ->init_solution();
   }
   else if ( currentTab == 2 )
   {
DbgLv(1) << "ckTab:   editTab    init_solution";
//editTab    ->init_solution();
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
void US_SolutionGui::editAnaAccepted( void )
{
DbgLv(1) << "main: editAnaAccepted";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for Edit panel solution-changes-rejected
void US_SolutionGui::editAnaCanceled( void )
{
DbgLv(1) << "main: editAnaCanceled";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel solution accepted
void US_SolutionGui::newAnaAccepted( void )
{
DbgLv(1) << "main: newAnaAccepted";
   tabWidget->setCurrentIndex( 0 );
}

// Slot for New panel solution add rejected
void US_SolutionGui::newAnaCanceled( void )
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

   accept();
}

// Exit and signal caller that solution selection/changes were rejected
void US_SolutionGui::solutionRejected( void )
{
   solution   = orig_solution;

   reject();
}

