//! \file us_simparms.h
#ifndef US_SIM_PARAMS_H
#define US_SIM_PARAMS_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO2.h"

//! A class to hold parameters of a run for simulation purposes.

class US_EXTERN US_SimulationParameters
{
   public:
  
   //! The type of mesh used internally for the simulation.
   enum MeshType  { ASTFEM, CLAVERIE, MOVING_HAT, USER, ASVFEM };
   
   //! For a sumulation, specify the grid type.
   enum GridType  { FIXED, MOVING };

   class SpeedProfile;

   US_SimulationParameters();

   //! \brief A function to update the simulation parameters to match 
   //! an experiment's edited data.
   //! \param editdata Data structure of edited data that contains run info.
   void initFromData( US_DataIO2::EditedData& );

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
   
   double    bottom;            //!< Bottom of cell position without rotor stretch
   double    rnoise;            //!< Random noise
   double    tinoise;           //!< Time invariant noise
   double    rinoise;           //!< Radially invariant noise
   int       rotor;             //!< Rotor serial number in database
   bool      band_forming;      //!< True for band-forming centerpieces
   double    band_volume;       //!< Loading volume (of lamella) in a 
                                //!< Band-forming centerpiece

   //! First band sedimentation scan is initializer for concentration
   bool      band_firstScanIsConcentration; 

   //! Each distinct RPM value in a series of speeds 
   class SpeedProfile
   {
      public:

      SpeedProfile();

      int    duration_hours;   //!< Hours at the given speed
      int    duration_minutes; //!< Minutes at the given speed (0-59)
      int    delay_hours;      //!< Hours delay before starting scans
      double delay_minutes;    //!< Minutes delay before starting scans (0-59)
      int    scans;            //!< Number of scans at this RPM
      int    acceleration;     //!< Acceleration rate from previus RPM (RPM/second)
      int    rotorspeed;       //!< RPM for this step
      bool   acceleration_flag; //!< Flag to simulate RPM acceleration or not
   };
};

#endif
