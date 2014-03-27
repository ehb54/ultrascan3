#ifndef US_HYDRODYN_SAXS_BUFFER_NTH_H
#define US_HYDRODYN_SAXS_BUFFER_NTH_H

#include "us_hydrodyn_saxs_buffer.h"
#include "qlabel.h"
#include "qstring.h"
#include "qlayout.h"
#include "qvalidator.h"
//Added by qt3to4:
#include <QCloseEvent>
#include "us_mqt.h"

using namespace std;

class US_EXTERN US_Hydrodyn_Saxs_Buffer_Nth : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Saxs_Buffer_Nth(
                                void                     *              us_hydrodyn_saxs_buffer,
                                map < QString, QString > *              parameters,
                                QWidget *                               p = 0,
                                const char *                            name = 0
                                );

      ~US_Hydrodyn_Saxs_Buffer_Nth();

   private:

      US_Config *                             USglobal;

      QLabel *                                lbl_title;


      QLabel *                                lbl_files;
      Q3ListBox *                              lb_files;

      QLabel *                                lbl_files_sel;
      Q3ListBox *                              lb_files_sel;
      QLabel *                                lbl_files_selected;

      // select

      mQLabel *                               lbl_select_nth;
      vector < QWidget * >                    select_widgets;

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

      // contain

      mQLabel *                               lbl_contain;
      vector < QWidget * >                    contain_widgets;

   // QLabel *                                lbl_starts_with;
   // QLineEdit *                             le_starts_with;

      QLabel *                                lbl_contains;
      QLineEdit *                             le_contains;

   // QLabel *                                lbl_ends_with;
   // QLineEdit *                             le_ends_with;

      QPushButton *                           pb_contains_only;
      QPushButton *                           pb_contains_add;

      // intensity

      mQLabel *                               lbl_intensity;
      vector < QWidget * >                    intensity_widgets;

      QCheckBox *                             cb_q_range;
      QLineEdit *                             le_q_start;
      QLineEdit *                             le_q_end;

      QPushButton *                           pb_i_avg_all;
      QPushButton *                           pb_i_avg_sel;
      
      QCheckBox *                             cb_i_above;
      QLineEdit *                             le_i_level;

      Q3TextEdit *                             te_q;

      QPushButton *                           pb_i_only;
      QPushButton *                           pb_i_add;


      QPushButton *                           pb_help;
      QPushButton *                           pb_quit;
      QPushButton *                           pb_do_select;
      QPushButton *                           pb_go;

      void                     *              us_hydrodyn_saxs_buffer;
      map < QString, QString > *              parameters;

      void                                    setupGUI();

   private slots:

      // select
      void                                    hide_select();
      void                                    nth_only();
      void                                    nth_add();

      // contain
      void                                    hide_contain();
      void                                    contains_only();
      void                                    contains_add();

      // intensity 
      void                                    hide_intensity();
      void                                    i_avg_all();
      void                                    i_avg_sel();
      void                                    i_only();
      void                                    i_add();

      void                                    update_files_selected();

      void                                    do_select( bool update = true );
      void                                    go();
      void                                    quit();
      void                                    help();

      void                                    update_enables();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#endif
