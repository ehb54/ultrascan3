#ifndef US_FRACTAL_DIMENSION_H
#define US_FRACTAL_DIMENSION_H

#include "us_hydrodyn_pdbdefs.h"
#include <set>
#include <qdebug.h>

#define UNICODE_ANGSTROM u8"\u212B"
#define UNICODE_ANGSTROM_QS QString( "%1" ).arg( UNICODE_ANGSTROM )

using namespace std;

class US_Fractal_Dimension {
 public:
   US_Fractal_Dimension();
   ~US_Fractal_Dimension();

   bool compute_box_counting(
                             const vector < point > & points
                             ,double                box_angstrom_start
                             ,double                box_angstrom_end
                             ,double                box_angstrom_steps
                             ,double                & fd
                             ,QString               & errormsg
                             ,vector < double >     & x
                             ,vector < double >     & y
                             );

 private:

};

#endif
