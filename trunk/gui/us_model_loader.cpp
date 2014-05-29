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
   editIDs( weditIDs )
{
   multi    = false;
   editGUID = eGUID;

   build_dialog();
}

// Alternate constructor for loading a single model (with editIDs)
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
      US_Model& amodel, QString& adescr, QStringList& aeditIDs )
  :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( amodel ), odescr( adescr ), omodels( wmodels ), odescrs( mdescrs ),
   editIDs( aeditIDs )
{
   multi    = false;
   editGUID = "";

   build_dialog();
}

// Alternate constructor that allows loading multiple models
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
   QList< US_Model >& amodels, QStringList& adescrs,
   QStringList& aeditIDs )
   :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( model ), odescr( search ), omodels( amodels ), odescrs( adescrs ),
   editIDs( aeditIDs )
{
   multi    = true;
   editGUID = "";

   build_dialog();
}

// Alternate constructor that allows loading multiple models (no editIDs list)
US_ModelLoader::US_ModelLoader( bool dbSrc, QString& search,
   QList< US_Model >& amodels, QStringList& adescrs )
   :US_WidgetsDialog( 0, 0 ), loadDB( dbSrc ), dsearch( search ),
   omodel( model ), odescr( search ), omodels( amodels ), odescrs( adescrs ),
   editIDs( weditIDs )
{
   multi    = true;
   editGUID = "";

   build_dialog();
}

// Main shared method to build the model loader dialog
void US_ModelLoader::build_dialog( void )
{
qDebug() << "ML:BD: editIDs empty" << editIDs.isEmpty();
   setWindowTitle( multi ? tr( "Load Distribution Model(s)" )
                         : tr( "Load Distribution Model" ) );
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
   singprev          = false;

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
   QString number = ( inv > 0 ) ? QString::number( inv ) + ": " : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
         0, true );

   pb_filtmodels   = us_pushbutton( tr( "Search" ) );
   connect( pb_filtmodels, SIGNAL( clicked() ),
            this,          SLOT( list_models() ) );

   do_single  = false;
   do_edit    = ( editIDs.size() > 0  ||  !editGUID.isEmpty() );
   do_manual  = false;

   if ( ! dsearch.isEmpty() )
   {  // If an input search string is given, look for special flags
      do_single  = dsearch.contains( "=s" );
      do_manual  = dsearch.contains( "=m" );
      do_edit    = do_manual ? false : do_edit;
      dsearch.replace( "=m ", "" ).simplified();
      dsearch.replace( "=m",  "" ).simplified();
   }

   le_mfilter      = us_lineedit( "", -1, false );
   //dsearch         = dsearch.isEmpty() ? QString( "" ) : dsearch;
   le_mfilter->setText( dsearch );
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
   QGridLayout* lo_edit    = us_checkbox( tr( "Filter by Edit" ),
                                          ck_edit,    do_edit   );
   QGridLayout* lo_manual  = us_checkbox( tr( "Custom/Global Only" ),
                                          ck_manual,  do_manual );
   int arow   = 0;
   advtypes->addWidget( lb_advopts, arow++, 0, 1, 3 );
   advtypes->addLayout( lo_single,  arow,   0, 1, 1 );
   advtypes->addLayout( lo_edit,    arow,   1, 1, 1 );
   advtypes->addLayout( lo_manual,  arow++, 2, 1, 1 );

   connect( ck_single, SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_single( bool ) ) );
   connect( ck_edit,   SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_edit  ( bool ) ) );
   connect( ck_manual, SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_manual( bool ) ) );

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

      rc   = model.load( modelID, &db );
   }

   else
   {
      QString   filename = model_descriptions[ index ].filename;

      rc   = model.load( filename );
   }

   if ( model_descriptions[ index ].iterations > 1 )
   {  // Multiple model load
      US_Passwd pw;
      US_DB2* dbP = loadDB ? new US_DB2( pw.getPasswd() ) : NULL;

      int nm = model_descriptions[ index ].iterations;
      int kk = model_descriptions[ index ].asd_index;

      for ( int ii = 1; ii < nm; ii++ )
      {
         US_Model model2;

         if ( loadDB )
         {
            QString modelID  = all_single_descrs[ ++kk ].DB_id;
            rc   = model2.load( modelID, dbP );
         }

         else
         {
            QString filename = all_single_descrs[ ++kk ].filename;
            rc   = model2.load( filename );
         }

         // Append group member's components to the original
         model.components << model2.components;
      }

      double scfactor = 1.0 / (double)nm;
      QStringList sklist;
      QStringList skvals;
      QVector< US_Model::SimulationComponent > comps;
//qDebug() << "ML: nm scfactor" << nm << scfactor;

      // Do a scan to determine if constant vbar
      double vbarmax = model.components[ 0 ].vbar20;
      double vbarmin = vbarmax;

      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         vbarmax = qMax( vbarmax, model.components[ ii ].vbar20 );
         vbarmin = qMin( vbarmin, model.components[ ii ].vbar20 );
      }

      bool cnst_vbar = ( vbarmax == vbarmin );

      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         QString skval;
         comps << model.components[ ii ];

         if ( cnst_vbar )
         {
            skval = QString().sprintf( "%9.4e %9.4e",
               model.components[ ii ].s, model.components[ ii ].f_f0 );
         }
         else
         {
            skval = QString().sprintf( "%9.4e %9.4e",
               model.components[ ii ].s, model.components[ ii ].vbar20 );
         }
         sklist << skval;

         if ( ! skvals.contains( skval ) )
            skvals << skval;
