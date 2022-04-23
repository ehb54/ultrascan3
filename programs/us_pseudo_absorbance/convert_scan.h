#ifndef CONVERT_SCAN_H
#define CONVERT_SCAN_H
#include <us_widgets.h>
#include <us_settings.h>
#include <us_dataIO.h>
#include <../us_convert/us_select_triples.h>
#include <refScan_dataIO.h>
#include "us_plot.h"
#include "build_colormap.h"
#include "us_db2.h"
#include "us_passwd.h"

class convertScan : public US_Widgets
{
    Q_OBJECT

public:
    convertScan();

    signals:
    void sig_plot(void);
    void sig_save_button(void);

private slots:
    void slt_import(void);
    void slt_set_ccw_default(void);
    void slt_del_item(void);
    void slt_del_ccws(void);
    void slt_new_lambda_ctrl(int);
    void slt_reset(void);
    void slt_set_id(int);
    void slt_prev_id(void);
    void slt_next_id(void);
    void slt_plot(void);
//    void slt_edit_runid(QString);
    void slt_load_refScans(void);
    void slt_new_scan_range(double);
    void slt_check_box(int);
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
//    void slt_cpos(int, int);

private:

    class CCW_LIST{
    public:
        QList<int> cell;
        QList<char> channel;
        QList<QVector<double>> wavelength;
        QList<QVector<int>> index;
        QList<double> min_wl;
        QList<double> max_wl;
        QList<int> n_wl;

        void clear(void){
            cell.clear();
            channel.clear();
            wavelength.clear();
            index.clear();
            min_wl.clear();
            max_wl.clear();
            n_wl.clear();
        }

        int size(void){
            return cell.size();
        }
    };


    class CCW{
    public:
        QVector<int> index;
        QVector<int> cell;
        QVector<char> channel;
        QVector<double> wavelength;

        void clear(void){
            cell.clear();
            channel.clear();
            wavelength.clear();
            index.clear();
        }

        int size(void){
            return cell.size();
        }
    };
    const double abs_max = 3;
    CCW ccwList;
    CCW ccwListMain;
    QStringList ccwStrListMain;
    CCW_LIST ccwItemList;

    refScanDataIO::RefData refData;

    int lambda_id;
    int n_wavelengths;
    int refId;
    QVector<int> intDataId;
    QVector<double> wavelength;
    QString status, percent;
    QLineEdit* le_lambstrt;
    QLineEdit* le_lambstop;
    QLineEdit* le_runIdInt;
    QLineEdit* le_dir;
    QLineEdit* le_runIdAbs;
    QLineEdit* le_desc;
    QListWidget*  lw_triple;
    QLabel* plot_title;
    QComboBox* cb_plot_id;
    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_import;
    QPushButton* pb_import_refScans;
    QPushButton* pb_reset_refData;
    US_Disk_DB_Controls* diskDB_ctrl;
    QLineEdit* le_status;
    QPushButton* pb_reset;
    US_Plot* usplot_insty;
    QwtPlot* qwtplot_insty;
    int le_cursor_pos = -1;
    QVector<double> xvalues;
    QVector<US_DataIO::Scan> intensity;
    QVector<US_DataIO::Scan> absorbance;
    QVector<US_DataIO::Scan> absorbance_buffer;

    US_Plot* usplot_abs;
    QwtPlot* qwtplot_abs;
    QPushButton* pb_save;

    QwtCounter* ct_lower;
    QwtCounter* ct_upper;
    QString runID;
    QString runType;
    QString currentDir;
    QVector<US_DataIO::RawData> allIntData;
    QVector<QVector<int>> scans_range;
    QFileInfoList allIntDataFiles;
    QCheckBox *ckb_shift_zero;
    QPushButton *pb_pick_rp;
    QwtPlotGrid* grid;
    int cpos;
    QComboBox *cb_buffer;
    QwtCounter* ct_smooth;
    QLineEdit* le_ref_range;
    double x_min_picked, x_max_picked;
    QLineEdit *le_lower_x;
    QLineEdit *le_upper_x;
    US_PlotPicker *picker;
    US_DB2 *dbCon;
    US_Passwd pw;


    void clear(void);

    void make_ccwItemList(void);
    void set_lambda_ctrl(void);
    void set_widgetList(void);
    void offon_prev_next(void);
    void plot_intensity(void);
    void plot_refscan(void);
    void plot_absorbance(void);
    void set_scan_range(void);

    void get_absorbance(int, int, bool buffer);
    void get_intensity(int);
    bool get_refId(double);
    void set_buffer_list(void);
    void get_relative_absorbance(int);
    void shift_absorbance(void);
    QVector<double> get_smooth(QVector<double>, int);
    void load_from_DB(void);

};

class LoadDBWidget : public US_WidgetsDialog{
    Q_OBJECT
public:
    LoadDBWidget(QWidget* w, US_DB2 *dbCon, refScanDataIO::RefData&);

private slots:
    void slt_set_refTable(void);
    void slt_apply(void);
//    void slt_cancel(void);
private:
    US_DB2 *db;
    refScanDataIO::RefData *refData;
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


#endif // CONVERT_SCAN_H
