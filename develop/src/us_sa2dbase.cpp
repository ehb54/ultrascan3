// c++ class implementation for SA2D implementation version 0.3
// Written by E. Brookes 2005. ebrookes@cs.utsa.edu
// added boolean pointer to fit_status flag in constructors 6/24 -b.d-

#include <list>
#include "../include/us_sa2dbase.h"
#include "../include/us_sa2d.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <qstring.h>

#define DEBUG
#define SLIST

SA2D::SA2D(bool *fit_status) {
  this->fit_status = fit_status;
};

SA2D::~SA2D() {};

void SA2D::set_params(bool *fit_status,
		      Simulation_values (*simulation_routine)(Simulation_values),
		      unsigned int s_gridsize, unsigned int k_gridsize,
		      double s_min, double s_max,
		      double f_ratio_min, double f_ratio_max) {
  this->fit_status = fit_status;
  max_solutes = s_gridsize * k_gridsize;
  this->fit_status = fit_status;
  this->simulation_routine = simulation_routine;
  this->s_gridsize = s_gridsize;
  this->k_gridsize = k_gridsize;
  this->s_min = s_min;
  this->s_max = s_max;
  this->f_ratio_min = f_ratio_min;
  this->f_ratio_max = f_ratio_max;
  use_sa2d_w = 0;
}

void SA2D::set_params(bool *fit_status,
		      void *us_sa2d_w,
		      unsigned int s_gridsize, unsigned int k_gridsize,
		      double s_min, double s_max,
		      double f_ratio_min, double f_ratio_max) {
  max_solutes = s_gridsize * k_gridsize;
  this->fit_status = fit_status;
  this->us_sa2d_w = us_sa2d_w;
  this->s_gridsize = s_gridsize;
  this->k_gridsize = k_gridsize;
  this->s_min = s_min;
  this->s_max = s_max;
  this->f_ratio_min = f_ratio_min;
  this->f_ratio_max = f_ratio_max;
  use_sa2d_w = 1;
}

SA2D::SA2D(bool *fit_status, Simulation_values (*simulation_routine)(Simulation_values),
	   unsigned int s_gridsize, unsigned int k_gridsize,
	   double s_min, double s_max,
	   double f_ratio_min, double f_ratio_max) {
  set_params(fit_status, simulation_routine, s_gridsize, k_gridsize, s_min, s_max, f_ratio_min, f_ratio_max);
}

SA2D::SA2D(bool *fit_status, void *us_sa2d_w,
	   unsigned int s_gridsize, unsigned int k_gridsize,
	   double s_min, double s_max,
	   double f_ratio_min, double f_ratio_max) {
  set_params(fit_status, us_sa2d_w, s_gridsize, k_gridsize, s_min, s_max, f_ratio_min, f_ratio_max);
}

int SA2D::uniform_run(unsigned int steps) {
  return uniform_run(steps, 0);
}

