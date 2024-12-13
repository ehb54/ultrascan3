#include <QPrinter>
#include <QPdfWriter>
#include <QPainter>

#include "us_reporter_gmp.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_protocol_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include "us_solution_vals.h"
#include "us_lamm_astfvm.h"
#include "../us_fematch/us_thread_worker.h"
#include "us_tmst_plot.h"
#include "us_tar.h"
#include "us_defines.h"
#include "../us/us_revision.h"

#define MIN_NTC   25

// Test case(s): Constructor
US_ReporterGMP::US_ReporterGMP( QMap< QString, QString> t_c ) : US_Widgets()
{
  QString report_filepath  = "/home/alexey/ultrascan/reports/eGFP-DNA-MW-08OCT23-run1981_GMP_DB.tar";
  QString html_filePath    = "/home/alexey/ultrascan/reports/eGFP-DNA-MW-08OCT23-run1981/html_string.html";
  int autolfowGMPReportID  = 25;
  QString autoStatusID     = QString::number(231);
  QString autoID           = QString::number(1002);

  write_gmp_report_DB_test(report_filepath, html_filePath, autolfowGMPReportID,
			   autoStatusID, autoID);
}

//write GMP report to DB Test
void US_ReporterGMP::write_gmp_report_DB_test( QString report_filepath, QString html_filePath,
					       int autolfowGMPReportID, QString autoStatusID, QString autoID )
{
  
  bool clear_GMP_report_record = false;
  
  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }


  //Write HTML strign to file & later save to DB withing general archive
  qDebug() << "[TEST] Writing HTML Blob of filePath -- " << html_filePath;
  
  int writeStatus_html = db.writeBlobToDB( html_filePath,
					   QString( "upload_gmpReportData_html" ),
					   autolfowGMPReportID );
  
  if ( writeStatus_html == US_DB2::DBERROR )
    {
      QMessageBox::warning(this, "Error", "Error processing html file:\n"
			   + html_filePath + "\n" + db.lastError() +
			   "\n" + "Could not open file or no data \n");
      clear_GMP_report_record = true;
    }
  
  else if ( writeStatus_html != US_DB2::OK )
    {
      QMessageBox::warning(this, "Error", "returned processing html file:\n" +
			   html_filePath + "\n" + db.lastError() + "\n");
      
      clear_GMP_report_record = true;
    }
  
  /**********************************************************************************/
  //// .Tar Blob
  qDebug() << "[TEST] Writing .TAR Blob of filePath -- " << report_filepath;
  
  int writeStatus= db.writeBlobToDB(report_filepath,
				    QString( "upload_gmpReportData" ),
				    autolfowGMPReportID );
  
  if ( writeStatus == US_DB2::DBERROR )
    {
      QMessageBox::warning(this, "Error", "Error processing file:\n"
			   + report_filepath + "\n" + db.lastError() +
			   "\n" + "Could not open file or no data \n");
      clear_GMP_report_record = true;
    }
  
  else if ( writeStatus != US_DB2::OK )
    {
      QMessageBox::warning(this, "Error", "returned processing file:\n" +
			   report_filepath + "\n" + db.lastError() + "\n");
      
      clear_GMP_report_record = true;
    }
  
  /*************************************************************************/

  
  /*************************************************************************/
      
  QStringList qry;    
  if ( clear_GMP_report_record )
    {
      qDebug() << "Something went wrong!!";
      // qry.clear();
      // qry << "clear_autoflowGMPReportRecord" << QString::number( autolfowGMPReportID );
      // db.query( qry );
      
      //Maybe revert 'reporting' stage in the autoflowStages??
    }
  else
    {
      //Report generated && .PDF GMP report saved to autoflowGMPReport:
      //No, we can write information on who/when generated report: ///////////////////////
      
      //get user info
      qry.clear();
      qry <<  QString( "get_user_info" );
      db.query( qry );
      db.next();
      
      int ID        = db.value( 0 ).toInt();
      QString fname = db.value( 1 ).toString();
      QString lname = db.value( 2 ).toString();
      QString email = db.value( 4 ).toString();
      int     level = db.value( 5 ).toInt();
      
      QString reporting_Json;
      
      reporting_Json. clear();
      reporting_Json += "{ \"Person\": ";
      
      reporting_Json += "[{";
      reporting_Json += "\"ID\":\""     + QString::number( ID )     + "\",";
      reporting_Json += "\"fname\":\""  + fname                     + "\",";
      reporting_Json += "\"lname\":\""  + lname                     + "\",";
      reporting_Json += "\"email\":\""  + email                     + "\",";
      reporting_Json += "\"level\":\""  + QString::number( level )  + "\"";
      reporting_Json += "}]}";
      
      qry.clear();
      qry << "update_autoflowStatusReport_record"
	  << autoStatusID
	  << autoID
	  << reporting_Json;
      
      db.query( qry );
    }
}
 
// END Test Case /////////////////////////////////////////////////////////////////////////////////


// Constructor
US_ReporterGMP::US_ReporterGMP() : US_Widgets()
{
  setWindowTitle( tr( "GMP Report Generator & Viewer"));
  setPalette( US_GuiSettings::frameColor() );

  first_time_gen_tree_build = true;
  first_time_misc_tree_build = true;
  first_time_perChan_tree_build = true;
  auto_mode  = false;
  GMP_report = true;
  
  // primary layouts
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );
  QVBoxLayout* leftLayout     = new QVBoxLayout();
  QVBoxLayout* rghtLayout     = new QVBoxLayout();
  QGridLayout* buttonsLayout  = new QGridLayout();
  QGridLayout* genTreeLayout  = new QGridLayout();
  QGridLayout* miscTreeLayout  = new QGridLayout();
  QGridLayout* perChanTreeLayout  = new QGridLayout();
  QGridLayout* combPlotsLayout  = new QGridLayout();
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
  miscTreeLayout->setSpacing        ( 1 );
  miscTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  perChanTreeLayout->setSpacing        ( 1 );
  perChanTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  combPlotsLayout->setSpacing        ( 1 );
  combPlotsLayout->setContentsMargins( 0, 0, 0, 0 );

  //leftLayout
  //GMP or Custom Report Generation Items
  QLabel*      bn_actions     = us_banner( tr( "Generate GMP or Custom Report from Completed GMP Run:" ), 1 );
  QLabel*      lb_loaded_run  = us_label( tr( "Loaded Run:" ) );
  le_loaded_run               = us_lineedit( tr(""), 0, true );
  QPushButton* pb_loadrun       = us_pushbutton( tr( "Load GMP Run" ) );
  pb_gen_report    = us_pushbutton( tr( "Generate Report" ) );
  pb_view_report   = us_pushbutton( tr( "View Generated Report" ) );
  pb_select_all    = us_pushbutton( tr( "Select All Tree Items" ) );
  pb_unselect_all  = us_pushbutton( tr( "Unselect All Tree Items" ) );
  pb_expand_all    = us_pushbutton( tr( "Expand All Tree Items" ) );
  pb_collapse_all  = us_pushbutton( tr( "Collapse All Tree Items" ) );

  //Filename path
  QLabel*      lb_fpath_info = us_label( tr( "Report File \nLocation:" ) );
  te_fpath_info =  us_textedit();
  QFontMetrics m (te_fpath_info -> font());
  int RowHeight  = m.lineSpacing();
  RowHeight *= 3;
  te_fpath_info -> setFixedHeight  ( RowHeight);
  te_fpath_info -> setText( tr( "" ) );
  us_setReadOnly( te_fpath_info, true );

  //GMP Report From DB Items
  QLabel*      bn_actions_db     = us_banner( tr( "Download and View GMP Report from DB:" ), 1 );
  QLabel*      lb_loaded_run_db  = us_label( tr( "Loaded Run:" ) );
  le_loaded_run_db               = us_lineedit( tr(""), 0, true );
  QPushButton* pb_loadreport_db  = us_pushbutton( tr( "Load GMP Report from DB (.PDF)" ) );
  pb_view_report_db              = us_pushbutton( tr( "View Downloaded Report" ) );

  //Filename DB path
  QLabel*      lb_fpath_info_db  = us_label( tr( "Report File \nLocation:" ) );
  te_fpath_info_db =  us_textedit();
  te_fpath_info_db -> setFixedHeight  ( RowHeight );
  te_fpath_info_db -> setText( tr( "" ) );
  us_setReadOnly( te_fpath_info_db, true );
  
  //Misc
  QLabel*      bn_actions_misc   = us_banner( tr( "" ), 1 );
  pb_help          = us_pushbutton( tr( "Help" ) );
  pb_close         = us_pushbutton( tr( "Close" ) );
		
  int row           = 0;
  buttonsLayout->addWidget( bn_actions,       row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_loaded_run,    row,   0, 1, 2 );
  buttonsLayout->addWidget( le_loaded_run,    row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_loadrun,       row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_gen_report,    row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_fpath_info,    row,   0, 1, 2 );
  buttonsLayout->addWidget( te_fpath_info,    row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_view_report,   row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_select_all,    row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_unselect_all,  row++, 6, 1, 6 );
  buttonsLayout->addWidget( pb_expand_all,    row  , 0, 1, 6 );
  buttonsLayout->addWidget( pb_collapse_all,  row++, 6, 1, 6 );

  buttonsLayout->addWidget( bn_actions_db,    row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_loaded_run_db, row,   0, 1, 2 );
  buttonsLayout->addWidget( le_loaded_run_db, row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_loadreport_db, row++, 0, 1, 12 );
  buttonsLayout->addWidget( lb_fpath_info_db, row,   0, 1, 2 );
  buttonsLayout->addWidget( te_fpath_info_db, row++, 2, 1, 10 );
  buttonsLayout->addWidget( pb_view_report_db,row++, 0, 1, 12 );

  buttonsLayout->addWidget( bn_actions_misc,  row++, 0, 1, 12 );
  buttonsLayout->addWidget( pb_help,          row,   0, 1, 6, Qt::AlignBottom );
  buttonsLayout->addWidget( pb_close,         row++, 6, 1, 6, Qt::AlignBottom );

  pb_gen_report     ->setEnabled( false );
  pb_view_report    ->setEnabled( false );
  pb_select_all     ->setEnabled( false );
  pb_unselect_all   ->setEnabled( false );
  pb_expand_all     ->setEnabled( false );
  pb_collapse_all   ->setEnabled( false );
  pb_view_report_db ->setEnabled( false );
  
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

  connect( pb_loadreport_db,  SIGNAL( clicked()      ),
  	   this,              SLOT(   load_gmp_report_db()   ) );
  connect( pb_view_report_db, SIGNAL( clicked()      ),
	   this,              SLOT(   view_report_db()   ) );
  
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

  //rightLayout: miscTree
  QLabel*      lb_misctree  = us_banner(      tr( "Miscellaneous Report Profile Settings:" ), 1 );
  miscTree = new QTreeWidget();
  QStringList misc_theads;
  misc_theads << "Selected" << "Protocol Settings";
  miscTree->setHeaderLabels( misc_theads );
  miscTree->setFont( QFont( US_Widgets::fixedFont().family(),
			      US_GuiSettings::fontSize() + 1 ) );
  miscTree->installEventFilter   ( this );
  miscTreeLayout->addWidget( lb_misctree );
  miscTreeLayout->addWidget( miscTree );
  
  miscTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //rightLayout: perChannel tree
  QLabel*      lb_chantree  = us_banner(      tr( "Per-Triple Report Profile Settings:" ), 1 );
  perChanTree = new QTreeWidget();
  QStringList chan_theads;
  chan_theads << "Selected" << "Protocol Settings";
  perChanTree->setHeaderLabels( chan_theads );
  perChanTree->setFont( QFont( US_Widgets::fixedFont().family(),
			       US_GuiSettings::fontSize() + 1 ) );
  perChanTreeLayout->addWidget( lb_chantree );
  perChanTreeLayout->addWidget( perChanTree );
  perChanTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");

  //rightLayout: combPlotLayout
  QLabel*      lb_combplots  = us_banner(      tr( "Combined Plot Distributions Settings:" ), 1 );
  combPlotsTree = new QTreeWidget();
  QStringList combplots_theads;
  combplots_theads << "Selected" << "Protocol Settings";
  combPlotsTree->setHeaderLabels( combplots_theads );
  combPlotsTree->setFont( QFont( US_Widgets::fixedFont().family(),
				 US_GuiSettings::fontSize() + 1 ) );
  combPlotsLayout->addWidget( lb_combplots );
  combPlotsLayout->addWidget( combPlotsTree );
  combPlotsTree  ->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");
      
  // put layouts together for overall layout
  leftLayout->addLayout( buttonsLayout );
  leftLayout->addStretch();
  rghtLayout->addLayout( genTreeLayout );
  rghtLayout->addLayout( perChanTreeLayout );
  rghtLayout->addLayout( combPlotsLayout );
  rghtLayout->addLayout( miscTreeLayout );
  
  mainLayout->addLayout( leftLayout );
  mainLayout->addLayout( rghtLayout );
  mainLayout->setStretchFactor( leftLayout, 6 );
  mainLayout->setStretchFactor( rghtLayout, 8 );
  
  resize( 1350, 800 );

  qDebug() << "Init gen Tree: height -- "     << genTree->height();
  qDebug() << "Init perChan Tree: height -- " << perChanTree->height();
  qDebug() << "Init Combo Tree: height -- "   << combPlotsTree->height();
}

//For autoflow: constructor
US_ReporterGMP::US_ReporterGMP( QString a_mode ) : US_Widgets()
{
  setWindowTitle( tr( "GMP Report Generator"));
  setPalette( US_GuiSettings::frameColor() );

  first_time_gen_tree_build = true;
  first_time_misc_tree_build = true;
  first_time_perChan_tree_build = true;
  auto_mode  = true;
  GMP_report = true;
  
  // primary layouts
  QVBoxLayout* superLayout    = new QVBoxLayout( this );
  superLayout->setSpacing        ( 2 );
  superLayout->setContentsMargins( 2, 2, 2, 2 );
  //banner with (to be) run name
  lb_hdr1          = us_banner( tr( "" ), 1 );
  superLayout->addWidget(lb_hdr1);
  //gridLayout for View Rpeort in autoflow
  QGridLayout* genL   = new QGridLayout();
  genL->setSpacing        ( 2 );
  genL->setContentsMargins( 2, 2, 2, 2 );

  int row1 = 1;
  int ihgt        = lb_hdr1->height();
  QSpacerItem* spacer2 = new QSpacerItem( 20, 100*ihgt, QSizePolicy::Expanding);
  genL->setRowStretch( row1, 1 );
  genL->addItem( spacer2,  row1++,  0, 1, 1 );

  //mainLayout to host left/right
  QHBoxLayout* mainLayout     = new QHBoxLayout( this );

  QWidget* leftWidget         = new QWidget();
  QVBoxLayout* leftLayout     = new QVBoxLayout(leftWidget);
  QWidget* rightWidget        = new QWidget();
  QVBoxLayout* rghtLayout     = new QVBoxLayout(rightWidget);

  QGridLayout* buttonsLayout  = new QGridLayout();
  QGridLayout* genTreeLayout  = new QGridLayout();
  QGridLayout* miscTreeLayout  = new QGridLayout();
  QGridLayout* perChanTreeLayout  = new QGridLayout();
  QGridLayout* combPlotsLayout  = new QGridLayout();
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
  miscTreeLayout->setSpacing        ( 1 );
  miscTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  perChanTreeLayout->setSpacing        ( 1 );
  perChanTreeLayout->setContentsMargins( 0, 0, 0, 0 );
  combPlotsLayout->setSpacing        ( 1 );
  combPlotsLayout->setContentsMargins( 0, 0, 0, 0 );

  
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

  //rightLayout: miscTree
  QLabel*      lb_misctree  = us_banner(      tr( "Miscellaneous Report Profile Settings:" ), 1 );
  miscTree = new QTreeWidget();
  QStringList misc_theads;
  misc_theads << "Selected" << "Protocol Settings";
  miscTree->setHeaderLabels( misc_theads );
  miscTree->setFont( QFont( US_Widgets::fixedFont().family(),
			      US_GuiSettings::fontSize() + 1 ) );
  miscTree->installEventFilter   ( this );
  miscTreeLayout->addWidget( lb_misctree );
  miscTreeLayout->addWidget( miscTree );
  
  miscTree->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");


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

  //rightLayout: combPlotLayout
  QLabel*      lb_combplots  = us_banner(      tr( "Combined Plot Distributions Settings:" ), 1 );
  combPlotsTree = new QTreeWidget();
  QStringList combplots_theads;
  combplots_theads << "Selected" << "Protocol Settings";
  combPlotsTree->setHeaderLabels( combplots_theads );
  combPlotsTree->setFont( QFont( US_Widgets::fixedFont().family(),
				 US_GuiSettings::fontSize() + 1 ) );
  combPlotsLayout->addWidget( lb_combplots );
  combPlotsLayout->addWidget( combPlotsTree );
  combPlotsTree  ->setStyleSheet( "QTreeWidget { font: bold; font-size: " + QString::number(sfont.pointSize() ) + "pt;}  QTreeView { alternate-background-color: yellow;} QTreeView::item:hover { border: black;  border-radius:1px;  background-color: rgba(0,128,255,95);}");
      
    
  // put layouts together for overall layout
  leftLayout->addLayout( buttonsLayout );
  leftLayout->addStretch();
  rghtLayout->addLayout( genTreeLayout );
  rghtLayout->addLayout( perChanTreeLayout );
  rghtLayout->addLayout( combPlotsLayout );
  rghtLayout->addLayout( miscTreeLayout );
  
  // mainLayout->addLayout( leftLayout );
  // mainLayout->addLayout( rghtLayout );
  // mainLayout->setStretchFactor( leftLayout, 6 );
  // mainLayout->setStretchFactor( rghtLayout, 8 );

  mainLayout->addWidget( leftWidget, 6 );
  mainLayout->addWidget( rightWidget, 8 );

  //superLayout
  superLayout -> addLayout( mainLayout );
  pb_view_report_auto   = us_pushbutton( tr( "View Report" ) );
  genL->addWidget( pb_view_report_auto );
  pb_view_report_auto ->setVisible( false );
  connect( pb_view_report_auto,  SIGNAL( clicked()      ),
	   this,                 SLOT(   view_report()   ) );
  superLayout -> addLayout( genL );
  
  // Hide layouts
  if ( a_mode.toStdString() == "AUTO")
    {
      leftWidget  -> hide();
      rightWidget -> hide();
      
    }
  
  resize( 1350, 800 );
}


//Autoflow: loadRun_auto
void US_ReporterGMP::loadRun_auto ( QMap < QString, QString > & protocol_details )
{
  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];
  runName            = protocol_details[ "experimentName" ];
  FileName           = protocol_details[ "filename" ];
  intensityID        = protocol_details[ "intensityID" ];
  AutoflowID_auto    = protocol_details[ "autoflowID" ];
  analysisIDs        = protocol_details[ "analysisIDs" ];
  autoflowStatusID   = protocol_details[ "statusID" ];
  optimaName         = protocol_details[ "OptimaName" ] ;
  dataSource         = protocol_details[ "dataSource" ] ;

  simulatedData      = false;

  QString full_runname = protocol_details[ "filename" ];
  FullRunName_auto = runName + "-run" + runID;
  if ( full_runname.contains(",") && full_runname.contains("IP") && full_runname.contains("RI") )
    {
      QString full_runname_edited  = full_runname.split(",")[0];
      full_runname_edited.chop(3);
      full_runname = full_runname_edited + " (combined RI+IP) ";

      FullRunName_auto += " (combined RI+IP) ";   //Captures protDev names...
    }
  
  //lb_hdr1 ->setText( QString( tr("Report for run: %1") ).arg(FileName) );
  lb_hdr1 ->setText( QString( tr("Report for run: %1") ).arg( FullRunName_auto ) );
  lb_hdr1->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
  
  //show progress dialog
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 12, this);
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
  qApp->processEvents();

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
  progress_msg->setValue( 7 );
  qApp->processEvents();

  //check triples for failure
  check_failed_triples( );
  
  //check models existence
  check_models( AutoflowID_auto.toInt() );
  progress_msg->setValue( 8 );
  qApp->processEvents();

  //identify what's intended to be simulated
  check_for_missing_models( );
  ////

  //Check for dropped triples
  check_for_dropped_triples();
  ////
  
  //build Trees
  build_genTree();  
  progress_msg->setValue( 9 );
  qApp->processEvents();

  build_perChanTree();
  progress_msg->setValue( 10);
  qApp->processEvents();

  build_combPlotsTree();
  progress_msg->setValue( 11 );
  qApp->processEvents();

  build_miscTree();  
  progress_msg->setValue( 12 );
  qApp->processEvents();

  progress_msg->setValue( progress_msg->maximum() );
  qApp->processEvents();
  progress_msg->close();

  //compose a message on missing models
  QString msg_missing_models = missing_models_msg();
  
  //Inform user that current configuraiton corresponds to GMP report
  if ( !GMP_report )
    {
      QMessageBox::information( this, tr( "Report Profile Uploaded" ),
				tr( "<font color='red'><b>ATTENTION:</b> There are missing models for certain triples: </font><br><br>"
				    "%1<br><br>"
				    "As a result, a non-GMP report will be generated!")
				.arg( msg_missing_models) );
    }
  
  //generate report (download models, simulate, create PDFs)
  generate_report();
  
}


//read eSign GMP record for assigned oper(s) && rev(s) && status
QMap< QString, QString> US_ReporterGMP::read_autoflowGMPReportEsign_record( US_DB2* db )
{
  QMap< QString, QString> eSign_record;
  
  QStringList qry;
  qry << "get_gmp_review_info_by_autoflowID" << AutoflowID_auto;
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
	}
    }
  else
    {
      //No record, so no oper/revs assigned!
      qDebug() << "No e-Sign GMP record exists in main table!!";
      qDebug() << "Checking History...";
      qry. clear();

      qry << "get_gmp_review_info_by_autoflowID_history" << AutoflowID_auto;
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
	    }
	}
      else
	{
	  eSign_record[ "isEsignRecord" ]        = QString("NO");
	  eSign_record. clear();
	}
    }

  return eSign_record;
}


//form a string of opers/revs out of jsonDoc
QString US_ReporterGMP::get_assigned_oper_revs( QJsonDocument jsonDoc )
{
  QString smry;
  QStringList assigned_list;
  
  if ( !jsonDoc. isArray() )
    {
      qDebug() << "jsonDoc not a JSON, and/or not an JSON Array!";
      return smry;
    }
  
  QJsonArray jsonDoc_array  = jsonDoc.array();
  for (int i = 0; i < jsonDoc_array.size(); ++i )
    assigned_list << jsonDoc_array[i].toString();
  
  for ( int ii = 0; ii < assigned_list.count(); ii++ )
    {
      smry += assigned_list[ ii ];
      if ( ii != assigned_list.count() -1 )
	smry += "; ";
    }
  
  return smry;
}


//check for dropped triples (at 3. IMPORT)
void US_ReporterGMP::check_for_dropped_triples( void )
{
  droppedTriplesList. clear();
  
  QStringList dropped_triples_RI, dropped_triples_IP;
  read_reportLists_from_aprofile( dropped_triples_RI, dropped_triples_IP );

  for ( int i=0; i<dropped_triples_RI.size(); ++i )
    droppedTriplesList << dropped_triples_RI[ i ];
  for ( int i=0; i<dropped_triples_IP.size(); ++i )
    {
      QStringList tname = dropped_triples_IP[ i ].split(".");
      QString tname_mod = tname[0] + "." + "Interference";
      droppedTriplesList << tname_mod;
    }
  
  qDebug() << "::check_for_dropped_triples(): List of ALL dropped triples : " << droppedTriplesList;

}

//check for failed triples
void US_ReporterGMP::check_failed_triples( void )
{

  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );

  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }

  QStringList tripleNames;
  
  QStringList analysisIDs_list = analysisIDs.split(",");
  for( int i=0; i < analysisIDs_list.size(); ++i )
    {
      QMap <QString, QString> analysis_details;
      QString requestID = analysisIDs_list[i];
      
      qDebug() << "GMP Report: RequestID: " << requestID;
      
      analysis_details = read_autoflowAnalysis_record( &db, requestID );

      if ( !analysis_details.size() )
	{
	  analysis_details = read_autoflowAnalysisHistory_record( &db, requestID );
	  qDebug() << "GMP Report: analysis_details.size() FROM autoflowAnalysisHistory -- " << analysis_details.size();
	}
      else
	qDebug() << "GMP Report: analysis_details.size() FROM autoflowAnalysis -- " << analysis_details.size();

      QString triple_name    = analysis_details[ "triple_name" ]  ;
      QString cluster        = analysis_details[ "cluster" ]      ;
      QString filename       = analysis_details[ "filename" ]     ;
      QString curr_gfacID    = analysis_details[ "CurrentGfacID" ];
      QString curr_HPCAnalysisRequestID   = analysis_details[ "currentHPCARID"];
      QString status_json    = analysis_details[ "status_json" ]  ;
      QString status         = analysis_details[ "status" ]       ;
      QString status_msg     = analysis_details[ "status_msg" ]   ;
      QString create_time    = analysis_details[ "create_time" ]  ;   
      QString update_time    = analysis_details[ "update_time" ]  ;
      QString nextWaitStatus = analysis_details[ "nextWaitStatus" ] ;
      QString nextWaitStatusMsg = analysis_details[ "nextWaitStatusMsg" ] ;

      qDebug() << "Triple_name, Status -- " << triple_name << status;
      tripleNames << triple_name;

      QJsonDocument jsonDoc = QJsonDocument::fromJson( status_json.toUtf8() );
      if (!jsonDoc.isObject())
	{
	  qDebug() << "GMP Report: All Doc: NOT a JSON Doc !!";
	  
	  QMessageBox::warning( this, tr( "JSON Format Problem" ),
				tr( "JSON message for status of the analysis performed on triple %1 appears to be corrupted. "
				    "This may be an indicaiton of the problem with the analysis run for this triple.\n\n"
				    "This problem will preclude status monitoring for other triples. "
				    "Please check log messages, or contact administrator for help.\n\n"
				    "The program will return to the autoflow runs dialog.").arg( triple_name ) );
	  return;
	}
      
      const QJsonValue &to_process = jsonDoc.object().value("to_process");
      const QJsonValue &processed  = jsonDoc.object().value("processed");
      const QJsonValue &submitted  = jsonDoc.object().value("submitted");          

      QJsonArray to_process_array  = to_process.toArray();
      QJsonArray processed_array   = processed.toArray();

      if ( status == "FAILED" || status == "CANCELED" )
	{
	  if ( processed.isUndefined())
	    qDebug() << "Nothing has been processed yet !!";
	  else
	    {
	      qDebug() << "analysing last() of processed:" ;
	      //get last() array's element
	      QString stage_failed = processed_array.last().toObject().keys().last();

	      qDebug() << "GMP Report: stage_failed for triple -- " << triple_name << stage_failed;

	      Triple_to_FailedStage[ triple_name ] = stage_failed;
	    }
	}
    }

  //Determine if 'S' data
  for (int i=0; i< tripleNames.size(); ++i )
    {
      if ( tripleNames[i].contains(".S.") )
	{
	  simulatedData = true;
	  break;
	}
    }

  qDebug() << "[in check_failed().. ] : simulatedData? " << simulatedData;
}

// Read AutoflowAnalysisRecord
QMap< QString, QString>  US_ReporterGMP::read_autoflowAnalysis_record( US_DB2* db, const QString& requestID )
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
QMap< QString, QString>  US_ReporterGMP::read_autoflowAnalysisHistory_record( US_DB2* db, const QString& requestID )
{
  QMap <QString, QString> analysis_details;
  
  QStringList qry;
  qry << "read_autoflowAnalysisHistory_record"
      << requestID;

  qDebug() << "In read_autoflowAnalysisHistory_record(), qry -- " << qry;
  
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



//check models existence for a run/protocol loaded
void US_ReporterGMP::check_for_missing_models ( void )
{
  bool hasIT   = cAP2. job4run; //2dsa-it       
  bool hasMC   = cAP2. job5run; //2dsa-mc
  bool hasPCSA = cAPp .job_run; //pcsa

   
  QMap < QString, QStringList >::iterator mm;
  for ( mm = Triple_to_Models.begin(); mm != Triple_to_Models.end(); ++mm )
    {
      QStringList missing_models;
      qDebug() << "For triple -- " << mm.key() << ", there are models: " << mm.value();

      if ( hasIT && !mm.value().contains( "2DSA-IT" ))
	missing_models << "2DSA-IT";
      if ( hasMC && !mm.value().contains( "2DSA-MC" ))
	missing_models << "2DSA-MC";
      if ( hasPCSA && !mm.value().contains( "PCSA" ))
	missing_models << "PCSA";
	
      Triple_to_ModelsMissing[ mm.key() ] = missing_models;
    }

  //debug
  for ( mm = Triple_to_ModelsMissing.begin(); mm != Triple_to_ModelsMissing.end(); ++mm )
    qDebug() << "For triple -- " << mm.key() << ", there are missing models: " << mm.value();
  
}

//Compose a string of missing models
QString US_ReporterGMP::missing_models_msg( void )
{
  QString models_str;
  int num_dropped_triples = 0;
  int num_total_missed_triples = 0;

  QMap < QString, QStringList >::iterator mmm;
  for ( mmm = Triple_to_ModelsMissing.begin(); mmm != Triple_to_ModelsMissing.end(); ++mmm )
    {
      if ( !mmm.value().isEmpty() )
	{
	  ++num_total_missed_triples;
	  //check if missing models because of triple dropped
	  bool isDropped = false;
	  QString c_triple = mmm.key();
	  c_triple.replace(".","");
	  for (int i=0; i<droppedTriplesList.size(); ++i)
	    {
	      QString d_triple = droppedTriplesList[i];
	      d_triple.replace(".","");
	      if ( c_triple == d_triple )
		{
		  ++num_dropped_triples;
		  isDropped = true;
		  break;
		}
	    }
	  
	  //compose : Do we want to report on dropped triples at all ?
	  models_str += mmm.key() + ", missing models: " + mmm.value().join(", ");
	  if( isDropped )
	    models_str += "<font color='red'> [triple dropped]</font>";
	    
	  models_str += "<br>";
	}
    }

  qDebug() << "Number of total missing triples; Number of dropped triples -- "
	   << num_total_missed_triples << "; " << num_dropped_triples;

  // Do we want to report on dropped triples at all ?
  // Do ONLY dropped triples trigger report to be non-GMP ?
  if ( !models_str.isEmpty() & num_total_missed_triples != num_dropped_triples ) 
    GMP_report = false;
  
  return models_str;
}

//Failed Stages | Missing Models
QString US_ReporterGMP::compose_html_failed_stage_missing_models( void )
{
  QString failed_str;
  bool areFailed = false;
  
  failed_str += "<table>";
  
  // QMap < QString, QString >::iterator mm;
  // for ( mm = Triple_to_FailedStage.begin(); mm != Triple_to_FailedStage.end(); ++mm )
  //   {
  //     if ( !mm.value().isEmpty() )
  // 	{
  // 	  areFailed = true;
	  
  // 	  failed_str += "<tr><td style=\"color:red;\">" + mm.key() + ",</td><td> analysis failed/canceled at stage: </td><td style=\"color:red;\">"
  // 	                + mm.value() + ";</td>" + 
  // 	                + "<td>Models missing: </td><td style=\"color:red;\">" +  Triple_to_ModelsMissing[ mm.key() ].join(", ") + "</td></tr>";
  // 	}
  //   }
  
   QMap < QString, QStringList >::iterator mmm;
   for ( mmm = Triple_to_ModelsMissing.begin(); mmm != Triple_to_ModelsMissing.end(); ++mmm )
     {
       if ( !mmm.value().isEmpty() )
	 {
   	  areFailed = true;

	  bool isDropped = false;
	  QString c_triple = mmm.key();
	  c_triple.replace(".","");
	  for (int i=0; i<droppedTriplesList.size(); ++i)
	    {
	      QString d_triple = droppedTriplesList[i];
	      d_triple.replace(".","");
	      if ( c_triple == d_triple )
		{
		  isDropped = true;
		  break;
		}
	    }
  	  
   	  failed_str += "<tr><td style=\"color:red;\">" + mmm.key() + ":</td>" + 
	    + "<td>Models missing: </td><td style=\"color:red;\">" +  mmm.value().join(", ") + "</td>";
	  if ( isDropped )
	    failed_str += "<td>[Triple dropped]</td>";
	  
	  failed_str +="</tr>";
   	}
     }
  
  
  failed_str += "</table>";

  if( !areFailed )
    failed_str.clear();
    
  return failed_str;
}

//check models existence for a run loaded
void US_ReporterGMP::check_models ( int autoflowID )
{
  //build Array of triples
  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < nchna; i++ )
    {
      QString channel_desc_alt = chndescs_alt[ i ];
      QString channel_desc     = chndescs[ i ];

      //check if channel meant to be analyzed/reported
      int analysis_to_be_run = analysis_runs[ i ];
      int report_to_be_run   = report_runs[ i ];

      if ( analysis_to_be_run == 0 || report_to_be_run == 0 )
	continue;

      QList < double > chann_wvls    = ch_wvls[ channel_desc_alt ];
      int chann_wvl_number           = chann_wvls.size();
      
      for ( int jj = 0; jj < chann_wvl_number; ++jj )
	{
	  QString wvl            = QString::number( chann_wvls[ jj ] );
	  
	  QString tripleName = channel_desc_alt.section( ":", 0, 0 )[0] + "." + channel_desc_alt.section( ":", 0, 0 )[1];

	  if ( channel_desc_alt.contains( "Interf" ) ) 
	    tripleName += ".Interference";
	  else
	    tripleName += "." + wvl;

	  //'S' data
	  if ( dataSource. contains("DiskAUC:Absorbance") && simulatedData )
	    tripleName = tripleName.replace( ".A.", ".S." );
	  
	  qDebug() << "[in check_models()]: TripleName -- " << tripleName; 
	  Array_of_tripleNames.push_back( tripleName );
	}
    }
      
  US_Passwd   pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
    
  if ( db->lastErrno() != US_DB2::OK )
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
				tr( "DB Connection Problem" ),
				tr( "There was an error connecting to the database:\n" )
				+ db->lastError() );
      
      return;
    }
  
  //iterate over triples && get eID for each triple's data:
  for ( int i=0; i < Array_of_tripleNames.size(); ++ i )
    {
      QString triple_name_actual = Array_of_tripleNames[ i ];
      
      if ( triple_name_actual.contains("Interference") )
	triple_name_actual.replace( "Interference", "660" );
      
      //get requestID in autoflowAnalysis based on tripleName & autoflowID
      QStringList query;
      query << "get_modelAnalysisInfo" << Array_of_tripleNames[ i ] << QString::number( autoflowID );
      qDebug() << "check_models qry -- " << query;
      db->query( query );

      QString modelDescJson;
      while ( db->next() )
	{
	  modelDescJson           = db->value( 0 ).toString();
	  //Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ] = modelDescJson;
	  
	  qDebug() << "Triple, modelDesc -- " << Array_of_tripleNames[ i ] << modelDescJson;
	}
      ///

      //Now parse modelDecsJson for eID, modelIDs
      //Triple_to_ModeslDesc[ "1.A.225" ] [ "2DSA-IT" ] = modelID; 
      //Triple_to_ModeslDesc[ "1.A.225" ] [ "2DSA-MC" ] = modelID;
      //Triple_to_ModeslDesc[ "1.A.225" ] [ "PCSA" ]    = modelID;
      //Triple_to_ModeslDesc[ "1.A.225" ] [ "eID" ]     = eID;
      Triple_to_ModelsDesc    [ Array_of_tripleNames[ i ] ] = parse_models_desc_json( modelDescJson, "modelID"   );
      Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ] = parse_models_desc_json( modelDescJson, "modelGUID" );

            
      QString eID = Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "eID" ];
      qDebug() << "In check_models: eID -- " << eID;
      
      //now check models based on eID:
      query.clear();
      query << "get_modelDescsIDs" << eID;
      db->query( query );

      QStringList model_list;
      
      while ( db->next() )
	{
	  QString  description         = db->value( 0 ).toString();
	  QString  modelID             = db->value( 1 ).toString();
	  //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
	  QDateTime date               = db->value( 2 ).toDateTime();
	  
	  QDateTime now = QDateTime::currentDateTime();
	  
	  if ( description.contains( "2DSA-IT" ) && modelID == Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "2DSA-IT" ] )
	    {
	      qDebug() << "2DSA-IT Ids: modelID, read from modelLink: " << modelID << Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "2DSA-IT" ];
	      model_list << "2DSA-IT";
	    }
	  
	  if ( description.contains( "2DSA-MC" ) && modelID == Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "2DSA-MC" ] )
	    {
	      qDebug() << "2DSA-MC Ids: modelID, read from modelLink: " << modelID << Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "2DSA-MC" ];
	      model_list << "2DSA-MC";
	    }
	  
	  if ( description.contains( "PCSA" ) && modelID == Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "PCSA" ] )
	    {
	      qDebug() << "PCSA Ids: modelID, read from modelLink: " << modelID << Triple_to_ModelsDesc[ Array_of_tripleNames[ i ] ][ "PCSA" ];
	      model_list << "PCSA";
	    }
	}

      //populate QMap connecting triple name to it's existing models
      Triple_to_Models[ Array_of_tripleNames[ i ] ] = model_list;
    }

  //DEBUG

  //test
  delete db;
  
}

