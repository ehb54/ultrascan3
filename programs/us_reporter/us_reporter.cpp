//! \file us_reporter.cpp

#include <QApplication>
#include <QtSvg>

#include "us_reporter.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_editor.h"
#include "us_util.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Reporter w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_Reporter class constructor
US_Reporter::US_Reporter() : US_Widgets()
{
   // set up the GUI
   setWindowTitle( tr( "Report Chooser/Generator" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level   = US_Settings::us_debug();

   hsclogo .clear();
   becklogo.clear();
   us3logo .clear();
   QString logopath = US_Settings::appBaseDir() + "/etc/";

   if ( US_Settings::debug_match( "hsclogo" ) )
      hsclogo  = logopath + "logo_hsc.png";

   if ( US_Settings::debug_match( "becklogo" ) )
      becklogo = logopath + "beckman_logo.png";

   if ( US_Settings::debug_match( "us3logo" ) )
      us3logo  = logopath + "ultrascan3.png";

   // primary layouts
   QHBoxLayout* mainLayout  = new QHBoxLayout( this );
   QVBoxLayout* leftLayout  = new QVBoxLayout();
   QVBoxLayout* rghtLayout  = new QVBoxLayout();
   QGridLayout* dctlLayout  = new QGridLayout();
   QGridLayout* tctlLayout  = new QGridLayout();
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );
   leftLayout->setSpacing        ( 0 );
   leftLayout->setContentsMargins( 0, 1, 0, 1 );
   rghtLayout->setSpacing        ( 0 );
   rghtLayout->setContentsMargins( 0, 1, 0, 1 );
   dctlLayout->setSpacing        ( 1 );
   dctlLayout->setContentsMargins( 0, 0, 0, 0 );
   tctlLayout->setSpacing        ( 1 );
   tctlLayout->setContentsMargins( 0, 0, 0, 0 );

   // fill in the GUI components
   QLabel*      lb_runids  = us_label(      tr( "Runs:" ) );
                cb_runids  = us_comboBox();
                pb_view    = us_pushbutton( tr( "View" ) );
                pb_save    = us_pushbutton( tr( "Save" ) );
   QPushButton* pb_loadpr  = us_pushbutton( tr( "Load Profile" ) );
   QPushButton* pb_savepr  = us_pushbutton( tr( "Save Profile" ) );
                pb_help    = us_pushbutton( tr( "Help" ) );
                pb_close   = us_pushbutton( tr( "Close" ) );

   build_runids();

   int row       = 0;
   dctlLayout->addWidget( lb_runids,  row,   0, 1, 1 );
   dctlLayout->addWidget( cb_runids,  row++, 1, 1, 3 );
   dctlLayout->addWidget( pb_view,    row++, 0, 1, 4 );
   dctlLayout->addWidget( pb_save,    row++, 0, 1, 4 );
   dctlLayout->addWidget( pb_loadpr,  row++, 0, 1, 4 );
   dctlLayout->addWidget( pb_savepr,  row++, 0, 1, 4 );
   dctlLayout->addWidget( pb_help,    row,   0, 1, 2 );
   dctlLayout->addWidget( pb_close,   row++, 2, 1, 2 );

   connect( cb_runids,  SIGNAL( currentIndexChanged( int ) ),
            this,       SLOT(   new_runid(           int ) ) );
   connect( pb_view,    SIGNAL( clicked()      ),
            this,       SLOT(   view()         ) );
   connect( pb_save,    SIGNAL( clicked()      ),
            this,       SLOT(   save()         ) );
   connect( pb_loadpr,  SIGNAL( clicked()      ),
            this,       SLOT(   load_profile() ) );
   connect( pb_savepr,  SIGNAL( clicked()      ),
            this,       SLOT(   save_profile() ) );
   connect( pb_help,    SIGNAL( clicked()      ),
            this,       SLOT(   help()         ) );
   connect( pb_close,   SIGNAL( clicked()      ),
            this,       SLOT(   close()        ) );

   cb_runids->setToolTip(
      tr( "Select the Run for which to produce a composite report" ) );
   pb_view  ->setToolTip(
      tr( "View the composite report of selected individual reports" ) );
   pb_save  ->setToolTip(
      tr( "Create a composite report using selected individual reports" ) );
   pb_loadpr->setToolTip(
      tr( "Load a previously saved report-selection profile" ) );
   pb_savepr->setToolTip(
      tr( "Save a report-selection profile for later use" ) );
   pb_help  ->setToolTip(
      tr( "Display detailed US_Reporter documentation text and images" ) );
   pb_close ->setToolTip(
      tr( "Close the US_Reporter window and exit" ) );
   pb_view->setEnabled( false );
   pb_save->setEnabled( false );

   // set up data tree; populate with sample data
   rbtn_click        = false;
   change_tree       = true;
   tw_recs           = new QTreeWidget();
   tw_recs->setPalette( pb_help->palette() );
   tctlLayout->addWidget( tw_recs );

   QStringList theads;
   theads << "Selected" << "Report" << "Item Type";
   ntrows = 5;
   ntcols = theads.size();
   tw_recs->setHeaderLabels( theads );
   tw_recs->setFont(  QFont( "monospace", US_GuiSettings::fontSize() - 1 ) );
   tw_recs->setObjectName( QString( "tree-widget" ) );
   tw_recs->setAutoFillBackground( true );
   tw_recs->installEventFilter   ( this );

   sample_tree();

   connect( tw_recs, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ),
            this,    SLOT(   clickedItem( QTreeWidgetItem*      ) ) );
   connect( tw_recs, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
            this,    SLOT(   changedItem( QTreeWidgetItem*, int ) ) );

   // put layouts together for overall layout
   leftLayout->addLayout( dctlLayout );
   rghtLayout->addLayout( tctlLayout );

   mainLayout->addLayout( leftLayout );
   mainLayout->addLayout( rghtLayout );
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rghtLayout, 8 );

   show();
}

