//! \file us_remove_ri.h
#ifndef US_REMOVE_RI_H
#define US_REMOVE_RI_H

#include <us_widgets.h>
#include <us_settings.h>
#include "us_dataIO.h"
#include "us_refScan_dataIO.h"
#include "us_plot.h"
#include "us_pabs_common.h"
#include "us_images.h"
#include "us_util.h"
#include "us_math2.h"
#include <algorithm>
#include "us_matrix.h"
#include <QtMath>
#include "qwt_legend.h"

/**
 * @class US_RemoveRI
 * @brief The US_RemoveRI class provides functionality to remove refractive index (RI) effects from data.
 */
class US_RemoveRI : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_RemoveRI.
         */
        US_RemoveRI();
        QPushButton* pb_close; ///< Close button
        bool hasData = false; ///< Flag to check if data is available

        signals:
                /**
                 * @brief Signal to plot data.
                 * @param plot Boolean flag to indicate whether to plot
                 */
                void sig_plot(bool plot);

    private slots:
                void slt_import(void);
        void slt_new_ccw(int);
        void slt_reset(void);
        void slt_set_id(int);
        void slt_prev_id(void);
        void slt_next_id(void);
        void slt_prev_ccw(void);
        void slt_next_ccw(void);

        void slt_autofit_state(int);
        void slt_plot(bool);
        void slt_rm_fit(double);
        void slt_save(void);
        void slt_pick_point(void);
        void slt_mouse(const QwtDoublePoint&);
        void slt_polyfit(void);

    private:
        /**
         * @enum states
         * @brief Enumeration for different states of the data.
         */
        enum states { INTG_S, FIT_S, INTG_FIT_S, RDATA_S, CDATA_S };

        QPushButton* pb_prev_id; ///< Previous ID button
        QPushButton* pb_next_id; ///< Next ID button
        QPushButton* pb_prev_ccw; ///< Previous CCW button
        QPushButton* pb_next_ccw; ///< Next CCW button
        QPushButton* pb_import; ///< Import button
        QPushButton* pb_reset; ///< Reset button
        QPushButton* pb_save; ///< Save button
        QPushButton* pb_pick_rp; ///< Pick reference point button
        QPushButton* pb_fit; ///< Fit button

        QLineEdit* le_lambstrt; ///< Line edit for start wavelength
        QLineEdit* le_lambstop; ///< Line edit for stop wavelength
        QLineEdit* le_runIdIn; ///< Line edit for input run ID
        US_LineEdit_RE* le_runIdOut; ///< Line edit for output run ID
        QLineEdit* le_dir; ///< Line edit for directory
        QLineEdit* le_desc; ///< Line edit for description
        QLineEdit* le_status; ///< Line edit for status
        QLineEdit* le_xrange; ///< Line edit for X range
        QLineEdit* le_fitorder; ///< Line edit for fit order
        QLineEdit* le_fitrsqrd; ///< Line edit for fit R-squared

        QLabel* lb_maxorder; ///< Label for max order
        QLabel* lb_manorder; ///< Label for manual order
        QLabel* lb_fitorder; ///< Label for fit order
        QLabel* lb_fitrsqrd; ///< Label for fit R-squared

        QString runIdOut; ///< Output run ID

        QComboBox* cb_plot_id; ///< Combo box for plot ID
        QComboBox* cb_triples; ///< Combo box for triples

        US_PlotPicker *picker; ///< Plot picker
        US_Plot* usplot_data; ///< Data plot
        US_Plot* usplot_fit; ///< Fit plot
        QwtPlot* qwtplot_data; ///< Qwt plot for data
        QwtPlot* qwtplot_fit; ///< Qwt plot for fit
        QwtPlotGrid* grid; ///< Plot grid

        QwtCounter* ct_max_order; ///< Counter for max order
        QwtCounter* ct_order; ///< Counter for order
        QCheckBox* cb_autofit; ///< Check box for auto fit

        CCW ccwList; ///< List of CCW
        QStringList ccwStrList; ///< String list of CCW
        CCW_ITEM ccwItemList; ///< Item list of CCW

        QVector<double> idMin; ///< Minimum ID values
        QVector<double> idMax; ///< Maximum ID values
        QVector<bool> ccwFitState; ///< CCW fit state
        QVector<bool> ccwIntgState; ///< CCW integration state
        int wavl_id; ///< Wavelength ID
        int ccw_id; ///< CCW ID
        int n_ccw; ///< Number of CCW
        int n_wavls; ///< Number of wavelengths
        int max_ct = 20; ///< Maximum count
        QVector<int> intDataId; ///< Internal data IDs
        QVector<double> wavelength; ///< Wavelengths
        QVector<US_DataIO::RawData> allData; ///< All data
        QVector<US_DataIO::RawData> allDataC; ///< All corrected data
        QVector<QVector<double>> allIntegrals; ///< All integrals
        QVector<QVector<double>> allIntegralsC; ///< All corrected integrals
        QVector<bool> intgState; ///< Integration state
        QVector<bool> fitState; ///< Fit state
        QFileInfoList allDataFInfo; ///< File info list of all data
        QVector<int> fitOrder; ///< Fit order
        QVector<double> fitRsqrd; ///< Fit R-squared values
        QVector<QVector<double>> fitParam; ///< Fit parameters

        void set_cb_triples(void);
        void pn_id_avail(void);
        void pn_ccw_avail(void);
        double trapz(const double*, const double*, int, int);
        double get_rsqrd(double*, double*, int, QVector<double>);
        void integrate(int);
        void correct_data(void);
        void clean_states(int);
        void pb_fit_avail(void);
        void pb_save_avail(void);
        void plot_data(bool);
        void plot_fit(bool);
        int get_id(QVector<double>, double);
};

#endif // US_REMOVE_RI_H
