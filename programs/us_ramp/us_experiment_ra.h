//! \file us_experiment_ra.h
#ifndef US_EXPERIMENT_H
#define US_EXPERIMENT_H

#include "us_extern.h"
#include "us_project.h"
#include "us_ramp.h"
#include "us_rotor.h"

/*! \class US_Experiment
           This class provides a data structure and associated routines to 
           represent the relevant experiment parameters, such as the lab, 
           instrument, and rotor on which the experiment was run. 
*/

class US_ExperimentRa {
   public:
      struct CalibrationInfo {
            int calibrationID; //!< The ID of the rotor calibration
            double rotorCoeff1; //!< The first rotor stretch calibration coefficient
            double rotorCoeff2; //!< The second rotor stretch coefficient
      };

      struct RotorInfo {
            int rotorID; //!< The rotor that was used
            QString rotorGUID; //!< The GUID of the rotor
            QString rotorSerial; //!< The serial number of the rotor
            QList<CalibrationInfo> calibrationInfo; //!< All available calibration profiles for this rotor
      };

      int invID; //!< The personID of the investigator
      QString invGUID; //!< The GUID of the investigator

      QString name; //!< The name of the investigator
      int expID; //!< The ID of the experiment itself
      QString expGUID; //!< The GUID of the experiment
      US_Project project; //!< The project this experiment is associated with
      QString runID; //!< The run ID
      int labID; //!< The lab in which the experiment was conducted
      int instrumentID; //!< The identifier of the ultra-centrifuge
      QString instrumentSerial; //!< The serial number of the instrument
      int operatorID; //!< The personID of the person who operated the centrifuge
      QString operatorGUID; //!< The GUID of the operator
      int rotorID; //!< The rotor that was used
      QString rotorGUID; //!< The GUID of the rotor
      QString rotorSerial; //!< The serial number of the rotor
      QString rotorName; //!< The name of the rotor
      int calibrationID; //!< The ID of the rotor calibration
      double rotorCoeff1; //!< The first rotor stretch calibration coefficient
      double rotorCoeff2; //!< The second rotor stretch coefficient
      QDate rotorUpdated; //!< The date of the calibration
      QString expType; //!< The type of experiment
      QByteArray opticalSystem; //!< The type of optical system used
      //       QList< double >  rpms;               //!< A list of rotor speeds observed during the experiment
      QString runTemp; //!< The run temperature
      QString label; //!< The experiment label, or identifying information
      QString comments; //!< Comments that were associated with the experiment
      QString centrifugeProtocol; //!< The governing centrifuge protocol
      QString date; //!< The date the record was entered or last updated
      bool syncOK; //!< The user has connected with the db
      QStringList experimentTypes; //!< A list of possible experiment types
      //       QVector< double > RIProfile;         //!< If RI data, the intensity profile
      //       QVector< int >    RIwvlns;           //!< For RI+MWL, wavelengths present
      //       int               RI_nscans;         //!< RI+MWL scans per profile
      //       int               RI_nwvlns;         //!< RI+MWL wavelengths (profiles count)

      /*! \brief Generic constructor for the US_ExperimentRa class.
      */
      US_ExperimentRa(void);

      /*! \brief    Determine if the current experiment runID exists in the DB. 
                    Updates expID accordingly, or to 0 if not found.

          \param    db For database access, an open database connection
      */
      int checkRunID(US_DB2 * = 0);

      /*! \brief    Function to save the experiment information to db
          \param    update Is it ok to update an existing database runID 
                           (maybe the user is updating a DB record)?
          \param    db For database access, an open database connection
      */
      int saveToDB(bool = false, US_DB2 * = 0);

      /*! \brief    Reads experiment information from the db
          \param runID  The run ID of the experiment.
          \param    db For database access, an open database connection
          \returns  One of the US_DB2 error codes
      */
      int readFromDB(QString, US_DB2 * = 0);

      /*! \brief    Writes an xml file
          \param data    A reference to a RampRawData vector.
          \param triples A reference to a structure provided by the calling
                        function that already contains all the different
                        cell/channel/wavelength combinations in the data. 
          \param runType A reference to a variable that already contains the
                        type of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
      */
      int saveToDisk(QVector<US_mwlRamp::RampRawData *> &, QList<US_Ramp::TripleInfo> &, QString, QString, QString);

      /*! \brief    Reads an xml file

          \param triples A reference to a structure provided by the calling
                        function that will contain all the different
                        cell/channel/wavelength defined by the xml file.
          \param runType A reference to a variable that will contain the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        stored.
          \param runID  The run ID of the experiment.
          \param dirname The directory from which the files are read.
      */
      int readFromDisk(QList<US_Ramp::TripleInfo> &, QString, QString, QString);


      void clear(void); //!< Function to reset all class variables to defaults
      void show(void); // Temporary function to display current exp info

      RotorInfo hwInfo;

   private:
      void readExperiment(QXmlStreamReader &, QList<US_Ramp::TripleInfo> &, QString, QString);

      void readDataset(QXmlStreamReader &, US_Ramp::TripleInfo &);
};
#endif
