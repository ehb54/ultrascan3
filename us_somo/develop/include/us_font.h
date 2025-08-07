#ifndef US_FONT_H
#define US_FONT_H

#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#include <qdialog.h>
#include <qfont.h>
#include <qfontdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
//Added by qt3to4:
#include <QCloseEvent>

class US_EXTERN US_Font : public QDialog {
      Q_OBJECT

   public:
      US_Font(QString *, int *, QWidget *p = 0, const char *name = 0);
      ~US_Font();

      QLabel *lbl_info;
      QLabel *lbl_sample;
      QLabel *lbl_family1;
      QLabel *lbl_family2;
      QLabel *lbl_point1;
      QLabel *lbl_point2;
      QLabel *lbl_font1;
      QLabel *lbl_font2;
      QLabel *lbl_font3;
      QLabel *lbl_font4;
      QLabel *lbl_font5;
      QLabel *lbl_font6;
      QPushButton *pb_font;
      QPushButton *pb_cancel;
      QPushButton *pb_ok;
      QPushButton *pb_help;

      US_Config *USglobal; /*!< A US_Config reference. */

      int *fontSize;
      QString *fontFamily;
      QFont currentFont;
      QFont oldFont;

   public slots:

      void cancel();
      void check();
      void help();
      void selectFont();

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
};

#endif
