//! \file us_analysis_profile.cpp

#include "us_analysis_profile.h"
#include "us_table.h"
#include "us_license.h"
#include "us_sleep.h"
#include "us_util.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


// Constructor:  build the main layout with tab widget panels
US_AnalysisProfileGui::US_AnalysisProfileGui() : US_Widgets()
{
   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;

   setWindowTitle( tr( "Define An Analysis Profile" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   QGridLayout* statL     = new QGridLayout();
   QHBoxLayout* buttL     = new QHBoxLayout();

   connection_status   = false;
   automode            = false;

      // Create tab and panel widgets
   tabWidget           = us_tabwidget();

   tabWidget->setTabPosition( QTabWidget::North );

DbgLv(1) << "MAIN: create panels";
   apanGeneral         = new US_AnaprofPanGen   ( this );
DbgLv(1) << "MAIN:  apGE done";
   apan2DSA            = new US_AnaprofPan2DSA  ( this );
DbgLv(1) << "MAIN:  ap2D done";
   apanPCSA            = new US_AnaprofPanPCSA  ( this );
DbgLv(1) << "MAIN:  apPC done";
//   apanStatus          = new US_AnaprofPanStatus( this );
//DbgLv(1) << "MAIN:  apUP done";
   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( apanGeneral,   tr( "1: General" ) );
   tabWidget->addTab( apan2DSA,      tr( "2: 2DSA"    ) );
   tabWidget->addTab( apanPCSA,      tr( "3: PCSA"    ) );
//   tabWidget->addTab( apanStatus,    tr( "9: Status"  ) );
   tabWidget->setCurrentIndex( curr_panx );
DbgLv(1) << "MAIN:  tabs added";

   //tabWidget->tabBar()->setEnabled(false);

   // Add bottom buttons
   pb_help   = us_pushbutton( tr( "Help" ) );
   pb_next   = us_pushbutton( tr( "Next Panel" ) );
   pb_prev   = us_pushbutton( tr( "Previous Panel" ) );
   pb_close  = us_pushbutton( tr( "Close" ) );;
   pb_apply  = us_pushbutton( tr( "Save/Apply" ) );;
   pb_apply->setEnabled( false );

   buttL->addWidget( pb_help  );
   buttL->addWidget( pb_prev  );
   buttL->addWidget( pb_next  );
   buttL->addWidget( pb_close );
   buttL->addWidget( pb_apply );

   // Connect signals to slots
   connect( tabWidget, SIGNAL( currentChanged( int ) ),
            this,      SLOT  ( newPanel      ( int ) ) );
   connect( pb_next,   SIGNAL( clicked()   ),
            this,      SLOT  ( panelUp()   ) );
   connect( pb_prev,   SIGNAL( clicked()   ),
            this,      SLOT  ( panelDown() ) );
   connect( pb_close,  SIGNAL( clicked()       ),
            this,      SLOT  ( close_program() ) );
   connect( pb_apply,  SIGNAL( clicked()       ),
            this,      SLOT  ( apply_profile() ) );
   connect( pb_help,   SIGNAL( clicked()   ),
            this,      SLOT  ( help()      ) );

   main->addWidget( tabWidget );
   main->addLayout( statL );
   main->addLayout( buttL );

   connect( apanGeneral, SIGNAL( set_tabs_buttons_inactive( void ) ),
            this,        SLOT  ( disable_tabs_buttons     ( void ) ) );
   connect( apanGeneral, SIGNAL( set_tabs_buttons_active_readonly( void ) ),
            this,        SLOT  ( enable_tabs_buttons_readonly    ( void ) ) );
   connect( apanGeneral, SIGNAL( set_tabs_buttons_active  ( void ) ),
            this,        SLOT  ( enable_tabs_buttons      ( void ) ) );

   //int min_width = tabWidget->tabBar()->width();

   //setMinimumSize( QSize( min_width, 450 ) );
   setMinimumSize( 950, 450 );
   adjustSize();

   //apanGeneral->initPanel();
   //apanGeneral->loaded_proto = 0;
   //apanGeneral->update_inv();
DbgLv(1) << "MAIN:  CALL check_user_level()";
   apanGeneral->check_user_level();
DbgLv(1) << "MAIN:  CALL check_runname()";
   apanGeneral->check_runname();

DbgLv(1) << "MAIN:  CALL reset()";
//reset();
   resize( 500, 450 );
}


// Reset parameters to their defaults
void US_AnalysisProfileGui::reset( void )
{
  qDebug() << "Resetting internal protocol...";
  currProf = US_AnaProfile();
  initPanels();
  
}

// Set auto mode (comes from ComProject or Experiment)
void US_AnalysisProfileGui::auto_mode_passed( void )
{
   automode            = true;
}

// Set auto protocol and aprofile names
void US_AnalysisProfileGui::auto_name_passed( QString& p_protname, QString& p_aproname )
{
   currProf.protoname   = p_protname;
   currProf.aprofname   = p_aproname;

   if ( automode )
   {
      apanGeneral->disable_name_buttons();
      apanGeneral->pass_names( p_protname, p_aproname );
   }
}

// Update analysis profile based on inherited protocol
void US_AnalysisProfileGui::inherit_protocol( US_RunProtocol* iProto )
{
   int kchn        = currProf.pchans.count();
   QStringList sl_chns;
   QStringList sl_sols;
   QStringList sl_opts;
   int nchs        = iProto->rpSolut.chsols.count();
   int ncho        = iProto->rpOptic.chopts.count();

 DbgLv(1) << "APG00: ipro: kchn nchs ncho" << kchn << nchs << ncho;  

   if ( nchs < 1  ||  ncho < 1 )
     return;
   
   int nchn        = 0;
DbgLv(1) << "APG: ipro: kchn nchs ncho" << kchn << nchs << ncho;
if(iProto->rpOptic.chopts.count()>0)
DbgLv(1) << "APG: ipro: 0)ch s1 s2 s3"
 << iProto->rpOptic.chopts[0].channel
 << iProto->rpOptic.chopts[0].scan1
 << iProto->rpOptic.chopts[0].scan2
 << iProto->rpOptic.chopts[0].scan3; 
if(iProto->rpOptic.chopts.count()>1)
DbgLv(1) << "APG: ipro: 1)ch s1 s2 s3"
 << iProto->rpOptic.chopts[1].channel
 << iProto->rpOptic.chopts[1].scan1
 << iProto->rpOptic.chopts[1].scan2
 << iProto->rpOptic.chopts[1].scan3;

   for ( int ii = 0; ii < nchs; ii++ )
   {  // Examine protocol's channel solutions
      QString chname  = iProto->rpSolut.chsols[ ii ].channel;
      QString sodesc  = iProto->rpSolut.chsols[ ii ].solution;
      chname          = QString( chname ).section( ":", 0, 0 )
                                         .section( ",", 0, 0 )
                                         .replace( " / ", "" );
      sl_chns << chname;
      sl_sols << sodesc;
DbgLv(1) << "APG: ipro:  s:ii" << ii << "chname" << chname << "sodesc" << sodesc;
   }

   for ( int ii = 0; ii < ncho; ii++ )
   {  // Examine protocol's channel optics
      QString chname  = iProto->rpOptic.chopts[ ii ].channel;
      QString scan1   = iProto->rpOptic.chopts[ ii ].scan1;
      QString scan2   = iProto->rpOptic.chopts[ ii ].scan2;
      QString scan3   = iProto->rpOptic.chopts[ ii ].scan3;
DbgLv(1) << "APG: ipro:  o.ii" << ii << "chname" << chname
 << "scan1 scan2 scan3" << scan1 << scan2 << scan3;
      if ( scan1.isEmpty() )  continue;

      QStringList ods;       // Optics descriptions, this channel
      chname          = QString( chname ).section( ":", 0, 0 )
                                         .section( ",", 0, 0 )
                                         .replace( " / ", "" );
      ods << scan1;          // First optics
      if ( ! scan2.isEmpty() )
         ods << scan2;       // Additional optics
      if ( ! scan3.isEmpty() )
         ods << scan3;       // Additional optics
      int chx         = sl_chns.indexOf( chname );
      if ( chx < 0 )          continue;

      QString sodesc  = sl_sols[ chx ];  // Channel's solution

      for ( int jj = 0; jj < ods.count(); jj++ )
      {  // Create a channel entry for each optics type of this channel
         QString opdesc  = ods[ jj ];
         opdesc          = opdesc.replace( "visible", "vis." );
         opdesc          = opdesc.replace( "Rayleigh Interference", "Interf." );
         opdesc          = opdesc.replace( "Fluorescence", "Fluor." );
         QString chentr  = chname + ":" + opdesc + ":" + sodesc;
DbgLv(1) << "APG: ipro:    o.jj" << jj << "chentr" << chentr;

         if ( nchn < kchn )
         {  // Replace channel and channel description
            currProf.pchans  [ nchn ] = chname;
            currProf.chndescs[ nchn ] = chentr;
         }
         else
         {  // Append channel and channel description
            currProf.pchans   << chname;
            currProf.chndescs << chentr;
            int lch         = nchn - 1;
            // Duplicate previous parameter values
            currProf.lc_ratios << currProf.lc_ratios[ lch ];
            currProf.lc_tolers << currProf.lc_tolers[ lch ];
            currProf.l_volumes << currProf.l_volumes[ lch ];
            currProf.lv_tolers << currProf.lv_tolers[ lch ];
         }
         nchn++;
      }
   }

   kchn            = currProf.pchans.count();

   if ( kchn > nchn )
   {  // Drop any Profile channel values left over from previous
      for ( int ii = nchn; ii < kchn; ii++ )
      {
         currProf.pchans   .removeLast();
         currProf.chndescs .removeLast();
         currProf.lc_ratios.removeLast();
         currProf.lc_tolers.removeLast();
         currProf.l_volumes.removeLast();
         currProf.lv_tolers.removeLast();
      }
   }

DbgLv(1) << "APG: ipro: nchn" << nchn << "call pGen iP";
   apanGeneral->initPanel();
}

// Reset parameters to their defaults
void US_AnalysisProfileGui::close_program( void )
{
   emit us_exp_is_closed();
   close();
}

// Apply Profile
void US_AnalysisProfileGui::apply_profile( void )
{
DbgLv(1) << "MN:SL: APPLY_PROFILE";
}

// Set even spacing in the grid layout for all 12 columns
void US_AnalysisProfileGui::setColumnStretches( QGridLayout* genL )
{
   for ( int ii = 0; ii < 12; ii++ )
      genL->setColumnStretch( ii, 1 );
}

// Panel for run and other general parameters
US_AnaprofPanGen::US_AnaprofPanGen( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_AnalysisProfileGui*)topw;
   dbg_level           = US_Settings::us_debug();
   use_db              = ( US_Settings::default_data_location() < 2 );
   panel               = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "1: Specify OD range and other general parameters" ) );
   panel->addWidget( lb_panel );

   // Create layout and GUI components
