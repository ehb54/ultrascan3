//! \file us_convert_gui.h
#ifndef US_CONVERT_GUI_H
#define US_CONVERT_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_convert.h"
#include "us_experiment.h"
//#include "us_license_t.h"
//#include "us_license.h"
#include "us_dataIO.h"
#include "us_solution.h"
#include "us_simparms.h"
#include "us_selectbox.h"
#include "us_mwl_data.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< debug-level-conditioned qDebug()
#endif

class US_ConvertGui : public US_Widgets
{
  Q_OBJECT

  public:

      //! \brief Some status codes to keep track of where data has been saved to
      enum aucStatus
      {
         NOT_SAVED,         //!< The file has not been saved
         EDITING,           //!< Data being edited; certain opers. not permitted
         HD_ONLY,           //!< The file has been saved to the HD
         BOTH               //!< The file has been saved to both HD and DB
      };

      //! \brief  Generic constructor for the US_ConvertGui() program.
      US_ConvertGui();

      // New constructor for automated read/upload/update
      US_ConvertGui(QString auto_mode);

      US_Disk_DB_Controls*    disk_controls;  //!< Radiobuttons for disk/db choice
      bool                    save_diskDB;    //!< To keep track of changes
      US_SimulationParameters simparams;      //!< Simulation parameters
      QList< double >         subsets;        //!< A list of subset boundaries

      void us_mode_passed  ( void );
      bool usmode;

  signals:
      void saving_complete_auto( QMap < QString, QString > &  );
      //void saving_complete_back_to_exp( QString & );
      void saving_complete_back_to_initAutoflow( void );
      void data_loaded( void );

      void process_next_optics( void );
      
  public slots:

  private:

      struct ChanOptics
      {
	QString     channel;
	QString     data_type;
      };
      
      struct ProtocolOptics
      {
	QVector< ChanOptics > chopts;
      };
	
      struct ChanSolu
      {
	QString     channel;       //!< Channel name ("2 / A")
	QString     solution;      //!< Solution name
	QString     sol_id;        //!< Solution Id/GUID
	QString     ch_comment;    //!< Channel protocol comment
      };
  
      struct ProtocolSolutions
      {
	QVector< ChanSolu > chsols;  //!< Channel solutions,comments
      };

      struct Cell
      {
	int         cell;          //!< Cell number
	QString     centerpiece;   //!< Centerpiece description
	QString     windows;       //!< Windows (quartz|sapphire)
	QString     cbalance;      //!< Counterbalance description
      };
  
      struct ProtocolCells
      {
	int         ncell;         //!< Number of total cells
	int         nused;         //!< Number of cells used
	QVector< Cell > cells_used;
      };
  
      struct ProtocolRotor
      {
	QString     laboratory;    //!< Laboratory with rotor
	QString     rotor;         //!< Rotor description
	QString     calibration;   //!< Rotor Calibration description
	QString     exptype;
	QString     operatorname; 
	QString     instrumentname;
	
	QString     labGUID;       //!< Laboratory GUID
	QString     rotGUID;       //!< Rotor GUID
	QString     calGUID;       //!< Rotor Calibration GUID
	QString     absGUID;       //!< Abstract Rotor GUID
	
	int         labID;         //!< Laboratory bB Id
	int         rotID;         //!< Rotor DB Id
	int         calID;         //!< Rotor Calibration DB Id
	int         absID;         //!< Abstract Rotor DB Id
	int         operID;
	int         instID;
      };
  
      struct ProtocolInfo
      {
	QString      investigator;   //!< Investigator name
	QString      runname;        //!< Run ID (name)
	QString      protname;       //!< Protocol name (description)
	QString      protGUID;       //!< Protocol GUID
	QString      project;        //!< Project description
	QString      optimahost;     //!< Optima host (numeric IP address)
	
	int          projectID; 
	double       temperature;    //!< Run temperature in degrees centigrade
	double       temeq_delay;    //!< Temperature-equilibration delay minutes

	ProtocolRotor     ProtRotor;
	ProtocolCells     ProtCells;
	ProtocolSolutions ProtSolutions;
	ProtocolOptics    ProtOptics;

      };
       
      ProtocolInfo ProtInfo;

      bool  us_convert_auto_mode;
      QMap<QString, QString> gmp_submitter_map;
      
      enum { SPLIT, REFERENCE, NONE } step;

      aucStatus      saveStatus;
      US_Help        showHelp;
      US_PlotPicker* picker;
      US_Plot*       usplot;

      bool          runType_combined_IP_RI;
      QMap < QString, int > runTypes_map;
      QString       type_to_process;
      QStringList   runTypeList;
      QString       intensityJsonRI;

      QMap < int, bool > description_changed;

      bool auto_ref_scan;
      int  autoflowStatusID;
 
      double   centerpoint_ref_def;
  bool first_time_plot_auto;
      
      QString       runType;
      QString       oldRunType;
      QString       runID;
      QString       runID_numeric;
      QString       tmst_fnamei;

      QLabel*       lb_description;