int SA2D::uniform_run(unsigned int steps, bool use_iterative) {
  // runs the shifting uniform grid method for 'int' steps, 0 steps is a single fixed grid,
  // returns non zero if there is an error
  
  unsigned int s_partition = s_gridsize;   // (int)floor(sqrt(max_solutes));
  unsigned int f_partition = k_gridsize;   // s_partition;
  
  unsigned int i, step_s, step_k, m, n;
  double inc_s, inc_k;
  double s, k, use_s, use_k;
  vector<Solute> saved_sv;
  Solute solute;
  Simulation_values sv;
  int runs_since_merge = 0;
  bool any_change = 1;
  vector<Solute> prev_solutes;
  bool use_merge = 1;
  unsigned int sv_added_count;
  if(use_iterative) {
    use_merge = 0;
  }
  
#if defined(DEBUG)
  puts("SA2D::uniform_run");
  printf("steps %d\n", steps);
  printf("s spacing %.12g s_partition %d\n"
	 "f/f0 spacing %.12g f_partition %d\n",
	 (s_max - s_min) / (s_partition - 1), s_partition,
	 (f_ratio_max - f_ratio_min) / (f_partition - 1), f_partition);
#endif
  
  if(f_partition <= 1 || s_partition <= 1 || (s_partition * f_partition) > max_solutes) {
    sprintf(last_error, "bad partition size(s) %d %d\n",  s_partition, f_partition);
		QString str; 
		str.sprintf("bad partition size(s) %d %d %d\n",  s_partition, f_partition, max_solutes);
		cout<< str;
    return(-1);
  }
  
  while(any_change) {
    for(step_s = 0; step_s < steps; step_s++) {
      for(step_k = 0; step_k < steps; step_k++) {
	
	inc_s = step_s * (s_max - s_min) / ((s_partition - 1) * steps);
	inc_k = step_k * (f_ratio_max - f_ratio_min) / ((f_partition - 1) * steps);
#if defined(DEBUG)
	printf("step_s %d step_k %d inc_s %g inc_k %g\n", step_s, step_k, inc_s, inc_k);
#endif    
	i = 0;
	for(m = 0; m < s_partition; m++) {
	  s = s_min + m * (s_max - s_min) / (s_partition - 1);
	  for(n = 0; n < f_partition; n++) {
	    k = f_ratio_min + n * (f_ratio_max - f_ratio_min) / (f_partition - 1);
	    
	    use_s = s + inc_s;
            use_k = k + inc_k;
	    if(use_s > s_max) {
	      use_s -= (s_max - s_min);
	    }
	    if(use_k > f_ratio_max) {
	      use_k -= (f_ratio_max - f_ratio_min);
	    }
	    solute.s = use_s;
	    solute.k = use_k;
	    // clipping
	    if(use_s <= s_max && use_s >= s_min &&
	       use_k <= f_ratio_max && use_k >= f_ratio_min) {
	      sv.solutes.push_back(solute);
	    }
	    //	    sv.solutes.push_back(solute);
	    //	  printf("solute %d %g %g\n", i++, use_s, use_k);
	  }
	}
	{
	  unsigned int i;
	  for(i = 0; i < prev_solutes.size(); i++) {
	    if(find(sv.solutes.begin(), sv.solutes.end(), prev_solutes[i]) == sv.solutes.end()) {
#if defined(DEBUG)
	      printf("adding previous solute %d\n",i);
#endif
	      sv.solutes.push_back(prev_solutes[i]);
	    }
	  }
	}
	    
	
	// run on this grid
#if defined(DEBUG)
	printf("mfem start!\n");
#endif
	if(!fit_status) {
	  return 0;
	}
	if(use_sa2d_w) {
	  sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
	} else {
	  sv = simulation_routine(sv);
	}
	if(!fit_status) {
	  return 0;
	}
#if defined(DEBUG)
	printf("fitness = %g\n", sv.variance);
	fflush(stdout);
#endif
	variance = sv.variance;
	
	
	sv_added_count = 0;
	for(i = 0; i < sv.solutes.size(); i++) {
	  if(find(saved_sv.begin(), saved_sv.end(), sv.solutes[i]) == saved_sv.end()) {
	    sv_added_count++;
	  }
	}
#if defined(DEBUG)
	if(!use_merge && saved_sv.size() + sv_added_count > max_solutes) {
		printf("note - intermediate solutes large vector %d > %d\n", 
		    (int) ( saved_sv.size() + sv_added_count ), max_solutes);
	}
#endif
	if(use_merge && saved_sv.size() + sv_added_count > max_solutes) {
	  // Another idea is to add individual solutes from the last run
	  // upto the max_solutes size, but I am uncomfortable about splitting
	  // a solute solution, maybe add upto the size, and then add the full
	  // solute solution again ?
	  
#if defined(DEBUG)
		printf("merging size due to large vector %d > %d\n", 
		    (int) ( saved_sv.size() + sv_added_count ), max_solutes);
		printf("merge mfem start!\n");
#endif
	  
	  sv.solutes = saved_sv;
	  if(!fit_status) {
	    return 0;
	  }
	  if(use_sa2d_w) {
	    sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
	  } else {
	    sv = simulation_routine(sv);
	  }
	  if(!fit_status) {
	    return 0;
	  }
	  variance = sv.variance;
	  
	  fflush(stdout);
	  
	  saved_sv = sv.solutes;
	  
#if defined(DEBUG)
	  printf("fitness = %g\n", sv.variance);
	  printf("size after merge %d\n", (int) saved_sv.size() );
#endif
	  runs_since_merge = 1;
	}
	
	// add non-zero solutes into vector SV
	{
	  unsigned int i;
	  for(i = 0; i < sv.solutes.size(); i++) {
	    if(find(saved_sv.begin(), saved_sv.end(), sv.solutes[i]) == saved_sv.end()) {
	      saved_sv.push_back(sv.solutes[i]);
	    }
	  }
	  runs_since_merge++;
	}
	// SV & L have all non zero solutes so far
	sv.solutes.clear();
      } // step_k
    } // step_s
    
    if(runs_since_merge > 1) {
      // need to do a final merge
#if defined(DEBUG)
      puts("final merge");
#endif
      
      sv.solutes = saved_sv;
      if(!fit_status) {
	return 0;
      }
      if(use_sa2d_w) {
	sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
      } else {
	sv = simulation_routine(sv);
      }
      if(!fit_status) {
	return 0;
      }
      variance = sv.variance;
      
      saved_sv = sv.solutes;
      
#if defined(DEBUG)
      printf("fitness = %g\n", sv.variance);
      printf("size after merge %d\n", (int) saved_sv.size());
      fflush(stdout);
#endif
    }
    solutes = saved_sv;
#if defined(SLIST)
    {
      unsigned int i;
      vector<Solute> solutes = get_solutes();
      puts("s\tk\tc");
      for(i = 0; i < solutes.size(); i++) {
	printf("%d\t%g\t%g\t%g\n",
	       i,
	       solutes[i].s,
	       solutes[i].k,
	       solutes[i].c);
      }
    }
#endif
    if(steps == 1 || !use_iterative) {
      any_change = 0;
    } else {
      unsigned int i;
      vector<Solute> solutes = get_solutes();
      if(solutes.size() == prev_solutes.size()) {
	any_change = 0;
	for(i = 0; i < solutes.size(); i++) {
	  if(find(solutes.begin(), solutes.end(), prev_solutes[i]) == solutes.end()) {
	    any_change = 1;
	    break;
	  }
	}
      }
      if(any_change) {
	prev_solutes = solutes;
	sv.solutes.clear();
      }
    }
#if defined(DEBUG)
    printf("any_change %d\n", any_change);
#endif
  }
  return 0;
}  

