#ifndef US_HYDRODYN_SAXS_BUFFER_NTH_H
#define US_HYDRODYN_SAXS_BUFFER_NTH_H

#include "qlabel.h"
#include "qlayout.h"
#include "qstring.h"
#include "qvalidator.h"
#include "us_hydrodyn.h"
#include "us_hydrodyn_saxs_buffer.h"
//Added by qt3to4:
#include <QCloseEvent>
#include "us_mqt.h"

using namespace std;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif

class US_EXTERN US_Hydrodyn_Saxs_Buffer_Nth : public QDialog {
      Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Buffer_Nth(
         void *us_hydrodyn_saxs_buffer, map<QString, QString> *parameters, QWidget *p = 0, const char *name = 0);

      ~US_Hydrodyn_Saxs_Buffer_Nth();

   private:
      US_Config *USglobal;

      mQLabel *lbl_title;
      vector<QWidget *> files_widgets;

      QLabel *lbl_files;
      QListWidget *lb_files;

      QLabel *lbl_files_sel;
      QListWidget *lb_files_sel;
      QLabel *lbl_files_selected;

      // select

      mQLabel *lbl_select_nth;
      vector<QWidget *> select_widgets;

      QLabel *lbl_n;
      QLineEdit *le_n;

      QLabel *lbl_start;
      QLineEdit *le_start;
      QLabel *lbl_start_name;

      QLabel *lbl_end;
      QLineEdit *le_end;
      QLabel *lbl_end_name;

      QPushButton *pb_nth_only;
      QPushButton *pb_nth_add;

      // contain

      mQLabel *lbl_contain;
      vector<QWidget *> contain_widgets;

      // QLabel *                                lbl_starts_with;
      // QLineEdit *                             le_starts_with;

      QLabel *lbl_contains;
      QLineEdit *le_contains;

      // QLabel *                                lbl_ends_with;
      // QLineEdit *                             le_ends_with;

      QPushButton *pb_contains_only;
      QPushButton *pb_contains_add;

      // ------ plot section

      PC *pc;
      mQwtPlot *plot_data;
      US_Plot *usp_plot_data;
   private slots:
      void usp_config_plot_data(const QPoint &);

   private:
      ScrollZoomer *plot_data_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid *grid_data;
#endif
#if QT_VERSION >= 0x040000
      vector<QwtPlotCurve *> plotted_curves;
      QwtPlotMarker *plot_marker;
#else
      vector<long> plotted_curves;
      long *plot_marker;
#endif
      vector<QString> plotted_names;
      vector<vector<double>> plotted_x;
      vector<vector<double>> plotted_y;

      QPushButton *pb_color_rotate;
      QPushButton *pb_clear_plot;
      QPushButton *pb_save_dat;

      // intensity

      mQLabel *lbl_intensity;
      vector<QWidget *> intensity_widgets;

      QCheckBox *cb_q_range;
      QLineEdit *le_q_start;
      QLineEdit *le_q_end;

      QPushButton *pb_i_avg_all;
      QPushButton *pb_i_avg_sel;

      QGroupBox *bg_i_above_below;
      QRadioButton *rb_i_above;
      QRadioButton *rb_i_below;
      QLineEdit *le_i_level;

      QTextEdit *te_q;

      QPushButton *pb_i_only;
      QPushButton *pb_i_add;

      QwtWheel *qwtw_wheel;

      QPushButton *pb_help;
      QPushButton *pb_quit;
      QPushButton *pb_do_select;
      QPushButton *pb_go;

      void *us_hydrodyn_saxs_buffer;
      void *us_hydrodyn;
      map<QString, QString> *parameters;

      void setupGUI();

      void i_avg(QStringList files);

      set<int> get_intensity_selected();

      bool wheel_is_pressed;

   private slots:

      // files
      void hide_files();

      // select
      void hide_select();
      void nth_only();
      void nth_add();

      // contain
      void hide_contain();
      void contains_only();
      void contains_add();

      // intensity
      void hide_intensity();
      void i_avg_all();
      void i_avg_sel();
      void update_i_level();
      void adjust_wheel(double);
      void i_only();
      void i_add();

      void color_rotate();
      void clear_plot();
      void save_dat();

      void update_files_selected();

      void do_select(bool update = true);
      void go();
      void quit();
      void help();

      void update_enables();

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
