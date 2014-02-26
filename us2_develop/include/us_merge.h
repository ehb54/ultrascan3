#ifndef US_MERGE_H
#define US_MERGE_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qapp.h>
#include <iostream>
#include <qmsgbox.h>
#include <fstream>
#include <stdlib.h>
#include <locale.h>
#include <qtstream.h>
#include <qlistbox.h>
#include <qprogressbar.h>
#include <string.h>
#include <qfile.h>
#include <qfiledlg.h>
#include <qradiobutton.h> 
#include <qstrlist.h> 
#include "us_util.h"


extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_Merge : public QFrame
{
   Q_OBJECT
   
   public:
      US_Merge( QWidget *p=0, const char *name=0 );
      ~US_Merge();

   private:
      int selected_cell1, selected_cell2;
      QString directory1, directory2, extension1, extension2;
      QDir data_dir1, data_dir2;
      bool ready, out_of_sequence1[8], out_of_sequence2[8], move_data;
      int scan_count1[8], scan_count2[8];
      int data_type; //1 = absorbance, 2 = interference
      QRadioButton *bt_if, *bt_abs, *bt_copy, *bt_move, *bt_wl;
      QLabel *absorbance;
      QLabel *interference;
      QLabel *lbl_header;
      QLabel *lbl_blank;
      QLabel *lbl_directory1;
      QLabel *lbl_directory2;
      QLabel *lbl_dir1_cell_details;
      QLabel *lbl_dir1_cell_contents;
      QLabel *lbl_dir2_cell_details;
      QLabel *lbl_dir2_cell_contents;
      QLabel *lbl_progress;
      QLabel *lbl_dir1_lambda1;
      QLabel *lbl_dir1_lambda2;
      QLabel *lbl_dir2_lambda1;
      QLabel *lbl_dir2_lambda2;
      QLabel *lbl_dir1_cells1;
      QLabel *lbl_dir1_cells2;
      QLabel *lbl_dir2_cells1;
      QLabel *lbl_dir2_cells2;
      QLabel *lbl_dir1_scans1;
      QLabel *lbl_dir2_scans1;
      QLabel *lbl_dir1_scans2;
      QLabel *lbl_dir2_scans2;
      QLabel *lbl_message1;
      QLabel *lbl_message2;
      QLabel *lbl_dir1_info1;
      QLabel *lbl_dir1_info2;
      QLabel *lbl_dir2_info1;
      QLabel *lbl_dir2_info2;
      QLabel *lbl_dir1_first_scan1;
      QLabel *lbl_dir1_first_scan2;
      QLabel *lbl_dir2_first_scan1;
      QLabel *lbl_dir2_first_scan2;
      QLabel *lbl_dir1_last_scan1;
      QLabel *lbl_dir2_last_scan1;
      QLabel *lbl_dir1_last_scan2;
      QLabel *lbl_dir2_last_scan2;
      QListBox *lb_dir1_cells;
      QListBox *lb_dir2_cells;
      QProgressBar *pgb_progress;
      QPushButton *pb_select_dir1;
      QPushButton *pb_select_dir2;
      QPushButton *pb_order_dir1_cell;
      QPushButton *pb_order_dir2_cell;
      QPushButton *pb_order_dir1_all;
      QPushButton *pb_order_dir2_all;
      QPushButton *pb_merge;
      QPushButton *pb_check_dir1_lambda;
      QPushButton *pb_check_dir2_lambda;
      QPushButton *pb_help;
      QPushButton *pb_quit;
      US_Config *USglobal;

   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *e);

   private slots:
      void select_dir1();
      void select_dir2();
      void merge();
      void show_dir1_cell(int);
      void show_dir2_cell(int);
      void order_dir1_cell();
      void order_dir2_cell();
      void order_dir1_all();
      void order_dir2_all();
      void update_if_button();
      void update_wl_button();
      void update_abs_button();
      void update_copy_button();
      void update_move_button();
      void check_dir1_lambda();
      void check_dir2_lambda();
      void help();
      void quit();
};


#endif

