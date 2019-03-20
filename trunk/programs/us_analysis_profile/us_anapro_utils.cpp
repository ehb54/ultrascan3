//! \file us_anapro_utils.cpp

// This source file is meant to hold those portions of the
// US_AnalysisProfileGui class and its subclasses that are common
// utility functions used by the class and its subclasses.
// Thereby, the length of any one source file is reduced and the
// us_analysis_profile.cpp file contains only very class-specific source
//
// The division is somewhat arbitrary. The functions contained herein
// include:
//       childSValue();      (Main class)
//       childIValue();      (Main class)
//       childDValue();      (Main class)
//       childLValue();      (Main class)
//       newPanel();         (Main class)
//       statUpdate();       (Main class)
//       panelUp();          (Main class)
//       panelDown();        (Main class)
//       help();             (both Main and subclasses)
//       getSValue();        (subclasses)
//       getIValue();        (subclasses)
//       getDValue();        (subclasses)
//       getLValue();        (subclasses)
//       sibSValue();        (subclasses)
//       sibIValue();        (subclasses)
//       sibDValue();        (subclasses)
//       sibLValue();        (subclasses)
//       initPanel();        (subclasses)
//       savePanel();        (subclasses)
//       status();           (subclasses)

#include "us_analysis_profile.h"
#include "us_ana_profile.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_crypto.h"
#include "us_db2.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#define setCbCurrentText(a,b)  a->setCurrentIndex(a->findText(b))
#else
#define setCbCurrentText(a,b)  a->setCurrentText(b)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//========================= Start: Main      section =========================

// Return a string parameter value from a US_AnalysisProfileGui child panel
QString US_AnalysisProfileGui::childSValue( const QString child, const QString type )
{
   QString value( "" );
   if      ( child == "general"  ) { value = apanGeneral->getSValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getSValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getSValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getSValue( type ); }
   return value;
}

// Return an integer parameter value from a US_AnalysisProfileGui child panel
int US_AnalysisProfileGui::childIValue( const QString child, const QString type )
{
   int value      = 0;
   if      ( child == "general"  ) { value = apanGeneral->getIValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getIValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getIValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getIValue( type ); }
   return value;
}

// Return a double parameter value from a US_AnalysisProfileGui child panel
double US_AnalysisProfileGui::childDValue( const QString child, const QString type )
{
   double value   = 0.0;
   if      ( child == "general"  ) { value = apanGeneral->getDValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getDValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getDValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getDValue( type ); }
   return value;
}

// Return a stringlist parameter value from a US_AnalysisProfileGui child panel
QStringList US_AnalysisProfileGui::childLValue( const QString child, const QString type )
{
   QStringList value;

   if      ( child == "general"  ) { value = apanGeneral->getLValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getLValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getLValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getLValue( type ); }

   return value;
}

// Slot to handle a new panel selected
void US_AnalysisProfileGui::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx;
   // Save any changes in the old current panel
   if      ( curr_panx == panx )  return;  // No change in panel

   if      ( curr_panx == 0 ) apanGeneral->savePanel();
   else if ( curr_panx == 1 ) apan2DSA   ->savePanel();
   else if ( curr_panx == 2 ) apanPCSA   ->savePanel();
DbgLv(1) << "newPanel   savePanel done";

   // Initialize the new current panel after possible changes
   if      ( panx == 0 )      apanGeneral->initPanel();
   else if ( panx == 1 )      apan2DSA   ->initPanel();
   else if ( panx == 2 )      apanPCSA   ->initPanel();
   {
      if ( panx - curr_panx > 1 )
      {
         apanPCSA    ->initPanel();
         apanPCSA    ->savePanel();
      }
      apanGeneral->initPanel();
   }
DbgLv(1) << "newPanel   initPanel done";
   
   curr_panx              = panx;         // Set new current panel
   
   // Update status flag for all panels
   statUpdate();
DbgLv(1) << "newPanel   statUpdate done";
}

// Slot to update status flag for all panels
void US_AnalysisProfileGui::statUpdate()
{
DbgLv(1) << "statUpd: IN stat" << statflag;
   statflag   = apanGeneral->status()
              + apan2DSA   ->status()
              + apanPCSA   ->status();
DbgLv(1) << "statUpd:  MOD stat" << statflag;
}

