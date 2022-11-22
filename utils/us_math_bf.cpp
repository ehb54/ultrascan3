//! \file us_math_bf.cpp
// Created by Lukas on 18.08.2022.
//
#include <cmath>

#include "us_math_bf.h"
#include "us_constants.h"


#define ROOT_EIGHT (2.0*M_SQRT2)


/*-*-*-*-*-*-*-*-*-*-*-* Private Section *-*-*-*-*-*-*-*-*-*-*-*/



/* based on SLATEC besj0, 1977 version, w. fullerton */

/* chebyshev expansions for Bessel functions

 series for bj0        on the interval  0.          to  1.60000d+01
                                        with weighted error   7.47e-18
                                         log weighted error  17.13
                               significant figures required  16.98
                                    decimal places required  17.68

*/

const QVector<double> US_Math_BF::bj0_data{
      0.100254161968939137,
      -0.665223007764405132,
      0.248983703498281314,
      -0.0332527231700357697,
      0.0023114179304694015,
      -0.0000991127741995080,
      0.0000028916708643998,
      -0.0000000612108586630,
      0.0000000009838650793,
      -0.0000000000124235515,
      0.0000000000001265433,
      -0.0000000000000010619,
      0.0000000000000000074,
};
const US_Math_BF::cheb_series US_Math_BF::bj0_cs = {
      US_Math_BF::bj0_data,
      12,
      -1, 1,
      9
};

/* based on SLATEC besj1, 1983 version, w. fullerton */

/* chebyshev expansions

 series for bj1        on the interval  0.          to  1.60000d+01
                                        with weighted error   4.48e-17
                                         log weighted error  16.35
                               significant figures required  15.77
                                    decimal places required  16.89

*/
const QVector<double> US_Math_BF::bj1_data {
      -0.11726141513332787,
      -0.25361521830790640,
      0.050127080984469569,
      -0.004631514809625081,
      0.000247996229415914,
      -0.000008678948686278,
      0.000000214293917143,
      -0.000000003936093079,
      0.000000000055911823,
      -0.000000000000632761,
      0.000000000000005840,
      -0.000000000000000044,
};
const US_Math_BF::cheb_series US_Math_BF::bj1_cs = {
      US_Math_BF::bj1_data,
      11,
      -1, 1,
      8
};

/* based on SLATEC besy0, 1980 version, w. fullerton */

/* chebyshev expansions

 series for by0        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.20e-17
                                         log weighted error  16.92
                               significant figures required  16.15
                                    decimal places required  17.48
*/

const QVector<double> US_Math_BF::by0_data {
      -0.011277839392865573,
      -0.128345237560420350,
      -0.104378847997942490,
      0.023662749183969695,
      -0.002090391647700486,
      0.000103975453939057,
      -0.000003369747162423,
      0.000000077293842676,
      -0.000000001324976772,
      0.000000000017648232,
      -0.000000000000188105,
      0.000000000000001641,
      -0.000000000000000011
};
const US_Math_BF::cheb_series US_Math_BF::by0_cs = {
      US_Math_BF::by0_data,
      12,
      -1, 1,
      8
};

/* based on SLATEC besy1, 1977 version, w. fullerton */

/* chebyshev expansions

 series for by1        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.87e-18
                                         log weighted error  17.73
                               significant figures required  17.83
                                    decimal places required  18.30
*/

const QVector<double> US_Math_BF::by1_data {
      0.03208047100611908629,
      1.262707897433500450,
      0.00649996189992317500,
      -0.08936164528860504117,
      0.01325088122175709545,
      -0.00089790591196483523,
      0.00003647361487958306,
      -0.00000100137438166600,
      0.00000001994539657390,
      -0.00000000030230656018,
      0.00000000000360987815,
      -0.00000000000003487488,
      0.00000000000000027838,
      -0.00000000000000000186
};
const US_Math_BF::cheb_series US_Math_BF::by1_cs = {
      US_Math_BF::by1_data,
      13,
      -1, 1,
      10
};

/* chebyshev expansions for amplitude and phase
   functions used in bessel evaluations

   These are the same for J0,Y0 and for J1,Y1, so
   they sit outside those functions.
*/

