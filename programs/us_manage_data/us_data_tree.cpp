//! \file us_data_tree.cpp

#include "us_data_tree.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_editor.h"
#include "us_noise.h"
#include "us_util.h"

const QColor colorRed(   240,   0,   0 );
const QColor colorBlue(    0,   0, 255 );
const QColor colorBrown( 120,  60,   0 );
const QColor colorGreen(   0, 150,   0 );
const QColor colorGray(  110, 110, 110 );
const QColor colorWhite( 255, 255, 240 );

// class to manage a tree widget, underlying data, and processing
US_DataTree::US_DataTree( US_DataModel* dmodel, QTreeWidget* treewidg,
      QWidget* parent /*=0*/ )
{
   da_model   = dmodel;
   tw_recs    = treewidg;
   parentw    = (QWidget*)parent;
   dbg_level  = US_Settings::us_debug();

   QStringList theads;

   theads << "Type" << "Label" << "SubType" << "Source"
          << "Children\nDB, Local" << "Descendants\nDB, Local";
   ntrows = 5;
   ntcols = theads.size();
   tw_recs->setHeaderLabels( theads );
   tw_recs->setFont(  QFont( US_Widgets::fixedFont().family(),
                      US_GuiSettings::fontSize() - 1 ) );
   tw_recs->setObjectName( QString( "tree-widget" ) );
   tw_recs->setAutoFillBackground( true );
}

// show or hide (expand/collapse) all rows of specified type
void US_DataTree::toggle_expand( QString c1str, bool show )
{
   QList< QTreeWidgetItem* > listi = tw_recs->findItems(
      c1str, Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive, 0 );

DbgLv(1) << "DT: togl_exp str show" << c1str << show;
   for ( int ii = 0; ii < listi.size(); ii++ )
      listi.at( ii )->setExpanded( show );
}

// build the data tree from descriptions read
void US_DataTree::build_dtree( )
{
   QString rtyp;
   QString subt;
   QString labl;
   QString dguid;
   QString rsrc;
   QString anch;
   QString ande;
   int     ityp  = 1;
   int     nchdb = 0;
   int     nchlo = 0;
   int     ndedb = 0;
   int     ndelo = 0;
   QBrush  fbru( colorBrown );
   QBrush  bbru( colorWhite );
   QTreeWidgetItem* pitems[4];
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );
   US_DB2*   dbP = &db;

   da_process = (US_DataProcess*)da_model->procobj();

   ncrecs     = da_model->recCount();
   ndrecs     = da_model->recCountDB();
   nlrecs     = da_model->recCountLoc();
   ntrows     = ncrecs;
   ncraws     = ncedts = ncmods = ncnois = 0;
   ndraws     = ndedts = ndmods = ndnois = 0;
   nlraws     = nledts = nlmods = nlnois = 0;
   tw_recs->clear();
   QProgressBar* progress = da_model->progrBar();
   progress->setMaximum( ncrecs );
   progress->setValue  ( 0 );

   for ( int ii = 0; ii < ncrecs; ii++ )
   {
      cdesc      = da_model->row_datadesc( ii );
      ityp       = cdesc.recType;
      QStringList cvals;
      bool isDba = ( ( cdesc.recState & US_DataModel::REC_DB ) != 0 );
      bool isLoc = ( ( cdesc.recState & US_DataModel::REC_LO ) != 0 );

      if ( cdesc.recordID < 0  &&  isLoc )
      { // local only
         fbru    = QBrush( colorBrown );
         rsrc    = "Local";

         if      ( ityp == 1 )
         {
            nlraws++; ncraws++;
            rtyp    = "Raw";
         }
         else if ( ityp == 2 )
         {
            nledts++; ncedts++;
            rtyp    = "Edited";
         }
         else if ( ityp == 3 )
         {
            nlmods++; ncmods++;
            rtyp    = "Model";
         }
         else if ( ityp == 4 )
         {
            nlnois++; ncnois++;
            rtyp    = "Noise";
         }
         else
         {
            rtyp    = "none";
            ityp    = 0;
         }
      }

      else if ( isDba  &&  !isLoc )
      { // database only
         fbru    = QBrush( colorBlue );
         rsrc    = "DB";

         if      ( ityp == 1 )
         {
            ndraws++; ncraws++;
            rtyp    = "Raw";
         }
         else if ( ityp == 2 )
         {
            ndedts++; ncedts++;
            rtyp    = "Edited";
         }
         else if ( ityp == 3 )
         {
            ndmods++; ncmods++;
            rtyp    = "Model";
         }
         else if ( ityp == 4 )
         {
            ndnois++; ncnois++;
            rtyp    = "Noise";
         }
         else
         {
            rtyp    = "none";
            ityp    = 0;
         }
      }

      else
      { // both local and database
         fbru    = QBrush( colorGreen );
         rsrc    = "In Sync";

         if ( ! cdesc.contents.isEmpty() )
         {
            QString cont1 = cdesc.contents.section( " ", 0, 1 ).simplified();
            QString cont2 = cdesc.contents.section( " ", 2, 4 ).simplified();
            if ( cont1 != cont2  &&  significant_diffs( cdesc, dbP ) )
            {
               fbru    = QBrush( colorRed );
               rsrc    = "Conflict";
DbgLv(1) << "CONFLICT cont1 cont2" << cont1 << cont2;
            }
         }

         if      ( ityp == 1 )
         {
            nlraws++; ndraws++; ncraws++;
            rtyp    = "Raw";
         }
         else if ( ityp == 2 )
         {
            nledts++; ndedts++; ncedts++;
            rtyp    = "Edited";
         }
         else if ( ityp == 3 )
         {
            nlmods++; ndmods++; ncmods++;
            rtyp    = "Model";
         }
         else if ( ityp == 4 )
         {
            nlnois++; ndnois++; ncnois++;
            rtyp    = "Noise";
         }
         else
         {
            rtyp    = "none";
            ityp    = 0;
            rsrc    = "Conflict";
         }
      }

      labl       = cdesc.label;
      dguid      = cdesc.dataGUID;
      subt       = cdesc.subType;
      nchdb      = nchlo = ndedb = ndelo = 0;

      if ( cdesc.recState == US_DataModel::NOSTAT )
      {  // mark artificial record with color and source text
         fbru       = QBrush( colorGray );
         rsrc       = "dummy";
      }

      for ( int jj = ( ii + 1 ); jj < ncrecs; jj++ )
      {  // count children and descendants from next-row until back-to-level
         US_DataModel::DataDesc ddesc = da_model->row_datadesc( jj );

         if ( ddesc.recType <= ityp )
            break;   // once we've reached or passed same level, break

         // from database?
         bool isDba = ( ( ddesc.recState & US_DataModel::REC_DB ) != 0 );
         // from local?
         bool isLoc = ( ( ddesc.recState & US_DataModel::REC_LO ) != 0 );
         // direct child?
         //bool child = ( ddesc.parentGUID == dguid );
         bool child = ( ( ddesc.recType - ityp ) == 1 );

         ndedb   += ( isDba          ) ? 1 : 0;  // bump db descendant count
         nchdb   += ( isDba && child ) ? 1 : 0;  // bump db child count
         ndelo   += ( isLoc          ) ? 1 : 0;  // bump local descendant count
         nchlo   += ( isLoc && child ) ? 1 : 0;  // bump local child count
      }

      anch = ( ityp < 4 ) ?
         QString( "%1, %2" ).arg( nchdb ).arg( nchlo ) : "";  // children
      ande = ( ityp < 3 ) ?
         QString( "%1, %2" ).arg( ndedb ).arg( ndelo ) : "";  // descendants

      QTreeWidgetItem* item;
      int wiutype = (int)QTreeWidgetItem::UserType + ii; // type: encoded index

      cvals << rtyp << labl << subt << rsrc << anch << ande;

      if ( ityp == 1 )
      {  // Raws are children of the root
         item = new QTreeWidgetItem( tw_recs,            cvals, wiutype );
      }

      else
      {  // others are children of the next level up
         item = new QTreeWidgetItem( pitems[ ityp - 2 ], cvals, wiutype );
      }

      pitems[ ityp - 1 ] = item;  // save next parent of this type

      for ( int jj = 0; jj < ntcols; jj++ )
      {
         item->setForeground( jj, fbru );
         item->setBackground( jj, bbru );
      }

      progress->setValue( ii + 1 );
      qApp->processEvents();
   }

   // resize so all of columns are shown
   tw_recs->expandAll();                      // expand the entire tree

   for ( int jj = 0; jj < ntcols; jj++ )
   {
      tw_recs->resizeColumnToContents( jj );  // resize to fit contents
   }

   tw_recs->collapseAll();                    // collapse the entire tree
}

