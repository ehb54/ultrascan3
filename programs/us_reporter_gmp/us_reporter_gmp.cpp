#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_reporter_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_protocol_util.h"
#include "us_math2.h"

// Constructor
US_ReporterGMP::US_ReporterGMP() : US_Widgets()
{
  setWindowTitle( tr( "GMP Report Generator"));
  setPalette( US_GuiSettings::frameColor() );
  
  // primary layouts
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );
  QVBoxLayout* leftLayout     = new QVBoxLayout();
  QVBoxLayout* rghtLayout     = new QVBoxLayout();
  QGridLayout* buttonsLayout  = new QGridLayout();
  QGridLayout* genTreeLayout  = new QGridLayout();
  QGridLayout* perChanTreeLayout  = new QGridLayout();
  mainLayout->setSpacing        ( 2 );
  mainLayout->setContentsMargins( 2, 2, 2, 2 );
  leftLayout->setSpacing        ( 0 );
  leftLayout->setContentsMargins( 0, 1, 0, 1 );
  rghtLayout->setSpacing        ( 0 );
  rghtLayout->setContentsMargins( 0, 1, 0, 1 );
  buttonsLayout->setSpacing     ( 1 );
  buttonsLayout->setContentsMargins( 0, 0, 0, 0 );
  genTreeLayout->setSpacing        ( 1 );
  genTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  perChanTreeLayout->setSpacing        ( 1 );
  perChanTreeLayout->setContentsMargins( 0, 0, 0, 0 );

  //leftLayout
  QLabel*      bn_actions     = us_banner( tr( "Actions:" ), 1 );
  QLabel*      lb_loaded_run  = us_label( tr( "Loaded Run:" ) );
  le_loaded_run               = us_lineedit( tr(""), 0, true );

  QPushButton* pb_loadrun       = us_pushbutton( tr( "Load GMP Run" ) );
  pb_gen_report    = us_pushbutton( tr( "Generate Report" ) );
  pb_view_report   = us_pushbutton( tr( "View Report" ) );
  pb_select_all    = us_pushbutton( tr( "Select All" ) );
  pb_unselect_all  = us_pushbutton( tr( "Unselect All" ) );
  pb_expand_all    = us_pushbutton( tr( "Expand All" ) );
  pb_collapse_all  = us_pushbutton( tr( "Collapse All" ) );
  pb_help          = us_pushbutton( tr( "Help" ) );
  pb_close         = us_pushbutton( tr( "Close" ) );
		
  int row           = 0;
  buttonsLayout->addWidget( bn_actions,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_loaded_run,  row,   0, 1, 2 );
  buttonsLayout->addWidget( le_loaded_run,  row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_loadrun,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_gen_report,  row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_view_report, row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_select_all,  row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_unselect_all,row++, 6, 1, 6 );
  buttonsLayout->addWidget( pb_expand_all,  row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_collapse_all,row++, 6, 1, 6 );

  buttonsLayout->addWidget( pb_help,        row,   0, 1, 6, Qt::AlignBottom );
  buttonsLayout->addWidget( pb_close,       row++, 6, 1, 6, Qt::AlignBottom );

  pb_gen_report  ->setEnabled( false );
  pb_view_report ->setEnabled( false );
  pb_select_all  ->setEnabled( false );
  pb_unselect_all->setEnabled( false );
  pb_expand_all  ->setEnabled( false );
  pb_collapse_all->setEnabled( false );
  
  connect( pb_help,    SIGNAL( clicked()      ),
	   this,       SLOT(   help()         ) );
  connect( pb_close,   SIGNAL( clicked()      ),
	   this,       SLOT(   close()        ) );

  connect( pb_loadrun,      SIGNAL( clicked()      ),
	   this,            SLOT(   load_gmp_run()   ) );
  connect( pb_gen_report,   SIGNAL( clicked()      ),
	   this,            SLOT(   generate_report()   ) );
  connect( pb_view_report,  SIGNAL( clicked()      ),
	   this,            SLOT(   view_report()   ) );
  connect( pb_select_all,   SIGNAL( clicked()      ),
	   this,            SLOT( select_all()   ) );
  connect( pb_unselect_all, SIGNAL( clicked()      ),
	   this,            SLOT(   unselect_all()   ) );
  connect( pb_expand_all,   SIGNAL( clicked()      ),
	   this,            SLOT( expand_all()   ) );
  connect( pb_collapse_all, SIGNAL( clicked()      ),
	   this,            SLOT(   collapse_all()   ) ); 
    
  //rightLayout: genTree
  QLabel*      lb_gentree  = us_banner(      tr( "General Report Profile Settings:" ), 1 );
  QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
  genTree = new QTreeWidget();
  QStringList theads;
  theads << "Selected" << "Protocol Settings";
  genTree->setHeaderLabels( theads );
  genTree->setFont( QFont( US_Widgets::fixedFont().family(),
			      US_GuiSettings::fontSize() + 1 ) );
  genTree->installEventFilter   ( this );
  genTreeLayout->addWidget( lb_gentree );
  genTreeLayout->addWidget( genTree );
  
  genTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //rightLayout: perChannel tree
  QLabel*      lb_chantree  = us_banner(      tr( "Per-Triple Report Profile Settings:" ), 1 );
  perChanTree = new QTreeWidget();
  QStringList chan_theads;
  chan_theads << "Selected" << "Protocol Settings";
  perChanTree->setHeaderLabels( theads );
  perChanTree->setFont( QFont( US_Widgets::fixedFont().family(),
			       US_GuiSettings::fontSize() + 1 ) );
  perChanTreeLayout->addWidget( lb_chantree );
  perChanTreeLayout->addWidget( perChanTree );
  perChanTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  
  // put layouts together for overall layout
  leftLayout->addLayout( buttonsLayout );
  leftLayout->addStretch();
  rghtLayout->addLayout( genTreeLayout );
  rghtLayout->addLayout( perChanTreeLayout );

  mainLayout->addLayout( leftLayout );
  mainLayout->addLayout( rghtLayout );
  mainLayout->setStretchFactor( leftLayout, 6 );
  mainLayout->setStretchFactor( rghtLayout, 8 );
  
  resize( 1350, 800 );
}

//load GMP run
void US_ReporterGMP::load_gmp_run ( void )
{
  list_all_autoflow_records( autoflowdata  );

  QString pdtitle( tr( "Select GMP Run" ) );
  QStringList hdrs;
  int         prx;
  hdrs << "ID"
       << "Run Name"
       << "Optima Name"
       << "Created"
       << "Run Status"
       << "Stage"
       << "GMP";
  
  QString autoflow_btn = "AUTOFLOW_GMP_REPORT";

  pdiag_autoflow = new US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -2 );

  QString autoflow_id_selected("");
  if ( pdiag_autoflow->exec() == QDialog::Accepted )
    {
      autoflow_id_selected  = autoflowdata[ prx ][ 0 ];

      //reset Gui && internal structures
      reset_report_panel();
    }
  else
    return;

  
  //show progress dialog
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 7, this);
  progress_msg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setModal( true );
  progress_msg->setWindowTitle(tr("Assessing Run's Protocol"));
  QFont font_d  = progress_msg->property("font").value<QFont>();
  QFontMetrics fm(font_d);
  int pixelsWide = fm.width( progress_msg->windowTitle() );
  qDebug() << "Progress_msg: pixelsWide -- " << pixelsWide;
  progress_msg ->setMinimumWidth( pixelsWide*2 );
  progress_msg->adjustSize();
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );
  
  protocol_details[ "autoflowID" ] = QString::number(autoflowID);

  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];

  progress_msg->setValue( 1 );
  qApp->processEvents();

  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

  qDebug() << "Exp. Label: "    << protocol_details[ "label" ];
  qDebug() << "GMP Run ? "      << protocol_details[ "gmpRun" ];

  qDebug() << "AnalysisIDs: "   << protocol_details[ "analysisIDs" ];
  qDebug() << "aprofileguid: "  << AProfileGUID ;
  

  //Now, read protocol's 'reportMask' && reportItems masks && populate trees
  read_protocol_and_reportMasks( );

  build_genTree();  
  progress_msg->setValue( 6 );
  qApp->processEvents();

  build_perChanTree();
  progress_msg->setValue( 7 );
  qApp->processEvents();

  progress_msg->setValue( progress_msg->maximum() );
  progress_msg->close();

  //Enable some buttons
  le_loaded_run   ->setText( protocol_details[ "filename" ] );
  pb_gen_report   ->setEnabled( true );
  pb_view_report  ->setEnabled( false );
  pb_select_all   ->setEnabled( true );
  pb_unselect_all ->setEnabled( true );
  pb_expand_all   ->setEnabled( true );
  pb_collapse_all ->setEnabled( true );
}

// Query autoflow (history) table for records
int US_ReporterGMP::list_all_autoflow_records( QList< QStringList >& autoflowdata )
{
  int nrecs        = 0;   
  autoflowdata.clear();
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return nrecs;
    }
  
  QStringList qry;
  qry << "get_autoflow_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList autoflowentry;
      QString id                 = db->value( 0 ).toString();
      QString runname            = db->value( 5 ).toString();
      QString status             = db->value( 8 ).toString();
      QString optimaname         = db->value( 10 ).toString();
      
      QDateTime time_started     = db->value( 11 ).toDateTime().toUTC();

      QDateTime time_created     = db->value( 13 ).toDateTime().toUTC();
      QString gmpRun             = db->value( 14 ).toString();
      QString full_runname       = db->value( 15 ).toString();
      
      QDateTime local(QDateTime::currentDateTime());

      autoflowentry << id << full_runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

      if ( time_started.toString().isEmpty() )
	autoflowentry << QString( tr( "NOT STARTED" ) );
      else
	{
	  if ( status == "LIVE_UPDATE" )
	    autoflowentry << QString( tr( "RUNNING" ) );
	  if ( status == "EDITING" || status == "EDIT_DATA" || status == "ANALYSIS" || status == "REPORT" )
	    autoflowentry << QString( tr( "COMPLETED" ) );
	    //autoflowentry << time_started.toString();
	}

      if ( status == "EDITING" )
	status = "LIMS_IMPORT";
      
      autoflowentry << status << gmpRun;

      if ( !full_runname.isEmpty() )
	autoflowdata  << autoflowentry;

      nrecs++;
    }

  return nrecs;
}


// Query autoflow for # records
QMap< QString, QString>  US_ReporterGMP::read_autoflow_record( int autoflowID  )
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
	   protocol_details[ "label" ]          = db->value( 15 ).toString();
	   protocol_details[ "gmpRun" ]         = db->value( 16 ).toString();

	   protocol_details[ "filename" ]       = db->value( 17 ).toString();
	   protocol_details[ "aprofileguid" ]   = db->value( 18 ).toString();

	   protocol_details[ "analysisIDs" ]   = db->value( 19 ).toString();
	   	   
	 }
     }

   return protocol_details;
}

