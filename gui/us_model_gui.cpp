//! \file us_model_gui.cpp

#include "us_model_gui.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_properties.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_associations_gui.h"
#include <uuid/uuid.h>

US_ModelGui::US_ModelGui( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Editor" );
   setPalette       ( US_GuiSettings::frameColor() );
   setAttribute     ( Qt::WA_DeleteOnClose );
   setWindowModality( Qt::WindowModal );
   
   model_descriptions.clear();

   // Very light gray
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   //US_Model m;
   //model = working_model = m;
   working_model = model;
   model_saved   = true;
  
   recent_row   = -1;
   investigator = -1;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   int row = 0;
   
   // Start widgets
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( get_person() ) );
   main->addWidget( pb_investigator, row, 0 );

   le_investigator = us_lineedit( );
   le_investigator->setPalette ( gray );
   le_investigator->setReadOnly( true );
   main->addWidget( le_investigator, row++, 1 );
   
   QGridLayout* db_layout   = us_radiobutton( tr( "Use Database" ),   rb_db );
   main->addLayout( db_layout, row, 0 );
   rb_db  ->setChecked( true  );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk );
   connect( rb_db, SIGNAL( clicked() ), SLOT( check_db() ) );
   rb_disk->setChecked( false );
   main->addLayout( disk_layout, row++, 1 );

   QPushButton* pb_models = us_pushbutton( tr( "List Available Models" ) );
   connect( pb_models, SIGNAL( clicked() ), SLOT( list_models() ) );
   main->addWidget( pb_models, row, 0 );

   QPushButton* pb_new = us_pushbutton( tr( "Create New Model" ) );
   main->addWidget( pb_new, row++, 1 );
   connect( pb_new,         SIGNAL( clicked()   ),
                            SLOT(   new_model() ) );

   QLabel* lb_description = us_label( tr( "Model Description:" ) );
   main->addWidget( lb_description, row, 0 );

   le_description = us_lineedit( "" );
   connect( le_description, SIGNAL( returnPressed () ),
                            SLOT  ( edit_description() ) );
   main->addWidget( le_description, row++, 1 );

   // Models List Box
   lw_models = new US_ListWidget;

   //connect( lw_models, SIGNAL( currentRowChanged( int  ) ),
   //                    SLOT  ( change_model     ( int  ) ) );

   //connect( lw_models, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
   connect( lw_models, SIGNAL( itemClicked      ( QListWidgetItem* ) ),
                       SLOT  ( select_model     ( QListWidgetItem* ) ) );

   main->addWidget( lw_models, row, 0, 5, 2 );
   row += 5;

   QPushButton* pb_components = us_pushbutton( tr( "Manage Components" ) );
   connect( pb_components, SIGNAL( clicked() ), SLOT( manage_components() ) );
   main->addWidget( pb_components, row, 0 );

   QPushButton* pb_associations = us_pushbutton( tr( "Manage Associations" ) );
   connect( pb_associations, SIGNAL( clicked() ), SLOT( associations() ) );
   main->addWidget( pb_associations, row++, 1 );

   QLabel* lb_wavelength = us_label( tr( "Wavelength:" ) );
   main->addWidget( lb_wavelength, row, 0 );

   le_wavelength = us_lineedit( );
   main->addWidget( le_wavelength, row++, 1 );

   QLabel* lb_optics = us_label( tr( "Optical System:" ) );
   main->addWidget( lb_optics, row, 0 );

   cb_optics = us_comboBox();
   cb_optics->addItem( tr( "Absorbance"   ), ABSORBANCE   );
   cb_optics->addItem( tr( "Interference" ), INTERFERENCE );
   cb_optics->addItem( tr( "Fluorescence" ), FLUORESCENCE );
   main->addWidget( cb_optics, row++, 1 );

   QLabel* lb_guid = us_label( tr( "Global Identifier:" ) );
   main->addWidget( lb_guid, row, 0 );

   le_guid = us_lineedit( "" );
   le_guid->setPalette( gray );
   main->addWidget( le_guid, row++, 1 );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }
 
   pb_save   = us_pushbutton( tr( "Save / Update Model" ) );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_model() ) );
   main->addWidget( pb_save,   row,   0 );

   pb_delete = us_pushbutton( tr( "Delete Selected Model" ) );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( delete_model() ) );
   main->addWidget( pb_delete, row++, 1 );

   pb_save  ->setEnabled( false );
   pb_delete->setEnabled( false );

   // Pushbuttons
   QBoxLayout* buttonbox = new QHBoxLayout;

   QPushButton* pb_help = us_pushbutton( tr( "Help") );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help()) );
   buttonbox->addWidget( pb_help );

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   buttonbox->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
   buttonbox->addWidget( pb_accept );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept_model()) );

   main->addLayout( buttonbox, row++, 0, 1, 4 );

   if ( model.description != "New Model" )
   {  // if re-loading a previous model, list that model
      ModelDesc desc;
      desc.description = model.description;
      desc.DB_id       = -1;
      desc.filename .clear();
      desc.modelGUID   = model.modelGUID;
      desc.editGUID    = model.editGUID;

      working_model = model;
      model_descriptions << desc;
      show_model_desc();
      recent_row    = 0;

      le_description->setText( model.description );
      le_wavelength ->setText( QString::number( model.wavelength, 'f', 1 ) );
      le_guid       ->setText( model.modelGUID );
      cb_optics     ->setCurrentIndex( model.optics );
      lw_models     ->setCurrentRow( recent_row );
   }

   check_db();
   adjustSize();
}