// Filter events to catch right-mouse-button-click on tree widget
bool US_Reporter::eventFilter( QObject *obj, QEvent *e )
{
   if ( obj->objectName() == "tree-widget"  &&
        e->type() == QEvent::ContextMenu )
   {  // catch tree row right-mouse click
      rbtn_click = true;
DbgLv(1) << "eventFilter   rbtn_click" << rbtn_click;
      return false;
   }

   else
   {  // pass all others for normal handling
      return US_Widgets::eventFilter( obj, e );
   }
}

// Build up an initial sample tree
void US_Reporter::sample_tree()
{
   QTreeWidgetItem* pitems[ 8 ];
   QTreeWidgetItem* item;
   int wiubase = (int)QTreeWidgetItem::UserType;
   QStringList cvals;

   int wiutype = wiubase + 0;
   cvals.clear();
   cvals << "" << "demo1_veloc" << "Run";

   item = new QTreeWidgetItem( tw_recs, cvals, wiutype );
   pitems[ 0 ] = item;
   item->setCheckState( 0, Qt::PartiallyChecked );

   wiutype = wiubase + 1;
   cvals.clear();
   cvals << "" << "  Cell 2/Channel A/280 nm" << "Data(triple)";
   item = new QTreeWidgetItem( pitems[ 0 ], cvals, wiutype );
   item->setCheckState( 0, Qt::PartiallyChecked );
   pitems[ 1 ] = item;

   wiutype = wiubase + 2;
   cvals.clear();
   cvals << "" << "    Time Derivative" << "Analysis";
   item = new QTreeWidgetItem( pitems[ 1 ], cvals, wiutype );
   item->setCheckState( 0, Qt::PartiallyChecked );
   pitems[ 2 ] = item;

   wiutype = wiubase + 3;
   cvals.clear();
   cvals << "" << "      Analysis Report" << "HTML General Report";
   item = new QTreeWidgetItem( pitems[ 2 ], cvals, wiutype );
   item->setCheckState( 0, Qt::PartiallyChecked );
   pitems[ 3 ] = item;

   wiutype = wiubase + 4;
   cvals.clear();
   cvals << "" << "      Data Scans Plot" << "SVG Plot";
   item = new QTreeWidgetItem( pitems[ 2 ], cvals, wiutype );
   item->setCheckState( 0, Qt::PartiallyChecked );
   pitems[ 3 ] = item;

   wiutype = wiubase + 5;
   cvals.clear();
   cvals << "" << "      Data x-to-radius plot" << "SVG Plot";
   item = new QTreeWidgetItem( pitems[ 2 ], cvals, wiutype );
   item->setCheckState( 0, Qt::Unchecked );
   pitems[ 3 ] = item;

   wiutype = wiubase + 6;
   cvals.clear();
   cvals << "" << "    2-dimensional Spectrum Analysis" << "Analysis";
   item = new QTreeWidgetItem( pitems[ 1 ], cvals, wiutype );
   item->setCheckState( 0, Qt::Checked );
   pitems[ 2 ] = item;

   wiutype = wiubase + 7;
   cvals.clear();
   cvals << "" << "      Analysis Report" << "HTML General Report";
   item = new QTreeWidgetItem( pitems[ 2 ], cvals, wiutype );
   item->setCheckState( 0, Qt::PartiallyChecked );
   pitems[ 3 ] = item;
}

// Bring up a context menu when an item click is right-mouse-button
void US_Reporter::clickedItem( QTreeWidgetItem* item )
{
DbgLv(1) << "clickedItem rbtn_click" << rbtn_click;
   if ( rbtn_click )
   {
      row_context( item );
   }
   rbtn_click = false;
}

// Propagate checked states when one item has its state changed
void US_Reporter::changedItem( QTreeWidgetItem* item, int col )
{
//DbgLv(1) << "changedItem";
   if ( col == 0  &&  change_tree )
   {  // If column 0 changed (state), set new state in tree
      int state = (int)item->checkState( 0 );
      int row   = item->type() - (int)QTreeWidgetItem::UserType;
      DataDesc* pdesc = (DataDesc*)&adescs.at( row );

      state_children( pdesc, state );  // Children get parent's state
      state_parents( pdesc, state );   // Parents get unchk/part-chk/chk

      mark_checked();                  // Reflect checked state in the tree
   }
}

// Build and display a context menu for a right-button-selected row
void US_Reporter::row_context( QTreeWidgetItem* item )
{
   int row = item->type() - (int)QTreeWidgetItem::UserType;
DbgLv(1) << " context menu row" << row + 1;
   QMenu*         cmenu = new QMenu();
   QAction* showact = new QAction( tr( "Show Details" ), this );
   QAction* viewact = new QAction( tr( "View Item" ), this );
   QAction* dataact = new QAction( tr( "Include Table Data" ), this );

   connect( showact, SIGNAL( triggered() ),
            this,    SLOT( item_show()   ) );
   connect( viewact, SIGNAL( triggered() ),
            this,    SLOT( item_view()   ) );
   connect( dataact, SIGNAL( triggered() ),
            this,    SLOT( item_data()   ) );

   cmenu->addAction( showact );
   cmenu->addAction( viewact );
   cmenu->addAction( dataact );

   cmenu->exec( QCursor::pos() );
}

