#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include "us_autoflow_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_lamm_astfvm.h"
#include "../us_fematch/us_thread_worker.h"

#define MIN_NTC   25

const QColor colorRed       ( 210, 0, 0 );
const QColor colorDarkGreen ( 2, 88, 57 );
const QColor colorGreen     ( 50, 205, 50 );
const QColor colorYellow    ( 255, 255, 102 );
const QColor colorBlue      ( 95, 152, 238 );

// Constructor
US_Analysis_auto::US_Analysis_auto() : US_Widgets()
{
  setWindowTitle( tr( "Autoflow Analysis Monitor"));
                       
  //setPalette( US_GuiSettings::frameColor() );
  
  QVBoxLayout* panel  = new QVBoxLayout( this );
  panel->setSpacing        ( 2 );
  panel->setContentsMargins( 2, 2, 2, 2 );

  QLabel* lb_hdr1          = us_banner( tr( "Analysis Stages for All Triples" ) );
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
  //QScroller *scroller = QScroller::scroller(treeWidget);
  //QScroller *scroller = QScroller::scroller(treeWidget->viewport());
  //scroller->grabGesture(treeWidget, QScroller::MiddleMouseButtonGesture);
  treeWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  
  setMinimumSize( 950, 450 );
  adjustSize();

  in_gui_update = false;
  in_reload_end_process = false;
  all_processed = true;
  
  // // // // // // ---- Testing ----
  // QMap < QString, QString > protocol_details;


  // protocol_details[ "invID_passed" ] = QString("95");
  // protocol_details[ "protocolName" ] = QString("MartinR_RP12_EcoRI_Digest_Optima1_24823-v3");
  // protocol_details[ "aprofileguid" ] = QString("19a5ef33-0a1e-499e-95ef-45daac7bdcb7");
  // protocol_details[ "filename" ]     = QString("MartinR_RP12_EcoRI_Digest_Optima1_24823-v3-run1963");
  // protocol_details[ "analysisIDs"  ] = QString( "3657,3658");

  // // //What's needed ////////////////////////////////////////////////////////
  // AProfileGUID       = protocol_details[ "aprofileguid" ];
  // ProtocolName_auto  = protocol_details[ "protocolName" ];
  // invID              = protocol_details[ "invID_passed" ].toInt();

  // FileName           = protocol_details[ "filename" ];
  
  // analysisIDs        = protocol_details[ "analysisIDs" ];
  // /////////////////////////////////////////////////////////////////////

  
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

  autoflowStatusID   = protocol_details[ "statusID" ].toInt();
  autoflowID_passed  = protocol_details[ "autoflowID" ].toInt();

  sim_msg_pos_x      = protocol_details[ "sim_msg_pos_x" ].toInt();
  sim_msg_pos_y      = protocol_details[ "sim_msg_pos_y" ].toInt();

  //Copy protocol details
  protocol_details_at_analysis = protocol_details;

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

      job1run      = false;
      job2run      = false;
      job3run      = false;
      job4run      = false;
      job5run      = false;
      job6run_pcsa = false;
      job3auto     = false;

      QString json = ana_details["status_json"];
      qDebug() << "triple: " << triple_curr << ", status_json: " << json;
      
      //TEST
      //json = "{\"to_process\":[\"FITMEN\",\"2DSA_IT\",\"2DSA_MC\"],\"processed\":[\"2DSA\"],\"submitted\":\"2DSA_FM\"}" ;

      if ( json.contains("2DSA") )    //ALEXEY: maybe more strict condition here? (as 2DSA is in other stages...)
	job1run = true;
      if ( json.contains("2DSA_FM") )
	job2run = true;
      if ( json.contains("FITMEN") )
	job3run = true;
      if ( json.contains("FITMEN_AUTO") )
	job3run = false;
      if ( json.contains("2DSA_IT") )
	job4run = true;	  
      if ( json.contains("2DSA_MC") )
	job5run = true;
      if ( json.contains("PCSA") )
	job6run_pcsa = true;
    
      
      
      triple_name_width = fmet.width( triple_curr );
      //triple_name_width = fmet.horizontalAdvance( triple_curr );
      
      qDebug() << "Triple,  width:  " << triple_curr << ", " << triple_name_width;
      qDebug() << "GUI: job1run, job2run, job3run, job4run, job5run -- "
	       << job1run << job2run << job3run << job4run << job5run;

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

      if ( job6run_pcsa )  //PCSA
	{
	  QString stage_name( tr("PCSA") );
	  QString child_name = stage_name + " (" + triple_curr + ")";
	  groupbox_PCSA[ triple_curr ] = createGroup( child_name  );

	  childItem_PCSA [ triple_curr ] = new QTreeWidgetItem();
	  topItem [ triple_curr ] -> addChild( childItem_PCSA [ triple_curr ] );
	  treeWidget->setItemWidget( childItem_PCSA [ triple_curr ] , 1, groupbox_PCSA [ triple_curr ] );
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

//Parse filename and extract one for given optics type in combined runs
QString US_Analysis_auto::get_filename( QString triple_name )
{
  qDebug() << "FileName -- " << FileName;
  qDebug() << "triple_name -- " << triple_name;
 
  QString filename_parsed;
  
  if ( FileName.contains(",") && FileName.contains("IP") && FileName.contains("RI") )
    {
      QStringList fileList  = FileName.split(",");

      //Interference
      if ( triple_name.contains("Interference") )
	{
	  for (int i=0; i<fileList.size(); ++i )
	    {
	      QString fname = fileList[i];
	      int pos = fname.lastIndexOf(QChar('-'));
	      qDebug() << "IP: pos -- " << pos;
	      qDebug() << "IP: fname.mid( pos ) -- " << fname.mid( pos );
	      if ( fname.mid( pos ) == "-IP")
		{
		  filename_parsed = fname;
		  break;
		}
	    }
	}
      //UV/vis.
      else
	{
	  for (int i=0; i<fileList.size(); ++i )
	    {
	      QString fname = fileList[i];
	      int pos = fname.lastIndexOf(QChar('-'));
	      qDebug() << "RI: pos -- " << pos;
	      qDebug() << "RI: fname.mid( pos ) -- " << fname.mid( pos );
	      if ( fname.mid( pos ) == "-RI")
		{
		  filename_parsed = fname;
		  break;
		}
	    }
	}
    }
  else
    filename_parsed = FileName;

  qDebug() << "Parsed filename: " <<  filename_parsed;

  return filename_parsed;
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
	  qDebug() << "All Doc: NOT a JSON Doc !!";
	  
	  QMessageBox::warning( this, tr( "JSON Format Problem" ),
				tr( "JSON message for status of the analysis performed on triple %1 appears to be corrupted. "
				    "This may be an indicaiton of the problem with the analysis run for this triple.\n\n"
				    "This problem will preclude status monitoring for other triples. "
				    "Please check log messages, or contact administrator for help.\n\n"
				    "The program will return to the autoflow runs dialog.").arg( triple_curr ) );
	  
	  in_gui_update  = false;

	  emit analysis_back_to_initAutoflow( );
	  
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

	      if ( stage_to_process == "FITMEN" || stage_to_process == "FITMEN_AUTO" )
		continue;
	      
	      
	      if ( stage_to_process == "2DSA" )
		to_process_stage_groupbox = groupbox_2DSA[ triple_curr ];
	      if ( stage_to_process == "2DSA_FM" )
		to_process_stage_groupbox = groupbox_2DSA_FM [ triple_curr ];
	      if ( stage_to_process == "2DSA_IT" )
		to_process_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
	      if ( stage_to_process == "2DSA_MC" )
		to_process_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];
	      if ( stage_to_process == "PCSA" )
		to_process_stage_groupbox = groupbox_PCSA [ triple_curr ];

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
	      if ( stage_name.contains("FITMEN") && ( stage_status == "CANCELED" || stage_status == "canceled" ) )
		{
		  if ( groupbox_2DSA_IT.contains( triple_curr ) )
		    {
		      processed_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
		      stage_name = "2DSA-IT";
		    }
		  
		  else if ( groupbox_2DSA_MC.contains( triple_curr ) )
		    {
		      processed_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];
		      stage_name = "2DSA-MC";
		    }
		  
		  qDebug() << "CANCELED status for triple/stage (FITMEN) -- " << triple_curr_key << "/" << submitted.toString();

		  stage_HPCAnalysisRequestID = "N/A";
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
	      if ( stage_name == "PCSA" )
		processed_stage_groupbox = groupbox_PCSA [ triple_curr ];

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
		  QPushButton * pb_overlay      = processed_stage_groupbox->findChild<QPushButton *>("overlay", Qt::FindDirectChildrenOnly);

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

		  
		  if ( stage_status == "COMPLETE" || stage_status == "complete" )
		    pb_overlay->setEnabled( true ); 
		  
		  //IF Canceled by user:
		  if ( stage_status == "CANCELED" || stage_status == "canceled" )
		    {
		      lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		      topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));
		      
		      pb_delete->setEnabled( false );
		      pb_overlay->setEnabled( false );
		      
		      Completed_triples[ triple_curr_key ] = true;
		      Canceled_triples [ triple_curr_key ] = true;
		      qDebug() << "CANCELED status( Processed ) for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
		    }
		  if ( stage_status == "FAILED" || stage_status == "failed" )
		    {
		      lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		      topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));

		      pb_delete->setEnabled( false );
		      pb_overlay->setEnabled( false );
		      
		      Completed_triples[ triple_curr_key ] = true;
		      Failed_triples [ triple_curr_key ] = true;
		      qDebug() << "FAILED status( Processed ) for triple/stage -- " << triple_curr_key << "/" << submitted.toString();
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


	  //New case of automated 'FITMEN_AUTO' stage:
	  if ( submitted.toString() == "FITMEN_AUTO" && Process_2dsafm[ triple_curr_key ] )
	    {
	      //What if submited stage still "FITMEN_AUTO" but nextWaitStatus="COMPLETE" ? (the submission daemon didn't yet updated submitted status...)
	      //I.e., FITMEN_AUTO was processed BUT submission daemon still sees it in 'status' "FITMEN_AUTO" ?
	      qDebug() << "FITMEN_AUTO: nextWaitStatus -- " << nextWaitStatus;
	      if ( nextWaitStatus == "COMPLETE" || nextWaitStatus == "complete"
		   || nextWaitStatus == "CANCELED" || nextWaitStatus == "canceled" )
		continue;
	      
	      //--- Check status of the FITMEN_AUTO | Entire Analysis for the triple 
	      bool fitmen_auto_processed;
	      fitmen_auto_processed = check_fitmen_status( requestID, stage_name );

	      if ( fitmen_auto_processed )
		{
		  qDebug() << "CHECK on FITMEN_AUTO: already processed ? -- " << fitmen_auto_processed;
		  in_gui_update  = false; 
		  
		  return;
		}
	      
	      //Stop timer here, and restart after processed automatically
	      timer_update -> stop();
	      disconnect(timer_update, SIGNAL(timeout()), 0, 0);
	      in_gui_update  = false;
	      qDebug() << "Update stopped at FITMEN_AUTO for triple -- " << triple_curr_key;
	      
	      //Process here:
	      QMap< QString, QString > triple_info_map_auto;
	      triple_info_map_auto[ "triple_name" ]     = triple_curr;
	      triple_info_map_auto[ "triple_name_key" ] = triple_curr_key;
	      triple_info_map_auto[ "requestID" ]       = requestID;
	      triple_info_map_auto[ "invID" ]           = QString::number(invID);
	      triple_info_map_auto[ "filename" ]        = filename; // ALEXEY -- NOT 'FileName' for combined runs;

	      fitmen_bad_vals   = false;
	      no_fm_data_auto   = false;
	      have3val          = true;  
	      
	      scan_dbase_auto      ( triple_info_map_auto );
	      get_editProfile_copy ( triple_info_map_auto );
	      file_loaded_auto     ( triple_info_map_auto );

	      if ( no_fm_data_auto )
		{
		  //triple_analysis_processed( );
		  delete_jobs_at_fitmen( triple_info_map_auto );

		  return;
		}
	      
	      //Now, update editProfiles
	      edit_update_auto  ( triple_info_map_auto );
	      	      
	      //End of automatic processing

	      return;	      
	    }
	  // END of new automated 'FITMEN_AUTO' stage

	  
	  if ( submitted.toString() == "FITMEN" && !Manual_update[ triple_curr_key ] && Process_2dsafm[ triple_curr_key ] )
	    {
	      QMessageBox::information( this,
					tr( "ATTENTION: FITMEN stage reached" ),
					tr( "FITMEN stage for triple %1 will be processed manually." ).arg( triple_curr ) );


	      //--- Check status of the FITMEN | Entire Analysis for the triple before calling FitMen constructor:
	      bool fitmen_processed;
	      fitmen_processed = check_fitmen_status( requestID, stage_name );

	      if ( fitmen_processed )
		{
		  Manual_update[ triple_curr_key ] = true;
		  qDebug() << "CHECK on FITMEN: already processed ? -- " << fitmen_processed;
		  
		  in_gui_update  = false; 
		  
		  return;
		}
	      //-------------------------------------------------------------------------------------------------//
	    	      
	      /** To FitMeniscus class -- pass:
		  1. Name of the triple to title && and possibly to pass to scan_db() method
		  2. Filename, triple_name
	      ****/
	      QMap< QString, QString > triple_info_map;
	      triple_info_map[ "triple_name" ]     = triple_curr;
	      triple_info_map[ "triple_name_key" ] = triple_curr_key;
	      triple_info_map[ "requestID" ]       = requestID;
	      triple_info_map[ "invID" ]           = QString::number(invID);
	      triple_info_map[ "filename" ]        = filename; // ALEXEY -- NOT 'FileName' for combined runs;
	      
	      fitmen_bad_vals   = false;
	      	      
	      FitMen = new US_FitMeniscus( triple_info_map );
	      	      
	      /** The following will block parent windows from closing BUT not from continuing timer execution ***/
	      FitMen->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
	      FitMen->setWindowModality(Qt::ApplicationModal);
	      /***************************************************************************************************/
	      connect( FitMen, SIGNAL( editProfiles_updated(  QMap < QString, QString > & ) ),
		       this, SLOT( update_autoflowAnalysis_statuses (  QMap < QString, QString > &) ) );

	      connect( FitMen, SIGNAL( editProfiles_updated_earlier( ) ),
		       this, SLOT( editProfiles_updated_earlier( ) ) );

	      connect( FitMen, SIGNAL( triple_analysis_processed( ) ),
		       this, SLOT( triple_analysis_processed( ) ) );

	      // connect( FitMen, SIGNAL( bad_meniscus_values( QMap < QString, QString > & ) ),  //This will take care of BAD values
	      // 	       this, SLOT( delete_jobs_at_fitmen( QMap < QString, QString > & ) ) );
		      
		       
	      FitMen->show();

	      Manual_update[ triple_curr_key ] = true;

	      //Stop timer here, and restart with the editProfile_updated signal ??
	      timer_update -> stop();
	      disconnect(timer_update, SIGNAL(timeout()), 0, 0);
	      
	      qDebug() << "Update stopped at FITMEN for triple -- " << triple_curr_key;
	      
	      in_gui_update  = false;

	      if ( FitMen -> no_fm_data )
		{
		  FitMen->close();
		  
		  delete_jobs_at_fitmen( triple_info_map );
		}

	      if ( FitMen -> bad_men_vals )
		{
		  FitMen->close();
		  fitmen_bad_vals = true;
		  
		  delete_jobs_at_fitmen( triple_info_map );
		}

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
	  if ( submitted.toString() == "PCSA" )
	    current_stage_groupbox = groupbox_PCSA [ triple_curr ];

	  //Special case: children triple of the channel, while parent triple (selected wvl) is CANCELED
	  if ( ( submitted.toString() == "FITMEN" || submitted.toString() == "FITMEN_AUTO" )
	       && ( nextWaitStatus == "CANCELED" || nextWaitStatus == "canceled") )
	    {
	      if ( groupbox_2DSA_IT.contains( triple_curr ) )
		{
		  current_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
		  stage_name = "2DSA-IT";
		}
	      
	      else if ( groupbox_2DSA_MC.contains( triple_curr ) )
		{
		  current_stage_groupbox = groupbox_2DSA_MC [ triple_curr ];
		  stage_name = "2DSA-MC";
		}
	      
	      //current_stage_groupbox = groupbox_2DSA_IT [ triple_curr ];
	      //stage_name = "2DSA-IT";
	      
	      curr_HPCAnalysisRequestID = "N/A";
	      status_msg = "Job has been scheduled for deletion";
	      status = "CANCELED";

	      if ( fitmen_bad_vals  )
		status = "FAILED";
	      
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
	      QPushButton * pb_overlay      = current_stage_groupbox->findChild<QPushButton *>("overlay", Qt::FindDirectChildrenOnly);

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
	      if ( status == "READY" || status == "SUBMITTED" )
		{
		  lineedit_status -> setText( "QUEUED" );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(95, 152, 238); }");
		}
	      else
		{
		  lineedit_status -> setText( status );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(50, 205, 50); }");
		}
	      
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
		  pb_overlay->setEnabled( true );
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

		      /** Comment the line BELOW as this MAY cause RACE CONDITIONS !!!!!***************************/
		      //Completed_triples[ triple_curr_key ] = true;  // May cause race conditions!!!!
		      /*******************************************************************************************/
		      
		      qDebug() << "set Completed_triples[ " << triple_curr_key << " ] to " << Completed_triples[ triple_curr_key ];
		      
		    }
		}

	      //check if failed -- due to the abortion/deletion of the job
	      if ( status == "FAILED" || status == "failed" || status == "CANCELED" || status == "canceled")
		{
		  pb_overlay->setEnabled( false );
		  pb_delete->setEnabled( false );
		  lineedit_status   -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); color : white; }");
		  topItem [ triple_curr ]  -> setForeground( 0,  QBrush( colorRed ));

		  /** Comment the line BELOW as this MAY cause RACE CONDITIONS !!!!!***************************/
		  //Completed_triples[ triple_curr_key ] = true;
		  /*******************************************************************************************/

		  if ( status == "FAILED" || status == "failed" )
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
  all_processed = true;
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
	msg_text += QString("\n\nNOTE: analyses for the following triples FAILED: \n\n%1").arg( Failed_triples_list.join(", ") );

      if ( canceled_triples )
	msg_text += QString("\n\nNOTE: analyses for the following triples have been CANCELED: \n\n%1").arg( Canceled_triples_list.join(", ") );

      msg_text +=  QString("\n\nThe program will proceed to the Reporting stage. ");

      //Update autoflow record at Analysis completion
      update_autoflow_record_atAnalysis();

      if ( failed_triples || canceled_triples )
	{
	  QMessageBox::information( this,
				    tr( "All Triples Processed !" ),
				    msg_text  );
	}
      
      in_gui_update  = false; 
      
      //ALEXEY: Append with info on failed triples when bad_meniscus values:
      protocol_details_at_analysis[ "failed" ] = "";

      //ALEXEY: Switch to next stage (Report)
      emit analysis_complete_auto( protocol_details_at_analysis );  
    }

  in_gui_update  = false; 
}


//Update autoflow record upon Analysis completion
void US_Analysis_auto::update_autoflow_record_atAnalysis( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }

   QStringList qry;
   // qry << "update_autoflow_at_analysis"
   //     << protocol_details_at_analysis[ "runID" ]
   //     << protocol_details_at_analysis[ "OptimaName" ];
   qry << "update_autoflow_at_analysis"
       << QString::number( autoflowID_passed );

   //db->query( qry );

   int status = db->statusQuery( qry );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
     {
       QMessageBox::warning( this,
			     tr( "Autoflow Record Not Updated" ),
			     tr( "No autoflow record\n"
				 "associated with this experiment." ) );
       return;
     }

}

// public function to get pointer to edit data
US_DataIO::EditedData*      US_Analysis_auto::aa_editdata() { return edata;     }

// public function to get pointer to list of excluded scans
QList< int >*               US_Analysis_auto::aa_excllist() { return &excludedScans;}

// public function to get pointer to sim data
US_DataIO::RawData*         US_Analysis_auto::aa_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_Analysis_auto::aa_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_Analysis_auto::aa_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_Analysis_auto::aa_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_Analysis_auto::aa_resbmap()  { return rbmapd;    }

QString  US_Analysis_auto::aa_tripleInfo()  { return tripleInfo;    }

