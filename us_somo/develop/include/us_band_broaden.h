// band broadening code

#ifndef US_BAND_BROADEN_H
#define US_BAND_BROADEN_H

#include <vector>
#include <cmath>
#include <unordered_map>
#include <functional>
#include <QString>
#include <cfloat>

using namespace std;

// #define BROADEN_TESTING

class US_Band_Broaden {
 public:

   enum kernel_type : int {
      BAND_BROADEN_KERNEL_EXPONENTIAL = 0
     ,BAND_BROADEN_KERNEL_GAUSSIAN    = 1
   };
   
   static QString kernel_type_name( const enum kernel_type & ktype );

   US_Band_Broaden( bool debug = false );

   // broaden with both exponential & gaussian kernels
   vector < double > broaden(
                             const vector < double > & f
                             ,const double & tau_e
                             ,const double & tau_g
                             ,const double & time_start = 0
                             ,const double & time_end   = 100
                             ,const double & time_delta = 0.1
                             );

   // broaden with a defined kernel
   vector < double > broaden(
                             const vector < double > & f
                             ,const double           & tau
                             ,const enum kernel_type & type = US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL
                             ,const double & time_start = 0
                             ,const double & time_end   = 100
                             ,const double & time_delta = 0.1
                             );

   void clear();

   QString errormsg;

#if defined( BROADEN_TESTING )
   void test();

   void test( double tau, double kernel_time_end, double kernel_time_delta, enum kernel_type ktype  );
#endif

 private:

   bool debug;

   unordered_map < double, vector < double > > K1;
   unordered_map < double, double > K1_sum_inverse;
   
   vector < double > convolve(
                              const vector < double > & f
                              ,const vector < double > & g
                              ,const enum kernel_type & ktype
                              );

   bool build_kernel(
                     const double & tau
                     ,const enum kernel_type & type = US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL
                     ,const double & time_start = 0
                     ,const double & time_end   = 100
                     ,const double & time_delta = 0.05
                     );

   size_t peak( const vector < double > & f );

};

#endif
