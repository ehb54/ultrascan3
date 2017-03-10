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

#include "us_experiment_main.h"
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
   else if ( child == "spectra"  ) { value = epanSpectra  ->getSValue( type ); }
   else if ( child == "upload"   ) { value = epanUpload   ->getSValue( type ); }
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
   else if ( child == "spectra"  ) { value = epanSpectra  ->getIValue( type ); }
   else if ( child == "upload"   ) { value = epanUpload   ->getIValue( type ); }
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
   else if ( child == "spectra"  ) { value = epanSpectra  ->getDValue( type ); }
   else if ( child == "upload"   ) { value = epanUpload   ->getDValue( type ); }
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
   else if ( child == "spectra"  ) { value = epanSpectra  ->getLValue( type ); }
   else if ( child == "upload"   ) { value = epanUpload   ->getLValue( type ); }

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
   else if ( curr_panx == 6 ) epanSpectra  ->savePanel();
   else if ( curr_panx == 7 ) epanRotor    ->savePanel();

   curr_panx              = panx;         // Set new current panel

   // Initialize the new current panel after possible changes
   if      ( panx == 0 )      epanGeneral  ->initPanel();
   else if ( panx == 1 )      epanRotor    ->initPanel();
   else if ( panx == 2 )      epanSpeeds   ->initPanel();
   else if ( panx == 3 )      epanCells    ->initPanel();
   else if ( panx == 4 )      epanSolutions->initPanel();
   else if ( panx == 5 )      epanOptical  ->initPanel();
   else if ( panx == 6 )      epanSpectra  ->initPanel();
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
              + epanSpectra  ->status()
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
   else if ( curr_panx == 6 ) epanSpectra  ->help();
   else if ( curr_panx == 7 ) epanUpload   ->help();
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
   epanSpectra  ->initPanel();
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
   // Populate GUI settings from protocol controls
   bool was_changed     = changed;       // Save changed state
   setCbCurrentText( cb_lab,    QString::number( rpRotor->labID ) + ": "
                                + rpRotor->laboratory );
   setCbCurrentText( cb_rotor,  QString::number( rpRotor->rotID ) + ": "
                                + rpRotor->rotor );
   setCbCurrentText( cb_calibr, QString::number( rpRotor->calID ) + ": "
                                + rpRotor->calibration );
   changed              = was_changed;   // Restore changed state
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
   rpRotor->labID       = QString( cal ).section( ":", 0, 0 ).toInt();
   rpRotor->rotID       = QString( cal ).section( ":", 0, 0 ).toInt();
   rpRotor->calID       = QString( cal ).section( ":", 0, 0 ).toInt();
}

// Get a specific panel string value
QString US_ExperGuiRotor::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "lab" )     { value = rpRotor->laboratory; }
   else if ( type == "rotor" )   { value = rpRotor->rotor; }
   else if ( type == "calib" )   { value = rpRotor->calibration; }
   else if ( type == "changed" ) { value = changed ? "1" : "0"; }

   else if ( type == "abstractRotor" )
   {
      QString rotor  = QString( rpRotor->rotor ).section( ":", 1, 1 )
                                                .simplified();
      int arID       = -1;
      for ( int ii = 0; ii < rotors.count(); ii++ )
      {
         if ( rotors[ ii ].name == rotor )
         {
            arID           = rotors[ ii ].abstractRotorID;
            break;
         }
      }
      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value          = arotors[ ii ].name;
            break;
         }
      }
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
      QString rotor  = rpRotor->rotor;
      int arID       = -1;
DbgLv(1) << "EGRo:gIV:  rotor" << rotor << rpRotor->rotor
 << "r_count" << rotors.count();

      for ( int ii = 0; ii < rotors.count(); ii++ )
      {
DbgLv(1) << "EGRo:gIV:   ii" << ii << "i_rotor" << rotors[ii].name;
         if ( rotors[ ii ].name == rotor )
         {
            arID           = rotors[ ii ].abstractRotorID;
DbgLv(1) << "EGRo:gIV:    +MATCH+ arID" << arID;
            break;
         }
      }