//Create QMap for shorter desctiption of triple's models (2DSA-IT, ...) to modelIDs, OR to modelGUIDs
QMap< QString, QString > US_ReporterGMP::parse_models_desc_json( QString modelDescJson, QString model_property )
{
  QMap <QString, QString>  modelDesc_shortened;

  if ( !modelDescJson.isEmpty() )
    {
      QJsonDocument jsonDoc = QJsonDocument::fromJson( modelDescJson.toUtf8() );
      QJsonObject json_obj = jsonDoc.object();
      
      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);
	  
	  qDebug() << "ModelsDesc key, value: " << key << value;

	  if ( key == "2DSA_IT" || key == "2DSA_MC" || key == "PCSA" ) 
	    {
	      QString key_mod = key;
	      key_mod. replace("_","-");
	      
	      QJsonArray json_array = value.toArray();
	      for (int i=0; i < json_array.size(); ++i )
		{
		  foreach(const QString& array_key, json_array[i].toObject().keys())
		    {
		      //by modelID
		      if ( array_key == "modelID" && array_key == model_property )
			{
			  if ( !modelDesc_shortened.contains( key_mod ) )  //Temporary, for PCSA (2 entries)
			    {
			      modelDesc_shortened[ key_mod ] = json_array[i].toObject().value(array_key).toString();
			      qDebug() << "modelDescJson Map: -- model, property, value: "
				       << key_mod
				       << array_key
				       << json_array[i].toObject().value(array_key).toString();
			    }
			}
		      
		      //by modelGUID
		      if ( array_key == "modelGUID" && array_key == model_property )
			{
			  if ( !modelDesc_shortened.contains( key_mod ) )  //Temporary, for PCSA (2 entries)
			    {
			      modelDesc_shortened[ key_mod ] = json_array[i].toObject().value(array_key).toString();
			      qDebug() << "modelDescJson Map: -- model, property, value: "
				       << key_mod
				       << array_key
				       << json_array[i].toObject().value(array_key).toString();
			    }
			}
		      
		      if ( array_key == "editeddataID" )
			{
			  modelDesc_shortened[ "eID" ] = json_array[i].toObject().value(array_key).toString();
			  qDebug() << "modelDescJson Map: -- meID, value: "
				   << array_key
				   << json_array[i].toObject().value(array_key).toString();
			  
			}
		    }
		}
	    }
	}
    }
  
  return modelDesc_shortened;
}


//load GMP Rpeort from DB (.PDF)
void US_ReporterGMP::load_gmp_report_db ( void )
{
  // //TESTING *************
  // QFile fin( "/home/alexey/ultrascan/reports/SBird-DNA-EcoRI-101322-PD9-run1843_GMP_DB_bySysTar.tar.gz" );
  // if ( ! fin.open( QIODevice::ReadOnly ) )
  //  {
  //    qDebug() << "cannot open file ";;
  //    return;
  //  }
  // QByteArray blobData = fin.readAll();
  // fin.close();

  // qDebug() << "BlobData size -- " << blobData.size();
  // return;
  // //END TESTING *********
  
  US_Passwd pw;
  US_DB2 db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db.lastError() );
      return;
    }
  
  list_all_gmp_reports_db( gmpReportsDBdata, &db );

  QString pdtitle( tr( "Select GMP Report" ) );
  QStringList hdrs;
  int         prx;
  
  hdrs << "ID"
       << "Run Name"
    //<< "Protocol Name"
       << "Created"
       << "Filename (.pdf)";
         
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

      pb_view_report_db -> setEnabled( false );
      te_fpath_info_db  -> setText( "" );
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
  
  QString subDirName = gmpReport_runname_selected + "_GMP_DB";
  mkdir( US_Settings::reportDir(), subDirName );
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;

  //Clean folder (if exists) where .tar.gz to be unpacked
  QStringList f_exts = QStringList() <<  "*.*";
  QString i_folder = dirName + "/" + gmpReport_runname_selected;
  remove_files_by_mask( i_folder, f_exts );

  // <---- TESTING: gZip algorithm NOT compatible (even for different Linux distros...) **
  //QString GMPReportfname = "GMP_Report_from_DB.tgz";
  // END TESTING *************************************************************************

  QString GMPReportfname = "GMP_Report_from_DB.tar";
  //QString GMPReportfname = "GMP_Report_from_DB.rar";
  QString GMPReportfpath = dirName + "/" + GMPReportfname;
  
  int db_read = db.readBlobFromDB( GMPReportfpath,
				   "download_gmpReportData",
				   gmpReport_id_selected.toInt() );

  if ( db_read == US_DB2::DBERROR )
    {
      QMessageBox::warning(this, "Error", "Error processing file:\n"
			   + GMPReportfpath + "\n" + db.lastError() +
			   "\n" + "Could not open file or no data \n");

      return;
    }
  else if ( db_read != US_DB2::OK )
    {
      QMessageBox::warning(this, "Error", "returned processing file:\n" +
			   GMPReportfpath + "\n" + db.lastError() + "\n");

      return;
    }
  
  // <--- TESTING: tried .tar.gz - NOT compatible (even for different Linux distros...) ****
  // //Un-tar using system TAR && enable View Report btn:
  // QProcess *process = new QProcess(this);
  // process->setWorkingDirectory( dirName );
  // process->start("tar", QStringList() << "-zxvf" << GMPReportfname );
  // END TESTING ****************************************************************************
  
  // // Using .tar (NOT gzip: .tgz or tar.gz !!!)
  QProcess *process = new QProcess(this);
  process->setWorkingDirectory( dirName );
  process->start("tar", QStringList() << "-xvf" << GMPReportfname );
  //process->start("unrar", QStringList() << "x" << GMPReportfname );
  process -> waitForFinished();
  process -> close();
  
  filePath_db = dirName + "/" + gmpReport_runname_selected + "/" + gmpReport_filename_pdf;
  qDebug() << "Extracted .PDF GMP Report filepath -- " << filePath_db;

  //Gui fields
  le_loaded_run_db  -> setText( gmpReport_runname_selected_c );
  pb_view_report_db -> setEnabled( true );
  te_fpath_info_db  -> setText( filePath_db );

  //Inform user of the PDF location
  QMessageBox msgBox;
  msgBox.setText(tr("Report PDF Ready!"));
  msgBox.setInformativeText(tr( "Report was downloaded form DB in .PDF format and saved at: \n%1\n\n"
				"When this dialog is closed, the report can be re-opened by clicking \'View Downloaded Report\' button on the left.")
			    .arg( filePath_db ) );
  
  msgBox.setWindowTitle(tr("Report Generation Complete"));
  QPushButton *Open      = msgBox.addButton(tr("View Report"), QMessageBox::YesRole);
  //QPushButton *Cancel  = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
  
  msgBox.setIcon(QMessageBox::Information);
  msgBox.exec();
  
  if (msgBox.clickedButton() == Open)
    {
      view_report_db( );
    }  
}


// Query autoflow (history) table for records
int US_ReporterGMP::list_all_gmp_reports_db( QList< QStringList >& gmpReportsDBdata, US_DB2* db)
{
  int nrecs        = 0;   
  gmpReportsDBdata.clear();

  QStringList qry;
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
         
      // gmpreportentry << id << autoflowHistoryName // << protocolName
      // 		     << time_created.toString() << filenamePdf;
      gmpreportentry << id << autoflowHistoryName // << protocolName
		     << ptime_created << filenamePdf;
      
      gmpReportsDBdata << gmpreportentry;
      nrecs++;
    }

  return nrecs;
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

  pdiag_autoflow = new US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -4 );

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
  progress_msg = new QProgressDialog ("Accessing run's protocol...", QString(), 0, 11, this);
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
  qApp->processEvents();
  
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );
  
  protocol_details[ "autoflowID" ] = QString::number(autoflowID);

  AutoflowID_auto    = protocol_details[ "autoflowID" ];
  AProfileGUID       = protocol_details[ "aprofileguid" ];
  ProtocolName_auto  = protocol_details[ "protocolName" ];
  invID              = protocol_details[ "invID_passed" ].toInt();
  runID              = protocol_details[ "runID" ];
  runName            = protocol_details[ "experimentName" ];
  FileName           = protocol_details[ "filename" ];
  intensityID        = protocol_details[ "intensityID" ];
  analysisIDs        = protocol_details[ "analysisIDs" ];
  autoflowStatusID   = protocol_details[ "statusID" ];
  optimaName         = protocol_details[ "OptimaName" ];
  dataSource         = protocol_details[ "dataSource" ];

  simulatedData      = false;
  
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

  //check triples for failure
  check_failed_triples( );
  
  //check models existence
  check_models( autoflowID );
  progress_msg->setValue( 7 );
  qApp->processEvents();

  //debug
  for ( int i=0; i < Array_of_tripleNames.size(); ++ i )
    {
      QMap< QString, QString > tmap     =  Triple_to_ModelsDesc    [ Array_of_tripleNames[ i ] ];
      QMap< QString, QString > tmapguid =  Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ];

      QMap < QString, QString >::iterator it;
      for ( it = tmap.begin(); it != tmap.end(); ++it )
	{
	  qDebug() << "ModelsDesc: Triple, QMap -- "
		   << Array_of_tripleNames[ i ]
		   << it.key()
		   << it.value();

	  qDebug() << "ModelsDescGuid: Triple, QMap -- "
		   << Array_of_tripleNames[ i ]
		   << it.key()
		   << tmapguid [ it.key() ];

	}
    }
  
  //DEBUG
  //exit(1);
  
  //identify what's intended to be simulated
  check_for_missing_models();
  ////

  //Check for dropped triples
  check_for_dropped_triples();
  ////

  build_genTree();  
  progress_msg->setValue( 8 );
  qApp->processEvents();

  build_perChanTree();
  progress_msg->setValue( 0 );
  qApp->processEvents();
  
  build_combPlotsTree();
  progress_msg->setValue( 10 );
  qApp->processEvents();

  build_miscTree();  
  progress_msg->setValue( 11 );
  qApp->processEvents();
  
  //debug
  qDebug() << "Built gen Tree: height -- "     << genTree->height();
  qDebug() << "Built perChan Tree: height -- " << perChanTree->height();
  qDebug() << "Built Combo Tree: height -- "   << combPlotsTree->height();

  
  progress_msg->setValue( progress_msg->maximum() );
  qApp->processEvents();
  progress_msg->close();

  //Enable some buttons
  //process runname: if combined, correct for nicer appearance
  QString full_runname = protocol_details[ "filename" ];
  FullRunName_auto = runName + "-run" + runID;
  if ( full_runname.contains(",") && full_runname.contains("IP") && full_runname.contains("RI") )
    {
      QString full_runname_edited  = full_runname.split(",")[0];
      full_runname_edited.chop(3);
      full_runname = full_runname_edited + " (combined RI+IP) ";
      full_runname = runName + " (combined RI+IP)";  //Just use runName (captures ProtDev names)

      FullRunName_auto += " (combined RI+IP)";
    }
      
  //le_loaded_run   ->setText( full_runname );
  le_loaded_run   ->setText( FullRunName_auto );
  pb_gen_report   ->setEnabled( true );
  pb_view_report  ->setEnabled( false );
  pb_select_all   ->setEnabled( true );
  pb_unselect_all ->setEnabled( true );
  pb_expand_all   ->setEnabled( true );
  pb_collapse_all ->setEnabled( true );

  //Capture tree state:
  JsonMask_gen_loaded     = tree_to_json ( topItem );
  JsonMask_perChan_loaded = tree_to_json ( chanItem );
  //Debug
  tree_to_json ( topItemCombPlots );
  GMP_report = true;

  //compose a message on missing models
  QString msg_missing_models = missing_models_msg();
  
  //Inform user that current configuraiton corresponds to GMP report
  if ( GMP_report )
    {
      QMessageBox::information( this, tr( "Report Profile Uploaded" ),
				tr( "Report profile uploaded for GMP run:\n"
				    "%1\n\n"
				    "ATTENTION: Current profile configuration corresponds to GMP report settings.\n\n"
				    "Any changes in the profile settings will result in generation of the non-GMP report!")
				.arg( full_runname ) );
    }
  else
    {
      QMessageBox::information( this, tr( "Report Profile Uploaded" ),
      				QString ( "Report profile uploaded for GMP run:<br><br>"
					  "<b>%1</b><br><br>"
					  "<font color='red'><b>ATTENTION:</b> There are missing models for certain triples: </font><br><br>"
					  "%2<br><br>"
					  "As a result, a <b>non-GMP</b> report will be generated!")
      				.arg( full_runname )
      				.arg( msg_missing_models) );
    }
}


// //public functions:
// int US_ReporterGMP::list_all_autoflow_records_pub( QList< QStringList >& autoflowdata )
// {
//   return list_all_autoflow_records( autoflowdata );
// }

// QMap < QString, QString > US_ReporterGMP::read_autoflow_record_pub( int aID )
// {
//   return read_autoflow_record( aID );
// }
// // END of public functions ///////////////////////////////////////


// Query autoflow (history) table for records
int US_ReporterGMP::list_all_autoflow_records( QList< QStringList >& autoflowdata )
{
  int nrecs        = 0;   
  autoflowdata.clear();

  QStringList qry;
  
  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
			    tr( "Could not connect to database \n" ) + db->lastError() );

      return nrecs;
    }
  
  //Check user level && ID
  QStringList defaultDB = US_Settings::defaultDB();
  QString user_guid   = defaultDB.at( 9 );
  
  //get personID from personGUID
  qry.clear();
  qry << QString( "get_personID_from_GUID" ) << user_guid;
  db->query( qry );
  
  int user_id = 0;
  
  if ( db->next() )
    user_id = db->value( 0 ).toInt();
  

  //deal with autoflowHistory descriptions
  qry. clear();
  qry << "get_autoflow_history_desc";
  db->query( qry );

  while ( db->next() )
    {
      QStringList autoflowentry;
      QString id                 = db->value( 0 ).toString();
      QString runname            = db->value( 5 ).toString();
      QString status             = db->value( 8 ).toString();
      QString optimaname         = db->value( 10 ).toString();
      
      QDateTime time_started     = db->value( 11 ).toDateTime().toUTC();
      QString invID              = db->value( 12 ).toString();

      QDateTime time_created     = db->value( 13 ).toDateTime().toUTC();
      //         QString ptime_created  = US_Util::toUTCDatetimeText( time_created
      //                               .toString( Qt::ISODate ), true )
      //                               .section( " ", 0, 0 ).simplified();
      QString ptime_created    = US_Util::toUTCDatetimeText( time_created
						     .toString( Qt::ISODate ), true )
	                                             .section( ":", 0, 1 ) + " UTC";

      
      QString gmpRun             = db->value( 14 ).toString();
      QString full_runname       = db->value( 15 ).toString();

      QString operatorID         = db->value( 16 ).toString();
      QString devRecord          = db->value( 18 ).toString();

      QDateTime local(QDateTime::currentDateTime());

      if ( devRecord == "Processed" )
	continue;
      
      //process runname: if combined, correct for nicer appearance
      if ( full_runname.contains(",") && full_runname.contains("IP") && full_runname.contains("RI") )
	{
	  QString full_runname_edited  = full_runname.split(",")[0];
	  full_runname_edited.chop(3);

	  full_runname = full_runname_edited + " (combined RI+IP) ";
	  runname += " (combined RI+IP) ";
	}
      
      autoflowentry << id << runname << optimaname  << ptime_created;
	//time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );

      if ( time_started.toString().isEmpty() )
	autoflowentry << QString( tr( "NOT STARTED" ) );
      else
	{
	  if ( status == "LIVE_UPDATE" )
	    autoflowentry << QString( tr( "RUNNING" ) );
	  if ( status == "EDITING" || status == "EDIT_DATA" || status == "ANALYSIS" || status == "REPORT" || status == "E-SIGNATURES" )
	    autoflowentry << QString( tr( "COMPLETED" ) );
	    //autoflowentry << time_started.toString();
	}

      if ( status == "EDITING" )
	status = "LIMS_IMPORT";
      
      autoflowentry << status << gmpRun;

      //Check user level && GUID; if <3, check if the user is operator || investigator
      if ( US_Settings::us_inv_level() < 3 )
	{
	  qDebug() << "User level low: " << US_Settings::us_inv_level();
	  qDebug() << "user_id, operatorID.toInt(), invID.toInt() -- " << user_id << operatorID.toInt() << invID.toInt();

	  //if ( user_id && ( user_id == operatorID.toInt() || user_id == invID.toInt() ) )
	  if ( user_id && user_id == invID.toInt() )
	    {//Do we allow operator as defined in autoflow record to also see reports?? 
	    
	      autoflowdata  << autoflowentry;
	      nrecs++;
	    }
	}
      else
	{
	  autoflowdata  << autoflowentry;
	  nrecs++;
	}
      
    }

  //test
  delete db;
  
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


   //test
   delete db;
   
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

  qDebug() << "After Inheriting Protocol -- ";
  
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
  //Channel run/analysis_run
  analysis_runs          = currAProf.analysis_run;
  report_runs            = currAProf.report_run;
  //Channel reports
  ch_reports             = currAProf.ch_reports;
  //Channel wavelengths
  ch_wvls                = currAProf.ch_wvls;
  //Replicates
  replicates                   = currAProf. replicates;
  //replicates_to_channdesc
  replicates_to_channdesc      = currAProf. replicates_to_channdesc_main; //Empty ? (not needed?)
  //channdesc_to_overlapping_wvls
  channdesc_to_overlapping_wvls = currAProf. channdesc_to_overlapping_wvls_main;
  
  //Debug: AProfile
  qDebug() << "chndescs_alt QStringList -- " <<  chndescs_alt;
  qDebug() << "ch_reports.keys() -- " <<  ch_reports.keys();
  
  QString channel_desc_alt = chndescs_alt[ 0 ];
  QString channel_desc     = chndescs[ 0 ];
  QString wvl              = QString::number( ch_wvls[ channel_desc_alt ][ 0 ] );

  qDebug() << "Wavelengths ch_wvls[ channel_desc_alt ] " << ch_wvls[ channel_desc_alt ] << " for channel: " << channel_desc_alt;
  US_ReportGMP reportGMP   = ch_reports[ channel_desc_alt ][ wvl ];

  if ( reportGMP.reportItems.size() > 0  )
    qDebug() << "AProfile's && ReportGMP's details: -- "
	     << currAProf.aprofname
	     << currAProf.protoname
	     << currAProf.chndescs
	     << currAProf.chndescs_alt
	     << currAProf.lc_ratios
	     << cAP2.parms[ 0 ].channel
	     << cAPp.parms[ 0 ].channel
	     << reportGMP.rmsd_limit
	     << reportGMP.wavelength
	     << reportGMP.reportItems[ 0 ].type
	     << reportGMP.reportItems[ 0 ].ind_combined_plot;
  else
    qDebug() << "AProfile's && ReportGMP's details: -- "
	     << currAProf.aprofname
	     << currAProf.protoname
	     << currAProf.chndescs
	     << currAProf.chndescs_alt
	     << currAProf.lc_ratios
	     << cAP2.parms[ 0 ].channel
	     << cAPp.parms[ 0 ].channel
	     << reportGMP.rmsd_limit
	     << reportGMP.wavelength
	     << "No GMP_Report_Items!!!";

  qDebug() << "Number of wvls in channel: " << chndescs_alt[ 0 ] << ": " <<  ch_wvls[ channel_desc_alt ].size();
  qDebug() << "Wvls in channel: " << chndescs_alt[ 0 ] << ": " << ch_wvls[ channel_desc_alt ];
  qDebug() << "Pseudo3d pars: "
	   << reportGMP .pseudo3d_2dsait_s_ff0
	   << reportGMP .pseudo3d_2dsait_s_d
	   << reportGMP .pseudo3d_2dsait_mw_ff0
	   << reportGMP .pseudo3d_2dsait_mw_d;

  //autoflowIntensity
  intensityRIMap = read_autoflowIntensity( intensityID, &db );
  progress_msg->setValue( 5 );
  qApp->processEvents();
  
  //report Mask
  QString gen_reportMask = currAProf.report_mask;
  parse_gen_mask_json( gen_reportMask );

  progress_msg->setValue( 6 );
  qApp->processEvents();

  qDebug() << "General ReportMask: " << gen_reportMask;

  //combined_plots params
  QString combPlots_parms = currAProf.combPlots_parms;
  comboPlotsMap = parse_comb_plots_json( combPlots_parms );

  //read eSign record for opers, revs, apprs.
  eSign_details. clear();
  eSign_details = read_autoflowGMPReportEsign_record( &db );
    
}

//combPlots parms
QMap< QString, QMap< QString, QString > > US_ReporterGMP::parse_comb_plots_json( QString combPlots )
{
  QMap< QString, QMap< QString, QString > > iMap;
  qDebug() << "combPlots: " << combPlots;
  //parse JSON into QMap
  if ( !combPlots.isEmpty() )
    {
      QJsonDocument jsonDoc = QJsonDocument::fromJson( combPlots.toUtf8() );
      QJsonObject json_obj = jsonDoc.object();

      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);

	  qDebug() << "CombPlots key (type,method), value: " << key << value;
	  
	  QJsonArray json_array = value.toArray();
	  for (int i=0; i < json_array.size(); ++i )
	    {
	      foreach(const QString& array_key, json_array[i].toObject().keys())
		{
		  iMap[ key ][ array_key ] = json_array[i].toObject().value(array_key).toString();

		  qDebug() << "ComboPlots Map: -- type_method, parameter, value: "
			   << key
			   << array_key
			   << json_array[i].toObject().value(array_key).toString();
		}
	    }
	}
    }

  return iMap;
}

//read Intensity info
QMap< QString, QString > US_ReporterGMP::read_autoflowIntensity( QString ID, US_DB2* db )
{
  QString iJson;
  QMap< QString, QString > iMap;
  QStringList qry;
  qry << "read_autoflow_intensity_record" << ID;
  db->query( qry );

  qDebug() << "readIntens: qry -- " << qry;
    
  if ( db->lastErrno() == US_DB2::OK )      // Intensity record exists
    {
      while ( db->next() )
	{
	  iJson  = db->value( 0 ).toString();
	}
    }

  qDebug() << "iJson: " << iJson;
  //parse JSON into QMap
  if ( !iJson.isEmpty() )
    {
      QJsonDocument jsonDoc = QJsonDocument::fromJson( iJson.toUtf8() );
      QJsonObject json_obj = jsonDoc.object();

      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);

	  if ( value.isString() )  //single-wvl
	    {
	      iMap["singlewvl"] = value.toString();
	    }
	  else if ( value.isArray() ) //MWL
	    {
	      QJsonArray json_array = value.toArray();
	      for (int i=0; i < json_array.size(); ++i )
		{
		  foreach(const QString& array_key, json_array[i].toObject().keys())
		    {
		      iMap[ array_key ] = json_array[i].toObject().value(array_key).toString();

		      qDebug() << "Intensity: MWL -- array_key, val: "
			       << array_key
			       << json_array[i].toObject().value(array_key).toString();
		    }
		}
	    }
	}
    }
  
  return iMap;
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

//build miscellaneous report mask  tree
void US_ReporterGMP::build_miscTree ( void )
{
  QString indent( "  " );
  QStringList topItemNameList;
  int wiubase = (int)QTreeWidgetItem::UserType;

  miscTopLevelItems. clear();
  miscTopLevelItems << "User Input" << "Run Details";
  
  if ( !channdesc_to_overlapping_wvls.isEmpty() ) 
    miscTopLevelItems << "Replicate Groups Averaging";

  for ( int i=0; i<miscTopLevelItems.size(); ++i )
    {
      QString topItemName = miscTopLevelItems[i];
      topItemNameList.clear();
      topItemNameList << "" << indent + topItemName;
      miscItem [ topItemName ] = new QTreeWidgetItem( miscTree, topItemNameList, wiubase );

      miscItem [ topItemName ] ->setCheckState( 0, Qt::Checked );
    }

  miscTree->expandAll();    
  miscTree->resizeColumnToContents( 0 );
  miscTree->resizeColumnToContents( 1 );

  // qDebug() << "misc Tree first time build ? " << first_time_misc_tree_build;
  // if ( first_time_misc_tree_build )
  //   {
  //     qDebug() << "Resizing misc Tree: ";
  //     miscTree->setMinimumHeight( (miscTree->height())*0.6 );
  //     first_time_misc_tree_build = false;
  //   }
  
  connect( miscTree, SIGNAL( itemChanged   ( QTreeWidgetItem*, int ) ),
  	   this,    SLOT  ( changedItem   ( QTreeWidgetItem*, int ) ) );

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

  //qDebug() << "Build gen Tree: height -- " << genTree->height();
  qDebug() << "gen Tree first time build ? " << first_time_gen_tree_build;
  if ( first_time_gen_tree_build )
    {
      qDebug() << "Resizing gen Tree: ";
      genTree->setMinimumHeight( (genTree->height())*1.9 );
      first_time_gen_tree_build = false;
    }
  
  connect( genTree, SIGNAL( itemChanged   ( QTreeWidgetItem*, int ) ),
  	   this,    SLOT  ( changedItem   ( QTreeWidgetItem*, int ) ) );

}

//get Item's children
void US_ReporterGMP::get_item_childs( QList< QTreeWidgetItem* > & children_list, QTreeWidgetItem* item)
{
  children_list << item;
  
  int children = item->childCount();
  for ( int i = 0; i < children; ++i )
    get_item_childs( children_list, item->child(i) ); 
}