//read protocol's rpeortMask
void US_ReporterGMP::read_protocol_and_reportMasks( void )
{
  //read protocol into US_RunProtocol structure:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return;
    }


  //read protocol
  progress_msg->setValue( 2 );
  qApp->processEvents();
  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  QXmlStreamReader xmli( xmlstr );
  currProto. fromXml( xmli );
  progress_msg->setValue( 3 );
  qApp->processEvents();
  
  //Debug: protocol
  qDebug() << "Protocols' details: -- "
	   << currProto.investigator
	   << currProto.runname
	   << currProto.protoname
	   << currProto.protoID
	   << currProto.project
	   << currProto.temperature
	   << currProto.temeq_delay
	   << currProto.exp_label;

  //read AProfile into US_AnaProfile structure
  sdiag = new US_AnalysisProfileGui;
  sdiag->inherit_protocol( &currProto );
  progress_msg->setValue( 4 );
  qApp->processEvents();
  
  currAProf              = sdiag->currProf;
  currAProf.protoGUID    = currProto.protoGUID;
  currAProf.protoID      = currProto.protoID;
  currAProf.protoname    = currProto.protoname;
  //2DSA parms
  cAP2                   = currAProf.ap2DSA;
  //PCSA parms
  cAPp                   = currAProf.apPCSA;
  //Channel descriptions
  chndescs               = currAProf.chndescs;
  //Channel alt_descriptions
  chndescs_alt           = currAProf.chndescs_alt;
  //Channel reports
  ch_reports             = currAProf.ch_reports;
  ch_reports_internal    = currAProf.ch_reports;
  //Channel wavelengths
  ch_wvls                = currAProf.ch_wvls;

  //report Mask
  QString gen_reportMask = currAProf.report_mask;
  parse_gen_mask_json( gen_reportMask );

  progress_msg->setValue( 5 );
  qApp->processEvents();

  qDebug() << "General ReportMask: " << gen_reportMask;
}

//parse JSON for general rpeort mask
void US_ReporterGMP::parse_gen_mask_json ( const QString reportMask  )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( reportMask.toUtf8() );
  json = jsonDoc.object();

  topLevelItems = json.keys();
  
  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();
      
      if ( key.contains("Solutions") || key.contains("Analysis") )
	{
	   QJsonArray json_array = value.toArray();
	   for (int i=0; i < json_array.size(); ++i )
	     {
	       foreach(const QString& array_key, json_array[i].toObject().keys())
		 {
		   if (  key.contains("Solutions") )
		     {
		       solutionItems      << array_key;
		       solutionItems_vals << json_array[i].toObject().value(array_key).toString(); 
		     }
		   if (  key.contains("Analysis") )
		     {
		       QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();
		       analysisItems << array_key;

		       foreach ( const QString& n_key, newObj.keys() )
			 {
			   if ( array_key.contains("General") )
			     {
			       analysisGenItems << n_key;
			       analysisGenItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("2DSA") )
			     {
			       analysis2DSAItems << n_key;
			       analysis2DSAItems_vals << newObj.value( n_key ).toString();
			     }
			   if ( array_key.contains("PCSA") ) 
			     {
			       analysisPCSAItems << n_key;
			       analysisPCSAItems_vals << newObj.value( n_key ).toString();
			     }
			 }
		     }
		 }
	     }
	}
    }

  qDebug() << "solutionItems: " << solutionItems;
  qDebug() << "solutionItems_vals: " << solutionItems_vals;

  qDebug() << "analysisItems: " << analysisItems;
  
  qDebug() << "analysisGenItems: " << analysisGenItems;
  qDebug() << "analysisGenItems_vals: " << analysisGenItems_vals;

  qDebug() << "analysis2DSAItems: " << analysis2DSAItems;
  qDebug() << "analysis2DSAItems_vals: " << analysis2DSAItems_vals;

  qDebug() << "analysisPCSAItems: " << analysisPCSAItems;
  qDebug() << "analysisPCSAItems_vals: " << analysisPCSAItems_vals;

}

//build general report mask tree
void US_ReporterGMP::build_genTree ( void )
{
  QString indent( "  " );
  QStringList topItemNameList, solutionItemNameList, analysisItemNameList,
              analysisGenItemNameList, analysis2DSAItemNameList, analysisPCSAItemNameList;
  int wiubase = (int)QTreeWidgetItem::UserType;

  for ( int i=0; i<topLevelItems.size(); ++i )
    {
      QString topItemName = topLevelItems[i];
      topItemNameList.clear();
      topItemNameList << "" << indent + topItemName;
      topItem [ topItemName ] = new QTreeWidgetItem( genTree, topItemNameList, wiubase );

      //Solutions: add 1-level children
      if( topItemName.contains("Solutions") )
	{
	  int checked_childs = 0;
	  for ( int is=0; is<solutionItems.size(); ++is )
	    {
	      QString solutionItemName = solutionItems[ is ];
	      solutionItemNameList.clear();
	      solutionItemNameList << "" << indent.repeated( 2 ) + solutionItemName;
	      solutionItem [ solutionItemName ] = new QTreeWidgetItem( topItem [ topItemName ], solutionItemNameList, wiubase);

	      if ( solutionItems_vals[ is ].toInt() )
		{
		  solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Checked );
		  ++checked_childs;
		}
	      else
		solutionItem [ solutionItemName ] ->setCheckState( 0, Qt::Unchecked );
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	  
	}
      
      //Analysis Profile: add 2-levelchildren
      else if ( topItemName.contains("Analysis Profile") )
	{
	  int checked_childs = 0;
	  for ( int ia=0; ia < analysisItems.size(); ++ia )
	    {
	      QString analysisItemName = analysisItems[ ia ];
	      analysisItemNameList.clear();
	      analysisItemNameList << "" << indent.repeated( 2 ) + analysisItemName;
	      analysisItem [ analysisItemName ] = new QTreeWidgetItem( topItem [ topItemName ], analysisItemNameList, wiubase);
	      
	      //General analysis
	      if( analysisItemName.contains("General") )
		{
		  int checked_gen = 0;
		  for ( int iag=0; iag < analysisGenItems.size(); ++iag )
		    {
		      QString analysisGenItemName = analysisGenItems[ iag ];
		      analysisGenItemNameList.clear();
		      analysisGenItemNameList << "" << indent.repeated( 3 ) + analysisGenItemName;
		      analysisGenItem [ analysisGenItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisGenItemNameList, wiubase);

		      if ( analysisGenItems_vals[ iag ].toInt() )
			{
			  analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_gen;
			}
		      else
			analysisGenItem [ analysisGenItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_gen )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}
	      //2DSA analysis
	      if( analysisItemName.contains("2DSA") )
		{
		  int checked_2dsa = 0;
		  for ( int ia2=0; ia2 < analysis2DSAItems.size(); ++ia2 )
		    {
		      QString analysis2DSAItemName = analysis2DSAItems[ ia2 ];
		      analysis2DSAItemNameList.clear();
		      analysis2DSAItemNameList << "" << indent.repeated( 3 ) + analysis2DSAItemName;
		      analysis2DSAItem [ analysis2DSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysis2DSAItemNameList, wiubase);

		      if ( analysis2DSAItems_vals[ ia2 ].toInt() )
			{
			  analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_2dsa;
			}
		      else
			analysis2DSAItem [ analysis2DSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_2dsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}

	      //PCSA analysis
	      if( analysisItemName.contains("PCSA") )
		{
		  int checked_pcsa = 0;
		  for ( int iap=0; iap < analysisPCSAItems.size(); ++iap )
		    {
		      QString analysisPCSAItemName = analysisPCSAItems[ iap ];
		      analysisPCSAItemNameList.clear();
		      analysisPCSAItemNameList << "" << indent.repeated( 3 ) + analysisPCSAItemName;
		      analysisPCSAItem [ analysisPCSAItemName ] = new QTreeWidgetItem( analysisItem [ analysisItemName ], analysisPCSAItemNameList, wiubase);

		      if ( analysisPCSAItems_vals[ iap ].toInt() )
			{
			  analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_pcsa;
			}
		      else
			analysisPCSAItem [ analysisPCSAItemName ] ->setCheckState( 0, Qt::Unchecked );
		    }

		  if ( checked_pcsa )
		    {
		      analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_childs;
		    }
		  else
		    analysisItem [ analysisItemName ] ->setCheckState( 0, Qt::Unchecked );
		}
	    }
	  if ( checked_childs )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}

      //set checked/unchecked for top-level item
      else
	{
	  if ( json.value( topItemName ).toString().toInt() )
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    topItem [ topItemName ] ->setCheckState( 0, Qt::Unchecked );
	}
    }

  
  genTree->expandAll();    
  genTree->resizeColumnToContents( 0 );
  genTree->resizeColumnToContents( 1 );

  connect( genTree, SIGNAL( itemChanged    ( QTreeWidgetItem*, int ) ),
  	   this,    SLOT  ( changedItem_gen( QTreeWidgetItem*, int ) ) );

}

//What to check/uncheck upon change in items status
void US_ReporterGMP::changedItem_gen( QTreeWidgetItem* item, int col )
{
  if ( col == 0  ) //we deal with col 0 only...
    {
      //qDebug() << "Changed item name0" << item->text( 1 );

      //if has (nested) children items
      int children_lev1 = item->childCount();
      if ( children_lev1 )
	{
	  genTree -> disconnect();

	  for( int i = 0; i < children_lev1; ++i )
	    {
	      item->child(i)->setCheckState( 0, (Qt::CheckState) item->checkState(0) );

	      int children_lev2 = item->child(i)->childCount();
	      if ( children_lev2 )
		{
		  for( int ii = 0; ii < children_lev2; ++ii )
		    {
		      item->child(i)->child(ii)->setCheckState( 0, (Qt::CheckState) item->child(i)->checkState(0) );
		    }
		}
	    }
	  
	  connect( genTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	           this,       SLOT(   changedItem_gen( QTreeWidgetItem*, int ) ) );
	}
      
           
      //qDebug() << "Changed item name1 " << item->text( 1 );
      	
      //if has parent item
      QTreeWidgetItem* parent_item = item->parent();

      //qDebug() << "Changed item name2: " << item->text( 1 );
      
      if ( parent_item )
	{
	  //qDebug() << "Changed item name3: " << item->text( 1 );
	  genTree -> disconnect();
	  
	  //qDebug() << " Current item, " << item->text( 1 ) << ", has a parent: " << parent_item->text( 1 );
	    
	  int checked_children = 0;
	  int parent_item_children = parent_item ->childCount();
	  for( int i = 0; i < parent_item_children; ++i )
	    {
	      if ( int( parent_item->child( i )->checkState(0) ) )
		++checked_children;
	    }
	  if ( checked_children )
	    parent_item->setCheckState( 0, Qt::Checked );
	  else
	    parent_item->setCheckState( 0, Qt::Unchecked );

	  connect( genTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	           this,    SLOT(   changedItem_gen( QTreeWidgetItem*, int ) ) );
	}
    }
}

//build perChanTree
void US_ReporterGMP::build_perChanTree ( void )
{
  QString indent( "  " );
  QStringList chanItemNameList, tripleItemNameList, tripleMaskItemNameList;
  QStringList tripleReportMasksList;
  QList< bool > tripleReportMasksList_vals;
  int wiubase = (int)QTreeWidgetItem::UserType;
  
  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];


      bool triple_report = false;
      
      if ( currAProf.analysis_run[ i ] )
	{
	  //now check if report will be run:
	  QString run_report;
	  if ( currAProf.report_run[ i ] )
	    triple_report = true;
	}

      if ( triple_report )
	{
	  // Channel Name: topItem in a perChanTree
	  QString chanItemName = "Channel: " + channel_desc_alt.section( ":", 0, 1 );
	  chanItemNameList.clear();
	  chanItemNameList << "" << indent + chanItemName;
	  chanItem [ chanItemName ] = new QTreeWidgetItem( perChanTree, chanItemNameList, wiubase );
	  
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc ];
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];
	  
	  int chann_wvl_number = chann_wvls.size();

	  int checked_triples = 0;
	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;

	      //Triple item: child-level 1 in a perChanTree
	      QString tripleItemName = "Triple:  " + wvl + " nm";
	      tripleItemNameList.clear();
	      tripleItemNameList << "" << indent.repeated( 2 ) + tripleItemName;
	      tripleItem [ tripleItemName ] = new QTreeWidgetItem( chanItem [ chanItemName ], tripleItemNameList, wiubase);

	      US_ReportGMP reportGMP = chann_reports[ wvl ];
	      
	      qDebug() << reportGMP. tot_conc_mask
		       << reportGMP. rmsd_limit_mask
		       << reportGMP. av_intensity_mask
		       << reportGMP. experiment_duration_mask
		       << reportGMP. integration_results_mask;

	      tripleReportMasksList.clear();
	      tripleReportMasksList << "Total Concentration"
				    << "RMSD Limit"
				    << "Minimum Intensity"
				    << "Experiment Duration"
				    << "Integraiton Results";
	      
	      tripleReportMasksList_vals.clear();
	      tripleReportMasksList_vals << reportGMP. tot_conc_mask
	      				 << reportGMP. rmsd_limit_mask
	      				 << reportGMP. av_intensity_mask
	      				 << reportGMP. experiment_duration_mask
	      				 << reportGMP. integration_results_mask;

	      int checked_masks = 0;
	      for ( int kk = 0; kk < tripleReportMasksList.size(); ++kk )
		{
		  //Triple's mask params: child-level 2 in a perChanTree
		  QString tripleMaskItemName = tripleReportMasksList[ kk ];
		  tripleMaskItemNameList.clear();
		  tripleMaskItemNameList << "" << indent.repeated( 3 ) + tripleMaskItemName;
		  tripleMaskItem [ tripleItemName ] = new QTreeWidgetItem(  tripleItem [ tripleItemName ], tripleMaskItemNameList, wiubase);

		  if ( tripleReportMasksList_vals[ kk ] )
		    {
		      tripleMaskItem [ tripleItemName ] ->setCheckState( 0, Qt::Checked );
		      ++checked_masks;
		    }
		  else
		    tripleMaskItem [ tripleItemName ] ->setCheckState( 0, Qt::Unchecked );
		}

	      if ( checked_masks )
		{
		  tripleItem [ tripleItemName ] ->setCheckState( 0, Qt::Checked );
		  ++checked_triples;
		}
	      else
		tripleItem [ tripleItemName ] ->setCheckState( 0, Qt::Unchecked );
	    }
	  
	  //set checked/unchecked for channel (parent)
	  if ( checked_triples )
	    chanItem [ chanItemName ] ->setCheckState( 0, Qt::Checked );
	  else
	    chanItem [ chanItemName ] ->setCheckState( 0, Qt::Unchecked );
	}
    }

  perChanTree->expandAll();    
  perChanTree->resizeColumnToContents( 0 );
  perChanTree->resizeColumnToContents( 1 );

  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	   this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
}

