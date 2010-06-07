//! \file us_model_loader.cpp

#include "us_model_loader.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_editor.h"

// main constructor with flags for multiple-selection and local-data
US_ModelLoader::US_ModelLoader( bool multisel, bool local, QString search,
      QString invtext )
  :US_WidgetsDialog( 0, 0 ), multi( multisel ), ondisk( local ),
   dsearch( search ), dinvtext( invtext )
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
   connect( le_investigator, SIGNAL( returnPressed() ),
            this,            SLOT( investigator() ) );

   pb_filtmodels   = us_pushbutton( tr( "Search" ) );
   top->addWidget( pb_filtmodels,   row,   0 );
   connect( pb_filtmodels, SIGNAL( clicked() ),
            this,          SLOT( list_models() ) );
   le_mfilter      = us_lineedit();
   le_mfilter->setReadOnly( false );
   dsearch         = dsearch.isEmpty() ? QString( "" ) : dsearch;
   le_mfilter->setText( dsearch );
   top->addWidget( le_mfilter,      row++, 1 );
   connect( le_mfilter,    SIGNAL( returnPressed() ),
            this,          SLOT( list_models() ) );

   main->addLayout( top );

   // List widget to show model choices
   lw_models       = new US_ListWidget;

   if ( multi )
      lw_models->setSelectionMode( QAbstractItemView::ExtendedSelection );

   lw_models->installEventFilter( this );
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

   // Default investigator; possibly to current user, based on home directory
   if ( dinvtext.compare( QString( "USER" ) ) == 0 )
      dinvtext = QDir( QDir::homePath() ).dirName();

   le_investigator->setText( dinvtext );

   // Trigger models list from disk or db source
   select_disk( ondisk );
}

// Public method to return count of models selected
int US_ModelLoader::models_count()
{
   return modelsCount;
}

// Public method to load model data by index
int US_ModelLoader::load_model( US_Model& model, int index )
{
   int rc = 0;

   model.components.clear();
   model.associations.clear();

   if ( rb_disk->isChecked() )
   {
      QString   filename = model_descriptions[ index ].filename;

//qDebug() << "Model load from file" << filename;
      rc   = model.load( filename );
//qDebug() << "Model load RETURN" << rc;
//qDebug() << "LD model desc" << model.description;
//qDebug() << "LD model components count" << model.components.size();
   }

   else
   {
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

      if ( ( rc = db.lastErrno() ) != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "DB Connection Problem" ),
               tr( "There was an error connecting to the database:\n" )
               + db.lastError() );
         return rc;
      }

      QString   modelID  = model_descriptions[ index ].DB_id;

//qDebug() << "Model load from modelID" << modelID;
      rc   = model.load( modelID, &db );
//qDebug() << "Model load RETURN" << rc;
   }

   return rc;
}

// Public method to return description string for model by index
QString US_ModelLoader::description( int index )
{
   QString sep    = ";";     // use semi-colon as separator

   if ( model_descriptions[ index ].description.contains( sep ) )
      sep            = "^";  // use carat if semi-colon already in use

   // create and return a composite description string
   QString cdesc  = sep + model_descriptions[ index ].description
                  + sep + model_descriptions[ index ].filename
                  + sep + model_descriptions[ index ].guid
                  + sep + model_descriptions[ index ].DB_id;
   return cdesc;
}

// Public method to return the current search string
QString US_ModelLoader::search_filter()
{
   return le_mfilter->text();
}

// Public method to return the current investigator string
QString US_ModelLoader::investigator_text()
{
   return le_investigator->text();
}

// Slot to respond to change in disk/db radio button select
void US_ModelLoader::select_disk( bool checked )
{
   // Disable investigator field if from disk; Enable if from db
   pb_investigator->setEnabled( !rb_disk->isChecked() );
   le_investigator->setEnabled( !rb_disk->isChecked() );

   lw_models->clear();

   if ( !checked )
   {  // Disk unchecked (DB checked):  fill models list from DB
      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );

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
         query.clear();
         query << "get_people" << "";
         db.query( query );
      }

      while ( db.next() )
      {  // Loop through investigators looking for match
         data.invID     = db.value( 0 ).toInt();
         data.lastName  = db.value( 1 ).toString();
         data.firstName = db.value( 2 ).toString();
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
   select_disk( false );
}

// List model choices (disk/db and possibly filtered by search text)
void US_ModelLoader::list_models()
{
   QString mfilt = le_mfilter->text();
   QRegExp mpart = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
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

      if ( db.lastErrno() != US_DB2::OK )
      {
         QMessageBox::information( this,
               tr( "DB Connection Problem" ),
               tr( "There was an error connecting to the database:\n" )
               + db.lastError() );
         return;
      }

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

   reject();
   close();
}

