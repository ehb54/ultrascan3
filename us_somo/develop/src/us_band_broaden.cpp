#include "../include/us_band_broaden.h"

#include <QDebug>
#include <QTextStream>

#define TSO QTextStream( stdout )

vector < double > US_Band_Broaden::convolve( const vector < double > & f, const vector < double > & g ) {
   const size_t nf = f.size();
   const size_t ng = g.size();
   const size_t n = nf + ng - 1;
   vector<double> full_out( n, 0.0 );

   // Compute the full convolution
   for ( size_t i = 0; i < n; ++i ) {
      size_t jmn = ( i >= ng - 1 ) ? i - ( ng - 1 ) : 0;
      size_t jmx = ( i < nf - 1 )  ? i : nf - 1;
      for ( size_t j = jmn; j <= jmx; ++j ) {
         full_out[ i ] += f[ j ] * g[ i - j ];
      }
   }

   // Adjust the output to match the size of f
   vector < double > out( f.begin(), f.end() );
   for ( size_t i = 0; i < nf; ++i ) {
      out[ i ] = full_out[ i + ( ng / 2 ) ];
   }

   return out;
}

bool US_Band_Broaden::build_kernel(
                                   const double & tau
                                   ,const enum kernel_type & type
                                   ,const double & time_start
                                   ,const double & time_end
                                   ,const double & time_delta
                                   ) {

   TSO <<
      QString( "US_Band_Broaden::build_kernel( %1, ... )\n" )
      .arg( tau )
      ;

   if ( time_delta <= 0 ) {
      errormsg = QString( "US_Band_Broaden::build_kernel() non positive time_delta (%1)!" ).arg( time_delta );
      qDebug() << errormsg;
      return false;
   }

   vector < double > time;
   for ( double t = time_start; t <= time_end; t += time_delta ) {
      time.push_back(t);
   }
   
   // Compute the K1 vector as exp(-time/2)
   K1[ tau ].clear();

   switch ( type ) {

   case BAND_BROADEN_KERNEL_EXPONENTIAL :
      for ( const auto& t : time ) {
         K1[ tau ].push_back( exp( -t / tau ) );
      }
      break;

   case BAND_BROADEN_KERNEL_GAUSSIAN :
      for ( const auto& t : time ) {
         K1[ tau ].push_back( exp( -( t * t ) / ( tau * tau ) ) );
      }
      break;
 
   default :
      errormsg = "US_Band_Broaden::build_kernel() unknown kernel_type";
      qDebug() << errormsg;
      return false;

   }

   double K1_sum = accumulate( K1[ tau ].begin(), K1[ tau ].end(), 0.0 );
   if ( K1_sum == 0 ) {
      K1[ tau ].clear();
      errormsg = "US_Band_Broaden::build_kernel() kernel sum is zero!";
      qDebug() << errormsg;
      return false;
   }

   K1_sum_inverse[ tau ] = 1e0 / K1_sum;

   // normalize 
   for ( auto & value : K1[ tau ] ) {
      value *= K1_sum_inverse[ tau ];
   }
   
   double K1_sum_after_norm = accumulate( K1[ tau ].begin(), K1[ tau ].end(), 0.0 );
   TSO << QString( "US_Band_Broaden::build_kernel() K1 sum after norm %1\n" ).arg( K1_sum_after_norm );

   return true;
}

vector < double > US_Band_Broaden::broaden(
                                           const vector < double > & f
                                           ,const double           & tau
                                           ,const enum kernel_type & ktype
                                           ,const double           & time_start
                                           ,const double           & time_end
                                           ,const double           & time_delta
                                           ) {
   TSO <<
      QString( "US_Band_Broaden::broaden( vector size(%1), %2 )\n" )
      .arg( f.size() )
      .arg( tau )
      ;

   if ( !K1.count( tau ) ) {
      if ( !build_kernel( tau, ktype, time_start, time_end, time_delta ) ) {
         return {};
      }
   }

   vector < double > result = convolve( f, K1[ tau ] );

   return result;
}

US_Band_Broaden::US_Band_Broaden() {
   // constructor
   TSO << "US_Band_Broaden::US_Band_Broaden()\n";

   clear();
}

void US_Band_Broaden::clear() {
   TSO << "US_Band_Broaden::clear()\n";

   K1            .clear();
   K1_sum_inverse.clear();
}   

QString US_Band_Broaden::kernel_type_name( const enum kernel_type & ktype ) {
   switch ( ktype ) {

   case BAND_BROADEN_KERNEL_EXPONENTIAL :
      return "Exponential";

   case BAND_BROADEN_KERNEL_GAUSSIAN :
      return "Gaussian";
 
   default :
      return "Unknown";
   }
}

#if defined( BROADEN_TESTING ) 

#include "/Users/eb/sda1/mattia/band_broadening_issue_202410/code/data.h"
#include "../include/us_file_util.h"

void US_Band_Broaden::test( double tau, double kernel_time_end, double kernel_time_delta, kernel_type ktype ) {

   TSO <<
      QString( "US_Band_Broaden::test( %1, %2, %3 )\n" )
      .arg( tau )
      .arg( kernel_time_end )
      .arg( kernel_time_delta )
      ;

   build_kernel( tau
                 ,ktype
                 ,0
                 ,kernel_time_end 
                 ,kernel_time_delta
                 );

   vector < double > UV_broad = broaden( UV_ct, tau );

   if ( UV_broad.size() != UV_ct.size() ) {
      TSO << "US_Band_Broaden::test() UV BROADEND data size mismatch!\n";
      exit( -1 );
   }

   QString fname =
      QString( "/Users/eb/sda1/mattia/band_broadening_issue_202410/code/broad_test_tau%1_kte_%2_ktdelta_%3_kernel_%4.txt" )
      .arg( tau )
      .arg( kernel_time_end )
      .arg( kernel_time_delta )
      .arg( kernel_type_name( ktype ) )
      ;
      
   QString contents;

   for ( size_t i = 0; i < UV_broad.size(); ++i ) {
      contents += QString( "%1\t%2\n" ).arg( UV_t[ i ] ).arg( UV_broad[ i ] );
   }

   if ( !US_File_Util::putcontents( fname, contents, errormsg ) ) {
      TSO << errormsg;
      exit( -1 );
   }

   TSO << "Created:\n" << fname << "\n";
}

void US_Band_Broaden::test() {
   TSO << "US_Band_Broaden::test()\n";
   
   if ( UV_t.size() != UV_ct.size() ) {
      TSO << "US_Band_Broaden::test() UV ORIGINAL data size mismatch!\n";
      exit( -1 );
   }

   vector < double > taus = { 1, 2, 4, 8 };
   vector < double > kernel_end_times = { 50.0, 100.0, 200.0, 400.0};
   vector < double > kernel_delta_times = { 0.2, 0.1, 0.05, 0.025 };
   vector < kernel_type > ktypes = { US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL, US_Band_Broaden::BAND_BROADEN_KERNEL_GAUSSIAN };
   for ( auto & ktype : ktypes ) {
      for ( auto & tau : taus ) {
         for ( auto & kde : kernel_end_times ) {
            for ( auto & kdt : kernel_delta_times ) {
               test( tau, kde, kdt, ktype );
            }
         }
      }
   }
}

#endif
