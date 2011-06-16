//! \file us_buffer_gui.cpp
#include "us_buffer_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_constants.h"
#include "us_investigator.h"
#include "us_table.h"
#include "us_util.h"
#include "us_math2.h"

US_BufferGui::US_BufferGui( 
      bool             signal_wanted,
      const US_Buffer& buf, 
      int              select_db_disk
   ) : US_WidgetsDialog( 0, 0 ), signal( signal_wanted ), buffer( buf )
{
   personID      = US_Settings::us_inv_ID();
   bufferCurrent = false;
   manualUpdate  = false;
   view_shared   = false;

   US_BufferComponent::getAllFromHD( component_list );

   setWindowTitle( tr( "Buffer Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   normal = US_GuiSettings::editColor();

   // Very light gray for read only line edit widgets
   gray = normal;
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );
   
   int row = 0;
   
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";
   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_DB, row++, 0, 1, 3 );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row++, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   QBoxLayout* lo_search = new QHBoxLayout;

   // Search
   QLabel* lb_search = us_label( tr( "Search:" ) );
   lo_search->addWidget( lb_search );

   le_search = us_lineedit();
   le_search->setReadOnly( true );
   connect( le_search, SIGNAL( textChanged( const QString& ) ), 
                       SLOT  ( search     ( const QString& ) ) );
   lo_search->addWidget( le_search );
   main->addLayout( lo_search, row++, 0 );

   // Buffer descriptions from DB
   QLabel* lb_banner1 = us_banner( 
         tr( "Doubleclick on buffer data to select" ), -2 );
   lb_banner1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner1, row++, 0 );

   lw_buffer_db = us_listwidget();
   connect( lw_buffer_db, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                          SLOT  ( select_buffer    ( QListWidgetItem* ) ) );

   main->addWidget( lw_buffer_db, row, 0, 6, 1 );
   row += 7;

   // Labels
   QHBoxLayout* desc = new QHBoxLayout;

   QLabel* lb_description = us_label( tr( "Buffer Description:" ) );
   desc->addWidget( lb_description );

   QGridLayout* shared = us_checkbox( tr( "Shared" ), cb_shared );

   // Set up checkbox colors to match label (as set in us_config)
   QPalette p = US_GuiSettings::labelColor();
   p.setColor( QPalette::Base,   p.color( QPalette::WindowText ) );  // CB BG
   p.setColor( QPalette::Button, p.color( QPalette::Window     ) );  // Text BG
   p.setColor( QPalette::Text,   p.color( QPalette::Window     ) );  // CB FG

   shared->itemAt( 0 )->widget()->setPalette( p );
   shared->itemAt( 1 )->widget()->setPalette( p );

   desc->addLayout( shared );

   main->addLayout( desc, row++, 0 );

   QLabel* lb_guid = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_guid, row++, 0 );

   QLabel* lb_buffer1 = us_label( tr( "Please select a Buffer Component:" ) );
   lb_buffer1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_buffer1, row++, 0 );

   lb_selected = us_label( "" );
   main->addWidget( lb_selected, row++, 0 );

   // Buffer Components
   QLabel* lb_banner2 = us_banner( tr( "Click on item to select" ), -2  );
   lb_banner2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner2, row++, 0 );

   lw_ingredients = us_listwidget();
   lw_ingredients-> setSortingEnabled( true );

   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() ) );
   int width = 0;

   QStringList keys = component_list.keys();
   qSort( keys );

   for ( int i = 0; i < keys.size(); i++ )
   {
      QString key = keys[ i ];
      QString s = component_list[ key ].name +
                  " (" + component_list[ key ].range + ")";

      // Insert the buffer component with it's key
      new QListWidgetItem( s, lw_ingredients, key.toInt() );
      width = max( fm.width( s ), width );
   }

   // Allow for vertical scroll bar too
   lw_ingredients->setMinimumWidth( width + 30 );
   lw_ingredients->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

   connect( lw_ingredients, SIGNAL( itemSelectionChanged( void ) ), 
                            SLOT  ( list_component      ( void ) ) );
   
   main->addWidget( lw_ingredients, row, 0, 5, 1 );

   row += 5;

   QPushButton* pb_synch = us_pushbutton( tr( "Synch components with DB" ) );
   connect( pb_synch, SIGNAL( clicked() ), SLOT( synch_components() ) );
   main->addWidget( pb_synch, row++, 0 );

   row = 1;

   // Investigator
   
   QString number  = ( personID > 0 )
      ? QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";

   le_investigator = us_lineedit( number + US_Settings::us_inv_name() );
   le_investigator->setReadOnly( true );
   le_investigator->setPalette( gray );
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   disk_controls = new US_Disk_DB_Controls( select_db_disk );
   connect( disk_controls, SIGNAL( changed       ( bool ) ), 
                           SLOT  ( source_changed( bool ) ) );
   main->addLayout( disk_controls, row++, 1, 1, 2 );

   QLabel* lb_banner3 = us_banner( tr( "Database/Disk Functions" ), -2 );
   lb_banner3->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_banner3, row++, 1, 1, 2 );

   QPushButton* pb_query = us_pushbutton( tr( "Query Descriptions" ) );
   connect( pb_query, SIGNAL( clicked() ), SLOT( query() ) );
   main->addWidget( pb_query, row, 1 );

   pb_save = us_pushbutton( tr( "Save Buffer" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   main->addWidget( pb_save, row++, 2 );

   pb_update = us_pushbutton( tr( "Update Buffer" ), false );
   connect( pb_update, SIGNAL( clicked() ), SLOT( update() ) );
   main->addWidget( pb_update, row, 1 );

   pb_del = us_pushbutton( tr( "Delete Buffer" ), false );
   connect( pb_del, SIGNAL( clicked() ), SLOT( delete_buffer() ) );
   main->addWidget( pb_del, row++, 2 );

   // Buffer parameters
   QLabel* lb_density = us_label( 
         tr( "Density (20" ) + DEGC + tr( ", g/cm<sup>3</sup>):" ) );
   main->addWidget( lb_density, row, 1 );

   le_density = us_lineedit();
   connect( le_density, SIGNAL( textEdited( const QString& ) ), 
                        SLOT  ( density   ( const QString& ) ) );
   main->addWidget( le_density, row++, 2 );

   QLabel* lb_viscosity = 
      us_label( tr( "Viscosity (20" ) + DEGC + tr( ", cp):" ) );
   main->addWidget( lb_viscosity, row, 1 );

   le_viscosity = us_lineedit();
   connect( le_viscosity, SIGNAL( textEdited( const QString& ) ), 
                          SLOT  ( viscosity ( const QString& ) ) );
   main->addWidget( le_viscosity, row++, 2 );

   QLabel* lb_ph = us_label( tr( "pH:" ) );
   main->addWidget( lb_ph, row, 1 );

   le_ph = us_lineedit();
   main->addWidget( le_ph, row++, 2 );

   QLabel* lb_compressibility = us_label( tr( "Compressibility:" ) );
   main->addWidget( lb_compressibility, row, 1 );

   le_compressibility = us_lineedit();
   main->addWidget( le_compressibility, row++, 2 );

   QLabel* lb_optics = us_label( tr( "Optics:" ) );
   main->addWidget( lb_optics, row, 0 );

   cmb_optics = us_comboBox();
   cmb_optics->addItem( tr( "Absorbance"   ) );
   cmb_optics->addItem( tr( "Interference" ) );
   cmb_optics->addItem( tr( "Fluorescence" ) );
   main->addWidget( cmb_optics, row, 1 );

   QPushButton* pb_spectrum = us_pushbutton( tr( "Manage Spectrum" ) );
   connect( pb_spectrum, SIGNAL( clicked() ), SLOT( spectrum() ) );
   main->addWidget( pb_spectrum, row++, 2 );

   le_description = us_lineedit();
   main->addWidget( le_description, row++, 1, 1, 2 );
   connect( le_description, SIGNAL( editingFinished() ),
                            SLOT  ( new_description() ) );
   le_description->setEnabled( false );

   le_guid = us_lineedit();
   le_guid->setReadOnly( true );
   le_guid->setPalette ( gray );
   main->addWidget( le_guid, row++, 1, 1, 2 );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }

   lb_units = us_label( "" );
   p        = lb_units->palette();
   p.setColor( QPalette::WindowText, Qt::red );
   lb_units->setPalette( p );
   lb_units->setAlignment( Qt::AlignCenter );
   main->addWidget( lb_units, row++, 1, 1, 2 );

   le_concentration = us_lineedit();
   connect( le_concentration, SIGNAL( editingFinished() ), 
                              SLOT  ( add_component  () ) );
   main->addWidget( le_concentration, row++, 1, 1, 2 );

   // Current buffer
   QLabel* lb_buffer = us_banner( tr( "Doubleclick on item to remove" ), -2 );
   lb_buffer->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_buffer, row++, 1, 1, 2 );

   lw_buffer = us_listwidget();
   connect( lw_buffer, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ), 
                       SLOT  ( remove_component ( QListWidgetItem* ) ) );
   main->addWidget( lw_buffer, row, 1, 6, 2 );
   row += 6;

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

   init_buffer();
}