void US_ModelGui::new_model( void )
{
   if ( ! ignore_changes() ) return;

   US_Model m;  // Create a new model
   m.description    = le_description->text().trimmed();

   if ( m.description.isEmpty() )
   {
      m.description    = "New Model";
      le_description->setText( m.description );
   }

   ModelDesc desc;
   desc.description = m.description;
   desc.DB_id       = "-1";
   desc.filename .clear();
   desc.modelGUID.clear();
   desc.editGUID .clear();

   model         = m;
   working_model = m;
   model_saved   = false;

   model_descriptions << desc;
   show_model_desc();

   // Account for sorting
   int last = model_descriptions.size() - 1;

   for ( int i = 0; i < lw_models->count(); i++ )
   {
      if ( lw_models->item( i )->type() - QListWidgetItem::UserType == last )
      {
         lw_models->setCurrentRow( i );
         recent_row = i;
         break;
      }
   }

   le_guid->clear();
}

void US_ModelGui::show_model_desc( void )
{
   lw_models->clear();

   for ( int i = 0; i < model_descriptions.size(); i++ )
   {
      QString desc = model_descriptions[ i ].description;
      // Add a type value to the that is the position of the item
      // in the model_description list plus the Qt set minimum value
      // for custom types.
      new QListWidgetItem( desc, lw_models, i + QListWidgetItem::UserType );
   }

   lw_models->sortItems();
}

bool US_ModelGui::ignore_changes( void )
{
   if ( working_model == model ) return true;

   int response = QMessageBox::question( this,
      tr( "Model Changed" ),
      tr( "The model has changed.  Do you want to ignore the changes?" ),
      QMessageBox::Cancel, QMessageBox::Yes );

   if ( response == QMessageBox::Cancel ) return false;

   working_model = model;
   return true;
}

void US_ModelGui::edit_description( void )
{
   int row = lw_models->currentRow();
   if ( row < 0  ||  model_descriptions.size() == 0 ) return;

   QString desc = le_description->text().trimmed();
   if ( desc.isEmpty() ) return;

   if ( desc != "New Model" )
   {
      pb_save  ->setEnabled( true );
      pb_delete->setEnabled( true );
   }

   if ( desc == lw_models->item( row )->text() ) return;

   // Find index into model_descriptions
   int index  = lw_models->item( row )->type() - QListWidgetItem::UserType;

   model_descriptions[ index ].description = desc;
   model.description = desc;
   show_model_desc();

   // Re-select row (it was sorted)
   for ( int i = 0; i < model_descriptions.size(); i++ )
   {
      if ( lw_models->item( i )->type() - QListWidgetItem::UserType == index )
      {
         lw_models->setCurrentRow( i );
         break;
      }
   }
}

