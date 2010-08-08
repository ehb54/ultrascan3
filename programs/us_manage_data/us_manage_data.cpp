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

const QColor colrRed(   240,   0,   0 );
const QColor colrBlue(    0,   0, 255 );
const QColor colrBrown( 120,  60,   0 );
const QColor colrGreen(   0, 150,   0 );
const QColor colrGray(  110, 110, 110 );
const QColor colrWhite( 255, 255, 240 );

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
   theads << "Type" << "Label"
      << "nDc" << "nDd" << "nLc" << "nLd"
      << "SubType" << "Source";
   ntrows = 5;
   ntcols = theads.size();
   tw_recs->setHeaderLabels( theads );
   tw_recs->setFont(  QFont( "monospace", US_GuiSettings::fontSize() - 1 ) );
   QTreeWidgetItem* pitems[4];

   for ( int ii = 0; ii < ntrows; ii++ )
   {
      QColor  bg   = colrWhite;
      QColor  fg   = colrBlue;
      QString rtyp = "Raw";
      QString labl = QString( "item%11" ).arg( ii + 1 );
      QString subt = "";
      QString rsrc = "DB";
      QString andc = "1";
      QString andd = "2";
      QString anlc = "0";
      QString anld = "3";
      QStringList cvals;
      int ityp     = 1;

      if      ( ii == 1 )
      {
         fg   = colrGreen;
         rtyp = "Edited";
         ityp = 2;
         rsrc = "In Sync";
         andc = "0";
         andd = "1";
         anlc = "1";
         anld = "1";
         subt = "RA";
      }
      else if ( ii == 2 )
      {
         fg   = colrBrown;
         rtyp = "Model";
         ityp = 3;
         rsrc = "Local";
         andc = "1";
         andd = "1";
         anlc = "1";
         anld = "1";
         subt = "2DSA";
      }
      else if ( ii == 3 )
      {
         fg   = colrRed;
         rtyp = "Noise";
         ityp = 4;
         rsrc = "Conflict";
         andc = "0";
         andd = "0";
         anlc = "0";
         anld = "0";
         subt = "TI";
      }
      else if ( ii == 4 )
      {
         fg   = colrGray;
         rtyp = "Edited";
         ityp = 2;
         rsrc = "dummy";
         andc = "0";
         andd = "0";
         anlc = "0";
         anld = "0";
         subt = "";
      }
      labl = QString( "item %1 2" ).arg( ii + 1 );

      QTreeWidgetItem* item;
      int wiutype = (int)QTreeWidgetItem::UserType + ii;

      cvals << rtyp << labl << andc << andd << anlc << anld << subt << rsrc;

      if ( ityp <= 1 )
         item = new QTreeWidgetItem( tw_recs, cvals, wiutype );
      else
         item = new QTreeWidgetItem( pitems[ ityp - 2 ], cvals, wiutype );

      pitems[ ityp - 1 ] = item;

      for ( int jj = 0; jj < ntcols; jj++ )
      {
         item->setForeground( jj, QBrush( fg ) );
         item->setBackground( jj, QBrush( bg ) );
         if ( jj > 1  &&  jj < 6 )
            item->setTextAlignment( jj, Qt::AlignRight );
      }
      //item->setObjectName( QString( "Rec#%1" ).arg( ii ) );
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
   QString blabl = pb_hsedit->text();          // current button label
   bool show     = blabl.startsWith( "Show" ); // show or hide?

   toggle_hidden( "Raw",    show );            // expand/collapse one level up

   blabl.replace( show ? "Show" : "Hide", show ? "Hide" : "Show" );

   pb_hsedit->setText( blabl );

   if ( !show )
   {  // for "hide", must toggle labels below
      toggle_hidden( "Model", false );
      toggle_hidden( "Noise", false );
      blabl = pb_hsmodl->text();
      blabl.replace( "Hide", "Show" );
      pb_hsmodl->setText( blabl );
      blabl = pb_hsnois->text();
      blabl.replace( "Hide", "Show" );
      pb_hsnois->setText( blabl );
   }
}

