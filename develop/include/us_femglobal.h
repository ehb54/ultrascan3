#ifndef US_FEMGLOBAL_H
#define US_FEMGLOBAL_H

#include <vector>
#include <iostream>

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qapp.h>

#include "us_extern.h"
#include "us_math.h"
#include "us_util.h"

using namespace std;

struct constraint
{
   bool fit;
   double low;
   double high;
};

struct mfem_initial
{
   std::vector <double> radius;
   std::vector <double> concentration;
};

struct mfem_scan
{
   double time;
   double omega_s_t;
   unsigned int rpm;
   double temperature;
  // vector <short> ignore; // set to Y if this point should be ignored
   std::vector <double> conc;
};

struct mfem_data                // a data set comprised of scans from one sample taken at constant speed
{
   QString id;                  // description of this dataset
   unsigned int cell;         // cell position in rotor
   unsigned int channel;      // channel number from centerpiece
   unsigned int wavelength;   // single wavelength at which data was acquired (for UV/Vis)
   unsigned int rpm;            // constant rotor speed
   double s20w_correction;      // this is the number with which a s20,w value needs
                              // to be multiplied to get the s value in experimental space
                              // sT,B = s20,W * s20W_correction:
                              // sT,B = [s_20,W * [(1-vbar*rho)_T,B * eta_20,W] / [(1-vbar*rho)_20,W * eta_T,B]
   double D20w_correction;      // this is the number with which a D20,w value needs
                              // to be multiplied to get the s value in experimental space
                              // DT,B = D20,W * D20w_correction
                              // DT,B = [D20,W * T * eta_20,W] / [293.15 * eta_T,B]
   double viscosity;            // viscosity of solvent
   double density;            // density of solvent
   double vbar;               // temperature corrected vbar
   double avg_temperature;      // average temperature of all scans
   double vbar20;               // vbar at 20C
   double meniscus, bottom;    // corrected for speed dependent rotor stretch
   std::vector <double> radius;
   std::vector <struct mfem_scan> scan;
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
   bool show_conc;
   bool show_keq;
   bool show_koff;
   int show_stoich;
   vector <unsigned int> show_component; // list of associated components for combobox, if size == zero component is non-interacting
   QString shape;
   QString name;
   struct mfem_initial c0; // the radius/concentration points for a user-defined initial concentration grid
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
   double keq;
   double k_off;
   QString units;                // OpticalDensity, MolecularWeight, MgPerMl, Fringes, Fluorescence
   unsigned int component1;      // which component is associating
   unsigned int component2;      // which component is dissociating (in heteroassociation this component is associating)
   int component3;               // which component is dissociating (only for heteroassoc., otherwise, if -1 it means self-assoc)
   unsigned int stoichiometry1;   // stoichiometry of the first component
   unsigned int stoichiometry2;   // stoichiometry of the second component
   unsigned int stoichiometry3;   // stoichiometry of the third component (0 if reaction only has 2 components)
   vector <unsigned int> comp;   // vector of all components involved in this reaction (new)
   vector <unsigned int> stoich; // vector of stoichiometry of each component (new)
   vector <int> react;            // =1 for reactant, = -1 for product
};

struct AssociationConstraints
{
   struct constraint keq;
   struct constraint koff;
};

struct ModelSystem
{
   QString description;
   int model;
   vector <struct SimulationComponent> component_vector;
   vector <struct Association> assoc_vector;
};

struct ModelSystemConstraints
{
   vector <struct SimulationComponentConstraints> component_vector_constraints;
   vector <struct AssociationConstraints> assoc_vector_constraints;
   unsigned int simpoints;      // number of radial grid points used in simulation
   unsigned int mesh;          // 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 3 = user-selected mesh, 4 = nonuniform constant mesh
   int moving_grid;             // Use moving or fixed time grid
   double band_volume;         // loading volume (of lamella) in a band-forming centerpiece
};

