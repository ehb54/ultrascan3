//! \file us_sync_db.h
#ifndef US_SYNC_DB_H
#define US_SYNC_DB_H

#include "us_db2.h"
#include "us_extern.h"
#include "us_report.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

//! \brief A dialog class to select and load data (edited or raw)
class US_SyncWithDB : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \brief Main constructor for dialog to select edit and raw
      //!        data and load it

      US_SyncWithDB();

   private:
      QPushButton *pb_invest;
      QLineEdit *le_invest; // investigator text entry
      QTextEdit *te_desc;


      int nrunrpl; // Number of runids replaced
      int nrunadd; // Number of runids newly added
      int ndocrpl; // Number of documents replaced
      int ndocadd; // Number of documents newly added

      bool newonly; // Flag if downloads are only of new records

   private slots:
      void get_person(void);
      void scan_db_reports(void);
      void cancelled(void);
      void accepted(void);
      void downnew(void);
};
#endif
