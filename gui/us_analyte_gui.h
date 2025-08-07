//! \file us_analyte_gui.h
#ifndef US_ANALYTE_GUI_H
#define US_ANALYTE_GUI_H

#include <qwt_counter.h>

#include "us_analyte.h"
#include "us_constants.h"
#include "us_db2.h"
#include "us_editor.h"
#include "us_extern.h"
#include "us_extinctfitter_gui.h"
#include "us_extinction_gui.h"
#include "us_help.h"
#include "us_math2.h"
#include "us_minimize.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

// #include "us_new_spectrum.h"

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

class US_Extinction;

//! A class to bring up a dialog that edits an analyte sequence
class US_GUI_EXTERN US_SequenceEditor : public US_WidgetsDialog {
  Q_OBJECT
 public:
  //! Constructor
  //! \param sequence - The sequence to edit
  US_SequenceEditor(const QString&);

 signals:
  //! A signal to indicate that sequence editing is done and
  //! the change was accepted
  //! \param new_sequence - The new sequence retruned with the signal
  void sequenceChanged(QString new_sequence);

 private:
  US_Editor* edit;

 private slots:
  void accept(void);
};

//! \class US_AnalyteMgrSelect
//!      This class provides a tabbed entry for analyte selection
class US_AnalyteMgrSelect : public US_Widgets {
  Q_OBJECT

 public:
  //! brief Analyte Selection Tab. To
  //! instantiate the class a calling function must
  //! provide the ID of the investigator.

  //! \param invID          A pointer to the current investigator ID
  //! \param select_db_disk Indicates whether the default search
  //!    is on the local disk or in the DB
  //! \param tmp_analyte    Pointer for selected analyte
  US_AnalyteMgrSelect(int*, int*, US_Analyte*);

  US_Analyte* analyte;
  US_Analyte* tmp_analyte;
  US_Analyte orig_analyte;

  QList<US_Analyte> ms_analytes;  // Multi-select analytes

  int* personID;
  int* db_or_disk;
  bool from_db;

 signals:
  //! Currently selected analyte is accepted by User
  void analyteAccepted(void);
  void selectionCanceled(void);
  void analytesAccepted(QList<US_Analyte>&);

 private:
  int dbg_level;

  QHBoxLayout* radios;
  QButtonGroup* typeButtons;
  QGridLayout* gfbox;

  QLabel* bn_select;
  QLabel* bn_calcmw;
  QLabel* bn_ratnuc;

  QLabel* lb_search;
  QLabel* lb_molecwt;
  QLabel* lb_vbar20;
  QLabel* lb_residue;
  QLabel* lb_e280;
  QLabel* lb_strand;
  QLabel* lb_3prime;
  QLabel* lb_5prime;
  QLabel* lb_sodium;
  QLabel* lb_potassium;
  QLabel* lb_lithium;
  QLabel* lb_magnesium;
  QLabel* lb_calcium;

  QLineEdit* le_search;
  QLineEdit* le_molecwt;
  QLineEdit* le_vbar20;
  QLineEdit* le_residue;
  QLineEdit* le_e280;
  QLineEdit* le_strand;
  QLineEdit* le_3prime;
  QLineEdit* le_5prime;
  QLineEdit* le_sodium;
  QLineEdit* le_potassium;
  QLineEdit* le_lithium;
  QLineEdit* le_magnesium;
  QLineEdit* le_calcium;

  QPushButton* pb_cancel;
  QPushButton* pb_accept;
  QPushButton* pb_spectrum;
  QPushButton* pb_sequence;
  QPushButton* pb_delete;
  QPushButton* pb_info;
  QPushButton* pb_help;

  QListWidget* lw_analyte_list;
  QTextEdit* te_analyte_smry;

  QRadioButton* rb_protein;
  QRadioButton* rb_dna;
  QRadioButton* rb_rna;
  QRadioButton* rb_carbo;
  // QRadioButton* rb_3_hydroxyl;
  // QRadioButton* rb_3_phosphate;
  // QRadioButton* rb_5_hydroxyl;
  // QRadioButton* rb_5_phosphate;

  QCheckBox* ck_gradform;

  US_Help showHelp;

  // For list widget
  class AnalyteInfo {
   public:
    QString description;
    QString analyteID;
    QString guid;
    int index;
  };

  QList<AnalyteInfo> analyte_metadata;
  QStringList filenames;
  QStringList descriptions;
  QStringList GUIDs;
  QStringList analyteIDs;

 private slots:

  void query(void);
  void spectrum(void);
  void delete_analyte(void);
  void delete_disk(void);
  void delete_db(void);
  bool analyte_in_use(QString&);
  void info_analyte(void);
  void accept_analyte(void);
  void select_analyte(QListWidgetItem*);
  void select_analyte();
  void read_from_disk(QListWidgetItem*);
  void read_from_db(QListWidgetItem*);
  void read_from_db(const QString&);
  void search(const QString& = QString());
  //      void show_component  ( const QString&, double );
  void reject(void);
  void accept(void);
  void read_analyte(void);
  void read_db(void);
  void connect_error(const QString&);
  bool analyte_path(QString&);
  void reset(void);
  void set_analyte_type(int);
  QString analyte_info(US_Analyte*);
  QString analyte_smry(US_Analyte*);
  void sequence(void);

