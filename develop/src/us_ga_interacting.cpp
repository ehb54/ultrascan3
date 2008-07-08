// this file contains routines for converting the ga produced stack values 
// into models for US_Astfem_RSA::calculate() and running calculate to
// determine residuals

#include <mpi.h>

#include "../include/us_fe_nnls_t.h"
#include "../include/us_ga_interacting.h"
#include "../include/us_astfem_rsa.h"
#include "../include/us_ga.h"
#include "../include/us_ga_stacks.h"

#include <stdio.h>

ModelSystem last_model_system;
SimulationParameters last_simulation_parameters;

void us_ga_interacting_init() 
{
};

static void update_sD(SimulationComponent *sc, struct mfem_data *exp)
{
	QString str;
	sc->s = 	
	    (sc->mw * (1.0 - sc->vbar20 * DENS_20W))
	    / (AVOGADRO * sc->f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO) * sc->mw * sc->vbar20 * M_PI * M_PI, 1.0/3.0))
	    / exp->s20w_correction;
	sc->D = 
	    (R * K20)/
	    (AVOGADRO *
	     sc->f_f0 * 9.0 * VISC_20W * M_PI *
	     pow((2.0 * sc->s * sc->f_f0 * sc->vbar20 * VISC_20W) / 
		 (1.0 - sc->vbar20 * DENS_20W), 0.5)) / exp->D20w_correction;
}

Simulation_values us_ga_interacting_calc(vector <struct mfem_data> experiment, 
					 vector <Solute> solutes,
					 double meniscus_offset)
{
//    printf("rss: at start of us_ga_interacting %lu\n", getrss(0)); fflush(stdout);
  vector <struct mfem_data> org_experiment = experiment;
  vector <struct mfem_data> residuals = experiment;
  SimulationParameters our_simulation_parameters = simulation_parameters;
  ModelSystem our_model_system = model_system;

  us_ga_interacting_init();
//  experiment[0].meniscus += meniscus_offset;
//  printf("mensicus %g %g bottom %g %g\n",
//	 our_simulation_parameters.meniscus , experiment[0].meniscus,
//	 our_simulation_parameters.bottom , experiment[0].bottom); fflush(stdout);
	 
//  our_simulation_parameters.meniscus = experiment[0].meniscus;
//  our_simulation_parameters.bottom = experiment[0].bottom;


  US_Astfem_RSA astfem_rsa(false);

  double rmsd = 0e0;
  unsigned int i, j;
  double k;

  // go through the model_system_constraints & the solute vector to setup the model_system
  {
      unsigned int i;
      unsigned int spos = 0;
      SimulationComponentConstraints *scp;
      AssociationConstraints *acp;
      for (i = 0; i < model_system_constraints.component_vector_constraints.size(); i++) 
      {
	  scp = &model_system_constraints.component_vector_constraints[i];
	  
	  if (scp->vbar20.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 1\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].vbar20 = solutes[spos++].s;
	  }
	  if (scp->mw.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 2\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].mw = solutes[spos++].s;
	  }
	  if (scp->s.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 3\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].s = solutes[spos++].s;
	  }
	  if (scp->D.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 4\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].D = solutes[spos++].s;
	  }
	  if (scp->sigma.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 5\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].sigma = solutes[spos++].s;
	  }
	  if (scp->delta.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 6\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].delta = solutes[spos++].s;
	  }
	  if (scp->concentration.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 7\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].concentration = solutes[spos++].s;
	  }
	  if (scp->f_f0.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 8\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.component_vector[i].f_f0 = solutes[spos++].s;
	  }
      }
      
      for (i = 0; i < model_system_constraints.assoc_vector_constraints.size(); i++) 
      {
	  acp = &model_system_constraints.assoc_vector_constraints[i];
	  
	  if (acp->keq.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 9\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.assoc_vector[i].keq = solutes[spos++].s;
	  }
	  
	  if (acp->koff.fit) 
	  {
	      if (spos >= solutes.size()) 
	      {
		  cerr << "spos out of bound error in ga_interacting 10\n"; 
		  MPI_Abort(MPI_COMM_WORLD, -99);
		  exit(-1);
	      }
	      our_model_system.assoc_vector[i].k_off = solutes[spos++].s;
	  }
      }
      if (spos < solutes.size()) 
      {
	  cerr << "spos unused error in ga_interacting 11\n"; 
	  MPI_Abort(MPI_COMM_WORLD, -99);
	  exit(-1);
      }
  }
