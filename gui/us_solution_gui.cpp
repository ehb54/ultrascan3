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
      int   expID,
      int   chID,
      bool  signal_wanted,
      int   select_db_disk,
      const US_Solution& dataIn,
      bool  auto_save
      ) : US_WidgetsDialog( 0, 0 ), experimentID( expID ), channelID( chID ),
        signal( signal_wanted ), solution( dataIn ), autosave( auto_save )
{
   setAttribute( Qt::WA_DeleteOnClose );

   investigatorID = US_Settings::us_inv_ID();

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
   if ( US_Settings::us_inv_level() > 2 )
   {
      QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
      connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
      main->addWidget( pb_investigator, row++, 0 );
   }
   else
   {
      QLabel* lb_investigator = us_label( tr( "Investigator:" ) );
      main->addWidget( lb_investigator, row++, 0 );
   }

   // Available solutions
   QLabel* lb_banner2 = us_banner( tr( "Click on solution to select" ), -2  );
   lb_banner2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   lb_banner2->setMinimumWidth( 400 );
   main->addWidget( lb_banner2, row++, 0 );

   lw_solutions = us_listwidget();
   lw_solutions-> setSortingEnabled( true );
   connect( lw_solutions, SIGNAL( itemClicked    ( QListWidgetItem* ) ),
                          SLOT  ( selectSolution ( QListWidgetItem* ) ) );
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

   int add_rows = ( US_Settings::us_debug() == 0 ) ? 6 : 8;

   main->addWidget( lw_analytes, row, 0, add_rows, 1 );

   row += add_rows;

   // Second column
   row = 1;

   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   disk_controls = new US_Disk_DB_Controls( select_db_disk );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SLOT  ( source_changed( bool ) ) );
   main->addLayout( disk_controls, row++, 1, 1, 2 );

   pb_query = us_pushbutton( tr( "Query Solutions" ), true );
   connect( pb_query, SIGNAL( clicked() ), SLOT( load() ) );
   main->addWidget( pb_query, row, 1 );

   pb_save = us_pushbutton( tr( "Save Solution" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   main->addWidget( pb_save, row++, 2 );

   pb_addAnalyte = us_pushbutton( tr( "Add Analyte" ), true );
   connect( pb_addAnalyte, SIGNAL( clicked() ), SLOT( addAnalyte() ) );
   main->addWidget( pb_addAnalyte, row, 1 );

   pb_del = us_pushbutton( tr( "Delete Solution" ), false );
   connect( pb_del, SIGNAL( clicked() ), SLOT( delete_solution() ) );
   main->addWidget( pb_del, row++, 2 );

   pb_removeAnalyte = us_pushbutton( tr( "Remove Analyte" ), false );
   connect( pb_removeAnalyte, SIGNAL( clicked() ), SLOT( removeAnalyte() ) );
   main->addWidget( pb_removeAnalyte, row, 1 );

   pb_buffer = us_pushbutton( tr( "Select Buffer" ), true );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( selectBuffer() ) );
   main->addWidget( pb_buffer, row++, 2 );

   le_bufferInfo = us_lineedit( "", 1 );
   le_bufferInfo ->setPalette ( gray );
   le_bufferInfo ->setReadOnly( true );
   main->addWidget( le_bufferInfo, row++, 1, 1, 2 );

   QLabel* lb_banner4 = us_banner( tr( "Edit solution properties" ), -2  );
   lb_banner4->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner4, row++, 1, 1, 2 );

   QLabel* lb_solutionDesc = us_label( tr( "Solution Name:" ) );
   main->addWidget( lb_solutionDesc, row++, 1, 1, 2 );

   le_solutionDesc = us_lineedit( "", 1 );
   connect( le_solutionDesc, SIGNAL( textEdited      ( const QString&   ) ),
                             SLOT  ( saveDescription ( const QString&   ) ) );
   main->addWidget( le_solutionDesc, row++, 1, 1, 2 );

   QLabel* lb_commonVbar20 = us_label( tr( "Common VBar (20C):" ) );
   main->addWidget( lb_commonVbar20, row, 1 );

   le_commonVbar20 = us_lineedit( "", 1 );
   le_commonVbar20->setPalette ( gray );
   le_commonVbar20->setReadOnly( true );
   main->addWidget( le_commonVbar20, row++, 2 );

   QLabel* lb_density = us_label( tr( "Buffer density:" ) );
   main->addWidget( lb_density, row, 1 );

   le_density = us_lineedit( "", 1 );
   le_density->setPalette ( gray );
   le_density->setReadOnly( true );
   main->addWidget( le_density, row++, 2 );

   QLabel* lb_viscosity = us_label( tr( "Buffer viscosity:" ) );
   main->addWidget( lb_viscosity, row, 1 );

   le_viscosity = us_lineedit( "", 1 );
   le_viscosity->setPalette ( gray );
   le_viscosity->setReadOnly( true );
   main->addWidget( le_viscosity, row++, 2 );

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
   main->addWidget( te_notes, row, 1, 5, 2 );
   te_notes->setMinimumHeight( 200 );
   te_notes->setReadOnly( false );
   row += 3;

   QLabel* lb_guid = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_guid, row++, 1, 1, 2 );

   le_guid = us_lineedit( "" ); 
   le_guid->setPalette ( gray );
   le_guid->setReadOnly( true );
   main->addWidget( le_guid, row++, 1, 1, 2 );
 
   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( newSolution() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   pb_accept = us_pushbutton( tr( "Close" ) );

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
   
   reset();

   // Load the solution descriptions
   load();

   changed = false;

   // Select the current one if we know what it is
   if ( solution.solutionID > 0 )
   {
      QList< QListWidgetItem* > items 
        = lw_solutions->findItems( solution.solutionDesc, Qt::MatchExactly );

      // should be exactly 1, but let's make sure
      if ( items.size() == 1 )
      {
         selectSolution( items[ 0 ] );
         lw_solutions->setCurrentItem( items[ 0 ] );
      }
   }
}

