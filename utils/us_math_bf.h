//! \brief us_math_bf.h
// copied from GSL, the GNU Scientific Library, a collection of numerical
// routines for scientific computing. GSL is free software, you can redistribute it and/or modify it under
// the terms of the GNU General Public License.
//
// The GNU General Public License does not permit this software to be
// redistributed in proprietary programs.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

#ifndef US_MATH_BF_H
#define US_MATH_BF_H

#include <QtCore>
#include "us_simparms.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_extern.h"


#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

//! \brief A collection of mathematical routines related to the band forming experiment & simulation.
//! all function are static
class US_UTIL_EXTERN US_Math_BF  : public QObject {
   Q_OBJECT
public:
   //! \brief Class to represent a solver using the secant method.
   //! \note The current implementation works fine for the roots with a change in the leading sign, it struggles with
   //! minima/maxima which are at the same time roots due to the method of choice. As a first step into solving that
   //! every grid point is checked if it is within epsilon to 0. But this can cause false result if multiple grid points
   //! around an actual root are within the epsilon tolerance.
   class US_UTIL_EXTERN Secant_Solver {
   public:
      //! \brief Constructor for Secant_Solver
      //! \param i_min_ The double value representing the lower interval border
      //! \param i_max_ The double value representing the upper interval border
      //! \param foo_   The function representing the left part of the equation, for which foo_(x) = 0 should be solved
      //! \param grid_res_ The double value representing the distance of two grid points, defaults to 0.01
      //! \param epsilon_ The double value representing the size of the epsilon slope around 0, defaults to 0.000001
      //! \param iter_max_ The integer value representing the maximum of iterations to find the root in a sub-interval,
      //! defaults to 20
      Secant_Solver(const double &i_min_, const double &i_max_, std::function<double(const double &)> foo_,
                    const double &grid_res_, const double &epsilon_, const int &iter_max_);

      //! \brief destructor for Secant_Solver
      ~Secant_Solver();

      const std::function<double(const double &)> func; //!< function to find roots for
      const double i_min; //!< lower border of the interval in which the roots are searched
      const double i_max; //!< upper border of the interval in which the roots are searched
      const int iter_max; //!< maximum of iterations to find a root in a sub-interval
      const double grid_res; //!< width of the sub-intervals
      const double epsilon; //!< maximum acceptable deviation from 0
      QVector<double> solutions; //!< QVector with the found roots of the function func

      //! \brief Finds the root in a given interval with the boundaries x1 and x2 within epsilon of the
      //! function func using the secant method
      //! \param x1 The lower boundary of the interval
      //! \param x2 The upper boundary of the interval
      //! \return A boolean if a root was found, which was appended to the solutions QVector
      bool solve(double &x1, double &x2);

      //! \brief Finds all roots in a given interval [i_min, i_max] within epsilon for the function func using
      //! the secant method and a grid resolution of grid_res
      //! \return A boolean if a root was found, which means the solutions QVector isn't empty
      bool solve_wrapper( );

   private:
      int dbg_level;
   };


   class US_UTIL_EXTERN Band_Forming_Gradient {
   public:
      double meniscus; //!< Meniscus position in cm
      double bottom; //!< Bottom position in cm
      double overlay_volume; //!< Overlay volume in mL
      double overlay_thickness; //!< Overlay layer thickness in cm
      double cp_pathlen;        //!< Pathlength of centerpiece in cm
      double cp_angle;          //!< Angle of centerpiece sector
      QList<US_CosedComponent> cosed_component; //!< Cosedimenting components
      bool is_empty;


      QList<US_CosedComponent> base_comps; //!< The cosedimenting components, which form the base of the buffer
      QList<US_CosedComponent> upper_comps; //!< The relative cosedimenting components in the upper part
      QList<US_CosedComponent> lower_comps; //!< The relative cosedimenting components in the lower part
      double base_density; //!< The density of the buffer base
      double base_viscosity; //!< The viscosity of the buffer base
      QVector<double> eigenvalues; //!< QVector with the found eigenvalues
      QVector<double> pre_calc_betas; //!< Map eigenvalues to their precalculated norm * Integral value
      US_DataIO::RawData dens_bfg_data; //!< Scan like data of the gradient density
      US_DataIO::RawData visc_bfg_data; //!< Scan like data of the gradient viscosity
      US_DataIO::RawData conc_bfg_data; //!< Scan like data of the gradient viscosity
      US_SimulationParameters simparms; //!< Simulation parameters

      //! \brief Create a band forming gradient
      //! \param m The double value representing the meniscus
      //! \param b The double value representing the bottom
      //! \param band_loading The double value representing the band loading volume in mL
      //! \param comps The cosedimenting components of the buffer
      //! \param pathlen The double value representing the path length of the cell in cm
      //! \param angle The double value representing the angle of the cell in degree
      Band_Forming_Gradient(double m, double b, double band_loading,
                            QList<US_CosedComponent> &comps, double pathlen,
                            double angle);

