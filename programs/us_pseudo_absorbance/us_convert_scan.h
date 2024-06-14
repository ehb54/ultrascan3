//! \file us_convert_scan.h
#ifndef US_CONVERT_SCAN_H
#define US_CONVERT_SCAN_H

#include <us_widgets.h>
#include <us_settings.h>
#include <us_dataIO.h>
#include "us_refScan_dataIO.h"
#include "us_plot.h"
#include "us_pabs_common.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_images.h"
#include "us_util.h"
#include "us_math2.h"
//#include "us_defines.h"
//#include "us_extern.h"
//#include "us_help.h"

/**
 * @class US_ConvertScan
 * @brief The US_ConvertScan class handles the conversion and management of scan data.
 */
class US_ConvertScan : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_ConvertScan.
         */
        US_ConvertScan();
        QPushButton* pb_close; ///< Close button
        bool hasData; ///< Flag to check if data is available

        signals:
                /**
                 * @brief Signal to plot data.
                 */
                void sig_plot(void);

    /**
     * @brief Signal to enable save button.
     */
    void sig_save_button(void);

    private slots:
                void slt_import(void);
        void slt_set_ccw_default(void);
        void slt_del_item(void);
        void slt_wavl_ctrl(int);
        void slt_reset(void);
        void slt_set_id(int);
        void slt_prev_id(void);
        void slt_next_id(void);
        void slt_plot(void);
        void slt_act_refScans(int);
        void slt_load_refScans(void);
        void slt_update_scrng(double);
        void slt_zeroing(int);
        void slt_xrange(int);
        void slt_reset_scans(void);
        void slt_reset_allscans(void);
        void slt_apply_allscans(void);
        void slt_reset_refData(void);
        void slt_save_avail(void);
        void slt_save(void);
        void slt_update_buffer(int);
        void slt_update_smooth(double);
        void slt_pick_point(void);
        void slt_mouse(const QwtDoublePoint&);
        void slt_cac(int);

    private:
        const double maxAbs = 3; ///< Maximum absorbance

        QPushButton* pb_prev_id; ///< Previous ID button
        QPushButton* pb_next_id; ///< Next ID button
        QPushButton* pb_import; ///< Import button
        QPushButton* pb_reset; ///< Reset button
        QPushButton* pb_import_refScans; ///< Import reference scans button
        QPushButton* pb_reset_refData; ///< Reset reference data button
        QPushButton* pb_save; ///< Save button
        QPushButton* pb_pick_rp; ///< Pick reference point button

        QLabel* lb_lambstrt; ///< Label for start wavelength
        QLabel* lb_lambstop; ///< Label for stop wavelength
        QLineEdit* le_runIdInt; ///< Line edit for internal run ID
        US_LineEdit_RE* le_runIdAbs; ///< Line edit for absolute run ID
        QLineEdit* le_desc; ///< Line edit for description
        QLineEdit* le_status; ///< Line edit for status
        QLineEdit* le_ref_range; ///< Line edit for reference range
        QLineEdit* le_xrange; ///< Line edit for X range
        QLabel* plot_title; ///< Plot title label
        QString runIdAbs; ///< Absolute run ID

        QComboBox* cb_plot_id; ///< Combo box for plot ID
        QComboBox* cb_buffer; ///< Combo box for buffer

        US_PlotPicker *picker_abs; ///< Absorbance plot picker
        US_PlotPicker *picker_insty; ///< Intensity plot picker
        US_Plot* usplot_insty; ///< Intensity plot
        US_Plot* usplot_abs; ///< Absorbance plot
        QwtPlot* qwtplot_insty; ///< Qwt plot for intensity
        QwtPlot* qwtplot_abs; ///< Qwt plot for absorbance
        QwtPlotGrid* grid; ///< Plot grid

        QwtCounter* ct_scan_l; ///< Scan lower counter
        QwtCounter* ct_scan_u; ///< Scan upper counter
        QwtCounter* ct_smooth; ///< Smooth counter
        QwtCounter* ct_last_scans; ///< Last scans counter

        QFrame *frm_refScan; ///< Reference scan frame

        QListWidget* lw_triple; ///< List widget for triples
        QCheckBox *ckb_act_refscan; ///< Check box for active reference scan
        QCheckBox *ckb_zeroing; ///< Check box for zeroing
        QCheckBox *ckb_xrange; ///< Check box for X range
        QCheckBox *ckb_ChroAberCorr; ///< Check box for Chromatic Aberration Correction
        US_Disk_DB_Controls* diskDB_ctrl; ///< Disk/DB controls

        CCW ccwList; ///< List of CCW
        CCW ccwListMain; ///< Main list of CCW
        QStringList ccwStrListMain; ///< Main string list of CCW
        CCW_ITEM ccwItemList; ///< List of CCW items

        US_DB2 *dbCon; ///< Database connection
        US_Passwd pw; ///< Password object
        int cpos, le_cursor_pos = -1; ///< Cursor positions
        double x_min_picked, x_max_picked; ///< Picked X range
        int wavl_id; ///< Wavelength ID
        int n_wavls; ///< Number of wavelengths
        int refId; ///< Reference ID
        QStringList intRunIds; ///< Internal run IDs
        QVector<int> intDataId; ///< Internal data IDs
        QVector<double> wavelength; ///< Wavelengths
        QVector<double> xvalues; ///< X values
        QVector<QVector<double>> xvaluesRefCAC; ///< X values for Reference Chromatic Aberration Correction
        QVector<US_DataIO::Scan> intensity; ///< Intensity data
        QVector<US_DataIO::Scan> absorbance; ///< Absorbance data
        QVector<US_DataIO::Scan> absorbanceBuffer; ///< Absorbance buffer data
        QVector<US_DataIO::RawData> allIntData; ///< All intensity data
        QVector<QVector<int>> scansRange; ///< Scans range
        US_RefScanDataIO::RefData refData; ///< Reference data
        QFileInfoList allIntDataFiles; ///< All intensity data files

        void make_ccwItemList(void);
        void set_wavl_ctrl(void);
        void set_listWidget(void);
        void offon_prev_next(void);
        void plot_intensity(void);
        void plot_refscan(void);
        void plot_absorbance(void);
        void set_scan_ct(void);
        void get_pseudo_absorbance(int, int, bool buffer);
        bool get_absorbance(int, int);
        void get_intensity(int);
        bool get_refId(double);
        void set_buffer_list(void);
        void get_relative_absorbance(int);
        void trim_absorbance(void);
        QVector<double> get_smooth(QVector<double>, int, bool, bool);
        void load_from_DB(void);
        void uncheck_CA_silently(void);
        bool linear_interpolation(QVector<double>&, QVector<double>&, QVector<double>&);
        void select_CCW_save(QVector<int>&);
};

