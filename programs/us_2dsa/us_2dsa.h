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
        void help(void) { showHelp.show_help("manual/2dsa.html"); };
};

#endif // US_2DSA_H