void US_BufferGui::check_db( void )
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }
   else
   {
      personID = US_Settings::us_inv_ID();

      if ( personID > 0 )
         le_investigator->setText( QString::number( personID ) + ": "
             + US_Settings::us_inv_name() );
   }
}

void US_BufferGui::density( const QString& d )
{
   buffer.density = d.toDouble();

   if ( ! manualUpdate )
   {
      buffer.component    .clear();
      buffer.concentration.clear();
      buffer.componentIDs .clear();
      buffer.GUID         .clear();
      buffer.bufferID     .clear();
      le_guid            ->clear();
      lw_buffer          ->clear();
      lw_buffer          ->addItem( tr( "Manual Override" ) );

      lw_buffer_db->setCurrentRow( -1 );
      manualUpdate  = true;
      bufferCurrent = false;
   }
}

void US_BufferGui::viscosity( const QString& v )
{
   buffer.viscosity = v.toDouble();

   if ( ! manualUpdate )
   {
      buffer.component    .clear();
      buffer.concentration.clear();
      buffer.componentIDs .clear();
      buffer.GUID         .clear();
      buffer.bufferID     .clear();
      le_guid            ->clear();
      lw_buffer          ->clear();
      lw_buffer          ->addItem( tr( "Manual Override" ) );

      lw_buffer_db->setCurrentRow( -1 );
      manualUpdate  = true;
      bufferCurrent = false;
   }
}

void US_BufferGui::init_buffer( void )
{
   query();

   if ( ! buffer.GUID.isEmpty() )
   {
      if ( ! disk_controls->db() ) // Disk access
      {
         // Search for GUID
         for ( int i = 0; i < buffer_metadata.size(); i++ )
         {
            if ( buffer.GUID == buffer_metadata[ i ].guid )
            {
               lw_buffer_db->setCurrentRow( i );
               QListWidgetItem* item = lw_buffer_db->item( i );
               select_buffer( item );
               manualUpdate = false;
               break;
            }
         }
      }

      else // DB access
      {
         // Search for bufferID
         for ( int i = 0; i < buffer_metadata.size(); i++ )
         {
            if ( buffer.bufferID == buffer_metadata[ i ].bufferID )
            {
               lw_buffer_db->setCurrentRow( i );
               QListWidgetItem* item = lw_buffer_db->item( i );
               select_buffer( item );
               manualUpdate = false;
               break;
            }
         }
      }
   }
   else
   {
      le_description->setText( buffer.description );
      le_density    ->setText( QString::number( buffer.density,   'f', 6 ) );
      le_viscosity  ->setText( QString::number( buffer.viscosity, 'f', 5 ) );
      le_ph         ->setText( QString::number( buffer.pH,        'f', 4 ) );
      le_compressibility->setText( 
                         QString::number( buffer.compressibility, 'e', 4 ) );
   }
}

//! \brief Get buffer components from the DB and  write to 
//  etc/bufferComponents.xml
void US_BufferGui::synch_components( void )
{
   US_Passwd pw;

   qApp->processEvents();

   component_list.clear();
   US_BufferComponent::getAllFromDB( pw.getPasswd(), component_list );
   US_BufferComponent::putAllToHD  ( component_list );

   // Update the list widget with components from DB
   lw_ingredients->clear();

   QStringList keys = component_list.keys();
   qSort( keys );

   for ( int i = 0; i < keys.size(); i++ )
   {
      QString key = keys[ i ];
      QString s   = component_list[ key ].name + 
                    " (" + component_list[ key ].range + ")";

      // Insert the buffer component with it's key
      new QListWidgetItem( s, lw_ingredients, key.toInt() );
   }
}

