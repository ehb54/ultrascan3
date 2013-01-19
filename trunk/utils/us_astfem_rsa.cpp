//! \file us_astfem_rsa.cpp

#include "us_astfem_rsa.h"
#include "us_astfem_math.h"
#include "us_hardware.h"
#include "us_math2.h"
#include "us_stiffbase.h"
#include "us_settings.h"
#include "us_sleep.h"

US_Astfem_RSA::US_Astfem_RSA( US_Model&                model,
                              US_SimulationParameters& params,
                              QObject*                 parent )
   : QObject( parent ), system( model ), simparams( params )
{
   stopFlag        = false;
   use_time        = false;
   time_correction = true;
   simout_flag     = false;
   show_movie      = false;

   dbg_level       = US_Settings::us_debug();
}

int US_Astfem_RSA::calculate( US_DataIO2::RawData& exp_data )
{
   US_AstfemMath::MfemInitial* vC0 = NULL; // Initial concentration for
                                           //  multiple components
   US_AstfemMath::MfemInitial  CT0;        // Initial total concentration
   US_AstfemMath::MfemData     simdata;
   double        current_time   = 0.0;
   double        current_om2t   = 0.0;
   double        current_speed  = 0.0;
   double        duration       = 0.0;
   double        delay          = 0.0;

   int           initial_npts = 1000;
   int           current_assoc = 0;
   int           size_cv         = system.components.size();
   QVector< bool >                 reactVec( size_cv );
   bool*         reacting        = reactVec.data();

   double        accel_time;
   double        dr;
#if 0
#define TIMING_RA_INC 500
#define TIMING_RA
#endif
#ifdef TIMING_RA
QDateTime calcStart = QDateTime::currentDateTime();
static int ncalls=0;
static int totTC=0;
static int totT1=0;
static int totT2=0;
static int totT3=0;
static int totT4=0;
static int totT5=0;
static int totT6=0;
static int totT7=0;
static int totT8=0;
#endif

   af_params.first_speed = simparams.speed_step[ 0 ].rotorspeed;
   af_params.simpoints   = simparams.simpoints;

   load_mfem_data( exp_data, af_data );

   int npts      = af_data.scan[ 0 ].conc.size();
   initial_npts  = ( initial_npts < npts ) ? initial_npts : npts;
DbgLv(2) << "RSA: rotorspeed" << af_params.first_speed;
DbgLv(2) << "RSA: simpoints" << af_params.simpoints;
DbgLv(2) << "RSA:  scan0size" << npts;
DbgLv(2) << "RSA:  af_c0size" << initial_npts;
   update_assocv();
   initialize_rg();  // Reaction group
   adjust_limits( af_params.first_speed );
DbgLv(2) << "RSA:   sbottom acbottom" << simparams.bottom
 << af_params.current_bottom;
   af_data.meniscus = simparams.meniscus;
   af_data.bottom   = simparams.bottom;

   for ( int k = 0; k < size_cv; k++ )
   {
#ifdef TIMING_RA
QDateTime clcSt1 = QDateTime::currentDateTime();
#endif
      US_Model::SimulationComponent* sc = &system.components[ k ];
      US_Model::Association*         as;
      reacting[ k ] = false;
DbgLv(2) << "RSA:  k assoc.size" << k << system.associations.size();

      for ( int j = 0; j <  system.associations.size(); j++ )
      {
         as  = &system.associations[ j ];
DbgLv(2) << "RSA:   j react.size" << k << as->rcomps.size();

         for ( int n = 0; n < as->rcomps.size(); n++ )
         {
            if ( k == (int)as->rcomps[ n ] )
            {
                reacting[ k ] = true;
                current_assoc = j;
                break;   // Since a comp appears at most once in an assoc rule
            }
         }
DbgLv(2) << "RSA:  k j current_assoc" << k << j << current_assoc;
      }

      current_time  = 0.0;
      current_om2t  = 0.0;
      last_time     = 0.0;
      current_speed = 0.0;
      w2t_integral  = 0.0;

      CT0.radius       .clear();
      CT0.concentration.clear();
      CT0.radius       .reserve( initial_npts );
      CT0.concentration.reserve( initial_npts );

      dr = ( af_params.current_bottom - af_params.current_meniscus ) /
           ( initial_npts - 1 );

      for ( int j = 0; j < initial_npts; j++ )
      {
         CT0.radius.append( af_params.current_meniscus + j * dr );
         CT0.concentration.append( 0.0 );
         //CT0.concentration.append( sc->signal_concentration );
      }

      af_c0.radius       .clear();
      af_c0.concentration.clear();

      // Once time invariant noise has been removed in a band experiment, we
      // can use the first scan of the first speed step of the experiment as
      // the initial concentration of the simulation. The approach will copy
      // the 1st scan concentration vector into each component's c0 vector.
      // NNLS will scale the appropriate concentration for each component. The
      // assumption is made that any potential differentiation of components in
      // the initial scan is minimal compared to any solute flow disturbances
      // at the meniscus. For this approach to work well it is necessary to
      // pick the first data point close to the meniscus and to include the
      // earliest possible scan in the experiment. Also, time invariant noise
      // should be subtracted first.

      if ( simparams.firstScanIsConcentration )
      {
         US_AstfemMath::MfemScan*   scan0 = &af_data.scan[ 0 ];
         US_AstfemMath::MfemInitial scan1;
         scan1.radius       .clear();
         scan1.concentration.clear();
         scan1.radius       .reserve( af_data.radius.size() );
         scan1.concentration.reserve( af_data.radius.size() );

         for ( int j = 0; j < af_data.radius.size(); j++ )
         {
            scan1.radius       .append( af_data.radius[ j ] );
            scan1.concentration.append( scan0->conc   [ j ] );
         }

         US_AstfemMath::interpolate_C0( scan1, af_c0 );
      }

      if ( ! reacting[ k ] ) // noninteracting
      {
         initialize_conc( k, CT0, true );
int nn=CT0.concentration.size()-1;
int mm=nn/2;
DbgLv(1) << "RSA: s0 i_conc k step lsc" << k << 0 << 0 << ": c0 cm cn"
   << CT0.concentration[0] << CT0.concentration[mm] << CT0.concentration[nn];

         af_params.s   .resize( 1 );
         af_params.D   .resize( 1 );
         af_params.kext.resize( 1 );

         af_params.s   [ 0 ] = sc->s;
         af_params.D   [ 0 ] = sc->D;
         af_params.kext[ 0 ] = sc->extinction;
#ifdef TIMING_RA
totT1+=(clcSt1.msecsTo(QDateTime::currentDateTime()));
#endif
         int    lscan = 0;
         int    fscan = 0;
         double time0 = 0.0;
         double time1 = 0.0;
         double time2 = 0.0;
         double omeg0 = 0.0;
         double omeg1 = 0.0;
         double omeg2 = 0.0;
         US_SimulationParameters::SpeedProfile* sp;
         US_AstfemMath::MfemData*               ed;

         for ( int step = 0; step < simparams.speed_step.size(); step++ )
         {
#ifdef TIMING_RA
QDateTime clcSt2 = QDateTime::currentDateTime();
#endif
            sp      = &simparams.speed_step[ step ];
            ed      = &af_data;

            adjust_limits( sp->rotorspeed );

            fscan   = lscan;
            lscan   = fscan + sp->scans;
            time0   = time2;
            time1   = ed->scan[ fscan     ].time;
            time2   = ed->scan[ lscan - 1 ].time;
            omeg0   = omeg2;
            omeg1   = ed->scan[ fscan     ].omega_s_t;
            omeg2   = ed->scan[ lscan - 1 ].omega_s_t;

            ed->meniscus = af_params.current_meniscus;
            ed->bottom   = af_params.current_bottom;
            accel_time   = 0.0;

            // We need to simulate acceleration
            if ( sp->acceleration_flag )
            {
               // If the speed difference is larger than acceleration rate then
               // we have at least 1 acceleration step

               af_params.time_steps = (int)
                  fabs( sp->rotorspeed - current_speed ) / sp->acceleration;

               // Each simulation step is 1 second in the acceleration phase
               // Use a fixed grid with refinement at both ends and with
               //   twice the number of points
               af_params.dt         = 1.0;
               af_params.simpoints  = 2 * simparams.simpoints;
               accel_time           = af_params.dt * af_params.time_steps;
               delay        = time1 - time0;

               // Do calculations to find the position of the acceleration
               //  zone within the gap between speed steps.
               //
               //  time0    t2         t3        time1
               //  | sp1    | sp1->sp2 |   sp2   |
               //  | dt1    | dt2      |   dt3   |
               //  -------------------------------
               //  | dw1    | dw2      |   dw3   |
               //  omeg0    w2         w3        omeg1
               //
               // The code below is meant to determine "dt1", the time
               // difference between the end of the previous step and the
               // beginning of the acceleration zone. This is done by
               // solving the simultaneous equations:
               //   dt1 * ddw1 + dt3 * ddw3 = omeg1 - omeg0 - dw2
               //   dt1 + dt3 = time1 - time0 - dt2
               // which leads to:
               //
               //   dt1 = ( omeg1 - omeg0 - dw2 - drtm * ddw3 )
               //         / ( ddw1 - ddw3 )
               //
               // for ddw1, ddw3 being changes in omega per second in 2 zones
               // and drtm = time1 - time0 - dt2

               double drtm  = delay - accel_time;
               double wfac  = af_params.dt * sq( M_PI / 30.0 );
DbgLv(1) << "RSA:PO: time0 time1 omeg0 omeg1"
 << time0 << time1 << omeg0 << omeg1 << "delay atime" << delay << accel_time;
               double dw2   = 0.0;
               double dt2   = accel_time / af_params.dt;
               double ddw1  = wfac * sq( current_speed );
               double ddw3  = wfac * sq( sp->rotorspeed );
               int    ndt2  = (int)dt2;
               double crpm  = current_speed;
               double rpmi  = ( sp->rotorspeed - current_speed ) / dt2;
               for ( int kk = 0; kk < ndt2; kk++ )
               { // Accumulate omega^2t over acceleration zone
                  crpm += rpmi;
                  dw2  += ( wfac * sq( crpm ) );
               }
               double dw1   = 0.0;
               double dt1   = 0.0;
               if ( current_speed == 0.0 )
               { // For the first (only?) speed step
                  double dt3   = ( omeg1 - dw2 ) / ddw3;
                  dt1          = (double)qFloor( time1 - dt3 - accel_time );
               }
               else
               { // For speed steps beyond the first
                  dt1          = (double)qFloor(
                                   ( omeg1 - omeg0 - dw2 - drtm * ddw3 )
                                   / ( ddw1 - ddw3 ) );
                  dw1          = ddw1 * dt1;
               }
DbgLv(1) << "RSA:PO:   ddw1 ddw3 dt2 dw2" << ddw1 << ddw3 << dt2 << dw2
 << "dt1 dw1" << dt1 << dw1;
               af_params.start_time = time0 + dt1;
               af_params.start_om2t = omeg0 + dw1;
               int    nradi   = simdata.radius.size();
DbgLv(1) << "RSA:PO:     st_ time om2t"
 << af_params.start_time << af_params.start_om2t;

               // If beyond the 1st speed step and acceleration begins
               //  a little after the end of the previous step, calculate
               //  the simulation for constant speed leading up to the
               //  acceleration zone
               if ( step > 0  &&  dt1 > 2.0 )
               {
                  int svnpts           = af_params.time_steps;
                  af_params.start_time = time0;
                  af_params.start_om2t = omeg0;
                  af_params.time_steps = (int)dt1 + 1;

                  calculate_ni( current_speed, current_speed,
                                CT0, simdata, false );

                  af_params.start_time = time0 + dt1;
                  af_params.start_om2t = omeg0 + dw1;
                  af_params.time_steps = svnpts;
DbgLv(1) << "RSA:PO:    Accel nradi" << nradi << "CT0size" << CT0.radius.size();
               }

               // Calculate the simulation for the acceleration zone

               calculate_ni( current_speed, sp->rotorspeed,
                             CT0, simdata, true );

               if ( stopFlag ) return 1;

               // Add the acceleration time:
               current_time  = af_params.start_time + accel_time;
               current_om2t  = af_params.start_om2t + dw2;

#ifndef NO_DB
               emit new_time( current_time );
               qApp->processEvents();
#endif
            }  // End of acceleration

            else
            {  // No acceleration
               current_time  = time1;
               current_om2t  = omeg1;
            }
#ifdef TIMING_RA
QDateTime clcSt3 = QDateTime::currentDateTime();
totT2+=(clcSt2.msecsTo(clcSt3));
#endif

            duration   = time2 - current_time;

            if ( step == simparams.speed_step.size() - 1 )
               duration += (double)( (int)( duration * 0.05 ) );  // +5%

            if ( accel_time > duration )
            {
               qDebug() << "Attention: acceleration time exceeds duration - "
                           "please check initialization\n";
               return -1;
            }

            double omega = sp->rotorspeed * M_PI / 30;
            af_params.omega_s = sq( omega );

            double lg_bm_rat = log(
               af_params.current_bottom / af_params.current_meniscus );
            double s_omg_fac = fabs( sc->s ) * af_params.omega_s;
            af_params.dt     = lg_bm_rat 
               / ( s_omg_fac * ( simparams.simpoints - 1 ) );

            if ( af_params.dt > duration)
            {
               af_params.dt        = duration;
               af_params.simpoints = 1 +
                  (int)( lg_bm_rat / ( s_omg_fac * af_params.dt ) );
            }

            if ( af_params.simpoints > 10000 ) af_params.simpoints = 10000;

            // Find out the minimum number of simpoints needed to provide
            // the necessary dt:
            af_params.time_steps = (int)( duration / af_params.dt ) + 1;
            af_params.start_time = current_time;
            af_params.start_om2t = current_om2t;
#ifdef TIMING_RA
QDateTime clcSt4 = QDateTime::currentDateTime();
totT3+=(clcSt3.msecsTo(clcSt4));
#endif
            current_time  = time1;

            // Calculate the simulation for the bulk of the speed step

            calculate_ni( sp->rotorspeed, sp->rotorspeed,
                          CT0, simdata, false );

            if ( stopFlag ) return 1;

            // Set the current time to the last scan of this speed step
            duration      = time2 - time0;
            current_time  = time2;
            current_om2t  = omeg2;
DbgLv(1) << "RSA:    step rpm" << step << sp->rotorspeed
 << "start_ current_time" << af_params.start_time << current_time;
int mmm=simdata.scan.size()-1;
DbgLv(1) << "RSA:     eomg1 eomg2" << ed->scan[fscan].omega_s_t
 << ed->scan[lscan-1].omega_s_t << " somg1 somg2"
 << simdata.scan[0].omega_s_t << simdata.scan[mmm].omega_s_t;
#ifdef TIMING_RA
QDateTime clcSt5 = QDateTime::currentDateTime();
totT4+=(clcSt4.msecsTo(clcSt5));
#endif

            // Interpolate the simulated data onto the experimental time and
            // radius grid
            US_AstfemMath::interpolate( *ed, simdata, use_time, fscan, lscan );

            // Set the current speed to the constant rotor speed of the
            // current speed step
            current_speed = sp->rotorspeed;

#ifndef NO_DB
            qApp->processEvents();
#endif
#ifdef TIMING_RA
totT5+=(clcSt5.msecsTo(QDateTime::currentDateTime()));
#endif
DbgLv(1) << "RSA: step=" << step << "tsteps sttime" << af_params.time_steps
 << current_time << "bottoms" << simparams.bottom_position
 << simparams.bottom << af_params.current_bottom << "sp" << current_speed;

         } // Speed step loop

#ifndef NO_DB
         emit current_component( k + 1 );
         qApp->processEvents();
#endif
      } // Non-interacting case
   } // Component loop
#ifdef TIMING_RA
QDateTime clcSt6 = QDateTime::currentDateTime();
#endif

   // Resize af_params.local_index
   af_params.local_index.resize( size_cv );

   US_AstfemMath::ComponentRole cr;

   for ( int group = 0; group < rg.size(); group++ )
   {
      int num_comp = rg[ group ].GroupComponent.size();
      int num_rule = rg[ group ].association.size();
      af_params.rg_index = group;
DbgLv(2) << "RSA:  group nrule ncomp" << group << num_rule << num_comp;
      af_params.s          .resize( num_comp );
      af_params.D          .resize( num_comp );
      af_params.kext       .resize( num_comp );
      af_params.role       .resize( num_comp );
      af_params.association.resize( num_rule );

      for ( int m = 0; m < num_rule; m++ )
      {
         af_params.association[ m ] =
               system.associations[ rg[ group ].association[ m ] ];
      }

      for ( int j = 0; j < num_comp; j++ )
      {
         int index = rg[ group ].GroupComponent[ j ];
DbgLv(2) << "RSA:    j index" << j << index;

         US_Model::SimulationComponent* sc = &system.components[ index ];
         af_params.s   [ j ] = sc->s;
         af_params.D   [ j ] = sc->D;
         af_params.kext[ j ] = sc->extinction;

         // Global to local index
         af_params.local_index[ index ] = j;

         af_params.role[ j ] =  cr;  // Add j'th rule

         af_params.role[ j ].comp_index = index;
         af_params.role[ j ].rcomps .clear();
         af_params.role[ j ].stoichs.clear();

         // Check all assoc rule in this rg
         for ( int m = 0; m < rg[ group ].association.size(); m++ )
         {
            // Check all comp in rule
            int rule   = rg[ group ].association[ m ];
DbgLv(2) << "RSA:     m rule" << m << rule;
            US_Model::Association* as   = &system.associations[ rule ];

            for ( int n = 0; n < as->rcomps.size(); n++ )
            {
DbgLv(2) << "RSA:      n af-index as-react" << n
 << af_params.role[j].comp_index << as->rcomps[n];
               if ( af_params.role[ j ].comp_index ==
                    as->rcomps[ n ] )
               {
                  // local index for the rule
                  af_params.role[ j ].rcomps .append( m );
                  af_params.role[ j ].stoichs.append( as->stoichs[ n ] );
                  break;
               }
            }
         }
      }

      for ( int m = 0; m < num_rule; m++ )
      {
         US_Model::Association* as = &af_params.association[ m ];
          for ( int n = 0; n < as->rcomps.size(); n++ )
          {
            as->rcomps[ n ] =
               af_params.local_index[ as->rcomps[ n ] ];
DbgLv(2) << "RSA:     m n rcn" << m << n << as->rcomps[n];
          }
      }

      current_time  = 0.0;
      current_speed = 0.0;
      w2t_integral  = af_params.start_om2t;
      last_time     = af_params.start_time;

      dr = ( af_params.current_bottom - af_params.current_meniscus ) /
           ( initial_npts - 1 );

      QVector< US_AstfemMath::MfemInitial > vC0Vec( num_comp );
      vC0 = vC0Vec.data();

      for ( int j = 0; j < num_comp; j++ )
      {
         CT0.radius       .clear();
         CT0.concentration.clear();
         CT0.radius       .reserve( initial_npts );
         CT0.concentration.reserve( initial_npts );
DbgLv(2) << "RSA:      j in_npts" << j << initial_npts;

         for ( int i = 0; i < initial_npts; i++ )
         {
            CT0.radius       .append( af_params.current_meniscus + i * dr );
            CT0.concentration.append( 0.0 );
         }

         initialize_conc( rg[ group ].GroupComponent[ j ], CT0, false );
         vC0[ j ] = CT0;
      }

      decompose( vC0 );
DbgLv(2) << "RSA: decompose OUT";
#ifdef TIMING_RA
QDateTime clcSt5 = QDateTime::currentDateTime();
#endif
      int    lscan = 0;
      int    fscan = 0;
      double time0 = 0.0;
      double time1 = 0.0;
      double time2 = 0.0;
      US_SimulationParameters::SpeedProfile* sp;
      US_AstfemMath::MfemData*               ed;

      for ( int ss = 0; ss < simparams.speed_step.size(); ss++ )
      {
         sp           = &simparams.speed_step[ ss ];
         ed           = &af_data;

         adjust_limits( sp->rotorspeed );
         ed->meniscus = af_params.current_meniscus;
         ed->bottom   = af_params.current_bottom;
         accel_time   = 0.0;
         fscan        = lscan;
         lscan        = fscan + sp->scans;
         time0        = time2;
         time1        = ed->scan[ fscan     ].time;
         time2        = ed->scan[ lscan - 1 ].time;

         // We need to simulate acceleration
         if ( sp->acceleration_flag )
         {
            // If the speed difference is larger than acceleration
            // rate then we have at least 1 acceleration step

            af_params.time_steps = (int)
               ( fabs( sp->rotorspeed - current_speed ) / sp->acceleration );

            // Each simulation step is 1 second long in the acceleration phase
            af_params.dt        = 1.0;
            af_params.simpoints = 2 * simparams.simpoints;

            // Use a fixed grid with refinement at both ends and with twice
            // the number of points
            duration      = time2 - time0;
            current_time  = time0;
            af_params.start_time = current_time;

            calculate_ra2( current_speed, (double) sp->rotorspeed,
                  vC0, simdata, true );

            // Add the acceleration time:
            accel_time    = af_params.dt * af_params.time_steps;
            current_time += accel_time;

#ifndef NO_DB
            emit new_time( current_time );
            qApp->processEvents();
#endif

            if ( stopFlag ) return 1;
         }  // End of for acceleration

         duration      = time2 - time0;

         if ( ss == simparams.speed_step.size() - 1 )
            duration += (double)( (int)( duration * 0.05 ) );  // +5%

         if ( accel_time > duration )
         {
            qDebug() << "Attention: acceleration time exceeds duration - "
                        "please check initialization\n";
            return -1;
         }
         else
         {
            duration -= accel_time;
         }
         double s_max = fabs( af_params.s[ 0 ] );     // Find the largest s

         for ( int m = 1; m < af_params.s.size(); m++ )
             if ( s_max < fabs( af_params.s[m] ) )
                s_max = fabs( af_params.s[m] );

         af_params.omega_s = sq( sp->rotorspeed * M_PI / 30 );

         double lg_bm_rat = log( af_params.current_bottom
                                 / af_params.current_meniscus );
         double s_omg_fac = af_params.omega_s * s_max;
         af_params.dt     = lg_bm_rat
                            / ( s_omg_fac * ( simparams.simpoints - 1 ) );

         if (af_params.dt > duration )
         {
            af_params.dt        = duration;
            af_params.simpoints = 1 +
               (int)( lg_bm_rat / ( s_omg_fac * af_params.dt ) );
         }

         if ( af_params.simpoints > 10000 ) af_params.simpoints = 10000;

         // Find out the minimum number of simpoints needed to provide the
         // necessary dt:
         af_params.time_steps = (int)( duration / af_params.dt ) + 1;
         af_params.start_time = current_time;

DbgLv(2) << "RSA:   tsteps sttime" << af_params.time_steps << current_time;
         calculate_ra2( (double) sp->rotorspeed, (double) sp->rotorspeed,
               vC0, simdata, false );

         // Set the current time to the last scan of this speed step
         duration      = sp->duration_hours * 3600.
                       + sp->duration_minutes * 60.;
         delay         = sp->delay_hours * 3600.0
                       + sp->delay_minutes * 60.0;
         current_time  = time1;
DbgLv(2) << "RSA:    current_time" << current_time << "fscan lscan"
 << fscan << lscan;

         // Interpolate the simulated data onto the experimental
         // time and radius grid
         US_AstfemMath::interpolate( *ed, simdata, use_time, fscan, lscan );

         // Set the current speed to the constant rotor speed of the
         // current speed step
         current_speed = sp->rotorspeed;

#ifndef NO_DB
         qApp->processEvents();
#endif

         if ( stopFlag ) return 1;
      } // Speed step loop
#ifdef TIMING_RA
totT5+=(clcSt5.msecsTo(QDateTime::currentDateTime()));
#endif
   } // RG Group loop
DbgLv(2) << "RSA: Speed step OUT";
#ifdef TIMING_RA
QDateTime clcSt7 = QDateTime::currentDateTime();
totT6+=(clcSt6.msecsTo(clcSt7));
#endif

#ifndef NO_DB
   //emit current_component( -1 );
   qApp->processEvents();
#endif

   if ( time_correction  &&  !simout_flag )
   {
      int soff   = 0;
      int nsstep = simparams.speed_step.size();
      double correction = 0.0;

      // Check each speed step to see if it contains acceleration
      for ( int ss = 0; ss < nsstep; ss++ )
      {
         US_SimulationParameters::SpeedProfile* sp = &simparams.speed_step[ss];
         US_AstfemMath::MfemData*               ed = &af_data;
         int nscans = sp->scans;

         // We need to correct time
         if ( sp->acceleration_flag )
         {
            double slope;
            double intercept;
            double correlation;
            double sigma;

            if ( nscans > 1 )
            {
               QVector< double > xtmpVec( nscans );
               QVector< double > ytmpVec( nscans );
               double* xtmp = xtmpVec.data();
               double* ytmp = ytmpVec.data();

               // Only fit the scans that belong to this speed step
               for ( int i = 0; i < nscans; i++ )
               {
                  xtmp[ i ] = ed->scan[ i + soff ].time;
                  ytmp[ i ] = ed->scan[ i + soff ].omega_s_t;
               }

               US_Math2::linefit( &xtmp, &ytmp, &slope, &intercept, &sigma,
                                  &correlation, nscans );

               correction = -intercept / slope;
            }

            for ( int i = 0; i < nscans; i++ )
               ed->scan[ i + soff ].time -= correction;
         }
         soff += nscans;
      }
   }
DbgLv(2) << "RSA: Time Corr OUT";
#ifdef TIMING_RA
QDateTime clcSt8 = QDateTime::currentDateTime();
totT7+=(clcSt7.msecsTo(clcSt8));
#endif

   if ( !simout_flag )
      store_mfem_data( exp_data, af_data );    // normal experiment grid
   else
      store_mfem_data( exp_data, simdata );    // raw simulation grid

#ifdef TIMING_RA
QDateTime clcSt9 = QDateTime::currentDateTime();
totT8+=(clcSt8.msecsTo(clcSt9));
int elapsedCalc = calcStart.msecsTo( clcSt9 );
ncalls++;
totTC+=elapsedCalc;
if((ncalls%TIMING_RA_INC)<1) {
 DbgLv(0) << "  Elapsed fem-calc ms" << elapsedCalc << "nc totC" << ncalls << totTC << "  size_cv" << size_cv;
 DbgLv(0) << "   t1 t2 t3 t4 t5 t6 t7 t8"
  << totT1 << totT2 << totT3 << totT4 << totT5 << totT6 << totT7 << totT8;
}
#endif
   return 0;
}


void US_Astfem_RSA::update_assocv( void )
{
   for ( int ii = 0; ii < system.associations.size(); ii++ )
   {
      US_Model::Association* as      = &system.associations[ ii ];
      int ncomp   = as->rcomps.size();
      int stoich1 = as->stoichs[ 0 ];
      int stoich2 = as->stoichs[ 1 ];
DbgLv(2) << "AFRSA:  ncomp st1 st2" << ncomp << stoich1 << stoich2;

      if ( ncomp == 2 )
      {
         as->stoichs[ 1 ] = stoich2 < 0 ? stoich2 : -stoich2;
         as->stoichs[ 0 ] = stoich1 > 0 ? stoich1 : -stoich1;
         //as->rcomps[  0 ] = 0;
         //as->rcomps[  1 ] = 1;
      }

      else if ( ncomp == 3 )
      {
         int stoich3      = as->stoichs[ 2 ];
         as->stoichs[ 0 ] = stoich1 > 0 ? stoich1 : -stoich1;
         as->stoichs[ 1 ] = stoich2 > 0 ? stoich2 : -stoich2;
         as->stoichs[ 2 ] = stoich3 < 0 ? stoich3 : -stoich3;
         //as->rcomps[ 0 ] = 1;
         //as->rcomps[ 1 ] = 1;
         //as->rcomps[ 2 ] = 0;
      }
   }
}

// Adjust meniscus and bottom based on rotor coefficients
void US_Astfem_RSA::adjust_limits( int speed )
{
   // First correct meniscus to theoretical position at rest:
   double stretch_value        = stretch( simparams.rotorcoeffs,
                                          af_params.first_speed );

   // This is the meniscus at rest
   af_params.current_meniscus  = simparams.meniscus - stretch_value;

   // Calculate rotor stretch at current speed
   stretch_value = stretch( simparams.rotorcoeffs, speed );

   // Add current stretch to meniscus at rest
   af_params.current_meniscus += stretch_value;

   // Add current stretch to bottom at rest
   af_params.current_bottom    = simparams.bottom + stretch_value;
}

// Calculate stretch for rotor coefficients array and rpm
double US_Astfem_RSA::stretch( double* rotorcoeffs, int rpm )
{
   double speed    = (double)rpm;
   return ( rotorcoeffs[ 0 ] * speed
          + rotorcoeffs[ 1 ] * sq( speed ) );
}

// Setup reaction groups
void US_Astfem_RSA::initialize_rg( void )
{
   rg.clear();

   // If there are no reactions, then it is all noninteracting
   if ( system.associations.size() == 0 ) return;
   QVector< bool > reaction_used;
   reaction_used.clear();

   for ( int i = 0; i < system.associations.size(); i++ )
      reaction_used.append( false );

   // Initialize the first reaction group and put it into a temporary reaction
   // group, use as test against all assoc vector entries:

   US_Model::Association*       as     = &system.associations[ 0 ];
   US_AstfemMath::ReactionGroup tmp_rg;

   tmp_rg.GroupComponent.clear();
   tmp_rg.association.clear();

   tmp_rg.association   .append( 0 );
   tmp_rg.GroupComponent.append( as->rcomps[ 0 ] );
   tmp_rg.GroupComponent.append( as->rcomps[ 1 ] );

   // Only 2 components react in first reaction
   if ( as->rcomps.size() > 2 )
      tmp_rg.GroupComponent .append( as->rcomps[ 2 ] );

   reaction_used[ 0 ] = true;

   // There is only one reaction, so add it and return
   if ( system.associations.size() == 1 )
   {
      rg .append( tmp_rg );
      return;
   }

   bool flag3 = false;

   for ( int i = 0; i < system.associations.size(); i++ )
   {
      int ncomp;
      int component1;
      int component2;
      int component3;

      // Check each association rule to see if it contains components that
      // match tmp_rg components

      for ( int counter = 1; counter < system.associations.size(); counter++ )
      {
         US_Model::Association* av    = &system.associations[ counter ];
         ncomp      = av->rcomps.size();
         component1 = ( ncomp > 0 ) ? av->rcomps[ 0 ] : 0;
         component2 = ( ncomp > 1 ) ? av->rcomps[ 1 ] : 0;
         component3 = ( ncomp > 2 ) ? av->rcomps[ 2 ] : 0;

         while ( reaction_used[ counter ] )
         {
            counter++;
            if ( counter == system.associations.size() ) return;
         }

         // Check if any component already present in tmp_rg matches any of the
         // three components in current (*system).associations entry

         bool flag1 = false;

         for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ )
         {
            flag1 = false;

            if ( component1 == (int) tmp_rg.GroupComponent[ j ]  ||
                 component2 == (int) tmp_rg.GroupComponent[ j ]  ||
                 component3 == (int) tmp_rg.GroupComponent[ j ] )
            {
               flag1 = true;
               break;
            }
         }

         // If the component from tmp_rg is present in another
         // system.associations entry, find out if the component from
         // system.associations is already in tmp_rg.GroupComponent

         if ( flag1 )
         {
            // It is present (flag1=true) so add this rule to the tmp_rg vector
            tmp_rg.association .append( counter );
            reaction_used[ counter ] = true;

            // There is at least one of all system.associations entries in
            // this reaction_group, so set flag3 to true

            flag3 = true;
            bool flag2 = false;

            // Check if 1st component is already in the GroupVector from tmp_rg
            for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ )
            {
               if ( component1 == (int) tmp_rg.GroupComponent[ j ])
                  flag2 = true;
            }

            // Add if not present already
            if ( ! flag2 ) tmp_rg.GroupComponent.append( component1 );

            flag2 = false;

            // Check if 2nd component is already in the GroupVector from tmp_rg
            for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ )
            {
               if ( component2 == (int) tmp_rg.GroupComponent[ j ])
                  flag2 = true;
            }

            // Add if not present already
            if ( ! flag2 )  tmp_rg.GroupComponent .append( component2 );

            flag2 = false;

            // Check if 3rd component is already in the GroupVector from tmp_rg
            // (but only if non-zero)

            if ( ncomp > 2 )
            {
               for ( int j = 0; j < tmp_rg.GroupComponent.size(); j++ )
               {
                  if ( component3 == (int) tmp_rg.GroupComponent[ j ] )
                     flag2 = true;
               }

               // Add if not present already
               if ( ! flag2 ) tmp_rg.GroupComponent .append( component3 );
            }
         }
      }

      if ( flag3 )
      {
         flag3 = false;
         rg .append( tmp_rg );

         tmp_rg.GroupComponent.clear();
         tmp_rg.association.clear();

         // Make the next unused reaction the test reaction
         int j = 1;

         while ( reaction_used[ j ] )
         {
            j++;
            if ( j >= reaction_used.size() ) return;
         }

         US_Model::Association* avj = &system.associations[ j ];
         ncomp      = avj->rcomps.size();
         component1 = ( ncomp > 0 ) ? avj->rcomps[ 0 ] : 0;
         component2 = ( ncomp > 1 ) ? avj->rcomps[ 1 ] : 0;
         component3 = ( ncomp > 2 ) ? avj->rcomps[ 2 ] : 0;

         if ( j < system.associations.size() )
         {
            tmp_rg.association   .append( j );
            tmp_rg.GroupComponent.append( component1 );
            tmp_rg.GroupComponent.append( component2 );

            // Only 2 components react in first reaction
            if ( ncomp > 2 )
               tmp_rg.GroupComponent.append( component3 );

            reaction_used[ j ] = true;
            //counter++;   // Out of scope!!!!
         }

         if ( j == system.associations.size() - 1 )
         {
            rg .append( tmp_rg );
            return;
         }
      }
   }
}

// Initializes total concentration vector
void US_Astfem_RSA::initialize_conc( int kk, US_AstfemMath::MfemInitial& CT0,
      bool noninteracting )
{
DbgLv(2) << "RSA: init_conc() ENTER kk" << kk;
   US_Model::SimulationComponent* sc = &system.components[ kk ];

   // We don't have an existing CT0 concentration vector. Build up the initial
   // concentration vector with constant concentration

   if ( af_c0.concentration.size() == 0 )
   {
double mxct=0.0;
int jmxc=0;
      if ( simparams.band_forming )
      {
         // Calculate the width of the lamella
         double angl = simparams.cp_angle   != 0.0 ? simparams.cp_angle   : 2.5;
         double plen = simparams.cp_pathlen != 0.0 ? simparams.cp_pathlen : 1.2;
DbgLv(2) << "RSA: angle pathlen" << angl << plen;
DbgLv(2) << "RSA:  bandvol" << simparams.band_volume << " CT0concsz" << CT0.concentration.size();
         double base = sq( af_params.current_meniscus )
            + simparams.band_volume * 360.0 / ( angl * plen * M_PI );

         double lamella_width = sqrt( base ) - af_params.current_meniscus;
DbgLv(2) << "RSA:   menisc base lwid" << af_params.current_meniscus << base << lamella_width;

         // Calculate the spread of the lamella:
         for ( int j = 0; j < CT0.concentration.size(); j++ )
         {
            base = ( CT0.radius[ j ] - af_params.current_meniscus )
               / lamella_width;

            CT0.concentration[ j ] +=
               sc->signal_concentration * exp( -pow( base, 4.0 ) );
if(j<2||j>(CT0.concentration.size()-3)||j==(CT0.concentration.size()/40))
DbgLv(1) << "RSA:  j base conc" << j << base << CT0.concentration[j];
if(mxct<CT0.concentration[j]) {mxct=CT0.concentration[j];jmxc=j;}
         }
      }

      else  // !simparams.band_forming
      {
         for ( int j = 0; j < CT0.concentration.size(); j++ )
         {
            CT0.concentration[j] += sc->signal_concentration;
if(mxct<CT0.concentration[j]) {mxct=CT0.concentration[j];jmxc=j;}
         }
      }
DbgLv(2) << "RSA:   kk jmxc" << kk << jmxc << "max_conc" << mxct;
   }

   else  // af_c0.concentration.size() > 0
   {
      if ( noninteracting )
      {
         // Take the existing initial concentration vector and copy it to the
         // temporary CT0 vector: needs rubber band to make sure meniscus and
         // bottom equal current_meniscus and current_bottom

         int    nval  = af_c0.radius.size();
         CT0.radius       .clear();
         CT0.concentration.clear();
         CT0.radius       .reserve( nval );
         CT0.concentration.reserve( nval );
         //CT0 = system.components[ k ].c0;
         double conc0 = 0.0;
         for ( int jj = 0; jj < nval; jj++ )
         {
            CT0.radius       .append( af_c0.radius       [ jj ] );
            CT0.concentration.append( af_c0.concentration[ jj ] );
            conc0 += af_c0.concentration[ jj ];
         }
      }

      else // interpolation
      {
         US_AstfemMath::MfemInitial C;
         C.radius       .clear();
         C.concentration.clear();
         C.radius       .reserve( CT0.concentration.size() );
         C.concentration.reserve( CT0.concentration.size() );

         double dr  = ( af_params.current_bottom - af_params.current_meniscus )
                     / ( CT0.concentration.size() - 1 );
         double rad = af_params.current_meniscus;

         for ( int j = 0; j < CT0.concentration.size(); j++ )
         {
            C.radius       .append( rad );
            C.concentration.append(  0.0 );
            rad += dr;
         }

         US_AstfemMath::interpolate_C0( af_c0, C );

         for ( int j = 0; j < CT0.concentration.size(); j++ )
            CT0.concentration[ j ] += C.concentration[ j ];
      }
   }
DbgLv(2) << "RSA: init_conc() RETURN";
}

// Non-interacting solute, constant speed
int US_Astfem_RSA::calculate_ni( double rpm_start, double rpm_stop,
      US_AstfemMath::MfemInitial& C_init, US_AstfemMath::MfemData& simdata,
      bool accel )
{
#ifdef NO_DB
   static int      Nsave  = 0;
   static int      Nsavea = 0;
   static double** CA = NULL;     // stiffness matrix on left hand side
                                  // CA[0...Ms-1][0...N-1][4]

   static double** CB = NULL;     // stiffness matrix on right hand side
                                  // CB[0...Ms-1][0...N-1][4]

   static double** CA1;           // for matrices used in acceleration
   static double** CA2;
   static double** CB1;
   static double** CB2;
#else
   double** CA = NULL;            // stiffness matrix on left hand side
                                  // CA[0...Ms-1][0...N-1][4]

   double** CB = NULL;            // stiffness matrix on right hand side
                                  // CB[0...Ms-1][0...N-1][4]


   double** CA1;                  // for matrices used in acceleration
   double** CA2;
   double** CB1;
   double** CB2;
#endif

   double*         C0 = NULL;     // C[m][j]: current/next concentration of
                                  // m-th component at x_j
   double*         C1 = NULL;     // C[0...Ms-1][0....N-1]:

#if 0
#define TIMING_NI 1
#endif
#ifdef TIMING_NI
static int nccall=0;
static int ttTC=0;
static int ttT1=0;
static int ttT2=0;
static int ttT3=0;
static int ttT4=0;
static int ttT5=0;
static int ttT6=0;
static int ttT7=0;
static int ttT8=0;
QDateTime clcSt0 = QDateTime::currentDateTime();
QDateTime clcSt1 = clcSt0;
QDateTime clcSt2 = clcSt0;
QDateTime clcSt3 = clcSt0;
QDateTime clcSt4 = clcSt0;
QDateTime clcSt5 = clcSt0;
QDateTime clcSt6 = clcSt0;
QDateTime clcSt7 = clcSt0;
QDateTime clcSt8 = clcSt0;
QDateTime clcSt9 = clcSt0;
#endif

   simdata.radius.clear();
   simdata.scan  .clear();
   w2t_integral  = af_params.start_om2t;
   last_time     = af_params.start_time;

   US_AstfemMath::MfemScan simscan;

   // Generate the adaptive mesh

   xA         = x.data();
   double sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );
   QVector< double > nu;
   nu.clear();
   nu .append( sw2 / af_params.D[ 0 ] );

   mesh_gen( nu, simparams.meshType );

   // Refine left hand side (when s>0) or right hand side (when s < 0) for
   // acceleration

   if ( accel )
   {
      int   j;
      double xc;

      if ( af_params.s[ 0 ] > 0 )
      {
         // Radial distance from meniscus how far the boundary will move during
         // this acceleration step (without diffusion)

         xc = af_params.current_meniscus +
              sw2 * ( af_params.time_steps * af_params.dt ) / 3.0;

         for ( j = 0; j < N - 3; j++ )
            if ( xA[ j ] > xc ) break;
      }
      else
      {
         xc = af_params.current_bottom +
              sw2 * ( af_params.time_steps * af_params.dt ) / 3.0;

         for ( j = 0; j < N - 3; j++ )
            if ( xA[ N - j - 1 ] < xc ) break;
      }

      mesh_gen_RefL( j + 1, 4 * j );
   }

   simdata.radius.reserve( N );
   simdata.scan  .reserve( N );

   for ( int i = 0; i < N; i++ ) simdata.radius .append( xA[ i ] );

   // Initialize the coefficient matrices

#ifdef NO_DB
   if ( N > Nsave )
   {
      if ( Nsave > 0 )
      {
         US_AstfemMath::clear_2d( 3, CA );
         US_AstfemMath::clear_2d( 3, CB );
      }

      Nsave = N;
      US_AstfemMath::initialize_2d( 3, Nsave, &CA );
      US_AstfemMath::initialize_2d( 3, Nsave, &CB );
   }
#else
   US_AstfemMath::initialize_2d( 3, N, &CA );
   US_AstfemMath::initialize_2d( 3, N, &CB );
#endif

   bool fixedGrid = ( simparams.gridType == US_SimulationParameters::FIXED );
#ifdef TIMING_NI
clcSt2 = QDateTime::currentDateTime();
ttT1+=(clcSt1.msecsTo(clcSt2));
#endif

   if ( ! accel ) // No acceleration
   {
      sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );

      if ( fixedGrid )
      {
         ComputeCoefMatrixFixedMesh( af_params.D[ 0 ], sw2, CA, CB );
      }
      else
      {
         if ( af_params.s[ 0 ] > 0 )
           ComputeCoefMatrixMovingMeshR( af_params.D[ 0 ], sw2, CA, CB );
         else
           ComputeCoefMatrixMovingMeshL( af_params.D[ 0 ], sw2, CA, CB );
      }
   }
   else // For acceleration
   {
#ifdef NO_DB
      if ( N > Nsavea )
      {
         if ( Nsavea > 0 )
         {
            US_AstfemMath::clear_2d( 3, CA1 );
            US_AstfemMath::clear_2d( 3, CB1 );
            US_AstfemMath::clear_2d( 3, CA2 );
            US_AstfemMath::clear_2d( 3, CB2 );
         }

         US_AstfemMath::initialize_2d( 3, N, &CA1 );
         US_AstfemMath::initialize_2d( 3, N, &CA2 );
         US_AstfemMath::initialize_2d( 3, N, &CB1 );
         US_AstfemMath::initialize_2d( 3, N, &CB2 );
         Nsavea = N;
      }
#else
      US_AstfemMath::initialize_2d( 3, N, &CA1 );
      US_AstfemMath::initialize_2d( 3, N, &CA2 );
      US_AstfemMath::initialize_2d( 3, N, &CB1 );
      US_AstfemMath::initialize_2d( 3, N, &CB2 );
#endif

      sw2 = 0.0;
      ComputeCoefMatrixFixedMesh( af_params.D[ 0 ], sw2, CA1, CB1 );

      sw2 = af_params.s[ 0 ] * sq( rpm_stop * M_PI / 30 );
      ComputeCoefMatrixFixedMesh( af_params.D[ 0 ], sw2, CA2, CB2 );
   }
#ifdef TIMING_NI
clcSt3 = QDateTime::currentDateTime();
ttT2+=(clcSt2.msecsTo(clcSt3));
#endif

   // Initial condition
   QVector< double > C0Vec( N );
   QVector< double > C1Vec( N );
   QVector< double > rhVec( N );
   C0 = C0Vec.data();
   C1 = C1Vec.data();

   // Interpolate the given C_init vector on the new C0 grid
   US_AstfemMath::interpolate_C0( C_init, C0, x );

   // Time evolution
   double* right_hand_side = rhVec.data();
