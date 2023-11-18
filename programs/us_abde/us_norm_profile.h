#ifndef US_NORM_PROFILE_H
#define US_NORM_PROFILE_H

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_images.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_minimize.h"
#include "qwt_legend.h"


class US_Norm_Profile : public US_Widgets{
    Q_OBJECT
public:
    explicit US_Norm_Profile();

signals:
    void widgetClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QPushButton* pb_load;
    QPushButton* pb_reset;
    QPushButton* pb_close;
    QPushButton* pb_save;

    QLineEdit* le_investigator;
    QLineEdit* le_runinfo;

    US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

    US_Plot* usplot;
    QwtPlot* plot;
    QwtPlotGrid* grid;

    int scanid;
    int nscans;
    QStringList filenames;
    QStringList filePaths;
    QStringList selFilenames;
    QVector<QVector<double>> xvalues;
    QVector<QVector<double>> yvalues;
    QVector<QVector<double>> xvalues_sel;
    QVector<QVector<double>> midxval_sel;
    QVector<QVector<double>> yvalues_sel;
    QVector<QVector<double>> yvaluesN_sel;
    QVector<QVector<double>> integral_sel;
    QVector<QVector<double>> integralN_sel;
    double x_min_picked = -1;
    double x_max_picked = -1;

    QListWidget *lw_inpData;
    QListWidget *lw_selData;

    QPushButton *pb_rmItem;
    QPushButton *pb_cleanList;
    QPushButton *pb_pick_rp;
    QCheckBox *ckb_rawData;
    QCheckBox *ckb_integral;
    QCheckBox *ckb_norm;
    QCheckBox *ckb_legend;
    QCheckBox *ckb_xrange;
    US_PlotPicker *picker;

    void selectData(void);
    void plotData(void);
    QMap<QString, QVector<double>> trapz(QVector<double>,
                                                  QVector<double>);
    QVector<double> getXlimit(QVector<double>, double, double, int *, int *);
    void enableWidgets(bool);

private slots:
    void slt_loadAUC(void);
    void slt_addRmItem(QListWidgetItem *);
    void slt_rmItem(void);
    void slt_inItemSel(int);
    void slt_outItemSel(int);
    void slt_cleanList(void);
    void slt_pickPoint(void);
    void slt_mouse(const QwtDoublePoint& point);
    void slt_xrange(int);
    void slt_legend(int);
    void slt_integral(int);
    void slt_norm(int);
    void slt_rawData(int);
    void slt_reset(void);
    void slt_save(void);
};

#endif // US_NORM_PROFILE_H