void US_BufferGui::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog, 
      SIGNAL( investigator_accepted( int ) ),
      SLOT  ( assign_investigator  ( int ) ) );
   
   inv_dialog->exec();
}

void US_BufferGui::assign_investigator( int invID )
{
   personID    = invID;
   view_shared = false;

   QString number = ( personID > 0 ) 
      ? QString::number( invID ) + ": "
      : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );

   if ( disk_controls->db() ) read_db();
}

// Get the path to the buffers.  Create it if necessary.
bool US_BufferGui::buffer_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/buffers";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
               tr( "Bad Buffer Path" ),
               tr( "Could not create default directory for buffers\n" )
               + path );
         return false;
      }
   }

   return true;
}

/*! Load buffer data and populate listbox. If an investigator is defined, only
    select the buffer files from the investigator. */
void US_BufferGui::query( void )
{
   if ( ! disk_controls->db() ) read_buffer();
   else                         read_db(); 

   le_description->setEnabled( true );
}

//! Load buffer data from Hard Drive
void US_BufferGui::read_buffer( void )
{
   QString path;
   if ( ! buffer_path( path ) ) return;

   filenames   .clear();
   descriptions.clear();
   GUIDs       .clear();
   bufferIDs   .clear();
   le_search->  clear();
   le_search->setPalette( gray );
   le_search->setReadOnly( true );

   bool desc_set = le_description->text().size() > 0;
   pb_save  ->setEnabled( desc_set );
   pb_update->setEnabled( desc_set );
   pb_del   ->setEnabled( false );

   QDir f( path );
   QStringList filter( "B*.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile b_file( path + "/" + f_names[ i ] );

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
               filenames    << path + "/" + f_names[ i ];
               bufferIDs    << "";
               break;
            }
         }
      }
   }

   lw_buffer_db->clear();
   
   if ( descriptions.size() == 0 )
      lw_buffer_db->addItem( "No buffer files found." );
   else
   {
      le_search->setReadOnly( false );
      le_search->setPalette ( normal );
      search();
   }
}

void US_BufferGui::read_db( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   bufferIDs   .clear();
   descriptions.clear();
   GUIDs       .clear();
   le_search->  clear();
   le_search->setPalette( gray );
   le_search->setReadOnly( true );

   bool desc_set = le_description->text().size() > 0;
   pb_save  ->setEnabled( desc_set );
   pb_update->setEnabled( desc_set );
   pb_del   ->setEnabled( false );

   le_search->setText( "" );
   le_search->setReadOnly( true );

   view_shared    = cb_shared->isChecked();
   QString person = view_shared ? "0" : QString::number( personID );
   QStringList q( "get_buffer_desc" );
   q << person;

   db.query( q );

   while ( db.next() )
   {
      bufferIDs    << db.value( 0 ).toString();
      descriptions << db.value( 1 ).toString();
      GUIDs        << "";
   }

   lw_buffer_db->clear();
   
   if ( descriptions.size() == 0 )
   {
      lw_buffer_db->addItem( "No buffer files found." );
   }
   else
   { 
      le_search->setReadOnly( false );
      le_search->setPalette ( normal );
      search();
   }
}

void US_BufferGui::search( const QString& text )
{
   QString     sep = ";";
   QStringList sortdesc;
   lw_buffer_db  ->clear();
   buffer_metadata.clear();
   sortdesc       .clear();

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

   for ( int jj = 0; jj < sortdesc.size(); jj++ )
   {  // build list of sorted meta data and ListWidget entries
      int ii     = sortdesc[ jj ].section( sep, 1, 1 ).toInt();

      BufferInfo info;
      info.index       = ii;
      info.description = descriptions[ ii ];
      info.guid        = GUIDs       [ ii ];
      info.bufferID    = bufferIDs   [ ii ];

      buffer_metadata << info;

      lw_buffer_db->addItem( info.description );
   }
}

