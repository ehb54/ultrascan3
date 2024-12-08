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
   return value;
}

// Return an integer parameter value from a US_AnalysisProfileGui child panel
int US_AnalysisProfileGui::childIValue( const QString child, const QString type )
{
   int value      = 0;
   if      ( child == "general"  ) { value = apanGeneral->getIValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getIValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getIValue( type ); }
   return value;
}

// Return a double parameter value from a US_AnalysisProfileGui child panel
double US_AnalysisProfileGui::childDValue( const QString child, const QString type )
{
   double value   = 0.0;
   if      ( child == "general"  ) { value = apanGeneral->getDValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getDValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getDValue( type ); }
   return value;
}

// Return a stringlist parameter value from a US_AnalysisProfileGui child panel
QStringList US_AnalysisProfileGui::childLValue( const QString child, const QString type )
{
   QStringList value;

   if      ( child == "general"  ) { value = apanGeneral->getLValue( type ); }
   else if ( child == "2dsa"     ) { value = apan2DSA   ->getLValue( type ); }
   else if ( child == "pcsa"     ) { value = apanPCSA   ->getLValue( type ); }

   return value;
}

// Slot to handle a new panel selected
void US_AnalysisProfileGui::newPanel( int panx )
{
DbgLv(1) << "APG:newPanel panx=" << panx << "prev.panx=" << curr_panx;
   // Save any changes in the old current panel
   if      ( curr_panx == panx )  return;  // No change in panel

   if      ( curr_panx == 0 ) apanGeneral->savePanel();
   else if ( curr_panx == 1 ) apan2DSA   ->savePanel();
   else if ( curr_panx == 2 ) apanPCSA   ->savePanel();
DbgLv(1) << "APG:newPanel   savePanel done" << curr_panx;

   // Initialize the new current panel after possible changes
   if      ( panx == 0 )      apanGeneral->initPanel();
   else if ( panx == 1 )      apan2DSA   ->initPanel();
   else if ( panx == 2 )      apanPCSA   ->initPanel();
DbgLv(1) << "APG:newPanel   initPanel done" << panx;

   curr_panx              = panx;         // Set new current panel

   // Update status flag for all panels
   statUpdate();
DbgLv(1) << "APG:newPanel   statUpdate done";
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
}

//Slot to DISABLE tabs and Next/Prev buttons
void US_AnalysisProfileGui::disable_tabs_buttons( void )
{
  // apan2DSA     ->initPanel();
  // apan2DSA     ->savePanel();

  // apanPCSA     ->initPanel();
  // apanPCSA     ->savePanel();
  
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

   qDebug() << "In ENABLING 1";

   for ( int ii = 1; ii < tabWidget->count(); ii++ )
   {
      qDebug() << "In ENABLING 2";
      tabWidget ->setTabEnabled( ii, true );
      QPalette pal = tabWidget ->tabBar()->palette();
      tabWidget ->tabBar()->setTabTextColor( ii, pal.color(QPalette::WindowText) ); // Qt::black
      qDebug() << "In ENABLING 2a";
   }

   qDebug() << "In ENABLING 3";
}

//Slot to ENABLE tabs and Next/Prev buttons, but make all Widgets read-only
void US_AnalysisProfileGui::enable_tabs_buttons_readonly( void )
{
DbgLv(1) << "AProfie: ENABLING in read-only mode!!!";
   // pb_next   ->setEnabled( true );
   // pb_prev   ->setEnabled( true );

   qDebug() << "TbWidget count: " << tabWidget->count();

   for ( int ii = 0; ii < tabWidget->count(); ii++ )
   {
      tabWidget ->setTabEnabled( ii, true );
      QPalette pal = tabWidget ->tabBar()->palette();
//DbgLv(1) << "PALETTE: " << pal.color(QPalette::WindowText);
      tabWidget ->tabBar()->setTabTextColor( ii, pal.color( QPalette::WindowText ) ); // Qt::black

      QWidget* pWidget= tabWidget->widget( ii );

      //Find all children of each Tab in QTabWidget [children of all types...]
      QList< QPushButton* > allPButtons  = pWidget->findChildren< QPushButton* >();
      QList< QComboBox* >   allCBoxes    = pWidget->findChildren< QComboBox* >();
      QList< QSpinBox* >    allSBoxes    = pWidget->findChildren< QSpinBox* >();
      QList< QwtCounter* >  allCounters  = pWidget->findChildren< QwtCounter* >();
      QList< QCheckBox* >   allChBoxes   = pWidget->findChildren< QCheckBox* >();
      QList< QLineEdit* >   allLineedits = pWidget->findChildren< QLineEdit* >();
      QList< QGroupBox* >   allGBoxes    = pWidget->findChildren< QGroupBox* >();

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
      for (int jj = 0; jj < allLineedits.count(); jj++ )
	allLineedits[jj] ->setEnabled(false);
      for (int jj = 0; jj < allGBoxes.count(); jj++ )
	allGBoxes[jj] ->setEnabled(false);
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

//Hide 2DSA & PCSA if extType == ABDE
 qDebug() << "AProfile::initPanels(): abde_mode_aprofile ? " << abde_mode_aprofile;
 if ( abde_mode_aprofile )
   {
     this->tabWidget->setTabText( 0, "ABDE Settings");
     this->tabWidget->setTabVisible(1, false);
     this->tabWidget->setTabVisible(2, false);

     //General Gui: modify
     apanGeneral ->set_abde_panel();
     
   }
 else
   {
     this->tabWidget->setTabText( 0, "1: General");
     this->tabWidget->setTabVisible(1, true);
     this->tabWidget->setTabVisible(2, true);

     //General Gui: restore
     
   }
}




// Save all panels in preparation for leaving an AProfile panel
void US_AnalysisProfileGui::savePanels()
{
DbgLv(1) << "AP:sP: IN savePanels()";
   apanGeneral  ->savePanel();
DbgLv(1) << "AP:sP: pG return";

//ALEXEY: need to also re-initiate 2DSA && PCSA (if General tab was changed)
//apan2DSA     ->initPanel();
   apan2DSA     ->savePanel();
DbgLv(1) << "AP:sP: p2 return";
//apanPCSA     ->initPanel();
   apanPCSA     ->savePanel();
DbgLv(1) << "AP:sP: pP return";
}

//========================= End:   Main      section =========================


//========================= Start: General   section =========================

// Initialize a General panel, especially after clicking on its tab
void US_AnaprofPanGen::initPanel()
{
#if 0
use_db=false;
#endif

 AProfIsIntiated = false;

//Setting ref report && DEBUG: check how current ch_reports looks like
 QMap< QString, QMap < QString, US_ReportGMP > >::iterator ri;
 QMap < QString, US_ReportGMP > triple_reports_ref;
 QString chan_desc_ref;
 QString wvl_ref;
 
  for ( ri = currProf->ch_reports.begin(); ri != currProf->ch_reports.end(); ++ri )
    {
      QString chan_desc = ri.key();

      qDebug() << "[BEGIN]US_AnaprofPanGen::initPanel(): chan_desc:::  " << chan_desc;

      //if chan_desc contains "(unspecified)", save this ch_report as the refrence one..
      //copy it to all other channels
      
      if ( chan_desc.contains("(unspecified)") )
	{
	  triple_reports_ref = ri.value();
	  chan_desc_ref      = chan_desc;
	  wvl_ref            = triple_reports_ref.keys()[ 0 ];
	  qDebug() << "Reference report name, wvls, #wvl -- "
		   << chan_desc << triple_reports_ref.keys() << triple_reports_ref.keys().size();
	  continue;
	}
      
      QMap < QString, US_ReportGMP > triple_reports = ri.value();
      QMap < QString, US_ReportGMP >::iterator tri;
      for ( tri = triple_reports.begin(); tri != triple_reports.end(); ++tri )
	{
	  QString c_wvl = tri.key();

	  //if ref_report was not replaced yet, substitute all channels' reports witht he ref. one...
	  if ( !triple_reports_ref.isEmpty() )
	    {
	      currProf->ch_reports[ chan_desc ] [ c_wvl ] = triple_reports_ref[ wvl_ref ];
		  
	    }
	  else //check if some channel's report still contain 's'/'2DSA' etc. (caused by adding/removing wvl(s)/channels)
	    {
	      if ( mainw->abde_mode_aprofile )
		{
		  for(int ii=0; ii< currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems.size(); ++ii)
		    {
		      US_ReportGMP::ReportItem initItem = currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ];
		      if ( initItem.type == 's' && initItem.method.contains("2DSA") )
			{
			  //if attempts to insert reportItem from Velocity-type, replace with ABDE-type
			  currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ].type        = QString("Radius");
			  currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ].method      = QString("raw");
			  currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ].range_low   = 5.8;
			  currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ].range_high  = 7.0;
			}
		    }
		}
	    }
	  //&& debug 
	  for(int ii=0; ii< currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems.size(); ++ii)
	    {
	      
	      US_ReportGMP::ReportItem initItem = currProf->ch_reports[ chan_desc ] [ c_wvl ].reportItems[ ii ];
	      
	      qDebug() << "wvl, type, method, lo, hi -- "
		       << c_wvl
		       << initItem.type
		       << initItem.method
		       << initItem.range_low
		       << initItem.range_high ;
	      
	    }
	}
    }

  //wvls:
  //QMap< QString, QList< double > > ch_wvls;
  QMap< QString, QList < double > >::iterator wi;
  for ( wi = currProf->ch_wvls.begin(); wi != currProf->ch_wvls.end(); ++wi )
    {
      QString chan_desc = wi.key();
      qDebug() << "[BEGIN]US_AnaprofPanGen::initPanel(): chan_desc [ch_wvls]:::  " << chan_desc;
      qDebug() << "Wvls -- " << wi.value();
      
    }
  //END DEBUG
 

