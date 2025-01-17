//! \file us_experiment/us_experiment_gui_optima.h

#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include <QApplication>
#include <unistd.h>
#include <fstream>
#include <QtSql>
#include <QSslSocket>

#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"
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
#include "us_link_ssl.h"
#include "../us_convert/us_convert.h"
#include "us_dataIO.h"
#include "us_simparms.h"
#include "us_mwl_data.h"

//#include "us_license_t.h"
//#include "us_license.h"



class US_ExperimentMain;

//! \brief Experiment General panel
class US_ExperGuiGeneral : public US_WidgetsDialog 
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
      void        setProtos   ( QStringList );
  

      void check_user_level( void );
      void update_inv( void );
      void check_runname( void );
      int loaded_proto;

      QLabel*      lb_label;
      QLineEdit*   le_label;            // Run name line edit

      void resetPanel( void );
            
   private:
      US_ExperimentMain*    mainw;      // Parent to all panels
      US_RunProtocol*       currProto;  // Current RunProtocol controls pointer
      US_Help  showHelp;

      QPushButton* pb_investigator;     // Button to select investigator
      QPushButton* pb_project;          // Button to select project

      QLineEdit*   le_runid;            // Run name line edit
      QLineEdit*   le_protocol;         // Protocol name line edit
      QLineEdit*   le_project;          // Project name line edit
      QLineEdit*   le_investigator;     // Investigator line edit

      QwtCounter*  ct_tempera;          // Temperature counter
      QwtCounter*  ct_tedelay;          // Temp-equil-delay counter

      int          dbg_level;
      bool         use_db;              // Using the LIMS database?
      bool         usr_enab;            // Overrideable user level

      QList< QStringList >  protdata;   // List of all protocol data strings
      QStringList           cp_names;   // List of Centerpiece names
      QStringList           pr_names;   // List of protocol names
      QStringList           instr_opers;  // Instrument operators

      QMap < QString, bool >  ul2_operator_for_optima; //for UL<3, determine if the user is an operator, for each machine


      QList< US_AbstractCenterpiece >  acp_list; // Full Centerpiece information

   private slots:
      void sel_project     ( void );        // Slot for project button clicked
      void project_info    ( US_Project& ); // Slot for project diag results
      void sel_investigator( void );        // Slot for investigator changed
      void run_name_entered( void );        // Slot for run name entered
      void label_name_entered( void );      // Slot for label name entered
      void load_protocol   ( void );        // Slot for protocol loaded
      void changed_protocol( void );        // Slot for change in protocol name
      void centerpieceInfo ( void );        // Function for all centerpieces
      void check_empty_runname(const QString &);
      void update_protdata( void );
      
 signals:
      void  set_tabs_buttons_inactive ( void );
      void  set_tabs_buttons_active_readonly   ( void );
      void  set_tabs_buttons_active  ( void );
      void  go_back_to_run_manager( void );
      
};

