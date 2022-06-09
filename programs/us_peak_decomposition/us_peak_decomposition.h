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
    QPushButton* pb_load_auc;
    QPushButton* pb_load_txt;
    QPushButton* pb_save;
    QPushButton* pb_reset;

    QwtCounter* ct_scans;
    QLabel* lb_descr;

    US_Plot* usplot1;
    QwtPlot* plot1;
    US_Plot* usplot2;
    QwtPlot* plot2;
    QwtPlotGrid* grid;

    int scanid;
    int nscans;
    QStringList descr;
    QVector<QVector<double>> xvalues;
    QVector<QVector<double>> yvalues;
    QVector<QVector<int>> xlimits;


private slots:
    void slt_load_auc(void);
    void slt_plot1(bool);
    void slt_scan(double);
};


#endif // US_PEAK_DECOMPOSITION_H
