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

  p_2dsa_auto_fitmen = false;
  p_2dsa_run_fitmen  = false;

  QLabel* bn_loadGMPReport = us_banner( tr( "Select GMP Report for Audit Trail:" ), 1 );
  QFontMetrics m (bn_loadGMPReport -> font()) ;
  RowHeight = m.lineSpacing() ;
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

void US_auditTrailGMP::printAPDF( void )
{
  QString subDirName = gmpRunName_passed + "_AudirTrail";
  mkdir( US_Settings::reportDir(), subDirName );
  filePath_pdf = US_Settings::reportDir() + "/" + subDirName + "/" + subDirName + ".pdf";
  
  QTextDocument document;
  document.setHtml( html_assembled );
  
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);

  printer.setOutputFileName( filePath_pdf );
  printer.setFullPage(true);
  printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
  
  document.print(&printer);

  /** END of standard way of printing *************************/

  qDebug() << "HTMP_assembled -- " << html_assembled;
}


//view report
void US_auditTrailGMP::viewAPDF ( void )
{
  qDebug() << "Opening PDF at -- " << filePath_pdf;

  QFileInfo check_file( filePath_pdf );
  if (check_file.exists() && check_file.isFile())
    {
      //Open with OS's applicaiton settings ?
      QDesktopServices::openUrl(QUrl( filePath_pdf ));
    }
  else
    {
      QMessageBox::warning( this, tr( "Error: Cannot Open .PDF File" ),
			    tr( "%1 \n\n"
				"No such file or directory...") .arg( filePath_pdf ) );
    }
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

  pdiag_autoflow_db = new US_SelectItem( gmpReportsDBdata, hdrs, pdtitle, &prx, autoflow_btn, -3 );

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
      gmpReport_runname_selected = gmpReport_runname_selected_c.split("(")[0]. simplified();
      //gmpReport_runname_selected. simplified();
    }
  else
    gmpReport_runname_selected = gmpReport_runname_selected_c;


  QMap< QString, QString> p_details = read_autoflow_record( gmpRunID_eSign.toInt() );
  p_details[ "autoflowID" ] = gmpRunID_eSign;
  // QStringList qry;
  // qry << "get_autoflowStatus_id" << gmpRunID_eSign;
  // qDebug() << "In load(), qry -- " << qry;
  // autoflowStatusID = db.functionQuery( qry );

  autoflowStatusID = p_details[ "statusID" ].toInt();
  qDebug() << "autoflowStatusID: " << autoflowStatusID;

  AProfileGUID = p_details[ "aprofileguid" ];
  qDebug() << "AProfileGUID: " << AProfileGUID;
  
  p_details[ "gmp_runname" ] = gmpReport_runname_selected;
  initPanel_auto( p_details );

  // Print PDF && enable View:
  
  printAPDF();
  pb_viewAPDF -> setEnabled( true );
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
      QString ptime_created          = US_Util::toUTCDatetimeText( time_created
								   .toString( Qt::ISODate ), true )
	                                                           .section( ":", 0, 1 ) + " UTC";
      
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
			 << ptime_created //<< time_created.toString()
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
	  eSign_record[ "isHistory" ]            = QString("NO");
	  isEsignRecord = true;
	}
    }
  else
    {
      //No record, so no oper/revs assigned!
      qDebug() << "No e-Sign GMP record exists in main table!!";
      qDebug() << "Checking History...";
      qry. clear();
      
      qry << "get_gmp_review_info_by_autoflowID_history" << aID;
      qDebug() << "read eSing rec HISTORY, qry -- " << qry;
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
	      eSign_record[ "isHistory" ]            = QString("YES");
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
	}
    }

  return eSign_record;
}

