#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>
//#include <QThread>    


#include "us_audit_trail_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_protocol_util.h"
#include "us_run_protocol.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_tar.h"

#define MIN_NTC 25

// Constructor
US_auditTrailGMP::US_auditTrailGMP() : US_Widgets()
{
  setWindowTitle( tr( "GMP Audit Trail"));
  //setPalette( US_GuiSettings::frameColor() );
  setPalette( US_GuiSettings::normalColor() );

  // primary layout
  topLayout_auto  = new QVBoxLayout( this );
  topLayout_auto->setSpacing        ( 0 );
  topLayout_auto->setContentsMargins( 0, 0, 0, 0 );

  //set to NULL internal layouts
  loadedRunGrid         = NULL;
  eSignersGrid          = NULL;
  userInteractions      = NULL;
  lowerButtons          = NULL;

  QLabel* bn_loadGMPReport = us_banner( tr( "Select GMP Report for Audit Trail:" ), 1 );
  QFontMetrics m (bn_loadGMPReport -> font()) ;
  int RowHeight = m.lineSpacing() ;
  bn_loadGMPReport -> setFixedHeight  (1.5 * RowHeight);
  
  pb_loadreport_db  =  us_pushbutton( tr( "Load GMP Run" ) );
  
  topLayout_auto -> addWidget(  bn_loadGMPReport );
  topLayout_auto -> addWidget(  pb_loadreport_db );
  // int ihgt        = pb_loadreport_db ->height();
  // QSpacerItem* spacer2 = new QSpacerItem( 20, 1*ihgt, QSizePolicy::Expanding);
  // topLayout_auto->addItem( spacer2 );
  
  connect( pb_loadreport_db,  SIGNAL( clicked() ), SLOT ( loadGMPReport() ) );
  
  //resize( 1200, 300 );
  resize( 600, 500 );
}

//Load GMP Run
void US_auditTrailGMP::loadGMPReport( void )
{
  US_Passwd pw;
  US_DB2 db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db.lastError() );
      return;
    }
  
  list_all_gmp_reports_db( gmpReportsDBdata, &db );

  QString pdtitle( tr( "Select GMP Report for Audit Trail" ) );
  QStringList hdrs;
  int         prx;
  
  hdrs << "ID"
       << "Run Name"
    //<< "Protocol Name"
       << "Created"
       << "Filename (.pdf)"
       << "GMP Run ID";
         
  QString autoflow_btn = "AUTOFLOW_GMP_REPORT";

  pdiag_autoflow_db = new US_SelectItem( gmpReportsDBdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );

  QString gmpReport_id_selected("");
  QString gmpReport_runname_selected("");
  QString gmpReport_runname_selected_c("");
  QString gmpReport_filename_pdf ("");
        
  if ( pdiag_autoflow_db->exec() == QDialog::Accepted )
    {
      gmpReport_id_selected        = gmpReportsDBdata[ prx ][ 0 ];
      gmpReport_runname_selected_c = gmpReportsDBdata[ prx ][ 1 ];
      gmpReport_filename_pdf       = gmpReportsDBdata[ prx ][ 3 ];
      gmpRunID_eSign               = gmpReportsDBdata[ prx ][ 4 ];

    }
  else
    return;
  
  //read 'data' .tar.gz for autoflowGMPReport record:
  if ( gmpReport_runname_selected_c.  contains("combined") )
    {
      gmpReport_runname_selected = gmpReport_runname_selected_c.split("(")[0];
      gmpReport_runname_selected. simplified();
    }
  else
    gmpReport_runname_selected = gmpReport_runname_selected_c;


  QMap< QString, QString> p_details;
  p_details[ "autoflowID" ] = gmpRunID_eSign;
  QStringList qry;
  qry << "get_autoflowStatus_id" << gmpRunID_eSign;
  qDebug() << "In load(), qry -- " << qry;
  autoflowStatusID = db.functionQuery( qry );

  qDebug() << "autoflowStatusID: " << autoflowStatusID;
  
  p_details[ "gmp_runname" ] = gmpReport_runname_selected_c;
  initPanel_auto( p_details );
}