//What to check/uncheck upon change in items status
void US_ReporterGMP::changedItem( QTreeWidgetItem* item, int col )
{
  //we deal with col 0 only
  if ( col != 0  ) 
    return;

  //disconnect
  item -> treeWidget() -> disconnect();

  //go over children: recursive search for all children down the tree
  int children = item->childCount();
  QList< QTreeWidgetItem* > children_list;

  for( int i = 0; i < children; ++i )
    get_item_childs( children_list, item->child(i) ); 

  for ( int i = 0; i < children_list.size(); ++i )
    children_list[ i ] -> setCheckState( 0, (Qt::CheckState) item->checkState(0) );

  //Go over parents
  QTreeWidgetItem* parent_item = item->parent();
  QList< QTreeWidgetItem* > parents_list;
  QTreeWidgetItem* current_p_item = new QTreeWidgetItem;
  
  while ( parent_item )
    {
      parents_list << parent_item;
      current_p_item = parent_item;
      parent_item  = current_p_item -> parent();
    }
  
  for( int j = 0; j < parents_list.size(); ++j )
    {
      int checked_children = 0;
      int parent_item_children = parents_list[ j ] ->childCount();
      for( int jj = 0; jj < parent_item_children; ++jj )
	{
	  if ( int( parents_list[ j ]->child( jj )->checkState(0) ) )
	    ++checked_children;
	}
      
      if ( checked_children )
	parents_list[ j ]->setCheckState( 0, Qt::Checked );
      else
	parents_list[ j ]->setCheckState( 0, Qt::Unchecked );
    }

  //reconnect
  connect( item -> treeWidget(), SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
	   this,                 SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
}


//build perChanTree
void US_ReporterGMP::build_perChanTree ( void )
{
  QString indent( "  " );
  QStringList chanItemNameList, tripleItemNameList,
    tripleItemModelNameList, tripleMaskItemNameList,
    tripleMaskItemPlotNameList, tripleMaskItemPseudoNameList;

  QStringList tripleReportModelsList;
  QStringList tripleReportMasksList;
  QList< bool > tripleReportMasksList_vals;
  QList< bool > tripleReportMasksPseudoList_vals;
  QStringList tripleReportMasksPlotList;
  QStringList tripleReportMasksPseudoList;
  QStringList tripleReportMasksIndividualPlotList;
  
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
	  QString chanItemName = "Channel " + channel_desc_alt.section( ":", 0, 1 ).replace(":","-");
	  chanItemNameList.clear();
	  chanItemNameList << "" << indent + chanItemName;
	  chanItem [ chanItemName ] = new QTreeWidgetItem( perChanTree, chanItemNameList, wiubase );
	  
	  //QList < double > chann_wvls                  = ch_wvls[ channel_desc ];
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc_alt ];
	  QMap < QString, US_ReportGMP > chann_reports = ch_reports[ channel_desc_alt ];

	  qDebug() << "channel_desc_alt , channel_desc: "   << channel_desc_alt << " , " <<  channel_desc;
	  qDebug() << "chann_wvls [ channel_desc ] -- "     << chann_wvls;
	  qDebug() << "chann_wvls [ channel_desc_alt ] -- " << ch_wvls[ channel_desc_alt ];
	  
	  int chann_wvl_number = chann_wvls.size();

	  int checked_triples = 0;
	  for ( int jj = 0; jj < chann_wvl_number; ++jj )
	    {
	      QString wvl            = QString::number( chann_wvls[ jj ] );
	      QString triple_name    = channel_desc.split(":")[ 0 ] + "/" + wvl;

	      //Push to Array_of_triples;
	      //QString tripleName = channel_desc_alt.section( ":", 0, 0 )[0] + "." + channel_desc_alt.section( ":", 0, 0 )[1] + "." + wvl;
	      QString tripleName = channel_desc_alt.section( ":", 0, 0 )[0] + "." + channel_desc_alt.section( ":", 0, 0 )[1];

	      if ( channel_desc_alt.contains( "Interf" ) ) 
		tripleName += ".Interference";
	      else
		tripleName += "." + wvl;

	      //'S' data
	      if ( dataSource. contains("DiskAUC:Absorbance") && simulatedData )
		tripleName = tripleName.replace( ".A.", ".S." );
	      	      
	      qDebug() << "[in build_perChanTree()]: TripleName -- " << tripleName; 
	      Array_of_triples.push_back( tripleName );

	      //Triple item: child-level 1 in a perChanTree
	      QString tripleItemName = wvl + " nm Wavelength";
	      tripleItemNameList.clear();
	      tripleItemNameList << "" << indent.repeated( 2 ) + tripleItemName;
	      tripleItem [ tripleItemName ] = new QTreeWidgetItem( chanItem [ chanItemName ], tripleItemNameList, wiubase);

	      US_ReportGMP reportGMP = chann_reports[ wvl ];

	      //Identify which type-methods will be in Combined ptols
	      int report_items_number = reportGMP.reportItems.size();
	      for ( int kk = 0; kk < report_items_number; ++kk )
		{
		  US_ReportGMP::ReportItem curr_item = reportGMP.reportItems[ kk ];
		  QString type           = curr_item.type;
		  QString method         = curr_item.method;
		  if( method.contains ("PCSA") )
		    method = "PCSA";
		  int     combPlot       = curr_item.combined_plot;

		  QString t_m = type + "," + method;
		  if ( combPlot )
		    comboPlotsMapTypes[ t_m ] = 1;

		  //Use ind_combined_plot here, to mark if ind. plots for triple's reportItem is to be plot
		  int     ind_combPlot   = curr_item.ind_combined_plot;
		  QString range_l_h = QString::number( curr_item.range_low ) + ":" + QString::number( curr_item.range_high );

		  if ( Triple_to_Models[ tripleName ].contains( method ))
		    {
		      if ( ind_combPlot )
			{
			  //QMap< QString (triple), QMap< QString( type_method ), QStringList( ranges ) > >
			  indComboPlotsMapTripleTypeRangeBool[ tripleName ][ t_m ][range_l_h] = true;
			}
		      else
			indComboPlotsMapTripleTypeRangeBool[ tripleName ][ t_m ][range_l_h] = false;
		    }
		}
	      ////////////////////////////////////////////////////////
	      
	      qDebug() << "For triple, " << tripleName << ", and models: " << Triple_to_Models[ tripleName ]
		       << reportGMP. tot_conc_mask
		       << reportGMP. rmsd_limit_mask
		       << reportGMP. av_intensity_mask
		       << reportGMP. experiment_duration_mask
		       << reportGMP. integration_results_mask
		       << reportGMP. plots_mask
		       << reportGMP. pseudo3d_mask;
	      


	  
	      //define models per triple:
	      tripleReportModelsList.clear();
	      tripleReportModelsList = Triple_to_Models[ tripleName ];
	      
	      int checked_masks = 0;
	      //start triple's models
	      for ( int mm = 0; mm < tripleReportModelsList.size(); ++mm )
		{
		  //Triple's mask params: child-level 3 in a perChanTree
		  QString tripleItemModelName = tripleReportModelsList[ mm ] + " Model";
		  QString tripleModelName     = tripleItemName + "," + tripleItemModelName;
		  tripleItemModelNameList.clear();
		  tripleItemModelNameList << "" << indent.repeated( 3 ) + tripleItemModelName;
		  tripleModelItem [ tripleModelName ] = new QTreeWidgetItem(  tripleItem [ tripleItemName ], tripleItemModelNameList, wiubase);

		  //Populate tripleReportMasksList && values from scratch
		  tripleReportMasksList.clear();
		  tripleReportMasksList << "Total Concentration"
					<< "RMSD Limit"
					<< "Minimum Intensity"
		    //<< "Experiment Duration"
					<< "Loading Volume"
					<< "Integration Results"
					<< "Plots"
					<< "Pseudo3d Distributions";
		  
		  tripleReportMasksList_vals.clear();
		  tripleReportMasksList_vals << reportGMP. tot_conc_mask
					     << reportGMP. rmsd_limit_mask
					     << reportGMP. av_intensity_mask
		    //<< reportGMP. experiment_duration_mask
					     << true
					     << reportGMP. integration_results_mask
					     << reportGMP. plots_mask
					     << reportGMP. pseudo3d_mask;
		  
		  //Here: add "Individual Distributions" to tripleReportMasksList IF there is/are type-method(s) matching current model:
		  QStringList type_methods_list = indComboPlotsMapTripleTypeRangeBool[ tripleName ].keys();
		  for ( int tm=0; tm<type_methods_list.size(); tm++ )
		    {
		      //select type-method for current model only! [e.g. {s,D}-2DSA-IT for 2DSA-IT model]
		      QString c_method = type_methods_list[ tm ].split(",")[1];
		      if ( c_method == tripleReportModelsList[ mm ] )
			{
			  qDebug() << "In Model, list, ind. : " <<  type_methods_list << tripleReportModelsList[ mm ];
			  tripleReportMasksList      << "Individual Combined Distributions";
			  tripleReportMasksList_vals << true;

			  break;
			}
		    }
		  
		  //start triple's masks
		  for ( int kk = 0; kk < tripleReportMasksList.size(); ++kk )
		    {
		      //Triple's mask params: child-level 3 in a perChanTree
		      QString tripleMaskItemName = tripleReportMasksList[ kk ];
		      tripleMaskItemNameList.clear();
		      tripleMaskItemNameList << "" << indent.repeated( 4 ) + tripleMaskItemName;
		      tripleMaskItem [ tripleModelName ] = new QTreeWidgetItem(  tripleModelItem [ tripleModelName ], tripleMaskItemNameList, wiubase);
		      
		      if ( tripleReportMasksList_vals[ kk ] )
			{
			  tripleMaskItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
			  ++checked_masks;
			}
		      else
			tripleMaskItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
		      
		      //plots
		      if ( tripleMaskItemName.contains("Plots") )
			{
			  tripleReportMasksPlotList.clear();
			  tripleReportMasksPlotList << "Experiment-Simulation Velocity Data (noise corrected) "
						    << "Experiment-Simulation Residuals"
						    << "Sedimentation Coefficient Distribution"
						    << "Molecular Weight Distribution"
						    << "Diffusion Coefficient Distribution"
						    << "f/f0-vs-s 2D Model"
						    << "f/f0-vs-MW 2D Model"
						    << "D-vs-s 2D Model"
						    << "D-vs-MW 2D Model";
			  //<< "3D Model Plot";
								  
			  for ( int hh = 0; hh < tripleReportMasksPlotList.size(); ++hh )
			    {
			      QString tripleMaskItemPlotName = tripleReportMasksPlotList[ hh ];
			      tripleMaskItemPlotNameList.clear();
			      tripleMaskItemPlotNameList << "" << indent.repeated( 5 ) + tripleMaskItemPlotName;
			      tripleMaskPlotItem [ tripleModelName ] = new QTreeWidgetItem( tripleMaskItem [ tripleModelName ], tripleMaskItemPlotNameList, wiubase);

			      if ( tripleReportMasksList_vals[ kk ] )
				{
				  tripleMaskPlotItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
				  ++checked_masks;
				}
			      else
				tripleMaskPlotItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
			    }
			}
		      //end of triple's masks
		      
		      //pseudo3d distr.
		      if ( tripleMaskItemName.contains("Pseudo3d") )
			{
			  tripleReportMasksPseudoList.clear();
			  tripleReportMasksPseudoList << "Pseudo3d s-vs-f/f0 Distribution"
						      << "Pseudo3d s-vs-D Distribution"
						      << "Pseudo3d MW-vs-f/f0 Distribution"
						      << "Pseudo3d MW-vs-D Distribution";

			  tripleReportMasksPseudoList_vals. clear();
			  if ( tripleItemModelName.contains( "2DSA-IT" )  )
			    {
			      tripleReportMasksPseudoList_vals << reportGMP. pseudo3d_2dsait_s_ff0
							       << reportGMP. pseudo3d_2dsait_s_d
							       << reportGMP. pseudo3d_2dsait_mw_ff0
							       << reportGMP. pseudo3d_2dsait_mw_d;
			      qDebug() << "In pseudo3d tree: tripleReportMasksPseudoList_vals (2DSA-IT) -- " << tripleReportMasksPseudoList_vals;
			    }
			  else if ( tripleItemModelName.contains( "2DSA-MC" ) )
			    {
			      tripleReportMasksPseudoList_vals << reportGMP. pseudo3d_2dsamc_s_ff0
							       << reportGMP. pseudo3d_2dsamc_s_d
							       << reportGMP. pseudo3d_2dsamc_mw_ff0
							       << reportGMP. pseudo3d_2dsamc_mw_d;
			      qDebug() << "In pseudo3d tree: tripleReportMasksPseudoList_vals (2DSA-MC) -- " << tripleReportMasksPseudoList_vals;
			    }
			  else if ( tripleItemModelName.contains( "PCSA" ) )
			    {
			      tripleReportMasksPseudoList_vals << reportGMP. pseudo3d_pcsa_s_ff0
							       << reportGMP. pseudo3d_pcsa_s_d
							       << reportGMP. pseudo3d_pcsa_mw_ff0
							       << reportGMP. pseudo3d_pcsa_mw_d;
			      qDebug() << "In pseudo3d tree: tripleReportMasksPseudoList_vals (PCSA) -- " << tripleReportMasksPseudoList_vals;
			    }
			  			  
			  for ( int ps = 0; ps < tripleReportMasksPseudoList.size(); ++ps )
			    {
			      QString tripleMaskItemPseudoName = tripleReportMasksPseudoList[ ps ];
			      tripleMaskItemPseudoNameList.clear();
			      tripleMaskItemPseudoNameList << "" << indent.repeated( 5 ) + tripleMaskItemPseudoName;
			      tripleMaskPseudoItem [ tripleModelName ] = new QTreeWidgetItem( tripleMaskItem [ tripleModelName ], tripleMaskItemPseudoNameList, wiubase);

			      if ( tripleReportMasksPseudoList_vals[ ps ] )
				{
				  tripleMaskPseudoItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
				  ++checked_masks;
				}
			      else
				tripleMaskPseudoItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
			    }
			}
		      //end of pseudo3d distr. masks

		      //Individual Combined Distro
		      if ( tripleMaskItemName.contains("Individual") )
			{
			  tripleReportMasksIndividualPlotList.clear();
			  //populate tripleReportMasksIndividualPlotList based on type-method entries in the reportItem:
			  QStringList types_methods_list = indComboPlotsMapTripleTypeRangeBool[ tripleName ].keys();
			  for ( int tm=0; tm<types_methods_list.size(); tm++ )
			    {
			      //select type-method for current model only! [e.g. {s,D}-2DSA-IT for 2DSA-IT model]
			      QString c_method = types_methods_list[ tm ].split(",")[1];
			      QString c_type   = types_methods_list[ tm ].split(",")[0];
			      if ( c_method ==  tripleReportModelsList[ mm ] )
				{
				  QMap< QString, bool> c_ranges_bool = indComboPlotsMapTripleTypeRangeBool[ tripleName ][ types_methods_list[ tm ] ];
				  QMap<QString, bool>::iterator r_bool;
				  for ( r_bool = c_ranges_bool.begin(); r_bool != c_ranges_bool.end(); ++r_bool )
				    {
				      QString r_key = r_bool.key();
				      bool r_value  = r_bool.value();
				      QString to_show_ind = r_value ? "YES" : "NO";
				      QString c_type_range = c_type + "[" + r_key + "]" + "--" + to_show_ind;
				      tripleReportMasksIndividualPlotList << c_type_range;
				    }
				}
			    }
			  //Now, mark checkboxes
			  bool top_ind_comboPlot = false;
			  for ( int icp = 0; icp < tripleReportMasksIndividualPlotList.size(); ++icp )
			    {
			      QString individualPlotName = tripleReportMasksIndividualPlotList[ icp ];
			      QString individualPlotName_name = tripleReportMasksIndividualPlotList[ icp ].split("--")[0];
			      QString individualPlotName_show = tripleReportMasksIndividualPlotList[ icp ].split("--")[1];
			      QStringList individualPlotNameList;
			      individualPlotNameList << "" << indent.repeated( 5 ) + individualPlotName_name;

			      tripleMaskIndComboPlotItem [ tripleModelName ] = new QTreeWidgetItem( tripleMaskItem [ tripleModelName ],
												    individualPlotNameList, wiubase);
			      //Checked/no
			      qDebug() << "To show/no Ind.ComboPlots -- "
				       << tripleReportMasksList_vals[ kk ]
				       << individualPlotName_show;
			      if ( tripleReportMasksList_vals[ kk ] && individualPlotName_show=="YES")
				{
				  tripleMaskIndComboPlotItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
				  ++checked_masks;
				  top_ind_comboPlot = true;
				}
			      else
				tripleMaskIndComboPlotItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
			    }

			  //Now, set top-level, model-specific Ind. Comboplot item
			  if ( top_ind_comboPlot )
			    tripleMaskItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
			  else
			    tripleMaskItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
			}
		      //end of Individual Combined plots
		    }
		  
		  if ( checked_masks )
		    tripleModelItem [ tripleModelName ] ->setCheckState( 0, Qt::Checked );
		  else
		    tripleModelItem [ tripleModelName ] ->setCheckState( 0, Qt::Unchecked );
		  
		}
	      //end of triple's models

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

  //Now check if tripleItem tree widgets have any children (have any models):
  QMap < QString, QTreeWidgetItem * >::iterator ch;
  for ( ch = chanItem.begin(); ch != chanItem.end(); ++ch )
    {
      int channel_children = ch.value()->childCount();
      int hidden_triples = 0;
      
      for ( int i = 0; i < channel_children; ++i )
	{
	  QTreeWidgetItem* child_triple = ch.value()->child( i );
	  if ( !child_triple-> childCount() )
	    {
	      qDebug() << "Channel's " << ch.value()->text( 1 )
		       << " triple: "
		       << child_triple->text(1)
		       << " has NO children";

	      ++hidden_triples;
	      //Hide triple treeItem
	      child_triple->setHidden( true );
	    }
	}
      if ( channel_children == hidden_triples )
	{
	  qDebug() << "We have to hide entire channel  " << ch.value()->text( 1 );
	  //Hide channel treeItem
	  ch.value()->setHidden( true );
	}
    }
  //End of disabling empty tree branches
  
  perChanTree->expandAll();
  //perChanTree->expandToDepth( 2 );
  perChanTree->resizeColumnToContents( 0 );
  perChanTree->resizeColumnToContents( 1 );

  //qDebug() << "Build perChan Tree: height -- " << perChanTree->height();
  qDebug() << "perChan Tree first time build ? " << first_time_perChan_tree_build;
  if ( first_time_perChan_tree_build )
    {
      qDebug() << "Resizing perChan Tree: ";
      perChanTree->setMinimumHeight( (perChanTree->height())*2.0 );
      first_time_perChan_tree_build = false;
    }
  
  connect( perChanTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	   this,        SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
}


//select all items in trees
void US_ReporterGMP::build_combPlotsTree ( void )
{
  QString indent( "  " );

  QStringList topItemNameList, ItemNameList;
 
  int wiubase = (int)QTreeWidgetItem::UserType;

  CombPlots_Type_to_Models.clear();

  QMap<QString, int >::iterator cpt;
  for ( cpt = comboPlotsMapTypes.begin(); cpt != comboPlotsMapTypes.end(); ++cpt )
    {
      if ( cpt.value() )
	{
	  QString type  = cpt.key().split(",")[ 0 ];
	  QString model = cpt.key().split(",")[ 1 ];

	  CombPlots_Type_to_Models[ type ] << model;
	  
	}
    }

  //remove duplicates && build tree
  QMap<QString, QStringList >::iterator tm;
  for ( tm = CombPlots_Type_to_Models.begin(); tm != CombPlots_Type_to_Models.end(); ++tm )
    {
      CombPlots_Type_to_Models[ tm.key() ].removeDuplicates();
      QStringList unique_models = CombPlots_Type_to_Models[ tm.key() ];

      topItemNameList.clear();
      topItemNameList << "" << indent + tm.key();
      topItemCombPlots [ tm.key() ] = new QTreeWidgetItem( combPlotsTree, topItemNameList, wiubase );

      topItemCombPlots [ tm.key() ] ->setCheckState( 0, Qt::Checked );

      for( int i=0; i<unique_models.size(); ++i )
	{
	  QString modelName =  tm.key() + "," + unique_models[ i ];
	  ItemNameList.clear();
	  ItemNameList << "" << indent.repeated( 2 ) + unique_models[ i ];
	  ItemCombPlots [ modelName ] = new QTreeWidgetItem( topItemCombPlots [ tm.key() ], ItemNameList, wiubase);

	  ItemCombPlots [ modelName ] ->setCheckState( 0, Qt::Checked );
	}
    }

  combPlotsTree->expandAll();
  //perChanTree->expandToDepth( 2 );
  combPlotsTree->resizeColumnToContents( 0 );
  combPlotsTree->resizeColumnToContents( 1 );

  //combPlotsTree->setMaximumHeight(30);

  connect( combPlotsTree, SIGNAL( itemChanged( QTreeWidgetItem*, int ) ),
  	   this,          SLOT(   changedItem( QTreeWidgetItem*, int ) ) );
  
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

  QTreeWidgetItem* combPlotsTree_rootItem = combPlotsTree -> invisibleRootItem();
  for( int i = 0; i < combPlotsTree_rootItem->childCount(); ++i )
    {
      combPlotsTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      combPlotsTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
    }
  
  QTreeWidgetItem* miscTree_rootItem     = miscTree     -> invisibleRootItem();
  for( int i = 0; i < miscTree_rootItem->childCount(); ++i )
    {
      miscTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
      miscTree_rootItem->child(i)->setCheckState( 0, Qt::Checked );
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

  QTreeWidgetItem* combPlotsTree_rootItem = combPlotsTree -> invisibleRootItem();
  for( int i = 0; i < combPlotsTree_rootItem->childCount(); ++i )
    {
      combPlotsTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }

  QTreeWidgetItem* miscTree_rootItem     = miscTree     -> invisibleRootItem();
  for( int i = 0; i < miscTree_rootItem->childCount(); ++i )
    {
      miscTree_rootItem->child(i)->setCheckState( 0, Qt::Unchecked );
    }
}

//expand all items in trees
void US_ReporterGMP::expand_all ( void )
{
  genTree       ->expandAll();
  perChanTree   ->expandAll();
  combPlotsTree ->expandAll();
  miscTree      ->expandAll();
}

//collapse all items in trees
void US_ReporterGMP::collapse_all ( void )
{
  genTree       ->collapseAll();
  perChanTree   ->collapseAll();
  combPlotsTree ->collapseAll();
  miscTree      ->collapseAll();
}

//view report
void US_ReporterGMP::view_report ( void )
{
  qDebug() << "Opening PDF at -- " << filePath;

  QFileInfo check_file( filePath );
  if (check_file.exists() && check_file.isFile())
    {
      //Open with OS's applicaiton settings ?
      QDesktopServices::openUrl(QUrl( filePath ));
    }
  else
    {
      QMessageBox::warning( this, tr( "Error: Cannot Open .PDF File" ),
			    tr( "%1 \n\n"
				"No such file or directory...") .arg( filePath ) );
    }
}

//view report DB
void US_ReporterGMP::view_report_db ( void )
{
  qDebug() << "Opening PDF (for downloaded form DB) at -- " << filePath_db;

  QFileInfo check_file( filePath_db );
  if (check_file.exists() && check_file.isFile())
    {
      //Open with OS's applicaiton settings ?
      QDesktopServices::openUrl(QUrl( filePath_db ));
    }
  else
    {
      QMessageBox::warning( this, tr( "Error: Cannot Open .PDF File" ),
			    tr( "%1 \n\n"
				"No such file or directory...") .arg( filePath_db ) );
    }
}
 

//reset
void US_ReporterGMP::reset_report_panel ( void )
{
  le_loaded_run -> setText( "" );

  if ( !auto_mode )
    {
      te_fpath_info    -> setText( "" );
      le_loaded_run_db -> setText( "" );
      te_fpath_info_db -> setText( "" );
    }
      
  //cleaning genTree && it's objects
  // for (int i = 0; i < genTree->topLevelItemCount(); ++i)
  //   {
  //     qDeleteAll(genTree->topLevelItem(i)->takeChildren());
  //   }
  genTree     -> clear();
  genTree     -> disconnect();

  qDebug() << "Size Hint for gen Tree -- " << genTree->sizeHint();
  //genTree->resize( genTree->sizeHint() );
  qApp->processEvents();

  if ( auto_mode )
    pb_view_report_auto->setVisible( false );
  
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
  perChanTree ->disconnect();
  qDebug() << "Size Hint for perChan Tree -- " << perChanTree->sizeHint();
  //perChanTree->resize( perChanTree->sizeHint() );
  qApp->processEvents();

  chanItem           .clear();
  tripleItem         .clear();
  tripleModelItem    .clear();
  tripleMaskItem     .clear();
  tripleMaskPlotItem .clear();
  tripleMaskIndComboPlotItem. clear();

  //cleaning combPlotsTriple tree & it's objects
  combPlotsTree ->clear();
  combPlotsTree ->disconnect();
  qDebug() << "Size Hint for combPlots Tree -- " << combPlotsTree->sizeHint();
  //combPlotsTree->resize( combPlotsTree->sizeHint() );
  qApp->processEvents();

  topItemCombPlots .clear();
  ItemCombPlots    .clear();

  //cleaning misc tree & it's objects
  miscTree ->clear();
  miscTree ->disconnect();
  qDebug() << "Size Hint for misc Tree -- " << miscTree->sizeHint();
  qApp->processEvents();

  miscItem             .clear();
  miscTopLevelItems    .clear();

  //Clear loaded JsonMasks for gen/perChan trees
  JsonMask_gen_loaded     .clear();
  JsonMask_perChan_loaded .clear();

  //Set GMP_report bool to true:
  GMP_report = true;

  //clean triple_array
  Array_of_triples.clear();
  Array_of_tripleNames.clear();

  //clean intensity RI Map
  intensityRIMap .clear();

  //clear comboplots Maps
  comboPlotsMap            .clear();
  comboPlotsMapTypes       .clear();
  indComboPlotsMapTripleTypeRangeBool. clear();
  CombPlots_Type_to_Models .clear();

  //
  CombPlotsParmsMap        .clear();
  CombPlotsParmsMap_Colors .clear();
  
  //clean QMap connecting triple names to their models
  Triple_to_Models         . clear();
  Triple_to_ModelsDesc     . clear();
  Triple_to_ModelsDescGuid . clear();
  Triple_to_ModelsMissing  . clear();
  Triple_to_FailedStage    . clear();

  droppedTriplesList       . clear();
  
  //reset US_Protocol && US_AnaProfile
  currProto = US_RunProtocol();  
  currAProf = US_AnaProfile();

  //GMP Esign Map
  eSign_details. clear();

  //reset html assembled strings
  html_assembled.clear();
  html_failed. clear();
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
  //create main folder & clean it of anything
  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  QStringList f_exts = QStringList() <<  "*.*"; 
  remove_files_by_mask( dirName, f_exts );
  ///////////////////////////////////////////////////
  
  progress_msg->setWindowTitle(tr("Generating Report"));
  progress_msg->setLabelText( "Generating Report: Part 1..." );
  int msg_range = currProto.rpSolut.nschan + 5;

  qDebug() << "Generate report: msg_range -- " << msg_range;
  progress_msg->setRange( 0, msg_range );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();

  //reset html assembled strings
  html_assembled.clear();
  html_failed.clear();
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

  
  //Part 1
  gui_to_parms();
  progress_msg->setValue( 1 );
  qApp->processEvents();
  
  get_current_date();
  progress_msg->setValue( 2 );
  qApp->processEvents();
  
  format_needed_params();
  progress_msg->setValue( 3 );
  qApp->processEvents();

  assemble_pdf( progress_msg );
  int progress_val = progress_msg->value(); 
  progress_msg->setValue( ++progress_val );
  qApp->processEvents();

  //here, add info on user interactions at 3.IMPORT && 4.EDIT:
  if ( miscMask_edited. ShowMiscParts[ "User Input" ] ) 
    assemble_user_inputs_html( );

  //here, add Run Details based on timestamp info (OR timestapms of IP+RI?)
  if ( miscMask_edited. ShowMiscParts[ "Run Details" ] ) 
    assemble_run_details_html( ) ;

  //add trailing </body>\n</html> to part 1
  html_assembled += "</body>\n</html>";
  
  progress_msg->setValue( progress_msg->maximum() );
  progress_msg->close();
  qApp->processEvents();
  

  //Part 2

  //Get proper filename
  QStringList fileNameList;
  fileNameList. clear();
  if ( FileName.contains(",") && FileName.contains("IP") && FileName.contains("RI") )
    fileNameList  = FileName.split(",");
  else
    fileNameList << FileName;
  
  
  if ( auto_mode )
    {
      for ( int i=0; i<Array_of_triples.size(); ++i )
	{
	  currentTripleName = Array_of_triples[i];
	      
	  //here should be cycle over triple's models ( 2DSA-IT, 2DSA-MC etc..)
	  QStringList models_to_do = Triple_to_Models[ currentTripleName ];
	  
	  for ( int j = 0; j < models_to_do.size(); ++j )
	    {
	      simulate_triple ( currentTripleName, models_to_do[ j ] );

	      //Pseudo3D Distr.
	      plot_pseudo3D( currentTripleName, models_to_do[ j ]);

	      //Individual Combo plots
	      process_combined_plots_individual ( currentTripleName, models_to_do[ j ] );
	    }
	}

      //Combined Plots
      for ( int i=0; i<fileNameList.size(); ++i )
	process_combined_plots( fileNameList[i] );

      //Replicas' averages
      assemble_replicate_av_integration_html();
      
    }
  else
    { //Will be modified for stand-alone GMP Reporter based on edited tree JSON
      for ( int i=0; i<Array_of_triples.size(); ++i )
	{
	  currentTripleName = Array_of_triples[i];
	  
	  //here should be cycle over triple's models ( 2DSA-IT, 2DSA-MC etc..)
	  QStringList models_to_do = Triple_to_Models[ currentTripleName ];
	  
	  for ( int j = 0; j < models_to_do.size(); ++j )
	    {
	      QString triplename_alt = currentTripleName;
	      triplename_alt.replace(".","");

	      //'S' data
	      if ( dataSource . contains("DiskAUC:Absorbance") &&  simulatedData )
		triplename_alt = triplename_alt. replace( "S", "A");

	      qDebug() << "Triple / Model " <<  triplename_alt << " / " <<  models_to_do[ j ] << "has items ? "
		       << perChanMask_edited. has_tripleModel_items     [ triplename_alt ][ models_to_do[ j ] ]
		       << perChanMask_edited. has_tripleModelPlot_items [ triplename_alt ][ models_to_do[ j ] ];
	      
	      if ( perChanMask_edited. has_tripleModel_items     [ triplename_alt ][ models_to_do[ j ] ] ||
		   perChanMask_edited. has_tripleModelPlot_items [ triplename_alt ][ models_to_do[ j ] ] ||
		   perChanMask_edited. has_tripleModelIndCombo_items[ triplename_alt ][ models_to_do[ j ] ] ) 
		{
		  simulate_triple ( currentTripleName, models_to_do[ j ] );

		  //Pseudo3D Distr.
		  plot_pseudo3D( currentTripleName, models_to_do[ j ]);

		  //Individual Combo plots
		  qDebug() << "INDCOMBO, perChanMask_edited. has_tripleModelIndCombo_items[ triplename_alt ][ models_to_do[ j ] ] -- "
			   << triplename_alt << models_to_do[ j ]
			   << perChanMask_edited. has_tripleModelIndCombo_items[ triplename_alt ][ models_to_do[ j ] ];
		  process_combined_plots_individual ( currentTripleName, models_to_do[ j ] );
		}
	    }
	}

      //Combined Plots
      if ( combPlotsMask_edited. has_combo_plots )
	{
	  for ( int i=0; i<fileNameList.size(); ++i )
	    process_combined_plots( fileNameList[i] );
	}

      //Replicas' averages
      if ( miscMask_edited. ShowMiscParts[ "Replicate Groups Averaging" ] ) 
	assemble_replicate_av_integration_html();
      
    }
  //End of Part 2

  //Create .PDF file && write to Db:
  write_pdf_report( );
  qApp->processEvents();

  pb_view_report -> setEnabled( true );
  
  if ( auto_mode )
    {

      pb_view_report_auto->setVisible( true );

      /******* NOTES *******************************************
	       
	       1. For assigned oper/rev/appr. cases (default): 
	          -- inform user that they are assigned && those assignees will re-attach later;
		  -- do NOT let to proceed to 7. e-Signatures
		  -- still enable to View GMP Report
		  -- update autoflow's status to 'E-SIGNATURE'
      ************************************************************/

      //Update autoflow status to 'E-SIGNATURES':
     

      //Compose "{to_sign:["","",...]}" out of operatorListJson, reviewersListJson, approversListJson
      //&& update 'eSignStatusJson' of the autoflowGMPReportEsign record with it
      QMap< QString, QString> eSign_details = read_autoflowGMPReportEsign_record( AutoflowID_auto );
      QStringList oper_rev_joinedList;

      QJsonDocument jsonDocOperList = QJsonDocument::fromJson( eSign_details[ "operatorListJson" ] .toUtf8() );
      get_assigned_oper_revs( jsonDocOperList, oper_rev_joinedList );
      
      QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( eSign_details[ "reviewersListJson" ] .toUtf8() );
      get_assigned_oper_revs( jsonDocRevList, oper_rev_joinedList );
      
      QJsonDocument jsonDocApprList  = QJsonDocument::fromJson( eSign_details[ "approversListJson" ] .toUtf8() );
      get_assigned_oper_revs( jsonDocApprList, oper_rev_joinedList );

      qDebug() << "OperRevAppr_list -- " << oper_rev_joinedList;

      //Minimum structure of eSignStatusJson field:
      QString eSignStatusJson = "{\"to_sign\":[";
      for (int i=0; i<oper_rev_joinedList.size(); ++i )
	{
	  eSignStatusJson += "\"" + oper_rev_joinedList[i] + "\",";
	}
      eSignStatusJson. chop(1);
      eSignStatusJson += "]}";
       
      qDebug() << "operRevToSignJsonObject -- "  << eSignStatusJson;


      US_Passwd   pw;
      US_DB2* db = new US_DB2( pw.getPasswd() );
      
      if ( db->lastErrno() != US_DB2::OK )
	{
	  QApplication::restoreOverrideCursor();
	  QMessageBox::information( this,
				    tr( "DB Connection Problem" ),
				    tr( "AutoflowHistory: there was an error connecting to the database:\n" )
				    + db->lastError() );
	  	  return;
	}
      QStringList qry;
            
      //Update autoflow record with 'E-SIGNATURES'
      qry. clear();
      // qry << "update_autoflow_at_report"
      // 	  << runID
      // 	  << optimaName;
      qry << "update_autoflow_at_report"
	  << AutoflowID_auto;
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

      //Update autoflowGMPReportEsign record with eSignStatusJson
      qry. clear();
      qry << "update_gmp_review_record_by_esigner"
	  << eSign_details[ "ID" ]
	  << AutoflowID_auto
	  << eSignStatusJson
	  << "NO";
      
      qDebug() << "Update \"to_sign\": qry -- " << qry;
      db->query( qry );

      
      /************************************************************************/
      //copy autoflow record to autoflowHistory table:
      //INSERT INTO autoflowHistory SELECT * FROM autoflow WHERE ID=${ID}//
      
      qry. clear();
      qry << "new_autoflow_history_record" << AutoflowID_auto;
      qDebug() << "Query for autoflowHistory -- " << qry;
      db->query( qry );

      //delete autoflow record
      qry.clear();
      qry << "delete_autoflow_record_by_id" << AutoflowID_auto;
      db->statusQuery( qry );

      // //Also delete record from autoflowStages table:           //DO NOT DELETE autoflowStages yet - req. by eSigning process!!
      // qry.clear();
      // qry << "delete_autoflow_stages_record" << AutoflowID_auto;
      // db->statusQuery( qry );
      // //END of copy to History, deletion of primary autoflow record
      /***************************************************************************/

      //Inform user of the PDF location
      QMessageBox msgBox_a;
      msgBox_a.setText(tr("Report PDF Ready!"));
      msgBox_a.setInformativeText(tr( "Report PDF was saved at: \n%1\n\n"
				      "When this dialog is closed, the report can be re-opened by clicking \'View Generated Report\' button at the bottom."
				      "\n\n"
				      "Reviwer(s) of the current GMP report will be notified.")
				  .arg( filePath ) );
				    
      msgBox_a.setWindowTitle(tr("Report Generation Complete"));
      QPushButton *Open      = msgBox_a.addButton(tr("View Report"), QMessageBox::YesRole);
      //QPushButton *Cancel  = msgBox_a.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
	      
      msgBox_a.setIcon(QMessageBox::Information);
      msgBox_a.exec();
      
      if (msgBox_a.clickedButton() == Open)
	{
	  view_report();
	}
      
    }
  else
    {
      te_fpath_info  -> setText( filePath );
      
      //Inform user of the PDF location
      QMessageBox msgBox;
      msgBox.setText(tr("Report PDF Ready!"));
      msgBox.setInformativeText(tr( "Report PDF was saved at: \n%1\n\n"
				    "When this dialog is closed, the report can be re-opened by clicking \'View Generated Report\' button on the left.")
				.arg( filePath ) );
				    
      msgBox.setWindowTitle(tr("Report Generation Complete"));
      QPushButton *Open      = msgBox.addButton(tr("View Report"), QMessageBox::YesRole);
      //QPushButton *Cancel  = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
	      
      msgBox.setIcon(QMessageBox::Information);
      msgBox.exec();
      
      if (msgBox.clickedButton() == Open)
	{
	  view_report();
	}
    }
  
}


//read eSign GMP record for assigned oper(s) && rev(s) && status
QMap< QString, QString> US_ReporterGMP::read_autoflowGMPReportEsign_record( QString aID)
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
	}
    }
  else
    {
      //No record, so no oper/revs assigned!
      qDebug() << "No e-Sign GMP record exists!!";
      eSign_record. clear();
    }


  //test
  delete db;
  
  return eSign_record;
}

//form a string of opers/revs out of jsonDoc
void US_ReporterGMP::get_assigned_oper_revs( QJsonDocument jsonDoc, QStringList& roa_list )
{
  if ( !jsonDoc. isArray() )
    {
      qDebug() << "jsonDoc not a JSON, and/or not an JSON Array!";
      return;
    }
  
  QJsonArray jsonDoc_array  = jsonDoc.array();
  for (int i = 0; i < jsonDoc_array.size(); ++i )
    roa_list << jsonDoc_array[i].toString();
}


// Scan database for models associated with run sets
QStringList  US_ReporterGMP::scan_dbase_models( QStringList runIDs )
{
   QStringList   wDescrs;
  
   US_Passwd   pw;
   US_DB2      db( pw.getPasswd() );

   if ( db.lastErrno() != US_DB2::OK )
   {
      QMessageBox::information( this,
         tr( "DB Connection Problem" ),
         tr( "There was an error connecting to the database:\n" )
         + db.lastError() );
      return wDescrs;
   }

   QStringList        mmIDs;        // Model modelIDs
   QStringList        mmGUIDs;      // Model modelGUIDs
   QStringList        meIDs;        // Model editIDs;
   QVector< QString > mmDescs;      // Model descriptions
   QMap< QString, QString > ddmap;  // editID,dataDescr map

   QStringList query;
   int          ntmodel = 0;
   int          nmodel  = 0;
   bool         no_una  = ! runIDs.contains( "UNASSIGNED" );
DbgLv(1) << "ScMd: UNASSGN: no-UNASSIGNED" << no_una;

   // First accumulate model information for UNASSIGNED models
   query.clear();
   query << "get_model_desc_by_editID" << QString::number( invID ) << "1";
   db.query( query );

   while ( db.next() )
   {
      QString mdlid    = db.value( 0 ).toString();
      QString mdlGid   = db.value( 1 ).toString();
      QString mdesc    = db.value( 2 ).toString();
      QString edtid    = db.value( 6 ).toString();
      int     kk       = mdesc.lastIndexOf( ".model" );
      mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
      QString mrunid   = mdesc.section( ".", -3, -3 );
DbgLv(1) << "ScMd: UNASSGN: mid,eid,mdesc,mrun" << mdlid << edtid << mdesc << mrunid;
      if ( ! runIDs.contains( mrunid )  &&  no_una )
         continue;       // Skip any without a desired run prefix
DbgLv(1) << "ScMd: UNASSGN:  ++USED++";
      mmIDs   << mdlid;
      mmGUIDs << mdlGid;
      meIDs   << edtid;
      mmDescs << mdesc;
      nmodel++;
   }
DbgLv(1) << "ScMd: runid UNASGN editId 1   nmodel" << nmodel;

QTime timer;
timer.start();
   // Accumulate model information for runs present
   for ( int rr = 0; rr < runIDs.count(); rr++ )
   {
      QString runid    = runIDs[ rr ];
      query.clear();
      query << "get_model_desc_by_runID" << QString::number( invID ) << runid;
      db.query( query );

      while( db.next() )
      {
         QString mdlid    = db.value( 0 ).toString();
         QString mdlGid   = db.value( 1 ).toString();
         QString mdesc    = db.value( 2 ).toString();
         QString edtid    = db.value( 6 ).toString();
         int     kk       = mdesc.lastIndexOf( ".model" );
         mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         mmIDs   << mdlid;
         mmGUIDs << mdlGid;
         meIDs   << edtid;
         mmDescs << mdesc;
         nmodel++;
      }
DbgLv(1) << "ScMd: runid" << runid << "nmodel" << nmodel;

      // Repeat the scan for "global-<run>%" variation
      QString grunid   = "Global-" + runid + "%";
      query.clear();
      query << "get_model_desc_by_runID" << QString::number( invID ) << grunid;
      db.query( query );

      while( db.next() )
      {
         QString mdlid    = db.value( 0 ).toString();
         QString mdlGid   = db.value( 1 ).toString();
         QString mdesc    = db.value( 2 ).toString();
         QString edtid    = db.value( 6 ).toString();
         int     kk       = mdesc.lastIndexOf( ".model" );
         mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
         mmIDs   << mdlid;
         mmGUIDs << mdlGid;
         meIDs   << edtid;
         mmDescs << mdesc;
         nmodel++;
      }
DbgLv(1) << "ScMd:  runid" << runid << "nmodel" << nmodel;
   }
DbgLv(1) << "ScMd:scan time(1)" << timer.elapsed();

   // Accumulate model information for "Global-" UNASSIGNED models
   query.clear();
   query << "get_model_desc_by_runID" << QString::number( invID ) << "Global-%";
   db.query( query );

   while ( db.next() )
   {
      QString mdlid    = db.value( 0 ).toString();
      if ( mmIDs.contains( mdlid ) )
         continue;

      QString mdlGid   = db.value( 1 ).toString();
      QString mdesc    = db.value( 2 ).toString();
      int     kk       = mdesc.lastIndexOf( ".model" );
      mdesc            = ( kk < 1 ) ? mdesc : mdesc.left( kk );
      mmIDs   << mdlid;
      mmGUIDs << mdlGid;
      meIDs   << "1";
      mmDescs << mdesc;
      nmodel++;
   }
DbgLv(1) << "ScMd: runid glob% UNASGN editId 1   nmodel" << nmodel;

   query.clear();
   query << "count_models" << QString::number( invID );
   ntmodel  = db.functionQuery( query );
DbgLv(1) << "ScMd: ntmodel" << ntmodel << "nmodel" << nmodel;
DbgLv(1) << "ScMd:scan time(2)" << timer.elapsed();
int m=nmodel-1;
if ( m>1 ) {
DbgLv(1) << "ScMd: 0: id,gid,eid,desc" << mmIDs[0] << mmGUIDs[0] << meIDs[0] << mmDescs[0];
DbgLv(1) << "ScMd: m: id,gid,eid,desc" << mmIDs[m] << mmGUIDs[m] << meIDs[m] << mmDescs[m]; }

   // Scan all saved models from the end back, saving any
   //   cell description by edit ID
   for ( int mm = nmodel - 1; mm >=0; mm-- )
   {
      QString medtid   = meIDs[ mm ];

      if ( ddmap.contains( medtid ) )  continue;   // Skip if already mapped

      // Not yet mapped, so find any cell description in the model XML
      QString mdlid    = mmIDs[ mm ];
      query.clear();
      query << "get_model_info" << mdlid;
      db.query( query );
      db.next();
      QString mxml     = db.value( 2 ).toString();
      int     kk       = mxml.indexOf( "dataDescrip=" );
DbgLv(1) << "ScMd: mm kk medtid" << mm << kk << medtid;

      if ( kk > 0 )
      {  // We have found the data description, so map it
         QString ddesc    = mxml.mid( kk + 13 );
         kk               = ddesc.indexOf( "\"" );
         ddesc            = ddesc.left( kk );
         ddmap[ medtid ]  = ddesc;
      }
   }
DbgLv(1) << "ScMd:scan time(3)" << timer.elapsed();

   // Do one more pass through all the models, completing
   //  the model descriptions
   for ( int mm = 0; mm < nmodel; mm++ )
   {
      QString mID    = mmIDs  [ mm ];
      QString mGUID  = mmGUIDs[ mm ];
      QString mdesc  = mmDescs[ mm ];
      QString meID   = meIDs  [ mm ];
      QString ddesc  = ddmap.contains( meID ) ? ddmap[ meID ] : "";
      QString runid  = mdesc.section( ".", 0, -3 );
      if ( meID == "1" )
              runid  = "UNASSIGNED";
      QString odesc  = runid + "\t" + mGUID + "\t" + mdesc + "\t" + ddesc;
      wDescrs << odesc;
      DbgLv(1) << "ScMd:  mm meID" << mm << meID << "ddesc" << ddesc;
   }

   DbgLv(1) << "ScMd:scan time(9)" << timer.elapsed();

   return wDescrs;
}