// ok, now we have to do final preps... 
// we are exclusive searching (possibly) on mw & f_f0
// use update_sD & use s20_w & d20_w corrections
//
// go through compnents 1 & up
  for(i = 0; i < our_model_system.component_vector.size(); i++) {
      if (our_model_system.component_vector[i].show_stoich > 0)
      { // This species is the dissociating species in a self-associating system
	  our_model_system.component_vector[i].mw =
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw *
	      our_model_system.component_vector[i].show_stoich;
	  our_model_system.component_vector[i].vbar20 =
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].vbar20;
      }
      else if (our_model_system.component_vector[i].show_stoich == -1)
      { // This species is the dissociating species in a 2-component hetero-associating system
// only A+B <-> AB
	  our_model_system.component_vector[i].mw =
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw +
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].mw;
	  float fraction1, fraction2;
	  fraction1 = our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw/
	      our_model_system.component_vector[i].mw;
	  fraction2 = our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].mw/
	      our_model_system.component_vector[i].mw;
	  our_model_system.component_vector[i].vbar20 =
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].vbar20 * fraction1 +
	      our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].vbar20 * fraction2;
      }
      update_sD(&our_model_system.component_vector[i], &experiment[0]);
  }

  
// print_ModelSystem(&our_model_system);
//  print_ModelSystem(&model_system);
//  print_SimulationParameters(&our_simulation_parameters);
//  print_SimulationParameters(&simulation_parameters);
//
//  puts("back from print_SimulationParameters!"); fflush(stdout);

  
// ok, we should be ready to run
  if(0)
  { // special test 
      float mw;
      for (mw = 49950; mw < 50050; mw += 1 ) {
	  rmsd = 0e0;
	  experiment = org_experiment;
	  for (i = 0; i < experiment[0].scan.size(); i++)
	  {
	      for (j = 0; j < experiment[0].radius.size(); j++)
	      {
		  experiment[0].scan[i].conc[j] = 0e0;
	      }
	  }
	  our_model_system = model_system;
	  our_model_system.component_vector[0].mw = mw;
	  
	  for(i = 0; i < our_model_system.component_vector.size(); i++) {
	      if (our_model_system.component_vector[i].show_stoich > 0)
	      { // This species is the dissociating species in a self-associating system
		  our_model_system.component_vector[i].mw =
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw *
		      our_model_system.component_vector[i].show_stoich;
		  our_model_system.component_vector[i].vbar20 =
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].vbar20;
	      }
	      else if (our_model_system.component_vector[i].show_stoich == -1)
	      { // This species is the dissociating species in a 2-component hetero-associating system
// only A+B <-> AB
		  our_model_system.component_vector[i].mw =
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw +
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].mw;
		  float fraction1, fraction2;
		  fraction1 = our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].mw/
		      our_model_system.component_vector[i].mw;
		  fraction2 = our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].mw/
		      our_model_system.component_vector[i].mw;
		  our_model_system.component_vector[i].vbar20 =
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[0]].vbar20 * fraction1 +
		      our_model_system.component_vector[our_model_system.component_vector[i].show_component[1]].vbar20 * fraction2;
	      }
	      update_sD(&our_model_system.component_vector[i], &experiment[0]);
	  }

//	  our_model_system = model_system;
	  our_simulation_parameters.simpoints = 100;
//	  our_model_system.component_vector[0].s = 4.0764e-13;
//	  our_model_system.component_vector[0].D = 7.0649e-7;
//	  our_model_system.component_vector[1].s = 5.3931e-13;
//	  our_model_system.component_vector[1].D = 4.6734e-7;

//	  print_ModelSystem(&our_model_system);
//	  print_SimulationParameters(&our_simulation_parameters);

