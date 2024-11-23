//! \file us_analysis_profile.cp

#include "us_analysis_profile.h"
#include "us_table.h"
#include "us_license.h"
#include "us_datafiles.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_report_gmp.h"

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
   abde_mode_aprofile  = false;

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

   connect( apanPCSA, SIGNAL( back_to_pcsa () ), this, SLOT( back_to_pcsa() )  );

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

   // //test
   // US_AnaProfile profile1;
   // US_AnaProfile profile2;

   // if ( profile1 == profile2 )
   //   qDebug() << "ana profs EQUAL";
   // else
   //   qDebug() << "ana profs NOT EQUAL";
}


void US_AnalysisProfileGui::back_to_pcsa( void )
{
  if ( tabWidget-> currentIndex() != 2 )
    {
      tabWidget-> setCurrentIndex( 2 ) ;
      return;
    }
    
  emit back_to_pcsa_signal();
}

// Reset parameters to their defaults
void US_AnalysisProfileGui::reset( void )
{
DbgLv(1) << "MAIN:  Resetting internal protocol...";
   currProf = US_AnaProfile();
   loadProf = US_AnaProfile();
   initPanels();                      //if commented and nothing else done causes crash

   ap_xml.clear();
   
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
//      apanGeneral->pass_names( p_protname, p_aproname );
   }
   apanGeneral->pass_names( p_protname, p_aproname );
DbgLv(1) << "APG: pass_names" << p_protname << p_aproname;
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

   //ALEXEY: pass scanCount && scanCount_global from protocol
   // scanCount     = iProto->scanCount;
   // scanCount_int = iProto->scanCount_int;
   scanCount     = iProto->rpSpeed.ssteps[ 0 ].scancount;
   scanCount_int = iProto->rpSpeed.ssteps[ 0 ].scancount_int;

   qDebug() << "In inherit: ScanCounts : "
	    << iProto->scanCount
	    << iProto->scanCount_int
	    << iProto->rpSpeed.ssteps[ 0 ].scancount
	    << iProto->rpSpeed.ssteps[ 0 ].scancount_int;
   

DbgLv(1) << "APG00: ipro: kchn nchs ncho" << kchn << nchs << ncho;

//currProf.ch_wvls.clear();
      
   if ( nchs < 1  ||  ncho < 1 )
     return;

   currProf.ch_wvls.clear();
   
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
      if ( scan1.isEmpty()  &&
           scan2.isEmpty()  &&
           scan3.isEmpty() )  continue;

      QStringList ods;       // Optics descriptions, this channel
      chname          = QString( chname ).section( ":", 0, 0 )
                                         .section( ",", 0, 0 )
                                         .replace( " / ", "" );
      if ( ! scan1.isEmpty() )
         ods << scan1;       // First optics
      if ( ! scan2.isEmpty() )
         ods << scan2;       // Additional optics
      if ( ! scan3.isEmpty() )
         ods << scan3;       // Additional optics
      int chx         = sl_chns.indexOf( chname );
      if ( chx < 0 )          continue;

      QString sodesc  = sl_sols[ chx ];  // Channel's solution
      
      //ALEXEY: also Ranges
      QList< double > wvls;
      for ( int ch_r = 0; ch_r < iProto->rpRange.chrngs.size(); ++ch_r )
	{
	  QString chdesc_from_ranges = (iProto->rpRange.chrngs[ ch_r ].channel).split(",")[0];
	  chdesc_from_ranges.replace(" / ","");

	  qDebug() << " chdesc_from_ranges, chname_rpOptics  -- " << chdesc_from_ranges << chname;

	  if ( chname.contains(chdesc_from_ranges) )
	    {
	      //QList< double > wvls = iProto->rpRange.chrngs[ ii ].wvlens;
	      wvls = iProto->rpRange.chrngs[ ch_r ].wvlens;

	      qDebug() << " wvls -- " << wvls;
	    }
	}

      // //ALEXEY: also US_ReportGMP (blank/default for now)
      // US_ReportGMP reportGMP = US_ReportGMP();
      
      
      for ( int jj = 0; jj < ods.count(); jj++ )
      {  // Create a channel entry for each optics type of this channel
         QString opdesc  = ods[ jj ];
         opdesc          = opdesc.replace( "visible", "vis." );
         opdesc          = opdesc.replace( "Rayleigh Interference", "Interf." );
         opdesc          = opdesc.replace( "Fluorescence", "Fluor." );
         QString chentr  = chname + ":" + opdesc + ":" + sodesc;
DbgLv(1) << "APG: ipro:    o.jj" << jj << "chentr" << chentr;

        //ALEXEY: chname for currProf.ch_wvls:
        QString sodesc_cut  = QString( chentr ).section( ":", 2, 2 );
        QString chentr_wvls = chname + ":" + opdesc + ":" + sodesc_cut;
	
	
         if ( nchn < kchn )
         {  // Replace channel and channel description

	   qDebug() << "inheritprotocol, Replacing channel..";
	   qDebug() << "currProf.chndescs_alt[ nchn ], chentr_wvls -- "
		    << currProf.chndescs_alt[ nchn ] << chentr_wvls;

	   qDebug() << "Old wvl -- " << currProf.ch_wvls[ currProf.chndescs_alt[ nchn ] ];
	    QString old_desc = currProf.chndescs_alt[ nchn ];
	    QString old_wvl;
	    
	    //DEBUG: check how current ch_reports looks like
	    QMap< QString, QMap < QString, US_ReportGMP > >::iterator ri;
	    for ( ri = currProf.ch_reports.begin(); ri != currProf.ch_reports.end(); ++ri )
	      {
		QString chan_desc = ri.key();

		qDebug() << "Old ch_reports: chan_desc:: " << chan_desc;
		
		QMap < QString, US_ReportGMP > triple_reports = ri.value();
		QMap < QString, US_ReportGMP >::iterator tri;
		for ( tri = triple_reports.begin(); tri != triple_reports.end(); ++tri )
		  {
		    old_wvl = tri.key();
		    
		    for(int ii=0; ii< currProf.ch_reports[ chan_desc ] [ old_wvl ].reportItems.size(); ++ii)
		      {
			US_ReportGMP::ReportItem initItem = currProf.ch_reports[ chan_desc ] [ old_wvl ].reportItems[ ii ];
			qDebug() << "wvl: type, method, lo, hi -- "
				 << old_wvl << ": " 
				 << initItem.type
				 << initItem.method
				 << initItem.range_low
				 << initItem.range_high ;
			
		      }
		  }
	      }
	    //END DEBUG
	    
            currProf.pchans  [ nchn ] = chname;
            currProf.chndescs[ nchn ] = chentr;
	    currProf.chndescs_alt[ nchn ] = chentr_wvls;

	    // //Fill out Excl. Scans QMap:
	    // QStringList scan_ranges = { "0", "0" };
	    // currProf.ch_scans_excl[ chentr_wvls ] = scan_ranges;

	    //ALEXEY: also ranges for each channel
	    if ( opdesc.contains("vis.") )
	      currProf.ch_wvls[ chentr_wvls ] = wvls;
	    if ( opdesc.contains("Interf.") )
	      {
		QList< double > wvl_interf = { 660 };
		currProf.ch_wvls[ chentr_wvls ] = wvl_interf;
	      }
	    qDebug() << "In inherit_prot: [new] wvl, wvls.size() for channel -- "
		     << currProf.ch_wvls[ chentr_wvls ]
		     << currProf.ch_wvls[ chentr_wvls ].size() << " for " << chentr_wvls;

	    // //ALEXEY: insert ch_reports here
	    // //currProf.ch_reports[ chentr_wvls ] = reportGMP;
	    // currProf.ch_reports[ chentr_wvls ] = currProf.ch_reports[ currProf.chndescs_alt[ nchn ] ];
	    // currProf.ch_reports[ chentr_wvls ].channel_name = chentr_wvls;

	    //ALEXEY_NEW_REPORT: move these 2 lines below setting currProf.ch_wvls[ chentr_wvls ]
	    //Next, iterate over wvls & fill QMap< QString, QMap < QString( wvls[i]), US_ReportGMP > > currProf.ch_reports[ chentr_wvls ][wvl]  
	    QList< double > ch_wavelengths = currProf.ch_wvls[ chentr_wvls ];

	    for ( int i=0; i<ch_wavelengths.size(); ++i )
	      {
		QString c_wvl = QString::number ( ch_wavelengths[ i ] );
		currProf.ch_reports[ chentr_wvls ][ c_wvl ] = currProf.ch_reports[ currProf.chndescs_alt[ nchn ] ][ c_wvl ];
		
		//currProf.ch_reports[ chentr_wvls ][ c_wvl ] = currProf.ch_reports[ old_desc ][ old_wvl ];
		currProf.ch_reports[ chentr_wvls ][ c_wvl ].channel_name = chentr_wvls;

		qDebug() << "nchn < kchn: Filling currProf.ch_reports -- ";
		qDebug() << "chentr_wvls, c_wvl -- " << chentr_wvls << ", " << c_wvl;
		
		//set different default params for Interf. reportGMP
		if ( chentr_wvls.contains("Interf.") )
		  {
		    if ( currProf.ch_reports[ chentr_wvls ][ c_wvl ].DBread || currProf.ch_reports[ chentr_wvls ][ c_wvl ].interf_report_changed )
		      {
			qDebug() << "nchn < kchn: IP Report: read from DB";
		      }
		    else
		      {
			currProf.ch_reports[ chentr_wvls ][ c_wvl ].tot_conc = 3.0;
			currProf.ch_reports[ chentr_wvls ][ c_wvl ].reportItems.clear();
		      }
		  }

		double tot_time_exp = iProto->rpSpeed.ssteps[0].duration + iProto->rpSpeed.ssteps[0].delay_stage;

		if ( currProf.ch_reports[ chentr_wvls ][ c_wvl ].DBread || currProf.ch_reports[ chentr_wvls ][ c_wvl ].exp_time_changed )
		  {
		  }
		else
		  currProf.ch_reports[ chentr_wvls ][ c_wvl ].experiment_duration = tot_time_exp;      // <== 1st speed step!!!

		qDebug() << "Exp. Duraiton -- " << tot_time_exp;

	      }
		
	    chx             = currProf.lc_ratios.count() - 1;
	    if ( chx < nchn )
	      {
		currProf.lc_ratios << currProf.lc_ratios[ chx ];
               currProf.lc_tolers << currProf.lc_tolers[ chx ];
               currProf.l_volumes << currProf.l_volumes[ chx ];
               currProf.lv_tolers << currProf.lv_tolers[ chx ];
               currProf.data_ends << currProf.data_ends[ chx ];

	       //abde
	       currProf.ld_dens_0s << currProf.ld_dens_0s[ chx ];
	       currProf.gm_vbars   << currProf.gm_vbars[ chx ];
	       currProf.gm_mws     << currProf.gm_mws[ chx ];
	       currProf.ref_channels << currProf.ref_channels[ chx ];
	       currProf.ref_use_channels << currProf.ref_use_channels[ chx ];

	       currProf.analysis_run << currProf.analysis_run[ chx ];
	       currProf.report_run   << currProf.report_run[ chx ];
	       currProf.wvl_edit     << currProf.wvl_edit[ chx ];
	       currProf.wvl_not_run  << currProf.wvl_not_run[ chx ];

	       currProf.scan_excl_begin  << currProf.scan_excl_begin[ chx ];
	       currProf.scan_excl_end    << currProf.scan_excl_end[ chx ];
	       
	       currProf.replicates << currProf.replicates[ chx ];
	      }
DbgLv(1) << "APG: ipro:     chx nchn dae" << chx << nchn
	 << "dae size" << currProf.data_ends.count() << "chentr" << chentr
	 << "currProf.analysis_run[ chx ]" << currProf.analysis_run[ chx ]
	 << "currProf.scan_excl_begin[ chx ]" << currProf.scan_excl_begin[ chx ]
	 << "currProf.scan_excl_end[ chx ]" << currProf.scan_excl_end[ chx ];


         }
         else
         {  // Append channel and channel description

	   qDebug() << "inheritprotocol, Appending channels..";
            currProf.pchans   << chname;
            currProf.chndescs << chentr;
	    currProf.chndescs_alt << chentr_wvls;
	    
	    
	    //ALEXEY: also ranges for each channel
	    if ( opdesc.contains("vis.") )
	      currProf.ch_wvls[ chentr_wvls ] = wvls;
	    if ( opdesc.contains("Interf.") )
	      {
		QList< double > wvl_interf = { 660 };
		currProf.ch_wvls[ chentr_wvls ] = wvl_interf;
	      }
	    qDebug() << "In inherit_prot: wvls.size() for channel -- " << currProf.ch_wvls[ chentr_wvls ].size() << " for " << chentr_wvls;

	    
	    // //ALEXEY: insert ch_reports here
	    // //currProf.ch_reports[ chentr_wvls ] = reportGMP;
	    // currProf.ch_reports[ chentr_wvls ] = currProf.ch_reports[ currProf.chndescs_alt.last() ];
	    // currProf.ch_reports[ chentr_wvls ].channel_name = chentr_wvls;

	    //ALEXEY_NEW_REPORT: move these 2 lines below setting currProf.ch_wvls[ chentr_wvls ]
	    //Next, iterate over wvls & fill QMap< QString, QMap < QString( wvls[i]), US_ReportGMP > > currProf.ch_reports[ chentr_wvls ][wvl]  
	    QList< double > ch_wavelengths = currProf.ch_wvls[ chentr_wvls ];
	    
	    for ( int i=0; i<ch_wavelengths.size(); ++i )
	      {
		QString c_wvl = QString::number ( ch_wavelengths[ i ] );
		currProf.ch_reports[ chentr_wvls ][ c_wvl ] = currProf.ch_reports[ currProf.chndescs_alt[ nchn ] ][ c_wvl ];
		//currProf.ch_reports[ chentr_wvls ][ c_wvl ] = currProf.ch_reports[ currProf.chndescs_alt[ nchn - 1 ] ][ old_wvl ];
		currProf.ch_reports[ chentr_wvls ][ c_wvl ].channel_name = chentr_wvls;

		qDebug() << "nchn > kchn: Filling currProf.ch_reports -- ";
		qDebug() << "chentr_wvls, c_wvl -- " << chentr_wvls << ", " << c_wvl;

		//set different default params for Interf. reportGMP
		if ( chentr_wvls.contains("Interf.") )
		  {
		    if ( currProf.ch_reports[ chentr_wvls ][ c_wvl ].DBread || currProf.ch_reports[ chentr_wvls ][ c_wvl ].interf_report_changed )
		      {
			qDebug() << "nchn > kchn: IP Report: read from DB";
		      }
		    else
		      {
			currProf.ch_reports[ chentr_wvls ][ c_wvl ].tot_conc = 3.0;
			currProf.ch_reports[ chentr_wvls ][ c_wvl ].reportItems.clear();
		      }
		  }
		
		
		double tot_time_exp = iProto->rpSpeed.ssteps[0].duration + iProto->rpSpeed.ssteps[0].delay_stage;

		if ( currProf.ch_reports[ chentr_wvls ][ c_wvl ].DBread || currProf.ch_reports[ chentr_wvls ][ c_wvl ].exp_time_changed )
		  {
		  }
		else
		  currProf.ch_reports[ chentr_wvls ][ c_wvl ].experiment_duration = tot_time_exp;      // <== 1st speed step!!! 

		qDebug() << "Exp. Duraiton -- " << tot_time_exp;

	      }
	    	    
	    
	    
            int lch         = nchn - 1;
            // Duplicate previous parameter values
            currProf.lc_ratios << currProf.lc_ratios[ lch ];
            currProf.lc_tolers << currProf.lc_tolers[ lch ];
            currProf.l_volumes << currProf.l_volumes[ lch ];
            currProf.lv_tolers << currProf.lv_tolers[ lch ];
            currProf.data_ends << currProf.data_ends[ lch ];

	    //abde
	    currProf.ld_dens_0s << currProf.ld_dens_0s[ lch ];
	    currProf.gm_vbars   << currProf.gm_vbars[ lch ];
	    currProf.gm_mws     << currProf.gm_mws[ lch ];
	    currProf.ref_channels << currProf.ref_channels[ lch ];
	    currProf.ref_use_channels << currProf.ref_use_channels[ lch ];
	    
	    currProf.analysis_run << currProf.analysis_run[ lch ];
	    currProf.report_run   << currProf.report_run[ lch ];
	    currProf.wvl_edit     << currProf.wvl_edit[ lch ];
	    currProf.wvl_not_run  << currProf.wvl_not_run[ lch ];

	    currProf.scan_excl_begin  << currProf.scan_excl_begin[ lch ];
	    currProf.scan_excl_end    << currProf.scan_excl_end[ lch ];
	    
	    currProf.replicates << currProf.replicates[ lch ];
	    	    
	    DbgLv(1) << "APG: ipro:     lch" << lch << "lv_tol da_end"
		     << currProf.lv_tolers[ lch ] << currProf.data_ends[ lch ]
		     << "dae size" << currProf.data_ends.count()
		     << "currProf.analysis_run[ lch ]" << currProf.analysis_run[ lch ]
		     << "currProf.scan_excl_begin[ lch ]" << currProf.scan_excl_begin[ lch ]
		     << "currProf.scan_excl_end[ lch ]" << currProf.scan_excl_end[ lch ];

         }
         nchn++;
      }
   }

   //ALEXEY:Debug
   qDebug() << "In inherit_prot: size of currProf.ch_wvls QMap -- " << currProf.ch_wvls.count();
   //////////////

   kchn            = currProf.pchans.count();
   qDebug() <<  "In inherit 1: pchans -- " << currProf.pchans;

   if ( kchn > nchn )
   {  // Drop any Profile channel values left over from previous
      for ( int ii = nchn; ii < kchn; ii++ )
      {
         currProf.pchans   .removeLast();
         currProf.chndescs .removeLast();
	 currProf.chndescs_alt .removeLast();
         currProf.lc_ratios.removeLast();
         currProf.lc_tolers.removeLast();
         currProf.l_volumes.removeLast();
         currProf.lv_tolers.removeLast();
         currProf.data_ends.removeLast();

	 qDebug() << "In inherit 1: ii, nchn, kchn " << ii << nchn << kchn;
	 
	 //abde
	 currProf.ld_dens_0s.removeLast();
	 currProf.gm_vbars.removeLast(); 
	 currProf.gm_mws .removeLast();
	 currProf.ref_channels.removeLast();
	 currProf.ref_use_channels.removeLast();

	 qDebug() << "In inherit 2: ii, nchn, kchn " << ii << nchn << kchn;

	 currProf.analysis_run .removeLast();
	 currProf.report_run   .removeLast();
	 currProf.wvl_edit     .removeLast();
	 currProf.wvl_not_run  .removeLast();

	 qDebug() << "In inherit 3: ii, nchn, kchn " << ii << nchn << kchn;
	 
	 currProf.scan_excl_begin .removeLast();
	 currProf.scan_excl_end   .removeLast();

	 qDebug() << "In inherit 4: ii, nchn, kchn " << ii << nchn << kchn;
	 currProf.replicates   .removeLast();
	 qDebug() << "In inherit 5: ii, nchn, kchn " << ii << nchn << kchn;
	 //currProf.ch_wvls.removeLast();  //ALEXEY: needed?
      }
   }

