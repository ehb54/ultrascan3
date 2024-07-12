#ifndef US_FFD_H
#define US_FFD_H

#include "us_extern.h"
#include "us_hydrodyn_pdbdefs.h"
#include "qdebug.h"
#include "us_timer.h"

#include <vector>
#include <map>
#include <set>

typedef double ftype;

struct ffd_saxs_coeff {
   vector < ftype >    a;               // a coefficients
   vector < ftype >    b;               // b coefficients
   ftype               c;               // c coefficient
   ftype               volume;          // atomic volume (excluded volume?)
};

struct fifj {
   vector < ftype > a;
   vector < ftype > b;
   ftype            c;
};

class US_EXTERN US_FFD
{
   public:
      US_FFD( int coeff_terms );
      ~US_FFD();
      QString errormsg;

      bool set_saxs_coeffs( vector < saxs > & saxs_coeffs );
      bool set_saxs_coeffs( const QString & filename );

      QString list_saxs_coeffs( set < QString > filter = {} );
      QString list_fifj( set < QString > filter = {} );

      bool set_q_grid( const vector < double > & q );
      bool set_q_grid( const vector < float > & q );
      bool set_q_grid( ftype qstart, ftype qend, ftype increment );

      bool test();
      bool calc_fq();
      bool calc_fifjq();

   private:
      vector < saxs > *saxs_coeffs;
      map < QString, ffd_saxs_coeff > saxs_coeff_map;
      int coeff_terms;

      map < set < QString >, fifj > fifj_map;

      vector < ftype > q;

      map < QString, map < ftype, ftype > > fq_map;
      map < set < QString >, map < ftype, ftype > > fifjq_map;

      bool make_saxs_coeff_map();
      void test_resize_saxs_coeff( int size, ftype val = 1 );

      ftype calc_I( const QString & name, ftype q );
      ftype calc_I( const set < QString > & index, ftype q );

      bool init_fifj(); // compute coeffs for 4 term gaussians // ATTIC

      US_Timer    timers;
};


#endif
