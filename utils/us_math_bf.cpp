//! \file us_math_bf.cpp
// Created by Lukas on 18.08.2022.
//
#include <cmath>

#include "us_math_bf.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"


#define ROOT_EIGHT (2.0*M_SQRT2)

US_Math_BF::Band_Forming_Gradient::Band_Forming_Gradient(const double m, const double b, const double band_loading,
                                                         QList<US_CosedComponent> &comps, const double pathlen,
                                                         const double angle) : meniscus(m), bottom(b),
                                                                               overlay_volume(band_loading),
                                                                               cp_pathlen(pathlen), cp_angle(angle),
                                                                               cosed_component(comps) {
   eigenvalues.clear();
   base_comps.clear();
   upper_comps.clear();
   lower_comps.clear();
   value_cache.clear();
   base_density = 0.0;
   base_viscosity = 0.0;
   double base = sq(meniscus) + overlay_volume * 360.0 / (cp_angle * cp_pathlen * M_PI);
   overlay_thickness = sqrt(base) - meniscus;

   QMap<QString, US_CosedComponent> upper_cosed;
   QMap<QString, US_CosedComponent> lower_cosed;
   foreach (US_CosedComponent i, comps) {
      if (i.s_coeff != 0.0)continue;
      if (!i.overlaying && upper_cosed.contains(i.name)) {
         // the current component is in the lower part, but there is another component with the same name in the
         // overlaying section of the band forming gradient
         US_CosedComponent j = upper_cosed[i.name];
         if (j.conc > i.conc) {
            // the concentration is higher in upper part, move it completely to the upper part and set the
            // concentration to the excess concentration
            j.conc = j.conc - i.conc;
            upper_cosed[j.name] = j;
            continue;
         } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
            // the concentration of both components is roughly equal, remove the component from the upper and lower part
            upper_cosed.remove(j.name);
            continue;
         } else {
            j.conc = i.conc - j.conc;
            lower_cosed[j.name] = j;
            upper_cosed.remove(j.name);
            continue;
         }
      }
      if (i.overlaying && lower_cosed.contains(i.name)) {
         // the current component is in the lower part, but there is another component with the same name in the
         // overlaying section of the band forming gradient
         US_CosedComponent j = lower_cosed[i.name];
         if (j.conc > i.conc) {
            // the concentration is higher in lower part, move it completely to the lower part and set the
            // concentration to the excess concentration
            j.conc = j.conc - i.conc;
            lower_cosed[j.name] = j;
            continue;
         } else if (fabs(j.conc - i.conc) < GSL_ROOT5_DBL_EPSILON) {
            // the concentration of both components is roughly equal, remove the component from the upper and lower part
            lower_cosed.remove(j.name);
            continue;
         } else {
            j.conc = i.conc - j.conc;
            upper_cosed[j.name] = j;
            lower_cosed.remove(j.name);
            continue;
         }
      }
      if (i.overlaying)
         upper_cosed[i.name] = i;
      else
         lower_cosed[i.name] = i;

      }
   // Determine the base of the buffer
   foreach (US_CosedComponent cosed_comp, comps) {
      if (cosed_comp.s_coeff != 0.0)continue;
      if (cosed_comp.overlaying) { continue; } // overlaying components can't be part of the base of the buffer
      if (lower_cosed.contains(cosed_comp.name) &&
          (fabs(lower_cosed[cosed_comp.name].conc - cosed_comp.conc) < GSL_ROOT5_DBL_EPSILON)) {
         // the concentration matches the original one entered. -> part of the buffer base
         base_comps << cosed_comp;
         base_density += cosed_comp.dens_coeff[0];
         base_viscosity += cosed_comp.visc_coeff[0];

      }
   }
   // normalize base density and viscosity
   base_density = base_density / base_comps.count();
   base_viscosity = base_viscosity / base_comps.count();
   // init upper_comps and lower_comps
   foreach (US_CosedComponent i, upper_cosed) { upper_comps << i; }
   foreach (US_CosedComponent i, lower_cosed) { lower_comps << i; }
   qDebug() << "Constructor BFG finished bc uc lc" << base_comps.count() << upper_comps.count() << lower_comps.count();
   qDebug() << "Constructor BFG finished bd bv" << base_density << base_viscosity;
}

