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
   if      ( curr_panx == panx )  return;
   if      ( curr_panx == 0 ) epanGeneral  ->savePanel();
   else if ( curr_panx == 1 ) epanRotor    ->savePanel();
   else if ( curr_panx == 2 ) epanSpeeds   ->savePanel();
   else if ( curr_panx == 3 ) epanCells    ->savePanel();
   else if ( curr_panx == 4 ) epanSolutions->savePanel();
   else if ( curr_panx == 5 ) epanOptical  ->savePanel();
   else if ( curr_panx == 6 ) epanSpectra  ->savePanel();
   else if ( curr_panx == 7 ) epanRotor    ->savePanel();

   curr_panx              = panx;  // Set new current panel

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
   if ( type == "dbdisk" ) { value = use_db ? 1 : 0; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiGeneral::getDValue( const QString type )
{
   double value   = 0.0;
   if ( type == "temperature" ) { value = currProto->temperature; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiGeneral::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "centerpieces" )
   {
      value = cp_names;
   }

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
   cb_lab   ->setCurrentText( rpRotor->laboratory );
   cb_rotor ->setCurrentText( rpRotor->rotor );
   cb_calibr->setCurrentText( rpRotor->calibration );
}

// Save panel controls when about to leave the panel
void US_ExperGuiRotor::savePanel()
{
   // Populate protocol controls from GUI settings
   rpRotor->laboratory  = cb_lab   ->currentText();
   rpRotor->rotor       = cb_rotor ->currentText();
   rpRotor->calibration = cb_calibr->currentText();
}

// Get a specific panel value
QString US_ExperGuiRotor::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "lab" )
   {
      value = rpRotor->laboratory;
   }
   else if ( type == "rotor" )
   {
      value = rpRotor->rotor;
   }
   else if ( type == "calib" )
   {
      value = rpRotor->calibration;
   }
   else if ( type == "abstractRotor" )
   {
      int rx      = cb_rotor->currentIndex();
      int arID    = rotors[ rx ].abstractRotorID;
      value       = "";

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value       = arotors[ ii ].name;
            break;
         }
      }
   }
   else if ( type == "nholes" )
   {
      int rx      = cb_rotor->currentIndex();
      int arID    = rotors[ rx ].abstractRotorID;
      value       = "";

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value       = QString::number( arotors[ ii ].numHoles );
            break;
         }
      }
   }
   else if ( type == "changed" )
   {
      value       = changed ? "1" : "0";
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiRotor::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "nholes" )
   {
      int rx      = cb_rotor->currentIndex();
      int arID    = rotors[ rx ].abstractRotorID;
      value       = 0;

      for ( int ii = 0; ii < arotors.count(); ii++ )
      {
         if ( arotors[ ii ].ID == arID )
         {
            value       = arotors[ ii ].numHoles;
            break;
         }
      }
   }
   else if ( type == "changed" )
   {
      value       = changed ? 1 : 0;
   }

   return value;
}

// Get a specific panel double value
double US_ExperGuiRotor::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

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
   bool is_done = ( cb_lab   ->currentIndex() >= 0  &&
                    cb_rotor ->currentIndex() >= 0  &&
                    cb_calibr->currentIndex() >= 0 );
   return ( is_done ? 2 : 0 );
}

//========================= End:   Rotor     section =========================


//========================= Start: Speeds    section =========================

// Initialize a Speeds panel, especially after clicking on its tab
void US_ExperGuiSpeeds::initPanel()
{
}

