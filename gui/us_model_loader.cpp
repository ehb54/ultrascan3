//! \file us_model_loader.cpp

#include "us_model_loader.h"
#include "us_select_edits.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_editor.h"

// Main constructor for loading a single model
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
      US_Model& amodel, QString& adescr, const QString eGUID )
  :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( amodel ), odescr( adescr ), omodels( wmodels ), odescrs( mdescrs ),
   runIDs( wrunIDs )
{
   multi    = false;
   editGUID = eGUID;

   build_dialog();
}

// Alternate constructor for loading a single model (with runIDs)
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
      US_Model& amodel, QString& adescr, QStringList& arunIDs )
  :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( amodel ), odescr( adescr ), omodels( wmodels ), odescrs( mdescrs ),
   runIDs( arunIDs )
{
   multi    = false;
   editGUID = "";

   build_dialog();
}

// Alternate constructor that allows loading multiple models
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
   QList< US_Model >& amodels, QStringList& adescrs,
   QStringList& arunIDs )
   :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( model ), odescr( search ), omodels( amodels ), odescrs( adescrs ),
   runIDs( arunIDs )
{
   multi    = true;
   editGUID = "";

   build_dialog();
}

// Alternate constructor that allows loading multiple models (no runIDs list)
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
   QList< US_Model >& amodels, QStringList& adescrs )
   :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( model ), odescr( search ), omodels( amodels ), odescrs( adescrs ),
   runIDs( wrunIDs )
{
   multi    = true;
   editGUID = "";

   build_dialog();
}

// Main shared method to build the model loader dialog
void US_ModelLoader::build_dialog( void )
{
qDebug() << "ML:BD: runIDs empty" << runIDs.isEmpty();
   setWindowTitle( multi ? tr( "Load Distribution Model(s)" )
                         : tr( "Load Distribution Model" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize( 320, 300 );

   model_descriptions.clear();
   model_descrs_recs .clear();

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout( );
   singprev          = false;
   dbP               = NULL;

   int iload         = loadDB ? US_Disk_DB_Controls::DB
                              : US_Disk_DB_Controls::Disk;
   dkdb_cntrls       = new US_Disk_DB_Controls( iload );

   connect( dkdb_cntrls, SIGNAL( changed(     bool ) ),
            this,        SLOT(   select_diskdb()     ) );

   QPalette gray   = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked()       ),
            this,            SLOT(   get_person()    ) );

   int inv = US_Settings::us_inv_ID();
   QString number  = ( inv > 0 ) ? QString::number( inv ) + ": " : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
         0, true );

   pb_filtmodels   = us_pushbutton( tr( "Search" ) );
   connect( pb_filtmodels, SIGNAL( clicked() ),
            this,          SLOT( list_models() ) );

   QString edGUID  = editGUID;
   do_single       = false;
   can_edit        = !editGUID.isEmpty();
   can_run         = ( runIDs.size() > 0 );
   do_edit         = can_edit;
   do_run          = can_run;
   do_manual       = false;
   do_unasgn       = false;

   if ( ! dsearch.isEmpty() )
   {  // If an input search string is given, look for special flags
      do_single       = dsearch.contains( "=s" );
      do_manual       = dsearch.contains( "=m" );
      do_unasgn       = dsearch.contains( "=u" );

      if ( ( do_manual || do_unasgn )  &&  can_edit )
      {
         do_edit         = ( edGUID == "1" );
      }

      dsearch         = dsearch.replace( "=e",  "" ).simplified();
      dsearch         = dsearch.replace( "=s",  "" ).simplified();
      dsearch         = dsearch.replace( "=m",  "" ).simplified();
      dsearch         = dsearch.replace( "=u",  "" ).simplified();
   }
