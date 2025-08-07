//! \file us_solution_gui.h
#ifndef US_SOLUTION_GUI_H
#define US_SOLUTION_GUI_H

#include "us_analyte.h"
#include "us_buffer.h"
#include "us_extern.h"
#include "us_extinctfitter_gui.h"
#include "us_extinction_gui.h"
#include "us_help.h"
#include "us_minimize.h"
#include "us_new_spectrum.h"
#include "us_solution.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

//! This class provides a tabbed entry for solution selection
class US_SolutionMgrSelect : public US_Widgets {
  Q_OBJECT

 public:
  //! brief Solution Selection Tab. To
  //! instantiate the class a calling function must
  //! provide the ID of the investigator.

  //! \param invID          A pointer to the current investigator ID
  //! \param select_db_disk Indicates whether the default search
  //!    is on the local disk or in the DB
  //! \param tmp_solution    Pointer for selected solution
  //! \param tmp_experimentID Experiment ID
  //! \param tmp_channelID    Channel ID
  US_SolutionMgrSelect(int*, int*, US_Solution*, int, int);

  US_Solution* solution;
  US_Solution* tmp_solution;
  US_Solution orig_solution;

  int* personID;
  int* db_or_disk;
  bool from_db;

 signals:
  //! Currently selected solution is accepted by User
  void solutionAccepted(void);
  void selectionCanceled(void);

 private:
  int dbg_level;
  US_Help showHelp;
  QLineEdit* le_bufferInfo;
  QLineEdit* le_commonVbar20;
  QLineEdit* le_density;
  QLineEdit* le_viscosity;
  QLineEdit* le_storageTemp;
  QLineEdit* le_guid;
  QLineEdit* le_amount;
  QLineEdit* le_buffeInfo;
  QLineEdit* le_search;

  QListWidget* lw_solutions;
  QListWidget* lw_analytes;

  QTextEdit* te_notes;

  QPushButton* pb_cancel;
  QPushButton* pb_accept;
  QPushButton* pb_delete;
  QPushButton* pb_spectrum;
  QPushButton* pb_help;
  QPushButton* pb_upload;

  // For list widget
  class SolutionInfo {
   public:
    int solutionID;
    QString GUID;
    QString description;
    QString filename;
    int index;
  };

  QList<SolutionInfo> info;
  int investigatorID;
  int experimentID;
  int channelID;
  bool signal;
  bool autosave;
  bool changed;

  QStringList IDs;
  QStringList filenames;
  QStringList descriptions;
  QStringList GUIDs;
  QStringList solutionIDs;

  QMap<QListWidgetItem*, int> solutionMap;
  QMap<QListWidgetItem*, int> analyteMap;

  QLabel* lb_amount;

 private slots:

  void search(const QString& = QString());
  void selectSolution(QListWidgetItem*);
  void reset(void);
  void db_error(const QString&);

  void selectAnalyte(QListWidgetItem*);

  void loadDisk(void);
  void loadDB(void);

  void accept_solution(void);
  void spectrum(void);
  void delete_solution(void);
  void reject(void);
  void upload_solution(void);

  /* void delete_disk     ( void ); */
  /* void delete_db       ( void ); */
  /* bool solution_in_use  ( QString& ); */
  /* void info_solution    ( void ); */
  /* void select_solution  ( QListWidgetItem* ); */
  /* void select_solution  ( ); */
  /* void read_from_disk  ( QListWidgetItem* ); */
  /* void read_from_db    ( QListWidgetItem* ); */
  /* void read_from_db    ( const QString&   ); */

  //      void show_component  ( const QString&, double );

  /* void set_solution_type( int  ); */
  /* QString solution_info ( US_Solution* ); */
  /* QString solution_smry ( US_Solution* ); */
  /* void sequence        (void); */

  void help(void) { showHelp.show_help("solution_select.html"); };

 public slots:
  void load(void);
  // void init_solution      ( void );
};

//!      This class provides a tabbed entry for new solution creation
class US_SolutionMgrNew : public US_Widgets {
  Q_OBJECT

 public:
  //! brief Tab for entering a new Solution. To instantiate the class,
  //! a calling function must provide the ID of the investigator.
  //!
  //! \param invID            Pointer to the current investigator ID
  //! \param select_db_disk   Pointer to flag if the default search is on
  //!                           the local disk (0) or in the DB (1)
  //! \param tmp_solution     Pointer to a US_Solution object holding
  //!                           the active solution (for editing
  //!                           and adding new solutions)
  //! \param tmp_experimentID Experiment ID
  //! \param tmp_channelID    Channel ID
  US_SolutionMgrNew(int*, int*, US_Solution*, int, int);

