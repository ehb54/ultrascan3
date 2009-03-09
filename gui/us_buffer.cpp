//! \file us_buffer.cpp
#include "us_buffer.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_db.h"
#include "us_passwd.h"
#include "us_constants.h"

US_Buffer_DB::US_Buffer_DB() : US_WidgetsDialog( 0, 0 )
{
   read_template_file();

   buffer.component.clear();
   buffer.refractive_index = 0.0;
   buffer.density          = 0.0;
   buffer.viscosity        = 0.0;
   buffer.description      = "";
   buffer.bufferID         = -1;
   buffer.investigatorID   = -1;

   fromHD = false;

   setWindowTitle( tr( "Buffer Management" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );

   QGridLayout* main = new QGridLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   int row = 0;

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   main->addWidget( pb_investigator, row++, 0 );

   /*
   if ( temp_InvID > 0 )
   {
      lbl_investigator->setText( show_investigator( temp_InvID ) );
      Buffer.investigatorID = temp_InvID;
   }
   */

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

   main->addWidget( lw_buffer_db, row, 0, 7, 1 );

   row += 7;

   // Labels
   QLabel* lb_description = us_label( tr( "Buffer Description:" ) );
   main->addWidget( lb_description, row++, 0 );

   QLabel* lb_buffer1 = us_label( tr( "Please select a Buffer Component:" ) );
   main->addWidget( lb_buffer1, row++, 0 );

   lb_selected = us_label( "" );
   main->addWidget( lb_selected, row++, 0 );

   // Ingredients
   QLabel* lb_banner2 = us_banner( tr( "Click on item to select" ), -2  );
   main->addWidget( lb_banner2, row++, 0 );

   lw_ingredients = us_listwidget();

   for ( int i = 0; i < component_list.size(); i++ )
      lw_ingredients->addItem( component_list[ i ].name +  
                        " (" + component_list[ i ].range + ")" );

   connect( lw_ingredients, SIGNAL( itemSelectionChanged( void ) ), 
                            SLOT  ( list_component      ( void ) ) );
   
   main->addWidget( lw_ingredients, row, 0, 6, 1 );

   row = 0;

   // Investigator
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1, 1, 2 );

   // Pushbuttons
   QPushButton* pb_load = us_pushbutton( tr( "Load Buffer from HD" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( read_buffer() ) );
   main->addWidget( pb_load, row++, 1, 1, 2 );

   QPushButton* pb_load_db = us_pushbutton( tr( "Query Buffer Descriptions from DB" ) );
   connect( pb_load_db, SIGNAL( clicked() ), SLOT( read_db() ) );
   main->addWidget( pb_load_db, row++, 1, 1, 2 );

   pb_save = us_pushbutton( tr( "Save Buffer to HD" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_buffer() ) );
   main->addWidget( pb_save, row++, 1, 1, 2 );

   pb_save_db = us_pushbutton( tr( "Backup Buffer to DB" ), false );
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

   QLabel* lb_refraction = 
      us_label( tr( "Refractive Index (20<span>&deg;</span>C):" ) );
   main->addWidget( lb_refraction, row, 1 );

   le_refraction = us_lineedit();
   main->addWidget( le_refraction, row++, 2 );

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

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons, row, 0, 1, 3 );
   reset();
}

//! \brief Read buffer.dat file and get buffer component values.
bool US_Buffer_DB::read_template_file( void )
{
   QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
   QFile f( home + "/etc/buffer.dat" );
   
   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts( &f );

      QString s = ts.readLine();
      int components = s.toInt();
      
      component_list.clear();
      
      for ( int i = 0; i < components; i++ )
      {
         struct BufferIngredient ingredient;

         ingredient.name = ts.readLine();
         ingredient.unit = ts.readLine();
         
         for ( int j = 0; j < 6; j++ )
            ts >> ingredient.dens_coeff[ j ];

         ingredient.range = ts.readLine(); // Read rest of line
         
         for ( int j = 0; j < 6; j++ )
            ts >> ingredient.visc_coeff[ j ];

         ts.readLine();  // Skip to end of line

         component_list << ingredient;
      }

      f.close();
      return true;
   }
   else
   {
      QMessageBox::information( this, 
            tr( "Attention" ),
            tr( "UltraScan can not find the buffer definition file:\n\n" ) +
                home +  "/etc/buffer.dat" + 
                tr( "\n\nPlease re-install the buffer definition file." ) );

      return false;
   }
}