const QVector<double> US_Math_BF::bm0_data {
      0.09284961637381644,
      -0.00142987707403484,
      0.00002830579271257,
      -0.00000143300611424,
      0.00000012028628046,
      -0.00000001397113013,
      0.00000000204076188,
      -0.00000000035399669,
      0.00000000007024759,
      -0.00000000001554107,
      0.00000000000376226,
      -0.00000000000098282,
      0.00000000000027408,
      -0.00000000000008091,
      0.00000000000002511,
      -0.00000000000000814,
      0.00000000000000275,
      -0.00000000000000096,
      0.00000000000000034,
      -0.00000000000000012,
      0.00000000000000004
};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs = {
      US_Math_BF::bm0_data,
      20,
      -1, 1,
      10
};

const QVector<double> US_Math_BF::bth0_data {
      -0.24639163774300119,
      0.001737098307508963,
      -0.000062183633402968,
      0.000004368050165742,
      -0.000000456093019869,
      0.000000062197400101,
      -0.000000010300442889,
      0.000000001979526776,
      -0.000000000428198396,
      0.000000000102035840,
      -0.000000000026363898,
      0.000000000007297935,
      -0.000000000002144188,
      0.000000000000663693,
      -0.000000000000215126,
      0.000000000000072659,
      -0.000000000000025465,
      0.000000000000009229,
      -0.000000000000003448,
      0.000000000000001325,
      -0.000000000000000522,
      0.000000000000000210,
      -0.000000000000000087,
      0.000000000000000036
};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs = {
      US_Math_BF::bth0_data,
      23,
      -1, 1,
      12
};

const QVector<double> US_Math_BF::bm1_data {
      0.1047362510931285,
      0.00442443893702345,
      -0.00005661639504035,
      0.00000231349417339,
      -0.00000017377182007,
      0.00000001893209930,
      -0.00000000265416023,
      0.00000000044740209,
      -0.00000000008691795,
      0.00000000001891492,
      -0.00000000000451884,
      0.00000000000116765,
      -0.00000000000032265,
      0.00000000000009450,
      -0.00000000000002913,
      0.00000000000000939,
      -0.00000000000000315,
      0.00000000000000109,
      -0.00000000000000039,
      0.00000000000000014,
      -0.00000000000000005,
};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm1_cs = {
      US_Math_BF::bm1_data,
      20,
      -1, 1,
      10
};

const QVector<double> US_Math_BF::bth1_data {
      0.74060141026313850,
      -0.004571755659637690,
      0.000119818510964326,
      -0.000006964561891648,
      0.000000655495621447,
      -0.000000084066228945,
      0.000000013376886564,
      -0.000000002499565654,
      0.000000000529495100,
      -0.000000000124135944,
      0.000000000031656485,
      -0.000000000008668640,
      0.000000000002523758,
      -0.000000000000775085,
      0.000000000000249527,
      -0.000000000000083773,
      0.000000000000029205,
      -0.000000000000010534,
      0.000000000000003919,
      -0.000000000000001500,
      0.000000000000000589,
      -0.000000000000000237,
      0.000000000000000097,
      -0.000000000000000040,
};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth1_cs = {
      US_Math_BF::bth1_data,
      23,
      -1, 1,
      12
};


double US_Math_BF::cheb_eval(const US_Math_BF::cheb_series *cs, const double& x)
{
   int j;
   double d  = 0.0;
   double dd = 0.0;

   double y  = (2.0*x - cs->a - cs->b) / (cs->b - cs->a);
   double y2 = 2.0 * y;


   for(j = cs->order; j>=1; j--) {
      double temp = d;
      d = y2*d - dd + cs->c[j];
      dd = temp;
   }

   {
      d = y*d - dd + 0.5 * cs->c[0];
   }

   return d;
}

double US_Math_BF::bessel_asymp_Mnu(const double& nu, const double& x)
{
   double result;
   const double r  = 2.0*nu/x;
   const double r2 = r*r;
   const double x2 = x*x;
   const double term1 = (r2-1.0/x2)/8.0;
   const double term2 = (r2-1.0/x2)*(r2-9.0/x2)*3.0/128.0;
   const double Mnu2_c = 2.0/(M_PI) * (1.0 + term1 + term2);
   result = sqrt(Mnu2_c)/sqrt(x); /* will never underflow this way */
   return result;
}