// toggle models between hide/show
void US_ManageData::toggle_models()
{
   QString blabl = pb_hsmodl->text();          // current button label
   bool show     = blabl.startsWith( "Show" ); // show or hide?

   toggle_hidden( "Edited", show );            // expand/collapse one level up

   blabl.replace( show ? "Show" : "Hide", show ? "Hide" : "Show" );

   pb_hsmodl->setText( blabl );

   if ( show )
   { // for "show", must expand next level up
      toggle_hidden( "Raw",    show );
      blabl = pb_hsedit->text();
      blabl.replace( "Show", "Hide" );
      pb_hsedit->setText( blabl );
   }

   else
   {  // for "hide", must toggle label below
      toggle_hidden( "Noise", false );
      blabl = pb_hsnois->text();
      blabl.replace( "Hide", "Show" );
      pb_hsnois->setText( blabl );
   }
}

// toggle noises between hide/show
void US_ManageData::toggle_noises()
{
   QString blabl = pb_hsnois->text();          // current button label
   bool show     = blabl.startsWith( "Show" ); // show or hide?

   toggle_hidden( "Model",  show );            // expand/collapse one level up

   blabl.replace( show ? "Show" : "Hide", show ? "Hide" : "Show" );

   pb_hsnois->setText( blabl );

   if ( show )
   { // for "show", must expand levels above
      toggle_hidden( "Edited", show );
      toggle_hidden( "Raw",    show );
      blabl = pb_hsedit->text();
      blabl.replace( "Show", "Hide" );
      pb_hsedit->setText( blabl );
      blabl = pb_hsmodl->text();
      blabl.replace( "Show", "Hide" );
      pb_hsmodl->setText( blabl );
   }

}

// hide or show all rows of specified type
void US_ManageData::toggle_hidden( QString c1str, bool show )
{
   QList< QTreeWidgetItem* > listi = tw_recs->findItems(
      c1str, Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive, 0 );
qDebug() << "TOG_HIDD: match show listi.size" << c1str << show << listi.size();

   for ( int ii = 0; ii < listi.size(); ii++ )
   {
      listi.at( ii )->setExpanded( show );
   }
}

// browse the database and local disk for R/E/M/N data sets
void US_ManageData::browse_data()
{
   ddescs.clear();   // db descriptions
   ldescs.clear();   // local descriptions
   adescs.clear();   // all descriptions

   cdesc.recordID       = 1;
   cdesc.recType        = 1;
   cdesc.subType        = "";
   cdesc.parentID       = 1;
   cdesc.recState       = REC_DB | PAR_DB;
   cdesc.dataGUID       = "6fab-def";
   cdesc.parentGUID     = "7a24-72c";
   cdesc.filename       = "";
   cdesc.contents       = "";
   cdesc.label          = "demo1-veloc";
   cdesc.description    = "demo1-veloc.1.A.260.auc";
   cdesc.lastmodDate    = QDateTime::currentDateTime().toString();
   ddescs.append( cdesc );

   cdesc.recType        = 2;
   cdesc.parentGUID     = cdesc.dataGUID;
   cdesc.dataGUID       = "a94b-c71";
   cdesc.description    = "demo1-veloc.20100714.1.A.260.xml";
   ddescs.append( cdesc );

   cdesc.recType        = 3;
   cdesc.subType        = "2DSA";
   cdesc.parentGUID     = cdesc.dataGUID;
   cdesc.dataGUID       = "921b-e75";
   cdesc.description    = "demo1-veloc.20100714.model.11";
   ddescs.append( cdesc );

   cdesc.recType        = 4;
   cdesc.subType        = "TI";
   cdesc.parentGUID     = cdesc.dataGUID;
   cdesc.dataGUID       = "271e-7e5";
   cdesc.description    = "demo1-veloc.ti_noise.11";
   ddescs.append( cdesc );
#if 0
   cdesc.recordID       = -1;
   cdesc.recType        = 2;
   cdesc.parentGUID     = "6fab-def";
   cdesc.dataGUID       = "a94b-c71";
   cdesc.description    = "demo1-veloc.20100714.1.A.260.xml";
   cdesc.filename       = "demo1-veloc.20100714.1.A.260.xml";
   ldescs.append( cdesc );

   cdesc.recType        = 3;
   cdesc.parentGUID     = cdesc.dataGUID;
   cdesc.dataGUID       = "921b-e75";
   cdesc.description    = "demo1-veloc.20100714.model.11";
   cdesc.filename       = "demo1-veloc.20100714.model.11";
   ldescs.append( cdesc );
#endif
   kdmy        = 0;

   browse_dbase( false  );     // read db to build db descriptions

   sort_descs(   ddescs );     // sort db descriptions

   browse_local( false  );     // read files to build local descriptions

   sort_descs(   ldescs );     // sort local descriptions

   merge_dblocal();            // merge database and local descriptions
qDebug() << " a/d/l sizes" << adescs.size() << ddescs.size() << ldescs.size();

   build_dtree();              // rebuild the data tree with present data
}