//What to check/uncheck upon change in items status
void US_ReporterGMP::changedItem_triple( QTreeWidgetItem* item, int col )
{
  if ( col == 0  ) //we deal with col 0 only...
    {
      //qDebug() << "Changed item name0" << item->text( 1 );

      //if has (nested) children items
      int children_lev1 = item->childCount();
      if ( children_lev1 )
	{
	  perChanTree -> disconnect();

	  for( int i = 0; i < children_lev1; ++i )
	    {
	      item->child(i)->setCheckState( 0, (Qt::CheckState) item->checkState(0) );

	      int children_lev2 = item->child(i)->childCount();
	      if ( children_lev2 )
		{
		  for( int ii = 0; ii < children_lev2; ++ii )
		    {
		      item->child(i)->child(ii)->setCheckState( 0, (Qt::CheckState) item->child(i)->checkState(0) );
		    }
		}
	    }
	  
	  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	           this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
	}
      
           
      //qDebug() << "Changed item name1 " << item->text( 1 );
      	
      //if has parent item
      QTreeWidgetItem* parent_item = item->parent();

      //qDebug() << "Changed item name2: " << item->text( 1 );
      
      if ( parent_item )
	{
	  //qDebug() << "Changed item name3: " << item->text( 1 );
	  perChanTree -> disconnect();
	  
	  //qDebug() << " Current item, " << item->text( 1 ) << ", has a parent: " << parent_item->text( 1 );
	    
	  int checked_children = 0;
	  int parent_item_children = parent_item ->childCount();
	  for( int i = 0; i < parent_item_children; ++i )
	    {
	      if ( int( parent_item->child( i )->checkState(0) ) )
		++checked_children;
	    }
	  if ( checked_children )
	    parent_item->setCheckState( 0, Qt::Checked );
	  else
	    parent_item->setCheckState( 0, Qt::Unchecked );

	  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	           this,        SLOT(   changedItem_triple( QTreeWidgetItem*, int ) ) );
	}
    }
}


//select all items in trees
void US_ReporterGMP::select_all ( void )
{
  QTreeWidgetItem* getTree_rootItem     = genTree     -> invisibleRootItem();
  for( int i = 0; i < getTree_rootItem->childCount(); ++i )
    {
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
    }
  
  QTreeWidgetItem* perChanTree_rootItem = perChanTree -> invisibleRootItem();
  for( int i = 0; i < perChanTree_rootItem->childCount(); ++i )
    {
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
    }
}


//unselect all items in trees
void US_ReporterGMP::unselect_all ( void )
{
  QTreeWidgetItem* getTree_rootItem     = genTree     -> invisibleRootItem();
  for( int i = 0; i < getTree_rootItem->childCount(); ++i )
    {
      getTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }
  
  QTreeWidgetItem* perChanTree_rootItem = perChanTree -> invisibleRootItem();
  for( int i = 0; i < perChanTree_rootItem->childCount(); ++i )
    {
      perChanTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }
}

//expand all items in trees
void US_ReporterGMP::expand_all ( void )
{
  genTree     ->expandAll();
  perChanTree ->expandAll();
}

//collapse all items in trees
void US_ReporterGMP::collapse_all ( void )
{
  genTree       ->collapseAll();
  perChanTree   ->collapseAll();
}

//view report
void US_ReporterGMP::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;
  
  //Open with OS's applicaiton settings ?
  QDesktopServices::openUrl(QUrl( filePath ));
}

//reset
void US_ReporterGMP::reset_report_panel ( void )
{
  le_loaded_run ->setText( "" );

  //cleaning genTree && it's objects
  // for (int i = 0; i < genTree->topLevelItemCount(); ++i)
  //   {
  //     qDeleteAll(genTree->topLevelItem(i)->takeChildren());
  //   }
  genTree     ->clear();
  qApp->processEvents();
  
  topItem.clear();
  solutionItem.clear();
  analysisItem.clear();
  analysisGenItem.clear();
  analysis2DSAItem.clear();
  analysisPCSAItem.clear();

  topLevelItems.clear();
  solutionItems.clear();
  solutionItems_vals.clear();
  analysisItems.clear();
  analysisGenItems.clear();
  analysisGenItems_vals.clear();
  analysis2DSAItems.clear();
  analysis2DSAItems_vals.clear();
  analysisPCSAItems.clear();
  analysisPCSAItems_vals.clear();

  //cleaning perTriple tree & it's objects
  perChanTree ->clear();
  qApp->processEvents();

  chanItem   .clear();
  tripleItem .clear();
  tripleMaskItem . clear();
  
  //reset US_Protocol && US_AnaProfile
  currProto = US_RunProtocol();  
  currAProf = US_AnaProfile();   

  //reset html assembled strings
  html_general.clear();
  html_lab_rotor.clear();
  html_operator.clear();
  html_speed.clear();
  html_cells.clear();
  html_solutions.clear();
  html_optical.clear();
  html_ranges.clear();
  html_scan_count.clear();
  html_analysis_profile.clear();
  html_analysis_profile_2dsa.clear();
  html_analysis_profile_pcsa .clear();

  qApp->processEvents();
}


//Generate report
void US_ReporterGMP::generate_report( void )
{
  gui_to_parms();
  
  get_current_date();
  format_needed_params();
  assemble_pdf();

  pb_view_report->setEnabled( true );

  //Inform user of the PDF location
  QMessageBox::information( this, tr( "Report PDF Ready" ),
			    tr( "Report PDF was saved at \n%1\n\n"
				"You can view it by pressing \'View Report\' button on the left" ).arg( filePath ) );
}