bool US_Math_BF::Band_Forming_Gradient::get_eigenvalues(void) {
   double men = meniscus;
   double bot = bottom;
   std::function<double(const double &)> func = [&men, &bot](const double &a) {
      return US_Math_BF::transcendental_equation(a, men, bot);
   };
   US_Math_BF::Secant_Solver secantSolver = *new US_Math_BF::Secant_Solver(0.01, 20000, func,
                                                                           0.01,
                                                                           GSL_DBL_EPSILON, 20);
   bool return_value = secantSolver.solve_wrapper();
   eigenvalues = secantSolver.solutions;
   return return_value;
}

double US_Math_BF::Band_Forming_Gradient::norm(const double &beta) {
   return (sq(M_PI) / 2 * sq(beta) * sq(bessel_J1(beta * meniscus)) /
           (sq(bessel_J1(beta * meniscus)) - sq(bessel_J1(beta * bottom))));
}

double US_Math_BF::Band_Forming_Gradient::eigenfunction(const double &beta, const double &x) {
   return (bessel_J0(beta * x) * bessel_Y1(beta * bottom) - bessel_Y0(beta * x) * bessel_J1(beta * bottom));
}

double US_Math_BF::Band_Forming_Gradient::calc_eq_comp_conc(US_CosedComponent &cosed_comp) {
   double init_volume;
   double total_volume;
   // calculate the total cell volume
   total_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus));
   if (cosed_comp.overlaying) {
      init_volume = overlay_volume;
   } else {
      // calculate the volume of the lower section
      init_volume = M_PI * cp_pathlen * cp_angle / 360 * (sq(bottom) - sq(meniscus + overlay_thickness));
   }
   return init_volume * cosed_comp.conc / total_volume;
}

double US_Math_BF::Band_Forming_Gradient::calc_comp_conc(const double &x, const double &t, const double &temp,
                                                         US_CosedComponent &cosed_comp) {
   double eq_conc = calc_eq_comp_conc(cosed_comp);
   double decay = 0;
   for (double beta: eigenvalues) {
      double integral = cosed_comp.conc / beta * ((-meniscus * bessel_J1(meniscus * beta) +
                                                   (meniscus + overlay_thickness) *
                                                   bessel_J1(beta * (meniscus + overlay_thickness))) *
                                                  bessel_Y1(bottom * beta) + bessel_J1(bottom * beta) *
                                                                             (meniscus * bessel_Y1(meniscus * beta) -
                                                                              (meniscus + overlay_thickness) *
                                                                              bessel_Y1(beta * (meniscus +
                                                                                                overlay_thickness))));
      decay += norm(beta) * eigenfunction(beta, x) * exp(-cosed_comp.d_coeff * temp / 293.15 * sq(beta) * t) * integral;
   }

   return eq_conc + decay;
}

