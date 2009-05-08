#ifndef US_DB_TEMPLATE_H
#define US_DB_TEMPLATE_H

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qdatastream.h>
#include <qstringlist.h>
#include <qlayout.h>

#include "us_database.h"
#include "us_db_admin.h"

//! An Interface for initialization database.
/*!
   Create UltraScan Database is a simple thing.
   Just type in your desired Database Name in US_Database interface and save it.
   Come here to click the 'Initialize' button, 
   this program will check the database and create all tables and tables' fields for you.\n
   This interface can be found in "Database->Initialize Database Template".
*/
class US_EXTERN US_DB_Template : public QFrame
{
   Q_OBJECT
   
   public:
      US_DB_Template(QWidget *p=0, const char *name="us_db_template");
      ~US_DB_Template();
      
      struct US_LoginData login_list;   /*!< A struct US_LoginData for storing DB login information. */
      
      QString *table_name;               /*!< A String Array store DB name.*/
      QLabel *lbl_blank;               /*!< A raised Label shows 'Database Template Initialization'. */
      QLabel *lbl_dbname;               /*!< A Label shows 'Initialize DB Template:'. */
      QLabel *lbl_dbname2;               /*!< A Label shows DB name which is the value of struct US_LoginData <var>dbname</var>. */

      QPushButton *pb_help;            /*!< A PushButton connect to help(). */
      QPushButton *pb_submit;            /*!< A PushButton connect to initialize(). */
      QPushButton *pb_close;            /*!< A PushButton connect to quit(). */
      
      US_Config *USglobal;         /*!< A US_Config reference. */
      QProcess *proc;
      void read();
      void create();
   public slots:   
      void readFromStderr();   
   private slots:
      void setup_GUI();
      void check_permission();
      void initialize(bool);
      void help();
      void quit();
};

#endif