#ifdef TIMING_NI
ttT3+=(clcSt3.msecsTo(QDateTime::currentDateTime()));
clcSt3 = QDateTime::currentDateTime();
#endif
   double time_inc    = ( af_params.time_steps > 0  &&  rpm_stop != rpm_start )
                        ? (double)( af_params.time_steps ) : 1.0;
   double rpm_inc     = ( rpm_stop - rpm_start ) / time_inc;
   double rpm_current = rpm_start - rpm_inc;

   // Calculate all time steps (plus a little overlap)
   for ( int ii = 0; ii < af_params.time_steps + 1; ii++ )
   {
#ifdef TIMING_NI
clcSt4 = QDateTime::currentDateTime();
ttT3+=(clcSt3.msecsTo(clcSt4));
#endif
      rpm_current   += rpm_inc;

#ifndef NO_DB
      emit current_speed( (int) rpm_current );
#endif

      if ( accel ) // Then we have acceleration
      {
         double rpm_ratio = sq( rpm_current / rpm_stop );

         for ( int j1 = 0; j1 < 3; j1++ )
         {
            double* CAj  = CA[ j1 ];
            double* CBj  = CB[ j1 ];
            double* CA1j = CA1[ j1 ];
            double* CA2j = CA2[ j1 ];
            double* CB1j = CB1[ j1 ];
            double* CB2j = CB2[ j1 ];

            for ( int j2 = 0; j2 < N; j2++ )
            {
               CAj[ j2 ] = CA1j[ j2 ] + rpm_ratio * ( CA2j[ j2 ] - CA1j[ j2 ] );

               CBj[ j2 ] = CB1j[ j2 ] + rpm_ratio * ( CB2j[ j2 ] - CB1j[ j2 ] );
            }
         }
      }

      simscan.rpm   = (int) rpm_current;
      simscan.time  = af_params.start_time + ii * af_params.dt;

      w2t_integral += ( simscan.time - last_time ) *
                      sq( rpm_current * M_PI / 30.0 );
if(ii<2||ii>af_params.time_steps-1) {
DbgLv(1) << "TMS:RSA:ni: sc" << ii << "time ltime omg"
 << simscan.time << last_time << w2t_integral << "rpm" << rpm_current
 << "st_omg" << af_params.start_om2t << "N" << N;}

      last_time           = simscan.time;
      simscan.omega_s_t   = w2t_integral;
      simscan.temperature = af_data.scan[ 0 ].temperature;
DbgLv(2) << "TMS:RSA:ni: time omega_s_t" << simscan.time << simscan.omega_s_t
   << "rpm_c" << rpm_current << "step-scan" << simdata.scan.size();

      simscan.conc.clear();
      simscan.conc.reserve( N );

      for ( int j = 0; j < N; j++ ) simscan.conc .append( C0[ j ] );

      simdata.scan.append( simscan );
if(ii==0) DbgLv(1) << "TMS:RSA:ni:  Scan Added";
#ifdef TIMING_NI
clcSt5 = QDateTime::currentDateTime();
ttT4+=(clcSt4.msecsTo(clcSt5));
#endif

      // Sedimentation part:
      // Calculate the right hand side vector

      if ( accel || fixedGrid )
      {
         right_hand_side[ 0 ] = - CB[ 1 ][ 0 ] * C0[ 0 ]
                                - CB[ 2 ][ 0 ] * C0[ 1 ];

         for ( int j = 1; j < N - 1; j++ )
         {
            right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j - 1 ]
                                   - CB[ 1 ][ j ] * C0[ j     ]
                                   - CB[ 2 ][ j ] * C0[ j + 1 ];
         }

         int j = N - 1;
         right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j - 1 ]
                                - CB[ 1 ][ j ] * C0[ j     ];
      }
      else
      {
         if ( af_params.s[ 0 ] > 0 )
         {
            right_hand_side[ 0 ] = - CB[ 2 ][ 0 ] * C0[ 0 ];
            right_hand_side[ 1 ] = - CB[ 1 ][ 1 ] * C0[ 0 ]
                                   - CB[ 2 ][ 1 ] * C0[ 1 ];

            for ( int j = 2; j < N; j++ )
            {
               right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j - 2 ]
                                      - CB[ 1 ][ j ] * C0[ j - 1 ]
                                      - CB[ 2 ][ j ] * C0[ j     ];
            }
         }
         else
         {
            for ( int j = 0; j < N - 2; j++ )
            {
               right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j     ]
                                      - CB[ 1 ][ j ] * C0[ j + 1 ]
                                      - CB[ 2 ][ j ] * C0[ j + 2 ];
            }

            int j = N - 2;
            right_hand_side[ j ] = - CB[ 0 ][ j ] * C0[ j     ]
                                   - CB[ 1 ][ j ] * C0[ j + 1 ];

            j = N - 1;
            right_hand_side[ j ] = -CB[ 0 ][ j ] * C0[ j ];
         }
      }

#ifdef TIMING_NI
clcSt6 = QDateTime::currentDateTime();
ttT5+=(clcSt5.msecsTo(clcSt6));
#endif
      US_AstfemMath::tridiag( CA[0], CA[1], CA[2], right_hand_side, C1, N );
#ifdef TIMING_NI
clcSt7 = QDateTime::currentDateTime();
ttT6+=(clcSt6.msecsTo(clcSt7));
#endif

      for ( int j = 0; j < N; j++ ) C0[ j ] = C1[ j ];

#ifndef NO_DB
      //if ( show_movie  &&  (ii%4) == 0 )
      if ( show_movie )
      {
         if ( stopFlag ) break;

         emit new_scan( &x, C0 );
         emit new_time( simscan.time );
         qApp->processEvents();
         //US_Sleep::msleep( 10 ); // 10 ms to let the display update.
         //US_Sleep::msleep( 1 );  // 1 ms to let the display update.
      }
#endif
#ifdef TIMING_NI
clcSt3 = QDateTime::currentDateTime();
ttT7+=(clcSt7.msecsTo(clcSt3));
#endif
   } // time loop

#ifdef TIMING_NI
clcSt8 = QDateTime::currentDateTime();
#endif
   C_init.radius       .clear();
   C_init.concentration.clear();
   C_init.radius       .reserve( N );
   C_init.concentration.reserve( N );

   for ( int j = 0; j < N; j++ )
   {
      C_init.radius        .append( x [ j ] );
      C_init.concentration .append( C1[ j ] );
   }

#ifndef NO_DB
   US_AstfemMath::clear_2d( 3, CA );
   US_AstfemMath::clear_2d( 3, CB );

   if ( accel )
   {
      US_AstfemMath::clear_2d( 3, CA1 );
      US_AstfemMath::clear_2d( 3, CB1 );
      US_AstfemMath::clear_2d( 3, CA2 );
      US_AstfemMath::clear_2d( 3, CB2 );
   }
#endif

#ifdef TIMING_NI
clcSt9 = QDateTime::currentDateTime();
ttT8+=(clcSt8.msecsTo(clcSt9));
int elapsCalc=clcSt0.msecsTo(clcSt9);
nccall++;
ttTC+=elapsCalc;
if((nccall%1000)==0) {
 int totTK=ttT1+ttT2+ttT3+ttT4+ttT5+ttT6+ttT7+ttT8;
 qDebug() << "   ++Elapsed calc-ni ms" << elapsCalc << " nc totC totK" << nccall
  << ttTC << totTK << " N" << N << " acc fixG" << accel << fixedGrid;
 qDebug() << "     CN: t1 t2 t3 t4 t5 t6 t7 t8" << ttT1 << ttT2 << ttT3 << ttT4
  << ttT5 << ttT6 << ttT7 << ttT8;
 int p0=totTK/2;
 int p1=(ttT1*100+p0)/totTK;
 int p2=(ttT2*100+p0)/totTK;
 int p3=(ttT3*100+p0)/totTK;
 int p4=(ttT4*100+p0)/totTK;
 int p5=(ttT5*100+p0)/totTK;
 int p6=(ttT6*100+p0)/totTK;
 int p7=(ttT7*100+p0)/totTK;
 int p8=(ttT8*100+p0)/totTK;
 qDebug() << "      CN: p1 p2 p3 p4 p5 p6 p7 p8" << p1 << p2 << p3 << p4
  << p5 << p6 << p7 << p8;
}
#endif

   return 0;
}

void US_Astfem_RSA::mesh_gen( QVector< double >& nu, int MeshOpt )
{
//////////////////////////////////////////////////////////////%
//
// Generate adaptive grids for multi-component Lamm equations
//
//
// Here: N: Number of points in the ASTFEM
//    m, b: meniscus, bottom
//    nuMax, nuMin = max and min of nu=sw^2/D
//    MeshType: = 0 ASTFEM grid based on all nu (composite in sharp region)
//              = 1 Claverie (uniform), etc,
//              = 2 Exponential mesh (Schuck's form., no refinement at bottom)
//              = 3 input from data file: "mesh_data.dat"
//              = 4 ASTFVM grid (Finite Volume Method)
//              = 10, acceleration mesh (left and right refinement)
//////////////////////////////////////////////////////////////%

////////////////////%
// generate the mesh
////////////////////%

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    NN = af_params.simpoints;

   x.clear();
   x.reserve( NN * 2 + 2 );

   switch ( MeshOpt )
   {
      //////////////////////%
      // Mesh Type 0 (default): adaptive mesh based on all nu
      //////////////////////%

      case (int)US_SimulationParameters::ASTFEM:
         // Adaptive Space Time FE Mesh without left hand refinement
         qSort( nu );   // put nu in ascending order

         if ( nu[ 0 ] > 0 )
            mesh_gen_s_pos( nu );

         else if ( nu[ nu.size() - 1 ] < 0 )
            mesh_gen_s_neg( nu );

         else       // Some species with s < 0 and some with s > 0
         {
            double bmval  = m;
            double deltbm = ( b - m ) / (double)( NN - 1 );

            for ( int i = 0; i < NN; i++ )
            {
               x .append( bmval );
               bmval += deltbm;
            }
         }
         break;

      case (int)US_SimulationParameters::CLAVERIE:
         // Claverie mesh without left hand refinement

         for ( int i = 0; i < NN; i++ )
            x .append( m + ( b - m ) * i / ( NN - 1 ) );
         break;

      case (int)US_SimulationParameters::MOVING_HAT:
         // Moving Hat (Peter Schuck's Mesh) w/o left hand side refinement

         x .append( m );

         // Standard Schuck grids
         for ( int i = 1; i < NN - 1; i++ )
            x .append( m * pow( b / m, ( i - 0.5 ) / ( NN - 1 ) ) );

         x .append( b );
         break;

      case (int)US_SimulationParameters::USER:
         // User defined mesh generated from data file
         {
            //QString home = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
            QString home = US_Settings::appBaseDir();

            QFile f( home + "/etc/mesh.dat" );

            if ( f.open( QIODevice::ReadOnly ) )
            {
               QTextStream ts( &f );

               while ( ! ts.atEnd() )  x .append( ts.readLine().toDouble() );

               f.close();

               if ( fabs( xA[ 0 ] - m ) > 1.0e7 )
               {
                  qDebug() << "The meniscus from the mesh file does not"
                     " match the set meniscus - using Claverie Mesh instead\n";
               }

               if ( fabs( xA[ x.size() - 1 ] - b ) > 1.0e7 )
               {
                  qDebug() << "The cell bottom from the mesh file does not"
                     " match the set cell bottom - using Claverie Mesh"
                     " instead\n";
               }
            }
            else
            {
               qDebug() << "Could not read the mesh file - "
                           "using Claverie Mesh instead\n";

               for ( int i = 0; i < af_params.simpoints; i++ )
                  x .append( m + ( b - m ) * i / ( NN - 1 ) );
            }
            break;
         }

      case (int)US_SimulationParameters::ASTFVM:
         // Adaptive Space Time Finite Volume Method
         qSort( nu );   // put nu in ascending order

         if ( nu[ 0 ] > 0 )
            mesh_gen_s_pos( nu );

         else if ( nu[ nu.size() - 1 ] < 0 )
            mesh_gen_s_neg( nu );

         else       // Some species with s < 0 and some with s > 0
         {
            for ( int i = 0; i < NN; i++ )
               x .append( m + ( b - m ) * i / ( NN - 1 ) );
         }
         break;

      default:
         qDebug() << "undefined mesh option\n";
         break;

   }

   N  = x.size();
   xA = x.data();
}

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell bottom (for s>0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_pos( const QVector< double >& nuvec )
{
   double            tmp_Hstar;
   QVector< double > xc;
   QVector< double > Hstar;
   QVector< double > y;
   QVector< int >    Nf;
   const double*     nu = nuvec.data();

   xc   .clear();
   Hstar.clear();
   Nf   .clear();
   xc   .reserve( af_params.s.size() * 2 );
   Hstar.reserve( af_params.s.size() );
   Nf   .reserve( af_params.s.size() );

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    NN = af_params.simpoints;

   int    IndLayer = 0;         // number of layers for grids in steep region
   double uth      = 1.0 / NN;  // threshold of u for steep region
   double bmsqd    = sq( b ) - sq( m );
   double uth2     = uth * 2.0;
   double NNm1     = (double)( NN - 1 );
   double NNm2     = NNm1 - 1.0;
   double NNrat    = NNm2 / NNm1;
   double bmrat    = b / m;
   double bmrlog   = log( bmrat );
   double k2log    = log( 2.0   );
   double bmNpow   = pow( bmrat, NNrat );
   double bmdiff   = b - m * bmNpow;
   const double PIhalf   = M_PI / 2.0;

   for ( int i = 0; i < af_params.s.size(); i++ ) // Markers for steep regions
   {
      double tmp_xc = b - ( 1.0 / ( nu[ i ] * b ) ) *
         log( nu[ i ] * bmsqd ) / uth2;

      // # of pts for i-th layer
      int tmp_Nf = (int) ( PIhalf * ( b - tmp_xc )
            * nu[ i ] * b / 2.0 + 0.5 ) + 1;

      // Step required by Pac(i) < 1
      tmp_Hstar = ( b - tmp_xc ) / tmp_Nf * PIhalf;

      if ( ( tmp_xc > m ) && ( bmdiff > tmp_Hstar ) )
      {
         xc    .append( tmp_xc );
         Nf    .append( tmp_Nf );
         Hstar .append( tmp_Hstar );
         IndLayer++;
      }
   }

   xc .append( b );

   int indp = 0;   // Index for a grid point

   if ( IndLayer > 0 )
   { // Steep region  (potentially)

      for ( int i = 0; i < IndLayer; i++ )  // Consider i-th steep region
      {
         double xci  = xc[ i ];

         if ( i < IndLayer - 1 )  // Linear distribution for step size distrib
         {
            double xcip = xc[ i + 1 ];
            double HL = Hstar[ i ];
            double HR = Hstar[ i + 1 ];
            int Mp = (int) ( ( xcip - xci ) * 2.0 / ( HL + HR ) );

            if ( Mp > 1 )
            {
               double beta  = ( ( HR - HL ) / 2.0 ) * Mp;
               double alpha = xcip - xci - beta;

               for ( int j = 0; j <= Mp - 1; j++ )
               {
                  double xi = (double) j / (double) Mp;
                  y .append( xci + alpha * xi + beta * sq( xi ) );
                  indp++;
               }
            }
         }
         else     // Last layer, use sine distribution for grids
         {
            double xcip = xc[ qMin( i + 1, xc.size() - 1 ) ];

            for ( int j = 0; j <= Nf[ i ] - 1; j++ )
            {
               indp++;
               y .append( xci + ( b - xci ) *
                    sin( j / ( Nf[ i ] - 1.0 ) * PIhalf ) );

               if ( y[ indp - 1 ] > xcip )   break;
            }
         }
      }
   }

   if ( indp < 2 )
   {  // IndLayer==0  or  indp count less than 2
      x .append( m );

      // Add one more point to Schuck's grids
      for ( int k = 1; k < NN - 1 ; k++ )
      { // Schuck's mesh
         x .append( m * pow( bmrat, (double) k / NNm1 ) );
      }

      x .append( b );
   }

   else
   {  // IndLayer>0  and  indp greater than 1
      int NfTotal = indp;

      // Reverse the order of y
      int jj = 0;
      int kk = NfTotal - 1;
      double* yary = y.data();

      while ( jj < kk )
      {
         double ysav  = yary[ jj ];
         yary[ jj++ ] = yary[ kk ];
         yary[ kk-- ] = ysav;
      }

      // Transition region
      // Smallest step size in transit region
      double Hf = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];

      // Number of pts in trans region
      int Nm = (int)( log( b / ( NNm1 * Hf ) * bmrlog ) / k2log ) + 1;


      double xa = y[ NfTotal - 1 ] - Hf * ( pow( 2.0, (double) Nm ) - 1.0 );

      int Js = (int) ( NNm1 * log( xa / m ) / bmrlog );

      // xa is  modified so that y[ NfTotal - Nm ] matches xa exactly
      xa = m * pow( bmrat, Js / NNm1 );

      double tmp_xc = y[ NfTotal - 1 ];
      double HL     = xa * ( 1.0 - m / b );
      double HR     = y[ NfTotal - 2 ] - y[ NfTotal - 1 ];

      int Mp = (int)( ( tmp_xc - xa ) * 2.0 / ( HL + HR ) ) + 1;

      if ( Mp > 1 )
      {
         double beta  = ( ( HR - HL ) / 2.0 ) * Mp;
         double alpha = ( tmp_xc - xa ) - beta;

         for ( int j = Mp - 1; j > 0; j-- )
         {
            double xi = (double) j / Mp;
            y .append( xa + alpha * xi + beta * sq( xi ) );
         }
      }

      Nm = Mp;

      // Regular region
      x .append( m );
      yary = y.data();

      for ( int j = 1; j <= Js; j++ )
         x .append( m * pow( bmrat, (double)j / NNm1 ) );

      for ( int j = NfTotal + Nm - 2; j >=0; j-- )
         x .append( yary[ j ] );
   }

   xA = x.data();
}