//Load rawData/editedData
bool US_Analysis_auto::loadData( QMap < QString, QString > & triple_information )
{
  rawData.clear();
  editedData.clear();
  
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  int rID=0;
  QString rfilename;
  int eID=0;
  QString efilename;
  
  //get EditedData filename && editedDataID for current triple, then infer rawDataID 
  QStringList query;
  query << "get_editedDataFilenamesIDs" << triple_information["filename"];
  db->query( query );

  qDebug() << "In loadData() Query: " << query;
  qDebug() << "In loadData() Query: triple_information[ \"triple_name\" ]  -- " << triple_information[ "triple_name" ];

  int latest_update_time = 1e100;

  QString triple_name_actual = triple_information[ "triple_name" ];

  if ( triple_name_actual.contains("Interference") )
    triple_name_actual.replace( "Interference", "660" );
  
  while ( db->next() )
    {
      QString  filename            = db->value( 0 ).toString();
      int      editedDataID        = db->value( 1 ).toInt();
      int      rawDataID           = db->value( 2 ).toInt();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date               = db->value( 3 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();
               
      if ( filename.contains( triple_name_actual ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time )
	    {
	      latest_update_time = time_to_now;
	      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

	      rID       = rawDataID;
	      eID       = editedDataID;
	      efilename = filename;
	    }
	}
    }

  qDebug() << "In loadData() after Query ";
  
  QString edirpath  = US_Settings::resultDir() + "/" + triple_information[ "filename" ];
  QDir edir( edirpath );
  if (!edir.exists())
    edir.mkpath( edirpath );
  
  QString efilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + efilename;

  qDebug() << "In loadData() efilename: " << efilename;

  
  // Can check here if such filename exists
  // QFileInfo check_file( efilepath );
  // if ( check_file.exists() && check_file.isFile() )
  //   qDebug() << "EditProfile file: " << efilepath << " exists";
  // else
  db->readBlobFromDB( efilepath, "download_editData", eID );

  qDebug() << "In loadData() after readBlobFromDB ";

  //Now download rawData corresponding to rID:
  QString efilename_copy = efilename;
  QStringList efilename_copy_list = efilename_copy.split(".");

  rfilename = triple_information[ "filename" ] + "." + efilename_copy_list[2] + "."
                                               + efilename_copy_list[3] + "."
                                               + efilename_copy_list[4] + "."
                                               + efilename_copy_list[5] + ".auc";
  
  QString rfilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + rfilename;
  //do we need to check for existance ?
  db->readBlobFromDB( rfilepath, "download_aucData", rID );

  qApp->processEvents();

  qDebug() << "Loading eData, rawData: efilepath, rfilepath, eID, rID --- " << efilepath << rfilepath << eID << rID;

  //Put downloaded data in memory:
  QString uresdir = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/"; 
  US_DataIO::loadData( uresdir, efilename, editedData, rawData );

  eID_global = eID;

  qDebug() << "END of loadData(), eID_global: " << eID_global;

  return true;
}

//Load rawData/editedData
bool US_Analysis_auto::loadModel( QMap < QString, QString > & triple_information )
{
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  //first, get ModelIDs corresponding to editedDataID AND triple_stage && select latest one
  QStringList query;
  query << "get_modelDescsIDs" << triple_information[ "eID" ];
  db->query( query );
  
  qDebug() << "In loadModel() Query: " << query;
  
  int latest_update_time = 1e100;
  int mID=0;
  
  while ( db->next() )
    {
      QString  description         = db->value( 0 ).toString();
      int      modelID             = db->value( 1 ).toInt();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date               = db->value( 2 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();
      
      if ( description.contains( triple_information[ "stage_name" ] ) ) 
	{
	  if ( triple_information[ "stage_name" ] == "2DSA" )
	    {
	      if ( !description.contains("-FM_") && !description.contains("-IT_") && !description.contains("-MC_") && !description.contains("_mcN") )
		{
		  int time_to_now = date.secsTo(now);
		  if ( time_to_now < latest_update_time )
		    {
		      latest_update_time = time_to_now;
		      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

		      qDebug() << "Model 2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  		      
		      mID       = modelID;
		    }
		}
	    }
	  else
	    {
	      int time_to_now = date.secsTo(now);
	      if ( time_to_now < latest_update_time )
		{
		  latest_update_time = time_to_now;
		  //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);
		  
		  qDebug() << "Model NON-2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  
		  mID       = modelID;
		}
	    }
	}
    }
  
  int  rc      = 0;
  qDebug() << "ModelID to retrieve: -- " << mID;
  rc   = model.load( QString::number( mID ), db );
  qDebug() << "LdM:  model load rc" << rc;
  qApp->processEvents();

  model_loaded = model;   // Save model exactly as loaded
  model_used   = model;   // Make that the working model
  is_dmga_mc   = ( model.monteCarlo  &&
		   model.description.contains( "DMGA" )  &&
		   model.description.contains( "_mcN" ) );
  qDebug() << "post-Load mC" << model.monteCarlo << "is_dmga_mc" << is_dmga_mc
	   << "description" << model.description;
  
  if ( model.components.size() == 0 )
    {
      QMessageBox::critical( this, tr( "Empty Model" ),
			     tr( "Loaded model has ZERO components!" ) );
      return false;
    }
  
  ti_noise.count = 0;
  ri_noise.count = 0;
  ti_noise.values.clear();
  ri_noise.values.clear();

  //Load noise files
  triple_information[ "mID" ] = QString::number( mID );

  progress_msg->setValue( 4 );
  qApp->processEvents();

  loadNoises( triple_information );
  
  return true;
}

//Load Noises
bool US_Analysis_auto::loadNoises( QMap < QString, QString > & triple_information )
{
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return false;
    }

  // get noiseIDs, types & lastUpd by modelID
  QStringList query;
  query << "get_noiseTypesIDs" << triple_information[ "mID" ];
  db->query( query );
  
  qDebug() << "In loadNoises() Query: " << query;

  int latest_update_time_ti = 1e100;
  int latest_update_time_ri = 1e100;
  int nID_ti=0;
  int nID_ri=0;
  
  while ( db->next() )
    {
      int       noiseID        = db->value( 0 ).toInt();
      QString   noiseType      = db->value( 1 ).toString();
      //QString  date                = US_Util::toUTCDatetimeText( db->value( 2 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
      QDateTime date          = db->value( 2 ).toDateTime();

      QDateTime now = QDateTime::currentDateTime();

      qDebug() << "Noises: noiseID, noiseType, date -- " << noiseID << noiseType << date; 
      
      if ( noiseType.contains( "ti_" ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time_ti )
	    {
	      latest_update_time_ti = time_to_now;
	      	      
	      nID_ti       = noiseID;
	    }
	}
      if ( noiseType.contains( "ri_" ) ) 
	{
	  int time_to_now = date.secsTo(now);
	  if ( time_to_now < latest_update_time_ri )
	    {
	      latest_update_time_ri = time_to_now;
	      	      
	      nID_ri       = noiseID;
	    }
	}
    }


  //ALEXEY: treat the case when model (like -MC does not possess its own noises -- use latest available noises for prior model like -IT  )
  //int US_LoadableNoise::count_noise() in ../../gui/us_loadable_noise.cpp
  //void US_FeMatch::load_noise( ) in us_fematch.cpp
  if ( !nID_ti && !nID_ri ) 
    loadNoises_whenAbsent();
  
  //creare US_noise objects
  if ( nID_ti )
    {
      ti_noise.load( QString::number( nID_ti ), db );
      qDebug() << "loadNoises() NORMAL: ti_noise created: ID -- " << nID_ti;
    }
  if ( nID_ri )
    {
      ri_noise.load( QString::number( nID_ri ), db );
      qDebug() << "loadNoises() NORMAL: ri_noise created: ID -- " << nID_ri;
    }
  
  // noise loaded:  insure that counts jive with data
  int ntinois = ti_noise.values.size();
  int nrinois = ri_noise.values.size();
  int nscans  = edata->scanCount();
  int npoints = edata->pointCount();
  int npadded = 0;


  qDebug() << "ti_noise.count, ri_noise.count: " <<  ti_noise.count << ri_noise.count;
  qDebug() << "ti_noise.values.size(), ri_noise.values.size(): " << ti_noise.values.size() << ri_noise.values.size();
  
  if ( ntinois > 0  &&  ntinois < npoints )
    {  // pad out ti noise values to radius count
      int jj      = ntinois;
      while ( jj++ < npoints )
	ti_noise.values << 0.0;
      ti_noise.count = ti_noise.values.size();
      npadded++;
    }
  
  if ( nrinois > 0  &&  nrinois < nscans )
    {  // pad out ri noise values to scan count
      int jj      = nrinois;
      while ( jj++ < nscans )
	ri_noise.values << 0.0;
      ri_noise.count = ri_noise.values.size();
      npadded++;
    }
  
  if ( npadded  > 0 )
      {  // let user know that padding occurred
	QString pmsg;
	
	if ( npadded == 1 )
	  pmsg = tr( "The noise file was padded out with zeroes\n"
		     "in order to match the data range." );
	else
	  pmsg = tr( "The noise files were padded out with zeroes\n"
		     "in order to match the data ranges." );
	
	QMessageBox::information( this, tr( "Noise Padded Out" ), pmsg );
      }

  return true;
}

//Load Noises when absent for the model loaded (like -MC models)
void US_Analysis_auto::loadNoises_whenAbsent( )
{
   QStringList mieGUIDs;  // list of GUIDs of models-in-edit
   QStringList nieGUIDs;  // list of GUIDS:type:index of noises-in-edit
   QString     editGUID  = edata->editGUID;         // loaded edit GUID
   QString     modelGUID = model.modelGUID;         // loaded model GUID
   
   int noisdf  = US_Settings::noise_dialog();
   int nenois  = count_noise_auto( edata, &model, mieGUIDs, nieGUIDs );

   qDebug() << "load_noise_whenAbsent(): mieGUIDs, nieGUIDs, editGUID, modelGUID -- " <<  mieGUIDs << nieGUIDs << editGUID << modelGUID;
   qDebug() << "load_noise_whenAbsent(): noisdf, nenois -- " << noisdf << nenois;

   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      US_Passwd pw;
      US_DB2* dbP  = new US_DB2( pw.getPasswd() );

      if ( nenois > 1  &&  noisdf == 0 )
      {  // Noise exists and noise-dialog flag set to "Auto-load"
         QString descn = nieGUIDs.at( 0 );
         QString noiID = descn.section( ":", 0, 0 );
         QString typen = descn.section( ":", 1, 1 );
         QString mdlx1 = descn.section( ":", 2, 2 );

         if ( typen == "ti" )
	   {
	     ti_noise.load( true, noiID, dbP );
	     qDebug() << "load_noise_whenAbsent(): ti_noise created: ID -- " << noiID;
	     qDebug() << "load_noise_whenAbsent(): ti_noise.count: -- " << ti_noise.count;
	   }
         else
	   {
	     ri_noise.load( true, noiID, dbP );
	     qDebug() << "load_noise_whenAbsent(): ri_noise created: ID -- " << noiID;
	     qDebug() << "load_noise_whenAbsent(): ri_noise.count: -- " << ri_noise.count;
	   }
	 
         descn         = nieGUIDs.at( 1 );
         QString mdlx2 = descn.section( ":", 2, 2 );
         int kenois    = ( mdlx1 == mdlx2 ) ? 2 : 1;
         if ( kenois == 2 )
         {  // Second noise from same model:  g/et it, too
            noiID         = descn.section( ":", 0, 0 );
            typen         = descn.section( ":", 1, 1 );
            if ( typen == "ti" )
	      {
		ti_noise.load( true, noiID, dbP );
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti--" << noiID;
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ti_noise.count --" << ti_noise.count;
	      }
            else
	      {
		ri_noise.load( true, noiID, dbP );
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri--" << noiID;
		qDebug() << "load_noise_whenAbsent(): Noise 2: noiID, loaded ri_noise.count --" << ri_noise.count;
	      }
	 }

      }
      else
      {  // only 1:  just load it
         QString noiID = nieGUIDs.at( 0 );
         QString typen = noiID.section( ":", 1, 1 );
         noiID         = noiID.section( ":", 0, 0 );

         if ( typen == "ti" )
	   ti_noise.load( true, noiID, dbP );
	 
         else
	   ri_noise.load( true, noiID, dbP );
      }
     
   }
      
      
}


// Determine if edit/model related noise available and build lists
int US_Analysis_auto::count_noise_auto( US_DataIO::EditedData* edata,
					US_Model* model, QStringList& mieGUIDs, QStringList& nieGUIDs  )
{
   int noidiag = US_Settings::noise_dialog();

   int nenois  = 0;       // number of edited-data-related noises
   
   if ( edata == NULL )
     return nenois;

   QStringList nimGUIDs;  // list of GUIDs:type:index of noises-in-models
   QStringList tmpGUIDs;  // temporary noises-in-model list
   QString     daEditGUID = edata->editGUID;        // loaded edit GUID
   QString     modelGUID  = ( model == 0 ) ?        // loaded model GUID
                           "" : model->modelGUID;
   QString     lmodlGUID;                           // list model GUID
   QString     lnoisGUID;                           // list noise GUID
   QString     modelIndx;                           // "0001" style model index

   id_list_db_auto  ( daEditGUID );

   // Get a list of models-with-noise tied to the loaded edit
   int nemods  = models_in_edit_auto ( daEditGUID, mieGUIDs );

   if ( nemods == 0 )
   {
     //QApplication::restoreOverrideCursor();
      return nemods;          // Go no further if no models-with-noise for edit
   }

   int latemx  = 0;   // Index to latest model-in-edit

   // If no model is loaded, pick the model GUID of the latest noise
   if ( model == 0 )
      modelGUID   = mieGUIDs[ latemx ];

   // Get a list of noises tied to the loaded model
   int nmnois  = noises_in_model_auto( modelGUID, nimGUIDs );

   // If the loaded model has no noise, try the latest model
   if ( nmnois == 0 )
   {
      modelGUID   = mieGUIDs[ latemx ];
      nmnois      = noises_in_model_auto( modelGUID, nimGUIDs );
   }

   // Insure that the loaded/latest model heads the model-in-edit list
   if ( modelGUID != mieGUIDs[ 0 ] )
   {
      if ( ! mieGUIDs.removeOne( modelGUID ) )
      {
         qDebug( "*ERROR* Loaded/Latest model not in model-in-edit list!" );
         QApplication::restoreOverrideCursor();
         return 0;
      }

      mieGUIDs.insert( 0, modelGUID );
   }


   int kk = 0;                // running output models index

   if ( nmnois > 0 )
   {  // If loaded model has noise, put noise in list
      nieGUIDs << nimGUIDs;   // initialize noise-in-edit list
      kk++;
   }

   nenois      = nmnois;      // initial noise-in-edit count is noises in model

   for ( int ii = 1; ii < nemods; ii++ )
   {  // Search through models in edit
      lmodlGUID  = mieGUIDs[ ii ];                    // this model's GUID
      modelIndx  = QString().sprintf( "%4.4d", kk );  // models-in-edit index

      // Find the noises tied to this model
      int kenois = noises_in_model_auto( lmodlGUID, tmpGUIDs );

      if ( kenois > 0 )
      {  // if we have 1 or 2 noises, add to noise-in-edit list
         nenois    += qMin( 2, kenois );
         // adjust entry to have the right model-in-edit index
         lnoisGUID  = tmpGUIDs.at( 0 ).section( ":", 0, 1 )
            + ":" + modelIndx;
         nieGUIDs << lnoisGUID;
         if ( kenois > 1 )
         {  // add a second noise to the list
            lnoisGUID  = tmpGUIDs.at( 1 ).section( ":", 0, 1 )
               + ":" + modelIndx;
            nieGUIDs << lnoisGUID;
         }
	 
         kk++;
      }
   }


   if ( nenois > 0 )
   {  // There is/are noise(s):  ask user if she wants to load
      QMessageBox msgBox;
      QString     amsg;
      QString     msg;

      if ( model == 0 )
         amsg = tr( ", associated with the loaded edit.\n" );

      else
         amsg = tr( ", associated with the loaded edit/model.\n" );

      if ( nenois > 1 )
      {
         msg  = tr( "There are noise files" ) + amsg
              + tr( "Do you want to load some of them?" );
      }

      else
      {  // Single noise file: check its value range versus experiment
         QString noiID  = nieGUIDs.at( 0 ).section( ":", 0, 0 );
         US_Noise i_noise;

	 US_Passwd pw;
	 US_DB2 db( pw.getPasswd() );
	 i_noise.load( true, noiID, &db );
         
         double datmin  = edata->value( 0, 0 );
         double datmax  = datmin;
         double noimin  = 1.0e10;
         double noimax  = -noimin;
         int    npoint  = edata->pointCount();

         for ( int ii = 0; ii < edata->scanData.size(); ii++ )
         {
            for ( int jj = 0; jj < npoint; jj++ )
            {
               double datval = edata->value( ii, jj );
               datmin        = qMin( datmin, datval );
               datmax        = qMax( datmax, datval );
            }
         }

         for ( int ii = 0; ii < i_noise.values.size(); ii++ )
         {
            double noival = i_noise.values[ ii ];
            noimin        = qMin( noimin, noival );
            noimax        = qMax( noimax, noival );
         }

         if ( ( noimax - noimin ) > ( datmax - datmin ) )
         {  // Insert a warning if noise appears corrupt or unusual
            amsg = amsg
               + tr( "\nBUT THE NOISE HAS AN UNUSUALLY LARGE DATA RANGE.\n\n" );
         }

         msg  = tr( "There is a noise file" ) + amsg
              + tr( "Do you want to load it?" );
      }

DbgLv(2) << "LaNoi:noidiag  " << noidiag;
      if ( noidiag > 0 )
      {
         msgBox.setWindowTitle( tr( "Edit/Model Associated Noise" ) );
         msgBox.setText( msg );
         msgBox.setStandardButtons( QMessageBox::No | QMessageBox::Yes );
         msgBox.setDefaultButton( QMessageBox::Yes );

         if ( msgBox.exec() != QMessageBox::Yes )
         {  // user did not say "yes":  return zero count
            nenois  = 0;       // number of edited-data-related noises
         }
      }

      if ( kk < nemods )
      {  // Models with noise were found, so truncate models list
         for ( int ii = 0; ii < ( nemods - kk ); ii++ )
            mieGUIDs.removeLast();
      }
   }

   return nenois;
}


// build a list of noise(GUIDs) for a given model(GUID)
int US_Analysis_auto::noises_in_model_auto( QString mGUID, QStringList& nGUIDs )
{
   QString xnGUID;
   QString xmGUID;
   QString xntype;

   nGUIDs.clear();

   for ( int ii = 0; ii < noiIDs.size(); ii++ )
   {  // Examine noises list; Save to this list if model GUID matches
      xnGUID = noiIDs  .at( ii );
      xmGUID = noiMoIDs.at( ii );
      xntype = noiTypes.at( ii );

      xntype = xntype.contains( "ri_nois", Qt::CaseInsensitive ) ?
	"ri" : "ti";
     
      if ( mGUID == xmGUID )
	nGUIDs << xnGUID + ":" + xntype + ":0000";
   }
   
   return nGUIDs.size();
}

// Build a list of models(GUIDs) for a given edit(GUID)
int US_Analysis_auto::models_in_edit_auto( QString eGUID, QStringList& mGUIDs )
{
   QString xmGUID;
   QString xeGUID;
   QString xrGUID;
   QStringList reGUIDs;

   mGUIDs.clear();

   for ( int ii = 0; ii < modIDs.size(); ii++ )
   {  // Examine models list; Save to this list if edit GUID matches
      xmGUID = modIDs.at( ii );
      xeGUID = modEdIDs.at( ii );
     
      if ( eGUID == xeGUID )
      {
         mGUIDs << xmGUID;
      }
   }

   return mGUIDs.size();
}

// Build lists of noise and model IDs for database
int US_Analysis_auto::id_list_db_auto( QString daEditGUID )
{
   QStringList query;
   
   US_Passwd pw;
   US_DB2    db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
      return 0;

   query.clear();
   query << "get_editID" << daEditGUID;
   db.query( query );
   db.next();
   QString daEditID = db.value( 0 ).toString();
DbgLv(1) << "LaNoi:idlDB:  daEdit ID GUID" << daEditID << daEditGUID;

   noiIDs  .clear();
   noiEdIDs.clear();
   noiMoIDs.clear();
   noiTypes.clear();
   modIDs  .clear();
   modEdIDs.clear();
   modDescs.clear();

   QStringList reqIDs;
   QString     noiEdID;

   // Build noise, edit, model ID lists for all noises
   query.clear();
   query << "get_noise_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate lists from noise records
      noiEdID   = db.value( 2 ).toString();

      noiIDs   << db.value( 1 ).toString();
      noiTypes << db.value( 4 ).toString();
      noiMoIDs << db.value( 5 ).toString();
   }

DbgLv(1) << "LaNoi:idlDB: noiTypes size" << noiTypes.size();
   // Build model, edit ID lists for all models
   query.clear();
   query << "get_model_desc_by_editID" << QString::number( invID ) << daEditID;
   db.query( query );

   while ( db.next() )
   {  // Accumulate from db desc entries matching noise model IDs
      QString modGUID = db.value( 1 ).toString();
      QString modEdID = db.value( 6 ).toString();

      if ( noiMoIDs.contains( modGUID )  &&   modEdID == daEditID )
      {  // Only list models that have associated noise and match edit
         modIDs   << modGUID;
         modDescs << db.value( 2 ).toString();
         modEdIDs << db.value( 5 ).toString();
      }
   }
