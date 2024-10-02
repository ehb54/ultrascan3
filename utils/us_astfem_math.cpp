//! \file us_astfem_math.cpp
#include "us_astfem_math.h"
#include "us_math2.h"
#include "us_hardware.h"
#include "us_settings.h"
#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif
#define DSS_LO_ACC 250.0  // default SetSpeedLowAccel

#if 0
#ifdef NO_DB
#include <mpi.h>
#endif
#endif

//----------------------------------------------------------------------
// Write time state
//----------------------------------------------------------------------
int US_AstfemMath::writetimestate( const QString&           tmst_fpath,
                                   US_SimulationParameters& simparams,
                                   US_DataIO::RawData&      sim_data )
{
   int dbg_level        = US_Settings::us_debug();
   US_TimeState timestate;
   int nspeed           = simparams.speed_step.size();
//DbgLv(1) << "AMATH:wrts::writetimestate : tmst_fpath=" << tmst_fpath;

   if ( timestate.open_write_data( tmst_fpath, 1.0, 0.0 ) != 0 )
   {
DbgLv(1) << "AMATH: wrts: Unable to open" << tmst_fpath;
      return 0;
   }

DbgLv(1)<< "AMATH:wrts: number of speeds=" << nspeed
 << "Time state file path=" << tmst_fpath;
simparams.debug();
   timestate.set_key( "Time",        "I4" );
   timestate.set_key( "RawSpeed",    "F4" );
   timestate.set_key( "SetSpeed",    "I4" );
   timestate.set_key( "Omega2T",     "F4" );
   timestate.set_key( "Temperature", "F4" );
   timestate.set_key( "Step",        "I2" );
   timestate.set_key( "Scan",        "I2" );

   double duration      = 0.0;
   double duration_prev = 0.0;
   double rpm           = 0.0;
   double omega2t       = 0.0;
   double prvs_speed    = 0.0;
   int    scan_nbr      = 0;
   int    step_nbr      = 0;
   double temperature   = sim_data.scanData[ 0 ].temperature;
   int nscans           = sim_data.scanData.size(); // Used for number of scans
   int t_acc            = 1; // Used for time when accelerated up to the specified rotor speed
   double rate          = 1.0;
   double speed         = 1000.0;
   int ss_reso          = 100;
   // If debug_text so directs, change set_speed_resolution
   QStringList dbgtxt = US_Settings::debug_text();
   for ( int ii = 0; ii < dbgtxt.count(); ii++ )
   {  // If debug text modifies ss_reso, apply it
      if ( dbgtxt[ ii ].startsWith( "SetSpeedReso" ) )
         ss_reso        = QString( dbgtxt[ ii ] ).section( "=", 1, 1 ).toInt();
   }
   US_SimulationParameters::SpeedProfile* sp;
   US_SimulationParameters::SpeedProfile* sp_prev;
DbgLv(1) << "AMATH:wrts: writetimestate : no of scans" << nscans;
   QList< int > scantimes;
   QList< double > scantemps;

   for ( int ii = 0; ii < nscans; ii++ )
   {  // Accumulate the times and temperatures at scans
      scantimes << sim_data.scanData[ ii ].seconds;
      scantemps << sim_data.scanData[ ii ].temperature;
DbgLv(1) << "AMATH:wrts:scantimes" << scantimes[ii] << sim_data.scanData[ii].omega2t
 << ii << sim_data.scanData[ii].rpm;
   }
//DbgLv(1) << " writetimestate : no of scans" << nscans;

   if ( simparams.sim == false )
   {  // Handle 1st acceleration zone for real data (compute acceleration rate)
      // If debug_text so directs, change set_speed_low_accel
      QString dbgval   = US_Settings::debug_value( "SetSpeedLowAcc" );
      double low_accel = dbgval.isEmpty() ? DSS_LO_ACC : dbgval.toDouble();

      // Get the 1st acceleration zone's rate and end-time
      low_acceleration( simparams.speed_step, low_accel, rate );
      t_acc       = (int)qRound( (double)( simparams.speed_step[ 0 ].rotorspeed )
                                           / rate );
DbgLv(1)<< "AMATH:wrts: computed rate:" << rate;
   }
   else
   {  // Handle 1st acceleration zone for simulation (astfem_sim) data
      sp         = &simparams.speed_step[ 0 ];
      // Use specified acceleration rate
      t_acc      = (int)qRound( (double)sp->rotorspeed
                              / (double)sp->acceleration );
      rate       = (double)sp->rotorspeed / (double)t_acc;
DbgLv(1)<< "AMATH:wrts: rate is given by user : t_acc from timestate" << t_acc << rate;
   }

   int d1     = 0;
   int itime  = 0;

   for ( int step = 0; step < nspeed; step++ )
   {
      step_nbr   = step + 1;
      sp         = &simparams.speed_step[ step ];
      speed      = (double)sp->rotorspeed;

      if ( step > 0 )
      {
         sp_prev      = &simparams.speed_step[ step - 1 ];
         prvs_speed   = (double)sp_prev->rotorspeed;
      }
      else
         prvs_speed   = 0.0;

      if ( step == 0 )
      {
         d1           = 0;
         rpm         -= rate;
      }
      else
      {
//         duration_prev += ( simparams.speed_step[ step-1 ].duration_hours * 3600.0 )
//                        + ( simparams.speed_step[ step-1 ].duration_minutes * 60.0 );
         duration_prev = duration;

         d1            = (int)duration_prev + 1;

         if ( simparams.sim == false )
            // Use calculated rate when user doesn't know it
            t_acc       = ( int)qRound( double( qAbs( speed - prvs_speed ) )
                                        / rate );
         else
            // When user knows acceleration rate
            t_acc       = ( int)qRound( double( qAbs( speed - prvs_speed  ) )
                                       / sp->acceleration );

         rate        = (double)( speed - prvs_speed ) / (double)t_acc;
      }

      duration   += ( sp->duration_hours * 3600.0 )
                  + ( sp->duration_minutes * 60.0 );
//DbgLv(1) << "duration from timestate = "<<  duration;
      int tacc    = d1 + t_acc;
      int d2      = (int)duration + 1;

      for ( int ii = d1; ii < d2; ii++ )
      {

         if ( ii < tacc )
            rpm        += rate;
         else
            rpm         = speed;

         int set_speed  = (int)qRound( rpm / (double)ss_reso ) * ss_reso;

         double om1t = rpm * M_PI / 30.0;
         omega2t    += sq( om1t );

         itime       = ii;
         int scanx   = scantimes.indexOf( itime );
         scan_nbr    = ( scanx < 0 ) ? 0 : ( scanx + 1 );
         temperature = ( scanx < 0 ) ? temperature : scantemps[ scanx ];
//if(scan_nbr>0)
if((scan_nbr>0)||(ii<(d1+2))||((ii>(tacc-2))&&(ii<(tacc+2)))||((ii+3)>d2))
 DbgLv(1) << "AMATH:wrts: ii(time)" << ii << "scanx scan_nbr" << scanx << scan_nbr
  << "rpm" << rpm << "omega2t" << omega2t;

         timestate.set_value( "Time",        itime       );
         timestate.set_value( "RawSpeed",    rpm         );
         timestate.set_value( "SetSpeed",    set_speed   );
         timestate.set_value( "Omega2T",     omega2t     );
         timestate.set_value( "Temperature", temperature );
         timestate.set_value( "Step",        step_nbr    );
         timestate.set_value( "Scan",        scan_nbr    );
#ifndef NO_DB
//DbgLv(1) << "Timestate:" << "time=" << itime
// << "w2t=" << omega2t << "rpm=" << rpm << " t_acc=" << t_acc
// << "scan_nbr=" << scan_nbr << "step=" << step;
#endif
         timestate.flush_record();
      }
   }

   // Pad out TimeState for some seconds beyond last scan time
   duration      += 10;
   d1             = itime + 1;
   int step       = nspeed - 1;
   int set_speed  = (int)qRound( rpm / (double)ss_reso ) * ss_reso;
   scan_nbr       = 0;
   double omg2ti  = pow( ( rpm * M_PI / 30.0 ), 2.0 );

   for ( int ii = d1; ii <= int(duration); ii++ )
   {
      omega2t       += omg2ti;
      itime          = ii;

      timestate.set_value( "Time",        itime       );
      timestate.set_value( "RawSpeed",    rpm         );
      timestate.set_value( "SetSpeed",    set_speed   );
      timestate.set_value( "Omega2T",     omega2t     );
      timestate.set_value( "Temperature", temperature );
      timestate.set_value( "Step",        step        );
      timestate.set_value( "Scan",        scan_nbr    );
#ifndef NO_DB
//DbgLv(1) <<"Timestate:" << "time= " << itime
// << "w2t= " << omega2t << "rpm= " << rpm << "  t_acc=" << t_acc
// << "scan_nbr= " << scan_nbr << "step= " << step;
#endif

      timestate.flush_record();
   }

   timestate.close_write_data();
   timestate.write_defs( 1.0 );

   return timestate.time_count();
}

// Determine if first time step's acceleration is too low
bool US_AstfemMath::low_acceleration(
      const QVector< US_SimulationParameters::SpeedProfile >& speedsteps,
      const double min_accel, double& rate )
{
   int dbg_level     = US_Settings::us_debug();
   const double tfac = ( 4.0 / 3.0 );
   double t2         = speedsteps[ 0 ].time_first;
   double w2t        = speedsteps[ 0 ].w2t_first;
   double om1t       = speedsteps[ 0 ].rotorspeed * M_PI / 30.0;
   double w2         = sq( om1t ); 
   double t1w        = w2t / w2;

   // =====================================================================
   // For the first speed step, we compute "t1", the end of the initial
   // acceleration zone, using
   //   "t2"   , the time in seconds for the first scan;
   //   "w2t"  , the omega^2_t integral for the first scan time;
   //   "w2"   , the omega^2 value for the constant zone speed;
   //   "tfac" , a factor (==(4/3)==1.333333) derived from the following.
   // The acceleration zone begins at t0=0.0.
   // It ends at time "t1".
   // The time between t1 and t2 is at constant speed.
   // The time between 0 and t1 is at changing speeds averaging (rpm/2).
   // For I1 and I2, the omega^2t integrals at t1 and t2,
   //        ( I2 - I1 ) = ( t2 - t1 ) * w2       (equ.1)
   //        I1 = ( (rpm/2) * PI / 30 )^2 * t1    (equ.2)
   //        I1 = ( ( rpm * PI / 30 )^2 / 4 ) * t1
   //        w2 = ( rpm * PI / 30 )^2
   //        I1 = ( w2 / 4 ) * t1
   //        I2 = w2t
   // Substituting into equ.1, we get:
   //        ( w2t - ( ( w2 / 4 ) * t1 ) ) = ( t2 - t1 ) * w2
   //        t1 * ( w2 - ( w2 / 4 ) )      = t2 * w2 - w2t
   //        t1 * ( 3 / 4 ) * w2           = t2 * w2 - w2t
   //        t1  = ( 4 / 3 ) * ( t2 - ( w2t / w2 ) )
   // =====================================================================

   double t1      = tfac * ( t2 - t1w );
DbgLv(1) << "AMATH:loac: om1t w2 w2t" << om1t << w2 << w2t
 << "t1w" << t1w << "tfac" << tfac;

   if ( t1 >= t2 )
   {  // Something wrong!  set t1 to a few seconds before t2
      t1             = t2 - 5.0;
   }

   int t_acc      = (int)qRound( t1 );
   rate           = (double)( speedsteps[ 0 ].rotorspeed ) / (double)t_acc;
DbgLv(1) << "AMATH:loac:t1 t2" << t1 << t2 << "t_acc rate" << t_acc << rate;

   return ( rate < min_accel );
}

// Determine if a timestate file holds one-second-interval records
bool US_AstfemMath::timestate_onesec( const QString& tmst_fpath,
                                      US_DataIO::RawData&      sim_data )
{
   bool onesec_intv = false;
   bool constti     = false;
   double timeinc   = 0.0;
   US_TimeState timestate;

   if ( timestate.open_read_data( tmst_fpath, false ) != 0 )
      return onesec_intv;

   int ntimes       = timestate.time_count();
   timestate.time_range( &constti, &timeinc, NULL );
   timestate.close_read_data();

   if ( constti  &&  timeinc == 1.0 )
   {  // If constant increment and it is 1.0, one-second-interval
      onesec_intv      = true;
   }

   else
   {  // Otherwise, compare timestate times count to last scan time
      int nscan        = sim_data.scanCount();
      int lsctime      = (int)sim_data.scanData[ nscan - 1 ].seconds;
      // If number of times is greater than or equal to the last scan time,
      //  then one-second-interval
      if ( ntimes >= lsctime )
         onesec_intv      = true;
   }

   return onesec_intv;
}

// Interpolate the C1 array from the C0 grid
void US_AstfemMath::interpolate_C0( MfemInitial& C0, double* C1,
      QVector< double >& xvec )
{
   int ja = 0;
   double* x    = xvec            .data();
   double* radi = C0.radius       .data();
   double* conc = C0.concentration.data();
   int     rsiz = C0.radius       .size();

   for ( int j = 0; j < xvec.size(); j++ )
   {
      int    i;
      double xs = x[ j ];
      double xc = xs + 1.e-12;

      for ( i = ja; i < rsiz; i++ )
         if ( radi[ i ] > xc ) break;


      if ( i == 0 )                  // x[j] < C0.radius[0]
         C1[ j ] = conc[ 0 ];        // Use the first value

      else if ( i == rsiz )          // x[j] > last point in C0.radius[]
         C1[ j ] = conc[ i - 1 ];

      else
      {
         double a = radi[ i - 1 ];
         double b = radi[ i ];

         double tmp = ( xs - a ) / ( b - a );

         ja      = i - 1;

         C1[ j ] = conc[ ja ] * ( 1.0 - tmp ) +
                   conc[ i  ] * tmp;

      }
   }
}

// Original grid: C0, final grid: C1
void US_AstfemMath::interpolate_C0( MfemInitial& C0, MfemInitial& C1 )
{
   int ja = 0;
   double* C0radi = C0.radius       .data();
   double* C1radi = C1.radius       .data();
   double* C0conc = C0.concentration.data();
   double* C1conc = C1.concentration.data();
   int     r0size = C0.radius       .size();
   int     r1size = C1.radius       .size();

   for ( int j = 0; j < r1size; j++ )
   {
      int   i;
      double xs = C1radi[ j ];
      double xc = xs + 1.e-12;

      for ( i = ja; i < r0size; i++ )
         if ( C0.radius[ i ] > xc )  break;


      if ( i == 0 )                     // x[j] < C0.radius[0]
         C1conc[ j ] = C0conc[ 0 ];     // Use the first value

      else if ( i == r0size )           // x[j] > last point in C0.radius[]
         C1conc[ j ] = C0conc[ i - 1 ];

      else
      {
         ja = i - 1;

         double a   = C0radi[ ja ];
         double b   = C0radi[ i  ];

         double tmp = ( xs - a ) / ( b - a );

         C1conc[ j ] = C0conc[ ja ] * ( 1.0 - tmp ) + C0conc[ i ] * tmp;
      }
   }
}