//Start assembling PDF file
void US_ReporterGMP::assemble_pdf()
{
  //HEADER: begin
  QString html_header = tr( 
    "<div align=left>"
      "Created, %1<br>"
      "with UltraScan-GMP<br>"
      "by AUC Solutions<br>"
    "</div>"
		     )
    .arg( current_date )
    ;
  //HEADER: end

  
  //TITLE: begin
  QString html_title = tr(
    "<h1 align=center>REPORT FOR RUN <br><i>%1</i></h1>"
    "<hr>"
			  )
    .arg( currProto. protoname + "-run" + runID )    //1
    ;
  //TITLE: end

  QString html_paragraph_open = tr(
    "<p align=justify>"
				   )
    ;
  
  //GENERAL: begin
  html_general = tr(
    
    "<h3 align=left>General Settings</h3>"
      "<table>"
        "<tr><td>Investigator: </td>                           <td>%1</td></tr>"
        "<tr><td>Run Name:</td>                                <td>%2</td></tr>"
        "<tr><td>Project:</td>                                 <td>%3</td></tr>"
        "<tr><td>Run Temperature (&#8451;):</td>               <td>%4</td></tr>"
        "<tr><td>Temperature-Equilibration Delay (mins):</td>  <td>%5</td></tr>"
      "</table>"
    "<hr>"
			    )
    .arg( currProto. investigator)  //1
    //.arg( currProto. runname)       //2  
    .arg( currProto. protoname + "-run" + runID) //2
    .arg( currProto. project)       //3
    .arg( currProto. temperature)   //4
    .arg( currProto. temeq_delay)   //5
    ;
  //GENERAL: end


  //ROTOR/LAB: begin
  html_lab_rotor = tr(
    "<h3 align=left>Lab/Rotor Parameters</h3>"
      "<table>"
        "<tr><td>Laboratory:</td>      <td>%1</td></tr>"
        "<tr><td>Rotor: </td>          <td>%2</td></tr>"
        "<tr><td>Calibration Date:</td><td>%3</td></tr>"
      "</table>"
    "<hr>"
			      )
    .arg( currProto. rpRotor.laboratory )  //1
    .arg( currProto. rpRotor.rotor )       //2
    .arg( currProto. rpRotor.calibration)  //3
    ;
  //ROTOR/LAB: end 	      
  
  //OPERATOR: begin
  html_operator = tr(     
    "<h3 align=left>Optima Machine/Operator </h3>"
      "<table>"
        "<tr><td>Optima: </td>           <td>%1</td></tr>"
        "<tr><td>Operator: </td>         <td>%2</td></tr>"
        "<tr><td>Experiment Type:</td>   <td>%3</td></tr>"
      "</table>"
    "<hr>"
				  )
    .arg( currProto. rpRotor.instrname )   //1
    .arg( currProto. rpRotor.opername  )   //2
    .arg( currProto. rpRotor.exptype )     //3
    ;
  //OPERATOR: end 	  

  
  //SPEEDS: begin
  html_speed = tr(
    "<h3 align=left>Speed Parameters </h3>"
      "<table>"
        "<tr><td>Rotor Speed  (RPM):    </td>                   <td>%1</td></tr>"
        "<tr><td>Acceleration (RMP/sec): </td>                  <td>%2</td></tr>"
        "<tr><td>Active Scaning Time:    </td>                  <td>%3</td></tr>"
        "<tr><td>Stage Delay:          </td>                    <td>%4</td></tr>"
        "<tr><td>Total Time (without equilibration):  </td>     <td>%5</td></tr>"
        "<tr><td><br><b><i>UV-visible optics (total):</i></b>   </td>  "
        "<tr><td>Delay to First Scan:            </td>          <td>%6</td></tr>"
        "<tr><td>Scan Interval:                  </td>          <td>%7</td></tr>"
        "<tr><td><br><b><i>Interference optics (per cell):</i></b></td>   "
        "<tr><td>Delay to First Scan:  </td>                    <td>%8</td></tr>"
        "<tr><td>Scan Interval:        </td>                    <td>%9</td></tr>"
      "</table>"
    "<hr>"
			  )
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].speed ) )       //1
    .arg( QString::number ( currProto. rpSpeed. ssteps[0].accel ) )       //2
    .arg( duration_str    )                                               //3
    .arg( delay_stage_str )                                               //4
    .arg( total_time_str  )                                               //5
    .arg( delay_uvvis_str )                                               //6
    .arg( scanint_uvvis_str )                                             //7
    .arg( delay_int_str   )                                               //8
    .arg( scanint_int_str )                                               //9
    ;
  //SPEEDS: end


  //CELLS: begin
  html_cells = tr(
    "<h3 align=left>Cell Centerpiece Usage </h3>"
			   )
     ;
   
  ncells_used = currProto. rpCells. nused;
  html_cells += tr(
     "# of Used Cells:  %1<br>"
		    )
    .arg( QString::number ( ncells_used ) )                                        //1
    ;
   
  html_cells += tr(
      "<table>"
		   )
    ;

  for ( int i=0; i<ncells_used; ++i )
    {
      QString cell        = QString::number( currProto. rpCells. used[i].cell );
      QString centerpiece = currProto. rpCells. used[i].centerpiece;
      QString cell_label  = QString( tr( "Centerpiece" ) );
      QString window      = currProto. rpCells. used[i].windows;
      
      //check if last cell counterbalance
      if ( i == ncells_used - 1 && 
	   currProto. rpCells. used[i].cbalance.contains( tr("counterbalance") ) )
	{
	  cell_label = tr("Counterbalance");
	  centerpiece = currProto. rpCells. used[i].cbalance;
	  window = tr("N/A");
	}
	
      html_cells += tr(
		       "<tr>" 
		       "<td>Cell Number:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;" 
		       "<td> %2: </td><td> %3 </td> &nbsp;&nbsp;&nbsp;&nbsp;"
		       "<td>Windows:</td><td> %4 </td>"
		       "</tr>"
			)
	.arg( cell )                          //1
	.arg( cell_label )                    //2
	.arg( centerpiece )                   //3
	.arg( window )                        //4 
	;
    }
  
  html_cells += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //CELLS: end

  
  //SOLUTIONS: begin
  html_solutions = tr(
		      "<h3 align=left>Solutions for Channels</h3>"			      
		      )
    ;
   
  nsol_channels = currProto.rpSolut.nschan;
  html_solutions += tr(
     "# of Solution Channels:  %1 <br>"
		    )
    .arg( nsol_channels )                                                   //1
    ;
  
  for ( int i=0; i<nsol_channels; ++i )
    {
      html_solutions += tr(
			   "<table>"		   
			   "<tr>"
			      "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			)
	.arg( currProto. rpSolut. chsols[i].channel )                       //1
	;

      QString sol_id      = currProto. rpSolut. chsols[i].sol_id;
      QString sol_comment = currProto. rpSolut. chsols[i].ch_comment;
      add_solution_details( sol_id, sol_comment, html_solutions );
    }

  html_solutions += tr( "<hr>" );
  //SOLUTIONS: end


  //OPTICS: begin
  html_optical = tr(
		    "<h3 align=left>Optical Systems </h3>"
		    )
    ;
  nchan_optics = currProto. rpOptic. nochan;
  html_optical += tr(
		     "# of Channels With Optics:  %1<br>"
		     )
    .arg( QString::number( nchan_optics ))                                                     //1
    ;
  html_optical += tr(
		     "<table>"
		     );

  for ( int i=0; i<nchan_optics; ++i )
    {
      QString channel  = currProto. rpOptic. chopts[i].channel;
      QString scan1    = currProto. rpOptic. chopts[i].scan1;
      QString scan2    = currProto. rpOptic. chopts[i].scan2;
      QString scan3    = currProto. rpOptic. chopts[i].scan3;

      // //test
      // if ( i == 3 )
      // 	scan2 = tr( "Interf." );
      // if ( i == nchan_optics - 2 )
      // 	scan3 = tr( "Fluorescense" );
      // if ( i == nchan_optics - 1 )
      // 	scan2 = tr( "Interference" );
      // //////

      
      html_optical += tr(
			 "<tr>" 
			    "<td>Cell/Channel:</td><td>%1</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			 )
	.arg( channel );                      //1
      
      
      if ( !scan1.isEmpty() )
	{
	  has_uvvis = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan1 );                    //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>");
      
      if ( !scan2.isEmpty() )
	{
	  has_interference = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan2 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
      
      if ( !scan3.isEmpty() )
	{
	  has_fluorescense = true;
	  html_optical += tr(
			     "<td> %1: </td> &nbsp;&nbsp;&nbsp;&nbsp;"
			     )
	    .arg( scan3 );                        //1
	}
      else
	html_optical += tr("<td> &nbsp;&nbsp;&nbsp;&nbsp; </td>" );    
           
      
      html_optical += tr(
			 "</tr>"
			 );
    }
  
  html_optical += tr(
      "</table>"
    "<hr>"
		   )
    ;
  //OPTICS: end


  //RANGES: begin
  html_ranges = tr(
		   "<h3 align=left> Ranges </h3>"
		   )
    ;
  nchan_ranges  = currProto. rpRange. nranges;
  html_ranges  += tr(
		     "# of Channels With Ranges:  %1<br>"
		     )
    .arg( QString::number( nchan_ranges ))                                  //1
    ;
  
  for ( int i=0; i < nchan_ranges; ++i )
    {
      html_ranges += tr(
			"<table>"		   
			  "<tr>"
			    "<td><b>Cell/Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			  "</tr>"
			"</table>"
			)
	.arg( currProto. rpRange. chrngs[i].channel )                       //1
	;

      int w_count     = currProto. rpRange. chrngs[i].wvlens.size();
      double  w_min   = currProto. rpRange. chrngs[i].wvlens[0];
      double  w_max   = currProto. rpRange. chrngs[i].wvlens[ w_count - 1 ];
      double  r_min   = currProto. rpRange. chrngs[i].lo_rad;
      double  r_max   = currProto. rpRange. chrngs[i].hi_rad;
      QString w_range = QString::number( w_min ) + tr(" to ") + QString::number( w_max );
      QString r_range = QString::number( r_min ) + tr(" to ") + QString::number( r_max );

      //wavelengths:
      QString all_wvl = QString("");
      QList< double > wvl_list = currProto. rpRange. chrngs[i].wvlens;
      int counter = 0;
      for (int  jj =0; jj < w_count; jj++)
	{
	  ++counter;
	  all_wvl += tr("%1").arg( wvl_list[jj] );
         if( jj != wvl_list.size() -1 )
	   all_wvl += tr(", ");
         if(counter % 8 == 0)
	   all_wvl += tr("<br>");
	}
      
      html_ranges += tr(
			"<table style=\"margin-left:30px\">"
			  "<tr><td> Selected Wavelength count: </td>  <td> %1 </td> </tr>"
			  "<tr><td> Selected Wavelength range: </td>  <td> %2 </td> </tr>"
			  "<tr><td> Radius range:              </td>  <td> %3 </td> </tr>"
			  "<tr><td> Selected Wavelengths:      </td>  <td> %4 </td> </tr>"
			"</table>"
			)
	.arg( QString::number( w_count ) )        //1
	.arg( w_range )                           //2
	.arg( r_range )                           //3
	.arg( all_wvl )                           //4	
	;
      
    }  

  html_ranges += tr( "<hr>" ) ;
  //RANGES: end

  
  //SCAN_COUNT: begin
  html_scan_count = tr(
		       "<h3 align=left> Scan Counts and Scan Intervals For Optics in Use </h3>"
		       "&nbsp;&nbsp;<br>"
		       )
    ;

  double scanintv     = currProto. rpSpeed. ssteps[0].scanintv;
  double scanintv_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  int scancount       = currProto. rpSpeed. ssteps[0].scancount;
  int scancount_int   = currProto. rpSpeed. ssteps[0].scancount_int;

  //UV-vis
  QString html_scan_count_uv = tr(
				  "<table>"		   
				    "<tr>"
				      "<td><b><i>UV-visible optics:</i></b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_scan_count_uv += tr(
			   "<table style=\"margin-left:30px\">"
			     "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			     "<tr><td> # Scans per Triple:        </td>  <td> %2 </td> </tr>"
			   "</table>"
			   )
    .arg( scanint_uvvis_str )                             //1
    .arg( QString::number( scancount ) )                  //2
    ;
  
  //Interference
  QString html_scan_count_int = tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b><i>Interference optics:</i></b></td>"
				     "</tr>"
				   "</table>"
				   )
    ;
  html_scan_count_int += tr(
			    "<table style=\"margin-left:30px\">"
			      "<tr><td> Scan Interval:             </td>  <td> %1 </td> </tr>"
			      "<tr><td> # Scans per Cell:          </td>  <td> %2 </td> </tr>"
			    "</table>"
			   )
    .arg( scanint_int_str )                                //1
    .arg( QString::number( scancount_int ) )               //2
    ;


  if ( has_uvvis )
    html_scan_count += html_scan_count_uv;
  if ( has_interference )
    html_scan_count += html_scan_count_int;  
  
  html_scan_count += tr( "<hr>" ) ;
  //SCAN_COUNT: end

  
  //APROFILE: begin
  //ANALYSIS: General settings && Reports: begin
  html_analysis_profile = tr(
			     "<h3 align=left> Analysis Profile: General Settings and Reports  </h3>"
			     "&nbsp;&nbsp;<br>"
			     )
    ;

  //Begin of the General Analysis Section
  QString html_analysis_gen;
  
  html_analysis_gen += tr(
			  "<table>"		   
			  "<tr> <td> Profile Name:  &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %1 </td></tr>"
			  "<tr> <td> Protocol Name: &nbsp;&nbsp;&nbsp;&nbsp; </td>  <td> %2 </td></tr>"
			  "</table>"
			  "<br>"
			  )
    .arg( currAProf.aprofname  )         //1
    .arg( currAProf.protoname  )         //2
    ;

  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];
      
      html_analysis_gen += tr(
			      "<table>"		   
			      "<tr>"
			      "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			      "</tr>"
			      "</table>"
			      )
	.arg( channel_desc )              //1
	;

      QString loading_ratio  = QString::number( currAProf.lc_ratios[ i ] );
      QString ratio_tol      = QString::number( currAProf.lc_tolers[ i ] );
      QString volume         = QString::number( currAProf.l_volumes[ i ] );
      QString volume_tol     = QString::number( currAProf.lv_tolers[ i ] );
      QString data_end       = QString::number( currAProf.data_ends[ i ] );

      QString run_analysis;
      if ( currAProf.analysis_run[ i ] )
	run_analysis = tr("YES");
      else
	run_analysis = tr("NO");
      
      html_analysis_gen += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> Loading Ratio:              </td>  <td> %1 </td> </tr>"
				     "<tr><td> Ratio Tolerance (&#177;%):  </td>  <td> %2 </td> </tr>"
				     "<tr><td> Loading Volume (&#181;l):   </td>  <td> %3 </td> </tr>"
				     "<tr><td> Volume Tolerance (&#177;%): </td>  <td> %4 </td> </tr>"
				     "<tr><td> Data End (cm):              </td>  <td> %5 </td> </tr>"
				  "</table>"
				  )
	.arg( loading_ratio )             //1
	.arg( ratio_tol )                 //2
	.arg( volume )                    //3
	.arg( volume_tol )                //4
	.arg( data_end )                  //5
	;

      
      html_analysis_gen    += tr(
				  "<table style=\"margin-left:30px\">"
				     "<tr><td> <i>Run Analysis:</i>        </td>  <td> %1 </td> </tr>"
				  "</table>"
				  )
	.arg( run_analysis )               //1
	;
				  
      bool triple_report = false;
      
      if ( currAProf.analysis_run[ i ] )
	{
	  //check what representative wvl is:
	  QString rep_wvl = QString::number( currAProf.wvl_edit[ i ] );
	  html_analysis_gen += tr(
				      "<table style=\"margin-left:50px\">"
				         "<tr><td> Wavelength for Edit, 2DSA-FM & Fitmen Stages (nm):   </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( rep_wvl )                //1
	    ;
	  
	  //now check if report will be run:
	  QString run_report;
	  if ( currAProf.report_run[ i ] )
	    {
	      run_report    = tr("YES");
	      triple_report = true;
	    }
	  else
	    run_report = tr("NO");

	  html_analysis_gen     += tr(
				      "<table style=\"margin-left:30px\">"
				         "<tr><td> <i> Run Report: </i>    </td>  <td> %1 </td> </tr>"
				      "</table>"
				      )
	    .arg( run_report)              //1
	    ;
	}

      if ( genMask_edited.ShowAnalysisGenParts[ "Channel General Settings" ].toInt()  )
	{
	  html_analysis_profile += html_analysis_gen;
	  html_analysis_gen.clear();
	}
      //End of the General Analysis Section

       
      //Separate Report | ReportItems table
      if ( triple_report )
	{
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc ];
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];
	    
	  int chann_wvl_number = chann_wvls.size();

	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;
	      US_ReportGMP reportGMP = chann_reports[ wvl ];

	      //Exp. duration entered in the Channel Report Editor
	      QList< int > hms_tot;
	      double total_time = reportGMP.experiment_duration;
	      US_RunProtocol::timeToList( total_time, hms_tot );
	      QString exp_dur_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

	      if ( genMask_edited.ShowAnalysisGenParts[ "Report Parameters (per-triple)" ].toInt()  )
		{
		  html_analysis_profile += tr(
					      "<table style=\"margin-left:50px\">"
					      "<tr><td> <b><i> Report Parameters for Triple: </i> &nbsp;&nbsp;&nbsp; %1 </b></td> </tr>"
					      "</table>"
					      )
		    .arg( triple_name )                                             //1
		    ;
		  
		  html_analysis_profile += tr(
					      "<table style=\"margin-left:70px\">"
					      "<tr><td> Total Concentration:           </td>  <td> %1 </td> </tr>"
					      "<tr><td> Total Concentration Tolerance: </td>  <td> %2 </td> </tr>"
					      "<tr><td> RMSD (upper limit):            </td>  <td> %3 </td> </tr>"
					      "<tr><td> Average Intensity:             </td>  <td> %4 </td> </tr>"
					      "<tr><td> Experiment Duration:           </td>  <td> %5 </td> </tr>"
					      "<tr><td> Experiment Duration Tolerance: </td>  <td> %6 </td> </tr>"
					      "</table>"
					      )
		    .arg( QString::number( reportGMP.tot_conc ) )                    //1
		    .arg( QString::number( reportGMP.tot_conc_tol ) )                //2
		    .arg( QString::number( reportGMP.rmsd_limit )  )                 //3
		    .arg( QString::number( reportGMP.av_intensity )  )               //4
		    .arg( exp_dur_str )                                              //5
		    .arg( QString::number( reportGMP.experiment_duration_tol ) )     //6
		    ;
		}

	      //Now go over ReportItems for the current triple:
	      int report_items_number = reportGMP.reportItems.size();

	      for ( int kk = 0; kk < report_items_number; ++kk )
		{
		  US_ReportGMP::ReportItem curr_item = reportGMP.reportItems[ kk ];

		  if ( genMask_edited.ShowAnalysisGenParts[ "Report Item Parameters (per-triple)" ].toInt()  )
		    {
		      html_analysis_profile += tr(
						  "<table style=\"margin-left:90px\">"
						  "<tr><td> <b><i> Report Item #%1: </i> &nbsp;&nbsp; Type | Method Parameters </b></td> </tr>"
						  "</table>"
						  )
			.arg( QString::number( kk + 1 ) )                                 //1
			;
		      
		      html_analysis_profile += tr(
						  "<table style=\"margin-left:110px\">"
						  "<tr><td> Type:                  </td>  <td> %1 </td> </tr>"
						  "<tr><td> Method:                </td>  <td> %2 </td> </tr>"
						  "<tr><td> Range Low:             </td>  <td> %3 </td> </tr>"
						  "<tr><td> Range High:            </td>  <td> %4 </td> </tr>"
						  "<tr><td> Integration Value:     </td>  <td> %5 </td> </tr>"
						  "<tr><td> Tolerance (%):         </td>  <td> %6 </td> </tr>"
						  "<tr><td> Fraction of Total (%): </td>  <td> %7 </td> </tr>"
						  "</table>"
						  )
			.arg( curr_item.type )                                      //1
			.arg( curr_item.method   )                                  //2
			.arg( QString::number( curr_item.range_low )  )             //3
			.arg( QString::number( curr_item.range_high )  )            //4
			.arg( QString::number( curr_item.integration_val )  )       //5
			.arg( QString::number( curr_item.tolerance )  )             //6
			.arg( QString::number( curr_item.total_percent )  )         //7
			;
		    }
		}
	    }
	}
      
      html_analysis_profile += tr(
				  "<br>"
				  )
	;
    }
  html_analysis_profile += tr( "<hr>" ) ;
  //ANALYSIS: General settings && Reports: end
    
    
  //ANALYSIS: 2DSA per-channel settings: begin
  html_analysis_profile_2dsa = tr(
				  "<h3 align=left> Analysis Profile: 2DSA Controls </h3>"
				  "&nbsp;&nbsp;<br>"
				  )
    ;

  //Job Flow Summary:
  QString html_analysis_profile_2dsa_flow;
  html_analysis_profile_2dsa_flow += tr(
				   "<table>"		   
				    "<tr>"
				       "<td><b>Job Flow Summary:</b></td>"
				    "</tr>"
				  "</table>"
			)
    ;
  html_analysis_profile_2dsa_flow += tr(
				   "<table style=\"margin-left:10px\">"
				      //2DSA
				      "<tr>"
				         "<td> \"2DSA  (TI Noise)\": </td>"
				         "<td> Run?                  </td>"    
                                         "<td> %1                    </td>"
				         "<td> &nbsp;&nbsp;          </td>"
				         "<td> &nbsp;&nbsp;          </td>"
                                      "</tr>"

				      //2DSA-FM
				      "<tr>"
                                         "<td> \"2DSA-FM  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %2                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Grid Points:       </td>"
				         "<td> %3                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Meniscus Fit Range (cm):    </td>"
				         "<td> %4                          </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Fit Type:                   </td>"
				         "<td> %5                          </td>"
				      "</tr>"				   

				      //FITMEN
				      "<tr>"
				         "<td> \"FITMEN\":                 </td>"
				         "<td> Run?                        </td>"
				         "<td> %6                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Auto-pick?                  </td>"
				         "<td> %7                          </td>"
				      "</tr>"

				      //2DSA-IT
				      "<tr>"
				         "<td> \"2DSA-IT  (TI+RI Noise)\": </td>"
				         "<td> Run?                        </td>"
				         "<td> %8                          </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
                                      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> Refinement Iterations:      </td>"
				         "<td> %9                          </td>"
				      "</tr>"
				   
				      //2DSA-MC
				      "<tr>"
				         "<td> \"2DSA-MC\":                </td>"
				         "<td> Run?                        </td>"
				         "<td> %10                         </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				      "</tr>"
				      "<tr>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> &nbsp;&nbsp;                </td>"
				         "<td> MonteCarlo Iterations:      </td>"
				         "<td> %11                         </td>"
				      "</tr>"
				   
				   "</table>"
				   "<br>"
				   )
    .arg( ( cAP2.job1run ? tr( "YES" ) : tr( "NO" ) ) )       //1

    .arg( ( cAP2.job2run ? tr( "YES" ) : tr( "NO" ) ) )       //2
    .arg( QString::number( cAP2.grpoints ) )                  //3
    .arg( QString::number( cAP2.fitrng ) )                    //4
    .arg( cAP2.fmb )                                          //5

    .arg( ( cAP2.job3run ? tr( "YES" ) : tr( "NO" ) ) )       //6
    .arg( ( cAP2.job3auto ? tr( "YES" ) : tr( "NO" ) ) )      //7

    .arg( ( cAP2.job4run ? tr( "YES" ) : tr( "NO" ) ) )       //8
    .arg( ( QString::number( cAP2.rfiters ) ) )               //9
    
    .arg( ( cAP2.job5run ? tr( "YES" ) : tr( "NO" ) ) )       //10
    .arg( ( QString::number( cAP2.mciters ) ) )               //11	  
    ;

  if ( genMask_edited.ShowAnalysis2DSAParts[ "Job Flow Summary" ].toInt()  )
    html_analysis_profile_2dsa +=  html_analysis_profile_2dsa_flow;
  

  //Per-Channel params:
  QString html_analysis_profile_2dsa_per_channel;
  html_analysis_profile_2dsa_per_channel += tr(
				   "<table>"		   
				     "<tr>"
				       "<td><b>Per-Channel Profile:</b></td>"
				     "</tr>"
				   "</table>"
				   "<br>"
				   )
    ;
  int nchna_2dsa   = cAP2.parms.size();
  for ( int i = 0; i < nchna_2dsa; i++ )
    {
      html_analysis_profile_2dsa_per_channel += tr(
				       "<table>"		   
				         "<tr>"
				            "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
				          "</tr>"
				       "</table>"
				       )
	.arg( cAP2.parms[ i ].channel )            //1
	;


      QString s_data = QString::number( cAP2.parms[ i ].s_min ) + ", " +
	               QString::number( cAP2.parms[ i ].s_max ) + ", " +
                       QString::number( cAP2.parms[ i ].s_grpts );

      QString ff0_data = QString::number( cAP2.parms[ i ].k_min ) + ", " +
	               QString::number( cAP2.parms[ i ].k_max ) + ", " +
                       QString::number( cAP2.parms[ i ].k_grpts );

      QString grid_rep =  QString::number( cAP2.parms[ i ].gridreps );

      QString custom_grid = cAP2.parms[ i ].cgrid_name;

      QString vary_vbar = cAP2.parms[ i ].varyvbar ? tr( "YES" ) : tr( "no" );

      QString const_ff0 = QString::number( cAP2.parms[ i ].ff0_const );

      html_analysis_profile_2dsa_per_channel += tr(
                                       "<table style=\"margin-left:30px\">"
                                          "<tr><td> s Min, Max, Grid Points:     </td>  <td> %1 </td> </tr>"
                                          "<tr><td> f/f0 Min, Max, Grid Points:  </td>  <td> %2 </td> </tr>"
                                          "<tr><td> Grid Repetitions:            </td>  <td> %3 </td> </tr>"
                                          "<tr><td> Custom Grid:                 </td>  <td> %4 </td> </tr>"
                                          "<tr><td> Varying Vbar:                </td>  <td> %5 </td> </tr>"
                                          "<tr><td> Constant f/f0:               </td>  <td> %6 </td> </tr>"
                                       "</table>"
                                       )
	.arg( s_data )                     //1                                                                                 
        .arg( ff0_data )                   //2                                                                                 
	.arg( grid_rep )                   //3                                                                                 
        .arg( custom_grid )                //4                                                                                 
	.arg( vary_vbar )                  //5                                                                                 
        .arg( const_ff0 )                  //6
	;
    }

  if ( genMask_edited.ShowAnalysis2DSAParts[ "Per-Channel Profiles" ].toInt()  )
    html_analysis_profile_2dsa +=  html_analysis_profile_2dsa_per_channel;
  
  html_analysis_profile_2dsa += tr( "<hr>" ) ;
  //ANALYSIS: 2DSA per-channel settings: end
  
  
  //ANALYSIS: PCSA per-channel settings: begin 
  html_analysis_profile_pcsa = tr(
				  "<h3 align=left> Analysis Profile: PCSA Controls </h3>"
				  "&nbsp;&nbsp;<br>"
				  )
    ;

  QString html_analysis_profile_pcsa_flow;
  html_analysis_profile_pcsa_flow += tr(
				   "<table style=\"margin-left:10px\">"
				     //PCSA
				     "<tr>"
				         "<td> \"PCSA\":    </td>"
				         "<td> Run?         </td>"    
                                         "<td> %1           </td>"
				      "</tr>"
				   "</table>"
				   )
    .arg( cAPp.job_run ? tr( "YES" ) : tr( "no" ) )
    ;

  if ( genMask_edited.ShowAnalysisPCSAParts[ "Job Flow Summary" ].toInt()  )
    html_analysis_profile_pcsa +=  html_analysis_profile_pcsa_flow;
  //End of PCSA Flow
  

  QString html_analysis_profile_pcsa_per_channel;
  if ( cAPp.job_run )
    {
      int nchna_pcsa   = cAPp.parms.size();
      for ( int i = 0; i < nchna_pcsa; i++ )
	{
	  html_analysis_profile_pcsa_per_channel += tr(
					   "<table>"		   
					     "<tr>"
					        "<td><b>Channel:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
					     "</tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].channel )            //1
	    ;

	  QString x_data =  cAPp.parms[ i ].x_type + ", " +
	                    QString::number( cAPp.parms[ i ].x_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].x_max );
	  QString y_data =  cAPp.parms[ i ].y_type + ", " +
	                    QString::number( cAPp.parms[ i ].y_min ) + ", " +
	                    QString::number( cAPp.parms[ i ].y_max );	  
	  QString z_data =  cAPp.parms[ i ].z_type + ", " +
	                    QString::number( cAPp.parms[ i ].z_value );
	                 	  
	  html_analysis_profile_pcsa_per_channel += tr(
					   "<table style=\"margin-left:30px\">"
					     "<tr><td> Curve Type:                </td>  <td> %1 </td> </tr>"
					     "<tr><td> X Axis Type, Min, Max:     </td>  <td> %2 </td> </tr>"
					     "<tr><td> Y Axis Type, Min, Max:     </td>  <td> %3 </td> </tr>"
					     "<tr><td> Z Axis Type, Value:        </td>  <td> %4 </td> </tr>"
					     "<tr><td> Variations Count:          </td>  <td> %5 </td> </tr>"
					     "<tr><td> Grid Fit Iterations:       </td>  <td> %6 </td> </tr>"
					     "<tr><td> Curve Resolution Points:   </td>  <td> %7 </td> </tr>"
					     "<tr><td> Noise Type:                </td>  <td> %8 </td> </tr>"
					     "<tr><td> Tikhonov Regularization:   </td>  <td> %9 </td> </tr>"
					     "<tr><td> Tikhonov Alpha:            </td>  <td> %10 </td> </tr>"
					     "<tr><td> MonteCarlo Iterations:     </td>  <td> %11 </td> </tr>"
					   "</table>"
					   )
	    .arg( cAPp.parms[ i ].curv_type )                     //1
	    .arg( x_data )                                        //2
	    .arg( y_data )                                        //3
	    .arg( z_data )                                        //4
	    .arg( QString::number( cAPp.parms[ i ].varcount ) )   //5
	    .arg( QString::number( cAPp.parms[ i ].grf_iters ) )  //6
	    .arg( QString::number( cAPp.parms[ i ].creso_pts ) )  //7
	    .arg( cAPp.parms[ i ].noise_type )                    //8
	    .arg( cAPp.parms[ i ].treg_type )                     //9
	    .arg( QString::number( cAPp.parms[ i ].tr_alpha ) )   //10
	    .arg( QString::number( cAPp.parms[ i ].mc_iters ) )   //11
	;
	}
    }

  if ( genMask_edited.ShowAnalysisPCSAParts[ "Per-Channel Profiles" ].toInt()  )
    html_analysis_profile_pcsa +=  html_analysis_profile_pcsa_per_channel;
  
  html_analysis_profile_pcsa += tr( "<hr>" ) ;
  //ANALYSIS: PCSA per-channel settings: end
  //APROFILE: end
  
  
  QString html_paragraph_close = tr(
    "</p>"
				    )
    ;

  QString html_footer = tr( 
    "<div align=right>"
       "<br>End of report: <i>\"%1\"</i>"
    "</div>"
			    )
    .arg( currProto. protoname )
    ;

  
  //Main assembly: reportMask based
  QString html_assembled = QString("");
  html_assembled +=
    html_header
    + html_title
    + html_paragraph_open;

  assemble_parts( html_assembled );
  
  html_assembled += html_paragraph_close
    + html_footer;
  
  
  // html_assembled +=
  //   html_header
  //   + html_title
  //   + html_paragraph_open
  //   + html_general
  //   + html_lab_rotor
  //   + html_operator
  //   + html_speed
  //   + html_cells
  //   + html_solutions
  //   + html_optical
  //   + html_ranges
  //   + html_scan_count
  //   + html_analysis_profile
  //   + html_analysis_profile_2dsa
  //   + html_analysis_profile_pcsa 
  //   + html_paragraph_close
  //   + html_footer;
    
  QTextDocument document;
  document.setHtml( html_assembled );
  
  QPrinter printer(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);

  QString fileName  = currProto. protoname + "-run" + runID + ".pdf";
  filePath  = US_Settings::tmpDir() + "/" + fileName;
  printer.setOutputFileName( filePath );
  printer.setPageMargins(QMarginsF(15, 15, 15, 15));
  
  document.print(&printer);
}