//slot to..
void US_auditTrailGMP::initPanel_auto( QMap < QString, QString > & protocol_details )
{
  //clear all GUI, internals
  reset_panel();
  
  //Main ID for parent GMP run:
  autoflowID_passed = protocol_details[ "autoflowID" ];

  //GMP Run Name
  gmpRunName_passed = protocol_details["gmp_runname"];

  //dataSource
  dataSource        = protocol_details["dataSource"];
  
  //init HTML
  initHTML();
  
  qDebug() << "After reset...";
    
  //0. Loaded Run
  loadedRunGrid     = new QGridLayout();
  loadedRunGrid ->setSpacing        ( 2 );
  loadedRunGrid ->setContentsMargins( 1, 1, 1, 1 );

  int row = 0;
  QLabel*      lb_runloaded  = us_banner( tr( "Loaded GMP Run:" ), 1 );
  QLabel*      lb_runName    = us_label(  tr( "GMP Run Name:" ), 1  );
  QLineEdit*   le_runName    = us_lineedit( gmpRunName_passed, 0, true );

  //QLabel* bn_viewAPDF = us_banner( tr( "View .PDF of the Audit Trail for the Currenlty Loaded GMP Run:" ), 1 );
  pb_viewAPDF  =  us_pushbutton( tr( "View .PDF of the Audit Trail for the Currenlty Loaded GMP Run:" ) );

  loadedRunGrid  -> addWidget( lb_runloaded,    row++,   0,  1,  10  );
  loadedRunGrid  -> addWidget( lb_runName,      row,     0,  1,  3  );
  loadedRunGrid  -> addWidget( le_runName,      row++,   3,  1,  7  );
  loadedRunGrid  -> addWidget( pb_viewAPDF,     row++,   0,  1,  10  );

  connect( pb_viewAPDF,  SIGNAL( clicked() ), SLOT ( viewAPDF() ) );

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
	   << "ANALYSIS"
	   << "E-SIGNATURES";

  for ( int i=0; i< stages_i.size(); ++i )
    {
      QTreeWidgetItem*  uintsItem = new QTreeWidgetItem();
      QString uintsItemName = stages_i[i];
      uintsItem -> setText( 0, uintsItemName );
      uintsItem -> setFont(0, sfont);
      uInteractionsTree  -> addTopLevelItem( uintsItem );

      QVector< QGroupBox *> groupBox_stages = createGroup_stages( "", uintsItemName );

      for ( int j=0; j<groupBox_stages.size(); ++j )
	{
	  QTreeWidgetItem* Item_childItem = new QTreeWidgetItem();
	  uintsItem -> addChild( Item_childItem );
	  uInteractionsTree  -> setItemWidget( Item_childItem, 1, groupBox_stages[j] );
	}
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
  uInteractionsTree->setMinimumHeight( (uInteractionsTree->height())*1.025 );

  eSignTree         ->expandAll();
  uInteractionsTree->topLevelItem(0)->setExpanded(true);

  //conclude HTML
  html_assembled += html_assembled_esigs;
  closeHTML();
  
  resize( 1400, 1000 );
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

  //init eSigs HTML
  html_assembled_esigs += tr( "<p class=\"pagebreak \">\n");
  html_assembled_esigs += tr("<h2 align=left>Electronic Signatures:</h2>" );
  
  //&& Set defined Operator/Reviewers (if any)
  display_reviewers_auto( row, eSign_details_auto, "operatorListJson",  genL );
  display_reviewers_auto( row, eSign_details_auto, "reviewersListJson", genL );
  display_reviewers_auto( row, eSign_details_auto, "approversListJson", genL );

  html_assembled_esigs += tr( "</p>" ); 
  
  groupBox->setLayout(genL);

  return groupBox;
}


//create groupBox: autoflow stages
QVector< QGroupBox *> US_auditTrailGMP::createGroup_stages( QString name, QString s_name )
{
  QVector< QGroupBox * > groupBoxes;
  
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
  QMap < QString, QString >::iterator im;
  
  if ( s_name == "GMP Run Initiation" )
    {
      status_map = parse_autoflowStatus_json( createdGMPrunJson, "" );

      //GUI
      QHBoxLayout* genL   = new QHBoxLayout();
      genL->setSpacing        ( 2 );
      genL->setContentsMargins( 20, 10, 20, 15 );

      //Person
      QLabel* lb_init         = us_label( tr("Initiated by:") );
      QLabel* lb_ID           = us_label( tr("User ID:") );
      QLabel* lb_name         = us_label( tr("Name:") );
      QLabel* lb_email        = us_label( tr("E-mail:") );
      QLabel* lb_level        = us_label( tr("Level:") );
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
      QGridLayout* genL2  = new QGridLayout();
      QVBoxLayout* genL21 = new QVBoxLayout();
      
      QLabel* lb_time         = us_label( tr("Initiation Time:") );
      lb_time->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      QLabel* lb_time1        = us_label( tr("Initiated on:") );
      QLineEdit* le_time1     = us_lineedit( createdGMPrunts + " (UTC)", 0, true );

      row=0;
      genL2 -> addWidget( lb_time,      row++,   0,  1,  6  );
      genL2 -> addWidget( lb_time1,     row,     1,  1,  2  );
      genL2 -> addWidget( le_time1,     row++,   3,  1,  3  );

      genL21 -> addLayout( genL2);
      genL21 -> addStretch();
      
      // int ihgt        = lb_time1->height();
      // QSpacerItem* spacer2 = new QSpacerItem( 20, 3*ihgt, QSizePolicy::Expanding);
      // genL2->setRowStretch( 1, 1 );
      // genL2->addItem( spacer2,  row++,  0, 1, 6 );

      //Comment
      QGridLayout* genL3  = new QGridLayout();
      QVBoxLayout* genL31 = new QVBoxLayout();
      
      QLabel* lb_comm         = us_label( tr("Comment at Initiation:") );
      lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      QLabel* lb_comm1        = us_label( tr("Comment:") );

      QTextEdit* te_comm1    = us_textedit();
      te_comm1    -> setFixedHeight  ( RowHeight * 2 );
      te_comm1    ->setFont( QFont( US_Widgets::fixedFont().family(),
				    US_GuiSettings::fontSize() - 1) );
      us_setReadOnly( te_comm1, true );
      te_comm1 -> setText( status_map[ "Comment" ][ "comment" ] );
    
      row=0;
      genL3 -> addWidget( lb_comm,      row++,   0,  1,  6  );
      genL3 -> addWidget( lb_comm1,     row,     1,  1,  2  );
      genL3 -> addWidget( te_comm1,     row++,   3,  1,  3  );

      genL31 -> addLayout( genL3);
      genL31 -> addStretch();
    
      //assemble
      genL->addLayout( genL11);
      genL->addLayout( genL21);
      genL->addLayout( genL31);

      //Set GroupBox
      QGroupBox *groupBox = new QGroupBox ( name );
      QPalette p = groupBox->palette();
      p.setColor(QPalette::Dark, Qt::white);
      groupBox->setPalette(p);
      
      groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
      
      groupBox->setFlat(true);
      
      groupBox->setLayout(genL);
      groupBoxes. push_back( groupBox );

      //assemble html:
      assemble_GMP_init( status_map, createdGMPrunts );
    }
  
  else if ( s_name == "LIVE UPDATE" )
    {
      html_assembled += tr( "<h3 align=left>Remote Stage Skipping, Stopping Machine (2. LIVE_UPDATE)</h3>" );
      
      operation_types_live_update[ "STOP" ] = stopOptimaJson;
      operation_types_live_update[ "SKIP" ] = skipOptimaJson;
      
      operation_types_live_update_ts[ "STOP" ] = stopOptimats;
      operation_types_live_update_ts[ "SKIP" ] = skipOptimats;

      if ( stopOptimaJson.isEmpty() && skipOptimaJson.isEmpty() )
	{
	  html_assembled += tr( "<table>" );
	  html_assembled += tr( "<tr><td> There were NO remote operations. </td></tr>" );
	  html_assembled += tr( "</table>" );

	  //GUI
	  QHBoxLayout* genL   = new QHBoxLayout();
	  genL->setSpacing        ( 2 );
	  genL->setContentsMargins( 20, 10, 20, 15 );
	  
	  QGridLayout* genL1  = new QGridLayout();
	  QVBoxLayout* genL11 = new QVBoxLayout();

	  QLabel* lb_remote         = us_label( tr("Remote Operations on Optima Instrument: SKIP stage, STOP:") );
	  lb_remote->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  row=0;
	  genL1-> addWidget( lb_remote,      row++,   0,  1,  22  );

	  QTextEdit* te_remote    = us_textedit();
	  te_remote    -> setFixedHeight  ( RowHeight * 2 );
	  te_remote    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_remote, true );

	  te_remote -> setText( "There were NO remote operations." );

	  genL1 -> addWidget( te_remote,     row++,    1,  1,  21  );

	  genL11 -> addLayout( genL1);
	  genL11 -> addStretch();
	  
	  //assemble
	  genL->addLayout( genL11);
	  
	  //Set GroupBox
	  QGroupBox *groupBox = new QGroupBox ( name );
	  QPalette p = groupBox->palette();
	  p.setColor(QPalette::Dark, Qt::white);
	  groupBox->setPalette(p);
	  
	  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	  
	  groupBox->setFlat(true);
	  
	  groupBox->setLayout(genL);
	  groupBoxes. push_back( groupBox );
	  
	}
      else
	{
	  //There were one or more remote operations:
	  for ( im = operation_types_live_update.begin(); im != operation_types_live_update.end(); ++im )
	    {
	      QString json_str = im.value();
	      
	      if ( json_str.isEmpty() )
		continue;
	      
	      QString      dtype_opt;
	      
	      if ( im.key() == "STOP" )
		dtype_opt    = "Stopping Optima";
	      
	      if ( im.key() == "SKIP" )
		dtype_opt =  "Skipping Stage";
	      
	      status_map = parse_autoflowStatus_json( json_str, im.key() );

	      //GUI
	      QHBoxLayout* genL   = new QHBoxLayout();
	      genL->setSpacing        ( 2 );
	      genL->setContentsMargins( 20, 10, 20, 15 );
	      
	      //Person
	      QLabel* lb_init         = us_label( tr("Performed by:") );
	      QLabel* lb_ID           = us_label( tr("User ID:") );
	      QLabel* lb_name         = us_label( tr("Name:") );
	      QLabel* lb_email        = us_label( tr("E-mail:") );
	      QLabel* lb_level        = us_label( tr("Level:") );
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
	      
	      //Operation Type | TimeStamp
	      QGridLayout* genL2  = new QGridLayout();
	      QVBoxLayout* genL21 = new QVBoxLayout();
	      
	      QLabel* lb_time_o         = us_label( tr("Operation, TimeStamp:") );
	      lb_time_o->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      QLabel* lb_time_o1        = us_label( tr("Type:") );
	      QLineEdit* le_time_o1     = us_lineedit( status_map[ "Remote Operation" ][ "type"], 0, true );
	      QLabel* lb_time_o2        = us_label( tr("Performed on:") );
	      QLineEdit* le_time_o2     = us_lineedit( operation_types_live_update_ts[ im.key() ] + " (UTC)", 0, true );
	      
	      row=0;
	      genL2 -> addWidget( lb_time_o,      row++,   0,  1,  6  );
	      genL2 -> addWidget( lb_time_o1,     row,     1,  1,  2  );
	      genL2 -> addWidget( le_time_o1,     row++,   3,  1,  3  );
	      genL2 -> addWidget( lb_time_o2,     row,     1,  1,  2  );
	      genL2 -> addWidget( le_time_o2,     row++,   3,  1,  3  );
	      
	      genL21 -> addLayout( genL2);
	      genL21 -> addStretch();
	      
	      //Comment
	      QGridLayout* genL3  = new QGridLayout();
	      QVBoxLayout* genL31 = new QVBoxLayout();
	      
	      QLabel* lb_comm         = us_label( tr("Reason for Operation:") );
	      lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      QLabel* lb_comm1        = us_label( tr("Comment:") );
	      QString t_comment       = status_map[ "Comment" ][ "comment"].isEmpty() ? "N/A" : status_map[ "Comment" ][ "comment"];
	      
	      QTextEdit* te_comm1    = us_textedit();
	      te_comm1    -> setFixedHeight  ( RowHeight * 2 );
	      te_comm1    ->setFont( QFont( US_Widgets::fixedFont().family(),
					    US_GuiSettings::fontSize() - 1) );
	      us_setReadOnly( te_comm1, true );
	      te_comm1 -> setText( t_comment );
	      
	      row=0;
	      genL3 -> addWidget( lb_comm,      row++,   0,  1,  6  );
	      genL3 -> addWidget( lb_comm1,     row,     1,  1,  2  );
	      genL3 -> addWidget( te_comm1,     row++,   3,  1,  3  );
	      
	      genL31 -> addLayout( genL3);
	      genL31 -> addStretch();
	      
	      //assemble
	      genL->addLayout( genL11);
	      genL->addLayout( genL21);
	      genL->addLayout( genL31);
	      
	      //Set GroupBox
	      QString gBox_name = "Remote Operation: " + dtype_opt;
	      QGroupBox *groupBox = new QGroupBox ( gBox_name );
	      QPalette p = groupBox->palette();
	      p.setColor(QPalette::Dark, Qt::white);
	      groupBox->setPalette(p);
	      
	      groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	      
	      groupBox->setFlat(true);
	      
	      groupBox->setLayout(genL);
	      groupBoxes. push_back( groupBox );

	      //assemble html
	      QString oper_ts = operation_types_live_update_ts[ im.key() ];
	      assemble_GMP_live_update( status_map, dtype_opt, oper_ts );
	    }
	}
      html_assembled += tr("<hr>");
    }
  
  else if ( s_name == "IMPORT" )
    {
      html_assembled += tr( "<h3 align=left>Reference Scan Determination, Triples Dropped, Data Saving (3. IMPORT)</h3>" );
      
      data_types_import [ "RI" ] = importRIJson;
      data_types_import [ "IP" ] = importIPJson;
      
      data_types_import_ts [ "RI" ] = importRIts;
      data_types_import_ts [ "IP" ] = importIPts;
      
      //Check for dropped triples for each Optical System:
      QStringList dropped_triples_RI, dropped_triples_IP;
      read_reportLists_from_aprofile( dropped_triples_RI, dropped_triples_IP );
      qDebug() << "List of dropped triples (all OSs): "
	       <<  dropped_triples_RI
	       <<  dropped_triples_IP;

      for ( im = data_types_import.begin(); im != data_types_import.end(); ++im )
	{
	  QString json_str = im.value();
	  
	  if ( json_str.isEmpty() )
	    continue;
	  
	  QString      dtype_opt;
	  QStringList  dtype_opt_dropped_triples;
	  
	  if ( im.key() == "RI" )
	    {
	      dtype_opt = "RI (UV/vis.)";
	      dtype_opt_dropped_triples = dropped_triples_RI;
	    }
	  if ( im.key() == "IP" )
	    {
	      dtype_opt =  "IP (Interf.)";
	      dtype_opt_dropped_triples = dropped_triples_IP;
	    }

	  status_map = parse_autoflowStatus_json( json_str, im.key() );

	  //GUI
	  QHBoxLayout* genL   = new QHBoxLayout();
	  genL->setSpacing        ( 2 );
	  genL->setContentsMargins( 20, 10, 20, 15 );
	  
	  QHBoxLayout* genL_sec_row = new QHBoxLayout();
	  genL_sec_row->setSpacing        ( 2 );
	  genL_sec_row->setContentsMargins( 20, 10, 20, 15 );
	  
	  QVBoxLayout* genL_v_rows = new QVBoxLayout();
	  genL_v_rows->setSpacing        ( 2 );
	  genL_v_rows->setContentsMargins( 20, 10, 20, 15 );
	  

	  //Person
	  QLabel* lb_init         = us_label( tr("Performed by:") );
	  QLabel* lb_ID           = us_label( tr("User ID:") );
	  QLabel* lb_name         = us_label( tr("Name:") );
	  QLabel* lb_email        = us_label( tr("E-mail:") );
	  QLabel* lb_level        = us_label( tr("Level:") );
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

	  //Ref. Scan | TimeStamp
	  QGridLayout* genL2  = new QGridLayout();
	  QVBoxLayout* genL21 = new QVBoxLayout();
	  
	  QLabel* lb_time_ref         = us_label( tr("Reference Scan, Data Saving:") );
	  lb_time_ref->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  QLabel* lb_time_ref1        = us_label( tr("Ref. Scan Method:") );
	  QString ref_scan_method     = ( dataSource. contains( "Absorbance" ) ) ? "N/A" : status_map[ "RefScan" ][ "type"];
	  QLineEdit* le_time_ref1     = us_lineedit( ref_scan_method, 0, true );
	  QLabel* lb_time_ref2        = us_label( tr("Data Saved on:") );
	  QLineEdit* le_time_ref2     = us_lineedit( data_types_import_ts[ im.key() ] + " (UTC)", 0, true );
	  
	  row=0;
	  genL2 -> addWidget( lb_time_ref,      row++,   0,  1,  6  );
	  genL2 -> addWidget( lb_time_ref1,     row,     1,  1,  2  );
	  genL2 -> addWidget( le_time_ref1,     row++,   3,  1,  3  );
	  genL2 -> addWidget( lb_time_ref2,     row,     1,  1,  2  );
	  genL2 -> addWidget( le_time_ref2,     row++,   3,  1,  3  );
	  
	  genL21 -> addLayout( genL2);
	  genL21 -> addStretch();

	  //Comment
	  QGridLayout* genL3  = new QGridLayout();
	  QVBoxLayout* genL31 = new QVBoxLayout();
	  
	  QLabel* lb_comm         = us_label( tr("Comment when Saved:") );
	  lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

	  QLabel* lb_comm1        = us_label( tr("Comment:") );
	  QTextEdit* te_comm1    = us_textedit();
	  te_comm1    -> setFixedHeight  ( RowHeight * 2 );
	  te_comm1    ->setFont( QFont( US_Widgets::fixedFont().family(),
					US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_comm1, true );
	  QString t_comment       = status_map[ "Comment when SAVED" ][ "comment_when_saved"].isEmpty() ?
	    "N/A" : status_map[ "Comment when SAVED" ][ "comment_when_saved"];
	  te_comm1 -> setText( t_comment );
	  	  
	  row=0;
	  genL3 -> addWidget( lb_comm,      row++,   0,  1,  6  );
	  genL3 -> addWidget( lb_comm1,     row,     1,  1,  2  );
	  genL3 -> addWidget( te_comm1,     row++,   3,  1,  3  );
	  
	  genL31 -> addLayout( genL3);
	  genL31 -> addStretch();

	  //assemble html
	  QString oper_ts = data_types_import_ts[ im.key() ];
	  assemble_GMP_import( status_map, dtype_opt, oper_ts );

	  //Dropped [if any]
	  QGridLayout* genL4  = NULL;
	  QVBoxLayout* genL41 = NULL;
	  if ( !dtype_opt_dropped_triples. isEmpty() )
	    {
	      html_assembled += tr(
				   "<table style=\"margin-left:10px\">"
				   "<caption style=\"color:red;\" align=left> <b><i>List of Dropped Triples: </i></b> </caption>"
				   "</table>"

				   "<table style=\"margin-left:25px\">"
				   );
	      
	      genL4  = new QGridLayout();
	      genL41 = new QVBoxLayout();

	      QLabel* lb_drop         = us_label( tr("Dropped Triples:") );
	      lb_drop->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      row=0;
	      genL4 -> addWidget( lb_drop,      row++,   0,  1,  6  );

	      QLabel* lb_drop1        = us_label( tr("Triple List:") );
	      genL4 -> addWidget( lb_drop1,     row,     1,  1,  2  );

	      QStringList triples_dropped;
	      for ( int i=0; i < dtype_opt_dropped_triples.size(); ++i )
		{
		  triples_dropped << dtype_opt_dropped_triples[ i ];

		  html_assembled += tr(
				       "<tr>"
				       "<td> Triple Name: </td> <td style=\"color:red;\"> %1 </td> "
				       "</tr>"
				   )
		    .arg( dtype_opt_dropped_triples[ i ] )
		    ;
		}

	      html_assembled += tr(
				   "</table>"
				   );
	      
	      QTextEdit* te_dropped_tr    = us_textedit();
	      te_dropped_tr    -> setFixedHeight  ( RowHeight * 2 );
	      te_dropped_tr    ->setFont( QFont( US_Widgets::fixedFont().family(),
						 US_GuiSettings::fontSize() - 1) );
	      us_setReadOnly( te_dropped_tr, true );
	      
	      te_dropped_tr -> setText( triples_dropped.join(",\n") );
	      genL4 -> addWidget( te_dropped_tr,     row++,   3,  1,  3  );
	      
	      genL41 -> addLayout( genL4);
	      genL41 -> addStretch();
	    }

	  //Comments for Dropped [if any]
	  QGridLayout* genL5  = NULL;
	  QVBoxLayout* genL51 = NULL;
	  if ( status_map. contains("Dropped") )
	    {
	      html_assembled += tr(
				   "<table style=\"margin-left:10px\">"
				   "<caption align=left> <b><i>Comments on [triples | channels | select channel] dropped: </i></b> </caption>"
				   "</table>"
				   
				   "<table style=\"margin-left:25px\">"
				   )
		;
	      
	      genL5  = new QGridLayout();
	      genL51 = new QVBoxLayout();

	      QLabel* lb_drop_c         = us_label( tr("Comments [Dropped Triple(s) | Channel(s)]:") );
	      lb_drop_c->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      row=0;
	      genL5 -> addWidget( lb_drop_c,      row++,   0,  1,  6  );

	      QLabel* lb_drop_c1        = us_label( tr("Comment:") );
	      genL5 -> addWidget( lb_drop_c1,     row,     1,  1,  2  );

	      QStringList comm_list;
	      QMap < QString, QString >::iterator dr;
	      for ( dr = status_map[ "Dropped" ].begin(); dr != status_map[ "Dropped" ].end(); ++dr )
		{
		  QString comm_curr = dr.key() + ":\n" + dr.value();
		  comm_list << comm_curr;

		  html_assembled += tr(
				       "<tr>"
				       "<td> Dropped:     %1 </td>"
				       "<td> Comment:     %2 </td>"
				       "</tr>"
				       )
		    .arg( dr.key()   )     //1
		    .arg( dr.value() )     //2
		    ;
		}
	      html_assembled += tr( "</table>" );

	      QTextEdit* te_drop_c1    = us_textedit();
	      te_drop_c1    -> setFixedHeight  ( RowHeight * 2 );
	      te_drop_c1    ->setFont( QFont( US_Widgets::fixedFont().family(),
						 US_GuiSettings::fontSize() - 1) );
	      us_setReadOnly( te_drop_c1, true );
	      
	      te_drop_c1 -> setText( comm_list.join(",\n") );
	      genL5 -> addWidget( te_drop_c1,     row++,   3,  1,  3  );
	 		  
	      genL51 -> addLayout( genL5);
	      genL51 -> addStretch();
	    }
	  
	  //assemble
	  genL->addLayout( genL11);
	  genL->addLayout( genL21);
	  genL->addLayout( genL31);

	  if ( genL41 != NULL )
	    genL_sec_row->addLayout( genL41);
	  if ( genL51 != NULL )
	    genL_sec_row->addLayout( genL51);

	  genL_v_rows->addLayout( genL);
	  genL_v_rows->addLayout( genL_sec_row);
	  
	  //Set GroupBox
	  QString gBox_name = "Data Type, Optics: " + dtype_opt;
	  QGroupBox *groupBox = new QGroupBox ( gBox_name );
	  QPalette p = groupBox->palette();
	  p.setColor(QPalette::Dark, Qt::white);
	  groupBox->setPalette(p);
	  
	  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	  
	  groupBox->setFlat(true);
	  
	  groupBox->setLayout(genL_v_rows);
	  groupBoxes. push_back( groupBox );
	  
	}
      html_assembled += tr("<hr>");
    }

  else if ( s_name == "EDITING" )
    {
      html_assembled += tr( "<h3 align=left>Meniscus Position Determination, Edit Profiles Saving (4. EDITING)</h3>" );
      
      data_types_edit [ "RI" ] = editRIJson;
      data_types_edit [ "IP" ] = editIPJson;
      
      data_types_edit_ts [ "RI" ] = editRIts;
      data_types_edit_ts [ "IP" ] = editIPts;
      
      for ( im = data_types_edit.begin(); im != data_types_edit.end(); ++im )
	{
	  QString json_str = im.value();
	  
	  if ( json_str.isEmpty() )
	    continue;
	  
	  QString dtype_opt;
	  
	  if ( im.key() == "RI" )
	    dtype_opt = "RI (UV/vis.)";
	  if ( im.key() == "IP" )
	    dtype_opt =  "IP (Interf.)";

	  status_map = parse_autoflowStatus_json( json_str, im.key() );

	  //GUI
	  QHBoxLayout* genL   = new QHBoxLayout();
	  genL->setSpacing        ( 2 );
	  genL->setContentsMargins( 20, 10, 20, 15 );
	  
	  QHBoxLayout* genL_sec_row = new QHBoxLayout();
	  genL_sec_row->setSpacing        ( 2 );
	  genL_sec_row->setContentsMargins( 20, 10, 20, 15 );
	  
	  QVBoxLayout* genL_v_rows = new QVBoxLayout();
	  genL_v_rows->setSpacing        ( 2 );
	  genL_v_rows->setContentsMargins( 20, 10, 20, 15 );

	  //Person
	  QLabel* lb_init         = us_label( tr("Performed by:") );
	  QLabel* lb_ID           = us_label( tr("User ID:") );
	  QLabel* lb_name         = us_label( tr("Name:") );
	  QLabel* lb_email        = us_label( tr("E-mail:") );
	  QLabel* lb_level        = us_label( tr("Level:") );
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

	  //Meniscus Position Determination
	  QGridLayout* genL2  = new QGridLayout();
	  QVBoxLayout* genL21 = new QVBoxLayout();
	  
	  QLabel* lb_men         = us_label( tr("Meniscus Position Determination:") );
	  lb_men->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  row=0;
	  genL2 -> addWidget( lb_men,      row++,   0,  1,  12  );

	  QMap < QString, QString >::iterator mp;
	  for ( mp = status_map[ "Meniscus" ].begin(); mp != status_map[ "Meniscus" ].end(); ++mp )
	    {
	      QLabel* lb_men1             = us_label( tr("Channel:") );
	      QLineEdit* le_men1          = us_lineedit( mp.key(), 0, true );
	      QLabel* lb_men2        = us_label( tr("Type:") );
	      QLineEdit* le_men2     = us_lineedit( mp.value(), 0, true );
	      
	      genL2 -> addWidget( lb_men1,     row,     1,  1,  2  );
	      genL2 -> addWidget( le_men1,     row,     3,  1,  3  );
	      genL2 -> addWidget( lb_men2,     row,     6,  1,  2  );
	      genL2 -> addWidget( le_men2,     row++,   8,  1,  3  );

	    }
	  
	  genL21 -> addLayout( genL2);
	  genL21 -> addStretch();


	  //TimeStamp
	  QGridLayout* genL3  = new QGridLayout();
	  QVBoxLayout* genL31 = new QVBoxLayout();
	  
	  QLabel* lb_ts         = us_label( tr("Edit Profiles Saved on:") );
	  lb_ts->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  QLabel* lb_ts1        = us_label( tr("TimeStamp:") );
	  QLineEdit* le_ts1     = us_lineedit( data_types_edit_ts[ im.key() ] + " (UTC)", 0, true );
	  
	  row=0;
	  genL3 -> addWidget( lb_ts,      row++,   0,  1,  6  );
	  genL3 -> addWidget( lb_ts1,     row,     1,  1,  2  );
	  genL3 -> addWidget( le_ts1,     row++,   3,  1,  3  );

	  genL31 -> addLayout( genL3);
	  genL31 -> addStretch();
	  
	  
	  //Comment
	  QGridLayout* genL4  = new QGridLayout();
	  QVBoxLayout* genL41 = new QVBoxLayout();
	  
	  QLabel* lb_comm         = us_label( tr("Comment when Saved:") );
	  lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  QLabel* lb_comm1        = us_label( tr("Comment:") );

	  QTextEdit* te_comm1    = us_textedit();
	  te_comm1    -> setFixedHeight  ( RowHeight * 2 );
	  te_comm1    ->setFont( QFont( US_Widgets::fixedFont().family(),
					US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_comm1, true );
	  QString t_comment       = status_map[ "Comment when SAVED" ][ "comment_when_saved"].isEmpty() ?
	    "N/A" : status_map[ "Comment when SAVED" ][ "comment_when_saved"];
	  
	  te_comm1 -> setText( t_comment );
	  	  
	  row=0;
	  genL4 -> addWidget( lb_comm,      row++,   0,  1,  6  );
	  genL4 -> addWidget( lb_comm1,     row,     1,  1,  2  );
	  genL4 -> addWidget( te_comm1,     row++,   3,  1,  3  );
	  
	  genL41 -> addLayout( genL4);
	  genL41 -> addStretch();

	  //assemble
	  genL->addLayout( genL11);
	  genL->addLayout( genL21);
	  genL_sec_row->addLayout( genL31);
	  genL_sec_row->addLayout( genL41);

	  genL_v_rows->addLayout( genL);
	  genL_v_rows->addLayout( genL_sec_row);
	  	  
	  //Set GroupBox
	  QString gBox_name = "Data Type, Optics: " + dtype_opt;
	  QGroupBox *groupBox = new QGroupBox ( gBox_name );
	  QPalette p = groupBox->palette();
	  p.setColor(QPalette::Dark, Qt::white);
	  groupBox->setPalette(p);
	  
	  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	  
	  groupBox->setFlat(true);
	  
	  groupBox->setLayout(genL_v_rows);
	  groupBoxes. push_back( groupBox );

	  //assemble html
	  QString oper_ts = data_types_edit_ts[ im.key() ];
	  assemble_GMP_editing( status_map, dtype_opt, oper_ts );
	  
	}
      html_assembled += tr("<hr>");
    }

  else if ( s_name == "ANALYSIS" )
    {
      html_assembled += tr( "<h3 align=left>Meniscus Position from FITMEN Stage, Job Cancellation (5. ANALYSIS)</h3>" );
      
      QMap < QString, QString > analysis_status_map       = parse_autoflowStatus_analysis_json( analysisJson );
      QMap < QString, QString > analysisCancel_status_map = parse_autoflowStatus_analysis_json( analysisCancelJson );

      //GUI
      QHBoxLayout* genL   = new QHBoxLayout();
      genL->setSpacing        ( 2 );
      genL->setContentsMargins( 20, 10, 20, 15 );
      
      QGridLayout* genL1  = new QGridLayout();
      QVBoxLayout* genL11 = new QVBoxLayout();

      QLabel* lb_men         = us_label( tr("Meniscus Position Determination at FITMEN Stage:") );
      lb_men->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      row=0;
      genL1-> addWidget( lb_men,      row++,   0,  1,  22  );
      
      if ( !p_2dsa_auto_fitmen ) // interactive FITMEN (manual)
	{
	  QMap < QString, QString >::iterator mfa;
	  for ( mfa = analysis_status_map.begin(); mfa != analysis_status_map.end(); ++mfa )
	    {
	      
	      QString mfa_value         = mfa.value();
	      QString pos               = mfa_value.split(", by")[0];
	      QString performed_by_time = mfa_value.split(", by")[1];
	      
	      QString performed_by, when;
	      if ( performed_by_time.contains(";") )
		{
		  performed_by      = performed_by_time.split(";")[0];
		  when              = performed_by_time.split(";")[1];  
		}
	      else
		{
		  performed_by = performed_by_time;
		  when         = "N/A";
		}

	      QLabel* lb_men1        = us_label( tr("Channel:") );
	      QLineEdit* le_men1     = us_lineedit( mfa.key(), 0, true );
	      QLabel* lb_men2        = us_label( tr("Type:") );
	      QLineEdit* le_men2     = us_lineedit( pos, 0, true );
	      QLabel* lb_men3        = us_label( tr("Performed by:") );
	      QLineEdit* le_men3     = us_lineedit( performed_by, 0, true );
	      QLabel* lb_men4        = us_label( tr("TimeStamp:") );
	      QLineEdit* le_men4     = us_lineedit( when + " (UTC)", 0, true );
	      
	      genL1 -> addWidget( lb_men1,     row,     1,  1,  2  );
	      genL1 -> addWidget( le_men1,     row,     3,  1,  3  );
	      genL1 -> addWidget( lb_men2,     row,     6,  1,  2  );
	      genL1 -> addWidget( le_men2,     row,     8,  1,  3  );
	      genL1 -> addWidget( lb_men3,     row,     11, 1,  2  );
	      genL1 -> addWidget( le_men3,     row,     13, 1,  3  );
	      genL1 -> addWidget( lb_men4,     row,     16, 1,  2  );
	      genL1 -> addWidget( le_men4,     row++,   18, 1,  3  );
	      
	    }

	  //assemble html
	  assemble_GMP_analysis_fitmen( analysis_status_map );
	}
      else //automatic mode
	{
	  QTextEdit* te_fitmen    = us_textedit();
	  te_fitmen    -> setFixedHeight  ( RowHeight * 2 );
	  te_fitmen    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_fitmen, true );

	  te_fitmen -> setText( "Meniscus positions have been determined automatically as best fit values for all channels." );

	  genL1 -> addWidget( te_fitmen,     row++,    1,  1,  21  );

	  html_assembled += tr( "Meniscus positions have been determined automatically as best fit values for all channels." );
	}

      //Cancelled Jobs
      QLabel* lb_canc         = us_label( tr("Information on CANCELED analysis jobs:") );
      lb_canc->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
      genL1-> addWidget( lb_canc,      row++,   0,  1,  22  );

      if ( !analysisCancelJson. isEmpty() )
	{
	  QMap < QString, QString >::iterator cj;
	  for ( cj = analysisCancel_status_map.begin(); cj != analysisCancel_status_map.end(); ++cj )
	    {
	      
	      QString cj_value                 = cj.value();
	      QString performed_by_reason_time = cj_value.split("CANCELED, by")[1];
	      
	      QString performed_by, reason, when;
	      if ( performed_by_reason_time.contains(";") )
		{
		  performed_by      = performed_by_reason_time.split(";")[0];
		  reason            = performed_by_reason_time.split(";")[1];
		  when              = performed_by_reason_time.split(";")[2];  
		}
	      else
		{
		  performed_by = performed_by_reason_time;
		  reason       = "N/A";
		  when         = "N/A";
		}
	      
	      QLabel* lb_canc1        = us_label( tr("Jobs Canceled for:") );
	      QLineEdit* le_canc1     = us_lineedit( cj.key(), 0, true );
	      QLabel* lb_canc2        = us_label( tr("Canceled by:") );
	      QLineEdit* le_canc2     = us_lineedit( performed_by, 0, true );
	      QLabel* lb_canc3        = us_label( tr("Reason:") );
	      QLineEdit* le_canc3     = us_lineedit( reason, 0, true );
	      QLabel* lb_canc4        = us_label( tr("TimeStamp:") );
	      QLineEdit* le_canc4     = us_lineedit( when + " (UTC)", 0, true );
	      
	      
	      genL1 -> addWidget( lb_canc1,     row,     1,  1,  2  );
	      genL1 -> addWidget( le_canc1,     row,     3,  1,  3  );
	      genL1 -> addWidget( lb_canc2,     row,     6,  1,  2  );
	      genL1 -> addWidget( le_canc2,     row,     8,  1,  3  );
	      genL1 -> addWidget( lb_canc3,     row,     11, 1,  2  );
	      genL1 -> addWidget( le_canc3,     row,     13, 1,  3  );
	      genL1 -> addWidget( lb_canc4,     row,     16, 1,  2  );
	      genL1 -> addWidget( le_canc4,     row++,   18, 1,  3  );
	    }
	}
      else //no cancelled jobs
	{
	  QTextEdit* te_canceled    = us_textedit();
	  te_canceled    -> setFixedHeight  ( RowHeight * 2 );
	  te_canceled    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_canceled, true );

	  te_canceled -> setText( "No CANCELLED jobs." );

	  genL1 -> addWidget( te_canceled,     row++,    1,  1,  21  );
	}
      

      genL11 -> addLayout( genL1);
      genL11 -> addStretch();

      //assemble
      genL->addLayout( genL11);
      	  	  
      //Set GroupBox
      QGroupBox *groupBox = new QGroupBox ( name );
      QPalette p = groupBox->palette();
      p.setColor(QPalette::Dark, Qt::white);
      groupBox->setPalette(p);
      
      groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
      
      groupBox->setFlat(true);
      
      groupBox->setLayout(genL);
      groupBoxes. push_back( groupBox );

      //assemble html
      assemble_GMP_analysis_cancelled( analysisCancel_status_map, analysisCancelJson );
      html_assembled += tr("<hr>");
    }
  
  else if ( s_name == "E-SIGNATURES" )
    {
      html_assembled += tr( "<h3 align=left>Information on Reassigning Reviewers (E-SIGNING)</h3>" );

      //take all info from  eSign_details_auto[ "createUpdateLogJson" ];
      status_map = parse_autoflowStatus_json( eSign_details_auto[ "createUpdateLogJson" ], "" );

      //Gui
      if ( status_map.isEmpty()  )
	{
	  //GUI
	  QHBoxLayout* genL   = new QHBoxLayout();
	  genL->setSpacing        ( 2 );
	  genL->setContentsMargins( 20, 10, 20, 15 );
      	  
	  QGridLayout* genL1  = new QGridLayout();
	  QVBoxLayout* genL11 = new QVBoxLayout();

	  QLabel* lb_remote         = us_label( tr("Reassignments of Operator(s), Reviewer(s), Approver(s):") );
	  lb_remote->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	  row=0;
	  genL1-> addWidget( lb_remote,      row++,   0,  1,  22  );

	  QTextEdit* te_remote    = us_textedit();
	  te_remote    -> setFixedHeight  ( RowHeight * 2 );
	  te_remote    ->setFont( QFont( US_Widgets::fixedFont().family(),
					 US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_remote, true );

	  te_remote -> setText( "There were NO reassignments." );

	  genL1 -> addWidget( te_remote,     row++,    1,  1,  21  );

	  genL11 -> addLayout( genL1);
	  genL11 -> addStretch();
	  
	  //assemble
	  genL->addLayout( genL11);
	  
	  //Set GroupBox
	  QGroupBox *groupBox = new QGroupBox ( name );
	  QPalette p = groupBox->palette();
	  p.setColor(QPalette::Dark, Qt::white);
	  groupBox->setPalette(p);
	  
	  groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	  
	  groupBox->setFlat(true);
	  
	  groupBox->setLayout(genL);
	  groupBoxes. push_back( groupBox );
	  
	}
      else
	{
	  QStringList s_map_keys = status_map. keys();
	  for (int i=0; i < s_map_keys.size(); ++i  )
	    {
	      QMap <QString, QString > admin_map = status_map[ s_map_keys [ i ] ]; 
	      
	      QString p_id   = admin_map[ "Person" ].split(".")[0].trimmed();
	      QString p_name = admin_map[ "Person" ].split(".")[1].trimmed();
	      
	      //Comment (replacements):
	      QString c_repls = admin_map[ "Comment" ];
	      QString c_repls_str;
	      if ( c_repls. contains("replacement:") )
		{
		  QStringList c_repls_list = c_repls. split(";");
		  for ( int j=0; j<c_repls_list.size(); ++j )
		    c_repls_str += c_repls_list[j].replace("replacement:",": ") + "<br>";
		}
	      else
		c_repls_str = c_repls;

	      QHBoxLayout* genL_i   = new QHBoxLayout();
	      genL_i->setSpacing        ( 2 );
	      genL_i->setContentsMargins( 20, 10, 20, 15 );
	      
	      //Person
	      QLabel* lb_init         = us_label( tr("Performed by:") );
	      QLabel* lb_ID           = us_label( tr("User ID:") );
	      QLabel* lb_name         = us_label( tr("Name:") );
	      QLineEdit* le_ID        = us_lineedit( p_id, 0, true);
	      QLineEdit* le_name      = us_lineedit( p_name, 0, true);
	      
	      QGridLayout* genL1  = new QGridLayout();
	      QVBoxLayout* genL11 = new QVBoxLayout();
	      
	      row=0;
	      genL1 -> addWidget( lb_init,      row++,   0,  1,  6  );
	      genL1 -> addWidget( lb_ID,        row,     1,  1,  2  );
	      genL1 -> addWidget( le_ID,        row++,   3,  1,  3  );
	      genL1 -> addWidget( lb_name,      row,     1,  1,  2  );
	      genL1 -> addWidget( le_name,      row++,   3,  1,  3  );
	  	      
	      genL11 -> addLayout( genL1);
	      genL11 -> addStretch();
	      
	      //TimeStamp
	      QGridLayout* genL2  = new QGridLayout();
	      QVBoxLayout* genL21 = new QVBoxLayout();
	      
	      QLabel* lb_time_o         = us_label( tr("TimeStamp:") );
	      lb_time_o->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      QLabel* lb_time_o2        = us_label( tr("Performed on:") );
	      QLineEdit* le_time_o2     = us_lineedit( admin_map["timeDate"], 0, true );
	      
	      row=0;
	      genL2 -> addWidget( lb_time_o,      row++,   0,  1,  6  );
	      genL2 -> addWidget( lb_time_o2,     row,     1,  1,  2  );
	      genL2 -> addWidget( le_time_o2,     row++,   3,  1,  3  );
	      
	      genL21 -> addLayout( genL2);
	      genL21 -> addStretch();
	      
	      //Comment
	      QGridLayout* genL3  = new QGridLayout();
	      QVBoxLayout* genL31 = new QVBoxLayout();
	      
	      QLabel* lb_comm         = us_label( tr("Reassignments:") );
	      lb_comm->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
	      QLabel* lb_comm1        = us_label( tr("Actions:") );
	    	      
	      QTextEdit* te_comm1    = us_textedit();
	      te_comm1    -> setFixedHeight  ( RowHeight * 2 );
	      te_comm1    ->setFont( QFont( US_Widgets::fixedFont().family(),
					    US_GuiSettings::fontSize() - 1) );
	      us_setReadOnly( te_comm1, true );
	      te_comm1 -> setText( c_repls_str);
	      
	      row=0;
	      genL3 -> addWidget( lb_comm,      row++,   0,  1,  6  );
	      genL3 -> addWidget( lb_comm1,     row,     1,  1,  2  );
	      genL3 -> addWidget( te_comm1,     row++,   3,  1,  3  );
	      
	      genL31 -> addLayout( genL3);
	      genL31 -> addStretch();
	      
	      //assemble
	      genL_i->addLayout( genL11);
	      genL_i->addLayout( genL21);
	      genL_i->addLayout( genL31);

	      //Set GroupBox
	      QString gBox_name = "Reviewers Update: " + QString::number(i+1);
	      QGroupBox *groupBox = new QGroupBox ( gBox_name );
	      QPalette p = groupBox->palette();
	      p.setColor(QPalette::Dark, Qt::white);
	      groupBox->setPalette(p);
	      
	      groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
	      
	      groupBox->setFlat(true);
	      
	      groupBox->setLayout(genL_i);
	      groupBoxes. push_back( groupBox );
	      
	    }
	}
      
      //assemble html
      assemble_GMP_esign_reassign( status_map );
      
    }
  else
    {
      //GUI
      QHBoxLayout* genL   = new QHBoxLayout();
      genL->setSpacing        ( 2 );
      genL->setContentsMargins( 20, 10, 20, 15 );
      
      QLabel* lb_dev         = us_label( tr("Under Development:") );
      genL->addWidget( lb_dev );

      //Set GroupBox
      QGroupBox *groupBox = new QGroupBox ( name );
      QPalette p = groupBox->palette();
      p.setColor(QPalette::Dark, Qt::white);
      groupBox->setPalette(p);
      
      groupBox-> setStyleSheet( "QGroupBox { font: bold;  background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border: 2px solid gray; border-radius: 10px; margin-top: 20px; margin-bottom: 10px; padding-top: 5px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; margin: 0 5px; background-color: black; color: white; padding: 0 3px;}  QGroupBox::indicator { width: 13px; height: 13px; border: 1px solid grey; background-color: rgba(204, 204, 204, 255);} QGroupBox::indicator:hover {background-color: rgba(235, 235, 235, 255);} QLabel {background-color: rgb(105,105,105);}");
      
      groupBox->setFlat(true);
      groupBox->setLayout(genL);
      groupBoxes. push_back( groupBox );
    }
  
  return groupBoxes;
}

// Query autoflow for # records
QMap< QString, QString>  US_auditTrailGMP::read_autoflow_record( int autoflowID  )
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
   qry << "read_autoflow_history_record"
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
	   protocol_details[ "label" ]          = db->value( 15 ).toString();
	   protocol_details[ "gmpRun" ]         = db->value( 16 ).toString();

	   protocol_details[ "filename" ]       = db->value( 17 ).toString();
	   protocol_details[ "aprofileguid" ]   = db->value( 18 ).toString();

	   protocol_details[ "analysisIDs" ]    = db->value( 19 ).toString();
	   protocol_details[ "intensityID" ]    = db->value( 20 ).toString();
	   protocol_details[ "statusID" ]       = db->value( 21 ).toString();
	   protocol_details[ "failedID" ]       = db->value( 22 ).toString();
	   protocol_details[ "operatorID" ]     = db->value( 23 ).toString();
	   protocol_details[ "devRecord" ]      = db->value( 24 ).toString();
	   protocol_details[ "gmpReviewID" ]    = db->value( 25 ).toString();

	   protocol_details[ "expType" ]        = db->value( 26 ).toString();
	   protocol_details[ "dataSource" ]     = db->value( 27 ).toString();
	 }
     }

   return protocol_details;
}