DbgLv(1) << "APG: ipro: name" << iProto->protoname
 << "cPname" << currProf.aprofname << "xml len" << ap_xml.length()
 << "GUID" << currProf.aprofGUID << iProto->rpAprof.aprofGUID;

   QString aprsrc( "currProf" );
   bool need_rec        = ap_xml.isEmpty();
   need_rec             = ( currProf.aprofname == iProto->rpAprof.aprofname )
                          ? need_rec : true;
   currProf.aprofGUID   = iProto->rpAprof.aprofGUID = iProto->protoGUID;
   currProf.aprofname   = iProto->rpAprof.aprofname = iProto->protoname;
DbgLv(1) << "APG: ipro: name GUID" << currProf.aprofname << currProf.aprofGUID;

 qDebug() << "APG: ipro: currProf.aprofname, iProto->rpAprof.aprofname, need_rec -- " << currProf.aprofname << iProto->rpAprof.aprofname << need_rec;

   // Load Analysis Profile from database or local file
   if ( need_rec )
   {
      bool use_db          = ( US_Settings::default_data_location() < 2 );
      bool fromdisk        = US_Settings::debug_match( "protocolFromDisk" );
      bool load_db         = fromdisk ? false : use_db;
      US_Passwd  pw;
      US_DB2* dbP          = load_db ? new US_DB2( pw.getPasswd() ) : NULL;
      load_db              = ( dbP != NULL );

      QString reportMask_DB;
      QString combinedPlotsParms_DB;
      
      if ( load_db )
      {  // Get AProfile XML from the database
//load_db=false;
	 
         int status           = US_DB2::OK;
         QStringList qry;
         qry << "get_aprofile_info" << currProf.aprofGUID;
         dbP->query( qry );
         status               = dbP->lastErrno();
         int numrow           = dbP->numRows();
DbgLv(1) << "APG: ipro:  db stat nrow" << status << numrow << "lerr" << dbP->lastError();
         if ( status != US_DB2::OK  ||  numrow < 1 )
         {
            load_db              = false;
DbgLv(1) << "APG: ipro:   load_db" << load_db;
         }
         else
         {
            while ( dbP->next() )
            {
               currProf.aprofID     = dbP->value( 0 ).toInt();
               currProf.aprofname   = dbP->value( 1 ).toString();
               ap_xml               = dbP->value( 2 ).toString();

	       reportMask_DB         = dbP->value( 4 ).toString();
	       combinedPlotsParms_DB = dbP->value( 5 ).toString();
            }
            aprsrc               = QString( "readDB" );
DbgLv(1) << "APG: ipro:  ap ID,name,lnxml" << currProf.aprofID
 << currProf.aprofname << ap_xml.length();

	 }
      }

DbgLv(1) << "APG: ipro:  load_db" << load_db;
      if ( !load_db )
      {  // Get AProfile XML from local disk
         bool new_file        = false;
         QString dapath       = US_Settings::dataDir() + "/projects";
         QString xmlipath     = US_DataFiles::get_filename( dapath, currProf.aprofGUID,
                                   "A", "analysis_profile", "guid", new_file );

DbgLv(1) << "APG: ipro:   new_file" << new_file << "xmlipath" << xmlipath
 << "guid" << currProf.aprofGUID;
         if ( new_file )
         {  // No local file, so clear XML
            ap_xml.clear();
            currProf.aprofID     = 0;
         }
         else
         {  // Read XML from local file
            QFile xifile( xmlipath );
            if ( xifile.open( QIODevice::ReadOnly | QIODevice::Text ) )
            {
               QTextStream xitext( &xifile );
               ap_xml               = xitext.readAll();
               xifile.close();
               aprsrc               = QString( "readLocf" );
               currProf.aprofID     = -( QString( xmlipath ).section( "/", -1, -1 )
                                                            .section( ".", -2, -2 )
                                                            .mid( 1 ).toInt() );
DbgLv(1) << "APG: ipro:    aprofID" << currProf.aprofID;
            }
         }
      }

      //Get reportMask (if not empty)
      if ( ! reportMask_DB.isEmpty() )
	currProf.report_mask = reportMask_DB;

      //Get combinedPlotsParms (if not empty)
      if ( ! combinedPlotsParms_DB.isEmpty() )
	currProf.combPlots_parms = combinedPlotsParms_DB;
	
      // Get AnalysisProfile from XML
DbgLv(1) << "APG: ipro:  ap_xml isEmpty" << ap_xml.isEmpty();
      if ( !ap_xml.isEmpty() )
      {
DbgLv(1) << "APG: ipro:  ap_xml length" << ap_xml.length();
         QXmlStreamReader xmli( ap_xml );
         currProf.fromXml( xmli );
	 
	 DbgLv(1) << "APG: ipro:   2kparm pkparm" << currProf.ap2DSA.parms.count() << currProf.apPCSA.parms.count()
		  << "kpchan" << currProf.pchans.count();
	 //         initPanels();

	 //Check here if ch_report_ids[ channel_alt_desc ] and/or ch_report_guids[ channel_alt_desc ] are not empty
	 //Also check if ch_report_ids.values() - the IDs - are not 0!!!

	 //ALEXEY_NEW_REPORT:
	 // currProf.ch_report_ids IS QMap< QString, QList < int > >:
	 // 
	 
	 // QList< int > reportIDs_list  = currProf.ch_report_ids.values();
	 // bool null_reportID = false;
	 // for (int i=0; i<reportIDs_list.size(); ++i)
	 //   {
	 //     if( reportIDs_list[i] <= 0 ) //reportid is -1 or 0, so no report associated with the channel
	 //       {
	 // 	 null_reportID = true;
	 // 	 break;
	 //       }
	 //   }
	 
	 // if ( currProf.ch_report_ids.size() && !null_reportID )
	 if ( currProf.ch_report_ids.size() )
	   {
	     //fill in currProf->ch_reports[ chdesc_alt ] by reading ReportIDs from ch_report_ids[ chdesc_alt ]
	     
	     //Also, clean  currProf->ch_reports first  (since it will be re-created form DB here )
	     currProf.ch_reports.clear();  // <-- Will this cause crash - as this omits "B:Interf." cannels ??

	     // iterate over QMap ch_report_ids[ chdesc_alt ]
	     QMap<QString, QList <int >>::iterator ri;
	     for ( ri = currProf.ch_report_ids.begin(); ri != currProf.ch_report_ids.end(); ++ri )
	       {
		 QString channel_alt_desc = ri.key();
		 //int     reportID         = ri.value();

		 QList< int > reportIDs  = ri.value();
		 
		 //ALEXEY_NEW_REPORT: here ri.value() will be QList/QVector of reportIDs for a given channel
		 //next will be cycle over this QList... 

		 QString wvl_read_abde;
		 for (int rid = 0; rid < reportIDs.size(); ++rid )
		   {
		     int reportID = reportIDs[ rid ];
		     qDebug() << "Reading reportItems form DB: channel_alt_desc, reportID  -- " << channel_alt_desc << ", " << reportID;
		 
		     //retrieve report from DB
		     US_ReportGMP * reportFromDB = new US_ReportGMP();
		     get_report_by_ID( reportFromDB, reportID );
		     
		     
		     //ALEXEY_NEW_REPORT: here the QMap< QString, QMap < QString(wvl), US_ReportGMP > > currProf.ch_reports will be filled
		     //Something like currProf.ch_reports[ channel_alt_desc ][ wvl ] -- where wvl == *reportFromDB->wavelength;
		     // OR  wvl is returned from get_report_by_ID( reportFromDB, reportID );

		     QString wvl_read = QString::number( reportFromDB->wavelength );

		     if ( reportIDs.size() == 1 )
		       wvl_read_abde = wvl_read;
		     
		     //assign retieved report to currProf.ch_reports[ channel_alt_desc ];
		     currProf.ch_reports[ channel_alt_desc ][ wvl_read ] = *reportFromDB;

		     qDebug() << "Filling currProf.ch_reports from DB: channel_alt_desc, wvl_read -- " << channel_alt_desc << ", " << wvl_read;
		   }

		 //In case of ABDE: the above loop would contain only 1 report (in DB: 1 report per channel [1st wvl])
		 //For internal consistency, we need to populate (COPY) reports for the rest of wvls in the channel (will not be used)
		 QList< double > ch_wavelengths = currProf.ch_wvls[ channel_alt_desc ];
		 if ( abde_mode_aprofile
		      && reportIDs.size() == 1
		      && ch_wavelengths.size() > 1
		      && !wvl_read_abde.isEmpty() )
		   {
		     qDebug() << "AProfile[ABDE]: reading from DB: replicating reports for MWL based on the 1st channel's wvl -- ";
		     for ( int i=0; i<ch_wavelengths.size(); ++i )
		       {
			 QString c_wvl = QString::number ( ch_wavelengths[ i ] );
			 if ( c_wvl == wvl_read_abde )
			   {
			     qDebug() << "Skipping 1st wvl: wvl = " << wvl_read_abde;
			     continue;
			   }
			 
			 qDebug() << "Filling wvl = " << c_wvl << ", with ref_wvl = " << wvl_read_abde;
			 currProf.ch_reports[ channel_alt_desc ][ c_wvl ] = currProf.ch_reports[ channel_alt_desc ][ wvl_read_abde ];
		       }
		   }
		 //END of replicating ch_reports for ABDE
	       }
	   }
      }
   }

   apanGeneral->initPanel();

   // If currProf purely from upstream profile, insure proper channel descriptions
   if ( aprsrc == "currProf" )
   {
      QStringList sl_chnsel_g = childLValue( "general", "channels" );
      QStringList sl_chnsel_2 = childLValue( "2dsa",    "channels" );
      QStringList sl_chnsel_p = childLValue( "pcsa",    "channels" );
      QStringList pa_chnsel_p;
      QStringList pa_chnsel_2;
      int nchan_g     = sl_chnsel_g.count();
      int nchan_2     = sl_chnsel_2.count();
      int nchan_p     = sl_chnsel_p.count();
      int nchan_p2    = currProf.ap2DSA.parms.count();
      int nchan_pp    = currProf.apPCSA.parms.count();
      bool need_reset = false;
      if ( nchan_g == nchan_2  &&  nchan_g == nchan_p )
      {  // Channels same size, so check contents
         for ( int ii = 0; ii < nchan_g; ii++ )
         {
            if ( sl_chnsel_g[ ii ] != sl_chnsel_2[ ii ]  ||
                 sl_chnsel_g[ ii ] != sl_chnsel_p[ ii ] )
               need_reset      = true;
         }
      }
      else
      {  // Channels size difference, so mark need to reset
         need_reset      = true;
         //   and effectively resize the string lists
         if ( nchan_2 < nchan_g )
         {
            QString chann_2 = sl_chnsel_2[ nchan_2 - 1];
            for ( int ii = nchan_2; ii < nchan_g; ii++ )
               sl_chnsel_2 << chann_2;
         }
         else if ( nchan_2 > nchan_g )
         {
            for ( int ii = nchan_g; ii < nchan_2; ii++ )
               sl_chnsel_2.removeLast();
         }
         if ( nchan_p < nchan_g )
         {
            QString chann_p = sl_chnsel_p[ nchan_p - 1];
            for ( int ii = nchan_p; ii < nchan_g; ii++ )
               sl_chnsel_p << chann_p;
         }
         else if ( nchan_p > nchan_g )
         {
            for ( int ii = nchan_g; ii < nchan_p; ii++ )
               sl_chnsel_p.removeLast();
         }
      }

      if ( need_reset )
      {  // Reset channel descriptions for 2DSA,PCSA
         for ( int ii = 0; ii < nchan_g; ii++ )
         {
            sl_chnsel_2[ ii ] = sl_chnsel_g[ ii ];
            sl_chnsel_p[ ii ] = sl_chnsel_g[ ii ];
         }

         apan2DSA->sl_chnsel  = sl_chnsel_2;
         apanPCSA->sl_chnsel  = sl_chnsel_p;
         nchan_2         = nchan_g;
         nchan_p         = nchan_g;
      }

      // Test need to reset 2DSA,PCSA parms channels
      need_reset      = false;
      if ( nchan_p2 == nchan_2  &&  nchan_pp == nchan_p )
      {  // Channels same size, so check contents
         for ( int ii = 0; ii < nchan_g; ii++ )
         {
            if ( currProf.ap2DSA.parms[ ii ].channel != sl_chnsel_2[ ii ]  ||
                 currProf.ap2DSA.parms[ ii ].channel != sl_chnsel_p[ ii ] )
               need_reset      = true;
         }
      }
      else
      {  // Channels size difference, so mark need to reset
         need_reset      = true;
      }
      if ( need_reset )
      {  // Reset channel descriptions for 2DSA,PCSA parms
         currProf.ap2DSA.parms.resize( nchan_2 );
         currProf.apPCSA.parms.resize( nchan_p );
         int lch_p2      = nchan_p2 - 1;
         int lch_pp      = nchan_pp - 1;

         for ( int ii = 0; ii < nchan_g; ii++ )
         {
            if ( ii >= nchan_p2 )
            {
               currProf.ap2DSA.parms[ ii ] = currProf.ap2DSA.parms[ lch_p2 ];
            }
            if ( ii >= nchan_pp )
            {
               currProf.apPCSA.parms[ ii ] = currProf.apPCSA.parms[ lch_pp ];
            }
            currProf.ap2DSA.parms[ ii ].channel = sl_chnsel_g[ ii ];
            currProf.apPCSA.parms[ ii ].channel = sl_chnsel_g[ ii ];
         }

         nchan_p2        = nchan_g;
         nchan_pp        = nchan_g;
      }
   } // END: aprsrc == "currProf"
DbgLv(1) << "APG: ipro: Aprof source: " << aprsrc;
DbgLv(1) << "APG: ipro: nchn" << nchn << "call pGen iP";

    // //ALEXEY: make copies of original currProf.ap2DSA && currProf.apPCSA objects for further editing (analysis -- o run or not to run)
    // qDebug() << "Inherit: BEFORE copying..";
    // currProf_copy.ap2DSA = currProf.ap2DSA;
    // currProf_copy.apPCSA = currProf.apPCSA;

    // qDebug() << "Inherit: AFTER copying..";


 qDebug() << "Size, Keys of currProf.ch_reports -- " << currProf.ch_reports.size() << ", " <<  currProf.ch_reports.keys();
}