// Slot to advance to the next panel
void US_AnalysisProfileGui::panelUp()
{
   int newndx = tabWidget->currentIndex() + 1;
   int maxndx = tabWidget->count() - 1;
   newndx     = ( newndx > maxndx ) ? 0 : newndx;
DbgLv(1) << "panUp: newndx, maxndx" << newndx << maxndx;
   tabWidget->setCurrentIndex( newndx );
}

// Slot to retreat to the previous panel
void US_AnalysisProfileGui::panelDown()
{
   int newndx = tabWidget->currentIndex() - 1;
   newndx     = ( newndx < 0 ) ? tabWidget->count() - 1 : newndx;
DbgLv(1) << "panDown: newndx" << newndx;
   tabWidget->setCurrentIndex( newndx );
}

// Open manual help appropriate to the current panel
void US_AnalysisProfileGui::help( void )
{
   if      ( curr_panx == 0 ) apanGeneral ->help();
   else if ( curr_panx == 1 ) apan2DSA    ->help();
   else if ( curr_panx == 2 ) apanPCSA    ->help();
//   else if ( curr_panx == 3 ) apanStatus  ->help();
}

//Slot to DISABLE tabs and Next/Prev buttons
void US_AnalysisProfileGui::disable_tabs_buttons( void )
{
DbgLv(1) << "DISBLING Tabs...";
   pb_next   ->setEnabled( false );
   pb_prev   ->setEnabled( false );
  
   for ( int ii = 1; ii < tabWidget->count(); ii++ )
   {
      tabWidget ->setTabEnabled( ii, false );
      tabWidget ->tabBar()->setTabTextColor( ii, Qt::darkGray );
   }

   qApp->processEvents();
  
}

//Slot to ENABLE tabs and Next/Prev buttons
void US_AnalysisProfileGui::enable_tabs_buttons( void )
{
DbgLv(1) << "ENABLING!!!";
   pb_next   ->setEnabled(true);
   pb_prev   ->setEnabled(true);

   for ( int ii = 1; ii < tabWidget->count(); ii++ )
   {
      tabWidget ->setTabEnabled( ii, true );
      QPalette pal = tabWidget ->tabBar()->palette();
      tabWidget ->tabBar()->setTabTextColor( ii, pal.color(QPalette::WindowText) ); // Qt::black
   }
}

//Slot to ENABLE tabs and Next/Prev buttons, but make all Widgets read-only
void US_AnalysisProfileGui::enable_tabs_buttons_readonly( void )
{
DbgLv(1) << "ENABLING!!!";
   pb_next   ->setEnabled( true );
   pb_prev   ->setEnabled( true );
  
 
   for ( int ii = 1; ii < tabWidget->count(); ii++ )
   {
      tabWidget ->setTabEnabled( ii, true );
      QPalette pal = tabWidget ->tabBar()->palette();
//DbgLv(1) << "PALETTE: " << pal.color(QPalette::WindowText);
      tabWidget ->tabBar()->setTabTextColor( ii, pal.color( QPalette::WindowText ) ); // Qt::black
      
      QWidget* pWidget= tabWidget->widget( ii );
      
      //Find all children of each Tab in QTabWidget [children of all types...]
      QList< QPushButton* > allPButtons = pWidget->findChildren< QPushButton* >();
      QList< QComboBox* >   allCBoxes   = pWidget->findChildren< QComboBox* >();
      QList< QSpinBox* >    allSBoxes   = pWidget->findChildren< QSpinBox* >();
      QList< QwtCounter* >  allCounters = pWidget->findChildren< QwtCounter* >();
      QList< QCheckBox* >   allChBoxes  = pWidget->findChildren< QCheckBox* >();

      // and so on ..
      
      for ( int jj = 0; jj < allPButtons.count(); jj++ )
      {
         if ( ( allPButtons[ jj ]->text()).contains( "View Solution Details" ) || 
              ( allPButtons[ jj ]->text()).contains( "View Current Range Settings" ) ||
              ( allPButtons[ jj ]->text()).contains( "View Experiment Details" ) ||
              ( allPButtons[ jj ]->text()).contains( "Test Connection" ) )
            allPButtons[ jj ]->setEnabled( true );
         else
            allPButtons[ jj ]->setEnabled( false );
      }
      
      for ( int jj = 0; jj < allCBoxes.count(); jj++ )
      {
         if ( (allCBoxes[jj]->currentText()).contains("Speed Profile") )
            allCBoxes[jj]->setEnabled(true);
         else
            allCBoxes[jj]->setEnabled(false);
      }
      for (int jj = 0; jj < allSBoxes.count(); jj++ )
         allSBoxes[jj]  ->setEnabled(false);
      for (int jj = 0; jj < allCounters.count(); jj++ )
         allCounters[jj]->setEnabled(false); 
      for (int jj = 0; jj < allChBoxes.count(); jj++ )
         allChBoxes[jj] ->setEnabled(false);
      // and so on ..
      
   }

}