//Read AProfile's reportIDs per channel:
void US_auditTrailGMP::read_reportLists_from_aprofile( QStringList & dropped_triples_RI, QStringList & dropped_triples_IP )
{
  dropped_triples_RI. clear();
  dropped_triples_IP. clear();
  QMap< QString, QString> channame_to_reportIDs_RI;
  QMap< QString, QString> channame_to_reportIDs_IP;
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

  qDebug() << "AProfGUID: " << AProfileGUID;
    
  QStringList qry;
  qry << "get_aprofile_info" << AProfileGUID;
  db.query( qry );
  
  while ( db.next() )
    {
      aprofile_xml         = db.value( 2 ).toString();
    }

  if ( !aprofile_xml.isEmpty() )
    {
      QXmlStreamReader xmli( aprofile_xml );
      readReportLists( xmli, channame_to_reportIDs_RI, channame_to_reportIDs_IP );
    }

  //Now, construct list of dropped triples per optical system used:
  dropped_triples_RI = buildDroppedTriplesList( &db, channame_to_reportIDs_RI );
  dropped_triples_IP = buildDroppedTriplesList( &db, channame_to_reportIDs_IP );
}

//Build list of dropped triples out of channame_to_reportIDs QMap;
QStringList US_auditTrailGMP::buildDroppedTriplesList ( US_DB2* dbP, QMap <QString, QString> channame_to_reportIDs )
{
  QStringList dropped_triples_list;
  
  QMap<QString, QString>::iterator chan_rep;
  for ( chan_rep = channame_to_reportIDs.begin(); chan_rep != channame_to_reportIDs.end(); ++chan_rep )
    {
      QString chan_key  = chan_rep.key();
      QString reportIDs = chan_rep.value();
      qDebug() << "Channel name -- " << chan_key << ", reportIDs -- " << reportIDs;
      
      QStringList reportIDs_list = reportIDs.split(",");
      for (int i=0; i<reportIDs_list.size(); ++i)
	{
	  QString rID = reportIDs_list[i];
	  QString Wavelength;
	  QString TripleDropped;
	  
	  QStringList qry;
	  qry << "get_report_info_by_id" << rID;
	  dbP->query( qry );
	  
	  if ( dbP->lastErrno() == US_DB2::OK )      
	    {
	      while ( dbP->next() )
		{
		  Wavelength    = dbP->value( 5 ).toString();
		  TripleDropped = dbP->value( 9 ).toString();
		}
	      
	      if ( TripleDropped == "YES" )
		{
		  QString dropped_triple_name = chan_key + "." + Wavelength;
		  dropped_triples_list << dropped_triple_name;
		}
	    }
	}
    }
	  
  return dropped_triples_list;
}