qDebug() << "Bld: single" << do_single << " manual" << do_manual
 << " edit" << do_edit << " unasgn" << do_unasgn << " dsearch" << dsearch;

   le_mfilter      = us_lineedit( dsearch, -1, false );
   connect( le_mfilter,    SIGNAL( returnPressed() ),
            this,          SLOT(   list_models()   ) );
   connect( le_mfilter,    SIGNAL( textChanged( const QString& ) ),
            this,          SLOT(   msearch(     const QString& ) ) );

   int row          = 0;
   top->addLayout( dkdb_cntrls,     row++, 0, 1, 2 );
   top->addWidget( pb_investigator, row,   0 );
   top->addWidget( le_investigator, row++, 1 );
   top->addWidget( pb_filtmodels,   row,   0 );
   top->addWidget( le_mfilter,      row++, 1 );

   main->addLayout( top );

   // List widget to show model choices
   lw_models       = new US_ListWidget;

   if ( multi )
      lw_models->setSelectionMode( QAbstractItemView::ExtendedSelection );

   lw_models->installEventFilter( this );
   main->addWidget( lw_models );

   // Advanced Model List Options
   QGridLayout* advtypes   = new QGridLayout;
   QLabel*      lb_advopts = us_banner( tr( "Advanced Model List Options" ) );
   lb_advopts->setMaximumHeight( le_mfilter->height() );

   QGridLayout* lo_single  = us_checkbox( tr( "Monte Carlo Singles" ),
                                          ck_single,  do_single );
   QGridLayout* lo_edit    = us_checkbox( tr( "Filter by Edit/Run" ),
                                          ck_edit,    do_edit || do_run );
   QGridLayout* lo_unasgn  = us_checkbox( tr( "Edit-Unassigned Only" ),
                                          ck_unasgn,  do_unasgn || do_manual );
   int arow   = 0;
   advtypes->addWidget( lb_advopts, arow++, 0, 1, 3 );
   advtypes->addLayout( lo_single,  arow,   0, 1, 1 );
   advtypes->addLayout( lo_edit,    arow,   1, 1, 1 );
   advtypes->addLayout( lo_unasgn,  arow++, 2, 1, 1 );

   connect( ck_single, SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_single( bool ) ) );
   connect( ck_edit,   SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_edit  ( bool ) ) );
   connect( ck_unasgn, SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_unasgn( bool ) ) );

   main->addLayout( advtypes );

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

   db_id1     = -2;      // flag all_models start,end IDs unknown
   db_id2     = -2;

   // Trigger models list from db or disk source
   select_diskdb();
}

// Load model data by index
int US_ModelLoader::load_model( US_Model& model, int index )
{
   int  rc      = 0;
   loadDB       = dkdb_cntrls->db();

   model.components.clear();
   model.associations.clear();

   if ( loadDB )
   {
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP         = new US_DB2( pw.getPasswd() );

         if ( ( rc = dbP->lastErrno() ) != US_DB2::OK )
         {
            QMessageBox::information( this,
                  tr( "DB Connection Problem" ),
                  tr( "There was an error connecting to the database:\n" )
                  + dbP->lastError() );
            return rc;
         }
      }

qDebug() << "LdM: index" << index << "descsz" << model_descriptions.size();
      QString   modelID  = model_descriptions[ index ].DB_id;

      rc   = model.load( modelID, dbP );
qDebug() << "LdM:  model load rc" << rc;
   }

   else
   {
      QString   filename = model_descriptions[ index ].filename;

      rc   = model.load( filename );
   }

   return rc;
}

// Return a description string for a model by index
QString US_ModelLoader::description( int index )
{
   QString sep    = ";";     // use semi-colon as separator

   if ( model_descriptions[ index ].description.contains( sep ) )
      sep            = "^";  // use carat if semi-colon already in use

   // Create and return a composite description string
   QString cdesc  = sep + model_descriptions[ index ].description
                  + sep + model_descriptions[ index ].filename
                  + sep + model_descriptions[ index ].modelGUID
                  + sep + model_descriptions[ index ].DB_id
                  + sep + model_descriptions[ index ].editGUID;

   if ( model_descriptions[ index ].iterations > 1 )
      cdesc  = cdesc + sep
         + QString::number( model_descriptions[ index ].iterations );

   return cdesc;
}

// Slot to respond to change in disk/db radio button select
void US_ModelLoader::select_diskdb()
{
   // Disable investigator field if from disk or normal user; Enable otherwise
   loadDB       = dkdb_cntrls->db();
   dbP          = loadDB ? dbP : NULL;
   pb_investigator->setEnabled( loadDB && ( US_Settings::us_inv_level() > 0 ) );

   // Signal model-loader caller that Disk/DB source has changed
   emit changed( loadDB );

   // Show the list of available models
   list_models();
}