//! \brief Experiment Rotor panel
class US_ExperGuiRotor : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_ExperGuiRotor( QWidget* );
      ~US_ExperGuiRotor() {};

      void        initPanel( void );    // Standard panel utilities
      void        savePanel( void );
      void        setFirstLab( void );
      void        reset_dataSource_public( void );
      void        get_chann_ranges_public( QString, QMap <QString, QStringList>& );
   
           
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

      bool message_instr_shown;
      QString runID;
      QString runType;
      QMap <QString, QStringList> runTypes_map;
      QMap <QString, QStringList> channs_ranges;
      QStringList unique_runTypes;
      bool ra_data_type;
      bool ra_data_sim;
      bool isMwl;
      QMap<QString, QString> run_details;
  
      QVector< US_DataIO::RawData >      allData;      //!< All loaded data
      QVector< US_DataIO::RawData* >     outData;      //!< Output data pointers
      QList< US_Convert::TripleInfo >    all_tripinfo; //!< all triple info
      QList< US_Convert::TripleInfo >    out_tripinfo; //!< output triple info
      QList< US_Convert::TripleInfo >    all_chaninfo; //!< all channel info
      QList< US_Convert::TripleInfo >    out_chaninfo; //!< output channel info
      QStringList                        all_triples;  //!< all triple strings
      QStringList                        all_channels; //!< all channel strings
      QStringList                        out_triples;  //!< out triple strings
      QStringList                        out_channels; //!< out channel strings
      QList< int >                       out_chandatx; //!< chn.start data index
      US_MwlData    mwl_data;                  //!< MWL data object
      QVector< SP_SPEEDPROFILE >         speedsteps;   //!< Speed steps
  
   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoRotor*      rpRotor;        // Rotor protocol
      US_RunProtocol::RunProtoSpeed*      rpSpeed;  
      US_RunProtocol::RunProtoCells*      rpCells;
      US_RunProtocol::RunProtoSolutions*  rpSolut;
      US_RunProtocol::RunProtoOptics*     rpOptic;  
      US_RunProtocol::RunProtoRanges*     rpRange;
      US_RunProtocol::RunProtoAProfile*   rpAprof;
  
      US_Help  showHelp;
      QComboBox* cb_lab;                              // Lab combo box
      QComboBox* cb_rotor;                            // Rotor combo box
      QComboBox* cb_calibr;                           // Calibration combo box
      QComboBox* cb_operator;                         // Operator combo box
      QLabel*    lb_operator;
      QComboBox* cb_exptype;                          // Exp. Type combo box
      QComboBox*   cb_optima;
      QStringList  sl_optimas;
  QCheckBox* ck_disksource;
  QCheckBox* ck_absorbance_t;
  QPushButton* pb_importDisk;
  QLineEdit *  le_dataDiskPath;
      
      QVector< US_Rotor::Lab >               labs;    // All labs
      QVector< US_Rotor::Rotor >             rotors;  // All rotors in lab
      QVector< US_Rotor::AbstractRotor >     arotors; // All abstract rotors
      QVector< US_Rotor::RotorCalibration >  calibs;  // Calibrations of rotor
      QList  < US_Rotor::Instrument >       instruments;

      US_Rotor::Instrument     currentInstrument; 
	
      QStringList sl_labs;         // Lab combo choices
      QStringList sl_rotors;       // Rotor combo choices
      QStringList sl_arotors;      // Abstract rotor combo choices
      QStringList sl_calibs;       // Calibration combo choices

      QStringList sl_operators;    // Operator combo choices
      QStringList sl_operators_copy;    // Operator combo choices
  QLabel*      lb_instrument;
  QLabel*      lb_optima_connected;
  //QLineEdit *  le_instrument;
      QLineEdit*   le_optima_connected;
      
      int         dbg_level;
      int         nholes;          // Number of holes for current rotor
      bool        changed;         // Has rotor protocol changed?
      bool        first_time_init;
      int         curr_rotor;
      //int         currentInstrumentID;

      QStringList  experimentTypes;
      //int          currentOperator_index;
      QString      currentOperator;

  QString importDataPath;
  
      QString expType_old;

      //Assigning oper(s) && rev(s)
      QLabel*      lb_operator_reviewer_banner;
      QLabel* lb_choose_oper;
      QLabel* lb_choose_rev;
      QLabel* lb_choose_appr;
      QLabel* lb_choose_sme;
      QLabel* lb_opers_to_assign;
      QLabel* lb_revs_to_assign;
      QLabel* lb_apprs_to_assign;
      QLabel* lb_smes_to_assign;
  
      QPushButton*  pb_add_oper;
      QPushButton*  pb_remove_oper;
      QPushButton*  pb_add_rev;
      QPushButton*  pb_remove_rev;
      QPushButton*  pb_add_appr;
      QPushButton*  pb_remove_appr;
      QPushButton*  pb_add_sme;
      QPushButton*  pb_remove_sme;
  
      QTextEdit*    te_opers_to_assign;
      QTextEdit*    te_revs_to_assign;
      QTextEdit*    te_apprs_to_assign;
      QTextEdit*    te_smes_to_assign;

      QComboBox*    cb_choose_operator;
      QComboBox*    cb_choose_rev;
      QComboBox*    cb_choose_appr;
      QComboBox*    cb_choose_sme;				  
      
   private slots:
      void changeLab  ( int );     // Slot for change in lab
      void changeRotor( int );     // Slot for change in rotor
      void changeCalib( int );     // Slot for change in calibration

      void changeOperator( int );  // Slot for change in operator
      void changeExpType( int );   // Slot for change in exp. type
      void changeOptima ( int );   // Slot for change in exp. type

  void importDisk( void );
  void importDiskChecked( bool );
  void dataDiskAbsChecked( bool );
  QMap<QString,QStringList> build_protocol_for_data_import( QMap< QString, QStringList > );
  void importDisk_cleanProto( void );
  bool init_output_data  ( void );
  void runDetails        ( void );
  bool rotorForUploadedData ( void );
      
      void advRotor   ( void );    // Function for advanced rotor dialog
      // Rotor dialog value selected and accepted return values
      void advRotorChanged( US_Rotor::Rotor&,
                            US_Rotor::RotorCalibration& );
      // Get pointer to abstractRotor for named rotor
      US_Rotor::AbstractRotor* abstractRotor( const QString );
      void test_optima_connection( void );
      void init_grevs( void );
      void init_gapprs( void );
      void init_gsmes( void );
      void addOpertoList( void );
      void removeOperfromList( void );
      void addRevtoList( void );
      void removeRevfromList( void );
      void addApprtoList( void );
      void removeApprfromList( void );
      void addSmetoList( void );
      void removeSmefromList( void );
  
};