int  US_AnalysisProfileGui::getProfiles( QStringList& par1,
                                   QList< QStringList >& par2 )
{ return apanGeneral->getProfiles( par1, par2 ); }

bool US_AnalysisProfileGui::updateProfiles( const QStringList par1 )
{ return apanGeneral->updateProfiles( par1 ); }

// Initialize all panels after loading a new protocol
void US_AnalysisProfileGui::initPanels()
{
DbgLv(1) << "AP:iP: IN initPanels()";
   apanGeneral  ->initPanel();
DbgLv(1) << "AP:iP: pG return";
   apan2DSA     ->initPanel();
DbgLv(1) << "AP:iP: p2 return";
   apanPCSA     ->initPanel();
DbgLv(1) << "AP:iP: pP return";
//   apanStatus   ->initPanel();
//DbgLv(1) << "AP:iP: pP return";
}

//========================= End:   Main      section =========================


//========================= Start: General   section =========================

// Initialize a General panel, especially after clicking on its tab
void US_AnaprofPanGen::initPanel()
{
#if 0
use_db=false;
#endif
   // Populate GUI settings from protocol,analysis controls
   le_protname   ->setText ( currProf->protoname );
   le_aproname   ->setText ( currProf->aprofname );
DbgLv(1) << "APGe: inP: aname pname" << currProf->aprofname << currProf->protoname;

DbgLv(1) << "APGe: inP: CALL check_user_level()";
   check_user_level();
DbgLv(1) << "APGe: inP:  RTN check_user_level()";
 
   int nchan      = currProf->pchans.count();
DbgLv(1) << "APGe: inP: nchan" << nchan;
   sl_chnsel.clear();

   // Recompose the list of channel descriptions from protocol
   for ( int ii = 0; ii < nchan; ii++ )
   {
      QString chann  = currProf->pchans  [ ii ];
      QString chdesc = currProf->chndescs[ ii ];
      QString chopts = QString( chdesc ).section( ":", 1, 1 );
      QString chname = QString( chdesc ).section( ":", 0, 0 )
                                        .section( ",", 0, 0 );

      // Skip adding this channel if Optics is unspecified
      // or not installed or if channel B of Interference
      if ( ( chopts.contains( tr( "(unspecified)"   ) ) )  ||
           ( chopts.contains( tr( "(not installed)" ) ) )  ||
           ( chopts.contains( "nterf" )  &&
             chname.contains( "B" ) ) )
         continue;

      // Otherwise, recompose "channel:optics:solution" and add
      QString chsolu = QString( chdesc ).section( ":", 2, 2 );
      QString chnsel = chname + ":" + chopts + ":" + chsolu;

      sl_chnsel << chnsel;
DbgLv(1) << "APGe: inP:  ch" << ii << "chdesc" << chdesc
 << "chnsel" << chnsel;
   }
//*DEBUG*
QObject* pwidg=le_aproname->parent();
QList<QObject*> allObjects=pwidg->children();
for ( int ii=0; ii<allObjects.count(); ii++ )
{ QObject* child  = allObjects[ ii ];
  QString objname = child->objectName();
DbgLv(1) << "APGe: inP:   ox" << ii << "oName" << objname;
}
//*DEBUG*

   // Reset layout
   delete genL;
   genL            = new QGridLayout();
   genL->setObjectName( "GeneralLayout" );

   // Build the General Layout
   build_general_layout( );

}

