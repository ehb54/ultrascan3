#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include <QApplication>

#include "us_run_protocol.h"
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

class US_ExperimentMain;

//! \brief Experiment General panel
class US_ExperGuiGeneral : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiGeneral( QWidget* );
      ~US_ExperGuiGeneral() {};

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
         { showHelp.show_help( "manual/experiment_general.html" ); };

      // Detailed information for specific centerpiece
      bool        centpInfo   ( const QString, US_AbstractCenterpiece& );
      // All protocols: names and summary information
      int         getProtos   ( QStringList&, QList< QStringList >& );
      // Append to the names,summary-data protocol lists
      bool        updateProtos( const QStringList );

   private:
      US_ExperimentMain*    mainw;      // Parent to all panels
      US_RunProtocol*       currProto;  // Current RunProtocol controls pointer
      US_Help  showHelp;

      QLineEdit*   le_runid;            // Run name line edit
      QLineEdit*   le_protocol;         // Protocol name line edit
      QLineEdit*   le_project;          // Project name line edit
      QLineEdit*   le_investigator;     // Investigator line edit

      QwtCounter*  ct_tempera;          // Temperature counter

      int          dbg_level;
      bool         use_db;              // Using the LIMS database?

      QList< QStringList >  protdata;   // List of all protocol data strings
      QStringList           cp_names;   // List of Centerpiece names
      QStringList           pr_names;   // List of protocol names

      QList< US_AbstractCenterpiece >  acp_list; // Full Centerpiece information

   private slots:
      void sel_project     ( void );        // Slot for project button clicked
      void project_info    ( US_Project& ); // Slot for project diag results
      void sel_investigator( void );        // Slot for investigator changed
      void run_name_entered( void );        // Slot for run name entered
      void load_protocol   ( void );        // Slot for protocol loaded
      void changed_protocol( void );        // Slot for change in protocol name
      void centerpieceInfo ( void );        // Function for all centerpieces

};

//! \brief Experiment Rotor panel
class US_ExperGuiRotor : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiRotor( QWidget* );
      ~US_ExperGuiRotor() {};

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
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoRotor*  rpRotor;        // Rotor protocol
      US_Help  showHelp;
      QComboBox* cb_lab;                              // Lab combo box
      QComboBox* cb_rotor;                            // Rotor combo box
      QComboBox* cb_calibr;                           // Calibration combo box

      QVector< US_Rotor::Lab >               labs;    // All labs
      QVector< US_Rotor::Rotor >             rotors;  // All rotors in lab
      QVector< US_Rotor::AbstractRotor >     arotors; // All abstract rotors
      QVector< US_Rotor::RotorCalibration >  calibs;  // Calibrations of rotor

      QStringList sl_labs;         // Lab combo choices
      QStringList sl_rotors;       // Rotor combo choices
      QStringList sl_arotors;      // Abstract rotor combo choices
      QStringList sl_calibs;       // Calibration combo choices
      int         dbg_level;
      int         nholes;          // Number of holes for current rotor
      bool        changed;         // Has rotor protocol changed?

   private slots:
      void changeLab  ( int );     // Slot for change in lab
      void changeRotor( int );     // Slot for change in rotor
      void changeCalib( int );     // Slot for change in calibration
      void advRotor   ( void );    // Function for advanced rotor dialog
      void advRotorChanged( US_Rotor::Rotor&,
                            US_Rotor::RotorCalibration& ); // Rotor diag change
};

//! \brief Experiment Speeds panel
class US_ExperGuiSpeeds : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiSpeeds( QWidget* );
      ~US_ExperGuiSpeeds() {};

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
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoSpeed* rpSpeed;
      US_Help      showHelp;

      QComboBox*   cb_prof;      // Choice: current speed step
      QwtCounter*  ct_count;     // Counter: number of speed steps
      QwtCounter*  ct_speed;     // Counter: step's rotor speed
      QwtCounter*  ct_accel;     // Counter: step's acceleration
      QwtCounter*  ct_durhr;     // Counter: step's duration in hours
      QwtCounter*  ct_durmin;    // Counter: step's duration in minutes
      QwtCounter*  ct_dlyhr;     // Counter: step's delay in hours
      QwtCounter*  ct_dlymin;    // Counter: step's delay in minutes
      QwtCounter*  ct_dlysec;    // Counter: step's delay in seconds

      QVector< QString >  profdesc;              // Speed profile description
      QVector< QMap< QString, double> >  ssvals; // Speed-step values

      int          dbg_level;     // Debug flag
      int          nspeed;        // Number of speed steps
      int          curssx;        // Current speed step index
      bool         changed;       // Flag if any speed step changes

   private slots:
      //! \brief Compose a speed step description
      QString speedp_description( const int );
      //! \brief Slot for SS change in number of steps
      void    ssChangeCount ( double );
      //! \brief Slot for SS change in profile index
      void    ssChangeProfx ( int    );
      //! \brief Slot for SS change in speed
      void    ssChangeSpeed ( double );
      //! \brief Slot for SS change in acceleration
      void    ssChangeAccel ( double );
      //! \brief Slot for SS change in duration-hours
      void    ssChangeDurhr ( double );
      //! \brief Slot for SS change in duration-minutes
      void    ssChangeDurmin( double );
      //! \brief Slot for SS change in delay hours
      void    ssChangeDlyhr ( double );
      //! \brief Slot for SS change in delay minutes
      void    ssChangeDlymin( double );
      //! \brief Slot for SS change in delay seconds
      void    ssChangeDlysec( double );
      //! \brief Function to adjust delay based on speed,accel,delay-hrs
      void    adjustDelay   ( void   );
};

//! \brief Experiment Cells panel
class US_ExperGuiCells : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiCells( QWidget* );
      ~US_ExperGuiCells() {};

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
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoCells* rpCells;
      US_Help  showHelp;
      QList< QLabel* >     cc_labls;   // Cell label object pointers
      QList< QComboBox* >  cc_cenps;   // Centerpiece object pointers
      QList< QComboBox* >  cc_winds;   // Windows object pointers
      QStringList          cpnames;    // Centerpiece names
      QStringList          tcb_centps; // CPs paired w/ Titanium Counterbalance
      int          dbg_level;
      int          ncells;             // Number of cell rows
      int          nused;              // Number of cell centerpieces given

   private slots:
      void centerpieceChanged( int );  // Centerpiece choice selected
      void windowsChanged    ( int );  // Windows choice selected
      void rebuild_Cells     ( void ); // Rebuild run protocol for cells
};

//! \brief Experiment Solutions panel
class US_ExperGuiSolutions : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiSolutions( QWidget* );
      ~US_ExperGuiSolutions() {};

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
         { showHelp.show_help( "manual/experiment_solutions.html" ); };

   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoSolutions*  rpSolut;
      US_Help  showHelp;
      int      dbg_level;
      int      mxrow;
      int      nchant;
      int      nchanu;
      int      nchanf;

      QVector< QLabel* >       cc_labls;        // Channel label pointers
      QVector< QComboBox* >    cc_solus;        // Solution choice pointers
      QVector< QPushButton* >  cc_comms;        // Comment pushbutton pointers
      QStringList              sonames;         // Solution names

      QMap< QString, QString >      solu_ids;   // Map solution IDs
      QMap< QString, US_Solution >  solu_data;  // Map solution objects
      QMap< QString, QString >      pro_comms;  // Map protocol channel comments
      QMap< QString, QString >      run_comms;  // Map run channel comments

   private slots:
      // Open a solution dialog and return selected solution
      void manageSolutions( void );
      // Detailed solution information dialog
      void detailSolutions( void );
      // Get the solution ID/GUID for a given name
      bool solutionID     ( const QString, QString& );
      // Get the solution object for a given name
      bool solutionData   ( const QString, US_Solution& );
      // Get all solution names and associated IDs
      int  allSolutions   ( void );
      // Dialog to add run comments for a channel
      void addComments    ( void );
      // Compose a comment string for a solution and a list of comment parts
      void commentStrings ( const QString, QString&,
                            QStringList& );
      // Rebuild the solution part of the current run protocol
      void rebuild_Solut     ( void );
};


