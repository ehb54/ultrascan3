//! \file us_model_editor.cpp

#include "us_model_editor_new2.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_properties.h"
#include "us_investigator.h"
#include "us_buffer_gui.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_associations_gui.h"
#include <uuid/uuid.h>

US_ModelEditorNew::US_ModelEditorNew( 
      US_FemGlobal_New::ModelSystem& current_model )
   : US_Widgets(), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Editor" );
   setPalette       ( US_GuiSettings::frameColor() );
   setWindowModality( Qt::WindowModal );
   
   model_descriptions.clear();

   // Very light gray
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

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
   
   QGridLayout* db_layout = us_radiobutton( tr( "Use Database" ), rb_db );
   main->addLayout( db_layout, row, 0 );

   QGridLayout* disk_layout = us_radiobutton( tr( "Use Local Disk" ), rb_disk, true );
   main->addLayout( disk_layout, row++, 1 );

   QPushButton* pb_models = us_pushbutton( tr( "List Available Models" ) );
   connect( pb_models, SIGNAL( clicked() ), SLOT( list_models() ) );
   main->addWidget( pb_models, row, 0 );

   QPushButton* pb_new = us_pushbutton( tr( "Create New Model" ) );
   connect( pb_new, SIGNAL( clicked() ), SLOT( new_model() ) );
   main->addWidget( pb_new, row++, 1 );

   QLabel* lb_description = us_label( tr( "Model Description:" ) );
   main->addWidget( lb_description, row, 0 );

   le_description = us_lineedit( "" );
   connect( le_description, SIGNAL( editingFinished () ),
                            SLOT  ( edit_description() ) );
   main->addWidget( le_description, row++, 1 );

   // Models List Box
   lw_models = new US_ListWidget;

   //connect( lw_models, SIGNAL( currentRowChanged( int  ) ),
   //                    SLOT  ( change_model     ( int  ) ) );

   connect( lw_models, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
                       SLOT  ( select_model     ( QListWidgetItem* ) ) );

   main->addWidget( lw_models, row, 0, 5, 2 );
   row += 5;

   QPushButton* pb_components = us_pushbutton( tr( "Manage Components" ) );
   connect( pb_components, SIGNAL( clicked() ), SLOT( manage_components() ) );
   main->addWidget( pb_components, row, 0 );

   QPushButton* pb_associations = us_pushbutton( tr( "Manage Associations" ) );
   connect( pb_associations, SIGNAL( clicked() ), SLOT( associations() ) );
   main->addWidget( pb_associations, row++, 1 );

   QPushButton* pb_buffer = us_pushbutton( tr( "Select Buffer" ) );
   connect( pb_buffer, SIGNAL( clicked() ), SLOT( get_buffer() ) );
   main->addWidget( pb_buffer, row, 0 );

   le_buffer = us_lineedit( );
   le_buffer->setPalette ( gray );
   le_buffer->setReadOnly( true );
   main->addWidget( le_buffer, row++, 1 );

   QLabel* lb_density = us_label( tr( "Density:" ) );
   main->addWidget( lb_density, row, 0 );

   le_density = us_lineedit( );
   le_density->setPalette ( gray );
   le_density->setReadOnly( true );
   main->addWidget( le_density, row++, 1 );

   QLabel* lb_viscosity = us_label( tr( "Viscosity:" ) );
   main->addWidget( lb_viscosity, row, 0 );

   le_viscosity = us_lineedit( );
   le_viscosity->setPalette ( gray );
   le_viscosity->setReadOnly( true );
   main->addWidget( le_viscosity, row++, 1 );

   QLabel* lb_compressibility = us_label( tr( "Compressibility:" ) );
   main->addWidget( lb_compressibility, row, 0 );

   le_compressibility = us_lineedit( );
   le_compressibility->setPalette ( gray );
   le_compressibility->setReadOnly( true );
   main->addWidget( le_compressibility, row++, 1 );

   QLabel* lb_temperature = us_label( tr( "Temperature:" ) );
   main->addWidget( lb_temperature, row, 0 );

   le_temperature = us_lineedit( "20.0" );
   main->addWidget( le_temperature, row++, 1 );

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
 
   QPushButton* pb_save = us_pushbutton( tr( "Save / Update Model" ) );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save_model() ) );
   main->addWidget( pb_save, row, 0 );

   QPushButton* pb_delete = us_pushbutton( tr( "Delete Selected Model" ) );
   connect( pb_delete, SIGNAL( clicked() ), SLOT( delete_model() ) );
   main->addWidget( pb_delete, row++, 1 );

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
}