void US_Buffer_DB::sel_investigator( void )
{
   buffer.investigatorID   = 999;
   QMessageBox::information( this, "Under construction", "Not implemented yet." );
/*   US_DB_TblInvestigator* investigator_dlg = new US_DB_TblInvestigator;

   investigator_dlg->setCaption("Investigator Information");
   investigator_dlg->pb_exit->setText("Accept");
   connect( investigator_dlg, SIGNAL( valueChanged( QString, int ) ), 
                              SLOT  ( update_investigator_lbl(QString, int)));
   investigator_dlg->exec();

   lb_current_buffer->clear();
   lbl_buffer2        ->setText("");
   le_description     ->setText("");
   le_density         ->setText(" 0.0");
   le_viscosity       ->setText(" 0.0");
   le_refractive_index->setText(" 0.0");
   
   Buffer.component.clear();
   read_db();
   */
}

/*! Load buffer data from database table tblBuffer and populate listbox. If
    an investigator is defined, only select the buffer files from the 
    investigator. */

void US_Buffer_DB::read_db()
{
   US_Passwd pw;
   US_DB*    db = new US_DB;;
   QString   error;
   
   if ( ! db->open( pw.getPasswd(), error ) )
   {
      // Error message here

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

   QString query;

   if ( buffer.investigatorID > 0)
      query = "SELECT BuffID, BufferDescription FROM tblBuffer "
              "WHERE InvestigatorID = " + 
              QString::number( buffer.investigatorID ) +
              " ORDER BY BuffID DESC";
   else
      query = "SELECT BuffID, BufferDescription FROM tblBuffer "
              "ORDER BY BuffID DESC;";

   db->query( query );

   while ( db->next() )
   {
      info.bufferID    = db->value( 0 ).toInt();
      info.description = db->value( 1 ).toString();
  
      buffer_metadata << info;

      lw_buffer_db->addItem( new QListWidgetItem(
            QString::number( info.bufferID ) + ": " + info.description, lw_buffer_db ) );
   }

   delete db;
   QSqlDatabase::removeDatabase( "UltraScan" );

   fromHD = false;

   if ( buffer_metadata.size() < 1 && buffer.investigatorID > 0 )
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

void US_Buffer_DB::search( const QString& text )
{
   lw_buffer_db->clear();
   for ( int i = 0; i < buffer_metadata.size(); i++ )
   {
      struct buffer_info* info = &buffer_metadata[ i ];

      if ( info->description.contains( QRegExp( ".*" + text + ".*" ) ) )
      {
        lw_buffer_db->addItem( new QListWidgetItem( 
              QString::number( info->bufferID ) + ": " + info->description ) );
      }
   }
}

/*! \brief Display the appropriate data when the buffer name in the list widget
 *  is selected with a double click.
   
    \param item The description of the buffer selected.
*/
void US_Buffer_DB::select_buffer( QListWidgetItem* item )
{
   if ( fromHD ) return;

   QStringList description = item->text().split( ':' );
   
   QString query = "SELECT InvestigatorID, BufferDescription, BufferData,"
           "RefractiveIndex20, Density20, Viscosity20 "
           "FROM tblBuffer "
           "WHERE BuffID=" + description[ 0 ];

   US_DB*    db = new US_DB;
   US_Passwd pw;
   QString   error;
   
   // Get the buffer data from the database
   if ( ! db->open( pw.getPasswd(), error ) )
   {
      // Error message here

      return;
   }

   db->query( query );
   db->next(); 
   
   QString InvestigatorID  = db->value( 0 ).toString();

   buffer.investigatorID   = InvestigatorID.toInt();
   buffer.description      = db->value( 1 ).toString();
   buffer.data             = db->value( 2 ).toString();
   buffer.refractive_index = db->value( 3 ).toDouble();
   buffer.density          = db->value( 4 ).toDouble();
   buffer.viscosity        = db->value( 5 ).toDouble();
   buffer.bufferID         = description[ 0 ].toInt();
   
   // Parse the components and display them
   QStringList components = buffer.data.split( '\n' );
   int         size       = components[ 0 ].toInt();

   buffer.component.clear();
   lw_buffer->clear();

   for ( int i = 0; i < size; i++ )
   {
      struct BufferIngredient bi;
      bi.name                  = components[ i * 2 + 1 ];
      bi.partial_concentration = components[ i * 2 + 2 ].toDouble();
      buffer.component << bi;

      QString s;
      lw_buffer->addItem( bi.name + 
            " (" + s.sprintf( "%.1f", bi.partial_concentration ) + " mM)" );
   }

   // Get the investigator's name and display it

   query = "SELECT FirstName, LastName FROM tblInvestigators WHERE InvID =" + 
      InvestigatorID;

   db->query( query );
   db->next(); 

   QString fname = db->value( 0 ).toString();
   QString lname = db->value( 1 ).toString();

   le_investigator->setText( "InvID (" + InvestigatorID + ") " + fname + " " + lname );

   delete db;
   QSqlDatabase::removeDatabase( "UltraScan" );

   // Update the rest of the buffer data
   
   le_description->setText( buffer.description );

   QString s;
   le_density   ->setText( s.sprintf( " %6.4f", buffer.density          ) );
   le_viscosity ->setText( s.sprintf( " %6.4f", buffer.viscosity        ) );
   le_refraction->setText( s.sprintf( " %6.4f", buffer.refractive_index ) );
   
   // Allow modification of the just selected buffer from the DB
   pb_update_db->setEnabled ( true ); 
   pb_del_db   ->setEnabled ( true ); 

   le_viscosity->setReadOnly( true );
   le_density  ->setReadOnly( true );
}

/*! Load buffer data from Hard Drive, add warning message when 
 *  this widget is called by US_Cell_DB */
void US_Buffer_DB::read_buffer( void )
{
   /*
   if(cell_flag)
   {
      QMessageBox::message(
        tr("Attention:"),
        tr("Please use the 'Query Buffer from DB' button\n"
           "to select a buffer file from the database.\n\n"
           "If the desired file is not stored in the database,\n"
           "you have to click on 'Backup Buffer to DB' first, store\n"
           "a buffer file to the database, then select it from the database."));
   }
   */

   if ( buffer.investigatorID < 1 )
      le_investigator->setText( "Not Selected" );
   
   QString filename = QFileDialog::getOpenFileName( this, 
         tr( "Select the buffer fiel to load" ), 
         US_Settings::dataDir(), "*.buf" );

   if ( filename.isEmpty() ) return;


   lw_buffer->clear();
   
   lw_buffer_db->clear();
   lw_buffer_db->addItem( filename );
   
   lw_buffer_db->addItem( "Current Buffer was loaded" );
   lw_buffer_db->addItem( "from Hard Drive" );
   fromHD = true;

   QFile f( filename );
   if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream ts ( &f );

      buffer.description = ts.readLine();
      le_description->setText( buffer.description );
      
      int counter;

      ts >> counter;
      ts.readLine(); // Skip to eol

      ts >> buffer.density;
      ts.readLine(); // Skip to eol
      
      ts >> buffer.viscosity;
      ts.readLine(); // Skip to eol
      
      
      buffer.component.clear();
      
      for ( int i = 0; i < counter; i++ )
      {
         struct BufferIngredient bi;

         bi.name = ts.readLine();
         
         ts >> bi.partial_concentration;
         ts.readLine(); // Skip to eol

         buffer.component << bi;

         QString s;
         s.sprintf( " (%.1f mM)", bi.partial_concentration );
         lw_buffer->addItem( bi.name + s );
      }

      if ( ! ts.atEnd() )
      {
         ts >> buffer.refractive_index;
         ts.readLine(); // Skip to eol
      }

      f.close();

      // This buffer is from disk, we flag this with a negative ID
      buffer.bufferID = -1;

      if ( buffer.component.size() > 0 )
      {
         if ( buffer.density   == 0.0 ) recalc_density();
         if ( buffer.viscosity == 0.0 ) recalc_viscosity();
      }
      else // Assume water
      {

         if ( buffer.viscosity == 0.0 ) buffer.viscosity = 100.0 * VISC_20W;
         if ( buffer.density   == 0.0 ) buffer.density   = DENS_20W; 
      }

      QString s;
      le_density   ->setText( s.sprintf( "%6.4f", buffer.density          ) );
      le_viscosity ->setText( s.sprintf( "%6.4f", buffer.viscosity        ) );
      le_refraction->setText( s.sprintf( "%6.4f", buffer.refractive_index ) );
   }
   else
   {
      QMessageBox::information( this, 
            tr( "Attention" ), 
            tr( "UltraScan can not open the selected buffer file:\n\n" ) +
                filename + 
                tr( "\n\nPlease check for read permission on drive." ) );
      return;
   }

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
   //emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
   //emit valueChanged(Buffer.density, Buffer.viscosity);
}

/*! Save buffer data to disk drive in *.buf format. */
void US_Buffer_DB::save_buffer( void )
{
   if ( le_description->text().isEmpty() )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please enter a description for\n"
                "your buffer before saving it!" ) );
      return;
   }

   QString filename = QFileDialog::getSaveFileName( this,
           tr( "Enter the buffer filename" ),
           US_Settings::dataDir(), "*.buf" );
   
   if ( filename.isEmpty() ) return;

   if ( filename.right( 4 ) != ".buf" ) filename.append( ".buf" );
   
   switch ( QMessageBox::question( this, 
             tr( "UltraScan - Buffer:" ),
             tr( "Click 'OK' to save buffer information to :\n" ) + filename ,
             QMessageBox::Ok, QMessageBox::Cancel ) ) 
   {
      case QMessageBox::Ok:
      {
         update_buffer();
         QFile f( filename );
         if ( f.open( QIODevice::WriteOnly | QIODevice::Text  ) )
         {
            QTextStream ts ( & f);

            ts << buffer.description      << endl;
            ts << buffer.component.size() << endl;
            ts << buffer.density          << endl;
            ts << buffer.viscosity        << endl;
            
            for ( int i = 0; i < buffer.component.size(); i++ )
            {
               ts << buffer.component[ i ].name                  << endl;
               ts << buffer.component[ i ].partial_concentration << endl;
            }

            ts << buffer.refractive_index;
            f.close();
         }
         else
            QMessageBox::information( this,
                  tr( "Attention" ), 
                  tr( "Unable to write the buffer to a file!\n"
                      "Please check diskspace and make sure\n"
                      "that the permissions allow writing and\n"
                      "the disk is not write-protected." ) );
         break;
      }

      default:
         break;
   }
}