DbgLv(1) << "EGRo:gIV:   arID" << arID;

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
DbgLv(1) << "EGRo:gIV:    ii" << ii << "i_arID" << arotors[ii].ID;
         if ( arotors[ ii ].ID == arID )
         {
            value          = arotors[ ii ].numHoles;
DbgLv(1) << "EGRo:gIV:     +MATCH+ numHoles" << value;
            break;
         }
      }
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
   if      ( type == "dbdisk" ) { value = 1; }

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
   // Populate GUI settings from protocol controls
   int nstep            = rpSpeed ->nstep;
   int istep            = qMin( (nstep-1), qMax( 0, cb_prof->currentIndex() ) );
   double duration      = rpSpeed->ssteps[ istep ].duration;
   double delay         = rpSpeed->ssteps[ istep ].delay;

   double durathr       = qFloor( duration / 60.0 );
   double duratmin      = duration - ( durathr * 60.0 );

   double delaymin      = qFloor( delay / 60.0 );
   double delaysec      = delay - ( delaymin * 60.0 );
   double delayhr       = qFloor( delaymin / 60.0 );
   delaymin            -= ( delayhr * 60.0 );

   bool was_changed     = changed;       // Save changed state
   ct_count ->setValue( nstep  );
   ct_speed ->setValue( rpSpeed->ssteps[ istep ].speed );
   ct_accel ->setValue( rpSpeed->ssteps[ istep ].accel );
   ct_durhr ->setValue( durathr  );
   ct_durmin->setValue( duratmin );
   ct_dlyhr ->setValue( delayhr  );
   ct_dlymin->setValue( delaymin );
   ct_dlysec->setValue( delaysec );
   changed              = was_changed;   // Restore changed state

DbgLv(1) << "EGSp:inP: nstep" << nstep;
   // Populate internal speed-step control
   ssvals.resize( nstep );
   for ( int ii = 0; ii < nstep; ii++ )
   {
      ssvals[ ii ][ "speed"    ] = rpSpeed->ssteps[ ii ].speed;
      ssvals[ ii ][ "accel"    ] = rpSpeed->ssteps[ ii ].accel;
      ssvals[ ii ][ "duration" ] = rpSpeed->ssteps[ ii ].duration;
      ssvals[ ii ][ "delay"    ] = rpSpeed->ssteps[ ii ].delay;
DbgLv(1) << "EGSp:inP:  ii" << ii << "speed accel durat delay"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"];
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiSpeeds::savePanel()
{
   // Populate protocol speed controls from internal panel control
   int nstep            = ssvals.count();
   rpSpeed ->nstep      = nstep;
DbgLv(1) << "EGSp:svP: nstep" << nstep;

   for ( int ii = 0; ii < nstep; ii++ )
   {
      rpSpeed->ssteps[ ii ].speed    = ssvals[ ii ][ "speed"    ];
      rpSpeed->ssteps[ ii ].accel    = ssvals[ ii ][ "accel"    ];
      rpSpeed->ssteps[ ii ].duration = ssvals[ ii ][ "duration" ];
      rpSpeed->ssteps[ ii ].delay    = ssvals[ ii ][ "delay"    ];
DbgLv(1) << "EGSp:svP:  ii" << ii << "speed accel durat delay"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"];
   }
}

// Get a specific panel value
QString US_ExperGuiSpeeds::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "nspeeds" )
   {
      value = QString::number( ct_count ->value() );
   }
   else if ( type == "speed" )
   {
      value = QString::number( ct_speed ->value() );
   }
   else if ( type == "accel" )
   {
      value = QString::number( ct_accel ->value() );
   }
   else if ( type == "durhr" )
   {
      value = QString::number( ct_durhr ->value() );
   }
   else if ( type == "durmin" )
   {
      value = QString::number( ct_durmin->value() );
   }
   else if ( type == "delayhr" )
   {
      value = QString::number( ct_dlyhr ->value() );
   }
   else if ( type == "delaymin" )
   {
      value = QString::number( ct_dlymin->value() );
   }
   else if ( type == "delaysec" )
   {
      value = QString::number( ct_dlysec->value() );
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
         double duration      = rpSpeed->ssteps[ ii ].duration;
         double delay         = rpSpeed->ssteps[ ii ].delay;
         double durathrs      = qFloor( duration / 60.0 );
         double duratmin      = duration - ( durathrs * 60.0 );
         double delaymin      = qFloor( delay / 60.0 );
         double delaysec      = delay - ( delaymin * 60.0 );

         value << tr( "%1 rpm" ).arg( speed );
         value << tr( "%1 rpm/sec" ).arg( accel );
         value << tr( "%1 minutes  ( %2 h %3 m )" )
                  .arg( duration ).arg( durathrs ).arg( duratmin );
         value << tr( "%1 seconds  ( %2 m %3 s )" )
                  .arg( delay )   .arg( delaymin ).arg( delaysec );
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

   for ( int ii = 0; ii < nholes; ii++ )
   {  // Count used (non-empty) cell rows
      QString celnam      = cc_labls[ ii ]->text();
      QString centp       = cc_cenps[ ii ]->currentText();
DbgLv(1) << "EGCe:svP:  ii" << ii << "celnam" << celnam << "centp" << centp;
      if ( ! centp.contains( tr( "empty" ) ) )
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
      for ( int ii = 0; ii < nholes; ii++ )
      {
         QString celnm       = cc_labls[ ii ]->text();
         QString centp       = cc_cenps[ ii ]->currentText();
         QString windo       = cc_winds[ ii ]->currentText();
DbgLv(1) << "EGCe:getSL:   ii Text" << ii << centp;

         if ( ! centp.contains( tr( "empty" ) ) )
         {
            QString centry   = ( ( ii + 1 ) != nholes )
                             ? celnm + " : " + centp + "  ( " + windo + " )"
                             : celnm + " : " + centp;
            value << centry;
         }
      }

      if ( value.count() == 0 )
      {  // Nothing from the GUI, so get it from the protocol
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
   QString unspec      = tr( "(unspecified)" );
   QString lab_none( "none" );

   // Build the set of channel labels implied by centerpieces given
   QStringList chn_labls;
   int nclabs          = 0;  // Count of solution panel channel rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
   
DbgLv(1) << "EGSo:inP: cpnames" << cpnames;

   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname      = cpnames[ ii ];
      int chx             = cpname.indexOf( "-channel" );
DbgLv(1) << "EGSo:inP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell       = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan           = cpname.left( chx ).section( " ", -1, -1 )
                                 .simplified().toInt();
DbgLv(1) << "EGSo:inP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn      = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGSo:inP:      jj" << jj << "celchn" << celchn;
            chn_labls << celchn;
            nclabs++;
         }
      }
   }

   // Build the list of channels with solutions from protocol
   QStringList sol_labls;
   nchanf              = rpSolut->nschan;  // Number solution channels
   for ( int ii = 0; ii < nchanf; ii++ )
   {
      sol_labls << rpSolut->chsols[ ii ].channel;
DbgLv(1) << "EGSo:inP:  ii" << ii << "rp channel" << rpSolut->chsols[ii].channel;
   }
