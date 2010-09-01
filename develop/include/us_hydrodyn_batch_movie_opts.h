#ifndef US_HYDRODYN_BATCH_MOVIE_OPTS_H
#define US_HYDRODYN_BATCH_MOVIE_OPTS_H

// QT defs:

#include <qlabel.h>
#include <qpushbutton.h>
#include <qdialog.h>
#include <qfileinfo.h>
#include <qcheckbox.h>

#include "us_util.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Batch_Movie_Opts : public QDialog
{
   Q_OBJECT

   public:

      US_Hydrodyn_Batch_Movie_Opts(
                                   QString    msg,
                                   QString    *dir,
                                   QString    somo_dir,
                                   QString    *file,
                                   float      *fps,
                                   float      *scale,
                                   bool       *cancel_req,
                                   bool       *clean_up,
                                   bool       *use_tc,
                                   QString    *tc_unit,
                                   float      *tc_start,
                                   float      *tc_delta,
                                   QWidget    *p = 0, 
                                   const char *name = 0
                                   );
      ~US_Hydrodyn_Batch_Movie_Opts();

   private:

      QLabel *lbl_info;
      QLabel *lbl_dir;
      QLabel *lbl_file;
      QLabel *lbl_fps;
      QLabel *lbl_scale;
      QLabel *lbl_tc_unit;
      QLabel *lbl_tc_start;
      QLabel *lbl_tc_delta;

      QLineEdit *le_dir;
      QLineEdit *le_file;
      QLineEdit *le_fps;
      QLineEdit *le_scale;
      QLineEdit *le_tc_unit;
      QLineEdit *le_tc_start;
      QLineEdit *le_tc_delta;

      QPushButton *pb_cancel;
      QPushButton *pb_help;

      QCheckBox *cb_clean_up;
      QCheckBox *cb_use_tc;

      void setupGUI();

      US_Config *USglobal;
      
      QString msg;
      
      QString    *dir;
      QString    somo_dir;
      QString    *file;
      float      *fps;
      float      *scale;
      bool       *cancel_req;
      bool       *clean_up;
      bool       *use_tc;
      QString    *tc_unit;
      float      *tc_start;
      float      *tc_delta;

      QPalette label_font_ok;
      QPalette label_font_warning;

   public:

   public slots:

   private slots:

      void update_dir(const QString &str);
      void update_file(const QString &str);
      void update_fps(const QString &str);
      void update_scale(const QString &str);
      void update_tc_unit(const QString &str);
      void update_tc_start(const QString &str);
      void update_tc_delta(const QString &str);

      void update_dir_msg();

      void update_enables();

      void set_clean_up();
      void set_use_tc();

      void cancel();
      void help();
};

#endif