//retrieve Report by ID from DB into ReportGMP structure
void US_AnalysisProfileGui::get_report_by_ID( US_ReportGMP* reportFromDB, int reportID )
{
  US_DB2* db  = NULL;
  US_Passwd pw;
  db            = new US_DB2( pw.getPasswd() );
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Reading Report: Could not connect to database: \n" ) + db->lastError() );
      return;
    }

  //Read parent Report
  QStringList qry;
  qry << "get_report_info_by_id" << QString::number( reportID );
  qDebug() << "In get_report_by_ID(): query -- " << qry;
  db->query( qry );

  QString reportMaskJson;
  
  if ( db->lastErrno() == US_DB2::OK )      
    {
      while ( db->next() )
	{
	  reportFromDB->channel_name            = db->value( 0 ).toString();  
	  reportFromDB->tot_conc                = db->value( 1 ).toString().toDouble();  
	  reportFromDB->rmsd_limit              = db->value( 2 ).toString().toDouble();
	  reportFromDB->av_intensity            = db->value( 3 ).toString().toDouble();
	  reportFromDB->experiment_duration     = db->value( 4 ).toString().toDouble();
	  reportFromDB->wavelength              = db->value( 5 ).toString().toDouble();

	  reportFromDB->tot_conc_tol            = db->value( 6 ).toString().toDouble();
	  reportFromDB->experiment_duration_tol = db->value( 7 ).toString().toDouble();

	  reportMaskJson                        = db->value( 8 ).toString();
	}

      qDebug() << "report's channelname, wvl -- " << reportFromDB->channel_name << reportFromDB->wavelength;
      qDebug() << "reportMaskJson form DB -- " << reportMaskJson;
      //process report Mask Json into US_ReportGMP fields:
      QJsonDocument jsonDoc = QJsonDocument::fromJson( reportMaskJson.toUtf8() );
      if (!jsonDoc.isObject())
	{
	  qDebug() << "All Doc: NOT a JSON Doc !!";
	}
      QJsonObject rootObj = jsonDoc.object();
      if (
          rootObj.contains("tot_conc") &&
          rootObj.contains("rmsd_lim") &&
          rootObj.contains("intensity") &&
          rootObj.contains("exp_dur") &&
          rootObj.contains("integration") &&
	  rootObj.contains("plots")
	  )
	{
	  rootObj["tot_conc"].toString().toInt() ? reportFromDB->tot_conc_mask   = true : reportFromDB->tot_conc_mask   = false;
	  rootObj["rmsd_lim"].toString().toInt() ? reportFromDB->rmsd_limit_mask = true : reportFromDB->rmsd_limit_mask = false;
	  rootObj["intensity"].toString().toInt() ? reportFromDB->av_intensity_mask = true : reportFromDB->av_intensity_mask = false;
	  rootObj["exp_dur"].toString().toInt() ? reportFromDB->experiment_duration_mask = true : reportFromDB->experiment_duration_mask = false;
	  rootObj["integration"].toString().toInt() ? reportFromDB->integration_results_mask = true : reportFromDB->integration_results_mask = false;
	  rootObj["plots"].toString().toInt() ? reportFromDB->plots_mask = true : reportFromDB->plots_mask = false;
	}
      else
	{
	  qDebug() << "No json mask set for report !\n";
	}
      if (
          rootObj.contains("pseudo3d_2dsait_s_ff0") &&
          rootObj.contains("pseudo3d_2dsait_s_d") &&
	  rootObj.contains("pseudo3d_2dsait_mw_ff0") &&
	  rootObj.contains("pseudo3d_2dsait_mw_d") &&
	  rootObj.contains("pseudo3d_2dsamc_s_ff0") &&
          rootObj.contains("pseudo3d_2dsamc_s_d") &&
	  rootObj.contains("pseudo3d_2dsamc_mw_ff0") &&
	  rootObj.contains("pseudo3d_2dsamc_mw_d") &&
	  rootObj.contains("pseudo3d_pcsa_s_ff0") &&
          rootObj.contains("pseudo3d_pcsa_s_d") &&
	  rootObj.contains("pseudo3d_pcsa_mw_ff0") &&
	  rootObj.contains("pseudo3d_pcsa_mw_d") &&
	  rootObj.contains("pseudo3d_mask") 
	  )
	{
	  rootObj["pseudo3d_2dsait_s_ff0"].toString().toInt() ? reportFromDB->pseudo3d_2dsait_s_ff0   = true : reportFromDB-> pseudo3d_2dsait_s_ff0  = false;
	  rootObj["pseudo3d_2dsait_s_d"].toString().toInt() ? reportFromDB->pseudo3d_2dsait_s_d   = true : reportFromDB-> pseudo3d_2dsait_s_d  = false;
	  rootObj["pseudo3d_2dsait_mw_ff0"].toString().toInt() ? reportFromDB->pseudo3d_2dsait_mw_ff0   = true : reportFromDB-> pseudo3d_2dsait_mw_ff0  = false;
	  rootObj["pseudo3d_2dsait_mw_d"].toString().toInt() ? reportFromDB->pseudo3d_2dsait_mw_d   = true : reportFromDB-> pseudo3d_2dsait_mw_d  = false;

	  rootObj["pseudo3d_2dsamc_s_ff0"].toString().toInt() ? reportFromDB->pseudo3d_2dsamc_s_ff0   = true : reportFromDB-> pseudo3d_2dsamc_s_ff0  = false;
	  rootObj["pseudo3d_2dsamc_s_d"].toString().toInt() ? reportFromDB->pseudo3d_2dsamc_s_d   = true : reportFromDB-> pseudo3d_2dsamc_s_d  = false;
	  rootObj["pseudo3d_2dsamc_mw_ff0"].toString().toInt() ? reportFromDB->pseudo3d_2dsamc_mw_ff0   = true : reportFromDB-> pseudo3d_2dsamc_mw_ff0  = false;
	  rootObj["pseudo3d_2dsamc_mw_d"].toString().toInt() ? reportFromDB->pseudo3d_2dsamc_mw_d   = true : reportFromDB-> pseudo3d_2dsamc_mw_d  = false;	  

	  rootObj["pseudo3d_pcsa_s_ff0"].toString().toInt() ? reportFromDB->pseudo3d_pcsa_s_ff0   = true : reportFromDB-> pseudo3d_pcsa_s_ff0  = false;
	  rootObj["pseudo3d_pcsa_s_d"].toString().toInt() ? reportFromDB->pseudo3d_pcsa_s_d   = true : reportFromDB-> pseudo3d_pcsa_s_d  = false;
	  rootObj["pseudo3d_pcsa_mw_ff0"].toString().toInt() ? reportFromDB->pseudo3d_pcsa_mw_ff0   = true : reportFromDB-> pseudo3d_pcsa_mw_ff0  = false;
	  rootObj["pseudo3d_pcsa_mw_d"].toString().toInt() ? reportFromDB->pseudo3d_pcsa_mw_d   = true : reportFromDB-> pseudo3d_pcsa_mw_d  = false;

	  rootObj["pseudo3d_mask"].toString().toInt() ? reportFromDB->pseudo3d_mask   = true : reportFromDB-> pseudo3d_mask  = false;
	  
	}
      else
	{
	  qDebug() << "No json Pseudo3D plots mask set for report !\n";
	}
      
      
      //was read from DB
      reportFromDB->DBread = true;
    }

  //Read children: reportItems-by-reportID && insert them to parent report
  reportFromDB->reportItems.clear();
  
  qry.clear();
  qry << "get_report_items_ids_by_report_id" << QString::number( reportID );
  qDebug() << "In get_report_by_ID() 1: query -- " << qry;
  db->query( qry );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no ReportItem(s) are associated with the parent Report in DB!!
    {
      QList< int > reportItemsIDs;
      
      while ( db->next() )
	{
	  int ID = db->value( 0 ).toString().toInt();
	  reportItemsIDs << ID;
	  
	  qDebug() << "ReportItemID: " << ID;
	}
      
      // reportItem information
      foreach ( int ID, reportItemsIDs )
	{
	  qry.clear();
	  qry  << QString( "get_report_item_info_by_id" )
	       << QString::number( ID );
	  db->query( qry );
	  db->next();

	  US_ReportGMP::ReportItem reportItem_read;
	  
	  reportItem_read.type              =   db->value( 0 ).toString();
	  reportItem_read.method            =   db->value( 1 ).toString();
	  reportItem_read.range_low         =   db->value( 2 ).toString().toDouble();	   
	  reportItem_read.range_high        =   db->value( 3 ).toString().toDouble();
	  reportItem_read.integration_val   =   db->value( 4 ).toString().toDouble();	   
	  reportItem_read.tolerance         =   db->value( 5 ).toString().toDouble();
	  reportItem_read.total_percent     =   db->value( 6 ).toString().toDouble();
	  reportItem_read.combined_plot     =   db->value( 7 ).toString().toInt();
	  reportItem_read.ind_combined_plot =   db->value( 8 ).toString().toInt();

	  reportItem_read.integration_val_sim   = -1;
	  reportItem_read.total_percent_sim     = -1;
	  reportItem_read.passed                = QString("N/A");
  
	  reportFromDB->reportItems.push_back( reportItem_read );
	}
      
    }
  else //No reportItem(s) for report in the DB: push default rpeortItem structure??
    {
      qDebug() << "Adding plain reportItem to channel -- " ;
      //Add plain ReportItem
      US_ReportGMP::ReportItem initItem;

      if ( !abde_mode_aprofile )
	{
	  initItem.type             = QString("s");
	  initItem.method           = QString("2DSA-IT");
	  initItem.range_low        = 3.2;
	  initItem.range_high       = 3.7;
	  initItem.integration_val  = 0.57;
	  initItem.tolerance        = 10; 
	  initItem.total_percent    = 0.58;
	  initItem.combined_plot    = 1;
	  initItem.ind_combined_plot  = 1;
	  
	  initItem.integration_val_sim   = -1;
	  initItem.total_percent_sim     = -1;
	  initItem.passed                = QString("N/A");
	}
      else
	{
	  initItem.type             = QString("Radius");
	  initItem.method           = QString("raw");
	  initItem.range_low        = 5.8;
	  initItem.range_high       = 7.0;
	  initItem.integration_val  = 0.57;
	  initItem.tolerance        = 10; 
	  initItem.total_percent    = 0.58;
	  initItem.combined_plot    = 1;
	  initItem.ind_combined_plot  = 1;
	  
	  initItem.integration_val_sim   = -1;
	  initItem.total_percent_sim     = -1;
	  initItem.passed                = QString("N/A");
	}
      
      if ( !reportFromDB->channel_name.contains("Interf.") ) 
	reportFromDB->reportItems.push_back( initItem );
    }
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
   //middle_h = new QHBoxLayout;
   //middle_h = new QGridLayout;
   middle_h = new QGridLayout;
   left     = new QVBoxLayout;
   right    = new QVBoxLayout;
   
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

   //pb_scan_excl->setObjectName( "Scan Exclusion" );

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
   bool have_genl  = true;      // Flag if a general layout exists
   int nchn        = sl_chnsel.count();
DbgLv(1) << "APGe: bgL: nchn" << nchn << "sl_chnsel" << sl_chnsel;



   int kchnl       = nchn;
   int kchnh       = nchn;
   if ( nchn > 0  &&  genL != NULL )
   {
      kchnl           = le_lcrats.count();
      kchnl           = kchnl;
      kchnl           = qMin( kchnl, le_lctols.count() );
      kchnh           = qMax( kchnh, le_lctols.count() );
      kchnl           = qMin( kchnl, le_ldvols.count() );
      kchnh           = qMax( kchnh, le_ldvols.count() );
      kchnl           = qMin( kchnl, le_lvtols.count() );
      kchnh           = qMax( kchnh, le_lvtols.count() );
      kchnl           = qMin( kchnl, le_daends.count() );
      kchnh           = qMax( kchnh, le_daends.count() );

      kchnl           = qMin( kchnl, ck_runs.count() );
      kchnh           = qMax( kchnh, ck_runs.count() );

   }
   else if ( nchn == 0 )
   {
      kchnh           = 1;
   }

   //ALEXEY:  BUG !!! Always rebuild !!!  
 //    if (  nchn == kchnl  &&  nchn == kchnh )
 //    {  // Channel elements all have the same count
 // DbgLv(1) << "APGe: bgL: REBUILD skipped";
 //       return;
 //    }

   
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
      have_genl       = false;
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
   le_channs.clear();
   le_lcrats.clear();
   le_lctols.clear();
   le_ldvols.clear();
   le_lvtols.clear();
   le_daends.clear();

   //ABDE
   le_dens0s.clear();
   le_vbars .clear();
   le_MWs   .clear();
   //sb_ref_chs. clear();
   le_ref_chs. clear();
   sb_use_ref_chs. clear();

   ck_runs       .clear();
   ck_report_runs.clear();
   pb_reports    .clear();
   sb_repl_groups.clear();
   //internal_reports.clear();

   // Start building main layout
   int row         = 0;

   // genL->addWidget( pb_aproname,     row,    0, 1, 5 );
   // genL->addWidget( le_aproname,     row++,  5, 1, 6 );
   // genL->addWidget( pb_protname,     row,    0, 1, 5 );
   // genL->addWidget( le_protname,     row++,  5, 1, 6 );

   //Button to call geneal profile mask settings (section 1 of the report PDF)
   QPushButton* pb_gen_report_sttings    = us_pushbutton( tr( "General Report Settings" ) );
   
   //Scan exclusion at the beginnig && end of the channel's scan set:
   QPushButton* pb_scan_excl    = us_pushbutton( tr( "Excluded Scans Ranges" ) );

   genL->addWidget( pb_aproname,     row,    0, 1, 3 );
   genL->addWidget( le_aproname,     row,    3, 1, 6 );
   genL->addWidget( pb_gen_report_sttings,   row++,  9, 1, 3 );
   
   genL->addWidget( pb_protname,     row,    0, 1, 3 );
   genL->addWidget( le_protname,     row,    3, 1, 6 );

   if ( !mainw->abde_mode_aprofile )
     genL->addWidget( pb_scan_excl,    row++,  9, 1, 3 );
   else
     pb_scan_excl -> setVisible( false ); row++;

   connect( pb_aproname, SIGNAL( clicked            ( ) ),
            this,        SLOT(   apro_button_clicked( ) ) );
   connect( pb_protname, SIGNAL( clicked            ( ) ),
            this,        SLOT(   prot_button_clicked( ) ) );
   connect( le_aproname, SIGNAL( editingFinished  ( void ) ),
            this,        SLOT(   apro_text_changed( void ) ) );
   connect( le_protname, SIGNAL( editingFinished  ( void ) ),
            this,        SLOT(   prot_text_changed( void ) ) );
   connect( pb_scan_excl, SIGNAL( clicked            ( ) ),
	    this,        SLOT(   set_scan_ranges( ) ) );
   connect( pb_gen_report_sttings, SIGNAL( clicked            ( ) ),
	    this,        SLOT(   set_gen_report_settings( ) ) );  
   
   // Build channel lists and rows