      Band_Forming_Gradient();

      //! \brief Calculate the eigenvalues
      bool get_eigenvalues( );


      //! \brief Given an Vector of eigenvalues the norm is calculated for a given internal radius meniscus and
      //! the external radius bottom
      //! \param beta The double value representing the eigenvalue
      double norm(const double &beta) const;

      //! \brief Calculate the eigenfunction for a given eigenvalue beta, external radius bottom and position x
      //! (x<= bottom)
      //! \param beta The double value representing the eigenvalue
      //! \param x    The double value representing the position r inside the cell
      double eigenfunction(const double &beta, const double &x) const;

      //! \brief Calculate the equilibrium concentration of a cosedimenting component
      //! \param cosed_comp The cosedimenting component to calculate for
      double calc_eq_comp_conc(US_CosedComponent &cosed_comp) const;

      //! \brief Calculate the concentration of a given cosedimenting component at a given radius and a given time
      //! \param x The double value representing the radial position
      //! \param t The double value representing the point of time
      //! \param temp The double value representing the current temperature
      //! \param cosed_comp The cosedimenting component to calculate for
      double calc_comp_conc(const double &x, const double &t,const double &temp, US_CosedComponent &cosed_comp);

      //! \brief Calculate the density/viscosity of the band forming gradient at a given radius x and a given point
      //! at time t
      //! \param N The number of elements in the arrays
      //! \param x The pointer to start of radial position array
      //! \param t The double value representing the point of time
      //! \param T The double value representing the experimental temperature
      //! \param Dens The pointer to the start of the Density array
      //! \param Visc The pointer to the start of the Viscosity array
      //! \param Conc The pointer to the start of the Concentration array
      //! \return boolean flag for the success of the adjustment
      bool calc_dens_visc(int N, const double* x, const double &t, double& T, double* Dens, double* Visc);

      bool adjust_sd(const double &x, const double &t, double& s, double& d, double& T, double& vbar);

      bool calc_dens_visc(const double &x, const double &t, double& s, double& d, const double& T, double& conc);

      bool calculate_gradient(US_SimulationParameters asparms, US_DataIO::RawData* editedData);

      //! \brief Calculate the density/viscosity of the band forming gradient at a given radius x and a given point
      //! at time t
      //! \param N The number of elements in the arrays
      //! \param x The pointer to start of radial position array
      //! \param t The double value representing the point of time
      //! \param DensCosed The pointer to the start of the Density array
      //! \param ViscCosed The pointer to the start of the Viscosity array
      void interpolateCCodiff(int N, const double *x, double t, double *DensCosed, double *ViscCosed);

       double dt;
   private:
      QMap<QString, std::array<double,2>> value_cache;



      int     Nx;       // number of points in radial direction
      int     dbg_level;          // debug level
   };


   //! A structure used for chebyshev series
   struct cheb_series_struct {
      const QVector<double> c;
      int order;    /* order of expansion          */
      double a;     /* lower interval point        */
      double b;     /* upper interval point        */
      int order_sp; /* effective single precision order */
   };


   typedef struct cheb_series_struct cheb_series;

   typedef std::function<double(const double &)> solver_func;


   //! \brief Given a chebyshev series and a value, return the value of the chebyshev series
   //! \param cheb_series The pointer to the chebyshev series
   //! \param x           The value to calculate the chebyshev series for
   static double cheb_eval(const cheb_series *, const double &);

   static double bessel_asymp_Mnu(const double &, const double &);

   static double bessel_asymp_thetanu_corr(const double &, const double &);

   static double bessel_cos_pi4(const double &, const double &);

   static double bessel_sin_pi4(const double &, const double &);

   static double bessel_J0(const double &);

   static double bessel_J1(const double &);

   static double bessel_Y0(const double &);

   static double bessel_Y1(const double &);

   //! \brief Given an x, the internal radius a and the external radius b, return the value of the function
   //! \param x    The double value x to calculate the function f(x)
   //! \param a    The double value representing the internal radius a from the cell (top)
   //! \param b    The double value representing the external radius b from the cell (bottom)
   static double transcendental_equation(const double &x, const double &a, const double &b);


   static const cheb_series _gsl_sf_bessel_amp_phase_bth1_cs;
   static const QVector<double> bj0_data;
   static const cheb_series bj0_cs;
   static const QVector<double> bj1_data;
   static const cheb_series bj1_cs;
   static const QVector<double> by0_data;
   static const cheb_series by0_cs;
   static const QVector<double> by1_data;
   static const cheb_series by1_cs;
   static const QVector<double> bm0_data;
   static const cheb_series _gsl_sf_bessel_amp_phase_bm0_cs;
   static const QVector<double> bth0_data;
   static const cheb_series _gsl_sf_bessel_amp_phase_bth0_cs;
   static const QVector<double> bm1_data;
   static const cheb_series _gsl_sf_bessel_amp_phase_bm1_cs;
   static const QVector<double> bth1_data;

};

#endif