void US_BufferGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Connection Problem" ),
         tr( "Could not connect to databasee \n" ) + error );
}

void US_BufferGui::spectrum( void )
{
   QString   spectrum_type = cmb_optics->currentText();
   US_Table* dialog;
   QString   s = tr( "Extinction:" );

   if ( spectrum_type == tr( "Absorbance" ) )
     dialog = new US_Table( buffer.extinction, s, bufferCurrent );
   else if ( spectrum_type == tr( "Interference" ) )
     dialog = new US_Table( buffer.refraction, s, bufferCurrent );
   else
     dialog = new US_Table( buffer.fluorescence, s, bufferCurrent );

   dialog->setWindowTitle( tr( "Manage %1 Values" ).arg( spectrum_type ) );
   dialog->exec();
}

/*! \brief Display the appropriate data when the buffer name in the list widget
           is selected with a double click.
           \param item The description of the buffer selected.
*/
void US_BufferGui::select_buffer( QListWidgetItem* item )
{
   if ( ! disk_controls->db() ) read_from_disk( item );
   else                         read_from_db  ( item ); 
   
   // Write values to screen
   le_description->setText( buffer.description );
   le_guid       ->setText( buffer.GUID );
   le_density    ->setText( QString::number( buffer.density,   'f', 6 ) );
   le_viscosity  ->setText( QString::number( buffer.viscosity, 'f', 5 ) );
   le_ph         ->setText( QString::number( buffer.pH,        'f', 4 ) );
   le_compressibility->setText( 
                      QString::number( buffer.compressibility, 'e', 4 ) );

   lw_buffer->clear();

   for ( int i = 0; i < buffer.componentIDs.size(); i ++ )
      update_lw_buf( buffer.componentIDs[ i ], buffer.concentration[ i ] );

   // Allow modification of the just selected buffer
   bufferCurrent = true;
   pb_save  ->setEnabled ( true ); 
   pb_update->setEnabled ( true ); 
   pb_del   ->setEnabled ( true ); 
}

void US_BufferGui::read_from_disk( QListWidgetItem* item )
{
   int row = lw_buffer_db->row( item );
   int buf = buffer_metadata[ row ].index;

   if ( ! buffer.readFromDisk( filenames[ buf ] ) )
      qDebug() << "read failed";

   buffer.component.clear();
   cb_shared ->setChecked( false );

   for ( int i = 0; i < buffer.componentIDs.size(); i++ )
   {
      QString index = buffer.componentIDs[ i ];
      buffer.component << component_list[ index ];
   }
}

void US_BufferGui::read_from_db( QListWidgetItem* item )
{
   int row = lw_buffer_db->row( item );
   QString bufferID = buffer_metadata[ row ].bufferID;
   read_from_db( bufferID );
}

void US_BufferGui::read_from_db( const QString& bufferID )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   
   // Get the buffer data from the database
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }
  
   buffer.readFromDB( &db, bufferID );
}

void US_BufferGui::update_lw_buf( const QString& componentID, double conc )
{
   QString name = component_list[ componentID ].name;
   QString unit = component_list[ componentID ].unit;
   
   QString s = QString::number( conc, 'f', 1 );
         
   lw_buffer->addItem( name + " (" + s + " " + unit + ")" );
}

void US_BufferGui::update_buffer( void )
{
   buffer.description     = le_description->text();
                          
   buffer.pH              = ( le_ph->text().isEmpty() ) 
                            ? 7.0 
                            : le_ph->text().toDouble();
   
   buffer.density         = le_density    ->text().toDouble();
   buffer.viscosity       = le_viscosity  ->text().toDouble();
   buffer.compressibility = le_compressibility->text().toDouble();

   // These are updated in other places
   //buffer.component
   //buffer.concentration
   //buffer.bufferID
   //buffer.personID
}

void US_BufferGui::delete_buffer( void )
{
   if ( buffer.GUID.size() == 0 || lw_buffer_db->currentRow() < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select the buffer which you "
                "want to delete." ) );
      return;
   }

   int response = QMessageBox::question( this, 
            tr( "Confirmation" ),
            tr( "Do you really want to delete this entry?\n"
                "Clicking 'OK' will delete the selected buffer data." ),
            QMessageBox::Ok, QMessageBox::Cancel );
   
   if ( response != QMessageBox::Ok ) return;
   
   if ( disk_controls->db() )
      delete_db();
   else
      delete_disk();

   reset();
   query();
   bufferCurrent = true;
}

