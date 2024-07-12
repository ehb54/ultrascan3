#include "../include/us_ffd.h"
#include <math.h>

#define FFD_REMOVE_DUP_B // to trim products
#define TSO QTextStream(stdout)
#define TSE QTextStream(stderr)

US_FFD::US_FFD( int coeff_terms ) {
   TSO << "US_FFD::US_FFD() start\n";

   errormsg    = "";
   this->coeff_terms = coeff_terms;
   saxs_coeffs = (vector < saxs > *)0;

   // these are be cleared by default, but made explicit here
   saxs_coeff_map.clear();
   fifj_map.clear();
   q.clear();
}

US_FFD::~US_FFD() {
}

bool US_FFD::set_saxs_coeffs( vector < saxs > & saxs_coeffs ) {
   errormsg = "";
   this->saxs_coeffs = & saxs_coeffs;
   return make_saxs_coeff_map();
}

bool US_FFD::set_saxs_coeffs( const QString & /* filename */ ) {
   errormsg = "US_FFD::set_saxs_coeff( const QString & filename ) :: not yet implemented";
   qDebug() << errormsg;
   return false;
}

bool US_FFD::make_saxs_coeff_map() {
   if ( coeff_terms != 4 &&
        coeff_terms != 5 ) {
      errormsg = QString( "US_FFD::make_saxs_coeff_map() : %1 terms not supported\n" ).arg( coeff_terms );
      qDebug() << errormsg;
      return false;
   }
   timers.clear_timers();
   timers.init_timer( "make_saxs_coeff_map()" );
   timers.start_timer( "make_saxs_coeff_map()" );

   saxs_coeff_map.clear();
   ffd_saxs_coeff tmp;
   tmp.a.resize( coeff_terms );
   tmp.b.resize( coeff_terms );

   switch ( coeff_terms ) {
   case 4 :
      for ( int i = 0; i < (int) saxs_coeffs->size(); ++i ) {
         for ( int j = 0; j < 4; ++j ) {
            tmp.a[j]     = (*saxs_coeffs)[i].a[j];
            tmp.b[j]     = (*saxs_coeffs)[i].b[j];
         }
         tmp.c      = (*saxs_coeffs)[i].c;
         tmp.volume = (*saxs_coeffs)[i].volume;
         saxs_coeff_map[ (*saxs_coeffs)[i].saxs_name ] = tmp;
      }
      break;
   case 5 :
      for ( int i = 0; i < (int) saxs_coeffs->size(); ++i ) {
         for ( int j = 0; j < 5; ++j ) {
            tmp.a[j]     = (*saxs_coeffs)[i].a5[j];
            tmp.b[j]     = (*saxs_coeffs)[i].b5[j];
         }
         tmp.c      = (*saxs_coeffs)[i].c5;
         tmp.volume = (*saxs_coeffs)[i].volume;
         saxs_coeff_map[ (*saxs_coeffs)[i].saxs_name ] = tmp;
      }
      break;
   default :
      errormsg = QString( "US_FFD::make_saxs_coeff_map() : unexpected internal error - %1 terms not supported\n" ).arg( coeff_terms );
      qDebug() << errormsg;
      return false;
   }

   timers.end_timer( "make_saxs_coeff_map()" );
   TSO << timers.list_times();
   
   TSO << "saxs_coeff_map.size() " << saxs_coeff_map.size() << "\n";
   return true;
}

QString US_FFD::list_saxs_coeffs( set < QString > filter ) {
   QStringList result;
   for ( auto it = saxs_coeff_map.begin();
         it != saxs_coeff_map.end();
         ++it ) {
      if ( !filter.size()
           || filter.count( it->first ) ) {

         QString qs = QString( "%1 :" ).arg( it->first );

         qs += "   terms:\n";

         ftype suma = it->second.c;
         for ( int i = 0; i < coeff_terms; ++i ) {
            qs += QString( " %1 %2" ).arg( it->second.a[i] ).arg( it->second.b[i] );
            suma += it->second.a[i];
         }
         qs += QString( " %1\n" ).arg( it->second.c );
         qs += QString( "   excl vol: %1\n" ).arg( it->second.volume );
         qs += QString( "   q0 value: %1" ).arg( suma );
         
         result << qs;
      }
   }
   return result.join( "\n" ) + "\n";
}