//////////////////////////////////////////////////////////////%
//
// Generate exponential mesh and refine cell meniscus (for  s<0)
//
//////////////////////////////////////////////////////////////%
void US_Astfem_RSA::mesh_gen_s_neg( const QVector< double >& nu )
{

   const double PIhalf   = M_PI / 2.0;
   const double PIquar   = M_PI / 4.0;
   const double k2log    = log( 2.0 );
   int               j, Js, Nf, Nm;
   double            xc, xa, Hstar;
   QVector< double > yr, ys, yt;

   x .clear();
   yr.clear();
   ys.clear();
   yt.clear();

   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    NN = af_params.simpoints;

   double uth    = 1.0 / NN;   // Threshold of u for steep region
   double nu0    = qAbs( nu[ 0 ] );
   double bmrlog = log( b / m );
   double mbrat  = m / b;
   double NNm1   = (double)( NN - 1 );

   x .reserve( NN );
   yr.reserve( NN );
   ys.reserve( NN );
   yt.reserve( NN );

   xc = m + 1. / ( nu0 * m ) * log( ( sq( b ) - sq( m ) ) * nu0 / ( 2. * uth ) );

   Nf = 1 + (int)( ( xc - m ) * nu0 * m * PIquar );

   Hstar = ( xc - m ) / Nf * PIhalf;

   Nm = 1 + (int)( log( m / ( NNm1 * Hstar ) * bmrlog ) / k2log );

   xa = xc + ( pow( 2.0, (double) Nm ) - 1.0 ) * Hstar;

   Js = (int) ( NNm1 * log( b / xa ) / bmrlog + 0.5 );


   // All grid points at exponentials
   yr .append( b );

   // Is there a difference between simparams.meniscus and
   // af_params.current_meniscus??
   for( j = 1; j < NN; j++ )    // Add one more point to Schuck's grids
      yr .append( b * pow( simparams.meniscus / b, ( j - 0.5 ) / NNm1 ) );

   yr .append( m );

   if ( b * ( pow( mbrat, ( NN - 3.5 ) / NNm1 )
            - pow( mbrat, ( NN - 2.5 ) / NNm1 ) ) < Hstar || Nf <= 2 )
   {
      double* yrA = yr.data();

      // No need for steep region
      for ( j = NN - 1; j >= 0; j-- )
      {
         x .append( yrA[ j ] );
      }

      xA           = x.data();

      qDebug() << "Use exponential grid only!  NN Nf b m nu0" << NN << Nf << b << m << nu0;
   }
   else
   {
      // Nf > 2
      double xcm  = xc - m;
      double Nfm1 = (double)( Nf - 1 );
      for ( j = 0; j < Nf - 1; j++ )
         ys .append( xc - xcm * sin( (double)j / Nfm1 * PIhalf ) );

      ys .append( m );

      for ( j = 0; j < Nm; j++ )
         yt .append( xc + ( pow( 2.0, (double) j ) - 1.0 ) * Hstar );

      double* ysA = ys.data();
      double* ytA = yt.data();
      double* yrA = yr.data();

      // set x:
      for ( j = Nf - 1; j >= 0; j-- )
         x .append( ysA[ j ] );

      for ( j = 1; j < Nm; j++ )
         x .append( ytA[ j ] );

      for ( j = Js; j >= 0; j-- )
         x .append( yrA[ j ] );

      // Smooth out
      xA           = x.data();
      int jj       = Nf + Nm;
      xA[ jj     ] = ( xA[ jj - 1 ] + xA[ jj + 1 ] ) / 2.0;
      xA[ jj + 1 ] = ( xA[ jj     ] + xA[ jj + 2 ] ) / 2.0;
   } // if
}

///////////////////////////////////////////////////////////
//
// mesh_gen_RefL: refine mesh near meniscus (for s>0) or near bottom (for s<0)
//                to be used for the acceleration stage
//
//  parameters: N0 = number of elements near meniscus (or bottom) to be refined
//              M0 = number of elements to be used for the refined region
//
///////////////////////////////////////////////////////////

void US_Astfem_RSA::mesh_gen_RefL( int N0, int M0 )
{
   const double PIhalf   = M_PI / 2.0;
   QVector< double > zz;  // temporary array for adaptive grids
   double*           zA;

   zz.clear();
   zz.reserve( x.size() );
   xA = x.data();

   if ( US_AstfemMath::minval( af_params.s ) > 0 ) // All species with s>0
   {
      // Refine around the meniscus for acceleration
      for ( int j = 0; j < M0; j++ )
      {
         double tmp = (double) j / (double)M0;
         tmp        = 1.0 - cos( tmp * PIhalf );
         zz .append( xA[ 0 ] * ( 1.0 - tmp ) + xA[ N0 ] * tmp );
      }

      for ( int j = N0; j < x.size(); j++ )
         zz .append( xA[ j ] );

      x.clear();
      x.reserve( zz.size() );
      zA = zz.data();

      for ( int j = 0; j < zz.size(); j++ )
         x .append( zA[ j ] );
   }
   else if ( US_AstfemMath::maxval( af_params.s ) < 0 ) //  All species with s<0
   {
      for ( int j = 0; j < x.size() - N0; j++ )
         zz .append( xA[ j ] );

      // Refine around the bottom for acceleration
      int    kk = x.size() - 1;
      double x1 = xA[ kk - N0 ];
      double x2 = xA[ kk ];
      double tinc = PIhalf / (double)M0;
      double tval = 0.0;

      for ( int j = 1; j <= M0; j++ )
      {
         tval      += tinc;
         double tmp = sin( tval );
         zz .append( x1 * ( 1.0 - tmp ) + x2 * tmp );
      }

      x.clear();
      x.reserve( zz.size() );
      zA = zz.data();

      for ( int j = 0; j < zz.size(); j++ )
         x .append( zA[ j ] );
   }
   else                  // Sedimentation and floating mixed up
      qDebug() << "No refinement at ends since sedimentation "
                  "and floating mixed ...\n" ;

   N  = x.size();
   xA = x.data();
}

// Compute the coefficient matrices based on fixed mesh

