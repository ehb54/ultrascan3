//! \file us_buffer_gui.cpp
#include "us_buffer_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_constants.h"
#include "us_investigator.h"

US_BufferGui::US_BufferGui( bool signal_wanted ) : US_WidgetsDialog( 0, 0 )
{
   signal = signal_wanted;

   US_BufferComponent::getAllFromHD( component_list );

   fromHD = false;

   setWindowTitle( tr( "Buffer Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;
   
   QStringList DB = US_Settings::defaultDB();
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   main->addWidget( lb_DB, row++, 0, 1, 3 );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row++, 0 );

   QBoxLayout* search = new QHBoxLayout;

   // Search
   QLabel* lb_search = us_label( tr( "Search:" ) );
   search->addWidget( lb_search );

   le_search = us_lineedit();
   le_search->setReadOnly( true );
   connect( le_search, SIGNAL( textChanged( const QString& ) ), 
                       SLOT  ( search     ( const QString& ) ) );
   search->addWidget( le_search );
   main->addLayout( search, row++, 0 );

   // Buffer descriptions from DB
   QLabel* lb_banner1 = us_banner( tr( "Doubleclick on buffer data to select" ), -2 );
   main->addWidget( lb_banner1, row++, 0 );

   lw_buffer_db = us_listwidget();
   connect( lw_buffer_db, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                          SLOT  ( select_buffer    ( QListWidgetItem* ) ) );

   main->addWidget( lw_buffer_db, row, 0, 8, 1 );
   row += 8;

   QLabel* lb_spectrum = us_label( tr( "Spectrum: (wavelengh/extinction pairs)" ) );
   main->addWidget( lb_spectrum, row++, 0 );

   // Labels
   QLabel* lb_description = us_label( tr( "Buffer Description:" ) );
   main->addWidget( lb_description, row++, 0 );

   QLabel* lb_buffer1 = us_label( tr( "Please select a Buffer Component:" ) );
   main->addWidget( lb_buffer1, row++, 0 );

   lb_selected = us_label( "" );
   main->addWidget( lb_selected, row++, 0 );

   // Buffer Components
   QLabel* lb_banner2 = us_banner( tr( "Click on item to select" ), -2  );
   main->addWidget( lb_banner2, row++, 0 );

   lw_ingredients = us_listwidget();

   for ( int i = 0; i < component_list.size(); i++ )
      lw_ingredients->addItem( component_list[ i ].name +  
                        " (" + component_list[ i ].range + ")" );

   connect( lw_ingredients, SIGNAL( itemSelectionChanged( void ) ), 
                            SLOT  ( list_component      ( void ) ) );
   
   main->addWidget( lw_ingredients, row, 0, 6, 1 );

   row += 6;

   QPushButton* pb_synch = us_pushbutton( tr( "Synch components with DB" ) );
   connect( pb_synch, SIGNAL( clicked() ), SLOT( synch_components() ) );
   main->addWidget( pb_synch, row++, 0 );

   row = 1;

   // Investigator
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   buffer.personID = -1;
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   // Pushbuttons
   QPushButton* pb_load = us_pushbutton( tr( "Load Buffer from HD" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( read_buffer() ) );
   main->addWidget( pb_load, row++, 1, 1, 2 );

   pb_save = us_pushbutton( tr( "Save Buffer to HD" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_buffer() ) );
   main->addWidget( pb_save, row++, 1, 1, 2 );

   QLabel* lb_banner3 = us_banner( tr( "Database Functions" ), -2 );
   main->addWidget( lb_banner3, row++, 1, 1, 2 );

   QPushButton* pb_load_db = us_pushbutton( tr( "Query Buffer Descriptions from DB" ) );
   connect( pb_load_db, SIGNAL( clicked() ), SLOT( read_db() ) );
   main->addWidget( pb_load_db, row++, 1, 1, 2 );

   pb_save_db = us_pushbutton( tr( "Save Buffer to DB" ), false );
   connect( pb_save_db, SIGNAL( clicked() ), SLOT( save_db() ) );
   main->addWidget( pb_save_db, row++, 1, 1, 2 );

   pb_update_db = us_pushbutton( tr( "Update Buffer in DB" ), false );
   connect( pb_update_db, SIGNAL( clicked() ), SLOT( update_db() ) );
   main->addWidget( pb_update_db, row++, 1, 1, 2 );

   pb_del_db = us_pushbutton( tr( "Delete Buffer from DB" ), false );
   connect( pb_del_db, SIGNAL( clicked() ), SLOT( delete_buffer() ) );
   main->addWidget( pb_del_db, row++, 1, 1, 2 );

   // Buffer parameters
   QLabel* lb_density = us_label( tr( "Density (20&deg;C, g/cm<sup>3</sup>):" ) );
   main->addWidget( lb_density, row, 1 );

   le_density = us_lineedit();
   main->addWidget( le_density, row++, 2 );

   QLabel* lb_viscosity = 
      us_label( tr( "Viscosity (20<span>&deg;</span>C, cp):" ) );
   main->addWidget( lb_viscosity, row, 1 );

   le_viscosity = us_lineedit();
   main->addWidget( le_viscosity, row++, 2 );

   QLabel* lb_ph = us_label( tr( "pH:" ) );
   main->addWidget( lb_ph, row, 1 );

   le_ph = us_lineedit();
   main->addWidget( le_ph, row++, 2 );

   le_spectrum = us_lineedit();
   main->addWidget( le_spectrum, row++, 1, 1, 2 );

   le_description = us_lineedit();
   main->addWidget( le_description, row++, 1, 1, 2 );

   lb_units = us_label( "" );
   QPalette p = lb_units->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_units->setPalette( p );
   lb_units->setAlignment( Qt::AlignCenter );
   main->addWidget( lb_units, row++, 1, 1, 2 );

   le_concentration = us_lineedit();
   connect( le_concentration, SIGNAL( returnPressed() ), 
                              SLOT  ( add_component() ) );
   main->addWidget( le_concentration, row++, 1, 1, 2 );

   // Current buffer
   QLabel* lb_buffer = us_banner( tr( "Doubleclick on item to remove" ), -2 );
   main->addWidget( lb_buffer, row++, 1, 1, 2 );

   lw_buffer = us_listwidget();
   connect( lw_buffer, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                       SLOT  ( remove_component ( QListWidgetItem* ) ) );
   main->addWidget( lw_buffer, row, 1, 7, 2 );
   row += 7;

   // Standard buttons
   QBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );;

   if ( signal )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( reject() ) );
      buttons->addWidget( pb_cancel );

      pb_accept->setText( tr( "Accept" ) );
   }

   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept_buffer() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row, 0, 1, 3 );
   reset();
}