//! \brief Experiment Speeds panel
class US_ExperGuiSpeeds : public US_WidgetsDialog 
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

      //duration
      QSpinBox*    sb_durat_dd;
      QSpinBox*    sb_durat_hh;
      QSpinBox*    sb_durat_mm;
      QSpinBox*    sb_durat_ss;

      //Uv-vis delay
      QSpinBox*    sb_delay_dd;
      QSpinBox*    sb_delay_hh;
      QSpinBox*    sb_delay_mm;
      QSpinBox*    sb_delay_ss;

      //interference delay
      QSpinBox*    sb_delay_int_dd;
      QSpinBox*    sb_delay_int_hh;
      QSpinBox*    sb_delay_int_mm;
      QSpinBox*    sb_delay_int_ss;

      //Stage delay
      QSpinBox*    sb_delay_st_dd;
      QSpinBox*    sb_delay_st_hh;
      QSpinBox*    sb_delay_st_mm;
      QSpinBox*    sb_delay_st_ss;

      //Total Time
      QLineEdit *  le_total_time;

      //Scan # estimator
      QSpinBox*    sb_wvl_per_cell;
      QLineEdit *  le_scans_per_cell;

      //Uv-vis scanint
      QSpinBox*    sb_scnint_dd;
      QSpinBox*    sb_scnint_hh;
      QSpinBox*    sb_scnint_mm;
      QSpinBox*    sb_scnint_ss;

      //interference scanint
      QSpinBox*    sb_scnint_int_dd;
      QSpinBox*    sb_scnint_int_hh;
      QSpinBox*    sb_scnint_int_mm;
      QSpinBox*    sb_scnint_int_ss;

      QCheckBox*   ck_endoff;
      QCheckBox*   ck_radcal;
      QCheckBox*   ck_sync_delay;

      QVector< QString >  profdesc;              // Speed profile description
      QVector< QMap< QString, double> >  ssvals; // Speed-step values

      int          dbg_level;     // Debug flag
      int          nspeed;        // Number of speed steps
      int          curssx;        // Current speed step index
      bool         changed;       // Flag if any speed step changes
      //Uv-vis
      QVector<int> scanint_ss_min;// Min value for ScanInt seconds Counter
      QVector<int> scanint_mm_min;// Min value for ScanInt minutes Counter
      QVector<int> scanint_hh_min;// Min value for ScanInt hours Counter
      QVector<int> delay_mm_min;  // Min value for Delay minutes Counter
      //interference
      QVector<int> scanint_ss_int_min;// Min value for ScanInt seconds Counter
      QVector<int> scanint_mm_int_min;// Min value for ScanInt minutes Counter
      QVector<int> scanint_hh_int_min;// Min value for ScanInt hours Counter
      QVector<int> delay_mm_int_min;  // Min value for Delay minutes Counter
      
   private slots:
      //! \brief Compose a speed step description
      QString speedp_description( const int );
      //! \brief Slot for SS change in number of steps
      void    ssChangeCount ( int    );
      //! \brief Slot for SS change in profile index
      void    ssChangeProfx ( int    );
      //! \brief Slot for SS change in speed
      void    ssChangeSpeed ( double );
      //! \brief Slot for SS change in acceleration
      void    ssChangeAccel ( double );
      //! \brief Slot for SS change in duration day
      void    ssChgDuratDay ( int );
      //! \brief Slot for SS change in Scan Int
      void    ssChangeScInt ( double, int );

      //Scan # estimator
      void ssChgWvlPerCell( int );
      
      //! \brief Slot for SS change in duration time
      //void    ssChgDuratTime( const QTime& );
      void    ssChgDuratTime_hh( int );
      void    ssChgDuratTime_mm( int );
      void    ssChgDuratTime_ss( int );

      //! \brief Slot for SS change in delay day
      //Uv-vis
      void    ssChgDelayDay ( int );
      void    ssChgDelayTime_hh( int );
      void    ssChgDelayTime_mm( int );
      void    ssChgDelayTime_ss( int );
      //interference
      void    ssChgDelayDay_int ( int );
      void    ssChgDelayTime_int_hh( int );
      void    ssChgDelayTime_int_mm( int );
      void    ssChgDelayTime_int_ss( int );
     

      //! \brief Slot for SS change in delay_stage time
      void    ssChgDelayStageTime_hh( int );
      void    ssChgDelayStageTime_mm( int );
      
      //! \brief Slot for change in Scan Interval time
      //Uv-vis
      void    ssChgScIntTime_hh( int );
      void    ssChgScIntTime_mm( int );
      void    ssChgScIntTime_ss( int );

      //interference
      void    ssChgScIntTime_int_hh( int );
      void    ssChgScIntTime_int_mm( int );
      void    ssChgScIntTime_int_ss( int );     
      
      //! \brief Function to adjust delay based on speed,accel,delay-hrs
      void    adjustDelay   ( void   );

      void    stageDelay_sync     ( void );
      void    syncdelayChecked    ( bool );
