//! \file us_rotor_gui.h
#ifndef US_ROTOR_GUI_H
#define US_ROTOR_GUI_H

#include <uuid/uuid.h>

#include <QtGui>
#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_db2.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_investigator.h"

//! \brief the abstractRotor structure describes a generic 4- or 8-hole rotor
struct abstractRotor
{
   int ID;
   QString GUID;
   QString name;
   QString material;
   int numHoles;
   int maxRPM;
   double cellCenter;
   QString manufacturer;
};

//! \brief the rotor structure describes a derived 4- or 8-hole rotor
struct rotor
{
   int ID;
   int abstractRotorID;
   QString abstractRotorGUID;
   int labID;                 //!< The ID of the laboratory which owns this rotor
   QString GUID;
   QString name;
   QString serialNumber;
};

//! \brief the rotorCalbration structure describes a calibration experiment and associated data
struct rotorCalibration
{
   int ID;
   QString GUID;
   QString rotorGUID;   //!< The GUID of the rotor this calibration is associated with
   QString calibrationExperimentGUID; //!< The GUID of the experiment that contains the calibration data
   double coeff1;       //!< The first order coefficient for the second order polynomial
   double coeff2;       //!< The second order coefficient for the second order polynomial
                        //!< the zeroth order coefficient is always zero (no stretch at 0 RPM)
   QString report;      //!< a calibration report 
   QDate lastUpdated;   //!< the date on which the rotor was calibrated
   double omega2t;      //!< the cumulative force exerted on the rotor
};


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

      //! \brief Generic constructor for the US_RotorGui class.
      US_RotorGui();
      /*! \brief Generic constructor for the US_RotorGui class.
          \param rotorCalibration The calibration structure passed in from the calibration program
      */
      US_RotorGui( rotorCalibration );

      
      //! A destructor.
      ~US_RotorGui();


      //! \brief   Some status codes to keep track of where solution data has been saved to
      enum Status
      {
         NOT_SAVED,         //!< The file has not been saved
         EDITING,           //!< Data is being edited; certain operations not permitted
         HD_ONLY,           //!< The file has been saved to the HD
         DB_ONLY,           //!< The file has been saved to the DB
         BOTH               //!< The file has been saved to both HD and DB
      };

      Status            rotorStatus;            //!< Most recent rotor status
      Status            calibrationStatus;      //!< Most recent calibration status
      abstractRotor     currentAbstractRotor;   //!< Current abstractRotor structure
      rotor             currentRotor;           //!< Current rotor structure
      rotorCalibration  currentCalibration;     //!< Current calibration structure
      US_Help           showHelp;
      int               labID;

      /*! \brief Functions to read an entire abstractrotor structure from the disk

          \param    guid The GUID of the rotor to look for
      */
//      void readAbstractRotorFromDisk( QString& );
//      void readRotorFromDisk( QString& );
//      void readCalibrationFromDisk( QString& );

      /*! \brief    Function to read an entire rotor structure from the DB
          \param    ID The database ID of the desired abstractRotor/rotor/calibration
          \param    db For database access, an open database connection
      */
//      abstractRotor     readAbstractRotorFromDB    ( int, US_DB2* = 0 );
//      rotor             readRotorFromDB            ( int, US_DB2* = 0 );
//      rotorCalibration  readCalibrationFromDB      ( int, US_DB2* = 0 );

      //! \brief    Quick method to zero out the current structures for rotor/abstractRotor/calibration
//      void clearRotor             ( void );
//      void clearAbstractRotor     ( void );
//      void clearCalibration       ( void );

      //! \brief    Method to save the current rotor/calibration to disk
//      void saveRotorToDisk        ( void );
//      void saveCalibrationToDisk  ( void );

      /*! \brief    Function to save the current calibration/rotor information to db

          \param    db For database access, an open database connection
      */
//      void saveRotorToDB          ( US_DB2* = 0 );
//      void saveCalibrationToDB    ( US_DB2* = 0 );

      /*! \brief    Function to find the filename of a solution on disk, if it exists.
                    Returns true if successful, false otherwise
          \param    guid The GUID of the solution to look for on disk
          \param    filename The function will return the filename here if it is found
      */
