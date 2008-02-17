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
//#include "us_global.h"

using namespace std;

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
	float temperature;
	std::vector <double> conc;
};

struct mfem_data 					// a data set comprised of scans from one sample taken at constant speed
{
	QString id;						// description of this dataset
	unsigned int cell;			// cell position in rotor
	unsigned int channel;		// channel number from centerpiece
	unsigned int wavelength;	// single wavelength at which data was acquired (for UV/Vis)
	unsigned int rpm;				// constant rotor speed
	float s20w_correction;		// this is the number with which a s20,w value needs
										// to be multiplied to get the s value in experimental space
										// sT,B = s20,W * s20W_correction:
										// sT,B = [s_20,W * [(1-vbar*rho)_T,B * eta_20,W] / [(1-vbar*rho)_20,W * eta_T,B]
	float D20w_correction;		// this is the number with which a D20,w value needs
										// to be multiplied to get the s value in experimental space
										// DT,B = D20,W * D20w_correction
										// DT,B = [D20,W * T * eta_20,W] / [293.15 * eta_T,B]
	double viscosity;				// viscosity of solvent
	double density;				// density of solvent
	double vbar;					// temperature corrected vbar
	double avg_temperature;		// average temperature of all scans
	double vbar20;					// vbar at 20C
	double meniscus, bottom; 	// corrected for speed dependent rotor stretch
	std::vector <double> radius;
	std::vector <struct mfem_scan> scan;
};

struct SimulationComponent
{
	float vbar20;
	float mw;
	float s;
	float D;
	float sigma;
	float delta;
	float extinction;
	float concentration;
	float f_f0;
	bool show_conc;
	bool show_keq;
	bool show_koff;
	int show_stoich;
	vector <unsigned int> show_component;
	QString shape;
	QString name;
	struct mfem_initial c0; // the radius/concentration points for a user-defined initial concentration grid
};

struct Association
{
	float keq;
	float k_off;
	QString units; 					// OpticalDensity, MolecularWeight, MgPerMl, Fringes, Fluorescence
	int component1;					// which component is associating
	int component2;					// which component is dissociating
	int component3;					// which component is dissociating (for heteroassoc., if -1 it means self-assoc)
	unsigned int stoichiometry1;	// stoichiometry of the first component
	unsigned int stoichiometry2;	// stoichiometry of the second component
	unsigned int stoichiometry3;	// stoichiometry of the third component
};

struct ModelSystem
{
	QString description;
	int model;
	vector <struct SimulationComponent> component_vector;
	vector <struct Association> assoc_vector;
};

struct SpeedProfile
{
	unsigned int duration_hours;
	unsigned int duration_minutes;
	unsigned int delay_hours;
	float delay_minutes;
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
	unsigned int simpoints;
	unsigned int mesh; 			// 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 3 = user-selected mesh, 4 = nonuniform constant mesh
	int moving_grid; 				// Use moving or fixed time grid
	float radial_resolution;	// the radial datapoint increment/resolution of the final data
	float meniscus;				// meniscus position without rotor stretch
	float bottom;					// bottom of cell position without rotor stretch
	float rnoise;					// random noise
	float inoise;					// time invariant noise
	float rinoise;					// radially invariant noise
	int centerpiece;				// centerpiece serial number in database
	int rotor;						// rotor serial number in database
	bool band_forming;			// true for band-forming centerpieces
	float band_volume;			// loding volume (of lamella) in a band-forming centerpiece
};

class US_EXTERN US_FemGlobal : public QObject
{
	Q_OBJECT
	public:

		US_FemGlobal(QObject * parent=0, const char * name=0);
		~US_FemGlobal();

	public slots:
		
		int read_experiment(struct ModelSystem *, struct SimulationParameters *, QString);
		int write_experiment(struct ModelSystem *, struct SimulationParameters *, QString);
		
		int read_simulationParamaters(struct SimulationParameters *, QString);
		int write_simulationParamaters(struct SimulationParameters *, QString);
		
		int read_modelSystem(struct ModelSystem *, QString);
		int write_modelSystem(struct ModelSystem *, QString);
		
	signals:
		void new_error(QString);
};

#endif