// Get .pdf GMP reports with assigned reviewers:
int US_auditTrailGMP::list_all_gmp_reports_db( QList< QStringList >& gmpReportsDBdata, US_DB2* db)
{
  int nrecs        = 0;   
  gmpReportsDBdata.clear();

  QStringList qry;
  qry <<  QString( "get_user_info" );
  db -> query( qry );
  db -> next();
  int u_ID        = db->value( 0 ).toInt();
  QString u_fname = db->value( 1 ).toString();
  QString u_lname = db->value( 2 ).toString();

  QString logged_user =  QString::number(u_ID) + ". " + u_lname + ", " + u_fname;

  //
  qry.clear();
  qry << "get_autoflowGMPReport_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList gmpreportentry;
      QString id                     = db->value( 0 ).toString();
      QString autoflowHistoryID      = db->value( 1 ).toString();
      QString autoflowHistoryName    = db->value( 2 ).toString();
      QString protocolName           = db->value( 3 ).toString();
      QDateTime time_created         = db->value( 4 ).toDateTime().toUTC();
      QString filenamePdf            = db->value( 5 ).toString();

      //check if report has assigned operator(s) & reviewer(s)
      QMap< QString, QString > eSign = read_autoflowGMPReportEsign_record( autoflowHistoryID );
      QString operatorListJson  = eSign[ "operatorListJson" ];
      QString reviewersListJson = eSign[ "reviewersListJson" ];
      QString approversListJson = eSign[ "approversListJson" ];
      QString eSignStatusJson   = eSign[ "eSignStatusJson" ];
    
      qDebug() << "In listing GMP Reports with assigned reviewers -- ";
      qDebug() << "logged_user, operatorListJson, reviewersListJson, approversListJson -- "
	       << logged_user << ", "
	       << operatorListJson << ",   "
	       << reviewersListJson << ", "
	       << approversListJson;

      QJsonDocument jsonDocOperList = QJsonDocument::fromJson( operatorListJson .toUtf8() );
      QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( reviewersListJson.toUtf8() );
      QJsonDocument jsonDocApprList = QJsonDocument::fromJson( approversListJson .toUtf8() );
  
      if ( jsonDocRevList. isArray() && jsonDocOperList. isArray()
      	   && !operatorListJson.isEmpty() && !reviewersListJson.isEmpty() )
      	{
	  gmpreportentry << id << autoflowHistoryName // << protocolName
			 << time_created.toString()
			 << filenamePdf
			 << autoflowHistoryID;

	  // //additionally filter by userID for {operator, reviewer, appr.} in case of SEPARATE const.
	  // if ( operatorListJson.contains( logged_user ) ||
	  //      	   reviewersListJson.contains( logged_user ) ||
	  //      	   approversListJson.contains( logged_user )  )
	  // 	gmpReportsDBdata << gmpreportentry;
	  //   }
	  // else
	  //   gmpReportsDBdata << gmpreportentry;

	  
	  gmpReportsDBdata << gmpreportentry;
	  nrecs++;
	}
    }

  return nrecs;
}


//read eSign GMP record for assigned oper(s) && rev(s) && status
QMap< QString, QString> US_auditTrailGMP::read_autoflowGMPReportEsign_record( QString aID)
{
  QMap< QString, QString> eSign_record;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return eSign_record;
    }

  QStringList qry;
  qry << "get_gmp_review_info_by_autoflowID" << aID;
  qDebug() << "read eSing rec, qry -- " << qry;
  
  db->query( qry );

  if ( db->lastErrno() == US_DB2::OK )      // e-Sign record exists
    {
      while ( db->next() )
	{
	  eSign_record[ "ID" ]                   = db->value( 0 ).toString(); 
	  eSign_record[ "autoflowID" ]           = db->value( 1 ).toString();
	  eSign_record[ "autoflowName" ]         = db->value( 2 ).toString();
	  eSign_record[ "operatorListJson" ]     = db->value( 3 ).toString();
	  eSign_record[ "reviewersListJson" ]    = db->value( 4 ).toString();
	  eSign_record[ "eSignStatusJson" ]      = db->value( 5 ).toString();
	  eSign_record[ "eSignStatusAll" ]       = db->value( 6 ).toString();
	  eSign_record[ "createUpdateLogJson" ]  = db->value( 7 ).toString();
	  eSign_record[ "approversListJson" ]    = db->value( 8 ).toString();
	  eSign_record[ "smeListJson" ]          = db->value( 9 ).toString();
	                
	  eSign_record[ "isEsignRecord" ]        = QString("YES");
	  isEsignRecord = true;
	}
    }
  else
    {
      //No record, so no oper/revs assigned!
      qDebug() << "No e-Sign GMP record exists!!";

      eSign_record[ "isEsignRecord" ]        = QString("NO");
      isEsignRecord = false;
      eSign_record. clear();

      // //TEST ---------------------------
      // eSign_record[ "operatorListJson" ]  = QString( tr( "[\"Operator 1\",\"Operator 2\",\"Operator 3\"]" ));
      // eSign_record[ "reviewersListJson" ] = QString( tr( "[\"Reviewer 1\",\"Reviewer 2\",\"Reviewer 3\"]" ));
      // //END TEST ------------------------
    }

  return eSign_record;
}