DbgLv(1) << "LaNoi:idlDB: modDescs size" << modDescs.size();

   // Loop through models to edit out any extra monteCarlo models
   for ( int ii = modIDs.size() - 1; ii >=0; ii-- )
   {  // Work from the back so any removed records do not affect indexes
      QString mdesc  = modDescs.at( ii );
      QString asysID = mdesc.section( ".", -2, -2 );
      bool    mCarlo = ( asysID.contains( "-MC" )  &&
                         asysID.contains( "_mc" ) );
      QString reqID  = asysID.section( "_", 0, -2 );

      if ( mCarlo )
      {  // Treat monte carlo in a special way (as single composite model)
         if ( reqIDs.contains( reqID ) )
         {  // already have this request GUID, so remove this model
            modIDs  .removeAt( ii );
            modDescs.removeAt( ii );
            modEdIDs.removeAt( ii );
         }

         else
         {  // This is the first time for this request, so save it in a list
            reqIDs << reqID;
         }
      }
   }

   // Create list of edit GUIDs for noises
   for ( int ii = 0; ii < noiTypes.size(); ii++ )
   {
      QString moGUID  = noiMoIDs.at( ii );
      int     jj      = modIDs.indexOf( moGUID );
DbgLv(2) << "LaNoi:idlDB: ii jj moGUID" << ii << jj << moGUID;

      QString edGUID  = ( jj < 0 ) ? "" : modEdIDs.at( jj );

      noiEdIDs << edGUID;
   }

   return noiIDs.size();
}


//Simulate Model
void US_Analysis_auto::simulateModel( )
{
  progress_msg->setLabelText( "Simulating model..." );
  progress_msg->setValue( 0 );
  
  int    nconc   = edata->pointCount();
  double radlo   = edata->radius( 0 );
  double radhi   = edata->radius( nconc - 1 );

  int lc=model_used.components.size()-1;
  qDebug() << "SimMdl: 0) s D c"
	   << model_used.components[ 0].s << model_used.components[ 0].D
	   << model_used.components[ 0].signal_concentration << "  n" << lc;
  qDebug() << "SimMdl: n) s D c"
	   << model_used.components[lc].s << model_used.components[lc].D
	   << model_used.components[lc].signal_concentration;

  adjustModel();

  qDebug() << "SimMdl: 0) s D c"
	   << model.components[ 0].s << model.components[ 0].D
	   << model.components[ 0].signal_concentration;
  qDebug() << "SimMdl: n) s D c"
	   << model.components[lc].s << model.components[lc].D
	   << model.components[lc].signal_concentration;
   
  // Initialize simulation parameters using edited data information
  US_Passwd pw;
  US_DB2* dbP = new US_DB2( pw.getPasswd() );
  
  simparams.initFromData( dbP, *edata, dat_steps );
  simparams.simpoints         = adv_vals[ "simpoints" ].toInt();
  simparams.meshType          = US_SimulationParameters::ASTFEM;
  simparams.gridType          = US_SimulationParameters::MOVING;
  simparams.radial_resolution = (double)( radhi - radlo ) / ( nconc - 1 );
  //   simparams.bottom            = simparams.bottom_position;
  qDebug() << "SimMdl: simpoints" << simparams.simpoints
	   << "rreso" << simparams.radial_resolution
	   << "bottom_sim" << simparams.bottom << "bottom_dat" << edata->bottom;
  //simparams.meniscus          = 5.8;
  
  //sdata.scanData.resize( total_scans );
  //int terpsize    = ( points + 7 ) / 8;
  
  if ( exp_steps )
    simparams.speed_step        = speed_steps;
  
  qDebug() << "SimMdl: speed_steps:" << simparams.speed_step.size();
  
  QString mtyp = adv_vals[ "meshtype"   ];
  QString gtyp = adv_vals[ "gridtype"   ];
  QString bvol = adv_vals[ "bandvolume" ];
  
  
  if ( gtyp.contains( "Constant" ) )
    simparams.gridType = US_SimulationParameters::FIXED;
  
  if ( model.components[ 0 ].sigma == 0.0  &&
       model.components[ 0 ].delta == 0.0)
    simparams.meshType = US_SimulationParameters::ASTFEM;
  else
    simparams.meshType = US_SimulationParameters::ASTFVM;
  
  simparams.firstScanIsConcentration = false;
  
  double concval1      = 0.0;
  
  if ( simparams.band_forming )
    {
      simparams.band_volume = bvol.toDouble();
      //concval1              = 1.0;
      //simparams.firstScanIsConcentration = true;
    }
  else
    simparams.band_volume = 0.0;
  
  // Make a simulation copy of the experimental data without actual readings

  qDebug() << "SimulateModel: --- 1";
  
  US_AstfemMath::initSimData( *sdata, *edata, concval1 );// Gets experimental time grid set

  qDebug() << "SimulateModel: --- 2";
  
  
  QString tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/"
    + FileName_parsed + ".time_state.tmst";
  QFileInfo check_file( tmst_fpath );
  simparams.sim      = ( edata->channel == "S" );
  
  if ( ( check_file.exists() ) && ( check_file.isFile() ) )
    {
      if ( US_AstfemMath::timestate_onesec( tmst_fpath, *sdata ) )
	{  // Load timestate that is already at 1-second-intervals
	  simparams.simSpeedsFromTimeState( tmst_fpath );
	  qDebug() << "SimMdl: timestate file exists" << tmst_fpath << " timestateobject,count = "
		   << simparams.tsobj << simparams.sim_speed_prof.count();
	  simparams.speedstepsFromSSprof();
	}
      
      else
	{  // Replace timestate with a new one that is at 1-second-intervals
	  // if ( drow == 0 )
	  //   {
	  QString tmst_fdefs = QString( tmst_fpath ).replace( ".tmst", ".xml" );
	  QString tmst_fpsav = tmst_fpath + "-orig";
	  QString tmst_fdsav = tmst_fdefs + "-orig";
	  simparams.sim      = ( edata->channel == "S" );
	  
	  // Rename existing (non-1sec-intv) files
	  QFile::rename( tmst_fpath, tmst_fpsav );
	  QFile::rename( tmst_fdefs, tmst_fdsav );
	  // Create a new 1-second-interval file set
	  US_AstfemMath::writetimestate( tmst_fpath, simparams, *sdata );
	  qDebug() << "SimMdl: 1-sec-intv file created";
	  // }
	  
	  simparams.simSpeedsFromTimeState( tmst_fpath );
	  simparams.speedstepsFromSSprof();
	}
    }
  else
    {
      qDebug() << "SimMdl: timestate file does not exist";
      if ( ! simparams.sim )
	{  // Compute acceleration rate for non-astfem_sim data
	  const double tfac = ( 4.0 / 3.0 );
	  double t2   = simparams.speed_step[ 0 ].time_first;
	  double w2t  = simparams.speed_step[ 0 ].w2t_first;
	  double om1t = simparams.speed_step[ 0 ].rotorspeed * M_PI / 30.0;
	  double w2   = sq( om1t );
	  double t1   = tfac * ( t2 - ( w2t / w2 ) );
	  int t_acc   = (int)qRound( t1 );
	  double rate = (double)( simparams.speed_step[ 0 ].rotorspeed )
	    / (double)t_acc;
	  qDebug() << "SimMdl:  accel-calc:  t1 t2 w2t t_acc speed rate"
		   << t1 << t2 << w2t << t_acc << simparams.speed_step[0].rotorspeed << rate;
	  simparams.speed_step[ 0 ].acceleration = (int)qRound( rate );
	}
    }
  
  // Do a quick test of the speed step implied by TimeState
  int tf_scan   = simparams.speed_step[ 0 ].time_first;
  int accel1    = simparams.speed_step[ 0 ].acceleration;
  QString svalu = US_Settings::debug_value( "SetSpeedLowA" );
  int lo_ss_acc = svalu.isEmpty() ? 250 : svalu.toInt();
  int rspeed    = simparams.speed_step[ 0 ].rotorspeed;
  int tf_aend   = ( rspeed + accel1 - 1 ) / ( accel1 == 0 ? 1 : accel1 );
  
  qDebug() << "SimMdl: ssck: rspeed accel1 lo_ss_acc"
	   << rspeed << accel1 << lo_ss_acc << "tf_aend tf_scan"
	   << tf_aend << tf_scan;
  //x0  1  2  3  4  5
  if ( accel1 < lo_ss_acc  ||  tf_aend > ( tf_scan - 3 ) )
    {
      QString wmsg = tr( "The TimeState computed/used is likely bad:<br/>"
			 "The acceleration implied is %1 rpm/sec.<br/>"
			 "The acceleration zone ends at %2 seconds,<br/>"
			 "with a first scan time of %3 seconds.<br/><br/>"
			 "<b>You should rerun the experiment without<br/>"
			 "any interim constant speed, and then<br/>"
			 "you should reimport the data.</b>" )
	.arg( accel1 ).arg( tf_aend ).arg( tf_scan );
      
      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Bad TimeState Implied!" ) );
      msgBox.setTextFormat( Qt::RichText );
      msgBox.setText( wmsg );
      msgBox.addButton( tr( "Continue" ), QMessageBox::RejectRole );
      QPushButton* bAbort = msgBox.addButton( tr( "Abort" ),
					      QMessageBox::YesRole    );
      msgBox.setDefaultButton( bAbort );
      msgBox.exec();
      if ( msgBox.clickedButton() == bAbort )
	{
	  QApplication::restoreOverrideCursor();
	  qApp->processEvents();
	  return;
	}
    }
  sdata->cell        = rdata->cell;
  sdata->channel     = rdata->channel;
  sdata->description = rdata->description;
  
  if ( dbg_level > 0 )
    simparams.save_simparms( US_Settings::etcDir() + "/sp_fematch.xml" );
  
  //start_time = QDateTime::currentDateTime();
  int ncomp  = model.components.size();
  //compress   = le_compress->text().toDouble();
  progress_msg->setRange( 1, ncomp );
  // progress_msg->reset();
  
  nthread    = US_Settings::threads();
  int ntc    = ( ncomp + nthread - 1 ) / nthread;
  nthread    = ( ntc > MIN_NTC ) ? nthread : 1;

  /*
  //TEST
  nthread = 10;
  */
  
  qDebug() << "SimMdl: nthread" << nthread << "ncomp" << ncomp
	   << "ntc" << ntc << "meshtype" << simparams.meshType;
  
  // Do simulation by several possible ways: 1-/Multi-thread, ASTFEM/ASTFVM
  if ( nthread < 2 )
    {
      if ( model.components[ 0 ].sigma == 0.0  &&
	   model.components[ 0 ].delta == 0.0  &&
	   model.coSedSolute           <  0.0  &&
	   compress                    == 0.0 )
	{
	  qDebug() << "SimMdl: (fematch:)Finite Element Solver is called";
	  //*DEBUG*
	  for(int ii=0; ii<model.components.size(); ii++ )
	    {
	      qDebug() << "SimMdl:   comp" << ii << "s D v"
		       << model.components[ii].s
		       << model.components[ii].D
		       << model.components[ii].vbar20 << "  c"
		       << model.components[ii].signal_concentration;
	    }
	  qDebug() << "SimMdl: (fematch:)Sim Pars--";
	  simparams.debug();
	  //*DEBUG*
	  US_Astfem_RSA* astfem_rsa = new US_Astfem_RSA( model, simparams );
	  
	  connect( astfem_rsa, SIGNAL( current_component( int ) ),
	   	   this,       SLOT  ( update_progress  ( int ) ) );
	  astfem_rsa->set_debug_flag( dbg_level );
	  solution_rec.buffer.compressibility = compress;
	  solution_rec.buffer.manual          = manual;
	  //astfem_rsa->set_buffer( solution_rec.buffer );
	  
	  astfem_rsa->calculate( *sdata );
	  //*DEBUG*
	  int kpts=0;
	  double trmsd=0.0;
	  double tnoi=0.0;
	  double rnoi=0.0;
	  bool ftin=ti_noise.count > 0;
	  bool frin=ri_noise.count > 0;
	  for(int ss=0; ss<sdata->scanCount(); ss++)
	    {
	      rnoi = frin ? ri_noise.values[ss] : 0.0;
	      for (int rr=0; rr<sdata->pointCount(); rr++)
		{
		  tnoi = ftin ? ti_noise.values[rr] : 0.0;
		  double rval=edata->value(ss,rr) - sdata->value(ss,rr) - rnoi - tnoi;
		  trmsd += sq( rval );
		  kpts++;
		}
	    }
	  trmsd = sqrt( trmsd / (float)kpts );
	  qDebug() << "SimMdl: (1)trmsd" << trmsd;
	  //*DEBUG*
	}
      else
	{
	  qDebug() << "SimMdl: (fematch:)Finite Volume Solver is called";
	  US_LammAstfvm *astfvm     = new US_LammAstfvm( model, simparams );
	  astfvm->calculate(     *sdata );
	}
      //-----------------------
      //Simulation part is over
      //-----------------------
      
      show_results();
    }
  
  else
    {  // Do multi-thread calculations

      qDebug() << "Simulate Model: Multi-thread -- ";
      
      solution_rec.buffer.compressibility = compress;
      solution_rec.buffer.manual          = manual;
      tsimdats.clear();
      tmodels .clear();
      kcomps  .clear();
      QList< ThreadWorker* >         tworkers;
      QList< QThreadEx* >            wthreads;
      
      // Build models for each thread
      for ( int ii = 0; ii < ncomp; ii++ )
	{
	  if ( ii < nthread )
	    {  // First time through per thread:  get initial model and sim data
	      tmodels << model;
	      tmodels[ ii ].components.clear();
	      US_DataIO::RawData sdat = *sdata;
	      tsimdats << sdat;
	      kcomps   << 0;
	    }
	  
	  // Partition thread models from round-robin fetch of components
	  int jj = ii % nthread;
	  tmodels[ jj ].components << model.components[ ii ];
	}
      
      thrdone   = 0;
      solution_rec.buffer.manual = manual;
      
      // Build worker threads and begin running
      for ( int ii = 0; ii < nthread; ii++ )
	{
	  ThreadWorker* tworker = new ThreadWorker( tmodels[ ii ], simparams,
						    tsimdats[ ii ], solution_rec.buffer, ii );
	  QThreadEx*    wthread = new QThreadEx();
	  
	  tworker->moveToThread( wthread );
	  
	  tworkers << tworker;
	  wthreads << wthread;
	  
	  connect( wthread, SIGNAL( started()         ),
		   tworker, SLOT  ( calc_simulation() ) );
	  
	  connect( tworker, SIGNAL( work_progress  ( int, int ) ),
	   	   this,    SLOT(   thread_progress( int, int ) ) );
	  connect( tworker, SIGNAL( work_complete  ( int )      ),
		   this,    SLOT(   thread_complete( int )      ) );
	  
	  wthread->start();
	}
    }
 
}

// Update progress bar as each component is completed
void US_Analysis_auto::update_progress( int icomp )
{
  qDebug () << "Updating progress single thread, icomp  -- " << icomp;
  
  progress_msg->setValue( icomp );
}


// Show simulation and residual when the simulation is complete
void US_Analysis_auto::show_results( )
{
   progress_msg->setValue( progress_msg->maximum() );
  
   haveSim     = true;
   // pb_distrib->setEnabled( true );
   // pb_view   ->setEnabled( true );
   // pb_save   ->setEnabled( true );
   // pb_plot3d ->setEnabled( true );
   // pb_plotres->setEnabled( true );

   calc_residuals();             // calculate residuals

   //distrib_plot_resids();        // plot residuals

   //data_plot();                  // re-plot data+simulation


   
   // plot3d();


   // //ResBitMap
   // if ( rbmapd != 0 )
   //   {
   //     // bmd_pos  = rbmapd->pos();
   //     rbmapd->close();
   //   }
   
   // rbmapd = new US_ResidsBitmap( resids );
   // //rbmapd->move( bmd_pos );
   // rbmapd->show();
   // rbmapd->raise();
   // //rbmapd->activateWindow();
   // //////////////////////////////////////////

   
   plotres();
   QApplication::restoreOverrideCursor();
}

// Calculate residual absorbance values (data - sim - noise)
void US_Analysis_auto::calc_residuals()
{
   int     dsize  = edata->pointCount();
   int     ssize  = sdata->pointCount();
   QVector< double > vecxx( dsize );
   QVector< double > vecsx( ssize );
   QVector< double > vecsy( ssize );
   double* xx     = vecxx.data();
   double* sx     = vecsx.data();
   double* sy     = vecsy.data();
   double  yval;
   double  sval;
   double  rmsd   = 0.0;
   double  tnoi   = 0.0;
   double  rnoi   = 0.0;
   bool    ftin   = ti_noise.count > 0;
   bool    frin   = ri_noise.count > 0;
   bool    matchd = ( dsize == ssize );
   int     kpts   = 0;
   qDebug() << "CALC_RESID: matchd" << matchd << "dsize ssize" << dsize << ssize;
   int kexcls=0;
 
   QVector< double > resscan;

   resids .clear();
   resscan.resize( dsize );

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xx[ jj ] = edata->radius( jj );
   }

   for ( int jj = 0; jj < ssize; jj++ )
   {
      sx[ jj ] = sdata->radius( jj );
      if ( sx[ jj ] != xx[ jj ] )  matchd = false;
   }

   for ( int ii = 0; ii < scanCount; ii++ )
   {
      bool usescan = !excludedScans.contains( ii );
if(!usescan) kexcls++;

      rnoi     = frin ? ri_noise.values[ ii ] : 0.0;

      for ( int jj = 0; jj < ssize; jj++ )
      {
         sy[ jj ] = sdata->value( ii, jj );
      }

      for ( int jj = 0; jj < dsize; jj++ )
      { // Calculate the residuals and the RMSD
         tnoi          = ftin ? ti_noise.values[ jj ] : 0.0;

         if ( matchd )
            sval          = sy[ jj ];
         else
            sval          = interp_sval( xx[ jj ], sx, sy, ssize );

         yval          = edata->value( ii, jj ) - sval - rnoi - tnoi;

         if ( usescan )
         {
            rmsd         += sq( yval );
            kpts++;
         }

         resscan[ jj ] = yval;
      }

      resids << resscan;
   }

   rmsd  /= (double)( kpts );
   //le_variance->setText( QString::number( rmsd ) );
   rmsd   = sqrt( rmsd );
   //le_rmsd    ->setText( QString::number( rmsd ) );
   qDebug() << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;
}

// Interpolate an sdata y (readings) value for a given x (radius)
double US_Analysis_auto::interp_sval( double xv, double* sx, double* sy, int ssize )
{
   for ( int jj = 1; jj < ssize; jj++ )
   {
      if ( xv == sx[ jj ] )
      {
         return sy[ jj ];
      }

      if ( xv < sx[ jj ] )
      {  // given x lower than array x: interpolate between point and previous
         int    ii = jj - 1;
         double dx = sx[ jj ] - sx[ ii ];
         double dy = sy[ jj ] - sy[ ii ];
         return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
      }
   }

   // given x position not found:  interpolate using last two points
   int    jj = ssize - 1;
   int    ii = jj - 1;
   double dx = sx[ jj ] - sx[ ii ];
   double dy = sy[ jj ] - sy[ ii ];
   return ( sy[ ii ] + ( xv - sx[ ii ] ) * dy / dx );
}

// Open a residual plot dialog
void US_Analysis_auto::plotres( )
{
   // if ( resplotd != 0 )
   // {
   //    rpd_pos  = resplotd->pos();
   //    resplotd->close();
   // }

   //resplotd = new US_ResidPlotFem( this, true );
   resplotd = new US_ResidPlotFem( this, QString("ANALYSIS") );

   //resplotd->move( rpd_pos );
   //resplotd->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
   resplotd->setWindowFlags( Qt::Dialog );
   resplotd->setWindowModality(Qt::ApplicationModal);
   resplotd->show();
   
   connect( resplotd, SIGNAL( on_close() ), this, SLOT( resplot_done() ) );
}


// Public slot to mark residuals plot dialog closed
void US_Analysis_auto::resplot_done()
{
  qDebug() << "RESPLOT being closed -- ";
  resplotd   = 0;

  //Restart timer (if not Active):
  if ( !timer_update -> isActive() && !all_processed ) 
    {
      connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
      timer_update->start(5000);
      
      qDebug() << "Timer restarted after RESPLOT closed -- ";
    }
}


// Update progress when thread reports
void US_Analysis_auto::thread_progress( int thr, int icomp )
{
  qDebug() <<  "Updating progress multiple threads, thr, icomp -- " << thr << icomp;
   int kcomp     = 0;
   kcomps[ thr ] = icomp;
   for ( int ii = 0; ii < nthread; ii++ )
      kcomp += kcomps[ ii ];
   progress_msg->setValue( kcomp );
   qDebug() << "THR PROGR thr icomp" << thr << icomp << "kcomp" << kcomp;
}