//TESTING
   QMap < QString, US_ReportGMP* > ch_report_map;
   US_ReportGMP* reportGMP_1 = new US_ReportGMP();
   US_ReportGMP* reportGMP_2 = new US_ReportGMP();
   US_ReportGMP* reportGMP_3 = new US_ReportGMP();
   US_ReportGMP* reportGMP_4 = new US_ReportGMP();
   ch_report_map.insert ( QString( "280" ), reportGMP_1 ); 
   ch_report_map.insert ( QString( "290" ), reportGMP_2 );
   ch_report_map.insert ( QString( "300" ), reportGMP_3 ); 
   ch_report_map.insert ( QString( "310" ), reportGMP_4 );

//END of TESTING
 
  
   // Populate GUI settings from protocol,analysis controls
   le_protname   ->setText( currProf->protoname );
   le_aproname   ->setText( currProf->aprofname );
DbgLv(1) << "APGe: inP: aname pname" << currProf->aprofname << currProf->protoname;

// DbgLv(1) << "APGe: inP: CALL check_user_level()";
//    check_user_level();
// DbgLv(1) << "APGe: inP:  RTN check_user_level()";

   int nchan      = currProf->pchans.count();
DbgLv(1) << "APGe: inP: nchan" << nchan;
   sl_chnsel.clear();

   //get Rotor's ra_data_type var.
  
   
   // Recompose the list of channel descriptions from protocol
   for ( int ii = 0; ii < nchan; ii++ )
   {
      QString chann  = currProf->pchans  [ ii ];
      //QString chdesc_alt = currProf->chndescs_alt[ ii ];
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
	{
	  
	  continue;
	}
      
      // Otherwise, recompose "channel:optics:solution" and add
      QString chsolu = QString( chdesc ).section( ":", 2, 2 );
      QString chnsel = chname + ":" + chopts + ":" + chsolu;

      sl_chnsel << chnsel;
DbgLv(1) << "APGe: inP:  ch" << ii << "chdesc" << chdesc
 << "chnsel" << chnsel;
   }

    // //ALEXEY: now exclude channels for B:Interf. from the following QLists/QMaps:
    QStringList      chndescs_alt_copy;
    chndescs_alt_copy.clear();
   
    for ( int ii = 0; ii < nchan; ii++ )
      {
        QString chann_desc = currProf->chndescs_alt[ ii ];
        if ( chann_desc.contains("B:Interf") )
    	 continue;
     
        chndescs_alt_copy <<  currProf->chndescs_alt[ ii ];
      }

    qDebug() << "chndescs_alt_copy  -- " << chndescs_alt_copy ;
        
   
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

   // Build the General Layout if need be
DbgLv(1) << "APGe: inP: 0)tol,dae size" << currProf->lv_tolers.count() << currProf->data_ends.count();
DbgLv(1) << "APGe: inP: 0)le_chn,lcr size" << le_channs.count() << le_lcrats.count();
   build_general_layout( );
   nchan          = le_channs.count();