DbgLv(1) << "Ge:SL: nchn" << nchn << "sl_chnsel" << sl_chnsel;
   QLabel* lb_chann  = us_label( tr( "CellChannel:\n"
                                     "Optics: Solution" ) );
   lb_chann->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
   lb_lcrat  = us_label( tr( "Loading\nRatio" ) );
   lb_lctol  = us_label( tr( "+/- %\nToler." ) );
   QLabel* lb_ldvol  = us_label( tr( "Loading\nVol. (" )
                                 + QString( QChar( 181 ) ) + "l)" );
   QLabel* lb_lvtol  = us_label( tr( "+/- %\nToler." ) );
   lb_daend  = us_label( tr( "Data End\n(cm)" ) );
   QLabel* lb_channelana  = us_label( tr( "Run" ) );
   QLabel* lb_runreport  = us_label( tr( "Run\nReport" ) );
   
   QLabel* lb_report  = us_label( tr( "Report" ) );

   QLabel* lb_repl_group  = us_label( tr( "Replicate\nGroup" ) );

   //Add new widgets for ABDE case:
   lbl_dens_0 = us_label( tr( "Loading \nDensity (g/ml):" ) );
   lbl_vbar = us_label( tr( "Gradient Mat. \n vbar (ml/g):" ) );
   lbl_MW = us_label( tr( "Gradient Mat. \n MW (g/mol):" ) );
   lbl_refc = us_label( tr( "Reference?" ) );
   lbl_use_refc = us_label( tr( "Use\nReference#:" ) );
   
   //END Add new widgets for ABDE case:
   
   
   lb_mwvprefs    = us_label( tr( "MWL\nPrefs." ) );
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

   //ABDE
   lbl_dens_0->setMaximumHeight( lbhgt );
   lbl_vbar  ->setMaximumHeight( lbhgt );
   lbl_MW    ->setMaximumHeight( lbhgt );
   lbl_refc  ->setMaximumHeight( lbhgt );
   lbl_use_refc  ->setMaximumHeight( lbhgt );

   // genL->addWidget( lb_chann, row,    0, 2, 5 );
   // genL->addWidget( lb_lcrat, row,    5, 2, 1 );
   // genL->addWidget( lb_lctol, row,    6, 2, 1 );
   // genL->addWidget( lb_ldvol, row,    7, 2, 1 );
   // genL->addWidget( lb_lvtol, row,    8, 2, 1 );
   // genL->addWidget( lb_daend, row,    9, 2, 1 );
   // genL->addWidget( lb_channelana, row,  10, 2, 1 );
   // genL->addWidget( lb_report, row,   11, 2, 1 );
   // genL->addWidget( lb_mwvprefs,   row++,12, 2, 1 ); row++;

   genL->addWidget( lb_chann, row,    0, 2, 3 );
   if ( !mainw->abde_mode_aprofile )
     {
       genL->addWidget( lb_lcrat, row,    3, 2, 1 );
       genL->addWidget( lb_lctol, row,    4, 2, 1 );
       genL->addWidget( lb_ldvol, row,    5, 2, 1 );
       genL->addWidget( lb_lvtol, row,    6, 2, 1 );
       genL->addWidget( lb_daend, row,    7, 2, 1 );

       lbl_dens_0 -> setVisible( false );
       lbl_vbar   -> setVisible( false );
       lbl_MW     -> setVisible( false );
     }
   else
     {
       genL->addWidget( lbl_dens_0, row,    3, 2, 1 );
       genL->addWidget( lbl_vbar,   row,    4, 2, 1 );
       genL->addWidget( lbl_MW,     row,    5, 2, 1 );
       genL->addWidget( lb_ldvol,   row,    6, 2, 1 );
       genL->addWidget( lb_lvtol,   row,    7, 2, 1 );

       lb_lcrat -> setVisible( false );
       lb_lctol -> setVisible( false );
       lb_daend -> setVisible( false );
     }
   
   genL->addWidget( lb_channelana, row,  8, 2, 1 );
   genL->addWidget( lb_runreport, row,  9, 2, 1 );
   genL->addWidget( lb_report, row,  10, 2, 1 );
   genL->addWidget( lb_repl_group, row,  11, 2, 1 );

   if ( !mainw->abde_mode_aprofile )
     {
       genL->addWidget( lb_mwvprefs,   row++,12, 2, 1 ); row++;
       lbl_refc     -> setVisible( false );
       lbl_use_refc -> setVisible( false );
     }
   else
     {
       genL->addWidget( lbl_refc,      row,    12, 1, 1);//, Qt::AlignHCenter );
       genL->addWidget( lbl_use_refc,  row,    13, 1, 1);//, Qt::AlignHCenter );
       
       lb_mwvprefs -> setVisible( false );
       row++; row++;
     }
   
   // genL->setRowStretch( 0, 0 );
   // genL->setRowStretch( 1, 0 );

   row_global = row;
   QCheckBox*     ck_analysisrun;
   QCheckBox*     ck_reportrun;
   QPushButton*   pb_reportprefs;
   QCheckBox*     ck_mwvprefs;
   QSpinBox*      sb_repl_group;
   //QSpinBox*      sb_ref_ch;
   //QLineEdit*     le_ref_ch;
   QSpinBox*      sb_use_ref_ch;

   // Clear the right layout from QGroupboxes
   qDebug() << "Right.count(), gr_mwvbox.size() BEFORE deletion -- " << right->count() << gr_mwvbox.size();
   if ( right->layout() != NULL )
     {
       QLayoutItem* item;
       while ( ( item = right->layout()->takeAt( 0 ) ) != NULL )
	 {
	   delete item->widget();
	   delete item;
	 }
       //delete m_view->layout();
     }
   ck_mwv   .clear();
   gr_mwvbox.clear();
   //currProf->wvl_edit.clear();

   qDebug() << "Right.count(), gr_mwvbox.size() AFTER deletion -- " << right->count() << gr_mwvbox.size();
   //END clearing right
      
   for ( int ii = 0; ii < nchn; ii++ )
   {
      QString schan( sl_chnsel[ ii ] );
DbgLv(1) << "Ge:SL:  ii" << ii << "schan" << schan;
 
      QLineEdit* le_chann = us_lineedit( schan, 0, true  );
      QLineEdit* le_lcrat = us_lineedit( "1.0", 0, false );
      QLineEdit* le_lctol = us_lineedit( "5",   0, false );
      QLineEdit* le_ldvol = us_lineedit( "460", 0, false );
      QLineEdit* le_lvtol = us_lineedit( "10",  0, false );
      QLineEdit* le_daend = us_lineedit( "7.0", 0, false );
      
      QString stchan      = QString::number( ii ) + ": ";
      le_chann->setObjectName( stchan + "channel" );
      le_lcrat->setObjectName( stchan + "loadconc_ratio" );
      le_lctol->setObjectName( stchan + "loadconc_tolerance" );
      le_ldvol->setObjectName( stchan + "load_volume" );
      le_lvtol->setObjectName( stchan + "loadvol_tolerance" );
      le_daend->setObjectName( stchan + "dataend" );

      le_channs << le_chann;
      le_lcrats << le_lcrat;
      le_lctols << le_lctol;
      le_ldvols << le_ldvol;
      le_lvtols << le_lvtol;
      le_daends << le_daend;

      //Add new widgets for ABDE case:
      QLineEdit* le_dens_0 = us_lineedit( "1.42",   0, false );
      QLineEdit* le_vbar   = us_lineedit( "0.2661", 0, false );
      QLineEdit* le_MW     = us_lineedit( "168.36", 0, false );
      
      // sb_ref_ch  = us_spinbox();
      // sb_ref_ch ->setObjectName( stchan + "RefChan --chann_name--" + schan );
      // sb_ref_ch ->setMinimum( 0 );
      // sb_ref_ch ->setMaximum( int(nchn/2.0) );
      // sb_ref_ch-> setEnabled( false );

      QLineEdit *le_ref_ch  =  us_lineedit( "", 0, true  );
      le_ref_ch ->setObjectName( stchan + "RefChan --chann_name--" + schan );
      
      sb_use_ref_ch  = us_spinbox();
      sb_use_ref_ch ->setObjectName( stchan + "RefUseChan --chann_name--" + schan );
      sb_use_ref_ch ->setMinimum( 0 );
      sb_use_ref_ch ->setMaximum( int(nchn/2.0) );
      sb_use_ref_ch -> setEnabled( false );

      le_dens0s  << le_dens_0;
      le_vbars   << le_vbar;
      le_MWs     << le_MW;
      //sb_ref_chs << sb_ref_ch;
      le_ref_chs << le_ref_ch;
      sb_use_ref_chs << sb_use_ref_ch;
      //END Add new widgets for ABDE case:
      
      genL->addWidget( le_chann,  row,    0, 1, 3 );
      if ( !mainw->abde_mode_aprofile )
	{
	  genL->addWidget( le_lcrat,  row,    3, 1, 1 );
	  genL->addWidget( le_lctol,  row,    4, 1, 1 );
	  genL->addWidget( le_ldvol,  row,    5, 1, 1 );
	  genL->addWidget( le_lvtol,  row,    6, 1, 1 );
	  genL->addWidget( le_daend,  row,    7, 1, 1 );

	  le_dens_0 -> setVisible( false );
	  le_vbar   -> setVisible( false );
	  le_MW     -> setVisible( false );
	}
      else
	{
	  genL->addWidget( le_dens_0, row,    3, 1, 1 );
	  genL->addWidget( le_vbar,   row,    4, 1, 1 );
	  genL->addWidget( le_MW,     row,    5, 1, 1 );
	  genL->addWidget( le_ldvol,  row,    6, 1, 1 );
	  genL->addWidget( le_lvtol,  row,    7, 1, 1 );

	  le_lcrat -> setVisible( false );
	  le_lctol -> setVisible( false );
	  le_daend -> setVisible( false );
	}
      
      QFont font   = le_chann->property("font").value<QFont>();
      QFontMetrics fm(font);
      int pixelsWide = fm.width( le_chann->text() );
      //int pixelsHigh = fm.height();
      //pb_aproname->setMinimumWidth( pixelsWide );
      le_chann->setMinimumWidth( pixelsWide*1.1 );
      le_chann->adjustSize();
      
      //ALEXEY: add checkbox to define analysis
      //ck_analysisrun = new QCheckBox( tr("Run"), this );
      ck_analysisrun = new QCheckBox( tr(""), this );
      ck_analysisrun ->setAutoFillBackground( true );
      ck_analysisrun ->setChecked( true );
      QString strow  = QString::number( ii );
      ck_analysisrun ->setObjectName( strow + ": Run" );
      genL->addWidget( ck_analysisrun,  row,  8, 1, 1, Qt::AlignHCenter );
      connect( ck_analysisrun, SIGNAL( toggled     ( bool ) ),
               this,           SLOT  ( runChecked( bool ) ) );

      ck_runs << ck_analysisrun;
      //END of run checkbox seciton

      //Run Report checkboxes
      ck_reportrun = new QCheckBox( tr(""), this );
      ck_reportrun ->setAutoFillBackground( true );
      ck_reportrun ->setChecked( true );
      ck_reportrun ->setObjectName( strow + ": RunReport" );
      genL->addWidget( ck_reportrun,  row,  9, 1, 1, Qt::AlignHCenter );
      connect( ck_reportrun, SIGNAL( toggled     ( bool ) ),
               this,         SLOT  ( reportRunChecked( bool ) ) );

      ck_report_runs << ck_reportrun;
      //END of runReport cks
      

      //Report
      pb_reportprefs = us_pushbutton( tr( "Report" ) );
      pb_reportprefs ->setObjectName( strow + ": Report --chann_name--" + schan );
      genL->addWidget( pb_reportprefs,  row,  10, 1, 1, Qt::AlignHCenter );
      connect( pb_reportprefs, SIGNAL( clicked     ( ) ),
	       this,        SLOT  ( setReport( ) ) );


      pb_reports << pb_reportprefs;
      //End of Report

      //Replicate Groups
      sb_repl_group  = us_spinbox();
      sb_repl_group->setObjectName( strow + ": Replicate --chann_name--" + schan );
      sb_repl_group ->setMaximum( nchn );
      genL->addWidget( sb_repl_group,  row,  11, 1, 1, Qt::AlignHCenter );
      // connect( sb_repl_group, SIGNAL( clicked     ( ) ),
      //          this,        SLOT  ( setReplicate( ) ) );

      sb_repl_groups << sb_repl_group;
      
      //End Repl.groups
      
      //MWL prefs
      ck_mwvprefs = new QCheckBox( tr(""), this );
      ck_mwvprefs ->setAutoFillBackground( true );
      ck_mwvprefs ->setChecked( false );
      ck_mwvprefs ->setObjectName( strow + ": MWV" );

      connect( ck_mwvprefs, SIGNAL( toggled     ( bool ) ),
               this,        SLOT  ( mwvChecked( bool ) ) );

      ck_mwv << ck_mwvprefs;
      
      if ( !mainw->abde_mode_aprofile )
	{
	  genL->addWidget( ck_mwvprefs,  row,  12, 1, 1, Qt::AlignHCenter );

	  //sb_ref_ch     -> setVisible( false );
	  le_ref_ch     -> setVisible( false );
	  sb_use_ref_ch -> setVisible( false );
	}
      else
	{
	  //genL->addWidget( sb_ref_ch,      row,  12, 1, 1, Qt::AlignHCenter );
	  genL->addWidget( le_ref_ch,      row,  12, 1, 1);//, Qt::AlignHCenter );
	  genL->addWidget( sb_use_ref_ch,  row,  13, 1, 1, Qt::AlignHCenter );
	  
	  ck_mwvprefs -> setVisible( false );
	  ck_mwvprefs -> setChecked( false );
	}
      //END of MWL prefs
      
      if ( ii == 0 )
      {
	int appall_pos = ( mainw->abde_mode_aprofile ) ? 14 : 13;
	genL->addWidget( pb_applya, row++, appall_pos, 1, 2 );
	//genL->addWidget( pb_applya, row++, 13, 1, 2 );
	connect( pb_applya, SIGNAL( clicked       ( ) ),
		 this,      SLOT(   applied_to_all( ) ) );
      }
      else
      {
         row++;
      }

      //MWV dialog
      // //QGroupBox for Fit meniscus/bottom options
      QString ch_name_c = schan;
 
      //QList< double > wvlss = { 280, 340, 500 };
      QList< double > curr_wvls = currProf -> ch_wvls[ schan ];

      qDebug() << "In build_gen: curr_wvls.size() for schan -- " << curr_wvls.size() << " for " << schan;

      QString ch_name = ch_name_c.split(":")[0] + " : " + ch_name_c.split(":")[1];

      qDebug() << "In build_gen: ch_name -- " << ch_name;
      
      QGroupBox * wvl_box = createGroup( ch_name,  curr_wvls );
      scrollArea_r      = new QScrollArea( this );
      scrollArea_r       ->setMaximumWidth( (pixelsWide_w + pixelsWide_w +  pixelsWide_r )*3 );
      scrollArea_r       ->setWidgetResizable( true );
      //scrollArea_r     ->setObjectName( strow  + ",wvl_box," + ch_name );
      scrollArea_r     ->setObjectName( strow  + ": wvl_box");
      scrollArea_r     ->setWidget( wvl_box );
      
      gr_mwvbox << scrollArea_r;
      right->addWidget( scrollArea_r  );
      
      qDebug() << "Right.count(), gr_mwvbox.size() AFTER insert -- " << right->count() << gr_mwvbox.size(); ;
      // //END MWV dialog
      
      
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
DbgLv(1) << "Ge:SL: nchn" << nchn << "lcrat size" << le_lcrats.count();


   //Add button to set global combuned plot parameters
   QPushButton* pb_combplot_global    = us_pushbutton( tr( "Global Parameters for \nCombined Plots" ) );
   genL->addWidget( pb_combplot_global,    row++,  9, 2, 3 );
   connect( pb_combplot_global, SIGNAL( clicked  ( ) ),
	    this,         SLOT(   set_combplot_parms( ) ) );

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

   scrollArea     ->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
   scrollArea     ->setWidgetResizable( true );
   scrollArea     ->setWidget( containerWidget );
   //panel          ->addWidget( scrollArea );


   left  -> addWidget( scrollArea, Qt::AlignTop  );

   QWidget *controlsRestrictorWidget_right = new QWidget();
   controlsRestrictorWidget_right->setLayout( right );
   controlsRestrictorWidget_right->setMinimumHeight(500);

   QWidget *controlsRestrictorWidget_left = new QWidget();
   controlsRestrictorWidget_left->setLayout( left );
   controlsRestrictorWidget_left->setMinimumHeight(500);
 

   //middle_h->addLayout( left,  0, 0, -1, 7 );
   //middle_h->addLayout( right, 0, 7, -1, 2 );
   //middle_h->addWidget( controlsRestrictorWidget_left,  0, 0, -1, 7 );
   middle_h->addWidget( controlsRestrictorWidget_left, 0, 0 );
   middle_h->addWidget( controlsRestrictorWidget_right, 0, 7, -1, 2, Qt::AlignRight);
   //middle_h->setSizeConstraint(QLayout::SetNoConstraint);
   
   //middle_h->setRowStretch( 0, 1);
   //Hide all gr_mwvbox instances:
   for ( int i=0; i < gr_mwvbox.size(); ++i )
     gr_mwvbox[ i ]->setVisible( false );
  
   panel->addLayout( middle_h );

   panel->addStretch();
   
   adjustSize();
}

//Togle MWLPrefs checkbox
void US_AnaprofPanGen::mwvChecked( bool checked )
{
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();

   QString channel_name = sl_chnsel[ irow ];

   qDebug() << "oname, Channel name to MWL edit: Checked: " << oname << channel_name << " : " << checked;

   QString mwvbox_oname = QString::number( irow ) + ": wvl_box";
   qDebug() << "mwvbox_oname -- " << mwvbox_oname;

   //if checked, show groupbox dialog with all wavelengths
   if ( checked )
     {
        for ( int i=0; i < gr_mwvbox.size(); ++i )
	 {
	   if ( gr_mwvbox[ i ]->objectName() == mwvbox_oname )
	     {
	       //right->addWidget( gr_mwvbox[ i ]  );
	       gr_mwvbox[ i ]->setVisible( true );
	       break;
	     }
	 }
       //make all checkboxes unselectable ?
     }
   else
     {
       
       for ( int i=0; i < gr_mwvbox.size(); ++i )
	 {
	   if ( gr_mwvbox[ i ]->objectName() == mwvbox_oname )
	     {
	       //right->removeWidget( gr_mwvbox[ i ]  );
	       gr_mwvbox[ i ]->setVisible( false );
	       break;
	     }
	 }
     }
}


//create groupBox
QGroupBox * US_AnaprofPanGen::createGroup( QString & triple_name, QList< double > & wvls )
{
  QGroupBox *groupBox = new QGroupBox ( triple_name );
  groupBox -> setObjectName( triple_name );

  QPalette p = groupBox->palette();
  p.setColor(QPalette::Dark, Qt::white);
  groupBox->setPalette(p);

  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 10px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
    
  groupBox->setFlat(true);

  //GUI
  QGridLayout* genL   = new QGridLayout();
  genL->setSpacing        ( 1 );
  //genL->setContentsMargins( 20, 10, 20, 15 );
  
  int row = 0;

  QLabel*     lb_wvl     = us_label( tr( "Wvl" ) );
  QFont font_w   = lb_wvl->property("font").value<QFont>();
  QFontMetrics fm_w(font_w);
  pixelsWide_w = fm_w.width( lb_wvl->text() );
  
  QLabel*     lb_edit    = us_label( tr( "FitMen" ) );
  QFont font_e   = lb_edit->property("font").value<QFont>();
  QFontMetrics fm_e(font_e);
  pixelsWide_e = fm_e.width( lb_edit->text() );
  // lb_edit->setMaximumWidth( pixelsWide_e*1.1 );
  // lb_edit->adjustSize();

  QLabel*     lb_run     = us_label( tr( "Run" ) );
  QFont font_r   = lb_run->property("font").value<QFont>();
  QFontMetrics fm_r(font_r);
  pixelsWide_r = fm_r.width( lb_run->text() );
  // lb_run->setMaximumWidth( pixelsWide_r*1.1 );
  // lb_run->adjustSize();

  
  genL->addWidget( lb_wvl,    row,    0, 1, 1 );
  genL->addWidget( lb_edit,   row,    1, 1, 1 );
  genL->addWidget( lb_run,    row++,  2, 1, 1 );

  

  QLineEdit    * le_wvl;
  QRadioButton * rb_edit;
  QCheckBox    * ck_run;

      
  for ( int ii = 0; ii < wvls.size(); ii++ )
    {
      QString strow  = QString::number( ii );
      
      //wvl
      le_wvl = new QLineEdit;
      le_wvl ->setPlaceholderText( QString::number (wvls[ii]) );
      le_wvl ->setReadOnly(true);
      QFont font   = le_wvl->property("font").value<QFont>();
      QFontMetrics fm(font);
      int pixelsWide = fm.width( le_wvl->placeholderText() );
      le_wvl->setMaximumWidth( pixelsWide*3 );
      le_wvl->adjustSize();
      
      genL->addWidget( le_wvl,  row,   0, 1, 1 );

      //edit meniscus
      rb_edit =  new QRadioButton(tr(""));
      rb_edit -> setObjectName( strow + ":triple_edit:" + QString::number (wvls[ii]) );
      genL->addWidget( rb_edit,  row,   1, 1, 1, Qt::AlignHCenter );
      
      //Create signalMapper, to pass argument to standard (argument-less) signals:
      signalMapper = new QSignalMapper(this);
      connect(signalMapper, SIGNAL( mapped( QString ) ), this, SLOT( rbEditClicked( QString ) ) );
      connect( rb_edit, SIGNAL( clicked() ), signalMapper, SLOT(map()));
      QString arg_passed = triple_name + "," + strow;
      signalMapper->setMapping ( rb_edit, arg_passed );
            
      //Run checkbox
      ck_run = new QCheckBox( tr(""), this );
      ck_run ->setAutoFillBackground( true );
      ck_run ->setChecked( true );
      ck_run ->setObjectName( strow + ":triple_run:" + QString::number (wvls[ii]) );
      genL->addWidget( ck_run,  row++,  2, 1, 1, Qt::AlignHCenter );

      if ( ii == int(wvls.size()/2) )
	{
	  rb_edit->setChecked( true );
	  ck_run ->setChecked( true );
	  ck_run ->setEnabled( false );

	  //currProf->wvl_edit <<  wvls[ ii ]; 
	}

    }

  //Disable radiobtn && checkbox if Interf.
  if ( triple_name.split(":")[1].contains("Interf") &&  wvls.size() )
    {
      ck_run ->setEnabled( false );
      rb_edit->setEnabled( false );
    }

  //Disable radiobtn && checkbox if only 1 wvl in a channel (not MWL).
  if ( wvls.size() == 1 )
    {
      ck_run ->setEnabled( false );
      rb_edit->setEnabled( false );
    } 

  int ihgt_r        = pb_aproname->height();
  QSpacerItem* spacer2 = new QSpacerItem( 20, ihgt_r );
  genL ->setRowStretch( row, 1 );
  genL ->addItem( spacer2,  row++,  0, 1, 1 );

  groupBox->setLayout(genL);
   
  return groupBox;
}

