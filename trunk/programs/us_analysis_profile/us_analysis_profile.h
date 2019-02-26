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

      QPushButton* pb_investigator;     // Button to select investigator
      QPushButton* pb_project;          // Button to select project

      QLineEdit*   le_runid;            // Run name line edit
      QLineEdit*   le_project;          // Project name line edit
      QLineEdit*   le_investigator;     // Investigator line edit

      QwtCounter*  ct_tempera;          // Temperature counter
      QwtCounter*  ct_tedelay;          // Temp-equil-delay counter

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
      QComboBox* cb_lab;                              // Lab combo box
      QComboBox* cb_calibr;                           // Calibration combo box
      QComboBox* cb_operator;                         // Operator combo box
      QComboBox* cb_exptype;                          // Exp. Type combo box
      QComboBox*   cb_optima;
      QStringList  sl_optimas;
      
      QLineEdit *  le_instrument;
      
      int         dbg_level;
      int         nholes;          // Number of holes for current rotor
      bool        first_time_init;
      int         curr_rotor;
      bool        changed;
      //int         currentInstrumentID;

      QStringList            experimentTypes;

   private slots:
      
};

//! \brief Experiment Speeds panel
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

      QComboBox*   cb_prof;      // Choice: current speed step
      QwtCounter*  ct_speed;     // Counter: step's rotor speed
      QwtCounter*  ct_accel;     // Counter: step's acceleration

      QLineEdit*   le_maxrpm;    // Text line: max speed for current rotor

      QTimeEdit*   tm_delay;
      QTimeEdit*   tm_durat;
      QTimeEdit*   tm_scnint;

      QSpinBox*    sb_count;
      QSpinBox*    sb_delay;
      QSpinBox*    sb_durat;
      QSpinBox*    sb_scnint;

      QSpinBox*    sb_durat_dd;
      QSpinBox*    sb_durat_hh;
      QSpinBox*    sb_durat_mm;
      QSpinBox*    sb_durat_ss;

      QSpinBox*    sb_delay_dd;
      QSpinBox*    sb_delay_hh;
      QSpinBox*    sb_delay_mm;
      QSpinBox*    sb_delay_ss;

      QSpinBox*    sb_delay_st_dd;
      QSpinBox*    sb_delay_st_hh;
      QSpinBox*    sb_delay_st_mm;
      QSpinBox*    sb_delay_st_ss;     

      QSpinBox*    sb_scnint_dd;
      QSpinBox*    sb_scnint_hh;
      QSpinBox*    sb_scnint_mm;
      QSpinBox*    sb_scnint_ss;

      QCheckBox*   ck_endoff;
      QCheckBox*   ck_radcal;
      QCheckBox*   ck_sync_delay;

      QVector< QString >  profdesc;              // Speed profile description
      QVector< QMap< QString, double> >  ssvals; // Speed-step values

      int          dbg_level;     // Debug flag
      int          nspeed;        // Number of speed steps
      int          curssx;        // Current speed step index
      bool         changed;       // Flag if any speed step changes
      QVector<int> scanint_ss_min;// Min value for ScanInt seconds Counter
      QVector<int> scanint_mm_min;// Min value for ScanInt minutes Counter
      QVector<int> scanint_hh_min;// Min value for ScanInt minutes Counter
      QVector<int> delay_mm_min;  // Min value for Delay minutes Counter
      
   private slots:
      //! \brief Compose a speed step description
      QString speedp_description( const int );

      //! \brief Function to adjust delay based on speed,accel,delay-hrs
      void    adjustDelay   ( void   );

      void    stageDelay_sync     ( void );
      void    syncdelayChecked    ( bool );
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
      QList< QLabel* >     cc_labls;   // Cell label object pointers
      QList< QComboBox* >  cc_winds;   // Windows object pointers
      QStringList          tcb_centps; // CPs paired w/ Titanium Counterbalance
      bool         changed;
      int          dbg_level;
      int          ncells;             // Number of cell rows
      int          nused;              // Number of cell centerpieces given

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
      QString      json_upl;    // JSON to upload

      QJsonObject absorbanceObject;

      QSqlDatabase dbxpn;
      

   private slots:
      void    detailExperiment( void );  // Dialog to detail experiment
      void    testConnection  ( void );  // Test Optima connection
      void    submitExperiment_confirm( void );  // Submit the experiment
      void    submitExperiment( void );  // Submit the experiment
      QString buildJson       ( void );  // Build the JSON

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

      US_AnaProfParms  loadProf;   // Prof params as loaded from AP record
      US_AnaProfParms  currProf;   // Current AnaProfParms controls

      QPushButton* pb_next;
      QPushButton* pb_prev;
      QPushButton* pb_close;

      bool solutions_change;

      bool    connection_status;
      QString xpnhost;
      int     xpnport;
      QMap< QString, QString > currentInstrument;
      
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