void US_BufferGui::delete_disk( void )
{
   QString bufGUID = le_guid->text();
   QString path;
   if ( ! buffer_path( path ) ) return;

   bool    newFile;
   QString filename = US_Buffer::get_filename( path, bufGUID, newFile );

   if ( buffer_in_use( bufGUID ) )
   {
      QMessageBox::warning( this,
         tr( "Buffer Not Deleted" ),
         tr( "The buffer could not be deleted,\n"
             "since it is in use in one or more solutions." ) );
      return;
   }

   if ( ! newFile )
   {
      QFile f( filename );
      f.remove();
   }
}

// Delete the buffer data from the database
void US_BufferGui::delete_db( void )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }
   
   QStringList q( "get_bufferID" );
   q << le_guid->text();

   db.query( q );

   int status = db.lastErrno();
   
   if (  status == US_DB2::OK )
   {
      db.next(); 
      QString bufferID = db.value( 0 ).toString();
  
      q[ 0 ] = "delete_buffer";
      q[ 1 ] = bufferID;
      status = db.statusQuery( q );
   }

   if ( status == US_DB2::BUFFR_IN_USE )
   {
      QMessageBox::warning( this,
         tr( "Buffer Not Deleted" ),
         tr( "The buffer could not be deleted,\n"
             "since it is in use in one or more solutions." ) );
      return;
   }

   if ( status != US_DB2::OK )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "Delete failed.\n\n" ) + db.lastError() );
   }
}

void US_BufferGui::save( void )
{
   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please enter a description for\n"
                "your buffer before saving it!" ) );
      return;
   }

   update_buffer();

   if ( le_guid->text().size() != 36 )
      le_guid->setText( US_Util::new_guid() );

   buffer.GUID = le_guid->text();

   if ( ! disk_controls->db() ) save_disk();
   else                         save_db(); 
   
   bufferCurrent = true;
}

void US_BufferGui::save_disk( void )
{
   QString path;
   if ( ! buffer_path( path ) ) return;

   if ( buffer.GUID.isEmpty()  ||  buffer.description.isEmpty() )
   {
      QMessageBox::information( this,
         tr( "NO Save of Results" ),
         tr( "The buffer with an empty GUID and/or description\n"
             "was not saved." ) );
      return;
   }

   bool    newFile;
   QString filename = US_Buffer::get_filename( path, buffer.GUID, newFile );
   buffer.writeToDisk( filename );

   QString s = ( newFile ) ? tr( "saved" ) : tr( "updated" );

   QMessageBox::information( this,
         tr( "Save results" ),
         tr( "Buffer " ) + s );

   read_buffer();
}

void US_BufferGui::save_db( void )
{
   if ( personID < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an investigator first!" ) );
      return;
   }

   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
         
   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return;
   }

   QString private_buffer = ( cb_shared->isChecked() ) ? "0" : "1";
   int idBuf = buffer.saveToDB( &db, private_buffer );

   if ( idBuf < 0 )
   {
      QString msg = tr( "( Return Code = %1 ) " ).arg( idBuf )
                    + db.lastError();

qDebug() << "savDB error" << db.lastErrno() << db.lastError() << idBuf;
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Error updating buffer in the database:\n" )
            + msg );

      return;
   }

   //reset();
   read_db();
}

void US_BufferGui::update( void )
{
   update_buffer();
   buffer.GUID = le_guid->text();

   if ( ! disk_controls->db() ) save_disk();
   else                         update_db(); 

   bufferCurrent = true;
}

/*!  Update changed buffer data to DB table  */
void US_BufferGui::update_db( void )
{
   if ( buffer.bufferID.toInt() <= 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an existing Buffer first!" ) );
      return;
   }
   
   //if ( buffer.personID <= 0 )
   if ( personID <= 0 )
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
         
      QString private_buffer = ( cb_shared->isChecked() ) ? "0" : "1";
      int idBuf = buffer.saveToDB( &db, private_buffer );

      if ( idBuf < 0 )
      {
         QString msg = tr( "( Return Code = %1 ) " ).arg( idBuf )
                       + db.lastError();
qDebug() << "updDB error" << db.lastErrno() << db.lastError() << idBuf;

         QMessageBox::information( this,
               tr( "Attention" ), 
               tr( "Error updating buffer in the database:\n" )
               + msg );
         return;
      }

      QMessageBox::information( this, 
           tr( "Success" ),
           tr( "The database has been updated\n" ) );
   }
}

