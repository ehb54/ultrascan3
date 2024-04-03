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

US_ModelGui::US_ModelGui( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   setWindowTitle   ( "UltraScan Model Editor" );
   setPalette       ( US_GuiSettings::frameColor() );
   setAttribute     ( Qt::WA_DeleteOnClose );
   setWindowModality( Qt::WindowModal );
   
   model_descriptions.clear();

   working_model = model;
   model_saved   = true;
  
   recent_row    = -1;
   investigator  = -1;

   QGridLayout* main = new QGridLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Start widgets
   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );

   if ( US_Settings::us_inv_level() < 3 )
      pb_investigator->setEnabled( false );
   
   le_investigator   = us_lineedit( "", 0, true );
   

   dkdb_cntrls       = new US_Disk_DB_Controls;

   QLabel* lb_mlfilt = us_label( tr( "List Filter:" ) );
   le_mlfilt         = us_lineedit( "" );
   le_nlines         = us_lineedit( "", 0, true );

   QPushButton* pb_models = us_pushbutton( tr( "List Available Models" ) );
   QPushButton* pb_new    = us_pushbutton( tr( "Create New Model" ) );

   QLabel* lb_description = us_label( tr( "Model Description:" ) );
   le_description         = us_lineedit( "" );

   // Models List Box
   lw_models              = us_listwidget();

   QPushButton* pb_components   = us_pushbutton( tr( "Manage Components" ) );
   QPushButton* pb_associations = us_pushbutton( tr( "Manage Associations" ) );

   QLabel* lb_wavelength  = us_label( tr( "Wavelength:" ) );
   le_wavelength          = us_lineedit( );

   QLabel* lb_optics      = us_label( tr( "Optical System:" ) );

   cb_optics = us_comboBox();
   cb_optics->addItem( tr( "Absorbance"   ), ABSORBANCE   );
   cb_optics->addItem( tr( "Interference" ), INTERFERENCE );
   cb_optics->addItem( tr( "Fluorescence" ), FLUORESCENCE );

   QLabel* lb_guid        = us_label( tr( "Global Identifier:" ) );
   le_guid                = us_lineedit( "", 0, true );

   if ( US_Settings::us_debug() == 0 )
   {
      lb_guid->setVisible( false );
      le_guid->setVisible( false );
   }
 
   pb_save   = us_pushbutton( tr( "Save / Update Model" ) );
   pb_delete = us_pushbutton( tr( "Delete Selected Model" ) );
   pb_save  ->setEnabled( false );
   pb_delete->setEnabled( false );


   // Pushbuttons
   QBoxLayout* buttonbox   = new QHBoxLayout;
   QPushButton* pb_help    = us_pushbutton( tr( "Help") );
   QPushButton* pb_close   = us_pushbutton( tr( "Cancel") );
   //QPushButton* pb_accept  = us_pushbutton( tr( "Accept") );
                pb_accept  = us_pushbutton( tr( "Accept") );
   buttonbox->addWidget( pb_help );
   buttonbox->addWidget( pb_close );
   buttonbox->addWidget( pb_accept );

   int row = 0;
   main->addWidget( pb_investigator, row,   0, 1, 1 );
   main->addWidget( le_investigator, row++, 1, 1, 3 );
   main->addLayout( dkdb_cntrls,     row++, 0, 1, 4 );
   main->addWidget( lb_mlfilt,       row,   0, 1, 1 );
   main->addWidget( le_mlfilt,       row,   1, 1, 2 );
   main->addWidget( le_nlines,       row++, 3, 1, 1 );
   main->addWidget( pb_models,       row,   0, 1, 2 );
   main->addWidget( pb_new,          row++, 2, 1, 2 );
   main->addWidget( lb_description,  row,   0, 1, 1 );
   main->addWidget( le_description,  row++, 1, 1, 3 );
   main->addWidget( lw_models,       row,   0, 5, 4 ); row += 5;
   main->addWidget( pb_components,   row,   0, 1, 2 );
   main->addWidget( pb_associations, row++, 2, 1, 2 );
   main->addWidget( lb_wavelength,   row,   0, 1, 1 );
   main->addWidget( le_wavelength,   row,   1, 1, 1 );
   main->addWidget( lb_optics,       row,   2, 1, 1 );
   main->addWidget( cb_optics,       row++, 3, 1, 1 );
   main->addWidget( lb_guid,         row,   0, 1, 1 );
   main->addWidget( le_guid,         row++, 1, 1, 3 );
   main->addWidget( pb_save,         row,   0, 1, 2 );
   main->addWidget( pb_delete,       row++, 2, 1, 2 );
   main->addLayout( buttonbox,       row++, 0, 1, 4 );

   connect( pb_investigator, SIGNAL( clicked()           ),
                             SLOT(   get_person()        ) );
   connect( dkdb_cntrls,     SIGNAL( changed    ( bool ) ),
                             SLOT(   upd_disk_db( bool ) ) );
   connect( le_mlfilt,       SIGNAL( editingFinished()   ),
                             SLOT(   filter_changed()    ) );
   connect( pb_models,       SIGNAL( clicked()           ),
                             SLOT(   list_models()       ) );
   connect( pb_new,          SIGNAL( clicked()           ),
                             SLOT(   new_model()         ) );
   connect( le_description,  SIGNAL( editingFinished ()  ),
                             SLOT  ( edit_description()  ) );
   connect( pb_components,   SIGNAL( clicked()           ),
                             SLOT(   manage_components() ) );
   connect( pb_associations, SIGNAL( clicked()           ),
                             SLOT(   associations()      ) );
   connect( pb_save,         SIGNAL( clicked()           ),
                             SLOT(   save_model()        ) );
   connect( pb_delete,       SIGNAL( clicked()           ),
                             SLOT(   delete_model()      ) );
   connect( pb_help,         SIGNAL( clicked()           ),
                             SLOT(   help()              ) );
   connect( pb_close,        SIGNAL( clicked()           ),
                             SLOT(   close()             ) );
   connect( pb_accept,       SIGNAL( clicked()           ),
                             SLOT(   accept_model()      ) );
   connect( lw_models, SIGNAL( itemClicked ( QListWidgetItem* ) ),
                       SLOT  ( select_model( QListWidgetItem* ) ) );

   if ( !model.description.isEmpty()  &&  model.description != "New Model" )
   {  // if re-loading a previous model, list that model
      ModelDesc desc;
      desc.description = model.description;
      desc.DB_id       = -1;
      desc.filename .clear();
      desc.modelGUID   = model.modelGUID;
      desc.editGUID    = model.editGUID;

      working_model    = model;
      model_descriptions << desc;
      show_model_desc();
      recent_row       = 0;

      le_description->setText( model.description );
      le_wavelength ->setText( QString::number( model.wavelength, 'f', 1 ) );
      le_guid       ->setText( model.modelGUID );
      cb_optics     ->setCurrentIndex( model.optics );
      lw_models     ->setCurrentRow( recent_row );
   }

   check_db();
   resize( 500, 600 );
}

