//! \file us_manage_data.cpp

#include <QApplication>

#include "us_manage_data.h"
#include "us_data_model.h"
#include "us_data_tree.h"
#include "us_data_process.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_editor.h"
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
   //w.show();                   //!< \memberof QWidget
   if ( w.db != NULL )
      return application.exec();  //!< \memberof QApplication
   else
      return -1;
}

// US_ManageData class constructor
US_ManageData::US_ManageData() : US_Widgets()
{
   // set up the GUI

   setWindowTitle( tr( "Manage US DB/Local Data Sets" ) );
   setPalette( US_GuiSettings::frameColor() );
DbgLv(1) << "GUI setup begun";

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
   dbg_level     = US_Settings::us_debug();

   // fill in the GUI components
   int row       = 0;

   pb_invtor     = us_pushbutton( tr( "Investigator" ) );
   dctlLayout->addWidget( pb_invtor, row,   0, 1, 1 );

   le_invtor     = us_lineedit();
   dctlLayout->addWidget( le_invtor, row++, 1, 1, 7 );

   pb_reset      = us_pushbutton( tr( "Reset" ), false );
   dctlLayout->addWidget( pb_reset,  row,   0, 1, 4 );
   connect( pb_reset,   SIGNAL( clicked()     ),
            this,       SLOT  (   reset()     ) );

   pb_scanda     = us_pushbutton( tr( "Scan Data" ) );
   dctlLayout->addWidget( pb_scanda, row++, 4, 1, 4 );
   connect( pb_scanda,  SIGNAL( clicked()     ),
            this,       SLOT( scan_data()     ) );

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

   pb_help       = us_pushbutton( tr( "Help" ) );
   dctlLayout->addWidget( pb_help,   row,   0, 1, 4 );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   dctlLayout->addWidget( pb_close,  row++, 4, 1, 4 );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT(   close()   ) );

   pb_invtor->setToolTip( 
      tr( "Use an Investigator dialog to set the database person ID" ) );
   pb_scanda->setToolTip(
      tr( "Scan Database and Local data, with content analysis" ) );
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
   QFontMetrics fm( te_status->font() );
   int   fontw  = fm.maxWidth();
   int   fonth  = fm.lineSpacing();
   int   minsw  = fontw * 40 + 10;
   int   minsh  = fonth * 18 + 10;
DbgLv(1) << "te_status fw fh  mw mh" << fontw << fonth << " " << minsw << minsh;
   te_status->setMinimumSize( minsw, minsh );
   te_status->adjustSize();

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
   mainLayout->setStretchFactor( leftLayout, 2 );
   mainLayout->setStretchFactor( rghtLayout, 8 );

   show();    // display main window before password dialog appears

   // insure we can connect to the database
   US_Passwd pw;
   db            = new US_DB2( pw.getPasswd() );
   if ( db->lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db->lastError() 
         + tr( "Cannot continue.  Closing" ) );
      db = NULL;
      return;
   }

   personID      = 0;
DbgLv(2) << "db passwd complete";

   // Set default db investigator

   investig = QString::number( US_Settings::us_inv_ID() )
      + ": " + US_Settings::us_inv_name();

   le_invtor->setText( investig );

   find_investigator(  investig );

   connect( pb_invtor,  SIGNAL( clicked()           ),
            this,       SLOT( sel_investigator()    ) );
   connect( le_invtor,  SIGNAL( editingFinished()   ),
            this,       SLOT( chg_investigator() )  );
DbgLv(1) << "GUI setup complete";
   // set up for synchronizing experiments
   //syncExper      = new US_SyncExperiment( db, investig, this );

   // create a class to handle the data itself
   da_model       = new US_DataModel(                      this );

// set needed pointers for class interaction in model object
   da_model->setDatabase( db,         investig  );
   da_model->setProgress( progress,   lb_status );

   // create a class to handle processing the data (upload,download,remove)
   da_process     = new US_DataProcess( da_model,          this );

   // create a class to handle the data tree display
   da_tree        = new US_DataTree(    da_model, tw_recs, this );

   // set needed pointers to sibline classes in model object
   da_model->setSiblings( (QObject*)da_process, (QObject*)da_tree   );
DbgLv(1) << "classes setup complete";

   // set up initial state of GUI
   connect( pb_helpdt,  SIGNAL( clicked()     ),
            da_tree,    SLOT( dtree_help() ) );

   reset();
}

// reset the GUI
void US_ManageData::reset( void )
{
   da_model->dummy_data();
   da_tree->build_dtree();

   reportDataStatus();
}

