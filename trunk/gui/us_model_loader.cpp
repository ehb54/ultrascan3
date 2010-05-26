//! \file us_model_loader.cpp

#include "us_model_loader.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"

// main constructor with flags for multiple-selection and local-data
US_ModelLoader::US_ModelLoader( bool multisel, bool local )
  :US_WidgetsDialog( 0, 0 ), multi( multisel ), ondisk( local )
{
   setWindowTitle( tr( "Load Distribution Model" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize( 320, 300 );

   model_descriptions.clear();

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout( );
   int row           = 0;

   QGridLayout* db_layout   =
      us_radiobutton( tr( "Use Database" ),   rb_db,  !ondisk );
   QGridLayout* disk_layout =
      us_radiobutton( tr( "Use Local Disk" ), rb_disk, ondisk );
   top->addLayout( db_layout,   row,   0 );
   top->addLayout( disk_layout, row++, 1 );
   connect( rb_disk, SIGNAL( toggled( bool ) ),
            this,    SLOT( select_disk( bool ) ) );

   pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   top->addWidget( pb_investigator, row,   0 );
   connect( pb_investigator, SIGNAL( clicked() ),
            this,            SLOT( investigator() ) );
   le_investigator = us_lineedit();
   le_investigator->setReadOnly( false );
   top->addWidget( le_investigator, row++, 1 );

   pb_filtmodels   = us_pushbutton( tr( "Search" ) );
   top->addWidget( pb_filtmodels,   row,   0 );
   connect( pb_filtmodels, SIGNAL( clicked() ),
            this,          SLOT( list_models() ) );
   le_mfilter      = us_lineedit();
   le_mfilter->setReadOnly( false );
   top->addWidget( le_mfilter,      row++, 1 );

   main->addLayout( top );

   // List widget to show model choices
   lw_models       = new US_ListWidget;

   if ( multi )
      lw_models->setSelectionMode( QAbstractItemView::ExtendedSelection );

   main->addWidget( lw_models );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help,   SIGNAL( clicked() ), this, SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), this, SLOT( cancelled() ) );
   buttons->addWidget( pb_cancel );

   QPushButton* pb_accept = us_pushbutton( tr( "Accept" ) );
   connect( pb_accept, SIGNAL( clicked() ), this, SLOT( accepted() ) );
   buttons->addWidget( pb_accept );

   main->addLayout( buttons );

   // Default investigator to current user, based on home directory
   le_investigator->setText( QDir( QDir::homePath() ).dirName() );

   // Trigger models list from disk or db source
   select_disk( ondisk );
}

// Alternate constructor with multi-select and defaulted local-disk flags
US_ModelLoader::US_ModelLoader( bool multisel ):US_WidgetsDialog( 0, 0 )
{
   US_ModelLoader( multisel, true );
}

// Alternate constructor with both multi-select and local-disk flags defaulted
US_ModelLoader::US_ModelLoader( void ):US_WidgetsDialog( 0, 0 )
{
   US_ModelLoader( false, true );
}

// Public method to return count of models selected
int US_ModelLoader::models_count()
{
   return modelsCount;
}

// Public method to return model data by index
US_FemGlobal_New::ModelSystem US_ModelLoader::load_model( int index )
{
   QTemporaryFile temporary;
   QString        file;

   if ( rb_disk->isChecked() )
   {  // local-disk:  file is actual local file
      file      = model_descriptions[ index ].filename;
   }

   else
   {  // DB:  build temporary file by DB query
      US_Passwd   pw;
      QString     masterPW = pw.getPasswd();
      US_DB2      db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "DB Connection Problem" ),
               tr( "There was an error connecting to the database:\n" )
               + db.lastError() );
         return model;
      }
      QStringList query;

      query << "get_model_info" << model_descriptions[ index ].DB_id;
      db.query( query );

      if ( db.lastErrno() != US_DB2::OK )
         return model;

      db.next();

      QByteArray contents = db.value( 2 ).toString().toAscii();

      temporary.open();
      temporary.write( contents );
      temporary.close();

      file      = temporary.fileName();
   }

   // Common load of model information from file
   model.read_from_disk( file );

   return model;
}

// Public method to return description string for model by index
QString US_ModelLoader::description( int index )
{
   QString sep    = ";";

   if ( model_descriptions[ index ].description.contains( sep ) )
      sep            = "^";

   QString cdesc  = sep + model_descriptions[ index ].description
                  + sep + model_descriptions[ index ].filename
                  + sep + model_descriptions[ index ].guid
                  + sep + model_descriptions[ index ].DB_id;
   return cdesc;
}

// Slot to respond to change in disk/db radio button select
void US_ModelLoader::select_disk( bool checked )
{
qDebug() << "DISK: ckd isChkd" << checked << rb_disk->isChecked();
   // Disable investigator field if from disk; Enable if from db
   pb_investigator->setEnabled( !rb_disk->isChecked() );
   le_investigator->setEnabled( !rb_disk->isChecked() );

   lw_models->clear();

   if ( !checked )
   {  // Disk unchecked (DB checked):  fill models list from DB
      US_Passwd   pw;
      QString     masterPW = pw.getPasswd();
      US_DB2      db( masterPW );

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "DB Connection Problem" ),
               tr( "There was an error connecting to the database:\n" )
               + db.lastError() );
         return;
      }

      US_Investigator::US_InvestigatorData data;
      QStringList query;
      QString     invtext = le_investigator->text();
      QString     newinve = invtext;
      bool        haveInv = false;

      query << "get_people" << invtext;
      db.query( query );

      if ( db.numRows() < 1 )
      {  // Investigator text yields nothing:  retry with blank field
qDebug() << "INITIAL DB numRows invtext" << db.numRows() << invtext;
         query.clear();
         query << "get_people" << "";
         db.query( query );
      }