void US_Buffer_DB::update_buffer( void )
{
   buffer.description      = le_description->text();
   buffer.refractive_index = le_refraction ->text().toDouble();
   buffer.density          = le_density    ->text().toDouble();
   buffer.viscosity        = le_viscosity  ->text().toDouble();
   //buffer.component
   //buffer.data
   //buffer.bufferID
   //buffer.investifatorID
}

void US_Buffer_DB::delete_buffer( void )
{
   if ( buffer.bufferID < 0 || lw_buffer_db->currentRow() < 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ),
            tr( "First select the buffer which you\n"
                "want to delete from the database" ) );
      return;
   }

   switch ( QMessageBox::information( this, 
            tr( "Confirmation" ),
            tr( "Do you really want to delete this entry?\n"
                "Clicking 'OK' will delete the selected buffer data "
                "from the database." ),
            QMessageBox::Ok, QMessageBox::Cancel ) )
   {
      case 0:
      {
         {
            US_DB     db;
            US_Passwd pw;
            QString   error;
               
            if ( ! db.open( pw.getPasswd(), error ) )
            {
               // Error message here
          
               return;
            }
          
            QString query = "DELETE FROM tblBuffer WHERE BuffID =" +
                    QString::number( buffer.bufferID );
           
            db.query( query );

            if ( db.lastQueryErrorType() != QSqlError::NoError )
            {
               QMessageBox::information( this,
                     tr( "Attention" ),
                     tr( "Delete failed.\n"
                         "Attempted to execute this command:\n\n" )
                         + query + "\n\n"
                         + tr( "Causing the following error:\n\n" )
                         + db.lastQueryErrorText() );
            }
         }

         QSqlDatabase::removeDatabase( "UltraScan" );

         reset();
         read_db();
         
         break;
      }
      default:
            break;
   }


}