void US_AstfemMath::zero_2d( int val1, int val2, double** matrix )
{
   for ( int i = 0; i < val1; i++ )
      for ( int j = 0; j < val2; j++ )
         matrix[ i ][ j ] = 0.0;
}

void US_AstfemMath::initialize_2d( int val1, int val2, double*** matrix )
{
   *matrix = new double* [ val1 ];

   for ( int i = 0; i < val1; i++ )
   {
      (*matrix)[ i ] = new double [ val2 ];

      for ( int j = 0; j < val2; j++ )
         (*matrix)[ i ][ j ] = 0.0;

   }
}

void US_AstfemMath::clear_2d( int val1, double** matrix )
{
   for ( int i = 0; i < val1; i++ ) delete [] matrix[i];

   delete [] matrix;
}

double US_AstfemMath::minval( const QVector< double >& value )
{
   const double* avalue = value.data();
   double minimum = 1.0e300;

   for ( int i = 0; i < value.size(); i++ )
      minimum = qMin( minimum, avalue[ i ] );

   return minimum;
}

double US_AstfemMath::minval( const QVector< US_Model::SimulationComponent >& value )
{
   double minimum = 1.0e300;

   for ( int i = 0; i < value.size(); i++ )
      minimum = min( minimum, value[ i ].s );

   return minimum;
}

double US_AstfemMath::maxval( const QVector< double >& value )
{
   const double* avalue = value.data();
   double maximum = -1.0e300;

   for ( int i = 0; i < value.size(); i++ )
      maximum = qMax( maximum, avalue[ i ]);

   return maximum;
}

double US_AstfemMath::maxval( const QVector< US_Model::SimulationComponent >& value )
{
   double maximum = -1.0e300;

   for ( int i = 0; i < value.size(); i++ )
      maximum = max( maximum, value[ i ].s );

   return maximum;
}

void US_AstfemMath::initialize_3d(
      int val1, int val2, int val3, double**** matrix )
{
   *matrix = new double** [ val1 ];

   for ( int i = 0; i < val1; i++ )
   {
      (*matrix)[ i ] = new double *[ val2 ];

      for ( int j = 0; j < val2; j++ )
      {
         (*matrix)[ i ][ j ] = new double [ val3 ];

         for ( int k = 0; k < val3; k++ )
         {
            (*matrix)[ i ][ j ][ k ] = 0.0;
         }
      }
   }
}

void US_AstfemMath::clear_3d( int val1, int val2, double*** matrix )
{
   for ( int i = 0; i < val1; i++ )
   {
      for ( int j = 0; j < val2; j++ )
      {
         delete [] matrix[ i ][ j ];
      }

      delete [] matrix[ i ];
   }

   delete [] matrix;
}

void US_AstfemMath::tridiag( double* a, double* b, double* c,
                             double* r, double* u, int N )
{
   double bet = b[ 0 ];
#ifdef NO_DB
   // If used within mpi_analysis, avoid reallocating gamvec at each
   //  call. Just reallocate if N has grown larger.
   static QVector< double > gamvec;
   static int Nsave = 0;

   if ( N > Nsave )
   {
      Nsave = ( N * 3 ) / 2;
      gamvec.resize( Nsave );
   }
#else
   // If used by a desktop application, simply reallocate gamvec
   //  at each call.
   QVector< double > gamvec( N );
#endif
   double* gam = gamvec.data();

   if ( bet == 0.0 )  { qDebug() << "Error 1 in tridiag"; return; }

   u[ 0 ] = r[ 0 ] / bet;

   for ( int jj = 1; jj < N; jj++ )
   {
      gam[ jj ] = c[ jj - 1 ] / bet;
      bet       = b[ jj ] - a[ jj ] * gam[ jj ];

      if ( bet == 0.0 )  { qDebug() << "Error 2 in tridiag"; return; }

      u[ jj ]   = ( r[ jj ] - a[ jj ] * u[ jj - 1 ] ) / bet;
   }

   gam[ 0 ] = gam[ 1 ];

   for ( int jj = N - 1; jj >= 1; jj-- )
      u[ jj - 1 ]  -= gam[ jj ] * u[ jj ];
}

//////////////////////////////////////////////////////////////////
//
// cube_root: find the positive cube-root of a cubic polynomial
//       p(x)=a0+a1*x+a2*x^2+x^3
//
// with: a0<=0,  and  a1, a2>=0;
//
//////////////////////////////////////////////////////////////////

double US_AstfemMath::cube_root( double a0, double a1, double a2 )
{
   double x;

   long double Q = ( 3.0 * a1 - sq( a2 ) ) / 9.0;
   long double S = ( 9.0 * a1 * a2 - 27.0 * a0 - 2.0 * pow( a2, 3.0 ) ) / 54.0;
   long double D = pow( Q, 3.0 ) + sq( S );

   if ( D < 0 )
   {
      double theta = acos( S / sqrt( pow( -Q, 3.0 ) ) );
      x = 2.0 * sqrt( -Q ) * cos( theta / 3.0);
   }
   else
   {
      long double B;
      long double Dc = sqrt( D );

      if ( S + Dc < 0 )
         B = - pow( - ( S + Dc ), 1.0 / 3.0 ) - pow( Dc - S, 1.0 / 3.0 );

      else if (S - Dc < 0)
         B = pow( S + Dc, 1.0 / 3.0) - pow( Dc - S, 1.0 / 3.0);

      else
         B = pow( S + Dc, 1.0 / 3.0 ) + pow( S - Dc, 1.0 / 3.0 );

      long double Dc2 = -3.0 * (pow(B, 2.0) + 4 * Q);

      if ( Dc2 > 0 )
         x = max( B, 0.5 * ( -B + sqrt( Dc2 ) ) );
      else
         x = B;
   }

   x = x - a2 / 3.0;

   return x;
}

#if 0
double US_AstfemMath::cube_root( double a0, double a1, double a2 )
{
   double x;

   double Q = ( 3.0 * a1 - sq( a2 ) ) / 9.0;
   double S = ( 9.0 * a1 * a2 - 27.0 * a0 - 2.0 * pow( a2, 3.0 ) ) / 54.0;
   double D = pow( Q, 3.0 ) + sq( S );

   if ( D < 0 )
   {
      double theta = acos( S / sqrt( pow( -Q, 3.0 ) ) );
      x = 2.0 * sqrt( -Q ) * cos( theta / 3.0);
   }
   else
   {
      double B;
      double Dc = sqrt( D );

      if ( S + Dc < 0 )
         B = - pow( - ( S + Dc ), 1.0 / 3.0 ) - pow( Dc - S, 1.0 / 3.0 );

      else if (S - Dc < 0)
         B = pow( S + Dc, 1.0 / 3.0) - pow( Dc - S, 1.0 / 3.0);

      else
         B = pow( S + Dc, 1.0 / 3.0 ) + pow( S - Dc, 1.0 / 3.0 );

      double Dc2 = -3.0 * (pow(B, 2.0) + 4 * Q);

      if ( Dc2 > 0 )
         x = max( B, 0.5 * ( -B + sqrt( Dc2 ) ) );
      else
         x = B;
   }

   x = x - a2 / 3.0;

   return x;
}
#endif

//////////////////////////////////////////////////////////////////
//
// find_C1_mono_Nmer:   find C1 from    C1 + K * C1^n = CT
//
//////////////////////////////////////////////////////////////////
double US_AstfemMath::find_C1_mono_Nmer( int n, double K, double CT )
{
   // use Newton's method for f(x) = x + K*x^n - CT
   //    x_{i+1} = x_i - f(x_i)/f'(x_i)

   double    x1;
   double    x0       = 0.0;
   const int MaxNumIt = 1000;
#if 0
   int       i;

   if ( CT <= 0.0     ) return 0.0;
   if ( CT <= 1.0e-12 ) return CT;

   for ( i = 1; i < MaxNumIt; i++ )
   {
      x1 = ( K * ( n - 1.0 ) * pow( x0, (double)n ) + CT ) /
           ( 1.0 + K * n * pow( x0, n - 1.0 ) );

      if ( fabs( x1 - x0 ) / ( 1.0 + fabs( x1 ) ) < 1.e-12 ) break;
      x0 = x1;
   }

   if ( i == MaxNumIt )
   {
      qDebug() << "WARNING: Newton's method did not converge "
                  "in find_C1_mono_Nmer()";
      return -1.0;
   }
#endif
#if 1
   double    zn       = (double)n;
   double    zn1      = zn - 1.0;
   double    zKn      = K * zn;
   double    zKn1     = K * zn1;
   int       ii;

   if ( CT <= 1.0e-12 )
      return qMax( 0.0, CT );

   for ( ii = 1; ii < MaxNumIt; ii++ )
   {
      x1 = ( CT  + zKn1 * pow( x0, zn  ) ) /
           ( 1.0 + zKn  * pow( x0, zn1 ) );

      if ( ( qAbs( x1 - x0 ) / ( 1.0 + qAbs( x1 ) ) ) < 1.e-12 )
         break;

      x0 = x1;
   }

   if ( ii == MaxNumIt )
   {
      qDebug() << "WARNING: Newton's method did not converge "
                  "in find_C1_mono_Nmer()";
      return -1.0;
   }
#endif

   return 0.5 * ( x0 + x1 );
}

/////////////////////////////////////////////////////////////////
//
// Gaussian Elimination for n X n system: Ax=b
//
// return value: -1: A singular, no solution,
//                1: successful
// in return:     A has been altered, and b stores the solution x
//
/////////////////////////////////////////////////////////////////
int US_AstfemMath::GaussElim( int n, double** a, double* b )
{
    // Elimination
    for ( int i = 0; i < n; i++ )
    {
      // find the pivot
      double amax = fabs( a[ i ][ i ] );
      int    Imx  = i;

      for ( int ip = i + 1; ip < n; ip++ )
      {
        if ( fabs( a[ ip ][ i ] ) > amax )
        {
          amax = fabs( a[ ip ][ i ]);
          Imx  = ip;
        }
      }

      if ( amax == 0 )
      {
        qDebug() << "Singular matrix in routine GaussElim";
        return -1;
      }

      double tmp;

      // interchange i-th and Imx-th row
      if ( Imx != i )
      {
        double* ptmp = a[ i ];
        a[ i ]       = a[ Imx ];
        a[ Imx ]     = ptmp;

        tmp          = b[ i ];
        b[ i ]       = b[ Imx ];
        b[ Imx ]     = tmp;
      }

      // Elimination
      tmp = a[ i ][ i ];

      for ( int j = i; j < n; j++ ) a[ i ][ j ] /= tmp;

      b[ i ] /= tmp;

      for ( int ip = i + 1; ip < n; ip++ )
      {
        tmp = a[ ip ][ i ];

        for ( int j = i + 1; j < n; j++ )
           a[ ip ][ j ] -= tmp * a[ i ][ j ];

        b[ ip ] -= tmp * b[ i ];
      }
    }

    // Backward substitution
    for ( int i = n - 2; i >= 0; i-- )
      for ( int j = i + 1; j < n; j++ ) b[ i ] -= a[ i ][ j ] * b[ j ];

    return 1;
}

// Interpolation routine By B. Demeler 041708
int US_AstfemMath::interpolate( MfemData& expdata, MfemData& simdata,
                                bool      use_time )
{
   // NOTE: *expdata has to be initialized to have the proper size
   //        (filled with zeros) before using this routine!
   //        The radius also has to be assigned!

   if ( expdata.scan.size() == 0  ||  expdata.scan[ 0 ].conc.size() == 0  ||
        simdata.scan.size() == 0  ||  simdata.radius.size()         == 0 )
      return -1;

   // Iterate through all experimental data scans and find the first time point
   // in simdata that is higher or equal to each time point in expdata:
   int dbg_level  = US_Settings::us_debug();
   int    fscan   = -1;
   int    lscan   = -1;
   int    escans  = expdata.scan.size();
   int    sscans  = simdata.scan.size();
   double e_omega = 0.0;
   double e_time  = 0.0;

   if ( use_time )
   {
      double s_time  = simdata.scan[          0 ].time;
      double l_time  = simdata.scan[ sscans - 1 ].time;

      for ( int expscan = 0; expscan < escans; expscan++ )
      { // First determine the scan range of current experiment data
         e_time     = expdata.scan[ expscan ].time;
         if ( fscan < 0  &&  e_time >= s_time )
            fscan      = expscan;
         if ( e_time > l_time  )
         {  // Save scan upper limit (use scan count if close)
DbgLv(2) << "MATHi: l_time e_time" << l_time << e_time << "expscan" << expscan;
            lscan      = ( expscan > ( escans - 5 ) ) ? escans : expscan;
            break;
         }
      }
//DbgLv(1) << "MATHi: s_time e_time" << s_time << e_time
// << "fscan lscan" << fscan << lscan <<"expscansize=" <<  escans << "simscansize="<< sscans;
      fscan         = ( fscan < 0 ) ?      0 : fscan;
      lscan         = ( lscan < 0 ) ? escans : lscan;
//DbgLv(1) << "MATHi: s_time e_time" << s_time << e_time
// << "fscan lscan" << fscan << lscan << "escans=" <<  escans << "sscans=" << sscans;
   }
   else // Use omega^2t integral for interpolation
   {
      double s_omega = simdata.scan[          0 ].omega_s_t;
      double l_omega = simdata.scan[ sscans - 1 ].omega_s_t;

      for ( int expscan = 0; expscan < escans; expscan++ )
      { // First determine the scan range of current experiment data
         e_omega    = expdata.scan[ expscan ].omega_s_t;
         if ( fscan < 0  &&  e_omega >= s_omega )
            fscan      = expscan;
         if ( e_omega > l_omega )
         {
            lscan      = ( expscan > ( escans - 5 ) ) ? escans : expscan;
            break;
         }
      }

      fscan         = ( fscan < 0 ) ?      0 : fscan;
      lscan         = ( lscan < 0 ) ? escans : lscan;
//DbgLv(1) << "MATHi: s_omega l_omega" << s_omega << e_omega
// << "fscan lscan" << fscan << lscan;
   }

   if ( fscan < 0  ||  lscan < 0 )
      return -1;

   // Interpolate all radial points from each scan in tmp_data onto expdata
   interpolate( expdata, simdata, use_time, fscan, lscan );

   return 0;
}

