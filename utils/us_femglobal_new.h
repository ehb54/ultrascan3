#ifndef US_FEMGLOBAL_NEW_H
#define US_FEMGLOBAL_NEW_H

#include <QtCore>
#include <vector>
#include "us_extern.h"

using namespace std;

class US_EXTERN US_FemGlobal_New
{
   public:

   class MfemInitial
   {
      public:
      vector< double > radius;
      vector< double > concentration;
   };

   class MfemScan
   {
      public:
      double           time;
      double           omega2t;
      uint             rpm;
      double           temperature;
      vector< double > conc;
   };

   class MfemData                
   // A data set comprised of scans from one sample taken at constant speed
   {
      public:
      QString id;               // description of this dataset
      uint    cell;             // cell position in rotor
      uint    channel;          // channel number from centerpiece
      uint    wavelength;       // single wavelength at which data was acquired
                                // (for UV/Vis)
      uint    rpm;              // constant rotor speed

      double  s20w_correction;  // this is the number with which a s20,w value 
                                // needs to be multiplied to get the s value in 
                                // experimental space
                                // sT,B = s20,W * s20W_correction:
                                // sT,B = [s_20,W * [(1-vbar*rho)_T,B * eta_20,W]
                                //    /   [(1-vbar*rho)_20,W * eta_T,B]
                                
      double  D20w_correction;  // this is the number with which a D20,w value 
                                // needs to be multiplied to get the s value in 
                                // experimental space
                                // DT,B = D20,W * D20w_correction
                                // DT,B = [D20,W * T * eta_20,W] 
                                //    / [293.15 * eta_T,B]
      double viscosity;         // viscosity of solvent
      double density;           // density of solvent
      double vbar;              // temperature corrected vbar
      double avg_temperature;   // average temperature of all scans
      double vbar20;            // vbar at 20C
      double meniscus;          // corrected for speed dependent rotor stretch
      double bottom;            // corrected for speed dependent rotor stretch
      vector< double >   radius;
      vector< MfemScan > scan;
   };

   enum ShapeType { SPHERE, PROLATE, OBLATE, ROD };
   enum MeshType  { ASTFEM, CLAVERIE, MOVING_HAT, USER, ADAPTIVE };
   enum GridType  { FIXED, MOVING };
   enum OpticsType{ ABSORBANCE, INTERFERENCE, FLUORESCENCE };

   class SimulationComponent
   {
      public:
      SimulationComponent();
      uchar       analyteGUID[ 16 ];    // GUID for the analyte in the MySQL DB
      double      molar_concentration;
      double      signal_concentration; // To be assigned prior to simulation
      double      vbar20;
      double      mw;
      double      s;
      double      D;
      double      f;
      double      f_f0;
      double      wavelength;
      double      extinction;
      double      sigma;   // Concentration dependency of s
      double      delta;   // concentration dependency of D
      ShapeType   shape;
      QString     name;
      int         analyte_type;
      MfemInitial c0;      // The radius/concentration points for a user-defined
                           // initial concentration grid
   };

   class Association
   {
      public:
      Association();
      double         k_eq;
      double         k_off;
   
      // A list of all system components involved in this reaction
      vector< uint > reaction_components;   

      // Stoichiometry of components in chemical equation.
      // Positive for reactant, negative for product
      vector< int >  stoichiometry; 
   };

   class ModelSystem
   {
      public:
      ModelSystem();
      double                         viscosity;
      double                         compressibility;
      double                         temperature;
      OpticsType                     optics;
      QString                        description;

      //! An index of components (-1 means none)$
      int                            coSedSolute;

      QVector< SimulationComponent > components;
      QVector< Association >         associations;
   };

   class SpeedProfile
   {
      public:
      uint   duration_hours;
      uint   duration_minutes;
      uint   delay_hours;
      double delay_minutes;
      uint   scans;
      uint   acceleration;
      uint   rotorspeed;
      bool   acceleration_flag;
   };

   class SimulationParameters
   {
      public:
      
      // The radii from a user-selected mesh file (mesh == USER)
      vector< double > mesh_radius; 

      // Note: the radius points of c0 do not have to match the radii in the
      // mesh file. The radius values of the c0 vector will be interpolated
      // onto whatever mesh the user has selected.  however, the first and last
      // point of either the c0 or mesh_radius should match the meniscus,
      // otherwise they will be ignored or interpolated to the actual meniscus
      // and bottom position set by the user, which will take precedence.

      vector< SpeedProfile > speed_step;
      uint      simpoints;         // number of radial grid points used in sim
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
   };


   static int write_model( const QString&, const ModelSystem& ); 

/*
   static int read_experiment(     ModelSystem&,
                                   SimulationParameters&,
                                   const QString& );

   static int read_experiment(     vector< struct ModelSystem >&,
                                   SimulationParameters&,
                                   const QString& );

   static int write_experiment(    ModelSystem&,
                                   SimulationParameters&,
                                   const QString& );

   static int read_simulationParameters(
                                   SimulationParameters&,
                                   const QString& );

   static int read_simulationParameters(
                                   SimulationParameters&,
                                   const QStringList& );

   static int write_simulationParameters(
                                   SimulationParameters&,
                                   const QString& );

   static int read_modelSystem(    ModelSystem&,
                                   const QString&,
                                   bool = false );

   static int read_modelSystem(    vector< ModelSystem >&,
                                   const QString& );

   static int read_modelSystem(    ModelSystem&,
                                   const QStringList&,
                                   bool = false, int = 0 );

   static int write_modelSystem(   ModelSystem&,
                                   const QString&,
                                   bool = false );

   static int read_model_data(     vector< MfemData >&,
                                   const QString&,
                                   bool = false );

   static int write_model_data(    vector< MfemData >&,
                                   const QString& );

   static int accumulate_model_monte_carlo_data(
                                   vector< MfemData >&,
                                   vector< MfemData >&,
                                   uint );

   static int read_mwl_model_data( vector< MfemData >&,
                                   const QString& );

   private:
      static double  getDouble( const QStringList&, int, int );
      static QString getString( const QStringList&, int, int );
      static int     getInt   ( const QStringList&, int, int );
      static quint32 getUInt  ( const QStringList&, int, int );
*/
};

#endif
