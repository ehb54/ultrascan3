//! \file us_rotor.h
#ifndef US_ROTOR_H
#define US_ROTOR_H

#include "us_db2.h"
#include "us_extern.h"

/*! \class US_Rotor
   This class provides a low-level interface to the Rotor tables, allowing
   the user to read and write them from db or disk.
*/

class US_UTIL_EXTERN US_Rotor {
   public:
      //! \brief   Some status codes to keep track of latest conditions
      enum Status {
         ROTOR_OK, //!< The last disk/db operation completed successfully
         NOT_FOUND, //!< The rotor, abstractRotor or RotorCalibration ID/GUID was not found
         NOT_OPENED, //!< The rotor, abstractRotor or RotorCalibration file could not be opened
         CONNECT_ERROR, //!< Could not connect to the DB
         MISC_ERROR, //!< An unspecified error occurred
         NOT_SAVED, //!< The file has not been saved
         EDITING, //!< Data is being edited; certain operations not permitted
         HD_ONLY, //!< The file has been saved to the HD
         DB_ONLY, //!< The file has been saved to the DB
         BOTH //!< The file has been saved to both HD and DB
      };

      //! \brief the Operator class defines who can operate an AUC instrument
      class Operator {
         public:
            int ID; //!< The database ID of the operator in the person table
            QString GUID; //!< The GUID of the operator
            QString lname; //!< The last name of the operator
            QString fname; //!< The first name of the operator
      };

      //! \brief the Instrument class describes an AUC instrument
      class Instrument {
         public:
            int ID; //!< The database ID of the instrument
            QString name; //!< The name of the instrument
            QString serial; //!< The serial number of the instrument
            int radialCalID;
            QString optimaHost;
            int optimaPort;
            QString optimaDBname;
            QString optimaDBusername;
            QString optimaDBpassw;
            int selected;
            QString os1;
            QString os2;
            QString os3;

            QString radcalwvl;
            QString chromoab;

            QList<Operator> operators; //!< A list of people authorized to use the instrument
      };

      //! \brief the Lab class describes a location where AUC hardware is used
      class US_UTIL_EXTERN Lab {
         public:
            int ID; //!< The database ID of the lab
            QString GUID; //!< The GUID of the lab
            QString name; //!< The name of the lab
            QString building; //!< The building where the lab is located
            QString room; //!< The room where the lab is located
            QList<Instrument> instruments; //!< A list of instruments in the lab

            //! \brief Generic constructor for the Lab class.
            Lab();

            /*! \brief    Function to read an entire lab structure
                       from the DB
         
             \param    labID The database labID of the
                       desired abstract rotor
             \param    db For database access, an open database connection
         */
            Status readDB(int, US_DB2 * = 0);

            //! \brief Resets the class variables to default values
            void reset(void);

            //! \brief Displays the contents of the class variables in qDebug() statements
            void show(void);
      };

      //! \brief the AbstractRotor structure describes a generic 4- or 8-hole rotor
      class US_UTIL_EXTERN AbstractRotor {
         public:
            int ID; //!< The database ID of the abstract rotor
            QString GUID; //!< The GUID of the abstract rotor
            QString name; //!< The abstract rotor type (Simulation, AN50, AN60, CFA)
            QString material; //!< What this rotor type is made of (Simulation, Titanium, CarbonFiber)
            int numHoles; //!< The number of holes in this rotor type
            int maxRPM; //!< The maximum RPM
            double magnetOffset; //!< The magnet offset
            double cellCenter; //!< The center of the cell
            QString manufacturer; //!< The manufacturer (Beckman, SpinAnalytical)

            //! \brief Generic constructor for the AbstractRotor class.
            AbstractRotor();

            /*! \brief    Function to read an entire abstract rotor structure
                       from the DB
         
             \param    abstractRotorID The database abstractRotorID of the
                       desired abstract rotor
             \param    db For database access, an open database connection
         */
            Status readDB(int, US_DB2 * = 0);

            //! \brief Resets the class variables to default values
            void reset(void);