//! \brief Get buffer components from the DB and  write to 
//  etc/bufferComponents.xml
void US_BufferGui::synch_components( void )
{
   US_Passwd pw;

   qApp->processEvents();
   // TODO: Error check 

   component_list.clear();
   US_BufferComponent::getAllFromDB( pw.getPasswd(), component_list );
   US_BufferComponent::putAllToHD  ( component_list );

   // Update the list widget with components from DB
   lw_ingredients->clear();

   for ( int i = 0; i < component_list.size(); i++ )
      lw_ingredients->addItem( component_list[ i ].name +  
                        " (" + component_list[ i ].range + ")" );
}

void US_BufferGui::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true );

   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );
   
   inv_dialog->exec();
}

void US_BufferGui::assign_investigator( int invID, 
      const QString& lname, const QString& fname)
{
   buffer.personID = invID;
   le_investigator->setText( "InvID (" + QString::number( invID ) + "): " +
         lname + ", " + fname );
}

/*! Load buffer data from the database and populate listbox. If
    an investigator is defined, only select the buffer files from the 
    investigator. */

void US_BufferGui::read_db()
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   pb_save     ->setEnabled( false );
   pb_save_db  ->setEnabled( false );
   pb_update_db->setEnabled( false );
   pb_del_db   ->setEnabled( false );

   le_search->setText( "" );
   le_search->setReadOnly( true );

   buffer_metadata.clear();
   lw_buffer_db->clear();
   
   struct buffer_info info;

   QStringList q( "get_buffer_desc" );
   q << QString::number( buffer.personID );

   db.query( q );

   while ( db.next() )
   {
      info.bufferID    = db.value( 0 ).toString();
      info.description = db.value( 1 ).toString();
  
      buffer_metadata << info;

      lw_buffer_db->addItem( new QListWidgetItem(
            info.bufferID + ": " + info.description, lw_buffer_db ) );
   }

   fromHD = false;

   if ( buffer_metadata.size() < 1  &&  buffer.personID != -1 )
      QMessageBox::information( this,
            tr( "No data" ),
            tr( "No buffer file found for the selected investigator,\n"
                "You can click on \"Reset\", then query the DB to \n"
                "find buffers from all Investigators." ) );
   
   else if ( buffer_metadata.size() < 1 )
      QMessageBox::information( this,
            tr( "No data" ),
            tr( "There are no buffer entries." ) );
   else
      le_search->setReadOnly( false );
}

