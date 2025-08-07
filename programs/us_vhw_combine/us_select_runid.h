//! \file us_select_runid.h
#ifndef US_SELECT_RUNID_H
#define US_SELECT_RUNID_H

#include "us_db2.h"
#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief A dialog class to select run ID for vHW plot distributions
class US_SelectRunid : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \brief Main constructor for dialog to select edits(s)
      //! \param dbase    Flag: Data source is database
      //! \param runIDs   A reference to the selected runIDs list

      US_SelectRunid(bool, QStringList &);

   signals:
      //! \brief A signal to tell the parent when the disk/db selection
      //!        has chagned
      //! \param isDB  True if DB is selected
      void changed(bool isDB);

   private:
      QStringList &runIDs; // Selected run IDs
      bool sel_db; // Select-database flag

      QPushButton *pb_invest; // Investigator button

      QLineEdit *le_invest; // Investigator id:name text entry
      QLineEdit *le_dfilter; // Data search filter text entry

      QTextEdit *te_status; // Text box for current list status

      QListWidget *lw_data; // Data list widget

      US_Disk_DB_Controls *dkdb_cntrls; // Radio buttons for disk/db choice

      QString dsearch; // Current data search string

      QStringList rlabels; // Run ID labels
      QStringList slabels; // Date-sorted run ID labels

      QMap<QString, QString> mRDates; // Mapping of dates to runIDs

      int count_allr; // Count of all user runs
      int count_list; // Count of runs with vHW distribution plots
      int count_seld; // Count of runs currently selected;
      int dbg_level; // Debug level

   private slots:

      void scan_dbase_runs(void);
      void scan_local_runs(void);
      void list_data(void);
      void search(const QString &);
      void cancelled(void);
      void accepted(void);
      void get_person(void);
      void update_person(int);
      void update_disk_db(bool);
      void selectionChanged(void);
      void sort_rlabels(const QStringList);
};
#endif