//simulate triple 
void US_ReporterGMP::simulate_triple( const QString triplesname, QString stage_model )
{
  // Show msg while data downloaded and simulated
  progress_msg = new QProgressDialog (QString("Downloading data and models for triple %1...").arg( triplesname ), QString(), 0, 5, this);
  progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setWindowModality(Qt::WindowModal);
  progress_msg->setWindowTitle(tr("Generating Report: Part 2..."));
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();
  
  speed_steps  .clear();
  edata = NULL;
  rdata = NULL;
  //sdata = NULL;
  eID_global  = 0;
  eID_updated = "";
  
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
  resplotd   = 0;       //<--TEMP
  ti_noise.count = 0;
  ri_noise.count = 0;


  //QString stage_n   = QString("2DSA-IT");
  QString stage_n   = stage_model;
  QString triple_n  = triplesname;
  //stage_n : '2DSA-IT'
  //triple_n: '2.A.255'

  qDebug() << "In SHOW OVERLAY: triple_stage / triple_name: " << stage_n << " / " << triple_n;

  tripleInfo = ": " + triple_n + " (" + stage_n + ")";

  //Parse filename
  FileName_parsed = get_filename( triple_n );
  qDebug() << "In show_overlay(): FileName_parsed: " << FileName_parsed;
  
  //LoadData
  triple_info_map.clear();
  triple_info_map[ "triple_name" ]     = triple_n;
  triple_info_map[ "stage_name" ]      = stage_n;
  triple_info_map[ "invID" ]           = QString::number(invID);
  triple_info_map[ "filename" ]        = FileName_parsed;

  dataLoaded = false;
  buffLoaded = false;
  haveSim    = false;
  
  loadData( triple_info_map );
  progress_msg->setValue( 1 );
  
  triple_info_map[ "eID" ]         = QString::number( eID_global );
  triple_info_map[ "eID_updated" ] = eID_updated;
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


  //test
  delete dbP;

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
  
  // triple_info_map[ "stage_name" ] = QString("2DSA-IT");
  // if ( !loadModel( triple_info_map  ) && !model_exists )
  //   {
  //     triple_info_map[ "stage_name" ] = QString("2DSA-FM");
  //     if ( !loadModel( triple_info_map  ) && !model_exists )
  // 	{
  // 	  triple_info_map[ "stage_name" ] = QString("2DSA");
  // 	  if ( !loadModel( triple_info_map  ) && !model_exists )
  // 	    {
  // 	      qDebug() << "In loadModel(): No models (2DSA-IT, 2DSA-FM, 2DSA) found for triple -- " << triple_info_map[ "triple_name" ];

  // 	      QMessageBox::critical( this, tr( "No Model Found" ),
  // 				     QString( tr( "In loadModel(): No models (2DSA-IT, 2DSA-FM, 2DSA) found for triple %1" ))
  // 				     .arg( triple_info_map[ "triple_name" ] ) );
	      
  // 	      return;
  // 	    }
  // 	}
  //   }

  progress_msg->setValue( 5 );
  qApp->processEvents();

  //Simulate Model
  simulateModel( triple_info_map );

  
  qDebug() << "Closing sim_msg-- ";
  //msg_sim->accept();
  progress_msg->close();
  qApp->processEvents();

  /*
  // Show plot
  resplotd = new US_ResidPlotFem( this, true );
  //resplotd->setWindowFlags( Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint);
  resplotd->setWindowModality(Qt::ApplicationModal);
  resplotd->show();
  */
}

// public function to get pointer to edit data
US_DataIO::EditedData*      US_ReporterGMP::rg_editdata() { return edata;     }

// public function to get pointer to list of excluded scans
QList< int >*               US_ReporterGMP::rg_excllist() { return &excludedScans;}

// public function to get pointer to sim data
US_DataIO::RawData*         US_ReporterGMP::rg_simdata()  { return sdata;     }

// public function to get pointer to load model
US_Model*                   US_ReporterGMP::rg_model()    { return &model;    }

// public function to get pointer to TI noise
US_Noise*                   US_ReporterGMP::rg_ti_noise() { return &ti_noise; }

// public function to get pointer to RI noise
US_Noise*                   US_ReporterGMP::rg_ri_noise() { return &ri_noise; }

// public function to get pointer to resid bitmap diag
QPointer< US_ResidsBitmap > US_ReporterGMP::rg_resbmap()  { return rbmapd;    }

QString  US_ReporterGMP::rg_tripleInfo()  { return tripleInfo;    }


//Load rawData/editedData
bool US_ReporterGMP::loadData( QMap < QString, QString > & triple_information )
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

  qDebug() << "In load Data: triple, eID (from modelsLink) -- "
	   << triple_information[ "triple_name" ]
	   << Triple_to_ModelsDesc[ triple_information[ "triple_name" ] ] [ "eID" ] ;
  
  int rID=0;
  QString rfilename;
  //int eID=0;
  int eID = Triple_to_ModelsDesc[ triple_information[ "triple_name" ] ] [ "eID" ].toInt();

  QString efilename;
  
  //get EditedData filename && editedDataID for current triple, then infer rawDataID 
  QStringList query;
  query << "get_editedDataFilenamesIDs_forReport" << triple_information["filename"] << QString::number( eID ) ;
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
      rID         = rawDataID;
      efilename   = filename;
      
      QDateTime date               = db->value( 3 ).toDateTime();
      eID_updated                  = db->value( 3 ).toString();
      
      // QDateTime now = QDateTime::currentDateTime();
               
      // if ( filename.contains( triple_name_actual ) ) 
      // 	{
      // 	  int time_to_now = date.secsTo(now);
      // 	  if ( time_to_now < latest_update_time )
      // 	    {
      // 	      latest_update_time = time_to_now;
      // 	      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

      // 	      rID         = rawDataID;
      // 	      eID         = editedDataID;
      // 	      efilename   = filename;
      // 	      eID_updated = db->value( 3 ).toString();
      // 	    }
      // 	}
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

  //test:
  delete db;
    
  return true;
}

//Load rawData/editedData
bool US_ReporterGMP::loadModel( QMap < QString, QString > & triple_information )
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

  // //first, get ModelIDs corresponding to editedDataID AND triple_stage && select latest one
  // QStringList query;
  // query << "get_modelDescsIDs" << triple_information[ "eID" ];
  // db->query( query );
  
  // qDebug() << "In loadModel() Query: " << query;
  
  // int latest_update_time = 1e100;
  // int mID=0;

  // model_exists = false;
  
  // while ( db->next() )
  //   {
  //     QString  description         = db->value( 0 ).toString();
  //     int      modelID             = db->value( 1 ).toInt();
  //     //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
  //     QDateTime date               = db->value( 2 ).toDateTime();

  //     QDateTime now = QDateTime::currentDateTime();
      
  //     if ( description.contains( triple_information[ "stage_name" ] ) ) 
  // 	{
  // 	  //if contains, it matches & the model exists (e.g. 2DSA-IT); now find the latest one
  // 	  model_exists = true;
	  
  // 	  if ( triple_information[ "stage_name" ] == "2DSA" )
  // 	    {
  // 	      if ( !description.contains("-FM_") && !description.contains("-IT_") && !description.contains("-MC_") && !description.contains("_mcN") )
  // 		{
  // 		  int time_to_now = date.secsTo(now);
  // 		  if ( time_to_now < latest_update_time )
  // 		    {
  // 		      latest_update_time = time_to_now;
  // 		      //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

  // 		      qDebug() << "Model 2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  		      
  // 		      mID       = modelID;
  // 		    }
  // 		}
  // 	    }
  // 	  else
  // 	    {
  // 	      int time_to_now = date.secsTo(now);
  // 	      if ( time_to_now < latest_update_time )
  // 		{
  // 		  latest_update_time = time_to_now;
  // 		  //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);
		  
  // 		  qDebug() << "Model NON-2DSA: ID, desc, timetonow -- " << modelID << description << time_to_now;
		  
  // 		  mID       = modelID;
  // 		}
  // 	    }
  // 	}
  //   }

  // if ( ! model_exists )
  //   {
  //     // QMessageBox::critical( this, tr( "Model Does Not Exists!" ),
  //     // 			     QString (tr( "Triple %1 does not have  %2 model !" ))
  //     // 			     .arg( triple_information[ "triple_name" ] )
  //     // 			     .arg( triple_information[ "stage_name" ] ) );

  //     progress_msg->setLabelText( QString("Model %1 is NOT found for triple %2.\n Trying other models...")
  // 				  .arg( triple_information[ "stage_name" ] )
  // 				  .arg( triple_information[ "triple_name" ] ) );
  //     progress_msg->setValue( 4 );
  //     qApp->processEvents();
      
  //     return false;
  //   }


  int mID = Triple_to_ModelsDesc[ triple_information[ "triple_name" ] ] [ triple_information[ "stage_name" ] ].toInt();
  
  int  rc      = 0;
  qDebug() << "ModelID to retrieve for triple, stage: -- "
	   << triple_information[ "triple_name" ]
	   << triple_information[ "stage_name" ]
	   << mID;
  
  rc   = model.load( QString::number( mID ), db );
  qDebug() << "LdM:  model load rc" << rc;
  qApp->processEvents();

  //EditDataUpdated for the model:
  model.editDataUpdated = triple_information[ "eID_updated" ];

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

  //test:
  delete db;
  
  loadNoises( triple_information );
  
  return true;
}

//Load Noises
bool US_ReporterGMP::loadNoises( QMap < QString, QString > & triple_information )
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

  //test:
  delete db;
  return true;
}

//Load Noises when absent for the model loaded (like -MC models)
void US_ReporterGMP::loadNoises_whenAbsent( )
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
int US_ReporterGMP::count_noise_auto( US_DataIO::EditedData* edata,
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
int US_ReporterGMP::noises_in_model_auto( QString mGUID, QStringList& nGUIDs )
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
int US_ReporterGMP::models_in_edit_auto( QString eGUID, QStringList& mGUIDs )
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
int US_ReporterGMP::id_list_db_auto( QString daEditGUID )
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
void US_ReporterGMP::simulateModel( QMap < QString, QString> & tripleInfo )
{
  progress_msg->setLabelText( QString("Simulating model %1 for triple %2...")
			      .arg( tripleInfo[ "stage_name" ])
			      .arg( tripleInfo[ "triple_name" ] ) );
  progress_msg->setValue( 0 );
  qApp->processEvents();
  
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


  //test
  delete dbP;
  
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
  //if ( nthread < 2 )
  if ( nthread < 999 )
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
      
      show_results( triple_info_map );
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
void US_ReporterGMP::update_progress( int icomp )
{
  qDebug () << "Updating progress single thread, icomp  -- " << icomp;
  
  progress_msg->setValue( icomp );
}


// Show simulation and residual when the simulation is complete
void US_ReporterGMP::show_results( QMap <QString, QString> & tripleInfo )
{
   progress_msg->setValue( progress_msg->maximum() );
   qApp->processEvents();
   
   haveSim     = true;
   calc_residuals();             // calculate residuals

   //distrib_plot_resids();        // plot residuals

   // data_plot();                  // re-plot data+simulation

   // if ( rbmapd != 0 )
   // {
   //    bmd_pos  = rbmapd->pos();
   //    rbmapd->close();
   // }

   assemble_distrib_html( tripleInfo );

   //assemble_integration_results_html( );
   
   plotres( tripleInfo ); // <------- save plots into files locally

   //plot_pseudo3D( tripleInfo );       // <--- psedo3d per triple/model

   
   
   QApplication::restoreOverrideCursor();
}

//Check if model exists for particular stage (2DSA-IT, or 2DSA-MC...) by its modelGUID (across all triples) 
bool US_ReporterGMP::modelGuidExistsForStage( QString model, QString mguid)
{
  bool isModelGuid = false;

  for ( int i=0; i < Array_of_tripleNames.size(); ++ i )
    {
      QMap< QString, QString > tmapguid =  Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ];

      if ( tmapguid[ model ] == mguid )
	{
	  qDebug() << "For stage: " << model << ", there is modelGuid: " << mguid << " (" <<  Array_of_tripleNames[ i ] << ")";
	  
	  isModelGuid = true;
	  break;
	}
    }
  
  return isModelGuid;
}


//[IND] Check if model exists for particular stage (2DSA-IT, or 2DSA-MC...) by its modelGUID (for specified triple) 
bool US_ReporterGMP::modelGuidExistsForStage_ind( QString triple_n, QString model, QString mguid)
{
  bool isModelGuid = false;

  for ( int i=0; i < Array_of_tripleNames.size(); ++ i )
    {
      QString c_triple_n =  Array_of_tripleNames[ i ];
      c_triple_n. replace(".","");

      qDebug() << "IN modelGuidExistsForStage_ind(): triple_n, c_triple_n, model  -- "
	       << triple_n << c_triple_n << model;
      qDebug() << "IN modelGuidExistsForStage_ind(): mguid, Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ][ model ] -- "
	       << mguid << Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ][ model ];
      
      if ( c_triple_n == triple_n )
	{
	  QMap< QString, QString > tmapguid =  Triple_to_ModelsDescGuid[ Array_of_tripleNames[ i ] ];
	  
	  if ( tmapguid[ model ] == mguid )
	    {
	      qDebug() << "For triple, stage: " << triple_n << model << ", there is modelGuid: " << mguid << " (" <<  Array_of_tripleNames[ i ] << ")";
	      
	      isModelGuid = true;
	      break;
	    }
	}
    }
  
  return isModelGuid;
}



//Individual Combined Plots
void US_ReporterGMP::process_combined_plots_individual ( QString triplesname_p, QString stage_model )
{
  QString triplesname_passed = triplesname_p;
  qDebug() << "[in process_combined_plots_individual()]: triplesname_passed -- " << triplesname_passed;
  QString filename_passed = get_filename( triplesname_p );
  qDebug() << "[in process_combined_plots_individual()]: filename_passed -- " << filename_passed;
  QString triplesname = triplesname_p.replace(".","");
  qDebug() << "[in process_combined_plots_individual()]: triplesname -- " << triplesname;

  sdiag_combplot = new US_DDistr_Combine( "REPORT" );
  QStringList runIDs_single;
    
  runIDs_single << filename_passed;
  QStringList aDescrs = scan_dbase_models( runIDs_single );
  //QStringList modelDescModified = sdiag_combplot->load_auto( runIDs_single, aDescrs );
  QList < QStringList > modelDescModifiedList = sdiag_combplot->load_auto( runIDs_single, aDescrs );
  QStringList modelDescModified     = modelDescModifiedList[ 0 ];
  QStringList modelDescModifiedGuid = modelDescModifiedList[ 1 ];
  
  qDebug() << "[IND] ComboPlots generation: modelDescModified -- "     << modelDescModified;
  qDebug() << "[IND] ComboPlots generation: modelDescModifiedGuid -- " << modelDescModifiedGuid;

  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  //mkdir( US_Settings::reportDir(), filename_passed );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );
  QString basename = dirName + "/" + filename_passed + ".";


  QStringList CombPlotsFileNames;
  QStringList plottedIDs_s, plottedIDs_other_type;

  QMap< QStringList, QList< QColor > > plotted_ids_colors_map_s_type;
  
  //Choose model from modelDescModifiedGuid matching triplesname && stage_model:
  //bool isModel = false;
  //QString imgComb01File = basename + "Ind_combined" + "." + triplesname + "." + stage_model + ".s20" + svgext;
  
  for ( int ii = 0; ii < modelDescModified.size(); ii++ )  
    {
      QString triplesname_mod  = triplesname;
      QString triplesname_chann;                   //Should be "1A:UV/vis." OR "1A:Interf."

      if ( triplesname.contains("Interference") )
	{
	  triplesname_mod   = triplesname_mod.replace( "Interference" , "660");
	  triplesname_chann = triplesname_passed.split(".")[0] + triplesname_passed.split(".")[1] + ":Interf.";
	}
      else
	triplesname_chann = triplesname_passed.split(".")[0] + triplesname_passed.split(".")[1] + ":UV/vis.";

      qDebug() << "INDCOMBO_1: " << modelDescModified[ ii ];
      qDebug() << "INDCOMBO_2: " << triplesname << stage_model << triplesname_chann;
          
      //fiter by type|model
      if ( modelDescModified[ ii ].contains( triplesname_mod ) &&
	   modelDescModified[ ii ].contains( stage_model ) &&
	   modelGuidExistsForStage_ind( triplesname, stage_model, modelDescModifiedGuid[ ii ] ) )
	{
	  qDebug()  << "INDCOMBO_3: YES ";

	  //'S' data
	  if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
	    {
	      triplesname       = triplesname. replace( "S" , "A");
	      triplesname_chann = triplesname_chann. replace( "S" , "A");
	    }
	  
	  //compose map of [{"s_ranges","k_ranges"}, etc] from cAP2 & cAPp (for given channel & model!!!)
	  QMap< QString, QStringList > sim_ranges = find_sim_ranges( triplesname_chann, stage_model );  

	  QString t_m = "s," + stage_model;
	  QMap < QString, QString > c_params = comboPlotsMap[ t_m ];
	  //ALEXEY: here it plots s20 combPlot (xtype == 0) -- Need to do first thing!!!
	  plotted_ids_colors_map_s_type = sdiag_combplot-> model_select_auto ( modelDescModified[ ii ], c_params ); 
	  
	  // Check if to plot individual Combined distributons:
	  // {s,D,f/f0,MW,Radius} {2DSA-IT,2DSA-MC,PCSA,raw} {p_key: {s[3.2:3.7], D[11:15], etc.} }
	  // MaskStr.ShowTripleTypeModelRangeIndividualCombo[ t_name ][ s_name ][ p_key ] = feature_indCombo_value; // 1/0
	  QMap <QString, QStringList> ind_compoplots_type_ranges;

	  qDebug() << "BEFORE INDCOMBO_4: triplesname, stage_model -- " <<  triplesname <<  stage_model;
	  
	  QMap < QString, QString > ind_comboplots = perChanMask_edited.ShowTripleTypeModelRangeIndividualCombo[ triplesname ][ stage_model ];
	  QMap<QString, QString >::iterator i_cp;
	  for ( i_cp = ind_comboplots.begin(); i_cp != ind_comboplots.end(); ++i_cp )
	    {
	      QString type_range = i_cp.key();
	      
	      QString type  = type_range.split("[")[0];
	      QString range = type_range.split("[")[1].split("]")[0];

	      qDebug() << "INDCOMBO_4: map's parms -- " << type_range << i_cp.value().toInt()
		       << type << range;
	      
	      if ( i_cp.value().toInt() )
		{
		  ind_compoplots_type_ranges[ type ] << range;
		}
	    }

	  if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
	    triplesname       = triplesname. replace( "A" , "S");
	  
	  //plot different types {s,D,MW...} types:  0: s20; 1: MW; 2: D; 3: f/f0
	  QMap<QString, QStringList >::iterator i_cpt;
	  for ( i_cpt = ind_compoplots_type_ranges.begin(); i_cpt != ind_compoplots_type_ranges.end(); ++i_cpt )
	    {
	      QString     type    = i_cpt.key();
	      QStringList ranges  = i_cpt.value();
	      
	      QMap < QString, QString > c_parms;
	      QString t_m;
	      QString imgComb02File = basename + "Ind_combined" + "." + triplesname + "." + stage_model;

	      qDebug() << "in the ind_compoplots_type_ranges QMap: type, ranges --  " << type << ranges;
	      
	      //type: 0:s20
	      if ( type == "s" )
		{
		  imgComb02File += ".s20" + svgext;
		  
		  t_m = "s," + stage_model;
		  c_parms = comboPlotsMap[ t_m ];
		  //put ranges into c_parms:
		  c_parms[ "Ranges" ] = ranges.join(",");

		  qDebug() << "s-type: sim_ranges.keys(), sim_ranges[\"s_ranges\"] -- "
			   << sim_ranges.keys()
			   << sim_ranges["s_ranges"];
		  if ( sim_ranges. contains("s_ranges") )
		    c_parms[ "s_ranges" ] = sim_ranges["s_ranges"].join(",");
		  	    
		  
		  //qDebug() << "over models: c_params -- " << c_params;
		  
		  //ALEXEY: here it plots s20 combPlot (xtype == 0)	  
		  plotted_ids_colors_map_s_type = sdiag_combplot-> changedPlotX_auto( 0, c_parms );
		 		  
		  write_plot( imgComb02File, sdiag_combplot->rp_data_plot1() );                //<-- rp_data_plot1() gives combined plot
		  imgComb02File.replace( svgext, pngext ); 
		  CombPlotsFileNames << imgComb02File;
		  
		  CombPlotsParmsMap       [ imgComb02File ] = plotted_ids_colors_map_s_type. firstKey();
		  CombPlotsParmsMap_Colors[ imgComb02File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
		}
	      //type: 1:MW
	      else if ( type == "MW" )
		{
		  imgComb02File += ".MW" + svgext;
		  
		  t_m = "MW," + stage_model;
		  c_parms = comboPlotsMap[ t_m ];
		  //put ranges into c_parms:
		  c_parms[ "Ranges" ] = ranges.join(",");
		  
		  plotted_ids_colors_map_s_type = sdiag_combplot-> changedPlotX_auto( 1, c_parms );
		  
		  write_plot( imgComb02File, sdiag_combplot->rp_data_plot1() );              //<-- rp_data_plot1() gives combined plot
		  imgComb02File.replace( svgext, pngext );
		  CombPlotsFileNames << imgComb02File;
		  
		  CombPlotsParmsMap       [ imgComb02File ] = plotted_ids_colors_map_s_type. firstKey();
		  CombPlotsParmsMap_Colors[ imgComb02File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
		}
	      //type 2:D
	      else if ( type == "D" )
		{
		  imgComb02File += ".D20" + svgext;
		  
		  t_m = "D," + stage_model;
		  c_parms = comboPlotsMap[ t_m ];
		  //put ranges into c_parms:
		  c_parms[ "Ranges" ] = ranges.join(",");
		  
		  plotted_ids_colors_map_s_type = sdiag_combplot-> changedPlotX_auto( 2, c_parms );
		  
		  write_plot( imgComb02File, sdiag_combplot->rp_data_plot1() );              //<-- rp_data_plot1() gives combined plot
		  imgComb02File.replace( svgext, pngext );
		  CombPlotsFileNames << imgComb02File;
		  
		  CombPlotsParmsMap       [ imgComb02File ] = plotted_ids_colors_map_s_type. firstKey();
		  CombPlotsParmsMap_Colors[ imgComb02File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
		}
	      //type 3:f/f0
	      else if ( type == "f/f0" )
		{
		  imgComb02File += ".f_f0" + svgext;
		  
		  t_m = "f/f0," + stage_model;
		  c_parms = comboPlotsMap[ t_m ];
		  //put ranges into c_parms:
		  c_parms[ "Ranges" ] = ranges.join(",");

		  if ( sim_ranges. contains("k_ranges") )
		    c_parms[ "k_ranges" ] = sim_ranges["k_ranges"].join(",");
		  
		  plotted_ids_colors_map_s_type = sdiag_combplot-> changedPlotX_auto( 3, c_parms );
		  
		  write_plot( imgComb02File, sdiag_combplot->rp_data_plot1() );              //<-- rp_data_plot1() gives combined plot
		  imgComb02File.replace( svgext, pngext );
		  CombPlotsFileNames << imgComb02File;
		  
		  CombPlotsParmsMap       [ imgComb02File ] = plotted_ids_colors_map_s_type. firstKey();
		  CombPlotsParmsMap_Colors[ imgComb02File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
		}
	      
	      // reset plot after processign certain type {s,D,MW...}
	      sdiag_combplot->reset_data_plot1();
	    }
	}
    }
  
  //assemble IND combined plots into html
  assemble_plots_html( CombPlotsFileNames, "CombPlots"  );
  qApp->processEvents();
}

//pull s_ranges, k_ranges from AProfile
QMap< QString, QStringList > US_ReporterGMP::find_sim_ranges( QString chann_desc, QString model )
{
  QMap < QString, QStringList > sim_ranges;

  qDebug() << "[in find_sim_ranges()1] -- " << chann_desc << model;
  
  if ( model. contains("2DSA") )
    {
      //2DSA
      for (int i=0; i<cAP2.parms.size(); ++i )
	{
	  QString channame = cAP2.parms[i].channel;
	  qDebug() << "channame -- " << channame;
	  
	  if ( channame. contains( chann_desc ) )
	    {
	      sim_ranges[ "s_ranges" ] << QString::number( cAP2.parms[i].s_min )
				       << QString::number( cAP2.parms[i].s_max );
	      sim_ranges[ "k_ranges" ] << QString::number( cAP2.parms[i].k_min )
				       << QString::number( cAP2.parms[i].k_max );
	      break;
	    }
	}
    }
  else if ( model. contains("PCSA") )
    {
      //PCSA
      for (int i=0; i<cAPp.parms.size(); ++i )
	{
	  QString channame = cAPp.parms[i].channel;
	  if ( channame == chann_desc )
	    {
	      sim_ranges[ "s_ranges" ] << QString::number( cAPp.parms[i].x_min )
				       << QString::number( cAPp.parms[i].x_max );
	      sim_ranges[ "y_ranges" ] << QString::number( cAPp.parms[i].y_min )
				       << QString::number( cAPp.parms[i].y_max );
	      break;
	    }
	}
    }

  return sim_ranges;
}

//Combined Plots
void US_ReporterGMP::process_combined_plots ( QString filename_passed )
{
  sdiag_combplot = new US_DDistr_Combine( "REPORT" );
  QStringList runIDs_single;
    
  runIDs_single << filename_passed;
  QStringList aDescrs = scan_dbase_models( runIDs_single );
  //QStringList modelDescModified = sdiag_combplot->load_auto( runIDs_single, aDescrs );
  QList < QStringList > modelDescModifiedList = sdiag_combplot->load_auto( runIDs_single, aDescrs );
  QStringList modelDescModified     = modelDescModifiedList[ 0 ];
  QStringList modelDescModifiedGuid = modelDescModifiedList[ 1 ];
  
  qDebug() << "ComboPlots generation: modelDescModified -- "     << modelDescModified;
  qDebug() << "ComboPlots generation: modelDescModifiedGuid -- " << modelDescModifiedGuid;

  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  //mkdir( US_Settings::reportDir(), filename_passed );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );
  QString basename = dirName + "/" + filename_passed + ".";
  //QString basename  = US_Settings::reportDir() + "/" + filename_passed + "/" + filename_passed + ".";

  
  //estimate # of combined plots
  int combpl_number = 3*3;
  // Show msg while data downloaded and simulated
  progress_msg = new QProgressDialog (QString("Generating combined plots..."), QString(), 0, combpl_number, this);
  progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setWindowModality(Qt::WindowModal);
  progress_msg->setWindowTitle(tr("Combined Plots"));
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();

  int pr_cp_val = 0;
  
  //go over modelDescModified
  QStringList modelNames;
  modelNames << "2DSA-IT" << "2DSA-MC" << "PCSA";
  QList< int > xtype;
  xtype <<  1 << 2 << 3; //ALEXEY: 0: s20; 1: MW; 2: D; 3: f/f0
                         //Note: xtype==0 (s20) is the default, so iterate later starting from 1... 
  QStringList CombPlotsFileNames;
  QStringList plottedIDs_s, plottedIDs_other_type;

  QMap< QStringList, QList< QColor > > plotted_ids_colors_map_s_type;

  for ( int m = 0; m < modelNames.size(); m++ )  
    {
      bool isModel = false;
      QString imgComb01File = basename + "combined" + "." + modelNames[ m ]  + ".s20" + svgext;

      for ( int ii = 0; ii < modelDescModified.size(); ii++ )  
	{
	  //fiter by type|model
	  if ( modelDescModified[ ii ].contains( modelNames[ m ] ) && modelGuidExistsForStage( modelNames[ m ], modelDescModifiedGuid[ ii ] ) )
	    {
	      isModel = true;

	      //retrieve s,Model combPlot params:
	      QString t_m = "s," + modelNames[ m ];
	      QMap < QString, QString > c_params = comboPlotsMap[ t_m ];
	      //qDebug() << "over models: c_params -- " << c_params;
	      plotted_ids_colors_map_s_type = sdiag_combplot-> model_select_auto ( modelDescModified[ ii ], c_params ); //ALEXEY: here it plots s20 combPlot (xtype == 0)

	    }
	}
      
      ++pr_cp_val;
      progress_msg->setValue( pr_cp_val );
      
      //write plot
      if ( isModel )  //TEMPORARY: will read a type-method combined plot QMap defined at the beginnig
	{
	  //here identify what to show:
	  bool show_combo_s    = (combPlotsMask_edited.ShowCombPlotParts[ "s"    ][ modelNames[ m ] ].toInt()) ? true : false ;
	  bool show_combo_D    = (combPlotsMask_edited.ShowCombPlotParts[ "D"    ][ modelNames[ m ] ].toInt()) ? true : false ;
	  bool show_combo_ff0  = (combPlotsMask_edited.ShowCombPlotParts[ "f/f0" ][ modelNames[ m ] ].toInt()) ? true : false ;
	  bool show_combo_MW   = (combPlotsMask_edited.ShowCombPlotParts[ "MW"   ][ modelNames[ m ] ].toInt()) ? true : false ;

	  qDebug() << "In process ComboPlots: Model: show_combo_s, show_combo_D, show_combo_ff0, show_combo_MW -- "
		   << modelNames[m] << ": " << show_combo_s << show_combo_D << show_combo_ff0 << show_combo_MW ;
	  
	  //here writes a 's'-type IF it's to be included:
	  // QString t_m = "s," + modelNames[ m ];
	  // if ( comboPlotsMapTypes.contains( t_m ) && comboPlotsMapTypes[ t_m ] != 0  )
	  if ( show_combo_s ) 
	    {
	      qDebug() << "PLOTTED_IDs_S_type -- " << plottedIDs_s;
	      write_plot( imgComb01File, sdiag_combplot->rp_data_plot1() );                //<-- rp_data_plot1() gives combined plot
	      imgComb01File.replace( svgext, pngext ); 
	      CombPlotsFileNames << imgComb01File;

	      //CombPlotsParmsMap[ imgComb01File ] = plottedIDs_s;
	      
	      CombPlotsParmsMap       [ imgComb01File ] = plotted_ids_colors_map_s_type. firstKey();
	      CombPlotsParmsMap_Colors[ imgComb01File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
	    }

	  ++pr_cp_val;
	  progress_msg->setValue( pr_cp_val );
	  
	  //Now that we have s20 plotted, plot other types [ MW, D, f/f0 ]
	  for ( int xt= 0; xt < xtype.size(); ++xt )
	    {
	      QString imgComb02File = basename + "combined" + "." + modelNames[ m ];
	      QMap < QString, QString > c_parms;
	      QString t_m, c_type;
	      
	      if( xtype[ xt ] == 1 )
		{
		  imgComb02File += ".MW" + svgext;
		  t_m = "MW," + modelNames[ m ];
		  c_type = "MW";
		  c_parms = comboPlotsMap[ t_m ];
		}
	      
	      else if( xtype[ xt ] == 2 )
		{
		  imgComb02File += ".D20" + svgext;
		  t_m = "D," + modelNames[ m ];
		  c_type = "D";
		  c_parms = comboPlotsMap[ t_m ];
		}
	      
	      else if( xtype[ xt ] == 3 )
		{
		  imgComb02File += ".f_f0" + svgext;
		  t_m = "f/f0," + modelNames[ m ];
		  c_type = "f/f0";
		  c_parms = comboPlotsMap[ t_m ];
		}

	      //check if to generate Combined plot for current 'type-method':
	      //if ( comboPlotsMapTypes.contains( t_m ) && comboPlotsMapTypes[ t_m ] != 0  )
	      bool show_combo_plot_other_types   = (combPlotsMask_edited.ShowCombPlotParts[ c_type ][ modelNames[ m ] ].toInt()) ? true : false ;
	      if ( show_combo_plot_other_types )
		{
		  
		  //plottedIDs_other_type = sdiag_combplot-> changedPlotX_auto( xtype[ xt ], c_parms );
		  plotted_ids_colors_map_s_type = sdiag_combplot-> changedPlotX_auto( xtype[ xt ], c_parms );
		    
		  //qDebug() << "PLOTTED_IDs_" << c_type << "_type -- " << plottedIDs_other_type;
		  
		  
		  write_plot( imgComb02File, sdiag_combplot->rp_data_plot1() );              //<-- rp_data_plot1() gives combined plot
		  imgComb02File.replace( svgext, pngext );
		  CombPlotsFileNames << imgComb02File;

		  CombPlotsParmsMap       [ imgComb02File ] = plotted_ids_colors_map_s_type. firstKey();
		  CombPlotsParmsMap_Colors[ imgComb02File ] = plotted_ids_colors_map_s_type[ plotted_ids_colors_map_s_type. firstKey() ];
		  
		  //CombPlotsParmsMap[ imgComb02File ] = plottedIDs_other_type;
		}

	      ++pr_cp_val;
	      progress_msg->setValue( pr_cp_val );
	    }
	}
      // reset plot
      sdiag_combplot->reset_data_plot1();
    }
  //assemble combined plots into html
  assemble_plots_html( CombPlotsFileNames, "CombPlots"  );
  
  progress_msg->setValue( progress_msg->maximum() );
  progress_msg->close();
  qApp->processEvents();
}

//Plot pseudo3d distr.
void US_ReporterGMP::plot_pseudo3D( QString triple_name,  QString stage_model)
{
  qDebug() << "In plot_pseudo3D: init -- " << triple_name << stage_model;
  QString modelid = Triple_to_ModelsDesc[ triple_name ][ stage_model ];
  QString t_name  = triple_name;
  t_name.replace(".", "");

  if ( t_name. contains( "Interference" ) )
    t_name. replace( "Interference", "660" );
  
  //Pseudo3D Plots Generation (after simulations? Actual simulations are NOT needed for Pseudo3d plots?)
  //FileName; //<-- a single-type runID (e.g. RI) - NOT combined runs for now...

  QString filename_returned = get_filename( triple_name );
  qDebug() << "In plot_pseudo3D, filename_returned -- " << filename_returned;


  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  //mkdir( US_Settings::reportDir(), filename_returned );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );
  QString basename  = dirName + "/" + filename_returned + ".";
  //QString basename  = US_Settings::reportDir() + "/" + filename_returned + "/" + filename_returned + ".";
  
  QString imgPseudo3d01File;
  QStringList Pseudo3dPlotsFileNames;


  //ALEXEY: should it be a stricter requirement (modelID ?)
  QStringList m_t_r_id;  
  m_t_r_id << stage_model << t_name << filename_returned << modelid;
  
  qDebug() << "m_t_r_id to model_loader -- " << m_t_r_id;
  
  sdiag_pseudo3d = new US_Pseudo3D_Combine();
  sdiag_pseudo3d -> load_distro_auto ( QString::number( invID ), m_t_r_id );

  //Replace back for internals
  if ( triple_name.contains("Interference") )
    t_name. replace( "660", "Interference");

  //'S' data
  if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
    t_name = t_name. replace( "S", "A" );
  
  //here identify what to show:
  bool show_s_ff0  = (perChanMask_edited.ShowTripleModelPseudo3dParts[ t_name ][ stage_model ][ "Pseudo3d s-vs-f/f0 Distribution" ].toInt()) ? true : false ;
  bool show_s_d    = (perChanMask_edited.ShowTripleModelPseudo3dParts[ t_name ][ stage_model ][ "Pseudo3d s-vs-D Distribution" ].toInt()) ? true : false ;
  bool show_mw_ff0 = (perChanMask_edited.ShowTripleModelPseudo3dParts[ t_name ][ stage_model ][ "Pseudo3d MW-vs-f/f0 Distribution" ].toInt()) ? true : false ;
  bool show_mw_d   = (perChanMask_edited.ShowTripleModelPseudo3dParts[ t_name ][ stage_model ][ "Pseudo3d MW-vs-D Distribution" ].toInt()) ? true : false ;

  qDebug() << "[in plot_pseudo3D()]: show_s_ff0 -- "  << show_s_ff0;
  qDebug() << "[in plot_pseudo3D()]: show_s_d -- "    << show_s_d;
  qDebug() << "[in plot_pseudo3D()]: show_mw_ff0 -- " << show_mw_ff0;
  qDebug() << "[in plot_pseudo3D()]: show_mw_d -- "   << show_mw_d;

  if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
    t_name = t_name. replace( "A", "S" );
  
  //write plot: here default is [s-f/f0] coordinates (x,y)
  if( show_s_ff0 )
    {
      sdiag_pseudo3d -> select_x_axis_auto( 0 ); // [s-]
      sdiag_pseudo3d -> select_y_axis_auto( 1 ); // [s-f/f0]
      imgPseudo3d01File = basename + "pseudo3D" + "." + stage_model  + "." +  t_name + ".sff0" + svgext;  // [s-f/f0]
      write_plot( imgPseudo3d01File, sdiag_pseudo3d->rp_data_plot() );                //<-- rp_data_plot() gives pointer to pseudo3D plot
      imgPseudo3d01File.replace( svgext, pngext ); 
      Pseudo3dPlotsFileNames << imgPseudo3d01File;
    }
  // ++pr_val;
  // progress_msg->setValue( pr_val );
  
  //here, we have to go over [x-y] coordinates for given [triple-model]: 
  // s: 0; f/f0: 1; MW: 2; D: 4
  //[0-1] (s-f/f0) already processed:
  //to_process: [0-4], [2-1], [2-4]:
  if( show_s_d )
    {
      sdiag_pseudo3d -> select_x_axis_auto( 0 ); // [s-]
      sdiag_pseudo3d -> select_y_axis_auto( 4 ); // [s-D]
      imgPseudo3d01File = basename + "pseudo3D" + "." + stage_model  + "." +  t_name + ".sD" + svgext;
      write_plot( imgPseudo3d01File, sdiag_pseudo3d->rp_data_plot() );                //<-- rp_data_plot() gives pointer to pseudo3D plot
      imgPseudo3d01File.replace( svgext, pngext ); 
      Pseudo3dPlotsFileNames << imgPseudo3d01File;
    }
  // ++pr_val;
  // progress_msg->setValue( pr_val );
  
  if( show_mw_ff0 )
    {
      sdiag_pseudo3d -> select_x_axis_auto( 2 ); // [MW-]
      sdiag_pseudo3d -> select_y_axis_auto( 1 ); // [MW-f/f0]
      imgPseudo3d01File = basename + "pseudo3D" + "." + stage_model  + "." +  t_name + ".mwff0" + svgext;
      write_plot( imgPseudo3d01File, sdiag_pseudo3d->rp_data_plot() );                //<-- rp_data_plot() gives pointer to pseudo3D plot
      imgPseudo3d01File.replace( svgext, pngext ); 
      Pseudo3dPlotsFileNames << imgPseudo3d01File;
    }
  // ++pr_val;
  // progress_msg->setValue( pr_val );
  
  if( show_mw_d )
    {
      sdiag_pseudo3d -> select_x_axis_auto( 2 ); // [MW-]
      sdiag_pseudo3d -> select_y_axis_auto( 4 ); // [MW-D]
      imgPseudo3d01File = basename + "pseudo3D" + "." + stage_model  + "." +  t_name + ".mwD" + svgext;
      write_plot( imgPseudo3d01File, sdiag_pseudo3d->rp_data_plot() );                //<-- rp_data_plot() gives pointer to pseudo3D plot
      imgPseudo3d01File.replace( svgext, pngext ); 
      Pseudo3dPlotsFileNames << imgPseudo3d01File;
    }
  // ++pr_val;
  // progress_msg->setValue( pr_val );
  
  //reset plots
  sdiag_pseudo3d->reset_auto();
  
  //assemble combined plots into html
  assemble_plots_html( Pseudo3dPlotsFileNames  );
  //progress_msg->setValue( progress_msg->maximum() );
}

// Calculate residual absorbance values (data - sim - noise)
void US_ReporterGMP::calc_residuals()
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

   rmsd_global =  QString::number( rmsd );

   qDebug() << "CALC_RESID: matchd" << matchd << "kexcls" << kexcls << "rmsd" << rmsd;

}

//output HTML string for run details
void  US_ReporterGMP::assemble_run_details_html( void )
{
  html_assembled += "<p class=\"pagebreak \">\n";
  html_assembled += "<h2 align=left>Run Details</h2>";
 
  //1. get runID OR runIDs 
  QStringList fileNameList;
  fileNameList. clear();
  if ( FileName.contains(",") && FileName.contains("IP") && FileName.contains("RI") )
    fileNameList  = FileName.split(",");
  else
    fileNameList << FileName;

  
  US_Passwd pw;
  US_DB2*   dbP  = new US_DB2( pw.getPasswd() );
  if ( dbP->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem, run details" ),
			    tr( "Could not connect to database: \n" ) + dbP->lastError() );
      return;
    }


  //Create dir for plots: (if any)
  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  //mkdir( US_Settings::reportDir(), filename_passed );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );
  
  QStringList RunDetailsPlotsFileNames;

  
  //2. Iterate over runIds
  for ( int i=0; i<fileNameList.size(); ++i )
    {

      if ( fileNameList[ i ].contains("IP") )
	continue;
      
      // html_assembled += tr( "<h3 align=left>TimeStamp Parameters for Run: %1</h3>" ).
      // 	arg( fileNameList[ i ] );
      html_assembled += tr( "<h3 align=left>TimeStamp Parameters:</h3>" );
      //html_assembled += tr("<br>");
      
      //3. get expID based on runID && invID
      int experimentID = get_expID_by_runID_invID( dbP, fileNameList[ i ] );

      if ( experimentID == 0 )
       	{
       	  QMessageBox::warning( this, tr( "ExpID zero, run details" ),
				tr( "Experiment ID zero!!!: \n" ) );
       	  return; 
       	}
      
      //4. get timeSateID from expID
      int tmstID     = 0;
      QString tfname = fileNameList[ i ] + ".time_state.tmst";
      QString xdefs;
      QString cksumd;
      QDateTime datedt;
      US_TimeState::dbExamine( dbP, &tmstID, &experimentID, &tfname,
			       &xdefs, &cksumd, &datedt );

      qDebug() << "Assembling Run Details: expID tmstID tfname cksumd datedt"
	       << experimentID << tmstID << tfname << cksumd << datedt;
      
      if ( tmstID == 0 )
	{
	  QMessageBox::warning( this, tr( "TimeStateID zero, run details" ),
				tr( "TimeState ID zero!!!: \n" ) );
       	  return;
       	}

      
      //5. Download the .tmst file
      QDir        readDir( US_Settings::resultDir() );
      QString     dirname = readDir.absolutePath() + "/" +  fileNameList[ i ];
      QDir edir( dirname );
      if (!edir.exists())
	edir.mkpath( dirname );
      
      QString tfpath = dirname + "/" +  tfname;
      
      US_TimeState::dbDownload( dbP, tmstID, tfpath );
      // And write the xdefs sibling file
      QString xfpath = QString( tfpath ).replace( ".tmst", ".xml" );
      qDebug() << "Assembling Run Details:   xfpath" << xfpath;
      QFile fileo( xfpath );
      if ( fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
	  QTextStream tso( &fileo );
	  tso << xdefs;
	  tso.flush();
	  fileo.close();
	  qDebug() << "Assembling Run Details:    xdefs WRITTEN";
	}


      //6. Get access to Timestamp info, possibly plots also...
      // Point to any existing time state file
      QString tmst_fnamei  = QString( "" );
      QString defs_fnamei  = QString( "" );
      
      QDir ddir( dirname );
      QStringList tmsfs = ddir.entryList( QStringList( "*.time_state.*" ),
					  QDir::Files, QDir::Name );
      QString tmst_dname  = dirname;
      if ( tmst_dname.right( 1 ) != "/" )
	tmst_dname   += "/";
      qDebug() << "Assembling Run Details: tmst_dname" << tmst_dname << "tmsfs count" << tmsfs.count();

      if ( tmsfs.count() == 2 )
	{  // Looks like we have a TMST and corresponding Defs XML
	  tmst_fnamei   = tmsfs[ 0 ];
	  defs_fnamei   = tmsfs[ 1 ];
	  qDebug() << "Assembling Run Details: tmsfs" << tmst_fnamei << defs_fnamei;
	  
	  if ( tmst_fnamei.contains( ".tmst" )  &&
	       defs_fnamei.contains( ".xml" ) )
	    {  // Have both files, so save full path to TMST
	      tmst_fnamei   = tmst_dname + tmst_fnamei;
	    }
	  
	  else if ( tmsfs[ 0 ].contains( ".xml" )  &&
		    tmsfs[ 1 ].contains( ".tmst" ) )
	    {  // Have both files (in opposite order), so save full path to TMST
	      tmst_fnamei   = tmst_dname + tmsfs[ 1 ];
	    }
	  
	  else
	    {  // Do not have both files, so clear TMST file path
	      tmst_fnamei.clear();
	    }
	  
	  qDebug() << "Assembling Run Details: tmst,defs fnamei" << tmst_fnamei << defs_fnamei;
	}
      else  // If file does not exist, clear name
	{
	  qDebug() << "Assembling Run Details: TimeStamp NON_EXIST:" << tmst_fnamei;
	  tmst_fnamei.clear();
	}
      
      US_TmstPlot* tsdiag = new US_TmstPlot( this, tmst_fnamei );

      //7.  access all needed params from the current timestamp
      QStringList dkeys = tsdiag -> timestamp_data_dkeys();
      QVector< QVector< double > > dvals = tsdiag -> timestamp_data_dvals();
      QMap< QString, double > dmins = tsdiag -> timestamp_data_mins();
      QMap< QString, double > dmaxs = tsdiag -> timestamp_data_maxs();
      QMap< QString, double > davgs = tsdiag -> timestamp_data_avgs();
      //QMap< QString, double > davgs_first_scan = tsdiag -> timestamp_data_avgs_first_scan();
      QMap < QString, QMap< QString, double > > davgs_stdd_first_scan =  tsdiag ->  timestamp_data_avgs_stdd_first_scan();
      
      html_assembled += tr(
			     "<table style=\"margin-left:10px\">"
			   );

      html_assembled += table_row( tr( "Parameter: " ),
				   tr( "Target Value:" ),
				   tr( "Tolerance:"),
				   tr( "Measured Value:" ),
				   tr( "Std. Dev." ),
				   tr( "PASSED ?" ));

      
      //Iterate over Time Stamp params...
      for ( int i=0; i < dkeys.size(); ++i )
	{
	  qDebug() << "Assembling Run Details: " << dkeys[ i ] << dmins[ dkeys[ i ] ] << " to " << dmaxs[ dkeys[ i ] ]
		   << "; Avg: " << davgs [ dkeys[ i ] ]
		   << "; Avg_1st_scan: " << davgs_stdd_first_scan [ dkeys[ i ] ][ "Avg" ]
		   << "; Stdd_1st_scan: " << davgs_stdd_first_scan [ dkeys[ i ] ][ "Stdd" ];

	  double val_tol      = 0;
	  double val_target   = 0;
	  double val_measured = 0;
	  QString val_passed  = "";
	  double val_measured_stdd  = 0;

	  //Exp. Duration
	  if ( dkeys[ i ] == "Time" )
	    {
	      val_target       = currProto. rpSpeed. ssteps[0].duration +  (double) ( currProto. rpSpeed. ssteps[0].speed / currProto. rpSpeed. ssteps[0].accel );
	      qDebug() << "Assembling Run Details: exp. duration, speed, accel -- "
		       << currProto. rpSpeed. ssteps[0].duration
		       << currProto. rpSpeed. ssteps[0].speed
		       << currProto. rpSpeed. ssteps[0].accel;
	      int  hours_r     = (int)qFloor( val_target / 3600.0 );
	      int  mins_r      = (int)qRound( ( val_target - hours_r * 3600.0 ) / 60.0 );
	      QString hh_r     = "h";
	      QString val_target_hh_mm    = QString().sprintf( "%d %s %02d m", hours_r, hh_r.toLatin1().data(), mins_r );

	      //tol: based on 1st AProfile->ReportGMP
	      QString channel_desc_alt = chndescs_alt[ 0 ];
	      QString channel_desc     = chndescs[ 0 ];
	      QString wvl              = QString::number( ch_wvls[ channel_desc_alt ][ 0 ] );
	      US_ReportGMP reportGMP   = ch_reports[ channel_desc_alt ][ wvl ];
	      val_tol = reportGMP. experiment_duration_tol;
	      
	      val_measured = dmaxs[ dkeys[ i ] ] - dmins[ dkeys[ i ] ]; //in mins
	      double val_measured_sec = val_measured * 60.0;
	      hours_r     = (int)qFloor( val_measured_sec / 3600.0 );
	      mins_r      = (int)qRound( ( val_measured_sec - hours_r * 3600.0 ) / 60.0 );
	      QString val_measured_hh_mm   = QString().sprintf( "%d %s %02d m", hours_r, hh_r.toLatin1().data(), mins_r );

	      val_passed  = ( val_measured_sec >= ( val_target * (1 - val_tol/100.0)  ) && val_measured_sec <= ( val_target * (1 + val_tol/100.0) ) ) ? "YES" : "NO";
	      
	      html_assembled += table_row( tr( "Experiment Duration" ),
					   val_target_hh_mm,
					   QString::number( val_tol ) + "%",
					   val_measured_hh_mm,
					   "",
					   val_passed ) ;
	    }

	  //Temperature
	  if ( dkeys[ i ] .contains("Temperature") )
	    {
	      val_tol    = 0.5;   //plus-minus 0.5 C
	      val_target = currProto.temperature; 
	      val_measured = davgs_stdd_first_scan [ dkeys[ i ] ][ "Avg" ];
	      val_passed  = ( val_measured  >= ( val_target - val_tol ) && val_measured  <= ( val_target + val_tol ) ) ? "YES" : "NO";

	      val_measured_stdd = davgs_stdd_first_scan [ dkeys[ i ] ][ "Stdd" ];
	  
	      html_assembled += table_row( dkeys[ i ] + " (&#8451;)",
					   QString::number( val_target ),
					   "&#177;" + QString::number( val_tol ),
					   QString::number( val_measured ),
					   QString::number( val_measured_stdd ),
					   val_passed );

	      //Plot Temp. plot
	      QString img01File = dirName + "/" + "RunDetails_Temperature" + svgext;
	      write_plot( img01File, tsdiag->rp_data_plot1( "Temperature" ) );
	      img01File.replace( svgext, pngext ); 
	      RunDetailsPlotsFileNames << img01File;
	    }

	  //RawSpeed
	  if ( dkeys[ i ] .contains("RawSpeed") )
	    {
	      val_tol    = 4; //plus-minus 4 RPM
	      val_target = currProto. rpSpeed. ssteps[0].speed; 
	      val_measured = davgs_stdd_first_scan [ dkeys[ i ] ][ "Avg" ];
	      val_passed  = ( val_measured  >= ( val_target - val_tol ) && val_measured  <= ( val_target + val_tol ) ) ? "YES" : "NO"; 
	  	  
	      html_assembled += table_row( dkeys[ i ] + " (RPM)",
					   QString::number( val_target ),
					   "&#177;" + QString::number( val_tol ),
					   QString::number( val_measured ),
					   "",
					   val_passed );
	    }
	   
	  
	  // html_assembled += tr(
	  // 		       "<tr><td>%1: </td> <td>%2 to %3</td> <td>Avg.: %4</td></tr>"
	  // 		       )
	  //   .arg( dkeys[ i ] )                 //1
	  //   .arg( dmins[ dkeys[ i ] ] )        //2
	  //   .arg( dmaxs[ dkeys[ i ] ] )        //3
	  //   .arg( davgs [ dkeys[ i ] ] )       //4
	  //   ;
	}
      
      html_assembled += tr(
			   "</table>"
			   );
      
      //end of the loop over filenames (only one IF not a combined RI+IP run)
    }
  
  assemble_plots_html( RunDetailsPlotsFileNames );
    
  html_assembled += tr("<hr>");
  //
  html_assembled += "</p>\n";

  //test
  delete dbP;
}

//get expID
int US_ReporterGMP::get_expID_by_runID_invID( US_DB2* dbP, QString runID_filename )
{
  QStringList query;
  int         idExp  = 0;
  query << "get_experiment_info_by_runID"
	<< runID_filename
	<< QString::number(invID);
  qDebug() << "get_experiment_info_by_runID: qry -- " << query;
  dbP->query( query );
  
  if ( dbP->lastErrno() == US_DB2::OK )
    {
      dbP->next();
      idExp              = dbP->value( 1 ).toInt();
    }

  return idExp;
}


//output HTML string for user interactions
void US_ReporterGMP::assemble_user_inputs_html( void )
{
  html_assembled += "<p class=\"pagebreak \">\n";
  html_assembled += "<h2 align=left>User Interactions:\n GMP Run Initiation, Live Update, Data Import, Editing, and Analysis</h2>";

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
  
  // //TEMP: DEBUG
  // importRIJson =
  //   "{ \"Person\": [{\"ID\":\"12\",\"fname\":\"Alexey\",\"lname\":\"Savelyev\",\"email\":\"oleksiy.savelyev@umontana.edu\",\"level\":\"4\"}],\"RefScan\": \"automated\"}";
  // importIPJson =
  //   "{ \"Person\": [{\"ID\":\"12\",\"fname\":\"Alexey\",\"lname\":\"Savelyev\",\"email\":\"oleksiy.savelyev@umontana.edu\",\"level\":\"4\"}],\"RefScan\": \"manual\"}";
  // importRIts = "2022-03-29 16:24:36";
  // importIPts = "2022-03-30 17:24:36";

  // //editing
  // editRIJson =
  //   "{ \"Person\": [{\"ID\":\"12\",\"fname\":\"Alexey\",\"lname\":\"Savelyev\",\"email\":\"oleksiy.savelyev@umontana.edu\",\"level\":\"4\"}],\"Meniscus\": [{\"1 / A\":\"automated\",\"1 / B\":\"manual\",\"2 / A\":\"automated\",\"2 / B\":\"automated\", \"3 / A\":\"automated\",\"3 / B\":\"manual\",\"4 / A\":\"automated\",\"4 / B\":\"automated\"}]}";
  // editIPJson = "{ \"Person\": [{\"ID\":\"12\",\"fname\":\"Alexey\",\"lname\":\"Savelyev\",\"email\":\"oleksiy.savelyev@umontana.edu\",\"level\":\"4\"}],\"Meniscus\": [{\"1 / A\":\"manual\"}]}";
  // editRIts = "2022-03-29 22:24:36";
  // editIPts = "2022-03-30 23:26:36";
  // // END of DEBUG ////////////


  //read autoflowStatus record:
  read_autoflowStatus_record( importRIJson, importRIts, importIPJson, importIPts,
			      editRIJson, editRIts, editIPJson, editIPts, analysisJson,
			      stopOptimaJson, stopOptimats, skipOptimaJson, skipOptimats,
			      analysisCancelJson, createdGMPrunJson, createdGMPrunts ); 
  /////////////////////////////

  //1. GMP run creation
  html_assembled += tr("<hr>");
  html_assembled += tr( "<h3 align=left>GMP Run Initiation (1. EXPERIMENT)</h3>" );
  QMap< QString, QMap < QString, QString > > status_map_c = parse_autoflowStatus_json( createdGMPrunJson, "" );

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
			   "<td> Initiated at:     %1 (UTC) </td>"
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
  

  //2. LIVE_UPDATE
  QMap < QString, QString >::iterator im;
  operation_types_live_update[ "STOP" ] = stopOptimaJson;
  operation_types_live_update[ "SKIP" ] = skipOptimaJson;

  operation_types_live_update_ts[ "STOP" ] = stopOptimats;
  operation_types_live_update_ts[ "SKIP" ] = skipOptimats;

  if ( !stopOptimaJson.isEmpty() || !skipOptimaJson.isEmpty() )
    html_assembled += tr( "<h3 align=left>Remote Stage Skipping, Stopping Machine (2. LIVE_UPDATE)</h3>" );
  
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
      
      html_assembled += tr("<br>");

      html_assembled += tr(
			   "<table>"		   
			   "<tr>"
			        "<td><b>Operation Type::</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			   )
	.arg( dtype_opt )                       //1
	;

      //Parse Json
      QMap< QString, QMap < QString, QString > > status_map = parse_autoflowStatus_json( json_str, im.key() );
      
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
			   "<td> Performed at:     %2 (UTC) </td>"
			   "</tr>"
			   "</table>"
			   )
	.arg( status_map[ "Remote Operation" ][ "type"] )      //1
	.arg( operation_types_live_update_ts[ im.key() ] )     //2
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
    }
  if ( !stopOptimaJson.isEmpty() || !skipOptimaJson.isEmpty() )
    html_assembled += tr("<hr>");

  
  //3. IMPORT
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
  
  html_assembled += tr( "<h3 align=left>Reference Scan Determination, Triples Dropped, Data Saving (3. IMPORT)</h3>" );
  
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
      
      //html_assembled += tr("<br>");

      html_assembled += tr(
			   "<table>"		   
			   "<tr>"
			      "<td><b>Data Type, Optics:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			)
	.arg( dtype_opt )                       //1
	;

      //Parse Json
      QMap< QString, QMap < QString, QString > > status_map = parse_autoflowStatus_json( json_str, im.key() );
      
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
	.arg( ref_scan_method )                      //1
	.arg( data_types_import_ts[ im.key() ] )     //2
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
      

      //Add list if dropped triples per optics system:
      if ( !dtype_opt_dropped_triples. isEmpty() )
	{
	  html_assembled += tr(
			       "<table style=\"margin-left:10px\">"
			       "<caption style=\"color:red;\" align=left> <b><i>List of Dropped Triples: </i></b> </caption>"
			       "</table>"

			       "<table style=\"margin-left:25px\">"
			       );

	  for ( int i=0; i < dtype_opt_dropped_triples.size(); ++i )
	    {
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
			       
	}

      //Add comments for Dropped triples|channels|select channels (if any):
      if ( status_map. contains("Dropped") )
	{
	  //iterate over comments for different types of dropping operations:
	  html_assembled += tr(
			       "<table style=\"margin-left:10px\">"
			       "<caption align=left> <b><i>Comments on [triples | channels | select channel] dropped: </i></b> </caption>"
			       "</table>"
			       
			       "<table style=\"margin-left:25px\">"
			       )
	    ;
	  
	  QMap < QString, QString >::iterator dr;
	  for ( dr = status_map[ "Dropped" ].begin(); dr != status_map[ "Dropped" ].end(); ++dr )
	    {
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
	}
    }
   
  html_assembled += tr("<hr>");

  
  //4. EDITING
  data_types_edit [ "RI" ] = editRIJson;
  data_types_edit [ "IP" ] = editIPJson;

  data_types_edit_ts [ "RI" ] = editRIts;
  data_types_edit_ts [ "IP" ] = editIPts;

  html_assembled += tr( "<h3 align=left>Meniscus Position Determination, Edit Profiles Saving (4. EDITING)</h3>" );
  
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
      
      //html_assembled += tr("<br>");

      html_assembled += tr(
			   "<table>"		   
			   "<tr>"
			      "<td><b>Data Type, Optics:</b> &nbsp;&nbsp;&nbsp;&nbsp; </td> <td><b>%1</b></td>"
			   "</tr>"
			   "</table>"
			)
	.arg( dtype_opt )                       //1
	;

      //Parse Json
      QMap< QString, QMap < QString, QString > > status_map = parse_autoflowStatus_json( json_str, im.key() );
      
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
			   "<caption align=left> <b><i>Edit Profiles Saved on: </i></b> </caption>"
			   "</table>"
			   
			   "<table style=\"margin-left:25px\">"
			   "<tr><td> %1 (UTC)</td>"
			   "</table>"
			   )
	.arg( data_types_edit_ts[ im.key() ] )           //1
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
   
  html_assembled += tr("<hr>");

  //5. ANALYSIS
  html_assembled += tr( "<h3 align=left>Meniscus Position from FITMEN Stage, Job Cancellation (5. ANALYSIS)</h3>" );

  QMap < QString, QString > analysis_status_map       = parse_autoflowStatus_analysis_json( analysisJson );
  QMap < QString, QString > analysisCancel_status_map = parse_autoflowStatus_analysis_json( analysisCancelJson );

  if ( !cAP2.job3auto ) // interactive FITMEN (manual)
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
			       "<td> at:       %4  (UTC)</td>"
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
  else  // FITMEN_AUTO (automatic)
    {
      html_assembled += tr( "Meniscus positions have been determined automatically as best fit values for all channels." );
    }

  
  //Now add info on the CANCELED Jobs as captured in DB:
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
			       "<td> When:               %4 (UTC) </td>"
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
  
  html_assembled += tr("<hr>");
  //
  html_assembled += "</p>\n";
}

//Read AProfile's reportIDs per channel:
void US_ReporterGMP::read_reportLists_from_aprofile( QStringList & dropped_triples_RI, QStringList & dropped_triples_IP )
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
QStringList US_ReporterGMP::buildDroppedTriplesList ( US_DB2* dbP, QMap <QString, QString> channame_to_reportIDs )
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
bool US_ReporterGMP::readReportLists( QXmlStreamReader& xmli, QMap< QString, QString> & channame_to_reportIDs_RI, QMap< QString, QString> & channame_to_reportIDs_IP )
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
	}
      
      bool was_end    = xmli.isEndElement();  // Just read was End of element?
      xmli.readNext();                        // Read the next element
      
      if ( was_end  &&  ename == "p_2dsa" )   // Break 
	break;
    }
  
  return ( ! xmli.hasError() );
}

//read autoflowStatus, populate internals
void US_ReporterGMP::read_autoflowStatus_record( QString& importRIJson, QString& importRIts, QString& importIPJson, QString& importIPts,
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
  qry << "read_autoflow_status_record" << autoflowStatusID;

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

  qDebug() << "Read_autoflow_status: stopOptimaJson, skipOptimaJson, analysisJson -- "
	   << stopOptimaJson
	   << skipOptimaJson
	   << analysisJson;
}

//Parse autoflowStatus Analysis Json
QMap < QString, QString > US_ReporterGMP::parse_autoflowStatus_analysis_json( QString statusJson )
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

//Parse autoflowStatus RI/IP Json
QMap< QString, QMap < QString, QString > > US_ReporterGMP::parse_autoflowStatus_json( QString statusJson, QString dtype )
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

//output HTML string for Distributions for current triple:
void  US_ReporterGMP::assemble_distrib_html( QMap < QString, QString> & tripleInfo )
{
  //QString html_distibutions = distrib_info();
  html_assembled += "<p class=\"pagebreak \">\n";
  html_assembled += html_header( "US_Fematch", text_model( model, 2 ), edata );
  html_assembled += distrib_info( tripleInfo );
  html_assembled += "</p>\n";
  html_assembled += "</body></html>";
}


//output HTML string for Average Integration Results:
void  US_ReporterGMP::assemble_replicate_av_integration_html( void )
{
  //QString html_distibutions = distrib_info();
  html_assembled += "<p class=\"pagebreak \">\n";
  html_assembled += calc_replicates_averages();
  html_assembled += "</p>\n";
  
}

//output HTML plots for currentTriple
void  US_ReporterGMP::assemble_plots_html( QStringList PlotsFilenames, const QString plot_type )
{
  // Embed plots in the composite report
  html_assembled += "<p class=\"pagebreak \">\n";
  for ( int i = 0;  i < PlotsFilenames.size(); ++ i )
    {
      QString filename = PlotsFilenames[ i ];
      //QString filename = "./" + PlotsFilenames[ i ]. section('/', -1); //try relative path!
      
      QString label = "";

      //Get size of the image
      QImageReader reader( filename );
      QSize sizeOfImage = reader.size();
      int i_height = sizeOfImage.height();
      int i_width = sizeOfImage.width();

      qDebug() << "Image, " << filename << "width, height: " << i_width << i_height;
      
      //QPrinter below must be the same as defined just prior painting QTextDocument in ::write_pdf
      QPrinter printer_t(QPrinter::PrinterResolution);//(QPrinter::HighResolution);//(QPrinter::PrinterResolution);
      printer_t.setOutputFormat(QPrinter::PdfFormat);
      printer_t.setPaperSize(QPrinter::Letter);
      QSizeF pageSize = printer_t.pageRect().size();
      qreal qprinters_width = pageSize.width()*0.8; //500 DEPENDS on QPrinter's constructor settings {QPrinter::PrinterResolution, 500; QPrinter::HighResolution, 9066}
      qDebug() << "qprinters_width: " << qprinters_width; 
      double i_scale_factor = double( qprinters_width / i_width ); 
      int scaled_i_width  = i_width  * i_scale_factor;
      int scaled_i_height = i_height * i_scale_factor;

      qDebug() << "Image scaled, " << filename << "scaled_width, scaledheight: " << scaled_i_width << scaled_i_height;
      
      /* for Combined Plots 

	 <img style='height: 100%; width: 100%; object-fit: contain'/>
	 <img style='height: 100%; width: 100%; object-fit: cover'/>
	 <img style='max-height: 50%; max-width: 100%;'/>
       */
      
      // html_assembled   += "    <div><img style=\"height: 50px; width: 100px\" src=\"" + filename 
      // 	+ "\" alt=\"" + label + "\"/></div>\n\n";

      html_assembled   += "    <div><img src=\"" + filename 
       	+ "\" alt=\"" + label;

      if ( !plot_type.isEmpty() ) // For Combined plots, scale down .png 
       	//html_assembled  += "\"height=\"500 \"width=\"500";
	html_assembled  += " \"height=\"" + QString::number( scaled_i_width ) + "\"width=\"" + QString::number( scaled_i_width);
      else
	{
	  html_assembled  += " \"height=\"" + QString::number( scaled_i_height ) + "\"width=\"" + QString::number( scaled_i_width);
	}

      qDebug() << "Image size html string: " << "\"height=\"" + QString::number( scaled_i_height ) + "\"width=\"" + QString::number( scaled_i_width);
      
      html_assembled   += "\"/></div>\n\n";
      
      html_assembled   += "<br>";

      //add custom legen for combined plots
      if ( !plot_type.isEmpty() )
	{
	  QStringList combparms            = CombPlotsParmsMap[ filename ];
	  QList< QColor > combparms_colors = CombPlotsParmsMap_Colors[ filename ];

	  qDebug() << "COMBOPLOT-parms/colors SZIES: combparms.size(), combparms_colors.size() --  "
		   << combparms.size() << combparms_colors.size();
	  
	  for ( int i=0; i < combparms.size(); ++i )
	    {
	      if ( !combparms_colors.isEmpty() &&  combparms_colors.size() == combparms.size() )
		{
		  qDebug() << "COMBOPLOT-COLORS -- " << combparms_colors[ i ].name();
		  //html_assembled   += "<br><span style='color:blue'>&#9726;</span>";
		  html_assembled   += "<br><span style='color:" + combparms_colors[ i ].name() + "'>&#9726;</span>";
		}
	      else
		html_assembled   += "<br><span style='color:blue'>&#9726;</span>"; // SOME default color!!!

	      html_assembled   += combparms[ i ] + ")&nbsp;"; 
	      //html_assembled   += combparms[ i ].split(")")[1] + ")&nbsp;"; 
	    }
	}
    }
  
  html_assembled += "</p>\n";
}

  
// Interpolate an sdata y (readings) value for a given x (radius)
double US_ReporterGMP::interp_sval( double xv, double* sx, double* sy, int ssize )
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

// Model type text string
QString US_ReporterGMP::text_model( US_Model model, int width )
{
   QString stitle = model.typeText();
   QString title  = stitle;
   QString m_desc = model. description;

   if ( width != 0 )
   {  // long title:  add any suffixes and check need to center
      switch ( (int)model.analysis )
      {
         case (int)US_Model::TWODSA:
         case (int)US_Model::TWODSA_MW:
            title = tr( "2-Dimensional Spectrum Analysis" );

	    if ( m_desc. contains("-IT") )
	      title = title + " (IT)";

	    break;

         case (int)US_Model::GA:
         case (int)US_Model::GA_MW:
            title = tr( "Genetic Algorithm Analysis" );
            break;

         case (int)US_Model::COFS:
            title = tr( "C(s) Analysis" );
            break;

         case (int)US_Model::FE:
            title = tr( "Finite Element Analysis" );
            break;

         case (int)US_Model::PCSA:
            title = tr( "Parametrically Constrained Spectrum Analysis\n" );

            if ( stitle.contains( "-SL" ) )
               title += tr( "(Straight Line)" );

            else if ( stitle.contains( "-IS" ) )
               title += tr( "(Increasing Sigmoid)" );

            else if ( stitle.contains( "-DS" ) )
               title += tr( "(Decreasing Sigmoid)" );

            else if ( stitle.contains( "-HL" ) )
               title += tr( "(Horizontal Line)" );

            else if ( stitle.contains( "-2O" ) )
               title += tr( "(2nd-Order Power Law)" );

            break;

         case (int)US_Model::DMGA:
            title = tr( "Discrete Model Genetic Algorithm" );
            break;

         case (int)US_Model::MANUAL:
         default:
            title = tr( "2-Dimensional Spectrum Analysis" );
            break;
      }


            if ( stitle.contains( "-SL" ) )
      if ( model.associations.size() > 1 )
         title = title + " (RA)";

      if ( model.global == US_Model::MENISCUS )
         title = title + " (Menisc.)";

      else if ( model.global == US_Model::GLOBAL )
         title = title + " (Global)";

      else if ( model.global == US_Model::SUPERGLOBAL )
         title = title + " (S.Glob.)";

      if ( model.monteCarlo )
         title = title + " (MC)";

      if ( width > title.length() )
      {  // long title centered:  center it in fixed-length string
         int lent = title.length();
         int lenl = ( width - lent ) / 2;
         int lenr = width - lent - lenl;
         title    = QString( " " ).repeated( lenl ) + title
                  + QString( " " ).repeated( lenr );
      }
   }

   return title;
}


// Compose a report HTML header
QString US_ReporterGMP::html_header( QString title, QString head1,
				     US_DataIO::EditedData* edata )
{
   QString s = QString( "<?xml version=\"1.0\"?>\n" );
   s  += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
   s  += "                      \"http://www.w3.org/TR/xhtml1/DTD"
         "/xhtml1-strict.dtd\">\n";
   s  += "<html xmlns=\"http://www.w3.org/1999/xhtml\""
         " xml:lang=\"en\" lang=\"en\">\n";
   s  += "  <head>\n";
   s  += "    <title> " + title + " </title>\n";
   s  += "    <meta http-equiv=\"Content-Type\" content="
         "\"text/html; charset=iso-8859-1\"/>\n";
   s  += "    <style type=\"text/css\" >\n";
   s  += "      td { padding-right: 1em; }\n";
   s  += "      body { background-color: white; }\n";

   s  += "    .pagebreak\n";
   s  += "    {\n";
   s  += "      page-break-before: always; border: 1px solid; \n";
   s  += "    }\n";
   
   s  += "    </style>\n";
   s  += "  </head>\n  <body>\n";
   s  += "    <h1>" + head1 + "</h1>\n";
   s  += indent( 2 ) + tr( "<h2>Data Report for Run \"" ) + edata->runID;
   s  += "\",<br/>\n" + indent( 2 ) + "&nbsp;" + tr( " Cell " ) + edata->cell;
   s  += tr( ", Channel " ) + edata->channel;
   s  += tr( ", Wavelength " ) + edata->wavelength;
   s  += ",<br/>\n" + indent( 2 ) + "&nbsp;" + tr( " Edited Dataset " );
   s  += edata->editID + "</h2>\n";

   return s;
}