void US_ModelGui::select_model( QListWidgetItem* item )
{
   if ( ! ignore_changes() ) 
   {
      // Reset to last row;
      item -> listWidget()->setCurrentRow( recent_row );
      return;
   }

   // Get the current index
   if ( model_descriptions.size() == 0 )
      return;

   int     index = item->listWidget()-> currentRow();
   QString mdesc = item->text();
   int     modlx = modelIndex( mdesc, model_descriptions );
qDebug() << "SelM: modlx md.desc md.GUID" << modlx
   << mdesc << model_descriptions[modlx].modelGUID;
   
   // For the case of the user clicking on "New Model"
   if ( model_descriptions[ modlx ].modelGUID.isEmpty() )
   {
      model.description = mdesc;
      model.modelGUID   = "";
      le_description->setText( model.description );
      le_guid       ->setText( model.modelGUID   );
qDebug() << "SelM:   NEW mx md.desc" << modlx << model.description;
      return;
   }

   QString        file;

   if ( rb_disk->isChecked() ) // Load from disk
   {
      file = model_descriptions[ modlx ].filename;
      if ( file.isEmpty() ) return;
      
      model.load( file );
   }
   else // Load from db
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      QString modelID = model_descriptions[ modlx ].DB_id;
      model.load( modelID, &db );
qDebug() << "SelM:  DB ID GUID" << modelID << model.modelGUID;
   
   }

   if ( mdesc != "New Model" )
   {
      pb_save  ->setEnabled( true );
      pb_delete->setEnabled( true );
   }

   model_descriptions[ modlx ].editGUID = model.editGUID;
 
   working_model = model;

   recent_row    = index;

   // Populate 
   le_description    ->setText( model.description );
   le_wavelength     ->setText( QString::number( model.wavelength, 'f', 1 ) );

   le_guid           ->setText( model.modelGUID );

   cb_optics         ->setCurrentIndex( model.optics );
}

void US_ModelGui::delete_model( void )
{
   int row = lw_models->currentRow();
   if ( row < 0 ) return;

   ModelDesc md = model_descriptions.takeAt( row );
   show_model_desc();

   // Delete from DB or disk

   if ( rb_disk->isChecked() )
   {
      QString path;
      if ( ! US_Model::model_path( path ) ) return;

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = get_filename( path, le_guid->text() );
      if ( newFile ) return;

      QFile::remove( fn );
   }
   else // Remove from DB
   {
      QStringList q;
      q << "delete_model" << md.DB_id;

      if ( status_query( q ) )
         QMessageBox::information( this,
            tr( "Model Deleted" ),
            tr( "The model has been deleted from the database." ) );
   }
}

bool US_ModelGui::status_query( const QStringList& q )
{
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      connect_error( db.lastError() );
      return false;
   }

   db.statusQuery( q );

   return database_ok( db );
}

void US_ModelGui::check_db( void )
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
      investigator = US_Settings::us_inv_ID();

      if ( investigator > 0 )
         le_investigator->setText( QString::number( investigator ) + ": "
               + US_Settings::us_inv_name() );
   }
}

void US_ModelGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, 
      tr( "Connection Problem" ),
      tr( "Could not connect to databasee\n" ) + error );
}

bool US_ModelGui::database_ok( US_DB2& db )
{
   if ( db.lastErrno() == US_DB2::OK ) return true;

   QMessageBox::information( this, 
      tr( "Database Error" ),
      tr( "The following error was returned:\n" ) + db.lastError() );

   return false;
}

void US_ModelGui::get_person( void )
{
   US_Investigator* dialog = new US_Investigator( true, investigator );
   
   connect( dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( update_person        ( int, const QString&, const QString& ) ) );

   dialog->exec();
}