// Save panel controls when about to leave the panel
void US_ExperGuiSpeeds::savePanel()
{
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
   if ( type == "dbdisk" ) { value = 1; }
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
   {  // Compose list of all speed-step values
      value.clear();
      int nspeed  = (int)ct_count ->value();
DbgLv(1) << "EGSp:getLV: nspeed" << nspeed;

      for ( int ii = 0; ii < nspeed; ii++ )
      {  // Build list of QString forms of speed-step int
         for ( int jj = 0; jj < ssvals[ ii ].size(); jj++ )
         {
            QStringList keys = ssvals[ ii ].keys();
            for ( int kk = 0; kk < keys.count(); kk++ )
            {
               QString key      = keys[ kk ];
               int ssval        = ssvals[ ii ][ key ];
               value << ( key + ":" + QString::number( ssval ) );
            }
         }
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
   int nholes          = sibSValue( "rotor", "nholes" ).toInt();
   int icbal           = nholes - 1;     // Counter-balance index
   QStringList sl_bals;
   sl_bals << "empty (counterbalance)"
           << "Beckman counterbalance"
           << "Titanium counterbalance"
           << "Fluorescence 5-channel counterbalance";
DbgLv(1) << "EGC:initP:  nholes mxcels" << nholes << mxcels
 << "icbal" << icbal;

   for ( int ii = 0; ii < mxcels; ii++ )
   {
DbgLv(1) << "EGC:initP:   ii cenps-count" << ii << cc_cenps[ii]->count();
      bool make_vis       = ( ii < nholes );
      QComboBox* cb_cenp  = cc_cenps[ ii ];
      QComboBox* cb_wind  = cc_winds[ ii ];
      QString cp_text     = cb_cenp->currentText();
      cc_labls[ ii ]->setVisible( make_vis );
      cb_cenp       ->setVisible( make_vis );
      cb_wind       ->setVisible( make_vis );

      if ( ii == icbal )
      {  // This is a counterbalance cell
         cb_cenp->clear();
         cb_cenp->addItems( sl_bals );
         cb_wind->setVisible( false );
         // Select counterbalance based on cross cell
         int halfnh          = nholes / 2; // Half number holes
         int xrow            = ( ii < halfnh ) ? ii + halfnh : ii - halfnh;
         int jsel            = 1;       // Usually "Beckman counterbalance"
         QString cpname      = cc_cenps[ xrow ]->currentText();
         if ( tcb_centps.contains( cpname ) )
            jsel                = 2;    // In some cases "Titanium counterbalance"
         cb_cenp->setCurrentIndex( jsel );
      }

      else if ( ii < nholes  && cp_text.contains( "counterbalance" ) )
      {  // Was previously counterbalance, but now needs to be centerpiece
         cb_cenp->clear();
         cb_cenp->addItems( cpnames );
         cb_wind->setVisible( true );
      }
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiCells::savePanel()
{
}

// Get a specific panel value
QString US_ExperGuiCells::getSValue( const QString type )
{
DbgLv(1) << "EGC:getSV: type" << type;
   QString value( "" );
   int nholes  = sibSValue( "rotor", "nholes" ).toInt();

   if ( type == "ncells" )
   {
      value       = QString::number( nholes );
   }
   else if ( type == "nonEmpty" )
   {
      int nonemp  = 0;
      for ( int ii = 0; ii < nholes; ii++ )
      {
         if ( ! cc_cenps[ ii ]->currentText().contains( "empty" ) )
            nonemp++;
      }
      value       = QString::number( nonemp );
   }
   else if ( type == "alldone" )
   {
      int nonemp  = 0;
      for ( int ii = 0; ii < nholes; ii++ )
      {
         if ( ! cc_cenps[ ii ]->currentText().contains( "empty" ) )
            nonemp++;
      }
      value       = ( nonemp > 0 ) ? "1" : "0";
   }
   else if ( type == "counterbalance" )
   {
DbgLv(1) << "EGC:getSV:   CB nholes cenps-size"
 << nholes << cc_cenps.count();
      QString cbal = cc_cenps[ nholes - 1 ]->currentText();
      if ( ! cbal.contains( "empty" ) )
         value       = cbal;
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiCells::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "dbdisk" ) { value = 1; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiCells::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiCells::getLValue( const QString type )
{
   QStringList value;
DbgLv(1) << "EGC:getSL: type" << type;

   if ( type == "centerpieces" )
   {
      int nholes          = sibSValue( "rotor", "nholes" ).toInt();
DbgLv(1) << "EGC:getSL:  cc_cenps size" << cc_cenps.count();
      for ( int ii = 0; ii < nholes; ii++ )
      {
         QString celnm       = cc_labls[ ii ]->text();
         QString centp       = cc_cenps[ ii ]->currentText();
         QString windo       = cc_winds[ ii ]->currentText();
DbgLv(1) << "EGC:getSL:   ii Text" << ii << centp;
         if ( ! centp.contains( tr( "empty" ) ) )
         {
            QString centry      = celnm + " : " + centp;
            if ( ( ii + 1 ) != nholes )
            {
               centry             += "  ( " + windo + " )";
            }
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
   int nholes          = sibSValue( "rotor", "nholes" ).toInt();
   int nempty          = 0;

   for ( int ii = 0; ii < nholes; ii++ )
   {
      if ( cc_cenps[ ii ]->currentText().contains( tr( "empty" ) ) )
         nempty++;      // Keep count of "empty" centerpieces
   }

   bool is_done        = ( nempty < nholes );  // Done when not all empty
DbgLv(1) << "EGC:st: nholes nempty is_done" << nholes << nempty << is_done;
   return ( is_done ? 8 : 0 );
}

//========================= End:   Cells     section =========================


//========================= Start: Solutions section =========================
 
// Initialize a Solutions panel, especially after clicking on its tab
void US_ExperGuiSolutions::initPanel()
{
   int nsrows          = 0;  // Count of visible solution rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
DbgLv(1) << "EGSo:initP: cpnames" << cpnames;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname      = cpnames[ ii ];
      int chx             = cpname.indexOf( "-channel" );
DbgLv(1) << "EGSo:initP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell       = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan           = cpname.left( chx ).section( " ", -1, -1 )
                                 .simplified().toInt();
DbgLv(1) << "EGSo:initP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn      = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGSo:initP:      jj" << jj << "celchn" << celchn;
            slabls << celchn;
            nsrows++;
         }
      }
   }

   QString slabl;
   int nslabs          = nsrows;
   //nsrows              = qMax( nsrows, 3 );  // Show at least 3 dummy rows
DbgLv(1) << "EGSo:initP:  nslabs nsrows" << nslabs << nsrows
 << "k_labs k_sols" << cc_labls.count() << cc_solus.count();
 
   // Set cell/channel labels, make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      slabl               = ( ii < nslabs ) ? slabls[ ii ] : tr( "none" );
DbgLv(1) << "EGSo:initP:   ii" << ii << "slabl" << slabl;
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( true );
      cc_solus[ ii ]->setVisible( true );
      cc_comms[ ii ]->setVisible( true );
   }

   slabl               = tr( "none" );

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( false );
      cc_solus[ ii ]->setVisible( false );
      cc_comms[ ii ]->setVisible( false );
   }

}

// Save panel controls when about to leave the panel
void US_ExperGuiSolutions::savePanel()
{
}

// Get a specific panel value
QString US_ExperGuiSolutions::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "alldone" )
   {  // Status string
      value            = ( status() > 0 ) ? "1" : "0";
   }
   else if ( type == "nchant" )
   {  // Number channels total
      value               = QString::number( nchant );
   }
   else if ( type == "nchanu" )
   {  // Number channels unspecified
      value               = QString::number( nchanu );
   }
   else if ( type == "nchanf" )
   {  // Number channels filled with centerpiece
      value               = QString::number( nchanf );
   }
   else if ( type == "nusols" )
   {  // Number unique solutions given
      QString elabl       = tr( "none" );
      QString usolu       = tr( "(unspecified)" );
      QStringList solus;

      // Accumulate unique solutions specified
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString solu        = cc_solus[ ii ]->currentText();
         if ( ! solu.contains( usolu )  && ! solus.contains( solu ) )
            solus << solu;  // Add to list of unique solution names
      }
      // Return string:  number unique solutions specified
      value               = QString::number( solus.count() );
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiSolutions::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "dbdisk" ) { value = 1; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiSolutions::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

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
   {
      QString usolu       = tr( "(unspecified)" );

      // Accumulate unique solutions specified, with map to channels
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString labl        = cc_labls[ ii ]->text();
         QString solu        = cc_solus[ ii ]->currentText();
         QStringList sochan;

         if ( ! solu.contains( usolu ) )
         {  // Solution is specified for the channel
            if ( solus.contains( solu ) )
            {  // Previously encountered solution
               sochan              = sochans[ solu ];  // Solution channels
               sochan << labl;                         // Add to them
               sochans[ solu ]     = sochan;           // Update
               
            }
            else
            {  // Newly encounted solution
               solus  << solu;                         // Unique solution
               sochan << labl;                         // Begin solution channels
               sochans[ solu ]     = sochan;           // Update
            }
         }
      }

      // Form list of unique solutions with list of channels for each
      for ( int ii = 0; ii < solus.count(); ii++ )
      {
         QString solu        = solus[ ii ];            // Solution description
         QStringList sochan  = sochans[ solu ];        // Solution channels list
         solu               += "  ( " + sochan[ 0 ];   // Initial channel append
         // Append channels list to solution entry
         for ( int jj = 1; jj < sochan.count(); jj++ )
         {
            solu               += ", " + sochan[ jj ]; // Build channel list
         }
         solu               += "  )";                  // Close out channels string
         // Add to the output list
         value << solu;
      }
   }

   else if ( type == "channel_solutions" )
   {
      QString usolu       = tr( "(unspecified)" );

      // Accumulate list of strings in "cell/chan : solution" form
      for ( int ii = 0; ii < nchant; ii++ )
      {
         QString labl        = cc_labls[ ii ]->text();
         QString solu        = cc_solus[ ii ]->currentText();
         QString sochan      = labl + " : " + solu;

         if ( ! solu.contains( usolu ) )   // Not unspecified
            value << sochan;               // Append to the output list
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
   nchant              = 0;      // Number channels total
   nchanu              = 0;      // Number channels unspecified
   nchanf              = 0;      // Number channels filled
   QString elabl       = tr( "none" );
   QString usolu       = tr( "(unspecified)" );

   // Count rows, those unspecified, those filled
   for ( int ii = 0; ii < mxrow; ii++ )
   {
      if ( cc_labls[ ii ]->text() == elabl )
         break;        // The "none" label is end of used channels

      nchant++;        // Bump cell/channel count to fill

      if ( cc_solus[ ii ]->currentText().contains( usolu ) )
         nchanu++;     // Bump "(unspecified)" Solution count
      else
         nchanf++;     // Bump filled Solution count
   }

   bool is_done        = ( nchant > 0  &&  nchanu < 1 );
DbgLv(1) << "EGSo:st: nchant nchanu nchanf is_done"
 << nchant << nchanu << nchanf << is_done;
   return ( is_done ? 16 : 0 );
}

//========================= End:   Solutions section =========================


//========================= Start: Optical   section =========================

// Initialize an Optical panel, especially after clicking on its tab
void US_ExperGuiOptical::initPanel()
{
   int nsrows          = 0;  // Count of visible optical rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
DbgLv(1) << "EGOp:initP: cpnames" << cpnames;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname   = cpnames[ ii ];
      int chx          = cpname.indexOf( "-channel" );
DbgLv(1) << "EGOp:initP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell    = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan        = cpname.left( chx ).section( " ", -1, -1 )
                              .simplified().toInt();
DbgLv(1) << "EGOp:initP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn   = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGOp:initP:      jj" << jj << "celchn" << celchn;
            slabls << celchn;
            nsrows++;
         }
      }
      
   }

   QString notinst  = tr( "(not installed)" );
DbgLv(1) << "EGOp:initP:  nsrows" << nsrows
 << "k_labs k_sys1" << cc_labls.count() << cc_osyss.count();
 
   // Set cell/channel labels and checkboxes; make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      cc_labls[ ii ]->setText( slabls[ ii ] );
      cc_labls[ ii ]->setVisible( true );
      QCheckBox* ckbox1 = (QCheckBox*)cc_osyss[ ii ]->button( 1 );
      QCheckBox* ckbox2 = (QCheckBox*)cc_osyss[ ii ]->button( 2 );
      QCheckBox* ckbox3 = (QCheckBox*)cc_osyss[ ii ]->button( 3 );
      ckbox1->setVisible( ! ckbox1->text().contains( notinst ) );
      ckbox2->setVisible( ! ckbox2->text().contains( notinst ) );
      ckbox3->setVisible( ! ckbox3->text().contains( notinst ) );
DbgLv(1) << "EGOp:initP:   ii" << ii << "slabl" << slabls[ ii ]
 << "boxtexts" << ckbox1->text() << ckbox2->text() << ckbox3->text();
   }

   QString slabl    = tr( "none" );

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( false );
      cc_osyss[ ii ]->button( 1 )->setVisible( false );
      cc_osyss[ ii ]->button( 2 )->setVisible( false );
      cc_osyss[ ii ]->button( 3 )->setVisible( false );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiOptical::savePanel()
{
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
   if ( type == "dbdisk" ) { value = 1; }
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
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
is_done=true;
   return ( is_done ? 32 : 0 );
}

