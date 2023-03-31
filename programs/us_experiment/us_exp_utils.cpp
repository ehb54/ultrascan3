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

#include "us_experiment_gui_optima.h"
#include "us_run_protocol.h"
#include "us_rotor_gui.h"
#include "us_rotor.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
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
   else if ( child == "aprofile" ) { value = epanAProfile ->getSValue( type ); }
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
   else if ( child == "aprofile" ) { value = epanAProfile ->getIValue( type ); }
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
   else if ( child == "aprofile" ) { value = epanAProfile ->getDValue( type ); }
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
   else if ( child == "aprofile" ) { value = epanAProfile ->getLValue( type ); }
   else if ( child == "submit"   ) { value = epanUpload   ->getLValue( type ); }

   return value;
}

// Slot to handle a new panel selected
void US_ExperimentMain::newPanel( int panx )
{
DbgLv(1) << "newPanel panx=" << panx << "prev.panx=" << curr_panx << "usmode" << usmode;
   // Save any changes in the old current panel
   if      ( curr_panx == panx )  return;  // No change in panel

   if      ( curr_panx == 0 ) epanGeneral  ->savePanel();
   else if ( curr_panx == 1 ) epanRotor    ->savePanel();
   else if ( curr_panx == 2 ) epanSpeeds   ->savePanel();
   else if ( curr_panx == 3 ) epanCells    ->savePanel();
   else if ( curr_panx == 4 ) epanSolutions->savePanel();
   else if ( curr_panx == 5 ) epanOptical  ->savePanel();
   else if ( curr_panx == 6 ) epanRanges   ->savePanel();
   else if ( curr_panx == 7 )
   {
      if ( !usmode )
                              epanAProfile ->savePanel();
      else
                              epanUpload   ->savePanel();
   }
   else if ( curr_panx == 8 ) epanUpload   ->savePanel();

   // Initialize the new current panel after possible changes
   int pandiff      = panx - curr_panx;
   if      ( panx == 0 )      epanGeneral  ->initPanel();
   else if ( panx == 1 )      epanRotor    ->initPanel();
   else if ( panx == 2 )      epanSpeeds   ->initPanel();
   else if ( panx == 3 )      epanCells    ->initPanel();
   else if ( panx == 4 )                                    // Solutions
   {
      if ( pandiff > 1 )
      {
         epanCells    ->initPanel();
         epanCells    ->savePanel();
      }
      epanSolutions->initPanel();
   }
   else if ( panx == 5 )                                    // Optics
   {
      if ( pandiff > 1 )
      {
         epanCells    ->initPanel();
         epanCells    ->savePanel();
         epanSolutions->initPanel();
         epanSolutions->savePanel();
      }
      epanOptical  ->initPanel();
   }
   else if ( panx == 6 )                                    // Ranges
   {
      if ( pandiff > 1 )
      {
         epanCells    ->initPanel();
         epanCells    ->savePanel();
         epanSolutions->initPanel();
         epanSolutions->savePanel();
         epanOptical  ->initPanel();
         epanOptical  ->savePanel();
      }
      epanRanges   ->initPanel();
   }
   else if ( panx == 7 )                                    // AProfile OR Upload
   {
      if ( pandiff > 1 )
      {
         epanCells    ->initPanel();
         epanCells    ->savePanel();
         epanSolutions->initPanel();
         epanSolutions->savePanel();
         epanOptical  ->initPanel();
         epanOptical  ->savePanel();
         epanRanges   ->initPanel();
         epanRanges   ->savePanel();
      }
      if ( !usmode )                                        // Active AProfile panel => 7 AProfile
         epanAProfile ->initPanel();
      else                                                  // No active AProfile panel => 7 Upload
         epanUpload   ->initPanel();
   }
   else if ( panx == 8 )                                    // Upload OR NONE
   {
      if ( pandiff > 1 )
      {
         epanCells    ->initPanel();
         epanCells    ->savePanel();
         epanSolutions->initPanel();
         epanSolutions->savePanel();
         epanOptical  ->initPanel();
         epanOptical  ->savePanel();
         epanRanges   ->initPanel();
         epanRanges   ->savePanel();
         epanAProfile ->initPanel();
         epanAProfile ->savePanel();
      }
      epanUpload   ->initPanel();
   }

   curr_panx              = panx;         // Set new current panel

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
              + epanAProfile ->status()
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
   else if ( curr_panx == 7 ) epanAProfile ->help();
   else if ( curr_panx == 8 ) epanUpload   ->help();
}

//Slot to DISABLE tabs and Next/Prev buttons
void US_ExperimentMain::disable_tabs_buttons( void )
{
  DbgLv(1) << "DISBLING Tabs...";
  pb_next   ->setEnabled(false);
  pb_prev   ->setEnabled(false);

  for (int i=1; i<tabWidget->count(); i++)
    {
      tabWidget ->setTabEnabled( i, false );
      tabWidget ->tabBar()->setTabTextColor( i, Qt::darkGray);
    }

  qApp->processEvents();

}

//Slot to ENABLE tabs and Next/Prev buttons
void US_ExperimentMain::enable_tabs_buttons( void )
{
  DbgLv(1) << "ENABLING!!!";
  pb_next   ->setEnabled(true);
  pb_prev   ->setEnabled(true);

  for (int i=1; i<tabWidget->count(); i++)
    {
      tabWidget ->setTabEnabled( i, true );
      QPalette pal = tabWidget ->tabBar()->palette();
      tabWidget ->tabBar()->setTabTextColor( i, pal.color(QPalette::WindowText) ); // Qt::black
    }
}

//Slot to ENABLE tabs and Next/Prev buttons, but make all Widgets read-only
void US_ExperimentMain::enable_tabs_buttons_readonly( void )
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
         if ( (allPButtons[i]->text()).contains("View Solution Details" ) ||
              (allPButtons[i]->text()).contains("View Current Range Settings" ) ||
              (allPButtons[i]->text()).contains("View Experiment Details" ) ||
              (allPButtons[i]->text()).contains("Test Connection" ) )
            allPButtons[i]->setEnabled(true);
         else
            allPButtons[i]->setEnabled(false);
      }

      for ( int i = 0; i < allCBoxes.count(); i++ )
      {
	if ( (allCBoxes[i]->currentText()).contains("Speed Profile") ||
	     ( allCBoxes[i]->currentText()).contains(": Optima") )
	  allCBoxes[i]->setEnabled(true);
	else
	  allCBoxes[i]->setEnabled(false);
      }
      for ( int i = 0; i < allSBoxes.count(); i++ )
         allSBoxes[i]->setEnabled(false);
      for ( int i = 0; i < allCounters.count(); i++ )
         allCounters[i]->setEnabled(false);
      for ( int i = 0; i < allChBoxes.count(); i++ )
         allChBoxes[i]->setEnabled(false);
      // and so on ..

    }

}