// Build a list of runIDs from result directories with AUC files in them
void US_Reporter::build_runids()
{
   int nruns = 0;
   QString rdir       = US_Settings::resultDir().replace( "\\", "/" );
   QStringList rdirs  = QDir( rdir )
      .entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name );
   QStringList aucfil( "*.auc" );
   int nrdir = rdirs.size();
   rdir      = rdir + "/";

   for ( int ii = 0; ii < nrdir; ii++ )
   {  // Examine the subdirectories of the results directory
      QString     runid  = rdirs.at( ii );
      QString     subdir = rdir + runid;
      // Get the list of AUC files in the subdirectory
      QStringList afiles = QDir( subdir )
         .entryList( aucfil, QDir::Files, QDir::Name );
      int naucf = afiles.size();

      if ( naucf > 0 )
      {  // Add the Run to the list for the Runs combo box
         if ( nruns == 0 )
            sl_runids << "all";        // Prefix the list with "all"

         sl_runids << runid;           // Add a Run to the list
         nruns++;
      }
   }

   cb_runids->addItems( sl_runids );   // Populate the Runs combo box
   cb_runids->setCurrentIndex( 0 );
}

void US_Reporter::new_runid( int row )
{
DbgLv(1) << "new runID row" << row;
   QString runID = sl_runids.at( row );
DbgLv(1) << "  new runID runid" << runID;
DbgLv(1) << "WIDGET SIZE" << size();

   // Build mappings of names,labels
   build_map( QString( "application" ), appmap );
   build_map( QString( "extension" ),   extmap );
   build_map( QString( "report" ),      rptmap );

   int linex  = 0;

   if ( runID == "all" )
   {  // For run "all", loop to build descriptions for all runs
      for ( int ii = 1; ii < sl_runids.size(); ii++ )
      {
         runID  = sl_runids.at( ii );

         build_descs( runID, linex );
      }
   }

   else
      // Build descriptions for a specific run
      build_descs( runID, linex );

   // Rebuild the tree widget
   build_tree();
}

// Build data description records for a specified Run
void US_Reporter::build_descs( QString& runID, int& linex )
{
DbgLv(1) << "build_descs runID" << runID << " linex" << linex;

   QStringList appnames = appmap.keys();
   QStringList extnames = extmap.keys();
   QStringList rptnames = rptmap.keys();

   if ( linex == 0 )
      adescs.clear();              // If first line, clear list

   cdesc.linen       = linex + 1;  // Compose the Run item description
   cdesc.level       = 0;
   cdesc.checkState  = 0;
   cdesc.children    = 0;
   cdesc.label       = runID;
   cdesc.type        = "Run";
   cdesc.filename    = "";
   cdesc.filepath    = US_Settings::reportDir() + "/" + runID + "/";
   cdesc.runid       = runID;
   cdesc.triple      = "all";
   cdesc.analysis    = "all";
   cdesc.lastmodDate = "";

   QString path      = cdesc.filepath;
   adescs << cdesc;                // Update the list and line count
   linex++;
DbgLv(1) << " BD: line lev label" << cdesc.linen << cdesc.level << cdesc.label;

   QString rdir       = US_Settings::resultDir().replace( "\\", "/" )
      + "/" + runID + "/";
   QStringList aucfil( "*.auc" );
   QStringList afiles = QDir( rdir )
         .entryList( aucfil, QDir::Files, QDir::Name );
   int naucf = afiles.size();
DbgLv(1) << " BD:  naucf" << naucf << "aucfil" << aucfil[0];

   for ( int ii = 0; ii < naucf; ii++ )
   {  // Examine the AUC files in the Run's results subdirectory
      QString fname = afiles.at( ii );
      QString trnam = fname.section( ".", -4, -2 ).replace( ".", "" );
      QString tripl = fname.section( ".", -4, -4 ) + " / "
                    + fname.section( ".", -3, -3 ) + " / "
                    + fname.section( ".", -2, -2 );
      QStringList trifil( "*." + trnam + ".*.*" );
      QStringList rfiles = QDir( path )
         .entryList( trifil, QDir::Files, QDir::Name );
      int nrptf = rfiles.size();
DbgLv(1) << " BD:   nrptf" << nrptf << "trifil" << trifil[0];

      if ( nrptf == 0 ) continue;

      // There are reports, so add level-1 (triple) item
      cdesc.linen       = linex + 1;
      cdesc.level       = 1;
      cdesc.label       = tr( "Cell " )
                        + tripl.section( "/", 0, 0 ).simplified()
                        + tr( "/Channel " )
                        + tripl.section( "/", 1, 1 ).simplified() + "/"
                        + tripl.section( "/", 2, 2 ).simplified() + " nm";
      cdesc.type        = "Data(triple)";
      cdesc.filename    = fname;
      cdesc.filepath    = rdir + fname;
      cdesc.triple      = tripl;
      cdesc.analysis    = "all";
      cdesc.lastmodDate = "";

      adescs << cdesc;
      linex++;
DbgLv(1) << " BD: line lev label" << cdesc.linen << cdesc.level << cdesc.label;

      for ( int jj = 0; jj < appnames.size(); jj++ )
      {  // Examine possible application (analysis) names
         QString appname = appnames.at( jj );
         QString aplabel = appmap[ appname ];
                 appname = appname.section( ":", 1, 1 );

         QStringList rafilt( appname + "." + trnam + ".*.*" );
         QStringList rafiles = QDir( path )
            .entryList( rafilt, QDir::Files, QDir::Name );
DbgLv(1) << " BD:   nappf" << rafiles.size() << "rafilt" << rafilt[0];

         if ( rafiles.size() < 1 )  continue;

         // There are reports for this application, add level-2 item
         cdesc.linen       = linex + 1;
         cdesc.level       = 2;
         cdesc.label       = aplabel;
         cdesc.type        = "Analysis";
         cdesc.filename    = rafilt[ 0 ];
         cdesc.filepath    = path + rafilt[ 0 ];
         cdesc.triple      = tripl;
         cdesc.analysis    = aplabel;
         cdesc.lastmodDate = "";

         adescs << cdesc;
         linex++;

         for ( int kk = 0; kk < extnames.size(); kk++ )
         {  // Examine possible extensions
            QString extname = extnames.at( kk );
            QString exlabel = extmap[ extname ];
                    extname = extname.section( ":", 1, 1 );

            for ( int mm = 0; mm < rptnames.size(); mm++ )
            {  // Examine possible reports
               QString rptname = rptnames.at( mm );
               QString rplabel = rptmap[ rptname ];
                       rptname = rptname.section( ":", 1, 1 );

               // Get a specific report file name
               QString rpfname = appname + "." + trnam + "."
                  + rptname + "."  + extname;
//DbgLv(1) << " BD:     nrptf" << rafiles.size() << "rpfname" << rpfname;

               if ( rafiles.indexOf( rpfname ) < 0 )  continue;

               // There are reports of this type, add level-3 item
               cdesc.linen       = linex + 1;
               cdesc.level       = 3;
               cdesc.label       = rplabel;
               cdesc.type        = exlabel;
               cdesc.filename    = rpfname;
               cdesc.filepath    = path + rpfname;
               cdesc.triple      = tripl;
               cdesc.analysis    = aplabel;
               cdesc.lastmodDate = US_Util::toUTCDatetimeText(
                                   QFileInfo( cdesc.filepath ).lastModified()
                                   .toUTC().toString( Qt::ISODate ), true );

               adescs << cdesc;
               linex++;
DbgLv(1) << " BD: line lev label" << cdesc.linen << cdesc.level << cdesc.label;
            }  // END: report names loop
         }  // END:  extension names loop
      }  // END:  application names loop
   }  // END:  triples loop
}