static list<double> vectorsolute_to_listdouble(vector<Solute> SV) {
  unsigned int i;
  list<double> L;
  for(i = 0; i < SV.size(); i++) {
    L.push_back(SV[i].s);
    L.push_back(SV[i].k);
  }
  return L;
}

int SA2D::local_uniform_run(unsigned int steps, double initial_scaling, double scaling) {
  // zoom in on non-zero vectors
  // we take a grid about each non-zero and run a big nnls & iterate
  unsigned int i, j;

  list<double> L;
  // ok, It will really be cleaner when I rewrite this using solute vectors,
  // but this is a version when I was using lists, so I convert the solute vector
  // into a list
  for(i = 0; i < solutes.size(); i++) {
    L.push_back(solutes[i].s);
    L.push_back(solutes[i].k);
  }

  list<double>::iterator Li;
  double s, k;
  list<double> L2;
  list<double>::iterator L2i;

  Solute solute;
  vector<Solute> SV;
  Simulation_values sv;
  
  list<Solute> checklist;
  list<Solute>::iterator checklisti;
  unsigned int iter;
  int size_per, root_size_per;
  double start_s, start_k;
  double delta_s = (s_max - s_min) / (s_gridsize - 1);
  double delta_k = (f_ratio_max - f_ratio_min) / (k_gridsize - 1);

#if defined(DEBUG)
  printf("partition iteration steps %d scaling factors initial %g afterwards %g\n"
	 "original delta_s %g delta_k %g\n", 
	 steps,
	 initial_scaling,
	 scaling,
	 delta_s, delta_k);
#endif

  delta_s *= initial_scaling;
  delta_k *= initial_scaling;

  for(iter = 0; iter < steps; iter++) {
    checklist.clear();
    L2.clear();
    size_per = (2 * (max_solutes - L.size() / 2) / L.size());
    root_size_per = (int) sqrt(1e0 * size_per);
    start_s = - delta_s * (root_size_per - 1) / 2;
    start_k = - delta_k * (root_size_per - 1) / 2;
    
    if(root_size_per <= 1) {
      sprintf(last_error, "Not enough memory to handle the number of non-zero solutes after %d steps completed.\n", iter);
		QString str;
      str.sprintf("Not enough memory to handle the number of non-zero solutes after %d steps completed.\n", iter);
		cout << str << endl;
      return(-1);
    }

#if defined(DEBUG)
    printf("max_solutes %d L.size() %d\n"
	   "size per point %d root_size_per %d\n"
	   "start_s %g start_k %g\n"
	   "delta_s %g delta_k %g\n", 
	   max_solutes,
	   (int) L.size(),
	   size_per, root_size_per,
	   start_s, start_k,
	   delta_s, delta_k);
#endif
    
    sv.solutes.clear();
    Li = L.begin();
    while(Li != L.end()) {
      solute.s = s = *Li; Li++;
      solute.k = k = *Li; Li++;
      if(find(checklist.begin(), checklist.end(), solute) == checklist.end()) {
	// not already in list
	checklist.push_back(solute);
	sv.solutes.push_back(solute);
	L2.push_back(solute.s);
	L2.push_back(solute.k);
#if defined(DEBUG)
	printf("main solute %d %g %g\n", (int) sv.solutes.size(), solute.s, solute.k);
      } else {
	printf("main skipped solute %g %g\n", solute.s, solute.k);
#endif
      }
      
      solute.s = s + start_s;
      for(i = 0; i < (unsigned int)root_size_per; i++, solute.s += delta_s) {
	if(solute.s >= s_min && solute.s <= s_max) {
	  solute.k = k + start_k;
	  for(j = 0; j < (unsigned int)root_size_per; j++, solute.k += delta_k) {
	    if(solute.k >= f_ratio_min && solute.k <= f_ratio_max) {
	      if(find(checklist.begin(), checklist.end(), solute) == checklist.end()) {
		// not already in list
		checklist.push_back(solute);
		sv.solutes.push_back(solute);
		L2.push_back(solute.s);
		L2.push_back(solute.k);
#if defined(DEBUG)
		printf("solute %d %g %g\n", (int) sv.solutes.size(), solute.s, solute.k);
#endif
	      }
	    }
	  }
	}
      }
    }
    Li = L.begin();
    while(Li != L.end()) {
      solute.s = s = *Li; Li++;
      solute.k = k = *Li; Li++;
      if(find(checklist.begin(), checklist.end(), solute) == checklist.end()) {
	sprintf(last_error, "!!missing solute %g %g\n", solute.s, solute.k);
      }
    }
    
#if defined(DEBUG)
    printf("iteration mfem start %d!\n", iter);
    fflush(stdout);
#endif
    if(!fit_status) {
      return 0;
    }
    if(use_sa2d_w) {
      sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
    } else {
      sv = simulation_routine(sv);
    }
    if(!fit_status) {
      return 0;
    }
#if defined(DEBUG)
    printf("fitness = %g\n", sv.variance);
    fflush(stdout);
#endif
    
    L.clear();
    SV.clear();
    delta_s *= scaling;
    delta_k *= scaling;
    L = vectorsolute_to_listdouble(sv.solutes);
    SV = sv.solutes;
    solutes = sv.solutes;
    variance = sv.variance;
  }
#if defined(SLIST)
 {
   unsigned int i;
   vector<Solute> solutes = get_solutes();
   puts("s\tk\tc");
   for(i = 0; i < solutes.size(); i++) {
     printf("%d\t%g\t%g\t%g\n",
	    i,
	    solutes[i].s,
	    solutes[i].k,
	    solutes[i].c);
   }
 }
#endif
  return 0;
}

