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



    US_Plot* usplot1;
    QwtPlot* plot1;
    US_Plot* usplot2;
    QwtPlot* plot2;
    QwtPlotGrid* grid;

    int scanid;
    int nscans;
    QStringList filenames;
    QStringList filePaths;
    QStringList selFilenames;
    QVector<QVector<double>> xvalues;
    QVector<QVector<QVector<double>>> yvalues;
    QVector<QVector<int>> scanRange;                                                                                                                  ;
    QVector<QVector<int>> xlimits;

    QListWidget *lw_inpData;
    QListWidget *lw_selData;
    QComboBox *cb_scan;

    QPushButton *pb_rmItem;
    QPushButton *pb_clsList;


private slots:
    void slt_load_auc(void);
    void slt_plot1(bool);
    void slt_scan(double);
    void slt_addRmItem(QListWidgetItem *);
    void slt_rmItem(void);
    void slt_clsList(void);
};


#endif // US_PEAK_DECOMPOSITION_H
