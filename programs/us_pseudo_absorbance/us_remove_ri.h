#ifndef US_REMOVE_RI_H
#define US_REMOVE_RI_H
#include <us_widgets.h>
#include <us_settings.h>
#include <us_dataIO.h>
#include <us_refScan_dataIO.h>
#include "us_plot.h"
#include "us_pabs_common.h"
#include "us_images.h"
#include "us_util.h"
#include "us_math2.h"



class US_RemoveRI : public US_Widgets
{
    Q_OBJECT

public:
    US_RemoveRI();
    QPushButton* pb_close;
    bool hasData = false;

    signals:
    void sig_plot_data(bool);
    void sig_plot_fit(bool);
    void sig_save_button(void);

private slots:
    void slt_import(void);
    void slt_new_ccw(int);
    void slt_reset(void);
    void slt_set_id(int);
    void slt_prev_id(void);
    void slt_next_id(void);
    void slt_prev_ccw(void);
    void slt_next_ccw(void);
    void slt_plot_data(bool);
    void slt_plot_fit(bool);
    void slt_autofit(int);
////    void slt_save_avail(void);
//    void slt_save(void);
    void slt_pick_point(void);
    void slt_mouse(const QwtDoublePoint&);
    void slt_residual(void);
//    void slt_edit_le(QString);

private:
    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_prev_ccw;
    QPushButton* pb_next_ccw;
    QPushButton* pb_import;
    QPushButton* pb_reset;
    QPushButton* pb_save;
    QPushButton* pb_pick_rp;
    QPushButton* pb_fit;
    QPushButton* pb_calc_res;

    QLineEdit* le_lambstrt;
    QLineEdit* le_lambstop;
    QLineEdit* le_runIdIn;
    QLineEdit* le_runIdOut;
    QLineEdit* le_dir;
    QLineEdit* le_desc;
    QLineEdit* le_status;
    QLineEdit* le_xrange;
    QLineEdit* le_fitorder;
    QLineEdit* le_fiterror;

    QLabel* lb_maxorder;
    QLabel* lb_manorder;
    QLabel* lb_fitorder;
    QLabel* lb_fiterror;

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

//    int cpos, le_cursor_pos = -1;
    QVector<double> pmin;
    QVector<double> pmax;
    int wavl_id;
    int ccw_id;
    int n_ccw;
    int n_wavls;
    int max_ct = 20;
    QVector<int> intDataId;
    QVector<double> wavelength;
    QVector<US_DataIO::RawData> allData;
    QVector<US_DataIO::RawData> allDataC;
    QVector<QVector<double>> allResiduals;
    QFileInfoList allDataFInfo;
    QVector<int> fitOrder;
    QVector<double> fitError;
    QVector<QVector<double>> fitParam;

    void set_cb_triples(void);
    void offon_prev_next(void);
    double trapz(const double*, const double*, double, double);

};



#endif // US_REMOVE_RI_H