//! \brief Experiment Optical Systems panel
class US_ExperGuiOptical : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiOptical( QWidget* );
      ~US_ExperGuiOptical() {};

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
         { showHelp.show_help( "manual/experiment_photomult.html" ); };

   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoOptics*  rpOptic;
      QList< QLabel* >        cc_labls;  // Cell label GUI objects
      QList< QButtonGroup* >  cc_osyss;  // Ck-boxes OptSys GUI objects

      US_Help  showHelp;
      int      dbg_level;
      int      mxrow;                    // Max optics rows
      int      nochan;                   // Number optics rows
      int      nuchan;                   // Number used rows

   private slots:
      // Handle a (un)check of the optical systems to use for a channel
      void opsysChecked      ( bool );
      // Rebuild the Optical System part of the current run protocol
      void rebuild_Optic     ( void );
};

//! \brief Experiment Spectra panel
class US_ExperGuiSpectra : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiSpectra( QWidget* );
      ~US_ExperGuiSpectra() {};

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
         { showHelp.show_help( "manual/experiment_photomult.html" ); };

   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoSpectra*  rpSpect;
      US_Help  showHelp;
      QList< QLabel* >         cc_labls;   // Pointers to channel labels
      QList< QPushButton* >    cc_wavls;   // Pointers to wavelength buttons
      QList< QCheckBox* >      cc_optis;   // Pointers to Auto-Optima checkboxes
      QList< QPushButton* >    cc_loads;   // Pointers to Load Spect. buttons
      QList< QPushButton* >    cc_manus;   // Pointers to Manual Spect. buttons
      QList< QCheckBox* >      cc_dones;   // Pointers to Done checkboxes

      int          dbg_level;              // Debug level
      int          mxrow;                  // Maximum possible rows (24)
      int          nspchan;                // Number Spectra channels (rows)

      QStringList  schans;                 // Selected Spectra channels, ea. row
      QStringList  stypes;                 // Selected Spectra types, ea. row
      QList< QVector< double > > slambds;  // Selected wavelenths, ea. channel
      QList< QVector< double > > plambds;  // Profile wavelenths, ea. channel
      QList< QVector< double > > pvalues;  // Profile values, ea. channel

   private slots:
      // Manage extinction profiles in a dialog
      void manageEProfiles  ( void );
      // Process the results of the extinction dialog
      void process_results  ( QMap< double, double >& );
      // Display details on current Spectra parameter values
      void detailSpectra    ( void );
      // Select the wavelengths to scan for a channel
      void selectWavelengths( void );
      // Handle (un)check of Auto in Optima box
      void checkOptima      ( bool );
      // Load an extinction spectrum
      void loadSpectrum     ( void );
      // Manually enter a lambda/value spectrum 
      void manualSpectrum   ( void );
      // Rebuild the Spectra part of the current run protocol
      void rebuild_Spect    ( void );
};