DbgLv(1) << "APGe: inP: 1)tol,dae size" << currProf->lv_tolers.count() << currProf->data_ends.count();
DbgLv(1) << "APGe: inP: 1)le_chn,lcr size" << le_channs.count() << le_lcrats.count()
 << "nchan_gui" << nchan;

   //Clear internal_reports QMap
   internal_reports.clear();
   ref_numbers_list. clear();
 
   if ( le_lcrats.count() == nchan )
   { // Reset General channel parameter gui elements
      for ( int ii = 0; ii < nchan; ii++ )
      {
	qDebug() <<  "currProf->analysis_run.size(): " << currProf->analysis_run.count();
	qDebug() <<  "currProf->scan_excl_begin.size(): " << currProf->scan_excl_begin.count();
	qDebug() <<  "currProf->scan_excl_end.size(): " << currProf->scan_excl_end.count();
	
	//ALEXEY: the next 5 following QLists/QMaps must be stripped of the enties associated with B:Interf. channels!!!
	qDebug() <<  "currProf->wvl_edit.size(): "     << currProf->wvl_edit.count();
	qDebug() <<  "currProf->wvl_not_run.size(): "  << currProf->wvl_not_run.count();
	qDebug() <<  "currProf->ch_wvls.size(): "      << currProf->ch_wvls.count();
	qDebug() <<  "currProf->ch_reports.size():   "  << currProf->ch_reports.count();
	
	qDebug() <<  "currProf->chndescs_alt.size(): " << currProf->chndescs_alt.count();
	
	qDebug() <<  "nchan, sl_chnsel.size(): " << nchan << sl_chnsel.count();
	qDebug() << "ch_wvls for --  " << currProf->chndescs_alt[ ii ] << currProf->ch_wvls[ currProf->chndescs_alt[ ii  ] ];

	qDebug() << "scan_beg, scan_end for -- " << currProf->chndescs_alt[ ii ] << currProf->scan_excl_begin[ ii  ]  << currProf->scan_excl_end[ ii  ];
	
	 int kk          = qMin( ii, sl_chnsel.count() - 1 );
         le_channs[ ii ]->setText( sl_chnsel[ kk ] );
         kk              = qMin( ii, currProf->lc_ratios.count() - 1 );
         le_lcrats[ ii ]->setText( QString::number( currProf->lc_ratios[ kk ] ) );
         kk              = qMin( ii, currProf->lc_tolers.count() - 1 );
         le_lctols[ ii ]->setText( QString::number( currProf->lc_tolers[ kk ] ) );
         kk              = qMin( ii, currProf->l_volumes.count() - 1 );
         le_ldvols[ ii ]->setText( QString::number( currProf->l_volumes[ kk ] ) );
         kk              = qMin( ii, currProf->lv_tolers.count() - 1 );
         le_lvtols[ ii ]->setText( QString::number( currProf->lv_tolers[ kk ] ) );
         kk              = qMin( ii, currProf->data_ends.count() - 1 );
         le_daends[ ii ]->setText( QString::number( currProf->data_ends[ kk ] ) );
	 //abde
	 kk              = qMin( ii, currProf->ld_dens_0s.count() - 1 );
         le_dens0s[ ii ]->setText( QString::number( currProf->ld_dens_0s[ kk ] ) );
	 kk              = qMin( ii, currProf->gm_vbars.count() - 1 );
         le_vbars[ ii ]->setText( QString::number( currProf->gm_vbars[ kk ] ) );
	 kk              = qMin( ii, currProf->gm_mws.count() - 1 );
         le_MWs[ ii ]->setText( QString::number( currProf->gm_mws[ kk ] ) );

	 kk              = qMin( ii, currProf->analysis_run.count() - 1 );

	 qDebug() << "kk out of currProf->analysis_run: " << kk;

	 //Analysis run
	 if ( currProf->analysis_run[ kk ] )
	   ck_runs[ ii ] ->setChecked( true  );
	 else
	   ck_runs[ ii ] ->setChecked( false  );

	 //Replicates
	 sb_repl_groups[ ii ]->setValue( currProf->replicates[ kk ] );
	 
	 //Report run
	 if ( currProf->report_run[ kk ] )
	   ck_report_runs[ ii ] ->setChecked( true  );
	 else
	   ck_report_runs[ ii ] ->setChecked( false  );

	 //ABDE: ref, use_ref
	 //sb_ref_chs[ ii ]     ->setValue( currProf->ref_channels[ kk ] );
	 ( currProf->ref_channels[ kk ] > 0 ) ?
	   le_ref_chs[ ii ]     ->setText( "Ref:" + QString::number( currProf->ref_channels[ kk ]) ) :
	   le_ref_chs[ ii ]     ->setText( "" );
	 ref_numbers_list << currProf->ref_channels[ kk ];
	 sb_use_ref_chs[ ii ] ->setValue( currProf->ref_use_channels[ kk ] );

	 qDebug() << "Init APfor::Gen: currProf->ref_use_channels[ kk ] -- " <<  currProf->ref_use_channels[ kk ];
	 qDebug() << "Init APfor::Gen: ref_numbers_list -- " <<  ref_numbers_list;
	 
	 DbgLv(1) << "APGe: inP:    ii kk" << ii << kk << "chann" << sl_chnsel[kk] << "lvtol daend dae[kk]"
		  << currProf->lv_tolers[ii] << currProf->data_ends[ii] << currProf->data_ends[kk]
		  << "currProf->analysis_run[ ii] currProf->analysis_run[ kk ]" << currProf->analysis_run[ ii ] << currProf->analysis_run[ kk ];

       
	 //ALEXEY: also set info on wvl for edit 
	 kk              = qMin( ii, currProf->wvl_edit.count() - 1 );

	 qDebug() << "ii kk currProf->wvl_edit[ ii] currProf->wvl_edit[ kk ]" << ii << kk << currProf->wvl_edit[ ii ] << currProf->wvl_edit[ kk ];

	 QScrollArea *sa = gr_mwvbox[ ii ];
	 foreach (QRadioButton *button, sa->findChildren<QRadioButton*>())
	   {
	     int wvl_to_edit = (button->objectName()).split(":")[2].toInt();

	     qDebug() << "currProf->wvl_edit[ kk ] == wvl_to_edit -- " << currProf->wvl_edit[ kk ] <<  " = " <<  wvl_to_edit;
	     
	     if ( currProf->wvl_edit[ kk ] == wvl_to_edit )  // <-- NOT FULFILLED ????????
	       {
		 button->setChecked( true );
		 button->click();
		 qDebug() << "US_AnaprofPanGen::initPanel(): wvl_to_edit " <<  wvl_to_edit << " for channel " << ii; 
		 break;
	       }
	   }

	 //ALEXEY: set ReportGMP per channel
	 kk              = qMin( ii, currProf->chndescs_alt.count() - 1 );
	 qDebug() << "US_AnaprofPanGen::initPanel(): ReportGMP kk, ii currProf->chndescs_alt.count() " << kk <<  ii << currProf->chndescs_alt.count();
	 
	 //QString chdesc_alt = currProf->chndescs_alt[ kk ];
	 QString chdesc_alt = chndescs_alt_copy[ ii ]; 
	 qDebug() << "US_AnaprofPanGen::initPanel(): chdesc_alt " << chdesc_alt;

	 //ALEXEY_NEW_REPORT: there will be cycle over wvls:
	 // QList of currProf->ch_wvls[ chdesc_alt ] -> current_wvl:
	 // internal_reports[ chdesc_alt ][ wvl ] = currProf->ch_reports[ chdesc_alt ] [ wvl ];

	 //TESTING
	 //report_map[ chdesc_alt ] = ch_report_map;
	 //END_OF_TESTING

	 QList < double > ch_wavelengths = currProf->ch_wvls[ chdesc_alt ];
	 for ( int w=0; w<ch_wavelengths.size(); ++w )
	   {
	     QString curr_w = QString::number ( ch_wavelengths[ w ] );
	     internal_reports[ chdesc_alt ][ curr_w ] = currProf->ch_reports[ chdesc_alt ] [ curr_w ];
	     internal_reports[ chdesc_alt ][ curr_w ].wavelength = ch_wavelengths[ w ];
	     //internal_reports[ chdesc_alt ][ curr_w ].experiment_duration = currProf->ch_reports[ chdesc_alt ] [ curr_w ].experiment_duration;

	     qDebug() << "US_AnaprofPanGen::initPanel(): ch_wavelengths[ w ] --  " << ch_wavelengths[ w ];
	   }
	 
	 //internal_reports[ chdesc_alt ] = currProf->ch_reports[ chdesc_alt ];
	 qDebug() << "US_AnaprofPanGen::initPanel(): internal_reports[ chdesc_alt ].size() -- " << internal_reports.size();
	 qDebug() << "US_AnaprofPanGen::initPanel(): internal_reports[ chdesc_alt ][ QString::number ( ch_wavelengths[ 0 ] )] -- "
		  << internal_reports[ chdesc_alt ][ QString::number ( ch_wavelengths[ 0 ] )].tot_conc
		  << internal_reports[ chdesc_alt ][ QString::number ( ch_wavelengths[ 0 ] )].wavelength;

	 // //ALEXEY_NEW_REPORT: this .wavelength needs to be properly set for each triple
	 // internal_reports[ chdesc_alt ].wavelength = currProf->wvl_edit[ kk ];
	 // qDebug() << "US_AnaprofPanGen::initPanel(): internal_reports[ chdesc_alt ].wavelength, currProf->wvl_edit.size() -- "
	 // 	  << internal_reports[ chdesc_alt ].wavelength
	 // 	  << currProf->wvl_edit.size();

	 ///////////////////////////////////////////////////////////
	 
	 

	 //Also, important to re-insert correct-order chndescs_alt!!!
	 currProf->chndescs_alt[ ii ] = chdesc_alt;
	 
	 //ALEXEY: also set info on wvl not to be analyzed 
	 kk              = qMin( ii, currProf->wvl_not_run.count() - 1 );

	 //ALEXEY: this string caused crash
	 //qDebug() << "currProf->wvl_not_run[ ii] currProf->wvl_not_run[ kk ]" << currProf->wvl_not_run[ ii ] << currProf->wvl_not_run[ kk ];

	 foreach (QCheckBox *ckbox, sa->findChildren<QCheckBox*>())
	   {
	     QString wvl_not_to_run = (ckbox->objectName()).split(":")[2];
	     
	     if ( currProf->wvl_not_run[ kk ].contains( wvl_not_to_run ) )
	       {
		 ckbox->setChecked( false );
		 
		 qDebug() << "US_AnaprofPanGen::initPanel(): wvl_not_to_run " <<  wvl_not_to_run << " for channel " << ii; 
	       }
	   }
      }
   }
