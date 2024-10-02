#ifndef US_HYDRODYN_ADDHYBRID_H
#define US_HYDRODYN_ADDHYBRID_H


#include <memory>
#include <QtCore/qbytearray.h>
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
      QPushButton *pb_help;
      QPushButton *pb_select_file;
      QPushButton *pb_select_saxs_file;

      QLabel *lbl_info;
      QLabel *lbl_table;
      QLabel *lbl_table_saxs;
      QLabel *lbl_mw;
      QLabel *lbl_radius;
      QLabel *lbl_scat_len;
      QLabel *lbl_exch_prot;
      QLabel *lbl_num_elect;
      QLabel *lbl_name;
      QLabel *lbl_number_of_hybrids;
      QLabel *lbl_number_of_saxs;

      QComboBox *cmb_hybrid;
      QComboBox *cmb_saxs;

      QLineEdit *le_name;
      QLineEdit *le_hybrid;
      QLineEdit *le_mw;
      QLineEdit *le_radius;
      QLineEdit *le_scat_len;
      QLineEdit *le_exch_prot;
      QLineEdit *le_num_elect;

      struct hybridization current_hybrid;
      vector <struct hybridization> hybrid_list;
      struct saxs current_saxs;
      vector <struct saxs> saxs_list;
      QString hybrid_filename;
      QString saxs_filename;

   private slots:
      void add();
      void help();
      void select_file();
      void select_saxs_file();
      void setupGUI();
      void update_name(const QString &);
      void update_mw(const QString &);
      void update_radius(const QString &);
      void update_scat_len(const QString &);
      void update_exch_prot(const QString &);
      void update_num_elect(const QString &);
      void select_hybrid(int);
      void select_saxs(int);

   protected slots:
      void closeEvent(QCloseEvent *);
};

#endif