#define DEFAULT_SOLUTE_ROUNDING 3

static double floorn(double d, double e, int n) {
  double m = pow(1e0 * 10,n-1e0) / e;
  d = floor(d * m) / m;
  return d;
}

int SA2D::local_random_run(unsigned int steps, double parameter) {
  // here we take each non-zero vector and 
  // a bunch of random vectors centered at each previous non-zero and nnls
  // then, iterate

  unsigned int i;

  list<double> L;
  // ok, It will really be cleaner when I rewrite this using solute vectors,
  // but this is a version when I was using lists, so I convert the solute vector
  // into a list
  for(i = 0; i < solutes.size(); i++) {
    L.push_back(solutes[i].s);
    L.push_back(solutes[i].k);
  }
  int solute_rounding = DEFAULT_SOLUTE_ROUNDING; // ok, this is to round random numbers to a reasonable tolerance
                                                 // so we keep points spaced out a bit

  list<double> prevL;
  list<double>::iterator Li;
  
  double s, k;
  vector<Solute> SV;
  Simulation_values sv;
  list<double> L2;
  list<double>::iterator L2i;
  
  Solute solute;
  list<Solute> checklist;
  list<Solute>::iterator checklisti;
  unsigned int iter;
  int size_per;
  double sel, ds, dk;

#if defined(DEBUG)
  printf("local_random_run steps %d\n"
	 "parameter %g\n",
	 steps,
	 parameter);
#endif

  for(iter = 0; iter < steps; iter++) {
    checklist.clear();
    L2.clear();
    size_per = (max_solutes - prevL.size()) / (L.size() / 2);
    
#if defined(DEBUG)
    printf("max_solutes = %d L.size()/2 %d size per point %d\n",
	   max_solutes,
	   (int) L.size() / 2,
	   size_per);
#endif
    
    sv.solutes.clear();
    
    Li = L.begin();
    while(Li != L.end()) {
      solute.s = s = *Li; Li++;
      solute.k = k = *Li; Li++;
      if(find(checklist.begin(), checklist.end(), solute) == checklist.end()) {
	// not already in list (this is possible!)
	checklist.push_back(solute);
	sv.solutes.push_back(solute);
	L2.push_back(solute.s);
	L2.push_back(solute.k);
#if defined(DEBUG)
	printf("solute %d %g %g\n", (int) sv.solutes.size(), solute.s, solute.k);
      } else {
	printf("main skipped solute %g %g\n", solute.s, solute.k);
#endif
      }
      
      for(i = 1; i < (unsigned int)size_per; i++) {
	do {
	  sel = rand()/(double)RAND_MAX;
	  if(sel < .6) {
	    ds = (- log(1e0 - (rand()/(double)RAND_MAX))) * parameter;
	    if((rand()/(double)RAND_MAX) < .5) {
	      ds = -ds;
	    }
	  } else {
	    ds = 0;
	  }
	  if(sel > .4) {
	    dk = (- log(1e0 -  (rand()/(double)RAND_MAX))) * parameter;
	    if((rand()/(double)RAND_MAX) < .5) {
	      dk = -dk;
	    }
	  } else {
	    dk = 0;
	  }

	  solute.s += ds * 1e-13;
	  solute.k += dk;
	  solute.s = floorn(solute.s, 1e-13, solute_rounding);
	  solute.k = floorn(solute.k, 1, solute_rounding);
	  // clip
	  if(solute.k < f_ratio_min) {
	    solute.k = f_ratio_min;
	  } else {
	    if(solute.k > f_ratio_max) {
	      solute.k = f_ratio_max;
	    }
	  }
	  if(solute.s < s_min) {
	    solute.s = s_min;
	  } else {
	    if(solute.s > s_max) {
	      solute.s = s_max;
	    }
	  }
	} while(find(checklist.begin(), checklist.end(), solute) != checklist.end());

	checklist.push_back(solute);
	sv.solutes.push_back(solute);
	L2.push_back(solute.s);
	L2.push_back(solute.k);
#if defined(DEBUG)
	printf("solute %d %g %g\n", (int) sv.solutes.size(), solute.s, solute.k);
#endif
      }
    }
    Li = L.begin();
    while(Li != L.end()) {
      solute.s = s = *Li; Li++;
      solute.k = k = *Li; Li++;
      if(find(checklist.begin(), checklist.end(), solute) == checklist.end()) {
	sprintf(last_error, "!!missing solute %g %g\n", solute.s, solute.k);
      }
    }
    
#if defined(DEBUG)
    printf("local random mfem start %d!\n", iter);
    fflush(stdout);
#endif    
    if(!fit_status) {
      return 0;
    }
    if(use_sa2d_w) {
      sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
    } else {
      sv = simulation_routine(sv);
    }
    if(!fit_status) {
      return 0;
    }
#if defined(DEBUG)
    printf("fitness = %g\n", sv.variance);
    fflush(stdout);
#endif    
    L = vectorsolute_to_listdouble(sv.solutes);
    SV = sv.solutes;
    solutes = SV;
    variance = sv.variance;
  }
#if defined(SLIST)
 {
   unsigned int i;
   vector<Solute> solutes = get_solutes();
   puts("s\tk\tc");
   for(i = 0; i < solutes.size(); i++) {
     printf("%d\t%g\t%g\t%g\n",
	    i,
	    solutes[i].s,
	    solutes[i].k,
	    solutes[i].c);
   }
 }
#endif
  return 0;
}