// Function to refresh the display with values from the solution structure,
//  and to enable/disable features
void US_SolutionGui::reset( void )
{
   QList< US_Solution::AnalyteInfo >&   ai         = solution.analyteInfo;
   QString                              bufferDesc = solution.buffer.description;

   le_bufferInfo   -> setText( solution.buffer.description );
   le_solutionDesc -> setText( solution.solutionDesc );
   le_commonVbar20 -> setText( QString::number( solution.commonVbar20 ) );
   le_density      -> setText( QString::number( solution.buffer.density ) );
   le_viscosity    -> setText( QString::number( solution.buffer.viscosity ) );
   le_storageTemp  -> setText( QString::number( solution.storageTemp  ) );
   te_notes        -> setText( solution.notes        );
   le_guid         -> setText( solution.solutionGUID );
   ct_amount       -> disconnect();
   ct_amount       -> setEnabled( false );
   ct_amount       -> setValue( 1 );

   pb_buffer       -> setEnabled( true );

   pb_addAnalyte   -> setEnabled( true );
   pb_removeAnalyte-> setEnabled( false );

   // Let's calculate if we're eligible to save this solution
   pb_save         -> setEnabled( false );
   if ( ! bufferDesc.isEmpty() ) //we can have a solution with buffer only
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

   // Figure out if the accept button should be enabled
   if ( ! signal )      // Then it's just a close button
      pb_accept->setEnabled( true );

   else if ( solution.saveStatus == US_Solution::BOTH )
      pb_accept->setEnabled( true );

   else if ( ( ! disk_controls->db() ) && solution.saveStatus == US_Solution::HD_ONLY )
      pb_accept->setEnabled( true );

   else if ( (   disk_controls->db() ) && solution.saveStatus == US_Solution::DB_ONLY )
      pb_accept->setEnabled( true );

   else
      pb_accept->setEnabled( false );

   // Display investigator
   investigatorID = US_Settings::us_inv_ID();

   QString number = ( investigatorID > 0 ) 
      ? QString::number( investigatorID ) + ": " 
      : "";

   le_investigator->setText( number +  US_Settings::us_inv_name() );
}

