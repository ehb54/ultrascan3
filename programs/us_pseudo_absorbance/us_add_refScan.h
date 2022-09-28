#ifndef US_ADD_REF_SCAN_H
#define US_ADD_REF_SCAN_H

#include "us_widgets.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "qwt_plot_marker.h"
#include "us_refScan_dataIO.h"
#include "us_settings.h"
#include "us_util.h"
#include "us_images.h"
//#include "us_extern.h"
//#include "us_help.h"


class US_AddRefScan : public US_Widgets
{
    Q_OBJECT

public:
    US_AddRefScan();
    QPushButton* pb_close;
    bool hasData;

    signals:
    void sig_plot_l(bool);
    void sig_plot_r(bool);
    void sig_plot_dist(bool);
    void sig_plot_ovlp(bool);
    void sig_plot_inty(bool);
    void sig_cluster(int);

private slots:
    void slt_import(void);
    void slt_prev_id(void);
    void slt_next_id(void);
    void slt_new_bws(double);
    void slt_new_wlen(double);
    void slt_set_id(int);
    void slt_cls_ctrl(void);
    void slt_cls_state(int);
    void slt_cluster(int);
    void slt_reset_bws(void);
    void slt_find_merge(void);
    void slt_aln_state(int);
    void slt_plot_l(bool);
    void slt_plot_r(bool);
    void slt_plot_dist(bool);
    void slt_plot_ovlp(bool);
    void slt_plot_inty(bool);
    void slt_save(void);
    void slt_db_local(bool);
    void slt_reset(void);
    void slt_CA_state(int);
    void slt_load_CA(void);


private:
    class currentValues{
    public:
        QVector<int> scid;
        QVector<double> mean;
        QVector<double> rmsd;
        QVector<double> ref;
        QVector<QVector<double>> dev;
        QVector<QVector<double>> dev_aln;
        double aveMean;

        QVector<int> scid_S;
        QVector<double> mean_S;
        QVector<double> rmsd_S;
        QVector<double> ref_S;
        QVector<QVector<double>> dev_S;
        QVector<QVector<double>> dev_S_aln;
        double aveMean_S;

        void clear(void){
            scid.clear();
            mean.clear();
            rmsd.clear();
            ref.clear();
            dev.clear();
            dev_aln.clear();
            aveMean = 0;

            scid_S.clear();
            mean_S.clear();
            rmsd_S.clear();
            ref_S.clear();
            dev_S.clear();
            dev_S_aln.clear();
            aveMean_S = 0;
        }
    };

    class refScanTableInfo{
    public:
        int id;
        int instrumentID;
        int personID;
        QString type;
        QVector<int> experimentIDs;
        QDate referenceTime;
        int nWavelength;
        int nPoints;
        double startWavelength;
        double stopWavelength;
        bool null_blob;
        QDate lastUpdated;
    };

    enum cls_state{OFF, ON, DONE};

    currentValues current;
    QMap<QString, double> plotParams;

    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_import;
    QPushButton* pb_reset;
    QPushButton* pb_clscltr;
    QPushButton* pb_reset_bws;
    QPushButton* pb_find_merge;
    QPushButton* pb_save;
    QPushButton* pb_loadCA;

    QLineEdit* le_lambstrt;
    QLineEdit* le_lambstop;
    QLineEdit* le_status;
    QLineEdit* le_dir;
    QLineEdit* le_dbName;

    QLabel* lb_dbName;
    QLabel* lb_dir;
    QLabel* lb_wavlBw;
    QLabel* lb_runIDs;
    QLabel* lb_wavl;

    QComboBox* cb_plot_id;

    QCheckBox* ckb_cluster;
    QCheckBox* ckb_align;
    QCheckBox* ckb_bws_all;
    QCheckBox* ckb_CA_state;
    QCheckBox* ckb_CA_local;

    QwtCounter* ct_bws;
    QwtCounter* ct_winlen;

    QwtPlot* tab0_plotLU;
    QwtPlot* tab0_plotRU;
    QwtPlot* tab0_plotLD;
    QwtPlot* tab0_plotRD;
    QwtPlot* tab1_plotLU;
    QwtPlot* tab1_plotRU;
    QwtPlot* tab1_plotLD;
    QwtPlot* tab1_plotRD;
    QwtPlotGrid* grid;

