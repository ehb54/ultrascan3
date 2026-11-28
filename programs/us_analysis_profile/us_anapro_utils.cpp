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

#define setCbCurrentText(a,b)  a->setCurrentText(b)

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
   US_AnaProfile                 profile = *currProf;
   US_AnaProfileLoader::Channels channels;
   const QString                 error = US_AnaProfileLoader::prepare( profile, mainw->abde_mode_aprofile, channels );
   if ( !error.isEmpty() )
   {
      QMessageBox::warning( this, tr( "Analysis Profile" ), error );
      return;
   }
   *currProf = profile;
   showProfile( channels );
   savePanel();
   check_user_level();
}

// Show the profile, with the values of its Run and Report boxes
void US_AnaprofPanGen::showProfile( const US_AnaProfileLoader::Channels& channels )
{
   le_protname->setText( currProf->protoname );
   le_aproname->setText( currProf->aprofname );

   values    = channels;
   sl_chnsel = US_AnaProfileLoader::channels( *currProf );

   // Reset layout
   delete genL;
   genL = new QGridLayout();
   genL->setObjectName( "GeneralLayout" );
   build_general_layout();

   internal_reports = currProf->ch_reports;

   for ( int ii = 0; ii < le_channs.count(); ii++ )
   {
      le_channs[ii]->setText( sl_chnsel[ii] );
      le_lcrats[ii]->setText( QString::number( currProf->lc_ratios[ii] ) );
      le_lctols[ii]->setText( QString::number( currProf->lc_tolers[ii] ) );
      le_ldvols[ii]->setText( QString::number( currProf->l_volumes[ii] ) );
      le_lvtols[ii]->setText( QString::number( currProf->lv_tolers[ii] ) );
      le_daends[ii]->setText( QString::number( currProf->data_ends[ii] ) );
      //abde
      le_dens0s[ii]->setText( QString::number( currProf->ld_dens_0s[ii] ) );
      le_vbars[ii]->setText( QString::number( currProf->gm_vbars[ii] ) );
      le_MWs[ii]->setText( QString::number( currProf->gm_mws[ii] ) );

      sb_repl_groups[ii]->setValue( currProf->replicates[ii] );

      // Wavelength to edit, and wavelengths that are not analyzed
      const QList<QRadioButton*> buttons = gr_mwvbox[ii]->findChildren<QRadioButton*>();
      const QList<QCheckBox*>    boxes   = gr_mwvbox[ii]->findChildren<QCheckBox*>();
      const QStringList          not_run = currProf->wvl_not_run[ii].split( ":" );
      const int                  edit    = values.edit[ii];
      buttons[edit]->setChecked( true );
      buttons[edit]->click();
      boxes[edit]->setEnabled( false );
      for ( QCheckBox* box : boxes )
      {
         if ( not_run.contains( box->objectName().split( ":" )[2] ) )
         {
            box->setChecked( false );
         }
      }
   }

   showChannels();
   if ( values.run.contains( 0 ) )
   {
      showTabs();
   }

   //Also show MWL prefs for the first channel to fill the space
   ck_mwv[0]->setChecked( ck_runs[0]->isChecked() );
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
   // Populate protocol controls from GUI settings
   currProf->protoname = le_protname->text();
   currProf->aprofname = le_aproname->text();

   currProf->lc_ratios.clear();
   currProf->lc_tolers.clear();
   currProf->l_volumes.clear();
   currProf->lv_tolers.clear();
   currProf->data_ends.clear();
   currProf->ld_dens_0s.clear();
   currProf->gm_vbars.clear();
   currProf->gm_mws.clear();
   currProf->ref_use_channels.clear();
   currProf->replicates.clear();
   currProf->wvl_edit.clear();
   currProf->wvl_not_run.clear();
   currProf->ch_reports.clear();

   currProf->analysis_run = values.run;
   currProf->report_run   = values.report;
   currProf->ref_channels = values.ref;

   for ( int ii = 0; ii < ck_runs.count(); ii++ )
   {
      currProf->lc_ratios << le_lcrats[ii]->text().toDouble();
      currProf->lc_tolers << le_lctols[ii]->text().toDouble();
      currProf->l_volumes << le_ldvols[ii]->text().toDouble();
      currProf->lv_tolers << le_lvtols[ii]->text().toDouble();
      currProf->data_ends << le_daends[ii]->text().toDouble();
      //abde
      currProf->ld_dens_0s << le_dens0s[ii]->text().toDouble();
      currProf->gm_vbars << le_vbars[ii]->text().toDouble();
      currProf->gm_mws << le_MWs[ii]->text().toDouble();
      currProf->ref_use_channels << sb_use_ref_chs[ii]->value();
      currProf->replicates << sb_repl_groups[ii]->value();

      // Wavelength to edit, and wavelengths that are not analyzed
      QScrollArea* sa = gr_mwvbox[ii];
      foreach ( QRadioButton* button, sa->findChildren<QRadioButton*>() )
      {
         if ( button->isChecked() )
         {
            currProf->wvl_edit << button->objectName().split( ":" )[2].toInt();
            break;
         }
      }
      QStringList not_run;
      foreach ( QCheckBox* ckbox, sa->findChildren<QCheckBox*>() )
      {
         if ( !ckbox->isChecked() )
         {
            not_run << ckbox->objectName().split( ":" )[2];
         }
      }
      currProf->wvl_not_run << not_run.join( ":" );

      const QString channel         = currProf->chndescs_alt[ii];
      currProf->ch_reports[channel] = internal_reports[channel];
   }

   US_AnaProfileLoader::finish( *currProf );
   internal_reports = currProf->ch_reports;
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

   // for VELOCITY-MWL: 2DSA-MC is not run and cannot be selected.
   // Otherwise leave the checkbox as restored from the profile above
   // (do NOT force it back to checked), and just make sure it is editable.
   bool vel_mwl_mode = mainw->velmwl_mode_aprofile;
   if ( vel_mwl_mode )
   {
      ck_j5run->setChecked( false );
   }
   ck_j5run->setEnabled( !vel_mwl_mode );
   le_j5iter->setEnabled( !vel_mwl_mode );
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

// for VELOCITY-MWL: PCSA is not included and cannot be selected.
// Otherwise leave the checkbox as restored from the profile above
// (do NOT force it back to unchecked), and just make sure it is editable.
bool vel_mwl_mode = mainw->velmwl_mode_aprofile;
if ( vel_mwl_mode )
{
   ck_nopcsa->setChecked( true );
}
ck_nopcsa->setEnabled( !vel_mwl_mode );
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