void US_Buffer_DB::save_db( void )
{
   if ( buffer.investigatorID < 0 )
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

   buffer.data.sprintf( "%d", buffer.component.size() );

   for ( int i = 0; i < buffer.component.size(); i++ )
   {
      buffer.data += "\n" + buffer.component[ i ].name;
      buffer.data += 
         QString::number( buffer.component[ i ].partial_concentration );
   }

   switch ( QMessageBox::information( this, 
            tr( "UltraScan - Buffer Database" ),
            tr( "Click 'OK' to save the selected\n"
                "buffer file into the database" ),
            QMessageBox::Ok, QMessageBox::Cancel ) )
   {
      case 0:
      {
         QString query = 
            "INSERT INTO tblBuffer( BufferDescription, BufferData, Density20, "
            "Viscosity20, RefractiveIndex20, InvestigatorID) VALUES("  
              "'" + buffer.description                   + "'," 
              "'" + buffer.data                          + "',"
            + QString::number( buffer.density )          + ","
            + QString::number( buffer.viscosity )        + ","
            + QString::number( buffer.refractive_index ) + ","
            + QString::number( buffer.investigatorID )   + ")";
            
            {
               US_DB     db;
               US_Passwd pw;
               QString   error;
                  
               if ( ! db.open( pw.getPasswd(), error ) )
               {
                  // Error message here
             
                  return;
               }
             
               db.query( query );
            }

            QSqlDatabase::removeDatabase( "UltraScan" );

            reset();
            read_db();

         break;
      }
      default:
         break;
   }
}