bool US_Math_BF::Band_Forming_Gradient::calc_dens_visc(const int N, double* x, const double &t, double &T, double* Dens, double* Visc) {
   // check if eigenvalues exist already
   int cached = 0;
   int calculated = 0;
   if ( eigenvalues.isEmpty()) {
      return false;
   }
   for ( int i = 0; i < N; i++ ) {
      double x_c = x[i];
      QString key = QString::number(x_c, 'f', 4) + QString::number(t, 'f', 0);
      if ( value_cache.contains(key)) {
         std::array<double,2> tmp = value_cache.value(key);
         Dens[i] += tmp[ 0 ];
         Visc[i] += tmp[ 1 ];
         cached ++;
      } else {
         // loop over all cosedimenting stuff and determine the current concentration
         // -> for now iterate only over upper_cosed
         double tmp_d = 0.0;
         double tmp_v = 0.0;
         for ( US_CosedComponent &cosed_comp: upper_comps ) {
            double c1 = calc_comp_conc(x_c, t, T, cosed_comp);
            if ( c1 < 0.002 ) { continue; }
            double c2 = c1 * c1;      // c1^2
            double c3 = c2 * c1;      // c1^3
            double c4 = c3 * c1;      // c1^4
            tmp_d += (cosed_comp.dens_coeff[ 1 ] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.dens_coeff[ 2 ] * 1.0e-2 * c1 +
                        cosed_comp.dens_coeff[ 3 ] * 1.0e-3 * c2 + cosed_comp.dens_coeff[ 4 ] * 1.0e-4 * c3 +
                        cosed_comp.dens_coeff[ 5 ] * 1.0e-6 * c4);
            tmp_v += (cosed_comp.visc_coeff[ 1 ] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.visc_coeff[ 2 ] * 1.0e-2 * c1 +
                          cosed_comp.visc_coeff[ 3 ] * 1.0e-3 * c2 + cosed_comp.visc_coeff[ 4 ] * 1.0e-4 * c3 +
                          cosed_comp.visc_coeff[ 5 ] * 1.0e-6 * c4);

         }
         // cache the value
         std::array<double,2> tmp{tmp_d,tmp_v};
         value_cache[ key ] = tmp;
         calculated ++;
      }
   }
   qDebug() << "calc_dens_visc with" << cached << "cached &" << calculated << "calculated values";
   return true;
}

bool US_Math_BF::Band_Forming_Gradient::adjust_sd(const double &x, const double &t, double &s, double &d, double &T,
                                                  double &vbar) {
   // check if eigenvalues exist already
   if (eigenvalues.isEmpty()) {
      return false;
   }
   double density = base_density;
   double viscosity = base_viscosity;
   if (value_cache.contains(QString::number(x, 'f', 3)+ QString::number(t, 'f', 0)))
   {
      std::array<double,2> tmp = value_cache.value(QString::number(x, 'f', 3)+QString::number(t, 'f', 0));
      density = tmp[0];
      viscosity = tmp[1];
   }
   else
   {
      // loop over all cosedimenting stuff and determine the current concentration
      // -> for now iterate only over upper_cosed
      for (US_CosedComponent &cosed_comp: upper_comps) {
         double c1 = calc_comp_conc(x, t, T, cosed_comp);
         if (c1 < 0.002) { continue; }
         double c2 = c1 * c1;      // c1^2
         double c3 = c2 * c1;      // c1^3
         double c4 = c3 * c1;      // c1^4
         density += (cosed_comp.dens_coeff[1] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.dens_coeff[2] * 1.0e-2 * c1 +
                     cosed_comp.dens_coeff[3] * 1.0e-3 * c2 + cosed_comp.dens_coeff[4] * 1.0e-4 * c3 +
                     cosed_comp.dens_coeff[5] * 1.0e-6 * c4);
         viscosity += (cosed_comp.visc_coeff[1] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.visc_coeff[2] * 1.0e-2 * c1 +
                       cosed_comp.visc_coeff[3] * 1.0e-3 * c2 + cosed_comp.visc_coeff[4] * 1.0e-4 * c3 +
                       cosed_comp.visc_coeff[5] * 1.0e-6 * c4);
      }
      // cache the value
      std::array<double,2> tmp{density,viscosity};
      value_cache[QString::number(x, 'f', 3)+ QString::number(t, 'f', 0)] = tmp;
   }
   s = s * VISC_20W * (1 - vbar * density) / (1.0 - vbar * DENS_20W) / viscosity;
   d = d * VISC_20W / viscosity * (T + 273.15) / 293.15;
   return true;
}

