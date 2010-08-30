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
                                   bool       *cancel_req,
                                   bool       *clean_up,
                                   QWidget    *p = 0, 
                                   const char *name = 0
                                   );
      ~US_Hydrodyn_Batch_Movie_Opts();

   private:

      QLabel *lbl_info;
      QLabel *lbl_dir;
      QLabel *lbl_file;
      QLabel *lbl_fps;

      QLineEdit *le_dir;
      QLineEdit *le_file;
      QLineEdit *le_fps;

      QPushButton *pb_cancel;
      QPushButton *pb_help;

      QCheckBox *cb_clean_up;

      void setupGUI();

      US_Config *USglobal;
      
      QString msg;
      
      QString    *dir;
      QString    somo_dir;
      QString    *file;
      float      *fps;
      bool       *cancel_req;
      bool       *clean_up;

      QPalette label_font_ok;
      QPalette label_font_warning;

   public:

   public slots:

   private slots:

      void update_dir(const QString &str);
      void update_file(const QString &str);
      void update_fps(const QString &str);

      void update_dir_msg();

      void set_clean_up();

      void cancel();
      void help();
};

#endif