void US_ModelGui::update_person( int            ID, 
                                 const QString& lname, 
                                 const QString& fname )
{
   investigator = ID;

   if ( ID < 0 ) 
      le_investigator->setText( "" );
   else
      le_investigator->setText( 
            QString::number( ID ) + ": " + lname + ", " + fname );
}

void US_ModelGui::manage_components( void )
{
   int index = lw_models->currentRow();

   if ( index < 0 )
   {
      QMessageBox::information( this,
         tr( "Model not selected" ),
         tr( "Please select a model first.\n" 
             "If necessary, create a new model." ) );
      return;
   }

   bool dbAccess = rb_db->isChecked();
   working_model = model;

   US_Properties* dialog = 
      new US_Properties( working_model, investigator, dbAccess );
   
   connect( dialog, SIGNAL( done() ), SLOT( update_sim() ) );
   dialog->exec();
}

void US_ModelGui::update_sim( void )
{
   model       = working_model;
   model_saved = false;
}

void US_ModelGui::associations( void )
{
   int index = lw_models->currentRow();

   if ( index < 0 )
   {
      QMessageBox::information( this,
         tr( "Model not selected" ),
         tr( "Please select a model first.\n" 
             "If necessary, create a new model." ) );
      return;
   }

   if ( model.components.size() < 2 )
   {
      QMessageBox::information( this,
         tr( "Model Incomplete" ),
         tr( "There must be at least two analytes in the selected\n" 
             "model to specify associations." ) );
      return;
   }

   working_model = model;

   US_AssociationsGui* dialog = new US_AssociationsGui( working_model );
   connect( dialog, SIGNAL( done() ), SLOT( update_assoc() ) );
   dialog->exec();
}

void US_ModelGui::update_assoc( void )
{
   model       = working_model;
   model_saved = false;
}

void US_ModelGui::accept_model( void )
{
qDebug() << "MG:accept:  saved? desc" << model_saved << model.description;
   if ( ! ignore_changes() )
      return;

   if ( ! model_saved )
   {
      int response = QMessageBox::question( this,
         tr( "Model Changed" ),
         tr( "The model may have changed and not been saved.\n"
             "Do you want to accept the possibly unsaved model?" ),
         QMessageBox::Cancel, QMessageBox::Yes );

      if ( response == QMessageBox::Cancel ) return;
   }

   if ( recent_row >= 0 )
      emit valueChanged( model );

   close();
}

QString US_ModelGui::get_filename( const QString& path, const QString& guid )
{
   QDir f( path );
   QStringList filter( "M???????.xml" );
   QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

   for ( int i = 0; i < f_names.size(); i++ )
   {
      QFile m_file( path + "/" + f_names[ i ] );

      if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      QXmlStreamReader xml( &m_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "model" )
            {
               QXmlStreamAttributes a = xml.attributes();

               if ( a.value( "modelGUID" ).toString() == guid )
               {
                  newFile = false;
                  return path + "/" + f_names[ i ];
               }
            }
         }
      }

      m_file.close();
   }

   int number = ( f_names.size() > 0 ) ? f_names.last().mid( 1, 7 ).toInt() : 0;
   newFile    = true;

   return path + "/M" + QString().sprintf( "%07i", number + 1 ) + ".xml";
}

void US_ModelGui::save_model( void )
{
   if ( ! verify_model() ) return;

   model.wavelength      = le_wavelength->text().toDouble();

   if ( rb_disk->isChecked() )
   {
      QString path;
      if ( ! US_Model::model_path( path ) ) return;

      // If guid is null, generate a new one.
      if ( le_guid->text().size() != 36 )
         le_guid->setText( US_Util::new_guid() );

      model.modelGUID = le_guid->text();

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = get_filename( path, le_guid->text() );
      QFile   file( fn );

      if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
          qDebug() << "Cannot open file for writing: " << fn;
          return;
      }

      model.write( fn );

      QString save_type = newFile ? "saved" : "updated";

      QMessageBox::information( this,
         tr( "Model Saved" ),
         tr( "The model has been %1 in the disk." ).arg( save_type ) );

      working_model = model;
   }
   else // Save/update in DB
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      if ( model.write( &db ) == US_DB2::OK )
      {
         QMessageBox::information( this,
            tr( "Model Written" ),
            tr( "The model has been %1 in the database." ).arg( model.message ) );
         
         working_model = model;
         le_guid->setText( model.modelGUID   );
      }
      else
         QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The model could not be saved:\n" ) + model.message );
   }
   model_saved = true;
}