void US_BufferGui::search( const QString& text )
{
   lw_buffer_db->clear();
   for ( int i = 0; i < buffer_metadata.size(); i++ )
   {
      struct buffer_info* info = &buffer_metadata[ i ];

      if ( info->description.contains( 
               QRegExp( ".*" + text + ".*", Qt::CaseInsensitive ) ) )
      {
        lw_buffer_db->addItem( new QListWidgetItem( 
              info->bufferID + ": " + info->description, lw_buffer_db ) );
      }
   }
}

void US_BufferGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

/*! \brief Display the appropriate data when the buffer name in the list widget
           is selected with a double click.
           \param item The description of the buffer selected.
*/
void US_BufferGui::select_buffer( QListWidgetItem* item )
{
   if ( fromHD ) return;

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   
   // Get the buffer data from the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QString bufferID = item->text().split( ':' )[ 0 ];
   
   QStringList q( "get_buffer_info" );
   q << bufferID;   // bufferID from list widget entry
      
   db.query( q );
   db.next(); 
   
   buffer.bufferID    = bufferID;
   buffer.description = db.value( 0 ).toString();
   buffer.spectrum    = db.value( 1 ).toString();
   buffer.pH          = db.value( 2 ).toString().toDouble();
   buffer.viscosity   = db.value( 3 ).toString().toDouble();
   buffer.density     = db.value( 4 ).toString().toDouble();

   QString personID   = db.value( 5 ).toString();
   buffer.personID    = personID.toInt();
   
   buffer.component    .clear();
   buffer.concentration.clear();
   lw_buffer->clear();

   q.clear();
   q << "get_buffer_components" <<  bufferID;

   db.query( q );

   while ( db.next() )
   {
      QString componentID = db.value( 0 ).toString();
      double  conc        = db.value( 4 ).toString().toDouble();

      buffer.concentration << conc;
      
      for ( int i = 0; i < component_list.size(); i ++ )
      {
         if ( componentID == component_list[ i ].componentID )
         {
            buffer.component << component_list[ i ];

            QString name = component_list[ i ].name;
            QString unit = component_list[ i ].unit;
      
            QString s;
            s.sprintf( "( %.1f ", conc );
            
            lw_buffer->addItem( name + s + unit + ")" );
            break;
         }
      }
      
      buffer.concentration << db.value( 4 ).toString().toDouble();
   }

   // Get the investigator's name and display it

   q.clear();
   q << "get_person_info" << personID;

   db.query( q );
   db.next(); 

   QString fname = db.value( 0 ).toString();
   QString lname = db.value( 1 ).toString();

   le_investigator->setText( "InvID (" + personID + ") " + fname + " " + lname );

   // Update the rest of the buffer data
   
   le_description->setText( buffer.description );

   QString s;
   le_density    ->setText( s.sprintf( " %6.4f", buffer.density     ) );
   le_viscosity  ->setText( s.sprintf( " %6.4f", buffer.viscosity ) );
   le_spectrum   ->setText( buffer.spectrum );
   le_ph         ->setText( s.sprintf( " %6.4f", buffer.pH          ) );
   le_description->setText( buffer.description );
   
   // Allow modification of the just selected buffer from the DB
   pb_update_db->setEnabled ( true ); 
   pb_del_db   ->setEnabled ( true ); 

   le_viscosity->setReadOnly( true );
   le_density  ->setReadOnly( true );
}

