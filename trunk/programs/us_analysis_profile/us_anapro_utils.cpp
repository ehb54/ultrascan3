//! \file us_anapro_utils.cpp

// This source file is meant to hold those portions of the
// US_AnalysisProfile class and its subclasses that are common
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
#include "us_anapro_parms.h"
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

// Return a string parameter value from a US_AnalysisProfile child panel
QString US_AnalysisProfile::childSValue( const QString child, const QString type )
{
   QString value( "" );
   if      ( child == "general"  ) { value = apanGeneral->getSValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getSValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getSValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getSValue( type ); }
   return value;
}

// Return an integer parameter value from a US_AnalysisProfile child panel
int US_AnalysisProfile::childIValue( const QString child, const QString type )
{
   int value      = 0;
   if      ( child == "general"  ) { value = apanGeneral->getIValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getIValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getIValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getIValue( type ); }
   return value;
}

// Return a double parameter value from a US_AnalysisProfile child panel
double US_AnalysisProfile::childDValue( const QString child, const QString type )
{
   double value   = 0.0;
   if      ( child == "general"  ) { value = apanGeneral->getDValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getDValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getDValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getDValue( type ); }
   return value;
}

// Return a stringlist parameter value from a US_AnalysisProfile child panel
QStringList US_AnalysisProfile::childLValue( const QString child, const QString type )
{
   QStringList value;

   if      ( child == "general"  ) { value = apanGeneral->getLValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getLValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getLValue( type ); }
//   else if ( child == "status"   ) { value = apanStatus ->getLValue( type ); }

   return value;
}

// Slot to handle a new panel selected
void US_AnalysisProfile::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx;
   // Save any changes in the old current panel
   if      ( curr_panx == panx )  return;  // No change in panel

   if      ( curr_panx == 0 ) apanGeneral->savePanel();
   else if ( curr_panx == 1 ) apan2DSA   ->savePanel();
   else if ( curr_panx == 2 ) apanPCSA   ->savePanel();
//   else if ( curr_panx == 3 ) apanStatus ->savePanel();
DbgLv(1) << "newPanel   savePanel done";

   // Initialize the new current panel after possible changes
   if      ( panx == 0 )      apanGeneral->initPanel();
   else if ( panx == 1 )      apan2DSA   ->initPanel();
   else if ( panx == 2 )      apanPCSA   ->initPanel();
//   else if ( panx == 3 )      apanStatus ->initPanel();
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
void US_AnalysisProfile::statUpdate()
{
DbgLv(1) << "statUpd: IN stat" << statflag;
   statflag   = apanGeneral->status()
              + apan2DSA   ->status()
              + apanPCSA   ->status();
DbgLv(1) << "statUpd:  MOD stat" << statflag;
}

// Slot to advance to the next panel
void US_AnalysisProfile::panelUp()
{
   int newndx = tabWidget->currentIndex() + 1;
   int maxndx = tabWidget->count() - 1;
   newndx     = ( newndx > maxndx ) ? 0 : newndx;
DbgLv(1) << "panUp: newndx, maxndx" << newndx << maxndx;
   tabWidget->setCurrentIndex( newndx );
}

// Slot to retreat to the previous panel
void US_AnalysisProfile::panelDown()
{
   int newndx = tabWidget->currentIndex() - 1;
   newndx     = ( newndx < 0 ) ? tabWidget->count() - 1 : newndx;
DbgLv(1) << "panDown: newndx" << newndx;
   tabWidget->setCurrentIndex( newndx );
}

// Open manual help appropriate to the current panel
void US_AnalysisProfile::help( void )
{
   if      ( curr_panx == 0 ) apanGeneral ->help();
   else if ( curr_panx == 1 ) apan2DSA    ->help();
   else if ( curr_panx == 2 ) apanPCSA    ->help();
//   else if ( curr_panx == 3 ) apanStatus  ->help();
}

//Slot to DISABLE tabs and Next/Prev buttons
void US_AnalysisProfile::disable_tabs_buttons( void )
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
void US_AnalysisProfile::enable_tabs_buttons( void )
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
void US_AnalysisProfile::enable_tabs_buttons_readonly( void )
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

int  US_AnalysisProfile::getProfiles( QStringList& par1,
                                   QList< QStringList >& par2 )
{ return apanGeneral->getProfiles( par1, par2 ); }

bool US_AnalysisProfile::updateProfiles( const QStringList par1 )
{ return apanGeneral->updateProfiles( par1 ); }

// Initialize all panels after loading a new protocol
void US_AnalysisProfile::initPanels()
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