int SA2D::regularize(double alpha) {
  // take a set of vectors, compute regularized fitness
  // delete minimum concentration, repeat until regularized fitness goes up

  unsigned int i;
  double min_c;
  int min_c_pos;
  double fitness;
  double reg_fitness = 1e99;
  vector<Solute> prev_solutes;
  vector<Solute_vector> solute_vectors;
  Solute_vector solute_vector;
  Solute solute;
  Simulation_values sv;

  do {
    sv.solutes = solutes;
    if(!fit_status) {
      return 0;
    }
    if(use_sa2d_w) {
      sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
    } else {
      sv = simulation_routine(sv);
    }
    if(!fit_status) {
      return 0;
    }
    prev_solutes = sv.solutes;
    fitness = sv.variance;
    solutes.clear();
    min_c = 1e99;
    min_c_pos = -1;
    for(i = 0; i < prev_solutes.size(); i++) {
      if(prev_solutes[i].c < min_c) {
	min_c = prev_solutes[i].c;
	min_c_pos = i;
      }
    }
	
    for(i = 0; i < prev_solutes.size(); i++) {
      if(prev_solutes[i].c > 0) {
	solutes.push_back(prev_solutes[i]);
      }
    }
    reg_fitness = pow(sqrt(fitness) * (1e0 + (1e0 - alpha) * solutes.size()),2e0);
    solute_vector.solutes = solutes;
    solute_vector.fitness = fitness;
    solute_vector.reg_fitness = reg_fitness;
    solute_vectors.push_back(solute_vector);
#if defined(DEBUG)
    printf("solutes %d fitness %g regularized %g\n", (int) solutes.size(), fitness, reg_fitness);
    printf("min_c_pos %d %g\n", min_c_pos, min_c);
#endif
    
    // now remove the mininum one
    solutes.clear();
    for(i = 0; i < prev_solutes.size(); i++) {
      if(prev_solutes[i].c > 0 &&
	 i != (unsigned) min_c_pos) {
	solutes.push_back(prev_solutes[i]);
      }
    }
  } while(solutes.size() > 0);

  double min_reg_fitness = 1e99;
  int min_reg_fitness_pos = 0;
  for(i = 0; i < solute_vectors.size(); i++) {
    if(solute_vectors[i].reg_fitness < min_reg_fitness) {
      min_reg_fitness = solute_vectors[i].reg_fitness;
      min_reg_fitness_pos = i;
    }
  }
  sv.solutes = solute_vectors[min_reg_fitness_pos].solutes;
  if(use_sa2d_w) {
    sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
  } else {
    sv = simulation_routine(sv);
  }
  //  solutes = solute_vectors[min_reg_fitness_pos].solutes;
  //  variance = solute_vectors[min_reg_fitness_pos].fitness;
  solutes = sv.solutes;
  variance = sv.variance;
#if defined(SLIST)
 {
   unsigned int i;
   vector<Solute> solutes = get_solutes();
   puts("s\tk\tc");
   for(i = 0; i < solutes.size(); i++) {
     printf("%d\t%g\t%g\t%g\n",
	    i,
	    solutes[i].s,
	    solutes[i].k,
	    solutes[i].c);
   }
 }
#endif
  return 0;
}

