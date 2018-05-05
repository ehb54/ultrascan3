//! \file us_exp_utils.cpp

// This source file is meant to hold those portions of the
// US_ExperimentMain class and its subclasses that are common
// utility functions used by the class and its subclasses.
// Thereby, the length of any one source file is reduced and the
// us_experiment_main.cpp file contains only very class-specific source
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

#include "us_experiment_gui.h"
#include "us_run_protocol.h"
#include "us_rotor_gui.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"

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

// Return a string parameter value from a US_ExperimentMain child panel
QString US_ExperimentMain::childSValue( const QString child, const QString type )
{
   QString value( "" );
   if      ( child == "general"  ) { value = epanGeneral  ->getSValue( type ); }
   else if ( child == "rotor"    ) { value = epanRotor    ->getSValue( type ); }
   else if ( child == "speeds"   ) { value = epanSpeeds   ->getSValue( type ); }
   else if ( child == "cells"    ) { value = epanCells    ->getSValue( type ); }
   else if ( child == "solutions") { value = epanSolutions->getSValue( type ); }
   else if ( child == "optical"  ) { value = epanOptical  ->getSValue( type ); }
   else if ( child == "ranges"   ) { value = epanRanges   ->getSValue( type ); }
   else if ( child == "submit"   ) { value = epanUpload   ->getSValue( type ); }
   return value;
}

// Return an integer parameter value from a US_ExperimentMain child panel
int US_ExperimentMain::childIValue( const QString child, const QString type )
{
   int value      = 0;
   if      ( child == "general"  ) { value = epanGeneral  ->getIValue( type ); }
   else if ( child == "rotor"    ) { value = epanRotor    ->getIValue( type ); }
   else if ( child == "speeds"   ) { value = epanSpeeds   ->getIValue( type ); }
   else if ( child == "cells"    ) { value = epanCells    ->getIValue( type ); }
   else if ( child == "solutions") { value = epanSolutions->getIValue( type ); }
   else if ( child == "optical"  ) { value = epanOptical  ->getIValue( type ); }
   else if ( child == "ranges"   ) { value = epanRanges   ->getIValue( type ); }
   else if ( child == "submit"   ) { value = epanUpload   ->getIValue( type ); }
   return value;
}

// Return a double parameter value from a US_ExperimentMain child panel
double US_ExperimentMain::childDValue( const QString child, const QString type )
{
   double value   = 0.0;
   if      ( child == "general"  ) { value = epanGeneral  ->getDValue( type ); }
   else if ( child == "rotor"    ) { value = epanRotor    ->getDValue( type ); }
   else if ( child == "speeds"   ) { value = epanSpeeds   ->getDValue( type ); }
   else if ( child == "cells"    ) { value = epanCells    ->getDValue( type ); }
   else if ( child == "solutions") { value = epanSolutions->getDValue( type ); }
   else if ( child == "optical"  ) { value = epanOptical  ->getDValue( type ); }
   else if ( child == "ranges"   ) { value = epanRanges   ->getDValue( type ); }
   else if ( child == "submit"   ) { value = epanUpload   ->getDValue( type ); }
   return value;
}

// Return a stringlist parameter value from a US_ExperimentMain child panel
QStringList US_ExperimentMain::childLValue( const QString child, const QString type )
{
   QStringList value;

   if      ( child == "general"  ) { value = epanGeneral  ->getLValue( type ); }
   else if ( child == "rotor"    ) { value = epanRotor    ->getLValue( type ); }
   else if ( child == "speeds"   ) { value = epanSpeeds   ->getLValue( type ); }
   else if ( child == "cells"    ) { value = epanCells    ->getLValue( type ); }
   else if ( child == "solutions") { value = epanSolutions->getLValue( type ); }
   else if ( child == "optical"  ) { value = epanOptical  ->getLValue( type ); }
   else if ( child == "ranges"   ) { value = epanRanges   ->getLValue( type ); }
   else if ( child == "submit"   ) { value = epanUpload   ->getLValue( type ); }

   return value;
}

// Slot to handle a new panel selected
void US_ExperimentMain::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx;
   // Save any changes in the old current panel
   if      ( curr_panx == panx )  return;  // No change in panel

   if      ( curr_panx == 0 ) epanGeneral  ->savePanel();
   else if ( curr_panx == 1 ) epanRotor    ->savePanel();
   else if ( curr_panx == 2 ) epanSpeeds   ->savePanel();
   else if ( curr_panx == 3 ) epanCells    ->savePanel();
   else if ( curr_panx == 4 ) epanSolutions->savePanel();
   else if ( curr_panx == 5 ) epanOptical  ->savePanel();
   else if ( curr_panx == 6 ) epanRanges   ->savePanel();
   else if ( curr_panx == 7 ) epanRotor    ->savePanel();

   curr_panx              = panx;         // Set new current panel

   // Initialize the new current panel after possible changes
   if      ( panx == 0 )      epanGeneral  ->initPanel();
   else if ( panx == 1 )      epanRotor    ->initPanel();
   else if ( panx == 2 )      epanSpeeds   ->initPanel();
   else if ( panx == 3 )      epanCells    ->initPanel();
   else if ( panx == 4 )      epanSolutions->initPanel();
   else if ( panx == 5 )      epanOptical  ->initPanel();
   else if ( panx == 6 )      epanRanges   ->initPanel();
   else if ( panx == 7 )      epanUpload   ->initPanel();

   // Update status flag for all panels
   statUpdate();
}

// Slot to update status flag for all panels
void US_ExperimentMain::statUpdate()
{
DbgLv(1) << "statUpd: IN stat" << statflag;
   statflag   = epanGeneral  ->status()
              + epanRotor    ->status()
              + epanSpeeds   ->status()
              + epanCells    ->status()
              + epanSolutions->status()
              + epanOptical  ->status()
              + epanRanges   ->status()
              + epanUpload   ->status();
DbgLv(1) << "statUpd:  MOD stat" << statflag;
}

// Slot to advance to the next panel
void US_ExperimentMain::panelUp()
{
   int newndx = tabWidget->currentIndex() + 1;
   int maxndx = tabWidget->count() - 1;
DbgLv(1) << "panUp: newndx, maxndx" << newndx << maxndx;
   tabWidget->setCurrentIndex( qMin( newndx, maxndx ) );
}

// Slot to retreat to the previous panel
void US_ExperimentMain::panelDown()
{
   int newndx = tabWidget->currentIndex() - 1;
   tabWidget->setCurrentIndex( qMax( newndx, 0 ) );
}

// Open manual help appropriate to the current panel
void US_ExperimentMain::help( void )
{
   if      ( curr_panx == 0 ) epanGeneral  ->help();
   else if ( curr_panx == 1 ) epanRotor    ->help();
   else if ( curr_panx == 2 ) epanSpeeds   ->help();
   else if ( curr_panx == 3 ) epanCells    ->help();
   else if ( curr_panx == 4 ) epanSolutions->help();
   else if ( curr_panx == 5 ) epanOptical  ->help();
   else if ( curr_panx == 6 ) epanRanges   ->help();
   else if ( curr_panx == 7 ) epanUpload   ->help();
}

//Slot to DISABLE tabs and Next/Prev buttons
void US_ExperimentMain::unable_tabs_buttons( void )
{
  DbgLv(1) << "UNABLING!!!";
  pb_next   ->setEnabled(false);
  pb_prev   ->setEnabled(false);
  
  for (int i=1; i<tabWidget->count(); i++)
    {
      tabWidget ->setTabEnabled( i, false );
      tabWidget ->tabBar()->setTabTextColor( i, Qt::darkGray);
     
    }
  
}