//toggle radioBtns in groupBox layouts: bind radiobutton to checkbox
void US_AnaprofPanGen::rbEditClicked ( QString arg_passed )
{
   // QObject* sobj       = sender();      // Sender object
   // QString oname       = sobj->objectName();
   // QString irow        = oname.split(":")[0];

   // // casting to the known class sender connected with (QRadioButton)
   // QRadioButton* rb_edit = qobject_cast<QRadioButton*>(sender());

   // qDebug() << "Radiobtn CLICKED: oname, irow -- " << oname << irow;

   QString triple_name = arg_passed.split(",")[0];
   QString irow        = arg_passed.split(",")[1];
  
   //get QGroupBox associated with the clicked RadioButton
   QScrollArea *sa = NULL;
   for ( int i=0; i < gr_mwvbox.size(); ++i )
     {
       QGroupBox *gb = gr_mwvbox[ i ]->findChild<QGroupBox *>( );
       QString gbox_objectName =  gb->objectName();

       qDebug() << "gbox_objectName -- " << gbox_objectName;

       if ( gbox_objectName == triple_name )
	 {
	   sa = gr_mwvbox[ i ];
	   break;
	 }
     }

   qDebug() << "QGroubbox identified:  ObjectName -- " << sa->objectName();
   
   // Now get the checkbox located on the same row as radiobtn
   if ( sa != NULL )
     {
       //identify QRadioButton:
       QRadioButton* rb_selected = NULL;
       foreach (QRadioButton *rb, sa->findChildren<QRadioButton*>())
	 {
	   QString rb_row = (rb->objectName()).split(":")[0];
	   if ( rb_row == irow )
	     {
	       rb_selected = rb;
	       break;
	     }
	 }

       if ( rb_selected == NULL )
	 qDebug() << "No RadioButton selected ...";
       
       foreach (QCheckBox *ckbox, sa->findChildren<QCheckBox*>())
	 {
	   QString ck_row = (ckbox->objectName()).split(":")[0];
	   QString wvl    = (ckbox->objectName()).split(":")[2];
	   	     
	   if ( ck_row == irow )
	     {
	       if ( rb_selected -> isChecked() )
		 {
		   ckbox->setChecked( true );
		   ckbox->setEnabled( false );


		   // //ALEXEY_NEW_REPORT: this setting wvl will not be needed...
		   // //Update report.wavelength with the selected wvl for current channel:
		   // QMap<QString, US_ReportGMP>::iterator ri;
		   // QString chann_desc_short = triple_name;
		   // chann_desc_short.simplified();
		   // chann_desc_short.replace( " ", "" );
		   // for ( ri = internal_reports.begin(); ri != internal_reports.end(); ++ri )
		   //   {
		   //     qDebug() << "Key [chandesc] of internal_reports QMap VS triple_name:  " << ri.key() << " VS " << chann_desc_short;
		   //     if ( ri.key().contains( chann_desc_short ) )
		   // 	 {
		   // 	   qDebug() << "SETTING ref. wvl for channel -- " << wvl << " for " << ri.key();  
		   // 	   internal_reports[ ri.key() ].wavelength = wvl.toDouble();
		   // 	   break;
		   // 	 }
		   //   }
		   ///////////////////////////////////////////////////////
		 }
	       else
		 {
		   //ckbox->setChecked( true );
		   ckbox->setEnabled( true );
		 }
	     }
	   //Enable the rest of checkboxes (associated with not-checked  radiobuttons)
	   else
	     {
	        ckbox->setEnabled( true );
	     }
	   	     
	 }
     }
}

//Togle Report Run checkbox
void US_AnaprofPanGen::reportRunChecked( bool checked )
{
  QObject* sobj       = sender();      // Sender object
  QString oname       = sobj->objectName();
  int irow            = oname.section( ":", 0, 0 ).toInt();

  QString channel_name = sl_chnsel[ irow ];
  qDebug() << "Report to RUN: Checked: " << channel_name << " : " << checked;
    
  QString report_oname = QString::number( irow ) + ": Report";
  qDebug() << "report_oname -- " << report_oname;

  QString replicate_oname = QString::number( irow ) + ": Replicate";
  qDebug() << "replicate_oname -- " << replicate_oname;

  QString use_ch_oname = QString::number( irow ) + ": RefChan --chann_name--" + channel_name;
  qDebug() << "use_ch_oname -- " << use_ch_oname;
   
  QString use_ref_ch_oname = QString::number( irow ) + ": RefUseChan --chann_name--" + channel_name;
  qDebug() << "use_ref_ch_oname -- " << use_ref_ch_oname;

  //old ref_numbers_list
  QList<int> ref_numbers_list_old = ref_numbers_list;
  
   
  //if not checked, disable Report btn && otherwise:
   if ( !checked )
     {
       //Report
       for ( int i=0; i < pb_reports.size(); ++i )
	 {
	   if ( pb_reports[ i ]->objectName().contains( report_oname ) )
	     {
	       pb_reports[ i ] -> setEnabled( false );
	       break;
	     }
	 }
       //Replicate
       for ( int i=0; i < sb_repl_groups.size(); ++i )
	 {
	   if ( sb_repl_groups[ i ]->objectName().contains( replicate_oname ) )
	     {
	       sb_repl_groups[ i ] -> setEnabled( false );
	       break;
	     }
	 }

       //if runReport unChecked:
       int ref_number = 0; // will be largest of all in ref_numbers_list;
       for ( int i=0; i<le_ref_chs.size(); ++i)
       	 {
       	   if ( le_ref_chs[ i ]->objectName().contains( use_ch_oname ) )
       	     {
	       QString ref_text = "Ref:";
	       	       
	       for (int rn=0; rn<ref_numbers_list.size(); ++rn )
		 if (ref_numbers_list[rn] > ref_number )
		   ref_number = ref_numbers_list[rn];
	       ++ref_number;
	       // ( ref_numbers_list.size() > 0 ) ?
	       // 	 ref_number = ref_numbers_list[ ref_numbers_list.size() - 1 ] + 1 :
	       // 	 ref_number = 1 ;

	       qDebug() << "reportUnChkd: ref_number -- " << ref_number;
	       ref_numbers_list[ i ] = ref_number;
	       ref_text += QString::number( ref_number );
       	       le_ref_chs[ i ] -> setText( ref_text );
       	       break;
       	     }
       	 }

       
       qDebug() << "Old/Updated ref_numbers_list_old,  ref_numbers_list -- "
		<< ref_numbers_list_old << ref_numbers_list
		<< ", SAME? " << ( ref_numbers_list_old == ref_numbers_list );
       
       //Use Reference#
       for ( int i=0; i<sb_use_ref_chs.size(); ++i)
	 {
	   if ( sb_use_ref_chs[ i ]->objectName().contains( use_ref_ch_oname ) )
	     {
	       sb_use_ref_chs[ i ] -> setEnabled( false );
	       sb_use_ref_chs[ i ] -> setValue( sb_use_ref_chs[ i ]-> minimum() );
	       //break;
	     }
	   else //check the rest of chnns: if runReport unchecked, set false, otherwise true
	     {
	       ( ck_report_runs[ i ]->isChecked() ) ?
		 sb_use_ref_chs[ i ] -> setEnabled( true ) : sb_use_ref_chs[ i ] -> setEnabled( false ) ;

	       //ONLY when AProf->Gen initialized:
	       if ( AProfIsIntiated )
		 {
		   //and re-set maxValue to to the max in ref_numbers_list:
		   sb_use_ref_chs[ i ]->setMaximum( ref_number );
		   
		   //finnally, re-set to min (0) if there are more than 1 ref_numbers_list != 0
		   //&& ref_numbers_list changed
		   int ref_chann_number = 0;
		   for (int rn=0; rn<ref_numbers_list.size(); ++rn )
		     if ( ref_numbers_list[rn] > 0 )
		       ++ref_chann_number;
		   if ( ref_chann_number > 1 && ref_numbers_list_old != ref_numbers_list ) 
		     sb_use_ref_chs[ i ] -> setValue( sb_use_ref_chs[ i ]->minimum() );
		   else if ( ref_chann_number == 1 )
		     sb_use_ref_chs[ i ] -> setValue( 1 );
		 }
	     }
	 }
     }
   else
     {
       //Report
       for ( int i=0; i < pb_reports.size(); ++i )
	 {
	   if ( pb_reports[ i ]->objectName().contains( report_oname ) )
	     {
	       pb_reports[ i ] -> setEnabled( true );
	       break;
	     }
	 }
       //Replicate
       for ( int i=0; i < sb_repl_groups.size(); ++i )
	 {
	   if ( sb_repl_groups[ i ]->objectName().contains( replicate_oname ) )
	     {
	       sb_repl_groups[ i ] -> setEnabled( true );
	       break;
	     }
	 }

       //Reference? (set #) disable...//if runReport Checked back: 
       for ( int i=0; i<le_ref_chs.size(); ++i)
       	 {
       	   if ( le_ref_chs[ i ]->objectName().contains( use_ch_oname ) )
       	     {
	       int curr_ref_number = le_ref_chs[ i ] -> text().split(":")[1].toInt();
	       //reset properly all other ref_numbers
	       for (int rn=0; rn<ref_numbers_list.size(); ++rn )
		 if ( ref_numbers_list[rn] > curr_ref_number )
		   {
		     --ref_numbers_list[ rn ];
		     le_ref_chs[ rn ] -> setText("Ref:" + QString::number( ref_numbers_list[ rn ] ));
		   }
	       
	       ref_numbers_list[ i ] = 0;
	       le_ref_chs[ i ] -> setText("");
       	       break;
       	     }
       	 }

       //find new max ref_#
       int ref_number_max = 0;
       for (int rn=0; rn<ref_numbers_list.size(); ++rn )
	 if (ref_numbers_list[rn] > ref_number_max )
	   ref_number_max = ref_numbers_list[rn];
       
       //Use Reference?
       for ( int i=0; i<sb_use_ref_chs.size(); ++i)
       	 {
       	   if ( sb_use_ref_chs[ i ]->objectName().contains( use_ref_ch_oname ) )
       	     {
       	       sb_use_ref_chs[ i ] -> setEnabled( true );
       	       //break;
       	     }
	   else//check the rest of chnns: if runReport unchecked, set false, otherwise true
	     {
	       ( ck_report_runs[ i ]->isChecked() ) ?
		 sb_use_ref_chs[ i ] -> setEnabled( true ) : sb_use_ref_chs[ i ] -> setEnabled( false ) ;
	     
	       //ONLY when AProf->Gen initialized:
	       if ( AProfIsIntiated )
		 {
		   //and re-set maxValue to to the max in ref_numbers_list:
		   sb_use_ref_chs[ i ]->setMaximum( ref_number_max );

		   //finnally, re-set to min (0) if there are more than 1 ref_numbers_list != 0
		   // //&& ref_numbers_list changed
		   int ref_chann_number = 0;
		   for (int rn=0; rn<ref_numbers_list.size(); ++rn )
		     if ( ref_numbers_list[rn] > 0 )
		       ++ref_chann_number;
		   if ( ref_chann_number > 1 && ref_numbers_list_old != ref_numbers_list ) 
		     sb_use_ref_chs[ i ] -> setValue( sb_use_ref_chs[ i ]->minimum() );
		   else if ( ref_chann_number == 1 && ck_report_runs[ i ]->isChecked() )
		     sb_use_ref_chs[ i ] -> setValue( 1 );
		 }
	     }
	 }
     }

   //check if ALL runReport checkboxes ARE checked, then disable ALL "Use Reference?"
   bool all_reports_run  = true;
   for ( int ii = 0; ii < sl_chnsel.size(); ii++ )
     {
       if ( !ck_report_runs[ ii ]->isChecked() )
	 {
	   all_reports_run = false;
	   break;
	 }
     }

   if ( all_reports_run )
     {
       for ( int i=0; i<sb_use_ref_chs.size(); ++i)
       	 {
	   sb_use_ref_chs[ i ] -> setEnabled( false );
	   sb_use_ref_chs[ i ] -> setValue( sb_use_ref_chs[ i ]-> minimum() );

	   // sb_ref_chs[ i ] -> setEnabled( false );
	   // sb_ref_chs[ i ] -> setValue( sb_ref_chs[ i ]-> minimum() );

	   ref_numbers_list[ i ] = 0;
	   le_ref_chs[ i ] -> setText("");
	 }
     }

   // //Reset use_ref maximum() based on max # references
   // int max_ref = 0;
   // for (int rn=0; rn<ref_numbers_list.size(); ++rn )
   //   if (ref_numbers_list[rn] > max_ref )
   //     max_ref = ref_numbers_list[rn];

   // qDebug() << "MAX_REF -- " << max_ref;

   // for ( int i=0; i<sb_use_ref_chs.size(); ++i)
   //   if ( ck_report_runs[ i ]->isChecked() )
   // 	 sb_use_ref_chs[ i ] -> setMaximum( max_ref );
}

