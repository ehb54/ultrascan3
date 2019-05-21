//! \file us_experiment_main.cpp
/////////////////////////////////////////
#include "us_com_project_gui.h"
#include "us_rotor_gui.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_crypto.h"
#include "us_select_item.h"
#include "us_images.h"
#include "us_select_item.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


//// Constructor:  ACADEMIC: 'Data Acquision' / us_comproject_academic programs
US_ComProjectMain::US_ComProjectMain(QString us_mode) : US_Widgets()
{
  //   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;
   window_closed = false;
   
   us_mode_bool = true;

   setWindowTitle( tr( "UltraScan Optima AUC Interface" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   //main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   //QGridLayout* statL     = new QGridLayout();
   //QHBoxLayout* buttL     = new QHBoxLayout();

   gen_banner = us_banner( tr( "TEST PROGRAM, v. 0.1" ) );
   //gen_banner = new QLabel;
   //gen_banner->setText("TEST PROGRAM, v. 0.1");
   //gen_banner->setAlignment(Qt::AlignCenter);
   //gen_banner->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
   
   //set font
   QFont font_gen = gen_banner->font();
   font_gen.setPointSize(20);
   font_gen.setBold(true);
   gen_banner->setFont(font_gen);
   
   main->addWidget(gen_banner);

   welcome = new QTextEdit;
   welcome->setText(" <br> Welcome to the TEST PROGRAM <br> for setting up, monitoring, editing and analyzing <br> AUC experiments and produced data... <br>");
   //welcome->setMaximumHeight(120);
   welcome->setReadOnly(true);
   welcome->setAlignment(Qt::AlignCenter);
   QFont font_wel = welcome->font();
   font_wel.setPointSize(10);
   font_wel.setItalic(true);
   font_wel.setBold(true);
   welcome->setStyleSheet("color: black; background-color: #979aaa;");
   welcome->setFont(font_wel);
   QFontMetrics m (welcome -> font()) ;
   int RowHeight = m.lineSpacing() ;
   welcome -> setFixedHeight  (6* RowHeight) ;
   
   //main->addWidget(welcome);

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   //tabWidget           = new QTabWidget;
   tabWidget->setTabPosition( QTabWidget::West );
   tabWidget->tabBar()->setStyle(new VerticalTabStyle);
   
   epanExp             = new US_ExperGui   ( this );
   epanObserv          = new US_ObservGui  ( this );
   epanPostProd        = new US_PostProdGui( this );
   // epanAnalysis        = new US_AnalysisGui( this );
   // epanReport          = new US_ReportGui  ( this );
   
   //   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanExp,       tr( "1: Experiment"   ) );
   tabWidget->addTab( epanObserv,    tr( "2: Live Update" ) );
   tabWidget->addTab( epanPostProd,  tr( "3: Editing"  ) );
   // tabWidget->addTab( epanAnalysis,  tr( "4: Analysis"  ) );
   // tabWidget->addTab( epanReport,    tr( "5: Report"  ) );
   
   tabWidget->setCurrentIndex( curr_panx );
   tabWidget->tabBar()->setFixedHeight(500);
   
   //icon_path = std::getenv("ULTRASCAN");
   //qDebug() << "Path is: " << icon_path;
   //icon_path.append("/etc/"); 
   //tabWidget->setTabIcon(0,QIcon(icon_path + "setup.png"));
   //tabWidget->setTabIcon(1,QIcon(icon_path + "live_update.gif"));
   //tabWidget->setTabIcon(2,QIcon(icon_path + "analysis.png"));
   
   tabWidget->setTabIcon( 0, US_Images::getIcon( US_Images::SETUP_COM  ) );
   tabWidget->setTabIcon( 1, US_Images::getIcon( US_Images::LIVE_UPDATE_COM  ) );
   tabWidget->setTabIcon( 2, US_Images::getIcon( US_Images::ANALYSIS_COM ) );
   // tabWidget->setTabIcon( 3, US_Images::getIcon( US_Images::ANALYSIS_COM_2 ) );
   // tabWidget->setTabIcon( 4, US_Images::getIcon( US_Images::REPORT_COM ) );

   // if ( us_mode.toStdString() == "US_MODE")
   //   {
   //     tabWidget->removeTab(3);
   //     tabWidget->removeTab(3);
   //   }
   
   tabWidget->tabBar()->setIconSize(QSize(50,50));

   tabWidget->tabBar()->setStyleSheet("QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:hover {background: lightgray;}");
   main->addWidget( tabWidget );
   
   logWidget = us_textedit();
   logWidget->setMaximumHeight(30);
   logWidget->setReadOnly(true);
   logWidget->append("Log comes here...");
   logWidget->verticalScrollBar()->setValue(logWidget->verticalScrollBar()->maximum());
   logWidget->hide();
   //main->addWidget( logWidget );
   
   test_footer = new QTextEdit;
   test_footer->setText("UltraScan by AUC Solutions");
   test_footer->setTextColor(Qt::white);
   test_footer->setMaximumHeight(30);
   test_footer->setReadOnly(true);
   test_footer->setStyleSheet("color: #D3D9DF; background-color: #36454f;");
   main->addWidget( test_footer );

   connect( this, SIGNAL( pass_used_instruments( QStringList & ) ), epanExp, SLOT( pass_used_instruments( QStringList &)  ) );
   
   connect( epanExp, SIGNAL( switch_to_live_update( QMap < QString, QString > &) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( this   , SIGNAL( pass_to_live_update( QMap < QString, QString > &) ),   epanObserv, SLOT( process_protocol_details( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( to_autoflow_records( ) ), this, SLOT( to_autoflow_records( ) ) );
   
   // connect( epanObserv, SIGNAL( switch_to_post_processing( QString &, QString &, QString &, SQtring &) ), this, SLOT( switch_to_post_processing( QString &, QString &, QString &, QString &) ) );
   // connect( this, SIGNAL( import_data_us_convert( QString &, QString &, QString &, QString & ) ),  epanPostProd, SLOT( import_data_us_convert( QString &, QString &, QString &, QString & )  ) );
   connect( epanObserv, SIGNAL( switch_to_post_processing( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & ) ) );
   connect( this, SIGNAL( import_data_us_convert( QMap < QString, QString > & ) ),  epanPostProd, SLOT( import_data_us_convert( QMap < QString, QString > & )  ) );
   
   connect( epanObserv, SIGNAL( switch_to_experiment( QString &) ), this, SLOT( switch_to_experiment(  QString & )  ) );
   connect( this, SIGNAL( clear_experiment( QString & ) ),  epanExp, SLOT( clear_experiment( QString & )  ) );
   connect( epanObserv, SIGNAL( close_everything() ), this, SLOT( close_all() ));
   
   //connect( epanPostProd, SIGNAL( switch_to_analysis( QString &, QString &) ),  this, SLOT( switch_to_analysis( QString &, QString & )  ) );
   //connect( this, SIGNAL( pass_to_analysis( QString &, QString & ) ),   epanAnalysis, SLOT( do_analysis( QString &, QString & )  ) );
   connect( epanPostProd, SIGNAL( switch_to_exp( QString & ) ), this, SLOT( switch_to_experiment( QString & )  ) );

   
   setMinimumSize( QSize( 1350, 800 ) );
   adjustSize();

   /* Check for current stage & redirect to specific tab */
   //check_current_stage();

}



//// Constructor:  COMMERCIAL: us_comproject
US_ComProjectMain::US_ComProjectMain() : US_Widgets()
{
  //   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;

   window_closed = false;
   us_mode_bool = false;
   
   setWindowTitle( tr( "UltraScan Optima AUC Interface" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   //main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   //QGridLayout* statL     = new QGridLayout();
   //QHBoxLayout* buttL     = new QHBoxLayout();

   gen_banner = us_banner( tr( "TEST PROGRAM, v. 0.1" ) );
   //gen_banner = new QLabel;
   //gen_banner->setText("TEST PROGRAM, v. 0.1");
   //gen_banner->setAlignment(Qt::AlignCenter);
   //gen_banner->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
   
   //set font
   QFont font_gen = gen_banner->font();
   font_gen.setPointSize(20);
   font_gen.setBold(true);
   gen_banner->setFont(font_gen);
   
   main->addWidget(gen_banner);

   welcome = new QTextEdit;
   welcome->setText(" <br> Welcome to the TEST PROGRAM <br> for setting up, monitoring, editing and analyzing <br> AUC experiments and produced data... <br>");
   //welcome->setMaximumHeight(120);
   welcome->setReadOnly(true);
   welcome->setAlignment(Qt::AlignCenter);
   QFont font_wel = welcome->font();
   font_wel.setPointSize(10);
   font_wel.setItalic(true);
   font_wel.setBold(true);
   welcome->setStyleSheet("color: black; background-color: #979aaa;");
   welcome->setFont(font_wel);
   QFontMetrics m (welcome -> font()) ;
   int RowHeight = m.lineSpacing() ;
   welcome -> setFixedHeight  (6* RowHeight) ;
   
   //main->addWidget(welcome);

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   //tabWidget           = new QTabWidget;
   tabWidget->setTabPosition( QTabWidget::West );
   tabWidget->tabBar()->setStyle(new VerticalTabStyle);
   
   epanExp             = new US_ExperGui   ( this );
   epanObserv          = new US_ObservGui  ( this );
   epanPostProd        = new US_PostProdGui( this );
   epanAnalysis        = new US_AnalysisGui( this );
   epanReport          = new US_ReportGui  ( this );
   
   //   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanExp,       tr( "1: Experiment"   ) );
   tabWidget->addTab( epanObserv,    tr( "2: Live Update" ) );
   tabWidget->addTab( epanPostProd,  tr( "3: Editing"  ) );
   tabWidget->addTab( epanAnalysis,  tr( "4: Analysis"  ) );
   tabWidget->addTab( epanReport,    tr( "5: Report"  ) );
   
   tabWidget->setCurrentIndex( curr_panx );
   tabWidget->tabBar()->setFixedHeight(500);
   
   //icon_path = std::getenv("ULTRASCAN");
   //qDebug() << "Path is: " << icon_path;
   //icon_path.append("/etc/"); 
   //tabWidget->setTabIcon(0,QIcon(icon_path + "setup.png"));
   //tabWidget->setTabIcon(1,QIcon(icon_path + "live_update.gif"));
   //tabWidget->setTabIcon(2,QIcon(icon_path + "analysis.png"));
   
   tabWidget->setTabIcon( 0, US_Images::getIcon( US_Images::SETUP_COM  ) );
   tabWidget->setTabIcon( 1, US_Images::getIcon( US_Images::LIVE_UPDATE_COM  ) );
   tabWidget->setTabIcon( 2, US_Images::getIcon( US_Images::ANALYSIS_COM ) );
   tabWidget->setTabIcon( 3, US_Images::getIcon( US_Images::ANALYSIS_COM_2 ) );
   tabWidget->setTabIcon( 4, US_Images::getIcon( US_Images::REPORT_COM ) );
   
   tabWidget->tabBar()->setIconSize(QSize(50,50));

   tabWidget->tabBar()->setStyleSheet("QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:hover {background: lightgray;}");
   main->addWidget( tabWidget );
   
   logWidget = us_textedit();
   logWidget->setMaximumHeight(30);
   logWidget->setReadOnly(true);
   logWidget->append("Log comes here...");
   logWidget->verticalScrollBar()->setValue(logWidget->verticalScrollBar()->maximum());
   logWidget->hide();
   //main->addWidget( logWidget );
   
   test_footer = new QTextEdit;
   test_footer->setText("UltraScan by AUC Solutions");
   test_footer->setTextColor(Qt::white);
   test_footer->setMaximumHeight(30);
   test_footer->setReadOnly(true);
   test_footer->setStyleSheet("color: #D3D9DF; background-color: #36454f;");
   main->addWidget( test_footer );

   connect( this, SIGNAL( pass_used_instruments( QStringList & ) ), epanExp, SLOT( pass_used_instruments( QStringList &)  ) );
   
   connect( epanExp, SIGNAL( switch_to_live_update( QMap < QString, QString > &) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( this   , SIGNAL( pass_to_live_update( QMap < QString, QString > &) ),   epanObserv, SLOT( process_protocol_details( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( to_autoflow_records( ) ), this, SLOT( to_autoflow_records( ) ) );
   
   // connect( epanObserv, SIGNAL( switch_to_post_processing( QString &, QString &, QString &, QString & ) ), this, SLOT( switch_to_post_processing( QString &, QString &, QString &, QString & )));
   // connect( this, SIGNAL( import_data_us_convert( QString &, QString &, QString &, QString & ) ),  epanPostProd, SLOT( import_data_us_convert( QString &, QString &, QString &, QString & )  ) );
   connect( epanObserv, SIGNAL( switch_to_post_processing( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & ) ) );
   connect( this, SIGNAL( import_data_us_convert( QMap < QString, QString > & ) ),  epanPostProd, SLOT( import_data_us_convert( QMap < QString, QString > & )  ) );
   
   connect( epanObserv, SIGNAL( switch_to_experiment( QString &) ), this, SLOT( switch_to_experiment(  QString & )  ) );
   connect( this, SIGNAL( clear_experiment( QString & ) ),  epanExp, SLOT( clear_experiment( QString & )  ) );
   connect( epanObserv, SIGNAL( close_everything() ), this, SLOT( close_all() ));
      
   connect( epanPostProd, SIGNAL( switch_to_analysis( QString &, QString &) ),  this, SLOT( switch_to_analysis( QString &, QString & )  ) );
   connect( this, SIGNAL( pass_to_analysis( QString &, QString & ) ),   epanAnalysis, SLOT( do_analysis( QString &, QString & )  ) );
   
   setMinimumSize( QSize( 1350, 800 ) );
   adjustSize();

   /* Check for current stage & redirect to specific tab */
   //check_current_stage();
}


void US_ComProjectMain::closeEvent( QCloseEvent* event )
{
    window_closed = true;
    emit us_comproject_closed();
    event->accept();
}

void US_ComProjectMain::to_autoflow_records( void )
{
  check_current_stage();
}


// Function that checks for current program stage based on US-lims DB entry
void US_ComProjectMain::check_current_stage( void )
{
  // (0) 'autoflow' table is initially updated with protocol name/ID & returned Optima's ExpID in us_experimet (submitExperiment(); ) 
  // (1) Query 'autoflow' table for stage#, protocol name/ID, ExperimentID 
  // (2) Query 'protocol' table for details: e.g. CellChNumber, TripleNumber
  // (3) Identify currDir where .auc data have been saved (unique name based on protocolName + runID)
  //     * this maybe an 'autoflow' table field recorded after stage 1 (Live Update); DEFAULT empty

  /*
 
    1. Query 'autoflow' table - proc. 'count_autoflow_records()': 
            if 0 - return; 
	    else
	       sort by OptimaName:
	       list active Optima machines:
	       identify which Optima is running via autoflow, which is free & offer to chose - either monitor existing run, OR submit new run on free machine
	       When to consider Optima "free" ? (after stage 'LIVE_UPDATE' passed ?)
    
    2. If autoflow record exists: 
             proc. 'read_autoflow_record()'
  
  */

  // // Ling's exp. Optima 1!!!
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "experimentId" ]   = QString("465");   
  // protocol_details[ "protocolName" ]   = QString("CCLing-PZ5077-27k-021519");
  // protocol_details[ "experimentName" ] = QString("some_name");
  // protocol_details[ "CellChNumber" ]   = QString("2");
  // protocol_details[ "TripleNumber" ]   = QString("2");
  // protocol_details[ "OptimaName" ]     = QString("Optima 1");    // <-- Optima 1
  // protocol_details[ "duration" ]       = QString("27000");
  // protocol_details[ "invID_passed" ]   = QString("34"); //Ling's ID
  
  // -------------------------------------------------------------------------------------------  

  // // Nemetchek's exp. Optima 2 !!!
  // QMap < QString, QString > protocol_details;
  // protocol_details["experimentId"] = QString("287");   
  // protocol_details["protocolName"] = QString("RxRPPARhet-PPRE-MWL_180419");
  // protocol_details[ "experimentName" ] = QString("some_name");
  // protocol_details[ "CellChNumber" ] = QString("8");
  // protocol_details[ "TripleNumber" ] = QString("38");
  // //protocol_details[ "TripleNumber" ] = QString("39"); //ALEXEY - incorrect - for testing
  // protocol_details[ "OptimaName" ] = QString("Optima 2");     // <-- Optima 2
  // protocol_details[ "duration" ]   = QString("36000");
  // protocol_details[ "dataPath" ]   = QString("/home/alexey/ultrascan/imports/RxRPPARhet-PPRE-MWL_180419-run352");
  // protocol_details[ "invID_passed" ]  = QString("41");  //Nemetchek's ID
  
  // QString stage                        = "LIVE_UPDATE";
  // QString currDir                      = protocol_details[ "dataPath" ];
  // QString ProtName                     = protocol_details[ "protocolName" ];
  // QString invID_passed                 = protocol_details[ "invID_passed" ];


  // // --------------------------------------------------------------------------------------------
  
  // // Amy's exp. Optima 2 !!!
  // QMap < QString, QString > protocol_details;
  // protocol_details["experimentId"] = QString("299");   
  // protocol_details["protocolName"] = QString("data-aquisition-test29");
  // protocol_details[ "experimentName" ] = QString("some_name");
  // protocol_details[ "CellChNumber" ] = QString("8");
  // protocol_details[ "TripleNumber" ] = QString("12");
  // protocol_details[ "OptimaName" ] = QString("Optima 2");     // <-- Optima 2
  // protocol_details[ "duration" ]   = QString("300");
  // protocol_details[ "dataPath" ]   = QString("/home/alexey/ultrascan/imports/data-aquisition-test29-run364");
  // protocol_details[ "invID_passed" ]  = QString("6");  //Amy's ID
  
  // QString stage                        = "LIVE_UPDATE";
  // QString currDir                      = protocol_details[ "dataPath" ];
  // QString ProtName                     = protocol_details[ "protocolName" ];
  // QString invID_passed                 = protocol_details[ "invID_passed" ];

  // --------------------------------------------------------------------------------------------
  
  
  // Query 'autoflow': get count of records
  int autoflow_records = get_autoflow_records();

  qDebug() << "Autoflow record #: " << autoflow_records;

  if ( autoflow_records < 1 )
    return;

  //ALEXEY:
  /*
      -- Read (and count) Optima Instruments;
      -- If the number of 'autoflow' records with the status LIVE_UPDATE && RUNNING is the same as # instruments, disable 'Def. New Exp.' buttion
      -- Identify free Optima(s) & pass the list to the us_experiment (Lab/Rotors tab -> instruments)
      -- 
   */


  // Dialog of existing autoflow records
  US_Passwd  pw;
  US_DB2* dbP  = new US_DB2( pw.getPasswd() );

  //read_optima_machines
  read_optima_machines( dbP );

  //read autoflow records
  list_all_autoflow_records( autoflowdata, dbP );

  //count instruments in use
  occupied_instruments.clear();
  for ( int i=0; i < autoflowdata.size(); i++ )
    {
      if ( autoflowdata[ i ][ 5 ] == "LIVE_UPDATE" )
	occupied_instruments << autoflowdata[ i ][ 2 ];
    }
  
  
  QString pdtitle( tr( "Select Optima Run to Follow" ) );
  QStringList hdrs;
  int         prx;
  hdrs << "ID"
       << "Run Name"
       << "Optima Name"
       << "Created"
       << "Run Status"
       << "Stage";
  
  QString autoflow_btn = "AUTOFLOW";
  
  //US_SelectItem pdiag( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );
  US_SelectItem* pdiag = new  US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );
  
  pdiag->setParent(this, Qt::Window);
  pdiag->setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint  );
  //pdiag->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint  );

  //disable 'Define Another Exp.' button if all instruments are in use
  if ( occupied_instruments.size() >= instruments.size() )
    pdiag->pb_cancel->setEnabled( false );
  
  
  QString autoflow_id_selected("");
  
  if ( pdiag->exec() == QDialog::Accepted )
    autoflow_id_selected  = autoflowdata[ prx ][ 0 ];
  else
    {
      //ALEXEY: define what to do if some Optima(s) are occupied
      // should emit signal sending list of optima's in use to us_experiment.

      QString list_instruments_in_use = occupied_instruments.join(", ");
      QMessageBox::warning( this, tr( "Occupied Instruments" ),
			    tr( "The following Optima instrument(s) \n"
				"are currently in use: \n\n"
				"%1 \n\n"
				"You will not be able to submit another run \n"
				"to these instruments at the moment.").arg( list_instruments_in_use ) );
      
      define_new_experiment( occupied_instruments );
      return;
    }

  // -------------------------------------------------------------------------------------------------
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  int autoflowID = autoflow_id_selected.toInt();

  protocol_details = read_autoflow_record( autoflowID );

  QString stage        = protocol_details[ "status" ];
  QString currDir      = protocol_details[ "dataPath" ];
  QString invID_passed = protocol_details[ "invID_passed" ];
  QString ProtName     = protocol_details[ "protocolName" ];
  QString correctRadii = protocol_details[ "correctRadii" ];
  QString expAborted   = protocol_details[ "expAborted" ];

  QDir directory( currDir );
  
  qDebug() << "CURR DIRECTORY : " << currDir;
  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

 
  //ALEXEY: if stage=="EDITING" && curDir.isEmpty() (NULL)
  /*
        -- that means that Run Completed but Directory was created on different computer
	-- possible solution: return (set) to stage == "LIVE_UPDATE" & re-save to local directory on current computer
    
  */
  
  if ( stage == "LIVE_UPDATE" )
    {
      //do something
      switch_to_live_update( protocol_details );
      return;
    }
  
  if ( stage == "EDITING" )
    {
      //do something
      //switch_to_post_processing( currDir, ProtName, invID_passed, correctRadii );

      if ( currDir.isEmpty() || !directory.exists() )
	switch_to_live_update( protocol_details );
      else	
	switch_to_post_processing( protocol_details );
      
     
      return;
    }
  //and so on...
  
  
}

// Slot to read all Optima machines <------------------------------- // New
void US_ComProjectMain::read_optima_machines( US_DB2* db )
{
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );
  db->query( q );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no instruments defined
    {
      QList< int > instrumentIDs;
      
      while ( db->next() )
	{
	  int ID = db->value( 0 ).toString().toInt();
	  instrumentIDs << ID;
	  
	  qDebug() << "InstID: " << ID;
	}
      
      // Instrument information
      foreach ( int ID, instrumentIDs )
	{
	  QMap<QString,QString> instrument;
	  
	  q.clear();
	  q  << QString( "get_instrument_info_new" )
	     << QString::number( ID );
	  db->query( q );
	  db->next();

	  instrument[ "ID" ]              =   QString::number( ID );
	  instrument[ "name" ]            =   db->value( 0 ).toString();
	  instrument[ "serial" ]          =   db->value( 1 ).toString();
	  instrument[ "optimaHost" ]      =   db->value( 5 ).toString();	   
	  instrument[ "optimaPort" ]      =   db->value( 6 ).toString(); 
	  instrument[ "optimaDBname" ]    =   db->value( 7 ).toString();	   
	  instrument[ "optimaDBusername" ] =  db->value( 8 ).toString();	   
	  instrument[ "optimaDBpassw" ]    =  db->value( 9 ).toString();	   
	  instrument[ "selected" ]        =   db->value( 10 ).toString();
	    
	  instrument[ "opsys1" ]  = db->value( 11 ).toString();
	  instrument[ "opsys2" ]  = db->value( 12 ).toString();
	  instrument[ "opsys3" ]  = db->value( 13 ).toString();

	  instrument[ "radcalwvl" ]  =  db->value( 14 ).toString();
	  instrument[ "chromoab" ]   =  db->value( 15 ).toString();

	  
	  if ( instrument[ "name" ].contains("Optima") || instrument[ "optimaHost" ].contains("AUC_DATA_DB") )
	    this->instruments << instrument;
	}
    }
  qDebug() << "Reading Instrument: FINISH";
}



// Query autoflow for # records
int US_ComProjectMain::list_all_autoflow_records( QList< QStringList >& autoflowdata, US_DB2* dbP )
{
  int nrecs        = 0;   
  autoflowdata.clear();

  QStringList qry;
  qry << "get_autoflow_desc";
  dbP->query( qry );

  if ( dbP->lastErrno() != US_DB2::OK )
    return nrecs;
  
  while ( dbP->next() )
    {
      QStringList autoflowentry;
      QString id                 = dbP->value( 0 ).toString();
      QString runname            = dbP->value( 5 ).toString();
      QString status             = dbP->value( 8 ).toString();
      QString optimaname         = dbP->value( 10 ).toString();
      
      QDateTime time_started     = dbP->value( 11 ).toDateTime().toUTC();

      QDateTime time_created     = dbP->value( 13 ).toDateTime().toUTC();
      QDateTime local(QDateTime::currentDateTime());

      autoflowentry << id << runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

      if ( time_started.toString().isEmpty() )
	autoflowentry << QString( tr( "NOT STARTED" ) );
      else
	{
	  if ( status == "LIVE_UPDATE" )
	    autoflowentry << QString( tr( "RUNNING" ) );
	  if ( status == "EDITING" )
	    autoflowentry << QString( tr( "COMPLETED" ) );
	    //autoflowentry << time_started.toString();
	}
      
      autoflowentry << status;
      
      autoflowdata  << autoflowentry;
      nrecs++;
    }

  return nrecs;
}

    
// Query autoflow for # records
int US_ComProjectMain::get_autoflow_records( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   int record_number = 0;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return record_number;
     }

   QStringList qry;
   qry << "count_autoflow_records";
   
   record_number = db->functionQuery( qry );

   return record_number;
}


// Query autoflow for # records
QMap< QString, QString> US_ComProjectMain::read_autoflow_record( int autoflowID  )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   QMap <QString, QString> protocol_details;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return protocol_details;
     }

   QStringList qry;
   qry << "read_autoflow_record"
       << QString::number( autoflowID );
   
   db->query( qry );

   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   protocol_details[ "protocolName" ]   = db->value( 0 ).toString();
	   protocol_details[ "CellChNumber" ]   = db->value( 1 ).toString();
	   protocol_details[ "TripleNumber" ]   = db->value( 2 ).toString();
	   protocol_details[ "duration" ]       = db->value( 3 ).toString();
	   protocol_details[ "experimentName" ] = db->value( 4 ).toString();
	   protocol_details[ "experimentId" ]   = db->value( 5 ).toString();
	   protocol_details[ "runID" ]          = db->value( 6 ).toString();
	   protocol_details[ "status" ]         = db->value( 7 ).toString();
           protocol_details[ "dataPath" ]       = db->value( 8 ).toString();   
	   protocol_details[ "OptimaName" ]     = db->value( 9 ).toString();
	   protocol_details[ "runStarted" ]     = db->value( 10 ).toString();
	   protocol_details[ "invID_passed" ]   = db->value( 11 ).toString();

	   protocol_details[ "correctRadii" ]   = db->value( 13 ).toString();
	   protocol_details[ "expAborted" ]     = db->value( 14 ).toString();
	 }
     }

   return protocol_details;
}


// Slot to define new exp. (from initial dialog)
void US_ComProjectMain::define_new_experiment( QStringList & occupied_instruments )
{
  tabWidget->setCurrentIndex( 0 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ?? 

  qDebug() << "In define_new_experiment( QStringList & occupied_instruments )";
    
  emit pass_used_instruments( occupied_instruments );
}


// Slot to pass submitted to Optima run info to the Live Update tab
void US_ComProjectMain::switch_to_live_update( QMap < QString, QString > & protocol_details)
{
  tabWidget->setCurrentIndex( 1 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ?? 

   // ALEXEY:
   // (1) Make a record to 'autoflow' table - stage# = 1;
   // (2) inside us_xpn_viewer - update 'curDirr' field with generated directory where .auc data saved 
   
   emit pass_to_live_update( protocol_details );
}

// Slot to pass submitted to Optima run info to the Live Update tab
void US_ComProjectMain::close_all( void )
{
  tabWidget->setCurrentIndex( 0 );   
  qDebug() << "CLOSING PROGRAM !!!";

  close();
  
  // QProcess process;
  // QString pgm("pgrep");
  // QStringList args = QStringList() << "us_comproject";
  // process.start(pgm, args);
  // process.waitForReadyRead();
  // if(!process.readAllStandardOutput().isEmpty()) 
  //   {
  //     //app still running
  //     //Linux
  //     system("pkill us_comproject");
  //     //Windows
  //     system("taskkill /im us_comproject /f");
      
  //     process.kill();
  //   }

  //Linux
  //system("pkill us_comproject");

  
  //system("pkill us_comproject_academic");
  //Windows//
  //system("taskkill /im us_comproject /f");
  //system("taskkill /im us_comproject_academic /f");
  //Mac ??
  
}

// // Slot to switch from the Live Update to Editing tab
// void US_ComProjectMain::switch_to_post_processing( QString  & currDir, QString & protocolName,  QString & invID_passed, QString & correctRadii )
// {
//    tabWidget->setCurrentIndex( 2 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??

//    // ALEXEY: Make a record to 'autoflow' table: stage# = 2; 

//    emit import_data_us_convert( currDir, protocolName, invID_passed, correctRadii );
// }

// Slot to switch from the Live Update to Editing tab
void US_ComProjectMain::switch_to_post_processing( QMap < QString, QString > & protocol_details )
{
  tabWidget->setCurrentIndex( 2 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  
  // ALEXEY: Make a record to 'autoflow' table: stage# = 2; 
  
  emit import_data_us_convert( protocol_details );
}
     
// Slot to switch back from the Live Update to Experiment tab
void US_ComProjectMain::switch_to_experiment( QString & protocolName )
{
   tabWidget->setCurrentIndex( 0 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??

   //delete_autoflow_record( runID );
   
   emit clear_experiment( protocolName );
}
   

// Slot to switch from the Editing to Analysis tab
void US_ComProjectMain::switch_to_analysis( QString  & currDir, QString & protocolName)
{
   tabWidget->setCurrentIndex( 3 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  
   // ALEXEY: Make a record to 'autoflow' table: stage# = 3; 

   emit pass_to_analysis( currDir, protocolName );
}



// US_ExperGUI
US_ExperGui::US_ExperGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   //lb_exp_banner    = us_banner( tr( "1: Define Experiment Run" ) );
   //panel->addWidget( lb_exp_banner );

   // Create layout and GUI components
   QGridLayout* genL   = new QGridLayout();
   pb_openexp = new QPushButton();
   
   // QString temp_path = std::getenv("ULTRASCAN");
   // temp_path += "/etc/";
   
   // //pb_openexp  = us_pushbutton( tr( "Open Experiment SetUp Dialog" ) );
   // pb_openexp  = new QPushButton(this);
   // //xpb_openexp->setGeometry(QRect(0, 0, 150, 100));
   // pb_openexp->setFixedSize( QSize(285, 55) );
   // pb_openexp->setText("  Setup New Experiment ");
   // pb_openexp->setLayoutDirection(Qt::RightToLeft);
   // pb_openexp->setIcon(QIcon(temp_path + "new_item.png"));
   // pb_openexp->setIconSize(QSize(20,20));
   
   // qDebug() << "Icon: " << temp_path + "new_item.png";
   // pb_openexp->setStyleSheet("QPushButton{background-color: #318CE7; border: 1px solid white; background-image: url(temp_path); color: black; border-radius: 15px; font-size: 18px; font-weight: bold; } QPushButton:hover{background-color: #6699CC;} QPushButton:disabled{background-color:#6699CC ; color: white}");
   
   //QString iconpath = temp_path + "new_item2.png";
   //QPixmap* pixmap = new QPixmap(iconpath);
   //QIcon icon(*pixmap);
   //QSize iconSize(pixmap->width(), pixmap->height());
   //pb_openexp->setIconSize(iconSize);
   //pb_openexp->setIcon(icon);

   pb_openexp->setIcon(US_Images::getIcon( US_Images::NEW_ITEM_COM ) );
   pb_openexp->setIconSize(QSize(50,50));
   pb_openexp->setStyleSheet("QPushButton{border: none} QPushButton:hover{border: 1px solid gray}");
   //pb_openexp->setStyleSheet("QPushButton{background-color: #318CE7; border: 1px solid white; background-image: url(temp_path); color: black; border-radius: 15px; font-size: 18px; font-weight: bold; } QPushButton:hover{background-color: #6699CC;} QPushButton:disabled{background-color:#6699CC ; color: white}");

   
   // Build main layout
   //int row         = 0;

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Set Up New Experiment...");
   // panel_desc->setReadOnly(true);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;
   
   // genL->addWidget( panel_desc,  row,   0, 1, 12);

   
   //genL->addWidget( pb_openexp,  row,   12, 1, 1, Qt::AlignTop);
   // genL->addWidget( panel_desc,  row,   0, 1, 2);
   // genL->addWidget( pb_openexp,  row,   2, 1, 2, Qt::AlignCenter);

   panel->addLayout( genL );
   panel->addStretch();

   // connect( pb_openexp,      SIGNAL( clicked()          ), 
   //          this,            SLOT(   manageExperiment() ) );

   //manageExperiment();
   
   // Open US_Experiment without button...  
   //US_ExperimentMain* sdiag = new US_ExperimentMain;

   sdiag = new US_ExperimentMain;

   sdiag->setParent(this, Qt::Widget);

    
   
   connect( sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
   //connect( this,  SIGNAL( set_auto_mode() ),   sdiag, SLOT( auto_mode_passed() ) );
   
   connect( this, SIGNAL( define_used_instruments( QStringList & ) ), sdiag, SLOT( exclude_used_instruments( QStringList & ) ) );
   
   connect( sdiag, SIGNAL( to_live_update( QMap < QString, QString > & ) ),
	    this,  SLOT( to_live_update( QMap < QString, QString > & ) ) );

   connect( this, SIGNAL( reset_experiment( QString & ) ), sdiag, SLOT( us_exp_clear( QString & ) ) );
   
   connect( sdiag, SIGNAL( exp_cleared( ) ), this, SLOT( exp_cleared( ) ) );
   
   sdiag->pb_close->setEnabled(false);  // Disable Close button
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2); 

   sdiag->auto_mode_passed();

   if ( mainw->us_mode_bool )
     sdiag->us_mode_passed();
   
   sdiag->show();

   
}


void US_ExperGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

void US_ExperGui::pass_used_instruments( QStringList & occupied_instruments )
{
  qDebug() << "In US_ExperGui::pass_used_instruments( QStringList & occupied_instruments )";
  emit define_used_instruments( occupied_instruments );
}


//When run is submitted to Optima & protocol details are passed .. 
void US_ExperGui::to_live_update( QMap < QString, QString > & protocol_details)
{
  emit switch_to_live_update( protocol_details );
}

//When US_Experiment is closed
void US_ExperGui::us_exp_is_closed_set_button()
{
  pb_openexp->setEnabled(true);
  mainw->resize(QSize(1000, 700));
  mainw->logWidget->append("US_Experiment has been closed!");
}

//Clear Experiment after manual abortion & data not saved .. 
void US_ExperGui::clear_experiment( QString & protocolName )
{
  emit reset_experiment( protocolName );
}

//Upon clearing Experiment .. 
void US_ExperGui::exp_cleared( void )
{
  emit to_autoflow_records();
}


// On click to open US_Experiment  <-- NOT USED, us_experimnet is loaded immediately
void US_ExperGui::manageExperiment()
{
  qDebug() << "test00";
  //mainw->logWidget->append("Opening US_Experiment...");
  //qApp->processEvents();

  //Calculate cumulative height/width of all widgets in Main && US_ExperGui
  int height_cum = 0;
  int offset = 20;

  qDebug() << "test0";
  
  height_cum += pb_openexp->height();
  height_cum += offset;
  
  int tab_width = mainw->tabWidget->tabBar()->width();  

  qDebug() << "test1";
  
  US_ExperimentMain* sdiag = new US_ExperimentMain;
  sdiag->setParent(this, Qt::Widget);

  connect(sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
  
  sdiag->move(offset, height_cum);
  sdiag->setFrameShape( QFrame::Box);
  sdiag->setLineWidth(2); 
  
  sdiag->show();

  qDebug() << "test2";
  
  int new_width, new_height;

  new_width  = tab_width  + sdiag->width()  + 2*offset;
  new_height = mainw->gen_banner->height() + mainw->welcome->height() + height_cum + sdiag->height() + mainw->logWidget->height() + mainw->test_footer->height() + offset;

  qDebug() << "Tab width: " << tab_width << ", US_Exp width: " << sdiag->width();
  qDebug() << "New Width: " << new_width;
  qDebug() << "New Height: " << new_height;
  
  if ( mainw->height() < new_height || mainw->width() < new_width)
    mainw->resize(new_width, new_height);

  pb_openexp->setEnabled(false);
  mainw->logWidget->append("US_Experiment opened successfully!");
}


// US_Observe /////////////////////////////////////////////////////////////////////////////////
US_ObservGui::US_ObservGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Monitor Experiment Progress...");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);


   
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Xpn_Viewer ...  
   sdiag = new US_XpnDataViewer("AUTO");
   sdiag->setParent(this, Qt::Widget);

   connect( this, SIGNAL( to_xpn_viewer( QMap < QString, QString > &) ), sdiag, SLOT( check_for_data ( QMap < QString, QString > & )  ) );

   //ALEXEY: devise SLOT saying what to do upon completion of experiment and exporting AUC data to hard drive - Import Experimental Data  !!! 
   //connect( sdiag, SIGNAL( experiment_complete_auto( QString &, QString &, QString &, QString & ) ), this, SLOT( to_post_processing ( QString &, QString &, QString &, QString & ) ) );
   connect( sdiag, SIGNAL( experiment_complete_auto( QMap < QString, QString > & ) ), this, SLOT( to_post_processing ( QMap < QString, QString > & ) ) );
   
   
   //ALEXEY: return to 1st panel when exp. aborted & no data saved..
   connect( sdiag, SIGNAL( return_to_experiment( QString & ) ), this, SLOT( to_experiment ( QString &) ) );

   //ALEXEY: close program, emitted from sdiag
   connect( sdiag, SIGNAL( close_program() ), this, SLOT( to_close_program()  ) );
   
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();

}


void US_ObservGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

// Live Update's get and use submitted run's protocol details
void US_ObservGui::process_protocol_details( QMap < QString, QString > & protocol_details )
{
  // Use protocol details to retrieve data from Optima's DB
  // Query ExperimentRun table for runname/ExpDefId
  // If null (i.e. run is not launched yet), Information box - "Run was submitted, but not launched yet.. Awaiting for data to emerge."

  // QString mtitle    = tr( "Reading Protocol" );
  // QString message   = tr( "Protocol details passed. <br> Name: %1 <br> ID: %2" ).arg(protocol_details["experimentName"]).arg(protocol_details["experimentId"]);
  // QMessageBox::information( this, mtitle, message );

  emit to_xpn_viewer( protocol_details );
}

// void US_ObservGui::to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed,  QString & correctRadii )
// {
//   emit switch_to_post_processing( currDir, protocolName, invID_passed, correctRadii );
// }

void US_ObservGui::to_post_processing( QMap < QString, QString > & protocol_details )
{
  emit switch_to_post_processing( protocol_details );
}

void US_ObservGui::to_experiment( QString & protocolName )
{
  emit switch_to_experiment( protocolName );
}

void US_ObservGui::to_close_program( void )
{
  //sdiag->close();
  emit close_everything();
}

// US_PostProd
US_PostProdGui::US_PostProdGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Retrieve and Process Experimental Data...");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();


   // Open US_Convert ...  
   sdiag = new US_ConvertGui("AUTO");
   sdiag->setParent(this, Qt::Widget);

   //connect( this, SIGNAL( to_post_prod( QString &, QString &, QString &, QString & ) ), sdiag, SLOT( import_data_auto ( QString &, QString &, QString &, QString & )  ) );
   connect( this, SIGNAL( to_post_prod( QMap < QString, QString > & ) ), sdiag, SLOT( import_data_auto ( QMap < QString, QString > & )  ) );
   
   //ALEXEY: switch to Analysis
   connect( sdiag, SIGNAL( saving_complete_auto( QString &, QString & ) ), this, SLOT( to_analysis ( QString &, QString &) ) );
   //ALEXEY: for academic ver. switch back to experiment
   connect( sdiag, SIGNAL( saving_complete_back_to_exp( QString & ) ), this, SLOT( to_experiment (  QString & ) ) );
   
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   if ( mainw->us_mode_bool )
     sdiag->us_mode_passed();

   sdiag->show();

}

// void US_PostProdGui::import_data_us_convert( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii )
// {
//   emit to_post_prod( currDir, protocolName, invID_passed, correctRadii );
// }

void US_PostProdGui::import_data_us_convert(  QMap < QString, QString > & protocol_details )
{
  emit to_post_prod( protocol_details );
}

void US_PostProdGui::to_analysis( QString & currDir, QString & protocolName )
{
  emit switch_to_analysis( currDir, protocolName );
}

void US_PostProdGui::to_experiment( QString & protocolName )
{
  emit switch_to_exp( protocolName  );
} 

void US_PostProdGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}






// US_Analysis
US_AnalysisGui::US_AnalysisGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   QTextEdit* panel_desc = new QTextEdit(this);
   panel_desc->viewport()->setAutoFillBackground(false);
   panel_desc->setFrameStyle(QFrame::NoFrame);
   panel_desc->setPlainText(" Tab to Analyse Experimental Data... ---UNDER CONSTRUCTION--- ");
   panel_desc->setReadOnly(true);
   //panel_desc->setMaximumHeight(30);
   QFontMetrics m (panel_desc -> font()) ;
   int RowHeight = m.lineSpacing() ;
   panel_desc -> setFixedHeight  (2* RowHeight) ;

   int row = 0;
   genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();


   // //Later - do actual analysis form sdiag - whatever it will be:
   // connect( this, SIGNAL( start_analysis( QString &, QString & ) ), sdiag, SLOT( analyze_auto ( QString &, QString & )  ) );
   
}

void US_AnalysisGui::do_analysis( QString & currDir, QString & protocolName )
{
  emit start_analysis( currDir, protocolName );
}





// US_Report
US_ReportGui::US_ReportGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   QTextEdit* panel_desc = new QTextEdit(this);
   panel_desc->viewport()->setAutoFillBackground(false);
   panel_desc->setFrameStyle(QFrame::NoFrame);
   panel_desc->setPlainText(" Tab to Generate Report...  ---UNDER CONSTRUCTION--- ");
   panel_desc->setReadOnly(true);
   //panel_desc->setMaximumHeight(30);
   QFontMetrics m (panel_desc -> font()) ;
   int RowHeight = m.lineSpacing() ;
   panel_desc -> setFixedHeight  (2* RowHeight) ;

   int row = 0;
   genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

}

