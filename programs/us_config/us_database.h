//! \file us_database.h
#ifndef US_DATABASE_H
#define US_DATABASE_H

#include <QtCore>

#include "us_widgets.h"

/*! \brief A class to allow the user to set, modify or delete
    parameters for database access.
*/
class US_Database : public US_Widgets {
      Q_OBJECT

   public:
      //!  Construct the window to manage database parameters.
      explicit US_Database(QWidget *w = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
      int dbg_level;

   private:
      QList<QStringList> dblist;
      QString uuid;

      QPushButton *pb_delete;
      QPushButton *pb_save;
      QPushButton *pb_reset;
      QPushButton *pb_testConnect;
      QPushButton *pb_paste;
      QLineEdit *le_description;
      QLineEdit *le_username;
      QLineEdit *le_password;
      QLineEdit *le_dbname;
      QLineEdit *le_host;
      QLineEdit *le_investigator_email;
      QLineEdit *le_investigator_pw;

      QListWidget *lw_entries;

      void update_lw(const QString & = nullptr);
      bool parse_database_url(const QString &);
      static QString validate_value(QLineEdit *, const QString &);

   private slots:
      void help();
      int get_autoflow_records();
      void select_db(QListWidgetItem *);
      void check_add();
      void reset();
      void save_default();
      void deleteDB();
      bool test_connect();
      void update_inv();
      void paste_db();
      void values_updated() const;
};
#endif