else
 DbgLv(1) << "APGe: inP:  lcrats count, nchan" << le_lcrats.count() << nchan;

   //Also show MWL prefs for the first channel to fill the space
   if ( ck_runs[ 0 ] ->isChecked( ) )
     ck_mwv[ 0 ] ->setChecked( true  );
   else
     ck_mwv[ 0 ] ->setChecked( false  );

   //ABDE: for ref./use ref fields, set the 1st B-chann, to a reference
   //ABDE: IF NO refs. set at all
   AProfIsIntiated = true;
   if ( mainw->abde_mode_aprofile )
     {
       bool all_reps_run = true;
       for( int rr=0; rr<currProf->report_run.size(); ++rr)
	 if ( currProf->report_run[rr] == 0  )
	   all_reps_run = false;

       if ( all_reps_run )
	 {
	   for( int rr=0; rr<currProf->chndescs_alt.size(); ++rr )
	     {
	       QString ch_name = currProf->chndescs_alt[rr];
	       if( ch_name.split(":")[0].contains("B") )
		 {
		   ck_runs[ rr ] -> setChecked( false );
		   break;
		 }
	     }
	 }
       else //boud max for sb_use_ref_chs counters
	 {
	   int ref_number_max = 0;
	   for (int rn=0; rn<ref_numbers_list.size(); ++rn )
	     if (ref_numbers_list[rn] > ref_number_max )
	       ref_number_max = ref_numbers_list[rn];

	   for ( int i=0; i<sb_use_ref_chs.size(); ++i)
	     sb_use_ref_chs[ i ]->setMaximum( ref_number_max );
	 }
     }

   // Save to update Gui
   qDebug() << "US_AnaprofPanGen::initPanel(): before save: currProf->chndescs_alt, size() -- " << currProf->chndescs_alt << currProf->chndescs_alt.size();
   savePanel();

   DbgLv(1) << "APGe: FROM initAprfile:General - inP: CALL check_user_level()";
      check_user_level();
   DbgLv(1) << "APGe: inP:  FROM initAprfile:General - RTN check_user_level()";

   
   //if expTyp "ABDE": modify AProfile's GUI
   
}


