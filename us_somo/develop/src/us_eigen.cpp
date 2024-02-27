#include "us_eigen.h"

QString US_Eigen::errors() {
   return error_text;
}

bool US_Eigen::evaluate_polynomial(
                                   const vector < double > & coeff
                                   ,const double x_start
                                   ,const double x_end
                                   ,const int points
                                   ,vector < double > & x
                                   ,vector < double > & y
                                   ) {
   error_text = "";
   if ( x_start >= x_end ) {
      error_text = "empty x-range";
      return false;
   }

   if ( points <= 2 ) {
      error_text = "at least 2 points must be specified";
      return false;
   }

   int coeff_size = (int) coeff.size();
   if ( coeff_size == 0 ) {
      error_text = "empty coefficients";
      return false;
   }

   double delta = ( x_end - x_start ) / ( (double) points - 1 );

   x.resize( points );
   y.resize( points );

   for ( double pos = x_start, i = 0; pos <= x_end && i < points; pos += delta, ++i ) {
      x[ i ] = pos;
      double this_y = coeff[0];
      for ( int i = 1; i < coeff_size; ++i ) {
         this_y += coeff[i] * pow( pos, i );
      }
      y[ i ] = this_y;
   }

   return true;
}

void US_Eigen::polyfit(
                       const std::vector<double> &t,
                       const std::vector<double> &v,
                       std::vector<double> &coeff,
                       int order
                       ) {
   // Create Matrix Placeholder of size n x k, n= number of datapoints, k = order of polynomial, for exame k = 3 for cubic polynomial
   Eigen::MatrixXd T(t.size(), order + 1);
   Eigen::VectorXd V = Eigen::VectorXd::Map(&v.front(), v.size());
   Eigen::VectorXd result;

   // check to make sure inputs are correct
   assert(t.size() == v.size());
   assert((int) t.size() >= order + 1);
   // Populate the matrix
   for(size_t i = 0 ; i < t.size(); ++i)
   {
      for(size_t j = 0; j < (size_t)order + 1; ++j)
      {
         T(i, j) = pow(t.at(i), j);
      }
   }
   std::cout<<T<<std::endl;
	
   // Solve for linear least square fit
   result  = T.householderQr().solve(V);
   coeff.resize(order+1);
   for (int k = 0; k < order+1; ++k) {
      coeff[k] = result[k];
   }
}

int US_Eigen::test() {
   // time value
   std::vector<double> time = {0, 0.0192341804504395, 0.0394501686096191,  0.059575080871582, 0.0790810585021973, 0.0792751312255859, 0.0987141132354736,  0.119336366653442,  0.138712167739868,  0.159000158309937,  0.178890228271484,   0.19960618019104,  0.219112157821655,   0.23919415473938,  0.259442090988159,  0.279186248779297,  0.299112319946289,  0.319219350814819,  0.339494228363037,  0.339675188064575,  0.359552145004272,   0.37941837310791,  0.399189233779907,  0.419828176498413,  0.439810276031494,  0.459331274032593,  0.479461193084717,  0.499663114547729,  0.519809246063232,  0.539092063903809,  0.559118270874023,  0.579315185546875,  0.598889112472534,  0.619685173034668,  0.638863086700439,  0.639052152633667,  0.658920288085938,  0.679149150848389,  0.699787139892578,   0.71905517578125,   0.73898720741272,  0.739143371582031,  0.758654117584229,  0.779210329055786,  0.799195289611816,  0.819046258926392,  0.839539289474487,   0.85923433303833,   0.87903618812561,  0.899263143539429,  0.919251203536987,  0.939138174057007,  0.959244251251221,  0.979074239730835,  0.998935222625732,   1.01904726028442,    1.0387852191925,   1.03895926475525,   1.05906510353088,   1.07873225212097,   1.09908628463745,   1.11907029151917,   1.13899827003479,   1.15879201889038};
   // velocity value
   std::vector<double> velocity = {1.8, 1.86, 2.03, 2.08, 2.14, 2.14, 2.25, 2.36, 2.42, 2.59,  2.7, 2.81, 2.87, 3.04, 3.15, 3.26, 3.32, 3.43, 3.54, 3.54,  3.6, 3.71, 3.83, 3.94, 4.11, 4.22, 4.33, 4.44, 4.56, 4.67, 4.78, 4.84, 4.84, 4.89, 4.89, 4.89, 4.95, 5.01, 5.06, 5.06, 5.06, 5.06, 5.01, 5.06, 5.12, 5.18, 5.18, 5.23, 5.23, 5.23, 5.29, 5.34, 5.29,  5.4,  5.4, 5.46, 5.51, 5.51, 5.51, 5.46,  5.4, 5.34, 5.34, 5.34};

   // placeholder for storing polynomial coefficient
   std::vector<double> coeff;
   polyfit(time, velocity, coeff, 3);

   std::vector<double> fitted_velocity;
   std::cout<< "Printing fitted values" << std::endl;
   for(int p = 0; p < (int)time.size(); ++ p)
   {
      double vfitted = coeff[0] + coeff[1]*time.at(p) + coeff[2]*(pow(time.at(p), 2)) +coeff[3]*(pow(time.at(p), 3)) ;
      std::cout<< vfitted<<", ";
      fitted_velocity.push_back(vfitted);
   }
   std::cout<<std::endl;
   return 0;
}   