// Build a name,label map from the reports.html file
void US_Reporter::build_map( QString ttag, QMap< QString, QString >& labmap )
{
   int     kmap = 0;
   QString path = US_Settings::appBaseDir() + "/etc/reports.xml";
DbgLv(1) << "build_map:  ttag" << ttag << " path" << path;
   QFile   file( path );

   if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QXmlStreamReader xml( &file );

      while( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == ttag )
         {
            QXmlStreamAttributes a = xml.attributes();
            QString name   = a.value( "name"  ).toString();
            QString label  = a.value( "label" ).toString();
            QString anum   = QString().sprintf( "%3.3i", kmap++ );
            QString akey   = anum + ":" + name;
DbgLv(1) << "     b_m: name label" << name << label;

            labmap[ akey ] = label;
         }
      }

      file.close();
   }
else { qDebug() << "***UNABLE TO OPEN FILE" << path; return; }
int nmap = labmap.count();
QString nam0 = labmap.keys().at( 0 );
QString namn = labmap.keys().at( nmap-1 );
DbgLv(1) << "build_map:  ttag" << ttag << " nmap" << nmap;
DbgLv(1) << "  b_m: name0,label0" << nam0 << labmap[ nam0 ];
DbgLv(1) << "  b_m: namen,labeln" << namn << labmap[ namn ];
}

// Build the tree from newly created descriptions
void US_Reporter::build_tree()
{
   QTreeWidgetItem* pitems[ 8 ];
   QTreeWidgetItem* item;
   int wiubase = (int)QTreeWidgetItem::UserType;
   int wiutype = wiubase + 0;
   int nitems  = adescs.size();
   QStringList cvals;
   QString indent( "          " );

   change_tree = false;
   tw_recs->clear();

   for ( int ii = 0; ii < nitems; ii++ )
   {
      cdesc     = adescs.at( ii );
      int lev   = cdesc.level;
      cvals.clear();
      cvals << "" << indent.left( lev * 2 ) + cdesc.label << cdesc.type;
      wiutype   = wiubase + ii;

      if ( lev > 0 )
      {  // For all beyond first, tree item is attached to a parent item
         item = new QTreeWidgetItem( pitems[ lev - 1 ], cvals, wiutype );
      }

      else
      {  // First item is attached to the base tree root
         item = new QTreeWidgetItem( tw_recs, cvals, wiutype );
      }

      item->setCheckState( 0, Qt::Unchecked );
      pitems[ lev ] = item;
   }

   tw_recs->expandAll();
   tw_recs->resizeColumnToContents( 0 );
   tw_recs->resizeColumnToContents( 1 );
   tw_recs->resizeColumnToContents( 2 );

   if ( nitems > 15 )
      tw_recs->collapseAll();

   resize( 750, 250 );
DbgLv(1) << "WIDGET SIZE" << size();

   change_tree = true;
}

// Count total and checked children (descendants) for an item
void US_Reporter::count_children( DataDesc* idesc, int& nchild, int& nchkd )
{
   int nitems = adescs.size();
   int jndx   = idesc->linen;
   int plev   = idesc->level;
DbgLv(1) << "  CnCh: jndx nitems plev" << jndx << nitems << plev;
   nchild     = 0;
   nchkd      = 0;

   for ( int jj = jndx; jj < nitems; jj++ )
   {  // Examine items from next line to the end
      DataDesc* chdesc = (DataDesc*)&adescs.at( jj );
      int       clev   = chdesc->level;
      
      if ( clev <= plev )  break;   // Break at sibling or end of siblings

      nchild++;                     // Bump count of descendants

      if ( chdesc->checkState == 2 )
         nchkd++;                   // Bump count of checked descendants
DbgLv(1) << "  CnCh: jj clev plev nchild nchkd" << jj << clev << plev
 << nchild << nchkd;
   }

   idesc->children = nchild;        // Update item description with child count

   if ( nchkd == nchild  &&  nchild > 0 )
      idesc->checkState = 2;        // All children checked:  item is checked

   else if ( nchkd > 0 )
      idesc->checkState = 1;        // Some checked:  item is partially checked

   else
      idesc->checkState = 0;        // None checked:  item is unchecked
}