  void help(void) { showHelp.show_help("analyte_select.html"); };

 public slots:
  void init_analyte(void);
};

//! \class US_AnalyteMgrNew
//!      This class provides a tabbed entry for new analyte creation
class US_AnalyteMgrNew : public US_Widgets {
  Q_OBJECT

 public:
  /*! brief Tab for entering a new Analyte. To
     instantiate the class a calling function must
     provide the ID of the investigator.

     \param invID          The current investigator ID
     \param select_db_disk Indicates whether the default search
                             is on the local disk or in the DB
     \param tmp_analyte    Pointer to a US_Analyte object holding the active
                             analyte (for editing and adding new analytes)
     \param temperature    The average analyte temperature
     \param signal         Flag if signal is to be output
  */
  US_AnalyteMgrNew(int*, int*, US_Analyte*, double, bool);
  // US_Extinction *w;

  // QLineEdit*    le_protein_e280;

 signals:
  void newAnaAccepted(void);  //! New analyte accepted
  void newAnaCanceled(void);

 private:
  int* personID;
  int* db_or_disk;
  bool from_db;
  int dbg_level;

  int anatype;
  bool signal_tmp;

  bool inReset;

  uint A;
  uint C;
  uint T;
  uint G;
  uint U;

  US_Analyte* analyte;
  US_Analyte orig_analyte;

  QPushButton* pb_sequence;
  QPushButton* pb_spectrum;
  QPushButton* pb_accept;
  QPushButton* pb_reset;
  QPushButton* pb_cancel;

  QLabel* bn_calcmw;
  QLabel* bn_ratnuc;

  QLabel* lb_bselect;
  QLabel* lb_descrip;
  QLabel* lb_molecwt;
  QLabel* lb_vbar20;
  QLabel* lb_residue;
  QLabel* lb_e280;

  QLineEdit* le_concen;
  QLineEdit* le_density;
  QLineEdit* le_viscos;
  QLineEdit* le_ph;
  QLineEdit* le_compress;
  QLineEdit* le_descrip;
  QLineEdit* le_molecwt;
  QLineEdit* le_vbar20;
  QLineEdit* le_residue;
  QLineEdit* le_e280;

  QLineEdit* le_protein_temp;
  QLineEdit* le_protein_mw;
  QLineEdit* le_protein_vbar20;
  QLineEdit* le_protein_vbar;
  QLineEdit* le_protein_residues;
  QLineEdit* le_protein_e280;

  QLineEdit* le_nucle_mw;
  QLineEdit* le_nucle_vbar;
  QLineEdit* le_carbs_mw;
  QLineEdit* le_carbs_vbar;

  QWidget* protein_widget;
  QWidget* dna_widget;
  QWidget* carbs_widget;

  QCheckBox* ck_stranded;
  QCheckBox* ck_mw_only;
  QCheckBox* ck_grad_form;
  QRadioButton* rb_3_hydroxyl;
  QRadioButton* rb_3_phosphate;
  QRadioButton* rb_5_hydroxyl;
  QRadioButton* rb_5_phosphate;

  QwtCounter* ct_sodium;
  QwtCounter* ct_potassium;
  QwtCounter* ct_lithium;
  QwtCounter* ct_magnesium;
  QwtCounter* ct_calcium;

  QCheckBox* ck_manual;

  QRadioButton* rb_protein;
  QRadioButton* rb_dna;
  QRadioButton* rb_rna;
  QRadioButton* rb_carbo;

  // QListWidget*  lw_allcomps;
  // QListWidget*  lw_bufcomps;

  US_Help showHelp;

  void parse_dna(void);
  bool data_ok(void);
 private slots:

  void new_description();
  void add_component();
  void select_bcomp();
  void remove_bcomp(QListWidgetItem*);
  void recalc_density(void);
  void recalc_viscosity(void);
  void ph(void);
  void compressibility(void);
  void density(void);
  void viscosity(void);
  void manual_flag(bool);
  void spectrum(void);
  void newAccepted(void);
  void newCanceled(void);
  void write_db(void);
  void write_disk(void);
  void help(void) { showHelp.show_help("analyte_new.html"); };

  void manage_sequence(void);
  // void manage_spectrum ( void );
  void spectrum_class(void);

  void update_sequence(QString);

  void value_changed(const QString&);
  void value_changed_e280(const QString&);
  void temp_changed(const QString&);

  void update_stranded(bool);
  void update_mw_only(bool);
  void update_nucleotide(bool);
  void update_nucleotide(double);
  void update_nucleotide(void);

  void set_analyte_type(int);
  void reset(void);
  void verify_vbar(void);

  void change_prot_e280(void);

 public slots:
  void init_analyte(void);
};

//! \class US_AnalyteMgrEdit
//!      This class provides a tabbed entry for non-hydrodynamic analyte mods
class US_AnalyteMgrEdit : public US_Widgets {
  Q_OBJECT