/*!  Update changed buffer data to DB table tblBuffer */
void US_Buffer_DB::update_db( void )
{
   QString str;

   if ( buffer.bufferID <= 0 )
   {
      QMessageBox::information( this,
            tr( "Attention" ), 
            tr( "Please select an existing Buffer first!" ) );
      return;
   }
   
   if ( buffer.investigatorID <= 0 )
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
   QString s;

   buffer.data.sprintf( "%d", buffer.component.size() );

   for ( int i = 0; i < buffer.component.size(); i++ )
   {
      buffer.data += "\n" + buffer.component[ i ].name;
      buffer.data += "\n" + 
         QString::number( buffer.component[ i ].partial_concentration );
   }

   switch ( QMessageBox::information( this, 
            tr( "UltraScan - Buffer Database" ),
            tr( "Click 'OK' to update buffer " ) + 
            QString::number( buffer.bufferID ) + tr( "\nin the database" ),
            QMessageBox::Ok, QMessageBox::Cancel ) )
   {
      case QMessageBox::Ok:
      {
         QString query = "UPDATE tblBuffer SET "
            "BufferDescription='" + buffer.description                         + "'," 
            "BufferData='"        + buffer.data                                + "',"
            "Density20="          + QString::number( buffer.density )          + ","
            "Viscosity20="        + QString::number( buffer.viscosity )        + ","
            "RefractiveIndex20="  + QString::number( buffer.refractive_index ) + ","
            "InvestigatorID="     + QString::number( buffer.investigatorID )   + " " 
            "WHERE BuffID="       + QString::number( buffer.bufferID );

         US_DB     db;
         US_Passwd pw;
         QString   error;
            
         // Get the buffer data from the database
         if ( ! db.open( pw.getPasswd(), error ) )
         {
            // Error message here

            return;
         }

         db.query( query );
      }

      QSqlDatabase::removeDatabase( "UltraScan" );
      break;

      default:
         break;
   }
}

