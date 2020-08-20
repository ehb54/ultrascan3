#include "us_autoflow_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"


// Constructor
US_Analysis_auto::US_Analysis_auto() : US_Widgets()
{
  setWindowTitle( tr( "Autoflow Analysis Monitor"));
                       
  //setPalette( US_GuiSettings::frameColor() );
  
  QVBoxLayout* panel  = new QVBoxLayout( this );
  panel->setSpacing        ( 2 );
  panel->setContentsMargins( 2, 2, 2, 2 );

  QLabel* lb_hdr1          = us_banner( tr( "Analsyis Stages for All Triples" ) );
  panel->addWidget(lb_hdr1);

  QHBoxLayout* buttons     = new QHBoxLayout();
  int row              = 1;
  pb_show_all   = us_pushbutton( tr( "Expand All Triples" ) );
  pb_hide_all   = us_pushbutton( tr( "Collapse All Triples" ) );
  buttons->addWidget( pb_show_all );
  buttons->addWidget( pb_hide_all );

  connect( pb_show_all,   SIGNAL( clicked()    ),
	   this,          SLOT  ( show_all() ) );
  
  connect( pb_hide_all,   SIGNAL( clicked()    ),
	   this,          SLOT  ( hide_all() ) );
  
  panel->addLayout(buttons);
  
  // TreeWidget
  treeWidget = new QTreeWidget();
  treeWidget->setColumnCount(2);
  treeWidget->headerItem()->setText(0, "Tripels");
  treeWidget->headerItem()->setText(1, "Analysis Stages");

  panel->addWidget(treeWidget);

  setMinimumSize( 950, 450 );
  adjustSize();

  in_gui_update = false;
  in_reload_end_process = false;
  
  // // ---- Testing ----
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "aprofileguid" ] = QString("d13ffad0-6f27-4fd8-8aa0-df8eef87a6ea");
  // protocol_details[ "protocolName" ] = QString("alexey-abs-itf-test1");
  // protocol_details[ "invID_passed" ] = QString("12");
  //protocol_details[ "analysisIDs"  ] = QString("4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65");
  
  
  //initPanel( protocol_details );

  // -----------------

}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_Analysis_auto::initPanel( QMap < QString, QString > & protocol_details )
{
  //Clear TreeWidget
  treeWidget->clear();
  Array_of_triples.clear();
  Array_of_analysis.clear();

  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();

  analysisIDs        = protocol_details[ "analysisIDs" ];

 
  QStringList analysisIDs_list = analysisIDs.split(",");

  //retrieve AutoflowAnalysis records, build autoflowAnalysis objects:
  for( int i=0; i < analysisIDs_list.size(); ++i )
    {
      QMap <QString, QString> analysis_details;
      QString requestID = analysisIDs_list[i];

      qDebug() << "RequestID: " << requestID;
      
      analysis_details = read_autoflowAnalysis_record( requestID );

      QString triple_name = analysis_details["triple_name"];
      Array_of_analysis[ triple_name ] = analysis_details;
    }
  
  // Close msg on setting up triple list from main program
  emit close_analysissetup_msg();
  

  //qDebug() << "ANALYSIS INIT: AProfileGUID, ProtocolName_auto, invID: " <<  AProfileGUID << ", " <<  ProtocolName_auto << ", " <<  invID;

  // job1run     = false;
  // job2run     = false;
  // job3run     = false;
  // job4run     = false;
  // job5run     = false;
  // job3auto    = false;

  //qDebug() << "Reading Aprofile data: ";
  //read_aprofile_data_from_aprofile();

  //qDebug() << "job1run, job2run, job3run, job4run, job5run: " << job1run << ", " <<  job2run << ", " <<  job3run << ", " << job4run << ", " <<  job5run ;

  // qDebug() << "Reading protocol's triple data: ";
  //read_protocol_data_triples();
   
  //Generate GUI
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
  QFontMetrics fmet( sfont );

  int triple_name_width;
  int max_width = 0;
  
  // for ( int i=0; i<Array_of_triples.size(); ++i )
  //   {

  QMap<QString, QMap <QString, QString> >::iterator jj;
  for ( jj = Array_of_analysis.begin(); jj != Array_of_analysis.end(); ++jj )
    {
      QString triple_curr = jj.key();
      triple_curr.replace("."," / ");
      
      QMap <QString, QString > ana_details = jj.value();

      job1run     = false;
      job2run     = false;
      job3run     = false;
      job4run     = false;
      job5run     = false;
      job3auto    = false;

      QString json = ana_details["status_json"];
      qDebug() << "triple: " << triple_curr << ", status_json: " << json;
      
      //TEST
      //json = "{\"to_process\":[\"FITMEN\",\"2DSA_IT\",\"2DSA_MC\"],\"processed\":[\"2DSA\"],\"submitted\":\"2DSA_FM\"}" ;

      if ( json.contains("2DSA") )
	job1run = true;
      if ( json.contains("2DSA_FM") )
	job2run = true;
      if ( json.contains("FITMEN") )
	job3run = true;
      if ( json.contains("2DSA_IT") )
	job4run = true;	  
      if ( json.contains("2DSA_MC") )
	job5run = true;
    
      
      
      triple_name_width = fmet.width( triple_curr );

      qDebug() << "Triple,  width:  " << triple_curr << ", " << triple_name_width;

      if ( triple_name_width > max_width )
       	max_width =  triple_name_width; 

      topItem [ triple_curr ] = new QTreeWidgetItem();
      topItem [ triple_curr ] -> setText( 0, triple_curr );
      treeWidget -> addTopLevelItem( topItem [ triple_curr ] );
      
      if ( job1run )  //2DSA
	{
	  QString stage_name( tr("2DSA") );
	  QString child_name = stage_name + " (" + triple_curr + ")";
	  groupbox_2DSA[ triple_curr ] = createGroup( child_name  );

	  childItem_2DSA [ triple_curr ] = new QTreeWidgetItem();
	  topItem [ triple_curr ] -> addChild( childItem_2DSA [ triple_curr ] );
	  treeWidget->setItemWidget( childItem_2DSA [ triple_curr ] , 1, groupbox_2DSA[ triple_curr ] );
	}
    
      
      if ( job2run )  //2DSA-FM
	{
	  QString stage_name( tr("2DSA-FM") );
	  QString child_name = stage_name + " (" + triple_curr + ")";
	  groupbox_2DSA_FM[ triple_curr ] = createGroup( child_name  );

	  childItem_2DSA_FM [ triple_curr ] = new QTreeWidgetItem();
	  topItem [ triple_curr ] -> addChild( childItem_2DSA_FM [ triple_curr ] );
	  treeWidget->setItemWidget( childItem_2DSA_FM [ triple_curr ] , 1, groupbox_2DSA_FM[ triple_curr ] );
	}

      if ( job3run )  //FITMEN
	{
	  
	}
      
      if ( job4run )  //2DSA-IT
	{
	  QString stage_name( tr("2DSA-IT") );
	  QString child_name = stage_name + " (" + triple_curr + ")";
	  groupbox_2DSA_IT[ triple_curr ] = createGroup( child_name  );

	  childItem_2DSA_IT [ triple_curr ] = new QTreeWidgetItem();
	  topItem [ triple_curr ] -> addChild( childItem_2DSA_IT [ triple_curr ] );
	  treeWidget->setItemWidget( childItem_2DSA_IT [ triple_curr ] , 1, groupbox_2DSA_IT [ triple_curr ] );
	}
      
      if ( job5run )  //2DSA-MC
	{
	  QString stage_name( tr("2DSA-MC") );
	  QString child_name = stage_name + " (" + triple_curr + ")";
	  groupbox_2DSA_MC[ triple_curr ] = createGroup( child_name  );

	  childItem_2DSA_MC [ triple_curr ] = new QTreeWidgetItem();
	  topItem [ triple_curr ] -> addChild( childItem_2DSA_MC [ triple_curr ] );
	  treeWidget->setItemWidget( childItem_2DSA_MC [ triple_curr ] , 1, groupbox_2DSA_MC [ triple_curr ] );
	}
    }
  
  qDebug() << "Triple Max Size: " << int(max_width);

  max_width *= 2;

  //treeWidget->setColumnWidth(0, 200);
  treeWidget->header()->resizeSection(0, max_width );

  // treeWidget->headerItem()->setText(0, "Tripels");
  // treeWidget->headerItem()->setText(1, "Analysis Stages");

  treeWidget->setStyleSheet( "QTreeWidget { font: bold; font-size: 15px;} QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //TEST: QTimer for GroupBoxes' GUI update
  timer_end_process = new QTimer;
  timer_update      = new QTimer;
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  //timer_update->start(1000);     // 5 sec

  gui_update_temp();
 
}

//Gui update temp: for paper
void US_Analysis_auto::gui_update_temp( )
{
  
  //TEST access to certain groupboxes' children... Mocup
  // for ( int i=0; i<Array_of_triples.size(); ++i )
  //   {
  //     QString triple_curr = Array_of_triples[i];

  QMap<QString, QMap <QString, QString> >::iterator jj;
  for ( jj = Array_of_analysis.begin(); jj != Array_of_analysis.end(); ++jj )
    {
      QString triple_curr = jj.key();
      triple_curr.replace("."," / ");
      
      QMap <QString, QString > ana_details = jj.value();
      
      // if( triple_curr.contains("Interference")) 
      // 	{
	  //2DSA
	  QLineEdit * lineedit_2dsa_runID    = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_owner    = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_lastmsg  = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_status   = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_anatype  = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_submit   = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_lastupd  = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_cluster  = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
	  
	  lineedit_2dsa_runID    ->setText( "demo1_interference (2/A/660) uslims3_CAUMA (ID: 8837)" );
	  lineedit_2dsa_owner    ->setText( "alexsav.science@gmail.com" );
	  lineedit_2dsa_lastmsg  ->setText( "Starting -- 2.0.1642 2020-05-10" );
	  lineedit_2dsa_status   ->setText( "Running" );
	  lineedit_2dsa_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(50, 205, 50); }");
	  lineedit_2dsa_anatype  ->setText( "2DSA" );
	  lineedit_2dsa_submit   ->setText( "2020-05-10 08:53:10" );
	  lineedit_2dsa_lastupd  ->setText( "2020-05-10 08:53:47" );
	  lineedit_2dsa_cluster  ->setText( "ls5.tacc.utexas.edu" );

	  
	  //2DSA-FM
	  QLineEdit * lineedit_2dsa_fm_runID    = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_fm_owner    = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_fm_lastmsg  = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_fm_status   = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_fm_anatype  = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_fm_submit   = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_fm_lastupd  = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_fm_cluster  = groupbox_2DSA_FM    [ triple_curr ]->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
	  
	  lineedit_2dsa_fm_runID    ->setText( "demo1_interference (2/A/660) uslims3_CAUMA (ID: N/A)" );
	  lineedit_2dsa_fm_owner    ->setText( "alexsav.science@gmail.com" );
	  lineedit_2dsa_fm_lastmsg  ->setText( "N/A" );
	  lineedit_2dsa_fm_status   ->setText( "Waiting for 2DSA stage to complete" );
	  lineedit_2dsa_fm_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(255, 255, 102); }");
	  lineedit_2dsa_fm_anatype  ->setText( "2DSA-FM" );
	  lineedit_2dsa_fm_submit   ->setText( "N/A" );
	  lineedit_2dsa_fm_lastupd  ->setText( "N/A" );
	  lineedit_2dsa_fm_cluster  ->setText( "ls5.tacc.utexas.edu" );

	  //2DSA-IT
	  QLineEdit * lineedit_2dsa_it_runID    = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_it_owner    = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_it_lastmsg  = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_it_status   = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_it_anatype  = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_it_submit   = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_it_lastupd  = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_it_cluster  = groupbox_2DSA_IT    [ triple_curr ]->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
	  
	  lineedit_2dsa_it_runID    ->setText( "demo1_interference (2/A/660) uslims3_CAUMA (ID: N/A)" );
	  lineedit_2dsa_it_owner    ->setText( "alexsav.science@gmail.com" );
	  lineedit_2dsa_it_lastmsg  ->setText( "N/A" );
	  lineedit_2dsa_it_status   ->setText( "Waiting for 2DSA-FM stage to complete" );
	  lineedit_2dsa_it_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(255, 255, 102); }");
	  lineedit_2dsa_it_anatype  ->setText( "2DSA-IT" );
	  lineedit_2dsa_it_submit   ->setText( "N/A" );
	  lineedit_2dsa_it_lastupd  ->setText( "N/A" );
	  lineedit_2dsa_it_cluster  ->setText( "ls5.tacc.utexas.edu" );

	  //2DSA-MC
	  QLineEdit * lineedit_2dsa_mc_runID    = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_mc_owner    = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_mc_lastmsg  = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_mc_status   = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_mc_anatype  = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_mc_submit   = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_mc_lastupd  = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
	  QLineEdit * lineedit_2dsa_mc_cluster  = groupbox_2DSA_MC    [ triple_curr ]->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
	  
	  lineedit_2dsa_mc_runID    ->setText( "demo1_interference (2/A/660) uslims3_CAUMA (ID: N/A)" );
	  lineedit_2dsa_mc_owner    ->setText( "alexsav.science@gmail.com" );
	  lineedit_2dsa_mc_lastmsg  ->setText( "N/A" );
	  lineedit_2dsa_mc_status   ->setText( "Waiting for 2DSA-IT stage to complete" );
	  lineedit_2dsa_mc_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(255, 255, 102); }");
	  lineedit_2dsa_mc_anatype  ->setText( "2DSA-MC" );
	  lineedit_2dsa_mc_submit   ->setText( "N/A" );
	  lineedit_2dsa_mc_lastupd  ->setText( "N/A" );
	  lineedit_2dsa_mc_cluster  ->setText( "ls5.tacc.utexas.edu" );

	  //}
    }
}