// Accept button:  set up to return model information
void US_ModelLoader::accepted()
{
   QList< ModelDesc >        allmods = model_descriptions;
   QList< QListWidgetItem* > selmods = lw_models->selectedItems();
   modelsCount = selmods.size();

   if ( modelsCount > 0 )
   {  // loop through selections
      model_descriptions.clear();

      for ( int ii = 0; ii < modelsCount; ii++ )
      {  // get row of selection then index in original descriptions list
         QString mdesc = selmods[ ii ]->text();
         int     mdx   = modelIndex( mdesc, allmods );

         // repopulate descriptions with only selected row(s)
         model_descriptions.append( allmods.at( mdx ) );
      }
   }

   else
   {
      QMessageBox::information( this,
            tr( "No Model Selected" ),
            tr( "You have not selected a model.\nSelect+Accept or Cancel" ) );
      return;
   }

   accept();        // signal that selection was accepted
   close();
}

// Filter events to catch right-mouse-button-click on list widget
bool US_ModelLoader::eventFilter( QObject* obj, QEvent* e )
{
   if ( obj == lw_models  &&
         e->type() == QEvent::ContextMenu )
   {
//qDebug() << "Right-mouse list select";
      QPoint mpos = ((QContextMenuEvent*)e)->pos();
//qDebug() << "  pos" << mpos;

      show_model_info( mpos );

      return false;
   }

   else
   {  // pass all other events to normal handler
      return US_WidgetsDialog::eventFilter( obj, e );
   }
}

// Get type string corresponding to the type int enum
QString US_ModelLoader::typeText( US_Model::ModelType mtype,
   int nassoc, int iters )
{
   struct typemap
   {
      US_Model::ModelType  typeval;
      QString              typedesc;
   };

   const typemap tmap[] =
   {
      { US_Model::MANUAL,    QObject::tr( "Manual"  ) },
      { US_Model::TWODSA,    QObject::tr( "2DSA"    ) },
      { US_Model::TWODSA_MW, QObject::tr( "2DSA-MW" ) },
      { US_Model::GA,        QObject::tr( "GA"      ) },
      { US_Model::GA_MW,     QObject::tr( "GA-MW"   ) },
      { US_Model::GA_RA,     QObject::tr( "GA-RA"   ) },
      { US_Model::COFS,      QObject::tr( "COFS"    ) },
      { US_Model::FE,        QObject::tr( "FE"      ) },
      { US_Model::GLOBAL,    QObject::tr( "GLOBAL"  ) },
      { US_Model::ONEDSA,    QObject::tr( "1DSA"    ) }
   };
   const int ntmap = sizeof( tmap ) / sizeof( tmap[ 0 ] );

   QString tdesco  = QString( tr( "Unknown" ) );

   for ( int jj = 0; jj < ntmap; jj++ )
   {
      if ( mtype == tmap[ jj ].typeval )
      {
         tdesco       = tmap[ jj ].typedesc;

         if ( nassoc > 0 )
            tdesco       = tdesco + "-RA";

         if ( iters > 1 )
            tdesco       = tdesco + "-MC";

         return tdesco;
      }
      
   }

   return QString( tr( "Unknown" ) );
}

// Get index in model description list of a model description
int US_ModelLoader::modelIndex( QString mdesc, QList< ModelDesc > mds )
{
   int mdx = 0;

   for ( int jj = 0; jj < mds.size(); jj++ )
   {  // search for matching description and save its index
      if ( mdesc.compare( mds[ jj ].description ) == 0 )
      {
         mdx        = jj;
         break;
      }
   }

   return mdx;
}

