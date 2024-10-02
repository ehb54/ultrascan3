//! \file us_mwl_species_fit.h
#ifndef US_MWL_SPECIES_FIT_H
#define US_MWL_SPECIES_FIT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_mwl_sf_plot3d.h"

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

        QLineEdit* le_fit_error; //!< Line edit for fit error

        QVector<SFData> synFitError; //!< Synthetic fit error vector

        //! \brief Plot data
        void data_plot(void);

        //! \brief Get fit error
        void get_fit_error(void);

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
        { showHelp.show_help("manual/mwl_species_fit.html"); };
};

#endif // US_MWL_SPECIES_FIT_H