//Set all tabs their all Widgets (but 8. AProfile section) READ-ONLY for US_ProtocolDev
void US_ExperimentMain::set_tabs_buttons_readonly( void )
{
  pb_next   ->setEnabled(true);
  pb_prev   ->setEnabled(true);

  for (int ii=0; ii<tabWidget->count(); ii++)
    {
      tabWidget ->setTabEnabled( ii, true );
      QPalette pal = tabWidget ->tabBar()->palette();
      //DbgLv(1) << "PALETTE: " << pal.color(QPalette::WindowText);
      tabWidget ->tabBar()->setTabTextColor( ii, pal.color(QPalette::WindowText) ); // Qt::black

      
      
      if ( ii == 7 ) //8. AProfile
	continue;
      
      QWidget* pWidget= tabWidget->widget(ii);

      //Find all children of each Tab in QTabWidget [children of all types...]
      QList<QPushButton *> allPButtons = pWidget->findChildren<QPushButton *>();
      QList<QComboBox *>   allCBoxes   = pWidget->findChildren<QComboBox *>();
      QList<QSpinBox *>    allSBoxes   = pWidget->findChildren<QSpinBox *>();
      QList<QwtCounter *>  allCounters = pWidget->findChildren<QwtCounter *>();
      QList<QCheckBox *>   allChBoxes  = pWidget->findChildren<QCheckBox *>();

      // and so on ..

      if ( ii == 0 ) //1. General
	{
	  for (int i=0; i < allPButtons.count(); i++)
	    allPButtons[i]->setEnabled(false);
	  for ( int i = 0; i < allCounters.count(); i++ )
	    allCounters[i]->setEnabled(false);
	      
	  continue;
	}
      
      for (int i=0; i < allPButtons.count(); i++)
      {
         if ( (allPButtons[i]->text()).contains("View Solution Details" ) ||
              (allPButtons[i]->text()).contains("View Ranges" ) ||
              (allPButtons[i]->text()).contains("View Experiment Details" ) ||
              (allPButtons[i]->text()).contains("Test Connection" ) )
            allPButtons[i]->setEnabled(true);
         else
            allPButtons[i]->setEnabled(false);
      }

      for ( int i = 0; i < allCBoxes.count(); i++ )
      {
	// if ( (allCBoxes[i]->currentText()).contains("Speed Profile") ||
	//      ( allCBoxes[i]->currentText()).contains(": Optima") )
	//   {
	//     allCBoxes[i]->setEnabled(true);
	//   }
	// else
	allCBoxes[i]->setEnabled(false);
      }
      for ( int i = 0; i < allSBoxes.count(); i++ )
         allSBoxes[i]->setEnabled(false);
      for ( int i = 0; i < allCounters.count(); i++ )
         allCounters[i]->setEnabled(false);
      for ( int i = 0; i < allChBoxes.count(); i++ )
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

void  US_ExperimentMain::setProtos( QStringList par1 )
{
  epanGeneral->setProtos( par1 );
}

bool US_ExperimentMain::updateProtos( const QStringList par1 )
{ return epanGeneral->updateProtos( par1 ); }

// Initialize all panels after loading a new protocol
void US_ExperimentMain::initPanels()
{
DbgLv(1) << "main: iP:";
DbgLv(1) << "main:  Ge:iP:";
   epanGeneral  ->initPanel();
DbgLv(1) << "main:  Ro:iP:";
   epanRotor    ->initPanel();
DbgLv(1) << "main:  Sp:iP:";
   epanSpeeds   ->initPanel();
DbgLv(1) << "main:  Ce:iP:";
   epanCells    ->initPanel();

DbgLv(1) << "main:  So:iP:";
   epanSolutions->initPanel();
   for ( int ii = 0; ii < epanSolutions->mxrow; ii++ )    //ALEXEY: reset channel comment while protocol re-loaded
     {
       epanSolutions->solution_comment_init[ ii ] = false;

       // QComboBox*   cb_solution =   epanSolutions->cc_solus[ ii ];
       // cb_solution->disconnect( SIGNAL( currentIndexChanged( int ) ) );

     }
   
DbgLv(1) << "main:  Op:iP:";
   epanOptical  ->initPanel();
DbgLv(1) << "main:  Rn:iP:";
   epanRanges   ->initPanel();
DbgLv(1) << "main:  Ap:iP:";
   epanAProfile ->initPanel();
DbgLv(1) << "main:  Up:iP:";
   epanUpload   ->initPanel();
DbgLv(1) << "main: iP DONE";
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
   le_protocol    ->setText ( currProto->protoname );
   le_project     ->setText ( currProto->project );
   ct_tempera     ->setValue( currProto->temperature );
   ct_tedelay     ->setValue( currProto->temeq_delay );

   le_label       ->setText ( currProto->exp_label );

   check_user_level();

}


void US_ExperGuiGeneral::check_empty_runname( const QString &/*str*/ )
{
#if 0
   if ( str.isEmpty() )
      emit set_tabs_buttons_inactive();
   else
   {
#endif
      if ( !le_project->text().isEmpty() )
         emit set_tabs_buttons_active();
#if 0
   }
#endif
}

void US_ExperGuiGeneral::check_runname()
{
#if 0
   QString rname     = le_runid->text();
   if ( rname.isEmpty() )
      emit set_tabs_buttons_inactive();
   else
   {
#endif
      if ( !le_project->text().isEmpty() )
         emit set_tabs_buttons_active();
#if 0
   }
#endif
   qApp->processEvents();
}


void US_ExperGuiGeneral::update_inv( void )
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
void US_ExperGuiGeneral::check_user_level()
{
  //update_inv();                                                       //ALEXEY: this might be needed!!!
  // MAYBE check for actual usr_level &&  US_Settings::set_us_inv_level( level ) ONLY!!! (NO inv_id && NO inv name)

  
   // Default flag:  user not enabled to change investigator
   usr_enab       = false;
   int inv_lev    = US_Settings::us_inv_level();

   qDebug() << "IN check_user_level(): inv_lev -- " << inv_lev; 

   if ( inv_lev > 2 )
   {  // All admin users and above are enabled
      usr_enab       = true;
   }
   /*
   else
   {  // Non-admin users enabled if they have instrument permit
      int inv_id     = US_Settings::us_inv_ID();
      QString pID    = QString::number( inv_id );

      if ( instr_opers.count() < 1 )
      {  // Must read lab/instrument/operator info one time
         US_Passwd   pw;
         US_DB2      db( pw.getPasswd() );

         if ( db.lastErrno() != US_DB2::OK )
         {
            QMessageBox::information( this, tr( "Error" ),
               tr( "Error making the DB connection.\n" ) );
         }

         QVector< US_Rotor::Lab > lablist;
         US_Rotor::readLabsDB( lablist, &db );

         for ( int ii = 0; ii < lablist.count(); ii++ )
         {  // Look through lab info
            QList< US_Rotor::Instrument > instruments
                           = lablist[ ii ].instruments;

            for ( int jj = 0; jj < instruments.count(); jj++ )
            {  // Look at instruments in the lab
               QString inname = instruments[ jj ].name;

               // Skip any non-Optima
               if ( ! inname.startsWith( "Optima" ) )
                  continue;

               QList< US_Rotor::Operator > operators
                              = instruments[ jj ].operators;

               for ( int kk = 0; kk < operators.count(); kk++ )
               {  // Look at operators permitted on the instrument
                  int opr_id     = operators[ kk ].ID;
                  QString olname = operators[ kk ].lname;
                  QString ofname = operators[ kk ].fname;
                  QString oID    = QString::number( opr_id );

                  // Entry is string combining operator and instrument name
                  instr_opers << oID + ": " + olname + ", " + ofname
                                 + "^" + inname;
               } // END: operators for instrument
            } // END: instruments for lab
         } // END: labs
      } // END: No list of operator^instrument entries

      // See if investigator is in permit list
      for ( int ii = 0; ii < instr_opers.count(); ii++ )
      {
         QString iID    = instr_opers[ ii ].section( ":", 0, 0 );

         if ( pID == iID )
         {  // Person ID matches an Instrument ID in the list
            usr_enab       = true;
            break;
         }
      }
   } // END: Test of non-admin instrument permit
   */

   if ( ! usr_enab )
   {  // User not enabled to set investigator
      // pb_investigator->setEnabled( false );
      // pb_project     ->setEnabled( false );
      // ct_tempera     ->setEnabled( false );
      // ct_tedelay     ->setEnabled( false );
      // le_protocol    ->setEnabled( false );

      // if ( !loaded_proto )
      //    emit set_tabs_buttons_inactive();
      // else
      //    emit set_tabs_buttons_active_readonly();

DbgLv(1) << "EGGe:ckulev: SIGNAL!!!!" ;
   }
   
   // //Old way of disabling all (read-only mode) for UL < 3
   // if ( ! usr_enab )
   // {  // User not enabled to set investigator
   //    pb_investigator->setEnabled( false );
   //    pb_project     ->setEnabled( false );
   //    ct_tempera     ->setEnabled( false );
   //    ct_tedelay     ->setEnabled( false );
   //    le_protocol    ->setEnabled( false );

   //    if ( !loaded_proto )
   //       emit set_tabs_buttons_inactive();
   //    else
   //       emit set_tabs_buttons_active_readonly();

   //    DbgLv(1) << "EGGe:ckulev: SIGNAL!!!!" ;
   // }
     
   
   // //For future DEV.
   // if ( ! usr_enab ) //UL <= 2
   //   {  // User not enabled to set investigator

   //     //Check if selected investigator is the SAME as logged in user
   //     US_Passwd pw;
   //     QString masterpw = pw.getPasswd();
   //     US_DB2* db = new US_DB2( masterpw );
      
   //     if ( db->lastErrno() != US_DB2::OK )
   // 	 {
   // 	   QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
   // 				 tr( "Could not connect to database \n" ) + db->lastError() );
   // 	   return;
   // 	 }
       
   //     QStringList qry;
   //     //Check user level && ID
   //     QStringList defaultDB = US_Settings::defaultDB();
   //     QString user_guid   = defaultDB.at( 9 );
       
   //     //get personID from personGUID
   //     qry.clear();
   //     qry << QString( "get_personID_from_GUID" ) << user_guid;
   //     db->query( qry );
       
   //     int user_id = 0;
       
   //     if ( db->next() )
   // 	 user_id = db->value( 0 ).toInt();

   //     qDebug() << "Check_user_level: US_Settings::us_inv_ID(), user_id -- "
   // 		<< US_Settings::us_inv_ID() << ", "
   // 		<< user_id;
       
   //     if ( user_id )
   // 	 {
   // 	   if ( US_Settings::us_inv_ID() != user_id )
   // 	     {
   // 	       pb_project     ->setEnabled( false );
   // 	       ct_tempera     ->setEnabled( false );
   // 	       ct_tedelay     ->setEnabled( false );
   // 	       le_protocol    ->setEnabled( false );
	       
   // 	       if ( !loaded_proto )
   // 		 emit set_tabs_buttons_inactive();
   // 	       else
   // 		 emit set_tabs_buttons_active_readonly();
   // 	     }
   // 	   else 
   // 	     {
   // 	       pb_investigator->setEnabled( false );
   // 	       emit set_tabs_buttons_active();
   // 	     }
       
   // 	   DbgLv(1) << "EGGe:ckulev: SIGNAL!!!!" ;
   // 	 }
   //   }
      
}

// Save panel controls when about to leave the panel
void US_ExperGuiGeneral::savePanel()
{
   // Populate protocol controls from GUI settings
   currProto->investigator = le_investigator->text();
   currProto->runname      = le_runid       ->text();
   currProto->protoname    = le_protocol    ->text();
   currProto->project      = le_project     ->text();
   currProto->temperature  = ct_tempera     ->value();
   currProto->temeq_delay  = ct_tedelay     ->value();

   currProto->exp_label    = le_label       ->text();

}

// Get a specific panel string value
QString US_ExperGuiGeneral::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "runID"  ||
             type == "runname" )      { value = currProto->runname; }
   else if ( type == "protocol" )     { value = currProto->protoname; }
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

   qDebug() << "GLOBAL RESET: " << mainw->global_reset;
   if (mainw->global_reset)
     mainw->global_reset = false;
   qDebug() << "GLOBAL RESET: " << mainw->global_reset;

   // Populate GUI settings from protocol controls
   bool was_changed     = changed;       // Save changed state
DbgLv(1) << "EGRo: inP: was_changed" << was_changed;

   QString cal_entr     = QString::number( rpRotor->calID ) + ": "
                          + rpRotor->calibration;
   QString rot_entr     = QString::number( rpRotor->rotID ) + ": "
                          + rpRotor->rotor;
   setCbCurrentText( cb_lab,    QString::number( rpRotor->labID ) + ": "
                                + rpRotor->laboratory );
   setCbCurrentText( cb_rotor,  rot_entr );

DbgLv(1) << "EGRo: inP: calib_entr" << cal_entr;
   if ( cb_calibr->findText( cal_entr ) < 0 )
   {  // Repopulate calibration combo box to have current calibration
      int rndx          = cb_rotor->findText( rot_entr );
      bool save_fti     = first_time_init;
      first_time_init   = true;
      changeRotor( rndx );
      first_time_init   = save_fti;
   }

   setCbCurrentText( cb_calibr,   cal_entr );

   // ALEXEY - set Optima name (with checking connection), THEN operator - ORDER IMPORTANT!!!
   QString optima_name  = QString::number( rpRotor->instID ) + ": "
                          + rpRotor->instrname;

   qDebug() << "OPTIMA NAME in initPANEL()  :" << optima_name;
   qDebug() << "OPTIMA INDEX in initPANEL() :" << cb_optima->findText( optima_name ) ;

   // optima machine
   // IF optima_name is NOT present in the list of the available instruments (from autoflow),
   // then use 1st available instr. with informimng user that Optima was changed
   if( cb_optima->findText( optima_name ) < 0 )
     {
       changeOptima( 0 );                                //<-- Do actual connection test in changeLab();
       //Message
       QMessageBox * msg_instr_avail = new QMessageBox;
       msg_instr_avail->setIcon(QMessageBox::Information);
       msg_instr_avail->setText(tr( "Loaded protocol specified <b>%1</b> as the instrument in use.<br><br>"
                                    "However, due to its current unavailability, the instrument in use will be changed to the first available intrument, <b>%2</b>." )
                                .arg(  rpRotor->instrname ).arg( mainw->currentInstrument[ "name" ] ));

       if ( !message_instr_shown )
         {
           msg_instr_avail->show();
           message_instr_shown = true;
         }
     }
   else
     changeOptima( cb_optima->findText( optima_name ) ); //<-- Do actual connection test in changeLab();

   //setCbCurrentText( cb_optima,   optima_name );    // <-- NOT ENOUGH, no connection check

   //operator
   setCbCurrentText( cb_operator, QString::number( rpRotor->operID ) + ": "
                                + rpRotor->opername );


   setCbCurrentText( cb_exptype,  rpRotor->exptype );

   changed              = was_changed;   // Restore changed state
DbgLv(1) << "EGRo: inP:  rotID" << rpRotor->rotID << "rotor" << rpRotor->rotor
 << "cb_rotor text" << cb_rotor->currentText();
DbgLv(1) << "EGRo: inP:   calID" << rpRotor->calID << "calib" << rpRotor->calibration;

}


void US_ExperGuiRotor::test_optima_connection()
{

   QString name        = mainw->currentInstrument[ "name" ];
   QString xpnhost     = mainw->currentInstrument[ "optimaHost" ];
   int     xpnport     = mainw->currentInstrument[ "optimaPort" ].toInt();
   QString dbname      = mainw->currentInstrument[ "optimaDBname" ];
   QString dbuser      = mainw->currentInstrument[ "optimaDBusername" ];
   QString dbpasw      = mainw->currentInstrument[ "optimaDBpassw" ];

qDebug() << "Optima in use: name, host, port, dbname, dbuser, dbpasw: " << name << " " << xpnhost << " "
 << xpnport << " "  << dbname << " " << dbuser << " " << dbpasw ;


   QPalette orig_pal = le_optima_connected->palette();

   if ( xpnhost.isEmpty() || QString::number(xpnport).isEmpty()
     || dbname.isEmpty() || dbuser.isEmpty() || dbpasw.isEmpty()  )
   {
      le_optima_connected->setText( "disconnected" );
      QPalette *new_palette = new QPalette();
      new_palette->setColor(QPalette::Text,Qt::red);
      new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
      le_optima_connected->setPalette(*new_palette);

      mainw->connection_status = false;
      return;
   }

   US_XpnData* xpn_data = new US_XpnData();
   bool o_connected           = xpn_data->connect_data( xpnhost, xpnport, dbname, dbuser,  dbpasw );
   xpn_data->close();
   delete xpn_data;

   if ( o_connected )
     {
       le_optima_connected->setText( "connected" );
       QPalette *new_palette = new QPalette();
       new_palette->setColor(QPalette::Text, Qt::darkGreen);
       new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
       le_optima_connected->setPalette(*new_palette);
     }
   else
     {
       le_optima_connected->setText( "disconnected" );
       QPalette *new_palette = new QPalette();
       new_palette->setColor(QPalette::Text,Qt::red);
       new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
       le_optima_connected->setPalette(*new_palette);
     }

   mainw->connection_status = o_connected;
   mainw->xpnhost = xpnhost;
   mainw->xpnport = xpnport;

   mainw->connection_for_instrument[ name ] = QString("checked");
}