//Read AProfile's reportIDs per channel:
bool US_auditTrailGMP::readReportLists( QXmlStreamReader& xmli, QMap< QString, QString> & channame_to_reportIDs_RI,
					QMap< QString, QString> & channame_to_reportIDs_IP )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      
      if ( xmli.isStartElement() )
	{
	  if ( ename == "channel_parms" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      
	      if ( attr.hasAttribute("load_volume") ) //ensure it reads upper-level <channel_parms>
		{
		  QString channel_name = attr.value( "channel" ).toString();
		  QString channel_desc = attr.value( "chandesc" ).toString();
		  
		  QString opsys = channel_desc.split(":")[1]; // UV/vis. or Interf.
		  
		  if ( opsys.contains("UV/vis")  ) //RI
		    {
		      //Read what reportID corresponds to channel:
		      if ( attr.hasAttribute("report_id") )
			channame_to_reportIDs_RI[ channel_name ] = attr.value( "report_id" ).toString();
		    }
		  if ( opsys.contains("Interf") )  //IP
		    {
		      //Read what reportID corresponds to channel:
		      if ( attr.hasAttribute("report_id") )
			channame_to_reportIDs_IP[ channel_name ] = attr.value( "report_id" ).toString();
		    }
		}
	    }

	  //2DSA
	  else if ( ename == "p_2dsa" )
	    {
	      read_2dsa_settings( xmli );
	    }
	}
      
      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element
      
      if ( was_end  &&  ename == "p_2dsa" )   // Break 
	break;
    }
  
  return ( ! xmli.hasError() );
}

