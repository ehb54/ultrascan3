#ifndef US_HYDRODYN_ADDHYSAXS_H
#define US_HYDRODYN_ADDHYSAXS_H

#include "us_hydrodyn_pdbdefs.h"
#include "us.h"
#include "us_extern.h"
#include "us_util.h"

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qframe.h>

class US_AddSaxs : public QWidget
{
   Q_OBJECT

   public:
      US_AddSaxs(bool *, QWidget *p=0, const char *name=0);
      ~US_AddSaxs();

   private:

      US_Config *USglobal;
      bool *widget_flag;

      QPushButton *pb_add;
      QPushButton *pb_close;
      QPushButton *pb_select_file;

      QLabel *lbl_info;
      QLabel *lbl_table;
      QLabel *lbl_a1;
      QLabel *lbl_a2;
      QLabel *lbl_a3;
      QLabel *lbl_a4;
      QLabel *lbl_b1;
      QLabel *lbl_b2;
      QLabel *lbl_b3;
      QLabel *lbl_b4;
      QLabel *lbl_c;
      QLabel *lbl_volume;
      QLabel *lbl_name;
      QLabel *lbl_number_of_saxs;

      QComboBox *cmb_saxs;

      QLineEdit *le_name;
      QLineEdit *le_a1;
      QLineEdit *le_a2;
      QLineEdit *le_a3;
      QLineEdit *le_a4;
      QLineEdit *le_b1;
      QLineEdit *le_b2;
      QLineEdit *le_b3;
      QLineEdit *le_b4;
      QLineEdit *le_c;
      QLineEdit *le_volume;
      struct saxs current_saxs;
      vector <struct saxs> saxs_list;
      QString saxs_filename;

   private slots:
      void add();
      void select_file();
      void setupGUI();
      void update_name(const QString &);
      void update_a1(const QString &);
      void update_a2(const QString &);
      void update_a3(const QString &);
      void update_a4(const QString &);
      void update_b1(const QString &);
      void update_b2(const QString &);
      void update_b3(const QString &);
      void update_b4(const QString &);
      void update_c(const QString &);
      void update_volume(const QString &);
      void select_saxs(int);

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif
