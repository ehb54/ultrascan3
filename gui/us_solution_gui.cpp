//! \file us_solution_gui.cpp

#include <QtGui>

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_solution.h"
#include "us_solution_gui.h"

US_SolutionGui::US_SolutionGui( 
      int   invID,
      int   expID,
      int   chID,
      bool  signal_wanted,
      const US_Solution& dataIn ) : US_WidgetsDialog( 0, 0 )
{
   solution.invID = invID;
   experimentID   = expID,
   channelID      = chID,
   signal         = signal_wanted;
   solution       = dataIn;

   setWindowTitle( tr( "Solution Management" ) );
   setPalette( US_GuiSettings::frameColor() );

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* main      = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() ) );

   int row = 0;

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_DB, row++, 0, 1, 3 );

   // First column
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row++, 0 );

   // Available solutions
   QLabel* lb_banner2 = us_banner( tr( "Double-click on solution to select" ), -2  );
   lb_banner2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   lb_banner2->setMinimumWidth( 400 );
   main->addWidget( lb_banner2, row++, 0 );

   lw_solutions = us_listwidget();
   lw_solutions-> setSortingEnabled( true );
   connect( lw_solutions, SIGNAL( itemDoubleClicked ( QListWidgetItem* ) ),
                          SLOT  ( selectSolution    ( QListWidgetItem* ) ) );
   main->addWidget( lw_solutions, row, 0, 7, 1 );

   row += 7;

   QHBoxLayout* lo_amount = new QHBoxLayout();

   lb_amount = us_label( tr( "Analyte Molar Ratio:" ) );
   lo_amount->addWidget( lb_amount );

   ct_amount = us_counter ( 2, 0, 100, 1 ); // #buttons, low, high, start_value
   ct_amount->setStep( 1 );
   ct_amount->setFont( QFont( US_GuiSettings::fontFamily(),
                              US_GuiSettings::fontSize() ) );
   lo_amount->addWidget( ct_amount );
   main->addLayout( lo_amount, row++, 0 );

   QLabel* lb_banner3 = us_banner( tr( "Current solution contents" ), -2  );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner3, row++, 0 );

   lw_analytes = us_listwidget();
   lw_analytes-> setSortingEnabled( true );
   connect( lw_analytes, SIGNAL( itemClicked  ( QListWidgetItem* ) ),
                         SLOT  ( selectAnalyte( QListWidgetItem* ) ) );
   main->addWidget( lw_analytes, row, 0, 6, 1 );

   row += 6;

   // Second column
   row = 1;

   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   QGridLayout* db_layout = us_radiobutton( tr( "Use Database" ), rb_db );
   connect( rb_db, SIGNAL( clicked() ),  SLOT( check_db() ) );
   main->addLayout( db_layout, row, 1 );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk );
   connect( rb_disk, SIGNAL( clicked() ),  SLOT( check_disk() ) );
   //disk ? rb_disk->setChecked( true ) : rb_db->setChecked( true );
   rb_disk->setChecked( true );  // for now
   main->addLayout( disk_layout, row++, 2 );

   pb_query = us_pushbutton( tr( "Query Solutions" ), true );
   connect( pb_query, SIGNAL( clicked() ), SLOT( load() ) );
   main->addWidget( pb_query, row, 1 );

   pb_newSolution = us_pushbutton( tr( "New solution" ), true );
   connect( pb_newSolution, SIGNAL( clicked() ), SLOT( newSolution() ) );
   main->addWidget( pb_newSolution, row++, 2 );

   pb_save = us_pushbutton( tr( "Save Solution" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   main->addWidget( pb_save, row, 1 );

   pb_del = us_pushbutton( tr( "Delete Solution" ), false );
   connect( pb_del, SIGNAL( clicked() ), SLOT( delete_solution() ) );
   main->addWidget( pb_del, row++, 2 );

   pb_addAnalyte = us_pushbutton( tr( "Add Analyte" ), true );
   connect( pb_addAnalyte, SIGNAL( clicked() ), SLOT( addAnalyte() ) );
   main->addWidget( pb_addAnalyte, row, 1 );

   pb_removeAnalyte = us_pushbutton( tr( "Remove Analyte" ), false );
   connect( pb_removeAnalyte, SIGNAL( clicked() ), SLOT( removeAnalyte() ) );
   main->addWidget( pb_removeAnalyte, row++, 2 );

   pb_buffer = us_pushbutton( tr( "Select Buffer" ), true );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( selectBuffer() ) );
   main->addWidget( pb_buffer, row, 1 );

   le_bufferInfo = us_lineedit( "", 1 );
   le_bufferInfo ->setPalette ( gray );
   le_bufferInfo ->setReadOnly( true );
   main->addWidget( le_bufferInfo, row++, 2 );

   QLabel* lb_banner4 = us_banner( tr( "Edit solution properties" ), -2  );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner4, row++, 1, 1, 2 );

   QLabel* lb_solutionDesc = us_label( tr( "Description of Solution:" ) );
   main->addWidget( lb_solutionDesc, row++, 1, 1, 2 );

   le_solutionDesc = us_lineedit( "", 1 );
   connect( le_solutionDesc, SIGNAL( textEdited      ( const QString&   ) ),
                             SLOT  ( saveDescription ( const QString&   ) ) );
   main->addWidget( le_solutionDesc, row++, 1, 1, 2 );

   QLabel* lb_storageTemp = us_label( tr( "Storage Temperature:" ) );
   main->addWidget( lb_storageTemp, row, 1 );

   le_storageTemp = us_lineedit( "", 1 );
   connect( le_storageTemp, SIGNAL( textEdited      ( const QString&   ) ),
                            SLOT  ( saveTemperature ( const QString&   ) ) );
   main->addWidget( le_storageTemp, row++, 2 );

   QLabel* lb_notes = us_label( tr( "Solution notes:" ) );
   main->addWidget( lb_notes, row++, 1, 1, 2 );

   te_notes = us_textedit();
   connect( te_notes, SIGNAL( textChanged( void ) ),
                      SLOT  ( saveNotes  ( void ) ) );
   main->addWidget( te_notes, row, 1, 6, 2 );
   te_notes->setMinimumHeight( 200 );
   te_notes->setReadOnly( false );
   row += 6;

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( newSolution() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );

   if ( signal )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
      buttons->addWidget( pb_cancel );

      pb_accept -> setText( tr( "Accept" ) );
   }

   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );
   
   // Now let's assemble the page
   
   main->addLayout( buttons, row, 0, 1, 3 );
   
   saveStatus = US_Solution::NOT_SAVED;

   reset();
}

// Function to refresh the display with values from the solution structure,
//  and to enable/disable features
void US_SolutionGui::reset( void )
{
   QList< US_Solution::AnalyteInfo >&   analytes   = solution.analytes;
   QString                              bufferDesc = solution.bufferDesc;

   le_bufferInfo   -> setText( solution.bufferDesc   );
   le_solutionDesc -> setText( solution.solutionDesc );
   le_storageTemp  -> setText( QString::number( solution.storageTemp )  );
   te_notes        -> setText( solution.notes        );
   ct_amount       -> disconnect();
   ct_amount       -> setEnabled( false );
   ct_amount       -> setValue( 0                    );

   pb_buffer       -> setEnabled( true );
   pb_newSolution  -> setEnabled( true );

   pb_addAnalyte   -> setEnabled( true );
   pb_removeAnalyte-> setEnabled( false );

   // Let's calculate if we're eligible to save this solution
   pb_save         -> setEnabled( false );
   if ( analytes.size() > 0    &&
        ! bufferDesc.isEmpty() && 
        bufferDesc != QString( "New Solution" ) )
   {
      pb_save      -> setEnabled( true );
   }

   // We can always delete something, even if it's just what's in the dialog
   pb_del          -> setEnabled( false );
   if ( lw_solutions->currentRow() != -1 )
   {
      pb_del       -> setEnabled( true );
   }

   // Display analytes that have been selected
   lw_analytes->clear();
   analyteMap.clear();
   for ( int i = 0; i < analytes.size(); i++ )
   {
      // Create a map to account for sorting of the list
      QListWidgetItem* item = new QListWidgetItem( analytes[ i ].analyteDesc, lw_analytes );
      analyteMap[ item ] = i;

      lw_analytes->addItem( item );
   }

   // Turn the red label back
   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::white );
   lb_amount->setPalette( p );
}