// filter events to catch right-mouse-button-click on tree widget
bool US_ManageData::eventFilter( QObject *obj, QEvent *e )
{
   if ( obj->objectName() == "tree-widget"  &&
        e->type() == QEvent::ContextMenu )
   {  // catch tree row right-mouse click
      rbtn_click = true;
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

   da_model->setDatabase( db, investig );
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

   da_tree->toggle_expand( "Raw",    show );    // expand/collapse one level up

   if ( show )
   {  // for show edits, need only relabel edit button
      reset_hsbuttons( show, true, false, false );
   }

   else
   {  // for hide edits, must toggle labels below; relabel all buttons
      da_tree->toggle_expand( "Model", show );
      da_tree->toggle_expand( "Noise", show );

      reset_hsbuttons( show, true, true, true );
   }
}

// toggle models between hide/show
void US_ManageData::toggle_models()
{
   bool show = pb_hsmodl->text().startsWith( "Show" ); // show or hide?

   da_tree->toggle_expand( "Edited", show );    // expand/collapse one level up

   if ( show )
   { // for show models, must expand levels above; reset edit,model buttons
      da_tree->toggle_expand( "Raw",    show );

      reset_hsbuttons( show, false, true, true );
   }

   else
   {  // for hide models, must collapse below;  reset model,noise buttons
      da_tree->toggle_expand( "Noise", false );

      reset_hsbuttons( show, false, true, true );
   }
}

// toggle noises between hide/show
void US_ManageData::toggle_noises()
{
   bool show = pb_hsnois->text().startsWith( "Show" )  // show or hide?
            || pb_hsnois->text().startsWith( "Expand" );

   da_tree->toggle_expand( "Model",  show );    // expand/collapse one level up

   if ( show )
   {  // for show noise, must expand above; reset edit,model,noise buttons
      da_tree->toggle_expand( "Edited", show );
      da_tree->toggle_expand( "Raw",    show );

      reset_hsbuttons( show, true, true, true );
   }

   else
   {  // for hide noise, need only reset noise button
      reset_hsbuttons( show, false, false, true );
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

// scan the database and local disk for R/E/M/N data sets
void US_ManageData::scan_data()
{
   da_model->scan_data();            // scan the data

   da_tree ->build_dtree();          // rebuild the data tree with present data

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
   reportDataStatus();

   pb_reset->setEnabled( true );
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
      const QString& /*lname*/, const QString& /*fname*/ )
{
   personID   = invID;

   investig   = QString::number( US_Settings::us_inv_ID() )
      + ": " + US_Settings::us_inv_name();
   le_invtor->setText( investig );
   da_model->setDatabase( db, investig );
}

// handle a right-mouse click of a row cell
void US_ManageData::clickedItem( QTreeWidgetItem* item )
{
//DbgLv(2) << "TABLE ITEM CLICKED rbtn_click" << rbtn_click;

   if ( rbtn_click )
   {  // only bring up context menu if right-mouse-button was clicked
      da_tree->row_context_menu( item );
   }
   rbtn_click        = false;
}

// open a dialog and display data tree help
void US_ManageData::dtree_help()
{
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

// report status of data
void US_ManageData::reportDataStatus()
{
   US_DataModel::DataDesc cdesc;

   // count each type of record
   ncrecs     = da_model->recCount();
   ndrecs     = da_model->recCountDB();
   nlrecs     = da_model->recCountLoc();
   ncraws     = ncedts = ncmods = ncnois = 0;
   ndraws     = ndedts = ndmods = ndnois = 0;
   nlraws     = nledts = nlmods = nlnois = 0;

   for ( int ii = 0; ii < ncrecs; ii++ )
   {
      cdesc   = da_model->row_datadesc( ii );

      if ( cdesc.recType == 1 )
      {
         ncraws++;
         ndraws    += ( cdesc.recState & US_DataModel::REC_DB ) != 0 ? 1 : 0;
         nlraws    += ( cdesc.recState & US_DataModel::REC_LO ) != 0 ? 1 : 0;
      }

      else if ( cdesc.recType == 2 )
      {
         ncedts++;
         ndedts    += ( cdesc.recState & US_DataModel::REC_DB ) != 0 ? 1 : 0;
         nledts    += ( cdesc.recState & US_DataModel::REC_LO ) != 0 ? 1 : 0;
      }

      else if ( cdesc.recType == 3 )
      {
         ncmods++;
         ndmods    += ( cdesc.recState & US_DataModel::REC_DB ) != 0 ? 1 : 0;
         nlmods    += ( cdesc.recState & US_DataModel::REC_LO ) != 0 ? 1 : 0;
      }

      else if ( cdesc.recType == 4 )
      {
         ncnois++;
         ndnois    += ( cdesc.recState & US_DataModel::REC_DB ) != 0 ? 1 : 0;
         nlnois    += ( cdesc.recState & US_DataModel::REC_LO ) != 0 ? 1 : 0;
      }
   }

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

