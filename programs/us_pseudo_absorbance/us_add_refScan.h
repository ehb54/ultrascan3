//! \file us_pseudo_absorbance.h
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

/**
 * @class US_AddRefScan
 * @brief The US_AddRefScan class handles the addition and management of reference scans in the application.
 */
class US_AddRefScan : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_AddRefScan.
         */
        US_AddRefScan();
        QPushButton* pb_close; ///< Close button
        bool hasData; ///< Flag to check if data is available

        signals:
                /**
                 * @brief Signal to plot left graph.
                 * @param flag Boolean flag
                 */
                void sig_plot_l(bool flag);

            /**
             * @brief Signal to plot right graph.
             * @param flag Boolean flag
             */
            void sig_plot_r(bool flag);

            /**
             * @brief Signal to plot distance.
             * @param flag Boolean flag
             */
            void sig_plot_dist(bool flag);

            /**
             * @brief Signal to plot overlap.
             * @param flag Boolean flag
             */
            void sig_plot_ovlp(bool flag);

            /**
             * @brief Signal to plot intensity.
             * @param flag Boolean flag
             */
            void sig_plot_inty(bool flag);

            /**
             * @brief Signal for clustering.
             * @param id Cluster ID
             */
            void sig_cluster(int id);

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
        // void slt_db_local(bool);
        void slt_reset(void);
        void slt_CA_state(int);
        void slt_load_CA(void);

    private:
        /**
         * @class currentValues
         * @brief Holds the current values for reference scan calculations.
         */
        class currentValues {
            public:
                QVector<int> scid; ///< Scan IDs
                QVector<double> mean; ///< Mean values
                QVector<double> rmsd; ///< RMSD values
                QVector<double> ref; ///< Reference values
                QVector<QVector<double>> dev; ///< Deviation values
                QVector<QVector<double>> dev_aln; ///< Aligned deviation values
                double aveMean; ///< Average mean value

                QVector<int> scid_S; ///< Secondary scan IDs
                QVector<double> mean_S; ///< Secondary mean values
                QVector<double> rmsd_S; ///< Secondary RMSD values
                QVector<double> ref_S; ///< Secondary reference values
                QVector<QVector<double>> dev_S; ///< Secondary deviation values
                QVector<QVector<double>> dev_S_aln; ///< Secondary aligned deviation values
                double aveMean_S; ///< Secondary average mean value

                /**
                 * @brief Clears all current values.
                 */
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

        /**
         * @class refScanTableInfo
         * @brief Holds information about reference scan table entries.
         */
        class refScanTableInfo {
            public:
                int id; ///< Table ID
                int instrumentID; ///< Instrument ID
                int personID; ///< Person ID
                QString type; ///< Type of scan
                QVector<int> experimentIDs; ///< Experiment IDs
                QDate referenceTime; ///< Reference time
                int nWavelength; ///< Number of wavelengths
                int nPoints; ///< Number of points
                double startWavelength; ///< Start wavelength
                double stopWavelength; ///< Stop wavelength
                bool null_blob; ///< Null blob flag
                QDate lastUpdated; ///< Last updated date
        };

        enum cls_state { OFF, ON, DONE }; ///< Enumeration for cluster state

        currentValues current; ///< Current values
        QMap<QString, double> plotParams; ///< Plot parameters

        QPushButton* pb_prev_id; ///< Previous ID button
        QPushButton* pb_next_id; ///< Next ID button
        QPushButton* pb_import; ///< Import button
        QPushButton* pb_reset; ///< Reset button
        QPushButton* pb_clscltr; ///< Cluster control button
        QPushButton* pb_reset_bws; ///< Reset BWS button
        QPushButton* pb_find_merge; ///< Find and merge button
        QPushButton* pb_save; ///< Save button
        QPushButton* pb_loadCA; ///< Load CA button

        QLineEdit* le_lambstrt; ///< Line edit for start wavelength
        QLineEdit* le_lambstop; ///< Line edit for stop wavelength
        QLineEdit* le_status; ///< Line edit for status
        // QLineEdit* le_dir; ///< Line edit for directory
        // QLineEdit* le_dbName; ///< Line edit for database name

        // QLabel* lb_dbName; ///< Label for database name
        // QLabel* lb_dir; ///< Label for directory
        QLabel* lb_wavlBw; ///< Label for wavelength bandwidth
        QLabel* lb_runIDs; ///< Label for run IDs
        QLabel* lb_wavl; ///< Label for wavelength

        QComboBox* cb_plot_id; ///< Combo box for plot ID

        QCheckBox* ckb_cluster; ///< Check box for cluster
        QCheckBox* ckb_align; ///< Check box for align
        QCheckBox* ckb_bws_all; ///< Check box for BWS all
        QCheckBox* ckb_CA_state; ///< Check box for CA state
        QCheckBox* ckb_CA_local; ///< Check box for CA local

        QwtCounter* ct_bws; ///< BWS counter
        QwtCounter* ct_winlen; ///< Window length counter

        QwtPlot* tab0_plotLU; ///< Top left plot on tab 0
        QwtPlot* tab0_plotRU; ///< Top right plot on tab 0
        QwtPlot* tab0_plotLD; ///< Bottom left plot on tab 0
        QwtPlot* tab0_plotRD; ///< Bottom right plot on tab 0
        QwtPlot* tab1_plotLU; ///< Top left plot on tab 1
        QwtPlot* tab1_plotRU; ///< Top right plot on tab 1
        QwtPlot* tab1_plotLD; ///< Bottom left plot on tab 1
        QwtPlot* tab1_plotRD; ///< Bottom right plot on tab 1
        QwtPlotGrid* grid; ///< Plot grid

        QRadioButton *rb_db; ///< Radio button for database
        QTabWidget* tabs; ///< Tab widget
        // US_Disk_DB_Controls* dkdb_ctrl; ///< Disk/DB controls
        US_Passwd pw; ///< Password object
        US_DB2* dbCon; ///< Database connection

        QStringList runIDs; ///< List of run IDs
        QString runType; ///< Run type
        int wavl_id; ///< Wavelength ID
        int winlen; ///< Window length
        const int winlen_dflt = 1; ///< Default window length
        const int winlen_max = 4; ///< Maximum window length
        const double bws_dflt = 2; ///< Default BWS
        int n_scans, n_wavls, n_points; ///< Number of scans, wavelengths, and points
        QVector<int> wavelength; ///< Wavelengths
        QVector<QVector<int>> wavlScid; ///< Wavelength scan IDs
        QVector<QVector<int>> wavlScid_S; ///< Secondary wavelength scan IDs
        QVector<double> xvalues; ///< X values
        QVector<QVector<double>> scanRvalues; ///< Scan R values
        QVector<int> scanWavl; ///< Scan wavelengths
        QVector<int> scanWavl_W; ///< Weighted scan wavelengths
        QVector<int> scanWavl_S; ///< Secondary scan wavelengths
        QVector<double> scanMean; ///< Scan mean values
        QVector<double> scanRmsd; ///< Scan RMSD values
        QVector<double> wavlBw; ///< Wavelength bandwidth
        QVector<double> wavlBwS; ///< Secondary wavelength bandwidth
        QVector<double> CAValues; ///< CA values
        QVector<QVector<QVector<double>>> clusterIDs; ///< Cluster IDs
        QVector<QVector<QVector<double>>> clusterRng; ///< Cluster ranges
        US_RefScanDataIO::RefData referenceScans; ///< Reference scans

        void set_wavl_ctrl(void);
        void write2txt(const QString&, US_RefScanDataIO::RefData&);
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

/**
 * @class FileNameWidget
 * @brief The FileNameWidget class provides a dialog for editing file names.
 */
class FileNameWidget : public US_WidgetsDialog {
    Q_OBJECT
    public:
        /**
         * @brief Constructor for FileNameWidget.
         * @param inputName Reference to the input file name
         */
        FileNameWidget(QString &inputName);

    private slots:
                void slt_ok(void);
        void slt_cancel(void);
        void slt_edit(QString);

    private:
        QString *fileName; ///< File name
        QString base_prev; ///< Previous base name
        QLineEdit *le_prep; ///< Line edit for prefix
        QLineEdit *le_base; ///< Line edit for base
        QLineEdit *le_apen; ///< Line edit for append
        QLineEdit *le_ext; ///< Line edit for extension
        QLineEdit *caution; ///< Caution line edit
        QLineEdit *le_final; ///< Final line edit
        QDir importDir; ///< Import directory

        QFileInfo check_fname(void);
};

/**
 * @class setRefTime
 * @brief The setRefTime class provides a dialog for setting reference times.
 */
class setRefTime : public US_WidgetsDialog {
    Q_OBJECT
    public:
        /**
         * @brief Constructor for setRefTime.
         * @param parent Parent widget
         * @param refDate Reference date
         */
        setRefTime(QWidget* parent, QDate& refDate);

    private slots:
        void slt_apply(void);
        void slt_set_days(int month);

    private:
        QDate *refDate; ///< Reference date
        QComboBox *cb_year; ///< Year combo box
        QComboBox *cb_month; ///< Month combo box
        QComboBox *cb_day; ///< Day combo box
};

#endif // US_ADD_REF_SCAN_H
