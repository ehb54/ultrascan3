//! \file us_reporter.cpp

#include <QApplication>
#include <QtSvg>

#include "us_reporter.h"
#include "us_sync_db.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_editor.h"
#include "us_util.h"
#include "us_sleep.h"
#include "us_dataIO.h"

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
   archdir     = US_Settings::archiveDir() + "/";
   clean_etc_dir();

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
   QPushButton* pb_syncdb  = us_pushbutton( tr( "Sync with Database" ) );
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
   dctlLayout->addWidget( pb_syncdb,  row++, 0, 1, 4 );
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
   connect( pb_syncdb,  SIGNAL( clicked()      ),
            this,       SLOT(   sync_db()      ) );
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
   change_tree       = true;
   tw_recs           = new QTreeWidget();
   QPalette tpal     = pb_help->palette();
   tpal.setColor( QPalette::Base, QColor( Qt::white ) );
   tw_recs->setPalette( tpal );
   tctlLayout->addWidget( tw_recs );

   QStringList theads;
   theads << "Selected" << "Report" << "Item Type";
   ntrows = 5;
   ntcols = theads.size();
   tw_recs->setHeaderLabels( theads );
   tw_recs->setFont( QFont( US_Widgets::fixedFont().family(),
                            US_GuiSettings::fontSize() - 1 ) );
   tw_recs->setObjectName( QString( "tree-widget" ) );
   tw_recs->setAutoFillBackground( true );
   tw_recs->installEventFilter   ( this );

   connect( tw_recs, SIGNAL( itemPressed( QTreeWidgetItem*, int ) ),
            this,    SLOT(   clickedItem( QTreeWidgetItem*      ) ) );
   connect( tw_recs, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
            this,    SLOT(   changedItem( QTreeWidgetItem*, int ) ) );

   // put layouts together for overall layout
   leftLayout->addLayout( dctlLayout );
   leftLayout->addStretch();
   rghtLayout->addLayout( tctlLayout );

   mainLayout->addLayout( leftLayout );
   mainLayout->addLayout( rghtLayout );
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rghtLayout, 8 );

   resize( 1060, 480 );
   show();
   changed = false;
}

// Bring up a context menu when an item click is right-mouse-button
void US_Reporter::clickedItem( QTreeWidgetItem* item )
{
DbgLv(1) << "clickedItem rbtn_click" << rbtn_click;
   if ( QApplication::mouseButtons() == Qt::RightButton )
   {
      row_context( item );
   }
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

      if ( row == 0  &&  state == 2 )
      {
         pb_view->setEnabled( true );
         pb_save->setEnabled( true );
      }

      changed = true;
   }
}

// Build and display a context menu for a right-button-selected row
void US_Reporter::row_context( QTreeWidgetItem* item )
{
   int row = item->type() - (int)QTreeWidgetItem::UserType;
DbgLv(1) << " context menu row" << row + 1;
   QMenu*         cmenu = new QMenu();
   QAction* showact = new QAction( tr( "Show Details" ), this );
   QAction* viewact = new QAction( tr( "View Item" ),    this );
   QAction* saveact = new QAction( tr( "Save As" ),      this );

   connect( showact, SIGNAL( triggered() ),
            this,    SLOT( item_show()   ) );
   connect( viewact, SIGNAL( triggered() ),
            this,    SLOT( item_view()   ) );
   connect( saveact, SIGNAL( triggered() ),
            this,    SLOT( item_save()   ) );

   cmenu->addAction( showact );
   cmenu->addAction( viewact );
   cmenu->addAction( saveact );

   if ( adescs.at( row ).level < 3 )
   {
      viewact->setEnabled( false );
      saveact->setEnabled( false );
   }

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
   rdir      = rdir + "/";

   for ( int ii = 0; ii < rdirs.size(); ii++ )
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

   cb_runids->setMaxVisibleItems( 20 );
   cb_runids->addItems( sl_runids );   // Populate the Runs combo box
   cb_runids->setCurrentIndex( 0 );
}

