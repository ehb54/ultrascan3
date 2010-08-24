//! \file us_manage_data.cpp

#include <QApplication>

#include <uuid/uuid.h>

#include "us_manage_data.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_editor.h"
#include "us_dataIO2.h"
#include "us_noise.h"
#include "us_util.h"
#include "us_convertio.h"
#include "us_expinfo.h"
#include "us_selectbox.h"
#include "us_buffer_gui.h"
#include "us_analyte_gui.h"
#include "us_util.h"

const QColor colorRed(   240,   0,   0 );
const QColor colorBlue(    0,   0, 255 );
const QColor colorBrown( 120,  60,   0 );
const QColor colorGreen(   0, 150,   0 );
const QColor colorGray(  110, 110, 110 );
const QColor colorWhite( 255, 255, 240 );

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_ManageData w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_ManageData class constructor
US_ManageData::US_ManageData() : US_Widgets()
{
   // set up the GUI

   setWindowTitle( tr( "Manage US DB/Local Data Sets" ) );
   setPalette( US_GuiSettings::frameColor() );

   // primary layouts
   QHBoxLayout* mainLayout  = new QHBoxLayout( this );
   QVBoxLayout* leftLayout  = new QVBoxLayout();
   QVBoxLayout* rghtLayout  = new QVBoxLayout();
   QGridLayout* dctlLayout  = new QGridLayout();
   QVBoxLayout* smryLayout  = new QVBoxLayout();
   QGridLayout* statLayout  = new QGridLayout();
   QGridLayout* tctlLayout  = new QGridLayout();
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );
   leftLayout->setSpacing        ( 0 );
   leftLayout->setContentsMargins( 0, 1, 0, 1 );
   rghtLayout->setSpacing        ( 0 );
   rghtLayout->setContentsMargins( 0, 1, 0, 1 );
   dctlLayout->setSpacing        ( 1 );
   dctlLayout->setContentsMargins( 0, 0, 0, 0 );
   smryLayout->setSpacing        ( 0 );
   smryLayout->setContentsMargins( 0, 1, 0, 1 );
   statLayout->setSpacing        ( 1 );
   statLayout->setContentsMargins( 0, 0, 0, 0 );
   tctlLayout->setSpacing        ( 1 );
   tctlLayout->setContentsMargins( 0, 0, 0, 0 );

   // fill in the GUI components
   int row   = 0;

   pb_invtor     = us_pushbutton( tr( "Investigator" ) );
   dctlLayout->addWidget( pb_invtor, row,   0, 1, 3 );
   le_invtor     = us_lineedit();
   dctlLayout->addWidget( le_invtor, row++, 3, 1, 5 );

   pb_browse     = us_pushbutton( tr( "Browse Data" ) );
   dctlLayout->addWidget( pb_browse, row,   0, 1, 4 );
   connect( pb_browse,  SIGNAL( clicked()     ),
            this,       SLOT( browse_data()   ) );

   pb_detail     = us_pushbutton( tr( "Detail Data" ) );
   dctlLayout->addWidget( pb_detail, row++, 4, 1, 4 );
   connect( pb_detail,  SIGNAL( clicked()     ),
            this,       SLOT( detail_data()   ) );

   pb_hsedit     = us_pushbutton( tr( "Show All Edits" ) );
   dctlLayout->addWidget( pb_hsedit, row,   0, 1, 4 );
   connect( pb_hsedit,  SIGNAL( clicked()     ),
            this,       SLOT( toggle_edits()  ) );

   pb_hsmodl     = us_pushbutton( tr( "Show All Models" ) );
   dctlLayout->addWidget( pb_hsmodl, row++, 4, 1, 4 );
   connect( pb_hsmodl,  SIGNAL( clicked()     ),
            this,       SLOT( toggle_models() ) );

   pb_hsnois     = us_pushbutton( tr( "Show All Noises" ) );
   dctlLayout->addWidget( pb_hsnois, row,   0, 1, 4 );
   connect( pb_hsnois,  SIGNAL( clicked()     ),
            this,       SLOT( toggle_noises() ) );

   pb_helpdt     = us_pushbutton( tr( "Data Tree Help" ) );
   dctlLayout->addWidget( pb_helpdt, row++, 4, 1, 4 );
   connect( pb_helpdt,  SIGNAL( clicked()     ),
            this,       SLOT( dtree_help() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ), false );
   dctlLayout->addWidget( pb_reset,  row,   2, 1, 2 );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT(   reset()   ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   dctlLayout->addWidget( pb_help,   row,   4, 1, 2 );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   dctlLayout->addWidget( pb_close,  row++, 6, 1, 2 );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT(   close()   ) );
   pb_invtor->setToolTip( 
      tr( "Use an Investigator dialog to set the database person ID" ) );
   pb_browse->setToolTip(
      tr( "Review DB and Local data, skipping content analysis" ) );
   pb_detail->setToolTip(
      tr( "Review DB and Local data, with detailed content analysis" ) );
   pb_helpdt->setToolTip(
      tr( "Show a short Help/Legend dialog for notes on the data tree" ) );
   pb_reset ->setToolTip(
      tr( "Reset the data display to its default state" ) );
   pb_help  ->setToolTip(
      tr( "Display detailed US_ManageData documentation text and images" ) );
   pb_close ->setToolTip(
      tr( "Close the US_ManageData window and exit" ) );

   QLabel* lb_info2 = us_banner( tr( "User Data Sets Summary:" ) );
   dctlLayout->addWidget( lb_info2,  row++, 0, 1, 8 );

   QPalette pa( le_invtor->palette() );
   te_status    = us_textedit( );
   te_status->setPalette( pa );
   te_status->setPalette( US_GuiSettings::normalColor() );
   te_status->setTextBackgroundColor( pa.color( QPalette::Window ) );
   te_status->setTextColor( pa.color( QPalette::WindowText ) );
   te_status->setFont(  QFont( "monospace", US_GuiSettings::fontSize() - 2 ) );
   te_status->setText(
      tr( "%1 Combined Total data sets;\n" ).arg( 0 ) +
      tr( "  %1 Combined RawData    records;\n" ).arg( 0 ) +
      tr( "  %1 Combined EditedData records;\n" ).arg( 0 ) +
      tr( "  %1 Combined Model      records;\n" ).arg( 0 ) +
      tr( "  %1 Combined Noise      records.\n" ).arg( 0 ) +
      tr( "%1 Database Total data sets;\n" ).arg( 0 ) +
      tr( "  %1 Database RawData    records;\n" ).arg( 0 ) +
      tr( "  %1 Database EditedData records;\n" ).arg( 0 ) +
      tr( "  %1 Database Model      records;\n" ).arg( 0 ) +
      tr( "  %1 Database Noise      records.\n" ).arg( 0 ) +
      tr( "%1 Local    Total data sets;\n" ).arg( 0 ) +
      tr( "  %1 Local    RawData    records;\n" ).arg( 0 ) +
      tr( "  %1 Local    EditedData records;\n" ).arg( 0 ) +
      tr( "  %1 Local    Model      records;\n" ).arg( 0 ) +
      tr( "  %1 Local    Noise      records.\n" ).arg( 0 ) );
   smryLayout->addWidget( te_status );

   row  = 0;
   QLabel* lb_progr  = us_label( tr( "% Completed:" ) );
   progress          = us_progressBar( 0, 100, 0 );
   statLayout->addWidget( lb_progr,  row,   0, 1, 3 );
   statLayout->addWidget( progress,  row++, 3, 1, 5 );

   lb_status         = us_label( tr( "Status" ) );
   statLayout->addWidget( lb_status, row,   0, 1, 8 );

   // set up data tree; populate with sample data
   rbtn_click        = false;
   tw_recs           = new QTreeWidget();
   tw_recs->setPalette( te_status->palette() );
   tctlLayout->addWidget( tw_recs );

   QStringList theads;
   theads << "Type" << "Label" << "SubType" << "Source"
      << "Children\nDB, Local" << "Descendants\nDB, Local";
   ntrows = 5;
   ntcols = theads.size();
   tw_recs->setHeaderLabels( theads );
   tw_recs->setFont(  QFont( "monospace", US_GuiSettings::fontSize() - 1 ) );
   QTreeWidgetItem* pitems[4];

   for ( int ii = 0; ii < ntrows; ii++ )
   {
      QColor  bg   = colorWhite;
      QColor  fg   = colorBlue;
      QString rtyp = "Raw";
      QString labl = QString( "item%11" ).arg( ii + 1 );
      QString subt = "";
      QString rsrc = "DB";
      QString anch = "1, 1";
      QString ande = "2, 3";
      QStringList cvals;
      int ityp     = 1;

      if      ( ii == 1 )
      {
         fg   = colorGreen;
         rtyp = "Edited";
         ityp = 2;
         rsrc = "In Sync";
         anch = "0, 1";
         ande = "1, 1";
         subt = "RA";
      }
      else if ( ii == 2 )
      {
         fg   = colorBrown;
         rtyp = "Model";
         ityp = 3;
         rsrc = "Local";
         anch = "1, 1";
         ande = "";
         subt = "2DSA";
      }
      else if ( ii == 3 )
      {
         fg   = colorRed;
         rtyp = "Noise";
         ityp = 4;
         rsrc = "Conflict";
         anch = "";
         ande = "";
         subt = "TI";
      }
      else if ( ii == 4 )
      {
         fg   = colorGray;
         rtyp = "Edited";
         ityp = 2;
         rsrc = "dummy";
         anch = "0, 0";
         ande = "0, 0";
         subt = "";
      }
      labl = QString( "item_%1_2" ).arg( ii + 1 );

      QTreeWidgetItem* item;
      int wiutype = (int)QTreeWidgetItem::UserType + ii; // type: encoded index

      cvals << rtyp << labl << subt << rsrc << anch << ande;

      if ( ityp <= 1 )
         // Raws are children of root
         item = new QTreeWidgetItem( tw_recs,            cvals, wiutype );
      else
         // others are children of next level up
         item = new QTreeWidgetItem( pitems[ ityp - 2 ], cvals, wiutype );

      pitems[ ityp - 1 ] = item;  // save next parent item of this type;

      for ( int jj = 0; jj < ntcols; jj++ )
      {
         item->setForeground( jj, QBrush( fg ) );
         item->setBackground( jj, QBrush( bg ) );
      }
   }

   tw_recs->expandAll();

   for ( int jj = 0; jj < ntcols; jj++ )
   {
      tw_recs->resizeColumnToContents( jj );
   }

   tw_recs->collapseAll();

   tw_recs->setObjectName( QString( "tree-widget" ) );
   tw_recs->setAutoFillBackground( true );
   tw_recs->installEventFilter( this );
   connect( tw_recs, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
            this,    SLOT(   clickedItem( QTreeWidgetItem* ) ) );

   reset_hsbuttons( false, true, true, true );  // hs button labels,tooltips

   // put layouts together for overall layout
   leftLayout->addLayout( dctlLayout );
   leftLayout->addLayout( smryLayout );
   leftLayout->setStretchFactor( smryLayout, 10 );
   leftLayout->addLayout( statLayout );
   rghtLayout->addLayout( tctlLayout );

   mainLayout->addLayout( leftLayout );
   mainLayout->addLayout( rghtLayout );
   mainLayout->setStretchFactor( leftLayout, 1  );
   mainLayout->setStretchFactor( rghtLayout, 10 );

   show();    // display main window before password dialog appears

   // insure we can connect to the database
   US_Passwd pw;
   db            = new US_DB2( pw.getPasswd() );
   if ( db->lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db->lastError() );
      close();
   }

   personID      = 0;

   // if possible get db investigator from user name
   investig      = QDir( QDir::homePath() ).dirName();
   le_invtor->setText( investig );

   find_investigator(  investig );

   le_invtor->setText( investig );

   connect( pb_invtor,  SIGNAL( clicked()           ),
            this,       SLOT( sel_investigator()    ) );
   connect( le_invtor,  SIGNAL( editingFinished()   ),
            this,       SLOT( chg_investigator() )  );

   // set up initial state of GUI
   reset();
}