//Togle Analysis Run checkbox
void US_AnaprofPanGen::runChecked( bool checked )
{
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();

   QString channel_name = sl_chnsel[ irow ];
   qDebug() << "Channel name to RUN: Checked: " << channel_name << " : " << checked;

   QString mwv_oname = QString::number( irow ) + ": MWV";
   qDebug() << "mwv_oname -- " << mwv_oname;

   QString report_oname = QString::number( irow ) + ": Report";
   qDebug() << "report_oname -- " << report_oname;

   QString run_report_oname = QString::number( irow ) + ": RunReport";
   qDebug() << "run_report_oname -- " << run_report_oname;
   
   QString replicate_oname = QString::number( irow ) + ": Replicate";
   qDebug() << "replicate_oname -- " << replicate_oname;

   QString use_ch_oname = QString::number( irow ) + ": RefChan --chann_name--";
   qDebug() << "use_ch_oname -- " << use_ch_oname;
   
   QString use_ref_ch_oname = QString::number( irow ) + ": RefUseChan --chann_name--";
   qDebug() << "use_ref_ch_oname -- " << use_ref_ch_oname;
   
   //if not checked, disable Report btn && MWV checkbox, and otherwise:
   if ( !checked )
     {

       //Run Report
       for ( int i=0; i < ck_report_runs.size(); ++i )
	 {
	   if ( ck_report_runs[ i ]->objectName().contains( run_report_oname ) )
	     {
	       ck_report_runs[ i ] -> setChecked( false );
	       ck_report_runs[ i ] -> setEnabled( false );
	       break;
	     }
	 }

              
       // //Report: No need for Report btn since it's bound now to Run Report checkbox...
       // for ( int i=0; i < pb_reports.size(); ++i )
       // 	 {
       // 	   if ( pb_reports[ i ]->objectName().contains( report_oname ) )
       // 	     {
       // 	       pb_reports[ i ] -> setEnabled( false );
       // 	       break;
       // 	     }
       // 	 }
       

       //Replicates
       for ( int i=0; i < sb_repl_groups.size(); ++i )
	 {
	   if ( sb_repl_groups[ i ]->objectName().contains( replicate_oname ) )
	     {
	       sb_repl_groups[ i ] -> setEnabled( false );
	       break;
	     }
	 }

       
       //MWV
       for ( int i=0; i<ck_mwv.size(); ++i )
	 {
	   if ( ck_mwv[ i ]->objectName() == mwv_oname )
	     {
	       gr_mwvbox[ i ]->setVisible( false );
	       ck_mwv[ i ]   ->setChecked( false );
	       ck_mwv[ i ]   ->setEnabled( false );
	       break;
	     }
	 }

       // //Reference? (set #) enable first...
       // for ( int i=0; i<sb_ref_chs.size(); ++i)
       // 	 {
       // 	   if ( sb_ref_chs[ i ]->objectName().contains( use_ch_oname ) )
       // 	     {
       // 	       sb_ref_chs[ i ] -> setEnabled( true );
       // 	       break;
       // 	     }
       // 	 }
       
       
       // //Use Reference#
       // for ( int i=0; i<sb_use_ref_chs.size(); ++i)
       // 	 {
       // 	   if ( sb_use_ref_chs[ i ]->objectName().contains( use_ref_ch_oname ) )
       // 	     {
       // 	       sb_use_ref_chs[ i ] -> setEnabled( false );
       // 	       break;
       // 	     }
       // 	 }
     }
   else
     {
       //Run Report
       for ( int i=0; i < ck_report_runs.size(); ++i )
	 {
	   if ( ck_report_runs[ i ]->objectName().contains( run_report_oname ) )
	     {
	       ck_report_runs[ i ] -> setEnabled( true );
	       //if abde, also set checked
	       if ( mainw->abde_mode_aprofile )
		 ck_report_runs[ i ] -> setChecked( true );
	       break;
	     }
	 }
       
       // //Report: No need for Report btn since it's bound now to Run Report checkbox...
       // for ( int i=0; i < pb_reports.size(); ++i )
       // 	 {
       // 	   if ( pb_reports[ i ]->objectName().contains( report_oname ) )
       // 	     {
       // 	       pb_reports[ i ] -> setEnabled( true );
       // 	       break;
       // 	     }
       // 	 }

       // //Replicates: No need for Report btn since it's bound now to Run Report checkbox...
       // for ( int i=0; i < sb_repl_groups.size(); ++i )
       // 	 {
       // 	   if ( sb_repl_groups[ i ]->objectName().contains( replicate_oname ) )
       // 	     {
       // 	       sb_repl_groups[ i ] -> setEnabled( true );
       // 	       break;
       // 	     }
       // 	 }
       
       //MWV
       for ( int i=0; i<ck_mwv.size(); ++i )
	 {
	   if ( ck_mwv[ i ]->objectName() == mwv_oname )
	     {
	       ck_mwv[ i ]->setEnabled( true );
	       break;
	     }
	 }
       
       // //Reference? (set #) disable...
       // for ( int i=0; i<sb_ref_chs.size(); ++i)
       // 	 {
       // 	   if ( sb_ref_chs[ i ]->objectName().contains( use_ch_oname ) )
       // 	     {
       // 	       sb_ref_chs[ i ] -> setEnabled( false );
       // 	       break;
       // 	     }
       // 	 }
       
       
       // //Use Reference?
       // for ( int i=0; i<sb_use_ref_chs.size(); ++i)
       // 	 {
       // 	   if ( sb_use_ref_chs[ i ]->objectName().contains( use_ref_ch_oname ) )
       // 	     {
       // 	       sb_use_ref_chs[ i ] -> setEnabled( true );
       // 	       break;
       // 	     }
       // 	 }
     }

   //ALEXEY: check what's unselected -- if all triples then disable 2DSA && PCSA tabs
   int run_sum = 0;
   for ( int ii = 0; ii < sl_chnsel.size(); ii++ )
     {
       run_sum += int(ck_runs[ ii ]->isChecked()); 
     }

   if ( run_sum )
     {
       emit set_tabs_buttons_active();
     }
   else
     {
       emit set_tabs_buttons_inactive();
     }
}

//Report Btn clicked
void US_AnaprofPanGen::setReport( void )
{
   QObject* sobj       = sender();      // Sender object
   QString oname       = sobj->objectName();
   int irow            = oname.section( ":", 0, 0 ).toInt();

   QString channel_name = sl_chnsel[ irow ];

   qDebug() << "Set Report: Clicked: " << channel_name;
   qDebug() << "Report_oname -- " << oname;

   //Find out channel desc
   QString chan_desc = oname.section( "--chann_name--", 1, 1 );
   qDebug() << "Channel_desc -- " << chan_desc;

   
   //ALEXEY_NEW_REPORT: internal_reports[ chan_desc ] is now QMap < QString( wvl), US_ReportGMP >
   // OR should I create a small dialog with all channel's wvls, choose one && pass to US_ReportGui ? (one-by-one)
   //US_ReportGMP* chan_report = &(internal_reports[ chan_desc ]);

// //TESTING !!!!
//    QMap < QString, US_ReportGMP* > channel_report_map = report_map[ chan_desc ];
//    reportGui = new US_ReportGui( channel_report_map );
// ///////////////

   QMap < QString, US_ReportGMP* > channel_report_map;
   QMap < QString, US_ReportGMP>::iterator ri;
   for ( ri = internal_reports[ chan_desc ].begin(); ri != internal_reports[ chan_desc ].end(); ++ri )
     {
       QString wvl = ri.key();

       //qDebug() << "In setReport: wvl -- " << wvl;
       channel_report_map[ wvl ] = &( internal_reports[ chan_desc ][ wvl] );
     }
         
   reportGui = new US_ReportGui(  channel_report_map );
   reportGui->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
   reportGui->setWindowModality(Qt::ApplicationModal);
   
   connect( reportGui, SIGNAL( cancel_changes ( QMap< QString, US_ReportGMP>& ) ), this, SLOT( restore_report  ( QMap< QString, US_ReportGMP>& )  ) );

   //ALEXEY_NEW_REPORT: this signals may not be needed...
   // connect( reportGui, SIGNAL( apply_to_all_reports( US_ReportGMP* ) ), this, SLOT( apply_to_other_reports( US_ReportGMP* )  ) );
   // /////////////////////////////////////////////////////

   //abde
   if ( mainw->abde_mode_aprofile )
     reportGui->abde_mode_passed();
   
   reportGui->show();

}

//ALEXEY_NEW_REPORT: this method may not be needed...
void US_AnaprofPanGen::restore_report( QMap < QString, US_ReportGMP> & orig_report_map )
{
  QString channame = orig_report_map.values()[0].channel_name;
  qDebug() << "Channel Name in orig -- " << channame;
  
  internal_reports[ channame ] = orig_report_map;
}
/////////////////////////////////////////////////////

//ALEXEY_NEW_REPORT: this method may not be needed...
void US_AnaprofPanGen::apply_to_other_reports( US_ReportGMP* report )
{
  // QString primary_report_channel = report->channel_name;
  
  // QMap<QString, US_ReportGMP>::iterator ri;
  // for ( ri = internal_reports.begin(); ri != internal_reports.end(); ++ri )
  //   {
  //     if ( ri.key().contains( primary_report_channel ) )
  // 	continue;
      
  //     qDebug() << "Applying report's " << primary_report_channel << " settings to channel " << ri.key();  

  //     internal_reports[ ri.key() ].tot_conc            = report->tot_conc;
  //     internal_reports[ ri.key() ].rmsd_limit          = report->rmsd_limit;
  //     internal_reports[ ri.key() ].av_intensity        = report->av_intensity;
  //     //internal_reports[ ri.key() ].wavelength          = report->wavelength;
  //     //internal_reports[ ri.key() ].experiment_duration = report->experiment_duration;

  //     //Now go over reportItems:
  //     //1st, clear current array of reportItems:
  //     internal_reports[ ri.key() ].reportItems.clear();

  //     for ( int ic = 0; ic < report->reportItems.size(); ++ic )
  // 	{
  // 	  //US_ReportGMP::ReportItem copied_item = report->reportItems[ ic ];
  // 	  internal_reports[ ri.key() ].reportItems.push_back( report->reportItems[ ic ] );
  // 	}
  //   }
}
//////////////////////////////////////////////////////

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
DbgLv(1) << "APGe:  pn: passed:" << protname << aproname
 << "McP names:" << mainw->currProf.protoname << mainw->currProf.aprofname
 << "GcP names:" << currProf->protoname << currProf->aprofname;
   currProf->protoname = protname;
   currProf->aprofname = aproname;

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

//Set general Rpeort settings
void US_AnaprofPanGen::set_gen_report_settings()
{
  QString reportMask_tmp;
  
  reportMask_tmp =
   "{"
      "\"A. General Settings\":        \"1\","
      "\"B. Lab/Rotor Parameters\":    \"1\","
      "\"C. Optima Machine Operator\": \"1\","
      "\"D. Speed Parameters\":        \"1\","
      "\"E. Cell Centerpiece Usage\":  \"1\","
      "\"F. Solutions for Channels\":"
               "["
                  "{\"Solution Information\":  \"1\"},"
                  "{\"Analyte Information\":   \"1\"},"
                  "{\"Buffer  Information\":   \"1\"}"
               "],"
      "\"G. Optics\":                  \"1\","
      "\"H. Ranges\":                  \"1\","
      "\"I. Scan Counts and Intervals for Optics\": \"1\","
      "\"J. Analysis Profile\":"
               "["
                  "{\"General Settings\":"
                          "{"
                             "\"Channel General Settings\":            \"1\","
                             "\"Report Parameters (per-triple)\":      \"1\","
                             "\"Report Item Parameters (per-triple)\": \"1\""
                           "}},"
                   "{\"2DSA Controls\":" 
                          "{"
                             "\"Job Flow Summary\":     \"1\","
                             "\"Per-Channel Profiles\": \"1\""
                          "}},"
                   "{\"PCSA Controls\":"
                          "{"
                              "\"Job Flow Summary\":     \"1\","
                              "\"Per-Channel Profiles\": \"1\""
                          "}}"
  
               "]"
   "}"
     ;
  
  //reportMask_tmp = "{\"A. General Settings\":\"2\",\"B. Lab/Rotor Parameters\":\"2\",\"C. Optima Machine Operator\":\"2\",\"D. Speed Parameters\":\"2\",\"E. Cell Centerpiece Usage\":\"2\",\"F. Solutions for Channels\":[{\"Solution Information\":\"0\"},{\"Analyte Information\":\"0\"},{\"Buffer  Information\":\"0\"}],\"G. Optics\":\"2\",\"H. Ranges\":\"0\",\"I. Scan Counts and Intervals for Optics\":\"2\",\"J. Analysis Profile\":[{\"General Settings\":{\"Channel General Settings\":\"0\",\"Report Item Parameters (per-triple)\":\"0\",\"Report Parameters (per-triple)\":\"0\"}},{\"2DSA Controls\":{\"Job Flow Summary\":\"2\",\"Per-Channel Profiles\":\"2\"}},{\"PCSA Controls\":{\"Job Flow Summary\":\"2\",\"Per-Channel Profiles\":\"2\"}}]}";
  
    
  //reportGenGui = new US_ReportGenGui( reportMask_tmp );
  reportGenGui = new US_ReportGenGui( currProf->report_mask );
  reportGenGui->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  reportGenGui->setWindowModality(Qt::ApplicationModal);
  
  connect( reportGenGui, SIGNAL(  update_details( QString& ) ), this, SLOT( update_gen_report_settings  ( QString& )  ) );

  //abde
  if ( mainw->abde_mode_aprofile )
    reportGenGui->abde_mode_passed();
     
  reportGenGui->show();
}

//Update general report mask settings
void US_AnaprofPanGen::update_gen_report_settings( QString& mask_updated )
{
  currProf->report_mask.clear();
  currProf->report_mask = mask_updated;
}

//Set Combined Plot Global Parms
void US_AnaprofPanGen::set_combplot_parms( ) 
{
  qDebug() << "currProf->combPlots_parms --  " << currProf->combPlots_parms;
  qDebug() << "loadProf->combPlots_parms --  " << mainw->loadProf.combPlots_parms;

  if ( currProf->combPlots_parms == mainw->loadProf.combPlots_parms )
    qDebug() << "They are EQUAL!";
  
  
  //collect all 'type-method' combinations across all triples:
  QStringList type_method_list;
  //int nchna   = currProf->pchans.count();
  int nchna    = le_channs.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = currProf-> chndescs_alt[ i ];
      QString channel_desc     = currProf-> chndescs[ i ];

      bool triple_report = false;
      
      if ( ck_runs[ i ] -> isChecked()  )
	{
	  //now check if report will be run:
	  QString run_report;
	  if ( ck_report_runs[ i ] -> isChecked() )
	    triple_report = true;
	}

      if ( triple_report )
	{
	  QMap < QString, US_ReportGMP > chann_reports = internal_reports[ channel_desc_alt ];
	  QList < QString > ch_wavelengths             = chann_reports.keys();

	  int chann_wvl_number = ch_wavelengths.size();
	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl              = ch_wavelengths[ jj ];
	      US_ReportGMP c_reportGMP = chann_reports[ wvl ];

	      int report_items_number = c_reportGMP.reportItems.size();
	      for ( int kk = 0; kk < report_items_number; ++kk )
		{
		  US_ReportGMP::ReportItem curr_item = c_reportGMP.reportItems[ kk ];
		  QString c_type   = curr_item.type;
		  QString c_method = curr_item.method;

		  QString c_t_m = c_type + "," + c_method;
		  qDebug() << "c_t_m -- " << c_t_m;
		  type_method_list << c_t_m;
		}
	    }
	}
    }
  type_method_list.removeDuplicates();
  qDebug() << "Type-Method list: " << type_method_list;
  //end of collecting 'type-methods' combinations
  
  combPlotsGui = new US_CombPlotsGui( currProf->combPlots_parms, type_method_list ); 
  combPlotsGui ->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  combPlotsGui ->setWindowModality(Qt::ApplicationModal);
  
  connect( combPlotsGui, SIGNAL( update_combplots_parms( QString& ) ), this, SLOT( update_combplots_settings  ( QString& )  ) );
     
  combPlotsGui->show();
}

//Update combined plots global params
void US_AnaprofPanGen::update_combplots_settings( QString& combPlots_updated )
{
  currProf->combPlots_parms .clear();
  currProf->combPlots_parms = combPlots_updated;
}

  
//Scan Range to exclude
void US_AnaprofPanGen::set_scan_ranges()
{
  qDebug() << "Set_scan_ranges 2: scanCount && scanCount_int -- "
	   << mainw->scanCount << " && "
	   << mainw->scanCount_int;

  qDebug() << "Set_scan_ranges: SIZES:  currProf->chndescs, currProf->scan_excl_begin, currProf->scan_excl_end -- "
	   << currProf->chndescs.size()
	   << currProf->scan_excl_begin.size()
	   << currProf->scan_excl_end.size();

  //debug
  for ( int i=0; i < currProf->chndescs.size(); ++i  )
    qDebug() << "Ch_desc, scan_beg, scan_end -- " << currProf->chndescs[i] << currProf->scan_excl_begin[i] << currProf->scan_excl_end[i];
  for ( int i=0; i < currProf->scan_excl_begin.size(); ++i ) 
    qDebug() << "scan_beg, scan_end -- " << currProf->scan_excl_begin[i] << currProf->scan_excl_end[i];
  
  //scanExclGui = new US_ScanExclGui( currProf->chndescs, currProf->scan_excl_begin, currProf->scan_excl_end, mainw->scanCount, mainw->scanCount_int );
  scanExclGui = new US_ScanExclGui( sl_chnsel, currProf->scan_excl_begin, currProf->scan_excl_end, mainw->scanCount, mainw->scanCount_int );
  scanExclGui->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  scanExclGui->setWindowModality(Qt::ApplicationModal);
  
  connect( scanExclGui, SIGNAL(  update_aprofile_scans( QStringList& ) ), this, SLOT( update_excl_scans  ( QStringList& )  ) );
     
  scanExclGui->show();
}

//Update excl. scan ranges
void US_AnaprofPanGen::update_excl_scans( QStringList& scan_ranges_list )
{
  currProf->scan_excl_begin.clear();
  currProf->scan_excl_end.clear();

  qDebug() << "Scan update: init sizes scan_excl_beginn, scan_excl_end -- "
	   << currProf->scan_excl_begin.size()
	   << currProf->scan_excl_end.size();

  // QMap< int, int >::iterator ri;
  // for ( ri = scan_ranges_pairs.begin(); ri != scan_ranges_pairs.end(); ++ri )
  //   {
  //     currProf->scan_excl_begin << ri.key();
  //     currProf->scan_excl_end   << ri.value();

  //     qDebug() << "in QMap update: " << ri.key() << ri.value();
  //   }

  for ( int i=0; i < scan_ranges_list.size(); ++i )
    {
      QString s_b = scan_ranges_list[ i ].split(":")[0];
      QString s_e = scan_ranges_list[ i ].split(":")[1];
      
      currProf->scan_excl_begin << s_b.toInt();
      currProf->scan_excl_end   << s_e.toInt();

      qDebug() << "in scan_list update: " << s_b.toInt() << s_e.toInt();
    }

  qDebug() << "Scan update: final sizes scan_excl_beginn, scan_excl_end -- "
	   << currProf->scan_excl_begin.size()
	   << currProf->scan_excl_end.size();
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

   //abde
   QString lddens_c = le_dens0s[ 0 ]->text();
   QString vbar_c   = le_vbars[ 0 ]->text();
   QString mw_c     = le_MWs[ 0 ]->text();

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

      //abde
      if ( le_dens0s[ jj ]->text() != lddens_c )
	le_dens0s[ jj ]->setText( lddens_c );

      if ( le_vbars[ jj ]->text() != vbar_c )
	le_vbars[ jj ]->setText( vbar_c );

      if ( le_MWs[ jj ]->text() != mw_c )
	le_MWs[ jj ]->setText( mw_c );
   }
}


