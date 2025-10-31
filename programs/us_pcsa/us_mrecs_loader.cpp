//! \file us_mrecs_loader.cpp

#include "us_mrecs_loader.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_util.h"

// Main constructor for loading a single model records entry
US_MrecsLoader::US_MrecsLoader( bool dbSrc, QString& search,
      VEC_MRECS& amrecs, QString& adescr,
      const QString eGUID, const QString arunID )
  :US_WidgetsDialog( nullptr, Qt::WindowFlags() ), loadDB( dbSrc ), dsearch( search ),
   omrecs( amrecs ), odescr( adescr )
{
   editGUID = eGUID;
   runID    = arunID;

   setWindowTitle( tr( "Load a Model Records Vector" ) );
   setPalette( US_GuiSettings::frameColor() );
   setMinimumSize( 320, 300 );

   mrecs_descriptions.clear();
   mrecs_descrs_recs .clear();

   // Main layout
   QVBoxLayout* main = new QVBoxLayout( this );
   main->setContentsMargins( 2, 2, 2, 2 );
   main->setSpacing        ( 2 );

   // Top layout: buttons and fields above list widget
   QGridLayout* top  = new QGridLayout( );
   dbP               = NULL;

   pb_filtvmrecs   = us_pushbutton( tr( "Search" ) );
   connect( pb_filtvmrecs, SIGNAL( clicked() ),
            this,          SLOT( list_vmrecs() ) );

   QString edGUID  = editGUID;
   can_edit        = !editGUID.isEmpty();
   do_edit         = can_edit;

   if ( ! dsearch.isEmpty() )
   {  // If an input search string is given, look for special flags
      do_edit         = do_edit || dsearch.contains( "=e" );
      dsearch         = dsearch.replace( "=e",  "" ).simplified();
   }
qDebug() << "Bld: edit" << do_edit << " dsearch" << dsearch;

   le_mfilter      = us_lineedit( dsearch, -1, false );
   connect( le_mfilter,    SIGNAL( returnPressed() ),
            this,          SLOT(   list_vmrecs()   ) );
   connect( le_mfilter,    SIGNAL( textChanged( const QString& ) ),
            this,          SLOT(   msearch(     const QString& ) ) );

   int row          = 0;
   top->addWidget( pb_filtvmrecs,   row,   0 );
   top->addWidget( le_mfilter,      row++, 1 );

   main->addLayout( top );

   // List widget to show model choices
   lw_vmrecs       = new US_ListWidget;

   main->addWidget( lw_vmrecs );

   // Advanced Mrecs List Options
   QGridLayout* lo_edit    = us_checkbox( tr( "Filter by Edit" ),
                                          ck_edit,    do_edit  );
   connect( ck_edit,   SIGNAL( toggled      ( bool ) ),
                       SLOT  ( change_edit  ( bool ) ) );

   main->addLayout( lo_edit );

   // Button Row
   QHBoxLayout* buttons = new QHBoxLayout;

                pb_delete   = us_pushbutton( tr( "Delete Entry" ) );
                pb_details  = us_pushbutton( tr( "Entry Details" ) );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel   = us_pushbutton( tr( "Cancel" ) );
                pb_accept   = us_pushbutton( tr( "Accept" ) );

   connect( pb_delete,  SIGNAL( clicked() ), this, SLOT( delete_mrecs()    ) );
   connect( pb_details, SIGNAL( clicked() ), this, SLOT( show_mrecs_info() ) );
   connect( pb_help,    SIGNAL( clicked() ), this, SLOT( help()            ) );
   connect( pb_cancel,  SIGNAL( clicked() ), this, SLOT( cancelled()       ) );
   connect( pb_accept,  SIGNAL( clicked() ), this, SLOT( accepted()        ) );

   buttons->addWidget( pb_details );
   buttons->addWidget( pb_delete  );
   buttons->addWidget( pb_help    );
   buttons->addWidget( pb_cancel  );
   buttons->addWidget( pb_accept  );

   pb_delete ->setEnabled( false );
   pb_details->setEnabled( false );
   pb_accept ->setEnabled( false );

   main->addLayout( buttons );

   db_id1     = -2;      // flag all_models start,end IDs unknown
   db_id2     = -2;

   list_vmrecs();
}

