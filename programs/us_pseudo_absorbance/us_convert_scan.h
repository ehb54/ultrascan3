#ifndef US_CONVERT_SCAN_H
#define US_CONVERT_SCAN_H
#include <us_widgets.h>
#include <us_settings.h>
#include <us_dataIO.h>
#include "us_refScan_dataIO.h"
#include "us_plot.h"
#include "us_pabs_common.h"
#include "us_db2.h"
// #include "us_passwd.h"
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

private slots:
    void import_run();
    void set_wavl_ctrl();
    void reset();
    void select_id(int);
    void prev_id();
    void next_id();
    void plot_all();
    void plot_absorbance();
    void select_refscan(int);
    void update_nscans();
    void apply_nscans();
    void save_run(void);
    void pick_region(void);
    void mouse_click(const QwtDoublePoint&);
    void default_region();
    void lower_scan_range(double);
    void upper_scan_range(double);
    void plot_ref_state();
    void load_ref_scan();

private:

    class RefscanFile
    {
    public:
        RefscanFile() {}

        QString filename;
        QString name;
        double min_wvl;
        double max_wvl;
        int nwvl;
        int wavl_id;
        QVector<double> wavelength;
        QVector<double> xvalues;
        QVector<QVector<double>> yvalues;
    };

    class CellChannel
    {
    public:
        CellChannel() {};

        char channel;
        int cell;
        int ref_id;
        QString runid;
        int wavl_id;
        QVector<double> wavelength;
        QVector<int> rawdata_ids;
        QPair<double, double> minmax_x;
    };

    US_Disk_DB_Controls* disk_controls;
    QPushButton* pb_prev_id;
    QPushButton* pb_next_id;
    QPushButton* pb_import;
    QPushButton* pb_reset;
    QPushButton* pb_load_ref;
    QPushButton* pb_set_sample;
    QPushButton* pb_save;
    QPushButton* pb_pick_rp;
    QPushButton* pb_apply;

    QLineEdit* le_lambstrt;
    QLineEdit* le_lambstop;
    QLineEdit* le_runid;
    QLineEdit* le_desc;
    QLineEdit* le_status;
    QLabel* plot_title;
    QComboBox* cb_plot_id;

    US_PlotPicker *picker_abs;
    US_PlotPicker *picker_insty;
    US_Plot* usplot_insty;
    US_Plot* usplot_abs;
    QwtPlot* qwtplot_insty;
    QwtPlot* qwtplot_abs;
    QwtPlotGrid* grid;
    QCheckBox* chkb_abs_int;

    // QwtCounter* ct_smooth;
    QwtCounter* ct_maxod;
    QwtCounter* ct_scans;
    QwtCounter* ct_scan_from;
    QwtCounter* ct_scan_to;

    QTableWidget* tb_triple;
    QFont font;

    bool abs_plt_on;
    int nscans;
    int max_nscans;
    int smooth;
    QVector<CellChannel> ccw_items;
    QVector<US_DataIO::RawData> intensity_data;
    QVector<QVector<QVector<double>>> absorbance_data;
    QVector<QVector<QVector<double>>> refscan_data;
    QVector<QVector<double>> absorbance_shifts;
    QVector<RefscanFile> refscan_files;
    QVector<int> absorbance_state;

    void list_ccw_items(QString&);
    void set_table();
    void highlight();
    void offon_prev_next();
    void plot_intensity();
    void plot_refscan();

    void calc_absorbance(int);
    void trim_absorbance(QVector<QVector<double>>&, QVector<double>&, bool);
    void update_shifts(int);
    bool get_refval_file(int, int);
    bool get_refval_buffer(int, int);
    QVector<double> smooth_refscan(QVector<double>, int, bool, bool, double);
    bool linear_interpolation(const QVector<double>&, QVector<double>&, QVector<double>&);
    void disconnect_picker();
    bool set_abs_runid(QString&);
    void set_ct_scans(int = 0);
    void align_center(QwtCounter*);
    int read_auc(QVector<US_DataIO::RawData>&, QVector<QVector<QVector<double>>>&,
                  QVector<QVector<QVector<double>>>&, QVector<QVector<double>>&, QString&);

};

#endif // US_CONVERT_SCAN_H