// Panel for 2DSA parameters
US_AnaprofPan2DSA::US_AnaprofPan2DSA( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_AnalysisProfileGui*)topw;
   dbg_level           = US_Settings::us_debug();
   cchx                = 0;
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
   QLabel*  lb_j2iter  = us_label ( tr( "Refinement Iterations" ) );
   
   QLabel*  lb_j4iter  = us_label ( tr( "Refinement Iterations" ) );
   QLabel*  lb_j5iter  = us_label ( tr( "Monte-Carlo Iterations" ) );

   //QGroupBox for Fit meniscus/bottom options
   meniscus_box = new QGroupBox(tr("Fit Meniscus | Bottom"));
   meniscus_box-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 10px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");

   QRadioButton *fm  = new QRadioButton(tr("Fit Meniscus"));
   fm-> setObjectName("fm");
   QRadioButton *fb  = new QRadioButton(tr("Fit Bottom"));
   fb-> setObjectName("fb");
   QRadioButton *fmb = new QRadioButton(tr("Fit Meniscus and Bottom"));
   fmb-> setObjectName("fmb");
   
   fm->setChecked(true);
   QVBoxLayout *vbox = new QVBoxLayout;
   vbox->addWidget(fm);
   vbox->addWidget(fb);
   vbox->addWidget(fmb);
   vbox->addStretch(1);
   meniscus_box->setLayout(vbox);
   
   
//   int ihgt            = lb_smin->height();
//   QSpacerItem* spacer1 = new QSpacerItem( 20, ihgt );

   // Text boxes and other value elements
   le_smin         = us_lineedit( "1", 0, false );
   le_smax         = us_lineedit( "10", 0, false );
   le_sgrpts       = us_lineedit( "64", 0, false );
   le_kmin         = us_lineedit( "1", 0, false );
   le_kmax         = us_lineedit( "4", 0, false );
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
   cb_chnsel->setCurrentIndex( cchx );
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
   le_j2iter       = us_lineedit( "1", 0, false );
   
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
   //Meniscus/bottom fit
   genL->addWidget( meniscus_box,  row++,  9, 4,  2 );
   genL->addWidget( lb_j2mrng,  row,    5, 1,  2 );
   genL->addWidget( le_j2mrng,  row++,  7, 1,  1 );
   genL->addWidget( lb_j2iter,  row,    5, 1,  2 );
   genL->addWidget( le_j2iter,  row++,  7, 1,  1 );  

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

   connect( le_j2iter,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( mfiter_changed  ( )      ) );
   
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

// // Channel Selected
// void US_AnaprofPan2DSA::channel_selected( int chnx )
// {
//    int prvx    = cchx;
//    cchx        = chnx;
// DbgLv(1) << "2D:SL: CHAN_SEL" << chnx << "prvx" << prvx;
//    // Save parameters from previous channel and load current
//    gui_to_parms( prvx );
//    parms_to_gui( chnx );

//    // Enable/disable Next based on last index
//    int lndx    = sl_chnsel.count() - 1;
//    pb_nextch->setEnabled( chnx < lndx );
// }

// ALEXEY: Channel Selected modified to account for arbitrary channel selection
void US_AnaprofPan2DSA::channel_selected( int chnx )
{
   int prvx    = cchx;
   cchx        = chnx;
DbgLv(1) << "2D:SL: CHAN_SEL" << chnx << "prvx" << prvx;
   // Save parameters from previous channel and load current
   gui_to_parms( prvx );
   parms_to_gui( chnx );

   // Enable/disable Next based on last index
   int lndx = 0;
   if ( active_items_2dsa.size() )
     lndx = active_items_2dsa.last();
   
   //int lndx    = sl_chnsel.count() - 1;
   pb_nextch->setEnabled( chnx < lndx );
}

// Transfer gui elements to parms vector row
void US_AnaprofPan2DSA::gui_to_parms( int rowx )
{
   US_AnaProfile* currProf  = &mainw->currProf;
   US_AnaProfile::AnaProf2DSA::Parm2DSA parm1;
   int kchan       = currProf->pchans.count();
   int kparm       = currProf->ap2DSA.parms.size();
DbgLv(1) << "2D:SL-gp: rowx" << rowx << "kchan" << kchan << "kparm" << kparm;
   if ( rowx >= kparm )
   {
DbgLv(1) << "2D:SL-gp: *ERROR* rowx>=kparm";
      return;
   }
   parm1.s_min      = le_smin  ->text().toDouble();
   parm1.s_max      = le_smax  ->text().toDouble();
   parm1.k_min      = le_kmin  ->text().toDouble();
   parm1.k_max      = le_kmax  ->text().toDouble();
   parm1.ff0_const  = le_constk->text().toDouble();
   parm1.s_grpts    = le_sgrpts->text().toDouble();
   parm1.k_grpts    = le_kgrpts->text().toDouble();
   parm1.gridreps   = le_grreps->text().toDouble();
   parm1.varyvbar   = ck_varyvb->isChecked();
   parm1.have_custg = ( !le_custmg->text().isEmpty()  &&
                       le_custmg->text() != "(none)" );
   parm1.channel    = sl_chnsel[ rowx ];
   parm1.cust_grid  = le_custmg->text();
   parm1.cgrid_name = le_custmg->text();

   currProf->ap2DSA.parms.replace( rowx, parm1 );
}

// Transfer parms vector element to gui elements
void US_AnaprofPan2DSA::parms_to_gui( int rowx )
{
   US_AnaProfile* currProf  = &mainw->currProf;
   QVector< US_AnaProfile::AnaProf2DSA::Parm2DSA >*
      parms        = &currProf->ap2DSA.parms;
   int kchan       = currProf->pchans.count();
   int kparm       = currProf->ap2DSA.parms.size();
DbgLv(1) << "2D:SL-pg: rowx" << rowx << "kchan" << kchan << "kparm" << kparm;
   if ( rowx >= kparm )
   {
DbgLv(1) << "2D:SL-pg: *ERROR* rowx>=kparm";
      return;
   }
   US_AnaProfile::AnaProf2DSA::Parm2DSA
      parm1        = parms->at( rowx );
   le_smin  ->setText( QString::number( parm1.s_min     ) );
   le_smax  ->setText( QString::number( parm1.s_max     ) );
   le_kmin  ->setText( QString::number( parm1.k_min     ) );
   le_kmax  ->setText( QString::number( parm1.k_max     ) );
   le_constk->setText( QString::number( parm1.ff0_const ) );
   le_sgrpts->setText( QString::number( parm1.s_grpts   ) );
   le_kgrpts->setText( QString::number( parm1.k_grpts   ) );
   le_grreps->setText( QString::number( parm1.gridreps  ) );
   le_custmg->setText( parm1.cust_grid );
   QString chan    = parm1.channel;
   QString chan_l  = sl_chnsel[ rowx ];
DbgLv(1) << "2D:SL-pg:  rowx" << rowx << "channel" << chan << chan_l;
   if ( chan != chan_l )
   {  // Parms channel wrong, so replace with correct string
      parm1.channel     = chan_l;
      currProf->ap2DSA.parms.replace( rowx, parm1 );
DbgLv(1) << "2D:SL-pg:    chan replaced:" << parms->at( rowx ).channel;
      chan              = chan_l;
   }
   int ixchns      = cb_chnsel->findText( chan );
   if ( ixchns < 0 )
   {  // Channel string not found, so search for starting substring
      for ( int ii = 0; ii < cb_chnsel->count(); ii++ )
      {
         if ( cb_chnsel->itemText( ii ).startsWith( chan ) )
         {
            ixchns          = ii;
DbgLv(1) << "2D:SL-pg:   ixchns(I)" << ixchns << chan << cb_chnsel->itemText(ii);
            break;
         }
      }
   }
else
 DbgLv(1) << "2D:SL-pg:   ixchns(X)" << ixchns << chan << cb_chnsel->itemText(ixchns);
   cb_chnsel->setCurrentIndex( ixchns );

   pb_nextch->setEnabled( rowx < ( sl_chnsel.count() - 1 ) );

}

// // Next button clicked
// void US_AnaprofPan2DSA::next_channel( )
// {
// DbgLv(1) << "2D:SL: NEXT_CHAN";
//    int chnx    = qMin( cb_chnsel->currentIndex() + 1,
//                        sl_chnsel.count() - 1 );
//    cb_chnsel->setCurrentIndex( chnx );

//    channel_selected( chnx );
// }

// ALEXEY: Next Channel taking into account arbitrary selection of channels in Gen tab
void US_AnaprofPan2DSA::next_channel( )
{
DbgLv(1) << "2D:SL: NEXT_CHAN";
  int lndx = 0;
  if ( active_items_2dsa.size() )
    lndx = active_items_2dsa.last();

  for (int i=0; i<active_items_2dsa.size(); i++)
    qDebug() << "Active_Items: ii, value " << i << active_items_2dsa[i];

  qDebug() << "Last 2dsa index: " << lndx;
  qDebug() << "cb_chnsel->currentIndex(): " << cb_chnsel->currentIndex();

  int active_curr_ind = active_items_2dsa.indexOf( cb_chnsel->currentIndex() );

  qDebug() << "Active curr Ind (2DSA): " << active_curr_ind;
  int chnx    = qMin( active_items_2dsa[ active_curr_ind +1 ], lndx );
  cb_chnsel->setCurrentIndex( chnx );

  channel_selected( chnx );
}




// smin value changed
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
  // // Iterate over all active channels && apply gui_to_pams()
  // for (int i=0; i<active_items_2dsa.size(); i++)
  //   {
  //     gui_to_parms( i );
  //   }

  int active_curr_ind = active_items_2dsa.indexOf( cb_chnsel->currentIndex() );
  int chnx    = active_items_2dsa[ active_curr_ind ];

  qDebug() << "Apply to All: 2DSA: Current chan index -- " <<  active_curr_ind << chnx;

  US_AnaProfile* currProf  = &mainw->currProf;
  QVector< US_AnaProfile::AnaProf2DSA::Parm2DSA >* parms = &currProf->ap2DSA.parms;
  int kchan       = currProf->pchans.count();
  int kparm       = currProf->ap2DSA.parms.size();
  qDebug() << "2DSA: Size of chans, parms -- " << kchan <<  kparm;

  //Save current chan parameters
  gui_to_parms( chnx );

  //Get current row parameters:
  US_AnaProfile::AnaProf2DSA::Parm2DSA parm_curr  = parms->at( chnx );

  //Replace params for all other channels with the current ones:
  for ( int i = 0; i < kparm; ++i  )
    {
      QString chan_g  = parms->at( i ).channel;
      currProf->ap2DSA.parms.replace( i, parm_curr );
      currProf->ap2DSA.parms[ i ].channel = chan_g;
    }
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
void US_AnaprofPan2DSA::mfiter_changed( )
{
DbgLv(1) << "2D:SL: J2ITER_CHG";
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
   cchx                = 0;
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
   
   // QStringList sl_axistype;
   // sl_axistype << "s" << "f/f0" << "mw" << "vbar" << "D";
   QStringList sl_xaxistype;
   sl_xaxistype << "s";

   QStringList sl_yaxistype;
   sl_yaxistype << "f/f0" << "mw" << "vbar";
   
   QStringList sl_zaxistyp;
   //sl_zaxistyp << "vbar" << "f/f0" << "mw";
   sl_zaxistyp << "vbar" << "f/f0";

   
   cb_curvtype     = new QComboBox( this );
   cb_curvtype->addItems( sl_curvtype );
   cb_curvtype->setCurrentIndex( 0 );
   
   cb_xaxistyp     = new QComboBox( this );
   //cb_xaxistyp->addItems( sl_axistype );
   cb_xaxistyp->addItems( sl_xaxistype );
   cb_xaxistyp->setEnabled( false );
   
   le_xmin         = us_lineedit( "1", 0, false );
   le_xmax         = us_lineedit( "10", 0, false );

   //Add objectName
   le_xmin      -> setObjectName( "xmin" );
   le_xmax      -> setObjectName( "xmax" );

   cb_yaxistyp     = new QComboBox( this );
   //cb_yaxistyp->addItems( sl_axistype );
   cb_yaxistyp->addItems( sl_yaxistype );
   //cb_yaxistyp->setCurrentIndex( 1 );

   le_ymin         = us_lineedit( "1", 0, false );
   le_ymax         = us_lineedit( "5", 0, false );

   //Add objectName
   le_ymin      -> setObjectName( "ymin" );
   le_ymax      -> setObjectName( "ymax" );

   cb_zaxistyp     = new QComboBox( this );
   cb_zaxistyp->addItems( sl_zaxistyp );
   cb_zaxistyp->setEnabled( false );

   //le_zvalue       = us_lineedit( "0.732", 0, false );
   le_zvalue       = us_lineedit( "0", 0, false );
   //Add objectName
   le_zvalue      -> setObjectName( "zvalue" );
   
   
   le_varcount     = us_lineedit( "10", 0, false );
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
   le_mciters      = us_lineedit( "1", 0, false );
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
   cb_chnsel->setCurrentIndex( cchx );
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

   // connect( le_xmin,      SIGNAL( editingFinished  ( )      ),
   //          this,         SLOT  ( xmin_changed     ( )      ) );
   // connect( le_xmax,      SIGNAL( editingFinished  ( )      ),
   //          this,         SLOT  ( xmax_changed     ( )      ) );
   // connect( le_xmin, SIGNAL( textChanged ( const QString& ) ),
   // 	       this,   SLOT  ( verify_xyz ( const QString& ) ) );
   // connect( le_xmax, SIGNAL( textChanged ( const QString& ) ),
   // 	       this,   SLOT  ( verify_xyz ( const QString& ) ) );

   connect( le_xmin, SIGNAL( editingFinished  ( ) ),
   	       this,   SLOT  ( verify_xyz ( ) ) );
   connect( le_xmax, SIGNAL( editingFinished (  ) ),
   	       this,   SLOT  ( verify_xyz ( ) ) );
   

   connect( cb_yaxistyp,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( yaxis_selected   ( int )  ) );
   
   // connect( le_ymin,      SIGNAL( editingFinished  ( )      ),
   //          this,         SLOT  ( ymin_changed     ( )      ) );
   // connect( le_ymax,      SIGNAL( editingFinished  ( )      ),
   //          this,         SLOT  ( ymax_changed     ( )      ) );
   // connect( le_ymin, SIGNAL( textChanged ( const QString& ) ),
   // 	       this,   SLOT  ( verify_xyz ( const QString& ) ) );
   // connect( le_ymax, SIGNAL( textChanged ( const QString& ) ),
   // 	       this,   SLOT  ( verify_xyz ( const QString& ) ) );
   connect( le_ymin, SIGNAL( editingFinished  ( ) ),
   	       this,   SLOT  ( verify_xyz ( ) ) );
   connect( le_ymax, SIGNAL( editingFinished (  ) ),
   	       this,   SLOT  ( verify_xyz ( ) ) );

   connect( cb_zaxistyp,  SIGNAL( activated        ( int )  ),
            this,         SLOT  ( zaxis_selected   ( int )  ) );
   
   // connect( le_zvalue,    SIGNAL( editingFinished  ( )      ),
   //          this,         SLOT  ( zvalue_changed   ( )      ) );
   // connect( le_zvalue, SIGNAL( textChanged ( const QString& ) ),
   // 	    this,   SLOT  ( verify_xyz ( const QString& ) ) );
   connect( le_zvalue,    SIGNAL( editingFinished  ( )      ),
            this,         SLOT  ( verify_xyz ( )    ) );
   

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
DbgLv(1) << "APpc: CALL initPanel()";
   initPanel();
DbgLv(1) << "APpc:  RTN initPanel()";
QString pval1 = sibSValue( "rotor", "rotor" );
DbgLv(1) << "APpc: rotor+rotor=" << pval1;
}

// PCSA Panel Slots

// NO PCSA Checked
void US_AnaprofPanPCSA::nopcsa_checked( bool chkd )
{
DbgLv(1) << "PC:SL: NOPCSA_CKD" << chkd;
}

// // Channel Selected
// void US_AnaprofPanPCSA::channel_selected( int chnx )
// {
// DbgLv(1) << "PC:SL: CHAN_SEL" << chnx;
//    int prvx    = cchx;
//    cchx        = chnx;
//    int lndx    = sl_chnsel.count() - 1;
//    pb_nextch->setEnabled( chnx < lndx );

// DbgLv(1) << "PC:SL: CHAN_SEL" << chnx << "prvx" << prvx;
//    // Save parameters from previous channel and load current
//    gui_to_parms( prvx );
//    parms_to_gui( chnx );

//    // Enable/disable Next based on last index
//    pb_nextch->setEnabled( chnx < lndx );
// }