// browse the database,local for R/E/M/N data sets, with content detail
void US_ManageData::detail_data()
{
   kdmy        = 0;

   browse_dbase( true   );     // read db to build db descriptions

   sort_descs(   ddescs );     // sort db descriptions

   browse_local( true   );     // read files to build local descriptions

   sort_descs(   ldescs );     // sort local descriptions

   merge_dblocal();            // merge database and local descriptions

   build_dtree();              // rebuild the data tree with present data
}

// build the data tree from descriptions read
void US_ManageData::build_dtree()
{
   QString rtyp;
   QString styp;
   QString labl;
   QString dguid;
   QString rsrc;
   QString andc;
   QString andd;
   QString anlc;
   QString anld;
   int     ityp = 1;
   int     ndc  = 0;
   int     ndd  = 0;
   int     nlc  = 0;
   int     nld  = 0;
   QBrush  fbru( colrBrown );
   QBrush  bbru( colrWhite );
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
      QStringList cvals;

      if ( cdesc.recordID < 0 )
      { // local only
         fbru    = QBrush( colrBrown );
         rsrc    = "Local";

         if ( cdesc.recType == 1 )
         {
            nlraws++; ncraws++;
            rtyp    = "Raw";
            ityp    = 1;
         }
         else if ( cdesc.recType == 2 )
         {
            nledts++; ncedts++;
            rtyp    = "Edited";
            ityp    = 2;
         }
         else if ( cdesc.recType == 3 )
         {
            nlmods++; ncmods++;
            rtyp    = "Model";
            ityp    = 3;
         }
         else if ( cdesc.recType == 4 )
         {
            nlnois++; ncnois++;
            rtyp    = "Noise";
            ityp    = 4;
         }
         else
         {
            rtyp    = "none";
            ityp    = 0;
         }
      }

      else if ( cdesc.filename.isEmpty() )
      { // database only
         fbru    = QBrush( colrBlue );
         rsrc    = "DB";

         if ( cdesc.recType == 1 )
         {
            ndraws++; ncraws++;
            rtyp    = "Raw";
            ityp    = 1;
         }
         else if ( cdesc.recType == 2 )
         {
            ndedts++; ncedts++;
            rtyp    = "Edited";
            ityp    = 2;
         }
         else if ( cdesc.recType == 3 )
         {
            ndmods++; ncmods++;
            rtyp    = "Model";
            ityp    = 3;
         }
         else if ( cdesc.recType == 4 )
         {
            ndnois++; ncnois++;
            rtyp    = "Noise";
            ityp    = 4;
         }
         else
         {
            rtyp    = "none";
            ityp    = 0;
         }
      }

      else
      { // both local and database
         fbru    = QBrush( colrGreen );
         rsrc    = "In Sync";

         if ( cdesc.recType == 1 )
         {
            nlraws++; ndraws++; ncraws++;
            rtyp    = "Raw";
            ityp    = 1;
         }
         else if ( cdesc.recType == 2 )
         {
            nledts++; ndedts++; ncedts++;
            rtyp    = "Edited";
            ityp    = 2;
         }
         else if ( cdesc.recType == 3 )
         {
            nlmods++; ndmods++; ncmods++;
            rtyp    = "Model";
            ityp    = 3;
         }
         else if ( cdesc.recType == 4 )
         {
            nlnois++; ndnois++; ncnois++;
            rtyp    = "Noise";
            ityp    = 4;
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
      styp       = cdesc.subType;
      ndc        = ndd = nlc = nld = 0;

      if ( cdesc.recState == NOSTAT )
      {
         fbru       = QBrush( colrGray );
         rsrc       = "dummy";
      }

      for ( int jj = ( ii + 1 ); jj < ncrecs; jj++ )
      {  // count children and descendants
         int     crtyp = adescs[ jj ].recType;
         int     cdbid = adescs[ jj ].recordID;
         QString cfnam = adescs[ jj ].filename;
         QString cpgid = adescs[ jj ].parentGUID;

         if ( crtyp <= cdesc.recType )
            break;

         if ( cpgid == dguid )
         {  // this child has parent GUID matching level above
            if ( cdbid < 0 )            { nlc++; nld++; }
            else if ( cfnam.isEmpty() ) { ndc++; ndd++; }
            else                        { ndc++; nlc++; ndd++; nld++; }
         }
         else
         {  // this is descendant, not direct child
//qDebug() << "cpgid dguid" << cpgid << dguid;
            if ( cdbid < 0 )            { nld++; }
            else if ( cfnam.isEmpty() ) { ndd++; }
            else                        { ndd++; nld++; }
         }
      }

      andc = QString::number( ndc );
      andd = QString::number( ndd );
      anlc = QString::number( nlc );
      anld = QString::number( nld );

      QTreeWidgetItem* item;
      int wiutype = (int)QTreeWidgetItem::UserType + ii;

      cvals << rtyp << labl << andc << andd << anlc << anld << styp << rsrc;

      if ( ityp == 1 )
         item = new QTreeWidgetItem( tw_recs, cvals, wiutype );
      else
         item = new QTreeWidgetItem( pitems[ ityp - 2 ], cvals, wiutype );

      pitems[ ityp - 1 ] = item;

      for ( int jj = 0; jj < ntcols; jj++ )
      {
         item->setForeground( jj, fbru );
         item->setBackground( jj, bbru );
         if ( jj > 1  &&  jj < 6 )
            item->setTextAlignment( jj, Qt::AlignRight );
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

   QString blabl;                             // make buttons say "Show ..."
   blabl = pb_hsedit->text();
   blabl.replace( "Hide", "Show" );
   pb_hsedit->setText( blabl );
   blabl = pb_hsmodl->text();
   blabl.replace( "Hide", "Show" );
   pb_hsmodl->setText( blabl );
   blabl = pb_hsnois->text();
   blabl.replace( "Hide", "Show" );
   pb_hsnois->setText( blabl );

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
qDebug() << "TABLE ITEM CLICKED rbtn_click" << rbtn_click;

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
   //int irow = item->type() - (int)QTreeWidgetItem::UserType;
   //int ixpo = cur_pos->x();
   //int iypo = cur_pos->y();

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

   cmenu->exec( QCursor::pos() );

}

// open a dialog and display data tree help
void US_ManageData::dtree_help()
{
   QString fileexts = tr( "Text,Log files (*.txt *.log);;" )
      + tr( "All files (*)" );
   QString mtext =
      tr( "Data Tree Columns --\n\n" ) +
      tr( "  \"Type\"    :  type of data set record -\n" ) +
      tr( "                 \"Raw\", \"Edited\", \"Model\" or \"Noise\".\n" ) +
      tr( "  \"Label\"   :  short description of specific record.\n" ) +
      tr( "  \"nDc\"     :  number of Database Children of this row.\n" ) +
      tr( "  \"nDd\"     :  number of Database Descendants of this row.\n" ) +
      tr( "  \"nLc\"     :  number of Local-disk Children of this row.\n" ) +
      tr( "  \"nLd\"     :  number of Local Descendants of this row.\n" ) +
      tr( "  \"SubType\" :  record-specific type (e.g. \"2DSA\", \"TI\").\n" ) +
      tr( "  \"Source\"  :  location/state (see color legend below)-\n" ) +
      tr( "                 \"DB\"        (Blue, database); \n" ) +
      tr( "                 \"Local\"     (Brown, local disk); \n" ) +
      tr( "                 \"In Sync\"   (Green, both, consistent); \n" ) +
      tr( "                 \"Conflict\"  (Red), both, inconsistent); \n" ) +
      tr( "                 \"dummy\"     (Gray, missing parent filler).\n\n" )+
      tr( "Tree Row Color Legend --\n\n" ) +
      tr( "  Blue      :  This record exists for database only.\n" ) +
      tr( "  Brown     :  This record exists for local disk only.\n" ) +
      tr( "  Green     :  This record exists and is consistent\n" ) +
      tr( "               for both database and local.\n" ) +
      tr( "  Red       :  This record exists for both, but is inconsistent.\n")+
      tr( "  Gray      :  This row is a filler for a missing parent.\n\n" ) +
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

// perform item upload action
void US_ManageData::item_upload()
{
qDebug() << "ITEM Upload";
}

// perform item download action
void US_ManageData::item_download()
{
qDebug() << "ITEM Download";
}

// perform item remove-from-db action
void US_ManageData::item_remove_db()
{
qDebug() << "ITEM Remove from DB";
}

// perform item remove-from-local action
void US_ManageData::item_remove_loc()
{
qDebug() << "ITEM Remove from Local";
}

// perform item remove-from-all action
void US_ManageData::item_remove_all()
{
qDebug() << "ITEM Remove Both DB and Local";
}

// perform item details action
void US_ManageData::item_details(  )
{
   const char* rtyps[] = { "RawData", "EditedData", "Model", "Noise" };
   int irow = tw_item->type() - (int)QTreeWidgetItem::UserType;
qDebug() << "ITEM Show Details  row" << irow;
   QString fileexts = tr( "Text,Log files (*.txt *.log);;" )
      + tr( "All files (*)" );

   cdesc             = adescs[ irow++ ];  // get desc., index as 1...

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
      tr( "  Last Mod Date  : " ) + cdesc.lastmodDate;

   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Data Tree Entry Details" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// perform item show-children action
void US_ManageData::item_show_child()
{
qDebug() << "ITEM Show Children";
#if 0
   int irow = tw_item->row();
   int ityp = adescs[ irow ].recType;

   for ( int jj = ( irow + 1 ); jj < ncrecs; jj++ )
   {  // browse children and descendants
      int ctyp = adescs[ jj ].recType;

      if ( ctyp <= ityp )      // break out when a sibling is reached
         break;

      // this record has greater type number and so is a child or descendant
      tw_recs->showRow( jj );  // show this child row
   }
#endif
}

// perform item hide-children action
void US_ManageData::item_hide_child()
{
qDebug() << "ITEM Hide Children";
#if 0
   int irow = tw_item->row();
   int ityp = adescs[ irow ].recType;

   for ( int jj = ( irow + 1 ); jj < ncrecs; jj++ )
   {  // browse children and descendants
      int ctyp = adescs[ jj ].recType;

      if ( ctyp <= ityp )      // break out when a sibling is reached
         break;

      // this record has greater type number and so is a child or descendant
      tw_recs->hideRow( jj );  // hide this child row
   }
#endif
}

// browse the database for R/E/M/N data sets, with/without content details
void US_ManageData::browse_dbase( bool details = false )
{
   QStringList rawIDs;
   QStringList edtIDs;
   QStringList modIDs;
   QStringList noiIDs;
   QStringList query;
   QString     invID = QString::number( personID );
   QString     recID;
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
      QString rawGUID   = db->value( 0 ).toString();
      QString label     = db->value( 1 ).toString();
      QString filename  = db->value( 2 ).toString();
      QString comment   = db->value( 3 ).toString();
      QString experID   = db->value( 4 ).toString();
      QString date      = db->value( 6 ).toString();
      QString runID     = filename.section( ".", 0, 0 );
      QString subType   = filename.section( ".", 1, 1 );
qDebug() << "BrDb:   raw ii id eid rGid label date"
   << ii << irecID << experID << rawGUID << label << date;

      query.clear();
      query << "get_experiment_info" << experID;
      db->query( query );
      db->next();

      QString expGUID   = db->value( 0 ).toString();
qDebug() << "BrDb:     raw expGid" << expGUID;
if ( rawGUID.startsWith( "0000" ) )
{
  qDebug() << "BrDb:        *** 0000 rawGUID ***";
  qDebug() << "BrDb:           filename" << filename;
  qDebug() << "BrDb:           comment" << comment;
  rawGUID = US_Util::new_guid();
  query.clear();
  query << "delete_rawData" << experID;
  int rmstat = db->statusQuery( query );
  if ( rmstat != US_DB2::OK )
  {
     qDebug() << "*ERROR* deleting eID" << experID;
     break;
  }
  qDebug() << "BrDb:             new raw GUID" << rawGUID;
  query.clear();
  query << "new_rawData"
     << rawGUID
     << label
     << filename
     << comment
     << experID
     << "1"; // channel ID
  int status = db->statusQuery( query );
  irecID = db->lastInsertID();
  if ( status == US_DB2::OK )
  {
     QString rdir = US_Settings::resultDir();
     QString filepath = rdir + "/" + runID + "/" + filename;
     int wrstat = db->writeBlobToDB( filepath, QString( "upload_aucData" ),
           irecID );
     if ( wrstat == US_DB2::ERROR )
     {
        qDebug() << "*ERROR* processing" << filepath;
     }
     else
     {
        qDebug() << "+SUCCESS+ loading" << filepath;
     }
  }
  else
  {
     qDebug() << "*ERROR* processing" << rawGUID;
  }
}

      cdesc.recordID    = irecID;
      cdesc.recType     = 1;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = rawGUID;
      cdesc.parentGUID  = expGUID;
      //cdesc.filename    = filename;
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = label;
      cdesc.description = comment + " File: " + filename;
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

      query.clear();
      query << "get_rawData" << rawID;
      db->query( query );
      db->next();

      QString rawGUID   = db->value( 0 ).toString();

qDebug() << "BrDb:     edt ii id eGID rGID label" << ii << irecID << editGUID
   << rawGUID << label;

      cdesc.recordID    = irecID;
      cdesc.recType     = 2;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = editGUID;
      cdesc.parentGUID  = rawGUID;
      //cdesc.filename    = filename;
      cdesc.filename    = "";
      cdesc.contents    = "";
      cdesc.label       = label;
      cdesc.description = comment.isEmpty() ?
                          "File: " + filename :
                          comment + " (File: " + filename + ")";
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
      QString contents  = db->value( 2 ).toString();
      QString label     = descript;

      if ( label.length() > 40 )
         label = descript.left( 18 ) + "..." + descript.right( 19 );

      QString subType   = model_type( contents );
      int     jj        = contents.indexOf( "editGUID=" );
      QString editGUID  = ( jj < 1 ) ? "" :
                          contents.mid( jj ).section( QChar( '"' ), 1, 1 );
//qDebug() << "BrDb:       mod ii id mGID dsc"
//   << ii << irecID << modelGUID << descript;

      cdesc.recordID    = irecID;
      cdesc.recType     = 3;
      cdesc.subType     = subType;
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = modelGUID;
      cdesc.parentGUID  = editGUID;
      cdesc.filename    = "";
      cdesc.contents    = details ? contents : "";
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

      if ( label.length() > 40 )
         label = descript.left( 18 ) + "..." + descript.right( 19 );

      cdesc.recordID    = irecID;
      cdesc.recType     = 4;
      cdesc.subType     = "";
      cdesc.recState    = REC_DB;
      cdesc.dataGUID    = noiseGUID;
      //cdesc.parentGUID  = modelGUID;
      //cdesc.filename    = filename;
      cdesc.contents    = details ? contents : "";
      cdesc.label       = label;
      cdesc.description = descript;
      //cdesc.lastmodDate = QFileInfo( aucfile ).lastModified().toString();

      ddescs << cdesc;
      progress->setValue( ++istep );
   }

qDebug() << "BrDb: work done"; 
   progress->setValue( nstep );
   lb_status->setText( tr( "Database Review Complete" ) );
qDebug() << "BrDb: progress reported";
}

// browse the local disk for R/E/M/N data sets, with/without content details
void US_ManageData::browse_local( bool details = false )
{
   // start with AUC (raw) and edit files in directories of resultDir
   QString     rdir    = US_Settings::resultDir();
   QString     ddir    = US_Settings::dataDir();
   QString     dirm    = ddir + "/models";
   QString     dirn    = ddir + "/noises";
   QStringList aucdirs = QDir( rdir )
      .entryList( QDir::AllDirs, QDir::Name );
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
         QString     fname   = aucfiles.at( jj );
         QString     runid   = fname.section( ".", 0, 0 );
         QString     tripl   = fname.section( ".", -5, -2 );
         QString     aucfile = subdir + "/" + fname;
qDebug() << "BrLoc: ii jj file" << ii << jj << aucfile;

         // read in the raw data and build description record
         US_DataIO2::readRawData( aucfile, rdata );

         char uuid[ 37 ];
         uuid_unparse( (uchar*)rdata.rawGUID, uuid );
         cdesc.recordID    = -1;
         cdesc.recType     = 1;
         cdesc.subType     = fname.section( ".", 1, 1 );
         cdesc.recState    = REC_LO;
         cdesc.dataGUID    = QString( uuid );
         cdesc.parentGUID  = "0";
         cdesc.filename    = aucfile;
         cdesc.contents    = "";
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
            QString efname  = edtfiles.at( kk );
            QString editid  = efname.section( ".", 1, 3 );
            QString edtfile = subdir + "/" + efname;
//qDebug() << "BrLoc:    kk file" << jj << edtfile;

            // read EditValues for the edit data and build description record
            US_DataIO2::readEdits( edtfile, edval );

            cdesc.recordID    = -1;
            cdesc.recType     = 2;
            cdesc.subType     = efname.section( ".", 2, 2 );
            cdesc.recState    = REC_LO;
            cdesc.dataGUID    = edval.editGUID;
            cdesc.parentGUID  = edval.dataGUID;
            cdesc.filename    = edtfile;
            cdesc.contents    = "";
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

      model.load( modfil );

      cdesc.recordID    = -1;
      cdesc.recType     = 3;
      cdesc.subType     = model_type( model );
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = model.modelGUID;
      cdesc.parentGUID  = model.editGUID;
      cdesc.filename    = modfil;
      cdesc.contents    = "";
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

      noise.load( noifil );

      cdesc.recordID    = -1;
      cdesc.recType     = 4;
      cdesc.subType     = ( noise.type == US_Noise::RI ) ? "RI" : "TI";
      cdesc.recState    = REC_LO;
      cdesc.dataGUID    = noise.noiseGUID;
      cdesc.parentGUID  = noise.modelGUID;
      cdesc.filename    = noifil;
      cdesc.contents    = "";
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
   int kar   = 0;

   DataDesc  descd = ddescs.at( 0 );
   DataDesc  descl = ldescs.at( 0 );

   lb_status->setText( tr( "Merging Data ..." ) );
   progress->setMaximum( nstep );

   while ( jdr < nddes  &&  jlr < nldes )
   {  // main loop to merge records until one is exhausted

      progress->setValue( kar + 1 );

      if ( kar > nstep )
      {
         nstep = ( kar * 9 ) / 8;
         progress->setMaximum( nstep );
      }

      while ( descd.dataGUID == descl.dataGUID )
      {  // records match in GUID:  merge them into one
         descd.recState    |= descl.recState;
         descd.filename     = descl.filename;
         descd.lastmodDate  = descl.lastmodDate;
         adescs << descd;
         kar++;

         if ( ++jdr >= nddes )
            break;

         descd = ddescs.at( jdr );

         if ( ++jlr >= nldes )
            break;

         descl = ldescs.at( jlr );
      }

      while ( descd.recType > descl.recType )
      {  // output db records that are left-over children
         adescs << descd;
         kar++;

         if ( ++jdr >= nddes )
            break;

         descd = ddescs.at( jdr );
      }

      while ( descl.recType > descd.recType )
      {  // output local records that are left-over children
         adescs << descl;
         kar++;

         if ( ++jlr >= nldes )
            break;

         descl = ldescs.at( jlr );
      }

      // if we've reached another matching pair or if we are not at
      // the same level, go back up to start of main loop
      if ( descd.dataGUID == descl.dataGUID  ||
           descd.recType  != descl.recType  )
         continue;

      if ( descd.label < descl.label )
      {  // output db record first based on alphabetic label sort
         adescs << descd;
         kar++;

         if ( ++jdr >= nddes )
            break;

         descd = ddescs.at( jdr );
      }

      else
      {  // output local record first based on alphabetic label sort
         adescs << descl;
         kar++;

         if ( ++jlr >= nddes )
            break;

         descl = ddescs.at( jlr );
      }
   }

   // after breaking from main loop, output any records left from one
   // source (db,local) or the other.
   //
   while ( jdr < nddes )
   {
      adescs << ddescs.at( jdr++ );
      kar++;
   }

   while ( jlr < nldes )
   {
      adescs << ldescs.at( jlr++ );
      kar++;
   }

qDebug() << "MERGE: nddes nldes kar" << nddes << nldes << kar;

   //adescs.clear();
   //adescs << ddescs;
   //adescs << ldescs;

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
   int nddm = 0;     // flag of duplicate dummies

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
         if ( nddm == 0 )      // first time:  create one
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

         if ( nddm > 0 )       // after 1st time, skip creating new parent
            continue;

         nddm++;               // flag that we have a parent for invalid ones
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
qDebug() << "N orphan:" << orphn.at( ii );
qDebug() << "  M dummy:" << dsorts;
   }

   nddm   = 0;

   for ( int ii = 0; ii < orphm.size(); ii++ )
   {  // for each orphan model, create a dummy edit
      dsorts = orphm.at( ii );
      dlabel = dsorts.section( ":", 0, 0 );
      dindex = dsorts.section( ":", 1, 1 );
      ddGUID = dsorts.section( ":", 2, 2 );
      dpGUID = dsorts.section( ":", 3, 3 );
      jndx   = dindex.toInt();
      cdesc  = tdess[ jndx ];

      if ( dpGUID.length() < 2 )
      { // handle case where there is no valid parentGUID
         if ( nddm == 0 )      // first time:  create one
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

         if ( nddm > 0 )       // after 1st time, skip creating new parent
            continue;

         nddm++;               // flag that we have a parent for invalid ones
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

   nddm   = 0;

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
         if ( nddm == 0 )      // first time:  create one
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

         if ( nddm > 0 )       // after 1st time, skip creating new parent
            continue;

         nddm++;               // flag that we have a parent for invalid ones
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
qDebug() << "sort/dumy: count REMN" << countR << countE << countM << countN;

   int noutR  = 0;               // count of each kind in hierarchical output
   int noutE  = 0;
   int noutM  = 0;
   int noutN  = 0;
   int pstate = REC_LO | PAR_LO;

   descs.clear();                // reset input vector to become sorted output

   // rebuild the description vector with sorted trees

   for ( int ii = 0; ii < countR; ii++ )
   {  // loop to output sorted Raw records
      QString recr   = sortr[ ii ];
      QString didr   = recr.section( ":", 2, 2 );
      QString pidr   = recr.section( ":", 3, 3 );
      int     indx   = recr.section( ":", 1, 1 ).toInt();

      cdesc          = tdess.at( indx );

      if ( ii == 0 )
      {  // set up a default parent state flag
         pstate = cdesc.recState;
         pstate = ( pstate & REC_DB ) != 0 ? ( pstate | PAR_DB ) : pstate;
         pstate = ( pstate & REC_LO ) != 0 ? ( pstate | PAR_LO ) : pstate;
      }

      // new state is the default,  or NOSTAT if this is a dummy record
      cdesc.recState = record_state( cdesc, pstate );

      descs << cdesc;                   // output Raw rec
      noutR++;

      for ( int jj = 0; jj < countE; jj++ )
      {  // loop to output sorted Edit records for the above Raw
         QString rece   = sorte[ jj ];
         QString pide   = rece.section( ":", 3, 3 );

         if ( pide != didr )            // skip if current Raw not parent
            continue;

         QString dide   = rece.section( ":", 2, 2 );
         indx           = rece.section( ":", 1, 1 ).toInt();
         cdesc          = tdess.at( indx );
         cdesc.recState = record_state( cdesc, pstate );
//qDebug() << "     ii jj indx" << ii << jj << indx;

         descs << cdesc;                // output Edit rec
         noutE++;

         for ( int mm = 0; mm < countM; mm++ )
         {  // loop to output sorted Model records for above Edit
            QString recm   = sortm[ mm ];
            QString pidm   = recm.section( ":", 3, 3 );

            if ( pidm != dide )         // skip if current Edit not parent
               continue;

            QString didm   = recm.section( ":", 2, 2 );
            indx           = recm.section( ":", 1, 1 ).toInt();
            cdesc          = tdess.at( indx );
            cdesc.recState = record_state( cdesc, pstate );

            descs << cdesc;             // output Model rec

            noutM++;

            for ( int nn = 0; nn < countN; nn++ )
            {  // loop to output sorted Noise records for above Model
               QString recn   = sortn[ nn ];
               QString pidn   = recn.section( ":", 3, 3 );

               if ( pidn != didm )      // skip if current Model not parent
                  continue;

               indx           = recn.section( ":", 1, 1 ).toInt();
               cdesc          = tdess.at( indx );
               cdesc.recState = record_state( cdesc, pstate );

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

   switch ( imtype )
   {  // format the base model type string
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

   if ( nassoc > 1 )
      mtype = mtype + "-RA";            // add RA for Reversible Associations

   if ( niters > 1 )
      mtype = mtype + "-MC";            // add MC for Monte Carlo

   return mtype;
}

// compose string describing model type
QString US_ManageData::model_type( US_Model model )
{
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

   jj       = modxml.indexOf( " type=" );
   imtype   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

   nassoc   = modxml.count( "k_eq=" );

   jj       = modxml.indexOf( " iterations=" );
   niters   = ( jj < 1 ) ? 0 : modxml.mid( jj ).section( quo, 1, 1 ).toInt();

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

// get sublist from string list of substring matches at given position
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

// return a record state flag: default or NOSTAT if artificial
int US_ManageData::record_state( DataDesc descr, int defstate )
{
   int state = defstate;

   if ( descr.recState == NOSTAT )
      state = NOSTAT;

   if ( descr.recordID < 0  &&  descr.filename.isEmpty() )
      state = NOSTAT;

   if ( descr.description.contains( "ARTIFICIAL" ) )
      state = NOSTAT;

   if ( descr.label.contains( "ARTIFICIAL" ) )
      state = NOSTAT;

   return state;
}