// Function to accept the current solution and return
void US_SolutionGui::accept( void )
{
   if ( changed )
   {
      bool save_it = autosave;

      if ( ! autosave )
      {
         int response = QMessageBox::question( this,
            tr( "Save Changed Solution?" ),
            tr( "Changes were made to the solution and you did not save them.\n"
                "Do you wish to save the solution now?" ),
            QMessageBox::Yes, QMessageBox::No );

         save_it = ( response == QMessageBox::Yes );
      }

      if ( save_it )
      {
         save( false );     // make sure the current selections have been saved
      }
   }

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
   US_Investigator* inv_dialog = new US_Investigator( true, investigatorID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_SolutionGui::assign_investigator( int invID,
      const QString& /*lname*/, const QString& /*fname*/ )
{
   investigatorID = invID;

   QString number = ( investigatorID > 0 ) 
      ? QString::number( investigatorID ) + ": " 
      : "";

   le_investigator->setText( number +  US_Settings::us_inv_name() );
}

// Function to load solutions into solutions list widget
void US_SolutionGui::load( void )
{
   if ( disk_controls->db() )
      loadDB();

   else
      loadDisk();

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

   if ( investigatorID < 1 ) investigatorID = US_Settings::us_inv_ID();

   QStringList q( "all_solutionIDs" );
   q << QString::number( investigatorID );
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

   int status = US_DB2::OK;

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      status = solution.readFromDB  ( solutionID, &db );

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
      status = solution.readFromDisk( solutionGUID );

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

// Function to add analyte to solution
void US_SolutionGui::addAnalyte( void )
{
   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_AnalyteGui* analyte_dialog = new US_AnalyteGui( true, QString(), dbdisk );

   connect( analyte_dialog, SIGNAL( valueChanged  ( US_Analyte ) ),
            this,           SLOT  ( assignAnalyte ( US_Analyte ) ) );

   connect( analyte_dialog, SIGNAL( use_db        ( bool ) ), 
                            SLOT  ( update_disk_db( bool ) ) );

   analyte_dialog->exec();
   qApp->processEvents();
   changed = true;

}

// Get information about selected analyte
void US_SolutionGui::assignAnalyte( US_Analyte data )
{
   US_Solution::AnalyteInfo newInfo;
   newInfo.analyte = data;
   newInfo.amount  = 1;

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

   // Make sure item has not been added already
   if ( solution.analyteInfo.contains( newInfo ) )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "Your solution already contains this analyte\n"
                "If you wish to change the amount, remove it and "
                "add it again.\n" ) );
      return;
   }

   solution.analyteInfo << newInfo;

   calcCommonVbar20();

   // We're maintaining a map to account for automatic sorting of the list
   QListWidgetItem* item = new QListWidgetItem( newInfo.analyte.description, lw_analytes );
   analyteMap[ item ] = solution.analyteInfo.size() - 1;      // The one we just added

   reset();
   changed = true;
}

// Function to handle when solution listwidget item is selected
void US_SolutionGui::selectAnalyte( QListWidgetItem* item )
{
   // Get the right index in the sorted list, and load the amount
   int ndx = analyteMap[ item ];
   ct_amount ->setValue( solution.analyteInfo[ ndx ].amount );

   // Now turn the label red to catch attention
   QPalette p = lb_amount->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_amount->setPalette( p );

   pb_removeAnalyte ->setEnabled( true );
   ct_amount        ->setEnabled( true );
   connect( ct_amount, SIGNAL( valueChanged ( double ) ),      // if the user has changed it
                       SLOT  ( saveAmount   ( double ) ) );
   changed = true;
}

// Function to add analyte to solution
void US_SolutionGui::removeAnalyte( void )
{
   // Allow for the fact that this listwidget is sorted
   QListWidgetItem* item = lw_analytes->currentItem();
   int ndx = analyteMap[ item ];

   solution.analyteInfo.removeAt( ndx );
   lw_analytes ->removeItemWidget( item );

   calcCommonVbar20();

   reset();
   changed = true;
}

// Function to calculate the default commonVbar20 value
void US_SolutionGui::calcCommonVbar20( void )
{
   solution.commonVbar20 = 0.0;

   if ( solution.analyteInfo.size() == 1 )
      solution.commonVbar20 = solution.analyteInfo[ 0 ].analyte.vbar20;

   else     // multiple analytes
   {
      double numerator   = 0.0;
      double denominator = 0.0;
      foreach ( US_Solution::AnalyteInfo ai, solution.analyteInfo )
      {
         numerator   += ai.analyte.vbar20 * ai.analyte.mw * ai.amount;
         denominator += ai.analyte.mw * ai.amount;
      }

      solution.commonVbar20 = ( denominator == 0 ) ? 0.0 : ( numerator / denominator );

   }
   changed = true;

}