// Interpolation routine By B. Demeler 041708
int US_AstfemMath::interpolate( MfemData& expdata, MfemData& simdata,
                                bool      use_time, int fscan, int lscan )
{
   static int rrdk      = 0;
   int dbg_level        = US_Settings::us_debug();
//*TIMING*
//static int ncall=0;
//static int totMs=0;
//QDateTime tStart = QDateTime::currentDateTime();
//*TIMING*
   // NOTE: *expdata has to be initialized to have the proper size
   //        (filled with zeros) before using this routine!
   //        The radius also has to be assigned!
   int    nsscan  = simdata.scan.size();
   int    nsconc  = simdata.radius.size();
   int    nescan  = expdata.scan.size();
   int    neconc  = expdata.scan[ 0 ].conc.size();

   if ( nescan == 0  ||  neconc == 0  ||  nsscan == 0  ||  nsconc == 0 )
      return -1;

   // First, create a temporary MfemData instance (tmp_data) that has the
   // same radial grid as simdata, but the same time grid as the experimental
   // data. The time and w2t integral values are interpolated for the tmp_data
   // structure.

   MfemData tmp_data;
   MfemScan tmp_scan;

   tmp_data.scan  .clear();
   tmp_data.radius.clear();
   tmp_data.scan  .reserve( nescan );
   tmp_data.radius.reserve( nsconc );

   // Fill tmp_data.radius with radius positions from the simdata array:

   for ( int ii = 0; ii < nsconc; ii++ )
   {
      tmp_data.radius << simdata.radius[ ii ];
   }

   // Iterate through all experimental data scans and find the first time point
   // in simdata that is higher or equal to each time point in expdata:

   int    simscan = 0;
   double e_omega = 0.0;
   double e_time  = 0.0;
   double s_omega1;
   double s_omega2;
   double s_time1;
   double s_time2;

   if ( use_time )
   {
      int       eftime = (int)expdata.scan[          0 ].time;
      int       sltime = (int)simdata.scan[ nsscan - 1 ].time;
//int ner=expdata.radius.size();
//int nsr=simdata.radius.size();
//int nec=expdata.scan[0].conc.size();
//int nsc=simdata.scan[0].conc.size();
//DbgLv(1) << "MATHi2: eftime sltime" << eftime << sltime << "fscan lscan" << fscan << lscan;
//DbgLv(1) << "MATHi2:  erad0 eradn" << expdata.radius[0] << expdata.radius[ner-1] << ner << nec;
//DbgLv(1) << "MATHi2:  srad0 sradn" << simdata.radius[0] << simdata.radius[nsr-1] << nsr << nsc;

      for ( int expscan = fscan; expscan < lscan; expscan++ )
      {  // Interpolate where needed to a range of experiment scans
         MfemScan* sscan1 = &simdata.scan[ simscan ];
         MfemScan* sscan2 = sscan1;
         MfemScan* escan  = &expdata.scan[ expscan ];

         e_omega    = escan->omega_s_t;
         e_time     = escan->time;

         while ( simdata.scan[ simscan ].time < e_time )
         {
            simscan ++;
            // Make sure we don't overrun bounds:
            if ( simscan == nsscan )
            {  // Sim scan count has exceeded limit
               if ( sltime <= e_time )
               {  // Output a message if time ranges overlap
                  qDebug() << "simulation time scan[" << simscan << "]: "
                           << simdata.scan[ simscan - 1 ].time
                           << ", expdata scan time[" << expscan << "]: "
                           << expdata.scan[ expscan ].time;

                  qDebug() << "The simulated data does not cover the entire "
                              "experimental time range and ends too early!\n"
                              "exiting...\n";
		  // exit(-1);
               }

#ifdef NO_DB
               //MPI_Abort( MPI_COMM_WORLD, -1 );
#endif
               simscan--;
	       break;
            }
         }

         int sscm  = ( simscan > 0 ) ? ( simscan - 1 ) : 1;
         sscan1    = &simdata.scan[ sscm ];
         sscan2    = &simdata.scan[ simscan ];
         s_omega1  = sscan1->omega_s_t;
         s_time1   = sscan1->time;
         s_omega2  = sscan2->omega_s_t;
         s_time2   = sscan2->time;

         // Check to see if the time is equal or larger:
         if ( s_time2 == e_time )
         { // they are the same, so take this scan
           // and push it onto the tmp_data array.
            tmp_data.scan << simdata.scan[ simscan ];
         }
         else // interpolation is needed
         {
//DbgLv(1) << "MATHi2:  sscm" << sscm << "s_time1 s_time2" << s_time1 << s_time2
// << "e_time" << e_time;
            double a;
            double b;
            tmp_scan.conc.fill( 0.0, nsconc );
            double* tsco  = tmp_scan.conc.data();
            double* s1co  = sscan1->conc.data();
            double* s2co  = sscan2->conc.data();
            double tdelt  = s_time2 - s_time1;

            // interpolate the concentration points:
            for ( int ii = 0; ii < nsconc; ii++ )
            {
               a          = ( s2co[ ii ] - s1co[ ii ] ) / tdelt;

               b          = s2co[ ii ] - a * s_time2;

               tsco[ ii ] = ( a * e_time + b );
            }

            // interpolate the omega_square_t integral data:

            a = ( s_omega2 - s_omega1 ) / tdelt;

            b = s_omega2 - a * s_time2;

            //escan->omega_s_t = a * e_time + b;
            tmp_scan.time      = e_time;
            tmp_scan.omega_s_t = a * e_time + b;

            tmp_data.scan << tmp_scan;
         }
      }
   }
   else // Use omega^2t integral for interpolation
   {
      for ( int expscan = fscan; expscan < lscan; expscan++ )
      { // Interpolate where needed to a range of experiment scans
         MfemScan* sscan1 = &simdata.scan[ simscan ];
         MfemScan* sscan2 = sscan1;
         MfemScan* escan  = &expdata.scan[ expscan ];

         e_omega    = escan->omega_s_t;
         e_time     = escan->time;
//DbgLv(1) << "MATHi: somg eomg" << simdata.scan[simscan].omega_s_t << e_omega
// << " escn sscn" << expscan << simscan;

         while ( simdata.scan[ simscan ].omega_s_t < e_omega )
         {
            simscan ++;
            // Make sure we don't overrun bounds:
            if ( simscan == (int) simdata.scan.size() )
            {
               qDebug() << "simulation omega^2t scan[" << simscan << "]: "
                        << simdata.scan[ simscan - 1 ].omega_s_t
                        << ", expdata scan omega^2t[" << expscan << "]: "
                        << expdata.scan[ expscan ].omega_s_t;
//DbgLv(1) << "e_omega e_time" << e_omega << e_time;
//int lsc=expdata.scan.size()-1;
//e_omega=expdata.scan[lsc].omega_s_t;
//e_time=expdata.scan[lsc].time;
//DbgLv(1) << "e-lsc e_omega e_time" << lsc << e_omega << e_time;
//lsc=simdata.scan.size()-1;
//e_omega=simdata.scan[lsc].omega_s_t;
//e_time=simdata.scan[lsc].time;
//DbgLv(1) << "s-lsc e_omega e_time" << lsc << e_omega << e_time;
               qDebug() << "The simulated data does not cover the entire "
                           "experimental time range and ends too early!\n"
                           "exiting...";
#ifdef NO_DB
               //MPI_Abort( MPI_COMM_WORLD, -1 );
#endif
//               exit( -1 );
               simscan--;
               break;
            }
         }

         int sscm  = ( simscan > 0 ) ? ( simscan - 1 ) : 1;
         sscan1    = &simdata.scan[ sscm ];
         sscan2    = &simdata.scan[ simscan ];
         s_omega1  = sscan1->omega_s_t;
         s_time1   = sscan1->time;
         s_omega2  = sscan2->omega_s_t;
         s_time2   = sscan2->time;

         // Check to see if the time is equal or larger:
         if ( s_omega2 == e_omega )
         { // They are the same, so take this scan and
           // push it onto the tmp_data array.
            tmp_data.scan << simdata.scan[ simscan ];
         }
         else // Interpolation is needed
         {
            double a;
            double b;
            tmp_scan.conc.clear();
            tmp_scan.conc.reserve( nsconc );

            // Interpolate the concentration points:
            for ( int ii = 0; ii < nsconc; ii++ )
            {
               a = ( sscan2->conc[ ii ] - sscan1->conc[ ii ] ) /
                   ( s_omega2 - s_omega1 );

               b = sscan2->conc[ ii ] - a * s_omega2;

               tmp_scan.conc << ( a * e_omega + b );
            }

#if 0
            // Interpolate the omega_square_t integral data:
            a = ( s_time2 - s_time1 ) / ( s_omega2 - s_omega1 );

            b = s_time2 - a * s_omega2;

            escan->time = a * e_omega + b;
#endif

            tmp_data.scan << tmp_scan;
         }
      }
   }

   // Interpolate all radial points from each scan in tmp_data onto expdata

   if ( tmp_data.radius[ 0 ] > expdata.radius[ 0 ] )
   {
      qDebug() << "Radius comparison: " << tmp_data.radius[ 0 ]
               << " (simulated), " << expdata.radius[ 0 ]
               << " (experimental)";

      qDebug() << "jj = " << 0 << ", simdata radius: "
               << tmp_data.radius[ 0 ] << ", expdata radius: "
               << expdata.radius[ 0 ];  // Changed from radius[ ii ]
                                         // ii out of scope

      qDebug() << "The simulated data radial range does not include the "
                  "beginning of the experimental data's radii!\n"
                  "exiting...";

#ifdef NO_DB
      //MPI_Abort( MPI_COMM_WORLD, -3 );
#endif
      exit( -3 );
   }

   int jj           = 0;
   double* tdrad    = tmp_data.radius.data();
   double* exrad    = expdata .radius.data();
   int tdradsz      = tmp_data.radius.size();

   for ( int ii = 0; ii < neconc; ii++ )
   {
      while ( tdrad[ jj ] < exrad[ ii ] )
      {
         double rrdif     = qAbs( tdrad[ jj ] - exrad[ ii ] );
if(rrdif<1.0e-4 && dbg_level>0)
{
 rrdk++;
 if ( rrdk < 11 )
  DbgLv(0) << "    jj ii" << jj << ii << "  trad erad diff"
   << tdrad[jj] << exrad[ii] << rrdif;
}
         if ( rrdif  < 1.0e-8 )
            break;
         jj++;
         // make sure we don't overrun bounds:
         if ( jj == tdradsz )
         {
            qDebug() << "The simulated data does not have enough "
                        "radial points and ends too early!\n"
	                "\n Please check your centerpiece selection \n"
                        "and make sure the bottom of the cell is correctly defined for this experiment!"
                        "exiting...";
DbgLv(1) << "jj ii sztrad szerad" << jj << ii << tdradsz << expdata.radius.size()
 <<  "trad erad" << tmp_data.radius[jj-1] << expdata.radius[ii] << rrdif;
#ifdef NO_DB
            //MPI_Abort( MPI_COMM_WORLD, -2 );
#endif
            exit( -2 );
         }
      }

      // check to see if the radius is equal or larger:
      if ( qAbs( tdrad[ jj ] - exrad[ ii ] ) < 1.0e-8 )
      { // they are virtually the same, so simply update the concentration value:
         int    ee        = 0;
         for ( int expscan = fscan; expscan < lscan; expscan++ )
            expdata.scan[ expscan ].conc[ ii ] +=
                             tmp_data.scan[ ee++ ].conc[ jj ];
      }
      else // interpolation is needed
      {
         int    ee        = 0;
         int    mm        = jj - 1;
         double radius1   = tdrad[ mm ];
         double radius2   = tdrad[ jj ];
         double eradius   = exrad[ ii ];
         double radrange  = radius2 - radius1;
//DbgLv(1) << "MATHi:  esize jj" << expdata.scan.size() << jj;
//DbgLv(1) << "MATHi:   r1 r2 er rr" << radius1 << radius2 << eradius << radrange
// << "fscan lscan" << fscan << lscan;

         for ( int expscan = fscan; expscan < lscan; expscan++ )
         {
            MfemScan* tscan = &tmp_data.scan[ ee++ ];

            double a = ( tscan->conc[ jj ] - tscan->conc[ mm ] ) / radrange;

            double b = tscan->conc[ jj ] - a * radius2;

            expdata.scan[ expscan ].conc[ ii ] += ( a * eradius + b );
         }
      }
   }
//*TIMING*
//QDateTime tEnd=QDateTime::currentDateTime();
//ncall++;
//totMs+=tStart.msecsTo(tEnd);
//if ((ncall%100)==1) DbgLv(1) << "interpolate() calls time-ms" << ncall << totMs;
//*TIMING*
   return 0;
}

void US_AstfemMath::QuadSolver( double* ai, double* bi, double* ci,
      double* di, double* cr, double* solu, int N )
{
// Solve Quad-diagonal system [a_i, *b_i*, c_i, d_i]*[x]=[r_i]
// b_i are on the main diagonal line
//
// Test
// n=100; a=-1+rand(100,1); b=2+rand(200,1); c=-0.7*rand(100,1); d=-0.3*rand(100,1);
// xs=rand(100,1);
// r(1)=b(1)*xs(1)+c(1)*xs(2)+d(1)*xs(3);
// for i=2:n-2,
// r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1)+d(i)*xs(i+2);
// end;
// i=n-1; r(i)=a(i)*xs(i-1)+b(i)*xs(i)+c(i)*xs(i+1);
// i=n;  r(i)=a(i)*xs(i-1)+b(i)*xs(i);

   QVector< double > caVec( N );
   QVector< double > cbVec( N );
   QVector< double > ccVec( N );
   QVector< double > cdVec( N );
   double* ca = caVec.data();
   double* cb = cbVec.data();
   double* cc = ccVec.data();
   double* cd = cdVec.data();

   for ( int i = 0; i < N; i++ )
   {
      ca[ i ] = ai[ i ];
      cb[ i ] = bi[ i ];
      cc[ i ] = ci[ i ];
      cd[ i ] = di[ i ];
   }

   for ( int i = 1; i <= N - 2; i++ )
   {
      double tmp = ca[ i ] / cb[ i - 1 ];

      cb[ i ] = cb[ i ] -cc [ i - 1] * tmp;
      cc[ i ] = cc[ i ] -cd [ i - 1] * tmp;
      cr[ i ] = cr[ i ] -cr [ i - 1] * tmp;
   }

   int   i   = N - 1;
   double tmp = ca[ i ] / cb[ i - 1 ];

   cb[ i]  = cb[ i ] - cc[ i - 1 ] * tmp;
   cr[ i ] = cr[ i ] - cr[ i - 1 ] * tmp;

   solu[ N - 1 ] =   cr[ N - 1 ]                                / cb[ N - 1 ];
   solu[ N - 2 ] = ( cr[ N - 2 ] - cc[ N - 2 ] * solu[ N - 1] ) / cb[ N - 2 ];

   i = N - 2;
   do
   {
      i--;
      solu[ i ] = (   cr[ i ]
                    - cc[ i ] * solu[ i + 1 ]
                    - cd[ i ] * solu[ i + 2 ] ) /
                  cb[ i ];

   } while ( i != 0 );
}