// Update count of threads completed and colate simulations when all are done
void US_Analysis_auto::thread_complete( int thr )
{
   thrdone++;
   qDebug() << "THR COMPL thr" << thr << "thrdone" << thrdone;

   if ( thrdone >= nthread )
   {  // All threads are done, so sum thread simulation data
      for ( int ii = 0; ii < sdata->scanData.size(); ii++ )
      {
         for ( int jj = 0; jj < sdata->xvalues.size(); jj++ )
         {
            //double conc = 0.0;
            double conc = sdata->value( ii, jj );

            for ( int kk = 0; kk < nthread; kk++ )
               conc += tsimdats[ kk ].value( ii, jj );

            sdata->setValue( ii, jj, conc );
         }
      }

      // Then show the results
      show_results();
   }
}

// Adjust model components based on buffer, vbar, and temperature
void US_Analysis_auto::adjustModel( )
{
   model              = model_used;

   // build model component correction factors
   double avgTemp     = edata->average_temperature();
   double vbar20      = svbar_global.toDouble();

   solution.density   = density;
   solution.viscosity = viscosity;
   solution.manual    = manual;
   solution.vbar20    = vbar20;
   solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
//   solution.vbar      = US_Math2::adjust_vbar20( solution.vbar20, avgTemp );

   US_Math2::data_correction( avgTemp, solution );

   double scorrec  = solution.s20w_correction;
   double dcorrec  = solution.D20w_correction;
   double mc_vbar  = model.components[ 0 ].vbar20;
   // Set constant-vbar and constant-ff0 flags
   cnstvb          = model.constant_vbar();
   cnstff          = model.constant_ff0();

   US_Math2::SolutionData sd;
   sd.density      = solution.density;
   sd.viscosity    = solution.viscosity;
   sd.vbar20       = solution.vbar20;
   sd.vbar         = solution.vbar;
   sd.manual       = solution.manual;
   qDebug() << "Fem:Adj:  avgT" << avgTemp << "scorr dcorr" << scorrec << dcorrec;

   if ( cnstvb  &&  mc_vbar != sd.vbar20  &&  mc_vbar != 0.0 )
   {  // Use vbar from the model component, instead of from the solution
      sd.vbar20    = mc_vbar;
      sd.vbar      = US_Math2::adjust_vbar20( sd.vbar20, avgTemp );
      US_Math2::data_correction( avgTemp, sd );
      scorrec      = sd.s20w_correction;
      dcorrec      = sd.D20w_correction;
      qDebug() << "Fem:Adj:   cnstvb" << cnstvb << "  scorr dcorr" << scorrec << dcorrec;
   }
   qDebug() << "Fem:Adj:  avgT" << avgTemp << "vb20 vb" << sd.vbar20 << sd.vbar;

   // Convert to experiment space: adjust s,D based on solution,temperature

   for ( int jj = 0; jj < model.components.size(); jj++ )
   {
      US_Model::SimulationComponent* sc = &model.components[ jj ];

      sc->vbar20  = ( sc->vbar20 == 0.0 ) ? vbar20 : sc->vbar20;

      if ( ! cnstvb )
      {  // Set s,D corrections based on component vbar
         sd.vbar20   = sc->vbar20;
         sd.vbar     = US_Math2::adjust_vbar20( sd.vbar20, avgTemp );
         US_Math2::data_correction( avgTemp, sd );
         scorrec     = sd.s20w_correction;
         dcorrec     = sd.D20w_correction;
      }

double s20w=sc->s;
double d20w=sc->D;
      sc->s      /= scorrec;
      sc->D      /= dcorrec;
      qDebug() << "Fem:Adj:  s20w D20w" << s20w << d20w
	       << "s D" << sc->s << sc->D << "  jj" << jj << "vb20 vb" << sc->vbar20 << sd.vbar;

      if ( sc->extinction > 0.0 )
         sc->molar_concentration = sc->signal_concentration / sc->extinction;
   }

}



//Slot to show overlay plot
void US_Analysis_auto::show_overlay( QString triple_stage )
{
  /** Stop update timer for now? ***/
  if ( timer_update -> isActive() ) 
    {
      timer_update -> stop();
      disconnect(timer_update, SIGNAL(timeout()), 0, 0);
    
      qDebug() << "Update stopped at View Fit for triple -- " << triple_stage;
    
      in_gui_update  = false;
    }
  /********************************/

  // Show msg while data downloaded and simulated
  progress_msg = new QProgressDialog ("Downloading data and models...", QString(), 0, 5, this);
  progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setWindowModality(Qt::WindowModal);
  progress_msg->setWindowTitle(tr("Overlay Plot Generation"));
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  
  // msg_sim = new QMessageBox(this);
  // msg_sim->setIcon(QMessageBox::Information);
  
  // msg_sim->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
  // //msg_sim->setWindowModality(Qt::ApplicationModal);
  // msg_sim->setStandardButtons(0);
  // msg_sim->setWindowTitle(tr("Overlay Generation..."));
  // msg_sim->setText( QString( tr( "Downloading data and model and simulating... \nPlease wait...") ) );
  // msg_sim->setStyleSheet("background-color: #36454f; color : #D3D9DF;");

  // qDebug() << "Msg POSITION: -- " << sim_msg_pos_x << sim_msg_pos_y;
  
  // msg_sim->move( sim_msg_pos_x, sim_msg_pos_y );
  
  // msg_sim->show();
  qApp->processEvents();
  /******************************************************/
  
  speed_steps  .clear();
  edata = NULL;
  rdata = NULL;
  //sdata = NULL;
  eID_global = 0;
  sdata          = &wsdata;
  
  dbg_level  = US_Settings::us_debug();

  adv_vals[ "simpoints"  ] = "200";
  adv_vals[ "bandvolume" ] = "0.015";
  adv_vals[ "parameter"  ] = "0";
  adv_vals[ "modelnbr"   ] = "0";
  adv_vals[ "meshtype"   ] = "ASTFEM";
  adv_vals[ "gridtype"   ] = "Moving";
  adv_vals[ "modelsim"   ] = "mean";

  resids.clear();
  dataLoaded = false;
  buffLoaded = false;
  haveSim    = false;
  resplotd   = 0;
  ti_noise.count = 0;
  ri_noise.count = 0;
  
  QString tr_st = triple_stage.simplified();
  tr_st.replace( " ", "" );
    
  QStringList triple_stage_parts = tr_st.split("(");
  QString stage_n = triple_stage_parts[0];
  QString triple_n  = triple_stage_parts[1];
  triple_n.chop(1);
  triple_n.replace("/",".");

  qDebug() << "In SHOW OVERLAY: triple_stage / triple_name: " << stage_n << " / " << triple_n;

  tripleInfo = ": " + triple_n + " (" + stage_n + ")";

  //Parse filename
  FileName_parsed = get_filename( triple_n );
  qDebug() << "In show_overlay(): FileName_parsed: " << FileName_parsed;
  
  //LoadData
  QMap< QString, QString > triple_info_map;
  triple_info_map[ "triple_name" ]     = triple_n;
  triple_info_map[ "stage_name" ]      = stage_n;
  triple_info_map[ "invID" ]           = QString::number(invID);
  triple_info_map[ "filename" ]        = FileName_parsed;

  dataLoaded = false;
  buffLoaded = false;
  haveSim    = false;
  
  loadData( triple_info_map );
  progress_msg->setValue( 1 );
  
  triple_info_map[ "eID" ]        = QString::number( eID_global );
  // Assign edata && rdata
  edata     = &editedData[ 0 ];
  rdata     = &rawData[ 0 ];

  
  // Get speed steps from DB experiment (and maybe timestate)
  QString tmst_fpath = US_Settings::resultDir() + "/" + FileName_parsed + "/"
    + FileName_parsed + ".time_state.tmst";

  US_Passwd   pw;
  US_DB2*     dbP    = new US_DB2( pw.getPasswd() );
  QStringList query;
  QString     expID;
  int         idExp  = 0;
  query << "get_experiment_info_by_runID"
	<< FileName_parsed
	<< QString::number(invID);
  dbP->query( query );
  
  if ( dbP->lastErrno() == US_DB2::OK )
    {
      dbP->next();
      idExp              = dbP->value( 1 ).toInt();
      US_SimulationParameters::speedstepsFromDB( dbP, idExp, speed_steps );
    }
  
  // Check out whether we need to read TimeState from the DB
  bool newfile       = US_TimeState::dbSyncToLF( dbP, tmst_fpath, idExp );

  //Get speed info
  QFileInfo check_file( tmst_fpath );
  if ( check_file.exists()  &&  check_file.isFile() )
    {  // Get speed_steps from an existing timestate file
      simparams.simSpeedsFromTimeState( tmst_fpath );
      simparams.speedstepsFromSSprof();

      //*DEBUG*
      int essknt=speed_steps.count();
      int tssknt=simparams.speed_step.count();
      qDebug() << "LD: (e)ss knt" << essknt << "(t)ss knt" << tssknt;
      for ( int jj = 0; jj < qMin( essknt, tssknt ); jj++ )
	{
	  qDebug() << "LD:  jj" << jj << "(e) tf tl wf wl scns"
		   << speed_steps[jj].time_first
		   << speed_steps[jj].time_last
		   << speed_steps[jj].w2t_first
		   << speed_steps[jj].w2t_last
		   << speed_steps[jj].scans;
	  qDebug() << "LD:    (t) tf tl wf wl scns"
		   << simparams.speed_step[jj].time_first
		   << simparams.speed_step[jj].time_last
		   << simparams.speed_step[jj].w2t_first
		   << simparams.speed_step[jj].w2t_last
		   << simparams.speed_step[jj].scans;
	}
      //*DEBUG*

      int kstep      = speed_steps.count();
      int kscan      = speed_steps[ 0 ].scans;
      for ( int jj = 0; jj < simparams.speed_step.count(); jj++ )
	{
	  if ( jj < kstep )
	    {
	      kscan          = speed_steps[ jj ].scans;
	      speed_steps[ jj ] = simparams.speed_step[ jj ];
	    }
	  else
            speed_steps << simparams.speed_step[ jj ];
	  
	  speed_steps[ jj ].scans = kscan;
	  simparams.speed_step[ jj ].scans = kscan;
	  qDebug() << "LD:    (s) tf tl wf wl scns"
		   << speed_steps[jj].time_first
		   << speed_steps[jj].time_last
		   << speed_steps[jj].w2t_first
		   << speed_steps[jj].w2t_last
		   << speed_steps[jj].scans;
	}
    }
  progress_msg->setValue( 2 );
  qApp->processEvents();

  dataLoaded = true;
  haveSim    = false;
  scanCount  = edata->scanData.size();
 

  //Read Solution/Buffer
  density      = DENS_20W;
  viscosity    = VISC_20W;
  compress     = 0.0;
  manual       = false;
  QString solID;
  QString bufid;
  QString bguid;
  QString bdesc;
  QString bdens = QString::number( density );
  QString bvisc = QString::number( viscosity );
  QString bcomp = QString::number( compress );
  QString bmanu = manual ? "1" : "0";
  QString svbar = QString::number( 0.7200 );
  bool    bufvl = false;
  QString errmsg;
  qDebug() << "Fem:Upd: (0)svbar" << svbar;
  
  bufvl = US_SolutionVals::values( dbP, edata, solID, svbar, bdens,
				   bvisc, bcomp, bmanu, errmsg );
  progress_msg->setValue( 3 );

  //Hardwire compressibility to zero, for now
  bcomp="0.0";
  if ( bufvl )
    {
      buffLoaded  = false;
      buffLoaded  = true;
      density     = bdens.toDouble();
      viscosity   = bvisc.toDouble();
      compress    = bcomp.toDouble();
      manual      = ( !bmanu.isEmpty()  &&  bmanu == "1" );
      if ( solID.isEmpty() )
	{
	  QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
				tr( "Empty solution ID value!" ) );
	}
      
      else if ( solID.length() < 36  &&  dbP != NULL )
	{
	  solution_rec.readFromDB( solID.toInt(), dbP );
	}
      
      else
	{
	  solution_rec.readFromDisk( solID );
	}
      
      vbar          = solution_rec.commonVbar20;
      svbar         = QString::number( vbar );
      
    }
  else
    {
      QMessageBox::warning( this, tr( "Solution/Buffer Fetch" ),
			    errmsg );
      solution_rec.commonVbar20 = vbar;
    }
  
  ti_noise.count = 0;
  ri_noise.count = 0;

  // Calculate basic parameters for other functions [ from us_fematch's ::update()-> data_plot() ]
  double avgTemp     = edata->average_temperature();
  solution.density   = density;
  solution.viscosity = viscosity;
  solution.manual    = manual;
  solution.vbar20    = svbar.toDouble();
  svbar_global       = svbar; 
  solution.vbar      = US_Math2::calcCommonVbar( solution_rec, avgTemp );
  
  US_Math2::data_correction( avgTemp, solution );
  

  //Load Model (latest ) && noise(s)
  loadModel( triple_info_map  );
  progress_msg->setValue( 5 );
  qApp->processEvents();

  //Simulate Model
  simulateModel();

  
  qDebug() << "Closing sim_msg-- ";
  //msg_sim->accept();
  progress_msg->close();

  /*
  // Show plot
  resplotd = new US_ResidPlotFem( this, true );
  //resplotd->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  resplotd->setWindowModality(Qt::ApplicationModal);
  resplotd->show();
  */
}


//Cancel all jobs if FITMEN for a channel was processed by other means: NO FM modles
void US_Analysis_auto::delete_jobs_at_fitmen( QMap < QString, QString > & triple_info )
{

  qDebug() << "At delete_jobs_at_fitmen: triple_name: " << triple_info[ "triple_name_key" ];

  QString triple_name   = triple_info[ "triple_name_key" ];
  QString triple_n_copy = triple_info[ "triple_name_key" ];
  QStringList triple_n_parts = triple_n_copy.split(".");
  QString channel_n = triple_n_parts[0] + "." + triple_n_parts[1];

  QStringList requestID_list = Channel_to_requestIDs[ channel_n ];
  qDebug() << "In delete_at_fitmen other WVLs: channel info -- " <<  requestID_list;

  QMap <QString, QString > ana_details = Array_of_analysis[ triple_name ];
  QString requestID = ana_details["requestID"];
  qDebug() << "RequestID -- " << requestID;

  bool mwl_channel = false;
  QStringList triple_list_affected;
  
  if ( requestID_list.size() > 1 ) 
    {
      mwl_channel = true;
      //remove currently selected requestID -- for primary wvl
      requestID_list.removeOne( requestID );
      qDebug() << "In delete_at_fitmen other WVLs: MODIFIED channel info -- " <<  requestID_list;
      
      for ( int i=0; i<requestID_list.size(); ++i )
	{
	  //extract triple information for the rest of wvl
	  QString requestID_affected = requestID_list[i];
	  QMap< QString, QString > ana_details_affected = Array_of_analysis_by_requestID[ requestID_affected ];
	  triple_list_affected << ana_details_affected[ "triple_name" ];
	}
    }

  if ( !fitmen_bad_vals )
    {
      QMessageBox msg_delete;
      msg_delete.setIcon(QMessageBox::Critical);
      msg_delete.setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      msg_delete.setWindowTitle(tr("Job Cancelation!"));
      
      QString msg_sys_text = QString( tr( "ATTENTION!\n\nNo -FM models for triple %1 have been found. \n\n"
					  "This means one of the following: \n"
					  "1. No -FM models have been generated; \n"
					  "2. The data have been analyzed outside of the GMP framework.\n\n"
					  "All scheduled jobs will be canceled for this triple." ) )
				      .arg( triple_name );
            
      //if ( mwl_channel && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) )
      if ( mwl_channel ) 
	msg_sys_text += QString("\n\nSince this is a multi-wavelength analysis, scheduled jobs for the following same-channel triples will be canceled as well: \n\n%1")
	  .arg( triple_list_affected.join(", ") );
      
      msg_delete.setText( msg_sys_text );
      
      //QString msg_sys_text_info = QString("Do you want to proceed?");
      //msg_delete.setInformativeText( msg_sys_text_info );
      
      QPushButton *Accept_sys    = msg_delete.addButton(tr("OK"),    QMessageBox::YesRole);
      //QPushButton *Cancel_sys    = msg_delete.addButton(tr("Cancel"), QMessageBox::RejectRole);
      
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
	  
	  
	  /** DEBUG **/
	  // QMap <QString, QString > current_analysis;
	  // current_analysis = read_autoflowAnalysis_record( &db, requestID );
	  
	  // qDebug() << "GUID to DETETE! -- " << current_analysis[ "CurrentGfacID" ];
	  // qDebug() << "Status && statusMsg to DETETE! -- " << current_analysis[ "status" ] << current_analysis[ "status_msg" ];
	  /* **********/
	  
	  update_autoflowAnalysis_uponDeletion( &db, requestID );
	  update_autoflowAnalysis_uponDeletion_other_wvl( &db, requestID_list );
	  
	}
    }
  else
    {
      qDebug() << "DELETION DUE TO BAD Meniscus|Bottom values chosen !!";
      qDebug() << "Reason for deletion -- " << triple_info[ "failed" ];

      QMessageBox msg_delete_bad_vals;
      msg_delete_bad_vals.setIcon(QMessageBox::Critical);
      msg_delete_bad_vals.setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      msg_delete_bad_vals.setWindowTitle(tr("Job Cancelation!"));
      
      QString msg_sys_text_bv = QString( tr("ATTENTION!\n\n The meniscus fit for tiple %1 resulted in a best-fit value that lies outside "
					    "of the considered range for the meniscus. "
					    "This should not happen, unless: \n\n"
					    "1. The initial meniscus was not selected correctly; \n"
					    "2. The experimental data are non-ideal and cannot be modeled with the standard finite element model;\n"
					    "3. The radial calibration of the instrument is incorrect. \n\n"
					    "The user should examine the data manually and determine the cause for this discrepancy."
					    "The automatic GMP processing for this triple cannot continue.\n"
					    "All scheduled jobs will be canceled for this triple."
					    ) ).arg( triple_name );
	      
      //if ( mwl_channel && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) )
      if ( mwl_channel ) 
	msg_sys_text_bv += QString("\n\nSince this is a multi-wavelength analysis, scheduled jobs for the following same-channel triples will be canceled as well: \n\n%1")
	  .arg( triple_list_affected.join(", ") );
      
      msg_delete_bad_vals.setText( msg_sys_text_bv );
      
      //QString msg_sys_text_info = QString("Do you want to proceed?");
      //msg_delete.setInformativeText( msg_sys_text_info );
      
      QPushButton *Accept_sys    = msg_delete_bad_vals.addButton(tr("OK"),    QMessageBox::YesRole);
      //QPushButton *Cancel_sys    = msg_delete.addButton(tr("Cancel"), QMessageBox::RejectRole);
      
      msg_delete_bad_vals.exec();
      
      if (msg_delete_bad_vals.clickedButton() == Accept_sys)
	{
	  US_Passwd pw;
	  US_DB2    db( pw.getPasswd() );
	  
	  // Get the buffer data from the database
	  if ( db.lastErrno() != US_DB2::OK )
	    {
	      QMessageBox::warning( this, tr( "Connection Problem" ),
				    tr( "Could not connect to database \n" ) +  db.lastError() );
	      return;
	    }
	  
	  update_autoflowAnalysis_uponDeletion( &db, requestID );
	  update_autoflowAnalysis_uponDeletion_other_wvl( &db, requestID_list );
	}
    }
      
  //Restart timer:
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  timer_update->start(5000);

  qDebug() << "Timer restarted after Canceling jobs at FITMEN (processed by other means) for channel -- " << channel_n;
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

  //if ( mwl_channel && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) )
  if ( Process_2dsafm [ triple_n ] && mwl_channel && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) ) // don't we have to do this ONLY fro representative triple??
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

      //Dialog for a Comment, why triple(s) analyses are set for deletion:
      bool ok;
      QString msg = QString(tr("Put a comment describing reason for a STOP:"));
      QString default_text = QString(tr("Reason for job CANCELLATION: "));
      QString comment_t    = QInputDialog::getText( this,
						    tr( "Reason for job CANCELLATION" ),
						    msg, QLineEdit::Normal, default_text, &ok );

      if ( !ok )
	{
	  return;
	}
      ////////////////////////////////////////////////////////////////////

      US_Passwd pw;
      US_DB2    db( pw.getPasswd() );
      
      // Get the buffer data from the database
      if ( db.lastErrno() != US_DB2::OK )
	{
	  QMessageBox::warning( this, tr( "Connection Problem" ),
				tr( "Could not connect to database \n" ) +  db.lastError() );
	  return;
	}

      /** DEBUG **/
      // QMap <QString, QString > current_analysis;
      // current_analysis = read_autoflowAnalysis_record( &db, requestID );
      
      // qDebug() << "GUID to DETETE! -- " << current_analysis[ "CurrentGfacID" ];
      // qDebug() << "Status && statusMsg to DETETE! -- " << current_analysis[ "status" ] << current_analysis[ "status_msg" ];
      /* **********/

      update_autoflowAnalysis_uponDeletion( &db, requestID );

      // Now, if 2DSA or 2DSA-FM of the representative wvl in the MWL case: DELETE jobs for all other triples in a channel:
      QString other_chan_triples;
      if ( Process_2dsafm [ triple_n ] && ( stage_n == "2DSA" || stage_n == "2DSA-FM" ) && mwl_channel )
	{
	  update_autoflowAnalysis_uponDeletion_other_wvl( &db, requestID_list );

	  if ( !triple_list_affected. isEmpty() )
	    other_chan_triples = triple_list_affected.join(",");
	}
      
      //Also update autoflowStatus's "analysisCancel" JSON:
      record_or_update_analysis_cancel_status( &db, triple_n, other_chan_triples, comment_t );
           
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
      << QString("Job scheduled for deletion, analysis stage")
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

  
  //Try for "FM_changed" || "FB_changed" keys in triple_info:
  QString FMB_changed = triple_info. contains( "FMB_changed" ) ? triple_info[ "FMB_changed" ] : QString("");
  
  qDebug() << "In update_statuses: FMB_changed -- " << FMB_changed;

  /* HOW to add && APPEND JSON in the DB ********

  1. If field is empty, first add initial record:
       UPDATE autoflowStatus set analysis='[{"2 / A / 260": "changed"}]' where ID=22;

  2. If not empty (then and after)
       UPDATE autoflowStatus set analysis=JSON_ARRAY_APPEND(analysis, '$', JSON_OBJECT('2 / B / 260', 'NO')) where ID=22;
  **********************************************/
 
  //Update autoflowStatus's 'analysis' field: per-channel (reference triple) basis, by whom:
  US_Passwd pw;
  US_DB2* dbP = new US_DB2( pw.getPasswd() );
    
  record_or_update_analysis_meniscus_status( dbP, triple_curr_key, FMB_changed );
  
  
  
  // Now update autoflowAnalysis records:
  QStringList requestID_list; 

  if ( triple_curr_key.contains("Interference") )
    requestID_list << requestID;
  else
    requestID_list = Channel_to_requestIDs[ channel_name ];

  qDebug() << "Channel_name, requestIDs to update: " << channel_name << requestID_list;
  
  

  // --- Before updating autoflowAnalysis records ---
  // --- MAKE sure that ALL original 2DSA stages for other wavelength in a channel (besides representative wvl) are completed
  // --- i.e. are in WAIT status
  update_autoflowAnalysis_status_at_fitmen( dbP, requestID_list );

  //Restart timer:
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  timer_update->start(5000);

  qDebug() << "Timer restarted after updating EditProfiles for channel -- " << channel_name;
}