//save trees' selections into internal structures
void US_ReporterGMP::gui_to_parms( void )
{
  //tree-to-json: genTree && json-to-genMask structure
  QString editedMask_gen = tree_to_json ( topItem );
  parse_edited_gen_mask_json( editedMask_gen, genMask_edited );
  
  //tree-to-json: perChanTree
  QString editedMask_perChan = tree_to_json ( chanItem );
  parse_edited_perChan_mask_json( editedMask_perChan, perChanMask_edited );
}

//Pasre reportMask JSON
void US_ReporterGMP::parse_edited_gen_mask_json( const QString maskJson, GenReportMaskStructure & MaskStr )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
  QJsonObject json = jsonDoc.object();

  int has_sol_items = 0;
  int has_analysis_items = 0;

  MaskStr.ShowReportParts      .clear();
  MaskStr.ShowSolutionParts    .clear();
  MaskStr.ShowAnalysisGenParts .clear();
  MaskStr.ShowAnalysis2DSAParts.clear();
  MaskStr.ShowAnalysisPCSAParts.clear();
  MaskStr.has_anagen_items  = 0;
  MaskStr.has_ana2dsa_items = 0;
  MaskStr.has_anapcsa_items = 0;
  
  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();

      if ( value.isString() )
	{
	  if ( value.toString().toInt() )
	      MaskStr.ShowReportParts[ key ] = true;
	  else
	    MaskStr.ShowReportParts[ key ] = false;
	}
      else if ( value.isArray() )
	MaskStr.ShowReportParts[ key ] = true;  //for now

      //treat Solutions && Analysis: nested JSON
      if ( key.contains("Solutions") || key.contains("Analysis") )
	{
	   QJsonArray json_array = value.toArray();
	   for (int i=0; i < json_array.size(); ++i )
	     {
	       foreach(const QString& array_key, json_array[i].toObject().keys())
		 {
		   if (  key.contains("Solutions") )
		     {
		       MaskStr.ShowSolutionParts[ array_key ] = json_array[i].toObject().value(array_key).toString();
		       if ( MaskStr.ShowSolutionParts[ array_key ].toInt() )
			 ++has_sol_items;
		     }
		   if (  key.contains("Analysis") )
		     {
		       QJsonObject newObj = json_array[i].toObject().value(array_key).toObject();
		      
		       foreach ( const QString& n_key, newObj.keys() )
			 {
			   QString analysis_cathegory_item_value  = newObj.value( n_key ).toString();
			   
			   if ( analysis_cathegory_item_value.toInt() )
			     ++has_analysis_items;
			   
			   if ( array_key.contains("General") )
			     {
			       MaskStr.ShowAnalysisGenParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysisGenParts[ n_key ].toInt() )
				 ++MaskStr.has_anagen_items;
			     }
			   
			   if ( array_key.contains("2DSA") )
			     {
			       MaskStr.ShowAnalysis2DSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysis2DSAParts[ n_key ].toInt() )
				 ++MaskStr.has_ana2dsa_items;
			     }
			   
			   if ( array_key.contains("PCSA") ) 
			     {
			       MaskStr.ShowAnalysisPCSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysisPCSAParts[ n_key ].toInt() )
				 ++MaskStr.has_anapcsa_items;
			     }
			 }
		     }
		 }
	     }
	   
	   //Set if to show "Solutions" based on children items
	   if ( key.contains("Solutions") &&  !has_sol_items )
	     MaskStr.ShowReportParts[ key ] = false;

	   if ( key.contains("Analysis") &&  !has_analysis_items )
	     MaskStr.ShowReportParts[ key ] = false;
	}
    }
}