// old version: perform integration on supp(test function) separately
// on left Q and right T

void US_AstfemMath::IntQT1( double* vx, double D, double sw2,
                            double** Stif, double dt )
{
   // element to define basis functions

   int npts, i, k;
   double x_gauss, y_gauss, dval;
   double hh, slope, xn1, phiC, phiCx;
   double Lx[ 3 ], Ly[ 3 ];
   double Rx[ 4 ];
   double Qx[ 3 ], Qy[ 3 ];
   double Tx[ 3 ], Ty[ 3 ];
   double** StifL;
   double** StifR;
   double** Lam;
   double DJac;
   double phiL [ 3 ];
   double phiLx[ 3 ];
   double phiLy[ 3 ];
   double phiR [ 4 ];
   double phiRx[ 4 ];
   double phiRy[ 4 ];

   // elements for define the trial function phi
   Lx[ 0 ] = vx[0];    // vertices of left Triangle
   Lx[ 1 ] = vx[3];
   Lx[ 2 ] = vx[2];

   Ly[ 0 ] = 0.0;
   Ly[ 1 ] = dt;
   Ly[ 2 ] = dt;

   Rx[ 0 ] = vx[0]; // vertices of Q on right quadrilateral
   Rx[ 1 ] = vx[1];
   Rx[ 2 ] = vx[4];
   Rx[ 3 ] = vx[3];

   initialize_2d( 3, 2, &StifL );
   initialize_2d( 4, 2, &StifR );

   hh      = vx[ 3 ] - vx[ 2 ];
   slope   = ( vx[ 3 ] - vx[ 5 ] ) / dt;
   npts    = 28;
   initialize_2d( npts, 4, &Lam );
   DefineFkp( npts,    (double**)Lam );

   //
   // integration over element Q (a triangle):
   //
   Qx[ 0 ] = vx[0]; // vertices of Q on left
   Qx[ 1 ] = vx[3];
   Qx[ 2 ] = vx[2];

   Qy[ 0 ] = 0.0;
   Qy[ 1 ] = dt;
   Qy[ 2 ] = dt;

   for (k=0; k<npts; k++)
   {
      x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
      y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
      DJac    = 2.0 * AreaT( Qx, Qy );

      // trace-forward point at t_n+1 from (x_g, y_g)
      xn1     = x_gauss + slope * ( dt - y_gauss );

      //
      // find phi, phi_x, phi_y on L and C at (x,y)
      //

      BasisTR( Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy);
      // hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
      phiC    = ( xn1 - vx[ 2 ] ) / hh;
      phiCx   = 1. / hh;

      for (i=0; i<3; i++)
      {
         dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
               1.-phiC, -phiCx );
         StifL[i][0] += Lam[k][3] * DJac * dval;

         dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
               phiC, phiCx );
         StifL[i][1] += Lam[k][3] * DJac * dval;
      }
   }

   //
   // integration over T:
   //
   Tx[ 0 ] = vx[0]; // vertices of T on right
   Tx[ 1 ] = vx[5];
   Tx[ 2 ] = vx[3];

   Ty[ 0 ] = 0.0;
   Ty[ 1 ] = 0.0;
   Ty[ 2 ] = dt;

   for ( k = 0; k < npts; k++ )
   {
      x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
      y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
      DJac = 2.0 * AreaT( Tx, Ty );

      if( DJac < 1.e-22 ) break;

      xn1 = x_gauss + slope * ( dt - y_gauss ); // trace-forward point
                                                //  at t_n+1 from (x_g, y_g)
      //
      // find phi, phi_x, phi_y on R and C at (x,y)
      //

      BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt);
      phiC  = ( xn1 - vx[ 2 ] ) / hh;  // hat function on t_n+1, =1 at vx[3];
                                       //  =0 at vx[2]
      phiCx = 1. / hh;

      for (i = 0; i < 4; i++ )
      {
         dval             = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ],
                                       phiRy[ i ], 1. - phiC, -phiCx );
         StifR[ i ][ 0 ] += Lam[ k ][ 3 ] * DJac * dval;

         dval             = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ],
                                       phiRy[ i ], phiC, phiCx );
         StifR[ i ][ 1 ] += Lam[k][3] * DJac * dval;
      }
   }

   clear_2d( npts, Lam );

   for ( i = 0; i < 2; i++ )
   {
      Stif[ 0 ][ i ] = StifL[ 0 ][ i ] + StifR[ 0 ][ i ];
      Stif[ 1 ][ i ] =                   StifR[ 1 ][ i ];
      Stif[ 2 ][ i ] = StifL[ 2 ][ i ];
      Stif[ 3 ][ i ] = StifL[ 1 ][ i ] + StifR[ 3 ][ i ];
      Stif[ 4 ][ i ] =                   StifR[ 2 ][ i ];
   }

   clear_2d( 3, StifL );
   clear_2d( 4, StifR );
}

void US_AstfemMath::IntQTm( double* vx, double D, double sw2,
                            double** Stif, double dt )
{
   // element to define basis functions
   //
   int    npts, i, k;
   double x_gauss, y_gauss, dval;
   double Lx[ 4 ];
   double Cx[ 4 ];
   double Rx[ 4 ];
   double Qx[ 4 ], Qy[ 4 ];
   double Tx[ 3 ], Ty[ 3 ];
   double DJac;
   double** StifL;
   double** StifR;
   double** Lam;
   double** Gs;
   double phiL [ 4 ];
   double phiLx[ 4 ];
   double phiLy[ 4 ];
   double phiCx[ 4 ];
   double phiCy[ 4 ];
   double phiC [ 4 ];
   double phiR [ 4 ];
   double phiRx[ 4 ];
   double phiRy[ 4 ];

   Lx[ 0 ] = vx[ 0 ];
   Lx[ 1 ] = vx[ 1 ];
   Lx[ 2 ] = vx[ 4 ];
   Lx[ 3 ] = vx[ 3 ];      // vertices of left T

   Cx[ 0 ] = vx[ 6 ];
   Cx[ 1 ] = vx[ 7 ];
   Cx[ 2 ] = vx[ 4 ];
   Cx[ 3 ] = vx[ 3 ];

   Rx[ 0 ] = vx[ 1 ];      // vertices of Q on right
   Rx[ 1 ] = vx[ 2 ];
   Rx[ 2 ] = vx[ 5 ];
   Rx[ 3 ] = vx[ 4 ];

   initialize_2d( 4, 2, &StifL );
   initialize_2d( 4, 2, &StifR );

   //
   // integration over element Q :
   //
   Qx[ 0 ] = vx[ 6 ]; // vertices of Q on right
   Qx[ 1 ] = vx[ 1 ];
   Qx[ 2 ] = vx[ 4 ];
   Qx[ 3 ] = vx[ 3 ];

   Qy[ 0 ] = 0.0;
   Qy[ 1 ] = 0.0;
   Qy[ 2 ] = dt;
   Qy[ 3 ] = dt; // vertices of left T

   npts    = 5 * 5;
   initialize_2d( 25, 3, &Gs );
   DefineGaussian( 5,       (double**)Gs );

   double psi[  4 ];
   double psi1[ 4 ];
   double psi2[ 4 ];
   double jac[  4 ];

   for ( k = 0; k < npts; k++ )
   {
      BasisQS( Gs[k][0], Gs[k][1], psi, psi1, psi2 );

      x_gauss = 0.0;
      y_gauss = 0.0;
      for ( i = 0; i < 4; i++ )
      {
         jac[ i ] = 0.0;
      }
      for ( i = 0; i < 4; i++ )
      {
         x_gauss  += psi[ i ] * Qx[ i ];
         y_gauss  += psi[ i ] * Qy[ i ];
         jac[ 0 ] += Qx[ i ] * psi1[ i ];
         jac[ 1 ] += Qx[ i ] * psi2[ i ];
         jac[ 2 ] += Qy[ i ] * psi1[ i ];
         jac[ 3 ] += Qy[ i ] * psi2[ i ];
      }

      DJac = jac[ 0 ] * jac[ 3 ] - jac[ 1 ] * jac[ 2 ];

      //
      // find phi, phi_x, phi_y on L and C at (x,y)
      //

      BasisQR( Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt );
      BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );

      for ( i = 0; i < 4; i++ )
      {
         dval = Integrand( x_gauss, D, sw2, phiL[ i ], phiLx[ i ], phiLy[ i ],
                           phiC[ 0 ] + phiC[ 3 ], phiCx[ 0 ] + phiCx[ 3 ] );
         StifL[ i ][ 0 ] += Gs[ k ][ 2 ] * DJac * dval;

         dval = Integrand( x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
                           phiC[ 1 ] + phiC[ 2 ], phiCx[ 1 ] + phiCx[ 2 ] );
         StifL[ i ][ 1 ] += Gs[ k ][ 2 ] * DJac * dval;
      }
   }

   clear_2d( npts, Gs );

   //
   // integration over T:
   //
   Tx[ 0 ] = vx[ 1 ]; // vertices of T on right
   Tx[ 1 ] = vx[ 7 ];
   Tx[ 2 ] = vx[ 4 ];

   Ty[ 0 ] = 0.0;
   Ty[ 1 ] = 0.0;
   Ty[ 2 ] = dt;

   npts = 28;
   initialize_2d( npts, 4, &Lam );
   DefineFkp( npts,    (double**)Lam );

   for ( k = 0; k < npts; k++ )
   {
      x_gauss = Lam[ k ][ 0 ] * Tx[ 0 ] + Lam[ k ][ 1 ] * Tx[ 1 ] +
         Lam[ k ][ 2 ] * Tx[ 2 ];
      y_gauss = Lam[ k ][ 0 ] * Ty[ 0 ] + Lam[ k ][ 1 ] * Ty[ 1 ] +
         Lam[ k ][ 2 ] * Ty[ 2 ];
      DJac    = 2.0 * AreaT( Tx, Ty );

      if ( DJac < 1.e-22 ) break;

      //
      // find phi, phi_x, phi_y on R and C at (x,y)
      //

      BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt );
      BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );

      for (i = 0; i < 4; i++ )
      {
         dval = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ], phiRy[ i ],
                           phiC[ 0 ] + phiC[ 3 ], phiCx[ 0 ] + phiCx[ 3 ] );
         StifR[ i ][ 0 ] += Lam[ k ][ 3 ] * DJac * dval;

         dval = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ], phiRy[ i ],
                           phiC[ 1 ] + phiC[ 2 ], phiCx[ 1 ] + phiCx[ 2 ] );
         StifR[ i ][ 1 ] += Lam[ k ][ 3 ] * DJac * dval;
      }
   }

   clear_2d( npts, Lam );

   for ( i = 0; i < 2; i++ )
   {
      Stif[ 0 ][ i ] = StifL[ 0 ][ i ];
      Stif[ 1 ][ i ] = StifL[ 1 ][ i ] + StifR[ 0 ][ i ];
      Stif[ 2 ][ i ] =                   StifR[ 1 ][ i ];
      Stif[ 3 ][ i ] = StifL[ 3 ][ i ];
      Stif[ 4 ][ i ] = StifL[ 2 ][ i ] + StifR[ 3 ][ i ];
      Stif[ 5 ][ i ] =                   StifR[ 2 ][ i ];
   }

   clear_2d( 4, StifL );
   clear_2d( 4, StifR );
}


void US_AstfemMath::IntQTn2( double* vx, double D, double sw2,
                             double** Stif, double dt )
{
   // element to define basis functions
   //
   int    npts, i, k;
   double x_gauss, y_gauss, dval;
   double Lx[ 4 ];
   double Cx[ 4 ];
   double Rx[ 3 ], Ry[ 3 ];
   double Qx[ 4 ], Qy[ 4 ];
   double Tx[ 3 ], Ty[ 3 ];
   double DJac;
   double** StifL;
   double** StifR;
   double** Gs;
   double** Lam;
   double phiL [ 4 ];
   double phiLx[ 4 ];
   double phiLy[ 4 ];
   double phiCx[ 4 ];
   double phiCy[ 4 ];
   double phiC [ 4 ];
   double phiR [ 3 ];
   double phiRx[ 3 ];
   double phiRy[ 3 ];

   Lx[ 0 ] = vx[ 0 ];
   Lx[ 1 ] = vx[ 1 ];
   Lx[ 2 ] = vx[ 4 ];
   Lx[ 3 ] = vx[ 3 ];     // vertices of left T

   Cx[ 0 ] = vx[ 5 ];
   Cx[ 1 ] = vx[ 6 ];
   Cx[ 2 ] = vx[ 4 ];
   Cx[ 3 ] = vx[ 3 ];

   Rx[ 0 ] = vx[ 1 ];    // vertices of Q on right
   Rx[ 1 ] = vx[ 2 ];
   Rx[ 2 ] = vx[ 4 ];

   Ry[ 0 ] = 0.0;
   Ry[ 1 ] = 0.0;
   Ry[ 2 ] = dt;

   initialize_2d( 4, 2, &StifL );
   initialize_2d( 4, 2, &StifR );

   //
   // integration over element Q
   //
   Qx[ 0 ] = vx[ 5 ]; // vertices of Q on right
   Qx[ 1 ] = vx[ 1 ];
   Qx[ 2 ] = vx[ 4 ];
   Qx[ 3 ] = vx[ 3 ];

   Qy[ 0 ] = 0.0;
   Qy[ 1 ] = 0.0;
   Qy[ 2 ] = dt;
   Qy[ 3 ] = dt;

   npts    = 5 * 5;
   initialize_2d( npts, 3, &Gs );
   DefineGaussian( 5,       (double**)Gs );

   double psi[ 4 ], psi1[ 4 ], psi2[ 4 ], jac[ 4 ];

   for ( k = 0; k < npts; k++ )
   {
      BasisQS( Gs[ k ][ 0 ], Gs[ k ][ 1 ], psi, psi1, psi2 );

      x_gauss = 0.0;
      y_gauss = 0.0;

      for ( i = 0; i < 4; i++ )
      {
         jac[ i ] = 0.0;
      }

      for ( i = 0; i < 4; i++ )
      {
         x_gauss  += psi[ i ] * Qx[ i ];
         y_gauss  += psi[ i ] * Qy[ i ];
         jac[ 0 ] += Qx[ i ] * psi1[ i ];
         jac[ 1 ] += Qx[ i ] * psi2[ i ];
         jac[ 2 ] += Qy[ i ] * psi1[ i ];
         jac[ 3 ] += Qy[ i ] * psi2[ i ];
      }

      DJac = jac[ 0 ] * jac[ 3 ] - jac[ 1] * jac[ 2 ];

      //
      // find phi, phi_x, phi_y on L and C at (x,y)
      //

      BasisQR( Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt );
      BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );

      for ( i = 0; i < 4; i++ )
      {
         dval = Integrand( x_gauss, D, sw2, phiL[ i ], phiLx[ i ], phiLy[ i ],
                           phiC[ 0 ] + phiC[ 3 ], phiCx[ 0 ] + phiCx[ 3 ] );
         StifL[ i ][ 0 ] += Gs[ k ][ 2 ] * DJac * dval;

         dval = Integrand( x_gauss, D, sw2, phiL[ i ], phiLx[ i ], phiLy[ i ],
                           phiC[ 1 ] + phiC[ 2 ], phiCx[ 1 ] + phiCx[ 2 ] );
         StifL[ i ][ 1 ] += Gs[ k ][ 2 ] * DJac * dval;
      }
   }

   clear_2d( npts, Gs );

   //
   // integration over T:
   //
   Tx[ 0 ] = vx[ 1 ]; // vertices of T on right
   Tx[ 1 ] = vx[ 6 ];
   Tx[ 2 ] = vx[ 4 ];

   Ty[ 0 ] = 0.0;
   Ty[ 1 ] = 0.0;
   Ty[ 2 ] = dt;

   npts    = 28;
   initialize_2d( npts, 4, &Lam );
   DefineFkp( npts,    (double**)Lam );

   for ( k = 0; k < npts; k++ )
   {
      x_gauss = Lam[ k ][ 0 ] * Tx[ 0 ] + Lam[ k ][ 1 ] *
         Tx[ 1 ] + Lam[ k ][ 2 ] * Tx[ 2 ];
      y_gauss = Lam[ k ][ 0 ] * Ty[ 0 ] + Lam[ k ][ 1 ] *
         Ty[ 1 ] + Lam[ k ][ 2 ] * Ty[ 2 ];
      DJac    = 2.0 * AreaT( Tx, Ty );

      if ( DJac < 1.e-22 ) break;

      //
      // find phi, phi_x, phi_y on R and C at (x,y)
      //

      BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );
      BasisTR( Rx, Ry, x_gauss, y_gauss, phiR, phiRx, phiRy );

      for ( i = 0; i < 3; i++ )
      {
         dval = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ], phiRy[ i ],
                           phiC[ 0 ] + phiC[ 3 ], phiCx[ 0 ] + phiCx[ 3 ] );
         StifR[ i ][ 0 ] += Lam[ k ][ 3 ] * DJac * dval;

         dval = Integrand( x_gauss, D, sw2, phiR[ i ], phiRx[ i ], phiRy[ i ],
                           phiC[ 1 ] + phiC[ 2 ], phiCx[ 1 ] + phiCx[ 2 ] );
         StifR[ i ][ 1 ] += Lam[ k ][ 3 ] * DJac * dval;
      }
   }

   clear_2d( npts, Lam );

   for ( i = 0; i < 2; i++ )
   {
      Stif[ 0 ][ i ] = StifL[ 0 ][ i ];
      Stif[ 1 ][ i ] = StifL[ 1 ][ i ] + StifR[ 0 ][ i ];
      Stif[ 2 ][ i ] =                   StifR[ 1 ][ i ];
      Stif[ 3 ][ i ] = StifL[ 3 ][ i ];
      Stif[ 4 ][ i ] = StifL[ 2 ][ i ] + StifR[ 2 ][ i ];
   }

   clear_2d( 4, StifL );
   clear_2d( 4, StifR );
}