// Load model records data by index
int US_MrecsLoader::load_mrecs( VEC_MRECS& mrecs, int index )
{
   int rc      = 0;
   int nmrec   = 0;
   QString xmlstr;

   if ( loadDB )
   {
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP         = new US_DB2( pw.getPasswd() );

         if ( ( rc = dbP->lastErrno() ) != IUS_DB2::OK )
         {
            QMessageBox::information( this,
                  tr( "DB Connection Problem" ),
                  tr( "There was an error connecting to the database:\n" )
                  + dbP->lastError() );
            return rc;
         }
      }

qDebug() << "LdM: index" << index << "descsz" << mrecs_descriptions.size();
      QString     mrecsID  = mrecs_descriptions[ index ].DB_id;
      QStringList query;
      query << "get_mrecs_info" << mrecsID;
qDebug() << " query" << query;
      dbP->query( query );

      if ( dbP->lastErrno() == IUS_DB2::OK  &&  dbP->numRows() == 1 )
      {
qDebug() << "  numRows" << dbP->numRows();
         dbP->next();
         xmlstr               = dbP->value( 1 ).toString();
         QString dcksm        = dbP->value( 7 ).toString();
         int     dsize        = dbP->value( 8 ).toInt();
qDebug() << "  len(xmlstr)" << xmlstr.length();

         // If no local copy exists, make one
         QString descript     = mrecs_descriptions[ index ].description;
         QString frunID       = QString( descript ).section( ".", -4, -4 );
         QString triple       = QString( descript ).section( ".", -3, -3 );
         QString analID       = QString( descript ).section( ".", -2, -2 );
         QString anedit       = QString( analID ).section( "_", -5, -5 );
         QString anadat       = QString( analID ).section( "_", -4, -4 );
         QString antype       = QString( analID ).section( "_", -3, -3 );
         QString anrqid       = QString( analID ).section( "_", -2, -2 );
         analID               = anedit.left( 7 ) + "_"
                              + anadat.left( 7 ) + "_"
                              + antype + "_" + anrqid;
         QString fname        = "pcsa-mrs." + triple + "." + analID + ".xml";
         QString fpath        = US_Settings::resultDir() + "/" + frunID
                              + "/" + fname;
         QFile filemr( fpath );
         if ( filemr.exists() )
         {  // File exists:  check it size
            QString sumsiz       = US_Util::md5sum_file( fpath );
            QString fcksm        = QString( sumsiz ).section( " ", 0, 0 );
            QString sfsize       = QString( sumsiz ).section( " ", 1, 1 );
            int     fsize        = sfsize.toInt();
            if ( fcksm != dcksm  ||  fsize != dsize )
            {  // Differs in checksum or size:  so overwrite existing file
               if ( filemr.open( QIODevice::WriteOnly | QIODevice::Text ) )
               {
                  QTextStream tso( &filemr );
                  tso << xmlstr;
                  filemr.close();
qDebug() << "File written:" << fpath;
               }
            }
         }
         else
         {  // File does not exist:  create it
            if ( filemr.open( QIODevice::WriteOnly | QIODevice::Text ) )
            {
               QTextStream tso( &filemr );
               tso << xmlstr;
               filemr.close();
qDebug() << "File written:" << fpath;
            }
         }
      }

      else
      {
qDebug() << " *ERROR*" << dbP->lastErrno() << dbP->lastError();
      }
   }

   else
   {
      QString filepath   = mrecs_descriptions[ index ].filename;
      QString filedir    = filepath.section( "/",  0, -2 );
      QString filename   = filepath.section( "/", -1, -1 );

      QFile filei( filepath );

      if ( filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream tsi( &filei );
         xmlstr               = tsi.readAll();
         filei.close();
      }

      else
      {
qDebug() << " *ERROR* cannot open" << filepath;
      }
   }

   QXmlStreamReader xmlr( xmlstr );
   int ctype            = CTYPE_NONE;
   double xmin          = 0.0;
   double xmax          = 0.0;
   double ymin          = 0.0;
   double ymax          = 0.0;
   int stype            = 11;

qDebug() << "  ==Call load_modelrecs==";
   nmrec   = US_ModelRecord::load_modelrecs( xmlr, mrecs, mrdesc, ctype,
                                             xmin, xmax, ymin, ymax, stype );
   rc      = ( nmrec > 0 ) ? 0 : 1;
qDebug() << "  ==load_modelrecs== rc" << rc << "nmrec" << nmrec;

   return rc;
}

