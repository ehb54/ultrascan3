#ifndef US_HYDRODYN_SAXS_1D_H
#define US_HYDRODYN_SAXS_1D_H

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
#include <qtablewidget.h>
#include <qtextedit.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us_util.h"

//standard C and C++ defs:

#include <complex>
#include <iostream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "us_hydrodyn_saxs.h"

#include <qwt_plot.h>
#include "us3i_plot.h"
#if QT_VERSION >= 0x040000
#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#endif

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_1d : public QFrame {
      Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Saxs_1d(void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
      ~US_Hydrodyn_Saxs_1d();

   private:
      void *us_hydrodyn;

      US_Config *USglobal;

      QLabel *lbl_title;

      QLabel *lbl_atom_file;
      QLineEdit *le_atom_file;

      QLabel *lbl_lambda;
      QLineEdit *le_lambda;

      QLabel *lbl_detector_distance;
      QLineEdit *le_detector_distance;

      QLabel *lbl_detector_geometry;
      QLineEdit *le_detector_width;

      QLabel *lbl_detector_pixels;
      QLineEdit *le_detector_pixels_width;

      QLabel *lbl_rho0;
      QLineEdit *le_rho0;

      QLabel *lbl_deltaR;
      QLineEdit *le_deltaR;

      QLabel *lbl_probe_radius;
      QLineEdit *le_probe_radius;

      QPushButton *pb_find_target_ev_thresh;
      QLineEdit *le_threshold;

      QPushButton *pb_set_target_ev;
      QLineEdit *le_target_ev;

      QLabel *lbl_sample_rotations;
      QLineEdit *le_sample_rotations;

      QLabel *lbl_axis_rotations;
      QLineEdit *le_axis_rotations;

      QLabel *lbl_spec_multiplier;
      QLineEdit *le_spec_multiplier;

      QCheckBox *cb_save_pdbs;
      QCheckBox *cb_memory_conserve;
      QCheckBox *cb_planar_method;
      QCheckBox *cb_random_rotations;
      QCheckBox *cb_ev_from_file;
      QCheckBox *cb_only_ev;
      QCheckBox *cb_vvv;

      QProgressBar *progress;

      QLabel *lbl_1d;

      QPushButton *pb_info;
      QPushButton *pb_start;
      QPushButton *pb_stop;

      QFont ft;
      QTextEdit *editor;
      QMenuBar *m;

      mQwtPlot *plot_saxs;
      US_Plot *usp_plot_saxs;
   private slots:
      void usp_config_plot_saxs(const QPoint &);

   private:
      ScrollZoomer *plot_saxs_zoomer;
#if QT_VERSION >= 0x040000
      QwtPlotGrid *grid_pr;
      QwtPlotGrid *grid_saxs;
#endif
      QPushButton *pb_save_data;
      QPushButton *pb_to_somo;

      QPushButton *pb_help;
      QPushButton *pb_cancel;

      void editor_msg(QString color, QString msg);

      bool running;
      void update_enables();

      US_Hydrodyn_Saxs *saxs_window;
      bool *saxs_widget;
      bool activate_saxs_window();
      void run_one();

      bool validate(bool quiet = false);
      void reset_1d();
      bool update_image();

      saxs_options *our_saxs_options;

      int unit;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif
      vector<complex<double>> data;

      vector<atom> atom_list;
      vector<hybridization> hybrid_list;
      vector<saxs> saxs_list;
      vector<residue> residue_list;
      vector<PDB_model> model_vector;
      vector<vector<PDB_atom>> bead_models;
      vector<unsigned int> selected_models;

      map<QString, saxs> saxs_map;
      map<QString, hybridization> hybrid_map;
      map<QString, atom> atom_map;
      map<QString, QString> residue_atom_hybrid_map;

      vector<QColor> plot_colors;
      vector<double> total_modulii;

      vector<point> excluded_volume;

      bool save_rotations(vector<vector<double>> &rotations);
      bool load_rotations(int number, vector<vector<double>> &rotations);
#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

      void compute_variables();
      void report_variables();
      double q_of_pixel(int pixels_width);
      double q_of_pixel(double width);

      double lambda;

      int detector_pixels_width;
      double detector_width;
      double detector_width_per_pixel;

      double rho0;
      double deltaR;

      double detector_distance;

      double spec_multiplier;

      unsigned int plot_count;

      QString errormsg;
      QString filepathname;
      QString mapname;
      bool setup_excluded_volume_map();
      bool get_excluded_volume_map();
      bool save_copy_excluded_volume_map(QString name);

      double probe_radius;
      double threshold;

      QString ev_file_name;

   private slots:

      void setupGUI();

      void update_lambda(const QString &);
      void update_detector_distance(const QString &);
      void update_detector_width(const QString &);
      void update_detector_pixels_width(const QString &);
      void update_sample_rotations(const QString &);
      void update_axis_rotations(const QString &);
      void update_rho0(const QString &);
      void update_deltaR(const QString &);
      void update_probe_radius(const QString &);
      void update_threshold(const QString &);
      void update_target_ev(const QString &);
      void update_spec_multiplier(const QString &);

      // cb's

      void set_random_rotations();
      void set_planar_method();

      // pb's

      bool find_target_ev_thresh();
      void set_target_ev();

      void info();

      void start();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void save_data();
      void to_somo();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
};

#endif