//Keep track on meniscus (or bottom) deviations in FITMEN from the best fit:
void US_Analysis_auto::record_or_update_analysis_cancel_status( US_DB2* db, QString triple_name, QString other_chan_triples, QString comment_t )
{
  // Check DB connection
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "Updating autoflowStatus's analysis at FITMEN: Could not connect to database \n" ) + db->lastError() );
      return;
    }
  
  QStringList qry;

  //get user info
  qry.clear();
  qry <<  QString( "get_user_info" );
  db->query( qry );
  db->next();

  int ID        = db->value( 0 ).toInt();
  QString fname = db->value( 1 ).toString();
  QString lname = db->value( 2 ).toString();
  QString email = db->value( 4 ).toString();
  int     level = db->value( 5 ).toInt();

  qry.clear();

  QString CancelTriples = triple_name;
  if ( !other_chan_triples. isEmpty() )
    CancelTriples += "," + other_chan_triples;
  
  QString CancelAction = "CANCELED, by " + fname + ", " + lname + "; COMMENT, " + comment_t;

  if ( autoflowStatusID )
    {
      //update OR insert NEW Json
      qry << "update_autoflowStatusAnalysisCancel_record"
	  << QString::number( autoflowStatusID )
	  << QString::number( autoflowID_passed )
	  << CancelTriples
	  << CancelAction;
	  
      db->query( qry );
    }
  else
    {
      QMessageBox::warning( this, tr( "AutoflowStatus Record Problem" ),
			    tr( "autoflowStatus (Analysis {CANCELED triples}): "
				"There was a problem with identifying a record in autoflowStatus table for a given run! \n" ) );
      
      return;
    }
  
}


//Keep track on meniscus (or bottom) deviations in FITMEN from the best fit:
void US_Analysis_auto::record_or_update_analysis_meniscus_status( US_DB2* db, QString triple_name, QString FMB_changed )
{
  if ( FMB_changed.isEmpty() )
    {
      qDebug() << "FBM_changed EMPTY: It seems it's impossible to determine if meniscus| bottom was modified from the best FIT...";
      return;
    }
    
  // Check DB connection
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "Updating autoflowStatus's analysis at FITMEN: Could not connect to database \n" ) + db->lastError() );
      return;
     }
  
  QStringList qry;

  //get user info
  qry.clear();
  qry <<  QString( "get_user_info" );
  db->query( qry );
  db->next();

  int ID        = db->value( 0 ).toInt();
  QString fname = db->value( 1 ).toString();
  QString lname = db->value( 2 ).toString();
  QString email = db->value( 4 ).toString();
  int     level = db->value( 5 ).toInt();

  qDebug() << "IN ANALYSIS, record autoflowStatus: ID,name,email,lev" << ID << fname << lname << email << level;
  
  //Record to autoflowStatus:
  qry.clear();
  
  QString analysisJson;   //  [{"6 / B / 260": "fitted"}]','6 / B / 260', 'fitted'
   
  QString analysisTriple = triple_name;
  analysisTriple.replace( ".", " / ");

  QString analysisAction = (FMB_changed == "YES") ? "modified" : "best fit selected";
  analysisAction += ", by " + fname + ", " + lname;

  // analysisJson += "[{";
  // analysisJson += "\"" + analysisTriple + "\":\"" + analysisAction + "\"";  
  // analysisJson += "}]";
   
  if ( autoflowStatusID )
    {
      //update OR insert NEW Json
      qry << "update_autoflowStatusAnalysisFitmen_record"
	  << QString::number( autoflowStatusID )
	  << QString::number( autoflowID_passed )
	// << analysisJson                            //ALEXEY: not needed with new proc.
	  << analysisTriple
	  << analysisAction;
	  
      db->query( qry );
    }
  else
    {
      QMessageBox::warning( this, tr( "AutoflowStatus Record Problem" ),
			    tr( "autoflowStatus (Analysis {FMB}): There was a problem with identifying a record in autoflowStatus table for a given run! \n" ) );
      
      return;
    }

  return;
}


// slot to update autoflowAnalysis record at fitmen stage WHEN already treated from different session
void US_Analysis_auto::editProfiles_updated_earlier ( void )
{
  //Restart timer:
  connect(timer_update, SIGNAL(timeout()), this, SLOT( gui_update ( ) ));
  timer_update->start(5000);

  qDebug() << "Timer restarted: editProfiles were updated EARLIER -- ";
}

// slot to update autoflowAnalysis record at fitmen stage WHEN already treated from different session
void US_Analysis_auto::triple_analysis_processed ( void )
{
  qDebug() << "FITMEN: entire analysis for triple completed -- ";

  emit analysis_back_to_initAutoflow( );
}

//reset Analysis GUI: stopping all update processes
void US_Analysis_auto::reset_analysis_panel_public( )
{
  reset_analysis_panel( );
}

// Check FITMEN | FITMEN_AUTO status before calling FITMEN constructor OR processing automatically
bool US_Analysis_auto::check_fitmen_status( const QString& requestID, const QString& stageName )
{
  bool status = false;
  
  US_Passwd pw;
  US_DB2    dbP( pw.getPasswd() );

  // Get the buffer data from the database
  if ( dbP.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  dbP.lastError() );
      return status;
    }

  QMap <QString, QString> analysis_details;
  
  analysis_details = read_autoflowAnalysis_record(  &dbP, requestID );

  qDebug() << "In check_fitmen 1: -- " << requestID << stageName;
  
  if ( !analysis_details.size() )
    {
      //no record, so analysis completed/cancelled and already in the autoflowAnalysisHistory
      
      QMessageBox::information( this,
				QString( tr( "%1 | Triple Analysis already processed" ) ).arg( stageName ),
				QString( tr( "It appears that %1 stage has already been processed by "
					     "a different user from different session and "
					     "the entire analysis for the current triple is completed. \n\n"
					     "The program will return to the autoflow runs dialog where "
					     "you can re-attach to the actual current stage of the run. ")).arg( stageName ) );
      
      status = true;
    }
  else
    {
      //there is an autoflowAnalysis record:
      
      QString status_gen     = analysis_details["status"];
      QString nextWaitStatus = analysis_details[ "nextWaitStatus" ] ;
      QString status_json    = analysis_details["status_json"];

      qDebug() << "In check_fitmen 2: -- "
	       << status_gen     << "\n"
	       << nextWaitStatus << "\n"
	       << status_json ;
        
      QJsonDocument jsonDoc = QJsonDocument::fromJson( status_json.toUtf8() );
      if (!jsonDoc.isObject())
	{
	  qDebug() << stageName << ": NOT a JSON Doc !!";
	  status = true;
	}
      
      const QJsonValue &submitted  = jsonDoc.object().value("submitted");          
      
      //look for FITMEN | FITMEN_AUTO stage in "submitted" stages
      QString stage_name = submitted.toString();

      qDebug() << "In check_fitmen 3: -- "
	       << submitted.toString() <<  stageName;
	
      if ( submitted.toString() == stageName )
	{
	  qDebug() << "In check_fitmen 4: ";
	  
	  if ( status_gen != "WAIT" )
	    {
	      qDebug() << "In check_fitmen 5: ";
	      
	      QMessageBox::information( this,
					QString( tr( "%1 already processed" )).arg( stageName ),
					QString( tr( "It appears that %1 stage has already been processed by "
						     "a different user from different session.\n\n"
						     "The program will return to ANALYSIS tab where "
						     "you can continue to monitor the overall analysis progress." )).arg( stageName ) );
	     
	      status = true;
	    }
	}
      else
	{
	  qDebug() << "In check_fitmen 6: ";
	  //FITMEN | FITMEN_AUTO  is no in "submitted" stages anymore, so it's processed:
	  QMessageBox::information( this,
				    QString( tr( "%1 already processed" )).arg( stageName ),
				    QString( tr( "It appears that %1 stage has already been processed by "
						 "a different user from different session.\n\n"
						 "The program will return to ANALYSIS tab where "
						 "you can continue to monitor the overall analysis progress." )).arg( stageName ) );
	  
	  status = true;
	}
    }
  
  return status;
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
  QPushButton* pb_delete = new QPushButton( tr( "Cancel" ) );
  pb_delete-> setObjectName("delete");

  //Overlay button
  QPushButton* pb_overlay = new QPushButton( tr( "View Fit" ) );
  pb_overlay-> setObjectName("overlay");
  
  
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

  genL->addWidget( pb_delete,  row++,  10, 3, 1);

  genL->addWidget( lb_owner,   row,    0, 1, 2 );
  genL->addWidget( le_owner,   row++,  2, 1, 8 );  

  genL->addWidget( lb_lastmsg, row,    0, 1, 2 );
  genL->addWidget( le_lastmsg, row++,  2, 1, 8 );

  genL->addWidget( lb_status,  row,    0, 1, 2 );
  genL->addWidget( le_status,  row,    2, 1, 3 );
  genL->addWidget( lb_anatype, row,    5, 1, 2 );
  genL->addWidget( le_anatype, row++,  7, 1, 3 );

  genL->addWidget( pb_overlay,  row++,  10, 3, 1);

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


  // Disable overlay btn by default
  pb_overlay->setEnabled( false );

  signalMapper_overlay = new QSignalMapper(this);
  connect(signalMapper_overlay, SIGNAL( mapped( QString ) ), this, SLOT( show_overlay( QString ) ) );
  connect( pb_overlay, SIGNAL( clicked() ), signalMapper_overlay, SLOT(map()));
  signalMapper_overlay->setMapping ( pb_overlay, triple_name );
  
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


