//! \file us_simparms.h
#ifndef US_SIM_PARAMS_H
#define US_SIM_PARAMS_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO.h"
#include "us_db2.h"

//! A class to hold parameters of a run for simulation purposes.

class US_UTIL_EXTERN US_SimulationParameters
{
   public:
  
   //! The type of mesh used internally for the simulation.
   enum MeshType  { ASTFEM, CLAVERIE, MOVING_HAT, USER, ASTFVM };
   
   //! For a simulation, specify the grid type.
   enum GridType  { FIXED, MOVING };

   class SpeedProfile;

   US_SimulationParameters();

   //! \brief A function to update the simulation parameters to match 
   //! an experiment's raw data.
   //! \param db         Pointer to opened database connection or NULL
   //! \param rawdata    Data structure of raw data that contains run info.
   //! \param incl_speed Flag to include speed steps in update
   //! \param runID      Run ID string
   //! \param dataType   Data type string ("RA", "RI", ...)
   void initFromData( US_DB2*, US_DataIO::RawData&, bool = true, 
		   QString = "",  QString = "");

   //! \brief A function to update the simulation parameters to match 
   //! an experiment's edited data.
   //! \param db         Pointer to opened database connection or NULL
   //! \param editdata   Data structure of edited data that contains run info.
   //! \param incl_speed Flag to include speed steps in update
   void initFromData( US_DB2*, US_DataIO::EditedData&, bool = true );

   //! \brief Read hardware files to update bottom and rotor coefficients array
   //! \param db     Pointer to opened database connection or NULL
   //! \param rCalID New rotor calibration identifier to set and use for
   //!               coefficients (default = "0")
   //! \param cp     Centerpiece index (default = 0)
   //! \param ch     Index to channel in centerpiece (default = 0) 
   void setHardware( US_DB2* = NULL, QString = "0", int = 0, int = 0 );

   //! \brief Read hardware files to update bottom and rotor coefficients array
   //! \param rCalID New rotor calibration identifier to set and use for
   //!               coefficients (default = "0")
   //! \param cp     Centerpiece index (default = 0)
   //! \param ch     Index to channel in centerpiece (default = 0) 
   void setHardware( QString = "0", int = 0, int = 0 );

   //! \brief Load simulation parameters from an xml file
   //! \param fname Full path name of file from which to load simulation
   //!              parameters; name part in "sp_*.xml" form.
   //! \returns     Status flag:  0 if able to read from file
   int  load_simparms( QString );

   //! \brief Save simulation parameters into an xml file
   //! \param fname Full path name of file to which to save simulation
   //!              parameters; name part usually in "sp_*.xml" form.
   //! \returns     Status flag:  0 if able to write to file
   int  save_simparms( QString );

   //! \brief Static function to load simulation parameters from an xml file
   //! \param sparms Reference to simulation parameters object to load
   //! \param fname  Full path name of file from which to load simulation
   //!               parameters; name part in "sp_*.xml" form.
   //! \returns      Status flag:  0 if able to read from file
   static int get_simparms( US_SimulationParameters&, QString );

   //! \brief Static function to save simulation parameters into an xml file
   //! \param sparms Reference to simulation parameters object to save
   //! \param fname  Full path name of file to which to save simulation
   //!               parameters; name part usually in "sp_*.xml" form.
   //! \returns      Status flag:  0 if able to write to file
   static int put_simparms( US_SimulationParameters&, QString );

   //! \brief A function to read speed steps from runID file
   //! \param runID      Run ID string
   //! \param dataType   Data type string (e.g., "RI")
   //! \param speedsteps Returned vector of speed step profiles
   //! \returns          Number of speed steps found
   static int  readSpeedSteps( QString, QString, QVector< SpeedProfile >& );

   //! \brief A function to compute speed steps from data scans
   //! \param scans      Pointer to vector of data scans
   //! \param speedsteps Returned vector of speed step profiles
   static void computeSpeedSteps( QVector< US_DataIO::Scan >*, QVector< SpeedProfile >& );

   //! \brief Static function to get a speed step profile from an xml portion
   //! \param xmli   Reference to xml stream from which to read
   //! \param spo    Reference to speed profile to populate
   static void speedstepFromXml( QXmlStreamReader&, SpeedProfile& );

