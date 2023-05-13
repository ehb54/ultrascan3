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



class US_RemoveRI : public US_Widgets
{
    Q_OBJECT

public:
    US_RemoveRI();
    QPushButton* pb_close;
    bool hasData = false;

    signals:
    void sig_plot(bool);

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
    enum states {INTG_S, FIT_S, INTG_FIT_S, RDATA_S, CDATA_S};
    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_prev_ccw;
    QPushButton* pb_next_ccw;
    QPushButton* pb_import;
    QPushButton* pb_reset;
    QPushButton* pb_save;
    QPushButton* pb_pick_rp;
    QPushButton* pb_fit;

    QLineEdit* le_lambstrt;
    QLineEdit* le_lambstop;
    QLineEdit* le_runIdIn;
    US_LineEdit_RE* le_runIdOut;
    QLineEdit* le_dir;
    QLineEdit* le_desc;
    QLineEdit* le_status;
    QLineEdit* le_xrange;
    QLineEdit* le_fitorder;
    QLineEdit* le_fitrsqrd;

    QLabel* lb_maxorder;
    QLabel* lb_manorder;
    QLabel* lb_fitorder;
    QLabel* lb_fitrsqrd;

    QString runIdOut;

    QComboBox* cb_plot_id;
    QComboBox* cb_triples;

    US_PlotPicker *picker;
    US_Plot* usplot_data;
    US_Plot* usplot_fit;
    QwtPlot* qwtplot_data;
    QwtPlot* qwtplot_fit;
    QwtPlotGrid* grid;

    QwtCounter* ct_max_order;
    QwtCounter* ct_order;
    QCheckBox* cb_autofit;

    CCW ccwList;
    QStringList ccwStrList;
    CCW_ITEM ccwItemList;

    QVector<double> idMin;
    QVector<double> idMax;
    QVector<bool> ccwFitState;
    QVector<bool> ccwIntgState;
    int wavl_id;
    int ccw_id;
    int n_ccw;
    int n_wavls;
    int max_ct = 20;
    QVector<int> intDataId;
    QVector<double> wavelength;
    QVector<US_DataIO::RawData> allData;
    QVector<US_DataIO::RawData> allDataC;
    QVector<QVector<double>> allIntegrals;
    QVector<QVector<double>> allIntegralsC;
    QVector<bool> intgState;
    QVector<bool> fitState;
    QFileInfoList allDataFInfo;
    QVector<int> fitOrder;
    QVector<double> fitRsqrd;
    QVector<QVector<double>> fitParam;

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
