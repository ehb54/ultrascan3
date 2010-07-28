//! \file us_simparms.h
#ifndef US_SIM_PARAMS_H
#define US_SIM_PARAMS_H

#include <QtCore>
#include "us_extern.h"
#include "us_dataIO2.h"

class US_EXTERN US_SimulationParameters
{
   public:
  
   enum MeshType  { ASTFEM, CLAVERIE, MOVING_HAT, USER, ASVFEM };
   enum GridType  { FIXED, MOVING };

   class SpeedProfile;

   US_SimulationParameters();

   void initFromData( US_DataIO2::EditedData& );

   // The radii from a user-selected mesh file (mesh == USER)
   QVector< double > mesh_radius; 

   // Note: the radius points of c0 do not have to match the radii in the
   // mesh file. The radius values of the c0 vector will be interpolated
   // onto whatever mesh the user has selected.  however, the first and last
   // point of either the c0 or mesh_radius should match the meniscus,
   // otherwise they will be ignored or interpolated to the actual meniscus
   // and bottom position set by the user, which will take precedence.

   QVector< SpeedProfile > speed_step;

   int       simpoints;         // number of radial grid points used in sim
   MeshType  meshType;    
   GridType  gridType;          // Type of grid
   double    radial_resolution; // The radial datapoint increment/resolution 
                                // of the final data

   double    meniscus;          // Meniscus position at first constant speed
                                // For multiple speeds, the user must measure 
                                // the meniscus at the first constant speed 
                                // and use that to initialize the routine
   
   double    bottom;            // Bottom of cell position without rotor stretch
   double    rnoise;            // Random noise
   double    tinoise;           // Time invariant noise
   double    rinoise;           // Radially invariant noise
   int       rotor;             // Rotor serial number in database
   bool      band_forming;      // True for band-forming centerpieces
   double    band_volume;       // Loading volume (of lamella) in a 
                                // Band-forming centerpiece

   // First band sedimentation scan is initializer for concentration
   bool      band_firstScanIsConcentration; 

   class SpeedProfile
   {
      public:

      SpeedProfile();

      int    duration_hours;
      int    duration_minutes;
      int    delay_hours;
      double delay_minutes;
      int    scans;
      int    acceleration;
      int    rotorspeed;
      bool   acceleration_flag;
   };
};

#endif
