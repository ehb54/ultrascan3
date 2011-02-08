//! \file us_rotor_gui.h
#ifndef US_ROTOR_GUI_H
#define US_ROTOR_GUI_H

#include <QtGui>
#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_db2.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_investigator.h"
#include "us_abstractrotor_gui.h"
#include "us_rotor.h"

/*! \class US_RotorGui
   This class provides an interface to the Rotor tables of USLIMS and
   allows the user to add, delete, vide and modify rotors in the database.
   This class is also used to pick rotors from the database for various
   Applications that need rotors.
*/

class US_EXTERN US_RotorGui : public US_WidgetsDialog
{

   Q_OBJECT
         
   public:

      /*! \brief Generic constructor for the US_RotorGui class.
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
      */
      US_RotorGui( bool = false, 
                   int  = US_Disk_DB_Controls::Default );

      /*! \brief Overloaded constructor for the US_RotorGui class. In this case,
                 it expects to be called from US_RotorCalibration
          \param RotorCalibration The calibration structure passed in from the calibration program
          \param new_calibration  A boolean value indicating whether the caller is
                                  US_RotorCalibration with new calibration data or not
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
      */
      US_RotorGui( US_Rotor::RotorCalibration&,
                   bool = false,
                   bool = false, 
                   int  = US_Disk_DB_Controls::Default );

      //! A destructor.
      ~US_RotorGui();


      US_Rotor::Status            rotorStatus;            //!< Most recent rotor status
      US_Rotor::Status            calibrationStatus;      //!< Most recent calibration status
      US_Rotor::Rotor             currentRotor;           //!< Current rotor structure
      US_Rotor::RotorCalibration  currentCalibration;     //!< Current calibration structure
      US_Help                     showHelp;
      int                         labID;

   signals:
      
      /*! \brief    Signal to pass the accepted rotor calibration to the calling program
          \param    RotorCalibration the rotor calibration structure selected by the user
      */
      void RotorCalibrationSelected ( US_Rotor::RotorCalibration );
   
      /*! \brief    Signal to pass to the calling program if the operation was canceled
      */
      void RotorCalibrationCanceled ( void );
   
      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:

      QPushButton*       pb_help;
      QPushButton*       pb_reset;
      QPushButton*       pb_accept;
      QPushButton*       pb_cancel;
      QPushButton*       pb_addRotor;
      QPushButton*       pb_deleteRotor;
      QPushButton*       pb_saveCalibration;
      QPushButton*       pb_deleteCalibration;
      QPushButton*       pb_viewReport;
                  
      QLineEdit*         le_name;
      QLineEdit*         le_serialNumber;
      QLineEdit*         le_coefficient1;
      QLineEdit*         le_coefficient2;
      QLineEdit*         le_omega2t;
                  
      QListWidget*       lw_rotors;
      QListWidget*       lw_calibrations;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QComboBox*         cb_lab;
      
      bool               signal;            //!< True if the calling program wants a signal
      bool               savingCalibration; //!< True if we're saving the calibration profile

      void setupGui      ( int );

//      void readRotorInfo  ( QXmlStreamReader& xml );
//      void readRotorCalibrationInfo  ( QXmlStreamReader& xml );
//      QString get_filename   ( const QString&, bool& );
      
   private slots:
      
      void source_changed     ( bool db );
      bool loadRotors         ( const int );
      void addRotor           ( void );
      void update_disk_db     ( bool db );
      void selectRotor        ( QListWidgetItem * );
      void deleteRotor        ( void );
      bool readCalibrationProfiles( int );
      void selectCalibration  ( QListWidgetItem * );
      void viewReport         ( void );
      void deleteCalibration  ( void );
      void updateName         ( const QString & );
      void updateSerialNumber ( const QString & );
      void saveCalibration    ( void );
      void help (void)
      {
         showHelp.show_help( "manual/rotor.html" );
      };
      void reset              ( void );
      void accept             ( void );
      void cancel             ( void );
      bool load               ( void );
      void changeLab          ( int  );
      void connect_error      ( const QString & );
      int  getIndex           ( void );
      void db_error           ( const QString & );
      void newRotor           ( void );
};

#endif
