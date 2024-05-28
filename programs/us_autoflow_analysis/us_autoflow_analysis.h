//! \file us_analysis_auto.h
//! \brief Contains the declaration of the US_Analysis_auto class and its members.

#ifndef US_ANALYSIS_AUTO_H
#define US_ANALYSIS_AUTO_H

#include "us_widgets.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "../us_fit_meniscus/us_fit_meniscus.h"
#include "../us_fematch/us_fematch.h"

//! \class US_Analysis_auto
//! \brief A class for automated analysis in UltraScan.
class US_Analysis_auto : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_Analysis_auto class.
        US_Analysis_auto();

        QTreeWidget* treeWidget; //!< Tree widget for displaying analysis stages.
        QMap<QString, QTreeWidgetItem*> topItem; //!< Map of top-level tree items.
        QMap<QString, QTreeWidgetItem*> childItem_2DSA; //!< Map of child items for 2DSA.
        QMap<QString, QTreeWidgetItem*> childItem_2DSA_FM; //!< Map of child items for 2DSA FitMeniscus.
        QMap<QString, QTreeWidgetItem*> childItem_2DSA_FITMEN; //!< Map of child items for 2DSA FITMEN.
        QMap<QString, QTreeWidgetItem*> childItem_2DSA_IT; //!< Map of child items for 2DSA IT.
        QMap<QString, QTreeWidgetItem*> childItem_2DSA_MC; //!< Map of child items for 2DSA MC.
        QMap<QString, QTreeWidgetItem*> childItem_PCSA; //!< Map of child items for PCSA.

        QMap<QString, QGroupBox*> groupbox_2DSA; //!< Map of group boxes for 2DSA.
        QMap<QString, QGroupBox*> groupbox_2DSA_FM; //!< Map of group boxes for 2DSA FitMeniscus.
        QMap<QString, QGroupBox*> groupbox_2DSA_FITMEN; //!< Map of group boxes for 2DSA FITMEN.
        QMap<QString, QGroupBox*> groupbox_2DSA_IT; //!< Map of group boxes for 2DSA IT.
        QMap<QString, QGroupBox*> groupbox_2DSA_MC; //!< Map of group boxes for 2DSA MC.
        QMap<QString, QGroupBox*> groupbox_PCSA; //!< Map of group boxes for PCSA.

        QTimer* timer_update; //!< Timer for update.
        QTimer* timer_end_process; //!< Timer for end process.

        bool in_gui_update; //!< Flag for GUI update.
        bool in_reload_end_process; //!< Flag for reload end process.

        //! \brief Reset the analysis panel.
        void reset_analysis_panel_public(void);

        US_FitMeniscus* FitMen; //!< Pointer to US_FitMeniscus object.

        QPointer<US_ResidPlotFem> resplotd; //!< Pointer to US_ResidPlotFem object.

        //! \brief Get edited data.
        //! \return Pointer to US_DataIO::EditedData.
        US_DataIO::EditedData* aa_editdata();

        //! \brief Get simulated data.
        //! \return Pointer to US_DataIO::RawData.
        US_DataIO::RawData* aa_simdata();

        //! \brief Get exclusion list.
        //! \return Pointer to a list of integers.
        QList<int>* aa_excllist();

        //! \brief Get model.
        //! \return Pointer to US_Model.
        US_Model* aa_model();

        //! \brief Get time-invariant noise.
        //! \return Pointer to US_Noise.
        US_Noise* aa_ti_noise();

        //! \brief Get radially invariant noise.
        //! \return Pointer to US_Noise.
        US_Noise* aa_ri_noise();

        //! \brief Get residual bitmap.
        //! \return QPointer to US_ResidsBitmap.
        QPointer<US_ResidsBitmap> aa_resbmap();

        //! \brief Get triple information.
        //! \return QString with triple information.
        QString aa_tripleInfo();

        QMessageBox* msg_sim; //!< Message box for simulation.
        QProgressDialog* progress_msg; //!< Progress dialog for simulation.

    private:
        QVector<US_DataIO::RawData> rawData; //!< Vector of raw data.
        QVector<US_DataIO::EditedData> editedData; //!< Vector of edited data.

        QVector<SP_SPEEDPROFILE> speed_steps; //!< Vector of speed profiles.

        US_DataIO::EditedData* edata; //!< Pointer to edited data.
        US_DataIO::RawData* rdata; //!< Pointer to raw data.
        US_DataIO::RawData* sdata; //!< Pointer to simulated data.
        US_DataIO::RawData wsdata; //!< Working set of raw data.

        QPointer<US_ResidsBitmap> rbmapd; //!< Pointer to residual bitmap.

        //! \class ModelDesc
        //! \brief Class to hold model descriptions.
        class ModelDesc
        {
            public:
                QString description; //!< Full model description.
                QString baseDescr; //!< Base analysis-set description.
                QString fitfname; //!< Associated fit file name.
                QString modelID; //!< Model database ID.
                QString modelGUID; //!< Model GUID.
                QString filepath; //!< Full path model file name.
                QString editID; //!< Edit parent database ID.
                QString editGUID; //!< Edit parent GUID.
                QString antime; //!< Analysis date and time (yymmddHHMM).
                QDateTime lmtime; //!< Record last modification date and time.
                double variance; //!< Variance value.
                double meniscus; //!< Meniscus radius value.
                double bottom; //!< Bottom radius value.

                //! \brief Less than operator to enable sorting.
                //! \param md The model description to compare.
                //! \return True if this description is less than the other, false otherwise.
                bool operator<(const ModelDesc& md) const { return (description < md.description); }
        };

        //! \class NoiseDesc
        //! \brief Class to hold noise descriptions.
        class NoiseDesc
        {
            public:
                QString description; //!< Full noise description.
                QString baseDescr; //!< Base analysis-set description.
                QString noiseID; //!< Noise database ID.
                QString noiseGUID; //!< Noise GUID.
                QString filepath; //!< Full path noise file name.
                QString modelID; //!< Model parent database ID.
                QString modelGUID; //!< Model parent GUID.
                QString antime; //!< Analysis date and time (yymmddHHMM).

                //! \brief Less than operator to enable sorting.
                //! \param nd The noise description to compare.
                //! \return True if this description is less than the other, false otherwise.
                bool operator<(const NoiseDesc& nd) const { return (description < nd.description); }
        };

        QMap<QString, QString> protocol_details_at_analysis; //!< Protocol details at analysis.

        bool fitmen_bad_vals; //!< Flag for bad fitmen values.
        bool no_fm_data_auto; //!< Flag for no fitmen data in auto mode.

        QProgressDialog* progress_msg_fmb; //!< Progress message for fitmen.
        QVector<double> v_meni; //!< Vector of meniscus values.
        QVector<double> v_bott; //!< Vector of bottom values.
        QVector<double> v_rmsd; //!< Vector of RMSD values.

        QString filedir; //!< File directory.
        QString fname_load; //!< Filename for load.
        QString fname_edit; //!< Filename for edit.
        QStringList edtfiles; //!< List of edit files.
        int nedtfs; //!< Number of edit files.
        int ix_best; //!< Index of best fit.
        int ix_setfit; //!< Index of set fit.
        bool have3val; //!< Flag for having 3D values.
        bool bott_fit; //!< Flag for bottom fit.
        int idEdit; //!< Edit ID.

        double fit_xvl; //!< Fit value for 2D data.
        double f_meni; //!< Fit value for meniscus in 3D data.
        double f_bott; //!< Fit value for bottom in 3D data.

        double dy_global; //!< Global delta Y.
        double miny_global; //!< Global minimum Y.

        QString tripleInfo; //!< Triple information.

        int eID_global; //!< Global edit ID.

        US_Model model; //!< Model object.
        US_Model model_loaded; //!< Loaded model object.
        US_Model model_used; //!< Used model object.

        US_Noise ri_noise; //!< Radially invariant noise object.
        US_Noise ti_noise; //!< Time-invariant noise object.
        QList<int> excludedScans; //!< List of excluded scans.
        US_Solution solution_rec; //!< Solution record.

        US_Math2::SolutionData solution; //!< Solution data.
        QVector<QVector<double>> resids; //!< Vector of residuals.

        US_SimulationParameters simparams; //!< Simulation parameters.
        QList<US_DataIO::RawData> tsimdats; //!< List of simulated raw data.
        QList<US_Model> tmodels; //!< List of models.
        QVector<int> kcomps; //!< Vector of components.

        QStringList noiIDs; //!< Noise GUIDs.
        QStringList noiEdIDs; //!< Noise edit GUIDs.
        QStringList noiMoIDs; //!< Noise model GUIDs.
        QStringList noiTypes; //!< Noise types.
        QStringList modIDs; //!< Model GUIDs.
        QStringList modEdIDs; //!< Model edit GUIDs.
        QStringList modDescs; //!< Model descriptions.

        int thrdone; //!< Thread done flag.

        double density; //!< Density value.
        double viscosity; //!< Viscosity value.
        double vbar20; //!< v-bar value at 20°C.

        double meniscus_value; //!< Meniscus value.
        double bottom_value; //!< Bottom value.
        double baseline_value; //!< Baseline value.

        int modelx; //!< Model index.

        QVector<ModelDesc> modelDescs; //!< Vector of model descriptions.
        QVector<NoiseDesc> tiNoiseDescs; //!< Vector of time-invariant noise descriptions.
        QVector<NoiseDesc> riNoiseDescs; //!< Vector of radially invariant noise descriptions.

        //! \brief Reset the analysis panel.
        void reset_analysis_panel(void);

    private slots:
        //! \brief Update the protocol details.
        void update_prot_details(void);

        //! \brief Reload data at the end of the process.
        void reload_data_at_end_process(void);

    signals:
        //! \brief Signal for US_Analysis_auto class.
        void analysis_done_auto(void);

        //! \brief Signal for setting meniscus value.
        //! \param x The meniscus value.
        void set_meniscus(double x);

        //! \brief Signal for setting bottom value.
        //! \param x The bottom value.
        void set_bottom(double x);

        //! \brief Signal for setting triple information.
        //! \param tripleInfo The triple information.
        void setTripleInfo(QString tripleInfo);
};

#endif // US_ANALYSIS_AUTO_H
