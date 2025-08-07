//! \file us_select_edits.h
#ifndef US_SELECT_EDITS_H
#define US_SELECT_EDITS_H

#include "us_dataIO.h"
#include "us_db2.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

//! \brief A dialog class to select edit IDs of runs for models,noises
//! pre-filter
class US_GUI_EXTERN US_SelectEdits : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! \brief Main constructor for dialog to select edits(s)
  //! \param dbase    Flag: Data source is database
  //! \param edIDs    A reference to a list of selected editIDs

  US_SelectEdits(bool, QStringList&);

 signals:
  //! \brief A signal to the parent that disk/db selectin has changed
  //! \param DB  True if DB source is now selected
  void dkdb_changed(bool DB);

 private:
  bool sel_db;  //!< Select-database flag

  QStringList& editIDs;  //!< Selected edit IDs (GUIDs for local)

  US_Help showHelp;

  class EditDesc  //!< description of each edit set in the list presented
  {
   public:
    QString key;     //!< Edit key (run:triple:edit)
    QString runID;   //!< Run identifier
    QString label;   //!< Experiment label
    QString editID;  //!< Edit ID (DB) or GUID (Local)
    QString date;    //!< Updated-date
    QString DB_id;   //!< Database ID of experiment
  };

  QMap<QString, EditDesc> editmap;  //!< map of labels,data-desc-objs
  QMap<QString, EditDesc> runmap;   //!< map of runids,data-desc-objs

  EditDesc edesc;       //!< Current selected data's description
  QStringList elabels;  //!< Keys for editmap
  QStringList rlabels;  //!< RunIDs in editmap

  QPushButton* pb_invest;  //!< Investigator button

  QLineEdit* le_invest;   //!< Investigator id:name text entry
  QLineEdit* le_dfilter;  //!< Data search filter text entry

  QLabel* lb_list;  //!< Label at top of data list

  QTableWidget* tw_data;  //!< Data table list widget

  US_Disk_DB_Controls* dkdb_cntrls;  //!< Disk/DB controls

  QString dsearch;  //!< Current data search string

 private slots:

  void scan_dbase_edit(void);
  void scan_local_edit(void);
  void build_runids(void);
  void list_data(void);
  void search(const QString&);
  void cancelled(void);
  void accepted(void);
  void get_person(void);
  void update_person(int);
  void update_disk_db(bool);
  void help(void) { showHelp.show_help("select_edits.html"); };
};
#endif