// Set checked state for children (descendants) of an item
void US_Reporter::state_children( DataDesc* idesc, int& state )
{
   int nitems = adescs.size();
   int jndx   = idesc->linen;
   int plev   = idesc->level;
   idesc->checkState = state;       // Set the item's state
DbgLv(1) << "  StCh: items jndx plev state" << nitems << jndx << plev << state;

   if ( jndx == nitems )  return;   // Nothing more to do for the last item

   for ( int jj = jndx; jj < nitems; jj++ )
   {  // Examine items from next one to the end
      DataDesc* chdesc = (DataDesc*)&adescs.at( jj );
      int      clev    = chdesc->level;
DbgLv(1) << "  StCh:   jj clev state" << jj << clev << state;
      
      if ( clev <= plev )  break;   // Break at sibling or end of siblings

      chdesc->checkState = state;   // Make descendant state same as item
   }
}

// Set checked state for parents (ancestors) of an item
void US_Reporter::state_parents( DataDesc* idesc, int& state )
{
   int lndx     = idesc->linen - 2;
   int clev     = idesc->level;
   int nchild   = 0;
   int nchkd    = 0;

   for ( int jj = lndx; jj >= 0; jj-- )
   {  // Examine items from next above back to the first item
      DataDesc* pdesc = (DataDesc*)&adescs.at( jj );
      int plev  = pdesc->level;
DbgLv(1) << "  StPa: jj plev clev state" << jj << plev << clev << state;

      if ( plev >= clev )  continue;  // Ignore children or siblings

      count_children( pdesc, nchild, nchkd );   // Reset parent state
DbgLv(1) << "  StPa:   nchild nchkd" << nchild << nchkd;
   }

   if ( nchkd > 0 )
   {  // If any items are checked, enable View and Save buttons
      pb_view->setEnabled( true );
      pb_save->setEnabled( true );
   }

   else
   {  // If no items are checked, disable View and Save buttons
      pb_view->setEnabled( false );
      pb_save->setEnabled( false );
   }
}

// Mark checked state for all of tree, from updated descriptions
void US_Reporter::mark_checked()
{
   QList< QTreeWidgetItem* > items = tw_recs->findItems( QString( "" ),
         Qt::MatchFixedString | Qt::MatchWrap | Qt::MatchRecursive, 0 );
   QTreeWidgetItem* item;
   change_tree = false;           // Disable slot for item change
//DbgLv(1) << "  MkCk: items size" << items.size();

   for ( int jj = 0; jj < adescs.size(); jj++ )
   {  // Set the tree item state according to the data description setting
//DbgLv(1) << "  MkCk:   jj" << jj;
      item = items.at( jj );
      item->setCheckState( 0, (Qt::CheckState)adescs.at( jj ).checkState );
   }

   change_tree = true;
}

// View
void US_Reporter::view()
{
   write_report();                       // Write the report file

   showHelp.show_html_file( pagepath );  // Display it in a browser
}

// Save
void US_Reporter::save()
{
   if ( write_report() )   // Write the report file; tell user the result
   {
      QMessageBox::information( this, tr( "Composite Report" ),
            tr( "A composite report file has been saved:\n" )
            + pagepath );
   }

   else
   {
      QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
            tr( "Unable to create a composite report file:\n" )
            + pagepath );
   }
}