void US_Reporter::new_runid( int row )
{
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   qApp->processEvents();
   QString runID = sl_runids.at( row );
DbgLv(1) << "  new runID row runid" << row << runID;

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
   changed = true;
   QApplication::restoreOverrideCursor();
   qApp->processEvents();
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
   cdesc.description = "";

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

   for ( int ii = 0; ii <= naucf; ii++ )
   {  // Examine the AUC files in the Run's results subdirectory
      QString fname;
      QString trnam;
      QString tripl;

      if ( ii < naucf )
      {
         fname = afiles.at( ii );
         trnam = fname.section( ".", -4, -2 ).replace( ".", "" );
         tripl = fname.section( ".", -4, -4 ) + " / "
                    + fname.section( ".", -3, -3 ) + " / "
                    + fname.section( ".", -2, -2 );
      }

      else
      {
         fname = "vHW.0Z9999.combo-distrib.svgz";
         trnam = "0Z9999";
         tripl = "0 / Z / 9999";
         cdesc.label = "Combined Analyses";
      }

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
      cdesc.description = "";
      QString trdesc;

      // Attempt to add a triple description to the label
      if ( ii < naucf )
      {
         US_DataIO::RawData rdata;
         int rstat = US_DataIO::readRawData( cdesc.filepath, rdata );
         if ( rstat == US_DataIO::OK )
         {
            trdesc = rdata.description;
         }
         if ( ! trdesc.isEmpty() )
         {
            cdesc.label       = QString( cdesc.label ) + "   " + trdesc;
            cdesc.description = trdesc;
         }
      }

      else
      {
         trdesc            = "Combined Analyses";
         cdesc.label       = trdesc;
         cdesc.description = trdesc;
      }

      adescs << cdesc;
      linex++;
DbgLv(1) << " BD: line lev label" << cdesc.linen << cdesc.level << cdesc.label;

      for ( int jj = 0; jj < appnames.size(); jj++ )
      {  // Examine possible application (analysis) names
         QString appname = appnames.at( jj );
         QString aplabel = appmap[ appname ];
                 appname = appname.section( ":", 1, 1 );
         QString appnmLo = appname.toLower();
         QString appnmUp = appname.toUpper();

         QStringList rafilt( appname + "." + trnam + ".*.*" );
         rafilt  << appnmLo + "." + trnam + ".*.*"
                 << appnmUp + "." + trnam + ".*.*";
         QStringList rafiles = QDir( path )
            .entryList( rafilt, QDir::Files, QDir::Name );
DbgLv(1) << " BD:   nappf" << rafiles.size() << "rafilt" << rafilt[0]
   << rafilt[1] << rafilt[2];

         if ( rafiles.size() < 1 )  continue;

         // There are reports for this application, so add a level-2 item
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
               QString basname = "." + trnam + "." + rptname + "."  + extname;
               QString rpfname = appname + basname;
               QString rpfnmLo = appnmLo + basname;
               QString rpfnmUp = appnmUp + basname;
//DbgLv(1) << " BD:     nrptf" << rafiles.size() << "rpfname" << rpfname;

               if ( rafiles.indexOf( rpfname ) < 0 )
               {  // Skip if name (even lower/upper case version) not in list
                  int jj1 = rafiles.indexOf( appnmLo + basname );
                  int jj2 = rafiles.indexOf( appnmUp + basname );
                  if ( jj1 < 0  &&  jj2 < 0 )
                     continue;
                  rpfname = ( jj1 < 0 ) ? rpfname : rpfnmLo;
                  rpfname = ( jj2 < 0 ) ? rpfname : rpfnmUp;
               }

               if ( rpfname.contains( ".svg" ) )
               {  // Skip if SVG file has PNG equivalent
                  QString rpfnpng = QString( rpfname ).section( ".", 0, -2 )
                                    + ".png";
                  if ( rafiles.indexOf( rpfnpng ) >= 0 )
                     continue;
               }

               // There are reports of this type, so add a level-3 item
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
            QString anum   = QString::asprintf( "%3.3i", kmap++ );
            QString akey   = anum + ":" + name;
DbgLv(1) << "     b_m: name label" << name << label;

            labmap[ akey ] = label;
         }
      }

      file.close();
   }

   else
   {
      QMessageBox::warning( this,
            tr( "UltraScan Error" ),
            tr( "Unable to open the file:\n\n" ) + path );
      return;
   }
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
   int nl2its  = 0;
   QStringList cvals;
   QString indent( "          " );

   change_tree = false;
   tw_recs->clear();

   for ( int ii = 0; ii < nitems; ii++ )
   {  // Loop to add items to the data tree
      cdesc     = adescs.at( ii );
      int lev   = cdesc.level;
      cvals.clear();
      cvals << "" << indent.left( lev * 2 ) + cdesc.label << cdesc.type;
      wiutype   = wiubase + ii;

      if ( lev > 0 )
      {  // For all beyond the first, a tree item is attached to a parent item
         item = new QTreeWidgetItem( pitems[ lev - 1 ], cvals, wiutype );

         if ( lev == 2 )  nl2its++;
      }

      else
      {  // The first item is attached to the base tree root
         item = new QTreeWidgetItem( tw_recs, cvals, wiutype );
      }

      item->setCheckState( 0, Qt::Unchecked );
      pitems[ lev ] = item;
   }

   tw_recs->expandAll();                   // Expand all to resize by contents
   tw_recs->resizeColumnToContents( 0 );
   tw_recs->resizeColumnToContents( 1 );
   tw_recs->resizeColumnToContents( 2 );

   if ( nitems > 15 )
   {  // If there are many items, collapse some or all of them
      if ( nl2its > 10 )
         tw_recs->collapseAll();           // Collapse all if many analyses

      else
      {  // If not so many, collapse only analysis subtrees
         QList< QTreeWidgetItem* > items = tw_recs->findItems( QString( "" ),
            Qt::MatchFixedString | Qt::MatchWrap | Qt::MatchRecursive, 0 );

         for ( int ii = 0; ii < nitems; ii++ )
            if ( adescs.at( ii ).level == 2 )
               tw_recs->collapseItem( items.at( ii ) );
      }
   }

   resize( 1060, 480 );
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
   if ( write_report() )                 // Write the report file
   {  // Open the PDF file for viewing with system's "open-with" app
#ifdef Q_OS_WIN
      QString file_url = QString( "file:///" ) + ppdfpath;
#else
      QString file_url = QString( "file://" ) + ppdfpath;
#endif
      if ( ! QDesktopServices::openUrl( file_url ) )
         QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
               tr( "Unable to open the composite report file:\n" )
               + ppdfpath );
   }

   else
   {  // Report error in creating PDF
      QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
            tr( "Unable to create a composite report file:\n" )
            + ppdfpath );
   }
}

