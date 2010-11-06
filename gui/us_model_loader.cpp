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
   all_model_descrips.clear();

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
   connect( rb_disk, SIGNAL( toggled(     bool ) ),
            this,    SLOT(   select_disk( bool ) ) );

   pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   top->addWidget( pb_investigator, row,   0 );
   connect( pb_investigator, SIGNAL( clicked()       ),
            this,            SLOT(   get_person()    ) );
   le_investigator = us_lineedit();
   le_investigator->setReadOnly( false );
   top->addWidget( le_investigator, row++, 1 );
   connect( le_investigator, SIGNAL( returnPressed() ),
            this,            SLOT(   investigator()  ) );

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
   {
      QStringList DB     = US_Settings::defaultDB();
      int         idPers = US_Settings::us_inv_ID();

      if ( DB.size() < 5  ||  idPers < 1 )  // no default DB:  use user name
         dinvtext = QDir( QDir::homePath() ).dirName();

      else                                  // investigator from default DB
         dinvtext = QString::number( idPers ) + ": "
                    + US_Settings::us_inv_name();
   }

   le_investigator->setText( dinvtext );

   db_id1     = -2;
   db_id2     = -2;

   // Trigger models list from disk or db source
   if ( !dsearch.contains( "=edit" ) )
      select_disk( ondisk );           // only list if not "=edit" filtered
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
                  + sep + model_descriptions[ index ].DB_id
                  + sep + model_descriptions[ index ].editguid;
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

