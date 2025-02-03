//! \file us_experiment_main.cpp
/////////////////////////////////////////
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
//#include "us_select_item.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define QRegularExpression(a)  QRegExp(a)
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//// Constructor:  ACADEMIC: 'Data Acquision' / us_comproject_academic programs
US_ComProjectMain::US_ComProjectMain(QString us_mode) : US_Widgets()
{
  //   dbg_level    = US_Settings::us_debug();

  //-- Check if Database is selected in the DB preferences
  checkDataLocation();
  // --------------------------------------------------------

   curr_panx    = 0;
   window_closed = false;
   xpn_viewer_closed_soft = false;  
   
   us_mode_bool = true;

   setWindowTitle( tr( "UltraScan Optima AUC Interface" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   //main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   //QGridLayout* statL     = new QGridLayout();
   //QHBoxLayout* buttL     = new QHBoxLayout();

   gen_banner = us_banner( tr( "UltraScan R&D Data Acquisition Program" ) );
   //gen_banner = new QLabel;
   //gen_banner->setText("UltraScan GMP, v. 0.1");
   //gen_banner->setAlignment(Qt::AlignCenter);
   //gen_banner->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
   
   //set font
   QFont font_gen = gen_banner->font();
   font_gen.setPointSize(20);
   font_gen.setBold(true);
   gen_banner->setFont(font_gen);
   
   main->addWidget(gen_banner);

   welcome = new QTextEdit;
   welcome->setText(" <br> Welcome to UltraScan GMP<br> for setting up, monitoring, editing and analyzing <br> AUC experiments and produced data... <br>");
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
   
   //main->addWidget(welcome);

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   //tabWidget           = new QTabWidget;
   tabWidget->setTabPosition( QTabWidget::West );
   tabWidget->tabBar()->setStyle(new VerticalTabStyle);

   epanInit            = new US_InitDialogueGui   ( this );
   epanExp             = new US_ExperGui   ( this );
   epanObserv          = new US_ObservGui  ( this );
   epanPostProd        = new US_PostProdGui( this );
   // epanAnalysis        = new US_AnalysisGui( this );
   // epanReport          = new US_ReportStageGui  ( this );
   
   //   statflag            = 0;

   // Add panels to the tab widget
   tabWidget->addTab( epanInit,      tr( "Manage Optima Runs"   ) );
   tabWidget->addTab( epanExp,       tr( "1: Experiment"   ) );
   tabWidget->addTab( epanObserv,    tr( "2: Live Update" ) );
   tabWidget->addTab( epanPostProd,  tr( "3: LIMS Import"  ) );
   // tabWidget->addTab( epanAnalysis,  tr( "4: Analysis"  ) );
   // tabWidget->addTab( epanReport,    tr( "5: Report"  ) );

   
   // tabWidget->addTab( epanExit,  tr( "Close Program"  ) );
   // QPushButton *b1 = new QPushButton("Close");
   // connect( b1, SIGNAL( clicked() ), this, SLOT( close()  ) );
   // tabWidget->addTab( b1,      tr( "Close"   ) );

   // QToolButton* m_exit = new QToolButton();
   // m_exit->setText("Close");
   // //m_exit->setIcon(QIcon(":/Resources/exit.png"));
   // //m_exit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
   // tabWidget->setCornerWidget(m_exit, Qt::TopRightCorner);
   
   tabWidget->setCurrentIndex( curr_panx );
   tabWidget->tabBar()->setFixedHeight(400);
   //tabWidget->tabBar()->setFixedWidth(200);
   
   //icon_path = std::getenv("ULTRASCAN");
   //qDebug() << "Path is: " << icon_path;
   //icon_path.append("/etc/"); 
   //tabWidget->setTabIcon(0,QIcon(icon_path + "setup.png"));
   //tabWidget->setTabIcon(1,QIcon(icon_path + "live_update.gif"));
   //tabWidget->setTabIcon(2,QIcon(icon_path + "analysis.png"));
   
   tabWidget->setTabIcon( 1, US_Images::getIcon( US_Images::SETUP_COM  ) );
   tabWidget->setTabIcon( 2, US_Images::getIcon( US_Images::LIVE_UPDATE_COM  ) );
   tabWidget->setTabIcon( 3, US_Images::getIcon( US_Images::IMPORT_COM_1 ) );
   // tabWidget->setTabIcon( 3, US_Images::getIcon( US_Images::ANALYSIS_COM_2 ) );
   // tabWidget->setTabIcon( 4, US_Images::getIcon( US_Images::REPORT_COM ) );

   // if ( us_mode.toStdString() == "US_MODE")
   //   {
   //     tabWidget->removeTab(3);
   //     tabWidget->removeTab(3);
   //   }
   
   tabWidget->tabBar()->setIconSize(QSize(50,50));

   //tabWidget->tabBar()->setStyleSheet( "QTabBar::tab {min-width: 70;} QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:hover {background: lightgray;} QTabBar::tab:first {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:first:hover {background: #4169E1; color: white}  QTabBar::tab:disabled { color: rgba(0, 0, 0, 70%) } ");

   //no hoover
   tabWidget->tabBar()->setStyleSheet( "QTabBar::tab {min-width: 70;} QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:first {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:first:hover {background: #4169E1; color: white}   QTabBar::tab:disabled { color: rgba(0, 0, 0, 70%) } ");
   //tabWidget->tabBar()->setStyleSheet( "QTabBar::tab {min-width: 70;} QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:first {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:first:hover {background: #4169E1; color: white}  QTabBar::tab:last {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:last:hover {background: #4169E1; color: white} QTabBar::tab:disabled { color: rgba(0, 0, 0, 70%) } ");


   //Close & Help
   cornerWidget = new QWidget(tabWidget);
   auto *hbox = new QVBoxLayout(cornerWidget);
   auto *m_help = us_pushbutton( tr( "Help" ) );
   auto *m_exit = us_pushbutton( tr( "Exit" ) );
   connect( m_exit, SIGNAL( clicked() ), this, SLOT( close()  ) );
   //connect( m_help, SIGNAL( clicked() ), this, SLOT( help_m()  ) );
   
   hbox->addWidget(m_help);
   hbox->addWidget(m_exit);
   hbox->setSpacing(1);
   hbox->setContentsMargins(1, 1, 1, 1);

   QFont font_t = tabWidget->property("font").value<QFont>();
   qDebug() << font_t.family() << font_t.pointSize();
   QFontMetrics fm_t(font_t);
   int pixelsWide = fm_t.width("Manage Optima Runs");
   
   qDebug() << "FONT_T: fm_t.width() -- " <<  pixelsWide;
   cornerWidget->setMinimumWidth( pixelsWide );

   //cornerWidget->setFixedWidth( tabWidget->tabBar()->width() - 20 );
   //hbox->setStretch(0,1);
 
   qDebug() << "TabWidget position: " << tabWidget->x() << tabWidget->y();
   qDebug() << "TabWidget size    : " << tabWidget->width() << tabWidget->height();
   qDebug() << "TabWidget->tabBar position: " << tabWidget->tabBar()->x() << tabWidget->tabBar()->y();
   qDebug() << "TabWidget->tabBar size    : " << tabWidget->tabBar()->width() << tabWidget->tabBar()->height();

   int pos_x_offset = fm_t.width("M");
   int pos_x = tabWidget->tabBar()->x() + pos_x_offset*1.2;
   //int pos_x = (tabWidget->tabBar()->width())/4;
   int pos_y = (tabWidget->tabBar()->height())*1.22;
   qDebug() << "pos_x, pos_y: " << pos_x << pos_y;
   //m_exit->move(pos_x, pos_y);
   cornerWidget->move(pos_x, pos_y);
   // End of Help & Exit
        
   main->addWidget( tabWidget );

   for (int i=0; i < tabWidget->count(); ++i )
     {
       //ALEXEY: OR enable all tabs ? (e.g. for demonstration, in a read-only mode or the like ?)
       if ( i == 0 ) 
   	 tabWidget->tabBar()->setTabEnabled(i, true);
       else
   	 tabWidget->tabBar()->setTabEnabled(i, false);
     }

   connect( tabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( initPanels( int ) ) );

   // QPushButton* pb_close_program = us_pushbutton( tr( "CLOSE PROGRAM"));
   // pb_close_program->setStyleSheet( "QPushButton { background: red; color: lightgray; min-height: 20 } QPushButton::hover {  background-color: #9E0606; color: white } ");
   // connect( pb_close_program, SIGNAL( clicked() ), this, SLOT( close()  ) );
   // main->addWidget( pb_close_program );


   logWidget = us_textedit();
   logWidget->setMaximumHeight(30);
   logWidget->setReadOnly(true);
   logWidget->append("Log comes here...");
   logWidget->verticalScrollBar()->setValue(logWidget->verticalScrollBar()->maximum());
   logWidget->hide();
   //main->addWidget( logWidget );
   
   test_footer = new QTextEdit;
   test_footer->setText("UltraScan by AUC Solutions");
   test_footer->setTextColor(Qt::white);
   test_footer->setMaximumHeight(30);
   test_footer->setReadOnly(true);
   test_footer->setStyleSheet("color: #D3D9DF; background-color: #36454f;");
   main->addWidget( test_footer );

   connect( epanInit, SIGNAL( define_new_experiment_init( QStringList & ) ), this, SLOT( define_new_experiment( QStringList &)  ) );
   connect( epanInit, SIGNAL( switch_to_live_update_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_post_processing_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
   
   connect( this, SIGNAL( pass_used_instruments( QStringList & ) ), epanExp, SLOT( pass_used_instruments( QStringList &)  ) );
   
   connect( epanExp, SIGNAL( switch_to_live_update( QMap < QString, QString > &) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( switch_to_import( QMap < QString, QString > &) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & )  ) );
   connect( this   , SIGNAL( pass_to_live_update( QMap < QString, QString > &) ),   epanObserv, SLOT( process_protocol_details( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( to_autoflow_records( ) ), this, SLOT( to_autoflow_records( ) ) );
   connect( epanExp, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
   
   connect( epanObserv, SIGNAL( switch_to_post_processing( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & ) ) );
   connect( this, SIGNAL( pass_to_post_processing( QMap < QString, QString > & ) ),  epanPostProd, SLOT( import_data_us_convert( QMap < QString, QString > & )  ) );
   
   //connect( this, SIGNAL( clear_experiment( QString & ) ),  epanExp, SLOT( clear_experiment( QString & )  ) );
   connect( epanObserv, SIGNAL( close_everything() ), this, SLOT( close_all() ));
   connect( this, SIGNAL( reset_live_update() ),  epanObserv, SLOT( reset_live_update( )  ) );
   connect( epanObserv, SIGNAL( processes_stopped() ), this, SLOT( liveupdate_stopped() ));
   connect( epanObserv, SIGNAL( stop_nodata() ), this, SLOT( close_all() ));
   
   connect( epanPostProd, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
   connect( this, SIGNAL( reset_lims_import() ),  epanPostProd, SLOT( reset_lims_import( )  ) );
   
   setMinimumSize( QSize( 1350, 800 ) );
   adjustSize();

}



//// Constructor:  COMMERCIAL: us_comproject
US_ComProjectMain::US_ComProjectMain() : US_Widgets()
{
  //   dbg_level    = US_Settings::us_debug();

  //update user-level as set in DB
  US_Passwd   pw;
  US_DB2      db( pw.getPasswd() );
  
  if ( db.lastErrno() != US_DB2::OK )
    {
      //qDebug() << "USCFG: UpdInv: ERROR connect";
      QMessageBox msgBox;
      msgBox.setWindowTitle ("ERROR");
      msgBox.setText("Error making the DB connection!");
      msgBox.setIcon  ( QMessageBox::Critical );
      msgBox.exec();
     
      exit( -1 );
    }
  
  QStringList q( "get_user_info" );
  db.query( q );
  db.next();
  
  int ID        = db.value( 0 ).toInt();
  QString fname = db.value( 1 ).toString();
  QString lname = db.value( 2 ).toString();
  int     level = db.value( 5 ).toInt();

  qDebug() << "USCFG: UpdInv: ID,name,lev" << ID << fname << lname << level;
  //if(ID<1) return;
  
  US_Settings::set_us_inv_name ( lname + ", " + fname );
  US_Settings::set_us_inv_ID   ( ID );
  US_Settings::set_us_inv_level( level );
  //END OF user-level check/update
  
  //-- Check if Database is selected in the DB preferences
  checkDataLocation();
  // --------------------------------------------------------
   
   curr_panx    = 0;

   window_closed = false;
   xpn_viewer_closed_soft = false;  
   
   us_mode_bool = false;
   
   setWindowTitle( tr( "UltraScan Optima AUC Interface" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   //main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );
   //QGridLayout* statL     = new QGridLayout();
   //QHBoxLayout* buttL     = new QHBoxLayout();

   gen_banner = us_banner( tr( "UltraScan GMP" ) );
   //gen_banner = new QLabel;
   //gen_banner->setText("UltraScan GMP, v. 0.1");
   //gen_banner->setAlignment(Qt::AlignCenter);
   //gen_banner->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
   
   //set font
   QFont font_gen = gen_banner->font();
   font_gen.setPointSize(20);
   font_gen.setBold(true);
   gen_banner->setFont(font_gen);
   
   main->addWidget(gen_banner);

   welcome = new QTextEdit;
   welcome->setText(" <br> Welcome to UltraScan GMP<br> for setting up, monitoring, editing and analyzing <br> AUC experiments and produced data... <br>");
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
   
   //main->addWidget(welcome);

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
   //tabWidget           = new QTabWidget;
   tabWidget->setTabPosition( QTabWidget::West );
   tabWidget->tabBar()->setStyle(new VerticalTabStyle);

   epanInit            = new US_InitDialogueGui ( this );
   epanExp             = new US_ExperGui   ( this );
   epanObserv          = new US_ObservGui  ( this );
   epanPostProd        = new US_PostProdGui( this );
   epanEditing         = new US_EditingGui ( this );
   epanAnalysis        = new US_AnalysisGui( this );
   epanReport          = new US_ReportStageGui  ( this );
   epanSign            = new US_eSignaturesGui ( this );
      
   // Add panels to the tab widget
   tabWidget->addTab( epanInit,      tr( "Manage Optima Runs"   ) );
   tabWidget->addTab( epanExp,       tr( "1: Experiment"   ) );
   tabWidget->addTab( epanObserv,    tr( "2: Live Update" ) );
   tabWidget->addTab( epanPostProd,  tr( "3: LIMS Import"  ) );
   tabWidget->addTab( epanEditing,   tr( "4: Editing"  ) );
   tabWidget->addTab( epanAnalysis,  tr( "5: Analysis"  ) );
   tabWidget->addTab( epanReport,    tr( "6: Report"  ) );
   tabWidget->addTab( epanSign,      tr( "e-Signatures"  ) );

   
   tabWidget->setCurrentIndex( curr_panx );
   tabWidget->tabBar()->setFixedHeight(600);
  
   //icon_path = std::getenv("ULTRASCAN");
   //qDebug() << "Path is: " << icon_path;
   //icon_path.append("/etc/"); 
   //tabWidget->setTabIcon(0,QIcon(icon_path + "setup.png"));
   //tabWidget->setTabIcon(1,QIcon(icon_path + "live_update.gif"));
   //tabWidget->setTabIcon(2,QIcon(icon_path + "analysis.png"));
   
   tabWidget->setTabIcon( 1, US_Images::getIcon( US_Images::SETUP_COM  ) );
   tabWidget->setTabIcon( 2, US_Images::getIcon( US_Images::LIVE_UPDATE_COM  ) );
   tabWidget->setTabIcon( 3, US_Images::getIcon( US_Images::IMPORT_COM_1 ) );
   tabWidget->setTabIcon( 4, US_Images::getIcon( US_Images::EDITING_COM ) );
   tabWidget->setTabIcon( 5, US_Images::getIcon( US_Images::ANALYSIS_COM_2 ) );
   tabWidget->setTabIcon( 6, US_Images::getIcon( US_Images::REPORT_COM ) );
   

   tabWidget->tabBar()->setIconSize(QSize(50,50));

   //no hoover
   tabWidget->tabBar()->setStyleSheet( "QTabBar::tab {min-width: 70;} QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:first {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:first:hover {background: #4169E1; color: white}  QTabBar::tab:disabled { color: rgba(0, 0, 0, 70%)  } QTabBar::tab:last:disabled {background: #90EE90;  min-width: 50;}  QTabBar::tab:last:selected {background: #556B2F; color: lightgray;  min-width: 50;}");

   //Close & Help
   cornerWidget = new QWidget(tabWidget);
   auto *hbox = new QVBoxLayout(cornerWidget);
   auto *m_help = us_pushbutton( tr( "Help" ) );
   auto *m_exit = us_pushbutton( tr( "Exit" ) );
   connect( m_exit, SIGNAL( clicked() ), this, SLOT( close()  ) );
   //connect( m_help, SIGNAL( clicked() ), this, SLOT( help_m()  ) );
   
   hbox->addWidget(m_help);
   hbox->addWidget(m_exit);
   hbox->setSpacing(1);
   hbox->setContentsMargins(1, 1, 1, 1);

   QFont font_t = tabWidget->property("font").value<QFont>();
   qDebug() << font_t.family() << font_t.pointSize();
   QFontMetrics fm_t(font_t);
   int pixelsWide = fm_t.width("Manage Optima Runs");
   
   qDebug() << "FONT_T: fm_t.width() -- " <<  pixelsWide;
   cornerWidget->setMinimumWidth( pixelsWide );

   //cornerWidget->setFixedWidth( tabWidget->tabBar()->width() - 20 );
   //hbox->setStretch(0,1);
 
   qDebug() << "TabWidget position: " << tabWidget->x() << tabWidget->y();
   qDebug() << "TabWidget size    : " << tabWidget->width() << tabWidget->height();
   qDebug() << "TabWidget->tabBar position: " << tabWidget->tabBar()->x() << tabWidget->tabBar()->y();
   qDebug() << "TabWidget->tabBar size    : " << tabWidget->tabBar()->width() << tabWidget->tabBar()->height();

   int pos_x_offset = fm_t.width("M");
   int pos_x = tabWidget->tabBar()->x() + pos_x_offset*1.2;
   //int pos_x = (tabWidget->tabBar()->width())/4;
   int pos_y = (tabWidget->tabBar()->height())*1.08;
   qDebug() << "pos_x, pos_y: " << pos_x << pos_y;
   //m_exit->move(pos_x, pos_y);
   cornerWidget->move(pos_x, pos_y);
   // End of Help & Exit
     
   main->addWidget( tabWidget );

   for (int i=0; i < tabWidget->count(); ++i )
     {
       //ALEXEY: OR enable all tabs ? (e.g. for demonstration, in a read-only mode or the like ?)
       if ( i == 0 )
	 tabWidget->tabBar()->setTabEnabled(i, true);
       else
    	 tabWidget->tabBar()->setTabEnabled(i, false);
     }
   
   connect( tabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( initPanels( int ) ) );

   // QPushButton* pb_close_program = us_pushbutton( tr( "CLOSE PROGRAM"));
   // pb_close_program->setStyleSheet( "QPushButton { background: red; color: lightgray; min-height: 20 } QPushButton::hover {  background-color: #9E0606; color: white } ");
   // connect( pb_close_program, SIGNAL( clicked() ), this, SLOT( close()  ) );
   // main->addWidget( pb_close_program );

   
      
   logWidget = us_textedit();
   logWidget->setMaximumHeight(15);
   logWidget->setReadOnly(true);
   logWidget->append("Log comes here...");
   logWidget->verticalScrollBar()->setValue(logWidget->verticalScrollBar()->maximum());
   logWidget->hide();
   //main->addWidget( logWidget );
   
   test_footer = new QTextEdit;
   test_footer->setText("UltraScan by AUC Solutions");
   test_footer->setTextColor(Qt::white);
   test_footer->setMaximumHeight(30);
   test_footer->setReadOnly(true);
   test_footer->setStyleSheet("color: #D3D9DF; background-color: #36454f;");
   main->addWidget( test_footer );


   connect( epanInit, SIGNAL( define_new_experiment_init( QStringList & ) ), this, SLOT( define_new_experiment( QStringList &)  ) );
   connect( epanInit, SIGNAL( switch_to_live_update_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_post_processing_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_editing_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_editing( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_analysis_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_analysis( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_report_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_report( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( switch_to_esign_init( QMap < QString, QString > & ) ), this, SLOT( switch_to_esign( QMap < QString, QString > & )  ) );
   connect( epanInit, SIGNAL( to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
         
   connect( this, SIGNAL( pass_used_instruments( QStringList & ) ), epanExp, SLOT( pass_used_instruments( QStringList &)  ) );
   
   connect( epanExp, SIGNAL( switch_to_live_update( QMap < QString, QString > &) ), this, SLOT( switch_to_live_update( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( switch_to_import( QMap < QString, QString > &) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & )  ) );
   connect( this   , SIGNAL( pass_to_live_update( QMap < QString, QString > &) ),   epanObserv, SLOT( process_protocol_details( QMap < QString, QString > & )  ) );
   connect( epanExp, SIGNAL( to_autoflow_records( ) ), this, SLOT( to_autoflow_records( ) ) );
   connect( epanExp, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
   
   connect( epanObserv, SIGNAL( switch_to_post_processing( QMap < QString, QString > & ) ), this, SLOT( switch_to_post_processing( QMap < QString, QString > & ) ) );
   connect( this, SIGNAL(  pass_to_post_processing( QMap < QString, QString > & ) ),  epanPostProd, SLOT( import_data_us_convert( QMap < QString, QString > & )  ) );
   connect( epanObserv, SIGNAL( close_everything() ), this, SLOT( close_all() ));
   connect( this, SIGNAL( reset_live_update() ),  epanObserv, SLOT( reset_live_update( )  ) );
   connect( epanObserv, SIGNAL( processes_stopped() ), this, SLOT( liveupdate_stopped() ));
   connect( epanObserv, SIGNAL( stop_nodata() ), this, SLOT( close_all() ));
   
   connect( epanPostProd, SIGNAL( switch_to_editing( QMap < QString, QString > & ) ),  this, SLOT( switch_to_editing ( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( pass_to_editing( QMap < QString, QString > & ) ),   epanEditing, SLOT( do_editing( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( reset_lims_import() ),  epanPostProd, SLOT( reset_lims_import( )  ) );
   connect( epanPostProd, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );
   connect( this, SIGNAL( reset_data_editing() ),  epanEditing, SLOT( reset_data_editing( )  ) );

   connect( epanEditing, SIGNAL( switch_to_analysis( QMap < QString, QString > & ) ), this, SLOT( switch_to_analysis( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( pass_to_analysis( QMap < QString, QString > & ) ),   epanAnalysis, SLOT( do_analysis( QMap < QString, QString > & )  ) );
   connect( epanEditing, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );

   connect( epanAnalysis, SIGNAL( processes_stopped() ), this, SLOT( analysis_update_stopped() ));
   connect( epanAnalysis, SIGNAL( switch_to_initAutoflow( ) ), this, SLOT( close_all( )  ) );

   connect( epanAnalysis, SIGNAL( switch_to_report( QMap < QString, QString > & ) ), this, SLOT( switch_to_report( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( pass_to_report( QMap < QString, QString > & ) ),   epanReport, SLOT( do_report( QMap < QString, QString > & )  ) );

   connect( this, SIGNAL( reset_reporting() ),  epanReport, SLOT( reset_reporting( )  ) );

   //E-Signs
   connect( epanReport, SIGNAL( switch_to_esign( QMap < QString, QString > & ) ), this, SLOT( switch_to_esign( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( pass_to_esign( QMap < QString, QString > & ) ),  epanSign, SLOT( do_esign( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( reset_esigning() ),  epanSign, SLOT( reset_esigning( )  ) );

   //Hide/disable eSigs tab:
   this->tabWidget->removeTab(7);
   tabWidget->tabBar()->setStyleSheet( "QTabBar::tab {min-width: 70;} QTabBar::tab:selected {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); } QTabBar::tab:first {background: blue; color: lightgray; min-width: 50;}  QTabBar::tab:first:hover {background: #4169E1; color: white}  QTabBar::tab:disabled { color: rgba(0, 0, 0, 70%) }");
   //this->tabWidget->setTabEnabled( 7, false );
     
   setMinimumSize( QSize( 1350, 850 ) );
   adjustSize();

   

 }


// Check if data location set to DB
void US_ComProjectMain::checkDataLocation( void )
{
  qDebug() << "Data Location ( DB==1; Disk==2 ) -- " << US_Settings::default_data_location();

  if ( US_Settings::default_data_location() == 2 ) //Disk 
    {
      data_location_disk = true;
      //window_closed = true;
      QMessageBox::warning( this, tr( "Data Location set to Disk" ),
			    tr( "Please change Data Location to \"Database\" in the configuraiton of your database! \n" ) );
      
      exit(1);
    }
  
  else if ( US_Settings::default_data_location() == 1 ) //DB
    {
      data_location_disk = false;
      
      //Check DB connection: exit if no connection..
      US_Passwd pw;
      QString masterpw = pw.getPasswd();
      US_DB2* db = new US_DB2( masterpw );
      
      if ( db->lastErrno() != US_DB2::OK )
	{
	  QMessageBox::warning( this, tr( "LIMS DB Connection Problem" ),
				tr( "Could not connect to database \n" ) + db->lastError() );
	  exit(1);
	}
    }
}

// Slot to init some panels (mainly Manage Opima Runs)
void US_ComProjectMain::initPanels( int  panx )
{
  qDebug() << "NEWPanel panx=" << panx << "OLDPanel curr_panx=" << curr_panx;
  
  // if ( curr_panx == panx )
  //   return;
  
  if ( panx == 0 )  //Autoflow Records
    {
      // Depending on from we jump to InitDialogue, we may need to stop/reset current stages:
      // E.g., LIVE_UPDATE: stop all timers, reset GUI etc..
      // Maybe a warning message "You are going to leave this panel.." needs to be shown 
      
      if ( curr_panx == 1 )
	{
	  //Fully set: nothing needs to be done/reset:
	  qDebug() << "Jumping from EXPERIMENT.";
	}

      if ( curr_panx == 2 )
	{
	  // 2 cases:
	  /* 
	        1. Avaiting for run to be launched ("Back to Managing Optima Runs") -- SET
		2. More complex: Back to Managing runs from active LIVE_UPDATE stage -- stop all timers and other processes...
	  */

	  qDebug() << "Jumping from LIVE UPDATE.";

	  //2. Stop all timers/threads, reset GUI - when stopping fully working LIVE UPDATE
	  if ( !xpn_viewer_closed_soft )
	    {
	      show_liveupdate_finishing_msg();
	      
	      epanObserv->sdiag->reset_liveupdate_panel_public();
	      qApp->processEvents();

	      xpn_viewer_closed_soft = false;
	      //epanInit  ->initAutoflowPanel();

	      return;
	    }
	}

      if ( curr_panx == 3 )
	{
	  qDebug() << "Jumping from LIMS IMPORT.";
	  //Send signal to reset LIMS IMPORT tab... Fully SET
	  emit reset_lims_import();
	}
      
      if ( curr_panx == 4 )
	{
	  qDebug() << "Jumping from EDITING.";
	  emit reset_data_editing();
	}

            
      if ( curr_panx == 5 )
	{
	  qDebug() << "Jumping from ANALYSIS.";
	  show_analysis_update_finishing_msg();
	  
	  epanAnalysis->sdiag->reset_analysis_panel_public();
	  qApp->processEvents();

	  return;
	}

      if ( curr_panx == 6 )
	{
	  qDebug() << "Jumping from Report.";
	  emit reset_reporting();
	}

      if ( curr_panx == 7 )
	{
	  qDebug() << "Jumping from e-Signs.";
	  emit reset_esigning();
	}
      
      xpn_viewer_closed_soft = false;
      epanInit  ->initAutoflowPanel();
    }

  
  //curr_panx = panx;         // Set new current panel
}


// Slot to define new exp. (from initial dialog)
void US_ComProjectMain::show_liveupdate_finishing_msg( void )
{

   msg_liveupdate_finishing = new QMessageBox(this);
   msg_liveupdate_finishing->setIcon(QMessageBox::Information);
  
   msg_liveupdate_finishing->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
   msg_liveupdate_finishing->setStandardButtons(0);
   msg_liveupdate_finishing->setWindowTitle(tr("Updating..."));
   msg_liveupdate_finishing->setText(tr( "Finishing LIVE UPDATE processes... Please wait...") );
   msg_liveupdate_finishing->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
  
  
   int tab_width = this->tabWidget->tabBar()->width();
   int upper_height = this->gen_banner->height() + //this->welcome->height()
     + this->logWidget->height() + this->test_footer->height();

   int pos_x = this->width()/2 - tab_width;
   int pos_y = this->height()/2 - upper_height;     
   msg_liveupdate_finishing->move(pos_x, pos_y);
  
   msg_liveupdate_finishing->show();
  
   qApp->processEvents();

}

// 
void US_ComProjectMain::show_analysis_update_finishing_msg( void )
{
  
   msg_analysis_update_finishing = new QMessageBox(this);
   msg_analysis_update_finishing->setIcon(QMessageBox::Information);
  
   msg_analysis_update_finishing->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
   msg_analysis_update_finishing->setStandardButtons(0);
   msg_analysis_update_finishing->setWindowTitle(tr("Updating..."));
   msg_analysis_update_finishing->setText(tr( "Finishing ANALYSIS UPDATE processes... Please wait...") );
   msg_analysis_update_finishing->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
  
  
   int tab_width = this->tabWidget->tabBar()->width();
   int upper_height = this->gen_banner->height() + //this->welcome->height()
     + this->logWidget->height() + this->test_footer->height();

   int pos_x = this->width()/2 - tab_width;
   int pos_y = this->height()/2 - upper_height;     
   msg_analysis_update_finishing->move(pos_x, pos_y);
  
   msg_analysis_update_finishing->show();
  
   qApp->processEvents();

}

void US_ComProjectMain::liveupdate_stopped( void  )
{
  //Close message on finishing LIVE_UPDATE processes...
  msg_liveupdate_finishing->accept();
  
  epanInit  ->initAutoflowPanel();
}

void US_ComProjectMain::analysis_update_stopped( void  )
{
  //Close message on finishing ANALYSIS_UPDATE processes...
  msg_analysis_update_finishing->accept();
  
  epanInit  ->initAutoflowPanel();
}

void US_ComProjectMain::closeEvent( QCloseEvent* event )
{
    window_closed = true;

    qDebug() << "Closing 1: ";
    qDebug() << "data_location_disk: " <<  data_location_disk;

    if ( !data_location_disk )
      {
	qDebug() << "initDialogue: true/false 1 : " << epanInit->initDialogueOpen ;
	emit us_comproject_closed();
	close_initDialogue();
	qDebug() << "initDialogue: true/false 3 : " << epanInit->initDialogueOpen ;

	qApp->processEvents();
      }

    qApp->processEvents();
    
    event->accept();
}

void US_ComProjectMain::to_autoflow_records( void )
{
  //check_current_stage();
}


void US_ComProjectMain::close_initDialogue( void )
{
  qDebug() << "initDialogue: true/false 2 : " << epanInit->initDialogueOpen ;

  if ( epanInit-> initDialogueOpen)
    epanInit->pdiag_autoflow->close();
  if ( epanInit-> initMsgNorecOpen)
    epanInit->msg_norec->reject();
  //msg_norec->close();
}


//Slot to delete Postgres Optima ExperimentDefinition record
void US_ComProjectMain::delete_psql_record( int ExpId )
{
  QString schname( "AUC_schema" );
  
  QString tabname_expdef( "ExperimentDefinition" );
  QString tabname_fuge  ( "CentrifugeRunProfile" );
  QString tabname_abs   ( "AbsorbanceScanParameters" );
  
  QString qrytab_expdef  = "\"" + schname + "\".\"" + tabname_expdef + "\"";
  QString qrytab_fuge    = "\"" + schname + "\".\"" + tabname_fuge + "\"";
  QString qrytab_abs     = "\"" + schname + "\".\"" + tabname_abs + "\"";
  
  
  QString dbhost      = "demeler5.uleth.ca";
  int     dbport      = 5552;
  QString dbname      = "AUC_DATA_DB";
  QString dbuser      = "";
  QString dbpasw      = "";

  QSqlDatabase dbxpn;
  
  dbxpn           = QSqlDatabase::addDatabase( "QPSQL", "" );
  dbxpn.setDatabaseName( "XpnData" );
  dbxpn.setHostName    ( dbhost );
  dbxpn.setPort        ( dbport );
  dbxpn.setDatabaseName( dbname  );
  dbxpn.setUserName    ( dbuser  );
  dbxpn.setPassword    ( dbpasw );

  qDebug() << "Opening Postgres Connection!!!";
  
  if (  dbxpn.open() )
    {
      qDebug() << "Connected !!!";
      
      QSqlQuery query_expdef(dbxpn);
      QSqlQuery query_fuge(dbxpn);
      QSqlQuery query_abs_scan(dbxpn);
     
      /*
      // AbsorbanceScanParameters
      QString ScanId = "5996";
      if(! query_abs_scan.prepare(QString("DELETE FROM %1 WHERE \"ScanId\" = %2").arg(qrytab_abs).arg(ScanId) ) )
	qDebug() << query_abs_scan.lastError().text();

      if (query_abs_scan.exec())
	{
	  qDebug() << "AbsorbanceScanParameters record # :" << ScanId  << "deleted !";
	}
      else
	{
	  QString errmsg   = "Delete record error: " + query_abs_scan.lastError().text();
	  QMessageBox::critical( this,
				 tr( "*ERROR* in Deleting Absorbance Record" ),
				 tr( "An error occurred in the attempt to delete"
				     " AbsorbanceScanParameters from AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_abs ).arg( errmsg ) );
	  return;
	}
      
      // Cell Parameters
      
      
      // FugeProfile
      int FugeId = 308;
      if(! query_fuge.prepare(QString("DELETE FROM %1 WHERE \"FugeRunProfileId\" = %2").arg(qrytab_fuge).arg(FugeId) ) )
	qDebug() << query_fuge.lastError().text();

      if (query_fuge.exec())
	{
	  qDebug() << "FugeProfile record # :" << FugeId  << "deleted !";
	}
      else
	{
	  QString errmsg   = "Delete record error: " + query_fuge.lastError().text();;
	  QMessageBox::critical( this,
				 tr( "*ERROR* in Deleting Fuge Profile" ),
				 tr( "An error occurred in the attempt to delete"
				     " FugeProfile from AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_fuge ).arg( errmsg ) );
	  return;
	}

      */

      // ExperimentalDefinition
      if(! query_expdef.prepare(QString("DELETE FROM %1 WHERE \"ExperimentId\" = %2").arg(qrytab_expdef).arg(ExpId) ) )
	qDebug() << query_expdef.lastError().text();
      
      if (query_expdef.exec())
	{
	  qDebug() << "ExperimentDefinition record # :" << ExpId  << "deleted !";
	}
      else
	{
	  QString errmsg   = "Delete record error: " + query_expdef.lastError().text();;
	  QMessageBox::critical( this,
				 tr( "*ERROR* in Deleting Experimental Method" ),
				 tr( "An error occurred in the attempt to delete"
				     " exp. method from AUC DB\n  %1 table\n  %2 ." ).arg( qrytab_expdef ).arg( errmsg ) );
	  return;
	}
				
    }
}


// Slot to define new exp. (from initial dialog)
void US_ComProjectMain::define_new_experiment( QStringList & occupied_instruments )
{

   msg_expsetup = new QMessageBox(this);
   msg_expsetup->setIcon(QMessageBox::Information);

   
   msg_expsetup->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
   msg_expsetup->setStandardButtons(0);
   msg_expsetup->setWindowTitle(tr("Updating..."));
   msg_expsetup->setText(tr( "Setting up EXPERIMENT panel... Please wait...") );
   msg_expsetup->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
  
  
   int tab_width = this->tabWidget->tabBar()->width();
   int upper_height = this->gen_banner->height() + //this->welcome->height()
     + this->logWidget->height() + this->test_footer->height();

   int pos_x = this->width()/2 - tab_width;
   int pos_y = this->height()/2 - upper_height;     
   msg_expsetup->move(pos_x, pos_y);
  
   msg_expsetup->show();
   //msg_expsetup->repaint();
   qApp->processEvents();
  
  
  // //msg_expsetup->exec();

  // diag_expsetup = new QDialog( this );
  // Qt::WindowFlags flags = diag_expsetup->windowFlags();
  // diag_expsetup->setWindowFlags(flags | Qt::Tool);
  // diag_expsetup->show();


  tabWidget->setCurrentIndex( 1 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ?? 
  curr_panx = 1;

  for (int i = 1; i < tabWidget->count(); ++i )
    {
      if ( i == 1 )
	tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
    }

  qDebug() << "In define_new_experiment( QStringList & occupied_instruments )";

  emit pass_used_instruments( occupied_instruments );

}





// Slot to pass submitted to Optima run info to the Live Update tab
void US_ComProjectMain::close_all( void )
{
  xpn_viewer_closed_soft = true;  
  tabWidget->setCurrentIndex( 0 );   
  //epanInit  ->initAutoflowPanel();  //DO NOT Duplicate - was called in previous line while changing Tab!!!
  qDebug() << "XPN viewer /US_convert  closed!";
  qApp->processEvents();
  

  //qDebug() << "CLOSING PROGRAM !!!";
  //close();
  
  // QProcess process;
  // QString pgm("pgrep");
  // QStringList args = QStringList() << "us_comproject";
  // process.start(pgm, args);
  // process.waitForReadyRead();
  // if(!process.readAllStandardOutput().isEmpty()) 
  //   {
  //     //app still running
  //     //Linux
  //     system("pkill us_comproject");
  //     //Windows
  //     system("taskkill /im us_comproject /f");
      
  //     process.kill();
  //   }

  //Linux
  //system("pkill us_comproject");

  
  //system("pkill us_comproject_academic");
  //Windows//
  //system("taskkill /im us_comproject /f");
  //system("taskkill /im us_comproject_academic /f");
  //Mac ??
  
}


// Slot to pass submitted to Optima run info to the Live Update tab
void US_ComProjectMain::switch_to_live_update( QMap < QString, QString > & protocol_details)
{
  tabWidget->setCurrentIndex( 2 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()->setEnabled(false) ?? 
  curr_panx = 2;

 
  for (int i = 1; i < tabWidget->count(); ++i )
    {
      if ( i == 2 )
	tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
    }

  emit pass_to_live_update( protocol_details );
}


// Slot to switch from the Live Update to Editing tab
void US_ComProjectMain::switch_to_post_processing( QMap < QString, QString > & protocol_details )
{
  tabWidget->setCurrentIndex( 3 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  curr_panx = 3;

  // Trigger resize to update size of the PostProcs
  int curr_h = this->height() + 1;
  int curr_w = this->width() + 1;

  this->resize( QSize(curr_w, curr_h) );
  
  for (int i = 1; i < tabWidget->count(); ++i )
    {
      if ( i == 3 )
	tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
    }
  
  // ALEXEY: Make a record to 'autoflow' table: stage# = 2; 
  
  emit pass_to_post_processing( protocol_details );
}
   

// Slot to switch from the Import to Editing tab
void US_ComProjectMain::switch_to_editing( QMap < QString, QString > & protocol_details )
{
   tabWidget->setCurrentIndex( 4 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
   curr_panx = 4;

   // Trigger resize to update size of the Edit_data
  int curr_h = this->height() - 1;
  int curr_w = this->width() - 1;

  this->resize( QSize(curr_w, curr_h) );
   
   for (int i = 1; i < tabWidget->count(); ++i )
     {
       if ( i == 4 )
	 tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
     }
   

   // ALEXEY: Make a record to 'autoflow' table: stage# = 3; 

   emit pass_to_editing( protocol_details );
}

// Slot to switch from the Import to Editing tab
void US_ComProjectMain::switch_to_analysis( QMap < QString, QString > & protocol_details )
{
  // tabWidget->setCurrentIndex( 5 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  // curr_panx = 5;

  // Show msg while triple list is generated
  msg_analysissetup = new QMessageBox(this);
  msg_analysissetup->setIcon(QMessageBox::Information);

  msg_analysissetup->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
  msg_analysissetup->setStandardButtons(0);
  msg_analysissetup->setWindowTitle(tr("Updating..."));
  msg_analysissetup->setText(tr( "Generating triple list for ANALYSIS... Please wait...") );
  msg_analysissetup->setStyleSheet("background-color: #36454f; color : #D3D9DF;");
  
  
  int tab_width = this->tabWidget->tabBar()->width();
  int upper_height = this->gen_banner->height() + //this->welcome->height()
    + this->logWidget->height() + this->test_footer->height();
  
  int pos_x = this->width()/2 - tab_width;
  int pos_y = this->height()/2 - upper_height;     
  msg_analysissetup->move(pos_x, pos_y);
  
  msg_analysissetup->show();
  qApp->processEvents();

  // Pass msg posisitons
  int pos_x_n = pos_x - tab_width;
  int pos_y_n = pos_y - upper_height;
  protocol_details[ "sim_msg_pos_x" ] = QString::number( pos_x );
  protocol_details[ "sim_msg_pos_y" ] = QString::number( pos_y );
  
  tabWidget->setCurrentIndex( 5 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  curr_panx = 5;

   // ALEXEY: Temporariy NOT lock here... Will need later
   
   for (int i = 1; i < tabWidget->count(); ++i )
     {
       if ( i == 5 )
	 tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
     }
   

   // ALEXEY: Make a record to 'autoflow' table: stage# = 4; 

   emit pass_to_analysis( protocol_details );
}


// Slot to switch to Report tab
void US_ComProjectMain::switch_to_report( QMap < QString, QString > & protocol_details )
{
   tabWidget->setCurrentIndex( 6 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
   curr_panx = 6;

   // ALEXEY: Temporariy NOT lock here... Will need later
   
   for (int i = 1; i < tabWidget->count(); ++i )
     {
       if ( i == 6 )
	 tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
     }

   
   qApp->processEvents();
   // ALEXEY: Make a record to 'autoflow' table: stage# = 4; 

   emit pass_to_report( protocol_details );
}

// Slot to switch to e-Signtab
void US_ComProjectMain::switch_to_esign( QMap < QString, QString > & protocol_details )
{
  /**  BEFORE going to e-Signatures: Check if user is among operators|reviewers|approvers**/

  /* 
     Covered in list_all_autoflow_records():
      -- isOperRev( user_id, autolfowId ): if false, run NOT shown in the GMP run list
      Do we want to rather show all runs BUT not allow to switch to E-SIGN if ( !isOperRev( user_id, autolfowId )) ??
  */
  
  /****************************************************************************************/
  
  tabWidget->setCurrentIndex( 7 );   // Maybe lock this panel from now on? i.e. tabWidget->tabBar()-setEnabled(false) ??
  curr_panx = 7;
  
  // ALEXEY: Temporariy NOT lock here... Will need later
  
  for (int i = 1; i < tabWidget->count(); ++i )
    {
      if ( i == 7 )
	tabWidget->tabBar()->setTabEnabled(i, true);
      else
	tabWidget->tabBar()->setTabEnabled(i, false);
    }
  
  
  qApp->processEvents();
  // ALEXEY: Make a record to 'autoflow' table: stage# = 4; 
  
  emit pass_to_esign( protocol_details );
}



// Function to Call initiation of the Autoflow Record Dialogue form _main.cpp
void US_ComProjectMain::call_AutoflowDialogue( void )                           //<--- Entry point from main 
{
  //Check Optima's certificates first:
  // if ( !mainw->us_mode_bool ) 
  epanInit->checkCertificates();
    
  epanInit->initRecordsDialogue();
}


//////////////////////////////////////////////////////////////////////////////////////////
//New Initial Decision-making Tab:
US_InitDialogueGui::US_InitDialogueGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   initDialogueOpen = false;
   initMsgNorecOpen = false;
   initMsgNorecDelOpen = false;

   
   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Show Init Dialogue...  ---UNDER CONSTRUCTION--- ");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   
   // movie_label = new QLabel;
   // QString icon_path = std::getenv("ULTRASCAN");
   // icon_path.append("/etc/"); 
   // QString path_gif = icon_path + "setup.gif";
   
   // QMovie * movie_diag = new QMovie(path_gif);
   // movie_label->setMovie(movie_diag);
   // movie_diag->start();
   
   //assemble main
   //main->addWidget(movie_label); 
   main->addLayout(genL);
   main->addStretch();

   initRecords( );
   
}

void US_InitDialogueGui::resizeEvent(QResizeEvent *event)
{
     int tab_width = mainw->tabWidget->tabBar()->width();
     int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
       + mainw->logWidget->height() + mainw->test_footer->height();
    
     int new_main_w = mainw->width() - 3*offset - tab_width;
     int new_main_h = mainw->height() - 4*offset - upper_height;

     if ( initMsgNorecOpen )
       {
	 int pos_x = this->width()/2 - msg_norec->width()/2;
	 int pos_y = this->height()/2 - msg_norec->height() * 0.75;
	 msg_norec->move(pos_x,pos_y);

	 update();
       }

     if ( initMsgNorecDelOpen )
       {
	 int pos_x = this->width()/2 - msg_norec_del->width()/2;
	 int pos_y = this->height()/2 - msg_norec_del->height() * 0.75;
	 msg_norec_del->move(pos_x,pos_y);

	 update();
       }
     
     if ( initDialogueOpen ){
       //qDebug() << "Is PDIAG visible? " << pdiag_autoflow->isVisible();
          
       if ( new_main_w > pdiag_autoflow->width() || new_main_h > pdiag_autoflow->height()) {
	 int newWidth = qMax( new_main_w, pdiag_autoflow->width());
	 int newHeight = qMax( new_main_h, pdiag_autoflow->height());
	 pdiag_autoflow->setMaximumSize( newWidth, newHeight );
	 pdiag_autoflow->resize( QSize(newWidth, newHeight) );
	 	  
	 update();
       }
       
       if ( new_main_w < pdiag_autoflow->width() ||  new_main_h < pdiag_autoflow->height() ) {
         int newWidth = qMin( new_main_w, pdiag_autoflow->width());
         int newHeight = qMin( new_main_h, pdiag_autoflow->height());
         pdiag_autoflow->setMaximumSize( newWidth, newHeight );
         pdiag_autoflow->resize( QSize(newWidth, newHeight) );

	 update();
       }
       
     }     
     QWidget::resizeEvent(event);
}


// Init Autoflow Panel
void US_InitDialogueGui::initAutoflowPanel( void )
{
  initRecords();
  initRecordsDialogue();
}


//Check for certificates
void US_InitDialogueGui::checkCertificates( void )
{
  US_Passwd  pw;
  US_DB2* dbP  = new US_DB2( pw.getPasswd() );

  
  //read_optima_machines
  read_optima_machines( dbP );

  QMap < QString, QString > Optimas_disconnect_msg;
  Optimas_disconnect_msg.clear();

  int count_instruments_disconnected = 0;
  QStringList Optima_names;

  for ( int i=0; i < this->instruments.size(); ++i )
    {
      QMap< QString, QString > instrument_curr = this->instruments[i];
      
      QString alias          = instrument_curr[ "name" ];
      QString dbhost         = instrument_curr[ "optimaHost" ];
      QString optima_msgPort = instrument_curr[ "msgPort" ];
      
      qDebug() << "In CHECK Connection: alias, dbhost, optima_msgPort -- "
	       << alias << dbhost << optima_msgPort;

      //Collect all Optima names:
      Optima_names << alias;
      
      // Check for certificate license key and its expiraiton
      Link *link = new Link( alias );

      bool status_sys_data = link->connectToServer( dbhost, optima_msgPort.toInt() );
      bool combined_check = status_sys_data & link->connected_itself;
      
      // Ceritificate location && check for nearing or actual expiration date ////////////////////
      QString certPath = US_Settings::etcDir() + QString("/optima/");
      
      QString client_name = alias;
      client_name.simplified();
      client_name.replace(" ", "");
      client_name = client_name.toLower();
      
      // QString keyFile  = certPath + QString( "client.key" );
      // QString pemFile  = certPath + QString( "client.pem" );

      QString keyFile  = certPath + client_name + ".key";
      QString pemFile  = certPath + client_name + ".pem";

      qDebug() << "keyFile, pemFile -- " << keyFile << pemFile;
      
      QFile certfile( pemFile );
      bool certfile_exists = certfile.open(QIODevice::ReadOnly);
      
      QSslCertificate cert(&certfile,QSsl::Pem);
      
      QDateTime expdate = cert.expiryDate();
      
      QString fmt = "yyyy-MM-dd";
      QString expdate_str = expdate.toString(fmt);
      
      qDebug() << "Qt's way of showing expiraiton date: " << cert.expiryDate() << expdate_str; 
      
      // get current QDate
      QDate dNow(QDate::currentDate());
	  
      // Certs expiration QDate 
      QDate dEndCerts =  QDate::fromString(expdate_str, fmt);
      
      // Difference
      int daysToExpiration = dNow.daysTo(dEndCerts);
      
      qDebug() << "Now, End, expiration in days:  " << dNow << dEndCerts << daysToExpiration;
      
      // End of checking expiraiton date ////////////////////////////////////////////////////////////
      qDebug() << "combined_check, daysToExpiration, certfile_exists -- " << combined_check <<  daysToExpiration << certfile_exists;
      
      if ( !combined_check || daysToExpiration <= 0 || !certfile_exists )
	{
	  ++count_instruments_disconnected;
	  QString msg_sys_text_info = QString("");
	  
	  if ( certfile_exists  )
	    {
	      if ( daysToExpiration > 0 )
		{
		  msg_sys_text_info += QString(tr("\n%1 Please check the following for %2:\n")).arg( QChar(0x2022), alias );
		  msg_sys_text_info += QString( tr("1. %1 is turned on\n"
						   "2. the data acquisition server on %1 is running\n"
						   "3. your license key is stored in $HOME/ultrascan/etc/optima and is valid and not expired.\n")).arg( alias );
		  //"Submission of the experimental protocol is suspended until this condition is resolved." )).arg(alias);
		}
	      else
		{
		  msg_sys_text_info += QString( tr("\n%1 Your license key for %2 is expired!\n"
						   "Please renew the key and try to submit again.\n")).arg( QChar(0x2022), alias );
		  // "Submission of the experimental protocol is suspended.\n"
		  //"Please renew the key and try to submit again.")).arg( alias );
		}
	    }
	  else
	    {
	      msg_sys_text_info += QString(tr("\n%1 It appears the license key for %2 is missing:")).arg( QChar(0x2022), alias);
	      msg_sys_text_info += QString( tr("\nPlease check that your license key is stored in\n"
					       "$HOME/ultrascan/etc/optima/ and is not expired.\n"));
	      //"Submission of the experimental protocol is suspended until this condition is resolved." ));
	    }

	  Optimas_disconnect_msg[ alias ] = msg_sys_text_info;
	  
	  // //msg_sys_text_info += QString( tr("\n\nThe program will be closed.") );
	  // msgBox_sys_data.setText( msg_sys_text );
	  // msgBox_sys_data.setInformativeText( msg_sys_text_info );
	  
	  // QPushButton *Cancel_sys    = msgBox_sys_data.addButton(tr("OK"), QMessageBox::RejectRole);
	  
	  // msgBox_sys_data.exec();
	  
	  // exit(1);
	  // return;
	}
      else
	{
	  if ( daysToExpiration <= 30 )
	    {
	      qDebug() << "Certs nearing expiration!! ";
	      
	      QMessageBox::warning( this,
				    QString(tr( "License Key for %1 Nearing Expiraiton" )).arg( alias ),
				    QString(tr( "Your license key will expire within %1 days. \n\n This program will not function without it.") ).arg(daysToExpiration));
	      
	    }
	  
	  //Disconnect link
	  link->disconnectFromServer();
	}
      link->disconnectFromServer();
    }
  
  // Display message & close the program if all machines are disconnected
  if ( count_instruments_disconnected == this->instruments.size() )
    {
      QString inst_names;
      if( Optima_names.size() > 1 )
	inst_names = Optima_names.join(", ");
      else
	inst_names = Optima_names.join("");
      
      QMessageBox msgBox_sys_data;
      msgBox_sys_data.setIcon(QMessageBox::Critical);
      msgBox_sys_data.setWindowTitle(tr("Optima System Data Server Connection Problem!"));

      QString msg_sys_text = QString("Attention! UltraScan is not able to communicate with the data acquisition server(s) on the %1.").arg(inst_names);
      msgBox_sys_data.setText( msg_sys_text );

      QString msg_sys_text_info_final  = QString("");
      for (int i=0; i < Optima_names.size(); ++i )
	{
	  msg_sys_text_info_final += Optimas_disconnect_msg[ Optima_names[i] ];
	}

      if ( Optima_names.size() > 1 )  
	msg_sys_text_info_final += QString( tr("\n\nSubmission of the experimental protocol is suspended until at least one of these conditions is resolved."));
      else
	msg_sys_text_info_final += QString( tr("\n\nSubmission of the experimental protocol is suspended until this condition is resolved."));
      
      msg_sys_text_info_final += QString( tr("\n\nThe program will be closed.") );

      msgBox_sys_data.setInformativeText( msg_sys_text_info_final );
      QPushButton *Cancel_sys    = msgBox_sys_data.addButton(tr("OK"), QMessageBox::RejectRole);
      msgBox_sys_data.exec();

      exit(1);
      return;
    }
  
  //  End of checkig for conneciton to Optima sys_data server ///////////////////////////////////////////////
}


// Init Autoflow records
void US_InitDialogueGui::initRecords( void )               // <-- 1st entry point !!
{
  for (int i=0; i < mainw->tabWidget->count(); ++i )
    {
      //ALEXEY: OR enable all tabs ? (e.g. for demonstration, in a read-only mode or the like ?)
      if ( i == 0 ) 
	mainw->tabWidget->tabBar()->setTabEnabled(i, true);
      else
	mainw->tabWidget->tabBar()->setTabEnabled(i, false);
    }

  
  // Query 'autoflow': get count of records
  autoflow_records = get_autoflow_records();
  
  qDebug() << "Autoflow record #: " << autoflow_records;
  
  // //Temporary: delete ExperimentDefinition record ( ExpId = 306, 301 )
  // int ExpId = 285;
  // delete_psql_record( ExpId );
  
  if ( autoflow_records < 1 )
    return;
  
  
  // Dialog of existing autoflow records
  US_Passwd  pw;
  US_DB2* dbP  = new US_DB2( pw.getPasswd() );
  
  //read_optima_machines
  read_optima_machines( dbP );
  
  //read autoflow records
  list_all_autoflow_records( autoflowdata, dbP );
  
  //count instruments in use
  occupied_instruments.clear();
  for ( int i=0; i < autoflowdata.size(); i++ )
    {
      if ( autoflowdata[ i ][ 5 ] == "LIVE_UPDATE" )
	occupied_instruments << autoflowdata[ i ][ 2 ];
    }

  //ALEXEY: to allow queueing, I clear occupied instruments:
  occupied_instruments.clear();
  
  qDebug() << "Init Autoflow Records: DONE, occupied instruments: " << occupied_instruments;
}

// Init AutoflowRecords Dialogue: call from _main.cpp
void US_InitDialogueGui::initRecordsDialogue( void )
{
 
  // ALEXEY: ensure that NO pdiag_dialog(s) open before initialization... IMPORTANT!
  // Will Not be needed in the production version where all but currently active Tab is enabled...
  // But may be a good idea to retain... 
  /* ---------------------------------------------------------------------------------------*/
  if ( initDialogueOpen )
    {
      pdiag_autoflow->close();
      initDialogueOpen = false;

      qDebug() << "Spurious/Old Autoflow dialogues closed!!!";
    }
  if ( initMsgNorecOpen )
    {
      msg_norec->reject();      // Not just close() - as it triggers setting up exp panel...
      initMsgNorecOpen = false;

      qDebug() << "Spurious/Old NoRec msg closed!!!";
    }
  if ( initMsgNorecDelOpen )
    {
      msg_norec_del->reject();  // Not just close() - as it triggers setting up exp panel...
      initMsgNorecDelOpen = false;
      
      qDebug() << "Spurious/Old NoRec After Deletion msg closed!!!";
    }
  /* ---------------------------------------------------------------------------------------*/
    
  //if ( autoflow_records < 1 )
  if ( autoflow_records < 1 || autoflowdata.size() < 1 ) //can be that for UL<3, autoflow runs from others are not shown
    {
      //ALEXEY: should close pdiag_autoflow if wasn't closed already
      initMsgNorecOpen = true;
      
      occupied_instruments.clear();
      
      msg_norec = new QMessageBox;
      msg_norec->setIcon(QMessageBox::Information);
      msg_norec->setText(tr( "There are no Optima runs to follow.<br><br>"
			     "You will be switched to <b>Experiment</b> stage to design and submit new protocol."
			     ));
      
      msg_norec->setParent(this, Qt::Widget);
      msg_norec->setStyleSheet("background-color: #36454f; color : #D3D9DF;");

      //position
      int pos_x = this->width()/2 - msg_norec->width()/2;
      int pos_y = this->height()/2 - msg_norec->height()/4;
      msg_norec->move(pos_x, pos_y);
 
      if( msg_norec->exec() == QMessageBox::Ok )
       	{
       	  msg_norec->close();
	  emit define_new_experiment_init( occupied_instruments );
       	  //qApp->processEvents();
       	}

      //emit define_new_experiment_init( occupied_instruments );
      
      return;
    }
  
  QString pdtitle( tr( "Select Optima Run to Follow" ) );
  QStringList hdrs;
  int         prx;
  hdrs << "ID"
       << "Run Name"
       << "Optima Name"
       << "Created"
       << "Optima Run Status"
       << "Stage"
       << "GMP"
       << "Failed";
  
  QString autoflow_btn;

  if ( mainw->us_mode_bool )
    autoflow_btn = "AUTOFLOW_DA";
  else
    autoflow_btn = "AUTOFLOW_GMP";

  pdiag_autoflow = new US_SelectItem( autoflowdata, hdrs, pdtitle, &prx, autoflow_btn, -3 );

  connect( pdiag_autoflow, SIGNAL( accept_autoflow_deletion() ), this, SLOT( update_autoflow_data() ));
  pdiag_autoflow->setParent(this, Qt::Widget);

  offset = 20;
  pdiag_autoflow->move(offset, 2*offset);

  //mainw->cornerWidget->setFixedWidth( mainw->tabWidget->tabBar()->width() - 20 );

  initDialogueOpen = true;

  // Trigger resize to update size of the InitDialogue
  int curr_h = this->height() + 1;
  int curr_w = this->width() + 1;

  this->resize( QSize(curr_w, curr_h) );
  //qDebug() << "Size of the InitDialogue widget: " << this->size();

  //pdiag_autoflow->setFrameShape( QFrame::Box);
  //pdiag_autoflow->setLineWidth(2); 

  // pdiag_autoflow->setObjectName("hello");
  // pdiag_autoflow->setWindowFlags(Qt::FramelessWindowHint);
  // pdiag_autoflow->setStyleSheet("#hello{border:4px solid darkgrey}");


  //ALEXEY: "Define Another Exp." button should be always available
  // //disable 'Define Another Exp.' button if all instruments are in use
  // if ( occupied_instruments.size() >= instruments.size() )
  //   pdiag_autoflow->pb_cancel->setEnabled( false );

  qDebug() << "occupied_instruments.size(), instruments.size()" << occupied_instruments.size() << ", " <<  instruments.size();
  

  //ALEXEY: clear occupied instr. as we decided on queueing...
  occupied_instruments.clear();
  
  QString autoflow_id_selected("");
  
  if ( pdiag_autoflow->exec() == QDialog::Accepted )
    autoflow_id_selected  = autoflowdata[ prx ][ 0 ];
  else
    {
      //ALEXEY: define what to do if some Optima(s) are occupied
      // should emit signal sending list of optima's in use to us_experiment.

      if ( mainw-> window_closed)
	return;
      
      if ( occupied_instruments.size() == 0 )
	{
	  qDebug() << "No occuied instruments (No LIVE_UPDATE status)";
	  emit define_new_experiment_init( occupied_instruments );
	  return;
	}
      else
	{
	  QString list_instruments_in_use = occupied_instruments.join(", ");
	  
	  QMessageBox * msg_instr_use = new QMessageBox(this);
	  msg_instr_use->setIcon(QMessageBox::Information);
	  msg_instr_use->setText(tr( "The following Optima instrument(s)<br>"
				       "are currently in use: <br><br>"
				       "<b>%1</b> <br><br>"
				       "You will not be able to submit another run<br>"
				       "to these instruments at the moment." )
				   .arg( list_instruments_in_use ) );

	  msg_instr_use->exec();
	  
	  emit define_new_experiment_init( occupied_instruments );
	  return;
	}
    }

  // -------------------------------------------------------------------------------------------------
  // Get detailed info on the autoflow record
  QMap < QString, QString > protocol_details;
  
  int autoflowID = autoflow_id_selected.toInt();
  protocol_details = read_autoflow_record( autoflowID );

  //Check if run selected still exists
  if ( protocol_details.isEmpty() )
    {
      qDebug() << "Run does NOT exists!!! Updating list...";

      //emit to_initAutoflow();
      initAutoflowPanel();
      return;
    }
  
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
  QString statusID     = protocol_details[ "statusID" ];
  QString failedID     = protocol_details[ "failedID" ];

  QString expType      = protocol_details[ "expType" ];
  QString instName     = protocol_details[ "OptimaName" ];
  QString dataSource   = protocol_details[ "dataSource" ];
  QString opticsFailedType = protocol_details[ "opticsFailedType"];
    
  QDir directory( currDir );
  
  qDebug() << "CURR DIRECTORY : "   << currDir;
  qDebug() << "1.ExpAborted: "      << protocol_details[ "expAborted" ];
  qDebug() << "1.CorrectRadii: "    << protocol_details[ "correctRadii" ];

  qDebug() << "Exp. Label: "    << protocol_details[ "label" ];
  qDebug() << "GMP Run ? "      << protocol_details[ "gmpRun" ];

  qDebug() << "AnalysisIDs: "   << protocol_details[ "analysisIDs" ];
  qDebug() << "statusID: "      << protocol_details[ "statusID" ];
  qDebug() << "failedID: str, INT --  "
	   << protocol_details[ "failedID" ]
	   << protocol_details[ "failedID" ].toInt();

  qDebug() << "ExpType: " << expType;
  qDebug() << "opticsFailedType: " << opticsFailedType;


  //Re-attachment to FAILED GMP run
  if ( failedID. toInt() != 0 )
    {
      // jump to 2. LIVE_UPDATE for now, i.e. re-initialize from scratch
      qDebug() << "Re-initializing FAILED GMP rinID -- " << autoflowID;
      
      //read autoflowFailed record
      QMap< QString, QString > failed_details = read_autoflow_failed_record( failedID );
      
      QMessageBox msgBox_f;
      msgBox_f.setText(tr( "The selected GMP run is marked as FAILED:<br><br>" )
		       + tr("<b>Run Name:&emsp;</b>") + ProtName
		       + tr("<br>")
		       + tr("<b>Failing stage:&emsp;</b> ") + failed_details[ "failedStage" ]
		       + tr("<br>")
		       + tr("<b>Reason:&emsp;</b> ") + failed_details[ "failedMsg" ] );
		      		       
      msgBox_f.setInformativeText( tr("<font color='red'><b>ATTENTION:</b></font> If you choose to Procceed, ")
				   + tr("all existing data for this run will be deleted from DB, ")
				   + tr("and the processing flow will reinitialize. ")
				   + tr("<br><br><font color='red'><b>This action is not reversible. Proceed?</b></font>"));
      
      msgBox_f.setWindowTitle(tr("Reinitialization of Failed Run"));
      QPushButton *Confirm   = msgBox_f.addButton(tr("Proceed"), QMessageBox::YesRole);
      QPushButton *Cancel    = msgBox_f.addButton(tr("Cancel"),  QMessageBox::RejectRole);
      
      msgBox_f.setIcon(QMessageBox::Question);
      msgBox_f.exec();
      
      if (msgBox_f.clickedButton() == Cancel)
	{
	  initAutoflowPanel();
	  return;
	}
      else if (msgBox_f.clickedButton() == Confirm)
	{
	  qDebug() << "Choosing REINITIALIZATION!!!";

	  //Delete and reset everything related to the run:
	  /*
	    1. revert 'liveUpdate, import, editing' fields in autolfowStages to DEFAULT 
	    2. delete autoflowIntensity && autoflowStatus records
	    3. ?? do we also delete immediately autoflowFailed record ??
	    4. DELETE all exp. | rawData (maybe models, editedData etc.)
	    5. delete autoflowHistory record (if was already created - ONLY case if REPORT was proceeded ?)
	    6. Reset all autoflow record fields updated starting from LIVE_UPDATE
	                 - dataPath
			 - filename
			 - intensityID
			 - statusID
			 - failedID (IF record in autoflowFailed deleted )
			 - analysisIDs

	  */
	  
	  
	  do_run_tables_cleanup( protocol_details );

	  do_run_data_cleanup( protocol_details );

	  //Create fresh autoflowStatus record with the submitter person info & timestamp: 
	  do_create_autoflowStatus_for_failedRun( protocol_details );
	  
	  //Switch to 2. LIVE_UPDATE:
	  emit switch_to_live_update_init( protocol_details );
	  
	  return;
	}
    }
  // Normal re-attachement
  else
    {
      //pdiag_autoflow -> close();
      
      if ( stage == "LIVE_UPDATE" )
	{
	  //do something
	  //switch_to_live_update( protocol_details );
	  
	  emit switch_to_live_update_init( protocol_details );
	  
	  return;
	}
      
      if ( stage == "EDITING" )
	{
	  //do something
	  if ( currDir.isEmpty() || !directory.exists()  )
	    {
	      if ( instName. contains("Optima") && dataSource == "INSTRUMENT" )
		emit switch_to_live_update_init( protocol_details );
	      else // disk Data
		{
		  QMessageBox::warning( this, tr( "Data Absent on Disk!" ),
					tr( "No data associated with this run is found on the disk! \n" ) );
		  initAutoflowPanel();
		  //emit to_initAutoflow();
		  return;
		}
	    }
	  else
	    {
	      qDebug() << "DataDisk, proceeding...";
	      emit switch_to_post_processing_init( protocol_details );
	    }
	  
	  
	  return;
	}
      if ( stage == "EDIT_DATA" )
	{
	  emit switch_to_editing_init( protocol_details );
	}
      
      if ( stage == "ANALYSIS" )
	{
	  qDebug() << "To ANALYSIS SWITCH ";
	  emit switch_to_analysis_init( protocol_details );
	  
	}
      
      if ( stage == "REPORT" )
	{
	  qDebug() << "To REPORT SWITCH ";
	  emit switch_to_report_init( protocol_details );
	  
	}

      if ( stage == "E-SIGNATURES" )
	{
	  qDebug() << "To E-SIGNS SWITCH ";
	  emit switch_to_esign_init( protocol_details );
	  
	}

      //and so on...
    }
   
}

//Cleanup failed run for further re-initializaiton:
void US_InitDialogueGui::do_run_tables_cleanup( QMap < QString, QString > run_details )
{
  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem: Failed Run Cleanup" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return;
    }

  QStringList qry;

  //reverting autoflowSatges
  qry << "autoflow_stages_revert"
      << run_details[ "autoflowID" ];
  db->query( qry );
  
  //deleting autoflowIntensity Record
  qry. clear();
  qry << "delete_autoflow_intensity_record"
      << run_details[ "autoflowID" ]
      << run_details[ "intensityID" ];
  db->query( qry );

  //deleting autoflowStatus Record
  qry. clear();
  qry << "delete_autoflow_status_record"
      << run_details[ "autoflowID" ]
      << run_details[ "statusID" ];
  db->query( qry );

  //deleting autoflowFailed Record
  qry. clear();
  qry << "delete_autoflow_failed_record"
      << run_details[ "autoflowID" ]
      << run_details[ "failedID" ];
  db->query( qry );

  //set autoflowAnalysis records for deletion (status = 'CANCELED')
  QStringList analysisIDs_list = run_details[ "analysisIDs" ].split(",");
  for( int i=0; i < analysisIDs_list.size(); ++i )
    {
      QString requestID = analysisIDs_list[i];
      
      qry.clear();
      qry << "update_autoflow_analysis_record_at_deletion"
	  << QString("Canceled for Failed run, top-level")
	  << requestID;
      
      db->query( qry );
    }

  //delete autoflowAnalysisHistory records for given autoflowID : 
  qry.clear();
  qry << "delete_autoflow_analysis_history_records_by_autoflowID"
      << run_details[ "autoflowID" ];
  db->query( qry );

  //delete autoflowModelsLink records for given autoflowID :
  qry.clear();
  qry << "delete_autoflow_model_links_records_by_autoflowID"
      << run_details[ "autoflowID" ];
  db->query( qry );
  
  
  //deleting autoflowHistory Record (if exists)
  qry. clear();
  qry << "delete_autoflow_history_record"
      << run_details[ "autoflowID" ];
  db->query( qry );

  //Clean certain field in autoflow record for re-initializing from 2. LIVE_UPDATE
  qry. clear();
  qry << "update_autoflow_at_reset_live_update"
      << run_details[ "autoflowID" ];
  db->query( qry );

  //Restore autoflowReports' records 'tripleDropped' to DEFAULT ('none')
  // 1. Create reportIDs list from protocol's AProfile;
  // 2. Go over reports in the autoflowReport table & SET 'tripleDropped' to 'none'  

  channels_report. clear();
  QString aprofile_xml;
  
  qry. clear();
  qry << "get_aprofile_info" << run_details[ "aprofileguid" ];
  db->query( qry );
  
  while ( db->next() )
    {
      aprofile_xml         = db->value( 2 ).toString();
    }
  
  if ( !aprofile_xml.isEmpty() )
    {
      QXmlStreamReader xmli( aprofile_xml );
      readAProfileBasicParms_auto( xmli );
    }

  QMap<QString, QString>::iterator chan_rep;
  for ( chan_rep = channels_report.begin(); chan_rep != channels_report.end(); ++chan_rep )
    {
      QString chan_key  = chan_rep.key();
      QString reportIDs = chan_rep.value();
      qDebug() << "Channel name -- " << chan_key << ", reportIDs -- " << reportIDs;
      
      QStringList reportIDs_list = reportIDs.split(",");
      for (int i=0; i<reportIDs_list.size(); ++i)
	{
	  qry. clear();
	  QString rID = reportIDs_list[i];
	  
	  qry << "update_autoflow_report_at_import"
	      << rID
	      << QString("none");
	  
	  qDebug() << "Reverting 'tripleDropped' autoflowReport record: query, rID -- " << qry << rID;
	  db->query( qry );
	}
    }
  
}

//Create fresh autofowStatus record: failed run re-init
void US_InitDialogueGui::do_create_autoflowStatus_for_failedRun( QMap < QString, QString > run_details )
{
  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem: Failed Run Cleanup" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return;
    }

  QStringList qry;

  //first, get current user (submitter) info
  qry.clear();
  qry <<  QString( "get_user_info" );
  db -> query( qry );
  db -> next();
  int     u_ID    = db->value( 0 ).toInt();
  QString u_fname = db->value( 1 ).toString();
  QString u_lname = db->value( 2 ).toString();
  QString u_email = db->value( 4 ).toString();
  int     u_level = db->value( 5 ).toInt();

  //autoflowStatus record
  QString createGMPRun_Json;
  createGMPRun_Json. clear();
  createGMPRun_Json += "{ \"Person\": ";
  
  createGMPRun_Json += "[{";
  createGMPRun_Json += "\"ID\":\""     + QString::number( u_ID )     + "\",";
  createGMPRun_Json += "\"fname\":\""  + u_fname                     + "\",";
  createGMPRun_Json += "\"lname\":\""  + u_lname                     + "\",";
  createGMPRun_Json += "\"email\":\""  + u_email                     + "\",";
  createGMPRun_Json += "\"level\":\""  + QString::number( u_level )  + "\"";
  createGMPRun_Json += "}],";
  
  //createGMPRun_Json += "\"Comment\": \""   + gmp_submitter_map[ "Comment:" ]   + "\"";
  QString resubComm = tr("Resubmitting Failed GMP Run");
  createGMPRun_Json += "\"Comment\": \""   + resubComm   + "\"";
  
  createGMPRun_Json += "}";
  
  qry. clear();
  qry << "new_autoflowStatusGMPCreate_record"
      << run_details[ "autoflowID" ]
      << createGMPRun_Json;
  
  qDebug() << "[FAILED run init]: new_autoflowStatusGMPCreate_record qry -- " << qry;
  
  int autoflowStatusID = 0;
  autoflowStatusID = db->functionQuery( qry );
  
  if ( !autoflowStatusID )
    {
      QMessageBox::warning( this, tr( "AutoflowStatus Record Problem" ),
			    tr( "autoflowStatus (FAILED GMP run re-INIT): "
				"There was a problem with creating a record in autoflowStatus table \n" ) + db->lastError() );
      
      return;
    }
  qDebug() << "in do_create_autoflowStatus_for_failedRun: createGMPRun_Json -- " << createGMPRun_Json;
  
  run_details[ "statusID" ] = QString::number( autoflowStatusID );
  
  /************** finally, update autoflow record with StatusID: ****************/
  qry. clear();
  qry <<  "update_autoflow_with_statusID"
      <<  run_details[ "autoflowID" ]
      <<  QString::number( autoflowStatusID );
  
  qDebug() << "[FAILED run init]: update_autoflow_with_statusID qry -- " << qry;
  db->query( qry );

}



//Read channel-to-ref_wvl info from AProfile
bool US_InitDialogueGui::readAProfileBasicParms_auto( QXmlStreamReader& xmli )
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
		//Channel Name
		QString channel_name = attr.value( "channel" ).toString();
		
		//Read what reportID corresponds to channel:
		if ( attr.hasAttribute("report_id") )
		  channels_report[ channel_name ] = attr.value( "report_id" ).toString();
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


//Cleanup failed run's data for further re-initializaiton:
void US_InitDialogueGui::do_run_data_cleanup( QMap < QString, QString > run_details )
{

  //Get proper filename
  QString FileName = run_details[ "filename" ];
  QStringList fileNameList;
  fileNameList. clear();
  if ( FileName.contains(",") && FileName.contains("IP") && FileName.contains("RI") )
    fileNameList  = FileName.split(",");
  else
    fileNameList << FileName;

  //show progress dialog
  int progress_total = fileNameList.size()*6 + 1;
  QProgressDialog* progress_msg = new QProgressDialog ("Cleaning Data for Current Run...", QString(), 0, progress_total, this);
  progress_msg->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  progress_msg->setModal( true );
  progress_msg->setWindowTitle(tr("Cleaning Data..."));
  progress_msg->setAutoClose( false );
  progress_msg->setValue( 0 );
  progress_msg->show();
  qApp->processEvents();

  progress_msg->setValue( 1 );
  qApp->processEvents();
  
  
  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem: Failed Run Cleanup" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return;
    }

  QStringList qry;

  int progress = progress_msg->value();
  
  /*** Iterate over fileNameList *********************************************/
  for ( int i=0; i<fileNameList.size(); ++i )
    {
      qry.clear();
      //get experimentID from 'experiment' table:
      qry << "get_experiment_info_by_runID"
	  << fileNameList[ i ]
	  << run_details[ "invID_passed" ];

      db->query( qry );
      db->next();
      QString expID  = db->value( 1 ).toString();
      
      progress_msg->setValue( ++progress);
      qApp->processEvents();
      
      // Let's make sure it's not a calibration experiment in use
      qry. clear();
      qry << "count_calibration_experiments" << expID;
      int count = db->functionQuery( qry );
      qDebug() << "Cleaning Failed Run: calexp count" << count;

      progress_msg->setValue( ++progress);
      qApp->processEvents();
      
      if ( count < 0 )
	{
	  qDebug() << "count_calibration_experiments( "
		   << expID
		   << " ) returned a negative count";

	  progress_msg->close();
	  return;
	}
      
      else if ( count > 0 )
	{
	  QMessageBox::information( this,
				    tr( "Error" ),
				    tr( "Cannot delete an experiment that is associated "
					"with a rotor calibration\n" ) );
	  progress_msg->close();
	  return;
	}

      int status;
      
      // Delete links between experiment and solutions
      qry. clear();
      qry << "delete_experiment_solutions"
	  << expID;
      status = db -> statusQuery( qry );
      qDebug() << "Cleaning Failed Run: del sols status" << status;

      progress_msg->setValue( ++progress);
      qApp->processEvents();
      
      // Same with cell table
      qry. clear();
      qry  << "delete_cell_experiments"
	   << expID;
      status = db -> statusQuery( qry );
      qDebug() << "Cleaning Failed Run: del cells status" << status;

      progress_msg->setValue( ++progress);
      qApp->processEvents();
      
      // Let's delete any pcsa_modelrecs records to avoid
      //  constraints problems
      //US_Experiment::deleteRunPcsaMrecs( db, run_details[ "invID_passed" ], run_details[ "filename" ] );
      qry. clear();
      qry << "delete_run_pcsa_recs"
	  << fileNameList[ i ];
      status = db -> statusQuery( qry );
      qDebug() << "Cleaning Data for Run PRotDev(): del_exp stat" << status;

      progress_msg->setValue( ++progress);
      qApp->processEvents();

      // Now delete editedData, models, noises, reports, 
      qry. clear();
      qry << "clear_data_for_experiment"
       	  << expID;
      status = db -> statusQuery( qry );
      qDebug() << "Cleaning Data (del data) for FAILED run: del_exp stat" << status;

      progress_msg->setValue( ++progress);
      qApp->processEvents();
      
      if ( status != US_DB2::OK )
       	{
       	  QMessageBox::information( this,
       				    tr( "Error / Warning" ),
       				    db -> lastError() + tr( " (error=%1, expID=%2)" )
       				    .arg( status ).arg( expID ) );
       	}
      
      // // Now delete the experiment and all existing rawData, 
      // qry. clear();
      // qry << "delete_experiment"
      // 	  << expID;
      // status = db -> statusQuery( qry );
      // qDebug() << "Cleaning Failed Run: del_exp stat" << status;
      
      // if ( status != US_DB2::OK )
      // 	{
      // 	  QMessageBox::information( this,
      // 				    tr( "Error / Warning" ),
      // 				    db -> lastError() + tr( " (error=%1, expID=%2)" )
      // 				    .arg( status ).arg( expID ) );
      // 	}
    }

  progress_msg->setValue( progress_msg->maximum() );
  qApp->processEvents();
  progress_msg->close();
  /** End Iterate over fileNameList ****************************************************************/
}



//Re-evaluate autoflow records & occupied instruments & if Define Another Exp. should be enabled....
void US_InitDialogueGui::update_autoflow_data( void )
{
  qDebug() << "Updating autoflow records!!!";

  initMsgNorecDelOpen = false;
  
  US_Passwd  pw;
  US_DB2* dbP  = new US_DB2( pw.getPasswd() );

  //Re-read autoflow records
  list_all_autoflow_records( autoflowdata, dbP );

  if ( autoflowdata.size() < 1 )
    {
      initMsgNorecDelOpen = true;

      //pdiag_autoflow->reject();
      pdiag_autoflow->close();
      qApp->processEvents();
      
      qDebug() << "Was pdiag closed ?? ";
      
      occupied_instruments.clear();
      
      msg_norec_del = new QMessageBox;
      msg_norec_del->setIcon(QMessageBox::Information);
      msg_norec_del->setText(tr( "There are no Optima runs to follow.<br><br>"
			     "You will be switched to <b>Experiment</b> stage to design and submit new protocol."
			     ));

      msg_norec_del->setParent(this, Qt::Widget);
      msg_norec_del->setStyleSheet("background-color: #36454f; color : #D3D9DF;");

      //position
      int pos_x = this->width()/2 - msg_norec_del->width()/2;
      int pos_y = this->height()/2 - msg_norec_del->height()/4;
      msg_norec_del->move(pos_x, pos_y);

      //connect(msg_norec_del, SIGNAL( accept() ), SLOT(close()) ); 
      if( msg_norec_del->exec() == QMessageBox::Ok )
	{
	  msg_norec_del->close();
	  emit define_new_experiment_init( occupied_instruments );
	  
	  //qApp->processEvents();
	}
      
      //msg_norec_del->exec();
      
      //emit define_new_experiment_init( occupied_instruments );
      
      return;
    }
  

  
  //Re-count instruments in use
  occupied_instruments.clear();
  for ( int i=0; i < autoflowdata.size(); i++ )
    {
      if ( autoflowdata[ i ][ 5 ] == "LIVE_UPDATE" )
	occupied_instruments << autoflowdata[ i ][ 2 ];
    }

  //ALEXEY: "Define Another Exp." button should be always available
  // //Re-set Define Another Exp. button
  // if ( occupied_instruments.size() >= instruments.size() )
  //   pdiag_autoflow->pb_cancel->setEnabled( false );
  // else
  //   pdiag_autoflow->pb_cancel->setEnabled( true );

  //ALEXEY: also always clear occupied_instruments as we decide on queueing...
  occupied_instruments. clear();

  qDebug() << "Define Another Exp. button reset";

}

// Slot to read all Optima machines <------------------------------- // New
void US_InitDialogueGui::read_optima_machines( US_DB2* db )
{
  this->instruments.clear();
  
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );
  db->query( q );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no instruments defined
    {
      QList< int > instrumentIDs;
      
      while ( db->next() )
	{
	  int ID = db->value( 0 ).toString().toInt();
	  instrumentIDs << ID;
	  
	  qDebug() << "InstID: " << ID;
	}
      
      // Instrument information
      foreach ( int ID, instrumentIDs )
	{
	  QMap<QString,QString> instrument;
	  
	  q.clear();
	  q  << QString( "get_instrument_info_new" )
	     << QString::number( ID );
	  db->query( q );
	  db->next();

	  instrument[ "ID" ]              =   QString::number( ID );
	  instrument[ "name" ]            =   db->value( 0 ).toString();
	  instrument[ "serial" ]          =   db->value( 1 ).toString();
	  instrument[ "optimaHost" ]      =   db->value( 5 ).toString();	   
	  instrument[ "optimaPort" ]      =   db->value( 6 ).toString(); 
	  instrument[ "optimaDBname" ]    =   db->value( 7 ).toString();	   
	  instrument[ "optimaDBusername" ] =  db->value( 8 ).toString();	   
	  instrument[ "optimaDBpassw" ]    =  db->value( 9 ).toString();	   
	  instrument[ "selected" ]        =   db->value( 10 ).toString();
	    
	  instrument[ "opsys1" ]  = db->value( 11 ).toString();
	  instrument[ "opsys2" ]  = db->value( 12 ).toString();
	  instrument[ "opsys3" ]  = db->value( 13 ).toString();

	  instrument[ "radcalwvl" ]  =  db->value( 14 ).toString();
	  instrument[ "chromoab" ]   =  db->value( 15 ).toString();

	  instrument[ "msgPort" ]   =  db->value( 16 ).toString();

	  
	  if ( instrument[ "name" ].contains("Optima") || instrument[ "optimaHost" ].contains("AUC_DATA_DB") )
	    this->instruments << instrument;
	}
    }
  qDebug() << "Reading Instrument: FINISH";
}



// Query autoflow for # records
int US_InitDialogueGui::list_all_autoflow_records( QList< QStringList >& autoflowdata, US_DB2* dbP )
{
  int nrecs        = 0;   
  autoflowdata.clear();

  QStringList qry;
  //Check user level && ID
  QStringList defaultDB = US_Settings::defaultDB();
  QString user_guid   = defaultDB.at( 9 );
  
  //get personID from personGUID
  qry.clear();
  qry << QString( "get_personID_from_GUID" ) << user_guid;
  dbP->query( qry );
  
  int user_id = 0;
  
  if ( dbP->next() )
    user_id = dbP->value( 0 ).toInt();
    
  //now look at autpflow runs
  qry.clear();
  qry << "get_autoflow_desc";
  dbP->query( qry );

  if ( dbP->lastErrno() != US_DB2::OK )
    return nrecs;
  
  while ( dbP->next() )
    {
      QStringList autoflowentry;
      QString id                 = dbP->value( 0 ).toString();
      QString runname            = dbP->value( 5 ).toString();
      QString status             = dbP->value( 8 ).toString();
      QString optimaname         = dbP->value( 10 ).toString();
      
      QDateTime time_started     = dbP->value( 11 ).toDateTime().toUTC();

      QString invID              = dbP->value( 12 ).toString();
      
      QDateTime time_created     = dbP->value( 13 ).toDateTime().toUTC();
      QString ptime_created       = US_Util::toUTCDatetimeText( time_created
								.toString( Qt::ISODate ), true )
	                                                         .section( ":", 0, 1 ) + " UTC";
      
      QString gmpRun             = dbP->value( 14 ).toString();
      QString operatorID         = dbP->value( 16 ).toString();
      QString failedID           = dbP->value( 17 ).toString();

      qDebug() << "OperatorID -- " << operatorID;
      qDebug() << "failedID -- "   << failedID;
      
      QDateTime local(QDateTime::currentDateTime());

      //autoflowentry << id << runname << optimaname  << time_created.toString(); // << time_started.toString(); // << local.toString( Qt::ISODate );
      autoflowentry << id << runname << optimaname  << ptime_created;

      qDebug() << "1. IN list_all_autoflow_records(), autoflowentry -- " << autoflowentry;
	
      if ( time_started.toString().isEmpty() )
	{
	  if ( optimaname. contains("Optima") ) 
	    autoflowentry << QString( tr( "NOT STARTED" ) );
	  else
	    autoflowentry << QString( tr( "N/A" ) );
	}
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

      if ( failedID.toInt() )
	autoflowentry << "YES";
      else
	autoflowentry << "NO";
    

      //Treat eSign separately: only show runs at this stage to reviewers && operator:
      if ( status == "E-SIGNATURES" )
	{
	  // //taking care of UL=2, who owns a run, and thus, is an operator
	  // if (  user_id && user_id == operatorID.toInt() )
	  //   {
	  //     autoflowdata  << autoflowentry;
	  //     nrecs++;
	  //   }

	  //reviewers: check against new 'autoflowGMPReportEsign. statusSignJson["to_process"]' table's field: 
	  if ( isOperRev( user_id, id ) )
	    {
	      autoflowdata  << autoflowentry;
	      nrecs++;
	    }
	  
	}
      else
	{
	  //Check user level && GUID; if <3, check if the user is investigator
	  if ( US_Settings::us_inv_level() < 3 )
	    {
	      qDebug() << "User level low: " << US_Settings::us_inv_level();
	      qDebug() << "user_id, operatorID.toInt(), invID.toInt() -- " << user_id << operatorID.toInt() << invID.toInt();
	      
	      //if ( user_id && ( user_id == operatorID.toInt() || user_id == invID.toInt() ) )
	      if ( user_id && user_id == invID.toInt() )
		{
		  autoflowdata  << autoflowentry;
		  nrecs++;
		}
	    }
	  else
	    {
	      autoflowdata  << autoflowentry;
	      nrecs++;

	      qDebug() << "2. IN list_all_autoflow_records(), autoflowentry -- " << autoflowentry;
	    }
	}
    }

  return nrecs;
}


//Check for e-Signature stage, if the logged in user an operator || reviewer
bool US_InitDialogueGui::isOperRev( int uID, QString autoflow_id )
{
  bool yesRev = false;

  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );

  if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return yesRev;
     }

  QStringList qry;
  qry << "get_gmp_review_info_by_autoflowID" << autoflow_id;

  db->query( qry );
  
  if ( db->lastErrno() != US_DB2::OK )
    return yesRev;
  
  QString esign_id;
  QString gmp_run_id;
  QString autoflow_name ;
  QString operator_list;
  QString reviewers_list;
  QString approvers_list;
  QString eSignStatusJson;
  QString eSignStatusAll;
  QString createUpdateJsonLog;
  
  while ( db->next() )
    {
      esign_id             = db->value( 0 ).toString();    //INT
      gmp_run_id           = db->value( 1 ).toString();    //INT
      autoflow_name        = db->value( 2 ).toString();    //TEXT
      operator_list        = db->value( 3 ).toString();    //json array [1 value for now]
      reviewers_list       = db->value( 4 ).toString();    //json array
      eSignStatusJson      = db->value( 5 ).toString();    //json
      eSignStatusAll       = db->value( 6 ).toString();    //ENUM
      createUpdateJsonLog  = db->value( 7 ).toString();    //json
      approvers_list       = db->value( 8 ).toString();    //json array
    }

  //process 'reviewers_list' & 'operList' Json arrays:
  QJsonDocument jsonDocRevList  = QJsonDocument::fromJson( reviewers_list.toUtf8() );
  QJsonDocument jsonDocOperList = QJsonDocument::fromJson( operator_list .toUtf8() );
  QJsonDocument jsonDocApprList = QJsonDocument::fromJson( approvers_list .toUtf8() );
  
  if ( !jsonDocRevList. isArray() || !jsonDocOperList. isArray() || !jsonDocApprList. isArray() )
    {
      qDebug() << "jsonDocRevList OR jsonDocOperList OR jsonDocApprList not a JSON Array!";
      return yesRev;
    }
  
  QJsonArray jsonDocRevList_array  = jsonDocRevList.array();
  for (int i=0; i < jsonDocRevList_array.size(); ++i )
    {
      QString current_reviewer = jsonDocRevList_array[i].toString();
      //uname =  oID + ": " + olname + ", " + ofname;
      int current_reviewer_id = current_reviewer. section( ".", 0, 0 ).toInt();

      if ( uID == current_reviewer_id )
	{
	  return true;
	}
    }

  QJsonArray jsonDocOperList_array  = jsonDocOperList.array();
  for (int i=0; i < jsonDocOperList_array.size(); ++i )
    {
      QString current_reviewer = jsonDocOperList_array[i].toString();
      //uname =  oID + ": " + olname + ", " + ofname;
      int current_reviewer_id = current_reviewer. section( ".", 0, 0 ).toInt();

      if ( uID == current_reviewer_id )
	{
	  return true;
	}
    }

  QJsonArray jsonDocApprList_array  = jsonDocApprList.array();
  for (int i=0; i < jsonDocApprList_array.size(); ++i )
    {
      QString current_reviewer = jsonDocApprList_array[i].toString();
      //uname =  oID + ": " + olname + ", " + ofname;
      int current_reviewer_id = current_reviewer. section( ".", 0, 0 ).toInt();

      if ( uID == current_reviewer_id )
	{
	  return true;
	}
    }
 
  
  return yesRev;
}


// Query autoflow for # records
int US_InitDialogueGui::get_autoflow_records( void )
{

 
   //--------------------------------------////
  
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   int record_number = 0;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return record_number;
     }

   QStringList qry;
   qry << "count_autoflow_records";
   
   record_number = db->functionQuery( qry );

   return record_number;
}


// Query autoflow for # records
QMap< QString, QString> US_InitDialogueGui::read_autoflow_record( int autoflowID  )
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
	   protocol_details[ "intensityID" ]   = db->value( 20 ).toString();
	   protocol_details[ "statusID" ]      = db->value( 21 ).toString();
	   protocol_details[ "failedID" ]      = db->value( 22 ).toString();
	   protocol_details[ "operatorID" ]    = db->value( 23 ).toString();
	   protocol_details[ "devRecord" ]     = db->value( 24 ).toString();
	   protocol_details[ "gmpReviewID" ]   = db->value( 25 ).toString();

	   protocol_details[ "expType" ]       = db->value( 26 ).toString();
	   protocol_details[ "dataSource" ]    = db->value( 27 ).toString();
	   protocol_details[ "opticsFailedType" ]    = db->value( 28 ).toString();
	 }
     }
   else
     {
       QMessageBox::warning( this, tr( "No Run Exists" ),
			     tr( "Selected run does not exists in the DB!  \n\n" )
			     + db->lastError()
			     + tr("\n\nThis means that the run either completed OR "
				  "has been manually deleted.\n\n"
				  "The Run List will be updated...") );
     }

   return protocol_details;
}


// Query autoflow for # records
QMap< QString, QString> US_InitDialogueGui::read_autoflow_failed_record( QString failedID  )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   QMap <QString, QString> failed_details;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return failed_details;
     }

   QStringList qry;
   qry << "read_autoflow_failed_record"
       << failedID ;
   
   db->query( qry );

   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   failed_details[ "failedStage" ]    = db->value( 0 ).toString();
	   failed_details[ "failedMsg" ]      = db->value( 1 ).toString();
	   failed_details[ "failedTs" ]       = db->value( 2 ).toString();
	 }
     }
   else
     {
       QMessageBox::warning( this, tr( "No autoflowFailed Record Exists" ),
			     tr( "Selected record does not exists in the DB!  \n\n" )
			     + db->lastError()
			     + tr("\n\nThis means that the run either completed OR "
				  "has been manually deleted.\n\n"
				  "The Run List will be updated...") );
     }

   return failed_details;
}








//////////////////////////////////////////////////////////////////////////////////
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
   //int row         = 0;

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Set Up New Experiment...");
   // panel_desc->setReadOnly(true);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;
   
   // genL->addWidget( panel_desc,  row,   0, 1, 12);

   
   //genL->addWidget( pb_openexp,  row,   12, 1, 1, Qt::AlignTop);
   // genL->addWidget( panel_desc,  row,   0, 1, 2);
   // genL->addWidget( pb_openexp,  row,   2, 1, 2, Qt::AlignCenter);

   panel->addLayout( genL );
   panel->addStretch();

   // connect( pb_openexp,      SIGNAL( clicked()          ), 
   //          this,            SLOT(   manageExperiment() ) );

   //manageExperiment();
   
   // Open US_Experiment without button...  
   //US_ExperimentMain* sdiag = new US_ExperimentMain;

   sdiag = new US_ExperimentMain;

   sdiag->setParent(this, Qt::Widget);
      
   connect( sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
   //connect( this,  SIGNAL( set_auto_mode() ),   sdiag, SLOT( auto_mode_passed() ) );
   
   connect( this, SIGNAL( define_used_instruments( QStringList & ) ), sdiag, SLOT( exclude_used_instruments( QStringList & ) ) );

   connect( sdiag, SIGNAL( close_expsetup_msg() ), this, SLOT ( expsetup_msg_closed() ) ); 
   
   connect( sdiag, SIGNAL( to_live_update( QMap < QString, QString > & ) ),
	    this,  SLOT( to_live_update( QMap < QString, QString > & ) ) );

   connect( sdiag, SIGNAL( to_import( QMap < QString, QString > & ) ),
	    this,  SLOT( to_import( QMap < QString, QString > & ) ) );

   connect( this, SIGNAL( reset_experiment( QString & ) ), sdiag, SLOT( us_exp_clear( QString & ) ) );
   
   connect( sdiag, SIGNAL( exp_cleared( ) ), this, SLOT( exp_cleared( ) ) );

   //return automatically to Run Manager:
   connect( sdiag, SIGNAL( back_to_initAutoflow( ) ), this, SLOT( to_initAutoflow ( ) ) );
   
   
   sdiag->pb_close->setEnabled(false);  // Disable Close button
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2); 

   sdiag->auto_mode_passed();

   if ( mainw->us_mode_bool )
     sdiag->us_mode_passed();
   
   sdiag->show();

   
}


void US_ExperGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

void US_ExperGui::pass_used_instruments( QStringList & occupied_instruments )
{
  qDebug() << "In US_ExperGui::pass_used_instruments( QStringList & occupied_instruments )";
  emit define_used_instruments( occupied_instruments );
}


void US_ExperGui::to_initAutoflow( void )
{
   emit switch_to_initAutoflow();
}

//When run is submitted to Optima & protocol details are passed .. 
void US_ExperGui::to_live_update( QMap < QString, QString > & protocol_details)
{
  emit switch_to_live_update( protocol_details );
}

//When run is submitted with Data from Disk.. 
void US_ExperGui::to_import( QMap < QString, QString > & protocol_details )
{
  emit switch_to_import( protocol_details );
}

//When US_Experiment is closed
void US_ExperGui::us_exp_is_closed_set_button()
{
  pb_openexp->setEnabled(true);
  mainw->resize(QSize(1000, 700));
  mainw->logWidget->append("US_Experiment has been closed!");
}


//Obsolete
// //Clear Experiment after manual abortion & data not saved .. 
// void US_ExperGui::clear_experiment( QString & protocolName )
// {
//   emit reset_experiment( protocolName );
// }


//Upon clearing Experiment .. 
void US_ExperGui::exp_cleared( void )
{
  emit to_autoflow_records();
}

//Close msg on setting up new EXP
void US_ExperGui::expsetup_msg_closed( void )
{
  mainw->msg_expsetup->accept();
  //mainw->diag_expsetup->close();
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
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Monitor Experiment Progress...");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);


   
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Xpn_Viewer ...  
   sdiag = new US_XpnDataViewer("AUTO");
   sdiag->setParent(this, Qt::Widget);

   connect( this, SIGNAL( to_xpn_viewer( QMap < QString, QString > &) ), sdiag, SLOT( check_for_data ( QMap < QString, QString > & )  ) );

   //ALEXEY: devise SLOT saying what to do upon completion of experiment and exporting AUC data to hard drive - Import Experimental Data  !!! 
   connect( sdiag, SIGNAL( experiment_complete_auto( QMap < QString, QString > & ) ), this, SLOT( to_post_processing ( QMap < QString, QString > & ) ) );

   //ALEXEY: to reset timers/threads and GUI when returning back to InitDialog
   connect( this, SIGNAL( reset_live_update_passed( ) ), sdiag, SLOT( reset_liveupdate_panel ( )  ) );
   
   //ALEXEY: close program, emitted from sdiag
   connect( sdiag, SIGNAL( close_program() ), this, SLOT( to_close_program()  ) );

   connect( sdiag, SIGNAL( liveupdate_processes_stopped() ), this, SLOT( processes_stopped_passed()  ) );

   //ALEXEY: premature abortion with no data
   connect( sdiag, SIGNAL( aborted_back_to_initAutoflow( ) ), this, SLOT( to_initAutoflow_xpnviewer ( ) ) );
   
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();

}


void US_ObservGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

void US_ObservGui::to_initAutoflow_xpnviewer ( void )
{
  emit stop_nodata();
}

void US_ObservGui::processes_stopped_passed( void )
{
  emit processes_stopped(); 
}

void US_ObservGui::reset_live_update( void )
{
  emit reset_live_update_passed(); 
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

// void US_ObservGui::to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed,  QString & correctRadii )
// {
//   emit switch_to_post_processing( currDir, protocolName, invID_passed, correctRadii );
// }

void US_ObservGui::to_post_processing( QMap < QString, QString > & protocol_details )
{
  emit switch_to_post_processing( protocol_details );
}


void US_ObservGui::to_close_program( void )
{
  //sdiag->close();
  emit close_everything();
}

// US_PostProd
US_PostProdGui::US_PostProdGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Retrieve and Process Experimental Data...");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();


   // Open US_Convert ...  
   sdiag = new US_ConvertGui("AUTO");
   sdiag->setParent(this, Qt::Widget);

   connect( this, SIGNAL( to_post_prod( QMap < QString, QString > & ) ), sdiag, SLOT( import_data_auto ( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( reset_lims_import_passed( ) ), sdiag, SLOT( reset_limsimport_panel ( )  ) );

   //emit signal after data loaded
   connect( sdiag, SIGNAL( data_loaded(  ) ), this, SLOT( resize_main ( ) ) );
   
   //ALEXEY: switch to Editing
   connect( sdiag, SIGNAL( saving_complete_auto(  QMap < QString, QString > & ) ), this, SLOT( to_editing (  QMap < QString, QString > &) ) );
   //ALEXEY: for academic ver. switch back to experiment
   connect( sdiag, SIGNAL( saving_complete_back_to_initAutoflow( ) ), this, SLOT( to_initAutoflow ( ) ) );
   
   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   if ( mainw->us_mode_bool )
     sdiag->us_mode_passed();

   sdiag->show();

}

void US_PostProdGui::resize_main( void )
{
  // Trigger resize to update size of the Edit_Data
  int curr_h = mainw->height() + 1;
  int curr_w = mainw->width() + 1;

  mainw->resize( QSize(curr_w, curr_h) );
}

void US_PostProdGui::reset_lims_import( void )
{
  emit reset_lims_import_passed();
}

void US_PostProdGui::import_data_us_convert(  QMap < QString, QString > & protocol_details )
{
  emit to_post_prod( protocol_details );
}

void US_PostProdGui::to_editing( QMap < QString, QString > & protocol_details )
{
  emit switch_to_editing( protocol_details );
}

// void US_PostProdGui::to_experiment( QString & protocolName )
// {
//   emit switch_to_exp( protocolName  );
// }

void US_PostProdGui::to_initAutoflow( void )
{
  emit switch_to_initAutoflow();
}

void US_PostProdGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}



// US_Editing
US_EditingGui::US_EditingGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Edit Experimental Data... ---UNDER CONSTRUCTION--- ");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Edit ...  
   sdiag = new US_Edit("AUTO");
   sdiag->setParent(this, Qt::Widget);
   
   
   // //Later - do actual editing form sdiag (load_auto() ) - whatever it will be: (us_edit.cpp)
   connect( this, SIGNAL( start_editing( QMap < QString, QString > & ) ), sdiag, SLOT( load_auto ( QMap < QString, QString > & )  ) );
   //emit signal after data loaded
   connect( sdiag, SIGNAL( data_loaded(  ) ), this, SLOT( resize_main ( ) ) );
   
   connect( this, SIGNAL( reset_data_editing_passed( ) ), sdiag, SLOT(  reset_editdata_panel (  )  ) );

   //ALEXEY: switch to Analysis
   connect( sdiag, SIGNAL( edit_complete_auto(  QMap < QString, QString > & ) ), this, SLOT( to_analysis (  QMap < QString, QString > &) ) );

   //ALEXEY: back to initAutoflow 
   connect( sdiag, SIGNAL( back_to_initAutoflow( ) ), this, SLOT( to_initAutoflow ( ) ) );

   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   // if ( mainw->us_mode_bool )
   //   sdiag->us_mode_passed();

   sdiag->show();
   
}

void US_EditingGui::resize_main( void )
{
  // Trigger resize to update size of the Edit_Data
  int curr_h = mainw->height() + 1;
  int curr_w = mainw->width() + 1;

  mainw->resize( QSize(curr_w, curr_h) );
}

void US_EditingGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}



void US_EditingGui::to_analysis( QMap < QString, QString > & protocol_details )
{
  emit switch_to_analysis( protocol_details );
}

void US_EditingGui::to_initAutoflow( void )
{
  emit switch_to_initAutoflow();
}


void US_EditingGui::reset_data_editing( void )
{
  emit reset_data_editing_passed();
}


void US_EditingGui::do_editing( QMap < QString, QString > & protocol_details )
{
  emit start_editing( protocol_details );
}


// US_Analysis
US_AnalysisGui::US_AnalysisGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   //setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Analyse Experimental Data... ---UNDER CONSTRUCTION--- ");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Analysis_auto ...  
   sdiag = new US_Analysis_auto();
   sdiag->setParent(this, Qt::Widget);
   
   //Later - do actual analysis form sdiag - whatever it will be:
   connect( this, SIGNAL( start_analysis( QMap < QString, QString > & ) ), sdiag, SLOT( initPanel ( QMap < QString, QString > & )  ) );
   // In initPanel() - re-generate GUI based on # of rows corresponding to # stages in AProfile...

   // When coming back to Manage Optima Runs
   connect( sdiag, SIGNAL( analysis_update_process_stopped() ), this, SLOT( processes_stopped_passed()  ) );

   connect( sdiag, SIGNAL( close_analysissetup_msg() ), this, SLOT ( analysissetup_msg_closed() ) );

   //ALEXEY: back to initAutoflow when user stuck at FITMEN (already processed) and program proceeded to REPORT
   connect( sdiag, SIGNAL( analysis_back_to_initAutoflow( ) ), this, SLOT( to_initAutoflow ( ) ) );

   //ALEXEY: switch to Report
   connect( sdiag, SIGNAL( analysis_complete_auto(  QMap < QString, QString > & ) ), this, SLOT( to_report (  QMap < QString, QString > &) ) );

   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();

}

void US_AnalysisGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}


void US_AnalysisGui::do_analysis( QMap < QString, QString > & protocol_details )
{
  emit start_analysis( protocol_details );
}

void US_AnalysisGui::processes_stopped_passed( void )
{
  emit processes_stopped(); 
}

//Close msg on setting up new EXP
void US_AnalysisGui::analysissetup_msg_closed( void )
{
  mainw->msg_analysissetup->accept();
  //mainw->diag_expsetup->close();
}

void US_AnalysisGui::to_initAutoflow( void )
{
  emit switch_to_initAutoflow();
}

//proceed to Report stage
void US_AnalysisGui::to_report( QMap < QString, QString > & protocol_details )
{
  emit switch_to_report( protocol_details );
}


// US_Report
US_ReportStageGui::US_ReportStageGui( QWidget* topw )
  : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Generate Report...  ---UNDER CONSTRUCTION--- ");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // Open US_Analysis_auto ...  
   //sdiag = new US_Reports_auto();
   sdiag = new US_ReporterGMP( "AUTO" );
   sdiag->setParent(this, Qt::Widget);
   
   //connect( this, SIGNAL( start_report( QMap < QString, QString > & ) ), sdiag, SLOT( initPanel ( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( start_report( QMap < QString, QString > & ) ), sdiag, SLOT( loadRun_auto ( QMap < QString, QString > & )  ) );
   
   connect( this, SIGNAL( reset_reporting_passed( ) ), sdiag, SLOT(  reset_report_panel (  )  ) );

   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();
   
}

void US_ReportStageGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

void US_ReportStageGui::do_report( QMap < QString, QString > & protocol_details )
{
  emit start_report( protocol_details );
}

void US_ReportStageGui::reset_reporting( void )
{
  emit reset_reporting_passed();
}


//eSignatures
US_eSignaturesGui::US_eSignaturesGui( QWidget* topw )
  : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   setPalette( US_GuiSettings::frameColor() );
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   //int fwid     = fmet.maxWidth();
   //int lwid     = fwid * 4;
   //int swid     = lwid + fwid;
   
   // Main VBox
   QVBoxLayout* main     = new QVBoxLayout (this);
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
      
   QGridLayout* genL   = new QGridLayout();

   // // //QPlainTextEdit* panel_desc = new QPlainTextEdit(this);
   // QTextEdit* panel_desc = new QTextEdit(this);
   // panel_desc->viewport()->setAutoFillBackground(false);
   // panel_desc->setFrameStyle(QFrame::NoFrame);
   // panel_desc->setPlainText(" Tab to Generate Report...  ---UNDER CONSTRUCTION--- ");
   // panel_desc->setReadOnly(true);
   // //panel_desc->setMaximumHeight(30);
   // QFontMetrics m (panel_desc -> font()) ;
   // int RowHeight = m.lineSpacing() ;
   // panel_desc -> setFixedHeight  (2* RowHeight) ;

   // int row = 0;
   // genL->addWidget( panel_desc,  row++,   0, 1, 12);
 
   // assemble main
   main->addLayout(genL);
   main->addStretch();

   // // Open  ...  
   sdiag = new US_eSignaturesGMP( "AUTO" );
   sdiag->setParent(this, Qt::Widget);
   
   connect( this, SIGNAL( start_esign( QMap < QString, QString > & ) ), sdiag, SLOT( initPanel_auto ( QMap < QString, QString > & )  ) );
   connect( this, SIGNAL( reset_esigning_passed( ) ), sdiag, SLOT(  reset_esign_panel (  )  ) );

   offset = 0;
   sdiag->move(offset, 2*offset);
   sdiag->setFrameShape( QFrame::Box);
   sdiag->setLineWidth(2);

   sdiag->show();
}

void US_eSignaturesGui::resizeEvent(QResizeEvent *event)
{
    int tab_width = mainw->tabWidget->tabBar()->width();
    int upper_height = mainw->gen_banner->height() + //mainw->welcome->height()
      + mainw->logWidget->height() + mainw->test_footer->height();
     
    int new_main_w = mainw->width() - 3*offset - tab_width;
    int new_main_h = mainw->height() - 4*offset - upper_height;
    
    //if (mainw->width() - offset > sdiag->width() || mainw->height() - 2*offset > sdiag->height()) {
    if ( new_main_w > sdiag->width() || new_main_h > sdiag->height()) {
      int newWidth = qMax( new_main_w, sdiag->width());
      int newHeight = qMax( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }

    //if (mainw->width() < sdiag->width() || mainw->height() < sdiag->height()) {
    if ( new_main_w < sdiag->width() ||  new_main_h < sdiag->height() ) {
      int newWidth = qMin( new_main_w, sdiag->width());
      int newHeight = qMin( new_main_h, sdiag->height());
      sdiag->setMaximumSize( newWidth, newHeight );
      sdiag->resize( QSize(newWidth, newHeight) );
      update();
    }
     
    QWidget::resizeEvent(event);
}

void US_eSignaturesGui::do_esign( QMap < QString, QString > & protocol_details )
{
  emit start_esign( protocol_details );
}

void US_eSignaturesGui::reset_esigning( void )
{
  emit reset_esigning_passed();
}