//      bool diskFilename      ( const QString& , QString& );


   signals:
      
      /*! \brief    Signal to pass the accepted rotor calibration to the calling program
          \param    rotorCalibration the rotor calibration structure selected by the user
      */
//      void rotorCalibrationSelected ( rotorCalibration );
   

   private:

      QPushButton       *pb_help;
      QPushButton       *pb_reset;
      QPushButton       *pb_accept;
      QPushButton       *pb_close;
      QPushButton       *pb_addRotor;
      QPushButton       *pb_deleteRotor;
      QPushButton       *pb_saveCalibration;
      QPushButton       *pb_deleteCalibration;
      QPushButton       *pb_viewReport;

      QLineEdit         *le_name;
      QLineEdit         *le_serialNumber;
      QLineEdit         *le_coefficients;
      QLineEdit         *le_date;
      QLineEdit         *le_force;

      QListWidget       *lw_rotors;
      QListWidget       *lw_calibrations;

      QRadioButton      *rb_db;
      QRadioButton      *rb_disk;

      QComboBox         *cb_lab;
      
      void setupGui              ( void );
      void resetAbstractRotor    ( void );
      void resetRotor            ( void );
      void resetRotorCalibration ( void );

//      void readRotorInfo  ( QXmlStreamReader& xml );
//      void readRotorCalibrationInfo  ( QXmlStreamReader& xml );
//      QString get_filename   ( const QString&, bool& );
      
   private slots:
      
      void check_db           ( void );
      void check_disk         ( void );
      bool loadRotors         ( const int & );
      void addRotor           ( void );
      void deleteRotor        ( void );
      void updateName         ( const QString & );
      void updateSerialNumber ( const QString & );
      void saveCalibration    ( void );
      void deleteCalibration  ( void );
      void viewReport         ( void );
      void selectRotor        ( QListWidgetItem * );
      void selectCalibration  ( QListWidgetItem * );
      void help (void)
      {
         showHelp.show_help( "manual/rotor.html" );
      };
      void reset              ( void );
      void accept             ( void );
      bool loadLabs           ( void );
      void changeLab          ( int );
      void connect_error      ( const QString & );
      int  getIndex           ( const QString & );
      void db_error           ( const QString & );
      void newRotor           ( void );
};

class US_EXTERN US_AbstractRotorGui : public US_WidgetsDialog
{

   Q_OBJECT
         
   public:

      //! \brief Generic constructor for the US_AbstractRotorGui class used to add a new rotor.
      
      US_AbstractRotorGui(abstractRotor *, rotor *);
      /*! 
          \param abstractRotor The abstractRotor structure from which the rotor will be derived
          \param rotor The Rotor structure that will be added to the database
      */
      int rotorID;

      abstractRotor     *currentAbstractRotor;
      rotor             *currentRotor;
      
      QPushButton       *pb_help;
      QPushButton       *pb_reset;
      QPushButton       *pb_accept;
      QPushButton       *pb_close;
      
      QRadioButton      *rb_db;
      QRadioButton      *rb_disk;

      QLineEdit         *le_name;
      QLineEdit         *le_serialNumber;
      
      QComboBox         *cb_rotors;
      
      QTextEdit         *te_details;

      QVector <abstractRotor> abstractRotorList;

      US_Help           showHelp;
      
      //! A destructor.
      ~US_AbstractRotorGui();

   private slots:

      void setupGui           ( void );
      void select             ( void );
      void reset              ( void );
      void check_db           ( void );
      void check_disk         ( void );
      bool loadAbstractRotors ( void );
      void changeRotor        ( int  );
      int  getIndex           ( const QString & );
      void connect_error      ( const QString & );
      void help (void)
      {
         showHelp.show_help( "manual/abstractrotor.html" );
      };
      void setAbstractRotorInfo ( int id );
      void updateName           ( const QString & );
      void updateSerialNumber   ( const QString & );
};
#endif