bool US_Math_BF::Band_Forming_Gradient::calc_dens_visc(const double &x, const double &t, double &dens, double &visc,const double& T) {
   // check if eigenvalues exist already
   if (eigenvalues.isEmpty()) {
      return false;
   }
   double density = base_density;
   double viscosity = base_viscosity;
   if (value_cache.contains(QString::number(x, 'f', 3)+ QString::number(t, 'f', 0)))
   {
      std::array<double,2> tmp = value_cache.value(QString::number(x, 'f', 3)+QString::number(t, 'f', 0));
      density = tmp[0];
      viscosity = tmp[1];
   }
   else
   {
      // loop over all cosedimenting stuff and determine the current concentration
      // -> for now iterate only over upper_cosed
      for (US_CosedComponent &cosed_comp: upper_comps) {
         double c1 = calc_comp_conc(x, t, T, cosed_comp);
         if (c1 < 0.002) { continue; }
         double c2 = c1 * c1;      // c1^2
         double c3 = c2 * c1;      // c1^3
         double c4 = c3 * c1;      // c1^4
         density += (cosed_comp.dens_coeff[1] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.dens_coeff[2] * 1.0e-2 * c1 +
                     cosed_comp.dens_coeff[3] * 1.0e-3 * c2 + cosed_comp.dens_coeff[4] * 1.0e-4 * c3 +
                     cosed_comp.dens_coeff[5] * 1.0e-6 * c4);
         viscosity += (cosed_comp.visc_coeff[1] * 1.0e-3 * sqrt(fabs(c1)) + cosed_comp.visc_coeff[2] * 1.0e-2 * c1 +
                       cosed_comp.visc_coeff[3] * 1.0e-3 * c2 + cosed_comp.visc_coeff[4] * 1.0e-4 * c3 +
                       cosed_comp.visc_coeff[5] * 1.0e-6 * c4);
      }
      // cache the value
      std::array<double,2> tmp{density,viscosity};
      value_cache[QString::number(x, 'f', 3)+ QString::number(t, 'f', 0)] = tmp;
   }
   dens = density;
   visc = viscosity;
   return true;
}


US_Math_BF::Secant_Solver::Secant_Solver(const double &i_min_, const double &i_max_,
                                         std::function<double(const double &)> foo_ = [](const double &a) {
                                            return US_Math_BF::transcendental_equation(a, 1, 2);
                                         }, const double &grid_res_ = 0.01, const double &epsilon_ = 0.000001,
                                         const int &iter_max_ = 20) : func(foo_), i_min(i_min_), i_max(i_max_),
                                                                      iter_max(iter_max_), grid_res(grid_res_),
                                                                      epsilon(epsilon_) {
   solutions.clear();
   dbg_level = US_Settings::us_debug();
}


US_Math_BF::Secant_Solver::~Secant_Solver() {
   solutions.clear();
   DbgLv(3) << "destroyed Secant Solver";
}


bool US_Math_BF::Secant_Solver::solve(double &x0, double &x1) {
   DbgLv(4) << "called solve, x0=" << x0 << "x1=" << x1 << "\n";
   int n = 0;
   double x2, f2;
   do {
      double f0 = func(x0);
      double f1 = func(x1);

      if (f0 == f1) {
         DbgLv(3) << "Found nothing";
         return false;
      }

      x2 = x1 - (x1 - x0) * f1 / (f1 - f0);
      f2 = func(x2);
      DbgLv(5) << "n=" << n << "\nf(x0)=f(" << x0 << ")=" << f0 << "\n" << " f(x1)=f(" << x1 << ")=" << f1 << "\n"
               << " f(x2)=f(" << x2 << ")=" << f2 << "\n";
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
      if (!solutions.contains(x2)) {
         solutions << x2;
      }
      DbgLv(3) << "found solution x2=" << x2 << " f2=" << f2 << " n=" << n;
      return true;
   }
      // loop end was iter_max
   else DbgLv(3) << "found no solution " << "x0=" << x0 << " xm=" << x2 << " check=" << (fabs(x2 - x0) >= epsilon)
                 << " n=" << n << "\n";
   return false;
}

bool US_Math_BF::Secant_Solver::solve_wrapper() {
   // init iter variables
   double x0 = i_min;
   double x1 = i_min + grid_res;
   int n = 0;
   do {
      double f0, f1;
      f0 = func(x0);
      f1 = func(x1);
      // check if func(x1) and func(x2) have different signs
      if ((f0 * f1) < 0) {
         solve(x0, x1);
      }
      if (((f0 * f1) > 0) && (fabs(f0) < epsilon)) {
         // append solution to solutions vector
         if (!solutions.contains(x0)) {
            solutions << x0;
         }
         DbgLv(3) << "found grid solution x0=" << x0 << " f0=" << f0;
      }
      if ((((f0 * f1) > 0) && (fabs(f1) < epsilon))) {
         // append solution to solutions vector
         if (!solutions.contains(x1)) {
            solutions << x1;
         }
         DbgLv(3) << "found grid solution x1=" << x1 << " f1=" << f1;
      }
      n = fmax(n + 1, int(floor(x1 - i_min / grid_res) + 1));
      x0 = i_min + n * grid_res;
      x1 = i_min + (n + 1) * grid_res;
   } while (x1 < i_max);
   DbgLv(1) << "found Solutions " << solutions.length() << "\n";
   // sort solutions and clean them up, just in case
   qSort(solutions);
   int length = solutions.count();
   int i = 0;
   while (i < length - 1){
      while(i+1<length && solutions.value(i+1)-solutions.value(i)<grid_res+grid_res/10){
         solutions.remove(i+1);
         length--;
      }
      i++;
   }
   DbgLv(1) << "found unqiue Solutions " << solutions.length() << "\n";
   if (solutions.isEmpty()) {
      return false;
   }
   return true;
}


/*-*-*-*-*-*-*-*-*-*-*-* Private Section *-*-*-*-*-*-*-*-*-*-*-*/



/* based on SLATEC besj0, 1977 version, w. fullerton */

/* chebyshev expansions for Bessel functions

 series for bj0        on the interval  0.          to  1.60000d+01
                                        with weighted error   7.47e-18
                                         log weighted error  17.13
                               significant figures required  16.98
                                    decimal places required  17.68

*/

const QVector<double> US_Math_BF::bj0_data{0.100254161968939137, -0.665223007764405132, 0.248983703498281314,
                                           -0.0332527231700357697, 0.0023114179304694015, -0.0000991127741995080,
                                           0.0000028916708643998, -0.0000000612108586630, 0.0000000009838650793,
                                           -0.0000000000124235515, 0.0000000000001265433, -0.0000000000000010619,
                                           0.0000000000000000074,};
const US_Math_BF::cheb_series US_Math_BF::bj0_cs = {US_Math_BF::bj0_data, 12, -1, 1, 9};

/* based on SLATEC besj1, 1983 version, w. fullerton */

/* chebyshev expansions

 series for bj1        on the interval  0.          to  1.60000d+01
                                        with weighted error   4.48e-17
                                         log weighted error  16.35
                               significant figures required  15.77
                                    decimal places required  16.89

*/
const QVector<double> US_Math_BF::bj1_data{-0.11726141513332787, -0.25361521830790640, 0.050127080984469569,
                                           -0.004631514809625081, 0.000247996229415914, -0.000008678948686278,
                                           0.000000214293917143, -0.000000003936093079, 0.000000000055911823,
                                           -0.000000000000632761, 0.000000000000005840, -0.000000000000000044,};
const US_Math_BF::cheb_series US_Math_BF::bj1_cs = {US_Math_BF::bj1_data, 11, -1, 1, 8};

/* based on SLATEC besy0, 1980 version, w. fullerton */

/* chebyshev expansions

 series for by0        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.20e-17
                                         log weighted error  16.92
                               significant figures required  16.15
                                    decimal places required  17.48
*/

const QVector<double> US_Math_BF::by0_data{-0.011277839392865573, -0.128345237560420350, -0.104378847997942490,
                                           0.023662749183969695, -0.002090391647700486, 0.000103975453939057,
                                           -0.000003369747162423, 0.000000077293842676, -0.000000001324976772,
                                           0.000000000017648232, -0.000000000000188105, 0.000000000000001641,
                                           -0.000000000000000011};
const US_Math_BF::cheb_series US_Math_BF::by0_cs = {US_Math_BF::by0_data, 12, -1, 1, 8};

/* based on SLATEC besy1, 1977 version, w. fullerton */

/* chebyshev expansions

 series for by1        on the interval  0.          to  1.60000d+01
                                        with weighted error   1.87e-18
                                         log weighted error  17.73
                               significant figures required  17.83
                                    decimal places required  18.30
*/