DbgLv(1) << "EGSo:inP: sol_labls" << sol_labls;

   // Fill in rows
   nchant              = nclabs;
   nchanu              = 0;
   for ( int ii = 0; ii < nchant; ii++ )
   {
      QString channel     = chn_labls[ ii ];
      int solx            = sol_labls.indexOf( channel );
      QString solution    = unspec;

      if ( solx >= 0 )
      {
         solution            = rpSolut->chsols[ solx ].solution;
      }
      else
         nchanu++;
DbgLv(1) << "EGSo:inP:  ii" << ii << "channel" << channel << "solx" << solx << "solution" << solution;
  
      cc_labls[ ii ]->setText( channel );
      setCbCurrentText( cc_solus[ ii ], solution );
DbgLv(1) << "EGSo:inP:      pan-row solut" << cc_solus[ii]->currentText();
      setCbCurrentText( cc_solus[ ii ], solution );
DbgLv(1) << "EGSo:inP:      pan-row solut2" << cc_solus[ii]->currentText();
  
      cc_labls[ ii ]->setVisible( true );
      cc_solus[ ii ]->setVisible( true );
      cc_comms[ ii ]->setVisible( true );
   }
DbgLv(1) << "EGSo:inP: nchant" << nchant << "nchanf" << nchanf << "nchanu" << nchanu;

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
   // Get current channel counts: total, unspecified, solution-filled
   status();

   int nuniqs          = 0;
   rpSolut->chsols.resize( nchanf );
   QStringList solus;                      // Unique solutions list
   QStringList sids;                       // Corresponding Id list
   QString unspec      = tr( "(unspecified)" );
   int kk              = 0;