// Calculate and output Averages from Replicate groups
QString US_ReporterGMP::calc_replicates_averages( void )
{
  /*
    Replicate group #1: [channels 1A, 5A, 6A]
       Subgroup 1:  ----- Same: wvl, type/method, ranges, integraiton value 
         triples:           1A.280, 5A.280, 6A.280 (overlapping wvls)
	 type/method:       [s / 2DSA-IT]
	 ranges:            [3.2 - 3.7]
	 integration value: 0.57 

        Subgroup 2:  ----- Same: wvl, type/method, ranges, integraiton value 
         triples:           1A.280, 5A.280, 6A.280 (overlapping wvls)
	 type/method:       [s / 2DSA-MC]
	 ranges:            [3.4 - 3.9]
	 integration value: 0.51 
      	 
        Subgroup 3:  ----- Same: wvl, type/method, ranges, integraiton value 
         triples:           1A.320, 5A.320, 6A.320 (overlapping wvls)
	 type/method:       [D / PCSA]
	 ranges:            [3.1 - 4.9]
	 integration value: 0.59 
=====================================================================================

    Replicate group #2: [channels 2A, 4A, 6B]
       Subgroup 1:  ----- Same: wvl, type/method, ranges, integraiton value 
         triples:           2A.290, 6B.290 (overlapping wvls)  --> IT Maybe that 4A does not have wvl 290!!!
	 type/method:       [s / 2DSA-IT]
	 ranges:            [3.2 - 3.7]
	 integration value: 0.57 

        Subgroup 2:  ----- Same: wvl, type/method, ranges, integraiton value 
         triples:           2A.290, 6B.290 (overlapping wvls)  --> IT Maybe that 4A does not have wvl 290!!!
	 type/method:       [D / 2DSA-IT]
	 ranges:            [3.4 - 3.9]
	 integration value: 0.51 

=====================================================================================

 Now, the 1st triple in a given subgroup (same-wvl triples) is the REFERENCE one: 
    - e.g., 1A.280, in Replicate #1, subgroup 1,2; 
            1A.320, in Replicate #1, subgroup 3;
            2A.290, in Replicate #2, subgroup 1,2; 

    **  Retrieve US_ReportGMP for these REFERENCE triples && go over theirs ReportItems:
    
       -- US_ReportGMP ref_report = ch_reports[ channel_desc_alt ][ wvl ];
       
          channel_desc_alt &&  wvl are from:
	  QMap< QString, QStringList > channdesc_to_overlapping_wvls[ channel_desc_alt ] [ QStringList( "2A.260","2B.260","4A.260","2A.275","2B.275","4A.275","2A.280","2B.280","4A.280","4B.280","6A.280","6B.280" )]
	  
	  IMPORTANT: in QStringList, group triples by same-wvl (subgroups), then [0] in each group will be REFERENCE wvl!
	  So, reference triples in the above QStringList example will be: 
	                      2A.260, 2A.275 && 2A.280 

       -- Start by iterating over QMap< QString, QStringList > channdesc_to_overlapping_wvls: 
                       iterator::key()   - channel_desc_at; 
		       iterator::value() - access to wvl; IMPORTANT, break into same-wvl groups, then take 1st to access wvl of the REFERNECE triple 
       
       -- Iterate over ReportItems in ref_report.reportItems; 
       -- For each unique ReportItem, identify type-method, ranges, int.value;
       -- Retrive earlier processed (in ::distrib_info() ) integration results for subgroup triples && AVERAGE

       -- BEFORE: in ::distrib_info() STORE integration results into respective reportItem[ kk ]. { integration_val_sim; ...}

   */

  QString html_str_replicate_av;

  if ( !channdesc_to_overlapping_wvls.isEmpty() ) 
    html_str_replicate_av += tr( "<h2>Replicate Groups Averaging Results:</h2>\n" );
  
  QMap< QString, QStringList>::iterator chw;
  for ( chw = channdesc_to_overlapping_wvls.begin(); chw != channdesc_to_overlapping_wvls.end(); ++chw )
    {
      QString ch_alt_desc  = chw.key();
      QStringList all_wvls = chw.value();
      QString o_type       = ch_alt_desc.split(":")[1];

      QStringList unique_wvls;
      QStringList unique_channels;
      QMap < QString, QStringList > same_wvls_chann_map; // wvl: channel list
      
      for( int i=0; i<all_wvls.size(); ++i )
	{
	  QString curr_triple = all_wvls[ i ];
	  QString curr_chann  = all_wvls[ i ].split(".")[0];  
	  QString curr_wvl    = all_wvls[ i ].split(".")[1];

	  unique_wvls               << curr_wvl;
	  unique_channels           << curr_chann;

	  //here, add to list FULL channel desc, e.g. "1A:Iterf.", or "1A:UV/vis."
	  //same_wvls_chann_map[ curr_wvl ] << curr_chann; //BEFORE
	  same_wvls_chann_map[ curr_wvl ] << curr_chann + ":" + o_type; //Will this work?
	}
      
      unique_wvls.     removeDuplicates();
      unique_channels. removeDuplicates();

      QString replicate_group_number = get_replicate_group_number( ch_alt_desc );
      
      html_str_replicate_av += "\n" + indent( 2 ) + tr( "<h3>Replicate Group #%1: [Channels: %2 (%3)] </h3>\n" )
	.arg( replicate_group_number )
	.arg( unique_channels.join(",") )
	.arg( o_type );
      
      
      //iterate over unique wvls
      for ( int j=0; j < unique_wvls.size(); ++j )
	{
	  QString u_wvl = unique_wvls[j];

	  QString replicate_subgroup_triples;
	  for ( int jj=0; jj < same_wvls_chann_map[ u_wvl ].size(); ++jj )
	    replicate_subgroup_triples += same_wvls_chann_map[ u_wvl ][ jj ].split(":")[0] + "." + u_wvl + ",";

	  replicate_subgroup_triples.chop(1);
	  
	  html_str_replicate_av += "\n" + indent( 2 ) + tr( "<h3>Subgroup #%1: [Triples: %2] </h3>\n" )
	    .arg( QString::number( j+1 ) )
	    .arg( replicate_subgroup_triples );

	  html_str_replicate_av += indent( 2 ) + "<table style=\"font-size:80%\">\n";
	  html_str_replicate_av += table_row( tr( "Type:" ),
					      tr( "Method:" ),
					      tr( "Range:"),
					      tr( "Av. Integration, Model (target):" ),
					      tr( "St. Dev.:"),
					      tr( "Av. Fraction %, Model (target):"),
					      tr( "St. Dev.:" ),
					      tr( "Tol.%:"),
					      tr( "PASSED?")
					      );
	  	  
	  //Reference GMP Report, ReportItems for 1st channel in a replicate group && over each wavelength:
	  US_ReportGMP ref_group_report = ch_reports[ ch_alt_desc ][ u_wvl ];
	  int report_items_number = ref_group_report.reportItems.size();
	  for ( int kk = 0; kk < report_items_number; ++kk )
	    {
	      US_ReportGMP::ReportItem ref_group_item = ref_group_report.reportItems[ kk ];

	      QString type           = ref_group_item.type;
	      QString method         = ref_group_item.method;
	      if( method.contains ("PCSA") )
		method = "PCSA";
	      QString int_val_r      = QString::number( ref_group_item.integration_val );
	      double  frac_tot_r     = ref_group_item.total_percent;
	      double  frac_tot_tol_r = ref_group_item.tolerance ;
	      double  low            = ref_group_item.range_low;
	      double  high           = ref_group_item.range_high;

	      QString range          = "[" + QString::number(low) + " - " + QString::number(high) + "]";
	   
	      QMap<QString, double> replicate_g_results = get_replicate_group_results( ref_group_item, u_wvl, same_wvls_chann_map[ u_wvl ] );

	      //print results into HTML report:
	      /* 
		 Replicate Group #: [channles: 1A, 2A, 5A]
		    Sub-Group #: [wvl: 280]
		      type,  method,   range,    integration_AV (target),   St.Dev.   Fraction of Total AV. (target)   PASSED?
		      s      2DSA-MC   3.2-3.7   integration_sim_av         st_dev      XXX                             YES/NO 
	      */

	      double frac_tot_m_av = replicate_g_results["tot_percent_av"];
	      QString tot_av_frac_passed = ( frac_tot_m_av >= ( frac_tot_r * (1 - frac_tot_tol_r/100.0)  )
					     && frac_tot_m_av <= ( frac_tot_r * (1 + frac_tot_tol_r/100.0)  ) ) ? "YES" : "NO";
	      
	      html_str_replicate_av += table_row( type,
						  method,
						  range,
						  QString().sprintf( "%10.4e",  replicate_g_results["int_av"] ) + " (" + int_val_r + ")",
						  QString().sprintf( "%10.2e",  replicate_g_results["int_st_dev"] ),
						  QString().sprintf( "%5.2f%%", replicate_g_results["tot_percent_av"] ) +
						                                   " (" + QString().sprintf( "%5.2f%%", frac_tot_r) + ")",
						  QString().sprintf( "%5.2f%%",  replicate_g_results["tot_percent_st_dev"] ),
						  QString::number( frac_tot_tol_r ),
						  tot_av_frac_passed
						  );
	    }
	  
	  html_str_replicate_av += indent( 3 ) + "</table>\n";
	}
    }
  
  return html_str_replicate_av;
}

//Get Replicate Group # from channel_desc_alt
QString US_ReporterGMP::get_replicate_group_number( QString ch_alt_desc )
{
  QString replicate_gn;
  
  for( int i=0; i < chndescs_alt.size(); ++i )
    {
      if( ch_alt_desc == chndescs_alt[ i ] )
	{
	  replicate_gn = QString::number ( replicates[ i ] );
	  break;
	}
    }
  qDebug() << "Replicate Group # for channel: " << ch_alt_desc << ", IS -- " << replicate_gn;
  
  return replicate_gn;
}

//Get Integration Results from same Replicate subGroup | same-wvl | same-report-items: 
QMap<QString, double> US_ReporterGMP::get_replicate_group_results( US_ReportGMP::ReportItem ref_report_item, QString u_wvl, QStringList channs_for_wvl )
{
  double int_res_sim = 0;
  double int_res_sim_av = 0;
  double tot_percent_sim = 0;
  double tot_percent_sim_av = 0;
  
  int    same_item_counter = 0;
  double st_dev_int_1 = 0;
  double st_dev_int_final = 0;
  double st_dev_tot_percent_1 = 0;
  double st_dev_tot_percent_final = 0;
  QVector< double > int_res_sim_vector;
  QVector< double > tot_percent_sim_vector;

  QMap< QString, double > results;
  
  //iterate over UR_ReportsGMPs && pick ones correspondning to 'chan_desc_alt.contains("ch_wvls[i]")' && wavelength == u_wvl;
  int nchna   = currAProf.pchans.count();
  for ( int i = 0; i < channs_for_wvl.size(); ++i )   //over channels for a given wvl in Replicate group:
    {
      US_ReportGMP reportGMP;
      for ( int j = 0; j < nchna; j++ )       //over all channels
	{
	  QString channel_desc_alt = chndescs_alt[ j ];

	  // //For now, do not consider IP type!!!
	  // if ( channel_desc_alt.contains("Interf") ) 
	  //   continue;
	  
	  //if ( channel_desc_alt.split(":")[0].contains( channs_for_wvl[ i ] ) )  
	  if ( channel_desc_alt .contains( channs_for_wvl[ i ] ) )  
	    {
	      qDebug() << "In get_replicate_group_results(): channel_desc_alt, channs_for_wvl[ i ], wvl -- "
		       << channel_desc_alt
		       << channs_for_wvl[ i ]
		       << u_wvl;

	      //Select US_ReportGMP for channel in a Replicate group && representative wvl!
	      reportGMP = ch_reports[ channel_desc_alt ][ u_wvl ];

	      break;
	    }
	}
            
      //then pick report's ReportItem corresponding to the passed ref_report_item:
      int report_items_number = reportGMP. reportItems.size();
      for ( int kk = 0; kk < report_items_number; ++kk )
	{
	   US_ReportGMP::ReportItem curr_item = reportGMP. reportItems[ kk ];

	   //Compare ref_report_item with curr_item:
	   QString ref_type           = ref_report_item.type;
	   QString ref_method         = ref_report_item.method;
	   QString ref_int_val        = QString::number( ref_report_item.integration_val );
	   double  ref_frac_tot       = ref_report_item.total_percent;
	   double  ref_frac_tot_tol   = ref_report_item.tolerance ;
	   double  ref_low            = ref_report_item.range_low;
	   double  ref_high           = ref_report_item.range_high;

	   QString _type           = curr_item.type;
	   QString _method         = curr_item.method;
	   QString _int_val        = QString::number( curr_item.integration_val );
	   double  _frac_tot       = curr_item.total_percent;
	   double  _frac_tot_tol   = curr_item.tolerance ;
	   double  _low            = curr_item.range_low;
	   double  _high           = curr_item.range_high;

	   //Comparison nased on: type/method [s-2DSA-IT], ranges [3.2 - 3.7] && integration value: CAN BE EXTENDED if needed 
	   if ( ref_type    == _type      &&
		ref_method  == _method    &&
		ref_low     == _low       &&
		ref_high    == _high      &&
		ref_int_val == _int_val      )
	     {
	       
	       //can add an average fraction percent from model (if needed)

	       qDebug() << "For Triple: " << channs_for_wvl[ i ] << "." << u_wvl
			<< ", Type/Method: " << _type << ": " << _method
			<< ", Range: " << "[" << _low << " - " << _high << "]" 
			<< ", Simulated Integr. Val: " << curr_item. integration_val_sim
			<< ", Fraction %: " << curr_item. total_percent_sim;

	       if ( curr_item. integration_val_sim >= 0 )
		 {
		   int_res_sim            += curr_item. integration_val_sim;
		   tot_percent_sim        += curr_item. total_percent_sim;
		   int_res_sim_vector     .push_back( curr_item. integration_val_sim );
		   tot_percent_sim_vector .push_back( curr_item. total_percent_sim );

		   ++same_item_counter;
		 }
	     }
	}
    }

  if ( same_item_counter )
    {
      int_res_sim_av     = double( int_res_sim / same_item_counter );
      tot_percent_sim_av = double( tot_percent_sim / same_item_counter ); 

      for( int i=0; i<int_res_sim_vector.size(); ++i )
	{
	  st_dev_int_1         += ( int_res_sim_av - int_res_sim_vector[i] ) * ( int_res_sim_av - int_res_sim_vector[i] );
	  st_dev_tot_percent_1 += ( tot_percent_sim_av - tot_percent_sim_vector[i] ) * ( tot_percent_sim_av - tot_percent_sim_vector[i] );
	}
      
      st_dev_int_final         = sqrt( st_dev_int_1 ) / (sqrt( int_res_sim_vector.size() ));  
      st_dev_tot_percent_final = sqrt( st_dev_tot_percent_1 ) / (sqrt( tot_percent_sim_vector.size() ));  
    }

  results[ "int_av" ]             = int_res_sim_av;
  results[ "tot_percent_av" ]     = tot_percent_sim_av;
  results[ "int_st_dev" ]         = st_dev_int_final;
  results[ "tot_percent_st_dev" ] = st_dev_tot_percent_final;

    
  return results;
}

// Distribution information HTML string
QString US_ReporterGMP::distrib_info( QMap < QString, QString> & tripleInfo )
{
   int  ncomp     = model_used.components.size();
   double vari_m  = model_used.variance;
   double rmsd_m  = ( vari_m == 0.0 ) ? 0.0 : sqrt( vari_m );

   qDebug() << "Distrib_info(): Model Name, time created, editUpdated -- "
	    << model.description
	    << model.timeCreated
	    << model.editDataUpdated;
   
   if ( ncomp == 0 )
      return "";

   QString msim   = adv_vals[ "modelsim" ];

   if ( is_dmga_mc )
   {

      if ( msim == "model" )
      {  // Use DMGA-MC single-iteration model
         msim           = "<b>&nbsp;&nbsp;( single iteration )</b>";
      }
      else
      {  // Use mean|median|mode model
         msim           = "<b>&nbsp;&nbsp;( " + msim + " )</b>";
      }
   }
   else
   {  // Normal non-DMGA-MC model
      msim           = "";
      if ( model_used.monteCarlo  &&
           ! model_used.description.contains( "_mcN" ) )
         msim           = "<b>&nbsp;&nbsp;( single iteration )</b>";
   }

   QString mdla = model_used.description
                  .section( ".", -2, -2 ).section( "_", 1, -1 );
   if ( mdla.isEmpty() )
      mdla         = model_used.description.section( ".", 0, -2 );

   
   //TimeStamps
   QString mstr = "\n" + indent( 2 )
                  + tr( "<h3>Timestamps:</h3>\n" )
                  + indent( 2 ) + "<table>\n";
   mstr += table_row( tr( "Data Edited at:" ), model.editDataUpdated + " (UTC)");
   mstr += table_row( tr( "Model Analysed at:" ), model.timeCreated + " (UTC)");
   mstr += indent( 2 ) + "</table>\n";
      
   //Main Analysis Settings
   mstr +=        "\n" + indent( 2 )
                  + tr( "<h3>Data Analysis Settings:</h3>\n" )
                  + indent( 2 ) + "<table>\n";

   mstr += table_row( tr( "Model Analysis:" ), mdla + msim );
   mstr += table_row( tr( "Number of Components:" ),
                      QString::number( ncomp ) );
   mstr += table_row( tr( "Residual RMS Deviation:" ),
                      rmsd_global  );
   mstr += table_row( tr( "Model-reported RMSD:"    ),
                      ( rmsd_m > 0.0 ) ? QString::number( rmsd_m ) : "(none)" );

   double sum_mw  = 0.0;
   double sum_s   = 0.0;
   double sum_D   = 0.0;
   double sum_c   = 0.0;
   double sum_v   = 0.0;
   double sum_k   = 0.0;
   double mink    = 1e+99;
   double maxk    = -1e+99;
   double minv    = 1e+99;
   double maxv    = -1e+99;

   for ( int ii = 0; ii < ncomp; ii++ )
   {
      double conc = model_used.components[ ii ].signal_concentration;
      double kval = model_used.components[ ii ].f_f0;
      double vval = model_used.components[ ii ].vbar20;
      sum_c      += conc;
      sum_mw     += ( model_used.components[ ii ].mw * conc );
      sum_s      += ( model_used.components[ ii ].s  * conc );
      sum_D      += ( model_used.components[ ii ].D  * conc );
      sum_v      += ( vval * conc );
      sum_k      += ( kval * conc );
      mink        = qMin( kval, mink );
      maxk        = qMax( kval, maxk );
      minv        = qMin( vval, minv );
      maxv        = qMax( vval, maxv );
   }

   mstr += table_row( tr( "Weight Average s20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_s  / sum_c ) ) );
   mstr += table_row( tr( "Weight Average D20,W:" ),
                      QString().sprintf( "%6.4e", ( sum_D  / sum_c ) ) );
   mstr += table_row( tr( "W.A. Molecular Weight:" ),
                      QString().sprintf( "%6.4e", ( sum_mw / sum_c ) ) );
   if ( ! cnstff )
      mstr += table_row( tr( "Weight Average f/f0:" ),
                         QString::number( ( sum_k / sum_c ) ) );
   if ( ! cnstvb )
      mstr += table_row( tr( "Weight Average vbar20:" ),
                         QString::number( ( sum_v / sum_c ) ) );
   mstr += table_row( tr( "Total Concentration:" ),
                      QString().sprintf( "%6.4e", sum_c ) );

   if ( cnstvb )
      mstr += table_row( tr( "Constant vbar20:" ),
                         QString::number( minv ) );
   else if ( cnstff )
      mstr += table_row( tr( "Constant f/f0:" ),
                         QString::number( mink ) );
   mstr += indent( 2 ) + "</table>\n";


   //Distribution Info - to separate file //////////////////////////////////////////////////
   mstr += "\n" + indent( 2 ) + tr( "<h3>Distribution Information:</h3>\n" );
   mstr += indent( 2 ) + "<table>\n";

   QString subDirName  = runName + "-run" + runID;
   QString dirName     = US_Settings::reportDir() + "/" + subDirName;
   mkdir( US_Settings::reportDir(), subDirName );

   QString model_desc_edited = model.description;
   model_desc_edited. replace(".", "_");
   
   // QString fileName_str      = dirName + "/" + model_desc_edited + "_csv.txt";
   // QString fileName_str_only = model_desc_edited + "_csv.txt";
   // QFile file_model_info(fileName_str);
   // file_model_info.open(QIODevice::WriteOnly | QIODevice::Text);
   // QTextStream out_model_info(&file_model_info);
   
   QString model_dist_info;
   QString html_model_s;
   
   if ( cnstvb )
     {  // Normal constant-vbar distribution
      model_dist_info = "Molec. Wt., S Apparent, S 20 (W), D Apparent, D 20 (W), f/f0, Concentration";

      //out_model_info << model_dist_info << endl;
      html_model_s += model_dist_info + "<br>";
      
      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         model_dist_info  =
	   QString().sprintf( "%10.4e", model_used.components[ ii ].mw ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].D  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].D  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].f_f0 ) + ", " + 
	   QString().sprintf( "%10.4e (%5.2f %%)", conc, perc );

	 //out_model_info << model_dist_info << endl;
	 html_model_s += model_dist_info + "<br>";
      }
   }

   else if ( cnstff )
   {  // Constant-f/f0, varying vbar
     model_dist_info = "Molec. Wt., S Apparent, S 20 (W), D Apparent, D 20 (W), Vbar20, Concentration";

     //out_model_info << model_dist_info << endl;
     html_model_s += model_dist_info + "<br>";

     for ( int ii = 0; ii < ncomp; ii++ )
       {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
	 model_dist_info  =
	   QString().sprintf( "%10.4e", model_used.components[ ii ].mw ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].D  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].D  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].vbar20 ) + ", " + 
	   QString().sprintf( "%10.4e (%5.2f %%)", conc, perc );

	 //out_model_info << model_dist_info << endl;
	 html_model_s += model_dist_info + "<br>";
       }
   }

   else
   {  // Neither vbar nor f/f0 are constant

      model_dist_info = "Molec. Wt., S Apparent, S 20 (W), D 20 (W), f/f0, Vbar20, Concentration";
      
      //out_model_info << model_dist_info << endl;
      html_model_s += model_dist_info + "<br>";

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
	 model_dist_info  =
	   QString().sprintf( "%10.4e", model_used.components[ ii ].mw ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].s  ) + ", " + 
	   QString().sprintf( "%10.4e", model     .components[ ii ].D  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].f_f0  ) + ", " + 
	   QString().sprintf( "%10.4e", model_used.components[ ii ].vbar20 ) + ", " + 
	   QString().sprintf( "%10.4e (%5.2f %%)", conc, perc );
	 
	 //out_model_info << model_dist_info << endl;
	 html_model_s += model_dist_info + "<br>";
      }
   }
   
   //file_model_info.close();

   //also, create a .pdf
   QString f_model_path = dirName + "/" + model_desc_edited + ".pdf";
   QString f_model_path_str_only = model_desc_edited + ".pdf";
   //QString html_model_s = model_dist_info;
   QTextDocument document;
   document.setHtml( html_model_s );
  
   QPrinter printer(QPrinter::PrinterResolution);
   printer.setOutputFormat(QPrinter::PdfFormat);
   printer.setPaperSize(QPrinter::Letter);
   
   printer.setOutputFileName( f_model_path );
   printer.setFullPage(true);
   printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
  
   document.print(&printer);
  
   //mstr += "<a href=\"./" + fileName_str_only + "\">View Model Distributions</a>";
   //mstr += "<br>";
   mstr += "<a href=\"./" + f_model_path_str_only + "\">View Model Distributions</a>";
   
   /*
   mstr += "\n" + indent( 2 ) + tr( "<h3>Distribution Information:</h3>\n" );
   mstr += indent( 2 ) + "<table>\n";

   if ( cnstvb )
   {  // Normal constant-vbar distribution
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "f / f0" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw   ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].s    ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D    ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].f_f0 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   else if ( cnstff )
   {  // Constant-f/f0, varying vbar
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D Apparent" ),
                         tr( "D 20,W" ),     tr( "Vbar20" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw     ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].vbar20 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }

   else
   {  // Neither vbar nor f/f0 are constant
      mstr += table_row( tr( "Molec. Wt." ), tr( "S Apparent" ),
                         tr( "S 20,W" ),     tr( "D 20,W"     ),
                         tr( "f / f0" ),     tr( "Vbar20" ),
                         tr( "Concentration" ) );

      for ( int ii = 0; ii < ncomp; ii++ )
      {
         double conc = model_used.components[ ii ].signal_concentration;
         double perc = 100.0 * conc / sum_c;
         mstr       += table_row(
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].mw     ),
               QString().sprintf( "%10.4e",
                  model       .components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                 model_used.components[ ii ].s      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].D      ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].f_f0 ),
               QString().sprintf( "%10.4e",
                  model_used.components[ ii ].vbar20 ),
               QString().sprintf( "%10.4e (%5.2f %%)", conc, perc ) );
      }
   }
   */
   // END OF Distribution Info //////////////////////////////////////////////////////////////////////
   
   
   // Show associations information if present
   if ( model_used.associations.size() > 0 )
   {
      mstr += indent( 2 ) + "</table>\n" + indent( 2 );
      mstr += tr( "<h3>Reversible Associations Information:</h3>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Reactant 1" ), tr( "Reactant 2" ),
                         tr( "Product"    ), tr( "K_dissociation"  ),
                         tr( "k_off Rate" ) );

      for ( int ii = 0; ii < model_used.associations.size(); ii++ )
      {
         US_Model::Association as1 = model.associations[ ii ];
         double k_d    = as1.k_d;
         double k_off  = as1.k_off;
         QString reac1 = tr( "component %1" ).arg( as1.rcomps[ 0 ] + 1 );
         QString reac2 = tr( "(none)" );
         QString prod  = tr( "component %1" ).arg( as1.rcomps[ 1 ] + 1 );
         if ( as1.rcomps.size() > 2 )
         {
            reac2         = prod;
            prod          = tr( "component %1" ).arg( as1.rcomps[ 2 ] + 1 );
         }

         mstr       += table_row( reac1, reac2, prod,
                                  QString().sprintf( "%10.4e", k_d   ),
                                  QString().sprintf( "%10.4e", k_off ) );
      }
   }

   mstr += indent( 2 ) + "</table>\n";


   //Now add Basic Report params comparison to actual Run Details
   QString t_name = model_used.description.split(".")[1];
   QString wvl    = t_name.mid(2,3);
   QString cellID = t_name.mid(0,1);
   qDebug() << "In DISTRIB: triple, cellID, wvl -- " << t_name << cellID << wvl;

   qDebug() << "Model description, mdla, msim -- " << model_used.description <<  mdla << msim;
   QString model_name = mdla.split("_")[1];
   if ( model_name.contains("PCSA") )
     model_name = "PCSA";
   
   double tot_conc_r, tot_conc_r_factor, tot_conc_tol_r, rmsd_r, av_int_r, exp_dur_r, exp_dur_tol_r;
   double loading_volume_r, loading_volume_tol_r ;

   //length of run (based on rdata - for current triple )
   double last_f  = rdata ->scanData.last().seconds;
   double first_f = rdata ->scanData.first().seconds;
   int  hours     = (int)qFloor( last_f / 3600.0 );
   int  mins      = (int)qRound( ( last_f - hours * 3600.0 ) / 60.0 );
   QString hh     = ( hours == 1 ) ? tr( "hr" ) : tr( "hrs" );
   hh    = "h";
   QString wks    = QString().sprintf( "%d %s %02d m", hours, hh.toLatin1().data(), mins );
   int fmins     = (int)qFloor( first_f / 60.0 );
   int fsecs     = first_f - fmins * 60.0;
   QString mm    = ( fmins == 1 ) ? tr( "min" ) : tr( "mins" );
   QString scan1time = QString().sprintf( "%d m %02d s", fmins, fsecs );
   //end of length
   
   US_ReportGMP* reportGMP;  //reference
   int nchna   = currAProf.pchans.count();
   for ( int i = 0; i < nchna; i++ )
     {
       QString channel_desc_alt = chndescs_alt[ i ];

       qDebug() << "Identifying report for triple -- " << t_name   // 2A660
		<< ", tripleInfo:  "                   << tripleInfo[ "triple_name" ]                     // 2AInterference
		<< ", channel_desc_alt: "              << channel_desc_alt;

       if ( tripleInfo[ "triple_name" ].contains("Interference") && !channel_desc_alt.contains("Interf") )
	 continue;

       //'S' data
       QString channelNameProt = channel_desc_alt.split(":")[0];
       if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
	 channelNameProt = channelNameProt. replace( "A", "S" );
       
       if ( t_name. contains( channelNameProt ) )  
	 {
	   qDebug() << "So, what are channel_desc_alt, wvl ? " << channel_desc_alt << wvl;
	     
	   reportGMP = &( ch_reports[ channel_desc_alt ][ wvl ] );

	   tot_conc_r     = reportGMP-> tot_conc ;
	   tot_conc_tol_r = reportGMP-> tot_conc_tol ;
	   rmsd_r         = reportGMP-> rmsd_limit ;
	   av_int_r       = reportGMP-> av_intensity ;
	   exp_dur_r      = reportGMP-> experiment_duration ;
	   exp_dur_tol_r  = reportGMP-> experiment_duration_tol ;

	   tot_conc_r_factor    = currAProf. lc_ratios [ i ] ;
	   loading_volume_r     = currAProf. l_volumes [ i ] ;
	   loading_volume_tol_r = currAProf. lv_tolers [ i ] ;
	   
	   break;
	 }
     }
   //transform exp_duration_report into hh mm format
   int  hours_r     = (int)qFloor( exp_dur_r / 3600.0 );
   int  mins_r      = (int)qRound( ( exp_dur_r - hours_r * 3600.0 ) / 60.0 );
   QString hh_r     = ( hours_r == 1 ) ? tr( "hr" ) : tr( "hrs" );
   hh_r    = "h";
   QString exp_dur_r_hh_mm    = QString().sprintf( "%d %s %02d m", hours_r, hh_r.toLatin1().data(), mins_r );
   //end of exp_dur_r transformation

   //'S' data
   if ( dataSource .contains("DiskAUC:Absorbance") && simulatedData )
     t_name = t_name. replace("S","A");
   
   //autoflowIntensity (for specific wvl)
   double av_int_exp;
   QList< QString > intensity_keys = intensityRIMap.keys();
   if ( intensity_keys.size() ==  1 ) //single wvl
     {
       av_int_exp = intensityRIMap.first().toDouble();
       qDebug() << "single wvl: key, value -- " << intensityRIMap.firstKey() << intensityRIMap.first();
     }
   else //MWL
     {
       QMap<QString, QString >::iterator jj;
       for ( jj = intensityRIMap.begin(); jj != intensityRIMap.end(); ++jj )
	 {
	   if ( t_name.contains( jj.key() ) )
	     {
	       av_int_exp = jj.value().toDouble();
	       break;
	     }
	 }
     }
   //

   //Caclulate loading volume based on centerpiece geomentry && fitted meniscus
   //double loading_volume_measured = 451;
   double loading_volume_measured = get_loading_volume( cellID.toInt() );

   //passes
   tot_conc_r *= tot_conc_r_factor;
   QString tot_conc_passed = ( sum_c  >= ( tot_conc_r * (1 - tot_conc_tol_r/100.0)  ) && sum_c  <= ( tot_conc_r * (1 + tot_conc_tol_r/100.0) ) ) ? "YES" : "NO";
   QString exp_dur_passed  = ( last_f >= ( exp_dur_r * (1 - exp_dur_tol_r/100.0)  )   && last_f <= ( exp_dur_r * (1 + exp_dur_tol_r/100.0) ) ) ? "YES" : "NO";
   QString rmsd_passed = ( rmsd_global.toDouble() <= rmsd_r ) ? "YES" : "NO";
   QString av_int_passed = ( av_int_exp > av_int_r ) ? "YES" : "NO";
   QString loading_volume_passed  = ( loading_volume_measured >= ( loading_volume_r * (1 - loading_volume_tol_r/100.0)  )
				      && loading_volume_measured <= ( loading_volume_r * (1 + loading_volume_tol_r/100.0) ) ) ? "YES" : "NO";
   //end passes

   //check what to show on the report
   if ( tripleInfo[ "triple_name" ].contains("Interference") )
     t_name.replace("660", "Interference");

   qDebug() << "[XXXXXX] t_name, model_name: " << t_name <<  model_name;
   
   QMap < QString, QString > Model_parms_to_compare = perChanMask_edited.ShowTripleModelParts[ t_name ][ model_name ]; //2A660 => 2AInterference, 
   bool show_tot_conc = false;
   bool show_rmsd     = false;
   bool show_exp_dur  = false;
   bool show_min_int  = false;
   bool show_integration = false;
   bool show_loading_vol = false;

   QMap< QString, QString >::iterator sh;
   for ( sh = Model_parms_to_compare.begin(); sh != Model_parms_to_compare.end(); ++sh )
     {
       if ( sh.key().contains( "Concentration" ) && sh.value().toInt() )
	 show_tot_conc = true;

       if ( sh.key().contains( "RMSD" ) && sh.value().toInt() )
	 show_rmsd = true;

       if ( sh.key().contains( "Duration" ) && sh.value().toInt() )
	 show_exp_dur = true;

       if ( sh.key().contains( "Intensity" ) && sh.value().toInt() )
	 show_min_int = true;
       
       if ( sh.key().contains( "Loading" ) && sh.value().toInt() )
	 show_loading_vol = true;
       
       if ( sh.key().contains( "Integration" ) && sh.value().toInt() )
	 show_integration = true;
     }

   bool show_comparison_section = false;
   if ( show_tot_conc || show_rmsd || show_exp_dur || show_min_int || show_loading_vol )
     show_comparison_section = true;
   //////////////////////////////////////

   qDebug() << "XXXXX: show_comparison_section -- " << show_comparison_section;
   
   if ( show_comparison_section )
     {
       mstr += "\n" + indent( 2 ) + tr( "<h3>Comparison Between Run/Simulation Results and Report Parameters:</h3>\n" );
       mstr += indent( 2 ) + "<table>\n";
       
       mstr += table_row( tr( "Parameter: " ),
			  tr( "Target Value:" ),
			  tr( "Tolerance, %:"),
			  tr( "Measured Value:" ),
			  tr( "PASSED ?" ));

       if ( show_tot_conc ) 
	 {
	   mstr += table_row( tr( "Total Concentration" ),
			      QString::number( tot_conc_r ) + " (factor " + QString::number( tot_conc_r_factor ) + ")",
			      QString::number(tot_conc_tol_r) + "%",
			      QString().sprintf( "%6.4e", sum_c ),
			      tot_conc_passed) ;
	 }
       
       if ( show_rmsd )
	 {
	   mstr += table_row( tr( "RMSD limit" ),
			      QString::number( rmsd_r ),
			      QString(""),
			      rmsd_global,
			      rmsd_passed );
	 }
       
       if ( show_exp_dur )
	 {
	   mstr += table_row( tr( "Experiment Duration" ),
			      exp_dur_r_hh_mm,
			      QString::number(exp_dur_tol_r) + "%",
			      wks,
			      exp_dur_passed ) ;
	 }

       // Show only if not Absorbance || not Interference
       qDebug() << "Show_INTENSITY: dataSource -- " << dataSource;
       qDebug() << "Show_INTENSITY: tripleInfo[ \"triple_name\" ] -- " << tripleInfo[ "triple_name" ];
       bool RIdata = ( dataSource. contains( "DiskAUC:Absorbance" ) || tripleInfo[ "triple_name" ].contains("Interference") ) ?
	 false : true;
       qDebug() << "Show_INTENSITY: RIdata ? " << RIdata;
       if ( show_min_int && RIdata )  
	 {
	   mstr += table_row( tr( "Minimum Intensity" ),
			      QString::number( av_int_r ),
			      QString(""),
			      QString::number( av_int_exp ) ,
			      av_int_passed );
	 }

       if ( show_loading_vol ) 
	 {
	   mstr += table_row( tr( "Loading Volume" ),
			      QString::number( loading_volume_r ),
			      QString::number( loading_volume_tol_r) + "%",
			      QString::number( loading_volume_measured ) ,
			      loading_volume_passed );
	 }

       mstr += indent( 2 ) + "</table>\n";
     }

   //Now, integration results
   //1st, check if for a given model (method) there are reportItems:
   bool method_type_combo_exists = false;
   int report_items_number = reportGMP-> reportItems.size();
   for ( int kk = 0; kk < report_items_number; ++kk )
     {
       US_ReportGMP::ReportItem curr_item = reportGMP-> reportItems[ kk ];
       QString method         = curr_item.method;
       if( method.contains ("PCSA") )
	 method = "PCSA";

       if ( mdla.contains ( method ) )
	 method_type_combo_exists = true;
     }
   
   if ( show_integration && method_type_combo_exists )
     {
       mstr += "\n" + indent( 2 ) + tr( "<h3>Integration Results: Fraction of Total Concentration:</h3>\n" );
       mstr += indent( 2 ) + "<table>\n";
       mstr += table_row( tr( "Type:" ),
			  tr( "Range:"),
			  tr( "Integration from Model (target):" ),
			  tr( "Fraction % from Model (target):" ),
			  tr( "Tolerance, %:"),
			  tr( "PASSED ?" ));
       for ( int kk = 0; kk < report_items_number; ++kk )
	 {
	   US_ReportGMP::ReportItem curr_item = reportGMP-> reportItems[ kk ];
	   QString type           = curr_item.type;
	   QString method         = curr_item.method;
	   if( method.contains ("PCSA") )
	     method = "PCSA";
	   
	   QString int_val_r      = QString::number( curr_item.integration_val );
	   double  frac_tot_r     = curr_item.total_percent;
	   double  frac_tot_tol_r = curr_item.tolerance ;
	   double  low            = curr_item.range_low;
	   double  high           = curr_item.range_high;
	   
	   QString range = "[" + QString::number(low) + " - " + QString::number(high) + "]";
	   
	   //integrate over model_used
	   double int_val_m = 0;
	   for ( int ii = 0; ii < ncomp; ii++ )
	     {
	       double conc = model_used.components[ ii ].signal_concentration;
	       double s_20 = model_used.components[ ii ].s;
	       double D_20 = model_used.components[ ii ].D;
	       double f_f0 = model_used.components[ ii ].f_f0;
	       double mw   = model_used.components[ ii ].mw;
	       
	       if ( type == "s" )
		 {
		   if ( s_20 >= low*pow(10,-13) && s_20 <= high*pow(10,-13) )
		     int_val_m += conc;
		 }
	       else if ( type == "D" )
		 {
		   if ( D_20 >= low*pow(10,-7) && D_20 <= high*pow(10,-7) )
		     int_val_m += conc;
		 }
	       else if ( type == "f/f0")
		 {
		   if ( f_f0 >= low && f_f0 <= high )
		     int_val_m += conc;
		 }
	       else if ( type == "MW")
		 {
		   if ( mw >= low*pow(10,3) && mw <= high*pow(10,3) )
		     int_val_m += conc;
		 }
	     }
	   
	   double frac_tot_m = double( int_val_m / sum_c ) * 100.0;
	   // QString tot_frac_passed = ( frac_tot_m >= ( frac_tot_r * (1 - frac_tot_tol_r/100.0)  )
	   // 			       && frac_tot_m <= ( frac_tot_r * (1 + frac_tot_tol_r/100.0)  ) ) ? "YES" : "NO";

	   QString tot_frac_passed = ( qAbs( frac_tot_m - frac_tot_r ) <= frac_tot_tol_r ) ? "YES" : "NO";
	   
	   if ( mdla.contains ( method ) )
	     {
	       // curr_item. integration_val_sim = int_val_m;
	       // curr_item. total_percent_sim   = frac_tot_m;
	       // curr_item. passed              = tot_frac_passed;

	       reportGMP-> reportItems[ kk ]. integration_val_sim = int_val_m;
	       reportGMP-> reportItems[ kk ]. total_percent_sim   = frac_tot_m;
	       reportGMP-> reportItems[ kk ]. passed              = tot_frac_passed;

	       qDebug() << "In distrib_info(), fill simulated integration vals: for chann/wvl/type/method/low/high, "
			<< "Inter. val. Sim -- "
			<< wvl
			<< curr_item.type
			<< curr_item.method
			<< curr_item.range_low
			<< curr_item.range_high
			<< int_val_m;
	       
	       mstr += table_row( type,
				  range,
				  QString().sprintf( "%10.4e", int_val_m) + " (" + int_val_r + ")",
				  QString().sprintf( "%5.2f%%", frac_tot_m ) + " (" + QString::number( frac_tot_r ) + "%)",
				  QString::number( frac_tot_tol_r ),
				  tot_frac_passed );
	     }

	 }
       mstr += indent( 2 ) + "</table>\n";
     }
   //End of integration results

   /*
   if ( is_dmga_mc )
   {  // Compute DMGA-MC statistics and add them to the report
      QVector< double >             rstats;
      QVector< QVector< double > >  mstats;
      int niters     = imodels.size();
      int ncomp      = imodels[ 0 ].components  .size();
      int nreac      = imodels[ 0 ].associations.size();

      // Build RMSD statistics across iterations
      US_DmgaMcStats::build_rmsd_stats( niters, imodels, rstats );

      // Build statistics across iterations
      int ntatt = US_DmgaMcStats::build_model_stats( niters, imodels, mstats );

      // Compose the summary statistics chart
      mstr += indent( 2 );
      mstr += tr( "<h3>Discrete Model GA-MC Summary Statistics:</h3>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Component" ), tr( "Attribute" ),
                         tr( "Mean_Value" ), tr( "95%_Confidence(low)"  ),
                         tr( "95%_Confidence(high)" ) );
      int kd         = 0;
      QString fixd   = tr( "(Fixed)" );
      QString blnk( "" );
      QStringList atitl;
      QStringList rtitl;
      atitl << tr( "Concentration" )
            << tr( "Vbar20" )
            << tr( "Molecular Weight" )
            << tr( "Sedimentation Coefficient" )
            << tr( "Diffusion Coefficient" )
            << tr( "Frictional Ratio" );
      rtitl << tr( "K_dissociation" )
            << tr( "K_off Rate" );

      // Show summary of RMSDs
      mstr += table_row( tr( "(All)" ), tr( "RMSD" ),
                         QString().sprintf( "%10.4e", rstats[  2 ] ),
                         QString().sprintf( "%10.4e", rstats[  9 ] ),
                         QString().sprintf( "%10.4e", rstats[ 10 ] ) );

      // Show summary of component attributes
      for ( int ii = 0; ii < ncomp; ii++ )
      {
         QString compnum = QString().sprintf( "%2d", ii + 1 );
         for ( int jj = 0; jj < 6; jj++ )
         {
            bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
            QString strclo  = is_fixed ? fixd :
                              QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
            QString strchi  = is_fixed ? blnk :
                              QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
            mstr += table_row( compnum, atitl[ jj ],
                              QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                              strclo, strchi );
            kd++;
         }
      }

      mstr += indent( 2 ) + "</table>\n";
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Reaction" ), tr( "Attribute" ),
                         tr( "Mean_Value" ), tr( "95%_Confidence(low)"  ),
                         tr( "95%_Confidence(high)" ) );
      // Show summary of reaction attributes;
      for ( int ii = 0; ii < nreac; ii++ )
      {
         QString reacnum = QString().sprintf( "%2d", ii + 1 );
         bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
         QString strclo  = is_fixed ? fixd :
                            QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
         QString strchi  = is_fixed ? blnk :
                            QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
         mstr += table_row( reacnum, tr( "K_dissociation" ),
                            QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                            strclo, strchi );
         kd++;
         is_fixed        = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );
         strclo          = is_fixed ? fixd :
                            QString().sprintf( "%10.4e", mstats[ kd ][  9 ] );
         strchi          = is_fixed ? blnk :
                            QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] );
         mstr += table_row( reacnum, tr( "K_off Rate" ),
                            QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ),
                            strclo, strchi );
         kd++;
      }

      mstr += indent( 2 ) + "</table>\n";

      // Compose the details statistics entries
      mstr += indent( 2 );
      mstr += tr( "<h3>Discrete Model GA-MC Detailed Statistics:</h3>\n" );
      int icomp       = 1;
      int ireac       = 1;
      int kdmax       = 6;
      int kk          = 0;

      // First, the RMSDs
      mstr += indent( 2 ) + tr( "<h4>Details for MC Iteration RMSDs:</h4>\n" );
      mstr += indent( 2 ) + "<table>\n";
      mstr += table_row( tr( "Minimum:" ),
              QString().sprintf( "%10.4e", rstats[  0 ] ) );
      mstr += table_row( tr( "Maximum:" ),
              QString().sprintf( "%10.4e", rstats[  1 ] ) );
      mstr += table_row( tr( "Mean:" ),
              QString().sprintf( "%10.4e", rstats[  2 ] ) );
      mstr += table_row( tr( "Median:" ),
              QString().sprintf( "%10.4e", rstats[  3 ] ) );
      mstr += table_row( tr( "Skew:" ),
              QString().sprintf( "%10.4e", rstats[  4 ] ) );
      mstr += table_row( tr( "Kurtosis:" ),
              QString().sprintf( "%10.4e", rstats[  5 ] ) );
      mstr += table_row( tr( "Lower Mode:" ),
              QString().sprintf( "%10.4e", rstats[  6 ] ) );
      mstr += table_row( tr( "Upper Mode:" ),
              QString().sprintf( "%10.4e", rstats[  7 ] ) );
      mstr += table_row( tr( "Mode Center:" ),
              QString().sprintf( "%10.4e", rstats[  8 ] ) );
      mstr += table_row( tr( "95% Confidence Interval Low:" ),
              QString().sprintf( "%10.4e", rstats[  9 ] ) );
      mstr += table_row( tr( "95% Confidence Interval High:" ),
              QString().sprintf( "%10.4e", rstats[ 10 ] ) );
      mstr += table_row( tr( "99% Confidence Interval Low:" ),
              QString().sprintf( "%10.4e", rstats[ 11 ] ) );
      mstr += table_row( tr( "99% Confidence Interval High:" ),
              QString().sprintf( "%10.4e", rstats[ 12 ] ) );
      mstr += table_row( tr( "Standard Deviation:" ),
              QString().sprintf( "%10.4e", rstats[ 13 ] ) );
      mstr += table_row( tr( "Standard Error:" ),
              QString().sprintf( "%10.4e", rstats[ 14 ] ) );
      mstr += table_row( tr( "Variance:" ),
              QString().sprintf( "%10.4e", rstats[ 15 ] ) );
      mstr += table_row( tr( "Correlation Coefficient:" ),
              QString().sprintf( "%10.4e", rstats[ 16 ] ) );
      mstr += table_row( tr( "Number of Bins:" ),
              QString().sprintf( "%10.0f", rstats[ 17 ] ) );
      mstr += table_row( tr( "Distribution Area:" ),
              QString().sprintf( "%10.4e", rstats[ 18 ] ) );
      mstr += table_row( tr( "95% Confidence Limit Low:" ),
              QString().sprintf( "%10.4e", rstats[ 19 ] ) );
      mstr += table_row( tr( "95% Confidence Limit High:" ),
              QString().sprintf( "%10.4e", rstats[ 20 ] ) );
      mstr += table_row( tr( "99% Confidence Limit Low:" ),
              QString().sprintf( "%10.4e", rstats[ 21 ] ) );
      mstr += table_row( tr( "99% Confidence Limit High:" ),
              QString().sprintf( "%10.4e", rstats[ 22 ] ) );
      mstr += indent( 2 ) + "</table>\n";

      // Then, components and attributes
      for ( kd = 0; kd < ntatt; kd++ )
      {
         QString compnum = tr( "Component %1 " ).arg( icomp );
         QString reacnum = tr( "Reaction %1 "  ).arg( ireac );
         QString attrib  = compnum + atitl[ kk ];
         mstr += indent( 2 ) + "<h4>" + tr( "Details for " );

         if ( icomp <= ncomp )
         {  // Title for component detail
            if ( imodels[ 0 ].is_product( icomp - 1 )  &&  kk == 0 )
               mstr += compnum + tr( "(Product) Total Concentration" )
                     + ":</h4>\n";
            else
               mstr += compnum + atitl[ kk ] + ":</h4>\n";
         }
         else
         {  // Title for reaction detail
            mstr += reacnum + rtitl[ kk ] + ":</h4>\n";
         }

         mstr += indent( 2 ) + "<table>\n";
         bool is_fixed   = ( mstats[ kd ][ 0 ] == mstats[ kd ][ 1 ] );

         if ( is_fixed )
         {  // Fixed has limited lines
            mstr += table_row( tr( "Minimum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  0 ] ) );
            mstr += table_row( tr( "Maximum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  1 ] ) );
            mstr += table_row( tr( "Mean:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ) );
            mstr += table_row( tr( "Median (Fixed)" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  3 ] ) );
         }

         else
         {  // Float has full set of statistics details
            mstr += table_row( tr( "Minimum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  0 ] ) );
            mstr += table_row( tr( "Maximum:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  1 ] ) );
            mstr += table_row( tr( "Mean:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  2 ] ) );
            mstr += table_row( tr( "Median:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  3 ] ) );
            mstr += table_row( tr( "Skew:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  4 ] ) );
            mstr += table_row( tr( "Kurtosis:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  5 ] ) );
            mstr += table_row( tr( "Lower Mode:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  6 ] ) );
            mstr += table_row( tr( "Upper Mode:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  7 ] ) );
            mstr += table_row( tr( "Mode Center:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  8 ] ) );
            mstr += table_row( tr( "95% Confidence Interval Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][  9 ] ) );
            mstr += table_row( tr( "95% Confidence Interval High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 10 ] ) );
            mstr += table_row( tr( "99% Confidence Interval Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 11 ] ) );
            mstr += table_row( tr( "99% Confidence Interval High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 12 ] ) );
            mstr += table_row( tr( "Standard Deviation:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 13 ] ) );
            mstr += table_row( tr( "Standard Error:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 14 ] ) );
            mstr += table_row( tr( "Variance:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 15 ] ) );
            mstr += table_row( tr( "Correlation Coefficient:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 16 ] ) );
            mstr += table_row( tr( "Number of Bins:" ),
                    QString().sprintf( "%10.0f", mstats[ kd ][ 17 ] ) );
            mstr += table_row( tr( "Distribution Area:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 18 ] ) );
            mstr += table_row( tr( "95% Confidence Limit Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 19 ] ) );
            mstr += table_row( tr( "95% Confidence Limit High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 20 ] ) );
            mstr += table_row( tr( "99% Confidence Limit Low:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 21 ] ) );
            mstr += table_row( tr( "99% Confidence Limit High:" ),
                    QString().sprintf( "%10.4e", mstats[ kd ][ 22 ] ) );
         }

         mstr += indent( 2 ) + "</table>\n";

         if ( (++kk) >= kdmax )
         {
            kk              = 0;
            icomp++;
            if ( icomp > ncomp )
            {
               ireac           = icomp - ncomp;
               kdmax           = 2;
            }
         }
      }
   }
   */

   return mstr;
}

