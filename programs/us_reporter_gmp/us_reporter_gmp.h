#ifndef US_REPORTER_GMP_H
#define US_REPORTER_GMP_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"
#include "../us_fematch/us_fematch.h"
#include "../us_ddist_combine/us_ddist_combine.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

class US_ReporterGMP : public US_Widgets
{
   Q_OBJECT

      public:
         US_ReporterGMP();
	 US_ReporterGMP( QString );
	 
	 QTreeWidget     * genTree;
	 QTreeWidget     * perChanTree;

	 bool auto_mode;
	 QLabel* lb_hdr1 ;
	 
	 QProgressDialog * progress_msg;
	 US_RunProtocol    currProto;

	 QString ap_xml;

	 US_DDistr_Combine*         sdiag_combplot;
	 US_AnalysisProfileGui*     sdiag; 
	 US_AnaProfile              currAProf;
	 US_AnaProfile::AnaProf2DSA cAP2;
	 US_AnaProfile::AnaProfPCSA cAPp;
	 QStringList  chndescs;       
	 QStringList  chndescs_alt;
	 QMap< QString, QMap < QString, US_ReportGMP > > ch_reports;
	 QMap< QString, QMap < QString, US_ReportGMP > > ch_reports_internal;
	 QMap< QString, QList< double > > ch_wvls;
	 // QString      reportMask;

	 //for model simulations:
	 QPointer< US_ResidPlotFem >    resplotd;
	 QPointer< US_PlotControlFem >  eplotcd;
	 US_DataIO::EditedData*      rg_editdata();
	 US_DataIO::RawData*         rg_simdata();
	 QList< int >*               rg_excllist();
	 US_Model*                   rg_model();
	 US_Noise*                   rg_ti_noise();
	 US_Noise*                   rg_ri_noise();
	 QPointer< US_ResidsBitmap > rg_resbmap();
	 QString                     rg_tripleInfo();

	 struct GenReportMaskStructure
	 {
	   QMap <QString, bool >    ShowReportParts;
	   QMap <QString, QString > ShowSolutionParts;
	   QMap <QString, QString > ShowAnalysisGenParts;
	   QMap <QString, QString > ShowAnalysis2DSAParts;
	   QMap <QString, QString > ShowAnalysisPCSAParts;
	   int has_anagen_items;
	   int has_ana2dsa_items;
	   int has_anapcsa_items;
	 };

	 GenReportMaskStructure genMask_edited;

	 struct PerChanReportMaskStructure
	 {
	   QMap < QString, bool >  ShowChannelParts;
   
	   //     triple_name     model           feature  value  [Exp. Duration, Intensity, RMSD, Integration Results, Tot. Conc.]
	   QMap < QString, QMap < QString, QMap < QString, QString > > > ShowTripleModelParts;
	   QMap < QString, QMap < QString, int > >  has_tripleModel_items;

	   //     triple_name     model           feature  value  [All types of plots] 
	   QMap < QString, QMap < QString, QMap < QString, QString > > > ShowTripleModelPlotParts;
	   QMap < QString, QMap < QString, int > >  has_tripleModelPlot_items;

	 };

	 PerChanReportMaskStructure perChanMask_edited;

	 US_Plot*     plotLayout1;  // Derived from QVBoxLayout
	 US_Plot*     plotLayout2;

	 // Widgets
	 QwtPlot*     data_plot1;
	 QwtPlot*     data_plot2;
	 	 
      private:

	 //General report Mask
	 QJsonObject json;
	 QMap<QString, QTreeWidgetItem *> topItem;
	 QMap<QString, QTreeWidgetItem *> solutionItem;
	 QMap<QString, QTreeWidgetItem *> analysisItem;
	 QMap<QString, QTreeWidgetItem *> analysisGenItem;
	 QMap<QString, QTreeWidgetItem *> analysis2DSAItem;
	 QMap<QString, QTreeWidgetItem *> analysisPCSAItem;
	 QStringList topLevelItems;
	 
	 QStringList solutionItems;
	 QStringList solutionItems_vals;
	 
	 QStringList analysisItems;
	 
	 QStringList analysisGenItems;
	 QStringList analysisGenItems_vals;
	 
	 QStringList analysis2DSAItems;
	 QStringList analysis2DSAItems_vals;
	 
	 QStringList analysisPCSAItems;
	 QStringList analysisPCSAItems_vals;
	 //End of general report mask

	 //perChan Report masks
	 QMap<QString, QTreeWidgetItem *> chanItem;
	 QMap<QString, QTreeWidgetItem *> tripleItem;
	 QMap<QString, QTreeWidgetItem *> tripleModelItem;
	 QMap<QString, QTreeWidgetItem *> tripleMaskItem;
	 QMap<QString, QTreeWidgetItem *> tripleMaskPlotItem; 
	 	 
	 QList< QStringList >  autoflowdata;
	 US_SelectItem* pdiag_autoflow;

