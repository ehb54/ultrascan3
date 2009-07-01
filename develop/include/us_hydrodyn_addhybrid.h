#ifndef US_HYDRODYN_ADDHYBRID_H
#define US_HYDRODYN_ADDHYBRID_H

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

class US_AddHybridization : public QWidget
{
   Q_OBJECT

   public:
      US_AddHybridization(bool *, QWidget *p=0, const char *name=0);
      ~US_AddHybridization();

   private:

      US_Config *USglobal;
      bool *widget_flag;

      QPushButton *pb_add;
      QPushButton *pb_close;
      QPushButton *pb_select_file;
      QPushButton *pb_select_saxs_file;

      QLabel *lbl_info;
      QLabel *lbl_table;
      QLabel *lbl_table_saxs;
      QLabel *lbl_mw;
      QLabel *lbl_radius;
      QLabel *lbl_name;
      QLabel *lbl_number_of_hybrids;
      QLabel *lbl_number_of_saxs;

      QComboBox *cmb_hybrid;
      QComboBox *cmb_saxs;

      QLineEdit *le_name;
      QLineEdit *le_hybrid;
      QLineEdit *le_mw;
      QLineEdit *le_radius;
      struct hybridization current_hybrid;
      vector <struct hybridization> hybrid_list;
      struct saxs current_saxs;
      vector <struct saxs> saxs_list;
      QString hybrid_filename;
      QString saxs_filename;

   private slots:
      void add();
      void select_file();
      void select_saxs_file();
      void setupGUI();
      void update_name(const QString &);
      void update_mw(const QString &);
      void update_radius(const QString &);
      void select_hybrid(int);
      void select_saxs(int);

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif
