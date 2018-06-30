//! \file us_experiment_main.cpp

#include "us_com_project_gui.h"
#include "us_rotor_gui.h"
#include "us_solution_gui.h"
#include "us_extinction_gui.h"
#include "us_table.h"
#include "us_xpn_data.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_sleep.h"
#include "us_util.h"
#include "us_crypto.h"
#include "us_select_item.h"
#include "us_images.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif


// Constructor:  build the main layout with tab widget panels
US_ComProjectMain::US_ComProjectMain() : US_Widgets()
{
  //   dbg_level    = US_Settings::us_debug();
   curr_panx    = 0;

   setWindowTitle( tr( "Commercial Project" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   //main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   //QGridLayout* statL     = new QGridLayout();
   //QHBoxLayout* buttL     = new QHBoxLayout();

   gen_banner = us_banner( tr( "TEST PROGRAM, v. 0.1" ) );
   //gen_banner = new QLabel;
   //gen_banner->setText("TEST PROGRAM, v. 0.1");
   //gen_banner->setAlignment(Qt::AlignCenter);
   //gen_banner->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
   
   //set font
   QFont font_gen = gen_banner->font();
   font_gen.setPointSize(20);
   font_gen.setBold(true);
   gen_banner->setFont(font_gen);
   
   main->addWidget(gen_banner);

   welcome = new QTextEdit;
   welcome->setText(" <br> Welcome to the TEST PROGRAM <br> for setting up, monitoring, editing and analyzing <br> AUC experiments and produced data... <br>");
   //welcome->setMaximumHeight(120);
   welcome->setReadOnly(true);
   welcome->setAlignment(Qt::AlignCenter);
   QFont font_wel = welcome->font();
   font_wel.setPointSize(10);
   font_wel.setItalic(true);
   font_wel.setBold(true);
   welcome->setStyleSheet("color: black; background-color: #979aaa;");
   welcome->setFont(font_wel);
   QFontMetrics m (welcome -> font()) ;
   int RowHeight = m.lineSpacing() ;
   welcome -> setFixedHeight  (6* RowHeight) ;
   
   main->addWidget(welcome);

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   //tabWidget           = new QTabWidget;
   tabWidget->setTabPosition( QTabWidget::West );
   tabWidget->tabBar()->setStyle(new VerticalTabStyle);
   
   epanExp             = new US_ExperGui   ( this );
   epanObserv          = new US_ObservGui  ( this );
   epanPostProd        = new US_PostProdGui( this );
   //   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanExp,       tr( "1: Experiment"   ) );
   tabWidget->addTab( epanObserv,    tr( "2: Live Update" ) );
   tabWidget->addTab( epanPostProd,  tr( "3: Post Production"  ) );
   tabWidget->setCurrentIndex( curr_panx );
   tabWidget->tabBar()->setFixedHeight(500);
   
   //icon_path = std::getenv("ULTRASCAN");
   //qDebug() << "Path is: " << icon_path;
   //icon_path.append("/etc/"); 
   //tabWidget->setTabIcon(0,QIcon(icon_path + "setup.png"));
   //tabWidget->setTabIcon(1,QIcon(icon_path + "live_update.gif"));
   //tabWidget->setTabIcon(2,QIcon(icon_path + "analysis.png"));
   
   tabWidget->setTabIcon( 0, US_Images::getIcon( US_Images::SETUP_COM  ) );
   tabWidget->setTabIcon( 1, US_Images::getIcon( US_Images::LIVE_UPDATE_COM  ) );
   tabWidget->setTabIcon( 2, US_Images::getIcon( US_Images::ANALYSIS_COM ) );
   
   tabWidget->tabBar()->setIconSize(QSize(50,50));

   tabWidget->tabBar()->setStyleSheet("QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:hover {background: lightgray;}");
   main->addWidget( tabWidget );
   
   logWidget = us_textedit();;
   logWidget->setMaximumHeight(60);
   logWidget->setReadOnly(true);
   logWidget->append("Log comes here...");
   logWidget->verticalScrollBar()->setValue(logWidget->verticalScrollBar()->maximum());
   main->addWidget( logWidget );
   
   test_footer = new QTextEdit;
   test_footer->setText("Test footer: by AUC solutions");
   test_footer->setTextColor(Qt::white);
   test_footer->setMaximumHeight(30);
   test_footer->setReadOnly(true);
   test_footer->setStyleSheet("color: #D3D9DF; background-color: #36454f;");
   main->addWidget( test_footer );

   connect( epanExp, SIGNAL( switch_to_live_update( QMap < QString, QString > &) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( this   , SIGNAL( pass_to_live_update( QMap < QString, QString > &) ),   epanObserv, SLOT( process_protocol_details( QMap < QString, QString > & )  ) );
   
   
   setMinimumSize( QSize( 1225, 825 ) );
   adjustSize();

}

// Slot to pass submitted to Optima run info to the Live Update tab
void US_ComProjectMain::switch_to_live_update( QMap < QString, QString > & protocol_details)
{
  tabWidget->setCurrentIndex( 1 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ?? 

   emit pass_to_live_update( protocol_details );
}
     

// US_ExperGUI
US_ExperGui::US_ExperGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   //lb_exp_banner    = us_banner( tr( "1: Define Experiment Run" ) );
   //panel->addWidget( lb_exp_banner );

   // Create layout and GUI components
   QGridLayout* genL   = new QGridLayout();
   pb_openexp = new QPushButton();
   
   // QString temp_path = std::getenv("ULTRASCAN");
   // temp_path += "/etc/";
   
   // //pb_openexp  = us_pushbutton( tr( "Open Experiment SetUp Dialog" ) );
   // pb_openexp  = new QPushButton(this);
   // //xpb_openexp->setGeometry(QRect(0, 0, 150, 100));
   // pb_openexp->setFixedSize( QSize(285, 55) );
   // pb_openexp->setText("  Setup New Experiment ");
   // pb_openexp->setLayoutDirection(Qt::RightToLeft);
   // pb_openexp->setIcon(QIcon(temp_path + "new_item.png"));
   // pb_openexp->setIconSize(QSize(20,20));
   
   // qDebug() << "Icon: " << temp_path + "new_item.png";
   // pb_openexp->setStyleSheet("QPushButton{background-color: #318CE7; border: 1px solid white; background-image: url(temp_path); color: black; border-radius: 15px; font-size: 18px; font-weight: bold; } QPushButton:hover{background-color: #6699CC;} QPushButton:disabled{background-color:#6699CC ; color: white}");
   
   //QString iconpath = temp_path + "new_item2.png";
   //QPixmap* pixmap = new QPixmap(iconpath);
   //QIcon icon(*pixmap);
   //QSize iconSize(pixmap->width(), pixmap->height());
   //pb_openexp->setIconSize(iconSize);
   //pb_openexp->setIcon(icon);

   pb_openexp->setIcon(US_Images::getIcon( US_Images::NEW_ITEM_COM ) );
   pb_openexp->setIconSize(QSize(50,50));
   pb_openexp->setStyleSheet("QPushButton{border: none} QPushButton:hover{border: 1px solid gray}");
   //pb_openexp->setStyleSheet("QPushButton{background-color: #318CE7; border: 1px solid white; background-image: url(temp_path); color: black; border-radius: 15px; font-size: 18px; font-weight: bold; } QPushButton:hover{background-color: #6699CC;} QPushButton:disabled{background-color:#6699CC ; color: white}");

   
   // Build main layout
   int row         = 0;

   //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   QTextEdit* panel_desc = new QTextEdit(this);
   panel_desc->viewport()->setAutoFillBackground(false);
   panel_desc->setFrameStyle(QFrame::NoFrame);
   panel_desc->setPlainText(" Tab to Set Up New Experiment...");
   panel_desc->setReadOnly(true);
   QFontMetrics m (panel_desc -> font()) ;
   int RowHeight = m.lineSpacing() ;
   panel_desc -> setFixedHeight  (2* RowHeight) ;
   
   genL->addWidget( panel_desc,  row,   0, 1, 12);
   //genL->addWidget( pb_openexp,  row,   12, 1, 1, Qt::AlignTop);
   // genL->addWidget( panel_desc,  row,   0, 1, 2);
   // genL->addWidget( pb_openexp,  row,   2, 1, 2, Qt::AlignCenter);

   panel->addLayout( genL );
   panel->addStretch();

   // connect( pb_openexp,      SIGNAL( clicked()          ), 
   //          this,            SLOT(   manageExperiment() ) );

   //manageExperiment();

   // Open US_Experiment without button...  
   US_ExperimentMain* sdiag = new US_ExperimentMain;
   sdiag->setParent(this, Qt::Widget);
   
   connect( sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
   connect( sdiag, SIGNAL( to_live_update( QMap < QString, QString > & ) ),
	    this,  SLOT( to_live_update( QMap < QString, QString > & ) ) );

   sdiag->pb_close->setEnabled(false);  // Disable Close button
   int offset = 20;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2); 
   
   sdiag->show();
     
}

//When run is submitted to Optima & protocol details are passed .. 
void US_ExperGui::to_live_update( QMap < QString, QString > & protocol_details)
{
  emit switch_to_live_update( protocol_details );
}

//When US_Experiment is closed
void US_ExperGui::us_exp_is_closed_set_button()
{
  pb_openexp->setEnabled(true);
  mainw->resize(QSize(1000, 700));
  mainw->logWidget->append("US_Experiment has been closed!");
}


// On click to open US_Experiment  <-- NOT USED, us_experimnet is loaded immediately
void US_ExperGui::manageExperiment()
{
  qDebug() << "test00";
  //mainw->logWidget->append("Opening US_Experiment...");
  //qApp->processEvents();

  //Calculate cumulative height/width of all widgets in Main && US_ExperGui
  int height_cum = 0;
  int offset = 20;

  qDebug() << "test0";
  
  height_cum += pb_openexp->height();
  height_cum += offset;
  
  int tab_width = mainw->tabWidget->tabBar()->width();  

  qDebug() << "test1";
  
  US_ExperimentMain* sdiag = new US_ExperimentMain;
  sdiag->setParent(this, Qt::Widget);

  connect(sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
  
  sdiag->move(offset, height_cum);
  sdiag->setFrameShape( QFrame::Box);
  sdiag->setLineWidth(2); 
  
  sdiag->show();

  qDebug() << "test2";
  
  int new_width, new_height;

  new_width  = tab_width  + sdiag->width()  + 2*offset;
  new_height = mainw->gen_banner->height() + mainw->welcome->height() + height_cum + sdiag->height() + mainw->logWidget->height() + mainw->test_footer->height() + offset;

  qDebug() << "Tab width: " << tab_width << ", US_Exp width: " << sdiag->width();
  qDebug() << "New Width: " << new_width;
  qDebug() << "New Height: " << new_height;
  
  if ( mainw->height() < new_height || mainw->width() < new_width)
    mainw->resize(new_width, new_height);

  pb_openexp->setEnabled(false);
  mainw->logWidget->append("US_Experiment opened successfully!");
}


// US_Observe /////////////////////////////////////////////////////////////////////////////////
US_ObservGui::US_ObservGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid     = fmet.maxWidth();
   int lwid     = fwid * 4;
   int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();
   //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   QTextEdit* panel_desc = new QTextEdit(this);
   panel_desc->viewport()->setAutoFillBackground(false);
   panel_desc->setFrameStyle(QFrame::NoFrame);
   panel_desc->setPlainText(" Tab to Monitor Experiment Progress...");
   panel_desc->setReadOnly(true);
   //panel_desc->setMaximumHeight(30);
   QFontMetrics m (panel_desc -> font()) ;
   int RowHeight = m.lineSpacing() ;
   panel_desc -> setFixedHeight  (2* RowHeight) ;

   int row = 0;
   genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Xpn_Viewer ...  
   US_XpnDataViewer* sdiag = new US_XpnDataViewer("AUTO");
   sdiag->setParent(this, Qt::Widget);

   connect( this, SIGNAL( to_xpn_viewer( QMap < QString, QString > &) ), sdiag, SLOT( check_for_data ( QMap < QString, QString > & )  ) );
   
   int offset = 20;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();

}

// Live Update's get and use submitted run's protocol details
void US_ObservGui::process_protocol_details( QMap < QString, QString > & protocol_details )
{
  // Use protocol details to retrieve data from Optima's DB
  // Query ExperimentRun table for runname/ExpDefId
  // If null (i.e. run is not launched yet), Information box - "Run was submitted, but not launched yet.. Awaiting for data to emerge."

  // QString mtitle    = tr( "Reading Protocol" );
  // QString message   = tr( "Protocol details passed. <br> Name: %1 <br> ID: %2" ).arg(protocol_details["experimentName"]).arg(protocol_details["experimentId"]);
  // QMessageBox::information( this, mtitle, message );

  emit to_xpn_viewer( protocol_details );
}


// US_PostProd
US_PostProdGui::US_PostProdGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;
   
}