// Public method to set Edit GUID for possible list filtering
void US_ModelLoader::set_edit_guid( QString guid )
{
   edguid   = guid;           // edit GUID

   select_disk( ondisk );     // trigger list delayed at constructor
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

// Investigator button clicked:  get investigator from dialog
void US_ModelLoader::get_person()
{
   dinvtext      = le_investigator->text();
   int idPers    = dinvtext.section( ":", 0, 0 ).toInt();
   US_Investigator* dialog = new US_Investigator( true, idPers );

   connect( dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT(   update_person(         int, const QString&, const QString& ) ) );

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_ModelLoader::update_person( int ID, const QString& lname,
      const QString& fname)
{
   dinvtext      = QString::number( ID ) + ": " + lname + ", " + fname;
   le_investigator->setText( dinvtext );
}
      
// List model choices (disk/db and possibly filtered by search text)
void US_ModelLoader::list_models()
{
   QString mfilt = le_mfilter->text();
   bool listall  = mfilt.isEmpty();
   bool listdesc = !listall;
   bool listedit = !listall;
   QRegExp mpart = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
   ondisk     = rb_disk->isChecked();
   model_descriptions.clear();         // clear model descriptions

   if ( !listall )
   {  // filter is not empty
      listedit = mfilt.contains( "=edit" );  // edit filtered?
      listdesc = !listedit;                  // description filtered?

      if ( listedit  &&  edguid.isEmpty() )
      {  // disallow edit filter if no edit GUID has been given
         QMessageBox::information( this,
               tr( "Edit GUID Problem" ),
               tr( "No Edit GUID was given.\n\"=edit\" reset to blank." ) );
         listall  = true;
         listdesc = false;
         listedit = false;
         le_mfilter->setText( "" );
      }
   }
//qDebug() << "listall" << listall;
//qDebug() << "  listdesc" << listdesc;
//qDebug() << "  listedit" << listedit;
         
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
//qDebug() << "   md size" << all_model_descrips.size();
//qDebug() << "   fn size" << f_names.size();

      if ( f_names.size() != all_model_descrips.size() )
      { // only re-fetch all-models list if we don't yet have it
         QXmlStreamAttributes attr;

         all_model_descrips.clear();

         for ( int ii = 0; ii < f_names.size(); ii++ )
         {
            QString fname( path + "/" + f_names[ ii ] );
//qDebug() << "fname" << f_names[ii];
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
                  desc.guid        = attr.value( "modelGUID"   ).toString();
                  desc.filename    = fname;
                  desc.DB_id       = "-1";
                  desc.editguid    = attr.value( "editGUID"    ).toString();
//*DEBUG
//if (!listall) {
//qDebug() << " ddesc" << desc.description;
//qDebug() << "   mpart" << mpart.pattern();
//qDebug() << "   degid" << desc.editguid;
//qDebug() << "   edgid" << edguid;
//}
//*DEBUG

                  all_model_descrips << desc;   // add to full models list
                  break;
               }
            }

            m_file.close();
         }
      }
      db_id1            = -2;
      db_id2            = -2;
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

      int countMD = all_model_descrips.size();
      int kid1    = -3;
      int kid2    = -3;
//qDebug() << " md count" << countMD;
//      query << "count_models" << invID;
//      int countDB = db.statusQuery( query );
//qDebug() << " db count" << countDB;

      if ( countMD > 0 )
      {
         kid1 = all_model_descrips[ 0           ].DB_id.toInt();
         kid2 = all_model_descrips[ countMD - 1 ].DB_id.toInt();
      }
//qDebug() << "  kid1 kid2" << kid1 << kid2;
//qDebug() << "  db_id1 db_id2" << db_id1 << db_id2;

      if ( countMD == 0  ||  kid1 != db_id1  ||  kid2 != db_id2 )
      { // only re-fetch all-models list if we don't yet have it
         db_id1            = kid1;
         db_id2            = kid2;
//qDebug() << "        db_id1 db_id2" << db_id1 << db_id2;
         all_model_descrips.clear();
         query.clear();

         query << "get_model_desc" << invID;
         db.query( query );
//qDebug() << " NumRows" << db.numRows();

         while ( db.next() )
         {
            ModelDesc desc;
            desc.DB_id        = db.value( 0 ).toString();
            desc.guid         = db.value( 1 ).toString();
            desc.description  = db.value( 2 ).toString();
            desc.editguid     = db.value( 3 ).toString();
            desc.filename.clear();

            all_model_descrips << desc;   // add to full models list
         }
      }
   }

   // possibly pare down models list based on search field

   if ( listall )
   {
      for ( int jj = 0; jj < all_model_descrips.size(); jj++ )
      {
         model_descriptions << all_model_descrips[ jj ];
      }
   }

   else if ( listedit )
   {
      for ( int jj = 0; jj < all_model_descrips.size(); jj++ )
      {
         if ( all_model_descrips[ jj ].editguid.contains(    edguid ) )
         {  // edit filter matches
            model_descriptions << all_model_descrips[ jj ];
//ModelDesc desc = all_model_descrips[jj];
//qDebug() << " ddesc" << desc.description;
//qDebug() << "   degid" << desc.editguid;
//qDebug() << "   edgid" << edguid;
         }
      }
   }

   else if ( listdesc )
   {
      for ( int jj = 0; jj < all_model_descrips.size(); jj++ )
      {
         if ( all_model_descrips[ jj ].description.contains( mpart  ) )
         {  // description filter matches
            model_descriptions << all_model_descrips[ jj ];
//ModelDesc desc = all_model_descrips[jj];
//qDebug() << " ddesc" << desc.description << jj;
//qDebug() << "   mpart" << mpart.pattern();
         }
      }
   }

   lw_models->disconnect( SIGNAL( currentRowChanged( int ) ) );
   lw_models->clear();

   if ( model_descriptions.size() > 0 )
   {
      for ( int ii = 0; ii < model_descriptions.size(); ii++ )
      {  // propagate list widget with descriptions
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
QString US_ModelLoader::typeText( US_Model::AnalysisType mtype,
   int nassoc, US_Model::GlobalType gtype, bool monteCarlo )
{
   struct typemap
   {
      US_Model::AnalysisType typeval;
      QString                typedesc;
   };

   const typemap tmap[] =
   {
      { US_Model::MANUAL,    QObject::tr( "Manual"  ) },
      { US_Model::TWODSA,    QObject::tr( "2DSA"    ) },
      { US_Model::TWODSA_MW, QObject::tr( "2DSA-MW" ) },
      { US_Model::GA,        QObject::tr( "GA"      ) },
      { US_Model::GA_MW,     QObject::tr( "GA-MW"   ) },
      { US_Model::COFS,      QObject::tr( "COFS"    ) },
      { US_Model::FE,        QObject::tr( "FE"      ) },
      { US_Model::ONEDSA,    QObject::tr( "1DSA"    ) }
   };

   const int ntmap = sizeof( tmap ) / sizeof( tmap[ 0 ] );

   QString tdesco  = QString( tr( "Unknown" ) );

   for ( int jj = 0; jj < ntmap; jj++ )
   {  // look for model type match

      if ( mtype == tmap[ jj ].typeval )
      {  // we have a match:  build match type description
         tdesco       = tmap[ jj ].typedesc;  // base type

         if ( nassoc > 0 )
            tdesco       = tdesco + "-RA";    // Reversible Associations subtype

         if ( gtype == US_Model::MENISCUS )
            tdesco       = tdesco + "-MN";    // Meniscus subtype

         else if ( gtype == US_Model::GLOBAL )
            tdesco       = tdesco + "-GL";    // Global subtype

         else if ( gtype == US_Model::SUPERGLOBAL )
            tdesco       = tdesco + "-SG";    // SuperGlobal subtype

         if ( monteCarlo )
            tdesco       = tdesco + "-MC";    // MonteCarlo subtype

         break;                               // break to return description
      }
   }

   return tdesco;
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
   US_Model::AnalysisType mtype;

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
   {  // ID is filename
      lblid    = tr( "\n  Model File Name:       " );
   }

   else
   {  // ID is DB id
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

      mdx      = modelIndex( mdesc, model_descriptions );  // find index
//qDebug() << "  row" << row;
//qDebug() << "   mdx" << mdx;

      load_model( model, mdx );                            // load model

      mtype    = model.analysis;                           // model info
      ncomp    = model.components.size();
      nassoc   = model.associations.size();
      tdesc    = typeText( mtype, nassoc, model.global, model.monteCarlo );
      runid    = mdesc.section( ".", 0, 0 );
      mdlid    = frDisk ?
         model_descriptions[ mdx ].filename :              // ID is filename
         model_descriptions[ mdx ].DB_id;                  // ID is DB id
      mdlid    = mdlid.length() < 50 ? mdlid :
         "*/" + mdlid.section( "/", -3, -1 );              // short filename

      dtext    = tr( "Model Information:" )
         + tr( "\n  Description:           " ) + mdesc
         + tr( "\n  Implied RunID:         " ) + runid
         + tr( "\n  Type:                  " ) + tdesc
         + "  (" + QString::number( (int)mtype ) + ")"
         + tr( "\n  Model Global ID:       " ) + model.modelGUID
         + tr( "\n  Description Global ID: " ) + model_descriptions[ mdx ].guid
         + tr( "\n  Edit Global ID:        " ) + model.editGUID
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
      QString aegid;
      QString eguid;

      row      = lw_models->row( selmods[ 0 ] );  // 1st model values
      mdesc    = selmods[ 0 ]->text();
      mdx      = modelIndex( mdesc, model_descriptions ); // 1st model index

      load_model( model, mdx );                           // load model

      runid    = mdesc.section( ".", 0, 0 );              // model info
      mtype    = model.analysis;
      nassoc   = model.associations.size();
      tdesc    = typeText( mtype, nassoc, model.global, model.monteCarlo );
      aruni    = runid;                           // potential common values
      atype    = tdesc;
      aegid    = model.editGUID;

      // make a pass to see if runID and type are common

      for ( int jj = 1; jj < modelsCount; jj++ )
      {
         row      = lw_models->row( selmods[ jj ] );
         mdesc    = selmods[ jj ]->text();
         mdx      = modelIndex( mdesc, model_descriptions ); // model index

         load_model( model, mdx );                           // load model

         runid    = mdesc.section( ".", 0, 0 );
         tdesc    = typeText( model.analysis, model.associations.size(),
                              model.global, model.monteCarlo );
         eguid    = model.editGUID;

         if ( !aruni.isEmpty()  &&  aruni.compare( runid ) != 0 )
            aruni    = "";   // turn off common if mismatch

         if ( !atype.isEmpty()  &&  atype.compare( tdesc ) != 0 )
            atype    = "";   // turn off common if mismatch

         if ( !aegid.isEmpty()  &&  aegid.compare( eguid ) != 0 )
            aegid    = "";   // turn off common if mismatch

         if ( aruni.isEmpty()  &&  atype.isEmpty()  &&  aegid.isEmpty() )
            break;           // none common:  break
      }

      // Report on common RunID and/or Type
      dtext    = tr( "Common Model Information   ( " )
         + QString::number( modelsCount )
         + tr( " models ):" );
      
      if ( !aruni.isEmpty() )
         dtext    = dtext + tr( "\n  Run ID:                " ) + aruni;
      
      if ( !atype.isEmpty() )
         dtext    = dtext + tr( "\n  Type:                  " ) + atype;

      if ( !aegid.isEmpty() )
         dtext    = dtext + tr( "\n  Related Edit GUID:     " ) + aegid;

      // Now loop to report on each model

      for ( int jj = 0; jj < modelsCount; jj++ )
      {
         row      = lw_models->row( selmods[ jj ] );          // row selected
         mdesc    = selmods[ jj ]->text();
         mdx      = modelIndex( mdesc, model_descriptions );  // model index

         load_model( model, mdx );                            // load model

         mtype    = model.analysis;                           // model info
         ncomp    = model.components.size();
         nassoc   = model.associations.size();
         tdesc    = typeText( mtype, nassoc, model.global, model.monteCarlo );
         runid    = mdesc.section( ".", 0, 0 );
         mdlid    = frDisk ?
            model_descriptions[ mdx ].filename :              // ID is filename
            model_descriptions[ mdx ].DB_id;                  // ID is DB id
         mdlid    = mdlid.length() < 50 ?  mdlid :
            "*/" + mdlid.section( "/", -3, -1 );              // short filename

         dtext    = dtext + tr( "\n\nModel Information: (" )
            + QString::number( ( jj + 1 ) ) + "):"
            + tr( "\n  Description:           " ) + mdesc
            + tr( "\n  Implied RunID:         " ) + runid
            + tr( "\n  Type:                  " ) + tdesc
            + "  (" + QString::number( (int)mtype ) + ")"
            + tr( "\n  Model Global ID:       " ) + model.modelGUID
            + tr( "\n  Description Global ID: " ) + model_descriptions[ mdx ].guid
            + tr( "\n  Edit Global ID:        " ) + model.editGUID
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