            //! \brief Displays the contents of the class variables in qDebug() statements
            void show(void);
      };

      //! \brief the rotor structure describes a derived 4- or 8-hole rotor
      class US_UTIL_EXTERN Rotor {
         public:
            int ID; //!< The database ID of this rotor
            int abstractRotorID; //!< The ID of the rotor type
            QString abstractRotorGUID; //!< The GUID of the rotor type
            int labID; //!< The ID of the laboratory which owns this rotor
            QString GUID; //!< The GUID of this rotor
            QString name; //!< The local name of the rotor
            QString serialNumber; //!< The serial number of the rotor

            //! \brief Generic constructor for the Rotor class.
            Rotor();

            /*! \brief    Function to add the current rotor as a new rotor in the DB

             \param    db For database access, an open database connection
         */
            int addRotorDB(US_DB2 * = 0);

            /*! \brief    Function to read an entire rotor structure from the DB
         
             \param    rotorID The database rotorID of the desired rotor
             \param    db For database access, an open database connection
         */
            Status readDB(int, US_DB2 * = 0);

            /*! \brief    Function to delete the specified rotor from the DB

             \param    rotorID The database rotorID of the rotor to delete
             \param    db For database access, an open database connection
         */
            static int deleteRotorDB(int, US_DB2 * = 0);

            //! \brief    Method to save the current rotor to disk
            void saveDisk(void);

            /*! \brief A function to read information about a rotor from disk
         
             \param id A reference to the id that identifies the rotor 
         */
            Status readDisk(const int &);

            //! \brief Resets the class variables to default values
            void reset(void);

            //! \brief Displays the contents of the class variables in qDebug() statements
            void show(void);
      };

      //! \brief the RotorCalibration class describes a calibration experiment and associated data
      class US_UTIL_EXTERN RotorCalibration {
         public:
            int ID; //!< The ID of this calibration
            QString GUID; //!< The GUID of this calibration
            int rotorID; //!< The ID of the rotor this calibration is associated with
            QString rotorGUID; //!< The GUID of the rotor this calibration is associated with
            int calibrationExperimentID; //!< The ID of the experiment that contains the calibration data
            QString calibrationExperimentGUID; //!< The GUID of the experiment that contains the calibration data
            double coeff1; //!< The first order coefficient for the second order polynomial
            double coeff2; //!< The second order coefficient for the second order polynomial
               //!< the zeroth order coefficient is always zero (no stretch at 0 RPM)
            QString label; //!< An identifying label for the calibration profile
            QString report; //!< a calibration report
            QDate lastUpdated; //!< the date on which the rotor was calibrated
            double omega2t; //!< the cumulative force exerted on the rotor

            //! \brief Generic constructor for the RotorCalibration class.
            RotorCalibration();

            /*! \brief    Function to save the current calibration in the DB

             \param    rotorID The ID of the rotor this calibration is associated with
             \param    db For database access, an open database connection
         */
            int saveDB(int, US_DB2 * = 0);

            /*! \brief    Function to read an entire rotor calibration structure from the DB
         
             \param    calibrationID The database rotorCalibrationID of the desired calibration
             \param    db For database access, an open database connection
         */
            Status readDB(int, US_DB2 * = 0);

            /*! \brief    Function to delete the specified rotor calibration from the DB

             \param    calibrationID The database calibrationID of the rotor calibration to delete
             \param    db For database access, an open database connection
         */
            static int deleteCalibrationDB(int, US_DB2 * = 0);

            /*! \brief    Function to find the dummy calibration for the current rotor in the DB,
                       and to replace it in all experiments with this one if it exists.
                       Most likely it is the single calibration experiment that generated the
                       dummy calibration in the first place.

             \param    oldCalibrationID Returns the database ID of the old dummy calibration here
             \param    db For database access, an open database connection
         */
            int replaceDummyDB(int &, US_DB2 * = 0);

            //! \brief    Method to save the current rotor calibration to disk
            void saveDisk(void);