 public:
  /*! brief Tab for entering a new Analyte. To
     instantiate the class a calling function must
     provide the ID of the investigator.

     \param invID          The current investigator ID
     \param select_db_disk Indicates whether the default search
                             is on the local disk or in the DB
     \param tmp_analyte    Pointer to a US_Analyte object holding the active
                             analyte (for editing and adding new analytes)
  */
  US_AnalyteMgrEdit(int*, int*, US_Analyte*);
  QString edit_analyte_description;
  // US_Extinction *w;

 signals:
  void editAnaAccepted(void);  //! Edited analyte accepted
  void editAnaCanceled(void);

 private:
  int* personID;
  int* db_or_disk;
  bool from_db;
  int dbg_level;

  US_Analyte* analyte;
  US_Analyte orig_analyte;

  QPushButton* pb_accept;
  QLineEdit* le_descrip;
  QLineEdit* le_bguid;
  QLineEdit* le_ph;

  US_Help showHelp;

 private slots:

  void spectrum_class(void);
  void editAccepted(void);
  void editCanceled(void);
  void write_db(void);
  void write_disk(void);

  void help(void) { showHelp.show_help("analyte_edit.html"); };
  void change_spectrum(void);
  void accept_enable(void);
  void description(void);

 public slots:
  void init_analyte(void);
};

//! \class US_AnalyteMgrSettings
//!      This class provides a tabbed entry for general analyte settings
class US_AnalyteMgrSettings : public US_Widgets {
  Q_OBJECT

 public:
  //! \brief Selection tab for changing investigator and
  //!        choosing between db/disk access, as well as
  //!        for synchronizing the analyte components local
  //!        file with the database.
  //!
  //! \param invID          A pointer to the current investigator ID
  //! \param select_db_disk A pointer to a flag that indicates whether
  //!                       the default search is on the local disk or
  //!                       in the DB
  US_AnalyteMgrSettings(int*, int*);

 signals:
  //! A signal to indicate that the current disk/db selection has changed.
  //! /param DB True if DB is the new selection
  void use_db(bool DB);
  //! A signal to indicate that the current investigator was changed.
  //! /param invID is the new selection
  void investigator_changed(int invID);

 private:
  int* personID;
  int* db_or_disk;
  int dbg_level;
  bool from_db;
  bool signal_wanted;

  QLineEdit* le_investigator;

  US_Help showHelp;

  US_Disk_DB_Controls* disk_controls;

 private slots:
  void sel_investigator(void);
  void assign_investigator(int);
  void db_changed(bool);

  void help(void) { showHelp.show_help("analytes.html"); };
};

class US_GUI_EXTERN US_AnalyteGui : public US_WidgetsDialog {
  Q_OBJECT

 public:
  //! Main constructor
  //! \param signal  A flag to specify if one of the signals
  //!                should be emitted when terminating the dialog
  //! \param GUID    The default analyte
  //! \param accessf An indicator of whether to search the disk
  //!                or DB for the default analyte
  //! \param temper  The average analyte temperature
  US_AnalyteGui(bool = false, const QString& = QString(),
                int = US_Disk_DB_Controls::Default, double = NORMAL_TEMP);

 signals:
  //! A signal to indicate that the analyte density and viscosity
  //!  has been updated and the screen is closing.
  //! \param density   - new density of the analyte
  //! \param viscosity - new viscosity of the analyte
  void valueChanged(double density, double viscosity);

  //! A signal to indicate that the analyte data
  //!  has been updated and the screen is closing.
  //! \param analyte   - the updated analyte data.
  void valueChanged(US_Analyte analyte);

  //! A signal to indicate that the current disk/db selection has changed.
  //! Return the ID of the analyte in the current database.  A
  //! value of -1 indicates the data was manually input or was
  //! returned from the local disk.
  //! \param analyteID - A string value of the returned ID
  void valueAnalyteID(const QString analyteID);

  //! A signal to indicate that the current disk/db selection has changed.
  //! /param DB      - Disk/Db flag: True if DB is the new selection.
  void use_db(bool DB);

 private:
  bool signal;
  QString guid;
  double temperature;

  int disk_or_db;
  int personID;
  int dbg_level;
  bool from_db;
  bool analyteCurrent;
  bool manualUpdate;
  bool view_shared;
  bool access;

  QTabWidget* tabWidget;
  US_AnalyteMgrSelect* selectTab;
  US_AnalyteMgrNew* newTab;
  US_AnalyteMgrEdit* editTab;
  US_AnalyteMgrSettings* settingsTab;

  //!< The currently active analyte Data.
  US_Analyte analyte;
  US_Analyte orig_analyte;  // saves original analyte upon entry,
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
  void editAnaAccepted(void);
  void editAnaCanceled(void);
  void newAnaAccepted(void);
  void newAnaCanceled(void);
  void analyteAccepted(void);
  void analyteRejected(void);
  void analytesAccepted(QList<US_Analyte>&);
};
#endif
