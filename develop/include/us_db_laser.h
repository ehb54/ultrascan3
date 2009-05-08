#ifndef US_DB_LASER_H
#define US_DB_LASER_H

#include <qcombobox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qwt_plot.h>

#include "us_laser.h"
#include "us_db_tbl_investigator.h"
#include "us_db_tbl_nucleotide.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"

class US_EXTERN US_DB_Laser : public US_Laser 
{
   Q_OBJECT
   
   public:
      US_DB_Laser(QWidget *p = 0, const char *name = "us_db_laser");
      ~US_DB_Laser();
   
   QPushButton *pb_load_db;            /*!< A PushButton connect to read_db(). */
   QPushButton *pb_save_db;            /*!< A PushButton connect to save_db(). */
   QPushButton *pb_del_db;               /*!< A PushButton connect to del_db(). */   
   QPushButton *pb_reset;               /*!< A PushButton connect to clear(). */
   QPushButton   *pb_investigator;         /*!< A PushButton connect to sel_investigator(). */
   QPushButton *pb_peptide;            /*!< A PushButton connect to sel_vbar(). */
   QPushButton *pb_DNA;                  /*!< A PushButton connect to sel_DNA(). */
   QPushButton *pb_buffer;               /*!< A PushButton connect to sel_buffer(). */

   QLabel *instr_lbl;                  /*!< A Label shows 'Doubleclick to select:'. */
   QLineEdit *le_investigator;         /*!< A LineEdit shows selected investigator Information(read only). */
   QLineEdit *le_buffer;               /*!< A LineEdit shows selected buffer Information(read only). */
   QLineEdit *le_peptide;               /*!< A LineEdit shows selected peptide Information(read only). */
   QLineEdit *le_DNA;                  /*!< A LineEdit shows selected DNA Information(read only). */
   QListBox *lb_data;                  /*!< A ListBox shows a DB data list. */   
         
   int *item_LaserID, Item;
   QString *display_Str, *item_Sample;
   bool sel_flag;
   protected slots:
      void sel_investigator();
      void update_investigator_lbl(QString, int );
      void sel_buffer();
      void update_buffer_lbl(int);
      void sel_vbar();
      void update_vbar_lbl(int);
      void sel_DNA();
      void update_DNA_lbl(int);
      void save_db();
      void load_db();
      void select_data(int);
      void check_permission();
      void del_db(bool);
      void init();
      void reset();
      virtual void resizeEvent(QResizeEvent *e);
      void closeEvent(QCloseEvent *e);
};

#endif