// Check the Run name
void US_AnaprofPanGen::check_runname()
{
#if 0
   QString rname     = le_runid->text();
   if ( rname.isEmpty() )
      emit set_tabs_buttons_inactive();
   else
   {
      if ( !le_project->text().isEmpty() )
      emit set_tabs_buttons_active();
   }
   qApp->processEvents();
#endif
}

// Update the investigator
void US_AnaprofPanGen::update_inv( void )
{
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
//qDebug() << "USCFG: UpdInv: ERROR connect";
      QMessageBox::information( this,
         tr( "Error" ),
         tr( "Error making the DB connection.\n" ) );

      return;
   }

   QStringList q( "get_user_info" );
   db.query( q );
   db.next();

   int ID        = db.value( 0 ).toInt();
   QString fname = db.value( 1 ).toString();
   QString lname = db.value( 2 ).toString();
   int     level = db.value( 5 ).toInt();

   qDebug() << "USCFG: UpdInv: ID,name,lev" << ID << fname << lname << level;
//if(ID<1) return;

   US_Settings::set_us_inv_name ( lname + ", " + fname );
   US_Settings::set_us_inv_ID   ( ID );
   US_Settings::set_us_inv_level( level );
}


//IF USER cannot edit anything (low-level user)
void US_AnaprofPanGen::check_user_level()
{
   //update_inv();
DbgLv(1) << "APGe:ckul: level" << US_Settings::us_inv_level();
   if ( US_Settings::us_inv_level() < 3 )
   {
      pb_aproname->setEnabled( false );
      pb_protname->setEnabled( false );
   
//      if ( !loaded_prof )
//         emit set_tabs_buttons_inactive();
//      else
//         emit set_tabs_buttons_active_readonly();
   
DbgLv(1) << "SIGNAL!!!!" ; 
   }
}

// Save panel controls when about to leave the panel
void US_AnaprofPanGen::savePanel()
{
DbgLv(1) << "APge: svP: IN";
   // Populate protocol controls from GUI settings
   currProf->protoname  = le_protname->text();
   currProf->aprofname  = le_aproname->text();
DbgLv(1) << "APge: svP:  done";
}

// Get a specific panel string value
QString US_AnaprofPanGen::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "aprofname" )  { value = currProf->aprofname; }
   else if ( type == "protoname" )  { value = currProf->protoname; }
   else if ( type == "aprofGUID" )  { value = currProf->aprofGUID; }
   else if ( type == "protoGUID" )  { value = currProf->protoGUID; }

   return value;
}

// Get a specific panel integer value
int US_AnaprofPanGen::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "dbdisk"  )    { value = use_db ? 1 : 0; }
   else if ( type == "aprofID" )    { value = currProf->aprofID; }
   else if ( type == "protoID" )    { value = currProf->protoID; }
   else if ( type == "nchan"   )    { value = sl_chnsel.count(); }
   return value;
}

// Get a specific panel double value
double US_AnaprofPanGen::getDValue( const QString type )
{
   double value   = 0.0;
   int nchan      = sl_chnsel.count();
   int kk         = nchan - 1;

   if      ( type == "none"    )    { value = 0.0; }
   else if ( type == "f_lcrat"   )
   {  value = ( nchan > 0 ) ? currProf->lc_ratios[  0 ] : 0.0; }
   else if ( type == "l_lcrat"   )
   {  value = ( nchan > 0 ) ? currProf->lc_ratios[ kk ] : 0.0; }
   else if ( type == "f_lctol"   )
   {  value = ( nchan > 0 ) ? currProf->lc_tolers[  0 ] : 0.0; }
   else if ( type == "l_lctol"   )
   {  value = ( nchan > 0 ) ? currProf->lc_tolers[ kk ] : 0.0; }
   else if ( type == "f_ldvol"   )
   {  value = ( nchan > 0 ) ? currProf->l_volumes[  0 ] : 0.0; }
   else if ( type == "l_ldvol"   )
   {  value = ( nchan > 0 ) ? currProf->l_volumes[ kk ] : 0.0; }
   else if ( type == "f_lvtol"   )
   {  value = ( nchan > 0 ) ? currProf->lv_tolers[  0 ] : 0.0; }
   else if ( type == "l_lvtol"   )
   {  value = ( nchan > 0 ) ? currProf->lv_tolers[ kk ] : 0.0; }

   return value;
}