DbgLv(1) << "APGe: CALL check_user_level()";
   check_user_level();
DbgLv(1) << "APGe:  RTN check_user_level()";
   
}


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
   QList< int > dhms1;
   QList< int > dhms2;
   QList< int > dhms2a;
   QList< int > dhms3;
   
   // Populate GUI settings from protocol controls
//   nspeed               = ap2DSA ->nstep;
//   curssx               = qMin( (nspeed-1), qMax( 0, cb_prof->currentIndex() ) );
//curssx=0;
DbgLv(1) << "AP2d:   iP: AA";
   double duration      = ap2DSA->ssteps[ curssx ].duration;
   double delay         = ap2DSA->ssteps[ curssx ].delay;
   double delay_stage   = ap2DSA->ssteps[ curssx ].delay_stage;
   double scanintv      = ap2DSA->ssteps[ curssx ].scanintv;
//   double speedmax      = sibDValue( "rotor", "maxrpm" );
DbgLv(1) << "AP2d:   iP: BB";

   US_AnaProfParms::timeToList( duration, dhms1 );
   US_AnaProfParms::timeToList( delay,    dhms2 );
   US_AnaProfParms::timeToList( delay_stage,    dhms2a );
   US_AnaProfParms::timeToList( scanintv, dhms3 );
DbgLv(1) << "AP2d:   iP: CC";
#endif

   bool was_changed     = changed;       // Save changed state
#if 0
   sb_count ->setValue  ( nspeed  );
   ct_speed ->setMaximum( speedmax );    // Set speed max based on rotor max
   ct_speed ->setValue  ( ap2DSA->ssteps[ curssx ].speed );
   ct_accel ->setValue  ( ap2DSA->ssteps[ curssx ].accel );
#endif
   
   //ALEXEY Comment for now -> transform "Speeds" panel: seperate spinBoxes...
   // sb_durat ->setValue  ( dhms1[ 0 ] );
   // tm_durat ->setTime   ( QTime( dhms1[ 1 ], dhms1[ 2 ], dhms1[ 3 ] ) );
   // sb_delay ->setValue  ( dhms2[ 0 ] );
   // tm_delay ->setTime   ( QTime( dhms2[ 1 ], dhms2[ 2 ], dhms2[ 3 ] ) );
   // sb_scnint->setValue  ( dhms3[ 0 ] );
   // tm_scnint->setTime   ( QTime( dhms3[ 1 ], dhms3[ 2 ], dhms3[ 3 ] ) );

#if 0
   sb_durat_dd ->setValue( (int)dhms1[ 0 ] );
   sb_durat_hh ->setValue( (int)dhms1[ 1 ] );
   sb_durat_mm ->setValue( (int)dhms1[ 2 ] );
   sb_durat_ss ->setValue( (int)dhms1[ 3 ] );
   sb_delay_dd ->setValue( (int)dhms2[ 0 ] );
   sb_delay_hh ->setValue( (int)dhms2[ 1 ] );
   sb_delay_mm ->setValue( (int)dhms2[ 2 ] );
   sb_delay_ss ->setValue( (int)dhms2[ 3 ] );
   sb_delay_st_dd ->setValue( (int)dhms2a[ 0 ] );
   sb_delay_st_hh ->setValue( (int)dhms2a[ 1 ] );
   sb_delay_st_mm ->setValue( (int)dhms2a[ 2 ] );
   sb_delay_st_ss ->setValue( (int)dhms2a[ 3 ] );
   sb_scnint_dd ->setValue( (int)dhms3[ 0 ] );
   sb_scnint_hh ->setValue( (int)dhms3[ 1 ] );
   sb_scnint_mm ->setValue( (int)dhms3[ 2 ] );
   sb_scnint_ss ->setValue( (int)dhms3[ 3 ] );

   ck_endoff->setChecked( ap2DSA->spin_down );
   ck_radcal->setChecked( ap2DSA->radial_calib );
#endif
   changed              = was_changed;   // Restore changed state
DbgLv(1) << "AP2d:   iP: EE";

//   QString arotor       = sibSValue( "rotor", "arotor" );
//   le_maxrpm->setText( tr( "Maximum speed for %1 rotor:  %2 rpm" )
//                       .arg( arotor ).arg( speedmax ) );

DbgLv(1) << "AP2d:   iP: FF";
   // Populate internal speed-step control