// Create a dialog to request a buffer selection
void US_SolutionGui::selectBuffer( void )
{
   int dbdisk = ( disk_controls->db() ) ? US_Disk_DB_Controls::DB
                                        : US_Disk_DB_Controls::Disk;

   US_BufferGui* buffer_dialog = new US_BufferGui( true,
                                                   solution.buffer, dbdisk );

   connect( buffer_dialog, SIGNAL( valueChanged ( US_Buffer ) ),
            this,          SLOT  ( assignBuffer ( US_Buffer ) ) );

   connect( buffer_dialog, SIGNAL( use_db        ( bool ) ), 
                           SLOT  ( update_disk_db( bool ) ) );

   buffer_dialog->exec();
   qApp->processEvents();
   changed = true;
}

// Get information about selected buffer
void US_SolutionGui::assignBuffer( US_Buffer newBuffer )
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

   solution.buffer = newBuffer;

   reset();
   changed = true;
}

// Function to update the amount that is associated with an individual analyte
void US_SolutionGui::saveAmount( double amount )
{
   // Get the right index in the sorted list of analytes
   QListWidgetItem* item = lw_analytes->currentItem();

   // if item not selected return

   int ndx = analyteMap[ item ];
   solution.analyteInfo[ ndx ].amount = amount;

   calcCommonVbar20();

   // Update commonVbar20 value in GUI
   le_commonVbar20 -> setText( QString::number( solution.commonVbar20 ) );
   changed = true;
}

// Function to update the description associated with the current solution
void US_SolutionGui::saveDescription( const QString& )
{
   solution.solutionDesc = le_solutionDesc ->text();
   changed = true;
   QListWidgetItem* item = new QListWidgetItem( solution.solutionDesc );

   if ( ! solutionMap.contains( item ) )
   {
      solution.solutionGUID.clear();
      reset();
   }
}

// Function to update the storage temperature associated with the current solution
void US_SolutionGui::saveTemperature( const QString& )
{
   solution.storageTemp = le_storageTemp ->text().toDouble();
   changed = true;
}

// Function to update the notes associated with the current solution
void US_SolutionGui::saveNotes( void )
{
   // Let's see if the notes have actually changed
   if ( solution.notes != te_notes->toPlainText() )
   {
      solution.notes        = te_notes        ->toPlainText();
   }
   changed = true;
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
   changed = true;
}

// Function to save solution information to disk or db
void US_SolutionGui::save( bool display_status )
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

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      int status = solution.saveToDB( experimentID, channelID, &db );

      if ( status != US_DB2::OK && ! display_status )  // then we return but no status msg
         return;

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

      else if ( status != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "Attention" ) ,
               db.lastError() );
         return;
      }
    
   }

   else
      solution.saveToDisk();

   if ( display_status )
   {
      QMessageBox::information( this,
            tr( "Save results" ),
            tr( "Solution saved" ) );
   }

   // Refresh solution list
   solutionMap.clear();
   lw_solutions->clear();

   load();
   reset();

   // Select the solution
   QList< QListWidgetItem* > items 
     = lw_solutions->findItems( solution.solutionDesc, Qt::MatchExactly );

   // should be exactly 1, but let's make sure
   if ( items.size() == 1 )
   {
      selectSolution( items[ 0 ] );
      lw_solutions->setCurrentItem( items[ 0 ] );
   }

   changed = false;
}

// Function to delete a solution from disk, db, or in the current form
void US_SolutionGui::delete_solution( void )
{
   int status = US_DB2::OK;

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      status = solution.deleteFromDB( &db );
   }

   else
      status = solution.deleteFromDisk();

   if ( status == US_DB2::SOLUT_IN_USE )
   {
      QMessageBox::warning( this,
         tr( "Delete aborted" ),
         tr( "Solution NOT Deleted, since it is in use\n"
             "by one or more experiments" ) );
      return;
   }

   solution.clear();
   analyteMap.clear();
   load();
   reset();

   QMessageBox::information( this,
         tr( "Delete results" ),
         tr( "Solution Deleted" ) );
   changed = true;
}

void US_SolutionGui::source_changed( bool db )
{
   QStringList DB = US_Settings::defaultDB();

   if ( db && ( DB.size() < 5 ) )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

   emit use_db( db );
   qApp->processEvents();

   // Clear out solution list
   solutionMap.clear();
   lw_solutions->clear();

   load();
   reset();
}

void US_SolutionGui::update_disk_db( bool db )
{
   ( db ) ? disk_controls->set_db() : disk_controls->set_disk();

   // Pass it on
   emit use_db( db );
}

// Function to display an error returned from the database
void US_SolutionGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}