//slot to..
void US_auditTrailGMP::initPanel_auto( QMap < QString, QString > & protocol_details )
{
  //Main ID for parent GMP run:
  autoflowID_passed = protocol_details[ "autoflowID" ];
  
  //clear all GUI, internals
  reset_panel();

  qDebug() << "After reset...";
    
  //0. Loaded Run
  loadedRunGrid     = new QGridLayout();
  loadedRunGrid ->setSpacing        ( 2 );
  loadedRunGrid ->setContentsMargins( 1, 1, 1, 1 );

  int row = 0;
  QLabel*      lb_runloaded  = us_banner( tr( "Loaded GMP Run:" ), 1 );
  QLabel*      lb_runName    = us_label(  tr( "GMP Run Name:" ), 1  );
  QLineEdit*   le_runName    = us_lineedit( protocol_details["gmp_runname"], 0, true );
  
  loadedRunGrid  -> addWidget( lb_runloaded,    row++,   0,  1,  10  );
  loadedRunGrid  -> addWidget( lb_runName,      row,     0,  1,  3  );
  loadedRunGrid  -> addWidget( le_runName,      row++,   3,  1,  7  );

  //1. e_signers layout
  eSignersGrid     = new QGridLayout();
  eSignersGrid ->setSpacing        ( 2 );
  eSignersGrid ->setContentsMargins( 1, 1, 1, 1 );
  
  //eSigners Info Tree
  QLabel*      lb_esigners  = us_banner(      tr( "e-Signers Information:" ), 1 );
  eSignTree = new QTreeWidget();
  QStringList esigns_theads;
  esigns_theads << "Selected" << "Details";
  eSignTree->setHeaderLabels( esigns_theads );
  eSignTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  eSignTree->setFont( QFont( US_Widgets::fixedFont().family(),
			     US_GuiSettings::fontSize() + 1 ) );
  eSignersGrid->addWidget( lb_esigners );
  eSignersGrid->addWidget( eSignTree );

  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  QFontMetrics fmet( sfont );
  
  eSignTree  ->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(2*sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  QTreeWidgetItem*  esigntItem = new QTreeWidgetItem();
  QString esignItemName = "e-Sign STATUS";
  esigntItem -> setText( 0, esignItemName );
  esigntItem -> setFont(0, sfont);
  eSignTree  -> addTopLevelItem( esigntItem );

  QGroupBox *groupBox_esign = createGroup_eSign( "" );
  
  QTreeWidgetItem* eSignItem_childItem = new QTreeWidgetItem();
  esigntItem -> addChild( eSignItem_childItem );
  eSignTree  -> setItemWidget( eSignItem_childItem, 1, groupBox_esign );

  int max_width = fmet.width( esignItemName );
  max_width *= 2;
  eSignTree->header()->resizeSection(0, max_width );
  
  //2. User Interations layout
  userInteractions     = new QGridLayout();
  userInteractions ->setSpacing        ( 2 );
  userInteractions ->setContentsMargins( 1, 1, 1, 1 );

  //user Intaracitons Info Tree
  QLabel*      lb_uints  = us_banner(      tr( "User INPUTS:" ), 1 );
  uInteractionsTree = new QTreeWidget();
  QStringList uints_theads;
  uints_theads << "Selected" << "Details";
  uInteractionsTree->setHeaderLabels( uints_theads );
  uInteractionsTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  uInteractionsTree->setFont( QFont( US_Widgets::fixedFont().family(),
			     US_GuiSettings::fontSize() + 1 ) );
  userInteractions->addWidget( lb_uints );
  userInteractions->addWidget( uInteractionsTree );
  
  uInteractionsTree  ->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(2*sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //Stages Interactions:
  QStringList stages_i;
  stages_i << "GMP Run Initiation"
	   << "LIVE UPDATE"
	   << "IMPORT"
	   << "EDITING"
	   << "ANALYSIS";

  for ( int i=0; i< stages_i.size(); ++i )
    {
      QTreeWidgetItem*  uintsItem = new QTreeWidgetItem();
      QString uintsItemName = stages_i[i];
      uintsItem -> setText( 0, uintsItemName );
      uintsItem -> setFont(0, sfont);
      uInteractionsTree  -> addTopLevelItem( uintsItem );

      QGroupBox *groupBox_stages = createGroup_stages( "", uintsItemName );
      
      QTreeWidgetItem* Item_childItem = new QTreeWidgetItem();
      uintsItem -> addChild( Item_childItem );
      uInteractionsTree  -> setItemWidget( Item_childItem, 1, groupBox_stages );
    }
  uInteractionsTree->header()->resizeSection(0, max_width );
  
  

  //assemble
  topLayout_auto  -> addLayout( loadedRunGrid );
  topLayout_auto  -> addLayout( eSignersGrid );
  topLayout_auto  -> addLayout( userInteractions );

  //copy layouts
  gridLayoutVector. clear();
  gridLayoutVector.push_back( loadedRunGrid );
  gridLayoutVector.push_back( eSignersGrid );
  gridLayoutVector.push_back( userInteractions );
  //gridLayoutVector.push_back( lowerButtons );

  qDebug() << "Resizing trees: ";
  //eSignTree->setMinimumHeight( (eSignTree->height())*0.8 );
  uInteractionsTree->setMinimumHeight( (uInteractionsTree->height())*1.125 );
  
  resize( 1200, 1000 );
}

//create groupBox: eSigners
QGroupBox * US_auditTrailGMP::createGroup_eSign( QString name )
{
  QGroupBox *groupBox = new QGroupBox ( name );

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
  QLabel* lb_name    = us_label( tr("Name:") );
  QLabel* lb_role    = us_label( tr("Role:") );
  QLabel* lb_date    = us_label( tr("Date:") );
  QLabel* lb_comment = us_label( tr("Comment:") );
  QLabel* lb_status  = us_label( tr("Status:") );
 
  genL -> addWidget( lb_name,      row,     0,  1,  3  );
  genL -> addWidget( lb_role,      row,     3,  1,  2  );
  genL -> addWidget( lb_date,      row,     5,  1,  2  );
  genL -> addWidget( lb_comment,   row,     7,  1,  3  );
  genL -> addWidget( lb_status,    row++,   10, 1,  2  );

  //read e-Sign record, to check e-Signing status of each reviewer/operator:
  eSign_details_auto. clear();
  eSign_details_auto = read_autoflowGMPReportEsign_record( autoflowID_passed );
  
  //&& Set defined Operator/Reviewers (if any)
  display_reviewers_auto( row, eSign_details_auto, "operatorListJson",  genL );
  display_reviewers_auto( row, eSign_details_auto, "reviewersListJson", genL );
  display_reviewers_auto( row, eSign_details_auto, "approversListJson", genL ); 
  
  groupBox->setLayout(genL);

  return groupBox;
}


//create groupBox: autoflow stages
QGroupBox * US_auditTrailGMP::createGroup_stages( QString name, QString s_name )
{
  QGroupBox *groupBox = new QGroupBox ( name );

  QPalette p = groupBox->palette();
  p.setColor(QPalette::Dark, Qt::white);
  groupBox->setPalette(p);

  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");

  
  groupBox->setFlat(true);

  //GUI
  QHBoxLayout* genL   = new QHBoxLayout();
  genL->setSpacing        ( 2 );
  genL->setContentsMargins( 20, 10, 20, 15 );

  int row;
 
  //read autoflowStatus record:
  //Maps && timestamps from DB
  //Create GMP Run
  QString createdGMPrunJson, createdGMPrunts;
  //LIVE_UPDATE
  QMap < QString, QString > operation_types_live_update;
  QMap < QString, QString > operation_types_live_update_ts;
  QString stopOptimaJson, stopOptimats, skipOptimaJson, skipOptimats;
  // IMPORT
  QMap < QString, QString > data_types_import;
  QMap < QString, QString > data_types_import_ts;
  QString importRIJson, importIPJson, importRIts, importIPts;
  //EDITING & ANALYSIS
  QMap < QString, QString > data_types_edit;
  QMap < QString, QString > data_types_edit_ts;
  QString editRIJson, editIPJson, editRIts, editIPts, analysisJson, analysisCancelJson;

  read_autoflowStatus_record( importRIJson, importRIts, importIPJson, importIPts,
			      editRIJson, editRIts, editIPJson, editIPts, analysisJson,
			      stopOptimaJson, stopOptimats, skipOptimaJson, skipOptimats,
			      analysisCancelJson, createdGMPrunJson, createdGMPrunts ); 

  QMap< QString, QMap < QString, QString > > status_map;
  
  if ( s_name == "GMP Run Initiation" )
    {
      status_map = parse_autoflowStatus_json( createdGMPrunJson, "" );

      //Person
      QLabel* lb_init         = us_label( tr("Initiated by:") );
      QLabel* lb_ID           = us_label( tr("User ID:") );
      QLabel* lb_name         = us_label( tr("Name:") );
      QLabel* lb_email        = us_label( tr("E-mail:") );
      QLabel* lb_level        = us_label( tr("User Level:") );
      QLineEdit* le_ID        = us_lineedit( status_map[ "Person" ][ "ID"], 0, true);
      QLineEdit* le_name      = us_lineedit( status_map[ "Person" ][ "lname" ] + "," + status_map[ "Person" ][ "fname"], 0, true);
      QLineEdit* le_email     = us_lineedit( status_map[ "Person" ][ "email" ], 0, true);
      QLineEdit* le_level     = us_lineedit( status_map[ "Person" ][ "level" ], 0, true);

      QGridLayout* genL1  = new QGridLayout();
      QVBoxLayout* genL11 = new QVBoxLayout();
      
      row=0;
      genL1 -> addWidget( lb_init,      row++,   0,  1,  6  );
      genL1 -> addWidget( lb_ID,        row,     1,  1,  2  );
      genL1 -> addWidget( le_ID,        row++,   3,  1,  3  );
      genL1 -> addWidget( lb_name,      row,     1,  1,  2  );
      genL1 -> addWidget( le_name,      row++,   3,  1,  3  );
      genL1 -> addWidget( lb_email,     row,     1,  1,  2  );
      genL1 -> addWidget( le_email,     row++,   3,  1,  3  );
      genL1 -> addWidget( lb_level,     row,     1,  1,  2  );
      genL1 -> addWidget( le_level,     row++,   3,  1,  3  );

      genL11 -> addLayout( genL1);
      genL11 -> addStretch();

      //TimeStamp
      QGridLayout* genL2 = new QGridLayout();
      QVBoxLayout* genL21 = new QVBoxLayout();
      
      QLabel* lb_time         = us_label( tr("Initiation Time:") );
      lb_time->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      QLabel* lb_time1        = us_label( tr("Initiated on:") );
      QLineEdit* le_time1     = us_lineedit( createdGMPrunts, 0, true );

      row=0;
      genL2 -> addWidget( lb_time,      row++,   0,  1,  6  );
      genL2 -> addWidget( lb_time1,     row,     1,  1,  2  );
      genL2 -> addWidget( le_time1,     row++,   3,  1,  3  );

      genL21 -> addLayout( genL2);
      genL21 -> addStretch();
      
      // int ihgt        = lb_time1->height();
      // QSpacerItem* spacer2 = new QSpacerItem( 20, 3*ihgt, QSizePolicy::Expanding);
      genL2->setRowStretch( 1, 1 );
      // genL2->addItem( spacer2,  row++,  0, 1, 6 );

      //Comment
      QGridLayout* genL3 = new QGridLayout();
      QVBoxLayout* genL31 = new QVBoxLayout();
      
      QLabel* lb_comm         = us_label( tr("Comment at Initiation:") );
      lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      QLabel* lb_comm1        = us_label( tr("Comment:") );
      QLineEdit* le_comm1     = us_lineedit( status_map[ "Comment" ][ "comment" ], 0, true );

      row=0;
      genL3 -> addWidget( lb_comm,      row++,   0,  1,  6  );
      genL3 -> addWidget( lb_comm1,     row,     1,  1,  2  );
      genL3 -> addWidget( le_comm1,     row++,   3,  1,  3  );

      genL31 -> addLayout( genL3);
      genL31 -> addStretch();
    
      //assemble
      genL->addLayout( genL11);
      genL->addLayout( genL21);
      genL->addLayout( genL31);
    }
  else
    {
      QLabel* lb_dev         = us_label( tr("Under Development:") );
      genL->addWidget( lb_dev );
    }
  
  groupBox->setLayout(genL);

  return groupBox;
}

//read autoflowStatus, populate internals
void US_auditTrailGMP::read_autoflowStatus_record( QString& importRIJson, QString& importRIts, QString& importIPJson, QString& importIPts,
						   QString& editRIJson, QString& editRIts, QString& editIPJson, QString& editIPts, QString& analysisJson,
						   QString& stopOptimaJson, QString& stopOptimats, QString& skipOptimaJson, QString& skipOptimats,
						   QString& analysisCancelJson, QString& createdGMPrunJson, QString& createdGMPrunts  )
{
  importRIJson.clear();
  importRIts  .clear();
  importIPJson.clear();
  importIPts  .clear();
  editRIJson  .clear();
  editRIts    .clear();
  editIPJson  .clear();
  editIPts    .clear();
  analysisJson.clear();
  stopOptimaJson. clear();
  stopOptimats  . clear();
  skipOptimaJson. clear();
  skipOptimats  . clear();
  analysisCancelJson. clear();
  createdGMPrunJson .clear();
  createdGMPrunts   .clear();

  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem: Read autoflowStatus" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }

  QStringList qry;
  qry.clear();
  qry << "read_autoflow_status_record" << QString::number( autoflowStatusID );
  qDebug() << "In read_autoflowStatus_record(), qry -- " << qry;
  
  db.query( qry );

  if ( db.lastErrno() == US_DB2::OK )    
    {
      while ( db.next() )
	{
	  importRIJson  = db.value( 0 ).toString();
	  importRIts    = db.value( 1 ).toString();
	  importIPJson  = db.value( 2 ).toString();
	  importIPts    = db.value( 3 ).toString();

	  editRIJson    = db.value( 4 ).toString();
	  editRIts      = db.value( 5 ).toString();
	  editIPJson    = db.value( 6 ).toString();
	  editIPts      = db.value( 7 ).toString();

	  analysisJson  = db.value( 8 ).toString();

	  stopOptimaJson = db.value( 9 ).toString();
	  stopOptimats   = db.value( 10 ).toString();

	  skipOptimaJson = db.value( 11 ).toString();
	  skipOptimats   = db.value( 12 ).toString();

	  analysisCancelJson = db.value( 13 ).toString();

	  createdGMPrunJson = db.value( 14 ).toString();
	  createdGMPrunts   = db.value( 15 ).toString();
	}
    }

  qDebug() << "Read_autoflow_status: stopOptimaJson, skipOptimaJson -- "
	   << stopOptimaJson
	   << skipOptimaJson;
}


//Parse autoflowStatus RI/IP Json
QMap< QString, QMap < QString, QString > > US_auditTrailGMP::parse_autoflowStatus_json( QString statusJson, QString dtype )
{
  QMap< QString, QMap <QString, QString> > status_map;

  QJsonDocument jsonDoc = QJsonDocument::fromJson( statusJson.toUtf8() );
  QJsonObject json_obj = jsonDoc.object();
  
  foreach(const QString& key, json_obj.keys())
    {
      QJsonValue value = json_obj.value(key);
      
      qDebug() << "statusJson key, value: " << key << value;
      
      if ( key == "Person" )  //GMP init || live_update || import || edit
	{	  
	  QJsonArray json_array = value.toArray();
	  QMap< QString, QString > person_map;
	  
	  for (int i=0; i < json_array.size(); ++i )
	    {
	      foreach(const QString& array_key, json_array[i].toObject().keys())
		{
		  person_map[ array_key ] = json_array[i].toObject().value(array_key).toString();
		  qDebug() << "Person Map: -- key, value: "
			   << array_key
			   << json_array[i].toObject().value(array_key).toString();
		}
	    }

	  status_map[ key ] = person_map;
	}

      if ( key == "RefScan" )    //import: Ref. scan
	{
	  status_map[ key ][ "type" ] = value.toString();
	}

      //Comment when SAVED:
      if ( key == "Comment when SAVED" )    //import || edit
	{
	  status_map[ key ][ "comment_when_saved" ] = value.toString();
	}
      
      
      if ( key == "Dropped" )   // import: Dropped triples/channels/select channels operaitons 
	{	  
	  QJsonArray json_array = value.toArray();
	  QMap< QString, QString > dropped_map;
	  
	  for (int i=0; i < json_array.size(); ++i )
	    {
	      foreach(const QString& array_key, json_array[i].toObject().keys())
		{
		  dropped_map[ array_key ] = json_array[i].toObject().value(array_key).toString();
		  qDebug() << "Dropped Map: -- key, value: "
			   << array_key
			   << json_array[i].toObject().value(array_key).toString();
		}
	    }

	  status_map[ key ] = dropped_map;
	}
      

      if ( key == "Meniscus" )   //edit  
	{	  
	  QJsonArray json_array = value.toArray();
	  QMap< QString, QString > meniscus_map;
	  
	  for (int i=0; i < json_array.size(); ++i )
	    {
	      foreach(const QString& array_key, json_array[i].toObject().keys())
		{
		  meniscus_map[ array_key ] = json_array[i].toObject().value(array_key).toString();
		  qDebug() << "Meniscus Map: -- key, value: "
			   << array_key
			   << json_array[i].toObject().value(array_key).toString();
		}
	    }

	  status_map[ key ] = meniscus_map;
	}

      if ( key == "Remote Operation" )  //Live Update's remote operations [SKIP | STOP]
	{
	  status_map[ key ][ "type" ] = value.toString();
	}
      if ( key == "Comment" )           //GMP init || Live Update's remote operations [SKIP | STOP]
	{
	  status_map[ key ][ "comment" ] = value.toString();
	}
      
    }
  
  return status_map;
}


//display oper/revs/appr.
void US_auditTrailGMP::display_reviewers_auto( int& row, QMap< QString, QString> eSign_d, QString JsonListName, QGridLayout* genL )
{
  if ( eSign_d. contains( JsonListName ) )
    {
      QJsonDocument jsonDoc_signed = QJsonDocument::fromJson( eSign_d[ "eSignStatusJson" ].toUtf8() );
      if (!jsonDoc_signed.isObject())
	{
	  qDebug() << "json_to_qmap in display_reviewers_aut(): eSignStatusJson: NOT a JSON Doc !!";
	  return;
	}
      
      //const QJsonValue &to_esign = jsonDoc.object().value("to_sign");
      const QJsonValue &esigned  = jsonDoc_signed.object().value("signed");
      QJsonArray esigned_array   = esigned  .toArray();
      
      QJsonDocument jsonDocOperList = QJsonDocument::fromJson( eSign_d[ JsonListName ] .toUtf8() );
      QJsonArray jsonDocOperList_array  = jsonDocOperList.array();
      for (int i=0; i < jsonDocOperList_array.size(); ++i )
	{
	  QString current_reviewer = jsonDocOperList_array[i].toString();
	  //uname =  oID + ": " + olname + ", " + ofname;
	  
	  QLineEdit* le_name = us_lineedit( current_reviewer, 0, true );
	  le_name -> setObjectName( "name: " + current_reviewer );
	  QString u_role;
	  if ( JsonListName. contains("operator") )
	    u_role = "Operator";
	  else if ( JsonListName. contains("reviewer") )
	    u_role = "Reviewer";
	  else if ( JsonListName. contains("approver") )
	    u_role = "Approver";
	      
	  QLineEdit* le_role = us_lineedit( u_role, 0, true );

	  //TimeDate && Comment
	  QString e_date, e_comment;
	  for (int i=0; i < esigned_array.size(); ++i )
	    {
	      foreach(const QString& key, esigned_array[i].toObject().keys())
		{
		  if ( key == current_reviewer )
		    {
		      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
		      
		      e_comment  = newObj["Comment"]   .toString();
		      e_date     = newObj["timeDate"]  .toString();
		     		      
		      qDebug() << "E-Signed - " << key << ": Comment, timeDate -- "
			       << newObj["Comment"]   .toString()
			       << newObj["timeDate"]  .toString();
		      
		      break;
		    }
		}
	    }
	  QLineEdit* le_date    = us_lineedit( e_date, 0, true );
	  QLineEdit* le_comment = us_lineedit( e_comment, 0, true );
	  
	  QLineEdit* le_stat = check_eSign_status_for_gmpReport_auto( current_reviewer, eSign_d ); 
	  le_stat -> setObjectName( "status: " + current_reviewer );

	  qDebug() << "Object Name of le_stat -- " << le_stat->objectName();
	  
	  genL -> addWidget( le_name,      row,      0,  1,  3 );
	  genL -> addWidget( le_role,      row,      3,  1,  2 );
	  genL -> addWidget( le_date,      row,      5,  1,  2 );
	  genL -> addWidget( le_comment,   row,      7,  1,  3 );
	  genL -> addWidget( le_stat,      row++,    10, 1,  2 );
	}
    }
}

//Check eSign status for GMP Report fior particular reviewer:
QLineEdit* US_auditTrailGMP::check_eSign_status_for_gmpReport_auto( QString u_passed, QMap <QString, QString > eSign_stats )
{
  QLineEdit* le_stat = us_lineedit( "", 0, true );
  //QPalette *new_palette = new QPalette();
      
  QString eSignStatusJson   = eSign_stats[ "eSignStatusJson" ];
  QString eSignStatusAll    = eSign_stats[ "eSignStatusAll" ];

  qDebug() << "In check_eSign_status_for_gmpReport_auto(): eSignStatusJson, eSignStatusAll -- "
	   << eSignStatusJson << eSignStatusAll;
    
    
  QJsonDocument jsonDocEsign = QJsonDocument::fromJson( eSignStatusJson.toUtf8() );
  if (!jsonDocEsign.isObject())
    {
      qDebug() << "to_eSign(): ERROR: eSignStatusJson: NOT a JSON Doc !!";
      return le_stat;
    }
  
  const QJsonValue &to_esign = jsonDocEsign.object().value("to_sign");
  const QJsonValue &esigned  = jsonDocEsign.object().value("signed");

  QJsonArray to_esign_array  = to_esign .toArray();
  QJsonArray esigned_array   = esigned  .toArray();


  //to_sign:
  if ( to_esign.isUndefined() || to_esign_array.size() == 0
       || !to_esign_array.size() || eSignStatusAll == "YES" )
    {
      qDebug() << "check_eSign_status(): All signatures have been collected; none left to e-sign !!";

      le_stat -> setText( QString("SIGNED") );
      le_stat -> setStyleSheet( "QLineEdit { background-color:  rgb(50, 205, 50); }"); //green
      // new_palette->setColor(QPalette::Base, Qt::darkGreen);
      // new_palette->setColor(QPalette::Text, Qt::black);
      // le_stat->setPalette(*new_palette);

      return le_stat;
    }

  //signed:
  if ( esigned.isUndefined() || esigned_array.size() == 0 || !esigned_array.size() )
    {
      qDebug() << "check_eSign_Status(): Nothing has been e-Signed yet !!!";

      le_stat -> setText( QString("NOT SIGNED") );
      le_stat -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); }"); //red
      
      // new_palette->setColor(QPalette::Base, Qt::red);
      // new_palette->setColor(QPalette::Text, Qt::black);
      // le_stat->setPalette(*new_palette);
      
      return le_stat;
    }
  else
    {
      qDebug() << "check_eSign_status(): Some parties have e-Signed already !!!";
      //DEBUG
      QStringList eSignees_current;
      for (int i=0; i < esigned_array.size(); ++i )
	{
	  foreach(const QString& key, esigned_array[i].toObject().keys())
	    {
	      QJsonObject newObj = esigned_array[i].toObject().value(key).toObject();
	      
	      qDebug() << "E-Signed - " << key << ": Comment, timeDate -- "
		       << newObj["Comment"]   .toString()
		       << newObj["timeDate"]  .toString();

	      QString current_reviewer = key;
	      QString current_reviewer_id = current_reviewer. section( ".", 0, 0 );

	      eSignees_current << key;
	      
	    }
	}
      //END DEBUG:
      qDebug() << "check_eSign_status(): so far, e-signed by: " << eSignees_current;
      if ( eSignees_current.contains( u_passed ) )
	{
	  le_stat -> setText( QString("SIGNED") );
	  le_stat -> setStyleSheet( "QLineEdit { background-color:  rgb(50, 205, 50); }"); //green
	  // new_palette->setColor(QPalette::Base, Qt::darkGreen);
	  // new_palette->setColor(QPalette::Text, Qt::black);
	  // le_stat->setPalette(*new_palette);
	  return le_stat;
	}
      else
	{
	  le_stat -> setText( QString("NOT SIGNED") );
	  le_stat -> setStyleSheet( "QLineEdit { background-color:  rgb(210, 0, 0); }"); //red
	  // new_palette->setColor(QPalette::Base, Qt::red);
	  // new_palette->setColor(QPalette::Text, Qt::black);
	  // le_stat->setPalette(*new_palette);
	  return  le_stat;
	}
    }
}

void US_auditTrailGMP::reset_panel( void )
{
  //reset inernals && clear GUI:
  for ( int i=0; i<gridLayoutVector.size(); ++i )
    {
      qDebug() << "In reset(): layout # -- " << i;
      QGridLayout* c_layout = gridLayoutVector[i];
      if ( c_layout != NULL && c_layout->layout() != NULL )
  	{
  	  QLayoutItem* item;
  	  while ( ( item = c_layout->layout()->takeAt( 0 ) ) != NULL )
  	    {
  	      delete item->widget();
  	      delete item;
  	    }
  	  delete c_layout;
  	}
    }

}