/*!  Input the value of component selected in lw_ingredients. After 'Return'
 *   key was pressed, this function will display the selected component value
 *   in lw_buffer and recalculate the density and viscosity.  */
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

   if ( manualUpdate ) lw_buffer->clear();

   QString          s;
   bool             newItem = true;
   int              row     = lw_ingredients->currentRow();
   QListWidgetItem* item    = lw_ingredients->item( row );
   QString          index   = QString::number( item->type() );

   US_BufferComponent std_bc = component_list[ index ];
   
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
      buffer.concentration << partial_concentration;
      buffer.component     << std_bc;
      buffer.componentIDs  << std_bc.componentID;
      
      s.sprintf( " (%.1f ", partial_concentration );
      lw_buffer->addItem( std_bc.name + s + std_bc.unit + ")" );
   }

   recalc_density();
   recalc_viscosity();

   le_density      ->setText( QString::number( buffer.density,   'f', 6 ) );
   le_viscosity    ->setText( QString::number( buffer.viscosity, 'f', 5 ) );
   le_concentration->setText( "" );
   le_compressibility
                   ->setText( QString::number( buffer.compressibility, 'e', 4));
   
   pb_save->setEnabled( true );
   bufferCurrent = false;
   manualUpdate  = false;
}

/*!  After selection of the buffer component in lw_ingredients, this method will
     display in lb_selected and wait for a partial concentartion input value. */
void US_BufferGui::list_component( void )
{
   QString id = QString::number( lw_ingredients->currentRow() );

   lb_selected->setText( lw_ingredients->currentItem()->text() );
   lb_units->setText( tr( "Please enter with units in: " ) + 
         component_list[ id ].unit );
   
   le_concentration->setFocus();
}

/*! When double clicked, the selected item in lw_buffer
    will be removed  and the density and viscosity will be recalculated.  */
void US_BufferGui::remove_component( QListWidgetItem* item )
{
   if ( manualUpdate ) return;
   int row = lw_buffer->row( item );

   buffer.component    .removeAt( row );
   buffer.concentration.removeAt( row );
   buffer.componentIDs .removeAt( row );
   
   recalc_viscosity();
   recalc_density();
   
   le_density  ->setText( QString::number( buffer.density,   'f', 6 ) );
   le_viscosity->setText( QString::number( buffer.viscosity, 'f', 5 ) );
   
   QListWidgetItem* oldItem = lw_buffer->takeItem( row );
   delete oldItem;

   bufferCurrent = false;
}

bool US_BufferGui::up_to_date( void )
{
   if ( ! bufferCurrent                                                 ||
        le_description    ->text()            != buffer.description     ||
        le_compressibility->text().toDouble() != buffer.compressibility ||
        le_ph             ->text().toDouble() != buffer.pH )
      return false;

   return true;
}

void US_BufferGui::accept_buffer( void )
{
   if ( ! up_to_date()  &&  signal )
   {
      int response = QMessageBox::question( this,
            tr( "Buffer changed" ),
            tr( "Changes have not been saved.\n\nContinue?" ),
            QMessageBox::Yes, QMessageBox::Cancel );
      
      if ( response != QMessageBox::Yes ) return;
   }

   if ( signal ) 
   {
      update_buffer();
      buffer.GUID = le_guid->text();
      emit valueChanged ( buffer.density, buffer.viscosity );
      emit valueChanged ( buffer );
      emit valueBufferID( buffer.bufferID );
   }

   accept();
}

