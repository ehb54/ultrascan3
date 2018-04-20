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

   // Create tab and panel widgets
   tabWidget           = us_tabwidget();
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
   tabWidget->setTabIcon(0,QIcon("images/setup.ico"));
   
   main->addWidget( tabWidget );

   setMinimumSize( QSize( 1180, 600 ) );
   adjustSize();

   
   // //epanGeneral->initPanel();
   // epanGeneral->loaded_proto = 0;
   // epanGeneral->check_user_level();
   // reset();
}


// US_ExperGUI
US_ExperGui::US_ExperGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;

   QVBoxLayout* panel  = new QVBoxLayout( this );
   panel->setSpacing        ( 2 );
   panel->setContentsMargins( 2, 2, 2, 2 );
   lb_exp_banner    = us_banner( tr( "1: Define Experiment Run" ) );
   panel->addWidget( lb_exp_banner );


   // Create layout and GUI components
   QGridLayout* genL   = new QGridLayout();
   
   pb_openexp  = us_pushbutton( tr( "Open Experiment SetUp Dialog" ) );

   // Build main layout
   int row         = 0;
   genL->addWidget( pb_openexp, row,   0, 1, 2 );

   panel->addLayout( genL );
   panel->addStretch();
   
   connect( pb_openexp,      SIGNAL( clicked()          ), 
            this,            SLOT(   manageExperiment() ) );
     
}


//When US_Ex[eriment is closed
void US_ExperGui::us_exp_is_closed_set_button()
{
  pb_openexp->setEnabled(true);
}


// On click to open US_Experiment
void US_ExperGui::manageExperiment()
{
  //Calculate cumulative height/width of all widgets in US_ExperGui
  int height_cum = 0;
  int offset = 20;
  height_cum += lb_exp_banner->height();
  height_cum += pb_openexp->height();
  height_cum += offset;
  
  int tab_width = mainw->tabWidget->tabBar()->width();  
  
  US_ExperimentMain* sdiag = new US_ExperimentMain;
  sdiag->setParent(this, Qt::Widget);

  connect(sdiag, SIGNAL( us_exp_is_closed() ), this, SLOT( us_exp_is_closed_set_button() ) );
  
  sdiag->move(offset, height_cum);
  sdiag->show();

  int new_width, new_height;

  new_width  = tab_width  + sdiag->width()  + 2*offset;
  new_height = height_cum + sdiag->height() + offset;

  qDebug() << "Tab width: " << tab_width << ", US_Exp width: " << sdiag->width();
  qDebug() << "New Width: " << new_width;
  qDebug() << "New Height: " << new_height;
  
  if ( mainw->height() < new_height || mainw->width() < new_width)
    mainw->resize(new_width, new_height);

  pb_openexp->setEnabled(false);
}

// US_Observ
US_ObservGui::US_ObservGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;
   
}

// US_PostProd
US_PostProdGui::US_PostProdGui( QWidget* topw )
   : US_WidgetsDialog( topw, 0 )
{
   mainw               = (US_ComProjectMain*)topw;
   
}
