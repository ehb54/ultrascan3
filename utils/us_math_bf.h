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

      //! \brief Given a chebyshev series and a value, return the value of the chebyshev series
      //! \param cheb_series The pointer to the chebyshev series
      //! \param x           The value to calculate the chebyshev series for
      static double cheb_eval(const cheb_series*, double);

      static double bessel_asymp_Mnu(double, double);

      static double bessel_asymp_thetanu_corr(double, double);

      static double bessel_cos_pi4(double, double);

      static double bessel_sin_pi4(double, double);

      static double bessel_J0(double);

      static double bessel_J1(double);

      static double bessel_Y0(double);

      static double bessel_Y1(double);

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