//! Load buffer data from Hard Drive
void US_BufferGui::read_buffer( void )
{
   QString filename = QFileDialog::getOpenFileName( this, 
         tr( "Select the buffer file to load" ), 
         US_Settings::dataDir(), tr( "Buffer Files (*_buf.xml)" ) );

   if ( filename.isEmpty() ) return;

   le_investigator->setText( "Not Selected" );
   lw_buffer_db   ->clear();
   lw_buffer_db   ->addItem( filename );
   
   fromHD = true;

   if ( ! buffer.readFromDisk( filename ) )
   {
      QMessageBox::information( this, 
            tr( "Attention" ), 
            tr( "UltraScan can not read selected buffer file:\n\n" ) +
                filename );
      return;
   }

   // readFromDisk only populates buffer.component[ i ].bufferID, 
   // so we need to replace it with the full component

   for ( int i = 0; i < buffer.component.size(); i++ )
   {
      QString id = buffer.component[ i ].componentID;

      for ( int j = 0; j < component_list.size(); j++ )
      {
         if ( id == component_list[ j ].componentID )
         {
            buffer.component.replace( i, component_list[ j ] );
            break;
         }
      }
   }

   lw_buffer_db->addItem( "Current Buffer was loaded" );
   lw_buffer_db->addItem( "from Hard Drive" );
   
   QString s;
   lw_buffer->clear();

   for ( int i = 0; i < buffer.component.size(); i++ )
   {
      QString name = buffer.component[ i ].name;
      QString unit = buffer.component[ i ].unit;
      s.sprintf( "( %.1f ", buffer.concentration[ i ] );
      lw_buffer->addItem( name + s + unit + ")" );
   }

   le_density    ->setText( s.sprintf( "%6.4f", buffer.density   ) );
   le_viscosity  ->setText( s.sprintf( "%6.4f", buffer.viscosity ) );
   le_ph         ->setText( s.sprintf( "%6.4f", buffer.pH        ) );
   le_spectrum   ->setText( buffer.spectrum );
   le_description->setText( buffer.description );

   // Don't let the user override density and viscosity calculations
   if ( buffer.component.size() > 0 )
   {
      le_viscosity->setReadOnly( true );
      le_density  ->setReadOnly( true );
   }
   else
   {
      le_viscosity->setReadOnly( false );
      le_density  ->setReadOnly( false );
   }

   pb_save->setEnabled( true );
}

/*! Save buffer data to disk drive in *_buf.xml. */
void US_BufferGui::save_buffer( void )
{
   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Enter a description for\n"
                "your buffer before saving it!" ) );
      return;
   }

   QString filename = QFileDialog::getSaveFileName( this,
           tr( "Enter the buffer filename" ),
           US_Settings::dataDir(), tr( "Buffer Files (*_buf.xml)" ) );
   
   if ( filename.isEmpty() ) return;

   if ( filename.right( 8 ) != "_buf.xml" ) filename.append( "_buf.xml" );
   
   int response = QMessageBox::question( this, 
             tr( "UltraScan - Buffer:" ),
             tr( "Click 'OK' to save buffer information to :\n" ) + filename ,
             QMessageBox::Ok, QMessageBox::Cancel );
  
   if ( response == QMessageBox::Ok )
   {
      update_buffer();
      
      if ( buffer.writeToDisk( filename ) )
      {
         QMessageBox::information( this,
               tr( "Success" ), 
               tr( "The file was written to the disk:\n\n" ) 
               + filename );
      }
      else
      {
         QMessageBox::information( this,
               tr( "Attention" ), 
               tr( "Unable to write the buffer to a file!\n"
                   "Please check diskspace and make sure\n"
                   "that the permissions allow writing and\n"
                   "the disk is not write-protected." ) );
      }
   }
}