//Pasre reportMask JSON: perChan
void US_ReporterGMP::parse_edited_perChan_mask_json( const QString maskJson, PerChanReportMaskStructure & MaskStr )
{
  
}

//transform 3-level tree to JSON
QString US_ReporterGMP::tree_to_json( QMap < QString, QTreeWidgetItem * > topLevItems )
{
  QString mask_edited;
  mask_edited += "{";
  
  QMap < QString, QTreeWidgetItem * >::iterator top;
  for ( top = topLevItems.begin(); top != topLevItems.end(); ++top )
    {
      qDebug() << "Top item " << top.key() << " is " <<  int(top.value()->checkState(0)) << "\n";

      mask_edited += "\"" + top.key().trimmed() + "\":";
      
      int children_lev1 = top.value()->childCount();
      if ( !children_lev1 )
	{
	  mask_edited += "\"" + QString::number( int(top.value()->checkState(0)) ) + "\",";
	}
      else
	{
	  mask_edited += "[";
	  for( int i = 0; i < top.value()->childCount(); ++i )
	    {
	      qDebug() << "\tThe child's " << top.value()->child(i)->text(1) << ", state is: " << top.value()->child(i)->checkState(0);

	      mask_edited += "{\"" + top.value()->child(i)->text(1).trimmed() + "\":";

	      int children_lev2 = top.value()->child(i)->childCount();
	      if ( !children_lev2 )
		{
		  mask_edited += "\"" + QString::number( int(top.value()->child(i)->checkState(0)) ) + "\"}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	      else
		{
		  mask_edited += "{";
		  for( int ii = 0; ii < top.value()->child(i)->childCount(); ++ii )
		    {
		      qDebug() << "\t\tThe child's " << top.value()->child(i)->child(ii)->text(1) << ", state is: " << top.value()->child(i)->child(ii)->checkState(0);

		      mask_edited += "\"" + top.value()->child(i)->child(ii)->text(1).trimmed() + "\":";
		      int children_lev3 = top.value()->child(i)->child(ii)->childCount();
		      if ( !children_lev3 )
			{
			  mask_edited += "\"" + QString::number( int(top.value()->child(i)->child(ii)->checkState(0)) ) + "\"";
			  if ( ii != top.value()->child(i)->childCount()-1 )
			    mask_edited += ",";
			}
		      else
			{
			  //Here 3th level of nestedness may be considered if needed... and so on...
			}
		      
		    }
		  mask_edited += "}}";
		  if ( i != top.value()->childCount()-1 )
		    mask_edited += ",";
		}
	    }
	  mask_edited += "],";
	}
    }
  mask_edited.chop(1);
  mask_edited += "}";

  qDebug() << "Edited Mask: " << mask_edited;

  return mask_edited;
}

//Format times
void US_ReporterGMP::format_needed_params()
{
  //Duration
  QList< int > hms_dur;
  double duration = currProto. rpSpeed. ssteps[0].duration;
  US_RunProtocol::timeToList( duration, hms_dur );
  duration_str = QString::number( hms_dur[ 0 ] ) + "d " + QString::number( hms_dur[ 1 ] ) + "h " + QString::number( hms_dur[ 2 ] ) + "m ";
  
  //Delay Stage
  QList< int > hms_delay_stage;
  double delay_stage = currProto. rpSpeed. ssteps[0].delay_stage;
  US_RunProtocol::timeToList( delay_stage, hms_delay_stage );
  delay_stage_str = //QString::number( hms_delay_stage[ 0 ] ) + "d " +
    QString::number( hms_delay_stage[ 1 ] ) + "h " + QString::number( hms_delay_stage[ 2 ] ) + "m ";

  //Total Time
  QList< int > hms_tot;
  double total_time = currProto. rpSpeed. ssteps[0].total_time;
  US_RunProtocol::timeToList( total_time, hms_tot );
  total_time_str = QString::number( hms_tot[ 0 ] ) + "d " + QString::number( hms_tot[ 1 ] ) + "h " + QString::number( hms_tot[ 2 ] ) + "m ";

  //UV-vis delay
  QList< int > hms_delay_uvvis;
  double delay_uvvis = currProto. rpSpeed. ssteps[0].delay;
  qDebug() << "Delay UV-vis: " << delay_uvvis;
  US_RunProtocol::timeToList( delay_uvvis, hms_delay_uvvis );
  delay_uvvis_str = QString::number( hms_delay_uvvis[ 0 ] ) + "d " + QString::number( hms_delay_uvvis[ 1 ] ) + "h " + QString::number( hms_delay_uvvis[ 2 ] ) + "m ";
  qDebug() << "Delay UV-vis str: " << delay_uvvis_str;

  //UV-vis scanint
  QList< int > hms_scanint_uvvis;
  double scanint_uvvis = currProto. rpSpeed. ssteps[0].scanintv;
  qDebug() << "ScanInt UV-vis: " << scanint_uvvis;
  US_RunProtocol::timeToList( scanint_uvvis, hms_scanint_uvvis );
  scanint_uvvis_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_uvvis[ 1 ] ) + "h " + QString::number( hms_scanint_uvvis[ 2 ] ) + "m " + QString::number( hms_scanint_uvvis[ 3 ] ) + "s ";
  qDebug() << "ScanInt UV-vis str: " << scanint_uvvis_str;

  //Interference delay
  QList< int > hms_delay_int;
  double delay_int = currProto. rpSpeed. ssteps[0].delay_int;
  qDebug() << "Delay Interference: " << delay_int;
  US_RunProtocol::timeToList( delay_int, hms_delay_int );
  delay_int_str = QString::number( hms_delay_int[ 0 ] ) + "d " + QString::number( hms_delay_int[ 1 ] ) + "h " + QString::number( hms_delay_int[ 2 ] ) + "m ";
  qDebug() << "Delay Interference str: " << delay_int_str;
  
  //Interference scanint
  QList< int > hms_scanint_int;
  double scanint_int = currProto. rpSpeed. ssteps[0].scanintv_int;
  qDebug() << "ScanInt Interference: " << scanint_int;
  US_RunProtocol::timeToList( scanint_int, hms_scanint_int );
  scanint_int_str = //QString::number( hms_scanint_uvvis[ 0 ] ) + "d " +
    QString::number( hms_scanint_int[ 1 ] ) + "h " + QString::number( hms_scanint_int[ 2 ] ) + "m " + QString::number( hms_scanint_int[ 3 ] ) + "s ";
  qDebug() << "ScanInt Interference str: " << scanint_int_str;
}