double US_Math_BF::bessel_asymp_thetanu_corr(const double& nu, const double& x)
{
   double result;
   const double r  = 2.0*nu/x;
   const double r2 = r*r;
   const double x2 = x*x;
   const double term1 = x*(r2 - 1.0/x2)/8.0;
   const double term2 = x*(r2 - 1.0/x2)*(r2 - 25.0/x2)/384.0;
   result = (-0.25*M_PI + term1 + term2);
   return result;
}

double US_Math_BF::bessel_cos_pi4(const double& y, const double& eps)
{
   double result;
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if(fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps*eps;
      seps = eps * (1.0 - e2/6.0 * (1.0 - e2/20.0));
      ceps = 1.0 - e2/2.0 * (1.0 - e2/12.0);
   }
   else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   result = (ceps * s - seps * d)/ M_SQRT2;
   return result;
}

double US_Math_BF::bessel_sin_pi4(const double& y, const double& eps)
{
   double result;
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if(fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps*eps;
      seps = eps * (1.0 - e2/6.0 * (1.0 - e2/20.0));
      ceps = 1.0 - e2/2.0 * (1.0 - e2/12.0);
   }
   else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   result = (ceps * d + seps * s)/ M_SQRT2;


   return result;
}

double US_Math_BF::bessel_J0(const double& x)
{
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   if(y < 2.0*GSL_SQRT_DBL_EPSILON) {
      result = 1.0;
      return result;
   }
   else if(y <= 4.0) {
      return US_Math_BF::cheb_eval(&US_Math_BF::bj0_cs, 0.125*y*y - 1.0);
   }
   else {
      const double z = 32.0/(y*y) - 1.0;
      double ca = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs,  z);
      double ct = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs, z);
      double cp = bessel_cos_pi4(y, ct/y);
      const double sqrty = sqrt(y);
      const double ampl  = (0.75 + ca) / sqrty;
      result  = ampl * cp;
      return result;
   }
}

double US_Math_BF::bessel_J1(const double& x)
{
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   // underflow error or J1(0) = 0
   if(y < 2.0*GSL_DBL_MIN) {
      result = 0.0;
      return result;
   }
   else if(y < ROOT_EIGHT * GSL_SQRT_DBL_EPSILON) {
      result = 0.5*x;
      return result;
   }
   else if(y < 4.0) {
      double c;
      c = cheb_eval(&US_Math_BF::bj1_cs, 0.125*y*y-1.0);
      result = x * (0.25 + c);
      return result;
   }
   else {
      /* Because the leading term in the phase is y,
       * which we assume is exactly known, the error
       * in the cos() evaluation is bounded.
       */
      const double z  = 32.0/(y*y) - 1.0;
      double ca;
      double ct;
      double sp;
      ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs,  z);
      ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      sp = bessel_sin_pi4(y, ct/y);
      const double sqrty = sqrt(y);
      const double ampl  = (0.75 + ca) / sqrty;
      result = (x < 0.0 ? -ampl : ampl) * sp;
      return result;
   }
}