#if 0
   ssvals.resize( nspeed );
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      ssvals[ ii ][ "speed"    ] = ap2DSA->ssteps[ ii ].speed;
      ssvals[ ii ][ "accel"    ] = ap2DSA->ssteps[ ii ].accel;
      ssvals[ ii ][ "duration" ] = ap2DSA->ssteps[ ii ].duration;
      ssvals[ ii ][ "delay"    ] = ap2DSA->ssteps[ ii ].delay;
      ssvals[ ii ][ "delay_stage"    ] = ap2DSA->ssteps[ ii ].delay_stage;
      ssvals[ ii ][ "scanintv" ] = ap2DSA->ssteps[ ii ].scanintv;
      ssvals[ ii ][ "scanintv_min" ] = ap2DSA->ssteps[ ii ].scanintv_min;
DbgLv(1) << "AP2d:inP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"];

      //profdesc[ curssx ] = speedp_description( curssx );
//      cb_prof->setItemText( ii, speedp_description( ii ) );
   }
#endif
DbgLv(1) << "AP2d:   iP: ZZ";
}

// Save panel controls when about to leave the panel
void US_AnaprofPan2DSA::savePanel()
{
DbgLv(1) << "AP2d:svP: IN";
   // Populate protocol speed controls from internal panel control
#if 0
   nspeed                = ssvals.count();
   ap2DSA->nstep        = nspeed;
   ap2DSA->spin_down    = ck_endoff->isChecked();
   ap2DSA->radial_calib = ck_radcal->isChecked();
DbgLv(1) << "AP2d:svP: nspeed" << nspeed;

   ap2DSA->ssteps.resize( nspeed );  //ALEXEY BUG FIX
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      ap2DSA->ssteps[ ii ].speed    = ssvals[ ii ][ "speed"    ];
      ap2DSA->ssteps[ ii ].accel    = ssvals[ ii ][ "accel"    ];
      ap2DSA->ssteps[ ii ].duration = ssvals[ ii ][ "duration" ];
      ap2DSA->ssteps[ ii ].delay    = ssvals[ ii ][ "delay"    ];
      ap2DSA->ssteps[ ii ].delay_stage    = ssvals[ ii ][ "delay_stage"    ];
      ap2DSA->ssteps[ ii ].scanintv = ssvals[ ii ][ "scanintv" ];
      ap2DSA->ssteps[ ii ].scanintv_min = ssvals[ ii ][ "scanintv_min" ];
 DbgLv(1) << "AP2d:svP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"]
 << ssvals[ii]["delay_stage"] << ssvals[ii]["scanintv"];

 qDebug() << " DURATION SAVED IN  PROTOTCOL: speed " << ii <<  ", duration: " << ap2DSA->ssteps[ ii ].duration;
   }
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
   if ( type == "nspeeds" ) { value = ap2DSA->nstep;  }
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
   if ( type == "profiles" )
   {  // Compose list of all speed-step values (4 lines per step)
      value.clear();

      for ( int ii = 0; ii < ap2DSA->nstep; ii++ )
      {  // Build list of QString forms of speed-step double
         double speed         = ap2DSA->ssteps[ ii ].speed;
         double accel         = ap2DSA->ssteps[ ii ].accel;
         double duration      = ap2DSA->ssteps[ ii ].duration;   // In seconds
         double delay         = ap2DSA->ssteps[ ii ].delay;      // In seconds
         double delay_stage   = ap2DSA->ssteps[ ii ].delay_stage;// In seconds
         double scint         = ap2DSA->ssteps[ ii ].scanintv;

         qDebug() << "ScanInt: " << scint;
         
         // double durathrs      = qFloor( duration / 60.0 );
         // double duratmin      = duration - ( durathrs * 60.0 );
         // double delaymin      = qFloor( delay / 60.0 );
         // double delaysec      = delay - ( delaymin * 60.0 );

         double durathrs      = qFloor( duration / 3600.0 );
         double duratmin      = qFloor(duration - ( durathrs * 3600.0 )) / 60.0;
         
         double delayhrs      = qFloor( delay / 3600.0 );
         double delaymin      = qFloor(delay - ( delayhrs * 3600.0 )) / 60.0;

         double delaystagehrs = qFloor( delay_stage / 3600.0 );
         double delaystagemin = qFloor(delay_stage - ( delaystagehrs * 3600.0 )) / 60.0;
         
         double scinthrs      = qFloor( scint / 3600.0 );
         double scintmin      = qFloor(( scint - ( scinthrs * 3600.0 )) / 60.0);
         double scintsec      = scint - ( scinthrs * 3600.0 ) - ( scintmin * 60.0 );
         
         value << tr( "%1 rpm" ).arg( speed );
         value << tr( "%1 rpm/sec" ).arg( accel );
         value << tr( "%1 h %2 m" )
                  .arg( durathrs ).arg( duratmin );
         value << tr( "%1 h %2 m " )
                  .arg( delayhrs ).arg( delaymin );
         value << tr( "%1 h %2 m " )
                  .arg( delaystagehrs ).arg( delaystagemin );
         value << tr( "%1 h %2 m %3 s" )                                        //ALEXEY: added scan interval
                  .arg( scinthrs ).arg( scintmin ).arg( scintsec );
      }
   }

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
DbgLv(1) << "EGCe:inP: prb: nholes" << apPCSA->ncell << "nused" << apPCSA->nused;
//   rebuild_Cells();

   // Now build cell rows from protocol
   int nholes          = apPCSA->ncell;
   int nused           = apPCSA->nused;
   int kused           = 0;