/*!  Input the value of component  selected in lw_ingredients.  After 'Return'
 *   key was pressed, this function will dispaly the selected component value in
 *   lw_buffer and recalculate the density and viscosity.  */

void US_Buffer_DB::add_component( void )
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

   struct BufferIngredient* std_bi  = &component_list[ current ];
   
   // Find out if this inredient already exists, otherwise add a new component
   for ( int i = 0; i < buffer.component.size(); i++ ) 
   {
      struct BufferIngredient* bi = &buffer.component[ i ];

      if ( std_bi->name == bi->name )
      {
         // Simply update the partial concentration of the existing ingredient
         bi->partial_concentration = partial_concentration;

         s.sprintf( " (%.1f ", partial_concentration );

         lw_buffer->item( i )->setText( bi->name + s + bi->unit + ")" );
         newItem = false;
         break;
      }
   }

   // Add a new ingredient to this buffer
   if ( newItem ) 
   {
      struct BufferIngredient ingredient;
      
      ingredient.partial_concentration = partial_concentration;
      ingredient.range                 = std_bi->range;
      ingredient.unit                  = std_bi->unit;
      ingredient.name                  = std_bi->name;
      
      for ( unsigned int i = 0; i < 6; i++ )
      {
         ingredient.dens_coeff[ i ] = std_bi->dens_coeff[ i ];
         ingredient.visc_coeff[ i ] = std_bi->visc_coeff[ i ];
      }

      buffer.component << ingredient;

      s.sprintf( " (%.1f ", partial_concentration );
      lw_buffer->addItem( ingredient.name + s + ingredient.unit + ")" );
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

/*!  After you select the buffer component in lw_ingredients, this method will
     display in lbl_buffer2  and wait for a partial concentartion input value.
*/

void US_Buffer_DB::list_component( void )
{
   int row = lw_ingredients->currentRow();

   lb_selected->setText( lw_ingredients->currentItem()->text() );
   lb_units->setText( tr( "Please enter with units in: " ) + 
         component_list[ row ].unit );
   
   le_concentration->setReadOnly( false );
   le_concentration->setFocus();
}

/*! When double clicked, the selected item in lw_buffer
    will be removed  and the density and viscosity will be recalculated.
*/

void US_Buffer_DB::remove_component( QListWidgetItem* item )
{
   int row = lw_buffer->row( item );

   buffer.component.removeAt( row );
   
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
/*! Reset some variables to initialization. */
void US_Buffer_DB::reset()
{
   buffer.component.clear();
   buffer.refractive_index = 0.0;
   buffer.density          = 0.0;
   buffer.viscosity        = 0.0;
   buffer.description      = "";
   buffer.data             = "";
   buffer.bufferID         = -1;
   buffer.investigatorID   = -1;

   le_search       ->setText( "" );
   le_search       ->setReadOnly( false );

   lw_buffer_db    ->clear();
   lw_buffer       ->clear();

   lb_selected     ->setText( "" );

   le_investigator ->setText( "Not Selected" );

   le_density      ->setText( "0.0" );
   le_density      ->setReadOnly( false );
                   
   le_viscosity    ->setText( "0.0" );
   le_viscosity    ->setReadOnly( false );
                   
   le_refraction   ->setText( "0.0" );
   le_description  ->setText( "" );
                   
   pb_save         ->setEnabled( false );
   pb_save_db      ->setEnabled( false );
   pb_update_db    ->setEnabled( false );
   pb_del_db       ->setEnabled( false );
   
   lb_units        ->setText( "" );
   le_concentration->setText( "" );
   le_concentration->setReadOnly( true );
}

/*!  Recalculate the density of the buffer based on the information in the
     template file
*/

void US_Buffer_DB::recalc_density( void )
{
   buffer.density = DENS_20W;

   // Iterate over all components in this buffer
   for ( int i = 0; i < buffer.component.size(); i++ ) 
   {
      struct BufferIngredient* bi = &buffer.component[ i ];

      // Find the component in the buffer database structure
      for ( int j = 0; j < component_list.size(); j++ ) 
      {

         // ff we find it, assign the values
         if ( bi->name == component_list[ j ].name ) 
         {
            bi->unit  = component_list[ j ].unit;
            bi->range = component_list[ j ].range;
            
            for ( int k = 0; k < 6; k++ )
               bi->dens_coeff[ k ] = component_list[ j ].dens_coeff[ k ];


            double c1 = bi->partial_concentration;
            if ( bi->unit == "mM" ) c1 /= 1000;

            double c2 = c1 * c1; // c1^2
            double c3 = c2 * c1; // c1^3
            double c4 = c1 * c4; // c1^4 
            double croot = pow( c1, 0.5 );  // sqrt c1

            if ( bi->partial_concentration > 0.0 )
            {
               buffer.density += 
                 bi->dens_coeff[ 0 ] +
                 bi->dens_coeff[ 1 ] * 1.0e-3 * croot
               + bi->dens_coeff[ 2 ] * 1.0e-2 * c1
               + bi->dens_coeff[ 3 ] * 1.0e-3 * c2
               + bi->dens_coeff[ 4 ] * 1.0e-4 * c3
               + bi->dens_coeff[ 5 ] * 1.0e-6 * c4
               - DENS_20W;
            }
         }
      }
   }
}


/*!  Recalculate the viscosity of the buffer based on the information in the
     template file
 */

void US_Buffer_DB::recalc_viscosity( void )
{
   buffer.viscosity = 100.0 * VISC_20W;

   // Iterate over all components in this buffer
   for ( int i = 0; i < buffer.component.size(); i++) 
   {
      struct BufferIngredient* bi = &buffer.component[ i ];

      // Find the component in the buffer structure
      for ( int j = 0; j < component_list.size(); j++ ) 
      {
         // If we find it, assign the values
         if ( bi->name == component_list[ j ].name) 
         {
            bi->unit  = component_list[ j ].unit;
            bi->range = component_list[ j ].range;
            
            for ( int k = 0; k < 6; k++ )
               bi->visc_coeff[ k ] = component_list[ j ].visc_coeff[ k ];
            
            double c1 = bi->partial_concentration;
            if ( bi->unit == "mM" ) c1 /= 1000;

            double c2 = c1 * c1; // c1^2
            double c3 = c2 * c1; // c1^3
            double c4 = c1 * c4; // c1^4 
            double croot = pow( c1, 0.5 );  // sqrt c1
            
            if ( bi->unit == "mM" )
            {
               buffer.viscosity += 
                 bi->visc_coeff[ 0 ]  
               + bi->visc_coeff[ 1 ] * 1.0e-3 * croot
               + bi->visc_coeff[ 2 ] * 1.0e-2 * c1
               + bi->visc_coeff[ 3 ] * 1.0e-3 * c2 
               + bi->visc_coeff[ 4 ] * 1.0e-4 * c3
               + bi->visc_coeff[ 5 ] * 1.0e-6 * c4
               - 100.0 * VISC_20W;
            }
         }
      }
   }
}