      QLineEdit*    le_investigator;
      QLineEdit*    le_status;
      QLineEdit*    le_runID;
      US_LineEdit_RE*    le_runID2;
      QLineEdit*    le_dir;
      QLineEdit*    le_description;
      QLineEdit*    le_solutionDesc;

      QLabel*       lb_triple;
      QListWidget*  lw_triple;                   // cell, channel, wavelength
      QListWidget*  lw_todoinfo;                 // to do list

      QLabel*       lb_scan;
      QLabel*       lb_from;
      QLabel*       lb_to;
      QwtCounter*   ct_from;
      QwtCounter*   ct_to;

      QwtCounter*   ct_tolerance;

      QTextEdit*    te_comment;
      
      QLabel*       lb_mwlctrl;
      QLabel*       lb_mwlctre;
      QLabel*       lb_lambstrt;
      QLabel*       lb_lambstop;
      QLabel*       lb_lambplot;
      QLineEdit*    le_lambraw;
      QComboBox*    cb_lambstrt;
      QComboBox*    cb_lambstop;
      QComboBox*    cb_lambplot;
      QCheckBox*    ck_average;
      QPushButton*  pb_lambprev;
      QPushButton*  pb_lambnext;
      QGridLayout*  lo_average;
      QVector< int >  all_lambdas;
      QVector< int >  exp_lambdas;

      QPushButton*  pb_editRuninfo;
      QPushButton*  pb_import;
      QPushButton*  pb_loadUS3;
      QPushButton*  pb_details;
      QPushButton*  pb_applyAll;
      QPushButton*  pb_solution;
      QPushButton*  pb_exclude;
      QPushButton*  pb_include;
      QPushButton*  pb_define;
      QPushButton*  pb_process;
      QPushButton*  pb_reference;
      QPushButton*  pb_intensity;
      QPushButton*  pb_cancelref;
      QPushButton*  pb_dropTrips;
      QPushButton*  pb_dropChan;
      QPushButton*  pb_dropCelch;
      QPushButton*  pb_saveUS3;
      QPushButton*  pb_showTmst;

      US_SelectBox*  cb_centerpiece;
      QLineEdit*     le_centerpieceDesc;

      QList< US_DataIO::BeckmanRawScan > legacyData;   //!< legacy data fr file
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

      QVector< US_Convert::Excludes >    allExcludes;  //!< All triple excludes

      QVector< SP_SPEEDPROFILE >         speedsteps;   //!< Speed steps

      US_MwlData    mwl_data;                  //!< MWL data object

      QwtPlot*      data_plot;
      QwtPlotGrid*  grid;

      double        reference_start;           // Boundary of reference scans
      double        reference_end;
      bool          referenceDefined;          // True if RI averages done
      int           Pseudo_reference_triple;   // Number of reference triple
      bool          isPseudo;                  // Is RI data pseudo-absorbance?
      bool          toleranceChanged;          // Has the tolerance changed?
      double        scanTolerance;             // Remember scan tolerance value
      int           countSubsets;              // Number of subsets maximum = 4
      bool          isMwl;                     // Is Multi-Wavelength?
      int           tripDatax;                 // Triple data index
      int           tripListx;                 // Triple list index
      int           nlambda;                   // Lambda (wavelength) count
      int           slambda;                   // Start Lambda on output
      int           elambda;                   // End Lambda on output
      int           nlamb_i;                   // Lambda count for raw input
      int           dbg_level;                 // Debug level
      int           impType;                   // Import type flag: 0,1,2 == Beck,MWL,AUC

      QVector<int> slambdas_per_channel;
      QVector<int> elambdas_per_channel;

      QString       currentDir;                // Current data file directory
      QString       saveDescription;           // Saved channel description

      bool          show_plot_progress;        // Flag to show plot progress
      US_Experiment ExpData;                   // Experiment data object

      void reset           ( void );
      void reset_auto      ( void );
      
      void enableRunIDControl( bool );
      void enableScanControls( void );
      void enableSaveBtn   ( void );
      void enableSaveBtn_auto   ( void );
      
      void getExpInfo      ( void );
      void readProtocol_auto ( void );
      bool readProtocolRotor_auto ( QXmlStreamReader& );
      bool readProtocolCells_auto ( QXmlStreamReader& );
      bool readProtocolSolutions_auto ( QXmlStreamReader& );
      bool readProtocolOptics_auto ( QXmlStreamReader& );
      bool isCorrectDataType( QString, QString );
  int  getProtSolIndex( QString , QString);

      void read_aprofile_data_from_aprofile( void );
      bool readAProfileBasicParms_auto ( QXmlStreamReader& );
      static bool bool_flag( const QString );
      bool isSet_to_edit_triple( QString );

      int autoflowID_passed;
      QString ProtocolName_auto;
      QString OptimaName;
      QString AProfileGUID;
      QString Exp_label;
      bool    gmpRun_bool;
      bool    protDev_bool;
      bool dataSavedOtherwise;
      QString expType;
      QString dataSource;
      QString opticsFailedType;
      
      void getExpInfo_auto ( void );
      void getLabInstrumentOperatorInfo_auto   ( void );
      void delete_autoflow_record ( void );