bool US_ModelGui::verify_model( void )
{
   if ( model.components.size() == 0 )
   {
      QMessageBox::information( this,
            tr( "Model Error" ),
            tr( "The model has no components." ) );
      return false;
   }

   if ( model.description.size()    == 0  ||  
        model.description.trimmed() == "New Model" )
   {
      QMessageBox::information( this,
            tr( "Model Name Error" ),
            tr( "The model does not have an acceptable name.\n"
                "Enter a non-blank, non-\"New Model\" name in the\n"
                "Model Description field. You may then save the model." ) );
      return false;
   }

   return true;
}

void US_ModelGui::list_models( void )
{
   if ( ! ignore_changes() ) return;

   QString path;
   if ( ! US_Model::model_path( path ) ) return;

   model_descriptions.clear();
   le_description->clear();

   if ( rb_disk->isChecked() )
   {
      QDir f( path );
      QStringList filter( "M*.xml" );
      QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes a;

      for ( int i = 0; i < f_names.size(); i++ )
      {
         QFile m_file( path + "/" + f_names[ i ] );

         if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

         QXmlStreamReader xml( &m_file );

         while ( ! xml.atEnd() )
         {
            xml.readNext();

            if ( xml.isStartElement() )
            {
               if ( xml.name() == "model" )
               {
                  ModelDesc md;
                  a                = xml.attributes();
                  md.description = a.value( "description" ).toString();
                  md.modelGUID   = a.value( "modelGUID"        ).toString();
                  md.editGUID    = a.value( "editGUID"    ).toString();
                  md.filename    = path + "/" + f_names[ i ];
                  md.DB_id       = -1;
                  model_descriptions << md;
                  break;
               }
            }
         }

         m_file.close();
      }
   }
   else  // DB Access
   {
      if ( investigator < 0 )
      {
         QMessageBox::information( this,
            tr( "Investigator not set" ),
            tr( "Please select an investigator first." ) );
         return;
      }

      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      QStringList q;
      q << "get_model_desc" << QString::number( investigator );

      db.query( q );

      while ( db.next() )
      {
         ModelDesc md;

         md.DB_id       = db.value( 0 ).toString();
         md.modelGUID   = db.value( 1 ).toString();
         md.description = db.value( 2 ).toString();
         md.editGUID    = db.value( 3 ).toString();
         md.filename.clear();

         model_descriptions << md;
      }
   }
 
   int olhgt  = lw_models->height();  // old list dimensions before any change
   int olwid  = lw_models->width();

   show_model_desc();                 // re-do list of model descriptions
 
   int nlines = model_descriptions.size();

   if ( nlines == 0 )
      lw_models->addItem( "No models found." );

   else
   {  // resize to show reasonable portion of list
      nlines     = nlines > 13 ? 13 : nlines;        // show at most 13 lines,
      nlines     = nlines <  5 ?  5 : nlines;        //   and no less than 5
      QFontMetrics fm = lw_models->fontMetrics();    // list font metrics
      int height = nlines * fm.lineSpacing();        // list height needed
      int width  = fm.maxWidth() * 20;               // list width needed
      height     = height > olhgt ? height : olhgt;  // max of old,new height
      width      = width  > olwid ? width  : olwid;  // max of old,new width
      height     = this->height() + height - olhgt;  // bump by difference in
      width      = this->width()  + width  - olwid;  //   old,new list size for
      resize( width, height );                       //   new total dialog size
   }
}

int US_ModelGui::modelIndex( QString mdesc, QList< ModelDesc > mds )
{
   for ( int i = 0; i < mds.size(); i++ )
   {
      if ( mdesc == mds[ i ].description )
         return i;
   }

   return -1;
}

