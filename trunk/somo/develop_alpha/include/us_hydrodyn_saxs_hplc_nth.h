#ifndef US_HYDRODYN_SAXS_HPLC_NTH_H
#define US_HYDRODYN_SAXS_HPLC_NTH_H

#include "us_hydrodyn_saxs_hplc.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Hplc_Nth : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Hplc_Nth(
                                void                     *              us_hydrodyn_saxs_hplc,
                                map < QString, QString > *              parameters,
                                QWidget *                               p = 0,
                                const char *                            name = 0
                                );

      ~US_Hydrodyn_Saxs_Hplc_Nth();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;


      QLabel *                                lbl_files;
      Q3ListBox *                              lb_files;

      QLabel *                                lbl_files_sel;
      Q3ListBox *                              lb_files_sel;
      QLabel *                                lbl_files_selected;

      QLabel *                                lbl_select_nth;

      QLabel *                                lbl_n;
      QLineEdit *                             le_n;

      QLabel *                                lbl_start;
      QLineEdit *                             le_start;
      QLabel *                                lbl_start_name;

      QLabel *                                lbl_end;
      QLineEdit *                             le_end;
      QLabel *                                lbl_end_name;

      QPushButton *                           pb_nth_only;
      QPushButton *                           pb_nth_add;

      QLabel *                                lbl_contain;

   // QLabel *                                lbl_starts_with;
   // QLineEdit *                             le_starts_with;

      QLabel *                                lbl_contains;
      QLineEdit *                             le_contains;

   // QLabel *                                lbl_ends_with;
   // QLineEdit *                             le_ends_with;

      QPushButton *                           pb_contains_only;
      QPushButton *                           pb_contains_add;

      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_go;

      void                     *              us_hydrodyn_saxs_hplc;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

   private slots:

      void                                    nth_only();
      void                                    nth_add();

      void                                    contains_only();
      void                                    contains_add();

      void                                    update_files_selected();

      void                                    go();
      void                                    quit();
      void                                    help();

      void                                    update_enables();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