//functions for FITMEN_AUTO stage
void US_Analysis_auto::scan_dbase_auto( QMap <QString, QString> & triple_information )
{
   QVector< ModelDesc > mDescrs;   // Vector of model description objects
   US_Passwd pw;                   // DB password
   US_DB2 db( pw.getPasswd() );    // DB control
   QStringList query;              // DB query string list
   QStringList mfnams;             // List of FM model fit file names
   QStringList ufnams;             // List of unique model fit file names
   QStringList uantms;             // List of unique model fit analysis times
   QStringList tmodels;            // List: IDs of models with truncated descrs
   QStringList tedGIDs;            // List: edit GUIDs of models w/ trunc descrs
   QStringList tedIDs;             // List: edit IDs of models w/ trunc descrs
   QList< int > botredo;           // List: bottom-redo Mdescr vector

   int         nfmods = 0;         // Number of fit-meniscus models
   int         nfsets = 0;         // Number of fit-meniscus analysis sets
   int         nfrpls = 0;         // Number of fit file replacements
   int         nfadds = 0;         // Number of fit file additions
   int         nfexss = 0;         // Number of fit files left as they existed

   QString invID = triple_information[ "invID" ];

   QRegExp fmIter  = QRegExp( "i\?\?-[mb]*",
         Qt::CaseSensitive, QRegExp::Wildcard );

   //QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Scan the database and find fit-meniscus models

   // le_status->setText(
   //       tr( "Scanning DB fit-meniscus models ..." ) );
   
   query << "get_model_desc_auto" << invID << triple_information[ "filename" ];
   // Make use of the description && '2DSA-FM'
   //query << "get_model_desc" << invID;
   db.query( query );

   while( db.next() )
   {
      ModelDesc mdescr;
      QString modelID    = db.value( 0 ).toString();
      QString modelGUID  = db.value( 1 ).toString();
      QString descript   = db.value( 2 ).toString();
      QString editGUID   = db.value( 5 ).toString();
      QString editID     = db.value( 6 ).toString();

      if ( descript.length() == 80 )
      {  // Truncated description:  save ID and skip update for now
DbgLv(1) << "DbSc:     TRUNC: modelID" << modelID << "descr" << descript;
         tmodels << modelID;
         tedGIDs << editGUID;
         tedIDs  << editID;
         continue;
      }

      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      double  bottom     = 0.0;
      QDateTime lmtime   = db.value( 7 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
DbgLv(1) << "DbSc:   modelID vari meni" << modelID << variance << meniscus
 << "ansysID" << ansysID << "iterID" << iterID;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information

         // Format and save the potential fit table file name
         QString fitVals    = iterID  .section( '-',  1,  1 );
DbgLv(1) << "DbSc:    *FIT* " << descript << "fitVals" << fitVals;
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";
DbgLv(1) << "DbSc:     fittype" << fittype << "fextn" << fextn;
         if ( fittype == 2  ||  fittype == 3 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString anType     = ansysID .section( '_',  2, -3 );

DbgLv(1) << "DbSc:       anType" << anType << "editLabel" << editLabel << "ansysID" << ansysID;
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      } // END: model is fit type
   } // END: database model record reads
DbgLv(1) << "DbSc: tmodels size" << tmodels.size() << "ted sizes"
 << tedGIDs.size() << tedIDs.size();

   // Review models with truncated descriptions
   for ( int ii = 0; ii < tmodels.size(); ii++ )
   {
      QString modelID    = tmodels[ ii ];
      query.clear();
      query << "get_model_info" << modelID;
      db.query( query );

      if ( db.lastErrno() != US_DB2::OK )  continue;

      db.next();

      QString modelGUID  = db.value( 0 ).toString();
      QString descript1  = db.value( 1 ).toString();
      QString contents   = db.value( 2 ).toString();
      int     jdx        = contents.indexOf( "description=" );
//DbgLv(1) << "DbSc:    ii jdtx" << ii << jdtx << "modelID" << modelID
//   << "  dsc1" << descript1 << " cont" << contents.left( 20 );

      if ( jdx < 1 )  continue;

//DbgLv(1) << "DbSc:      jdx lend" << jdx << lend;
      QString descript   = contents.mid( jdx ).section( '"', 1, 1 );
      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QString editGUID   = tedGIDs[ ii ];
      QString editID     = tedIDs [ ii ];

      QDateTime lmtime   = db.value( 6 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
//DbgLv(1) << "DbSc:   dscr1" << descript1 << "dcs" << descript;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;
         ModelDesc mdescr;
         double  bottom     = 0.0;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;
         if ( fitVals.length() > 6 )
            fittype            = 3;
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;
         else if ( fitVals.startsWith( "f" ) )
            fittype            = 1;
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";

         if ( fittype != 1 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }

         QString anType     = ansysID .section( '_',  2, -3 );
	 
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      }
   }

   // Redo any model descriptions that need a bottom value
   for ( int ii = 0; ii < botredo.count(); ii++ )
   {
      int jj             = botredo[ ii ];
      ModelDesc mdescr   = mDescrs[ jj ];
      QString modelID    = mdescr.modelID;
      US_Model wmodel;
      wmodel.load( modelID, &db );
      double bottom      = wmodel.bottom;

      if ( bottom < 1.0 )
      {  // Bottom not reliable in model, get from model description
         QString descript   = mdescr.description;
         QString ansysID    = descript.section( '.', -2, -2 );
         QString iterID     = ansysID .section( '_', -1, -1 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         bottom             = ( fittype == 2 ) ?
                              ( fitVals.mid( 1, 5 ).toDouble() / 10000.0 ) :
                              ( fitVals.mid( 6, 5 ).toDouble() / 10000.0 );
      }

      mdescr.bottom      = bottom;
      mDescrs[ jj ]      = mdescr;
   }

   nfmods     = mDescrs.count();
DbgLv(1) << "Number of FM models found: " << nfmods;
if(nfmods>0) {
DbgLv(1) << " pre:D0" <<  mDescrs[0].description;
DbgLv(1) << " pre:Dn" <<  mDescrs[nfmods-1].description; }
   qSort( mDescrs );
if(nfmods>0) {
DbgLv(1) << " sorted:D0" <<  mDescrs[0].description;
DbgLv(1) << " sorted:Dn" <<  mDescrs[nfmods-1].description; }

   // Scan local files to see what fit table files already exist

   // le_status->setText(
   //       tr( "Comparing to existing local meniscus,rmsd table files ..." ) );
   mfnams.clear();
   ufnams.clear();
   uantms.clear();

   for ( int ii = 0; ii < nfmods; ii++ )
   {  // Find unique file names in order to create sets
      QString ftfname    = mDescrs[ ii ].fitfname;
      QString antime     = mDescrs[ ii ].antime;

      if ( ! ufnams.contains( ftfname ) )
      {  // This is a new fit-file name, so new analysis set
         ufnams << ftfname;
         uantms << antime;
      }

      else if ( ! uantms.contains( antime ) )
      {  // Already seen fit-file, but new analysis time, so duplicate
         uantms << antime;
      }

      mfnams << mDescrs[ ii ].fitfname;
   }

   nfsets     = ufnams.size();
   int nantm  = uantms.size();
   int ndupl  = nantm - nfsets;
DbgLv(1) << "Number of FM analysis sets: " << nfsets;
DbgLv(1) << "Number of FM analysis set duplicates: " << ndupl;
   int kfsets = nfsets;
   QString rdir = US_Settings::resultDir().replace( "\\", "/" ) + "/";
   QString fnamesv;

   for ( int ii = 0; ii < kfsets; ii++ )
   {  // Find out for each set whether a corresponding fit file exists
      QString ftfname    = ufnams.at( ii );

      if ( mfnams.count( ftfname ) == 1 )
      {  // Not really a set; single fit model after previous fm run
         nfsets--;
DbgLv(1) << "ScDB: SINGLE:" << ftfname;
         continue;
      }

      QString ftfpath    = rdir + ftfname;
      QFile   ftfile( ftfpath );

      if ( ftfile.exists() )
      {  // File exists, so we must check the need to replace it
         QString ftfpath    = rdir + ftfname;
         QDateTime fdate    = QFileInfo( ftfile ).lastModified().toUTC();
         int       jj       = mfnams.lastIndexOf( ftfname );
         QDateTime rdate    = mDescrs[ jj ].lmtime;
DbgLv(1) << " ii rdate fdate" << ii << rdate << fdate << "   ftfname"
   << ftfname << "  fdate.msecsTo(rdate)" << fdate.msecsTo(rdate);
DbgLv(1) << "   jj desc" << jj << mDescrs[jj].description
 << "antime meniscus" << mDescrs[jj].antime << mDescrs[jj].meniscus;

         if ( fdate.msecsTo( rdate ) > 0 )
         {  // DB record date is later than file date, so must replace file
            nfrpls++;
            ftfile.remove();
         }

         else
         {  // DB record date is not later than file date, so leave file as is
            nfexss++;
            continue;
         }
      }

      else
      {  // File does not exist, so we definitely need to create it
         nfadds++;
         QString ftfpath    = QString( rdir + ftfname ).section( "/", 0, -2 );
	 DbgLv(1) << "ScDB: NOT-EXIST local:  nfadds" << nfadds << "ftfpath: " << ftfpath;
         QDir().mkpath( ftfpath );
      }

      if ( ! ftfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {  // Problem!!!
         qDebug() << "*ERROR* Unable to open file" << ftfname;
         continue;
      }

      // Creating a new or replacement file:  build list of meniscus,rmsd pairs
      int       jfirst   = mfnams.indexOf( ftfname );
      int       jlast    = mfnams.lastIndexOf( ftfname ) + 1;
      QString   antiml   = mDescrs[ jlast - 1 ].antime;
      QStringList mrpairs;

DbgLv(1) << " Creating" << ftfname << "jf,jl" << jfirst << jlast;
      for ( int jj = jfirst; jj < jlast; jj++ )
      {  // First build the pairs (or triples) list
         double bottom   = mDescrs[ jj ].bottom;
         double meniscus = mDescrs[ jj ].meniscus;
         double variance = mDescrs[ jj ].variance;
         double rmsd     = sqrt( variance );
         QString antime  = mDescrs[ jj ].antime;
         QString mrpair  = QString::number( meniscus, 'f', 6 ) + " "
                         + QString::number( rmsd,     'e', 6 ); 

         if ( bottom > 1.0 )
         {  // Either Bottom or Meniscus+Bottom
            if ( ftfname.contains( "FB" ) )
            {  // Bottom only
               mrpair          = QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
            else
            {  // Meniscus and Bottom
               mrpair          = QString::number( meniscus, 'f', 6 ) + " "
                               + QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
         }
DbgLv(1) << "  jj desc" << jj << mDescrs[jj].description;

         if ( antime == antiml )
            mrpairs << mrpair;
      }

      mrpairs.sort();
      QTextStream ts( &ftfile );

      // Output the pairs to the file
      for ( int jj = 0; jj < mrpairs.size(); jj++ )
         ts << mrpairs.at( jj ) + "\n";

      ftfile.close();

      fnamesv = fnamesv.isEmpty() ? ftfname : fnamesv;
   }

DbgLv(1) << "Number of FM REPLACE  sets: " << nfrpls;
DbgLv(1) << "Number of FM ADD      sets: " << nfadds;
DbgLv(1) << "Number of FM EXISTING sets: " << nfexss;

   // Report
   QString msg = tr( "File" );
   int nftota  = nfadds + nfrpls;

   if      ( nfadds == 1  &&  nfrpls == 0 )
      msg += tr( " added: " );

   else if ( nfadds == 0  &&  nfrpls == 1 )
      msg += tr( " updated: " );

   else if ( nfadds == 0  &&  nfrpls == 0 )
      msg  = tr( "No new fit files were created." );

   else
      msg  = tr( "Last of %1 added/updated: " ).arg( nftota );

   if ( nftota > 0 )
      msg += fnamesv;

   //le_status->setText( msg );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

   qDebug() << "End of scan auto:";
  
}

void US_Analysis_auto::get_editProfile_copy( QMap < QString, QString > & triple_information  )
{
  qDebug() << "In get_edit COPY: ";
  /* ALEXEY: we can use only triple_information[ "filename" ] (the editedData table's 'label' filed) 
     to extract all editProfiles for this runID (which is triple_information[ "filename" ] )
   */
  
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  // To be extracted from editedData table ('filename' field)
  // QMap below - establish correspondence btw EditProfile filename && EditDataID
  // select filename, editedDataID from editedData where label='ISSF-KulkarniJ_NP1-pDNA-D2O-0_091020-run822-2A';
  QMap <QString, int> EProfs_to_IDs;
  
  QStringList query;
  query << "get_editedDataFilenamesIDs" << triple_information["filename"];
  db->query( query );

  qDebug() << "Query: " << query;
  
  while ( db->next() )
    {
      QString  filename  = db->value( 0 ).toString();
      int      ID        = db->value( 1 ).toInt();

      EProfs_to_IDs[ filename ] = ID;
    }
  
   QMap<QString, int>::iterator fn;
   for ( fn = EProfs_to_IDs.begin(); fn != EProfs_to_IDs.end(); ++fn )
   {
qDebug() << "EditProfile filename / EditDataID: " << fn.key() << "/" << fn.value();
      QString filename = fn.key();
      int editedDataID = fn.value();

      QString dirpath  = US_Settings::resultDir() + "/" + triple_information[ "filename" ];
      QString filepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + filename;

      // Can check here if such filename exists
      QFileInfo check_file( filepath );
      if ( check_file.exists() && check_file.isFile() )
qDebug() << "EditProfile file: " << filepath << " exists";
      else
	{
	  if ( !QDir( dirpath ).exists() )
	    QDir().mkdir( dirpath );
	    
	  db->readBlobFromDB( filepath, "download_editData", editedDataID );
	}
   }
}

bool US_Analysis_auto::file_loaded_auto( QMap < QString, QString > & triple_information  )
{
  qDebug() << "In file_loaded_auto: ";
  QString file_directory = US_Settings::resultDir() + QString("/") + triple_information[ "filename" ];
  QString triple_name_cut = triple_information[ "triple_name" ];
  triple_name_cut.simplified();
  triple_name_cut.replace("/","");
  triple_name_cut.replace(" ","");

  if ( triple_name_cut.contains("Interference") )
    triple_name_cut.replace("Interference","660");
  

  qDebug() << "In file_loaded_auto: 11: " << triple_name_cut;
  qDebug() << "Triple filename: " << triple_information[ "filename" ];
  
  QDir directory (file_directory);
  QStringList fm_files = directory.entryList( QStringList() << "2DSA-FM*" + triple_name_cut + "*.fitmen.dat", QDir::Files | QDir::NoSymLinks);

  qDebug() << "In file_loaded_auto: 22 -- triple: " << triple_name_cut;

  //ALEXEY: if there is no files (since no "-FM" models produced for what ever reason), issue a warning:
  if ( !fm_files.size()  )
    {
      QMessageBox::warning( this,
			    QString( tr( "FM models problem: %1" ) ).arg( triple_name_cut ),
			    tr( "No \"FM | FMB\" models have been found for the present run. \n\n"
				"Program will proceed with the analysis of other triples...") );

      no_fm_data_auto = true;
      return false;
    }
  
  QString fn = directory.absoluteFilePath( fm_files[0] );  //ALEXEY: *should be* the only one fitmen.dat file 

  qDebug() << "In file_loaded_auto: 33";
  
  qDebug() << "File to open: " << fn;
  
  QString text_content;
  QFile f( fn );

  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
     QTextStream t( &f );

     text_content = t.readAll();
     f.close(  );

     //te_data->e->setPlainText( text );
  }
  else
     QMessageBox::information( this,
           tr( "Error" ),
           tr( "Could not open\n\n" ) + fn + tr( "\n\n for reading." ) );
  
  
DbgLv(1) << "FL: IN:  fn" << fn;
   filedir    = fn.section( "/",  0, -2 );
   fname_load = fn.section( "/", -1, -1 );
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();

   QString runID    = filedir.section( "/", -1, -1 );
   QString anType   = fname_load.section( ".", -2, -2 );
   QString tripnode = fname_load.section( ".", -3, -3 );
   QString edtLabel = fname_load.section( ".", -4, -4 ).mid( 1 );
   QString tripl    = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 ) + "." +
                      tripnode.mid( 2 );
   QStringList edtfilt;
DbgLv(1) << "edtLabel" << edtLabel;
   if ( !edtLabel.contains( "DSA-F" ) )
   {  // fitmen file name has edit label
      edtfilt << runID + "." + edtLabel + ".*." + tripl + ".xml";
   }
   else
   {  // fitmen file name has NO edit label (older form)
      edtfilt << runID + ".*.*." + tripl + ".xml";
   }

   fname_edit = "";
   edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
   nedtfs     = edtfiles.size();
DbgLv(1) << "EDITFILT" << edtfilt << "nedtfs" << nedtfs;

   if ( nedtfs == 1 )
   {  // Choose the single corresponding edit
      fname_edit       = edtfiles.at( 0 );
      //pb_update->setEnabled( true );
   }

   else if ( nedtfs > 0 )
   {  // Choose the latest edit
      int min_ms       = 999999;
      QString fname    = edtfiles.at( 0 );
      QString fpath    = filedir + "/" + fname;
      QDateTime cdate  = QFileInfo( QFile( fpath ) )
                         .lastModified().toUTC();

      for ( int jj = 0; jj < nedtfs; jj++ )
      {
         fname            = edtfiles.at( jj );
         fpath            = filedir + "/" + fname;
         QDateTime fdate  = QFileInfo( QFile( fpath ) )
                            .lastModified().toUTC();
         int file_ms      = fdate.msecsTo( cdate );
DbgLv(1) << "     jj" << jj << "fdate" << fdate << "file_ms" << file_ms;
         if ( file_ms < min_ms )
         {
            min_ms           = file_ms;
            fname_edit       = fname;
         }
      }
DbgLv(1) << " nedtfs" << nedtfs << "fname_edit" << fname_edit;
DbgLv(1) << "   f0" << edtfiles.at(0);
if(nedtfs>1) DbgLv(1) << "   f1" << edtfiles.at(1);

//pb_update->setEnabled( true );
   }

   else
   {  // Could not find edit file, so inform the user
      //pb_update->setEnabled( false );

      QMessageBox::warning( this,
            tr( "Missing Local Edit" ),
            tr( "Update Edit is not possible\n"
                "without a local copy of the Edit file\n"
                "corresponding to the FM models.\n"
                "Use\n     Convert Legacy Data\nand\n     Manage Data\n"
                "to create a local copy of an Edit file for\n     " )
            + fname_load + tr( "\nof run\n     " ) + runID );
   }

   // // If apply-to-all-wavelengths, get list with all wavelengths
   // if ( ck_applymwl->isChecked() )
   // {
   edtfilt.clear();
   edtfilt << fname_edit.section( ".", 0, -3 ) + ".*.xml";
   edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
   nedtfs     = edtfiles.size();
   DbgLv(1) << "FL: aplmwl: nedtfs" << nedtfs << "edtfilt" << edtfilt;
   //  }
   
   //plot_data();
   
   //le_status->setText( tr( "Data loaded:  " ) + runID + "/" + fname_load );
   
   // //Meniscus position, user specified curve
   // minimum_curve_sel = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   // minimum_curve_sel->setPen( QPen( QBrush( Qt::red ), 3.0 ) );
   

   // Load data && process data depending on if it's 2d (Fit Men. || Fit Bott.) OR 3d (Fit Men. && Bott.) 
   load_data_auto( text_content );
   if ( have3val )
     {
       process_3d();
     }
   else
     {
       process_2d();
     }

   return true;
}

//Load data from text_content
void US_Analysis_auto::load_data_auto( const QString& text_content  )
{
   int count         = 0;
   QString contents  = text_content;
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();
   bott_fit          = fname_load.contains( "fitbot" );
DbgLv(1) << "LD:  bott_fit" << bott_fit << "fname_load" << fname_load;

   for ( int ii = 0; ii < lines.size(); ii++ )
   {
      QStringList values = lines[ ii ].split( ' ', QString::SkipEmptyParts );

      int valsize        = values.size();
DbgLv(1) << "LD:  ii" << ii << "valsize" << valsize;

      if ( valsize < 2 )   continue;

      if ( valsize > 3 )
      {
         values.removeFirst();
         valsize--;
      }

      double rmeni  = values[ 0 ].toDouble();
      double rbott  = values[ 1 ].toDouble();
      double rmsdv  = rbott;

      if ( rmeni < 5.0  || rmeni > 8.0 )  continue;

      count++;

      if ( valsize == 3 ) 
      {
         rmsdv         = values[ 2 ].toDouble();
         v_meni << rmeni;
         v_bott << rbott;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rbott, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 

      }

      else if ( valsize == 2 )
      {
         v_meni << rmeni;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 
      }
   }

   bool was3val  = have3val;
   have3val      = ( v_bott.count() > 0 );
   //te_data->e->setPlainText( parsed.join( "\n" ) );
DbgLv(1) << "LD:  was3val have3val" << was3val << have3val
 << "v_rmsd size" << v_rmsd.size() << "parsed length"
 << parsed.length();

   // if ( ( have3val && !was3val )  || 
   //      ( !have3val && was3val ) )
   //   change_plot_type();
}


//process 2d data
void US_Analysis_auto::process_2d( void )
{
   int     count = v_meni.count();
   double* radius_values = v_meni.data();
   double* rmsd_values   = v_rmsd.data();

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int ii = 0; ii < count; ii++ )
   {
      // Find min and max
      minx = qMin( minx, radius_values[ ii ] );
      maxx = qMax( maxx, radius_values[ ii ] );

      miny = qMin( miny, rmsd_values[ ii ] );
      maxy = qMax( maxy, rmsd_values[ ii ] );
   }

   if ( count < 3 ) return;

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   // meniscus_plot->setAxisScale( QwtPlot::xBottom, 
   //       minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;
   dy_global = dy;
   
   //meniscus_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   // raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   // raw_curve->setPen    ( QPen( Qt::yellow ) );
   // raw_curve->setSamples( radius_values, rmsd_values, count );

   // Do the fit and get the minimum

   double c[ 10 ];

   //int order = sb_order->value();
   int order = 2; //ALEXEY ---------------------------------------------------------> How order is passed??

   if ( ! US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
      
      // le_men_fit   ->clear();
      // le_bot_fit   ->clear();
      // le_mprads    ->clear();
      // le_rms_error ->clear();
      // meniscus_plot->replot();

      return;  
   }

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   QVector< double > vfitx( fit_count );
   QVector< double > vfity( fit_count );
   double* fit_x = vfitx.data();
   double* fit_y = vfity.data();
   double  x     = minx - overscan;
   
   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = radius_values[ i ];
      double y = rmsd_values  [ i ];

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   //le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 5 ) );

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      fit_xvl = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivative
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      US_Matrix::lsfit( q, radius_values, rmsd_values, count, 3 );
      fit_xvl = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivative
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( fit_xvl, i );

        // f_prime is the 2nd derivative
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( fit_xvl, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        fit_xvl -= f / f_prime;

      } while ( true );
   }

   // fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   // fit_curve->setPen    ( QPen( Qt::red ) );
   // fit_curve->setSamples( fit_x, fit_y, fit_count );


   // copy miny to global variables
   miny_global = miny;
   
   // Plot the minimum

   // minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   // minimum_curve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = fit_xvl;
   radius_min[ 1 ] = fit_xvl;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 1 ] = miny + 2.0 * dy;

   // minimum_curve->setSamples( radius_min, rmsd_min, 2 );

   // Display selected meniscus/bottom
   //le_men_sel->setText( QString::number( fit_xvl, 'f', 5 ) );

   // Find the "best-index", index where X closest to fit
   ix_best           = 0;
   double diff_min   = 1.0e+99;

   for ( int ii = 0; ii < v_meni.count(); ii++ )
   {
      double diff_x     = qAbs( v_meni[ ii ] - fit_xvl );

      if ( diff_x < diff_min )
      {  // Running least difference and index to it
         diff_min          = diff_x;
         ix_best           = ii;
      }
   }
   

   // // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   // QPen markerPen( QBrush( Qt::white ), 3.0 );
   // markerPen.setWidth( 3 );
   
   // QwtPlotMarker* pm  = new QwtPlotMarker();
   // QwtText        label( QString::number( fit_xvl, 'f', 5 ) );
   // QFont          font( pm->label().font() );
   // QwtSymbol*     sym = new QwtSymbol( QwtSymbol::Cross, QBrush( Qt::white ),
   //                                     markerPen, QSize( 9, 9 ) );

   // font.setBold( true );
   // font.setPointSize( font.pointSize() + 1 );
   // label.setFont( font );

   // pm->setValue         ( fit_xvl, miny + 3.0 * dy );
   // pm->setSymbol        ( sym ); 
   // pm->setLabel         ( label );
   // pm->setLabelAlignment( Qt::AlignTop );

   // pm->attach( meniscus_plot );

   // meniscus_plot->replot();
}


//process 3d data
void US_Analysis_auto::process_3d( void )
{
   bool auto_lim = false;
   double min_x  = 1.0e+99;
   double min_y  = 1.0e+99;
   double min_z  = 1.0e+99;
   double max_x  = -1.0e+99;
   double max_y  = -1.0e+99;
   double max_z  = -1.0e+99;
   double b_meni = 0.0;
   double b_bott = 0.0;
   int min_ix    = -1;

   // Get minima,maxima and meniscus,bottom at best rmsd
   for ( int ii = 0; ii < v_meni.size(); ii++ )
   {
      double rmeni  = v_meni[ ii ];
      double rbott  = v_bott[ ii ];
      double rrmsd  = v_rmsd[ ii ];

      // Find min and max
      min_x        = qMin( min_x, rmeni );
      max_x        = qMax( max_x, rmeni );
      min_y        = qMin( min_y, rbott );
      max_y        = qMax( max_y, rbott );
      max_z        = qMax( max_z, rrmsd );

      if ( rrmsd < min_z )
      {  // Save best-rmsd meniscus,bottom
         min_z        = rrmsd;
         b_meni       = rmeni;
         b_bott       = rbott;
         min_ix       = ii + 1;
      }
   }
   double min_r = 1.0 / max_z;
   double max_r = 1.0 / min_z;

   // // Report low-rmsd meniscus and bottom values
   // le_men_lor->setText( QString::number( b_meni, 'f', 5 ) );
   // le_bot_lor->setText( QString::number( b_bott, 'f', 5 ) +
   //                      "  ( " +
   //                      QString::number( min_ix ) + " )" );
DbgLv(1) << "pl3d: v_meni size" << v_meni.size() << "min,max x,yz"
 << min_x << max_x << min_y << max_y << min_z << max_z;

   // Compute and report fitted meniscus and bottom

   int nmeni     = v_meni.count( v_meni[ 0 ] );
   int nbott     = v_bott.count( v_bott[ 0 ] );
   ix_best       = min_ix - 1;
   int ix_men    = ix_best / nbott;
   int ix_bot    = ix_best % nbott;
   QList< int >  ixs;
DbgLv(1) << "pl3d:  nmeni nbott" << nmeni << nbott << "ix_best" << ix_best
 << "ix_men ix_bot" << ix_men << ix_bot;

   // Compute indexes for up to 9 points centered at best index.
   //  Exclude some if center point is on edge(s).
   for ( int jmx = ix_men - 1; jmx < ix_men + 2; jmx++ )
   {
      if ( jmx < 0  ||  jmx >= nmeni )    // Skip out-of-bounds index
         continue;

      for ( int jbx = ix_bot - 1; jbx < ix_bot + 2; jbx++ )
      {
         if ( jbx < 0  ||  jbx >= nbott ) // Skip out-of-bounds index
            continue;

         ixs << ( jmx * nbott + jbx );    // Save full vector index
DbgLv(1) << "pl3d:   jmx jbx ixs" << jmx << jbx << (jmx*nbott+jbx);
      }
   }

   // Now calculate the weight averages of meniscus and bottom
   f_meni = 0.0;
   f_bott = 0.0;
   double w_rmsd = 0.0;
   for ( int ii = 0; ii < ixs.count(); ii++ )
   {
      int jx        = ixs[ ii ];
      double f_rmsd = 1.0 / v_rmsd[ jx ] - min_r;
      f_meni       += ( v_meni[ jx ] * f_rmsd );
      f_bott       += ( v_bott[ jx ] * f_rmsd );
      w_rmsd       += f_rmsd;
DbgLv(1) << "pl3d:  ixs" << jx << "f_meni f_bott w_rmsd"
 << f_meni << f_bott << w_rmsd;
   }
   f_meni       /= w_rmsd;
   f_bott       /= w_rmsd;
DbgLv(1) << "pl3d:  f_meni f_bott" << f_meni << f_bott;

   // le_men_fit->setText( QString::number( f_meni, 'f', 5 ) );
   // le_bot_fit->setText( QString::number( f_bott, 'f', 5 ) );
}

//get status of the unique start of the FITMEN update
int US_Analysis_auto::read_autoflowAnalysisStages( const QString& requestID )
{
  int status = 0;

  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "FitMeniscus: Could not connect to database \n" ) + db->lastError() );
      return status;
    }


  //qDebug() << "BEFORE query ";
  QStringList qry;
  qry << "fitmen_autoflow_analysis_status"
      << requestID;

  status = db->statusQuery( qry );
  //qDebug() << "AFTER query ";

  return status;
}


// Slot to revert autoflowAnalysisStages record
void US_Analysis_auto::revert_autoflow_analysis_stages_record( const QString& requestID )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "FitMen revert: Could not connect to database \n" ) + db->lastError() );
       return;
     }
   
   //qDebug() << "BEFORE query ";
   QStringList qry;
   qry << "fitmen_autoflow_analysis_status_revert"
       << requestID;
   
   db->query( qry );
   //qDebug() << "AFTER query ";
}



