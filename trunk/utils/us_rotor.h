//! \file us_rotor.h
#ifndef US_ROTOR_H
#define US_ROTOR_H

#include "us_extern.h"
#include "us_db2.h"

/*! \class US_Rotor
   This class provides a low-level interface to the Rotor tables, allowing
   the user to read and write them from db or disk.
*/

class US_EXTERN US_Rotor
{

   public:

      //! \brief   Some status codes to keep track of latest conditions
      enum Status
      {
         ROTOR_OK,          //!< The last disk/db operation completed successfully
         NOT_FOUND,         //!< The rotor, abstractRotor or RotorCalibration ID/GUID was not found
         MISC_ERROR,        //!< An unspecified error occurred
         NOT_SAVED,         //!< The file has not been saved
         EDITING,           //!< Data is being edited; certain operations not permitted
         HD_ONLY,           //!< The file has been saved to the HD
         DB_ONLY,           //!< The file has been saved to the DB
         BOTH               //!< The file has been saved to both HD and DB
      };

      //! \brief the AbstractRotor structure describes a generic 4- or 8-hole rotor
      class AbstractRotor
      {
         public:
         int       ID;
         QString   GUID;
         QString   name;
         QString   material;
         int       numHoles;
         int       maxRPM;
         double    magnetOffset;
         double    cellCenter;
         QString   manufacturer;
      
         //! \brief Generic constructor for the AbstractRotor class.
         AbstractRotor();

         /*! \brief    Function to read an entire abstract rotor structure
                       from the DB
         
             \param    abstractRotorID The database abstractRotorID of the
                       desired abstract rotor
             \param    db For database access, an open database connection
         */
         Status readDB        ( int, US_DB2* = 0 );

         //! \brief Resets the class variables to default values
         void      reset( void );
      
         //! \brief Displays the contents of the class variables in qDebug() statements
         void      show  ( void );
      };

      //! \brief the rotor structure describes a derived 4- or 8-hole rotor
      class Rotor
      {
         public:
         int       ID;
         int       abstractRotorID;
         QString   abstractRotorGUID;
         int       labID;              //!< The ID of the laboratory which owns this rotor
         QString   GUID;
         QString   name;
         QString   serialNumber;
   
         //! \brief Generic constructor for the Rotor class.
         Rotor();

         /*! \brief    Function to add the current rotor as a new rotor in the DB

             \param    db For database access, an open database connection
         */
         int    addRotorDB( US_DB2* = 0 );

         /*! \brief    Function to read an entire rotor structure from the DB
         
             \param    rotorID The database rotorID of the desired rotor
             \param    db For database access, an open database connection
         */
         Status readDB        ( int, US_DB2* = 0 );

         /*! \brief    Function to delete the specified rotor from the DB

             \param    rotorID The database rotorID of the rotor to delete
             \param    db For database access, an open database connection
         */
         static int    deleteRotorDB( int, US_DB2* = 0 );

         //! \brief Resets the class variables to default values
         void      reset( void );
   
         //! \brief Displays the contents of the class variables in qDebug() statements
         void      show ( void );
      };
   
      //! \brief the RotorCalibration class describes a calibration experiment and associated data
      class RotorCalibration
      {
         public:
         int     ID;           //!< The ID of this calibration
         QString GUID;         //!< The GUID of this calibration
         int     rotorID;      //!< The ID of the rotor this calibration is associated with
         QString rotorGUID;    //!< The GUID of the rotor this calibration is associated with
         int     calibrationExperimentID; //!< The ID of the experiment that contains the calibration data
         QString calibrationExperimentGUID; //!< The GUID of the experiment that contains the calibration data
         double  coeff1;       //!< The first order coefficient for the second order polynomial
         double  coeff2;       //!< The second order coefficient for the second order polynomial
                               //!< the zeroth order coefficient is always zero (no stretch at 0 RPM)
         QString report;       //!< a calibration report 
         QDate   lastUpdated;  //!< the date on which the rotor was calibrated
         double  omega2t;      //!< the cumulative force exerted on the rotor
   
         //! \brief Generic constructor for the RotorCalibration class.
         RotorCalibration();

         /*! \brief    Function to save the current calibration in the DB

             \param    rotorID The ID of the rotor this calibration is associated with
             \param    db For database access, an open database connection
         */
         int    saveDB( int, US_DB2* = 0 );

         /*! \brief    Function to read an entire rotor calibration structure from the DB
         
             \param    calibrationID The database rotorCalibrationID of the desired calibration
             \param    db For database access, an open database connection
         */
         Status readDB( int, US_DB2* = 0 );

         /*! \brief    Function to delete the specified rotor calibration from the DB

             \param    calibrationID The database calibrationID of the rotor calibration to delete
             \param    db For database access, an open database connection
         */
         static int    deleteCalibrationDB( int, US_DB2* = 0 );

         //! \brief Resets the class variables to their default vaules
         void    reset ( void );
   
         //! \brief Displays the contents of the class variables in qDebug() statements
         void    show  ( void );
   
      };

      /*! \brief Generic constructor for the US_Rotor class.
      */
      US_Rotor();

      //! A destructor.
      ~US_Rotor();


      AbstractRotor     currentAbstractRotor;   //!< Current abstractRotor structure
      Rotor             currentRotor;           //!< Current rotor structure
      RotorCalibration  currentCalibration;     //!< Current calibration structure
      int               labID;

      /*! \brief A function to read information about all abstract rotors from DB

          \param arList A reference to a vector that contains,
                        or will contain, the list of available abstract rotors
          \param    db  For database access, an open database connection
      */
      static Status readAbstractRotorsDB( QVector< US_Rotor::AbstractRotor >&, US_DB2* db );

      /*! \brief A function to read information about all abstract rotors from disk

          \param arList A reference to a vector that contains,
                        or will contain, the list of available abstract rotors
      */
      static Status readAbstractRotorsDisk( QVector< US_Rotor::AbstractRotor >& );

//      void readRotorFromDisk( QString& );
//      void readCalibrationFromDisk( QString& );

      //! \brief    Method to save the current rotor/calibration to disk
//      void saveRotorToDisk        ( void );
//      void saveCalibrationToDisk  ( void );

      /*! \brief    Function to find the filename of a solution on disk, if it exists.
                    Returns true if successful, false otherwise
          \param    guid The GUID of the solution to look for on disk
          \param    filename The function will return the filename here if it is found
      */
//      bool diskFilename      ( const QString& , QString& );


   private:

        static bool    diskPath          ( QString& );
        static QString get_filename      ( const QString&, bool& );
        static void    saveAbstractRotorsDisk( QVector< US_Rotor::AbstractRotor >& );

//      void readRotorInfo  ( QXmlStreamReader& xml );
//      void readRotorCalibrationInfo  ( QXmlStreamReader& xml );
//      QString get_filename   ( const QString&, bool& );
      
};

#endif
