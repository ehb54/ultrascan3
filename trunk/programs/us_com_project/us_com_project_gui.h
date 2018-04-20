#ifndef US_COMPROJECT_H
#define US_COMPROJECT_H

#include <QApplication>
#include <unistd.h>
#include <fstream>
#include <QtSql>

#include "../us_experiment/us_experiment_gui.h"
#include "us_protocol_util.h"
#include "us_project_gui.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_rotor.h"
#include "us_solution.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_hardware.h"
#include "us_select_runs.h"
#include "us_license_t.h"
#include "us_license.h"

class US_ComProjectMain;

class VerticalTabStyle : public QProxyStyle {
public:
  QSize sizeFromContents(ContentsType type, const QStyleOption* option,
                         const QSize& size, const QWidget* widget) const {
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if (type == QStyle::CT_TabBarTab) {
      s.transpose();
    }
    return s;
  }

  void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const {
    if (element == CE_TabBarTabLabel) {
      if (const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab*>(option)) {
        QStyleOptionTab opt(*tab);
        opt.shape = QTabBar::RoundedNorth;
        QProxyStyle::drawControl(element, &opt, painter, widget);
        return;
      }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
  }
};


//! \brief Experiment panel
class US_ExperGui : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_ExperGui( QWidget* );
      ~US_ExperGui() {};


   private:
      US_ComProjectMain*    mainw;      // Parent to all panels

      QLabel*     lb_exp_banner;
      QPushButton* pb_openexp;          // Button to open exper.
      
   private slots:
      void manageExperiment ( void );        // Slot for exp.  button clicked
      void us_exp_is_closed_set_button( void );
     
   signals:
      
};


//! \brief Observ panel
class US_ObservGui : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_ObservGui( QWidget* );
      ~US_ObservGui() {};


   private:
      US_ComProjectMain*    mainw;      // Parent to all panels
      
   private slots:
                 
   signals:
      
};


//! \brief PostProd panel
class US_PostProdGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_PostProdGui( QWidget* );
    ~US_PostProdGui() {};
  
  
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
  
  private slots:
    
  
      
};


//! \brief ComProject Main Window
class US_ComProjectMain : public US_Widgets
{
  Q_OBJECT
  
 public:
  US_ComProjectMain();
  QTabWidget*       tabWidget;       // Tab Widget holding the panels
    
 private:
  
  //QTabWidget*       tabWidget;       // Tab Widget holding the panels
  US_ExperGui*      epanExp;         // US_Exp panel
  US_ObservGui*     epanObserv;      // US_Observ panel
  US_PostProdGui*   epanPostProd;    // US_PostProd panel
  
  //int         statflag;        // Composite panels status flag
  //int         dbg_level;       // Debug print flag
  int         curr_panx;       // Current panel index (0-7)
  
  
private slots:
  //void reset     ( void );
  //void newPanel  ( int  );     // Move to a new panel
  //void statUpdate( void );     // Get a status flag update
  // void panelUp   ( void );     // Move to next panel
  //void panelDown ( void );     // Move to previous panel
  //void help      ( void );     // Show documentation window
   
  //void unable_tabs_buttons( void);  // Slot to unable Tabs and Buttons when user level is low
  //void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after protocol is loaded
};




#endif
