//! \file us_select_runs.h
#ifndef US_SELECT_RUNS_H
#define US_SELECT_RUNS_H

#include "us_dataIO.h"
#include "us_db2.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

//! \brief A dialog class to select run IDs of runs for models,noises pre-filter
class US_GUI_EXTERN US_SelectRuns : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief Main constructor for dialog to select runs(s)
  //! \param dbase    Flag: Data source is database
  //! \param runIDs   A reference for an output list of selected runIDs

  US_SelectRuns(bool, QStringList&);

 signals:
  //! \brief A signal to the parent that disk/db selectin has changed
  //! \param DB  True if DB source is now selected
  void dkdb_changed(bool DB);

 private:
  bool sel_db;  //!< Select-database flag

  QStringList& runIDs;  //!< Selected run IDs

  US_Help showHelp;

  class RunDesc  //!< description of each edit set in the list presented
  {
   public:
    QString runID;  //!< Run identifier
    QString label;  //!< Experiment label
    QString date;   //!< Updated-date
    QString DB_id;  //!< Database ID of experiment
  };

  QMap<QString, RunDesc> runmap;  //!< map of runids,data-desc-objs

  RunDesc rdesc;        //!< Current selected data's description
  QStringList rlabels;  //!< RunIDs in runmap

  QPushButton* pb_invest;  //!< Investigator button

  QLineEdit* le_invest;   //!< Investigator id:name text entry
  QLineEdit* le_dfilter;  //!< Data search filter text entry

  QLabel* lb_list;  //!< Label at top of data list

  QTableWidget* tw_data;  //!< Data table list widget

  US_Disk_DB_Controls* dkdb_cntrls;  //!< Disk/DB controls

  QString dsearch;  //!< Current data search string

 private slots:

  void scan_dbase_run(void);
  void scan_local_run(void);
  void build_runids(void);
  void list_data(void);
  void search(const QString&);
  void cancelled(void);
  void accepted(void);
  void get_person(void);
  void update_person(int);
  void update_disk_db(bool);
  void help(void) { showHelp.show_help("select_runs.html"); };
};
#endif
