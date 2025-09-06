#include "../include/us_band_broaden.h"

#include <QDebug>
#include <QTextStream>
#include <QString>
#include "../include/us_vector.h"

#define TSO QTextStream( stdout )

// #define SAVE_KERNELS
#if defined( SAVE_KERNELS )
# include "../include/us_file_util.h"
#endif

vector < double > US_Band_Broaden::convolve(
                                            const vector < double > & f
                                            ,const vector < double > & g
                                            ,const enum kernel_type & ktype
                                            ) {
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

   // Adjust the output to match the size of f & keep peak centered

   const size_t peak_f        = peak( f );
   const size_t peak_full_out = peak( full_out );
   const size_t nfull_out     = full_out.size();

   if ( debug ) {
      TSO << QString(
                     "f size %1, peak %2\n"
                     "g size %3\n"
                     "full conv size %4, peak %5\n"
                     )
         .arg( nf )
         .arg( peak_f )
         .arg( ng )
         .arg( nfull_out )
         .arg( peak_full_out )
         ;
   }

   vector < double > out( nf );
   // old way
   // for ( size_t i = 0; i < nf; ++i ) {
   //    out[ i ] = full_out[ i + ( ng / 2 ) ];
   // }

   size_t offset;
   switch ( ktype ) {
   case BAND_BROADEN_KERNEL_EXPONENTIAL :
   case BAND_BROADEN_KERNEL_HALF_GAUSSIAN :
      // offset = 0;
      // break;
   case BAND_BROADEN_KERNEL_GAUSSIAN :
   case BAND_BROADEN_KERNEL_EMG_GMG :
      // redo out
      {
         offset = peak_full_out - peak_f;
         if ( offset + nf >= nfull_out ) {
            qDebug() << QString(
                                "Warning: convolution center lost, insufficient data after peak finding\n"
                                "offset %1 nf %2 nfull_out %3"
                                )
               .arg( offset )
               .arg( nf )
               .arg( nfull_out )
               ;
            offset = nfull_out - nf - 2;
         }
         if ( offset < 0 ) {
            qDebug() << QString( "Error: convolution offset (%1) is less than 0!" ).arg( offset );
            exit(-1);
         }
      }
      break;

   default :
      qDebug() <<"Internal error: unknown kernel_type provided to US_Band_Broaden::convolve()";
      exit(-1);
      break;
   }      
   
   if ( debug ) {
      TSO << QString( "offset %1, ng/2 %2\n" ).arg( offset ).arg( ng / 2 );
   }

   for ( size_t i = 0; i < nf; ++i ) {
      out[ i ] = full_out[ i + offset ];
   }

   return out;
   // return full_out;
}

size_t US_Band_Broaden::peak( const vector < double > & f ) {
   const size_t nf = f.size();
   size_t peak_pos = 0;
   double peak_val = -DBL_MAX;
   for ( size_t i = 0; i < nf; ++i ) {
      if ( peak_val < f[i] ) {
         peak_val = f[i];
         peak_pos = i;
      }
   }

   return peak_pos;
}