// set up and display a row context menu
void US_DataTree::row_context_menu( QTreeWidgetItem* item )
{
   selitems = tw_recs->selectedItems();
   int nsel = selitems.size();
DbgLv(1) << "    context_menu nbr sel rows" << selitems.size();
   tw_item  = item;
   int irow = item->type() - (int)QTreeWidgetItem::UserType;

   if ( nsel > 1 )
   {  // If multiple selections, set first row
      for ( int ii = 0; ii < nsel; ii++ )
      {
         QTreeWidgetItem* jitem = selitems[ ii ];
         int jrow = jitem->type() - (int)QTreeWidgetItem::UserType;
         irow     = qMin( irow, jrow );
      }
   }

DbgLv(2) << "    context_menu row" << irow+1;
   da_model->setCurrent( irow );
DbgLv(2) << "    context_menu RTN setCurrent";
   cdesc    = da_model->current_datadesc();
DbgLv(2) << "    context_menu RTN current_datadesc";

   QString tupload = tr( " upload record to DB" );
   QString tdnload = tr( " download record to local" );
   QString tdbasrm = tr( " remove branch from DB" );
   QString tloclrm = tr( " remove branch from local" );
   QString tbothrm = tr( " remove branch from both" );
   QString tshdeta = tr( " show record details" );

   if ( nsel > 1 )
   {  // Multiple selections:  "record/branch" to "branches"
      tupload.replace( tr( "record" ), tr( "branches" ) );
      tdnload.replace( tr( "record" ), tr( "branches" ) );
      tdbasrm.replace( tr( "branch" ), tr( "branches" ) );
      tloclrm.replace( tr( "branch" ), tr( "branches" ) );
      tbothrm.replace( tr( "branch" ), tr( "branches" ) );
      tshdeta.replace( tr( "record" ), tr( "records"  ) );
   }

   if ( cdesc.recType == 1 )
   {  // Raw:  "record/branches" to "descendants"
      tupload.replace( tr( "record"   ), tr( "descendants" ) );
      tupload.replace( tr( "branches" ), tr( "descendants" ) );
      tdnload.replace( tr( "record"   ), tr( "descendants" ) );
      tdnload.replace( tr( "branches" ), tr( "descendants" ) );
      tdbasrm.replace( tr( "branches" ), tr( "descendants" ) );
      tdbasrm.replace( tr( "branch"   ), tr( "descendants" ) );
      tloclrm.replace( tr( "branches" ), tr( "descendants" ) );
      tloclrm.replace( tr( "branch"   ), tr( "descendants" ) );
      tbothrm.replace( tr( "branches" ), tr( "descendants" ) );
      tbothrm.replace( tr( "branch"   ), tr( "descendants" ) );
   }

   QMenu*   cmenu   = new QMenu();
   QAction* aupload = new QAction( tupload, parentw );
   QAction* adnload = new QAction( tdnload, parentw );
   QAction* adbasrm = new QAction( tdbasrm, parentw );
   QAction* aloclrm = new QAction( tloclrm, parentw );
   QAction* abothrm = new QAction( tbothrm, parentw );
   QAction* ashdeta = new QAction( tshdeta, parentw );

   cmenu->addAction( aupload );
   cmenu->addAction( adnload );
   cmenu->addAction( adbasrm );
   cmenu->addAction( aloclrm );
   cmenu->addAction( abothrm );
   cmenu->addAction( ashdeta );
DbgLv(2) << "    context_menu RTN addAction";

   connect( aupload, SIGNAL( triggered() ),
            this,    SLOT(   item_upload()     ) );
   connect( adnload, SIGNAL( triggered() ),
            this,    SLOT(   item_download()   ) );
   connect( adbasrm, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_db()  ) );
   connect( aloclrm, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_loc() ) );
   connect( abothrm, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_all() ) );
   connect( ashdeta, SIGNAL( triggered() ),
            this,    SLOT(   item_details()    ) );

   // disable menu items that are not appropriate to the record

   if ( ( cdesc.recState & US_DataModel::REC_LO ) == 0 )
   {  // if record not local, no upload and no remove from local or both
      aupload->setEnabled( false );
      aloclrm->setEnabled( false );
      abothrm->setEnabled( false );
   }

   if ( ( cdesc.recState & US_DataModel::REC_DB ) == 0 )
   {  // if record not db, no download and no remove from db or both
      adnload->setEnabled( false );
      adbasrm->setEnabled( false );
      abothrm->setEnabled( false );
   }

   // Disable upload if parent not in the DB
   int jrow = irow;
   US_DataModel::DataDesc jdesc = da_model->row_datadesc( jrow );

   while ( --jrow > 0  &&  cdesc.recType > 1 )
   {
      jdesc = da_model->row_datadesc( jrow );
      if ( jdesc.recType < cdesc.recType )
         break;
   }

   if ( ( jdesc.recState & US_DataModel::REC_DB ) == 0 )
      aupload->setEnabled( false );

   // display the context menu and act on selection