//========================= End:   Optical   section =========================


//========================= Start: Spectra   section =========================
                   
// Initialize a Spectra panel, especially after clicking on its tab
void US_ExperGuiSpectra::initPanel()
{
   int nsrows          = 0;  // Count of visible optical rows
   QStringList cpnames = sibLValue( "cells", "centerpieces" );
DbgLv(1) << "EGwS:initP: cpnames" << cpnames;
   QStringList opprofs = sibLValue( "optical", "profiles" );
DbgLv(1) << "EGwS:initP:  opprofs" << opprofs;
   QStringList slabls;

   // Build channel labels based on cells information
   for ( int ii = 0; ii < cpnames.count(); ii++ )
   {
      QString cpname   = cpnames[ ii ];
      int chx          = cpname.indexOf( "-channel" );
DbgLv(1) << "EGwS:initP:   ii" << ii << "cpname chx" << cpname << chx;
      if ( chx > 0 )
      {  // Non-empty cell centerpiece:  get channel count, build rows
         QString scell    = QString( cpname ).section( " ", 1, 1 );
         QString schans( "ABCDEF" );
         int nchan        = cpname.left( chx ).section( " ", -1, -1 )
                              .simplified().toInt();
         bool show        = true;

DbgLv(1) << "EGwS:initP:     scell" << scell << "nchan" << nchan;
         for ( int jj = 0; jj < nchan; jj++ )
         {
            QString celchn   = scell + " / " + QString( schans ).mid( jj, 1 );
DbgLv(1) << "EGwS:initP:      jj" << jj << "celchn" << celchn;
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

   QString slabl;
   int nslabs       = nsrows;
   //nsrows           = qMax( nsrows, 3 );  // Show at least 3 dummy rows
   QString unavail  = tr( "(unavailable)" );
DbgLv(1) << "EGwS:initP:  nslabs nsrows" << nslabs << nsrows
 << "k_labs" << cc_labls.count();
 
   // Set cell/channel labels and elements; make visible, all live rows
   for ( int ii = 0; ii < nsrows; ii++ )
   {
      slabl            = ( ii < nslabs ) ? slabls[ ii ] : tr( "none" );
      cc_labls[ ii ]->setText( slabl );
      cc_labls[ ii ]->setVisible( true );
      cc_wavls[ ii ]->setVisible( true );
      cc_optis[ ii ]->setVisible( true );
      cc_loads[ ii ]->setVisible( true );
      cc_manus[ ii ]->setVisible( true );
      cc_dones[ ii ]->setVisible( true );
DbgLv(1) << "EGwS:initP:   ii" << ii << "slabl" << slabl;
   }

   slabl            = tr( "none" );

   // Make remaining rows invisible
   for ( int ii = nsrows; ii < mxrow; ii++ )
   {
      cc_labls[ ii ]->setText( slabl );
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
   if ( type == "dbdisk" ) { value = 1; }
   return value;
}

// Get a specific panel double value
double US_ExperGuiSpectra::getDValue( const QString type )
{
   double value   = 0;
   if ( type == "dbdisk" ) { value = 1; }

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
//   bool is_done  = ( ! le_runid->text().isEmpty() &&
//                     ! le_project->text().isEmpty() );
bool is_done=false;
is_done=true;
   return ( is_done ? 64 : 0 );
}

//========================= End:   Spectra   section =========================

//========================= Start: Upload    section =========================

// Initialize an Upload panel, especially after clicking on its tab
void US_ExperGuiUpload::initPanel()
{
   bool chk_run      = ! sibSValue( "general",   "runID"    ).isEmpty();
   bool chk_project  = ! sibSValue( "general",   "project"  ).isEmpty();
   bool chk_rotor_ok = ( sibSValue( "rotor",     "changed"  ).toInt() > 0 );
   bool chk_speed_ok = ( sibSValue( "speeds",    "changed"  ).toInt() > 0 );
   bool chk_centerp  = ( sibSValue( "cells",     "alldone"  ).toInt() > 0 );
   bool chk_solution = ( sibSValue( "solutions", "alldone"  ).toInt() > 0 );
   bool chk_optical  = ( sibSValue( "optical",   "alldone"  ).toInt() > 0 );
   bool chk_extprofs = ( sibSValue( "photomult", "alldone"  ).toInt() > 0 );
DbgLv(1) << "EGU:iP: ck: run proj cent solu epro"
 << chk_run << chk_project << chk_centerp << chk_solution << chk_extprofs;
   bool chk_upl_enab = ( chk_run       &&  chk_project   &&
                         chk_centerp   &&  chk_solution  &&
                         chk_extprofs  &&  connected   );
   bool chk_upl_done = uploaded;

   ck_run     ->setChecked( chk_run      );
   ck_project ->setChecked( chk_project  );
   ck_rotor_ok->setChecked( chk_rotor_ok );
   ck_speed_ok->setChecked( chk_speed_ok );
   ck_centerp ->setChecked( chk_centerp  );
   ck_solution->setChecked( chk_solution );
   ck_optical ->setChecked( chk_optical  );
   ck_extprofs->setChecked( chk_extprofs );
   ck_connect ->setChecked( connected    );
   ck_upl_enab->setChecked( chk_upl_enab );
   ck_upl_done->setChecked( chk_upl_done );

   pb_upload  ->setEnabled( chk_upl_enab );
}

// Save panel controls when about to leave the panel
void US_ExperGuiUpload::savePanel()
{
}

// Get a specific panel value
QString US_ExperGuiUpload::getSValue( const QString type )
{
   QString value( "" );

   if ( type == "lab" )
   {
   //   value = cb_lab->currentText();
   }

   return value;
}

// Get a specific panel integer value
int US_ExperGuiUpload::getIValue( const QString type )
{
   int value   = 0;
   if ( type == "dbdisk" ) { value = 1; }
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