void US_Astfem_RSA::ComputeCoefMatrixFixedMesh(
      double D, double sw2, double** CA, double** CB )
{
   if ( N != x.size()  ||  N < 1 )
      qDebug() << "***FixedMesh ERROR*** N x.size" << N << x.size()
         << " params.s[0] D sw2" << af_params.s[0] << D << sw2;

#ifdef NO_DB
   static int       Nsave = 0;
   static double*** Stif  = NULL;
   xA = x.data();

   if ( N > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = N + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
//qDebug() << "FixedMesh   Nsave" << Nsave;
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( N, 4, 4, &Stif );
#endif

   double xd[ 4 ][ 2 ];     // coord for vertices of quad elem

   for ( int k = 0; k < N - 1; k++ )
   {  // loop for all elem
      xd[ 0 ][ 0 ] = xA[ k ];
      xd[ 0 ][ 1 ] = 0.0;
      xd[ 1 ][ 0 ] = xA[ k + 1 ];
      xd[ 1 ][ 1 ] = 0.0;
      xd[ 2 ][ 0 ] = xA[ k + 1 ];
      xd[ 2 ][ 1 ] = af_params.dt;
      xd[ 3 ][ 0 ] = xA[ k ];
      xd[ 3 ][ 1 ] = af_params.dt;

      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // Assemble coefficient matrices
   // elem[ 0 ]; i=0
   int k = 0;
   int m = 0;
   CA[ 1 ][ k ] = Stif[ k ][ 3 ][ 0 ] + Stif[ k ][ 3 ][ 3 ]; // j=3;
   CA[ 2 ][ k ] = Stif[ k ][ 2 ][ 0 ] + Stif[ k ][ 2 ][ 3 ]; // j=2;
   CB[ 1 ][ k ] = Stif[ k ][ 0 ][ 0 ] + Stif[ k ][ 0 ][ 3 ]; // j=0;
   CB[ 2 ][ k ] = Stif[ k ][ 1 ][ 0 ] + Stif[ k ][ 1 ][ 3 ]; // j=1;

   for ( k = 1, m = 0; k < N - 1; k++, m++ )
   {  // loop for all elem
      // elem k-1: i=1,2
      CA[ 0 ][ k ]  = Stif[ m ][ 3 ][ 1 ] + Stif[ m ][ 3 ][ 2 ];  // j=3;
      CA[ 1 ][ k ]  = Stif[ m ][ 2 ][ 1 ] + Stif[ m ][ 2 ][ 2 ];  // j=2;
      CB[ 0 ][ k ]  = Stif[ m ][ 0 ][ 1 ] + Stif[ m ][ 0 ][ 2 ];  // j=0;
      CB[ 1 ][ k ]  = Stif[ m ][ 1 ][ 1 ] + Stif[ m ][ 1 ][ 2 ];  // j=1;

      // elem k: i=0,3
      CA[ 1 ][ k ] += Stif[ k ][ 3 ][ 0 ] + Stif[ k ][ 3 ][ 3 ];  // j=3;
      CA[ 2 ][ k ]  = Stif[ k ][ 2 ][ 0 ] + Stif[ k ][ 2 ][ 3 ];  // j=2;
      CB[ 1 ][ k ] += Stif[ k ][ 0 ][ 0 ] + Stif[ k ][ 0 ][ 3 ];  // j=0;
      CB[ 2 ][ k ]  = Stif[ k ][ 1 ][ 0 ] + Stif[ k ][ 1 ][ 3 ];  // j=1;
   }

   // elem[ N-2 ]; i=1,2
   k = N - 1;
   m = k - 1;
   CA[ 0 ][ k ]  = Stif[ m ][ 3 ][ 1 ] + Stif[ m ][ 3 ][ 2 ];  // j=3;
   CA[ 1 ][ k ]  = Stif[ m ][ 2 ][ 1 ] + Stif[ m ][ 2 ][ 2 ];  // j=2;
   CB[ 0 ][ k ]  = Stif[ m ][ 0 ][ 1 ] + Stif[ m ][ 0 ][ 2 ];  // j=0;
   CB[ 1 ][ k ]  = Stif[ m ][ 1 ][ 1 ] + Stif[ m ][ 1 ][ 2 ];  // j=1;

#ifndef NO_DB
   US_AstfemMath::clear_3d( N, 4, Stif );
#endif
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshR(
      double D, double sw2, double** CA, double** CB )
{
   // Compute local stiffness matrices
   double       xd[ 4 ][ 2 ]; // coord for verices of quad elem
   xA = x.data();

#ifdef NO_DB
   static int       Nsave = 0;
   static double*** Stif  = NULL;

   if ( N > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = N + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
//qDebug() << "MovMeshR   Nsave" << Nsave;
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( N, 4, 4, &Stif );
#endif

   // elem[0]: triangle
   xd[ 0 ][ 0 ] = xA[ 0 ];  xd[ 0 ][ 1 ] = 0.;
   xd[ 1 ][ 0 ] = xA[ 1 ];  xd[ 1 ][ 1 ] = af_params.dt;
   xd[ 2 ][ 0 ] = xA[ 0 ];  xd[ 2 ][ 1 ] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(N-2), quadrilateral
   for ( int k = 1; k < N - 1; k++ ) // loop for all elem
   {
      xd[ 0 ][ 0 ] = xA[ k - 1 ];   xd[ 0 ][ 1 ] = 0.0;
      xd[ 1 ][ 0 ] = xA[ k     ];   xd[ 1 ][ 1 ] = 0.0;
      xd[ 2 ][ 0 ] = xA[ k + 1 ];   xd[ 2 ][ 1 ] = af_params.dt;
      xd[ 3 ][ 0 ] = xA[ k     ];   xd[ 3 ][ 1 ] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[N-1]: triangle
   xd[ 0 ][ 0 ] = xA[ N - 2 ];   xd[ 0 ][ 1 ] = 0.0;
   xd[ 1 ][ 0 ] = xA[ N - 1 ];   xd[ 1 ][ 1 ] = 0.0;
   xd[ 2 ][ 0 ] = xA[ N - 1 ];   xd[ 2 ][ 1 ] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ N - 1 ] );

   // assembly coefficient matrices

   int k  = 0;
   int mm = 0;
   CA[ 1 ][ k ]  = Stif[ k ][ 2 ][ 2 ] ;
   CA[ 2 ][ k ]  = Stif[ k ][ 1 ][ 2 ] ;
   CB[ 2 ][ k ]  = Stif[ k ][ 0 ][ 2 ] ;

   k = 1;
   CA[ 0 ][ k ]  = Stif[ mm ][ 2 ][ 0 ] + Stif[ mm ][ 2 ][ 1 ];
   CA[ 1 ][ k ]  = Stif[ mm ][ 1 ][ 0 ] + Stif[ mm ][ 1 ][ 1 ];
   CA[ 1 ][ k ] += Stif[ k  ][ 3 ][ 0 ] + Stif[ k  ][ 3 ][ 3 ];
   CA[ 2 ][ k ]  = Stif[ k  ][ 2 ][ 0 ] + Stif[ k  ][ 2 ][ 3 ] ;

   CB[ 1 ][ k ]  = Stif[ mm ][ 0 ][ 0 ] + Stif[ mm ][ 0 ][ 1 ];   // j=0;
   CB[ 1 ][ k ] += Stif[ k  ][ 0 ][ 0 ] + Stif[ k  ][ 0 ][ 3 ];   // j=0;
   CB[ 2 ][ k ]  = Stif[ k  ][ 1 ][ 0 ] + Stif[ k  ][ 1 ][ 3 ];   // j=1;

   for( k = 2; k < N - 1; k++ )
   {  // loop for all elem
      // elem k-1: i=1,2
      mm = k - 1;
      CA[ 0 ][ k ]  = Stif[ mm ][ 3 ][ 1 ] + Stif[ mm ][ 3 ][ 2 ];  // j=3;
      CA[ 1 ][ k ]  = Stif[ mm ][ 2 ][ 1 ] + Stif[ mm ][ 2 ][ 2 ];  // j=2;
      CB[ 0 ][ k ]  = Stif[ mm ][ 0 ][ 1 ] + Stif[ mm ][ 0 ][ 2 ];  // j=0;
      CB[ 1 ][ k ]  = Stif[ mm ][ 1 ][ 1 ] + Stif[ mm ][ 1 ][ 2 ];  // j=1;

      // elem k: i=0,3
      CA[ 1 ][ k ] += Stif[ k ][ 3 ][ 0 ] + Stif[ k ][ 3 ][ 3 ];  // j=3;
      CA[ 2 ][ k ]  = Stif[ k ][ 2 ][ 0 ] + Stif[ k ][ 2 ][ 3 ];  // j=2;
      CB[ 1 ][ k ] += Stif[ k ][ 0 ][ 0 ] + Stif[ k ][ 0 ][ 3 ];  // j=0;
      CB[ 2 ][ k ]  = Stif[ k ][ 1 ][ 0 ] + Stif[ k ][ 1 ][ 3 ];  // j=1;
   }

   k  = N - 1;
   mm = k - 1;
   // elem[k-1]: quadrilateral
   CA[ 0 ][ k ]  = Stif[ mm ][3][1] + Stif[ mm ][3][2];  // j=3;
   CA[ 1 ][ k ]  = Stif[ mm ][2][1] + Stif[ mm ][2][2];  // j=2;
   CB[ 0 ][ k ]  = Stif[ mm ][0][1] + Stif[ mm ][0][2];  // j=0;
   CB[ 1 ][ k ]  = Stif[ mm ][1][1] + Stif[ mm ][1][2];  // j=1;

   // elem[k]: triangle
   CA[ 1 ][ k ] += Stif[ k  ][2][0] + Stif[ k  ][2][1] + Stif[ k ][2][2];
   CB[ 1 ][ k ] += Stif[ k  ][0][0] + Stif[ k  ][0][1] + Stif[ k ][0][2];
   CB[ 2 ][ k ]  = Stif[ k  ][1][0] + Stif[ k  ][1][1] + Stif[ k ][1][2];

#ifndef NO_DB
   US_AstfemMath::clear_3d( N, 4, Stif );
#endif
}

void US_Astfem_RSA::ComputeCoefMatrixMovingMeshL(
      double D, double sw2, double** CA, double** CB )
{
   // compute local stiffness matrices
   double       xd[4][2];   // coord for verices of quad elem
   xA = x.data();

#ifdef NO_DB
   static int       Nsave = 0;
   static double*** Stif  = NULL;

   if ( N > Nsave )
   {
      if ( Nsave > 0 )
         US_AstfemMath::clear_3d( Nsave, 4, Stif );

      Nsave = N + 200;
      US_AstfemMath::initialize_3d( Nsave, 4, 4, &Stif );
   }
#else
   double*** Stif  = NULL;
   US_AstfemMath::initialize_3d( N, 4, 4, &Stif );
#endif

   // elem[0]: triangle
   xd[0][0] = xA[0];
   xd[0][1] = 0.0;
   xd[1][0] = xA[1];  xd[1][1] = 0.0;
   xd[2][0] = xA[0];  xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ 0 ] );

   // elem[k]: k=1..(N-2), quadrilateral
   for ( int k = 1; k < N - 1; k++ )
   {  // loop for all elem
      xd[0][0] = xA[k  ];   xd[0][1] = 0.0;
      xd[1][0] = xA[k+1];   xd[1][1] = 0.0;
      xd[2][0] = xA[k  ];   xd[2][1] = af_params.dt;
      xd[3][0] = xA[k-1];   xd[3][1] = af_params.dt;
      stfb0.CompLocalStif( 4, xd, D, sw2, Stif[ k ] );
   }

   // elem[N-1]: triangle
   xd[0][0] = xA[N-1];   xd[0][1] = 0.0;
   xd[1][0] = xA[N-1];   xd[1][1] = af_params.dt;
   xd[2][0] = xA[N-2];   xd[2][1] = af_params.dt;
   stfb0.CompLocalStif( 3, xd, D, sw2, Stif[ N - 1 ] );

   // assembly coefficient matrices

   int k = 0;
   CA[1][0] = Stif[0][2][0] + Stif[0][2][1] + Stif[0][2][2];
   CB[0][0] = Stif[0][0][0] + Stif[0][0][1] + Stif[0][0][2] ;
   CB[1][0] = Stif[0][1][0] + Stif[0][1][1] + Stif[0][1][2] ;

   CA[1][0]+= Stif[1][3][0] + Stif[1][3][3] ;
   CA[2][0] = Stif[1][2][0] + Stif[1][2][3] ;
   CB[1][0]+= Stif[1][0][0] + Stif[1][0][3] ;
   CB[2][0] = Stif[1][1][0] + Stif[1][1][3] ;

   for ( int k = 1; k < N - 2; k++ )
   {  // loop for all elem
      // elem k:
      CA[0][k]  = Stif[k  ][3][1] + Stif[k  ][3][2];  // j=3;
      CA[1][k]  = Stif[k  ][2][1] + Stif[k  ][2][2];  // j=2;
      CB[0][k]  = Stif[k  ][0][1] + Stif[k  ][0][2];  // j=0;
      CB[1][k]  = Stif[k  ][1][1] + Stif[k  ][1][2];  // j=1;

      // elem k+1:
      CA[1][k] += Stif[k+1][3][0] + Stif[k+1][3][3];  // j=3;
      CA[2][k]  = Stif[k+1][2][0] + Stif[k+1][2][3];  // j=2;
      CB[1][k] += Stif[k+1][0][0] + Stif[k+1][0][3];  // j=0;
      CB[2][k]  = Stif[k+1][1][0] + Stif[k+1][1][3];  // j=1;
   }

   k = N - 2;
   // elem k:
   CA[0][k]  = Stif[k  ][3][1] + Stif[k  ][3][2];  // j=3;
   CA[1][k]  = Stif[k  ][2][1] + Stif[k  ][2][2];  // j=2;
   CB[0][k]  = Stif[k  ][0][1] + Stif[k  ][0][2];  // j=0;
   CB[1][k]  = Stif[k  ][1][1] + Stif[k  ][1][2];  // j=1;

   // elem k+1: (triangle)
   CA[1][k] += Stif[k+1][2][0] + Stif[k+1][2][2];  // j=3;
   CA[2][k]  = Stif[k+1][1][0] + Stif[k+1][1][2];  // j=2;
   CB[1][k] += Stif[k+1][0][0] + Stif[k+1][0][2];  // j=0;

   k = N - 1;
   // elem[k]: triangle
   CA[0][k]  = Stif[k  ][2][1] ;
   CA[1][k]  = Stif[k  ][1][1] ;
   CB[0][k]  = Stif[k  ][0][1] ;

#ifndef NO_DB
   US_AstfemMath::clear_3d( N, 4, Stif );
#endif
}

// Given total concentration of a group of components involved,
// find the concentration of each component by equilibrium condition
void US_Astfem_RSA::decompose( US_AstfemMath::MfemInitial* C0 )
{
   int num_comp = af_params.role.size();

   // Note: all components must be defined on the same radial grids
   int Npts = C0[ 0 ].radius.size();
DbgLv(2) << "RSA: decompose() num_comp Npts" << num_comp << Npts;

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // Only 2 components and one association rule
   {
      int    st0 = af_params.association[ 0 ].stoichs[ 0 ];
      int    st1 = af_params.association[ 0 ].stoichs[ 1 ];
      double keq = af_params.association[ 0 ].k_eq;
#ifndef NO_DB
      emit current_component( -Npts );
#endif

      for ( int j = 0; j < Npts; j++ )
      {
          double c1 = 0.0;
          double ct = C0[ 0 ].concentration[ j ] + C0[ 1 ].concentration[ j ] ;
//DbgLv(2) << "RSA:  j st0 st1" << j << st0 << st1;

          if ( st0 == 2 && st1 == -1 )                // mono <--> dimer
             c1 = ( sqrt( 1.0 + 4.0 * keq * ct ) - 1.0 ) / ( 2.0 * keq );

          else if ( st0 == 3 && st1 == -1 )           // mono <--> trimer
             c1 = US_AstfemMath::cube_root( -ct / keq, 1.0 / keq, 0.0 );

          else if ( st0 > 3 && st1 == -1 )           // mono <--> n-mer
             c1 = US_AstfemMath::find_C1_mono_Nmer( st0, keq, ct );

          else
          {
             qDebug() << "Warning: invalid stoichiometry in decompose()";
             qDebug() << "  st0 st1 c1" << st0 << st1 << c1;
             return;
          }

          double c2 = keq * pow( c1, (double)st0 );

          if ( af_params.role[ 0 ].stoichs[ 0 ] > 0 )    // c1=reactant
          {
              C0[ 0 ].concentration[ j ] = c1 ;
              C0[ 1 ].concentration[ j ] = c2 ;
          }
          else
          {
              C0[ 0 ].concentration[ j ] = c2 ;          // c1=product
              C0[ 1 ].concentration[ j ] = c1 ;
          }
#ifndef NO_DB
          emit current_component( j + 1 );
#endif
      }
DbgLv(2) << "RSA:  decompose NCOMP=2 return";
      return;
   }

   // General cases
   double** C1;
   double** C2;    // Arrays for all components at all radius position

   US_AstfemMath::initialize_2d( num_comp, Npts, &C1 );
   US_AstfemMath::initialize_2d( num_comp, Npts, &C2 );

   for( int i = 0; i < num_comp; i++ )
   {
      for( int j = 0; j < Npts; j++ )
      {
          C1[ i ][ j ] = C0[ i ].concentration[ j ];
          C2[ i ][ j ] = C1[ i ][ j ];
      }
   }

   // Estimate max time to reach equilibrium and suitable step size:
   // using e^{-k_min * N * dt ) < 1.e-7

   double k_min    = 1.0e12;

   // Get minimum k
   for ( int i = 0; i < af_params.association.size(); i++ )
   {
      if ( k_min > af_params.association[ i ].k_off )
           k_min = af_params.association[ i ].k_off;
   }

   if ( k_min < 1.0e-12 ) k_min = 1.0e-12;

   // Max number of time steps to get to equilibrium
   const int time_max     = 100;
   double    timeStepSize = - log( 1.0e-7 ) / ( k_min * time_max );

   // time loop
#ifndef NO_DB
   emit calc_start( time_max );
   emit current_component( -time_max );
#endif

   for ( int ti = 0; ti < time_max; ti++ )
   {
#ifndef NO_DB
      if ( show_movie  &&  (ti%8) == 0 )
      {
//DbgLv(2) << "AR: calc_progr ti" << ti;
         emit calc_progress( ti );
         qApp->processEvents();
         //US_Sleep::msleep( 10 );
      }
#endif

      ReactionOneStep_Euler_imp( Npts, C1, timeStepSize );

      double diff = 0.0;
      double ct   = 0.0;

      for ( int i = 0; i < num_comp; i++ )
      {
         for ( int j = 0; j < Npts; j++ )
         {
             diff        += fabs( C2[ i ][ j ] - C1[ i ][ j ] );
             ct          += fabs( C1[ i ][ j ] );
             C2[ i ][ j ] = C1[ i ][ j ];
         }
      }

#ifndef NO_DB
      emit current_component( ti + 1 );
      qApp->processEvents();
#endif

      if ( diff < 1.0e-5 * ct )
      {
#ifndef NO_DB
         int step = ti + 1;
         emit current_component( -step );
         emit current_component( step );
#endif
         break;
      }
   } // end time steps

#ifndef NO_DB
   if ( show_movie )
   {
      emit calc_done();
      qApp->processEvents();
   }
#endif

   for ( int i = 0; i < num_comp; i++ )
   {
      for ( int j = 0; j < Npts; j++ )
          C0[ i ].concentration[ j ] = C1[ i ][ j ];
   }

   US_AstfemMath::clear_2d( num_comp, C1 );
   US_AstfemMath::clear_2d( num_comp, C2 );
}

//////////////////////////////%
//
// ReactionOneStep_Euler_imp:  implicit Mid-point Euler
//
//////////////////////////////%
void US_Astfem_RSA::ReactionOneStep_Euler_imp(
      int Npts, double** C1, double timeStep )
{
   int num_comp = af_params.role.size();

   // Special case:  self-association  n A <--> An
   if ( num_comp == 2 )       // only  2 components and one association rule
   {
       double uhat;

       // Current rule used
       int    rule = rg[ af_params.rg_index ].association[ 0 ];
       int    st0  = system.associations[ rule ].stoichs[ 0 ];
       int    st1  = system.associations[ rule ].stoichs[ 1 ];
       double keq  = system.associations[ rule ].k_eq;
       double koff = system.associations[ rule ].k_off;

       for ( int j = 0; j < Npts; j++ )
       {
          double ct = C1[ 0 ][ j ] + C1[ 1 ][ j ];

          double dva = timeStep * koff * keq;
          double dvb = timeStep * koff + 2.;
          double dvc = timeStep * koff * ct + 2.0 * C1[ 0 ][ j ];

          if ( st0 == 2 && st1 == -1 )                // mono <--> dimer
             uhat = 2 * dvc / ( dvb + sqrt( dvb * dvb + 4 * dva * dvc ) );

          else if ( st0 == 3 && st1 == -1 )           // mono <--> trimer
             uhat = US_AstfemMath::cube_root( -dvc / dva, dvb / dva, 0.0 );

          else if ( st0  > 3 && st1 == -1 )            // mono <--> n-mer
             uhat = US_AstfemMath::find_C1_mono_Nmer( st0, dva / dvb, dvc / dvb);

          else
          {
             qDebug() << "Warning: invalid stoichiometry in decompose()";
             return;
          }

          if ( af_params.role[ 0 ].stoichs[ 0 ] > 0 )   // c1=reactant
          {
              C1[ 0 ][ j ] = 2 * uhat - C1[ 0 ][ j ];
              C1[ 1 ][ j ] = ct - C1[ 0 ][ j ];
          }

          else
          {                                             // c1=product
              C1[ 1 ][ j ] = 2 * uhat - C1[ 1 ][ j ];
              C1[ 0 ][ j ] = ct - C1[ 1 ][ j ];
          }
      }
       return;
   }

   // General cases
   const int iter_max = 20;      // maximum number of Newton iteration allowed

   double** A;

DbgLv(1) << "RSA: newX3 num_comp" << num_comp;
   QVector< double >  y0Vec( num_comp );
   QVector< double >  dnVec( num_comp );
   QVector< double >  bbVec( num_comp );
   double*  y0      = y0Vec.data();
   double*  delta_n = dnVec.data();
   double*  b       = bbVec.data();

   US_AstfemMath::initialize_2d( num_comp, num_comp, &A );

   for ( int j = 0; j < Npts; j++ )
   {
      double ct = 0.0;

      for ( int i = 0; i < num_comp; i++ )
      {
         y0[ i ]      = C1[ i ][ j ];
         delta_n[ i ] = 0.0;
         ct          += fabs( y0[ i ] );
      }

      for ( int iter = 0; iter < iter_max; iter++ ) // Newton iteration
      {
         double diff;

         for ( int i = 0; i < num_comp; i++ )
            y0[ i ] = C1[ i ][ j ] + delta_n[ i ];

         Reaction_dydt( y0, b );                  // b=dy/dt
         Reaction_dfdy( y0, A );                  // A=df/dy

         for ( int i = 0; i < num_comp; i++ )
         {
            for ( int k = 0; k < num_comp; k++ ) A[ i ][ k ] *= ( -timeStep );

            A[ i ][ i ] += 2.0;
            b[ i ]       = - ( 2 * delta_n[ i ] - timeStep * b[ i ] );
         }



         if ( US_AstfemMath::GaussElim( num_comp, A, b ) == -1 )
         {
            qDebug() << "Matrix singular in Reaction_Euler_imp: model 12";
            break;
         }
         else
         {
            diff = 0.0;

            for ( int i = 0; i < num_comp; i++ )
            {
               delta_n[ i ] += b[ i ];
               diff         += fabs( delta_n[ i ] );
            }
         }

         if ( diff < 1.0e-7 * ct ) break;
      } // End of Newton iteration;

      for ( int i = 0; i < num_comp; i++ ) C1[ i ][ j ] += delta_n[ i ];

   } // End of j (pts)

   US_AstfemMath::clear_2d( num_comp, A );
}

void US_Astfem_RSA::Reaction_dydt( double* y0, double* yt )
{
    US_AstfemMath::ReactionGroup* rgp = &rg[ af_params.rg_index ];
    int     num_comp  = rgp->GroupComponent.size();
    int     num_rule  = rgp->association.size();
    QVector< double > qqVec( num_rule );
    double* Q         = qqVec.data();

    for ( int m = 0; m < num_rule; m++ )
    {
       US_Model::Association* as = &af_params.association[ m ];
       double k_1        = as->k_off;
       double k1         = as->k_eq * k_1;
       double Q_reactant = 1.0;
       double Q_product  = 1.0;

       for ( int n = 0; n < as->rcomps.size(); n++ )
       {
          // local index of the n-th component in assoc[rule]
          int    ind_cn = as->rcomps[  n ] ;

          // stoichiometry of n-th component in the rule
          int    kstoi  = as->stoichs[ n ] ;
          int    react  = ( kstoi < 0 ) ? -1 : 1;
          double rstoi  = (double)( kstoi * react );

          // extinction coefficient of n'th component
          double extn   = af_params.kext[ rgp->GroupComponent[ ind_cn ] ];

          if ( react > 0 ) // comp[n] here is reactant
          {
             Q_reactant *= pow( y0[ ind_cn ] / extn, rstoi );
          }
          else             // comp[n] here is product
          {
             Q_product  *= pow( y0[ ind_cn ] / extn, rstoi );
          }
       }

       Q[ m ] = k1 * Q_reactant - k_1 * Q_product;
    }

    for ( int i = 0; i < num_comp; i++ )
    {
       yt[ i ] = 0.0;
       US_AstfemMath::ComponentRole* cr = &af_params.role[ i ];

       for ( int m = 0; m < cr->rcomps.size(); m++ )
       {
          yt[ i ] -= ( (double)cr->stoichs[ m ] * Q[ cr->rcomps[ m ] ] );
       }

       // convert molar into signal concentration
       yt[ i ] *= af_params.kext[ rgp->GroupComponent[ i ] ];
    }
}

void US_Astfem_RSA::Reaction_dfdy( double* y0, double** dfdy )
{
   double** QC;

   US_AstfemMath::ReactionGroup* rgp = &rg[ af_params.rg_index ];
   int num_comp  = rgp->GroupComponent.size();
   int num_rule  = rgp->association.size();

   US_AstfemMath::initialize_2d( num_rule, num_comp, &QC );

   for ( int m = 0; m < num_rule; m++ )
   {
      US_Model::Association* as = &af_params.association[ m ];
      double k_1  = as->k_off;
      double k1   = as->k_eq * k_1;

      for ( int j = 0; j < num_comp; j++ )
      {
         double Q_reactant = 1.0;
         double Q_product  = 1.0;
         double deriv_r    = 0.0;
         double deriv_p    = 0.0;

         for( int n = 0; n < as->rcomps.size(); n++ )
         {
            // Local index of the n-th component in assoc[rule]
            int    ind_cn = as->rcomps[ n ] ;

            // Stoichiometry of n-th comp in the rule
            int    kstoi  = as->stoichs[ n ] ;
            int    react  = ( kstoi < 0 ) ? -1 : 1;
            double rstoi  = (double)( kstoi * react );

            // Extinction coefficient of n'th component
            double extn   = af_params.kext[ rgp->GroupComponent[ ind_cn ] ];
            double yext   = y0[ ind_cn ] / extn;

            // comp[j] is in the rule
            if ( as->rcomps[ n ] == j )
            {
               if ( react > 0 ) // comp[n] is reactant
                  deriv_r = rstoi / extn * pow( yext, rstoi - 1.0 );

               else             // comp[n] in this rule is product
                  deriv_p = rstoi / extn * pow( yext, rstoi - 1.0 );
            }

            else                // comp[j] is not in the rule
            {
               if ( react > 0 ) // comp[n] is reactant
                  Q_reactant *= pow( yext, rstoi );

               else             // comp[n] in this rule is product
                  Q_product  *= pow( yext, rstoi );
            }
         }

         QC[ m ][ j ] = k1 * Q_reactant * deriv_r - k_1 * Q_product * deriv_p;
      }  // C_j
   }    // m-rule

   for ( int i = 0; i < num_comp; i++ )
   {
      US_AstfemMath::ComponentRole* cr = &af_params.role[ i ];

      for ( int j = 0; j < num_comp; j++ )
      {
         dfdy[ i ][ j ] = 0.0;

         for ( int m = 0; m < cr->rcomps.size(); m++ )
         {
            dfdy[ i ][ j ] -= ( (double)cr->stoichs[ m ]
                              *  QC[ cr->rcomps[ m ] ][ j ] );
         }

         // convert molar into signal concentration
         dfdy[ i ][ j ] *= af_params.kext[ rgp->GroupComponent[ i ] ];
      }
   }

   US_AstfemMath::clear_2d( num_rule, QC );
}

// *** this is the SNI version of operator scheme

int US_Astfem_RSA::calculate_ra2( double rpm_start, double rpm_stop,
      US_AstfemMath::MfemInitial* C_init, US_AstfemMath::MfemData& simdata,
      bool accel )
{
   int Mcomp = af_params.s.size();
   xA        = x.data();

   simdata.radius.clear();
   simdata.scan  .clear();
   simdata.radius.reserve( N );
   simdata.scan  .reserve( N );

   US_AstfemMath::MfemScan simscan;

   // Generate the adaptive mesh
   QVector< double > nu;
   nu.clear();
   nu.reserve( Mcomp );

DbgLv(2) << "RSA:     cra2: Mcomp" << Mcomp;
   for ( int i = 0; i < Mcomp; i++ )
   {
      double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
      nu .append( sw2 / af_params.D[ i ] );
   }
DbgLv(2) << "RSA:     cra2:  nu[N]" << nu[nu.size()-1];

   mesh_gen( nu, simparams.meshType );

   bool   fixedGrid = ( simparams.gridType == US_SimulationParameters::FIXED );
   double m  = af_params.current_meniscus;
   double b  = af_params.current_bottom;
   int    NN = af_params.time_steps;
   double dt = af_params.dt;

   // Refine left hand side (when s_max>0) or
   // right hand side (when s<0) for acceleration

   // Used for mesh and dt
   double s_max = US_AstfemMath::maxval( af_params.s );
   double s_min = US_AstfemMath::minval( af_params.s );

   if ( accel )
   {
      double xc ;

      if ( s_min > 0 )              // all sediment towards bottom
      {
         int   j;
         double sw2 = s_max * sq( rpm_stop * M_PI / 30 );
         xc = m + sw2 * ( NN * dt ) / 3;

         for ( j = 0; j < N - 3; j++ )
         {
            if ( xA[ j ] > xc ) break;
         }

         mesh_gen_RefL( j + 1, 4 * j );
      }
      else if ( s_max < 0 )      // all float towards meniscus
      {
         // s_min corresponds to fastest component
         int   j;
         double sw2 = s_min * sq( rpm_stop * M_PI / 30 );

         xc = b + sw2 * ( NN * dt) / 3;

         for ( j = 0; j < N - 3; j++ )
         {
            if ( xA[ N - j - 1 ] < xc )  break;
         }

         mesh_gen_RefL( j + 1, 4 * j );
      }
      else
      {
         qDebug() << "Multicomponent system with sedimentation and "
                     "floating mixed, use uniform mesh";
      }
   }

   for ( int i = 0; i < N; i++ ) simdata.radius .append( xA[ i ] );

   // Stiffness matrix on left hand side
   // CA[0...Ms-1][4][0...N-1]
   double*** CA;
   double*** CA1;
   double*** CA2;

   // Stiffness matrix on right hand side
   // CB[0...Ms-1][4][0...N-1]
   double*** CB;
   double*** CB1;
   double*** CB2;

   // Initialize the coefficient matrices
   US_AstfemMath::initialize_3d( Mcomp, 4, N, &CA );
   US_AstfemMath::initialize_3d( Mcomp, 4, N, &CB );

   if ( accel ) //  Acceleration, so use fixed grid
   {
      US_AstfemMath::initialize_3d( Mcomp, 3, N, &CA1 );
      US_AstfemMath::initialize_3d( Mcomp, 3, N, &CA2 );
      US_AstfemMath::initialize_3d( Mcomp, 3, N, &CB1 );
      US_AstfemMath::initialize_3d( Mcomp, 3, N, &CB2 );

      for( int i = 0; i < Mcomp; i++ )
      {
         double sw2 = 0.0;
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA1[ i ], CB1[ i ] );

         sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
         ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA2[ i ], CB2[ i ] );
      }
   }
   else     // Constant sedimentation speed
   {
      if ( fixedGrid )
      {
         for( int i = 0; i < Mcomp; i++ )
         {
            double sw2 = af_params.s[ i ] * sq( rpm_stop * M_PI / 30 );
            ComputeCoefMatrixFixedMesh( af_params.D[ i ], sw2, CA[ i ], CB[ i ] );
         }
      }
      else  // Moving grid
      {
         if ( s_min > 0)      // All components sedimenting
         {
            double stop_fact = sq( rpm_stop * M_PI / 30.0 );

            for ( int i = 0; i < Mcomp; i++ )
            {
               double            sw2  = af_params.s[ i ] * stop_fact;
               double            sw2D = sw2 / af_params.D[ i ];

               // Grid for moving adaptive FEM for faster sedimentation

               QVector< double > xbvec( N );
               double*           xb  = xbvec.data();
               double            sqb = sq( b );
               xb[ 0 ] = m;

               for ( int j = 1; j < N; j++ )
               {
                  double dval  = 0.1 * exp( sw2D *
                     ( sq( 0.5 * ( xA[ j - 1 ] + xA[ j ] ) ) - sqb ) / 2.0 );

                  double alpha = af_params.s[ i ] / s_max * ( 1 - dval ) + dval;
                  xb[ j ]      = ( pow( xA[ j - 1 ], alpha ) *
                                   pow( xA[ j     ], ( 1 - alpha) ) );
               }

               GlobalStiff( xb, CA[ i ], CB[ i ], af_params.D[ i ], sw2 );
            }
         }
         else if ( s_max < 0)    // all components floating
         {
            qDebug() << "all components floating, not implemented yet";
            return -1;
         }
         else     // sedmientation and floating mixed
         {
            qDebug() << "sedimentation and floating mixed, suppose use "
                        "fixed grid!";
            return -1;
         }
      } // moving mesh
   } // acceleration

   // Initial condition
   double** C0; // C[m][j]: current/next concentration of m-th component at x_j
   double** C1; // C[0...Ms-1][0....N-1]:

   US_AstfemMath::initialize_2d( Mcomp, N, &C0 );
   US_AstfemMath::initialize_2d( Mcomp, N, &C1 );

   // Here we need the interpolate the initial partial
   // concentration onto new grid x[j]

   for( int i = 0; i < Mcomp; i++ )
   {
      // Interpolate the given C_init vector on the new C0 grid
      US_AstfemMath::interpolate_C0( C_init[ i ], C0[ i ], x );
   }

   // Total concentration at current and next time step