void US_AstfemMath::IntQTn1( double* vx, double D, double sw2,
                             double** Stif, double dt )
{
   // element to define basis functions
   //
   int    npts, i, k;
   double x_gauss, y_gauss, dval;
   double Lx[ 3 ], Ly[ 3 ];
   double Tx[ 3 ], Ty[ 3 ];
   double** StifR;
   double** Lam;
   double DJac;
   double phiL [ 4 ];
   double phiLx[ 4 ];
   double phiLy[ 4 ];

   Lx[ 0 ] = vx[ 0 ];
   Lx[ 1 ] = vx[ 1 ];
   Lx[ 2 ] = vx[ 2 ];

   Ly[ 0 ] = 0.0;
   Ly[ 1 ] = 0.0;
   Ly[ 2 ] = dt;

   initialize_2d( 4, 2, &StifR );

   //
   // integration over T:
   //
   Tx[ 0 ] = vx[ 3 ]; // vertices of T on right
   Tx[ 1 ] = vx[ 1 ];
   Tx[ 2 ] = vx[ 2 ];

   Ty[ 0 ] = 0.0;
   Ty[ 1 ] = 0.0;
   Ty[ 2 ] = dt;

   npts    = 28;
   initialize_2d( npts, 4, &Lam );
   DefineFkp( npts, Lam );

   for ( k = 0; k < npts; k++ )
   {
      x_gauss = Lam[ k ][ 0 ] * Tx[ 0 ] + Lam[ k ][ 1 ] *
                Tx[ 1 ] + Lam[ k ][ 2 ] * Tx[ 2 ];
      y_gauss = Lam[ k ][ 0 ] * Ty[ 0 ] + Lam[ k ][ 1 ] *
                Ty[ 1 ] + Lam[ k ][ 2 ] * Ty[ 2 ];
      DJac    = 2.0 * AreaT( Tx, Ty );

      if ( DJac < 1.e-22 ) break;

      // find phi, phi_x, phi_y on R and C at (x,y)


      BasisTR( Lx, Ly, x_gauss, y_gauss, phiL, phiLx, phiLy );

      for ( i = 0; i < 3; i++ )
      {
         dval = Integrand( x_gauss, D, sw2, phiL[ i ], phiLx[ i ], phiLy[ i ],
                           1.0, 0.0 );
         StifR[ i ][ 0 ] += Lam[ k ][ 3 ] * DJac * dval;
      }
   }

   for ( i = 0; i < 2; i++ )
   {
      Stif[ 0 ][ i ] = StifR[ 0 ][ i ];
      Stif[ 1 ][ i ] = StifR[ 1 ][ i ];
      Stif[ 2 ][ i ] = StifR[ 2 ][ i ];
   }

   clear_2d( npts, Lam );
   clear_2d( 4, StifR );
}

void US_AstfemMath::DefineFkp( int npts, double** Lam )
{
   // source: http://people.scs.fsu.edu/~burkardt/datasets/
   //                   quadrature_rules_tri/quadrature_rules_tri.html

   switch ( npts )
   {
      case 12:
      //  STRANG9, order 12, degree of precision 6.
      {
            Lam[  0 ][ 0 ] = 0.873821971016996;
            Lam[  1 ][ 0 ] = 0.063089014491502;
            Lam[  2 ][ 0 ] = 0.063089014491502;
            Lam[  3 ][ 0 ] = 0.501426509658179;
            Lam[  4 ][ 0 ] = 0.249286745170910;
            Lam[  5 ][ 0 ] = 0.249286745170910;
            Lam[  6 ][ 0 ] = 0.636502499121399;
            Lam[  7 ][ 0 ] = 0.636502499121399;
            Lam[  8 ][ 0 ] = 0.310352451033785;
            Lam[  9 ][ 0 ] = 0.310352451033785;
            Lam[ 10 ][ 0 ] = 0.053145049844816;
            Lam[ 11 ][ 0 ] = 0.053145049844816;

            Lam[  0 ][ 1 ] = 0.063089014491502;
            Lam[  1 ][ 1 ] = 0.873821971016996;
            Lam[  2 ][ 1 ] = 0.063089014491502;
            Lam[  3 ][ 1 ] = 0.249286745170910;
            Lam[  4 ][ 1 ] = 0.501426509658179;
            Lam[  5 ][ 1 ] = 0.249286745170910;
            Lam[  6 ][ 1 ] = 0.310352451033785;
            Lam[  7 ][ 1 ] = 0.053145049844816;
            Lam[  8 ][ 1 ] = 0.636502499121399;
            Lam[  9 ][ 1 ] = 0.053145049844816;
            Lam[ 10 ][ 1 ] = 0.636502499121399;
            Lam[ 11 ][ 1 ] = 0.310352451033785;

            Lam[  0 ][ 3 ] = 0.050844906370207;
            Lam[  1 ][ 3 ] = 0.050844906370207;
            Lam[  2 ][ 3 ] = 0.050844906370207;
            Lam[  3 ][ 3 ] = 0.116786275726379;
            Lam[  4 ][ 3 ] = 0.116786275726379;
            Lam[  5 ][ 3 ] = 0.116786275726379;
            Lam[  6 ][ 3 ] = 0.082851075618374;
            Lam[  7 ][ 3 ] = 0.082851075618374;
            Lam[  8 ][ 3 ] = 0.082851075618374;
            Lam[  9 ][ 3 ] = 0.082851075618374;
            Lam[ 10 ][ 3 ] = 0.082851075618374;
            Lam[ 11 ][ 3 ] = 0.082851075618374;
         break;
      }
      case 28:
      // TOMS612_28, order 28, degree of precision 11,
      // a rule from ACM TOMS algorithm #612
      {
         Lam[  0 ][ 0 ] = 0.33333333333333333;
         Lam[  1 ][ 0 ] = 0.9480217181434233;
         Lam[  2 ][ 0 ] = 0.02598914092828833;
         Lam[  3 ][ 0 ] = 0.02598914092828833;
         Lam[  4 ][ 0 ] = 0.8114249947041546;
         Lam[  5 ][ 0 ] = 0.09428750264792270;
         Lam[  6 ][ 0 ] = 0.09428750264792270;
         Lam[  7 ][ 0 ] = 0.01072644996557060;
         Lam[  8 ][ 0 ] = 0.4946367750172147;
         Lam[  9 ][ 0 ] = 0.4946367750172147;
         Lam[ 10 ][ 0 ] = 0.5853132347709715;
         Lam[ 11 ][ 0 ] = 0.2073433826145142;
         Lam[ 12 ][ 0 ] = 0.2073433826145142;
         Lam[ 13 ][ 0 ] = 0.1221843885990187;
         Lam[ 14 ][ 0 ] = 0.4389078057004907;
         Lam[ 15 ][ 0 ] = 0.4389078057004907;
         Lam[ 16 ][ 0 ] = 0.6779376548825902;
         Lam[ 17 ][ 0 ] = 0.6779376548825902;
         Lam[ 18 ][ 0 ] = 0.04484167758913055;
         Lam[ 19 ][ 0 ] = 0.04484167758913055;
         Lam[ 20 ][ 0 ] = 0.27722066752827925;
         Lam[ 21 ][ 0 ] = 0.27722066752827925;
         Lam[ 22 ][ 0 ] = 0.8588702812826364;
         Lam[ 23 ][ 0 ] = 0.8588702812826364;
         Lam[ 24 ][ 0 ] = 0.0000000000000000;
         Lam[ 25 ][ 0 ] = 0.0000000000000000;
         Lam[ 26 ][ 0 ] = 0.1411297187173636;
         Lam[ 27 ][ 0 ] = 0.1411297187173636;

         Lam[  0 ][ 1 ] = 0.333333333333333333;
         Lam[  1 ][ 1 ] = 0.02598914092828833;
         Lam[  2 ][ 1 ] = 0.9480217181434233;
         Lam[  3 ][ 1 ] = 0.02598914092828833;
         Lam[  4 ][ 1 ] = 0.09428750264792270;
         Lam[  5 ][ 1 ] = 0.8114249947041546;
         Lam[  6 ][ 1 ] = 0.09428750264792270;
         Lam[  7 ][ 1 ] = 0.4946367750172147;
         Lam[  8 ][ 1 ] = 0.01072644996557060;
         Lam[  9 ][ 1 ] = 0.4946367750172147;
         Lam[ 10 ][ 1 ] = 0.2073433826145142;
         Lam[ 11 ][ 1 ] = 0.5853132347709715;
         Lam[ 12 ][ 1 ] = 0.2073433826145142;
         Lam[ 13 ][ 1 ] = 0.4389078057004907;
         Lam[ 14 ][ 1 ] = 0.1221843885990187;
         Lam[ 15 ][ 1 ] = 0.4389078057004907;
         Lam[ 16 ][ 1 ] = 0.04484167758913055;
         Lam[ 17 ][ 1 ] = 0.27722066752827925;
         Lam[ 18 ][ 1 ] = 0.6779376548825902;
         Lam[ 19 ][ 1 ] = 0.27722066752827925;
         Lam[ 20 ][ 1 ] = 0.6779376548825902;
         Lam[ 21 ][ 1 ] = 0.04484167758913055;
         Lam[ 22 ][ 1 ] = 0.00000000000000000;
         Lam[ 23 ][ 1 ] = 0.1411297187173636;
         Lam[ 24 ][ 1 ] = 0.8588702812826364;
         Lam[ 25 ][ 1 ] = 0.1411297187173636;
         Lam[ 26 ][ 1 ] = 0.8588702812826364;
         Lam[ 27 ][ 1 ] = 0.0000000000000000;

         Lam[  0 ][ 3 ] = 0.08797730116222190;
         Lam[  1 ][ 3 ] = 0.008744311553736190;
         Lam[  2 ][ 3 ] = 0.008744311553736190;
         Lam[  3 ][ 3 ] = 0.008744311553736190;
         Lam[  4 ][ 3 ] = 0.03808157199393533;
         Lam[  5 ][ 3 ] = 0.03808157199393533;
         Lam[  6 ][ 3 ] = 0.03808157199393533;
         Lam[  7 ][ 3 ] = 0.01885544805613125;
         Lam[  8 ][ 3 ] = 0.01885544805613125;
         Lam[  9 ][ 3 ] = 0.01885544805613125;
         Lam[ 10 ][ 3 ] = 0.07215969754474100;
         Lam[ 11 ][ 3 ] = 0.07215969754474100;
         Lam[ 12 ][ 3 ] = 0.07215969754474100;
         Lam[ 13 ][ 3 ] = 0.06932913870553720;
         Lam[ 14 ][ 3 ] = 0.06932913870553720;
         Lam[ 15 ][ 3 ] = 0.06932913870553720;
         Lam[ 16 ][ 3 ] = 0.04105631542928860;
         Lam[ 17 ][ 3 ] = 0.04105631542928860;
         Lam[ 18 ][ 3 ] = 0.04105631542928860;
         Lam[ 19 ][ 3 ] = 0.04105631542928860;
         Lam[ 20 ][ 3 ] = 0.04105631542928860;
         Lam[ 21 ][ 3 ] = 0.04105631542928860;
         Lam[ 22 ][ 3 ] = 0.007362383783300573;
         Lam[ 23 ][ 3 ] = 0.007362383783300573;
         Lam[ 24 ][ 3 ] = 0.007362383783300573;
         Lam[ 25 ][ 3 ] = 0.007362383783300573;
         Lam[ 26 ][ 3 ] = 0.007362383783300573;
         Lam[ 27 ][ 3 ] = 0.007362383783300573;
         break;
      }
      case 37:
      //   TOMS706_37, order 37, degree of precision 13, a rule from ACM TOMS algorithm #706.
      {
         Lam[  0 ][ 0 ] = 0.333333333333333333333333333333;
         Lam[  1 ][ 0 ] = 0.950275662924105565450352089520;
         Lam[  2 ][ 0 ] = 0.024862168537947217274823955239;
         Lam[  3 ][ 0 ] = 0.024862168537947217274823955239;
         Lam[  4 ][ 0 ] = 0.171614914923835347556304795551;
         Lam[  5 ][ 0 ] = 0.414192542538082326221847602214;
         Lam[  6 ][ 0 ] = 0.414192542538082326221847602214;
         Lam[  7 ][ 0 ] = 0.539412243677190440263092985511;
         Lam[  8 ][ 0 ] = 0.230293878161404779868453507244;
         Lam[  9 ][ 0 ] = 0.230293878161404779868453507244;
         Lam[ 10 ][ 0 ] = 0.772160036676532561750285570113;
         Lam[ 11 ][ 0 ] = 0.113919981661733719124857214943;
         Lam[ 12 ][ 0 ] = 0.113919981661733719124857214943;
         Lam[ 13 ][ 0 ] = 0.009085399949835353883572964740;
         Lam[ 14 ][ 0 ] = 0.495457300025082323058213517632;
         Lam[ 15 ][ 0 ] = 0.495457300025082323058213517632;
         Lam[ 16 ][ 0 ] = 0.062277290305886993497083640527;
         Lam[ 17 ][ 0 ] = 0.468861354847056503251458179727;
         Lam[ 18 ][ 0 ] = 0.468861354847056503251458179727;
         Lam[ 19 ][ 0 ] = 0.022076289653624405142446876931;
         Lam[ 20 ][ 0 ] = 0.022076289653624405142446876931;
         Lam[ 21 ][ 0 ] = 0.851306504174348550389457672223;
         Lam[ 22 ][ 0 ] = 0.851306504174348550389457672223;
         Lam[ 23 ][ 0 ] = 0.126617206172027096933163647918;
         Lam[ 24 ][ 0 ] = 0.126617206172027096933163647918;
         Lam[ 25 ][ 0 ] = 0.018620522802520968955913511549;
         Lam[ 26 ][ 0 ] = 0.018620522802520968955913511549;
         Lam[ 27 ][ 0 ] = 0.689441970728591295496647976487;
         Lam[ 28 ][ 0 ] = 0.689441970728591295496647976487;
         Lam[ 29 ][ 0 ] = 0.291937506468887771754472382212;
         Lam[ 30 ][ 0 ] = 0.291937506468887771754472382212;
         Lam[ 31 ][ 0 ] = 0.096506481292159228736516560903;
         Lam[ 32 ][ 0 ] = 0.096506481292159228736516560903;
         Lam[ 33 ][ 0 ] = 0.635867859433872768286976979827;
         Lam[ 34 ][ 0 ] = 0.635867859433872768286976979827;
         Lam[ 35 ][ 0 ] = 0.267625659273967961282458816185;
         Lam[ 36 ][ 0 ] = 0.267625659273967961282458816185;

         Lam[  0 ][ 1 ] = 0.333333333333333333333333333333;
         Lam[  1 ][ 1 ] = 0.024862168537947217274823955239;
         Lam[  2 ][ 1 ] = 0.950275662924105565450352089520;
         Lam[  3 ][ 1 ] = 0.024862168537947217274823955239;
         Lam[  4 ][ 1 ] = 0.414192542538082326221847602214;
         Lam[  5 ][ 1 ] = 0.171614914923835347556304795551;
         Lam[  6 ][ 1 ] = 0.414192542538082326221847602214;
         Lam[  7 ][ 1 ] = 0.230293878161404779868453507244;
         Lam[  8 ][ 1 ] = 0.539412243677190440263092985511;
         Lam[  9 ][ 1 ] = 0.230293878161404779868453507244;
         Lam[ 10 ][ 1 ] = 0.113919981661733719124857214943;
         Lam[ 11 ][ 1 ] = 0.772160036676532561750285570113;
         Lam[ 12 ][ 1 ] = 0.113919981661733719124857214943;
         Lam[ 13 ][ 1 ] = 0.495457300025082323058213517632;
         Lam[ 14 ][ 1 ] = 0.009085399949835353883572964740;
         Lam[ 15 ][ 1 ] = 0.495457300025082323058213517632;
         Lam[ 16 ][ 1 ] = 0.468861354847056503251458179727;
         Lam[ 17 ][ 1 ] = 0.062277290305886993497083640527;
         Lam[ 18 ][ 1 ] = 0.468861354847056503251458179727;
         Lam[ 19 ][ 1 ] = 0.851306504174348550389457672223;
         Lam[ 20 ][ 1 ] = 0.126617206172027096933163647918;
         Lam[ 21 ][ 1 ] = 0.022076289653624405142446876931;
         Lam[ 22 ][ 1 ] = 0.126617206172027096933163647918;
         Lam[ 23 ][ 1 ] = 0.022076289653624405142446876931;
         Lam[ 24 ][ 1 ] = 0.851306504174348550389457672223;
         Lam[ 25 ][ 1 ] = 0.689441970728591295496647976487;
         Lam[ 26 ][ 1 ] = 0.291937506468887771754472382212;
         Lam[ 27 ][ 1 ] = 0.018620522802520968955913511549;
         Lam[ 28 ][ 1 ] = 0.291937506468887771754472382212;
         Lam[ 29 ][ 1 ] = 0.018620522802520968955913511549;
         Lam[ 30 ][ 1 ] = 0.689441970728591295496647976487;
         Lam[ 31 ][ 1 ] = 0.635867859433872768286976979827;
         Lam[ 32 ][ 1 ] = 0.267625659273967961282458816185;
         Lam[ 33 ][ 1 ] = 0.096506481292159228736516560903;
         Lam[ 34 ][ 1 ] = 0.267625659273967961282458816185;
         Lam[ 35 ][ 1 ] = 0.096506481292159228736516560903;
         Lam[ 36 ][ 1 ] = 0.635867859433872768286976979827;

         Lam[  0 ][ 3 ] = 0.051739766065744133555179145422;
         Lam[  1 ][ 3 ] = 0.008007799555564801597804123460;
         Lam[  2 ][ 3 ] = 0.008007799555564801597804123460;
         Lam[  3 ][ 3 ] = 0.008007799555564801597804123460;
         Lam[  4 ][ 3 ] = 0.046868898981821644823226732071;
         Lam[  5 ][ 3 ] = 0.046868898981821644823226732071;
         Lam[  6 ][ 3 ] = 0.046868898981821644823226732071;
         Lam[  7 ][ 3 ] = 0.046590940183976487960361770070;
         Lam[  8 ][ 3 ] = 0.046590940183976487960361770070;
         Lam[  9 ][ 3 ] = 0.046590940183976487960361770070;
         Lam[ 10 ][ 3 ] = 0.031016943313796381407646220131;
         Lam[ 11 ][ 3 ] = 0.031016943313796381407646220131;
         Lam[ 12 ][ 3 ] = 0.031016943313796381407646220131;
         Lam[ 13 ][ 3 ] = 0.010791612736631273623178240136;
         Lam[ 14 ][ 3 ] = 0.010791612736631273623178240136;
         Lam[ 15 ][ 3 ] = 0.010791612736631273623178240136;
         Lam[ 16 ][ 3 ] = 0.032195534242431618819414482205;
         Lam[ 17 ][ 3 ] = 0.032195534242431618819414482205;
         Lam[ 18 ][ 3 ] = 0.032195534242431618819414482205;
         Lam[ 19 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 20 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 21 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 22 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 23 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 24 ][ 3 ] = 0.015445834210701583817692900053;
         Lam[ 25 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 26 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 27 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 28 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 29 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 30 ][ 3 ] = 0.017822989923178661888748319485;
         Lam[ 31 ][ 3 ] = 0.037038683681384627918546472190;
         Lam[ 32 ][ 3 ] = 0.037038683681384627918546472190;
         Lam[ 33 ][ 3 ] = 0.037038683681384627918546472190;
         Lam[ 34 ][ 3 ] = 0.037038683681384627918546472190;
         Lam[ 35 ][ 3 ] = 0.037038683681384627918546472190;
         Lam[ 36 ][ 3 ] = 0.037038683681384627918546472190;

         break;
      }
      default:
      {
         return;
      }
   }

   for( int i = 0; i < npts; i++ )
   {
      Lam[ i ][ 2 ]  = 1. - Lam[ i ][ 0 ] - Lam[ i ][ 1 ];
      // To make the sum( wt ) = 0.5 = area of standard elem
      Lam[ i ][ 3 ] /= 2.;
   }
}