#if 0
      //! \brief Function to populate a day,hour,minute,second list
      void    timeToList    ( double&, QList< int >& );
      //! \brief Function to get time from a day,hour,minute,second list
      void    timeFromList  ( double&, QList< int >& );
      //! \brief Function to compose a time's "day d hour:minute:second" string
      void    timeToString  ( double&, QString& );
      //! \brief Function to get time from a "day d hour:minute:second" string
      void    timeFromString( double&, QString& );
#endif
};

//! \brief Experiment Cells panel
class US_ExperGuiCells : public US_WidgetsDialog 
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
class US_ExperGuiSolutions : public US_WidgetsDialog 
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

      QMap <int, bool> solution_comment_init;
      int      mxrow;                           // Max rows (24)
      //QVector< QComboBox* >    cc_solus;        // Solution choice pointers
      QMap<QString,QString> get_solutions_public( void );
      
   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol::RunProtoSolutions*  rpSolut;
      US_RunProtocol::RunProtoRotor*      rpRotor; 
      US_Help  showHelp;
      int      dbg_level;
      //int      mxrow;                           // Max rows (24)
      int      nchant;                          // Number total rows
      int      nchanf;                          // Number rows with solutions

      QVector< QLabel* >       cc_labls;        // Channel label pointers
      QVector< QComboBox* >    cc_solus;        // Solution choice pointers
      QVector< QPushButton* >  cc_comms;        // Comment pushbutton pointers
      QStringList              sonames;         // Solution names (all)
      QStringList              srchans;         // Solution row channels
      QStringList              suchans;         // Solution used channels
      QStringList              susolus;         // Used channel solutions

      QMap< QString, QString >      solu_ids;   // Map solution IDs
      QMap< QString, US_Solution >  solu_data;  // Map solution objects
      QMap< QString, QString >      pro_comms;  // Map protocol channel comments
      QMap< QString, QString >      run_comms;  // Map run channel comments

      QMap< QString, QString >      manual_comment;  //Map for manual comment for solution
     
            
   private slots:
      // \brief Open a solution dialog and return selected solution
      void manageSolutions( void );
      // \brief Detailed solution information dialog
      void detailSolutions( void );
      // \brief Get the solution ID/GUID for a given name
      bool solutionID     ( const QString, QString& );
      // \brief Get the solution object for a given name
      bool solutionData   ( const QString, US_Solution& );
      // \brief Get all solution names and associated IDs
      int  allSolutions   ( void );
      // \brief Dialog to add run comments for a channel
      void addComments    ( void );
      // \brief Compose comment string for a solution and list of comment parts
      void commentStrings ( const QString, QString&,
                            QStringList&, const int );
      // \brief Rebuild the solution part of the current run protocol
      void rebuild_Solut     ( void );
      void regenSolList      ( void );

      void changeSolu        ( int );
};