//	  print_ModelSystem(&model_system);

	  astfem_rsa.setTimeCorrection(true);
	  astfem_rsa.setTimeInterpolation(false);

	  astfem_rsa.calculate(&our_model_system, &our_simulation_parameters, &experiment);

	  printf("points %u scans %u\n", org_experiment[0].radius.size(), org_experiment[0].scan.size()); fflush(stdout);
	  printf("points %u scans %u\n", experiment[0].radius.size(), experiment[0].scan.size()); fflush(stdout);

	  for (i = 0; i < org_experiment[0].scan.size(); i++)
	  {
	      for (j = 0; j < org_experiment[0].radius.size(); j++)
	      {
		  residuals[0].scan[i].conc[j] = org_experiment[0].scan[i].conc[j] - experiment[0].scan[i].conc[j];
		  //	printf("%u %u %g %g %g\n", i, j, org_experiment[0].radius[j], org_experiment[0].scan[i].conc[j], experiment[0].scan[i].conc[j]);
		  rmsd += residuals[0].scan[i].conc[j] * residuals[0].scan[i].conc[j];
	      }
	  }
	  rmsd /= experiment[0].scan.size() * experiment[0].radius.size();
	  printf("%g %g ds1 %g dD1 %g ds2 %g dD2 %g\n", mw, pow((double)rmsd, 0.5),
		 fabs(model_system.component_vector[0].s - our_model_system.component_vector[0].s) / 
		 model_system.component_vector[0].s, 
		 fabs(model_system.component_vector[0].D - our_model_system.component_vector[0].D) / 
		 model_system.component_vector[0].D,
		 fabs(model_system.component_vector[1].s - our_model_system.component_vector[1].s) / 
		 model_system.component_vector[1].s,
		 fabs(model_system.component_vector[1].D - our_model_system.component_vector[1].D) / 
		 model_system.component_vector[1].D
	      );
      }
      MPI_Abort(MPI_COMM_WORLD, -9999);
      exit(-9999);
  }

  for (i = 0; i < experiment[0].scan.size(); i++)
  {
      for (j = 0; j < experiment[0].radius.size(); j++)
      {
	  experiment[0].scan[i].conc[j] = 0e0;
      }
  }
  
  
//  printf("rss: astfem_rsa_calculate %lu\n", getrss(0)); fflush(stdout);
//  puts("astfem_rsa.calculate!"); fflush(stdout);
//  our_simulation_parameters.simpoints = 100;
//  our_model_system = model_system;
//  print_ModelSystem(&our_model_system); fflush(stdout);
//  our_simulation_parameters.moving_grid = 0;
//  print_SimulationParameters(&our_simulation_parameters); fflush(stdout);

  last_model_system = our_model_system;
  last_simulation_parameters = our_simulation_parameters;

  astfem_rsa.setTimeCorrection(true);
  astfem_rsa.setTimeInterpolation(false);

  astfem_rsa.calculate(&our_model_system, &our_simulation_parameters, &experiment);
//  printf("rss: exit astfem_rsa_calculate %lu\n", getrss(0)); fflush(stdout);

  for (i = 0; i < experiment[0].scan.size(); i++)
  {
      for (j = 0; j < (experiment[0].radius.size()); j++)
      {
	  residuals[0].scan[i].conc[j] = org_experiment[0].scan[i].conc[j] - experiment[0].scan[i].conc[j];
	  //	printf("%u %u %g %g %g\n", i, j, org_experiment[0].radius[j], org_experiment[0].scan[i].conc[j], experiment[0].scan[i].conc[j]);
	  rmsd += residuals[0].scan[i].conc[j] * residuals[0].scan[i].conc[j];
      }
  }

  last_residuals = residuals;

  rmsd /= experiment[0].scan.size() * (experiment[0].radius.size());
  
  Simulation_values sv;
  vector<double> no_noise;
  vector<double> variances;
    no_noise.clear();
    sv.solutes = solutes;
    sv.variance = rmsd;
    rmsd = pow((double) rmsd, 0.5);
    //    printf("rmsd %g\n", rmsd); fflush(stdout);
    variances.push_back(sv.variance);
    sv.variances = sv.variances;
    sv.ti_noise = no_noise;
    sv.ri_noise = no_noise;
