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
 * @brief The US_ReporterGMP class provides a user interface for generating GMP reports.
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
         * @brief Constructor for US_ReporterGMP with run ID.
         * @param runID The run ID
         */
        US_ReporterGMP(QString runID);

        /**
         * @brief Constructor for US_ReporterGMP with autoflow record data.
         * @param autoflowData The autoflow record data
         */
        US_ReporterGMP(QMap<QString, QString> autoflowData);

        /**
         * @brief Write GMP report to the database (for testing).
         * @param runID The run ID
         * @param reportText The report text
         * @param reportID The report ID
         * @param operatorName The operator name
         * @param reviewerName The reviewer name
         */
        void write_gmp_report_DB_test(QString runID, QString reportText, int reportID, QString operatorName, QString reviewerName);

        QTreeWidget* genTree;                   //!< General report tree widget
        QTreeWidget* miscTree;                  //!< Miscellaneous report tree widget
        QTreeWidget* perChanTree;               //!< Per-channel report tree widget
        QTreeWidget* combPlotsTree;             //!< Combined plots report tree widget
        bool first_time_gen_tree_build;         //!< Flag for first time building general tree
        bool first_time_misc_tree_build;        //!< Flag for first time building miscellaneous tree
        bool first_time_perChan_tree_build;     //!< Flag for first time building per-channel tree

        bool auto_mode;                         //!< Flag for automatic mode
        QLabel* lb_hdr1;                        //!< Header label

        QProgressDialog* progress_msg;          //!< Progress dialog
        US_RunProtocol currProto;               //!< Current protocol

        QString ap_xml;                         //!< Analysis profile XML

        US_Pseudo3D_Combine* sdiag_pseudo3d;    //!< Pseudo 3D combine dialog
        US_DDistr_Combine* sdiag_combplot;      //!< Combined plot dialog
        US_AnalysisProfileGui* sdiag;           //!< Analysis profile GUI dialog
        US_AnaProfile currAProf;                //!< Current analysis profile
        US_AnaProfile::AnaProf2DSA cAP2;        //!< 2DSA analysis profile
        US_AnaProfile::AnaProfPCSA cAPp;        //!< PCSA analysis profile
        QStringList chndescs;                   //!< Channel descriptions
        QStringList chndescs_alt;               //!< Alternative channel descriptions
        QMap<QString, QMap<QString, US_ReportGMP>> ch_reports;           //!< Channel reports map
        QMap<QString, QMap<QString, US_ReportGMP>> ch_reports_internal;  //!< Internal channel reports map
        QMap<QString, QList<double>> ch_wvls;   //!< Channel wavelengths map

        QList<int> replicates;                  //!< List of replicates
        QMap<int, QStringList> replicates_to_channdesc;    //!< Map of replicates to channel descriptions
        QMap<QString, QStringList> channdesc_to_overlapping_wvls;    //!< Map of channel descriptions to overlapping wavelengths

        QPointer<US_ResidPlotFem> resplotd;     //!< Residual plot dialog
        QPointer<US_PlotControlFem> eplotcd;    //!< Plot control dialog
        US_DataIO::EditedData* rg_editdata();   //!< Edited data
        US_DataIO::RawData* rg_simdata();       //!< Simulated data
        QList<int>* rg_excllist();              //!< Exclusion list
        US_Model* rg_model();                   //!< Model
        US_Noise* rg_ti_noise();                //!< TI noise
        US_Noise* rg_ri_noise();                //!< RI noise
        QPointer<US_ResidsBitmap> rg_resbmap(); //!< Residual bitmap
        QString rg_tripleInfo();                //!< Triple information

        // Report mask structures
        struct GenReportMaskStructure
        {
            QMap<QString, bool> ShowReportParts;                             //!< Show report parts map
            QMap<QString, QString> ShowSolutionParts;                        //!< Show solution parts map
            QMap<QString, QString> ShowAnalysisGenParts;                     //!< Show general analysis parts map
            QMap<QString, QString> ShowAnalysis2DSAParts;                    //!< Show 2DSA analysis parts map
            QMap<QString, QString> ShowAnalysisPCSAParts;                    //!< Show PCSA analysis parts map
            int has_anagen_items;                                            //!< Flag for general analysis items
            int has_ana2dsa_items;                                           //!< Flag for 2DSA analysis items
            int has_anapcsa_items;                                           //!< Flag for PCSA analysis items
        };

        GenReportMaskStructure genMask_edited;  //!< Edited general report mask

        struct PerChanReportMaskStructure
        {
            QMap<QString, bool> ShowChannelParts;                            //!< Show channel parts map
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelParts;          //!< Show triple model parts map
            QMap<QString, QMap<QString, int>> has_tripleModel_items;         //!< Flag for triple model items
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelPlotParts;       //!< Show triple model plot parts map
            QMap<QString, QMap<QString, int>> has_tripleModelPlot_items;     //!< Flag for triple model plot items
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleModelPseudo3dParts;   //!< Show triple model pseudo 3D parts map
            QMap<QString, QMap<QString, int>> has_tripleModelPseudo3d_items; //!< Flag for triple model pseudo 3D items
            QMap<QString, QMap<QString, QMap<QString, QString>>> ShowTripleTypeModelRangeIndividualCombo; //!< Show individual combined plot parts map
            QMap<QString, QMap<QString, int>> has_tripleModelIndCombo_items; //!< Flag for triple model individual combined plot items
        };

        PerChanReportMaskStructure perChanMask_edited;  //!< Edited per-channel report mask

        struct CombPlotsReportMaskStructure
        {
            QMap<QString, int> ShowCombPlotsTypes;                           //!< Show combined plots types map
            QMap<QString, QMap<QString, QString>> ShowCombPlotParts;         //!< Show combined plot parts map
            int has_combo_plots;                                             //!< Flag for combined plots
        };

        CombPlotsReportMaskStructure combPlotsMask_edited;  //!< Edited combined plots report mask

        struct MiscReportMaskStructure
        {
            QMap<QString, bool> ShowMiscParts;                               //!< Show miscellaneous parts map
        };

        MiscReportMaskStructure miscMask_edited;  //!< Edited miscellaneous report mask

        QString JsonMask_gen_loaded;     //!< Loaded general JSON mask
        QString JsonMask_perChan_loaded; //!< Loaded per-channel JSON mask

        bool GMP_report;                 //!< Flag for GMP report

        US_Plot* plotLayout1;            //!< Plot layout 1
        US_Plot* plotLayout2;            //!< Plot layout 2

        QwtPlot* data_plot1;             //!< Data plot 1
        QwtPlot* data_plot2;             //!< Data plot 2

    private:
        // General report mask
        QJsonObject json;                //!< JSON object for general report mask
        QMap<QString, QTreeWidgetItem*> topItem;                //!< Top item map for general report mask
        QMap<QString, QTreeWidgetItem*> solutionItem;           //!< Solution item map for general report mask
        QMap<QString, QTreeWidgetItem*> analysisItem;           //!< Analysis item map for general report mask
        QMap<QString, QTreeWidgetItem*> analysisGenItem;        //!< General analysis item map for general report mask
        QMap<QString, QTreeWidgetItem*> analysis2DSAItem;       //!< 2DSA analysis item map for general report mask
        QMap<QString, QTreeWidgetItem*> analysisPCSAItem;       //!< PCSA analysis item map for general report mask
        QStringList topLevelItems;       //!< Top-level items for general report mask
        QStringList solutionItems;       //!< Solution items for general report mask
        QStringList solutionItems_vals;  //!< Solution items values for general report mask
        QStringList analysisItems;       //!< Analysis items for general report mask
        QStringList analysisGenItems;    //!< General analysis items for general report mask
        QStringList analysisGenItems_vals; //!< General analysis items values for general report mask
        QStringList analysis2DSAItems;   //!< 2DSA analysis items for general report mask
        QStringList analysis2DSAItems_vals; //!< 2DSA analysis items values for general report mask
        QStringList analysisPCSAItems;   //!< PCSA analysis items for general report mask
        QStringList analysisPCSAItems_vals; //!< PCSA analysis items values for general report mask

        // Miscellaneous report masks
        QMap<QString, QTreeWidgetItem*> miscItem;  //!< Miscellaneous item map
        QStringList miscTopLevelItems;             //!< Top-level items for miscellaneous report mask

        // Per-channel report masks
        QMap<QString, QTreeWidgetItem*> chanItem;                          //!< Channel item map
        QMap<QString, QTreeWidgetItem*> tripleItem;                        //!< Triple item map
        QMap<QString, QTreeWidgetItem*> tripleModelItem;                   //!< Triple model item map
        QMap<QString, QTreeWidgetItem*> tripleMaskItem;                    //!< Triple mask item map
        QMap<QString, QTreeWidgetItem*> tripleMaskPlotItem;                //!< Triple mask plot item map
        QMap<QString, QTreeWidgetItem*> tripleMaskPseudoItem;              //!< Triple mask pseudo 3D item map
        QMap<QString, QTreeWidgetItem*> tripleMaskIndComboPlotItem;        //!< Triple mask individual combined plot item map

        // Combined plots masks
        QMap<QString, QTreeWidgetItem*> topItemCombPlots;                  //!< Top item map for combined plots
        QMap<QString, QTreeWidgetItem*> ItemCombPlots;                     //!< Item map for combined plots

        QList<QStringList> gmpReportsDBdata;                               //!< GMP reports data from the database
        QList<QStringList> autoflowdata;                                   //!< Autoflow data
        US_SelectItem* pdiag_autoflow;                                     //!< Autoflow selection dialog
        US_SelectItem* pdiag_autoflow_db;                                  //!< Autoflow selection dialog for the database

        QString html_assembled;                                            //!< Assembled HTML
        QString html_failed;                                               //!< HTML for failed stages
        QString html_general;                                              //!< General HTML
        QString html_lab_rotor;                                            //!< Lab rotor HTML
        QString html_operator;                                             //!< Operator HTML
        QString html_speed;                                                //!< Speed HTML
        QString html_cells;                                                //!< Cells HTML
        QString html_solutions;                                            //!< Solutions HTML
        QString html_optical;                                              //!< Optical HTML
        QString html_ranges;                                               //!< Ranges HTML
        QString html_scan_count;                                           //!< Scan count HTML
        QString html_analysis_profile;                                     //!< Analysis profile HTML
        QString html_analysis_profile_2dsa;                                //!< 2DSA analysis profile HTML
        QString html_analysis_profile_pcsa;                                //!< PCSA analysis profile HTML

        US_Help showHelp;                                                  //!< Help display object

        QPushButton* pb_download_report;                                   //!< Download report button
        QPushButton* pb_gen_report;                                        //!< Generate report button
        QPushButton* pb_view_report;                                       //!< View report button
        QPushButton* pb_view_report_db;                                    //!< View report button for the database
        QPushButton* pb_view_report_auto;                                  //!< View report button for autoflow
        QPushButton* pb_select_all;                                        //!< Select all button
        QPushButton* pb_unselect_all;                                      //!< Unselect all button
        QPushButton* pb_expand_all;                                        //!< Expand all button
        QPushButton* pb_collapse_all;                                      //!< Collapse all button
        QPushButton* pb_help;                                              //!< Help button
        QPushButton* pb_close;                                             //!< Close button
        QLineEdit* le_loaded_run;                                          //!< Line edit for loaded run
        QLineEdit* le_loaded_run_db;                                       //!< Line edit for loaded run from the database
        QTextEdit* te_fpath_info;                                          //!< Text edit for file path information
        QTextEdit* te_fpath_info_db;                                       //!< Text edit for file path information from the database

        QString AProfileGUID;                                              //!< Analysis profile GUID
        QString ProtocolName_auto;                                         //!< Protocol name for autoflow
        QString AutoflowID_auto;                                           //!< Autoflow ID for autoflow
        QString FullRunName_auto;                                          //!< Full run name for autoflow
        int invID;                                                         //!< Investigation ID
        QString runID;                                                     //!< Run ID
        QString runName;                                                   //!< Run name
        QString filePath;                                                  //!< File path
        QString filePath_db;                                               //!< File path from the database
        QString FileName;                                                  //!< File name
        QString intensityID;                                               //!< Intensity ID
        QString analysisIDs;                                               //!< Analysis IDs
        QString autoflowStatusID;                                          //!< Autoflow status ID
        QString optimaName;                                                //!< Optima name

        QString current_date;                                              //!< Current date

        QString duration_str;                                              //!< Duration string
        QString delay_stage_str;                                           //!< Delay stage string
        QString total_time_str;                                            //!< Total time string

        QString delay_uvvis_str;                                           //!< UV/Vis delay string
        QString scanint_uvvis_str;                                         //!< UV/Vis scan interval string
        QString delay_int_str;                                             //!< Interference delay string
        QString scanint_int_str;                                           //!< Interference scan interval string

        int ncells_used;                                                   //!< Number of cells used
        int nsol_channels;                                                 //!< Number of solution channels
        int nchan_optics;                                                  //!< Number of channels with optics
        int nchan_ranges;                                                  //!< Number of channels with ranges

        bool has_uvvis;                                                    //!< Flag for UV/Vis
        bool has_interference;                                             //!< Flag for interference
        bool has_fluorescence;                                             //!< Flag for fluorescence

        QVector<QString> Array_of_triples;                                 //!< Array of triples
        QVector<QString> Array_of_tripleNames;                             //!< Array of triple names
        QMap<QString, QStringList> Triple_to_Models;                       //!< Map of triples to models
        QMap<QString, QStringList> Triple_to_ModelsMissing;                //!< Map of triples to missing models
        QMap<QString, QString> Triple_to_FailedStage;                      //!< Map of triples to failed stages
        QMap<QString, QMap<QString, QString>> Triple_to_ModelsDesc;        //!< Map of triples to model descriptions
        QMap<QString, QMap<QString, QString>> Triple_to_ModelsDescGuid;    //!< Map of triples to model description GUIDs

        QStringList droppedTriplesList;                                    //!< List of dropped triples

        QMap<QString, QString> triple_info_map;                            //!< Triple information map
        QString currentTripleName;                                         //!< Current triple name

        QMap<QString, QString> intensityRIMap;                             //!< Intensity RI map
        QMap<QString, QMap<QString, QString>> comboPlotsMap;               //!< Combined plots map
        QMap<QString, int> comboPlotsMapTypes;                             //!< Combined plots map types
        QMap<QString, QMap<QString, QMap<QString, bool>>> indComboPlotsMapTripleTypeRangeBool; //!< Individual combined plots map
        QMap<QString, QStringList> CombPlots_Type_to_Models;               //!< Combined plots types to models map
        QMap<QString, QString> eSign_details;                              //!< E-signature details

        QMap<QString, QStringList> CombPlotsParmsMap;                      //!< Combined plots parameters map
        QMap<QString, QList<QColor>> CombPlotsParmsMap_Colors;             //!< Combined plots parameters colors map

        void get_current_date(void);                                       //!< Get the current date
        void format_needed_params(void);                                   //!< Format needed parameters
        void assemble_pdf(QProgressDialog* progress);                      //!< Assemble PDF
        void add_solution_details(const QString solution, const QString description, QString& html); //!< Add solution details
        void assemble_parts(QString& html);                                //!< Assemble parts
        int list_all_gmp_reports_db(QList<QStringList>& reportList, US_DB2* db); //!< List all GMP reports in the database
        int list_all_autoflow_records(QList<QStringList>& autoflowList);   //!< List all autoflow records
        QMap<QString, QString> read_autoflow_record(int autoflowID);       //!< Read autoflow record
        void write_pdf_report(void);                                       //!< Write PDF report
        void remove_files_by_mask(QString path, QStringList maskList);     //!< Remove files by mask
        void write_gmp_report_DB(QString runID, QString reportText);       //!< Write GMP report to the database

        void assemble_user_inputs_html(void);                              //!< Assemble user inputs HTML
        void assemble_run_details_html(void);                              //!< Assemble run details HTML
        int get_expID_by_runID_invID(US_DB2* db, QString runID);           //!< Get experiment ID by run ID and investigation ID
        double get_loading_volume(int cellID);                             //!< Get loading volume
        void read_autoflowStatus_record(QString& runID, QString& invID, QString& opName, QString& rpName, QString& statFlag, QString& progXML, QString& runID_str, QString& runType, QString& stageDelay, QString& scanintUVVis, QString& delayUVVis, QString& scanintInt, QString& delayInt, QString& channcount); //!< Read autoflow status record
        QMap<QString, QMap<QString, QString>> parse_autoflowStatus_json(const QString jsonString, const QString recordType); //!< Parse autoflow status JSON
        QMap<QString, QString> parse_autoflowStatus_analysis_json(const QString jsonString); //!< Parse autoflow status analysis JSON

        void read_reportLists_from_aprofile(QStringList& analysisList, QStringList& channelList); //!< Read report lists from analysis profile
        bool readReportLists(QXmlStreamReader& xml, QMap<QString, QString>& analysisList, QMap<QString, QString>& channelList); //!< Read report lists
        QStringList buildDroppedTriplesList(US_DB2* db, QMap<QString, QString> analysisList); //!< Build dropped triples list

        void read_protocol_and_reportMasks(void);                          //!< Read protocol and report masks
        QMap<QString, QString> read_autoflowIntensity(QString autoflowID, US_DB2* db); //!< Read autoflow intensity
        QMap<QString, QString> read_autoflowGMPReportEsign_record(US_DB2* db); //!< Read autoflow GMP report E-signature record
        QString get_assigned_oper_revs(QJsonDocument jsonDoc);             //!< Get assigned operators and reviewers from JSON document
        void parse_gen_mask_json(const QString jsonString);                //!< Parse general mask JSON
        QMap<QString, QMap<QString, QString>> parse_comb_plots_json(const QString jsonString); //!< Parse combined plots JSON
        QMap<QString, QString> parse_models_desc_json(const QString jsonString, const QString modelID); //!< Parse models description JSON

        void get_item_childs(QList<QTreeWidgetItem*>& childList, QTreeWidgetItem* item); //!< Get child items
        void build_genTree(void);                          //!< Build general tree
        void build_miscTree(void);                         //!< Build miscellaneous tree
        void build_perChanTree(void);                      //!< Build per-channel tree
        void build_combPlotsTree(void);                    //!< Build combined plots tree
        void gui_to_parms(void);                           //!< Convert GUI elements to parameters

        void get_children_to_json(QString& jsonString, QTreeWidgetItem* item); //!< Get child items to JSON
        QString tree_to_json(QMap<QString, QTreeWidgetItem*> treeItemMap); //!< Convert tree to JSON
        void parse_edited_gen_mask_json(const QString jsonString, GenReportMaskStructure& maskStruct); //!< Parse edited general mask JSON
        void parse_edited_perChan_mask_json(const QString jsonString, PerChanReportMaskStructure& maskStruct); //!< Parse edited per-channel mask JSON
        void parse_edited_combPlots_mask_json(const QString jsonString, CombPlotsReportMaskStructure& maskStruct); //!< Parse edited combined plots mask JSON
        void parse_edited_misc_mask_json(const QString jsonString, MiscReportMaskStructure& maskStruct); //!< Parse edited miscellaneous mask JSON

        bool model_exists;                                                 //!< Flag for model existence

        QVector<US_DataIO::RawData> rawData;                               //!< Vector of raw data
        QVector<US_DataIO::EditedData> editedData;                         //!< Vector of edited data

        QVector<SP_SPEEDPROFILE> speed_steps;                              //!< Vector of speed steps

        US_DataIO::EditedData* edata;                                      //!< Edited data pointer
        US_DataIO::RawData* rdata;                                         //!< Raw data pointer
        US_DataIO::RawData* sdata;                                         //!< Simulated data pointer
        US_DataIO::RawData wsdata;                                         //!< Workspace data

        QPointer<US_ResidsBitmap> rbmapd;                                  //!< Residuals bitmap dialog

        /**
         * @class ModelDesc
         * @brief Class to describe model entries.
         */
        class ModelDesc
        {
        public:
            QString description;    //!< Full model description
            QString baseDescr;      //!< Base analysis-set description
            QString fitfname;       //!< Associated fit file name
            QString modelID;        //!< Model DB ID
            QString modelGUID;      //!< Model GUID
            QString filepath;       //!< Full path model file name
            QString editID;         //!< Edit parent DB ID
            QString editGUID;       //!< Edit parent GUID
            QString antime;         //!< Analysis date & time (yymmddHHMM)
            QDateTime lmtime;       //!< Record lastmod date & time
            double variance;        //!< Variance value
            double meniscus;        //!< Meniscus radius value
            double bottom;          //!< Bottom radius value

            /**
             * @brief Less than operator to enable sort
             * @param md The other ModelDesc to compare
             * @return True if less than, false otherwise
             */
            bool operator<(const ModelDesc& md) const
            {
                return (description < md.description);
            }
        };

        /**
         * @class NoiseDesc
         * @brief Class to describe noise entries.
         */
        class NoiseDesc
        {
        public:
            QString description;    //!< Full noise description
            QString baseDescr;      //!< Base analysis-set description
            QString noiseID;        //!< Noise DB ID
            QString noiseGUID;      //!< Noise GUID
            QString filepath;       //!< Full path noise file name
            QString modelID;        //!< Model parent DB ID
            QString modelGUID;      //!< Model parent GUID
            QString antime;         //!< Analysis date & time (yymmddHHMM)

            /**
             * @brief Less than operator to enable sort
             * @param nd The other NoiseDesc to compare
             * @return True if less than, false otherwise
             */
            bool operator<(const NoiseDesc& nd) const
            {
                return (description < nd.description);
            }
        };

        QVector<double> v_meni;    //!< Meniscus values vector
        QVector<double> v_bott;    //!< Bottom values vector
        QVector<double> v_rmsd;    //!< RMSD values vector

        QString filedir;           //!< File directory
        QString fname_load;        //!< Load file name
        QString fname_edit;        //!< Edit file name
        QStringList edtfiles;      //!< Edited files list
        int nedtfs;                //!< Number of edited files
        int ix_best;               //!< Index of best file
        int ix_setfit;             //!< Index of set fit
        bool have3val;             //!< Flag for having 3 values
        bool bott_fit;             //!< Flag for bottom fit
        int idEdit;                //!< Edit ID

        double fit_xvl;            //!< Fit X value (for 2D data)
        double f_meni;             //!< Meniscus value (for 3D data)
        double f_bott;             //!< Bottom value (for 3D data)

        double dy_global;          //!< Global delta Y
        double miny_global;        //!< Global minimum Y
        QString tripleInfo;        //!< Triple information

        int eID_global;            //!< Global experiment ID
        QString eID_updated;       //!< Updated experiment ID

        US_Model model;            //!< Model
        US_Model model_loaded;     //!< Loaded model
        US_Model model_used;       //!< Used model

        US_Noise ri_noise;         //!< RI noise
        US_Noise ti_noise;         //!< TI noise
        QList<int> excludedScans;  //!< List of excluded scans
        US_Solution solution_rec;  //!< Solution record

        US_Math2::SolutionData solution; //!< Solution data
        QVector<QVector<double>> resids; //!< Residuals

        US_SimulationParameters simparams;      //!< Simulation parameters
        QList<US_DataIO::RawData> tsimdats;     //!< List of simulated data
        QList<US_Model> tmodels;                //!< List of models
        QVector<int> kcomps;                    //!< Components

        QStringList noiIDs;       //!< Noise GUIDs
        QStringList noiEdIDs;     //!< Noise edit GUIDs
        QStringList noiMoIDs;     //!< Noise model GUIDs
        QStringList noiTypes;     //!< Noise types
        QStringList modIDs;       //!< Model GUIDs
        QStringList modEdIDs;     //!< Model edit GUIDs
        QStringList modDescs;     //!< Model descriptions

        int thrdone;              //!< Thread done flag

        double density;           //!< Density
        double viscosity;         //!< Viscosity
        double vbar;              //!< Vbar
        double compress;          //!< Compressibility

        QString svbar_global;     //!< Global svbar

        bool manual;              //!< Manual flag
        bool dataLoaded;          //!< Data loaded flag
        bool haveSim;             //!< Simulation available flag
        bool dataLatest;          //!< Data latest flag
        bool buffLoaded;          //!< Buffer loaded flag
        bool cnstvb;              //!< Constant vbar flag
        bool cnstff;              //!< Constant flag
        bool exp_steps;           //!< Experimental steps flag
        bool dat_steps;           //!< Data steps flag
        bool is_dmga_mc;          //!< DMGA MC flag

        QMap<QString, QString> adv_vals; //!< Advanced values map

        int dbg_level;            //!< Debug level
        int nthread;              //!< Number of threads
        int scanCount;            //!< Scan count

        QPoint rpd_pos;           //!< Residuals plot dialog position
        QString FileName_parsed;  //!< Parsed file name

        QString rmsd_global;      //!< Global RMSD

        void simulate_triple(const QString tripleName, QString tripleInfo);  //!< Simulate triple
        bool loadData(QMap<QString, QString>& dataMap);                      //!< Load data
        bool loadModel(QMap<QString, QString>& modelMap);                    //!< Load model
        bool loadNoises(QMap<QString, QString>& noiseMap);                   //!< Load noises
        void loadNoises_whenAbsent(void);                                    //!< Load noises when absent
        int count_noise_auto(US_DataIO::EditedData* edata, US_Model* model, QStringList& noiseList, QStringList& noiTypes); //!< Count noise automatically
        int id_list_db_auto(QString runID);                                  //!< List IDs in the database automatically
        int models_in_edit_auto(QString runID, QStringList& modelList);      //!< List models in edit automatically
        int noises_in_model_auto(QString runID, QStringList& noiseList);     //!< List noises in model automatically
        void simulateModel(QMap<QString, QString>& modelMap);                //!< Simulate model
        void adjustModel(void);                                              //!< Adjust model

        QStringList scan_dbase_models(QStringList modelList);                //!< Scan database models

        QString text_model(US_Model model, int precision);                   //!< Get text representation of model
        QString html_header(QString runID, QString runDesc, US_DataIO::EditedData* edata); //!< Get HTML header
        QString distrib_info(QMap<QString, QString>& distMap);               //!< Get distribution information
        QString calc_replicates_averages(void);                             //!< Calculate replicates averages
        QString get_replicate_group_number(QString replicateID);             //!< Get replicate group number
        QMap<QString, double> get_replicate_group_results(US_ReportGMP::ReportItem reportItem, QString replicateID, QStringList groupResults); //!< Get replicate group results
        void assemble_replicate_av_integration_html(void);                   //!< Assemble replicate average integration HTML

        QString get_filename(QString path);                                  //!< Get file name

    public slots:
                void thread_progress(int progress, int total);                       //!< Slot for thread progress
        void thread_complete(int threadID);                                  //!< Slot for thread completion
        void resplot_done(void);                                             //!< Slot for residuals plot done
        void update_progress(int progress);                                  //!< Slot for updating progress

    private slots:
                void loadRun_auto(QMap<QString, QString>& dataMap);                  //!< Slot for loading run automatically
        void check_failed_triples(void);                                     //!< Check for failed triples
        QMap<QString, QString> read_autoflowAnalysis_record(US_DB2* db, const QString& runID); //!< Read autoflow analysis record
        QMap<QString, QString> read_autoflowAnalysisHistory_record(US_DB2* db, const QString& runID); //!< Read autoflow analysis history record
        void check_models(int modelID);                                      //!< Check models
        void check_for_missing_models(void);                                 //!< Check for missing models
        void check_for_dropped_triples(void);                                //!< Check for dropped triples
        QString compose_html_failed_stage_missing_models(void);              //!< Compose HTML for failed stage missing models
        QString missing_models_msg(void);                                    //!< Get missing models message
        void reset_report_panel(void);                                       //!< Reset report panel
        void view_report(void);                                              //!< View report
        void view_report_db(void);                                           //!< View report from the database
        void load_gmp_report_db(void);                                       //!< Load GMP report from the database
        void load_gmp_run(void);                                             //!< Load GMP run
        void generate_report(void);                                          //!< Generate report
        void changedItem(QTreeWidgetItem* item, int column);                 //!< Slot for changing item
        void select_all(void);                                               //!< Select all items
        void unselect_all(void);                                             //!< Unselect all items
        void expand_all(void);                                               //!< Expand all items
        void collapse_all(void);                                             //!< Collapse all items

        void show_results(QMap<QString, QString>& resultsMap);               //!< Show results
        void calc_residuals(void);                                           //!< Calculate residuals
        void assemble_distrib_html(QMap<QString, QString>& distMap);         //!< Assemble distribution HTML
        void assemble_plots_html(QStringList plotList, QString optionalString = QString("")); //!< Assemble plots HTML
        double interp_sval(double sval, double* xvals, double* yvals, int nvals); //!< Interpolate S-value
        void plotres(QMap<QString, QString>& plotMap);                       //!< Plot residuals
        void plot_pseudo3D(QString triple, QString pseudo3D);                //!< Plot pseudo 3D
        bool modelGuidExistsForStage(QString modelGUID, QString stage);      //!< Check if model GUID exists for stage
        bool modelGuidExistsForStage_ind(QString modelGUID, QString stage, QString triple); //!< Check if model GUID exists for stage individually
        void process_combined_plots(QString plotType);                       //!< Process combined plots
        void process_combined_plots_individual(QString plotType, QString triple); //!< Process combined plots individually

        QMap<QString, QString> read_autoflowGMPReportEsign_record(QString runID); //!< Read autoflow GMP report E-signature record
        void get_assigned_oper_revs(QJsonDocument jsonDoc, QStringList& assignedList); //!< Get assigned operators and reviewers from JSON document

        QString indent(int level) const;                                     //!< Indent string
        QString table_row(const QString& col1, const QString& col2) const;   //!< Create table row with 2 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3) const; //!< Create table row with 3 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4) const; //!< Create table row with 4 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5) const; //!< Create table row with 5 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5, const QString& col6) const; //!< Create table row with 6 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5, const QString& col6, const QString& col7) const; //!< Create table row with 7 columns
        QString table_row(const QString& col1, const QString& col2, const QString& col3, const QString& col4, const QString& col5, const QString& col6, const QString& col7, const QString& col8) const; //!< Create table row with 8 columns

        void paintPage(QPrinter& printer, int pageNumber, int pageCount, QPainter* painter, QTextDocument* doc, const QRectF& textRect, qreal footerHeight); //!< Paint page
        void printDocument(QPrinter& printer, QTextDocument* doc); //!< Print document
        double mmToPixels(QPrinter& printer, int mm); //!< Convert millimeters to pixels

        void write_plot(const QString& filePath, const QwtPlot* plot); //!< Write plot to file
        bool mkdir(const QString& path, const QString& dirName); //!< Make directory

        void distrib_plot_stick(int stickPlotType); //!< Distribution stick plot
        void distrib_plot_2d(int plotType); //!< 2D distribution plot

        void help(void)
        {
            showHelp.show_help("reporter_gmp.html");
        }

        signals:
                void reset_reports(void); //!< Signal to reset reports
};

#endif // US_REPORTER_GMP_H