// Function to accept the current set of solutions and return
void US_SolutionGui::accept( void )
{
   if ( signal )
   {
      if ( le_solutionDesc->text().isEmpty() )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "Please enter a description for\n"
                   "your solution before accepting." ) );
         return;
      }
   
      if ( le_storageTemp->text().isEmpty() )
         solution.storageTemp = 0;
   
      emit updateSolutionGuiSelection( solution );
   }

   close();
}

// Function to cancel the current dialog and return
void US_SolutionGui::cancel( void )
{
   if ( signal )
      emit cancelSolutionGuiSelection();

   close();
}

// Function to select the current investigator
void US_SolutionGui::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, solution.invID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_SolutionGui::assign_investigator( int invID,
      const QString& lname, const QString& fname)
{
   solution.invID = invID;
   le_investigator->setText( QString::number( invID ) + ": " +
         lname + ", " + fname );
}

// Function to load solutions into solutions list widget
void US_SolutionGui::load( void )
{
   if ( rb_disk -> isChecked() )
      loadDisk();

   else
      loadDB();

}

// Function to load solutions from disk
void US_SolutionGui::loadDisk( void )
{
   QString path;
   if ( ! solution.diskPath( path ) ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();

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

   loadSolutions();
}

// Function to load solutions from db
void US_SolutionGui::loadDB( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   QStringList q( "all_solutionIDs" );
   q << QString::number( 0 );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();

   while ( db.next() )
   {
      QString newID = db.value( 0 ).toString();
      IDs          << newID;
      descriptions << db.value( 1 ).toString();
      GUIDs        << QString( "" );
      filenames    << QString( "" );
   }

   loadSolutions();
}

// Function to load the solutions list widget from the solutions data structure
void US_SolutionGui::loadSolutions( void )
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

// Function to handle when analyte listwidget item is selected
void US_SolutionGui::selectSolution( QListWidgetItem* item )
{
   // Account for the fact that the list has been sorted
   int     ndx          = solutionMap[ item ];
   int     solutionID   = info[ ndx ].solutionID;
   QString solutionGUID = info[ ndx ].GUID;

   solution.clear();

   if ( rb_disk -> isChecked() )
      solution.readFromDisk( solutionGUID );

   else
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      solution.readFromDB  ( solutionID, &db );
   }

   reset();
}

// Function to add analyte to solution
void US_SolutionGui::addAnalyte( void )
{
   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( solution.invID, true );

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   analyte_dialog->exec();

}

// Get information about selected analyte
void US_SolutionGui::assignAnalyte( US_Analyte data )
{
   US_Solution::AnalyteInfo currentAnalyte;
   currentAnalyte.analyteGUID = data.analyteGUID;
   currentAnalyte.analyteDesc = data.description;
   currentAnalyte.analyteID   = data.analyteID.toInt();   // May not be accurate

   // Now get analyteID from db if we can
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() == US_DB2::OK )
   {
      QStringList q( "get_analyteID" );
      q << currentAnalyte.analyteGUID;
      db.query( q );
   
      if ( db.next() )
         currentAnalyte.analyteID = db.value( 0 ).toInt();
   }

   // Make sure item has not been added already
   if ( solution.analytes.contains( currentAnalyte ) )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Your solution already contains this analyte\n"
                "If you wish to change the amount, remove it and "
                "add it again.\n" ) );
      return;
   }

   currentAnalyte.amount = 0;

   solution.analytes << currentAnalyte;

   // We're maintaining a map to account for automatic sorting of the list
   QListWidgetItem* item = new QListWidgetItem( currentAnalyte.analyteDesc, lw_analytes );
   analyteMap[ item ] = solution.analytes.size() - 1;      // The one we just added

   reset();
}

// Function to handle when solution listwidget item is selected
void US_SolutionGui::selectAnalyte( QListWidgetItem* item )
{
   // Get the right index in the sorted list, and load the amount
   int ndx = analyteMap[ item ];
   ct_amount ->setValue( solution.analytes[ ndx ].amount );

   // Now turn the label red to catch attention
   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_amount->setPalette( p );

   pb_removeAnalyte ->setEnabled( true );
   connect( ct_amount, SIGNAL( valueChanged ( double ) ),      // if the user has changed it
                       SLOT  ( saveAmount   ( double ) ) );
   ct_amount -> setEnabled( true );
}

