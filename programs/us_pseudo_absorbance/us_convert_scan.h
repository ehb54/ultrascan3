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


class US_ConvertScan : public US_Widgets
{
    Q_OBJECT

public:
    US_ConvertScan();
    QPushButton* pb_close;
    bool hasData;

    signals:
    void sig_plot(void);
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

    const double maxAbs = 3;

    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_import;
    QPushButton* pb_reset;
    QPushButton* pb_import_refScans;
    QPushButton* pb_reset_refData;
    QPushButton* pb_save;
    QPushButton* pb_pick_rp;

    QLabel* lb_lambstrt;
    QLabel* lb_lambstop;
    QLineEdit* le_runIdInt;
    US_LineEdit_RE* le_runIdAbs;
    QLineEdit* le_desc;
    QLineEdit* le_status;
    QLineEdit* le_ref_range;
    QLineEdit* le_xrange;
    QLabel* plot_title;
    QString runIdAbs;

    QComboBox* cb_plot_id;
    QComboBox* cb_buffer;

    US_PlotPicker *picker_abs;
    US_PlotPicker *picker_insty;
    US_Plot* usplot_insty;
    US_Plot* usplot_abs;
    QwtPlot* qwtplot_insty;
    QwtPlot* qwtplot_abs;
    QwtPlotGrid* grid;

    QwtCounter* ct_scan_l;
    QwtCounter* ct_scan_u;
    QwtCounter* ct_smooth;
    QwtCounter* ct_last_scans;

    QFrame *frm_refScan;

    QListWidget* lw_triple;
    QCheckBox *ckb_act_refscan;
    QCheckBox *ckb_zeroing;
    QCheckBox *ckb_xrange;
    QCheckBox *ckb_ChroAberCorr;
    US_Disk_DB_Controls* diskDB_ctrl;

    CCW ccwList;
    CCW ccwListMain;
    QStringList ccwStrListMain;
    CCW_ITEM ccwItemList;

    US_DB2 *dbCon;
    US_Passwd pw;
    int cpos, le_cursor_pos = -1;
    double x_min_picked, x_max_picked;
    int wavl_id;
    int n_wavls;
    int refId;
    QStringList intRunIds;
    QVector<int> intDataId;
    QVector<double> wavelength;
    QVector<double> xvalues;
    QVector<QVector<double>> xvaluesRefCAC;
    QVector<US_DataIO::Scan> intensity;
    QVector<US_DataIO::Scan> absorbance;
    QVector<US_DataIO::Scan> absorbanceBuffer;
    QVector<US_DataIO::RawData> allIntData;
    QVector<QVector<int>> scansRange;
    US_RefScanDataIO::RefData refData;
    QFileInfoList allIntDataFiles;

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

class LoadDBWidget : public US_WidgetsDialog{
    Q_OBJECT
public:
    LoadDBWidget(QWidget* w, US_DB2 *dbCon, US_RefScanDataIO::RefData&);

private slots:
    void slt_set_refTable(void);
    void slt_apply(void);
private:
    US_DB2 *db;
    US_RefScanDataIO::RefData *refData;
    QVector<int> instrumentIDs;
    QStringList instrumentNames;
    QVector<QVector<int>> instrumentRefList;
    QTreeWidget *tw_instruments;
    QTreeWidget *tw_refData;

    class refScanTableInfo{
    public:
        int id;
        int instrumentID;
        int personID;
        QString type;
        QStringList exprimentIds;
        QDate referenceTime;
        int nWavelength;
        int nPoints;
        double startWavelength;
        double stopWavelength;
        bool null_blob;
        QDate lastUpdated;
    };

    QVector<refScanTableInfo> refTable;
    QDate str2date(QString);
};

#endif // US_CONVERT_SCAN_H