/*! Reset some variables to initialization. */
void US_BufferGui::reset( void )
{
   buffer = US_Buffer();

   view_shared = false;
   le_search         ->clear();;
   le_search         ->setReadOnly( false );

   le_guid           ->clear();
                    
   lw_buffer_db      ->clear();
   lw_buffer         ->clear();
                    
   lb_selected       ->setText( "" );
                    
   le_density        ->setText( "0.0" );
   le_viscosity      ->setText( "0.0" );
                     
   le_description    ->clear();
   le_compressibility->clear();
   
   le_ph             ->setText( "7.0" );
                   
   pb_save           ->setEnabled( false );
   pb_update         ->setEnabled( false );
   pb_del            ->setEnabled( false );
   cb_shared         ->setChecked( false );
                     
   lb_units          ->setText( "" );
   le_concentration  ->clear();

   int id = US_Settings::us_inv_ID();
   QString number = ( id > 0 ) ? QString::number( id ) + ": " : "";
   le_investigator->setText( number + US_Settings::us_inv_name() );
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
   buffer.viscosity = VISC_20W;

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
         - VISC_20W;
      }
   }
}

// slot to handle an entered buffer description
void US_BufferGui::new_description()
{
   buffer.description = le_description->text();

   int row = -1;

   for ( int ii = 0; ii < descriptions.size(); ii++ )
   {
      if ( buffer.description == descriptions.at( ii ) )
      {
         row   = ii;
         break;
      }
   }

   pb_update->setEnabled( row >= 0 );
   pb_save  ->setEnabled( row < 0  );

   if ( row < 0 )
   {  // no match to description:  clear GUID, de-select any list item
      le_guid->clear();
      lw_buffer_db->setCurrentRow( -1 );
   }

   else
   {  // matching description:  get GUID, but ask user if new or update
      buffer.GUID   = GUIDs[ row ];

      if ( buffer.GUID.isEmpty()  &&  disk_controls->db() )
      {  // if no GUID yet and from DB, read GUID
         QString   bufferID = bufferIDs[ row ];
         US_Passwd pw;
         US_DB2    db( pw.getPasswd() );

         if ( db.lastErrno() != US_DB2::OK )
            connect_error( db.lastError() );

         QStringList q( "get_buffer_info" );
         q << bufferID;

         db.query( q );
         db.next(); 

         buffer.bufferID = bufferID;
         buffer.GUID     = db.value( 0 ).toString();
      }

      int response = QMessageBox::question( this,
         tr( "Update Buffer?" ),
         tr( "The buffer description is already used.\n"
             "Do you wish to replace that buffer?\n\n"
             "Click \"No\" to create a new buffer;\n"
             "Click \"Yes\" to update the existing buffer.\n" ),
         QMessageBox::Yes, QMessageBox::No );

      row = -1;

      if ( response == QMessageBox::No )
      {  // new description (even if duplicate)
         buffer.GUID.clear();
         pb_update->setEnabled( false );
         pb_save  ->setEnabled( true  );
      }

      else
      {  // find description in list if possible
         for ( int ii = 0; ii < lw_buffer_db->count(); ii++ )
         {
            if ( buffer.description == lw_buffer_db->item( ii )->text() )
            {
               row = ii;
               break;
            }
         }
      }

      // select any match in list; set existing GUID
      lw_buffer_db->setCurrentRow( row );
      le_guid->setText( buffer.GUID );
   }
}

void US_BufferGui::source_changed( bool db )
{
   emit use_db( db );
   query();
   qApp->processEvents();
}

// Determine by GUID whether a buffer is in use in any solution on disk
bool US_BufferGui::buffer_in_use( QString& bufferGUID )
{
   bool in_use = false;
   QString soldir = US_Settings::dataDir() + "/solutions/";
   QStringList sfilt( "S*.xml" );
   QStringList snames = QDir( soldir )
      .entryList( sfilt, QDir::Files, QDir::Name );

   for ( int ii = 0;  ii < snames.size(); ii++ )
   {
      QString sfname = soldir + snames.at( ii );
      QFile sfile( sfname );

      if ( ! sfile.open( QIODevice::ReadOnly | QIODevice::Text ) ) continue;

      QXmlStreamReader xml( &sfile );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "buffer" )
         {
            QXmlStreamAttributes atts = xml.attributes();

            if ( atts.value( "guid" ).toString() == bufferGUID )
            {
               in_use = true;
               break;
            }
         }
      }

      sfile.close();

      if ( in_use )  break;
   }

   return in_use;
}