// Get specific panel list values
QStringList US_AnaprofPanGen::getLValue( const QString type )
{
   QStringList value( "" );
   int nchan      = sl_chnsel.count();

   if      ( type == "channels" )       { value = sl_chnsel; }
   else if ( type == "lcratios" )
   {
      for ( int ii = 0; ii < nchan; ii++ )
      {
         value << QString::number( currProf->lc_ratios[ ii ] );
      }
   }
   else if ( type == "lctolers" )
   {
      for ( int ii = 0; ii < nchan; ii++ )
      {
         value << QString::number( currProf->lc_tolers[ ii ] );
      }
   }
   else if ( type == "lvolumes" )
   {
      for ( int ii = 0; ii < nchan; ii++ )
      {
         value << QString::number( currProf->l_volumes[ ii ] );
      }
   }
   else if ( type == "lvtolers" )
   {
      for ( int ii = 0; ii < nchan; ii++ )
      {
         value << QString::number( currProf->lv_tolers[ ii ] );
      }
   }

   return value;
}

// Get a specific panel string value from a sibling panel
QString US_AnaprofPanGen::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_AnaprofPanGen::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_AnaprofPanGen::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_AnaprofPanGen::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_AnaprofPanGen::status()
{
   bool is_done  = !( currProf->aprofname.isEmpty()  ||
                      currProf->protoname.isEmpty() );
   return ( is_done ? 1 : 0 );
}

//========================= End:   General   section =========================


//========================= Start: 2DSA      section =========================

// Initialize a Speeds panel, especially after clicking on its tab
void US_AnaprofPan2DSA::initPanel()
{
   ap2DSA             = &(mainw->currProf.ap2DSA);
DbgLv(1) << "AP2d:   iP: IN";

#if 0
   // Populate GUI settings from protocol controls
//   nchan                = ap2DSA ->nchan;
#endif

   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->clear();
   cb_chnsel->addItems( sl_chnsel );
   bool was_changed     = changed;       // Save changed state
   
   changed              = was_changed;   // Restore changed state

}

// Save panel controls when about to leave the panel
void US_AnaprofPan2DSA::savePanel()
{
DbgLv(1) << "AP2d:svP: IN";
   // Populate protocol 2DSA controls from internal panel control
#if 0
   nspeed                = ssvals.count();
   ap2DSA->nstep        = nspeed;
   ap2DSA->spin_down    = ck_endoff->isChecked();
   ap2DSA->radial_calib = ck_radcal->isChecked();
DbgLv(1) << "AP2d:svP: nspeed" << nspeed;
#endif
}

// Get a specific panel value
QString US_AnaprofPan2DSA::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "what?" )
   {
      value = "what";
   }
   else if ( type == "changed" )
   {
      value       = changed ? "1" : "0";
   }

   return value;
}

// Get a specific panel integer value
int US_AnaprofPan2DSA::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "nchan"   ) { value = ap2DSA->nchan;  }
   if ( type == "changed" ) { value = changed ? 1 : 0; }
   return value;
}

// Get a specific panel double value
double US_AnaprofPan2DSA::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_AnaprofPan2DSA::getLValue( const QString type )
{
   QStringList value( "" );

DbgLv(1) << "AP2d:getLV: type" << type;
#if 0
   if ( type == "profiles" )
   {  // Compose list of all speed-step values (4 lines per step)
      value.clear();
   }
#endif

   return value;
}

// Get a specific panel value from a sibling panel
QString US_AnaprofPan2DSA::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_AnaprofPan2DSA::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_AnaprofPan2DSA::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_AnaprofPan2DSA::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_AnaprofPan2DSA::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=true;
   return ( is_done ? 4 : 0 );
}

//========================= End:   2DSA      section =========================


//========================= Start: PCSA      section =========================