//get current date
void US_ReporterGMP::get_current_date()
{
  QDate dNow(QDate::currentDate());
  QString fmt = "MM/dd/yyyy";
  
  current_date = dNow.toString( fmt );
  qDebug() << "Current date -- " << current_date;
}



//assemble parts of the PDF based on mask
void US_ReporterGMP::assemble_parts( QString & html_str )
{
  QMap < QString, bool >::iterator top;
  for ( top = genMask_edited.ShowReportParts.begin(); top != genMask_edited.ShowReportParts.end(); ++top )
    {
      qDebug() << "QMap key, val -- " << top.key() << top.value();
      
      if ( top.key().contains("General") && top.value() )
    	html_str += html_general;
      if ( top.key().contains("Rotor") && top.value() )
    	html_str += html_lab_rotor;
      if ( top.key().contains("Operator") && top.value() )
    	html_str += html_operator;
      if ( top.key().contains("Speed") && top.value() )
    	html_str += html_speed;
      if ( top.key().contains("Cells") && top.value() )
    	html_str += html_cells;

      if ( top.key().contains("Solutions") && top.value() )
	html_str += html_solutions;
          
      if ( top.key().contains("Optical Systems") && top.value() )
    	html_str += html_optical;      
      if ( top.key().contains("Ranges") && top.value() )
    	html_str += html_ranges;
      if ( top.key().contains("Scan Counts") && top.value() )
    	html_str += html_scan_count;

      //Analysis
      if ( top.key().contains("Analysis Profile") && top.value() )
    	{
	  if ( genMask_edited.has_anagen_items ) 
	    html_str += html_analysis_profile;
	  if ( genMask_edited.has_ana2dsa_items ) 
	    html_str += html_analysis_profile_2dsa;
	  if ( genMask_edited.has_anapcsa_items ) 
	    html_str += html_analysis_profile_pcsa ;
    	}
    }
}