//    MPI_Abort(MPI_COMM_WORLD, -999);
//    exit(-1);
//    printf("rss: exit us_ga_interacting_calc %lu\n", getrss(0)); fflush(stdout);
    return sv;
}

void print_ModelSystem(struct ModelSystem *s) {
  //  printf("ModelSystem.description            \t%s\n", s->description.ascii());
  printf("+ModelSystem.model                  \t%d\n", s->model);
  printf("ModelSystem.component_vector.size()\t%u\n", s->component_vector.size());
  unsigned int i, j;
  for(i = 0; i < s->component_vector.size(); i++) {
    printf("  -ModelSystem.component_vector[%d].vbar20       \t%e\n", i, s->component_vector[i].vbar20);
    printf("  -ModelSystem.component_vector[%d].mw           \t%e\n", i, s->component_vector[i].mw);
    printf("  +ModelSystem.component_vector[%d].s            \t%e\n", i, s->component_vector[i].s);
    printf("  +ModelSystem.component_vector[%d].D            \t%e\n", i, s->component_vector[i].D);
    //    printf("  -ModelSystem.component_vector[%d].sigma        \t%e\n", i, s->component_vector[i].sigma);
    //    printf("  -ModelSystem.component_vector[%d].delta        \t%e\n", i, s->component_vector[i].delta);
    //    printf("  -ModelSystem.component_vector[%d].extinction   \t%e\n", i, s->component_vector[i].extinction);
    printf("  +ModelSystem.component_vector[%d].concentration\t%e\n", i, s->component_vector[i].concentration);
    printf("  -ModelSystem.component_vector[%d].f_f0         \t%e\n", i, s->component_vector[i].f_f0);
    //    printf("  -ModelSystem.component_vector[%d].show_conc    \t%d\n", i, s->component_vector[i].show_conc);
    //    printf("  -ModelSystem.component_vector[%d].show_keq     \t%d\n", i, s->component_vector[i].show_keq);
    //    printf("  -ModelSystem.component_vector[%d].show_koff    \t%d\n", i, s->component_vector[i].show_koff);
    //    printf("  -ModelSystem.component_vector[%d].show_stoich  \t%d\n", i, s->component_vector[i].show_stoich);
    //    printf("  -ModelSystem.component_vector[%d].show_component.size()\t%u\n", i, s->component_vector[i].show_component.size());
    for(j = 0; j < s->component_vector[i].show_component.size(); j++) {
      //      printf("    -ModelSystem.component_vector[%d].show_component[%d]   \t%u\n", i, j, s->component_vector[i].show_component[j]);
    }
    //    printf("  -ModelSystem.component_vector[%d].shape        \t%s\n", i, s->component_vector[i].shape.ascii());
    //    printf("  -ModelSystem.component_vector[%d].name         \t%s\n", i, s->component_vector[i].name.ascii());
  }
  printf("ModelSystem.assoc_vector.size()    \t%u\n", s->assoc_vector.size());
  for(i = 0; i < s->assoc_vector.size(); i++) {
    printf("  +ModelSystem.assoc_vector[%d].keq              \t%e\n", i, s->assoc_vector[i].keq);
    printf("  +ModelSystem.assoc_vector[%d].k_off            \t%e\n", i, s->assoc_vector[i].k_off);
    //    printf("  -ModelSystem.assoc_vector[%d].units            \t%s\n", i, s->assoc_vector[i].units.ascii());
    printf("  +ModelSystem.assoc_vector[%d].component1       \t%d\n", i, s->assoc_vector[i].component1);
    printf("  +ModelSystem.assoc_vector[%d].component2       \t%d\n", i, s->assoc_vector[i].component2);
    printf("  +ModelSystem.assoc_vector[%d].component3       \t%d\n", i, s->assoc_vector[i].component3);
    printf("  +ModelSystem.assoc_vector[%d].stoichiometry1   \t%u\n", i, s->assoc_vector[i].stoichiometry1);
    printf("  +ModelSystem.assoc_vector[%d].stoichiometry2   \t%u\n", i, s->assoc_vector[i].stoichiometry2);
    printf("  +ModelSystem.assoc_vector[%d].stoichiometry3   \t%u\n", i, s->assoc_vector[i].stoichiometry3);
  }
}

