#ifndef US_RTVDESCRIPTION_H
#define US_RTVDESCRIPTION_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qlistbox.h>

#include "us_db_tbl_expdata.h"

//! An Interface for retrieving experimental data from Database through Description.
/*!
   This interface can be found in "Database -> Retrive Experimental Data from DB -> Description".
   You can use this interface to input an experimental description as condition to retrieve 
   experimental data from DB or let DB list all experimental data names for your select.
*/
class US_EXTERN US_DB_RtvDescription : public US_DB
{
   Q_OBJECT
   
   public:
      US_DB_RtvDescription(QWidget *p=0, const char *name="us_rtvdescription");
      ~US_DB_RtvDescription();
      
      QString Description,                /*!< A string variable for Description. */
              Display;                     /*!< A string variable for data item dispaly. */ 
      int ExpdataID;                        /*!< An integer variable for ExpData id. */
      int *item_ExpdataID;                  /*!< An integer array stores ExpData id.*/
      QString *display_Str;               /*!< A string array stores dispaly string.*/
      bool sel_data,                      /*!< A flag use for data listbox select, initialize to <var>false</var>. */
           retrieve_flag;                  /*!< A flag use for 'Retrieve' pushbutton, initialize to <var>false</var>. */
      QLabel *lbl_blank;                  /*!< A label shows 'Retrieve Experimental Data by Description from DB:' + dbname. */
      QLabel *lbl_description;            /*!< A label shows 'Input Data Description:'. */
      QLabel *lbl_instr;                  /*!< A label shows 'Doubleclick on item to select:'. */
      QLabel *lbl_item;                     /*!< A label shows selected experimental data item. */
      QLineEdit *le_description;            /*!< A LineEdit for input data description. */
      QListBox *lb_data;                  /*!< A ListBox shows a experimental data name list. */
      QPushButton *pb_chkDescription;      /*!< A PushButton connect to checkDescription(). */
      QPushButton *pb_help;               /*!< A PushButton connect to help(). */
      QPushButton *pb_retrieve;            /*!< A PushButton connect to retrieve(). */
      QPushButton *pb_reset;               /*!< A PushButton connect to reset(). */
      QPushButton *pb_close;               /*!< A PushButton connect to quit(). */
      US_ExpData_DB *expdata_dlg;         /*!< A reference of US_ExpData_DB. */
         
   private slots:
      void setup_GUI();
// pushbuttons:
      void checkDescription();
      void reset();
      void retrieve();
      void help();
      void quit();
// lineedit:
      void update_description(const QString&);   
// Listbox:
      void select_data(int item);
};

#endif