	 QString html_assembled;
	 QString html_general;
	 QString html_lab_rotor;
	 QString html_operator;
	 QString html_speed;
	 QString html_cells;
	 QString html_solutions;
	 QString html_optical;
	 QString html_ranges;
	 QString html_scan_count;
	 QString html_analysis_profile;
	 QString html_analysis_profile_2dsa;
	 QString html_analysis_profile_pcsa; 
	 
	 US_Help       showHelp;

	 QPushButton*  pb_download_report;
	 QPushButton*  pb_gen_report  ;
	 QPushButton*  pb_view_report ;
	 QPushButton*  pb_view_report_auto ;
	 QPushButton*  pb_select_all ;
	 QPushButton*  pb_unselect_all ;
	 QPushButton*  pb_expand_all ;
	 QPushButton*  pb_collapse_all ;
	 QPushButton*  pb_help;
	 QPushButton*  pb_close;
	 QLineEdit*    le_loaded_run;

	 QString    AProfileGUID;
	 QString    ProtocolName_auto;
	 int        invID;
	 QString    runID;
	 QString    filePath;
	 QString    FileName;

	 QString    current_date;
	 
	 QString    duration_str;
	 QString    delay_stage_str;
	 QString    total_time_str;

	 QString    delay_uvvis_str;
	 QString    scanint_uvvis_str;
	 QString    delay_int_str;
	 QString    scanint_int_str;

	 int        ncells_used;
	 int        nsol_channels;
	 int        nchan_optics;
	 int        nchan_ranges;

	 bool       has_uvvis;
	 bool       has_interference;
	 bool       has_fluorescense;
	 
	 QVector< QString >  Array_of_triples;
	 QMap< QString, QStringList > Triple_to_Models;
	 QMap< QString, QString > triple_info_map;
	 QString   currentTripleName;
	 
	 void  get_current_date( void );
	 void  format_needed_params( void );
	 void  assemble_pdf( void );
	 void  add_solution_details( const QString, const QString, QString& );
	 void  assemble_parts( QString& );
	 int   list_all_autoflow_records( QList< QStringList >&  );
	 QMap < QString, QString > read_autoflow_record( int );
	 void  write_pdf_report( void );

	 void read_protocol_and_reportMasks( void );
	 void parse_gen_mask_json ( const QString  );
	 void get_item_childs( QList< QTreeWidgetItem* > &, QTreeWidgetItem* );
	 void build_genTree ( void );
	 void build_perChanTree ( void ) ;
	 void gui_to_parms ( void ) ;
	 
	 void get_children_to_json( QString &, QTreeWidgetItem* );
	 QString tree_to_json ( QMap < QString, QTreeWidgetItem * > );
	 void parse_edited_gen_mask_json( const QString, GenReportMaskStructure &  );
	 void parse_edited_perChan_mask_json( const QString, PerChanReportMaskStructure &  );



	 bool model_exists;
	 
	 QVector< US_DataIO::RawData    > rawData;
	 QVector< US_DataIO::EditedData > editedData;

	 QVector< SP_SPEEDPROFILE >     speed_steps;
	 
	 US_DataIO::EditedData*      edata;
	 US_DataIO::RawData*         rdata;
	 US_DataIO::RawData*         sdata;
	 US_DataIO::RawData          wsdata;
	 
	 QPointer< US_ResidsBitmap >    rbmapd;

	 // Class to hold model descriptions
	 class ModelDesc
	 {
         public:
	   QString   description;    // Full model description
	   QString   baseDescr;      // Base analysis-set description
	   QString   fitfname;       // Associated fit file name
	   QString   modelID;        // Model DB ID
	   QString   modelGUID;      // Model GUID
	   QString   filepath;       // Full path model file name
	   QString   editID;         // Edit parent DB ID
	   QString   editGUID;       // Edit parent GUID
	   QString   antime;         // Analysis date & time (yymmddHHMM)
	   QDateTime lmtime;         // Record lastmod date & time
	   double    variance;       // Variance value
	   double    meniscus;       // Meniscus radius value
	   double    bottom;         // Bottom radius value
	   
	   // Less than operator to enable sort
	   bool operator< ( const ModelDesc& md )
	     const { return ( description < md.description ); }
	 };
	 
	 // Class to hold noise description
	 class NoiseDesc
	 {
         public:
	   QString   description;    // Full noise description
	   QString   baseDescr;      // Base analysis-set description
	   QString   noiseID;        // Noise DB ID
	   QString   noiseGUID;      // Noise GUID
	   QString   filepath;       // Full path noise file name
	   QString   modelID;        // Model parent DB ID
	   QString   modelGUID;      // Model parent GUID
	   QString   antime;         // Analysis date & time (yymmddHHMM)
	   
	   // Less than operator to enable sort
	   bool operator< ( const NoiseDesc& nd )
	     const { return ( description < nd.description ); }
	 };