const QVector<double> US_Math_BF::by1_data{0.03208047100611908629, 1.262707897433500450, 0.00649996189992317500,
                                           -0.08936164528860504117, 0.01325088122175709545, -0.00089790591196483523,
                                           0.00003647361487958306, -0.00000100137438166600, 0.00000001994539657390,
                                           -0.00000000030230656018, 0.00000000000360987815, -0.00000000000003487488,
                                           0.00000000000000027838, -0.00000000000000000186};
const US_Math_BF::cheb_series US_Math_BF::by1_cs = {US_Math_BF::by1_data, 13, -1, 1, 10};

/* chebyshev expansions for amplitude and phase
   functions used in bessel evaluations

   These are the same for J0,Y0 and for J1,Y1, so
   they sit outside those functions.
*/

const QVector<double> US_Math_BF::bm0_data{0.09284961637381644, -0.00142987707403484, 0.00002830579271257,
                                           -0.00000143300611424, 0.00000012028628046, -0.00000001397113013,
                                           0.00000000204076188, -0.00000000035399669, 0.00000000007024759,
                                           -0.00000000001554107, 0.00000000000376226, -0.00000000000098282,
                                           0.00000000000027408, -0.00000000000008091, 0.00000000000002511,
                                           -0.00000000000000814, 0.00000000000000275, -0.00000000000000096,
                                           0.00000000000000034, -0.00000000000000012, 0.00000000000000004};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs = {US_Math_BF::bm0_data, 20, -1, 1, 10};

const QVector<double> US_Math_BF::bth0_data{-0.24639163774300119, 0.001737098307508963, -0.000062183633402968,
                                            0.000004368050165742, -0.000000456093019869, 0.000000062197400101,
                                            -0.000000010300442889, 0.000000001979526776, -0.000000000428198396,
                                            0.000000000102035840, -0.000000000026363898, 0.000000000007297935,
                                            -0.000000000002144188, 0.000000000000663693, -0.000000000000215126,
                                            0.000000000000072659, -0.000000000000025465, 0.000000000000009229,
                                            -0.000000000000003448, 0.000000000000001325, -0.000000000000000522,
                                            0.000000000000000210, -0.000000000000000087, 0.000000000000000036};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs = {US_Math_BF::bth0_data, 23, -1, 1, 12};

const QVector<double> US_Math_BF::bm1_data{0.1047362510931285, 0.00442443893702345, -0.00005661639504035,
                                           0.00000231349417339, -0.00000017377182007, 0.00000001893209930,
                                           -0.00000000265416023, 0.00000000044740209, -0.00000000008691795,
                                           0.00000000001891492, -0.00000000000451884, 0.00000000000116765,
                                           -0.00000000000032265, 0.00000000000009450, -0.00000000000002913,
                                           0.00000000000000939, -0.00000000000000315, 0.00000000000000109,
                                           -0.00000000000000039, 0.00000000000000014, -0.00000000000000005,};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bm1_cs = {US_Math_BF::bm1_data, 20, -1, 1, 10};

const QVector<double> US_Math_BF::bth1_data{0.74060141026313850, -0.004571755659637690, 0.000119818510964326,
                                            -0.000006964561891648, 0.000000655495621447, -0.000000084066228945,
                                            0.000000013376886564, -0.000000002499565654, 0.000000000529495100,
                                            -0.000000000124135944, 0.000000000031656485, -0.000000000008668640,
                                            0.000000000002523758, -0.000000000000775085, 0.000000000000249527,
                                            -0.000000000000083773, 0.000000000000029205, -0.000000000000010534,
                                            0.000000000000003919, -0.000000000000001500, 0.000000000000000589,
                                            -0.000000000000000237, 0.000000000000000097, -0.000000000000000040,};
const US_Math_BF::cheb_series US_Math_BF::_gsl_sf_bessel_amp_phase_bth1_cs = {US_Math_BF::bth1_data, 23, -1, 1, 12};