            /*! \brief A function to read information about a rotor calibration from disk
         
             \param id A reference to the id that identifies the rotor 
         */
            Status readDisk(const int &);

            //! \brief A function to read the report part of the rotor
            //!        calibration from disk
            void readReport(QXmlStreamReader &);

            //! \brief Resets the class variables to their default vaules
            void reset(void);

            //! \brief Displays the contents of the class variables in qDebug() statements
            void show(void);
      };

      /*! \brief Generic constructor for the US_Rotor class.
      */
      US_Rotor();

      //! A destructor.
      ~US_Rotor();


      AbstractRotor currentAbstractRotor; //!< Current abstractRotor structure
      Rotor currentRotor; //!< Current rotor structure
      RotorCalibration currentCalibration; //!< Current calibration structure
      int labID; //!< ID of the current lab

      /*! \brief A function to read information about all labs from DB

          \param labList A reference to a vector that contains,
                         or will contain, the list of available labs
          \param    db   For database access, an open database connection
      */
      static Status readLabsDB(QVector<US_Rotor::Lab> &, US_DB2 *db);

      /*! \brief A function to read information about all labs from disk

          \param labList A reference to a vector that contains,
                         or will contain, the list of available labs
      */
      static Status readLabsDisk(QVector<US_Rotor::Lab> &);

      /*! \brief A function to read information about all abstract rotors from DB

          \param arList A reference to a vector that contains,
                        or will contain, the list of available abstract rotors
          \param    db  For database access, an open database connection
      */
      static Status readAbstractRotorsDB(QVector<US_Rotor::AbstractRotor> &, US_DB2 *db);

      /*! \brief A function to read information about all abstract rotors from disk

          \param arList A reference to a vector that contains,
                        or will contain, the list of available abstract rotors
      */
      static Status readAbstractRotorsDisk(QVector<US_Rotor::AbstractRotor> &);

      /*! \brief    Function to retrieve all the rotors in a particular lab from DB

          \param    rotors A reference to where the rotors will be stored
          \param    labID The ID of the lab where the rotors are located
          \param    db  For database access, an open database connection
      */
      static Status readRotorsFromDB(QVector<US_Rotor::Rotor> &, int, US_DB2 *db);

      /*! \brief    Function to retrieve all the rotors in a particular lab from disk

          \param    rotors A reference to where the rotors will be stored
          \param    labID The ID of the lab where the rotors are located
      */
      static Status readRotorsFromDisk(QVector<US_Rotor::Rotor> &, int);

      /*! \brief    Function to retrieve all the calibration profiles about a
                    particular rotor from disk

          \param    profiles A reference to where the rotor calibration profiles 
                             will be stored
          \param    rotorID  The ID of the rotor 
          \param    db  For database access, an open database connection
      */
      static Status readCalibrationProfilesDB(QVector<US_Rotor::RotorCalibration> &, int, US_DB2 *db);

      /*! \brief    Function to retrieve all the calibration profiles about a
                    particular rotor from disk

          \param    profiles A reference to where the rotor calibration profiles 
                             will be stored
          \param    rotorID  The ID of the rotor 
      */
      static Status readCalibrationProfilesDisk(QVector<US_Rotor::RotorCalibration> &, int);

      /*! \brief    Function to find the filename of a rotor or calibration
                    on disk, if it exists.
                    Returns true if successful, false otherwise
          \param    fileMask  Describes what the filenames look like
          \param    lookupTag The xml tag to look for
          \param    lookupID  The ID to look for on disk
          \param    filename  The function will return the filename here if it is found
      */
      static bool diskFilename(const QString &, const QString &, const int &, QString &);


   private:
      static bool diskPath(QString &);
      static QString get_filename(const QString &, const QString &, const QString &, const int &, bool &);
      static void saveLabsDisk(QVector<US_Rotor::Lab> &);
      static void readInstrumentInfo(QXmlStreamReader &, US_Rotor::Lab &);
      static void readOperatorInfo(QXmlStreamReader &, US_Rotor::Instrument &);
};

#endif