DbgLv(1) << "EGSo: svP: nchanf" << nchanf << "nchant" << nchant;

   for ( int ii = 0; ii < nchant; ii++ )
   {
      QString channel     = cc_labls[ ii ]->text();
      QString solution    = cc_solus[ ii ]->currentText();
DbgLv(1) << "EGSo: svP:  ii" << ii << "channel" << channel << "solution" << solution;

      if ( ! solution.contains( unspec ) )
      {  // Solution given
         QString sol_id      = solu_ids[ solution ];
         QString ch_comment;
         QStringList cs;

         commentStrings( solution, ch_comment, cs );

         if ( ! solus.contains( solution ) )
         {
            solus << solution;    // Update unique solutions list
            sids  << sol_id;      //  and Ids list
            nuniqs++;
         }

DbgLv(1) << "EGSo: svP:    kk" << kk << "sol_id" << sol_id << "ch_comment" << ch_comment;
         rpSolut->chsols[ kk ].channel    = channel;
         rpSolut->chsols[ kk ].solution   = solution;
         rpSolut->chsols[ kk ].sol_id     = sol_id;
         rpSolut->chsols[ kk ].ch_comment = ch_comment;
         kk++;
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
             type == "nchanu"   ||
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
   if      ( type == "nchant" )  { value = nchant; }
   else if ( type == "nchanu" )  { value = nchanu; }
   else if ( type == "nchanf" )  { value = nchanf; }
   else if ( type == "nusols" )  { value = rpSolut->nuniqs; }
   else if ( type == "status" )  { value = status(); }
   else if ( type == "alldone" ) { value = qMin( status(), 1 ); }
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
   QMap< QString, QStringList >  sochans;  // Solution-to-channels map

   status();

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
   nchanf              = rpSolut->nschan;   // Number channels filled
   nchanu              = nchant - nchanf;   // Number channels unspecified

   bool is_done        = ( nchant > 0  &&  nchanf == nchant );
DbgLv(1) << "EGSo:st: nchant nchanf nchanu is_done"
 << nchant << nchanf << nchanu << is_done;
   return ( is_done ? 16 : 0 );
}

//========================= End:   Solutions section =========================


//========================= Start: Optical   section =========================

// Initialize an Optical panel, especially after clicking on its tab
void US_ExperGuiOptical::initPanel()
{
   rpOptic             = &(mainw->currProto.rpOptic);
   int norows          = 0;  // Count of visible optical rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
DbgLv(1) << "EGOp:inP: cpnames" << cpnames;
   QStringList olabls;       // Optical row labels (cell/chan)
   QString notinst     = tr( "(not installed)" );
   QString slabl_n( "none" );

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname      = cpnames[ ii ];
      int chx             = cpname.indexOf( "-channel" );
DbgLv(1) << "EGOp:inP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell       = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan           = cpname.left( chx ).section( " ", -1, -1 )
                              .simplified().toInt();
DbgLv(1) << "EGOp:inP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn      = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGOp:inP:      jj" << jj << "celchn" << celchn;
            olabls << celchn;
            norows++;
         }
      }
   }
DbgLv(1) << "EGOp:inP:  norows" << norows << "base channels" << olabls;

   // Build the list of channels specified in protocol
   QStringList opt_labls;
   for ( int ii = 0; ii < rpOptic->nochan; ii++ )
   {
      opt_labls << rpOptic->chopts[ ii ].channel;
   }

