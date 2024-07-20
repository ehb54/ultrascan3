#ifndef US_AVERAGE_H
#define US_AVERAGE_H

#include <QString>
#include <vector>

using namespace std;

class US_Average {
 public:
   US_Average();
   ~US_Average();

   enum weight_methods : int {
      NO_WEIGHTS = 0
      ,ONE_OVER_AMOUNT = 1
      ,ONE_OVER_AMOUNT_SQ = 2
      // how does a response differ from an amount?
      // ,ONE_OVER_RESPONSE
      // ,ONE_OVER_RESPONSE_SQ
      // is RSD (relative SD) different than SD?
      // The relative standard deviation (RSD) is often times more convenient. It is expressed in percent and is obtained by multiplying the standard deviation by 100 and dividing this product by the average.
      // ,ONE_OVER_RSD
      // ,ONE_OVER_RSD_SQ
      ,ONE_OVER_SD = 3
      ,ONE_OVER_SD_SQ = 4
   };
   
   static bool compute(
                       enum weight_methods method
                       ,const vector < double > & xv
                       ,const vector < double > & sdv
                       ,double & avg
                       ,double & sd
                       ,QString & errormsg
                       );

   bool test();
};

#endif

