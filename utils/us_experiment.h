//! \file us_experiment.h
#ifndef US_EXPERIMENT_CONVERT_H
#define US_EXPERIMENT_CONVERT_H

#include "us_extern.h"
#include "us_convert.h"
#include "us_project.h"
#include "us_simparms.h"
#include "us_rotor.h"

#ifndef SP_SPEEDPROFILE
#define SP_SPEEDPROFILE US_SimulationParameters::SpeedProfile
#endif

/*! \class US_Experiment
           This class provides a data structure and associated routines to 
           represent the relevant experiment parameters, such as the lab, 
           instrument, and rotor on which the experiment was run. 
*/
           
class US_Experiment
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
      QString          protocolGUID;       //!< The protocol GUID
      QString          date;               //!< The date the record was entered or last updated
      bool             syncOK;             //!< The user has connected with the db
      QStringList      experimentTypes;    //!< A list of possible experiment types
      QVector< double > RIProfile;         //!< If RI data, the intensity profile
      QVector< int >    RIwvlns;           //!< For RI+MWL, wavelengths present
      QMap< QString, double > RIProfileMap; //!< Referecne Profile, cell/chanell/wavelength/scan -> radial intensity
      int               RI_nscans;         //!< RI+MWL scans per profile
      int               RI_nwvlns;         //!< RI+MWL wavelengths (profiles count)

      /*! \brief Generic constructor for the US_Experiment class.
      */
      US_Experiment( void );

      /*! \brief    Determine if the current experiment runID exists in the DB. 
                    Updates expID accordingly, or to 0 if not found.

          \param    db For database access, an open database connection
      */
      int checkRunID( US_DB2* = 0 );
      int checkRunID_auto( int invID_passed, US_DB2* = 0 );

      /*! \brief    Function to save the experiment information to db

          \param    update Is it ok to update an existing database runID 
                           (maybe the user is updating a DB record)?
          \param    db For database access, an open database connection
          \param    speedsteps Reference to vector of experiment speed steps
      */
      int saveToDB( bool, US_DB2*, QVector< SP_SPEEDPROFILE >& );
      int saveToDB_auto( bool, US_DB2*, QVector< SP_SPEEDPROFILE >&, int );
      
      /*! \brief    Reads experiment information from the db

          \param runID  The run ID of the experiment.
          \param    db For database access, an open database connection
          \param    speedsteps Reference to vector of experiment speed steps
          \returns  One of the US_DB2 error codes
      */
      int readFromDB( QString, US_DB2*, QVector< SP_SPEEDPROFILE >& );

      /*! \brief    Writes an xml file

          \param triples A reference to a structure provided by the calling
                         function that already contains all the different
                         cell/channel/wavelength combinations in the data.
          \param runType A reference to a variable that already contains the
                         type of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                         This information will affect how the data is written.
          \param runID   The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
          \param speedsteps  Reference to a vector of speed steps for the
                             experiment.
      */
      int saveToDisk( QList< US_Convert::TripleInfo >&,
                      QString, QString, QString,
                      QVector< SP_SPEEDPROFILE >& );

      /*! \brief    Reads an xml file

          \param triples A reference to a structure provided by the calling
                         function that will contain all the different
                         cell/channel/wavelength defined by the xml file.
          \param runType A reference to a variable that will contain the type
                         of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                         This information will affect how the data is stored.
          \param runID   The run ID of the experiment.
          \param dirname The directory from which the files are read.
      */
      int readFromDisk( QList< US_Convert::TripleInfo >&,
                        QString, QString, QString );

      /*! \brief    Writes the radial intensity profile data to the HD

          \param    runID   The run ID associated with the RI data
          \param    dirname The location where the RI Profile is to go.
      */
      int saveRIDisk( QString , QString );

      /*! \brief    Reads radial intensity profile data from the HD

          \param    runID   The run ID associated with the RI data
          \param    dirname The location where the RI Profile is.
      */
      int readRIDisk( QString , QString );

      /*! \brief    Static function to delete any pcsa_modelrecs for the run
          \param    db      Database connection pointer or NULL for local
          \param    invID   Investigator
          \param    runID   Run ID for which to delete records
          \returns          Flag if delete was OK;
      */
      static bool deleteRunPcsaMrecs( US_DB2*, const QString, const QString );

      void clear( void ); //!< Function to reset all class variables to defaults
      void show ( void ); //!< Temporary function to display current exp info

      RotorInfo         hwInfo;

   private:
      void readExperiment( QXmlStreamReader&, 
                 QList< US_Convert::TripleInfo >&, QString , QString );

      void readDataset( QXmlStreamReader&, US_Convert::TripleInfo& );

      void createRIXml( QByteArray& );

      int  importRIxml( QByteArray& );
};
#endif