QString US_FFD::list_fifj( set < QString > filter ) {
   QStringList result;
   for ( auto it = fifj_map.begin();
         it != fifj_map.end();
         ++it ) {
      if ( !filter.size()
#warning list_fifj filter not properly set
           // || filter.count( it->first )
           ) {

         QString name;
         for ( auto & n : it->first ) {
            name += n + " ";
         }
         QString qs = QString( "%1 :" ).arg( name );
         int fifj_terms = (int) it->second.a.size();

         qs += QString( "  %1 terms:\n" ).arg( fifj_terms );

         ftype suma = it->second.c;
         for ( int i = 0; i < fifj_terms; ++i ) {
            qs += QString( " %1 %2" ).arg( it->second.a[i] ).arg( it->second.b[i] );
            suma += it->second.a[i];
         }
         qs += QString( " %1\n" ).arg( it->second.c );
         // qs += QString( "   excl vol: %1\n" ).arg( it->second.volume );
         qs += QString( "   q0 value: %1" ).arg( suma );
         
         result << qs;
      }
   }
   return result.join( "\n" ) + "\n";
}

bool US_FFD::set_q_grid( const vector < double > &q ) {
   int q_count = (int) q.size();

   this->q.resize( q_count );
   for ( int i = 0; i < q_count; ++i ) {
      this->q[i] = q[i];
   }
   return true;
}

bool US_FFD::set_q_grid( const vector < float > &q ) {
   int q_count = (int) q.size();

   this->q.resize( q_count );
   for ( int i = 0; i < q_count; ++i ) {
      this->q[i] = q[i];
   }
   return true;
}

bool US_FFD::set_q_grid( ftype qstart, ftype qend, ftype increment ) {
   q.clear();

   for ( ftype qi = qstart; qi <= qend; qi += increment ) {
      q.push_back( qi );
   }

   TSO << "US_FFD::set_q_grid : q grid points generated: " << q.size() << "\n";
   return true;
}

