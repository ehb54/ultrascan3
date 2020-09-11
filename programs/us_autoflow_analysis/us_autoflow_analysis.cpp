#include "us_autoflow_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>


const QColor colorRed       ( 210, 0, 0 );
const QColor colorDarkGreen ( 2, 88, 57 );
const QColor colorGreen     ( 50, 205, 50 );
const QColor colorYellow    ( 255, 255, 102 ); 

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
  // protocol_details[ "analysisIDs"  ] = QString( "252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287,288,289,290,291,292,293");
  
  
  // initPanel( protocol_details );

  // -----------------

}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_Analysis_auto::initPanel( QMap < QString, QString > & protocol_details )
{
  //Clear TreeWidget
  treeWidget->clear();
  Array_of_triples.clear();
  Array_of_analysis.clear();

  Completed_triples.clear();
  Failed_triples.clear();
  Manual_update.clear();

  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();

  analysisIDs        = protocol_details[ "analysisIDs" ];

  QStringList analysisIDs_list = analysisIDs.split(",");

  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  // Get the buffer data from the database
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }

  
  //get investigator e-mail:
  investigator_details.clear();
  investigator_details = get_investigator_info( &db, QString::number( invID ));

  //DB name
  QStringList DB = US_Settings::defaultDB();
  if ( DB.size() > 0 )
    {
      defaultDB = US_Settings::defaultDB().at( 2 );
      qDebug() << "defaultDB -- " << defaultDB;
    }

  //retrieve AutoflowAnalysis records, build autoflowAnalysis objects:
  for( int i=0; i < analysisIDs_list.size(); ++i )
    {
      QMap <QString, QString> analysis_details;
      QString requestID = analysisIDs_list[i];

      qDebug() << "RequestID: " << requestID;

      analysis_details = read_autoflowAnalysis_record( &db, requestID );

      qDebug() << "analysis_details.size() FROM autoflowAnalysis -- " << analysis_details.size();

      if ( !analysis_details.size() )
	analysis_details = read_autoflowAnalysisHistory_record( &db, requestID );

      qDebug() << "analysis_details.size() FROM autoflowAnalysisHistory -- " << analysis_details.size();

      QString triple_name = analysis_details["triple_name"];
      Array_of_analysis[ triple_name ] = analysis_details;

      Manual_update[ triple_name ]     = false;
      Completed_triples[ triple_name ] = false;
      Failed_triples[ triple_name ] = false;
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
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics fmet( sfont );

  int triple_name_width;
  int max_width = 0;

  qDebug() << "Font size: " <<  QString::number(sfont.pointSize());
  
  treeWidget->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize()) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");
  
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

      if ( json.contains("2DSA") )    //ALEXeY: maybe more strict condition here? (as 2DSA is in other stages...)
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
      //triple_name_width = fmet.horizontalAdvance( triple_curr );
      
      qDebug() << "Triple,  width:  " << triple_curr << ", " << triple_name_width;

      if ( triple_name_width > max_width )
       	max_width =  triple_name_width; 

      topItem [ triple_curr ] = new QTreeWidgetItem();
      topItem [ triple_curr ] -> setText( 0, triple_curr );
      topItem [ triple_curr ] -> setFont(0, sfont);
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

  treeWidget->header()->resizeSection(0, max_width );

  // treeWidget->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize()) + "pt;} QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");
  
  //TEST: QTimer for GroupBoxes' GUI update
  timer_end_process = new QTimer;
  timer_update      = new QTimer;
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  timer_update->start(5000);     // 5 sec

  //gui_update_temp();
 
}