void US_ModelEditorNew::new_model( void )
{
   ModelDesc desc;
   desc.description = "New Model";
   desc.DB_id       = "-1";
   desc.filename.clear();
   desc.guid    .clear();

   model_descriptions << desc;
   show_model_desc();
   lw_models->setCurrentRow( model_descriptions.size() - 1 );
}

void US_ModelEditorNew::show_model_desc( void )
{
   lw_models->disconnect( SIGNAL( currentRowChanged( int ) ) );
   lw_models->clear();

   for ( int i = 0; i < model_descriptions.size(); i++ )
      lw_models->addItem( model_descriptions[ i ].description );

   //connect( lw_modelss, SIGNAL( currentRowChanged( int  ) ),
   //                     SLOT  ( update           ( int  ) ) );
}

void US_ModelEditorNew::edit_description( void )
{
   int row = lw_models->currentRow();
   if ( row < 0 ) return;

   QString desc = le_description->text().trimmed();
   if ( desc.isEmpty() ) return;

   if ( desc == lw_models->item( row )->text() ) return;

   //if ( keep_standard() )  // Do we want to change from the standard values?
   //{
      // Restore the description
   //   le_description->setText( lw_components->item( row )->text() );
   //   return;
   //}

   model_descriptions[ row ].description = desc;
   model.description = desc;
   show_model_desc();

   model.guid.clear();
   le_guid->clear();
}

void US_ModelEditorNew::select_model( QListWidgetItem* item )
{
   // Get the current index
   int index = item -> listWidget()-> currentRow();
   
   QString        file;
   QTemporaryFile temporary;

   if ( rb_disk->isChecked() )
   {
      file = model_descriptions[ index ].filename;
   }
   else // Get from db
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      QStringList q;
      q << "get_model_info" << model_descriptions[ index ].DB_id;
      db.query( q );

      if ( ! database_ok( db ) ) return;
      db.next();

      QByteArray contents = db.value( 2 ).toString().toAscii();

      // Write the model file to a temporary file
      temporary.open();
      temporary.write( contents );
      temporary.close();

      file = temporary.fileName();
   }
   
   model.read_from_disk( file );
  
   // Populate 
   buffer.GUID = model.bufferGUID;

   le_description    ->setText( model.description );
   le_buffer         ->setText( model.bufferDesc  );
   
   le_density        ->setText( QString::number( model.density,        'f', 4));
   le_viscosity      ->setText( QString::number( model.viscosity,      'f', 4));
   le_compressibility->setText( QString::number( model.compressibility,'e', 3));
   le_temperature    ->setText( QString::number( model.temperature,    'f', 1));
   
   le_guid           ->setText( model.guid );

   cb_optics         ->setCurrentIndex( model.optics );
}

