#ifndef US_HYDRODYN_BATCH_MOVIE_OPTS_H
#define US_HYDRODYN_BATCH_MOVIE_OPTS_H

// QT defs:

#include <qcheckbox.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Batch_Movie_Opts : public QDialog {
  Q_OBJECT

 public:
  US_Hydrodyn_Batch_Movie_Opts(QString msg, QString *title, QString *dir,
                               QString somo_dir, QString *file, double *fps,
                               double *scale, bool *cancel_req, bool *clean_up,
                               bool *use_tc, QString *tc_unit, double *tc_start,
                               double *tc_delta, float *tc_pointsize,
                               bool *black_background, bool *do_pat,
                               QWidget *p = 0, const char *name = 0);
  ~US_Hydrodyn_Batch_Movie_Opts();

 private:
  QLabel *lbl_info;
  QLabel *lbl_title;
  QLabel *lbl_dir;
  QLabel *lbl_file;
  QLabel *lbl_fps;
  QLabel *lbl_scale;
  QLabel *lbl_tc_unit;
  QLabel *lbl_tc_start;
  QLabel *lbl_tc_delta;
  QLabel *lbl_tc_pointsize;

  QLineEdit *le_dir;
  QLineEdit *le_title;
  QLineEdit *le_file;
  QLineEdit *le_fps;
  QLineEdit *le_scale;
  QLineEdit *le_tc_unit;
  QLineEdit *le_tc_start;
  QLineEdit *le_tc_delta;
  QLineEdit *le_tc_pointsize;

  QPushButton *pb_cancel;
  QPushButton *pb_help;

  QCheckBox *cb_clean_up;
  QCheckBox *cb_use_tc;
  QCheckBox *cb_black_background;
  QCheckBox *cb_do_pat;

  void setupGUI();

  US_Config *USglobal;

  QString msg;

  QString *title;
  QString *dir;
  QString somo_dir;
  QString *file;
  double *fps;
  double *scale;
  bool *cancel_req;
  bool *clean_up;
  bool *use_tc;
  QString *tc_unit;
  double *tc_start;
  double *tc_delta;
  float *tc_pointsize;
  bool *black_background;
  bool *do_pat;

  QPalette label_font_ok;
  QPalette label_font_warning;

 public:
 public slots:

 private slots:

  void update_title(const QString &str);
  void update_dir(const QString &str);
  void update_file(const QString &str);
  void update_fps(const QString &str);
  void update_scale(const QString &str);
  void update_tc_unit(const QString &str);
  void update_tc_start(const QString &str);
  void update_tc_delta(const QString &str);
  void update_tc_pointsize(const QString &str);

  void update_dir_msg();

  void update_enables();

  void set_clean_up();
  void set_use_tc();
  void set_black_background();
  void set_do_pat();

  void cancel();
  void help();
};

#endif
