//! \file us_experiment.h
#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_selectbox.h"
#include "us_project.h"
#include "us_rotor.h"

/*! \class US_Experiment
           This class provides a data structure and associated routines to 
           represent the relevant experiment parameters, such as the lab, 
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_Experiment
{
   public:

      struct CalibrationInfo
      {
         int              calibrationID;      //!< The ID of the rotor calibration
         double           rotorCoeff1;        //!< The first rotor stretch calibration coefficient
         double           rotorCoeff2;        //!< The second rotor stretch coefficient
      };

      struct RotorInfo
      {
         int              rotorID;            //!< The rotor that was used
         QString          rotorGUID;          //!< The GUID of the rotor
         QString          rotorSerial;        //!< The serial number of the rotor
         QList< CalibrationInfo > calibrationInfo; //!< All available calibration profiles for this rotor
      };

      int              invID;              //!< The personID of the investigator
      QString          invGUID;            //!< The GUID of the investigator
      
      QString          name;               //!< The name of the investigator
      int              expID;              //!< The ID of the experiment itself
      QString          expGUID;            //!< The GUID of the experiment
      US_Project       project;            //!< The project this experiment is associated with
      QString          runID;              //!< The run ID
      int              labID;              //!< The lab in which the experiment was conducted
      int              instrumentID;       //!< The identifier of the ultra-centrifuge
      QString          instrumentSerial;   //!< The serial number of the instrument
      int              operatorID;         //!< The personID of the person who operated the centrifuge
      QString          operatorGUID;       //!< The GUID of the operator
      int              rotorID;            //!< The rotor that was used
      QString          rotorGUID;          //!< The GUID of the rotor
      QString          rotorSerial;        //!< The serial number of the rotor
      QString          rotorName;          //!< The name of the rotor
      int              calibrationID;      //!< The ID of the rotor calibration
      double           rotorCoeff1;        //!< The first rotor stretch calibration coefficient
      double           rotorCoeff2;        //!< The second rotor stretch coefficient
      QDate            rotorUpdated;       //!< The date of the calibration
      QString          expType;            //!< The type of experiment
      QByteArray       opticalSystem;      //!< The type of optical system used
      QList< double >  rpms;               //!< A list of rotor speeds observed during the experiment
      QString          runTemp;            //!< The run temperature
      QString          label;              //!< The experiment label, or identifying information
      QString          comments;           //!< Comments that were associated with the experiment
      QString          centrifugeProtocol; //!< The governing centrifuge protocol
      QString          date;               //!< The date the record was entered or last updated
      bool             syncOK;             //!< The user has connected with the db
      QStringList      experimentTypes;    //!< A list of possible experiment types

      /*! \brief Generic constructor for the US_Experiment class.
      */
      US_Experiment( void );

      /*! \brief    Determine if the current experiment runID exists in the DB. 
                    Updates expID accordingly, or to 0 if not found.

          \param    db For database access, an open database connection
      */
      int checkRunID( US_DB2* = 0 );

      /*! \brief    Function to save the experiment information to db

          \param    update Is it ok to update an existing database runID 
                           (maybe the user is updating a DB record)?
          \param    db For database access, an open database connection
      */
      int saveToDB           ( bool = false, US_DB2* = 0 );

      /*! \brief    Reads secondary experiment info from the database. Call 
                    this function when you already have IDs stored and
                    want to fill out with GUIDs, serial numbers and the like.
                    For instance, after loading the xml file to fill in the gaps,
                    or to load experiment info after reading
                    auc files.

          \param    db For database access, an open database connection
      */
      int readSecondaryInfoDB( US_DB2* = 0 );

      void clear( void );                  //!< Function to reset all class variables to defaults
      void show ( void );                  // Temporary function to display current exp info

      RotorInfo         hwInfo;

};
#endif