// Investigator button clicked:  get investigator from dialog
void US_ModelLoader::get_person()
{
   int invID     = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, invID );

   connect( dialog,
      SIGNAL( investigator_accepted( int ) ),
      SLOT(   update_person(         int ) ) );

   dialog->exec();
}

// Slot to handle accept in investigator dialog
void US_ModelLoader::update_person( int ID )
{
   QString number = ( ID > 0 ) ? QString::number( ID ) + ": " : "";
   le_investigator->setText( number + US_Settings::us_inv_name() );

   list_models();
}
      
// List model choices (disk/db and possibly filtered by search text)
void US_ModelLoader::list_models()
{
qDebug() << "LIST_MODELS";
QDateTime time0=QDateTime::currentDateTime();
QDateTime time1=QDateTime::currentDateTime();
QDateTime time2=QDateTime::currentDateTime();
   const QString uaGUID( "00000000-0000-0000-0000-000000000000" );
   QString mfilt = le_mfilter->text();
   le_mfilter->disconnect( SIGNAL( textChanged( const QString& ) ) );
   bool listdesc = !mfilt.isEmpty();         // description filtered?
   bool listedit = do_edit || do_run;        // edit filtered?
   bool listsing = do_single;                // show singles of MC groups?
   bool listall  = !listdesc;                // unfiltered by description?
   QRegExp mpart = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
   model_descriptions.clear();               // clear model descriptions
qDebug() << "LM: desc single edit" << listdesc << listsing << listedit
 << "editGUID" << editGUID << "nruns" << runIDs.size();
   int kmmnew    = 0;
   int kmmold    = 0;
   int kmmmod    = 0;

   if ( listdesc )
   {  // filter is not empty
      listedit      = listedit | do_edit;

      if ( listedit  &&  ! can_edit   &&  ! can_run )
      {  // disallow edit filter if no edit GUID and no runs have been given
         QMessageBox::information( this,
               tr( "Edit GUID Problem" ),
               tr( "No EditGUID/runIDs given.\nEdit filter turned off." ) );
         listedit = false;
         listdesc = listsing;
         listall  = !listdesc;
      }

      if ( listsing )
      {  // if showing MC singles, re-check for filtering
         if ( !singprev )
            db_id1 = -2;  // flag re-list when list-singles flag changes
      }

      else if ( singprev )
         db_id1 = -2;     // flag re-list when list-singles flag changes
   }
qDebug() << "listall" << listall << "do_manual" << do_manual;
qDebug() << "  listdesc listedit listsing" << listdesc << listedit << listsing;

   if ( loadDB )
   {  // Model list from DB
      if ( dbP == NULL )
      {
         US_Passwd   pw;
         dbP         = new US_DB2( pw.getPasswd() );

         if ( dbP->lastErrno() != US_DB2::OK )
         {
            QMessageBox::information( this,
                  tr( "DB Connection Problem" ),
                  tr( "There was an error connecting to the database:\n" )
                  + dbP->lastError() );
            return;
         }
      }

      QStringList query;
      QString     invID = le_investigator->text().section( ":", 0, 0 );

      int countRD = model_descrs_recs .size();
      int countSD = model_descrs_sings.size();
      int kid1    = -3;
      int kid2    = -3;
qDebug() << " rd count" << countRD;
//      query << "count_models" << invID;
//      int countDB = dbP->statusQuery( query );
//qDebug() << " db count" << countDB;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      if ( countRD > 0  &&  countRD == countSD )
      {
         kid1 = model_descrs_recs[ 0           ].DB_id.toInt();
         kid2 = model_descrs_recs[ countRD - 1 ].DB_id.toInt();
      }
qDebug() << "  kid1 kid2" << kid1 << kid2;
qDebug() << "  db_id1 db_id2" << db_id1 << db_id2;

      if ( countRD == 0  ||  kid1 != db_id1  ||  kid2 != db_id2 )
      { // only re-fetch all-models list if we don't yet have it
         db_id1         = kid1;  // save start,end all_models IDs
         db_id2         = kid2;
qDebug() << "        db_id1 db_id2" << db_id1 << db_id2;
         model_descrs_recs.clear();
         query.clear();
         int nruns      = runIDs.size();
         QString editID = editGUID;

         if ( nruns == 0  &&  ! editGUID.isEmpty() )
         {
            if ( editID != "1" )
            {
               query.clear();
               query << "get_editID" << editGUID;
               dbP->query( query );
               dbP->next();
               editID         = dbP->value( 0 ).toString();
            }
         }
qDebug() << "        edit GUID,ID" << editGUID << editID;

         int kruns      = listedit ? qMax( nruns, 1 ) : 1;
qDebug() << "    kruns listedit" << kruns << listedit;

         for ( int ii = 0; ii < kruns; ii++ )
         {
qDebug() << "     ii kruns nruns" << ii << kruns << nruns;
            query.clear();
time1=QDateTime::currentDateTime();

            if ( listedit  &&  can_edit )
            {
               query << "get_model_desc_by_editID"
                     << invID << editID;
            }

            else if ( listedit  &&  can_run )
            {
               query << "get_model_desc_by_runID"
                     << invID << runIDs[ ii ];
            }

            else if ( do_manual || do_unasgn )
               query << "get_model_desc_by_editID" << invID << "1";

            else
               query << "get_model_desc" << invID;

qDebug() << " query" << query;
            dbP->query( query );
qDebug() << " NumRows" << dbP->numRows();
time2=QDateTime::currentDateTime();
qDebug() << "Timing: get_model_desc" << time1.msecsTo(time2);

            while ( dbP->next() )
            {
               ModelDesc desc;
               desc.DB_id       = dbP->value( 0 ).toString();
               desc.modelGUID   = dbP->value( 1 ).toString();
               desc.description = dbP->value( 2 ).toString();
               desc.editGUID    = dbP->value( 5 ).toString();

               desc.filename.clear();
               desc.reqGUID     = desc.description.section( ".", -2, -2 )
                                                  .section( "_",  0,  3 );
               desc.iterations  = ( desc.description.contains( "-MC" )
                                 && desc.description.contains( "_mc" ) ) ? 1: 0;
               bool skip_it     = false;

               if ( desc.description.simplified().length() < 2 )
               {
                  desc.description = " ( ID " + desc.DB_id
                                     + tr( " : empty description )" );
               }
//qDebug() << "   desc" << desc.description << "DB_id" << desc.DB_id;

               if ( do_manual )
               {  // If MANUAL, select only type Manual
                  skip_it          = ( desc.editGUID != uaGUID );

                  if ( desc.description.contains( "2DSA" )  ||
                       desc.description.contains( "PCSA" )  ||
                       desc.description.contains( "GA"   )  ||
                       desc.description.contains( "-GL" )   ||
                       desc.description.contains( "Custom" ) )
                     skip_it          = true;
qDebug() << "   (m)desc" << desc.description << "skip_it" << skip_it;
               }

               else if ( do_unasgn )
               {  // If UnAssigned, select only type Manual/Custom/Global
                  skip_it          = ( desc.editGUID != uaGUID );
qDebug() << "   (u)desc" << desc.description << "skip_it" << skip_it;
               }

               if ( skip_it )
                  continue;

qDebug() << "   desc.iters" << desc.iterations;
               if ( desc.iterations > 0 )
               {  // Accumulate counts for MC models
                  kmmmod++;
                  int mcndx        = desc.description.indexOf( "_mc" );
qDebug() << "     mcndx" << mcndx << "descr" << desc.description;
                  if ( desc.description.contains( "_mcN" ) )
                  {
                     kmmnew++;
                     int nimods       = QString( desc.description )
                                        .mid( mcndx + 4, 3 ).toInt();
                     desc.iterations  = nimods;
qDebug() << "     desc.iters(nimods)" << desc.iterations;
                  }

                  else if ( desc.description.contains( "_mc0001" ) )
                     kmmold++;
               }

               model_descrs_recs << desc;   // add to full models list
            }
         }
QDateTime time3=QDateTime::currentDateTime();
qDebug() << "a_m size" << model_descrs_recs.size()
 << "m_d size" << model_descriptions.size();

         if ( !listsing )
            records_list();         // Default: list based on model records

         else
            singles_list();         // List expanded to include singles
QDateTime time4=QDateTime::currentDateTime();
qDebug() << " (2)m_d size" << model_descriptions.size();
qDebug() << "Timing: DB-read" << time0.msecsTo(time3) << time2.msecsTo(time3);
qDebug() << "Timing: Compress" << time3.msecsTo(time4) << time2.msecsTo(time4);
      }

      QApplication::restoreOverrideCursor();
   }

   else
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
qDebug() << "   md size" << model_descrs_recs.size();
qDebug() << "   fn size" << f_names.size();

      if ( f_names.size() != model_descrs_recs.size()  ||  !listsing )
      { // only re-fetch all-models list if we don't yet have it
         QXmlStreamAttributes attr;

         model_descrs_recs.clear();
         int nruns      = runIDs.size();
qDebug() << "nruns" << nruns << "editGUID" << editGUID;

         for ( int ii = 0; ii < f_names.size(); ii++ )
         {
            QString fname( path + "/" + f_names[ ii ] );
qDebug() << "fname" << f_names[ii] << "do_manual" << do_manual;
            QFile   m_file( fname );

            if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
               continue;

            int nimods     = 0;
            QXmlStreamReader xml( &m_file );

            while ( ! xml.atEnd() )
            {  // Search XML elements until we find "model"
               xml.readNext();

               if ( xml.isStartElement() && xml.name() == "model" )
               {  // Pick up model attributes for description
                  attr             = xml.attributes();
                  QString edGUID   = attr.value( "editGUID"    ).toString();
                  QString descript = attr.value( "description" ).toString();
                  QString runID    = QString( descript ).section( ".", 0, -4);
                  QString mCarl    = attr.value( "monteCarlo"  ).toString();
                  int iters        = mCarl.toInt();

                  // Skip save if filtering and edit/run mismatch
                  if ( ( do_edit  &&  edGUID != editGUID )  ||
                       ( do_run   &&  !runIDs.contains( runID ) ) )
                     continue;

qDebug() << "  nimods" << nimods << "edGUID" << edGUID << "iters" << iters;
                  if ( iters > 0 )
                  {
                     nimods++;
                     if ( nimods > 1 )  continue;
                  }

                  ModelDesc desc;
                  desc.description = descript;
                  desc.modelGUID   = attr.value( "modelGUID"   ).toString();
                  desc.filename    = fname;
                  desc.DB_id       = "-1";
                  desc.editGUID    = edGUID;
                  desc.reqGUID     = attr.value( "requestGUID" ).toString();
                  desc.iterations  = iters;
                  QString aType    = attr.value( "analysisType" ).toString();
                  QString gType    = attr.value( "globalType"   ).toString();

                  if ( desc.description.simplified().length() < 2 )
                  {
                     desc.description = " ( GUID " + desc.modelGUID.left( 8 ) +
                                        "... : " + tr( "empty description )" );
                  }
//*DEBUG
//if (!listall) {
//qDebug() << " ddesc" << desc.description;
//qDebug() << "   mpart" << mpart.pattern();
//qDebug() << "   degid" << desc.editGUID;
//qDebug() << "   edgid" << editGUID;
//}
//*DEBUG
                  if ( do_manual )
                  {  // If MANUAL, select only type Manual
                     int iaType    = aType.toInt();
//qDebug() << "   iaType igType" << iaType << igType;
                     if ( iaType != US_Model::MANUAL )
                        continue;
                  }

                  else if ( do_unasgn )
                  {  // If UnAssigned, select only type Manual/Custom/Global
                     int iaType    = aType.toInt();
                     int igType    = gType.toInt();
//qDebug() << "   iaType igType" << iaType << igType;
                     if ( iaType != US_Model::MANUAL      &&
                          iaType != US_Model::CUSTOMGRID  &&
                          iaType != US_Model::DMGA_CONSTR &&
                          igType != US_Model::GLOBAL      &&
                          igType != US_Model::SUPERGLOBAL )  
                        continue;
                  }

                  if ( iters > 0 )
                  {  // Accumulate counts for MC models
                     kmmmod++;
                     if ( desc.description.contains( "_mc0001" ) )
                        kmmold++;
qDebug() << "      kmmmod" << kmmmod << "kmmold" << kmmold;
//if(kmmold==0) qDebug() << "       desc=" << desc.description;
                  }

                  model_descrs_recs << desc;   // add to full models list

                  if ( iters == 0 )
                     break;
               }

               else if ( xml.isEndElement() && xml.name() == "ModelData" )
               {  // Handle end of XML for possible multi-model composite
                  if ( nimods > 1 )
                  {  // If composite MC, get and delete last description
                     ModelDesc desc   = model_descrs_recs.takeLast();
                     int mcndx        = desc.description.indexOf( "_mc0" );
                     desc.description =
                          QString( desc.description ).left( mcndx )
                        + QString().sprintf( "mcN%03i", nimods )
                        + QString( desc.description ).mid( mcndx + 7 );
                     desc.iterations  = nimods;

                     // Restore modified description to full models list
                     model_descrs_recs << desc;
                     kmmnew++;
                     kmmold--;
                  }
               }
            }

            m_file.close();
         }

         if ( !listsing )
            records_list();         // Default: list from model records

         else
            singles_list();         // List expanded to include singles
      }
      db_id1            = -2;       // Flag all_models start,end IDs unknown
      db_id2            = -2;
   }
 
   if ( kmmold > 0 )
   {
      QString msg = tr( "%1 MC model sets are old-style separate models\n"
                        "%2 MC models are new-style composite models\n"
                        "%3 total MC model records currently exist.\n"
                        "The old-style models should be converted\n"
                        "  or deleted." )
                    .arg( kmmold).arg( kmmnew ).arg( kmmmod );
      QMessageBox::information( this,
            tr( "Deprecated MC Model Types Exist" ), msg ); 
   }

   // possibly pare down models list based on search field