DbgLv(1) << "EGOp:inP:  nochan" << rpOptic->nochan << "proto channels" << opt_labls;
 
   // Set cell/channel labels and checkboxes; make visible, all live rows
   for ( int ii = 0; ii < norows; ii++ )
   {
      QString channel     = olabls[ ii ];
DbgLv(1) << "EGOp:inP:   ii" << ii << "channel" << channel;
      QCheckBox* ckbox1   = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2   = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3   = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
      QString ckscan1     = ckbox1->text();
      QString ckscan2     = ckbox2->text();
      QString ckscan3     = ckbox3->text();
      QStringList prscans;
      int optx            = opt_labls.indexOf( channel );

      if ( optx >= 0 )
      {  // Channel in protocol list
         prscans << rpOptic->chopts[ optx ].scan1
                 << rpOptic->chopts[ optx ].scan2
                 << rpOptic->chopts[ optx ].scan3;
DbgLv(1) << "EGOp:inP:    PROTO prscans" << prscans;
      }

      else
      {  // Not in protocol:  defer to existing GUI state
         if ( ckbox1->isChecked() )  prscans << ckscan1;
         if ( ckbox2->isChecked() )  prscans << ckscan2;
         if ( ckbox3->isChecked() )  prscans << ckscan3;
DbgLv(1) << "EGOp:inP:    GUI prscans" << prscans;
      }

      cc_labls[ ii ]->setText( channel );
      cc_labls[ ii ]->setVisible( true );
      ckbox1->setChecked( prscans.contains( ckscan1 ) );
      ckbox2->setChecked( prscans.contains( ckscan2 ) );
      ckbox3->setChecked( prscans.contains( ckscan3 ) );
      ckbox1->setVisible( ! ckscan1.contains( notinst ) );
      ckbox2->setVisible( ! ckscan2.contains( notinst ) );
      ckbox3->setVisible( ! ckscan3.contains( notinst ) );
DbgLv(1) << "EGOp:inP:    boxtexts"
 << ckbox1->text() << ckbox2->text() << ckbox3->text();
DbgLv(1) << "EGOp:inP:     checked:"
 << ckbox1->isChecked() << ckbox2->isChecked() << ckbox3->isChecked();
   }


   // Make remaining rows invisible
   for ( int ii = norows; ii < mxrow; ii++ )
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
   if      ( type == "alldone" )
      value            = ( status() > 0 ) ? 1 : 0;
   else if ( type == "status" )
      value            = status();
   else if ( type == "nochan" )
      value            = rpOptic->nochan;

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
      QString eslabl   = tr( "none" );
      QString notinst  = tr( "(not installed)" );
      int nsrow        = 0;

      // Build optical profile entries
      for ( int ii = 0; ii < mxrow; ii++ )
      {
         QString clabl     = cc_labls[ ii ]->text();
DbgLv(1) << "EGOp: gPL:" << ii << clabl;

         if ( clabl == eslabl )
            break;

         nsrow++;
         QCheckBox* ckbox1 = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
         QCheckBox* ckbox2 = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
         QCheckBox* ckbox3 = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
         QString cbtext1   = ckbox1->text();
         QString cbtext2   = ckbox2->text();
         QString cbtext3   = ckbox3->text();
         bool chked1       = ckbox1->isChecked();
         bool chked2       = ckbox2->isChecked();
         bool chked3       = ckbox3->isChecked();
DbgLv(1) << "EGOp: gPL:   (1) text" << cbtext1 << "checked" << chked1;
DbgLv(1) << "EGOp: gPL:   (2) text" << cbtext2 << "checked" << chked2;
DbgLv(1) << "EGOp: gPL:   (3) text" << cbtext3 << "checked" << chked3;
         int noptis        = 0;
         QString pentry( "" );

         if ( !cbtext1.isEmpty()  &&  ( cbtext1 != notinst )  && chked1 )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + cbtext1 );
            noptis++;
         }

         if ( !cbtext2.isEmpty()  &&  ( cbtext2 != notinst )  && chked2 )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + cbtext2 );
            noptis++;
         }

         if ( !cbtext3.isEmpty()  &&  ( cbtext3 != notinst )  && chked3 )
         {  // Checked optical system:  add to profile entry
            pentry           += ( ":" + cbtext3 );
            noptis++;
         }

         if ( noptis > 0 )
         {
            value << clabl + pentry;
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
   QString slabl_n( "none" );
   nochan              = 0;           // Initialize count of optical rows
   nuchan              = 0;           // Initialize count of used rows
   for ( int ii = 0; ii < mxrow; ii++ )
   {  // Loop to count visible rows and ones with at least one check
      QString channel     = cc_labls[ ii ]->text();
      if ( channel == slabl_n )
         break;                       // Break at "none" channel

      nochan++;                       // Otherwise, bump visible count
DbgLv(1) << "EGOp:st:  ii" << ii << "nochan" << nochan;
      
      QCheckBox* ckbox1   = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2   = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3   = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
DbgLv(1) << "EGOp:st:   cktexts"
 << ckbox1->text() << ckbox2->text() << ckbox3->text();
DbgLv(1) << "EGOp:st:   ckflags"
 << ckbox1->isChecked() << ckbox2->isChecked() << ckbox3->isChecked();
      // Is at least one checkbox visible (absent "not installed")?
      bool one_visi       = ( ( ckbox1->text() != notinst )  ||
                              ( ckbox2->text() != notinst )  ||
                              ( ckbox3->text() != notinst ) );
      // Is at least one checkbox checked?
      bool one_chkd       = ( ckbox1->isChecked()  ||
                              ckbox2->isChecked()  ||
                              ckbox3->isChecked() );

      if ( one_visi  &&  one_chkd )
         nuchan++;                    // This row is used:  bump count
   }

   bool is_done  = ( nochan > 0  &&  nuchan == nochan );
DbgLv(1) << "EGOp:st: nochan nuchan done" << nochan << nuchan << is_done;
//bool is_done=false;
//is_done=true;
   return ( is_done ? 32 : 0 );
}