//Slot to ENABLE tabs and Next/Prev buttons, but make all Widgets read-only
void US_ExperimentMain::enable_tabs_buttons( void )
{
  DbgLv(1) << "ENABLING!!!";
  pb_next   ->setEnabled(true);
  pb_prev   ->setEnabled(true);
  
 
  for (int i=1; i<tabWidget->count(); i++)
    {
      tabWidget ->setTabEnabled( i, true );
      QPalette pal = tabWidget ->tabBar()->palette();
      //DbgLv(1) << "PALETTE: " << pal.color(QPalette::WindowText);
      tabWidget ->tabBar()->setTabTextColor( i, pal.color(QPalette::WindowText) ); // Qt::black
      
      QWidget* pWidget= tabWidget->widget(i);
      
      //Find all children of each Tab in QTabWidget [children of all types...]
      QList<QPushButton *> allPButtons = pWidget->findChildren<QPushButton *>();
      QList<QComboBox *>   allCBoxes   = pWidget->findChildren<QComboBox *>();
      QList<QSpinBox *>    allSBoxes   = pWidget->findChildren<QSpinBox *>();
      QList<QwtCounter *>  allCounters = pWidget->findChildren<QwtCounter *>();
      QList<QCheckBox *>   allChBoxes  = pWidget->findChildren<QCheckBox *>();

      // and so on ..
      
      for (int i=0; i < allPButtons.count(); i++)
	{
	  if ( (allPButtons[i]->text()).contains("View Solution Details") || 
	       (allPButtons[i]->text()).contains("View Current Range Settings") ||
	       (allPButtons[i]->text()).contains("View Experiment Details") ||
	       (allPButtons[i]->text()).contains("Test Connection") )
	    allPButtons[i]->setEnabled(true);
	  else
	    allPButtons[i]->setEnabled(false);
	}
      
      for (int i=0; i < allCBoxes.count(); i++)
	{
	  if ( (allCBoxes[i]->currentText()).contains("Speed Profile") )
	    allCBoxes[i]->setEnabled(true);
	  else
	    allCBoxes[i]->setEnabled(false);
	}
      for (int i=0; i < allSBoxes.count(); i++)
	allSBoxes[i]->setEnabled(false);
      for (int i=0; i < allCounters.count(); i++)
	allCounters[i]->setEnabled(false); 
      for (int i=0; i < allChBoxes.count(); i++)
	allChBoxes[i]->setEnabled(false);
      // and so on ..
      
    }
  
}


// Use main interface to call general utility functions
bool US_ExperimentMain::centpInfo( const QString par1,
                                   US_AbstractCenterpiece& par2 )
{ return epanGeneral->centpInfo( par1, par2 ); }

int  US_ExperimentMain::getProtos( QStringList& par1,
                                   QList< QStringList >& par2 )
{ return epanGeneral->getProtos( par1, par2 ); }

bool US_ExperimentMain::updateProtos( const QStringList par1 )
{ return epanGeneral->updateProtos( par1 ); }

// Initialize all panels after loading a new protocol
void US_ExperimentMain::initPanels()
{
   epanGeneral  ->initPanel();
   epanRotor    ->initPanel();
   epanSpeeds   ->initPanel();
   epanCells    ->initPanel();
   epanSolutions->initPanel();
   epanOptical  ->initPanel();
   epanRanges   ->initPanel();
   epanUpload   ->initPanel();
}

//========================= End:   Main      section =========================


//========================= Start: General   section =========================

// Initialize a General panel, especially after clicking on its tab
void US_ExperGuiGeneral::initPanel()
{
//*Test*
QList<QStringList> lsl1;
QList<QStringList> lsl2;
QString llstring;
QString llstring2;
QStringList ii1;
QStringList ii2;
QStringList ii3;

ii1 << "1AA" << "1BB" << "1CC";
ii2 << "2AA" << "2BB";
ii3 << "3AA" << "3BB" << "3CC" << "3DD";
lsl1 << ii1 << ii2 << ii3;
US_Util::listlistBuild( lsl1, llstring );
US_Util::listlistParse( lsl2, llstring );
for ( int ii=0;ii<lsl1.count();ii++ )
{ qDebug() << "TEST: lsl1" << ii << lsl1[ii]; }
qDebug() << "TEST: llstring" << llstring;
for ( int ii=0;ii<lsl2.count();ii++ )
{ qDebug() << "TEST: lsl2" << ii << lsl2[ii]; }
US_Util::listlistBuild( lsl1, llstring2 );
qDebug() << "TEST: llstring2" << llstring2;
//*Test*
#if 0
use_db=false;
#endif
DbgLv(1) << "EGGe: inP: prn,prd counts" << protdata.count() << pr_names.count();

   // Populate GUI settings from protocol controls
   le_investigator->setText ( currProto->investigator );
   le_runid       ->setText ( currProto->runname );
   le_protocol    ->setText ( currProto->protname );
   le_project     ->setText ( currProto->project );
   ct_tempera     ->setValue( currProto->temperature );
   ct_tedelay     ->setValue( currProto->temeq_delay );
   
   check_user_level();

}

//IF USER cannot edit anything (low-level user)
void US_ExperGuiGeneral::check_user_level()
{
  if ( US_Settings::us_inv_level() < 3 )
    {
      pb_investigator->setEnabled( false );
      pb_project     ->setEnabled( false );
   
      if ( !loaded_proto )
	emit set_tabs_buttons_inactive();
      else
	emit set_tabs_buttons_active();
   
      DbgLv(1) << "SIGNAL!!!!" ; 
    }
}

// Save panel controls when about to leave the panel
void US_ExperGuiGeneral::savePanel()
{
   // Populate protocol controls from GUI settings
   currProto->investigator = le_investigator->text();
   currProto->runname      = le_runid       ->text();
   currProto->protname     = le_protocol    ->text();
   currProto->project      = le_project     ->text();
   currProto->temperature  = ct_tempera     ->value();
   currProto->temeq_delay  = ct_tedelay     ->value();
}