int SA2D::coalesce(double distance, double concentration) {
  unsigned int i, j;
  vector<Solute> next_solutes;
  list<double> return_list;
  int more_coalesce = 1;
 // double min_d[solutes.size()], this_d;
 double *min_d, this_d;
 min_d = new double [solutes.size()];
  Solute solute;
 // int min_solute_number[solutes.size()];
 int *min_solute_number;
  min_solute_number = new int [solutes.size()];
//  int coalesced[solutes.size()];
int *coalesced;
coalesced = new int [solutes.size()];

#if defined(DEBUG)
  puts("coalesce");
  printf("size of vector = %d\n", (int) solutes.size());
#endif

  if(concentration) {
    vector<Solute> solutes2;
    for(i = 0; i < solutes.size(); i++) {
      if(solutes[i].c > concentration) {
	solutes2.push_back(solutes[i]);
      }
    }
    solutes = solutes2;
  }
    
  while(more_coalesce) {
    more_coalesce = 0;
    for(i = 0; i < solutes.size(); i++) {
      min_d[i] = 1e99;
      coalesced[i] = 0;
      for(j = 0; j < solutes.size(); j++) {
	if(j != i) {
	  this_d = sqrt(pow(solutes[i].s * 1e13 - solutes[j].s * 1e13,2e0) + 
			pow(solutes[i].k - solutes[j].k,2e0));
	  if(this_d < min_d[i]) {
	    min_solute_number[i] = j;
	    min_d[i] = this_d;
	  }
	}
      }
    }
    for(i = 0; i < solutes.size(); i++) {
      if(!coalesced[i]) {
#if defined(DEBUG)
	printf("solute %d [%g,%g] closest vector %d [%g,%g] dist %g\n",
	       i, solutes[i].s, solutes[i].k, min_solute_number[i], 
	       solutes[min_solute_number[i]].s, solutes[min_solute_number[i]].k, min_d[i]);
#endif
	if(min_d[i] < distance) {
	  // coalesce
#if defined(DEBUG)
	  puts("coalesce this solute");
#endif
	  solute.s = (solutes[i].s + solutes[min_solute_number[i]].s) / 2e0;
	  solute.k = (solutes[i].k + solutes[min_solute_number[i]].k) / 2e0;
	  solute.c = (solutes[i].c + solutes[min_solute_number[i]].c);
	  next_solutes.push_back(solute);
	  coalesced[min_solute_number[i]] = 1;
	  more_coalesce = 1;
	} else {
	  next_solutes.push_back(solutes[i]);
	}
      }
    }
    solutes.clear();
    solutes = next_solutes;
    next_solutes.clear();
  }
#if defined(DEBUG)
  puts("coalesced solutes before NNLS");
  for(i = 0; i < solutes.size(); i++) {
    printf("%g %g %g\n",  solutes[i].s, solutes[i].k, solutes[i].c);
  }
#endif

  // set the final concentrations & variance
  Simulation_values sv;
  sv.solutes = solutes;
  if(!fit_status) {
    return 0;
  }
  if(use_sa2d_w) {
    sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
  } else {
    sv = simulation_routine(sv);
  }
  if(!fit_status) {
    return 0;
  }
  variance = sv.variance;
  solutes = sv.solutes;
  delete [] min_d;
  delete [] min_solute_number;
  delete [] coalesced; 
#if defined(SLIST)
 {
   unsigned int i;
   vector<Solute> solutes = get_solutes();
   puts("s\tk\tc");
   for(i = 0; i < solutes.size(); i++) {
     printf("%d\t%g\t%g\t%g\n",
	    i,
	    solutes[i].s,
	    solutes[i].k,
	    solutes[i].c);
   }
 }
#endif
  return 0;
}

