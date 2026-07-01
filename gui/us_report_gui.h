//! \file us_report_gui.h
#ifndef US_REPORT_GUI_H
#define US_REPORT_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_run_protocol.h"
#include "us_report_gmp.h"
#include "us_widgets_dialog.h"

//! This class provides a tabbed entry for solution selection
class US_GUI_EXTERN US_ReportGui: public US_Widgets
{
  Q_OBJECT

  public:

      //! brief Report Gui. To 
      //! instantiate the class a calling function must
      //! provide the ID of the investigator.
      //! US_ReportGui( US_ReportGMP* );
      US_ReportGui( QMap < QString, US_ReportGMP * > );
      ~US_ReportGui() {};

      QMap < QString, US_ReportGMP * > report_map;
      QMap < QString, US_ReportGMP  >  report_map_copy_original;
      US_ReportGMP*   report;
      US_ReportGMP    report_copy_original;

      //abde
      bool abde_mode;
      void abde_mode_passed( void );

   signals:
      void  cancel_changes       ( QMap <QString, US_ReportGMP> & );
      void  apply_to_all_reports ( US_ReportGMP* );
      
      
   private:
      int row;
      QVBoxLayout* main;
      QGridLayout* params;
      QGridLayout* ufiles;
      QGridLayout* genL;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;
      QWidget*     topContainerWidget;
      QGridLayout* addRem_buttons;
      QHBoxLayout* lower_buttons;
      QGridLayout* reportmask;

      QLabel* bn_report_t;

      QLabel* lb_tot_conc;    
      QLabel* lb_tot_conc_tol;  
      QLabel* lb_rmsd_limit;    
      QLabel* lb_av_intensity;  
      QLabel* lb_duration;      
      QLabel* lb_duration_tol;  
      QLabel* lb_wvl;           

      QLineEdit*   le_ufiles;
      QLineEdit*   le_tot_conc;
      QLineEdit*   le_tot_conc_tol;
      QLineEdit*   le_rmsd_limit;
      QLineEdit*   le_av_intensity;
      QComboBox*   cb_wvl;
      QPushButton* pb_prev_wvl;
      QPushButton* pb_next_wvl;
      QPushButton* pb_apply_all;
      QPushButton* pb_upload_files;
      

      //report mask
      QCheckBox*   ck_tot_conc;
      QCheckBox*   ck_rmsd;
      QCheckBox*   ck_exp_duration;
      QCheckBox*   ck_min_intensity;
      QCheckBox*   ck_integration;
      QCheckBox*   ck_plots;

      QGroupBox*   pseudo3d_box;
      QCheckBox*   ck_2dsait_s_ff0;
      QCheckBox*   ck_2dsait_s_d;
      QCheckBox*   ck_2dsait_mw_ff0;
      QCheckBox*   ck_2dsait_mw_d;
      QCheckBox*   ck_2dsamc_s_ff0;
      QCheckBox*   ck_2dsamc_s_d;
      QCheckBox*   ck_2dsamc_mw_ff0;
      QCheckBox*   ck_2dsamc_mw_d;
      QCheckBox*   ck_pcsa_s_ff0;
      QCheckBox*   ck_pcsa_s_d;
      QCheckBox*   ck_pcsa_mw_ff0;
      QCheckBox*   ck_pcsa_mw_d;
      
      //duration
      QSpinBox*    sb_durat_dd;
      QSpinBox*    sb_durat_hh;
      QSpinBox*    sb_durat_mm;
      QSpinBox*    sb_durat_ss;
      QLineEdit*   le_duration_tol;
      

      QPushButton*  pb_addRow;
      QPushButton*  pb_removeRow;
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;
      QPushButton*  pb_repmask;

      QMap< QString, bool > isErrorField;
      QStringList wvl_passed;
      int init_index;

      //for uploaded from files
      QStringList uploaded_part_concs;
      QMap <QString, QStringList> uploaded_variable_ranges;
      QString uploaded_tot_conc;
      
      void build_report_layout( void );
      void gui_to_report( void );
      int  check_syntax( void );
      void SetComboBoxItemEnabled(QComboBox*, int, bool);
      void parseGaStatsFile( const QString& );
      void add_rows_uploaded( QMap< QString, QStringList> );
      
      
   private slots:
     void verify_text ( const QString& );
     void update_report( void );
     void report_mask( void );
     void add_row( void );
     void remove_row( void );
     void cancel_update( void );
     void changeWvl ( int );                        
     void wvl_prev  ( void );
     void wvl_next  ( void );
     void apply_all_wvls( void );
     void upload_files( void );

     void    ssChgDuratTime_dd( int );
     void    ssChgDuratTime_hh( int );
     void    ssChgDuratTime_mm( int );
     void    ssChgDuratTime_ss( int );

     void    type_changed     ( int );
     void    method_changed   ( int );
     
   public slots:
};

// Dialog class for selecting model|vars combos from uploaded files
class US_ConfirmUpload : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_ConfirmUpload( QStringList variables, QWidget *parent = nullptr );
      QMap< QString, QStringList> getSelections() { return upload_sel; }

   private:
      QGroupBox*   ga_upload_box;
      QCheckBox*   ck_2dsait_s;
      QCheckBox*   ck_2dsait_d;
      QCheckBox*   ck_2dsait_ff0;
      QCheckBox*   ck_2dsait_mw;
      QCheckBox*   ck_2dsamc_s;
      QCheckBox*   ck_2dsamc_d;
      QCheckBox*   ck_2dsamc_ff0;
      QCheckBox*   ck_2dsamc_mw;
      QCheckBox*   ck_pcsa_s;
      QCheckBox*   ck_pcsa_d;
      QCheckBox*   ck_pcsa_ff0;
      QCheckBox*   ck_pcsa_mw;

      QPushButton* pb_reset;
      QPushButton* pb_cancel;
      QPushButton* pb_accept;

      QList<QRadioButton*> fields_radio;
      QMap< QString, QStringList> upload_sel;
      QStringList vars_passed;
      QMap< QString, QStringList> gui_to_json_upload();

   private slots:
     void do_reset();
     void cancel_upload();
     void accept_upload();
      
};

#endif