bool US_auditTrailGMP::read_2dsa_settings( QXmlStreamReader& xmli )
{
  while( ! xmli.atEnd() )
    {
      QString ename   = xmli.name().toString();
      //qDebug() << "AP:2fX: ename" << ename;

      if ( xmli.isStartElement() )
	{
	  if ( ename == "job_fitmen" || ename == "job_fitmen_auto" )
	    {
	      QXmlStreamAttributes attr = xmli.attributes();
	      p_2dsa_run_fitmen     = US_Util::bool_flag( attr.value( "run" ).toString() );
	      p_2dsa_auto_fitmen    = attr.value( "interactive" ).toString().toInt() == 0;

	      qDebug() << "In 2DSA's Settings run_fitmen, auto_fitmen -- "
		       << p_2dsa_run_fitmen
		       << p_2dsa_auto_fitmen;
	    }
	}

      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element

      if ( was_end  &&  ename == "p_2dsa" )   // Break after "</p_2dsa>"
	break;
    }
  
  return ( ! xmli.hasError() );
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

      //For parsing eSignUpdate status (by admin): revs. reassignment
      if ( key.contains( "Updated by" ) )
	{	  
	  QJsonArray json_array = value.toArray();
	  QMap< QString, QString > admin_info_map;
	  
	  for (int i=0; i < json_array.size(); ++i )
	    {
	      foreach(const QString& array_key, json_array[i].toObject().keys())
		{
		  admin_info_map[ array_key ] = json_array[i].toObject().value(array_key).toString();
		  qDebug() << "Admin update Map: -- key, value: "
			   << array_key
			   << json_array[i].toObject().value(array_key).toString();
		}
	    }

	  status_map[ key ] = admin_info_map;
	}
      
    }
  
  return status_map;
}