QDateTime time5=QDateTime::currentDateTime();
qDebug() << "Timing: Time5" << time0.msecsTo(time5) << time2.msecsTo(time5);
qDebug() << " (3)m_d_u size" << model_descrs_ufilt.size();

   if ( listall )
   {  // No filtering or filter by edit already done
      for ( int jj = 0; jj < model_descrs_ufilt.size(); jj++ )
      {
         model_descriptions << model_descrs_ufilt[ jj ];
      }
   }

   else
   {  // Filter by model description sub-string
      for ( int jj = 0; jj < model_descrs_ufilt.size(); jj++ )
      {
         if ( model_descrs_ufilt[ jj ].description.contains( mpart ) )
         {  // description filter matches
            model_descriptions << model_descrs_ufilt[ jj ];
//ModelDesc desc = model_descrs_recs[jj];
//qDebug() << " ddesc" << desc.description << jj;
//qDebug() << "   mpart" << mpart.pattern();
         }
      }
   }
qDebug() << " (4)m_d size" << model_descriptions.size();

   lw_models->disconnect( SIGNAL( currentRowChanged( int ) ) );
   lw_models->clear();
   int maxlch   = 0;

   if ( model_descriptions.size() > 0 )
   {
      for ( int ii = 0; ii < model_descriptions.size(); ii++ )
      {  // propagate list widget with descriptions
         lw_models->addItem( model_descriptions[ ii ].description );
         maxlch    = qMax( maxlch,
                           model_descriptions[ ii ].description.length() );
      }

      // Sort descriptions in ascending alphabetical order
      lw_models->sortItems();
   }

   else
   {
      lw_models->addItem( "No models found." );
   }