// Function to add analyte to solution
void US_SolutionGui::removeAnalyte( void )
{
   // Allow for the fact that this listwidget is sorted
   QListWidgetItem* item = lw_analytes->currentItem();
   int ndx = analyteMap[ item ];

   solution.analytes.removeAt( ndx );
   lw_analytes ->removeItemWidget( item );

   reset();
}

// Create a dialog to request a buffer selection
void US_SolutionGui::selectBuffer( void )
{
   US_BufferGui* buffer_dialog = new US_BufferGui( solution.invID, true );         // Ask for a signal

   connect( buffer_dialog, SIGNAL( valueChanged ( US_Buffer ) ),
            this,          SLOT  ( assignBuffer ( US_Buffer ) ) );

   buffer_dialog->exec();
}

// Get information about selected buffer
void US_SolutionGui::assignBuffer( US_Buffer buffer )
{
   solution.bufferID = buffer.bufferID.toInt();
   solution.bufferGUID = buffer.GUID;
   solution.bufferDesc = buffer.description;

   // Now get the corresponding bufferID, if we can
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   US_DB2 db( masterPW );

   if ( db.lastErrno() == US_DB2::OK )
   {
      QStringList q( "get_bufferID" );
      q << solution.bufferGUID;
      db.query( q );
   
      if ( db.next() )
         solution.bufferID = db.value( 0 ).toInt();

   }

   reset();
}

// Function to update the amount that is associated with an individual analyte
void US_SolutionGui::saveAmount( double amount )
{
   // Get the right index in the sorted list of analytes
   QListWidgetItem* item = lw_analytes->currentItem();

   // if item not selected return

   int ndx = analyteMap[ item ];
   solution.analytes[ ndx ].amount = amount;
}

// Function to update the description associated with the current solution
void US_SolutionGui::saveDescription( const QString& )
{
   solution.solutionDesc = le_solutionDesc ->text();
}

// Function to update the storage temperature associated with the current solution
void US_SolutionGui::saveTemperature( const QString& )
{
   solution.storageTemp = le_storageTemp ->text().toFloat();
}

// Function to update the notes associated with the current solution
void US_SolutionGui::saveNotes( void )
{
   solution.notes        = te_notes        ->toPlainText();
}

// Function to create a new solution
void US_SolutionGui::newSolution( void )
{
   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();

   analyteMap.clear();
   solutionMap.clear();

   info.clear();
   solution.clear();

   lw_solutions->clear();
   reset();
}

// Function to save solution information to disk or db
void US_SolutionGui::save( void )
{
   if ( le_solutionDesc->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Please enter a description for\n"
                "your solution before saving it!" ) );
      return;
   }

   if ( le_storageTemp->text().isEmpty() )
      solution.storageTemp = 0;

   if ( rb_disk -> isChecked() )
   {
      solution.saveToDisk();

      saveStatus = ( saveStatus == US_Solution::DB_ONLY ) 
                 ? US_Solution::BOTH : US_Solution::HD_ONLY;
   }

   else
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      solution.saveToDB( experimentID, channelID, &db );

      saveStatus = ( saveStatus == US_Solution::HD_ONLY ) 
                 ? US_Solution::BOTH : US_Solution::DB_ONLY;
   }

   QMessageBox::information( this,
         tr( "Save results" ),
         tr( "Solution saved" ) );
}

// Function to delete a solution from disk, db, or in the current form
void US_SolutionGui::delete_solution( void )
{
   if ( rb_disk -> isChecked() )
      solution.deleteFromDisk();

   else
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      solution.deleteFromDB( &db );
   }

   solution.clear();
   analyteMap.clear();
   load();
   saveStatus = US_Solution::NOT_SAVED;
   reset();

   QMessageBox::information( this,
         tr( "Delete results" ),
         tr( "Solution Deleted" ) );
}

void US_SolutionGui::check_db( void )
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }
   else
      solution.invID = US_Settings::us_inv_ID();

   // Clear out solution list
   solutionMap.clear();
   lw_solutions->clear();

   reset();
}

void US_SolutionGui::check_disk( void )
{
   // Clear out solution list
   solutionMap.clear();
   lw_solutions->clear();

   reset();
}

// Function to display an error returned from the database
void US_SolutionGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}