DbgLv(1) << "EGCe:inP: nholes" << nholes << "nused" << nused;


   // Insure rows beyond nholes are totally invisible

DbgLv(1) << "EGCe:inP: kused" << kused << "nused" << nused;
   nused               = kused;
}

// Save Cells panel controls when about to leave the panel
void US_AnaprofPanPCSA::savePanel()
{
DbgLv(1) << "APPc:svP: IN";
   // Recount cells used and fill vector based on GUI state
   int nholes          = sibIValue( "rotor", "nholes" );
   int nused           = 0;
   QStringList ulabs;
   QString txtempty    = tr( "empty" );

   // Now fill vector of used cell entries
   apPCSA->ncell      = nholes;
   apPCSA->nused      = nused;
   apPCSA->used.resize( nused );

}

// Get a specific panel value
QString US_AnaprofPanPCSA::getSValue( const QString type )
{
DbgLv(1) << "EGCe:getSV: type" << type;
   QString txtempty = tr( "empty" );
   QString value( "" );
   int ncell    = apPCSA->ncell;
   int nused    = apPCSA->nused;

   if ( type == "ncells" )
   {
      value        = QString::number( ncell );
   }
   else if ( type == "nused" )
   {
      value        = QString::number( nused );
   }
   else if ( type == "alldone" )
   {
      value        = ( nused > 0 ) ? "1" : "0";
   }
   else if ( type == "counterbalance" )
   {
      value        = ( nused > 0 ) ? apPCSA->used[ nused - 1 ].cbalance : value;
DbgLv(1) << "EGCe:getSV:   CB nused" << nused << "cbal" << value;
   }
DbgLv(1) << "EGCe:getSV: type" << type << "value" << value;

   return value;
}

// Get a specific panel integer value
int US_AnaprofPanPCSA::getIValue( const QString type )
{
   int value      = 0;

   if      ( type == "ncells" )  { value = apPCSA->ncell; }
   else if ( type == "nused" )   { value = apPCSA->nused; }
   else if ( type == "alldone" ) { value = ( apPCSA->nused > 0 ) ? 1 : 0; }
   else if ( type == "nchans" )
   {
      int nchans     = 0;

      for ( int ii = 0; ii < apPCSA->nused; ii++ )
      {
         QString centp  = apPCSA->used[ ii ].centerpiece;
         int chx        = centp.indexOf( "-channel" );

         if ( chx > 0 )
         {
            int ncchn      = QString( centp ).left( chx ).section( " ", -1, -1 )
                                             .simplified().toInt();
            nchans        += ncchn;
         }
      }

      value          = nchans;
   }

DbgLv(1) << "EGCe:getIV: type" << type << "value" << value;
   return value;
}