//Parse autoflowStatus Analysis Json
QMap < QString, QString > US_auditTrailGMP::parse_autoflowStatus_analysis_json( QString statusJson )
{
  QMap <QString, QString>  status_map;

  QJsonDocument jsonDoc = QJsonDocument::fromJson( statusJson.toUtf8() );
  
  if ( jsonDoc. isArray() )
    {
      QJsonArray json_array  = jsonDoc.array();
      qDebug() << "IN ANALYSIS_JSON [ARRAY]: " << json_array;
      
      for (int i=0; i < json_array.size(); ++i )
	{
	  foreach(const QString& key, json_array[ i ].toObject().keys())
	    {
	      QJsonValue value = json_array[ i ].toObject().value(key);
	      qDebug() << "ANALYSIS_JSON [ARRAY]: key, value: " << key << value.toString();
	      
	      status_map[ key ] = value.toString();
	    }
	}
    }
  else if ( jsonDoc. isObject() )
    {
      QJsonObject json_obj  = jsonDoc.object();
      qDebug() << "IN ANALYSIS_JSON [OBJECT]: " << json_obj;

      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);
	  qDebug() << "ANALYSIS_JSON [OBJECT]: key, value: " << key << value;

	  status_map[ key ] = value.toString();
	}
    }
    
  return status_map;
  
}

