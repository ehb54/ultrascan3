//! \file us_reporter_gmp.h
#ifndef US_REPORTER_GMP_H
#define US_REPORTER_GMP_H

#include <QPrinter>

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"
#include "../us_fematch/us_fematch.h"
#include "../us_ddist_combine/us_ddist_combine.h"
#include "../us_pseudo3d_combine/us_pseudo3d_combine.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_extern.h"
#include "us_select_item.h"

/**
 * @class US_ReporterGMP
 * @brief The US_ReporterGMP class provides functionality for generating GMP reports.
 */
class US_ReporterGMP : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Default constructor for US_ReporterGMP.
         */
        US_ReporterGMP();

        /**
         * @brief Constructor for US_ReporterGMP with a specified parameter.
         * @param param The specified parameter
         */
        US_ReporterGMP(QString param);

        /**
         * @brief Constructor for US_ReporterGMP for testing.
         * @param test_params The parameters for testing
         */
        US_ReporterGMP(QMap<QString, QString> test_params);

        /**
         * @brief Function to write GMP report to DB for testing.
         * @param runID The run ID
         * @param protocolID The protocol ID
         * @param invID The investigator ID
         * @param protocolName The protocol name
         * @param esign The electronic signature
         */
        void write_gmp_report_DB_test(QString runID, QString protocolID, int invID, QString protocolName, QString esign);

        QTreeWidget* genTree;               //!< General report tree
        QTreeWidget* miscTree;              //!< Miscellaneous report tree
        QTreeWidget* perChanTree;           //!< Per-channel report tree
        QTreeWidget* combPlotsTree;         //!< Combined plots tree
        bool first_time_gen_tree_build;     //!< Flag for first time general tree build
        bool first_time_misc_tree_build;    //!< Flag for first time miscellaneous tree build
        bool first_time_perChan_tree_build; //!< Flag for first time per-channel tree build

        bool auto_mode;                     //!< Flag for auto mode
        QLabel* lb_hdr1;                    //!< Header label

        QProgressDialog* progress_msg;      //!< Progress message dialog
        US_RunProtocol currProto;           //!< Current run protocol

        QString ap_xml;                     //!< XML string for analysis profile

        US_Pseudo3D_Combine* sdiag_pseudo3d; //!< Pseudo3D combine dialog
        US_DDistr_Combine* sdiag_combplot;   //!< D distribution combine dialog
        US_AnalysisProfileGui* sdiag;        //!< Analysis profile GUI dialog
        US_AnaProfile currAProf;            //!< Current analysis profile
        US_AnaProfile::AnaProf2DSA cAP2;    //!< 2DSA analysis profile
        US_AnaProfile::AnaProfPCSA cAPp;    //!< PCSA analysis profile
        QStringList chndescs;               //!< Channel descriptions
        QStringList chndescs_alt;           //!< Alternative channel descriptions
        QList<int> analysis_runs;           //!< Analysis runs  
        QList<int> report_runs;             //!< Report runs
        QMap<QString, QMap<QString, US_ReportGMP>> ch_reports; //!< Channel reports
        QMap<QString, QMap<QString, US_ReportGMP>> ch_reports_internal; //!< Internal channel reports
        QMap<QString, QList<double>> ch_wvls; //!< Channel wavelengths

        QList<int> replicates;              //!< Replicate list
        QMap<int, QStringList> replicates_to_channdesc; //!< Replicate to channel descriptions map
        QMap<QString, QStringList> channdesc_to_overlapping_wvls; //!< Channel descriptions to overlapping wavelengths map

        QPointer<US_ResidPlotFem> resplotd;    //!< Residual plot dialog
        QPointer<US_PlotControlFem> eplotcd;   //!< Plot control dialog
        US_DataIO::EditedData* rg_editdata();  //!< Edited data
        US_DataIO::RawData* rg_simdata();      //!< Simulated data
        QList<int>* rg_excllist();             //!< Exclusion list
        US_Model* rg_model();                  //!< Model
        US_Noise* rg_ti_noise();               //!< TI noise
        US_Noise* rg_ri_noise();               //!< RI noise
        QPointer<US_ResidsBitmap> rg_resbmap(); //!< Residuals bitmap
        QString rg_tripleInfo();               //!< Triple information

        struct GenReportMaskStructure
        {
            QMap<QString, bool> ShowReportParts;          //!< Map of report parts to show
            QMap<QString, QString> ShowSolutionParts;     //!< Map of solution parts to show
            QMap<QString, QString> ShowAnalysisGenParts;  //!< Map of general analysis parts to show
            QMap<QString, QString> ShowAnalysis2DSAParts; //!< Map of 2DSA analysis parts to show
            QMap<QString, QString> ShowAnalysisPCSAParts; //!< Map of PCSA analysis parts to show
            int has_anagen_items;                         //!< Flag for general analysis items
            int has_ana2dsa_items;                        //!< Flag for 2DSA analysis items
            int has_anapcsa_items;                        //!< Flag for PCSA analysis items
        };

        GenReportMaskStructure genMask_edited; //!< Edited general report mask structure

        struct PerChanReportMaskStructure
        {
            QMap<QString, bool> ShowChannelParts; //!< Map of channel parts to show

            // triple_name -> model -> feature -> value [Exp. Duration, Intensity, RMSD, Integration Results, Tot. Conc.]
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelParts; //!< Map of triple model parts to show
            QMap<QString, QMap<QString, int>> has_tripleModel_items;                   //!< Map of triple model items

            // triple_name -> model -> feature -> value [All types of plots]
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelPlotParts; //!< Map of triple model plot parts to show
            QMap<QString, QMap<QString, int>> has_tripleModelPlot_items;                   //!< Map of triple model plot items

            // triple_name -> model -> feature -> value [All types of pseudo distr.]
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelPseudo3dParts; //!< Map of triple model pseudo3d parts to show
            QMap<QString, QMap<QString, int>> has_tripleModelPseudo3d_items;                   //!< Map of triple model pseudo3d items

            // Individual Combined Plots
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleTypeModelRangeIndividualCombo; //!< Map of individual combined plots
            QMap<QString, QMap<QString, int>> has_tripleModelIndCombo_items;                               //!< Map of triple model individual combined items
        };

        PerChanReportMaskStructure perChanMask_edited; //!< Edited per-channel report mask structure

        struct CombPlotsReportMaskStructure
        {
            QMap<QString, int> ShowCombPlotsTypes; //!< Map of combined plots types to show

            // type (s,D,..) -> model -> yes/no
            QMap<QString, QMap<QString, QString>> ShowCombPlotParts; //!< Map of combined plot parts to show

            int has_combo_plots; //!< Flag for combined plots
        };

        CombPlotsReportMaskStructure combPlotsMask_edited; //!< Edited combined plots report mask structure

        struct MiscReportMaskStructure
        {
            QMap<QString, bool> ShowMiscParts; //!< Map of miscellaneous parts to show
        };

        MiscReportMaskStructure miscMask_edited; //!< Edited miscellaneous report mask structure

        QString JsonMask_gen_loaded;     //!< Loaded general JSON mask
        QString JsonMask_perChan_loaded; //!< Loaded per-channel JSON mask

        bool GMP_report;                 //!< Flag for GMP report

        US_Plot* plotLayout1; //!< Plot layout 1
        US_Plot* plotLayout2; //!< Plot layout 2

        // Widgets
        QwtPlot* data_plot1; //!< Data plot 1
        QwtPlot* data_plot2; //!< Data plot 2

    private:
        QJsonObject json;                         //!< JSON object
        QMap<QString, QTreeWidgetItem*> topItem;  //!< Map of top items
        QMap<QString, QTreeWidgetItem*> solutionItem; //!< Map of solution items
        QMap<QString, QTreeWidgetItem*> analysisItem; //!< Map of analysis items
        QMap<QString, QTreeWidgetItem*> analysisGenItem; //!< Map of general analysis items
        QMap<QString, QTreeWidgetItem*> analysis2DSAItem; //!< Map of 2DSA analysis items
        QMap<QString, QTreeWidgetItem*> analysisPCSAItem; //!< Map of PCSA analysis items
        QStringList topLevelItems;                     //!< List of top-level items

        QStringList solutionItems;             //!< List of solution items
        QStringList solutionItems_vals;        //!< List of solution item values

        QStringList analysisItems;             //!< List of analysis items

        QStringList analysisGenItems;          //!< List of general analysis items
        QStringList analysisGenItems_vals;     //!< List of general analysis item values

        QStringList analysis2DSAItems;         //!< List of 2DSA analysis items
        QStringList analysis2DSAItems_vals;    //!< List of 2DSA analysis item values

        QStringList analysisPCSAItems;         //!< List of PCSA analysis items
        QStringList analysisPCSAItems_vals;    //!< List of PCSA analysis item values

        // Miscellaneous tree masks
        QMap<QString, QTreeWidgetItem*> miscItem; //!< Map of miscellaneous items
        QStringList miscTopLevelItems;            //!< List of miscellaneous top-level items

        // Per-channel report masks
        QMap<QString, QTreeWidgetItem*> chanItem;               //!< Map of channel items
        QMap<QString, QTreeWidgetItem*> tripleItem;             //!< Map of triple items
        QMap<QString, QTreeWidgetItem*> tripleModelItem;        //!< Map of triple model items
        QMap<QString, QTreeWidgetItem*> tripleMaskItem;         //!< Map of triple mask items
        QMap<QString, QTreeWidgetItem*> tripleMaskPlotItem;     //!< Map of triple mask plot items
        QMap<QString, QTreeWidgetItem*> tripleMaskPseudoItem;   //!< Map of triple mask pseudo items
        QMap<QString, QTreeWidgetItem*> tripleMaskIndComboPlotItem; //!< Map of triple mask individual combined plot items

        // Combined plots masks
        QMap<QString, QTreeWidgetItem*> topItemCombPlots; //!< Map of top items for combined plots
        QMap<QString, QTreeWidgetItem*> ItemCombPlots;    //!< Map of items for combined plots

        QList<QStringList> gmpReportsDBdata; //!< GMP reports DB data
        QList<QStringList> autoflowdata;     //!< Autoflow data
        US_SelectItem* pdiag_autoflow;       //!< Autoflow select item dialog
        US_SelectItem* pdiag_autoflow_db;    //!< Autoflow select item dialog for DB

        QString html_assembled;              //!< Assembled HTML string
        QString html_failed;                 //!< Failed HTML string
        QString html_general;                //!< General HTML string
        QString html_lab_rotor;              //!< Lab rotor HTML string
        QString html_operator;               //!< Operator HTML string
        QString html_speed;                  //!< Speed HTML string
        QString html_cells;                  //!< Cells HTML string
        QString html_solutions;              //!< Solutions HTML string
        QString html_optical;                //!< Optical HTML string
        QString html_ranges;                 //!< Ranges HTML string
        QString html_scan_count;             //!< Scan count HTML string
        QString html_analysis_profile;       //!< Analysis profile HTML string
        QString html_analysis_profile_2dsa;  //!< 2DSA analysis profile HTML string
        QString html_analysis_profile_pcsa;  //!< PCSA analysis profile HTML string

        US_Help showHelp;                    //!< Help display object

        QPushButton* pb_download_report;     //!< Download report button
        QPushButton* pb_gen_report;          //!< Generate report button
        QPushButton* pb_view_report;         //!< View report button
        QPushButton* pb_view_report_db;      //!< View report from DB button
        QPushButton* pb_view_report_auto;    //!< View auto report button
        QPushButton* pb_select_all;          //!< Select all button
        QPushButton* pb_unselect_all;        //!< Unselect all button
        QPushButton* pb_expand_all;          //!< Expand all button
        QPushButton* pb_collapse_all;        //!< Collapse all button
        QPushButton* pb_help;                //!< Help button
        QPushButton* pb_close;               //!< Close button
        QLineEdit* le_loaded_run;            //!< Loaded run line edit
        QLineEdit* le_loaded_run_db;         //!< Loaded run DB line edit

        QTextEdit* te_fpath_info;            //!< File path info text edit
        QTextEdit* te_fpath_info_db;         //!< File path info DB text edit

        QString AProfileGUID;                //!< Analysis profile GUID
        QString ProtocolName_auto;           //!< Auto protocol name
        QString AutoflowID_auto;             //!< Auto autoflow ID
        QString FullRunName_auto;            //!< Auto full run name
        int invID;                           //!< Investigator ID
        QString runID;                       //!< Run ID
        QString runName;                     //!< Run name
        QString filePath;                    //!< File path
        QString filePath_db;                 //!< File path for DB
        QString FileName;                    //!< File name
        QString intensityID;                 //!< Intensity ID
        QString analysisIDs;                 //!< Analysis IDs
        QString autoflowStatusID;            //!< Autoflow status ID
        QString optimaName;                  //!< Optima name
        QString dataSource;
        bool    simulatedData;
  
        QString current_date;                //!< Current date

        QString duration_str;                //!< Duration string
        QString delay_stage_str;             //!< Delay stage string
        QString total_time_str;              //!< Total time string

        QString delay_uvvis_str;             //!< UVVIS delay string
        QString scanint_uvvis_str;           //!< UVVIS scan interval string
        QString delay_int_str;               //!< Intensity delay string
        QString scanint_int_str;             //!< Intensity scan interval string

        int ncells_used;                     //!< Number of cells used
        int nsol_channels;                   //!< Number of solution channels
        int nchan_optics;                    //!< Number of optical channels
        int nchan_ranges;                    //!< Number of channel ranges

        bool has_uvvis;                      //!< Flag for UVVIS
        bool has_interference;               //!< Flag for interference
        bool has_fluorescense;               //!< Flag for fluorescence

        QVector<QString> Array_of_triples;   //!< Array of triples
        QVector<QString> Array_of_tripleNames; //!< Array of triple names
        QMap<QString, QStringList> Triple_to_Models; //!< Triple to models map
        QMap<QString, QStringList> Triple_to_ModelsMissing; //!< Triple to missing models map
        QMap<QString, QString> Triple_to_FailedStage; //!< Triple to failed stage map
        QMap<QString, QMap<QString, QString>> Triple_to_ModelsDesc; //!< Triple to models description map
        QMap<QString, QMap<QString, QString>> Triple_to_ModelsDescGuid; //!< Triple to models description GUID map

        QStringList droppedTriplesList; //!< List of dropped triples

        QMap<QString, QString> triple_info_map; //!< Triple info map
        QString currentTripleName; //!< Current triple name

        QMap<QString, QString> intensityRIMap; //!< Intensity RI map
        QMap<QString, QMap<QString, QString>> comboPlotsMap; //!< Combined plots map
        QMap<QString, int> comboPlotsMapTypes; //!< Combined plots types map
        QMap<QString, QMap<QString, QMap<QString, bool>>> indComboPlotsMapTripleTypeRangeBool; //!< Individual combined plots map
        QMap<QString, QStringList> CombPlots_Type_to_Models; //!< Combined plots type to models map
        QMap<QString, QString> eSign_details; //!< Electronic signature details

        QMap<QString, QStringList> CombPlotsParmsMap; //!< Combined plots parameters map
        QMap<QString, QList<QColor>> CombPlotsParmsMap_Colors; //!< Combined plots parameters colors map

        void get_current_date(void); //!< Get the current date
        void format_needed_params(void); //!< Format needed parameters
        void assemble_pdf(QProgressDialog*); //!< Assemble PDF
        void add_solution_details(const QString, const QString, QString&); //!< Add solution details
        void assemble_parts(QString&); //!< Assemble parts
        int list_all_gmp_reports_db(QList<QStringList>&, US_DB2*); //!< List all GMP reports from DB
        int list_all_autoflow_records(QList<QStringList>&); //!< List all autoflow records
        QMap<QString, QString> read_autoflow_record(int); //!< Read autoflow record
        void write_pdf_report(void); //!< Write PDF report
        void remove_files_by_mask(QString, QStringList); //!< Remove files by mask
        void write_gmp_report_DB(QString, QString); //!< Write GMP report to DB

        void assemble_user_inputs_html(void); //!< Assemble user inputs in HTML
        void assemble_run_details_html(void); //!< Assemble run details in HTML
        int get_expID_by_runID_invID(US_DB2*, QString); //!< Get experiment ID by run ID and investigator ID
        double get_loading_volume(int); //!< Get loading volume

        void read_autoflowStatus_record(QString&, QString&, QString&, QString&,
                                        QString&, QString&, QString&, QString&, QString&,
                                        QString&, QString&, QString&, QString&, QString&,
                                        QString&, QString&); //!< Read autoflow status record
        QMap<QString, QMap<QString, QString>> parse_autoflowStatus_json(const QString, const QString); //!< Parse autoflow status JSON
        QMap<QString, QString> parse_autoflowStatus_analysis_json(const QString); //!< Parse autoflow status analysis JSON

        void read_reportLists_from_aprofile(QStringList&, QStringList&); //!< Read report lists from analysis profile
        bool readReportLists(QXmlStreamReader&, QMap<QString, QString>&, QMap<QString, QString>&); //!< Read report lists
        QStringList buildDroppedTriplesList(US_DB2*, QMap<QString, QString>); //!< Build dropped triples list

        void read_protocol_and_reportMasks(void); //!< Read protocol and report masks
        QMap<QString, QString> read_autoflowIntensity(QString, US_DB2*); //!< Read autoflow intensity
        QMap<QString, QString> read_autoflowGMPReportEsign_record(US_DB2*); //!< Read autoflow GMP report electronic signature record
        QString get_assigned_oper_revs(QJsonDocument); //!< Get assigned operator revisions
        void parse_gen_mask_json(const QString); //!< Parse general mask JSON
        QMap<QString, QMap<QString, QString>> parse_comb_plots_json(const QString); //!< Parse combined plots JSON
        QMap<QString, QString> parse_models_desc_json(const QString, const QString); //!< Parse models description JSON

        void get_item_childs(QList<QTreeWidgetItem*>&, QTreeWidgetItem*); //!< Get item children
        void build_genTree(void); //!< Build general tree
        void build_miscTree(void); //!< Build miscellaneous tree
        void build_perChanTree(void); //!< Build per-channel tree
        void build_combPlotsTree(void); //!< Build combined plots tree
        void gui_to_parms(void); //!< Convert GUI to parameters

        void get_children_to_json(QString&, QTreeWidgetItem*); //!< Get children to JSON
        QString tree_to_json(QMap<QString, QTreeWidgetItem*>); //!< Convert tree to JSON
        void parse_edited_gen_mask_json(const QString, GenReportMaskStructure&); //!< Parse edited general mask JSON
        void parse_edited_perChan_mask_json(const QString, PerChanReportMaskStructure&); //!< Parse edited per-channel mask JSON
        void parse_edited_combPlots_mask_json(const QString, CombPlotsReportMaskStructure&); //!< Parse edited combined plots mask JSON
        void parse_edited_misc_mask_json(const QString, MiscReportMaskStructure&); //!< Parse edited miscellaneous mask JSON

        bool model_exists; //!< Flag for model existence

        QVector<US_DataIO::RawData> rawData; //!< Raw data vector
        QVector<US_DataIO::EditedData> editedData; //!< Edited data vector

        QVector<SP_SPEEDPROFILE> speed_steps; //!< Speed steps

        US_DataIO::EditedData* edata; //!< Edited data
        US_DataIO::RawData* rdata; //!< Raw data
        US_DataIO::RawData* sdata; //!< Simulation data
        US_DataIO::RawData wsdata; //!< Workspace data

        QPointer<US_ResidsBitmap> rbmapd; //!< Residuals bitmap dialog

        /**
         * @class ModelDesc
         * @brief Class to hold model descriptions.
         */
        class ModelDesc
        {
            public:
                QString description; //!< Full model description
                QString baseDescr; //!< Base analysis-set description
                QString fitfname; //!< Associated fit file name
                QString modelID; //!< Model DB ID
                QString modelGUID; //!< Model GUID
                QString filepath; //!< Full path model file name
                QString editID; //!< Edit parent DB ID
                QString editGUID; //!< Edit parent GUID
                QString antime; //!< Analysis date & time (yymmddHHMM)
                QDateTime lmtime; //!< Record last modified date & time
                double variance; //!< Variance value
                double meniscus; //!< Meniscus radius value
                double bottom; //!< Bottom radius value

                /**
                 * @brief Less than operator to enable sort.
                 * @param md The ModelDesc object to compare with
                 * @return True if this object is less than the given object
                 */
                bool operator<(const ModelDesc& md) const { return (description < md.description); }
        };

        /**
         * @class NoiseDesc
         * @brief Class to hold noise descriptions.
         */
        class NoiseDesc
        {
            public:
                QString description; //!< Full noise description
                QString baseDescr; //!< Base analysis-set description
                QString noiseID; //!< Noise DB ID
                QString noiseGUID; //!< Noise GUID
                QString filepath; //!< Full path noise file name
                QString modelID; //!< Model parent DB ID
                QString modelGUID; //!< Model parent GUID
                QString antime; //!< Analysis date & time (yymmddHHMM)

                /**
                 * @brief Less than operator to enable sort.
                 * @param nd The NoiseDesc object to compare with
                 * @return True if this object is less than the given object
                 */
                bool operator<(const NoiseDesc& nd) const { return (description < nd.description); }
        };

        QVector<double> v_meni; //!< Meniscus vector
        QVector<double> v_bott; //!< Bottom vector
        QVector<double> v_rmsd; //!< RMSD vector

        QString filedir; //!< File directory
        QString fname_load; //!< File name to load
        QString fname_edit; //!< Edit file name
        QStringList edtfiles; //!< Edit files list
        int nedtfs; //!< Number of edit files
        int ix_best; //!< Best index
        int ix_setfit; //!< Set fit index
        bool have3val; //!< Flag for having 3 values
        bool bott_fit; //!< Flag for bottom fit
        int idEdit; //!< Edit ID

        double fit_xvl; //!< Fit x value for 2D data
        double f_meni; //!< Meniscus value for 3D data
        double f_bott; //!< Bottom value for 3D data

        double dy_global; //!< Global y value
        double miny_global; //!< Global minimum y value
        QString tripleInfo; //!< Triple information

        int eID_global; //!< Global experiment ID
        QString eID_updated; //!< Updated experiment ID

        US_Model model; //!< Model
        US_Model model_loaded; //!< Loaded model
        US_Model model_used; //!< Used model

        US_Noise ri_noise; //!< RI noise
        US_Noise ti_noise; //!< TI noise
        QList<int> excludedScans; //!< Excluded scans list
        US_Solution solution_rec; //!< Solution record

        US_Math2::SolutionData solution; //!< Solution data
        QVector<QVector<double>> resids; //!< Residuals vector

        US_SimulationParameters simparams; //!< Simulation parameters
        QList<US_DataIO::RawData> tsimdats; //!< Simulation data list
        QList<US_Model> tmodels; //!< Models list
        QVector<int> kcomps; //!< Components vector

        QStringList noiIDs; //!< Noise GUIDs list
        QStringList noiEdIDs; //!< Noise edit GUIDs list
        QStringList noiMoIDs; //!< Noise model GUIDs list
        QStringList noiTypes; //!< Noise types list
        QStringList modIDs; //!< Model GUIDs list
        QStringList modEdIDs; //!< Model edit GUIDs list
        QStringList modDescs; //!< Model descriptions list

        int thrdone; //!< Thread done flag

        double density; //!< Density
        double viscosity; //!< Viscosity
        double vbar; //!< Vbar
        double compress; //!< Compressibility

        QString svbar_global; //!< Global svbar

        bool manual; //!< Manual flag
        bool dataLoaded; //!< Data loaded flag
        bool haveSim; //!< Have simulation flag
        bool dataLatest; //!< Data latest flag
        bool buffLoaded; //!< Buffer loaded flag
        bool cnstvb; //!< Constant vbar flag
        bool cnstff; //!< Constant f/f0 flag
        bool exp_steps; //!< Experimental steps flag
        bool dat_steps; //!< Data steps flag
        bool is_dmga_mc; //!< DMGA Monte Carlo flag

        QMap<QString, QString> adv_vals; //!< Advanced values map

        int dbg_level; //!< Debug level
        int nthread; //!< Number of threads
        int scanCount; //!< Scan count

        QPoint rpd_pos; //!< Residuals plot position
        QString FileName_parsed; //!< Parsed file name

        QString rmsd_global; //!< Global RMSD

        void simulate_triple(const QString, QString); //!< Simulate triple
        bool loadData(QMap<QString, QString>&); //!< Load data
        bool loadModel(QMap<QString, QString>&); //!< Load model
        bool loadNoises(QMap<QString, QString>&); //!< Load noises
        void loadNoises_whenAbsent(void); //!< Load noises when absent
        int count_noise_auto(US_DataIO::EditedData*, US_Model*, QStringList&, QStringList&); //!< Count noise automatically

        int id_list_db_auto(QString); //!< Get ID list from DB automatically
        int models_in_edit_auto(QString, QStringList&); //!< Get models in edit automatically
        int noises_in_model_auto(QString, QStringList&); //!< Get noises in model automatically
        void simulateModel(QMap<QString, QString>&); //!< Simulate model
        void adjustModel(void); //!< Adjust model

        QStringList scan_dbase_models(QStringList); //!< Scan database models

        QString text_model(US_Model, int); //!< Convert model to text
        QString html_header(QString, QString, US_DataIO::EditedData*); //!< Generate HTML header
        QString distrib_info(QMap<QString, QString>&); //!< Generate distribution information
        QString calc_replicates_averages(void); //!< Calculate replicates averages
        QString get_replicate_group_number(QString); //!< Get replicate group number
        QMap<QString, double> get_replicate_group_results(US_ReportGMP::ReportItem, QString, QStringList); //!< Get replicate group results
        void assemble_replicate_av_integration_html(void); //!< Assemble replicate average integration HTML

        QString get_filename(QString); //!< Get file name

    public slots:
        void thread_progress(int, int); //!< Slot for thread progress
        void thread_complete(int); //!< Slot for thread completion
        void resplot_done(void); //!< Slot for residuals plot done
        void update_progress(int); //!< Slot for updating progress

    private slots:
        void loadRun_auto(QMap<QString, QString>&); //!< Load run automatically
        void check_failed_triples(void); //!< Check failed triples
        QMap<QString, QString> read_autoflowAnalysis_record(US_DB2*, const QString&); //!< Read autoflow analysis record
        QMap<QString, QString> read_autoflowAnalysisHistory_record(US_DB2*, const QString&); //!< Read autoflow analysis history record
        void check_models(int); //!< Check models
        void check_for_missing_models(void); //!< Check for missing models
        void check_for_dropped_triples(void); //!< Check for dropped triples
        QString compose_html_failed_stage_missing_models(void); //!< Compose HTML for failed stage with missing models
        QString missing_models_msg(void); //!< Generate missing models message
        void reset_report_panel(void); //!< Reset report panel
        void view_report(void); //!< View report
        void view_report_db(void); //!< View report from DB
        void load_gmp_report_db(void); //!< Load GMP report from DB
        void load_gmp_run(void); //!< Load GMP run
        void generate_report(void); //!< Generate report
        void changedItem(QTreeWidgetItem*, int); //!< Handle item change
        void select_all(void); //!< Select all items
        void unselect_all(void); //!< Unselect all items
        void expand_all(void); //!< Expand all items
        void collapse_all(void); //!< Collapse all items

        void show_results(QMap<QString, QString>&); //!< Show results
        void calc_residuals(void); //!< Calculate residuals
        void assemble_distrib_html(QMap<QString, QString>&); //!< Assemble distribution HTML
        void assemble_plots_html(QStringList, QString = QString("")); //!< Assemble plots HTML
        double interp_sval(double, double*, double*, int); //!< Interpolate s-value
        void plotres(QMap<QString, QString>&); //!< Plot residuals
        void plot_pseudo3D(QString, QString); //!< Plot pseudo 3D
        bool modelGuidExistsForStage(QString, QString); //!< Check if model GUID exists for stage
        bool modelGuidExistsForStage_ind(QString, QString, QString); //!< Check if model GUID exists for stage (individual)
        void process_combined_plots(QString); //!< Process combined plots
        void process_combined_plots_individual(QString, QString); //!< Process combined plots (individual)
        QMap< QString, QStringList > find_sim_ranges( QString, QString );

        QMap<QString, QString> read_autoflowGMPReportEsign_record(QString); //!< Read autoflow GMP report electronic signature record
        void get_assigned_oper_revs(QJsonDocument, QStringList&); //!< Get assigned operator revisions

        QString indent(int) const; //!< Generate indent
        QString table_row(const QString&, const QString&) const; //!< Generate table row with 2 columns
        QString table_row(const QString&, const QString&, const QString&) const; //!< Generate table row with 3 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 4 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 5 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 6 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 7 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 8 columns
        QString table_row(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) const; //!< Generate table row with 9 columns

        /**
         * @brief Paint the page for the PDF report.
         * @param printer The QPrinter object
         * @param pageNumber The page number
         * @param pageCount The total number of pages
         * @param painter The QPainter object
         * @param doc The QTextDocument object
         * @param textRect The text rectangle
         * @param footerHeight The footer height
         */
        void paintPage(QPrinter& printer, int pageNumber, int pageCount,
                       QPainter* painter, QTextDocument* doc,
                       const QRectF& textRect, qreal footerHeight);

        /**
         * @brief Print the document.
         * @param printer The QPrinter object
         * @param doc The QTextDocument object
         */
        void printDocument(QPrinter& printer, QTextDocument* doc);

        /**
         * @brief Convert millimeters to pixels.
         * @param printer The QPrinter object
         * @param mm The value in millimeters
         * @return The value in pixels
         */
        double mmToPixels(QPrinter& printer, int mm);

        /**
         * @brief Write plot to file.
         * @param filename The file name
         * @param plot The QwtPlot object
         */
        void write_plot(const QString& filename, const QwtPlot* plot);

        /**
         * @brief Create directory if it does not exist.
         * @param dirPath The directory path
         * @param dirName The directory name
         * @return True if the directory was created, false otherwise
         */
        bool mkdir(const QString& dirPath, const QString& dirName);

        /**
         * @brief Plot stick distribution.
         * @param index The index of the distribution
         */
        void distrib_plot_stick(int index);

        /**
         * @brief Plot 2D distribution.
         * @param index The index of the distribution
         */
        void distrib_plot_2d(int index);

        /**
         * @brief Display help.
         */
        void help(void)
        { showHelp.show_help("reporter_gmp.html"); };

        signals:
                /**
                 * @brief Signal to reset reports.
                 */
                void reset_reports(void);
};

#endif