//! \brief Experiment Optical Systems panel
class US_ExperGuiOptical : public US_WidgetsDialog 
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
      int      nuvvis;                   // Number UV/vis rows

   private slots:
      // \brief Handle a (un)check of the optical systems to use for a channel
      void opsysChecked      ( bool );
      // \brief Rebuild the Optical System part of the current run protocol
      void rebuild_Optic     ( void );
};

//! \brief Experiment Ranges panel
class US_ExperGuiRanges : public US_WidgetsDialog 
{
   Q_OBJECT

   public:

      US_ExperGuiRanges( QWidget* );
      ~US_ExperGuiRanges() {};

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
      US_RunProtocol::RunProtoRanges*  rpRange;
      US_RunProtocol::RunProtoSpeed*   rpSpeed;  //!< Speed controls
      US_RunProtocol::RunProtoSolutions*  rpSolut;  //!< Solutions controls
  
      US_Help  showHelp;
      QList< QLabel* >         cc_labls;   // Pointers to channel labels
      QList< QPushButton* >    cc_wavls;   // Pointers to wavelength buttons
      QList< QLabel* >         cc_lrngs;   // Pointers to wl range labels
      QList< QwtCounter* >     cc_lrads;   // Pointers to Radial Low counters
      QList< QwtCounter* >     cc_hrads;   // Pointers to Radial High counters
      QList< QLabel* >         cc_lbtos;   // Pointers to "to" labels
      QList< QWidget* >        cc_buff_sp;
      QList< QCheckBox* >      cc_buff_sp_ck;

      int          dbg_level;              // Debug level
      int          mxrow;                  // Maximum possible rows (24)
      int          nrnchan;                // Number Ranges channels (rows)
      int          chrow;                  // Channel row currently modified
      QString      protoname;              // Protocol used by Ranges

      QVector< QString >         rchans;   // Selected Range channel, ea. row
      QVector< QList< double > > swvlens;  // Selected wavelengths, ea. channel
      QVector< double >          locrads;  // Low radius value, ea. channel
      QVector< double >          hicrads;  // High radius value, ea. channel
      QVector< bool >            abde_buff;
      QVector< bool >            abde_mwl_deconv;

      QComboBox * cb_scancount;
      QComboBox * cb_scancount_int;
      QLineEdit * le_scanint;
      QLineEdit * le_scanint_int;

      QGridLayout* genL;			
   private slots:
      // \brief Manage extinction profiles in a dialog
//      void manageEProfiles  ( void );
      // \brief Process the results of the extinction dialog
//      void process_results  ( QMap< double, double >& );
      // \brief Display details on current Ranges parameter values
      void detailRanges     ( void );
      // \brief Select the wavelengths to scan for a channel
      void selectWavelengths( void );
  bool    iStwoOrMoreAnalytesSpectra_forChannel( QString, QStringList&, QString, int );
      bool    validExtinctionProfile( QString, QList< double >,
				      QList< double >, QStringList& );
  
      void selectWavelengths_manual( void );
      void Wavelengths_class( void );
      // \brief Handle (un)check of Auto in Optima box
//      void checkOptima      ( bool );
      // \brief Load an extinction spectrum
//      void loadSpectrum     ( void );
      // \brief Manually enter a wavelength/value spectrum 
//      void manualSpectrum   ( void );
      // \brief Handle a change in the low radius value
      void changedLowRadius ( double );
      // \brief Handle a change in the high radius value
      void changedHighRadius( double );
      // \brief Rebuild the Ranges part of the current run protocol
      void buffer_spectrum_checked( bool );
  
      void rebuild_Ranges   ( void );
};


// Dialog class for selecting wavelengths
class US_SelectWavelengths : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_SelectWavelengths( QStringList&, QStringList& );

   private:
      QStringList&   orig_wavls;
      QStringList&   select_wavls;

      QLineEdit*     le_original;
      QLineEdit*     le_selected;

      QListWidget*   lw_original;
      QListWidget*   lw_selected;

      QPushButton*   pb_add;
      QPushButton*   pb_remove;
      QPushButton*   pb_addall;
      QPushButton*   pb_rmvall;
      QPushButton*   pb_accept;

      QwtCounter*    ct_strwln;
      QwtCounter*    ct_endwln;
      QwtCounter*    ct_incwln;

      int            dbg_level;
      int            nbr_poten;
      int            nbr_selec;
      int            nbr_range;

      QStringList    original;
      QStringList    potential;
      QStringList    selected;
      
      US_Help        showHelp;

   private slots:
      void add_selections ( void );
      void rmv_selections ( void );
      void add_all_selects( void );
      void rmv_all_selects( void );
      void new_wl_start   ( double );
      void new_wl_end     ( double );
      void new_wl_incr    ( double );
      void new_wl_range   ( const int, const int, const int );
      void report         ( void );
      void cancel         ( void );
      void done           ( void );
      void reset          ( void );
      void help           ( void )
      { showHelp.show_help( "manual/convert-seltrip.html" ); };
};