double US_Math_BF::cheb_eval(const US_Math_BF::cheb_series *cs, const double &x) {
   int j;
   double d = 0.0;
   double dd = 0.0;

   double y = (2.0 * x - cs->a - cs->b) / (cs->b - cs->a);
   double y2 = 2.0 * y;


   for (j = cs->order; j >= 1; j--) {
      double temp = d;
      d = y2 * d - dd + cs->c[j];
      dd = temp;
   }

   {
      d = y * d - dd + 0.5 * cs->c[0];
   }

   return d;
}

double US_Math_BF::bessel_asymp_Mnu(const double &nu, const double &x) {
   double result;
   const double r = 2.0 * nu / x;
   const double r2 = r * r;
   const double x2 = x * x;
   const double term1 = (r2 - 1.0 / x2) / 8.0;
   const double term2 = (r2 - 1.0 / x2) * (r2 - 9.0 / x2) * 3.0 / 128.0;
   const double Mnu2_c = 2.0 / (M_PI) * (1.0 + term1 + term2);
   result = sqrt(Mnu2_c) / sqrt(x); /* will never underflow this way */
   return result;
}

double US_Math_BF::bessel_asymp_thetanu_corr(const double &nu, const double &x) {
   double result;
   const double r = 2.0 * nu / x;
   const double r2 = r * r;
   const double x2 = x * x;
   const double term1 = x * (r2 - 1.0 / x2) / 8.0;
   const double term2 = x * (r2 - 1.0 / x2) * (r2 - 25.0 / x2) / 384.0;
   result = (-0.25 * M_PI + term1 + term2);
   return result;
}

double US_Math_BF::bessel_cos_pi4(const double &y, const double &eps) {
   double result;
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if (fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps * eps;
      seps = eps * (1.0 - e2 / 6.0 * (1.0 - e2 / 20.0));
      ceps = 1.0 - e2 / 2.0 * (1.0 - e2 / 12.0);
   } else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   result = (ceps * s - seps * d) / M_SQRT2;
   return result;
}

double US_Math_BF::bessel_sin_pi4(const double &y, const double &eps) {
   double result;
   const double sy = sin(y);
   const double cy = cos(y);
   const double s = sy + cy;
   const double d = sy - cy;
   double seps;
   double ceps;
   if (fabs(eps) < GSL_ROOT5_DBL_EPSILON) {
      const double e2 = eps * eps;
      seps = eps * (1.0 - e2 / 6.0 * (1.0 - e2 / 20.0));
      ceps = 1.0 - e2 / 2.0 * (1.0 - e2 / 12.0);
   } else {
      seps = sin(eps);
      ceps = cos(eps);
   }
   result = (ceps * d + seps * s) / M_SQRT2;


   return result;
}

double US_Math_BF::bessel_J0(const double &x) {
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   if (y < 2.0 * GSL_SQRT_DBL_EPSILON) {
      result = 1.0;
      return result;
   } else if (y <= 4.0) {
      return US_Math_BF::cheb_eval(&US_Math_BF::bj0_cs, 0.125 * y * y - 1.0);
   } else {
      const double z = 32.0 / (y * y) - 1.0;
      double ca = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bm0_cs, z);
      double ct = cheb_eval(&US_Math_BF::_gsl_sf_bessel_amp_phase_bth0_cs, z);
      double cp = bessel_cos_pi4(y, ct / y);
      const double sqrty = sqrt(y);
      const double ampl = (0.75 + ca) / sqrty;
      result = ampl * cp;
      return result;
   }
}

double US_Math_BF::bessel_J1(const double &x) {
   double result;
   double y = fabs(x);

   /* CHECK_POINTER(result) */

   // underflow error or J1(0) = 0
   if (y < 2.0 * GSL_DBL_MIN) {
      result = 0.0;
      return result;
   } else if (y < ROOT_EIGHT * GSL_SQRT_DBL_EPSILON) {
      result = 0.5 * x;
      return result;
   } else if (y < 4.0) {
      double c;
      c = cheb_eval(&US_Math_BF::bj1_cs, 0.125 * y * y - 1.0);
      result = x * (0.25 + c);
      return result;
   } else {
      /* Because the leading term in the phase is y,
       * which we assume is exactly known, the error
       * in the cos() evaluation is bounded.
       */
      const double z = 32.0 / (y * y) - 1.0;
      double ca;
      double ct;
      double sp;
      ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs, z);
      ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      sp = bessel_sin_pi4(y, ct / y);
      const double sqrty = sqrt(y);
      const double ampl = (0.75 + ca) / sqrty;
      result = (x < 0.0 ? -ampl : ampl) * sp;
      return result;
   }
}

