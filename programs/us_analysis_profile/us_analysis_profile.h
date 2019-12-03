//! \file us_analysis_profile.h

#ifndef US_APROFG_H
#define US_APROFG_H

#include <QApplication>
#include <QtSql>

#include "us_ana_profile.h"
#include "us_run_protocol.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_select_runs.h"
//#include "us_license_t.h"
//#include "us_license.h"

class US_AnalysisProfileGui;

//! \brief Experiment General panel
class US_AnaprofPanGen : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_AnaprofPanGen( QWidget* );
      ~US_AnaprofPanGen() {};

      void        initPanel( void );    // Standard panel utilities
      void        savePanel( void );
      QString     getSValue( const QString );
      int         getIValue( const QString );
      double      getDValue( const QString );
      QStringList getLValue( const QString );
      QString     sibSValue( const QString, const QString );
      int         sibIValue( const QString, const QString );
      double      sibDValue( const QString, const QString );
      QStringList sibLValue( const QString, const QString );
      int         status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/analysprof_general.html" ); };

      // All protocols: names and summary information
      int         getProfiles   ( QStringList&, QList< QStringList >& );
      // Append to the names,summary-data protocol lists
      bool        updateProfiles( const QStringList );

      void check_user_level( void );
      void update_inv( void );
      void check_runname( void );
      void disable_name_buttons( void );
      void pass_names( QString&, QString& );
      void inherit_protocol( US_RunProtocol* );
            
   private:
      US_AnalysisProfileGui* mainw;   // Parent to all panels
      US_AnaProfile*    currProf;     // Current AnaProfile controls
      US_Help  showHelp;

      QGridLayout* genL;
      QVBoxLayout* panel;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;

      QPushButton* pb_aproname;
      QPushButton* pb_protname;
      QPushButton* pb_applya;

      QLineEdit*   le_aproname;
      QLineEdit*   le_protname;

      QList< QLineEdit* >    le_lcrats;
      QList< QLineEdit* >    le_lctols;
      QList< QLineEdit* >    le_ldvols;
      QList< QLineEdit* >    le_lvtols;
      QList< QLineEdit* >    le_daends;
      
      QStringList  sl_chnsel;

      int          dbg_level;
      bool         use_db;              // Using the LIMS database?


   private slots:
      void  build_general_layout( void );

      void  apro_button_clicked( void );
      void  prot_button_clicked( void );
      void  apro_text_changed  ( void );
      void  prot_text_changed  ( void );
      void  lcrat_text_changed ( void );
      void  lctol_text_changed ( void );
      void  ldvol_text_changed ( void );
      void  lvtol_text_changed ( void );
      void  daend_text_changed ( void );
      void  applied_to_all     ( void );
 signals:
      void  set_tabs_buttons_inactive       ( void );
      void  set_tabs_buttons_active_readonly( void );
      void  set_tabs_buttons_active         ( void );
};

//! \brief 2DSA controls panel
class US_AnaprofPan2DSA : public US_WidgetsDialog 
{
   Q_OBJECT 

   public:
      US_AnaprofPan2DSA( QWidget* );
      ~US_AnaprofPan2DSA() {};

      void        initPanel( void );    // Standard panel utilities
      void        savePanel( void );
      QString     getSValue( const QString );
      int         getIValue( const QString );
      double      getDValue( const QString );
      QStringList getLValue( const QString );
      QString     sibSValue( const QString, const QString );
      int         sibIValue( const QString, const QString );
      double      sibDValue( const QString, const QString );
      QStringList sibLValue( const QString, const QString );
      int         status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_speeds.html" ); };

   private:
      US_AnalysisProfileGui*  mainw;
      US_AnaProfile::AnaProf2DSA*   ap2DSA;    // 2DSA controls
      US_Help      showHelp;

      QPushButton* pb_custmg; 
      QPushButton* pb_applya; 
      QPushButton* pb_nextch; 

      QLineEdit*   le_smin; 
      QLineEdit*   le_smax; 
      QLineEdit*   le_sgrpts; 
      QLineEdit*   le_kmin; 
      QLineEdit*   le_kmax; 
      QLineEdit*   le_kgrpts; 
      QLineEdit*   le_grreps; 
      QLineEdit*   le_custmg; 
      QLineEdit*   le_constk; 
      QLineEdit*   le_j2gpts;
      QLineEdit*   le_j2mrng;
      QLineEdit*   le_j4iter;
      QLineEdit*   le_j5iter;

      QCheckBox*   ck_varyvb;
      QCheckBox*   ck_j1run;
      QCheckBox*   ck_j2run;
      QCheckBox*   ck_j3run;
      QCheckBox*   ck_j4run;
      QCheckBox*   ck_j5run;
      QCheckBox*   ck_j3auto;