//display oper/revs/appr.
void US_auditTrailGMP::display_reviewers_auto( int& row, QMap< QString, QString> eSign_d, QString JsonListName, QGridLayout* genL )
{
  if ( eSign_d. contains( JsonListName ) )
    {
      //for HTML
      QMap< QString, QString > esigs_html; 
            
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
	  
	  //QLineEdit* le_comment = us_lineedit( e_comment, 0, true );
	  QTextEdit* te_comment    = us_textedit();
	  te_comment    -> setFixedHeight  ( RowHeight * 2 );
	  te_comment    ->setFont( QFont( US_Widgets::fixedFont().family(),
					  US_GuiSettings::fontSize() - 1) );
	  us_setReadOnly( te_comment, true );
	  te_comment -> setText( e_comment );
	  
	  
	  QLineEdit* le_stat = check_eSign_status_for_gmpReport_auto( current_reviewer, eSign_d ); 
	  le_stat -> setObjectName( "status: " + current_reviewer );

	  qDebug() << "Object Name of le_stat -- " << le_stat->objectName();

	  if( le_stat->text(). contains("NOT") )
	    {
	      le_date    -> setText( "N/A" );
	      te_comment -> setText( "N/A" );
	    }
	  
	  genL -> addWidget( le_name,      row,      0,  1,  3 );
	  genL -> addWidget( le_role,      row,      3,  1,  2 );
	  genL -> addWidget( le_date,      row,      5,  1,  2 );
	  genL -> addWidget( te_comment,   row,      7,  1,  3 );
	  genL -> addWidget( le_stat,      row++,    10, 1,  2 );

	  //assemble eSigs html
	  esigs_html[ "Name" ] = le_name->text();
	  esigs_html[ "Role" ] = le_role->text();
	  esigs_html[ "Date" ] = le_date->text();
	  esigs_html[ "Comment" ] = te_comment->toPlainText();
	  esigs_html[ "Status" ] = le_stat->text();
	  assemble_esigs( esigs_html ); 
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

  //reset the rest
  html_assembled. clear();
  html_assembled_esigs. clear();
  filePath_pdf  . clear();

}

// Create a subdirectory if need be
bool US_auditTrailGMP::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;

   if ( folder.mkdir( subdir ) ) return true;

   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );

   return false;
}

//GMP init html
void US_auditTrailGMP::assemble_GMP_init( QMap< QString, QMap < QString, QString > > status_map_c, QString createdGMPrunts )
{
  //html_assembled += tr("<hr>");
  html_assembled += tr( "<h3 align=left>GMP Run Initiation (1. EXPERIMENT)</h3>" );
  
  //html_assembled += tr("<br>");
  html_assembled += tr(
		           "<table style=\"margin-left:10px\">"
			   "<caption align=left> <b><i>Initiated by: </i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:25px\">"
			   "<tr><td>User ID: </td> <td>%1</td></tr>"
			   "<tr><td>Name: </td><td> %2, %3 </td></tr>"
			   "<tr><td>E-mail: </td><td> %4 </td> </tr>"
			   "<tr><td>Level: </td><td> %5 </td></tr>"
			   "</table>"
			   )
    .arg( status_map_c[ "Person" ][ "ID"] )                       //1
    .arg( status_map_c[ "Person" ][ "lname" ] )                   //2
    .arg( status_map_c[ "Person" ][ "fname" ] )                   //3
    .arg( status_map_c[ "Person" ][ "email" ] )                   //4
    .arg( status_map_c[ "Person" ][ "level" ] )                   //5
    ;

  html_assembled += tr(
			   "<table style=\"margin-left:10px\">"
			   "<caption align=left> <b><i>Time of GMP Run Initiation: </i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:25px\">"
			   "<tr>"
			   "<td> Initiated at:     %1 (UTC)</td>"
			   "</tr>"
			   "</table>"
			   )
    .arg( createdGMPrunts )     //1
    ;
  
  html_assembled += tr(
			   "<table style=\"margin-left:10px\">"
			   "<caption align=left> <b><i>Comment at the Time of GMP Run Initiation: </i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:25px\">"
			   "<tr>"
			   "<td> Comment:  %1 </td> "
			   "</tr>"
			   "</table>"
			   )
    .arg( status_map_c[ "Comment" ][ "comment"] )     //1
    ;
  html_assembled += tr("<hr>");
}

//LIVE UPDATE html
void US_auditTrailGMP::assemble_GMP_live_update( QMap< QString, QMap < QString, QString > > status_map,
						 QString dtype_opt, QString oper_ts )
{
  html_assembled += tr(
		       "<table>"		   
		       "<tr>"
		       "<td><b>Operation Type:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
		       "</tr>"
		       "</table>"
		       )
    .arg( dtype_opt )                       //1
    ;

  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Performed by: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr><td>User ID: </td> <td>%1</td></tr>"
		       "<tr><td>Name: </td><td> %2, %3 </td></tr>"
		       "<tr><td>E-mail: </td><td> %4 </td> </tr>"
		       "<tr><td>Level: </td><td> %5 </td></tr>"
		       "</table>"
		       )
    .arg( status_map[ "Person" ][ "ID"] )                       //1
    .arg( status_map[ "Person" ][ "lname" ] )                   //2
    .arg( status_map[ "Person" ][ "fname" ] )                   //3
    .arg( status_map[ "Person" ][ "email" ] )                   //4
    .arg( status_map[ "Person" ][ "level" ] )                   //5
    ;
  
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Operation, Timestamp: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr>"
		       "<td> Type:             %1 </td> "
		       "<td> Performed at:     %2 (UTC)</td>"
		       "</tr>"
		       "</table>"
		       )
    .arg( status_map[ "Remote Operation" ][ "type"] )     //1
    .arg( oper_ts )                                       //2
    ;

  QString t_comment = status_map[ "Comment" ][ "comment"].isEmpty() ? "N/A" : status_map[ "Comment" ][ "comment"];
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Reason for Operation: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr>"
		       "<td> Comment:          %1 </td> "
		       "</tr>"
		       "</table>"
		       )
    .arg( t_comment )                                      //1
    ;

   html_assembled += tr("<br>");
}


//IMPORT html
void US_auditTrailGMP::assemble_GMP_import( QMap< QString, QMap < QString, QString > > status_map,
					    QString dtype_opt, QString oper_ts )
{
  
  html_assembled += tr(
		       "<table>"		   
		       "<tr>"
		       "<td><b>Data Type, Optics:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
		       "</tr>"
		       "</table>"
		       )
    .arg( dtype_opt )                       //1
    ;

  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Performed by: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr><td>User ID: </td> <td>%1</td></tr>"
		       "<tr><td>Name: </td><td> %2, %3 </td></tr>"
		       "<tr><td>E-mail: </td><td> %4 </td> </tr>"
		       "<tr><td>Level: </td><td> %5 </td></tr>"
		       "</table>"
		       )
    .arg( status_map[ "Person" ][ "ID"] )                       //1
    .arg( status_map[ "Person" ][ "lname" ] )                   //2
    .arg( status_map[ "Person" ][ "fname" ] )                   //3
    .arg( status_map[ "Person" ][ "email" ] )                   //4
    .arg( status_map[ "Person" ][ "level" ] )                   //5
    ;

  QString ref_scan_method = ( dataSource. contains( "Absorbance" ) ) ? "N/A" : status_map[ "RefScan" ][ "type"];
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Reference Scan, Data Saving: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr>"
		       "<td> Ref. Scan Method:  %1 </td> "
		       "<td> Data Saved at:     %2 (UTC)</td>"
		       "</tr>"
		       "</table>"
		       )
    .arg( ref_scan_method )     //1
    .arg( oper_ts  )            //2
    ;
  
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Comment at the Time of Data Saving: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr>"
		       "<td> Comment:  %1 </td> "
		       "</tr>"
		       "</table>"
		       )
    .arg( status_map[ "Comment when SAVED" ][ "comment_when_saved"] )     //1
    ;
      
}


//EDITING html
void US_auditTrailGMP::assemble_GMP_editing( QMap< QString, QMap < QString, QString > > status_map,
					     QString dtype_opt, QString oper_ts )
{

  html_assembled += tr(
		       "<table>"		   
		       "<tr>"
		       "<td><b>Data Type, Optics:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
		       "</tr>"
		       "</table>"
		       )
    .arg( dtype_opt )                       //1
    ;

  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Performed by: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr><td>User ID: </td> <td>%1</td>"
		       "<tr><td>Name: </td><td> %2, %3 </td></tr>"
		       "<tr><td>E-mail: </td><td> %4 </td> </tr>"
		       "<tr><td>Level: </td><td> %5 </td></tr>"
		       "</table>"
		       )
    .arg( status_map[ "Person" ][ "ID"] )                       //1
    .arg( status_map[ "Person" ][ "lname" ] )                   //2
    .arg( status_map[ "Person" ][ "fname" ] )                   //3
    .arg( status_map[ "Person" ][ "email" ] )                   //4
    .arg( status_map[ "Person" ][ "level" ] )                   //5
    ;
  
  //iterate over channels for Meniscus type:
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Meniscus Position Determination: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       )
    ;

  QMap < QString, QString >::iterator mp;
  for ( mp = status_map[ "Meniscus" ].begin(); mp != status_map[ "Meniscus" ].end(); ++mp )
    {
      html_assembled += tr(
			   "<tr>"
			   "<td> Channel:  %1 </td>"
			   "<td> Type:     %2 </td>"
			   "</tr>"
			   )
	.arg( mp.key()   )     //1
	.arg( mp.value() )     //2
	;
    }
  html_assembled += tr( "</table>" );
  
  //Edit Profiles Saved:
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Edit Profiles Saved at: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr><td> %1 (UTC)</td>"
		       "</table>"
		       )
    .arg( oper_ts )           //1
    ;
  
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Comment at the Time of Data Saving: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       "<tr>"
		       "<td> Comment:  %1 </td> "
		       "</tr>"
		       "</table>"
		       )
    .arg( status_map[ "Comment when SAVED" ][ "comment_when_saved"] )     //1
    ;
  
}


