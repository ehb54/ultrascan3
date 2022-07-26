#ifndef US_ABDE_ANALYSIS_H
#define US_ABDE_ANALYSIS_H

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_images.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_minimize.h"
#include "qwt_legend.h"

class US_PeakDecomposition : public US_Widgets{
    Q_OBJECT
public:
    explicit US_PeakDecomposition();

private:
    QPushButton* pb_load;
    QPushButton* pb_reset;
    QPushButton* pb_close;
    QPushButton* pb_save;

    US_Plot* usplot;
    QwtPlot* plot;
    QwtPlotGrid* grid;

    int scanid;
    int nscans;
    QStringList filenames;
    QStringList filePaths;
    QStringList selFilenames;
    QVector<QVector<double>> xvalues;
    QVector<QVector<QVector<double>>> yvalues;
    QVector<QVector<double>> xvalues_sel;
    QVector<QVector<double>> midxval_sel;
    QVector<QVector<QVector<double>>> yvalues_sel;
    QVector<QVector<QVector<double>>> integral_sel;
    QVector<QVector<QVector<double>>> integral_s_sel;
    double x_min_picked = -1;
    double x_max_picked = -1;

    QListWidget *lw_inpData;
    QListWidget *lw_selData;
    QComboBox *cb_scan;

    QPushButton *pb_rmItem;
    QPushButton *pb_cleanList;
    QPushButton *pb_pick_rp;
    QCheckBox *ckb_rawData;
    QCheckBox *ckb_integral;
    QCheckBox *ckb_scale;
    QCheckBox *ckb_legend;
    QCheckBox *ckb_xrange;
    US_PlotPicker *picker;
//    QwtLegend *legend;

    void selectData(void);
    void plotData(void);
    QMap<QString, QVector<QVector<double>>> trapz(QVector<double>,
                                                  QVector<QVector<double>>);
    QVector<double> getXlimit(QVector<double>, double, double, int *, int *);

private slots:
    void slt_loadAUC(void);
    void slt_scan(double);
    void slt_addRmItem(QListWidgetItem *);
    void slt_rmItem(void);
    void slt_cleanList(void);
    void slt_pickPoint(void);
    void slt_mouse(const QwtDoublePoint& point);
    void slt_xrange(int);
    void slt_legend(int);
    void slt_integral(int);
    void slt_scale(int);
    void slt_rawData(int);
    void slt_reset(void);
    void slt_save(void);
};


#endif // US_ABDE_ANALYSIS_H