    QRadioButton *rb_db;
    QTabWidget* tabs;
    US_Disk_DB_Controls* dkdb_ctrl;
    US_Passwd pw;
    US_DB2* dbCon;

    QStringList runIDs;
    QString runType;
    int wavl_id;
    int winlen;
    const int winlen_dflt = 1;
    const int winlen_max = 4;
    const double bws_dflt = 2;
    int n_scans, n_wavls, n_points;
    QVector<int> wavelength;
    QVector<QVector<int>> wavlScid;
    QVector<QVector<int>> wavlScid_S;
    QVector<double> xvalues;
    QVector<QVector<double>> scanRvalues;
    QVector<int> scanWavl;
    QVector<int> scanWavl_W;
    QVector<int> scanWavl_S;
    QVector<double> scanMean;
    QVector<double> scanRmsd;
    QVector<double> wavlBw;
    QVector<double> wavlBwS;
    QVector<double> CAValues;
    QVector<QVector<QVector<double>>> clusterIDs;
    QVector<QVector<QVector<double>>> clusterRng;
    US_RefScanDataIO::RefData referenceScans;

    void set_wavl_ctrl(void);
    void write2txt(const QString&, US_RefScanDataIO::RefData& );
    bool parse_files(QStringList);
    QVector<double> linspace(double start, double stop, int num);
    QVector<int> arange(int start, int stop, int step);
    QVector<int> arange(int start, int stop);
    QVector<int> arange(int stop);
    double pdf(double x, double mu, double sigma);
    double variance(QVector<double> x);
    double bwe_norm(QVector<double> x);
    double sj_param(QVector<QVector<double>> *W, int size_x,
                    double h, double tdb, double sda);
    double bwe_sj(QVector<double> x);
    double sj_phi6(double x);
    double sj_phi4(double x);
    double quantile(QVector<double> x, double q);
    double skewness(QVector<double> x);
    QVector<QVector<double>> kde(QVector<double> x, double bw);
    QMap<QString, QVector<int>> loc_minmax(QVector<double> sdiff);
    void get_clusters_i(QVector<double> rmsd_i, QVector<int> ids_i, double wl,
                        QVector<QVector<double>> kde_out,
                        QMap<QString, QVector<int>> minmax_loc);
    QVector<double> get_ref_rval(QVector<int> ids);
    QVector<QVector<QVector<double>>> get_ref_dev(QVector<int> ids, QVector<double> ref,
                                                  QVector<double> mean, double aveMean);
    QVector<double> get_std(QVector<QVector<double>> dev);
    void mean_rmsd(void);
    QVector<bool> trimming(QVector<double> rmsd, double threshold);
    QVector<bool> cropping(QVector<double> rmsd, QVector<bool>tsk, double margin);
    void estimate_bw(void);

    void get_current(int id);
    void get_plot_params(void);
    void find_clusters(void);
    void merge_clusters(void);
    void save_local(US_RefScanDataIO::RefData&);
    void save_db(US_RefScanDataIO::RefData&);
    void check_connection(void);
    bool check_runIDs(US_DB2*, QVector<int>&, QVector<QDate>&, int&);
    void get_refScanDBinfo(US_DB2*, QVector<refScanTableInfo>&);
    QDate str2date(QString);

};


class FileNameWidget : public US_WidgetsDialog{
    Q_OBJECT
public:
    FileNameWidget(QString &inputName);

private slots:
    void slt_ok(void);
    void slt_cancel(void);
    void slt_edit(QString);
private:
    QString *fileName;
    QString base_prev;
    QLineEdit *le_prep;
    QLineEdit *le_base;
    QLineEdit *le_apen;
    QLineEdit *le_ext;
    QLineEdit *caution;
    QLineEdit *le_final;
    QDir importDir;

    QFileInfo check_fname(void);
};


class setRefTime : public US_WidgetsDialog{
    Q_OBJECT
public:
    setRefTime(QWidget*, QDate&);
private slots:
    void slt_apply(void);
    void slt_set_days(int month);
private:
    QDate *refDate;
    QComboBox *cb_year;
    QComboBox *cb_month;
    QComboBox *cb_day;

};


#endif