//Fetch Solution details && add to html_solutions
void US_ReporterGMP::add_solution_details( const QString sol_id, const QString sol_comment, QString& html_solutions )
{
  //get Solution info by ID:
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) +  db.lastError() );
      
      return;
    }

  US_Solution*   solution = new US_Solution;
  int solutionID = sol_id.toInt();

  int status = US_DB2::OK;
  status = solution->readFromDB  ( solutionID, &db );

  // Error reporting
  if ( status == US_DB2::NO_BUFFER )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "The buffer this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status == US_DB2::NO_ANALYTE )
    {
      QMessageBox::information( this,
				tr( "Attention" ),
				tr( "One of the analytes this solution refers to was not found.\n"
				    "Please restore and try again.\n" ) );
    }
  
  else if ( status != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Database Problem" ),
			    tr( "Database returned the following error: \n" ) +  db.lastError() );
    }
  //End of reading Solution:

  //add general solution details to html_solutions string:
  html_solutions += tr(
		       "<table style=\"margin-left:5px\">"
		       "<caption align=left> <b><i>Solution Information </i></b> </caption>"
		       "</table>"
		       
		       "<table style=\"margin-left:20px\">"
		         "<tr><td> Solution Name: </td>               <td> %1 </td> </tr>"
		         "<tr><td> Solution Comment: </td>            <td> %2 </td> </tr>"
		         "<tr><td> Common VBar (20&#8451;):</td>      <td> %3 </td> </tr>"
		         "<tr><td> Storage Temperature (&#8451;):</td><td> %4 </td> </tr>" 
		       "</table>"
		       )
    .arg( solution->solutionDesc )                          //1
    .arg( sol_comment )                                     //2
    .arg( QString::number( solution->commonVbar20 ))        //3
    .arg( QString::number( solution->storageTemp ))         //4
    ;
  

  //Get analytes information
  if ( genMask_edited.ShowSolutionParts[ "Analyte Information" ].toInt()  )
    {
      html_solutions += tr(
			   "<table style=\"margin-left:20px\">"
			   "<caption align=left> <b><i>Analytes Information</i></b> </caption>"
			   "</table>"
			   )
	;
    }
  
  QString analyte_gen_info;
  QString analyte_detailed_info;
  int num_analytes = solution->analyteInfo.size();
  for (int i=0; i < num_analytes; ++i )
    {
      //clear analyte's strings:
      analyte_gen_info = QString("");
      analyte_detailed_info = QString("");

     
      US_Analyte analyte = solution->analyteInfo[ i ].analyte;
      QString a_name     = analyte.description;
      QString a_amount   = QString::number( solution->analyteInfo[ i ].amount );

      analyte_gen_info += tr(
			     "<tr>"
			       "<td> <i>Analyte #%1: </i></td> &nbsp;&nbsp;&nbsp;&nbsp; "
			       "<td> Name: </td>  <td> %2</td> &nbsp;&nbsp;&nbsp;&nbsp;"
			       "<td> Molar Ratio:</td> <td>%3</td>"
			     "</tr>"
			     )
	.arg( QString::number( i + 1 ) )  //1
	.arg( a_name )                    //2
	.arg( a_amount )                  //3
	;

      QString a_mw     = QString::number( analyte.mw ) + " D";
      QString a_vbar20 = QString::number( analyte.vbar20 );
      
      int seqlen       = analyte.sequence.length();
      QString seqsmry  = analyte.sequence;
      int total       = 0;
      
      if ( seqlen == 0 )
	seqsmry         = tr( "(empty)" );
      else
	{
	  seqsmry         = seqsmry.toLower()
	    .remove( QRegExp( "[\\s0-9]" ) );
	  seqlen          = seqsmry.length();
	  if ( seqlen > 25 )
	    {
	      seqsmry        = QString( seqsmry ).left( 10 ) + " ... "
		+ QString( seqsmry ).mid( seqlen - 10 );
	    }
	  //seqsmry          += "\n  ";
	  seqsmry          += "<br>  ";
	  
	  for ( int ii = 0; ii < 26; ii++ )
	    {
	      QString letter  = QString( QChar( 'a' + ii ) );
	      int lcount      = analyte.sequence.count( letter );
	      total          += lcount;
	      if ( lcount > 0 )
		{
		  seqsmry     += QString().sprintf( "%d", lcount )
		    + " " + letter.toUpper() + ", ";
		  //seqsmry          += "\n  ";
		}
	    }
	  seqsmry     += QString().sprintf( "%d", total ) + " tot";
	}

      QString a_type   = tr( "Carbohydrate/Other" );
      a_type           = analyte.type == ( US_Analyte::PROTEIN ) ? tr( "Protein" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::DNA ) ? tr( "DNA" ) : a_type;
      a_type           = analyte.type == ( US_Analyte::RNA ) ? tr( "RNA" ) : a_type;

      //absorbing residues
      US_Math2::Peptide p;
      US_Math2::calc_vbar( p, analyte.sequence, 20.0 );
      analyte.mw         = ( analyte.mw == 0.0 ) ? p.mw : analyte.mw;
      
      // Absorbing residues
      int cys = int(p.c);
      int hao = int(p.j);
      int orn = int(p.o);
      int trp = int(p.w);
      int tyr = int(p.y);
      int all_abs = 0;
      all_abs = cys + hao + orn + trp + tyr;
      qDebug() << "Tot AAs: " << all_abs;
      QString absorbing_residues = tr( "(empty)" );
      if (all_abs > 0)
	{
	  absorbing_residues = "";
	  if ( cys > 0)
	    absorbing_residues += QString().sprintf( "%d", cys ) + " " + "Cys"  + ", ";
	  if ( hao > 0)
	 absorbing_residues += QString().sprintf( "%d", hao ) + " " + "Hao"  + ", ";
	  if ( orn > 0)
	    absorbing_residues += QString().sprintf( "%d", orn ) + " " + "Orn"  + ", ";
	  if ( trp > 0)
	    absorbing_residues += QString().sprintf( "%d", trp ) + " " + "Trp"  + ", ";
	  if ( tyr > 0)
	    absorbing_residues += QString().sprintf( "%d", tyr ) + " " + "Tyr"  + ", ";

	  absorbing_residues += QString().sprintf( "%d", all_abs ) + " tot";
	}

      //remeber to exclude absorbing residues for non-protein species
      
      analyte_detailed_info += tr(
				  "<tr> <td> Type: </td> <td>%1</td>              </tr>"
				  "<tr> <td> Molecular Weight: </td>  <td> %2</td></tr>"
				  "<tr> <td> Vbar (20 &#8451;): </td>  <td> %3</td></tr>"
				  "<tr> <td> Sequence Length:  </td>  <td> %4</td></tr>"
				  "<tr> <td> Sequence Summary: </td>  <td> %5</td></tr>"
				  )
	.arg( a_type )                        //1
	.arg( a_mw )                          //2 
	.arg( a_vbar20 )                      //3
	.arg( QString::number( seqlen  ) )    //4
	.arg( seqsmry  )                      //5
	;

      //add absorbig residues if PROTEINS
      if ( analyte.type == 0 )
	{
	  analyte_detailed_info += tr(
				      "<tr> <td> AAs absorbing at 280 nm: </td> <td>%1</td> </tr>"
				      "<tr> <td> E280: </td>  <td> %2</td>                  </tr>"
				      "<tr> <td> Extinction count: </td>  <td> %3</td>      </tr>"
				      )
	    .arg( absorbing_residues )
	    .arg( QString::number( analyte.extinction[ 280.0 ] ) )
	    .arg( QString::number( analyte.extinction.keys().count() ) )
	    ;
	}

      //add info on the current analyte to html_solutions string
      if ( genMask_edited.ShowSolutionParts[ "Analyte Information" ].toInt()  )
	{
	  html_solutions += tr(
			       "<table style=\"margin-left:40px\">"
			         "%1"
			       "</table>"
			       "<table style=\"margin-left:60px\">"
			         "%2"
			       "</table>"  
			       )
	    .arg( analyte_gen_info )
	    .arg( analyte_detailed_info )
	    ;
	}
      
    }


  //general buffer information
  QString buffer_gen_info      = QString("");
  QString buffer_detailed_info = QString("");
  US_Buffer buffer = solution->buffer;

  buffer_gen_info += tr(
			"<tr>"
			  "<td> Buffer Name: </td> <td> %1 </td> "
			"</tr>"
			     )
    .arg( buffer.description )                //1
    ;

  buffer_detailed_info += tr(
			     "<tr><td> Density (20&#8451;, g/cm<sup>3</sup>):  </td>   <td>%1</td>  </tr>"
			     "<tr><td> Viscosity (20&#8451;, cP): </td>   <td>%2</td>               </tr>"
			     "<tr><td> pH:       </td>   <td>%3 </td>                              </tr>"
			     "<tr><td> Compressibility:</td>   <td>%4</td>                         </tr>"			     
			     )
    .arg( QString::number( buffer.density ) )                    //1
    .arg( QString::number( buffer.viscosity ) )                  //2
    .arg( QString::number( buffer.pH ) )                         //3
    .arg( QString::number( buffer.compressibility ) )            //4
    ;

  //buffer components (if any)
  QString buffer_components_info = QString("");
  for ( int i=0; i < buffer.component.size(); ++i )
    {
      QString component_desc =
	buffer.component[i].name
	+ " (" + QString::number( buffer.concentration[ i ] ) + " "
	+ buffer.component[i].unit
	+ ")";


      buffer_components_info += tr(
				   "<tr><td> Component Name:</td> &nbsp;&nbsp;  <td>%1</td>  </tr>"
				   )
	.arg( component_desc )
	;
    }
  

      


  //append html string: buffer general info
  if ( genMask_edited.ShowSolutionParts[ "Buffer Information" ].toInt()  )
    {
      html_solutions += tr(
			   "<table style=\"margin-left:30px\">"
			   "<caption align=left> <b><i>Buffer Information</i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:70px\">"
		           "%1"
			   "</table>"
			   "<table style=\"margin-left:100px\">"
		           "%2"
			   "</table>"
			   "<table style=\"margin-left:130px\">"
		           "%3"
			   "</table>"  
			   "<br>"
			   )
	.arg( buffer_gen_info )
	.arg( buffer_detailed_info )
	.arg( buffer_components_info )
	;
    }
}