// Write composite report page
bool US_Reporter::write_report()
{
   // Count checked items:  reports, runs, triples, htmls, plots
   count_reports( );

   if ( nsrpts == 0 )
   {
      QMessageBox::warning( this, tr( "Select Error" ),
            tr( "No reports have been selected" ) );
      return false;
   }

   // Create a composite directory
   if ( nsruns == 1 )
      pagedir   = cdesc.runid;
   else
      pagedir   = "combo";

   pagedir   = pagedir + QDateTime::currentDateTime().toString( "-yyMMddhhmm" );

   QString rptpath = US_Settings::reportDir();
   QString cmppath = rptpath + "/composite";
   pagepath        = cmppath + "/" + pagedir;
   QDir    dirrpt( rptpath );

   if ( ! dirrpt.exists( pagepath ) )
   {
      if ( ! dirrpt.mkpath( pagepath ) )
      {
         QMessageBox::warning( this, tr( "File Error" ),
               tr( "Could not create the directory:\n" ) + pagepath );
         return false;
      }
   }

   pagedir   = pagepath;
   pagepath  = pagedir + "/report_composite.html";

   QString rptpage;

   // Compose the report header
   rptpage   = QString( "<?xml version=\"1.0\"?>\n" );
   rptpage  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   rptpage  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
               "/xhtml1-strict.dtd\">\n";
   rptpage  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
               " xml:lang=\"en\" lang=\"en\">\n";
   rptpage  += "  <head>\n";
   rptpage  += "  <title> Ultrascan III Composite Report </title>\n";
   rptpage  += "  <meta http-equiv=\"Content-Type\" content="
               "\"text/html; charset=iso-8859-1\"/>\n";
   rptpage  += "  <style type=\"text/css\" >\n";
   rptpage  += "    td { padding-right: 1em; }\n";
   rptpage  += "    @media print\n";
   rptpage  += "    {\n";
   rptpage  += "      .page\n";
   rptpage  += "      {\n";
   rptpage  += "        page-break-before: always;\n";
   rptpage  += "      }\n";
   rptpage  += "    }\n";
   rptpage  += "    .parahead\n";
   rptpage  += "    {\n";
   rptpage  += "      font-weight: bold;\n";
   rptpage  += "      font-style:  italic;\n";
   rptpage  += "    }\n";
   rptpage  += "  </style>\n";
   rptpage  += "  </head>\n  <body>\n";

   // Possibly prefix the page with logos
   copy_logos( cmppath );    // Possibly add ./etc with logos
DbgLv(1) << " Post copy_logos hsclogo" << hsclogo;

   int logof = ( hsclogo .isEmpty() ? 0 : 4 )
             + ( becklogo.isEmpty() ? 0 : 2 )
             + ( us3logo .isEmpty() ? 0 : 1 );

   if ( logof != 0 )
   {
      rptpage  += "    <p>\n";

      if ( ( logof & 4 ) != 0 )
         rptpage  += "      <img src=\"" + hsclogo
                   + "\" alt=\"HSC logo\"/>\n";

      if ( ( logof & 2 ) != 0 )
         rptpage  += "      <img src=\"" + becklogo
                   + "\" alt=\"Beckman logo\"/>";

      if ( ( logof & 1 ) != 0 )
      {
         if ( logof > 1 )
            rptpage  += "<br/>";

         rptpage  += "\n      <img src=\"" + us3logo
                   + "\" alt=\"Ultrascan III logo\"/>\n";
      }

      else
         rptpage  += "\n";

      rptpage  += "    </p>\n";
   }

   // Compose the body of the composite report
   QString ppageclass = "    <p class=\"page parahead\">\n";
   QString pheadclass = "    <p class=\"parahead\">\n";
   int  jplot   = 0;

   if ( nsrpts > 1 )
   {  // Multiple reports
      QString tripl  = "";
      QString analys = "";

      if ( nsruns == 1 )
      {  // Single Run
         rptpage   += "    <h2> Reports from Run ";
         rptpage   += se_runids.at( 0 );
         rptpage   += ": </h2>\n\n";
      }

      else
      {  // Multiple Runs
         rptpage   += "    <h2> Reports from Multiple Runs </h2>";
      }

      for ( int ii = 0; ii < nsrpts; ii++ )
      {  // Compose an entry in the composite HTML for each component item
         DataDesc* idesc = (DataDesc*)&adescs.at( se_rptrows.at( ii ) );
         bool is_plot = ( idesc->type.contains( "Plot" ) );

         // Possible set for page printing
         if ( is_plot )
         {  // Is a plot:  new page if 2nd or after non-plot
            if ( jplot != 1  &&  ii > 0 )
            {  // Previous was 2nd plot in div or non-plot
               rptpage   += ppageclass;
               jplot      = 1;       // mark as 1st plot on page
            }

            else
            {  // Previous was the 1st plot on the page
               rptpage   += pheadclass;
               jplot      = 2;       // mark as 2nd plot on page
            }
         }

         else if ( ii > 0 )
         {  // Is not a plot and not the 1st item:  starts a new page
            rptpage   += ppageclass;
            jplot      = 0;          // mark as not a plot
         }

         else
         {
            // Is not a plot and is the 1st item:    not a new page
            rptpage   += pheadclass;
         }

         // Display a title for the item
         rptpage   += "      " + idesc->runid + " &nbsp;&nbsp;&nbsp;";
         rptpage   += idesc->triple + "<br/>\n      ";
         rptpage   += idesc->analysis + "<br/>\n       &nbsp;&nbsp;&nbsp;";
          rptpage   += idesc->label + "\n    </p>\n";

         if ( is_plot )
         {  // The item is a plot, so "<img.." will be used
            QString fileimg = idesc->filename;

            if ( idesc->filename.contains( ".svg" ) )
            {  // For SVG, create a PNG equivalent
               QSvgRenderer svgrend;
               QString   pathsvg = idesc->filepath;
               QString   filesvg = idesc->filename;
                         fileimg = QString( filesvg ).replace( ".svg", ".png" );
               QString   pathimg = pagedir + "/" + fileimg;
               svgrend.load( pathsvg );
               QSize     imgsize = svgrend.defaultSize();
               QPixmap   pixmap( imgsize );
               pixmap.fill( Qt::white );
               QPainter  pa( &pixmap );
               svgrend.render( &pa );            // Render the SVG to a pixmap
DbgLv(1) << " size" << imgsize << " filesvg" << filesvg;
DbgLv(1) << " size" << pixmap.size() << " fileimg" << fileimg;
               if ( ! pixmap.save( pathimg ) )   // Write the pixmap as a PNG
               {
                  QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
                        tr( "Unable to create an svg-to-png file:\n" )
                        + pathimg );
               }

            }

            // Embed the plot in the composite report
            rptpage   += "    <div><img src=\"" + fileimg 
                         + "\" alt=\"" + idesc->label + "\"/></div>\n\n";
         }

         else
         {  // The item is HTML or text, so just copy it into the composite
            QFile fi( idesc->filepath );
            if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
            {
               QTextStream ts( &fi );

               while ( ! ts.atEnd() )
               {
                  QString ln = ts.readLine() + "\n";

                  if ( ln.contains( "</body>" ) )
                  {
                     if ( ln.contains( "</table>" ) )  // strip body/head
                        ln = "</table>\n";
                     else                              // skip body/head
                        continue;
                  }

                  if ( ln.contains( "html>"  )  ||     // skip html/head/body
                       ln.contains( "head>"  )  ||     //  /style
                       ln.contains( "body>"  )  ||
                       ln.contains( "<style" ) )    continue;

                  if ( ln.contains( "<br>" ) )         // correct BR format
                     ln.replace( "<br>", "<br/>" );

                  rptpage += ln;
               }

               rptpage += "\n";
               fi.close();
            }
         }
      }  // END:  Reports loop

      // Complete composite page, output it to a file, and copy components
      rptpage += "  </body>\n</html>";

      QFile flo( pagepath );
      if ( flo.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
      {  // Output the composite page to a file in the composite folder
         QTextStream tso( &flo );
         tso << rptpage;
         flo.close();
      }

      // Copy all report files to the current report subdir
      for ( int ii = 0; ii < nsrpts; ii++ )
      {
         DataDesc* idesc = (DataDesc*)&adescs.at( se_rptrows.at( ii ) );

         QFile::copy( idesc->filepath, pagedir + "/" + idesc->filename );
      }
   }  // END:  Multiple reports

   else
   {  // A single report:  just copy the file to the composite folder
      pagepath  = pagedir + "/" + cdesc.filename;
      QFile::copy( cdesc.filepath, pagepath );
   }

   return true;
}