// Update an edit file with a new meniscus and/or bottom radius value
void US_Analysis_auto::edit_update_auto( QMap < QString, QString > & triple_information )
{
  /***/
  //ALEXEY: if autoflow: check if edit profiles already updated from other FITMEN session
  
  QString requestID = triple_information[ "requestID" ];
  //--- LOCK && UPDATE the autoflowStages' ANALYSIS field for the record
  int status_fitmen_unique;
  status_fitmen_unique = read_autoflowAnalysisStages( requestID );
  
  qDebug() << "status_fitmen_unique -- " << status_fitmen_unique ;
  
  if ( !status_fitmen_unique )
    {
      QMessageBox::information( this,
				tr( "FITMEN | Triple Analysis already processed" ),
				tr( "It appears that FITMEN stage has already been processed by "
				    "a different user from different session. \n\n"
				    "The program will return to the autoflow runs dialog where "
				    "you can re-attach to the actual current stage of the run. "));
      

      emit analysis_back_to_initAutoflow( );
      //emit triple_analysis_processed( );
      //close();
      return;
    }
  /****/

#define MENI_HIGHVAL 7.0
#define BOTT_LOWVAL 7.0
   QString fn = filedir + "/" + fname_edit;
DbgLv(1) << " eupd:  fname_edit" << fname_edit;
DbgLv(1) << " eupd:  fn" << fn;
   idEdit     = 0;
   QFile filei( fn );
   QString edtext;
   QStringList edtexts;

   if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      return;
   }

   bool confirm  = false;
   bool all_wvl  = ( nedtfs > 1 );
   bool rmv_mdls = true;
   bool db_upd   = true;

   qDebug() << "In FITMEN_AUTO's edit_update(): confirm, all_wvl, rmv_mdls, db_upd  ? " << confirm << all_wvl <<  rmv_mdls << db_upd;
//*DEBUG*
//db_upd=false;
//*DEBUG*
   int ixmlin    = 0;     // Meniscus line start index
   int ixmval    = 0;     // Meniscus value start index
   int ncmval    = 0;     // Meniscus value number characters
   int ncmlin    = 0;     // Meniscus line number characters
   int ixblin    = 0;     // Bottom line start index
   int ixbval    = 0;     // Bottom value start index
   int ncbval    = 0;     // Bottom value number characters
   int ncblin    = 0;     // Bottom line number characters
   int ixllin    = 0;     // Left-data line start index
   int ixlval    = 0;     // Left-data value start index
   int nclval    = 0;     // Left-data value number characters
   int demval    = 0;     // Delta old/new meniscus value
   int debval    = 0;     // Delta old/new bottom value

   // New meniscus and bottom values
   double mennew = 0.0;
   double botnew = 0.0;
   double lefval = 0.0;

   
   
   if ( have3val )
   {  // Fit is meniscus + bottom
      // mennew         = le_men_fit->text().toDouble();
      // botnew         = le_bot_fit->text().toDouble();
     mennew = f_meni;
     botnew = f_bott;
   }
   else if ( !bott_fit )
   {  // Fit is meniscus only
      //mennew         = le_men_sel->text().toDouble();
     mennew = fit_xvl;
   }
   else
   {  // Fit is bottom only
      //botnew         = le_men_sel->text().toDouble();
     botnew = fit_xvl;
   }

   QString s_meni = QString().sprintf( "%.5f", mennew );
   QString s_bott = QString().sprintf( "%.5f", botnew );
DbgLv(1) << " eupd:  s_meni s_bott" << s_meni << s_bott;
   QString mmsg   = "";
   QString mhdr   = "";
   bool bad_vals  = false;

   // Check meniscus and bottom values for reasonableness
   if ( mennew > MENI_HIGHVAL )
   {
      bad_vals       = true;

      if ( botnew != 0.0  &&  botnew < BOTT_LOWVAL )
      {
         mhdr           = tr( "Unreasonable Meniscus and Bottom" );
         mmsg           = tr( "Both the currently selected Mensicus and\n"
                              "Bottom values" ) + " ( " + s_meni + ", "
                        + s_bott + " )\n"
                        + tr( "are unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting reasonable values." );
      }
      else
      {
         mhdr           = tr( "Unreasonable Meniscus" );
         mmsg           = tr( "The currently selected Mensicus value" )
                        + " ( " + s_meni + " )\n"
                        + tr( "is unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting a reasonable value." );
      }

   }
   else if ( botnew != 0.0  &&  botnew < BOTT_LOWVAL )
   {
      bad_vals       = true;
      mhdr           = QString( tr( "Unreasonable Bottom: %1" )).arg( triple_information[ "triple_name" ] );
      mmsg           = tr( "The currently selected Bottom value" )
                     + " ( " + s_bott + " )\n"
                     + tr( "is unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting a reasonable value." );
   }

   if ( bad_vals )
   {
     //cancel job && restart timer();
     fitmen_bad_vals = true;
     QString reason_for_failure = mhdr + ", " + mmsg.split("!")[0];
     triple_information[ "failed" ] = reason_for_failure;
     delete_jobs_at_fitmen( triple_information );

     return;
   }
   
   mmsg           = "";

   //ALEXEY: Set progressDialog
   progress_msg_fmb = NULL;
   
   // Proceed with updating the Edit data

   if ( ! all_wvl  ||  nedtfs == 1 )
   {  // Apply to a single triple
      QTextStream ts( &filei );    // Build up XML Edit text
      while ( !ts.atEnd() )
         edtext += ts.readLine() + "\n";
      filei.close();

      // Compute indecies,lengths of meniscus,bottom,data lines,values
      ixmlin   = edtext.indexOf( "<meniscus radius=" );
      if ( ixmlin < 0 )  return;
      ixmval   = edtext.indexOf( "=\"", ixmlin ) + 2;
      ncmval   = edtext.indexOf( "\"",  ixmval + 1 ) - ixmval;
      ncmlin   = edtext.indexOf( ">", ixmlin ) - ixmlin + 1;
      ixllin   = edtext.indexOf( "<data_range left=" );
      if ( ixllin < 0 )  return;
      ixlval   = edtext.indexOf( "=\"", ixllin ) + 2;
      nclval   = edtext.indexOf( "\"",  ixlval + 1 ) - ixlval;
      lefval   = edtext.mid( ixlval, nclval ).toDouble();
      ixblin   = edtext.indexOf( "<bottom radius=" );
      if ( ixblin > 0 )
      {
         ixbval   = edtext.indexOf( "=\"", ixblin ) + 2;
         ncbval   = edtext.indexOf( "\"",  ixbval + 1 ) - ixbval;
         ncblin   = edtext.indexOf( ">", ixblin ) - ixblin + 1;
      }
DbgLv(1) << " eupd:  mennew" << mennew << "lefval" << lefval << "botnew" << botnew;
DbgLv(1) << " eupd:   ixmlin ixblin" << ixmlin << ixblin << "ncmlin ncblin" << ncmlin << ncblin;

      if ( mennew >= lefval )  //ALEXEY: HERE!!!!
      {
	//cancel job && restart timer();
	fitmen_bad_vals = true;
	QString reason_for_failure = QString( "The selected Meniscus value, %1 , extends into the data range whose left-side value is %2")
	  .arg( mennew )
	  .arg( lefval );
	
	triple_information[ "failed" ] = reason_for_failure;
	delete_jobs_at_fitmen( triple_information );
	
	return;
      }

      demval        = s_meni.length() - ncmval;  // Deltas old,new values
      debval        = s_bott.length() - ncbval;

      if ( have3val )
      {  // Replace meniscus and bottom values in edit
         edtext.replace( ixmval, ncmval, s_meni );
         ncmval       += demval;
         ncmlin       += demval;
         if ( ixbval > 0 )
         {  // Replace existing bottom value
            ixbval       += demval;
            edtext.replace( ixbval, ncbval, s_bott );
            ixblin       += demval;
            ncbval       += debval;
            ncblin       += debval;
         }
         else
         {  // Must insert an entirely new line for bottom
            QString bline = QString( "\n            <bottom radius=\"" )
                            + s_bott + QString( "\"/>" );
            ixblin        = ixmlin + ncmlin;
            edtext.insert( ixblin, bline );
            ncblin        = bline .length();
            ncbval        = s_bott.length();
         }
      }
      else if ( !bott_fit )
      {  // Replace meniscus value in edit
         edtext.replace( ixmval, ncmval, s_meni );
         ncmval       += demval;
         ncmlin       += demval;
      }
      else
      {  // Replace/insert bottom value in edit
         if ( ixbval > 0 )
         {  // Replace existing bottom value
            edtext.replace( ixbval, ncbval, s_bott );
            ncbval       += debval;
            ncblin       += debval;
         }
         else
         {  // Must insert an entirely new line for bottom
            QString bline = QString( "\n            <bottom radius=\"" )
                            + s_bott + QString( "\"/>" );
            ixblin        = ixmlin + ncmlin;
            edtext.insert( ixblin, bline );
            ncblin        = bline .length();
            ncbval        = s_bott.length();
         }
      }

      // if ( auto_mode )
      //    le_status->setText( "Updating edit profile: " + fname_edit ); 
      
      // Write out the modified Edit XML text
      QFile fileo( fn );

      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
	//-- Revert autoflowAnalysisSatges back to 'unknown'
	QString requestID = triple_information[ "requestID" ];
	revert_autoflow_analysis_stages_record( requestID );
	//---------------------------------------------------//
	
         return;
      }

      QTextStream tso( &fileo );
      tso << edtext;
      fileo.close();

      // If using DB, update the edit record there

      if ( db_upd )
      {
         update_db_edit( edtext, fn );
      }

 
   }  // END: apply to single triple

   else
   {  // Apply to all wavelengths in a cell/channel

     //ALEXEY: Set progressDialog
     progress_msg_fmb = new QProgressDialog ("Updating edit profiles...", QString(), 0, nedtfs, this);
     progress_msg_fmb->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
     progress_msg_fmb->setWindowModality(Qt::WindowModal);
     progress_msg_fmb->setWindowTitle( QString( tr("Updating Edit Profiles: %1")).arg( triple_information[ "triple_name" ] ));
     QFont font_d  = progress_msg_fmb->property("font").value<QFont>();
     QFontMetrics fm(font_d);
     int pixelsWide = fm.width( progress_msg_fmb->windowTitle() );
     qDebug() << "Progress_msg_fmb: pixelsWide -- " << pixelsWide;
     progress_msg_fmb ->setMinimumWidth( pixelsWide*2 );
     progress_msg_fmb->adjustSize();
     
     progress_msg_fmb->setAutoClose( false );
     progress_msg_fmb->setValue( 0 );
     //progress_msg_fmb->setRange( 1, nedtfs );
     progress_msg_fmb->show();
     ////////////////////////////

      QString dmsg   = "";
      int idEdsv   = idEdit;
DbgLv(1) << " eupd: AppWvl: nedtfs" << nedtfs;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      for ( int jj = 0; jj < nedtfs; jj++ )
      {  // Modify each Edit file corresponding to a wavelength
         QString fn = filedir + "/" + edtfiles.at( jj );
DbgLv(1) << " eupd:     jj" << jj << "fn" << fn;

         // if ( auto_mode )
         //    le_status->setText( "Updating edit profile: " + edtfiles.at( jj )  ); 
 
         QFile filei( fn );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {  // Skip any file we cannot read
DbgLv(1) << " eupd:       *OPEN ERROR*";
            continue;
         }

DbgLv(1) << " eupd:       edtext read";
         // Read in the Edit XML text
         QTextStream ts( &filei );
         edtext  = "";
         while ( !ts.atEnd() )
            edtext += ts.readLine() + "\n";
         filei.close();
DbgLv(1) << " eupd:       edtext len" << edtext.length();

         // Compute locations and lengths of meniscus,bottom,data lines,values
         ixmlin   = edtext.indexOf( "<meniscus radius=" );
         if ( ixmlin < 0 )  continue;
         ixmval   = edtext.indexOf( "=\"", ixmlin ) + 2;
         ncmval   = edtext.indexOf( "\"",  ixmval + 1 ) - ixmval;
         ncmlin   = edtext.indexOf( ">", ixmlin ) - ixmlin + 1;
         ixllin   = edtext.indexOf( "<data_range left=" );
         if ( ixllin < 0 )  continue;
         ixlval   = edtext.indexOf( "=\"", ixllin ) + 2;
         nclval   = edtext.indexOf( "\"",  ixlval + 1 ) - ixlval;
         lefval   = edtext.mid( ixlval, nclval ).toDouble();
         ixblin   = edtext.indexOf( "<bottom radius=" );
         if ( ixblin > 0 )
         {
            ixbval   = edtext.indexOf( "=\"", ixblin ) + 2;
            ncbval   = edtext.indexOf( "\"",  ixbval + 1 ) - ixbval;
            ncblin   = edtext.indexOf( ">", ixblin ) - ixblin + 1;
         }
DbgLv(1) << " eupd:       ixmlin ixblin ixllin" << ixmlin << ixblin << ixllin;

         if ( mennew >= lefval )  //ALEXEY: HERE !!!
         {
	    //cancel job && restart timer();
	    fitmen_bad_vals = true;
	    QString reason_for_failure = QString( "The selected Meniscus value, %1 , extends into the data range whose left-side value is %2")
	      .arg( mennew )
	      .arg( lefval );
	    
	    triple_information[ "failed" ] = reason_for_failure;
	    delete_jobs_at_fitmen( triple_information );

	    return;  //ALEXEY - if one wvl in a triple fails, ALL fail!!!
            //continue;
         }

         demval        = s_meni.length() - ncmval;  // Deltas in old,new value strings
         debval        = s_bott.length() - ncbval;

         if ( have3val )
         {  // Replace meniscus and bottom values in edit
            edtext.replace( ixmval, ncmval, s_meni );
            ncmval       += demval;
            ncmlin       += demval;
            if ( ixbval > 0 )
            {  // Replace an existing bottom line
               ixbval       += demval;
               edtext.replace( ixbval, ncbval, s_bott );
               ixblin       += demval;
               ncbval       += debval;
               ncblin       += debval;
            }
            else
            {  // Must insert an entirely new line for bottom
               QString bline = QString( "\n            <bottom radius=\"" )
                               + s_bott + QString( "\"/>" );
               ixblin        = ixmlin + ncmlin;
               edtext.insert( ixblin, bline );
               ncblin        = bline .length();
               ncbval        = s_bott.length();
            }
DbgLv(1) << " eupd:       3DVL replace";
         }
         else if ( !bott_fit )
         {  // Replace meniscus value in edit
            edtext.replace( ixmval, ncmval, s_meni );
            ncmval       += demval;
            ncmlin       += demval;
DbgLv(1) << " eupd:       MENI replace";
         }
         else
         {  // Replace bottom value in edit
            if ( ixbval > 0 )
            {
               edtext.replace( ixbval, ncbval, s_bott );
               ncbval       += debval;
               ncblin       += debval;
            }
            else
            {  // Must insert an entirely new line for bottom
               QString bline = QString( "\n            <bottom radius=\"" )
                               + s_bott + QString( "\"/>" );
               ixblin        = ixmlin + ncmlin;
               edtext.insert( ixblin, bline );
               ncblin        = bline .length();
               ncbval        = s_bott.length();
            }
DbgLv(1) << " eupd:       BOTT replace";
         }

DbgLv(1) << " eupd:  write: fn" << fn;
         // Write out the modified Edit XML file
         QFile fileo( fn );

         if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
            continue;

         QTextStream tso( &fileo );
         tso << edtext;
         fileo.close();

         // If using DB, update the edit record there

         if ( db_upd )
         {

	   if (progress_msg_fmb != NULL )
	     {
	       progress_msg_fmb->setValue( jj );
	     }
	     
DbgLv(1) << " eupd:       call upd_db_ed";
            update_db_edit( edtext, fn );
DbgLv(1) << " eupd:       ret fr upd_db_ed  idEdit" << idEdit;

            if ( edtfiles[ jj ] == fname_edit )
               idEdsv       = idEdit;
         }

      }  // END: wavelengths loop
DbgLv(1) << " eupd:       idEdit idEdsv" << idEdit << idEdsv;
      idEdit       = idEdsv;
DbgLv(1) << " eupd:       idEdit" << idEdit;

      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

            
 
   }  // END: apply to all wavelengths

    if ( rmv_mdls )
      {
	DbgLv(1) << " call Remove Models";
	remove_models_auto( triple_information[ "invID" ] );
      }

 

   //Set progress to max
   if ( progress_msg_fmb != NULL )
     {
       progress_msg_fmb->setValue( progress_msg_fmb->maximum() );  //ALEXEY -- bug fixed..
       progress_msg_fmb->close();
     }

   update_autoflowAnalysis_statuses( triple_information );
   //emit editProfiles_updated( triple_information );
   //close();
   
}

// Update the DB edit record with a new meniscus and/or bottom value
void US_Analysis_auto::update_db_edit( QString edtext, QString efilepath )
{
   int     elnx     = edtext.indexOf( "<editGUID " );
   int     esvx     = edtext.indexOf( "\"", elnx ) + 1;
   int     nvch     = edtext.indexOf( "\"", esvx ) - esvx;
   QString edGUID   = edtext.mid( esvx, nvch );
DbgLv(1) << "updDbEd: edGUID" << edGUID;

   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );
   QStringList query;
   query << "get_editID" << edGUID;
   db.query( query );
   db.next();
   idEdit           = db.value( 0 ).toString().toInt();
DbgLv(1) << "updDbEd: idEdit" << idEdit;
   if ( db.writeBlobToDB( efilepath, "upload_editData", idEdit ) != US_DB2::OK )
     {
       qDebug() << tr( "*ERROR* update_db_edit: " ) << db.lastError();
       //msg += tr( "*ERROR* update_db_edit: " );
       //msg += db.lastError();
       return;
     }
   
   
   return;
}