      QComboBox*   cb_chnsel; 

      QStringList  sl_chnsel;
      int          dbg_level;     // Debug flag
      bool         changed;       // Flag if any speed step changes
      
   private slots:
      void channel_selected ( int );
      void next_channel     ( );
      void smin_changed     ( );
      void smax_changed     ( );
      void sgpoints_changed ( );
      void kmin_changed     ( );
      void kmax_changed     ( );
      void kgpoints_changed ( );
      void grid_reps_changed( );
      void cust_grid_clicked( );
      void cust_grid_changed( );
      void vary_vbar_checked( bool );
      void constk_changed   ( );
      void apply_all_clicked( );
      void job1_run_checked ( bool );
      void job2_run_checked ( bool );
      void mgpoints_changed ( );
      void mfrange_changed  ( );
      void job3_run_checked ( bool );
      void autopick_checked ( bool );
      void job4_run_checked ( bool );
      void rfiters_changed  ( );
      void job5_run_checked ( bool );
      void mciters_changed  ( );
};

//! \brief Experiment Cells panel
class US_AnaprofPanPCSA : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_AnaprofPanPCSA( QWidget* );
      ~US_AnaprofPanPCSA() {};

      void        initPanel( void );    // Standard panel utilities
      void        savePanel( void );
      QString     getSValue( const QString );
      int         getIValue( const QString );
      double      getDValue( const QString );
      QStringList getLValue( const QString );
      QString     sibSValue( const QString, const QString );
      int         sibIValue( const QString, const QString );
      double      sibDValue( const QString, const QString );
      QStringList sibLValue( const QString, const QString );
      int         status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_cells.html" ); };

   private:
      US_AnalysisProfileGui*   mainw;
      US_AnaProfile::AnaProfPCSA*   apPCSA;    // PCSA controls
      US_Help  showHelp;

      QPushButton* pb_applya; 
      QPushButton* pb_nextch; 

      QComboBox*   cb_curvtype;
      QComboBox*   cb_xaxistyp;
      QComboBox*   cb_yaxistyp;
      QComboBox*   cb_zaxistyp;
      QComboBox*   cb_chnsel;

      QLineEdit*   le_xmin;
      QLineEdit*   le_xmax;
      QLineEdit*   le_ymin;
      QLineEdit*   le_ymax;
      QLineEdit*   le_zvalue;
      QLineEdit*   le_varcount;
      QLineEdit*   le_grfiters;
      QLineEdit*   le_crpoints;
      QLineEdit*   le_regalpha;
      QLineEdit*   le_mciters;

      QCheckBox*   ck_nopcsa;
      QCheckBox*   ck_tinoise;
      QCheckBox*   ck_rinoise;
      QCheckBox*   ck_tregspec;
      QCheckBox*   ck_tregauto;

      bool         changed;
      int          dbg_level;
      QStringList  sl_chnsel;

   private slots:
      void nopcsa_checked   ( bool );
      void channel_selected ( int );
      void next_channel     ( );
      void curvtype_selected( int );
      void apply_all_clicked( );
      void xaxis_selected   ( int );
      void xmin_changed     ( );
      void xmax_changed     ( );
      void yaxis_selected( int );
      void ymin_changed( );
      void ymax_changed( );
      void zaxis_selected( int );
      void zvalue_changed( );
      void varcount_changed( );
      void grfiters_changed( );
      void crpoints_changed( );
      void tinoise_checked( bool );
      void rinoise_checked( bool );
      void tregspec_checked( bool );
      void tregauto_checked( bool );
      void alpha_changed( );
      void mciters_changed ( );
};