// Save
void US_Reporter::save()
{
   if ( write_report() )   // Write the report file; tell user the result
   {
      QString pagesdir = pagedir;
      QString compdir  = pagedir;
      QString indent   = QString( "     " );
      int jj           = pagedir.lastIndexOf( "/" );
      pagesdir         = pagesdir.mid( jj + 1 );
      compdir          = compdir .left( jj );
      QMessageBox::information( this, tr( "Composite Report" ),
            tr( "In the composite reports folder:\n" )
            + indent + compdir + " ,\n"    
            + tr( "a composite report file has been saved:\n" )
            + indent + "report_composite.pdf ;\n"
            + tr( "with supporting HTML and components in subdirectory:\n" )
            + indent + pagesdir );
   }

   else
   {
      QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
            tr( "Unable to create a composite report file:\n" )
            + ppdfpath );
   }
}

// Write composite report page:  generate HTML, then PDF
bool US_Reporter::write_report()
{
   const int mxpageht = 1100;

   if ( !changed )
      return true;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   qApp->processEvents();
   load_ok    = false;

   // Count checked items:  reports, runs, triples, htmls, plots
   count_reports( );

   if ( nsrpts == 0 )
   {
      QMessageBox::warning( this, tr( "Select Error" ),
            tr( "No reports have been selected" ) );
      QApplication::restoreOverrideCursor();
      qApp->processEvents();
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
         QApplication::restoreOverrideCursor();
         qApp->processEvents();
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
   rptpage  += "    body { background-color: white; }\n";
   rptpage  += "    .pagebreak\n";
   rptpage  += "    {\n";
   rptpage  += "      page-break-before: always; border: 1px solid; \n";
   rptpage  += "    }\n";
   rptpage  += "    .parahead\n";
   rptpage  += "    {\n";
   rptpage  += "      font-weight: bold;\n";
   rptpage  += "      font-style:  italic;\n";
   rptpage  += "    }\n";
   rptpage  += "    .datatext\n";
   rptpage  += "    {\n";
   rptpage  += "      font-family: monospace;\n";
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
   QString ppageclass = "    <p class=\"pagebreak parahead\">\n";
   QString pheadclass = "    <p class=\"parahead\">\n";
   QString dtextclass = "\n    <p class=\"datatext\">\n";
   int  jplot   = 0;

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

   int phght = 0;
   int chght = 0;

   for ( int ii = 0; ii < nsrpts; ii++ )
   {  // Compose an entry in the composite HTML for each component item
      DataDesc* idesc = (DataDesc*)&adescs.at( se_rptrows.at( ii ) );
      bool is_plot = ( idesc->type.contains( "Plot" ) );
      bool is_data = ( idesc->type.contains( "text", Qt::CaseInsensitive ) )
                  || ( idesc->type.contains( "comma", Qt::CaseInsensitive ) );
      phght = chght;

      // Possible set for page printing
      if ( is_plot )
      {  // Is a plot:  new page if 2nd or after non-plot
         if ( idesc->filepath.contains( ".svg" ) )
         {
            QSvgRenderer svgrend;
            svgrend.load( idesc->filepath );
            chght      = svgrend.defaultSize().height();
         }

         else
         {
            chght      = QPixmap( idesc->filepath ).height();
         }
DbgLv(1) << "  plot.height" << chght << idesc->filename << "ph" << phght;

         if ( jplot != 1  &&  ii > 0 )
         {  // Previous was 2nd plot on the page or was a non-plot
//DbgLv(1) << "++jplot" << jplot << "ii" << ii << "NEW PAGE";
            rptpage   += ppageclass;
            jplot      = 1;       // mark as 1st plot on page
         }

         else
         {  // Previous was the 1st plot on the page
            if ( ( chght + phght ) < mxpageht )
            {  // If 2 plots will fit on a page, mark this plot as second
//DbgLv(1) << "++comb.height (NOPAGE)" << (chght+phght) << "mxpageht" << mxpageht;
               rptpage   += pheadclass;
               jplot      = 2;       // mark as 2nd plot on page
            }

            else
            {  // If second plot on page will exceed space, force a new page
DbgLv(1) << "++comb.height" << (chght+phght) << "NEW PAGE" << idesc->filename;
               rptpage   += ppageclass;
               jplot      = 1;       // mark as 1st plot on page
            }
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

      QString trdesc = idesc->triple;
      if ( ! idesc->description.isEmpty() )
      {
         trdesc         = trdesc + " &nbsp;&nbsp;&nbsp;" + idesc->description;
      }

      // Display a title for the item
      rptpage   += "      " + idesc->runid + " &nbsp;&nbsp;&nbsp;";
      rptpage   += trdesc + "<br/>\n      ";
      rptpage   += idesc->analysis + "<br/>\n       &nbsp;&nbsp;&nbsp;";
      rptpage   += idesc->label + "\n    </p>\n";

      if ( is_plot )
      {  // The item is a plot, so "<img.." will be used
         QString fileimg = idesc->filename;

         if ( fileimg.contains( ".svg" ) )
         {  // For SVG[Z], create a PNG equivalent
            QSvgRenderer svgrend;
            QString pathsvg = idesc->filepath;
            QString filesvg = idesc->filename;
            fileimg         = QString( filesvg ).section( ".", 0, -2 ) + ".png";
            QString pathimg = pagedir + "/" + fileimg;
            svgrend.load( pathsvg );
            QSize   imgsize = svgrend.defaultSize();
            QPixmap pixmap( imgsize );
            pixmap.fill( Qt::white );
            QPainter pa( &pixmap );
            svgrend.render( &pa );            // Render the SVG to a pixmap
DbgLv(1) << " size" << imgsize << " filesvg" << filesvg;
DbgLv(1) << " size" << pixmap.size() << " fileimg" << fileimg;
            if ( ! pixmap.save( pathimg ) )   // Write the pixmap as a PNG
            {
               QMessageBox::warning( this, tr( "Composite Report *ERROR*" ),
                     tr( "Unable to create an svg-to-png file:\n" )
                     + pathimg );
            }

            chght    = imgsize.height();

         }

         // Embed the plot in the composite report
         rptpage   += "    <div><img src=\"" + fileimg 
                         + "\" alt=\"" + idesc->label + "\"/></div>\n\n";
      }

      else if ( is_data )
      {  // The item is Data text, so copy it with line breaks added
         QFile fi( idesc->filepath );
         if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {
            rptpage   += dtextclass;
            QTextStream ts( &fi );

            while ( ! ts.atEnd() )
            {
               rptpage += pad_line( ts.readLine() );
            }

            rptpage += "    </p>\n\n";
            fi.close();
         }
      }

      else
      {  // The item is HTML, so copy it with header,footer removed
         QFile fi( idesc->filepath );
         if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {
            QTextStream ts( &fi );
            int stage = 0;

            while ( ! ts.atEnd() )
            {
               QString ln = ts.readLine() + "\n";

               if ( stage == 0 )
               {  // skip early part of component HTML until body
                  if ( ln.contains( "<body>" ) )
                     stage = 1;  // mark that we are now in body
                  continue;
               }

               else if ( stage == 2 )
                  // skip part of component HTML beyond body end
                  continue;

               else if ( ln.contains( "</body>" ) )
               {  // mark end of body
                  stage = 2;
                  if ( ln.contains( "</table>" ) )
                     ln = "</table>\n";
                  else
                     continue;
               }

               if ( ln.contains( "<br>" ) )         // correct BR format
                  ln.replace( "<br>", "<br/>" );

               rptpage += ln;
            }

            rptpage += "\n";
//            rptpage += "\n    <p class=\"page parahead\"></p>\n";
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

   write_pdf();                 // Create the PDF version of the report

   changed = false;
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   return load_ok;
}

// Create a PDF from the main HTML
void US_Reporter::write_pdf()
{
   ppdfpath = QString( pagepath ).replace( ".html", ".pdf" );
   QPrinter printer( QPrinter::HighResolution );
   printer.setOutputFormat  ( QPrinter::PdfFormat );
   printer.setOutputFileName( ppdfpath );
//   printer.setFullPage      ( true );
   printer.setCreator       ( "UltraScan" );
   printer.setDocName       ( QString( "report_composite.html" ) );
   printer.setPageOrientation   ( QPageLayout::Portrait );
   printer.setPageSize     ( QPageSize( QPageSize::A2 ) );

   QString       rpttext;
   QFile         fili( pagepath );
   if ( fili.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      QTextStream   ts( &fili );
      rpttext = ts.readAll();
      fili.close();
   }

   // Use TextDocument to produce PDF
   QString rptpath = US_Settings::reportDir();
   QString cmppath = QString( pagepath ).section( "/", 0, -2 );
   QDir::setCurrent( cmppath );
   QTextDocument document;
   document.setDefaultFont( QFont( "serif", 14 ) );
   QSizeF pgsize = printer.pageLayout().pageSize().size( QPageSize::Point );
   document.setPageSize( pgsize );
   document.setHtml( rpttext );
   document.print( &printer );

   QString ppdffold = ppdfpath;
   ppdfpath         = ppdfpath.section( "/", 0, -3 );
   ppdfpath         = ppdfpath + "/report_composite.pdf";
   QFile pdff( ppdfpath );

   // Overwrite the PDF in the composite folder with the one
   //  in the subdirectory where the HTML and components exist

   if ( pdff.exists() )
      pdff.remove();

   if ( ! QFile::copy( ppdffold , ppdfpath  ) )
   {
      QMessageBox::information( this, tr( "UltraScan Error" ),
            tr( "Unable to (over-)write the file:\n\n" ) + ppdfpath );
   }
   load_ok   = true;
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
   QString fileexts = tr(
         "HTML files (*.html);;PLOT files (*.svgz *.svg *.png);;"
         "Report file (*.rpt);;Data files (*.csv *.dat);;"
         "All files (*.*)" );
   int row = tw_recs->currentItem()->type() - (int)QTreeWidgetItem::UserType;
   cdesc   = adescs.at( row );
   bool    isHTML = true;
   QString mtext;

   if ( cdesc.type.contains( "Plot" ) )
   {  // For plots, write an <img ...> line
      mtext = QString( "<head>\n<title>" ) + cdesc.filepath +
              QString( "</title>\n</head>\n<img src=\"" ) + cdesc.filepath +
              QString( "\" alt=\"" + cdesc.label + "\" />\n" );
   }

   else
   {  // For non-plot, simply copy the file itself
      QFile fi( cdesc.filepath );
      if ( fi.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &fi );

         while ( ! ts.atEnd() )
            mtext += ts.readLine() + "\n";

         mtext += "\n";
         fi.close();
      }

      // Flag as HTML or plain-text
      isHTML   = cdesc.type.contains( "HTML" );
   }

   // Display the report (plot or text) in an editor dialog
   US_Editor* editd = new US_Editor( US_Editor::LOAD, true, fileexts );
   editd->setWindowTitle( tr( "Report Tree Item View" ) );
   editd->move( QCursor::pos() + QPoint( 100, 100 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() ) );
   if ( isHTML )
      editd->e->setHtml( mtext );
   else
      editd->e->setText( mtext );
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
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setText( mtext );
   editd->show();
}

// Open a dialog to save an item data file
void US_Reporter::item_save()
{ 
   int row = tw_recs->currentItem()->type() - (int)QTreeWidgetItem::UserType;
   cdesc   = adescs.at( row );
   int isx = cdesc.filepath.lastIndexOf( "/" ) + 1;
   // Determine base file name, extension, analysis prefix
   QString filename = cdesc.filepath.mid( isx );
   QString fileext  = filename.mid( filename.lastIndexOf( "." ) + 1 );
   QString fileanp  = filename.left( filename.indexOf( "." ) );
   // Determine file types string and default output file path
   QString fileexts = fileext + tr( " files (*." ) + fileext + ");;"
                    + fileanp + tr( " files (" ) + fileanp + "*);;"
                              + tr( "All files (*)" );
   QString ofilname = archdir + filename;

   // Open a dialog and get the Save-As file path name
   QString fn = QFileDialog::getSaveFileName( this,
         tr( "Save Report File As ..." ), ofilname, fileexts );

   if ( fn.isEmpty() )  return;
 
   // Copy the file to its specified archive location; save archive directory
   QFile::copy( cdesc.filepath, fn );

   archdir          = fn.left( fn.lastIndexOf( "/" ) + 1 );
}

// Load a report-selection profile
void US_Reporter::load_profile()
{
   QStringList selects;
   QString rselect;

   // Open a file dialog to get the profile file name
   QString fn = QFileDialog::getOpenFileName( this,
         tr( "Load Report-Select Parameters in:" ),
         US_Settings::etcDir(),
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
         US_Settings::etcDir(),
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

// Synchronize with the database
void US_Reporter::sync_db()
{
   US_SyncWithDB* syncdb = new US_SyncWithDB();

   syncdb->exec();
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

// Pad text line with non-blank spaces to simulate spaces and tabs
QString US_Reporter::pad_line( const QString linein )
{
   QString lineout;
   const QChar cbln( ' ' );
   const QChar ctab( '\t' );
   const QString s_nbsp( "&nbsp;" );
   QChar lchar;
   int   kk = 3;

   for ( int ii = 0; ii < linein.size(); ii++ )
   {
      QChar lchar = linein.at( ii );

      if ( lchar == cbln )           // Replace blank with NBSP
      {
         lineout.append( s_nbsp );
         kk++;
      }

      else if ( lchar == ctab )      // Replace tab with 2-5 NBSPs
      {
         int nspc = 4 - ( kk & 3 );
         kk      += nspc;
         lineout.append( cbln   );
         lineout.append( s_nbsp );

         for ( int jj = 0; jj < nspc; jj++ )
            lineout.append( s_nbsp );
      }

      else                           // Copy all other characters
      {
         lineout.append( lchar );
         kk++;
      }
   }

   lineout += "<br/>\n";

   return lineout;

}

