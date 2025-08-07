#ifndef US_CONFIG_GUI_H
#define US_CONFIG_GUI_H

// QT defs:

//#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#ifndef NO_DB
#include "us_database.h"
#endif

#include "us_color.h"
#include "us_font.h"
#include "us_write_config.h"

//#include <qwindowsstyle.h>
#include <qradiobutton.h>
// #include <qmotifstyle.h>
#include <qdir.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtranslator.h>
#include <qwidget.h>
//#include <q3frame.h>
//Added by qt3to4:
#include <QCloseEvent>

//extern int US_EXTERN global_Xpos;
//extern int US_EXTERN global_Ypos;

class US_EXTERN US_Config_GUI : public QFrame {
      Q_OBJECT

   public:
      US_Config_GUI(QWidget *parent = 0, const char *name = 0);
      ~US_Config_GUI();

      US_Config *USglobal;
      QLabel *lbl_directions;

   private:
      QPushButton *pb_browser;
      QPushButton *pb_root_dir;
      QPushButton *pb_tmp_dir;
      QPushButton *pb_data_dir;
      QPushButton *pb_archive_dir;
      QPushButton *pb_system_dir;
      QPushButton *pb_help_dir;
      QPushButton *pb_result_dir;
      QPushButton *pb_html_dir;
      QLabel *lbl_paths;
      QLabel *lbl_temperature_tol;
      QLabel *lbl_beckman_bug;
      QLabel *lbl_misc;
      QLabel *lbl_blank;
      QLabel *lbl_color;
      QLabel *lbl_font;
      QLabel *lbl_database;
      QLabel *lbl_numThreads;
      QLineEdit *le_browser;
      QLineEdit *le_data_dir;
      QLineEdit *le_archive_dir;
      QLineEdit *le_root_dir;
      QLineEdit *le_tmp_dir;
      QLineEdit *le_system_dir;
      QLineEdit *le_help_dir;
      QLineEdit *le_result_dir;
      QLineEdit *le_html_dir;
      QLineEdit *le_temperature_tol;
      QRadioButton *bt_on;
      QRadioButton *bt_off;
      QPushButton *pb_save;
      QPushButton *pb_help;
      QPushButton *pb_color;
      QPushButton *pb_font;
      QPushButton *pb_database;
      QPushButton *pb_cancel;
      QwtCounter *cnt_numThreads;

   public slots:
      void help();
      void cancel();
      void save();
      void update_screen();
      void open_browser_dir();
      void update_browser(const QString &);
      void open_tmp_dir();
      void update_tmp_dir(const QString &);
      void open_data_dir();
      void update_data_dir(const QString &);
      void open_archive_dir();
      void update_archive_dir(const QString &);
      void open_root_dir();
      void update_root_dir(const QString &);
      void open_result_dir();
      void update_result_dir(const QString &);
      void open_html_dir();
      void update_html_dir(const QString &);
      void update_temperature_tol(const QString &);
      void open_system_dir();
      void update_system_dir(const QString &);
      void open_help_dir();
      void update_help_dir(const QString &);
      void update_on_button();
      void update_off_button();
      void update_color();
      void update_margin(int);
      void update_font();
      void update_database();
      void update_numThreads(double);

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
};

#endif