// Initialize a Cells panel, especially after clicking on its tab
void US_AnaprofPanPCSA::initPanel()
{
   apPCSA             = &(mainw->currProf.apPCSA);

   QStringList sl_bals;                         // Counterbalance choices
   sl_bals << tr( "empty (counterbalance)" )
           << tr( "Beckman counterbalance" )
           << tr( "Titanium counterbalance" )
           << tr( "Fluorescence 5-channel counterbalance" );

   // Possibly rebuild Cells protocol if there was a rotor change
DbgLv(1) << "EGCe:inP: prb: nchan" << apPCSA->nchan;
//   rebuild_Cells();

   sl_chnsel       = sibLValue( "general", "channels" );
   cb_chnsel->clear();
   cb_chnsel->addItems( sl_chnsel );

   // Now build cell rows from protocol
   int nchan           = apPCSA->nchan;
   int kchan           = 0;
DbgLv(1) << "EGCe:inP: nchan" << nchan;


   // Insure rows beyond nholes are totally invisible

DbgLv(1) << "EGCe:inP: kchan" << kchan << "nchan" << nchan;
   nchan               = kchan;
}

// Save Cells panel controls when about to leave the panel
void US_AnaprofPanPCSA::savePanel()
{
DbgLv(1) << "APPc:svP: IN";
   // Recount cells used and fill vector based on GUI state
//   int nholes          = sibIValue( "rotor", "nholes" );
   int nchan           = 0;
   QStringList ulabs;
   QString txtempty    = tr( "empty" );

   // Now fill vector of used cell entries
   apPCSA->nchan      = nchan;
//   apPCSA->used.resize( nchan );

}

// Get a specific panel value
QString US_AnaprofPanPCSA::getSValue( const QString type )
{
DbgLv(1) << "EGCe:getSV: type" << type;
   QString txtempty = tr( "empty" );
   QString value( "" );
   int nchan    = apPCSA->nchan;

   if ( type == "nchan" )
   {
      value        = QString::number( nchan );
   }
   else if ( type == "alldone" )
   {
      value        = ( nchan > 0 ) ? "1" : "0";
   }
DbgLv(1) << "EGCe:getSV: type" << type << "value" << value;

   return value;
}

// Get a specific panel integer value
int US_AnaprofPanPCSA::getIValue( const QString type )
{
   int value      = 0;

   if      ( type == "nchan" )   { value = apPCSA->nchan; }
   else if ( type == "alldone" ) { value = ( apPCSA->nchan > 0 ) ? 1 : 0; }
   else if ( type == "nchans" )
   {
      int nchans     = 0;

      for ( int ii = 0; ii < apPCSA->nchan; ii++ )
      {
//         QString centp  = apPCSA->used[ ii ].centerpiece;
//         int chx        = centp.indexOf( "-channel" );

//         if ( chx > 0 )
//         {
//            int ncchn      = QString( centp ).left( chx ).section( " ", -1, -1 )
//                                             .simplified().toInt();
//            nchans        += ncchn;
//         }
      }

      value          = nchans;
   }

DbgLv(1) << "EGCe:getIV: type" << type << "value" << value;
   return value;
}

// Get a specific panel double value
double US_AnaprofPanPCSA::getDValue( const QString type )
{
   int nchan      = apPCSA->nchan;
   double value   = 0;

   if ( type == "nchan" )
   {
      value          = (double)nchan;
   }

   return value;
}

// Get specific panel list values
QStringList US_AnaprofPanPCSA::getLValue( const QString type )
{
   QStringList value;
DbgLv(1) << "EGCe:getSL: type" << type;

   if ( type == "centerpieces" )
   {  // Return row strings where a centerpiece is selected
   }

   else if ( type == "cpchannels" )
   {  // Return channel strings where a centerpiece is selected
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_AnaprofPanPCSA::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_AnaprofPanPCSA::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_AnaprofPanPCSA::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_AnaprofPanPCSA::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_AnaprofPanPCSA::status()
{
   bool is_done        = ( apPCSA->nchan > 0 );   // Done when not all empty
DbgLv(1) << "EGCe:st: nchan is_done" << apPCSA->nchan << is_done;
   return ( is_done ? 8 : 0 );
}

//========================= End:   PCSA      section =========================