// Show selected-model(s) information in text dialog
void US_ModelLoader::show_model_info( QPoint pos )
{
   US_Model::ModelType mtype;

   QString mdesc;
   QString tdesc;
   QString cdesc;
   QString runid;
   QString dtext;
   QString lblid;
   QString mdlid;

   int     row    = 0;
   int     mdx    = 0;
   int     iters  = 1;
   int     ncomp  = 0;
   int     nassoc = 0;

   bool    frDisk = ( model_descriptions[ 0 ].filename.length() > 0 );

   if ( frDisk )
   {
      lblid    = tr( "\n  Model File Name:       " );
   }

   else
   {
      lblid    = tr( "\n  Database Model ID:     " );
   }

   // get the list of selected models
   QList< QListWidgetItem* > selmods = lw_models->selectedItems();
   modelsCount = selmods.size();

   if ( modelsCount < 2 )
   {  // 1 or no rows selected:  build information for single model

      if ( modelsCount == 1 )
      {  // info on selected model
         row      = lw_models->row( selmods[ 0 ] );
         mdesc    = selmods[ 0 ]->text();
      }

      else
      {  // info on model at right-click row
         row      = lw_models->row( lw_models->itemAt( pos ) );
         mdesc    = lw_models->itemAt( pos )->text();
      }
//qDebug() << "  row" << row;

      mdx      = modelIndex( mdesc, model_descriptions );

      load_model( model, mdx );

      mtype    = model.type;
      iters    = model.iterations;
      ncomp    = model.components.size();
      nassoc   = model.associations.size();
      tdesc    = typeText( mtype, nassoc, iters );
      runid    = mdesc.section( ".", 0, 0 );
      mdlid    = frDisk ?
         model_descriptions[ mdx ].filename :
         model_descriptions[ mdx ].DB_id;
      mdlid    = mdlid.length() < 50 ? mdlid :
         "*/" + mdlid.section( "/", -3, -1 );

      dtext    = tr( "Model Information:" )
         + tr( "\n  Description:           " ) + mdesc
         + tr( "\n  Run ID:                " ) + runid
         + tr( "\n  Type:                  " ) + tdesc
         + "  (" + QString::number( (int)mtype ) + ")"
         + tr( "\n  Global ID:             " ) + model.guid
         + tr( "\n  Global ID: (descr.)    " ) + model_descriptions[ mdx ].guid
         + lblid + mdlid
         + tr( "\n  Iterations:            " ) + QString::number( iters )
         + tr( "\n  Components Count:      " ) + QString::number( ncomp )
         + tr( "\n  Associations Count:    " ) + QString::number( nassoc )
         + "";
   }

   else
   {  // multiple rows selected:  build multiple-model information text
      QString aruni;
      QString atype;

      row      = lw_models->row( selmods[ 0 ] );  // 1st model values
      mdesc    = selmods[ 0 ]->text();
      mdx      = modelIndex( mdesc, model_descriptions );

      load_model( model, mdx );

      runid    = mdesc.section( ".", 0, 0 );
      mtype    = model.type;
      iters    = model.iterations;
      nassoc   = model.associations.size();
      tdesc    = typeText( mtype, nassoc, iters );
      aruni    = runid;                           // potential common values
      atype    = tdesc;

      // make a pass to see if runID and type are common

      for ( int jj = 1; jj < modelsCount; jj++ )
      {
         row      = lw_models->row( selmods[ jj ] );
         mdesc    = selmods[ jj ]->text();
         mdx      = modelIndex( mdesc, model_descriptions );

         load_model( model, mdx );

         runid    = mdesc.section( ".", 0, 0 );
         tdesc    = typeText( model.type, model.associations.size(),
               model.iterations );

         if ( !aruni.isEmpty()  &&  aruni.compare( runid ) != 0 )
            aruni    = "";   // turn off common if mismatch

         if ( !atype.isEmpty()  &&  atype.compare( tdesc ) != 0 )
            atype    = "";   // turn off common if mismatch

         if ( aruni.isEmpty()  &&  atype.isEmpty() )
            break;           // neither common:  break
      }

      // Report on common RunID and/or Type
      dtext    = tr( "Common Model Information   ( " )
         + QString::number( modelsCount )
         + tr( " models ):" );
      
      if ( !aruni.isEmpty() )
         dtext    = dtext + tr( "\n  Run ID:                " ) + aruni;
      
      if ( !atype.isEmpty() )
         dtext    = dtext + tr( "\n  Type:                  " ) + atype;

      // Now loop to report on each model

      for ( int jj = 0; jj < modelsCount; jj++ )
      {
         row      = lw_models->row( selmods[ jj ] );
         mdesc    = selmods[ jj ]->text();
         mdx      = modelIndex( mdesc, model_descriptions );

         load_model( model, mdx );

         mtype    = model.type;
         iters    = model.iterations;
         ncomp    = model.components.size();
         nassoc   = model.associations.size();
         tdesc    = typeText( mtype, nassoc, iters );
         runid    = mdesc.section( ".", 0, 0 );
         mdlid    = frDisk ?
            model_descriptions[ mdx ].filename :
            model_descriptions[ mdx ].DB_id;
         mdlid    = mdlid.length() < 50 ?  mdlid :
            "*/" + mdlid.section( "/", -3, -1 );

         dtext    = dtext + tr( "\n\nModel Information: (" )
            + QString::number( ( jj + 1 ) ) + "):"
            + tr( "\n  Description:           " ) + mdesc
            + tr( "\n  Run ID:                " ) + runid
            + tr( "\n  Type:                  " ) + tdesc
            + "  (" + QString::number( (int)mtype ) + ")"
            + tr( "\n  Global ID:             " ) + model.guid
            + tr( "\n  Global ID: (descr.)    " )
              + model_descriptions[ mdx ].guid
            + lblid + mdlid
            + tr( "\n  Iterations:            " ) + QString::number( iters )
            + tr( "\n  Components Count:      " ) + QString::number( ncomp )
            + tr( "\n  Associations Count:    " ) + QString::number( nassoc )
            + "";
      }
   }

   // open a dialog and display model information
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true, "", this );
   edit->setWindowTitle( tr( "Model Information" ) );
   edit->move( this->pos() + pos + QPoint( 100, 100 ) );
   edit->resize( 600, 400 );
   edit->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   edit->e->setText( dtext );
   edit->show();
}

