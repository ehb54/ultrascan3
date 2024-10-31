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

#include "../us_edit/us_edit.h"
#include "../us_edit/us_edit_scan.h"
#include "../us_edit/us_exclude_profile.h"
#include "../us_edit/us_get_edit.h"
#include "../us_edit/us_ri_noise.h"
#include "../us_edit/us_select_lambdas.h"

#include "../us_autoflow_analysis/us_autoflow_analysis.h"
#include "../us_reporter_gmp/us_reporter_gmp.h"
#include "../us_esigner_gmp/us_esigner_gmp.h"

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
#include "us_plot.h"
#include "us_select_item.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_local_server.h"
#include "us_report_gmp.h"
#include "us_report_gui.h"


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


//! \brief Initial panel
class US_InitDialogueGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_InitDialogueGui( QWidget* );
    ~US_InitDialogueGui() {};


   QList< QStringList >  autoflowdata;
   QStringList occupied_instruments;
   US_SelectItem* pdiag_autoflow;

   void initRecordsDialogue( void );
   void checkCertificates( void );

   bool initDialogueOpen;
   bool initMsgNorecOpen;
   bool initMsgNorecDelOpen;
   
   void initAutoflowPanel( void );

   QLabel* movie_label;

   QMessageBox * msg_norec;
   QMessageBox * msg_norec_del;
   
            
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    int offset;

    int autoflow_records;
    QMap < QString, QString > channels_report;

    void initRecords( void );
    //void initRecordsDialogue( void );
    
    int  get_autoflow_records( void );
    bool isOperRev( int, QString );
    QMap < QString, QString > read_autoflow_record( int );
    QMap < QString, QString > read_autoflow_failed_record( QString );
    int list_all_autoflow_records( QList< QStringList >&, US_DB2* );
    
    void read_optima_machines( US_DB2* = 0 ); 
    QList< QMap<QString, QString> > instruments;

    void do_run_tables_cleanup( QMap< QString, QString > );
    void do_run_data_cleanup( QMap< QString, QString > );
    void do_create_autoflowStatus_for_failedRun( QMap< QString, QString > );
    bool readAProfileBasicParms_auto ( QXmlStreamReader& );
      
 protected:
    void resizeEvent(QResizeEvent *event) override;
      
  private slots:
     void update_autoflow_data( void );
          
  signals:
     void define_new_experiment_init ( QStringList & );
     void switch_to_live_update_init(  QMap < QString, QString > & protocol_details );
     void switch_to_post_processing_init(  QMap < QString, QString > & protocol_details );
     void switch_to_editing_init(  QMap < QString, QString > & protocol_details );
     void switch_to_analysis_init(  QMap < QString, QString > & protocol_details );
     void switch_to_report_init(  QMap < QString, QString > & protocol_details );
     void switch_to_esign_init(  QMap < QString, QString > & protocol_details );
     void to_initAutoflow( void );

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
      void to_import( QMap < QString, QString > & protocol_details );
      //void clear_experiment( QString & protocolName);
      void exp_cleared( void );
      void pass_used_instruments( QStringList & );
      void expsetup_msg_closed( void );
      void to_initAutoflow( void );
      
      
   signals:
      void switch_to_live_update( QMap < QString, QString > & protocol_details );
      void switch_to_import( QMap < QString, QString > & protocol_details );
      void set_auto_mode( void );
      void reset_experiment( QString & protocolName);
      void to_autoflow_records( void );
      void define_used_instruments( QStringList & );
      //void close_expsetup_msg( void );
      void switch_to_initAutoflow( void );
};


