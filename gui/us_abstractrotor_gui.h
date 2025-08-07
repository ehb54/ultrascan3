//! \file us_abstractrotor_gui.h
#ifndef US_ABSTRACTROTOR_GUI_H
#define US_ABSTRACTROTOR_GUI_H

#include "us_editor.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_investigator.h"
#include "us_rotor.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"


/*! \class US_AbstractRotorGui
   This class provides an interface to the Rotor tables of USLIMS and
   allows the user to add, delete, vide and modify rotors in the database.
   This class is also used to pick rotors from the database for various
   Applications that need rotors.
*/
class US_GUI_EXTERN US_AbstractRotorGui : public US_WidgetsDialog {
      Q_OBJECT

   public:
      /*! \brief Generic constructor for the US_AbstractRotorGui class used to add a new rotor.

          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
          \param dataIn The Rotor structure that will be added to the database
      */
      US_AbstractRotorGui(bool = false, int = US_Disk_DB_Controls::Default, US_Rotor::Rotor * = new US_Rotor::Rotor());

   signals:

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db(bool DB);

   private:
      US_Rotor::Rotor *currentRotor;
      bool signal;

      QPushButton *pb_help;
      QPushButton *pb_reset;
      QPushButton *pb_accept;
      QPushButton *pb_close;

      QRadioButton *rb_db;
      QRadioButton *rb_disk;

      QLineEdit *le_name;
      QLineEdit *le_serialNumber;

      QComboBox *cb_rotors;

      QTextEdit *te_details;

      US_Disk_DB_Controls *disk_controls; //!< Radiobuttons for disk/db choice

      QVector<US_Rotor::AbstractRotor> abstractRotorList;
      int currentARIndex; // current index into abstractRotorList

      US_Help showHelp;

   private slots:

      void setupGui(int);
      void reset(void);
      void source_changed(bool);
      bool loadAbstractRotors(void);
      void showDetails(int);
      int getIndex(void);
      void select(void);
      void connect_error(const QString &);
      void help(void) { showHelp.show_help("manual/abstractrotor.html"); };
};
#endif
