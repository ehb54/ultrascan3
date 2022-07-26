#ifndef US_PEAK_DECOMPOSITION_H
#define US_PEAK_DECOMPOSITION_H

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_images.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_minimize.h"

class US_PeakDecomposition : public US_Widgets{
    Q_OBJECT
public:
    explicit US_PeakDecomposition();

signals:
    void sig_plot1(bool);

private:
    QPushButton* pb_load_data;
    QPushButton* pb_reset_data;
    QPushButton* pb_reset_plot;
    QPushButton* pb_help;
    QPushButton* pb_close;
    QPushButton* pb_save;



    US_Plot* usplot;
    QwtPlot* plot;
//    US_Plot* usplot2;
//    QwtPlot* plot2;
    QwtPlotGrid* grid;

    int scanid;
    int nscans;
    QStringList filenames;
    QStringList filePaths;
    QStringList selFilenames;
    QVector<QVector<double>> xvalues;
    QVector<QVector<QVector<double>>> yvalues;
//    QVector<QVector<QVector<double>>> integral;
//    QVector<QVector<QVector<double>>> integral_s;
    QVector<QVector<double>> xvalues_sel;
    QVector<QVector<double>> midxval_sel;
    QVector<QVector<QVector<double>>> yvalues_sel;
    QVector<QVector<QVector<double>>> integral_sel;
    QVector<QVector<QVector<double>>> integral_s_sel;
//    QVector<QVector<int>> scanRange;                                                                                                                  ;
//    QVector<QVector<int>> xlimits;
    double x_min_picked = -1;
    double x_max_picked = -1;


    QListWidget *lw_inpData;
    QListWidget *lw_selData;
    QComboBox *cb_scan;

    QPushButton *pb_rmItem;
    QPushButton *pb_clsList;
    QPushButton *pb_pick_rp;
    QCheckBox *ckb_rawData;
    QCheckBox *ckb_integral;
    QCheckBox *ckb_scale;
    QCheckBox *ckb_xrange;
    US_PlotPicker *picker;

    void set_sel_data(void);
    QMap<QString, QVector<QVector<double>>> trapz(QVector<double>,
                                                  QVector<QVector<double>>);
    QVector<double> get_xlimit(QVector<double>, double, double, int *, int *);

private slots:
    void slt_load_auc(void);
    void slt_plot(bool);
    void slt_scan(double);
    void slt_addRmItem(QListWidgetItem *);
    void slt_rmItem(void);
    void slt_clsList(void);
    void slt_pick_point(void);
    void slt_mouse(const QwtDoublePoint& point);
    void slt_xrange(int);
};


#endif // US_PEAK_DECOMPOSITION_H