//========================= End:   Optical   section =========================


//========================= Start: Spectra   section =========================
                   
// Initialize a Spectra panel, especially after clicking on its tab
void US_ExperGuiSpectra::initPanel()
{
   QString ch_none( "none" );
   int nsrows          = 0;  // Count of visible spectra rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
DbgLv(1) << "EGwS:inP: cpnames" << cpnames;
   QStringList opprofs = sibLValue( "optical", "profiles" );
DbgLv(1) << "EGwS:inP:  opprofs" << opprofs;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname   = cpnames[ ii ];
      int chx          = cpname.indexOf( "-channel" );
DbgLv(1) << "EGwS:inP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell    = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan        = cpname.left( chx ).section( " ", -1, -1 )
                              .simplified().toInt();
         bool show        = true;

DbgLv(1) << "EGwS:inP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn   = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGwS:inP:      jj" << jj << "celchn" << celchn;
            for ( int kk = 0; kk < opprofs.size(); kk++ )
            {
               QString pentry  = opprofs[ kk ];
               QString pcchan  = QString( pentry ).section( ":", 0, 0 );
               if ( pcchan == celchn )
               {
                  show            = pentry.contains( tr( "UV/visible" ) );
               }
            }

            if ( show )
            {
               slabls << celchn;
               nsrows++;
            }
         }
      }
      
   }

   int nspect       = rpSpect->nspect;
   QString unavail  = tr( "(unavailable)" );
DbgLv(1) << "EGwS:inP:  nsrows" << nsrows << "nspect" << nspect
 << "k_labls" << cc_labls.count();
 
   // Set cell/channel labels and elements; make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      QString chan     = slabls[ ii ];
      QString prochan  = ( ii < nspect ) ? rpSpect->chspecs[ ii ].channel : "";
      cc_labls[ ii ]->setText( chan );
      cc_labls[ ii ]->setVisible( true );
      cc_wavls[ ii ]->setVisible( true );
      cc_optis[ ii ]->setVisible( true );
      cc_loads[ ii ]->setVisible( true );
      cc_manus[ ii ]->setVisible( true );
      cc_dones[ ii ]->setVisible( true );
DbgLv(1) << "EGwS:inP:    ii" << ii << "chan" << chan << "prochan" << prochan;

      if ( chan == prochan )
      {  // Panel row channel matches protocol: check any "auto"
         QString typeinp  = rpSpect->chspecs[ ii ].typeinp;
DbgLv(1) << "EGwS:inP:      rp typeinp" << typeinp;
         cc_optis[ ii ]->setChecked( typeinp == "auto" );
      }
   }

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( ch_none );
      cc_labls[ ii ]->setVisible( false );
      cc_wavls[ ii ]->setVisible( false );
      cc_optis[ ii ]->setVisible( false );
      cc_loads[ ii ]->setVisible( false );
      cc_manus[ ii ]->setVisible( false );
      cc_dones[ ii ]->setVisible( false );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiSpectra::savePanel()
{
   // Nothing to do:  protocol was dynamically changed
}

// Get a specific panel value
QString US_ExperGuiSpectra::getSValue( const QString type )
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
int US_ExperGuiSpectra::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "nspect" )  { value = rpSpect->nspect; }
   else if ( type == "alldone" ) { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  ) { value = status(); }

   return value; 
}

// Get a specific panel double value
double US_ExperGuiSpectra::getDValue( const QString type )
{
   double value   = 0.0;
   if ( type == "dbdisk" ) { value = 1.0; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiSpectra::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "eprofiles" )
   {
      //value << le_runid->text();
   }

   return value;
}

