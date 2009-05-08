#ifndef US_DB_TBL_CELL_H
#define US_DB_TBL_CELL_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>

#include "us_db_tbl_nucleotide.h"
#include "us_db_tbl_buffer.h"
#include "us_db_tbl_vbar.h"
#include "us_util.h"
//! A Struct for storing required info for Cell Data in DB.
/*! 
   Each element is according to an entry field in Database Table <tt>tblCell</tt>.
*/
struct US_CellData
{
   int CellID;                           /*!< Table ID of <tt>tblCell</tt>. */
   int CenterpieceID;                  /*!< Centerpiece file index. */
   int ExperimentID;                     /*!< Table ID of <tt>tblExpData</tt>. */
   int Position;                        /*!< Cell position (1-8). */
   int InvID;                           /*!< Table ID of <tt>tblInvestigators</tt>. */
   QString Description;                  /*!< Description of cell. */
   int Wavelength[3];                  /*!< Wavelength 1-3 of cell. */
   int Scans[3];                        /*!< Scan 1-3 of cell. */
   int BufferID[4];                     /*!< Table ID of <tt>tblBuffer</tt>, -1 means empty. */
   int DNAID[4][3];                     /*!< Table ID of <tt>tblDNA</tt>, -1 means empty. */
   int PepID[4][3];                     /*!< Table ID of <tt>tblPeptide</tt>, -1 means empty. */
   int Num_Channel;                     /*!< Number of channels, 1 or 3. */
};

//! An Interface for cell data of experimental data.
/*!
   This interface will show out when US_ExpData_db is called.
   This interface can store or retrieve cell data of a particular
   experimental data from DB table <tt>tblCell</tt>.
*/
class US_EXTERN US_Cell_DB : public US_DB
{
   Q_OBJECT
   public:
      US_Cell_DB(struct US_ExpData exp_info, int *cell_table_unfinished, QWidget *parent=0, const char *name="us_tblcell");
      ~US_Cell_DB();
      
      struct US_CellData cell_info;      /*!< A struct US_CellData for storing Cell info. */
      struct US_ExpData exp_info;      /*!< A struct US_ExpData for storing experimental data info. */
      int *item_CellID;                  /*!< An integer array stores tblCell id.*/

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif

      vector <struct centerpieceInfo> cp_info_vector;   /*!<centerpiece information */

#ifdef WIN32
        #pragma warning ( default: 4251 )
#endif

