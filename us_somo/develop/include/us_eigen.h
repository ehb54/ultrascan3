#ifndef US_EIGEN_H
#define US_EIGEN_H

#include "Eigen/Dense"
#include <iostream>
#include <cmath>
#include <vector>
#include "Eigen/QR"

#include <qstring.h>
#include <qtextstream.h>
#include <qdebug.h>

using namespace std;


class US_Eigen {
 public:

   enum fit_methods {
      EIGEN_SVD_JACOBI
      ,EIGEN_SVD_BDC
      ,EIGEN_HOUSEHOLDER_QR
      ,EIGEN_HOUSEHOLDER_QR_PIVOT_COL
      ,EIGEN_HOUSEHOLDER_QR_PIVOT_FULL
      ,EIGEN_NORMAL
   };

   bool polyfit(
                const vector < double > & x
                ,const vector < double > & y
                ,const vector < double > & e
                ,const int degree
                ,vector < double > & coeff // the polynomial coefficients
                ,double & chi2
                ,enum fit_methods fit_method = EIGEN_HOUSEHOLDER_QR_PIVOT_COL
            );

   bool polyfit(
                const vector < double > & x
                ,const vector < double > & y
                ,const int degree
                ,vector < double > & coeff // the polynomial coefficients
                ,double & chi2
                ,enum fit_methods fit_method = EIGEN_HOUSEHOLDER_QR_PIVOT_COL
                );

   bool evaluate_polynomial(
                            const vector < double > & coeff
                            ,const double x_start
                            ,const double x_end
                            ,const int points
                            ,vector < double > & x
                            ,vector < double > & y
                            );

   QString errors();
   
   static QString gnuplot_poly( const vector < double > & coeff, QString fname = "f" );

   int test();

 private:
   QString error_text;

   void polyfit(
                const std::vector<double> &t,
                const std::vector<double> &v,
                std::vector<double> &coeff,
                int order
                );
   
};
   
   
#endif // US_EIGEN_H
