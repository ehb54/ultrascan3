// This code is a wrapper to the Eigen C++ template library
// As of this writing, the Eigen software is available from https://gitlab.com/libeigen/eigen
// and further information about Eigen is available from https://eigen.tuxfamily.org
// The Eigen Source Code is subject to the terms of the Mozilla Public License, v. 2.0. 
// If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

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

   enum fit_methods : int {
      EIGEN_SVD_BDC = 0
      ,EIGEN_SVD_JACOBI = 1
      ,EIGEN_HOUSEHOLDER_QR_PIVOT_FULL = 2
      ,EIGEN_HOUSEHOLDER_QR_PIVOT_COL = 3
      ,EIGEN_HOUSEHOLDER_QR = 4
      ,EIGEN_NORMAL = 5
   };

   // taken from https://www.azdhs.gov/documents/preparedness/state-laboratory/lab-licensure-certification/technical-resources/calibration-training/11-weighted-least-squares-regression-calib.pdf
   // therein ref "Chromeleon Manual"

   enum weight_methods : int {
      EIGEN_NO_WEIGHTS = 0
      ,EIGEN_1_OVER_AMOUNT = 1
      ,EIGEN_1_OVER_AMOUNT_SQ = 2
      // how does a response differ from an amount?
      // ,EIGEN_1_OVER_RESPONSE
      // ,EIGEN_1_OVER_RESPONSE_SQ
      // is RSD (relative SD) different than SD?
      // The relative standard deviation (RSD) is often times more convenient. It is expressed in percent and is obtained by multiplying the standard deviation by 100 and dividing this product by the average.
      // ,EIGEN_1_OVER_RSD
      // ,EIGEN_1_OVER_RSD_SQ
      ,EIGEN_1_OVER_SD = 3
      ,EIGEN_1_OVER_SD_SQ = 4
   };
   
   bool polyfit(
                const vector < double > & x
                ,const vector < double > & y
                ,const vector < double > & e
                ,const int degree
                ,vector < double > & coeff // the polynomial coefficients
                ,double & chi2
                ,enum fit_methods fit_method = EIGEN_HOUSEHOLDER_QR_PIVOT_COL
                ,enum weight_methods weight_method = EIGEN_1_OVER_SD_SQ
            );

   bool polyfit(
                const vector < double > & x
                ,const vector < double > & y
                ,const int degree
                ,vector < double > & coeff // the polynomial coefficients
                ,double & chi2
                ,enum fit_methods fit_method = EIGEN_HOUSEHOLDER_QR_PIVOT_COL
                ,enum weight_methods weight_method = EIGEN_NO_WEIGHTS
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

   static QString qs_fit_method( enum fit_methods fit_method );
   static QString qs_weight_method( enum weight_methods weight_method );

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
