#ifndef US_NUCLEOTIDE_DB_H
#define US_NUCLEOTIDE_DB_H

#include <qlistbox.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qwt_counter.h>
#include "us_enter_dna.h"
//#include "us_util.h"
#include "us_db.h"
#include "us_db_tbl_investigator.h"

//! A Struct for storing required info for DNA data in DB.
/*! 
   Each element is according to an entry field in Database Table <tt>tblDNA</tt>.
*/
struct US_DNAData
{
   int DNAID;                  /*!< An integer number for tblDNA entry. */
   QString DNAFileName;         /*!< A string stored the value of DNA filename.*/
   QString Description;         /*!< A string variable for DNA description. */
   QString Sequence;            /*!< A string variable for DNA sequence. */
   int InvID;                  /*!< An integer number for tblInvestigators reference id. */
   float vbar;                  /*!< A floating value for the partial specific volume. */
   float e260;                  /*!< A floating value for the molar extinction at 260 nm. */
   float e280;                  /*!< A floating value for the molar extinction at 280 nm. */

};

//! An Interface for DNA data with Database.
/*!
   This interface can be found in "Database->Commit Data to DB->Nucleotide Sequence".
   You can use this interface to pickup your DNA values.
   When you Save Result to HD, the program will save your DNA data to hard drive.
   When you Save Sequence to DB, the program will save your DNA data to database table: tblDNA.
   You can use this interface to load your DNA data records from Hard Drive or DB.
   Also you can do some Molecular Weight calculation with nucleotide sequence.
*/
class US_EXTERN US_Nucleotide_DB : public US_DB
{
   Q_OBJECT
   
   public:
      US_Nucleotide_DB(bool from_cell, int temp_InvID, QWidget *p=0, const char *name = 0);
      ~US_Nucleotide_DB();
      
      struct US_DNAData DNA_info;         /*!< A struct US_DNAData for storing DNA Data. */
      int *item_DNAID;                     /*!< A Integer Array store DNA ID.*/
      QString *item_Description,          /*!< A String Array store DNA file name.*/
              *display_Str;               /*!< A String Array store DNA name dispaly.*/
      int Item;                           /*!< A Integer variable for ListBox item select. */
      bool select_flag,                   /*!< A flag for query listbox select, initialize to <var>false</var>. */
           cell_flag,                     /*!< A flag for checking this wigdet whether is called from US_Cell table or not. */
           from_HD;                        /*!< A flag for loading DNA data whether is from Hard Drive or not. */
      
      bool doubleStranded, _3prime_oh, _5prime_oh, isDNA, complement;
      float sodium, calcium, lithium, potassium, magnesium, MW, e260, e280, vbar;
      unsigned int A, C, G, T, U, total;
      QString filename, title;
      US_Enter_DNA_DB *enterDNADlg;

      QPushButton *pb_load;
      QPushButton *pb_download;
      QPushButton *pb_update;
      QPushButton *pb_save;
      QPushButton *pb_help;
      QPushButton *pb_quit;
      QPushButton *pb_load_db;            /*!< A PushButton connect to read_db(). */
      QPushButton *pb_enter_DNA;            /*!< A PushButton connect to save_db(). */
      QPushButton *pb_del_db;               /*!< A PushButton connect to delete_db(). */
      QPushButton *pb_reset;               /*!< A PushButton connect to reset(). */
      QPushButton   *pb_investigator;         /*!< A PushButton connect to sel_investigator(). */
      
      QCheckBox *cb_doubleStranded;
      QCheckBox *cb_3prime_oh;
      QCheckBox *cb_3prime_po4;
      QCheckBox *cb_5prime_oh;
      QCheckBox *cb_5prime_po4;
      QCheckBox *cb_DNA;
      QCheckBox *cb_RNA;
      QCheckBox *cb_complement;
      
      QLabel *lbl_banner4;                  /*!< A Label shows 'Doubleclick on buffer data to select:'. */
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_banner3;
      QLabel *lbl_doubleStranded;
      QLabel *lbl_complement;
      QLabel *lbl_3prime_oh;
      QLabel *lbl_3prime_po4;
      QLabel *lbl_5prime_oh;
      QLabel *lbl_5prime_po4;
      QLabel *lbl_sodium;
      QLabel *lbl_potassium;
      QLabel *lbl_lithium;
      QLabel *lbl_calcium;
      QLabel *lbl_magnesium;
      QLabel *lbl_sequence;
      QLabel *lbl_mw;
      QLabel *lbl_vbar;
      QLabel *lbl_e280;
      QLabel *lbl_e260;
      QLabel *lbl_DNA;
      QLabel *lbl_RNA;
      QLabel *lbl_investigator;            /*!< A Label shows selected investigator Information. */

      QLineEdit *le_sequence;
      QLineEdit *le_mw;
      QLineEdit *le_vbar;
      QLineEdit *le_e260;
      QLineEdit *le_e280;
      QwtCounter *cnt_sodium;
      QwtCounter *cnt_lithium;
      QwtCounter *cnt_potassium;
      QwtCounter *cnt_calcium;
      QwtCounter *cnt_magnesium;

      QListBox *lb_DNA;                     /*!< A ListBox shows a DNA filename list. */
   
   public slots:
      void retrieve_DNA(int);
         
   private slots:
   
      void setup_GUI();
      void load();
      void download();
      void update();
      void update_vbar(const QString &);
      void update_e280(const QString &);
      void update_e260(const QString &);
      void update_doubleStranded();
      void update_complement();
      void update_3prime_oh();
      void update_3prime_po4();
      void update_5prime_oh();
      void update_5prime_po4();
      void update_DNA();
      void update_RNA();
      void update_sodium(double);
      void update_potassium(double);
      void update_lithium(double);
      void update_calcium(double);
      void update_magnesium(double);
      void select_DNA(int item);
      void sel_investigator();
      void update_investigator_lbl(QString, int);
      void save();
      void help();
      void read_db();
      void enter_DNA();
      void check_permission();
      void delete_db(bool);
      void clear();
      void reset();
      void quit();
      void closeEvent(QCloseEvent *e);
         
   signals:
/*!
   This signal is emitted whenever the widget is called by US_Cell_DB.\n
   The argument is integer <var>DNAID</var>.
*/
      void IdChanged(int DNAID);
};

#endif