 signals:
  void newSolAccepted(void);  //! New solution accepted
  void newSolCanceled(void);

 private:
  int* personID;
  int* db_or_disk;
  bool from_db;
  int dbg_level;

  int experimentID;
  int channelID;

  US_Solution* solution;

  QLabel* lb_descrip;
  QLineEdit* le_descrip;
  QPushButton* pb_accept;
  QPushButton* pb_reset;
  QPushButton* pb_cancel;
  QPushButton* pb_analyte;
  QPushButton* pb_buffer;
  QPushButton* pb_removeAnalyte;
  QPushButton* pb_spectrum;

  QLineEdit* le_commonVbar20;
  QLineEdit* le_density;
  QLineEdit* le_viscosity;
  QLineEdit* le_storageTemp;

  QTextEdit* te_notes;

  QwtCounter* ct_amount;
  QLabel* lb_amount;

  QLabel* lb_bufferInfo;
  QLineEdit* le_bufferInfo;

  QListWidget* lw_analytes;

  QMap<QListWidgetItem*, int> analyteMap;

  //! A SolutionComponent map structure for all components in
  //!   template list (stored in us_home/etc/solution.xml).
  // QMap< QString, US_SolutionComponent > component_list;

  US_Help showHelp;

 private slots:

  void addAnalyte(void);
  void assignAnalyte(US_Analyte);
  void calcCommonVbar20(void);
  void reset(void);

  void new_description();
  void saveAmount(double);
  void selectAnalyte(QListWidgetItem*);
  void removeAnalyte(void);
  void newCanceled(void);
  void selectBuffer(void);
  void assignBuffer(US_Buffer);
  void newAccepted(void);
  void saveTemperature(const QString&);
  void saveNotes(void);
  void spectrum_class(void);

  void help(void) { showHelp.show_help("solution_new.html"); };

 public slots:
  void newSolution(void);
};

//! This class provides a tabbed entry for non-hydrodynamic solution mods
class US_SolutionMgrEdit : public US_Widgets {
  Q_OBJECT

 public:
  //! brief Tab for entering a new Solution. To instantiate the class,
  //!  a calling function must provide the ID of the investigator.
  //!
  //! \param invID            Pointer to the current investigator ID
  //! \param select_db_disk   Pointer to flag if the default search is on
  //!                           the local disk (0) or in the DB (1)
  //! \param tmp_solution     Pointer to a US_Solution object holding the
  //!                           active solution (for editing and adding
  //!                           new solutions)
  //! \param tmp_experimentID Experiment ID
  //! \param tmp_channelID    Channel ID
  US_SolutionMgrEdit(int*, int*, US_Solution*, int, int);
  QString edit_solution_description;
  // US_Extinction *w;

 signals:
  void editSolAccepted(void);  //! Edited solution accepted
  void editSolCanceled(void);

 private:
  int experimentID;
  int channelID;

  int* personID;
  int* db_or_disk;
  bool from_db;
  int dbg_level;

  US_Solution* solution;
  US_Solution orig_solution;

  QPushButton* pb_accept;
  QLineEdit* le_descrip;
  QLineEdit* le_bguid;
  QLineEdit* le_storageTemp;

  QTextEdit* te_notes;
  US_Help showHelp;

 private slots:
  void spectrum_class(void);
  void editAccepted(void);
  void editCanceled(void);
  // void ph          ( void );
  void change_spectrum(void);
  void accept_enable(void);
  void saveTemperature(const QString&);
  void saveNotes(void);
  void help(void) { showHelp.show_help("solution_edit.html"); };
  void description(void);

 public slots:
  void init_solution(void);
};

//! This class provides a tabbed entry for general solution settings
class US_SolutionMgrSettings : public US_Widgets {
  Q_OBJECT

 public:
  //! \brief Selection tab for changing investigator and
  //!        choosing between db/disk access, as well as
  //!        for synchronizing the solution components local
  //!        file with the database.
  //!
  //! \param invID          A pointer to the current investigator ID
  //! \param select_db_disk A pointer to a flag that indicates whether
  //!                       the default search is on the local disk or
  //!                       in the DB
  US_SolutionMgrSettings(int*, int*);

 private:
  int* personID;
  int* db_or_disk;
  int dbg_level;
  bool from_db;