#if 0
//! \brief AnalysisProfileGui Status panel
class US_AnaprofPanStatus : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_AnaprofPanStatus( QWidget* );
      ~US_AnaprofPanStatus() {};

      void        initPanel( void );    // Standard panel utilities
      void        savePanel( void );
      QString     getSValue( const QString );
      int         getIValue( const QString );
      double      getDValue( const QString );
      QStringList getLValue( const QString );
      QString     sibSValue( const QString, const QString );
      int         sibIValue( const QString, const QString );
      double      sibDValue( const QString, const QString );
      QStringList sibLValue( const QString, const QString );
      int         status   ( void );
      void        help     ( void )
         { showHelp.show_help( "manual/experiment_submit.html" ); };

      QPushButton* pb_saverp;
      QPushButton* pb_connect;      
      QPushButton* pb_submit;
      QPushButton* pb_details;

      QGridLayout* genL;
      
   private:
      US_AnalysisProfileGui*  mainw;
      US_AnaProfile*     loadProf;   // Prof params as loaded from AP record
      US_AnaProfile*     currProf;   // Current AnaProfile controls
      US_AnaProfile::AnaProf2DSA*   ap2DSA;    // 2DSA controls
      US_AnaProfile::AnaProfPCSA*   apPCSA;    // PCSA controls
      US_Help  showHelp;

      //QPushButton* pb_saverp;
      //QPushButton* pb_submit;

      QCheckBox*   ck_run;
      QCheckBox*   ck_project;
      QCheckBox*   ck_rotor;
      QCheckBox*   ck_rotor_ok;
      QCheckBox*   ck_speed;
      QCheckBox*   ck_speed_ok;
      QCheckBox*   ck_centerp;
      QCheckBox*   ck_solution;
      QCheckBox*   ck_optical;
      QCheckBox*   ck_ranges;
      QCheckBox*   ck_connect;
      QCheckBox*   ck_rp_diff;
      QCheckBox*   ck_prot_ena;
      QCheckBox*   ck_prot_svd;
      QCheckBox*   ck_sub_enab;
      QCheckBox*   ck_sub_done;

      int          dbg_level;
      bool         have_run;    // Have Run specified
      bool         have_proj;   // Have Project specified
      bool         have_speed;  // Have Speed parameters specified
      bool         chgd_speed;  // User Changed Speed parameters
      bool         have_cells;  // Have Cell parameters specified
      bool         have_solus;  // Have Solutions parameters specified
      bool         have_optic;  // Have Optics parameters specified
      bool         have_range;  // Have Ranges parameters specified
      bool         have_sol;    // Have Solution parameters specified
      bool         rps_differ;  // Run Protocols differ loaded/current
      bool         subm_enab;   // Submit of Run controls is Enabled
      bool         submitted;   // Run controls have been Submitted
      bool         connected;   // We are Connected to the Optima
      bool         changed;

      QSqlDatabase dbxpn;
      

   private slots:
      void    detailExperiment( void );  // Dialog to detail experiment
      void    testConnection  ( void );  // Test Optima connection
      void    submitExperiment_confirm( void );  // Submit the experiment
      void    submitExperiment( void );  // Submit the experiment

   signals:
};
#endif

//! \brief Analysis Profile GUI Main Window
class US_AnalysisProfileGui : public US_Widgets
{
   Q_OBJECT

   public:
      US_AnalysisProfileGui();

      // \brief Get a named child panel's value of a given type
      QString     childSValue ( const QString, const QString );
      int         childIValue ( const QString, const QString );
      double      childDValue ( const QString, const QString );
      QStringList childLValue ( const QString, const QString );
      // \brief Initialize all the panels
      void        initPanels  ( void );
      // \brief Get the list of protocol names and summary-data strings
      int         getProfiles   ( QStringList&, QList< QStringList >& );
      // \brief Update the list of protocols with a newly named entry
      bool        updateProfiles( const QStringList );
      // \brief Set even-12-column stretches in the grid layout
      void        setColumnStretches( QGridLayout* genL );

      US_AnaProfile  loadProf;   // Prof params as loaded from AP record
      US_AnaProfile  currProf;   // Current AnaProfile controls

      QPushButton* pb_help;
      QPushButton* pb_next;
      QPushButton* pb_prev;
      QPushButton* pb_close;
      QPushButton* pb_apply;

      bool    connection_status;
      QString xpnhost;
      int     xpnport;
      
      bool    automode;
      void    auto_mode_passed( void ); 
      void    auto_name_passed( QString&, QString& ); 
      void    inherit_protocol( US_RunProtocol* );
      
   private:

      QTabWidget*           tabWidget;      // Tab Widget holding the panels

      US_AnaprofPanGen*     apanGeneral;    // General panel
      US_AnaprofPan2DSA*    apan2DSA;       // 2DSA panel
      US_AnaprofPanPCSA*    apanPCSA;       // PCSA panel
//      US_AnaprofPanStatus*  apanStatus;     // Status panel

      int         statflag;        // Composite panels status flag
      int         dbg_level;       // Debug print flag
      int         curr_panx;       // Current panel index (0-7)

      

   private slots:

     //void reset     ( void );
      void newPanel  ( int  );     // Move to a new panel
      void statUpdate( void );     // Get a status flag update
      void panelUp   ( void );     // Move to next panel
      void panelDown ( void );     // Move to previous panel
      void help      ( void );     // Show documentation window

      void disable_tabs_buttons( void);  // Slot to unable Tabs and Buttons when user level is low
      void enable_tabs_buttons_readonly( void);  // Slot to enable Tabs and Buttons after profile is loaded
      void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after run_name is entered
      
    public slots:
      void reset     ( void );
      void close_program( void );
      void apply_profile( void );
      //void auto_mode_passed( void ); 

    signals:
      void us_exp_is_closed( void );
      
      
};
#endif

