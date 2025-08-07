#ifndef US_HYDRODYN_SAXS_BUFFER_H
#define US_HYDRODYN_SAXS_BUFFER_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//#include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qprinter.h>
#include <qprogressbar.h>
#include <qradiobutton.h>
#include <qtablewidget.h>
#include <qtextedit.h>
#include <qwt_plot_zoomer.h>
#include <qwt_wheel.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QMouseEvent>

#if QT_VERSION >= 0x040000
#include "qwt_plot_marker.h"
#include "qwt_symbol.h"
#endif

#include "us_util.h"

//standard C and C++ defs:

#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "us_hydrodyn_saxs.h"
#include "us_hydrodyn_saxs_buffer_conc.h"

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Buffer : public QFrame {
      Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn_Saxs;
      friend class US_Hydrodyn_Saxs_Buffer_Conc;
      friend class US_Hydrodyn_Saxs_Buffer_Nth;

   public:
      US_Hydrodyn_Saxs_Buffer(csv csv1, void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Saxs_Buffer();

      void add_plot(QString name, vector<double> q, vector<double> I);

      void add_plot(QString name, vector<double> q, vector<double> I, vector<double> errors);

   private:
      csv csv1;

      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title;

      QTableWidget *t_csv;

      QProgressBar *progress;

      QLabel *lbl_files;
      QCheckBox *cb_lock_dir;
      mQLabel *lbl_dir;
      QPushButton *pb_add_files;
      QPushButton *pb_similar_files;
      QPushButton *pb_conc;
      QPushButton *pb_clear_files;

      QPushButton *pb_regex_load;
      QLineEdit *le_regex;
      QLineEdit *le_regex_args;

      QPushButton *pb_select_all;
      QPushButton *pb_invert;
      // QPushButton   *pb_adjacent;
      QPushButton *pb_select_nth;
      QPushButton *pb_join;
      QPushButton *pb_to_saxs;
      QPushButton *pb_view;
      QPushButton *pb_rescale;

      QListWidget *lb_files;
      QLabel *lbl_selected;
      // QPushButton   *pb_plot_files;
      QPushButton *pb_avg;
      QPushButton *pb_asum;
      QPushButton *pb_normalize;
      QPushButton *pb_conc_avg;

      QPushButton *pb_set_buffer;
      QLabel *lbl_buffer;

      QPushButton *pb_set_empty;
      QLabel *lbl_empty;

      QPushButton *pb_set_signal;
      QLabel *lbl_signal;

      QLabel *lbl_np;
      QGroupBox *bg_np;
      QRadioButton *rb_np_crop;
      QRadioButton *rb_np_min;
      QRadioButton *rb_np_ignore;
      QRadioButton *rb_np_ask;

      QCheckBox *cb_multi_sub;
      QCheckBox *cb_multi_sub_avg;
      QCheckBox *cb_multi_sub_conc_avg;

      QLabel *lbl_created_files;
      mQLabel *lbl_created_dir;
      QListWidget *lb_created_files;
      QLabel *lbl_selected_created;

      QPushButton *pb_select_all_created;
      QPushButton *pb_adjacent_created;
      QPushButton *pb_save_created_csv;
      QPushButton *pb_save_created;
      QPushButton *pb_show_created;
      QPushButton *pb_show_only_created;

      QPushButton *pb_start;
      QPushButton *pb_run_current;
      QPushButton *pb_run_divide;
      QPushButton *pb_run_best;
      QPushButton *pb_stop;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      QLabel *lbl_wheel_pos;
      QwtWheel *qwtw_wheel;
      QPushButton *pb_wheel_cancel;
      QPushButton *pb_wheel_save;

      QPushButton *pb_join_start;
      QPushButton *pb_join_swap;
      QLabel *lbl_join_offset;
      mQLineEdit *le_join_offset;
      QLabel *lbl_join_mult;
      mQLineEdit *le_join_mult;
      QLabel *lbl_join_start;
      mQLineEdit *le_join_start;
      QLabel *lbl_join_point;
      mQLineEdit *le_join_point;
      QLabel *lbl_join_end;
      mQLineEdit *le_join_end;
      QPushButton *pb_join_fit_scaling;
      QPushButton *pb_join_fit_linear;
      QLabel *lbl_join_rmsd;

      mQwtPlot *plot_dist;
      US_Plot *usp_plot_dist;
   private slots:
      void usp_config_plot_dist(const QPoint &);

   private:
      ScrollZoomer *plot_dist_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid *grid_saxs;
      bool legend_vis;
#endif

      QPushButton *pb_color_rotate;

      QPushButton *pb_select_vis;
      QPushButton *pb_remove_vis;
      QPushButton *pb_crop_zero;
      QPushButton *pb_crop_vis;
      QPushButton *pb_crop_common;
      QPushButton *pb_crop_left;
      QPushButton *pb_crop_undo;
      QPushButton *pb_crop_right;
      QPushButton *pb_legend;
      QPushButton *pb_axis_x;
      QPushButton *pb_axis_y;

      QCheckBox *cb_guinier;
      QLabel *lbl_guinier;

      bool order_ascending;

      void editor_msg(QString color, QString msg);
      void editor_msg_qc(QColor qcolor, QString msg);

      bool running;

      bool validate();
      bool any_to_run();

      US_Hydrodyn_Saxs *saxs_window;
      bool *saxs_widget;
      bool activate_saxs_window();
      void run_one(bool do_plot = true);
      void run_one_divide();

      void do_replot_saxs();

      double best_fitness;

      QString saxs_header_iqq;

      QString errormsg;

      vector<QColor> plot_colors;

      map<QString, vector<QString>> f_qs_string;
      map<QString, vector<double>> f_qs;
      map<QString, vector<double>> f_Is;
      map<QString, vector<double>> f_errors;
      map<QString, unsigned int> f_pos;

      map<QString, QString> f_name;
      map<QString, double> f_psv;
      map<QString, double> f_I0se;
      map<QString, double> f_conc;
      map<QString, double> f_extc;

      map<QString, bool> created_files_not_saved;

      map<QString, double> current_concs(bool quiet = false);
      map<QString, double> window_concs();

      vector<crop_undo_data> crop_undos;

      bool is_nonzero_vector(vector<double> &v);
      bool is_zero_vector(vector<double> &v);

      void save_csv_saxs_iqq();

      csv current_csv();

      void recompute_interval_from_points();
      void recompute_points_from_interval();

      bool load_file(QString file);

      void plot_files();
      bool plot_file(QString file, double &minx, double &maxx, double &miny, double &maxy);

      bool get_min_max(QString file, double &minx, double &maxx, double &miny, double &maxy);

      bool disable_updates;

      QString qstring_common_head(QString s1, QString s2);
      QString qstring_common_tail(QString s1, QString s2);

      QString qstring_common_head(QStringList qsl, bool strip_digits = false);
      QString qstring_common_tail(QStringList qsl, bool strip_digits = false);

      QString last_load_dir;
      bool save_files(QStringList files);
      bool save_file(QString file, bool &overwrite, bool &cancel);
      bool save_files_csv(QStringList files);

      csv csv_conc;
      US_Hydrodyn_Saxs_Buffer_Conc *conc_window;
      bool conc_widget;
      void update_csv_conc();
      bool all_selected_have_nonzero_conc();

      void delete_zoomer_if_ranges_changed();
      QString vector_double_to_csv(vector<double> vd);

      bool adjacent_ok(QString name);

      void avg(QStringList files);
      void asum(QStringList files);
      void conc_avg(QStringList files);
      QString last_created_file;
      void zoom_info();
      void clear_files(QStringList files);
      void to_created(QString file);
      void add_files(QStringList files);
      bool axis_x_log;
      bool axis_y_log;

      void disable_all();

      QString wheel_file;
      QString join_file;
      double join_low_q;
      double join_high_q;
      double join_offset_start;
      double join_offset_end;
      double join_offset_delta;
      double join_mult_start;
      double join_mult_end;
      double join_mult_delta;

#if QT_VERSION >= 0x040000
      map<QString, QwtPlotCurve *> plotted_curves;
      vector<QwtPlotMarker *> plotted_markers;
      QwtPlotCurve *wheel_curve;
      QwtPlotCurve *join_curve;
#else
      map<QString, long> plotted_curves;
      vector<long> plotted_markers;
      long wheel_curve;
      long join_curve;
#endif

      mQLineEdit *le_last_focus;

      void join_enables();
      void join_add_marker(
         double pos, QColor color, QString text,
#if QT_VERSION < 0x040000
         int
#else
         Qt::Alignment
#endif
            align
         = Qt::AlignRight | Qt::AlignTop);
      void join_init_markers();
      void join_delete_markers();
      void replot_join();

      bool join_adjust_lowq;
      void join_set_wheel_range();
      void join_do_replot();

      void push_back_color_if_ok(QColor bg, QColor set);

      QString title;
      QStringList all_selected_files();

      bool adjacent_select(QListWidget *lb, QString match);

      void select_these(map<QString, QString> &parameters, bool reenable = true);

      double tot_intensity(QString &file, double q_min = 0e0, double q_max = 6e0);

      bool wheel_is_pressed;

   private slots:

      void setupGUI();

      void color_rotate();
      void dir_pressed();
      void created_dir_pressed();

      void update_enables();
      void update_files();
      void update_created_files();
      void add_files();
      void similar_files();
      void conc();
      void clear_files();
      void regex_load();

      void select_all();
      void invert();
      void join();
      // void adjacent();
      void select_nth();
      void to_saxs();
      void view();
      void rescale();
      void avg();
      void asum();
      void normalize();
      void conc_avg();
      void set_buffer();
      void set_empty();
      void set_signal();
      void select_all_created();
      void adjacent_created();
      void save_created_csv();
      void save_created();
      void show_created();
      void show_only_created();

      void table_value(int, int);

      void start();
      void run_current();
      void run_divide();
      void run_best();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

      void plot_zoomed(const QRectF &rect);
      void plot_mouse(const QMouseEvent &me);

      void adjust_wheel(double);
      void wheel_cancel();
      void wheel_save();

      void join_start();
      void join_swap();
      void join_fit_scaling();
      void join_fit_linear();

      void join_offset_text(const QString &);
      void join_mult_text(const QString &);
      void join_start_text(const QString &);
      void join_point_text(const QString &);
      void join_end_text(const QString &);

      void join_offset_focus(bool);
      void join_mult_focus(bool);
      void join_start_focus(bool);
      void join_point_focus(bool);
      void join_end_focus(bool);

      void select_vis();
      void remove_vis();
      void crop_left();
      void crop_common();
      void crop_vis();
      void crop_zero();
      void crop_undo();
      void crop_right();
      void legend();
      void axis_x();
      void axis_y();
      void legend_set();
      void guinier();

      void rename_created(QListWidgetItem *, const QPoint &);
      void rename_from_context(const QPoint &pos);

      void wheel_pressed();
      void wheel_released();

   protected slots:

      void closeEvent(QCloseEvent *);
};
#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

#endif
