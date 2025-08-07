#ifndef US_HYDRODYN_SAXS_2D_H
#define US_HYDRODYN_SAXS_2D_H

// QT defs:

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
// #include <q3frame.h>
#include <qcheckbox.h>
#include <qfileinfo.h>
#include <qimage.h>
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

#include <complex>
#include <iostream>
#include <map>
#include <vector>

#include "qwt_wheel.h"
#include "us_hydrodyn_saxs.h"

using namespace std;

struct ush2d_data {
#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif
  vector<vector<complex<double> > > data;
#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

  QImage i_2d;

  double lambda;

  double beam_center_pixels_height;
  double beam_center_pixels_width;
  double beam_center_height;
  double beam_center_width;

  int detector_pixels_height;
  int detector_pixels_width;
  double detector_height;
  double detector_width;
  double detector_height_per_pixel;
  double detector_width_per_pixel;

  double atomic_scaling;

  double detector_distance;
};

class US_EXTERN US_Hydrodyn_Saxs_2d : public QFrame {
  Q_OBJECT

  friend class US_Hydrodyn_Saxs;

 public:
  US_Hydrodyn_Saxs_2d(void *us_hydrodyn, QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_Saxs_2d();

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
  QLineEdit *le_detector_height;
  QLineEdit *le_detector_width;

  QLabel *lbl_detector_pixels;
  QLineEdit *le_detector_pixels_height;
  QLineEdit *le_detector_pixels_width;

  QLabel *lbl_beam_center;
  QLineEdit *le_beam_center_pixels_height;
  QLineEdit *le_beam_center_pixels_width;

  QLabel *lbl_atomic_scaling;
  QLineEdit *le_atomic_scaling;

  QLabel *lbl_sample_rotations;
  QLineEdit *le_sample_rotations;

  QCheckBox *cb_save_pdbs;

  QProgressBar *progress;

  QPushButton *pb_integrate;

  QLabel *lbl_2d;
  QwtWheel *qwtw_wheel;
  QLabel *lbl_wheel_pos;

  QPushButton *pb_info;
  QPushButton *pb_start;
  QPushButton *pb_stop;

  QFont ft;
  QTextEdit *editor;
  QMenuBar *m;

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
  void reset_2d();
  bool update_image();

  saxs_options *our_saxs_options;

  int unit;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(disable : 4251)
#endif
#endif
  vector<vector<complex<double> > > data;

  vector<atom> atom_list;
  vector<hybridization> hybrid_list;
  vector<saxs> saxs_list;
  vector<residue> residue_list;
  vector<PDB_model> model_vector;
  vector<vector<PDB_atom> > bead_models;
  vector<unsigned int> selected_models;

  map<QString, saxs> saxs_map;
  map<QString, hybridization> hybrid_map;
  map<QString, atom> atom_map;
  map<QString, QString> residue_atom_hybrid_map;

  vector<ush2d_data> data_stack;

#ifdef WIN32
#if QT_VERSION < 0x040000
#pragma warning(default : 4251)
#endif
#endif

  void compute_variables();
  void report_variables();
  double q_of_pixel(int pixels_height, int pixels_width);
  double q_of_pixel(double height, double width);

  QImage i_2d;

  double lambda;

  double beam_center_pixels_height;
  double beam_center_pixels_width;
  double beam_center_height;
  double beam_center_width;

  int detector_pixels_height;
  int detector_pixels_width;
  double detector_height;
  double detector_width;
  double detector_height_per_pixel;
  double detector_width_per_pixel;

  double atomic_scaling;

  double detector_distance;

  void push();
  void set_pos(unsigned int i);

  unsigned int last_wheel_pos;

 private slots:

  void setupGUI();

  void update_lambda(const QString &);
  void update_detector_distance(const QString &);
  void update_detector_height(const QString &);
  void update_detector_width(const QString &);
  void update_detector_pixels_height(const QString &);
  void update_detector_pixels_width(const QString &);
  void update_beam_center_pixels_height(const QString &);
  void update_beam_center_pixels_width(const QString &);
  void update_sample_rotations(const QString &);

  void adjust_wheel(double);

  void info();

  void start();
  void stop();

  void integrate();

  void clear_display();
  void update_font();
  void save();

  void cancel();
  void help();

 protected slots:

  void closeEvent(QCloseEvent *);
};

namespace bulatov {
vector<vector<double> > bulatov_main(int N, int Nstep);
};

#endif