// Get a specific panel value from a sibling panel
QString US_ExperGuiSpectra::sibSValue( const QString sibling, const QString type )
{ return mainw->childSValue( sibling, type ); }

// Get a specific panel integer value from a sibling panel
int US_ExperGuiSpectra::sibIValue( const QString sibling, const QString type )
{ return mainw->childIValue( sibling, type ); }

// Get a specific panel double value from a sibling panel
double US_ExperGuiSpectra::sibDValue( const QString sibling, const QString type )
{ return mainw->childDValue( sibling, type ); }

// Get a specific panel list from a sibling panel
QStringList US_ExperGuiSpectra::sibLValue( const QString sibling, const QString type )
{ return mainw->childLValue( sibling, type ); }

// Return status flag for the panel
int US_ExperGuiSpectra::status()
{
   bool is_done    = true;
DbgLv(1) << "EGwS:st: nspect" << rpSpect->nspect;

   for ( int ii = 0; ii < rpSpect->nspect; ii++ )
   {
      if ( rpSpect->chspecs[ ii ].lambdas.count() == 0 )
         is_done         = false;       // Not done if missing wavelengths
DbgLv(1) << "EGwS:st:   ii" << ii << "is_done(lambdas count)" << is_done;

      else if ( rpSpect->chspecs[ ii ].typeinp == "auto" )
         continue;                      // OK so far, if "auto"

      else if ( rpSpect->chspecs[ ii ].values[ 0 ] == 0.0 )
         is_done         = false;       // Not done if missing values
DbgLv(1) << "EGwS:st:     is_done(non-0 values)" << is_done;
   }

   return ( is_done ? 64 : 0 );
}

//========================= End:   Spectra   section =========================

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
   rpSpect         = &currProto->rpSpect;
   rpUload         = &currProto->rpUload;
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
DbgLv(1) << "EGUp:inP:   rpSpect diff" << (cRP->rpSpect!=lRP->rpSpect);
DbgLv(1) << "EGUp:inP:   rpUload diff" << (cRP->rpUload!=lRP->rpUload);
}

   have_run          = ! sibSValue( "general",   "runID"    ).isEmpty();
   have_proj         = ! sibSValue( "general",   "project"  ).isEmpty();
   chgd_rotor        = ( sibIValue( "rotor",     "changed"  ) > 0 );
   chgd_speed        = ( sibIValue( "speeds",    "changed"  ) > 0 );
   have_cells        = ( sibIValue( "cells",     "alldone"  ) > 0 );
   have_solus        = ( sibIValue( "solutions", "alldone"  ) > 0 );
   have_optic        = ( sibIValue( "optical",   "alldone"  ) > 0 );
   have_spect        = ( sibIValue( "spectra",   "alldone"  ) > 0 );
DbgLv(1) << "EGUp:inP: ck: run proj cent solu epro"
 << have_run << have_proj << have_cells << have_solus << have_spect;
   proto_ena         = ( have_cells  &&  have_solus  &&  have_optic  &&
                         have_spect );
   upld_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
                         connected );

   ck_run     ->setChecked( have_run   );
   ck_project ->setChecked( have_proj  );
   ck_rotor_ok->setChecked( chgd_rotor );
   ck_speed_ok->setChecked( chgd_speed );
   ck_centerp ->setChecked( have_cells );
   ck_solution->setChecked( have_solus );
   ck_optical ->setChecked( have_optic );
   ck_spectra ->setChecked( have_spect );
   ck_connect ->setChecked( connected  );
   ck_prot_ena->setChecked( proto_ena  );
   ck_prot_svd->setChecked( proto_svd  );
   ck_upl_enab->setChecked( upld_enab  );
   ck_upl_done->setChecked( uploaded   );
   ck_rp_diff ->setChecked( rps_differ );

   pb_upload  ->setEnabled( upld_enab  );
   pb_saverp  ->setEnabled( have_cells &&  have_solus &&  have_spect );
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
      value  = rpUload->us_xml;
   else if ( type == "json" ) 
      value  = rpUload->op_json;

   return value;
}

// Get a specific panel integer value
int US_ExperGuiUpload::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "alldone" )  { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  )  { value = status(); }
   else if ( type == "len_xml" )  { value = rpUload->us_xml.length(); }
   else if ( type == "len_json" ) { value = rpUload->op_json.length(); }
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