qDebug() << "DB connected?" << db.isConnected();
qDebug() << "DB numRows" << db.numRows();
qDebug() << " Investigator text" << invtext;

      while ( db.next() )
      {  // Loop through investigators looking for match
         data.invID     = db.value( 0 ).toInt();
         data.lastName  = db.value( 1 ).toString();
         data.firstName = db.value( 2 ).toString();
qDebug() << "id lName fName" << data.invID << data.lastName << data.firstName;
         if ( db.numRows() < 2  ||
              data.lastName.contains( invtext, Qt::CaseInsensitive )  ||
              data.firstName.contains( invtext, Qt::CaseInsensitive ) )
         {  // Single investigator or a match to last,first name
            haveInv     = true;
            newinve     = QString::number( data.invID )
               + ": " + data.lastName+ ", " + data.firstName;
            break;
         }
      }

      if ( haveInv )
         le_investigator->setText( newinve );
   }

   // Show the list of available models
   list_models();
}

// Investigator text changed:  act like DB radio button just checked
void US_ModelLoader::investigator()
{
qDebug() << "INVESTIGATOR";
   select_disk( false );
}

// List model choices (disk/db and possibly filtered by search text)
void US_ModelLoader::list_models()
{
qDebug() << "LIST_MODELS";
   QString mfilt = le_mfilter->text();
   QRegExp mpart = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
qDebug() << "MFILT:" << mfilt;
   ondisk     = rb_disk->isChecked();
   model_descriptions.clear();         // clear model descriptions

   if ( ondisk )
   {  // Models from local disk files
      QDir    dir;
      QString path = US_Settings::dataDir() + "/models";
      if ( !dir.exists( path ) )
      {
         dir.mkpath( path );
      }
      dir          = QDir( path );

      // Examine all "M*xml" files in models directory
      QStringList filter( "M*.xml" );
      QStringList f_names = dir.entryList( filter, QDir::Files, QDir::Name );

      QXmlStreamAttributes attr;

      for ( int ii = 0; ii < f_names.size(); ii++ )
      {
         QString fname( path + "/" + f_names[ ii ] );
         QFile   m_file( fname );

         if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            continue;

         QXmlStreamReader xml( &m_file );

         while ( ! xml.atEnd() )
         {  // Search XML elements until we find "model"
            xml.readNext();

            if ( xml.isStartElement() && xml.name() == "model" )
            {  // Pick up model attributes for description
               ModelDesc desc;
               attr             = xml.attributes();
               desc.description = attr.value( "description" ).toString();
               if ( mfilt.isEmpty()  ||  desc.description.contains( mpart ) )
               {  // unfiltered or filter matches description
                  desc.guid        = attr.value( "guid" ).toString();
                  desc.filename    = fname;
                  desc.DB_id       = "-1";

                  model_descriptions << desc;  // add to models list
               }
               break;
            }
         }

         m_file.close();
      }
   }

   else
   {  // Model list from DB
      US_Passwd   pw;
      US_DB2      db( pw.getPasswd() );
      QStringList query;
      QString     invID = le_investigator->text().section( ":", 0, 0 );

      query << "get_model_desc" << invID;
      db.query( query );

      while ( db.next() )
      {
         ModelDesc desc;
         desc.DB_id        = db.value( 0 ).toString();
         desc.guid         = db.value( 1 ).toString();
         desc.description  = db.value( 2 ).toString();
         desc.filename.clear();

         if ( mfilt.isEmpty()  ||  desc.description.contains( mpart ) )
         {  // unfiltered or filter matches
            model_descriptions << desc;   // add to models list
         }
      }
   }

   lw_models->disconnect( SIGNAL( currentRowChanged( int ) ) );
   lw_models->clear();

   if ( model_descriptions.size() > 0 )
   {
      for ( int ii = 0; ii < model_descriptions.size(); ii++ )
      {  // propogate list widget with descriptions
         lw_models->addItem( model_descriptions[ ii ].description );
      }

      // Sort descriptions in ascending alphabetical order
      lw_models->sortItems();
   }

   else
   {
      lw_models->addItem( "No models found." );
   }
}

// Cancel button:  no models returned
void US_ModelLoader::cancelled()
{
   modelsCount = 0;
qDebug() << "MLOAD cancel";
   reject();
   close();
}

// Accept button:  set up to return model information
void US_ModelLoader::accepted()
{
qDebug() << "MLOAD accept";
   QList< ModelDesc >        allmods = model_descriptions;
   QList< QListWidgetItem* > selmods = lw_models->selectedItems();
   modelsCount = selmods.size();

   if ( modelsCount > 0 )
   {  // loop through selections
      model_descriptions.clear();

      for ( int ii = 0; ii < modelsCount; ii++ )
      {  // get row of selection then row in original descriptions list
         int     row   = lw_models->row( selmods[ ii ] );
         QString mdesc = selmods[ ii ]->text();
qDebug() << "ii row item" << ii << row << mdesc;
         for ( int jj = 0; jj < allmods.size(); jj++ )
         {  // search for matching description and save its row
            if ( mdesc.compare( allmods[ jj ].description ) == 0 )
            {
               row        = jj;
               break;
            }
         }
qDebug() << "   row item" << row << mdesc;
         // repopulate descriptions with only selected row(s)
         model_descriptions.append( allmods.at( row ) );
      }
   }

   else
   {
      QMessageBox::information( this,
            tr( "No Model Selected" ),
            tr( "You have not selected a model\nSelect+Accept or Cancel" ) );
      return;
   }

   accept();        // signal that selection was accepted
   close();
}