//Gui update: timer's slot
void US_Analysis_auto::gui_update( )
{
  //qDebug() << "in_gui_update: " << in_gui_update;
    
  if ( in_gui_update )            // If already doing a reload,
    return;                            //  skip starting a new one
  
  in_gui_update  = true;          // Flag in the midst of a reload

  /**** JSON structure ****************************************************************/
  /*
  
    {
     "to_process":["2DSA_MC",".."],
     "processed" :[ 
                    {"2DSA"   :{"gfacID":"17","status":"COMPLETE","statusMsg":"Finished:  maxrss 441 MB,  total run seconds 48"}},
		    {"2DSA_FM":{"gfacID":"18","status":"COMPLETE","statusMsg":"Finished:  maxrss 462 MB,  total run seconds 32"}},
		    {"FITMEN" :{"gfacID":null,"status":"COMPLETE","statusMsg":"Waiting for manual stage FITMEN to complete."}}   
		  ],
     "submitted":"2DSA_IT"
     }
  
  */

  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  // Get the buffer data from the database
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }
  

      
  /*************** GUI UPDATE *************************************************************/ 
  QMap<QString, QMap <QString, QString> >::iterator jj;
  for ( jj = Array_of_analysis.begin(); jj != Array_of_analysis.end(); ++jj )
    {
      QString triple_current = jj.key();
      QString triple_curr_key = jj.key();
      QString triple_curr = triple_current.replace("."," / ");

      qDebug() << "triple_curr_key, triple_curr: " << triple_curr_key << ", " << triple_curr;
      
      QMap <QString, QString > ana_details = jj.value();

      QString requestID = ana_details["requestID"];
      
      QMap <QString, QString > current_analysis_details;
      current_analysis_details = read_autoflowAnalysis_record( &db, requestID );

      if ( !current_analysis_details.size() )
	{
	  current_analysis_details = read_autoflowAnalysisHistory_record( &db, requestID );
	  //set this triple as completed
	  Completed_triples[ triple_curr_key ] = true;
	  qDebug() << "FORM HISTORY: set Completed_triples[ " << triple_curr_key << " ] to " << Completed_triples[ triple_curr_key ];
	}

      
      QString cluster       = current_analysis_details[ "cluster" ]      ;
      QString filename      = current_analysis_details[ "filename" ]     ;
      QString curr_gfacID   = current_analysis_details[ "CurrentGfacID" ];
      QString curr_HPCAnalysisRequestID   = current_analysis_details[ "currentHPCARID"];
      QString status_json   = current_analysis_details[ "status_json" ]  ;
      QString status        = current_analysis_details[ "status" ]       ;
      QString status_msg    = current_analysis_details[ "status_msg" ]   ;
      QString create_time   = current_analysis_details[ "create_time" ]  ;   
      QString update_time   = current_analysis_details[ "update_time" ]  ;
            
      /********************* deal with JSON *****************************************/
      
      //qDebug() << "JSON: " << status_json;
      
      QJsonDocument jsonDoc = QJsonDocument::fromJson( status_json.toUtf8() );
      if (!jsonDoc.isObject())
	qDebug() << "NOT a JSON Doc !!";
      
      // const QJsonValue &to_process = jsonDoc["to_process"];
      // const QJsonValue &processed  = jsonDoc["processed"];
      // const QJsonValue &submitted  = jsonDoc["submitted"];

      const QJsonValue &to_process = jsonDoc.object().value("to_process");
      const QJsonValue &processed  = jsonDoc.object().value("processed");
      const QJsonValue &submitted  = jsonDoc.object().value("submitted");          

      QJsonArray to_process_array  = to_process.toArray();
      QJsonArray processed_array   = processed.toArray();
      
      //to_process
      if ( to_process.isUndefined())
	qDebug() << "All stages have been processed !!";
      else
	{
	  for (int i=0; i < to_process_array.size(); ++i )
	    {
	      qDebug() << "To process stage - " << to_process_array[i].toString();

	      QGroupBox * to_process_stage_groupbox = NULL;
	      QString stage_to_process = to_process_array[i].toString();

	      if ( stage_to_process == "FITMEN" )
		continue;
	      
	      
	      if ( stage_to_process == "2DSA" )
		to_process_stage_groupbox = groupbox_2DSA[ triple_curr ];
	      if ( stage_to_process == "2DSA_FM" )
		to_process_stage_groupbox = groupbox_2DSA_FM [ triple_curr ];
	      if ( stage_to_process == "2DSA_IT" )
		to_process_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
	      if ( stage_to_process == "2DSA_MC" )
		to_process_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];

	      if ( to_process_stage_groupbox != NULL ) 
		{
		  // update GUI
		  QLineEdit * lineedit_runid    = to_process_stage_groupbox->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_owner    = to_process_stage_groupbox->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_lastmsg  = to_process_stage_groupbox->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_status   = to_process_stage_groupbox->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_anatype  = to_process_stage_groupbox->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_submit   = to_process_stage_groupbox->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_cluster  = to_process_stage_groupbox->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_lastupd  = to_process_stage_groupbox->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
		  
		  //runID
		  QString runid_text = filename + " ( " + triple_curr + " )  " + defaultDB + " (ID: N/A)";
		  lineedit_runid -> setText(runid_text);
		  
		  //owner
		  QString investigator_text = investigator_details["email"] + " (" + investigator_details["fname"] + " " + investigator_details["lname"] +  ")";
		  lineedit_owner -> setText( investigator_text );
		  
		  //lastMsg
		  lineedit_lastmsg -> setText( "N/A" );
		  
		  //status
		  lineedit_status   ->setText( "Waiting for prior stage(s) to complete" );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb( 255, 255, 102 ); }");
		  
		  //analysis type
		  lineedit_anatype -> setText( stage_to_process );
		  
		  //submit
		  lineedit_submit -> setText( "N/A" );
		  
		  //updated
		  lineedit_lastupd -> setText( "N/A" );
		  
		  //cluster
		  lineedit_cluster -> setText( "N/A" );
		}
	    }
	}

      //processed
      if ( processed.isUndefined())
	qDebug() << "Nothing has been processed yet !!";
      else
	{
	  for (int i=0; i < processed_array.size(); ++i )
	    {
	      QGroupBox * processed_stage_groupbox = NULL;
	      QString stage_name, stage_gfacID, stage_status, stage_statusMsg,
		stage_createTime, stage_updateTime, stage_HPCAnalysisRequestID;
	      
	      foreach(const QString& key, processed_array[i].toObject().keys())
		{
		  QJsonObject newObj = processed_array[i].toObject().value(key).toObject();
		  
		  qDebug() << "Processed stage - " << key << ": gfacID, status, statusMsg -- "
			   << newObj["gfacID"]   .toString()
			   << newObj["HPCAnalysisRequestID"]. toString()
			   << newObj["status"]   .toString()
			   << newObj["statusMsg"].toString()
			   << newObj["createTime"].toString()
			   << newObj["updateTime"].toString();

		  stage_name   = key;
		  stage_gfacID = newObj["gfacID"]   .toString();
		  stage_HPCAnalysisRequestID = newObj["HPCAnalysisRequestID"]   .toString();
		  stage_status = newObj["status"]   .toString();
		  stage_statusMsg = newObj["statusMsg"].toString();
		  stage_createTime = newObj["createTime"].toString();
		  stage_updateTime = newObj["updateTime"].toString();
		}
	      
	      if ( stage_name == "FITMEN" ||  stage_name.isEmpty() )
		continue;
	      
	      if ( stage_name == "2DSA" )
		processed_stage_groupbox = groupbox_2DSA[ triple_curr ];
	      if ( stage_name == "2DSA_FM" )
		processed_stage_groupbox = groupbox_2DSA_FM [ triple_curr ];
	      if ( stage_name == "2DSA_IT" )
		processed_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
	      if ( stage_name == "2DSA_MC" )
		processed_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];

	      if ( processed_stage_groupbox != NULL )
		{
		  // update GUI
		  QLineEdit * lineedit_runid    = processed_stage_groupbox->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_owner    = processed_stage_groupbox->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_lastmsg  = processed_stage_groupbox->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_status   = processed_stage_groupbox->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_anatype  = processed_stage_groupbox->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_submit   = processed_stage_groupbox->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_cluster  = processed_stage_groupbox->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
		  QLineEdit * lineedit_lastupd  = processed_stage_groupbox->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
		  
		  //runID
		  QString runid_text = filename + " ( " + triple_curr + " )  " + defaultDB + " (ID: " + stage_HPCAnalysisRequestID + ")";
		  lineedit_runid -> setText(runid_text);
		  
		  //owner
		  QString investigator_text = investigator_details["email"] + " (" + investigator_details["fname"] + " " + investigator_details["lname"] +  ")";
		  lineedit_owner -> setText( investigator_text );
		  
		  //lastMsg
		  lineedit_lastmsg -> setText( stage_statusMsg );
		  
		  //status
		  lineedit_status -> setText( stage_status );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb( 2, 88, 57 ); color : white; }");
		  
		  //analysis type
		  lineedit_anatype -> setText( stage_name );
		  
		  //submit
		  lineedit_submit -> setText( stage_createTime );
		  
		  //updated
		  lineedit_lastupd -> setText( stage_updateTime );
		  
		  //cluster
		  lineedit_cluster -> setText( cluster );
	      
		}
	    }
	  //when re-attaching by reading history record for triple
	  if ( !to_process_array.size() )
	    topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorDarkGreen ) );
	}

      //submitted -- current active stage
     
      if ( submitted.isUndefined())
	qDebug() << "Nothing submitted yet !!";
      else
	{
	  QGroupBox * current_stage_groupbox = NULL;
	  qDebug() << "Submitted stage - " << submitted.toString() << ", for triple " << triple_curr;

	  if ( submitted.toString() == "FITMEN" && !Manual_update[ triple_curr_key ] )
	    {
	      QMessageBox::information( this,
					tr( "TEMPORARY: FITMEN stage reached" ),
					tr( "FITMET stage for triple %1 will be processed manually." ).arg( triple_curr ) );
	      update_autoflowAnalysis_status_at_fitmen( &db, requestID );

	      Manual_update[ triple_curr_key ] = true;
	      
	      //continue; 
	    }
	      
	  //QGroupBox * current_stage_groupbox;
	  
	  if ( submitted.toString() == "2DSA" )
	    current_stage_groupbox = groupbox_2DSA[ triple_curr ];
	  if ( submitted.toString() == "2DSA_FM" )
	    current_stage_groupbox = groupbox_2DSA_FM [ triple_curr ];
	  if ( submitted.toString() == "2DSA_IT" )
	    current_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
	  if ( submitted.toString() == "2DSA_MC" )
	    current_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];

	  if ( current_stage_groupbox != NULL )
	    {
	      
	      QLineEdit * lineedit_runid    = current_stage_groupbox->findChild<QLineEdit *>("runID", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_owner    = current_stage_groupbox->findChild<QLineEdit *>("owner", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_lastmsg  = current_stage_groupbox->findChild<QLineEdit *>("lastmsg", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_status   = current_stage_groupbox->findChild<QLineEdit *>("status", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_anatype  = current_stage_groupbox->findChild<QLineEdit *>("anatype", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_submit   = current_stage_groupbox->findChild<QLineEdit *>("submit", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_cluster  = current_stage_groupbox->findChild<QLineEdit *>("cluster", Qt::FindDirectChildrenOnly);
	      QLineEdit * lineedit_lastupd  = current_stage_groupbox->findChild<QLineEdit *>("lastupd", Qt::FindDirectChildrenOnly);
	      
	      //runID
	      QString runid_text = filename + " ( " + triple_curr + " )  " + defaultDB + " (ID: " + curr_HPCAnalysisRequestID + ")";
	      lineedit_runid -> setText(runid_text);
	      
	      //owner
	      QString investigator_text = investigator_details["email"] + " (" + investigator_details["fname"] + " " + investigator_details["lname"] +  ")";
	      lineedit_owner -> setText( investigator_text );
	      
	      //lastMsg
	      lineedit_lastmsg -> setText( status_msg );
	  
	      //status
	      lineedit_status -> setText( status );
	      lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(50, 205, 50); }");
	      
	      //analysis type
	      lineedit_anatype -> setText( submitted.toString() );
	      
	      //submit
	      lineedit_submit -> setText( create_time );
	      
	      //updated
	      lineedit_lastupd -> setText( update_time );
	      
	      //cluster
	      lineedit_cluster -> setText( cluster );


	      //check if final stage && it's complete 
	      if ( !to_process_array.size() && status == "COMPLETE" )
		{
		  lineedit_status          -> setStyleSheet( "QLineEdit { background-color:  rgb(2, 88, 57); color : white; }");
		  topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorDarkGreen ) );

		  /****
		  ALEXEY: maybe update QMap < triple_curr , bool > Completed_triples with TRUE (defined as all FALSES at the beginning):
		  Completed_triples[ triple_curr ] = true;
		  At the end of update cycle over triples, check if there is at least one FALSE value in the  Completed_triple QMap.
		  If NO - stop timer, show Msg that everything completed.
		  **/
		  Completed_triples[ triple_curr_key ] = true;
		  qDebug() << "set Completed_triples[ " << triple_curr_key << " ] to " << Completed_triples[ triple_curr_key ];
		  
		}

	      //check if failed -- due to the abortion/deletion of the job
	      if ( status == "FAILED" )
		{
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		  topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));
		  
		  Completed_triples[ triple_curr_key ] = true;
		  Failed_triples   [ triple_curr_key ] = true;
		  qDebug() << "FAILED status for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
		}
	    }
	}
    }

  // Check if all triples completed //
  bool all_processed = true;
  QMap<QString, bool>::iterator td;
  for ( td = Completed_triples.begin(); td != Completed_triples.end(); ++td )
    {
      qDebug() << "Key/Value of Completed_triples: " << td.key() << "/" << td.value();
      if ( !td.value() )
	{
	  all_processed = false;
	  break;
	}
    }

  qDebug() << "all_processed: " << all_processed;

  //check if there are failed triples
  bool failed_triples = false;
  QStringList Failed_triples_list;
  
  QMap<QString, bool>::iterator tdf;
  for ( tdf = Failed_triples.begin(); tdf != Failed_triples.end(); ++tdf )
    {
      qDebug() << "Key/Value of Failed_triples: " << tdf.key() << "/" << tdf.value();
      if ( tdf.value() )
	{
	  Failed_triples_list << tdf.key();
	  failed_triples = true;
	}
    }

  qDebug() << "Failed_triples exist ? Name(s): " << failed_triples << Failed_triples_list;
  
  //If all triples processed, show msg && Stop update timer
  if ( all_processed )
    {
      //stop timer
      if ( timer_update -> isActive() ) 
	{
	  timer_update -> stop();
	  disconnect(timer_update, SIGNAL(timeout()), 0, 0);
	  
	  qDebug() << "Update stopped upon final completion...";
	}
      
      in_gui_update  = false; 

      QString msg_text = "All triples have been processed.";
      
      if ( failed_triples )
	msg_text += QString("\n\n NOTE: analysises for the following triples failed: \n %1").arg( Failed_triples_list.join(", ") );
      
      QMessageBox::information( this,
				tr( "All Triples Processed !" ),
				msg_text  );

      //ALEXEY: Switch to next stage (Report) ?
    }

  
  
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

  Completed_triples.clear();
  Manual_update.clear();

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
QMap< QString, QString> US_Analysis_auto::read_autoflowAnalysis_record( US_DB2* db, const QString& requestID )
{
  QMap <QString, QString> analysis_details;
  
  // if ( db->lastErrno() != US_DB2::OK )
  //   {
  //     QMessageBox::warning( this, tr( "Connection Problem" ),
  // 			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
  //     return analysis_details;
  //   }

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
	  analysis_details[ "currentHPCARID" ] = db->value( 7 ).toString();
	  analysis_details[ "status_json" ]    = db->value( 8 ).toString();
	  analysis_details[ "status" ]         = db->value( 9 ).toString();
	  analysis_details[ "status_msg" ]     = db->value( 10 ).toString();
	  analysis_details[ "create_time" ]    = db->value( 11 ).toString();   
	  analysis_details[ "update_time" ]    = db->value( 12 ).toString();
	  analysis_details[ "create_userd" ]   = db->value( 13 ).toString();
	  analysis_details[ "update_user" ]    = db->value( 14 ).toString();
	}
    }

  //qDebug() << "In reading autoflwoAnalysis record: json: " << analysis_details[ "status_json" ] ;
  
  return analysis_details;
}

