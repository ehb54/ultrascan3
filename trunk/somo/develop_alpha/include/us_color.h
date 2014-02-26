#ifndef US_COLOR_H
#define US_COLOR_H

//#include "us.h"
//#include "us_util.h"
#include   "us_write_config.h"
#include "us_extern.h"

#include <qlabel.h>
#include <q3listbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qstring.h>
#include <qlcdnumber.h>
#include <q3progressbar.h>
#include <Q3ComboBox>
#include <qevent.h>
#include <q3frame.h>
#include <qtranslator.h>
#include <qpen.h>

#include <qwt_plot.h>
#include <qwt_counter.h>
//Added by qt3to4:
#include <QCloseEvent>
#ifdef QT4
# include "qwt_plot_grid.h"
# include "qwt_plot_curve.h"
#endif

/*
#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <qwidget.h>
*/

class US_EXTERN US_Color : public Q3Frame
{
   Q_OBJECT
   
   public:
      US_Color(QWidget *p=0, const char *name = 0);
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
      Q3ListBox *lb_scheme;
      Q3ListBox *lb_item;
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
      Q3ComboBox *cmbb_margin;
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
      QColorGroup temp_cg1;
      QColorGroup temp_cg2;
      QColorGroup temp_cg3;
      QColorGroup temp_cg4;
      QColorGroup temp_cg5;
      QColorGroup temp_cg6;
      QwtPlot *plot;
      QwtCounter *cnt;
#ifdef QT4
      QwtPlotGrid  *grid;
      QwtPlotCurve *curve;
#endif
      Q3ProgressBar *progress;
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
