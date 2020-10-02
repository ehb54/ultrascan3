#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include "us_autoflow_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"

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
  treeWidget->headerItem()->setText(0, "Triples");
  treeWidget->headerItem()->setText(1, "Analysis Stages");

  panel->addWidget(treeWidget);

  setMinimumSize( 950, 450 );
  adjustSize();

  in_gui_update = false;
  in_reload_end_process = false;
  
  // // ---- Testing ----
  // QMap < QString, QString > protocol_details;
  // protocol_details[ "invID_passed" ] = QString("12");
  // protocol_details[ "analysisIDs"  ] = QString( "61,62,63,64,65,66" );
  
  // initPanel( protocol_details );

  // // -----------------

}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_Analysis_auto::initPanel( QMap < QString, QString > & protocol_details )
{
  //Clear TreeWidget
  treeWidget->clear();
  Array_of_triples.clear();
  Array_of_analysis.clear();
  Array_of_analysis_by_requestID.clear();
  Channel_to_requestIDs.clear();
  channels_all.clear();

  Completed_triples.clear();
  Failed_triples.clear();
  Canceled_triples.clear();
  Manual_update.clear();
  History_read.clear();
  Process_2dsafm.clear();
  
  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();

  FileName           = protocol_details[ "filename" ];
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

      if ( !analysis_details.size() )
	{
	  analysis_details = read_autoflowAnalysisHistory_record( &db, requestID );
	  qDebug() << "analysis_details.size() FROM autoflowAnalysisHistory -- " << analysis_details.size();
	}
      else
	qDebug() << "analysis_details.size() FROM autoflowAnalysis -- " << analysis_details.size();
      
      QString triple_name = analysis_details["triple_name"];
      QString status_json = analysis_details["status_json"];
      
      Array_of_analysis[ triple_name ]  = analysis_details;
      Array_of_analysis_by_requestID[ requestID ] = analysis_details;

      Manual_update[ triple_name ]     = false;
      Completed_triples[ triple_name ] = false;
      Failed_triples[ triple_name ]    = false;
      Canceled_triples[ triple_name ]  = false;
      History_read[ triple_name ]      = false;

      if ( status_json.contains("2DSA_FM") ) 
	Process_2dsafm[ triple_name ] = true;
      else
	Process_2dsafm[ triple_name ] = false;
	
      //Define triple's channels
      QStringList triple_name_parts = triple_name.split(".");
      channels_all << triple_name_parts[0] + "." + triple_name_parts[1];
    }
  
  //Group requestIDs by channel, exclude "Intereference" runs
  channels_all.removeDuplicates();
  QMap<QString, QMap <QString, QString> >::iterator ti;
  
  for ( int i = 0; i < channels_all.size(); ++i  )
    {
      for ( ti = Array_of_analysis.begin(); ti != Array_of_analysis.end(); ++ti )
	{
	  QString triple_name = ti.key();
	  if ( triple_name.contains( channels_all[i] ) && !triple_name.contains("Interference") )
	    {
	      QMap <QString, QString > ana_details = ti.value();
	      Channel_to_requestIDs[ channels_all[i] ] << ana_details[ "requestID" ];
	    }
	}
    }
  
  // Close msg on setting up triple list from main program
  emit close_analysissetup_msg();

  /* DEBUG: check requestIDs per channel *****************/
  QMap<QString, QStringList >::iterator chanreq;
  for ( chanreq = Channel_to_requestIDs.begin(); chanreq != Channel_to_requestIDs.end(); ++chanreq )
    qDebug() << "channel-to-requestIDs " << chanreq.key() << ": " << chanreq.value();

  // return;
  // /*** END OF DEBUG *************************************/
   
  //Generate GUI
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics fmet( sfont );

  int triple_name_width;
  int max_width = 0;

  qDebug() << "Font size: " <<  QString::number(sfont.pointSize());
  
  treeWidget->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize()) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");
  
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
	  //set this triple as completed && history-read
	  Completed_triples[ triple_curr_key ] = true;
	  History_read[ triple_curr_key ]      = true;
	  qDebug() << "FORM HISTORY: set Completed_triples[ " << triple_curr_key << " ] to " << Completed_triples[ triple_curr_key ];
	}

      
      QString cluster        = current_analysis_details[ "cluster" ]      ;
      QString filename       = current_analysis_details[ "filename" ]     ;
      QString curr_gfacID    = current_analysis_details[ "CurrentGfacID" ];
      QString curr_HPCAnalysisRequestID   = current_analysis_details[ "currentHPCARID"];
      QString status_json    = current_analysis_details[ "status_json" ]  ;
      QString status         = current_analysis_details[ "status" ]       ;
      QString status_msg     = current_analysis_details[ "status_msg" ]   ;
      QString create_time    = current_analysis_details[ "create_time" ]  ;   
      QString update_time    = current_analysis_details[ "update_time" ]  ;
      QString nextWaitStatus = current_analysis_details[ "nextWaitStatus" ] ;
      QString nextWaitStatusMsg = current_analysis_details[ "nextWaitStatusMsg" ] ;
            
      /********************* deal with JSON *****************************************/
      
      //qDebug() << "JSON: " << status_json;
      
      QJsonDocument jsonDoc = QJsonDocument::fromJson( status_json.toUtf8() );
      if (!jsonDoc.isObject())
	{
	  qDebug() << "NOT a JSON Doc !!";
	  return;
	}
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

		  QPushButton * pb_delete       = to_process_stage_groupbox->findChild<QPushButton *>("delete", Qt::FindDirectChildrenOnly);

		  //delete button
		  pb_delete -> setEnabled( false );
		  
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

	      //IF Canceled by user:
	      if ( stage_name == "FITMEN" && ( stage_status == "CANCELED" || stage_status == "canceled" ) )
		{
		  processed_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
		  qDebug() << "CANCELED status for triple/stage (FITMEN) -- " << triple_curr_key << "/" << submitted.toString();

		  stage_HPCAnalysisRequestID = "N/A";
		  stage_name = "2DSA-IT";
		  stage_createTime = "N/A";
		  stage_updateTime = "N/A";
		  stage_status = "CANCELED";
		  
		}
	      
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

		  QPushButton * pb_delete       = processed_stage_groupbox->findChild<QPushButton *>("delete", Qt::FindDirectChildrenOnly);

		  //delete
		  pb_delete->setEnabled( false );

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
	      
		
		  //IF Canceled by user:
		  if ( stage_status == "CANCELED" || stage_status == "canceled" )
		    {
		      lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		      topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));
		      
		      pb_delete->setEnabled( false );
		      
		      Completed_triples[ triple_curr_key ] = true;
		      Canceled_triples [ triple_curr_key ] = true;
		      qDebug() << "CANCELED status for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
		    }
		}
	    }
	      
	  //when re-attaching by reading history record for triple
	  if ( !to_process_array.size() && History_read[ triple_curr_key ] &&
	       !status.contains("CANCELED") && !status.contains("canceled") &&
	       !status.contains("FAILED") && !status.contains("failed") )
	    {
	      qDebug() << "Setting triple to complete - read from History, status = " << status;
	      topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorGreen ) );
	    }
	}
      
      //submitted -- current active stage
      if ( submitted.isUndefined())
	qDebug() << "Nothing submitted yet !!";
      else
	{
	  QGroupBox * current_stage_groupbox = NULL;
	  qDebug() << "Submitted stage - " << submitted.toString() << ", for triple " << triple_curr;
	  qDebug() << "Process_2dsafm[] = " <<  Process_2dsafm[ triple_curr_key ];

	  QString stage_name = submitted.toString();
	    
	  if ( submitted.toString() == "FITMEN" && !Manual_update[ triple_curr_key ] && Process_2dsafm[ triple_curr_key ] )
	    {
	      QMessageBox::information( this,
					tr( "ATTENTION: FITMEN stage reached" ),
					tr( "FITMET stage for triple %1 will be processed manually." ).arg( triple_curr ) );


	      /** To FitMeniscus class -- pass:
		  1. Name of the triple to title && and possibly to pass to scan_db() method
		  2. Filename, triple_name
	      ****/
	      QMap< QString, QString > triple_info_map;
	      triple_info_map[ "triple_name" ]     = triple_curr;
	      triple_info_map[ "triple_name_key" ] = triple_curr_key;
	      triple_info_map[ "requestID" ]       = requestID;
	      triple_info_map[ "invID" ]           = QString::number(invID);
	      triple_info_map[ "filename" ]        = FileName;
	      
	      FitMen = new US_FitMeniscus( triple_info_map );
	      	      
	      /** The following will block parent windows from closing BUT not from continuing timer execution ***/
	      FitMen->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
	      FitMen->setWindowModality(Qt::ApplicationModal);
	      /***************************************************************************************************/
	      connect( FitMen, SIGNAL( editProfiles_updated(  QMap < QString, QString > & ) ),
		       this, SLOT( update_autoflowAnalysis_statuses (  QMap < QString, QString > &) ) );
	      
	      FitMen->show();

	      Manual_update[ triple_curr_key ] = true;

	      //Stop timer here, and restart with the editProfile_updated signal ??
	      timer_update -> stop();
	      disconnect(timer_update, SIGNAL(timeout()), 0, 0);
	      
	      qDebug() << "Update stopped at FITMEN for triple -- " << triple_curr_key;
	      
	      in_gui_update  = false; 

	      return;
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

	  //Special case: children triple of the channel, while parent triple (selected wvl) is CANCELED
	  if ( submitted.toString() == "FITMEN" && ( nextWaitStatus == "CANCELED" || nextWaitStatus == "canceled") )
	    {
	       current_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];

	       curr_HPCAnalysisRequestID = "N/A";
	       status_msg = "Job has been scheduled for deletion";
	       status = "CANCELED";
	       stage_name = "2DSA-IT";
	       create_time = "N/A";
	       update_time = "N/A";
	    }

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

	      QPushButton * pb_delete       = current_stage_groupbox->findChild<QPushButton *>("delete", Qt::FindDirectChildrenOnly);

	      //QString triple_stage = triple_curr_key + ":" + submitted.toString();
	      //delete button: if running && admin_user || user_owner, enable; otherwise, disable (should be disabled earlier)
	      if ( US_Settings::us_inv_level() > 3 || US_Settings::us_inv_ID() == invID )
		if ( !pb_delete->isEnabled() )
		  {
		    pb_delete->setEnabled( true );
		    
		    //signalMapper->setMapping ( pb_delete, triple_stage );
		  }
	      	          
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
	      lineedit_anatype -> setText( stage_name );
	      
	      //submit
	      lineedit_submit -> setText( create_time );
	      
	      //updated
	      lineedit_lastupd -> setText( update_time );
	      
	      //cluster
	      lineedit_cluster -> setText( cluster );


	      //if complete 
	      if ( status == "COMPLETE" )
		{
		  pb_delete->setEnabled( false );
		  lineedit_status          -> setStyleSheet( "QLineEdit { background-color:  rgb(2, 88, 57); color : white; }");
		  		  
		  // && if final stage
		  if ( !to_process_array.size() )
		    {
		      topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorGreen ) );
		      /****
			   ALEXEY: maybe update QMap < triple_curr , bool > Completed_triples with TRUE (defined as all FALSES at the beginning):
			   Completed_triples[ triple_curr ] = true;
			   At the end of update cycle over triples, check if there is at least one FALSE value in the  Completed_triple QMap.
			   If NO - stop timer, show Msg that everything completed.
		      **/
		      Completed_triples[ triple_curr_key ] = true;
		      qDebug() << "set Completed_triples[ " << triple_curr_key << " ] to " << Completed_triples[ triple_curr_key ];
		      
		    }
		}

	      //check if failed -- due to the abortion/deletion of the job
	      if ( status == "FAILED" || status == "CANCELED" || status == "canceled")
		{
		  pb_delete->setEnabled( false );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		  topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));

		  Completed_triples[ triple_curr_key ] = true;

		  if ( status == "FAILED" )
		    {
		      Failed_triples   [ triple_curr_key ] = true;
		      qDebug() << "FAILED status for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
		    }
		  if ( status == "CANCELED" || status == "canceled")
		    {
		      Canceled_triples   [ triple_curr_key ] = true;
		      qDebug() << "CANCELED status for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
		    }
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
  
  //check if there are canceled triples
  bool canceled_triples = false;
  QStringList Canceled_triples_list;
  
  QMap<QString, bool>::iterator tdc;
  for ( tdc = Canceled_triples.begin(); tdc != Canceled_triples.end(); ++tdc )
    {
      qDebug() << "Key/Value of Canceled_triples: " << tdc.key() << "/" << tdc.value();
      if ( tdc.value() )
	{
	  Canceled_triples_list << tdc.key();
	  canceled_triples = true;
	}
    }
  qDebug() << "Canceled_triples exist ? Name(s): " << canceled_triples << Canceled_triples_list;
  
  
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
      
      QString msg_text = "All triples have been processed.";
      
      if ( failed_triples )
	msg_text += QString("\n\n NOTE: analyses for the following triples failed: \n %1").arg( Failed_triples_list.join(", ") );

      if ( canceled_triples )
	msg_text += QString("\n\n NOTE: analyses for the following triples have been canceled: \n %1").arg( Canceled_triples_list.join(", ") );
 
      
      QMessageBox::information( this,
				tr( "All Triples Processed !" ),
				msg_text  );

      in_gui_update  = false; 
      
      //ALEXEY: Switch to next stage (Report) ?
    }

  in_gui_update  = false; 
}