bool US_FFD::init_fifj() {
   timers.clear_timers();
   timers.init_timer( "init_fifj()" );
   timers.start_timer( "init_fifj()" );

   fifj_map.clear();

#warning .c is wrong here, needs eV term subtracted

#if defined( FFD_REMOVE_DUP_B )
   int terms_removed = 0;
#endif
   
   for ( auto it = saxs_coeff_map.begin();
         it != saxs_coeff_map.end();
         ++it ) {
      for ( auto it2 = saxs_coeff_map.begin();
            it2 != saxs_coeff_map.end();
            ++it2 ) {
         set < QString > index = { it->first, it2->first };
         if ( fifj_map.count( index ) ) {
            continue;
         }
         
         map < ftype, int > used_b;
         
         for ( int i = 0; i < coeff_terms; ++i ) {
            for ( int j = 0; j < coeff_terms; ++j ) {
               fifj_map[ index ].a.push_back( it->second.a[i] * it2->second.a[j] );
               fifj_map[ index ].b.push_back( it->second.b[i] + it2->second.b[j] );
#if defined( FFD_REMOVE_DUP_B )
               if ( used_b.count( fifj_map[ index ].b.back() ) ) {
                  ++terms_removed;
                  fifj_map[ index ].a[ used_b[ fifj_map[ index ].b.back() ] ] += fifj_map[ index ].a.back();
                  fifj_map[ index ].a.pop_back();
                  fifj_map[ index ].b.pop_back();
               } else {
                  used_b[ fifj_map[ index ].b.back() ] = (int) fifj_map[ index ].a.size() - 1;
               }
#endif
            }
            fifj_map[ index ].a.push_back( it->second.a[i] * it2->second.c );
            fifj_map[ index ].b.push_back( it->second.b[i] );
#if defined( FFD_REMOVE_DUP_B )
            if ( used_b.count( fifj_map[ index ].b.back() ) ) {
               ++terms_removed;
               fifj_map[ index ].a[ used_b[ fifj_map[ index ].b.back() ] ] += fifj_map[ index ].a.back();
               fifj_map[ index ].a.pop_back();
               fifj_map[ index ].b.pop_back();
            } else {
               used_b[ fifj_map[ index ].b.back() ] = (int) fifj_map[ index ].a.size() - 1;
            }
#endif
            fifj_map[ index ].a.push_back( it2->second.a[i] * it->second.c );
            fifj_map[ index ].b.push_back( it2->second.b[i] );
#if defined( FFD_REMOVE_DUP_B )
            if ( used_b.count( fifj_map[ index ].b.back() ) ) {
               ++terms_removed;
               fifj_map[ index ].a[ used_b[ fifj_map[ index ].b.back() ] ] += fifj_map[ index ].a.back();
               fifj_map[ index ].a.pop_back();
               fifj_map[ index ].b.pop_back();
            } else {
               used_b[ fifj_map[ index ].b.back() ] = (int) fifj_map[ index ].a.size() - 1;
            }
#endif
         }
         fifj_map[ index ].c = it->second.c * it2->second.c;
      }
   }
#if defined( FFD_REMOVE_DUP_B )
   qDebug() << "terms removed : " << terms_removed << "\n";
#endif
   timers.end_timer( "init_fifj()" );
   TSO << timers.list_times();

   return true;
}

               
bool US_FFD::test() {
   TSO << "US_FFD::test() start\n";
   vector < saxs > test_coeffs;

   vector < QString > names = { "X", "Y", "Z" };
   
   int ofs = 0;
   
   for ( auto & x : names ) {
      saxs tmp;
      tmp.saxs_name = x;
      ofs += coeff_terms + 2;
      
      switch ( coeff_terms ) {
      case 4 :
         for ( int i = 0; i < coeff_terms; ++i ) {
            tmp.a[i] = ofs + i + 1;
            tmp.b[i] = ofs + i + 1.5;
         }
         tmp.c = ofs + coeff_terms + 1;
         tmp.volume = tmp.c * tmp.c;
         test_coeffs.push_back( tmp );
         break;
      case 5 :
         for ( int i = 0; i < coeff_terms; ++i ) {
            tmp.a5[i] = ofs + i + 1;
            tmp.b5[i] = ofs + i + 1.5;
         }
         tmp.c5 = ofs + coeff_terms + 1;
         tmp.volume = tmp.c5 * tmp.c5;
         test_coeffs.push_back( tmp );
         break;
      default:
         errormsg = QString( "US_FFD::test() : %1 terms not supported\n" ).arg( coeff_terms );
         qDebug() << errormsg;
         return false;
      }
   }

   set_saxs_coeffs( test_coeffs );
   // test_resize_saxs_coeff( 2 );
   init_fifj();

   TSO << "US_FFD::test() list_saxs_coeffs()\n";
   TSO << list_saxs_coeffs();

   TSO << "US_FFD::test() list_fifj()\n";
   TSO << list_fifj();
   
   TSO << "US_FFD::test() test products\n";

   for ( auto & x : names ) {
      for ( auto & y : names ) {
         for ( ftype q = 0; q <= 2; q += .1 ) {
            ftype valx = calc_I( x, q );
            ftype valy = calc_I( y, q );
            ftype valxy = calc_I( { x, y }, q );

            TSO << QString( "%1 %2 q %3 singly : %4 * %5 = %6  fifj : %7  diff : %8\n" )
               .arg( x )
               .arg( y )
               .arg( q )
               .arg( valx )
               .arg( valy )
               .arg( valx * valy )
               .arg( valxy )
               .arg( valx * valy - valxy )
               ;
         }
      }
   }
   
   TSO << "US_FFD::test() end\n";
   return true;
}

