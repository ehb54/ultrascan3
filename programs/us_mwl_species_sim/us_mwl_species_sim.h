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

    private:
        int dbg_level;         //!< Debug level
        int nmodels;           //!< Number of models
        int tripx;             //!< Triple index
        int npoint;            //!< Number of points
        int nscan;             //!< Number of scans

        bool dbload;           //!< Database load flag
        bool stopFlag;         //!< Stop flag

        double curr_meniscus;  //!< Current meniscus
        double curr_bottom;    //!< Current bottom

        QString mfilt;         //!< Model filter
        QString mrunid;        //!< Model run ID
        QString orunid;        //!< Original run ID

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
        QStringList mdescs;                  //!< Model descriptions

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
        { showHelp.show_help("manual/mwl_species_sim.html"); };
};

#endif // US_MWL_SPECIES_SIM_H