// Remove f-m models (and associated noise) except for the single chosen one
void US_Analysis_auto::remove_models_auto( QString invID )
{
   QString srchRun  = filedir.section   ( "/", -1, -1 );
   QString srchEdit = "e" +
                      fname_edit.section( ".", -6, -6 );
   QString srchTrip = fname_load.section( ".", -3, -3 );
   QString msetBase;
DbgLv(1) << "RmvMod: fname_load" << fname_load;
DbgLv(1) << "RmvMod: fname_edit" << fname_edit;
DbgLv(1) << "RmvMod: scn1  srchRun"
 << srchRun << "srchEdit" << srchEdit << "srchTrip" << srchTrip;

   // Get the model,list index to selected meniscus/bottom value(s)
   index_model_setfit();

   // Scan models files; get list of fit-meniscus type matching run/edit/triple
   QStringList modfilt;
   modfilt << "M*.xml";
   QString     moddir   = US_Settings::dataDir() + "/models";
   QStringList modfiles = QDir( moddir ).entryList(
         modfilt, QDir::Files, QDir::Name );
DbgLv(1) << "RmvMod: raw modfiles count" << modfiles.count();
   moddir               = moddir + "/";

   QList< ModelDesc >  lMDescrs;
   QList< ModelDesc >  dMDescrs;

   QStringList     lmodFnams;             // local model full path file names
   QStringList     lmodGUIDs;             // Local model GUIDs
   QList< double > lmodVaris;             // Local variance values
   QList< double > lmodMenis;             // Local meniscus values
   QStringList     lmodDescs;             // Local descriptions

   QStringList     dmodIDs;               // Database model IDs
   QStringList     dmodGUIDs;             // Database model GUIDs
   QList< double > dmodVaris;             // Database variance values
   QList< double > dmodMenis;             // Database meniscus values
   QStringList     dmodDescs;             // Database descriptions
   int nlmods           = 0;
   int ndmods           = 0;
   int nlnois           = 0;
   int ndnois           = 0;
   int lArTime          = 0;
   int dArTime          = 0;
   int lkModx           = -1;
   int dkModx           = -1;
   bool db_upd          = true;
//*DEBUG*
//db_upd=false;
//*DEBUG*

DbgLv(1) << "RmvMod: dk: modfiles size" << modfiles.size();
   for ( int ii = 0; ii < modfiles.size(); ii++ )
   {
      ModelDesc lmodd;
      QString modfname   = modfiles.at( ii );
      QString modpath    = moddir + modfname;
      US_Model model;
      
      if ( model.load( modpath ) != US_DB2::OK )
      {
DbgLv(1) << "RmvMod:  *LOAD ERR*" << modfname;
         continue;    // Can't use if can't load
      }

      QString descript   = model.description;
      QString runID      = descript.section( '.',  0, -4 );
      QString tripID     = descript.section( '.', -3, -3 );
      QString anRunID    = descript.section( '.', -2, -2 );
      QString editLabl   = anRunID .section( '_',  0, -5 );
      QString iterID     = anRunID .section( '_', -1, -1 );
      QString itNum      = iterID  .section( '-',  0,  0 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;
if(ii<3 || (ii+4)>modfiles.size() || ii==(modfiles.size()/2))
 DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
  << ii << runID << editLabl << tripID;

      if ( runID != srchRun  ||  editLabl != srchEdit  ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

      if ( !iterID.contains( "-m" )  &&  !iterID.contains( "-b" ) )
      {
DbgLv(1) << "RmvMod:   ii" << ii << "iterID" << iterID
 << "has -m" << iterID.contains("-m") << "has -b" << iterID.contains("-b");
         continue;    // Can't use if not a fit-meniscus or fit-bottom type
      }

      if ( itNum == "01"  &&  msetBase.isEmpty() )
      {  // When fit iteration is "01" and not yet saved, save model set base
         msetBase       = descript.section( ".", 0, -3 ) + "." + 
                          anRunID .section( "_", 0,  3 );
DbgLv(1) << "RmvMod:lfiles: msetBase" << msetBase << "ii" << ii;
      }

      // Probably a file from the right set, but let's check for other sets
      int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();
DbgLv(1) << "RmvMod:    arTime lArTime" << arTime << lArTime;

      if ( arTime > lArTime )
      {  // If first set or new one younger than previous, start lists
         lmodFnams.clear();
         lmodGUIDs.clear();
         lmodVaris.clear();
         lmodMenis.clear();
         lmodDescs.clear();
         lMDescrs .clear();
         lArTime            = arTime;
      }

      else if ( arTime < lArTime )
      {  // If new one older than previous, skip it
         continue;
      }

      lmodFnams << modpath;             // Save full path file name
      lmodGUIDs << model.modelGUID;     // Save model GUID
      lmodVaris << model.variance;      // Save variance
      lmodMenis << model.meniscus;      // Save meniscus
      lmodDescs << model.description;   // Save description

      lmodd.description = model.description;
      lmodd.modelGUID   = model.modelGUID;
      lmodd.modelID     = "-1";
      lmodd.variance    = model.variance;
      lmodd.meniscus    = model.meniscus;
      lMDescrs << lmodd;
   }

   nlmods         = lMDescrs.size();
   qSort( lMDescrs );
DbgLv(1) << "RmvMod: nlmods" << nlmods << "msetBase" << msetBase;

   for ( int ii = 0; ii < nlmods; ii++ )
   {  // Scan to identify model in set with iteration index
      //  corresponding to best fit
      ModelDesc lmodd = lMDescrs[ ii ];
      QString descrip = QString( lmodd.description );
      QString ansysID = descrip.section( '.', -2, -2 );
      QString iterID  = ansysID.section( '_', -1, -1 );
      int iterx       = iterID .section( '-',  0,  0 )
                               .mid( 1 ).toInt() - 1;
DbgLv(1) << "RmvMod: best ndx scan: ii vari" << ii << lmodd.variance
 << "iterID iters" << iterID << iterx;

      if ( iterx == ix_setfit )
      {
         lkModx         = ii;
DbgLv(1) << "RmvMod:   best ndx scan:   lkModx" << lkModx;
      }
   }
DbgLv(1) << "RmvMod:  ix_setfit lkModx" << ix_setfit << lkModx;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Make a list of fit models that match for DB, if possible
   if ( db_upd )
   {
      ModelDesc dmodd;
      
      //QString invID =  triple_information[ "invID" ];
      qDebug() << "FITMEN_AUTO: In remove_models(): invID -- " << invID;
      
      QString   edtID = QString::number( idEdit );
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;
      QStringList modIDs;

      if ( idEdit > 0 )
         query << "get_model_desc_by_editID" << invID << edtID;

      else
         query << "get_model_desc" << invID;

DbgLv(1) << "RmvMod:  idEdit" << idEdit << "query" << query;
      db.query( query );

      while( db.next() )
      {
         QString modelID    = db.value( 0 ).toString();
         QString descript   = db.value( 2 ).toString();
         QString runID      = descript.section( '.',  0, -4 );
         if ( runID == srchRun )
         {
            modIDs << modelID;
         }
      }

DbgLv(1) << "RmvMod:  modIDs size" << modIDs.size();
      for ( int ii = 0; ii < modIDs.size(); ii++ )
      {
         QString modelID    = modIDs.at( ii );
         query.clear();
         query << "get_model_info" << modelID;
         db.query( query );

         if ( db.lastErrno() != US_DB2::OK )  continue;

         db.next();

         QString modelGUID  = db.value( 0 ).toString();
         QString descript1  = db.value( 1 ).toString();
         QString contents   = db.value( 2 ).toString();
         int jdtx           = contents.indexOf( "description=" );

         if ( jdtx < 1 )  continue;

         int jdx            = contents.indexOf( "\"", jdtx ) + 1;
         int lend           = contents.indexOf( "\"", jdx  ) - jdx;
         QString descript   = contents.mid( jdx, lend );

         // Skip model that is not from the right set
         if ( !msetBase.isEmpty()  &&
              !descript.startsWith( msetBase ) )
         {
DbgLv(1) << "RmvMod:  descript" << descript << "msetBase" << msetBase << "ii" << ii;
            continue;
         }

         double  variance   = db.value( 3 ).toString().toDouble();
         double  meniscus   = db.value( 4 ).toString().toDouble();
         QString runID      = descript.section( '.',  0, -4 );
         QString tripID     = descript.section( '.', -3, -3 );
         QString anRunID    = descript.section( '.', -2, -2 );
         QString editLabl   = anRunID .section( '_',  0, -5 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

         if ( runID != srchRun  ||  editLabl != srchEdit  ||
              tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

         QString iterID     = anRunID .section( '_', -1, -1 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

         if ( !iterID.contains( "-m" )  &&  !iterID.contains( "-b" ) )
         {
            continue;    // Can't use if not a fit-meniscus type
         }

         // Probably a file from the right set, but let's check for other sets
         int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();

         if ( arTime > dArTime )
         {  // If first set or new one younger than previous, start lists
            dmodIDs  .clear();
            dmodGUIDs.clear();
            dmodVaris.clear();
            dmodMenis.clear();
            dmodDescs.clear();
            dMDescrs .clear();
            dArTime            = arTime;
         }

         else if ( arTime < dArTime )
         {  // If new one older than previous, skip it
            continue;
         }

         dmodIDs   << modelID;             // Save model DB ID
         dmodGUIDs << modelGUID;           // Save model GUID
         dmodVaris << variance;            // Save variance
         dmodMenis << meniscus;            // Save meniscus
         dmodDescs << descript;            // Save description

         dmodd.description = descript;
         dmodd.modelGUID   = modelGUID;
         dmodd.modelID     = modelID;
         dmodd.variance    = variance;
         dmodd.meniscus    = meniscus;
         dMDescrs << dmodd;
DbgLv(1) << "RmvMod:  scn2 ii dmodDesc" << descript; 
      }

      ndmods         = dMDescrs.size();
      qSort( dMDescrs );

      if ( dArTime > lArTime )      // Don't count any older group
         nlmods         = 0;
      else if ( lArTime > dArTime )
         ndmods         = 0;
DbgLv(1) << "RmvMod: ndmods" << ndmods;

      for ( int ii = 0; ii < ndmods; ii++ )
      {  // Scan to identify model in set with lowest variance
         ModelDesc dmodd = dMDescrs[ ii ];
         QString descrip = QString( dmodd.description );
         QString ansysID = descrip.section( '.', -2, -2 );
         QString iterID  = ansysID.section( '_', -1, -1 );
         int iterx       = iterID .section( '-',  0,  0 )
                               .mid( 1 ).toInt() - 1;
DbgLv(1) << "RmvMod: best ndx scan: ii vari" << ii << dmodd.variance
 << "iterID iters" << iterID << iterx;

         if ( iterx == ix_setfit )
         {
            dkModx         = ii;
DbgLv(1) << "RmvMod:   best ndx scan:   dkModx" << dkModx;
         }
      }

      // Now, compare the findings for local versus database
      if ( nlmods == ndmods  ||  ( ndmods > 0 && nlmods == 0 ) )
      {
         int    nmatch  = 0;

         for ( int jj = 0; jj < nlmods; jj++ )
         {
            ModelDesc lmodd = lMDescrs[ jj ];
            ModelDesc dmodd = dMDescrs[ jj ];

            if ( lmodd.modelGUID   == dmodd.modelGUID &&
                 lmodd.description == dmodd.description )
               nmatch++;

            lmodGUIDs[ jj ]    = lmodd.modelGUID;
            lmodVaris[ jj ]    = lmodd.variance;
            lmodMenis[ jj ]    = lmodd.meniscus; 
            lmodDescs[ jj ]    = lmodd.description;
         }

         for ( int jj = 0; jj < ndmods; jj++ )
         {
            ModelDesc dmodd = dMDescrs[ jj ];
            dmodIDs  [ jj ]    = dmodd.modelID;
            dmodGUIDs[ jj ]    = dmodd.modelGUID;
            dmodVaris[ jj ]    = dmodd.variance;
            dmodMenis[ jj ]    = dmodd.meniscus; 
            dmodDescs[ jj ]    = dmodd.description;
         }

         if ( nmatch == nlmods )
         {  // OK if they match or local only
DbgLv(1) << "++local/dbase match, or local only";
         }

         else
         {  // Not good if they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH";
DbgLv(1) << "  nmatch ndmods nlmods" << nmatch << ndmods << nlmods;
            return;
         }
      }

      else if ( nlmods == 0 )
      {  // It is OK if there are no local records, when DB ones were found
DbgLv(1) << "++only dbase records exist";
      }

      else if ( ndmods == 0 )
      {  // It is OK if there are only local records, when local ones found
DbgLv(1) << "++only local records exist";
      }

      else
      {  // Non-zero local & DB, but they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH in count";
DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
         return;
      }
   }

DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
   if ( ndmods > 0  ||  nlmods > 0 )
   {  // There are models to scan, so build a list of models,noises to remove
      ModelDesc       rmodDescrs;
      NoiseDesc       rnoiDescrs;
      QStringList     rmodIDs;
      QStringList     rmodDescs;
      QStringList     rmodFnams;
      QStringList     rnoiIDs;
      QStringList     rnoiFnams;
      QStringList     rnoiDescs;
      QStringList     nieDescs;
      QStringList     nieIDs;
      QStringList     nieFnams;
      int             nlrmod = 0;
      int             ndrmod = 0;
      int             nlrnoi = 0;
      int             ndrnoi = 0;
      int             ntmods = ( ndmods > 0 ) ? ndmods : nlmods;
      int             ikModx = ( ndmods > 0 ) ? dkModx : lkModx;
DbgLv(1) << "  ntmods ikModx" << ntmods << ikModx;
//*DEBUG*
//if ( ndmods > 0  ||  nlmods > 0 ) return;
//*DEBUG*

      QString modDesc    = "";

      for ( int jj = 0; jj < ntmods; jj++ )
      {  // Build the list of model files and DB ids for removal
         if ( jj != ikModx )
         {
            int itix;
            int irix;
            QString fname;
            QString mDesc;
            QString mID;
            QString tiDesc;
            QString riDesc;
            QString noiID; 
            QString noiFname;

            if ( nlmods > 0 )
            {
               fname  = lmodFnams[ jj ];
               mDesc  = lmodDescs[ jj ];
               nlrmod++;
               if ( ndmods == 0 )
                  mID    = "-1";
            }

            if ( ndmods > 0 )
            {
               mID    = dmodIDs  [ jj ];
               mDesc  = dmodDescs[ jj ];
               ndrmod++;
               if ( nlmods == 0 )
                  fname  = "";
            }
            rmodIDs   << mID;
            rmodFnams << fname;
            rmodDescs << mDesc;
//DbgLv(1) << "RmvMod: jj" << jj << "mID" << mID << "mDesc" << mDesc;

            if ( modDesc.isEmpty() )
            {
               modDesc = mDesc;   // Save 1st model's description
	       DbgLv(1) << "RmvMod: 1st rmv-mod: jj modDesc" << jj << modDesc;

	       // Build noises-in-edit lists for database and local
	       noises_in_edit( modDesc, nieDescs, nieIDs, nieFnams, invID );
            }

            tiDesc = QString( mDesc ).replace( ".model", ".ti_noise" )
                                     .replace( "2DSA-FM-IT", "2DSA-FM" )
                                     .replace( "2DSA-FB-IT", "2DSA-FB" );
            riDesc = QString( mDesc ).replace( ".model", ".ri_noise" )
                                     .replace( "2DSA-FM-IT", "2DSA-FM" )
                                     .replace( "2DSA-FB-IT", "2DSA-FB" );
            itix   = nieDescs.indexOf( tiDesc );
            irix   = nieDescs.indexOf( riDesc );
DbgLv(1) << "RmvMod:    itix irix" << itix << irix
 << "tiDesc" << tiDesc << "riDesc" << riDesc;

            if ( itix >= 0 )
            {  // There is a TI noise to remove
               noiID    = nieIDs  [ itix ];
               noiFname = nieFnams[ itix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << tiDesc;
            }

            if ( irix >= 0 )
            {  // There is an RI noise to remove
               noiID    = nieIDs  [ irix ];
               noiFname = nieFnams[ irix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << riDesc;
            }
         }
      }
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

      nlnois             = nlrnoi + ( nlrnoi > nlrmod ? 2 : 1 );
      ndnois             = ndrnoi + ( ndrnoi > ndrmod ? 2 : 1 );
      bool rmv_mdls      = true;
DbgLv(1) << "RmvMod: nlrmod ndrmod nlrnoi ndrnoi nlnois ndnois"
 << nlrmod << ndrmod << nlrnoi << ndrnoi << nlnois << ndnois;
 

      if ( rmv_mdls )
      {
         US_Passwd pw;
         US_DB2* dbP = db_upd ? new US_DB2( pw.getPasswd() ) : NULL;

         QStringList query;
         QString recID;
         QString recFname;
         QString recDesc;
DbgLv(1) << " Remove Models and Noises";
         for ( int ii = 0; ii < rmodIDs.size(); ii++ )
         {  // Remove models and db noises
            recID    = rmodIDs  [ ii ];
            recDesc  = rmodDescs[ ii ];
            recFname = rmodFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file model
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            if ( recID != "-1"  &&  dbP != NULL )
            {  // Delete model (and any child noise) from DB
               query.clear();
               query << "delete_model" << recID;
               int stat = dbP->statusQuery( query );
               if ( stat != 0 )
                  qDebug() << "delete_model error" << stat;
else DbgLv(1) << "     DB record deleted";
            }
         }

         if ( dbP != NULL )
         {
            delete dbP;
            dbP   = NULL;
         }

         for ( int ii = 0; ii < rnoiIDs.size(); ii++ )
         {  // Remove local noises
            recID    = rnoiIDs  [ ii ];
            recDesc  = rnoiDescs[ ii ];
            recFname = rnoiFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file noise
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            // No need to remove noises from DB; model remove did that
         }

	 //ALEXEY: crashed here!!!! After deleting noises
      }
   }

   else
   {  // No models were found!!! (huh!!!)
DbgLv(1) << "**NO local/dbase models-to-remove were found!!!!";
   }

   QApplication::restoreOverrideCursor();
   return;
}


// Set the fit models index for set fit meniscus/bottom
void US_Analysis_auto::index_model_setfit()
{
   if ( have3val )
   {  // Fit Mensicus + Bottom:  find closest x,y in list
      // double xmeni  = le_men_fit->text().toDouble();
      // double ybott  = le_bot_fit->text().toDouble();
      
     double xmeni  = f_meni;
     double ybott  = f_bott;

      double lowxyd = 1.0e+99;
DbgLv(1) << "IMS:  xmeni ybott" << xmeni << ybott;

      for ( int ii = 0; ii < v_meni.count(); ii++ )
      {  // Find lowest difference in X,Y list
         double xydiff = sq( ( xmeni - v_meni[ ii ] ) ) +
                         sq( ( ybott - v_bott[ ii ] ) );
         xydiff        = ( xydiff == 0.0 ) ? 0.0 : sqrt( xydiff );

         if ( xydiff < lowxyd )
         {  // Save lowest difference so far and its index
            ix_setfit     = ii;
            lowxyd        = xydiff;
DbgLv(1) << "IMS:    ii" << ii << "lowxyd" << lowxyd;
         }
      }
   }

   else
   {  // Fit Meniscus OR Fit Bottom:  find close X in list
      //double xselec = le_men_sel->text().toDouble();

     double xselec = fit_xvl;
      
      double lowxd  = 1.0e+99;
DbgLv(1) << "IMS:  xselec" << xselec << "v_meni count" << v_meni.count();

      for ( int ii = 0; ii < v_meni.count(); ii++ )
      {  // Find lowest difference in X list
         double xdiff  = qAbs( ( xselec - v_meni[ ii ] ) );

         if ( xdiff < lowxd )
         {  // Save lowest difference so far and its index
            ix_setfit     = ii;
            lowxd         = xdiff;
DbgLv(1) << "IMS:    ii" << ii << "lowxd" << lowxd << "v_meni-ii" << v_meni[ii];
         }
      }
   }
DbgLv(1) << "IMS: ix_setfit" << ix_setfit;

   return;
}


// Create lists of information for noises that match a sample model from a set
void US_Analysis_auto::noises_in_edit( QString modDesc, QStringList& nieDescs,
				       QStringList& nieIDs, QStringList& nieFnams, QString invID )
{
   QString msetBase = modDesc.section( ".", 0, -3 ) + "." + 
                      modDesc.section( ".", -2, -2 ).section( "_", 0, 3 );
   QString srchTlab = msetBase.section( ".", -2, -2 );
   QString srchTrip = srchTlab.left( 1 ) + "." + srchTlab.mid( 1, 1 ) + "." +
                      srchTlab.mid( 2 ) + ".xml";
   QString srchRun  = msetBase.section( ".", 0, -3 ) + "." +
                      msetBase.section( ".", -1, -1 )
                      .section( "_", 0, 0 ).mid( 1 );
DbgLv(1) << "NIE: msetBase" << msetBase;
   if ( msetBase.contains( "2DSA-FM-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FM-IT", "2DSA-FM" );
   }
   if ( msetBase.contains( "2DSA-FB-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FB-IT", "2DSA-FB" );
   }
   if ( msetBase.contains( "2DSA-FMB-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FMB-IT", "2DSA-FMB" );
   }
DbgLv(1) << "NIE: msetBase" << msetBase;
   QStringList query;
   QString fname;
   QString noiID;
   int nlnois   = 0;
   bool db_upd  = true;
//*DEBUG*
//db_upd=false;
//*DEBUG*

   QStringList noifilt;
   noifilt << "N*.xml";
   QString     noidir   = US_Settings::dataDir() + "/noises";
   QStringList noifiles = QDir( noidir ).entryList(
         noifilt, QDir::Files, QDir::Name );
   noidir               = noidir + "/";
DbgLv(1) << "NIE: noise-files-size" << noifiles.size();

   for ( int ii = 0; ii < noifiles.size(); ii++ )
   {
      QString noiFname   = noifiles.at( ii );
      QString noiPath    = noidir + noiFname;
      US_Noise noise;
      
      if ( noise.load( noiPath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiDesc" << ii << noiDesc;

      if ( ! noiDesc.startsWith( msetBase ) )
         continue;    // Can't use if not from the model set

      nlnois++;

      nieDescs << noiDesc;
      nieFnams << noiPath;
DbgLv(1) << "NIE:     noiFname" << noiFname;

      if ( ! db_upd )
         nieIDs   << "-1";
   }

   if ( db_upd )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList nIDs;
      
      //QString invID = QString::number( US_Settings::us_inv_ID() );

      QStringList edtIDs;
      QStringList edtNams;

      query.clear();
      query << "all_editedDataIDs" << invID;
      db.query( query );

      while( db.next() )
      {
         QString edtID    = db.value( 0 ).toString();
         QString edtName  = db.value( 2 ).toString();

         if ( edtName.startsWith( srchRun )  &&
              edtName.endsWith(   srchTrip ) )
         {
            edtIDs  << edtID;
            edtNams << edtName;
DbgLv(1) << "NIE:  edtID edtName" << edtID << edtName;
         }
      }
DbgLv(1) << "NIE: edtIDs-size" << edtIDs.size();
if ( edtIDs.size() > 0 ) DbgLv(1) << "NIE: edtName0" << edtNams[0];

      query.clear();

      if ( edtIDs.size() == 1 )
         query << "get_noise_desc_by_editID" << invID << edtIDs[ 0 ];

      else
         query << "get_noise_desc" << invID;

      db.query( query );

      while( db.next() )
      {
         QString noiID    = db.value( 0 ).toString();
         QString edtID    = db.value( 2 ).toString();
DbgLv(1) << "NIE:  noiID edtID" << noiID << edtID;

         if ( edtIDs.contains( edtID ) )
            nIDs << noiID;
      }
DbgLv(1) << "NIE: nIDs-size" << nIDs.size() << "msetBase" << msetBase;

      for ( int ii = 0; ii < nIDs.size(); ii++ )
      {
         QString noiID      = nIDs[ ii ];
         US_Noise noise;
      
         if ( noise.load( noiID, &db ) != US_DB2::OK )
            continue;    // Can't use if can't load

         QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiID noiDesc" << ii << noiID << noiDesc;

         if ( ! noiDesc.startsWith( msetBase ) )
            continue;    // Can't use if not from the model set

         nieIDs   << noiID;

         if ( nlnois == 0 )
         {
            nieFnams << "";
            nieDescs << noiDesc;
         }
      }
   }
DbgLv(1) << "NIE: db_upd" << db_upd << "nlnois" << nlnois
 << "nieDescs-size" << nieDescs.size() << "nieIDs-size" << nieIDs.size();

   return;
}