void US_ModelEditorNew::delete_model( void )
{
   int row = lw_models->currentRow();
   if ( row < 0 ) return;

   ModelDesc md = model_descriptions.takeAt( row );
   show_model_desc();

   // Delete from DB or disk

   if ( rb_disk->isChecked() )
   {
      QString path;
      if ( ! model_path( path ) ) return;

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = get_filename( path, le_guid->text() );
      if ( newFile ) return;

      QFile::remove( path + "/" + fn );
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

bool US_ModelEditorNew::status_query( const QStringList& q )
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

void US_ModelEditorNew::connect_error( const QString& error )
{
   QMessageBox::warning( this, 
      tr( "Connection Problem" ),
      tr( "Could not connect to databasee\n" ) + error );
}

bool US_ModelEditorNew::database_ok( US_DB2& db )
{
   if ( db.lastErrno() == US_DB2::OK ) return true;

   QMessageBox::information( this, 
      tr( "Database Error" ),
      tr( "The following error was returned:\n" ) + db.lastError() );

   return false;
}

void US_ModelEditorNew::get_person( void )
{
   US_Investigator* dialog = new US_Investigator( true );
   
   connect( dialog, 
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( update_person        ( int, const QString&, const QString& ) ) );

   dialog->exec();
}

void US_ModelEditorNew::update_person( int            ID, 
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

void US_ModelEditorNew::get_buffer( void )
{
   US_BufferGui* dialog = new US_BufferGui( investigator, true, buffer, 
         rb_disk->isChecked() );
   
   connect( dialog, SIGNAL( valueChanged ( US_Buffer ) ),
                    SLOT  ( update_buffer( US_Buffer ) ) );

   dialog->exec();
}

void US_ModelEditorNew::update_buffer( US_Buffer buf )
{
   buffer = buf;
   le_density        ->setText( QString::number( buf.density        , 'f', 4 ) );
   le_viscosity      ->setText( QString::number( buf.viscosity      , 'f', 4 ) );
   le_compressibility->setText( QString::number( buf.compressibility, 'e', 3 ) );
   le_buffer         ->setText( buf.description );

   model.density         = buf.density;
   model.viscosity       = buf.viscosity;
   model.compressibility = buf.compressibility;
   model.bufferGUID      = buf.GUID;
   model.bufferDesc      = buf.description;
}

void US_ModelEditorNew::manage_components( void )
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

   if ( le_density->text().isEmpty() )
   {
      QMessageBox::information( this,
         tr( "Buffer not selected" ),
         tr( "Please select a buffer first.\n" ) );
      return;
   }

   bool dbAccess = rb_db->isChecked();
   working_model = model;

   US_Properties* dialog = 
      new US_Properties( buffer, working_model, investigator, dbAccess );
   
   connect( dialog, SIGNAL( done() ), SLOT( update_sim() ) );
   dialog->exec();
}

void US_ModelEditorNew::update_sim( void )
{
   model = working_model;
}

void US_ModelEditorNew::associations( void )
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

void US_ModelEditorNew::update_assoc( void )
{
   model = working_model;
}

void US_ModelEditorNew::accept_model( void )
{
   emit valueChanged( model );
   close();
}

bool US_ModelEditorNew::model_path( QString& path )
{
   QDir dir;
   path = US_Settings::dataDir() + "/models";

   if ( ! dir.exists( path ) )
   {
      if ( ! dir.mkpath( path ) )
      {
         QMessageBox::critical( this,
               tr( "Bad Model Path" ),
               tr( "Could not create default directory for analytes\n" )
               + path );
         return false;
      }
   }

   return true;
}

QString US_ModelEditorNew::get_filename( const QString& path, const QString& guid )
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

               if ( a.value( "guid" ).toString() == guid )
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

void US_ModelEditorNew::save_model( void )
{
   if ( ! verify_model() ) return;

   if ( rb_disk->isChecked() )
   {
      QString path;
      if ( ! model_path( path ) ) return;

      // If guid is null, generate a new one.
      if ( le_guid->text().size() != 36 )
         le_guid->setText( US_Util::new_guid() );

      model.guid = le_guid->text();

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = get_filename( path, le_guid->text() );
      QFile   file( fn );

      if ( ! file.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
          qDebug() << "Cannot open file for writing: " << fn;
          return;
      }

      QTemporaryFile temporary;
      write_temp( temporary );

      temporary.open();

      file.write( temporary.readAll() );
      file.close();
      temporary.close();

      QString save_type = newFile ? "saved" : "updated";

      QMessageBox::information( this,
         tr( "Model Saved" ),
         tr( "The model has been %1 locally." ).arg( save_type ) );
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

      QString     modelID = "-1";
      QStringList q;

      if ( ! model.guid.isEmpty() )
      {
         q << "get_modelID" << model.guid;
         db.query( q );

         if ( db.lastErrno() == US_DB2::OK ) 
         {
            db.next();
            modelID = db.value( 0 ).toString();
         }
      }

      // Create the model xml file in a string
      QTemporaryFile temporary;
      write_temp( temporary );
      temporary.open();
      QByteArray contents = temporary.readAll();

      q.clear();

      if ( modelID == "-1" )  // New model
      {
         // Generate a guid if necessary
         // The guid may be valid from a disk read, but is not in the DB
         if ( ! model.guid.size() != 36 )
         {
            le_guid->setText( US_Util::new_guid() );
            model.guid = le_guid->text();
         }

         q << "new_model" << model.guid << model.description << contents;

         if ( status_query( q ) )
            QMessageBox::information( this,
               tr( "Model Saved" ),
               tr( "The model has been saved in the database." ) );
      }
      else
      {
         q << "update_model" << modelID << model.description << contents;

         if ( status_query( q ) )
            QMessageBox::information( this,
               tr( "Model Updated" ),
               tr( "The model has been updated in the database." ) );
      }
   }
}

void US_ModelEditorNew::write_temp( QTemporaryFile& file )
{
   file.open();// QIODevice::WriteOnly | QIODevice::Text );
   QXmlStreamWriter xml( &file );
   xml.setAutoFormatting( true );

   xml.writeStartDocument();
   xml.writeDTD         ( "<!DOCTYPE US_Model>" );
   xml.writeStartElement( "ModelData" );
   xml.writeAttribute   ( "version", "1.0" );

   xml.writeStartElement( "model" );
   xml.writeAttribute   ( "description",     model.description );
   xml.writeAttribute   ( "guid",            model.guid );
   xml.writeAttribute   ( "bufferGuid",      model.bufferGUID );
   xml.writeAttribute   ( "bufferDesc",      model.bufferDesc );
   xml.writeAttribute   ( "density",         QString::number( model.density ) );
   xml.writeAttribute   ( "viscosity",       QString::number( model.viscosity ) );
   xml.writeAttribute   ( "compressibility", QString::number( model.compressibility ) );
   xml.writeAttribute   ( "temperature",     QString::number( model.temperature ) );
   xml.writeAttribute   ( "coSedSolute",     QString::number( model.coSedSolute ) );
   xml.writeAttribute   ( "optics",          QString::number( model.optics ) );
   
   char uuid[ 37 ];
   uuid[ 36 ] = 0;

   // Write components
   for ( int i = 0; i < model.components.size(); i++ )
   {
      US_FemGlobal_New::SimulationComponent* sc = &model.components[ i ];
      xml.writeStartElement( "analyte" );

      if ( uuid_is_null( sc->analyteGUID ) )
         xml.writeAttribute( "guid",    "" );
      else
      {
         uuid_unparse( sc->analyteGUID, uuid );
         xml.writeAttribute( "guid",    QString( uuid                      ) );
      }

      xml.writeAttribute( "name",       sc->name                             );
      xml.writeAttribute( "vbar20",     QString::number( sc->vbar20        ) );
      xml.writeAttribute( "mw",         QString::number( sc->mw            ) );
      xml.writeAttribute( "s",          QString::number( sc->s             ) );
      xml.writeAttribute( "D",          QString::number( sc->D             ) );
      xml.writeAttribute( "f",          QString::number( sc->f             ) );
      xml.writeAttribute( "f_f0",       QString::number( sc->f_f0          ) );
      xml.writeAttribute( "wavelength", QString::number( sc->wavelength    ) );
      xml.writeAttribute( "extinction", QString::number( sc->extinction    ) );
      xml.writeAttribute( "axial",      QString::number( sc->axial_ratio   ) );
      xml.writeAttribute( "sigma",      QString::number( sc->sigma         ) );
      xml.writeAttribute( "delta",      QString::number( sc->delta         ) );
      xml.writeAttribute( "stoich",     QString::number( sc->stoichiometry ) );
      xml.writeAttribute( "shape",      QString::number( sc->shape         ) );
      xml.writeAttribute( "type",       QString::number( sc->analyte_type  ) );

      xml.writeAttribute( "molar",      QString::number( sc->molar_concentration  ) );
      xml.writeAttribute( "signal",     QString::number( sc->signal_concentration ) );

      for ( int j = 0; j < sc->c0.radius.size(); j++ )
      {
         xml.writeStartElement( "mfem_scan" );

         US_FemGlobal_New::MfemInitial* scan = &sc->c0;
         xml.writeAttribute( "radius", QString::number( scan->radius       [ j ] ) );
         xml.writeAttribute( "conc",   QString::number( scan->concentration[ j ] ) );
         xml.writeEndElement();  // mfem_scan
      }

      xml.writeEndElement(); // analyte (SimulationComponent)
   }

   // Write associations
   for ( int i = 0; i < model.associations.size(); i++ )
   {
      US_FemGlobal_New::Association* as = &model.associations[ i ];
      xml.writeStartElement( "association" );
      xml.writeAttribute( "k_eq", QString::number( as->k_eq ) );
      xml.writeAttribute( "k_off", QString::number( as->k_off ) );

      for ( int j = 0; j < as->reaction_components.size(); j++ )
      {
         xml.writeStartElement( "component" );

         QString index  = QString::number( as->reaction_components[ j ] );
         QString stoich = QString::number( as->stoichiometry      [ j ] );

         xml.writeAttribute( "index",         index  );
         xml.writeAttribute( "stoichiometry", stoich );
         xml.writeEndElement(); // component
      }

      xml.writeEndElement(); // association
   }

   xml.writeEndElement(); // model
   xml.writeEndElement(); // ModelData
   xml.writeEndDocument();
   file.close();
}

bool US_ModelEditorNew::verify_model( void )
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
            tr( "Model Error" ),
            tr( "The model does not have an acceptable name." ) );
      return false;
   }

   return true;
}

void US_ModelEditorNew::list_models( void )
{
   QString path;
   if ( ! model_path( path ) ) return;

   model_descriptions.clear();

   if ( rb_disk->isChecked() )
   {
      QDir f( path );
      QStringList filter( "M*.xml" );
      QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

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
                  ModelDesc desc;
                  attr             = xml.attributes();
                  desc.description = attr.value( "description" ).toString();
                  desc.guid        = attr.value( "guid"        ).toString();
                  desc.filename    = path + "/" + f_names[ i ];
                  desc.DB_id       = -1;
                  model_descriptions << desc;
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
         md.guid        = db.value( 1 ).toString();
         md.description = db.value( 2 ).toString();
         md.filename.clear();

         model_descriptions << md;
      }
   }

   show_model_desc();
  
   if ( model_descriptions.size() == 0 )
      lw_models->addItem( "No model files found." );
}

