#ifndef US_HYDRODYN_ADDHYSAXS_H
#define US_HYDRODYN_ADDHYSAXS_H

#include <qwidget.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <QComboBox>
#include <qpushbutton.h>
//#include <q3frame.h>
//Added by qt3to4:
#include <QCloseEvent>

#include "us.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_extern.h"
#include "us_util.h"

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
      QPushButton *pb_help;
      QPushButton *pb_close;
      QPushButton *pb_select_file;

      QLabel *lbl_info;
      QLabel *lbl_table;
      QLabel *lbl_4term;
      QLabel *lbl_5term;
      QLabel *lbl_a1;
      QLabel *lbl_a2;
      QLabel *lbl_a3;
      QLabel *lbl_a4;
      QLabel *lbl_a5;
      QLabel *lbl_b1;
      QLabel *lbl_b2;
      QLabel *lbl_b3;
      QLabel *lbl_b4;
      QLabel *lbl_b5;
      QLabel *lbl_c;
      QLabel *lbl_volume;
      QLabel *lbl_saxs_name;
      QLabel *lbl_number_of_saxs;

      QComboBox *cmb_saxs;

      QLineEdit *le_saxs_name;
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
      QLineEdit *le_5a1;
      QLineEdit *le_5a2;
      QLineEdit *le_5a3;
      QLineEdit *le_5a4;
      QLineEdit *le_5a5;
      QLineEdit *le_5b1;
      QLineEdit *le_5b2;
      QLineEdit *le_5b3;
      QLineEdit *le_5b4;
      QLineEdit *le_5b5;
      QLineEdit *le_5c;
      QLineEdit *le_5volume;
      struct saxs current_saxs;
      vector <struct saxs> saxs_list;
      QString saxs_filename;

      void info_saxs( const QString & msg, const struct saxs & );

   private slots:
      void add();
      void help();
      void select_file();
      void setupGUI();
      void update_saxs_name(const QString &);
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
      void update_5a1(const QString &);
      void update_5a2(const QString &);
      void update_5a3(const QString &);
      void update_5a4(const QString &);
      void update_5a5(const QString &);
      void update_5b1(const QString &);
      void update_5b2(const QString &);
      void update_5b3(const QString &);
      void update_5b4(const QString &);
      void update_5b5(const QString &);
      void update_5c(const QString &);
      void update_5volume(const QString &);
      void select_saxs(int);

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif
