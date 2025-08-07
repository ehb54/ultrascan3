#ifndef US_HYDRODYN_SAXS_CORMAP_H
#define US_HYDRODYN_SAXS_CORMAP_H

#include "qfile.h"
#include "qfiledialog.h"
#include "qfontmetrics.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlineedit.h"
#include "qprogressbar.h"
#include "qstring.h"
#include "qtextedit.h"
// Added by qt3to4:
#include <qwt_plot.h>
#include <qwt_thermo.h>

#include <QCloseEvent>
#include <QFrame>
#include <map>
#include <set>
#include <vector>

#include "us3i_plot.h"
#include "us_util.h"

#if QT_VERSION >= 0x040000
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#else
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#endif

#include "us_mqt.h"

using namespace std;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

class uhs_index_pair {
 public:
  int r;
  int c;
  bool operator<(const uhs_index_pair &objIn) const {
    return r < objIn.r || (r == objIn.r && c < objIn.c);
  }
  bool operator==(const uhs_index_pair &objIn) const {
    return r == objIn.r && c == objIn.c;
  }
};

class US_Hydrodyn_Saxs_Cormap_Cluster_Analysis {
 public:
  bool run(vector<vector<double> > &pvaluepairs,
           map<QString, QString> &parameters, map<QString, QString> &csv_report,
           QWidget *parent = 0);

  bool sliding(vector<vector<double> > &pvaluepairs,
               map<QString, QString> &parameters,
               map<QString, double> &sliding_results, QWidget *parent = 0,
               QProgressBar *progress = 0);

  bool sliding(vector<vector<double> > &pvaluepairs,
               map<QString, QString> &parameters,
               map<QString, double> &sliding_results,
               map<QString, double> &hb_sliding_results, QWidget *parent = 0,
               QProgressBar *progress = 0);

  map<int, int> cluster_size_histogram;
  QString errormsg;

 private:
  void cluster_expand(uhs_index_pair x);
  map<uhs_index_pair, int> cluster_data;
  set<uhs_index_pair> cluster_marked;
  set<uhs_index_pair> cluster_red;
};

class US_EXTERN US_Hydrodyn_Saxs_Cormap : public QFrame {
  Q_OBJECT

 public:
  US_Hydrodyn_Saxs_Cormap(void *us_hydrodyn, map<QString, QString> parameters,
                          vector<vector<double> > pvaluepairs,
                          vector<vector<double> > adjpvaluepairs,
                          vector<QString> selected_files, QWidget *p = 0,
                          const char *name = 0);

  ~US_Hydrodyn_Saxs_Cormap();

  static bool load_csv(QString filename, map<QString, QString> &parameters,
                       vector<vector<double> > &pvaluepairs,
                       vector<vector<double> > &adjpvaluepairs,
                       vector<QString> &selected_files,
                       QWidget *parent = (QWidget *)0);

 private:
  US_Config *USglobal;

  QCheckBox *cb_adj;
  QCheckBox *cb_hb;

  QPushButton *pb_sliding;
  QPushButton *pb_help;
  QPushButton *pb_save_csv;
  QPushButton *pb_load_csv;
  QPushButton *pb_close;
  void *us_hydrodyn;
  map<QString, QString> parameters;

  void setupGUI();

  QLabel *lbl_title;

  mQLabel *lbl_image;

  QFrame *f_brookesmap;

  QwtThermo *f_thermo_top;
  QwtThermo *f_thermo_left;

  QLabel *lbl_f_title;
  QLabel *lbl_f_none;
  QLabel *lbl_f_image;

  int last_height;
  int last_width;
  int last_mode;

  QImage *qi;
  QImage *qi_adj;
  QImage *qi_hb;

  mQwtPlot *plot;
  US_Plot *usp_plot;
 private slots:
  void usp_config_plot(const QPoint &);

 private:
  ScrollZoomer *plot_zoomer;
#if QT_VERSION >= 0x040000
  QwtPlotGrid *plot_grid;
#endif

  mQwtPlot *plot_cluster;
  US_Plot *usp_plot_cluster;
 private slots:
  void usp_config_plot_cluster(const QPoint &);

 private:
  ScrollZoomer *plot_cluster_zoomer;
#if QT_VERSION >= 0x040000
  QwtPlotGrid *plot_cluster_grid;
#endif

  vector<vector<double> > pvaluepairs;
  vector<vector<double> > adjpvaluepairs;
  vector<QString> selected_files;
  double alpha;
  double alpha_over_5;
  double hb_alpha;
  double hb_alpha_over_5;

  QFont ft;
  QTextEdit *editor;
  // QMenuBar      *m;
  void editor_msg(QString color, QString msg);

  bool streak_check(vector<int> &row1, vector<int> &row2, int &N, int &S,
                    int &C, double &P);

  bool cluster_analysis();

  bool big_green_box_analysis();

  QStringList csv_headers;
  map<QString, QString> csv_report;

  static void csv_corrupt_msg(QString file, int line,
                              QWidget *parent = (QWidget *)0);

  void displayData();

 private slots:

  void clear_display();
  void update_font();
  void save();

  void imageResized();
  void forceImageResized();
  void help();
  void sliding();
  void save_csv();
  void load_csv();
  void cancel();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