//Slot to delete Job
void US_Analysis_auto::delete_job( QString triple_stage )
{
  QString tr_st = triple_stage.simplified();
  tr_st.replace( " ", "" );
    
  QStringList triple_stage_parts = tr_st.split("(");

  // qDebug() << "In DELETE: triple_stage: " << triple_stage;
  // qDebug() << "Parts: " << triple_stage_parts;

  QString stage_n = triple_stage_parts[0];
  QString triple_n  = triple_stage_parts[1];
  triple_n.chop(1);
  triple_n.replace("/",".");

  QString triple_n_copy = triple_n;
  QStringList triple_n_parts = triple_n_copy.split(".");
  QString channel_n = triple_n_parts[0] + "." + triple_n_parts[1];

  QStringList requestID_list = Channel_to_requestIDs[ channel_n ];
  qDebug() << "In delete other WVLs: channel info -- " <<  requestID_list;

  QMap <QString, QString > ana_details = Array_of_analysis[ triple_n ];
  QString requestID = ana_details["requestID"];
  qDebug() << "RequestID -- " << requestID;

  bool mwl_channel = false;
  QStringList triple_list_affected;
  
  if ( requestID_list.size() > 1 ) 
    {
      mwl_channel = true;
      //remove currently selected requestID -- for primary wvl
      requestID_list.removeOne( requestID );
      qDebug() << "In delete other WVLs: MODIFIED channel info -- " <<  requestID_list;
      
      for ( int i=0; i<requestID_list.size(); ++i )
	{
	  //extract triple information for the rest of wvl
	  QString requestID_affected = requestID_list[i];
	  QMap< QString, QString > ana_details_affected = Array_of_analysis_by_requestID[ requestID_affected ];
	  triple_list_affected << ana_details_affected[ "triple_name" ];
	}
    }
  
  QMessageBox msg_delete;
  msg_delete.setIcon(QMessageBox::Critical);
  msg_delete.setWindowTitle(tr("Job Deletion!"));
  
  QString msg_sys_text = QString("ATTENTION!\nYou have chosen to abort an ongoing job for: \n triple %1, stage %2!" )
    .arg( triple_n )
    .arg( stage_n  );

  if ( mwl_channel )
    msg_sys_text += QString("\n\nSince this is a multi-wavelength analysis, ongoing jobs for the following same-channel triples will be canceled as well: \n %1")
      .arg( triple_list_affected.join(",") );
  
  QString msg_sys_text_info = QString("Do you want to proceed?");
  msg_delete.setText( msg_sys_text );
  
  msg_delete.setInformativeText( msg_sys_text_info );
  
  QPushButton *Accept_sys    = msg_delete.addButton(tr("YES"),    QMessageBox::YesRole);
  QPushButton *Cancel_sys    = msg_delete.addButton(tr("Cancel"), QMessageBox::RejectRole);
  
  msg_delete.exec();

  if (msg_delete.clickedButton() == Accept_sys)
    {
      //Send signal to autoflowAnalysis record ? 
      qDebug() << "DELETION chosen !!";

      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );
      
      // Get the buffer data from the database
      if ( db.lastErrno() != US_DB2::OK )
	{
	  QMessageBox::warning( this, tr( "Connection Problem" ),
				tr( "Could not connect to database \n" ) +  db.lastError() );
	  return;
	}
      
      QMap <QString, QString > current_analysis;
      current_analysis = read_autoflowAnalysis_record( &db, requestID );
      
      qDebug() << "GUID to DETETE! -- " << current_analysis[ "CurrentGfacID" ];
      qDebug() << "Status && statusMsg to DETETE! -- " << current_analysis[ "status" ] << current_analysis[ "status_msg" ];

      update_autoflowAnalysis_uponDeletion( &db, requestID );

      // Now, if 2DSA or 2DSA-FM of the representative wvl in the MWL case: DELETE jobs for all other triples in a channel:
      if ( Process_2dsafm [ triple_n ] && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) && mwl_channel ) 
	update_autoflowAnalysis_uponDeletion_other_wvl( &db, requestID_list );
      
    }
  else if ( msg_delete.clickedButton() == Cancel_sys )
    return;
    
}