//   genL            = new QGridLayout();
   genL            = NULL;

   pb_aproname     = us_pushbutton( tr( "Analysis Profile Name" ) );
   pb_protname     = us_pushbutton( tr( "Protocol Name" ) );

   // Set up line edits
   le_aproname     = us_lineedit( "(default)", 0, false );
   le_protname     = us_lineedit( "Test-1234", 0, false );
//   int ihgt        = pb_aproname->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

//   genL       ->setObjectName( "GeneralLayout" );
   pb_aproname->setObjectName( "Aprof Button" );
   le_aproname->setObjectName( "Aprof LineEdit" );
   pb_protname->setObjectName( "Proto Button" );
   le_protname->setObjectName( "Proto LineEdit" );

   // Set defaults
   currProf        = &mainw->currProf;

   // Build the general layout
   build_general_layout( );
QLayout* play=panel->layout();
DbgLv(1) << "APGe: layout-of-panel object:" << play;
QString plname=play->objectName();
DbgLv(1) << "APGe: layout-of-panel name:" << plname;
QList< QObject* > globjs = genL->children();
for (int jj=0; jj<globjs.count(); jj++)
 DbgLv(1) << "APGe: jj" << jj << "object" << globjs[jj]->objectName();

 //check_runname();

   // Do the initialization we do at panel entry
DbgLv(1) << "APGe: CALL initPanel()";
   initPanel();
DbgLv(1) << "APGe:  RTN initPanel()";
}