	 QVector< double >    v_meni;
	 QVector< double >    v_bott;
	 QVector< double >    v_rmsd;
	 
	 QString              filedir;
	 QString              fname_load;
	 QString              fname_edit;
	 QStringList          edtfiles;
	 int                  nedtfs;
	 int                  ix_best;
	 int                  ix_setfit;
	 bool                 have3val;
	 bool                 bott_fit;
	 int                  idEdit;

	 double               fit_xvl; //for 2d data
	 double               f_meni;  //for 3d data
	 double               f_bott;  //for 3d data

	 double               dy_global;
	 double               miny_global;
	 QString tripleInfo;
	 
	 int eID_global;
	 
	 US_Model                    model;
	 US_Model                    model_loaded;
	 US_Model                    model_used;
	 
	 US_Noise                    ri_noise;
	 US_Noise                    ti_noise;
	 QList< int >                excludedScans;
	 US_Solution                 solution_rec;

	 US_Math2::SolutionData      solution;
	 QVector< QVector< double > > resids;
	 
	 US_SimulationParameters     simparams;
	 QList< US_DataIO::RawData >   tsimdats;
	 QList< US_Model >             tmodels;
	 QVector< int >                kcomps;


	 QStringList noiIDs;      // Noise GUIDs
	 QStringList noiEdIDs;    // Noise edit GUIDs
	 QStringList noiMoIDs;    // Noise model GUIDs
	 QStringList noiTypes;    // Noise types
	 QStringList modIDs;      // Model GUIDs
	 QStringList modEdIDs;    // Model edit GUIDs
	 QStringList modDescs;    // Model descriptions

	 int           thrdone;
	 
	 double       density;
	 double       viscosity;
	 double       vbar;
	 double       compress;

	 QString      svbar_global;
	 
	 bool         manual;
	 bool          dataLoaded;
	 bool          haveSim;
	 bool          dataLatest;
	 bool          buffLoaded;
	 bool          cnstvb;
	 bool          cnstff;
	 bool          exp_steps;
	 bool          dat_steps;
	 bool          is_dmga_mc;

	 QMap< QString, QString >    adv_vals;

	 int           dbg_level;
	 int           nthread;
	 int           scanCount;

	 QPoint        rpd_pos;
	 QString    FileName_parsed;

	 //main simulation results
	 QString rmsd_global;

	 void simulate_triple( const QString, QString );
	 bool loadData( QMap < QString, QString > & );
	 bool loadModel( QMap < QString, QString > & );
	 bool loadNoises( QMap < QString, QString > & );
	 void loadNoises_whenAbsent ( void );
	 int  count_noise_auto( US_DataIO::EditedData*, US_Model*,
				QStringList&, QStringList& );

	 int id_list_db_auto( QString );
	 int models_in_edit_auto( QString, QStringList& );
	 int noises_in_model_auto( QString, QStringList& );
	 void simulateModel( QMap < QString, QString > & );
	 void adjustModel( void );

	 QStringList scan_dbase_models( QStringList );
	 
	 QString text_model(     US_Model, int );
	 QString html_header   ( QString, QString, US_DataIO::EditedData* );
	 QString distrib_info( void );
	 //QString integration_info( void );
	 
	 QString get_filename( QString );
	 
      public slots:
	void    thread_progress( int, int );
	void    thread_complete( int );
	void    resplot_done( void );
	void    update_progress( int );
	
      private slots:
	void loadRun_auto( QMap < QString, QString > & );
	void check_models ( void );
	void reset_report_panel ( void );
	void view_report ( void );
	void load_gmp_run ( void );
	void generate_report( void );
	void changedItem    ( QTreeWidgetItem*, int );
	void select_all( void );
	void unselect_all( void );
	void expand_all( void );
	void collapse_all( void );

	void show_results   ( QMap < QString, QString > & );
	void calc_residuals( void );
	void assemble_distrib_html( void );
	//void assemble_integration_results_html( void );
	void assemble_plots_html( QStringList );
	double  interp_sval( double, double*, double*,  int );
	void plotres( QMap < QString, QString > &   );

	QString indent    (     int  )  const;
	QString table_row( const QString&, const QString& ) const;
	QString table_row( const QString&, const QString&,
			   const QString& )                 const;
	QString table_row( const QString&, const QString&,
			   const QString&, const QString&,
			   const QString& )                 const;
	QString table_row( const QString&, const QString&,
			   const QString&, const QString& ) const;
	QString table_row( const QString&, const QString&,
			   const QString&, const QString&,
			   const QString&, const QString&,
			   const QString& )                 const;
	
	void    write_plot    ( const QString&, const QwtPlot* );
	bool    mkdir         ( const QString&, const QString& );

	void distrib_plot_stick(  int );
	void distrib_plot_2d   (  int );
	
	void help          ( void )
	{ showHelp.show_help( "reporter_gmp.html" ); };
	
      signals:
	void reset_reports( void );
   
};
#endif