// slot to update autoflowAnalysis record when DELETE pressed
void US_Analysis_auto::update_autoflowAnalysis_uponDeletion ( US_DB2* db, const QString& requestID )
{
  // //Update
  // US_Passwd pw;
  // US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "Updating autoflowAnalysis at DELETEe (primary wvl): Could not connect to database \n" ) + db->lastError() );
      return;
    }

  
  QStringList qry;

  qry << "update_autoflow_analysis_record_at_deletion"
      << requestID;

  qDebug() << "DELETION: Query -- " << qry;
  db->query( qry );
    
  return;
}

// slot to update autoflowAnalysis records when DELETE pressed for all other wvl in a channel
void US_Analysis_auto::update_autoflowAnalysis_uponDeletion_other_wvl ( US_DB2* db, const QStringList& requestID_list )
{
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "Updating autoflowAnalysis at DELETE (other wvl): Could not connect to database \n" ) + db->lastError() );
      return;
    }
  
  QStringList qry;
  
  for (int i=0; i < requestID_list.size(); ++i )
    {
      qry.clear();
      qry << "update_autoflow_analysis_record_at_deletion_other_wvl"
	  << requestID_list[i];

      qDebug() << "Updating triple's record: " << requestID_list[i];
      qDebug() << "Query: " << qry;
      //checks is status is WAIT: what if not???
      
      db->query( qry );
    }
  
  return;
}