DbgLv(2) << "    context_menu CALL cmenu exec";
   cmenu->exec( QCursor::pos() );

}

// open a dialog and display data tree help
void US_DataTree::dtree_help()
{
   QString fileexts = tr( "Text,Log files (*.txt *.log);;" )
      + tr( "All files (*)" );
   QString mtext =
      tr( "Data Tree Columns --\n\n" ) +
      tr( " \"Type\"       :  Type of data set record -\n" ) +
      tr( "                  \"Raw\", \"Edited\", \"Model\" or \"Noise\".\n") +
      tr( " \"Label\"      :  Short description of specific record.\n" ) +
      tr( " \"SubType\"    :  Record-specific type (e.g. \"2DSA\", \"TI\").\n")+
      tr( " \"Source\"     :  Location/state (see color legend below)-\n" ) +
      tr( "                  \"DB\"        (Blue,  database); \n" ) +
      tr( "                  \"Local\"     (Brown, local disk); \n" ) +
      tr( "                  \"In Sync\"   (Green, both, consistent); \n" ) +
      tr( "                  \"Conflict\"  (Red,   both, inconsistent); \n" ) +
      tr( "                  \"dummy\"     (Gray,  missing parent filler).\n") +
      tr( " \"Children    :  Number of Children of this row\n" ) +
      tr( "  DB, Local\"       from Database, Local-disk.\n" ) +
      tr( " \"Descendants :  Number of Descendants of this row\n" ) +
      tr( "  DB, Local\"       from Database, Local-disk.\n\n" ) +
      tr( "Tree Row Color Legend --\n\n" ) +
      tr( "  Blue   :  This record exists for database only.\n" ) +
      tr( "  Brown  :  This record exists for local disk only.\n" ) +
      tr( "  Green  :  This record exists and is consistent\n" ) +
      tr( "              for both database and local.\n" ) +
      tr( "  Red    :  This record exists for both, but is inconsistent.\n")+
      tr( "  Gray   :  This record is a filler for a missing parent.\n\n" ) +
      tr( "Tree Entry Processes --\n\n" ) +
      tr( "  A right-mouse-button click on any cell of a row pops up\n" ) +
      tr( "  a context menu with actions to take for the record.\n" ) +
      tr( "  Menu items will only be enabled if the action is\n" ) +
      tr( "  appropriate for the particular record shown in the row.\n" ) +
      tr( "  Possible actions are:\n" ) +
      tr( "    \"upload to DB\"\n" ) +
      tr( "       -> Upload this local record to the database;\n" ) +
      tr( "    \"download to local\"\n" ) +
      tr( "       -> Download this DB record to local disk.\n" ) +
      tr( "    \"remove from DB\"\n" ) +
      tr( "       -> Remove this record from the database.\n" ) +
      tr( "    \"remove from local\"\n" ) +
      tr( "       > Remove this record from local disk.\n" ) +
      tr( "    \"remove both\"\n" ) +
      tr( "       -> Remove this record from both DB and local.\n" ) +
      tr( "    \"show details\"\n" ) +
      tr( "       -> Pop up a text dialog with details about the record.\n" ) +
      tr( "Detailed Help --\n\n" ) +
      tr( "  Click on the \"Help\" button for standard documentation,\n" ) +
      tr( "  including sample images and details on all GUI elements.\n" );

   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Data Sets Tree Help and Legend" ) );
   editd->move( QCursor::pos() + QPoint( 200, 200 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                      US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// perform item upload action
void US_DataTree::item_upload()
{
   QMessageBox msgBox( parentw );
   QString     item_exs = tr( "Local disk only" );
   QString     item_act = tr( "DB create" );
DbgLv(2) << "ITEM Upload";
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   if ( ( cdesc.recState & US_DataModel::REC_DB ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "DB replace" );
   }

   record_type( cdesc.recType, item_act );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
DbgLv(2) << " ITEM ACTION: YES";
      int stat1  = do_actions( item_exs, item_act );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }

}

// perform item download action
void US_DataTree::item_download()
{
   QMessageBox msgBox( parentw );
   QString     item_exs = tr( "Database only" );
   QString     item_act = tr( "Local file create" );
DbgLv(2) << "ITEM Download";
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   if ( ( cdesc.recState & US_DataModel::REC_LO ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "Local file replace" );
   }

   record_type( cdesc.recType, item_act );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
DbgLv(2) << " ITEM ACTION: YES";
      int stat1  = do_actions( item_exs, item_act );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-db action
void US_DataTree::item_remove_db()
{
DbgLv(2) << "ITEM Remove from DB";
   QString     item_exs = tr( "Database only" );
   QString     item_act = tr( "DB-only remove" );
   QMessageBox msgBox( parentw );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   record_type( cdesc.recType, item_act );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
DbgLv(2) << " ITEM ACTION: YES";
      int stat1  = do_actions( item_exs, item_act );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-local action
void US_DataTree::item_remove_loc()
{
DbgLv(2) << "ITEM Remove from Local";
   QString     item_exs = tr( "Local disk only" );
   QString     item_act = tr( "Local-only remove" );
   QMessageBox msgBox( parentw );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   record_type( cdesc.recType, item_act );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
DbgLv(2) << " ITEM ACTION: YES";
      int stat1  = do_actions( item_exs, item_act );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-all action
void US_DataTree::item_remove_all()
{
DbgLv(2) << "ITEM Remove Both DB and Local";
   QString     item_exs = tr( "both DB and Local" );
   QString     item_act = tr( "DB+Local remove" );
   QMessageBox msgBox( parentw );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   record_type( cdesc.recType, item_act );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
DbgLv(2) << " ITEM ACTION: YES";
      int stat1  = do_actions( item_exs, item_act );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// Perform item details action
void US_DataTree::item_details(  )
{
   if ( selitems.size() > 1 )
   {
      items_details();
      return;
   }

   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   QString     fileexts = tr( "Text,Log files (*.txt *.log)"
                              ";;All files (*)" );
   int         irow     = tw_item->type() - (int)QTreeWidgetItem::UserType;
DbgLv(2) << "DT: i_details row" << irow;

   da_model->setCurrent( irow++ );   // get description record, index as 1...
   cdesc    = da_model->current_datadesc();

   QString mtext =
      tr( "Data Tree Item at Row %1 -- \n\n" ).arg( irow ) +
      tr( "  Type           : %1 (%2)\n" )
         .arg( cdesc.recType ).arg( rtyps[ cdesc.recType - 1 ] ) +
      tr( "  SubType        : " ) + cdesc.subType + "\n" +
      tr( "  Label          : " ) + cdesc.label + "\n" +
      tr( "  Description    : " ) + cdesc.description + "\n" +
      tr( "  DB record ID   : %1\n" ).arg( cdesc.recordID ) +
      tr( "  Global ID      : " ) + cdesc.dataGUID + "\n" +
      tr( "  Parent GUID    : " ) + cdesc.parentGUID + "\n" +
      tr( "  DB parent ID   : %1\n" ).arg( cdesc.parentID ) +
      tr( "  File Directory : " )
         + cdesc.filename.section( "/",  0, -2 ) + "\n" +
      tr( "  File Name      : " )
         + cdesc.filename.section( "/", -1, -1 ) + "\n" +
      tr( "  File Last Mod  : " ) + cdesc.filemodDate + "\n" +
      tr( "  DB   Last Mod  : " ) + cdesc.lastmodDate + "\n" +
      tr( "  Record State   : " ) + record_state( cdesc.recState ) + "\n";

   if ( cdesc.contents.length() < 60 )
   {
      mtext = mtext + 
         tr( "  Content Checks : " ) + cdesc.contents;
   }
   else
   {
      QString cont1 = cdesc.contents.section( " ", 0, 1 ).simplified();
      QString cont2 = cdesc.contents.section( " ", 2, 4 ).simplified();
      mtext = mtext + 
         tr( "  Content Checks : " ) + cont1 + "\n" +
             "                   "   + cont2 + "\n";
#if 0
if (cdesc.recType==3 && cont1!=cont2) {
US_Passwd pw;
US_DB2* dbP = new US_DB2( pw.getPasswd() );
US_Model model1;
US_Model model2;
QString tmpdir=US_Settings::tmpDir();
model1.load( QString::number(cdesc.recordID), dbP );
model2.load( cdesc.filename );
QString fname1=tmpdir+"/model1d.xml";
QString fname2=tmpdir+"/model2f.xml";
model1.write(fname1);
model2.write(fname2);
QString fname3=tmpdir+"/model3d.xml";
QString fname4=tmpdir+"/model4f.xml";
model1.update_coefficients();
model2.update_coefficients();
model1.write(fname3);
model2.write(fname4);
}
#endif
   }

   // display the text dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Data Tree Entry Details" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 720, 360 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                      US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// Perform multiple-items details action
void US_DataTree::items_details(  )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   QString     fileexts = tr( "Text,Log files (*.txt *.log)"
                              ";;All files (*)" );

   int narows = action_rows();

   int nsrows = selrows.size();
   int nrrows = rawrows.size();
   int irow   = selrows[ 0 ];
   int krow   = selrows[ nsrows - 1 ];
   krow       = ( narows > 0 ) ? actrows[ narows - 1 ] : krow;
DbgLv(2) << "DT: i_details row" << irow;
   cdesc    = da_model->row_datadesc( irow );
   US_DataModel::DataDesc kdesc    = da_model->row_datadesc( krow );

   QString mtext =
      tr( "Data Tree Items from Rows %1 to %2 -- \n\n" )
         .arg( irow ).arg( krow ) +
      tr( "  Type   (first) : %1\n" ).arg( rtyps[ cdesc.recType - 1 ] ) +
      tr( "    Description    : " ) + cdesc.description + "\n" +
      tr( "    DB record ID   : %1\n" ).arg( cdesc.recordID ) +
      tr( "    File Directory : " )
         + cdesc.filename.section( "/",  0, -2 ) + "\n" +
      tr( "    File Name      : " )
         + cdesc.filename.section( "/", -1, -1 ) + "\n" +
      tr( "  Type    (last) : %1\n" ).arg( rtyps[ kdesc.recType - 1 ] ) +
      tr( "    Description    : " ) + kdesc.description + "\n" +
      tr( "    DB record ID   : %1\n" ).arg( kdesc.recordID ) +
      tr( "    File Directory : " )
         + kdesc.filename.section( "/",  0, -2 ) + "\n" +
      tr( "    File Name      : " )
         + kdesc.filename.section( "/", -1, -1 ) + "\n" +
      tr( "  Total Selected Records : " ) + QString::number( nsrows ) + "\n" +
      tr( "  Total Action Records   : " ) + QString::number( narows ) + "\n" +
      tr( "  Total Raw Selections   : " ) + QString::number( nrrows ) + "\n";

   // display the text dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Data Tree Entries Details" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 760, 320 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                      US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// Prepend a record type string to an item action
void US_DataTree::record_type( int recType, QString& item_act )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   int sizert = sizeof( rtyps ) / sizeof( rtyps[ 0 ] );

   if ( recType > 0  &&  recType <= sizert )
   {
      item_act = QString( rtyps[ recType - 1 ] ) + " " + item_act;
   }
}

// Format an item action text for a message box
QString US_DataTree::action_text( QString exstext, QString acttext )
{
   QString lines = ( selitems.size() == 1 )
                 ?  tr( "This item exists on %1.<br>"
                        "Are you sure you want to proceed with a %2?<ul>" )
                    .arg( exstext ).arg( acttext )
                 :  tr( "These items exists on %1.<br>"
                        "Are you sure you want to proceed with %2s?<ul>" )
                    .arg( exstext ).arg( acttext );
   return lines +
           tr( "<li><b>No </b> to cancel the action;</li>"
               "<li><b>Yes</b> to proceed with the action.</li></ul>" );
}

// report the result of an item action
void US_DataTree::action_result( int stat, QString item_act )
{
   QLabel* lb_status = da_model->statlab();

   if ( stat != 999 )
   {  // proceed was selected:  test status of action

      if ( stat == 0 )
      {  // action was successful
         lb_status->setText( tr( "\"%1\" Success!" ).arg( item_act ) );
      }

      else
      {  // action got an error
         QMessageBox::warning( parentw,
               item_act + tr( " *ERROR*!" ),
               tr( "The \"%1\" action had an error: %2" )
               .arg( item_act ).arg( stat )
               + "\n\n" + da_process->lastError() );
         lb_status->setText( tr( "\"%1\" ERROR!" ).arg( item_act ) );
      }
   }

   else
   {  // cancel was selected:  report it
      lb_status->setText( tr( "\"%1\" Cancelled!" ).arg( item_act ) );
   }
}

// compose string describing record state
QString US_DataTree::record_state( int istate )
{
   QString hexn    = QString().sprintf( "0x%3.3x", istate );

   QString flags   = "NOSTAT";         // by default, no state

   if ( istate & US_DataModel::REC_DB )
      flags  = flags + "|REC_DB";      // record exists in db

   if ( istate & US_DataModel::REC_LO )
      flags  = flags + "|REC_LO";      // record exists locally

   if ( istate & US_DataModel::PAR_DB )
      flags  = flags + "|PAR_DB";      // parent exists in db

   if ( istate & US_DataModel::PAR_LO )
      flags  = flags + "|PAR_LO";      // parent exists locally

   if ( istate & US_DataModel::HV_DET )
      flags  = flags + "|DETAIL";      // content details are supplied

   if ( istate & US_DataModel::IS_CON )
      flags  = flags + "|CONSIS";      // record is consistent in db+local

   if ( istate & US_DataModel::ALL_OK )
      flags  = flags + "|ALL_OK";      // record is ok in all respects

   if ( flags != "NOSTAT" )
      flags  = flags.mid( 7, 999 );    // remove any "NOSTAT|"

    return "(" + hexn + ") " + flags;  // return hex flag and text version
}


//Check for model
bool US_DataTree::is_modelIDs_from_autoflow( QString mID )
{
  QStringList modelInfos_autoflow;
  QStringList modelIDs_autoflow;
  
  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( parentw, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db.lastError() );
      return false;
    }
  
  QStringList q;
  q << "get_modelIDs_for_autoflow";
  db.query( q );
  
  while ( db.next() )
    modelInfos_autoflow << db. value( 0 ).toString();

  return parse_models_desc_json ( modelInfos_autoflow, mID ); 
}

bool US_DataTree::parse_models_desc_json( QStringList modelInfos_autoflow, QString mID )
{
  qDebug() << "Size of modelInfos_autoflow: " << modelInfos_autoflow.size();
  qDebug() << "mID: " << mID;
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


//Check if the run [filename] a part of the GMP framework
bool US_DataTree::check_filename_for_autoflow( QString rFilename )
{
  bool isRequired = false;
  int  autoflowNumber = 0;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );

  QStringList q;
  q  << QString( "check_filename_for_autoflow" ) //for now, both GMP and R&D
     << rFilename;

  autoflowNumber  = db -> functionQuery( q );

  (autoflowNumber > 0) ? isRequired = true : isRequired = false;

  qDebug() << "in check_filename_for_autoflow(): autoflowNumber, isRequired -- "
	   << autoflowNumber << isRequired;
  
  return isRequired;
}

// Perform the action(s) chosen in context menu
int US_DataTree::do_actions( QString item_exs, QString item_act )
{
   int narows  = action_rows();
DbgLv(1) << "ITEM do_actions" << narows << item_exs << item_act;
 qDebug() <<  "ITEM do_actions" << narows << item_exs << item_act; 
   int naerrs  = 0;
   int istat   = 0;
   bool frDB   = item_exs.contains( "Datab" )
              || item_exs.contains( "DB "   );
   bool frLoc  = item_exs.contains( "Local" );
   bool frBoth = item_exs.contains( "both"  );
   frDB        = frBoth ? true : frDB;
   frLoc       = frBoth ? true : frLoc;
   bool dnLoad = item_act.contains( "Local file " );
   bool upLoad = item_act.contains( "DB create"   )
              || item_act.contains( "DB replace"  );
   bool remove = item_act.contains( "remove"      );
   QProgressBar* progress = da_model->progrBar();
   QLabel*       stlabel  = da_model->statlab();

   if ( remove && frDB )
   {                                  // REMOVE from DB
      int  lrtyp  = 9;   // last remove type
      int  karows = 0;

      for ( int jj = 0; jj < narows; jj++ )
      {
         int irow   = actrows[ jj ];
         US_DataModel::DataDesc ddesc = da_model->row_datadesc( irow );
         int ityp   = ddesc.recType;
	 DbgLv(1) << "RMV_REC: irow ityp lrtyp" << irow << ityp << lrtyp;

	 //Debug && Test ////////////////////////
	 qDebug() << "DataModel: recType, recordID, parentID, description: "
		  << ddesc.recType  // recType={1,2,3,4} => {Raw, EditedData, Model, Noise}
		  << ddesc.recordID // to delete EditedData, Model, OR Noise ( recType==2,3,4)
		  << ddesc.parentID // to delete Raw Data                    ( recType==1)
		  << ddesc.description
	   ;
	 
	 //Now, by the recType, determine the ID of the rawData (top-level parentID)
	 //Obvious for types {1,2}=> {Raw, EditedData}
	 QString rFilename = ddesc.description.split(".")[0].simplified();
	 qDebug() << "Filename of the run: " << rFilename;

	 if ( ddesc.recType == 1 || ddesc.recType == 2 || ddesc.recType == 3 || ddesc.recType == 4) // all types
	   {
	     bool isRequired = check_filename_for_autoflow( rFilename );
	     if ( isRequired )
	       {
		 QMessageBox::information( parentw, tr( "Data Cannot be Deleted" ),
					   tr( "The Data for the Run:\n\n"
					       "\"%1\"\n\n"
					       "can NOT be deleted since it is required by the GMP framework!" )
					   .arg( rFilename ) );
       
		 return istat;
	       }
	   }

	 /*****
	       NOTE: if we do not want to delete *any* model/noise associated but NOT actually required by GMP run,
	       we can stop here....
	       Below, is a finer model/noise treatment based on GMP run to require ONLY {2DSA-IT, 2DSA-MC & PSCA} models...
	  ****/
	 
	 if ( ddesc.recType == 3 || ddesc.recType == 4 ) //model OR noise
	   {
	     int model_idDB;
	     if ( ddesc.recType == 3 ) 
	       model_idDB = ddesc.recordID;
	     else if ( ddesc.recType == 4 )
	       model_idDB = ddesc.parentID;
	     if ( is_modelIDs_from_autoflow( QString::number( model_idDB ) ) )
	       {
		 qDebug() << "Select:: Model GMP/autolfow-generated!";
		 
		 QMessageBox::information( parentw, tr( "Selected Model GMP produced!" ),
					   tr( "Selected Model / Noise "
					       "was generated within GMP framework!\n\n"
					       "It can NOT be deleted...")
					   );
		 return istat;
	       }
	   }
	 
	 ////////////////////////////////
 

	 /** TEMP   ***/
         if ( ityp > lrtyp )
         {  // Just mark as deleted if descendant of last removed
            ddesc.recordID = -1;
            da_model->change_datadesc( ddesc, irow );
            continue;
         }

         lrtyp      = ityp;              // save type of last removed
         karows++;
         int stat1  = da_process->record_remove_db( irow );
DbgLv(1) << "RMV_REC:   karows stat1" << karows << stat1;

         if ( stat1 != 0 )
         {
            naerrs++;
            istat      = stat1;
         }
	 /****/
      }

      narows      = karows;
   }

   if ( remove && frLoc )
   {                                  // REMOVE from LOCAL
      for ( int ii = 4; ii > 1; ii-- )
      {  // Remove records from bottom up
         for ( int jj = 0; jj < narows; jj++ )
         {
            int irow   = actrows[ jj ];
            US_DataModel::DataDesc ddesc = da_model->row_datadesc( irow );
            int ityp   = ddesc.recType;

            if ( ityp != ii )  continue;  // skip if not at current level

            int stat1  = da_process->record_remove_local( irow );

            if ( stat1 != 0 )
            {
               naerrs++;
               istat      = stat1;
            }
         }
      }
   }

   if ( upLoad )           
   {                                  // UPLOAD to DB
      for ( int ii = 2; ii < 5; ii++ )
      {  // Upload records from top down
         for ( int jj = 0; jj < narows; jj++ )
         {
            int irow   = actrows[ jj ];
            US_DataModel::DataDesc ddesc = da_model->row_datadesc( irow );
            int ityp   = ddesc.recType;

            if ( ityp != ii )  continue;  // skip if not at current level

            int stat1  = da_process->record_upload( irow );

            if ( stat1 != 0 )
            {
               naerrs++;
               istat      = stat1;
            }
         }
      }
   }

   if ( dnLoad )            
   {                                  // DOWNLOAD to LOCAL
QTime timer;
timer.start();
      stlabel->setText( tr( "Downloading records to local disk..." ) );
      progress->setMaximum( narows );
      progress->setValue  ( 0 );
      int ndown = 0;
      for ( int ii = 2; ii < 5; ii++ )
      {  // Download records from top down
         for ( int jj = 0; jj < narows; jj++ )
         {
            int irow   = actrows[ jj ];
            US_DataModel::DataDesc ddesc = da_model->row_datadesc( irow );
            int ityp   = ddesc.recType;

            if ( ityp != ii )  continue;  // skip if not at current level

            int stat1  = da_process->record_download( irow );

            if ( stat1 != 0 )
            {
               naerrs++;
               istat      = stat1;
            }
            progress->setValue( ++ndown );
         }
qDebug() << "DT: DwnLd: ii" << ii << "time" << timer.elapsed();
      }
   }

   if ( naerrs > 0  &&  naerrs < narows )
   {  // Append to error message, explaining partial success
      da_process->partialError( naerrs, narows );
   }

   return istat;
}

// Create lists of selected rows and implied action rows
int US_DataTree::action_rows( )
{
   // Build up the list of selected rows; then sort it
   int nsrows = selitems.size();
   selrows.clear();
   actrows.clear();
   rawrows.clear();

   for ( int ii = 0; ii < nsrows; ii++ )
      selrows << selitems[ ii ]->type() - (int)QTreeWidgetItem::UserType;

   qSort( selrows );
DbgLv(1) << "acrow: nsrows" << nsrows;

   // Build up the list of action rows; include descendants, exclude Raw
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      int irow   = selrows[ ii ];
      US_DataModel::DataDesc ddesc = da_model->row_datadesc( irow );
      int ityp   = ddesc.recType;
DbgLv(1) << "acrow:  irow ityp" << irow << ityp;
      if ( ityp > 1 )
         actrows << irow;     // add to action list if not Raw
      else
         rawrows << irow;     // otherwise, add to Raw-selected list

      for ( int jrow = ( irow + 1 ); jrow < ncrecs; jrow++ )
      {  // add descendants from next-row until back-to-level
         US_DataModel::DataDesc ddesc = da_model->row_datadesc( jrow );

DbgLv(1) << "acrow:    jrow jtyp" << jrow << ddesc.recType;
         if ( ddesc.recType <= ityp )
            break;     // once we've reached or passed same level, break

         if ( selrows.contains( jrow ) )
            continue;  // if descendant already in select list, don't add

         actrows << jrow;
      }
   }
DbgLv(1) << "acrow:  narows nrrows" << actrows.size() << rawrows.size();

   return actrows.size();
}

// Check model or noise for significant differences
bool US_DataTree::significant_diffs( US_DataModel::DataDesc ddesc,
      US_DB2* dbP )
{
   bool differs = true;   // Assume significant differences unless model,noise
   const double dtoler = 5.0e-5;

   // If model, check for differences beyond tolerance in component values
   if ( ddesc.recType == 3 )
   {
      US_Model dmodel;
      US_Model fmodel;
      double   difmax = 0.0;

      int dst = dmodel.load( QString::number( ddesc.recordID ), dbP );
      int fst = fmodel.load( ddesc.filename );

      if ( dst != US_DB2::OK )
         dmodel.description = "DBAD:" + dmodel.description;

      if ( fst != US_DB2::OK )
         fmodel.description = "FBAD:" + fmodel.description;

DbgLv(1) << " CONFLICT? dst fst differs" << dst << fst << differs;
      if ( dmodel == fmodel )
         differs = false;          // Differences only in formatting

      else if ( ( dmodel.monteCarlo    == fmodel.monteCarlo    ) &&
                ( dmodel.description   == fmodel.description   ) &&
                ( dmodel.modelGUID     == fmodel.modelGUID     ) &&
                ( dmodel.editGUID      == fmodel.editGUID      ) &&
                ( dmodel.wavelength    == fmodel.wavelength    ) &&
                ( dmodel.variance      == fmodel.variance      ) &&
                ( dmodel.meniscus      == fmodel.meniscus      ) &&
                ( dmodel.optics        == fmodel.optics        ) &&
                ( dmodel.analysis      == fmodel.analysis      ) &&
                ( dmodel.global        == fmodel.global        ) &&
                ( dmodel.coSedSolute   == fmodel.coSedSolute   ) &&
                ( dmodel.components.size() == fmodel.components.size() ) )
      {  // Difference can only be in component values
         for ( int ii = 0; ii < dmodel.components.size(); ii++ )
         {
            if ( dmodel.components[ ii ] != fmodel.components[ ii ] )
            {  // There is a components difference, so measure it
               US_Model::SimulationComponent* dcomp = &dmodel.components[ ii ];
               US_Model::SimulationComponent* fcomp = &fmodel.components[ ii ];
               double dcvalu = qAbs( dcomp->signal_concentration );
               double fcvalu = qAbs( fcomp->signal_concentration );
               double valmin = qMin( dcvalu, fcvalu );
                      valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               double difrat = qAbs( dcvalu - fcvalu ) / valmin;
                      difmax = qMax( difrat, difmax );

               dcvalu = qAbs( dcomp->molar_concentration );
               fcvalu = qAbs( fcomp->molar_concentration );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );

               dcvalu = qAbs( dcomp->vbar20 );
               fcvalu = qAbs( fcomp->vbar20 );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );

               dcvalu = qAbs( dcomp->mw );
               fcvalu = qAbs( fcomp->mw );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );

               dcvalu = qAbs( dcomp->s );
               fcvalu = qAbs( fcomp->s );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );

               dcvalu = qAbs( dcomp->D );
               fcvalu = qAbs( fcomp->D );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );
DbgLv(1) << "   CONFLICT? D dcv fcv dr" << dcvalu << fcvalu << difrat;

               dcvalu = qAbs( dcomp->f_f0 );
               fcvalu = qAbs( fcomp->f_f0 );
               valmin = qMin( dcvalu, fcvalu );
               valmin = ( valmin == 0.0 ) ? dtoler : valmin;
               difrat = qAbs( dcvalu - fcvalu ) / valmin;
               difmax = qMax( difrat, difmax );
            }
         }

         differs = ( difmax > dtoler );
DbgLv(1) << " CONFLICT? model difmax differs" << difmax << differs;
      }

   }

   // if noise, check for differences beyond tolerance in noise values
   else if ( ddesc.recType == 4)
   {
      US_Noise dnoise;
      US_Noise fnoise;
      double   difmax = 0.0;

      int dst = dnoise.load( QString::number( ddesc.recordID ), dbP );
      int fst = fnoise.load( ddesc.filename );

      if ( dst != US_DB2::OK )
         dnoise.description = "DBAD:" + dnoise.description;

      if ( fst != US_DB2::OK )
         fnoise.description = "FBAD:" + fnoise.description;

      if ( dnoise == fnoise )
         differs = false;          // Differences only in formatting

      else if ( ( dnoise.type          == fnoise.type          ) &&
                ( dnoise.description   == fnoise.description   ) &&
                ( dnoise.noiseGUID     == fnoise.noiseGUID     ) &&
                ( dnoise.modelGUID     == fnoise.modelGUID     ) &&
                ( dnoise.minradius     == fnoise.minradius     ) &&
                ( dnoise.maxradius     == fnoise.maxradius     ) &&
                ( dnoise.values.size() == fnoise.values.size() ) )
      {  // Difference can only be in noise values
         for ( int ii = 0; ii < dnoise.values.size(); ii++ )
         {
            double dnvalu = qAbs( dnoise.values[ ii ] );
            double fnvalu = qAbs( fnoise.values[ ii ] );
            double valmin = qMin( dnvalu, fnvalu );
                   valmin = ( valmin == 0.0 ) ? dtoler : valmin;
            double difrat = qAbs( dnvalu - fnvalu ) / valmin;
                   difmax = qMax( difrat, difmax );
         }

         differs = ( difmax > dtoler );
DbgLv(1) << " CONFLICT? model difmax differs" << difmax << differs;
      }
   }

   return differs;
}