// AreaT: area of a triangle (v1, v2, v3)

double US_AstfemMath::AreaT( double* xv, double* yv )
{
   return ( 0.5 * ( ( xv[ 1 ] - xv[ 0 ] ) * ( yv[ 2 ] - yv[ 0 ] )
                  - ( xv[ 2 ] - xv[ 0 ] ) * ( yv[ 1 ] - yv[ 0 ] ) ) );
}

// Computer basis on standard element

void US_AstfemMath::BasisTS( double xi, double et, double* phi,
                             double* phi1, double* phi2 )
{
   //function [phi, phi1, phi2] = BasisTS(xi,et)

   phi [ 0 ] =  1.0 - xi - et;
   phi1[ 0 ] = -1.0;
   phi2[ 0 ] = -1.0;

   phi [ 1 ] = xi;
   phi1[ 1 ] = 1.0;
   phi2[ 1 ] = 0.0;

   phi [ 2 ] = et;
   phi1[ 2 ] = 0.0;
   phi2[ 2 ] = 1.0;
}

// Computer basis on standard element

void US_AstfemMath::BasisQS( double xi, double et, double* phi,
                             double* phi1, double* phi2 )
{

   phi [ 0 ] = ( 1.0 - xi ) * ( 1.0 - et );
   phi1[ 0 ] =  -1.0        * ( 1.0 - et );
   phi2[ 0 ] =  -1.0        * ( 1.0 - xi );

   phi [ 1 ] = xi * ( 1.0 - et );
   phi1[ 1 ] = 1.0 - et;
   phi2[ 1 ] = -xi;

   phi [ 2 ] = xi * et;
   phi1[ 2 ] = et;
   phi2[ 2 ] = xi;

   phi [ 3 ] = ( 1.0 - xi ) * et;
   phi1[ 3 ] = -et;
   phi2[ 3 ] = 1.0 - xi;
}

// Function BasisTR: compute basis on real element T:
// phi, phi_x, phi_t at a given (xs,ts) point
// the triangular is assumed to be (x1,y1), (x2, y2), (x3, y3)

void US_AstfemMath::BasisTR( double* vx, double* vy,
      double xs, double ys, double* phi, double* phix, double* phiy )
{
   // find (xi,et) corresponding to (xs, ts)

   int i;
   double tempv1[ 3 ], tempv2[ 3 ];

   tempv1[ 0 ] = xs;
   tempv1[ 1 ] = vx[ 2 ];
   tempv1[ 2 ] = vx[ 0 ];
   tempv2[ 0 ] = ys;
   tempv2[ 1 ] = vy[ 2 ];
   tempv2[ 2 ] = vy[ 0 ];

   double AreaK = AreaT( vx, vy );
   double xi    = AreaT( tempv1, tempv2 ) / AreaK;

   tempv1[ 0 ] = xs;
   tempv1[ 1 ] = vx[ 0 ];
   tempv1[ 2 ] = vx[ 1 ];
   tempv2[ 0 ] = ys;
   tempv2[ 1 ] = vy[ 0 ];
   tempv2[ 2 ] = vy[ 1 ];

   double et = AreaT( tempv1, tempv2 ) / AreaK;

   double phi1[ 3 ];
   double phi2[ 3 ];

   BasisTS( xi, et, phi, phi1, phi2 );

   double Jac[ 4 ], JacN[ 4 ], det;

   Jac[ 0 ] = vx[ 0 ] * phi1[ 0 ] + vx[ 1 ] * phi1[ 1 ] + vx[ 2 ] * phi1[ 2 ];
   Jac[ 1 ] = vx[ 0 ] * phi2[ 0 ] + vx[ 1 ] * phi2[ 1 ] + vx[ 2 ] * phi2[ 2 ];
   Jac[ 2 ] = vy[ 0 ] * phi1[ 0 ] + vy[ 1 ] * phi1[ 1 ] + vy[ 2 ] * phi1[ 2 ];
   Jac[ 3 ] = vy[ 0 ] * phi2[ 0 ] + vy[ 1 ] * phi2[ 1 ] + vy[ 2 ] * phi2[ 2 ];

   det      = Jac[ 0 ] * Jac[ 3 ] - Jac[ 1 ] * Jac [ 2 ];

   JacN[ 0 ] = Jac[3]/det;
   JacN[ 1 ] = -Jac[1]/det;
   JacN[ 2 ] = -Jac[2]/det;
   JacN[ 3 ] = Jac[0]/det;

   for ( i = 0; i < 3; i++ )
   {
      phix[ i ] = JacN[ 0 ] * phi1[ i ] + JacN[ 2 ] * phi2[ i ];
      phiy[ i ] = JacN[ 1 ] * phi1[ i ] + JacN[ 3 ] * phi2[ i ];
   }
}

void US_AstfemMath::BasisQR( double* vx, double xs, double ts,
      double* phi, double* phix, double* phiy, double dt )
{
   int    i;

   // find (xi,et) corresponding to (xs, ts)
   double et = ts / dt;
   double A  = vx[ 0 ] * ( 1.0 - et ) + vx[ 3 ] * et;
   double B  = vx[ 1 ] * ( 1.0 - et ) + vx[ 2 ] * et;
   double xi = ( xs - A ) / ( B - A );

   double phi1[ 4 ];
   double phi2[ 4 ];

   BasisQS( xi, et, phi, phi1, phi2 );

   double Jac[ 4 ], JacN[ 4 ], det;

   Jac[ 0] = vx[ 0 ] * phi1[ 0 ] + vx[ 1 ] * phi1[ 1 ] + vx[ 2 ] *
             phi1[ 2 ] + vx[ 3 ] * phi1[ 3 ];
   Jac[ 1] = vx[ 0 ] * phi2[ 0 ] + vx[ 1 ] * phi2[ 1 ] + vx[ 2 ] *
             phi2[ 2 ] + vx[ 3 ] * phi2[ 3 ];
   Jac[ 2] = dt * phi1[ 2 ] + dt * phi1[ 3 ];
   Jac[ 3] = dt * phi2[ 2 ] + dt * phi2[ 3 ];

   det = Jac[0] * Jac[3] - Jac[1] * Jac [2];

   JacN[ 0 ] =  Jac[ 3 ] / det;
   JacN[ 1 ] = -Jac[ 1 ] / det;
   JacN[ 2 ] = -Jac[ 2 ] / det;
   JacN[ 3 ] =  Jac[ 0 ] / det;

   for ( i = 0; i < 4; i++ )
   {
      phix[ i ] = JacN[ 0 ] * phi1[ i ] + JacN[ 2 ] * phi2[ i ];
      phiy[ i ] = JacN[ 1 ] * phi1[ i ] + JacN[ 3 ] * phi2[ i ];
   }
}