double  US_ReporterGMP::get_loading_volume( int cellID )
{
  double vol = 0;
  
  double speed   =  currProto. rpSpeed. ssteps[0].speed ;
  double stretch = simparams.rotorcoeffs[ 0 ] * speed + simparams.rotorcoeffs[ 1 ] * sq( speed ) ;

  qDebug() << "Centerpeice: stretch: " << stretch;

  
  //get centerpiece info
  QString centerpiece_name = "";
  for ( int i=0; i < currProto. rpCells. used. size(); ++i )
    {
      if ( currProto. rpCells. used [ i ]. cell == cellID )
	{
	  centerpiece_name = currProto. rpCells. used [ i ]. centerpiece; 
	  qDebug() << "In get_loading_volume(): cell, cenerpiece -- "
		   <<  currProto. rpCells. used [ i ]. cell
		   <<  currProto. rpCells. used [ i ]. centerpiece ;

	  break;
	}
    }
  
  // Check DB connection
  US_Passwd pw;
  QString masterPW = pw.getPasswd();
  US_DB2 db( masterPW );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db.lastError() );
      return vol;
    }
  
  //Find centerpieceID on DB based on centerpiece name:
  QString centerpieceID_read;
  QStringList query;
  query.clear();
  query << "get_abstractCenterpiece_names";
  db.query( query );
  
  while ( db.next() )
    {
      QString id   =  db.value( 0 ).toString();
      QString name =  db.value( 1 ).toString();
      
      if ( centerpiece_name == name )
	{
	  centerpieceID_read = id;
	  break;
	}
    }

  if ( centerpieceID_read.isEmpty() )
    {
      qDebug() << "In get_loading_volume(): Couldn't read centerpiece name ! ";

      return vol;
    }
  
  //Now, get centerpiece info
  query.clear();
  query << "get_abstractCenterpiece_info" << centerpieceID_read;
  db.query( query );
  db.next();
  
  QString c_name       = db.value( 1 ).toString();
  double  c_bottom     = db.value( 3 ).toString().toDouble();
  double  c_pathlength = db.value( 6 ).toString().toDouble();
  double  c_angle      = db.value( 7 ).toString().toDouble();

  qDebug() << "In get_loading_volume(): c_name, c_bottom, c_pathlength, c_angle -- "
	   << c_name << c_bottom << c_pathlength << c_angle;

  //add stretching
  c_bottom += stretch;

  //meniscus position
  double meniscus = model_used. meniscus;
  qDebug() << "In get_loading_volume(): meniscus -- " << meniscus;
  
  //calulate loading volume
  vol = ( c_bottom*c_bottom - meniscus*meniscus ) * ( 1000*c_pathlength*c_angle*M_PI ) / 360.0;
  
  return vol;
}


// String to accomplish line indentation
QString  US_ReporterGMP::indent( const int spaces ) const
{
   return ( QString( " " ).leftJustified( spaces, ' ' ) );
}

// Table row HTML with 2 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2 ) const
{
   return( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td></tr>\n" );
}

// Table row HTML with 3 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td></tr>\n" );
}

// Table row HTML with 4 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td><td>" + s4 + "</td></tr>\n" );
}

// Table row HTML with 5 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>" + s3
            + "</td><td>" + s4 + "</td><td>" + s5 + "</td></tr>\n" );
}

// Table row HTML with 6 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5, const QString& s6 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>\n" );
}

// Table row HTML with 7 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
                               const QString& s3, const QString& s4,
                               const QString& s5, const QString& s6,
                               const QString& s7 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td></tr>\n" );
}

// Table row HTML with 8 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
				    const QString& s3, const QString& s4,
				    const QString& s5, const QString& s6,
				    const QString& s7, const QString& s8 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td><td>" + s8 + "</td></tr>\n" );
}

// Table row HTML with 9 columns
QString  US_ReporterGMP::table_row( const QString& s1, const QString& s2,
				    const QString& s3, const QString& s4,
				    const QString& s5, const QString& s6,
				    const QString& s7, const QString& s8,
				    const QString& s9 ) const
{
   return ( indent( 6 ) + "<tr><td>" + s1 + "</td><td>" + s2 + "</td><td>"
            + s3 + "</td><td>" + s4 + "</td><td>" + s5 + "</td><td>"
            + s6 + "</td><td>" + s7 + "</td><td>" + s8 + "</td><td>" + s9 + "</td></tr>\n" );
}


// Open a residual plot dialog
void US_ReporterGMP::plotres( QMap < QString, QString> & tripleInfo )
{

  QString t_name = tripleInfo[ "triple_name" ];
  t_name.replace(".", "");

  //'S' data
  if ( dataSource. contains("DiskAUC:Absorbance") && simulatedData )
    t_name = t_name. replace( "S", "A" );

  qDebug() << "[in plotres() ]: t_name -- " << t_name;
  
  QString s_name = tripleInfo[ "stage_name" ] ;

  //bool show_3d   = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "3D Model Plot" ].toInt() ) ? true : false ;
  bool show_3d   = false;
  bool show_d_mw = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "D-vs-MW 2D Model" ].toInt() ) ? true : false ; 
  bool show_d_s  = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "D-vs-s 2D Model" ].toInt() ) ? true : false ; 
  bool show_dd   = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "Diffusion Coefficient Distribution" ].toInt() ) ? true : false ;
  bool show_res  = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "Experiment-Simulation Residuals" ].toInt() ) ? true : false ; ;
  bool show_vel  = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "Experiment-Simulation Velocity Data (noise corrected)" ].toInt() ) ? true : false ;
  bool show_mw   = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "Molecular Weight Distribution" ].toInt() ) ? true : false ;
  bool show_sd   = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "Sedimentation Coefficient Distribution" ].toInt() ) ? true : false ;
  bool show_f_mw = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "f/f0-vs-MW 2D Model" ] .toInt() ) ? true : false ;
  bool show_f_s  = ( perChanMask_edited. ShowTripleModelPlotParts[ t_name ][ s_name ][ "f/f0-vs-s 2D Model" ].toInt() ) ? true : false ;

  // // Check if to plot individual Combined distributons:
  // // {s,D,f/f0,MW,Radius} {2DSA-IT,2DSA-MC,PCSA,raw} {p_key: {s[3.2:3.7], D[11:15], etc.} }
  // // MaskStr.ShowTripleTypeModelRangeIndividualCombo[ t_name ][ s_name ][ p_key ] = feature_indCombo_value; // 1/0
  // bool show_ind_combo_s;
  // bool show_ind_combo_D;
  // bool show_ind_combo_f_f0;
  // bool show_ind_combo_MW;
  // bool show_ind_combo_Radius;

   
  //Debug
  qDebug() << "triple_name, stage_n: "
	   << tripleInfo[ "triple_name" ]
	   << tripleInfo[ "stage_name" ] ;

  qDebug() << "show_3d: "   << show_3d;
  qDebug() << "show_d_mw: " << show_d_mw; 
  qDebug() << "show_d_s: "  << show_d_s;  
  qDebug() << "show_dd: "   << show_dd ;  
  qDebug() << "show_res: "  << show_res;  
  qDebug() << "show_vel: "  << show_vel;  
  qDebug() << "show_mw: "   << show_mw;   
  qDebug() << "show_sd: "   << show_sd;   
  qDebug() << "show_f_mw: " << show_f_mw; 
  qDebug() << "show_f_s: "  << show_f_s; 
  //End Debug

  
  
  QStringList PlotsFileNames;
  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
  //mkdir( US_Settings::reportDir(), edata->runID );
  const QString svgext( ".svgz" );
  const QString pngext( ".png" );
  const QString csvext( ".csv" );

  ////TEMP -- REVERT back after test!!!
  //const QString pngext( ".svgz" );
  
  QString tripnode  = QString( currentTripleName ).replace( ".", "" );
  QString basename  = dirName + "/" + text_model( model, 0 ) + "." + tripnode + "."; 
  //QString basename  = US_Settings::reportDir() + "/" + edata->runID + "/" + text_model( model, 0 ) + "." + tripnode + ".";

  QString img01File = basename + "velocity_nc" + svgext;
  QString img02File = basename + "residuals"   + pngext;
  QString img03File = basename + "s_distrib"   + svgext;
  QString img04File = basename + "mw_distrib"  + svgext;
  QString img05File = basename + "D_distrib"   + svgext;
  QString img06File = basename + "ff0_vs_s"    + svgext;
  QString img07File = basename + "ff0_vs_mw"   + svgext;
  QString img08File = basename + "D_vs_s"      + svgext;
  QString img09File = basename + "D_vs_mw"     + svgext;
  QString img10File = basename + "3dplot"      + pngext;

  // Plots for Exp-Sim ovelray (with noises rmoved && residual plot)
  resplotd = new US_ResidPlotFem( this, "REPORT" );

  if ( show_vel )
    {
      write_plot( img01File, resplotd->rp_data_plot1() );  //<-- rp_data_plot1() gives overlay (Exp/Simulations) plot
      PlotsFileNames <<  basename + "velocity_nc"   + pngext;
    }

  if ( show_res )
    {
      write_plot( img02File, resplotd->rp_data_plot2() );  //<-- rp_data_plot2() gives residuals plot
      PlotsFileNames << img02File;
    }
      
  //Stick Plots for S-, MW-, D- distributions
  plotLayout1 = new US_Plot( data_plot1,
			     tr( "Residuals" ),
			     tr( "Radius (cm)" ),
			     tr( "OD Difference" ),
			     true, "^resids [0-9].*", "rainbow" );
  
  plotLayout2 = new US_Plot( data_plot2,
			     tr( "Velocity Data" ),
			     tr( "Radius (cm)" ),
			     tr( "Absorbance" ),
			     true, ".*in range", "rainbow" );
  
  data_plot1->setMinimumSize( 560, 240 );
  data_plot2->setMinimumSize( 560, 240 );

  if ( show_sd )
    {
      distrib_plot_stick( 0 );               // s-distr.
      write_plot( img03File, data_plot1 );
      PlotsFileNames <<  basename + "s_distrib"   + pngext;
    }

  if ( show_mw )
    {
      distrib_plot_stick( 1 );
      write_plot( img04File, data_plot1 );   // MW-distr.
      PlotsFileNames <<  basename + "mw_distrib"  + pngext;
    }

  if ( show_dd )
    {
      distrib_plot_stick( 2 );
      write_plot( img05File, data_plot1 );   // D-distr.
      PlotsFileNames <<  basename + "D_distrib"  + pngext;
    }

  
  //2D distributions: ff0_vs_s, ff0_vs_mw, D_vs_s, D_vs_mw
  if ( show_f_s )
    {
      distrib_plot_2d( ( cnstvb ? 3 : 5 ) );
      write_plot( img06File, data_plot1 );
      PlotsFileNames <<  basename + "ff0_vs_s"  + pngext;
    }

   if ( show_f_mw )
    {
      distrib_plot_2d( ( cnstvb ? 4 : 6 ) );
      write_plot( img07File, data_plot1 );
      PlotsFileNames <<  basename + "ff0_vs_mw"  + pngext;
    }

   if (show_d_s )
     {
       distrib_plot_2d(    7 );
       write_plot( img08File, data_plot1 );
       PlotsFileNames <<  basename + "D_vs_s"  + pngext;
     }

   if ( show_d_mw )
     {
       distrib_plot_2d(    8 );
       write_plot( img09File, data_plot1 );
       PlotsFileNames <<  basename + "D_vs_mw"  + pngext;
     }

   
   //3D plot
   if ( show_3d && !auto_mode )
     {
       write_plot( img10File, NULL );
       PlotsFileNames <<  img10File;
     }
   
   //add .PNG plots to combined PDF report
   assemble_plots_html( PlotsFileNames );
}


// Do stick type distribution plot
void US_ReporterGMP::distrib_plot_stick( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString xatitle;
   QString yatitle = tr( "Rel. Concentr." );

   if ( type == 0 )
   {
      pltitle = pltitle + tr( "\ns20,W Distribution" );
      xatitle = tr( "Corrected Sedimentation Coefficient" );
   }

   else if ( type == 1 )
   {
      pltitle = pltitle + tr( "\nMW Distribution" );
      xatitle = tr( "Molecular Weight (Dalton)" );
   }

   else if ( type == 2 )
   {
      pltitle = pltitle + tr( "\nD20,W Distribution" );
      xatitle = tr( "D20,W (cm^2/sec)" );
   }

   dataPlotClear( data_plot1 );

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );


   int     dsize  = model_used.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( type == 0 ) ? model_used.components[ jj ].s :
               ( ( type == 1 ) ? model_used.components[ jj ].mw :
                                 model_used.components[ jj ].D );
      yval     = model_used.components[ jj ].signal_concentration;
      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type == 0 ) ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin ( true );
   data_grid->enableY    ( true );
   data_grid->setMajorPen(
      QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

   data_curv->setSamples( xx, yy, dsize );
   data_curv->setPen    ( QPen( Qt::yellow, 3, Qt::SolidLine ) );
   data_curv->setStyle  ( QwtPlotCurve::Sticks );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Do 2d type distribution plot
void US_ReporterGMP::distrib_plot_2d( int type )
{
   QString pltitle = tr( "Run " ) + edata->runID + tr( " :\nCell " )
      + edata->cell + " (" + edata->wavelength + " nm)";
   QString yatitle;
   QString xatitle;

   if ( type == 3 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Sed. Coeff." );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 4 )
   {
      pltitle = pltitle + tr( "\nf/f0 vs Mol. Weight" );
      yatitle = tr( "Frictional Ratio f/f0" );
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 5 )
   {
      pltitle = pltitle + tr( "\nVbar vs Sed. Coeff." );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 6 )
   {
      pltitle = pltitle + tr( "\nVbar vs Mol. Weight" );
      yatitle = tr( "Vbar at 20" ) + DEGC;
      xatitle = tr( "Molecular Weight" );
   }

   else if ( type == 7 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Sed. Coeff." );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Sedimentation Coefficient s20,W" );
   }

   else if ( type == 8 )
   {
      pltitle = pltitle + tr( "\nDiff. Coeff. vs Molecular Weight" );
      yatitle = tr( "Diff. Coeff. D20,W" );
      xatitle = tr( "Molecular Weight" );
   }

   dataPlotClear( data_plot1 );

   data_plot1->setTitle(                       pltitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );

   QwtPlotGrid*  data_grid = us_grid(  data_plot1 );
   QwtPlotCurve* data_curv = us_curve( data_plot1, "distro" );
   QwtSymbol*    symbol    = new QwtSymbol;

   int     dsize  = model_used.components.size();
   QVector< double > vecx( dsize );
   QVector< double > vecy( dsize );
   double* xx     = vecx.data();
   double* yy     = vecy.data();
   double  xmin   = 1.0e30;
   double  xmax   = -1.0e30;
   double  ymin   = 1.0e30;
   double  ymax   = -1.0e30;
   double  xval;
   double  yval;
   double  rdif;

   for ( int jj = 0; jj < dsize; jj++ )
   {
      xval     = ( ( type & 1 ) == 1 ) ? model_used.components[ jj ].s :
                                         model_used.components[ jj ].mw;

      if ( type < 5 )             yval = model_used.components[ jj ].f_f0;
      else if ( type < 7 )        yval = model_used.components[ jj ].vbar20;
      else                        yval = model_used.components[ jj ].D;

      xx[ jj ] = xval;
      yy[ jj ] = yval;
      xmin     = min( xval, xmin );
      xmax     = max( xval, xmax );
      ymin     = min( yval, ymin );
      ymax     = max( yval, ymax );
   }

   rdif   = ( xmax - xmin ) / 20.0;
   xmin  -= rdif;
   xmax  += rdif;
   rdif   = ( ymax - ymin ) / 20.0;
   ymin  -= rdif;
   ymax  += rdif;
   xmin   = ( type & 1 ) == 1 ? xmin : max( xmin, 0.0 );
   ymin   = max( ymin, 0.0 );

   data_grid->enableYMin ( true );
   data_grid->enableY    ( true );
   data_grid->setMajorPen(
      QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine ) );

   symbol->setStyle( QwtSymbol::Ellipse );
   symbol->setPen(   QPen(   Qt::red    ) );
   symbol->setBrush( QBrush( Qt::yellow ) );
   if ( dsize > 100 )
      symbol->setSize(  5 );
   else if ( dsize > 50 )
      symbol->setSize(  8 );
   else if ( dsize > 20 )
      symbol->setSize( 10 );
   else
      symbol->setSize( 12 );

   data_curv->setStyle  ( QwtPlotCurve::NoCurve );
   data_curv->setSymbol ( symbol );
   data_curv->setSamples( xx, yy, dsize );

   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft   );
   data_plot1->setAxisScale( QwtPlot::xBottom, xmin, xmax );
   data_plot1->setAxisScale( QwtPlot::yLeft,   ymin, ymax );

   data_plot1->replot();
}

// Create a subdirectory if need be
bool US_ReporterGMP::mkdir( const QString& baseDir, const QString& subdir )
{
   QDir folder( baseDir );

   if ( folder.exists( subdir ) ) return true;

   if ( folder.mkdir( subdir ) ) return true;

   QMessageBox::warning( this,
      tr( "File error" ),
      tr( "Could not create the directory:\n" ) + baseDir + "/" + subdir );

   return false;
}

// Write out a plot
void US_ReporterGMP::write_plot( const QString& filename, const QwtPlot* plot )
{
   if ( filename.contains( ".svg" ) )
   {  // Save an SVG file and a PNG copy
      if ( US_GuiUtil::save_plot( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }
   
//    else if ( filename.endsWith( "rbitmap.png" ) )
//    {  // Special case of rbitmap PNG
//       if ( rbmapd == 0 )
//       {  // if it is not currently displayed, display it
//          rbmapd = new US_ResidsBitmap( resids );
//          rbmapd->move( bmd_pos );
//          rbmapd->show();
//          rbmapd->raise();
//       }

//       else
//       {  // if already displayed,  replot and re-activate
//          rbmapd->replot( resids );
//          rbmapd->raise();
//          rbmapd->activateWindow();
//       }

// #if QT_VERSION > 0x050000
//       QPixmap pixmap = ((QWidget*)rbmapd)->grab();
// #else
//       QPixmap pixmap = QPixmap::grabWidget( rbmapd, 0, 0,
//                                             rbmapd->width(), rbmapd->height() );
// #endif

//       if ( ! pixmap.save( filename ) )
//          QMessageBox::warning( this, tr( "File Write Error" ),
//             tr( "Unable to write file" ) + filename );
//    }

    else if ( filename.endsWith( "3dplot.png" ) )
    {  // Special case of 3dplot PNG
       // if ( eplotcd == 0 )
       // {  // if no 3d plot control up,  create it now
          eplotcd = new US_PlotControlFem( this, &model );
          // eplotcd->move( epd_pos );
          // eplotcd->show();
          eplotcd->do_3dplot_auto();
	  //}

 #if defined(Q_OS_WIN) || defined(Q_OS_MAC)
       qDebug() << "3D: Q_OS_WIN || Q_OS_MAC ";	  
       US_Plot3D* widgw = eplotcd->widget_3dplot();
       bool ok          = widgw->save_plot( filename, QString( "png" ) );
 #else
       qDebug() << "3D: Q_OS_LINUX || other";	  
       QGLWidget* dataw = eplotcd->data_3dplot();
       QPixmap pixmap   = dataw->renderPixmap( dataw->width(), dataw->height(),
                                             true  );
       bool ok          = pixmap.save( filename );
 #endif

       if ( ! ok )
          QMessageBox::warning( this, tr( "File Write Error" ),
             tr( "Unable to write file" ) + filename );
    }

   else if ( filename.endsWith( ".png" ) )
   {  // General case of PNG
      if ( US_GuiUtil::save_png( filename, plot ) != 0 )
         QMessageBox::warning( this, tr( "File Write Error" ),
            tr( "Unable to write file" ) + filename );
   }
}


// Public slot to mark residuals plot dialog closed
void US_ReporterGMP::resplot_done()
{
  qDebug() << "RESPLOT being closed -- ";

  resplotd   = 0; // <--- TEMP
}


// Update progress when thread reports
void US_ReporterGMP::thread_progress( int thr, int icomp )
{
  qDebug() <<  "Updating progress multiple threads, thr, icomp -- " << thr << icomp;
   int kcomp     = 0;
   kcomps[ thr ] = icomp;
   for ( int ii = 0; ii < nthread; ii++ )
      kcomp += kcomps[ ii ];
   progress_msg->setValue( kcomp );
   qApp->processEvents();
   qDebug() << "THR PROGR thr icomp" << thr << icomp << "kcomp" << kcomp;
}


// Update count of threads completed and colate simulations when all are done
void US_ReporterGMP::thread_complete( int thr )
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
      show_results( triple_info_map );
   }
}

// Adjust model components based on buffer, vbar, and temperature
void US_ReporterGMP::adjustModel( )
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


//Parse filename and extract one for given optics type in combined runs
QString US_ReporterGMP::get_filename( QString triple_name )
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

//Start assembling PDF file
void US_ReporterGMP::assemble_pdf( QProgressDialog * progress_msg )
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

  //rptpage  += "  <footer> This is the text that goes at the bottom of every page. </footer>\n";
  //rptpage  += " <div class='footer'>Footer</div>";

  
  
  //HEADER: begin
  QString us_version = "Version " + US_Version + " ( " REVISION " ) for " OS_TITLE;
  QString html_header = QString("");
  html_header += rptpage;
  html_header += tr( 
    "<div align=left>"
      "Created, %1<br>"
      "with UltraScan-GMP<br>"
      "%2<br>"
      "by AUC Solutions<br>"
    "</div>"
		     )
    .arg( current_date )
    .arg( us_version ) 
    ;
  //HEADER: end

  
  //TITLE: begin
  QString report_type;
  GMP_report ? report_type = "GMP" : report_type = "Non-GMP";
  QString html_title = tr(
    "<h1 align=center>%1 Report for Run: <br><i>%2</i></h1>"
    "<hr>"
			  )
    .arg( report_type )                              //1
    .arg( runName )                                  //2
    ;
  //TITLE: end

  QString html_paragraph_open = tr(
    "<p align=justify>"
				   )
    ;

  
  //Failed Triples' Analyses, Missing Models info (if any):
  QString str_failed_stage_missing_models = compose_html_failed_stage_missing_models();
  qDebug() << "STR_on_failed: " << str_failed_stage_missing_models;
  if ( !str_failed_stage_missing_models.isEmpty() )
    {
      html_failed  = tr(
			"<h3 style=\"color:red;\" align=left> ATTENTION: Analyses for Some Triples Failed, or Models are Missing!</h3>"
			"%1"
			"<hr>"
			)
	.arg( str_failed_stage_missing_models )
	;
    }
  //End of failed|missing
			
			
  //GENERAL: begin
  html_general = tr(
    
    "<h3 align=left>General Settings</h3>"
      "<table>"
        "<tr><td>Investigator: </td>                           <td>%1</td></tr>"
        "<tr><td>Protocol Name: </td>                          <td>%2</td></tr>"
        "<tr><td>Project:</td>                                 <td>%3</td></tr>"
        "<tr><td>Run Temperature (&#8451;):</td>               <td>%4</td></tr>"
        "<tr><td>Temperature-Equilibration Delay (mins):</td>  <td>%5</td></tr>"
      "</table>"
    "<hr>"
			    )
    .arg( currProto. investigator)  //1
    .arg( currProto. protoname )    //2
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
  
  //OPERATOR | REVIEWERS | APPROVERS: begin
  QJsonDocument jsonDocOperList = QJsonDocument::fromJson( eSign_details[ "operatorListJson" ] .toUtf8() );
  QString opers_a = get_assigned_oper_revs( jsonDocOperList );

  QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( eSign_details[ "reviewersListJson" ] .toUtf8() );
  QString revs_a = get_assigned_oper_revs( jsonDocRevList );

  QJsonDocument jsonDocApprList  = QJsonDocument::fromJson( eSign_details[ "approversListJson" ] .toUtf8() );
  QString apprs_a = get_assigned_oper_revs( jsonDocApprList );

  QString run_id = ( dataSource == "INSTRUMENT" ) ? runID : "N/A";
  QString instr_name = ( dataSource == "INSTRUMENT" ) ? currProto. rpRotor.instrname : "dataDisk";
  
  html_operator = tr(     
    "<h3 align=left>Optima Machine/Operator </h3>"
      "<table>"
        "<tr><td>Optima: </td>           <td>%1</td></tr>"
        "<tr><td>Optima's RunID: </td>   <td>%2</td></tr>"
        "<tr><td>Operator(s): </td>      <td>%3</td></tr>"
        "<tr><td>Reviewer(s): </td>      <td>%4</td></tr>"
        "<tr><td>Approver(s): </td>      <td>%5</td></tr>"
        "<tr><td>Experiment Type:</td>   <td>%6</td></tr>"
      "</table>"
    "<hr>"
				  )
    .arg( instr_name )                     //1
    .arg( run_id )                         //2
    //.arg( currProto. rpRotor.opername  )   //3 <-- OLD, incorrect
    .arg( opers_a  )                       //3
    .arg( revs_a  )                        //4
    .arg( apprs_a  )                       //5
    .arg( currProto. rpRotor.exptype )     //6
    ;
  //OPERATOR | REVIEWERS | APPROVERS: end 	  

  
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

      qDebug() << "Progress_msg value() -- " << progress_msg->value() ;
      int progress_val = progress_msg->value() ;
      progress_msg->setValue( ++progress_val );
      qApp->processEvents();
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
			)
	.arg( QString::number( w_count ) )        //1
	;

      if ( w_count > 1 )
	{
	  html_ranges += tr(
			    "<tr><td> Selected Wavelength range: </td>  <td> %1 </td> </tr>"
			    )
	    .arg( w_range )                        //1
	    ;
	}
      
      html_ranges += tr(
			"<tr><td> Radius range:              </td>  <td> %1 </td> </tr>"
			"<tr><td> Selected Wavelengths:      </td>  <td> %2 </td> </tr>"
			"</table>"
			)
	.arg( r_range )                           //1
	.arg( all_wvl )                           //2	
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
	  //QList < double > chann_wvls                = ch_wvls[ channel_desc ]; //ALEXEY: <-- BUG!
	  QList < double > chann_wvls                  = ch_wvls[ channel_desc_alt ]; 
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
  
  
  //Main assembly: reportMask based
  //QString html_assembled = QString("");
  html_assembled +=
    html_header
    + html_title
    + html_paragraph_open;

  assemble_parts( html_assembled );

  QString html_paragraph_close = tr( "</p>" );
  html_assembled += html_paragraph_close;
  
}