// ALEXEY: Channel Selected modified to account for arbitrary chennle selection
void US_AnaprofPanPCSA::channel_selected( int chnx )
{
DbgLv(1) << "PC:SL: CHAN_SEL" << chnx;
   int prvx    = cchx;
   cchx        = chnx;
   //int lndx    = sl_chnsel.count() - 1;
   int lndx = 0;
   if ( active_items_pcsa.size() )
     lndx = active_items_pcsa.last();
   
   pb_nextch->setEnabled( chnx < lndx );

DbgLv(1) << "PC:SL: CHAN_SEL" << chnx << "prvx" << prvx;
   // Save parameters from previous channel and load current
   gui_to_parms( prvx );
   parms_to_gui( chnx );

   // Enable/disable Next based on last index
   pb_nextch->setEnabled( chnx < lndx );
}

// // Next Channel
// void US_AnaprofPanPCSA::next_channel( )
// {
// DbgLv(1) << "PC:SL: NEXT_CHAN";
//    int lndx    = sl_chnsel.count() - 1;
//    int chnx    = qMin( cb_chnsel->currentIndex() + 1, lndx );
//    cb_chnsel->setCurrentIndex( chnx );

//    channel_selected( chnx );
// }

// ALEXEY: Next Channel taking into account arbitrary selection of channels in Gen tab
void US_AnaprofPanPCSA::next_channel( )
{
DbgLv(1) << "PC:SL: NEXT_CHAN";
  int lndx = 0;
  if ( active_items_pcsa.size() )
    lndx = active_items_pcsa.last();

  for (int i=0; i<active_items_pcsa.size(); i++)
    qDebug() << "Active_Items: ii, value " << i << active_items_pcsa[i];

  qDebug() << "Last 2dsa index: " << lndx;
  qDebug() << "cb_chnsel->currentIndex(): " << cb_chnsel->currentIndex();

  int active_curr_ind = active_items_pcsa.indexOf( cb_chnsel->currentIndex() );

  qDebug() << "Active curr Ind (PCSA): " << active_curr_ind;
  int chnx    = qMin( active_items_pcsa[ active_curr_ind +1 ], lndx );
  cb_chnsel->setCurrentIndex( chnx );

  channel_selected( chnx );
}


// Transfer gui elements to parms vector row
void US_AnaprofPanPCSA::gui_to_parms( int rowx )
{
   US_AnaProfile* currProf  = &mainw->currProf;
   US_AnaProfile::AnaProfPCSA::ParmPCSA parm1;
   int kchan       = currProf->pchans.count();
   int kparm       = currProf->apPCSA.parms.size();
DbgLv(1) << "PC:SL-gp: rowx" << rowx << "kchan" << kchan << "kparm" << kparm;
   if ( rowx >= kparm )
   {
DbgLv(1) << "PC:SL-gp: *ERROR* rowx>=kparm";
      return;
   }

   parm1.x_min       = le_xmin    ->text().toDouble();
   parm1.x_max       = le_xmax    ->text().toDouble();
   parm1.y_min       = le_ymin    ->text().toDouble();
   parm1.y_max       = le_ymax    ->text().toDouble();
   parm1.z_value     = le_zvalue  ->text().toDouble();
   parm1.tr_alpha    = le_regalpha->text().toDouble();
   parm1.varcount    = le_varcount->text().toInt();
   parm1.grf_iters   = le_grfiters->text().toInt();
   parm1.creso_pts   = le_crpoints->text().toInt();
   parm1.mc_iters    = le_mciters ->text().toInt();
   parm1.noise_flag  = 0;
   parm1.noise_flag += ( ck_tinoise ->isChecked() ? 1 : 0 );
   parm1.noise_flag += ( ck_rinoise ->isChecked() ? 2 : 0 );
   parm1.treg_flag   = 0;
   parm1.treg_flag   = ( ck_tregspec->isChecked() ? 1 : 0 );

   //ALEXEY : bug fixed
   //parm1.treg_flag   = ( ck_tregauto->isChecked() ? 2 : 0 );
   if ( !ck_tregspec->isChecked() )
     parm1.treg_flag   = ( ck_tregauto->isChecked() ? 2 : 0 );

   //ALEXEY: handle treg_type text:
   if ( parm1.treg_flag == 0 )
     parm1.treg_type = "none";
   if ( parm1.treg_flag == 1 )
     parm1.treg_type = "specified_alpha";
   if ( parm1.treg_flag == 2 )
     parm1.treg_type = "auto_computed_alpha";
   
   parm1.curv_type   = cb_curvtype->currentText();
   parm1.x_type      = cb_xaxistyp->currentText();
   parm1.y_type      = cb_yaxistyp->currentText();
   parm1.z_type      = cb_zaxistyp->currentText();
   parm1.channel     = sl_chnsel[ rowx ];
DbgLv(1) << "PC:SL-gp:  curv_type" << parm1.curv_type << "channel" << parm1.channel;

 qDebug() << "In gui_to_parms:  ck_tregspec->isChecked(), parm1.treg_flag -- " <<  ck_tregspec->isChecked() << parm1.treg_flag;

   currProf->apPCSA.parms.replace( rowx, parm1 );
}

// Transfer parms vector element to gui elements
void US_AnaprofPanPCSA::parms_to_gui( int rowx )
{
   US_AnaProfile* currProf  = &mainw->currProf;
   QVector< US_AnaProfile::AnaProfPCSA::ParmPCSA >*
      parms        = &currProf->apPCSA.parms;
   int kchan       = currProf->pchans.count();
   int kparm       = currProf->apPCSA.parms.size();
DbgLv(1) << "PC:SL-pg: rowx" << rowx << "kchan" << kchan << "kparm" << kparm;
   if ( rowx >= kparm )
   {
DbgLv(1) << "PC:SL-pg: *ERROR* rowx>=kparm";
      return;
   }
   US_AnaProfile::AnaProfPCSA::ParmPCSA
      parm1        = parms->at( rowx );
   QString chan    = parm1.channel;
   QString chan_g  = cb_chnsel->itemText( rowx );
   QString chan_l  = sl_chnsel[ rowx ];
DbgLv(1) << "PC:SL-pg:  rowx" << rowx << "channel" << chan_g << chan << chan_l;

   int ixcurv      = cb_curvtype->findText( parm1.curv_type );
   if ( ixcurv < 0 )
   {  // Curve type not found, so search for starting substring
      for ( int ii = 0; ii < cb_curvtype->count(); ii++ )
      {
         if ( cb_curvtype->itemText( ii ).startsWith( parm1.curv_type ) )
         {
            ixcurv          = ii;
DbgLv(1) << "PC:SL-pg:   ixcurv" << ixcurv << parm1.curv_type << cb_curvtype->itemText(ii);
            break;
         }
      }
   }

   if ( chan != chan_g )
   {  // Parms channel wrong, so replace with correct string
      parm1.channel     = chan_g;
      currProf->apPCSA.parms.replace( rowx, parm1 );
DbgLv(1) << "PC:SL-pg:    chan replaced:" << parms->at( rowx ).channel;
      chan              = chan_g;
   }

   int ixchns      = cb_chnsel->findText( chan );
   if ( ixchns < 0 )
   {  // Channel string not found, so search for starting substring
      for ( int ii = 0; ii < cb_chnsel->count(); ii++ )
      {
         if ( cb_chnsel->itemText( ii ).startsWith( chan ) )
         {
            ixchns          = ii;
DbgLv(1) << "PC:SL-pg:   ixchns(I)" << ixchns << chan << cb_chnsel->itemText(ii);
            break;
         }
      }
   }
else
 DbgLv(1) << "PC:SL-pg:   ixchns(X)" << ixchns << chan << cb_chnsel->itemText(ixchns);

   le_xmin    ->setText( QString::number( parm1.x_min     ) );
   le_xmax    ->setText( QString::number( parm1.x_max     ) );
   le_ymin    ->setText( QString::number( parm1.y_min     ) );
   le_ymax    ->setText( QString::number( parm1.y_max     ) );
   le_zvalue  ->setText( QString::number( parm1.z_value   ) );
   le_regalpha->setText( QString::number( parm1.tr_alpha  ) );
   le_varcount->setText( QString::number( parm1.varcount  ) );
   le_grfiters->setText( QString::number( parm1.grf_iters ) );
   le_crpoints->setText( QString::number( parm1.creso_pts ) );
   le_mciters ->setText( QString::number( parm1.mc_iters  ) );
   ck_tinoise ->setChecked( ( parm1.noise_flag & 1 ) != 0 );
   ck_rinoise ->setChecked( ( parm1.noise_flag & 2 ) != 0 );
   ck_tregspec->setChecked( ( parm1.treg_flag  & 1 ) != 0 );
   ck_tregauto->setChecked( ( parm1.treg_flag  & 2 ) != 0 );
   cb_curvtype->setCurrentIndex( ixcurv );
   cb_xaxistyp->setCurrentIndex( cb_xaxistyp->findText( parm1.x_type    ) );
   cb_yaxistyp->setCurrentIndex( cb_yaxistyp->findText( parm1.y_type    ) );
   cb_zaxistyp->setCurrentIndex( cb_zaxistyp->findText( parm1.z_type    ) );
   cb_chnsel  ->setCurrentIndex( ixchns );
DbgLv(1) << "PC:SL-pg:  curv_type" << parm1.curv_type << "channel" << chan;
DbgLv(1) << "PC:SL-pg:   curvtype ndx" << cb_curvtype->currentIndex()
 << "chnsel ndx" << cb_chnsel->currentIndex();

 qDebug() << "In param_to_gui:  parm1.treg_flag -- " << parm1.treg_flag;
 
   pb_nextch  ->setEnabled( rowx < ( sl_chnsel.count() - 1 ) );
}

void US_AnaprofPanPCSA::apply_all_clicked( )
{
  DbgLv(1) << "PC:SL: APLALL_CLK";

  
  int active_curr_ind = active_items_pcsa.indexOf( cb_chnsel->currentIndex() );
  int chnx    = active_items_pcsa[ active_curr_ind ];

  qDebug() << "Current chan index -- " <<  active_curr_ind << chnx;

  US_AnaProfile* currProf  = &mainw->currProf;
  QVector< US_AnaProfile::AnaProfPCSA::ParmPCSA >* parms = &currProf->apPCSA.parms;
  int kchan       = currProf->pchans.count();
  int kparm       = currProf->apPCSA.parms.size();
  qDebug() << "Size of chans, parms -- " << kchan <<  kparm;

  //Save current chan parameters
  gui_to_parms( chnx );

  //Get current row parameters:
  US_AnaProfile::AnaProfPCSA::ParmPCSA parm_curr  = parms->at( chnx );

  //Replace params for all other channels with the current ones:
  for ( int i = 0; i < kparm; ++i  )
    {
      QString chan_g  = parms->at( i ).channel;
      currProf->apPCSA.parms.replace( i, parm_curr );
      currProf->apPCSA.parms[ i ].channel = chan_g;
    }
  

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

//ALEXEY: New xyz validator
//void US_AnaprofPanPCSA::verify_xyz( const QString& text )
void US_AnaprofPanPCSA::verify_xyz(  )
{
  QObject* sobj       = sender();      // Sender object
  QString oname       = sobj->objectName();

  QString text;
  
  qDebug() << "QLineEdit oname -- " << oname;
  QLineEdit * curr_widget = NULL;

  QRegExp rx_double;

  
  if ( oname.contains("xmin") )
    {
      curr_widget = le_xmin;
      rx_double.setPattern("[+-]?\\d*\\.?\\d+");
    }
  if ( oname.contains("xmax") )
    {
      curr_widget = le_xmax;
      rx_double.setPattern("[+-]?\\d*\\.?\\d+");
    }
  if ( oname.contains("ymin") )
    {
      curr_widget = le_ymin;
      rx_double.setPattern("\\d*\\.?\\d+");
    }
  if ( oname.contains("ymax") )
    {
      curr_widget = le_ymax;
      rx_double.setPattern("\\d*\\.?\\d+");
    }
  if ( oname.contains("zvalue") )
    {
      curr_widget = le_zvalue;
      rx_double.setPattern("\\d*\\.?\\d+");
    }
  
  
  if ( curr_widget != NULL )
    {
      text = curr_widget->text();
      
      if ( !rx_double.exactMatch( text ) )
	{
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::white);
	  palette->setColor(QPalette::Base,Qt::red);
	  curr_widget->setPalette(*palette);

	  isErrorField[ oname ] = true;
	}
      else
	{
	  QPalette *palette = new QPalette();
	  palette->setColor(QPalette::Text,Qt::black);
	  palette->setColor(QPalette::Base,Qt::white);
	  curr_widget->setPalette(*palette);

	  isErrorField[ oname ] = false;

	  double x = text.toDouble();

	  //for xmin/xmax && y_min/y_max:
	  bind_min_max( oname, x );
	    
	}
    }
}

//Bind xmin/xmax or ymin/ymax
void US_AnaprofPanPCSA::bind_min_max( QString oname, double x )
{
  //x_min 
  if ( oname == "xmin" )
    {
      if ( x > 0 )
	{
	  if ( x < 0.1 )
	    {
	      le_xmin -> setText( QString::number( 0.1 ));
	      if ( le_xmax->text().toDouble() <= 0.1 )
		le_xmax -> setText( QString::number( 10 ));
	    }
	  else
	    {
	      if ( le_xmax->text().toDouble() < x )
		le_xmax -> setText( QString::number( x + 10 ));
	    }
	}

      if ( x < 0 )
	{
	  if ( x > -0.1 )
	    {
	      le_xmin -> setText( QString::number( -10 ));
	      le_xmax -> setText( QString::number( -0.1 ));
	    }
	  else
	    {
	      le_xmax -> setText( QString::number( -0.1 ));
	    }
	}
    }

  //x_max
  if ( oname == "xmax" )
    {
      if ( le_xmin->text().toDouble() > 0 )
	{
	  if ( x < le_xmin->text().toDouble() )
	    le_xmax -> setText( QString::number( le_xmin->text().toDouble() + 0.01 ));
	}
      if ( le_xmin->text().toDouble() < 0 )
	{
	  if ( x < le_xmin->text().toDouble() )
	    {
	      double new_xmax = le_xmin->text().toDouble() + 1;
	      if ( new_xmax < -0.1 ) 
		le_xmax -> setText( QString::number( new_xmax ));
	      else
		le_xmax -> setText( QString::number( -0.1 ));
	    }
	  else
	    {
	      if (  x > -0.1 )
		le_xmax -> setText( QString::number( -0.1 ));
	    }
		  
	}
    }

  //y_min
  if ( oname == "ymin" )
    {
      QString ytype =  cb_yaxistyp->currentText(  );

      //qDebug() << "In bind_min_max: ytype -- " << ytype;
      
      if ( ytype == "f/f0" ) 
	{
	  if ( le_ymin->text().toDouble() < 1.0 )
	    le_ymin -> setText( QString::number( 1 ) );
	}
    }

  //y_max
  if ( oname == "ymax" )
    {
      QString ytype =  cb_yaxistyp->currentText(  );
      
      if ( ytype == "f/f0" ) 
	{
	  if ( le_ymax->text().toDouble() < le_ymin->text().toDouble() )
	    le_ymax -> setText( QString::number( le_ymin->text().toDouble() + 0.01) );
	}
    }

  //z_value
  if ( oname == "zvalue" )
    {
      QString ztype =  cb_zaxistyp->currentText(  );
      qDebug() << "In bind_min_max: ztype -- " << ztype;
      
      if ( ztype == "f/f0" ) 
	{
	  if ( le_zvalue->text().toDouble() < 1.0 )
	    le_zvalue -> setText( QString::number( 1 ));
	}

      if ( ztype == "vbar" ) 
	{
	  if ( le_zvalue->text().toDouble() < 0 )
	    le_zvalue -> setText( QString::number( 0 ) );
	}
      
    }
  
}

//Check systax errors in the xyz fields
int US_AnaprofPanPCSA::check_syntax_xyz( void )
{
  int syntax_errors = 0;
  
  QMap<QString, bool>::iterator ri;
  for ( ri = isErrorField.begin(); ri != isErrorField.end(); ++ri )
    {
      if ( ri.value() )
	{
	  qDebug() << "Error for object name -- " << ri.key();
	  ++syntax_errors;
	}
    }

  return syntax_errors;
}


// Y Axis selected
void US_AnaprofPanPCSA::yaxis_selected( int yaxx )
{
  DbgLv(1) << "PC:SL: YAXIS_SEL" << yaxx << cb_yaxistyp->itemText( yaxx );
  QString ytype =  cb_yaxistyp->itemText( yaxx );

  if ( ytype == "f/f0" || ytype == "mw" )
    {
      if ( ytype == "f/f0" )
	{
	  //Default f/f0 range:
	  le_ymin -> setText( QString::number( 1 ));
	  le_ymax -> setText( QString::number( 4 ));
	}

      if ( ytype == "mw" )
	{
	  //Default mw range:
	  le_ymin -> setText( QString::number( 1 ));
	  le_ymax -> setText( QString::number( 1000 ));
	}
      
      int ind_z = cb_zaxistyp->findText("vbar");
      if ( ind_z != -1  )
	{
	  cb_zaxistyp -> setCurrentIndex( ind_z );
 
	  //default vbar: 0
	  le_zvalue -> setText( QString::number( 0 ));
	}
    }
  
  if ( ytype == "vbar" )
    {
      //Default vbar range:
      le_ymin -> setText( QString::number( 0.72 ));
      le_ymax -> setText( QString::number( 10   ));
      
      int ind_z = cb_zaxistyp->findText("f/f0");
      if ( ind_z != -1  )
	{
	  cb_zaxistyp -> setCurrentIndex( ind_z );

	  //default f/f0: 1
	  le_zvalue -> setText( QString::number( 1 ));
	}
    }
  
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