void US_AnaprofPanGen::build_general_layout()
{
   bool have_genl  = true;
   if ( genL != NULL )
   {
/*
QObject* pwidg=le_aproname->parent();
QList<QObject*> allObjects=pwidg->children();
for ( int ii=0; ii<allObjects.count(); ii++ )
{ QObject* child  = allObjects[ ii ];
  QString objname = child->objectName();
DbgLv(1) << "APGe: inP:   ox" << ii << "oName" << objname;
}
 */
      QObject* pwidg  = le_aproname->parent();
      QList< QObject* > allObjects = pwidg->children();
      for ( int ii = 0; ii < allObjects.count(); ii++ )
      {
         QObject* child  = allObjects[ ii ];
         QString objname = child->objectName();
DbgLv(1) << "APGe: bgL:   ox" << ii << "oName" << objname;
         if ( objname.isEmpty() )
            continue;

         if ( objname.contains( "GeneralLayout" ) )
         {  // Child is the general layout
            have_genl       = true;
DbgLv(1) << "APGe: bgL:    HAVE_GENL=true";
         }

         else if ( have_genl )
         {  // Child is a widget after layout was given
            if ( child->isWidgetType() )
            {
DbgLv(1) << "APGe: bgL:    CHILD DELETE (Widget)";
               genL->removeWidget( (QWidget*)child );
DbgLv(1) << "APGe: bgL:    CHILD DELETED (Widget)";
            }
            else
            {
DbgLv(1) << "APGe: bgL:    CHILD DELETE (Item)";
               genL->removeItem  ( (QLayoutItem*)child );
DbgLv(1) << "APGe: bgL:    CHILD DELETED (Item)";
            }
         }
         else
         {
DbgLv(1) << "APGe: bgL:    CHILD *NOT* DELETED  have_genl" << have_genl;
         }
      }

      scrollArea->takeWidget();
DbgLv(1) << "APGe: bgL:    scrollArea widget *REMOVED*";
      QList< QScrollArea* > ssaa = panel->findChildren< QScrollArea* >();
      if ( ssaa.count() == 1 )
      {
         QWidget* cwidg   = (QWidget*)ssaa[ 0 ];
         panel->removeWidget( cwidg );
DbgLv(1) << "APGe: bgL:    scrollArea child *REMOVED*";
      }
else
DbgLv(1) << "APGe: bgL:    scrollArea children count ZERO";

      delete genL;
      delete scrollArea;
      delete pb_applya;
      //delete containerWidget;
//   scrollArea      = new QScrollArea( this );
//   containerWidget = new QWidget;
//   genL->setContentsMargins ( 2, 2, 2, 2 );
//   containerWidget->setLayout( genL );
//   scrollArea->setWidgetResizable( true );
//   scrollArea->setWidget( containerWidget );
//   panel->addWidget( scrollArea );
   }

   genL            = new QGridLayout();
   genL->setObjectName( "GeneralLayout" );
   le_lcrats.clear();
   le_lctols.clear();
   le_ldvols.clear();
   le_lvtols.clear();
   le_daends.clear();

   // Start building main layout
   int row         = 0;

   genL->addWidget( pb_aproname,     row,    0, 1, 3 );
   genL->addWidget( le_aproname,     row++,  3, 1, 6 );
   genL->addWidget( pb_protname,     row,    0, 1, 3 );
   genL->addWidget( le_protname,     row++,  3, 1, 6 );

   connect( pb_aproname, SIGNAL( clicked            ( ) ),
            this,        SLOT(   apro_button_clicked( ) ) );
   connect( pb_protname, SIGNAL( clicked            ( ) ),
            this,        SLOT(   prot_button_clicked( ) ) );
   connect( le_aproname, SIGNAL( editingFinished  ( void ) ),
            this,        SLOT(   apro_text_changed( void ) ) );
   connect( le_protname, SIGNAL( editingFinished  ( void ) ),
            this,        SLOT(   prot_text_changed( void ) ) );

   // Build channel lists and rows
   int nchn          = sl_chnsel.count();
DbgLv(1) << "Ge:SL: nchn" << nchn << "sl_chnsel" << sl_chnsel;
   QLabel* lb_chann  = us_label( tr( "CellChannel:\n"
                                     "Optics: Solution" ) );
   lb_chann->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
   QLabel* lb_lcrat  = us_label( tr( "Loading\nRatio" ) );
   QLabel* lb_lctol  = us_label( tr( "+/- %\nToler." ) );
   QLabel* lb_ldvol  = us_label( tr( "Loading\nVol. (" )
                                 + QString( QChar( 181 ) ) + "l)" );
   QLabel* lb_lvtol  = us_label( tr( "+/- %\nToler." ) );
   QLabel* lb_daend  = us_label( tr( "Data End\n(cm)" ) );
           pb_applya = us_pushbutton( tr( "Apply to All" ) );
   lb_chann ->setObjectName( "Chann Label" );
   lb_lcrat ->setObjectName( "LcRat Label" );
   lb_lctol ->setObjectName( "LcTol Label" );
   lb_ldvol ->setObjectName( "LdVol Label" );
   lb_lvtol ->setObjectName( "LvTol Label" );
   lb_daend ->setObjectName( "DaEnd Label" );
   pb_applya->setObjectName( "ApplyAll Button" );

   int lbhgt       = pb_aproname->height() * 2;
   lb_chann->setMaximumHeight( lbhgt );
   lb_lcrat->setMaximumHeight( lbhgt );
   lb_lctol->setMaximumHeight( lbhgt );
   lb_ldvol->setMaximumHeight( lbhgt );
   lb_lvtol->setMaximumHeight( lbhgt );
   lb_daend->setMaximumHeight( lbhgt );

   genL->addWidget( lb_chann, row,    0, 2, 5 );
   genL->addWidget( lb_lcrat, row,    5, 2, 1 );
   genL->addWidget( lb_lctol, row,    6, 2, 1 );
   genL->addWidget( lb_ldvol, row,    7, 2, 1 );
   genL->addWidget( lb_lvtol, row,    8, 2, 1 );
   genL->addWidget( lb_daend, row++,  9, 2, 1 ); row++;
   genL->setRowStretch( 0, 0 );
   genL->setRowStretch( 1, 0 );

   for ( int ii = 0; ii < nchn; ii++ )
   {
      QString schan( sl_chnsel[ ii ] );
      QLineEdit* le_chann = us_lineedit( schan, 0, true  );
      QLineEdit* le_lcrat = us_lineedit( "1.0", 0, false );
      QLineEdit* le_lctol = us_lineedit( "5",   0, false );
      QLineEdit* le_ldvol = us_lineedit( "460", 0, false );
      QLineEdit* le_lvtol = us_lineedit( "10",  0, false );
      QLineEdit* le_daend = us_lineedit( "7.0", 0, false );

      QString stchan      = QString::number( ii );
      le_chann->setObjectName( stchan + ": channel" );
      le_lcrat->setObjectName( stchan + ": loadconc_ratio" );
      le_lctol->setObjectName( stchan + ": loadconc_tolerance" );
      le_ldvol->setObjectName( stchan + ": load_volume" );
      le_lvtol->setObjectName( stchan + ": loadvol_tolerance" );
      le_daend->setObjectName( stchan + ": dataend" );

      le_lcrats << le_lcrat;
      le_lctols << le_lctol;
      le_ldvols << le_ldvol;
      le_lvtols << le_lvtol;
      le_daends << le_daend;

      genL->addWidget( le_chann,  row,    0, 1, 5 );
      genL->addWidget( le_lcrat,  row,    5, 1, 1 );
      genL->addWidget( le_lctol,  row,    6, 1, 1 );
      genL->addWidget( le_ldvol,  row,    7, 1, 1 );
      genL->addWidget( le_lvtol,  row,    8, 1, 1 );
      genL->addWidget( le_daend,  row,    9, 1, 1 );
      if ( ii == 0 )
      {
         genL->addWidget( pb_applya, row++, 10, 1, 2 );
         connect( pb_applya, SIGNAL( clicked       ( ) ),
                  this,      SLOT(   applied_to_all( ) ) );
      }
      else
      {
         row++;
      }

      connect( le_lcrat,    SIGNAL( editingFinished   ( void ) ),
               this,        SLOT(   lcrat_text_changed( void ) ) );
      connect( le_lctol,    SIGNAL( editingFinished   ( void ) ),
               this,        SLOT(   lctol_text_changed( void ) ) );
      connect( le_ldvol,    SIGNAL( editingFinished   ( void ) ),
               this,        SLOT(   ldvol_text_changed( void ) ) );
      connect( le_lvtol,    SIGNAL( editingFinished   ( void ) ),
               this,        SLOT(   lvtol_text_changed( void ) ) );
      connect( le_daend,    SIGNAL( editingFinished   ( void ) ),
               this,        SLOT(   daend_text_changed( void ) ) );
   }

   int ihgt        = pb_aproname->height();
   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );
   genL->setRowStretch( row, 1 );
   genL->addItem( spacer1,  row++,  0, 1, 1 );
   mainw->setColumnStretches( genL );

//   QScrollArea *scrollArea  = new QScrollArea( this );
//   QWidget* containerWidget = new QWidget;
   scrollArea      = new QScrollArea( this );
   containerWidget = new QWidget;
   panel          ->setObjectName( "GeneralPanel" );
   scrollArea     ->setObjectName( "scrollArea" );
   containerWidget->setObjectName( "containerWidget" );
   genL           ->setSpacing         ( 2 );
   genL           ->setContentsMargins ( 2, 2, 2, 2 );
   containerWidget->setLayout( genL );
   scrollArea     ->setWidgetResizable( true );
   scrollArea     ->setWidget( containerWidget );
   panel          ->addWidget( scrollArea );
   adjustSize();
}