// Get a specific panel string value
QString US_ExperGuiGeneral::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "runID"  ||
             type == "runname" )      { value = currProto->runname; }
   else if ( type == "protocol" )     { value = currProto->protname; }
   else if ( type == "project" )      { value = currProto->project; }
   else if ( type == "investigator" ) { value = currProto->investigator; }
   else if ( type == "dbdisk" )       { value = use_db ? "DB" : "Disk"; }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiGeneral::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "dbdisk" )       { value = use_db ? 1 : 0; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiGeneral::getDValue( const QString type )
{
   double value   = 0.0;
   if      ( type == "temperature" ) { value = currProto->temperature; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiGeneral::getLValue( const QString type )
{
   QStringList value( "" );

   if      ( type == "centerpieces" )   { value = cp_names; }
   else if ( type == "protocol_names" ) { value = pr_names; }

   return value;
}

// Get a specific panel string value from a sibling panel
QString US_ExperGuiGeneral::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiGeneral::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiGeneral::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiGeneral::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiGeneral::status()
{
   bool is_done  = !( currProto->runname.isEmpty()  ||
                      currProto->project.isEmpty() );
   return ( is_done ? 1 : 0 );
}

//========================= End:   General   section =========================

 
//========================= Start: Rotor     section =========================
 
// Initialize a Rotor panel, especially after clicking on its tab
void US_ExperGuiRotor::initPanel()
{
   rpRotor             = &(mainw->currProto.rpRotor);

   // Populate GUI settings from protocol controls
   bool was_changed     = changed;       // Save changed state
DbgLv(1) << "EGRo: inP: was_changed" << was_changed;
   setCbCurrentText( cb_lab,    QString::number( rpRotor->labID ) + ": "
                                + rpRotor->laboratory );
   setCbCurrentText( cb_rotor,  QString::number( rpRotor->rotID ) + ": "
                                + rpRotor->rotor );
   setCbCurrentText( cb_calibr, QString::number( rpRotor->calID ) + ": "
                                + rpRotor->calibration );
   changed              = was_changed;   // Restore changed state
DbgLv(1) << "EGRo: inP:  rotID" << rpRotor->rotID << "rotor" << rpRotor->rotor
 << "cb_rotor text" << cb_rotor->currentText();
}

// Save panel controls when about to leave the panel
void US_ExperGuiRotor::savePanel()
{
   // Populate protocol controls from GUI settings
   QString lab          = cb_lab   ->currentText();
   QString rot          = cb_rotor ->currentText();
   QString cal          = cb_calibr->currentText();
   rpRotor->laboratory  = QString( lab ).section( ":", 1, 1 ).simplified();
   rpRotor->rotor       = QString( rot ).section( ":", 1, 1 ).simplified();
   rpRotor->calibration = QString( cal ).section( ":", 1, 1 ).simplified();
   rpRotor->labID       = QString( lab ).section( ":", 0, 0 ).toInt();
   rpRotor->rotID       = QString( rot ).section( ":", 0, 0 ).toInt();
   rpRotor->calID       = QString( cal ).section( ":", 0, 0 ).toInt();
DbgLv(1) << "EGRo:  svP:  rotID" << rpRotor->rotID << "rotor" << rpRotor->rotor
 << "cb_rotor text" << rot;

   // Set the GUIDs by matching up with IDs
   for ( int ii = 0; ii < labs.count(); ii++ )
   {
      if ( rpRotor->labID == labs[ ii ].ID )
         rpRotor->labGUID  = labs[ ii ].GUID;
   }
   for ( int ii = 0; ii < rotors.count(); ii++ )
   {
      if ( rpRotor->rotID == rotors[ ii ].ID )
      {
         rpRotor->rotGUID  = rotors[ ii ].GUID;
         rpRotor->absID    = rotors[ ii ].abstractRotorID;
      }
   }
   for ( int ii = 0; ii < arotors.count(); ii++ )
   {
      if ( rpRotor->absID == arotors[ ii ].ID )
         rpRotor->absGUID  = arotors[ ii ].GUID;
   }
   for ( int ii = 0; ii < calibs.count(); ii++ )
   {
      if ( rpRotor->calID == calibs[ ii ].ID )
         rpRotor->calGUID  = calibs[ ii ].GUID;
   }
}

// Get a specific panel string value
QString US_ExperGuiRotor::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "lab" )     { value = rpRotor->laboratory; }
   else if ( type == "rotor" )   { value = rpRotor->rotor; }
   else if ( type == "calib" )   { value = rpRotor->calibration; }
   else if ( type == "changed" ) { value = changed ? "1" : "0"; }

   else if ( type == "arotor" )
   {
      US_Rotor::AbstractRotor* arotor = abstractRotor( rpRotor->rotor );
      value          = ( arotor != NULL ) ? arotor->name : "";
   }

   else if ( type == "nholes" )
   {
      value          = QString::number( getIValue( "nholes" ) );
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiRotor::getIValue( const QString type )
{
   int value   = 0;
DbgLv(1) << "EGRo:gIV: type" << type;
   if ( type == "nholes" )
   {
      US_Rotor::AbstractRotor* arotor = abstractRotor( rpRotor->rotor );
      value          = ( arotor != NULL ) ? arotor->numHoles : 0;
DbgLv(1) << "EGRo:gIV:  arotor" << arotor->name << "value" << value;
   }
   else if ( type == "maxrpm" )
   {
      US_Rotor::AbstractRotor* arotor = abstractRotor( rpRotor->rotor );
DbgLv(1) << "EGRo:gIV:  arotor" << arotor << "rotor" << rpRotor->rotor;
      value          = ( arotor != NULL ) ? arotor->maxRPM : 50000;
DbgLv(1) << "EGRo:gIV:  arotor" << arotor->name << "value" << value;
   }
   else if ( type == "labID" )   { value = rpRotor->labID; }
   else if ( type == "rotID" )   { value = rpRotor->rotID; }
   else if ( type == "calID" )   { value = rpRotor->calID; }
   else if ( type == "changed" ) { value = changed ? 1 : 0; }

   return value;
}

// Get a specific panel double value
double US_ExperGuiRotor::getDValue( const QString type )
{
   double value   = 0;
   if      ( type == "maxrpm" ) { value = (double) getIValue( type ); }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiRotor::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "all" )
   {
      value.clear();
      value << getSValue( "lab" );
      value << getSValue( "rotor" );
      value << getSValue( "calib" );
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiRotor::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiRotor::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiRotor::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiRotor::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiRotor::status()
{
   bool is_done = !( rpRotor->laboratory .isEmpty()  ||
                     rpRotor->rotor      .isEmpty()  ||
                     rpRotor->calibration.isEmpty() );
   return ( is_done ? 2 : 0 );
}

//========================= End:   Rotor     section =========================


//========================= Start: Speeds    section =========================

// Initialize a Speeds panel, especially after clicking on its tab
void US_ExperGuiSpeeds::initPanel()
{
   rpSpeed             = &(mainw->currProto.rpSpeed);

   QList< int > dhms1;
   QList< int > dhms2;
   QList< int > dhms3;
   
   // Populate GUI settings from protocol controls
   nspeed               = rpSpeed ->nstep;
   curssx               = qMin( (nspeed-1), qMax( 0, cb_prof->currentIndex() ) );
   double duration      = rpSpeed->ssteps[ curssx ].duration;
   double delay         = rpSpeed->ssteps[ curssx ].delay;
   double scanintv      = rpSpeed->ssteps[ curssx ].scanintv;
   double speedmax      = sibDValue( "rotor", "maxrpm" );

   US_RunProtocol::timeToList( duration, dhms1 );
   US_RunProtocol::timeToList( delay,    dhms2 );
   US_RunProtocol::timeToList( scanintv, dhms3 );

   bool was_changed     = changed;       // Save changed state
   sb_count ->setValue  ( nspeed  );
   ct_speed ->setMaximum( speedmax );    // Set speed max based on rotor max
   ct_speed ->setValue  ( rpSpeed->ssteps[ curssx ].speed );
   ct_accel ->setValue  ( rpSpeed->ssteps[ curssx ].accel );
   
   //ALEXEY Comment for now -> transform "Speeds" panel: seperate spinBoxes...
   // sb_durat ->setValue  ( dhms1[ 0 ] );
   // tm_durat ->setTime   ( QTime( dhms1[ 1 ], dhms1[ 2 ], dhms1[ 3 ] ) );
   // sb_delay ->setValue  ( dhms2[ 0 ] );
   // tm_delay ->setTime   ( QTime( dhms2[ 1 ], dhms2[ 2 ], dhms2[ 3 ] ) );
   // sb_scnint->setValue  ( dhms3[ 0 ] );
   // tm_scnint->setTime   ( QTime( dhms3[ 1 ], dhms3[ 2 ], dhms3[ 3 ] ) );

   sb_durat_dd ->setValue( (int)dhms1[ 0 ] );
   sb_durat_hh ->setValue( (int)dhms1[ 1 ] );
   sb_durat_mm ->setValue( (int)dhms1[ 2 ] );
   sb_durat_ss ->setValue( (int)dhms1[ 3 ] );
   sb_delay_dd ->setValue( (int)dhms2[ 0 ] );
   sb_delay_hh ->setValue( (int)dhms2[ 1 ] );
   sb_delay_mm ->setValue( (int)dhms2[ 2 ] );
   sb_delay_ss ->setValue( (int)dhms2[ 3 ] );
   sb_scnint_dd ->setValue( (int)dhms3[ 0 ] );
   sb_scnint_hh ->setValue( (int)dhms3[ 1 ] );
   sb_scnint_mm ->setValue( (int)dhms3[ 2 ] );
   sb_scnint_ss ->setValue( (int)dhms3[ 3 ] );

   ck_endoff->setChecked( rpSpeed->spin_down );
   ck_radcal->setChecked( rpSpeed->radial_calib );
   changed              = was_changed;   // Restore changed state

   QString arotor       = sibSValue( "rotor", "arotor" );
   le_maxrpm->setText( tr( "Maximum speed for %1 rotor:  %2 rpm" )
                       .arg( arotor ).arg( speedmax ) );

DbgLv(1) << "EGSp:inP: nspeed" << nspeed;
   // Populate internal speed-step control
   ssvals.resize( nspeed );
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      ssvals[ ii ][ "speed"    ] = rpSpeed->ssteps[ ii ].speed;
      ssvals[ ii ][ "accel"    ] = rpSpeed->ssteps[ ii ].accel;
      ssvals[ ii ][ "duration" ] = rpSpeed->ssteps[ ii ].duration;
      ssvals[ ii ][ "delay"    ] = rpSpeed->ssteps[ ii ].delay;
      ssvals[ ii ][ "scanintv" ] = rpSpeed->ssteps[ ii ].scanintv;
DbgLv(1) << "EGSp:inP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"];

      //profdesc[ curssx ] = speedp_description( curssx );
      cb_prof->setItemText( ii, speedp_description( ii ) );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiSpeeds::savePanel()
{
   // Populate protocol speed controls from internal panel control
   nspeed                = ssvals.count();
   rpSpeed->nstep        = nspeed;
   rpSpeed->spin_down    = ck_endoff->isChecked();
   rpSpeed->radial_calib = ck_radcal->isChecked();
DbgLv(1) << "EGSp:svP: nspeed" << nspeed;

   rpSpeed->ssteps.resize( nspeed );  //ALEXEY BUG FIX
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      rpSpeed->ssteps[ ii ].speed    = ssvals[ ii ][ "speed"    ];
      rpSpeed->ssteps[ ii ].accel    = ssvals[ ii ][ "accel"    ];
      rpSpeed->ssteps[ ii ].duration = ssvals[ ii ][ "duration" ];
      rpSpeed->ssteps[ ii ].delay    = ssvals[ ii ][ "delay"    ];
      rpSpeed->ssteps[ ii ].scanintv = ssvals[ ii ][ "scanintv" ];
DbgLv(1) << "EGSp:svP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"]
 << ssvals[ii]["scanintv"];
   }
}

// Get a specific panel value
QString US_ExperGuiSpeeds::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "nspeeds" )
   {
      value = QString::number( ssvals.count() );
   }
   else if ( type == "speed" )
   {
      value = QString::number( ssvals[ 0 ][ "speed" ] );
   }
   else if ( type == "accel" )
   {
      value = QString::number( ssvals[ 0 ][ "accel" ] );
   }
   else if ( type == "duration" )
   {
      US_RunProtocol::timeToString( ssvals[ 0 ][ "duration" ], value );
   }
   else if ( type == "delay" )
   {
      US_RunProtocol::timeToString( ssvals[ 0 ][ "delay" ], value );
   }
   else if ( type == "scanintv" )
   {
      US_RunProtocol::timeToString( ssvals[ 0 ][ "scanintv" ], value );
   }
   else if ( type == "changed" )
   {
      value       = changed ? "1" : "0";
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiSpeeds::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "nspeeds" ) { value = rpSpeed->nstep;  }
   if ( type == "changed" ) { value = changed ? 1 : 0; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiSpeeds::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSpeeds::getLValue( const QString type )
{
   QStringList value( "" );

DbgLv(1) << "EGSp:getLV: type" << type;
   if ( type == "profiles" )
   {  // Compose list of all speed-step values (4 lines per step)
      value.clear();

      for ( int ii = 0; ii < rpSpeed->nstep; ii++ )
      {  // Build list of QString forms of speed-step double
         double speed         = rpSpeed->ssteps[ ii ].speed;
         double accel         = rpSpeed->ssteps[ ii ].accel;
         double duration      = rpSpeed->ssteps[ ii ].duration;   // In seconds
         double delay         = rpSpeed->ssteps[ ii ].delay;      // In seconds

	 double scint         = rpSpeed->ssteps[ ii ].scanintv;

	 qDebug() << "ScanInt: " << scint;
	 
         // double durathrs      = qFloor( duration / 60.0 );
         // double duratmin      = duration - ( durathrs * 60.0 );
         // double delaymin      = qFloor( delay / 60.0 );
         // double delaysec      = delay - ( delaymin * 60.0 );

	 double durathrs      = qFloor( duration / 3600.0 );
         double duratmin      = qFloor(duration - ( durathrs * 3600.0 )) / 60.0;
	 
	 double delayhrs      = qFloor( delay / 3600.0 );
	 double delaymin      = qFloor(delay - ( delayhrs * 3600.0 )) / 60.0;
         
	 double scinthrs      = qFloor( scint / 3600.0 );
	 double scintmin      = qFloor(( scint - ( scinthrs * 3600.0 )) / 60.0);
         double scintsec      = scint - ( scinthrs * 3600.0 ) - ( scintmin * 60.0 );
	 
         value << tr( "%1 rpm" ).arg( speed );
         value << tr( "%1 rpm/sec" ).arg( accel );
         value << tr( "%1 h %2 m" )
                  .arg( durathrs ).arg( duratmin );
         value << tr( "%1 h %2 m " )
                  .arg( delayhrs ).arg( delaymin );
	 value << tr( "%1 h %2 m %3 s" )                                        //ALEXEY: added scan interval
	          .arg( scinthrs ).arg( scintmin ).arg( scintsec );
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSpeeds::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiSpeeds::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiSpeeds::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSpeeds::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiSpeeds::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=true;
   return ( is_done ? 4 : 0 );
}

//========================= End:   Speeds    section =========================


//========================= Start: Cells     section =========================

// Initialize a Cells panel, especially after clicking on its tab
void US_ExperGuiCells::initPanel()
{
   rpCells             = &(mainw->currProto.rpCells);

   const int mxcels    = 8;
   QStringList sl_bals;                         // Counterbalance choices
   sl_bals << tr( "empty (counterbalance)" )
           << tr( "Beckman counterbalance" )
           << tr( "Titanium counterbalance" )
           << tr( "Fluorescence 5-channel counterbalance" );

   // Possibly rebuild Cells protocol if there was a rotor change
DbgLv(1) << "EGCe:inP: prb: nholes" << rpCells->ncell << "nused" << rpCells->nused;
   rebuild_Cells();

   // Now build cell rows from protocol
   int nholes          = rpCells->ncell;
   int nused           = rpCells->nused;
   int icbal           = nholes - 1;            // Counterbalance index
   int kused           = 0;
DbgLv(1) << "EGCe:inP: nholes" << nholes << "nused" << nused;

   for ( int ii = 0; ii < nholes; ii++ )
   {  // 
      int cell            = ii + 1;             // Cell number
      bool is_used        = false;              // By default unused

      if ( ii != icbal   &&
           cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) )
      {  // Centerpiece when list is counterbalances: reset list
         cc_cenps[ ii ]->clear();
         cc_cenps[ ii ]->addItems( cpnames );   // Choose from centerpieces
      }

      else if ( ii == icbal   &&
         ! cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) )
      {  // Counterbalance when list is centerpieces: reset list
         cc_cenps[ ii ]->clear();
         cc_cenps[ ii ]->addItems( sl_bals );   // Choose from counterbalances
      }

      for ( int jj = 0; jj < nused; jj++ )
      {  // Search to see if current is among the used cells
DbgLv(1) << "EGCe:inP:     ii" << ii << "jj" << jj << "cell cellj"
 << cell << rpCells->used[jj].cell;
         if ( cell == rpCells->used[ jj ].cell )
         {  // This is a used cell:  populate a table row
            QString cenbal      =  ( ii != icbal )
                                   ? rpCells->used[ jj ].centerpiece
                                   : rpCells->used[ jj ].cbalance;
            setCbCurrentText( cc_cenps[ ii ], cenbal );
            setCbCurrentText( cc_winds[ ii ], rpCells->used[ jj ].windows );
            is_used             = true;
            kused++;
DbgLv(1) << "EGCe:inP:       kused" << kused;
            break;
         }  // END: is used
      }  // END: used search loop

      // Set visibility of Windows column based on is/not counterbalance
      cc_labls[ ii ]->setVisible( true );
      cc_cenps[ ii ]->setVisible( true );
      cc_winds[ ii ]->setVisible( ii != icbal );

      // Select "empty" for not-used cells
      if ( ! is_used )
      {
         cc_cenps[ ii ]->setCurrentIndex( 0 );
         cc_winds[ ii ]->setCurrentIndex( 0 );
      }
DbgLv(1) << "EGCe:inP:   ii" << ii << "kused" << kused << "is_used" << is_used;
   }  // END: all cells in rotor loop

   // Insure rows beyond nholes are totally invisible
   for ( int ii = nholes; ii < mxcels; ii++ )
   {
      cc_labls[ ii ]->setVisible( false );
      cc_cenps[ ii ]->setVisible( false );
      cc_winds[ ii ]->setVisible( false );
   }

DbgLv(1) << "EGCe:inP: kused" << kused << "nused" << nused;
   nused               = kused;
}

// Save Cells panel controls when about to leave the panel
void US_ExperGuiCells::savePanel()
{
   // Recount cells used and fill vector based on GUI state
   int nholes          = sibIValue( "rotor", "nholes" );
   int nused           = 0;
   QStringList ulabs;
   QString txtempty    = tr( "empty" );

   for ( int ii = 0; ii < nholes; ii++ )
   {  // Count used (non-empty) cell rows
      QString celnam      = cc_labls[ ii ]->text();
      QString centp       = cc_cenps[ ii ]->currentText();
DbgLv(1) << "EGCe:svP:  ii" << ii << "celnam" << celnam << "centp" << centp;
      if ( ! centp.contains( txtempty ) )
      {
         nused++;
         ulabs << celnam;
DbgLv(1) << "EGCe:svP:     nused" << nused << "ulabs" << ulabs;
      }
else
DbgLv(1) << "EGCe:svP:        *UNused*";
   }
DbgLv(1) << "EGCe:svP: nholes nused" << nholes << nused << "ulabs" << ulabs;

   // Now fill vector of used cell entries
   int icbal           = nholes - 1;
   rpCells->ncell      = nholes;
   rpCells->nused      = nused;
   rpCells->used.resize( nused );

   for ( int ii = 0; ii < nholes; ii++ )
   {
      QString celnam      = cc_labls[ ii ]->text();
      int jj              = ulabs.indexOf( celnam );
DbgLv(1) << "EGCe:svP:  ii" << ii << "celnam" << celnam << "jj" << jj;

      if ( jj >= 0 )
      {
         rpCells->used[ jj ].cell = ii + 1;
         QString centp       = cc_cenps[ ii ]->currentText();
DbgLv(1) << "EGCe:svP:    centp" << centp;
         if ( ii != icbal )
         {
            rpCells->used[ jj ].centerpiece = centp;
            rpCells->used[ jj ].cbalance    = "(centerpiece)";
            rpCells->used[ jj ].windows     = cc_winds[ ii ]->currentText();
         }
         else
         {
            rpCells->used[ jj ].cbalance    = centp;
            rpCells->used[ jj ].centerpiece = "(counterbalance)";
            rpCells->used[ jj ].windows     = "";
         }
DbgLv(1) << "EGCe:svP:     cp cb wi" << rpCells->used[jj].centerpiece
 << rpCells->used[jj].cbalance << rpCells->used[jj].windows;
      }
   }
}

// Get a specific panel value
QString US_ExperGuiCells::getSValue( const QString type )
{
DbgLv(1) << "EGCe:getSV: type" << type;
   QString txtempty = tr( "empty" );
   QString value( "" );
   int ncell    = rpCells->ncell;
   int nused    = rpCells->nused;

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
      value        = ( nused > 0 ) ? rpCells->used[ nused - 1 ].cbalance : value;
DbgLv(1) << "EGCe:getSV:   CB nused" << nused << "cbal" << value;
   }
DbgLv(1) << "EGCe:getSV: type" << type << "value" << value;

   return value;
}

// Get a specific panel integer value
int US_ExperGuiCells::getIValue( const QString type )
{
   int value      = 0;

   if      ( type == "ncells" )  { value = rpCells->ncell; }
   else if ( type == "nused" )   { value = rpCells->nused; }
   else if ( type == "alldone" ) { value = ( rpCells->nused > 0 ) ? 1 : 0; }
   else if ( type == "nchans" )
   {
      int nchans     = 0;

      for ( int ii = 0; ii < rpCells->nused; ii++ )
      {
         QString centp  = rpCells->used[ ii ].centerpiece;
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
double US_ExperGuiCells::getDValue( const QString type )
{
   int ncell      = rpCells->ncell;
   double value   = 0;

   if ( type == "ncells" )
   {
      value          = (double)ncell;
   }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiCells::getLValue( const QString type )
{
   QStringList value;
DbgLv(1) << "EGCe:getSL: type" << type;

   if ( type == "centerpieces" )
   {  // Return row strings where a centerpiece is selected
      int nholes          = sibIValue( "rotor", "nholes" );
DbgLv(1) << "EGCe:getSL:  cc_cenps size" << cc_cenps.count();
      for ( int ii = 0; ii < rpCells->nused; ii++ )
      {
QString scel        = tr( "cell %1" ).arg( ii + 1 );
         QString celnm    = tr( "cell %1" ).arg( rpCells->used[ ii ].cell );
         QString centp    = rpCells->used[ ii ].centerpiece;
         QString windo    = rpCells->used[ ii ].windows;
         QString cobal    = rpCells->used[ ii ].cbalance;
         QString centry   = ( ( ii + 1 ) != nholes )
                          ? celnm + " : " + centp + "  ( " + windo + " )"
                          : celnm + " : " + cobal;
DbgLv(1) << "EGCe:getSL:     ii" << ii << " Entry" << centry;
         value << centry;
      }
   }

   else if ( type == "cpchannels" )
   {  // Return channel strings where a centerpiece is selected
      int icbal           = sibIValue( "rotor", "nholes" ) - 1;
      QString rchans( "ABCDEF" );

      for ( int ii = 0; ii < rpCells->nused; ii++ )
      {  // Examine the used cells
         int icell        = rpCells->used[ ii ].cell;
 DbgLv(1) << "USED CELL: " << icell;	 

 //if ( icell >= icbal )   continue;                   // Skip counterbal.      //ALEXEY bug
	 if ( icell > icbal )   continue;                   // Skip counterbal.
         QString channel  = QString( "%1 / " ).arg( icell ); // Start channel
         QString centp    = rpCells->used[ ii ].centerpiece; // Centerpiece
         int chx          = centp.indexOf( "-channel" );     // Index chan count

         if ( chx > 0 )
         {  // Complete string for each channel in centerpiece
            int ncchn      = QString( centp ).left( chx ).section( " ", -1, -1 )
                                             .simplified().toInt();
            for ( int jj = 0; jj < ncchn; jj++ )
            {  // Save channel strings (e.g., "1 / A", "1 / B", "2 / A", ...)
               //value << channel + rchans.mid( jj, 1 );
	      if ( (rchans.mid( jj, 1 )).contains( "A" ) )                   //ALEXEY: channel lables
		value << channel + rchans.mid( jj, 1 ) + ", sample [right]";
	      else if ( (rchans.mid( jj, 1 )).contains( "B" ) )
		value << channel + rchans.mid( jj, 1 ) + ", reference [left]";
	      else
		value << channel + rchans.mid( jj, 1 );
	      
            }
         }
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiCells::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiCells::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiCells::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiCells::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiCells::status()
{
   bool is_done        = ( rpCells->nused > 0 );   // Done when not all empty
DbgLv(1) << "EGCe:st: nused is_done" << rpCells->nused << is_done;
   return ( is_done ? 8 : 0 );
}

//========================= End:   Cells     section =========================


//========================= Start: Solutions section =========================
 
// Initialize a Solutions panel, especially after clicking on its tab
void US_ExperGuiSolutions::initPanel()
{
   rpSolut             = &(mainw->currProto.rpSolut);

DbgLv(1) << "EGSo:inP: call rbS";
   rebuild_Solut();
DbgLv(1) << "EGSo:inP:  aft rbS nchant" << nchant << "nchanf" << nchanf;

   QString unspec      = tr( "(unspecified)" );
   QString lab_none( "none" );

   // Reset GUI elements from protocol and internal panel parameters
   for ( int ii = 0; ii < nchant; ii++ )
   {
      QString channel     = srchans[ ii ];
      QString solution    = unspec;

      int srx             = suchans.indexOf( channel );
      if ( srx >= 0 )
      {
        solution             = rpSolut->chsols[ srx ].solution;
      }
      cc_labls[ ii ]->setText( channel );
      setCbCurrentText( cc_solus[ ii ], solution );

      cc_labls[ ii ]->setVisible( true );
      cc_solus[ ii ]->setVisible( true );
      cc_comms[ ii ]->setVisible( true );
   }

   // Make remaining rows invisible
   for ( int ii = nchant; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( lab_none );
      cc_labls[ ii ]->setVisible( false );
      cc_solus[ ii ]->setVisible( false );
      cc_comms[ ii ]->setVisible( false );
   }
DbgLv(1) << "EGSo:inP: mxrow" << mxrow << "labls count" << cc_labls.count();
}

// Save panel controls when about to leave the panel
void US_ExperGuiSolutions::savePanel()
{
   // Get current channel counts: total, solution-filled
   status();

   int nuniqs          = 0;
   nchant              = 0;
   nchanf              = 0;
   QString chn_none( "none" );
   QString unspec      = tr( "(unspecified)" );
   srchans.clear();
   suchans.clear();
   susolus.clear();

   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Fill panel lists and parameters from GUI elements
      QString channel     = cc_labls[ ii ]->text();
DbgLv(1) << "EGSo: svP:  ii" << ii << "channel" << channel;

      if ( channel == chn_none )
         break;

      nchant++;
      srchans << channel;
      QString solution    = cc_solus[ ii ]->currentText();
DbgLv(1) << "EGSo: svP:    nchant" << nchant << "solution" << solution;

      if ( solution == unspec )
         continue;

      nchanf++;
      suchans << channel;
      susolus << solution;
      QString sol_id      = solu_ids[ solution ];
      QString ch_comment;
      QStringList cs;
DbgLv(1) << "EGSo: svP:    nchanf" << nchanf << "sol_id" << sol_id;


 QString iistr = QString::number(ii);
//if ( pro_comms.keys().contains( solution ) )
    if ( pro_comms.keys().contains( iistr ) )
      {
	//ch_comment          = pro_comms[ solution ];
	ch_comment          = pro_comms[ iistr ];
	 //ALEXEY - to remember changes to Soluton comments if manual commnets was added while returninf to "Solutons" tab
	 
	commentStrings( solution, ch_comment, cs, ii );   
      }
    else
      {
	commentStrings( solution, ch_comment, cs, ii );
      }
    
    US_Solution soludata;
    solutionData( solution, soludata );
    solu_ids [ solution ]  = sol_id;
    solu_data[ solution ]  = soludata;
    //pro_comms[ solution ]  = ch_comment;
    pro_comms[ iistr ]  = ch_comment;
   }
   
   rpSolut->chsols.resize( nchanf );
   QStringList solus;                      // Unique solutions list
   QStringList sids;                       // Corresponding Id list
DbgLv(1) << "EGSo: svP: nchanf" << nchanf << "nchant" << nchant;

   for ( int ii = 0; ii < nchanf; ii++ )
   {  // Now fill protocol from internal lists and parameters
      QString channel     = suchans[ ii ];
      QString solution    = susolus[ ii ];
      QString sol_id      = solu_ids[ solution ];
      rpSolut->chsols[ ii ].channel    = channel;
      rpSolut->chsols[ ii ].solution   = solution;
      rpSolut->chsols[ ii ].sol_id     = sol_id;
      //rpSolut->chsols[ ii ].ch_comment = pro_comms[ solution ];

      QString iistr = QString::number(ii);
      rpSolut->chsols[ ii ].ch_comment = pro_comms[ iistr ];
      
      if ( !solus.contains( solution ) )
      {
         solus << solution;
         sids  << sol_id;
         nuniqs++;
      }
   }

   rpSolut->nschan     = nchanf;    // Channels with solution given
   rpSolut->nuniqs     = nuniqs;    // Unique solutions
   rpSolut->solus      = solus;     // List of unique solutions
   rpSolut->sids       = sids;      // Corresponding solution IDs
DbgLv(1) << "EGSo: svP: nuniqs" << nuniqs;
DbgLv(1) << "EGSo: svP:  solus" << solus;
DbgLv(1) << "EGSo: svP:  sids " << sids;
}

// Get a specific panel value
QString US_ExperGuiSolutions::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "nchant"   ||
             type == "nchanf"   ||
             type == "nusols"   ||
             type == "alldone"  ||
             type == "status" )
   {  // String representation of integer
      value            = QString::number( getIValue( type ) );
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiSolutions::getIValue( const QString type )
{
   int value   = 0;
DbgLv(1) << "EGSo: gIV: type" << type;
   if      ( type == "nchant" )  { value = nchant; }
   else if ( type == "nchanf" )  { value = nchanf; }
   else if ( type == "nusols" )  { value = rpSolut->nuniqs; }
   else if ( type == "status" )  { value = status(); }
   else if ( type == "alldone" ) { value = qMin( status(), 1 ); }
DbgLv(1) << "EGSo: gIV:  value" << value;
   return value;
}

// Get a specific panel double value
double US_ExperGuiSolutions::getDValue( const QString type )
{
   double value   = 0.0;
   if ( type == "dbdisk" ) { value = 1.0; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSolutions::getLValue( const QString type )
{
   QStringList value;                      // Output list
   QStringList solus;                      // Unique solutions list

DbgLv(1) << "EGSo: gLV: type" << type;
   status();
DbgLv(1) << "EGSo: gLV:  status rtn";

   if ( type == "solutions" )
   {  // Build a list of unique solutions
      QStringList chsolus;

      // Accumulate an ordered list of channel solutions
      for ( int ii = 0; ii < rpSolut->nschan; ii++ )
      {
         chsolus << rpSolut->chsols[ ii ].solution;
      }

      // Build list of unique solutions with appended channels lists
      for ( int ii = 0; ii < rpSolut->nuniqs; ii++ )
      {
         QString solution    = rpSolut->solus[ ii ];
         int nchanso         = chsolus.count( solution );
         QStringList sochans;

         for ( int jj = 0; jj < rpSolut->nschan; jj++ )
         {  // List channels that have this solution
            if ( rpSolut->chsols[ jj ].solution == solution )
               sochans << rpSolut->chsols[ jj ].channel;
         }

         // Build solution entry "solution  ( chan1, chan2, ... )"
         QString solentry    = solution + "  ( " + sochans[ 0 ];

         for ( int jj = 1; jj < nchanso; jj++ )
         {
            solentry           += ", " + sochans[ jj ];
         }

         solentry           += " )";
         value << solentry;
      }
   }

   else if ( type == "channel_solutions" )
   {  // Build a "channel : solution" list for used channels
      for ( int ii = 0; ii < rpSolut->nschan; ii++ )
      {
         value << ( rpSolut->chsols[ ii ].channel + " : " +
                    rpSolut->chsols[ ii ].solution );
      }
   }

   else if ( type == "sochannels" )
   {  // Build a channels list for used channels
      for ( int ii = 0; ii < rpSolut->nschan; ii++ )
      {
         value << rpSolut->chsols[ ii ].channel;
      }
   }

DbgLv(1) << "EGSo: gLV:  value" << value;
   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSolutions::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiSolutions::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiSolutions::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSolutions::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiSolutions::status()
{
   nchant              = srchans.count();   // Number total solution channels
   nchanf              = rpSolut->nschan;   // Number channels filled
   nchanf              = ( nchanf > 0 ) ? nchanf : suchans.count();

   bool is_done        = ( nchant > 0  &&  nchanf == nchant );
DbgLv(1) << "EGSo:st: nchant" << nchant << "nchanf" << nchanf << " is_done"
 << is_done;
   return ( is_done ? 16 : 0 );
}

//========================= End:   Solutions section =========================


//========================= Start: Optical   section =========================

// Initialize an Optical panel, especially after clicking on its tab
void US_ExperGuiOptical::initPanel()
{
   rpOptic             = &(mainw->currProto.rpOptic);
DbgLv(1) << "EGOp:inP:  call rbO";

   rebuild_Optic();

   QString notinst     = tr( "(not installed)" );
   QString slabl_n( "none" );
DbgLv(1) << "EGOp:inP: nochan" << nochan;

   for ( int ii = 0; ii < nochan; ii++ )
   {
      QString channel     = rpOptic->chopts[ ii ].channel;
      QString scan1       = rpOptic->chopts[ ii ].scan1;
      QString scan2       = rpOptic->chopts[ ii ].scan2;
      QString scan3       = rpOptic->chopts[ ii ].scan3;
DbgLv(1) << "EGOp:inP:  ii" << ii << "channel" << channel
 << "scan1-3" << scan1 << scan2 << scan3;

      cc_labls[ ii ]->setText( channel );
      cc_labls[ ii ]->setVisible( true );

      QStringList prscans;
      if ( scan1 != notinst )  prscans << scan1;
      if ( scan2 != notinst )  prscans << scan2;
      if ( scan3 != notinst )  prscans << scan3;

      QCheckBox* ckbox1   = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2   = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3   = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
      QString ckscan1     = ckbox1->text();
      QString ckscan2     = ckbox2->text();
      QString ckscan3     = ckbox3->text();
      ckbox1->setChecked( prscans.contains( ckscan1 ) );
      //ckbox2->setChecked( prscans.contains( ckscan2 ) );   //ALEXEY do not check Interference by default
      ckbox3->setChecked( prscans.contains( ckscan3 ) );
      ckbox1->setVisible( ! ckscan1.contains( notinst ) );
      ckbox2->setVisible( ! ckscan2.contains( notinst ) );
      ckbox3->setVisible( ! ckscan3.contains( notinst ) );
   }

DbgLv(1) << "EGOp:inP: nochan" << nochan << "mxrow" << mxrow;
   // Make remaining rows invisible
   for ( int ii = nochan; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( slabl_n );
      cc_labls[ ii ]->setVisible( false );
      cc_osyss[ ii ]->button( 1 )->setVisible( false );
      cc_osyss[ ii ]->button( 2 )->setVisible( false );
      cc_osyss[ ii ]->button( 3 )->setVisible( false );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiOptical::savePanel()
{
   // Get counts of optics rows visible and used
   status();

   // Save GUI settings to optics protocol object
   rpOptic->nochan     = nochan;
   rpOptic->chopts.resize( nochan );

   for ( int ii = 0; ii < nochan; ii++ )
   {
      QCheckBox* ckbox1   = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2   = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3   = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
DbgLv(1) << "EGOp:svP:   ii" << ii << "olabl" << cc_labls[ii]->text();

      rpOptic->chopts[ ii ].channel = cc_labls[ ii ]->text();
      rpOptic->chopts[ ii ].scan1   = "";
      rpOptic->chopts[ ii ].scan2   = "";
      rpOptic->chopts[ ii ].scan3   = "";
      int kchkd           = 0;

      if ( ckbox1->isChecked() )
      {
         if      ( kchkd == 0 )  rpOptic->chopts[ ii ].scan1 = ckbox1->text();
         else if ( kchkd == 1 )  rpOptic->chopts[ ii ].scan2 = ckbox1->text();
         else if ( kchkd == 2 )  rpOptic->chopts[ ii ].scan3 = ckbox1->text();
         kchkd++;
      }

      if ( ckbox2->isChecked() )
      {
         if      ( kchkd == 0 )  rpOptic->chopts[ ii ].scan1 = ckbox2->text();
         else if ( kchkd == 1 )  rpOptic->chopts[ ii ].scan2 = ckbox2->text();
         else if ( kchkd == 2 )  rpOptic->chopts[ ii ].scan3 = ckbox2->text();
         kchkd++;
      }

      if ( ckbox3->isChecked() )
      {
         if      ( kchkd == 0 )  rpOptic->chopts[ ii ].scan1 = ckbox3->text();
         else if ( kchkd == 1 )  rpOptic->chopts[ ii ].scan2 = ckbox3->text();
         else if ( kchkd == 2 )  rpOptic->chopts[ ii ].scan3 = ckbox3->text();
         kchkd++;
      }
DbgLv(1) << "EGOp:svP:     kchkd" << kchkd << "scans:"
 << rpOptic->chopts[ii].scan1 << rpOptic->chopts[ii].scan2 << rpOptic->chopts[ii].scan3;
   }
}

// Get a specific panel value
QString US_ExperGuiOptical::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "eprofiles" )
   {
   //   value = cb_lab->currentText();
   }
   else if ( type == "alldone" )
   {
      value            = ( status() > 0 ) ? "1" : "0";
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiOptical::getIValue( const QString type )
{
   int value   = 0;
   int istat   = status();

   if      ( type == "alldone" )
      value            = ( istat > 0 ) ? 1 : 0;
   else if ( type == "status" )
      value            = istat;
   else if ( type == "nochan" )
      value            = rpOptic->nochan;
   else if ( type == "nuchan" )
      value            = nuchan;
   else if ( type == "nuvvis" )
      value            = nuvvis;

   return value;
}

// Get a specific panel double value
double US_ExperGuiOptical::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiOptical::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "profiles" )
   {
      value.clear();
      QString eslabl    = tr( "none" );
      QString notinst   = tr( "(not installed)" );

      for ( int ii = 0; ii < rpOptic->nochan; ii++ )
      {
         QString channel   = rpOptic->chopts[ ii ].channel;
         QString scan1     = rpOptic->chopts[ ii ].scan1;
         QString scan2     = rpOptic->chopts[ ii ].scan2;
         QString scan3     = rpOptic->chopts[ ii ].scan3;
DbgLv(1) << "EGOp: gPL: ii" << ii << "channel" << channel
 << "scan1-3" << scan1 << scan2 << scan3;
         QString pentry    = channel;
         int noptis        = 0;

         if ( !scan1.isEmpty()  &&  ( scan1 != notinst ) )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + scan1 );
            noptis++;
         }

         if ( !scan2.isEmpty()  &&  ( scan2 != notinst ) )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + scan2 );
            noptis++;
         }

         if ( !scan3.isEmpty()  &&  ( scan3 != notinst ) )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + scan3 );
            noptis++;
         }

         if ( noptis > 0 )
         {
            value << pentry;
         }
DbgLv(1) << "EGOp: gPL:     noptis" << noptis << "pentry" << pentry;
      }
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiOptical::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiOptical::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiOptical::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiOptical::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiOptical::status()
{
   QString notinst     = tr( "(not installed)" );
   QString l_uvvis     = tr( "UV/visible" );
   QString slabl_n( "none" );
   nochan              = rpOptic->nochan; 
   nuchan              = 0;           // Initialize count of used rows
   nuvvis              = 0;           // Initialize count of UV/vis rows
   for ( int ii = 0; ii < nochan; ii++ )
   {  // Loop to count visible rows and ones with at least one check
      QString channel     = rpOptic->chopts[ ii ].channel;
      QString scan1       = rpOptic->chopts[ ii ].scan1;
      QString scan2       = rpOptic->chopts[ ii ].scan2;
      QString scan3       = rpOptic->chopts[ ii ].scan3;
      bool have_used      = false;
      bool have_uvvis     = false;

      if ( !scan1.isEmpty()  &&  !scan1.contains( notinst ) )
      {
         have_used           = true;
         if ( scan1.contains( l_uvvis ) )
            have_uvvis          = true;
      }

      if ( !scan2.isEmpty()  &&  !scan2.contains( notinst ) )
      {
         have_used           = true;
         if ( scan2.contains( l_uvvis ) )
            have_uvvis          = true;
      }

      if ( !scan3.isEmpty()  &&  !scan3.contains( notinst ) )
      {
         have_used           = true;
         if ( scan3.contains( l_uvvis ) )
            have_uvvis          = true;
      }

      if ( have_used )
         nuchan++;

      if ( have_uvvis )
         nuvvis++;
   }

   bool is_done  = ( nochan > 0  &&  nuchan == nochan );
DbgLv(1) << "EGOp:st: nochan nuchan done" << nochan << nuchan << is_done;

   return ( is_done ? 32 : 0 );
}

//========================= End:   Optical   section =========================


//========================= Start: Ranges    section =========================
                   
// Initialize a Ranges panel, especially after clicking on its tab
void US_ExperGuiRanges::initPanel()
{
   rpRange             = &(mainw->currProto.rpRange);

DbgLv(1) << "EGwS:inP:  call rbS";
   rebuild_Ranges();

   QString ch_none( "none" );
DbgLv(1) << "EGwS:inP:  nrnchan" << nrnchan;

   for ( int ii = 0; ii < nrnchan; ii++ )
   {
      QString channel     = rchans[ ii ];
      QString labwlr;
      int kswavl          = swvlens[ ii ].count();
DbgLv(1) << "EGwS:inP:    ii" << ii << "channel" << channel;
      if ( kswavl == 0 )
         labwlr              = tr( "0 selected" );
      else if ( kswavl == 1 )
         labwlr              = tr( "1,  %1" )
                               .arg( swvlens[ ii ][ 0 ] );
      else
         labwlr              = tr( "%1,  %2 to %3" )
                               .arg( kswavl )
                               .arg( swvlens[ ii ][ 0 ] )
                               .arg( swvlens[ ii ][ kswavl - 1 ] );
      cc_labls[ ii ]->setText( channel );
      cc_lrngs[ ii ]->setText( labwlr  );
      cc_labls[ ii ]->setVisible( true );
      cc_lrngs[ ii ]->setVisible( true );
      cc_lbtos[ ii ]->setVisible( true );

      cc_wavls[ ii ]->setVisible( true );
      cc_lrads[ ii ]->setVisible( true );
      cc_hrads[ ii ]->setVisible( true );

      if ( channel.contains("reference") )   //ALEXEY do not allow to set wavelengths/radial ranges for B channels (reference) 
	{
	   cc_wavls[ ii ]->setEnabled( false );
	   cc_lrads[ ii ]->setEnabled( false );
	   cc_hrads[ ii ]->setEnabled( false );
	}

   }

   // Make remaining rows invisible
   for ( int ii = nrnchan; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( ch_none );
      cc_labls[ ii ]->setVisible( false );
      cc_wavls[ ii ]->setVisible( false );
      cc_lrngs[ ii ]->setVisible( false );
      cc_lrads[ ii ]->setVisible( false );
      cc_hrads[ ii ]->setVisible( false );
      cc_lbtos[ ii ]->setVisible( false );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiRanges::savePanel()
{
DbgLv(1) << "EGwS:svP: nrnchan" << nrnchan << "nranges" << rpRange->nranges;
   rpRange->nranges  = nrnchan;            // Protocol channels               //ALEXEY not correct BUG-BUG
   rpRange->chrngs.resize( nrnchan );    // Expand or contract?

   for ( int ii = 0; ii < nrnchan; ii++ )
   {
      rpRange->chrngs[ ii ].channel = rchans [ ii ];
      rpRange->chrngs[ ii ].lo_rad  = locrads[ ii ];
      rpRange->chrngs[ ii ].hi_rad  = hicrads[ ii ];

      rpRange->chrngs[ ii ].wvlens.clear();

      for ( int jj = 0; jj < swvlens[ ii ].count(); jj++ )
      {  // Wavelength values from selected wavelengths
         rpRange->chrngs[ ii ].wvlens << swvlens[ ii ][ jj ];
      }
DbgLv(1) << "EGwS:svP:  ii" << ii << "wvl knt" << rpRange->chrngs[ii].wvlens.count();
   }
}

// Get a specific panel value
QString US_ExperGuiRanges::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "eprofiles" )
   {
   //   value = cb_lab->currentText();
   }
   else if ( type == "alldone" )
   {
      value            = ( status() > 0 ) ? "1" : "0";
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiRanges::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "nrange" )  { value = rpRange->nranges; }
   else if ( type == "alldone" ) { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  ) { value = status(); }

   return value; 
}

// Get a specific panel double value
double US_ExperGuiRanges::getDValue( const QString type )
{
   double value   = 0.0;
   if ( type == "dbdisk" ) { value = 1.0; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiRanges::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "eprofiles" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiRanges::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiRanges::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiRanges::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiRanges::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiRanges::status()
{
   bool is_done    = true;
DbgLv(1) << "EGwS:st: nranges" << rpRange->nranges;

   for ( int ii = 0; ii < rpRange->nranges; ii++ )
   {
      if ( rpRange->chrngs[ ii ].wvlens.count() == 0 )
         is_done         = false;       // Not done if missing wavelengths
DbgLv(1) << "EGwS:st:   ii" << ii << "is_done(wvlens count)" << is_done;
   }

   return ( is_done ? 64 : 0 );
}

//========================= End:   Ranges    section =========================

//========================= Start: Upload    section =========================

// Initialize an Upload panel, especially after clicking on its tab
void US_ExperGuiUpload::initPanel()
{
   currProto       = &mainw->currProto;
   loadProto       = &mainw->loadProto;
   rps_differ      = ( mainw->currProto !=  mainw->loadProto );
   rpRotor         = &currProto->rpRotor;
   rpSpeed         = &currProto->rpSpeed;
   rpCells         = &currProto->rpCells;
   rpSolut         = &currProto->rpSolut;
   rpOptic         = &currProto->rpOptic;
   rpRange         = &currProto->rpRange;
   rpSubmt         = &currProto->rpSubmt;
if(rps_differ)
{
US_RunProtocol* cRP = currProto;
US_RunProtocol* lRP = loadProto;
DbgLv(1) << "EGUp:inP: RPs DIFFER";
DbgLv(1) << "EGUp:inP:  cPname" << cRP->protname << "lPname" << lRP->protname;
DbgLv(1) << "EGUp:inP:  cInves" << cRP->investigator << "lInves" << lRP->investigator;
DbgLv(1) << "EGUp:inP:  cPguid" << cRP->pGUID << "lPguid" << lRP->pGUID;
DbgLv(1) << "EGUp:inP:  cOhost" << cRP->optimahost << "lOhost" << lRP->optimahost;
DbgLv(1) << "EGUp:inP:  cTempe" << cRP->temperature << "lTempe" << lRP->temperature;
DbgLv(1) << "EGUp:inP:   rpRotor diff" << (cRP->rpRotor!=lRP->rpRotor);
DbgLv(1) << "EGUp:inP:   rpSpeed diff" << (cRP->rpSpeed!=lRP->rpSpeed);
DbgLv(1) << "EGUp:inP:   rpCells diff" << (cRP->rpCells!=lRP->rpCells);
DbgLv(1) << "EGUp:inP:   rpSolut diff" << (cRP->rpSolut!=lRP->rpSolut);
DbgLv(1) << "EGUp:inP:   rpOptic diff" << (cRP->rpOptic!=lRP->rpOptic);
DbgLv(1) << "EGUp:inP:   rpRange diff" << (cRP->rpRange!=lRP->rpRange);
DbgLv(1) << "EGUp:inP:   rpSubmt diff" << (cRP->rpSubmt!=lRP->rpSubmt);
}

   have_run          = ! sibSValue( "general",   "runID"    ).isEmpty();
   have_proj         = ! sibSValue( "general",   "project"  ).isEmpty();
   chgd_rotor        = ( sibIValue( "rotor",     "changed"  ) > 0 );
   chgd_speed        = ( sibIValue( "speeds",    "changed"  ) > 0 );
   have_cells        = ( sibIValue( "cells",     "alldone"  ) > 0 );
   have_solus        = ( sibIValue( "solutions", "alldone"  ) > 0 );
   have_optic        = ( sibIValue( "optical",   "alldone"  ) > 0 );
   have_range        = ( sibIValue( "ranges",    "alldone"  ) > 0 );
DbgLv(1) << "EGUp:inP: ck: run proj cent solu epro"
 << have_run << have_proj << have_cells << have_solus << have_range;
   proto_ena         = ( have_cells  &&  have_solus  &&  have_optic  &&
                         have_range );
   subm_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
                         connected );

   ck_run     ->setChecked( have_run   );
   ck_project ->setChecked( have_proj  );
   ck_rotor_ok->setChecked( chgd_rotor );
   ck_speed_ok->setChecked( chgd_speed );
   ck_centerp ->setChecked( have_cells );
   ck_solution->setChecked( have_solus );
   ck_optical ->setChecked( have_optic );
   ck_ranges  ->setChecked( have_range );
   ck_connect ->setChecked( connected  );
   ck_prot_ena->setChecked( proto_ena  );
   ck_prot_svd->setChecked( proto_svd  );
   ck_sub_enab->setChecked( subm_enab  );
   ck_sub_done->setChecked( submitted  );
   ck_rp_diff ->setChecked( rps_differ );

   pb_submit  ->setEnabled( subm_enab  );                                  // <-- Temporary enabled for testing
   pb_saverp  ->setEnabled( have_cells && have_solus && have_range );
}

// Save panel controls when about to leave the panel
void US_ExperGuiUpload::savePanel()
{
}

// Get a specific panel value
QString US_ExperGuiUpload::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "alldone"  ||
             type == "status"   ||
             type == "len_xml"  ||
             type == "len_json" )
      value  = QString::number( getIValue( type ) );
   else if ( type == "xml" )  
      value  = rpSubmt->us_xml;
   else if ( type == "json" ) 
      value  = rpSubmt->op_json;

   return value;
}

// Get a specific panel integer value
int US_ExperGuiUpload::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "alldone" )  { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  )  { value = status(); }
   else if ( type == "len_xml" )  { value = rpSubmt->us_xml.length(); }
   else if ( type == "len_json" ) { value = rpSubmt->op_json.length(); }
   return value;
}

// Get a specific panel double value
double US_ExperGuiUpload::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiUpload::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "uploaded" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiUpload::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiUpload::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiUpload::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiUpload::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status string for the panel
int US_ExperGuiUpload::status()
{
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
   return ( is_done ? 128 : 0 );
}

//========================= End:   Upload    section =========================

