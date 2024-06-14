//! \file us_fit_meniscus.h
//! \brief Contains the declaration of the US_FitMeniscus class and its members.
#ifndef US_FIT_MENISCUS_H
#define US_FIT_MENISCUS_H

#include <QApplication>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"
#include "us_spectrodata.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"
#include "qwt_color_map.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< dbg_level-conditioned qDebug()

//! \class US_FitMeniscus
//! \brief A class for fitting meniscus values in UltraScan.
class US_FitMeniscus : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_FitMeniscus class.
        US_FitMeniscus();

        //! \brief Constructor for the US_FitMeniscus class with auto mode.
        //! \param triple_info_map A map containing triple information.
        US_FitMeniscus(QMap<QString, QString> triple_info_map);

        bool auto_mode; //!< Flag for auto mode.
        bool no_fm_data; //!< Flag for no fit meniscus data.
        bool bad_men_vals; //!< Flag for bad meniscus values.

        QProgressDialog* progress_msg; //!< Progress dialog.

        //! \class ModelDesc
        //! \brief A class to hold model descriptions.
        class ModelDesc
        {
            public:
                QString description; //!< Full model description.
                QString baseDescr; //!< Base analysis-set description.
                QString fitfname; //!< Associated fit file name.
                QString modelID; //!< Model DB ID.
                QString modelGUID; //!< Model GUID.
                QString filepath; //!< Full path model file name.
                QString editID; //!< Edit parent DB ID.
                QString editGUID; //!< Edit parent GUID.
                QString antime; //!< Analysis date & time (yymmddHHMM).
                QDateTime lmtime; //!< Record last modification date & time.
                double variance; //!< Variance value.
                double meniscus; //!< Meniscus radius value.
                double bottom; //!< Bottom radius value.

                //! \brief Less than operator to enable sorting.
                //! \param md The other ModelDesc object to compare.
                //! \return True if this description is less than the other.
                bool operator<(const ModelDesc& md) const { return (description < md.description); }
        };

        //! \class NoiseDesc
        //! \brief A class to hold noise descriptions.
        class NoiseDesc
        {
            public:
                QString description; //!< Full noise description.
                QString baseDescr; //!< Base analysis-set description.
                QString noiseID; //!< Noise DB ID.
                QString noiseGUID; //!< Noise GUID.
                QString filepath; //!< Full path noise file name.
                QString modelID; //!< Model parent DB ID.
                QString modelGUID; //!< Model parent GUID.
                QString antime; //!< Analysis date & time (yymmddHHMM).

                //! \brief Less than operator to enable sorting.
                //! \param nd The other NoiseDesc object to compare.
                //! \return True if this description is less than the other.
                bool operator<(const NoiseDesc& nd) const { return (description < nd.description); }
        };

    private:
        QLabel* lb_men_lor; //!< Label for meniscus Lorentzian.
        QLabel* lb_bot_lor; //!< Label for bottom Lorentzian.
        QLabel* lb_men_fit; //!< Label for meniscus fit.
        QLabel* lb_bot_fit; //!< Label for bottom fit.
        QLabel* lb_mprads; //!< Label for meniscus and bottom radii.
        QLabel* lb_zfloor; //!< Label for Z-floor.
        QLabel* lb_order; //!< Label for order.
        QLabel* lb_men_sel; //!< Label for selected meniscus.
        QLabel* lb_rms_error; //!< Label for RMS error.

        QLineEdit* le_men_lor; //!< Line edit for meniscus Lorentzian.
        QLineEdit* le_bot_lor; //!< Line edit for bottom Lorentzian.
        QLineEdit* le_men_fit; //!< Line edit for meniscus fit.
        QLineEdit* le_bot_fit; //!< Line edit for bottom fit.
        QLineEdit* le_mprads; //!< Line edit for meniscus and bottom radii.
        QLineEdit* le_status; //!< Line edit for status.
        QLineEdit* le_men_sel; //!< Line edit for selected meniscus.
        QLineEdit* le_rms_error; //!< Line edit for RMS error.
        QLineEdit* le_invest; //!< Line edit for investigator.

        US_Help showHelp; //!< Help object.

        US_Editor* te_data; //!< Text editor for data.

        QPushButton* pb_update; //!< Button to update data.
        QPushButton* pb_scandb; //!< Button to scan the database.
        QPushButton* pb_plot; //!< Button to plot data.
        QPushButton* pb_reset; //!< Button to reset data.

        QCheckBox* ck_confirm; //!< Checkbox to confirm data.
        QCheckBox* ck_applymwl; //!< Checkbox to apply MWL.

        QSpinBox* sb_order; //!< Spin box for order.

        QwtCounter* ct_zfloor; //!< Counter for Z-floor.

        QBoxLayout* plot; //!< Plot layout.
        QwtPlot* meniscus_plot; //!< Meniscus plot.
        QwtPlot* menibott_plot; //!< Meniscus and bottom plot.
        QwtPlotCurve* raw_curve; //!< Raw data plot curve.
        QwtPlotCurve* fit_curve; //!< Fit data plot curve.

        QwtPlotCurve* minimum_curve; //!< Minimum plot curve.
        QwtPlotCurve* minimum_curve_sel; //!< Selected minimum plot curve.

        QwtPlotSpectrogram* d_spectrogram; //!< Plot spectrogram.
        US_SpectrogramData* spec_dat; //!< Spectrogram data.
        QwtLinearColorMap* colormap; //!< Color map.
        US_PlotPicker* pick; //!< Plot picker.

        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk/DB controls.

        QVector<double> v_meni; //!< Vector of meniscus values.
        QVector<double> v_bott; //!< Vector of bottom values.
        QVector<double> v_rmsd; //!< Vector of RMSD values.

        QString filedir; //!< File directory.
        QString fname_load; //!< File name to load.
        QString fname_edit; //!< File name to edit.

        QStringList edtfiles; //!< List of edited files.

        int nedtfs; //!< Number of edited files.
        int idEdit; //!< Edit ID.
        int ix_best; //!< Index of best fit.
        int ix_setfit; //!< Index of set fit.
        int dbg_level; //!< Debug level.

        bool have3val; //!< Flag for having 3 values.
        bool bott_fit; //!< Flag for bottom fit.

        double dy_global; //!< Global Y value.
        double miny_global; //!< Global minimum Y value.

        double Meniscus_fitted_2d_val; //!< Fitted meniscus 2D value.

        QMap<QString, QString> triple_information; //!< Map of triple information.

    private slots:
        //! \brief Slot to reset data.
        void reset();

        //! \brief Slot to load data.
        void load_data();

        //! \brief Slot to plot data.
        void plot_data();

        //! \brief Slot to plot data.
        //! \param value The plot value.
        void plot_data(int value);

        //! \brief Slot to plot 2D data.
        void plot_2d();

        //! \brief Slot to plot 3D data.
        void plot_3d();

        //! \brief Slot to update edited data.
        void edit_update();

        //! \brief Slot to scan the database.
        void scan_dbase();

        //! \brief Slot to scan the database in auto mode.
        //! \param triple_info_map The map of triple information.
        void scan_dbase_auto(QMap<QString, QString>& triple_info_map);

        //! \brief Slot to handle file loaded event.
        //! \param filename The loaded file name.
        void file_loaded(QString filename);

        //! \brief Load file in auto mode.
        //! \param file_info_map The map of file information.
        //! \return True if file loaded successfully, false otherwise.
        bool file_loaded_auto(QMap<QString, QString>& file_info_map);

        //! \brief Get a copy of the edit profile.
        //! \param edit_profile_map The map of edit profile information.
        void get_editProfile_copy(QMap<QString, QString>& edit_profile_map);

        //! \brief Read autoflow analysis record.
        //! \param record_id The ID of the record.
        //! \return The map of analysis record information.
        QMap<QString, QString> read_autoflowAnalysis_record(const QString& record_id);

        //! \brief Read autoflow analysis stages.
        //! \param analysis_id The ID of the analysis.
        //! \return The number of stages read.
        int read_autoflowAnalysisStages(const QString& analysis_id);

        //! \brief Revert autoflow analysis stages record.
        //! \param analysis_id The ID of the analysis.
        void revert_autoflow_analysis_stages_record(const QString& analysis_id);

        //! \brief Slot to update disk/DB.
        //! \param disk_update Flag for disk update.
        void update_disk_db(bool disk_update);

        //! \brief Update DB edit.
        //! \param editID The ID of the edit.
        //! \param GUID The GUID.
        //! \param error_msg The error message.
        void update_db_edit(QString editID, QString GUID, QString& error_msg);

        //! \brief Remove models.
        void remove_models();

        //! \brief Get noises in edit.
        //! \param editID The edit ID.
        //! \param modelIDs The list of model IDs.
        //! \param noiseIDs The list of noise IDs.
        //! \param noiseDescr The list of noise descriptions.
        void noises_in_edit(QString editID, QStringList& modelIDs, QStringList& noiseIDs, QStringList& noiseDescr);

        //! \brief Index model set fit.
        void index_model_setfit();

        //! \brief Copy color map.
        //! \param color_map The color map.
        //! \return The copied color map.
        QwtLinearColorMap* ColorMapCopy(QwtLinearColorMap* color_map);

        //! \brief Change plot type.
        void change_plot_type();

        //! \brief Select investigator.
        void sel_investigator();

        //! \brief Assign investigator.
        //! \param investigator_id The ID of the investigator.
        void assign_investigator(int investigator_id);

        //! \brief Handle mouse event.
        //! \param point The point of the mouse event.
        void mouse(const QwtDoublePoint& point);

        //! \brief Show help.
        void help() { showHelp.show_help("manual/fit_meniscus.html"); };

        signals:
            //! \brief Signal when edit profiles are updated.
            //! \param edit_profile_map The map of edit profile information.
            void editProfiles_updated(QMap<QString, QString>& edit_profile_map);

            //! \brief Signal when edit profiles are updated earlier.
            void editProfiles_updated_earlier();

            //! \brief Signal when triple analysis is processed.
            void triple_analysis_processed();

            //! \brief Signal when bad meniscus values are detected.
            //! \param meniscus_info_map The map of meniscus information.
            void bad_meniscus_values(QMap<QString, QString>& meniscus_info_map);
};

#endif // US_FIT_MENISCUS_H
