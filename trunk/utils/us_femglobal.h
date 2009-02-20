#ifndef US_FEMGLOBAL_H
#define US_FEMGLOBAL_H

#include <QtCore>

#include "us_extern.h"

struct constraint
{
   bool   fit;
   double low;
   double high;
};

struct mfem_initial
{
   QList< double > radius;
   QList< double > concentration;
};

struct mfem_scan
{
   double          time;
   double          omega_s_t;
   unsigned int    rpm;
   double          temperature;
   QList< double > conc;
};

// A data set comprised of scans from one sample taken at constant speed
struct mfem_data  
{
   QString      id;           // Description of this dataset
   unsigned int cell;         // Cell position in rotor
   unsigned int channel;      // Channel number from centerpiece

   // Single wavelength at which data was acquired (for UV/Vis)
   unsigned int wavelength; 
   unsigned int rpm;          // Constant rotor speed
   
   // The number with which a s20,w value needs
   // to be multiplied to get the s value in experimental space
   // sT,B = s20,W * s20W_correction:
   // sT,B = [s_20,W * [(1-vbar*rho)_T,B * eta_20,W] / 
   //        [(1-vbar*rho)_20,W * eta_T,B]
   double       s20w_correction;  
   
   // The number with which a D20,w value needs 
   // to be multiplied to get the s value in experimental space
   // DT,B = D20,W * D20w_correction
   // DT,B = [D20,W * T * eta_20,W] / [293.15 * eta_T,B]
   
   double       D20w_correction;  
   double       viscosity;        // viscosity of solvent
   double       density;          // density of solvent
   double       vbar;             // temperature corrected vbar
   double       avg_temperature;  // average temperature of all scans
   double       vbar20;           // vbar at 20C
   double       meniscus;
   double       bottom;           // corrected for speed dependent rotor stretch
   QList< double>            radius;
   QList< struct mfem_scan > scan;
};

struct SimulationComponent
{
   double vbar20;
   double mw;
   double s;
   double D;
   double sigma;
   double delta;
   double extinction;
   double concentration;
   double f_f0;
   bool   show_conc;
   bool   show_keq;
   bool   show_koff;
   int    show_stoich;

   // List of associated components for combobox, 
   // if size == zero component is non-interacting
   QList< unsigned int > show_component; 
   QString shape;
   QString name;

   // The radius/concentration points for a user-defined 
   // initial concentration grid
   struct mfem_initial c0; 
};

struct SimulationComponentConstraints
{
   struct constraint vbar20;
   struct constraint mw;
   struct constraint s;
   struct constraint D;
   struct constraint sigma;
   struct constraint delta;
   struct constraint concentration;
   struct constraint f_f0;
};

struct Association
{
   double       keq;
   double       k_off;

   // OpticalDensity, MolecularWeight, MgPerMl, Fringes, Fluorescence 
   QString      units;

   int component1;      // which component is associating
   
   // Which component is dissociating (in heteroassociation this 
   // component is associating)
   int component2;
   
   // Which component is dissociating (only for heteroassoc., 
   // otherwise, if -1 it means self-assoc)
   int          component3;
   unsigned int stoichiometry1;  // stoichiometry of the first component
   unsigned int stoichiometry2;  // stoichiometry of the second component
   
   // Stoichiometry of the third component (0 if reaction only has 2 components)
   unsigned int stoichiometry3; 
   
   // Vector of all components involved in this reaction (new) 
   QList< unsigned int > comp;  
   
   // Vector of stoichiometry of each component (new) 
   QList< unsigned int > stoich; 
   QList< int >          react;    // =1 for reactant, = -1 for product
};

struct AssociationConstraints
{
   struct constraint keq;
   struct constraint koff;
};

struct ModelSystem
{
   QString description;
   int     model;
   QList< struct SimulationComponent > component_vector;
   QList< struct Association >         assoc_vector;
};

struct ModelSystemConstraints
{
   QList< struct SimulationComponentConstraints > component_vector_constraints;
   QList< struct AssociationConstraints         > assoc_vector_constraints;
   unsigned int simpoints;    // number of radial grid points used in simulation
   