      QString *item_Description;         /*!< A string array stores cell description.*/
      int *cell_table_unfinished;      /*!< An integer point variable for checking cell table finish.*/
      bool    select_flag,               /*!< A flag use for data listbox select, initialize to <var>false</var>. */
            from_db,                   /*!< A flag use for delete function, initialize to <var>false</var>. */
            from_cell;                  /*!< An argument use in US_Buffer_DB,US_Nucleotide_DB and US_Vbar_DB. 
                                          set to <var>true</var> here. */
      QPushButton *pb_help;            /*!< A PushButton connect to help(). */
      QPushButton *pb_close;            /*!< A PushButton connect to quit(). */
      QPushButton *pb_add;               /*!< A PushButton connect to add_db(). */
      QPushButton *pb_delete;            /*!< A PushButton connect to delete_db(). */
      QPushButton *pb_reset;            /*!< A PushButton connect to reset(). */
      QPushButton *pb_query;            /*!< A PushButton connect to query_db(). */   
      QPushButton *pb_peptide1;         /*!< A PushButton connect to sel_vbar1(). */
      QPushButton *pb_peptide2;         /*!< A PushButton connect to sel_vbar2(). */
      QPushButton *pb_peptide3;         /*!< A PushButton connect to sel_vbar3(). */
      QPushButton *pb_DNA1;               /*!< A PushButton connect to sel_DNA1(). */
      QPushButton *pb_DNA2;               /*!< A PushButton connect to sel_DNA2(). */
      QPushButton *pb_DNA3;               /*!< A PushButton connect to sel_DNA3(). */
      QPushButton *pb_buffer;            /*!< A PushButton connect to sel_buffer(). */
      QLabel *lbl_sample;               /*!< A label shows 'Cell Table ID:'. */
      QLabel *lbl_cellid;               /*!< A label shows seleted cell id. */
      QLabel *lbl_investigator;         /*!< A label shows 'Investigator ID:'. */
      QLabel *lbl_invest;               /*!< A label shows selected investigator id. */
      QLabel *lbl_centerpiece;         /*!< A label shows 'Centerpiece ID:'. */
      QLabel *lbl_ctpc;                  /*!< A label shows centerpiece index. */
      QLabel *lbl_experiment;            /*!< A label shows 'Experiment ID:'. */
      QLabel *lbl_exp;                  /*!< A label shows tblExpData id. */
      QLabel *lbl_cell;                  /*!< A label shows 'Cell Position:'. */
      QLabel *lbl_position;            /*!< A label shows cell position. */
      QLabel *lbl_wavelength1;         /*!< A label shows 'Wavelength 1:'. */
      QLabel *lbl_wavelength2;         /*!< A label shows 'Wavelength 2:'. */
      QLabel *lbl_wavelength3;         /*!< A label shows 'Wavelength 3:'. */
      QLabel *lbl_wl1;                  /*!< A label shows wavelength 1 value. */
      QLabel *lbl_wl2;                  /*!< A label shows wavelength 2 value. */
      QLabel *lbl_wl3;                  /*!< A label shows wavelength 3 value. */
      QLabel *lbl_wlscan1;               /*!< A label shows '# of Scans for WL 1:'. */
      QLabel *lbl_wlscan2;               /*!< A label shows '# of Scans for WL 2:'. */
      QLabel *lbl_wlscan3;               /*!< A label shows '# of Scans for WL 3:'. */
      QLabel *lbl_scan1;               /*!< A label shows the number of scans for wavelength 1. */
      QLabel *lbl_scan2;               /*!< A label shows the number of scans for wavelength 2. */
      QLabel *lbl_scan3;               /*!< A label shows the number of scans for wavelength 3. */
      QLabel *lbl_channel;               /*!< A label shows 'Channel:'. */
      QLabel *le_buffer;               /*!< A Label shows selected buffer Information */
      QLabel *le_peptide1;               /*!< A Label shows selected peptide1 Information. */
      QLabel *le_peptide2;               /*!< A Label shows selected peptide2 Information. */
      QLabel *le_peptide3;               /*!< A Label shows selected peptide3 Information. */
      QLabel *le_DNA1;                  /*!< A Label shows selected DNA1 Information). */
      QLabel *le_DNA2;                  /*!< A Label shows selected DNA2 Information. */   
      QLabel *le_DNA3;                  /*!< A Label shows selected DNA3 Information. */
//      QLineEdit *le_buffer;            /*!< A Label shows selected buffer Information. */
//      QLineEdit *le_peptide1;            /*!< A LineEdit shows selected peptide1 Information(read only). */
//      QLineEdit *le_peptide2;            /*!< A LineEdit shows selected peptide2 Information(read only). */
//      QLineEdit *le_peptide3;            /*!< A LineEdit shows selected peptide3 Information(read only). */
//      QLineEdit *le_DNA1;               /*!< A LineEdit shows selected DNA1 Information(read only). */
//      QLineEdit *le_DNA2;               /*!< A LineEdit shows selected DNA2 Information(read only). */   
//      QLineEdit *le_DNA3;               /*!< A LineEdit shows selected DNA3 Information(read only). */         
      QComboBox *cmbb_channel;         /*!< A Combobox shows selected channel. */
      QListBox *lb_query;               /*!< A ListBox shows cell name list. */
         
      US_Nucleotide_DB *DNA_dlg;         /*!< A reference of US_Nucleotide_DB. */
      US_Buffer_DB *buffer_dlg;         /*!< A reference of US_Buffer_DB. */
      US_Vbar_DB *vbar_dlg;            /*!< A reference of US_Vbar_DB. */
         
   private slots:
      void setup_GUI();
      void sel_DNA1();
      void sel_DNA2();
      void sel_DNA3();
      void update_DNA1_lbl(int DNAID);
      void update_DNA2_lbl(int DNAID);
      void update_DNA3_lbl(int DNAID);
      void sel_buffer();
      void update_buffer_lbl(int BuffID);
      void sel_vbar1();
      void sel_vbar2();
      void sel_vbar3();
      void update_vbar1_lbl(int PepID);
      void update_vbar2_lbl(int PepID);
      void update_vbar3_lbl(int PepID);
      void sel_channel(int);
      void add_db();
      void query_db();
      void sel_query(int);
      void check_permission();
      void delete_db(bool);
      void reset();
      void help();
      void quit();
      protected slots:   
   
      void closeEvent(QCloseEvent *e);
};

#endif

