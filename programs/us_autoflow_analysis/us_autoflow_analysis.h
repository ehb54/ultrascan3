//! \file us_autoflow_analysis.h
#ifndef US_ANALYSIS_AUTO_H
#define US_ANALYSIS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "../us_fit_meniscus/us_fit_meniscus.h"
#include "../us_fematch/us_fematch.h"

/**
 * @class US_Analysis_auto
 * @brief This class handles the automatic analysis of ultra-sedimentation data.
 */
class US_Analysis_auto : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for the US_Analysis_auto class.
         */
        US_Analysis_auto();

        QTreeWidget     *treeWidget;                             /**< Tree widget for displaying analysis data. */
        QMap<QString, QTreeWidgetItem *> topItem;                /**< Top-level items in the tree widget. */
        QMap<QString, QTreeWidgetItem *> childItem_2DSA;         /**< Child items for 2DSA analysis. */
        QMap<QString, QTreeWidgetItem *> childItem_2DSA_FM;      /**< Child items for 2DSA-FM analysis. */
        QMap<QString, QTreeWidgetItem *> childItem_2DSA_FITMEN;  /**< Child items for 2DSA-FITMEN analysis. */
        QMap<QString, QTreeWidgetItem *> childItem_2DSA_IT;      /**< Child items for 2DSA-IT analysis. */
        QMap<QString, QTreeWidgetItem *> childItem_2DSA_MC;      /**< Child items for 2DSA-MC analysis. */
        QMap<QString, QTreeWidgetItem *> childItem_PCSA;         /**< Child items for PCSA analysis. */

        QMap<QString, QGroupBox *> groupbox_2DSA;                /**< Group boxes for 2DSA analysis. */
        QMap<QString, QGroupBox *> groupbox_2DSA_FM;             /**< Group boxes for 2DSA-FM analysis. */
        QMap<QString, QGroupBox *> groupbox_2DSA_FITMEN;         /**< Group boxes for 2DSA-FITMEN analysis. */
        QMap<QString, QGroupBox *> groupbox_2DSA_IT;             /**< Group boxes for 2DSA-IT analysis. */
        QMap<QString, QGroupBox *> groupbox_2DSA_MC;             /**< Group boxes for 2DSA-MC analysis. */
        QMap<QString, QGroupBox *> groupbox_PCSA;                /**< Group boxes for PCSA analysis. */

        QTimer * timer_update;                                   /**< Timer for updating the analysis. */
        QTimer * timer_end_process;                              /**< Timer for ending the process. */

        bool in_gui_update;                                      /**< Flag indicating if the GUI is being updated. */
        bool in_reload_end_process;                              /**< Flag indicating if the process is being reloaded. */

        /**
         * @brief Resets the analysis panel for public use.
         */
        void reset_analysis_panel_public ( void );

        US_FitMeniscus* FitMen;                                  /**< Pointer to the FitMeniscus class. */
        QPointer< US_ResidPlotFem > resplotd;                    /**< Pointer to the Residual Plot for FEM data. */

        /**
         * @brief Retrieves the edited data.
         * @return Pointer to the edited data.
         */
        US_DataIO::EditedData* aa_editdata();

        /**
         * @brief Retrieves the simulated data.
         * @return Pointer to the simulated data.
         */
        US_DataIO::RawData* aa_simdata();

        /**
         * @brief Retrieves the exclusion list.
         * @return Pointer to the list of excluded data.
         */
        QList< int >* aa_excllist();

        /**
         * @brief Retrieves the model data.
         * @return Pointer to the model data.
         */
        US_Model* aa_model();

        /**
         * @brief Retrieves the TI noise data.
         * @return Pointer to the TI noise data.
         */
        US_Noise* aa_ti_noise();

        /**
         * @brief Retrieves the RI noise data.
         * @return Pointer to the RI noise data.
         */
        US_Noise* aa_ri_noise();

        /**
         * @brief Retrieves the residual bitmap.
         * @return Pointer to the residual bitmap.
         */
        QPointer< US_ResidsBitmap > aa_resbmap();

        /**
         * @brief Retrieves the triple information.
         * @return Triple information as a string.
         */
        QString aa_tripleInfo();

        QMessageBox * msg_sim;                                   /**< Message box for simulation messages. */
        QProgressDialog * progress_msg;                          /**< Progress dialog for showing progress messages. */

    private:
        QVector< US_DataIO::RawData > rawData;                   /**< Vector of raw data. */
        QVector< US_DataIO::EditedData > editedData;             /**< Vector of edited data. */
        QVector< SP_SPEEDPROFILE > speed_steps;                  /**< Vector of speed profiles. */
        US_DataIO::EditedData* edata;                            /**< Pointer to the edited data. */
        US_DataIO::RawData* rdata;                               /**< Pointer to the raw data. */
        US_DataIO::RawData* sdata;                               /**< Pointer to the simulated data. */
        US_DataIO::RawData wsdata;                               /**< Raw data for the workspace. */
        QPointer< US_ResidsBitmap > rbmapd;                      /**< Pointer to the residual bitmap for the dataset. */

        /**
         * @class ModelDesc
         * @brief This class holds the description of a model.
         */
        class ModelDesc
        {
            public:
                QString description;    /**< Full model description. */
                QString baseDescr;      /**< Base analysis-set description. */
                QString fitfname;       /**< Associated fit file name. */
                QString modelID;        /**< Model DB ID. */
                QString modelGUID;      /**< Model GUID. */
                QString filepath;       /**< Full path model file name. */
                QString editID;         /**< Edit parent DB ID. */
                QString editGUID;       /**< Edit parent GUID. */
                QString antime;         /**< Analysis date & time (yymmddHHMM). */
                QDateTime lmtime;       /**< Record last modified date & time. */
                double variance;        /**< Variance value. */
                double meniscus;        /**< Meniscus radius value. */
                double bottom;          /**< Bottom radius value. */

                /**
                 * @brief Less than operator to enable sorting.
                 * @param md Another ModelDesc object.
                 * @return True if this object is less than the other.
                 */
                bool operator< ( const ModelDesc& md ) const { return ( description < md.description ); }
            };

            /**
             * @class NoiseDesc
             * @brief This class holds the description of a noise.
             */
            class NoiseDesc
            {
                public:
                    QString description;    /**< Full noise description. */
                    QString baseDescr;      /**< Base analysis-set description. */
                    QString noiseID;        /**< Noise DB ID. */
                    QString noiseGUID;      /**< Noise GUID. */
                    QString filepath;       /**< Full path noise file name. */
                    QString modelID;        /**< Model parent DB ID. */
                    QString modelGUID;      /**< Model parent GUID. */
                    QString antime;         /**< Analysis date & time (yymmddHHMM). */

                    /**
                     * @brief Less than operator to enable sorting.
                     * @param nd Another NoiseDesc object.
                     * @return True if this object is less than the other.
                     */
                    bool operator< ( const NoiseDesc& nd ) const { return ( description < nd.description ); }
            };

            QMap < QString, QString > protocol_details_at_analysis;  /**< Protocol details at the time of analysis. */

            bool fitmen_bad_vals;                                    /**< Flag for bad FitMeniscus values. */
            bool no_fm_data_auto;                                    /**< Flag indicating no FitMeniscus data automatically. */

            QProgressDialog * progress_msg_fmb;                      /**< Progress dialog for FitMeniscus. */
            QVector< double > v_meni;                                /**< Vector of meniscus values. */
            QVector< double > v_bott;                                /**< Vector of bottom values. */
            QVector< double > v_rmsd;                                /**< Vector of RMSD values. */

            QString filedir;                                         /**< Directory of the file. */
            QString fname_load;                                      /**< File name to load. */
            QString fname_edit;                                      /**< File name for editing. */
            QStringList edtfiles;                                    /**< List of edited files. */
            int nedtfs;                                              /**< Number of edited files. */
            int ix_best;                                             /**< Index of the best file. */
            int ix_setfit;                                           /**< Index of the set fit file. */
            bool have3val;                                           /**< Flag indicating if 3D values are present. */
            bool bott_fit;                                           /**< Flag indicating if bottom fit is present. */
            int idEdit;                                              /**< Edit ID. */

            double fit_xvl;                                          /**< Fit X value for 2D data. */
            double f_meni;                                           /**< Fit meniscus value for 3D data. */
            double f_bott;                                           /**< Fit bottom value for 3D data. */

            double dy_global;                                        /**< Global Y difference. */
            double miny_global;                                      /**< Global minimum Y value. */

            QString tripleInfo;                                      /**< Information about the triple. */

            int eID_global;                                          /**< Global Edit ID. */

            US_Model model;                                          /**< Model data. */
            US_Model model_loaded;                                   /**< Loaded model data. */
            US_Model model_used;                                     /**< Used model data. */

            US_Noise ri_noise;                                       /**< RI noise data. */
            US_Noise ti_noise;                                       /**< TI noise data. */
            QList< int > excludedScans;                              /**< List of excluded scans. */
            US_Solution solution_rec;                                /**< Solution record. */

            US_Math2::SolutionData solution;                         /**< Solution data. */
            QVector< QVector< double > > resids;                     /**< Residuals data. */

            US_SimulationParameters simparams;                       /**< Simulation parameters. */
            QList< US_DataIO::RawData > tsimdats;                    /**< List of simulated raw data. */
            QList< US_Model > tmodels;                               /**< List of models. */
            QVector< int > kcomps;                                   /**< Vector of components. */

            QStringList noiIDs;                                      /**< List of noise IDs. */
            QStringList noiEdIDs;                                    /**< List of noise edit IDs. */
            QStringList noiMoIDs;                                    /**< List of noise model IDs. */
            QStringList noiTypes;                                    /**< List of noise types. */
            QStringList modIDs;                                      /**< List of model IDs. */
            QStringList modEdIDs;                                    /**< List of model edit IDs. */
            QStringList modDescs;                                    /**< List of model descriptions. */

            int thrdone;                                             /**< Thread done flag. */

            double density;                                          /**< Density value. */
            double viscosity;                                        /**< Viscosity value. */
            double vbar;                                             /**< Vbar value. */
            double compress;                                         /**< Compressibility value. */

            QString svbar_global;                                    /**< Global Vbar value. */

            bool manual;                                             /**< Manual mode flag. */
            bool dataLoaded;                                         /**< Data loaded flag. */
            bool haveSim;                                            /**< Simulation data present flag. */
            bool dataLatest;                                         /**< Latest data flag. */
            bool buffLoaded;                                         /**< Buffer loaded flag. */
            bool cnstvb;                                             /**< Constant Vbar flag. */
            bool cnstff;                                             /**< Constant FF flag. */
            bool exp_steps;                                          /**< Experimental steps flag. */
            bool dat_steps;                                          /**< Data steps flag. */
            bool is_dmga_mc;                                         /**< DMGA Monte Carlo flag. */

            QMap< QString, QString > adv_vals;                       /**< Advanced values. */

            int dbg_level;                                           /**< Debug level. */
            int nthread;                                             /**< Number of threads. */
            int scanCount;                                           /**< Scan count. */

            QPoint rpd_pos;                                          /**< Residual plot position. */

            QPushButton* pb_show_all;                                /**< Show all button. */
            QPushButton* pb_hide_all;                                /**< Hide all button. */

            bool all_processed;                                      /**< All processed flag. */

            QString AProfileGUID;                                    /**< Analysis profile GUID. */
            QString ProtocolName_auto;                               /**< Protocol name for auto analysis. */
            int invID;                                               /**< Investigator ID. */
            QString analysisIDs;                                     /**< Analysis IDs. */
            QMap <QString, QString > investigator_details;           /**< Investigator details. */
            QString defaultDB;                                       /**< Default database. */
            int autoflowStatusID;                                    /**< Auto flow status ID. */
            int autoflowID_passed;                                   /**< Passed auto flow ID. */

            QString FileName;                                        /**< File name. */
            QString FileName_parsed;                                 /**< Parsed file name. */

            int sim_msg_pos_x;                                       /**< X position of simulation message. */
            int sim_msg_pos_y;                                       /**< Y position of simulation message. */

            QVector< QString > Array_of_triples;                     /**< Array of triples. */

            QMap < QString, QMap< QString, QString > > Array_of_analysis; /**< Array of analysis data. */
            QMap < QString, QMap< QString, QString > > Array_of_analysis_by_requestID; /**< Array of analysis by request ID. */
            QMap < QString, QStringList > Channel_to_requestIDs;     /**< Mapping of channels to request IDs. */

            QStringList channels_all;                                /**< List of all channels. */

            QMap < QString, bool > Manual_update;                    /**< Manual update flag. */
            QMap < QString, bool > History_read;                     /**< History read flag. */
            QMap < QString, bool > Completed_triples;                /**< Completed triples flag. */
            QMap < QString, bool > Failed_triples;                   /**< Failed triples flag. */
            QMap < QString, bool > Canceled_triples;                 /**< Canceled triples flag. */
            QMap < QString, bool > Process_2dsafm;                   /**< Process 2DSA-FM flag. */

            /**
             * @brief Reads the autoflow analysis record from the database.
             * @param db Pointer to the database.
             * @param analysisID Analysis ID.
             * @return A map of the autoflow analysis record.
             */
            QMap < QString, QString > read_autoflowAnalysis_record( US_DB2*, const QString& );

            /**
             * @brief Reads the autoflow analysis history record from the database.
             * @param db Pointer to the database.
             * @param analysisID Analysis ID.
             * @return A map of the autoflow analysis history record.
             */
            QMap < QString, QString > read_autoflowAnalysisHistory_record( US_DB2*, const QString& );

            /**
             * @brief Retrieves investigator information from the database.
             * @param db Pointer to the database.
             * @param invID Investigator ID.
             * @return A map of the investigator information.
             */
            QMap < QString, QString > get_investigator_info ( US_DB2*, const QString& );

            /**
             * @brief Scans the database for analysis data.
             * @param analysisData A map to hold the analysis data.
             */
            void scan_dbase_auto( QMap <QString, QString> & );

            /**
             * @brief Copies the edit profile.
             * @param profile A map to hold the profile data.
             */
            void get_editProfile_copy( QMap < QString, QString >& );

            /**
             * @brief Loads the file data automatically.
             * @param fileData A map to hold the file data.
             * @return True if the file was loaded successfully, false otherwise.
             */
            bool file_loaded_auto( QMap < QString, QString >& );

            /**
             * @brief Loads the data automatically.
             * @param fileName The name of the file to load.
             */
            void load_data_auto( const QString& );

            /**
             * @brief Processes the 2D data.
             */
            void process_2d( void );

            /**
             * @brief Processes the 3D data.
             */
            void process_3d( void );

            /**
             * @brief Updates the edit profile automatically.
             * @param editData A map of the edit data.
             */
            void edit_update_auto( QMap < QString, QString >& );

            /**
             * @brief Reads the autoflow analysis stages from the database.
             * @param analysisID The analysis ID.
             * @return The number of stages read.
             */
            int read_autoflowAnalysisStages( const QString& );

            /**
             * @brief Reverts the autoflow analysis stages record in the database.
             * @param analysisID The analysis ID.
             */
            void revert_autoflow_analysis_stages_record( const QString& );

            /**
             * @brief Updates the database edit record.
             * @param editID The edit ID.
             * @param editGUID The edit GUID.
             */
            void update_db_edit( QString, QString );

            /**
             * @brief Removes the models automatically.
             * @param modelID The model ID.
             */
            void remove_models_auto( QString );

            /**
             * @brief Indexes the model set fit.
             */
            void index_model_setfit( void );

            /**
             * @brief Retrieves the noises in the edit profile.
             * @param editID The edit ID.
             * @param noiseIDs A list of noise IDs.
             * @param noiseEdIDs A list of noise edit IDs.
             * @param noiseMoIDs A list of noise model IDs.
             * @param modelID The model ID.
             */
            void noises_in_edit( QString, QStringList&, QStringList&, QStringList&, QString );

            /**
             * @brief Updates the autoflow analysis status at fit meniscus stage.
             * @param db Pointer to the database.
             * @param analysisIDs List of analysis IDs.
             */
            void update_autoflowAnalysis_status_at_fitmen ( US_DB2*, const QStringList& );

            /**
             * @brief Records or updates the analysis meniscus status in the database.
             * @param db Pointer to the database.
             * @param analysisID The analysis ID.
             * @param status The status to update.
             */
            void record_or_update_analysis_meniscus_status ( US_DB2*, QString, QString );

            /**
             * @brief Records or updates the analysis cancel status in the database.
             * @param db Pointer to the database.
             * @param analysisID The analysis ID.
             * @param status The status to update.
             * @param reason The reason for the cancellation.
             */
            void record_or_update_analysis_cancel_status ( US_DB2*, QString, QString, QString );

            /**
             * @brief Updates the autoflow analysis upon deletion of a record.
             * @param db Pointer to the database.
             * @param analysisID The analysis ID.
             */
            void update_autoflowAnalysis_uponDeletion ( US_DB2*, const QString& );

            /**
             * @brief Updates the autoflow analysis upon deletion of other wavelengths.
             * @param db Pointer to the database.
             * @param analysisIDs List of analysis IDs.
             */
            void update_autoflowAnalysis_uponDeletion_other_wvl ( US_DB2*, const QStringList& );

            /**
             * @brief Loads the data from the given map.
             * @param dataMap A map containing the data to load.
             * @return True if the data was loaded successfully, false otherwise.
             */
            bool loadData( QMap < QString, QString > & );

            /**
             * @brief Loads the model data from the given map.
             * @param dataMap A map containing the model data to load.
             * @return True if the model data was loaded successfully, false otherwise.
             */
            bool loadModel( QMap < QString, QString > & );

            /**
             * @brief Loads the noise data from the given map.
             * @param dataMap A map containing the noise data to load.
             * @return True if the noise data was loaded successfully, false otherwise.
             */
            bool loadNoises( QMap < QString, QString > & );

            /**
             * @brief Loads the noises when they are absent.
             */
            void loadNoises_whenAbsent ( void );

            /**
             * @brief Counts the noise automatically.
             * @param editData Pointer to the edited data.
             * @param modelData Pointer to the model data.
             * @param noiseIDs List of noise IDs.
             * @param noiseTypes List of noise types.
             * @return The number of noises counted.
             */
            int count_noise_auto( US_DataIO::EditedData*, US_Model*, QStringList&, QStringList& );

            /**
             * @brief Retrieves the list of IDs from the database automatically.
             * @param idType The type of ID to retrieve.
             * @return The number of IDs retrieved.
             */
            int id_list_db_auto( QString );

            /**
             * @brief Retrieves the models in the edit profile automatically.
             * @param editID The edit ID.
             * @param modelIDs List of model IDs.
             * @return The number of models retrieved.
             */
            int models_in_edit_auto( QString, QStringList& );

            /**
             * @brief Retrieves the noises in the model profile automatically.
             * @param modelID The model ID.
             * @param noiseIDs List of noise IDs.
             * @return The number of noises retrieved.
             */
            int noises_in_model_auto( QString, QStringList& );

            /**
             * @brief Retrieves the file name.
             * @param fileID The file ID.
             * @return The file name.
             */
            QString get_filename( QString );

            /**
             * @brief Simulates the model data.
             */
            void simulateModel( void );

            /**
             * @brief Adjusts the model data.
             */
            void adjustModel( void );

            /**
             * @brief Creates a group box with the given title.
             * @param title The title of the group box.
             * @return Pointer to the created group box.
             */
            QGroupBox *createGroup( QString &);

            QSignalMapper *signalMapper;                             /**< Signal mapper for signals. */
            QSignalMapper *signalMapper_overlay;                     /**< Signal mapper for overlay signals. */

            // 2DSA
            bool job1run;                                            /**< Run 1 (2DSA) run flag. */
            bool job2run;                                            /**< Run 2 (2DSA-FM) run flag. */
            bool job3run;                                            /**< Run 3 (FitMeniscus) run flag. */
            bool job4run;                                            /**< Run 4 (2DSA-IT) run flag. */
            bool job5run;                                            /**< Run 5 (2DSA-MC) run flag. */
            bool job3auto;                                           /**< FitMeniscus auto-run flag. */
            bool job6run_pcsa;                                       /**< Run 6 (PCSA) run flag. */
            QString job1nois;                                        /**< 2DSA noise type. */
            QString job2nois;                                        /**< 2DSA-FM noise type. */
            QString job4nois;                                        /**< 2DSA-IT noise type. */

    public slots:
        /**
         * @brief Slot to handle thread progress updates.
         * @param threadID The ID of the thread.
         * @param progress The progress value.
         */
        void thread_progress( int, int );

        /**
         * @brief Slot to handle thread completion.
         * @param threadID The ID of the completed thread.
         */
        void thread_complete( int );

        /**
         * @brief Slot to handle the completion of the residual plot.
         */
        void resplot_done( void );

        /**
         * @brief Slot to update the progress.
         * @param progress The progress value.
         */
        void update_progress( int );

    private slots:
        /**
         * @brief Initializes the panel with the given data.
         * @param panelData A map containing the panel data.
         */
        void initPanel( QMap < QString, QString > & );

        /**
         * @brief Shows all elements in the UI.
         */
        void show_all( void );

        /**
         * @brief Hides all elements in the UI.
         */
        void hide_all( void );

        /**
         * @brief Updates the GUI.
         */
        void gui_update ( void );

        /**
         * @brief Updates the GUI temporarily.
         */
        void gui_update_temp ( void );

        /**
         * @brief Resets the analysis panel.
         */
        void reset_analysis_panel ( void );

        /**
         * @brief Ends the process.
         */
        void end_process( void );

        /**
         * @brief Resets the auto analysis.
         */
        void reset_auto ( void );

        /**
         * @brief Updates the autoflow analysis statuses.
         * @param analysisData A map containing the analysis data.
         */
        void update_autoflowAnalysis_statuses( QMap < QString, QString > & );

        /**
         * @brief Updates the edit profiles that were updated earlier.
         */
        void editProfiles_updated_earlier( void );

        /**
         * @brief Processes the triple analysis.
         */
        void triple_analysis_processed( void );

        /**
         * @brief Updates the autoflow record at the time of analysis.
         */
        void update_autoflow_record_atAnalysis( void );

        /**
         * @brief Checks the fit meniscus status.
         * @param analysisID The analysis ID.
         * @param status The status to check.
         * @return True if the status is valid, false otherwise.
         */
        bool check_fitmen_status( const QString&, const QString& );

        /**
         * @brief Deletes the job with the given ID.
         * @param jobID The job ID.
         */
        void delete_job ( QString );

        /**
         * @brief Deletes the jobs at the fit meniscus stage.
         * @param jobData A map containing the job data.
         */
        void delete_jobs_at_fitmen ( QMap< QString, QString > & );

        /**
         * @brief Shows the overlay for the given analysis.
         * @param analysisID The analysis ID.
         */
        void show_overlay( QString );

        /**
         * @brief Shows the results.
         */
        void show_results ( void );

        /**
         * @brief Calculates the residuals.
         */
        void calc_residuals( void );

        /**
         * @brief Interpolates the S value.
         * @param x The X value.
         * @param xVals Pointer to the array of X values.
         * @param yVals Pointer to the array of Y values.
         * @param n The number of values.
         * @return The interpolated S value.
         */
        double interp_sval( double, double*, double*, int );

        /**
         * @brief Plots the residuals.
         */
        void plotres( void );

    signals:
        /**
         * @brief Signal emitted when the analysis update process is stopped.
         */
        void analysis_update_process_stopped( void );

        /**
         * @brief Signal emitted when the analysis setup message is closed.
         */
        void close_analysissetup_msg( void );

        /**
         * @brief Signal emitted when the analysis is returned to the initial autoflow stage.
         */
        void analysis_back_to_initAutoflow( void );

        /**
         * @brief Signal emitted when the automatic analysis is complete.
         * @param analysisData A map containing the analysis data.
         */
        void analysis_complete_auto( QMap <QString, QString> & );
};

#endif