// Save panel controls when about to leave the panel
void US_ExperGuiRotor::savePanel()
{
   // Populate protocol controls from GUI settings
   QString lab          = cb_lab   ->currentText();
   QString rot          = cb_rotor ->currentText();
   QString cal          = cb_calibr->currentText();
   QString oper         = cb_operator->currentText();
   QString exptype      = cb_exptype ->currentText();
   QString instr        = cb_optima ->currentText();

   rpRotor->laboratory  = QString( lab ).section( ":", 1, 1 ).simplified();
   rpRotor->rotor       = QString( rot ).section( ":", 1, 1 ).simplified();

qDebug() << "NAME OF THE ROTOR IN SAVE: rot, rpRotor->rotor: " << rot << ", "  << rpRotor->rotor;

   rpRotor->calibration = QString( cal ).section( ":", 1, 1 ).simplified();
   rpRotor->opername    = QString( oper ).section( ":", 1, 1 ).simplified();
   rpRotor->instrname   = QString( instr ).section( ":", 1, 1 ).simplified();

   rpRotor->labID       = QString( lab ).section( ":", 0, 0 ).toInt();
   rpRotor->rotID       = QString( rot ).section( ":", 0, 0 ).toInt();
   rpRotor->calID       = QString( cal ).section( ":", 0, 0 ).toInt();
   rpRotor->operID      = QString( oper ).section( ":", 0, 0 ).toInt();
   rpRotor->instID      = QString( instr ).section( ":", 0, 0 ).toInt();

   rpRotor->exptype     = exptype;

qDebug() << "OPERATORID / INSTRUMENT / ExpType in SAVE: "
         <<  rpRotor->operID  << ", " << rpRotor->opername << " / "
         <<  rpRotor->instID  << ", " << rpRotor->instrname  << " / "
         <<  rpRotor->exptype;

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
      {
DbgLv(1) << "EGRo:  svP:  calGUID was" << rpRotor->calGUID;
         rpRotor->calGUID  = calibs[ ii ].GUID;
DbgLv(1) << "EGRo:  svP:  calndx" << ii << "calGUID" << rpRotor->calGUID;
      }
   }

   qDebug() << "Rotor Save panel Done: " ;
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
DbgLv(1) << "EGRo:gIV:  arotor" << getSValue("arotor") << "value" << value;
   }
   else if ( type == "maxrpm" )
   {
      US_Rotor::AbstractRotor* arotor = abstractRotor( rpRotor->rotor );
DbgLv(1) << "EGRo:gIV:  arotor" << arotor << "rotor" << rpRotor->rotor;
      value          = ( arotor != NULL ) ? arotor->maxRPM : 50000;
DbgLv(1) << "EGRo:gIV:  arotor" << getSValue("arotor") << "value" << value;
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
   QList< int > dhms2a;
   QList< int > dhms3;
   QList< int > dhms4;
   QList< int > dhms5;


   // Populate GUI settings from protocol controls
   nspeed               = rpSpeed ->nstep;
   curssx               = qMin( (nspeed-1), qMax( 0, cb_prof->currentIndex() ) );
   double duration      = rpSpeed->ssteps[ curssx ].duration;
   double delay_stage   = rpSpeed->ssteps[ curssx ].delay_stage;
   //uv-vis
   double delay         = rpSpeed->ssteps[ curssx ].delay;
   double scanintv      = rpSpeed->ssteps[ curssx ].scanintv;
   //interference
   double delay_int     = rpSpeed->ssteps[ curssx ].delay_int;
   double scanintv_int  = rpSpeed->ssteps[ curssx ].scanintv_int;


   double speedmax      = sibDValue( "rotor", "maxrpm" );

   US_RunProtocol::timeToList( duration, dhms1 );
   US_RunProtocol::timeToList( delay,    dhms2 );
   US_RunProtocol::timeToList( delay_stage,    dhms2a );
   US_RunProtocol::timeToList( scanintv, dhms3 );
   //Interference
   US_RunProtocol::timeToList( delay_int,  dhms4 );
   US_RunProtocol::timeToList( scanintv_int, dhms5 );

   bool was_changed     = changed;       // Save changed state
   sb_count ->setValue  ( nspeed  );

   if ( mainw->automode  && !mainw->usmode )
     sb_count            ->setEnabled( false );
   
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
   sb_delay_st_dd ->setValue( (int)dhms2a[ 0 ] );
   sb_delay_st_hh ->setValue( (int)dhms2a[ 1 ] );
   sb_delay_st_mm ->setValue( (int)dhms2a[ 2 ] );
   sb_delay_st_ss ->setValue( (int)dhms2a[ 3 ] );
   sb_scnint_dd ->setValue( (int)dhms3[ 0 ] );
   sb_scnint_hh ->setValue( (int)dhms3[ 1 ] );
   sb_scnint_mm ->setValue( (int)dhms3[ 2 ] );
   sb_scnint_ss ->setValue( (int)dhms3[ 3 ] );
   //Interference
   sb_delay_int_dd ->setValue( (int)dhms4[ 0 ] );
   sb_delay_int_hh ->setValue( (int)dhms4[ 1 ] );
   sb_delay_int_mm ->setValue( (int)dhms4[ 2 ] );
   sb_delay_int_ss ->setValue( (int)dhms4[ 3 ] );
   sb_scnint_int_dd ->setValue( (int)dhms5[ 0 ] );
   sb_scnint_int_hh ->setValue( (int)dhms5[ 1 ] );
   sb_scnint_int_mm ->setValue( (int)dhms5[ 2 ] );
   sb_scnint_int_ss ->setValue( (int)dhms5[ 3 ] );

   //ALEXEY: ignore radial calibraion && spin down for now
   // ck_endoff->setChecked( rpSpeed->spin_down );
   // ck_radcal->setChecked( rpSpeed->radial_calib );

   
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
      ssvals[ ii ][ "delay_stage"    ] = rpSpeed->ssteps[ ii ].delay_stage;
      ssvals[ ii ][ "scanintv" ] = rpSpeed->ssteps[ ii ].scanintv;
      ssvals[ ii ][ "scanintv_min" ] = rpSpeed->ssteps[ ii ].scanintv_min;

      //interference
      ssvals[ ii ][ "delay_int"    ] = rpSpeed->ssteps[ ii ].delay_int;
      ssvals[ ii ][ "scanintv_int" ] = rpSpeed->ssteps[ ii ].scanintv_int;
      ssvals[ ii ][ "scanintv_int_min" ] = rpSpeed->ssteps[ ii ].scanintv_int_min;

DbgLv(1) << "EGSp:inP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"]
 << ssvals[ii]["delay_int"] << ssvals[ii]["scanintv_int"] ;

      //profdesc[ curssx ] = speedp_description( curssx );
      cb_prof->setItemText( ii, speedp_description( ii ) );

      if ( mainw->TotalWvlNum_global )
	sb_wvl_per_cell->setValue( mainw->TotalWvlNum_global );
   }
}