struct SpeedProfile
{
   unsigned int duration_hours;
   unsigned int duration_minutes;
   unsigned int delay_hours;
   double delay_minutes;
   unsigned int scans;
   unsigned int acceleration;
   unsigned int rotorspeed;
   bool acceleration_flag;
};

struct SimulationParameters
{
   vector <double> mesh_radius; // the radii from a user-selected mesh file (mesh == 3)

   // Note: the radius points of c0 do not have to match the radii in the mesh file. The
   // radius values of the c0 vector will be interpolated onto whatever mesh the user has selected.
   // however, the first and last point of either the c0 or mesh_radius should match the
   // meniscus, otherwise they will be ignored or interpolated to the actual meniscus and bottom
   // position set by the user, which will take precedence.

   vector <struct SpeedProfile> speed_step;
   unsigned int simpoints;      // number of radial grid points used in simulation
   unsigned int mesh;          // 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 3 = user-selected mesh, 4 = nonuniform constant mesh
   int moving_grid;             // use adaptive time steps = 1, fixed time steps = 0
   double radial_resolution;   // the radial datapoint increment/resolution of the final data
   double meniscus;            // meniscus position at first constant speed
                              // for multiple speeds, the user must measure the meniscus at
                              // the first constant speed and use that to initialize the routine
   double bottom;               // bottom of cell position without rotor stretch
   double rnoise;               // random noise
   double tinoise;            // time invariant noise
   double rinoise;            // radially invariant noise
   int rotor;                  // rotor serial number in database
   bool band_forming;         // true for band-forming centerpieces
   double band_volume;         // loading volume (of lamella) in a band-forming centerpiece
   bool band_firstScanIsConcentration; // first band sedimentation scan is initializer for concentration
};

class US_EXTERN US_FemGlobal : public QObject
{
   Q_OBJECT
   public:

      US_FemGlobal(QObject * parent=0, const char * name=0);
      ~US_FemGlobal();

   public slots:

      int read_experiment(struct ModelSystem *, struct SimulationParameters *, QString);
      int read_experiment(vector <struct ModelSystem> *, struct SimulationParameters *, QString);
      int write_experiment(struct ModelSystem *, struct SimulationParameters *, QString);

      int read_simulationParameters(struct SimulationParameters *, QString);
      int read_simulationParameters(struct SimulationParameters *, vector <QString>);
      int write_simulationParameters(struct SimulationParameters *, QString);

      int read_modelSystem(struct ModelSystem *, QString, bool flag=false);
      int read_modelSystem(vector<ModelSystem> *, QString);
      int read_modelSystem(struct ModelSystem *, vector <QString>, bool flag=false, int offset=0);
      int write_modelSystem(struct ModelSystem *, QString, bool flag=false);

      int read_constraints(struct ModelSystem *, struct ModelSystemConstraints *, QString);
      int read_constraints(struct ModelSystem *, struct ModelSystemConstraints *, vector <QString>);
      int write_constraints(struct ModelSystem *, struct ModelSystemConstraints *, QString);

      int read_model_data(vector <mfem_data> *, 
                          QString filename, 
                          bool ignore_errors = false,
                          QDataStream *ds = (QDataStream *) NULL);  // set ds to read from existing stream
      int write_model_data(vector <mfem_data> *, 
                           QString filename,
                           QDataStream *ds = (QDataStream *) NULL);  // set ds to append to existing stream
      int convert_analysis_data(QString infile, QString outfile);
      int convert_analysis_data(QString infile, QStringList *qsl);
      int write_ascii_model_data(vector <mfem_data> *, QString filename);
      int accumulate_model_monte_carlo_data(vector <mfem_data> *accumulated_model, vector <mfem_data> *source_model, unsigned int monte_carlo_iterations);
      int read_mwl_model_data(vector <mfem_data> *, QString filenamebase);


   signals:
      void new_error(QString);
      void simparams_name(QString);
      void model_name(QString);
};

#endif