int US_AnaprofPanGen::getProfiles( QStringList& prnames,
      QList< QStringList >& prentries )
{
DbgLv(1) << "APGe: getProtos IN";
//   prnames         = pr_names;   // Return a names list copy
//   prentries       = protdata;   // Return an entries list copy
DbgLv(1) << "APGe:  gP: prnames count" << prnames.count()
 << "prdat count" << prentries.count();
   return prnames.count();       // Return the current list count

}

bool US_AnaprofPanGen::updateProfiles( const QStringList )
{
   return true;
}

void US_AnaprofPanGen::disable_name_buttons()
{
   pb_protname->setEnabled( false );
   pb_aproname->setEnabled( false );
}

void US_AnaprofPanGen::pass_names( QString& protname, QString& aproname )
{
   le_protname->setText( protname );
   le_aproname->setText( aproname );
   if ( mainw->automode )
   {
      us_setReadOnly( le_protname, true );
      mainw->pb_help ->setVisible( false );
      mainw->pb_next ->setVisible( false );
      mainw->pb_prev ->setVisible( false );
      mainw->pb_close->setVisible( false );
      mainw->pb_apply->setVisible( false );
   }
}

// General Panel SLOTS

// Analysis Profile button clicked
void US_AnaprofPanGen::apro_button_clicked()
{
DbgLv(1) << "GP:SL: APRO BTN";
//*TEMPORARY
QMessageBox::information( this, "Under Development",
 "This will lead to an AnalysisProfileGui selection dialog" );
//*TEMPORARY
}

// Protocol button clicked
void US_AnaprofPanGen::prot_button_clicked()
{
DbgLv(1) << "GP:SL: PROT BTN";
//*TEMPORARY
QMessageBox::information( this, "Under Development",
 "This will lead to a Protocol selection dialog" );
//*TEMPORARY
}

// Analysis Profile name text changed
void US_AnaprofPanGen::apro_text_changed( )
{
   QString str = le_aproname->text();
DbgLv(1) << "GP:SL: APRO TEXT" << str;
}

// Protocol name text changed
void US_AnaprofPanGen::prot_text_changed()
{
   QString str = le_protname->text();
DbgLv(1) << "GP:SL: PROT TEXT" << str;
}

// Load Concentration Ratio text changed
void US_AnaprofPanGen::lcrat_text_changed( )
{
   QObject* sobj      = sender();
   QString sname      = sobj->objectName();
   int chnx           = sname.section( ":", 0, 0 ).toInt();
   QString str        = le_lcrats[ chnx ]->text();
DbgLv(1) << "GP:SL: LCRAT TEXT" << str << sname << chnx;
}

// Load Concentration Tolerance text changed
void US_AnaprofPanGen::lctol_text_changed( )
{
   QObject* sobj      = sender();
   QString sname      = sobj->objectName();
   int chnx           = sname.section( ":", 0, 0 ).toInt();
   QString str        = le_lctols[ chnx ]->text();
DbgLv(1) << "GP:SL: LCTOL TEXT" << str << sname << chnx;
}

// Load Voluume text changed
void US_AnaprofPanGen::ldvol_text_changed( )
{
   QObject* sobj      = sender();
   QString sname      = sobj->objectName();
   int chnx           = sname.section( ":", 0, 0 ).toInt();
   QString str        = le_ldvols[ chnx ]->text();
DbgLv(1) << "GP:SL: LDVOL TEXT" << str << sname << chnx;
}

// Load Volume Tolerance text changed
void US_AnaprofPanGen::lvtol_text_changed( )
{
   QObject* sobj      = sender();
   QString sname      = sobj->objectName();
   int chnx           = sname.section( ":", 0, 0 ).toInt();
   QString str        = le_lvtols[ chnx ]->text();
DbgLv(1) << "GP:SL: LVTOL TEXT" << str << sname << chnx;
}

// Data End text changed
void US_AnaprofPanGen::daend_text_changed( )
{
   QObject* sobj      = sender();
   QString sname      = sobj->objectName();
   int chnx           = sname.section( ":", 0, 0 ).toInt();
   QString str        = le_daends[ chnx ]->text();
DbgLv(1) << "GP:SL: DAEND TEXT" << str << sname << chnx;
}

// Apply to All button clicked
void US_AnaprofPanGen::applied_to_all()
{
DbgLv(1) << "GP:SL: APPLIED ALL";
//*TEMPORARY
//QMessageBox::information( this, "Under Development",
// "This will lead to all channel rows being populated from row 0" );
//*TEMPORARY

   // Get the string values of the columns in row 0
   QString lcrat  = le_lcrats[ 0 ]->text();
   QString lctol  = le_lctols[ 0 ]->text();
   QString ldvol  = le_ldvols[ 0 ]->text();
   QString lvtol  = le_lvtols[ 0 ]->text();
   QString daend  = le_daends[ 0 ]->text();

   for ( int jj = 1; jj < sl_chnsel.count(); jj++ )
   {  // Replace values in all other rows where row 0 changed
      if ( le_lcrats[ jj ]->text() != lcrat )
         le_lcrats[ jj ]->setText( lcrat );

      if ( le_lctols[ jj ]->text() != lctol )
         le_lctols[ jj ]->setText( lctol );

      if ( le_ldvols[ jj ]->text() != ldvol )
         le_ldvols[ jj ]->setText( ldvol );

      if ( le_lvtols[ jj ]->text() != lvtol )
         le_lvtols[ jj ]->setText( lvtol );

      if ( le_daends[ jj ]->text() != daend )
         le_daends[ jj ]->setText( daend );
   }
}


// Panel for 2DSA parameters
US_AnaprofPan2DSA::US_AnaprofPan2DSA( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_AnalysisProfileGui*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "2: Specify 2DSA Analysis Controls" ) );
   panel->addWidget( lb_panel );
   QGridLayout* genL   = new QGridLayout();

   // Labels and buttons
   QLabel*  lb_chnpro  = us_banner( tr( "Per-Channel Profile" ) );
   QLabel*  lb_chnsel  = us_label ( tr( "Channel [ Chn:Opt:Solut ]" ) );
   QLabel*  lb_smin    = us_label ( tr( "s Minimum:         " ) );
   QLabel*  lb_smax    = us_label ( tr( "s Maximum:         " ) );
   QLabel*  lb_sgrpts  = us_label ( tr( "s Grid Points:     " ) );
   QLabel*  lb_kmin    = us_label ( tr( "f/f0 Minimum:      " ) );
   QLabel*  lb_kmax    = us_label ( tr( "f/f0 Maximum:      " ) );
   QLabel*  lb_kgrpts  = us_label ( tr( "f/f0 Grid Points:  " ) );
   QLabel*  lb_varyvb  = us_label ( tr( "Varying Vbar:      " ) );
   QLabel*  lb_constk  = us_label ( tr( "Constant f/f0:     " ) );
   QLabel*  lb_grreps  = us_label ( tr( "Grid Repetitions:  " ) );
            pb_custmg  = us_pushbutton( tr( "Custom Grid" ) );;
            pb_applya  = us_pushbutton( tr( "Apply to All" ) );;
            pb_nextch  = us_pushbutton( tr( "Next Channel" ) );;

   QLabel*  lb_jflow   = us_banner( tr( "2DSA Job Flow"   ) );
   QLabel*  lb_sumry   = us_label ( tr( "Flow Summary:   "
                                        "2DSA, 2DSA-FM, FITMEN,"
                                        " 2DSA-IT, 2DSA-MC" ) );
   QLabel*  lb_jname1  = us_label ( tr( "2DSA" ) );
   QLabel*  lb_jname2  = us_label ( tr( "2DSA-FM" ) );
   QLabel*  lb_jname3  = us_label ( tr( "FITMEN" ) );
   QLabel*  lb_jname4  = us_label ( tr( "2DSA-IT" ) );
   QLabel*  lb_jname5  = us_label ( tr( "2DSA-MC" ) );
   QLabel*  lb_j1nois  = us_label ( tr( "(TI noise)" ) );
   QLabel*  lb_j2nois  = us_label ( tr( "(TI+RI noise)" ) );
   QLabel*  lb_j4nois  = us_label ( tr( "(TI+RI noise)" ) );
   QLabel*  lb_j2gpts  = us_label ( tr( "Meniscus Grid Points" ) );
   QLabel*  lb_j2mrng  = us_label ( tr( "Meniscus Fit Range (cm)" ) );
   QLabel*  lb_j4iter  = us_label ( tr( "Refinement Iterations" ) );
   QLabel*  lb_j5iter  = us_label ( tr( "Monte-Carlo Iterations" ) );