DbgLv(1) << "RSA: newX3 N" << N;
   QVector< double > CT0vec( N );
   QVector< double > CT1vec( N );
   QVector< double > rhVec ( N );
   double* CT0 = CT0vec.data();
   double* CT1 = CT1vec.data();

   for ( int j = 0; j < N; j++ )
   {
       CT0[ j ] = 0.0;

       for ( int i = 0; i < Mcomp; i++ )  CT0[ j ] += C0[ i ][ j ];

       CT1[ j ] = CT0[ j ];
   }

   // Time evolution
   double* right_hand_side = rhVec.data();
#ifndef NO_DB
   int     stepinc = 1000;
   int     stepmax = ( NN + 2 ) / stepinc + 1;
   bool    repprog = stepmax > 1;
   if ( repprog )
   {
      emit current_component( -stepmax );
      emit current_component( 0 );
   }
#endif

   for ( int kkk = 0; kkk < NN + 2; kkk += 2 )   // two steps in together
   {
      double rpm_current = rpm_start +
         ( rpm_stop - rpm_start ) * ( kkk + 0.5 ) / NN;

#ifndef NO_DB
      emit current_speed( (int) rpm_current);
#endif

      simscan.time      = af_params.start_time + kkk * dt;
      simscan.rpm       = (int) rpm_current;
      w2t_integral     += ( ( simscan.time - last_time )
                            * sq( rpm_current * M_PI / 30 ) );
      last_time         = simscan.time;
      simscan.omega_s_t = w2t_integral;
DbgLv(2) << "TMS:RSA:ra: time omegast" << simscan.time << simscan.omega_s_t
   << "step-scan" << simdata.scan.size();

      simscan.conc.clear();
      simscan.conc.reserve( N );

      for ( int j = 0; j < N; j++ ) simscan.conc .append( CT0[ j ] );

      simdata.scan .append( simscan );

      // First half step of sedimentation:

      if ( accel ) // need to reconstruct CA and CB by linear interpolation
      {
         double dval = sq( rpm_current / rpm_stop );

         for ( int i = 0; i < Mcomp; i++ )
         {
            for ( int j1 = 0; j1 < 3; j1++ )
            {
               for ( int j2 = 0; j2 < N; j2++ )
               {
                  CA[ i ][ j1 ][ j2 ] = CA1[ i ][ j1 ][ j2 ] +
                               dval * ( CA2[ i ][ j1 ][ j2 ] -
                                        CA1[ i ][ j1 ][ j2 ] );

                  CB[ i ][ j1 ][ j2 ] = CB1[ i ][ j1 ][ j2 ] +
                               dval * ( CB2[ i ][ j1 ][ j2 ] -
                                        CB1[ i ][ j1 ][ j2 ] );
               }
            }
         }
      }

      if ( accel || fixedGrid )    // For fixed grid
      {
         for ( int i = 0; i < Mcomp; i++ )
         {
            right_hand_side[ 0 ] = - CB[ i ][ 1 ][ 0 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 1 ];

            for ( int j = 1; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = N - 1;

            right_hand_side[ j ] = -CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                  - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                    right_hand_side, C1[ i ], N );
         }
      }
      else // Moving grid
      {
         for ( int i = 0; i < Mcomp; i++ )
         {
            // Calculate the right hand side vector
            right_hand_side[ 0 ] = - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 3 ][ 0 ] * C0[ i ][ 1 ];

            right_hand_side[ 1 ] = - CB[ i ][ 1 ][ 1 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 2 ][ 1 ] * C0[ i ][ 1 ]
                                   - CB[ i ][ 3 ][ 1 ] * C0[ i ][ 2 ];

            for ( int j = 2; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ] [0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ] [1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ] [2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = N - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::QuadSolver( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                       CA[ i ][ 3 ], right_hand_side, C1[ i ],
                                       N );
         }
      }

      // Reaction part: instantanuous reaction at each node
      //
      // instantanuous reaction at each node
      // [C1]=ReactionOneStep_inst(C1);
      //
      // Finite reaction rate: linear interpolation of instantaneous reaction

      ReactionOneStep_Euler_imp( N, C1, 2 * dt );

      // For next half time-step in SNI operator splitting scheme

      for ( int j = 0; j < N; j++ )
      {
         CT1[ j ] = 0.0;

         for ( int i = 0; i < Mcomp; i++ )
         {
            CT1[ j ]   += C1[ i ][ j ];
            C0[ i][ j ] = C1[ i ][ j ];
         }

         CT0[ j ] = CT1[ j ];
      }

      // 2nd half step of sedimentation:

      rpm_current = rpm_start + ( rpm_stop - rpm_start ) * ( kkk + 1.5 ) / NN;

      if ( accel ) // Need to reconstruct CA and CB by linear interpolation
      {
         double dval = sq( rpm_current / rpm_stop );

         for ( int i = 0; i < Mcomp; i++ )
         {
            for ( int j1 = 0; j1 < 3; j1++ )
            {
               for ( int j2 = 0; j2 < N; j2++ )
               {
                  CA[ i][ j1 ][ j2 ] = CA1[ i ][ j1 ][ j2 ] +
                              dval * ( CA2[ i ][ j1 ][ j2 ] -
                                       CA1[ i ][ j1 ][ j2 ] );

                  CB[ i][ j1 ][ j2 ] = CB1[ i ][ j1 ][ j2 ] +
                              dval * ( CB2[ i ][ j1 ][ j2 ] -
                                       CB1[ i ][ j1 ][ j2 ] ) ;
               }
            }
         }
      }

      if ( accel || fixedGrid )                     // For fixed grid
      {
         for ( int i = 0; i < Mcomp; i++ )
         {
            right_hand_side[ 0 ] = - CB[ i ][ 1 ][ 0 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 1 ];

            for ( int j = 1; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = N - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::tridiag( CA[ i ][ 0 ], CA[ i ][ 1 ], CA[ i ][ 2 ],
                                    right_hand_side, C1[ i ], N );
         }
      }
      else // Moving grid
      {
         for ( int i = 0; i < Mcomp; i++ )
         {
            // Calculate the right hand side vector //
            right_hand_side[ 0 ] = - CB[ i ][ 2 ][ 0 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 3 ][ 0 ] * C0[ i ][ 1 ];

            right_hand_side[ 1 ] = - CB[ i ][ 1 ][ 1 ] * C0[ i ][ 0 ]
                                   - CB[ i ][ 2 ][ 1 ] * C0[ i ][ 1 ]
                                   - CB[ i ][ 3 ][ 1 ] * C0[ i ][ 2 ];

            for ( int j = 2; j < N - 1; j++ )
            {
               right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                      - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                      - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ]
                                      - CB[ i ][ 3 ][ j ] * C0[ i ][ j + 1 ];
            }

            int j = N - 1;
            right_hand_side[ j ] = - CB[ i ][ 0 ][ j ] * C0[ i ][ j - 2 ]
                                   - CB[ i ][ 1 ][ j ] * C0[ i ][ j - 1 ]
                                   - CB[ i ][ 2 ][ j ] * C0[ i ][ j     ];

            US_AstfemMath::QuadSolver( CA[ i ][ 0 ], CA[ i ][ 1 ],
                                       CA[ i ][ 2 ], CA[ i ][ 3 ],
                                       right_hand_side, C1[ i ], N );
         }
      }

      // End of 2nd half step of sendimentation

      // For next 2 time steps

      for ( int j = 0; j < N; j++ )
      {
         CT1[ j ] = 0.0;

         for ( int i = 0; i < Mcomp; i++ )
         {
            CT1[ j ]    += C1[ i ][ j ];
            C0[ i ][ j ] = C1[ i ][ j ];
         }

         CT0[ j ] = CT1[ j ];
      }

#ifndef NO_DB
      //if ( show_movie  &&  (kkk%8) == 0 )
      if ( show_movie )
      {
         if ( stopFlag ) break;

         emit new_scan( &x, CT0 );
         emit new_time( simscan.time );
         qApp->processEvents();
         //US_Sleep::msleep( 10 ); // 10 ms to let the display update.
         //US_Sleep::msleep( 1 );  // 1 ms to let the display update.
      }

      if ( repprog  &&  ( ( kkk + 1 ) & stepinc ) == 0 )
      {
         emit current_component( ( kkk + 1 ) / stepinc );
      }
#endif

   } // time loop
