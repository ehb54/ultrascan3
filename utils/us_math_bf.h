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
#include "us_extern.h"


//! \brief A collection of mathematical routines related to the band forming experiment & simulation.
//! all function are static
class US_UTIL_EXTERN US_Math_BF
{
   public:
    //! A structure used for chebyshev series
    struct cheb_series_struct {
        const QVector<double>    c;
        int order;    /* order of expansion          */
        double a;     /* lower interval point        */
        double b;     /* upper interval point        */
        int order_sp; /* effective single precision order */
    };


    typedef struct cheb_series_struct cheb_series;

    typedef std::function<double(const double &)> solver_func;

      class Secant_Solver{
        public:
          //! \brief Constructor for Secant_Solver
          Secant_Solver(const double& i_min_, const double& i_max_,
                        std::function<double(const double &)> foo_ = [](const double& a)
                        {return US_Math_BF::transcendental_equation(a, 1, 2);},
                        const double& grid_res_ = 0.01, const double& epsilon_ = 0.000001,
                        const int& iter_max_ = 20):
                  func(foo_), i_min(i_min_), i_max(i_max_), iter_max(iter_max_),
                  grid_res(grid_res_), epsilon(epsilon_) {
              solutions.clear();
              qDebug() << "init Secant Solver";
              qDebug() << "call solve_wrapper";
              qDebug() << "finished solve_wrapper";
          };

          //! \brief destructor for Secant_Solver
          ~Secant_Solver(){
              solutions.clear();
              qDebug() << "destroyed Secant Solver";
          }

          const std::function<double(const double &)> func;
          const double i_min;
          const double i_max;
          const int iter_max;
          const double grid_res;
          const double epsilon;
          QVector<double> solutions;

          //! \brief Finds the root in a given interval with the boundaries x1 and x2 within epsilon of the
          //! function func using the secant method
          //! \param x1 The lower boundary of the interval
          //! \param x2 The upper boundary of the interval
          //! \return A boolean if a root was found, which was appended to the solutions QVector
          bool solve(double& x1, double& x2);

          //! \brief Finds all roots in a given interval [i_min, i_max] within epsilon for the function func using
          //! the secant method and a grid resolution of grid_res
          //! \return A boolean if a root was found, which means the solutions QVector isn't empty
          bool solve_wrapper(void);

          void test(){qDebug() << "call test";};
      };



      //! \brief Given a chebyshev series and a value, return the value of the chebyshev series
      //! \param cheb_series The pointer to the chebyshev series
      //! \param x           The value to calculate the chebyshev series for
      static double cheb_eval(const cheb_series*, const double&);

      static double bessel_asymp_Mnu(const double&, const double&);

      static double bessel_asymp_thetanu_corr(const double&, const double&);

      static double bessel_cos_pi4(const double&, const double&);

      static double bessel_sin_pi4(const double&, const double&);

      static double bessel_J0(const double&);

      static double bessel_J1(const double&);

      static double bessel_Y0(const double&);

      static double bessel_Y1(const double&);

      //! \brief Given an x, the internal radius a and the external radius b, return the value of the function
      //! \param x    The double value x to calculate the function f(x)
      //! \param a    The double value representing the internal radius a from the cell (top)
      //! \param b    The double value representing the external radius b from the cell (bottom)
      static double transcendental_equation(const double& x, const double& a, const double& b);

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