//   int ihgt            = lb_smin->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Text boxes and other value elements
   le_smin         = us_lineedit( "1", 0, false );
   le_smax         = us_lineedit( "10", 0, false );
   le_sgrpts       = us_lineedit( "64", 0, false );
   le_kmin         = us_lineedit( "1", 0, false );
   le_kmax         = us_lineedit( "5", 0, false );
   le_kgrpts       = us_lineedit( "64", 0, false );
   le_grreps       = us_lineedit( "8", 0, false );
   le_custmg       = us_lineedit( "(none)", 0, false );
   ck_varyvb       = new QCheckBox( "VV", this );
   ck_varyvb->setPalette( US_GuiSettings::normalColor() );
   ck_varyvb->setChecked( false );
   ck_varyvb->setAutoFillBackground( true  );
   le_constk       = us_lineedit( "2", 0, false );

   cb_chnsel       = new QComboBox( this );
   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->addItems( sl_chnsel );
   ck_j1run        = new QCheckBox( tr( "Run" ), this );
   ck_j1run ->setPalette( US_GuiSettings::normalColor() );
   ck_j1run ->setChecked( true );
   ck_j1run ->setAutoFillBackground( true  );
   ck_j2run        = new QCheckBox( tr( "Run" ), this );
   ck_j2run ->setPalette( US_GuiSettings::normalColor() );
   ck_j2run ->setChecked( true );
   ck_j2run ->setAutoFillBackground( true  );
   ck_j3run        = new QCheckBox( tr( "Run" ), this );
   ck_j3run ->setPalette( US_GuiSettings::normalColor() );
   ck_j3run ->setChecked( true );
   ck_j3run ->setAutoFillBackground( true  );
   ck_j4run        = new QCheckBox( tr( "Run" ), this );
   ck_j4run ->setPalette( US_GuiSettings::normalColor() );
   ck_j4run ->setChecked( true );
   ck_j4run ->setAutoFillBackground( true  );
   ck_j5run        = new QCheckBox( tr( "Run" ), this );
   ck_j5run ->setPalette( US_GuiSettings::normalColor() );
   ck_j5run ->setChecked( true );
   ck_j5run ->setAutoFillBackground( true  );
   le_j2gpts       = us_lineedit( "10", 0, false );
   le_j2mrng       = us_lineedit( "0.03", 0, false );
   ck_j3auto       = new QCheckBox( tr( "Auto-pick" ), this );
   ck_j3auto->setPalette( US_GuiSettings::normalColor() );
   ck_j3auto->setChecked( true );
   ck_j3auto->setAutoFillBackground( true  );
   le_j4iter       = us_lineedit( "10", 0, false );
   le_j5iter       = us_lineedit( "100", 0, false );

   // Create main layout rows
   int row     = 0;
   genL->addWidget( lb_chnpro,  row++,  0, 1, 12 );
   genL->addWidget( lb_chnsel,  row,    0, 1,  3 );
   genL->addWidget( cb_chnsel,  row,    3, 1,  7 );
   genL->addWidget( pb_nextch,  row++, 10, 1,  2 );
   genL->addWidget( lb_smin,    row,    0, 1,  3 );
   genL->addWidget( le_smin,    row,    3, 1,  1 );
   genL->addWidget( lb_smax,    row,    4, 1,  3 );
   genL->addWidget( le_smax,    row,    7, 1,  1 );
   genL->addWidget( lb_sgrpts,  row,    8, 1,  3 );
   genL->addWidget( le_sgrpts,  row++, 11, 1,  1 );
   genL->addWidget( lb_kmin,    row,    0, 1,  3 );
   genL->addWidget( le_kmin,    row,    3, 1,  1 );
   genL->addWidget( lb_kmax,    row,    4, 1,  3 );
   genL->addWidget( le_kmax,    row,    7, 1,  1 );
   genL->addWidget( lb_kgrpts,  row,    8, 1,  3 );
   genL->addWidget( le_kgrpts,  row++, 11, 1,  1 );
   genL->addWidget( lb_grreps,  row,    0, 1,  3 );
   genL->addWidget( le_grreps,  row,    3, 1,  1 );
   genL->addWidget( pb_custmg,  row,    4, 1,  3 );
   genL->addWidget( le_custmg,  row++,  7, 1,  5 );
   genL->addWidget( lb_varyvb,  row,    0, 1,  3 );
   genL->addWidget( ck_varyvb,  row,    3, 1,  1 );
   genL->addWidget( lb_constk,  row,    4, 1,  3 );
   genL->addWidget( le_constk,  row,    7, 1,  1 );
   genL->addWidget( pb_applya,  row++, 10, 1,  2 );
