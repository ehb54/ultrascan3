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
  QLineEdit*   le_loaded_run  = us_lineedit( tr(""), 0, true );

  QPushButton* pb_loadrun       = us_pushbutton( tr( "Load GMP Run" ) );
  QPushButton* pb_gen_report    = us_pushbutton( tr( "Generate Report" ) );
  QPushButton* pb_view_report   = us_pushbutton( tr( "View Report" ) );
  QPushButton* pb_reset_trees   = us_pushbutton( tr( "Reset Trees" ) );
  pb_help       = us_pushbutton( tr( "Help" ) );
  pb_close      = us_pushbutton( tr( "Close" ) );
		
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
  
  resize( 1060, 650 );
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

      //clean trees
      genTree     ->clear();
      perChanTree ->clear();
    }
  else
    return;
  
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );
  
  protocol_details[ "autoflowID" ] = QString::number(autoflowID);

  QString stage        = protocol_details[ "status" ];
  QString currDir      = protocol_details[ "dataPath" ];
  QString invID_passed = protocol_details[ "invID_passed" ];
  QString ProtName     = protocol_details[ "protocolName" ];
  QString expName      = protocol_details[ "experimentName" ];
  QString correctRadii = protocol_details[ "correctRadii" ];
  QString expAborted   = protocol_details[ "expAborted" ];
  QString runID        = protocol_details[ "runID" ];
  QString exp_label    = protocol_details[ "label" ];

  QString gmp_Run      = protocol_details[ "gmpRun" ];
  QString filename     = protocol_details[ "filename" ];
  QString aprofileguid = protocol_details[ "aprofileguid" ];
  QString analysisIDs  = protocol_details[ "analysisIDs" ];

  qDebug() << "CURR DIRECTORY : "   << currDir;
  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

  qDebug() << "Exp. Label: "    << protocol_details[ "label" ];
  qDebug() << "GMP Run ? "      << protocol_details[ "gmpRun" ];

  qDebug() << "AnalysisIDs: "   << protocol_details[ "analysisIDs" ];
  qDebug() << "aprofileguid: "  << aprofileguid;
  
  //Now, read protocol's 'reportMask' && reportItems masks && populate trees
  
  QString reportMask = read_reporMask( aprofileguid );
  qDebug() << "Report Mask General: " << reportMask;

  
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
      
      QDateTime local(QDateTime::currentDateTime());

      autoflowentry << id << runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

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
QString US_ReporterGMP::read_reporMask( QString aprofGUID )
{
  US_Passwd  pw;
  US_DB2* dbP    = new US_DB2( pw.getPasswd() );
  
  ap_xml.clear();
  int status;
  QString reportMask_DB;
  
  QStringList qry;
  qry << "get_aprofile_info" << aprofGUID;
  dbP->query( qry );
  
  qDebug() << "read_reportMask: query, status -- " << qry;
  
  if ( dbP->lastErrno() == US_DB2::OK ) 
    {
      while ( dbP->next() )
	{
	  //currProf.aprofID     = dbP->value( 0 ).toInt();
	  //currProf.aprofname   = dbP->value( 1 ).toString();
	  ap_xml               = dbP->value( 2 ).toString();
	  reportMask_DB        = dbP->value( 4 ).toString();
	}
    }

  return reportMask_DB;
}

//view report
void US_ReporterGMP::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;

}

//reset
void US_ReporterGMP::reset_report_panel ( void )
{
  
}

//init correct # of us_labels rows based on passed # stages from AProfile
void US_ReporterGMP::initPanel( QMap < QString, QString > & protocol_details )
{
  
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