//! \brief Observ panel
class US_ObservGui : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_ObservGui( QWidget* );
      ~US_ObservGui() {};

      US_XpnDataViewer*     sdiag;

 private:
      US_ComProjectMain*    mainw;      // Parent to all panels
      //US_XpnDataViewer*     sdiag;
      int offset;

 protected:
      void resizeEvent(QResizeEvent *event) override;
      
 private slots:
      void process_protocol_details( QMap < QString, QString > & protocol_details );
      //void to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii );
      void to_post_processing( QMap < QString, QString > & );
      void to_close_program( void );
      void reset_live_update( void );
      void processes_stopped_passed( void );
      void to_initAutoflow_xpnviewer ( void );
 signals:
      void to_xpn_viewer( QMap < QString, QString > & protocol_details );
      //void switch_to_post_processing( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii  );
      void switch_to_post_processing( QMap < QString, QString > &  );
      void close_everything( void );
      void reset_live_update_passed( void );
      void processes_stopped( void );
      void stop_nodata( void );
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
    
    void to_editing(  QMap < QString, QString > & );
    //void to_experiment( QString & protocolName );
    void to_initAutoflow( void );
    void reset_lims_import( void );
    void resize_main( void );
        
  signals:
    //void to_post_prod( QString & currDir, QString & protocolName, QString & invID_passed, QString & correctRadii  );
    void to_post_prod( QMap < QString, QString > & ); 
    
    void switch_to_editing(  QMap < QString, QString > & );
    //void switch_to_exp( QString & protocolName );
    void switch_to_initAutoflow( void);
    void reset_lims_import_passed( void );
};


//! \brief Editing panel
class US_EditingGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_EditingGui( QWidget* );
    ~US_EditingGui() {};
  
         
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    US_Edit*              sdiag;
    int offset;

 protected:
    void resizeEvent(QResizeEvent *event) override;
      
 private slots:
   void do_editing( QMap < QString, QString > & );
   void reset_data_editing( void );
   void to_analysis( QMap < QString, QString > & );
   void resize_main( void );
   void to_initAutoflow( void );

 signals:
   void start_editing( QMap < QString, QString > & );
   void reset_data_editing_passed ( void );
   void switch_to_analysis( QMap < QString, QString > &  );
   void switch_to_initAutoflow( void);
};

//! \brief Analysis panel
class US_AnalysisGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_AnalysisGui( QWidget* );
    ~US_AnalysisGui() {};

    US_Analysis_auto*     sdiag;
         
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    //US_Analysis_auto*     sdiag;
    int offset;

 protected:
    void resizeEvent(QResizeEvent *event) override;
      
 private slots:
   void do_analysis( QMap < QString, QString > & );
   void processes_stopped_passed( void );
   void analysissetup_msg_closed( void );
   void to_initAutoflow( void );
   void to_report( QMap < QString, QString > & );

 signals:
   void start_analysis( QMap < QString, QString > & );
   void processes_stopped( void );
   void switch_to_initAutoflow( void);
   void switch_to_report( QMap < QString, QString > &  );
    
};


//! \brief Report panel
class US_ReportStageGui : public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_ReportStageGui( QWidget* );
    ~US_ReportStageGui() {};
  
    //US_Reports_auto*     sdiag;
    US_ReporterGMP*     sdiag;
    
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    int offset;

 protected:
    void resizeEvent(QResizeEvent *event) override;
      
  private slots:
    void do_report( QMap < QString, QString > & );
    void reset_reporting( void );

  signals:
    void start_report( QMap < QString, QString > & );
    void reset_reporting_passed ( void );
    void switch_to_esign( QMap < QString, QString > & );
};


//! \brief eSign panel
class US_eSignaturesGui: public US_WidgetsDialog 
{
  Q_OBJECT
  
  public:
    US_eSignaturesGui( QWidget* );
    ~US_eSignaturesGui() {};
  
     US_eSignaturesGMP*   sdiag;
    
  private:
    US_ComProjectMain*    mainw;      // Parent to all panels
    int offset;

 protected:
    void resizeEvent(QResizeEvent *event) override;
      
  private slots:
    void do_esign( QMap < QString, QString > & );
    void reset_esigning( void );