void US_ModelGui::new_model( void )
{
   if ( ! ignore_changes() ) return;

   US_Model m;  // Create a new model

   le_description->setText( m.description );

   ModelDesc desc;
   desc.description = m.description;
   desc.DB_id       = "-1";
   desc.filename .clear();
   desc.modelGUID.clear();
   desc.editGUID .clear();

   le_guid->clear();

   model         = m;
   working_model = m;
   model_saved   = false;

   model_descriptions << desc;
   show_model_desc();

   // Account for sorting
   int last = model_descriptions.size() - 1;

   for ( int ii = 0; ii < lw_models->count(); ii++ )
   {
      if ( lw_models->item( ii )->type() - QListWidgetItem::UserType == last )
      {
         lw_models->setCurrentRow( ii );
         recent_row = ii;
         break;
      }
   }
}

void US_ModelGui::show_model_desc( void )
{
   lw_models->clear();
   QString mfilt    = le_mlfilt->text();
   bool    listdesc = !mfilt.isEmpty();
qDebug() << "ShMDsc: mfilt listdesc" << mfilt << listdesc;
   QRegExp mpart    = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   for ( int ii = 0; ii < model_descriptions.size(); ii++ )
   {
      QString desc = model_descriptions[ ii ].description;

      // If filtering, exclude non-matching items
      if ( listdesc  &&  !desc.contains( mpart ) )
         continue;

      // Add a type value to the lw_item that is the position of the item
      // in the model_description list plus the Qt minimum value
      // for custom types.
      new QListWidgetItem( desc, lw_models, ii + QListWidgetItem::UserType );
   }

   lw_models->sortItems();
   QApplication::restoreOverrideCursor();
}

bool US_ModelGui::ignore_changes( void )
{
   if ( working_model == model ) return true;

   //check if leaving GMP produced model after potential changes in Components..
   qDebug() << "In ignore changes(): modelID_global -- " << modelID_global;
   if ( is_modelIDs_from_autoflow( modelID_global ) )
     {
       return true;
     }

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
   for ( int ii = 0; ii < model_descriptions.size(); ii++ )
   {
      if ( lw_models->item( ii )->type() - QListWidgetItem::UserType == index )
      {
         lw_models->setCurrentRow( ii );
         break;
      }
   }
}