// Save panel controls when about to leave the panel
void US_ExperGuiSpeeds::savePanel()
{
   // Populate protocol speed controls from internal panel control
   nspeed                = ssvals.count();
   rpSpeed->nstep        = nspeed;

   //ALEXEY: ignore radial calibration && spinn down for now
   // rpSpeed->spin_down    = ck_endoff->isChecked();
   // rpSpeed->radial_calib = ck_radcal->isChecked();

   DbgLv(1) << "EGSp:svP: nspeed" << nspeed;

   rpSpeed->ssteps.resize( nspeed );  //ALEXEY BUG FIX
   for ( int ii = 0; ii < nspeed; ii++ )
   {
      rpSpeed->ssteps[ ii ].speed    = ssvals[ ii ][ "speed"    ];
      rpSpeed->ssteps[ ii ].accel    = ssvals[ ii ][ "accel"    ];
      rpSpeed->ssteps[ ii ].duration = ssvals[ ii ][ "duration" ];
      rpSpeed->ssteps[ ii ].delay    = ssvals[ ii ][ "delay"    ];
      rpSpeed->ssteps[ ii ].delay_stage    = ssvals[ ii ][ "delay_stage"    ];
      rpSpeed->ssteps[ ii ].scanintv = ssvals[ ii ][ "scanintv" ];
      rpSpeed->ssteps[ ii ].scanintv_min = ssvals[ ii ][ "scanintv_min" ];
      //interference
      rpSpeed->ssteps[ ii ].delay_int    = ssvals[ ii ][ "delay_int"    ];
      rpSpeed->ssteps[ ii ].scanintv_int = ssvals[ ii ][ "scanintv_int" ];
      rpSpeed->ssteps[ ii ].scanintv_int_min = ssvals[ ii ][ "scanintv_int_min" ];

 DbgLv(1) << "EGSp:svP:  ii" << ii << "speed accel durat delay scnint"
 << ssvals[ii]["speed"   ] << ssvals[ii]["accel"]
 << ssvals[ii]["duration"] << ssvals[ii]["delay"]
 << ssvals[ii]["delay_stage"] << ssvals[ii]["scanintv"];

 qDebug() << " DURATION SAVED IN  PROTOTCOL: speed " << ii <<  ", duration: " << rpSpeed->ssteps[ ii ].duration;
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
   else if ( type == "delay_stage" )
   {
      US_RunProtocol::timeToString( ssvals[ 0 ][ "delay_stage" ], value );
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
         double delay_stage   = rpSpeed->ssteps[ ii ].delay_stage;// In seconds
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

      //qDebug() << "cell: " << ii+1 << ", text: " << cc_cenps[ ii ]->currentText();

      if ( ( ii != icbal   &&
             cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) ) ||
           ( ii != icbal   &&
             !cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) )  )
      {  // Centerpiece when list is counterbalances: reset list

        //qDebug() << "CELL NOT LAST BUT counterbalance: text,  " << cc_cenps[ ii ]->currentText();
         cc_cenps[ ii ]->clear();
         cc_cenps[ ii ]->addItem( tr( "empty" ) );                     // ALEXEY
         cc_cenps[ ii ]->addItems( cpnames );   // Choose from centerpieces
      }

      else if ( ii == icbal   &&
         ! cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) )
      {  // Counterbalance when list is centerpieces: reset list

        //qDebug() << "CELL LAST BUT not counterbalance: text,  " << cc_cenps[ ii ]->currentText();
         cc_cenps[ ii ]->clear();
         cc_cenps[ ii ]->addItems( sl_bals );   // Choose from counterbalances

         cc_cenps[ ii ]->addItems( cpnames );   // ALEXEY: add to Choose from centerpieces  also
      }

      for ( int jj = 0; jj < nused; jj++ )
      {  // Search to see if current is among the used cells
DbgLv(1) << "EGCe:inP:     ii" << ii << "jj" << jj << "cell cellj"
 << cell << rpCells->used[jj].cell;
         if ( cell == rpCells->used[ jj ].cell )
         {  // This is a used cell:  populate a table row
           // QString cenbal      =  ( ii != icbal )                                //ALEXEY - change needed!
           //                         ? rpCells->used[ jj ].centerpiece
           //                         : rpCells->used[ jj ].cbalance;

           QString cenbal;
           if ( ii != icbal )
             {
               //qDebug() << "POPULATING INNER CELL - cbalance   !!!!! :  " << rpCells->used[ jj ].cbalance;
               //qDebug() << "POPULATING INNER CELL - centerpiece!!!!! :  " << rpCells->used[ jj ].centerpiece;
               cenbal = rpCells->used[ jj ].centerpiece;
             }
           else
             {
               //qDebug() << "POPULATING LAST CELL - cbalance   !!!!! :  " << rpCells->used[ jj ].cbalance;
               //qDebug() << "POPULATING LAST CELL - centerpiece!!!!! :  " << rpCells->used[ jj ].centerpiece;
               //if ( rpCells->used[ jj ].cbalance.contains( tr( "centerpiece" ) ) )
               if ( rpCells->used[ jj ].cbalance.isEmpty() || rpCells->used[ jj ].cbalance.contains( tr( "centerpiece" ) ) )
                 cenbal = rpCells->used[ jj ].centerpiece;
               else
                 cenbal = rpCells->used[ jj ].cbalance;
             }

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
      //cc_winds[ ii ]->setVisible( ii != icbal );                    //ALEXEY  - to change!!!
      if ( ii != icbal )
        cc_winds[ ii ]->setVisible( true );
      else
        {
          if ( ! cc_cenps[ ii ]->currentText().contains( tr( "counterbalance" ) ) )
            {
              cc_winds[ ii ]->setVisible( true );
            }
          else
            cc_winds[ ii ]->setVisible( false );
        }

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
         if ( ii != icbal )                               //ALEXEY - change needed
         {
            rpCells->used[ jj ].centerpiece = centp;
            rpCells->used[ jj ].cbalance    = "";
            rpCells->used[ jj ].windows     = cc_winds[ ii ]->currentText();
         }
         else
         {
           // ALEXEY: check if last cell is used as centerpiece
           qDebug() << "LAST CELL###: " << centp;
           if ( ! centp.contains( tr( "counterbalance" ) ) )
             {
               rpCells->used[ jj ].centerpiece = centp;
               rpCells->used[ jj ].cbalance    = "";
               rpCells->used[ jj ].windows     = cc_winds[ ii ]->currentText();
             }
           else
             {
               rpCells->used[ jj ].cbalance    = centp;
               rpCells->used[ jj ].centerpiece = "";
               rpCells->used[ jj ].windows     = "";
             }
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

         // QString centry   = ( ( ii + 1 ) != nholes )                            // ALEXEY: change needed
         //                  ? celnm + " : " + centp + "  ( " + windo + " )"
         //                  : celnm + " : " + cobal;

         QString centry;                            // ALEXEY: if last cell and not counterbalance, treat as centerpiece
         if ( ( ii + 1 ) != nholes )
         {
             centry =  celnm + " : " + centp + "  ( " + windo + " )";
         }
         else
         {
             if ( ! cobal.contains( tr( "counterbalance" )  ) )
               centry =  celnm + " : " + centp + "  ( " + windo + " )";
             else
               centry =  celnm + " : " + cobal;
         }

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

         if ( icell > icbal && rpCells->used[ ii ].cbalance.contains( tr( "counterbalance" ) ) )
           continue;                                             // Skip counterbal. ONLY if it's NOT centerpiece

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

   //ALEXEY: update solutions  if investigator change
 qDebug() << "SOLUTIONS: " << mainw->solutions_change;
  if ( mainw->solutions_change )
    {
      qDebug() << "Changing SOLUTIONS: !!!";
      regenSolList();
      mainw->solutions_change = false;
    }


  qDebug() << "SOLNAMES IN SOLINIT: " << sonames;
  rebuild_Solut();

  qDebug() << "SOLUTIONS: " << mainw->solutions_change;

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


//ALEXEY: connect changeSolu slot here:
 for ( int ii = 0; ii < mxrow; ii++ )
   {
     QComboBox*   cb_solution = cc_solus[ ii ];
     connect( cb_solution,  SIGNAL( currentIndexChanged( int ) ),
	      this,         SLOT  ( changeSolu         ( int ) ) );
      
   }
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
         //ALEXEY - to remember changes to Soluton comments if manual commnets was added while returning to "Solutons" tab

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

//ALEXEY: disconnect changeSolu slot here:
 for ( int ii = 0; ii < mxrow; ii++ )
   {
     QComboBox*   cb_solution = cc_solus[ ii ];
     cb_solution->disconnect();
      
   }
 
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
   nochan              = rpOptic->chopts.count();
DbgLv(1) << "EGOp:inP:  call rbO";

   rebuild_Optic();

   QString notinst     = tr( "(not installed)" );
   QString slabl_n( "none" );
DbgLv(1) << "EGOp:inP: nochan" << nochan;

   //ALEXEY: if last cell is used as centerpiece, disable Rayleigh Interference (ckbox2/ckbox3)
   int nholes                 = sibIValue( "rotor",   "nholes" );
   bool ctrbal_is_centerpiece = false;
   for ( int ii = 0; ii < nochan; ii++ )
     {
       QString channel     = rpOptic->chopts[ ii ].channel;

       int cell_number = ((channel.split(QRegExp("\\s+"), QString::SkipEmptyParts))[0]).toInt();
DbgLv(1) << "EGOp:inP: CELL #" << cell_number;
       if ( nholes == cell_number )
         ctrbal_is_centerpiece = true;
     }


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

      //ALEXEY: set checkboxes checked based on the protocol loaded, rather than by default
      if ( !scan1.isEmpty() )
      	ckbox1->setChecked( true);
      else
      	ckbox1->setChecked( false );
      if ( !scan2.isEmpty() )
      	ckbox2->setChecked( true  );
      else
      	ckbox2->setChecked( false );
      if ( !scan3.isEmpty()  &&  !ckscan3.contains( notinst ) )
      	ckbox3->setChecked( true  );
      else
      	ckbox3->setChecked( false );
      
      //ckbox1->setChecked( prscans.contains( ckscan1 ) );
      //ckbox2->setChecked( prscans.contains( ckscan2 ) );   //ALEXEY do not check Interference by default

      //ALEXEY: if last cell is used as centerpiece, disable Rayleigh Interference (ckbox2|ckbox3]
      if ( ctrbal_is_centerpiece )
      {
        if ( ! ckscan3.contains( notinst ) )
           ckbox2->setEnabled( false );
        else
           ckbox3->setEnabled( false );
      }

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
DbgLv(1) << "EGOp:svP:  nochan" << nochan;

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
         rpOptic->chopts[ ii ].scan1 = ckbox1->text();
         kchkd++;
      }

      if ( ckbox2->isChecked() )
      {
         rpOptic->chopts[ ii ].scan2 = ckbox2->text();
         kchkd++;
      }

      if ( ckbox3->isChecked() )
      {
         rpOptic->chopts[ ii ].scan3 = ckbox3->text();
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
DbgLv(1) << "EGOp:st:  ii" << ii << "ch s1 s2 s3"
 << channel << scan1 << scan2 << scan3;
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
DbgLv(1) << "EGOp:st:    hvu hvv nuch nuvv" << have_used << have_uvvis
 << nuchan << nuvvis;
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

DbgLv(1) << "EGRn:inP:  call rbS";
   rebuild_Ranges();

   QString ch_none( "none" );
   DbgLv(1) << "EGRn:inP:  nrnchan" << nrnchan;
   


   for ( int ii = 0; ii < nrnchan; ii++ )
   {
      QString channel     = rchans[ ii ];
      QString labwlr;
      int kswavl          = swvlens[ ii ].count();
DbgLv(1) << "EGRn:inP:    ii" << ii << "channel" << channel;
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

      cc_lrads[ ii ]->setValue( locrads[ ii ] );
      cc_hrads[ ii ]->setValue( hicrads[ ii ] );

      cc_labls[ ii ]->setVisible( true );
      cc_lrngs[ ii ]->setVisible( true );
      cc_lbtos[ ii ]->setVisible( true );

      cc_wavls[ ii ]->setVisible( true );
      cc_lrads[ ii ]->setVisible( true );
      cc_hrads[ ii ]->setVisible( true );

       if ( channel.contains(tr("reference")) )   //ALEXEY do not allow to set wavelengths/radial ranges for B channels (reference)
               {
          qDebug() << "Channel: " << channel;
                 cc_wavls[ ii ]->setEnabled( false );
                 cc_lrads[ ii ]->setEnabled( false );
                 cc_hrads[ ii ]->setEnabled( false );
               }
       else
         {
	   if ( US_Settings::us_inv_level() > 2 ) //ALEXEY: if user level 3 or more
	     {
	       cc_wavls[ ii ]->setEnabled( true );
	       cc_lrads[ ii ]->setEnabled( true );
	       cc_hrads[ ii ]->setEnabled( true );
	     }
	 }
   }

   //////////////////////////////////////////////////////////////
      //ALEXEY
   // get a list of same-cell rows; disconnect
   for ( int i = 0; i < nrnchan; i++ )
   {
     QString clabl       = cc_labls[ i ]->text();

     if ( !clabl.split(",")[0].contains("/ A") )
       continue;
       
     QString scell       = clabl.left( 1 );
     QString labnone     = tr( "none" );
     QList< int >  ccrows;
     int kswavl          = swvlens[ i ].count();
     QString labwlr      = cc_lrngs[ i ]->text();
     
     for ( int ii = 0; ii < nrnchan; ii++ )
       {
	 // Ignore the exact same row
	 if ( ii == i )
	   continue;
	 // Get row label and quit loop when at end visible rows
	 QString rlabl       = cc_labls[ ii ]->text();
	 if ( rlabl == labnone )
	   break;
	 // Compare the cell value to that of the one (un)checked
	 QString rcell       = rlabl.left( 1 );
	 if ( rcell == scell )
	   {  // Save same-cell row and disconnect the checkbox
	     ccrows << ii;
	   }
       }
     
     // Set check-state of  boxes in same-cell rows and reconnect : ALEXEY: here set of channel A is copied to B
     for ( int j = 0; j < ccrows.count(); j++ )
       {
	 int ccrow           = ccrows[ j ];
	 
	 qDebug() << "::initPanel(), Ranges: same-cell rows -- " << clabl << cc_labls[ ccrow ]->text();
	 
	 cc_lrngs[ ccrow ]->setText( labwlr );
	 
	 swvlens[ ccrow ].clear();
	 for ( int jj = 0; jj < kswavl; jj++ )
	   swvlens[ ccrow ] << swvlens[ i ][ jj ];
       }
   }
   ///////////////////////////////////////////////////////////////

   
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


   // Fill ScanCount info: # scans per stage/per wavelength
   int nsp = sibIValue( "speeds",  "nspeeds" );
   int ncells  = sibIValue( "rotor",   "nholes" );
DbgLv(1) << "EGRn:inP: # speeds, #cells: " << nsp << ", " << ncells;

   QVector < int > Total_wvl(nsp);
   QVector < int > ncells_used(nsp);

   for (int i=0; i<nsp; i++)
     {
       Total_wvl[i] = 0;
       ncells_used[i] = 0;
     }
   
DbgLv(1) << "EGRn:inP: Total_wvl.size():  " << Total_wvl.size();

   for (int i=0; i<nsp; i++)
   {
DbgLv(1) << "EGRn:inP: Speed # " << i;

      for (int j=0; j<ncells; j++)
      {
DbgLv(1) << "EGRn:inP:  Cell # " << j;
         //Compute total # wvl per stage
         QString channel;

DbgLv(1) << "EGRn:inP:  rpRange->nranges: " << rpRange->nranges;
         for ( int ii = 0; ii < rpRange->nranges; ii++ )
         {
            channel  = rpRange->chrngs[ ii ].channel;
            if ( channel.contains("sample") && channel.startsWith(QString::number(j+1)) )  // <-- Judge only by sample (channel A) for now
            {
DbgLv(1) << "EGRn:inP:   # of Ranges for cell " << j << ": " << rpRange->chrngs[ ii ].wvlens.count();
               Total_wvl[i]  += rpRange->chrngs[ ii ].wvlens.count();

	       ++ncells_used[i];
            }
         }
DbgLv(1) << "EGRn:inP:  #Wvl for cell: " << j << " is: " << Total_wvl[i];
      }
   }

   cb_scancount    ->clear();
   cb_scancount_int->clear();
   
   for ( int i = 0; i < nsp; i++ )
   {
      double duration_sec = rpSpeed->ssteps[ i ].duration;
      double scanint_sec  = rpSpeed->ssteps[ i ].scanintv;
      double scanint_sec_min;
      std::modf (rpSpeed->ssteps[ i ].scanintv_min, &scanint_sec_min);

      qDebug() << "RANGES INIT: duration_sec , scanint_sec, scanint_sec_min,  Total_wvl[i], ncells_used[i] -- "
	       << duration_sec << scanint_sec << scanint_sec_min << Total_wvl[i] << ncells_used[i];

      int scancount;
      int scaninterval = 0;
      bool scaninterval_updated = false;

      //ALEXEY: check if there is Absorbance
      QStringList oprof_a   = sibLValue( "optical", "profiles" );
      QString uvvis       = tr( "UV/visible" );
            
      bool has_absorbance = false;
      for ( int ii = 0; ii < oprof_a.count(); ii++ )
	{
	  if ( oprof_a[ ii ].contains( uvvis ) )
	    {
	      has_absorbance = true;
	      break;
	    }
	}
      
      if ( has_absorbance )
	{
	  
	  if ( Total_wvl[i] == 0 )
	    scancount = 0;
	  else
	    { 
	      //ALEXEY: use this algorithms:  
	      if ( scanint_sec > scanint_sec_min * Total_wvl[i])
		{
		  scancount     = int( duration_sec / scanint_sec );
		  scaninterval  = scanint_sec;
		}
	      else
		{
		  scancount    = int( duration_sec / ( scanint_sec_min * Total_wvl[i] ) );
		  scaninterval = int( scanint_sec_min * Total_wvl[i] );
		  scaninterval_updated = true; //updated: show in RED
		}
	      
	    }
	  
	  
	  //Increase scan interval if scancount >= 1500:
	  if( scancount >= 1500 )
	    {
	      scaninterval = int( duration_sec / 1500 );
	      scancount    = 1500;
	      scaninterval_updated = true; //updated: show in RED
	    }
	  
	  rpSpeed->ssteps[ i ].scancount = scancount;
	  mainw->ScanCount_global    = scancount;
	  mainw->currProto.scanCount = scancount;
	  mainw->TotalWvlNum_global  = Total_wvl[i];

      
	  DbgLv(1) << "EGRn:inP:  speed" << i << "scancount" << scancount;
	  
	  //Update le_scanint text: set text color RED if updated
	  QList< int > hms_scanint;
	  double scaninterval_d = scaninterval;
	  US_RunProtocol::timeToList( scaninterval_d, hms_scanint );
	  QString scint_str = QString::number( hms_scanint[ 1 ] ) + "h " + QString::number( hms_scanint[ 2 ] ) + "m " + QString::number( hms_scanint[ 3 ] ) + "s";
	  le_scanint->setText( scint_str );
	  QPalette *palette = new QPalette();
	  if ( scaninterval_updated )
	    {
	      palette->setColor(QPalette::Text,Qt::red);
	      //palette->setColor(QPalette::Base,Qt::white);
	      le_scanint->setPalette(*palette);
	      
	      rpSpeed->ssteps[ i ].scanintv = scaninterval;
	    }
	  else
	    {
	      palette->setColor(QPalette::Text,Qt::black);
	      //palette->setColor(QPalette::Base,Qt::white);
	      le_scanint->setPalette(*palette);
	    }

	  //Update cb_scancount string:
	  QString scancount_stage = tr( "Stage %1. Number of Scans per Triple (UV/vis): %2 " ).arg(i+1).arg(scancount);
	  cb_scancount->addItem( scancount_stage );
	}
      else //hide uv/vis related widgets
	{
	  // le_scanint   -> hide();
	  // cb_scancount -> hide();

	  QPalette *palette_1 = new QPalette();
	  palette_1->setColor(QPalette::Text,Qt::black);
	  le_scanint -> setText("N/A");
	  le_scanint ->setPalette(*palette_1);
	  //Update cb_scancount string:
	  QString scancount_stage = tr( "Stage %1. Number of Scans per Triple (UV/vis): N/A " ).arg(i+1);
	  cb_scancount->addItem( scancount_stage );
	}
      
      

      //ALEXEY: add interference info:
      double scanint_sec_int  = rpSpeed->ssteps[ i ].scanintv_int;
      int scancount_int = 0;
      int scaninterval_int;
      bool scaninterval_int_updated = false;
      
      //ALEXEY: check if there is interference
      QStringList oprof   = sibLValue( "optical", "profiles" );
      //QString uvvis       = tr( "UV/visible" );
      QString rayleigh    = tr( "Rayleigh Interference" );
      
      bool has_interference = false;
      int ncells_used_int = 0;
      for ( int ii = 0; ii < oprof.count(); ii++ )
	{
	  if ( oprof[ ii ].contains( rayleigh ) )
	    {
	      ++ncells_used_int;
	      has_interference = true;
	      //break;
	    }
	}
      if ( has_interference )
	{

	  ncells_used_int /= 2;
	  
	  //ALEXEY: use this algorithm for Interference: scanint_min=5; 
	  if ( scanint_sec_int > 5 * ncells_used_int )
	    {
	      scancount_int     = int( duration_sec / scanint_sec_int );
	      scaninterval_int  = scanint_sec_int;
	    }
	  else
	    {
	      scancount_int            = int( duration_sec / (5 * ncells_used_int ) );
	      scaninterval_int         = int( 5 * ncells_used_int );
	      scaninterval_int_updated = true; //updated: show in RED
	    }
	  
	  //Increase scan interval if scancount >= 1500:
	  if( scancount_int >= 1500 )
	    {
	      scaninterval_int         = int( duration_sec / 1500 );
	      scancount_int            = 1500;
	      scaninterval_int_updated = true; //updated: show in RED
	    }

	  qDebug() << "RANGES INIT Interference: duration_sec , scanint_sec_int, ncells_used -- "
		   << duration_sec << scanint_sec_int << ncells_used_int;

	  rpSpeed->ssteps[ i ].scancount_int = scancount_int;
	  mainw->ScanCount_global_int        = scancount_int;
	  mainw->currProto.scanCount_int     = scancount_int;
	  
	  //Update le_scanint text: set text color RED if updated
	  QList< int > hms_scanint_int;
	  double scaninterval_d_int = scaninterval_int;
	  US_RunProtocol::timeToList( scaninterval_d_int, hms_scanint_int );
	  QString scint_str_int = QString::number( hms_scanint_int[ 1 ] ) + "h " + QString::number( hms_scanint_int[ 2 ] ) + "m " + QString::number( hms_scanint_int[ 3 ] ) + "s";
	  le_scanint_int->setText( scint_str_int );
	  
	  qDebug() << "ScanInt_interference: " << scint_str_int; 
	  
	  
	  QPalette *palette_int = new QPalette();
	  if ( scaninterval_int_updated )
	    {
	      palette_int->setColor(QPalette::Text,Qt::red);
	      //palette->setColor(QPalette::Base,Qt::white);
	      le_scanint_int->setPalette(*palette_int);
	      
	      rpSpeed->ssteps[ i ].scanintv_int = scaninterval_int;
	    }
	  else
	    {
	      palette_int->setColor(QPalette::Text,Qt::black);
	      //palette->setColor(QPalette::Base,Qt::white);
	      le_scanint_int->setPalette(*palette_int);
	    }

	  QString scancount_stage_int = tr( "Stage %1. Number of Scans per Cell (Interference): %2 " ).arg(i+1).arg(scancount_int);
	  cb_scancount_int->addItem( scancount_stage_int );  
	  
	}
      else //hide interference related widgets
	{
	  // le_scanint_int   -> hide();
	  // cb_scancount_int -> hide();
	  
	  QPalette *palette_int1 = new QPalette();
	  palette_int1->setColor(QPalette::Text,Qt::black);
	  le_scanint_int -> setText("N/A");
	  le_scanint_int->setPalette(*palette_int1);
	  
	  QString scancount_stage_int = tr( "Stage %1. Number of Scans per Cell (Interference): N/A " ).arg(i+1);
	  cb_scancount_int->addItem( scancount_stage_int );   
	}
      
   }
   // End of ScanCount listbox

   //If for US_ProtocolDev mode, set all widgets in read-only mode:
   if ( mainw->us_prot_dev_mode )
     {
       for ( int ii = 0; ii < nrnchan; ii++ )
	 {
	   cc_wavls[ ii ]->setEnabled( false );
	   cc_lrads[ ii ]->setEnabled( false );
	   cc_hrads[ ii ]->setEnabled( false );
	 }
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

//========================= Start: Aprofile  section =========================

// Initialize an Aprofile panel, especially after clicking on its tab
void US_ExperGuiAProfile::initPanel()
{
DbgLv(1) << "EGAp:inP: IN";
   currProto       = &mainw->currProto;
   loadProto       = &mainw->loadProto;
   rpRotor         = &currProto->rpRotor;
   rpSpeed         = &currProto->rpSpeed;
   rpCells         = &currProto->rpCells;
   rpSolut         = &currProto->rpSolut;
   rpOptic         = &currProto->rpOptic;
   rpRange         = &currProto->rpRange;
   rpAprof         = &currProto->rpAprof;
DbgLv(1) << "EGAp:inP: aa  rpAprof" << rpAprof;
   QString protoname  = currProto->protoname;
DbgLv(1) << "EGAp:inP: bb" << protoname;
   QString aprofname  = rpAprof->aprofname;
DbgLv(1) << "EGAp:inP: protoname" << protoname << "aprofname" << aprofname;
   if ( protoname.isEmpty() )
      protoname       = mainw->loadProto.protoname;
   if ( aprofname.isEmpty() )
      aprofname       = protoname;
DbgLv(1) << "EGAp:inP:  protoname" << protoname << "aprofname" << aprofname;
DbgLv(1) << "EGAp:inP:  sdiag" << sdiag;
   sdiag->auto_name_passed( protoname, aprofname );


   //sdiag->reset();

   sdiag->currProf.aprofname  = aprofname;
   sdiag->currProf.protoname  = protoname;
   sdiag->currProf.protoGUID  = mainw->currProto.protoGUID;
   sdiag->currProf.protoID    = mainw->currProto.protoID;
DbgLv(1) << "EGAp:inP:  sdiag inherit" << "proto GUID ID"
 << mainw->currProto.protoGUID << mainw->currProto.protoID;
   sdiag->inherit_protocol( currProto );
DbgLv(1) << "EGAp:inP:  sdiag initPanels()";
   mainw->currAProf           = sdiag->currProf;
   sdiag->initPanels();
   mainw->currAProf           = sdiag->currProf;


   qDebug() << "EXP_APROFILE_GUI -- initPanel(): sdiag->currProf.aprofname -- " << sdiag->currProf.aprofname;
}

void US_ExperGuiAProfile::reset_sdiag( void )
{
   sdiag->reset();

   qDebug() << "EXP_APROFILE_GUI RESET-- reset_sdiag(): sdiag->currProf.aprofname -- " << sdiag->currProf.aprofname;

}

void US_ExperGuiAProfile::savePanel()
{
DbgLv(1) << "EGAp:svP:  sdiag savePanels()";
   sdiag->savePanels();
   mainw->currAProf = sdiag->currProf;
}

int  US_ExperGuiAProfile::status()
{
bool is_done=true;
   return ( is_done ? 128 : 0 );
}

QString US_ExperGuiAProfile::getSValue( const QString type )
{
   QString value( "" );

   if      ( type == "alldone"  ||
             type == "status" )
      value  = QString::number( getIValue( type ) );

   return value;
}

// Get a specific panel integer value
int US_ExperGuiAProfile::getIValue( const QString type )
{
   int value   = 0;
   if      ( type == "alldone" )  { value = ( status() > 0 ) ? 1 : 0; }
   else if ( type == "status"  )  { value = status(); }
   return value;
}

// Get a specific panel double value
double US_ExperGuiAProfile::getDValue( const QString type )
{
   double value   = 0.;
   if ( type == "dbdisk" ) { value = 1; }

   return value;
}

// Get specific panel list values
QStringList US_ExperGuiAProfile::getLValue( const QString type )
{
   QStringList value( "" );

   if ( type == "uploaded" )
   {
      //value << le_runid->text();
   }

   return value;
}

//========================= End:   Aprofile  section =========================

//========================= Start: Upload    section =========================

// Initialize an Upload panel, especially after clicking on its tab
void US_ExperGuiUpload::initPanel()
{
   currProto       = &mainw->currProto;
   loadProto       = &mainw->loadProto;
   //rps_differ      = ( mainw->currProto !=  mainw->loadProto );
   rpRotor         = &currProto->rpRotor;
   rpSpeed         = &currProto->rpSpeed;
   rpCells         = &currProto->rpCells;
   rpSolut         = &currProto->rpSolut;
   rpOptic         = &currProto->rpOptic;
   rpRange         = &currProto->rpRange;
   rpAprof         = &currProto->rpAprof;
   rpSubmt         = &currProto->rpSubmt;

   //Also, compare US_AnaProfile internals for current & loaded AProfiles
   US_AnaProfile aprof_curr   = *(mainw->get_aprofile());
   US_AnaProfile aprof_loaded = *(mainw->get_aprofile_loaded());
   
   rps_differ = false;
   if ( mainw->currProto !=  mainw->loadProto )
     rps_differ = true;

   if ( aprof_curr       !=  aprof_loaded )
     rps_differ = true;

   //finally, check for differences in currAProf's & loadAProf's nested  QMap< QString, QMap < QString, US_ReportGMP > > ch_reports
   // {it's easier to perform this check separately}
   if ( areReportMapsDifferent( aprof_curr, aprof_loaded ) )
     rps_differ = true;
   

   qDebug() << "rpSPEED: duration: " << rpSpeed->ssteps[0].duration;
   
   if(rps_differ)
     {
       US_RunProtocol* cRP = currProto;
       US_RunProtocol* lRP = loadProto;
       US_AnaProfile aprof_c   = *(mainw->get_aprofile());
       US_AnaProfile aprof_l   = *(mainw->get_aprofile_loaded());
       DbgLv(1) << "EGUp:inP: RPs DIFFER";

       DbgLv(1) << "EGUp:inP:   AProfile diff" << ( aprof_c != aprof_l );
       
       DbgLv(1) << "EGUp:inP:  cPname" << cRP->protoname << "lPname" << lRP->protoname;
       DbgLv(1) << "EGUp:inP:  cInves" << cRP->investigator << "lInves" << lRP->investigator;
       DbgLv(1) << "EGUp:inP:  cPguid" << cRP->protoGUID << "lPguid" << lRP->protoGUID;
       DbgLv(1) << "EGUp:inP:  cOhost" << cRP->optimahost << "lOhost" << lRP->optimahost;
       DbgLv(1) << "EGUp:inP:  cTempe" << cRP->temperature << "lTempe" << lRP->temperature;
       DbgLv(1) << "EGUp:inP:   rpRotor diff" << (cRP->rpRotor!=lRP->rpRotor);
       DbgLv(1) << "EGUp:inP:   rpSpeed diff" << (cRP->rpSpeed!=lRP->rpSpeed);
       DbgLv(1) << "EGUp:inP:   rpCells diff" << (cRP->rpCells!=lRP->rpCells);
       DbgLv(1) << "EGUp:inP:   rpSolut diff" << (cRP->rpSolut!=lRP->rpSolut);
       DbgLv(1) << "EGUp:inP:   rpOptic diff" << (cRP->rpOptic!=lRP->rpOptic);
       DbgLv(1) << "EGUp:inP:   rpRange diff" << (cRP->rpRange!=lRP->rpRange);
       DbgLv(1) << "EGUp:inP:   rpAprof diff" << (cRP->rpAprof!=lRP->rpAprof);
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
   // subm_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
   //                       connected );

   if ( mainw->automode )
     subm_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
                           mainw->connection_status &&                // ALEXEY: use top-level connection boolean!
                           !currProto->exp_label.isEmpty() );         // ALEXEY: and label is present
   else
     subm_enab         = ( have_run    &&  have_proj  &&  proto_ena  &&
                           mainw->connection_status );               // ALEXEY: use top-level connection boolean!


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
   pb_saverp  ->setEnabled( have_cells && have_solus && have_range && rps_differ );      // ALEXEY: add check here is rps_differ == true (protocols differ)

   // Show/hide Submit and Save buttons based on RunId given
   if ( !mainw-> us_prot_dev_mode )
     {
       if ( US_Settings::us_inv_level() > 2 )
	 {  // Can show/hide buttons as admin
	   DbgLv(1) << "EGUp:inP: have_run" << have_run;
	   if ( have_run )
	     {  // RunId given, so show submit
	       DbgLv(1) << "EGUp:inP: have_run";
	       pb_submit->show();
	       pb_saverp->hide();
	     }
	   else
	     {  // RunID not given, so show save
	       DbgLv(1) << "EGUp:inP: NOT have_run";
	       pb_submit->hide();
	       pb_saverp->show();
	     }
	 }
     }
   else
     {
       pb_submit -> disconnect();
       connect( pb_submit,    SIGNAL( clicked()          ),
		this,         SLOT  ( submitExperiment_confirm_protDev() ) );
       
       pb_submit->show();
       pb_saverp->hide();

       pb_submit  ->setEnabled( false );
       pb_submit  ->setEnabled( have_run && rps_differ );
     }

   //DEBUG
   //Opt system check, what cells will be uvvis and/or interference
   QStringList oprof   = sibLValue( "optical", "profiles" );
   QString uvvis       = tr( "UV/visible" );
   QString rayleigh    = tr( "Rayleigh Interference" );
   
   //get # cells with interference channels
   int ncells_interference = 0;
   int nchannels_uvvis = 0;
   QStringList active_channels;
   for ( int kk = 0; kk < oprof.count(); kk++ )
     {
       if ( oprof[ kk ].contains( rayleigh ) )
	 {
	   if  ( oprof[ kk ].section( ":", 0, 0 ).contains("sample") )
	     {
	       qDebug() << "ITF channel name: " <<  oprof[ kk ].section( ":", 0, 0 ).split(",")[0];
	       active_channels << oprof[ kk ].section( ":", 0, 0 ).split(",")[0];
	     }
	   
	   ++ncells_interference;
	 }
       
       if ( oprof[ kk ].contains( uvvis ) )
	 ++nchannels_uvvis;
     }

   qDebug() << "Upload::initPanel(): oprof -- " << oprof;
   qDebug() << "Upload::initPanel(): ncells_interference, nchannels_uvvis -- "
	    << ncells_interference << ", " << nchannels_uvvis;
   
}

bool US_ExperGuiUpload::areReportMapsDifferent( US_AnaProfile aprof_curr, US_AnaProfile aprof_load )
{
  bool maps_different = false;

  QMap< QString, QMap < QString, US_ReportGMP > > aprof_curr_ch_reports = aprof_curr.ch_reports;
  QMap< QString, QMap < QString, US_ReportGMP > > aprof_load_ch_reports = aprof_load.ch_reports;
  
  //check number & content of keys:
  QStringList aprof_curr_keys   = aprof_curr_ch_reports.  keys();
  QStringList aprof_load_keys   = aprof_load_ch_reports.  keys();

  qDebug() << "Top-level Report Maps Keys: curr, loaded -- "
	   << aprof_curr_keys
	   << aprof_load_keys ;
    
  if ( aprof_curr_keys.size() != aprof_load_keys.size() )
    return true;
    
  if ( aprof_curr_keys != aprof_load_keys ) 
    return true;
  
  //Now iterate over channels
  for ( int i=0; i<aprof_curr_keys.size(); i++ )
    {	
      QMap < QString, US_ReportGMP > triple_reports_curr = aprof_curr_ch_reports[ aprof_curr_keys[i] ];
      QMap < QString, US_ReportGMP > triple_reports_load = aprof_load_ch_reports[ aprof_curr_keys[i] ];

      //check number & content of keys:
      QStringList triple_reports_curr_keys   = triple_reports_curr.  keys();
      QStringList triple_reports_load_keys   = triple_reports_load.  keys();
      
      qDebug() << "Triple's Report Maps Keys: curr, loaded -- "
	       << triple_reports_curr_keys
	       << triple_reports_load_keys ;
      
      if ( triple_reports_curr_keys.size() != triple_reports_load_keys.size() )
	{
	  maps_different = true;
	  break;
	}
      if ( triple_reports_curr_keys != triple_reports_load_keys ) 
	{
	  maps_different = true;
	  break;
	}
      
      //now, iterate over triple's report & reportItems
      for ( int j=0; j < triple_reports_curr_keys.size(); j++ )
	{
	  US_ReportGMP report_curr = triple_reports_curr[ triple_reports_curr_keys[j] ];
	  US_ReportGMP report_load = triple_reports_load[ triple_reports_curr_keys[j] ];

	  //compare general report parameters
	  if ( report_curr.tot_conc                != report_load.tot_conc     ||
	       report_curr.tot_conc_tol            != report_load.tot_conc_tol ||
	       report_curr.rmsd_limit              != report_load.rmsd_limit   ||
	       report_curr.av_intensity            != report_load.av_intensity ||
	       report_curr.experiment_duration     != report_load.experiment_duration ||
	       report_curr.experiment_duration_tol != report_load.experiment_duration_tol
	       )
	    {
	      maps_different = true;
	      break;
	    }
	  
	  //compare report masks
	  if ( report_curr.tot_conc_mask            != report_load.tot_conc_mask     ||
	       report_curr.rmsd_limit_mask          != report_load.rmsd_limit_mask   ||
	       report_curr.av_intensity_mask        != report_load.av_intensity_mask ||
	       report_curr.experiment_duration_mask != report_load.experiment_duration_mask ||
	       report_curr.integration_results_mask != report_load.integration_results_mask ||
	       report_curr.plots_mask               != report_load.plots_mask 
	       )
	    {
	      maps_different = true;
	      break;
	    }
	  
	  //compare Pseudo3D masks
	  if ( report_curr.pseudo3d_mask           !=   report_load.pseudo3d_mask           ||    
	       report_curr.pseudo3d_2dsait_s_ff0   !=	report_load.pseudo3d_2dsait_s_ff0   ||
	       report_curr.pseudo3d_2dsait_s_d     !=   report_load.pseudo3d_2dsait_s_d     ||
	       report_curr.pseudo3d_2dsait_mw_ff0  != 	report_load.pseudo3d_2dsait_mw_ff0  ||
	       report_curr.pseudo3d_2dsait_mw_d	   != 	report_load.pseudo3d_2dsait_mw_d    ||	  
	       report_curr.pseudo3d_2dsamc_s_ff0   != 	report_load.pseudo3d_2dsamc_s_ff0   ||
	       report_curr.pseudo3d_2dsamc_s_d	   !=   report_load.pseudo3d_2dsamc_s_d	    ||
	       report_curr.pseudo3d_2dsamc_mw_ff0  != 	report_load.pseudo3d_2dsamc_mw_ff0  ||
	       report_curr.pseudo3d_2dsamc_mw_d    != 	report_load.pseudo3d_2dsamc_mw_d    ||
	       report_curr.pseudo3d_pcsa_s_ff0	   != 	report_load.pseudo3d_pcsa_s_ff0	    ||
	       report_curr.pseudo3d_pcsa_s_d	   != 	report_load.pseudo3d_pcsa_s_d	    ||
	       report_curr.pseudo3d_pcsa_mw_ff0	   != 	report_load.pseudo3d_pcsa_mw_ff0    ||	  
	       report_curr.pseudo3d_pcsa_mw_d      !=   report_load.pseudo3d_pcsa_mw_d      
	       )
	    {
	      maps_different = true;
	      break;
	    }
	  
	  //compare reportItems: first, check # of reportItems in curr, load
	  int reportItems_curr_size = report_curr.reportItems.size();
	  int reportItems_load_size = report_load.reportItems.size();
	  if ( reportItems_curr_size != reportItems_load_size )
	    {
	      maps_different = true;
	      break;
	    }
	  
	  for ( int k = 0; k < report_curr.reportItems.size(); k++ )
	    {
	      US_ReportGMP::ReportItem curr_reportItem = report_curr.reportItems[ k ];
	      US_ReportGMP::ReportItem load_reportItem = report_load.reportItems[ k ];

	      if ( curr_reportItem.type             != load_reportItem.type             ||
		   curr_reportItem.method           != load_reportItem.method           ||
		   curr_reportItem.range_low        != load_reportItem.range_low        ||
		   curr_reportItem.range_high       != load_reportItem.range_high       ||
		   curr_reportItem.integration_val  != load_reportItem.integration_val  ||
		   curr_reportItem.tolerance        != load_reportItem.tolerance        ||
		   curr_reportItem.total_percent    != load_reportItem.total_percent    ||
		   curr_reportItem.combined_plot    != load_reportItem.combined_plot
		   )
		{
		  maps_different = true;
		  break;
		}
	    }
	}
    }
  
  return maps_different;
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