// Return a concatenated description string for a model by index
QString US_MrecsLoader::concat_description( int index )
{
   QString sep    = ";";     // use semi-colon as separator

   if ( mrecs_descriptions[ index ].description.contains( sep ) )
      sep            = "^";  // use carat if semi-colon already in use

   // Create and return a composite description string
   QString cdesc  = sep + mrecs_descriptions[ index ].description
                  + sep + mrecs_descriptions[ index ].filename
                  + sep + mrecs_descriptions[ index ].mrecsGUID
                  + sep + mrecs_descriptions[ index ].DB_id
                  + sep + mrecs_descriptions[ index ].editGUID;

   return cdesc;
}

// List mrecs choices (disk/db and possibly filtered by search text)
void US_MrecsLoader::list_vmrecs()
{
qDebug() << "LIST_MODELS";
QDateTime time0=QDateTime::currentDateTime();
QDateTime time1=QDateTime::currentDateTime();
QDateTime time2=QDateTime::currentDateTime();
   const QString uaGUID( "00000000-0000-0000-0000-000000000000" );
   QString mfilt = le_mfilter->text();
   le_mfilter->disconnect( SIGNAL( textChanged( const QString& ) ) );
   bool listdesc = !mfilt.isEmpty();         // description filtered?
   bool listedit = do_edit;                  // edit filtered?
   bool listall  = !listdesc;                // unfiltered by description?
   QRegularExpression mpart( ".*" + mfilt + ".*" );
   mrecs_descriptions.clear();               // clear model descriptions
qDebug() << "LM: desc edit" << listdesc << listedit
 << "editGUID" << editGUID;
   int kmmnew    = 0;
   int kmmold    = 0;
   int kmmmod    = 0;

   if ( listdesc )
   {  // filter is not empty
      listedit      = listedit | do_edit;

      if ( listedit  &&  ! can_edit  )
      {  // disallow edit filter if no edit GUID
         QMessageBox::information( this,
               tr( "Edit GUID Problem" ),
               tr( "No EditGUID given.\nEdit filter turned off." ) );
         listedit = false;
         listdesc = listedit;
         listall  = !listdesc;
      }
   }
qDebug() << "listall" << listall << "listdesc listedit" << listdesc << listedit;

   if ( loadDB )
   {  // Mrecs list from DB
      if ( dbP == NULL )
      {
         US_Passwd   pw;
         dbP         = new US_DB2( pw.getPasswd() );

         if ( dbP->lastErrno() != IUS_DB2::OK )
         {
            QMessageBox::information( this,
                  tr( "DB Connection Problem" ),
                  tr( "There was an error connecting to the database:\n" )
                  + dbP->lastError() );
            return;
         }
      }

      QStringList query;
      QString     invID = QString::number( US_Settings::us_inv_ID() );

      int countRD = mrecs_descrs_recs .size();
      int kid1    = -3;
      int kid2    = -3;
qDebug() << " rd count" << countRD;
//      query << "count_models" << invID;
//      int countDB = dbP->statusQuery( query );
//qDebug() << " db count" << countDB;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      if ( countRD > 0 )
      {
         kid1 = mrecs_descrs_recs[ 0           ].DB_id.toInt();
         kid2 = mrecs_descrs_recs[ countRD - 1 ].DB_id.toInt();
      }
qDebug() << "  kid1 kid2" << kid1 << kid2;
qDebug() << "  db_id1 db_id2" << db_id1 << db_id2;

      if ( countRD == 0  ||  kid1 != db_id1  ||  kid2 != db_id2 )
      { // only re-fetch all-models list if we don't yet have it
         db_id1         = kid1;  // save start,end all_models IDs
         db_id2         = kid2;
qDebug() << "        db_id1 db_id2" << db_id1 << db_id2;
         mrecs_descrs_recs.clear();
         QString editID = editGUID;
         countRD        = 0;

         if ( can_edit )
         {
            query.clear();
            query << "get_editID" << editGUID;
            dbP->query( query );
            dbP->next();
            editID         = dbP->value( 0 ).toString();
         }
qDebug() << "        edit GUID,ID" << editGUID << editID;

         query.clear();
time1=QDateTime::currentDateTime();

         if ( listedit  &&  can_edit )
         {
            query << "get_mrecs_desc_by_editID" << invID << editID;
         }

         else
         {
            query << "get_mrecs_desc" << invID;
         }

qDebug() << " query" << query;
         dbP->query( query );
qDebug() << " NumRows" << dbP->numRows();
time2=QDateTime::currentDateTime();
qDebug() << "Timing: get_mrecs_desc" << time1.msecsTo(time2);

         while ( dbP->next() )
         {
            MrecsDesc desc;
            desc.DB_id       = dbP->value( 0 ).toString();
            desc.mrecsGUID   = dbP->value( 1 ).toString();
            desc.editGUID    = dbP->value( 3 ).toString();
            desc.modelGUID   = dbP->value( 5 ).toString();
            desc.description = dbP->value( 6 ).toString();
            desc.rec_index   = countRD++;

            desc.filename.clear();

            if ( desc.description.simplified().length() < 2 )
            {
               desc.description = " ( ID " + desc.DB_id
                                  + tr( " : empty description )" );
            }
//qDebug() << "   desc" << desc.description << "DB_id" << desc.DB_id;

            mrecs_descrs_recs << desc;   // add to full mrecs list
         }
      }
QDateTime time3=QDateTime::currentDateTime();
qDebug() << "a_m size" << mrecs_descrs_recs.size()
 << "m_d size" << mrecs_descriptions.size();

      QApplication::restoreOverrideCursor();
   }

   else
   {  // Mrecs list from local disk files
      QDir    dir;
      QString path = US_Settings::resultDir();
      if ( !dir.exists( path ) )
      {
         dir.mkpath( path );
      }
      dir          = QDir( path );

      // Examine all "M*xml" files in models directory
      QStringList filter( "pcsa-mrs*.xml" );
      QStringList f_names;
      QStringList f_paths;
      int countRD  = 0;

      if ( runID.isEmpty() )
      {  // Scan all results directories
         QStringList r_dirs = dir.entryList( QDir::Dirs );

         for ( int jj = 0; jj < r_dirs.size(); jj++ )
         {
            QString rpath   = r_dirs[ jj ];

            QDir rdir    = QDir( rpath );

            QStringList rf_names = rdir.entryList( filter, QDir::Files,
                                                           QDir::Name );

            for ( int ii = 0; ii < rf_names.size(); ii++ )
            {
               f_names << rf_names[ ii ];
               f_paths << path + "/" + rpath + "/" + rf_names[ ii ];
            }
         }
      }

      else
      {  // Scan a specific results run directory
         QString rpath   = path + "/" + runID;

         QDir rdir    = QDir( rpath );

         QStringList rf_names = rdir.entryList( filter, QDir::Files,
                                                        QDir::Name );

         for ( int ii = 0; ii < rf_names.size(); ii++ )
         {
            f_names << rf_names[ ii ];
            f_paths << rpath + "/" + rf_names[ ii ];
         }
      }

      if ( f_paths.size() != mrecs_descrs_recs.size() )
      { // only re-fetch all-models list if we don't yet have it
         QXmlStreamAttributes attr;

         mrecs_descrs_recs.clear();
qDebug() << "editGUID" << editGUID;

         for ( int ii = 0; ii < f_paths.size(); ii++ )
         {
            QString fpath( f_paths[ ii ] );
            QString fname( f_names[ ii ] );
qDebug() << "fpath" << f_paths[ii];
            QFile   m_file( fpath );

            if ( !m_file.open( QIODevice::ReadOnly | QIODevice::Text ) )
               continue;

            QString mrGUID   = ""; 
            QString edGUID   = ""; 
            QString moGUID   = ""; 
            QString descript = ""; 

            QXmlStreamReader xml( &m_file );

            while ( ! xml.atEnd() )
            {  // Search XML elements for description and editGUID
               xml.readNext();

               if ( xml.isStartElement() && xml.name() == "modelrecords" )
               {
                  attr             = xml.attributes();
                  mrGUID           = attr.value( "mrecGUID"    ).toString();
                  edGUID           = attr.value( "editGUID"    ).toString();
                  moGUID           = attr.value( "modelGUID"   ).toString();
                  descript         = attr.value( "description" ).toString();
               }

               else if ( xml.isStartElement() && xml.name() == "modelrecord" )
               {
                  attr             = xml.attributes();
                  mrGUID           = ( ! mrGUID.isEmpty() ) ? mrGUID :
                                     attr.value( "mrecGUID"    ).toString();
                  edGUID           = ( ! edGUID.isEmpty() ) ? edGUID :
                                     attr.value( "editGUID"    ).toString();
                  moGUID           = ( ! moGUID.isEmpty() ) ? moGUID :
                                     attr.value( "modelGUID"   ).toString();
                  descript         = ( ! descript.isEmpty() ) ? descript :
                                     attr.value( "description" ).toString();
               }

               else if ( xml.isEndElement() && xml.name() == "modelrecord" )
                  break;
            }

            m_file.close();

            // Skip save if filtering and edit mismatch
            if ( do_edit  &&  edGUID != editGUID )
               continue;

            if ( descript.isEmpty() )
            {
               descript         = QString( fpath ).section( "/", -2, -2 )
                                + ".triple."
                                + QString( fname ).replace( ".xml", ".mrecs" );
            }
qDebug() << "  edGUID" << edGUID << "descr" << descript;

            MrecsDesc desc;
            desc.DB_id       = "-1";
            desc.mrecsGUID   = mrGUID;
            desc.editGUID    = edGUID;
            desc.modelGUID   = moGUID;
            desc.description = descript;
            desc.filename    = fpath;
            desc.rec_index   = countRD++;

            if ( desc.description.simplified().length() < 2 )
            {
               desc.description = " ( Fname " + QString( fname ).left( 20 )
                                  + tr( " : empty description )" );
            }
//qDebug() << "   desc" << desc.description << "DB_id" << desc.DB_id;

            mrecs_descrs_recs << desc;   // add to full mrecs list
//*DEBUG
//if (!listall) {
//qDebug() << " ddesc" << desc.description;
//qDebug() << "   mpart" << mpart.pattern();
//qDebug() << "   degid" << desc.editGUID;
//qDebug() << "   edgid" << editGUID;
//}
//*DEBUG
         }
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
qDebug() << " (3)m_d_u size" << mrecs_descrs_recs.size();

   if ( listall )
   {  // No filtering or filter by edit already done
      for ( int jj = 0; jj < mrecs_descrs_recs.size(); jj++ )
      {
         mrecs_descriptions << mrecs_descrs_recs[ jj ];
      }
   }

   else
   {  // Filter by model description sub-string
      for ( int jj = 0; jj < mrecs_descrs_recs.size(); jj++ )
      {
         if ( mrecs_descrs_recs[ jj ].description.contains( mpart ) )
         {  // description filter matches
            mrecs_descriptions << mrecs_descrs_recs[ jj ];
//MrecsDesc desc = mrecs_descrs_recs[jj];
//qDebug() << " ddesc" << desc.description << jj;
//qDebug() << "   mpart" << mpart.pattern();
         }
      }
   }
qDebug() << " (4)m_d size" << mrecs_descriptions.size();

   lw_vmrecs->disconnect( SIGNAL( currentRowChanged( int ) ) );
   lw_vmrecs->clear();
   int maxlch   = 0;

   if ( mrecs_descriptions.size() > 0 )
   {
      for ( int ii = 0; ii < mrecs_descriptions.size(); ii++ )
      {  // propagate list widget with descriptions
         lw_vmrecs->addItem( mrecs_descriptions[ ii ].description );
         maxlch    = qMax( maxlch,
                           mrecs_descriptions[ ii ].description.length() );
      }

      // Sort descriptions in ascending alphabetical order
      lw_vmrecs->sortItems();
   }

   else
   {
      lw_vmrecs->addItem( "No models found." );
   }
QDateTime time6=QDateTime::currentDateTime();
qDebug() << "Timing: Time6" << time0.msecsTo(time6) << time2.msecsTo(time6);

   // Resize the widget to show listed items well
   QFontMetrics fm = lw_vmrecs->fontMetrics();
   int olwid    = lw_vmrecs->width();
   int olhgt    = lw_vmrecs->height();
   int nlines   = qMin( mrecs_descriptions.size(), 30 );
   int width    = qMin( 600, maxlch * fm.maxWidth()    );
   int height   = qMin( 800, nlines * fm.lineSpacing() );
   width        = qMax( width,  olwid );
   height       = ( height > olhgt ) ? height : ( ( olhgt + height ) / 2 );
   width        = this->width()  + width  - olwid;
   height       = this->height() + height - olhgt;

   resize( width, height );

   connect( lw_vmrecs,     SIGNAL( currentRowChanged( int )      ),
            this,          SLOT(   row_selected     ( int )      ) );
   connect( le_mfilter,    SIGNAL( textChanged( const QString& ) ),
            this,          SLOT(   msearch(     const QString& ) ) );
}

// Cancel button:  no models returned
void US_MrecsLoader::cancelled()
{
   reject();
   close();
}

// Accept button:  set up to return model information
void US_MrecsLoader::accepted()
{
   int mdx       = 0;

   if ( ( sel_row = lw_vmrecs->currentRow() ) >= 0 )
   {  // Get row of selection then index in original descriptions list
      QString mdesc = lw_vmrecs->currentItem()->text();
      mdx           = mrecsIndex( mdesc, mrecs_descriptions );
qDebug() << "ACC: sel_row mdx" << sel_row << mdx;
   }

   else
   {
      QMessageBox::information( this,
            tr( "No Mrecs Selected" ),
            tr( "You have not selected a model.\nSelect+Accept or Cancel" ) );
      return;
   }

   // Load the mrecs and set the description
qDebug() << "ACC: load... (single)";
   load_mrecs( omrecs, mdx );
qDebug() << "ACC: ...loaded (single)";
   odescr     = concat_description( mdx );


   // Return search string that reflects current state
   dsearch    = le_mfilter->text();

   if ( do_edit )
      dsearch    = "=e " + dsearch;

   dsearch    = dsearch.simplified();

   accept();        // signal that selection was accepted
   close();
}

// Get index in model description list of a model description
int US_MrecsLoader::mrecsIndex( QString mdesc, QList< MrecsDesc > mds )
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

// Show selected-mrecs information in text dialog
void US_MrecsLoader::show_mrecs_info( )
{
   int mtype;

   QString mdesc;
   QString tdesc;
   QString cdesc;
   QString runid;
   QString dtext;
   QString lblid;
   QString mrcid;
   QString anlid;

   int     row    = 0;
   int     mdx    = 0;

   bool    frDisk = ( mrecs_descriptions[ 0 ].filename.length() > 0 );

   if ( frDisk )
   {  // ID is filename
      lblid    = tr( "\n  Mrecs File Name:       " );
   }

   else
   {  // ID is DB id
      lblid    = tr( "\n  Database Mrecs ID:     " );
   }

   if ( ( sel_row = lw_vmrecs->currentRow() ) >= 0 )
   {  // Row selected:  build information for single model records entry

      mdesc    = lw_vmrecs->currentItem()->text();
      mdx      = mrecsIndex( mdesc, mrecs_descriptions );  // find index
qDebug() << "  sel_row" << sel_row << "mdx" << mdx;

      load_mrecs( mrecs, mdx );                            // load mrecs

      mrec     = mrecs[ 0 ];
      mtype    = mrec.ctype;                               // mrecs info
      tdesc    = US_ModelRecord::ctype_text( mtype );
      runid    = mdesc.section( ".", 0, -4 );
      mrcid    = frDisk ?
         mrecs_descriptions[ mdx ].filename :              // ID is filename
         mrecs_descriptions[ mdx ].DB_id;                  // ID is DB id
      mrcid    = mrcid.length() < 50 ? mrcid :
         "*/" + mrcid.section( "/", -3, -1 );              // short filename

      dtext    = tr( "Mrecs Information:" )
         + tr( "\n  Description:       " ) + mdesc
         + tr( "\n  Implied RunID:     " ) + runid
         + tr( "\n  Mrecs ID:          " ) + mrcid
         + tr( "\n  Mrecs Global ID:   " ) + mrec.mrecGUID
         + tr( "\n  Type:              " ) + tdesc
         +     "  (" + QString::number( (int)mtype ) + ")"
         + tr( "\n  Model Global ID:   " ) + mrec.modelGUID
         + tr( "\n  Edit Global ID:    " ) + mrec.editGUID
         + tr( "\n  List Row:          " ) + QString::number( row + 1 )
         + tr( "\n  Mrecs xmin:        " ) + QString::number( mrec.xmin )
         + tr( "\n  Mrecs xmax:        " ) + QString::number( mrec.xmax )
         + tr( "\n  Mrecs ymin:        " ) + QString::number( mrec.ymin )
         + tr( "\n  Mrecs ymax:        " ) + QString::number( mrec.ymax )
         + tr( "\n  Mrecs count:       " ) + QString::number( mrecs.count() )
         + tr( "\n  Best Model Type:   " )
         +     US_ModelRecord::ctype_text( mrec.ctype )
         + tr( "\n  Best Model taskx:  " ) + QString::number( mrec.taskx )
         + tr( "\n  Best Model str_y:  " ) + QString::number( mrec.str_y )
         + tr( "\n  Best Model end_y:  " ) + QString::number( mrec.end_y )
         + tr( "\n  Best Model par1:   " ) + QString::number( mrec.par1 )
         + tr( "\n  Best Model par2:   " ) + QString::number( mrec.par2 )
         + tr( "\n  Best Model RMSD:   " ) + QString::number( mrec.rmsd )
         + tr( "\n  BM C_Solutes size: " )
         +     QString::number( mrec.csolutes.size() )
         + tr( "\n  BM I_Solutes size: " )
         +     QString::number( mrec.isolutes.size() )
         + "";
   }

   // open a dialog and display model information
   US_Editor* edit = new US_Editor( US_Editor::LOAD, true, "", this );
   edit->setWindowTitle( tr( "Mrecs Entry Information" ) );
   edit->move( this->pos() + QPoint( 200, 200 ) );
   edit->resize( 800, 460 );
   edit->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   edit->e->setText( dtext );
   edit->show();
}

// Slot to re-list models when search text has changed
void US_MrecsLoader::msearch( const QString& search_string )
{
   dsearch  = search_string;

   if ( search_string.endsWith( "=" ) )
      return;

   list_vmrecs();
}

// Slot to re-list models after change in Edit checkbox
void US_MrecsLoader::change_edit( bool ckedit )
{
   do_edit   = ckedit;
   db_id1    = -2;  // flag re-list when list-edit flag changes
   db_id2    = -2;

   list_vmrecs();
}

// Slot to enable buttons when a row is selected and save that selection
void US_MrecsLoader::row_selected( int row )
{
   sel_row   = row;

   pb_details->setEnabled( true );
   pb_delete ->setEnabled( true );
   pb_accept ->setEnabled( true );
}

// Slot to delete an mrecs entry
void US_MrecsLoader::delete_mrecs()
{
   if ( ( sel_row = lw_vmrecs->currentRow() ) < 0 )
      return;

   QString mdesc    = lw_vmrecs->currentItem()->text();
   int  mdx         = mrecsIndex( mdesc, mrecs_descriptions );

   if ( loadDB )
   {  // Delete an entry from the database
      if ( dbP == NULL )
      {
         US_Passwd pw;
         dbP         = new US_DB2( pw.getPasswd() );

         if ( dbP->lastErrno() != IUS_DB2::OK )
         {
            QMessageBox::critical( this,
                  tr( "DB Connection Problem" ),
                  tr( "There was an error connecting to the database:\n" )
                  + dbP->lastError() );
            return;
         }
      }

      QString mrecsID  = mrecs_descriptions[ mdx ].DB_id;
      QStringList qry;
      qry.clear();
      qry << "delete_mrecs" << mrecsID;

      if ( dbP->statusQuery( qry ) == 0 )
      {
         QMessageBox::information( this,
            tr( "Successful DB Delete" ),
            tr( "The mrecs record '%1' has been deleted from the database." )
            .arg( mdesc ) );

         list_vmrecs();
      }

      else
      {
         QMessageBox::critical( this,
            tr( "DB Delete Problem" ),
            tr( "There was an error deleting an entry from the database:\n" )
            + dbP->lastError() );
      }
   }

   else
   {  // Delete a local file
      QString fpath    = mrecs_descriptions[ mdx ].filename;
      QFile filemr( fpath );
      if ( filemr.exists() )
      {
         if ( filemr.remove() )
         {
            QMessageBox::information( this,
               tr( "Successful Local File Delete" ),
               tr( "The mrecs file '%1' has been deleted." )
               .arg( fpath ) );

            list_vmrecs();
         }

         else
         {
            QMessageBox::critical( this,
               tr( "Error in Local File Delete" ),
               tr( "The mrecs file '%1' was not deleted"
                   " due to a remove error." ).arg( fpath ) );
         }
      }

      else
      {
         QMessageBox::critical( this,
            tr( "Error in Local File Delete" ),
            tr( "The mrecs file '%1' was not deleted."
                " It did not exist." ).arg( fpath ) );
      }
   }
}

