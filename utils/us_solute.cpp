//! \file us_solute.cpp

#include "us_solute.h"
#include "us_math2.h"

US_Solute::US_Solute(double s0, double k0, double c0, double v0, double d0) {
   s = s0;
   k = k0;
   c = c0;
   v = v0;
   d = d0;
}

void US_Solute::init_solutes(
   double s_min, double s_max, int s_res, double ff0_min, double ff0_max, int ff0_res, int grid_reps, double cnstff0,
   QList<QVector<US_Solute>> &solute_list) {
   grid_reps = qMax(grid_reps, 1);
   int nprs = qMax(1, (s_res - 1));
   int nprk = qMax(1, (ff0_res - 1));
   double s_grid = qAbs(s_max - s_min) / ( double ) nprs;
   double ff0_grid = qAbs(ff0_max - ff0_min) / ( double ) nprk;
   double s_step = s_grid * grid_reps;
   double ff0_step = ff0_grid * grid_reps;
   qDebug() << "InSo: nprs nprk" << nprs << nprk << "s_step k_step" << s_step * 1.e+13 << ff0_step << "s_grid k_grid"
            << s_grid * 1.e+13 << ff0_grid;

   // Allow a 1% overscan
   s_max += 0.01 * s_step;
   ff0_max += 0.01 * ff0_step;

   solute_list.reserve(sq(grid_reps));

   // Generate solutes for each grid repetition
   for (int js = 0; js < grid_reps; js++) {
      double s_min_g = s_min + s_grid * js;
      for (int jk = 0; jk < grid_reps; jk++) {
         double k_min_g = ff0_min + ff0_grid * jk;
         solute_list << create_solutes(s_min_g, s_max, s_step, k_min_g, ff0_max, ff0_step, cnstff0);
      }
   }
}

QVector<US_Solute> US_Solute::create_solutes(
   double s_min, double s_max, double s_step, double ff0_min, double ff0_max, double ff0_step, double cnstff0) {
   QVector<US_Solute> solute_vector;
   double off0 = cnstff0;
   double ovbar = 0.0;

   for (double ff0 = ff0_min; ff0 <= ff0_max; ff0 += ff0_step) {
      if (cnstff0 > 0.0)
         ovbar = ff0;
      else
         off0 = ff0;

      for (double svl = s_min; svl <= s_max; svl += s_step) {
         // Omit s values close to zero.
         if (svl >= -5.0e-15 && svl <= 5.0e-15)
            continue;

         solute_vector << US_Solute(svl, off0, 0.0, ovbar);
      }
   }

   return solute_vector;
}