int SA2D::clip_data(unsigned int steps)
{
// clip the lowest concentration element from the solutes vector and return 
// n-1 solutes. Repeat this for as many steps as are given in parameter list 
  unsigned int i, item = 0;
  vector<Solute> next_solutes;
//	list<double> return_list;
  Solute solute;
  vector<Solute> solutes2;
  while (steps > 0) {
    solutes2.clear();
    double min_c=1.0e99;
    for(i = 0; i < solutes.size(); i++) {
      if (solutes[i].c < min_c){
	item = i;
	min_c = solutes[i].c;
      }
    }
    for(i = 0; i < solutes.size(); i++) {
      if (i != item) {
	solutes2.push_back(solutes[i]);
      }
    }
    solutes = solutes2;
    steps--;
  }
  
  // set the final concentrations & variance
  Simulation_values sv;
  sv.solutes = solutes;
  if(!fit_status) {
    return 0;
  }
  if(use_sa2d_w) {
    sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
  } else	{
    sv = simulation_routine(sv);
  }
  if(!fit_status)	{
    return 0;
  }
  variance = sv.variance;
  solutes = sv.solutes;
#if defined(SLIST)
  {
    unsigned int i;
    vector<Solute> solutes = get_solutes();
    puts("s\tk\tc");
    for(i = 0; i < solutes.size(); i++) {
      printf("%d\t%g\t%g\t%g\n",
	     i,
	     solutes[i].s,
	     solutes[i].k,
	     solutes[i].c);
    }
  }
#endif
  return 0;
}


double SA2D::get_variance() {
  return variance;
}

vector<double> SA2D::get_s() {
  vector<double> s;
  unsigned int i;
  for(i = 0; i < solutes.size(); i++) {
    s.push_back(solutes[i].s);
  }
  return s;
}

vector<double> SA2D::get_f_ratio() {
  vector<double> k;
  unsigned int i;
  for(i = 0; i < solutes.size(); i++) {
    k.push_back(solutes[i].k);
  }
  return k;
}

static float Rb  = 8.314e7;
static float T   = (float) K20;
static float eta = (float) 0.01;
static float N   = (float) 6.022e23;
static float rho = 1.0;

static float D_calc(float s, float vbar, float k) {
// vbar for DNA .55, proteins .69 to .81 usually .72-.74
// k 1 to 4
  return (Rb*T)/(N*k*6.0*eta*M_PI*pow((9.0*s*k*vbar*eta)/(2.0*(1.0-vbar*rho)), 0.5));
}

vector<double> SA2D::get_D(double vbar) {
  vector<double> D;
  unsigned int i;
  for(i = 0; i < solutes.size(); i++) {
    D.push_back(D_calc(solutes[i].s, vbar, solutes[i].k));
  }
  return D;
}

Simulation_values SA2D::get_simulation_values() {
  Simulation_values sv;
  sv.variance = variance;
  sv.solutes = solutes;
  return(sv);
}

void SA2D::set_simulation_values(Simulation_values sv) {
  variance = sv.variance;
  solutes = sv.solutes;
}

vector<Solute> SA2D::get_solutes() {
  // protect out internal vector by copying it
  vector<Solute> ret_vector = solutes;
  return ret_vector;
}

