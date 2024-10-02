//! \file us_pcsa.h
#ifndef US_PCSA_H
#define US_PCSA_H

#include <unistd.h>
#include "us_analysis_base2.h"
#include "us_plot_control_pc.h"
#include "us_analysis_control_pc.h"
#include "us_noise_loader.h"
#include "us_resplot_pc.h"
#include "us_mlplot.h"
#include "us_pcsa_modelrec.h"
#include "us_db2.h"
#include "us_astfem_rsa.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_analyte.h"
#include "us_zsolute.h"
#include "qwt_plot_marker.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#include <process.h>
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif
#ifndef SS_DATASET
#define SS_DATASET US_SolveSim::DataSet
#endif
#ifndef US_ZS_ATTRIBS
#define US_ZS_ATTRIBS
#define ATTR_S US_ZSolute::ATTR_S
#define ATTR_K US_ZSolute::ATTR_K
#define ATTR_W US_ZSolute::ATTR_W
#define ATTR_V US_ZSolute::ATTR_V
#define ATTR_D US_ZSolute::ATTR_D
#define ATTR_C US_ZSolute::ATTR_C
#endif

//! \class US_pcsa
//! \brief Class for processing and analyzing PCSA data.
class US_pcsa : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_pcsa
        US_pcsa();

        //! \brief Indicates that the analysis is done
        //! \param status Status of the analysis
        void analysis_done(int status);

        //! \brief Gets the multi-wavelength edited data
        //! \return Pointer to the edited data
        US_DataIO::EditedData* mw_editdata();

        //! \brief Gets the multi-wavelength simulated data
        //! \return Pointer to the simulated data
        US_DataIO::RawData* mw_simdata();

        //! \brief Gets the multi-wavelength residual data
        //! \return Pointer to the residual data
        US_DataIO::RawData* mw_resdata();

        //! \brief Gets the exclusion list
        //! \return Pointer to the exclusion list
        QList<int>* mw_excllist();

        //! \brief Gets the model
        //! \return Pointer to the model
        US_Model* mw_model();

        //! \brief Gets the TI noise data
        //! \return Pointer to the TI noise data
        US_Noise* mw_ti_noise();

        //! \brief Gets the RI noise data
        //! \return Pointer to the RI noise data
        US_Noise* mw_ri_noise();

        //! \brief Gets the status text edit pointer
        //! \return Pointer to the status text edit
        QPointer<QTextEdit> mw_status_text();

        //! \brief Gets the model statistics
        //! \return Pointer to the model statistics string list
        QStringList* mw_model_stats();

        //! \brief Gets the model records
        //! \return Pointer to the model records vector
        QVector<US_ModelRecord>* mw_mrecs();

        //! \brief Gets the Monte Carlo model records
        //! \return Pointer to the Monte Carlo model records vector
        QVector<US_ModelRecord>* mw_mrecs_mc();

        //! \brief Gets the base RSS value
        //! \return Pointer to the base RSS value
        int* mw_base_rss();

    private:
        QGridLayout* progressLayout; //!< Layout for progress display

        US_Editor* te_results; //!< Editor for displaying results

        SS_DATASET dset; //!< Data set

        QList<SS_DATASET*> dsets; //!< List of data sets

        QVector<SP_SPEEDPROFILE> speed_steps; //!< Speed steps vector

        US_DataIO::EditedData* edata; //!< Edited data pointer
        US_DataIO::RawData sdata; //!< Simulated data
        US_DataIO::RawData rdata; //!< Residual data

        QPointer<US_ResidPlotPc> resplotd; //!< Residual plot pointer
        QPointer<US_PlotControlPc> eplotcd; //!< Experimental plot control pointer
        QPointer<US_AnalysisControlPc> analcd; //!< Analysis control pointer
        QPointer<US_MLinesPlot> mlplotd; //!< Multi-lines plot pointer

        QVector<US_ModelRecord> mrecs; //!< Model records vector
        QVector<US_ModelRecord> mrecs_mc; //!< Monte Carlo model records vector

        US_Model model; //!< Model
        US_Noise ri_noise_in; //!< RI noise input
        US_Noise ti_noise_in; //!< TI noise input

        QStringList model_stats; //!< Model statistics

        QPoint rbd_pos; //!< Position of residual plot dialog
        QPoint epd_pos; //!< Position of experimental plot dialog
        QPoint acd_pos; //!< Position of analysis control dialog

        QLineEdit* le_vari; //!< Line edit for variance
        QLineEdit* le_rmsd; //!< Line edit for RMSD

        QTextEdit* te_status; //!< Text edit for status

        QPushButton* pb_fitcntl; //!< Fit control button
        QPushButton* pb_plt3d; //!< 3D plot button
        QPushButton* pb_pltres; //!< Residual plot button

        double rmsd; //!< Root Mean Square Deviation

        int dbg_level; //!< Debug level
        int mc_iters; //!< Number of Monte Carlo iterations
        int baserss; //!< Base RSS value

        bool exp_steps; //!< Flag for experimental steps
        bool dat_steps; //!< Flag for data steps

    private slots:
        //! \brief Opens the residual plot
        void open_resplot(void);

        //! \brief Opens the 3D plot
        void open_3dplot(void);

        //! \brief Opens the fit control
        void open_fitcntl(void);

        //! \brief Gets model statistics
        //! \return Model statistics string
        QString model_statistics(void);

        //! \brief Gets distribution information
        //! \return Distribution information string
        QString distrib_info(void);

        //! \brief Plots data
        void data_plot(void);

        //! \brief Writes a report
        //! \param ts QTextStream to write the report to
        void write_report(QTextStream& ts);

        //! \brief Writes a bitmap
        //! \param filename Name of the bitmap file
        void write_bmap(const QString filename);

        //! \brief Slot to handle child closed event
        //! \param obj Pointer to the closed child object
        void child_closed(QObject* obj);

        //! \brief Loads data
        void load(void);

        //! \brief Views data
        void view(void);

        //! \brief Saves data
        void save(void);

        //! \brief Closes the application
        void close(void);

        //! \brief Handles new triple selection
        //! \param index Index of the new triple
        void new_triple(int index);

        //! \brief Shows help information
        void help(void)
        { showHelp.show_help("manual/pcsa.html"); };
};

#endif // US_PCSA_H
