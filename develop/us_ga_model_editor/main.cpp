#include "../include/us_ga_model_editor.h"
#include "../include/us_register.h"

void init_constraint(struct constraint *);

int main (int argc, char **argv)
{
	struct ModelSystem ms;
	struct ModelSystemConstraints msc;
	ms.component_vector.resize(1);
	ms.component_vector[0].show_conc = true;
	ms.component_vector[0].show_stoich = 0;
	ms.component_vector[0].show_keq = false;
	ms.component_vector[0].show_koff = false;
	ms.component_vector[0].show_component.clear();
	ms.component_vector[0].shape = "";
	ms.component_vector[0].s = 0.0;
	ms.component_vector[0].D = 0.0;
	ms.component_vector[0].concentration = 1.0;
	ms.component_vector[0].mw = 0.0;
	ms.component_vector[0].c0.radius.clear();
	ms.component_vector[0].c0.concentration.clear();
	ms.component_vector[0].vbar20 = (float) 0.72;
	ms.component_vector[0].sigma = 0.0;
	ms.component_vector[0].delta = 0.0;
	ms.component_vector[0].extinction = 1.0;
	ms.component_vector[0].shape = "not defined";
	ms.component_vector[0].name = "Component 1";
	ms.model = 0;
	ms.component_vector[0].mw = 50000.0;
	ms.component_vector[0].f_f0 = 1.25;
	ms.component_vector[0].s = (ms.component_vector[0].mw * (1.0 - ms.component_vector[0].vbar20 * DENS_20W))
	/ (AVOGADRO * ms.component_vector[0].f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO)
	* ms.component_vector[0].mw * ms.component_vector[0].vbar20 * M_PI * M_PI, 1.0/3.0));
	ms.component_vector[0].D = (R * K20)/(AVOGADRO * ms.component_vector[0].f_f0
	* 9.0 * VISC_20W * M_PI * pow((2.0 * ms.component_vector[0].s * ms.component_vector[0].f_f0
	* ms.component_vector[0].vbar20 * VISC_20W) / (1.0-ms.component_vector[0].vbar20 * DENS_20W), 0.5));

	msc.component_vector_constraints.resize(1);
	msc.assoc_vector_constraints.resize(0);

	msc.component_vector_constraints[0].mw.low  = 40000.0;
	msc.component_vector_constraints[0].mw.high = 60000.0;
	msc.component_vector_constraints[0].mw.fit = true;
	msc.component_vector_constraints[0].concentration.low = (float) 0.8;
	msc.component_vector_constraints[0].concentration.high = (float) 1.2;
	msc.component_vector_constraints[0].concentration.fit = true;
	msc.component_vector_constraints[0].f_f0.low = (float) 1.3;
	msc.component_vector_constraints[0].f_f0.high = (float) 1.8;
	msc.component_vector_constraints[0].f_f0.fit = true;
	init_constraint(&msc.component_vector_constraints[0].vbar20);
	init_constraint(&msc.component_vector_constraints[0].s);
	init_constraint(&msc.component_vector_constraints[0].D);
	init_constraint(&msc.component_vector_constraints[0].sigma);
	init_constraint(&msc.component_vector_constraints[0].delta);
	msc.simpoints = 200;		// number of radial grid points used in simulation
	msc.mesh = 0; 				// 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 
									// 3 = user-selected mesh, 4 = nonuniform constant mesh
	msc.moving_grid = 1; 	// Use moving (1) or fixed time grid (0)
	msc.band_volume = (float) 0.15;

	QApplication a(argc, argv);
  	US_Register *us_register;
	us_register = new US_Register();

	if(us_register->read())      //this is a correct file
	{
		delete us_register;
		US_GAModelEditor *gainit_w;
	  	gainit_w = new US_GAModelEditor(&ms, &msc);
		gainit_w->show();
		a.setMainWidget(gainit_w);
		a.setDesktopSettingsAware(false);
	}
	else     // problem with license (either missing, incorrect or expired)
	{
		a.setMainWidget(us_register);
		a.setDesktopSettingsAware(false);
	}
	return a.exec();
}

void init_constraint(struct constraint *c)
{
		(*c).low  = 0.0;
		(*c).high = 0.0;
		(*c).fit  = false;
}