QDateTime time6=QDateTime::currentDateTime();
qDebug() << "Timing: Time6" << time0.msecsTo(time6) << time2.msecsTo(time6);

   singprev   = listsing;    // save list-singles flag

   // Resize the widget to show listed items well
   QFontMetrics fm = lw_models->fontMetrics();
   int olwid    = lw_models->width();
   int olhgt    = lw_models->height();
   int nlines   = qMin( model_descriptions.size(), 30 );
   int width    = qMin( 600, maxlch * fm.maxWidth()    );
   int height   = qMin( 800, nlines * fm.lineSpacing() );
   width        = qMax( width,  olwid );
   height       = ( height > olhgt ) ? height : ( ( olhgt + height ) / 2 );
   width        = this->width()  + width  - olwid;
   height       = this->height() + height - olhgt;

   resize( width, height );

   connect( le_mfilter,    SIGNAL( textChanged( const QString& ) ),
            this,          SLOT(   msearch(     const QString& ) ) );
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

qDebug() << "ACC: multi" << multi;
   if ( ! multi )
   {  // in single-select mode, load the model and set the description
qDebug() << "ACC: load... (single)";
      load_model( omodel, 0 );
qDebug() << "ACC: ...loaded (single)";
      odescr     = description( 0 );
   }

   else
   {  // in multiple-select mode, load all models and descriptions
      omodels.clear();
      odescrs.clear();

QDateTime time1=QDateTime::currentDateTime();
qDebug() << "ACC: load... (multi) mCnt" << modelsCount;
      for ( int ii = 0; ii < modelsCount; ii++ )
      {
         load_model( model, ii );

         omodels << model;
         odescrs << description( ii );
      }
qDebug() << "ACC: ...loaded (multi)";
QDateTime time2=QDateTime::currentDateTime();
qDebug() << "Timing: accept-load: mcount" << modelsCount
 << "time(ms)" << time1.msecsTo(time2);
   }

   // Return search string that reflects current state
   dsearch    = le_mfilter->text();

   if ( do_edit )
      dsearch    = "=e " + dsearch;
   if ( do_single )
      dsearch    = "=s " + dsearch;
   if ( do_unasgn )
      dsearch    = "=u " + dsearch;
   if ( do_manual )
      dsearch    = "=m " + dsearch;

   dsearch    = dsearch.simplified();

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
   QString anlid;

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
      mdesc    = mdesc.length() < 50 ? mdesc :
                 mdesc.left( 23 ) + "..." + mdesc.right( 24 );
      ncomp    = model.components.size();
      nassoc   = model.associations.size();
      tdesc    = model.typeText();
      iters    = !model.monteCarlo ? 0 :
                 model_descriptions[ mdx ].iterations;
      anlid    = model_descriptions[ mdx ].reqGUID;
      runid    = mdesc.section( ".", 0, 0 );
      int jts  = runid.indexOf( "_" + tdesc );
      runid    = jts > 0 ? runid.left( jts ) : runid;
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
         + tr( "\n  Description Global ID: " ) + model_descriptions[ mdx ]
                                                 .modelGUID
         + tr( "\n  Edit Global ID:        " ) + model.editGUID
         + tr( "\n  Request Global ID:     " ) + model.requestGUID
         + lblid + mdlid
         + tr( "\n  Iterations:            " ) + QString::number( iters )
         + tr( "\n  Components Count:      " ) + QString::number( ncomp )
         + tr( "\n  Associations Count:    " ) + QString::number( nassoc )
         + tr( "\n  List Row:              " ) + QString::number( row )
         + tr( "\n  Analysis Run ID:       " ) + anlid
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
      tdesc    = model.typeText();
      int jts  = runid.indexOf( "_" + tdesc );
      runid    = jts > 0 ? runid.left( jts ) : runid;
      aruni    = runid;                           // potential common values
      atype    = tdesc;
      aegid    = model.editGUID;
      anlid    = model_descriptions[ mdx ].reqGUID;

      // make a pass to see if runID and type are common

      for ( int jj = 1; jj < modelsCount; jj++ )
      {
         row      = lw_models->row( selmods[ jj ] );
         mdesc    = selmods[ jj ]->text();
         mdx      = modelIndex( mdesc, model_descriptions ); // model index

         load_model( model, mdx );                           // load model

         runid    = mdesc.section( ".", 0, 0 );
         tdesc    = model.typeText();
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
         mdesc    = mdesc.length() < 50 ? mdesc :
                    mdesc.left( 23 ) + "..." + mdesc.right( 24 );
         ncomp    = model.components.size();
         nassoc   = model.associations.size();
         tdesc    = model.typeText();
         runid    = mdesc.section( ".", 0, 0 );
         mdlid    = frDisk ?
            model_descriptions[ mdx ].filename :              // ID is filename
            model_descriptions[ mdx ].DB_id;                  // ID is DB id
         mdlid    = mdlid.length() < 50 ? mdlid :
            "*/" + mdlid.section( "/", -3, -1 );              // short filename
         mdlid    = mdlid.length() < 50 ? mdlid :
            mdlid.left( 23 ) + "..." + mdlid.right( 24 );     // short filename
         iters    = !model.monteCarlo ? 0 :
                    model_descriptions[ mdx ].iterations;
         anlid    = model_descriptions[ mdx ].reqGUID;

         dtext    = dtext + tr( "\n\nModel Information: (" )
            + QString::number( ( jj + 1 ) ) + "):"
            + tr( "\n  Description:           " ) + mdesc
            + tr( "\n  Implied RunID:         " ) + runid
            + tr( "\n  Type:                  " ) + tdesc
            + "  (" + QString::number( (int)mtype ) + ")"
            + tr( "\n  Model Global ID:       " ) + model.modelGUID
            + tr( "\n  Description Global ID: " ) + model_descriptions[ mdx ]
                                                    .modelGUID
            + tr( "\n  Edit Global ID:        " ) + model.editGUID
            + tr( "\n  Request Global ID:     " ) + model.requestGUID
            + lblid + mdlid
            + tr( "\n  Iterations:            " ) + QString::number( iters )
            + tr( "\n  Components Count:      " ) + QString::number( ncomp )
            + tr( "\n  Associations Count:    " ) + QString::number( nassoc )
            + tr( "\n  List Row:              " ) + QString::number( row )
            + tr( "\n  Analysis Run ID:       " ) + anlid
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

// Duplicate the models-from-records list to the full unfiltered list
void US_ModelLoader::records_list( void )
{
   model_descrs_ufilt = model_descrs_recs;
}

// Expand the models to include singles in the full unfiltered list
void US_ModelLoader::singles_list( void )
{
   model_descrs_ufilt.clear();           // Clear the unfiltered models list

   for ( int ii = 0; ii < model_descrs_recs.size(); ii++ )
   {  // Duplicate/expand each model record to the unfiltered models list
      ModelDesc mdesc     = model_descrs_recs[ ii ];
      mdesc.rec_index     = ii;
      int mindx           = mdesc.description.indexOf( "_mcN" );
qDebug() << "SL: ii" << ii << "mindx" << mindx;

      if ( mindx < 0 )
      {  // For non-MC-composite, just duplicate
         model_descrs_ufilt << mdesc;
      }

      else
      {  // For MC-composite, expand to all the single iteration models
         QString cdtext      = mdesc.description;
         QString cmiter      = QString( cdtext ).mid( mindx, 7 );
         int niters          = QString( cmiter ).mid( 4 ).toInt();
         mdesc.iterations    = niters;
qDebug() << "SL:   niters" << niters << "cmiter" << cmiter;

         for ( int jj = 1; jj <= niters; jj++ )
         {  // Create and append a description for each iteration model
            QString imiter      = QString().sprintf( "_mc%04d", jj );
            ModelDesc idesc     = mdesc;
            idesc.description   = QString( cdtext ).replace( cmiter, imiter );
qDebug() << "SL:     jj" << jj << "imiter" << imiter;
         
            model_descrs_ufilt << idesc;
         }
      }
   }
}

// Slot to re-list models when search text has changed
void US_ModelLoader::msearch( const QString& search_string )
{
   dsearch  = search_string;

   if ( search_string.endsWith( "=" ) )
      return;

   list_models();
}

// Slot to re-list models after change in Single checkbox
void US_ModelLoader::change_single( bool cksing )
{
   do_single = cksing;

   list_models();
}

// Slot to re-list models after change in Edit checkbox
void US_ModelLoader::change_edit( bool ckedit )
{
   do_edit   = ckedit;
   do_unasgn = ck_unasgn->isChecked();
   db_id1    = -2;  // flag re-list when list-edit flag changes
   db_id2    = -2;

   list_models();
}

// Slot to re-list models after change in Unassigned checkbox
void US_ModelLoader::change_unasgn( bool ckunasgn )
{
   do_unasgn = ckunasgn;
   db_id1    = -2;  // flag re-list when list-unasgn flag changes
   db_id2    = -2;

   if ( do_unasgn )
   {  // If unassigned now checked, turn off edit (unless id=1)
      do_edit   = false;

      if ( can_edit )
      {
         do_edit         = ( editGUID == "1" );
      }

      ck_edit  ->setChecked( do_edit );
   }

   list_models();
}