double US_Math_BF::bessel_Y0(const double& x)
{
   double result;
   const double two_over_pi = 2.0/M_PI;
   const double xmax        = 1.0/GSL_DBL_EPSILON;

   /* CHECK_POINTER(result) */

   if (x <= 0.0) { // domain error
      result = NAN;
      return result;
   }
   else if(x < 4.0) {
      double J0;
      double c;
      J0 = bessel_J0(x);
      c = cheb_eval(&by0_cs, 0.125*x*x-1.0);
      result= two_over_pi*(-M_LN2 + log(x))*J0 + 0.375 + c;
      return result;
   }
   else if(x < xmax) {
      /* Leading behaviour of phase is x, which is exact,
       * so the error is bounded.
       */
      const double z  = 32.0/(x*x) - 1.0;
      double c1;
      double c2;
      double sp;
      c1 = cheb_eval(&_gsl_sf_bessel_amp_phase_bm0_cs,  z);
      c2 = cheb_eval(&_gsl_sf_bessel_amp_phase_bth0_cs, z);
      sp = bessel_sin_pi4(x, c2/x);
      const double sqrtx = sqrt(x);
      const double ampl  = (0.75 + c1) / sqrtx;
      result  = ampl * sp;
      return result;
   }
   else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::bessel_Y1(const double& x)
{
   double result;
   const double two_over_pi = 2.0 / M_PI; // = 0.6366198
   const double xmin = 1.571 * GSL_DBL_MIN; /*exp ( amax1(alog(r1mach(1)), -alog(r1mach(2)))+.01)  = 3.495591E-308*/
   const double x_small = 2.0 * GSL_SQRT_DBL_EPSILON; // = 2.980232E-8
   const double xmax    = 1.0 / GSL_DBL_EPSILON; // = 4.5036E15

   /* CHECK_POINTER(result) */

   if(x <= 0.0) { // domain error
      result = NAN;
      return result;
   }
   else if(x < xmin) { // overflow error
      result = INFINITY;
      return result;
   }
   else if(x < x_small) {
      const double lnterm = log(0.5*x);
      double J1;
      double c;
      J1 = bessel_J1(x);
      c = cheb_eval(&by1_cs, -1.0);
      result = two_over_pi * lnterm * J1 + (0.5 + c)/x;
      return result;
   }
   else if(x < 4.0) {
      const double lnterm = log(0.5*x);
      double J1;
      double c;
      c = cheb_eval(&by1_cs, 0.125*x*x-1.0);
      J1 = bessel_J1(x);
      result = two_over_pi * lnterm * J1 + (0.5 + c)/x;
      return result;
   }
   else if(x < xmax) {
      const double z = 32.0/(x*x) - 1.0;
      double ca;
      double ct;
      double cp;
      ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs,  z);
      ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      cp = bessel_cos_pi4(x, ct/x);
      const double sqrtx = sqrt(x);
      const double ampl  = (0.75 + ca) / sqrtx;
      result = -ampl * cp;
      return result;
   }
   else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::transcendental_equation(const double& x, const double& a, const double& b) {
    double xa = x * a;
    double xb = x * b;
    double y1 = bessel_J1(xa) * bessel_Y1(xb);
    double y2 = bessel_J1(xb) * bessel_Y1(xa);
    return y1 - y2;
}






bool US_Math_BF::Secant_Solver::solve(double &x0, double &x1) {
    qDebug() << "called solve, x0="<< x0 << "x1=" << x1 <<"\n";
    int n = 0;
    double x2, f2;
    do {
        double f0 = func(x0);
        double f1 = func(x1);

        if(f0 == f1)
        {
           qDebug() << "Found nothing";
            return false;
        }

        x2 = x1 - (x1 - x0) * f1/(f1-f0);
        f2 = func(x2);
        qDebug() << "n=" << n << "\nf(x0)=f(" << x0 << ")=" << f0 << "\n" << " f(x1)=f(" << x1 << ")=" << f1 << "\n" << " f(x2)=f(" << x2 << ")=" << f2 << "\n";
        x0 = x1;
        f0 = f1;
        x1 = x2;
        f1 = f2;

        n = n + 1;

    } while ((fabs(f2) >= epsilon) && n < iter_max); // repeat the loop until the convergence or
    // hitting iter_max

    // check if loop end was convergence
    if (fabs(f2) <= epsilon) {
       // append solution to solutions vector
        if (!solutions.contains(x2)){
           solutions << x2;
        }
        qDebug() << "found solution x2=" << x2 << " f2=" << f2 << " n=" << n <<"\n";
        return true;
    }
    // loop end was iter_max
    else
        qDebug() << "found no solution " << "x0=" << x0 << " xm="<< x2 << " check=" << (fabs(x2 - x0) >= epsilon)
        << " n=" << n <<"\n";
        return false;
}

bool US_Math_BF::Secant_Solver::solve_wrapper() {
    // init iter variables
    qDebug() << "called solve wrapper\n";
    double x0 = i_min;
    double x1 = i_min + grid_res;
    int n = 0;
    do {
        // check if func(x1) and func(x2) have different signs
        if (func(x0) * func(x1) < 0){
            solve(x0,x1);
        }
        n = fmax(n + 1, int(floor(x1-i_min / grid_res) + 1));
        x0 = i_min + n * grid_res;
        x1 = i_min + (n+1) * grid_res;
    } while (x1 < i_max);
    qDebug() << "found Solutions " << solutions.length() << "\n";
    if (solutions.isEmpty()){
        return false;
    }
    return true;
}