//! \file us_zsolute.h
#ifndef US_ZSOLUTE_H
#define US_ZSOLUTE_H

#include <QtCore>

#include "us_extern.h"
#include "us_model.h"

//! \brief Solutes for UltraScan (XYZ form)
//!
//! This class provides a solute structure and utilities

class US_UTIL_EXTERN US_ZSolute {
 public:
  //! \param x0 The initial X (e.g.,sedimentation)
  //! \param y0 The initial Y (e.g., frictional ratio)
  //! \param z0 The initial Z (e.g., vbar20)
  //! \param c0 The initial concentration
  US_ZSolute(double = 0.0, double = 0.0, double = 0.0, double = 0.0);

  double x;  //!< X value
  double y;  //!< Y value
  double z;  //!< Z value
  double c;  //!< Concentration

  //! A test for solute equality
  //! \param solute A value for comparison
  bool operator==(const US_ZSolute& solute) {
    return (x == solute.x && y == solute.y && z == solute.z);
  }

  //! A test for inequality.
  //! \param solute A value for comparison
  bool operator!=(const US_ZSolute& solute) {
    return (x != solute.x || y != solute.y || z != solute.z);
  }

  //! A test for ordering solutes.  Tests in x,y,z order.
  //! \param solute A value for comparison
  bool operator<(const US_ZSolute& solute) const {
    if (x < solute.x)
      return true;

    else if (x == solute.x && y < solute.y)
      return true;

    else if (x == solute.x && y == solute.y && z < solute.z)
      return true;

    else
      return false;
  }

  //! The types of attributes that map to XYZ
  enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_V, ATTR_D, ATTR_C = 9 };

  //! A static function to put a model component attribute value
  //! \param comp   Model component in which to put value
  //! \param aval   Attribute value to put
  //! \param a_type Attribute type to put (ATTR_S,...,ATTR_D).
  static void put_mcomp_attr(US_Model::SimulationComponent&, double&,
                             const int);

  //! A static function to put a solute attribute value
  //! \param solute Solute in which to put value
  //! \param aval   Attribute value to put
  //! \param a_type Attribute type to put (0,1,2 -> X,Y,Z).
  static void put_solute_attr(US_ZSolute&, double&, const int);

  //! A static function to get a model component attribute value
  //! \param comp   Model component from which to get value
  //! \param aval   Attribute value fetched
  //! \param a_type Attribute type to get (ATTR_S,...,ATTR_D).
  static void get_mcomp_attr(US_Model::SimulationComponent&, double&,
                             const int);

  //! A static function to get a solute attribute value
  //! \param solute Solute from which to get value
  //! \param aval   Attribute value fetched
  //! \param a_type Attribute type to get (0,1,2 -> X,Y,Z).
  static void get_solute_attr(US_ZSolute&, double&, const int);

  //! A static function to set model component attribute values
  //! \param comp   Model component in which to set values
  //! \param solute Solute from which to set values
  //! \param s_type Solute type mask
  //! \param concv  Flag to copy concentration value, too
  static void set_mcomp_values(US_Model::SimulationComponent&, US_ZSolute&,
                               const int, const bool = false);

  //! A static function to set solute attribute values
  //! \param comp   Model component from which to set values
  //! \param solute Solute to which to set values
  //! \param s_type Solute type mask
  static void set_solute_values(US_Model::SimulationComponent&, US_ZSolute&,
                                const int);

  //! A static function to initialize grid solutes
  //! \param x_min The minimum X value
  //! \param x_max The maximum X value
  //! \param x_res The number of points to evaluate between x_min and x_max
  //! \param y_min The minimum Y value
  //! \param y_max The maximum Y value
  //! \param y_res The number of points to evaluate between y_min and y_max
  //! \param grid_reps The number of grids used to partition the data
  //! \param z_coeffs  Z coefficients for compute-Z function
  //! \param solutes   A reference to the output values.  This is a
  //!                  list of vectors.
  static void init_grid_solutes(double, double, int, double, double, int, int,
                                double*, QList<QVector<US_ZSolute> >&);

 private:
  // Internal function to set a Z-coefficients function flag:
  //  0 -> constant, 1 -> linear, 2 -> polynomial
  static int zcoeff_flag(double*);

  // Internal function to create one vector of solutes
  static QVector<US_ZSolute> create_grid_solutes(double, double, double, double,
                                                 double, double, double*);
};
#endif