// Get a specific panel double value
double US_AnaprofPanPCSA::getDValue( const QString type )
{
   int ncell      = apPCSA->ncell;
   double value   = 0;

   if ( type == "ncells" )
   {
      value          = (double)ncell;
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
   bool is_done        = ( apPCSA->nused > 0 );   // Done when not all empty
DbgLv(1) << "EGCe:st: nused is_done" << apPCSA->nused << is_done;
   return ( is_done ? 8 : 0 );
}

//========================= End:   PCSA      section =========================


#if 0
//========================= Start: Status    section =========================

// Initialize an Status panel, especially after clicking on its tab
void US_AnaprofPanStatus::initPanel()
{
   currProf        = &mainw->currProf;
   loadProf        = &mainw->loadProf;
   rps_differ      = ( mainw->currProf !=  mainw->loadProf );
   ap2DSA          = &currProf->ap2DSA;
   apPCSA          = &currProf->apPCSA;
   apStat          = &currProf->apStat;


   qDebug() << "apSPEED: duration: " << ap2DSA->ssteps[0].duration;
   
if(rps_differ)
{
US_AnaProfParms* cAP = currProf;
US_AnaProfParms* lAP = loadProf;
DbgLv(1) << "APUp:inP: APs DIFFER";
DbgLv(1) << "APUp:inP:  cAname" << cAP->aprofname << "lAname" << lAP->aprofname;
DbgLv(1) << "APUp:inP:  cPname" << cAP->protoname << "lPname" << lAP->protoname;
DbgLv(1) << "APUp:inP:  cAguid" << cAP->aprofGUID << "lAguid" << lAP->aprofGUID;
DbgLv(1) << "APUp:inP:  cPguid" << cAP->protoGUID << "lPguid" << lAP->protoGUID;
DbgLv(1) << "APUp:inP:  cPId  " << cAP->protoID << "lPId" << lAP->protoID;
DbgLv(1) << "APUp:inP:   ap2DSA diff" << (cAP->ap2DSA!=lAP->ap2DSA);
DbgLv(1) << "APUp:inP:   apPCSA diff" << (cAP->apPCSA!=lAP->apPCSA);
DbgLv(1) << "APUp:inP:   apStat diff" << (cAP->apStat!=lAP->apStat);
}

   have_run          = ! sibSValue( "general",   "runID"    ).isEmpty();
   have_proj         = ! sibSValue( "general",   "project"  ).isEmpty();
   //chgd_rotor        = ( sibIValue( "rotor",     "changed"  ) > 0 );
   chgd_speed        = ( sibIValue( "speeds",    "changed"  ) > 0 );
   have_cells        = ( sibIValue( "cells",     "alldone"  ) > 0 );
   have_solus        = ( sibIValue( "solutions", "alldone"  ) > 0 );
   have_optic        = ( sibIValue( "optical",   "alldone"  ) > 0 );
   have_range        = ( sibIValue( "ranges",    "alldone"  ) > 0 );
DbgLv(1) << "APUp:inP: ck: run proj cent solu epro"
 << have_run << have_proj << have_cells << have_solus << have_range;
   //proto_ena         = ( have_cells  &&  have_solus  &&  have_optic  &&
   //                      have_range );
   //subm_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
   //                      connected );

//   ck_run     ->setChecked( have_run   );
//   ck_project ->setChecked( have_proj  );
   //ck_rotor_ok->setChecked( chgd_rotor );
DbgLv(1) << "APUp:inP:  KK";
//   ck_speed_ok->setChecked( chgd_speed );
//   ck_centerp ->setChecked( have_cells );
//   ck_solution->setChecked( have_solus );
//   ck_optical ->setChecked( have_optic );
DbgLv(1) << "APUp:inP:  LL";
//   ck_ranges  ->setChecked( have_range );
//   ck_connect ->setChecked( connected  );
   //ck_prot_ena->setChecked( proto_ena  );
   //ck_prot_svd->setChecked( proto_svd  );
   //ck_sub_enab->setChecked( subm_enab  );
DbgLv(1) << "APUp:inP:  MM";
//   ck_sub_done->setChecked( submitted  );
//   ck_rp_diff ->setChecked( rps_differ );
DbgLv(1) << "APUp:inP:  NN";

//   pb_submit  ->setEnabled( subm_enab  );                                  // <-- Temporary enabled for testing
//   pb_saverp  ->setEnabled( have_cells && have_solus && have_range );
DbgLv(1) << "APUp:inP: OUT";
}

// Save panel controls when about to leave the panel
void US_AnaprofPanStatus::savePanel()
{
DbgLv(1) << "APUp:svP: IN";
}

// Get a specific panel value
QString US_AnaprofPanStatus::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "alldone"  ||
             type == "status"   ||
             type == "len_xml"  )
      value  = QString::number( getIValue( type ) );
   else if ( type == "xml" )  
      value  = apStat->us_xml;

   return value;
}

// Get a specific panel integer value
int US_AnaprofPanStatus::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "alldone" )  { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  )  { value = status(); }
   else if ( type == "len_xml" )  { value = apStat->us_xml.length(); }
   return value;
}

// Get a specific panel double value
double US_AnaprofPanStatus::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_AnaprofPanStatus::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "uploaded" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_AnaprofPanStatus::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_AnaprofPanStatus::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_AnaprofPanStatus::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_AnaprofPanStatus::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status string for the panel
int US_AnaprofPanStatus::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? 128 : 0 );
}

//========================= End:   Status    section =========================
#endif