// Count selected reports:  runIDs, reports, htmls, plots
bool US_Reporter::count_reports()
{
   nsrpts  = 0;           // Clear counts and lists
   nsruns  = 0;
   nshtmls = 0;
   nsplots = 0;
   se_reports.clear();
   se_rptrows.clear();
   se_runids .clear();

   for ( int ii = 0; ii < adescs.size(); ii++ )
   {  // Review data description records
      DataDesc* idesc = (DataDesc*)&adescs.at( ii );

DbgLv(1) << "cnt_rpt: ii lev state" << ii << idesc->level << idesc->checkState;
      if ( idesc->level != 3   ||
           idesc->checkState != 2 )  continue;  // Only reports checked

      if ( nsrpts == 0 )
         cdesc   = *idesc;                      // Save first found

      nsrpts++;                                 // Update reports count,list
      se_reports << idesc->label;
      se_rptrows << ii;

      if ( nsruns == 0  ||  !se_runids.contains( idesc->runid ) )
      {  // Save a list of unique runIDs
         nsruns++;
         se_runids  << idesc->runid;            // Update runs count,list
      }

      if ( idesc->type.contains( "HTML" ) )
         nshtmls++;                             // Update HTMLs count

      else if ( idesc->type.contains( "Plot" ) )
         nsplots++;                             // Update PLOTs count
DbgLv(1) << "cnt_rpt:  ns rpts,runs,htmls,plots" << nsrpts << nsruns
 << nshtmls << nsplots;
   }

   return ( nsrpts > 0 );
}

// View an individual report file
void US_Reporter::item_view()
{
   QString fileexts = tr( "HTML files (*.html);;PLOT files (*svg *png);;"
                          "Report file (*.rpt);;Data files (*.dat);;"
                          "All files (*.*)" );
   int row = tw_recs->currentItem()->type() - (int)QTreeWidgetItem::UserType;
   cdesc   = adescs.at( row );
   QString mtext;

   if ( cdesc.type.contains( "Plot" ) )
   {  // For plots, write an <img ...> line
      mtext = QString( "<head>\n<title>" ) + cdesc.filepath +
              QString( "</title>\n</head>\n<img src=\"" ) + cdesc.filepath +
              QString( "\" alt=\"" + cdesc.label + "\" />\n" );
   }

   else
   {  // For HTML, simple copy the file itself
      QFile fi( cdesc.filepath );
      if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &fi );

         while ( ! ts.atEnd() )
            mtext += ts.readLine() + "\n";

         mtext += "\n";
         fi.close();
      }
   }

   // Display the report (HTML or PLOT) in an editor dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Report Tree Item View" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setHtml( mtext );
   editd->show();
}