unsigned int SA2D::get_solutes_size() {
  return solutes.size();
}
/*
void SA2D::fprint_solutes(FILE *out) {
  // protect out internal vector by copying it
  unsigned int i;
  fputs("fprint_solutes: s\tk\tc\n", out);
  for(i = 0; i < solutes.size(); i++) {
    fprintf(out, "\t%d\t%g\t%g\t%g\n",
	    i,
	    solutes[i].s,
	    solutes[i].k,
	    solutes[i].c);
  }
}
*/
void SA2D::set_solutes(vector<Solute> solutes) {
  this->solutes = solutes;
}

int SA2D::run_simulation() {
  Simulation_values sv;
  sv.solutes = solutes;
  if(!fit_status) {
    return 0;
  }
  if(use_sa2d_w) {
    sv = ((US_SA2D_W *)us_sa2d_w)->Simulation_Routine(sv);
  } else {
    sv = simulation_routine(sv);
  }
  if(!fit_status) {
    return 0;
  }
  solutes = sv.solutes;
  variance = sv.variance;
  return 0;
}

#define MAX_INPUT_LIN_LENGTH 4096
#define MAX_INPUT_LIN_OPTS 128

void sa2d_script(Simulation_values (*simulation_routine)(Simulation_values), char *filename) {
  FILE *scriptfile = fopen(filename, "r");
  if(!scriptfile) {
    fprintf(stderr, "sa2d script file open error, can't open \"%s\" for reading\n", filename);
		QString str;
		str.sprintf("sa2d script file open error, can't open \"%s\" for reading\n", filename);
		cout << str;
    exit(-1);
  }
  char line[MAX_INPUT_LIN_LENGTH];
  char *token[MAX_INPUT_LIN_OPTS];
  char *p, *p_new;
  unsigned int tokens;
  unsigned int i;
  bool fit_status = true;
  SA2D sa2d(&fit_status);
  vector<Solute> save_solutes;
  int echo_script = 0;

  while(fgets(line, sizeof(line), scriptfile)) {
    if(*line == '#') {
      continue;
    }
    for(i = 0; i < MAX_INPUT_LIN_OPTS; i++) {
      token[i] = "";
    }
    tokens = 1;
    p = line;
    while((p_new = strchr(p, ' '))) {
      token[tokens - 1] = p;
      *p_new = 0;
      p = p_new + 1;
      tokens++;
    }
    token[tokens - 1] = p;
    while(strlen(p) &&
	  (p[strlen(p) - 1] == '\n' ||
	   p[strlen(p) - 1] == '\r')) {
      p[strlen(p) - 1] = 0;
    }
    if(echo_script) {
      printf("[");
      for(i = 0; i < tokens; i++) {
	printf("%s ", token[i]);
      }
      puts("]");
    }
#if defined(DEBUG)
    printf("script file line:");
    for(i = 0; i < tokens; i++) {
      printf(" [%s]", token[i]);
    }
    puts("");
#endif
    if(!strcmp(token[0], "set_params")) {
      sa2d.set_params(&fit_status,
		      simulation_routine,
		      atoi(token[1]),
		      atoi(token[2]),
		      atof(token[3]),
		      atof(token[4]),
		      atof(token[5]),
		      atof(token[6]));
      continue;
    }
    if(!strcmp(token[0], "uniform_run")) {
      if(sa2d.uniform_run(atoi(token[1]))) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "local_uniform_run")) {
      if(sa2d.local_uniform_run(atoi(token[1]), atof(token[2]), atof(token[3]))) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "local_random_run")) {
      if(sa2d.local_random_run(atoi(token[1]), atof(token[2]))) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "regularize")) {
      if(sa2d.regularize(atof(token[1]))) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "coalesce")) {
      if(sa2d.coalesce(atof(token[1]), atof(token[2]))) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "get_variance")) {
      printf("get_variance = %g\n", sa2d.get_variance());
      continue;
    }
    if(!strcmp(token[0], "get_solutes")) {
      vector<Solute> solutes = sa2d.get_solutes();
      puts("s\tk\tc");
      for(i = 0; i < solutes.size(); i++) {
	printf("%d\t%g\t%g\t%g\n",
	       i,
	       solutes[i].s,
	       solutes[i].k,
	       solutes[i].c);
      }
      continue;
    }
    if(!strcmp(token[0], "run_simulation")) {
      if(sa2d.run_simulation()) {
	puts(sa2d.last_error);
      }
      continue;
    }
    if(!strcmp(token[0], "save_solutes")) {
      save_solutes = sa2d.get_solutes();
      continue;
    }
    if(!strcmp(token[0], "restore_solutes")) {
      sa2d.set_solutes(save_solutes);
      continue;
    }
    if(!strcmp(token[0], "echo_script")) {
      echo_script = !echo_script;
      continue;
    }
    printf("unrecognized sa2d script command %s\n", line);
  }
  fclose(scriptfile);
}