/**
 * @class LoadDBWidget
 * @brief The LoadDBWidget class provides a dialog for loading data from the database.
 */
class LoadDBWidget : public US_WidgetsDialog {
    Q_OBJECT
    public:
        /**
         * @brief Constructor for LoadDBWidget.
         * @param w Parent widget
         * @param dbCon Database connection
         * @param refData Reference data
         */
        LoadDBWidget(QWidget* w, US_DB2 *dbCon, US_RefScanDataIO::RefData& refData);

    private slots:
                void slt_set_refTable(void);
        void slt_apply(void);

    private:
        US_DB2 *db; ///< Database connection
        US_RefScanDataIO::RefData *refData; ///< Reference data
        QVector<int> instrumentIDs; ///< Instrument IDs
        QStringList instrumentNames; ///< Instrument names
        QVector<QVector<int>> instrumentRefList; ///< Instrument reference list
        QTreeWidget *tw_instruments; ///< Tree widget for instruments
        QTreeWidget *tw_refData; ///< Tree widget for reference data

        /**
         * @class refScanTableInfo
         * @brief Holds information about reference scan table entries.
         */
        class refScanTableInfo {
            public:
                int id; ///< Table ID
                int instrumentID; ///< Instrument ID
                int personID; ///< Person ID
                QString type; ///< Type of scan
                QStringList exprimentIds; ///< Experiment IDs
                QDate referenceTime; ///< Reference time
                int nWavelength; ///< Number of wavelengths
                int nPoints; ///< Number of points
                double startWavelength; ///< Start wavelength
                double stopWavelength; ///< Stop wavelength
                bool null_blob; ///< Null blob flag
                QDate lastUpdated; ///< Last updated date
        };

        QVector<refScanTableInfo> refTable; ///< Reference table
        QDate str2date(QString); ///< Convert string to date
};

#endif // US_CONVERT_SCAN_H
