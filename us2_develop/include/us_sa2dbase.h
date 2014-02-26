// c++ class for SA2D version 0.6
// Written by E. Brookes 2005. ebrookes@cs.utsa.edu
// added boolean pointer to fit_status flag in constructors 6/24 -b.d-

#ifndef US_SA2DBASE_H
#define US_SA2DBASE_H

#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Solute
{
   public:
      double s;
      double k;
      double c;
      Solute() {};
      ~Solute() {};
      bool operator==(const Solute& objIn) 
      {
         return (s == objIn.s && k == objIn.k);
      }
      bool operator!=(const Solute& objIn) 
      {
         return (s != objIn.s || k != objIn.k);
      }
      bool operator < (const Solute& objIn) const
      {
         if (s < objIn.s)
         {
            return (true);
         }
         else if (s == objIn.s && k < objIn.k)
         {
            return(true);
         }
         else
         {
            return(false);
         }
      }
};

class Solute_vector 
{
   public:
      vector<Solute> solutes;
      double fitness;
      double reg_fitness;
};

class Simulation_values 
{
   public:
      vector<Solute> solutes;
      double variance;
      vector<double> ti_noise;
      vector<double> ri_noise;
      vector<double> variances;
};
      
class SA2D 
{

   private:
      unsigned int max_solutes;                        // the maximum # of soluted allowed in a vector
      unsigned int s_gridsize, k_gridsize;             // the gridsizes
      double s_min, s_max;                    // the s values range
      double f_ratio_min, f_ratio_max;        // the f_ratio ranges (f/f0)
      double variance;                        // last variance
      vector<Solute> solutes;                 // last solutes
      Simulation_values (*simulation_routine)(Simulation_values); // the simulation function
      bool *fit_status;
      void *us_sa2d_w;
      bool use_sa2d_w;

   public:

      SA2D(bool *);
      //   pointer to fit status flag
      SA2D(bool *, Simulation_values (*)(Simulation_values), unsigned int, unsigned int, double, double, double, double); 
      //   pointer to fit status flag,
      //   a pointer to a function taking simulation_values with s & k populated, returning with c populates and the variance
      //        s_gridsize, k_gridsize, s_min, s_max, f/f0_min, f/f0_max

      SA2D(bool *, void *, unsigned int, unsigned int, double, double, double, double); 
      //   pointer to fit status flag, a pointer to a us_sa2d_w object
      //   a pointer to a function taking simulation_values with s & k populated, returning with c populates and the variance
      //        s_gridsize, k_gridsize, s_min, s_max, f/f0_min, f/f0_max

      ~SA2D();
      
      void set_params(bool *, Simulation_values (*)(Simulation_values), unsigned int, unsigned int, double, double, double, double);
      //   pointer to fit status flag
      //    a pointer to a function taking simulation_values with s & k populated, returning with c populates and the variance
      //        s_gridsize, k_gridsize, s_min, s_max, f/f0_min, f/f0_max

      void set_params(bool *, void *, unsigned int, unsigned int, double, double, double, double);
      //   pointer to fit status flag
      //    a pointer to a us_sa2d_w object
      //    a pointer to a function taking simulation_values with s & k populated, returning with c populates and the variance
      //        s_gridsize, k_gridsize, s_min, s_max, f/f0_min, f/f0_max

      int uniform_run(unsigned int);      // runs the shifting uniform grid method for 'int' steps, 0 steps is a single fixed grid,
                                           // returns non zero if there is an error

      int uniform_run(unsigned int, bool); // the additional 'bool' parameter if true runs the moving mesh iteratively only
                                            // when the 'steps' is not 1 (i.e. no movement of the mesh)

      int local_uniform_run(unsigned int, double, double); 
                                           // runs the local uniform grid for 'int' steps, applying a initial scaling factor of 'double'
                                           // and subsequent scaling factor of 'double'
                                           // The initial scaling factor should be in the range of .5 to .05 (I've been using a .3)
                                           // The subsequent scaling factor should close to 1, maybe .9 since this
                                           // gets multiplied each step, and can get too small for alot of steps
                                           // returns non zero if there is an error
                                           // it is an error if the uniform run has not been run at least once ! not trapped yet
                                           // the variance will not decrease
                                           // terminates early if there is no improvement and the double is 1e0 ! not implemented yet
                                           // returns an error if there is not enough memory to continue, reporting the number of
                                           // completed iterations and keeping the best solutes so far.  This can happen if there
                                           // are too many non-zero concentration solutes left for the grid.

                                           // Note - generally, it will use less than the max_solutes, due to edge clipping, close
                                           // solute overlap, and the integral uniform assignment of number of gridpoints solutes to each
                                           // non-zero concentration solute center.  Some fancy assignments like non-square grids, extra
                                           // points to higher concentration areas or random assignments could fill this up, but has not
                                           // been implemented or thought about much more than during the time it took to write this note.

      int local_random_run(unsigned int, double);  // runs the local random grid for 'int' steps, using a random distance with parameter 'double'
                                           // I'll give you a range of suggested parameters later
                                           // returns non zero if there is an error
                                           // it is an error if the uniform run has not been run at least once ! not trapped yet
                                           // the variance will not decrease

      int local_random_run(unsigned int, Solute (*)(Solute));
                                           // same as above, except the random function is externally defined ! not implemented yet

      int regularize(double);       // regularize with a regularization factor (0->1)
                                     // this is a 'lowest concentration prune' regularization method
                                     // returns non zero if there is an error
                                     // it is an error if the uniform run has not been run at least once ! not trapped yet
                                     // the variance may increase (generally will increase, unless the existing solutes are already
                                     //                            at maximum entropy


      int coalesce(double, double); // coalesce using a distance metric of 'double' & a concentration pruning threshold of 'double'.
                                     // the variance may increase (generally will increase, unless nothing is coalesced)
                                     // solutes joined by distance are weighted by concentration, and the joined concentration is the
                                     // sum of the previous concentrations
                                     // finally, a Simulation_routine is done to reset the concentrations
                                     // it is an error if the uniform run has not been run at least once ! not trapped yet

      int clip_data(unsigned int);  // clip the lowest concentration element from the solutes vector and return 
                                     // n-1 solutes. Repeat this for as many steps as are given in parameter list 
                                     // the variance may increase (generally will increase, unless nothing is coalesced)
                                     // solutes joined by distance are weighted by concentration, and the joined concentration is the
                                     // sum of the previous concentrations
                                     // finally, a Simulation_routine is done to reset the concentrations
                                     // it is an error if the uniform run has not been run at least once ! not trapped yet

      double get_variance();        // returns the variance of the last solution
      vector<double> get_s();       // returns a vector of the s values of the s values corresponding to the last solution
      vector<double> get_f_ratio(); // returns a vector of the s values of the f_ratio values corresponding to the last solution
      vector<double> get_D(double); // returns a vector of the D values given a vbar
      Simulation_values get_simulation_values();     // returns a copy of simulation_values
      void set_simulation_values(Simulation_values); // sets simulation_values
      vector<Solute> get_solutes();                  // returns a copy of the vector of the solutes
      unsigned int get_solutes_size();               // returns solutes.size()
//      void fprint_solutes(FILE *);                  // sends a list of last solutes to the stream
      void set_solutes(vector<Solute>);              // set the solute vector
      int run_simulation();              // call the simulation routine for the current solutes & set the concentrations & variance
      char last_error[1024];             // last error string
};

void sa2d_script(Simulation_values (*)(Simulation_values), char *);

#endif /*include US_SA2DBASE_H*/
