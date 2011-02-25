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
   tw_recs->setFont(  QFont( "monospace", US_GuiSettings::fontSize() - 1 ) );
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

   da_process = (US_DataProcess*)da_model->procobj();

   ncrecs     = da_model->recCount();
   ndrecs     = da_model->recCountDB();
   nlrecs     = da_model->recCountLoc();
   ntrows     = ncrecs;
   ncraws     = ncedts = ncmods = ncnois = 0;
   ndraws     = ndedts = ndmods = ndnois = 0;
   nlraws     = nledts = nlmods = nlnois = 0;
   tw_recs->clear();

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
            if ( cont1 != cont2 )
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
   tw_item  = item;
   int irow = item->type() - (int)QTreeWidgetItem::UserType;
DbgLv(2) << "    context_menu row" << irow+1;
   da_model->setCurrent( irow );
DbgLv(2) << "    context_menu RTN setCurrent";
   cdesc    = da_model->current_datadesc();
DbgLv(2) << "    context_menu RTN current_datadesc";

   QMenu*   cmenu   = new QMenu();
   QAction* upldact = new QAction( tr( " upload to DB" ),      parentw );
   QAction* dnldact = new QAction( tr( " download to local" ), parentw );
   QAction* rmdbact = new QAction( tr( " remove from DB" ),    parentw );
   QAction* rmloact = new QAction( tr( " remove from local" ), parentw );
   QAction* rmboact = new QAction( tr( " remove both" ),       parentw );
   QAction* shdeact = new QAction( tr( " show details" ),      parentw );

   cmenu->addAction( upldact );
   cmenu->addAction( dnldact );
   cmenu->addAction( rmdbact );
   cmenu->addAction( rmloact );
   cmenu->addAction( rmboact );
   cmenu->addAction( shdeact );
DbgLv(2) << "    context_menu RTN addAction";

   connect( upldact, SIGNAL( triggered() ),
            this,    SLOT(   item_upload()     ) );
   connect( dnldact, SIGNAL( triggered() ),
            this,    SLOT(   item_download()   ) );
   connect( rmdbact, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_db()  ) );
   connect( rmloact, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_loc() ) );
   connect( rmboact, SIGNAL( triggered() ),
            this,    SLOT(   item_remove_all() ) );
   connect( shdeact, SIGNAL( triggered() ),
            this,    SLOT(   item_details()    ) );

   // disable menu items that are not appropriate to the record

   if ( ( cdesc.recState & US_DataModel::REC_LO ) == 0 )
   {  // if record not local, no upload and no remove from local or both
      upldact->setEnabled( false );
      rmloact->setEnabled( false );
      rmboact->setEnabled( false );
   }

   if ( ( cdesc.recState & US_DataModel::REC_DB ) == 0 )
   {  // if record not db, no download and no remove from db or both
      dnldact->setEnabled( false );
      rmdbact->setEnabled( false );
      rmboact->setEnabled( false );
   }

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
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
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
      int stat1  = da_process->record_upload( irow );

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
      int stat1  = da_process->record_download( irow );

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
   QString     item_act = tr( "DB remove" );
   QMessageBox msgBox( parentw );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   if ( ( cdesc.recState & US_DataModel::REC_LO ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "DB-only remove" );
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
      int stat1  = da_process->record_remove_db( irow );

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
   QString     item_act = tr( "Local remove" );
   QMessageBox msgBox( parentw );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   da_model->setCurrent( irow );
   cdesc    = da_model->current_datadesc();

   if ( ( cdesc.recState & US_DataModel::REC_DB ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "Local-only remove" );
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
      int stat1  = da_process->record_remove_local( irow );

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
      int stat1  = da_process->record_remove_local( irow );
      stat1     += da_process->record_remove_db(    irow );

      action_result( stat1, item_act );
   }

   else
   {
DbgLv(2) << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item details action
void US_DataTree::item_details(  )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   QString     fileexts = tr( "Text,Log files (*.txt *.log);;" )
      + tr( "All files (*)" );
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
      tr( "  File Directory : " )
         + cdesc.filename.section( "/",  0, -2 ) + "\n" +
      tr( "  File Name      : " )
         + cdesc.filename.section( "/", -1, -1 ) + "\n" +
      tr( "  Record State   : " ) + record_state( cdesc.recState ) + "\n" +
      tr( "  Last Mod Date  : " ) + cdesc.lastmodDate + "\n";

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
   }

   // display the text dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Data Tree Entry Details" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// prepend a record type string to an item action
void US_DataTree::record_type( int recType, QString& item_act )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   int sizert = sizeof( rtyps ) / sizeof( rtyps[ 0 ] );

   if ( recType > 0  &&  recType <= sizert )
   {
      item_act = QString( rtyps[ recType - 1 ] ) + " " + item_act;
   }
}


// format an item action text for a message box
QString US_DataTree::action_text( QString exstext, QString acttext )
{
   return tr( "This item exists on %1.<br>"
              "Are you sure you want to proceed with a %2?<ul>"
              "<li><b>No </b> to cancel the action;</li>"
              "<li><b>Yes</b> to proceed with the action.</li></ul>" )
          .arg( exstext ).arg( acttext );
}

// report the result of an item action
void US_DataTree::action_result( int stat, QString item_act )
{
   if ( stat != 999 )
   {  // proceed was selected:  test status of action

      if ( stat == 0 )
      {  // action was successful
         QMessageBox::information( parentw,
               item_act + tr( " Successful!" ),
               tr( "The \"%1\" action was successfully performed." )
               .arg( item_act ) );
      }

      else
      {  // action got an error
         QMessageBox::warning( parentw,
               item_act + tr( " *ERROR*!" ),
               tr( "The \"%1\" action had an error: %2" )
               .arg( item_act ).arg( stat )
               + "\n\n" + da_process->lastError() );
      }
   }

   else
   {  // cancel was selected:  report it
      QMessageBox::information( parentw,
            item_act + tr( " Cancelled!" ),
            tr( "The \"%1\" action was cancelled." ).arg( item_act ) );
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