ftype US_FFD::calc_I( const QString & name, ftype q ) {
#warning .c is wrong here, needs eV term subtracted
   if ( saxs_coeff_map.count( name ) ) {
      ftype result = saxs_coeff_map[ name ].c;
      ftype mqq = - q * q;
      for ( int i = 0; i < coeff_terms; ++i ) {
         result += saxs_coeff_map[ name ].a[i] * exp( mqq * saxs_coeff_map[ name ].b[i] );
      }
      return result;
   }
   return 0;
}

ftype US_FFD::calc_I( const set < QString > & index, ftype q ) {
   if ( fifj_map.count( index ) ) {
      ftype result = fifj_map[ index ].c;
      ftype mqq = - q * q;
      int terms = (int) fifj_map[ index ].a.size();
      for ( int i = 0; i < terms; ++i ) {
         result += fifj_map[ index ].a[i] * exp( mqq * fifj_map[ index ].b[i] );
      }
      return result;
   }
   return 0;
}

void US_FFD::test_resize_saxs_coeff( int size, ftype val ) {
   for ( auto it = saxs_coeff_map.begin();
         it != saxs_coeff_map.end();
         ++it ) {
      it->second.a.resize( size, val );
      it->second.b.resize( size, val );
   }
   coeff_terms = size;
}

      
bool US_FFD::calc_fq() {
#warning .c is wrong here, needs eV term subtracted

   if ( !saxs_coeff_map.size() ) {
      errormsg = "US_FFD:calc_fq() : saxs_coeff_map is empty";
      qDebug() << errormsg;
      return false;
   }

   if ( !q.size() ) {
      errormsg = "US_FFD:calc_fq() : the grid q is empty";
      qDebug() << errormsg;
      return false;
   }

   timers.clear_timers();
   timers.init_timer( "calc_fq()" );
   timers.start_timer( "calc_fq()" );

   fq_map.clear();

   for ( auto it = saxs_coeff_map.begin();
         it != saxs_coeff_map.end();
         ++it ) {
      int terms = (int) it->second.a.size();
      for ( auto & qi : q ) {
         fq_map[ it->first ][ qi ] = it->second.c;
         ftype mqq = - qi * qi;
         for ( int i = 0; i < terms; ++i ) {
            fq_map[ it->first ][ qi ] += it->second.a[i] * exp( mqq * it->second.b[i] );
         }
      }
   }

   timers.end_timer( "calc_fq()" );
   TSO << timers.list_times();

   return true;
}
   
bool US_FFD::calc_fifjq() {
#warning .c is wrong here, needs eV term subtracted

   if ( !saxs_coeff_map.size() ) {
      errormsg = "US_FFD:calc_fq() : saxs_coeff_map is empty";
      qDebug() << errormsg;
      return false;
   }

   if ( !q.size() ) {
      errormsg = "US_FFD:calc_fq() : the grid q is empty";
      qDebug() << errormsg;
      return false;
   }

   if ( !fq_map.size() ) {
      errormsg = "US_FFD:calc_fq() : fq_map is empty";
      qDebug() << errormsg;
      return false;
   }

   timers.clear_timers();
   timers.init_timer( "calc_fifjq()" );
   timers.start_timer( "calc_fifjq()" );

   fifjq_map.clear();

   for ( auto it = saxs_coeff_map.begin();
         it != saxs_coeff_map.end();
         ++it ) {
      for ( auto it2 = saxs_coeff_map.begin();
            it2 != saxs_coeff_map.end();
            ++it2 ) {
         for ( auto & qi : q ) {
            fifjq_map[ { it->first, it2->first } ][ qi ] =
               fq_map[ it->first ][ qi ] * fq_map[ it2->first ][ qi ];
         }
      }
   }

   timers.end_timer( "calc_fifjq()" );
   TSO << timers.list_times();

   return true;
}