// Dialog class for selecting wavelengths MANUALLY
class US_SelectWavelengths_manual : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_SelectWavelengths_manual( QStringList&, QStringList& );

   private:
      QStringList&   orig_wavls;
      QStringList&   select_wavls;

      QTextEdit*     le_info;
      QLineEdit*     le_wrange;
      QLineEdit*     le_original;
      QLineEdit*     le_selected;

      QListWidget*   lw_original;
      QListWidget*   lw_selected;

      QPushButton*   pb_add;
      QPushButton*   pb_remove;
      QPushButton*   pb_addall;
      QPushButton*   pb_rmvall;
      QPushButton*   pb_accept;

      QwtCounter*    ct_strwln;
      QwtCounter*    ct_endwln;
      QwtCounter*    ct_incwln;

      int            dbg_level;
      int            nbr_poten;
      int            nbr_selec;
      int            nbr_range;

      QStringList    original;
      QStringList    potential;
      QStringList    selected;
      
      US_Help        showHelp;

   private slots:
      bool wln_entered    ( void );
      void wln_changed    ( QString );
      bool text_to_numbers( void );
      void done           ( void ); 
      void reset          ( void );
      void cancel         ( void );
      void help           ( void ) 
      { showHelp.show_help( "manual/convert-seltrip.html" ); }; 
};


