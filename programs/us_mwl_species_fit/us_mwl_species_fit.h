//! \file us_mwl_species_fit.h
#ifndef US_MWL_SPECIES_FIT_H
#define US_MWL_SPECIES_FIT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_mwl_sf_plot3d.h"
#include "us_run_protocol.h"
#include "us_protocol_util.h"
#include "../us_analysis_profile/us_analysis_profile.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_MwlSpeciesFit
//! \brief Class for managing and performing multi-wavelength species fit analysis.
class US_MwlSpeciesFit : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_MwlSpeciesFit
        US_MwlSpeciesFit();

        US_MwlSpeciesFit( QMap<QString, QString> &);

        bool us_gmp_auto_mode;
        QMap<QString, QString> protocol_details;
        US_RunProtocol currProto;           //!< Current run protocol
        US_AnalysisProfileGui* sdiag_aprof; //!< Analysis profile GUI dialog
        US_AnaProfile currAProf;            //!< Current analysis profile
        QMap<QString, QMap<QString, US_ReportGMP>> ch_reports; //!< Channel reports
        QStringList chndescs;               //!< Channel descriptions
        QStringList chndescs_alt;           //!< Alternative channel descriptions
        QMap<QString, QList<double>> ch_wvls; //!< Channel wavelengths
        QList<int> analysis_runs;           //!< Analysis runs  
        QList<int> report_runs;             //!< Report runs
  
  //QMap< QString, QString> channels_to_radial_ranges;
        QMap< QString, QMap <QString, QStringList>> editProfile_blc;
        QMap< QString, QMap< QString, QMap< double, double > > > extinction_profiles_per_channel;
             //chann.    //type[protein, DNA]  //ext. profile
        QString rmsd_for_gmp;
        QString chann_to_process_velmwl;

        //! \brief VEL-MWL channel -> Accept/Reject decision ("Accepted"/
        //! "Rejected") made by the user for the current channel this
        //! session. (Whether a channel needs (re-)processing at all --
        //! i.e. whether it already has a decision from a prior session --
        //! is decided one level up, by US_Analysis_auto, before this
        //! class is ever constructed for that channel.)
        QMap<QString, QString> velmwl_channel_decisions;

    private:
        int dbg_level;         //!< Debug level
        int nspecies;          //!< Number of species
        int jspec;             //!< Current species index
        int lmbxs;             //!< Lambda start index
        int lmbxe;             //!< Lambda end index
        int trpxs;             //!< Triple start index
        int trpxe;             //!< Triple end index
        int radxs;             //!< Radius start index
        int radxe;             //!< Radius end index
        int kradii;            //!< Radius index
        int kscan;             //!< Scan index
        int klambda;           //!< Lambda index

        QVector<double> radii;          //!< Radii vector
        QVector<double> spconcs;        //!< Species concentrations vector
        QVector<int> lambdas;           //!< Lambdas vector
        QVector<int> spwavls;           //!< Species wavelengths vector
        QVector<int> nwavls;            //!< Number of wavelengths vector

        QVector<US_DataIO::RawData> synData; //!< Synthetic data vector
        QVector<bool> have_p1;              //!< Flags for p1 availability

        QStringList spfiles;      //!< Species files list
        QStringList celchns;      //!< Cell channels list
        QList<int> ftndxs;        //!< First indices list
        QList<int> ltndxs;        //!< Last indices list
        QVector<QVector<int>> celchn_wvl; //!< Cell channels wavelengths vector

        US_Editor* te_results; //!< Text editor for results

        QPushButton* pb_loadsfit; //!< Load species fit button
        QPushButton* pb_sfitdata; //!< Species fit data button
        QPushButton* pb_prev;     //!< Previous plot button
        QPushButton* pb_next;     //!< Next plot button
        QPushButton* pb_plot3d;   //!< 3D plot button

        QPushButton* pb_reject_velmwl;
        QPushButton* pb_accept_velmwl;

        QLineEdit* le_fit_error; //!< Line edit for fit error

        QVector<SFData> synFitError; //!< Synthetic fit error vector

        //! \brief Plot data
        void data_plot(void);

        //! \brief Get fit error
        void get_fit_error(void);

        //! \brief Persist a VEL-MWL channel's Accept/Reject decision --
        //! and, for an Accept, its deconvolved-edit filename -- into the
        //! run's single autoflowAnalysisVelMwl row (one row per
        //! autoflowID, JSON-keyed by channel). Called once the user
        //! clicks Accept/Reject here; the decision of whether this
        //! channel needed (re-)processing at all is made one level up,
        //! by US_Analysis_auto, before this class is even constructed.
        //!
        //! ALEXEY: The filename write used to happen separately and
        //! later, from US_Analysis_auto::velmwl_deconv_accepted() (a
        //! second, fire-and-forget DB call made after import_ssf_data_
        //! auto()/load_auto_velmwl() ran) -- that left a window where a
        //! channel could be on record as "Accepted" with no filename yet
        //! (or forever, if that second write failed), which process_
        //! velmwl_after_all_channels_decided() then had to defensively
        //! detect and skip. Folding it in here removes that window:
        //! protocol_details["ssf_dir_name"] is already known at the
        //! moment of this call (it's copied into this class's
        //! protocol_details at construction, from the very same map
        //! US_Analysis_auto later reuses), so the filename can go out
        //! atomically with the decision, in the same DB call, instead of
        //! as a second, independent write.
        //!
        //! ALEXEY: First-decision-wins is enforced atomically inside
        //! update_autoflowAnalysisVelMwl_channel_decision() (see
        //! us3_autoflow_procs.sql), not by a separate check here -- a
        //! client-side check-then-write would itself race two sessions
        //! clicking at nearly the same moment. This call reports back
        //! what the DB actually ended up recording for the channel,
        //! which may not be what was just clicked if another session's
        //! decision (and filename) got there first. NOTE: this requires
        //! update_autoflowAnalysisVelMwl_channel_decision() in
        //! us3_autoflow_procs.sql to be extended to accept a filename
        //! parameter and echo back the recorded filename in its second
        //! result set (see the qry/db->next() handling below); that SQL
        //! change is not part of this file.
        //! \param chann              The channel, in "N / X" canonical form.
        //! \param decision           "Accepted" or "Rejected" -- what THIS click asked to record.
        //! \param filename           Deconvolved-edit filename to persist alongside an Accept
        //!                           (pass an empty string for Reject -- there is nothing to store).
        //! \param recorded_decision  [out] "Accepted" or "Rejected" -- what's actually on record now.
        //! \param recorded_decisionByName [out] Who recorded it (blank if this call recorded it and DB lookup of that name isn't needed).
        //! \param recorded_decisionTs     [out] When it was recorded.
        //! \param recorded_filename       [out] Filename actually on record now for this channel
        //!                                (may differ from `filename` if another session's Accept won first).
        //! \return true if THIS call's decision is the one now on record
        //!         (i.e. it won); false if an earlier decision (this or
        //!         another session) already won this channel and this
        //!         click was not recorded.
        bool record_velmwl_channel_decision( QString chann, QString decision,
                                              QString filename,
                                              QString& recorded_decision,
                                              QString& recorded_decisionByName,
                                              QString& recorded_decisionTs,
                                              QString& recorded_filename );

    private slots:
        //! \brief Write report to a text stream
        //! \param ts QTextStream to write the report to
        void write_report(QTextStream& ts);

        //! \brief Load data
        void load(void);

        //! \brief View data
        void view(void);

        //! \brief Save data
        void save(void);

        //! \brief Load species data
        void loadSpecs(void);
        void loadSpecs_auto( QMap< QString, QMap< double, double > > );
        bool read_protocol(QStringList &);
        bool validExtinctionProfile( QString, QList< double >,
				     QList< double >, QStringList& );
        void reject_velmwl( void );
        void accept_velmwl( void );
         
        //! \brief Perform species fit data analysis
        void specFitData(void);

        //! \brief Set new triple index
        //! \param index Index of the new triple
        void new_triple(const int index);

        //! \brief Get triple index
        //! \param index Index to get
        //! \return Triple index
        int triple_index(const int index);

        //! \brief Apply noise to data
        //! \param type Noise type
        //! \param ri_noise RI noise object
        //! \param ti_noise TI noise object
        void apply_noise(const int type, US_Noise& ri_noise, US_Noise& ti_noise);

        //! \brief Plot data (variant 2)
        void plot_data2(void);

        //! \brief Plot data (variant 1)
        void plot_data1(void);

        //! \brief Plot previous data
        void prev_plot(void);

        //! \brief Plot next data
        void next_plot(void);

        //! \brief Plot RMSD in 3D
        void rmsd_3dplot(void);

        //! \brief Show help information
        void help(void)
        { showHelp.show_help("manual/multi-wavelength/mwl_species_fit.html"); };

        void reset();
        void reset_data();
        void reset_gui();
 
    signals:
        void reject_velmwl_s( QString& );
        void accept_velmwl_s( QString& );
};

#endif // US_MWL_SPECIES_FIT_H