//ANALYSIS-fitmen html
void US_auditTrailGMP::assemble_GMP_analysis_fitmen( QMap < QString, QString > analysis_status_map )
{
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Meniscus Position Determination from FITMEN_MANUAL stage: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       )
    ;
  
  QMap < QString, QString >::iterator mfa;
  for ( mfa = analysis_status_map.begin(); mfa != analysis_status_map.end(); ++mfa )
    {
      
      QString mfa_value         = mfa.value();
      QString pos               = mfa_value.split(", by")[0];
      QString performed_by_time = mfa_value.split(", by")[1];
      
      QString performed_by, when;
      if ( performed_by_time.contains(";") )
	{
	  performed_by      = performed_by_time.split(";")[0];
	  when              = performed_by_time.split(";")[1];  
	}
      else
	{
	  performed_by = performed_by_time;
	  when         = "N/A";
	}
      html_assembled += tr(			       
			   "<tr>"
			   "<td> Channel:  %1, </td>"
			   "<td>           %2, </td>"
			   "<td> by:       %3, </td>"
			   "<td> at:       %4 (UTC)</td>"
			   "</tr>"
						       )
	.arg( mfa.key()   )     //1
	.arg( pos )             //2
	.arg( performed_by )    //3
	.arg( when )            //4
	;
    }
  
  html_assembled += tr( "</table>" );
}
						     
//ANALYSIS-cancelled html
void US_auditTrailGMP::assemble_GMP_analysis_cancelled( QMap < QString, QString > analysisCancel_status_map,
							QString analysisCancelJson )
{
  html_assembled += tr(
		       "<table style=\"margin-left:10px\">"
		       "<caption align=left> <b><i>Information on CANCELED analysis jobs: </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:25px\">"
		       )
    ;
  
  if ( !analysisCancelJson. isEmpty() )
    {
      qDebug() << "analysisCancelJson QMap NOT empty!";
      
      QMap < QString, QString >::iterator cj;
      for ( cj = analysisCancel_status_map.begin(); cj != analysisCancel_status_map.end(); ++cj )
	{
	  
	  QString cj_value                 = cj.value();
	  QString performed_by_reason_time = cj_value.split("CANCELED, by")[1];
	  
	  QString performed_by, reason, when;
	  if ( performed_by_reason_time.contains(";") )
	    {
	      performed_by      = performed_by_reason_time.split(";")[0];
	      reason            = performed_by_reason_time.split(";")[1];
	      when              = performed_by_reason_time.split(";")[2];  
	    }
	  else
	    {
	      performed_by = performed_by_reason_time;
	      reason       = "N/A";
	      when         = "N/A";
	    }
	  html_assembled += tr(			       
			       "<tr>"
			       "<td> Jobs Canceled for:  %1, </td>"
			       "</tr>"
			       "<tr>"
			       "<td> Jobs Canceled by:   %2, </td>"
			       "</tr>"
			       "<tr>"
			       "<td> Reason:             %3, </td>"
			       "</tr>"
			       "<tr>"
			       "<td> When:               %4  (UTC)</td>"
			       "</tr>"
						       )
	    .arg( cj.key()   )      //1
	    .arg( performed_by )    //2
	    .arg( reason )          //3
	    .arg( when )            //4
	    ;
	}
    }
  else
    {
      html_assembled += tr( "<tr><td> No CANCELLED jobs. </td></tr>" );
    }
  html_assembled += tr( "</table>" );
  
}

//E-SIGS: reviewers change html
void US_auditTrailGMP::assemble_GMP_esign_reassign( QMap < QString, QMap <QString, QString >> status_map )
{
  if ( status_map. isEmpty() )
    {
      html_assembled += tr( "<table>" );
      html_assembled += tr( "<tr><td> There were NO reviewer(s) reassignments. </td></tr>" );
      html_assembled += tr( "</table>" );

      return;
    }
  
  QStringList s_map_keys = status_map. keys();
  for (int i=0; i < s_map_keys.size(); ++i  )
    {
      QMap <QString, QString > admin_map = status_map[ s_map_keys [ i ] ]; 
      
      html_assembled += tr(
			   "<table>"		   
			   "<tr>"
			   "<td><b>Reviewers Update %1:</b></td>"
			   "</tr>"
			   "</table>"
			   )
	.arg( QString::number( i+1 ) )                       //1
	;

      QString p_id   = admin_map[ "Person" ].split(".")[0].trimmed();
      QString p_name = admin_map[ "Person" ].split(".")[1].trimmed();

      //Comment (replacements):
      QString c_repls = admin_map[ "Comment" ];
      QString c_repls_str;
      if ( c_repls. contains("replacement:") )
	{
	  QStringList c_repls_list = c_repls. split(";");
	  for ( int j=0; j<c_repls_list.size(); ++j )
	    c_repls_str += c_repls_list[j].replace("replacement:",": ") + "<br>";
	}
      else
	c_repls_str = c_repls;
      
      html_assembled += tr(
			   "<table style=\"margin-left:10px\">"
			   "<caption align=left> <b><i>Performed by: </i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:25px\">"
			   "<tr><td>User: </td> <td>%1 (ID: %2)</td>"
			   "<tr><td>TimeDate: </td><td> %3 </td></tr>"
			   "<tr><td>Reassingments: </td><td> %4 </td> </tr>"
			   "</table>"
			   )
	.arg( p_name )                            //1
	.arg( p_id )                              //2
	.arg( admin_map[ "timeDate" ] )           //3
	.arg( c_repls_str )                       //4
	;
    }
}
  


//initHTML
void US_auditTrailGMP::initHTML( void )
{
    QString rptpage;

  // Compose the report header
  rptpage   = QString( "<?xml version=\"1.0\"?>\n" );
  rptpage  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
  rptpage  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
              "/xhtml1-strict.dtd\">\n";
  rptpage  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
              " xml:lang=\"en\" lang=\"en\">\n";
  rptpage  += "  <head>\n";
  rptpage  += "  <title> Ultrascan III Composite Report </title>\n";
  rptpage  += "  <meta http-equiv=\"Content-Type\" content="
              "\"text/html; charset=iso-8859-1\"/>\n";
  rptpage  += "  <style type=\"text/css\" >\n";
  rptpage  += "    td { padding-right: 0.75em; }\n";
  rptpage  += "    body { background-color: white; }\n";
  rptpage  += "    .pagebreak\n";
  rptpage  += "    {\n";
  rptpage  += "      page-break-before: always; border: 1px solid; \n";
  rptpage  += "    }\n";
  rptpage  += "    .parahead\n";
  rptpage  += "    {\n";
  rptpage  += "      font-weight: bold;\n";
  rptpage  += "      font-style:  italic;\n";
  rptpage  += "    }\n";
  rptpage  += "    .datatext\n";
  rptpage  += "    {\n";
  rptpage  += "      font-family: monospace;\n";
  rptpage  += "    }\n";

  //rptpage  += "   @media print { footer { position: fixed; bottom: 0; } }";
  //rptpage  += "   footer { position: absolute; bottom: 0; }";
  
  // rptpage  += "  div.footer { display: block; text-align: center;  position: running(footer);";
  // rptpage  += "  @page { @bottom-center { content: element(footer) }}";
  
  rptpage  += "  </style>\n";
  rptpage  += "  </head>\n  <body>\n";

  QString html_title = tr(
			  "<h2 align=center>Audit Trail for GMP Run: <br><i>%1</i></h2>"
			  "<hr>"
			  )
    .arg( gmpRunName_passed )       //1                      
    ;
  
  html_assembled +=
    rptpage
    + html_title;

}

//close HTML
void US_auditTrailGMP::closeHTML( void )
{
  //do we need close remark?
  html_assembled += "</body>\n</html>";
}

//eSigs HTML
void US_auditTrailGMP::assemble_esigs( QMap<QString, QString> esigs_html )
{
  QString uname = esigs_html[ "Name" ].split(".")[1].simplified();
  QString uid   = esigs_html[ "Name" ].split(".")[0].simplified();

  QString name_c = uname + " (ID=" + uid + ")";
  
  html_assembled_esigs += tr( "<h3 style=\"margin-left:10px\">%1</h3>" )
    .arg( name_c );

  QString eStatus;
  if ( esigs_html[ "Status" ].contains("NOT") ) 
    eStatus = "<td style=\"color:red;\"><b><i>" + esigs_html[ "Status" ] + "</i></b></td>";
  else
    eStatus = "<td style=\"color:green;\"><b><i>" + esigs_html[ "Status" ] + "</i></b></td>";
  
  html_assembled_esigs += tr(
			     "<table style=\"margin-left:30px\">"
			     "<tr><td>Role:        </td> <td> %1 </td></tr>"
			     "<tr><td>Status:      </td>      %2      </tr>"
			     "<tr><td>Comment:     </td> <td> %3 </td></tr>"
			     "<tr><td>e-Signed at: </td> <td> %4 </td></tr>"
			     "</table>"
			     )
    .arg( esigs_html[ "Role" ] )                   //1
    .arg( eStatus )                                //2
    .arg( esigs_html[ "Comment" ] )                //3
    .arg( esigs_html[ "Date" ] )                   //4
    ;
  
}
