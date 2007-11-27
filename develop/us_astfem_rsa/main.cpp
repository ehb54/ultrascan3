#include "../include/us_astfem_rsa.h"

int main ()
{
	unsigned int i, scans=40, points=500;
	double dval, total_time = 10*14400.0;
	struct mfem_initial c0;
	struct astfem_rsa rsa;
	struct astfem_component comp;
	struct mfem_scan single_scan;
// initialize the starting grid c0 with 500 points
	dval = 5.8;
	c0.radius.clear();
	c0.concentration.clear();
	for (i=0; i<points; i++)
	{
		c0.radius.push_back(dval + i * 1.4/(points-1));
		c0.concentration.push_back(1.0); // uniform starting concentration
	}

// initialize the rsa structure:
// monomer - dimer equilibrium (see: http://www.ultrascan.uthscsa.edu/manual/equil7.html)
	rsa.component.clear();
	rsa.ModelNumber = 4; 
	rsa.k_off = 0.1; // Weiming, what's the range here?
	rsa.MeshOption = 5;
	rsa.Ninput = 20; // desired grid size used for simulation

// initialize the starting component (2 for model 4):
// Monomer:
	comp.exponent = 1;
//WMC	comp.s = 3.3038e-13;
	comp.s = 5.0e-13;
	comp.D = 5.0e-7;
	comp.K = 1.0;
	rsa.component.push_back(comp);

// Dimer:
	comp.exponent = 2;
//WMC	comp.s = 6.1585e-13;
	comp.s = 8.4894e-13;
	comp.D = 4.2447e-7;
	comp.K = 0.5;
	rsa.component.push_back(comp);

// mfem data:
	rsa.rsa_data.id = "Test Data";
	rsa.rsa_data.cell = 1;
	rsa.rsa_data.channel = 1;
	rsa.rsa_data.wavelength = 280;
	rsa.rsa_data.meniscus = 5.8;
	rsa.rsa_data.bottom = 7.2;
	rsa.rsa_data.rpm = 40000;
	rsa.rsa_data.s20w_correction = 1.0;
	rsa.rsa_data.D20w_correction = 1.0;
	rsa.rsa_data.radius.clear();
	rsa.rsa_data.scan.clear();

// initialize times and scans:
	rsa.rsa_data.radius = c0.radius; // initial radius should match final radius
	
	double start_time = 600.0; // first scan after 10 minutes
	for (i=0; i<scans; i++)
	{
		single_scan.conc.clear(); // will be populated later in astfem_rsa class
		single_scan.time = start_time + i * total_time/scans;
		rsa.rsa_data.scan.push_back(single_scan);
	}

	US_Astfem_RSA *astfem; 
	astfem = new US_Astfem_RSA();

	astfem->setParameters(&rsa, c0); // rsa will be populated with the solution on exit
	astfem->calculate();
	return (0);
}