//! \brief Experiment Upload panel
class US_ExperGuiUpload : public US_WidgetsDialog 
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
         { showHelp.show_help( "manual/experiment_submit.html" ); };

      QPushButton* pb_saverp;
      QPushButton* pb_connect;      
      QPushButton* pb_submit;
      QPushButton* pb_details;

      QGridLayout* genL;

   private:
      US_ExperimentMain*   mainw;
      US_RunProtocol*       loadProto;  // Loaded RunProtocol controls pointer
      US_RunProtocol*       currProto;  // Current RunProtocol controls pointer
      US_RunProtocol::RunProtoRotor*      rpRotor;  //!< Rotor controls
      US_RunProtocol::RunProtoSpeed*      rpSpeed;  //!< Speed controls
      US_RunProtocol::RunProtoCells*      rpCells;  //!< Cells controls
      US_RunProtocol::RunProtoSolutions*  rpSolut;  //!< Solutions controls
      US_RunProtocol::RunProtoOptics*     rpOptic;  //!< Optical Systems controls
      US_RunProtocol::RunProtoRanges*     rpRange;  //!< Ranges controls
      US_RunProtocol::RunProtoAProfile*   rpAprof;  //!< Analysis Profile controls
      US_RunProtocol::RunProtoUpload*     rpSubmt;  //!< Submit controls
      US_Help  showHelp;

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
      bool         have_rotor;  // Have Rotor parameters specified
      bool         chgd_rotor;  // User Changed Rotor parameters
      bool         have_speed;  // Have Speed parameters specified
      bool         chgd_speed;  // User Changed Speed parameters
      bool         have_cells;  // Have Cell parameters specified
      bool         have_solus;  // Have Solutions parameters specified
      bool         have_optic;  // Have Optics parameters specified
      bool         have_range;  // Have Ranges parameters specified
      bool         have_sol;    // Have Solution parameters specified
      bool         rps_differ;  // Run Protocols differ loaded/current
      bool         proto_ena;   // Protocol save is possible now
      bool         proto_svd;   // Protocol have been Saved
      bool         subm_enab;   // Submit of Run controls is Enabled
      bool         submitted;   // Run controls have been Submitted
      bool         connected;   // We are Connected to the Optima
      QString      json_upl;    // JSON to upload

      QMap<QString,QString> gmp_submitter_map;

      QJsonObject absorbanceObject;

      QSqlDatabase dbxpn;

      QList< QMap<QString, QString> > all_instruments;

   private slots:
      void    detailExperiment( void );  // Dialog to detail experiment
      void    testConnection  ( void );  // Test Optima connection
      void    submitExperiment_confirm( void );  // Submit the experiment
      void    submitExperiment_confirm_protDev( void );  // Submit the experiment when US_ProtDev
  void    submitExperiment_confirm_dataDisk( void );
      void    clearData_protDev( void );
 
      void    read_optima_machines( US_DB2* = 0 );
      void    submitExperiment( void );  // Submit the experiment
      void    submitExperiment_protDev( void );  // Submit the experiment when US_ProtDev
      void    submitExperiment_dataDisk( void );  
      bool    saveRunProtocol ( void );  // Save the Run Protocol
      bool    readAProfileBasicParms( QXmlStreamReader&, QMap<QString, QString>& );

      void    saveReports ( US_AnaProfile* );  // Save the Reports
      int     writeReportToDB( QString, US_ReportGMP ); //Write ReportItems && Parent Report
      int     writeReportItemToDB( US_DB2*, QString, int, US_ReportGMP::ReportItem ); 

      void    saveAnalysisProfile ( void );  // Save the Analysis Profile
  
      bool    areReportMapsDifferent( US_AnaProfile, US_AnaProfile );
      bool    protocolToDataDisk( QStringList& );
      bool    samplesReferencesWvlsMatch( QStringList& );
      bool    matchRefSampleWvls( QString, QString, QStringList&);
      bool    useReferenceNumbersSet( QStringList& );
      bool    extinctionProfilesExist( QStringList& );
      bool    validExtinctionProfile( QString, QList< double >,
				      QList< double >, QStringList& );

      QString buildJson       ( void );  // Build the JSON
      void    add_autoflow_record( QMap< QString, QString> &protocol_details );
      void    add_autoflow_record_protDev( QMap< QString, QString> &protocol_details );
      void    add_autoflow_record_dataDisk( QMap< QString, QString> &protocol_details );

      void    do_accept_reviewers( QMap< QString, QString >& );
      void    cancel_reviewers( QMap< QString, QString >& );
  
   signals:
      void expdef_submitted    ( QMap < QString, QString > &protocol_details );
      void expdef_submitted_dev( QMap < QString, QString > &protocol_details );
      void expdef_submitted_dataDisk ( QMap < QString, QString > &protocol_details );
      
};

//! \brief Experiment AnalysisProfile panel
class US_ExperGuiAProfile : public US_WidgetsDialog 
{
   Q_OBJECT

   public:
      US_ExperGuiAProfile( QWidget* );
      ~US_ExperGuiAProfile() {};

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

      QGridLayout* genL;

      void reset_sdiag( void );
      
      US_AnalysisProfileGui* sdiag;      // Analysis Profile in panel
      
   private:
      US_ExperimentMain*    mainw;
      US_RunProtocol*       loadProto;  // Loaded RunProtocol controls pointer
      US_RunProtocol*       currProto;  // Current RunProtocol controls pointer
      US_RunProtocol::RunProtoRotor*      rpRotor;  //!< Rotor controls
      US_RunProtocol::RunProtoSpeed*      rpSpeed;  //!< Speed controls
      US_RunProtocol::RunProtoCells*      rpCells;  //!< Cells controls
      US_RunProtocol::RunProtoSolutions*  rpSolut;  //!< Solutions controls
      US_RunProtocol::RunProtoOptics*     rpOptic;  //!< Optical Systems controls
      US_RunProtocol::RunProtoRanges*     rpRange;  //!< Ranges controls
      US_RunProtocol::RunProtoAProfile*   rpAprof;  //!< AProfile controls
      US_RunProtocol::RunProtoUpload*     rpSubmt;  //!< Submit controls
      US_Help  showHelp;

      int          dbg_level;
      bool         have_prof;   // Have Analysis Profile specified
      bool         submitted;   // Run controls have been Submitted
      bool         connected;   // We are Connected to the Optima

   protected:
      void resizeEvent(QResizeEvent *event) override;   

   private slots:
      void    detailProfile   ( void );  // Dialog to detail profile

   signals:
      void expdef_submitted( QMap < QString, QString > &protocol_details );
};

//! \brief Experiment Main Window
class US_ExperimentMain : public US_Widgets
{
   Q_OBJECT

