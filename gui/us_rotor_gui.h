//! \file us_rotor_gui.h
#ifndef US_ROTOR_GUI_H
#define US_ROTOR_GUI_H

#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_db2.h"
#include "us_rotor.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_editor_gui.h"
#include "us_investigator.h"
#include "us_abstractrotor_gui.h"
#include "us_simparms.h"

/*! \class US_RotorGui
   This class provides an interface to the Rotor tables of USLIMS and
   allows the user to add, delete, vide and modify rotors in the database.
   This class is also used to pick rotors from the database for various
   Applications that need rotors.
*/

class US_GUI_EXTERN US_RotorGui : public US_WidgetsDialog
{

   Q_OBJECT
         
   public:

      /*! \brief Overloaded constructor for the US_RotorGui class. This one is
                 used when passing rotor and rotor calibration information
                 into the class.
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
          \param rotorIn A reference to a structure that contains the currently
                        selected rotor information
          \param calibrationIn A reference to a structure that contains the currently
                        selected calibration information
      */
      US_RotorGui( bool = false, 
                   int  = US_Disk_DB_Controls::Default,
                   const US_Rotor::Rotor&            = US_Rotor::Rotor(),
                   const US_Rotor::RotorCalibration& = US_Rotor::RotorCalibration() );

      /*! \brief Overloaded constructor for the US_RotorGui class. In this case,
                 it expects to be called from US_RotorCalibration
          \param calibration The calibration structure passed in from the calibration program
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

      bool load_rotor( QString& load_init, double& coeff1, double& coeff2);

      US_Rotor::Status            rotorStatus;            //!< Most recent rotor status
      US_Rotor::Status            calibrationStatus;      //!< Most recent calibration status
      US_Rotor::Rotor             currentRotor;           //!< Current rotor structure
      US_Rotor::RotorCalibration  currentCalibration;     //!< Current calibration structure

   signals:
      
      /*! \brief    Signal to pass the accepted rotor calibration to the calling program
          \param    Rotor the rotor structure selected by the user
          \param    RotorCalibration the rotor calibration structure selected by the user
      */
      void RotorCalibrationSelected ( US_Rotor::Rotor& Rotor, 
                                      US_Rotor::RotorCalibration& RotorCalibration );
   
      /*! \brief    Signal to pass to the calling program if the operation was canceled
      */
      void RotorCalibrationCanceled ( void );
   
      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      int                labID;
      US_Help            showHelp;
      QString            load_data;

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

      QLineEdit*         le_calibrationLabel;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QComboBox*         cb_lab;
      
      bool               signal;            //!< True if the calling program wants a signal
      bool               savingCalibration; //!< True if we're saving the calibration profile

      void setupGui      ( int );

   private slots:
      US_Rotor::Status readCalibration( int, int );
      US_Rotor::Status readRotor      ( int, int );

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
      void updateLabel        ( const QString & );
      void updateOmega2t      ( const QString & );
      void saveCalibration    ( void );
      void replaceDummyCalibration( void );

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