   //! \brief Static function to write a speed step profile to an xml portion
   //! \param xmlo   Reference to xml stream to which to write
   //! \param spi    Pointer to speed profile to represent in xml
   static void speedstepToXml( QXmlStreamWriter&, SpeedProfile* );

   //! \brief Static function to get all speed steps for an experiment from DB
   //! \param dbP    Pointer to opened database connection
   //! \param expID  ID of experiment for which to fetch speed steps
   //! \param sps    Reference to vector of speed profiles to populate
   //! \returns      The number of speed steps found for experiment
   static int speedstepsFromDB( US_DB2*, int, QVector< SpeedProfile >& );

   //! \brief Static function to write a speed step profile to an xml portion
   //! \param dbP    Pointer to opened database connection
   //! \param expID  ID of experiment for which to upload a speed step
   //! \param spi    Pointer to speed profile to upload to the database
   //! \returns      The speedstep DB ID number (<1 if error)
   static int speedstepToDB( US_DB2*, int, SpeedProfile* );

   //! \brief Dump class contents to stderr
   void debug( void );


   //! The radii from a user-selected mesh file (mesh == USER)
   QVector< double > mesh_radius; 

   /*!< Note: the radius points of c0 do not have to match the radii in the
        mesh file. The radius values of the c0 vector will be interpolated
        onto whatever mesh the user has selected.  however, the first and last
        point of either the c0 or mesh_radius should match the meniscus,
        otherwise they will be ignored or interpolated to the actual meniscus
        and bottom position set by the user, which will take precedence. */

   //! Specifics for each rpm value in the simulation
   QVector< SpeedProfile > speed_step;

   int       simpoints;         //!< number of radial grid points used in sim
   MeshType  meshType;          //!< Type of radial grid 
   GridType  gridType;          //!< Designation if grid is fixed or can move
   double    radial_resolution; //!< The radial datapoint increment/resolution 
                                //!< of the final data
   double    meniscus;          //!< Meniscus position at first constant speed
                                //!< For multiple speeds, the user must measure 
                                //!< the meniscus at the first constant speed 
                                //!< and use that to initialize the routine
   double    bottom;            //!< Bottom of cell position w/o rotor stretch
   double    temperature;       //!< Temperature in degrees centigrade
   double    rnoise;            //!< Random noise
   double    tinoise;           //!< Time invariant noise
   double    rinoise;           //!< Radially invariant noise
   bool      band_forming;      //!< True for band-forming centerpieces
   double    band_volume;       //!< Loading volume (of lamella) in a 
                                //!< Band-forming centerpiece
   double    bottom_position;   //!< Bottom position from centerpiece,channel
   QString   rotorCalID;        //!< Rotor calibration identifier in DB/XML
   double    rotorcoeffs[ 2 ];  //!< Rotor coefficients for stretch calculation

   //! First sedimentation scan is initializer for concentration
   bool      firstScanIsConcentration; 

   int       cp_sector;         //!< Shape of centerpiece (0-4: see US_Hardware)
   double    cp_pathlen;        //!< Pathlength of centerpiece
   double    cp_angle;          //!< Angle of centerpiece sector
   double    cp_width;          //!< Width of centerpiece channel if rectangular

   //! Each distinct RPM value in a series of speeds 
   class US_UTIL_EXTERN  SpeedProfile
   {
      public:

      SpeedProfile();

      double duration_minutes;  //!< Minutes at the given speed (0-59)
      double delay_minutes;     //!< Minutes delay before starting scans (0-59)
      double w2t_first;         //!< omega2t at first scan of step
      double w2t_last;          //!< omega2t at last scan of step
      double avg_speed;         //!< Unrounded average speed in speed step
      double speed_stddev;      //!< Standard deviation of speeds in speed step
      int    duration_hours;    //!< Hours at the given speed
      int    delay_hours;       //!< Hours delay before starting scans
      int    time_first;        //!< Seconds at first scan of step
      int    time_last;         //!< Seconds at last scan of step
      int    scans;             //!< Number of scans at this RPM
      int    rotorspeed;        //!< RPM for this step
      int    acceleration;      //!< Acceleration rate from previous RPM (RPM/second)
      int    set_speed;         //!< Set speed for MWL data
      bool   acceleration_flag; //!< Flag to simulate RPM acceleration or not
   };
};

#endif
