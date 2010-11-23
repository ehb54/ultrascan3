//! \file us_abstractrotor_gui.h
#ifndef US_ABSTRACTROTOR_GUI_H
#define US_ABSTRACTROTOR_GUI_H

#include <uuid/uuid.h>

#include <QtGui>
//#include <QApplication>
//#include <QDomDocument>

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
   double magnetOffset;
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


/*! \class US_AbstractRotorGui
   This class provides an interface to the Rotor tables of USLIMS and
   allows the user to add, delete, vide and modify rotors in the database.
   This class is also used to pick rotors from the database for various
   Applications that need rotors.
*/



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
