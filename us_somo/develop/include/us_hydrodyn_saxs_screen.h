#ifndef US_HYDRODYN_SAXS_SCREEN_H
#define US_HYDRODYN_SAXS_SCREEN_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qtablewidget.h>
#include <qtextedit.h>
// Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

// standard C and C++ defs:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>
#include <vector>

#include "qwt_wheel.h"
#include "us_hydrodyn_saxs.h"
#if QT_VERSION >= 0x040000
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#endif
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Screen : public QFrame {
  Q_OBJECT

  friend class US_Hydrodyn_Batch;
  friend class US_Hydrodyn_Saxs;

 public:
  US_Hydrodyn_Saxs_Screen(csv csv1, void *us_hydrodyn, QWidget *p = 0,
                          const char *name = 0);
  ~US_Hydrodyn_Saxs_Screen();

 private:
  csv csv1;

  void *us_hydrodyn;

  US_Config *USglobal;

  QLabel *lbl_title;

  QTableWidget *t_csv;

  QProgressBar *progress;
  QProgressBar *progress2;

  QPushButton *pb_push;
  QPushButton *pb_clear_plot_all;
  QPushButton *pb_clear_plot_row;
  QPushButton *pb_save_plot;
  QPushButton *pb_load_plot;

  QCheckBox *cb_plot_average;
  QCheckBox *cb_plot_best;
  QCheckBox *cb_plot_rg;
  QCheckBox *cb_plot_chi2;

  QPushButton *pb_replot_saxs;
  QPushButton *pb_save_saxs_plot;
  QPushButton *pb_set_target;
  QLabel *lbl_current_target;
  QCheckBox *cb_normalize;

  QPushButton *pb_start;
  QPushButton *pb_run_all_targets;
  QPushButton *pb_stop;

  QFont ft;
  QTextEdit *editor;
  QMenuBar *m;

  QPushButton *pb_help;
  QPushButton *pb_cancel;

  mQwtPlot *plot_dist;
  US_Plot *usp_plot_dist;
 private slots:
  void usp_config_plot_dist(const QPoint &);

 private:
  ScrollZoomer *plot_dist_zoomer;

  QwtWheel *qwtw_wheel;
  QwtWheel *qwtw_wheel2;
  QLabel *lbl_pos_range;
  QLabel *lbl_pos_range2;
  QLabel *lbl_message;
  QLabel *lbl_message2;
  QLabel *lbl_message3;
  QLabel *lbl_message4;

  bool order_ascending;

  void editor_msg(QString color, QString msg);
  void editor_msg_qc(QColor qcolor, QString msg);

  bool running;
  void update_enables();

  bool validate();
  bool any_to_run();

  US_Hydrodyn_Saxs *saxs_window;
  bool *saxs_widget;
  bool activate_saxs_window();
  bool validate_saxs_window();

  void do_replot_saxs();

  double best_fitness;

  QString saxs_header_iqq;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif
  vector<vector<double> > qs;
  vector<vector<double> > Is;
  vector<vector<double> > I_errors;
  vector<QString> names;

  vector<QString> csv_source_name_iqq;
  vector<double> saxs_q;
  vector<vector<double> > saxs_iqq;

  vector<vector<QString> > messages;
  vector<vector<QString> > messages2;
  vector<vector<QString> > messages3;
  vector<vector<QString> > messages4;
  vector<vector<vector<double> > > radiis;  // [sic]
  vector<vector<vector<double> > > intensitys;
  vector<vector<double> > best_fit_radiuss;
  vector<vector<double> > best_fit_delta_rhos;
  vector<vector<double> > average_radiuss;
  vector<vector<double> > average_delta_rhos;
  vector<vector<double> > target_rgs;
  vector<vector<bool> > use_chi2s;
  vector<vector<double> > chi2_bests;
  vector<vector<double> > chi2_nnlss;

  map<QString, double> guinier_rgs;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif
  csv current_csv();

  void recompute_interval_from_points();
  void recompute_points_from_interval();

  void plot_one(QString message, QString message2, QString message3,
                QString message4, vector<double> radii,
                vector<double> intensity, double best_fit_radius,
                double best_fit_delta_rho, double average_radius,
                double average_delta_rho, double rg, bool use_chi2,
                double chi2_best, double chi2_nnls);

  void plot_pos(unsigned int);

  unsigned int last_plotted_pos;
  unsigned int current_row;

  void plot_row(unsigned int);
  void clear_plot();
  void update_wheel_range();

  double max_x_range;
  double max_y_range;

  bool anything_plotted();
  bool anything_plotted_since_save;
  csv plots_to_csv();
  void csv_to_plots(csv plot_csv);
  QStringList csv_parse_line(QString qs);
  void set_target(QString target);

  bool get_guinier_rg(QString name, double &rg);
  unsigned int last_target_pos;
  bool last_target_found;

  unsigned int pen_width;

 private slots:

  void setupGUI();

  void table_value(int, int);

  void push();
  void clear_plot_row();
  void clear_plot_all();
  void save_plot();
  void load_plot();

  void replot();

  void replot_saxs();
  void save_saxs_plot();
  void set_target();

  void adjust_wheel(double);
  void adjust_wheel2(double);

  void start(bool already_running = false);
  void run_all_targets();
  void stop();

  void clear_display();
  void update_font();
  void save();

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

#endif
