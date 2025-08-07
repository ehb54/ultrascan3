#ifndef US_COLOR_H
#define US_COLOR_H

// #include "us.h"
// #include "us_util.h"
#include <qcolor.h>
#include <qcolordialog.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qstring.h>

#include <QComboBox>

#include "us_extern.h"
#include "us_write_config.h"
// #include <q3frame.h>
#include <qpen.h>
#include <qtranslator.h>
#include <qwt_counter.h>
#include <qwt_plot.h>
// Added by qt3to4:
#include <QCloseEvent>
#if QT_VERSION >= 0x040000
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#endif

/*
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qwidget.h>
*/

class US_EXTERN US_Color : public QFrame {
  Q_OBJECT

 public:
  US_Color(QWidget *p = 0, const char *name = 0);
  ~US_Color();
  QLabel *lbl_background;
  QLabel *lbl_example;
  QLabel *lbl_margin;
  QLabel *lbl_banner;
  QLabel *lbl_progress;
  QLabel *lbl_counter;
  QLabel *lbl_lcd;
  QLabel *lbl_select;
  QLabel *lbl_text;
  QLabel *lbl_choice;
  QLabel *lbl_choices;
  QLabel *lbl_select_scheme;
  QLabel *lbl_select_element;
  QLabel *lbl_edit;
  QLabel *lbl_color1;
  QLabel *lbl_color2;
  QLabel *lbl_color3;
  QLabel *lbl_color4;
  QLabel *lbl_color5;
  QLabel *lbl_color6;
  QLabel *lbl_assign;
  QLabel *color_field1;
  QLabel *color_field2;
  QLabel *color_field3;
  QLabel *color_field4;
  QLabel *color_field5;
  QLabel *color_field6;
  QListWidget *lb_scheme;
  QListWidget *lb_item;
  QLineEdit *le_save_as;
  QPushButton *pb_quit;
  QPushButton *pb_delete;
  QPushButton *pb_save_as;
  QPushButton *pb_help;
  QPushButton *pb_reset;
  QPushButton *pb_apply;
  QPushButton *pb_normal;
  QPushButton *pb_active;
  QPushButton *pb_disabled;
  QPushButton *pb_color1;
  QPushButton *pb_color2;
  QPushButton *pb_color3;
  QPushButton *pb_color4;
  QPushButton *pb_color5;
  QPushButton *pb_color6;
  QComboBox *cmbb_margin;
  QColor color1;
  QColor color2;
  QColor color3;
  QColor color4;
  QColor color5;
  QColor color6;
  QColor temp_color1;
  QColor temp_color2;
  QColor temp_color3;
  QColor temp_color4;
  QColor temp_color5;
  QColor temp_color6;
  QPalette temp_cg1;
  QPalette temp_cg2;
  QPalette temp_cg3;
  QPalette temp_cg4;
  QPalette temp_cg5;
  QPalette temp_cg6;
  QwtPlot *plot;
  QwtCounter *cnt;
#if QT_VERSION >= 0x040000
  QwtPlotGrid *grid;
  QwtPlotCurve *curve;
#endif
  QProgressBar *progress;
  QLCDNumber *lcd;
  US_Config *USglobal;
  US_Write_Config *WConfig;
  QString save_str;

 private:
  int current_widget;
  int current_scheme;
  int current_index;
  int temp_margin;
  struct us_colors temp_colors;

 public slots:
  void setup_GUI();
  void help();
  void set_default();
  void sel_margin(int);
  void update_widgets(double);
  void update_save_str(const QString &);
  void apply();
  void reset();
  void quit();
  void save_as();
  void selected_scheme(int);
  void selected_item(int);
  void pick_color1();
  void pick_color2();
  void pick_color3();
  void pick_color4();
  void pick_color5();
  void pick_color6();
  void delete_scheme();
  void closeEvent(QCloseEvent *);

 signals:
  void marginChanged(int);
};

#endif
