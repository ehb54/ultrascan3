#ifndef US_COMPROJECT_H
#define US_COMPROJECT_H

#include <QApplication>
#include <unistd.h>
#include <fstream>
#include <QtSql>

#include "../us_xpn_viewer/us_xpn_viewer_gui.h"
#include "../us_experiment/us_experiment_gui_optima.h"
#include "../us_convert/us_experiment.h"     
#include "../us_convert/us_experiment_gui.h" 
#include "../us_convert/us_convert_gui.h"    
#include "../us_convert/us_convertio.h"      
#include "../us_convert/us_get_run.h"        
#include "../us_convert/us_intensity.h"      
#include "../us_convert/us_selectbox.h"      
#include "../us_convert/us_select_triples.h"

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
#include "us_plot.h"
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
      QLineEdit*   opening_msg;
      US_ExperimentMain*    sdiag;
      int offset;

 protected:
      void resizeEvent(QResizeEvent *event) override;
     
      
   private slots:
      void manageExperiment ( void );        // Slot for exp.  button clicked
      void us_exp_is_closed_set_button( void );
      void to_live_update( QMap < QString, QString > & protocol_details );
      void clear_experiment( QString & protocolName);
      void exp_cleared( void );
      void pass_used_instruments( QStringList & );
      
   signals:
      void switch_to_live_update( QMap < QString, QString > & protocol_details );
      void set_auto_mode( void );
      void reset_experiment( QString & protocolName);
      void to_autoflow_records( void );
      void define_used_instruments( QStringList & );
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
      US_XpnDataViewer*    sdiag;
      int offset;

 protected:
      void resizeEvent(QResizeEvent *event) override;
      
 private slots:
      void process_protocol_details( QMap < QString, QString > & protocol_details );
      //void to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii );
      void to_post_processing( QMap < QString, QString > & );
      
      void to_experiment( QString & protocolName );
      void to_close_program( void );
 signals:
      void to_xpn_viewer( QMap < QString, QString > & protocol_details );
      //void switch_to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii  );
      void switch_to_post_processing( QMap < QString, QString > &  );
      
      void switch_to_experiment( QString & protocolName );
      void close_everything( void );
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
    US_ConvertGui*        sdiag;
    int offset;

 protected:
    void resizeEvent(QResizeEvent *event) override;
      
  private slots:
    //void import_data_us_convert( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii  );
    void import_data_us_convert( QMap < QString, QString > &);
    
    void to_analysis( QString & currDir, QString & protocolName );
    void to_experiment( QString & protocolName );
    
  signals:
    //void to_post_prod( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii  );
    void to_post_prod( QMap < QString, QString > & ); 
    
    void switch_to_analysis( QString & currDir, QString & protocolName );
    void switch_to_exp( QString & protocolName );
};

//! \brief Analysis panel
class US_AnalysisGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_AnalysisGui( QWidget* );
    ~US_AnalysisGui() {};
  
         
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    int offset;

 protected:
    //void resizeEvent(QResizeEvent *event) override;
      
 private slots:
    void do_analysis( QString & currDir, QString & protocolName );

 signals:
    void start_analysis( QString & currDir, QString & protocolName );
    
};


//! \brief Report panel
class US_ReportGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_ReportGui( QWidget* );
    ~US_ReportGui() {};
  
         
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    int offset;

 protected:
    //void resizeEvent(QResizeEvent *event) override;
      
  private slots:
    

  signals:
};


//! \brief ComProject Main Window
class US_ComProjectMain : public US_Widgets
{
  Q_OBJECT
  
 public:
  US_ComProjectMain();

  US_ComProjectMain(QString us_mode);
  
  QLabel*           gen_banner;
  QTextEdit*        welcome;
  QTabWidget*       tabWidget;       // Tab Widget holding the panels
  QTextEdit*        logWidget;  
  QTextEdit*        test_footer;
  QString           icon_path;

  bool us_mode_bool;

  QList< QStringList >  autoflowdata;

  void check_current_stage( void );

  bool window_closed;
  
  QStringList occupied_instruments;
    
 private:
  US_ExperGui*      epanExp;         // US_Exp panel
  US_ObservGui*     epanObserv;      // US_Observ panel
  US_PostProdGui*   epanPostProd;    // US_PostProd panel
  US_AnalysisGui*   epanAnalysis;    // US_Analysis panel
  US_ReportGui*     epanReport;      // US_Report panel
  
  //int         statflag;        // Composite panels status flag
  //int         dbg_level;       // Debug print flag
  int         curr_panx;       // Current panel index (0-7)

  //void check_current_stage( void );
  int  get_autoflow_records( void );
  QMap < QString, QString > read_autoflow_record( int );
  static int list_all_autoflow_records( QList< QStringList >&, US_DB2* );

  void read_optima_machines( US_DB2* = 0 ); 
  QList< QMap<QString, QString> > instruments;  
  
private slots:
  //void reset     ( void );
  //void newPanel  ( int  );     // Move to a new panel
  //void statUpdate( void );     // Get a status flag update
  // void panelUp   ( void );     // Move to next panel
  //void panelDown ( void );     // Move to previous panel
  //void help      ( void );     // Show documentation window
   
  //void unable_tabs_buttons( void);  // Slot to unable Tabs and Buttons when user level is low
  //void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after protocol is loaded
  void switch_to_live_update( QMap < QString, QString > & protocol_details );
  //void switch_to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii );
  void switch_to_post_processing( QMap < QString, QString > & );
  
  void switch_to_analysis( QString & currDir, QString & protocolName );
  void switch_to_experiment( QString & protocolName );
  //void check_current_stage( void );
  void close_all( void );
  void closeEvent      ( QCloseEvent* );

  void to_autoflow_records( void );

  void define_new_experiment( QStringList & );

  void delete_psql_record( int );  
  
signals:
  void pass_to_live_update( QMap < QString, QString > & protocol_details ); 
  //void import_data_us_convert( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii );
  void import_data_us_convert(  QMap < QString, QString > & );

  void pass_to_analysis( QString & currDir, QString & protocolName );
  void clear_experiment( QString & protocolName);
  void us_comproject_closed( void );
  void pass_used_instruments( QStringList & );
};


#endif