// slot to update autoflowAnalysis record at fitmen stage && register manually updated triple
void US_Analysis_auto::update_autoflowAnalysis_statuses (  QMap < QString, QString > & triple_info )
{
  QString triple_curr_key = triple_info[ "triple_name_key" ];
  QString requestID       = triple_info[ "requestID" ];

  //Identify list of requestIDs needed to be updated at FITMEN (especcially for MWL cases):
  QStringList triple_name_parts = triple_curr_key.split(".");
  QString channel_name = triple_name_parts[0] + "." + triple_name_parts[1];

  qDebug() << "In update_statuses: triple_name_parts: " << triple_name_parts;
  
  QStringList requestID_list = Channel_to_requestIDs[ channel_name ];

  qDebug() << "Channel_name, requestIDs to update: " << channel_name << requestID_list;
  
  //Update
  US_Passwd pw;
  US_DB2* dbP = new US_DB2( pw.getPasswd() );

  // --- Before updating autoflowAnalysis records ---
  // --- MAKE sure that ALL original 2DSA stages for other wavelength in a channel (besides representative wvl) are completed
  // --- i.e. are in WAIT status
  update_autoflowAnalysis_status_at_fitmen( dbP, requestID_list );

  //Restart timer:
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  timer_update->start(5000);

  qDebug() << "Timer restarted after updating EditProfiles for channel -- " << channel_name;
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
  Array_of_analysis_by_requestID.clear();
  Channel_to_requestIDs.clear();
  channels_all.clear();

  Failed_triples.clear();
  Canceled_triples.clear();
  Manual_update.clear();
  History_read.clear();
  Completed_triples.clear();
  Process_2dsafm.clear();
  //TO DO MORE later - DB stop etc..
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

  //Delete button
  //QPushButton* pb_delete = us_pushbutton( tr( "Delete" ) );
  QPushButton* pb_delete = new QPushButton( tr( "Delete" ) );
  pb_delete-> setObjectName("delete");
  
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
  genL->addWidget( le_runID,   row,    2, 1, 8 );

  genL->addWidget( pb_delete,  row++,  10, 6, 1);

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

  // Disable delete btn by default
  pb_delete->setEnabled( false );

  signalMapper = new QSignalMapper(this);
  connect(signalMapper, SIGNAL( mapped( QString ) ), this, SLOT( delete_job( QString ) ) );
  connect( pb_delete, SIGNAL( clicked() ), signalMapper, SLOT(map()));
  signalMapper->setMapping ( pb_delete, triple_name );
  
  return groupBox;
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
	  analysis_details[ "nextWaitStatus" ] = db->value( 15 ).toString();
	  analysis_details[ "nextWaitStatusMsg" ] = db->value( 16 ).toString();
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
	  analysis_details[ "nextWaitStatus" ] = db->value( 15 ).toString();
	  analysis_details[ "nextWaitStatusMsg" ] = db->value( 16 ).toString();

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
void US_Analysis_auto::update_autoflowAnalysis_status_at_fitmen( US_DB2* db, const QStringList& requestID_list )
{
  // // Check DB connection
  // US_Passwd pw;
  // QString masterpw = pw.getPasswd();
  // US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "Updating autoflowAnalysis at FITMEN: Could not connect to database \n" ) + db->lastError() );
      return;
     }
  
  QStringList qry;

  for (int i=0; i < requestID_list.size(); ++i )
    {
      qry.clear();
      qry << "update_autoflow_analysis_record_at_fitmen"
	  << requestID_list[i];

      qDebug() << "Updating triple's record: " << requestID_list[i];
      qDebug() << "Query: " << qry;
      //checks is status is WAIT: what if not???
      
      db->query( qry );
    }
  
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