  signals:
    void start_esign( QMap < QString, QString > & );
    void reset_esigning_passed ( void );
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

  QMessageBox * msg_expsetup;
  QMessageBox * msg_liveupdate_finishing;
  QMessageBox * msg_analysis_update_finishing;
  QMessageBox * msg_analysissetup;

  
  QDialog *     diag_expsetup;
  
  //QList< QStringList >  autoflowdata;

  //void check_current_stage( void );

  void call_AutoflowDialogue();
  void close_initDialogue();

  bool window_closed;
  bool data_location_disk;
  bool xpn_viewer_closed_soft;
  
  int         curr_panx;       // Current panel index (0-7)

  QWidget * cornerWidget;
  //QStringList occupied_instruments;

  //US_SelectItem* pdiag_autoflow;
    
 private:
  US_InitDialogueGui*  epanInit;        // US_Init panel
  US_ExperGui*         epanExp;         // US_Exp panel
  US_ObservGui*        epanObserv;      // US_Observ panel
  US_PostProdGui*      epanPostProd;    // US_PostProd panel
  US_EditingGui*       epanEditing;     // US_Editing panel
  US_AnalysisGui*      epanAnalysis;    // US_Analysis panel
  US_ReportStageGui*   epanReport;      // US_Report panel
  US_eSignaturesGui*   epanSign;        // electronic signatures 
    
  //int         statflag;        // Composite panels status flag
  //int         dbg_level;       // Debug print flag
  //int         curr_panx;       // Current panel index (0-7)

  //int  get_autoflow_records( void );
  //QMap < QString, QString > read_autoflow_record( int );
  //static int list_all_autoflow_records( QList< QStringList >&, US_DB2* );

  //void read_optima_machines( US_DB2* = 0 ); 
  //QList< QMap<QString, QString> > instruments;  
  
private slots:
  void checkDataLocation( void );
  void initPanels  ( int  );     // Move to a new panel
  //void reset     ( void );
  //void newPanel  ( int  );     // Move to a new panel
  //void statUpdate( void );     // Get a status flag update
  // void panelUp   ( void );     // Move to next panel
  //void panelDown ( void );     // Move to previous panel
  //void help      ( void );     // Show documentation window
   
  //void unable_tabs_buttons( void);  // Slot to unable Tabs and Buttons when user level is low
  //void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after protocol is loaded

  void switch_to_live_update( QMap < QString, QString > & protocol_details );
  void switch_to_post_processing( QMap < QString, QString > & );
  void switch_to_editing( QMap < QString, QString > & protocol_details );
  void switch_to_analysis( QMap < QString, QString > & protocol_details );
  void switch_to_report( QMap < QString, QString > & protocol_details );
  void switch_to_esign( QMap < QString, QString > & ); 
  
    
  //void switch_to_experiment( QString & protocolName );
  //void check_current_stage( void );
  void close_all( void );
  void closeEvent      ( QCloseEvent* );

  void to_autoflow_records( void );

  void define_new_experiment( QStringList & );

  void delete_psql_record( int );
  
  void liveupdate_stopped( void );

  void show_liveupdate_finishing_msg( void );

  void analysis_update_stopped( void );

  void show_analysis_update_finishing_msg( void );
  
signals:
  void pass_to_live_update( QMap < QString, QString > & protocol_details );
  void pass_to_post_processing(  QMap < QString, QString > & );
  void pass_to_editing( QMap < QString, QString > & protocol_details );
  void pass_to_analysis( QMap < QString, QString > & protocol_details );
  void pass_to_report( QMap < QString, QString > & protocol_details );
  void pass_to_esign( QMap < QString, QString > & protocol_details );
  

  //void clear_experiment( QString & protocolName);
  void us_comproject_closed( void );
  void pass_used_instruments( QStringList & );
  void reset_lims_import( void );
  void reset_data_editing( void );
  void reset_live_update( void );
  void reset_reporting( void );
  void reset_esigning( void );
};


#endif