bool US_Band_Broaden::build_kernel(
                                   const vector < double > & params
                                   ,const enum kernel_type & ktype
                                   ,const double           & time_start
                                   ,const double           & time_end
                                   ,const double           & time_delta
                                   ) {

   if ( !params.size() ) {
      errormsg = "US_Band_Broaden::broaden_kernel() empty params";
      qDebug() << errormsg;
      return false;
   }      

   // need to kmode this (or have new kernel type for this)
   double tau = params[ 0 ];

   if ( debug ) {
      TSO <<
         QString( "US_Band_Broaden::build_kernel( %1, %2 )\n" )
         .arg( tau )
         .arg( kernel_type_name( ktype ) )
         ;
   }

   if ( time_delta <= 0 ) {
      errormsg = QString( "US_Band_Broaden::build_kernel() non positive time_delta (%1)!" ).arg( time_delta );
      qDebug() << errormsg;
      return false;
   }

   vector < double > time;
   
   // Compute the K1 kernel
   K1[ tau ].clear();

   switch ( ktype ) {

   case BAND_BROADEN_KERNEL_EXPONENTIAL :
      {
         for ( double t = time_start; t <= time_end; t += time_delta ) {
            time.push_back(t);
         }
         double abs_tau = abs( tau );
         for ( const auto& t : time ) {
            K1[ tau ].push_back( exp( -t / abs_tau ) );
         }
         if ( tau < 0 ) {
            reverse( K1[ tau ].begin(), K1[ tau ].end() );
         }
      }
      break;

   case BAND_BROADEN_KERNEL_HALF_GAUSSIAN :
      {
         for ( double t = time_start; t <= time_end; t += time_delta ) {
            time.push_back(t);
         }
         double abs_tau = abs( tau );
         for ( const auto& t : time ) {
            K1[ tau ].push_back( exp( -( t * t ) / abs_tau ) );
         }
         if ( tau < 0 ) {
            reverse( K1[ tau ].begin(), K1[ tau ].end() );
         }
      }
      break;

   case BAND_BROADEN_KERNEL_GAUSSIAN :
      // center gaussian kernel
      {
         double midtime = ( time_end - time_start ) / 2;
         for ( double t = time_start; t <= time_end; t += time_delta ) {
            time.push_back(t - midtime );
         }
         for ( const auto& t : time ) {
            K1[ tau ].push_back( exp( -( t * t ) / ( tau * tau ) ) );
         }
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
   
   // US_Vector::printvector2( QString( "%1 kernel t, v" ).arg( kernel_type_name( ktype ) ), time, K1[ tau ] );

#if defined( SAVE_KERNELS )
   {
      QStringList params_qsl;
      for ( auto const & p : params ) {
         params_qsl << QString( "%1" ).arg( p );
      }
      QString fname =
         QString( "/Users/eb/sda1/mattia/band_broadening_issue_202410/code/kernel_p%1_%2_%3_%4.dat" )
         .arg( params_qsl.join( "_p" ) )
         .arg( time_end )
         .arg( time_delta )
         .arg( kernel_type_name( ktype ) )
         ;
      
      QString contents;
      contents =
         "US-SOMO: Frame data: band broaden kernel\n"
         "t                 	I(t)\n"
         ;
      
      for ( size_t i = 0; i < K1[ tau ].size(); ++i ) {
         contents += QString( "%1\t%2\n" ).arg( i ).arg( K1[ tau ][ i ] );
      }

      if ( !US_File_Util::putcontents( fname, contents, errormsg ) ) {
         TSO << errormsg;
         exit( -1 );
      }
      TSO << "Created:\n" << fname << "\n";
   }
#endif   


   double K1_sum_after_norm = accumulate( K1[ tau ].begin(), K1[ tau ].end(), 0.0 );
   if ( debug ) {
      TSO << QString( "US_Band_Broaden::build_kernel() K1 sum after norm %1\n" ).arg( K1_sum_after_norm );
   }

   return true;
}

vector < double > US_Band_Broaden::broaden(
                                           const vector < double >  & f
                                           ,const vector < double > & kernel_params
                                           ,const double            & time_start
                                           ,const double            & time_end
                                           ,const double            & time_delta
                                           ,const enum kernel_mode  & kmode
                                           ) {
   switch( kmode ) {
   case BAND_BROADEN_KERNEL_MODE_DEFAULT :
      {
         if ( kernel_params.size() != 2 ) {
            errormsg = "US_Band_Broaden::broaden() default mode, parameters not exactly 2";
            qDebug() << errormsg;
            return {};
         }

         // note order reversed from historical way of first exponential then gaussian
         vector < double > result = broaden( f, { kernel_params[ 0 ] }, BAND_BROADEN_KERNEL_GAUSSIAN, time_start, time_end, time_delta );
         if ( !result.size() ) {
            return result;
         }
         return broaden( result, { kernel_params[ 1 ] }, BAND_BROADEN_KERNEL_EXPONENTIAL, time_start, time_end, time_delta );
         // return broaden( f, kernel_params[ 0 ], kernel_params[ 1 ], time_start, time_end, time_delta );
      }
      
   case BAND_BROADEN_KERNEL_MODE_HALF_GAUSSIAN :
      {
         if ( kernel_params.size() != 3 ) {
            errormsg = "US_Band_Broaden::broaden() half gaussian mode, parameters not exactly 3";
            qDebug() << errormsg;
            return {};
         }

         vector < double > result = broaden( f, { kernel_params[ 0 ] }, BAND_BROADEN_KERNEL_GAUSSIAN, time_start, time_end, time_delta );
         if ( !result.size() ) {
            return result;
         }
         result = broaden( result, { kernel_params[ 1 ] }, BAND_BROADEN_KERNEL_EXPONENTIAL, time_start, time_end, time_delta );
         if ( !result.size() ) {
            return result;
         }
         return broaden( result, { kernel_params[ 2 ] }, BAND_BROADEN_KERNEL_HALF_GAUSSIAN, time_start, time_end, time_delta );
      }

   case BAND_BROADEN_KERNEL_MODE_EMG_GMG :
      errormsg = "US_Band_Broaden::broaden() EMG+GMG mode not yet supported";
      qDebug() << errormsg;
      return {};

   default :
      errormsg = "US_Band_Broaden::broaden() unknown kernel mode";
      qDebug() << errormsg;
      return {};
   }
}

vector < double > US_Band_Broaden::broaden(
                                           const vector < double >  & f
                                           ,const vector < double > & kernel_params
                                           ,const enum kernel_type  & ktype
                                           ,const double            & time_start
                                           ,const double            & time_end
                                           ,const double            & time_delta
                                           ,const enum kernel_mode  & /* kmode */
                                           ) {
   if ( !kernel_params.size() ) {
      errormsg = "US_Band_Broaden::broaden() empty kernel_params";
      qDebug() << errormsg;
      return {};
   }      

   // need to kmode this

   double tau = kernel_params[ 0 ];
   
   if ( debug ) {
      TSO <<
         QString( "US_Band_Broaden::broaden( vector size(%1), %2, %3, %4, %5, %6 )\n" )
         .arg( f.size() )
         .arg( tau )
         .arg( kernel_type_name( ktype ) )
         .arg( time_start )
         .arg( time_end )
         .arg( time_delta )
         ;
   }

   if ( tau == 0 ) {
      return f;
   }
 
   // #warning not so sure we want to cache at this point, disabled until production & will need to test

   clear();

   if ( !K1.count( tau ) ) {
      if ( !build_kernel( kernel_params, ktype, time_start, time_end, time_delta ) ) {
         return {};
      }
   }

   vector < double > result = convolve( f, K1[ tau ], ktype );

   return result;
}

US_Band_Broaden::US_Band_Broaden( bool debug ) {
   // constructor
   this->debug = debug;

   if ( debug ) {
      TSO << "US_Band_Broaden::US_Band_Broaden()\n";
   }

   clear();
}

void US_Band_Broaden::clear() {
   if ( debug ) {
      TSO << "US_Band_Broaden::clear()\n";
   }

   K1            .clear();
   K1_sum_inverse.clear();
}   

QString US_Band_Broaden::kernel_type_name( const enum kernel_type & ktype ) {
   switch ( ktype ) {

   case BAND_BROADEN_KERNEL_EXPONENTIAL :
      return "Exponential";

   case BAND_BROADEN_KERNEL_HALF_GAUSSIAN :
      return "Half_Gaussian";

   case BAND_BROADEN_KERNEL_GAUSSIAN :
      return "Gaussian";
 
   default :
      return "Unknown";
   }
}

QString US_Band_Broaden::kernel_mode_name( const enum kernel_mode & kmode ) {
   switch ( kmode ) {

   case BAND_BROADEN_KERNEL_MODE_DEFAULT :
      return "Default";

   case BAND_BROADEN_KERNEL_MODE_HALF_GAUSSIAN :
      return "half Gaussian";

   case BAND_BROADEN_KERNEL_MODE_EMG_GMG :
      return "EMG+GMG";
 
   default :
      return "Unknown";
   }
}

#if defined( BROADEN_TESTING ) 

#include "/Users/eb/sda1/mattia/band_broadening_issue_202410/code/data.h"
#include "../include/us_file_util.h"

void US_Band_Broaden::test( const vector < double > & kernel_params, double kernel_time_end, double kernel_time_delta, kernel_type ktype ) {

   debug = true;

   if ( !kernel_params.size() ) {
      errormsg = "US_Band_Broaden::test() no kernel params";
      qDebug() << errormsg;
      exit(-1);
   }

   const double tau = kernel_params[ 0 ];

   if ( debug ) {
      TSO <<
         QString( "US_Band_Broaden::test( %1, %2, %3 )\n" )
         .arg( tau )
         .arg( kernel_time_end )
         .arg( kernel_time_delta )
         ;
   }
   
   // should be automatically called by "broaden()"
   // build_kernel( { tau }
   //               ,ktype
   //               ,0
   //               ,kernel_time_end 
   //               ,kernel_time_delta
   //               );

   vector < double > UV_broad = broaden( UV_ct, { tau }, ktype, 0, kernel_time_end, kernel_time_delta );

   if ( !UV_broad.size() ) {
      TSO << errormsg;
      exit( -1 );
   }

   if ( UV_broad.size() != UV_ct.size() ) {
      TSO << "US_Band_Broaden::test() UV BROADEND data size mismatch!\n";
      exit( -1 );
   }

   if ( 1 ) {
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
      if ( debug ) {
         TSO << "Created:\n" << fname << "\n";
      }
   }

   if ( 1 ) {
      QString fname =
         QString( "/Users/eb/sda1/mattia/band_broadening_issue_202410/code/broad_test_tau%1_kte_%2_ktdelta_%3_kernel_%4.dat" )
         .arg( tau )
         .arg( kernel_time_end )
         .arg( kernel_time_delta )
         .arg( kernel_type_name( ktype ) )
         ;
      
      QString contents;
      contents =
         "US-SOMO: Frame data: band broaden testing\n"
         "t                 	I(t)\n"
         ;
      
      for ( size_t i = 0; i < UV_broad.size(); ++i ) {
         contents += QString( "%1\t%2\n" ).arg( UV_t[ i ] ).arg( UV_broad[ i ] );
      }

      if ( !US_File_Util::putcontents( fname, contents, errormsg ) ) {
         TSO << errormsg;
         exit( -1 );
      }
      TSO << "Created:\n" << fname << "\n";
   }
}

void US_Band_Broaden::test() {
   TSO << "US_Band_Broaden::test()\n";
   
   if ( UV_t.size() != UV_ct.size() ) {
      TSO << "US_Band_Broaden::test() UV ORIGINAL data size mismatch!\n";
      exit( -1 );
   }

   // testing build kernel exponential with negative tau
   vector < double > lambda_1 = { -2, 0, 2 };
   vector < double > lambda_2 = { -2, 0, 2 };
   
   vector < double > taus = { -2, 2 };
   vector < double > kernel_end_times = { 100.0 };
   vector < double > kernel_delta_times = { 0.1 };
   vector < kernel_type > ktypes = { US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL, US_Band_Broaden::BAND_BROADEN_KERNEL_HALF_GAUSSIAN };
   for ( auto & ktype : ktypes ) {
      for ( auto & tau : taus ) {
         for ( auto & kde : kernel_end_times ) {
            for ( auto & kdt : kernel_delta_times ) {
               test( { tau }, kde, kdt, ktype );
            }
         }
      }
   }

   // testing for DEFAULT kernel types

   /*

   vector < double > taus = { 2 };
   // vector < double > kernel_end_times = { 50.0, 100.0, 200.0, 400.0 };
   vector < double > kernel_end_times = { 100.0 };
   vector < double > kernel_delta_times = { 0.1 };
   vector < kernel_type > ktypes = { US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL, US_Band_Broaden::BAND_BROADEN_KERNEL_GAUSSIAN };
   // vector < kernel_type > ktypes = { US_Band_Broaden::BAND_BROADEN_KERNEL_EXPONENTIAL };
   for ( auto & ktype : ktypes ) {
      for ( auto & tau : taus ) {
         for ( auto & kde : kernel_end_times ) {
            for ( auto & kdt : kernel_delta_times ) {
               test( { tau }, kde, kdt, ktype );
            }
         }
      }
   }

   */
}

#endif