// Read AutoflowAnalysisHistory Record
QMap< QString, QString> US_Analysis_auto::read_autoflowAnalysisHistory_record( US_DB2* db, const QString& requestID )
{
  QMap <QString, QString> analysis_details;
  
  // if ( db->lastErrno() != US_DB2::OK )
  //   {
  //     QMessageBox::warning( this, tr( "Connection Problem" ),
  // 			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
  //     return analysis_details;
  //   }

  QStringList qry;
  qry << "read_autoflowAnalysisHistory_record"
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
	  analysis_details[ "currentHPCARID" ] = db->value( 7 ).toString();
	  analysis_details[ "status_json" ]    = db->value( 8 ).toString();
	  analysis_details[ "status" ]         = db->value( 9 ).toString();
	  analysis_details[ "status_msg" ]     = db->value( 10 ).toString();
	  analysis_details[ "create_time" ]    = db->value( 11 ).toString();   
	  analysis_details[ "update_time" ]    = db->value( 12 ).toString();
	  analysis_details[ "create_userd" ]   = db->value( 13 ).toString();
	  analysis_details[ "update_user" ]    = db->value( 14 ).toString();

	}
    }

  //qDebug() << "In reading autoflwoAnalysis record: json: " << analysis_details[ "status_json" ] ;
  
  return analysis_details;
}



QMap< QString, QString> US_Analysis_auto::get_investigator_info( US_DB2* db, const QString& ID )
{
  QMap<QString, QString> inv_details;
  
  QStringList qry;
  qry << "get_person_info"
      << ID;
  
  db->query( qry );

  if ( db->lastErrno() == US_DB2::OK )    
    {
      while ( db->next() )
	{
	  inv_details["fname"] = db->value( 0 ).toString();
	  inv_details["lname"] = db->value( 1 ).toString();
	  inv_details["email"] = db->value( 8 ).toString();
	}
    }

  return inv_details;
}

// set status to COMPLETE && msg text
void US_Analysis_auto::update_autoflowAnalysis_status_at_fitmen( US_DB2* db, const QString& requestID )
{
  QStringList qry;
  qry << "update_autoflow_analysis_record_at_fitmen"
      << requestID;
  
  db->query( qry );
  
  return;
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