void US_ModelGui::select_model( QListWidgetItem* item )
{
qDebug() << "SelMdl: IN";
   if ( ! ignore_changes() ) 
   {
qDebug() << "SelMdl:  ignore_changes FALSE";
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
qDebug() << "SelMdl:  index modlx" << index << modlx << "mdesc" << mdesc;
   
   // For the case of the user clicking on "New Model"
   if ( model_descriptions[ modlx ].modelGUID.isEmpty() )
   {
      model.description = mdesc;
      model.modelGUID   = "";
      le_description->setText( model.description );
      le_guid       ->setText( model.modelGUID   );
      return;
   }

   if ( dkdb_cntrls->db() )
   {  // Load from database
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( db.lastErrno() != US_DB2::OK )
      {
         connect_error( db.lastError() );
         return;
      }

      QString modelID = model_descriptions[ modlx ].DB_id;
      modelID_global = modelID;
      model.load( modelID, &db );
   }

   else
   {  // Load from local disk
      QString filename  = model_descriptions[ modlx ].filename;
      
      if ( filename.isEmpty() )
         return;
      else
         model.load( filename );
   }

   if ( mdesc != "New Model" )
   {
      pb_save  ->setEnabled( true );
      pb_delete->setEnabled( true );
      pb_accept->setEnabled( true );
   }

   model_descriptions[ modlx ].editGUID = model.editGUID;
 
   working_model = model;

   recent_row    = index;
   
   // Populate 
   le_description->setText( model.description );
   le_wavelength ->setText( QString::number( model.wavelength, 'f', 1 ) );

   le_guid       ->setText( model.modelGUID );

   cb_optics     ->setCurrentIndex( model.optics );

   qDebug() << "Selected ModelID: " << model_descriptions[ modlx ].DB_id;

   //Check if the model was generated via autoflow framework
   if ( is_modelIDs_from_autoflow( model_descriptions[ modlx ].DB_id  ) )
     {
       qDebug() << "Select:: Model GMP/autolfow-generated!";

       QMessageBox::information( this, tr( "Selected Model GMP produced!" ),
       				 tr( "Selected Model:\n\n"
       				     "\"%1\"\n\n"
       				     "was generated within GMP framework!\n\n"
				     "It can NOT be deleted or altered...")
       				 .arg( mdesc ) );

       pb_save  ->setEnabled( false );
       pb_delete->setEnabled( false );
       pb_accept->setEnabled( false );
     }
}

bool US_ModelGui::is_modelIDs_from_autoflow( QString mID )
{
  QStringList modelInfos_autoflow;
  QStringList modelIDs_autoflow;
  
  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  if ( db.lastErrno() != US_DB2::OK )
    {
      connect_error( db.lastError() );
      return false;
    }
  
  QStringList q;
  q << "get_modelIDs_for_autoflow";
  db.query( q );
  
  while ( db.next() )
    modelInfos_autoflow << db. value( 0 ).toString();

  return parse_models_desc_json ( modelInfos_autoflow, mID ); 
}

bool US_ModelGui::parse_models_desc_json( QStringList modelInfos_autoflow, QString mID )
{
  qDebug() << "Size of modelInfos_autoflow: " << modelInfos_autoflow.size();
  for ( int i=0; i<modelInfos_autoflow.size(); ++i )
    {
      QString modelDescJson = modelInfos_autoflow[ i ];
      if ( modelDescJson.isEmpty() )
	return false;

      QJsonDocument jsonDoc = QJsonDocument::fromJson( modelDescJson.toUtf8() );
      QJsonObject json_obj = jsonDoc.object();

      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);
	  
	  if ( key == "2DSA_IT" || key == "2DSA_MC" || key == "PCSA" ) 
	    {
	      //qDebug() << "ModelsDesc key, value: " << key << value;
	      
	      QJsonArray json_array = value.toArray();
	      for (int i=0; i < json_array.size(); ++i )
		{
		  foreach(const QString& array_key, json_array[i].toObject().keys())
		    {
		      //by modelID
		      if ( array_key == "modelID" )
			{
			  QString c_modelID = json_array[i].toObject().value(array_key).toString();
			  // qDebug() << "modelDescJson Map: -- model, property, value: "
			  // 	   << key
			  // 	   << array_key
			  // 	   << c_modelID;
			  
			  if ( c_modelID == mID )
			    {
			      qDebug() << "Model to affect [delete:update], ID=" << mID << " IS autoflow produced!!!";
			      return true;
			    }
			}
		    }
		}
	    }
	}
    }

  return false;
}