bool US_Eigen::polyfit(
                       const vector < double > & x
                       ,const vector < double > & y
                       ,const int degree
                       ,vector < double > & coeff // the polynomial coefficients
                       ,double & chi2
                       ,fit_methods fit_method
                       ,weight_methods weight_method
                       ) {

   error_text = "";
   vector < double > e(x.size(),1);
   return polyfit( x, y, e, degree, coeff, chi2, fit_method, weight_method );
}

bool US_Eigen::polyfit(
                       const vector < double > & x_input
                       ,const vector < double > & y_input
                       ,const vector < double > & e_input
                       ,const int degree
                       ,vector < double > & coeff // the polynomial coefficients
                       ,double & chi2
                       ,fit_methods fit_method
                       ,weight_methods weight_method
                       ) {
   error_text = "";

   using namespace Eigen;
   
   int numCoefficients = degree + 1;
   size_t nCount = x_input.size();
    
   // fix up weights 1/variance

   vector < double > weights = e_input;

   // probably could swap the for and switch ... does compiler optimization do this?
   
   for ( size_t i = 0; i < weights.size(); ++i ) {
      double & w = weights[i];

      switch ( weight_method ) {
      case EIGEN_NO_WEIGHTS :
         w = 1;
         break;

      case EIGEN_1_OVER_AMOUNT :
         if ( y_input[i] == 0 ) {
            error_text = "zero amount";
            return false;
         }
         w = 1. / y_input[i];
         break;
         
      case EIGEN_1_OVER_AMOUNT_SQ :
         if ( y_input[i] == 0 ) {
            error_text = "zero amount";
            return false;
         }
         w = 1. / (y_input[i] * y_input[i]);
         break;
         
      case EIGEN_1_OVER_SD :
         if ( w <= 0 ) {
            error_text = "zero or negative SD";
            return false;
         }
         w = 1. / w;
         break;
         
      case EIGEN_1_OVER_SD_SQ :
         if ( w <= 0 ) {
            error_text = "zero or negative SD";
            return false;
         }
         w = 1. / (w * w);
         break;

      default :
         error_text = "unknown or unsupported weight method";
         break;
      }
   }

   MatrixXf X(nCount, numCoefficients);
   MatrixXf Y(nCount, 1);
    
   // fill Y matrix
   for (size_t i = 0; i < nCount; ++i) {
      Y(i, 0) = y_input[i] * weights[i];
   }
    
   // fill X matrix (Vandermonde matrix)
   for (size_t nRow = 0; nRow < nCount; ++nRow) {
      double nVal = 1.0;
      for (int nCol = 0; nCol < numCoefficients; ++nCol) {
         X(nRow, nCol) = nVal * weights[nRow];
         nVal *= x_input[nRow];
      }
   }
    
   VectorXf coefficients;

   switch ( fit_method ) {
   case EIGEN_SVD_JACOBI :
      coefficients = X.jacobiSvd(ComputeThinU | ComputeThinV).solve(Y);
      // qDebug() << "svd_jacobi degree " << degree;
      break;

   case EIGEN_SVD_BDC :
      coefficients = X.bdcSvd(ComputeThinU | ComputeThinV).solve(Y);
      // qDebug() << "svd_bdc " << degree;
      break;

   case EIGEN_HOUSEHOLDER_QR :
      coefficients = X.householderQr().solve(Y);
      // qDebug() << "householder_qr " << degree;
      break;

   case EIGEN_HOUSEHOLDER_QR_PIVOT_COL :
      coefficients = X.colPivHouseholderQr().solve(Y);
      // qDebug() << "householder_qr_pivot_col " << degree;
      break;

   case EIGEN_HOUSEHOLDER_QR_PIVOT_FULL :
      coefficients = X.fullPivHouseholderQr().solve(Y);
      // qDebug() << "householder_qr_pivot_full " << degree;
      break;

   case EIGEN_NORMAL :
      coefficients = (X.transpose() * X).ldlt().solve(X.transpose() * Y);
      // qDebug() << "LR " << degree;
      break;
   }

   coeff = std::vector<double>(coefficients.data(), coefficients.data() + numCoefficients);

   chi2 = 0;
   size_t y_input_size = y_input.size();
   size_t coeff_size   = coeff.size();
   
   for ( size_t i = 0; i < y_input_size; ++i ) {
      double y_model = coeff[0];
      for ( size_t j = 1; j < coeff_size; ++j ) {
         y_model += coeff[j] * pow( x_input[i], j );
      }
      // QTextStream(stdout) << QString( "chi2 calc x %1 y obs %2  y model %3\n" )
      //    .arg( x_input[i] )
      //    .arg( y_input[i] )
      //    .arg( y_model )
      //    ;
      chi2 += (y_input[i] - y_model)*(y_input[i] - y_model) / (e_input[i] * e_input[i]);
   }

   return true;
}