   // 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 
   // 3 = user-selected mesh, 4 = nonuniform constant mesh 
   unsigned int mesh;
   int          moving_grid;        // Use moving or fixed time grid
   
   // Loading volume (of lamella) in a band-forming centerpiece 
   double       band_volume;       
};

struct SpeedProfile
{
   unsigned int duration_hours;
   unsigned int duration_minutes;
   unsigned int delay_hours;
   double       delay_minutes;
   unsigned int scans;
   unsigned int acceleration;
   unsigned int rotorspeed;
   bool         acceleration_flag;
};

struct SimulationParameters
{
   // The radii from a user-selected mesh file (mesh == 3)
   QList< double > mesh_radius; 

   // Note: the radius points of c0 do not have to match the radii in the mesh
   // file. The radius values of the c0 vector will be interpolated onto
   // whatever mesh the user has selected.  however, the first and last point
   // of either the c0 or mesh_radius should match the meniscus, otherwise they
   // will be ignored or interpolated to the actual meniscus and bottom
   // position set by the user, which will take precedence.

   QList< struct SpeedProfile > speed_step;
   unsigned int simpoints;   // number of radial grid points used in simulation

   // 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 
   // 3 = user-selected mesh, 4 = nonuniform constant mesh
   unsigned int mesh;         

   // Use adaptive time steps = 1, fixed time steps = 0
   int          moving_grid;           
   
   // The radial datapoint increment/resolution of the final data 
   double       radial_resolution; 

   // Meniscus position at first constant speed
   // For multiple speeds, the user must measure the meniscus at
   // the first constant speed and use that to initialize the routine
   double       meniscus;     
   double       bottom;         // bottom of cell position without rotor stretch
   double       rnoise;         // random noise
   double       tinoise;        // time invariant noise
   double       rinoise;        // radially invariant noise
   int          rotor;          // rotor serial number in database
   bool         band_forming;   // true for band-forming centerpieces

   // Loading volume (of lamella) in a band-forming centerpiece
   double       band_volume;   
  
   // First band sedimentation scan is initializer for concentration 
   bool         band_firstScanIsConcentration; 
};

class US_EXTERN US_FemGlobal 
{
   public:

      static int read_experiment(     struct ModelSystem&,
                                      struct SimulationParameters&,
                                      const QString& );

      static int read_experiment(     QList< struct ModelSystem >&,
                                      struct SimulationParameters&,
                                      const QString& );

      static int write_experiment(    struct ModelSystem&, 
                                      struct SimulationParameters&,
                                      const QString& );

      static int read_simulationParameters( 
                                      struct SimulationParameters&,
                                      const QString& );

      static int read_simulationParameters( 
                                      struct SimulationParameters&,
                                      const QStringList& );

      static int write_simulationParameters( 
                                      struct SimulationParameters&,
                                      const QString& );

      static int read_modelSystem(    struct ModelSystem&,
                                      const QString&,
                                      bool = false );

      static int read_modelSystem(    QList< ModelSystem >&,
                                      const QString& );

      static int read_modelSystem(    struct ModelSystem&,
                                      const  QStringList&,
                                      bool = false, int = 0 );

      static int write_modelSystem(   struct ModelSystem&,
                                      const QString&,
                                      bool = false );

      static int read_constraints(    struct ModelSystem&,
                                      struct ModelSystemConstraints&,
                                      const QString& );

      static int read_constraints(    struct ModelSystem&,
                                      struct ModelSystemConstraints&,
                                      const QStringList& );

      static int write_constraints(   struct ModelSystem&,
                                      struct ModelSystemConstraints&,
                                      const QString& );

      static int read_model_data(     QList< mfem_data >&,
                                      const QString&,
                                      bool = false );

      static int write_model_data(    QList< mfem_data >&,
                                      const QString& );

      static int accumulate_model_monte_carlo_data(
                                      QList< mfem_data >&,
                                      QList< mfem_data >&,
                                      unsigned int );

      static int read_mwl_model_data( QList< mfem_data >&,
                                      const QString& );

   private:
      static double  getDouble( const QStringList&, int, int );
      static QString getString( const QStringList&, int, int );
      static int     getInt   ( const QStringList&, int, int );
      static quint32 getUInt  ( const QStringList&, int, int );
};
#endif