void US_ModelGui::delete_model( void )
{
   int     row   = lw_models->currentRow();
   if ( row < 0 ) return;

   QString mdesc = lw_models->item( row )->text();
   int     modlx = modelIndex( mdesc, model_descriptions );
   ModelDesc md  = model_descriptions.takeAt( modlx );
   
   // //Check if the model was generated via autoflow framework
   // qDebug() << "in delete_model(): modelID -- " << md.DB_id;
   // if ( is_modelIDs_from_autoflow( md.DB_id ) )
   //   {
   //     qDebug() << "Model can NOT be deleted, autolfow-generated!";

   //     QMessageBox::information( this, tr( "Selected Model Cannot be Deleted" ),
   // 				 tr( "The Model:\n\n"
   // 				     "\"%1\"\n\n"
   // 				     "can NOT be deleted since it was generated within GMP framework!" )
   // 				 .arg( mdesc ) );

   //     return;
   //   }
   // //END of checking for autoflow
   
   show_model_desc();  // must be AFTER check!!!
     
   // Delete from DB or disk
   if ( ! dkdb_cntrls->db() )
   {
      QString path;
      if ( ! US_Model::model_path( path ) ) return;

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = US_Model::get_filename( path, le_guid->text(), newFile );
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
   if ( dkdb_cntrls->db() )
   {
      QStringList DB = US_Settings::defaultDB();

      if ( DB.size() < 5 )
      {
         QMessageBox::warning( this,
            tr( "Attention" ),
            tr( "There is no default database set." ) );
      }
   }

   investigator = US_Settings::us_inv_ID();

   QString number = ( investigator > 0 ) 
      ? QString::number( investigator ) + ": "
      : "";
         
   le_investigator->setText( number + US_Settings::us_inv_name() );
}

void US_ModelGui::connect_error( const QString& error )
{
   QMessageBox::warning( this, 
      tr( "Connection Problem" ),
      tr( "Could not connect to database\n" ) + error );
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
   US_Investigator* dialog = new US_Investigator;
   dialog->exec();
   
   investigator = US_Settings::us_inv_ID();

   QString number = ( investigator >  0 ) 
      ? QString::number( investigator ) + ": "
      : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
}

void US_ModelGui::manage_components( void )
{
   int index = lw_models->currentRow();
qDebug() << "MngCmp: index" << index;

   if ( index < 0 )
   {
      QMessageBox::information( this,
         tr( "Model not selected" ),
         tr( "Please select a model first.\n" 
             "If necessary, create a new model." ) );
      return;
   }


   int dbdisk  = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                   : US_Disk_DB_Controls::Disk;
   working_model = model;

   US_Properties* dialog = new US_Properties( working_model, dbdisk );
   
   connect( dialog, SIGNAL( done  ( void ) ), SLOT( update_sim    ( void ) ) );
   connect( dialog, SIGNAL( use_db( bool ) ), SLOT( source_changed( bool ) ) );

   connect( this, SIGNAL( disable_components_gui( ) ), dialog, SLOT( disable_gui( ) ) );
   
   //Check if the model was generated via autoflow framework
   
   if ( is_modelIDs_from_autoflow( modelID_global ) )
     {
       qDebug() << "Manage Components: GMP model!";
       emit disable_components_gui( );
     }

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
   
   //Check if the model was generated via autoflow framework
   connect( this, SIGNAL( disable_components_gui( ) ), dialog, SLOT( disable_gui( ) ) );
   if ( is_modelIDs_from_autoflow( modelID_global ) )
     {
       qDebug() << "Manage Associationss: GMP model!";
       emit disable_components_gui( );
     }
   
   dialog->exec();
}

void US_ModelGui::update_assoc( void )
{
   model       = working_model;
   model_saved = false;
}

void US_ModelGui::accept_model( void )
{
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

void US_ModelGui::save_model( void )
{
  //Check is model ganerated with GMP
  int     row   = lw_models->currentRow();
  if ( row < 0 ) return;

  // QString mdesc = lw_models->item( row )->text();
  // int     modlx = modelIndex( mdesc, model_descriptions );
  // ModelDesc md  = model_descriptions.takeAt( modlx );
  // if ( is_modelIDs_from_autoflow( md.DB_id ) )
  //   {
  //     qDebug() << "Model can NOT be save/updated, autolfow-generated!";

  //     QMessageBox::information( this, tr( "Selected Model Cannot be Saved/Updated" ),
  // 				tr( "The Model:\n\n"
  // 				    "\"%1\"\n\n"
  // 				    "can NOT be saved/updated since it was generated within GMP framework!" )
  // 				 .arg( mdesc ) );
  //     return;
  //   }
  
   if ( ! verify_model() ) return;

   model.wavelength = le_wavelength->text().toDouble();

   if ( ! dkdb_cntrls->db() )
   {
      QString path;
      if ( ! US_Model::model_path( path ) ) return;

      // If guid is null, generate a new one.
      if ( le_guid->text().size() != 36 )
         le_guid->setText( US_Util::new_guid() );

      model.modelGUID = le_guid->text();
qDebug() << "MdlSv: path" << path << "guid" << model.modelGUID;

      // If guid matches one we already have, use that filename
      // otherwise create a new filename.
      QString fn = US_Model::get_filename( path, le_guid->text(), newFile );
      QFile   file( fn );
qDebug() << "MdlSv:  fn" << fn;

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
      {
         QMessageBox::information( this,
            tr( "Database Error" ),
            tr( "The model could not be saved:\n" ) + model.message );
         return;
      }
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

int US_ModelGui::modelIndex( QString mdesc, QList< ModelDesc > mds )
{
   for ( int ii = 0; ii < mds.size(); ii++ )
   {
      if ( mdesc == mds[ ii ].description )
         return ii;
   }

   return -1;
}

void US_ModelGui::source_changed( bool db )
{
qDebug() << "SrcChg: db" << db;
   if ( db ) 
      dkdb_cntrls->set_db();
   else
      dkdb_cntrls->set_disk();

   model_descriptions.clear();

   list_models();

   qApp->processEvents();
}

// Slot to handle change in list filter
void US_ModelGui::filter_changed()
{
qDebug() << "FltChg: mfilt" << le_mlfilt->text();
   list_models();
}

// Slot to handle change in disk/db selection
void US_ModelGui::upd_disk_db( bool db )
{
qDebug() << "UpdDDb: db" << db;
   model_descriptions.clear();

   list_models();

   qApp->processEvents();
}


void US_ModelGui::list_models( void )
{
qDebug() << "LsMdl: IN";
   if ( ! ignore_changes() ) return;

   QString path;
   if ( ! US_Model::model_path( path ) ) return;

qDebug() << "LsMdl: path" << path;
   le_description->clear();

   if ( model_descriptions.count() == 0 )
   {  // No model descriptions, so get them all
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      if ( ! dkdb_cntrls->db() )
      {
         QDir f( path );
         QStringList filter( "M*.xml" );
         QStringList f_names = f.entryList( filter, QDir::Files, QDir::Name );

         QXmlStreamAttributes a;

         for ( int ii = 0; ii < f_names.size(); ii++ )
         {
            QFile m_file( path + "/" + f_names[ ii ] );

            if ( ! m_file.open( QIODevice::ReadOnly | QIODevice::Text) )
               continue;

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
                     md.modelGUID   = a.value( "modelGUID"   ).toString();
                     md.editGUID    = a.value( "editGUID"    ).toString();
                     md.filename    = path + "/" + f_names[ ii ];
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
            md.editGUID    = db.value( 5 ).toString();
            md.filename.clear();

            model_descriptions << md;
         }
      }
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
   }
 
   qApp->processEvents();
   int olhgt  = lw_models->height();  // old list dimensions before any change
   int olwid  = lw_models->width();
qDebug() << "LsMdl: old h,w" << olhgt << olwid;

   show_model_desc();                 // re-do list of model descriptions
 
   int nmodls = model_descriptions.size();
   int nlines = lw_models->count();
qDebug() << "LsMdl: nmodls" << nmodls << "nlines" << nlines;
   le_nlines->setText( nlines == 1 ? tr( "1 list item" )
                                   : tr( "%1 list items" ).arg( nlines ) );
   qApp->processEvents();

   if ( nlines == 0 )
      lw_models->addItem( "No models found." );

   else
   {  // Resize to show reasonable portion of list
      nlines     = qMin( nlines, 13 );              // show at most 13 lines,
      nlines     = qMin( nlines, 5 );               //   and no less than 5

      QFontMetrics fm = lw_models->fontMetrics(); 
      int height = nlines * fm.lineSpacing();       // list height needed
      int width  = fm.maxWidth() * 20;              // list width needed
      
      height     = qMax( height, olhgt );           // max of old,new height
      width      = qMax( width,  olwid );           // max of old,new width
      
      height     = this->height() + height - olhgt; // bump by difference in
      width      = this->width()  + width  - olwid; //   old,new list size
      
      resize( width, height );                      //   new total dialog size
   }
qDebug() << "LsMdl: END";
}