// reset the GUI
void US_ManageData::reset( void )
{
   ncrecs     = 0;
   ndrecs     = 0;
   nlrecs     = 0;
}

// filter events to catch right-mouse-button-click on tree widget
bool US_ManageData::eventFilter( QObject *obj, QEvent *e )
{
   if ( obj->objectName() == "tree-widget"  &&
        e->type() == QEvent::ContextMenu )
   {  // catch tree row right-mouse click
      rbtn_click = true;
      cur_pos    = QCursor::pos();
      return false;
   }

   else
   {  // pass all others for normal handling
      return US_Widgets::eventFilter( obj, e );
   }
}

// handle change of investigator text
void US_ManageData::chg_investigator()
{
   investig = le_invtor->text();     // get current (partial) investigator text

   find_investigator( investig );    // find an actual one that matches

   le_invtor->setText( investig );   // display the full investigator string
}

// query database for match to (partial) investigator name
void US_ManageData::find_investigator( QString& invname )
{
   QStringList query;
   QString     dbInvestID;
   QString     dbLastName;
   QString     dbFirstName;
   int         irow    = 1;
   int         nrows   = 1;

   lb_status->setText( tr( "Examining Investigators..." ) );
   query << "get_people" << invname;
   db->query( query );

   if ( db->numRows() < 1 )
   {  // Investigator text yields nothing:  retry with blank field
      query.clear();
      query << "get_people" << "";
      db->query( query );
   }

   nrows    = db->numRows();
   progress->setMaximum( nrows);

   while ( db->next() )
   {  // Loop through investigators looking for a match
      dbInvestID  = db->value( 0 ).toString();
      dbLastName  = db->value( 1 ).toString();
      dbFirstName = db->value( 2 ).toString();

      if ( nrows < 2  ||
           dbLastName.contains(  invname, Qt::CaseInsensitive )  ||
           dbFirstName.contains( invname, Qt::CaseInsensitive ) )
      {  // Single investigator or a match to last/first name
         invname    = dbInvestID + ": " + dbLastName + ", " + dbFirstName;
         personID   = dbInvestID.toInt();
         break;
      }
      progress->setValue( irow++ );
   }

   progress->setValue( nrows );
   lb_status->setText( tr( "Investigator Search Complete" ) );
}

// toggle edits between hide/show
void US_ManageData::toggle_edits()
{
   bool show = pb_hsedit->text().startsWith( "Show" ); // show or hide?

   toggle_expand( "Raw",    show );    // expand/collapse one level up

   if ( show )
   {  // for show edits, need only relabel edit button
      reset_hsbuttons( show, true, false, false );
   }

   else
   {  // for hide edits, must toggle labels below; relabel all buttons
      toggle_expand( "Model", show );
      toggle_expand( "Noise", show );

      reset_hsbuttons( show, true, true, true );
   }
}

// toggle models between hide/show
void US_ManageData::toggle_models()
{
   bool show = pb_hsmodl->text().startsWith( "Show" ); // show or hide?

   toggle_expand( "Edited", show );    // expand/collapse one level up

   if ( show )
   { // for show models, must expand levels above; reset edit,model buttons
      toggle_expand( "Raw",    show );

      reset_hsbuttons( show, false, true, true );
   }

   else
   {  // for hide models, must collapse below;  reset model,noise buttons
      toggle_expand( "Noise", false );

      reset_hsbuttons( show, false, true, true );
   }
}

// toggle noises between hide/show
void US_ManageData::toggle_noises()
{
   bool show = pb_hsnois->text().startsWith( "Show" )  // show or hide?
            || pb_hsnois->text().startsWith( "Expand" );

   toggle_expand( "Model",  show );    // expand/collapse one level up

   if ( show )
   {  // for show noise, must expand above; reset edit,model,noise buttons
      toggle_expand( "Edited", show );
      toggle_expand( "Raw",    show );

      reset_hsbuttons( show, true, true, true );
   }

   else
   {  // for hide noise, need only reset noise button
      reset_hsbuttons( show, false, false, true );
   }

}

// show or hide (expand/collapse) all rows of specified type
void US_ManageData::toggle_expand( QString c1str, bool show )
{
   QList< QTreeWidgetItem* > listi = tw_recs->findItems(
      c1str, Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive, 0 );
//qDebug() << "TOG_HIDD: match show listi.size"
// << c1str << show << listi.size();

   for ( int ii = 0; ii < listi.size(); ii++ )
   {
      listi.at( ii )->setExpanded( show );
   }
}

// change hide/show button text and tooltips for new expand/collapse state
void US_ManageData::reset_hsbuttons( bool show,
      bool edts, bool mods, bool nois )
{
   QString blabl;
   QString btool;

   if ( show )
   {  // show (expand) is the new state
      if ( edts )
      { // Hide Edits 
         pb_hsedit->setText( tr( "Collapse All" ) );
         pb_hsedit->setToolTip( 
            tr( "Collapse All, so Edits and all descendants are hidden" ) );
      }

      if ( mods )
      { // Hide Models
         pb_hsmodl->setText( tr( "Hide All Models" ) );
         pb_hsmodl->setToolTip( 
            tr( "Collapse Edits, so Models and their children are hidden" ) );
      }

      if ( nois )
      { // Hide Noises
         pb_hsnois->setText( tr( "Hide All Noises" ) );
         pb_hsnois->setToolTip( 
            tr( "Collapse Models, so Noises are hidden" ) );
      }
   }

   else
   {  // hide (collapse) is the new state
      if ( edts )
      { // Show Edits 
         pb_hsedit->setText( tr( "Show All Edits" ) );
         pb_hsedit->setToolTip( 
            tr( "Expand Raws, so Edits and parent Raws are shown" ) );
      }

      if ( mods )
      { // Show Models
         pb_hsmodl->setText( tr( "Show All Models" ) );
         pb_hsmodl->setToolTip( 
            tr( "Expand Edits, so Models and their parents are shown" ) );
      }

      if ( nois )
      { // Show Noises
         pb_hsnois->setText( tr( "Expand All" ) );
         pb_hsnois->setToolTip( 
            tr( "Expand All, so Noises and all ancestors are shown" ) );
      }
   }
}

// browse the database and local disk for R/E/M/N data sets
void US_ManageData::browse_data()
{
   ddescs.clear();   // db descriptions
   ldescs.clear();   // local descriptions
   adescs.clear();   // all descriptions

   kdmy        = 0;
   details     = false;

   browse_dbase( );            // read db to build db descriptions

   sort_descs(   ddescs  );    // sort db descriptions

   browse_local( );            // read files to build local descriptions

   sort_descs(   ldescs  );    // sort local descriptions

   merge_dblocal();            // merge database and local descriptions

   build_dtree();              // rebuild the data tree with present data
}

// browse the database,local for R/E/M/N data sets, with content detail
void US_ManageData::detail_data()
{
   ddescs.clear();   // db descriptions
   ldescs.clear();   // local descriptions
   adescs.clear();   // all descriptions

   kdmy        = 0;
   details     = true;

   browse_dbase( );            // read db to build db descriptions

   sort_descs(   ddescs  );    // sort db descriptions

   browse_local( );            // read files to build local descriptions

   sort_descs(   ldescs  );    // sort local descriptions

   merge_dblocal();            // merge database and local descriptions

   build_dtree();              // rebuild the data tree with present data
}