void US_BufferGui::update_buffer( void )
{
   buffer.description  = le_description->text();
   buffer.spectrum     = le_spectrum   ->text();
   
   buffer.pH           = ( le_ph->text().isEmpty() ) ? 
                         7.0 : le_ph->text().toDouble();
   
   buffer.density      = le_density    ->text().toDouble();
   buffer.viscosity    = le_viscosity  ->text().toDouble();

   // These are updated in other places
   //buffer.component
   //buffer.concentration
   //buffer.bufferID
   //buffer.personID
}

void US_BufferGui::delete_buffer( void )
{
   if ( buffer.bufferID.toInt() < 0 || lw_buffer_db->currentRow() < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select the buffer which you\n"
                "want to delete from the database" ) );

      return;
   }

   int response = QMessageBox::question( this, 
            tr( "Confirmation" ),
            tr( "Do you really want to delete this entry?\n"
                "Clicking 'OK' will delete the selected buffer data "
                "from the database." ),
            QMessageBox::Ok, QMessageBox::Cancel );

   if ( response == QMessageBox::Ok )
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      // Delete the buffer data from the database
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }
         
      QStringList q( "delete_buffer" );
      q << buffer.bufferID;
     
      if ( db.statusQuery( q )  != US_DB2::OK )
      {
         QMessageBox::warning( this,
            tr( "Attention" ),
            tr( "Delete failed.\n\n" ) + db.lastError() );
      }
   }

   reset();
   read_db();
}

void US_BufferGui::save_db( void )
{
   if ( buffer.personID < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an investigator first!" ) );
      return;
   }

   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please enter a description for\n"
                "your buffer before saving it!" ) );
      return;
   }

   update_buffer();

   int response = QMessageBox::question( this, 
            tr( "UltraScan - Buffer Database" ),
            tr( "Click 'OK' to save the selected\n"
                "buffer file into the database" ) );
 
   if ( response == QMessageBox::Ok )
   {
      QStringList q( "new_buffer" );
      q << buffer.description
        << buffer.spectrum
        << QString::number( buffer.pH       , 'f', 5 )
        << QString::number( buffer.density  , 'f', 5 )
        << QString::number( buffer.viscosity, 'f', 5 );

      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );
            
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      db.statusQuery( q );

      int bufferID = db.lastInsertID();

      for ( int i = 0; i < buffer.component.size(); i++ )
      {
         q.clear();
         q << "add_buffer_component" 
           << QString::number ( bufferID ) 
           << buffer.component[ i ].componentID
           << QString::number( buffer.concentration[ i ], 'f', 5 );
      
         db.statusQuery( q );
      }

      reset();
      read_db();
   }
}

/*!  Update changed buffer data to DB table  */
void US_BufferGui::update_db( void )
{
   QString str;

   if ( buffer.bufferID.toInt() <= 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an existing Buffer first!" ) );
      return;
   }
   
   if ( buffer.personID <= 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an investigator first!" ) );
      return;
   }
   
   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please enter a buffer description first!" ) );
      return;
   }

   update_buffer();

   int response = QMessageBox::question( this, 
            tr( "Confirmation" ),
            tr( "Do you really want to update this entry in the database?\n" ),
            QMessageBox::Ok, QMessageBox::Cancel );

   if ( response == QMessageBox::Ok )
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      // Delete the buffer data from the database
      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }
         
      QStringList q( "update_buffer" );
      q << buffer.bufferID
        << buffer.description
        << buffer.spectrum
        << QString::number( buffer.pH       , 'f', 5 )
        << QString::number( buffer.density  , 'f', 5 )
        << QString::number( buffer.viscosity, 'f', 5 );

       db.statusQuery( q );

       q.clear();
       q << "delete_buffer_components" << buffer.bufferID;
       db.statusQuery( q );

       for ( int i = 0; i < buffer.component.size(); i++ )
       {
          q.clear();
          q << "add_buffer_component" 
            << buffer.bufferID
            << buffer.component[ i ].componentID
            << QString::number( buffer.concentration[ i ], 'f', 5 );

          db.statusQuery( q );
       }

       QMessageBox::question( this, 
            tr( "Success" ),
            tr( "The database has been updated\n" ) );
   }
}