QString US_Eigen::gnuplot_poly( const vector < double > & coeff, QString fname ) {
   QString out = QString( "%1(x) =" ).arg( fname );
   if( !coeff.size() ) {
      return out + " 0\n";
   }

   out += QString( " %1" ).arg( coeff[0], 0, 'g', 12 );

   if ( coeff.size() > 1 ) {
      out += QString( " + %1*x" ).arg( coeff[1], 0, 'g', 12 );
   }

   for ( size_t i = 2; i < coeff.size(); ++i ) {
      out += QString( " + %1*x**%2" ).arg( coeff[i], 0, 'g', 12 ).arg( i );
   }

   out += "\n";

   return out;
}

QString US_Eigen::qs_fit_method( enum fit_methods fit_method ) {
   switch( fit_method ) {
   case US_Eigen::EIGEN_SVD_BDC :
      return "SVD_BDC";
      break;
   case US_Eigen::EIGEN_SVD_JACOBI :
      return "SVD_Jacobi";
      break;
   case US_Eigen::EIGEN_HOUSEHOLDER_QR_PIVOT_FULL :
      return "QR_Householder_full_pivoting";
      break;
   case US_Eigen::EIGEN_HOUSEHOLDER_QR_PIVOT_COL :
      return "QR_Householder_column_pivoting";
      break;
   case US_Eigen::EIGEN_HOUSEHOLDER_QR :
      return "QR_Householder";
      break;
   case US_Eigen::EIGEN_NORMAL :
      return "LR";
      break;
   default :
      return "unknown";
      break;
   }
}

QString US_Eigen::qs_weight_method( enum weight_methods weight_method ) {
   switch( weight_method ) {
   case US_Eigen::EIGEN_NO_WEIGHTS :
      return "none";
      break;
   case US_Eigen::EIGEN_1_OVER_AMOUNT :
      return "1/amount";
      break;
   case US_Eigen::EIGEN_1_OVER_AMOUNT_SQ :
      return "1/amount^2";
      break;
   case US_Eigen::EIGEN_1_OVER_SD :
      return "1/SD";
      break;
   case US_Eigen::EIGEN_1_OVER_SD_SQ :
      return "1/SD^2";
      break;
   default :
      return "unknown";
      break;
   }
}


