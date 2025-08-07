//! \file us_solute.h
#ifndef US_SOLUTE_H
#define US_SOLUTE_H

#include <QtCore>

#include "us_extern.h"

//! \brief Solutes for UltraScan
//!
//! This class provides a solute structure and utilities

class US_UTIL_EXTERN US_Solute {
 public:
  //! \param s0 The initial sedimentation
  //! \param k0 The initial frictional ratio
  //! \param c0 The initial concentration
  //! \param v0 The initial vbar
  //! \param d0 The initial diffusion coefficient (or MW or f)
  US_Solute(double = 0.0, double = 0.0, double = 0.0, double = 0.0,
            double = 0.0);

  double s;  //!< Sedimentation value
  double k;  //!< Frictional ratio
  double c;  //!< Concentration
  double v;  //!< Vbar
  double d;  //!< Diffusion coefficient (or Molec.Weight or Fric.Coeff.)

  //! A test for solute equality
  //! \param solute A value for comparison
  bool operator==(const US_Solute& solute) const {
    return (s == solute.s && k == solute.k && v == solute.v && d == solute.d);
  }

  //! A test for inequality.
  //! \param solute A value for comparison
  bool operator!=(const US_Solute& solute) const {
    return (s != solute.s || k != solute.k || v != solute.v || d != solute.d);
  }

  //! A test for ordering solutes.  Tests sedimentation before frictional
  //! ratio.
  //! \param solute A value for comparison
  bool operator<(const US_Solute& solute) const {
    if (s < solute.s)
      return true;

    else if (s == solute.s && k < solute.k)
      return true;

    else if (s == solute.s && k == solute.k && d < solute.d)
      return true;

    else if (s == solute.s && k == solute.k && d == solute.d && v < solute.v)
      return true;

    else
      return false;
  }

  //! A static function to initialize solutes
  //! \param s_min The minimum sedimentation value
  //! \param s_max The maximum sedimentation value
  //! \param s_res The number of ponts to evaluate between s_min and s_max
  //! \param ff0_min The minimum frictional ratio
  //! \param ff0_max The maximum frictional ratio
  //! \param ff0_res The number of ponts to evaluate between ff0_min and
  //!                ff0_max
  //! \param grid_reps The number of grids used to partition the data
  //! \param cnstff0   Constant f/f0 (or 0.0 if vbar is constant)
  //! \param solute_list A reference to the output values.  This is a
  //!                    list of vectors.
  static void init_solutes(double, double, int, double, double, int, int,
                           double, QList<QVector<US_Solute> >&);

  static QVector<US_Solute> create_solutes(double s_min, double s_max,
                                           double s_step, double ff0_min,
                                           double ff0_max, double ff0_step,
                                           double cnstff0);
};
#endif