/*!  Input the value of component  selected in lw_ingredients.  After 'Return'
 *   key was pressed, this function will dispaly the selected component value in
 *   lw_buffer and recalculate the density and viscosity.  */

void US_BufferGui::add_component( void )
{
   // We are modifying the buffer, nothing should be selected in the DB list
   lw_buffer_db->clearSelection();
   
   if ( lw_ingredients->currentItem() < 0 )
   {
      QMessageBox::information( this, 
            tr( "Attention" ),
            tr( "First select a buffer component!\n" ) );
      return;
   }

   double partial_concentration = le_concentration->text().toDouble();
   if ( partial_concentration <= 0.0 ) return; 

   QString s;
   bool    newItem = true;
   int     current = lw_ingredients->currentRow();

   US_BufferComponent std_bc = component_list[ current ];
   
   // Find out if this inredient already exists, otherwise add a new component
   for ( int i = 0; i < buffer.component.size(); i++ ) 
   {
      US_BufferComponent* bc = &buffer.component[ i ];

      if ( std_bc.name == bc->name )
      {
         // Simply update the partial concentration of the existing ingredient
         buffer.concentration[ i ] = partial_concentration;

         s.sprintf( " (%.1f ", partial_concentration );

         lw_buffer->item( i )->setText( std_bc.name + s + std_bc.unit + ")" );
         newItem = false;
         break;
      }
   }

   // Add a new ingredient to this buffer
   if ( newItem ) 
   {
      US_BufferComponent component;
      
      buffer.concentration << partial_concentration;
      buffer.component     << std_bc;
      
      s.sprintf( " (%.1f ", partial_concentration );
      lw_buffer->addItem( std_bc.name + s + std_bc.unit + ")" );
   }

   recalc_density();
   recalc_viscosity();

   le_density      ->setText( s.sprintf( " %6.4f ", buffer.density   ) );
   le_viscosity    ->setText( s.sprintf( " %6.4f ", buffer.viscosity ) );
   le_concentration->setText( "" );
   
   if ( buffer.component.size() > 0 )
   {
      le_viscosity->setReadOnly( true );
      le_density  ->setReadOnly( true );
   }
   else
   {
      le_viscosity->setReadOnly( false );
      le_density  ->setReadOnly( false );
   }

   pb_save   ->setEnabled( true );
   pb_save_db->setEnabled( true );

}

/*!  After selection of the buffer component in lw_ingredients, this method will
     display in lb_selected and wait for a partial concentartion input value. */

void US_BufferGui::list_component( void )
{
   int row = lw_ingredients->currentRow();

   lb_selected->setText( lw_ingredients->currentItem()->text() );
   lb_units->setText( tr( "Please enter with units in: " ) + 
         component_list[ row ].unit );
   
   le_concentration->setReadOnly( false );
   le_concentration->setFocus();
}

/*! When double clicked, the selected item in lw_buffer
    will be removed  and the density and viscosity will be recalculated.  */

void US_BufferGui::remove_component( QListWidgetItem* item )
{
   int row = lw_buffer->row( item );

   buffer.component    .removeAt( row );
   buffer.concentration.removeAt( row );
   
   recalc_viscosity();
   recalc_density();
   
   QString s;

   le_density  ->setText( s.sprintf(" %6.4f ", buffer.density   ) );
   le_viscosity->setText( s.sprintf(" %6.4f ", buffer.viscosity ) );
   
   QListWidgetItem* oldItem = lw_buffer->takeItem( row );
   delete oldItem;

   if ( buffer.component.size() == 0 )
   {
      le_viscosity->setReadOnly( false );
      le_density  ->setReadOnly( false );
      pb_save     ->setEnabled ( false );
      pb_save_db  ->setEnabled ( false );
   }
   else
   {
      le_viscosity->setReadOnly( true );
      le_density  ->setReadOnly( true );
   }
}