double US_Math_BF::bessel_Y0(const double &x) {
   double result;
   const double two_over_pi = 2.0 / M_PI;
   const double xmax = 1.0 / GSL_DBL_EPSILON;

   /* CHECK_POINTER(result) */

   if (x <= 0.0) { // domain error
      result = NAN;
      return result;
   } else if (x < 4.0) {
      double J0;
      double c;
      J0 = bessel_J0(x);
      c = cheb_eval(&by0_cs, 0.125 * x * x - 1.0);
      result = two_over_pi * (-M_LN2 + log(x)) * J0 + 0.375 + c;
      return result;
   } else if (x < xmax) {
      /* Leading behaviour of phase is x, which is exact,
       * so the error is bounded.
       */
      const double z = 32.0 / (x * x) - 1.0;
      double c1;
      double c2;
      double sp;
      c1 = cheb_eval(&_gsl_sf_bessel_amp_phase_bm0_cs, z);
      c2 = cheb_eval(&_gsl_sf_bessel_amp_phase_bth0_cs, z);
      sp = bessel_sin_pi4(x, c2 / x);
      const double sqrtx = sqrt(x);
      const double ampl = (0.75 + c1) / sqrtx;
      result = ampl * sp;
      return result;
   } else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::bessel_Y1(const double &x) {
   double result;
   const double two_over_pi = 2.0 / M_PI; // = 0.6366198
   const double xmin = 1.571 * GSL_DBL_MIN; /*exp ( amax1(alog(r1mach(1)), -alog(r1mach(2)))+.01)  = 3.495591E-308*/
   const double x_small = 2.0 * GSL_SQRT_DBL_EPSILON; // = 2.980232E-8
   const double xmax = 1.0 / GSL_DBL_EPSILON; // = 4.5036E15

   /* CHECK_POINTER(result) */

   if (x <= 0.0) { // domain error
      result = NAN;
      return result;
   } else if (x < xmin) { // overflow error
      result = INFINITY;
      return result;
   } else if (x < x_small) {
      const double lnterm = log(0.5 * x);
      double J1;
      double c;
      J1 = bessel_J1(x);
      c = cheb_eval(&by1_cs, -1.0);
      result = two_over_pi * lnterm * J1 + (0.5 + c) / x;
      return result;
   } else if (x < 4.0) {
      const double lnterm = log(0.5 * x);
      double J1;
      double c;
      c = cheb_eval(&by1_cs, 0.125 * x * x - 1.0);
      J1 = bessel_J1(x);
      result = two_over_pi * lnterm * J1 + (0.5 + c) / x;
      return result;
   } else if (x < xmax) {
      const double z = 32.0 / (x * x) - 1.0;
      double ca;
      double ct;
      double cp;
      ca = cheb_eval(&_gsl_sf_bessel_amp_phase_bm1_cs, z);
      ct = cheb_eval(&_gsl_sf_bessel_amp_phase_bth1_cs, z);
      cp = bessel_cos_pi4(x, ct / x);
      const double sqrtx = sqrt(x);
      const double ampl = (0.75 + ca) / sqrtx;
      result = -ampl * cp;
      return result;
   } else { // underflow error
      result = 0.0;
      return result;
   }
}

double US_Math_BF::transcendental_equation(const double &x, const double &a, const double &b) {
   double xa = x * a;
   double xb = x * b;
   double y1 = bessel_J1(xa) * bessel_Y1(xb);
   double y2 = bessel_J1(xb) * bessel_Y1(xa);
   return y1 - y2;
}