//write PDF Report
void US_ReporterGMP::write_pdf_report( void )
{
  QString html_paragraph_close = tr( "</p>" );

  QString html_footer = tr( 
    "<div align=right>"
       "<br>End of report: <i>\"%1\"</i>"
    "</div>"
			    )
    .arg( runName )
    ;
  
  html_assembled += html_footer;

  QString subDirName  = runName + "-run" + runID;
  QString dirName     = US_Settings::reportDir() + "/" + subDirName;
  mkdir( US_Settings::reportDir(), subDirName );
    
  //QString fileName  = runName + ".pdf";
  QString fileName  = ProtocolName_auto + ".pdf"; //Use unique protocol name for .pdf filename
  
  //filePath  = US_Settings::tmpDir() + "/" + fileName;
  filePath  = dirName + "/" + fileName;
  
  // //Standard way if printing: ******************************/
  // QTextDocument document;
  // document.setHtml( html_assembled );
  
  // QPrinter printer(QPrinter::PrinterResolution);
  // printer.setOutputFormat(QPrinter::PdfFormat);
  // printer.setPaperSize(QPrinter::Letter);

  // printer.setOutputFileName( filePath );
  // printer.setFullPage(true);
  // printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
  
  // document.print(&printer);
  /** END of standard way of printing *************************/

  qDebug() << "HTMP_assembled -- " << html_assembled;


  
  /** ALT. painting ********************************************/
  QTextDocument textDocument;
  textDocument.setHtml( html_assembled );

  qDebug() << "Default QtextDoc font1: " << textDocument.defaultFont();
  QFont t_f = textDocument.defaultFont();
  t_f. setPointSize( 7 );
  textDocument. setDefaultFont( t_f );
  qDebug() << "Default QtextDoc font2: " << textDocument.defaultFont();
  
  QPrinter printer(QPrinter::PrinterResolution);//(QPrinter::HighResolution);//(QPrinter::PrinterResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setPaperSize(QPrinter::Letter);

  printer.setOutputFileName( filePath );
  printer.setFullPage(true);
  //printer.setFullPage(false);
  
  printDocument(printer, &textDocument ); //, 0);
  
  /*************************************************************/

  qApp->processEvents();
  

   // /**************************************************************************************/
   //  //TEST !!! Write HTML file to BLOB instead SEPARATELY for hard-coded GMP Report IDs: 
   //  //  
   //  //
   //  // Choose one of the below!
   //  //int r_ID = 1;  // < --- [FOR SBird-DNA-EcoRI-101322-PD9 .. autoflowID=838] 
   //  int r_ID = 12;   // < --- [FOR OkaK_MW-AAV8_17-19_062823 .. autoflowID=930]
  
   //    US_Passwd pw;
   //    US_DB2    db( pw.getPasswd() );
  
   //    if ( db.lastErrno() != US_DB2::OK )
   //      {
   //        QMessageBox::warning( this, tr( "Connection Problem" ),
   //    			    tr( "Could not connect to database \n" ) +  db.lastError() );
   //        return;
   //      }

   //    //Write HTML strign to file & later save to DB withing general archive
   //    QString html_filePath = dirName + "/" + "html_string.html";
   //    QFile file_html_str( html_filePath );
   //    qDebug() << "HTML to be written: filepath -- " << html_filePath;
   //    if(!file_html_str.open(QIODevice::WriteOnly))
   //      file_html_str.close();
   //    else
   //      {
   //        file_html_str.write( html_assembled.toUtf8() );
   //        //QTextStream out(&file_html_str); out << html_assembled;
   //        file_html_str.close();
   //      }

   //    qDebug() << "html_filePath: " << html_filePath;
   //    int writeStatus_html = db.writeBlobToDB(html_filePath,
   //    					  QString( "upload_gmpReportData_html" ),
   //    					  r_ID );

   //    qApp->processEvents();
   //    if ( writeStatus_html == US_DB2::DBERROR )
   //      {
   //        QMessageBox::warning(this, "Error", "Error processing html file:\n"
   //    			   + html_filePath + "\n" + db.lastError() +
   //    			   "\n" + "Could not open file or no data \n");
   //      }
  
   //    else if ( writeStatus_html != US_DB2::OK )
   //      {
   //        QMessageBox::warning(this, "Error", "returned processing html file:\n" +
   //    			   html_filePath + "\n" + db.lastError() + "\n");
  
   //      }

   //    //
  
   //    QStringList file_exts;
   //    //file_exts << "*.png" << "*.svgz";
   //    file_exts << "*.svgz" << "*.html";                // retain *pngs (BUT remove *html) for further assembly at e-Signing
   //    remove_files_by_mask( dirName, file_exts );

   //    QString tarFilename_t = subDirName + "_GMP_DB.tar";
   //    //QString tarFilename_t = subDirName + "_GMP_DB.rar";
  
   //    QProcess *process = new QProcess(this);
   //    process->setWorkingDirectory( US_Settings::reportDir() );
   //    process->start("tar", QStringList() << "-cvf" << tarFilename_t << subDirName );
   //    //process->start("rar", QStringList() << "a" << tarFilename_t << subDirName );
   //    //sleep( 5 );                     //Maybe implement something like a timer to check on completion!!!
   //    process -> waitForFinished();
   //    process -> close();
  
   //    qDebug() << "tar command: " << "tar " << "-cvf " << tarFilename_t << ", " << subDirName;

   //    QString r_filepath = US_Settings::reportDir() + "/" + tarFilename_t;
   //    qDebug() << "r_filepath: " << r_filepath;

  
   //    qApp->processEvents();
  
   //    //Like process -> returned signal??
  
   //    int writeStatus= db.writeBlobToDB(r_filepath,
   //    				    QString( "upload_gmpReportData" ),
   //    				    r_ID );
   //    qApp->processEvents();
  
   //    if ( writeStatus == US_DB2::DBERROR )
   //      {
   //        QMessageBox::warning(this, "Error", "Error processing file:\n"
   //    			   + r_filepath + "\n" + db.lastError() +
   //    			   "\n" + "Could not open file or no data \n");
   //      }
  
   //    else if ( writeStatus != US_DB2::OK )
   //      {
   //        QMessageBox::warning(this, "Error", "returned processing file:\n" +
   //    			   r_filepath + "\n" + db.lastError() + "\n");
  
   //      }

   //  /****** END TEST *************************************************************/

  
  //Now delete all .png && .svgz && tar entire directory
  if ( auto_mode )
    {
      QStringList file_exts;
      //file_exts << "*.png" << "*.svgz";
      file_exts << "*.svgz" << "*.html" << "*.txt";                // retain *pngs (BUT remove *html) for further assembly at e-Signing
      remove_files_by_mask( dirName, file_exts );

      
      //Archive using US_Tar [does NOT work for filename lengths >=100 char]
      /****************************************************************************
      QDir odir( dirName );
      QStringList fileList = odir.entryList( QStringList( "*.pdf" ), QDir::Files );

      QStringList fileList_fullPath;
      for (int i=0; i<fileList.size(); ++i )
       	{
       	  fileList_fullPath << dirName + "/" + fileList[i];
       	  qDebug() << "Files -- " << dirName + "/" + fileList[i];
       	}
      
      US_Tar tar;
      int result;
       QString tarFilename = dirName + "/" + subDirName + ".tar.gz";
       QStringList list_t;
       //result = tar.create( tarFilename, dirName, &list_t );
       result = tar.create( tarFilename, fileList_fullPath, &list_t );
       
       if ( result != TAR_OK )
	 {
	   QString errormsg = QString("Error: Problem creating tar archive %1").arg( tarFilename );
	   
	   QMessageBox::warning( this, tr( "Error with creating .TAR" ),
				 errormsg );
	   
	   return;
	 }
      ***************************************************************************/
      
      //Archive using system TAR: do NOT use gZip (.tgz, .tar.gz)!!! 
      //<------ TESTING!!!! ***************************************************************
      // QString tarFilename_t = subDirName + "_GMP_DB.tgz";
      // QProcess *process = new QProcess(this);
      // process->setWorkingDirectory( US_Settings::reportDir() );
      // process->start("tar", QStringList() << "-czvf" << tarFilename_t << subDirName );
      //END TESTING ***********************************************************************
            
      QString tarFilename_t = subDirName + "_GMP_DB.tar";
      QProcess *process = new QProcess(this);
      process->setWorkingDirectory( US_Settings::reportDir() );
      process->start("tar", QStringList() << "-cvf" << tarFilename_t << subDirName );
      process -> waitForFinished();
      process -> close();
      qApp->processEvents();
      
      //Write to autoflowGMPReport table as longblob
      write_gmp_report_DB( tarFilename_t, fileName );
      qApp->processEvents();
      
      //do we need to remove created .tar?
      QString tar_path = US_Settings::reportDir() + "/" + tarFilename_t;
      QFile::remove( tar_path );
    }
}


double US_ReporterGMP::mmToPixels(QPrinter& printer, int mm)
{
  return mm * 0.039370147 * printer.resolution();
}

void US_ReporterGMP::printDocument(QPrinter& printer, QTextDocument* doc) //, QWidget* parentWidget)
{
  int textMargins = 12; // in millimeters
  //int textMargins = 0; // in millimeters
  
  QPainter painter( &printer );
  QSizeF pageSize = printer.pageRect().size(); // page size in pixels
  // Calculate the rectangle where to lay out the text
  const double tm = mmToPixels(printer, textMargins);
  const qreal footerHeight = painter.fontMetrics().height();
  const QRectF textRect(tm, tm, pageSize.width() - 2 * tm, pageSize.height() - 2 * tm - footerHeight);
  qDebug() << "textRect=, width, height: " << textRect << textRect.width() << textRect.height();
  qDebug() << "footerHeigh: " << footerHeight;
  doc->setPageSize(textRect.size());
  
  const int pageCount = doc->pageCount();
  // QProgressDialog dialog( QObject::tr( "Printing" ), QObject::tr( "Cancel" ), 0, pageCount, parentWidget );
  // dialog.setWindowModality( Qt::ApplicationModal );
  progress_msg = new QProgressDialog ("Preparing .PDF...", QString(), 0, pageCount, this);
  progress_msg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setModal( true );
  progress_msg->setWindowTitle(tr("Printing..."));
  QFont font_d  = progress_msg->property("font").value<QFont>();
  QFontMetrics fm(font_d);
  int pixelsWide = fm.width( progress_msg->windowTitle() );
  qDebug() << "Progress_msg: pixelsWide -- " << pixelsWide;
  progress_msg ->setMinimumWidth( pixelsWide*2 );
  progress_msg->adjustSize();
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();
  
  bool firstPage = true;
  for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex)
    {
      progress_msg->setValue( pageIndex );
      
      if (!firstPage)
	{
	  //immediately before newPage, set new margins:
	  //printer.setPageMargins(0, -15, 0, -15, QPrinter::Millimeter);
	  printer.newPage();
	}
      
      paintPage( printer, pageIndex, pageCount, &painter, doc, textRect, footerHeight );
      firstPage = false;
    }

  progress_msg->close();
}


void US_ReporterGMP::paintPage(QPrinter& printer, int pageNumber, int pageCount,
			       QPainter* painter, QTextDocument* doc,
			       const QRectF& textRect, qreal footerHeight )
{
  int borderMargins = 10;  // in millimeters
  qDebug() << "Printing page" << pageNumber;
  const QSizeF pageSize = printer.paperRect().size();
  qDebug() << "pageSize=" << pageSize;
  qDebug() << "printerResolution=" << printer.resolution();
  
  // const double bm = mmToPixels(printer, borderMargins);
  // const QRectF borderRect(bm, bm, pageSize.width() - 2 * bm, pageSize.height() - 2 * bm);
  //painter->drawRect(borderRect);
  
  painter->save();
  
  // textPageRect is the rectangle in the coordinate system of the QTextDocument, in pixels,
  // and starting at (0,0) for the first page. Second page is at y=doc->pageSize().height().
  const QRectF textPageRect(0, pageNumber * doc->pageSize().height(), doc->pageSize().width(), doc->pageSize().height());
  // Clip the drawing so that the text of the other pages doesn't appear in the margins
  //painter->setClipRect(textRect);
  // Translate so that 0,0 is now the page corner
  painter->translate(0, -textPageRect.top());
  // Translate so that 0,0 is the text rect corner
  painter->translate(textRect.left(), textRect.top());

  // qDebug() << "Painter's settings: font, metrics -- "
  // 	   << painter->fontMetrics()
  // 	   << painter->fontInfo()
  // 	   << painter->font();

  qDebug() << "Painter's settings: font, metrics -- "
	   << painter->fontInfo().family()
	   << painter->fontInfo().pointSizeF()
	   << painter->fontInfo().pointSize();
  
  doc->drawContents(painter,textPageRect); //add second params, OR it appears to draw full QTextDoc.... (huge sizes)
  painter->restore();
  
  // Footer: e-Signer comment && page number
  QRectF footerRect = textRect;
  footerRect.setTop(textRect.bottom());
  footerRect.setHeight( 1*footerHeight ); //will a parameter on #of lines (footer height, depending on # reviewers...)
  //test below
  //footerRect.setHeight( 4*footerHeight ); //will a parameter on #of lines (footer height, depending on # reviewers...)
  //end test

  painter->setPen(Qt::blue);
  QFont pfont = painter -> font();
  qDebug() << "Paint page: painter: pixelSize(), pointSize() -- "
	   << pfont. pixelSize()
	   << pfont. pointSize();
  int original_pfont_size = pfont. pointSize();
  pfont. setPointSize( int ( original_pfont_size * 0.8 ) );
  painter-> setFont(pfont);

  QString init_footer;
  init_footer = auto_mode ?  "Not e-Signed/Not Reviewed..." : "Custom Report";
  //painter->drawText(footerRect, Qt::AlignLeft, QObject::tr("Not e-Signed/Not Reviewed..." ));
  painter->drawText(footerRect, Qt::AlignLeft, init_footer );
    //test below
  //painter->drawText(footerRect, Qt::AlignLeft, QObject::tr("Not e-Signed/Not Reviewed...\nNot e-Signed/Not Reviewed...\nNot e-Signed/Not Reviewed...\nNot e-Signed/Not Reviewed..." ));
  //end test
  painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("Page %1/%2").arg(pageNumber+1).arg(pageCount));

  // Footer: page number or "end"
  // if (pageNumber == pageCount - 1)
  //   painter->drawText(footerRect, Qt::AlignLeft, QObject::tr("Footer to be passed by e-Signers"));
  // else
  //   painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("Page %1/%2").arg(pageNumber+1).arg(pageCount));
  
  //and restore painter's font:
  pfont. setPointSize( original_pfont_size );
  painter-> setFont(pfont);
}



//write GMP report to DB
void US_ReporterGMP::write_gmp_report_DB( QString filename, QString filename_pdf )
{
  QString report_filepath = US_Settings::reportDir() + "/" + filename;
  qDebug() << "Writing Blob of filePath -- " << report_filepath;
  
  US_Passwd pw;
  US_DB2    db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Could not connect to database \n" ) +  db.lastError() );
      return;
    }

  QStringList qry;
  
  //BEFORE writing, check if writing has been initiated, or completed from different session:
  int status_report_unique = 0;
  qry << "autoflow_report_status"
      << AutoflowID_auto;
  
  status_report_unique = db.statusQuery( qry );

  qDebug() << "status_report_unique -- " << status_report_unique ;
  
  if ( !status_report_unique || status_report_unique < 0 )
    {
      QMessageBox::information( this,
				tr( "The Program State Updated / Being Updated" ),
				tr( "This happened because you, or different user "
				    "has already saved the GMP Report into DB using "
				    "different program session. \n\n"
				    "You can still view generated GMP Report in .PDF format "
				    "after closing this dialog window.") );
      return;
    }
  ///////////////////////////////////////////////////////////////////////////////////////////
     
  qry. clear();
  qry << "new_autoflow_gmp_report_record"
      << AutoflowID_auto            
      << FullRunName_auto           
      << ProtocolName_auto
      << filename_pdf;
    
  int autolfowGMPReportID = 0;
  int state_new_gmpReport = db.statusQuery( qry );
  autolfowGMPReportID = db.lastInsertID();
  
  bool clear_GMP_report_record = false;

  qDebug() << "state_new_gmpReport, autolfowGMPReportID -- "
	   << state_new_gmpReport
	   << autolfowGMPReportID;
  
  if ( state_new_gmpReport == US_DB2::OK  && autolfowGMPReportID > 0 )
    {
      int writeStatus= db.writeBlobToDB(report_filepath,
					QString( "upload_gmpReportData" ),
					autolfowGMPReportID );

      if ( writeStatus == US_DB2::DBERROR )
	{
	  QMessageBox::warning(this, "Error", "Error processing file:\n"
			       + report_filepath + "\n" + db.lastError() +
			       "\n" + "Could not open file or no data \n");
	  clear_GMP_report_record = true;
	}
      
      else if ( writeStatus != US_DB2::OK )
	{
	  QMessageBox::warning(this, "Error", "returned processing file:\n" +
			       report_filepath + "\n" + db.lastError() + "\n");
	  
	  clear_GMP_report_record = true;
	}

      /*************************************************************************/
      //Write HTML strign to file & later save to DB withing general archive
      QString subDirName1  = runName + "-run" + runID;
      QString dirName1     = US_Settings::reportDir() + "/" + subDirName1;
      QString html_filePath = dirName1 + "/" + "html_string.html";
      QFile file_html_str( html_filePath );
      if(!file_html_str.open(QIODevice::WriteOnly))
	file_html_str.close();
      else
	{
	  file_html_str.write( html_assembled.toUtf8() );
	  //QTextStream out(&file_html_str); out << html_assembled;
	  file_html_str.close();
	}

      int writeStatus_html = db.writeBlobToDB( html_filePath,
					       QString( "upload_gmpReportData_html" ),
					       autolfowGMPReportID );

      if ( writeStatus_html == US_DB2::DBERROR )
	{
	  QMessageBox::warning(this, "Error", "Error processing html file:\n"
			       + html_filePath + "\n" + db.lastError() +
			       "\n" + "Could not open file or no data \n");
	  clear_GMP_report_record = true;
	}
      
      else if ( writeStatus_html != US_DB2::OK )
	{
	  QMessageBox::warning(this, "Error", "returned processing html file:\n" +
			       html_filePath + "\n" + db.lastError() + "\n");
	  
	  clear_GMP_report_record = true;
	}

      /*************************************************************************/
      
      
      if ( clear_GMP_report_record )
	{
	  qry.clear();
	  qry << "clear_autoflowGMPReportRecord" << QString::number( autolfowGMPReportID );
	  db.query( qry );

	  //Maybe revert 'reporting' stage in the autoflowStages??
	}
      else
	{
	  //Report generated && .PDF GMP report saved to autoflowGMPReport:
	  //No, we can write information on who/when generated report: ///////////////////////

	  //get user info
	  qry.clear();
	  qry <<  QString( "get_user_info" );
	  db.query( qry );
	  db.next();
	  
	  int ID        = db.value( 0 ).toInt();
	  QString fname = db.value( 1 ).toString();
	  QString lname = db.value( 2 ).toString();
	  QString email = db.value( 4 ).toString();
	  int     level = db.value( 5 ).toInt();

	  QString reporting_Json;

	  reporting_Json. clear();
	  reporting_Json += "{ \"Person\": ";
	  
	  reporting_Json += "[{";
	  reporting_Json += "\"ID\":\""     + QString::number( ID )     + "\",";
	  reporting_Json += "\"fname\":\""  + fname                     + "\",";
	  reporting_Json += "\"lname\":\""  + lname                     + "\",";
	  reporting_Json += "\"email\":\""  + email                     + "\",";
	  reporting_Json += "\"level\":\""  + QString::number( level )  + "\"";
	  reporting_Json += "}]}";
	  	  
	  qry.clear();
	  qry << "update_autoflowStatusReport_record"
	      << autoflowStatusID
	      << AutoflowID_auto
	      << reporting_Json;
	  
	  db.query( qry );
      	}
    }
  else
    {
      QMessageBox::warning(this, "Error returned processing file", "File: " + report_filepath + db.lastError());
    }
}

//remove files by extension from dirPath
void US_ReporterGMP::remove_files_by_mask( QString dirPath, QStringList file_exts )
{
  QDir dir( dirPath );
  dir.setNameFilters(file_exts);
  dir.setFilter(QDir::Files);
  foreach( QString dirFile, dir.entryList() ) 
    {
      dir.remove(dirFile);
    }
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

  //tree-to-json: combPlotsTree
  QString editedMask_combPlots = tree_to_json ( topItemCombPlots );
  parse_edited_combPlots_mask_json( editedMask_combPlots, combPlotsMask_edited );

  //tree-to-json: miscTree
  QString editedMask_misc = tree_to_json ( miscItem );
  parse_edited_misc_mask_json( editedMask_misc, miscMask_edited );

  
  //For GMP Reporter only: Compare Json mask states to originally loaded:
  if ( auto_mode )
    GMP_report = true;
  else
    {
      if( editedMask_gen !=JsonMask_gen_loaded || editedMask_perChan != JsonMask_perChan_loaded )
	GMP_report = false;
    }
  //DEBUG
  //exit(1);
}

//Parse Miscellaneous JSON
void US_ReporterGMP::parse_edited_misc_mask_json( const QString maskJson, MiscReportMaskStructure & MaskStr )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
  QJsonObject json = jsonDoc.object();

  MaskStr.ShowMiscParts . clear();

  foreach(const QString& key, json.keys())
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();

      if ( value.isString() )
	{
	  if ( value.toString().toInt() )
	      MaskStr.ShowMiscParts[ key ] = true;
	  else
	    MaskStr.ShowMiscParts[ key ] = false;
	}
    }
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

		       qDebug() << "Parse_editedJsonGen: Solution: array_key, value: "
				<<  array_key
				<<  json_array[i].toObject().value(array_key).toString();
			       
		       
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

			       qDebug() << "Parse_editedJsonGen: Analysis:Gen: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysisGenParts[ n_key ];
			       
			     }
			   
			   if ( array_key.contains("2DSA") )
			     {
			       MaskStr.ShowAnalysis2DSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysis2DSAParts[ n_key ].toInt() )
				 ++MaskStr.has_ana2dsa_items;

			       qDebug() << "Parse_editedJsonGen: Analysis:2DSA: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysis2DSAParts[ n_key ];
			       
			     }
			   
			   if ( array_key.contains("PCSA") ) 
			     {
			       MaskStr.ShowAnalysisPCSAParts[ n_key ] = analysis_cathegory_item_value;
			       if ( MaskStr.ShowAnalysisPCSAParts[ n_key ].toInt() )
				 ++MaskStr.has_anapcsa_items;

			       qDebug() << "Parse_editedJsonGen: Analysis:PCSA: n_key, value: "
					<<  n_key
					<<  MaskStr.ShowAnalysisPCSAParts[ n_key ];
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
    QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
    QJsonObject json = jsonDoc.object();
    
    MaskStr.ShowChannelParts              .clear();
    MaskStr.ShowTripleModelParts          .clear();
    MaskStr.has_tripleModel_items         .clear();
    MaskStr.ShowTripleModelPlotParts      .clear();
    MaskStr.has_tripleModelPlot_items     .clear();
    MaskStr.ShowTripleModelPseudo3dParts  .clear();
    MaskStr.has_tripleModelPseudo3d_items .clear();
    
    MaskStr.ShowTripleTypeModelRangeIndividualCombo. clear();
    MaskStr.has_tripleModelIndCombo_items .clear();
    
    foreach(const QString& key, json.keys())                                          //over channels
      {
	int has_channel_items = 0;
	
	QJsonValue value = json.value(key);
	qDebug() << "Key = " << key << ", Value = " << value;//.toString();

	MaskStr.ShowChannelParts[ key ] = false;  //for now
	
	QJsonArray json_array = value.toArray();
	for (int i=0; i < json_array.size(); ++i )  
	  {
	    foreach(const QString& array_key, json_array[i].toObject().keys())        //over triples
	      {
		QJsonObject tripleObj = json_array[i].toObject().value(array_key).toObject();

		foreach ( const QString& n_key, tripleObj.keys() )                    //over models
		  {
		    QJsonObject modelObj = tripleObj.value( n_key ).toObject();

		    foreach ( const QString& j_key, modelObj.keys() )                 //over models' features (tot_conc, RMSD...) 
		      {
			QJsonValue feature_value = modelObj.value( j_key );
			
			if ( feature_value.isString() ) //deal with plots main characteristics ( tot_conc, RMSD)
			  {
			    if ( feature_value.toString().toInt() )
			      ++has_channel_items;

			    //QString triple_name = key.split(" ")[1].split("-")[0] + array_key.split(" ")[0];
			    QString triple_name = key.split(" ")[1].split("-")[0];

			    if ( key.contains( "Interf" ) )
			      triple_name += "Interference";
			    else
			      triple_name += array_key.split(" ")[0];
			    
			    QString model_name  = n_key.split(" ")[0];
			    MaskStr.ShowTripleModelParts[ triple_name ][ model_name ][ j_key ] = feature_value.toString();
			    if ( MaskStr.ShowTripleModelParts[ triple_name ][ model_name ][ j_key ].toInt() )
			      ++MaskStr.has_tripleModel_items[ triple_name ][ model_name ];
			    
			    qDebug() << "Parse_editedJsonTriples: triple_name: " <<  triple_name  << ": "
				     <<  "model_name, j_key, feature_value: "
				     <<  model_name
				     <<  j_key
				     <<  feature_value.toString();
			  }
			else  
			  {
			    //begin "Plots"
			    if ( j_key.contains("Plots") ) 
			      { 
				QJsonObject plotObj = modelObj.value( j_key ).toObject();
				
				foreach ( const QString& p_key, plotObj.keys() )           //over plots, per model/per triple/per channel
				  {
				    QString feature_plot_value = plotObj.value( p_key ).toString();
				    
				    //QString triple_name = key.split(" ")[1].split("-")[0]  + array_key.split(" ")[0];
				    QString triple_name = key.split(" ")[1].split("-")[0];

				    if ( key.contains( "Interf" ) )
				      triple_name += "Interference";
				    else
				      triple_name += array_key.split(" ")[0];
				    
				    QString model_name  = n_key.split(" ")[0];
				    MaskStr.ShowTripleModelPlotParts[ triple_name ][ model_name ][ p_key ] = feature_plot_value;
				    if ( MaskStr.ShowTripleModelPlotParts[ triple_name ][ model_name ][ p_key ].toInt() )
				      ++MaskStr.has_tripleModelPlot_items[ triple_name ][ model_name ];

				    qDebug() << "Parse_editedJsonTriples: triple_name: " <<  triple_name  << ": "
					     <<  "model_name, j_key, p_key, feature_PLOT_value: "
					     <<  model_name
					     <<  j_key
					     <<  p_key
					     <<  feature_plot_value;
				  }
			      }
			    //end of "Plots"

			    //begin "Pseudo3D"
			    else if ( j_key.contains("Pseudo3d") )  
			      {
				QJsonObject pseudo3dObj = modelObj.value( j_key ).toObject();
				
				foreach ( const QString& p_key, pseudo3dObj.keys() )           //over pseudo3d, per model/per triple/per channel
				  {
				    QString feature_pseudo3d_value = pseudo3dObj.value( p_key ).toString();
				    
				    //QString triple_name = key.split(" ")[1].split("-")[0]  + array_key.split(" ")[0];
				    QString triple_name = key.split(" ")[1].split("-")[0];

				    if ( key.contains( "Interf" ) )
				      triple_name += "Interference";
				    else
				      triple_name += array_key.split(" ")[0];

				    
				    QString model_name  = n_key.split(" ")[0];
				    MaskStr.ShowTripleModelPseudo3dParts[ triple_name ][ model_name ][ p_key ] = feature_pseudo3d_value;
				    if ( MaskStr.ShowTripleModelPseudo3dParts[ triple_name ][ model_name ][ p_key ].toInt() )
				      ++MaskStr.has_tripleModelPseudo3d_items[ triple_name ][ model_name ];
				    
				    qDebug() << "Parse_editedJsonTriples: triple_name: " <<  triple_name  << ": "
					     <<  "model_name, j_key, p_key, feature_PSEUDO_value: "
					     <<  model_name
					     <<  j_key
					     <<  p_key
					     <<  feature_pseudo3d_value;
				  }
			      }
			    //end of "Pseudo3D"

			    //begin "Individual"
			    else if ( j_key.contains("Individual") ) 
			      {
				QJsonObject indComboObj = modelObj.value( j_key ).toObject();
				
				foreach ( const QString& p_key, indComboObj.keys() )           //over type[ranges], per model/per triple/per channel
				  {
				    QString feature_indCombo_value = indComboObj.value( p_key ).toString();
				    
				    //QString triple_name = key.split(" ")[1].split("-")[0]  + array_key.split(" ")[0];
				    QString triple_name = key.split(" ")[1].split("-")[0];

				    if ( key.contains( "Interf" ) )
				      triple_name += "Interference";
				    else
				      triple_name += array_key.split(" ")[0];

				    QString model_name  = n_key.split(" ")[0];

				    MaskStr.ShowTripleTypeModelRangeIndividualCombo[ triple_name ][ model_name ][ p_key ] = feature_indCombo_value;
				    if ( MaskStr.ShowTripleTypeModelRangeIndividualCombo[ triple_name ][ model_name ][ p_key ].toInt() )
				      ++MaskStr.has_tripleModelIndCombo_items[ triple_name ][ model_name ];

				    qDebug() << "Parse_editedJsonTriples: triple_name: " <<  triple_name  << ": "
					     <<  "model_name, j_key, p_key, feature_INDCOMBO_value: "
					     <<  model_name
					     <<  j_key
					     <<  p_key
					     <<  feature_indCombo_value;
				  }
			      }
			    //end of "Individual"
			  }
		      }
		  }
	      }
	  }

	if ( !has_channel_items )
	  MaskStr.ShowChannelParts[ key ] = false;
      }
}


//Pasre reportMask JSON: combPlots
void US_ReporterGMP::parse_edited_combPlots_mask_json( const QString maskJson, CombPlotsReportMaskStructure & MaskStr )
{
  QJsonDocument jsonDoc = QJsonDocument::fromJson( maskJson.toUtf8() );
  QJsonObject json = jsonDoc.object();
    
  MaskStr. ShowCombPlotsTypes  .clear();
  MaskStr. ShowCombPlotParts   .clear();
  MaskStr. has_combo_plots = 0;
  
  foreach(const QString& key, json.keys())                                          //over types
    {
      QJsonValue value = json.value(key);
      qDebug() << "Key = " << key << ", Value = " << value;//.toString();
      
      QJsonArray json_array = value.toArray();
      for (int i=0; i < json_array.size(); ++i )  
	{
	  foreach(const QString& array_key, json_array[i].toObject().keys())        //over models (aka methods)
	    {
	      QJsonValue show_plot = json_array[i].toObject().value(array_key);

	      QString array_key_mod = array_key;
	      if ( array_key.contains("PCSA") )
		array_key_mod = "PCSA";
		
	      MaskStr. ShowCombPlotParts[ key ][ array_key_mod ] = show_plot.toString();

	      qDebug() << "CombPlots: type, model, yes/no -- "
		       << key << array_key_mod << show_plot.toString();

	      if ( MaskStr. ShowCombPlotParts[ key ][ array_key_mod ] .toInt() )
		++MaskStr. has_combo_plots;
	    }
	}
    }
}


void US_ReporterGMP::get_children_to_json( QString& mask_edited, QTreeWidgetItem* item )
{
  for( int i = 0; i < item->childCount(); ++i )
    {
      mask_edited += "\"" + item->child(i)->text(1).trimmed() + "\":";
      
      int children_lev2 = item->child(i)->childCount();
      if ( !children_lev2 )
	{
	  mask_edited += "\"" + QString::number( int(item->child(i)->checkState(0)) ) + "\"";
	  if ( i != item->childCount()-1 )
	    mask_edited += ",";
	}
      else
	{
	  mask_edited += "{";
	  get_children_to_json( mask_edited, item->child(i) );
	  mask_edited += "},";
	}
    }

  //ALEXEY: <-- little trick to enable super-fast recursive over arbitrary tree:))
  mask_edited.replace(",},","},");  
}


//transform arbitrary-level tree to JSON
QString US_ReporterGMP::tree_to_json( QMap < QString, QTreeWidgetItem * > topLevItems )
{
  QString mask_edited;
  mask_edited += "{";
  
  QMap < QString, QTreeWidgetItem * >::iterator top;
  for ( top = topLevItems.begin(); top != topLevItems.end(); ++top )
    {
      mask_edited += "\"" + top.key().trimmed() + "\":";
      int children_lev1 = top.value()->childCount();
      if ( !children_lev1 )
	{
	  mask_edited += "\"" + QString::number( int(top.value()->checkState(0)) ) + "\",";
	}
      else
	{
	  mask_edited += "[{";
	  
	  //here we need to generalize for any tree nestedness: recursive
	  get_children_to_json( mask_edited, top.value() );

	  mask_edited += "}],";
	}
    }

  
  //ALEXEY: <-- little trick to enable super-fast recursive over arbitrary tree:))
  mask_edited.replace(",}],","}],"); 
  QString to_replace = "}],";
  QString new_substr = "}]";
  if ( ! mask_edited.mid( mask_edited.lastIndexOf( to_replace ) +  to_replace.size(), mask_edited.length() ) .contains( "\"" ) )
    mask_edited.replace( mask_edited.lastIndexOf( to_replace ), to_replace.size(), new_substr );

  mask_edited += "}";
  mask_edited.replace( ",}", "}" );

  qDebug() << "Edited Mask: " << mask_edited;

  return mask_edited;
}


/*
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
*/

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
  // QDate dNow(QDate::currentDate());
  // QString fmt = "MM/dd/yyyy";
  
  // current_date = dNow.toString( fmt );
  
  QDateTime date = QDateTime::currentDateTimeUtc();
  current_date = date.toString("MM/dd/yyyy hh:mm:ss") + " (UTC)";

  qDebug() << "Current date -- " << current_date;
}



//assemble parts of the PDF based on mask
void US_ReporterGMP::assemble_parts( QString & html_str )
{
  //info on failed analyses
  html_str += html_failed;
  
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
      if ( top.key().contains("Cell") && top.value() )
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