// build the data tree from descriptions read
void US_ManageData::build_dtree()
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

   ncrecs     = adescs.size();
   ndrecs     = ddescs.size();
   nlrecs     = ldescs.size();
   ntrows     = ncrecs;
   ncraws     = ncedts = ncmods = ncnois = 0;
   ndraws     = ndedts = ndmods = ndnois = 0;
   nlraws     = nledts = nlmods = nlnois = 0;
   tw_recs->clear();

   for ( int ii = 0; ii < ncrecs; ii++ )
   {
      cdesc      = adescs[ ii ];
      ityp       = cdesc.recType;
      QStringList cvals;

      if ( cdesc.recordID < 0 )
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

      else if ( cdesc.filename.isEmpty() )
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

         if ( details )
         {
            QString cont1 = cdesc.contents.section( " ", 0, 1 ).simplified();
            QString cont2 = cdesc.contents.section( " ", 2, 4 ).simplified();
            if ( cont1 != cont2 )
            {
               fbru    = QBrush( colorRed );
               rsrc    = "Conflict";
qDebug() << "CONFLICT cont1 cont2" << cont1 << cont2;
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

      if ( cdesc.recState == NOSTAT )
      {  // mark artificial record with color and source text
         fbru       = QBrush( colorGray );
         rsrc       = "dummy";
      }

      for ( int jj = ( ii + 1 ); jj < ncrecs; jj++ )
      {  // count children and descendants from next-row until back-to-level

         if ( adescs[ jj ].recType <= cdesc.recType )
            break;   // once we've reached or passed same level, break

         bool isDba = ( ( adescs[ jj ].recState & REC_DB ) != 0 );  // database
         bool isLoc = ( ( adescs[ jj ].recState & REC_LO ) != 0 );  // local
//qDebug() << "  drow rTyp crTyp cSta"
// << jj+1 << ityp << adescs[jj].recType << adescs[jj].recState;

         if ( adescs[ jj ].parentGUID == dguid )
         {  // this is a child, since it has parent GUID matching level above
            if ( isDba ) { nchdb++; ndedb++; }
            if ( isLoc ) { nchlo++; ndelo++; }
         }

         else
         {  // this is descendant, not direct child
            if ( isDba ) { ndedb++; }
            if ( isLoc ) { ndelo++; }
//qDebug() << "isDba cpgid dguid row drow ndedb"
// << isDba << adescs[jj].parentGUID << dguid << ii+1 << jj+1 << ndedb;
         }
      }

      anch = ( ityp < 4 ) ?
         QString( "%1, %2" ).arg( nchdb ).arg( nchlo ) : "";  // children
      ande = ( ityp < 3 ) ?
         QString( "%1, %2" ).arg( ndedb ).arg( ndelo ) : "";  // descendants

      QTreeWidgetItem* item;
      int wiutype = (int)QTreeWidgetItem::UserType + ii; // type: encoded index

      cvals << rtyp << labl << subt << rsrc << anch << ande;
if ( ityp < 1 || ityp > 4 ) qDebug() << "*** row type ***" << ii+1 << ityp;

      if ( ityp == 1 )
      {  // Raws are children of the root
         item = new QTreeWidgetItem( tw_recs,            cvals, wiutype );
//if ( ii > 13 && ii < 20 )
// qDebug() << " row rtyp *ROOT*" << ii+1 << rtyp;
      }

      else
      {  // others are children of the next level up
         item = new QTreeWidgetItem( pitems[ ityp - 2 ], cvals, wiutype );
//if ( ii > 13 && ii < 20 )
// qDebug() << " row rtyp prtyp" << ii+1 << rtyp << pitems[ityp-2]->text(0);
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

   this->resize( 1000, 500 );

   reset_hsbuttons( false, true, true, true );  // hs button labels,tooltips

   // reformat and display report on record counts
   const char* fmtn[] = { "\%2d", "\%3d", "\%4d" };
   int         ff     = ( ncrecs < 100 )  ? 0 : ( ( ncrecs < 1000 ) ? 1 : 2 );
   QString actr = QString().sprintf( fmtn[ ff ], ncrecs );
   QString acrr = QString().sprintf( fmtn[ ff ], ncraws );
   QString acer = QString().sprintf( fmtn[ ff ], ncedts );
   QString acmr = QString().sprintf( fmtn[ ff ], ncmods );
   QString acnr = QString().sprintf( fmtn[ ff ], ncnois );
   QString adtr = QString().sprintf( fmtn[ ff ], ndrecs );
   QString adrr = QString().sprintf( fmtn[ ff ], ndraws );
   QString ader = QString().sprintf( fmtn[ ff ], ndedts );
   QString admr = QString().sprintf( fmtn[ ff ], ndmods );
   QString adnr = QString().sprintf( fmtn[ ff ], ndnois );
   QString altr = QString().sprintf( fmtn[ ff ], nlrecs );
   QString alrr = QString().sprintf( fmtn[ ff ], nlraws );
   QString aler = QString().sprintf( fmtn[ ff ], nledts );
   QString almr = QString().sprintf( fmtn[ ff ], nlmods );
   QString alnr = QString().sprintf( fmtn[ ff ], nlnois );
   te_status->setText(
      actr + tr( " Combined Total data sets;\n  "      ) +
      acrr + tr( " Combined RawData    records;\n  "   ) +
      acer + tr( " Combined EditedData records;\n  "   ) +
      acmr + tr( " Combined Model      records;\n  "   ) +
      acnr + tr( " Combined Noise      records.\n"     ) +
      adtr + tr( " Database Total data sets;\n  "      ) +
      adrr + tr( " Database RawData    records;\n  "   ) +
      ader + tr( " Database EditedData records;\n  "   ) +
      admr + tr( " Database Model      records;\n  "   ) +
      adnr + tr( " Database Noise      records.\n"     ) +
      altr + tr( " Local    Total data sets;\n  "      ) +
      alrr + tr( " Local    RawData    records;\n  "   ) +
      aler + tr( " Local    EditedData records;\n  "   ) +
      almr + tr( " Local    Model      records;\n  "   ) +
      alnr + tr( " Local    Noise      records.\n"     ) );

}

// open dialog and get investigator when button clicked
void US_ManageData::sel_investigator()
{
   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT(   assign_investigator  ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

// assign an investigator string in proper id:lastname,firstname form
void US_ManageData::assign_investigator( int invID,
      const QString& lname, const QString& fname )
{
   personID   = invID;
   investig   = QString::number( invID ) + ": " + lname + ", " + fname;
   le_invtor->setText( investig );
}

// handle a right-mouse click of a row cell
void US_ManageData::clickedItem( QTreeWidgetItem* item )
{
//qDebug() << "TABLE ITEM CLICKED rbtn_click" << rbtn_click;

   if ( rbtn_click )
   {  // only bring up context menu if right-mouse-button was clicked
      row_context_menu( item );
   }
   rbtn_click        = false;
}

// set up and display a row context menu
void US_ManageData::row_context_menu( QTreeWidgetItem* item )
{
   tw_item  = item;
   int irow = item->type() - (int)QTreeWidgetItem::UserType;
//qDebug() << "    context_menu row" << irow+1;

   QMenu*   cmenu   = new QMenu();
   QAction* upldact = new QAction( tr( " upload to DB" ), this );
   QAction* dnldact = new QAction( tr( " download to local" ), this );
   QAction* rmdbact = new QAction( tr( " remove from DB" ), this );
   QAction* rmloact = new QAction( tr( " remove from local" ), this );
   QAction* rmboact = new QAction( tr( " remove both" ), this );
   QAction* shdeact = new QAction( tr( " show details" ), this );

   cmenu->addAction( upldact );
   cmenu->addAction( dnldact );
   cmenu->addAction( rmdbact );
   cmenu->addAction( rmloact );
   cmenu->addAction( rmboact );
   cmenu->addAction( shdeact );

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
   cdesc    = adescs[ irow ];

   if ( ( cdesc.recState & REC_LO ) == 0 )
   {  // if record not local, no upload and no remove from local or both
      upldact->setEnabled( false );
      rmloact->setEnabled( false );
      rmboact->setEnabled( false );
   }

   if ( ( cdesc.recState & REC_DB ) == 0 )
   {  // if record not db, no download and no remove from db or both
      dnldact->setEnabled( false );
      rmdbact->setEnabled( false );
      rmboact->setEnabled( false );
   }

   // display the context menu and act on selection
   cmenu->exec( QCursor::pos() );

}

// open a dialog and display data tree help
void US_ManageData::dtree_help()
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
      tr( "       -> Upload this local record/tree to DB;\n" ) +
      tr( "    \"download to local\"\n" ) +
      tr( "       -> Download this DB record/tree to local disk.\n" ) +
      tr( "    \"remove from DB\"\n" ) +
      tr( "       -> Remove this record/tree from DB.\n" ) +
      tr( "    \"remove from local\"\n" ) +
      tr( "       > Remove this record/tree from local disk.\n" ) +
      tr( "    \"remove both\"\n" ) +
      tr( "       -> Remove this record/tree from both DB and local.\n" ) +
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

// format an item action text for a message box
QString US_ManageData::action_text( QString exstext, QString acttext )
{
   return tr( "This item exists on %1.<br>"
              "Are you sure you want to proceed with a %2?<ul>"
              "<li><b>No </b> to cancel the action;</li>"
              "<li><b>Yes</b> to proceed with the action.</li></ul>" )
          .arg( exstext ).arg( acttext );
}

// report the result of an item action
void US_ManageData::action_result( int stat, QString item_act )
{
   if ( stat != 999 )
   {  // proceed was selected:  test status of action

      if ( stat == 0 )
      {  // action was successful
         QMessageBox::information( this,
               item_act + tr( " Successful!" ),
               tr( "The \"%1\" action was successfully performed." )
               .arg( item_act ) );
      }

      else
      {  // action got an error
         QMessageBox::warning( this,
               item_act + tr( " *ERROR*!" ),
               tr( "The \"%1\" action had an error: %2" )
               .arg( item_act ).arg( stat ) );
      }
   }

   else
   {  // cancel was selected:  report it
      QMessageBox::information( this,
            item_act + tr( " Cancelled!" ),
            tr( "The \"%1\" action was cancelled." ).arg( item_act ) );
   }
}

// perform item upload action
void US_ManageData::item_upload()
{
   QMessageBox msgBox( this );
   QString     item_exs = tr( "Local disk only" );
   QString     item_act = tr( "DB create" );
qDebug() << "ITEM Upload";
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   cdesc    = adescs.at( irow );

   if ( ( cdesc.recState & REC_DB ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "DB replace" );
   }

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
qDebug() << " ITEM ACTION: YES";
      int stat1  = record_upload( irow );

      action_result( stat1, item_act );
   }

   else
   {
qDebug() << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }

}

// perform item download action
void US_ManageData::item_download()
{
   QMessageBox msgBox( this );
   QString     item_exs = tr( "Database only" );
   QString     item_act = tr( "Local file create" );
qDebug() << "ITEM Download";
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   cdesc    = adescs.at( irow );

   if ( ( cdesc.recState & REC_LO ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "Local file replace" );
   }

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
qDebug() << " ITEM ACTION: YES";
      int stat1  = record_download( irow );

      action_result( stat1, item_act );
   }

   else
   {
qDebug() << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-db action
void US_ManageData::item_remove_db()
{
qDebug() << "ITEM Remove from DB";
   QString     item_exs = tr( "Database only" );
   QString     item_act = tr( "DB remove" );
   QMessageBox msgBox( this );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   cdesc    = adescs.at( irow );

   if ( ( cdesc.recState & REC_LO ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "DB-only remove" );
   }

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
qDebug() << " ITEM ACTION: YES";
      int stat1  = record_remove_db( irow );

      action_result( stat1, item_act );
   }

   else
   {
qDebug() << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-local action
void US_ManageData::item_remove_loc()
{
qDebug() << "ITEM Remove from Local";
   QString     item_exs = tr( "Local disk only" );
   QString     item_act = tr( "Local remove" );
   QMessageBox msgBox( this );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   cdesc    = adescs.at( irow );

   if ( ( cdesc.recState & REC_DB ) != 0 )
   {
      item_exs = tr( "both DB and Local" );
      item_act = tr( "Local-only remove" );
   }

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
qDebug() << " ITEM ACTION: YES";
      int stat1  = record_remove_local( irow );

      action_result( stat1, item_act );
   }

   else
   {
qDebug() << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item remove-from-all action
void US_ManageData::item_remove_all()
{
qDebug() << "ITEM Remove Both DB and Local";
   QString     item_exs = tr( "both DB and Local" );
   QString     item_act = tr( "DB+Local remove" );
   QMessageBox msgBox( this );
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
   cdesc    = adescs.at( irow );

   msgBox.setWindowTitle( item_act );
   msgBox.setTextFormat(  Qt::RichText );
   msgBox.setText( action_text( item_exs, item_act ) );
   msgBox.addButton( QMessageBox::No );
   msgBox.addButton( QMessageBox::Yes );
   msgBox.setDefaultButton( QMessageBox::No );

   if ( msgBox.exec() == QMessageBox::Yes )
   {
qDebug() << " ITEM ACTION: YES";
      int stat1  = record_remove_local( irow );
      stat1     += record_remove_db(    irow );

      action_result( stat1, item_act );
   }

   else
   {
qDebug() << " ITEM ACTION: NO";
      action_result( 999, item_act );
   }
}

// perform item details action
void US_ManageData::item_details(  )
{
   const char* rtyps[]  = { "RawData", "EditedData", "Model", "Noise" };
   QString     fileexts = tr( "Text,Log files (*.txt *.log);;" )
      + tr( "All files (*)" );
   int         irow     = tw_item->type() - (int)QTreeWidgetItem::UserType;

   cdesc  = adescs[ irow++ ];  // get description record, index as 1...

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

// browse the database for R/E/M/N data sets, with/without content details
void US_ManageData::browse_dbase( )
{
   QStringList rawIDs;
   QStringList rawGUIDs;
   QStringList edtIDs;
   QStringList modIDs;
   QStringList noiIDs;
   QStringList query;
   QString     invID = QString::number( personID );
   QString     recID;
   QString     rawGUID;
   QString     contents;
   int         irecID;
   int         nraws = 0;
   int         nedts = 0;
   int         nmods = 0;
   int         nnois = 0;
   int         nstep = 20;
   int         istep = 0;

   lb_status->setText( tr( "Reading DataBase Data..." ) );
   progress->setMaximum( nstep );
   ddescs.clear();

   // get raw data IDs
   query.clear();
   query << "all_rawDataIDs" << invID;
   db->query( query );

   while ( db->next() )
   {
      rawIDs << db->value( 0 ).toString();
   }
   progress->setValue( ++istep );

   // get edited data IDs
   query.clear();
   query << "all_editedDataIDs" << invID;
   db->query( query );

   while ( db->next() )
   {
      edtIDs << db->value( 0 ).toString();
   }
   progress->setValue( ++istep );

   // get model IDs
   query.clear();
   query << "get_model_desc" << invID;
   db->query( query );

   while ( db->next() )
   {
      modIDs << db->value( 0 ).toString();
   }
   progress->setValue( ++istep );

   // get noise IDs
   query.clear();
   query << "get_noise_desc" << invID;
   db->query( query );

   while ( db->next() )
   {
      noiIDs << db->value( 0 ).toString();
   }
   progress->setValue( ++istep );
   nraws = rawIDs.size();
   nedts = edtIDs.size();
   nmods = modIDs.size();
   nnois = noiIDs.size();
   nstep = istep + nraws + nedts + nmods + nnois;
   progress->setMaximum( nstep );
qDebug() << "BrDb: kr ke km kn"
 << rawIDs.size() << edtIDs.size() << modIDs.size() << noiIDs.size();

   for ( int ii = 0; ii < nraws; ii++ )
   {  // get raw data information from DB
      recID             = rawIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_rawData" << recID;
      db->query( query );
      db->next();

      //db.readBlobFromDB( fname, "download_aucData", irecID );
              rawGUID   = db->value( 0 ).toString();
      QString label     = db->value( 1 ).toString();
      QString filename  = db->value( 2 ).toString();
      QString comment   = db->value( 3 ).toString();
      QString experID   = db->value( 4 ).toString();
      QString date      = db->value( 6 ).toString();
      QString runID     = filename.section( ".", 0, 0 );
      QString subType   = filename.section( ".", 1, 1 );
              contents  = "";
      QString filebase  = filename.section( "/", -1, -1 );
      rawGUIDs << rawGUID;
//qDebug() << "BrDb:   raw ii id eid rGid label date"
//   << ii << irecID << experID << rawGUID << label << date;

      query.clear();
      query << "get_experiment_info" << experID;
      db->query( query );
      db->next();

      QString expGUID   = db->value( 0 ).toString();
//qDebug() << "BrDb:     raw expGid" << expGUID;

      if ( details )
      {
         QString filetemp = US_Settings::tmpDir() + "/" + filebase;

         db->readBlobFromDB( filetemp, "download_aucData", irecID );

         contents         = md5sum_file( filetemp );
//qDebug() << "BrDb:       (R)contents filetemp" << contents << filetemp;
      }

      cdesc.recordID    = irecID;
      cdesc.recType     = 1;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = rawGUID;
      cdesc.parentGUID  = expGUID;
      cdesc.filename    = filename;
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = ( comment.isEmpty() ) ?
                          filename.section( ".", 0, 2 ) :
                          comment;
      cdesc.lastmodDate = date;

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   for ( int ii = 0; ii < nedts; ii++ )
   {  // get edited data information from DB
      recID             = edtIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_editedData" << recID;
      db->query( query );
      db->next();

      QString rawID     = db->value( 0 ).toString();
      QString editGUID  = db->value( 1 ).toString();
      QString label     = db->value( 2 ).toString();
      QString filename  = db->value( 3 ).toString();
      QString comment   = db->value( 4 ).toString();
      QString date      = db->value( 5 ).toString();
      QString subType   = filename.section( ".", 2, 2 );
              contents  = "";
      QString filebase  = filename.section( "/", -1, -1 );

              rawGUID   = rawGUIDs.at( rawIDs.indexOf( rawID ) );
#if 0
      query.clear();
      query << "get_rawData" << rawID;
      db->query( query );
      db->next();

      QString rawGUID   = db->value( 0 ).toString();
#endif

//qDebug() << "BrDb:     edt ii id eGID rGID label" << ii << irecID << editGUID
//   << rawGUID << label;

      if ( details )
      {
         QString filetemp = US_Settings::tmpDir() + "/" + filebase;

         db->readBlobFromDB( filetemp, "download_editData", irecID );

         contents         = md5sum_file( filetemp );
//qDebug() << "BrDb:       (E)contents filetemp" << contents << filetemp;
      }

      cdesc.recordID    = irecID;
      cdesc.recType     = 2;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = editGUID;
      cdesc.parentGUID  = rawGUID;
      //cdesc.filename    = filename;
      cdesc.filename    = "";
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = ( comment.isEmpty() ) ?
                          filename.section( ".", 0, 2 ) :
                          comment;
      //cdesc.lastmodDate = QFileInfo( aucfile ).lastModified().toString();

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   for ( int ii = 0; ii < nmods; ii++ )
   {  // get model information from DB
      recID             = modIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_model_info" << recID;
      db->query( query );
      db->next();

      QString modelGUID = db->value( 0 ).toString();
      QString descript  = db->value( 1 ).toString();
              contents  = db->value( 2 ).toString();
      QString label     = descript;

      if ( label.length() > 40 )
         label = descript.left( 18 ) + "..." + descript.right( 19 );

      QString subType   = model_type( contents );
      int     jj        = contents.indexOf( "editGUID=" );
      QString editGUID  = ( jj < 1 ) ? "" :
                          contents.mid( jj ).section( QChar( '"' ), 1, 1 );
//qDebug() << "BrDb:       mod ii id mGID dsc"
//   << ii << irecID << modelGUID << descript;

      if ( details )
      {
         QTemporaryFile temporary;
         temporary.open();
         temporary.write( contents.toAscii() );
         temporary.close();

         contents     = md5sum_file( temporary.fileName() );
//qDebug() << "BrDb:         det: cont" << contents;
      }

      else
         contents     = "";

      cdesc.recordID    = irecID;
      cdesc.recType     = 3;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = modelGUID;
      cdesc.parentGUID  = editGUID;
      cdesc.filename    = "";
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = descript;
      //cdesc.lastmodDate = QFileInfo( aucfile ).lastModified().toString();
      cdesc.lastmodDate = "";

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   for ( int ii = 0; ii < nnois; ii++ )
   {  // get noise information from DB
      recID             = noiIDs.at( ii );
      irecID            = recID.toInt();

      query.clear();
      query << "get_noise_info" << recID;
      db->query( query );
      db->next();

      QString noiseGUID = db->value( 0 ).toString();
      QString descript  = db->value( 1 ).toString();
      QString contents  = db->value( 2 ).toString();
      QString filename  = db->value( 3 ).toString();
      QString comment   = db->value( 4 ).toString();
      QString date      = db->value( 5 ).toString();
      QString label     = descript;

      if ( details )
      {
         QTemporaryFile temporary;
         temporary.open();
         temporary.write( contents.toAscii() );
         temporary.close();

         contents     = md5sum_file( temporary.fileName() );
      }

      else
         contents     = "";

      if ( label.length() > 40 )
         label = descript.left( 18 ) + "..." + descript.right( 19 );

      cdesc.recordID    = irecID;
      cdesc.recType     = 4;
      cdesc.subType     = "";
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = noiseGUID;
      //cdesc.parentGUID  = modelGUID;
      cdesc.filename    = filename;
      cdesc.contents    = contents;
      cdesc.label       = label;
      cdesc.description = descript;
      //cdesc.lastmodDate = QFileInfo( aucfile ).lastModified().toString();

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

   progress->setValue( nstep );
   lb_status->setText( tr( "Database Review Complete" ) );
}

// perform a record upload to the database from local disk
int US_ManageData::record_upload( int irow )
{
   int stat = 0;
stat = irow & 3;
stat = 888;
   QStringList query;
   QString filepath = cdesc.filename;
   QString pathdir  = filepath.section( "/",  0, -2 );
   QString filename = filepath.section( "/", -1, -1 );

   if      ( cdesc.recType == 1 )
   {  // upload a Raw record
      US_DataIO2::RawData rdata;

      QString runID    = filename.section( ".",  0,  0 );
      QString tripl    = filename.section( ".", -5, -2 );
      QString fileexp  = filename.section( ".",  0,  1 ) + ".xml";
      QString pathexp  = pathdir + "/" + fileexp;
      QString expID;
      QString expGUID  = "";
qDebug() << "REC_ULD:EXP: runID" << runID << "  tripl" << tripl;

      US_DataIO2::readRawData( filepath, rdata );

      query.clear();
      query << "get_experiment_info_by_runID" << runID;
      db->query( query );
      db->next();
   
      bool    havelexp    = QFile( pathexp ).exists();
      bool    havedexp    = ( db->lastErrno() == US_DB2::OK );
      bool    exp_create  = false;
qDebug() << "REC_ULD:EXP: havelexp havedexp" << havelexp << havedexp;
qDebug() << "REC_ULD:EXP:  dbErrno NOROWS" << db->lastErrno() << US_DB2::NOROWS;

      if ( havedexp )
      {  // experiment exists on the db
         expID     = db->value(  1 ).toString();
         expGUID   = db->value(  2 ).toString();

         if ( !havelexp )
         {  // but it does not exist locally, so create it
            stat = new_experiment_local( db, pathexp );
         }
      }

      else if ( havelexp )
      {  // experiment exists locally, but not on db, so create it
         stat = new_experiment_db( db, pathexp, expID, expGUID );
      }

      else
      {  // experiment does not exist on db or local:  may need to skip update
         QMessageBox msgBox;
         QString     msg;

         // format a message for the warning pop-up
         msg  =
            tr( "No parent Experiment record exists in the database\n" 
                "or on local disk for this Raw record.\n"
                "You may create an experiment now for the runID\n"
                "                \"%1\"\n"
                "using Buffer, Analyte, and Experiment dialogs.\n\n"
                "Do you wish to create a new experiment at this time?\n\n"
                "    Click \"No\"  to simply abort the upload;\n"
                "    Click \"Yes\" to create an experiment record.\n" )
                .arg( runID );
         msgBox.setWindowTitle( tr( "No Existing Parent Experiment" ) );
         msgBox.setText( msg );
         msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
         msgBox.setDefaultButton( QMessageBox::No );
         exp_create  = ( msgBox.exec() == QMessageBox::Yes );
         stat        = 30000;
      }

      if ( exp_create )
      {
         US_ExpInfo::ExperimentInfo expdata;
         US_ExpInfo::TripleInfo     triple;

         int centerpiece = 0;
         int projID      = 0;
         int insID       = 0;
         int operID      = 0;
         int rotorID     = 1;
         int labID       = 0;

         double  avgTemp = 0.0;

         QString projGUID;
         QString invGUID;
         QString labGUID;
         QString insSerial;
         QString expType       = "velocity";
         QString operGUID;
         QString optSystem     = filename.section( ".", 1, 1 );
         QString firstName     = investig.section( ":", 1, 1 )
                                         .section( ",", 1, 1 ).simplified();
         QString lastName      = investig.section( ":", 1, 1 )
                                         .section( ",", 0, 0 ).simplified();
         QString centrifuge;

         for ( int jj = 0; jj < rdata.scanData.size(); jj++ )
         {
            avgTemp += rdata.scanData[ jj ].temperature;
         }
         avgTemp /= (double)( rdata.scanData.size() );

         expdata.triples.clear();
         expdata.rpms   .clear();

         expdata.invID              = personID;
         expdata.invGUID            = invGUID;
         expdata.lastName           = lastName;
         expdata.firstName          = firstName;
         expdata.expID              = expID.toInt();
         expdata.expGUID            = expGUID;
         expdata.projectID          = projID;
         expdata.runID              = runID;
         expdata.labID              = labID;
         expdata.labGUID            = labGUID;
         expdata.instrumentID       = insID;
         expdata.instrumentSerial   = insSerial;
         expdata.operatorID         = operID;
         expdata.operatorGUID       = operGUID;
         expdata.rotorID            = rotorID;
         expdata.expType            = expType;
         expdata.opticalSystem      = optSystem.toAscii();
         expdata.runTemp            = QString::number( avgTemp );
         expdata.label              = runID;
         expdata.comments           = runID + " experiment";
         expdata.centrifugeProtocol = centrifuge;
         expdata.date               = QDateTime::currentDateTime().toString();

         double grpm = rdata.scanData[ 0 ].rpm; // greatest rpm so far
         double crpm;
         expdata.rpms << grpm;                  // first rpm list value

         for ( int jj = 1; jj < rdata.scanData.size(); jj++ )
         {  // loop to add unique rpms to a list, sorting as we go
            crpm   = rdata.scanData[ jj ].rpm;

            if ( crpm == grpm )
               continue;     // if equal to greatest, no need to add it

            else if ( crpm > grpm )
            {  // this rpm is greater than any so far:  append it
               expdata.rpms << crpm;
               grpm    = crpm;         // set new greatest
            }

            else if ( crpm < grpm )
            {  // this rpm is less than greatest, find where to insert it

               for ( int kk = 0; kk < expdata.rpms.size(); kk++ )
               {
                  if ( crpm == expdata.rpms.at( kk ) )
                     break;   // break from insert loop if equal to one in list

                  if ( crpm < expdata.rpms.at( kk ) )
                  {  // found list value we are less than, so insert before it
                     expdata.rpms.insert( kk, crpm );
                     break;
                  }
               }
            }
         }

         buffer.personID       = personID;
         buffer.person         = firstName + " " + lastName;

         analyte.invID         = personID;

         US_BufferGui*  budiag =
            new US_BufferGui(  expdata.invID, true, buffer, false );
         connect( budiag, SIGNAL( valueChanged(  US_Buffer  ) ),
                  this,   SLOT  ( assignBuffer(  US_Buffer  ) ) );
         budiag->exec();

         US_AnalyteGui* andiag =
            new US_AnalyteGui( expdata.invID, true, "", true, avgTemp );
         connect( andiag, SIGNAL( valueChanged(  US_Analyte ) ),
                  this,   SLOT  ( assignAnalyte( US_Analyte ) ) );
         andiag->exec();

         triple.centerpiece    = centerpiece;
         triple.bufferID       = buffer.bufferID.toInt();
         triple.bufferGUID     = buffer.GUID;
         triple.bufferDesc     = buffer.description;
         triple.analyteID      = analyte.analyteID.toInt();
         triple.analyteGUID    = analyte.analyteGUID;
         triple.analyteDesc    = analyte.description;
         triple.tripleFilename = filename;
qDebug() << "triple: center  " << triple.centerpiece;
qDebug() << "  b.bufferID    " << triple.bufferID;
qDebug() << "  b.bufferGUID  " << triple.bufferGUID;
qDebug() << "  b.bufferDesc  " << triple.bufferDesc;
qDebug() << "  a.analyteID   " << triple.analyteID;
qDebug() << "  a.analyteGUID " << triple.analyteGUID;
qDebug() << "  a.analyteDesc " << triple.analyteDesc;

         expdata.triples << triple;

         US_ExpInfo*    eidiag =
            new US_ExpInfo(    expdata, false );
         eidiag->exec();

//*DEBUG*
if ( exp_create ) return 40000;
//*DEBUG*

         stat        = 20000;
      }

      if ( stat == 0 )
      {  // all ok with experiment:  proceed with uploading Raw record
         cdesc.parentGUID  = expGUID;

         query.clear();
         query << "new_rawData"
            << cdesc.dataGUID
            << cdesc.label
            << cdesc.filename
            << cdesc.description
            << expID
            << "1"; // channel ID

         int dbstat = db->statusQuery( query );
         int irecID = db->lastInsertID();
         if ( dbstat == US_DB2::OK )
         {
            dbstat     = db->writeBlobToDB( filepath,
                                            QString( "upload_aucData" ),
                                            irecID );
            if ( dbstat == US_DB2::ERROR )
            {
               qDebug() << "*ERROR* processing" << filepath;
               stat        = 40000;
            }
            else
            {
               qDebug() << "+SUCCESS+ loading" << filepath;
            }
         }
      }
   }

   else if ( cdesc.recType == 2 )
   {  // upload an EditedData record
   }

   else if ( cdesc.recType == 3 )
   {  // upload a Model record
   }

   else if ( cdesc.recType == 4 )
   {  // upload a Noise record
   }

   else
   {  // *ERROR*:  invalid type
      stat        = 10000;
   }

   return stat;
}
// perform a record download from the database to local disk
int US_ManageData::record_download( int irow )
{
   int stat = 0;
stat = irow & 3;
stat = 888;
   return stat;
}
// perform a record remove from the database
int US_ManageData::record_remove_db( int irow )
{
   int stat = 0;
stat = irow & 3;
stat = 888;
   return stat;
}

// perform a record remove from local disk
int US_ManageData::record_remove_local( int irow )
{
   int stat = 0;
   QString filepath = cdesc.filename;
   QString filename = filepath.section( "/", -1, -1 );
   QFile   file( filepath );
   QBrush  fbru( colorGray );

   if ( file.exists() )
   {  // the file to remove does exist

      if ( file.remove() )
      {  // the remove was successful

         if ( ( cdesc.recState & REC_DB ) == 0 )
         {  // it was local-only, so now it's a dummy
            cdesc.recState  = NOSTAT;
         }

         else
         {  // it was on both, so now it's db-only
            cdesc.recState &= ~REC_LO;
            fbru            = QBrush( colorBlue );
         }

         for ( int jj = 0; jj < ntcols; jj++ )
         {  // re-color row to reflect its new state
            tw_item->setForeground( jj, fbru ); 
         }

         // update the current description record in the vector
         adescs[ irow ] = cdesc;
      }

      else
      {  // an error occurred in removing
         stat     = 1000;
         qDebug() << "*ERROR* removing row file " << filename
            << "  (row" << irow << ")";
      }
   }

   else
   {  // file did not exist
      stat     = 2000;
      qDebug() << "*ERROR* attempt to remove non-existent file " << filename
         << "  (row" << irow << ")";
   }

   return stat;
}

// browse the local disk for R/E/M/N data sets, with/without content details
void US_ManageData::browse_local( )
{
   // start with AUC (raw) and edit files in directories of resultDir
   QString     rdir     = US_Settings::resultDir();
   QString     ddir     = US_Settings::dataDir();
   QString     dirm     = ddir + "/models";
   QString     dirn     = ddir + "/noises";
   QString     contents = "";
   QStringList aucdirs  = QDir( rdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   QStringList aucfilt;
   QStringList edtfilt;
   QStringList modfilt( "M*xml" );
   QStringList noifilt( "N*xml" );
   QStringList modfils = QDir( dirm )
      .entryList( modfilt, QDir::Files, QDir::Name );
   QStringList noifils = QDir( dirn )
      .entryList( noifilt, QDir::Files, QDir::Name );
   int         ktask   = 0;
   int         naucd   = aucdirs.size();
   int         nmodf   = modfils.size();
   int         nnoif   = noifils.size();
   int         nstep   = naucd * 4 + nmodf + nnoif;
qDebug() << "BrLoc:  nau nmo nno nst" << naucd << nmodf << nnoif << nstep;
   aucfilt << "*.auc";
   edtfilt << "*.xml";
   rdir    = rdir + "/";
   lb_status->setText( tr( "Reading Local-Disk Data..." ) );
   progress->setMaximum( nstep );

   for ( int ii = 0; ii < naucd; ii++ )
   {  // loop thru potential data directories
      QString     subdir   = rdir + aucdirs.at( ii );
      QStringList aucfiles = QDir( subdir )
         .entryList( aucfilt, QDir::Files, QDir::Name );
      int         naucf    = aucfiles.size();
      US_DataIO2::RawData    rdata;
      US_DataIO2::EditValues edval;

      for ( int jj = 0; jj < naucf; jj++ )
      {  // loop thru .auc files found in a directory
         QString fname    = aucfiles.at( jj );
         QString runid    = fname.section( ".", 0, 0 );
         QString tripl    = fname.section( ".", -5, -2 );
         QString aucfile  = subdir + "/" + fname;
         QString descr    = "";
         QString rawGUID  = "";
                 contents = "";
//qDebug() << "BrLoc: ii jj file" << ii << jj << aucfile;

         // read in the raw data and build description record
         US_DataIO2::readRawData( aucfile, rdata );

         if ( details )
         {
            contents     = md5sum_file( aucfile );
//qDebug() << "BrLoc:      contents" << contents;
         }

         char uuid[ 37 ];
         uuid_unparse( (uchar*)rdata.rawGUID, uuid );

         cdesc.recordID    = -1;
         cdesc.recType     = 1;
         cdesc.subType     = fname.section( ".", 1, 1 );
         cdesc.recState    = REC_LO;
         cdesc.dataGUID    = QString( uuid );
         cdesc.parentGUID  = "0";
         cdesc.filename    = aucfile;
         cdesc.contents    = contents;
         cdesc.label       = runid + "." + tripl;
         cdesc.description = rdata.description;
         cdesc.lastmodDate = QFileInfo( aucfile ).lastModified().toString();

         ldescs << cdesc;

         // now load edit files associated with this auc file
         edtfilt.clear();
         edtfilt << runid + ".*." + tripl + ".xml";
//qDebug() << "BrLoc:  edtfilt" << edtfilt;

         QStringList edtfiles = QDir( subdir )
            .entryList( edtfilt, QDir::Files, QDir::Name );

         for ( int kk = 0; kk < edtfiles.size(); kk++ )
         {
            QString efname   = edtfiles.at( kk );
            QString editid   = efname.section( ".", 1, 3 );
            QString edtfile  = subdir + "/" + efname;
                    contents = "";
//qDebug() << "BrLoc:    kk file" << kk << edtfile;

            // read EditValues for the edit data and build description record
            US_DataIO2::readEdits( edtfile, edval );

            if ( details )
            {
               contents     = md5sum_file( edtfile );
//qDebug() << "BrLoc:      (E)contents edtfile" << contents << edtfile;
            }

            cdesc.recordID    = -1;
            cdesc.recType     = 2;
            cdesc.subType     = efname.section( ".", 2, 2 );
            cdesc.recState    = REC_LO;
            cdesc.dataGUID    = edval.editGUID;
            cdesc.parentGUID  = edval.dataGUID;
            cdesc.filename    = edtfile;
            cdesc.contents    = contents;
            cdesc.label       = runid + "." + editid;
            cdesc.description = cdesc.label;
            cdesc.lastmodDate = QFileInfo( edtfile ).lastModified().toString();

            ldescs << cdesc;
         }
         if ( ii == ( naucd / 2 )  &&  jj == ( naucf / 2 ) )
            progress->setValue( ++ktask );
      }
      progress->setValue( ++ktask );
   }
   progress->setValue( ++ktask );

   for ( int ii = 0; ii < nmodf; ii++ )
   {  // loop thru potential model files
      US_Model    model;
      QString     modfil   = dirm + "/" + modfils.at( ii );
                  contents = "";

      model.load( modfil );

      if ( details )
      {
         contents     = md5sum_file( modfil );
      }

      cdesc.recordID    = -1;
      cdesc.recType     = 3;
      cdesc.subType     = model_type( model );
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = model.modelGUID;
      cdesc.parentGUID  = model.editGUID;
      cdesc.filename    = modfil;
      cdesc.contents    = contents;
      cdesc.description = model.description;
      cdesc.lastmodDate = QFileInfo( modfil ).lastModified().toString();

      if ( model.description.length() < 41 )
         cdesc.label       = model.description;
      else
         cdesc.label       = model.description.left( 18 )  + "..."
                           + model.description.right( 19 );

      if ( details )
      {
      }

      ldescs << cdesc;

      progress->setValue( ++ktask );
   }

   for ( int ii = 0; ii < nnoif; ii++ )
   {  // loop thru potential noise files
      US_Noise    noise;
      QString     noifil   = dirn + "/" + noifils.at( ii );
                  contents = "";

      noise.load( noifil );

      if ( details )
      {
         contents     = md5sum_file( noifil );
      }

      cdesc.recordID    = -1;
      cdesc.recType     = 4;
      cdesc.subType     = ( noise.type == US_Noise::RI ) ? "RI" : "TI";
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = noise.noiseGUID;
      cdesc.parentGUID  = noise.modelGUID;
      cdesc.filename    = noifil;
      cdesc.contents    = contents;
      cdesc.description = noise.description;
      cdesc.lastmodDate = QFileInfo( noifil ).lastModified().toString();

      if ( noise.description.length() < 41 )
         cdesc.label       = noise.description;
      else
         cdesc.label       = noise.description.left( 18 )  + "..."
                           + noise.description.right( 19 );

      ldescs << cdesc;

      progress->setValue( ++ktask );
   }

   progress->setValue( nstep );
   lb_status->setText( tr( "Local Data Review Complete" ) );
}

// merge the database and local description vectors into a single combined
void US_ManageData::merge_dblocal( )
{
   int nddes = ddescs.size();
   int nldes = ldescs.size();
   int nstep = ( ( nddes + nldes ) * 5 ) / 8;

   int jdr   = 0;
   int jlr   = 0;
   int kar   = 1;

   DataDesc  descd = ddescs.at( 0 );
   DataDesc  descl = ldescs.at( 0 );

   lb_status->setText( tr( "Merging Data ..." ) );
   progress->setMaximum( nstep );

   while ( jdr < nddes  &&  jlr < nldes )
   {  // main loop to merge records until one is exhausted

      progress->setValue( kar );           // report progress

      if ( kar > nstep )
      {  // if count beyond max, bump max by one eighth
         nstep = ( kar * 9 ) / 8;
         progress->setMaximum( nstep );
      }

      while ( descd.dataGUID == descl.dataGUID )
      {  // records match in GUID:  merge them into one
         descd.recState    |= descl.recState;     // OR states
         descd.filename     = descl.filename;     // filename from local
         descd.lastmodDate  = descl.lastmodDate;  // last mod date from local

         if ( details )
         {
            descd.contents     = descd.contents + " " + descl.contents;
         }

         adescs << descd;                  // output combo record
//qDebug() << "MERGE:  kar jdr jlr (1)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr >= nddes )             // bump db count and test if done
         {
            if ( ++jlr >= nldes )
               break;
            descl = ldescs.at( jlr );      // get next local record

            break;
         }

         descd = ddescs.at( jdr );         // get next db record

         if ( ++jlr >= nldes )             // bump local count and test if done
            break;

         descl = ldescs.at( jlr );         // get next local record
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      while ( descd.recType > descl.recType )
      {  // output db records that are left-over children
         adescs << descd;
//qDebug() << "MERGE:  kar jdr jlr (2)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr >= nddes )
            break;

         descd = ddescs.at( jdr );
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      while ( descl.recType > descd.recType )
      {  // output local records that are left-over children
         adescs << descl;
//qDebug() << "MERGE:  kar jdr jlr (3)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if ( ++jlr >= nldes )
            break;

         descl = ldescs.at( jlr );
      }

      if ( jdr >= nddes  ||  jlr >= nldes )
         break;

      // If we've reached another matching pair or if we are not at
      // the same level, go back up to the start of the main loop.
      if ( descd.dataGUID == descl.dataGUID  ||
           descd.recType  != descl.recType  )
         continue;

      // If we are here, we have records at the same level,
      // but with different GUIDs. Output one of them, based on
      // an alphanumeric comparison of label values.

      if ( descd.label < descl.label )
      {  // output db record first based on alphabetic label sort
         adescs << descd;
//qDebug() << "MERGE:  kar jdr jlr (4)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if ( ++jdr >= nddes )
            break;

         descd = ddescs.at( jdr );
      }

      else
      {  // output local record first based on alphabetic label sort
         adescs << descl;
//qDebug() << "MERGE:  kar jdr jlr (5)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if ( ++jlr >= nldes )
            break;

         descl = ldescs.at( jlr );
      }

   }  // end of main merge loop;

   // after breaking from main loop, output any records left from one
   // source (db/local) or the other.
   nstep += ( nddes - jdr + nldes - jlr );
   progress->setMaximum( nstep );

   while ( jdr < nddes )
   {
      adescs << ddescs.at( jdr++ );
//descd=ddescs.at(jlr-1);
//qDebug() << "MERGE:  kar jdr jlr (8)GID" << kar << jdr << jlr << descd.dataGUID;
      kar++;
      progress->setValue( kar );
   }

   while ( jlr < nldes )
   {
      adescs << ldescs.at( jlr++ );
//descl=ldescs.at(jlr-1);
//qDebug() << "MERGE:  kar jdr jlr (9)GID" << kar << jdr << jlr << descl.dataGUID;
      kar++;
      progress->setValue( kar );
   }

//qDebug() << "MERGE: nddes nldes kar" << nddes << nldes << --kar;
//qDebug() << " a/d/l sizes" << adescs.size() << ddescs.size() << ldescs.size();

   progress->setValue( nstep );
   lb_status->setText( tr( "Data Merge Complete" ) );
}

// sort a data-set description vector
void US_ManageData::sort_descs( QVector< DataDesc >& descs )
{
   QVector< DataDesc > tdess;                 // temporary descr. vector
   DataDesc            desct;                 // temporary descr. entry
   QStringList         sortr;                 // sort string lists
   QStringList         sorte;
   QStringList         sortm;
   QStringList         sortn;
   int                 nrecs = descs.size();  // number of descr. records

   if ( nrecs == 0 )
      return;

   tdess.resize( nrecs );
//qDebug() << "SortD: nrecs" << nrecs;

   for ( int ii = 0; ii < nrecs; ii++ )
   {  // build sort strings for Raw,Edit,Model,Noise; copy unsorted vector
      desct        = descs[ ii ];

      if ( desct.recType == 1 )
         sortr << sort_string( desct, ii );

      else if ( desct.recType == 2 )
         sorte << sort_string( desct, ii );

      else if ( desct.recType == 3 )
         sortm << sort_string( desct, ii );

      else if ( desct.recType == 4 )
         sortn << sort_string( desct, ii );

      tdess[ ii ]  = desct;
   }
//qDebug() << "SortD: build done";

   // sort the string lists for each type
   sortr.sort();
   sorte.sort();
   sortm.sort();
   sortn.sort();
//qDebug() << "SortD: sort done";

   // review each type for duplicate GUIDs
   if ( review_descs( sortr, tdess ) )
      return;
   if ( review_descs( sorte, tdess ) )
      return;
   if ( review_descs( sortm, tdess ) )
      return;
   if ( review_descs( sortn, tdess ) )
      return;
//qDebug() << "SortD: review done";

   // create list of noise orphans
   QStringList orphn = list_orphans( sortn, sortm );
   // create list of model orphans
   QStringList orphm = list_orphans( sortm, sorte );
   // create list of edit orphans
   QStringList orphe = list_orphans( sorte, sortr );

   QString dmyGUID = "00000000-0000-0000-0000-000000000000";
   QString dmyBGID = dmyGUID.left( 32 );
   QString dsorts;
   QString dlabel;
   QString dindex;
   QString ddGUID;
   QString dpGUID;
   QString ppGUID;
   int kndx = tdess.size();
   int jndx;
   int kk;
   int ndmy = 0;     // flag of duplicate dummies

   // create dummy records to parent each orphan

   for ( int ii = 0; ii < orphn.size(); ii++ )
   {  // for each orphan noise, create a dummy model
      dsorts = orphn.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 );
      dpGUID = dsorts.section( ":", 3, 3 );
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 2 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sortn.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sortn.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sortm ) >= 0 )
         continue;

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 3;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = QDateTime::currentDateTime().toString();

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortm << dsorts;
      orphm << dsorts;
      tdess.append( cdesc );
//qDebug() << "N orphan:" << orphn.at( ii );
//qDebug() << "  M dummy:" << dsorts;
   }

   ndmy   = 0;

   for ( int ii = 0; ii < orphm.size(); ii++ )
   {  // for each orphan model, create a dummy edit
      dsorts = orphm.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 );
      dpGUID = dsorts.section( ":", 3, 3 );
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 16 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sortm.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sortm.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sorte ) >= 0 )
         continue;

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 2;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = QDateTime::currentDateTime().toString();

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sorte << dsorts;
      orphe << dsorts;
      tdess.append( cdesc );
qDebug() << "M orphan:" << orphm.at( ii );
qDebug() << "  E dummy:" << dsorts;
   }

   ndmy   = 0;

   for ( int ii = 0; ii < orphe.size(); ii++ )
   {  // for each orphan edit, create a dummy raw
      dsorts = orphe.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 );
      dpGUID = dsorts.section( ":", 3, 3 );
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 2 )
      { // handle case where there is no valid parentGUID
         if ( ndmy == 0 )      // first time:  create one
            dpGUID = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
         else
            dpGUID = ppGUID;   // afterwards:  re-use same parent

         kk     = sorte.indexOf( dsorts );  // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if ( kk >= 0 )
         {  // replace present record for new parentGUID
            sorte.replace( kk, dsorts );
            cdesc.parentGUID  = dpGUID;
            tdess[ jndx ]     = cdesc;
         }

         if ( ndmy > 0 )       // after 1st time, skip creating new parent
            continue;

         ndmy++;               // flag that we have a parent for invalid ones
         ppGUID = dpGUID;      // save the GUID for new dummy parent
      }

      // if this record is no longer an orphan, skip creating new parent
      if ( index_substring( dpGUID, 2, sortr ) >= 0 )
         continue;

      cdesc.parentID    = cdesc.recordID;
      cdesc.recordID    = -1;
      cdesc.recType     = 1;
      cdesc.subType     = "";
      cdesc.recState    = NOSTAT;
      cdesc.dataGUID    = dpGUID;
      cdesc.parentGUID  = dmyBGID + QString().sprintf( "%4.4d", kdmy++ );
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = cdesc.label.section( ".", 0, 0 );
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = QDateTime::currentDateTime().toString();

      dlabel = dlabel.section( ".", 0, 0 );
      dindex = QString().sprintf( "%4.4d", kndx++ );
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortr << dsorts;
      tdess.append( cdesc );
qDebug() << "E orphan:" << orphe.at( ii );
qDebug() << "  R dummy:" << dsorts;
   }

//for ( int ii = 0; ii < sortr.size(); ii++ )
// qDebug() << "R entry:" << sortr.at( ii );
   int countR = sortr.size();    // count of each kind in sorted lists
   int countE = sorte.size();
   int countM = sortm.size();
   int countN = sortn.size();

   sortr.sort();                 // re-sort for dummy additions
   sorte.sort();
   sortm.sort();
   sortn.sort();
qDebug() << "sort/dumy: count REMN" << countR << countE << countM << countN;

   int noutR  = 0;               // count of each kind in hierarchical output
   int noutE  = 0;
   int noutM  = 0;
   int noutN  = 0;
   int indx;
   int pstate = REC_LO | PAR_LO;

   descs.clear();                // reset input vector to become sorted output

   // rebuild the description vector with sorted trees

   for ( int ii = 0; ii < countR; ii++ )
   {  // loop to output sorted Raw records
      QString recr   = sortr[ ii ];
      QString didr   = recr.section( ":", 2, 2 );
      QString pidr   = recr.section( ":", 3, 3 );

      indx           = recr.section( ":", 1, 1 ).toInt();

      cdesc          = tdess.at( indx );

      // set up a default parent state flag
      pstate = cdesc.recState;
      pstate = ( pstate & REC_DB ) != 0 ? ( pstate | PAR_DB ) : pstate;
      pstate = ( pstate & REC_LO ) != 0 ? ( pstate | PAR_LO ) : pstate;

      // new state is the default,  or NOSTAT if this is a dummy record
      cdesc.recState = record_state_flag( cdesc, pstate );

      descs << cdesc;                   // output Raw rec
      noutR++;

      // set up parent state for children to follow
      int rpstate    = cdesc.recState;

      for ( int jj = 0; jj < countE; jj++ )
      {  // loop to output sorted Edit records for the above Raw
         QString rece   = sorte[ jj ];
         QString pide   = rece.section( ":", 3, 3 );

         if ( pide != didr )            // skip if current Raw not parent
            continue;

         QString dide   = rece.section( ":", 2, 2 );
         indx           = rece.section( ":", 1, 1 ).toInt();
         cdesc          = tdess.at( indx );
         cdesc.recState = record_state_flag( cdesc, rpstate );
//qDebug() << "     ii jj indx" << ii << jj << indx;

         descs << cdesc;                // output Edit rec
         noutE++;

         // set up parent state for children to follow
         int epstate    = cdesc.recState;

         for ( int mm = 0; mm < countM; mm++ )
         {  // loop to output sorted Model records for above Edit
            QString recm   = sortm[ mm ];
            QString pidm   = recm.section( ":", 3, 3 );

            if ( pidm != dide )         // skip if current Edit not parent
               continue;

            QString didm   = recm.section( ":", 2, 2 );
            indx           = recm.section( ":", 1, 1 ).toInt();
            cdesc          = tdess.at( indx );
            cdesc.recState = record_state_flag( cdesc, epstate );

            descs << cdesc;             // output Model rec

            noutM++;

            // set up parent state for children to follow
            int mpstate    = cdesc.recState;

            for ( int nn = 0; nn < countN; nn++ )
            {  // loop to output sorted Noise records for above Model
               QString recn   = sortn[ nn ];
               QString pidn   = recn.section( ":", 3, 3 );

               if ( pidn != didm )      // skip if current Model not parent
                  continue;

               indx           = recn.section( ":", 1, 1 ).toInt();
               cdesc          = tdess.at( indx );
               cdesc.recState = record_state_flag( cdesc, mpstate );

               descs << cdesc;          // output Noise rec

               noutN++;
            }
         }
      }
   }

   if ( noutR != countR  ||  noutE != countE  ||
        noutM != countM  ||  noutN != countN )
   {  // not all accounted for, so we will need some dummy parents
      qDebug() << "sort_desc: count REMN"
         << countR << countE << countM << countN;
      qDebug() << "sort_desc: nout REMN"
         << noutR << noutE << noutM << noutN;
   }
}

// compose concatenation on which to sort (label:index:dataGUID:parentGUID)
QString US_ManageData::sort_string( DataDesc ddesc, int indx )
{  // create string for ascending sort on label
   QString ostr = ddesc.label                        // label to sort on
      + ":"     + QString().sprintf( "%4.4d", indx ) // index in desc. vector
      + ":"     + ddesc.dataGUID                     // data GUID
      + ":"     + ddesc.parentGUID;                  // parent GUID
   return ostr;
}

// compose string describing model type
QString US_ManageData::model_type( int imtype, int nassoc, int niters )
{
   QString mtype;

   // format the base model type string
   switch ( imtype )
   {
      default:
      case (int)US_Model::TWODSA:
      case (int)US_Model::MANUAL:
         mtype = "2DSA";
         break;
      case (int)US_Model::TWODSA_MW:
         mtype = "2DSA-MW";
         break;
      case (int)US_Model::GA:
      case (int)US_Model::GA_RA:
         mtype = "GA";
         break;
      case (int)US_Model::GA_MW:
         mtype = "GA-MW";
         break;
      case (int)US_Model::COFS:
         mtype = "COFS";
         break;
      case (int)US_Model::FE:
         mtype = "FE";
         break;
      case (int)US_Model::GLOBAL:
         mtype = "GLOBAL";
         break;
      case (int)US_Model::ONEDSA:
         mtype = "1DSA";
         break;
   }

   // add RA for Reversible Associations (if associations count > 1)
   if ( nassoc > 1 )
      mtype = mtype + "-RA";

   // add MC for Monte Carlo (if iterations count > 1)
   if ( niters > 1 )
      mtype = mtype + "-MC";

   return mtype;
}

// compose string describing model type
QString US_ManageData::model_type( US_Model model )
{
   // return model type string based on integer flags in the model object
   return model_type( (int)model.type,
                      model.associations.size(),
                      model.iterations );
}

// compose string describing model type
QString US_ManageData::model_type( QString modxml )
{
   QChar quo( '"' );
   int   jj;
   int   imtype;
   int   nassoc;
   int   niters;

   // model type number from type attribute
   jj       = modxml.indexOf( " type=" );
   imtype   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // count of associations is count of k_eq attributes present
   nassoc   = modxml.count( "k_eq=" );

   // number of iterations from iterations attribute value
   jj       = modxml.indexOf( " iterations=" );
   niters   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   // return model type string based on integer flags
   return model_type( imtype, nassoc, niters );
}

// compose string describing record state
QString US_ManageData::record_state( int istate )
{
   QString hexn    = QString().sprintf( "0x%3.3x", istate );

   QString flags   = "NOSTAT";         // by default, no state

   if ( istate & REC_DB )
      flags  = flags + "|REC_DB";      // record exists in db

   if ( istate & REC_LO )
      flags  = flags + "|REC_LO";      // record exists locally

   if ( istate & PAR_DB )
      flags  = flags + "|PAR_DB";      // parent exists in db

   if ( istate & PAR_LO )
      flags  = flags + "|PAR_LO";      // parent exists locally

   if ( istate & HV_DET )
      flags  = flags + "|DETAIL";      // content details are supplied

   if ( istate & IS_CON )
      flags  = flags + "|CONSIS";      // record is consistent in db+local

   if ( istate & ALL_OK )
      flags  = flags + "|ALL_OK";      // record is ok in all respects

   if ( flags != "NOSTAT" )
      flags  = flags.mid( 7, 999 );    // remove any "NOSTAT|"

    return "(" + hexn + ") " + flags;  // return hex flag and text version
}

// review sorted string lists for duplicate GUIDs
bool US_ManageData::review_descs( QStringList& sorts,
      QVector< DataDesc >& descv )
{
   bool           abort = false;
   int            nrecs = sorts.size();
   int            nmult = 0;
   int            kmult = 0;
   int            ityp;
   QString        cGUID;
   QString        pGUID;
   QString        rtyp;
   QVector< int > multis;
   const char* rtyps[] = { "RawData", "EditedData", "Model", "Noise" };

   if ( nrecs < 1 )
      return abort;

   int ii = sorts[ 0 ].section( ":", 1, 1 ).toInt();
   ityp   = descv[ ii ].recType;
   rtyp   = QString( rtyps[ ityp - 1 ] );

   if ( descv[ ii ].recordID >= 0 )
      rtyp   = "DB " + rtyp;
   else
      rtyp   = "Local " + rtyp;
qDebug() << "RvwD: ii ityp rtyp nrecs" << ii << ityp << rtyp << nrecs;

   for ( int ii = 1; ii < nrecs; ii++ )
   {  // do a pass to determine if there are duplicate GUIDs
      cGUID    = sorts[ ii ].section( ":", 2, 2 );     // current rec GUID
      kmult    = 0;                                    // flag no multiples yet

      for ( int jj = 0; jj < ii; jj++ )
      {  // review all the records preceeding this one
         pGUID    = sorts[ jj ].section( ":", 2, 2 );  // a previous GUID

         if ( pGUID == cGUID )
         {  // found a duplicate
            kmult++;

            if ( ! multis.contains( jj ) )
            {  // not yet marked, so mark previous as multiple
               multis << jj;    // save index
               nmult++;         // bump count
            }

            else  // if it was marked, we can quit the inner loop
               break;
         }
      }

      if ( kmult > 0 )
      {  // this pass found a duplicate:  save the index and bump count
         multis << ii;
         nmult++;
      }
//qDebug() << "RvwD:   ii kmult nmult" << ii << kmult << nmult;
   }

qDebug() << "RvwD:      nmult" << nmult;
   if ( nmult > 0 )
   {  // there were multiple instances of the same GUID
      QMessageBox msgBox;
      QString     msg;

      // format a message for the warning pop-up
      msg  =
         tr( "There are %1 %2 records that have\n" ).arg( nmult ).arg( rtyp ) +
         tr( "the same GUID as another.\n" ) +
         tr( "You should correct the situation before proceeding.\n" ) +
         tr( "  Click \"Ok\" to see details, then abort.\n" ) +
         tr( "  Click \"Ignore\" to proceed to further review.\n" );
      msgBox.setWindowTitle( tr( "Duplicate %1 Records" ).arg( rtyp ) );
      msgBox.setText( msg );
      msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Ignore );
      msgBox.setDefaultButton( QMessageBox::Ok );

      if ( msgBox.exec() == QMessageBox::Ok )
      {  // user wants details, so display them
         QString fileexts = tr( "Text,Log files (*.txt *.log);;" )
            + tr( "All files (*)" );
         QString pGUID = "";
         QString cGUID;
         QString label;

         msg =
            tr( "Review the details below on duplicate records.\n" ) +
            tr( "Save or Print the contents of this message.\n" ) +
            tr( "Decide which of the duplicates should be removed.\n" ) +
            tr( "Close the main US_ManageData window after exiting here.\n" ) +
            tr( "\nSummary of Duplicates:\n\n" );

         for ( int ii = 0; ii < nmult; ii++ )
         {  // add summary lines on duplicates
            int jj = multis.at( ii );
            cGUID  = sorts.at( jj ).section( ":", 2, 2 );
            label  = sorts.at( jj ).section( ":", 0, 0 );

            if ( cGUID != pGUID )
            {  // first instance of this GUID:  show GUID
               msg  += tr( "GUID:  " ) + cGUID + "\n";
               pGUID = cGUID;
            }

            // one label line for each multiple
            msg  += tr( "  Label:  " ) + label + "\n";
         }

         msg += tr( "\nDetails of Duplicates:\n\n" );

         for ( int ii = 0; ii < nmult; ii++ )
         {  // add detail lines
            int jj = multis.at( ii );
            cGUID  = sorts.at( jj ).section( ":", 2, 2 );
            pGUID  = sorts.at( jj ).section( ":", 3, 3 );
            label  = sorts.at( jj ).section( ":", 0, 0 );
            int kk = sorts.at( jj ).section( ":", 1, 1 ).toInt();
            cdesc  = descv[ kk ];

            msg   += tr( "GUID:  " ) + cGUID + "\n" +
               tr( "  ParentGUID:  " ) + pGUID + "\n" +
               tr( "  Label:  " ) + label + "\n" +
               tr( "  Description:  " ) + cdesc.description + "\n" +
               tr( "  DB record ID:  %1" ).arg( cdesc.recordID ) + "\n" +
               tr( "  File Directory:  " ) +
               cdesc.filename.section( "/",  0, -2 ) + "\n" +
               tr( "  File Name:  " ) +
               cdesc.filename.section( "/", -1, -1 ) + "\n" +
               tr( "  Last Mod Date:  " ) +
               cdesc.lastmodDate + "\n";
         }

         // pop up text dialog
         US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
         editd->setWindowTitle( tr( "Data Set Duplicate GUID Details" ) );
         editd->move( QCursor::pos() + QPoint( 200, 200 ) );
         editd->resize( 600, 500 );
         editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
         editd->e->setText( msg );
         editd->show();

         abort = true;      // tell caller to abort data tree build
      }

      else
      {
         abort = false;     // signal to proceed with data tree build
      }
   }
qDebug() << "review_descs   abort" << abort;

   return abort;
}

// find index of substring at given position in strings of string list
int US_ManageData::index_substring( QString ss, int ixs, QStringList& sl )
{
   QString sexp;
   QRegExp rexp;
   int     kndx = -1;

   if ( ixs == 0 )
   {  // find label at beginning of strings in list
      sexp = ss + ":*";
   }

   else if ( ixs == 1  ||  ixs == 2 )
   {  // find RecIndex or recGUID in middle of strings in list
      sexp = "*:" + ss + ":*";
   }

   else if ( ixs == 3 )
   {  // find parentGUID at end of strings in list
      sexp = "*:" + ss;
   }

   rexp = QRegExp( sexp, Qt::CaseSensitive, QRegExp::Wildcard );
   kndx = sl.indexOf( rexp );
//qDebug() << "     index_sub kndx ixs search" << kndx << ixs << sexp;
   return kndx;
}

// get sublist from string list of substring matches at a given string position
QStringList US_ManageData::filter_substring( QString ss, int ixs,
   QStringList& sl )
{
   QStringList subl;

   if ( ixs == 0 )
   {  // match label at beginning of strings in list
      subl = sl.filter( QRegExp( "^" + ss + ":" ) );
   }

   else if ( ixs == 1  ||  ixs == 2 )
   {  // match RecIndex or recGUID in middle of strings in list
      subl = sl.filter( ":" + ss + ":" );
   }

   else if ( ixs == 3 )
   {  // match parentGUID at end of strings in list
      subl = sl.filter( QRegExp( ":" + ss + "$" ) );
   }

   return subl;
}

// list orphans of a record type (in rec list, no tie to parent list)
QStringList US_ManageData::list_orphans( QStringList& rlist,
   QStringList& plist )
{
   QStringList olist;

   for ( int ii = 0; ii < rlist.size(); ii++ )
   {  // examine parentGUID for each record in the list
      QString pGUID = rlist.at( ii ).section( ":", 3, 3 );

      // see if it is the recordGUID of any in the potential parent list
      int     pndx  = index_substring( pGUID, 2, plist );

      if ( pndx < 0 )
      {  // no parent found, so add to the orphan list
         olist << rlist.at( ii );
      }
   }

   return olist;
}

// return a record state flag with parent state ORed in
int US_ManageData::record_state_flag( DataDesc descr, int pstate )
{
   int state = descr.recState;

   if ( descr.recState == NOSTAT  ||
        descr.description.contains( "-ARTIFICIAL" ) )
      state = NOSTAT;                    // mark a dummy record

   else
   {  // detect and mark parentage of non-dummy
      if ( ( pstate & REC_DB ) != 0 )
         state = state | PAR_DB;         // mark a record with db parent

      if ( ( pstate & REC_LO ) != 0 )
         state = state | PAR_LO;         // mark a record with local parent
   }

   return state;
}

// use database information to create a new local XML experiment record
int US_ManageData::new_experiment_local( US_DB2* db, QString& pathexp )
{
   int stat = 70000;

   QString projID    = db->value(  0 ).toString();
   QString expID     = db->value(  1 ).toString();
   QString expGUID   = db->value(  2 ).toString();
   QString labID     = db->value(  3 ).toString();
   QString instrID   = db->value(  4 ).toString();
   QString operID    = db->value(  5 ).toString();
   QString rotorID   = db->value(  6 ).toString();
   QString expType   = db->value(  7 ).toString();
   QString runTemp   = db->value(  8 ).toString();
   QString label     = db->value(  9 ).toString();
   QString comment   = db->value( 10 ).toString();
   QString centrif   = db->value( 11 ).toString();
   QString dateUpd   = db->value( 12 ).toString();
   QString personID  = db->value( 13 ).toString();

   US_ExpInfo::ExperimentInfo expdata;
   US_ExpInfo::TripleInfo     triple;

   int centerpiece = 0;

   QString runType;
   QString invGUID;
   QString lastName;
   QString firstName;
   QString runID;
   QString labGUID;
   QString instrSerial;
   QString operGUID;
   QString optSystem;

   expdata.triples.clear();
   expdata.rpms   .clear();

   expdata.invID              = personID.toInt();
   expdata.invGUID            = invGUID;
   expdata.lastName           = lastName;
   expdata.firstName          = firstName;
   expdata.expID              = expID.toInt();
   expdata.expGUID            = expGUID;
   expdata.projectID          = projID.toInt();
   expdata.runID              = runID;
   expdata.labID              = labID.toInt();
   expdata.labGUID            = labGUID;
   expdata.instrumentID       = instrID.toInt();
   expdata.instrumentSerial   = instrSerial;
   expdata.operatorID         = operID.toInt();
   expdata.operatorGUID       = operGUID;
   expdata.rotorID            = rotorID.toInt();
   expdata.expType            = expType;
   expdata.opticalSystem      = optSystem.toAscii();
   expdata.runTemp            = runTemp;
   expdata.label              = runID;
   expdata.comments           = runID + " experiment";
   expdata.centrifugeProtocol = centrif;
   expdata.date               = QDateTime::currentDateTime().toString();

   triple.centerpiece    = centerpiece;
   //triple.bufferID       = buffer.bufferID.toInt();
   //triple.bufferGUID     = buffer.GUID;
   //triple.bufferDesc     = buffer.description;
   //triple.analyteID      = analyte.analyteID.toInt();
   //triple.analyteGUID    = analyte.analyteGUID;
   //triple.analyteDesc    = analyte.description;
   //triple.tripleFilename = filename;

   QList< int > tripmap;
   QStringList  triples;

   tripmap << 0;

   stat = US_ConvertIO::writeXmlFile( expdata, triples, tripmap,
                                      runType, runID, pathexp );
   return stat;
}

// use local XML information to create a new database experiment record
int US_ManageData::new_experiment_db( US_DB2* db, QString& pathexp,
      QString& expID, QString& expGUID )
{
   int stat = 80000;

   US_ExpInfo::ExperimentInfo expdata;
   US_ExpInfo::TripleInfo     triple;

   QString expdir   = pathexp.section( "/",  0, -2 );
   QString expfile  = pathexp.section( "/", -1, -1 );
   QString runID    = expfile.section( ".",  0,  0 );
   QString runType  = expfile.section( ".",  1,  1 );

   QStringList triples;

   stat = US_ConvertIO::readXmlFile( expdata, triples,
                                     runType, runID, expdir );

   int iexpID       = expdata.expID;
   expID            = QString::number( iexpID );
   expGUID          = expdata.expGUID;

   QStringList query( "new_experiment" );

   query << expdata.expGUID
         << QString::number( expdata.projectID )
         << runID
         << QString::number( expdata.labID )
         << QString::number( expdata.instrumentID )
         << QString::number( expdata.operatorID )
         << QString::number( expdata.rotorID )
         << expdata.expType
         << expdata.runTemp
         << expdata.label
         << expdata.comments
         << expdata.centrifugeProtocol;

   stat = db->statusQuery( query );

   if ( stat != US_DB2::OK )
      stat += 80000;

   return stat;
}

// slot to get analyte data returned by us_analyte_gui
void US_ManageData::assignAnalyte( US_Analyte data )
{
   analyte = data;
}

// slot to get buffer data returned by us_buffer_gui
void US_ManageData::assignBuffer(  US_Buffer  data )
{
   buffer  = data;
}

// calculate the md5hash and size of a named file
QString US_ManageData::md5sum_file( QString filename )
{
   QFile f( filename );

   if ( ! f.open( QIODevice::ReadOnly ) )
      return "0 0";

   QByteArray data = f.readAll();
   f.close();

   QString hashandsize =
      QString( QCryptographicHash::hash( data, QCryptographicHash::Md5 )
      .toHex() ) + " " + 
      QString::number( QFileInfo( filename ).size() );

   return hashandsize;
}