void US_BufferGui::accept_buffer( void )
{
   if ( signal ) 
      emit valueChanged( buffer.density, buffer.viscosity, buffer.bufferID );

   accept();
}

/*! Reset some variables to initialization. */
void US_BufferGui::reset( void )
{
   buffer.component.clear();

   buffer.density     = 0.0;
   buffer.viscosity   = 0.0;
   buffer.description = "";
   buffer.bufferID    = "-1";
   buffer.spectrum    = "";
   buffer.pH          = 7.0;

   le_search       ->setText( "" );
   le_search       ->setReadOnly( false );

   lw_buffer_db    ->clear();
   lw_buffer       ->clear();

   lb_selected     ->setText( "" );

   le_density      ->setText( "0.0" );
   le_density      ->setReadOnly( false );
                   
   le_viscosity    ->setText( "0.0" );
   le_viscosity    ->setReadOnly( false );
                   
   le_description  ->setText( "" );
   le_spectrum     ->setText( "" );
   le_ph           ->setText( "7.0" );
                   
   pb_save         ->setEnabled( false );
   pb_save_db      ->setEnabled( false );
   pb_update_db    ->setEnabled( false );
   pb_del_db       ->setEnabled( false );
   
   lb_units        ->setText( "" );
   le_concentration->setText( "" );
   le_concentration->setReadOnly( true );
}

/*!  Recalculate the density of the buffer based on the information in the
     template file */

void US_BufferGui::recalc_density( void )
{
   buffer.density = DENS_20W;

   // Iterate over all components in this buffer
   for ( int i = 0; i < buffer.component.size(); i++ ) 
   {
      US_BufferComponent* bc = &buffer.component[ i ];

      double c1 = buffer.concentration[ i ];
      if ( bc->unit == "mM" ) c1 /= 1000;

      double c2 = c1 * c1; // c1^2
      double c3 = c2 * c1; // c1^3
      double c4 = c3 * c1; // c1^4 

      if ( c1 > 0.0 )
      {
         buffer.density += 
           bc->dens_coeff[ 0 ] +
           bc->dens_coeff[ 1 ] * 1.0e-3 * sqrt( c1 )
         + bc->dens_coeff[ 2 ] * 1.0e-2 * c1
         + bc->dens_coeff[ 3 ] * 1.0e-3 * c2
         + bc->dens_coeff[ 4 ] * 1.0e-4 * c3
         + bc->dens_coeff[ 5 ] * 1.0e-6 * c4
         - DENS_20W;
      }
   }
}

/*!  Recalculate the viscosity of the buffer based on the information in the
     template file */

void US_BufferGui::recalc_viscosity( void )
{
   buffer.viscosity = 100.0 * VISC_20W;

   // Iterate over all components in this buffer
   for ( int i = 0; i < buffer.component.size(); i++) 
   {
      US_BufferComponent* bc = &buffer.component[ i ];

      double c1 = buffer.concentration[ i ];
      if ( bc->unit == "mM" ) c1 /= 1000;

      double c2 = c1 * c1; // c1^2
      double c3 = c2 * c1; // c1^3
      double c4 = c3 * c1; // c1^4 
      
      if ( c1 > 0.0 )
      {
         buffer.viscosity += 
           bc->visc_coeff[ 0 ]  
         + bc->visc_coeff[ 1 ] * 1.0e-3 * sqrt( c1 )
         + bc->visc_coeff[ 2 ] * 1.0e-2 * c1
         + bc->visc_coeff[ 3 ] * 1.0e-3 * c2 
         + bc->visc_coeff[ 4 ] * 1.0e-4 * c3
         + bc->visc_coeff[ 5 ] * 1.0e-6 * c4
         - 100.0 * VISC_20W;
      }
   }
}