  QLineEdit* le_investigator;
  // QLineEdit*    le_syncstat;

  US_Disk_DB_Controls* disk_controls;

  US_Help showHelp;

 signals:
  //! A signal to indicate that the current disk/db selection has changed.
  //! /param DB True if DB is the new selection
  void use_db(bool DB);
  //! A signal to indicate that the current investigator was changed.
  //! /param invID is the new selection
  void investigator_changed(int invID);

 private slots:
  void sel_investigator(void);
  void db_changed(bool);
  void assign_investigator(int);
  // void synch_components   ( void );

  void help(void) { showHelp.show_help("solution_settings.html"); };
};

//! This class provides the main Solutions GUI dialog
class US_GUI_EXTERN US_SolutionGui : public US_WidgetsDialog {
  Q_OBJECT

 public:
  /*! \brief Generic constructor for the US_SolutionGui class. To
             instantiate the class a calling function must
             provide a structure to contain all the data.

      \param expID   An integer value that indicates the ID of
                     the associated experiment
      \param chID    An integer value that indicates the ID of
                     the channel used
      \param signal_wanted A boolean value indicating whether the caller
                     wants a signal to be emitted
      \param select_db_disk Indicates whether the default search is on
                     the local disk or in the DB
      \param dataIn  A reference to a structure that contains
                     the currently selected c/c/w dataset.
      \param auto_save A boolean value indicating whether the caller
                     wants an automatic save at Accept.
  */
  US_SolutionGui(int = 1, int = 1, bool = false,
                 int = US_Disk_DB_Controls::Default,
                 const US_Solution = US_Solution(), bool = true);

  //! A null destructor.
  ~US_SolutionGui() {};

 signals:
  //! A signal to indicate that the solution density and viscosity
  //!  has been updated and the screen is closing.
  //! \param density   - new density of the solution
  //! \param viscosity - new viscosity of the solution
  void valueChanged(double density, double viscosity);

  //! A signal to indicate that the solution data
  //!  has been updated and the screen is closing.
  //! \param solution   - the updated solution data.
  void valueChanged(US_Solution solution);

  //! A signal to indicate that the current disk/db selection has changed.
  //! Return the ID of the solution in the current database.  A
  //! value of -1 indicates the data was manually input or was
  //! returned from the local disk.
  //! \param solutionID - A string value of the returned ID
  // void valueSolutionID( const QString solutionID );
  void valueSolutionID(const int solutionID);

  /*! \brief The signal that is emitted when the user chooses
    to accept the current choices. This information is
    passed back to the calling function.

    \param solution A reference to a structure that contains the
    solution, buffer and analyte choices for a
    single c/c/w combination
  */
  void updateSolutionGuiSelection(US_Solution solution);

  /*! \brief The signal that is emitted when the user chooses
    to cancel the current selection. In this case all
             previously-entered experiment parameter associations
             are erased.
  */
  void cancelSolutionGuiSelection(void);

  //! A signal to indicate that the current disk/db selection has changed.
  //! /param DB True if DB is the new selection
  void use_db(bool DB);

  // A signal to catch in US_Experiment when new Solution added while called
  // from us_Exp
  void newSolAdded(void);

 private:
  int investigatorID;
  int experimentID;
  int channelID;
  bool autosave;
  bool changed;
  QStringList IDs;
  QStringList descriptions;
  QStringList GUIDs;
  QStringList filenames;

  bool signal;
  QString guid;
  double temperature;

  int disk_or_db;
  int personID;
  int dbg_level;
  bool from_db;
  bool solutionCurrent;
  bool manualUpdate;
  bool view_shared;
  bool access;

  QTabWidget* tabWidget;
  US_SolutionMgrSelect* selectTab;
  US_SolutionMgrNew* newTab;
  US_SolutionMgrEdit* editTab;
  US_SolutionMgrSettings* settingsTab;

  //!< The currently active solution Data.
  US_Solution solution;
  US_Solution orig_solution;  // saves original solution upon entry,
                              //   is returned if cancel was pressed

 private slots:
  void checkTab(int);
  void update_disk_or_db(bool);
  void update_personID(int);

  //      void sel_investigator   ( void );
  //      void source_changed     ( bool );
  void value_changed(const QString&);
  //      void assign_investigator( int  );
  //      void synch_components   ( void );
  void editSolAccepted(void);
  void editSolCanceled(void);
  void newSolAccepted(void);
  void newSolCanceled(void);
  void solutionAccepted(void);
  void solutionRejected(void);
};

#endif