// Show details for an item
void US_Reporter::item_show()
{
   QString fileexts = tr( "Text,Log files (*.txt, *.log);;All files (*)" );
   int row = tw_recs->currentItem()->type() - (int)QTreeWidgetItem::UserType;
   cdesc   = adescs.at( row );
   int lsx = cdesc.filepath.lastIndexOf( "/" );
   QString filedir  = cdesc.filepath.left( lsx );

   QString mtext =
      tr( "Data Tree Item at Row %1 -- \n\n" ).arg( row + 1 ) +
      tr( "  Tree Level     : " ) + QString::number( cdesc.level      ) + "\n" +
      tr( "  Check State    : " ) + QString::number( cdesc.checkState ) + "\n" +
      tr( "  Label          : " ) + cdesc.label       + "\n" +
      tr( "  Type           : " ) + cdesc.type        + "\n" +
      tr( "  File Name      : " ) + cdesc.filename    + "\n" +
      tr( "  File Directory : " ) + filedir           + "\n" +
      tr( "  Run ID         : " ) + cdesc.runid       + "\n" +
      tr( "  Triple         : " ) + cdesc.triple      + "\n" +
      tr( "  Analysis       : " ) + cdesc.analysis    + "\n" +
      tr( "  Last Mod Date  : " ) + cdesc.lastmodDate + "\n";

   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Report Tree Item Details" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( "monospace", US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// Open a dialog to export data file(s)
void US_Reporter::item_data()
{
}

// Load a report-selection profile
void US_Reporter::load_profile()
{
   QStringList selects;
   QString rselect;

   // Open a file dialog to get the profile file name
   QString fn = QFileDialog::getOpenFileName( this,
         tr( "Load Report-Select Parameters in:" ),
         US_Settings::appBaseDir() + "/etc",
         tr( "ReportSelect files (rs_*.xml);;"
             "All XML files (*.xml);;"
             "All files (*)" ) );

   if ( fn.isEmpty() )  return;

   QFile xfi( fn );

   if ( xfi.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      selects.clear();
      QXmlStreamReader xml( &xfi );
      QXmlStreamAttributes att;

      while ( ! xml.atEnd() )
      {  // Get all unique analysis+report selections from the XML file
         xml.readNext();

         if ( xml.isStartElement()  &&  xml.name() == "selection" )
         {
            att       = xml.attributes();

            QString analys = att.value( "analysis" ).toString();
            QString report = att.value( "report"   ).toString();

            rselect = analys + report;   // Concatenated selection

            if ( selects.isEmpty()  ||  ! selects.contains( rselect ) )
               selects << rselect;       // Add a new one to the list
         }
      }

      xfi.close();
   }

   for ( int ii = 0; ii < adescs.size(); ii++ )
   {  // Propagate selections to report tree
      DataDesc* idesc = (DataDesc*)&adescs.at( ii );

      if ( idesc->level != 3 )  continue;                  // Only reports

      rselect   = idesc->analysis + idesc->label;          // Potential select
      int state = ( selects.contains( rselect ) ) ? 2 : 0; // Selected?

      state_children( idesc, state );                      // Propagate select
      state_parents( idesc, state );

      mark_checked();
   }
}

// Save a report-selection profile
void US_Reporter::save_profile()
{
   // Open a file dialog to get a name for the profile save file
   QString fn = QFileDialog::getSaveFileName( this,
         tr( "Save Report-Selection Parameters in:" ),
         US_Settings::appBaseDir() + "/etc",
         tr( "ReportSelect files (rs_*.xml);;"
             "All XML files (*.xml);;"
             "All files (*)" ) );

   if ( fn.isEmpty() )  return;

   // Allow variations with/without "rs_" and ".xml"
   fn     = fn.replace( "\\", "/" );
   int jj = fn.lastIndexOf( "/" ) + 1;
   QString fdir = fn.left( jj );
   QString fnam = fn.mid(  jj );

   if ( fn.endsWith( "." ) )
   {
      fn   = fn.left( fn.length() - 1 );
      fnam = fnam.left( fnam.length() - 1 );
   }

   else if ( ! fn.endsWith( ".xml" ) )
   {
      fn   = fn   + ".xml";
      fnam = fnam + ".xml";
   }

   if ( fnam.startsWith( "." ) )
   {
      fn   = fdir + fnam.mid( 1 );
   }

   else if ( ! fnam.startsWith( "rs_" ) )
   {
      fn   = fdir + "rs_" + fnam;
   }

   QFile xfo( fn );

   if ( xfo.exists() )
   {
      if ( QMessageBox::No == QMessageBox::warning( this,
               tr( "Warning" ),
               tr( "Attention:\n"
                   "This file exists already!\n\n"
                   "Do you want to overwrite it?" ),
               QMessageBox::Yes, QMessageBox::No ) )
         return;
   }

   bool saved_ok = false;

   if ( xfo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {  // Write unique selections to the XML file
      QXmlStreamWriter xml( &xfo );
      xml.setAutoFormatting( true );
      xml.writeStartDocument();
      xml.writeDTD         ( "<!DOCTYPE US_ReportSelect>" );
      xml.writeStartElement( "ReportSelect" );
      xml.writeAttribute   ( "version","1.0" );

      for ( int ii = 0; ii < adescs.size(); ii++ )
      {  // Examine all the data records, looking for selections
         DataDesc* idesc = (DataDesc*)&adescs.at( ii );

         if ( idesc->level != 3  ||  idesc->checkState != 2 )  continue;

         // Level-3 (reports) that are selected:  output an element
         xml.writeStartElement( "selection" );
         xml.writeAttribute   ( "analysis", idesc->analysis );
         xml.writeAttribute   ( "report",   idesc->label    );
         xml.writeEndElement  ();   // selection
      }

      xml.writeEndElement  ();   // ReportSelect
      xml.writeEndDocument ();
      xfo.close();
      saved_ok  = true;
   }

   if ( saved_ok )
      QMessageBox::information( this,
            tr( "UltraScan Information" ),
            tr( "Please note:\n\n"
                "The Report-Select Profile was successfully saved to:\n\n" ) +
            fn );

   else
      QMessageBox::information( this,
            tr( "UltraScan Error" ),
            tr( "Please note:\n\n"
                "The Report-Select Profile could not be saved to:\n\n" ) +
            fn );
}

// Create ./etc if need be and put copies of any logos there
void US_Reporter::copy_logos( QString cmppath )
{
   if ( hsclogo.isEmpty()  &&  becklogo.isEmpty()  &&  us3logo.isEmpty() )
      return;                           // Don't bother if no logos exist

   if ( hsclogo .startsWith( ".." )  &&
        becklogo.startsWith( ".." )  &&
        us3logo .startsWith( ".." ) )
      return;                           // Don't bother if already converted

   // Rename logo paths using a relative path
   QString hscorig  = hsclogo;
   QString beckorig = becklogo;
   QString us3orig  = us3logo;
   QString etcpath  = US_Settings::appBaseDir() + "/etc/";
   QString etcnewp  = cmppath + "/etc/";
   QString relpath  = "../etc/";
           hsclogo  = hsclogo .isEmpty() ? hsclogo  :
                      hsclogo .replace( etcpath, relpath );
           becklogo = becklogo.isEmpty() ? becklogo :
                      becklogo.replace( etcpath, relpath );
           us3logo  = us3logo .isEmpty() ? us3logo  :
                      us3logo .replace( etcpath, relpath );
   QString hscnewp  = QString( hscorig  ).replace( etcpath, etcnewp );
   QString becknewp = QString( beckorig ).replace( etcpath, etcnewp );
   QString us3newp  = QString( us3orig  ).replace( etcpath, etcnewp );
DbgLv(1) << "hscOrig" << hscorig;
DbgLv(1) << "hscLogo" << hsclogo;
DbgLv(1) << "hscNewp" << hscnewp;
DbgLv(1) << "etcPath" << etcpath;
DbgLv(1) << "etcNewp" << etcnewp;

   if ( ( hsclogo .isEmpty() || QFile( hscnewp  ).exists() )  &&
        ( becklogo.isEmpty() || QFile( becknewp ).exists() )  &&
        ( us3logo .isEmpty() || QFile( us3newp  ).exists() ) )
      return;                           // Don't bother if logos already exist

   // Need to copy logos, so start by insuring ./composite/etc exists
   QDir dircmp( cmppath );

   if ( ! dircmp.mkpath( etcnewp ) )
   {
      QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
            tr( "Unable to create a composite report directory:\n" )
            + etcnewp );
      return;
   }

   // Copy each existing logo file
   if ( ! hsclogo .isEmpty()  &&  QFile( hscorig  ).exists() )
      QFile::copy( hscorig , hscnewp  );

   if ( ! becklogo.isEmpty()  &&  QFile( beckorig ).exists() )
      QFile::copy( beckorig, becknewp );

   if ( ! us3logo .isEmpty()  &&  QFile( us3orig  ).exists() )
      QFile::copy( us3orig , us3newp  );

}