DbgLv(2) << "RSA:     cra2:  NN" << NN;

   for ( int i = 0; i < Mcomp; i++ )
   {
     C_init[ i ].radius       .clear();
     C_init[ i ].concentration.clear();
     C_init[ i ].radius       .reserve( N );
     C_init[ i ].concentration.reserve( N );

     for ( int j = 0; j < N; j++ )
     {
        C_init[ i ].radius        .append( xA[ j ] );
        C_init[ i ].concentration .append( C1[ i ][ j ] );
     }
   }

   US_AstfemMath::clear_2d(Mcomp, C0);
   US_AstfemMath::clear_2d(Mcomp, C1);
   US_AstfemMath::clear_3d(Mcomp, 4, CA);
   US_AstfemMath::clear_3d(Mcomp, 4, CB);

   if ( accel ) // then we have acceleration
   {
      US_AstfemMath::clear_3d( Mcomp, 3, CA1 );
      US_AstfemMath::clear_3d( Mcomp, 3, CB1 );
      US_AstfemMath::clear_3d( Mcomp, 3, CA2 );
      US_AstfemMath::clear_3d( Mcomp, 3, CB2 );
   }

   return 0;
}

void US_Astfem_RSA::GlobalStiff( double* xb, double** ca,
                                 double** cb, double D, double sw2 )
{
   //  4: global stifness matrix

   // function [CA, CB]=4(x, xb, dt, D, sw2)

   double*** Stif = NULL;
   double vx[ 8 ];

   US_AstfemMath::initialize_3d( N, 6, 2, &Stif );

   // 1st elem
   vx[ 0 ] = x [ 0 ];
   vx[ 1 ] = x [ 1 ];
   vx[ 2 ] = x [ 0 ];
   vx[ 3 ] = x [ 1 ];
   vx[ 4 ] = x [ 2 ];
   vx[ 5 ] = xb[ 1 ];
   US_AstfemMath::IntQT1( vx, D, sw2, Stif[ 0 ], af_params.dt );

   // elems in middle
   for ( int i = 1; i < N - 2; i++ )
   {
      vx[ 0 ] = x [ i - 1 ];
      vx[ 1 ] = x [ i     ];
      vx[ 2 ] = x [ i + 1 ];
      vx[ 3 ] = x [ i     ];
      vx[ 4 ] = x [ i + 1 ];
      vx[ 5 ] = x [ i + 2 ];
      vx[ 6 ] = xb[ i     ];
      vx[ 7 ] = xb[ i + 1 ];
      US_AstfemMath::IntQTm( vx, D, sw2, Stif[ i ], af_params.dt );
   }

   // 2nd last elems
   vx[ 0 ] = x [ N - 3 ];
   vx[ 1 ] = x [ N - 2 ];
   vx[ 2 ] = x [ N - 1 ];
   vx[ 3 ] = x [ N - 2 ];
   vx[ 4 ] = x [ N - 1 ];
   vx[ 5 ] = xb[ N - 2 ];
   vx[ 6 ] = xb[ N - 1 ];

   US_AstfemMath::IntQTn2( vx, D, sw2, Stif[ N - 2 ], af_params.dt );

   // last elems
   vx[ 0 ] = x [ N - 2 ];
   vx[ 1 ] = x [ N - 1 ];
   vx[ 2 ] = x [ N - 1 ];
   vx[ 3 ] = xb[ N - 1 ];
   US_AstfemMath::IntQTn1 ( vx, D, sw2, Stif[ N - 1 ], af_params.dt );

   // assembly into global stiffness matrix
   ca[ 0 ][ 0 ] = 0.0;
   ca[ 1 ][ 0 ] = Stif[ 0 ][ 2 ][ 0 ];
   ca[ 2 ][ 0 ] = Stif[ 0 ][ 3 ][ 0 ];
   ca[ 3 ][ 0 ] = Stif[ 0 ][ 4 ][ 0 ];

   cb[ 0 ][ 0 ] = 0.0;
   cb[ 1 ][ 0 ] = 0.0;
   cb[ 2 ][ 0 ] = Stif[ 0 ][ 0 ][ 0 ];
   cb[ 3 ][ 0 ] = Stif[ 0 ][ 1 ][ 0 ];

   // i=2
   ca[ 0 ][ 1 ] = Stif[ 0 ][ 2 ][ 1 ];
   ca[ 1 ][ 1 ] = Stif[ 0 ][ 3 ][ 1 ] + Stif[ 1 ][ 3 ][ 0 ];
   ca[ 2 ][ 1 ] = Stif[ 0 ][ 4 ][ 1 ] + Stif[ 1 ][ 4 ][ 0 ];
   ca[ 3 ][ 1 ] =                       Stif[ 1 ][ 5 ][ 0 ];

   cb[ 0 ][ 1 ] = 0.0;
   cb[ 1 ][ 1 ] = Stif[ 0 ][ 0 ][ 1 ] + Stif[ 1 ][ 0 ][ 0 ];
   cb[ 2 ][ 1 ] = Stif[ 0 ][ 1 ][ 1 ] + Stif[ 1 ][ 1 ][ 0 ];
   cb[ 3 ][ 1 ] =                       Stif[ 1 ][ 2 ][ 0 ];

   // i: middle
   for (  int i = 2; i < N - 2; i++ )
   {
      ca[ 0 ][ i ] = Stif[ i - 1 ][ 3 ][ 1 ];
      ca[ 1 ][ i ] = Stif[ i - 1 ][ 4 ][ 1 ] + Stif[ i ][ 3 ][ 0 ];
      ca[ 2 ][ i ] = Stif[ i - 1 ][ 5 ][ 1 ] + Stif[ i ][ 4 ][ 0 ];
      ca[ 3 ][ i ] =                           Stif[ i ][ 5 ][ 0 ];

      cb[ 0 ][ i ] = Stif[ i - 1 ][ 0 ][ 1 ];
      cb[ 1 ][ i ] = Stif[ i - 1 ][ 1 ][ 1 ] + Stif[ i ][ 0 ][ 0 ];
      cb[ 2 ][ i ] = Stif[ i - 1 ][ 2 ][ 1 ] + Stif[ i ][ 1 ][ 0 ];
      cb[ 3 ][ i ] =                           Stif[ i ][ 2 ][ 0 ];
   }

   // i=n
   int i = N - 2;
   ca[ 0 ][ i ] = Stif[ i - 1 ][ 3 ][ 1 ];
   ca[ 1 ][ i ] = Stif[ i - 1 ][ 4 ][ 1 ] + Stif[ i ][ 3 ][ 0 ];
   ca[ 2 ][ i ] = Stif[ i - 1 ][ 5 ][ 1 ] + Stif[ i ][ 4 ][ 0 ];
   ca[ 3 ][ i ] = 0.0;

   cb[ 0 ][ i ] = Stif[ i - 1 ][ 0 ][ 1 ];
   cb[ 1 ][ i ] = Stif[ i - 1 ][ 1 ][ 1 ] + Stif[ i ][ 0 ][ 0 ];
   cb[ 2 ][ i ] = Stif[ i - 1 ][ 2 ][ 1 ] + Stif[ i ][ 1 ][ 0 ];
   cb[ 3 ][ i ] =                           Stif[ i ][ 2 ][ 0 ];

   // i=n+1
   i = N - 1;
   ca[ 0 ][ i ] = Stif[ i - 1 ][ 3 ][ 1 ];
   ca[ 1 ][ i ] = Stif[ i - 1 ][ 4 ][ 1 ] + Stif[ i ][ 2 ][ 0 ];
   ca[ 2 ][ i ] = 0.0;
   ca[ 3 ][ i ] = 0.0;

   cb[ 0 ][ i ] = Stif[ i - 1 ][ 0 ][ 1 ];
   cb[ 1 ][ i ] = Stif[ i - 1 ][ 1 ][ 1 ] + Stif[ i ][ 0 ][ 0 ];
   cb[ 2 ][ i ] = Stif[ i - 1 ][ 2 ][ 1 ] + Stif[ i ][ 1 ][ 0 ];
   cb[ 3 ][ i ] = 0.0;

   US_AstfemMath::clear_3d( N, 6, Stif );
}

void US_Astfem_RSA::load_mfem_data( US_DataIO2::RawData&     edata,
                                    US_AstfemMath::MfemData& fdata )
{
   int  nscan  = edata.scanData.size();
   int  nconc  = edata.x.size();

   fdata.id    = edata.description;
   fdata.cell  = edata.cell;
   fdata.scan  .resize( nscan );
   fdata.radius.resize( nconc );

   for ( int ii = 0; ii < nscan; ii++ )
   {
      US_AstfemMath::MfemScan* fscan = &fdata.scan[ ii ];

      fscan->temperature = edata.scanData[ ii ].temperature;
      fscan->rpm         = edata.scanData[ ii ].rpm;
      fscan->time        = edata.scanData[ ii ].seconds;
      fscan->omega_s_t   = edata.scanData[ ii ].omega2t;
      fscan->conc.resize( nconc );

      for ( int jj = 0; jj < nconc; jj++ )
      {
         fscan->conc[ ii ] = edata.value( ii, jj );
      }
   }

   for ( int jj = 0; jj < nconc; jj++ )
   {
      fdata.radius[ jj ] = edata.radius( jj );
   }
DbgLv(2) << "RSA:f sc0 temp" << fdata.scan[0].temperature;
DbgLv(2) << "RSA:e sc0 temp" << edata.scanData[0].temperature;
}

void US_Astfem_RSA::store_mfem_data( US_DataIO2::RawData&     edata,
                                     US_AstfemMath::MfemData& fdata )
{
   int  nscan  = fdata.scan.size();
   int  nconc  = fdata.radius.size();

   edata.description = fdata.id;
   edata.cell        = fdata.cell;
   edata.scanData.resize( nscan );

   for ( int ii = 0; ii < nscan; ii++ )
   {
      US_AstfemMath::MfemScan* fscan = &fdata.scan    [ ii ];
      US_DataIO2::Scan*        escan = &edata.scanData[ ii ];

      escan->temperature = fscan->temperature;
      escan->rpm         = fscan->rpm;
      escan->seconds     = fscan->time;
      escan->omega2t     = fscan->omega_s_t;
      escan->plateau     = fdata.radius[ nconc - 1 ];
      escan->readings.resize( nconc );

      for ( int jj = 0; jj < nconc; jj++ )
      {
         escan->readings[ jj ] = US_DataIO2::Reading( fscan->conc[ jj ] );
      }
   }

   edata.x.resize( nconc );

   for ( int jj = 0; jj < nconc; jj++ )
   {
      edata.x[ jj ] = fdata.radius[ jj ];
   }
DbgLv(2) << "RSA:o-f sc0 temp" << fdata.scan[0].temperature;
DbgLv(2) << "RSA:o-e sc0 temp" << edata.scanData[0].temperature;
}