//   genL->addItem  ( spacer1,         row++,  0, 1, 12 );

   genL->addWidget( lb_jflow,   row++,  0, 1, 12 );
   genL->addWidget( lb_sumry,   row++,  0, 1, 12 );
   genL->addWidget( lb_jname1,  row,    0, 1,  2 );
   genL->addWidget( ck_j1run,   row,    2, 1,  1 );
   genL->addWidget( lb_j1nois,  row++,  3, 1,  2 );
   genL->addWidget( lb_jname2,  row,    0, 1,  2 );
   genL->addWidget( ck_j2run,   row,    2, 1,  1 );
   genL->addWidget( lb_j2nois,  row,    3, 1,  2 );
   genL->addWidget( lb_j2gpts,  row,    5, 1,  2 );
   genL->addWidget( le_j2gpts,  row,    7, 1,  1 );
   genL->addWidget( lb_j2mrng,  row,    8, 1,  2 );
   genL->addWidget( le_j2mrng,  row++, 10, 1,  1 );
   genL->addWidget( lb_jname3,  row,    0, 1,  2 );
   genL->addWidget( ck_j3run,   row,    2, 1,  1 );
   genL->addWidget( ck_j3auto,  row++,  5, 1,  2 );
   genL->addWidget( lb_jname4,  row,    0, 1,  2 );
   genL->addWidget( ck_j4run,   row,    2, 1,  1 );
   genL->addWidget( lb_j4nois,  row,    3, 1,  2 );
   genL->addWidget( lb_j4iter,  row,    5, 1,  2 );
   genL->addWidget( le_j4iter,  row++,  7, 1,  1 );
   genL->addWidget( lb_jname5,  row,    0, 1,  2 );
   genL->addWidget( ck_j5run,   row,    2, 1,  1 );
   genL->addWidget( lb_j5iter,  row,    5, 1,  2 );
   genL->addWidget( le_j5iter,  row++,  7, 1,  1 );

   mainw->setColumnStretches( genL );

   // Connect signals and slots
   connect( cb_chnsel,    SIGNAL( activated        ( int )  ),
            this,         SLOT  ( channel_selected ( int )  ) );
   connect( pb_nextch,    SIGNAL( clicked          ( )      ),
            this,         SLOT  ( next_channel     ( )      ) );
   connect( le_smin,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( smin_changed     ( )      ) );
   connect( le_smax,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( smax_changed     ( )      ) );
   connect( le_sgrpts,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( sgpoints_changed ( )      ) );
   connect( le_kmin,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( kmin_changed     ( )      ) );
   connect( le_kmax,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( kmax_changed     ( )      ) );
   connect( le_kgrpts,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( kgpoints_changed ( )      ) );
   connect( le_grreps,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( grid_reps_changed( )      ) );
   connect( pb_custmg,    SIGNAL( clicked          ( )      ),
            this,         SLOT  ( cust_grid_clicked( )      ) );
   connect( le_custmg,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( cust_grid_changed( )      ) );
   connect( ck_varyvb,    SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( vary_vbar_checked( bool ) ) );
   connect( le_constk,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( constk_changed   ( )      ) );
   connect( pb_applya,    SIGNAL( clicked          ( )      ),
            this,         SLOT  ( apply_all_clicked( )      ) );
   connect( ck_j1run,     SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( job1_run_checked ( bool ) ) );
   connect( ck_j2run,     SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( job2_run_checked ( bool ) ) );
   connect( le_j2gpts,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( mgpoints_changed ( )      ) );
   connect( le_j2mrng,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( mfrange_changed  ( )      ) );
   connect( ck_j3run,     SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( job3_run_checked ( bool ) ) );
   connect( ck_j3auto,    SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( autopick_checked ( bool ) ) );
   connect( ck_j4run,     SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( job4_run_checked ( bool ) ) );
   connect( le_j4iter,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( rfiters_changed  ( )      ) );
   connect( ck_j5run,     SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( job5_run_checked ( bool ) ) );
   connect( le_j5iter,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( mciters_changed  ( )      ) );

DbgLv(1) << "AP2d: addWidg/Layo II";
   // Complete overall layout
   panel->addLayout( genL );
   panel->addStretch();
   adjustSize();

DbgLv(1) << "AP2d: CALL initPanel()";
   initPanel();
DbgLv(1) << "AP2d:  RTN initPanel()";

//qDebug() << "SCANINT: " << ssvals[ 0 ][ "scanintv" ]  << ", SCANINT FROM rpSpeed: " <<  rpSpeed->ssteps[ 0 ].scanintv;
}

// 2DSA Panel Slots

// Channel Selected
void US_AnaprofPan2DSA::channel_selected( int chnx )
{
DbgLv(1) << "2D:SL: CHAN_SEL" << chnx;
   int lndx    = sl_chnsel.count() - 1;
   pb_nextch->setEnabled( chnx < lndx );
}
void US_AnaprofPan2DSA::next_channel( )
{
DbgLv(1) << "2D:SL: NEXT_CHAN";
   int lndx    = sl_chnsel.count() - 1;
   int chnx    = qMin( cb_chnsel->currentIndex() + 1, lndx );
   cb_chnsel->setCurrentIndex( chnx );
   pb_nextch->setEnabled( chnx < lndx );
}
void US_AnaprofPan2DSA::smin_changed( )
{
DbgLv(1) << "2D:SL: SMIN_CHG";
}
void US_AnaprofPan2DSA::smax_changed( )
{
DbgLv(1) << "2D:SL: SMAX_CHG";
}
void US_AnaprofPan2DSA::sgpoints_changed( )
{
DbgLv(1) << "2D:SL: SGPTS_CHG";
}
void US_AnaprofPan2DSA::kmin_changed( )
{
DbgLv(1) << "2D:SL: KMIN_CHG";
}
void US_AnaprofPan2DSA::kmax_changed( )
{
DbgLv(1) << "2D:SL: KMAX_CHG";
}
void US_AnaprofPan2DSA::kgpoints_changed( )
{
DbgLv(1) << "2D:SL: KGPTS_CHG";
}
void US_AnaprofPan2DSA::grid_reps_changed( )
{
DbgLv(1) << "2D:SL: GRDREPS_CHG";
}
void US_AnaprofPan2DSA::cust_grid_clicked( )
{
DbgLv(1) << "2D:SL: CUSTG_CLK";
}
void US_AnaprofPan2DSA::cust_grid_changed( )
{
DbgLv(1) << "2D:SL: CUSTG_CHG";
}
void US_AnaprofPan2DSA::vary_vbar_checked( bool chkd )
{
DbgLv(1) << "2D:SL: VVBAR_CKD" << chkd;
}
void US_AnaprofPan2DSA::constk_changed( )
{
DbgLv(1) << "2D:SL: CONSTK_CHG";
}
void US_AnaprofPan2DSA::apply_all_clicked( )
{
DbgLv(1) << "2D:SL: APLALL_CLK";
}
void US_AnaprofPan2DSA::job1_run_checked( bool chkd )
{
DbgLv(1) << "2D:SL: JOB1RUN_CKD" << chkd;
}
void US_AnaprofPan2DSA::job2_run_checked( bool chkd )
{
DbgLv(1) << "2D:SL: JOB2RUN_CKD" << chkd;
}
void US_AnaprofPan2DSA::mgpoints_changed( )
{
DbgLv(1) << "2D:SL: J2MGPT_CHG";
}
void US_AnaprofPan2DSA::mfrange_changed( )
{
DbgLv(1) << "2D:SL: J2MRNG_CHG";
}
void US_AnaprofPan2DSA::job3_run_checked( bool chkd )
{
DbgLv(1) << "2D:SL: JOB3RUN_CKD" << chkd;
}
void US_AnaprofPan2DSA::autopick_checked( bool chkd )
{
DbgLv(1) << "2D:SL: J3AUTO_CKD" << chkd;
}
void US_AnaprofPan2DSA::job4_run_checked( bool chkd )
{
DbgLv(1) << "2D:SL: JOB4RUN_CKD" << chkd;
}
void US_AnaprofPan2DSA::rfiters_changed( )
{
DbgLv(1) << "2D:SL: J4RFITER_CHG";
}
void US_AnaprofPan2DSA::job5_run_checked( bool chkd )
{
DbgLv(1) << "2D:SL: JOB5RUN_CKD" << chkd;
}
void US_AnaprofPan2DSA::mciters_changed( )
{
DbgLv(1) << "2D:SL: J5_MCITER_CHG";
}


