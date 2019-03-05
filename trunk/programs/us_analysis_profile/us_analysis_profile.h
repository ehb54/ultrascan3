//! \file us_analysis_profile.h

#ifndef US_ANAPROF_H
#define US_ANAPROF_H

#include <QApplication>
#include <QtSql>

#include "us_anapro_parms.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_select_runs.h"
#include "us_license_t.h"
#include "us_license.h"

class US_AnalysisProfile;

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
            
   private:
      US_AnalysisProfile* mainw;        // Parent to all panels
      US_AnaProfParms*    currProf;     // Current AnaProfParms controls
      US_Help  showHelp;

      QPushButton* pb_aproname;
      QPushButton* pb_protname;

      QLineEdit*   le_aproname;
      QLineEdit*   le_protname;

      QStringList  sl_chnsel;

      QList< QLabel* >       lb_chns;
      QList< QLabel* >       lb_lcrats;
      QList< QLabel* >       lb_lctols;
      QList< QLineEdit* >    le_lcrats;
      QList< QLineEdit* >    le_lctols;
      QList< QPushButton* >  pb_aplya;

      int          dbg_level;
      bool         use_db;              // Using the LIMS database?


   private slots:
 signals:
      void  set_tabs_buttons_inactive ( void );
      void  set_tabs_buttons_active_readonly   ( void );
      void  set_tabs_buttons_active  ( void );
};

//! \brief AnalysisProf Edit panel
class US_AnaprofPanEdit : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_AnaprofPanEdit( QWidget* );
      ~US_AnaprofPanEdit() {};

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
         { showHelp.show_help( "manual/experiment_rotor.html" ); };

   private:
      US_AnalysisProfile*  mainw;
      US_AnaProfParms::AProfParmsEdit*   apEdit;      // Edit controls
      US_Help  showHelp;
      
      QLineEdit *  le_loadvol;
      QLineEdit *  le_voltoler;
      QLineEdit *  le_dataend;
      
      QStringList  sl_chnsel;

      QList< QLabel* >       lb_chns;
      QList< QLabel* >       lb_ldvols;
      QList< QLabel* >       lb_lvtols;
      QList< QLabel* >       lb_daends;
      QList< QLineEdit* >    le_ldvols;
      QList< QLineEdit* >    le_lvtols;
      QList< QLineEdit* >    le_daends;
      QList< QPushButton* >  pb_aplya;
      int         dbg_level;
      bool        first_time_init;
      bool        changed;

      QStringList            experimentTypes;

   private slots:
      
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
      US_AnalysisProfile*  mainw;
      US_AnaProfParms::AProfParms2DSA*   ap2DSA;    // 2DSA controls
      US_Help      showHelp;

      QPushButton* pb_custmg; 
      QPushButton* pb_applya; 

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
      //! \brief XXXX
//      QString speedp_description( const int );
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
      US_AnalysisProfile*   mainw;
      US_AnaProfParms::AProfParmsPCSA*   apPCSA;    // PCSA controls
      US_Help  showHelp;

      QPushButton* pb_applya; 

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
      QLineEdit*   le_regparam;
      QLineEdit*   le_mciters;

      QCheckBox*   ck_tinoise;
      QCheckBox*   ck_rinoise;
      QCheckBox*   ck_tregoff;
      QCheckBox*   ck_tregspec;
      QCheckBox*   ck_tregauto;

      bool         changed;
      int          dbg_level;
      QStringList  sl_chnsel;

   private slots:
};


//! \brief Experiment Upload panel
class US_AnaprofPanUpload : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_AnaprofPanUpload( QWidget* );
      ~US_AnaprofPanUpload() {};

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
      US_AnalysisProfile*  mainw;
      US_AnaProfParms*     loadProf;   // Prof params as loaded from AP record
      US_AnaProfParms*     currProf;   // Current AnaProfParms controls
      US_AnaProfParms::AProfParmsEdit*   apEdit;    // Edit controls
      US_AnaProfParms::AProfParms2DSA*   ap2DSA;    // 2DSA controls
      US_AnaProfParms::AProfParmsPCSA*   apPCSA;    // PCSA controls
      US_AnaProfParms::AProfParmsUpload* apSubmt;   // Upload controls
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

//! \brief Analysis Profile Main Window
class US_AnalysisProfile : public US_Widgets
{
   Q_OBJECT

   public:
      US_AnalysisProfile();

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
      // \brief Add a even-12-column spacing row
      void        addColumnSpacing( QGridLayout* genL, int& row );

      US_AnaProfParms  loadProf;   // Prof params as loaded from AP record
      US_AnaProfParms  currProf;   // Current AnaProfParms controls

      QPushButton* pb_next;
      QPushButton* pb_prev;
      QPushButton* pb_close;
      QPushButton* pb_apply;

      bool    connection_status;
      QString xpnhost;
      int     xpnport;
      
      bool    automode;
      void    auto_mode_passed( void ); 
      
   private:

      QTabWidget*           tabWidget;      // Tab Widget holding the panels

      US_AnaprofPanGen*     apanGeneral;    // General panel
      US_AnaprofPanEdit*    apanEdit;       // Edit panel
      US_AnaprofPan2DSA*    apan2DSA;       // 2DSA panel
      US_AnaprofPanPCSA*    apanPCSA;       // PCSA panel
      US_AnaprofPanUpload*  apanUpload;     // Upload panel

      int         statflag;        // Composite panels status flag
      int         dbg_level;       // Debug print flag
      int         curr_panx;       // Current panel index (0-7)

      

   private slots:

      void reset     ( void );
      void newPanel  ( int  );     // Move to a new panel
      void statUpdate( void );     // Get a status flag update
      void panelUp   ( void );     // Move to next panel
      void panelDown ( void );     // Move to previous panel
      void help      ( void );     // Show documentation window

      void disable_tabs_buttons( void);  // Slot to unable Tabs and Buttons when user level is low
      void enable_tabs_buttons_readonly( void);  // Slot to enable Tabs and Buttons after profile is loaded
      void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after run_name is entered
      
    public slots:
      void close_program( void );
      //void auto_mode_passed( void ); 

    signals:
      void us_exp_is_closed( void );
      
      
};
#endif