//! \brief Experiment Upload panel
class US_ExperGuiUpload : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperGuiUpload( QWidget* );
      ~US_ExperGuiUpload() {};

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
         { showHelp.show_help( "manual/experiment_upload.html" ); };

   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol*       loadProto;  // Loaded RunProtocol controls pointer
      US_RunProtocol*       currProto;  // Current RunProtocol controls pointer
      US_RunProtocol::RunProtoRotor*      rpRotor;  //!< Rotor controls
      US_RunProtocol::RunProtoSpeed*      rpSpeed;  //!< Speed controls
      US_RunProtocol::RunProtoCells*      rpCells;  //!< Cells controls
      US_RunProtocol::RunProtoSolutions*  rpSolut;  //!< Solutions controls
      US_RunProtocol::RunProtoOptics*     rpOptic;  //!< Optical Systems controls
      US_RunProtocol::RunProtoSpectra*    rpSpect;  //!< Spectra controls
      US_RunProtocol::RunProtoUpload*     rpUload;  //!< Upload controls
      US_Help  showHelp;

      QPushButton* pb_saverp;
      QPushButton* pb_upload;

      QCheckBox*   ck_run;
      QCheckBox*   ck_project;
      QCheckBox*   ck_rotor;
      QCheckBox*   ck_rotor_ok;
      QCheckBox*   ck_speed;
      QCheckBox*   ck_speed_ok;
      QCheckBox*   ck_centerp;
      QCheckBox*   ck_solution;
      QCheckBox*   ck_optical;
      QCheckBox*   ck_spectra;
      QCheckBox*   ck_connect;
      QCheckBox*   ck_rp_diff;
      QCheckBox*   ck_prot_ena;
      QCheckBox*   ck_prot_svd;
      QCheckBox*   ck_upl_enab;
      QCheckBox*   ck_upl_done;

      int          dbg_level;
      bool         have_run;    // Have Run specified
      bool         have_proj;   // Have Project specified
      bool         have_rotor;  // Have Rotor parameters specified
      bool         chgd_rotor;  // User Changed Rotor parameters
      bool         have_speed;  // Have Speed parameters specified
      bool         chgd_speed;  // User Changed Speed parameters
      bool         have_cells;  // Have Cell parameters specified
      bool         have_solus;  // Have Solutions parameters specified
      bool         have_optic;  // Have Optical parameters specified
      bool         have_spect;  // Have Spectra parameters specified
      bool         have_sol;    // Have Solution parameters specified
      bool         rps_differ;  // Run Protocols differ loaded/current
      bool         proto_ena;   // Protocol save is possible now
      bool         proto_svd;   // Protocol have been Saved
      bool         upld_enab;   // Upload of Run controls is Enabled
      bool         uploaded;    // Run controls have been Uploaded
      bool         connected;   // We are Connected to the Optima
      QString      json_upl;    // JSON to upload

   private slots:
      void    detailExperiment( void );  // Dialog to detail experiment
      void    testConnection  ( void );  // Test Optima connection
      void    uploadExperiment( void );  // Upload the experiment
      void    saveRunProtocol ( void );  // Save the Run Protocol
      QString buildJson       ( void );  // Build the JSON
};

//! \brief Experiment Main Window
class US_ExperimentMain : public US_Widgets 
{
   Q_OBJECT

   public:
      US_ExperimentMain();

      // Get a named child panel's value of a given type
      QString     childSValue ( const QString, const QString );
      int         childIValue ( const QString, const QString );
      double      childDValue ( const QString, const QString );
      QStringList childLValue ( const QString, const QString );
      // Initialize all the panels
      void        initPanels  ( void );
      // Get a named abstract centerpiece information object
      bool        centpInfo   ( const QString, US_AbstractCenterpiece& );
      // Get the list of protocol names and summary-data strings
      int         getProtos   ( QStringList&, QList< QStringList >& );
      // Update the list of protocols with a newly named entry
      bool        updateProtos( const QStringList );

      US_RunProtocol  loadProto;   // Controls as loaded from an RP record
      US_RunProtocol  currProto;   // Current RunProtocol controls


   private:

      QTabWidget*           tabWidget;      // Tab Widget holding the panels

      US_ExperGuiGeneral*   epanGeneral;    // General panel
      US_ExperGuiRotor*     epanRotor;      // Lab/Rotor panel
      US_ExperGuiSpeeds*    epanSpeeds;     // Speeds panel
      US_ExperGuiCells*     epanCells;      // Cells panel
      US_ExperGuiSolutions* epanSolutions;  // Solutions panel
      US_ExperGuiOptical*   epanOptical;    // Optical Systems panel
      US_ExperGuiSpectra*   epanSpectra;    // Spectra panel
      US_ExperGuiUpload*    epanUpload;     // Upload panel

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
};
#endif