// Panel for PCSA parameters
US_AnaprofPanPCSA::US_AnaprofPanPCSA( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
DbgLv(1) << "APpc: IN";
   mainw               = (US_AnalysisProfileGui*)topw;
   dbg_level           = US_Settings::us_debug();
   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   QLabel* lb_panel    = us_banner( tr( "3: Define PCSA Analysis Controls" ) );
   panel->addWidget( lb_panel );
   ck_nopcsa       = new QCheckBox( tr( "Do NOT Include PCSA Among Analyses" ), this );
   ck_nopcsa  ->setPalette( US_GuiSettings::normalColor() );
   ck_nopcsa  ->setChecked( false );
   ck_nopcsa  ->setAutoFillBackground( true  );
   panel->addWidget( ck_nopcsa );
   QGridLayout* genL   = new QGridLayout();

   QLabel* lb_curvtype = us_label ( tr( "Curve Type:" ) );
   QLabel* lb_xaxistyp = us_label ( tr( "X Axis Type:" ) );
   QLabel* lb_yaxistyp = us_label ( tr( "Y Axis Type:" ) );
   QLabel* lb_zaxistyp = us_label ( tr( "Z Axis Type:" ) );
   QLabel* lb_zvalue   = us_label ( tr( "Z Value:" ) );
   QLabel* lb_xmin     = us_label ( tr( "X Minimum:" ) );
   QLabel* lb_xmax     = us_label ( tr( "X Maximum:" ) );
   QLabel* lb_ymin     = us_label ( tr( "Y Minimum:" ) );
   QLabel* lb_ymax     = us_label ( tr( "Y Maximum:" ) );
   QLabel* lb_varcount = us_label ( tr( "Variations Count:" ) );
   QLabel* lb_grfiters = us_label ( tr( "Grid Fit Iterations:" ) );
   QLabel* lb_crpoints = us_label ( tr( "Curve Resolution Points:" ) );
   QLabel* lb_regalpha = us_label ( tr( "Regularization Alpha:" ) );
   QLabel* lb_mciters  = us_label ( tr( "Monte-Carlo Iterations:" ) );
   QLabel* lb_tinoise  = us_label ( tr( "Fit Time-Invariant Noise:" ) );
   QLabel* lb_rinoise  = us_label ( tr( "Fit Radially-Invariant Noise:" ) );
   QLabel* lb_chnsel   = us_label ( tr( "Channel [ Chn:Opt:Solut ]" ) );
   QLabel* lb_tregtype = us_label ( tr( "Tikhonov Regularization:" ) );
           pb_applya   = us_pushbutton( tr( "Apply to All" ) );;
           pb_nextch   = us_pushbutton( tr( "Next Channel" ) );;

   QStringList sl_curvtype;
   sl_curvtype << tr( "All (IS + DS + SL)" )
               << tr( "Increasing Sigmoid" )
               << tr( "Decreasing Sigmoid" )
               << tr( "Straight Line" )
               << tr( "Horizontal Line [ C(s) ]" )
               << tr( "Second Order Power Law" );
   QStringList sl_axistype;
   sl_axistype << "s" << "f/f0" << "mw" << "vbar" << "D";
   QStringList sl_zaxistyp;
   sl_zaxistyp << "vbar" << "f/f0" << "mw";

   cb_curvtype     = new QComboBox( this );
   cb_curvtype->addItems( sl_curvtype );
   cb_xaxistyp     = new QComboBox( this );
   cb_xaxistyp->addItems( sl_axistype );
   le_xmin         = us_lineedit( "1", 0, false );
   le_xmax         = us_lineedit( "10", 0, false );
   cb_yaxistyp     = new QComboBox( this );
   cb_yaxistyp->addItems( sl_axistype );
   cb_yaxistyp->setCurrentIndex( 1 );
   le_ymin         = us_lineedit( "1", 0, false );
   le_ymax         = us_lineedit( "5", 0, false );
   cb_zaxistyp     = new QComboBox( this );
   cb_zaxistyp->addItems( sl_zaxistyp );
   le_zvalue       = us_lineedit( "0.732", 0, false );
   le_varcount     = us_lineedit( "6", 0, false );
   le_grfiters     = us_lineedit( "3", 0, false );
   le_crpoints     = us_lineedit( "200", 0, false );
   ck_tregspec     = new QCheckBox( tr( "On-specified" ), this );
   ck_tregspec->setPalette( US_GuiSettings::normalColor() );
   ck_tregspec->setChecked( false );
   ck_tregspec->setAutoFillBackground( true  );
   ck_tregauto     = new QCheckBox( tr( "On-auto" ), this );
   ck_tregauto->setPalette( US_GuiSettings::normalColor() );
   ck_tregauto->setChecked( false );
   ck_tregauto->setAutoFillBackground( true  );
   le_regalpha     = us_lineedit( "0", 0, true );
   us_setReadOnly( le_regalpha, true );
   le_mciters      = us_lineedit( "0", 0, false );
   ck_tinoise      = new QCheckBox( "TI", this );
   ck_tinoise ->setPalette( US_GuiSettings::normalColor() );
   ck_tinoise ->setChecked( false );
   ck_tinoise ->setAutoFillBackground( true  );
   ck_rinoise      = new QCheckBox( "RI", this );
   ck_rinoise ->setPalette( US_GuiSettings::normalColor() );
   ck_rinoise ->setChecked( false );
   ck_rinoise ->setAutoFillBackground( true  );
   QLabel*  lb_chnpro  = us_banner( tr( "Per-Channel Profile" ) );
   cb_chnsel       = new QComboBox( this );
   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->addItems( sl_chnsel );
//   int ihgt            = lb_curvtype->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Create main layout rows
   int row             = 0;
   genL->addWidget( lb_chnpro,   row++,  0, 1, 12 );
   genL->addWidget( lb_chnsel,   row,    0, 1,  3 );
   genL->addWidget( cb_chnsel,   row,    3, 1,  7 );
   genL->addWidget( pb_nextch,   row++, 10, 1,  2 );
   genL->addWidget( lb_curvtype, row,    0, 1,  3 );
   genL->addWidget( cb_curvtype, row,    3, 1,  3 );
   genL->addWidget( pb_applya,   row++, 10, 1,  2 );
   genL->addWidget( lb_xaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_xaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_xmin,     row,    4, 1,  3 );
   genL->addWidget( le_xmin,     row,    7, 1,  1 );
   genL->addWidget( lb_xmax,     row,    8, 1,  3 );
   genL->addWidget( le_xmax,     row++, 11, 1,  1 );
   genL->addWidget( lb_yaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_yaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_ymin,     row,    4, 1,  3 );
   genL->addWidget( le_ymin,     row,    7, 1,  1 );
   genL->addWidget( lb_ymax,     row,    8, 1,  3 );
   genL->addWidget( le_ymax,     row++, 11, 1,  1 );
   genL->addWidget( lb_zaxistyp, row,    0, 1,  3 );
   genL->addWidget( cb_zaxistyp, row,    3, 1,  1 );
   genL->addWidget( lb_zvalue,   row,    4, 1,  3 );
   genL->addWidget( le_zvalue,   row,    7, 1,  1 );
   genL->addWidget( lb_varcount, row,    8, 1,  3 );
   genL->addWidget( le_varcount, row++, 11, 1,  1 );
   genL->addWidget( lb_grfiters, row,    0, 1,  3 );
   genL->addWidget( le_grfiters, row,    3, 1,  1 );
   genL->addWidget( lb_crpoints, row,    4, 1,  3 );
   genL->addWidget( le_crpoints, row++,  7, 1,  1 );
   genL->addWidget( lb_tinoise,  row,    4, 1,  3 );
   genL->addWidget( ck_tinoise,  row,    7, 1,  1 );
   genL->addWidget( lb_rinoise,  row,    8, 1,  3 );
   genL->addWidget( ck_rinoise,  row++, 11, 1,  1 );
   genL->addWidget( lb_tregtype, row,    0, 1,  3 );
   genL->addWidget( ck_tregspec, row,    3, 1,  2 );
   genL->addWidget( ck_tregauto, row,    5, 1,  2 );
   genL->addWidget( lb_regalpha, row,    8, 1,  3 );
   genL->addWidget( le_regalpha, row++, 11, 1,  1 );
   genL->addWidget( lb_mciters,  row,    0, 1,  3 );
   genL->addWidget( le_mciters,  row++,  3, 1,  1 );

   mainw->setColumnStretches( genL );
   panel->addLayout( genL );
   panel->addStretch();

   connect( ck_nopcsa,    SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( nopcsa_checked   ( bool ) ) );
   connect( cb_chnsel,    SIGNAL( activated        ( int )  ),
            this,         SLOT  ( channel_selected ( int )  ) );
   connect( pb_nextch,    SIGNAL( clicked          ( )      ),
            this,         SLOT  ( next_channel     ( )      ) );
   connect( cb_curvtype,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( curvtype_selected( int )  ) );
   connect( pb_applya,    SIGNAL( clicked          ( )      ),
            this,         SLOT  ( apply_all_clicked( )      ) );
   connect( cb_xaxistyp,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( xaxis_selected   ( int )  ) );
   connect( le_xmin,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( xmin_changed     ( )      ) );
   connect( le_xmax,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( xmax_changed     ( )      ) );
   connect( cb_yaxistyp,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( yaxis_selected   ( int )  ) );
   connect( le_ymin,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( ymin_changed     ( )      ) );
   connect( le_ymax,      SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( ymax_changed     ( )      ) );
   connect( cb_zaxistyp,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( zaxis_selected   ( int )  ) );
   connect( le_zvalue,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( zvalue_changed   ( )      ) );
   connect( le_varcount,  SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( varcount_changed ( )      ) );
   connect( le_grfiters,  SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( grfiters_changed ( )      ) );
   connect( le_crpoints,  SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( crpoints_changed ( )      ) );
   connect( ck_tinoise,   SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( tinoise_checked  ( bool ) ) );
   connect( ck_rinoise,   SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( rinoise_checked  ( bool ) ) );
   connect( ck_tregspec,  SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( tregspec_checked ( bool ) ) );
   connect( ck_tregauto,  SIGNAL( toggled          ( bool ) ),
            this,         SLOT  ( tregauto_checked ( bool ) ) );
   connect( le_regalpha,  SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( alpha_changed    ( )      ) );
   connect( le_mciters,   SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( mciters_changed  ( )      ) );

   // Do first pass at initializing the panel layout
   initPanel();
QString pval1 = sibSValue( "rotor", "rotor" );
DbgLv(1) << "APpc: rotor+rotor=" << pval1;
}

