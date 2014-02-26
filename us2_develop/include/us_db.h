#ifndef US_DB_H
#define US_DB_H

#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlcursor.h>
#include <qapplication.h>
#include   <qlayout.h>

#include "us_database.h"
#include "us_db_t.h"

//! This class is a GUI wrapper for the basic non-GUI database implementation US_DB_T
/*!
   This basic class build basic functions for inherited class using, like database connect and disconnect. 
*/
class US_EXTERN US_DB : public QDialog
{
   Q_OBJECT
   
   public:
      US_DB (QWidget *p=0, const char *name="us_db");
      ~US_DB();
      
      struct US_LoginData login_list;      /*!< A struct US_LoginData for storing DB login information. */
      US_Config *USglobal;            /*!< A reference of US_Config. */   
      QSqlDatabase *defaultDB;            /*!< A reference of QSqlDatabase. */
      bool DB_flag;                        /*!< A bool variable for choosing use database or not. */
      int returnval;
      QString errorMessage;
      US_DB_T *terminalDB;
      int db_connect();
      int db_connect(struct US_DatabaseLogin);
      int get_newID(QString table, QString key);
      QString make_tempFile(QString, QString);
      void remove_temp_dir(QString);
      bool read_blob(QString, QSqlCursor, QString);
      bool write_blob(QString, QSqlRecord *, QString);
      void clean_db(QString, QString, QString, QString, int);
      QString show_investigator(int);
      QString show_buffer(int);
      QString show_peptide(int);
      QString show_DNA(int);
      QString show_image(int);
      QString show_request(int);
      QString show_sample(int);
      void close_db();

   public slots:

      void newMessage(QString, int);
};

#endif