   public:
      US_ExperimentMain();

      // \brief Get a named child panel's value of a given type
      QString     childSValue ( const QString, const QString );
      int         childIValue ( const QString, const QString );
      double      childDValue ( const QString, const QString );
      QStringList childLValue ( const QString, const QString );
      // \brief Initialize all the panels
      void        initPanels  ( void );
      // \brief Get a named abstract centerpiece information object
      bool        centpInfo   ( const QString, US_AbstractCenterpiece& );
      // \brief Get the list of protocol names and summary-data strings
      int         getProtos   ( QStringList&, QList< QStringList >& );
      // \brief Update the list of protocols with a newly named entry
      bool        updateProtos( const QStringList );

      void        setProtos   ( QStringList );

      US_RunProtocol  loadProto;   // Controls as loaded from an RP record
      US_RunProtocol  currProto;   // Current RunProtocol controls
      US_AnaProfile   currAProf;   // Current AnaProfile controls
      US_AnaProfile   loadAProf;   // Current AnaProfile controls
  
      QPushButton* pb_next;
      QPushButton* pb_prev;
      QPushButton* pb_close;

      bool solutions_change;

      int ScanCount_global;
      int ScanCount_global_int;
      int TotalWvlNum_global;
      
      bool    connection_status;
      QString xpnhost;
      int     xpnport;
      QMap< QString, QString > currentInstrument;
      QMap< QString, QString > connection_for_instrument;
      
      bool    automode;
      bool    usmode;
      bool    us_prot_dev_mode;
      bool    global_reset;
      bool    us_abde_mode;

  QMap <QString, QString> protocol_details_passed; 
      
      void    auto_mode_passed( void );
      void    us_mode_passed( void );
      void    set_abde_mode_aprofile( void );
      void    unset_abde_mode_aprofile( void );
      void    abde_sv_mode_change_reset_reports( QString  );

      QStringList instruments_in_use;
      QStringList instruments_no_permit;
  bool isOperatorAny;

      int tabHeight;
      int buttLHeight;
      
   private:

      QTabWidget*           tabWidget;      // Tab Widget holding the panels

      US_ExperGuiGeneral*   epanGeneral;    // General panel
      US_ExperGuiRotor*     epanRotor;      // Lab/Rotor panel
      US_ExperGuiSpeeds*    epanSpeeds;     // Speeds panel
      US_ExperGuiCells*     epanCells;      // Cells panel
      US_ExperGuiSolutions* epanSolutions;  // Solutions panel
      US_ExperGuiOptical*   epanOptical;    // Optics panel
      US_ExperGuiRanges*    epanRanges;     // Ranges panel
      US_ExperGuiAProfile*  epanAProfile;   // Analysis Profile panel
      US_ExperGuiUpload*    epanUpload;     // Submit panel

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
      void enable_tabs_buttons_readonly( void);  // Slot to enable Tabs and Buttons after protocol is loaded
      void enable_tabs_buttons( void);  // Slot to enable Tabs and Buttons after run_name is entered
      void set_tabs_buttons_readonly( void );
      void switch_to_run_manager( void );					
					    
      
    public slots:
      void close_program( void );
      void optima_submitted( QMap < QString, QString > &protocol_details );
      void submitted_protDev( QMap < QString, QString > & );
      void submitted_dataDisk( QMap < QString, QString > & );

      void us_exp_clear( QString &protocolName );
      //void auto_mode_passed( void ); 
      void reset     ( void );
      void    exclude_used_instruments( QStringList &);
      void accept_passed_protocol_details( QMap < QString, QString > &protocol_details );

      US_AnaProfile* get_aprofile( void );
      US_AnaProfile* get_aprofile_loaded( void );
      void set_loadAProf ( US_AnaProfile );
      QMap< QString, QString> get_all_solution_names( void );
      void initCells( void );
      void reset_dataDisk( void );
      void  get_importDisk_data( QString,  QMap< QString, QStringList>& );
  
      void back_to_pcsa( void );
	
    signals:
      void us_exp_is_closed( void );
      void to_live_update( QMap < QString, QString > &protocol_details );
      void to_import( QMap < QString, QString > &protocol_details );
      void to_editing_data( QMap < QString, QString > & );
      void exp_cleared ( void );
      void close_expsetup_msg( void );
      void back_to_initAutoflow( void );
      
      
};
#endif