      void update_autoflow_record_atLimsImport( void );

  QString correct_description( QString&, QString, QString );

      void check_scans     ( void );
      void setTripleInfo   ( void );
      void checkTemperature( void );
      int  findTripleIndex ( void );
      void focus           ( int, int );
      void init_excludes   ( void );
//      void start_reference  ( const QwtDoublePoint& );
      void process_reference( const QwtDoublePoint& );
  //void process_reference_auto( const double, const double );
  void process_reference_auto( const double );
  void PseudoCalcAvg   ( void );
  void absorbance_conversion_abde( void );
  //void PseudoCalcAvg_av  ( void );
  void PseudoCalcAvgMWL( void );
      bool read            ( void );
      bool read            ( QString dir );
      bool convert         ( void );
      void initTriples     ( void );
      bool centerpieceInfo    ( void );
      bool centerpieceInfoDB  ( void );
      bool centerpieceInfoDisk( void );
      void plot_current    ( void );
      void plot_titles     ( void );
      void plot_all        ( void );
      void replot          ( void );
      void set_colors      ( const QList< int >& );
      void draw_vline      ( double );
      void db_error        ( const QString& );
      void triple_index    ( void );
      void plot_last_scans ( double );

      //US_Solution * solution_auto;

      QMap < QString, QString > details_at_editing;

      QMap < QString, bool >    channels_to_analyse;
      QMap < QString, QString > triple_to_edit;
      QMap < QString, bool >    channels_to_drop;
      QMap < QString, QString > channels_report;
      QMap < QString, QStringList >    triples_dropped_from_channel;
  QMap < QString, QMap <QString, QString> > drop_operations;
  //ABDE
  QMap < QString, int >  channels_abde_refs;
  QMap < QString, int >  channels_abde_use_refs;					   
							   

  private slots:
      //! \brief Select the current investigator
      void sel_investigator( void );

      /*! \brief Assign the selected investigator as current
          \param invID  The ID of the selected investigator
      */
      void assign_investigator( int );

      //void import_data_auto  (QString &, QString &, QString &, QString &);
      void import_data_auto  ( QMap < QString, QString > & ) ;
      QMap < QString, QString > read_autoflow_record( int );
      int  read_autoflow_stages_record( int );
      void revert_autoflow_stages_record ( int );
      bool isSaved_auto( void );

      void process_optics( void );  
      
      
      void import            ( void );
      void reimport          ( void );
      void importMWL         ( void );
      void importAUC         ( void );

      int  getImports        ( void );
      int  getImports_auto   ( QString & );
      
      void enableControls    ( void );
      void runIDChanged      ( void );
      void toleranceValueChanged( double );     // signal to notify of change
      void lambdaStartChanged( int );
      void lambdaEndChanged  ( int );
      void lambdaPlotChanged ( int );
      void lambdaPrevClicked ( void );
      void lambdaNextClicked ( void );

      void editRuninfo       ( void );
      void editRuninfo_auto  ( void );
      
      void loadUS3           ( void );
      void loadUS3Disk       ( void );
      void loadUS3Disk       ( QString );
      void loadUS3DB         ( void );
      void updateExpInfo     ( US_Experiment& );
      void cancelExpInfo     ( void );
      void getSolutionInfo   ( void );
      void updateSolutionInfo( US_Solution );
      void cancelSolutionInfo( void );
      void tripleApplyAll    ( void );
      void runDetails        ( void );
      void showTimeState     ( void );
      void changeDescription ( void );
      void changeTriple      ( void );
      void getCenterpieceIndex( int );
      void focus_from        ( double );
      void focus_to          ( double );
      void exclude_scans     ( void );
      void include           ( void );
      void define_subsets    ( void );
      void cClick            ( const QwtDoublePoint& );
      void process_subsets   ( void );
      void define_reference  ( void );

      void show_intensity    ( void );
      void show_intensity_auto ( void );

      void cancel_reference  ( void );
  //void cancel_reference_av  ( void );
      int  check_for_data_left ( QString, QString );
      void drop_reference    ( void );
      void drop_channel      ( void );
      void drop_cellchan     ( void );
      QString comment_for_drop_dialog( QString );
      void saveUS3           ( void );
      int  saveUS3Disk       ( void );
      void saveUS3DB         ( void );
      void saveReportsToDB   ( void );

      void record_import_status( bool, QString );
      
      void resetAll          ( void );
      void resetAll_auto     ( void );
      void reset_limsimport_panel( void );
      
      void source_changed    ( bool );
      void update_disk_db    ( bool );
      void show_mwl_control  ( bool );
      void mwl_connect       ( bool );
      void reset_lambdas     ( void );
      void mwl_setup         ( void );
      bool init_output_data  ( void );
      void build_output_data ( void );
      void build_lambda_ctrl ( void );
      void connectTolerance  ( bool );
      int  countSpeeds       ( QVector< int >&, int* );
      int  writeTimeStateDisk( void );
      int  writeTimeStateDB  ( void );
      void help              ( void )
        { showHelp.show_help( "convert.html" ); };
};
#endif