//Gui update: timer's slot
void US_Analysis_auto::gui_update( )
{
  if ( in_gui_update )            // If already doing a reload,
    return;                            //  skip starting a new one
  
  in_gui_update  = true;          // Flag in the midst of a reload

  /*************** DEBUG *************************************************************/ 
  QStringList fields;
  fields << "runID" << "owner" << "lastmsg" << "anatype" << "submit" << "cluster" << "lastupd";
  int curr_index = rand() % fields.size(); // pick a random index
  QString field_name = fields[ curr_index ]; // 
  
  //TEST access to certain groupboxes' children... Mock-up
  // for ( int i=0; i<Array_of_triples.size(); ++i )
  //   {
  //     QString triple_curr = Array_of_triples[i];
  
  QMap<QString, QMap <QString, QString> >::iterator jj;
  for ( jj = Array_of_analysis.begin(); jj != Array_of_analysis.end(); ++jj )
    {
      QString triple_curr = jj.key();
      triple_curr.replace("."," / ");
      
      QMap <QString, QString > ana_details = jj.value();
 
      
      //if( triple_curr.contains("Interference")) 
      //{
   	  QLineEdit * lineedit_2dsa    = groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit *>(field_name, Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_fm = groupbox_2DSA_FM [ triple_curr ]->findChild<QLineEdit *>(field_name, Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_it = groupbox_2DSA_IT [ triple_curr ]->findChild<QLineEdit *>(field_name, Qt::FindDirectChildrenOnly);
   	  QLineEdit * lineedit_2dsa_mc = groupbox_2DSA_MC [ triple_curr ]->findChild<QLineEdit *>(field_name, Qt::FindDirectChildrenOnly);
	  
   	  lineedit_2dsa    ->setText( "Some Message" );
   	  lineedit_2dsa_fm ->setText( "Some Message" );
   	  lineedit_2dsa_it ->setText( "Some Message" );
   	  lineedit_2dsa_mc ->setText( "Some Message" );

   	  lineedit_2dsa    ->setStyleSheet("QLineEdit { color: red;}");
   	  lineedit_2dsa_fm ->setStyleSheet("QLineEdit { color: red;}");
   	  lineedit_2dsa_it ->setStyleSheet("QLineEdit { color: red;}");
   	  lineedit_2dsa_mc ->setStyleSheet("QLineEdit { color: red;}");
	  
   	  for ( int i=0; i < fields.size(); ++i )
   	    {
   	      if ( i != curr_index )
   		{
		  groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setText( "" );
   		  groupbox_2DSA_FM [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setText( "" );
   		  groupbox_2DSA_IT [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setText( "" );
   		  groupbox_2DSA_MC [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setText( "" );
		  
   		  groupbox_2DSA    [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setStyleSheet("QLineEdit { color: black;}");
   		  groupbox_2DSA_FM [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setStyleSheet("QLineEdit { color: black;}");
   		  groupbox_2DSA_IT [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setStyleSheet("QLineEdit { color: black;}");
   		  groupbox_2DSA_MC [ triple_curr ]->findChild<QLineEdit*>(fields[i], Qt::FindDirectChildrenOnly)->setStyleSheet("QLineEdit { color: black;}");
		  
   		}
   	    }
   	}
  //}
  
  in_gui_update  = false; 
}

//reset Analysis GUI: stopping all update processes
void US_Analysis_auto::reset_analysis_panel_public( )
{
  reset_analysis_panel( );
}

//reset Analysis GUI: stopping all update processes
void US_Analysis_auto::reset_analysis_panel( )
{
  //Stop  timer if active
  if ( timer_update -> isActive() ) 
    {
      timer_update -> stop();
      disconnect(timer_update, SIGNAL(timeout()), 0, 0);

      qDebug() << "Stopping timer_update !!!!";
    }

  //ALEXEY: now we should wait for completion of the last timer_update shot...
  connect(timer_end_process, SIGNAL(timeout()), this, SLOT( end_process ( ) ));
  timer_end_process->start(1000);     // 5 sec
}

//Periodically check for ended processes
void US_Analysis_auto::end_process( )
{
  qDebug() << "In the END process: in_reload_end_processes = " << in_reload_end_process;
  
  if ( in_reload_end_process )               // If already doing a reload,
    return;                                  //  skip starting a new one
  
  in_reload_end_process   = true;          // Flag in the midst of a reload

  if (  !in_gui_update )
    {
      
      timer_end_process->stop();
      disconnect(timer_end_process, SIGNAL( timeout() ), 0, 0);   //Disconnect timer from anything
      
      qDebug() << "ANALYSIS UPDATE panel has been reset!";
      qDebug() << "AFTER: in_gui_update: " << in_gui_update;
      
      reset_auto(); 
      qApp->processEvents();
      
      in_reload_end_process = false;

      emit analysis_update_process_stopped();
    }
  else
    {
      in_reload_end_process   = false; 
      qApp->processEvents();
    }
}

//reset
void US_Analysis_auto::reset_auto( )
{
  treeWidget->clear();
  Array_of_triples.clear();

  Array_of_analysis.clear();

  //TO DO MORE later - DB stopp etc..
}

//create groupBox
QGroupBox * US_Analysis_auto::createGroup( QString & triple_name )
{
  QGroupBox *groupBox = new QGroupBox ( triple_name );

  QPalette p = groupBox->palette();
  p.setColor(QPalette::Dark, Qt::white);
  groupBox->setPalette(p);

  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");

  
  groupBox->setFlat(true);

  //GUI
  QGridLayout* genL   = new QGridLayout();
  genL->setSpacing        ( 2 );
  genL->setContentsMargins( 20, 10, 20, 15 );
  
  int row = 0;

  //RunID
  QLabel*     lb_runID   = us_label( tr( "RunID:" ) );
  QLineEdit*  le_runID   = us_lineedit( "", 0, true );
  le_runID -> setObjectName("runID");

  //Owner
  QLabel*     lb_owner   = us_label( tr( "Owner:" ) );
  QLineEdit*  le_owner   = us_lineedit( "", 0, true );
  le_owner -> setObjectName("owner");

  //Last Msg
  QLabel*     lb_lastmsg = us_label( tr( "Last Message:" ) );
  QLineEdit*  le_lastmsg = us_lineedit( "", 0, true );
  le_lastmsg -> setObjectName("lastmsg");

  //Status
  QLabel*     lb_status  = us_label( tr( "Status:" ) );
  QLineEdit*  le_status  = us_lineedit( "", 0, true );
  le_status -> setObjectName("status");

  //Analysis Type
  QLabel*     lb_anatype = us_label( tr( "Analysis Type:" ) );
  QLineEdit*  le_anatype = us_lineedit( "", 0, true );
  le_anatype -> setObjectName("anatype");

  //Submitted On
  QLabel*     lb_submit  = us_label( tr( "Submitted on:" ) );
  QLineEdit*  le_submit  = us_lineedit( "", 0, true );
  le_submit -> setObjectName("submit");

  //Running On
  QLabel*     lb_cluster  = us_label( tr( "Running on:" ) );
  QLineEdit*  le_cluster  = us_lineedit( "", 0, true  );
  le_cluster -> setObjectName("cluster");
  le_cluster->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  
  //Last Status Update
  QLabel*     lb_lastupd  = us_label( tr( "Last Status Update:" ) );
  QLineEdit*  le_lastupd  = us_lineedit( "", 0, true );
  le_lastupd -> setObjectName("lastupd");
  
  
  genL->addWidget( lb_runID,   row,    0, 1, 2 );
  genL->addWidget( le_runID,   row++,  2, 1, 8 );

  genL->addWidget( lb_owner,   row,    0, 1, 2 );
  genL->addWidget( le_owner,   row++,  2, 1, 8 );  

  genL->addWidget( lb_lastmsg, row,    0, 1, 2 );
  genL->addWidget( le_lastmsg, row++,  2, 1, 8 );

  genL->addWidget( lb_status,  row,    0, 1, 2 );
  genL->addWidget( le_status,  row,    2, 1, 3 );
  genL->addWidget( lb_anatype, row,    5, 1, 2 );
  genL->addWidget( le_anatype, row++,  7, 1, 3 );

  genL->addWidget( lb_submit,  row,    0, 1, 2 );
  genL->addWidget( le_submit,  row,    2, 1, 3 );
  genL->addWidget( lb_cluster, row,    5, 2, 2 );
  genL->addWidget( le_cluster, row++,  7, 2, 3 );

  genL->addWidget( lb_lastupd, row,    0, 1, 2 );
  genL->addWidget( le_lastupd, row,    2, 1, 3 );
  
  groupBox->setLayout(genL);
  
  
  return groupBox;
}

//read from protocol all triples: if Interference - just one wvl (660); if UV/vis - read all wvl/channel pairs and store
void US_Analysis_auto::read_protocol_data_triples()
{
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }

  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  QXmlStreamReader xmli( xmlstr );

  while( ! xmli.atEnd() )
    {
      xmli.readNext();
      
      if ( xmli.isStartElement() )
	{
	  QString ename   = xmli.name().toString();

	  if ( ename == "optics" )
	    read_protoOptics( xmli );
	  else if ( ename == "ranges" )
	    read_protoRanges( xmli );
	}
    }
}

//read protoOptics
bool US_Analysis_auto::read_protoOptics( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
         if ( ename == "optical_system" )
         {
	   QXmlStreamAttributes attr = xmli.attributes();
            QString channel   = attr.value( "channel" ).toString();
            QString scan1     = attr.value( "scan1"   ).toString();
            QString scan2     = attr.value( "scan2"   ).toString();
            QString scan3     = attr.value( "scan3"   ).toString();

	    QStringList channel_parts = channel.split( "," );
	    QString channel_name = channel_parts[0];
	    //channel_name.replace( " ", "" );

	    if ( !scan2.isEmpty() && channel.contains("sample") )  // Interference
	      {
		QString channel_append(" / Interference");
		channel_name += channel_append;

		//Collect only non-UV/vis channels (Interference or fluorecsence)
		Array_of_triples.push_back( channel_name );
	      }
	 }
      }

      bool was_end    = xmli.isEndElement();   // Just read was End of element?
      xmli.readNext();                         // Read the next element

      if ( was_end  &&  ename == "optics" )    // Break after "</optics>"
         break;
   }

   return ( ! xmli.hasError() );
}

//read protoRanges
bool US_Analysis_auto::read_protoRanges( QXmlStreamReader& xmli )
{
  QString channel_name;
  
  while( ! xmli.atEnd() )
   {
      QString ename   = xmli.name().toString();
        
      if ( xmli.isStartElement() )
      {
         if ( ename == "range"  ||  ename == "spectrum" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString channel           = attr.value( "channel" )     .toString();

	    QStringList channel_parts = channel.split( "," );
	    channel_name = channel_parts[0];
          }

         else if ( ename == "wavelength"  ||  ename == "point" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            QString wvl = attr.value( "lambda" ).toString();

	    QString channel_name_wvl = channel_name;

	    channel_name_wvl.append(QString(" / ") + wvl);

	    Array_of_triples.push_back( channel_name_wvl );
         }
      }

      // else if ( xmli.isEndElement()  &&
      //          ( ename == "range"  ||  ename == "spectrum" ) )
      // {
      //    chrngs << rng;
      //    nranges++;
      // }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end )
      {
         if ( ename == "ranges"  ||          // Break after "</ranges>"
              ename == "spectra" )           // Break after "</spectra>"
           break;
      }
   }

   return ( ! xmli.hasError() );
}

//read related AProfile data (stages)
void US_Analysis_auto::read_aprofile_data_from_aprofile()
{
  QString aprofile_xml;
  
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }

  // qDebug() << "AProfGUID: " << AProfileGUID;

    
  QStringList qry;
  qry << "get_aprofile_info" << AProfileGUID;
  db.query( qry );
  
  while ( db.next() )
    {
      //currProf.aprofID     = db.value( 0 ).toInt();
      //currProf.aprofname   = db.value( 1 ).toString();
      aprofile_xml         = db.value( 2 ).toString();
    }

  //qDebug() << "aprofile_xml: " <<  aprofile_xml;
  
  if ( !aprofile_xml.isEmpty() )
    {
      QXmlStreamReader xmli( aprofile_xml );
      readAProfile( xmli );
    }
}

//Aprofile stages
bool US_Analysis_auto::readAProfile( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      xmli.readNext();
      
      if ( xmli.isStartElement() )
	{
	  QString ename   = xmli.name().toString();
	  
	  if ( ename == "p_2dsa" )
	    {
	      readAProfile_2DSA( xmli );
	    }

	}
    }

  return ( ! xmli.hasError() );
}
	  
// Read all current 2DSA controls from an XML stream
bool US_Analysis_auto::readAProfile_2DSA( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
	{
	 // ALEXEY: read channels... & # of triples to generate corresp. # of groupBoxes
	 if ( ename == "channel_parms" )
         {
	    QXmlStreamAttributes attr = xmli.attributes();
	    QString chan   = attr.value( "channel" ).toString();
	    
	 }
	 
         if ( ename == "job_2dsa" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job1run        = bool_flag( attr.value( "run" ).toString() );
            //job1nois       = attr.value( "noise" ).toString();
         }
         else if ( ename == "job_2dsa_fm" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job2run        = bool_flag( attr.value( "run" ).toString() );
            //job2nois       = attr.value( "noise" ).toString();
            //fitrng         = attr.value( "fit_range" ).toString().toDouble();
            //grpoints       = attr.value( "grid_points" ).toString().toInt();
         }
         else if ( ename == "job_fitmen" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job3run        = bool_flag( attr.value( "run" ).toString() );
            job3auto       = attr.value( "interactive" ).toString().toInt() == 0;
         }
         else if ( ename == "job_2dsa_it" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job4run        = bool_flag( attr.value( "run" ).toString() );
            job4nois       = attr.value( "noise" ).toString();
            //rfiters        = attr.value( "max_iterations" ).toString().toInt();
         }
         else if ( ename == "job_2dsa_mc" )
         {
            QXmlStreamAttributes attr = xmli.attributes();
            job5run        = bool_flag( attr.value( "run" ).toString() );
            //mciters        = attr.value( "mc_iterations" ).toString().toInt();
         }
      }

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "p_2dsa" )   // Break after "</p_2dsa>"
         break;
   }

   return ( ! xmli.hasError() );	      
}


// Return a flag if an XML attribute string represents true or false.
bool US_Analysis_auto::bool_flag( const QString xmlattr )
{
   return ( !xmlattr.isEmpty()  &&  ( xmlattr == "1"  ||  xmlattr == "T" ) );
}

//Unfold tree
void US_Analysis_auto::show_all( )
{
  treeWidget->expandAll();
}

//collapse tree
void US_Analysis_auto::hide_all( )
{
  treeWidget->collapseAll();
}


// Read AutoflowAnalysisRecord
QMap< QString, QString> US_Analysis_auto::read_autoflowAnalysis_record( QString requestID )
{
  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  QMap <QString, QString> analysis_details;
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return analysis_details;
    }

  QStringList qry;
  qry << "read_autoflowAnalysis_record"
      << requestID;
  
  db->query( qry );

  if ( db->lastErrno() == US_DB2::OK )    
    {
      while ( db->next() )
	{
	  analysis_details[ "requestID" ]      = db->value( 0 ).toString();
	  analysis_details[ "triple_name" ]    = db->value( 1 ).toString();
	  analysis_details[ "cluster" ]        = db->value( 2 ).toString();
	  analysis_details[ "filename" ]       = db->value( 3 ).toString();
	  analysis_details[ "aprofileGUID" ]   = db->value( 4 ).toString();
	  analysis_details[ "invID" ]          = db->value( 5 ).toString();
	  analysis_details[ "CurrentGfacID" ]  = db->value( 6 ).toString();
	  analysis_details[ "status_json" ]    = db->value( 7 ).toString();
	  analysis_details[ "status" ]         = db->value( 8 ).toString();
	  analysis_details[ "status_msg" ]     = db->value( 9 ).toString();
	  analysis_details[ "create_time" ]    = db->value( 10 ).toString();   
	  analysis_details[ "update_time" ]    = db->value( 11 ).toString();
	  analysis_details[ "create_userd" ]   = db->value( 12 ).toString();
	  analysis_details[ "update_user" ]    = db->value( 13 ).toString();

	}
    }

  //qDebug() << "In reading autoflwoAnalysis record: json: " << analysis_details[ "status_json" ] ;
  
  return analysis_details;
}