//qDebug() << "ML:   ii skval" << ii << skval;
      }

      int nskl = sklist.size();
      int nskv = skvals.size();
//qDebug() << "ML:  sizes sklist,skvals" << nskl << nskv;
      model.components.clear();
      skvals.sort();

      for ( int ii = 0; ii < nskv; ii++ )
      {
         QString skval = skvals[ ii ];
         double  conc  = 0.0;
         int nd = 0;
         int kk = 0;

         for ( int jj = 0; jj < nskl; jj++ )
         {
            if ( skval == sklist[ jj ] )
            {
               conc += comps[ jj ].signal_concentration;
               kk    = jj;
               nd++;
            }
         }

         comps[ kk ].signal_concentration = conc * scfactor;
//qDebug() << "ML:   ii skval" << ii << skval << "nd conc" << nd << conc;
         model.components << comps[ kk ];
      }

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
   QString mfilt = le_mfilter->text();
   le_mfilter->disconnect( SIGNAL( textChanged( const QString& ) ) );
   bool listall  = mfilt.isEmpty();          // unfiltered?
   bool listdesc = !listall;                 // description filtered?
   bool listedit = do_edit;                  // edit filtered?
   bool listsing = do_single;                // show singles of MC groups?
   QRegExp mpart = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
   model_descriptions.clear();               // clear model descriptions

   if ( listdesc )
   {  // filter is not empty
      listedit = mfilt.contains( "=e" );     // edit filtered?
      listedit = listedit | do_edit;
      listdesc = !listedit;                  // description filtered?
      listsing = mfilt.contains( "=s" );     // show singles of MC groups?
      listsing = listsing | do_single;

      if ( listedit  &&  editGUID.isEmpty() )
      {  // disallow edit filter if no edit GUID has been given
         QMessageBox::information( this,
               tr( "Edit GUID Problem" ),
               tr( "No Edit GUID was given.\n\"=e\" reset to blank." ) );
         listall  = true;
         listdesc = false;
         listedit = false;
         le_mfilter->setText( "" );
      }

      if ( listsing )
      {  // if showing MC singles, re-check for filtering
         if ( mfilt.contains( "=s" )  &&  !listedit )
         {  // a filter can be added after "=s "
            int jj   = mfilt.indexOf( "=s" );
            mfilt    = ( jj == 0 ) ? 
                       mfilt.mid( jj + 3 ).simplified() :
                       mfilt.left( jj ).simplified();
            listdesc = !mfilt.isEmpty();
            listall  = !listdesc;
            mpart    = QRegExp( ".*" + mfilt + ".*", Qt::CaseInsensitive );
qDebug() << "=listsing= jj mfilt mpart" << jj << mfilt << mpart.pattern();
         }

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
      int countSD = all_single_descrs .size();
      int kid1    = -3;
      int kid2    = -3;
qDebug() << " md count" << countMD;
//      query << "count_models" << invID;
//      int countDB = db.statusQuery( query );
//qDebug() << " db count" << countDB;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      if ( countMD > 0  &&  countMD == countSD )
      {
         kid1 = all_model_descrips[ 0           ].DB_id.toInt();
         kid2 = all_model_descrips[ countMD - 1 ].DB_id.toInt();
      }
qDebug() << "  kid1 kid2" << kid1 << kid2;
qDebug() << "  db_id1 db_id2" << db_id1 << db_id2;

      if ( countMD == 0  ||  kid1 != db_id1  ||  kid2 != db_id2 )
      { // only re-fetch all-models list if we don't yet have it
         db_id1            = kid1;  // save start,end all_models IDs
         db_id2            = kid2;
qDebug() << "        db_id1 db_id2" << db_id1 << db_id2;
         all_model_descrips.clear();
         query.clear();
         int nedits        = editIDs.size();
         QString editID = "";

         if ( nedits == 0  &&  ! editGUID.isEmpty() )
         {
            query.clear();
            query << "get_editID" << editGUID;
            db.query( query );
            db.next();
            editID = db.value( 0 ).toString();
            if ( !editID.isEmpty()  &&  dsearch.contains( "=e" ) )
            {
               dsearch = dsearch.replace( "=e", "" ).simplified();
               le_mfilter->setText( dsearch );
            }
         }
qDebug() << "        edit GUID,ID" << editGUID << editID;

         for ( int ii = 0; ii < qMax( nedits, 1 ); ii++ )
         {
qDebug() << "     ii nedits" << ii << nedits;
            query.clear();
time1=QDateTime::currentDateTime();
            if ( listedit && nedits > 0 )
               query << "get_model_desc_by_editID"
                     << invID << editIDs[ ii ];

            else if ( listedit && !editID.isEmpty() )
               query << "get_model_desc_by_editID"
                     << invID << editID;

            else
               query << "get_model_desc" << invID;

            db.query( query );
qDebug() << " NumRows" << db.numRows();
time2=QDateTime::currentDateTime();
qDebug() << "Timing: get_model_desc" << time1.msecsTo(time2);

            while ( db.next() )
            {
               ModelDesc desc;
               desc.DB_id       = db.value( 0 ).toString();
               desc.modelGUID   = db.value( 1 ).toString();
               desc.description = db.value( 2 ).toString();
               desc.editGUID    = db.value( 5 ).toString();

               desc.filename.clear();
               desc.reqGUID     = desc.description.section( ".", -2, -2 )
                                                  .section( "_",  0,  3 );
               desc.iterations  = ( desc.description.contains( "-MC" )
                                 && desc.description.contains( "_mc" ) ) ? 1: 0;

               if ( desc.description.simplified().length() < 2 )
               {
                  desc.description = " ( ID " + desc.DB_id
                                     + tr( " : empty description )" );
               }
//qDebug() << "   desc" << desc.description << "DB_id" << desc.DB_id;

               if ( do_manual )
               {  // If MANUAL, select only type Custom or Global
                  bool skip_it     = true;

                  if ( desc.description.contains( "-GL" )     ||
                       desc.description.contains( "Custom" )  ||
                       desc.description.contains( "Discrete" ) )
                     skip_it          = false;

                  else if ( ! desc.description.contains( "2DSA" )  &&
                            ! desc.description.contains( "PCSA" )  &&
                            ! desc.description.contains( "GA"   ) )
                     skip_it          = false;
qDebug() << "   desc" << desc.description << "skip_it" << skip_it;

                  if ( skip_it )
                     continue;
               }

               all_model_descrips << desc;   // add to full models list
            }
         }
QDateTime time3=QDateTime::currentDateTime();

         if ( !listsing )
            compress_list();          // default: compress MC groups

         else
            dup_singles();            // duplicate model list as singles
QDateTime time4=QDateTime::currentDateTime();
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
//qDebug() << "   md size" << all_model_descrips.size();
//qDebug() << "   fn size" << f_names.size();

      if ( f_names.size() != all_model_descrips.size()  ||  !listsing )
      { // only re-fetch all-models list if we don't yet have it
         QXmlStreamAttributes attr;

         all_model_descrips.clear();
         int nedits     = editIDs.size();

         if ( do_edit  &&   nedits == 0  &&  ! editGUID.isEmpty() )
         {
            if ( !editGUID.isEmpty()  &&  dsearch.contains( "=e" ) )
            {
               dsearch        = dsearch.replace( "=e", "" ).simplified();
               le_mfilter->setText( dsearch );
            }
         }

         for ( int ii = 0; ii < f_names.size(); ii++ )
         {
            QString fname( path + "/" + f_names[ ii ] );
//qDebug() << "fname" << f_names[ii] << "do_manual" << do_manual;
            QFile   m_file( fname );

            if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
               continue;

            QXmlStreamReader xml( &m_file );

            while ( ! xml.atEnd() )
            {  // Search XML elements until we find "model"
               xml.readNext();

               if ( xml.isStartElement() && xml.name() == "model" )
               {  // Pick up model attributes for description
                  attr             = xml.attributes();
                  QString edGUID   = attr.value( "editGUID"    ).toString();

                  if ( do_edit  &&
                       ( ( nedits > 0  &&  ! editIDs.contains( edGUID ) )  ||
                         ( !editGUID.isEmpty()  &&  edGUID != editGUID ) ) )
                     continue;

                  ModelDesc desc;
                  desc.description = attr.value( "description" ).toString();
                  desc.modelGUID   = attr.value( "modelGUID"   ).toString();
                  desc.filename    = fname;
                  desc.DB_id       = "-1";
                  desc.editGUID    = edGUID;
                  desc.reqGUID     = attr.value( "requestGUID" ).toString();
                  QString mCarl    = attr.value( "monteCarlo"  ).toString();
                  desc.iterations  = mCarl.toInt() == 0 ?  0 : 1;
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
                  {  // If MANUAL, select only type Custom or Global
                     int iaType    = aType.toInt();
                     int igType    = gType.toInt();
//qDebug() << "   iaType igType" << iaType << igType;
                     if ( iaType != US_Model::MANUAL      &&
                          iaType != US_Model::CUSTOMGRID  &&
                          iaType != US_Model::DISCRETEGA  &&
                          igType != US_Model::GLOBAL      &&
                          igType != US_Model::SUPERGLOBAL )  
                        continue;
                  }


                  all_model_descrips << desc;   // add to full models list
                  break;
               }
            }

            m_file.close();
         }

         if ( !listsing )
            compress_list();       // default: compress MC groups

         else
            dup_singles();         // duplicate model list as singles
      }
      db_id1            = -2;      // flag all_models start,end IDs unknown
      db_id2            = -2;
   }


   // possibly pare down models list based on search field
QDateTime time5=QDateTime::currentDateTime();
qDebug() << "Timing: Time5" << time0.msecsTo(time5) << time2.msecsTo(time5);

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
         if ( all_model_descrips[ jj ].editGUID.contains( editGUID ) )
         {  // edit filter matches
            model_descriptions << all_model_descrips[ jj ];
//ModelDesc desc = all_model_descrips[jj];
//qDebug() << " ddesc" << desc.description;
//qDebug() << "   degid" << desc.editGUID;
//qDebug() << "   edgid" << editGUID;
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

   if ( ! multi )
   {  // in single-select mode, load the model and set the description
      load_model( omodel, 0 );
      odescr     = description( 0 );
   }

   else
   {  // in multiple-select mode, load all models and descriptions
      omodels.clear();
      odescrs.clear();

      for ( int ii = 0; ii < modelsCount; ii++ )
      {
         load_model( model, ii );

         omodels << model;
         odescrs << description( ii );
      }
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

// Compress all-models description list for MC groups
void US_ModelLoader::compress_list( void )
{
   // First, produce a sorted singles copy of all-descriptions list
   dup_singles();

   // Now, produce all-models list with only first singles of groups
   all_model_descrips.clear();
   int     kiter  = 0;          // count of MC iterations in group
   QString pReqID = "";         // previous request GUID
   QString pEdiID = "";         // previous edit GUID

//qDebug() << "compress_list:";
   for ( int ii = 0; ii < all_single_descrs.size(); ii++ )
   {  // review each single model description
      ModelDesc desc  = all_single_descrs[ ii ];  // model description object
      QString cReqID  = desc.reqGUID;             // current request GUID
      QString cEdiID  = desc.editGUID;            // current edit GUID
      // protect against missing or dummy GUIDs causing false grouping
      cReqID  = ( cReqID.length() < 16  ||  cReqID.startsWith( "00000000-" ) )
                ? QString::number( ii ) : cReqID;
      cEdiID  = ( cEdiID.length() < 36  ||  cEdiID.startsWith( "00000000-" ) )
                ? QString::number( ii ) : cEdiID;
      // Protect against global mc Global/nonGlobal pairs
      cReqID  = desc.description.startsWith( "Global" ) ?
                ( cReqID + "-G" ) : cReqID;
//qDebug() << " c_l ii desc" << ii << desc.description << " kiter" << kiter;

      if ( kiter > 0  && ( cReqID != pReqID || cEdiID != pEdiID ) )
      {  // previous was end of group:  update the iterations count
         all_model_descrips.last().iterations = kiter;
//qDebug() << " KITER MC" << kiter;
         kiter = 0;
      }

      if ( desc.iterations == 0 )
      {  // not monte carlo:  copy the model description as is
         all_model_descrips << desc;
         kiter = 0;
      }

      else
      {  // monte carlo:  bump iterations count and copy if first
         if ( ++kiter == 1 )
            all_model_descrips << desc;
      }

      pReqID  = cReqID;      // save request,edit GUIDs for next pass
      pEdiID  = cEdiID;
   }

   if ( kiter > 0  )
   {  // last was end of MC group:  update the iterations count
      all_model_descrips.last().iterations = kiter;
//qDebug() << " KITER MC" << kiter;
   }
}

// Duplicate all-models description list to show singles in MC groups
void US_ModelLoader::dup_singles( void )
{
   QStringList descrs;
   QStringList sdescs;

   for ( int ii = 0; ii < all_model_descrips.size(); ii++ )
   { // create duplicate lists of concatenated descripion+GUID
      descrs << all_model_descrips[ ii ].description + "^"
             +  all_model_descrips[ ii ].modelGUID;
      sdescs << all_model_descrips[ ii ].description + "^"
             +  all_model_descrips[ ii ].modelGUID;
   }

   sdescs.sort();                // create a sorted version of desc+guid list
   all_single_descrs.clear();    // clear all-singles list

   for ( int ii = 0; ii < descrs.size(); ii++ )
   {  // find sorted entry in unsorted list; append that full model description
      int jj = descrs.indexOf( sdescs[ ii ] );
      ModelDesc desc  = all_model_descrips[ jj ];
      desc.asd_index  = ii; 

      all_single_descrs << desc;
   }

   // copy sorted all-singles list to all-models list
   all_model_descrips = all_single_descrs;
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
   do_manual = ck_manual->isChecked();
   db_id1    = -2;  // flag re-list when list-edit flag changes
   db_id2    = -2;

   list_models();
}

// Slot to re-list models after change in Manual checkbox
void US_ModelLoader::change_manual( bool ckmanu )
{
   do_manual = ckmanu;
   db_id1    = -2;  // flag re-list when list-manual flag changes
   db_id2    = -2;

   if ( do_manual )
   {  // If manual now checked, turn off edit
      ck_edit  ->setChecked( false );
      do_edit   = false;
   }

   list_models();
}


