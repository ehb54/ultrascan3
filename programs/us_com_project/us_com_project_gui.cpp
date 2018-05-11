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
   
   setMinimumSize( QSize( 1225, 825 ) );
   adjustSize();

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
   
   connect(sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );

   sdiag->pb_close->setEnabled(false);  // Disable Close button
   int offset = 20;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2); 
   
   sdiag->show();
     
}


//When US_Experiment is closed
void US_ExperGui::us_exp_is_closed_set_button()
{
  pb_openexp->setEnabled(true);
  mainw->resize(QSize(1000, 700));
  mainw->logWidget->append("US_Experiment has been closed!");
}


// On click to open US_Experiment
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

// US_Observe
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
      
   // VBox
   QVBoxLayout* upper     = new QVBoxLayout;
   QGridLayout* genL   = new QGridLayout();
   //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   QTextEdit* panel_desc = new QTextEdit(this);
   panel_desc->viewport()->setAutoFillBackground(false);
   panel_desc->setFrameStyle(QFrame::NoFrame);
   panel_desc->setPlainText(" Tab to Monitor Experiment Execution...");
   panel_desc->setReadOnly(true);
   //panel_desc->setMaximumHeight(30);
   QFontMetrics m (panel_desc -> font()) ;
   int RowHeight = m.lineSpacing() ;
   panel_desc -> setFixedHeight  (2* RowHeight) ;

   int row = 0;
   genL->addWidget( panel_desc,  row++,   0, 1, 12);
   upper->addLayout( genL );

   // Hbox
   QHBoxLayout* panel  = new QHBoxLayout;
   panel->setContentsMargins( 20, 2, 2, 2 );
   
   plot             = new US_Plot( data_plot,
                                   tr( "Intensity Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Intensity" ) );

   //data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );

   picker = new US_PlotPicker( data_plot );
   picker->setRubberBand     ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   //connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
   //         this, SLOT  ( currentRectf ( QRectF ) ) );

   
   QGridLayout* settings = new QGridLayout;

   // Plot controls
   const QChar chlamb( 955 );
   QLabel*      lb_prcntls  = us_banner( tr( "Plot Controls" ) );
   QLabel*      lb_rstart   = us_label( tr( "Radius Start:"   ), -1 );
                cb_rstart   = us_comboBox();
   QLabel*      lb_rend     = us_label( tr( "Radius End:"     ), -1 );
                cb_rend     = us_comboBox();
   QLabel*      lb_optsys   = us_label( tr( "Optical System:" ), -1 );
                cb_optsys   = us_comboBox();
   QLabel*      lb_cellchn  = us_label( tr( "Cell/Channel:" ), -1 );
                cb_cellchn  = us_comboBox();
   QLabel*      lb_lrange   = us_label( tr( "%1 Range:"   ).arg( chlamb ), -1 );
                le_lrange   = us_lineedit( "280 only", -1, true );

		ptype_mw     = tr( "Plot %1:"    ).arg( chlamb );
		ptype_tr     = tr( "Plot Triple:" );
		prectype     = ptype_tr;
		lb_pltrec   = us_label( prectype, -1 );
                cb_pltrec   = us_comboBox();

		pb_prev     = us_pushbutton( tr( "Previous" ) );
                pb_next     = us_pushbutton( tr( "Next" ) );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

   // Scan controls
   int rhgt     = le_lrange->height();
   QLabel*      lb_scanctl  = us_banner( tr( "Scan Control" ) );
   QLabel*      lb_from     = us_label( tr( "From:" ) );
   QLabel*      lb_to       = us_label( tr( "To:" ) );
                ct_from     = us_counter( 2, 0, 500, 1 );
                ct_to       = us_counter( 2, 0, 500, 1 );
                pb_exclude  = us_pushbutton( tr( "Exclude Scan Range" ) );
                pb_include  = us_pushbutton( tr( "Include All Scans"  ) );
   ct_from  ->setFont( sfont );
   ct_from  ->setMinimumWidth( lwid );
   ct_from  ->resize( rhgt, swid );
   ct_to    ->setFont( sfont );
   ct_to    ->setMinimumWidth( lwid );
   ct_to    ->resize( rhgt, swid );
   ct_from  ->setValue( 0 );
   ct_to    ->setValue( 0 );
   ct_from  ->setSingleStep( 1 );
   ct_to    ->setSingleStep( 1 );

   // Status
   QLabel*      lb_status   = us_banner( tr( "Status" ) );
                le_status   = us_lineedit( tr( "(no data loaded)" ), -1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );
		

   
   settings->addWidget( lb_prcntls,    row++, 0, 1, 8 );
   settings->addWidget( lb_rstart,     row,   0, 1, 2 );
   settings->addWidget( cb_rstart,     row,   2, 1, 2 );
   settings->addWidget( lb_rend,       row,   4, 1, 2 );
   settings->addWidget( cb_rend,       row++, 6, 1, 2 );
   settings->addWidget( lb_optsys,     row,   0, 1, 4 );
   settings->addWidget( cb_optsys,     row++, 4, 1, 4 );
   settings->addWidget( lb_cellchn,    row,   0, 1, 2 );
   settings->addWidget( cb_cellchn,    row,   2, 1, 2 );
   settings->addWidget( lb_lrange,     row,   4, 1, 2 );
   settings->addWidget( le_lrange,     row++, 6, 1, 2 );
   settings->addWidget( lb_pltrec,     row,   0, 1, 2 );
   settings->addWidget( cb_pltrec,     row,   2, 1, 2 );
   settings->addWidget( pb_prev,       row,   4, 1, 2 );
   settings->addWidget( pb_next,       row++, 6, 1, 2 );

   settings->addWidget( lb_scanctl,    row++, 0, 1, 8 );
   settings->addWidget( lb_from,       row,   0, 1, 1 );
   settings->addWidget( ct_from,       row,   1, 1, 3 );
   settings->addWidget( lb_to,         row,   4, 1, 1 );
   settings->addWidget( ct_to,         row++, 5, 1, 3 );
   settings->addWidget( pb_exclude,    row,   0, 1, 4 );
   settings->addWidget( pb_include,    row++, 4, 1, 4 );

   settings->addWidget( lb_status,     row++, 0, 1, 8 );
   settings->addWidget( le_status,     row++, 0, 1, 8 );
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   
   QVBoxLayout* right    = new QVBoxLayout;
   
   right->addLayout( plot );

   panel->addLayout( left );
   panel->addLayout( right );

   panel->setStretch( 0, 2 );
   panel->setStretch( 1, 4 );

   // assemble main
   main->addLayout(upper);
   main->addLayout(panel);
   
   adjustSize();
   
}

// US_PostProd
US_PostProdGui::US_PostProdGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;
   
}