// PCSA Panel Slots

// NO PCSA Checked
void US_AnaprofPanPCSA::nopcsa_checked( bool chkd )
{
DbgLv(1) << "PC:SL: NOPCSA_CKD" << chkd;
}
// Channel Selected
void US_AnaprofPanPCSA::channel_selected( int chnx )
{
DbgLv(1) << "PC:SL: CHAN_SEL" << chnx;
   int lndx    = sl_chnsel.count() - 1;
   pb_nextch->setEnabled( chnx < lndx );
}
// Next Channel
void US_AnaprofPanPCSA::next_channel( )
{
DbgLv(1) << "PC:SL: NEXT_CHAN";
   int lndx    = sl_chnsel.count() - 1;
   int chnx    = qMin( cb_chnsel->currentIndex() + 1, lndx );
   cb_chnsel->setCurrentIndex( chnx );
   pb_nextch->setEnabled( chnx < lndx );
}
void US_AnaprofPanPCSA::apply_all_clicked( )
{
DbgLv(1) << "PC:SL: APLALL_CLK";
}
// Curve Type Selected
void US_AnaprofPanPCSA::curvtype_selected( int curx )
{
DbgLv(1) << "PC:SL: CTYPE_SEL" << curx;
}
// X Axis Selected
void US_AnaprofPanPCSA::xaxis_selected( int xaxx )
{
DbgLv(1) << "PC:SL: XAXIS_SEL" << xaxx;
}
// X Min Changed
void US_AnaprofPanPCSA::xmin_changed( )
{
DbgLv(1) << "PC:SL: XMIN_CHG";
}
// X Max Changed
void US_AnaprofPanPCSA::xmax_changed( )
{
DbgLv(1) << "PC:SL: XMAX_CHG";
}
// Y Axis selected
void US_AnaprofPanPCSA::yaxis_selected( int yaxx )
{
DbgLv(1) << "PC:SL: YAXIS_SEL" << yaxx;
}
// Y Min changed
void US_AnaprofPanPCSA::ymin_changed( )
{
DbgLv(1) << "PC:SL: YMIN_CHG";
}
// Y Max changed
void US_AnaprofPanPCSA::ymax_changed( )
{
DbgLv(1) << "PC:SL: YMAX_CHG";
}
// Z Axis type selected
void US_AnaprofPanPCSA::zaxis_selected( int zaxx )
{
DbgLv(1) << "PC:SL: ZAXIS_SEL" << zaxx;
}
// Z Value changed
void US_AnaprofPanPCSA::zvalue_changed( )
{
DbgLv(1) << "PC:SL: ZVAL_CHG";
}
// Variations Count changed
void US_AnaprofPanPCSA::varcount_changed( )
{
DbgLv(1) << "PC:SL: VCNT_CHG";
}
// Grid Fit Iterations changed
void US_AnaprofPanPCSA::grfiters_changed( )
{
DbgLv(1) << "PC:SL: GFITER_CHG";
}
// Curve Resolution Points changed
void US_AnaprofPanPCSA::crpoints_changed( )
{
DbgLv(1) << "PC:SL: CRPTS_CHG";
}
// TI Noise checked
void US_AnaprofPanPCSA::tinoise_checked( bool chkd )
{
DbgLv(1) << "PC:SL: TNOI_CKD" << chkd;
}
// RI Noise checked
void US_AnaprofPanPCSA::rinoise_checked( bool chkd )
{
DbgLv(1) << "PC:SL: RNOI_CKD" << chkd;
}
// Regularization Type: On-Specified checked
void US_AnaprofPanPCSA::tregspec_checked( bool chkd )
{
DbgLv(1) << "PC:SL: TRSPEC_CKD" << chkd;
   if ( chkd )
   {  // On-Specified checked, so uncheck On-Auto
      ck_tregauto->setChecked( false );
   }
   // Alpha read-only except when On-Specified is checked
   us_setReadOnly( le_regalpha, !ck_tregspec->isChecked() );
}
// Regularization Type: On-Auto checked
void US_AnaprofPanPCSA::tregauto_checked( bool chkd )
{
DbgLv(1) << "PC:SL: TRAUTO_CKD" << chkd;
   if ( chkd )
   {  // On-Auto checked, so uncheck On-Specified
      ck_tregspec->setChecked( false );
   }
   // Alpha read-only except when On-Specified is checked
   us_setReadOnly( le_regalpha, !ck_tregspec->isChecked() );
}
// Regularization Alpha value changed
void US_AnaprofPanPCSA::alpha_changed( )
{
DbgLv(1) << "PC:SL: ALPHA_CHG";
}
// Monte-Carlo Iterations changed
void US_AnaprofPanPCSA::mciters_changed ( )
{
DbgLv(1) << "PC:SL: MCITER_CHG";
}