// Modify general setting for ABDE exptype
void US_AnaprofPanGen::set_abde_panel()
{

  lb_lcrat -> setVisible( false );
  lb_lctol -> setVisible( false );
  lb_daend -> setVisible( false );
  lb_mwvprefs -> setVisible( false );
  
  int nchn        = sl_chnsel.count();
  qDebug() << "modifying General tab for ABDE: nchn -- " << nchn;
  for ( int ii = 0; ii < nchn; ii++ )
    {
      le_lcrats[ ii ]->setVisible( false );
      le_lctols[ ii ]->setVisible( false );
      le_daends[ ii ]->setVisible( false );
      ck_mwv[ ii ]   ->setChecked( false );
      ck_mwv[ ii ]   ->setVisible( false );
    }
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






// //[OLD WAY] --  IF USER cannot edit anything (low-level user)
// void US_AnaprofPanGen::check_user_level()
// {
//    //update_inv();
// DbgLv(1) << "APGe:ckul: level" << US_Settings::us_inv_level();
//    if ( US_Settings::us_inv_level() < 3 )
//    {
//       pb_aproname->setEnabled( false );
//       pb_protname->setEnabled( false );

// //      if ( !loaded_prof )
// //         emit set_tabs_buttons_inactive();
// //      else
// //         emit set_tabs_buttons_active_readonly();

//       emit set_tabs_buttons_active_readonly();

// DbgLv(1) << "SIGNAL!!!!" ;
//    }
// }

//NEW -- do NOT disable 8. AProfile && internal GUI for any-level UL
void US_AnaprofPanGen::check_user_level()
{
   //update_inv();
DbgLv(1) << "APGe:ckul: level" << US_Settings::us_inv_level();
//    if ( US_Settings::us_inv_level() < 3 )
//    {
//       pb_aproname->setEnabled( false );
//       pb_protname->setEnabled( false );

// //      if ( !loaded_prof )
// //         emit set_tabs_buttons_inactive();
// //      else
// //         emit set_tabs_buttons_active_readonly();

//       emit set_tabs_buttons_active_readonly();

// DbgLv(1) << "SIGNAL!!!!" ;
//    }
}

// Save panel controls when about to leave the panel
void US_AnaprofPanGen::savePanel()
{
DbgLv(1) << "APGe: svP: IN";
   // Populate protocol controls from GUI settings
   currProf->protoname  = le_protname->text();
   currProf->aprofname  = le_aproname->text();
   int nchan       = le_lcrats.count();
   nchan           = qMin( nchan, le_lctols.count() );
   nchan           = qMin( nchan, le_ldvols.count() );
   nchan           = qMin( nchan, le_lvtols.count() );
   nchan           = qMin( nchan, le_daends.count() );

   nchan           = qMin( nchan, ck_runs.count() );
   
DbgLv(1) << "APGe: svP:  kle cr,ct,dv,vt,de"
 << le_lcrats.count() << le_lctols.count() << le_ldvols.count()
 << le_lvtols.count() << le_daends.count() << "nchan" << nchan;

 
// if ( currProf->pchans.count() == nchan )       <--- ALEXEY: BUG commented: very important to re-generate GUI (e.g. when Optics chenged by adding/removing Interfrence)
// Otherwise, with Interfence, no changes are saved from the Geb tab GUI && no changes written to Aprofile DB !!
    { // Reset General channel parameter gui elements
      currProf->lc_ratios.clear( );
      currProf->lc_tolers.clear( );
      currProf->l_volumes.clear( );
      currProf->lv_tolers.clear( );
      currProf->data_ends.clear( );

      //abde
      currProf->ld_dens_0s.clear( );
      currProf->gm_vbars.clear( );
      currProf->gm_mws.clear( );
      currProf->ref_channels.clear( );
      currProf->ref_use_channels.clear( );
      
      currProf->analysis_run .clear( );
      currProf->report_run   .clear( );
      currProf->wvl_edit     .clear( );
      currProf->wvl_not_run  .clear( );

      currProf->ch_reports.clear();

      currProf->replicates                         .clear();
      currProf->replicates_to_channdesc_main       .clear();
      currProf->channdesc_to_overlapping_wvls_main .clear();

      // currProf->ch_report_ids.clear();
      // currProf->ch_report_guids.clear();

      //ALEXEY: here -- for replicate groups -- we need to process internal_reports:
      //1. group channel_descriptions by replicate group #, e.g. QMap< repl_group_number, QStringList ( chdesc_alt ) >;
      //   1a. if repl_group_number == 0 (default), OR QStringList.size() == 1 : not really a group, so continue / skip;
      //2. internal_report corresponding to QStringList[ 0 ] - is a primary ReportGMP; make it to be all other reports in a group  
      QMap< int, QStringList >     replicates_to_channdesc;
      QMap< QString, QStringList > channdesc_to_overlapping_wvls;

      for ( int ii = 0; ii < nchan; ii++ )
	{
	  if ( sb_repl_groups[ ii ]->isEnabled() )
	    {
	      int replicate_group_number   = sb_repl_groups[ ii ]->value();
	      QString chdesc_alt           = currProf->chndescs_alt[ ii ];
	      replicates_to_channdesc[ replicate_group_number ] << chdesc_alt;

	      currProf->replicates_to_channdesc_main [ replicate_group_number ] << chdesc_alt;
	    }
	}
      QMap< int, QStringList >::iterator rgi;
      for ( rgi = replicates_to_channdesc.begin(); rgi != replicates_to_channdesc.end(); ++rgi )
	{
	  int group_number           = rgi.key();
	  QStringList group_channels = rgi.value();

	  if( group_number == 0 || group_channels.size() == 1 )
	    continue;

	  //do copying US_ReportGMPs here:
	  // internal_reports IS map-of-maps: QMap < chann_desc, QMap< wvl_number,  US_ReportGMP >>:
	  // So, what do we consider the primary ReportGMP?
	  //          -- 1st channel | 1st wavelength ?
	  //          -- e.g.: 1st wavelength:
	  //                   QList < QString > ch_wavelengths = internal_reports[ group_channels[0] ].keys();
	  //          -- e.g.: US_ReportGMP primary_report = internal_reports[ group_channels[0] ] [ ch_wavelengths[0] ] ?
	  //

	  //Go over all reference reports (triple reports in the 1st channel of the replicate group): 
	  QList < QString > reference_ch_wvls = internal_reports[ group_channels[0] ].keys();

	  for( int rgj = 0; rgj < reference_ch_wvls.size(); ++rgj )
	    {
	      US_ReportGMP reference_group_report = internal_reports[ group_channels[0] ] [ reference_ch_wvls[ rgj ] ];
	      //Now, go over the reports for other channels in a group (per-triple basis)
	      for( int i = 1; i < group_channels.size(); ++i )
		{
		  QList < QString > other_group_ch_wvls = internal_reports[ group_channels[i] ].keys();

		  for( int j = 0; j < other_group_ch_wvls.size(); ++j )
		    {
		      //If other channel's wvl is the same to the reference channel's wvl, copy Report's params
		      if ( other_group_ch_wvls[ j ] == reference_ch_wvls[ rgj ] )
			{
			  //QMap of the overlapping replicate group wavelengths (all overlapping triples):
			  QString ref_triple   = group_channels[ 0 ].split(":")[0] + "." + reference_ch_wvls[ rgj ];
			  QString other_triple = group_channels[ i ].split(":")[0] + "." + other_group_ch_wvls[ j ];
			  
			  channdesc_to_overlapping_wvls[ group_channels [0] ]                << ref_triple << other_triple;
			  currProf->channdesc_to_overlapping_wvls_main [ group_channels[0] ] << ref_triple << other_triple;

			  //Modify the report with the reference one:
			  US_ReportGMP *other_group_report_to_change = &(internal_reports[ group_channels[ i ] ] [ other_group_ch_wvls[ j ] ]);

			  other_group_report_to_change->tot_conc                 = reference_group_report.tot_conc;
			  other_group_report_to_change->tot_conc_tol             = reference_group_report.tot_conc_tol;
			  
			  other_group_report_to_change->rmsd_limit               = reference_group_report.rmsd_limit;
			  other_group_report_to_change->av_intensity             = reference_group_report.av_intensity;
			  
			  other_group_report_to_change->experiment_duration      = reference_group_report.experiment_duration;
			  other_group_report_to_change->experiment_duration_tol  = reference_group_report.experiment_duration_tol;

			  other_group_report_to_change->tot_conc_mask            = reference_group_report.tot_conc_mask;
			  other_group_report_to_change->rmsd_limit_mask          = reference_group_report.rmsd_limit_mask;
			  other_group_report_to_change->av_intensity_mask        = reference_group_report.av_intensity_mask;
			  other_group_report_to_change->experiment_duration_mask = reference_group_report.experiment_duration_mask;
			  other_group_report_to_change->integration_results_mask = reference_group_report.integration_results_mask;
			  other_group_report_to_change->plots_mask               = reference_group_report.plots_mask;

			  other_group_report_to_change->pseudo3d_2dsait_s_ff0  = reference_group_report.pseudo3d_2dsait_s_ff0  ;
			  other_group_report_to_change->pseudo3d_2dsait_s_d    = reference_group_report.pseudo3d_2dsait_s_d    ;
			  other_group_report_to_change->pseudo3d_2dsait_mw_ff0 = reference_group_report.pseudo3d_2dsait_mw_ff0 ;
			  other_group_report_to_change->pseudo3d_2dsait_mw_d   = reference_group_report.pseudo3d_2dsait_mw_d   ;
			  other_group_report_to_change->pseudo3d_2dsamc_s_ff0  = reference_group_report.pseudo3d_2dsamc_s_ff0  ;
			  other_group_report_to_change->pseudo3d_2dsamc_s_d    = reference_group_report.pseudo3d_2dsamc_s_d    ;
			  other_group_report_to_change->pseudo3d_2dsamc_mw_ff0 = reference_group_report.pseudo3d_2dsamc_mw_ff0 ;
			  other_group_report_to_change->pseudo3d_2dsamc_mw_d   = reference_group_report.pseudo3d_2dsamc_mw_d   ;
			  other_group_report_to_change->pseudo3d_pcsa_s_ff0    = reference_group_report.pseudo3d_pcsa_s_ff0    ;
			  other_group_report_to_change->pseudo3d_pcsa_s_d      = reference_group_report.pseudo3d_pcsa_s_d      ;
			  other_group_report_to_change->pseudo3d_pcsa_mw_ff0   = reference_group_report.pseudo3d_pcsa_mw_ff0   ;
			  other_group_report_to_change->pseudo3d_pcsa_mw_d     = reference_group_report.pseudo3d_pcsa_mw_d     ;

			  other_group_report_to_change->pseudo3d_mask          = reference_group_report.pseudo3d_mask          ;
			  
			  //now, clear rerortItems && fill with reference ReportGMP:
			  other_group_report_to_change->reportItems.clear();
			  
			  for ( int ic = 0; ic < reference_group_report.reportItems.size(); ++ic )
			    other_group_report_to_change->reportItems.push_back( reference_group_report.reportItems[ ic ] );
			}
		    }
		}
	    }
	}
      
      QMap< QString, QStringList >::iterator wvl_overlap;
      for ( wvl_overlap = channdesc_to_overlapping_wvls.begin(); wvl_overlap != channdesc_to_overlapping_wvls.end(); ++wvl_overlap )
	{
	  wvl_overlap.value().removeDuplicates();
	  qDebug() << "Reference Channel: " << wvl_overlap.key() << ", list of overlapping wvls: " << wvl_overlap.value();
	}
      //repeat duplicates removal for upper-level currPof->channdesc_to_overlapping_wvls_main:
      for ( wvl_overlap = currProf->channdesc_to_overlapping_wvls_main.begin(); wvl_overlap != currProf->channdesc_to_overlapping_wvls_main.end(); ++wvl_overlap )
	{
	  wvl_overlap.value().removeDuplicates();
	   qDebug() << "Reference Channel MAIN: " << wvl_overlap.key() << ", list of overlapping wvls: " << wvl_overlap.value();
	}
      // END of copying reference RpeortGMP per replicate group   //////////////////////////////////////////////////////////

      
      
      for ( int ii = 0; ii < nchan; ii++ )
      {
         currProf->lc_ratios << le_lcrats[ ii ]->text().toDouble();
         currProf->lc_tolers << le_lctols[ ii ]->text().toDouble();
         currProf->l_volumes << le_ldvols[ ii ]->text().toDouble();
         currProf->lv_tolers << le_lvtols[ ii ]->text().toDouble();
         currProf->data_ends << le_daends[ ii ]->text().toDouble();

	 //abde
	 currProf->ld_dens_0s << le_dens0s[ ii ]->text().toDouble();
	 currProf->gm_vbars   << le_vbars[ ii ]->text().toDouble();
	 currProf->gm_mws     << le_MWs[ ii ]->text().toDouble();
	 //currProf->ref_channels << sb_ref_chs[ ii ]->value();
	 ( le_ref_chs[ ii ]->text().isEmpty() ) ?
	   currProf->ref_channels << 0 : currProf->ref_channels << le_ref_chs[ ii ]->text().split(":")[1].toInt();
	 currProf->ref_use_channels << sb_use_ref_chs[ ii ]->value();
	 qDebug() << "in SavePanel() AProf_Gen: chann, sb_use_ref_chs[ ii ]->value() -- " << ii << sb_use_ref_chs[ ii ]->value();

	 //ALEXEY: add additional field for channels to be or not to be analysed
	 if ( ck_runs[ ii ]->isChecked() ) 
	   currProf->analysis_run << 1;
	 else
	   currProf->analysis_run << 0;

	 qDebug() << "APGR: SAVE: run channel analysis -- " << ii << int(ck_runs[ ii ]->isChecked());


	 //Replicates
	 currProf->replicates << sb_repl_groups[ ii ]->value();

	 
	 //ALEXEY: add additional field for channels' reports
	 if ( ck_report_runs[ ii ]->isChecked() ) 
	   currProf->report_run << 1;
	 else
	   currProf->report_run << 0;

	 
	 qDebug() << "APGR: SAVE: run channel report -- " << ii << int(ck_report_runs[ ii ]->isChecked());
	 

	 //ALEXEY: also save info on wvl for edit 
	 QScrollArea *sa = gr_mwvbox[ ii ];
	 foreach (QRadioButton *button, sa->findChildren<QRadioButton*>())
	   {
	     if ( button->isChecked() )
	       {
		 int wvl_to_edit = (button->objectName()).split(":")[2].toInt();
		 qDebug() << "US_AnaprofPanGen::savePanel(): wvl_to_edit " <<  wvl_to_edit << " for channel " << ii; 
		 
		 currProf->wvl_edit << wvl_to_edit;
		 break;
	       }
	   }

	 //ALEXEY: also save ReportGMP per channel
	 QString chdesc_alt = currProf->chndescs_alt[ ii ];
	 qDebug() << "US_AnaprofPanGen::savePanel(): chdesc_alt -- " << chdesc_alt;


	 
	 //ALEXEY_NEW_REPORT: will be a cycle over wvl
	 // QMap internal_reports[ chdesc_alt ]:: keys(); --> all channel's wvls
	 // currProf->ch_reports[ chdesc_alt ] [ wvl ] = internal_reports[ chdesc_alt ][ wvl ]; 
	 QList < QString > ch_wavelengths = internal_reports[ chdesc_alt ].keys();
	 qDebug() << "internal_reports[ chdesc_alt ].keys()   -- " << internal_reports[ chdesc_alt ].keys();
	 qDebug() << "internal_reports[ chdesc_alt ][ ch_wavelengths[0] ] -- "
		  << internal_reports[ chdesc_alt ][ ch_wavelengths[0] ].tot_conc
		  << internal_reports[ chdesc_alt ][ ch_wavelengths[0] ].wavelength;
	 
	 for ( int w=0; w<ch_wavelengths.size(); ++w )
	   {
	     QString curr_w = ch_wavelengths[ w ];
	     qDebug() << "curr_w -- " << curr_w;

	     currProf->ch_reports[ chdesc_alt ][ curr_w ] = internal_reports[ chdesc_alt ][ curr_w ];
	     qDebug() << "After insertion..";
	   }
	 
	 //currProf->ch_reports[ chdesc_alt ] = internal_reports[ chdesc_alt ];	 
	 
	 // //ALEXEY: fill QMap relating chdesc_alt to future reportID or reportGUID
	 // currProf->ch_report_ids[ chdesc_alt ]   = 0;
	 // currProf->ch_report_guids[ chdesc_alt ] = QString( "00000000-0000-0000-0000-000000000000" );
	 
	 //ALEXEY: also save info on wvl not to be analyzed
	 QString wvl_list_not_run;
	 foreach (QCheckBox *ckbox, sa->findChildren<QCheckBox*>())
	   {
	     if ( !ckbox->isChecked() )
	       {
		 qDebug() << "US_AnaprofPanGen::savePanel(): wvl_not_to_run " <<  (ckbox->objectName()).split(":")[2] << " for channel " << ii;

		 wvl_list_not_run += (ckbox->objectName()).split(":")[2] + ":";
	       }
	   }
	 wvl_list_not_run.chop(1);
	 currProf->wvl_not_run << wvl_list_not_run;
	 
      }
   }
DbgLv(1) << "APGe: svP:  done";
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
   int nchan      = currProf->lc_ratios.count();
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
   else if ( type == "f_daend"   )
   {  value = ( nchan > 0 ) ? currProf->data_ends[  0 ] : 0.0; }
   else if ( type == "l_daend"   )
   {  value = ( nchan > 0 ) ? currProf->data_ends[ kk ] : 0.0; }

   return value;
}

// Get specific panel list values
QStringList US_AnaprofPanGen::getLValue( const QString type )
{
   QStringList value( "" );
   int nchan      = currProf->lc_ratios.count();

   if      ( type == "channels" )       { value = sl_chnsel; }
   else if ( type == "analysisrun" )
   {
     value.clear();
     //for ( int ii = 0; ii < nchan; ii++ )
       for ( int ii = 0; ii < sl_chnsel.count(); ii++ )
       {
	 qDebug() << "Ana RUN for channel " << ii << " is: "  << currProf->analysis_run[ ii ];
	 value << QString::number (currProf->analysis_run[ ii ]) ;
	 qDebug() << "Ana RUN for channel " << ii << " value: " << value;
       }
   }
   else if ( type == "wvledit" )
   {
     value.clear();
     //for ( int ii = 0; ii < nchan; ii++ )
     for ( int ii = 0; ii < sl_chnsel.count(); ii++ )
       {
	 qDebug() << "WVL EDIT for channel " << ii << " is: "  << currProf->wvl_edit[ ii ];
	 value << QString::number (currProf->wvl_edit[ ii ]) ;
	 qDebug() << "WVL EDIT for channel " << ii << " value: " << value;
       }
   }
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
   else if ( type == "dataends" )
   {
      for ( int ii = 0; ii < nchan; ii++ )
      {
         value << QString::number( currProf->data_ends[ ii ] );
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

// Initialize a 2DSA sub-panel, especially after clicking on its tab
void US_AnaprofPan2DSA::initPanel()
{
  ap2DSA             = &(mainw->currProf.ap2DSA);
  //ap2DSA             = &(mainw->currProf_copy.ap2DSA);
  qDebug() << "INIT 2DSA-begin: ap2DSA.parm SIZE -- " << ap2DSA->parms.size();

  //Debug
  for ( int j = 0; j < ap2DSA->parms.size(); j++ )
    qDebug() << "Channels in COPY 2DCA parms vector --  " <<  ap2DSA->parms[ j ].channel;
  
  DbgLv(1) << "AP2d:inP:  IN";

   sl_chnsel          = sibLValue( "general", "channels" );
   qDebug() << "2DSA: sl_chnsel --- " << sl_chnsel;
   QStringList chnls_to_run  = sibLValue( "general", "analysisrun" );
   qDebug() << "2DSA: QStringList chnls_to_run: " << chnls_to_run;

   //chnls_to_run.clear();
   /*
   // //ALEXEY: remove those channels that are not selected for RUN
   // qobject_cast<QListView *>(cb_chnsel->view())->setRowHidden(0, true);
   QStringList chnls_to_run  = sibLValue( "general", "analysisrun" );
   qDebug() << "Size of chnls_to_run: " <<  chnls_to_run.size();
   qDebug() << "QStringList chnls_to_run: " << chnls_to_run;

   QStringList chnls_to_remove;
   for ( int i = 0; i < chnls_to_run.size(); i++ )
     {
       if ( ! chnls_to_run[ i ].toInt() )
	 chnls_to_remove << sl_chnsel[ i ];
     }
   
   for ( int i = 0; i < chnls_to_remove.size(); i++ )
     sl_chnsel.removeOne( chnls_to_remove[i] );
   
   qDebug() << "New size of sl_chnsel is: " << sl_chnsel.size();
   // End of channels removal
   */
   cb_chnsel->clear();
   cb_chnsel->addItems( sl_chnsel );

   active_items_2dsa.clear();
   qDebug() << "SIZE OF active_items_2dsa: " << active_items_2dsa.size();
   QStandardItemModel * model = qobject_cast<QStandardItemModel*>(cb_chnsel->model());
   for ( int i = 0; i < chnls_to_run.size(); i++ )
     {
       QStandardItem * item = model->item( i );
       if ( ! chnls_to_run[ i ].toInt() )  //deactivate item
	 item->setEnabled( false );
       else                                //activate item
	 {
	   item->setEnabled( true );
	   active_items_2dsa.push_back( i );
	 }
     }

   int first_avail = 0;
   if ( active_items_2dsa.size() )
     first_avail = active_items_2dsa[ 0 ];
   

   /*
   // Populate GUI settings from protocol controls
   //int kparm          = ap2DSA->parms.size();
   qDebug() << "2DSA params: OLD size/content: " << ap2DSA->parms.size();

   //ALEXEY: now we need to remove 2DSA params for removed channels:
   for ( int i = 0; i < chnls_to_remove.size(); i++ )
     {
       for ( int j = 0; j < ap2DSA->parms.size(); j++ )
	 {
	   if ( ap2DSA->parms[ j ].channel == chnls_to_remove[ i ] )
	     {
	       qDebug() << "2DSA parms for channel -- " << ap2DSA->parms[ j ].channel << " will be removed!";
	       ap2DSA->parms.remove( j );
	       break;
	     }
	 }
     }
   
   qDebug() << "2DSA params: NEW size/content: " << ap2DSA->parms.size();
   //End of 2DSA parm removal
   */

   int kparm          = ap2DSA->parms.size();
   
   int kchan          = sl_chnsel.size();
DbgLv(1) << "AP2d:inP:  kparm kchan" << kparm << kchan;
   US_AnaProfile::AnaProf2DSA::Parm2DSA parm1;
   if ( kparm > 0 )
     //parm1              = ap2DSA->parms[ 0 ];
     parm1              = ap2DSA->parms[ first_avail ];
   
   for ( int ii = kparm; ii < kchan; ii++ )
   {
      parm1.channel      = sl_chnsel[ ii ];
DbgLv(1) << "AP2d:inP:    set-parm ii" << ii << "channel" << parm1.channel;
      ap2DSA->parms << parm1;
   }

   // cchx            = 0;
   // parms_to_gui( 0 );
   cchx            = first_avail;
   parms_to_gui( first_avail );
   
DbgLv(1) << "AP2d:inP:  parms_to_gui complete";

   le_j2gpts->setText( QString::number( ap2DSA->grpoints ) );
   le_j2mrng->setText( QString::number( ap2DSA->fitrng ) );

   le_j2iter->setText( QString::number( ap2DSA->j2rfiters ) );
   
   le_j4iter->setText( QString::number( ap2DSA->rfiters ) );
   le_j5iter->setText( QString::number( ap2DSA->mciters ) );
   ck_j1run ->setChecked( ap2DSA->job1run );
   ck_j2run ->setChecked( ap2DSA->job2run );
   ck_j3run ->setChecked( ap2DSA->job3run );
   ck_j4run ->setChecked( ap2DSA->job4run );
   ck_j5run ->setChecked( ap2DSA->job5run );

   qDebug() << "In 2DSA's initPanel(): ap2DSA->job3auto -- " << ap2DSA->job3auto;
   ck_j3auto->setChecked( ap2DSA->job3auto );

   //ALEXEY: fit M|B
   foreach (QRadioButton *button, meniscus_box->findChildren<QRadioButton*>())
     {
       if ( button->objectName() == ap2DSA->fmb )
	 {
	   button->setChecked( true );
	   break;
	 }
     }

   bool was_changed     = changed;       // Save changed state
   changed              = was_changed;   // Restore changed state
}

// Save panel controls when about to leave the panel
void US_AnaprofPan2DSA::savePanel()
{
DbgLv(1) << "AP2d:svP: IN";
   // Populate protocol 2DSA controls from internal panel control
   ap2DSA               = &(mainw->currProf.ap2DSA);
   int nparm            = ap2DSA ->parms.count();
DbgLv(1) << "AP2d:svP: nparm" << nparm << "cchx" << cchx;

   gui_to_parms( cchx );

   ap2DSA->nchan        = nparm;
   ap2DSA->fitrng       = le_j2mrng->text().toDouble();
 
   ap2DSA->j2rfiters    = le_j2iter->text().toDouble();
   
   ap2DSA->grpoints     = le_j2gpts->text().toInt();
   ap2DSA->rfiters      = le_j4iter->text().toInt();
   ap2DSA->mciters      = le_j5iter->text().toInt();
   ap2DSA->job1run      = ck_j1run ->isChecked();
   ap2DSA->job2run      = ck_j2run ->isChecked();
   ap2DSA->job3run      = ck_j3run ->isChecked();
   ap2DSA->job4run      = ck_j4run ->isChecked();
   ap2DSA->job5run      = ck_j5run ->isChecked();
   ap2DSA->job3auto     = ck_j3auto->isChecked();
   ap2DSA->job1nois     = tr( "(TI Noise)" );
   ap2DSA->job2nois     = tr( "(TI+RI Noise)" );
   ap2DSA->job4nois     = tr( "(TI+RI Noise)" );

   //ALEXEY: FM|B radio butttons
   foreach (QRadioButton *button, meniscus_box->findChildren<QRadioButton*>())
     {
       if (button->isChecked())
	 {
	   ap2DSA->fmb = button->objectName();
	   break;
	 }
     }

   DbgLv(1) << "AP2d:svP:   runs:"
	    << ap2DSA->job1run << ap2DSA->job2run << ap2DSA->job3run
	    << ap2DSA->job4run << ap2DSA->job5run;

   //ALEXEY: here, process replicate groups defined in Gen tab:
   //1. Identify primary channel & its parameters;
   //2. Copy (replace) other's replicate group channel's parms with these reference parameters

   US_AnaProfile* currProf  = &mainw->currProf;
   QMap < int, QStringList > replicates_to_channdesc = currProf->replicates_to_channdesc_main;
   
   QMap< int, QStringList >::iterator rgi;
   for ( rgi = replicates_to_channdesc.begin(); rgi != replicates_to_channdesc.end(); ++rgi )
     {
       int group_number           = rgi.key();
       QStringList group_channels = rgi.value();

       qDebug() << "In 2DSA's savePanel(): repl. number, channels -- "
		<< group_number << group_channels;
       
       if( group_number == 0 || group_channels.size() == 1 )
	    continue;

       //identify reference 2DSA parms:
       US_AnaProfile::AnaProf2DSA::Parm2DSA reference_2dsa_parms;
       for ( int i = 0; i < nparm; ++i  )
	 {
	   QString chan_name  = ap2DSA->parms.at( i ).channel;
	   if ( chan_name == group_channels[0] )
	     {
	       reference_2dsa_parms = ap2DSA->parms.at( i );
	       break;
	     }
	 }
       //now apply ref_parms's parameters to other channels' 2DSA settings:
       qDebug() << "Reference 2DSA parms for channel " << group_channels[0] << reference_2dsa_parms.channel;
       for( int ii = 1; ii < group_channels.size(); ++ii )
	 {
	   for ( int jj = 0; jj < nparm; ++jj  )
	     {
	       QString chan_name  = ap2DSA->parms.at( jj ).channel;
	       if ( chan_name == group_channels[ ii ] )
		 {
		   ap2DSA->parms.replace( jj, reference_2dsa_parms );
		   //restore channel name:
		   ap2DSA->parms[ jj ].channel = chan_name;
		 }
	     }
	 }
     }
   //END of replicates processing for 2DSA
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
   if      ( type == "channels" )       { value = sl_chnsel; }
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

// Initialize a PCSA panel, especially after clicking on its tab
void US_AnaprofPanPCSA::initPanel()
{
  apPCSA             = &(mainw->currProf.apPCSA);
  //apPCSA             = &(mainw->currProf_copy.apPCSA);
  qDebug() << "INIT PCSA-begin: apPCSA.parm SIZE -- " << apPCSA->parms.size();
  //Debug
  for ( int j = 0; j < apPCSA->parms.size(); j++ )
    qDebug() << "Channels in COPY PSCA parms vector --  " <<  apPCSA->parms[ j ].channel;
  
  
   QStringList sl_bals;                         // Counterbalance choices
   sl_bals << tr( "empty (counterbalance)" )
           << tr( "Beckman counterbalance" )
           << tr( "Titanium counterbalance" )
           << tr( "Fluorescence 5-channel counterbalance" );

   // Possibly rebuild Cells protocol if there was a rotor change
//   rebuild_Cells();

   sl_chnsel       = sibLValue( "general", "channels" );
   QStringList chnls_to_run  = sibLValue( "general", "analysisrun" );
   qDebug() << "PCSA: QStringList chnls_to_run: " << chnls_to_run;

   //chnls_to_run.clear();
   
   
   /*
   // //ALEXEY: remove those channels that are not selected for RUN
   // qobject_cast<QListView *>(cb_chnsel->view())->setRowHidden(0, true);
   QStringList chnls_to_run  = sibLValue( "general", "analysisrun" );
   qDebug() << "Size of chnls_to_run: " <<  chnls_to_run.size();
   qDebug() << "QStringList chnls_to_run: " << chnls_to_run;

   QStringList chnls_to_remove;
   for ( int i = 0; i < chnls_to_run.size(); i++ )
   {
     if ( ! chnls_to_run[ i ].toInt() )
       {
	 chnls_to_remove << sl_chnsel[ i ];
	 qDebug() << "Chnls_to_remove -- " << chnls_to_remove;
       }
   }

   
   for ( int i = 0; i < chnls_to_remove.size(); i++ )
     sl_chnsel.removeOne( chnls_to_remove[i] );
   

   qDebug() << "New size of sl_chnsel is: " << sl_chnsel.size();
   // End of channels removal
   */
   
   cb_chnsel->clear();
   cb_chnsel->addItems( sl_chnsel );

   active_items_pcsa.clear();
   QStandardItemModel * model = qobject_cast<QStandardItemModel*>(cb_chnsel->model());
   for ( int i = 0; i < chnls_to_run.size(); i++ )
     {
       QStandardItem * item = model->item( i );
       if ( ! chnls_to_run[ i ].toInt() )  //deactivate item
	 item->setEnabled( false );
       else                                //activate item
	 {
	   item->setEnabled( true );
	   active_items_pcsa.push_back( i );
	 }
     }

   int first_avail = 0;
   if ( active_items_pcsa.size() )
     first_avail = active_items_pcsa[ 0 ];
   
   

   // Populate GUI settings from protocol controls
   DbgLv(1) << "APpc:inP: prb: nchan" << apPCSA->nchan << "job_run" << apPCSA->job_run;
   ck_nopcsa->setChecked( ! apPCSA->job_run );

   /*
   //int kparm          = apPCSA->parms.size();
   qDebug() << "PCSA params: OLD size/content: " << apPCSA->parms.size();

   //ALEXEY: now we need to remove PCSA params for removed channels:
   for ( int i = 0; i < chnls_to_remove.size(); i++ )
     {
       for ( int j = 0; j < apPCSA->parms.size(); j++ )
	 {
	   if ( apPCSA->parms[ j ].channel == chnls_to_remove[ i ] )
	     {
	       qDebug() << "PCSA parms for channel -- " << apPCSA->parms[ j ].channel << " will be removed!";
	       apPCSA->parms.remove( j );
	        break;
	     }
	 }
     }

   //Debug
   for ( int j = 0; j < apPCSA->parms.size(); j++ )
     {
       qDebug() << "Channels in NEW PSCA parms vector --  " <<  apPCSA->parms[ j ].channel;
     }
   //End Debug

   qDebug() << "PCSA params: NEW size/content: " << apPCSA->parms.size();
   //End of PCSA parm removal
   */
   
   int kparm          = apPCSA->parms.size();
   
   int kchan          = sl_chnsel.size();
DbgLv(1) << "APpc:inP:   kparm kchan" << kparm << kchan
 << "  job_run" << apPCSA->job_run;
   US_AnaProfile::AnaProfPCSA::ParmPCSA parm1;
   if ( kparm > 0 )
     //parm1              = apPCSA->parms[ 0 ];
     parm1              = apPCSA->parms[ first_avail ];
   for ( int ii = kparm; ii < kchan; ii++ )
   {
      parm1.channel      = sl_chnsel[ ii ];
DbgLv(1) << "APpc:inP:     set-parm ii" << ii << "channel" << parm1.channel;
      apPCSA->parms << parm1;
   }

   // cchx               = 0;
   // parms_to_gui( 0 );

   cchx               = first_avail;
   parms_to_gui( first_avail );   
DbgLv(1) << "APpc:inP:   parms_to_gui complete";
}

// Save PCSA panel controls when about to leave the panel
void US_AnaprofPanPCSA::savePanel()
{
  //ALEXEY: check syntax of xyz types:
  int syntax_errors = check_syntax_xyz();
  if ( syntax_errors != 0 )
    {
      QString mtitle_error    = tr( "Error" );
      QString message_error   = QString( tr( "There are %1 syntax errors in PCSA settings!"
					     "\n\nThe program will return to PCSA settings to fix the errors (red fields) before proceeding..." ))
	                        .arg( QString::number( syntax_errors ) );
      QMessageBox::critical( this, mtitle_error, message_error );

      emit back_to_pcsa();
      return;
    }
  //////////////////////////////////////////
  
   apPCSA             = &(mainw->currProf.apPCSA);
   int nparm          = apPCSA ->parms.count();
   apPCSA->job_run    = !ck_nopcsa->isChecked();
   apPCSA->nchan      = nparm;

   DbgLv(1) << "APpc:svP: nparm" << nparm << "cchx" << cchx
	    << "  job_run" << apPCSA->job_run;
   
   gui_to_parms( cchx );

   DbgLv(1) << "APpc:svP:   gui_to_parms complete   cchx" << cchx;

 
   //ALEXEY: here, process replicate groups defined in Gen tab:
   //1. Identify primary channel & its parameters;
   //2. Copy (replace) other's replicate group channel's parms with these reference parameters

   US_AnaProfile* currProf  = &mainw->currProf;
   QMap < int, QStringList > replicates_to_channdesc = currProf->replicates_to_channdesc_main;
   
   QMap< int, QStringList >::iterator rgi;
   for ( rgi = replicates_to_channdesc.begin(); rgi != replicates_to_channdesc.end(); ++rgi )
     {
       int group_number           = rgi.key();
       QStringList group_channels = rgi.value();

       qDebug() << "In PCSA's savePanel(): repl. number, channels -- "
		<< group_number << group_channels;
       
       if( group_number == 0 || group_channels.size() == 1 )
	    continue;

       //identify reference PCSA parms:
       US_AnaProfile::AnaProfPCSA::ParmPCSA reference_pcsa_parms;
       for ( int i = 0; i < nparm; ++i  )
	 {
	   QString chan_name  = apPCSA->parms.at( i ).channel;
	   if ( chan_name == group_channels[0] )
	     {
	       reference_pcsa_parms = apPCSA->parms.at( i );
	       break;
	     }
	 }
       //now apply ref_parms's parameters to other channels' PCSA settings:
       qDebug() << "Reference PCSA parms for channel " << group_channels[0] << reference_pcsa_parms.channel;
       for( int ii = 1; ii < group_channels.size(); ++ii )
	 {
	   for ( int jj = 0; jj < nparm; ++jj  )
	     {
	       QString chan_name  = apPCSA->parms.at( jj ).channel;
	       if ( chan_name == group_channels[ ii ] )
		 {
		   apPCSA->parms.replace( jj, reference_pcsa_parms );
		   //restore channel name:
		   apPCSA->parms[ jj ].channel = chan_name;
		 }
	     }
	 }
     }
   //END of replicates processing for PCSA
}

// Get a specific panel value
QString US_AnaprofPanPCSA::getSValue( const QString type )
{
DbgLv(1) << "APpc:getSV: type" << type;
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
DbgLv(1) << "APpc:getSV: type" << type << "value" << value;

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

   if      ( type == "channels" )       { value = sl_chnsel; }
   else if ( type == "centerpieces" )
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

