#ifndef US_GA_INTERACTING_H
#define US_GA_INTERACTING_H

#include "us_femglobal.h"
#include "../include/us_fe_nnls_t.h"

Simulation_values us_ga_interacting_calc(vector <struct mfem_data>, 
					 vector <Solute>,
					 double);

extern void print_ModelSystem(struct ModelSystem *);
extern void print_SimulationParameters(struct SimulationParameters *);
extern void print_AstFemParameters(struct AstFemParameters *);

extern ModelSystem last_model_system;
extern SimulationParameters last_simulation_parameters;

#endif
