//! \file us_2dsa.h
//! \brief Contains the declaration of the US_2dsa class and its members.
#ifndef US_2DSA_H
#define US_2DSA_H

#include "us_plot_control_2d.h"
#include "us_analysis_control_2d.h"
#include "us_noise_loader.h"
#include "us_resplot_2d.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_solve_sim.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_model_loader.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_analyte.h"
#include "qwt_plot_marker.h"
#include "us_analysis_base2.h"
#include "us_show_norm.h"

#ifndef DbgLv
//! \def DbgLv(a)
//! \brief Macro for debug level logging.
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

#ifndef SP_SPEEDPROFILE
//! \def SP_SPEEDPROFILE
//! \brief Alias for US_SimulationParameters::SpeedProfile.
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif
#ifndef SS_DATASET
//! \def SS_DATASET
//! \brief Alias for US_SolveSim::DataSet.
#define SS_DATASET US_SolveSim::DataSet
#endif

//! \class US_2dsa
//! \brief A class for 2-dimensional spectrum analysis.
class US_2dsa : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_2dsa class.
        US_2dsa();

        //! \brief Auto-mode constructor -- used by US_Analysis_auto's
        //! VELOCITY-MWL post-processing (process_velmwl_after_all_
        //! channels_decided(), via start_next_2dsa_channel(), in
        //! us_autoflow_analysis.cpp) to run a 2DSA-IT analysis, headlessly,
        //! for a single Approved VEL-MWL channel's deconvolved data.
        //! Mirrors US_MwlSpeciesFit's auto constructor: this one loads the
        //! channel's edited data (load(), via a protocol_details-driven
        //! US_DataLoader -- see US_MwlSpeciesFit::load()'s us_gmp_auto_mode
        //! branch for the pattern), then runs the fit and saves results,
        //! all synchronously, emitting twodsa_complete_s() when done. No
        //! GUI interaction is required; the widget is not intended to stay
        //! visible (the caller may still add/show it in a panel purely to
        //! keep plots/reports inspectable, exactly as US_MwlSpeciesFit's
        //! dialog is left up after its own auto constructor runs).
        //! \param protocol_details_p Run-wide + per-channel protocol
        //!        details (same map threaded through the VEL-MWL pipeline
        //!        -- see US_Analysis_auto::protocol_details_at_analysis_
        //!        velmwl). Must include at least:
        //!          "chan_to_analyse"  -- the channel, "N / X" canonical form
        //!          "autoflowID"       -- this run's autoflowID
        //!          "invID_passed"     -- investigator ID
        //!        plus whatever US_DataLoader's protocol_details-driven
        //!        constructor needs to locate/load THIS channel's
        //!        deconvolved edit (e.g. runID/editID or the ssf/edit
        //!        directory info already threaded through by
        //!        US_MwlSpeciesFit/US_Edit -- see get_ssf_dir_and_saveDB()
        //!        and US_Edit::load_auto_velmwl() in us_autoflow_analysis.cpp).
        US_2dsa( QMap<QString, QString> & protocol_details_p );

        //! \brief Function to handle analysis completion.
        //! \param status The status of the analysis.
        void analysis_done(int status);

        //! \brief Function to handle the completion of residual plot.
        void resplot_done(void);

        //! \brief Get the edited data.
        //! \return Pointer to US_DataIO::EditedData.
        US_DataIO::EditedData* mw_editdata();

        //! \brief Get the simulated data.
        //! \return Pointer to US_DataIO::RawData.
        US_DataIO::RawData* mw_simdata();

        //! \brief Get the residual data.
        //! \return Pointer to US_DataIO::RawData.
        US_DataIO::RawData* mw_resdata();

        //! \brief Get the list of excluded data.
        //! \return Pointer to QList of integers.
        QList<int>* mw_excllist();

        //! \brief Get the model.
        //! \return Pointer to US_Model.
        US_Model* mw_model();

        //! \brief Get the time-invariant noise.
        //! \return Pointer to US_Noise.
        US_Noise* mw_ti_noise();

        //! \brief Get the radially invariant noise.
        //! \return Pointer to US_Noise.
        US_Noise* mw_ri_noise();

        //! \brief Get the normalized A values.
        //! \return Pointer to QVector of doubles.
        QVector<double>* mw_Anorm();

        //! \brief Get the status text.
        //! \return QPointer to QTextEdit.
        QPointer<QTextEdit> mw_status_text();

        //! \brief Get the base RSS.
        //! \return Pointer to an integer.
        int* mw_base_rss();

        //! \brief Get the temporary ID name.
        //! \return QString of the temporary ID name.
        QString temp_Id_name();

        //! \brief true when this instance was constructed via the
        //! protocol_details-taking (auto) constructor -- gates the
        //! headless load()/fit/save path exactly as US_MwlSpeciesFit::
        //! us_gmp_auto_mode gates its own auto path.
        bool us_gmp_auto_mode;

        //! \brief Run-wide + per-channel protocol details, passed in via
        //! the auto constructor. See US_MwlSpeciesFit::protocol_details
        //! for the analogous field.
        QMap<QString, QString> protocol_details;

        //! \brief The VEL-MWL channel ("N / X" canonical form) this
        //! instance is auto-processing -- protocol_details["chan_to_analyse"],
        //! cached for convenience/reporting (see twodsa_complete_s()).
        QString chann_to_process_2dsa;

        //! \brief Index into dataList/lw_triples of the deconvolved
        //! species (e.g. S/1, S/2 -- US_MwlSpeciesFit's per-species
        //! output for this one physical channel) currently being
        //! processed by run_2dsa_auto(), within THIS channel's US_2dsa
        //! instance. A channel Approved at the VEL-MWL stage can
        //! resolve to more than one species, each needing its own
        //! independent 2DSA-IT fit; this cursor is what lets a single
        //! US_2dsa instance walk all of them sequentially -- with the
        //! same (default) US_AnalysisControl2D fit settings each time --
        //! before finally reporting the whole channel done. Advanced by
        //! analysis_done()'s auto-mode branch after each species'
        //! fit+save; twodsa_complete_s() is only emitted once
        //! auto_triple_idx reaches dataList.size().
        int auto_triple_idx;

        //! \brief cell+channel+wavelength ("tripleID", same string save()
        //! uses to label reports/models -- e.g. "2S1") of the triple most
        //! recently handed to US_AnalysisControl2D::fit_auto() by
        //! run_2dsa_auto(). Used purely as a regression guard: if the
        //! NEXT species' tripleID comes back identical to this one, the
        //! triple selection failed to actually advance (e.g. lw_triples'
        //! current row / edata got reset to a previous species by
        //! something -- such as mw_editdata() re-deriving edata from
        //! lw_triples->currentRow() -- before fit_auto() used it), and
        //! run_2dsa_auto() aborts loudly instead of silently re-fitting
        //! and re-saving the same triple under a new model number. Empty
        //! before the first species of a channel.
        QString auto_last_tripleID;

    private:
        QGridLayout* progressLayout; //!< Layout for progress display.

        US_Editor* te_results; //!< Text editor for results.

        SS_DATASET dset; //!< Dataset for solving simulation.

        QList<SS_DATASET*> dsets; //!< List of datasets.

        QVector<SP_SPEEDPROFILE> speed_steps; //!< Speed steps for the simulation.
        QVector<double> normvA; //!< Normalized A values.

        US_DataIO::EditedData* edata; //!< Edited data.
        US_DataIO::RawData sdata; //!< Simulated data.
        US_DataIO::RawData rdata; //!< Residual data.

        QPointer<US_ResidPlot2D> resplotd; //!< Residual plot 2D.
        QPointer<US_PlotControl2D> eplotcd; //!< Plot control 2D.
        QPointer<US_AnalysisControl2D> analcd; //!< Analysis control 2D.
        QPointer<US_show_norm> analcd1; //!< Show normalized data.

        US_Model model; //!< Model for the analysis.
        US_Noise ri_noise_in; //!< Radially invariant noise input.
        US_Noise ti_noise_in; //!< Time-invariant noise input.

        QList<US_Model> models; //!< List of models.
        QList<US_Noise> ri_noises; //!< List of radially invariant noises.
        QList<US_Noise> ti_noises; //!< List of time-invariant noises.

        QPoint rbd_pos; //!< Position for residual plot dialog.
        QPoint epd_pos; //!< Position for plot control dialog.
        QPoint acd_pos; //!< Position for analysis control dialog.

        QLineEdit* le_vari; //!< Line edit for variance.
        QLineEdit* le_rmsd; //!< Line edit for RMSD.

        QTextEdit* te_status; //!< Text edit for status.

        QPushButton* pb_fitcntl; //!< Button for fit control.
        QPushButton* pb_plt3d; //!< Button for 3D plot.
        QPushButton* pb_pltres; //!< Button for residual plot.
        QPushButton* pb_shownorm; //!< Button to show normalized data.

        bool loadDB; //!< Flag for loading from database.
        bool exp_steps; //!< Flag for experimental steps.

        double rmsd; //!< Root mean square deviation.

        int dbg_level; //!< Debug level.
        int baserss; //!< Base RSS value.

        //! \brief Shared dataset/simparams setup for row `drow` of
        //! dataList, factored out of open_fitcntl() so both the
        //! interactive Fit Control path and the headless auto path
        //! (run_2dsa_auto()) build `dset` identically. Returns false (and
        //! leaves dset unchanged) if drow is out of range.
        //! \param drow Index into dataList/lw_triples of the triple to fit.
        bool prep_fit_dataset( int drow );

        //! \brief Headless equivalent of open_fitcntl() + a user's "Start
        //! Fit"/Save click, for us_gmp_auto_mode. Calls prep_fit_dataset()
        //! for the (single) loaded triple, then constructs
        //! US_AnalysisControl2D and calls its fit_auto() -- which runs a
        //! full uniform-grid 2DSA-IT fit at that dialog's default
        //! parameters (no GUI shown) and, on completion, calls back into
        //! THIS widget's existing analysis_done( 2 ) itself (its
        //! completed_process()'s auto_mode branch does what a user's
        //! "Save Results" click would). That callback is what triggers
        //! save() and, from there, twodsa_complete_s() below -- no new
        //! completion-signal plumbing is needed on the US_2dsa side.
        void run_2dsa_auto( void );

        //! \brief Override of US_AnalysisBase2::update() -- called by
        //! new_triple() (itself triggered by lw_triples->setCurrentRow()
        //! in run_2dsa_auto()) to refresh runID/solution/buffer state for
        //! the newly-selected triple. In us_gmp_auto_mode, the base
        //! implementation's two QMessageBox::warning() calls (on a
        //! failed solution/buffer fetch for that triple) would block the
        //! headless pipeline waiting on a human to click OK -- the same
        //! class of problem save()'s QMessageBox::information() had. We
        //! can't gate those at the source without editing the shared
        //! US_AnalysisBase2::update() itself, so instead this override
        //! runs the base implementation but auto-dismisses (and logs)
        //! any QMessageBox it raises while us_gmp_auto_mode is true,
        //! rather than duplicating update()'s solution/buffer-fetch
        //! logic here.
        void update( int selection ) override;

    private slots:
        //! \brief Slot to open the residual plot.
        void open_resplot(void);

        //! \brief Slot to open the 3D plot.
        void open_3dplot(void);

        //! \brief Slot to open the fit control.
        void open_fitcntl(void);

        //! \brief Get the distribution information.
        //! \return QString containing distribution information.
        QString distrib_info(void);

        //! \brief Get the iteration information.
        //! \return QString containing iteration information.
        QString iteration_info(void);

        //! \brief Get the meniscus fitting data.
        //! \return QString containing meniscus fitting data.
        QString fit_meniscus_data(void);

        //! \brief Slot to plot the data.
        void data_plot(void);

        //! \brief Write the report to a text stream.
        //! \param stream Reference to QTextStream.
        void write_report(QTextStream& stream);

        //! \brief Write the bmap to a file.
        //! \param filename The name of the file.
        void write_bmap(const QString filename);

        //! \brief Load the data.
        void load(void);

        //! \brief View the data.
        void view(void);

        //! \brief Save the data.
        void save(void);

        //! \brief Slot for new triple.
        //! \param index The index of the new triple.
        void new_triple(int index);

        //! \brief Close all windows.
        void close_all(void);

        //! \brief Show help documentation.
        void help(void) { showHelp.show_help("manual/2dsa/index.html"); };

        void reset();

        void reset_data();
        void reset_gui();

    signals:
        //! \brief Emitted once, at the end of the headless auto path
        //! (run_2dsa_auto() -> analysis_done( 2 ) -> save()), reporting
        //! this channel's 2DSA-IT fit+save outcome back to the caller
        //! (US_Analysis_auto::twodsa_channel_complete(), which advances
        //! to the next Approved channel -- see start_next_2dsa_channel()
        //! in us_autoflow_analysis.cpp). Mirrors US_MwlSpeciesFit's
        //! accept_velmwl_s()/reject_velmwl_s() signals. Never emitted for
        //! interactively-constructed (non-auto) instances.
        //! \param chann   The channel just processed, "N / X" canonical form.
        //! \param success false if the fit or save step failed -- the
        //!        caller should treat this channel as unresolved rather
        //!        than advancing past it silently.
        void twodsa_complete_s( QString& chann, bool success );
};

#endif // US_2DSA_H