// Integrand for Lamm equation

double US_AstfemMath::Integrand( double x_gauss, double D, double sw2,
   double u, double ux, double ut, double v, double vx )
{
   return ( x_gauss * ut * v + D * x_gauss * ux * vx -
            sw2 * sq( x_gauss ) * u * vx );
}

// Source: http://www.math.ntnu.no/num/nnm/Program/Numlibc/

void US_AstfemMath::DefineGaussian( int nGauss, double** Gs2 )
{
   double* Gs1 = new double [ nGauss ];
   double* w   = new double [ nGauss ];

   switch ( nGauss )
   {
      case 3:
      {
         Gs1[ 0 ] = -0.774596669241483;  w[ 0 ] = 5.0 / 9.0;
         Gs1[ 1 ] = 0.0;                 w[ 1 ] = 8.0 / 9.0;
         Gs1[ 2 ] = 0.774596669241483;   w[ 2 ] = 5.0 / 9.0;
         break;
      }
      case 5:
      {
         Gs1[ 0 ] = 0.906179845938664;   w[ 0 ] = 0.236926885056189;
         Gs1[ 1 ] = 0.538469310105683;   w[ 1 ] = 0.478628670499366;
         Gs1[ 2 ] = 0.000000000000000;   w[ 2 ] = 0.568888888888889;
         Gs1[ 3 ] = -Gs1[ 1 ];           w[ 3 ] = w[ 1 ];
         Gs1[ 4 ] = -Gs1[ 0 ];           w[ 4 ] = w[ 0 ];
         break;
      }
      case 7:
      {
         Gs1[ 0 ] = 0.949107912342759;   w[ 0 ] = 0.129484966168870;
         Gs1[ 1 ] = 0.741531185599394;   w[ 1 ] = 0.279705391489277;
         Gs1[ 2 ] = 0.405845151377397;   w[ 2 ] = 0.381830050505119;
         Gs1[ 3 ] = 0.000000000000000;   w[ 3 ] = 0.417959183673469;
         Gs1[ 4 ] = -Gs1[ 2 ];           w[ 4 ] = w[ 2 ];
         Gs1[ 5 ] = -Gs1[ 1 ];           w[ 5 ] = w[ 1 ];
         Gs1[ 6 ] = -Gs1[ 0 ];           w[ 6 ] = w[ 0 ];
         break;
      }
      case 10:
      {
         Gs1[ 0 ] = 0.973906528517172;   w[ 0 ] = 0.066671344308688;
         Gs1[ 1 ] = 0.865063366688985;   w[ 1 ] = 0.149451349150581;
         Gs1[ 2 ] = 0.679409568299024;   w[ 2 ] = 0.219086362515982;
         Gs1[ 3 ] = 0.433395394129247;   w[ 3 ] = 0.269266719309996;
         Gs1[ 4 ] = 0.148874338981631;   w[ 4 ] = 0.295524224714753;
         Gs1[ 5 ] = -Gs1[ 4 ];           w[ 5 ] = w[ 4 ];
         Gs1[ 6 ] = -Gs1[ 3 ];           w[ 6 ] = w[ 3 ];
         Gs1[ 7 ] = -Gs1[ 2 ];           w[ 7 ] = w[ 2 ];
         Gs1[ 8 ] = -Gs1[ 1 ];           w[ 8 ] = w[ 1 ];
         Gs1[ 9 ] = -Gs1[ 0 ];           w[ 9 ] = w[ 0 ];
         break;
      }
      default:
      {
         return;
      }
   }

   for ( int i = 0; i < nGauss; i++ )
   {
      for ( int j = 0; j < nGauss; j++ )   // map to [0,1] x [0,1]
      {
         int k = j + i * nGauss;

         Gs2[ k ][ 0 ] = ( Gs1[ i ] + 1.0 ) / 2.0;
         Gs2[ k ][ 1 ] = ( Gs1[ j ] + 1.0 ) / 2.0;

         Gs2[ k ][ 2 ] = w[ i ] * w[ j ] / 4.0;
      }
   }

   delete [] w;
   delete [] Gs1;
}

// Initialize a simulation RawData object to have sizes,ranges,controls
//   that mirror those of an experimental EditedData object
void US_AstfemMath::initSimData( US_DataIO::RawData& simdata,
      US_DataIO::EditedData& editdata, double concval1 )
{

   int    nconc = editdata.pointCount();
   int    nscan = editdata.scanCount();

   // copy general control variable values
   simdata.type[ 0 ]     = ' ';
   simdata.type[ 1 ]     = ' ';

   for ( int jj = 0; jj < 16; jj++ )
      simdata.rawGUID[ jj ] = ' ';

   simdata.cell          = editdata.cell.toInt();
   simdata.channel       = editdata.channel.at( 0 ).toLatin1();
   simdata.description   = editdata.description;
   simdata.xvalues       = editdata.xvalues;

   simdata.scanData.resize( nscan );
   double rvalue         = concval1;  // 1st scan constant concentration value

   for ( int ii = 0; ii < nscan; ii++ )
   {  // Loop to copy scans
      US_DataIO::Scan* escan = &editdata.scanData[ ii ];
      US_DataIO::Scan  sscan;

      sscan.temperature      = escan->temperature;
      sscan.rpm              = escan->rpm;
      sscan.seconds          = escan->seconds;
      sscan.omega2t          = escan->omega2t;
      sscan.wavelength       = escan->wavelength;
      sscan.plateau          = escan->plateau;
      sscan.delta_r          = escan->delta_r;
      sscan.interpolated     = escan->interpolated;

      sscan.rvalues.fill( rvalue, nconc );

      simdata.scanData[ ii ] = sscan;

      // Set values to zero for 2nd and subsequent scans
      rvalue                 = 0.0;
   }

   return;
}

// Initialize a simulation global-fit RawData object to have
//   sizes,ranges,controls that mirror those of a list of EditedData objects
void US_AstfemMath::initSimData( US_DataIO::RawData& simdata,
      QList< US_DataIO::EditedData* >& edats, double concval1 )
{
   // Size simulation data and initialize concentrations to zero
   int ndats   = edats.count();
   int kscans  = 0;
   int jscan   = 0;

   for ( int ee = 0; ee < ndats; ee++ )
   {
      US_DataIO::RawData tdata;      // Init temp sim data with edata's grid
      US_AstfemMath::initSimData( tdata, *edats[ ee ], concval1 );

      int nscans  = tdata.scanData.size();
      kscans     += nscans;
      simdata.scanData.resize( kscans );

      if ( ee == 0 )
      {
         simdata     = tdata;   // Init (first) dataset sim data
         jscan      += nscans;
      }
      else
      {
         for ( int ss = 0; ss < nscans; ss++ )
         {  // Append zeroed-scans sim_data for multiple data sets
            simdata.scanData[ jscan++ ] = tdata.scanData[ ss ];
         }
      }
   }
}

// Calculate variance (average difference squared) between simulated and
//  experimental data.
double US_AstfemMath::variance( US_DataIO::RawData&    simdata,
                                US_DataIO::EditedData& editdata )
{
   QList< int > escns;

   return variance( simdata, editdata, escns );
}

// Calculate variance (average difference squared) between simulated and
//  experimental data.
double US_AstfemMath::variance( US_DataIO::RawData&    simdata,
                                US_DataIO::EditedData& editdata,
                                QList< int > exclScans )
{
   int    nscan = simdata .scanCount();
   int    kscan = editdata.scanCount();
   int    nconc = simdata .pointCount();
   int    kconc = editdata.pointCount();
   double vari  = 0.0;

   if ( nscan != kscan )
   {
      qDebug() << "*WARNING* variance(): sim/exp scan counts differ";
      nscan   = ( nscan < kscan ) ? nscan : kscan;
   }

   if ( nconc != kconc )
   {
      qDebug() << "*WARNING* variance(): sim/exp readings counts differ";
      nconc   = ( nconc < kconc ) ? nconc : kconc;
   }

   kscan      = 0;

   for ( int ii = 0; ii < nscan; ii++ )
   {  // accumulate sum of differences squared (readings in scans)

      if ( exclScans.contains( ii ) )  continue;

      kscan++;

      for ( int jj = 0; jj < nconc; jj++ )
      {
         vari   += sq( simdata.value( ii, jj ) - editdata.value( ii, jj ) );
      }
   }

   vari  /= (double)( kscan * nconc );  // variance is average diff-squared

   return vari;
}

// Calculate bottom radius value using channel bottom and rotor coeff. array
double US_AstfemMath::calc_bottom( double rpm, double bottom_chan,
      double* rotorcoefs )
{
   return ( bottom_chan
          + rotorcoefs[ 0 ] * rpm
          + rotorcoefs[ 1 ] * sq( rpm ) );
}


#ifdef NEVER
#if defined(DEBUG_ALLOC)

struct _created_matrices {
  long addr;
  int  val1;
  int  val2;
};

static list <_created_matrices> created_matrices;

void init_matrices_alloc()
{
  created_matrices.clear();
  puts( "init_matrices_alloc()" );
}

void list_matrices_alloc()
{
  if ( created_matrices.size() )
  {
    printf( "allocated matrices:\n" 0 );
  }

  list<_created_matrices>::iterator Li;

  for ( Li = created_matrices.begin(); Li != created_matrices.end(); Li++ )
  {
     printf( "addr %lx val1 %u val2 %u\n", Li->addr, Li->val1, Li->val2 );
  }
}

static list<_created_matrices>::iterator find_matrices_alloc( long addr )
{
   list<_created_matrices>::iterator Li;

   for ( Li = created_matrices.begin(); Li != created_matrices.end(); Li++ )
   {
      if ( Li->addr == addr )
      {
         return Li;
      }
   }
   return Li;
}

#endif












void IntQT1_ellam(QVector <double> vx, double D, double sw2, double **Stif, double dt)
{
   // element to define basis functions
   //
   int npts, i, k;
   double x_gauss, y_gauss, dval;
   QVector <double>  Rx, Ry, Qx, Qy;
   double **StifR=NULL, DJac;
   double hh, slope, xn1, phiC, phiCx;
   double phiR [4];
   double phiRx[4];
   double phiRy[4];

   Rx.clear();
   Ry.clear();
   Rx.push_back(vx[0]); // vertices of Q on right
   Rx.push_back(vx[1]);
   Rx.push_back(vx[3]);
   Rx.push_back(vx[2]);

   Ry.push_back(0.0);
   Ry.push_back(0.0);
   Ry.push_back(dt);
   Ry.push_back(dt);

   initialize_2d(4, 2, &StifR);
   hh = vx[3] - vx[2];
   slope = (vx[3] - vx[4])/dt;

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[1]-vx[4])/(vx[1]-vx[0]) <1.e-3 )     // Q_{0,4,3,2} is almost degenerated into a triangle
   {
       // elements for integration
       //
       Qx.clear();
       Qy.clear();
       Qx.push_back(vx[0]);   // vertices of Q on right
       Qx.push_back(vx[3]);
       Qx.push_back(vx[2]);
       Qy.push_back(0.0);
       Qy.push_back(dt);
       Qy.push_back(dt);

       double **Lam;
       npts = 28;
       initialize_2d(npts, 4, &Lam);
       DefineFkp(npts, Lam);

       for (k=0; k<npts; k++)
       {
          x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
          y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
          DJac = 2.0 * AreaT(&Qx, &Qy);

          //
          // find phi, phi_x, phi_y on R and C at (x,y)
          //

          BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt );

          xn1 = x_gauss + slope * ( dt - y_gauss );   // trace-forward point at t_n+1 from (x_g, y_g)
          phiC  = ( xn1 - vx[2] )/hh;     // hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
          phiCx = 1./hh;

          for (i=0; i<4; i++)
          {
             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
             StifR[i][0] += Lam[k][3] * DJac * dval;

             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
             StifR[i][1] += Lam[k][3] * DJac * dval;
          }
      }
      clear_2d(npts, Lam);

   }
   else
   {  // Q_{0,4,3,2} is non-degenerate
      // elements for integration
      //
      Qx.clear();
      Qy.clear();
      Qx.push_back(vx[0]); // vertices of Q on right
      Qx.push_back(vx[4]);
      Qx.push_back(vx[3]);
      Qx.push_back(vx[2]);

      Qy.push_back(0.0);
      Qy.push_back(0.0);
      Qy.push_back(dt);
      Qy.push_back(dt);

      double **Gs=NULL;
      npts = 5 * 5;
      initialize_2d(npts, 3, &Gs);
      DefineGaussian(5, Gs);

      double psi[4], psi1[4], psi2[4], jac[4];
      for (k=0; k<npts; k++)
      {
         BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);
         x_gauss = 0.0;
         y_gauss = 0.0;
         for (i=0; i<4; i++)
         {
            jac[i] = 0.0;
         }
         for (i=0; i<4; i++)
         {
            x_gauss += psi[i] * Qx[i];
            y_gauss += psi[i] * Qy[i];
            jac[0] += Qx[i] * psi1[i];
            jac[1] += Qx[i] * psi2[i];
            jac[2] += Qy[i] * psi1[i];
            jac[3] += Qy[i] * psi2[i];
         }
         DJac = jac[0] * jac[3] - jac[1] * jac[2];
         //
         // find phi, phi_x, phi_y on L and C at (x,y)
         //
         BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt );
         xn1 = x_gauss + slope * ( dt - y_gauss ); // trace-forward point at t_n+1 from (x_g, y_g)
         phiC  = ( xn1 - vx[2] )/hh;      // hat function on t_n+1, =1 at vx[3]; =0 at vx[2]
         phiCx = 1./hh;
         for (i=0; i<4; i++)
         {
            dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i], 1.-phiC, -phiCx);
            StifR[i][0] += Gs[k][2] * DJac * dval;
            dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],    phiC,  phiCx);
            StifR[i][1] += Gs[k][2] * DJac * dval;
         }
      }
      clear_2d(npts, Gs);
   }
   for (i=0; i<2; i++)
   {
      Stif[0][i] = StifR[0][i];
      Stif[1][i] = StifR[1][i];
      Stif[2][i] = StifR[3][i];
      Stif[3][i] = StifR[2][i];
   }

   clear_2d(4, StifR);
}

