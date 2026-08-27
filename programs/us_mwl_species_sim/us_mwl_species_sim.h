//! \file us_mwl_species_sim.h
#ifndef US_MWL_SPECIES_SIM_H
#define US_MWL_SPECIES_SIM_H

#include "us_extern.h"
#include "us_simparms.h"
#include "us_sim_params_gui.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_simparms.h"
#include "us_rotor_gui.h"
#include "us_buffer_gui.h"
#include "us_model_loader.h"
#include "us_plot.h"
#include "us_colorgradIO.h"
#include "us_spectrodata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_MwlSpeciesSim
//! \brief Class for simulating multi-wavelength species data.
class US_MwlSpeciesSim : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_MwlSpeciesSim
        US_MwlSpeciesSim();

        //! \brief Run the simulation headlessly using command-line options.
        //! \param flags Parsed input, output, and execution options.
        //! \return 0 on success, 1 if the GUI is required, or 2 on error.
        int init_from_args(const QMap<QString, QString>& flags);

        //! \brief The wavelength a per-wavelength model description names.
        //! \param description A model description whose third-from-last
        //!        dot-separated field carries the wavelength, as the temporary
        //!        models a multi-wavelength run derives all do.
        //! \return The three-digit wavelength, or an empty string if the
        //!         description does not carry one.
        static QString model_wavelength(const QString& description);
        //! \brief The cell of a model, from its description's triple field.
        static QString model_cell(const QString& description);
        //! \brief The channel letter of a model, from its description's triple field.
        static QString model_channel(const QString& description);

        //! \brief The wavelength field of one dataset's edit filename.
        //!
        //! Each edit names its dataset's plain numeric wavelength. Wavelength
        //! bands and "@" composite keys are internal lookup forms, not disk
        //! filename fields.
        //!
        //! \param descriptions Every model description in the run, in order.
        //! \param index Which of them this edit file is being written for.
        //! \return The filename field, empty if index is out of range.
        static QString edit_wavelength_field(const QStringList& descriptions,
                                             int index);

    private:
        //! \brief Load and aggregate models without displaying a selection dialog.
        bool load_models_from_paths(const QStringList& paths);

        //! \brief Save simulations to save_dir instead of US_Settings::importDir().
        bool save_sims_to(const QString& save_dir);
        bool write_edit_files(const QString& impdir, const QString& cell,
                              const QString& channel);

        //! \brief Write the experiment and solution records a database load
        //! needs, through the same utils writers us_astfem_sim uses.
        //! \param impdir The import directory the run was saved into.
        //! \param cell   The cell the run's datasets belong to.
        //! \return true if both documents were written.
        bool write_experiment_record(const QString& impdir, const QString& cell,
                                    const QString& channel);

        //! \brief Add the simparams noise terms to one wavelength's data.
        //! Ported from us_astfem_sim's finish(), which applies the same four
        //! terms in this same order to every speed step of one simulation.
        //! Here the unit is one wavelength's dataset instead -- see the
        //! comment on the definition for what is shared between wavelengths
        //! and what is not.
        //! \param rdata      The padded, clipped data for one wavelength.
        //! \param total_conc Total signal concentration of that wavelength's
        //!                   model; every sigma is a percentage of it.
        void apply_noise( US_DataIO::RawData& rdata, double total_conc );

        //! \brief Scale each loaded model's signal concentration by that
        //! component's extinction coefficient, so the wavelengths of a run
        //! differ in amplitude the way an absorbance spectrum makes them
        //! differ. Call after the models are loaded and before their total
        //! concentrations are summed. A model set carrying no extinction
        //! data is left exactly as it was.
        void apply_extinction_scaling( void );

        void add_ri_noise    ( US_DataIO::RawData& rdata, double total_conc );
        void add_baseline    ( US_DataIO::RawData& rdata );
        void add_random_noise( US_DataIO::RawData& rdata, double total_conc );
        void add_ti_noise    ( US_DataIO::RawData& rdata, double total_conc );

        //! Unit-sigma systematic noise, drawn once for the run and reused by
        //! every wavelength, each scaling it by its own concentration. Both
        //! are built on first use and cleared by start_sims(). Empty whenever
        //! the corresponding simparams term is zero, so a noise-free run
        //! draws nothing and consumes no random numbers.
        QVector< double > shared_ti; //!< TI walk over radius, one per run
        QVector< double > shared_ri; //!< RI offsets, one per scan

        int dbg_level;         //!< Debug level
        int nmodels;           //!< Number of models
        int tripx;             //!< Triple index
        int npoint;            //!< Number of points
        int nscan;             //!< Number of scans

        bool dbload;           //!< Database load flag
        bool stopFlag;         //!< Stop flag

        //! Stretched cell geometry, set by init_rawdata() and reused by every
        //! consumer so the data grid and the edit files cannot disagree.
        double curr_meniscus = 0.0;  //!< Current meniscus
        double curr_bottom   = 0.0;  //!< Current bottom

        QString mfilt;         //!< Model filter
        QString mrunid;        //!< Model run ID
        QString orunid;        //!< Original run ID
        QString run_type;      //!< Two-character optical data type tag, "RA" unless overridden by --runtype
        QString guid_seed;     //!< Seed for reproducible GUIDs, from --guid-seed; empty mints random ones
        QString edit_stamp;    //!< Fixed yyMMddhhmm for the edit filenames, from --edit-timestamp; empty uses the clock

        QVector<double> mtconcs; //!< Model concentrations

        QVector<US_DataIO::RawData> synData; //!< Synthetic data vector
        QList<US_Model> models;              //!< List of models
        QVector<bool> have_p1;               //!< Flags for p1 availability

        US_DataIO::RawData rdata0;           //!< Raw data
        US_SimulationParameters simparams;   //!< Simulation parameters
        US_Rotor::Rotor rotor;               //!< Rotor
        US_Rotor::RotorCalibration rotor_calib; //!< Rotor calibration
        US_Buffer buffer;                    //!< Buffer

        QStringList pfilts;                  //!< Pre-filter list
        QStringList mdescs;                  //!< Composite model descriptions as US_ModelLoader returns them; empty on the headless path

        US_Help showHelp;                    //!< Help dialog

        QwtPlot* data_plot1; //!< Data plot

        QPushButton* pb_prefilt; //!< Pre-filter button
        QPushButton* pb_semodels; //!< Select models button
        QPushButton* pb_defbuff; //!< Define buffer button
        QPushButton* pb_simparms; //!< Simulation parameters button
        QPushButton* pb_selrotor; //!< Select rotor button
        QPushButton* pb_strtsims; //!< Start simulations button
        QPushButton* pb_stopsims; //!< Stop simulations button
        QPushButton* pb_savesims; //!< Save simulations button
        QPushButton* pb_prev;     //!< Previous plot button
        QPushButton* pb_next;     //!< Next plot button
        QPushButton* pb_help;     //!< Help button
        QPushButton* pb_close;    //!< Close button

        QLineEdit* le_runid;      //!< Line edit for run ID
        QLineEdit* le_triples;    //!< Line edit for triples

        QTextEdit* te_status;     //!< Text edit for status

        //! \brief Plot data
        void data_plot(void);

    private slots:
        //! \brief Apply pre-filter
        void pre_filt(void);

        //! \brief Select models
        void select_models(void);

        //! \brief Define buffer
        void define_buffer(void);

        //! \brief Set simulation parameters
        void sim_params(void);

        //! \brief Select rotor
        void select_rotor(void);

        //! \brief Start simulations
        void start_sims(void);

        //! \brief Stop simulations
        void stop_sims(void);

        //! \brief Save simulations
        void save_sims(void);

        //! \brief Plot previous data
        void prev_plot(void);

        //! \brief Plot next data
        void next_plot(void);

        //! \brief Close all windows
        void close_all(void);

        //! \brief Initialize simulation parameters
        void init_simparams(void);

        //! \brief Set parameters
        void set_parameters(void);

        //! \brief Assign rotor
        //! \param rotor Rotor to assign
        //! \param rotor_calib Rotor calibration to assign
        void assign_rotor(US_Rotor::Rotor& rotor,
                          US_Rotor::RotorCalibration& rotor_calib);

        //! \brief Change buffer
        //! \param buffer Buffer to change
        void change_buffer(US_Buffer buffer);

        //! \brief Initialize raw data
        void init_rawdata(void);

        //! \brief Build raw data
        void build_rawdata(void);

        //! \brief Plot data (variant 1)
        void plot_data1(void);

        //! \brief Write time state
        //! \param filename File name to write to
        //! \param simparams Simulation parameters
        //! \param rdata Raw data
        //! \return Status of write operation
        int writeTimeState(const QString& filename, US_SimulationParameters& simparams, US_DataIO::RawData& rdata);

        //! \brief Show help information
        void help(void)
        { showHelp.show_help("manual/multi-wavelength/mwl_species_sim.html"); };
};

#endif // US_MWL_SPECIES_SIM_H