void print_SimulationParameters(struct SimulationParameters *s) {
  unsigned int i;
  printf("SimulationParameters.speed_step.size()\t%u\n", s->speed_step.size());
  for(i = 0; i < s->speed_step.size(); i++) {
    printf("  +SimulationParameters.speed_step[%d].duration_hours     \t%u\n", i, s->speed_step[i].duration_hours);
    printf("  +SimulationParameters.speed_step[%d].duration_minutes   \t%u\n", i, s->speed_step[i].duration_minutes);
    printf("  +SimulationParameters.speed_step[%d].delay_hours        \t%u\n", i, s->speed_step[i].delay_hours);
    printf("  +SimulationParameters.speed_step[%d].delay_minutes      \t%e\n", i, s->speed_step[i].delay_minutes);
    printf("  +SimulationParameters.speed_step[%d].scans              \t%u\n", i, s->speed_step[i].scans);
    printf("  +SimulationParameters.speed_step[%d].acceleration       \t%u\n", i, s->speed_step[i].acceleration);
    printf("  +SimulationParameters.speed_step[%d].rotorspeed         \t%u\n", i, s->speed_step[i].rotorspeed);
    printf("  +SimulationParameters.speed_step[%d].acceleration_flag  \t%d\n", i, s->speed_step[i].acceleration_flag);
  }
  printf("+SimulationParameters.simpoints        \t%u\n", s->simpoints);
  printf("+SimulationParameters.mesh             \t%u\n", s->mesh);
  printf("+SimulationParameters.moving_grid      \t%d\n", s->moving_grid);
  printf("+SimulationParameters.radial_resolution\t%e\n", s->radial_resolution);
  printf("+SimulationParameters.meniscus         \t%e\n", s->meniscus);
  printf("+SimulationParameters.bottom           \t%e\n", s->bottom);
  //  printf("-SimulationParameters.rnoise           \t%e\n", s->rnoise);
  //  printf("-SimulationParameters.inoise           \t%e\n", s->inoise);
  //  printf("-SimulationParameters.rinoise          \t%e\n", s->rinoise);
}

void print_AstFemParameters(struct AstFemParameters *s) {
  unsigned int i;
  printf("AstFemParameters.simpoints           \t%u\n", s->simpoints);
  printf("AstFemParameters.s.size()            \t%u\n", s->s.size());
  for (i = 0; i < s->s.size(); i++) {
    printf("  AstFemParameters.s[%u]           \t%e\n", i, s->s[i]);
    printf("  AstFemParameters.D[%u]           \t%e\n", i, s->D[i]);
//    printf("  AstFemParameters.keq[%u]         \t%e\n", i, s->keq[i]);
//    printf("  AstFemParameters.koff[%u]        \t%e\n", i, s->koff[i]);
//    printf("  AstFemParameters.n[%u]           \t%u\n", i, s->n[i]);
  }
  printf("AstFemParameters.dt                  \t%e\n", s->dt);
  printf("AstFemParameters.time_steps          \t%u\n", s->time_steps);
  printf("AstFemParameters.omega_s             \t%e\n", s->omega_s);
  printf("AstFemParameters.start_time          \t%e\n", s->start_time);
//  printf("AstFemParameters.meniscus            \t%e\n", s->meniscus);
//  printf("AstFemParameters.bottom              \t%e\n", s->bottom);
//  printf("AstFemParameters.mesh                \t%u\n", s->mesh);
//  printf("AstFemParameters.moving_grid         \t%d\n", s->moving_grid);
//  printf("AstFemParameters.acceleration        \t%d\n", s->acceleration);
//  printf("AstFemParameters.model               \t%u\n", s->model);
}
