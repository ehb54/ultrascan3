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
  QLabel*      bn_actions     = us_banner( tr( "Actions:" ) );
  QLabel*      lb_loaded_run  = us_label( tr( "Loaded Run:" ) );
  le_loaded_run               = us_lineedit( tr(""), 0, true );

  QPushButton* pb_loadrun       = us_pushbutton( tr( "Load GMP Run" ) );
  pb_gen_report    = us_pushbutton( tr( "Generate Report" ) );
  pb_view_report   = us_pushbutton( tr( "View Report" ) );
  pb_reset_trees   = us_pushbutton( tr( "Reset Trees" ) );
  pb_help          = us_pushbutton( tr( "Help" ) );
  pb_close         = us_pushbutton( tr( "Close" ) );
		
  int row           = 0;
  buttonsLayout->addWidget( bn_actions,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_loaded_run,  row,   0, 1, 2 );
  buttonsLayout->addWidget( le_loaded_run,  row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_loadrun,     row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_gen_report,  row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_view_report, row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_reset_trees, row++, 0, 1, 12 );

  buttonsLayout->addWidget( pb_help,        row,   0, 1, 6, Qt::AlignBottom );
  buttonsLayout->addWidget( pb_close,       row++, 6, 1, 6, Qt::AlignBottom );

  pb_gen_report  ->setEnabled( false );
  pb_view_report ->setEnabled( false );
  pb_reset_trees ->setEnabled( false );
    
  connect( pb_help,    SIGNAL( clicked()      ),
	   this,       SLOT(   help()         ) );
  connect( pb_close,   SIGNAL( clicked()      ),
	   this,       SLOT(   close()        ) );

  connect( pb_loadrun, SIGNAL( clicked()      ),
	   this,       SLOT(   load_gmp_run()         ) );

    
  //rightLayout: genTree
  QLabel*      lb_gentree  = us_label(      tr( "General Report Profile Settings:" ) );
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
  QLabel*      lb_chantree  = us_label(      tr( "Per-Triple Report Profile Settings:" ) );
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
  
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );
  
  protocol_details[ "autoflowID" ] = QString::number(autoflowID);

  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];
  

  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

  qDebug() << "Exp. Label: "    << protocol_details[ "label" ];
  qDebug() << "GMP Run ? "      << protocol_details[ "gmpRun" ];

  qDebug() << "AnalysisIDs: "   << protocol_details[ "analysisIDs" ];
  qDebug() << "aprofileguid: "  << AProfileGUID ;
  

  //Now, read protocol's 'reportMask' && reportItems masks && populate trees
  read_protocol_and_reportMasks( );
  build_genTree();

  le_loaded_run ->setText( protocol_details[ "filename" ] );
  pb_gen_report  ->setEnabled( true );
  //pb_view_report ->setEnabled( true );
  pb_reset_trees ->setEnabled( true );
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

  QString xmlstr( "" );
  US_ProtocolUtil::read_record_auto( ProtocolName_auto, invID,  &xmlstr, NULL, &db );
  QXmlStreamReader xmli( xmlstr );
  currProto. fromXml( xmli );
    
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
  //Channel wavelengths
  ch_wvls                = currAProf.ch_wvls;

  //report Mask
  reportMask             = currAProf.report_mask;
  parse_gen_mask_json();

  qDebug() << "ReportMask: " << reportMask;
}

//parse JSON for general rpeort mask
void US_ReporterGMP::parse_gen_mask_json ( void )
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


//view report
void US_ReporterGMP::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;

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
  
  currProto = US_RunProtocol();  //ALEXEY: we need to reset US_Protocol
  currAProf = US_AnaProfile();   //ALEXEY: we need to reset US_AnaProfile
  
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


//Format times
void US_ReporterGMP::format_needed_params()
{

}

//get current date
void US_ReporterGMP::get_current_date()
{

}

//Start assembling PDF file
void US_ReporterGMP::assemble_pdf()
{
  
}


//assemble parts of the PDF based on mask
void US_ReporterGMP::assemble_parts( QString & html_str )
{

}

//Pasre reportMask JSON
void US_ReporterGMP::parse_mask_json( void )
{
  
}


//Fetch Solution details && add to html_solutions
void US_ReporterGMP::add_solution_details( const QString sol_id, const QString sol_comment, QString& html_solutions )
{
  
}