void IntQTm_ellam(QVector <double> vx, double D, double sw2, double **Stif, double dt)
{
   // element to define basis functions
   //
   int npts, i, k;
   double x_gauss, y_gauss, dval;
   QVector <double> Lx, Ly, Cx, Cy, Rx, Ry, Qx, Qy, Tx, Ty;
   double *phiR, *phiRx, *phiRy;
   double **StifL=NULL, **StifR=NULL, **Lam=NULL, DJac;
   double *phiL, *phiLx, *phiLy, *phiCx, *phiCy, *phiC;
   double **Gs=NULL;
   double phiL [4];
   double phiLx[4];
   double phiLy[4];
   double phiCx[4];
   double phiCy[4];
   double phiC [4];
   double phiR [4];
   double phiRx[4];
   double phiRy[4];
   Lx.clear();
   Ly.clear();
   Cx.clear();
   Cy.clear();
   Rx.clear();
   Ry.clear();
   Qx.clear();
   Qy.clear();
   Tx.clear();
   Ty.clear();

   Lx.push_back(vx[0]);
   Lx.push_back(vx[1]);
   Lx.push_back(vx[4]);
   Lx.push_back(vx[3]);

   Ly.push_back(0.0);
   Ly.push_back(0.0);
   Ly.push_back(dt);
   Ly.push_back(dt);          // vertices of left T

   Cx.push_back(vx[6]);
   Cx.push_back(vx[7]);
   Cx.push_back(vx[5]);
   Cx.push_back(vx[4]);

   Cy.push_back(0.0);
   Cy.push_back(0.0);
   Cy.push_back(dt);
   Cy.push_back(dt);

   Rx.push_back(vx[1]); // vertices of Q on right
   Rx.push_back(vx[2]);
   Rx.push_back(vx[5]);
   Rx.push_back(vx[4]);

   Ry.push_back(0.0);
   Ry.push_back(0.0);
   Ry.push_back(dt);
   Ry.push_back(dt);


   initialize_2d(4, 2, &StifL);
   initialize_2d(4, 2, &StifR);

   //
   // integration over triangle T:
   //
   Tx.push_back(vx[6]); // vertices of T on left
   Tx.push_back(vx[1]);
   Tx.push_back(vx[4]);

   Ty.push_back(0.0);
   Ty.push_back(0.0);
   Ty.push_back(dt);

   npts = 28;
   initialize_2d(npts, 4, &Lam);
   DefineFkp(npts, Lam);

   for (k=0; k<npts; k++)
   {
      x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
      y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
      DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;

      //
      // find phi, phi_x, phi_y on R and C at (x,y)
      //

      BasisQR( Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt );
      BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );

      for (i=0; i<4; i++)
      {
         dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
                                 phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
         StifL[i][0] += Lam[k][3] * DJac * dval;

         dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i],
                                 phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
         StifL[i][1] += Lam[k][3] * DJac * dval;
      }
   }
   clear_2d(npts, Lam);

   //
   // integration over quadrilateral element Q :
   //
   if( (vx[7]-vx[1])/(vx[2]-vx[1]) <1.e-3 )     // Q_{1,7,5,4} is almost degenerated into a triangle
   {
       Qx.push_back(vx[1]);   // vertices of Q on right
       Qx.push_back(vx[5]);
       Qx.push_back(vx[4]);
       Qy.push_back(0.0);
       Qy.push_back(dt);
       Qy.push_back(dt);

       npts = 28;
       initialize_2d(npts, 4, &Lam);
       DefineFkp(npts, Lam);

       for (k=0; k<npts; k++)
       {
          x_gauss = Lam[k][0] * Qx[0] + Lam[k][1] * Qx[1] + Lam[k][2] * Qx[2];
          y_gauss = Lam[k][0] * Qy[0] + Lam[k][1] * Qy[1] + Lam[k][2] * Qy[2];
          DJac = 2.0 * AreaT(&Qx, &Qy);

          //
          // find phi, phi_x, phi_y on R and C at (x,y)
          //

          BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt );
          BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );
          for (i=0; i<4; i++)
          {
             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
                                     phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
             StifR[i][0] += Lam[k][3] * DJac * dval;

             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
                                     phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
             StifR[i][1] += Lam[k][3] * DJac * dval;
          }
       }
       clear_2d(npts, Lam);
   }
   else           // Q is a non-degenerate quadrilateral
   {
       Qx.push_back(vx[1]);   // vertices of Q on right
       Qx.push_back(vx[7]);
       Qx.push_back(vx[5]);
       Qx.push_back(vx[4]);

       Qy.push_back(0.0);
       Qy.push_back(0.0);
       Qy.push_back(dt);
       Qy.push_back(dt);

       npts = 5 * 5;
       initialize_2d(npts, 3, &Gs);
       DefineGaussian(5, Gs);

       double psi[4], psi1[4], psi2[4], jac[4];
       for (k=0; k<npts; k++)
       {
          BasisQS(Gs[k][0], Gs[k][1], psi, psi1, psi2);

          x_gauss = 0.0;
          y_gauss = 0.0;
          for (i=0; i<4; i++)
          {
             jac[i] = 0.0;
          }
          for (i=0; i<4; i++)
          {
             x_gauss += psi[i] * Qx[i];
             y_gauss += psi[i] * Qy[i];
             jac[0] += Qx[i] * psi1[i];
             jac[1] += Qx[i] * psi2[i];
             jac[2] += Qy[i] * psi1[i];
             jac[3] += Qy[i] * psi2[i];
          }

          DJac = jac[0] * jac[3] - jac[1] * jac[2];

          //
          // find phi, phi_x, phi_y on L and C at (x,y)
          //

          BasisQR( Rx, x_gauss, y_gauss, phiR, phiRx, phiRy, dt );
          BasisQR( Cx, x_gauss, y_gauss, phiC, phiCx, phiCy, dt );
          for (i=0; i<4; i++)
          {
             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
                                     phiC[0] + phiC[3], phiCx[0] + phiCx[3]);
             StifR[i][0] += Gs[k][2] * DJac * dval;

             dval = Integrand(x_gauss, D, sw2, phiR[i], phiRx[i], phiRy[i],
                                     phiC[1] + phiC[2], phiCx[1] + phiCx[2]);
             StifR[i][1] += Gs[k][2] * DJac * dval;
          }
       }
       clear_2d(npts, Gs);
   }

   for (i=0; i<2; i++)
   {
      Stif[0][i] = StifL[0][i];
      Stif[1][i] = StifL[1][i] + StifR[0][i];
      Stif[2][i] =               StifR[1][i];
      Stif[3][i] = StifL[3][i];
      Stif[4][i] = StifL[2][i] + StifR[3][i];
      Stif[5][i] =               StifR[2][i];
   }

   clear_2d(4, StifL);
   clear_2d(4, StifR);
}


void IntQTn1_ellam(QVector <double> vx, double D, double sw2, double **Stif, double dt)
{
   // element to define basis functions
   //
   int npts, i, k;
   double x_gauss, y_gauss, dval;
   QVector <double> Lx, Ly, Tx, Ty;
   double **StifL=NULL, **Lam=NULL, DJac;
   double phiL [4];
   double phiLx[4];
   double phiLy[4];
   Lx.clear();
   Ly.clear();
   Tx.clear();
   Ty.clear();

   Lx.push_back(vx[0]);
   Lx.push_back(vx[1]);
   Lx.push_back(vx[3]);
   Lx.push_back(vx[2]);

   Ly.push_back(0.0);
   Ly.push_back(0.0);
   Ly.push_back(dt);
   Ly.push_back(dt);

   initialize_2d(4, 2, &StifL);

   //
   // integration over T:
   //

   Tx.push_back(vx[4]); // triangle for integration
   Tx.push_back(vx[1]);
   Tx.push_back(vx[3]);

   Ty.push_back(0.0);
   Ty.push_back(0.0);
   Ty.push_back(dt);

   npts = 28;
   initialize_2d(npts, 4, &Lam);
   DefineFkp(npts, Lam);

   for (k=0; k<npts; k++)
   {
      x_gauss = Lam[k][0] * Tx[0] + Lam[k][1] * Tx[1] + Lam[k][2] * Tx[2];
      y_gauss = Lam[k][0] * Ty[0] + Lam[k][1] * Ty[1] + Lam[k][2] * Ty[2];
      DJac = 2.0 * AreaT(&Tx, &Ty);

      if(DJac<1.e-22) break;
      //
      // find phi, phi_x, phi_y on R and C at (x,y)
      //

      BasisQR( Lx, x_gauss, y_gauss, phiL, phiLx, phiLy, dt );

      for (i=0; i<4; i++)
      {
         dval = Integrand(x_gauss, D, sw2, phiL[i], phiLx[i], phiLy[i], 1.0, 0.0);
         StifL[i][0] += Lam[k][3] * DJac * dval;
      }
   }
   clear_2d(npts, Lam);


   for (i=0; i<2; i++)
   {
      Stif[0][i] = StifL[0][i];
      Stif[1][i] = StifL[1][i];
      Stif[2][i] = StifL[3][i];
      Stif[3][i] = StifL[2][i];
   }

   clear_2d(4, StifL);
}


void QuadSolver_ellam(double *ai, double *bi, double *ci, double *di, double *cr, double *solu, int N)
{
//
// solve Quad-diagonal system [a_i, b_i, *c_i*, d_i]*[x]=[r_i]
// c_i are on the main diagonal line
//

   int i;
   double tmp;
   QVector<double> ca, cb, cc, cd;
   ca.clear();
   cb.clear();
   cc.clear();
   cd.clear();
   for (i=0; i<N; i++)
   {
      ca.push_back( ai[i] );
      cb.push_back( bi[i] );
      cc.push_back( ci[i] );
      cd.push_back( di[i] );
   }

   for (i=N-2; i>=1; i--)
   {
      tmp = cd[i]/cc[i+1];
      cc[i] = cc[i]-cb[i+1]*tmp;
      cb[i] = cb[i]-ca[i+1]*tmp;
      cr[i] = cr[i]-cr[i+1]*tmp;
   }
   i=0;
   tmp = cd[i]/cc[i+1];
   cc[i] = cc[i]-cb[i+1]*tmp;
   cr[i] = cr[i]-cr[i+1]*tmp;
   solu[0] = cr[0] / cc[0];
   solu[1] = (cr[1] - cb[1] * solu[0]) / cc[1];
   i = 1;
   do
   {
      i++;
      solu[i] = (cr[i] - ca[i] * solu[i-2] - cb[i] * solu[i-1]) / cc[i];
   } while (i != N-1);
}
// ******* end of ELLAM *********************


////////////////////////////////////////////////////////////////////////%
// calculate total mass
// (r,u) concentration defined at r(1), ...., r(M)
//  M: r(1).... r(M): the interval for total mass, (M-1) subintervals
////////////////////////////////////////////////////////////////////////%

double IntConcentration(QVector<double> r, double *u)
{
//function T=IntConcentration(r,M,u)
   double T = 0.0;
   for ( int j=0; j<r.size()-1; j++)
   {
      T += (r[j+1] - r[j]) * ((r[j+1] - r[j]) * (u[j+1]/3.0 + u[j]/6.0)
         + r[j] * (u[j] + u[j+1])/2.0);
   }
   return T;
}



void DefInitCond(double **C0, int N)
{
   int j;
   for(j=0; j<N; j++)
   {
      C0[0][j] = 0.3;
      C0[1][j] = 0.7;
   }
}

int interpolate( MfemData *simdata, int scans, int points,
                 float *scantimes, double *radius, double **c )
{

/******************************************************************************
 * Interpolation:                                                             *
 *                                                                            *
 * First, we need to interpolate the time. Create a new array with the same   *
 * time dimensions as the raw data and the same radius dimensions as the      *
 * simulated data. Then find the time steps from the simulated data that      *
 * bracket the experimental data from the left and right. Then make a linear  *
 * interpolation for the concentration values at each radius point from the   *
 * simulated data. Then interpolate the radius points by linear interpolation.*
 *                                                                            *
 ******************************************************************************/

   int i, j;
   double slope, intercept;
   double **ip_array;
   ip_array =  new double* [scans];
   for (i=0; i<scans; i++)
   {
      ip_array[i] = new double [(*simdata).radius.size()];
   }
   int count = 0; // counting the number of time steps of the raw data
   for (i=0; i<scans; i++)
   {
      while (count < (*simdata).scan.size()-1
            && scantimes[i] >= (*simdata).scan[count].time)
      {
         count++;
      }
      if (scantimes[i] == (*simdata).scan[count].time)
      {
         for (j=0; j<(*simdata).radius.size(); j++)
         {
            ip_array[i][j] = (*simdata).scan[count].conc[j];
         }
      }
      else  // else, perform a linear time interpolation:
      {
         for (j=0; j<(*simdata).radius.size(); j++)
         {
            slope = ((*simdata).scan[count].conc[j] - (*simdata).scan[count-1].conc[j])
                     /((*simdata).scan[count].time - (*simdata).scan[count-1].time);
            intercept = (*simdata).scan[count].conc[j] - slope * (*simdata).scan[count].time;
            ip_array[i][j] = slope * scantimes[i] + intercept;
         }
      }
   }
   //interpolate radius then add to concentration vector
   for (i=0; i<scans; i++)
   {
      c[i][0] += ip_array[i][0]; // meniscus position is identical for all scans
   }
   // all other points may need interpolation:
   for (i=0; i<scans; i++)
   {
      count = 1;
      for (j=1; j<(int) points; j++)
      {
         while (radius[j] > (*simdata).radius[count] && count < (*simdata).radius.size()-1)
         {
            count++;
         }
         if (radius[j] == (*simdata).radius[count])
         {
            c[i][j] += ip_array[i][count];
         }
         else
         {
            slope = (ip_array[i][count] - ip_array[i][count-1])/((*simdata).radius[count] - (*simdata).radius[count-1]);
            intercept = ip_array[i][count] - (*simdata).radius[count] * slope;
            c[i][j] += slope * radius[j] + intercept;
         }
      }
   }
   for (i=0; i<scans; i++)
   {
      delete [] ip_array[i];
   }
   delete [] ip_array;
   return 0;
}


void interpolate_Cfinal( MfemInitial *C0, double *cfinal, QVector <double> *x )
{
// This routine also considers cases where C0 starts before the meniscus or
// stops after the bottom is reached. However, C0 needs to cover both meniscus
// and bottom. In those cases it will fill the C0 vector with the first and/or
// last value of C0, respectively, for the missing points.

   int    i;
   int    j;
   int    ja;
   double a;
   double b;
   double xs;
   double tmp;

   ja = 0;
   for(j=0; j<(*C0).radius.size(); j++)
   {
      xs = (*C0).radius[j];
      for (i=ja; i<(*x).size(); i++)
      {
         if( (*x)[i] > xs + 1.e-12)
         {
            break;
         }
      }

      if ( i == 0 )           // xs < x[0]
      {
         (*C0).concentration[j] = cfinal[0];    // use the first value
      }
      else if ( i == (*x).size() )     // xs > x[N]
      {
         (*C0).concentration[j] = cfinal[ i-1 ];
      }
      else                    // x[i-1] < xs <x[i]
      {
         a = (*x)[i-1];
         b = (*x)[i];
         tmp = (xs-a)/(b-a);
         (*C0).concentration[j] = cfinal[i-1] * (1. - tmp) + cfinal[i] * tmp;
         ja = i-1;
      }
   }
}
#endif
